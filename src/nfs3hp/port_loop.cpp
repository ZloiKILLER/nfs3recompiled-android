#include "game_settings.h"
#include "port_loop.h"
#include "diag_hooks.h"
#include "port_text.h"
#include "native_ea_memory.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_draw.h"
#include "native_frontend_menua.h"
#include "native_frontend_menub.h"
#include "native_frontend_menud.h"
#include "native_frontend_menue.h"
#include "native_frontend_misc.h"
#include "native_frontend_screen.h"
#include "native_frontend_loada.h"
#include "native_frontend_connect.h"
#include "native_frontend_loop.h"
#include "native_frontend_menuc.h"
#include "native_frontend_text.h"
#include "native_frontend_wata.h"
#include "native_graphics.h"
#include "native_render.h"
#include "port_sound_speech.h"

#include "native_load_cardb.h"
#include "native_watcom.h"
#include "port_dialogs.h"
#include "port_draw2d.h"
#include "port_menus.h"
#include "port_sound_effects.h"
#include "port_sound_music.h"
#include "thrash_calls.h"

#include <cstdint>
#include <cstring>
#include "native_guest.h"

namespace nfs3hp
{
namespace port
{
namespace
{

/* A call into the game's code through guest::pointerCall: the register arguments of `a`, `stack` pushed, from the
 * game's stack at esp.  Its eax; false when the game stops. */
inline bool callPointer(App* app, ThreadState& ts, u32 esp, u32 target, Args a, std::initializer_list<u32> stack,
                        u32& result)
{
    ts.sp = esp;
    nfs::Args registers;
    registers.eax = a.eax;
    registers.edx = a.edx;
    registers.ebx = a.ebx;
    registers.ecx = a.ecx;
    const std::optional<u32> value = guest::pointerCall(app, ts, target, registers, stack);
    if (!value)
        return false;
    result = *value;
    return true;
}

inline bool callPointer(App* app, ThreadState& ts, u32 esp, u32 target, Args a = {},
                        std::initializer_list<u32> stack = {})
{
    u32 ignored;
    return callPointer(app, ts, esp, target, a, stack, ignored);
}

}  // namespace
}  // namespace port
}  // namespace nfs3hp

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
 * Each function is a typed core and a thunk under the game's name.  The
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
 * named in the thunks. */
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
    u32 onLoad, unused04, onEnter, onExit, onFrame;
    u32 name;         // 0x14
    int16_t firstItem;  // 0x18
    int16_t selected;   // 0x1a
    int16_t previous;   // 0x1c
    uint8_t unused1e[0x24 - 0x1e];
};

/* A row of the widgets' registry: its type, its name, the item a new one
 * starts as. */
struct Widget
{
    u32 type, name, templ;
};

/* An item: its widget's type, its state and flags, its place, its layer, its
 * help and codelink, the box the mouse finds it in, and its handlers. */
struct Item
{
    u32 type;           // 0x00
    uint16_t state;       // 0x04: the state byte and the flags byte as one word
    int16_t x, y;         // 0x06, 0x08
    int16_t priority;     // 0x0a: its layer, drawn from the highest down
    u32 help;           // 0x0c
    u32 codelink;       // 0x10
    int16_t postRaceOnly; // 0x14
    int16_t mainOnly;     // 0x16
    uint8_t unused18[2];
    int16_t top;          // 0x1a
    int16_t left;         // 0x1c
    int16_t width;        // 0x1e
    int16_t height;       // 0x20
    uint8_t unused22[2];
    u32 onShow;         // 0x24
    u32 onHide;         // 0x28
    u32 onDraw;         // 0x2c
    u32 onKey;          // 0x30
    u32 onShown;        // 0x34
    u32 onPass;         // 0x38
    u32 text;           // 0x3c
    u32 nextMenu;       // 0x40
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
constexpr u32 kButton = 5;

/* ---- The game's variables ---- */

// The screens (the first is the one the loop goes back to), their count, the
// items, the widgets' registry.
constexpr Records<Screen> kScreens{0x6040d0};
constexpr Var<int32_t> kScreenCount{0x660bb0};
constexpr Records<Item> kItems{0x604ee0};
constexpr u32 kWidgetsAt = 0x557a84;

// The loop's state.
constexpr Var<u32> kCurrentMenu{0x559230};
constexpr Table<u32> kHistory{0x662800};
constexpr u32 kHistorySize = 20;
constexpr Var<u32> kInRace{0x559234};       // the menus are a race's pause screens
constexpr Var<u32> kAfterRace{0x662968};    // the menus opened on "postgame"
constexpr Var<int16_t> kNetworkGame{0x7a22dc};        // set up to race others over a network
constexpr Var<u32> kNetworkConnected{0x7a22e0};
constexpr Var<u32> kMouseX{0x55e5e4};
constexpr Var<u32> kMouseY{0x55e5e8};
constexpr Var<u32> kMouseBlocked{0x5556d0};  // the network screens have the mouse
constexpr Var<uint8_t> kMouseHeld{0x559250};   // the item under the mouse stays the selected one
// The depth range 2D is drawn in: one for the five lowest layers, one above.
constexpr Var<u32> kDepthNear{0x563aa8};
constexpr Var<u32> kDepthFar{0x563aac};
constexpr u32 kFloatAlmostOne = 0x3f7fff00;
constexpr u32 kFloatTiny = 0x37800080;

// The help box (0x449aa0).
constexpr Var<u32> kHelpMenu{0x559248};      // the screen it was for
constexpr Var<u32> kHelpNeedsMove{0x559238}; // not before the pointer moves on a new screen
constexpr Var<u32> kHelpLastX{0x55923c};     // where the pointer was
constexpr Var<u32> kHelpLastY{0x559240};
constexpr Var<u32> kHelpDue{0x662950};       // the tick it shows at
constexpr Var<u32> kHelpItem{0x559244};      // the item it is shown for, -1 for none
constexpr Var<u32> kHelpText{0x55924c};
constexpr Var<u32> kHelpFade{0x662964};      // 0 or 255
constexpr Var<u32> kHelpX{0x662958};
constexpr Var<u32> kHelpY{0x66295c};
constexpr Var<u32> kHelpWidth{0x662960};
constexpr Var<u32> kHelpHeight{0x662954};
constexpr Var<u32> kHelpAlpha{0x559450};     // float, 0.9 with translucency
constexpr u32 kHelpFont = 0x12;
constexpr u32 kHelpWrap = 380;
constexpr u32 kHelpBoxColour = 0xff4b6cad;
constexpr u32 kHelpTextColour = 0x488cfd;
constexpr u32 kFloatOne = 0x3f800000;
constexpr u32 kFloatPoint9 = 0x3f666666;
constexpr Var<u32> kTicks{0x79c1a4};         // the game's clock
constexpr Var<uint8_t> kDisplayFlags{0x7a3a58};
constexpr uint8_t kNoTranslucency = 0x02;      // no fades, an opaque help box
constexpr Var<u32> kDrawColour{0x563aa4};    // what 2D is drawn with, alpha on top

// The race settings 0x44be60 keeps consistent (see there), and what it last
// saw of them.
constexpr u32 kTournament = 1;
constexpr u32 kKnockout = 2;
constexpr u32 kHotPursuit = 3;
constexpr Var<u32> kSeen4d0{0x559258};       // -1 before the first pass
constexpr Var<u32> kSeen4d4{0x55925c};
constexpr Var<u32> kSeenRaceType{0x559260};
constexpr Var<u32> kSetting292c{0x55292c};
constexpr Var<uint16_t> kSetting639c{0x55639c};
constexpr Var<u32> kValue5d004{0x55d004};
constexpr Var<int16_t> kNetworkSetting2276{0x7a2276};
constexpr Var<u32> kNetworkSettingE5d0{0x55e5d0};

// The loop's (0x44b230).
constexpr Var<u32> kLastAction{0x66296c};    // what left the last screen
constexpr Var<u32> kScreenUp{0x8b7630};      // a screen is being shown
constexpr Var<u32> kFlagEb10{0x55eb10};
constexpr Var<u32> kFrontendArt{0x559228};   // mmnu.qfs (or .fsh), loaded once
constexpr Var<u32> kScreenShapes{0x55922c};  // a screen's own, let go as it closes
constexpr Var<u32> kAfterMovieScreen{0x559224};  // the screen a movie goes on to, by name
constexpr Var<uint8_t> kNoHelpFlags{0x55eb2e}; // 0x10 or 0x20, no help box
constexpr Var<uint8_t> kQuitting{0x559251};    // Yes was the answer
constexpr Var<u32> kNoQuitQuestion{0x55e894};
constexpr Var<u32> kSuspended{0x559444};     // the game's window gone to the background
constexpr Var<u32> kResumed{0x55fec0};
constexpr Var<u32> kFade{0x559c60};          // a screen's fade out, to 500
constexpr Var<u32> kFadeIn{0x559c68};
constexpr u32 kArtFolder = 0x7a2db4;
// The dialogs the loop opens, their lines and buttons.
constexpr u32 kDialogLines = 0x662970;
constexpr u32 kDialogButtons = 0x66297c;
constexpr u32 kQuitQuestion = 0xf4;
constexpr u32 kQuitYes = 0xc4;
constexpr u32 kQuitNo = 0xc5;
constexpr u32 kPlayer1NameTitle = 0xc9;
constexpr u32 kPlayer2NameTitle = 0xca;
constexpr u32 kPlayer1Default = 0xfa;        // "Player 1"
constexpr u32 kPlayer2Default = 0xfb;
constexpr u32 kNameDialogLine2 = 0xf8;
constexpr u32 kNameDialogOk = 0xf9;
constexpr u32 kNameDialogCancel = 0xc6;
constexpr u32 kPlayerNameLength = 10;
// What leaves a screen (its handlers' results, the loop's actions).
constexpr u32 kBackToFirst = 4;              // back screen by screen to the first one
constexpr u32 kGoOn = 5;                     // to the selected button's nextmenu
constexpr u32 kRace = 6;                     // the menus left for the race
constexpr u32 kRace7 = 7;
constexpr u32 kMoviePlayed = 9;              // the attract-mode movie has played over the screen
constexpr u32 kQuit = 10;
constexpr u32 kOut = 11;                     // Back from the first screen
// The dialogs' answers, and what a dialog's callback is told.
constexpr u32 kDialogEnter = 1;              // Enter, or a click
constexpr u32 kDialogEscape = 3;
constexpr Var<u32> kInputFromMouse{0x664658};

// Keys, as 0x451960 gives them: ASCII, or a scan code in the high byte.
constexpr uint16_t kKeyEnter = 0x0d;
constexpr uint16_t kKeyEscape = 0x1b;
constexpr uint16_t kKeyUp = 0x4800;
constexpr uint16_t kKeyLeft = 0x4b00;
constexpr uint16_t kKeyRight = 0x4d00;
constexpr uint16_t kKeyDown = 0x5000;

// What a key handler or a screen's handler returns.
constexpr u32 kHandled = 2;                  // taken, and nothing more: 0 to the loop
constexpr u32 kKeyEscapeButton = 0x0d;       // what the Escape button is handed

// Sounds, 0x4181d0(sound, volume).
constexpr u32 kSoundMove = 0;
constexpr u32 kSoundBack = 3;
constexpr u32 kSoundEnter = 4;
constexpr u32 kSoundLeave = 5;
constexpr u32 kSoundStart = 7;
constexpr u32 kVolumeFull = 0x7f;
constexpr u32 kVolumeTick = 0x2d;

// The game's strings.
constexpr u32 kRaceNow = 0x538fb4, kEscape = 0x538fbc, kIpx = 0x538fc4, kPostGame = 0x539048;
constexpr u32 kMmnu = 0x538fc8, kQfsFormat = 0x538fd0, kFshFormat = 0x538fdc;
constexpr u32 kOpponent = 0x538fe8, kCar1 = 0x538ff4, kCar2 = 0x538ffc;
constexpr u32 kShowcaseScreen = 0x539084, kInteriorScreen = 0x539090, kHistoryScreen = 0x53909c;
constexpr u32 kCompareScreen = 0x5390a4;

/* ---- What they call ---- */

// What is not rewritten, or is held on its own: by address.
constexpr u32 kCodelinkMatches = 0x4429c0;
constexpr u32 kTextBox = 0x451ff0;           // a text's size, wrapped: (text, font, wrap, &w, &h, 0)
constexpr u32 kDrawTextWrapped = 0x451fc0;
constexpr u32 kDrawTextInRace = 0x452190;
constexpr u32 kFillBox = 0x4d8550;
constexpr u32 kCarFlag10 = 0x4396b0;         // a car's flag 0x10
constexpr u32 kMap4cf160 = 0x4cf160;         // 2 -> 0, 4 -> 1, else 2
constexpr u32 kMap4cf190 = 0x4cf190;         // 0 -> 2, 1 -> 4, else 8
constexpr u32 kMarkOption8 = 0x442b60;
constexpr u32 kLoadMenus = 0x449510;
constexpr u32 kCheckMenuFiles = 0x449960;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kFree = 0x4e1890;
constexpr u32 kSetDisplay = 0x4d1750;
constexpr u32 kResetInput = 0x451b20;
constexpr u32 kStartMusic = 0x4104f0;        // (track, music choice, after a race)
constexpr u32 kStopMusic = 0x4109d0;
constexpr u32 kPollEvents = 0x4c9860;
constexpr u32 kViewLastReplay = 0x44e710;  // the results' View Replay: 7 when it loaded
constexpr u32 kReadKey = 0x451960;           // the next key, 0 for none
constexpr u32 kBeginFrame = 0x4d9120;
constexpr u32 kPresentFrame = 0x4dab90;
constexpr u32 kEndFrame = 0x4d9140;          // and waits for the next one
constexpr u32 kItemChanged = 0x460760;
constexpr u32 kScreenLeft = 0x4534d0;        // (its name)
constexpr u32 kMenusEnd = 0x452110;
constexpr u32 kNetworkLeft = 0x440d20;
constexpr u32 kDialogPass = 0x445f50;        // a dialog's pass, its result the loop's
constexpr u32 kDialogKey = 0x445e60;
constexpr u32 kCloseDialog = 0x445f10;
constexpr u32 kDialogShown = 0x4438b0;
constexpr u32 kNameValid = 0x4721f0;
constexpr u32 kResetDisplay = 0x4daf80;
constexpr u32 kLoadPointer = 0x49c140;
constexpr u32 kLoadShapes = 0x4956f0;        // a .qfs or .fsh file, 0 when it is not there
constexpr u32 kLoadDialogArt = 0x445de0;
constexpr u32 kClearBackground = 0x465d60;
constexpr u32 kSetBackground = 0x465d80;
constexpr u32 kLoadScreenArt = 0x4a4ba0;
constexpr u32 kFrameTime = 0x4f0650;
constexpr u32 kSleep = 0x4f0710;
constexpr u32 kResumeSound = 0x4bef50;
constexpr u32 kBoxBorder = 0x4d8930;
constexpr u32 kStrcmp = 0x4ee310;
constexpr u32 kSetMousePosition = 0x49bfc0;
constexpr u32 kNetworkFrame = 0x441040;      // the network screens' pass; nonzero leaves
constexpr u32 kNetworkBusy = 0x441680;
constexpr u32 kNetworkKey = 0x441700;
constexpr u32 kNetworkShown = 0x440970;
constexpr u32 kHotKey = 0x4524e0;            // the keys that work on any screen
constexpr u32 kSetCursor = 0x49c0e0;
constexpr u32 kPlaySound = 0x4181d0;
constexpr u32 kCurrentMenuName = 0x44ae00;
constexpr u32 kShowItems = 0x44a960;
constexpr u32 kQuitAnswerAt = 0x44b0f0;
constexpr u32 kPlayer1NameAnswerAt = 0x44af90;
constexpr u32 kPlayer2NameAnswerAt = 0x44b040;

/* ---- Small things ---- */

u32 word(App* app, u32 at)
{
    return app->getMemory<u32>(at);
}

void setWord(App* app, u32 at, u32 value)
{
    app->getMemory<u32>(at) = value;
}

uint8_t byteAt(App* app, u32 at)
{
    return app->getMemory<uint8_t>(at);
}

void setByte(App* app, u32 at, uint8_t value)
{
    app->getMemory<uint8_t>(at) = value;
}

void copyString(App* app, u32 to, u32 from)
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
constexpr u32 entered(u32 at)
{
    return at - 4;
}

/* The item `index` of the table, its address. */
u32 itemAt(int32_t index)
{
    return kItems.address(index);
}

Screen& screen(App* app, u32 at)
{
    return record<Screen>(app, at);
}

Item& item(App* app, u32 at)
{
    return record<Item>(app, at);
}

/* A screen's first item, and the item `offset` from it. */
int32_t firstItem(App* app, u32 menu)
{
    return screen(app, menu).firstItem;
}

u32 menuItem(App* app, u32 menu, int32_t offset)
{
    return itemAt(firstItem(app, menu) + offset);
}

bool isEnd(App* app, u32 at)
{
    return item(app, at).type == 0;
}

/* The game's code being run: the calls it makes, and what eax is as the
 * game's code would have it at this point (the last call's result, or what
 * was handed in). */
struct Game
{
    App* app;
    ThreadState& ts;
    u32 eax;
    bool call(u32 esp, u32 target, Args a = {}, std::initializer_list<u32> stack = {})
    {
        return callPointer(app, ts, esp, target, a, stack, eax);
    }
        /* A typed core called as C++ where the game's code calls its address, from `esp`: eax its
     * result (unchanged for a core that gives none); false when the game stopped in it. */
    template <u32 Address, auto Core, typename... A>
    bool direct(u32 esp, A... args)
    {
        ts.sp = esp;
        if constexpr (std::is_void_v<decltype(guest::direct<Address, Core>(app, ts, args...))>)
        {
            guest::direct<Address, Core>(app, ts, args...);
            return !ts.terminate;
        }
        else
        {
            const auto result = guest::direct<Address, Core>(app, ts, args...);
            if (ts.terminate)
                return false;
            eax = valueOf(result);
            return true;
        }
    }
    static u32 valueOf(u32 value) { return value; }
    template <class T>
    static u32 valueOf(const std::optional<T>& value)
    {
        return u32(*value);
    }
};

/* A call into the game's code that gives up the function when the game
 * stopped in it. */
/* The same for a typed core, called as C++ (Game::direct). */
#define GAME_DIRECT(address, core, ...)                        \
    do                                                         \
    {                                                          \
        if (!g.template direct<address, core>(__VA_ARGS__))    \
            return {};                                         \
    } while (0)

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

bool playSound(Game& g, u32 esp, u32 sound, u32 volume)
{
    return g.direct<kPlaySound, playMenuEffectOf>(esp, entered(esp), int32_t(sound), int32_t(volume));
}

/* ---- The selection ---- */

/* 0x4499b0's: whether an item can be used: in a network race's locked lobby,
 * only the one whose codelink names "racenow". */
Result itemUsableCore(Game& g, u32 esp, u32 at)
{
    App* app = g.app;
    if (kNetworkConnected(app) == 0 || gameSettings(app).networkLobby == 0 || kInRace(app) != 0)
        return 1;
    GAME_DIRECT(kCodelinkMatches, menus::codelinkMatched, esp - 8,
                entered(esp - 8), item(app, at).codelink, kRaceNow, u32(0));
    return g.eax != 0 ? 1 : 0;
}

/* An item's state word as selection reads it: greyed out too when it is not
 * usable now (0x4499b0). */
Result selectionState(Game& g, u32 esp, u32 at)
{
    uint16_t state = item(g.app, at).state;
    g.eax = at;
    NEEDED(usable, itemUsableCore(g, entered(esp), at));
    g.eax = u32(*usable);
    if (*usable == 0)
        state |= kUnusable;
    return state;
}

/* 0x4499f0's: whether a key may act on a screen: always, but in a locked
 * lobby while the network is idle only on a usable item, and never Escape. */
Result keyAllowedCore(Game& g, u32 esp, u32 menu, uint16_t key)
{
    App* app = g.app;
    if (kNetworkConnected(app) == 0 || gameSettings(app).networkLobby == 0 || kInRace(app) != 0)
        return 1;
    const u32 at = esp - 8;
    GAME_DIRECT(0x441680, frontend::menua::panelBusy, at);
    const u32 busy = g.eax;
    if (busy != 0)
        return 1;
    const Screen& s = screen(app, menu);
    if (s.selected == kNoItem || key == kKeyEscape)
        return int32_t(busy);
    g.eax = menuItem(app, menu, s.selected);
    NEEDED(usable, itemUsableCore(g, entered(at), g.eax));
    g.eax = u32(*usable);
    return *usable != 0 ? 1 : 0;
}

/* 0x449e50's: whether the item `offset` of a screen can be selected. */
Result selectableCore(Game& g, u32 esp, int32_t offset, u32 menu)
{
    if (offset < 0 || offset == kNoItem)
        return 0;
    NEEDED(state, selectionState(g, esp - 4, menuItem(g.app, menu, offset)));
    return (*state & kNotSelectable) == 0 ? 1 : 0;
}

/* 0x449ea0's: the mouse pointer moved onto the selected item, three quarters
 * along it and halfway down. */
Result pointAtSelectedCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const Screen& s = screen(app, menu);
    if (s.selected == kNoItem)
        return int32_t(g.eax);
    const Item& it = item(app, menuItem(app, menu, s.selected));
    const int32_t x = it.left + 3 * it.width / 4;
    kMouseX.set(app, u32(x));
    const int32_t y = it.top + it.height / 2;
    kMouseY.set(app, u32(y));
    GAME_DIRECT(0x49bfc0, frontend::screen::setPointer, esp - 12, u32(x), u32(y));
    return int32_t(g.eax);
}

/* What selecting another item ends with: the one left kept as the previous
 * when it could be selected, a tick when the selection moved -- in that order
 * going forward, the other way round going back -- and the pointer on the
 * new one. */
Result afterSelecting(Game& g, u32 at, u32 menu, int16_t was, bool forward)
{
    App* app = g.app;
    auto keepPrevious = [&]() -> bool {
        NEEDED(ok, selectableCore(g, entered(at), int32_t(was), menu));
        g.eax = u32(*ok);
        if (*ok != 0)
            screen(app, menu).previous = was;
        return true;
    };
    if (forward && !keepPrevious())
        return {};
    if (screen(app, menu).selected != was)
        GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundMove), int32_t(kVolumeFull));
    if (!forward && !keepPrevious())
        return {};
    g.eax = menu;
    return pointAtSelectedCore(g, entered(at), menu);
}

/* The previous selection back when the one selected cannot be: whether it was. */
Result backToPrevious(Game& g, u32 at, u32 menu, int16_t was)
{
    App* app = g.app;
    if (was != kNoItem)
    {
        NEEDED(ok, selectableCore(g, entered(at), int32_t(was), menu));
        g.eax = u32(*ok);
        if (*ok != 0)
            return 0;
    }
    const int16_t previous = screen(app, menu).previous;
    NEEDED(ok, selectableCore(g, entered(at), int32_t(previous), menu));
    g.eax = u32(*ok);
    if (*ok == 0)
        return 0;
    screen(app, menu).selected = screen(app, menu).previous;
    return 1;
}

/* 0x449f30's: the item before the selected one that can be selected, round
 * from the first to the last; with none selected, the last of them.  Not
 * while the selected item has the keys. */
Result selectPreviousCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 24;
    const int16_t was = screen(app, menu).selected;
    if (was != kNoItem && (item(app, menuItem(app, menu, was)).state & kHasKeys) != 0)
        return int32_t(u32(firstItem(app, menu) + was) * 47);  // the selected item's offset in the table, a quarter of it
    NEEDED(back, backToPrevious(g, at, menu, was));
    if (!*back)
    {
        if (screen(app, menu).selected == kNoItem)
        {
            for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
            {
                NEEDED(state, selectionState(g, at, itemAt(index)));
                g.eax = u32(*state);
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
                g.eax = u32(*state);
                if ((*state & kNotSelectable) == 0)
                    break;
            }
        }
    }
    return afterSelecting(g, at, menu, was, false);
}

/* 0x44a0c0's: the item after the selected one that can be selected, round
 * from the last to the first; with none selected, the first of them. */
Result selectNextCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 24;
    const int16_t was = screen(app, menu).selected;
    if (was != kNoItem && (item(app, menuItem(app, menu, was)).state & kHasKeys) != 0)
        return int32_t(u32(firstItem(app, menu) + was) * 47);  // the selected item's offset in the table, a quarter of it
    NEEDED(back, backToPrevious(g, at, menu, was));
    if (!*back)
    {
        if (screen(app, menu).selected == kNoItem)
        {
            for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
            {
                NEEDED(state, selectionState(g, at, itemAt(index)));
                g.eax = u32(*state);
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
                g.eax = u32(*state);
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
Result escapeButtonCore(Game& g, u32 esp, u32 menu, u32 offset)
{
    App* app = g.app;
    const u32 at = esp - 24;
    u32 found = 0;
    setWord(app, offset, 0);
    if (menu == 0)
        return 0;
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        const u32 it = itemAt(index);
        const u32 codelink = item(app, it).codelink;
        if (item(app, it).type != kButton || codelink == 0 || found != 0)
            continue;
        GAME_DIRECT(kCodelinkMatches, menus::codelinkMatched, at, entered(at), codelink, kEscape,
                    u32(0));
        if (g.eax != 0 && (item(app, it).state & kDisabledState) == 0)
        {
            found = it;
            setWord(app, offset, u32(index - firstItem(app, menu)));
        }
    }
    return int32_t(found);
}

/* 0x44a440's: the selected item of the screen shown, by its index in the item
 * table; -1 for none. */
Result selectedIndexCore(Game& g)
{
    App* app = g.app;
    const u32 menu = kCurrentMenu(app);
    if (screen(app, menu).selected == kNoItem)
        return -1;
    return firstItem(app, menu) + screen(app, menu).selected;
}

/* 0x44a470's: `it` selected on the screen shown, if it is one of its items:
 * whether it is.  The selection left is kept as the previous when it could be
 * selected. */
Result selectItemCore(Game& g, u32 esp, u32 it)
{
    App* app = g.app;
    const u32 at = esp - 16;
    const u32 menu = kCurrentMenu(app);
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        if (itemAt(index) != it)
            continue;
        const u32 current = kCurrentMenu(app);
        NEEDED(ok, selectableCore(g, entered(at), int32_t(screen(app, current).selected), current));
        g.eax = u32(*ok);
        if (*ok != 0)
            screen(app, current).previous = screen(app, current).selected;
        const u32 shown = kCurrentMenu(app);
        screen(app, shown).selected = int16_t(index - firstItem(app, shown));
        return 1;
    }
    return 0;
}

/* 0x44a660's: whether an item is shown: in the pause screens, only one marked
 * for them. */
bool shownNow(App* app, u32 it)
{
    return !(kInRace(app) != 0 && (item(app, it).state & kInRaceToo) == 0);
}

/* 0x44a690's: the mouse let go of the item it held.  The flag is cleared
 * through ah, so eax's second byte goes with it. */
Result releaseMouseCore(Game& g)
{
    kMouseHeld.set(g.app, 0);
    return int32_t(g.eax & ~u32(0xff00));
}

/* 0x44a6b0's: the item under the mouse, by its index from the screen's first,
 * -1 for none: the lowest layer first, the first item of it the pointer is
 * inside of.  While a dialog is up, the mouse holds an item, or the selected
 * item has the keys, the selected one; none while the network screens have
 * the mouse.  Moving onto another item ticks. */
Result itemUnderMouseCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    constexpr int32_t kLastLayer = 9999;
    const u32 at = esp - 40;
    if (kMouseBlocked(app) != 0)
        return -1;
    GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, at);
    if ((g.eax & 0xff) != 0 || kMouseHeld(app) != 0
        || (screen(app, menu).selected != kNoItem
            && (item(app, menuItem(app, menu, screen(app, menu).selected)).state & kHasKeys) != 0))
        return screen(app, menu).selected;
    for (int32_t layer = 0; layer < kLastLayer;)
    {
        int32_t next = kLastLayer;
        for (int16_t index = int16_t(firstItem(app, menu)); !isEnd(app, itemAt(index)); ++index)
        {
            const u32 it = itemAt(index);
            NEEDED(state, selectionState(g, at, it));
            g.eax = u32(*state);
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
                GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundMove), int32_t(kVolumeTick));
            return offset;
        }
        layer = next;
    }
    return -1;
}

/* ---- The items ---- */

/* 0x44a860's: every item's kOnShown handler, the screen being up; then the
 * mouse lets go. */
Result itemsShownCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 12;
    int16_t index = int16_t(firstItem(app, menu));
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const u32 it = itemAt(index);
        if (item(app, it).onShown != 0)
            GAME(at, item(app, it).onShown, {it, it});
    }
    g.eax = u32(int32_t(index)) * sizeof(Item);  // the end's offset in the table
    return releaseMouseCore(g);
}

/* 0x44a8b0's: a pass of the items: each one's kOnPass handler -- 3 or 4
 * from one ends it -- then the selected one's again; the result the pointer's
 * shape (0x49c0e0), 12 when a key would not be taken now. */
Result passItemsCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 16;
    u32 result = 0;
    for (int32_t index = firstItem(app, menu); !isEnd(app, itemAt(index)); ++index)
    {
        const u32 it = itemAt(index);
        if (item(app, it).onPass == 0)
            continue;
        GAME(at, item(app, it).onPass, {it, u32(index)});
        result = g.eax;
        if (result == kBackToFirst || result == 3)
        {
            GAME_DIRECT(kSetCursor, frontend::connect::setPointer, at, result);
            return int32_t(g.eax);
        }
        result = 0;
    }
    if (screen(app, menu).selected != kNoItem)
    {
        const u32 it = menuItem(app, menu, screen(app, menu).selected);
        if (item(app, it).onPass != 0)
        {
            GAME(at, item(app, it).onPass, {it, it});
            result = g.eax;
        }
    }
    NEEDED(allowed, keyAllowedCore(g, entered(at), menu, 0));
    g.eax = u32(*allowed);
    if (*allowed == 0)
        result = 12;
    GAME_DIRECT(kSetCursor, frontend::connect::setPointer, at, result);
    return int32_t(g.eax);
}

/* 0x44a960's: every item's kOnShow handler, the screen opening; an item for
 * after a race only, or for before one only, greyed out and hidden when it
 * is not the time for it.  The network screens get theirs.  (Native whole in
 * native_frontend_loop.cpp; called by address.) */

/* 0x44a9e0's: every item's kOnHide handler, the screen closing. */
Result hideItemsCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 12;
    int16_t index = int16_t(firstItem(app, menu));
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const u32 it = itemAt(index);
        if (item(app, it).onHide != 0)
            GAME(at, item(app, it).onHide, {it, it});
    }
    return int32_t(u32(int32_t(index)) * sizeof(Item));  // the end's offset in the table
}

/* 0x44aad0's: every item's state and flags back to its widget's template's,
 * the screen opening. */
Result resetItemsCore(Game& g, u32 menu)
{
    App* app = g.app;
    int32_t index = firstItem(app, menu);
    for (; !isEnd(app, itemAt(index)); ++index)
    {
        const u32 it = itemAt(index);
        for (u32 widget = kWidgetsAt; word(app, widget) != 0; widget += sizeof(Widget))
        {
            if (word(app, widget) != item(app, it).type)
                continue;
            const u32 from = record<Widget>(app, widget).templ;
            if (from != 0)
                item(app, it).state = item(app, from).state;
            break;
        }
    }
    return int32_t(u32(index) * sizeof(Item));  // the end's offset in the table
}

/* ---- The screens ---- */

/* 0x44ae60's: a screen by its name, 0 for none loaded. */
Result menuByNameCore(Game& g, u32 esp, u32 name)
{
    App* app = g.app;
    const u32 at = esp - 20;
    for (u32 index = 0; index < u32(kScreenCount(app)); ++index)
    {
        GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, name,
                    screen(app, kScreens.address(int32_t(index))).name);
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
    const u32 back = kHistory(app, 0);
    for (u32 i = 0; i + 1 < kHistorySize; ++i)
        kHistory.set(app, int32_t(i), kHistory(app, int32_t(i + 1)));
    kCurrentMenu.set(app, back != 0 ? back : kScreens.at);
    return int32_t(kHistorySize - 1);
}

/* 0x44af10's: the screen shown onto the history, before going to another. */
Result pushMenuCore(Game& g)
{
    App* app = g.app;
    for (u32 i = kHistorySize - 1; i > 0; --i)
        kHistory.set(app, int32_t(i), kHistory(app, int32_t(i - 1)));
    const u32 menu = kCurrentMenu(app);
    kHistory.set(app, 0, menu);
    return int32_t(menu);
}

/* 0x44af50's: the history set from a list of screen names, ended by a null
 * (the screens to come back through after a race), then the first of them
 * shown. */
Result setHistoryCore(Game& g, u32 esp, u32 names)
{
    App* app = g.app;
    const u32 at = esp - 20;
    for (u32 i = 0; word(app, names + 4 * i) != 0; ++i)
    {
        g.eax = word(app, names + 4 * i);
        NEEDED(menu, menuByNameCore(g, entered(at), g.eax));
        g.eax = u32(*menu);
        if (*menu != 0)
            kHistory.set(app, int32_t(i), u32(*menu));
    }
    return popMenuCore(g);
}

/* 0x449df0's: whether the keys go to something other than the screen: an
 * open list (an item of the list widgets that has the keys) or a dialog. */
Result keysTakenCore(Game& g, u32 esp)
{
    App* app = g.app;
    NEEDED(index, selectedIndexCore(g));
    g.eax = u32(*index);
    if (*index != -1)
    {
        const Item& it = item(app, itemAt(*index));
        if ((it.state & kHasKeys) != 0)
        {
            const u32 type = it.type;
            if (type == 0x11 || type == 0x1d || type == 0x12 || type == 0x13)
                return 1;
        }
    }
    GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, esp - 8);
    return (g.eax & 0xff) != 0 ? 1 : 0;
}

/* 0x44a2d0's: a key on a screen.  In a network race the network screens see
 * it first; then the keys that work anywhere; then the selected item's key
 * handler.  Whatever none of them took: Escape presses the Escape button
 * (1, the screen left, when there is none), the arrows move the selection.
 * The result is the loop's (kHandled reads as 0). */
Result handleKeyCore(Game& g, u32 esp, u32 menu, u32 key)
{
    App* app = g.app;
    const u32 at = esp - 24;
    const uint16_t code = uint16_t(key);
    const u32 signedCode = u32(int32_t(int16_t(code)));
    u32 result = 0;
    if (kNetworkConnected(app) != 0)
    {
        GAME_DIRECT(0x441700, frontend::menua::panelKey, at, std::uint16_t(signedCode));
        result = g.eax;
        if (result != 0)
            return int32_t(result == kHandled ? 0 : result);
        // (The allowed check's key is the edx the network's call was handed.)
        NEEDED(allowed, keyAllowedCore(g, entered(at), menu, code));
        g.eax = u32(*allowed);
        if (*allowed == 0)
            return 0;
    }
    if (kAfterRace(app) == 0)
        GAME_DIRECT(0x4524e0, frontend::menub::cheatKey, at, signedCode);
    if (screen(app, menu).selected == kNoItem)
    {
        if (code == kKeyEnter)
            return 0;
    }
    else
    {
        const u32 it = menuItem(app, menu, screen(app, menu).selected);
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
            GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundBack), int32_t(kVolumeFull));
            const u32 offset = at;  // [ebp - 4]
            setWord(app, offset, result);
            NEEDED(button, escapeButtonCore(g, entered(at), menu, offset));
            g.eax = u32(*button);
            if (*button != 0)
            {
                screen(app, menu).selected = int16_t(word(app, offset));
                screen(app, menu).previous = int16_t(word(app, offset));
                GAME(at, item(app, u32(*button)).onKey, {u32(*button), menu, kKeyEscapeButton});
                result = g.eax;
            }
            else
                result = 1;
            break;
        }
        case kKeyUp:
        case kKeyLeft:
        {
            GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundMove), int32_t(kVolumeTick));
            g.eax = menu;
            NEEDED(r, selectPreviousCore(g, entered(at), menu));
            g.eax = u32(*r);
            break;
        }
        case kKeyRight:
        case kKeyDown:
        {
            GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundMove), int32_t(kVolumeTick));
            g.eax = menu;
            NEEDED(r, selectNextCore(g, entered(at), menu));
            g.eax = u32(*r);
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
Result drawItemsCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 36;
    int32_t layer = 0;
    for (int16_t index = int16_t(firstItem(app, menu)); !isEnd(app, itemAt(index)); ++index)
    {
        const int32_t priority = item(app, itemAt(index)).priority;
        if (priority > layer)
            layer = priority;
    }
    u32 result = 0;
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
            const u32 it = itemAt(index);
            if (item(app, it).priority != layer)
                continue;
            const u32 isSelected = index == firstItem(app, menu) + screen(app, menu).selected ? 1 : 0;
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
                GAME_DIRECT(0x441040, frontend::menua::panelFrame, at);
                if (g.eax != 0)
                    result = g.eax;
            }
        }
        else if (layer == 8 && kNetworkConnected(app) != 0)
        {
            GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, screen(app, menu).name, kIpx);
            if (g.eax != 0)
            {
                GAME_DIRECT(0x441040, frontend::menua::panelFrame, at);
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
Result leaveScreenCore(Game& g, u32 esp, u32 menu, u32 action)
{
    App* app = g.app;
    const u32 at = esp - 16;
    const u32 step = (kDisplayFlags(app) & kNoTranslucency) != 0 ? 50 : 25;
    if (screen(app, menu).onExit != 0)
        GAME(at, screen(app, menu).onExit, {menu, action, step, menu});
    if (kInRace(app) == 0)
    {
        if (action != kMoviePlayed)
        {
            GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundLeave), int32_t(kVolumeFull));
            if ((kDisplayFlags(app) & kNoTranslucency) == 0)
            {
                for (kFade.set(app, step); int32_t(kFade(app)) < 500; kFade.set(app, kFade(app) + step))
                {
                    GAME_DIRECT(kBeginFrame, graphics::windowTakenAndCleared, at);
                    g.eax = menu;
                    NEEDED(drawn, drawItemsCore(g, entered(at), menu));
                    g.eax = u32(*drawn);
                    GAME_DIRECT(kEndFrame, graphics::flipAndWait, at);
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
Result enterScreenCore(Game& g, u32 esp, u32 menu)
{
    App* app = g.app;
    const u32 at = esp - 20 - 0x100;
    if (kInRace(app) == 0)
    {
        GAME_DIRECT(kSetCursor, frontend::connect::setPointer, at, 0u);
        // eax holds the art's name here, and edx 0, as the original has them.
        GAME(at, kResetDisplay, {kMmnu});
        GAME_DIRECT(0x49c140, frontend::menud::loadPointerArt, at);
        if (kFrontendArt(app) == 0)
        {
            const u32 path = at;  // char[0x100]
            GAME_DIRECT(kSprintf, watcom::clib::sprintf, at, path, kQfsFormat, guest::Words{kArtFolder, kMmnu});
            GAME_DIRECT(kLoadShapes, frontend::draw::loadQfsFlags, at, path, 0u);
            kFrontendArt.set(app, g.eax);
            if (kFrontendArt(app) == 0)
            {
                GAME_DIRECT(kSprintf, watcom::clib::sprintf, at, path, kFshFormat, guest::Words{kArtFolder, kMmnu});
                GAME_DIRECT(kLoadShapes, frontend::draw::loadQfsFlags, at, path, 0u);
                kFrontendArt.set(app, g.eax);
            }
        }
        GAME(at, kLoadDialogArt);
        GAME_DIRECT(kClearBackground, frontend::menuc::clearMenuArtSlots, at);
        GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, screen(app, menu).name, kOpponent);
        bool carScreen = g.eax == 0;
        if (!carScreen)
        {
            GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, screen(app, menu).name, kCar1);
            carScreen = g.eax == 0;
        }
        if (!carScreen)
        {
            GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, screen(app, menu).name, kCar2);
            carScreen = g.eax == 0;
        }
        if (carScreen)
            GAME_DIRECT(kSetBackground, frontend::menuc::loadMenuArtSlots, at, kFrontendArt(app));
        GAME_DIRECT(0x4a4ba0, frontend::menud::loadLogoPictures, at);
    }
    screen(app, menu).selected = kNoItem;
    screen(app, menu).previous = screen(app, menu).selected;
    kScreenShapes.set(app, 0);
    g.eax = menu;
    NEEDED(reset, resetItemsCore(g, menu));
    g.eax = u32(*reset);
    u32 result = 0;
    if (screen(app, menu).onEnter != 0)
    {
        GAME(at, screen(app, menu).onEnter, {menu, 0});
        result = g.eax;
    }
    GAME_DIRECT(kShowItems, frontend::loop::showItems, at, menu);
    g.eax = menu;
    NEEDED(shown, itemsShownCore(g, entered(at), menu));
    g.eax = u32(*shown);
    GAME_DIRECT(kFrameTime, frontend::wata::timerSinceLast, at);
    return int32_t(result);
}

/* 0x44b1a0's: the game suspended (0x559444, its window gone to the
 * background): waited out, then the screen closed as on leaving it, *action
 * 13 (back to the first screen).  Whether it was. */
Result waitWhileSuspendedCore(Game& g, u32 esp, u32 menu, u32 action)
{
    App* app = g.app;
    const u32 at = esp - 16;
    if (kSuspended(app) == 0)
        return 0;
    setWord(app, action, 13);
    do
    {
        GAME(at, kSleep, {0});
        GAME_DIRECT(kPollEvents, descriptionServed, at, entered(at));
    } while (kSuspended(app) != 0);
    // THRASH_treset, through the game's pointer to it (thrash_calls.h).
    const std::optional<u32> reset = guest::driverFrom<thrash::kThrashTReset, &driver::tReset>(app, g.ts, at);
    if (!reset)
        return {};
    g.eax = *reset;
    kResumed.set(app, 0);
    GAME_DIRECT(kResumeSound, render::screenModeSet, at, 1u);
    if (screen(app, menu).onExit != 0)
        GAME(at, screen(app, menu).onExit, {menu, action, menu});
    g.eax = menu;
    NEEDED(hidden, hideItemsCore(g, entered(at), menu));
    g.eax = u32(*hidden);
    return 1;
}

/* 0x44af90's, 0x44b040's: the answers of the name dialogs the loop opens
 * when a race is started with no name for player 1 (or, split screen, player
 * 2): Enter (or OK) keeps what was typed -- "Player 1"/"Player 2" if nothing
 * -- and starts the race (5); Cancel or Escape clears it (2). */
Result nameAnswerCore(Game& g, u32 esp, u32 answer, u32 close, u32 name, u32 defaultText)
{
    App* app = g.app;
    const u32 at = esp - 16;
    setByte(app, close, 0);
    u32 result = 0;
    if (answer == kDialogEnter)
    {
        GAME_DIRECT(0x4446c0, frontend::dialogs::focusedButton, at);
        const u32 focus = g.eax;
        if (focus == 0 || kInputFromMouse(app) == 0)
        {
            GAME_DIRECT(0x444c10, frontend::dialogs::editText, at);
            u32 typed = g.eax;
            GAME_DIRECT(kNameValid, frontend::text::nameValid, at, typed);
            if (g.eax == 0)
            {
                GAME_DIRECT(0x4d1850, text::lineText, at, entered(at), int32_t(defaultText), u32(0));
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
Result quitAnswerCore(Game& g, u32 esp, u32 answer, u32 close)
{
    App* app = g.app;
    const u32 at = esp - 8;
    if (answer < kDialogEnter)
        return 0;
    if (answer == kDialogEnter)
    {
        GAME_DIRECT(0x4446c0, frontend::dialogs::focusedButton, at);
        const u32 focus = g.eax;
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
Result drawHelpCore(Game& g, u32 esp)
{
    App* app = g.app;
    const u32 at = esp - 24 - 8;
    kHelpAlpha.set(app, (kDisplayFlags(app) & kNoTranslucency) != 0 ? kFloatOne : kFloatPoint9);
    g.eax = kCurrentMenu(app);
    bool restart = kCurrentMenu(app) != kHelpMenu(app);
    if (!restart)
    {
        GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, at);
        restart = (g.eax & 0xff) != 0;
    }
    if (restart)
    {
        kHelpNeedsMove.set(app, 1);
        g.eax = kCurrentMenu(app);
        kHelpMenu.set(app, g.eax);
        kHelpText.set(app, 0);
        kHelpLastX.set(app, kMouseX(app));
        kHelpItem.set(app, u32(-1));
        kHelpLastY.set(app, kMouseY(app));
    }
    NEEDED(taken, keysTakenCore(g, entered(at)));
    g.eax = u32(*taken);
    if (*taken != 0)
        return int32_t(g.eax);

    NEEDED(selected, selectedIndexCore(g));
    g.eax = u32(*selected);
    const u32 index = u32(*selected);
    bool timing = true;
    GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, at);
    if ((g.eax & 0xff) != 0 || index == u32(-1) || index != kHelpItem(app))
        kHelpItem.set(app, u32(-1));
    else if (kHelpItem(app) != u32(-1))
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
        else if (int32_t(kTicks(app)) > int32_t(kHelpDue(app)) && index != u32(-1) && kHelpNeedsMove(app) == 0)
        {
            // The box: the text's size, wrapped, and a margin.
            kHelpItem.set(app, index);
            kHelpFade.set(app, 0);
            kHelpText.set(app, item(app, itemAt(int32_t(index))).help);
            const u32 width = at, height = at + 4;  // [ebp - 8], [ebp - 4]
            GAME(at, kTextBox, {kHelpText(app), kHelpFont, kHelpWrap, width}, {height, 0});
            const int32_t w = int32_t(word(app, width)) + 7, h = int32_t(word(app, height)) + 3;
            kHelpX.set(app, kMouseX(app));
            kHelpWidth.set(app, u32(w));
            kHelpHeight.set(app, u32(h));
            if (int32_t(kMouseX(app)) + w > 630)
                kHelpX.set(app, u32(630 - w));
            kHelpY.set(app, kMouseY(app) + 20);
            if (int32_t(kMouseY(app)) + 20 + h > 460)
                kHelpY.set(app, u32(int32_t(kMouseY(app)) - h - 8));
        }
    }

    const u32 helpText = kHelpText(app);
    if (helpText != 0 && byteAt(app, helpText) == 0)
        kHelpText.set(app, 0);
    if (kHelpText(app) != 0)
    {
        if (kHelpItem(app) == u32(-1))
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

    const u32 x = kHelpX(app), y = kHelpY(app);
    const u32 w = kHelpWidth(app), h = kHelpHeight(app);
    const u32 alpha = kHelpFade(app) << 24;
    kDrawColour.set(app, alpha);
    GAME_DIRECT(kFillBox, draw2d::gradient, at, at - 4 - 4 * 2, int32_t(x), int32_t(y),
                int32_t(x + w), int32_t(y + h), u32(-100), kHelpBoxColour);
    GAME_DIRECT(kBoxBorder, draw2d::outline, at, at - 4 - 4, int32_t(x), int32_t(y), int32_t(w),
                int32_t(h), alpha);
    if (kInRace(app) != 0)
        GAME_DIRECT(kDrawTextInRace, text::menuTextDrawn, at, at - 4 - 4 * 2,
                    kHelpText(app), int32_t(x), int32_t(y), kHelpFont, 0, kHelpTextColour);
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
#define ALLOW(option, on) GAME_DIRECT(0x43a6e0, frontend::loada::carFilterLists, at, u32(option), u32(on))

Result fitRaceSettingsCore(Game& g, u32 esp)
{
    App* app = g.app;
    const u32 at = esp - 20;
    if (kAfterRace(app) != 0)
        return int32_t(g.eax);
    if (kSeen4d0(app) == u32(-1))
    {
        kSeen4d0.set(app, gameSettings(app).trafficLevel);
        kSeen4d4.set(app, gameSettings(app).opponentClass);
    }
    if (kSeenRaceType(app) == u32(-1))
        kSeenRaceType.set(app, gameSettings(app).raceType);
    gameSettings(app).soundFlag98 = gameSettings(app).musicMode == 0 ? 1 : 0;
    if (gameSettings(app).soundSetup == 1)
        kSetting292c.set(app, 0);
    else if (gameSettings(app).soundSetup == 0)
        kSetting292c.set(app, 1);
    if (gameSettings(app).raceType == kHotPursuit)
        gameSettings(app).trafficLevel = 2;

    const u32 first = gameSettings(app).trafficLevel;
    if (first != kSeen4d0(app))
    {
        kSeen4d0.set(app, first);
        auto setSecond = [&](u32 value) {
            kSeen4d4.set(app, value);
            gameSettings(app).opponentClass = value;
        };
        if (first == 0 && gameSettings(app).opponentClass != 0)
            setSecond(0);
        else if (first == 1 && gameSettings(app).opponentClass == 0)
            setSecond(4);
        else if (first == 2 && gameSettings(app).opponentClass == 0)
            setSecond(1);
    }
    else if (gameSettings(app).opponentClass != kSeen4d4(app))
    {
        const u32 second = gameSettings(app).opponentClass;
        kSeen4d4.set(app, second);
        auto setFirst = [&](u32 value) {
            kSeen4d0.set(app, value);
            gameSettings(app).trafficLevel = value;
        };
        if (second == 0)
            setFirst(0);
        else if (second <= 3)
        {
            if (gameSettings(app).trafficLevel == 0)
                setFirst(2);
        }
        else if (gameSettings(app).trafficLevel == 0)
            setFirst(1);
    }
    if (gameSettings(app).raceType != kSeenRaceType(app))
        kSeenRaceType.set(app, gameSettings(app).raceType);

    ALLOW(0xd, 0);
    ALLOW(0x4004, 1);
    if (int32_t(gameSettings(app).playMode) >= 2)
    {
        if (gameSettings(app).raceType == kTournament || gameSettings(app).raceType == kKnockout)
        {
            if (gameSettings(app).setting504 == 3)
                gameSettings(app).setting504 = 0;
            kSetting639c.set(app, 0);
        }
        else
            kSetting639c.set(app, 1);
        ALLOW(0x40004, 1);
        switch (gameSettings(app).setting504)
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
    if (gameSettings(app).raceType == kTournament || gameSettings(app).raceType == kKnockout)
    {
        gameSettings(app).noTraffic = 1;
        gameSettings(app).trafficLevel = 2;
        ALLOW(0x884, 1);
        switch (gameSettings(app).opponentClass)
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
    if (gameSettings(app).raceType != kHotPursuit)
        ALLOW(0x4008, 1);
    else
    {
        ALLOW(0x2008, 1);
        const int32_t players = int32_t(gameSettings(app).playMode);
        bool cops;
        if (players >= 2)
            cops = kNetworkSetting2276(app) == int32_t(kNetworkSettingE5d0(app));
        else if (players == 1)
        {
            GAME_DIRECT(kCarFlag10, load::policeFlag, at,
                        gameSettings(app).players[0].car);
            cops = g.eax != 0;
            if (cops)
            {
                GAME_DIRECT(kCarFlag10, load::policeFlag, at,
                            gameSettings(app).players[1].car);
                cops = g.eax != 0;
            }
        }
        else
            cops = true;
        ALLOW(u32(cops ? 0x2084 : 0xf04), 1);
    }

    // The car screens: the name of the one shown, against each.
    auto shown = [&](u32 name, bool& is) -> bool {
                if (!g.direct<kCurrentMenuName, frontend::loop::currentMenuName>(at))
            return false;
        if (!g.direct<kStrcmp, watcom::clib::strcmp>(at, g.eax, name))
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

    auto widen = [](u32 value) { return u32(int32_t(int16_t(value))); };
    if (gameSettings(app).raceType == kTournament)
    {
        const u32 kind = int32_t(gameSettings(app).playMode) > 1 ? 2 : 4;
        GAME_DIRECT(0x4cf160, frontend::menue::lapsToOption, at, kind);
        gameSettings(app).lapsClass = widen(g.eax);
        gameSettings(app).track = kValue5d004(app);
    }
    else if (gameSettings(app).raceType == kKnockout)
    {
        GAME_DIRECT(0x4cf160, frontend::menue::lapsToOption, at, 2);
        gameSettings(app).lapsClass = widen(g.eax);
        gameSettings(app).track = 0;
    }
    GAME_DIRECT(0x4cf190, frontend::menue::optionToLaps, at,
                u32(int32_t(int16_t(gameSettings(app).lapsClass))));
    gameSettings(app).laps = widen(g.eax);
    kSeen4d0.set(app, gameSettings(app).trafficLevel);
    kSeen4d4.set(app, gameSettings(app).opponentClass);
    if (gameSettings(app).unlockedAll != 0)
        GAME_DIRECT(0x442b60, frontend::misc::postgameUnlockTrack, at, 8);
    return int32_t(g.eax);
}

#undef ALLOW

/* A dialog over the screen asking for a player's name, when a race is
 * started without one: the default name ("Player 1") already in. */
bool askName(Game& g, u32 at, u32 name, u32 title, u32 defaultText, u32 answer)
{
    App* app = g.app;
    if (!g.direct<0x4d1850, text::lineText>(at, entered(at), int32_t(title), u32(0)))
        return false;
    setWord(app, kDialogLines, g.eax);
    if (!g.direct<0x4d1850, text::lineText>(at, entered(at), int32_t(kNameDialogLine2), u32(0)))
        return false;
    setWord(app, kDialogLines + 4, g.eax);
    if (!g.direct<0x4d1850, text::lineText>(at, entered(at), int32_t(kNameDialogOk), u32(0)))
        return false;
    setWord(app, kDialogButtons, g.eax);
    if (!g.direct<0x4d1850, text::lineText>(at, entered(at), int32_t(kNameDialogCancel), u32(0)))
        return false;
    setWord(app, kDialogButtons + 4, g.eax);
    if (!g.direct<0x4d1850, text::lineText>(at, entered(at), int32_t(defaultText), u32(0)))
        return false;
    copyString(app, name, g.eax);
    return g.direct<0x444c20, frontend::dialogs::openEdit>(at - 4 * 5, 2u, kDialogLines, 2u, kDialogButtons, 0u, name, kPlayerNameLength, 1u, answer);
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
Result menuLoopCore(Game& g, u32 esp, u32 name, u32 history)
{
    App* app = g.app;
    const u32 at = esp - 20 - 0x10;
    const u32 action = at;        // [ebp - 0x10]: 0x44b1a0 sets it
    const u32 offset = at + 4;    // [ebp - 0xc]
    bool running = true, firstScreen = true;
    setWord(app, action, 0);
    setWord(app, offset, 0);

    GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, name, kPostGame);
    kAfterRace.set(app, g.eax == 0 ? 1 : 0);
    kInRace.set(app, 0);
    kFlagEb10.set(app, 0);
    kFrontendArt.set(app, 0);
    GAME_DIRECT(kSetDisplay, text::loadedLanguageText, at, entered(at),
                int32_t(gameSettings(app).language));
    GAME(at, kResetInput);
    const u32 afterRace = kAfterRace(app);
    const u32 tune = afterRace != 0 ? gameSettings(app).track : u32(0x16);
    GAME_DIRECT(kStartMusic, musicStarted, at, entered(at),
                int32_t(tune), int32_t(gameSettings(app).menuMusicVolume), int32_t(afterRace));
    GAME_DIRECT(kLoadMenus, menus::menusLoaded, at, entered(at), name, u32(1), Args{name, 1});
    kLastAction.set(app, 5);
    for (u32 i = 0; i < kHistorySize; ++i)
        kHistory.set(app, int32_t(i), 0);
    if (history != 0)
    {
        g.eax = history;
        NEEDED(set, setHistoryCore(g, entered(at), history));
        g.eax = u32(*set);
    }
    else
        kCurrentMenu.set(app, kScreens.at);

    while (running)
    {
        g.eax = kCurrentMenu(app);
        NEEDED(opened, enterScreenCore(g, entered(at), kCurrentMenu(app)));
        g.eax = u32(*opened);
        setWord(app, action, g.eax);
        if (firstScreen)
        {
            g.eax = kCurrentMenu(app);
            NEEDED(next, selectNextCore(g, entered(at), kCurrentMenu(app)));
            g.eax = u32(*next);
            firstScreen = false;
        }
        if (word(app, action) != 0)
            setWord(app, action, 3);
        kScreenUp.set(app, 1);
        GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundEnter), int32_t(kVolumeFull));

        while (word(app, action) == 0)
        {
            GAME_DIRECT(kPollEvents, descriptionServed, at, entered(at));
            const u32 menu = kCurrentMenu(app);
            if (screen(app, menu).onFrame != 0)
            {
                GAME(at, screen(app, menu).onFrame, {menu, menu});
                setWord(app, action, g.eax);
            }
            if (diag::takeAutoReplay && word(app, action) == 0 && diag::takeAutoReplay())
            {
                GAME_DIRECT(0x44e710, frontend::misc::postgameViewReplayClicked, at);
                setWord(app, action, g.eax);
            }
            NEEDED(fitted, fitRaceSettingsCore(g, entered(at)));
            g.eax = u32(*fitted);
            if (word(app, action) == 0)
            {
                GAME_DIRECT(kBeginFrame, graphics::windowTakenAndCleared, at);
                g.eax = kCurrentMenu(app);
                NEEDED(drawn, drawItemsCore(g, entered(at), kCurrentMenu(app)));
                g.eax = u32(*drawn);
                setWord(app, action, g.eax);
                GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, at);
                if ((g.eax & 0xff) != 0)
                {
                    GAME_DIRECT(kDialogPass, dialogs::dialogPassOf, at, entered(at), 0u);
                    setWord(app, action, g.eax);
                }
                g.eax = kCurrentMenu(app);
                NEEDED(passed, passItemsCore(g, entered(at), kCurrentMenu(app)));
                g.eax = u32(*passed);
                GAME(at, kPresentFrame);
                const uint8_t noHelp = kNoHelpFlags(app);
                if ((noHelp & 0x10) == 0 && (noHelp & 0x20) == 0)
                {
                    NEEDED(helped, drawHelpCore(g, entered(at)));
                    g.eax = u32(*helped);
                }
                GAME_DIRECT(kEndFrame, graphics::flipAndWait, at);
            }
            if (word(app, action) == 0)
            {
                // The network screens hold the mouse only while connected with no network race set up.
                if (kNetworkGame(app) != 0 || kNetworkConnected(app) == 0)
                    kMouseBlocked.set(app, 0);
                g.eax = kCurrentMenu(app);
                NEEDED(under, itemUnderMouseCore(g, entered(at), kCurrentMenu(app)));
                g.eax = u32(*under);
                const u32 underMouse = u32(*under);
                const u32 current = kCurrentMenu(app);
                const int16_t was = screen(app, current).selected;
                bool ok = false;
                if (u32(int32_t(was)) != underMouse)
                {
                    NEEDED(selectedOk, selectableCore(g, entered(at), int32_t(was), current));
                    g.eax = u32(*selectedOk);
                    ok = *selectedOk != 0;
                }
                if (ok)
                {
                    GAME_DIRECT(kItemChanged, frontend::menuc::resetButtonPulse, at);
                    const u32 shown = kCurrentMenu(app);
                    screen(app, shown).previous = screen(app, shown).selected;
                }
                screen(app, kCurrentMenu(app)).selected = underMouse == u32(-1) ? kNoItem : int16_t(underMouse);
                GAME_DIRECT(kReadKey, text::menuKeyOf, at, entered(at), 1u);
                const uint16_t key = uint16_t(g.eax);
                if (key != 0)
                {
                    const u32 code = u32(int32_t(int16_t(key)));
                    GAME_DIRECT(kDialogShown, dialogs::dialogShownOf, at);
                    if ((g.eax & 0xff) != 0)
                    {
                        GAME_DIRECT(kDialogKey, dialogs::dialogKeyOf, at, entered(at), code);
                        setWord(app, action, g.eax);
                    }
                    else
                    {
                        g.eax = kCurrentMenu(app);
                        NEEDED(handled, handleKeyCore(g, entered(at), kCurrentMenu(app), code));
                        g.eax = u32(*handled);
                        setWord(app, action, g.eax);
                    }
                    g.eax = kCurrentMenu(app);
                    NEEDED(waited, waitWhileSuspendedCore(g, entered(at), kCurrentMenu(app), action));
                    g.eax = u32(*waited);
                    if (*waited != 0)
                        continue;
                }
            }

            if (word(app, action) == kQuit && kQuitting(app) == 0 && kNoQuitQuestion(app) == 0)
            {
                GAME_DIRECT(0x4d1850, text::lineText, at, entered(at), int32_t(kQuitQuestion), u32(0));
                setWord(app, kDialogLines, g.eax);
                GAME_DIRECT(0x4d1850, text::lineText, at, entered(at), int32_t(kQuitYes), u32(0));
                setWord(app, kDialogButtons, g.eax);
                GAME_DIRECT(0x4d1850, text::lineText, at, entered(at), int32_t(kQuitNo), u32(0));
                setWord(app, kDialogButtons + 4, g.eax);
                GAME_DIRECT(0x4446d0, frontend::dialogs::openQuestion, at - 4 * 4, 1u, kDialogLines, 2u, kDialogButtons, 0u, 0u, 0u, kQuitAnswerAt);
                setWord(app, action, 0);
                continue;
            }
            if (word(app, action) == kGoOn)
            {
                if (std::uint8_t(gameSettings(app).players[0].name[0]) == 0)
                {
                    if (!askName(g, at, SETTINGS_ADDRESS(players[0].name), kPlayer1NameTitle, kPlayer1Default, kPlayer1NameAnswerAt))
                        return {};
                    setWord(app, action, 0);
                }
                else if (gameSettings(app).playMode == 1 && std::uint8_t(gameSettings(app).players[1].name[0]) == 0)
                {
                    if (!askName(g, at, SETTINGS_ADDRESS(players[1].name), kPlayer2NameTitle, kPlayer2Default, kPlayer2NameAnswerAt))
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
            GAME_DIRECT(kFree, ea::purgeBlock, at, kScreenShapes(app));
            kScreenShapes.set(app, 0);
        }
        kScreenUp.set(app, 0);
        if (kNetworkConnected(app) != 0)
            GAME(at, kNetworkLeft);
        kLastAction.set(app, word(app, action));
        GAME_DIRECT(0x4534d0, frontend::menub::carViewRemember, at,
                    screen(app, kCurrentMenu(app)).name);

        bool back = false;
        switch (word(app, action))
        {
        case kBackToFirst:
        {
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = u32(*left);
            while (kCurrentMenu(app) != kScreens.at)
            {
                NEEDED(popped, popMenuCore(g));
                g.eax = u32(*popped);
            }
            break;
        }
        case kGoOn:
        {
            const u32 menu = kCurrentMenu(app);
            const u32 it = menuItem(app, menu, screen(app, menu).selected);
            g.eax = menu;
            NEEDED(escape, escapeButtonCore(g, entered(at), menu, offset));
            g.eax = u32(*escape);
            if (u32(*escape) == it)
            {
                setWord(app, action, 3);
                back = true;
                break;
            }
            const u32 next = item(app, it).type == kButton ? item(app, it).nextMenu : 0;
            if (next != 0)
            {
                for (setWord(app, offset, 0); int32_t(word(app, offset)) < kScreenCount(app);
                     setWord(app, offset, word(app, offset) + 1))
                {
                    GAME_DIRECT(kStrcmp, watcom::clib::strcmp, at, next, screen(app, kScreens.address(int32_t(word(app, offset)))).name);
                    if (g.eax == 0)
                        break;
                }
                if (int32_t(word(app, offset)) < kScreenCount(app))
                {
                    NEEDED(pushed, pushMenuCore(g));
                    g.eax = u32(*pushed);
                    NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
                    g.eax = u32(*left);
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
            GAME_DIRECT(kPlaySound, playMenuEffectOf, at, entered(at), int32_t(kSoundStart), int32_t(kVolumeFull));
            GAME_DIRECT(kBeginFrame, graphics::windowTakenAndCleared, at);
            g.eax = kCurrentMenu(app);
            NEEDED(drawn, drawItemsCore(g, entered(at), kCurrentMenu(app)));
            g.eax = u32(*drawn);
            running = false;
            kScreenUp.set(app, 1);
            GAME_DIRECT(kSetCursor, frontend::connect::setPointer, at, 3u);
            GAME(at, kPresentFrame);
            GAME_DIRECT(kEndFrame, graphics::flipAndWait, at);
            kScreenUp.set(app, 0);
            kFlagEb10.set(app, 0);
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = u32(*left);
            break;
        }
        case kMoviePlayed:
        {
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = u32(*left);
            if (kAfterMovieScreen(app) != 0)
            {
                g.eax = kAfterMovieScreen(app);
                NEEDED(found, menuByNameCore(g, entered(at), kAfterMovieScreen(app)));
                g.eax = u32(*found);
                if (*found != 0)
                {
                    g.eax = kAfterMovieScreen(app);
                    NEEDED(again, menuByNameCore(g, entered(at), kAfterMovieScreen(app)));
                    g.eax = u32(*again);
                    kCurrentMenu.set(app, u32(*again));
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
            g.eax = u32(*left);
            running = false;
            break;
        }
        case kOut:
        {
            kFlagEb10.set(app, 0);
            running = false;
            g.eax = kCurrentMenu(app);
            NEEDED(left, leaveScreenCore(g, entered(at), kCurrentMenu(app), word(app, action)));
            g.eax = u32(*left);
            break;
        }
        default:
            break;
        }
        if (back || word(app, action) == 3 || word(app, action) == 1)
        {
            const u32 menu = kCurrentMenu(app);
            if (menu == kScreens.at)
            {
                setWord(app, action, kOut);
                running = false;
            }
            else
            {
                g.eax = menu;
                NEEDED(left, leaveScreenCore(g, entered(at), menu, word(app, action)));
                g.eax = u32(*left);
                NEEDED(popped, popMenuCore(g));
                g.eax = u32(*popped);
            }
        }
    }

    GAME(at, kCheckMenuFiles);
    if (kFrontendArt(app) != 0)
    {
        GAME_DIRECT(kFree, ea::purgeBlock, at, kFrontendArt(app));
        kFrontendArt.set(app, 0);
    }
    GAME(at, kMenusEnd);
    GAME_DIRECT(kCloseDialog, dialogs::closeDialogOf, at, 0u);
    GAME_DIRECT(kStopMusic, musicStopped, at, entered(at), u32(0));
    return int32_t(word(app, action));
}

}  // namespace

/* ---- The cores the menus call as C++ (port_loop.h): a core above with the
 * Game it runs in made from `eax`, what the game's code has in eax at the
 * call (the first register argument, where there is one) ---- */

Result keysTakenOf(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    Game g{app, ts, eax};
    return keysTakenCore(g, esp);
}

Result selectableOf(App* app, ThreadState& ts, u32 esp, int32_t offset, u32 menu)
{
    Game g{app, ts, u32(offset)};
    return selectableCore(g, esp, offset, menu);
}

Result pointAtSelectedOf(App* app, ThreadState& ts, u32 esp, u32 menu)
{
    Game g{app, ts, menu};
    return pointAtSelectedCore(g, esp, menu);
}

Result escapeButtonOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 offset)
{
    Game g{app, ts, menu};
    return escapeButtonCore(g, esp, menu, offset);
}

Result handleKeyOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 key)
{
    Game g{app, ts, menu};
    return handleKeyCore(g, esp, menu, key);
}

Result drawItemsOf(App* app, ThreadState& ts, u32 esp, u32 menu)
{
    Game g{app, ts, menu};
    return drawItemsCore(g, esp, menu);
}

Result itemUnderMouseOf(App* app, ThreadState& ts, u32 esp, u32 menu)
{
    Game g{app, ts, menu};
    return itemUnderMouseCore(g, esp, menu);
}

Result passItemsOf(App* app, ThreadState& ts, u32 esp, u32 menu)
{
    Game g{app, ts, menu};
    return passItemsCore(g, esp, menu);
}

Result leaveScreenOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 action)
{
    Game g{app, ts, menu};
    return leaveScreenCore(g, esp, menu, action);
}

Result enterScreenOf(App* app, ThreadState& ts, u32 esp, u32 menu)
{
    Game g{app, ts, menu};
    return enterScreenCore(g, esp, menu);
}

Result popMenuOf(App* app, ThreadState& ts, u32 eax)
{
    Game g{app, ts, eax};
    return popMenuCore(g);
}

Result pushMenuOf(App* app, ThreadState& ts, u32 eax)
{
    Game g{app, ts, eax};
    return pushMenuCore(g);
}

Result menuLoopOf(App* app, ThreadState& ts, u32 esp, u32 name, u32 history)
{
    Game g{app, ts, name};
    return menuLoopCore(g, esp, name, history);
}

/* 0x44aeb0's: the screen shown. */
u32 currentMenuOf(App* app)
{
    return kCurrentMenu(app);
}

/* ---- The thunks ---- */

/* 0x4499b0: whether the item at eax can be used. */
nfs::u32 itemUsable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, itemUsableCore(g, e.esp, e.eax));
}

/* 0x4499f0: whether a key (edx) may act on the screen at eax. */
nfs::u32 keyAllowed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, keyAllowedCore(g, e.esp, e.eax, uint16_t(e.edx)));
}

/* 0x449aa0: the help box. */
nfs::u32 drawHelp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, drawHelpCore(g, e.esp));
}

/* 0x449df0: whether the keys go to something other than the screen. */
nfs::u32 keysTaken(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, keysTakenOf(app, ts, e.esp, e.eax));
}

/* 0x449e50: whether the item eax of the screen edx can be selected. */
nfs::u32 selectable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, selectableOf(app, ts, e.esp, int32_t(e.eax), e.edx));
}

/* 0x449ea0: the pointer moved onto the selected item of the screen at eax. */
nfs::u32 pointAtSelected(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, pointAtSelectedOf(app, ts, e.esp, e.eax));
}

/* 0x449f30: the item before the selected one of the screen at eax selected. */
nfs::u32 selectPrevious(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, selectPreviousCore(g, e.esp, e.eax));
}

/* 0x44a0c0: the item after the selected one of the screen at eax selected. */
nfs::u32 selectNext(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, selectNextCore(g, e.esp, e.eax));
}

/* 0x44a240: the Escape button of the screen at eax, its index into edx. */
nfs::u32 escapeButton(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, escapeButtonOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x44a2d0: the key edx on the screen at eax. */
nfs::u32 handleKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, handleKeyOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x44a440: the selected item of the screen shown. */
nfs::u32 selectedIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, selectedIndexCore(g));
}

/* 0x44a470: the item at eax selected, if the screen shown has it. */
nfs::u32 selectItem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, selectItemCore(g, e.esp, e.eax));
}

/* 0x44a4f0: the items of the screen at eax drawn. */
nfs::u32 drawItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, drawItemsOf(app, ts, e.esp, e.eax));
}

/* 0x44a690: the mouse let go of the item it held. */
nfs::u32 releaseMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, releaseMouseCore(g));
}

/* 0x44a6b0: the item of the screen at eax under the mouse. */
nfs::u32 itemUnderMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, itemUnderMouseOf(app, ts, e.esp, e.eax));
}

/* 0x44a860: every item of the screen at eax told it is up. */
nfs::u32 itemsShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, itemsShownCore(g, e.esp, e.eax));
}

/* 0x44a8b0: a pass of the items of the screen at eax. */
nfs::u32 passItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, passItemsOf(app, ts, e.esp, e.eax));
}

/* 0x44a9e0: every item of the screen at eax told it is hidden. */
nfs::u32 hideItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, hideItemsCore(g, e.esp, e.eax));
}

/* 0x44aa30: the screen at eax left, how (edx). */
nfs::u32 leaveScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, leaveScreenOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x44aad0: the items of the screen at eax back to their widgets' states. */
nfs::u32 resetItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, resetItemsCore(g, e.eax));
}

/* 0x44ab40: the screen at eax opened. */
nfs::u32 enterScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, enterScreenOf(app, ts, e.esp, e.eax));
}

/* 0x44ae60: the screen named eax. */
nfs::u32 menuByName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, menuByNameCore(g, e.esp, e.eax));
}

/* 0x44aeb0: the screen shown. */
nfs::u32 currentMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, currentMenuOf(app));
}

/* 0x44aec0: back to the screen gone from last. */
nfs::u32 popMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, popMenuOf(app, ts, e.eax));
}

/* 0x44af10: the screen shown onto the history. */
nfs::u32 pushMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, pushMenuOf(app, ts, e.eax));
}

/* 0x44af50: the history set from the names at eax. */
nfs::u32 setHistory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, setHistoryCore(g, e.esp, e.eax));
}

/* 0x44af90: the answer eax of the dialog asking for player 1's name; edx the
 * byte that closes it. */
nfs::u32 player1NameAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, nameAnswerCore(g, e.esp, e.eax, e.edx, SETTINGS_ADDRESS(players[0].name), kPlayer1Default));
}

/* 0x44b040: and player 2's. */
nfs::u32 player2NameAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, nameAnswerCore(g, e.esp, e.eax, e.edx, SETTINGS_ADDRESS(players[1].name), kPlayer2Default));
}

/* 0x44b0f0: the answer eax of the quit question; edx the byte that closes it. */
nfs::u32 quitAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, quitAnswerCore(g, e.esp, e.eax, e.edx));
}

/* 0x44b1a0: the game suspended waited out; edx where the action is kept. */
nfs::u32 waitWhileSuspended(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, waitWhileSuspendedCore(g, e.esp, e.eax, e.edx));
}

/* 0x44b230: the menus from the screen named eax, edx the history. */
nfs::u32 menuLoop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, menuLoopOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x44be60: the race settings kept consistent. */
nfs::u32 fitRaceSettings(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Game g{app, ts, e.eax};
    return e.finish(ts, args, fitRaceSettingsCore(g, e.esp));
}

}  // namespace loop
}  // namespace port
}  // namespace nfs3hp
