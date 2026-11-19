#include "game_settings.h"
#include "native_load_cardb.h"
#include "native_load_saves.h"
#include "native_raceload.h"
#include "port_buttons.h"
#include "port_gamecall.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_simd.h"
#include "port_draw2d.h"
#include "port_text.h"
#include "native_frontend_glue.h"
#include "native_frontend_menud.h"
#include "native_frontend_menuf.h"
#include "native_frontend_misc.h"
#include "native_frontend_pause.h"
#include "native_frontend_screen.h"
#include "native_graphics.h"
#include "native_watcom.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_frontend_connect.h"
#include "native_frontend_draw.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_loop.h"
#include "native_frontend_menus.h"
#include "native_frontend_wata.h"
#include "native_vertices.h"
#include "port_dialogs.h"
#include "port_loop.h"
#include "port_menus.h"
#include "port_shapes.h"
#include "port_sound_effects.h"
#include "port_sound_speech.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* The menus' widgets, rewritten (port.h says how).
 *
 * A screen's items (0xbc bytes each, the screens' table at 0x604ee0) are
 * widgets of a kind, each kind a row of the widgets' table at 0x557a84 whose
 * template names its handlers: shown, hidden, drawn (edx: the item under the
 * pointer), a key (ebx), laid out ("up"), a pass of the menu loop.  Every
 * item has its flags (1: greyed out), its place, its codelink (a name the
 * screens' code looks for), the demo's mark, its box (top, left, width,
 * height: where the pointer finds it) and mostly a text; the rest is its
 * kind's (the records below).  Their lists (0x5563e8: a name, the lines --
 * the language's text numbers, words to a 0 -- which of them may be chosen,
 * their colours, where the line chosen is kept) are looked up by name.
 *
 * Each function is a typed core and a thunk under the game's name.  The
 * cores call each other as C++; the menus' text, the 2D drawing and the
 * shapes, which are the port's own (port_text, port_draw2d, port_shapes),
 * through their thunks as the game's code calls them; what is not
 * rewritten -- the menu loop and the dialogs, the pointer, the codelinks,
 * the graphics, EA's bitmaps, the sound's descriptions, the runtime -- by
 * address, from where the game's code has its stack, with the registers it
 * has there.  The x87's arithmetic is the game's to the bit: each product
 * and sum as the single-precision arithmetic has them, each value stored where and
 * as the code stores it, the integers chopped as __CHP and fistp make them,
 * the compares leaving the status word as they do.
 *
 * The game's code leaves ebx and edx changed in buttonKey and edx in
 * findList, drawReadyMarks and buttonDraw; no caller reads them after (by
 * liveness on nfs3.exe, the menu loop's calls through the widgets' table
 * included), so each thunk gives back every register but eax as the
 * caller had it. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{
namespace port
{
namespace buttons
{

namespace
{

using nfs::Float;

/* ---- The records ---- */

#pragma pack(push, 1)
/* What every item has. */
#define NFS3HP_ITEM_HEAD                                                                                      \
    uint8_t head[4];                                                                                         \
    uint8_t flags;      /* 0x04: 1 greyed out */                                                             \
    uint8_t moreFlags;  /* 0x05: 4 a list open */                                                            \
    int16_t x, y;       /* 0x06, 0x08 */                                                                     \
    uint8_t unused0a[6];                                                                                     \
    u32 codelink;     /* 0x10 */                                                                           \
    uint8_t unused14[4];                                                                                     \
    int16_t demoMark;   /* 0x18: what the demo's command line marks */                                       \
    int16_t boxTop, boxLeft, boxWidth, boxHeight; /* 0x1a: where the pointer finds it */                    \
    uint8_t unused22[0x1a];                                                                                  \
    u32 text;         /* 0x3c */

struct Item
{
    NFS3HP_ITEM_HEAD
};

/* A text: its look (1 framed, scaled or swept in), the 2D screens' nudge,
 * its alignment (3 placed by the race's table), its sweep started again,
 * its colour's number, its scale (a float's bits), the list whose line it
 * shows. */
struct TextItem
{
    NFS3HP_ITEM_HEAD
    int16_t look;      // 0x40
    int16_t nudge;     // 0x42
    int16_t align;     // 0x44
    int16_t place;     // 0x46
    int16_t restart;   // 0x48
    int16_t colour;    // 0x4a
    u32 scale;       // 0x4c
    u32 listName;    // 0x50
};

/* An image: the art it is drawn from, its shape, its alpha, its frame, how
 * far it has slid in. */
struct ImageItem
{
    NFS3HP_ITEM_HEAD
    u32 file;        // 0x40
    int16_t shape;     // 0x44
    int16_t unused46;  // 0x46
    int16_t alpha;     // 0x48
    int16_t framed;    // 0x4a
    int16_t slide;     // 0x4c
};

/* A button: the list it opens, its style's name and number (0x40 bytes each
 * at 0x559fe4), its style's shapes, how far it has slid in, its alignment,
 * the list whose line it shows and where, its handler. */
struct ButtonItem
{
    NFS3HP_ITEM_HEAD
    u32 nextMenu;    // 0x40
    u32 styleName;   // 0x44
    int16_t style;     // 0x48
    int16_t place;     // 0x4a
    int16_t shape;     // 0x4c
    int16_t glow;      // 0x4e
    int16_t pulse;     // 0x50
    int16_t slide;     // 0x52
    int16_t unused54;  // 0x54
    int16_t align;     // 0x56
    u32 listName;    // 0x58
    int16_t listAlign; // 0x5c (its low byte)
    int16_t listGap;   // 0x5e
    int16_t listX;     // 0x60
    int16_t listY;     // 0x62
    u32 handler;     // 0x64
};

/* A cascade: its place's number, whether it is open, its shapes, the lines it
 * shows at most, the line chosen, under the pointer, shown at once, the first
 * and last shown, its list. */
struct CascadeItem
{
    NFS3HP_ITEM_HEAD
    int16_t place;     // 0x40
    int16_t open;      // 0x42
    int16_t unused44;  // 0x44
    int16_t shapes[6]; // 0x46: 0x46 to 0x50
    int16_t chosen;    // 0x52
    int16_t most;      // 0x54
    int16_t shown;     // 0x56
    int16_t first;     // 0x58
    int16_t last;      // 0x5a
    int16_t marks[2];  // 0x5c
    u32 listName;    // 0x60
};

/* A drop list: whether it is open, its shapes, which art (0x52, 0x54), the
 * line chosen, the lines it shows at most, shown at once, the first and last
 * shown, its list. */
struct DropItem
{
    NFS3HP_ITEM_HEAD
    int16_t open;      // 0x40
    int16_t shapes[8]; // 0x42: 0x42 to 0x50
    int16_t wide;      // 0x52
    int16_t right;     // 0x54
    int16_t chosen;    // 0x56
    int16_t most;      // 0x58
    int16_t shown;     // 0x5a
    int16_t first;     // 0x5c
    int16_t last;      // 0x5e
    int16_t marks[2];  // 0x60
    u32 listName;    // 0x64
};

/* A select list: its lines' texts (8 bytes each at 0x70), how many, the one
 * chosen, the lines it shows, the first and last shown, the arrows' boxes,
 * the function told of a choice. */
struct SelectItem
{
    NFS3HP_ITEM_HEAD
    u32 empty;       // 0x40: what it shows with no line
    int16_t unused44a, unused44b;  // 0x44
    u32 count;       // 0x48
    u32 chosen;      // 0x4c
    int16_t lines;     // 0x50
    int16_t shown;     // 0x52
    int16_t first;     // 0x54
    int16_t last;      // 0x56
    int16_t shapes[4]; // 0x58
    int16_t upBox[4];  // 0x60
    int16_t downBox[4];  // 0x68
    u32 texts;       // 0x70
    u32 told;        // 0x74
};

/* A text box: whether its text is its own, the lines it takes, its width and
 * height, its first line and the lines shown. */
struct TextBoxItem
{
    NFS3HP_ITEM_HEAD
    int16_t owned;     // 0x40
    int16_t lines;     // 0x42
    int16_t width;     // 0x44
    int16_t height;    // 0x46
    int16_t gap;       // 0x48
    int16_t first;     // 0x4a
};

/* A rectangle: its corners' colours (the first where the others' text is),
 * its width and height, a frame's thickness, the function that draws over
 * it, its kind. */
struct RectItem
{
    NFS3HP_ITEM_HEAD
    u32 moreColours[3];  // 0x40
    int16_t width;     // 0x4c
    int16_t height;    // 0x4e
    int16_t thick;     // 0x50
    int16_t unused52;  // 0x52
    u32 over;        // 0x54
    u32 kind;        // 0x58
};

/* A check box: whether it is ticked, its text's place. */
struct CheckItem
{
    NFS3HP_ITEM_HEAD
    int16_t ticked;    // 0x40
    uint8_t labelled;  // 0x42
};
#undef NFS3HP_ITEM_HEAD

/* A list of the lists' table (0x14 bytes each, at 0x5563e8). */
struct List
{
    u32 name, lines, allowed, colours, chosen;
};
#pragma pack(pop)
static_assert(sizeof(Item) == 0x40, "an item's head is 0x40 bytes");
static_assert(offsetof(TextItem, listName) == 0x50, "a text's list at 0x50");
static_assert(offsetof(ImageItem, slide) == 0x4c, "an image's slide at 0x4c");
static_assert(offsetof(ButtonItem, handler) == 0x64, "a button's handler at 0x64");
static_assert(offsetof(CascadeItem, listName) == 0x60, "a cascade's list at 0x60");
static_assert(offsetof(DropItem, listName) == 0x64, "a drop list's list at 0x64");
static_assert(offsetof(SelectItem, told) == 0x74, "a select list's function at 0x74");
static_assert(offsetof(TextBoxItem, first) == 0x4a, "a text box's first line at 0x4a");
static_assert(offsetof(RectItem, kind) == 0x58, "a rectangle's kind at 0x58");
static_assert(sizeof(List) == 0x14, "a list is 0x14 bytes");

template <class T>
T& item(App* app, u32 at)
{
    return record<T>(app, at);
}

/* ---- The game's variables ---- */

constexpr Records<List> kLists{0x5563e8};
constexpr u32 kListNotFound = 0x5389fc;  // "list not found: %s\n"
// The HUD's art, hud.fsh, the language's name of it, kept loaded and by name.
constexpr u32 kHud = 0x539004;
constexpr u32 kLanguageHuds[] = {0x53900c, 0x539018, 0x539024, 0x539030, 0x53903c};
constexpr Var<u32> kHudShapes{0x55922c};
constexpr u32 kHudName = 0x662850;
// The screen shown, the menus over a race (its pause screens), the screens
// sliding in, the renderer's state bits (2 alpha, 0x40 ...).
constexpr Var<u32> kScreen{0x559230};
constexpr Var<int32_t> kOverRace{0x559234};
constexpr Var<int32_t> kSlidingIn{0x559c60};
constexpr Var<int32_t> kSlideMore{0x559c68};
constexpr Var<uint8_t> kRenderFlags{0x7a3a58};
constexpr Var<int32_t> kClock{0x79c1a4};
// The demo's command line (0x55eb2e: 0x20, 0x10; 0x55eb2f: 0x40), a demo on.
constexpr Var<uint8_t> kDemoMarks{0x55eb2e}, kDemoFlags{0x55eb2f};
constexpr Var<int32_t> kDemoOn{0x553038};
// The 2D screens' text: its nudge, its size.
constexpr Var<int32_t> kNudge{0x563aa0};
constexpr Var<u32> kScreenTextSize{0x559450};
// The network: a race over it, this machine the host, ready.
constexpr Var<int32_t> kNetworkRace{0x7a22e0};
constexpr Var<int16_t> kNetworkHost{0x7a22da};
// The races' places on the screen (x, y words, 4 bytes apart).
constexpr Table<int16_t> kPlaceX{0x559c6c, 4}, kPlaceY{0x559c94, 4};
constexpr Var<int32_t> kScreenMode{0x66296c};
// The buttons' styles (0x40 bytes each): name, x, y, shapes' names, colours.
constexpr u32 kStyles = 0x559fe4;
// The strings the code compares.
constexpr u32 kGlow = 0x53a954, kLicence = 0x53a95c, kMainEffects = 0x53a964, kTextEffects = 0x53a970;
constexpr u32 kNfsLogo = 0x53a97c, kTrackImage = 0x53a984, kThreeButtons = 0x53a990, kTwoButtons = 0x53a998;
constexpr u32 kBackground = 0x53a9a0, kBandEffects = 0x53a9a8, kFedataFile = 0x53a9b4, kTwoStrings = 0x53a934;
constexpr u32 kMapImage = 0x53a4f0, kMapImage2 = 0x53a93c, kScreenImage = 0x53a890;
constexpr u32 kEllipsisGerman = 0x53a9c0, kEllipsis = 0x53a9c8;
constexpr u32 kControlsScreen = 0x53a5fc, kGraphicsScreen = 0x53a65c, kSmallBack = 0x53a604;
constexpr u32 kRaceNow = 0x53a9cc, kExitProgram = 0x53a9e4, kHostOnly = 0x53aa04;
constexpr u32 kPercentS = 0x53aa10, kOpponentCars = 0x53a68c, kClassA = 0x53aa14, kClassB = 0x53aa1c;
constexpr u32 kClassC = 0x53aa24, kSecondCar = 0x53aa2c, kCars = 0x53aa34, kTracks = 0x53aa3c;
constexpr u32 kCountedText = 0x53aa44, kDontHide = 0x53aa50, kRecordTracks = 0x53aa9c;
constexpr u32 kReplays = 0x53aaa4, kReplayFile = 0x53aaac, kJoinGameName = 0x53aab0;
constexpr u32 kGameFolder = 0x7a2db4;
// The menus' text: its fonts (0x12 the menus', 0x24 the big one).
constexpr int32_t kMenuFont = 0x12, kBigFont = 0x24;
constexpr u32 kOne = 0x3f800000;
// The colours the widgets are drawn in.
constexpr u32 kYellow = 0xffe440, kBlue = 0xff649dfd, kGrey = 0x908080, kDark = 0x4a96;

/* ---- What they call ---- */

// The port's own: as the game's code calls them.
constexpr u32 kLanguageText = 0x4d1850;  // 0x4d1850
constexpr u32 kDrawMenuText = 0x452190;  // 0x452190 (stdcall: align, colour)
constexpr u32 kTextWidth = 0x452020;  // 0x452020
constexpr u32 kSelectFont = 0x452100;  // 0x452100
constexpr u32 kDimBackground = 0x451530;  // 0x451530 (stdcall: a float)
constexpr u32 kKeepTextScale = 0x4d1400;  // 0x4d1400
constexpr u32 kRestoreTextScale = 0x4d1410;  // 0x4d1410
constexpr u32 kCharacterWidth = 0x4d1540;  // 0x4d1540
constexpr u32 kCharacterOverhang = 0x4d1590;  // 0x4d1590
constexpr u32 kWrappedText = 0x451cd0;  // 0x451cd0
constexpr u32 kShadedBox = 0x4d8860;  // 0x4d8860
constexpr u32 kFrame = 0x4d8930;  // 0x4d8930
constexpr u32 kPanel = 0x4d89f0;  // 0x4d89f0
constexpr u32 kFieldBox = 0x4d8ac0;  // 0x4d8ac0
constexpr u32 kTileRectangle = 0x4d8b80;  // 0x4d8b80
constexpr u32 kShadedRectangle = 0x4d81b0;  // 0x4d81b0
constexpr u32 kTurnedText = 0x4da120;  // 0x4da120
constexpr u32 kTitleText = 0x4da1e0;  // 0x4da1e0
constexpr u32 kTexturedQuad = 0x4db000;  // 0x4db000
constexpr u32 kShapeByName = 0x4d5db0;  // 0x4d5db0
constexpr u32 kShapeSize = 0x4d66f0;  // 0x4d66f0
constexpr u32 kDrawArtShape = 0x4d7160;  // 0x4d7160
constexpr u32 kDrawShape = 0x4d7990;  // 0x4d7990
constexpr u32 kMenuEffect = 0x4181d0;  // 0x4181d0
// What is not rewritten: by address.
constexpr u32 kStrcmp = 0x4ee310, kStricmp = 0x4e9ec0, kStrstr = 0x4e0820, kStrncpy = 0x4e0e30;
constexpr u32 kSprintf = 0x4df690, kFree = 0x4e1890, kWaitForKey = 0x4f0120;
constexpr u32 kLoadQfs = 0x4956f0;
constexpr u32 kTakeWindow = 0x4d9120, kFlip = 0x4d9140, kDrawScreen = 0x44a4f0, kDrawScreenItems = 0x44aeb0;
constexpr u32 kDialogShown = 0x4438b0, kDrawDialog = 0x445f50;
constexpr u32 kCloseDialog = 0x444a60, kJoinNetworkGame = 0x49e800, kMenuShownNow = 0x44a660;
constexpr u32 kCodelinkMatches = 0x4429c0, kMenuFindItem = 0x442a40;
constexpr u32 kAllPlayersReady = 0x49ab60, kNetworkOk = 0x4cf090, kHelpShown = 0x449df0;
constexpr u32 kPointerLine = 0x49c1c0, kPointerInBox = 0x49c1f0, kSetPointer = 0x49bfc0;
constexpr u32 kRenderState = 0x431900, kSetClip = 0x4bebe0, kRectangleOut = 0x4beb20, kClipQuad = 0x4c12e0;
constexpr u32 kSine = 0x4ea930, kCosine = 0x4ea950;
constexpr u32 kSpokenStatus = 0x4c95e0, kDescriptionStart = 0x4c95f0;
constexpr u32 kMainEffectsDraw = 0x4a49c0, kTextEffectsDraw = 0x4a4410, kLogoDraw = 0x4a4c20;
constexpr u32 kBandEffectsDraw = 0x4a47c0, kMainEffectsStart = 0x4a4a10, kTextEffectsStart = 0x4a4390;
constexpr u32 kNewBitmap = 0x4ef190, kSetBitmap = 0x4eada0, kClearBitmap = 0x4ef1b0, kBitmapShape = 0x4ef1e0;
constexpr u32 kDrawBitmapShape = 0x4f0a10, kBitmapDone = 0x4ead90, kFreeBitmap = 0x4eff00;
// port_shapes' art's textures (0x4d6390), by address.
constexpr u32 kArtShapeTextures = 0x4d6390;

/* ---- Small things ---- */

u32 word(App* app, u32 at)
{
    return app->getMemory<u32>(at);
}

void setWord(App* app, u32 at, u32 value)
{
    app->getMemory<u32>(at) = value;
}

int16_t half(App* app, u32 at)
{
    return app->getMemory<int16_t>(at);
}

void setHalf(App* app, u32 at, int16_t value)
{
    app->getMemory<int16_t>(at) = value;
}

uint8_t byteAt(App* app, u32 at)
{
    return app->getMemory<uint8_t>(at);
}

float floatAt(App* app, u32 at)
{
    return app->getMemory<float>(at);
}

double doubleAt(App* app, u32 at)
{
    return app->getMemory<double>(at);
}

u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A word sign-extended, as movsx and sar 0x10 make it. */
int32_t wide(int16_t value)
{
    return int32_t(value);
}

/* The game's string functions, inline: a string copied (two bytes a turn,
 * the same bytes as strcpy), one appended, how long. */
u32 length(App* app, u32 s)
{
    u32 n = 0;
    while (byteAt(app, s + n) != 0)
        ++n;
    return n;
}

void copyString(App* app, u32 to, u32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = byteAt(app, from);
        app->getMemory<uint8_t>(to) = c;
        if (c == 0)
            break;
    }
}

void appendString(App* app, u32 to, u32 from)
{
    copyString(app, to + length(app, to), from);
}

/* The code's buffer of 0x100 zeros in the code (0x45cbd0, 0x45ccd0), copied
 * into the frame as the code copies it. */
void copyBlock(App* app, u32 to, u32 from, u32 bytes)
{
    for (u32 i = 0; i < bytes; i += 4)
        setWord(app, to + i, word(app, from + i));
}

/* idiv: the remainder, as the x86's. */
int32_t remainderOf(int32_t dividend, int32_t by)
{
    return int32_t(int64_t(dividend) % int64_t(by));
}

/* sar 1 after subtracting the sign: halved toward 0. */
int32_t halved(int32_t value)
{
    return (value - (value >> 31)) >> 1;
}

/* The demo marks an item greyed: its flag 1 set where the demo's command
 * line (0x20) marks the item (its mark's bit 1), or (0x10) any mark -- or
 * the demo is on with neither of them. */
void demoGreys(App* app, u32 at)
{
    Item& it = item<Item>(app, at);
    const uint8_t marks = kDemoMarks(app);
    if ((marks & 0x20) != 0 && (uint8_t(it.demoMark) & 1) != 0)
        return;
    if ((kDemoMarks(app) & 0x10) != 0 && it.demoMark != 0)
        return;
    const uint8_t now = kDemoMarks(app);
    if ((now & 0x20) != 0 || (now & 0x10) != 0)
        it.flags |= 1;
}

/* Whether the demo hides the widgets: its flag and on. */
bool demoHides(App* app)
{
    return (kDemoFlags(app) & 0x40) != 0 && kDemoOn(app) != 0;
}

/* A call into the game's code; false when it stopped. */
struct Caller
{
    App* app;
    ThreadState& ts;
    u32 r = 0;
    bool operator()(u32 esp, u32 target, Args a = {}, std::initializer_list<u32> stack = {})
    {
        return gameCall(app, ts, esp, target, a, stack, r);
    }

    /* A typed core called as C++ where the game's code calls its address: from `esp`, the core's frame base
     * esp - 4 - 4 * Words (Words the stack words the game's call pushes); its answer in r; false when the game
     * stopped in it. */
    template <class F>
    bool run(u32 esp, F f)
    {
        ts.sp = esp;
        if constexpr (std::is_void_v<decltype(f())>)
        {
            f();
            return !ts.terminate;
        }
        else
        {
            const auto v = f();
            if (ts.terminate)
                return false;
            return store(v);
        }
    }
    template <class V>
    bool store(const std::optional<V>& v)
    {
        if (!v)
            return false;
        r = u32(*v);
        return true;
    }
    template <class V>
    bool store(const V& v)
    {
        r = u32(v);
        return true;
    }
    template <u32 Address, auto Core, unsigned Words, class... P>
    bool core(u32 esp, P... p)
    {
        return run(esp, [&] { return guest::direct<Address, Core>(app, ts, u32(esp - 4 - 4 * Words), p...); });
    }
    /* A core with no esp (and maybe no ts): the runtime's, the plain ones. */
    template <u32 Address, auto Core, class... P>
    bool plain(u32 esp, P... p)
    {
        return run(esp, [&] { return guest::direct<Address, Core>(app, ts, p...); });
    }
    /* The same of one that leaves eax as it came: r is `eax` after it. */
    template <u32 Address, auto Core, class... P>
    bool keep(u32 esp, u32 eax, P... p)
    {
        r = eax;
        return run(esp, [&] { return guest::direct<Address, Core>(app, ts, p...); });
    }
    /* A core that gives al: the rest of eax as it came. */
    template <u32 Address, auto Core, class... P>
    bool low(u32 esp, u32 eax, P... p)
    {
        const bool ok = run(esp, [&] { return guest::direct<Address, Core>(app, ts, p...); });
        r = (eax & 0xffffff00u) | (r & 0xff);
        return ok;
    }
    /* A box core whose colours the game's code pushes (four stack words): its colours as an array. */
    template <u32 Address, auto Core>
    bool boxed(u32 esp, u32 x, u32 y, u32 width, u32 height, u32 c0, u32 c1, u32 c2, u32 c3)
    {
        const uint32_t colours[4] = {c0, c1, c2, c3};
        return core<Address, Core, 4>(esp, x, y, width, height, static_cast<const uint32_t*>(colours));
    }
    /* The same, called by name, as C++. */
    template <auto Core, unsigned Words, class... P>
    bool named(u32 esp, P... p)
    {
        return run(esp, [&] {
            const auto v = Core(app, ts, u32(esp - 4 - 4 * Words), p...);
            ts.sp = esp;
            return v;
        });
    }
    template <auto Core, class... P>
    bool namedPlain(u32 esp, P... p)
    {
        return run(esp, [&] { return callNamed<Core>(p...); });
    }
    template <auto Core, class... P>
    bool keepNamed(u32 esp, u32 eax, P... p)
    {
        r = eax;
        return run(esp, [&] { return callNamed<Core>(p...); });
    }
    template <auto Core, class... P>
    bool lowNamed(u32 esp, u32 eax, P... p)
    {
        const bool ok = run(esp, [&] { return callNamed<Core>(p...); });
        r = (eax & 0xffffff00u) | (r & 0xff);
        return ok;
    }
    template <auto Core>
    bool boxedNamed(u32 esp, u32 x, u32 y, u32 width, u32 height, u32 c0, u32 c1, u32 c2, u32 c3)
    {
        const uint32_t colours[4] = {c0, c1, c2, c3};
        return named<Core, 4>(esp, x, y, width, height, static_cast<const uint32_t*>(colours));
    }
    template <auto Core, class... P>
    auto callNamed(P... p)
    {
        if constexpr (std::is_invocable_v<decltype(Core), App*, ThreadState&, P...>)
            return Core(app, ts, p...);
        else
            return Core(app, p...);
    }
    /* 0x4d1850, by name (esp restored). */
    bool lineByName(u32 esp, int32_t number, u32 edx)
    {
        ts.sp = esp;
        const auto v = text::lineText(app, ts, esp - 4, number, edx);
        ts.sp = esp;
        return store(v);
    }
    /* 0x4181d0, by name (esp restored). */
    bool effectByName(u32 esp, int32_t effect, int32_t volume)
    {
        ts.sp = esp;
        const auto v = playMenuEffectOf(app, ts, esp - 4, effect, volume);
        ts.sp = esp;
        return store(v);
    }
};

/* ---- The lists ---- */

/* The lists' table looked through for name `name` up to an entry whose name
 * it is or the end (a name 0): that entry's number.  strcmp called with eax
 * the name, edx the entry's, ebx the number, ecx the name. */
Result listNumber(App* app, ThreadState& ts, u32 at, u32 name)
{
    Caller call{app, ts};
    int32_t i = 0;
    for (;; ++i)
    {
        const u32 entry = kLists(app, i).name;
        if (entry == 0)
            break;
        if (!call.plain<0x4ee310, watcom::clib::strcmp>(at, name, entry))
            return {};
        if (call.r == 0)
            break;
    }
    return i;
}

/* 0x442b90's: the colours of list `name`'s lines, or 0. */
Result coloursOf(App* app, ThreadState& ts, u32 esp, u32 name)
{
    if (name == 0)
        return 0;
    const Result i = listNumber(app, ts, esp - 0x10, name);
    if (!i)
        return {};
    const List& l = kLists(app, *i);
    return l.name != 0 ? int32_t(l.colours) : 0;
}

/* 0x442bf0's: which lines of list `name` may be chosen (a word each), or 0. */
Result allowedOf(App* app, ThreadState& ts, u32 esp, u32 name)
{
    if (name == 0)
        return 0;
    const Result i = listNumber(app, ts, esp - 0x10, name);
    if (!i)
        return {};
    const List& l = kLists(app, *i);
    return l.name != 0 ? int32_t(l.allowed) : 0;
}

/* 0x442c80's: the lines of list `name`, where its line chosen is kept to
 * [chosenAt]; not found, "list not found" traced, a key waited for, and 0. */
Result linesOf(App* app, ThreadState& ts, u32 esp, u32 name, u32 chosenAt)
{
    if (name == 0)
        return 0;
    const u32 at = esp - 0x10;
    const Result i = listNumber(app, ts, at, name);
    if (!i)
        return {};
    const List l = kLists(app, *i);
    if (l.name != 0)
    {
        setWord(app, chosenAt, l.chosen);
        return int32_t(l.lines);
    }
    Caller call{app, ts};
    if (!call.keep<0x42d500, raceload::noHook>(at, u32(*i * 0x14))
        || !call.plain<kWaitForKey, ea::waitKey>(at))
        return {};
    return 0;
}

/* 0x442c50's: how many lines list `name` has. */
Result lengthOf(App* app, ThreadState& ts, u32 esp, u32 name)
{
    const u32 frame = esp - 0xc;
    const Result lines = linesOf(app, ts, frame - 4 - 4, name, frame - 4);
    if (!lines)
        return {};
    int32_t n = 0;
    while (half(app, u32(*lines) + u32(n) * 2) != 0)
        ++n;
    return n;
}

/* ---- The HUD's art ---- */

}  // namespace

/* 0x44ac90's: the shapes of the .fsh named at `name` loaded (0x4956f0) and
 * kept at 0x55922c -- hud.fsh's name made the language's in place first --
 * unless they are, by that name; those before freed.  ebx the caller's. */
Result shapesLoaded(App* app, ThreadState& ts, u32 esp, u32 name, u32 ebx)
{
    const u32 at = esp - 0x14;
    Caller call{app, ts};
    if (!call.plain<0x4e0820, watcom::clib::strstr>(at, name, kHud))
        return {};
    const u32 found = call.r;
    if (found != 0)
    {
        const u32 language = u32(gameSettings(app).language) - 1;
        if (language <= 4)
            copyString(app, found, kLanguageHuds[language]);
    }
    if (kHudShapes(app) != 0)
    {
        if (!call.plain<0x4ee310, watcom::clib::strcmp>(at, name, kHudName))
            return {};
        if (call.r == 0)
            return int32_t(kHudShapes(app));
        if (!call.plain<0x4e1890, watcom::clib::free>(at, kHudShapes(app)))
            return {};
    }
    if (!call.plain<kLoadQfs, frontend::draw::loadQfsFlags>(at, name, 0x10u))
        return {};
    kHudShapes.set(app, call.r);
    copyString(app, kHudName, name);
    return int32_t(kHudShapes(app));
}

namespace
{

/* ---- The screen redrawn ---- */

}  // namespace

/* 0x44b170's: the screen drawn again -- its items, a dialog over them -- and
 * shown.  The registers but eax as the caller had them. */
Result redrawn(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    const u32 at = esp - 4;
    Caller call{app, ts};
    if (!call.plain<kTakeWindow, graphics::windowTakenAndCleared>(at))
        return {};
    if (!call.core<0x44a4f0, loop::drawItemsOf, 0>(at, kScreen(app)))
        return {};
    if (!call.plain<0x4438b0, dialogs::dialogShownWith>(at, call.r))
        return {};
    if ((call.r & 0xff) != 0 && !call.core<0x445f50, dialogs::dialogPassOf, 0>(at, call.r))
        return {};
    if (!call.plain<kFlip, graphics::flipAndWait>(at))
        return {};
    return int32_t(call.r);
}

namespace
{

/* 0x440d20's: as a network game is left, if it was asked for (0x555748),
 * the screen drawn and shown until its slide (0x5f3fcc) reaches 0x251. */
Result slideFinished(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    constexpr Var<int32_t> kAsked{0x555748}, kSlide{0x5f3fcc};
    u32 eax = in.eax;
    if (kAsked(app) == 0)
        return int32_t(eax);
    kAsked.set(app, 0);
    const u32 at = esp - 8;
    Caller call{app, ts};
    while (kSlide(app) < 0x251)
    {
        if (!call.plain<kTakeWindow, graphics::windowTakenAndCleared>(at) || !call.plain<0x44aeb0, loop::currentMenuOf>(at)
            || !call.core<0x44a4f0, loop::drawItemsOf, 0>(at, call.r) || !call.plain<kFlip, graphics::flipAndWait>(at))
            return {};
        eax = call.r;
    }
    return int32_t(eax);
}

/* 0x43fdd0's: the network game chosen in the select list (0x5556c8) of the
 * games found (0x5f3ff0) joined: "joining" said, the screen drawn fifteen
 * times, the game's record (0x14 bytes, copied into the frame) handed to the
 * join (0x49e800); joined, the dialog closed and the screen drawn: 5, else 0. */
Result joined(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    constexpr Var<u32> kGames{0x5f3ff0}, kSelectList{0x5556c8};
    const u32 games = kGames(app);
    if (half(app, games) == 0)
        return 0;
    const u32 frame = esp - 0x14, at = frame - 0x14, game = frame - 0x14;
    const u32 row = word(app, kSelectList(app) + 0x4c) << 6;
    copyBlock(app, game, row + word(app, games + 4) + 0x2c, 0x14);
    Caller call{app, ts};
    if (!call.run(at, [&] { return guest::direct<0x444be0, frontend::dialogs::messageById>(app, ts, 0xcu); }))
        return {};
    u32 eax = call.r;
    for (u32 i = 0; i < 0xf; ++i)
    {
        const Result r = redrawn(app, ts, at - 4, {eax, i, in.ebx, 0});
        if (!r)
            return {};
        eax = u32(*r);
    }
    if (!call.plain<kJoinNetworkGame, frontend::connect::joinSession>(at, game))
        return {};
    if (call.r == 0)
        return 0;
    if (!call.plain<0x444a60, dialogs::endMessageOf>(at, call.r) || !redrawn(app, ts, at - 4, {call.r, 0xf, in.ebx, 0}))
        return {};
    return 5;
}

/* ---- A network race's marks ---- */

/* 0x440c40's: the ready mark beside the race's button at (x, y) -- this
 * machine ready or not (0x6fd508) -- its glow pulsing (0x55574c, 0x200 steps
 * up and down); eax as the last call left it. */
Result readyMarks(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y)
{
    constexpr Var<int32_t> kPulse{0x55574c};
    const int32_t step = remainderOf(kPulse(app) + 0xb, 0x200);
    kPulse.set(app, step);
    const u32 colour = (u32(step < 0x100 ? step : 0x1ff - step) << 24) + 0xffffff;
    u32 eax = colour;
    const u32 at = esp - 0x14 - 4;
    Caller call{app, ts};
    const int32_t ready = gameSettings(app).networkLobby;
    const u32 glow = ready != 0 ? 0x5f4004 : 0x5f3ff6, mark = ready != 0 ? 0x5f3ff4 : 0x5f3ff8;
    u32 ecx = ready != 0 ? u32(ready) : 0;
    if (half(app, glow) != 0)
    {
        if (!call.named<shapes::drawArt, 0>(at, u32(wide(half(app, glow))), u32(x - 6), u32(y - 5), colour))
            return {};
        eax = call.r;
        ecx = colour;
    }
    if (half(app, mark) != 0)
    {
        const int32_t dx = ready != 0 ? -1 : 0, dy = ready != 0 ? 1 : 0;
        if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(at, u32(wide(half(app, mark))), u32(x + dx), u32(y + dy)))
            return {};
        eax = call.r;
    }
    return int32_t(eax);
}

/* ---- The licence screen ---- */

}  // namespace

/* 0x4d88f0's: the rectangle (eax, edx) to (ebx, ecx) with a colour at each
 * corner, through 0x4d81b0. */
Result colouredRectangle(App* app, ThreadState& ts, u32 esp, const Args& a, const u32 colours[4])
{
    Caller call{app, ts};
    if (!call.core<0x4d81b0, draw2d::shadedRectangle, 4>(esp - 0xc, a.eax, a.edx, a.ebx, a.ecx, static_cast<const uint32_t*>(colours)))
        return {};
    return int32_t(call.r);
}

namespace
{

/* 0x442450's: the licence screen's pass, once the clock is 5 past the last
 * (0x5f40c4): on a card without alpha and in a 2D mode, the band of the page
 * shown (0x5f40c0: 3 on the left, 2 on the right) shaded, and its picture
 * drawn (0x555c38).  0. */
Result licenceDrawn(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    constexpr Var<int32_t> kLast{0x5f40c4}, kPage{0x5f40c0}, kSlideX{0x555c2c}, kMode{0x555c34};
    constexpr Var<u32> kPicture{0x555c38};
    while (*reinterpret_cast<volatile int32_t*>(&app->getMemory<int32_t>(kClock.at)) < kLast(app) + 5)
    {
    }
    kLast.set(app, 0);
    const u32 at = esp - 0x14;
    u32 ecx = in.ecx;
    const uint8_t flags = kRenderFlags(app);
    if ((flags & 2) == 0 && (flags & 0x40) == 0)
    {
        const u32 mode = word(app, (u32(kMode(app)) << 4) + 0x555758);
        if (mode == 0 || mode == 1)
        {
            if (kPage(app) == 3)
            {
                int32_t left = kSlideX(app) - 0x112;
                const int32_t right = kSlideX(app) + 0x2e;
                if (left < 0x2e)
                    left = 0x2e;
                const u32 colours[] = {0x106060a0, 0xff6060a0, 0xff6060a0, 0x106060a0};
                if (!colouredRectangle(app, ts, at - 0x10 - 4, {u32(left), 0x49, u32(right), 0x165}, colours))
                    return {};
                ecx = 0x165;
            }
            if (kPage(app) == 2)
            {
                int32_t right = kSlideX(app) + 0x392;
                const int32_t left = kSlideX(app) + 0x252;
                if (right > 0x252)
                    right = 0x252;
                const u32 colours[] = {0xff6060a0, 0x106060a0, 0x106060a0, 0xff6060a0};
                if (!colouredRectangle(app, ts, at - 0x10 - 4, {u32(left), 0x49, u32(right), 0x165}, colours))
                    return {};
                ecx = 0x165;
            }
        }
    }
    const u32 picture = kPicture(app);
    if (picture != 0)
    {
        Caller call{app, ts};
        if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(at, picture, u32(kSlideX(app) + 0x10), 0x39))
            return {};
    }
    return 0;
}

/* ---- The text box ---- */

/* The arrows' shapes: up and down, plain and under the pointer. */
constexpr Var<int16_t> kUpArrow{0x677d40}, kDownArrow{0x677d46}, kUpArrowLit{0x677d44}, kDownArrowLit{0x677d42};
constexpr u32 kArrowNames[] = {0x539918, 0x539920, 0x539928, 0x539930};  // bup1, bdn1, yup1, ydn1

/* Whether the screen of item `at` is the one shown (0x44a660, edx 0x1e). */
Result shownNow(App* app, ThreadState& ts, u32 at, u32 item, u32 ebx, u32 ecx)
{
    Caller call{app, ts};
    if (!call.plain<kMenuShownNow, frontend::loop::shownNow>(at, item))
        return {};
    return int32_t(call.r);
}

/* 0x455e30's: text box `at` drawn -- its field, its text wrapped into it from
 * its first line (the lines it takes noted), the arrows at its right, lit
 * under the pointer (`lit`). */
Result textBoxDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    const u32 frame = esp - 0x14, call_at = frame - 0x1c;
    const Result shown = shownNow(app, ts, call_at, at, lit, at);
    if (!shown)
        return {};
    if (*shown == 0)
        return 0;
    TextBoxItem& t = item<TextBoxItem>(app, at);
    u32 colour = 0xff649dfd;
    int16_t down;
    if ((t.flags & 1) != 0)
    {
        colour = kDark;
        down = kDownArrow(app);
    }
    else
    {
        if (lit != 0)
            colour = kYellow;
        down = colour == kYellow ? kDownArrowLit(app) : kDownArrow(app);
    }
    const u32 widthAt = frame - 4, heightAt = frame - 8;
    Caller call{app, ts};
    if (!call.namedPlain<shapes::shapeSizeOf>(call_at, u32(wide(down)), widthAt, heightAt))
        return {};
    const u32 edge = colour == kYellow ? 0xffffe440 : 0xff000040;
    if ((kRenderFlags(app) & 2) != 0 && !call.plain<0x431900, graphics::sendState>(call_at, 6, 1))
        return {};
    {
        const int32_t width = wide(half(app, widthAt)) + wide(t.width) + 1;
        if (!call.boxedNamed<draw2d::shadedBox>(call_at, u32(wide(t.x)), u32(wide(t.y)), u32(width), u32(wide(t.height)), 0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222))
            return {};
        if (!call.named<draw2d::outline, 1>(call_at, u32(wide(t.x)), u32(wide(t.y)), u32(width), u32(wide(t.height)), edge))
            return {};
        if ((kRenderFlags(app) & 2) != 0 && !call.plain<0x431900, graphics::sendState>(call_at, 6, 0))
            return {};
        if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont))
            return {};
    }
    kScreenTextSize.set(app, kOne);
    if (t.text != 0)
    {
        if (!call.core<0x451cd0, text::wrappedText, 8>(
                call_at, text::Box{t.text, kMenuFont, colour, wide(t.x), wide(t.y), wide(t.width) - 1, frame - 0x1c,
                                   frame - 0x18, frame - 0x14, 0, wide(t.gap), wide(t.first)}))
            return {};
        item<TextBoxItem>(app, at).lines = int16_t(word(app, frame - 0x14));
    }
    else
        t.lines = 1;
    const u32 up = u32(wide(colour == kYellow ? kUpArrowLit(app) : kUpArrow(app)));
    const int32_t right = wide(t.x) + wide(t.width);
    if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, up, u32(right), u32(wide(t.y))))
        return {};
    const int32_t bottom = wide(t.height) + wide(t.y) - 1 - wide(half(app, heightAt));
    if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(down)), u32(right), u32(bottom)))
        return {};
    return 0;
}

/* 0x456020's: key `key` on text box `at`: Enter on its up arrow scrolls it
 * up a line, on its down arrow down one, as far as its lines go: 2; else 0. */
Result textBoxKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    const u32 frame = esp - 0x10, call_at = frame - 0x18;
    const Result shown = shownNow(app, ts, call_at, at, key, at);
    if (!shown)
        return {};
    if (*shown == 0)
        return 0;
    Caller call{app, ts};
    if (!call.namedPlain<shapes::shapeSizeOf>(call_at, u32(wide(kDownArrowLit(app))), frame - 8, frame - 4))
        return {};
    TextBoxItem& t = item<TextBoxItem>(app, at);
    const int16_t width = int16_t(word(app, frame - 8)), height = int16_t(word(app, frame - 4));
    // The arrows' boxes: top, left, width, height.
    const u32 up = frame - 0x18, down = frame - 0x10;
    setHalf(app, up, t.y);
    setHalf(app, up + 2, int16_t(t.x + t.width));
    setHalf(app, up + 4, width);
    setHalf(app, up + 6, height);
    setHalf(app, down, int16_t(t.y + t.height - height));
    setHalf(app, down + 2, int16_t(t.x + t.width));
    setHalf(app, down + 6, height);
    setHalf(app, down + 4, width);
    if (int16_t(key) != 0xd)
        return 0;
    if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, up))
        return {};
    if (call.r != 0)
    {
        const int16_t first = int16_t(t.first - 1);
        t.first = first;
        if (first < 0)
            t.first = 0;
        return 2;
    }
    if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, down))
        return {};
    if (call.r == 0)
        return 0;
    ++t.first;
    if (wide(t.first) > wide(t.owned) - wide(t.gap))
        --t.first;
    return 2;
}

/* 0x456110's: text box `at` laid out: its box its field and its arrows.
 * What is left of the arrow's width's dword. */
Result textBoxLaidOut(App* app, ThreadState& ts, u32 esp, u32 at, u32 ebx)
{
    const u32 frame = esp - 0x10, call_at = frame - 8;
    const Result shown = shownNow(app, ts, call_at, at, ebx, at);
    if (!shown)
        return {};
    if (*shown == 0)
        return 0;
    TextBoxItem& t = item<TextBoxItem>(app, at);
    t.boxLeft = t.x;
    t.boxTop = t.y;
    t.boxHeight = t.height;
    t.boxWidth = t.width;
    Caller call{app, ts};
    if (!call.namedPlain<shapes::shapeSizeOf>(call_at, u32(wide(kDownArrowLit(app))), frame - 4, frame - 8))
        return {};
    const u32 width = word(app, frame - 4);
    t.boxWidth = int16_t(t.boxWidth + int16_t(width));
    return int32_t(width);
}

/* 0x456170's: as a text box is shown, its arrows' shapes found (bup1,
 * bdn1, yup1, ydn1). */
Result textBoxShowing(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    const u32 at = esp - 8;
    const Result shown = shownNow(app, ts, at, in.eax, in.ebx, in.ecx);
    if (!shown)
        return {};
    if (*shown == 0)
        return 0;
    Caller call{app, ts};
    const Var<int16_t> arrows[] = {kUpArrow, kDownArrow, kUpArrowLit, kDownArrowLit};
    for (int i = 0; i < 4; ++i)
    {
        if (!call.named<shapes::shapeNamed, 0>(at, kArrowNames[i]))
            return {};
        arrows[i].set(app, int16_t(call.r));
    }
    return int32_t(call.r);
}

/* 0x4561d0's: as a text box is hidden, its own text freed. */
Result textBoxHiding(App* app, ThreadState& ts, u32 esp, u32 at, u32 ebx)
{
    const u32 call_at = esp - 0xc;
    const Result shown = shownNow(app, ts, call_at, at, ebx, at);
    if (!shown)
        return {};
    TextBoxItem& t = item<TextBoxItem>(app, at);
    if (*shown == 0 || t.owned == 0)
        return shown;
    Caller call{app, ts};
    if (!call.plain<0x4e1890, watcom::clib::free>(call_at, t.text))
        return {};
    item<TextBoxItem>(app, at).owned = 0;
    return int32_t(call.r);
}

/* ---- The track's map ---- */

/* The map's slide: its step (0 waiting, 1 coming in, 2 going), how far. */
constexpr Var<int32_t> kMapStep{0x677d7c}, kMapAlong{0x677d78};
constexpr Var<int32_t> kMapLine{0x559818};
constexpr Table<int16_t> kMapWidth{0x677d66}, kMapX{0x677d5e};
constexpr Var<int16_t> kMapTile{0x677d90};

/* 0x456320's: the track's map slid in and out: 15 bands of it drawn, each at
 * its place along the slide (a float, [ebp-8] as the code keeps it) while
 * that is within 0 and 1, the slide stepped on -- `advance` lets it go from
 * coming in to going.  1 while it waits, 0 once it moves. */
Result mapSlid(App* app, ThreadState& ts, u32 esp, u32 advance)
{
    const u32 frame = esp - 0x18, at = frame - 0x34;
    // [ebp-8], unset where the step is past 2: the frame's own.
    u32 along = word(app, frame - 8);
    int32_t result = 1;
    const u32 step = u32(kMapStep(app));
    if (step == 0)
    {
        along = 0;
        kMapAlong.set(app, kMapAlong(app) + 1);
    }
    else if (step == 1)
    {
        const int32_t was = kMapAlong(app);
        if (was < 0x100)
        {
            kMapAlong.set(app, was + 0xc);
            if (was + 0xc > 0x100)
                kMapAlong.set(app, 0x100);
        }
        along = bitsOf(float(fp::add(fp::mul(Float(kMapAlong(app)), Float(floatAt(app, 0x539a48))),
                                   Float(doubleAt(app, 0x539a50)))));
        result = 0;
    }
    else if (step == 2)
    {
        const int32_t was = kMapAlong(app);
        if (was != 0)
        {
            kMapAlong.set(app, was - 0x14);
            if (was - 0x14 > 0)
                result = 0;
            else
                kMapAlong.set(app, 0);
        }
        along = bitsOf(float(fp::add(fp::mul(Float(kMapAlong(app)), Float(floatAt(app, 0x539a48))),
                                   Float(doubleAt(app, 0x539a58)))));
    }
    const bool wideMap = kMapWidth(app, kMapLine(app)) > 0x140;
    const int32_t height = 0xfa, top = wideMap ? 0x80 : 0x200;
    Caller call{app, ts};
    for (int32_t band = 0; band < 0xf; ++band)
    {
        const u32 now = u32(kMapStep(app));
        if (now == 0)
        {
            if (kMapAlong(app) == 0x3c)
            {
                kMapAlong.set(app, 0);
                kMapStep.set(app, 1);
            }
            band = 0xe;
            along = 0;
        }
        else if (now == 1)
        {
            if (kMapAlong(app) == 0x100)
            {
                along = kOne;
                band = 0xe;
            }
            else
                along = bitsOf(float(fp::add(Float(floatOf(along)), Float(doubleAt(app, 0x539a60)))));
            if (advance != 0)
                kMapStep.set(app, kMapStep(app) + 1);
        }
        else if (now == 2)
            along = bitsOf(float(fp::add(Float(floatOf(along)), Float(doubleAt(app, 0x539a68)))));
        const Float v = Float(floatOf(along));
        const Float vd = Float(double(v));
        if (Float(0.0) > vd)
            continue;
        if (!(Float(1.0) >= vd))
            continue;
        const int32_t line = kMapLine(app);
        const Float vk = fp::mul(v, Float(floatAt(app, 0x539a70)));
        const Float rest = fp::sub(Float(1.0), vd);
        const Float wRest = fp::mul(Float(kMapWidth(app, line)), rest);
        const Float xRest = fp::mul(rest, Float(kMapX(app, line)));
        const Float topV = fp::mul(Float(top), v);
        const Float size = fp::add(vk, Float(floatAt(app, 0x539a74)));
        const Float downV = fp::mul(v, Float(height));
        const int32_t side = chopped<int32_t>(ts, size);
        const Float across = fp::add(topV, wRest);
        const Float down = fp::add(downV, xRest);
        const int32_t x = chopped<int32_t>(ts, across);
        const int32_t y = chopped<int32_t>(ts, down);
        const u32 colour = band == 0xe ? 0xffffffff : 0x40ffffff;
        const int32_t half_ = halved(side);
        if (!call.core<0x4d8b80, draw2d::drawTileRectangleOf, 3>(at, u32(wide(kMapTile(app))), u32(x - half_), u32(x + half_), u32(y - half_), u32(y + half_), colour, colour))
            return {};
    }
    return result;
}

/* 0x456590's: the track's facts under its map: its name and length, the
 * length in km or miles by the language, at its place by the track. */
Result trackFacts(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    const u32 frame = esp - 0x18, at = frame - 0x50, text = frame - 0x50;
    const int32_t track = gameSettings(app).track;
    int32_t x, y;
    if (track == 4)
        y = 0x1ae, x = 0xc8;
    else if (track == 0 || track == 7 || track == 8)
        y = 0x19a, x = 0xfa;
    else
        y = 0x186, x = 0x1d1;
    Caller call{app, ts};
    const int32_t language = gameSettings(app).language;
    if (language == 0)
    {
        if (!call.lineByName(at, 6, u32(x)))
            return {};
        const u32 miles = call.r;
        if (!call.plain<kSprintf, watcom::clib::sprintf>(
                at, text, 0x539a78u,
                guest::Words({word(app, u32(gameSettings(app).track) * 4 + 0x559644), miles})))
            return {};
    }
    else
    {
        const u32 table = language == 1 || language == 2 || language == 5 ? 0x559668 : 0x55968c;
        const u32 length = word(app, u32(gameSettings(app).track) * 4 + table);
        if (!call.plain<kSprintf, watcom::clib::sprintf>(at, text, 0x539a80u, guest::Words({length})))
            return {};
    }
    if (!call.named<text::menuTextDrawn, 2>(at, text, u32(x), u32(y), kMenuFont, 0, 0x488cfd))
        return {};
    return int32_t(call.r);
}

}  // namespace

/* 0x456800's: the track's map's tile `index` (its 16 by 16 picture of the
 * track's, 0x559764, on EA's bitmap) made shape `shape`'s textures: their
 * record. */
Result mapShape(App* app, ThreadState& ts, u32 esp, u32 shape, int32_t index)
{
    constexpr Var<u32> kMapArt{0x677d80};
    const u32 at = esp - 0x14;
    Caller call{app, ts};
    if (!call.plain<kNewBitmap, frontend::eab::windowBitmapCreateDefault>(at, 0x100u, 0x100u, 0x10u, 0x10u))
        return {};
    const u32 bitmap = call.r;
    if (!call.plain<kSetBitmap, frontend::eaa::bitmapDrawInto>(at, bitmap))
        return {};
    if (!call.plain<kClearBitmap, frontend::eab::drawClearClip>(at, byteAt(app, 0x565010) == 8 ? 0xffu : 0u))
        return {};
    u32 edx = 0x100;
    if (kMapArt(app) != 0)
    {
        edx = word(app, (u32(gameSettings(app).track) << 4) + u32(index) * 4 + 0x559764);
        if (!call.plain<kBitmapShape, frontend::eab::shapeFileFind>(at, kMapArt(app), edx)
            || !call.plain<kDrawBitmapShape, frontend::wata::blitShapeAtPosition>(at, call.r))
            return {};
    }
    if (!call.plain<kBitmapDone, frontend::eaa::bitmapDrawToScreen>(at))
        return {};
    if (!call.core<kArtShapeTextures, shapes::artTexturesOf, 0>(at, word(app, bitmap + 0x20), int32_t(shape), bitmap))
        return {};
    const u32 textures = call.r;
    if (!call.plain<kFreeBitmap, frontend::eab::windowBitmapFree>(at, bitmap))
        return {};
    return int32_t(textures);
}

/* 0x4d6730's: menu shape `shape` drawn turned by `angle` and scaled by
 * `scale` (a float's bits) about (x, y), `nudge` to the left: a quad of each
 * of its tiles through the clip (0x4c12e0), as the 2D screens draw (depth and
 * w 0x563aa8, 0x563aac).  Its tiles across, or the angle, or the trace's. */
Result turnedShape(App* app, ThreadState& ts, u32 esp, int32_t shape, int32_t x, int32_t y, int32_t nudge, int32_t angle,
                   u32 scaleBits)
{
    const u32 frame = esp - 0xc, at = frame - 0x4c;
    const float scale = float(fp::mul(Float(floatOf(scaleBits)), Float(doubleAt(app, 0x54965c))));
    const u32 turn = bitsOf(float(fp::mul(Float(angle), Float(doubleAt(app, 0x549664)))));
    if (kOverRace(app) != 0)
        return angle;
    const u32 rec = u32(shape) * 0x14 + 0x8b48c0;
    int32_t tile = wide(half(app, rec + 0xc));
    Caller call{app, ts};
    if (tile == -1)
    {
        if (!call.keep<0x42d500, raceload::noHook>(at, u32(shape) * 0x14))
            return {};
        return int32_t(call.r);
    }
    const Float s = Float(scale);
    const Float k = Float(floatAt(app, 0x54966c));
    const Float across = fp::mul(Float(wide(half(app, rec + 4))), s);
    const Float down = fp::mul(s, Float(-wide(half(app, rec + 6))));
    const Float halfAcross = fp::mul(across, k);
    const Float halfDown = fp::mul(k, down);
    app->getMemory<float>(0x563b18) = float(Float(floatAt(app, 0x563aa8)));
    app->getMemory<float>(0x563b1c) = float(Float(floatAt(app, 0x563aac)));
    setWord(app, 0x563af8, word(app, 0x563b18));
    app->getMemory<float>(0x563afc) = float(Float(floatAt(app, 0x563b1c)));
    setWord(app, 0x563ad8, word(app, 0x563af8));
    app->getMemory<float>(0x563adc) = float(Float(floatAt(app, 0x563afc)));
    const Float depth = Float(floatAt(app, 0x563ad8)), w = Float(floatAt(app, 0x563adc));
    const Float left = fp::sub(Float(nudge), halfAcross);
    app->getMemory<float>(0x563ab8) = float(depth);
    const float x0 = float(fp::add(left, Float(wide(half(app, rec + 8)))));
    app->getMemory<float>(0x563abc) = float(w);
    const float y0 = float(fp::add(halfDown, Float(wide(half(app, rec + 0xa)))));
    for (const u32 code : {0x563b20u, 0x563b00u, 0x563ae0u, 0x563ac0u})
        setWord(app, code, 0xffffffff);
    // A trig function of the angle, left on the x87's stack by the game's code.
    // 0x4ea930's and 0x4ea950's sine and cosine of the float.
    const auto sine = [&](Float& value) {
        ts.sp = at;
        value = guest::direct<0x4ea930, frontend::simd::sineValue>(app, ts, turn);
        return !ts.terminate;
    };
    const auto cosine = [&](Float& value) {
        ts.sp = at;
        value = guest::direct<0x4ea950, frontend::simd::cosineValue>(app, ts, turn);
        return !ts.terminate;
    };
    int32_t eax = 0;
    for (int32_t column = 0;; ++column)
    {
        eax = wide(half(app, rec + 4));
        if (eax <= column)
            break;
        const Float xaFull = fp::add(fp::mul(Float(column), s), Float(x0));
        const float xa = float(xaFull);
        const float xb = float(fp::add(s, xaFull));
        for (int32_t row = 0;; ++row)
        {
            if (wide(half(app, rec + 6)) <= row)
                break;
            const Float yaFull = fp::add(fp::mul(Float(row), s), Float(y0));
            const float ya = float(yaFull);
            const float yb = float(fp::add(s, yaFull));
            const float xf = float(Float(x)), yf = float(Float(y));
            Float c, sn;
            float t;
            const auto vertexX = [&](u32 at_, float xs, float ys) {
                if (!cosine(c))
                    return false;
                t = float(fp::add(fp::mul(c, Float(xs)), Float(xf)));
                if (!sine(sn))
                    return false;
                app->getMemory<float>(at_) = float(fp::sub(Float(t), fp::mul(sn, Float(ys))));
                return true;
            };
            const auto vertexY = [&](u32 at_, float xs, float ys) {
                if (!sine(sn))
                    return false;
                t = float(fp::add(fp::mul(sn, Float(xs)), Float(yf)));
                if (!cosine(c))
                    return false;
                app->getMemory<float>(at_) = float(fp::add(fp::mul(c, Float(ys)), Float(t)));
                return true;
            };
            if (!vertexX(0x563ab0, xa, ya) || !vertexX(0x563ad0, xb, ya) || !vertexX(0x563af0, xb, yb)
                || !vertexX(0x563b10, xa, yb) || !vertexY(0x563ab4, xa, ya) || !vertexY(0x563ad4, xb, ya)
                || !vertexY(0x563af4, xb, yb) || !vertexY(0x563b14, xa, yb))
                return {};
            const u32 texture = word(app, u32(tile) * 0xc + 0x8b5090);
            ++tile;
            if (!call.plain<0x431900, graphics::sendState>(at, 1, texture))
                return {};
            for (u32 vertex = 0x563ab0; vertex <= 0x563b10; vertex += 0x20)
            {
                const int32_t wBits = int32_t(word(app, vertex + 0xc)), yBits = int32_t(word(app, vertex + 4));
                const int32_t xBits = int32_t(word(app, vertex));
                uint8_t code;
                if (wBits < 0 || wBits >= int32_t(word(app, 0x7d34f8)))
                    code = 0x10;
                else
                {
                    code = 0;
                    if (yBits < 0 || yBits < int32_t(word(app, 0x7d3504)))
                        code |= 8;
                    else if (yBits > int32_t(word(app, 0x7d3500)))
                        code |= 4;
                    if (xBits < 0 || xBits < int32_t(word(app, 0x7d34fc)))
                        code |= 1;
                    else if (xBits > int32_t(word(app, 0x7d350c)))
                        code |= 2;
                }
                app->getMemory<uint8_t>(vertex + 0x14) = code;
            }
            if ((kRenderFlags(app) & 2) == 0
                && !call.core<kClipQuad, vertices::quadClippedAt, 0>(at, 0x563ab0u, 0x563ad0u, 0x563af0u, 0x563b10u))
                return {};
        }
    }
    return eax;
}

namespace
{

/* 0x4d88f0's thunk's core is colouredRectangle (above). */

/* 0x456670's: the "glow" image's pass: on the track's screen once it has slid
 * in, the facts drawn, the map's shape turned, its line pulsing, the
 * description spoken a line at a time as the last ends (0x559818, 0x559820),
 * the map's tile of that line drawn and slid.  9 while the track is not the
 * map's (0x677d8c); 0. */
Result glowDrawn(App* app, ThreadState& ts, u32 esp, const Args& in)
{
    constexpr Var<int32_t> kMapTrack{0x677d8c}, kAngle{0x559824}, kPulse{0x55981c}, kSpoken{0x559820};
    if (kSlidingIn(app) != 0)
        return 0;
    if (gameSettings(app).track != kMapTrack(app))
        return 9;
    const u32 at = esp - 0x18;
    if (!trackFacts(app, ts, at - 4, {in.eax, 0, in.ebx, u32(gameSettings(app).track)}))
        return {};
    const int32_t angle = kAngle(app) + 4;
    kAngle.set(app, angle);
    const int32_t track = gameSettings(app).track;
    const Result turned =
        turnedShape(app, ts, at - 8 - 4, wide(half(app, 0x677d92)), wide(half(app, u32(track) * 2 + 0x5596b0)),
                    wide(half(app, u32(track) * 2 + 0x5596c2)), 0, angle, kOne);
    if (!turned)
        return {};
    const int32_t sum = kPulse(app) + 0x10;
    const int32_t pulse = remainderOf(sum, 0x200);
    kPulse.set(app, pulse);
    const u32 alpha = u32(pulse < 0x100 ? pulse : 0x1ff - pulse);
    Caller call{app, ts};
    if (!call.core<0x4c95e0, spokenStatusOf, 0>(at))
        return {};
    if (call.r != 0 && kSpoken(app) != 0)
    {
        const int32_t line = kMapLine(app) + 1;
        kMapLine.set(app, line);
        if (line < 5)
        {
            const Result tile = mapShape(app, ts, at - 4, u32(wide(half(app, 0x677d90))), line - 1);
            if (!tile)
                return {};
            kMapTile.set(app, int16_t(*tile));
        }
        kMapStep.set(app, 0);
        kMapAlong.set(app, 0);
        const int32_t lines = int32_t(word(app, u32(gameSettings(app).track) * 4 + 0x5597f4));
        if (kMapLine(app) <= lines
            && !call.core<kDescriptionStart, descriptionStarted, 0>(at, u32(kMapLine(app)), 0x33u,
                                                                   u32(gameSettings(app).language)))
            return {};
    }
    const int32_t line = kMapLine(app);
    const u32 colour = (alpha << 24) + 0xffffff;
    if (line > int32_t(word(app, u32(gameSettings(app).track) * 4 + 0x5597f4)) || line <= 0)
    {
        kSpoken.set(app, 1);
        return 0;
    }
    const u32 at2 = u32(line) * 2;
    if (half(app, at2 + 0x677d6e) != 0
        && !call.named<shapes::drawArt, 0>(at, u32(wide(half(app, at2 + 0x677d6e))), u32(wide(half(app, at2 + 0x677d4e))), u32(wide(half(app, at2 + 0x677d56))), colour))
        return {};
    if (!call.core<0x4c95e0, spokenStatusOf, 0>(at))
        return {};
    const Result slid = mapSlid(app, ts, at - 4, call.r);
    if (!slid)
        return {};
    kSpoken.set(app, *slid);
    return 0;
}

/* ---- The rectangle ---- */

/* The field box's four colours, as the code pushes them. */
constexpr u32 kFieldColours[] = {0xa0000060, 0xa0000050, 0xd0000020, 0xd0000010};

/* 0x45fae0's: rectangle `at` drawn by its kind: 0 shaded in its corners'
 * colours, 1 a frame of four such bars (its thickness at 0x50), 2 a field
 * box (not while the screens slide in or the help is up); then, for the
 * first two kinds and a field box drawn, its function over it (0x54).  0. */
Result rectDrawn(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0x14 - 4;
    RectItem& r = item<RectItem>(app, at);
    const u32 kind = r.kind;
    Caller call{app, ts};
    const int32_t x = wide(r.x), y = wide(r.y), width = wide(r.width), height = wide(r.height);
    const int32_t thick = wide(r.thick);
    const auto box = [&](int32_t bx, int32_t by, int32_t w, int32_t h) {
        return call.boxedNamed<draw2d::shadedBox>(call_at, u32(bx), u32(by), u32(w), u32(h), r.text, r.moreColours[0], r.moreColours[1], r.moreColours[2]);
    };
    if (kind == 0)
    {
        if (!box(x, y, width, height))
            return {};
    }
    else if (kind == 1)
    {
        if (!box(x, y, width, thick) || !box(x, y, thick, height) || !box(x, y + height - thick, width, thick)
            || !box(x + width - thick, y, thick, height))
            return {};
    }
    else if (kind == 2)
    {
        if (kSlidingIn(app) != 0)
            return 0;
        if (!call.core<0x449df0, loop::keysTakenOf, 0>(call_at, 2))
            return {};
        if (call.r != 0)
            return 0;
        if (!call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(x), u32(y), u32(width), u32(height), kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3]))
            return {};
    }
    else
        return 0;
    if (r.over != 0 && !call(call_at, r.over, {at, u32(y), u32(width), u32(height)}))
        return {};
    return 0;
}

/* ---- The text ---- */

/* 0x45fcd0's: text `at` drawn: in its colour (greyed, dark under the help),
 * the line of its list where it shows one, by its look -- framed, scaled and
 * shadowed or sweeping in on the 2D screens, plain and dimmed behind on a
 * race's pause screens; its sweep's restart cleared.  0. */
Result textDrawn(App* app, ThreadState& ts, u32 esp, u32 at, const Args& in)
{
    if (demoHides(app))
        return 0;
    const u32 frame = esp - 0x14, call_at = frame - 0x24;
    TextItem& t = item<TextItem>(app, at);
    static const u32 colours[] = {kYellow, 0xff649dfd, 0xffd59428, 0xff5abeff, 0xffffff};
    u32 colour = uint16_t(t.colour) <= 4 ? colours[uint16_t(t.colour)] : kYellow;
    Caller call{app, ts};
    if (t.align == 3)
    {
        // eax as the colour's switch left it: its number, the item's upper half
        // past 4.
        const u32 eax = uint16_t(t.colour) <= 4 ? u32(uint16_t(t.colour)) : (at & 0xffff0000) | uint16_t(t.colour);
        if (!call.core<0x449df0, loop::keysTakenOf, 0>(call_at, eax))
            return {};
        if (call.r != 0)
            colour = kDark;
        if (kSlidingIn(app) != 0)
            return 0;
    }
    demoGreys(app, at);
    if ((t.flags & 1) != 0)
        colour = kGrey;
    kNudge.set(app, wide(t.nudge));
    const int32_t x = wide(t.x), y = wide(t.y);
    const u32 list = t.listName;
    if (list != 0 && byteAt(app, list) != 0)
    {
        const Result lines = linesOf(app, ts, call_at - 4, list, frame - 0x24);
        if (!lines)
            return {};
        if (*lines != 0)
        {
            const u32 chosen = word(app, word(app, frame - 0x24));
            if (!call.lineByName(call_at, u32(wide(half(app, u32(*lines) + chosen * 2))), chosen))
                return {};
            item<TextItem>(app, at).text = call.r;
        }
    }
    const u32 align = t.align == 1 ? 4 : t.align == 2 ? 5 : 0;
    const u32 scale = t.scale;
    if (t.look == 1)
    {
        const u32 bright = colour | 0xff000000;
        if (kOverRace(app) != 0)
        {
            if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont))
                return {};
            kNudge.set(app, wide(t.nudge));
            if (!call.plain<0x4d1400, text::keepTextScaleOf>(call_at)
                || !(setWord(app, call_at - 4, kOne), call.plain<0x451530, text::dimmed>(call_at, call_at - 4, call.r)))
                return {};
            if ((scale & 0x7fffffff) != 0)
            {
                if (!call.core<0x4da120, draw2d::turnedText, 3>(call_at, t.text, u32(x + 2), u32(y + 2), scale, 0, 0xff000000)
                    || !call.core<0x4da120, draw2d::turnedText, 3>(call_at, t.text, u32(x), u32(y), t.scale, 0, bright))
                    return {};
            }
            else if (!call.named<text::menuTextDrawn, 2>(call_at, t.text, u32(x), u32(y), kMenuFont, align, colour))
                return {};
            if (!call.keep<0x4d1410, text::restoreTextScaleOf>(call_at, 0))
                return {};
        }
        else
        {
            if (!call.keepNamed<text::selectFontOf>(call_at, kBigFont, kBigFont))
                return {};
            if ((scale & 0x7fffffff) != 0)
            {
                if (!call.core<0x4da120, draw2d::turnedText, 3>(call_at, t.text, u32(x + 4), u32(y + 4), scale, 0, 0xff000040)
                    || !call.core<0x4da120, draw2d::turnedText, 3>(call_at, t.text, u32(x + 2), u32(y + 2), t.scale, 0, 0xff000008)
                    || !call.core<0x4da120, draw2d::turnedText, 3>(call_at, t.text, u32(x), u32(y), t.scale, 0, bright))
                    return {};
            }
            else
            {
                bool swept = false;
                if (t.colour == 0)
                {
                    if (!call.core<0x4da1e0, draw2d::titleText, 0>(call_at, t.text, u32(x), u32(y), u32(wide(t.restart))))
                        return {};
                    swept = call.r != 0;
                }
                if (!swept
                    && (!call.named<text::menuTextDrawn, 2>(call_at, t.text, u32(x + 4), u32(y + 4), kBigFont, align, 0x40)
                        || !call.named<text::menuTextDrawn, 2>(call_at, t.text, u32(x + 2), u32(y + 2), kBigFont, align, 8)
                        || !call.named<text::menuTextDrawn, 2>(call_at, t.text, u32(x), u32(y), kBigFont, align, colour)))
                    return {};
            }
        }
    }
    else
    {
        const u32 text = t.text;
        if (kOverRace(app) != 0)
        {
            if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, text))
                return {};
            if ((call.r & 0xff) != 0)
                return 0;
        }
        if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont))
            return {};
        kNudge.set(app, wide(t.nudge));
        if ((t.scale & 0x7fffffff) != 0)
        {
            if (!call.core<0x4da120, draw2d::turnedText, 3>(call_at, text, u32(x + 2), u32(y + 2), t.scale, 0, 0xff000000)
                || !call.core<0x4da120, draw2d::turnedText, 3>(call_at, text, u32(x), u32(y), t.scale, 0, colour | 0xff000000))
                return {};
        }
        else if (!call.named<text::menuTextDrawn, 2>(call_at, text, u32(x), u32(y), kMenuFont, align, colour))
            return {};
    }
    item<TextItem>(app, at).restart = 0;
    kNudge.set(app, 0);
    return 0;
}

/* 0x460070's: text `at` laid out: its box at its place, as wide as its text
 * in the menus' font, 0x1d high.  The width. */
Result textLaidOut(App* app, ThreadState& ts, u32 esp, u32 at, u32 ebx)
{
    TextItem& t = item<TextItem>(app, at);
    t.boxLeft = t.x;
    t.boxTop = t.y;
    t.boxHeight = 0x1d;
    Caller call{app, ts};
    if (!call.named<text::menuTextWidth, 0>(esp - 0xc, t.text, kMenuFont))
        return {};
    item<TextItem>(app, at).boxWidth = int16_t(call.r);
    return int32_t(call.r);
}

/* 0x4600b0's: as text `at` is shown: placed by the race's table where it is
 * aligned so, its sweep started but on the 2D screens' mode 9. */
void textShowing(App* app, u32 at)
{
    TextItem& t = item<TextItem>(app, at);
    if (t.align == 3)
    {
        t.x = int16_t(kPlaceX(app, wide(t.place)) + 0x92);
        t.y = kPlaceY(app, wide(t.place));
    }
    if (kScreenMode(app) != 9)
        t.restart = 1;
}

/* ---- The image ---- */

/* 0x460100's: image `at` drawn -- or what its codelink names: the track's
 * glow and map, the licence screen, the main and text effects, the logo, the
 * track's picture made from its art; slid in with the screen or by itself,
 * in its alpha, the 3 and 2 button bars and the small back's darkened, framed
 * where it is so; the band's effects after it.  The glow's or the licence's
 * answer, else 0. */
Result imageDrawn(App* app, ThreadState& ts, u32 esp, u32 at, const Args& in)
{
    constexpr Var<int32_t> kTrackShown{0x678870};
    constexpr Var<u32> kArt{0x559228};
    const u32 frame = esp - 0x14, call_at = frame - 8;
    Caller call{app, ts};
    const auto codelinkIs = [&](u32 name, u32 ebx) {
        return call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, item<ImageItem>(app, at).codelink, name, ebx);
    };
    if (!codelinkIs(kGlow, in.ebx))
        return {};
    if (call.r != 0)
        return glowDrawn(app, ts, call_at - 4, {call.r, kGlow, in.ebx, at});
    if (!codelinkIs(kLicence, in.ebx))
        return {};
    if (call.r != 0)
        return licenceDrawn(app, ts, call_at - 4, {call.r, kLicence, in.ebx, at});
    if (demoHides(app))
        return 0;
    ImageItem& m = item<ImageItem>(app, at);
    const int32_t x = wide(m.x), y = wide(m.y);
    bool drawPlain = false;
    if (m.shape != 0)
    {
        if (!codelinkIs(kMainEffects, in.ebx))
            return {};
        if (call.r != 0)
        {
            if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(m.shape)), u32(x), u32(y))
                || !call.plain<0x4a49c0, frontend::menud::drawScreenEffect>(call_at))
                return {};
        }
        else
        {
            if (!codelinkIs(kTextEffects, in.ebx))
                return {};
            if (call.r != 0)
            {
                if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(m.shape)), u32(x), u32(y))
                    || !call(call_at, kTextEffectsDraw, {call.r, u32(x), u32(y), at}))
                    return {};
            }
            else
            {
                if (!codelinkIs(kNfsLogo, in.ebx))
                    return {};
                if (call.r != 0)
                {
                    if (!call.plain<0x4a4c20, frontend::menud::drawLogo>(call_at, at, u32(x), u32(y)))
                        return {};
                }
                else
                {
                    if (!codelinkIs(kTrackImage, in.ebx))
                        return {};
                    if (call.r != 0)
                    {
                        if (gameSettings(app).track != kTrackShown(app))
                        {
                            const u32 shape = u32(wide(m.shape));
                            if (!call.plain<kBitmapShape, frontend::eab::shapeFileFind>(
                                    call_at, kArt(app), word(app, u32(gameSettings(app).track) * 4 + 0x559fb0))
                                || !call.core<kArtShapeTextures, shapes::artTexturesOf, 0>(call_at, call.r, int32_t(shape),
                                                                                           shape))
                                return {};
                            item<ImageItem>(app, at).shape = int16_t(call.r);
                            kTrackShown.set(app, std::int32_t(gameSettings(app).track));
                        }
                        drawPlain = true;
                    }
                    else if (m.alpha != 0)
                    {
                        int32_t ax = x;
                        const u32 colour = (u32(wide(m.alpha)) << 24) + 0xffffff;
                        if (kSlideMore(app) == 0)
                        {
                            const int32_t sliding = kSlidingIn(app);
                            if (sliding != 0)
                            {
                                const int32_t d = y - int32_t(word(app, 0x559c94)) + sliding - 0x12c;
                                if (d > 0x64)
                                    ax = 0x280;
                                else if (d > 0)
                                    ax += d;
                            }
                            else if (m.slide != 0)
                            {
                                if ((kRenderFlags(app) & 2) != 0)
                                    m.slide = 0;
                                const int16_t slide = int16_t(m.slide - 0x14);
                                m.slide = slide;
                                if (slide < 0)
                                {
                                    m.slide = 0;
                                    ax += wide(m.slide);
                                }
                                else
                                    ax += slide < 0x64 ? wide(m.slide) : 0x64;
                            }
                        }
                        if (!call.named<shapes::drawArt, 0>(call_at, u32(wide(m.shape)), u32(ax), u32(y), colour))
                            return {};
                    }
                    else
                    {
                        const u32 text = m.text;
                        u32 dark = 0;
                        if (text != 0)
                        {
                            if (!call.plain<0x4ee310, watcom::clib::strcmp>(call_at, text, kThreeButtons))
                                return {};
                            bool bar = call.r == 0;
                            if (!bar)
                            {
                                if (!call.plain<0x4ee310, watcom::clib::strcmp>(call_at, m.text, kTwoButtons))
                                    return {};
                                bar = call.r == 0;
                            }
                            if (bar)
                                dark = 0x78ffffff;
                            else
                            {
                                if (!call.plain<0x4ee310, watcom::clib::strcmp>(call_at, m.text, kBackground))
                                    return {};
                                if (call.r == 0)
                                    dark = 0xff787878;
                            }
                        }
                        if (dark != 0)
                        {
                            if (!call.named<shapes::drawArt, 0>(call_at, u32(wide(m.shape)), u32(x), u32(y), dark))
                                return {};
                        }
                        else
                        {
                            if (m.framed != 0)
                            {
                                if (!call.namedPlain<shapes::shapeSizeOf>(call_at, u32(wide(m.shape)), frame - 4, frame - 8))
                                    return {};
                                if (!call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(x), u32(y), u32(wide(half(app, frame - 4))), u32(wide(half(app, frame - 8))), kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3]))
                                    return {};
                            }
                            drawPlain = true;
                        }
                    }
                }
            }
        }
    }
    if (drawPlain
        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<ImageItem>(app, at).shape)), u32(x), u32(y)))
        return {};
    if (!codelinkIs(kBandEffects, in.ebx))
        return {};
    if (call.r != 0 && !call.plain<0x4a47c0, frontend::menud::drawBand>(call_at))
        return {};
    return 0;
}

/* 0x460410's: image `at` laid out: a box of 0x32 by 0x32 at its place. */
void imageLaidOut(App* app, u32 at)
{
    ImageItem& m = item<ImageItem>(app, at);
    m.boxHeight = 0x32;
    m.boxWidth = 0x32;
    m.boxLeft = m.x;
    m.boxTop = m.y;
}

/* 0x460440's: as image `at` is shown, its shape found -- for what its
 * codelink names, the effects started, the track's picture, the map's
 * pictures of the players' cars (mapImage, mapImage2) and a screen's picture
 * from its art file; else by its name, from its art file (fedata's, the
 * game's folder's) where it has one -- and its slide set going. */
Result imageShowing(App* app, ThreadState& ts, u32 esp, u32 at, const Args& in)
{
    constexpr Var<int32_t> kTrackShown{0x678870};
    constexpr Var<u32> kArt{0x559228};
    constexpr Var<int32_t> kMapCar{0x6788ec}, kMapCar2{0x6788e8};
    const u32 frame = esp - 0x18, call_at = frame - 0x50, text = frame - 0x50;
    Caller call{app, ts};
    ImageItem& m = item<ImageItem>(app, at);
    const auto codelinkIs = [&](u32 name) {
        return call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, item<ImageItem>(app, at).codelink, name, at);
    };
    const auto shapeOf = [&](u32 name) { return call.named<shapes::shapeNamed, 0>(call_at, name); };
    const auto loadArt = [&](u32 eax, u32 file) {
        if (!call.plain<kSprintf, watcom::clib::sprintf>(call_at, text, kTwoStrings,
                                                         guest::Words({kGameFolder, file})))
            return false;
        const Result shapes = shapesLoaded(app, ts, call_at - 4, text, at);
        if (!shapes)
            return false;
        kArt.set(app, u32(*shapes));
        call.r = u32(*shapes);
        return true;
    };
    if (m.codelink != 0)
    {
        if (!codelinkIs(kMainEffects))
            return {};
        if (call.r != 0)
        {
            if (!call.plain<0x4a4a10, frontend::menuf::startScreenEffects>(call_at, at, 0))
                return {};
            return int32_t(call.r);
        }
        if (!codelinkIs(kTextEffects))
            return {};
        if (call.r != 0 && !call.plain<0x4a4390, frontend::menud::startLogoTexture>(call_at))
            return {};
        if (!codelinkIs(kTrackImage))
            return {};
        if (call.r != 0)
        {
            if (!shapeOf(word(app, u32(gameSettings(app).track) * 4 + 0x559fb0)))
                return {};
            item<ImageItem>(app, at).shape = int16_t(call.r);
            kTrackShown.set(app, std::int32_t(gameSettings(app).track));
            return std::int32_t(gameSettings(app).track);
        }
        if (!codelinkIs(kMapImage))
            return {};
        if (call.r != 0)
        {
            u32 eax;
            if (gameSettings(app).playMode == 1)
            {
                kMapCar.set(app, int32_t((word(app, 0x678800) << 2) + word(app, 0x678860)));
                eax = word(app, 0x678860);
            }
            else
            {
                kMapCar.set(app, int32_t((word(app, 0x6787f8) << 2) + word(app, 0x678858)));
                eax = word(app, 0x678858);
            }
            const u32 saved = kArt(app);
            if (!loadArt(eax, item<ImageItem>(app, at).file))
                return {};
            if (!shapeOf(word(app, u32(kMapCar(app)) * 4 + 0x559cbc)))
                return {};
            kArt.set(app, saved);
            item<ImageItem>(app, at).shape = int16_t(call.r);
            return int32_t(call.r);
        }
        if (!codelinkIs(kMapImage2))
            return {};
        if (call.r != 0)
        {
            const u32 car = (word(app, 0x678808) << 2) + word(app, 0x678868);
            kMapCar2.set(app, int32_t(car));
            const u32 saved = kArt(app);
            if (!loadArt(car, item<ImageItem>(app, at).file))
                return {};
            if (word(app, 0x678808) == word(app, 0x678800) && word(app, 0x678868) == word(app, 0x678860))
            {
                const u32 other = kMapCar2(app) == 0 ? 1 : u32(kMapCar2(app)) - 1;
                if (!shapeOf(word(app, other * 4 + 0x559cbc)))
                    return {};
                item<ImageItem>(app, at).shape = int16_t(call.r);
                const u32 shape = u32(wide(item<ImageItem>(app, at).shape));
                if (!call.plain<kBitmapShape, frontend::eab::shapeFileFind>(
                        call_at, kArt(app), word(app, u32(kMapCar2(app)) * 4 + 0x559cbc))
                    || !call.core<kArtShapeTextures, shapes::artTexturesOf, 0>(call_at, call.r, int32_t(shape), at))
                    return {};
            }
            else if (!shapeOf(word(app, u32(kMapCar2(app)) * 4 + 0x559cbc)))
                return {};
            item<ImageItem>(app, at).shape = int16_t(call.r);
            kArt.set(app, saved);
            return int32_t(call.r);
        }
        if (!codelinkIs(kScreenImage))
            return {};
        if (call.r != 0)
        {
            const u32 saved = kArt(app);
            if (!loadArt(call.r, item<ImageItem>(app, at).file) || !shapeOf(item<ImageItem>(app, at).text))
                return {};
            kArt.set(app, saved);
            item<ImageItem>(app, at).shape = int16_t(call.r);
            return int32_t(call.r);
        }
    }
    ImageItem& n = item<ImageItem>(app, at);
    if (n.alpha != 0 && kScreenMode(app) != 9 && n.slide == 0)
        n.slide = int16_t(n.y - 0x46);
    const u32 name = n.text;
    n.unused46 = 0;
    if (name == 0)
        return 0;
    u32 eax = name;
    if (n.file != 0)
    {
        const u32 file = n.file;
        const u32 saved = kArt(app);
        if (byteAt(app, file) == 1)
        {
            if (!call.plain<kSprintf, watcom::clib::sprintf>(call_at, text, kFedataFile, guest::Words({file + 1})))
                return {};
        }
        else if (!call.plain<kSprintf, watcom::clib::sprintf>(call_at, text, kTwoStrings,
                                                              guest::Words({kGameFolder, file})))
            return {};
        const Result shapes = shapesLoaded(app, ts, call_at - 4, text, at);
        if (!shapes)
            return {};
        kArt.set(app, u32(*shapes));
        if (!call.named<shapes::shapeNamed, 0>(call_at, item<ImageItem>(app, at).text))
            return {};
        kArt.set(app, saved);
        eax = call.r;
    }
    else
    {
        if (!call.named<shapes::shapeNamed, 0>(call_at, eax))
            return {};
        eax = call.r;
    }
    item<ImageItem>(app, at).shape = int16_t(eax);
    return int32_t(eax);
}

/* ---- The pulse ---- */

constexpr Var<int32_t> kPulseStep{0x559fdc}, kPulseNext{0x559fe0};

/* 0x460700's: the pulse stepped on (8 of 0x200) when the clock is past its
 * next, the next 5 on in steps of 5 from it. */
int32_t pulsed(App* app)
{
    const int32_t clock = kClock(app);
    if (clock < kPulseNext(app))
        return clock;
    kPulseStep.set(app, remainderOf(kPulseStep(app) + 8, 0x200));
    const int32_t from = clock - kPulseNext(app);
    kPulseNext.set(app, clock + 5 - remainderOf(from, 5));
    return int32_t(int64_t(from) / 5);
}

/* 0x460770's: `alpha` (of 0xff) pulsing: its share of the pulse's height,
 * white. */
u32 pulseOf(App* app, int32_t alpha)
{
    const int32_t step = kPulseStep(app);
    const int32_t height = step < 0x100 ? step : 0x1ff - step;
    return (u32(int32_t(int64_t(int32_t(u32(alpha) * u32(height))) / 0xff)) << 24) + 0xffffff;
}

/* ---- The button ---- */

constexpr Var<int32_t> kHighlit{0x559c64};
constexpr Var<u32> kQuadTexture{0x749a38};

/* A button's style's word, dword. */
int16_t styleHalf(App* app, u32 style, u32 offset)
{
    return half(app, style + offset);
}

int32_t styleWord(App* app, u32 style, u32 offset)
{
    return int32_t(word(app, style + offset));
}

/* The button's text into the frame's `text` (from the code's zeros): its
 * text, " ..." after it where it opens a menu without a list, or names the
 * controls' or the graphics' screen (and, drawn, its text is not the
 * language's "back"). */
Result buttonText(App* app, ThreadState& ts, u32 at, u32 item_, u32 text, bool drawn)
{
    ButtonItem& b = item<ButtonItem>(app, item_);
    if (b.text == 0)
    {
        app->getMemory<uint8_t>(text) = 0;
        return 0;
    }
    copyString(app, text, b.text);
    const u32 ellipsis = gameSettings(app).language == 1 ? kEllipsisGerman : kEllipsis;
    if (b.nextMenu != 0 && b.listName == 0)
    {
        appendString(app, text, ellipsis);
        return 0;
    }
    if (b.styleName == 0)
        return 0;
    Caller call{app, ts};
    if (!call.plain<0x4ee310, watcom::clib::strcmp>(at, b.styleName, kControlsScreen))
        return {};
    if (call.r != 0)
    {
        if (!call.plain<0x4ee310, watcom::clib::strcmp>(at, item<ButtonItem>(app, item_).styleName, kGraphicsScreen))
            return {};
        if (call.r != 0)
            return 0;
    }
    if (drawn)
    {
        if (!call.lineByName(at, 0xc6, item_))
            return {};
        if (!call.plain<0x4ee310, watcom::clib::strcmp>(at, item<ButtonItem>(app, item_).text, call.r))
            return {};
        if (call.r == 0)
            return 0;
    }
    appendString(app, text, gameSettings(app).language == 1 ? kEllipsisGerman : kEllipsis);
    return 0;
}

/* 0x4607c0's: button `at` drawn, lit under the pointer: its text placed by
 * its style (0x559fe4, 0x40 bytes each) and alignment, slid in with the
 * screen or by itself; by its style a panel behind it, a title's bar (the
 * race's pause screens' quads, or two shaded lines), its glow and pulse when
 * lit or its shape; a network race's ready button and its marks; the line
 * of its list beside it.  0. */
Result buttonDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    const u32 frame = esp - 0x14, call_at = frame - 0x14c, text = frame - 0x14c;
    copyBlock(app, text, 0x45cbd0, 0x100);
    if (demoHides(app))
        return 0;
    demoGreys(app, at);
    ButtonItem& b = item<ButtonItem>(app, at);
    const u32 colour = (b.flags & 1) != 0 ? kGrey : lit != 0 ? kYellow : 0xff649dfd;
    const u32 style = u32(wide(b.style)) << 6;
    int16_t x = int16_t(b.x + styleHalf(app, style, 0x559fe8));
    const int16_t y = int16_t(b.y + styleHalf(app, style, 0x559fec));
    Caller call{app, ts};
    if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont))
        return {};
    if (!buttonText(app, ts, call_at, at, text, true))
        return {};
    int32_t width;
    if (b.style == 3 && kOverRace(app) != 0)
    {
        if (!call.plain<0x4d1400, text::keepTextScaleOf>(call_at) || !(setWord(app, call_at - 4, 0x3f19999a), call.plain<0x451530, text::dimmed>(call_at, call_at - 4, call.r))
            || !call.named<text::menuTextWidth, 0>(call_at, text, kMenuFont))
            return {};
        width = int32_t(call.r);
        if (!call.keep<0x4d1410, text::restoreTextScaleOf>(call_at, call.r))
            return {};
    }
    else
    {
        if (!call.named<text::menuTextWidth, 0>(call_at, text, kMenuFont))
            return {};
        width = int32_t(call.r);
    }
    if (b.style != 0xf)
    {
        if (b.align == 1)
            x = int16_t(x - int16_t(width));
        else if (b.align == 2)
            x = int16_t(wide(x) - halved(width));
    }
    if (lit != 0)
    {
        kHighlit.set(app, 0);
        if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
            return {};
        if ((call.r & 0xff) == 0 && (b.style == 1 || (b.style == 2 && b.nextMenu != 0)))
            kHighlit.set(app, 1);
    }
    const uint16_t kind = uint16_t(b.style);
    if (kind == 0 || kind == 0xf)
    {
        if (kOverRace(app) == 0
            && !call.core<0x4d89f0, draw2d::panel, 0>(call_at, u32(wide(x) - 1), u32(wide(y)), u32(width + 2), kMenuFont))
            return {};
    }
    else if (kind == 1 || kind == 2)
    {
        const int32_t sliding = kSlidingIn(app);
        if (sliding != 0)
        {
            const int32_t d = wide(b.y) - int32_t(word(app, 0x559c94)) + sliding - 0x12c;
            if (d > 0x64)
                x = 0x280;
            else if (d > 0)
                x = int16_t(x + int16_t(d));
        }
        else if (b.slide > 0)
        {
            const int16_t slide = b.slide;
            int32_t by;
            if ((kRenderFlags(app) & 2) != 0 || slide <= 0x14)
            {
                b.slide = 0;
                by = wide(b.slide);
            }
            else
            {
                const int16_t less = int16_t(slide - 0x14);
                b.slide = less;
                by = less < 0x64 ? wide(b.slide) : 0x64;
            }
            x = int16_t(wide(x) + by);
        }
    }
    else if (kind == 3)
    {
        if (kOverRace(app) != 0)
        {
            if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
                return {};
            if ((call.r & 0xff) == 0)
            {
                // The title's bar on the race's pause screens: three quads of
                // the menus' art, its ends and its middle.
                const int32_t left = wide(x) - 5, top = wide(y) - 1;
                const int32_t a = chopped<int32_t>(ts, fp::add(Float(y), Float(floatAt(app, 0x53a9d8))));
                const int32_t c0 = chopped<int32_t>(ts, fp::add(Float(left), Float(floatAt(app, 0x53a9d4))));
                if (!call.core<0x4db000, draw2d::texturedQuad, 14>(
                        call_at,
                        draw2d::ArtQuad{{c0, left, left, c0},
                                        {top, top, a, a},
                                        {0x3f6a0000, 0x3f740000, 0x3f740000, 0x3f6a0000},
                                        {0x3f6a0000, 0x3f6a0000, 0x3f7f8000, 0x3f7f8000},
                                        kQuadTexture(app),
                                        0xffffffff}))
                    return {};
                const int32_t right = width + 0xa + left;
                const float topF = float(Float(top));
                const float edge = float(fp::add(Float(topF), Float(floatAt(app, 0x53a9dc))));
                const int32_t c = chopped<int32_t>(ts, Float(edge));
                if (!call.core<0x4db000, draw2d::texturedQuad, 14>(
                        call_at,
                        draw2d::ArtQuad{{left, right, right, left},
                                        {top, top, c, c},
                                        {0x3f7f0000, 0x3f7f0000, 0x3f7f0000, 0x3f7f0000},
                                        {0x3f6a0000, 0x3f6a0000, 0x3f800000, 0x3f800000},
                                        kQuadTexture(app),
                                        0xffffffff}))
                    return {};
                const int32_t d = chopped<int32_t>(ts, fp::add(Float(right), Float(floatAt(app, 0x53a9e0))));
                if (!call.core<0x4db000, draw2d::texturedQuad, 14>(
                        call_at,
                        draw2d::ArtQuad{{right, d, d, right},
                                        {top, top, c, c},
                                        {0x3f740000, 0x3f7e0000, 0x3f7e0000, 0x3f740000},
                                        {0x3f6a0000, 0x3f6a0000, 0x3f800000, 0x3f800000},
                                        kQuadTexture(app),
                                        0xffffffff}))
                    return {};
            }
        }
        else
        {
            const u32 shade = lit != 0 ? 0xffd84b : 0x727eb3, dark = shade - 0x1000000;
            const int32_t left = wide(x) + 0xe;
            if (!call.boxedNamed<draw2d::shadedBox>(call_at, u32(left), u32(wide(y)), u32(wide(b.boxWidth) - 5), 1, dark, shade, shade, dark)
                || !call.boxedNamed<draw2d::shadedBox>(call_at, u32(left), u32(wide(y) + 0xf), u32(wide(item<ButtonItem>(app, at).boxWidth)), 1, dark, shade, shade, dark))
                return {};
        }
    }
    ButtonItem& c = item<ButtonItem>(app, at);
    const u32 s = u32(wide(c.style)) << 6;
    if (lit != 0)
    {
        if (kOverRace(app) == 0)
        {
            const u32 glow = word(app, s + 0x559ff8);
            if (c.glow != 0)
            {
                if (glow == 0xff)
                {
                    if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(c.glow)), u32(wide(x)), u32(wide(y))))
                        return {};
                }
                else if (!call.named<shapes::drawArt, 0>(call_at, u32(wide(c.glow)), u32(wide(x)), u32(wide(y)), (glow << 16) - 0x1000000 + (glow << 8) + glow))
                    return {};
            }
            if (item<ButtonItem>(app, at).pulse != 0 && kSlidingIn(app) == 0)
            {
                if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
                    return {};
                if ((call.r & 0xff) == 0 && (kRenderFlags(app) & 2) == 0)
                {
                    const u32 pulse = pulseOf(app, int32_t(glow));
                    pulsed(app);
                    const u32 s2 = u32(wide(item<ButtonItem>(app, at).style)) << 6;
                    if (!call.named<shapes::drawArt, 0>(call_at, u32(wide(item<ButtonItem>(app, at).pulse)), u32(wide(x) + styleWord(app, s2, 0x55a000)), u32(wide(y) + styleWord(app, s2, 0x55a004)), pulse))
                        return {};
                }
            }
        }
    }
    else
    {
        int32_t left = wide(x);
        const u32 name = word(app, s + 0x559ff0);
        if (name != 0)
        {
            if (!call.plain<0x4ee310, watcom::clib::strcmp>(call_at, name, kSmallBack))
                return {};
            if (call.r == 0)
                left -= 5;
        }
        if (kOverRace(app) == 0 && item<ButtonItem>(app, at).shape != 0
            && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<ButtonItem>(app, at).shape)), u32(left), u32(wide(y))))
            return {};
    }
    // A network race's start: the host's when every machine is ready, the
    // others' ready or not with their marks.
    if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, item<ButtonItem>(app, at).codelink, kRaceNow, 0))
        return {};
    if (call.r != 0 && kNetworkRace(app) != 0)
    {
        if (kOverRace(app) != 0)
            return 0;
        const u32 s3 = u32(wide(item<ButtonItem>(app, at).style)) << 6;
        const int32_t tx = wide(x) + styleWord(app, s3, 0x55a008), ty = wide(y) + styleWord(app, s3, 0x55a00c);
        const u32 align = word(app, s3 + 0x55a010);
        if (kNetworkHost(app) != 0)
        {
            bool ready = false;
            bool ask = true;
            if (word(app, 0x662968) != 0)
            {
                if (!call.plain<0x4cf090, frontend::misc::postgameRaceAgain>(call_at))
                    return {};
                ask = call.r != 0;
            }
            if (ask)
            {
                if (!call.plain<0x49ab60, frontend::glue::allPlayersReady>(call_at))
                    return {};
                ready = call.r != 0;
            }
            if (ready)
                item<ButtonItem>(app, at).flags &= 0xfe;
            else
                item<ButtonItem>(app, at).flags |= 1;
            if (!call.lineByName(call_at, 0x1bb, u32(tx))
                || !call.named<text::menuTextDrawn, 2>(call_at, call.r, u32(tx), u32(ty), kMenuFont, align, colour))
                return {};
            return 0;
        }
        if (!call.lineByName(call_at, 0x26, u32(tx))
            || !call.named<text::menuTextDrawn, 2>(call_at, call.r, u32(tx), u32(ty), kMenuFont, align, colour))
            return {};
        if (!readyMarks(app, ts, call_at - 4, wide(x) + 8, wide(y) + 8))
            return {};
        return 0;
    }
    ButtonItem& d = item<ButtonItem>(app, at);
    if (d.text != 0)
    {
        if (kOverRace(app) != 0 && d.style == 3)
        {
            if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
                return {};
            if ((call.r & 0xff) == 0)
            {
                if (!call.plain<0x4d1400, text::keepTextScaleOf>(call_at) || !(setWord(app, call_at - 4, 0x3f19999a), call.plain<0x451530, text::dimmed>(call_at, call_at - 4, call.r))
                    || !call.named<text::menuTextDrawn, 2>(call_at, text, u32(wide(x)), u32(wide(y)), kMenuFont, 0, colour)
                    || !call.keep<0x4d1410, text::restoreTextScaleOf>(call_at, 0))
                    return {};
            }
        }
        else
        {
            bool hidden = false;
            if (kOverRace(app) != 0)
            {
                if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
                    return {};
                hidden = (call.r & 0xff) != 0;
            }
            if (!hidden)
            {
                const u32 s4 = u32(wide(item<ButtonItem>(app, at).style)) << 6;
                if (!call.named<text::menuTextDrawn, 2>(call_at, text, u32(wide(x) + styleWord(app, s4, 0x55a008)), u32(wide(y) + styleWord(app, s4, 0x55a00c)), kMenuFont, word(app, s4 + 0x55a010), colour))
                    return {};
            }
        }
    }
    // A list's button: its line beside it.
    if (item<ButtonItem>(app, at).style != 0xf)
        return 0;
    if (kOverRace(app) != 0)
    {
        if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, call.r))
            return {};
        if ((call.r & 0xff) != 0)
            return 0;
    }
    ButtonItem& e = item<ButtonItem>(app, at);
    const u32 list = e.listName;
    if (list == 0 || byteAt(app, list) == 0)
        return 0;
    const int32_t gap = e.listGap > 0 ? wide(e.listGap) : 0x32;
    int32_t lx, ly;
    const uint8_t where = uint8_t(e.listAlign);
    if (where == 1)
        lx = wide(x) + gap, ly = wide(y);
    else if (where == 2)
        lx = wide(e.listX), ly = wide(e.listY);
    else
        lx = wide(x) - gap, ly = wide(y);
    const Result lines = linesOf(app, ts, call_at - 4, list, frame - 0x3c);
    if (!lines)
        return {};
    if (*lines == 0)
        return 0;
    const u32 chosen = word(app, word(app, frame - 0x3c));
    if (!call.lineByName(call_at, u32(wide(half(app, u32(*lines) + chosen * 2))), chosen))
        return {};
    const u32 line = call.r;
    if (line == 0)
        return 0;
    bool copied = false;
    if (kOverRace(app) != 0)
    {
        setWord(app, frame - 0x4c, 0);
        setWord(app, frame - 0x48, 0);
        if (!call.plain<0x4beb20, frontend::pause::readRect>(call_at - 4, 0x7cdac0, frame - 0x4c, frame - 0x4c, frame - 0x48, frame - 0x4c))
            return {};
        if (int32_t(word(app, frame - 0x48)) < 0x280)
        {
            const u32 n = length(app, line);
            if (n >= 0x19)
            {
                if (!call.plain<0x4e0e30, watcom::clib::strncpy>(call_at, text, line, 0x19))
                    return {};
                appendString(app, text, kEllipsis);
                copied = true;
            }
        }
    }
    if (!copied)
        copyString(app, text, line);
    if (!call.named<text::menuTextDrawn, 2>(call_at, text, u32(lx), u32(ly), kMenuFont, 0, colour))
        return {};
    return 0;
}

Result menuItemLaidOut(App* app, ThreadState& ts, u32 esp, u32 at);

}  // namespace

/* 0x461270's: key `key` on button `at`.  With a list (+0x58), Enter steps
 * its line to the next that may be chosen, round to the first (none other:
 * it stays), a list's button laid out again and its handler told: 2 or its
 * answer.  Without, Enter calls its handler -- its answer -- or on racenow
 * starts the race (6), over a network this machine ready or not (the host
 * once every machine is), 0; on exitprogram 10; with a menu to open 5; else
 * 3.  Other keys 0. */
Result buttonKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    const u32 frame = esp - 0x10, call_at = frame - 0x10;
    Caller call{app, ts};
    ButtonItem& b = item<ButtonItem>(app, at);
    const u32 list = b.listName;
    if (list == 0)
    {
        if (uint16_t(key) != 0xd)
            return 0;
        if (b.handler != 0)
        {
            if (!call(call_at, b.handler, {at, 0, 0, at}))
                return {};
            return int32_t(call.r);
        }
        if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, b.codelink, kRaceNow, 0))
            return {};
        if (call.r != 0)
        {
            if (kNetworkRace(app) == 0)
                return 6;
            if (kNetworkHost(app) != 0)
            {
                if (!call.plain<0x49ab60, frontend::glue::allPlayersReady>(call_at))
                    return {};
                if (call.r != 0)
                    gameSettings(app).networkLobby = 1;
                return 0;
            }
            gameSettings(app).networkLobby = gameSettings(app).networkLobby == 0 ? 1 : 0;
            return 0;
        }
        if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, item<ButtonItem>(app, at).codelink, kExitProgram, 0))
            return {};
        if (call.r != 0)
            return 0xa;
        return item<ButtonItem>(app, at).nextMenu != 0 ? 5 : 3;
    }
    const u32 chosenAt = frame - 0x10;
    const Result lines = linesOf(app, ts, call_at - 4, list, chosenAt);
    if (!lines)
        return {};
    const Result allowed = allowedOf(app, ts, call_at - 4, item<ButtonItem>(app, at).listName);
    if (!allowed)
        return {};
    if (uint16_t(key) != 0xd)
        return 0;
    const u32 chosen = word(app, chosenAt);
    const u32 start = word(app, chosen);
    for (;;)
    {
        setWord(app, chosen, word(app, chosen) + 1);
        if (half(app, u32(*lines) + word(app, chosen) * 2) == 0)
            setWord(app, chosen, 0);
        if (*allowed == 0 || half(app, u32(*allowed) + word(app, chosen) * 2) != 0 || start == word(app, chosen))
            break;
    }
    if (item<ButtonItem>(app, at).style != 0xf)
        return 2;
    if (!menuItemLaidOut(app, ts, call_at - 4, at))
        return {};
    const u32 handler = item<ButtonItem>(app, at).handler;
    if (handler == 0)
        return 2;
    if (!call(call_at, handler, {at, u32(*allowed), 2, at}))
        return {};
    return int32_t(call.r);
}

namespace
{

/* 0x4613e0's: menu item `at` laid out -- a button's box: its place and size
 * from its style's and its own, and from its text (with " ..." where it opens
 * a menu without a list, or names the controls' or the graphics' screen)
 * measured in the menus' font, a line of the fonts high on the race's pause
 * screens; a title (3) there in their scale 0.6 with a margin, a list's
 * button (0xf) as wide as its list's line.  Then placed by its alignment.  No
 * caller reads its eax: its result is not the game's. */
Result menuItemLaidOut(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 frame = esp - 0x18, call_at = frame - 0x114, text = frame - 0x114;
    copyBlock(app, text, 0x45ccd0, 0x100);
    setWord(app, frame - 0xc, 0);
    setWord(app, frame - 8, 0);
    ButtonItem& b = item<ButtonItem>(app, at);
    const u32 s = u32(wide(b.style)) << 6;
    b.boxLeft = int16_t(styleHalf(app, s, 0x559fe8) + styleHalf(app, s, 0x55a014) + b.x);
    b.boxTop = int16_t(styleHalf(app, s, 0x559fec) + styleHalf(app, s, 0x55a018) + b.y);
    b.boxWidth = styleHalf(app, s, 0x55a01c);
    b.boxHeight = styleHalf(app, s, 0x55a020);
    if (!buttonText(app, ts, call_at, at, text, false))
        return {};
    Caller call{app, ts};
    // A line of the fonts' text's height on the 3D card: an 'A''s height and
    // its drop each times [0x53a9f4] over the screen's height, whole pixels.
    const auto lineHeight = [&]() {
        if (!call.plain<0x4d1540, text::characterHeight>(call_at, 0x41u))
            return false;
        const Float scale = Float(doubleAt(app, 0x53a9f4));
        const Float screen = Float(int32_t(word(app, 0x7cdacc)));
        const double height = double(fp::div(fp::mul(Float(int32_t(call.r)), scale), screen));
        if (!call.plain<0x4d1590, text::characterDrop>(call_at, 0x41u))
            return false;
        const Float drop = fp::div(fp::mul(Float(int32_t(call.r)), scale), screen);
        item<ButtonItem>(app, at).boxHeight = int16_t(chopped<int32_t>(ts, fp::add(drop, Float(height))));
        return true;
    };
    const uint16_t kind = uint16_t(b.style);
    if (kind == 3)
    {
        if (kOverRace(app) != 0)
        {
            if (!call.plain<0x4d1400, text::keepTextScaleOf>(call_at) || !(setWord(app, call_at - 4, 0x3f19999a), call.plain<0x451530, text::dimmed>(call_at, call_at - 4, call.r)))
                return {};
            ButtonItem& t = item<ButtonItem>(app, at);
            t.boxTop = int16_t(t.boxTop - 1);
            if (!call.named<text::menuTextWidth, 0>(call_at, text, kMenuFont))
                return {};
            item<ButtonItem>(app, at).boxHeight = 0x16;
            item<ButtonItem>(app, at).boxWidth =
                int16_t(chopped<int32_t>(ts, fp::add(Float(int32_t(call.r) + 0xa), Float(floatAt(app, 0x53a9f0)))));
            if (!call.keep<0x4d1410, text::restoreTextScaleOf>(call_at, 0))
                return {};
        }
    }
    else if (kind <= 2 || kind == 0xf)
    {
        if (kind == 0)
        {
            if (!call.named<text::menuTextWidth, 0>(call_at, text, kMenuFont))
                return {};
            item<ButtonItem>(app, at).boxWidth = int16_t(call.r);
        }
        if (kOverRace(app) != 0 && !lineHeight())
            return {};
    }
    ButtonItem& p = item<ButtonItem>(app, at);
    if (p.style == 0xf)
    {
        const int32_t left = wide(p.boxWidth);
        const u32 list = p.listName;
        if (list == 0 || byteAt(app, list) == 0)
            return 0;
        const Result lines = linesOf(app, ts, call_at - 4, list, frame - 0xc);
        if (!lines)
            return {};
        if (*lines != 0)
        {
            const u32 chosen = word(app, word(app, frame - 0xc));
            if (!call.lineByName(call_at, u32(wide(half(app, u32(*lines) + chosen * 2))), chosen))
                return {};
            if (call.r != 0)
            {
                if (!call.named<text::menuTextWidth, 0>(call_at, call.r, kMenuFont))
                    return {};
                setWord(app, frame - 8, call.r);
            }
        }
        ButtonItem& q = item<ButtonItem>(app, at);
        const int32_t gap = q.listGap > 0 ? wide(q.listGap) : 0x32;
        const uint8_t where = uint8_t(q.listAlign);
        if (where == 1)
            q.boxWidth = int16_t(int32_t(word(app, frame - 8)) + gap);
        else if (where != 2)
        {
            q.boxLeft = int16_t(q.x - int16_t(gap));
            q.boxWidth = int16_t(gap + left);
        }
        return 0;
    }
    if (p.align == 1)
    {
        if (p.style != 3 || kOverRace(app) == 0)
        {
            p.boxLeft = int16_t(p.boxLeft - p.boxWidth);
            return 0;
        }
        p.boxLeft = int16_t(chopped<int32_t>(
            ts, fp::add(Float(wide(p.boxLeft) - wide(p.boxWidth) + 5), Float(floatAt(app, 0x53a9fc)))));
        return 0;
    }
    if (p.align == 2)
    {
        p.boxLeft = int16_t(wide(p.boxLeft) - halved(wide(p.boxWidth)));
        return 0;
    }
    if (p.style != 3 || kOverRace(app) == 0)
        return 0;
    p.boxLeft = int16_t(chopped<int32_t>(ts, fp::add(Float(wide(p.boxLeft) - 5), Float(floatAt(app, 0x53aa00)))));
    return 0;
}

/* 0x461900's: as button `at` is shown: its style found by name (0 if none),
 * its style's shapes (the art's; none over a race, the pulse none on a card
 * with alpha), placed by the race's table in the styles 1 and 2 and slid in
 * from below, greyed where only the host may press it. */
Result buttonShowing(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0x18;
    Caller call{app, ts};
    ButtonItem& b = item<ButtonItem>(app, at);
    if (b.styleName == 0)
        b.style = 0;
    else
    {
        int32_t i = 0;
        for (;; ++i)
        {
            const u32 name = word(app, (u32(i) << 6) + kStyles);
            if (name == 0)
                break;
            if (!call.plain<0x4ee310, watcom::clib::strcmp>(call_at, name, item<ButtonItem>(app, at).styleName))
                return {};
            if (call.r == 0)
            {
                item<ButtonItem>(app, at).style = int16_t(i);
                break;
            }
        }
        if (word(app, (u32(i) << 6) + kStyles) == 0)
            item<ButtonItem>(app, at).style = 0;
    }
    // The style's shapes by name: its shape, its glow, its pulse (none on a
    // card with alpha), none over a race.
    const u32 shapeNames[] = {0x559ff0, 0x559ff4, 0x559ffc};
    const u32 shapeFields[] = {0x4c, 0x4e, 0x50};
    for (int i = 0; i < 3; ++i)
    {
        const u32 name = word(app, (u32(wide(item<ButtonItem>(app, at).style)) << 6) + shapeNames[i]);
        int16_t shape = 0;
        if (name != 0 && kOverRace(app) == 0 && (i < 2 || (kRenderFlags(app) & 2) == 0))
        {
            if (!call.named<shapes::shapeNamed, 0>(call_at, name))
                return {};
            shape = int16_t(call.r);
        }
        setHalf(app, at + shapeFields[i], shape);
    }
    ButtonItem& c = item<ButtonItem>(app, at);
    if (c.style == 1 || c.style == 2)
    {
        c.x = kPlaceX(app, wide(c.place));
        const int16_t y = kPlaceY(app, wide(c.place));
        c.y = y;
        if (kScreenMode(app) != 9 && c.slide == 0)
            c.slide = int16_t(y - 0x46);
    }
    const u32 codelink = item<ButtonItem>(app, at).codelink;
    if (codelink == 0 || kNetworkRace(app) == 0 || kNetworkHost(app) != 0)
        return int32_t(codelink);
    if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, codelink, kHostOnly, 0))
        return {};
    if (call.r != 0)
        item<ButtonItem>(app, at).flags |= 1;
    return int32_t(call.r);
}

/* ---- The cascade ---- */

/* 0x461a70's: a closed cascade's box: 0x1e by 0x1b, a little left of it. */
void cascadeClosedBox(App* app, u32 at)
{
    Item& c = item<Item>(app, at);
    c.boxHeight = 0x1b;
    c.boxWidth = 0x1e;
    c.boxLeft = int16_t(c.x - 0xa);
    c.boxTop = c.y;
}

/* 0x461aa0's: the line of cascade `at` chosen ([chosenAt]) of its `lines`
 * into `out`: its text -- and an opponents' or a car's class, (A) to (C);
 * the tracks' its laps ("%s (%d %s)").  sprintf's eax. */
Result cascadeLine(App* app, ThreadState& ts, u32 esp, u32 at, u32 out, u32 lines, u32 chosenAt)
{
    const u32 frame = esp - 0xc, call_at = frame - 0xc;
    setWord(app, frame - 8, at);
    setWord(app, frame - 4, out);
    setWord(app, frame - 0xc, lines);
    Caller call{app, ts};
    const auto lineText = [&](u32 edx, u32 ebx, u32 ecx) {
        return call.lineByName(call_at, u32(wide(half(app, word(app, chosenAt) * 2 + lines))), edx);
    };
    if (!lineText(lines, chosenAt, chosenAt)
        || !call.plain<kSprintf, watcom::clib::sprintf>(call_at, out, kPercentS, guest::Words({call.r})))
        return {};
    u32 eax = call.r;
    const auto stricmp = [&](u32 name) {
        return call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<CascadeItem>(app, at).listName, name);
    };
    if (std::int32_t(gameSettings(app).playMode) <= 1)
    {
        u32 kindOf = 0;
        bool known = false;
        if (!stricmp(kOpponentCars))
            return {};
        if (call.r == 0)
        {
            const int32_t car = int32_t(word(app, chosenAt));
            if (car >= 4)
            {
                if (!call.plain<0x439730, load::classOfCar>(call_at, u32(car - 4)))
                    return {};
                kindOf = call.r;
                known = true;
            }
        }
        else
        {
            if (!stricmp(kSecondCar))
                return {};
            u32 player = 0;
            bool asked = false;
            if (call.r == 0)
                player = gameSettings(app).players[1].car, asked = true;
            else
            {
                if (!stricmp(kCars))
                    return {};
                if (call.r == 0)
                    player = gameSettings(app).players[0].car, asked = true;
            }
            if (asked)
            {
                if (!call.plain<0x4396b0, load::policeFlag>(call_at, player))
                    return {};
                if (call.r == 0)
                {
                    if (!call.plain<0x439730, load::classOfCar>(call_at, player))
                        return {};
                    kindOf = call.r;
                    known = true;
                }
            }
        }
        if (known && kindOf <= 2)
            appendString(app, out, kindOf == 0 ? kClassA : kindOf == 1 ? kClassB : kClassC);
        eax = call.r;
    }
    if (!stricmp(kTracks))
        return {};
    eax = call.r;
    if (call.r != 0)
        return int32_t(eax);
    const u32 laps = gameSettings(app).laps;
    if (!call.lineByName(call_at, 5, kTracks))
        return {};
    const u32 lapsText = call.r;
    if (gameSettings(app).raceType == 2 && gameSettings(app).seriesVariant == 1)
    {
        if (!call.lineByName(call_at, 0xac, kTracks))
            return {};
    }
    else if (!lineText(kTracks, chosenAt, laps))
        return {};
    if (!call.plain<kSprintf, watcom::clib::sprintf>(call_at, out, kCountedText, guest::Words({call.r, laps, lapsText})))
        return {};
    return int32_t(call.r);
}

Result cascadeLaidOut(App* app, ThreadState& ts, u32 esp, u32 at);

/* An open list's box as wide as its widest line in the menus' font, 0x28 at
 * least. */
Result widestLine(App* app, ThreadState& ts, u32 at, u32 item_, u32 lines)
{
    Caller call{app, ts};
    item<Item>(app, item_).boxWidth = 0x28;
    for (u32 i = 0;; ++i)
    {
        const int16_t line = half(app, lines + i * 2);
        if (line == 0)
            break;
        if (!call.lineByName(at, u32(wide(line)), kMenuFont)
            || !call.named<text::menuTextWidth, 0>(at, call.r, kMenuFont))
            return {};
        if (int32_t(call.r) > wide(item<Item>(app, item_).boxWidth))
            item<Item>(app, item_).boxWidth = int16_t(call.r);
    }
    return 0;
}

/* The tracks' marks beside a line of the tracks' lists (a race of three
 * players' choices, 0x6fd2b0 and 0x6fd2b2 a bit a line): their shapes. */
Result trackMarks(App* app, ThreadState& ts, u32 at, int32_t line, int32_t left, int32_t top, u32 first, u32 second)
{
    Caller call{app, ts};
    const int32_t bit = 1 << (line & 31);
    if ((wide(gameSettings(app).duelsLost) & bit) != 0
        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(at, u32(wide(half(app, first))), u32(left - 0x21), u32(top + 2)))
        return {};
    if ((wide(gameSettings(app).duelsWon) & bit) != 0
        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(at, u32(wide(half(app, second))), u32(left - 0x11), u32(top + 2)))
        return {};
    return 0;
}

/* The lines of an open list's box -- `chosen`, `first`, `last`, `shown` its
 * words -- followed by the pointer: off its top or bottom (or the box), the
 * lines scrolled a line every 16 ticks (0x55a424, 0x55a434), the menus'
 * click when they move; over a line, that line chosen when the pointer is in
 * the box.  Then the chosen kept among those shown. */
struct ListBox
{
    u32 chosen, shown, first, last, most;
};

Result followPointer(App* app, ThreadState& ts, u32 at, u32 item_, const ListBox& w, int32_t count, u32 scrolled)
{
    Caller call{app, ts};
    Item& it = item<Item>(app, item_);
    if (!call.plain<0x49c1c0, frontend::menud::pointerListLine>(at, int32_t(u32(wide(it.boxTop))), 0x10))
        return {};
    const int32_t line = int32_t(call.r);
    if (line < 0 || line >= wide(half(app, item_ + w.shown)))
    {
        const int32_t clock = kClock(app);
        if (int32_t(word(app, scrolled)) + 0x10 < clock)
        {
            setWord(app, scrolled, u32(clock));
            const int32_t was = wide(half(app, item_ + w.first));
            const int32_t most = count - 1;
            if (line < 0)
            {
                int32_t first = was - 1;
                first = first < 0 ? 0 : first > most ? most : first;
                setHalf(app, item_ + w.first, int16_t(first));
                setHalf(app, item_ + w.last, int16_t(first + int16_t(half(app, item_ + w.shown) - 1)));
            }
            else if (wide(half(app, item_ + w.shown)) <= line)
            {
                int32_t last = wide(half(app, item_ + w.last)) + 1;
                last = last < 0 ? 0 : last > most ? most : last;
                setHalf(app, item_ + w.last, int16_t(last));
                setHalf(app, item_ + w.first, int16_t(last - int16_t(half(app, item_ + w.shown) - 1)));
            }
            if (wide(half(app, item_ + w.first)) != was
                && !call.effectByName(at, 0, 0x7f))
                return {};
        }
    }
    else
    {
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(at, item_ + 0x1a))
            return {};
        if (call.r != 0 && line + wide(half(app, item_ + w.first)) != wide(half(app, item_ + w.chosen)))
        {
            const int16_t chosen = int16_t(line + half(app, item_ + w.first));
            setHalf(app, item_ + w.chosen, chosen);
            int32_t to = 0;
            if (chosen >= 0)
            {
                to = count - 1;
                if (wide(half(app, item_ + w.chosen)) <= to)
                    to = wide(half(app, item_ + w.chosen));
            }
            setHalf(app, item_ + w.chosen, int16_t(to));
        }
    }
    if (wide(half(app, item_ + w.first)) > wide(half(app, item_ + w.chosen)))
        setHalf(app, item_ + w.chosen, half(app, item_ + w.first));
    if (wide(half(app, item_ + w.last)) < wide(half(app, item_ + w.chosen)))
        setHalf(app, item_ + w.chosen, half(app, item_ + w.last));
    return 0;
}

/* 0x461d00's: cascade `at` drawn, lit under the pointer (`lit`).  Open: its
 * box, its lines from the first shown -- may be chosen or greyed, in their
 * colours, the chosen yellow, the tracks' marks -- the arrows to scroll, its
 * picture and its title.  Closed and lit: its picture, its line chosen
 * beside it; not lit: closed for good, laid out, its line (not while the
 * screens slide in, unless "donthide").  0. */
Result cascadeDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    if (demoHides(app))
        return 0;
    const u32 frame = esp - 0x14, call_at = frame - 0x238;
    Caller call{app, ts};
    if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<CascadeItem>(app, at).listName, kTracks))
        return {};
    const int32_t gap = call.r == 0 && gameSettings(app).raceType == 3 ? 0x23 : 0xf;
    const Result allowed = allowedOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName);
    if (!allowed)
        return {};
    const Result colours = coloursOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName);
    if (!colours)
        return {};
    const u32 chosenAt = frame - 0x38;
    const Result lines = linesOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName, chosenAt);
    if (!lines)
        return {};
    if (*lines == 0)
        return 0;
    CascadeItem& c = item<CascadeItem>(app, at);
    if (c.open != 0)
        c.moreFlags |= 4;
    else
        c.moreFlags &= 0xfb;
    const Result count = lengthOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName);
    if (!count)
        return {};
    {
        CascadeItem& d = item<CascadeItem>(app, at);
        d.shown = int16_t(*count < wide(d.most) ? *count : wide(d.most));
    }
    demoGreys(app, at);
    if (lit == 0)
    {
        CascadeItem& d = item<CascadeItem>(app, at);
        if (d.unused44 != 0)
            return 0;
        d.open = 0;
        d.moreFlags &= 0xfb;
        cascadeClosedBox(app, at);
        demoGreys(app, at);
        u32 colour = (item<CascadeItem>(app, at).flags & 1) != 0 ? kGrey : kYellow;
        if (!call.core<0x449df0, loop::keysTakenOf, 0>(call_at, (at & 0xffff0000) | uint16_t(item<CascadeItem>(app, at).y)))
            return {};
        if (call.r != 0)
            colour = kDark;
        if (kSlidingIn(app) != 0)
        {
            if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, item<CascadeItem>(app, at).codelink, kDontHide, 0))
                return {};
            if (call.r == 0)
                return 0;
        }
        const u32 out = frame - 0x138;
        if (!cascadeLine(app, ts, call_at - 4, at, out, u32(*lines), word(app, chosenAt)))
            return {};
        CascadeItem& e = item<CascadeItem>(app, at);
        if (!call.named<text::menuTextDrawn, 2>(call_at, out, u32(wide(e.x) + 0x19), u32(wide(e.y)), kMenuFont, 0, colour))
            return {};
        return 0;
    }
    CascadeItem& d = item<CascadeItem>(app, at);
    if (d.open == 0)
    {
        const int16_t x = d.x, y = d.y;
        if ((d.flags & 1) == 0
            && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(d.shapes[0])), u32(wide(x)), u32(wide(y) + 0xc)))
            return {};
        CascadeItem& e = item<CascadeItem>(app, at);
        if (e.unused44 != 0)
            return 0;
        const u32 colour = (e.flags & 1) != 0 ? kDark : kYellow;
        if (kSlidingIn(app) != 0)
            return 0;
        const u32 out = frame - 0x238;
        if (!cascadeLine(app, ts, call_at - 4, at, out, u32(*lines), word(app, chosenAt)))
            return {};
        if (!call.named<text::menuTextDrawn, 2>(call_at, out, u32(wide(x) + 0x19), u32(wide(y)), kMenuFont, 0, colour))
            return {};
        return 0;
    }
    const ListBox box{0x52, 0x56, 0x58, 0x5a, 0x54};
    if (!followPointer(app, ts, call_at, at, box, *count, 0x55a424))
        return {};
    {
        CascadeItem& e = item<CascadeItem>(app, at);
        if (!call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(wide(e.boxLeft)), u32(wide(e.boxTop)), u32(wide(e.boxWidth)), u32(wide(e.boxHeight)), kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3]))
            return {};
    }
    int16_t top = item<CascadeItem>(app, at).boxTop;
    for (int32_t i = wide(item<CascadeItem>(app, at).first);; ++i, top = int16_t(top + 0x10))
    {
        const u32 lineAt = u32(*lines) + u32(i) * 2;
        if (half(app, lineAt) == 0 || wide(item<CascadeItem>(app, at).last) < i)
            break;
        if (!call.lineByName(call_at, u32(wide(half(app, lineAt))), u32(i) * 2))
            return {};
        const u32 text = call.r;
        u32 colour = kBlue;
        int32_t may = 1;
        if (*allowed != 0)
            may = wide(half(app, u32(*allowed) + u32(i) * 2));
        if (*colours != 0)
            colour = word(app, u32(i) * 4 + u32(*colours));
        if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<CascadeItem>(app, at).listName, kTracks))
            return {};
        const int32_t left = wide(item<CascadeItem>(app, at).boxLeft) + gap;
        if (call.r == 0 && gameSettings(app).raceType == 3
            && !trackMarks(app, ts, call_at, i, left, wide(top), at + 0x5c, at + 0x5e))
            return {};
        const u32 shade = wide(item<CascadeItem>(app, at).chosen) == i && may != 0 ? kYellow : may != 0 ? colour : kGrey;
        if (!call.named<text::menuTextDrawn, 2>(call_at, text, u32(wide(item<CascadeItem>(app, at).boxLeft) + gap), u32(wide(top)), kMenuFont, 0, shade))
            return {};
    }
    CascadeItem& e = item<CascadeItem>(app, at);
    if (!call.plain<0x49c1c0, frontend::menud::pointerListLine>(call_at, int32_t(u32(wide(e.boxTop))), 0x10))
        return {};
    const int32_t line = int32_t(call.r);
    CascadeItem& g = item<CascadeItem>(app, at);
    if (g.first != 0
        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(line < 0 ? g.shapes[4] : g.shapes[2])), u32(wide(g.boxLeft) + 1), u32(wide(g.boxTop) + 2)))
        return {};
    CascadeItem& h = item<CascadeItem>(app, at);
    if (wide(h.last) < *count - 1
        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(wide(h.shown) > line ? h.shapes[3] : h.shapes[5])), u32(wide(h.boxLeft) + 1), u32(wide(h.boxHeight) + wide(h.boxTop) - 0x10)))
        return {};
    CascadeItem& k = item<CascadeItem>(app, at);
    const int32_t x = wide(k.x), y = wide(k.y);
    if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(k.shapes[0])), u32(x), u32(y + 0xc))
        || !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<CascadeItem>(app, at).shapes[1])), u32(x + 0xc), u32(y - 1)))
        return {};
    if (!call.named<text::menuTextDrawn, 2>(call_at, item<CascadeItem>(app, at).text, u32(wide(item<CascadeItem>(app, at).boxLeft) + gap), u32(y), kMenuFont, 0, kYellow))
        return {};
    return 0;
}

/* 0x462320's: an open cascade's pass: 5 to scroll up (the pointer above its
 * lines, some above them), 6 to scroll down (below, some below), else 0. */
Result cascadePassed(App* app, ThreadState& ts, u32 esp, u32 at, u32 open, u32 first, u32 last, u32 shown,
                     u32 most, u32 list)
{
    if (half(app, at + open) == 0)
        return 0;
    const u32 call_at = esp - 0x14;
    Caller call{app, ts};
    if (!call.plain<0x49c1c0, frontend::menud::pointerListLine>(call_at, int32_t(u32(wide(item<Item>(app, at).boxTop))), 0x10))
        return {};
    const int32_t line = int32_t(call.r);
    const Result count = lengthOf(app, ts, call_at - 4, word(app, at + list));
    if (!count)
        return {};
    const int32_t m = wide(half(app, at + most));
    setHalf(app, at + shown, int16_t(*count < m ? *count : m));
    if (line < 0 && half(app, at + first) != 0)
        return 5;
    if (line < wide(half(app, at + shown)))
        return 0;
    if (wide(half(app, at + last)) < *count - 1)
        return 6;
    return 0;
}

/* 0x4623a0's: as cascade `at` is shown: placed by the race's table, its
 * lines at most 2000, closed, its first lines shown, its shapes found (yar1,
 * bdd2, the arrows, weel, badg), greyed where only the host may open it. */
Result cascadeShowing(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0xc;
    CascadeItem& c = item<CascadeItem>(app, at);
    c.x = int16_t(kPlaceX(app, wide(c.place)) + 0x7e);
    c.y = int16_t(kPlaceY(app, wide(c.place)) + 1);
    if (c.most == 0)
        c.most = 0x7d0;
    c.open = 0;
    c.chosen = 0;
    c.last = int16_t(c.most - 1);
    c.first = 0;
    Caller call{app, ts};
    static const u32 names[] = {0x53aa5c, 0x53aa64, 0x53aa6c, 0x53aa74, 0x53aa7c, 0x53aa84, 0x53aa8c, 0x53aa94};
    static const u32 fields[] = {0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x5c, 0x5e};
    for (int i = 0; i < 8; ++i)
    {
        if (!call.named<shapes::shapeNamed, 0>(call_at, names[i]))
            return {};
        setHalf(app, at + fields[i], int16_t(call.r));
    }
    const u32 codelink = item<CascadeItem>(app, at).codelink;
    if (codelink == 0 || kNetworkRace(app) == 0 || kNetworkHost(app) != 0)
        return int32_t(call.r);
    if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, codelink, kHostOnly, 0))
        return {};
    if (call.r != 0)
        item<CascadeItem>(app, at).flags |= 1;
    return int32_t(call.r);
}

/* The pointer put on an open list's line chosen: across the middle of its
 * box, down that line's. */
Result pointAtChosen(App* app, ThreadState& ts, u32 at, u32 item_, int32_t chosen, int32_t first)
{
    Item& it = item<Item>(app, item_);
    Caller call{app, ts};
    if (!call.plain<0x49bfc0, frontend::screen::setPointer>(
            at, u32(halved(wide(it.boxWidth)) + wide(it.boxLeft)), u32((chosen - first) * 16 + wide(it.boxTop) + 8)))
        return {};
    return int32_t(call.r);
}

/* 0x4624a0's: key `key` on cascade `at`.  Closed: Enter or right opens it,
 * its line chosen shown, the pointer on it, with the menus' click: 2.  Open:
 * up and down move the line chosen by one, page up and down by the lines
 * shown, the lines shown following, the pointer on it (0); Enter or left on
 * the box on a line that may be chosen chooses it and closes it, Escape or
 * Enter off it closes it, with the click: 2.  Else 0. */
Result cascadeKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    const u32 frame = esp - 0x10, call_at = frame - 0x10, chosenAt = frame - 0x10;
    Caller call{app, ts};
    const Result lines = linesOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName, chosenAt);
    if (!lines)
        return {};
    const Result allowed = allowedOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName);
    if (!allowed)
        return {};
    if (*lines == 0)
        return 0;
    const Result counted = lengthOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName);
    if (!counted)
        return {};
    const int16_t count = int16_t(*counted);
    CascadeItem& c = item<CascadeItem>(app, at);
    c.shown = count < c.most ? int16_t(*counted) : c.most;
    const uint16_t k = uint16_t(key);
    const auto click = [&]() { return call.effectByName(call_at, 1, 0x7f); };
    const auto point = [&]() {
        CascadeItem& p = item<CascadeItem>(app, at);
        return pointAtChosen(app, ts, call_at, at, wide(p.chosen), wide(p.first));
    };
    const auto clampChosen = [&](int16_t chosen) {
        CascadeItem& p = item<CascadeItem>(app, at);
        p.chosen = chosen;
        int32_t to = 0;
        if (chosen >= 0)
        {
            to = wide(count) - 1;
            if (wide(p.chosen) <= to)
                to = wide(p.chosen);
        }
        p.chosen = int16_t(to);
    };
    if (c.open == 0)
    {
        if (k != 0xd && k != 0x4d00)
            return 0;
        if (!click())
            return {};
        CascadeItem& p = item<CascadeItem>(app, at);
        p.open = 1;
        p.chosen = half(app, word(app, chosenAt));
        if (p.chosen >= p.shown)
        {
            p.last = p.chosen;
            p.first = int16_t(p.chosen - (p.shown - 1));
        }
        else
        {
            p.first = 0;
            p.last = int16_t(p.first + p.shown - 1);
        }
        if (!cascadeLaidOut(app, ts, call_at - 4, at) || !point())
            return {};
        return 2;
    }
    const auto choose = [&]() -> Result {
        if (!click())
            return {};
        item<CascadeItem>(app, at).open = 0;
        setWord(app, word(app, chosenAt), u32(wide(item<CascadeItem>(app, at).chosen)));
        if (!cascadeLaidOut(app, ts, call_at - 4, at))
            return {};
        return 2;
    };
    const auto close = [&]() -> Result {
        if (!click())
            return {};
        item<CascadeItem>(app, at).open = 0;
        if (!cascadeLaidOut(app, ts, call_at - 4, at))
            return {};
        return 2;
    };
    CascadeItem& p = item<CascadeItem>(app, at);
    switch (k)
    {
    case 0xd:
    case 0x4b00:
    {
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x1a))
            return {};
        if (call.r == 0)
            return close();
        if (*allowed != 0 && half(app, u32(wide(p.chosen)) * 2 + u32(*allowed)) == 0)
            return close();
        return choose();
    }
    case 0x1b:
        return close();
    case 0x4900:
        clampChosen(int16_t(p.chosen - p.shown));
        if (p.chosen < p.first)
        {
            const int16_t first = int16_t(p.first - p.shown);
            p.first = first;
            int32_t to;
            if (first < 0)
                to = 0;
            else
            {
                to = wide(count) - 1;
                if (wide(p.first) <= to)
                    to = wide(p.first);
            }
            p.first = int16_t(to);
            p.last = int16_t(to + int16_t(p.shown - 1));
        }
        break;
    case 0x5100:
        clampChosen(int16_t(p.chosen + p.shown));
        if (p.chosen > p.last)
        {
            const int16_t last = int16_t(p.last + p.shown);
            p.last = last;
            int32_t to = 0;
            if (last >= 0)
            {
                to = wide(count) - 1;
                if (wide(p.last) <= to)
                    to = wide(p.last);
            }
            p.last = int16_t(to);
            const int16_t first = int16_t(to - int16_t(p.shown - 1));
            p.first = first;
            int32_t from = 0;
            if (first >= 0)
            {
                from = wide(count) - 1;
                if (wide(p.first) <= from)
                    from = wide(p.first);
            }
            p.first = int16_t(from);
        }
        break;
    case 0x4800:
        clampChosen(int16_t(p.chosen - 1));
        if (p.chosen < p.first)
        {
            --p.first;
            --p.last;
        }
        break;
    case 0x5000:
        clampChosen(int16_t(p.chosen + 1));
        if (p.chosen > p.last)
        {
            ++p.first;
            ++p.last;
        }
        break;
    default:
        return 0;
    }
    if (!point())
        return {};
    return 0;
}

/* 0x4628d0's: cascade `at` laid out: open, its box as wide as its widest
 * line (0x28 at least) and its marks' room, its lines shown high, beside its
 * picture; closed, as a closed one.  No caller reads its eax. */
Result cascadeLaidOut(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 frame = esp - 0x18, call_at = frame - 8;
    Caller call{app, ts};
    const Result lines = linesOf(app, ts, call_at - 4, item<CascadeItem>(app, at).listName, frame - 8);
    if (!lines)
        return {};
    if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<CascadeItem>(app, at).listName, kTracks))
        return {};
    bool tracks = call.r == 0;
    if (!tracks)
    {
        if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<CascadeItem>(app, at).listName, kRecordTracks))
            return {};
        tracks = call.r == 0;
    }
    const int32_t gap = tracks ? 0x23 : 0xf;
    if (item<CascadeItem>(app, at).open == 0 || *lines == 0)
    {
        cascadeClosedBox(app, at);
        return int32_t(at);
    }
    if (!widestLine(app, ts, call_at, at, u32(*lines)))
        return {};
    CascadeItem& c = item<CascadeItem>(app, at);
    const int16_t width = int16_t(c.boxWidth + gap + 5);
    c.boxHeight = int16_t((c.shown << 4) + 5);
    c.boxLeft = int16_t(c.x + 0x1c);
    c.boxWidth = width;
    c.boxTop = int16_t(c.y + 0x10);
    return 0;
}

/* ---- The check box ---- */

/* 0x4629c0's: check box `at` drawn, lit under the pointer: its text (left
 * of the box where it has one), the box, a tick in it.  0. */
Result checkDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    const u32 call_at = esp - 0x14 - 0x14;
    const u32 tick = lit != 0 ? 0xffffffff : 0xffffff50;
    const u32 colour = lit != 0 ? 0xffffff : kYellow;
    CheckItem& c = item<CheckItem>(app, at);
    const int32_t x = wide(c.x), y = wide(c.y);
    int32_t across = 0;
    Caller call{app, ts};
    if (c.labelled != 0)
    {
        if (!call.named<text::menuTextDrawn, 2>(call_at, c.text, u32(x), u32(y), kMenuFont, 0, colour)
            || !call.named<text::menuTextWidth, 0>(call_at, item<CheckItem>(app, at).text, kMenuFont))
            return {};
        across = int32_t(call.r) + 0xa;
        if (!call.core<0x4d89f0, draw2d::panel, 0>(call_at, u32(x + across), u32(y), 0x13, kMenuFont))
            return {};
    }
    else if (!call.named<text::menuTextDrawn, 2>(call_at, c.text, u32(x + 0x16), u32(y), kMenuFont, 0, colour)
             || !call.core<0x4d89f0, draw2d::panel, 0>(call_at, u32(x + 2), u32(y), 0x13, kMenuFont))
        return {};
    if (item<CheckItem>(app, at).ticked != 0)
    {
        const u32 colours[] = {tick, tick, tick, tick};
        if (!colouredRectangle(app, ts, call_at - 0x10 - 4,
                               {u32(x + 7 + across), u32(y + 7), u32(x + 0xe + across), u32(y + 0xe)}, colours))
            return {};
    }
    return 0;
}

/* 0x462ad0's: Enter on check box `at` ticks it or not, with the click: 2. */
Result checkKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    if (uint16_t(key) != 0xd)
        return 0;
    Caller call{app, ts};
    if (!call.effectByName(esp - 0xc, 1, 0x7f))
        return {};
    CheckItem& c = item<CheckItem>(app, at);
    c.ticked = c.ticked == 0 ? 1 : 0;
    return 2;
}

/* 0x462b30's: check box `at` laid out: its box at the box's place, its text
 * and the box wide, 0x1d high.  The width. */
Result checkLaidOut(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0x10;
    Caller call{app, ts};
    int32_t across = 0;
    if (item<CheckItem>(app, at).labelled != 0)
    {
        if (!call.named<text::menuTextWidth, 0>(call_at, item<CheckItem>(app, at).text, kMenuFont))
            return {};
        across = int32_t(call.r) + 0xa;
    }
    CheckItem& c = item<CheckItem>(app, at);
    c.boxLeft = int16_t(c.x + int16_t(across));
    c.boxTop = c.y;
    c.boxHeight = 0x1d;
    if (!call.named<text::menuTextWidth, 0>(call_at, c.text, kMenuFont))
        return {};
    const u32 width = call.r + 0x13;
    item<CheckItem>(app, at).boxWidth = int16_t(width);
    return int32_t(width);
}

/* ---- The box overlay ---- */

/* 0x462b90's: box overlay `at` drawn: a field 200 square at its place, its
 * text centred over its top.  0. */
Result overlayDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    const u32 call_at = esp - 0x14 - 8;
    const u32 colour = lit != 0 ? kYellow : 0xff649dfd;
    Item& o = item<Item>(app, at);
    const int32_t x = wide(o.x), y = wide(o.y);
    Caller call{app, ts};
    if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont)
        || !call.named<text::menuTextWidth, 0>(call_at, item<Item>(app, at).text, kMenuFont)
        || !call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(x), u32(y), 0xc8, 0xc8, kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3])
        || !call.named<text::menuTextDrawn, 2>(call_at, item<Item>(app, at).text, u32(x + 0x64), u32(y + 2), kMenuFont, 5, colour))
        return {};
    return 0;
}

/* ---- The select list ---- */

constexpr Var<int32_t> kSelectHeld{0x55a428}, kSelectHeldLine{0x55a42c}, kJoining{0x55a430};

}  // namespace

/* 0x4633d0's: select list `at` laid out: its lines shown (as many as it has,
 * at most its own), the line chosen kept among them, its box 200 wide, its
 * arrows' boxes (none where there is nothing to scroll to). */
void selectLaidOut(App* app, u32 at)
{
    SelectItem& s = item<SelectItem>(app, at);
    int32_t shown = wide(s.lines);
    if (shown > int32_t(s.count))
        shown = int32_t(s.count);
    s.last = int16_t(s.first + int16_t(shown) - 1);
    s.boxLeft = s.x;
    s.boxTop = s.y;
    s.boxHeight = int16_t((s.lines << 4) + 0x39);
    s.shown = int16_t(shown);
    s.boxWidth = 0xc8;
    if (wide(s.first) > int32_t(s.chosen))
        s.chosen = u32(wide(s.first));
    if (wide(s.last) < int32_t(s.chosen))
        s.chosen = u32(wide(s.last));
    s.upBox[1] = int16_t(s.boxLeft + s.boxWidth - 0x10);
    s.upBox[0] = int16_t(s.boxTop + 2);
    const int16_t up = s.first != 0 ? 0x10 : 0;
    s.upBox[2] = up;
    s.upBox[3] = up;
    s.downBox[1] = int16_t(s.boxLeft + s.boxWidth - 0x10);
    s.downBox[0] = int16_t(s.boxTop + s.boxHeight - 0x12);
    const int16_t down = wide(s.last) < int32_t(s.count) - 1 ? 0x10 : 0;
    s.downBox[2] = down;
    s.downBox[3] = down;
}

namespace
{

/* 0x462c30's: as select list `at` is shown: no lines, none chosen, 10 shown
 * where it says none, its arrows' shapes, the replays listed where it is the
 * replays' (rp3); the select list of the screen (0x5556c8). */
Result selectShowing(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0xc;
    SelectItem& s = item<SelectItem>(app, at);
    setWord(app, at + 0x44, 0);
    s.chosen = 0;
    if (s.lines == 0)
        s.lines = 0xa;
    s.first = 0;
    s.last = int16_t(s.lines - 1);
    s.count = 0;
    Caller call{app, ts};
    static const u32 fields[] = {0x58, 0x5a, 0x5c, 0x5e};
    for (int i = 0; i < 4; ++i)
    {
        if (!call.named<shapes::shapeNamed, 0>(call_at, kArrowNames[i]))
            return {};
        setHalf(app, at + fields[i], int16_t(call.r));
    }
    item<SelectItem>(app, at).texts = 0;
    const u32 codelink = item<SelectItem>(app, at).codelink;
    if (codelink != 0)
    {
        if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, codelink, kReplays, 0))
            return {};
        if (call.r != 0 && !call.plain<0x4475a0, load::replaysListed>(call_at, at, kReplayFile))
            return {};
    }
    setWord(app, 0x5556c8, at);
    return int32_t(call.r);
}

/* 0x462ce0's: as select list `at` is hidden, its lines' texts freed. */
Result selectHiding(App* app, ThreadState& ts, u32 esp, u32 at)
{
    u32 eax = at;
    SelectItem& s = item<SelectItem>(app, at);
    if (s.texts != 0)
    {
        Caller call{app, ts};
        if (!call.plain<0x4e1890, watcom::clib::free>(esp - 0x10, s.texts))
            return {};
        eax = call.r;
        item<SelectItem>(app, at).texts = 0;
    }
    item<SelectItem>(app, at).count = 0;
    setWord(app, at + 0x44, 0);
    return int32_t(eax);
}

/* 0x462d20's: select list `at` drawn (not under a dialog or while the
 * screens slide in): laid out, its field, its title's art and title, its
 * lines shown -- the chosen yellow -- or its empty line's text down its
 * rest, its arrows lit under the pointer.  0. */
Result selectDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    if (demoHides(app))
        return 0;
    const u32 frame = esp - 0x14, call_at = frame - 0xc;
    Caller call{app, ts};
    if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, at))
        return {};
    if ((call.r & 0xff) != 0 || kSlidingIn(app) != 0)
        return 0;
    const u32 colour = lit != 0 ? kYellow : 0xff649dfd;
    selectLaidOut(app, at);
    SelectItem& s = item<SelectItem>(app, at);
    int32_t height = wide(s.lines) << 4;
    if (height <= wide(s.boxHeight))
        height = wide(s.boxHeight);
    const int32_t left = wide(s.boxLeft), top = wide(s.boxTop);
    if (!call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(left), u32(top), u32(wide(s.boxWidth)), u32(height), kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3])
        || !call.named<shapes::drawArt, 0>(call_at, word(app, 0x66108c), u32(left - 0xe), u32(top - 9), 0xffffffff))
        return {};
    SelectItem& t = item<SelectItem>(app, at);
    if (t.text != 0
        && !call.named<text::menuTextDrawn, 2>(call_at, t.text, u32(wide(t.x) + 0x1a + halved(wide(t.boxWidth))), u32(wide(t.y) + 0x13), kMenuFont, 2, colour))
        return {};
    int32_t y = top + 0x39;
    if (item<SelectItem>(app, at).count != 0)
    {
        for (int32_t i = wide(item<SelectItem>(app, at).first);; ++i, y += 0x10)
        {
            SelectItem& u = item<SelectItem>(app, at);
            if (i >= int32_t(u.count) || wide(u.last) < i)
                break;
            const u32 shade = u32(i) == u.chosen ? kYellow : 0xff649dfd;
            if (!call.named<text::menuTextDrawn, 2>(call_at, word(app, u.texts + u32(i) * 8), u32(wide(item<SelectItem>(app, at).boxLeft) + 0xa), u32(y), kMenuFont, 0, shade))
                return {};
        }
        if (item<SelectItem>(app, at).empty != 0)
            while (y < wide(item<SelectItem>(app, at).boxTop) + height)
            {
                if (!call.named<text::menuTextDrawn, 2>(call_at, item<SelectItem>(app, at).empty, u32(wide(item<SelectItem>(app, at).boxLeft) + 0xa), u32(y), kMenuFont, 0, kDark))
                    return {};
                y += 0x10;
            }
    }
    else if (item<SelectItem>(app, at).empty != 0)
    {
        for (int32_t i = 0; wide(item<SelectItem>(app, at).lines) > i; ++i, y += 0x10)
            if (!call.named<text::menuTextDrawn, 2>(call_at, item<SelectItem>(app, at).empty, u32(wide(item<SelectItem>(app, at).boxLeft) + 0xa), u32(y), kMenuFont, 0, kDark))
                return {};
    }
    SelectItem& v = item<SelectItem>(app, at);
    if (v.first != 0)
    {
        const int32_t bx = wide(v.upBox[1]), by = wide(v.upBox[0]);
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x60))
            return {};
        SelectItem& w = item<SelectItem>(app, at);
        if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(call.r != 0 ? w.shapes[2] : w.shapes[0])), u32(bx), u32(by)))
            return {};
    }
    SelectItem& w = item<SelectItem>(app, at);
    if (wide(w.last) < int32_t(w.count) - 1)
    {
        const int32_t bx = wide(w.downBox[1]), by = wide(w.downBox[0]);
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x68))
            return {};
        SelectItem& z = item<SelectItem>(app, at);
        if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(call.r != 0 ? z.shapes[3] : z.shapes[1])), u32(bx), u32(by)))
            return {};
    }
    return 0;
}

/* 0x462fa0's: key `key` on select list `at` (with lines): Enter on its
 * arrows scrolls it a line (2); Enter on a line chooses it, and the mouse's
 * second click on the same line within 0x7d ticks joins the network game
 * chosen ("joingame" on the screen, 0x43fdd0) -- 2, or the join's answer
 * with the screen's item put on "joingame"; up, down, page up and down move
 * the line chosen, the pointer on it (2).  The list's function told of the
 * line chosen but after a join.  Else 0. */
Result selectKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    const u32 frame = esp - 0x10, call_at = frame - 0x10;
    SelectItem& s = item<SelectItem>(app, at);
    const int32_t count = int32_t(s.count);
    if (s.texts == 0 || count == 0)
        return 0;
    selectLaidOut(app, at);
    Caller call{app, ts};
    int32_t result = 2;
    bool joinTried = false;
    const uint16_t k = uint16_t(key);
    const auto clampChosen = [&](int32_t chosen) {
        SelectItem& t = item<SelectItem>(app, at);
        int32_t to = 0;
        if (chosen >= 0)
        {
            to = int32_t(t.count) - 1;
            if (!(to < chosen))
                to = chosen;
        }
        t.chosen = u32(to);
    };
    const auto point = [&]() {
        SelectItem& t = item<SelectItem>(app, at);
        return pointAtChosen(app, ts, call_at, at, int32_t(t.chosen), wide(t.first));
    };
    if (k == 0xd)
    {
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x60))
            return {};
        if (call.r != 0)
        {
            kSelectHeld.set(app, 0);
            --item<SelectItem>(app, at).first;
            selectLaidOut(app, at);
            return 2;
        }
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x68))
            return {};
        if (call.r != 0)
        {
            kSelectHeld.set(app, 0);
            ++item<SelectItem>(app, at).first;
            selectLaidOut(app, at);
            return 2;
        }
    }
    switch (k)
    {
    case 0xd:
    {
        SelectItem& t = item<SelectItem>(app, at);
        if (!call.plain<0x49c1c0, frontend::menud::pointerListLine>(call_at, int32_t(u32(wide(t.boxTop) + 0x39)), 0x10))
            return {};
        const int32_t line = int32_t(call.r);
        SelectItem& u = item<SelectItem>(app, at);
        if (line >= 0 && line < wide(u.shown) && line < int32_t(u.count)
            && wide(u.first) + line != int32_t(u.chosen))
        {
            const int32_t chosen = line + wide(u.first);
            u.chosen = u32(chosen);
            int32_t to = 0;
            if (chosen >= 0)
            {
                to = chosen;
                if (int32_t(u.count) - 1 < chosen)
                    to = int32_t(u.count) - 1;
            }
            setWord(app, at + 0x44, u32(to));
            u.chosen = u32(to);
        }
        if (word(app, 0x664658) != 0)
        {
            const int32_t clock = kClock(app);
            if (kSelectHeldLine(app) != int32_t(item<SelectItem>(app, at).chosen))
                kSelectHeld.set(app, 0);
            kSelectHeldLine.set(app, int32_t(item<SelectItem>(app, at).chosen));
            const int32_t held = kSelectHeld(app);
            if (held == 0 || held + 0x7d < clock)
                kSelectHeld.set(app, clock);
            else
            {
                kSelectHeld.set(app, 0);
                joinTried = true;
                if (!call.plain<0x44aeb0, loop::currentMenuOf>(call_at))
                    return {};
                const u32 screen = call.r;
                if (!call.plain<kMenuFindItem, frontend::menus::findItem>(call_at, screen, kJoinGameName))
                    return {};
                const u32 found = call.r;
                if (found != 0)
                {
                    kJoining.set(app, kJoining(app) + 1);
                    if (kJoining(app) < 2)
                    {
                        const Result joinedIt = joined(app, ts, call_at - 4, {0, kJoinGameName, at, at});
                        if (!joinedIt)
                            return {};
                        result = *joinedIt;
                    }
                    kJoining.set(app, kJoining(app) - 1);
                    if (result != 0)
                    {
                        int32_t n = wide(half(app, screen + 0x18));
                        for (;; ++n)
                        {
                            const u32 entry = u32(n) * 0xbc;
                            if (word(app, entry + 0x604ee0) == 0)
                            {
                                result = 0;
                                break;
                            }
                            if (entry + 0x604ee0 == found)
                            {
                                setHalf(app, screen + 0x1a, int16_t(n - half(app, screen + 0x18)));
                                return result;
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    case 0x4900:
    {
        SelectItem& t = item<SelectItem>(app, at);
        clampChosen(int32_t(t.chosen) - wide(t.shown));
        SelectItem& u = item<SelectItem>(app, at);
        if (wide(u.first) > int32_t(u.chosen))
        {
            const int16_t first = int16_t(u.first - u.shown);
            u.first = first;
            int32_t to = 0;
            if (first >= 0)
            {
                to = count - 1;
                if (wide(u.first) <= to)
                    to = wide(u.first);
            }
            u.first = int16_t(to);
            u.last = int16_t(to + int16_t(u.shown - 1));
        }
        if (!point())
            return {};
        break;
    }
    case 0x5100:
    {
        SelectItem& t = item<SelectItem>(app, at);
        clampChosen(int32_t(t.chosen) + wide(t.shown));
        SelectItem& u = item<SelectItem>(app, at);
        if (wide(u.last) < int32_t(u.chosen))
        {
            const int16_t last = int16_t(u.last + u.shown);
            u.last = last;
            int32_t to = 0;
            if (last >= 0)
            {
                to = count - 1;
                if (wide(u.last) <= to)
                    to = wide(u.last);
            }
            u.last = int16_t(to);
            const int16_t first = int16_t(to - int16_t(u.shown - 1));
            u.first = first;
            int32_t from = 0;
            if (first >= 0)
            {
                from = wide(u.first);
                if (from > count - 1)
                    from = count - 1;
            }
            u.first = int16_t(from);
        }
        if (!point())
            return {};
        break;
    }
    case 0x4800:
    {
        clampChosen(int32_t(item<SelectItem>(app, at).chosen) - 1);
        SelectItem& u = item<SelectItem>(app, at);
        if (wide(u.first) > int32_t(u.chosen))
        {
            --u.first;
            --u.last;
        }
        if (!point())
            return {};
        break;
    }
    case 0x5000:
    {
        clampChosen(int32_t(item<SelectItem>(app, at).chosen) + 1);
        SelectItem& u = item<SelectItem>(app, at);
        if (wide(u.last) < int32_t(u.chosen))
        {
            ++u.first;
            ++u.last;
        }
        if (!point())
            return {};
        break;
    }
    default:
        return 0;
    }
    if (joinTried)
        return result;
    const u32 told = item<SelectItem>(app, at).told;
    if (told != 0 && !call(call_at, told, {at, item<SelectItem>(app, at).chosen, at, at}))
        return {};
    return result;
}

/* ---- The drop list ---- */

Result dropLaidOut(App* app, ThreadState& ts, u32 esp, u32 at);

/* 0x4634d0's: drop list `at` drawn, lit under the pointer (`lit`).  Open:
 * its lines followed by the pointer, its box, its lines from the first
 * shown (may be chosen or greyed, in their colours, the chosen yellow, the
 * tracks' marks), its arrows.  Closed: its line chosen its text.  Then its
 * art (wide and right or left, the track's map's, plain) pulsing when lit,
 * a car's class or a track's kind beside it, a long line clipped, its text
 * where its art says.  0. */
Result dropDrawn(App* app, ThreadState& ts, u32 esp, u32 at, u32 lit)
{
    const u32 frame = esp - 0x14, call_at = frame - 0x60, chosenAt = frame - 0x60;
    const int32_t gap = gameSettings(app).raceType == 3 ? 0x23 : 0xf;
    if (demoHides(app))
        return 0;
    const Result allowed = allowedOf(app, ts, call_at - 4, item<DropItem>(app, at).listName);
    if (!allowed)
        return {};
    const Result colours = coloursOf(app, ts, call_at - 4, item<DropItem>(app, at).listName);
    if (!colours)
        return {};
    const Result lines = linesOf(app, ts, call_at - 4, item<DropItem>(app, at).listName, chosenAt);
    if (!lines)
        return {};
    if (*lines == 0)
        return 0;
    DropItem& d = item<DropItem>(app, at);
    if (d.open != 0)
        d.moreFlags |= 4;
    else
        d.moreFlags &= 0xfb;
    // Where its text goes by its art: across from x, down from y.
    int32_t textX, textY, artY;
    bool map = false;
    int16_t artX;
    if (d.wide != 0)
    {
        artY = 0xc;
        textY = 0xe;
        if (d.right != 0)
            textX = 0x62, artX = 0xbd;
        else
            artX = 0x10, textX = 0x75;
    }
    else if (d.x == 0x65 && d.y == 0x45)
        artX = 0x11, artY = 0x13, map = true, textX = 0x6d, textY = 0;
    else
        artY = 6, textX = 0x73, artX = 8, textY = 0;
    const Result counted = lengthOf(app, ts, call_at - 4, item<DropItem>(app, at).listName);
    if (!counted)
        return {};
    const int32_t count = *counted;
    Caller call{app, ts};
    {
        DropItem& e = item<DropItem>(app, at);
        e.shown = int16_t(count < wide(e.most) ? count : wide(e.most));
    }
    if (item<DropItem>(app, at).open != 0)
    {
        const ListBox box{0x56, 0x5a, 0x5c, 0x5e, 0x58};
        if (!followPointer(app, ts, call_at, at, box, count, 0x55a434))
            return {};
        DropItem& e = item<DropItem>(app, at);
        if (!call.boxedNamed<draw2d::drawFieldBoxOf>(call_at, u32(wide(e.boxLeft)), u32(wide(e.boxTop)), u32(wide(e.boxWidth)), u32(wide(e.boxHeight)), kFieldColours[0], kFieldColours[1], kFieldColours[2], kFieldColours[3]))
            return {};
        int16_t top = item<DropItem>(app, at).boxTop;
        for (int32_t i = wide(item<DropItem>(app, at).first);; ++i, top = int16_t(top + 0x10))
        {
            const u32 lineAt = u32(*lines) + u32(i) * 2;
            if (half(app, lineAt) == 0 || wide(item<DropItem>(app, at).last) < i)
                break;
            if (!call.lineByName(call_at, u32(wide(half(app, lineAt))), u32(i) * 2))
                return {};
            const u32 text = call.r;
            u32 colour = kBlue;
            int32_t may = 1;
            if (*allowed != 0)
                may = wide(half(app, u32(*allowed) + u32(i) * 2));
            if (*colours != 0)
                colour = word(app, u32(i) * 4 + u32(*colours));
            if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<DropItem>(app, at).listName, kTracks))
                return {};
            bool tracks = call.r == 0;
            if (!tracks)
            {
                if (!call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<DropItem>(app, at).listName, kRecordTracks))
                    return {};
                tracks = call.r == 0;
            }
            const int32_t left = wide(item<DropItem>(app, at).boxLeft) + gap;
            if (tracks && gameSettings(app).raceType == 3 && !trackMarks(app, ts, call_at, i, left, wide(top), at + 0x60, at + 0x62))
                return {};
            const u32 shade = wide(item<DropItem>(app, at).chosen) == i && may != 0 ? kYellow : may != 0 ? colour : kGrey;
            if (!call.named<text::menuTextDrawn, 2>(call_at, text, u32(wide(item<DropItem>(app, at).boxLeft) + gap), u32(wide(top)), kMenuFont, 0, shade))
                return {};
        }
        DropItem& f = item<DropItem>(app, at);
        if (!call.plain<0x49c1c0, frontend::menud::pointerListLine>(call_at, int32_t(u32(wide(f.boxTop))), 0x10))
            return {};
        const int32_t line = int32_t(call.r);
        DropItem& g = item<DropItem>(app, at);
        if (g.first != 0
            && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(line < 0 ? g.shapes[6] : g.shapes[4])), u32(wide(g.boxLeft)), u32(wide(g.boxTop) + 2)))
            return {};
        DropItem& h = item<DropItem>(app, at);
        if (wide(h.last) < count - 1
            && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(wide(h.shown) > line ? h.shapes[5] : h.shapes[7])), u32(wide(h.boxLeft)), u32(wide(h.boxHeight) + wide(h.boxTop) - 0x10)))
            return {};
    }
    else
    {
        const u32 chosen = word(app, word(app, chosenAt));
        if (!call.lineByName(call_at, u32(wide(half(app, chosen * 2 + u32(*lines)))), 0))
            return {};
        item<DropItem>(app, at).text = call.r;
        item<DropItem>(app, at).chosen = half(app, word(app, chosenAt));
    }
    const int16_t x = item<DropItem>(app, at).x;
    const int16_t y = item<DropItem>(app, at).y;
    if (lit != 0)
    {
        const u32 pulse = pulseOf(app, 0xff);
        pulsed(app);
        DropItem& e = item<DropItem>(app, at);
        const auto drawPulse = [&](int32_t px, int32_t py) {
            DropItem& p = item<DropItem>(app, at);
            if (p.shapes[2] == 0)
                return true;
            if (!call.plain<0x4438b0, dialogs::dialogShownWith>(call_at, 0))
                return false;
            if ((call.r & 0xff) != 0 || (kRenderFlags(app) & 2) != 0)
                return true;
            return call.named<shapes::drawArt, 0>(call_at, u32(wide(item<DropItem>(app, at).shapes[2])), u32(px), u32(py), pulse);
        };
        if (e.wide != 0)
        {
            if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(e.shapes[0])), u32(wide(x)), u32(wide(y))))
                return {};
            if (item<DropItem>(app, at).right != 0)
            {
                if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).shapes[1])), u32(wide(x) + 5), u32(wide(y) + 0xe))
                    || !drawPulse(wide(x) - 0xa, wide(y)))
                    return {};
            }
            else if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).shapes[1])), u32(wide(x) + 0xf), u32(wide(y) + 0xe))
                     || !drawPulse(wide(x) + 0x80, wide(y)))
                return {};
        }
        else
        {
            if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(e.shapes[1])), u32(wide(x)), u32(wide(y))))
                return {};
            if (map ? !drawPulse(wide(x) + 0xad, wide(y) - 0xf) : !drawPulse(wide(x) + 0x64, wide(y)))
                return {};
        }
        if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).shapes[3])), u32(wide(x) + wide(artX)), u32(wide(y) + artY)))
            return {};
    }
    else if (!call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).shapes[0])), u32(wide(x)), u32(wide(y))))
        return {};
    if (!call.keepNamed<text::selectFontOf>(call_at, kMenuFont, kMenuFont))
        return {};
    const auto stricmp = [&](u32 name) {
        return call.plain<0x4e9ec0, watcom::clib::stricmp>(call_at, item<DropItem>(app, at).listName, name);
    };
    int32_t across = textX;
    if (*lines != 0 && map)
    {
        const u32 chosen = word(app, chosenAt);
        if (!stricmp(kCars))
            return {};
        bool cars = call.r == 0;
        bool opponents = false;
        if (!cars)
        {
            if (!stricmp(kOpponentCars))
                return {};
            opponents = call.r == 0 && int32_t(word(app, chosen)) > 3;
        }
        if (cars || opponents)
        {
            u32 car = word(app, chosen);
            if (!stricmp(kCars))
                return {};
            if (call.r != 0)
                car -= 4;
            if (!call.plain<0x439730, load::classOfCar>(call_at, car))
                return {};
            if (!call.lineByName(call_at, call.r + 0xbe, 0))
                return {};
            kNudge.set(app, 4);
            if (!call.named<text::menuTextDrawn, 2>(call_at, call.r, u32(wide(x) + textX + 0x83), u32(textY + wide(y) - 1), kMenuFont, 1, kBlue))
                return {};
            kNudge.set(app, 0);
        }
        else
        {
            if (!stricmp(kTracks))
                return {};
            bool tracks = call.r == 0;
            if (!tracks)
            {
                if (!stricmp(kRecordTracks))
                    return {};
                tracks = call.r == 0;
            }
            if (tracks)
            {
                const int32_t n = int32_t(word(app, chosen));
                const u32 kind = n > 7 ? 9 : n > 3 ? 8 : 7;
                kNudge.set(app, 4);
                const int32_t tx = wide(x) + textX + 0x83, ty = textY + wide(y) - 1;
                if (!call.lineByName(call_at, kind, u32(tx))
                    || !call.named<text::menuTextDrawn, 2>(call_at, call.r, u32(tx), u32(ty), kMenuFont, 1, kBlue))
                    return {};
                kNudge.set(app, 0);
                if (gameSettings(app).raceType == 3)
                {
                    const int32_t bit = 1 << (word(app, chosen) & 31);
                    if ((wide(gameSettings(app).duelsLost) & bit) != 0
                        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).marks[0])), u32(wide(x) + 0x22), u32(wide(y) + textY + 1)))
                        return {};
                    if ((wide(gameSettings(app).duelsWon) & bit) != 0
                        && !call.core<0x4d7990, shapes::drawShapeOf, 0>(call_at, u32(wide(item<DropItem>(app, at).marks[1])), u32(wide(x) + 0x32), u32(wide(y) + textY + 1)))
                        return {};
                }
            }
            else
            {
                across = textX + 0x19;
                if (!call.named<text::menuTextWidth, 0>(call_at, item<DropItem>(app, at).text, kMenuFont))
                    return {};
                if (int32_t(call.r) > 0xc8)
                {
                    const int32_t left = across + wide(x);
                    if (!call.plain<kSetClip, graphics::clipSet>(call_at, 0, 0, left + 0x64, 0x1e0)
                        || !call.named<text::menuTextDrawn, 2>(call_at, item<DropItem>(app, at).text, u32(left - 0x64), u32(textY + wide(y) - 1), kMenuFont, 0, kYellow)
                        || !call.plain<kSetClip, graphics::clipSet>(call_at, 0, 0, 0x280, 0x1e0))
                        return {};
                    return 0;
                }
            }
        }
    }
    const u32 colour = (item<DropItem>(app, at).flags & 1) != 0 ? kGrey : kYellow;
    if (!call.named<text::menuTextDrawn, 2>(call_at, item<DropItem>(app, at).text, u32(wide(x) + across), u32(textY + wide(y) - 1), kMenuFont, 5, colour))
        return {};
    return 0;
}

/* 0x463e60's: as drop list `at` is shown: its lines at most 2000, closed,
 * none chosen, its first lines shown, its art's shapes by its kind (wide
 * and right or left, the track's map's, plain), its arrows', the pulse's
 * (not over a race nor with alpha), greyed where only the host may open
 * it. */
Result dropShowing(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 call_at = esp - 0x10;
    DropItem& d = item<DropItem>(app, at);
    if (d.most == 0)
        d.most = 0x7d0;
    d.chosen = 0;
    d.first = 0;
    d.shapes[2] = 0;
    d.open = 0;
    d.last = int16_t(d.most - 1);
    Caller call{app, ts};
    const auto shape = [&](u32 name, u32 field) {
        if (!call.named<shapes::shapeNamed, 0>(call_at, name))
            return false;
        setHalf(app, at + field, int16_t(call.r));
        return true;
    };
    if (kOverRace(app) == 0 && (kRenderFlags(app) & 2) == 0
        && !shape(item<DropItem>(app, at).right != 0 ? 0x53a578 : 0x53a614, 0x46))
        return {};
    DropItem& e = item<DropItem>(app, at);
    u32 last;
    if (e.wide != 0)
    {
        const bool right = e.right != 0;
        if (!shape(right ? 0x53aabc : 0x53aacc, 0x42) || !shape(right ? 0x53aac4 : 0x53aad4, 0x44))
            return {};
        last = 0x53aadc;
    }
    else if (e.x == 0x65 && e.y == 0x45)
    {
        if (!shape(0x53aae4, 0x42))
            return {};
        setHalf(app, at + 0x44, int16_t(call.r));
        last = 0x53aaec;
    }
    else
    {
        if (!shape(0x53aaf4, 0x42) || !shape(0x53aafc, 0x44))
            return {};
        last = 0x53ab04;
    }
    if (!shape(last, 0x48) || !shape(0x53aa6c, 0x4a) || !shape(0x53aa74, 0x4c) || !shape(0x53aa7c, 0x4e)
        || !shape(0x53aa84, 0x50) || !shape(0x53aa8c, 0x60) || !shape(0x53aa94, 0x62))
        return {};
    const u32 codelink = item<DropItem>(app, at).codelink;
    if (codelink == 0 || kNetworkRace(app) == 0 || kNetworkHost(app) != 0)
        return int32_t(call.r);
    if (!call.core<0x4429c0, menus::codelinkMatched, 0>(call_at, codelink, kHostOnly, codelink))
        return {};
    if (call.r != 0)
        item<DropItem>(app, at).flags |= 1;
    return int32_t(call.r);
}

/* 0x463ff0's: key `key` on drop list `at` (its list found; else 0).  Closed:
 * left and right step its line chosen back and on, round, to one that may be
 * chosen (as many tries as it has lines); Enter opens it -- the line chosen
 * shown, the box laid out, the pointer on it -- with the menus' click: 2.
 * Open: up and down move the line by one, page up and down by the lines
 * shown, the lines shown following, the pointer on it; Enter on the box on a
 * line that may be chosen chooses it and closes it, Escape or Enter off it
 * closes it, with the click: 2.  Else 0. */
Result listKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key)
{
    const u32 frame = esp - 0x10, call_at = frame - 0x10, chosenAt = frame - 0x10;
    const Result lines = linesOf(app, ts, call_at - 4, item<DropItem>(app, at).listName, chosenAt);
    if (!lines)
        return {};
    const Result allowed = allowedOf(app, ts, call_at - 4, item<DropItem>(app, at).listName);
    if (!allowed)
        return {};
    if (*lines == 0)
        return 0;
    const Result counted = lengthOf(app, ts, call_at - 4, item<DropItem>(app, at).listName);
    if (!counted)
        return {};
    const int32_t count = *counted;
    const int16_t count16 = int16_t(count);
    Caller call{app, ts};
    {
        DropItem& d = item<DropItem>(app, at);
        d.shown = count16 < d.most ? count16 : d.most;
    }
    const uint16_t k = uint16_t(key);
    const auto click = [&]() { return call.effectByName(call_at, 1, 0x7f); };
    const auto point = [&]() {
        DropItem& p = item<DropItem>(app, at);
        return pointAtChosen(app, ts, call_at, at, wide(p.chosen), wide(p.first));
    };
    const auto clampChosen = [&](int16_t chosen) {
        DropItem& p = item<DropItem>(app, at);
        p.chosen = chosen;
        int32_t to = 0;
        if (chosen >= 0)
        {
            to = wide(count16) - 1;
            if (wide(p.chosen) <= to)
                to = wide(p.chosen);
        }
        p.chosen = int16_t(to);
    };
    const u32 chosen = word(app, chosenAt);
    if (item<DropItem>(app, at).open == 0)
    {
        DropItem& p = item<DropItem>(app, at);
        if (k == 0x4b00 || k == 0x4d00)
        {
            const bool back = k == 0x4b00;
            p.chosen = half(app, chosen);
            p.chosen = int16_t(p.chosen + (back ? -1 : 1));
            if (back ? p.chosen < 0 : wide(p.chosen) >= wide(count16))
                p.chosen = back ? int16_t(count - 1) : 0;
            if (*allowed != 0)
                for (int16_t tries = int16_t(count);;)
                {
                    if (half(app, u32(wide(p.chosen)) * 2 + u32(*allowed)) != 0)
                        break;
                    if (--tries == -1)
                        break;
                    p.chosen = int16_t(p.chosen + (back ? -1 : 1));
                    if (back ? p.chosen < 0 : wide(p.chosen) >= wide(count16))
                        p.chosen = back ? int16_t(count - 1) : 0;
                }
            setWord(app, chosen, u32(wide(item<DropItem>(app, at).chosen)));
            return 2;
        }
        if (k != 0xd)
            return 0;
        if (!click())
            return {};
        DropItem& q = item<DropItem>(app, at);
        q.open = 1;
        q.chosen = half(app, chosen);
        if (q.chosen >= q.shown)
        {
            q.last = q.chosen;
            q.first = int16_t(q.chosen - (q.shown - 1));
        }
        else
        {
            q.first = 0;
            q.last = int16_t(q.first + q.shown - 1);
        }
        if (!dropLaidOut(app, ts, call_at - 4, at) || !point())
            return {};
        return 2;
    }
    const auto close = [&](bool choose) -> Result {
        if (!click())
            return {};
        item<DropItem>(app, at).open = 0;
        if (choose)
            setWord(app, chosen, u32(wide(item<DropItem>(app, at).chosen)));
        if (!dropLaidOut(app, ts, call_at - 4, at))
            return {};
        return 2;
    };
    DropItem& p = item<DropItem>(app, at);
    switch (k)
    {
    case 0xd:
    {
        if (!call.plain<0x49c1f0, frontend::glue::pointerInBox>(call_at, at + 0x1a))
            return {};
        if (call.r == 0)
            return close(false);
        if (*allowed != 0 && half(app, u32(wide(p.chosen)) * 2 + u32(*allowed)) == 0)
            return close(false);
        return close(true);
    }
    case 0x1b:
        return close(false);
    case 0x4900:
        clampChosen(int16_t(p.chosen - p.shown));
        if (p.chosen < p.first)
        {
            const int16_t first = int16_t(p.first - p.shown);
            p.first = first;
            int32_t to = 0;
            if (first >= 0)
            {
                to = wide(count16) - 1;
                if (wide(p.first) <= to)
                    to = wide(p.first);
            }
            p.first = int16_t(to);
            p.last = int16_t(to + int16_t(p.shown - 1));
        }
        break;
    case 0x5100:
        clampChosen(int16_t(p.chosen + p.shown));
        if (p.chosen > p.last)
        {
            const int16_t last = int16_t(p.last + p.shown);
            p.last = last;
            int32_t to = 0;
            if (last >= 0)
            {
                to = wide(count16) - 1;
                if (wide(p.last) <= to)
                    to = wide(p.last);
            }
            p.last = int16_t(to);
            const int16_t first = int16_t(to - int16_t(p.shown - 1));
            p.first = first;
            int32_t from = 0;
            if (first >= 0)
            {
                from = wide(count16) - 1;
                if (wide(p.first) <= from)
                    from = wide(p.first);
            }
            p.first = int16_t(from);
        }
        break;
    case 0x4800:
        clampChosen(int16_t(p.chosen - 1));
        if (p.chosen < p.first)
        {
            --p.first;
            --p.last;
        }
        break;
    case 0x5000:
        clampChosen(int16_t(p.chosen + 1));
        if (p.chosen > p.last)
        {
            ++p.first;
            ++p.last;
        }
        break;
    default:
        return 0;
    }
    if (!point())
        return {};
    return 2;
}

/* 0x464560's: drop list `at` laid out: open, its box as wide as its widest
 * line and its marks' room, its lines shown high -- above its art when wide,
 * at its right or left, else below and right of it; closed, the art's own
 * size.  No caller reads its eax. */
Result dropLaidOut(App* app, ThreadState& ts, u32 esp, u32 at)
{
    const u32 frame = esp - 0x18, call_at = frame - 8;
    const int32_t gap = gameSettings(app).raceType == 3 ? 0x23 : 0xf;
    const Result lines = linesOf(app, ts, call_at - 4, item<DropItem>(app, at).listName, frame - 8);
    if (!lines)
        return {};
    if (item<DropItem>(app, at).open != 0 && *lines != 0)
    {
        if (!widestLine(app, ts, call_at, at, u32(*lines)))
            return {};
        DropItem& d = item<DropItem>(app, at);
        const int16_t width = int16_t(d.boxWidth + gap + 5);
        d.boxHeight = int16_t((d.shown << 4) + 5);
        d.boxWidth = width;
        if (d.wide != 0)
        {
            d.boxTop = int16_t(d.y - d.boxHeight + 0xe);
            d.boxLeft = int16_t(d.x + (d.right != 0 ? 0xaa : 0xc8) - d.boxWidth);
        }
        else
        {
            d.boxTop = int16_t(d.y + 0x10);
            d.boxLeft = int16_t(d.x + 0x40);
        }
        return 0;
    }
    DropItem& d = item<DropItem>(app, at);
    if (d.wide != 0)
    {
        d.boxLeft = d.right != 0 ? d.x : int16_t(d.x + 0xa);
        d.boxWidth = 0xcf;
        d.boxHeight = 0x13;
        d.boxTop = int16_t(d.y + 0xc);
    }
    else
    {
        d.boxWidth = 0x101;
        d.boxHeight = 0x1f;
        d.boxLeft = d.x;
        d.boxTop = d.y;
    }
    return 0;
}

}  // namespace

/* ---- The thunks ---- */

namespace
{

Args regsOf(const Entry& e)
{
    return {e.eax, e.edx, e.ebx, e.ecx};
}

}  // namespace

nfs::u32 joinListedGame(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, joined(app, ts, e.esp, regsOf(e)));
}

nfs::u32 drawReadyMarks(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readyMarks(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

nfs::u32 networkLeft(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, slideFinished(app, ts, e.esp, regsOf(e)));
}

nfs::u32 drawLicence(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, licenceDrawn(app, ts, e.esp, regsOf(e)));
}

nfs::u32 listColours(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, coloursOf(app, ts, e.esp, e.eax));
}

nfs::u32 listAllowed(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, allowedOf(app, ts, e.esp, e.eax));
}

nfs::u32 listLength(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, lengthOf(app, ts, e.esp, e.eax));
}

nfs::u32 findList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, linesOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x442d20: whether the menu's record at eax was edited (its word at
 * +0x1e). */
nfs::u32 menuRecordEdited(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(wide(half(app, e.eax + 0x1e))));
}

/* 0x442d00: whether menu eax was edited (0x24 bytes a menu at 0x6040d0). */
nfs::u32 menuEdited(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(wide(half(app, e.eax * 0x24 + 0x6040d0 + 0x1e))));
}

nfs::u32 loadShapes(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, shapesLoaded(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 redrawMenu(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, redrawn(app, ts, e.esp, regsOf(e)));
}

nfs::u32 textBoxDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textBoxDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 textBoxKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textBoxKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 textBoxUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textBoxLaidOut(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 textBoxShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textBoxShowing(app, ts, e.esp, regsOf(e)));
}

nfs::u32 textBoxHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textBoxHiding(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 slideTrackMap(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, mapSlid(app, ts, e.esp, e.eax));
}

nfs::u32 drawTrackFacts(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, trackFacts(app, ts, e.esp, regsOf(e)));
}

nfs::u32 drawGlow(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, glowDrawn(app, ts, e.esp, regsOf(e)));
}

nfs::u32 trackMapShape(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, mapShape(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

nfs::u32 rectDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, rectDrawn(app, ts, e.esp, e.eax));
}

/* 0x45fc90: a speech item draws nothing: 0. */
nfs::u32 speechDraw(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, 0);
}

/* 0x45fca0, 0x45fcb0: nor does it do anything as it is shown or hidden. */
nfs::u32 speechShown(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, e.eax);
}

nfs::u32 speechHidden(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, e.eax);
}

nfs::u32 textDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textDrawn(app, ts, e.esp, e.eax, regsOf(e)));
}

nfs::u32 textUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, textLaidOut(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 textShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    textShowing(app, e.eax);
    return e.back(ts, args, e.eax);
}

nfs::u32 imageDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, imageDrawn(app, ts, e.esp, e.eax, regsOf(e)));
}

nfs::u32 imageUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    imageLaidOut(app, e.eax);
    return e.back(ts, args, e.eax);
}

nfs::u32 imageShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, imageShowing(app, ts, e.esp, e.eax, regsOf(e)));
}

nfs::u32 pulseOn(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(pulsed(app)));
}

nfs::u32 pulseColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, pulseOf(app, int32_t(e.eax)));
}

nfs::u32 buttonDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, buttonDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 buttonKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, buttonKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 layoutMenuItem(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, menuItemLaidOut(app, ts, e.esp, e.eax));
}

nfs::u32 buttonShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, buttonShowing(app, ts, e.esp, e.eax));
}

nfs::u32 cascadeLayoutClosed(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    cascadeClosedBox(app, e.eax);
    return e.back(ts, args, e.eax);
}

nfs::u32 cascadeLineText(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadeLine(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

nfs::u32 cascadeDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadeDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 cascadePass(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadePassed(app, ts, e.esp, e.eax, 0x42, 0x58, 0x5a, 0x56, 0x54, 0x60));
}

nfs::u32 cascadeShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadeShowing(app, ts, e.esp, e.eax));
}

nfs::u32 cascadeKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadeKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 cascadeUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadeLaidOut(app, ts, e.esp, e.eax));
}

nfs::u32 checkboxDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, checkDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 checkboxKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, checkKeyed(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x462b20: a check box does nothing as it is shown. */
nfs::u32 checkboxShown(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, e.eax);
}

nfs::u32 checkboxUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, checkLaidOut(app, ts, e.esp, e.eax));
}

nfs::u32 boxOverlayDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, overlayDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 selectListShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, selectShowing(app, ts, e.esp, e.eax));
}

nfs::u32 selectListHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, selectHiding(app, ts, e.esp, e.eax));
}

nfs::u32 selectListDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, selectDrawn(app, ts, e.esp, e.eax, e.edx));
}

/* 0x462f90: a select list's pass: 0. */
nfs::u32 selectListPass(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, 0);
}

nfs::u32 selectListKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, selectKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 selectListUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    selectLaidOut(app, e.eax);
    return e.back(ts, args, e.eax);
}

nfs::u32 dropListDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, dropDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 dropListPass(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cascadePassed(app, ts, e.esp, e.eax, 0x40, 0x5c, 0x5e, 0x5a, 0x58, 0x64));
}

nfs::u32 dropListShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, dropShowing(app, ts, e.esp, e.eax));
}

nfs::u32 listKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 layoutList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, dropLaidOut(app, ts, e.esp, e.eax));
}

/* 0x4d6730 (stdcall: the angle, the scale). */
nfs::u32 drawTurnedShape(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             turnedShape(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                         int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x4d88f0 (stdcall: the four corners' colours). */
nfs::u32 drawColouredRectangle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 colours[] = {e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)};
    return e.finish(ts, args, colouredRectangle(app, ts, e.esp, regsOf(e), colours), 0x10);
}

}  // namespace buttons
}  // namespace port
}  // namespace nfs3hp
