#include "port_loop.h"
#include "diag_hooks.h"
#include "port_text.h"

#include <cstdint>
#include <cstring>

/* The menu loop and what it moves through, rewritten (port.h says how): the
 * screen shown, its items drawn layer by layer, the item selected with the
 * keys or found under the mouse, a key handed to the item that has it, and the
 * screens gone to and come back from.
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
 * items marked for it.
 *
 * Each function is a typed core and an adapter under the game's name.  The
 * cores call each other as C++, each given the esp its game's code is entered
 * at, so that what the game's calls out of it are made at the stack addresses
 * the game's code has; what is not rewritten, or is another part's rewrite
 * held on its own, is called by address (the item and screen handlers through
 * the pointers the tables hold).  A core that leaves a leftover in eax for its
 * caller to read (what a callee left there, or the argument that was in it)
 * takes it from the Game it is run with, which keeps eax as the game's code
 * would have it.  The game's code leaves ebx, ecx and edx otherwise than it
 * got them in some of these (the scratch of a Watcom function); no caller
 * reads them after (by liveness on nfs3.exe), but the ones it does read are
 * named in the adapters.  tools/game_checks holds each adapter against the
 * game's code for it (loop.inc). */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{
namespace port
{
namespace loop
{

namespace
{

/* ---- The records ---- */

#pragma pack(push, 1)
/* A screen: its handlers (+0x08 as it opens, +0x0c as it closes, +0x10 every
 * pass of the loop), its name, its first item, the item selected and the one
 * before it. */
struct Screen
{
    reg32 onLoad, unused04, onEnter, onExit, onFrame;
    reg32 name;         // 0x14
    int16_t firstItem;  // 0x18
    int16_t selected;   // 0x1a
    int16_t previous;   // 0x1c
    uint8_t unused1e[0x24 - 0x1e];
};

/* A row of the widgets' registry: its type, its name, the item a new one
 * starts as. */
struct Widget
{
    reg32 type, name, templ;
};

/* An item: its widget's type, its state and flags, its place, its layer, its
 * help and codelink, the box the mouse finds it in, and its handlers. */
struct Item
{
    reg32 type;           // 0x00
    uint16_t state;       // 0x04: the state byte and the flags byte as one word
    int16_t x, y;         // 0x06, 0x08
    int16_t priority;     // 0x0a: its layer, drawn from the highest down
    reg32 help;           // 0x0c
    reg32 codelink;       // 0x10
    int16_t postRaceOnly; // 0x14
    int16_t mainOnly;     // 0x16
    uint8_t unused18[2];
    int16_t top;          // 0x1a
    int16_t left;         // 0x1c
    int16_t width;        // 0x1e
    int16_t height;       // 0x20
    uint8_t unused22[2];
    reg32 onShow;         // 0x24
    reg32 onHide;         // 0x28
    reg32 onDraw;         // 0x2c
    reg32 onKey;          // 0x30
    reg32 onShown;        // 0x34
    reg32 onPass;         // 0x38
    reg32 text;           // 0x3c
    reg32 nextMenu;       // 0x40
    uint8_t unused44[0xbc - 0x44];
};
#pragma pack(pop)
static_assert(sizeof(Screen) == 0x24, "a screen is 0x24 bytes");
static_assert(sizeof(Widget) == 12, "a widget's row is 12 bytes");
static_assert(sizeof(Item) == 0xbc, "an item is 0xbc bytes");

/* The state word's bits (and the flags byte's, above it). */
constexpr uint16_t kUnusable = 0x0001;       // greyed out
constexpr uint16_t kHasKeys = 0x0400;        // the item has the keys to itself: an open list
constexpr uint16_t kInRaceToo = 0x0800;      // shown in the pause screens too
constexpr uint16_t kGreyAndHide = 0x1001;
constexpr uint16_t kNotSelectable = 0x1301;
constexpr uint16_t kNotUnderMouse = 0x1101;
constexpr uint16_t kHiddenFlag = 0x1000;     // the flags byte's kHidden, in the word
constexpr uint16_t kDisabledState = 0x0001;

constexpr int16_t kNoItem = 100;             // selected: none
constexpr reg32 kButton = 5;

/* ---- The game's variables ---- */

// The screens (the first is the one the loop goes back to), their count, the
// items, the widgets' registry.
constexpr Records<Screen> kScreens{0x6040d0};
constexpr Var<int32_t> kScreenCount{0x660bb0};
constexpr Records<Item> kItems{0x604ee0};
constexpr reg32 kWidgetsAt = 0x557a84;

// The loop's state.
constexpr Var<reg32> kCurrentMenu{0x559230};
constexpr Table<reg32> kHistory{0x662800};
constexpr reg32 kHistorySize = 20;
constexpr Var<reg32> kInRace{0x559234};       // the menus are a race's pause screens
constexpr Var<reg32> kAfterRace{0x662968};    // the menus opened on "postgame"
constexpr Var<int16_t> kNetworkGame{0x7a22dc};        // set up to race others over a network
constexpr Var<reg32> kNetworkConnected{0x7a22e0};
constexpr Var<reg32> kLobbyLocked{0x6fd508};  // a network race's lobby: only "racenow"
constexpr Var<reg32> kMouseX{0x55e5e4};
constexpr Var<reg32> kMouseY{0x55e5e8};
constexpr Var<reg32> kMouseBlocked{0x5556d0};  // the network screens have the mouse
constexpr Var<uint8_t> kMouseHeld{0x559250};   // the item under the mouse stays the selected one
// The depth range 2D is drawn in: one for the five lowest layers, one above.
constexpr Var<reg32> kDepthNear{0x563aa8};
constexpr Var<reg32> kDepthFar{0x563aac};
constexpr reg32 kFloatAlmostOne = 0x3f7fff00;
constexpr reg32 kFloatTiny = 0x37800080;

// The help box (0x449aa0).
constexpr Var<reg32> kHelpMenu{0x559248};      // the screen it was for
constexpr Var<reg32> kHelpNeedsMove{0x559238}; // not before the pointer moves on a new screen
constexpr Var<reg32> kHelpLastX{0x55923c};     // where the pointer was
constexpr Var<reg32> kHelpLastY{0x559240};
constexpr Var<reg32> kHelpDue{0x662950};       // the tick it shows at
constexpr Var<reg32> kHelpItem{0x559244};      // the item it is shown for, -1 for none
constexpr Var<reg32> kHelpText{0x55924c};
constexpr Var<reg32> kHelpFade{0x662964};      // 0 or 255
constexpr Var<reg32> kHelpX{0x662958};
constexpr Var<reg32> kHelpY{0x66295c};
constexpr Var<reg32> kHelpWidth{0x662960};
constexpr Var<reg32> kHelpHeight{0x662954};
constexpr Var<reg32> kHelpAlpha{0x559450};     // float, 0.9 with translucency
constexpr reg32 kHelpFont = 0x12;
constexpr reg32 kHelpWrap = 380;
constexpr reg32 kHelpBoxColour = 0xff4b6cad;
constexpr reg32 kHelpTextColour = 0x488cfd;
constexpr reg32 kFloatOne = 0x3f800000;
constexpr reg32 kFloatPoint9 = 0x3f666666;
constexpr Var<reg32> kTicks{0x79c1a4};         // the game's clock
constexpr Var<uint8_t> kDisplayFlags{0x7a3a58};
constexpr uint8_t kNoTranslucency = 0x02;      // no fades, an opaque help box
constexpr Var<reg32> kDrawColour{0x563aa4};    // what 2D is drawn with, alpha on top

// The race settings 0x44be60 keeps consistent (see there), and what it last
// saw of them.
constexpr Var<reg32> kRaceType{0x6fd3b8};
constexpr reg32 kTournament = 1;
constexpr reg32 kKnockout = 2;
constexpr reg32 kHotPursuit = 3;
constexpr Var<reg32> kSplitScreen{0x6fd3b0};   // players on this machine, 2 over a network
constexpr Var<reg32> kSetting4d0{0x6fd4d0};    // 0 to 2, goes with kSetting4d4
constexpr Var<reg32> kSetting4d4{0x6fd4d4};    // 0 to 4
constexpr Var<reg32> kSeen4d0{0x559258};       // -1 before the first pass
constexpr Var<reg32> kSeen4d4{0x55925c};
constexpr Var<reg32> kSeenRaceType{0x559260};
constexpr Var<reg32> kSettingBbe4{0x6fbbe4};
constexpr Var<reg32> kSettingBbd8{0x6fbbd8};   // set when kSettingBbe4 is not
constexpr Var<reg32> kSettingBc00{0x6fbc00};   // 0 or 1, kSetting292c the other way
constexpr Var<reg32> kSetting292c{0x55292c};
constexpr Var<reg32> kSetting504{0x6fd504};    // 0 to 3; 3 not in a two-player series
constexpr Var<uint16_t> kSetting639c{0x55639c};
constexpr Var<reg32> kSetting4e8{0x6fd4e8};
constexpr Var<reg32> kSetting4ac{0x6fd4ac};
constexpr Var<reg32> kValue5d004{0x55d004};
constexpr Var<reg32> kSetting4b0{0x6fd4b0};
constexpr Var<reg32> kSetting4b4{0x6fd4b4};
constexpr Var<reg32> kSetting2ac{0x6fd2ac};
constexpr Var<int16_t> kNetworkSetting2276{0x7a2276};
constexpr Var<reg32> kNetworkSettingE5d0{0x55e5d0};
constexpr Var<reg32> kPlayer1Car{0x6fd2bc};    // the players' cars, as 0x4396b0 reads them
constexpr Var<reg32> kPlayer2Car{0x6fd328};

// The loop's (0x44b230).
constexpr Var<reg32> kLastAction{0x66296c};    // what left the last screen
constexpr Var<reg32> kScreenUp{0x8b7630};      // a screen is being shown
constexpr Var<reg32> kFlagEb10{0x55eb10};
constexpr Var<reg32> kFrontendArt{0x559228};   // mmnu.qfs (or .fsh), loaded once
constexpr Var<reg32> kScreenShapes{0x55922c};  // a screen's own, let go as it closes
constexpr Var<reg32> kAfterMovieScreen{0x559224};  // the screen a movie goes on to, by name
constexpr Var<uint8_t> kNoHelpFlags{0x55eb2e}; // 0x10 or 0x20, no help box
constexpr Var<uint8_t> kQuitting{0x559251};    // Yes was the answer
constexpr Var<reg32> kNoQuitQuestion{0x55e894};
constexpr Var<reg32> kDisplayChoice{0x6fbb4c};
constexpr Var<reg32> kMusicChoice{0x6fbbe8};
constexpr Var<reg32> kSuspended{0x559444};     // the game's window gone to the background
constexpr Var<reg32> kResumed{0x55fec0};
constexpr Var<reg32> kFade{0x559c60};          // a screen's fade out, to 500
constexpr Var<reg32> kFadeIn{0x559c68};
constexpr reg32 kArtFolder = 0x7a2db4;
constexpr Var<reg32> kRestoreDisplay{0x9ef918};  // an import's slot
// The dialogs the loop opens, their lines and buttons.
constexpr reg32 kDialogLines = 0x662970;
constexpr reg32 kDialogButtons = 0x66297c;
constexpr reg32 kQuitQuestion = 0xf4;
constexpr reg32 kQuitYes = 0xc4;
constexpr reg32 kQuitNo = 0xc5;
constexpr reg32 kPlayer1Name = 0x6fd2fc;
constexpr reg32 kPlayer2Name = 0x6fd368;
constexpr reg32 kPlayer1NameTitle = 0xc9;
constexpr reg32 kPlayer2NameTitle = 0xca;
constexpr reg32 kPlayer1Default = 0xfa;        // "Player 1"
constexpr reg32 kPlayer2Default = 0xfb;
constexpr reg32 kNameDialogLine2 = 0xf8;
constexpr reg32 kNameDialogOk = 0xf9;
constexpr reg32 kNameDialogCancel = 0xc6;
constexpr reg32 kPlayerNameLength = 10;
// What leaves a screen (its handlers' results, the loop's actions).
constexpr reg32 kBackToFirst = 4;              // back screen by screen to the first one
constexpr reg32 kGoOn = 5;                     // to the selected button's nextmenu
constexpr reg32 kRace = 6;                     // the menus left for the race
constexpr reg32 kRace7 = 7;
constexpr reg32 kMoviePlayed = 9;              // the attract-mode movie has played over the screen
constexpr reg32 kQuit = 10;
constexpr reg32 kOut = 11;                     // Back from the first screen
// The dialogs' answers, and what a dialog's callback is told.
constexpr reg32 kDialogEnter = 1;              // Enter, or a click
constexpr reg32 kDialogEscape = 3;
constexpr Var<reg32> kInputFromMouse{0x664658};

// Keys, as 0x451960 gives them: ASCII, or a scan code in the high byte.
constexpr uint16_t kKeyEnter = 0x0d;
constexpr uint16_t kKeyEscape = 0x1b;
constexpr uint16_t kKeyUp = 0x4800;
constexpr uint16_t kKeyLeft = 0x4b00;
constexpr uint16_t kKeyRight = 0x4d00;
constexpr uint16_t kKeyDown = 0x5000;

// What a key handler or a screen's handler returns.
constexpr reg32 kHandled = 2;                  // taken, and nothing more: 0 to the loop
constexpr reg32 kKeyEscapeButton = 0x0d;       // what the Escape button is handed

// Sounds, 0x4181d0(sound, volume).
constexpr reg32 kSoundMove = 0;
constexpr reg32 kSoundBack = 3;
constexpr reg32 kSoundEnter = 4;
constexpr reg32 kSoundLeave = 5;
constexpr reg32 kSoundStart = 7;
constexpr reg32 kVolumeFull = 0x7f;
constexpr reg32 kVolumeTick = 0x2d;

// The game's strings.
constexpr reg32 kRaceNow = 0x538fb4, kEscape = 0x538fbc, kIpx = 0x538fc4, kPostGame = 0x539048;
constexpr reg32 kMmnu = 0x538fc8, kQfsFormat = 0x538fd0, kFshFormat = 0x538fdc;
constexpr reg32 kOpponent = 0x538fe8, kCar1 = 0x538ff4, kCar2 = 0x538ffc;
constexpr reg32 kShowcaseScreen = 0x539084, kInteriorScreen = 0x539090, kHistoryScreen = 0x53909c;
constexpr reg32 kCompareScreen = 0x5390a4;

/* ---- What they call ---- */

// The port's own that the game's code calls with a stand-in of its own beside
// (so a call into it is made as C++, on a world it can run on).
constexpr guest::Native kLanguageText = text::languageText;    // 0x4d1850
// What is not rewritten, or is held on its own: by address.
constexpr reg32 kCodelinkMatches = 0x4429c0;
constexpr reg32 kTextBox = 0x451ff0;           // a text's size, wrapped: (text, font, wrap, &w, &h, 0)
constexpr reg32 kDrawTextWrapped = 0x451fc0;
constexpr reg32 kDrawTextInRace = 0x452190;
constexpr reg32 kFillBox = 0x4d8550;
constexpr reg32 kAllowOption = 0x43a6e0;       // (option bits, allowed)
constexpr reg32 kCarFlag10 = 0x4396b0;         // a car's flag 0x10
constexpr reg32 kMap4cf160 = 0x4cf160;         // 2 -> 0, 4 -> 1, else 2
constexpr reg32 kMap4cf190 = 0x4cf190;         // 0 -> 2, 1 -> 4, else 8
constexpr reg32 kMarkOption8 = 0x442b60;
constexpr reg32 kLoadMenus = 0x449510;
constexpr reg32 kCheckMenuFiles = 0x449960;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kFree = 0x4e1890;
constexpr reg32 kSetDisplay = 0x4d1750;
constexpr reg32 kResetInput = 0x451b20;
constexpr reg32 kStartMusic = 0x4104f0;        // (track, music choice, after a race)
constexpr reg32 kStopMusic = 0x4109d0;
constexpr reg32 kPollEvents = 0x4c9860;
constexpr reg32 kViewLastReplay = 0x44e710;  // the results' View Replay: 7 when it loaded
constexpr reg32 kReadKey = 0x451960;           // the next key, 0 for none
constexpr reg32 kBeginFrame = 0x4d9120;
constexpr reg32 kPresentFrame = 0x4dab90;
constexpr reg32 kEndFrame = 0x4d9140;          // and waits for the next one
constexpr reg32 kItemChanged = 0x460760;
constexpr reg32 kScreenLeft = 0x4534d0;        // (its name)
constexpr reg32 kMenusEnd = 0x452110;
constexpr reg32 kNetworkLeft = 0x440d20;
constexpr reg32 kDialogPass = 0x445f50;        // a dialog's pass, its result the loop's
constexpr reg32 kDialogKey = 0x445e60;
constexpr reg32 kCloseDialog = 0x445f10;
constexpr reg32 kDialogShown = 0x4438b0;
constexpr reg32 kNameValid = 0x4721f0;
constexpr reg32 kResetDisplay = 0x4daf80;
constexpr reg32 kLoadPointer = 0x49c140;
constexpr reg32 kLoadShapes = 0x4956f0;        // a .qfs or .fsh file, 0 when it is not there
constexpr reg32 kLoadDialogArt = 0x445de0;
constexpr reg32 kClearBackground = 0x465d60;
constexpr reg32 kSetBackground = 0x465d80;
constexpr reg32 kLoadScreenArt = 0x4a4ba0;
constexpr reg32 kFrameTime = 0x4f0650;
constexpr reg32 kSleep = 0x4f0710;
constexpr reg32 kResumeSound = 0x4bef50;
constexpr reg32 kBoxBorder = 0x4d8930;
constexpr reg32 kStrcmp = 0x4ee310;
constexpr reg32 kSetMousePosition = 0x49bfc0;
constexpr reg32 kNetworkFrame = 0x441040;      // the network screens' pass; nonzero leaves
constexpr reg32 kNetworkBusy = 0x441680;
constexpr reg32 kNetworkKey = 0x441700;
constexpr reg32 kNetworkShown = 0x440970;
constexpr reg32 kHotKey = 0x4524e0;            // the keys that work on any screen
constexpr reg32 kSetCursor = 0x49c0e0;
constexpr reg32 kPlaySound = 0x4181d0;
constexpr reg32 kCurrentMenuName = 0x44ae00;
constexpr reg32 kShowItems = 0x44a960;
constexpr reg32 kOpenDialog = 0x444c20;
constexpr reg32 kOpenQuestion = 0x4446d0;
constexpr reg32 kDialogEditText = 0x444c10;
constexpr reg32 kDialogFocus = 0x4446c0;
constexpr reg32 kQuitAnswerAt = 0x44b0f0;
constexpr reg32 kPlayer1NameAnswerAt = 0x44af90;
constexpr reg32 kPlayer2NameAnswerAt = 0x44b040;

/* ---- Small things ---- */

reg32 word(App* app, reg32 at)
{
    return app->getMemory<reg32>(at);
}

void setWord(App* app, reg32 at, reg32 value)
{
    app->getMemory<reg32>(at) = value;
}

uint8_t byteAt(App* app, reg32 at)
{
    return app->getMemory<uint8_t>(at);
}

void setByte(App* app, reg32 at, uint8_t value)
{
    app->getMemory<uint8_t>(at) = value;
}

void copyString(App* app, reg32 to, reg32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = byteAt(app, from);
        setByte(app, to, c);
        if (c == 0)
            break;
    }
}

/* A rewrite called from here as the game's code calls it: its entry esp is
 * the one under its return address. */
constexpr reg32 entered(reg32 at)
{
    return at - 4;
}

/* The item `index` of the table, its address. */
reg32 itemAt(int32_t index)
{
    return kItems.address(index);
}

Screen& screen(App* app, reg32 at)
{
    return record<Screen>(app, at);
}

Item& item(App* app, reg32 at)
{
    return record<Item>(app, at);
}

/* A screen's first item, and the item `offset` from it. */
int32_t firstItem(App* app, reg32 menu)
{
    return screen(app, menu).firstItem;
}

reg32 menuItem(App* app, reg32 menu, int32_t offset)
{
    return itemAt(firstItem(app, menu) + offset);
}

bool isEnd(App* app, reg32 at)
{
    return item(app, at).type == 0;
}

/* The game's code being run: the calls it makes, and what eax is as the
 * game's code would have it at this point (the last call's result, or what
 * was handed in). */
struct Game
{
    App* app;
    CPU& cpu;
    reg32 eax;
    bool call(reg32 esp, reg32 target, Args a = {}, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, a, stack, eax);
    }
    bool call(reg32 esp, guest::Native target, Args a = {}, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, a, stack, eax);
    }
};

/* A call into the game's code that gives up the function when the game
 * stopped in it. */
#define GAME(...)          \
    do                     \
    {                      \
        if (!g.call(__VA_ARGS__)) \
            return {};     \
    } while (0)

/* A rewrite's result, or give up the function when the game stopped in it. */
#define NEEDED(variable, expression) \
    const Result variable = (expression); \
    if (!variable)                   \
        return {}

bool playSound(Game& g, reg32 esp, reg32 sound, reg32 volume)
{
    return g.call(esp, kPlaySound, {sound, volume});
}

/* ---- The selection ---- */

/* 0x4499b0's: whether an item can be used: in a network race's locked lobby,
 * only the one whose codelink names "racenow". */
Result itemUsableCore(Game& g, reg32 esp, reg32 at)
{
    App* app = g.app;
    if (kNetworkConnected(app) == 0 || kLobbyLocked(app) == 0 || kInRace(app) != 0)
        return 1;
    GAME(esp - 8, kCodelinkMatches, {item(app, at).codelink, kRaceNow});
    return g.eax != 0 ? 1 : 0;
}

/* An item's state word as selection reads it: greyed out too when it is not
 * usable now (0x4499b0). */
Result selectionState(Game& g, reg32 esp, reg32 at)
{
    uint16_t state = item(g.app, at).state;
    g.eax = at;
    NEEDED(usable, itemUsableCore(g, entered(esp), at));
    g.eax = reg32(*usable);
    if (*usable == 0)
        state |= kUnusable;
    return state;
}

/* 0x4499f0's: whether a key may act on a screen: always, but in a locked
 * lobby while the network is idle only on a usable item, and never Escape. */
Result keyAllowedCore(Game& g, reg32 esp, reg32 menu, uint16_t key)
{
    App* app = g.app;
    if (kNetworkConnected(app) == 0 || kLobbyLocked(app) == 0 || kInRace(app) != 0)
        return 1;
    const reg32 at = esp - 8;
    GAME(at, kNetworkBusy);
    const reg32 busy = g.eax;
    if (busy != 0)
        return 1;
    const Screen& s = screen(app, menu);
    if (s.selected == kNoItem || key == kKeyEscape)
        return int32_t(busy);
    g.eax = menuItem(app, menu, s.selected);
    NEEDED(usable, itemUsableCore(g, entered(at), g.eax));
    g.eax = reg32(*usable);
    return *usable != 0 ? 1 : 0;
}

/* 0x449e50's: whether the item `offset` of a screen can be selected. */
Result selectableCore(Game& g, reg32 esp, int32_t offset, reg32 menu)
{
    if (offset < 0 || offset == kNoItem)
        return 0;
    NEEDED(state, selectionState(g, esp - 4, menuItem(g.app, menu, offset)));
    return (*state & kNotSelectable) == 0 ? 1 : 0;
}

/* 0x449ea0's: the mouse pointer moved onto the selected item, three quarters
 * along it and halfway down. */
Result pointAtSelectedCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const Screen& s = screen(app, menu);
    if (s.selected == kNoItem)
        return int32_t(g.eax);
    const Item& it = item(app, menuItem(app, menu, s.selected));
    const int32_t x = it.left + 3 * it.width / 4;
    kMouseX.set(app, reg32(x));
    const int32_t y = it.top + it.height / 2;
    kMouseY.set(app, reg32(y));
    GAME(esp - 12, kSetMousePosition, {reg32(x), reg32(y)});
    return int32_t(g.eax);
}

/* What selecting another item ends with: the one left kept as the previous
 * when it could be selected, a tick when the selection moved -- in that order
 * going forward, the other way round going back -- and the pointer on the
 * new one. */
Result afterSelecting(Game& g, reg32 at, reg32 menu, int16_t was, bool forward)
{
    App* app = g.app;
    auto keepPrevious = [&]() -> bool {
        NEEDED(ok, selectableCore(g, entered(at), int32_t(was), menu));
        g.eax = reg32(*ok);
        if (*ok != 0)
            screen(app, menu).previous = was;
        return true;
    };
    if (forward && !keepPrevious())
        return {};
    if (screen(app, menu).selected != was)
        GAME(at, kPlaySound, {kSoundMove, kVolumeFull});
    if (!forward && !keepPrevious())
        return {};
    g.eax = menu;
    return pointAtSelectedCore(g, entered(at), menu);
}

/* The previous selection back when the one selected cannot be: whether it was. */
Result backToPrevious(Game& g, reg32 at, reg32 menu, int16_t was)
{
    App* app = g.app;
    if (was != kNoItem)
    {
        NEEDED(ok, selectableCore(g, entered(at), int32_t(was), menu));
        g.eax = reg32(*ok);
        if (*ok != 0)
            return 0;
    }
    const int16_t previous = screen(app, menu).previous;
    NEEDED(ok, selectableCore(g, entered(at), int32_t(previous), menu));
    g.eax = reg32(*ok);
    if (*ok == 0)
        return 0;
    screen(app, menu).selected = screen(app, menu).previous;
    return 1;
}

/* 0x449f30's: the item before the selected one that can be selected, round
 * from the first to the last; with none selected, the last of them.  Not
 * while the selected item has the keys. */
Result selectPreviousCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    const int16_t was = screen(app, menu).selected;
    if (was != kNoItem && (item(app, menuItem(app, menu, was)).state & kHasKeys) != 0)
        return int32_t(reg32(firstItem(app, menu) + was) * 47);  // the selected item's offset in the table, a quarter of it
    NEEDED(back, backToPrevious(g, at, menu, was));
    if (!*back)
    {
        if (screen(app, menu).selected == kNoItem)
        {
            for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
            {
                NEEDED(state, selectionState(g, at, itemAt(index)));
                g.eax = reg32(*state);
                if ((*state & kNotSelectable) == 0)
                    screen(app, menu).selected = int16_t(index - firstItem(app, menu));
            }
        }
        else
        {
            for (;;)
            {
                Screen& s = screen(app, menu);
                if (s.selected != 0)
                    s.selected = int16_t(s.selected - 1);
                else
                    while (!isEnd(app, menuItem(app, menu, s.selected + 1)))
                        s.selected = int16_t(s.selected + 1);
                NEEDED(state, selectionState(g, at, menuItem(app, menu, s.selected)));
                g.eax = reg32(*state);
                if ((*state & kNotSelectable) == 0)
                    break;
            }
        }
    }
    return afterSelecting(g, at, menu, was, false);
}

/* 0x44a0c0's: the item after the selected one that can be selected, round
 * from the last to the first; with none selected, the first of them. */
Result selectNextCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    const int16_t was = screen(app, menu).selected;
    if (was != kNoItem && (item(app, menuItem(app, menu, was)).state & kHasKeys) != 0)
        return int32_t(reg32(firstItem(app, menu) + was) * 47);  // the selected item's offset in the table, a quarter of it
    NEEDED(back, backToPrevious(g, at, menu, was));
    if (!*back)
    {
        if (screen(app, menu).selected == kNoItem)
        {
            for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
            {
                NEEDED(state, selectionState(g, at, itemAt(index)));
                g.eax = reg32(*state);
                if ((*state & kNotSelectable) == 0)
                {
                    screen(app, menu).selected = int16_t(index - firstItem(app, menu));
                    break;
                }
            }
        }
        else
        {
            for (;;)
            {
                Screen& s = screen(app, menu);
                s.selected = int16_t(s.selected + 1);
                if (isEnd(app, menuItem(app, menu, s.selected)))
                    s.selected = 0;
                NEEDED(state, selectionState(g, at, menuItem(app, menu, s.selected)));
                g.eax = reg32(*state);
                if ((*state & kNotSelectable) == 0)
                    break;
            }
        }
    }
    return afterSelecting(g, at, menu, was, true);
}

/* 0x44a240's: a screen's Escape button: the button whose codelink names
 * "escape", the first of them, if it is not greyed out; 0 for none.  Its
 * index from the first item in *offset (0 for none). */
Result escapeButtonCore(Game& g, reg32 esp, reg32 menu, reg32 offset)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    reg32 found = 0;
    setWord(app, offset, 0);
    if (menu == 0)
        return 0;
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        const reg32 it = itemAt(index);
        const reg32 codelink = item(app, it).codelink;
        if (item(app, it).type != kButton || codelink == 0 || found != 0)
            continue;
        GAME(at, kCodelinkMatches, {codelink, kEscape});
        if (g.eax != 0 && (item(app, it).state & kDisabledState) == 0)
        {
            found = it;
            setWord(app, offset, reg32(index - firstItem(app, menu)));
        }
    }
    return int32_t(found);
}

/* 0x44a440's: the selected item of the screen shown, by its index in the item
 * table; -1 for none. */
Result selectedIndexCore(Game& g)
{
    App* app = g.app;
    const reg32 menu = kCurrentMenu(app);
    if (screen(app, menu).selected == kNoItem)
        return -1;
    return firstItem(app, menu) + screen(app, menu).selected;
}

/* 0x44a470's: `it` selected on the screen shown, if it is one of its items:
 * whether it is.  The selection left is kept as the previous when it could be
 * selected. */
Result selectItemCore(Game& g, reg32 esp, reg32 it)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    const reg32 menu = kCurrentMenu(app);
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        if (itemAt(index) != it)
            continue;
        const reg32 current = kCurrentMenu(app);
        NEEDED(ok, selectableCore(g, entered(at), int32_t(screen(app, current).selected), current));
        g.eax = reg32(*ok);
        if (*ok != 0)
            screen(app, current).previous = screen(app, current).selected;
        const reg32 shown = kCurrentMenu(app);
        screen(app, shown).selected = int16_t(index - firstItem(app, shown));
        return 1;
    }
    return 0;
}

/* 0x44a660's: whether an item is shown: in the pause screens, only one marked
 * for them. */
bool shownNow(App* app, reg32 it)
{
    return !(kInRace(app) != 0 && (item(app, it).state & kInRaceToo) == 0);
}

/* 0x44a690's: the mouse let go of the item it held.  The flag is cleared
 * through ah, so eax's second byte goes with it. */
Result releaseMouseCore(Game& g)
{
    kMouseHeld.set(g.app, 0);
    return int32_t(g.eax & ~reg32(0xff00));
}

/* 0x44a6b0's: the item under the mouse, by its index from the screen's first,
 * -1 for none: the lowest layer first, the first item of it the pointer is
 * inside of.  While a dialog is up, the mouse holds an item, or the selected
 * item has the keys, the selected one; none while the network screens have
 * the mouse.  Moving onto another item ticks. */
Result itemUnderMouseCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    constexpr int32_t kLastLayer = 9999;
    const reg32 at = esp - 40;
    if (kMouseBlocked(app) != 0)
        return -1;
    GAME(at, kDialogShown);
    if ((g.eax & 0xff) != 0 || kMouseHeld(app) != 0
        || (screen(app, menu).selected != kNoItem
            && (item(app, menuItem(app, menu, screen(app, menu).selected)).state & kHasKeys) != 0))
        return screen(app, menu).selected;
    for (int32_t layer = 0; layer < kLastLayer;)
    {
        int32_t next = kLastLayer;
        for (int16_t index = int16_t(firstItem(app, menu)); !isEnd(app, itemAt(index)); ++index)
        {
            const reg32 it = itemAt(index);
            NEEDED(state, selectionState(g, at, it));
            g.eax = reg32(*state);
            if ((*state & kNotUnderMouse) != 0)
                continue;
            const int32_t priority = item(app, it).priority;
            if (priority != layer)
            {
                if (priority < next && priority > layer)
                    next = priority;
                continue;
            }
            const int32_t x = int32_t(kMouseX(app)), y = int32_t(kMouseY(app));
            const int32_t top = item(app, it).top, left = item(app, it).left;
            if (y <= top || y >= top + item(app, it).height || x <= left || left + item(app, it).width <= x)
                continue;
            const int32_t offset = index - firstItem(app, menu);
            if (screen(app, menu).selected != offset && (*state & kGreyAndHide) == 0)
                GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
            return offset;
        }
        layer = next;
    }
    return -1;
}

/* ---- The items ---- */

/* 0x44a860's: every item's kOnShown handler, the screen being up; then the
 * mouse lets go. */
Result itemsShownCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 12;
    int16_t index = int16_t(firstItem(app, menu));
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const reg32 it = itemAt(index);
        if (item(app, it).onShown != 0)
            GAME(at, item(app, it).onShown, {it, it});
    }
    g.eax = reg32(int32_t(index)) * sizeof(Item);  // the end's offset in the table
    return releaseMouseCore(g);
}

/* 0x44a8b0's: a pass of the items: each one's kOnPass handler -- 3 or 4
 * from one ends it -- then the selected one's again; the result the pointer's
 * shape (0x49c0e0), 12 when a key would not be taken now. */
Result passItemsCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    reg32 result = 0;
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        const reg32 it = itemAt(index);
        if (item(app, it).onPass == 0)
            continue;
        GAME(at, item(app, it).onPass, {it, reg32(index)});
        result = g.eax;
        if (result == kBackToFirst || result == 3)
        {
            GAME(at, kSetCursor, {result});
            return int32_t(g.eax);
        }
        result = 0;
    }
    if (screen(app, menu).selected != kNoItem)
    {
        const reg32 it = menuItem(app, menu, screen(app, menu).selected);
        if (item(app, it).onPass != 0)
        {
            GAME(at, item(app, it).onPass, {it, it});
            result = g.eax;
        }
    }
    NEEDED(allowed, keyAllowedCore(g, entered(at), menu, 0));
    g.eax = reg32(*allowed);
    if (*allowed == 0)
        result = 12;
    GAME(at, kSetCursor, {result});
    return int32_t(g.eax);
}

/* 0x44a960's: every item's kOnShow handler, the screen opening; an item for
 * after a race only, or for before one only, greyed out and hidden when it
 * is not the time for it.  The network screens get theirs.  (Native whole in
 * native_frontend_loop.cpp; called by address.) */

/* 0x44a9e0's: every item's kOnHide handler, the screen closing. */
Result hideItemsCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 12;
    int16_t index = int16_t(firstItem(app, menu));
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const reg32 it = itemAt(index);
        if (item(app, it).onHide != 0)
            GAME(at, item(app, it).onHide, {it, it});
    }
    return int32_t(reg32(int32_t(index)) * sizeof(Item));  // the end's offset in the table
}

/* 0x44aad0's: every item's state and flags back to its widget's template's,
 * the screen opening. */
Result resetItemsCore(Game& g, reg32 menu)
{
    App* app = g.app;
    int32_t index = firstItem(app, menu);
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const reg32 it = itemAt(index);
        for (reg32 widget = kWidgetsAt; word(app, widget) != 0; widget += sizeof(Widget))
        {
            if (word(app, widget) != item(app, it).type)
                continue;
            const reg32 from = record<Widget>(app, widget).templ;
            if (from != 0)
                item(app, it).state = item(app, from).state;
            break;
        }
    }
    return int32_t(reg32(index) * sizeof(Item));  // the end's offset in the table
}

/* ---- The screens ---- */

/* 0x44ae60's: a screen by its name, 0 for none loaded. */
Result menuByNameCore(Game& g, reg32 esp, reg32 name)
{
    App* app = g.app;
    const reg32 at = esp - 20;
    for (reg32 index = 0; index < reg32(kScreenCount(app)); ++index)
    {
        GAME(at, kStrcmp, {name, screen(app, kScreens.address(int32_t(index))).name});
        if (g.eax == 0)
            return int32_t(kScreens.address(int32_t(index)));
    }
    return 0;
}

/* 0x44aec0's: back to the screen gone from last: off the history (its last
 * place kept as it was), the first screen when there is none. */
Result popMenuCore(Game& g)
{
    App* app = g.app;
    const reg32 back = kHistory(app, 0);
    for (reg32 i = 0; i + 1 < kHistorySize; ++i)
        kHistory.set(app, int32_t(i), kHistory(app, int32_t(i + 1)));
    kCurrentMenu.set(app, back != 0 ? back : kScreens.at);
    return int32_t(kHistorySize - 1);
}

/* 0x44af10's: the screen shown onto the history, before going to another. */
Result pushMenuCore(Game& g)
{
    App* app = g.app;
    for (reg32 i = kHistorySize - 1; i > 0; --i)
        kHistory.set(app, int32_t(i), kHistory(app, int32_t(i - 1)));
    const reg32 menu = kCurrentMenu(app);
    kHistory.set(app, 0, menu);
    return int32_t(menu);
}

/* 0x44af50's: the history set from a list of screen names, ended by a null
 * (the screens to come back through after a race), then the first of them
 * shown. */
Result setHistoryCore(Game& g, reg32 esp, reg32 names)
{
    App* app = g.app;
    const reg32 at = esp - 20;
    for (reg32 i = 0; word(app, names + 4 * i) != 0; ++i)
    {
        g.eax = word(app, names + 4 * i);
        NEEDED(menu, menuByNameCore(g, entered(at), g.eax));
        g.eax = reg32(*menu);
        if (*menu != 0)
            kHistory.set(app, int32_t(i), reg32(*menu));
    }
    return popMenuCore(g);
}

/* 0x449df0's: whether the keys go to something other than the screen: an
 * open list (an item of the list widgets that has the keys) or a dialog. */
Result keysTakenCore(Game& g, reg32 esp)
{
    App* app = g.app;
    NEEDED(index, selectedIndexCore(g));
    g.eax = reg32(*index);
    if (*index != -1)
    {
        const Item& it = item(app, itemAt(*index));
        if ((it.state & kHasKeys) != 0)
        {
            const reg32 type = it.type;
            if (type == 0x11 || type == 0x1d || type == 0x12 || type == 0x13)
                return 1;
        }
    }
    GAME(esp - 8, kDialogShown);
    return (g.eax & 0xff) != 0 ? 1 : 0;
}

/* 0x44a2d0's: a key on a screen.  In a network race the network screens see
 * it first; then the keys that work anywhere; then the selected item's key
 * handler.  Whatever none of them took: Escape presses the Escape button
 * (1, the screen left, when there is none), the arrows move the selection.
 * The result is the loop's (kHandled reads as 0). */
Result handleKeyCore(Game& g, reg32 esp, reg32 menu, reg32 key)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    const uint16_t code = uint16_t(key);
    const reg32 signedCode = reg32(int32_t(int16_t(code)));
    reg32 result = 0;
    if (kNetworkConnected(app) != 0)
    {
        GAME(at, kNetworkKey, {signedCode, signedCode});
        result = g.eax;
        if (result != 0)
            return int32_t(result == kHandled ? 0 : result);
        // (The allowed check's key is the edx the network's call was handed.)
        NEEDED(allowed, keyAllowedCore(g, entered(at), menu, code));
        g.eax = reg32(*allowed);
        if (*allowed == 0)
            return 0;
    }
    if (kAfterRace(app) == 0)
        GAME(at, kHotKey, {signedCode});
    if (screen(app, menu).selected == kNoItem)
    {
        if (code == kKeyEnter)
            return 0;
    }
    else
    {
        const reg32 it = menuItem(app, menu, screen(app, menu).selected);
        if (item(app, it).onKey != 0)
        {
            GAME(at, item(app, it).onKey, {it, menu, signedCode});
            result = g.eax;
        }
    }
    if (result == 0)
    {
        switch (code)
        {
        case kKeyEscape:
        {
            GAME(at, kPlaySound, {kSoundBack, kVolumeFull});
            const reg32 offset = at;  // [ebp - 4]
            setWord(app, offset, result);
            NEEDED(button, escapeButtonCore(g, entered(at), menu, offset));
            g.eax = reg32(*button);
            if (*button != 0)
            {
                screen(app, menu).selected = int16_t(word(app, offset));
                screen(app, menu).previous = int16_t(word(app, offset));
                GAME(at, item(app, reg32(*button)).onKey, {reg32(*button), menu, kKeyEscapeButton});
                result = g.eax;
            }
            else
                result = 1;
            break;
        }
        case kKeyUp:
        case kKeyLeft:
        {
            GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
            g.eax = menu;
            NEEDED(r, selectPreviousCore(g, entered(at), menu));
            g.eax = reg32(*r);
            break;
        }
        case kKeyRight:
        case kKeyDown:
        {
            GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
            g.eax = menu;
            NEEDED(r, selectNextCore(g, entered(at), menu));
            g.eax = reg32(*r);
            break;
        }
        default:
            break;
        }
    }
    return int32_t(result == kHandled ? 0 : result);
}

/* 0x44a4f0's: the screen's items drawn, layer by layer from the highest
 * priority down to 0, each told whether it is the selected one; hidden ones
 * not.  Layers 0 to 4 go in front.  Over a network race, the network screens
 * get each layer's pass (the lobby only its eighth); what they or an item's
 * drawing return, the last nonzero, is the result. */
Result drawItemsCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 36;
    int32_t layer = 0;
    for (int16_t index = int16_t(firstItem(app, menu)); !isEnd(app, itemAt(index)); ++index)
    {
        const int32_t priority = item(app, itemAt(index)).priority;
        if (priority > layer)
            layer = priority;
    }
    reg32 result = 0;
    for (; layer >= 0; --layer)
    {
        if (layer <= 4)
        {
            kDepthFar.set(app, kFloatAlmostOne);
            kDepthNear.set(app, 0);
        }
        else
        {
            kDepthFar.set(app, kFloatTiny);
            kDepthNear.set(app, kFloatAlmostOne);
        }
        for (int16_t index = int16_t(firstItem(app, menu)); !isEnd(app, itemAt(index)); ++index)
        {
            const reg32 it = itemAt(index);
            if (item(app, it).priority != layer)
                continue;
            const reg32 isSelected = index == firstItem(app, menu) + screen(app, menu).selected ? 1 : 0;
            if (item(app, it).onDraw != 0 && (item(app, it).state & kHiddenFlag) == 0)
            {
                GAME(at, item(app, it).onDraw, {it, isSelected});
                if (g.eax != 0)
                    result = g.eax;
            }
        }
        if (kInRace(app) != 0)
        {
            if (kNetworkConnected(app) != 0)
            {
                GAME(at, kNetworkFrame);
                if (g.eax != 0)
                    result = g.eax;
            }
        }
        else if (layer == 8 && kNetworkConnected(app) != 0)
        {
            GAME(at, kStrcmp, {screen(app, menu).name, kIpx});
            if (g.eax != 0)
            {
                GAME(at, kNetworkFrame);
                if (g.eax != 0)
                    result = g.eax;
            }
        }
    }
    kDepthFar.set(app, kFloatTiny);
    kDepthNear.set(app, kFloatAlmostOne);
    return int32_t(result);
}

/* 0x44aa30's: a screen left: its onExit handler (told how, `action`), then,
 * unless a movie played (9) or the pause screens are up, the leaving sound
 * and the items faded out -- drawn again as the fade (0x559c60) runs to 500,
 * twice as fast without fades -- then its items' kOnHide. */
Result leaveScreenCore(Game& g, reg32 esp, reg32 menu, reg32 action)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    const reg32 step = (kDisplayFlags(app) & kNoTranslucency) != 0 ? 50 : 25;
    if (screen(app, menu).onExit != 0)
        GAME(at, screen(app, menu).onExit, {menu, action, step, menu});
    if (kInRace(app) == 0)
    {
        if (action != kMoviePlayed)
        {
            GAME(at, kPlaySound, {kSoundLeave, kVolumeFull});
            if ((kDisplayFlags(app) & kNoTranslucency) == 0)
            {
                for (kFade.set(app, step); int32_t(kFade(app)) < 500; kFade.set(app, kFade(app) + step))
                {
                    GAME(at, kBeginFrame);
                    g.eax = menu;
                    NEEDED(drawn, drawItemsCore(g, entered(at), menu));
                    g.eax = reg32(*drawn);
                    GAME(at, kEndFrame);
                }
            }
            kFade.set(app, 0);
        }
        kFadeIn.set(app, 0);
    }
    g.eax = menu;
    return hideItemsCore(g, entered(at), menu);
}

/* 0x44ab40's: a screen opened.  Outside a race: the pointer reset, the
 * frontend's art loaded once (mmnu.qfs, else mmnu.fsh, from the art folder),
 * the car screens' backgrounds.  Then no item selected, the items' states
 * from their widgets', the onEnter handler -- its result is this one's --
 * and the items' kOnShow and kOnShown. */
Result enterScreenCore(Game& g, reg32 esp, reg32 menu)
{
    App* app = g.app;
    const reg32 at = esp - 20 - 0x100;
    if (kInRace(app) == 0)
    {
        GAME(at, kSetCursor, {0});
        // eax holds the art's name here, and edx 0, as the original has them.
        GAME(at, kResetDisplay, {kMmnu});
        GAME(at, kLoadPointer);
        if (kFrontendArt(app) == 0)
        {
            const reg32 path = at;  // char[0x100]
            GAME(at, kSprintf, {}, {path, kQfsFormat, kArtFolder, kMmnu});
            GAME(at, kLoadShapes, {path, 0});
            kFrontendArt.set(app, g.eax);
            if (kFrontendArt(app) == 0)
            {
                GAME(at, kSprintf, {}, {path, kFshFormat, kArtFolder, kMmnu});
                GAME(at, kLoadShapes, {path, 0});
                kFrontendArt.set(app, g.eax);
            }
        }
        GAME(at, kLoadDialogArt);
        GAME(at, kClearBackground);
        GAME(at, kStrcmp, {screen(app, menu).name, kOpponent});
        bool carScreen = g.eax == 0;
        if (!carScreen)
        {
            GAME(at, kStrcmp, {screen(app, menu).name, kCar1});
            carScreen = g.eax == 0;
        }
        if (!carScreen)
        {
            GAME(at, kStrcmp, {screen(app, menu).name, kCar2});
            carScreen = g.eax == 0;
        }
        if (carScreen)
            GAME(at, kSetBackground, {kFrontendArt(app)});
        GAME(at, kLoadScreenArt);
    }
    screen(app, menu).selected = kNoItem;
    screen(app, menu).previous = screen(app, menu).selected;
    kScreenShapes.set(app, 0);
    g.eax = menu;
    NEEDED(reset, resetItemsCore(g, menu));
    g.eax = reg32(*reset);
    reg32 result = 0;
    if (screen(app, menu).onEnter != 0)
    {
        GAME(at, screen(app, menu).onEnter, {menu, 0});
        result = g.eax;
    }
    GAME(at, kShowItems, {menu});
    g.eax = menu;
    NEEDED(shown, itemsShownCore(g, entered(at), menu));
    g.eax = reg32(*shown);
    GAME(at, kFrameTime);
    return int32_t(result);
}

/* 0x44b1a0's: the game suspended (0x559444, its window gone to the
 * background): waited out, then the screen closed as on leaving it, *action
 * 13 (back to the first screen).  Whether it was. */
Result waitWhileSuspendedCore(Game& g, reg32 esp, reg32 menu, reg32 action)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    if (kSuspended(app) == 0)
        return 0;
    setWord(app, action, 13);
    do
    {
        GAME(at, kSleep, {0});
        GAME(at, kPollEvents);
    } while (kSuspended(app) != 0);
    GAME(at, kRestoreDisplay(app));
    kResumed.set(app, 0);
    GAME(at, kResumeSound, {1});
    if (screen(app, menu).onExit != 0)
        GAME(at, screen(app, menu).onExit, {menu, action, menu});
    g.eax = menu;
    NEEDED(hidden, hideItemsCore(g, entered(at), menu));
    g.eax = reg32(*hidden);
    return 1;
}

/* 0x44af90's, 0x44b040's: the answers of the name dialogs the loop opens
 * when a race is started with no name for player 1 (or, split screen, player
 * 2): Enter (or OK) keeps what was typed -- "Player 1"/"Player 2" if nothing
 * -- and starts the race (5); Cancel or Escape clears it (2). */
Result nameAnswerCore(Game& g, reg32 esp, reg32 answer, reg32 close, reg32 name, reg32 defaultText)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    setByte(app, close, 0);
    reg32 result = 0;
    if (answer == kDialogEnter)
    {
        GAME(at, kDialogFocus);
        const reg32 focus = g.eax;
        if (focus == 0 || kInputFromMouse(app) == 0)
        {
            GAME(at, kDialogEditText);
            reg32 typed = g.eax;
            GAME(at, kNameValid, {typed});
            if (g.eax == 0)
            {
                GAME(at, kLanguageText, {defaultText});
                typed = g.eax;
            }
            copyString(app, name, typed);
            result = kGoOn;
            setByte(app, close, 1);
        }
        else if (focus == 1)
        {
            setByte(app, name, 0);
            result = kHandled;
            setByte(app, close, 1);
        }
    }
    else if (answer == kDialogEscape)
    {
        setByte(app, name, 0);
        result = kHandled;
        setByte(app, close, 1);
    }
    return int32_t(result);
}

/* 0x44b0f0's: the answer to "Quit the game?": Yes ends the menus (10). */
Result quitAnswerCore(Game& g, reg32 esp, reg32 answer, reg32 close)
{
    App* app = g.app;
    const reg32 at = esp - 8;
    if (answer < kDialogEnter)
        return 0;
    if (answer == kDialogEnter)
    {
        GAME(at, kDialogFocus);
        const reg32 focus = g.eax;
        if (focus == 0)
        {
            if (close != 0)
                setByte(app, close, 1);
            kQuitting.set(app, 1);
            return 10;
        }
        if (focus == 1)
        {
            if (close != 0)
                setByte(app, close, 1);
            kQuitting.set(app, 0);
            return kHandled;
        }
        if (close != 0)
            setByte(app, close, 0);
        return kHandled;
    }
    if (answer == kDialogEscape)
    {
        if (close != 0)
            setByte(app, close, 1);
        return kHandled;
    }
    return 0;
}

/* 0x449aa0's: the help box: an item's help text, shown once the pointer has
 * rested on the selected item for 100 ticks, beside the pointer (kept on the
 * 640x480 screen), wrapped at 380 pixels, and gone when the selection moves,
 * the screen changes or a dialog comes up.  Not while the keys are taken. */
Result drawHelpCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 24 - 8;
    kHelpAlpha.set(app, (kDisplayFlags(app) & kNoTranslucency) != 0 ? kFloatOne : kFloatPoint9);
    g.eax = kCurrentMenu(app);
    bool restart = kCurrentMenu(app) != kHelpMenu(app);
    if (!restart)
    {
        GAME(at, kDialogShown);
        restart = (g.eax & 0xff) != 0;
    }
    if (restart)
    {
        kHelpNeedsMove.set(app, 1);
        g.eax = kCurrentMenu(app);
        kHelpMenu.set(app, g.eax);
        kHelpText.set(app, 0);
        kHelpLastX.set(app, kMouseX(app));
        kHelpItem.set(app, reg32(-1));
        kHelpLastY.set(app, kMouseY(app));
    }
    NEEDED(taken, keysTakenCore(g, entered(at)));
    g.eax = reg32(*taken);
    if (*taken != 0)
        return int32_t(g.eax);

    NEEDED(selected, selectedIndexCore(g));
    g.eax = reg32(*selected);
    const reg32 index = reg32(*selected);
    bool timing = true;
    GAME(at, kDialogShown);
    if ((g.eax & 0xff) != 0 || index == reg32(-1) || index != kHelpItem(app))
        kHelpItem.set(app, reg32(-1));
    else if (kHelpItem(app) != reg32(-1))
        timing = false;
    if (timing)
    {
        if (kHelpLastX(app) != kMouseX(app) || kHelpLastY(app) != kMouseY(app))
        {
            kHelpLastX.set(app, kMouseX(app));
            kHelpNeedsMove.set(app, 0);
            kHelpLastY.set(app, kMouseY(app));
            kHelpDue.set(app, kTicks(app) + 100);
        }
        else if (int32_t(kTicks(app)) > int32_t(kHelpDue(app)) && index != reg32(-1) && kHelpNeedsMove(app) == 0)
        {
            // The box: the text's size, wrapped, and a margin.
            kHelpItem.set(app, index);
            kHelpFade.set(app, 0);
            kHelpText.set(app, item(app, itemAt(int32_t(index))).help);
            const reg32 width = at, height = at + 4;  // [ebp - 8], [ebp - 4]
            GAME(at, kTextBox, {kHelpText(app), kHelpFont, kHelpWrap, width}, {height, 0});
            const int32_t w = int32_t(word(app, width)) + 7, h = int32_t(word(app, height)) + 3;
            kHelpX.set(app, kMouseX(app));
            kHelpWidth.set(app, reg32(w));
            kHelpHeight.set(app, reg32(h));
            if (int32_t(kMouseX(app)) + w > 630)
                kHelpX.set(app, reg32(630 - w));
            kHelpY.set(app, kMouseY(app) + 20);
            if (int32_t(kMouseY(app)) + 20 + h > 460)
                kHelpY.set(app, reg32(int32_t(kMouseY(app)) - h - 8));
        }
    }

    const reg32 helpText = kHelpText(app);
    if (helpText != 0 && byteAt(app, helpText) == 0)
        kHelpText.set(app, 0);
    if (kHelpText(app) != 0)
    {
        if (kHelpItem(app) == reg32(-1))
        {
            if (int32_t(kHelpFade(app)) > 0)
                kHelpFade.set(app, 0);
            kHelpText.set(app, 0);
        }
        else if (kHelpFade(app) != 255)
            kHelpFade.set(app, 255);
    }
    if (kHelpText(app) == 0)
    {
        kHelpAlpha.set(app, kFloatOne);
        return int32_t(g.eax);
    }

    const reg32 x = kHelpX(app), y = kHelpY(app);
    const reg32 w = kHelpWidth(app), h = kHelpHeight(app);
    const reg32 alpha = kHelpFade(app) << 24;
    kDrawColour.set(app, alpha);
    GAME(at, kFillBox, {x, y, x + w, y + h}, {reg32(-100), kHelpBoxColour});
    GAME(at, kBoxBorder, {x, y, w, h}, {alpha});
    if (kInRace(app) != 0)
        GAME(at, kDrawTextInRace, {kHelpText(app), x, y, kHelpFont}, {0, kHelpTextColour});
    else
        GAME(at, kDrawTextWrapped, {kHelpText(app), kHelpFont, 8, x + 7}, {y, kHelpWrap, 0, 0, 0, 0});
    kDrawColour.set(app, 0xff000000);
    kHelpAlpha.set(app, kFloatOne);
    return int32_t(g.eax);
}

/* 0x44be60's: the race settings kept consistent on every pass, and the options
 * the screens offer fitted to them (0x43a6e0: an option allowed, by its
 * bits).  Which setting each of these is is not pinned down yet; they are
 * named by their place in the settings (0x6fd2a0 on).  Two of them follow
 * each other -- 0 goes with 0, 1 with 4, 2 with 1 to 3 -- whichever changed
 * since the last pass setting the other.  Nothing after a race. */
#define ALLOW(option, on) GAME(at, kAllowOption, {option, on})

Result fitRaceSettingsCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 20;
    if (kAfterRace(app) != 0)
        return int32_t(g.eax);
    if (kSeen4d0(app) == reg32(-1))
    {
        kSeen4d0.set(app, kSetting4d0(app));
        kSeen4d4.set(app, kSetting4d4(app));
    }
    if (kSeenRaceType(app) == reg32(-1))
        kSeenRaceType.set(app, kRaceType(app));
    kSettingBbd8.set(app, kSettingBbe4(app) == 0 ? 1 : 0);
    if (kSettingBc00(app) == 1)
        kSetting292c.set(app, 0);
    else if (kSettingBc00(app) == 0)
        kSetting292c.set(app, 1);
    if (kRaceType(app) == kHotPursuit)
        kSetting4d0.set(app, 2);

    const reg32 first = kSetting4d0(app);
    if (first != kSeen4d0(app))
    {
        kSeen4d0.set(app, first);
        auto setSecond = [&](reg32 value) {
            kSeen4d4.set(app, value);
            kSetting4d4.set(app, value);
        };
        if (first == 0 && kSetting4d4(app) != 0)
            setSecond(0);
        else if (first == 1 && kSetting4d4(app) == 0)
            setSecond(4);
        else if (first == 2 && kSetting4d4(app) == 0)
            setSecond(1);
    }
    else if (kSetting4d4(app) != kSeen4d4(app))
    {
        const reg32 second = kSetting4d4(app);
        kSeen4d4.set(app, second);
        auto setFirst = [&](reg32 value) {
            kSeen4d0.set(app, value);
            kSetting4d0.set(app, value);
        };
        if (second == 0)
            setFirst(0);
        else if (second <= 3)
        {
            if (kSetting4d0(app) == 0)
                setFirst(2);
        }
        else if (kSetting4d0(app) == 0)
            setFirst(1);
    }
    if (kRaceType(app) != kSeenRaceType(app))
        kSeenRaceType.set(app, kRaceType(app));

    ALLOW(0xd, 0);
    ALLOW(0x4004, 1);
    if (int32_t(kSplitScreen(app)) >= 2)
    {
        if (kRaceType(app) == kTournament || kRaceType(app) == kKnockout)
        {
            if (kSetting504(app) == 3)
                kSetting504.set(app, 0);
            kSetting639c.set(app, 0);
        }
        else
            kSetting639c.set(app, 1);
        ALLOW(0x40004, 1);
        switch (kSetting504(app))
        {
        case 0:
            ALLOW(0x66c, 1);
            break;
        case 1:
            ALLOW(0x55c, 1);
            break;
        case 2:
            ALLOW(0x33c, 1);
            break;
        default:
            break;
        }
    }
    if (kRaceType(app) == kTournament || kRaceType(app) == kKnockout)
    {
        kSetting4e8.set(app, 1);
        kSetting4d0.set(app, 2);
        ALLOW(0x884, 1);
        switch (kSetting4d4(app))
        {
        case 1:
            ALLOW(0x68, 1);
            break;
        case 2:
            ALLOW(0x58, 1);
            break;
        case 3:
            ALLOW(0x38, 1);
            break;
        default:
            break;
        }
    }
    if (kRaceType(app) != kHotPursuit)
        ALLOW(0x4008, 1);
    else
    {
        ALLOW(0x2008, 1);
        const int32_t players = int32_t(kSplitScreen(app));
        bool cops;
        if (players >= 2)
            cops = kNetworkSetting2276(app) == int32_t(kNetworkSettingE5d0(app));
        else if (players == 1)
        {
            GAME(at, kCarFlag10, {kPlayer1Car(app)});
            cops = g.eax != 0;
            if (cops)
            {
                GAME(at, kCarFlag10, {kPlayer2Car(app)});
                cops = g.eax != 0;
            }
        }
        else
            cops = true;
        ALLOW(reg32(cops ? 0x2084 : 0xf04), 1);
    }

    // The car screens: the name of the one shown, against each.
    auto shown = [&](reg32 name, bool& is) -> bool {
        if (!g.call(at, kCurrentMenuName, {g.eax, name}))
            return false;
        if (!g.call(at, kStrcmp, {g.eax, name}))
            return false;
        is = g.eax == 0;
        return true;
    };
    bool is = false;
    if (!shown(kShowcaseScreen, is))
        return {};
    if (!is && !shown(kInteriorScreen, is))
        return {};
    if (is)
    {
        ALLOW(9, 1);
        ALLOW(0x80008, 1);
    }
    if (!shown(kHistoryScreen, is))
        return {};
    if (is)
    {
        ALLOW(9, 1);
        ALLOW(0x280008, 1);
    }
    if (!shown(kCompareScreen, is))
        return {};
    if (is)
        ALLOW(9, 1);

    auto widen = [](reg32 value) { return reg32(int32_t(int16_t(value))); };
    if (kRaceType(app) == kTournament)
    {
        const reg32 kind = int32_t(kSplitScreen(app)) > 1 ? 2 : 4;
        GAME(at, kMap4cf160, {kind});
        kSetting4b0.set(app, widen(g.eax));
        kSetting4ac.set(app, kValue5d004(app));
    }
    else if (kRaceType(app) == kKnockout)
    {
        GAME(at, kMap4cf160, {2});
        kSetting4b0.set(app, widen(g.eax));
        kSetting4ac.set(app, 0);
    }
    GAME(at, kMap4cf190, {reg32(int32_t(int16_t(kSetting4b0(app))))});
    kSetting4b4.set(app, widen(g.eax));
    kSeen4d0.set(app, kSetting4d0(app));
    kSeen4d4.set(app, kSetting4d4(app));
    if (kSetting2ac(app) != 0)
        GAME(at, kMarkOption8, {8});
    return int32_t(g.eax);
}

#undef ALLOW

/* A dialog over the screen asking for a player's name, when a race is
 * started without one: the default name ("Player 1") already in. */
bool askName(Game& g, reg32 at, reg32 name, reg32 title, reg32 defaultText, reg32 answer)
{
    App* app = g.app;
    if (!g.call(at, kLanguageText, {title}))
        return false;
    setWord(app, kDialogLines, g.eax);
    if (!g.call(at, kLanguageText, {kNameDialogLine2}))
        return false;
    setWord(app, kDialogLines + 4, g.eax);
    if (!g.call(at, kLanguageText, {kNameDialogOk}))
        return false;
    setWord(app, kDialogButtons, g.eax);
    if (!g.call(at, kLanguageText, {kNameDialogCancel}))
        return false;
    setWord(app, kDialogButtons + 4, g.eax);
    if (!g.call(at, kLanguageText, {defaultText}))
        return false;
    copyString(app, name, g.eax);
    return g.call(at, kOpenDialog, {2, kDialogLines, 2, kDialogButtons}, {0, name, kPlayerNameLength, 1, answer});
}

/* 0x44b230's: the menus, from the screen `name` (its .MNU and every one it
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
Result menuLoopCore(Game& g, reg32 esp, reg32 name, reg32 history)
{
    App* app = g.app;
    const reg32 at = esp - 20 - 0x10;
    const reg32 action = at;        // [ebp - 0x10]: 0x44b1a0 sets it
    const reg32 offset = at + 4;    // [ebp - 0xc]
    bool running = true, firstScreen = true;
    setWord(app, action, 0);
    setWord(app, offset, 0);

    GAME(at, kStrcmp, {name, kPostGame});
    kAfterRace.set(app, g.eax == 0 ? 1 : 0);
    kInRace.set(app, 0);
    kFlagEb10.set(app, 0);
    kFrontendArt.set(app, 0);
    GAME(at, kSetDisplay, {kDisplayChoice(app)});
    GAME(at, kResetInput);
    const reg32 afterRace = kAfterRace(app);
    GAME(at, kStartMusic, {afterRace != 0 ? kSetting4ac(app) : reg32(0x16), kMusicChoice(app), afterRace});
    GAME(at, kLoadMenus, {name, 1});
    kLastAction.set(app, 5);
    for (reg32 i = 0; i < kHistorySize; ++i)
        kHistory.set(app, int32_t(i), 0);
    if (history != 0)
    {
        g.eax = history;
        NEEDED(set, setHistoryCore(g, entered(at), history));
        g.eax = reg32(*set);
    }
    else
        kCurrentMenu.set(app, kScreens.at);

    while (running)
    {
        g.eax = kCurrentMenu(app);
        NEEDED(opened, enterScreenCore(g, entered(at), kCurrentMenu(app)));
        g.eax = reg32(*opened);
        setWord(app, action, g.eax);
        if (firstScreen)
        {
            g.eax = kCurrentMenu(app);
            NEEDED(next, selectNextCore(g, entered(at), kCurrentMenu(app)));
            g.eax = reg32(*next);
            firstScreen = false;
        }
        if (word(app, action) != 0)
            setWord(app, action, 3);
        kScreenUp.set(app, 1);
        GAME(at, kPlaySound, {kSoundEnter, kVolumeFull});

        while (word(app, action) == 0)
        {
            GAME(at, kPollEvents);
            const reg32 menu = kCurrentMenu(app);
            if (screen(app, menu).onFrame != 0)
            {
                GAME(at, screen(app, menu).onFrame, {menu, menu});
                setWord(app, action, g.eax);
            }
            if (diag::takeAutoReplay && word(app, action) == 0 && diag::takeAutoReplay())
            {
                GAME(at, kViewLastReplay);
                setWord(app, action, g.eax);
            }
            NEEDED(fitted, fitRaceSettingsCore(g, entered(at)));
            g.eax = reg32(*fitted);
            if (word(app, action) == 0)
            {
                GAME(at, kBeginFrame);
                g.eax = kCurrentMenu(app);
                NEEDED(drawn, drawItemsCore(g, entered(at), kCurrentMenu(app)));
                g.eax = reg32(*drawn);
                setWord(app, action, g.eax);
                GAME(at, kDialogShown);
                if ((g.eax & 0xff) != 0)
                {
                    GAME(at, kDialogPass);
                    setWord(app, action, g.eax);
                }
                g.eax = kCurrentMenu(app);
                NEEDED(passed, passItemsCore(g, entered(at), kCurrentMenu(app)));
                g.eax = reg32(*passed);
                GAME(at, kPresentFrame);
                const uint8_t noHelp = kNoHelpFlags(app);
                if ((noHelp & 0x10) == 0 && (noHelp & 0x20) == 0)
                {
                    NEEDED(helped, drawHelpCore(g, entered(at)));
                    g.eax = reg32(*helped);
                }
                GAME(at, kEndFrame);
            }
            if (word(app, action) == 0)
            {
                // The network screens hold the mouse only while connected with no network race set up.
                if (kNetworkGame(app) != 0 || kNetworkConnected(app) == 0)
                    kMouseBlocked.set(app, 0);
                g.eax = kCurrentMenu(app);
                NEEDED(under, itemUnderMouseCore(g, entered(at), kCurrentMenu(app)));
                g.eax = reg32(*under);
                const reg32 underMouse = reg32(*under);
                const reg32 current = kCurrentMenu(app);
                const int16_t was = screen(app, current).selected;
                bool ok = false;
                if (reg32(int32_t(was)) != underMouse)
                {
                    NEEDED(selectedOk, selectableCore(g, entered(at), int32_t(was), current));
                    g.eax = reg32(*selectedOk);
                    ok = *selectedOk != 0;
                }
                if (ok)
                {
                    GAME(at, kItemChanged);
                    const reg32 shown = kCurrentMenu(app);
                    screen(app, shown).previous = screen(app, shown).selected;
                }
                screen(app, kCurrentMenu(app)).selected = underMouse == reg32(-1) ? kNoItem : int16_t(underMouse);
                GAME(at, kReadKey, {1});
                const uint16_t key = uint16_t(g.eax);
                if (key != 0)
                {
                    const reg32 code = reg32(int32_t(int16_t(key)));
                    GAME(at, kDialogShown);
                    if ((g.eax & 0xff) != 0)
                    {
                        GAME(at, kDialogKey, {code});
                        setWord(app, action, g.eax);
                    }
                    else
                    {
                        g.eax = kCurrentMenu(app);
                        NEEDED(handled, handleKeyCore(g, entered(at), kCurrentMenu(app), code));
                        g.eax = reg32(*handled);
                        setWord(app, action, g.eax);
                    }
                    g.eax = kCurrentMenu(app);
                    NEEDED(waited, waitWhileSuspendedCore(g, entered(at), kCurrentMenu(app), action));
                    g.eax = reg32(*waited);
                    if (*waited != 0)
                        continue;
                }
            }

            if (word(app, action) == kQuit && kQuitting(app) == 0 && kNoQuitQuestion(app) == 0)
            {
                GAME(at, kLanguageText, {kQuitQuestion});
                setWord(app, kDialogLines, g.eax);
                GAME(at, kLanguageText, {kQuitYes});
                setWord(app, kDialogButtons, g.eax);
                GAME(at, kLanguageText, {kQuitNo});
                setWord(app, kDialogButtons + 4, g.eax);
                GAME(at, kOpenQuestion, {1, kDialogLines, 2, kDialogButtons}, {0, 0, 0, kQuitAnswerAt});
                setWord(app, action, 0);
                continue;
            }
            if (word(app, action) == kGoOn)
            {
                if (byteAt(app, kPlayer1Name) == 0)
                {
                    if (!askName(g, at, kPlayer1Name, kPlayer1NameTitle, kPlayer1Default, kPlayer1NameAnswerAt))
                        return {};
                    setWord(app, action, 0);
                }
                else if (kSplitScreen(app) == 1 && byteAt(app, kPlayer2Name) == 0)
                {
                    if (!askName(g, at, kPlayer2Name, kPlayer2NameTitle, kPlayer2Default, kPlayer2NameAnswerAt))
                        return {};
                    setWord(app, action, 0);
                }
            }
            if (word(app, action) == kBackToFirst && kCurrentMenu(app) == kScreens.at)
                setWord(app, action, 0);
        }

        // The screen left.
        if (kScreenShapes(app) != 0)
        {
            GAME(at, kFree, {kScreenShapes(app)});
            kScreenShapes.set(app, 0);
        }
        kScreenUp.set(app, 0);
        if (kNetworkConnected(app) != 0)
            GAME(at, kNetworkLeft);
        kLastAction.set(app, word(app, action));
        GAME(at, kScreenLeft, {screen(app, kCurrentMenu(app)).name});

        bool back = false;
        switch (word(app, action))
        {
        case kBackToFirst:
        {
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = reg32(*left);
            while (kCurrentMenu(app) != kScreens.at)
            {
                NEEDED(popped, popMenuCore(g));
                g.eax = reg32(*popped);
            }
            break;
        }
        case kGoOn:
        {
            const reg32 menu = kCurrentMenu(app);
            const reg32 it = menuItem(app, menu, screen(app, menu).selected);
            g.eax = menu;
            NEEDED(escape, escapeButtonCore(g, entered(at), menu, offset));
            g.eax = reg32(*escape);
            if (reg32(*escape) == it)
            {
                setWord(app, action, 3);
                back = true;
                break;
            }
            const reg32 next = item(app, it).type == kButton ? item(app, it).nextMenu : 0;
            if (next != 0)
            {
                for (setWord(app, offset, 0); int32_t(word(app, offset)) < kScreenCount(app);
                     setWord(app, offset, word(app, offset) + 1))
                {
                    GAME(at, kStrcmp, {next, screen(app, kScreens.address(int32_t(word(app, offset)))).name});
                    if (g.eax == 0)
                        break;
                }
                if (int32_t(word(app, offset)) < kScreenCount(app))
                {
                    NEEDED(pushed, pushMenuCore(g));
                    g.eax = reg32(*pushed);
                    NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
                    g.eax = reg32(*left);
                    kCurrentMenu.set(app, kScreens.address(int32_t(word(app, offset))));
                }
                else
                    setWord(app, action, 3);
            }
            else
                setWord(app, action, 3);
            running = true;
            break;
        }
        case kRace:
        case kRace7:
        {
            GAME(at, kPlaySound, {kSoundStart, kVolumeFull});
            GAME(at, kBeginFrame);
            g.eax = kCurrentMenu(app);
            NEEDED(drawn, drawItemsCore(g, entered(at), kCurrentMenu(app)));
            g.eax = reg32(*drawn);
            running = false;
            kScreenUp.set(app, 1);
            GAME(at, kSetCursor, {3});
            GAME(at, kPresentFrame);
            GAME(at, kEndFrame);
            kScreenUp.set(app, 0);
            kFlagEb10.set(app, 0);
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = reg32(*left);
            break;
        }
        case kMoviePlayed:
        {
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = reg32(*left);
            if (kAfterMovieScreen(app) != 0)
            {
                g.eax = kAfterMovieScreen(app);
                NEEDED(found, menuByNameCore(g, entered(at), kAfterMovieScreen(app)));
                g.eax = reg32(*found);
                if (*found != 0)
                {
                    g.eax = kAfterMovieScreen(app);
                    NEEDED(again, menuByNameCore(g, entered(at), kAfterMovieScreen(app)));
                    g.eax = reg32(*again);
                    kCurrentMenu.set(app, reg32(*again));
                    kAfterMovieScreen.set(app, 0);
                }
            }
            running = true;
            break;
        }
        case kQuit:
        {
            kFlagEb10.set(app, 0);
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = reg32(*left);
            running = false;
            break;
        }
        case kOut:
        {
            kFlagEb10.set(app, 0);
            running = false;
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = reg32(*left);
            break;
        }
        default:
            break;
        }
        if (back || word(app, action) == 3 || word(app, action) == 1)
        {
            const reg32 menu = kCurrentMenu(app);
            if (menu == kScreens.at)
            {
                setWord(app, action, kOut);
                running = false;
            }
            else
            {
                g.eax = menu;
                NEEDED(left, leaveScreenCore(g, entered(at), menu, word(app, action)));
                g.eax = reg32(*left);
                NEEDED(popped, popMenuCore(g));
                g.eax = reg32(*popped);
            }
        }
    }

    GAME(at, kCheckMenuFiles);
    if (kFrontendArt(app) != 0)
    {
        GAME(at, kFree, {kFrontendArt(app)});
        kFrontendArt.set(app, 0);
    }
    GAME(at, kMenusEnd);
    GAME(at, kCloseDialog);
    GAME(at, kStopMusic);
    return int32_t(word(app, action));
}

}  // namespace

/* ---- The adapters ---- */

/* 0x4499b0: whether the item at eax can be used. */
void itemUsable(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, itemUsableCore(g, e.esp, e.eax));
}

/* 0x4499f0: whether a key (edx) may act on the screen at eax. */
void keyAllowed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, keyAllowedCore(g, e.esp, e.eax, uint16_t(e.edx)));
}

/* 0x449aa0: the help box. */
void drawHelp(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawHelpCore(g, e.esp));
}

/* 0x449df0: whether the keys go to something other than the screen. */
void keysTaken(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, keysTakenCore(g, e.esp));
}

/* 0x449e50: whether the item eax of the screen edx can be selected. */
void selectable(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, selectableCore(g, e.esp, int32_t(e.eax), e.edx));
}

/* 0x449ea0: the pointer moved onto the selected item of the screen at eax. */
void pointAtSelected(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, pointAtSelectedCore(g, e.esp, e.eax));
}

/* 0x449f30: the item before the selected one of the screen at eax selected. */
void selectPrevious(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, selectPreviousCore(g, e.esp, e.eax));
}

/* 0x44a0c0: the item after the selected one of the screen at eax selected. */
void selectNext(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, selectNextCore(g, e.esp, e.eax));
}

/* 0x44a240: the Escape button of the screen at eax, its index into edx. */
void escapeButton(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, escapeButtonCore(g, e.esp, e.eax, e.edx));
}

/* 0x44a2d0: the key edx on the screen at eax. */
void handleKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, handleKeyCore(g, e.esp, e.eax, e.edx));
}

/* 0x44a440: the selected item of the screen shown. */
void selectedIndex(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, selectedIndexCore(g));
}

/* 0x44a470: the item at eax selected, if the screen shown has it. */
void selectItem(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, selectItemCore(g, e.esp, e.eax));
}

/* 0x44a4f0: the items of the screen at eax drawn. */
void drawItems(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawItemsCore(g, e.esp, e.eax));
}

/* 0x44a690: the mouse let go of the item it held. */
void releaseMouse(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, releaseMouseCore(g));
}

/* 0x44a6b0: the item of the screen at eax under the mouse. */
void itemUnderMouse(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, itemUnderMouseCore(g, e.esp, e.eax));
}

/* 0x44a860: every item of the screen at eax told it is up. */
void itemsShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, itemsShownCore(g, e.esp, e.eax));
}

/* 0x44a8b0: a pass of the items of the screen at eax. */
void passItems(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, passItemsCore(g, e.esp, e.eax));
}

/* 0x44a9e0: every item of the screen at eax told it is hidden. */
void hideItems(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, hideItemsCore(g, e.esp, e.eax));
}

/* 0x44aa30: the screen at eax left, how (edx). */
void leaveScreen(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, leaveScreenCore(g, e.esp, e.eax, e.edx));
}

/* 0x44aad0: the items of the screen at eax back to their widgets' states. */
void resetItems(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, resetItemsCore(g, e.eax));
}

/* 0x44ab40: the screen at eax opened. */
void enterScreen(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, enterScreenCore(g, e.esp, e.eax));
}

/* 0x44ae60: the screen named eax. */
void menuByName(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, menuByNameCore(g, e.esp, e.eax));
}

/* 0x44aeb0: the screen shown. */
void currentMenu(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, kCurrentMenu(app));
}

/* 0x44aec0: back to the screen gone from last. */
void popMenu(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, popMenuCore(g));
}

/* 0x44af10: the screen shown onto the history. */
void pushMenu(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, pushMenuCore(g));
}

/* 0x44af50: the history set from the names at eax. */
void setHistory(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, setHistoryCore(g, e.esp, e.eax));
}

/* 0x44af90: the answer eax of the dialog asking for player 1's name; edx the
 * byte that closes it. */
void player1NameAnswer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, nameAnswerCore(g, e.esp, e.eax, e.edx, kPlayer1Name, kPlayer1Default));
}

/* 0x44b040: and player 2's. */
void player2NameAnswer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, nameAnswerCore(g, e.esp, e.eax, e.edx, kPlayer2Name, kPlayer2Default));
}

/* 0x44b0f0: the answer eax of the quit question; edx the byte that closes it. */
void quitAnswer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, quitAnswerCore(g, e.esp, e.eax, e.edx));
}

/* 0x44b1a0: the game suspended waited out; edx where the action is kept. */
void waitWhileSuspended(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, waitWhileSuspendedCore(g, e.esp, e.eax, e.edx));
}

/* 0x44b230: the menus from the screen named eax, edx the history. */
void menuLoop(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, menuLoopCore(g, e.esp, e.eax, e.edx));
}

/* 0x44be60: the race settings kept consistent. */
void fitRaceSettings(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, fitRaceSettingsCore(g, e.esp));
}

}  // namespace loop
}  // namespace port
}  // namespace nfs3hp
