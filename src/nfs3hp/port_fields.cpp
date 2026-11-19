#include "game_settings.h"
#include "native_load_carmodels.h"
#include "port_fields.h"
#include "port_gamecall.h"
#include "port_draw2d.h"
#include "port_text.h"
#include "os_window.h"
#include "native_ea_memory.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_glue.h"
#include "native_frontend_menud.h"
#include "native_frontend_screen.h"
#include "native_load_cardb.h"
#include "native_watcom.h"
#include "native_frontend_loada.h"
#include "native_frontend_loop.h"
#include "native_frontend_menus.h"
#include "native_frontend_wata.h"
#include "native_vertices.h"
#include "native_ea_formats.h"
#include "port_sound_effects.h"
#include "port_loop.h"
#include "port_shapes.h"
#include "native_graphics.h"

#include <cmath>
#include <optional>
#include <type_traits>
#include <cstring>
#include <functional>

/* Each sum and product rounds as the x87 code rounds it: none fused. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The menus' widgets that hold a value, rewritten (port_fields.h): the five
 * fields typed into, each drawn with its label and its value -- the line being
 * typed with its cursor (0x452220 on) while it is edited --, taking its keys,
 * laid out, its function told of a change, a value out of its range answered
 * by a dialog; the list of lines with its scroll bar and arrows; the sliders,
 * single and two-axis, their parts' places, values, art and the mouse on them;
 * the car's colours, a list of them opened under its swatch, and the car
 * records' colours (0x4397xx) and the colour wheel's from them.
 *
 * Each function is a typed core under its own name and a thunk under the
 * game's.  The thunk takes the arguments from the registers and the stack
 * and gives back what Watcom's convention promises the caller: the result in
 * eax -- what the game's code leaves there, bits it did not mean included --
 * every other register as it was, a stdcall's stack words taken off.  The
 * game's code leaves edx, ebx or ecx changed in a few (0x439810, 0x4397c0,
 * 0x4397d0, 0x457ae0, the fields' text, the range messages); no caller reads
 * them -- the callers by address and the menus' calls through the widget
 * table (0x44a2d0, 0x44a4f0, 0x44a860, 0x44a8b0, 0x44a960, 0x44a9e0 and the
 * screens' own), by liveness on nfs3.exe -- but registers those functions
 * keep.
 *
 * The cores call each other as C++; the port's 2D drawing, text, shapes and
 * the menu's click as the C++ thunks they are (as the game's code calls
 * their addresses); what is not rewritten -- the menu loop's, the dialogs',
 * the car records', the pointer's, the runtime's, EA's, the render state and
 * the clipper -- by address, from where the game's code had its stack, its
 * frame's words where the game's code points them.
 *
 * The x87's arithmetic is the game's to the bit: each product and sum through
 * single precision, a value stored as a float where
 * the code stores one, the compares leaving the status word as the code's. */
namespace nfs3hp
{
namespace port
{
namespace fields
{

namespace
{

using Float = nfs::Float;

/* What an fcom leaves in the status word (c3, c2 and c0 of the x87's), as fp::compare gives it. */
struct FpuStatus
{
    nfs::u16 word = 0;
    void set(const Float& a, const Float& b) { word = fp::compare(a, b); }
    bool c0() const { return (word & 0x100) != 0; }
    bool c3() const { return (word & 0x4000) != 0; }
};

/* ---- The game's records ---- */

#pragma pack(push, 1)
/* A menu item, the head every widget has: its flags (+4: bit 0 greyed, bit 2
 * not taking keys), its state (+5: bit 2 held -- being typed into, dragged),
 * its place, the screen it slides in with (a slider's), the box the pointer
 * finds it in (y, x, width, height). */
struct Item
{
    uint8_t head[4];
    uint8_t flags;      // +4
    uint8_t state;      // +5
    int16_t x, y;       // +6, +8
    int16_t screen;     // +0xa
    uint8_t unknown0c[0xc];
    int16_t greyed;     // +0x18
    int16_t boxY, boxX, boxWidth, boxHeight;  // +0x1a
    uint8_t unknown22[0x1a];
};
static_assert(sizeof(Item) == 0x3c, "a menu item's head is 0x3c bytes");

/* carclrselect: the player's car's colours, a list opened under a swatch. */
struct ColourList : Item
{
    u32 label;        // +0x3c
    int16_t player;     // +0x40: 0 or 1
    int16_t unknown42;
    int32_t chosen;     // +0x44
    int32_t highlighted;  // +0x48
    int32_t count;      // +0x4c
    uint32_t colour;    // +0x50: the chosen one's, ARGB
    u32 car;          // +0x54: the player's car's variable
    int16_t open;       // +0x58
    int16_t unknown5a;
    int16_t arrow;      // +0x5c: shapes
    int16_t box;        // +0x5e
    int32_t shownCar;   // +0x60
};

/* edint: a whole number typed in, within `least` and `most` where `checked`
 * is 1, shown with a unit's name (`unitsShown` 1 or 2). */
struct IntField : Item
{
    u32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t unknown43;
    int32_t least, most;  // +0x44
    uint8_t unitsShown;   // +0x4c
    uint8_t unknown4d[3];
    u32 unit, units;  // +0x50
    u32 value;        // +0x58: where it is kept
    int32_t cursor;     // +0x5c
    char typed[12];     // +0x60
    u32 changed;      // +0x6c: told when it changes
};

/* edstring: a text typed into a buffer of its own (`maxLength`), copied
 * where it is kept when Enter takes it; its cursor blinks. */
struct StringField : Item
{
    u32 label;        // +0x3c
    int16_t editing;    // +0x40
    int16_t blink;      // +0x42
    u32 target;       // +0x44
    u32 buffer;       // +0x48
    int16_t maxLength;  // +0x4c
    int16_t unknown4e;
    u32 changed;      // +0x50
};

/* edchar: a byte. */
struct CharField : Item
{
    u32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t least, most;  // +0x43
    uint8_t unitsShown;   // +0x45
    uint8_t unknown46[2];
    u32 unit, units;  // +0x48
    u32 value;        // +0x50
    int32_t cursor;     // +0x54
    char typed[8];      // +0x58
    u32 changed;      // +0x60
};

/* edfloat and edfixed: a float, a 16.16 number; shown with `decimals`. */
struct FloatField : Item
{
    u32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t unknown43;
    u32 least, most;  // +0x44: a float's bits (edfloat), an integer (edfixed)
    uint8_t unitsShown;   // +0x4c
    uint8_t unknown4d[3];
    u32 unit, units;  // +0x50
    u32 value;        // +0x58
    int32_t cursor;     // +0x5c
    int32_t decimals;   // +0x60
    char typed[16];     // +0x64
    u32 changed;      // +0x74
};

/* List: `rows` of its lines shown from `first`, `chosen` lit; a line 8 bytes
 * (its text first, its first byte's top bit marking it). */
struct ListBox : Item
{
    int16_t width;      // +0x3c
    int16_t rows;       // +0x3e
    int16_t chosen;     // +0x40
    int16_t first;      // +0x42
    u32 choose;       // +0x44: told of a line chosen by the pointer
    u32 lines;        // +0x48
    int16_t count;      // +0x4c
};
#pragma pack(pop)
static_assert(sizeof(ColourList) == 0x64, "carclrselect is 0x64 bytes");
static_assert(sizeof(IntField) == 0x70, "edint is 0x70 bytes");
static_assert(sizeof(FloatField) == 0x78, "edfloat is 0x78 bytes");

/* ---- The game's variables ---- */

// The menus drawn over a race (its pause screens): by the 3D card's font.
constexpr Var<int32_t> kOverRace{0x559234};
// The menus' state bits (the second byte: 0x20 the 3D card's fonts drawn at
// once), the colour set's, the 2D screens sliding in.
constexpr Var<uint8_t> kMenuBits1{0x55eb2d}, kMenuBits3{0x55eb2f};
constexpr Var<int32_t> kScreensSliding{0x559c60};
// A car's record's colours: the chosen (a dword a player), how many less
// one, the colours (4 bytes each), the custom one (a player's).
constexpr u32 kChosenColours = 0x32c, kLastColour = 0x334, kColours = 0x338, kCustomColours = 0x378;
// The pointer.
constexpr Var<int32_t> kPointerX{0x55e5e4}, kPointerY{0x55e5e8};
// The cursor's colour: the 3D card's font's, the 2D screens'.
constexpr Var<u32> kCursorColour3d{0x561b38}, kCursorColour{0x559438};
// The 3D card's font: its colour, whether only measured, which font; its
// characters' metrics by number.
constexpr Var<u32> kTextColour{0x561b34};
constexpr Var<u32> kMeasuring{0x561b3c};
constexpr Var<uint8_t> kFont3d{0x840184};
constexpr u32 kGlyphWidth3d = 0x83fc40, kGlyphHeight3d = 0x83fe00, kTextureY3d = 0x83f700, kTextureX3d = 0x83f8c0;
// The 2D screens' font: which (0x24 the accented one), its colour, its
// texture, the nudge, its characters' squares.
constexpr Var<int32_t> kScreenFont{0x664650};
constexpr Var<u32> kScreenTextColour{0x559434}, kFontTexture{0x8b7624};
constexpr Var<int32_t> kNudge{0x563aa0};
constexpr u32 kGlyphU = 0x8b7858, kGlyphV = 0x8b774a, kGlyphWidth = 0x8b7966, kGlyphHeight = 0x8b763c;
// The 2D screens' quad (0x20 bytes a corner), its depth and w.
constexpr u32 kQuad = 0x563ab0;
constexpr Var<float> kScreenDepth{0x563aa8}, kScreenW{0x563aac};
constexpr u32 kNearDepth = 0x37800080, kNearW = 0x3f7fff00;
// The clip's bounds, compared as the floats' bits: w, left, bottom, top, right.
constexpr Var<int32_t> kClipFar{0x7d34f8}, kClipLeft{0x7d34fc}, kClipBottom{0x7d3500}, kClipTop{0x7d3504};
constexpr Var<int32_t> kClipRight{0x7d350c};
// A list's line's height.
constexpr Var<int32_t> kLineHeight{0x678a40};
// The dialog's text and its lines and buttons.
constexpr u32 kDialogText = 0x559894, kDialogLines = 0x559994, kDialogButtons = 0x55999c;
// The fields' data the code copies into its frame (four blocks of 0x100).
constexpr u32 kIntBlank = 0x4576e0, kCharBlank = 0x4577e0, kFloatBlank = 0x4578e0, kFixedBlank = 0x4579e0;
// Their constants: 0.4, 1/65536 twice, 65536.
constexpr Var<double> kSpacing{0x5395f8}, kFixedUnit{0x539b7c}, kFixedUnit2{0x539b84}, kFixedScale{0x539b8c};

// The texts: "%s", "%d %s", "%d", the ranges', "_", "", "%.*f %s", "%.*f",
// "%f", "%.6f", "%s...", "%c ", "Ag", "<%s>"; the arrows' shapes' names; the
// screens' items.
constexpr u32 kS = 0x539b0c, kDS = 0x539b10, kD = 0x539b18, kIntRange = 0x539b1c, kUnderline = 0x539b38;
constexpr u32 kNoName = 0x539b3c, kFS = 0x539b40, kF = 0x539b48, kFloatRange = 0x539b50, kScanF = 0x539b6c;
constexpr u32 kF6 = 0x539b70, kEllipsis = 0x539ab0, kCharSpace = 0x5395f4, kAg = 0x53aba8;
constexpr u32 kMarked = 0x53abac, kPlain = 0x53abb4;
constexpr u32 kArrowName = 0x539ab8, kBoxName = 0x539ac0;
constexpr u32 kFirstCarItem = 0x539ac8, kSecondCarItem = 0x539adc, kColourEdit = 0x539af0;
constexpr u32 kNoCarText = 0x537690;

/* What is not rewritten, called by address: the menu loop's (whether an item
 * is on the screen shown now, a screen's item by name, a screen by name, an
 * item changed, the colour set's state), the dialog's question, the pointer
 * (placed, in a box, the line under it), the car records', the runtime's,
 * EA's memory, the text's height, the render state, the clippers. */
constexpr u32 kShownNow = 0x44a660, kFindItem = 0x442a40, kMenuNamed = 0x44aeb0, kItemChanged = 0x44a470;
constexpr u32 kColourSetState = 0x449df0;
constexpr u32 kSetPointer = 0x49bfc0, kPointerInBox = 0x49c1f0, kLineAtPointer = 0x49c1c0;
constexpr u32 kCarRecord = 0x439a50;
constexpr u32 kSaveSettings = 0x4dcbc0;
constexpr u32 kSprintf = 0x4df690, kSscanf = 0x4f0548, kStrncpy = 0x4e0e30, kMemset = 0x4e0640;
constexpr u32 kAllocate = 0x4e1620, kFree = 0x4e1890;
constexpr u32 kFontHeight = 0x4f0840, kTextExtent = 0x4f0c90, kCursorQuad = 0x4d1120;
constexpr u32 kRenderState = 0x431900, kClipTriangle = 0x4c11b0, kClipQuad = 0x4c12e0;

/* The port's own, called as the game's code calls their addresses. */
constexpr u32 kSelectFont = 0x452100, kTextWidth = 0x452020;  // 0x452100, 0x452020
constexpr u32 kDrawMenuText = 0x452190, kDrawTextAt = 0x451590;  // 0x452190, 0x451590
constexpr u32 kSetTextColour = 0x4513c0, kLanguageText = 0x4d1850;  // 0x4513c0, 0x4d1850
constexpr u32 kFlushFonts = 0x4d1340;  // 0x4d1340
constexpr u32 kToScreenX = 0x451400, kToScreenY = 0x451440;  // 0x451400, 0x451440
constexpr u32 kGradient = 0x4d8550, kFilled = 0x4d8910;  // 0x4d8550, 0x4d8910
constexpr u32 kFrame = 0x4d8930, kFieldBox = 0x4d8ac0;  // 0x4d8930, 0x4d8ac0
constexpr u32 kAccented = 0x4d9320, kScreenCharacter = 0x4d95c0;
constexpr u32 kShapeByName = 0x4d5db0, kDrawShape = 0x4d7990;  // 0x4d5db0, 0x4d7990
constexpr u32 kMenuClick = 0x4181d0;  // 0x4181d0

constexpr u32 kEnter = 0xd, kEscape = 0x1b, kBackspace = 8, kDelete = 0x5300;
constexpr u32 kUp = 0x4800, kDown = 0x5000, kLeft = 0x4b00, kRight = 0x4d00;

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

void setHalf(App* app, u32 at, int32_t value)
{
    app->getMemory<uint16_t>(at) = uint16_t(value);
}

uint8_t byteAt(App* app, u32 at)
{
    return app->getMemory<uint8_t>(at);
}

void setByte(App* app, u32 at, uint8_t value)
{
    app->getMemory<uint8_t>(at) = value;
}

float floatAt(App* app, u32 at)
{
    return app->getMemory<float>(at);
}

void setFloat(App* app, u32 at, float value)
{
    app->getMemory<float>(at) = value;
}

uint32_t bitsOf(float f)
{
    uint32_t b;
    std::memcpy(&b, &f, 4);
    return b;
}

float fromBits(uint32_t b)
{
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

/* fld and fstp of a float; fild and fstp of a dword. */
float viaX87(float f)
{
    return float(Float(f));
}

float ofInt(int32_t i)
{
    return float(Float(i));
}

/* A double's two words, as fstp qword leaves them on the stack. */
struct Words
{
    u32 lo, hi;
};

Words wordsOf(double d)
{
    uint64_t b;
    std::memcpy(&b, &d, 8);
    return {u32(b), u32(b >> 32)};
}

/* sar 0x1f, sub, sar 1: halved toward 0. */
int32_t halved(int32_t v)
{
    return int32_t(uint32_t(v) - uint32_t(v >> 31)) >> 1;
}

/* repne scasb's length of the text at `s`. */
u32 length(App* app, u32 s)
{
    u32 n = 0;
    while (byteAt(app, s + n) != 0)
        ++n;
    return n;
}

/* The registers a function pushes on entry, written below `esp` in the order
 * it pushes them -- what is read of its frame past its locals (a format's
 * words logged, a text written past a buffer) finds them there --, and popped
 * back from there on its way out. */
void pushed(App* app, u32 esp, std::initializer_list<u32> values)
{
    for (const u32 value : values)
        setWord(app, esp -= 4, value);
}

void popped(App* app, u32 esp, std::initializer_list<u32*> registers)
{
    for (u32* r : registers)
        *r = word(app, esp -= 4);
}

u32 valueOf(u32 value)
{
    return value;
}

template <class T>
u32 valueOf(const std::optional<T>& value)
{
    return u32(*value);
}

/* A typed core called as C++ where the game's code calls its address: from `esp`, the registers
 * loaded as the call by address loads them (`a`) and kept after it, as the callee keeps them;
 * `result` its eax; false when the game stopped in it. */
template <u32 Address, auto Core, typename... A>
bool callCore(App* app, ThreadState& ts, u32 esp, u32& result, A... args)
{
    ts.sp = esp;
    const auto value = guest::direct<Address, Core>(app, ts, args...);
    if (ts.terminate)
        return false;
    result = valueOf(value);
    return true;
}

/* The same for a core whose answer is no eax (the render state's): `result` left as it is. */
template <u32 Address, auto Core, typename... A>
bool callCoreNoEax(App* app, ThreadState& ts, u32 esp, A... args)
{
    ts.sp = esp;
    guest::direct<Address, Core>(app, ts, args...);
    return !ts.terminate;
}

/* The typed cores called as C++ where the game's code calls their addresses, from `esp` (the call's frame
 * base esp - 4 - 4 * Words): their answer in `*result` when given; false when the game stopped in one. */
template <class T>
struct is_optional : std::false_type
{
};
template <class T>
struct is_optional<std::optional<T>> : std::true_type
{
};

template <class F>
bool runCore(ThreadState& ts, u32 esp, u32* result, F f)
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
        if constexpr (is_optional<std::decay_t<decltype(v)>>::value)
        {
            if (!v)
                return false;
            if (result != nullptr)
                *result = u32(*v);
        }
        else if (result != nullptr)
            *result = u32(v);
        return true;
    }
}

template <u32 Address, auto Core, unsigned Words, typename... A>
bool coreAt(App* app, ThreadState& ts, u32 esp, u32* result, A... args)
{
    return runCore(ts, esp, result,
                   [&] { return guest::direct<Address, Core>(app, ts, u32(esp - 4 - 4 * Words), args...); });
}

template <u32 Address, auto Core, typename... A>
bool plainAt(App* app, ThreadState& ts, u32 esp, u32* result, A... args)
{
    return runCore(ts, esp, result, [&] { return guest::direct<Address, Core>(app, ts, args...); });
}

/* A core that leaves eax as it came: `*result` is `eax` after it. */
template <u32 Address, auto Core, typename... A>
bool keepAt(App* app, ThreadState& ts, u32 esp, u32* result, u32 eax, A... args)
{
    if (result != nullptr)
        *result = eax;
    return runCore(ts, esp, nullptr, [&] { return guest::direct<Address, Core>(app, ts, args...); });
}

/* A box core whose four colours the game's code pushes. */
template <u32 Address, auto Core>
bool boxedAt(App* app, ThreadState& ts, u32 esp, u32* result, u32 x, u32 y, u32 width, u32 height, u32 c0,
             u32 c1, u32 c2, u32 c3)
{
    const uint32_t colours[4] = {c0, c1, c2, c3};
    return coreAt<Address, Core, 4>(app, ts, esp, result, x, y, width, height, static_cast<const uint32_t*>(colours));
}

/* The same, called by name, as C++. */
template <auto Core, unsigned Words, typename... A>
bool namedAt(App* app, ThreadState& ts, u32 esp, u32* result, A... args)
{
    return runCore(ts, esp, result, [&] {
        const auto v = Core(app, ts, u32(esp - 4 - 4 * Words), args...);
        ts.sp = esp;
        return v;
    });
}

template <auto Core, typename... A>
auto callNamedCore(App* app, ThreadState& ts, A... args)
{
    if constexpr (std::is_invocable_v<decltype(Core), App*, ThreadState&, A...>)
        return Core(app, ts, args...);
    else
        return Core(app, args...);
}

template <auto Core, typename... A>
bool namedPlainAt(App* app, ThreadState& ts, u32 esp, u32* result, A... args)
{
    return runCore(ts, esp, result, [&] { return callNamedCore<Core>(app, ts, args...); });
}

template <auto Core, typename... A>
bool keepNamedAt(App* app, ThreadState& ts, u32 esp, u32* result, u32 eax, A... args)
{
    if (result != nullptr)
        *result = eax;
    return runCore(ts, esp, nullptr, [&] { return callNamedCore<Core>(app, ts, args...); });
}

template <auto Core>
bool boxedNamedAt(App* app, ThreadState& ts, u32 esp, u32* result, u32 x, u32 y, u32 width, u32 height, u32 c0,
                  u32 c1, u32 c2, u32 c3)
{
    const uint32_t colours[4] = {c0, c1, c2, c3};
    return namedAt<Core, 4>(app, ts, esp, result, x, y, width, height, static_cast<const uint32_t*>(colours));
}

/* 0x4d1850, by name (esp restored). */
bool lineAt(App* app, ThreadState& ts, u32 esp, u32* result, int32_t number, u32 edx)
{
    ts.sp = esp;
    const auto v = text::lineText(app, ts, esp - 4, number, edx);
    ts.sp = esp;
    if (!v)
        return false;
    if (result != nullptr)
        *result = u32(*v);
    return true;
}

/* The menu loop's: whether `item`, of widget `type`, is on the screen shown. */
bool shownNow(App* app, ThreadState& ts, u32 esp, u32 item, u32& r)
{
    return plainAt<kShownNow, frontend::loop::shownNow>(app, ts, esp, &r, item);
}

/* sprintf into `buffer`, its format's words after it. */
bool format(App* app, ThreadState& ts, u32 esp, std::initializer_list<u32> words)
{
    const u32* const w = words.begin();
    guest::Words more({});
    for (const u32* v = w + 2; v != words.end(); ++v)
        more.word[more.count++] = *v;
    u32 ignored;
    return callCore<kSprintf, watcom::clib::sprintf>(app, ts, esp, ignored, w[0], w[1], more);
}

/* The dialog's question, its text at kDialogText: an OK. */
bool question(App* app, ThreadState& ts, u32 esp, u32& r)
{
    return callCore<0x4446d0, frontend::dialogs::openQuestion>(app, ts, esp - 4 * 4, r, 2u, kDialogLines, 1u,
                                                               kDialogButtons, 0u, 1u, 0u, 0u);
}

/* The clip code of the corner at `at`: 0x10 behind the eye or past the far
 * clip, else 8 above the clip, 4 below, 1 left of it, 2 right of it.  Where
 * the code compared w alone, `farOnly`. */
uint8_t clipCode(App* app, u32 at, bool& farOnly)
{
    const int32_t w = int32_t(word(app, at + 0xc));
    farOnly = w < 0 || w >= kClipFar(app);
    if (farOnly)
        return 0x10;
    uint8_t code = 0;
    const int32_t y = int32_t(word(app, at + 4));
    if (y < 0 || y < kClipTop(app))
        code |= 8;
    else if (y > kClipBottom(app))
        code |= 4;
    const int32_t x = int32_t(word(app, at));
    if (x < 0 || x < kClipLeft(app))
        code |= 1;
    else if (x > kClipRight(app))
        code |= 2;
    return code;
}

/* The corners `count` of the quad marked; ecx as the code leaves it after the
 * last: the clip's left, or its far bound. */
u32 markCorners(App* app, int count)
{
    u32 ecx = 0;
    for (int corner = 0; corner < count; ++corner)
    {
        bool farOnly;
        const u32 at = kQuad + 0x20 * u32(corner);
        setByte(app, at + 0x14, clipCode(app, at, farOnly));
        ecx = u32(farOnly ? kClipFar(app) : kClipLeft(app));
    }
    return ecx;
}

}  // namespace

/* ======== The car records' colours ======== */

namespace
{

// colourCount and customColoursRefreshed: port_fields.h.
Result colourRgb(App* app, ThreadState& ts, u32 esp, u32 car, int32_t colour, int32_t player, u32 out0, u32 out1,
                 u32 out2, u32 out3);

bool carRecordOf(App* app, ThreadState& ts, u32 esp, u32 car, u32& rec)
{
    return callCore<kCarRecord, load::recordAt>(app, ts, esp, rec, car);
}

}  // namespace

/* 0x439760's: how many colours car `car` has, its custom one counted. */
Result colourCount(App* app, ThreadState& ts, u32 esp, u32 car)
{
    const u32 at = esp - 4;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    return int32_t(word(app, rec + kLastColour) + 1);
}

namespace
{

/* 0x439780's: the number of its custom colour, its last. */
Result customColour(App* app, ThreadState& ts, u32 esp, u32 car)
{
    const Result n = colourCount(app, ts, esp - 4 - 4, car);
    if (!n)
        return {};
    return int32_t(u32(*n) - 1);
}

}  // namespace

/* 0x439780's core: car `car`'s custom colour's number; esp the frame base. */
Result customColourOf(App* app, ThreadState& ts, u32 esp, u32 car)
{
    return customColour(app, ts, esp, car);
}

/* 0x439790's: the colour player `player` chose for it, -1 for no player. */
Result chosenColour(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player)
{
    const u32 at = esp - 4;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    if (player == 0 || player == 1)
        return int32_t(word(app, rec + u32(player) * 4 + kChosenColours));
    return -1;
}

/* 0x439ab0's: text `n` of car `car`'s, "" past them or without its record. */
Result carTextOf(App* app, ThreadState& ts, u32 esp, u32 car, int32_t n)
{
    const u32 at = esp - 8;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    if (rec == 0)
        return int32_t(kNoCarText);
    const int32_t last = int32_t(half(app, rec + 4)) - 1;
    if (n > last)
        return int32_t(kNoCarText);
    u32 r;
    if (!callCore<0x439af0, load::fecTextOf>(app, ts, at, r, rec + 4, u32(n)))
        return {};
    return int32_t(r);
}

namespace
{

/* 0x4397c0's: the name of colour `colour` (its text 0x1e on). */
Result colourName(App* app, ThreadState& ts, u32 esp, u32 car, int32_t colour)
{
    return carTextOf(app, ts, esp - 4 - 4, car, int32_t(u32(colour) + 0x1e));
}

/* 0x43b7f0's: car `car`'s record written to its .fec file: 0, or -1. */
Result colourSaved(App* app, ThreadState& ts, u32 esp, u32 car)
{
    const u32 ebp = esp - 0x10, at = ebp - 0x12c, path = ebp - 0x12c;
    u32 rec, r;
    if (!carRecordOf(app, ts, at, car, rec) || !callCore<0x43b680, load::fecPathOf>(app, ts, at, r, car, path)
        || !callCore<0x43b4f0, load::fecWritten>(app, ts, at, r, path, rec))
        return {};
    return r == 0 ? 0 : -1;
}

}  // namespace

/* 0x4397d0's: player `player`'s colour of car `car` set to `colour`, the
 * record saved and the custom colours taken afresh: 0, or -1. */
Result colourSet(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player, int32_t colour)
{
    const u32 at = esp - 0xc;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    if (player != 0 && player != 1)
        return -1;
    setWord(app, rec + u32(player) * 4 + kChosenColours, u32(colour));
    const Result saved = colourSaved(app, ts, at - 4, car);
    if (!saved || !customColoursRefreshed(app, ts, at - 4, u32(*saved)))
        return {};
    return *saved == 0 ? 0 : -1;
}

namespace
{

/* 0x439810's: colour `colour` of car `car` -- its custom one, the player's,
 * for the last -- its four bytes to `out0` to `out3`: 0, or -1 past them. */
Result colourRgb(App* app, ThreadState& ts, u32 esp, u32 car, int32_t colour, int32_t player, u32 out0, u32 out1,
                 u32 out2, u32 out3)
{
    const u32 at = esp - 0xc;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    const Result count = colourCount(app, ts, at - 4, car);
    if (!count)
        return {};
    if (colour > *count)
        return -1;
    u32 from;
    if (colour == int32_t(u32(*count) - 1))
    {
        if (player < 0 || player > 1)
            return -1;
        from = rec + u32(player) * 4 + kCustomColours;
    }
    else
        from = rec + u32(colour) * 4 + kColours;
    setByte(app, out0, byteAt(app, from));
    setByte(app, out1, byteAt(app, from + 1));
    setByte(app, out2, byteAt(app, from + 2));
    setByte(app, out3, byteAt(app, from + 3));
    return 0;
}

}  // namespace

Result carColourBytes(App* app, ThreadState& ts, u32 esp, u32 car, int32_t colour, int32_t player, u32 out0, u32 out1,
                      u32 out2, u32 out3)
{
    return colourRgb(app, ts, esp, car, colour, player, out0, out1, out2, out3);
}

/* 0x4398c0's: player `player`'s custom colour of car `car` set, saved: 0 or
 * -1. */
Result customColourSet(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player, uint8_t b0, uint8_t b1, uint8_t b2,
                       uint8_t b3)
{
    const u32 at = esp - 8;
    u32 rec;
    if (!carRecordOf(app, ts, at, car, rec))
        return {};
    if (player < 0 || player > 1)
        return -1;
    const u32 to = rec + u32(player) * 4 + kCustomColours;
    setByte(app, to, b0);
    setByte(app, to + 1, b1);
    setByte(app, to + 2, b2);
    setByte(app, to + 3, b3);
    const Result saved = colourSaved(app, ts, at - 4, car);
    if (!saved)
        return {};
    if (*saved != 0)
        return -1;
    if (!customColoursRefreshed(app, ts, at - 4, 0))
        return {};
    return 0;
}

/* 0x472360's: unless they are kept as they are, both players' cars' chosen
 * colours' bytes taken into the settings (0x6fd307 on, 0x6fd373 on) and the
 * settings saved; eax as it was otherwise. */
Result customColoursRefreshed(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    if (gameSettings(app).replayMode != 0)
        return int32_t(eax);
    const u32 at = esp - 0x14, inner = at - 0xc - 4;
    const Result c0 = chosenColour(app, ts, inner, gameSettings(app).players[0].car, 0);
    if (!c0 || !colourRgb(app, ts, inner, gameSettings(app).players[0].car, *c0, 0, SETTINGS_ADDRESS(players[0].colour), SETTINGS_ADDRESS(players[0].colour[1]), SETTINGS_ADDRESS(players[0].colour[2]), SETTINGS_ADDRESS(players[0].colour[3])))
        return {};
    const Result c1 = chosenColour(app, ts, inner, gameSettings(app).players[1].car, 1);
    if (!c1)
        return {};
    const Result r1 = colourRgb(app, ts, inner, gameSettings(app).players[1].car, *c1, 1, SETTINGS_ADDRESS(players[1].colour), SETTINGS_ADDRESS(players[1].colour[1]), SETTINGS_ADDRESS(players[1].colour[2]), SETTINGS_ADDRESS(players[1].colour[3]));
    if (!r1)
        return {};
    u32 r;
    if (!plainAt<kSaveSettings, frontend::loada::tuningForPlayers>(app, ts, at, &r))
        return {};
    return int32_t(r);
}

namespace
{

/* 0x43dbe0's: the colour wheel `item`'s colours (hue, saturation, value and
 * 0xff, twice) from its player's car's custom colour. */
Result wheelFromCar(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x18, at = ebp - 0x30;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const int32_t player = int32_t(word(app, item + 0x60));
    const u32 car = player == 0 ? gameSettings(app).players[0].car : gameSettings(app).players[1].car;
    const Result colour = customColour(app, ts, at - 4, car);
    if (!colour || !colourRgb(app, ts, at - 0xc - 4, car, *colour, player, ebp - 4, ebp - 0x10, ebp - 0xc, ebp - 8))
        return {};
    setWord(app, ebp - 0x30, byteAt(app, ebp - 4));
    setWord(app, ebp - 0x2c, byteAt(app, ebp - 0x10));
    setWord(app, ebp - 0x28, byteAt(app, ebp - 0xc));
    setWord(app, ebp - 0x24, 0xff);
    if (!callCore<0x49c880, load::colourToHsv>(app, ts, at, r, ebp - 0x30, ebp - 0x20))
        return {};
    for (u32 i = 0; i < 3; ++i)
    {
        setWord(app, item + 0x40 + 4 * i, word(app, ebp - 0x20 + 4 * i));
        setWord(app, item + 0x50 + 4 * i, word(app, ebp - 0x20 + 4 * i));
    }
    setWord(app, item + 0x4c, 0xff);
    setWord(app, item + 0x5c, 0xff);
    return int32_t(word(app, ebp - 0x18));
}

/* 0x43dd50's: the colour wheel `item` given to player `player`, its colours
 * taken from the car, the item changed and held. */
Result wheelGiven(App* app, ThreadState& ts, u32 esp, u32 item, int32_t player)
{
    const u32 at = esp - 0xc;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    setWord(app, item + 0x60, u32(player));
    if (!wheelFromCar(app, ts, at - 4, item)
        || !gameCall(app, ts, at, kItemChanged, {item, u32(player), u32(player), item}, {}, r))
        return {};
    const uint8_t state = uint8_t(byteAt(app, item + 5) & 0xeb);
    setByte(app, item + 5, uint8_t(state | 4));
    return int32_t((r & 0xffff00ffu) | u32(state) << 8);
}

}  // namespace

/* 0x439760: eax a car: how many colours it has. */
nfs::u32 carColourCount(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, colourCount(app, ts, e.esp, e.eax));
}

/* 0x439780: eax a car: its custom colour's number. */
nfs::u32 carCustomColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, customColour(app, ts, e.esp, e.eax));
}

/* 0x439790: eax a car, edx a player: the colour the player chose. */
nfs::u32 carColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, chosenColour(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x4397c0: eax a car, edx a colour: its name. */
nfs::u32 carColourName(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, colourName(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x4397d0: eax a car, edx a player, ebx a colour: set and saved. */
nfs::u32 setCarColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, colourSet(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x439810 (stdcall 0xc): eax a car, edx a colour, ebx a player: its bytes to
 * [ecx] and the stack's three pointers. */
nfs::u32 carColourRgb(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             colourRgb(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, e.arg(app, 0), e.arg(app, 1),
                       e.arg(app, 2)),
             0xc);
}

/* 0x4398c0 (stdcall 8): eax a car, edx a player: its custom colour bl, cl and
 * the stack's two bytes. */
nfs::u32 setCarCustomColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             customColourSet(app, ts, e.esp, e.eax, int32_t(e.edx), uint8_t(e.ebx), uint8_t(e.ecx),
                             uint8_t(e.arg(app, 0)), uint8_t(e.arg(app, 1))),
             8);
}

/* 0x439ab0: eax a car, edx a number: its text. */
nfs::u32 carText(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, carTextOf(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x43b7f0: eax a car: its record saved. */
nfs::u32 saveCarColours(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, colourSaved(app, ts, e.esp, e.eax));
}

/* 0x43dbe0: the colour wheel eax from its car. */
nfs::u32 wheelFromCarColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, wheelFromCar(app, ts, e.esp, e.eax));
}

/* 0x43dd50: the colour wheel eax given to player edx. */
nfs::u32 wheelForPlayer(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, wheelGiven(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x472360: the custom colours taken into the settings. */
nfs::u32 refreshCustomColours(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, customColoursRefreshed(app, ts, e.esp, e.eax));
}

/* ======== A field's text and cursor ======== */

namespace
{

/* 0x4513e0's: the cursor's colour, the 3D card's font's over a race. */
void cursorColourSet(App* app, u32 colour)
{
    (kOverRace(app) != 0 ? kCursorColour3d : kCursorColour).set(app, colour);
}

/* 0x4d11f0's cursor: a quad (0x4d1120) at x, y in the 3D card's font, of
 * `shape` (its float the stack's word). */
bool cursor3d(App* app, ThreadState& ts, u32 at, float x, int32_t y, int32_t shape, u32 counted)
{
    const int32_t whole = chopped<int32_t>(ts, Float(x));
    return gameCall(app, ts, at, kCursorQuad, {u32(whole), u32(y), 0, counted}, {bitsOf(ofInt(shape))});
}

/* 0x4d11f0's: text `text` in the 3D card's font at x, y, the cursor before
 * character `cursor` where `show` -- of `shape` --: how wide. */
Result cursorText3d(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, int32_t cursor, int32_t shape,
                    u32 show)
{
    const u32 ebp = esp - 0xc, at = ebp - 0x1c;
    const u32 colour = kTextColour(app) | 0xff000000;
    float place = ofInt(x);
    int32_t i = 0;
    for (u32 s = text;; ++s, ++i)
    {
        const uint8_t c = byteAt(app, s);
        if (c == 0)
            break;
        const uint8_t index = uint8_t(c - 0x20 + uint8_t(kFont3d(app) * 0xe0));
        const u32 width = byteAt(app, kGlyphWidth3d + index);
        if (i == cursor && show != 0 && !cursor3d(app, ts, at, place, y, shape, u32(i)))
            return {};
        const text::WidthResult advance = guest::direct<0x4d06f0, text::characterDrawn>(
            app, ts, at - 0x18 - 4,
            text::Character{index, int32_t(byteAt(app, kTextureX3d + index)), int32_t(byteAt(app, kTextureY3d + index)),
                            place, y, int32_t(width), int32_t(byteAt(app, kGlyphHeight3d + index)) + 1, colour,
                            int32_t(kMeasuring(app))});
        if (ts.terminate)
            return {};
        place = float(fp::add(Float(advance->value), Float(place)));
    }
    if (i == cursor && show != 0 && !cursor3d(app, ts, at, place, y, shape, u32(i)))
        return {};
    return chopped<int32_t>(ts, fp::sub(Float(place), Float(x)));
}

/* 0x4da560's: the 2D screens' cursor, a quad at x, y as high as the font's
 * characters and `width` wide, `left` its left side's colour and `right` its
 * right's. */
Result cursorDrawn(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, u32 left, u32 right)
{
    if (kOverRace(app) != 0)
        return x;
    const u32 at = esp - 0xc - 0x10;
    const int32_t font = kScreenFont(app);
    const int32_t size = font == 0x24 ? 0x20 : font == 0x10 ? 0x1c : font == 0xd ? 0x18 : 0x10;
    // The corners: 0 at x, y, 1 right of it, 2 below that, 3 below 0.
    setFloat(app, kQuad + 0x68, viaX87(kScreenDepth(app)));
    setFloat(app, kQuad + 0x6c, viaX87(kScreenW(app)));
    setWord(app, kQuad + 0x70, left);
    setWord(app, kQuad + 0x10, left);
    setFloat(app, kQuad + 0x48, viaX87(floatAt(app, kQuad + 0x68)));
    setFloat(app, kQuad + 0x4c, viaX87(floatAt(app, kQuad + 0x6c)));
    setFloat(app, kQuad + 0x60, ofInt(x));
    setFloat(app, kQuad + 0x24, ofInt(y));
    const float depth = viaX87(floatAt(app, kQuad + 0x48)), w = viaX87(floatAt(app, kQuad + 0x4c));
    const float left0 = viaX87(floatAt(app, kQuad + 0x60)), top = viaX87(floatAt(app, kQuad + 0x24));
    setFloat(app, kQuad + 0x28, depth);
    setFloat(app, kQuad + 0x2c, w);
    setWord(app, kQuad + 0x50, right);
    setWord(app, kQuad + 0x30, right);
    setFloat(app, kQuad + 0, left0);
    setFloat(app, kQuad + 4, top);
    setWord(app, kQuad + 8, word(app, kQuad + 0x28));
    setFloat(app, kQuad + 0xc, viaX87(floatAt(app, kQuad + 0x2c)));
    const float right0 = float(fp::add(Float(nfs::s32(width)), Float(left0)));
    const float bottom = float(fp::add(Float(top), Float(nfs::s32(size))));
    setFloat(app, kQuad + 0x40, right0);
    setFloat(app, kQuad + 0x64, bottom);
    setWord(app, kQuad + 0x20, word(app, kQuad + 0x40));
    setFloat(app, kQuad + 0x44, bottom);
    u32 r;
    if (!callCoreNoEax<kRenderState, graphics::sendState>(app, ts, at,
                                                          u32(1), u32(0)))
        return {};
    markCorners(app, 4);
    if (!coreAt<kClipQuad, vertices::quadClippedAt, 0>(app, ts, at, &r, kQuad, kQuad + 0x20, kQuad + 0x40, kQuad + 0x60))
        return {};
    for (const u32 colour : {0x70u, 0x10u, 0x50u, 0x30u})
        setWord(app, kQuad + colour, 0xffffffff);
    return -1;
}

/* 0x4da8c0's: text `text` on a 2D screen at x, y (`measure`: measured only),
 * the cursor before character `cursor` where `show`, `width` wide: how
 * wide; 0 over a race. */
Result cursorText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, u32 measure, int32_t cursor,
                  int32_t width, u32 show)
{
    const u32 ebp = esp - 0xc, at = ebp - 0x24;
    if (kOverRace(app) != 0)
    {
        kNudge.set(app, 0);
        return 0;
    }
    const u32 colour = kScreenTextColour(app) | 0xff000000;
    const u32 cursorColour = kCursorColour(app) | 0xff000000;
    u32 r;
    if (!callCoreNoEax<kRenderState, graphics::sendState>(app, ts, at, u32(1),
                                                          kFontTexture(app)))
        return {};
    int32_t place = x, i = 0;
    for (u32 s = text;; ++s, ++i)
    {
        const uint8_t c = byteAt(app, s);
        if (c == 0)
            break;
        u32 index;
        if (kScreenFont(app) == 0x24)
        {
            if (!coreAt<0x4d9320, draw2d::accented, 0>(app, ts, at, &index, u32(c)))
                return {};
        }
        else
            index = u32(c) - 0x20;
        const u32 w = byteAt(app, kGlyphWidth + index);
        if (i == cursor && show != 0
            && !cursorDrawn(app, ts, at - 4 - 4, place, y, width, cursorColour, cursorColour))
            return {};
        if (!coreAt<0x4d95c0, draw2d::screenCharacter, 5>(
                app, ts, at, &r, int32_t(byteAt(app, kGlyphU + index)), int32_t(byteAt(app, kGlyphV + index)), place, y,
                int32_t(w), int32_t(byteAt(app, kGlyphHeight + index)), colour, measure))
            return {};
        place = int32_t(u32(place) + r);
    }
    if (i == cursor && show != 0 && !cursorDrawn(app, ts, at - 4 - 4, place, y, width, cursorColour, cursorColour))
        return {};
    kNudge.set(app, 0);
    return int32_t(u32(place) - u32(x));
}

/* 0x4515e0's: a field's text at x, y of the menus' screen with its cursor
 * before character `cursor` (`width` wide, where `show`): by the 3D card's
 * font at its place on the screen's own over a race -- the fonts drawn at
 * once where the menus ask --, by the 2D screens' otherwise. */
Result fieldText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, int32_t cursor, int32_t width,
                 u32 show)
{
    const u32 at = esp - 0xc;
    if (kOverRace(app) != 0)
    {
        u32 sx, sy;
        if (!plainAt<0x451400, text::screenX>(app, ts, at, &sx, u32(x))
            || !plainAt<0x451440, text::screenY>(app, ts, at, &sy, u32(y)))
            return {};
        const Result r = cursorText3d(app, ts, at - 8 - 4, text, int32_t(sx), int32_t(sy), cursor, width, show);
        if (!r)
            return {};
        if ((kMenuBits1(app) & 0x20) == 0)
            return r;
        u32 f;
        if (!coreAt<0x4d1340, text::fontsDrawn, 0>(app, ts, at, &f))
            return {};
        return int32_t(f);
    }
    return cursorText(app, ts, at - 0xc - 4, text, x, y, 0, cursor, width, show);
}

/* 0x452220's: a field's line `text` in font `font` at x, y -- its right
 * there with alignment 1 or 4, its middle with 2 or 5; past 2 with a shadow
 * in colour 8 first -- in `colour`, its cursor before character `cursor`,
 * `width` wide and in `cursorColour`, where `show`. */
Result fieldLine(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, u32 font, int32_t align,
                 u32 colour, int32_t cursor, int32_t width, u32 cursorColour, u32 show)
{
    const u32 at = esp - 0xc - 4;
    u32 r;
    if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, font, font))
        return {};
    if (align == 1 || align == 4 || align == 2 || align == 5)
    {
        if (!namedAt<text::menuTextWidth, 0>(app, ts, at, &r, text, font))
            return {};
        x = int32_t(u32(x) - u32(align == 1 || align == 4 ? int32_t(r) : halved(int32_t(r))));
    }
    if (align > 2)
    {
        if (!keepAt<0x4513c0, text::textColourSet>(app, ts, at, &r, 8, 8)
            || !coreAt<0x451590, text::textDrawnAt, 0>(app, ts, at, &r, text, u32(x) + 1, u32(y) + 1))
            return {};
    }
    cursorColourSet(app, cursorColour);
    if (!keepAt<0x4513c0, text::textColourSet>(app, ts, at, &r, colour, colour))
        return {};
    return fieldText(app, ts, at - 8 - 4, text, x, y, cursor, width, show);
}

/* 0x4522c0's: text `text` in font `font` a character a line, down from y
 * (aligned by `align` as a menu text), each line its character's middle at x:
 * in `colour`, past 2 with a shadow.  How many characters. */
Result spacedText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, int32_t font, u32 edx)
{
    const u32 ebp = esp - 0xc, at = ebp - 0x20, character = ebp - 0x10;
    setWord(app, ebp - 0x18, text);
    setHalf(app, ebp - 0xc, x);
    setHalf(app, ebp - 8, font);
    const int32_t font16 = half(app, ebp - 8);
    u32 r;
    if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, u32(font16), u32(font16))
        || !plainAt<kFontHeight, frontend::wata::textDepth>(app, ts, at, &r))
        return {};
    const int32_t spacing = chopped<int32_t>(ts, fp::mul(Float(nfs::s32(r)), Float(kSpacing(app))));
    setWord(app, ebp - 0x14, r);
    setWord(app, ebp - 0x1c, u32(spacing));
    const int16_t align16 = half(app, esp + 4);
    setHalf(app, ebp - 4, int32_t(length(app, text)));
    if (align16 == 1 || align16 == 4)
        y = int32_t(u32(y) - u32(spacing * int32_t(word(app, ebp - 4))));
    else if (align16 == 2 || align16 == 5)
        y = int32_t(int16_t(y)) - halved(int32_t(half(app, ebp - 4)) * spacing);
    for (int16_t i = 0; int32_t(i) < int32_t(half(app, ebp - 4)); ++i)
    {
        if (!callCore<kStrncpy, watcom::clib::strncpy>(app, ts, at, r,
                                                       character, word(app, ebp - 0x18), u32(1)))
            return {};
        // The font as the code takes it, before the character is written.
        const u32 fontNow = u32(int32_t(half(app, ebp - 8)));
        if (!format(app, ts, at, {character, kCharSpace, byteAt(app, word(app, ebp - 0x18) + u32(int32_t(i)))})
            || !namedAt<text::menuTextWidth, 0>(app, ts, at, &r, character, fontNow))
            return {};
        const int32_t left = int32_t(half(app, ebp - 0xc)) - halved(int32_t(r));
        setWord(app, ebp - 0x20, u32(left));
        if (half(app, esp + 4) > 2)
        {
            const int32_t down = spacing * int32_t(i);
            if (!keepAt<0x4513c0, text::textColourSet>(app, ts, at, &r, 8, 8)
                || !coreAt<0x451590, text::textDrawnAt, 0>(app, ts, at, &r, character, u32(left) + 1, u32(int32_t(int16_t(y)) + down) + 1))
                return {};
        }
        if (!keepAt<0x4513c0, text::textColourSet>(app, ts, at, &r, word(app, esp + 8), word(app, esp + 8))
            || !coreAt<0x451590, text::textDrawnAt, 0>(app, ts, at, &r, character, u32(left), u32(int32_t(int16_t(y)) + int32_t(i) * spacing)))
            return {};
    }
    return half(app, ebp - 4);
}

}  // namespace

/* 0x4513e0: the cursor's colour, eax. */
nfs::u32 setCursorColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    cursorColourSet(app, e.eax);
    return e.back(ts, args, e.eax);
}

/* 0x4515e0 (stdcall 8): text eax at edx, ebx, the cursor before character
 * ecx, its width and whether it shows the stack's words. */
nfs::u32 drawFieldText(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             fieldText(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                       int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x452220 (stdcall 0x18): text eax in font ecx at edx, ebx; the stack's its
 * alignment, colour, cursor, the cursor's width, its colour, whether it
 * shows. */
nfs::u32 drawFieldLine(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             fieldLine(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, int32_t(e.arg(app, 0)),
                       e.arg(app, 1), int32_t(e.arg(app, 2)), int32_t(e.arg(app, 3)), e.arg(app, 4), e.arg(app, 5)),
             0x18);
}

/* 0x4522c0 (stdcall 8): text eax in font cx a character a line at dx, ebx;
 * its alignment (a word) and colour the stack's. */
nfs::u32 drawSpacedText(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pushed(app, e.esp, {e.esi, e.edi, e.ebp});
    const Result r = spacedText(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.edx);
    const u32 left = e.finish(ts, args, r, 8);
    if (r)
        popped(app, e.esp, {&args.esi, &args.edi, &args.ebp});
    return left;
}

/* 0x4d11f0 (stdcall 8): text eax in the 3D card's font at edx, ebx, the
 * cursor before character ecx where the stack's second word, of the first. */
nfs::u32 drawCursorText3d(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             cursorText3d(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                          int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x4da560 (stdcall 4): the 2D screens' cursor at eax, edx, ebx wide, in ecx
 * and the stack's colour. */
nfs::u32 drawCursor(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, cursorDrawn(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx, e.arg(app, 0)),
             4);
}

/* 0x4da8c0 (stdcall 0xc): text eax on a 2D screen at edx, ebx (ecx: only
 * measured), the cursor before the stack's character, its width, whether
 * it shows. */
nfs::u32 drawCursorText(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             cursorText(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, int32_t(e.arg(app, 0)),
                        int32_t(e.arg(app, 1)), e.arg(app, 2)),
             0xc);
}

/* ======== The car's colours (carclrselect) ======== */

namespace
{

ColourList& colourList(App* app, u32 item)
{
    return record<ColourList>(app, item);
}

Result listLaidOut(App* app, ThreadState& ts, u32 esp, u32 item);

/* 0x4569a0's: the list's width: its widest colour's name, or "custom...",
 * and the swatch's.  The last width measured. */
Result listWidth(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x100, text = ebp - 0x100;
    u32 r;
    if (!lineAt(app, ts, at, &r, 0x2b, edx) || !format(app, ts, at, {text, kEllipsis, r})
        || !namedAt<text::menuTextWidth, 0>(app, ts, at, &r, text, 0x12))
        return {};
    colourList(app, item).boxWidth = int16_t(r);
    for (int32_t i = 0; i < colourList(app, item).count; ++i)
    {
        const Result name = colourName(app, ts, at - 4, word(app, colourList(app, item).car), i);
        if (!name || !namedAt<text::menuTextWidth, 0>(app, ts, at, &r, u32(*name), 0x12))
            return {};
        if (int32_t(r) > colourList(app, item).boxWidth)
            colourList(app, item).boxWidth = int16_t(r);
    }
    colourList(app, item).boxWidth = int16_t(colourList(app, item).boxWidth + 0x2d);
    return int32_t(r);
}

}  // namespace

/* 0x456a20's: the list's player's car, its colours, the chosen one's. */
Result listRefreshed(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x18, at = ebp - 0x14;
    setByte(app, ebp - 8, 0xff);
    if (item == 0)
        return 0;
    const int32_t player = colourList(app, item).player != 0 ? 1 : 0;
    const u32 car = player != 0 ? SETTINGS_ADDRESS(players[1].car) : SETTINGS_ADDRESS(players[0].car);
    u32 rec;
    if (!carRecordOf(app, ts, at, gameSettings(app).players[player].car, rec))
        return {};
    if (rec == 0)
        return 0;
    colourList(app, item).car = car;
    colourList(app, item).shownCar = int32_t(gameSettings(app).players[player].car);
    const Result chosen = chosenColour(app, ts, at - 4, gameSettings(app).players[player].car, player);
    if (!chosen)
        return {};
    colourList(app, item).chosen = *chosen;
    colourList(app, item).highlighted = *chosen;
    if (!colourRgb(app, ts, at - 0xc - 4, gameSettings(app).players[player].car, *chosen, player, ebp - 0x14, ebp - 4, ebp - 0x10,
                   ebp - 0xc))
        return {};
    const Result count = colourCount(app, ts, at - 4, word(app, colourList(app, item).car));
    if (!count)
        return {};
    colourList(app, item).count = *count;
    colourList(app, item).colour = u32(byteAt(app, ebp - 8)) << 24 | u32(byteAt(app, ebp - 0x14)) << 16
                                   | u32(byteAt(app, ebp - 4)) << 8 | byteAt(app, ebp - 0x10);
    return byteAt(app, ebp - 0x10);
}

namespace
{

/* 0x4d8aa0's: a frame around the rectangle in `colour`. */
Result framed(App* app, ThreadState& ts, u32 esp, u32 x, u32 y, u32 width, u32 height, u32 colour)
{
    u32 r;
    if (!namedAt<draw2d::outline, 1>(app, ts, esp - 8, &r, x, y, width, height, colour))
        return {};
    return int32_t(r);
}

/* 0x456b50's: colour `colour`'s line of the list at x, y -- its swatch first
 * where `swatch` (framed lit where `lit`) --: its name, "custom..." for the
 * custom one. */
Result colourLine(App* app, ThreadState& ts, u32 esp, u32 item, int32_t colour, int32_t x, int32_t y, u32 lit,
                  u32 swatch)
{
    const u32 ebp = esp - 0xc, at = ebp - 0x11c, text = ebp - 0x11c;
    u32 r, textColour;
    if (swatch != 0)
    {
        if (!colourRgb(app, ts, at - 0xc - 4, word(app, colourList(app, item).car), colour,
                       colourList(app, item).player, ebp - 0xc, ebp - 8, ebp - 4, ebp - 0x10))
            return {};
        const u32 argb = 0xff000000 | u32(byteAt(app, ebp - 0xc)) << 16 | u32(byteAt(app, ebp - 8)) << 8
                           | byteAt(app, ebp - 4);
        if (!namedAt<draw2d::filled, 1>(app, ts, at, &r, u32(x), u32(y) + 1, 0x10, 0xd, argb))
            return {};
        const Result f = framed(app, ts, at - 4 - 4, u32(x), u32(y) + 1, 0x10, 0xc, lit != 0 ? 0xffffe440 : 0xff7b8ab5);
        if (!f)
            return {};
        r = u32(*f);
        x += 0x13;
        textColour = lit != 0 ? 0xffe440 : 0xff649dfd;
    }
    else
    {
        if (!callCore<kColourSetState, loop::keysTakenOf>(app, ts, at, r,
                                                          at - 4, item))
            return {};
        textColour = r != 0 ? 0x4a96 : 0xffe440;
    }
    if ((kMenuBits3(app) & 0x40) != 0)
        return int32_t(r);
    const u32 car = word(app, colourList(app, item).car);
    const Result custom = customColour(app, ts, at - 4, car);
    if (!custom)
        return {};
    u32 name;
    if (*custom == colour)
    {
        if (swatch != 0)
        {
            if (!lineAt(app, ts, at, &name, 0x2b, textColour)
                || !format(app, ts, at, {text, kEllipsis, name}))
                return {};
            name = text;
        }
        else if (!lineAt(app, ts, at - 8, &name, 0x2b, u32(x)))
            return {};
    }
    else
    {
        const Result n = colourName(app, ts, at - 8 - 4, car, colour);
        if (!n)
            return {};
        name = u32(*n);
    }
    if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, name, u32(x), u32(y), 0x12, 0, textColour))
        return {};
    return int32_t(r);
}

/* 0x456ce0's: the car's colours: the chosen one's line beside the arrow,
 * shut; open, the colours' box -- the one under the pointer lit where `hot`
 * --, the arrow and its box, the label. */
Result listDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    const u32 ebp = esp - 0x14, at = ebp - 8;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    if (colourList(app, item).car == 0)
        return 0;
    if (colourList(app, item).open != 0)
        colourList(app, item).state |= 4;
    else
        colourList(app, item).state &= 0xfb;
    if (colourList(app, item).shownCar != int32_t(word(app, colourList(app, item).car))
        && !listRefreshed(app, ts, at - 4, item))
        return {};
    if (hot != 0)
    {
        if (colourList(app, item).open != 0)
        {
            if (!callCore<kPointerInBox, frontend::glue::pointerInBox>(
                    app, ts, at, r, item + 0x1a))
                return {};
            if (r != 0)
            {
                if (!callCore<0x49c1c0, frontend::menud::pointerListLine>(
                        app, ts, at, r,
                        int32_t(colourList(app, item).boxY), 0x10))
                    return {};
                const int32_t line = int32_t(r);
                if (line >= 0 && line < colourList(app, item).count && line != colourList(app, item).highlighted)
                    colourList(app, item).highlighted = line;
            }
            const ColourList& l = colourList(app, item);
            if (!boxedNamedAt<draw2d::drawFieldBoxOf>(app, ts, at, &r, u32(int32_t(l.boxX)), u32(int32_t(l.boxY)), u32(int32_t(l.boxWidth)), u32(int32_t(l.boxHeight)), 0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222))
                return {};
            int16_t lineY = int16_t(colourList(app, item).boxY + 5);
            const int16_t lineX = int16_t(colourList(app, item).boxX + 0xf);
            for (int32_t i = 0; i < colourList(app, item).count; ++i)
            {
                if (!colourLine(app, ts, at - 8 - 4, item, i, lineX, lineY,
                                i == colourList(app, item).highlighted ? 1 : 0, 1))
                    return {};
                lineY = int16_t(lineY + 0x10);
            }
            const int32_t x = colourList(app, item).x, y = colourList(app, item).y;
            if (!coreAt<0x4d7990, shapes::drawShapeOf, 0>(app, ts, at, &r, u32(int32_t(colourList(app, item).arrow)), u32(x), u32(y + 0xd))
                || !coreAt<0x4d7990, shapes::drawShapeOf, 0>(app, ts, at, &r, u32(int32_t(colourList(app, item).box)), u32(x + 0xc), u32(y))
                || !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, colourList(app, item).label, u32(int32_t(colourList(app, item).boxX) + 0xf), u32(y - 1), 0x12, 0, 0xffe440))
                return {};
        }
        else
        {
            const int32_t x = colourList(app, item).x, y = colourList(app, item).y;
            if (!coreAt<0x4d7990, shapes::drawShapeOf, 0>(app, ts, at, &r, u32(int32_t(colourList(app, item).arrow)), u32(x), u32(y + 0xd)))
                return {};
        }
    }
    if (colourList(app, item).open == 0 && kScreensSliding(app) == 0
        && !colourLine(app, ts, at - 8 - 4, item, colourList(app, item).chosen,
                       int32_t(colourList(app, item).x) + 0x19, colourList(app, item).y, 0, 0))
        return {};
    return 0;
}

/* 0x456ea0's: the list shown: shut, its shapes found, its colours taken. */
Result listShown(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    const u32 at = esp - 0xc;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    colourList(app, item).open = 0;
    if (!namedAt<shapes::shapeNamed, 0>(app, ts, at, &r, kArrowName))
        return {};
    colourList(app, item).arrow = int16_t(r);
    if (!namedAt<shapes::shapeNamed, 0>(app, ts, at, &r, kBoxName))
        return {};
    ColourList& l = colourList(app, item);
    l.chosen = 0;
    l.count = 0;
    l.car = 0;
    l.colour = 0;
    l.box = int16_t(r);
    l.highlighted = 0;
    if (!listRefreshed(app, ts, at - 4, item))
        return {};
    return listWidth(app, ts, at - 4, item, 0x12);
}

/* 0x456f10's: the pointer put on the lit line. */
Result pointedAtColour(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const ColourList& l = colourList(app, item);
    const u32 y = (u32(l.highlighted) << 4) + u32(int32_t(l.boxY)) + 0x10;
    const u32 x = u32(halved(l.boxWidth) + l.boxX);
    u32 r;
    if (!callCore<0x49bfc0, frontend::screen::setPointer>(app, ts, esp - 0x10, r, x, y))
        return {};
    return int32_t(r);
}

/* 0x456f50's, 0x456f70's: the list opened (the chosen one lit) or shut. */
Result listOpened(App* app, ThreadState& ts, u32 esp, u32 item, int16_t open)
{
    colourList(app, item).open = open;
    colourList(app, item).highlighted = colourList(app, item).chosen;
    return listLaidOut(app, ts, esp - 8 - 4, item);
}

/* 0x456f90's: the lit colour chosen: set for the car and saved, the list
 * shut; its player's 3D car on the screen told to change. */
Result colourChosen(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 at = esp - 0x18;
    ColourList& l = colourList(app, item);
    const bool changed = l.chosen != l.highlighted;
    l.chosen = l.highlighted;
    l.open = 0;
    if (!colourSet(app, ts, at - 4, word(app, l.car), l.player, l.highlighted))
        return {};
    Result r = listLaidOut(app, ts, at - 4, item);
    if (!r || !changed)
        return r;
    u32 eax = u32(*r), menu;
    if (colourList(app, item).player == 0)
    {
        if (!callCore<kMenuNamed, loop::currentMenuOf>(app, ts, at, menu)
            || !plainAt<kFindItem, frontend::menus::findItem>(app, ts, at, &eax, menu, kFirstCarItem))
            return {};
        if (eax != 0)
            setByte(app, eax + 0xb3, 1);
    }
    if (colourList(app, item).player == 1)
    {
        if (!callCore<kMenuNamed, loop::currentMenuOf>(app, ts, at, menu)
            || !plainAt<kFindItem, frontend::menus::findItem>(app, ts, at, &eax, menu, kSecondCarItem))
            return {};
        if (eax != 0)
            setByte(app, eax + 0xb3, 1);
    }
    return int32_t(eax);
}

/* 0x457030's: a key on the car's colours: Enter or right opens them; open,
 * up and down light the line before or after, Enter or left chooses the line
 * under the pointer (the custom one opening the colour wheel), Escape -- or
 * Enter off the box -- shuts them.  2 for a key taken. */
Result listKey(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    const u32 at = esp - 0x10;
    const uint16_t key = uint16_t(ebx);
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    if (colourList(app, item).car == 0)
        return 0;
    const auto click = [&] { return coreAt<0x4181d0, playMenuEffectOf, 0>(app, ts, at, &r, 1, 0x7f); };
    const auto shut = [&]() -> Result {
        if (!click() || !listOpened(app, ts, at - 4, item, 0))
            return {};
        return 2;
    };
    if (colourList(app, item).open == 0)
    {
        if (key != kEnter && key != kRight)
            return 0;
        if (!listOpened(app, ts, at - 4, item, 1) || !click())
            return {};
        return 2;
    }
    if (key == kUp || key == kDown)
    {
        int32_t line = colourList(app, item).highlighted + (key == kUp ? -1 : 1);
        if (line < 0)
            line = 0;
        else if (line > colourList(app, item).count - 1)
            line = colourList(app, item).count - 1;
        colourList(app, item).highlighted = line;
        if (!pointedAtColour(app, ts, at - 4, item) || !click())
            return {};
        return 2;
    }
    if (key == kEscape)
        return shut();
    if (key != kEnter && key != kLeft)
        return 0;
    if (!callCore<kPointerInBox, frontend::glue::pointerInBox>(app, ts, at, r,
                                                               item + 0x1a))
        return {};
    if (r == 0)
        return shut();
    if (!click() || !colourChosen(app, ts, at - 4, item))
        return {};
    const Result custom = customColour(app, ts, at - 4, word(app, colourList(app, item).car));
    if (!custom)
        return {};
    if (*custom != colourList(app, item).highlighted)
        return 2;
    u32 menu, wheel;
    if (!callCore<kMenuNamed, loop::currentMenuOf>(app, ts, at, menu)
        || !plainAt<kFindItem, frontend::menus::findItem>(app, ts, at, &wheel, menu, kColourEdit))
        return {};
    if (wheel != 0 && !wheelGiven(app, ts, at - 4, wheel, colourList(app, item).player))
        return {};
    return 2;
}

/* 0x4571e0's: the box the pointer finds the list in: its colours' open, the
 * arrow's shut.  eax as the code leaves it, the box's top in its low half. */
Result listLaidOut(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 at = esp - 0xc;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    u32 e;
    if (colourList(app, item).open != 0)
    {
        const Result w = listWidth(app, ts, at - 4, item, 0x12);
        if (!w)
            return {};
        ColourList& l = colourList(app, item);
        e = ((u32(*w) & 0xffff0000) | uint16_t(l.count)) << 4;
        e += 5;
        l.boxHeight = int16_t(e);
        e = ((e & 0xffff0000) | uint16_t(l.x)) + 0x30;
        l.boxX = int16_t(e);
        e = ((e & 0xffff0000) | uint16_t(l.y)) + 0x11;
    }
    else
    {
        ColourList& l = colourList(app, item);
        l.boxHeight = 0x1b;
        e = (r & 0xffff0000) | uint16_t(l.x);
        l.boxWidth = 0x1e;
        e -= 0xa;
        l.boxX = int16_t(e);
        e = (e & 0xffff0000) | uint16_t(l.y);
    }
    colourList(app, item).boxY = int16_t(e);
    return int32_t(e);
}

}  // namespace

/* 0x4569a0: the colours' list eax's width. */
nfs::u32 colourListWidth(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listWidth(app, ts, e.esp, e.eax, e.edx));
}

/* 0x456a20: the colours' list eax's colours taken from its car. */
nfs::u32 colourListRefresh(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listRefreshed(app, ts, e.esp, e.eax));
}

/* 0x456b50 (stdcall 8): colour edx's line of the list eax at ebx, ecx; lit
 * and with its swatch the stack's words. */
nfs::u32 drawColourLine(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             colourLine(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0),
                        e.arg(app, 1)),
             8);
}

/* 0x456ce0: the colours' list eax drawn, the pointer's line lit where edx. */
nfs::u32 drawColourList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listDrawn(app, ts, e.esp, e.eax, e.edx));
}

/* 0x456ea0: the colours' list eax shown. */
nfs::u32 colourListShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listShown(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x456f10: the pointer on the list eax's lit line. */
nfs::u32 pointAtColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, pointedAtColour(app, ts, e.esp, e.eax));
}

/* 0x456f50, 0x456f70: the list eax opened, shut. */
nfs::u32 openColourList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listOpened(app, ts, e.esp, e.eax, 1));
}

nfs::u32 closeColourList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listOpened(app, ts, e.esp, e.eax, 0));
}

/* 0x456f90: the list eax's lit colour chosen. */
nfs::u32 chooseColour(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, colourChosen(app, ts, e.esp, e.eax));
}

/* 0x457030: key bx on the list eax. */
nfs::u32 colourListKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listKey(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x4571e0: the list eax laid out. */
nfs::u32 layoutColourList(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listLaidOut(app, ts, e.esp, e.eax));
}

/* 0x457250: the list hidden: nothing. */
nfs::u32 colourListHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, e.eax);
}

/* 0x4d8aa0 (stdcall 4): a frame around eax, edx, ebx wide, ecx high in the
 * stack's colour. */
nfs::u32 drawFrameAt(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, framed(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* ======== The fields typed into ======== */

namespace
{

/* 0x457ae0's: key `key` typed into the field's line `typed`, `size` bytes,
 * at the cursor (the word at `cursor`): a digit, or a dot (none where
 * `noDots`, one only), kept to the room a sign and its ending leave; a minus
 * first; Backspace or Delete clears it.  2 for a key taken, 0 else. */
Result keyTyped(App* app, ThreadState& ts, u32 esp, u32 typed, int32_t size, u32 cursor, u32 ecx, u32 noDots)
{
    const uint16_t key = uint16_t(ecx);
    const uint8_t c = uint8_t(ecx);
    const int32_t at0 = int32_t(word(app, cursor));
    if (at0 < 0 || size <= at0)
        setWord(app, cursor, 0);
    const bool digit = key >= '0' && key <= '9';
    if (key == kBackspace || key == kDelete)
    {
        setWord(app, cursor, 0);
        u32 r;
        if (!callCore<kMemset, watcom::clib::memset>(app, ts, esp - 0xc - 8, r, typed,
                                                     u32(0), u32(size)))
            return {};
        return 2;
    }
    if (key == '-')
    {
        if (word(app, cursor) != 0)
            return 2;
        setByte(app, typed, c);
        setWord(app, cursor, word(app, cursor) + 1);
        if (size - 1 <= int32_t(word(app, cursor)))
            setWord(app, cursor, u32(size - 2));
        return 2;
    }
    if (!digit && key != '.')
        return 0;
    if (key == '.' && noDots != 0)
        return 2;
    bool dot = false;
    for (int32_t i = 0; i < size; ++i)
        if (byteAt(app, typed + u32(i)) == '.')
            dot = true;
    if (key == '.' && dot)
        return 2;
    int32_t room = byteAt(app, typed) == '-' ? 2 : 3;
    if (key == '.' || dot)
        ++room;
    const int32_t limit = size - room;
    if (int32_t(word(app, cursor)) <= limit)
        setByte(app, typed + word(app, cursor), c);
    const int32_t next = int32_t(word(app, cursor) + 1);
    setWord(app, cursor, u32(next));
    if (limit < next)
        setWord(app, cursor, u32(limit));
    return 2;
}

/* What a field's draw shares: its label, the place its value goes. */
struct Labelled
{
    int32_t x, y;
    u32 colour, labelWidth;
};

/* The field drawn up to its value: the 0x100 bytes of its blank copied into
 * the frame's line, the font, the label (its width to [ebp-0x10], which the
 * code reads as it is without one).  False when the game stopped; `shown`
 * false (and eax 0) when the item is not on the screen. */
bool labelled(App* app, ThreadState& ts, u32 ebp, u32 item, u32 hot, u32 type, u32 blank, bool& shown,
              Labelled& l)
{
    const u32 at = ebp - 0x114;
    for (u32 i = 0; i < 0x100; i += 4)
        setWord(app, at + i, word(app, blank + i));
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return false;
    shown = r != 0;
    if (!shown)
        return true;
    const Item& it = record<Item>(app, item);
    l.colour = hot != 0 ? 0xffe440 : 0xff649dfd;
    l.x = it.x;
    l.y = it.y;
    if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, 0x12, 0x12))
        return false;
    const u32 label = word(app, item + 0x3c);
    if (label != 0)
    {
        if (!namedAt<text::menuTextWidth, 0>(app, ts, at, &r, label, 0x12))
            return false;
        setWord(app, ebp - 0x10, r);
        if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, label, u32(l.x), u32(l.y), 0x12, 0, l.colour))
            return false;
    }
    l.labelWidth = word(app, ebp - 0x10);
    return true;
}

/* A field's line being typed, its cursor at `cursor`. */
bool typedLine(App* app, ThreadState& ts, u32 ebp, u32 typed, const Labelled& l, int32_t cursor)
{
    const u32 at = ebp - 0x114, text = ebp - 0x114;
    return format(app, ts, at, {text, kS, typed})
           && fieldLine(app, ts, at - 0x18 - 4, text, int32_t(u32(l.x) + l.labelWidth), l.y, 0x12, 0, 0x800000,
                        cursor, 3, 0x800000, 1);
}

/* A field's value as the frame's line has it, drawn. */
bool valueLine(App* app, ThreadState& ts, u32 ebp, const Labelled& l)
{
    u32 r;
    return namedAt<text::menuTextDrawn, 2>(app, ts, ebp - 0x114, &r, ebp - 0x114, u32(l.x) + l.labelWidth, u32(l.y), 0x12, 0, l.colour);
}

/* A field's unit's name, its value's format's last word: 0 for none. */
u32 unitOf(uint8_t shown, u32 unit, u32 units)
{
    return shown == 1 && unit != 0 ? unit : shown == 2 && units != 0 ? units : 0;
}

/* 0x457c40's, 0x458480's: an edint's, an edchar's drawing. */
Result intDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    bool shown;
    Labelled l;
    if (!labelled(app, ts, ebp, item, hot, 0xa, kIntBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const IntField& f = record<IntField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, ts, ebp, item + 0x60, l, f.cursor) ? Result(0) : Result();
    const u32 value = word(app, f.value);
    const u32 unit = unitOf(f.unitsShown, f.unit, f.units);
    if (!(unit != 0 ? format(app, ts, at, {text, kDS, value, unit}) : format(app, ts, at, {text, kD, value}))
        || !valueLine(app, ts, ebp, l))
        return {};
    return 0;
}

Result charDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    bool shown;
    Labelled l;
    if (!labelled(app, ts, ebp, item, hot, 0xb, kCharBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const CharField& f = record<CharField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, ts, ebp, item + 0x58, l, f.cursor) ? Result(0) : Result();
    const u32 value = byteAt(app, f.value);
    const u32 unit = unitOf(f.unitsShown, f.unit, f.units);
    if (!(unit != 0 ? format(app, ts, at, {text, kDS, value, unit}) : format(app, ts, at, {text, kD, value}))
        || !valueLine(app, ts, ebp, l))
        return {};
    return 0;
}

/* 0x458940's, 0x458e30's: an edfloat's, an edfixed's: its value with its
 * decimals (1 to 6, else 6) as a double. */
Result floatDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot, bool fixed)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    bool shown;
    Labelled l;
    if (!labelled(app, ts, ebp, item, hot, fixed ? 0xd : 0xc, fixed ? kFixedBlank : kFloatBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const FloatField& f = record<FloatField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, ts, ebp, item + 0x64, l, f.cursor) ? Result(0) : Result();
    const u32 decimals = f.decimals > 0 && f.decimals <= 6 ? u32(f.decimals) : 6;
    const u32 unit = unitOf(f.unitsShown, f.unit, f.units);
    const Float value = fixed ? fp::mul(Float(nfs::s32(word(app, f.value))), Float(kFixedUnit(app)))
                              : Float(floatAt(app, f.value));
    const Words d = wordsOf(double(value));
    if (!(unit != 0 ? format(app, ts, at, {text, kFS, decimals, d.lo, d.hi, unit})
                    : format(app, ts, at, {text, kF, decimals, d.lo, d.hi}))
        || !valueLine(app, ts, ebp, l))
        return {};
    return 0;
}

/* The range messages: the dialog's text written, the question put. */
Result asked(App* app, ThreadState& ts, u32 at, std::initializer_list<u32> words)
{
    u32 r;
    if (!format(app, ts, at, words) || !question(app, ts, at, r))
        return {};
    return int32_t(r);
}

/* What the key handlers share: Enter starts typing (the line cleared, the
 * cursor at its start, the field held) where the field has a value. */
Result typingStarted(App* app, ThreadState& ts, u32 at, u32 item, u32 typed, u32 size, u32 cursor)
{
    setHalf(app, item + 0x40, 1);
    setWord(app, cursor, 0);
    u32 r;
    if (!callCore<kMemset, watcom::clib::memset>(app, ts, at, r, typed, u32(0), u32(size)))
        return {};
    record<Item>(app, item).state |= 4;
    return 2;
}

/* A field's function told of its change, eax the item. */
bool told(App* app, ThreadState& ts, u32 at, u32 function, u32 item, u32 edx, u32 ebx)
{
    u32 r;
    return function == 0 || gameCall(app, ts, at, function, {item, edx, ebx, item}, {}, r);
}

/* 0x457e30's: a key on an edint. */
Result intKey(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    const u32 ebp = esp - 0x10, at = ebp - 8;
    const uint16_t key = uint16_t(ebx);
    setWord(app, ebp - 8, 0);
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<IntField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<IntField>(app, item).value == 0)
            return 0;
        return typingStarted(app, ts, at, item, item + 0x60, 0xc, item + 0x5c);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<IntField>(app, item).editing = 0;
        record<IntField>(app, item).cursor = 0;
        u32 n;
        // By address: sscanf runs the runtime's own code below the words it pushes, which a direct
        // call would put 4 bytes lower than the game's call does.
        if (!plainAt<kSscanf, watcom::clib::sscanf>(app, ts, at, &n, item + 0x60, kD, guest::Words({ebp - 8})))
            return {};
        const IntField& f = record<IntField>(app, item);
        if (f.checked == 1)
        {
            const int32_t v = int32_t(word(app, ebp - 8));
            if (v < f.least || v > f.most)
            {
                if (!asked(app, ts, at - 4 - 0xc, {kDialogText, kIntRange, u32(f.least), u32(f.most)}))
                    return {};
                n = 0;
            }
        }
        if (n == 1)
        {
            setWord(app, record<IntField>(app, item).value, word(app, ebp - 8));
            if (!told(app, ts, at, record<IntField>(app, item).changed, item, n, item))
                return {};
        }
        record<IntField>(app, item).state &= 0xfb;
        return 2;
    }
    if (key == kEscape)
    {
        record<IntField>(app, item).editing = 0;
        record<IntField>(app, item).state &= 0xfb;
        record<IntField>(app, item).cursor = 0;
        return 2;
    }
    if (!keyTyped(app, ts, at - 4 - 4, item + 0x60, 0xc, item + 0x5c, u32(int32_t(int16_t(key))), 1))
        return {};
    return 2;
}

/* 0x458690's: a key on an edchar. */
Result charKey(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    const u32 ebp = esp - 0x10, at = ebp - 8;
    const uint16_t key = uint16_t(ebx);
    setByte(app, ebp - 4, 0);
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<CharField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<CharField>(app, item).value == 0)
            return 0;
        return typingStarted(app, ts, at, item, item + 0x58, 5, item + 0x54);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<CharField>(app, item).editing = 0;
        record<CharField>(app, item).cursor = 0;
        u32 n;
        // By address: sscanf runs the runtime's own code below the words it pushes, which a direct
        // call would put 4 bytes lower than the game's call does.
        if (!plainAt<kSscanf, watcom::clib::sscanf>(app, ts, at, &n, item + 0x58, kD, guest::Words({ebp - 4})))
            return {};
        const CharField& f = record<CharField>(app, item);
        if (f.checked == 1)
        {
            const uint8_t v = byteAt(app, ebp - 4);
            if (v < f.least || v > f.most)
            {
                if (!asked(app, ts, at - 4 - 0xc, {kDialogText, kIntRange, f.least, f.most}))
                    return {};
                n = 0;
            }
        }
        if (n == 1)
        {
            setByte(app, record<CharField>(app, item).value, byteAt(app, ebp - 4));
            if (!told(app, ts, at, record<CharField>(app, item).changed, item, n, item))
                return {};
        }
        record<CharField>(app, item).state &= 0xfb;
        return 2;
    }
    if (key == kEscape)
    {
        CharField& f = record<CharField>(app, item);
        f.editing = 0;
        f.cursor = 0;
        f.state &= 0xfb;
        if (!told(app, ts, at, f.changed, item, 0xb, item))
            return {};
        return 2;
    }
    if (!keyTyped(app, ts, at - 4 - 4, item + 0x58, 5, item + 0x54, u32(int32_t(int16_t(key))), 1))
        return {};
    return 2;
}

/* 0x458b80's, 0x459090's: a key on an edfloat, an edfixed. */
Result floatKey(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx, bool fixed)
{
    FpuStatus st;
    const u32 ebp = esp - 0x10, at = ebp - 8;
    const uint16_t key = uint16_t(ebx);
    setWord(app, ebp - 8, 0);
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<FloatField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<FloatField>(app, item).value == 0)
            return 0;
        return typingStarted(app, ts, at, item, item + 0x64, 0xf, item + 0x5c);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<FloatField>(app, item).editing = 0;
        record<FloatField>(app, item).cursor = 0;
        u32 n;
        // By address: sscanf runs the runtime's own code below the words it pushes, which a direct
        // call would put 4 bytes lower than the game's call does.
        if (!plainAt<kSscanf, watcom::clib::sscanf>(app, ts, at, &n, item + 0x64, kScanF, guest::Words({ebp - 8})))
            return {};
        const FloatField& f = record<FloatField>(app, item);
        const auto scaled = [&] {
            return fp::toInteger<int32_t>(fp::mul(Float(floatAt(app, ebp - 8)), Float(kFixedScale(app))));
        };
        if (f.checked == 1)
        {
            bool out;
            if (fixed)
                out = scaled() < int32_t(f.least) || scaled() > int32_t(f.most);
            else
            {
                st.set(Float(floatAt(app, ebp - 8)), Float(fromBits(f.least)));
                out = st.c0();
                if (!out)
                {
                    st.set(Float(floatAt(app, ebp - 8)), Float(fromBits(f.most)));
                    out = !st.c0() && !st.c3();
                }
            }
            if (out)
            {
                if (fixed)
                {
                    const Float unit = Float(kFixedUnit2(app));
                    const Words most = wordsOf(double(fp::mul(Float(nfs::s32(f.most)), unit)));
                    const Words least = wordsOf(double(fp::mul(unit, Float(nfs::s32(f.least)))));
                    if (!asked(app, ts, at - 4 - 0xc - 8,
                               {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}))
                        return {};
                }
                else
                {
                    const Words least = wordsOf(double(Float(fromBits(f.least))));
                    const Words most = wordsOf(double(Float(fromBits(f.most))));
                    if (!asked(app, ts, at - 8 - 4 - 0x10,
                               {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}))
                        return {};
                }
                n = 0;
            }
        }
        if (n == 1)
        {
            FloatField& g = record<FloatField>(app, item);
            setWord(app, g.value, fixed ? u32(scaled()) : word(app, ebp - 8));
            if (!told(app, ts, at, g.changed, item, item, g.value))
                return {};
        }
        record<FloatField>(app, item).state &= 0xfb;
        return 2;
    }
    if (key == kEscape)
    {
        record<FloatField>(app, item).editing = 0;
        record<FloatField>(app, item).state &= 0xfb;
        record<FloatField>(app, item).cursor = 0;
        return 2;
    }
    if (!keyTyped(app, ts, at - 4 - 4, item + 0x64, 0xf, item + 0x5c, u32(int32_t(int16_t(key))), 0))
        return {};
    return 2;
}

/* The fields' layout: the box at the field, 0x1d high and as wide as its
 * label and its value (`valueWidth`) and 10 between; esi's width without a
 * label, as the code leaves it. */
Result laidOut(App* app, ThreadState& ts, u32 at, u32 item, u32 type, u32 esi,
               const std::function<bool(u32&)>& valueWidth)
{
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    Item& it = record<Item>(app, item);
    it.boxHeight = 0x1d;
    it.boxX = it.x;
    it.boxY = it.y;
    u32 labelWidth = esi;
    const u32 label = word(app, item + 0x3c);
    if (label != 0 && !namedAt<text::menuTextWidth, 0>(app, ts, at, &labelWidth, label, 0x12))
        return {};
    u32 w;
    if (!valueWidth(w))
        return {};
    const u32 e = labelWidth + w + 0xa;
    record<Item>(app, item).boxWidth = int16_t(e);
    return int32_t(e);
}

/* A value's width: its typed line's while it is edited, else as `format`
 * writes it into the frame at `text`. */
bool valueWidth(App* app, ThreadState& ts, u32 at, u32 item, u32 typed, u32 text,
                std::initializer_list<u32> words, u32& w)
{
    if (record<IntField>(app, item).editing != 0)
        return namedAt<text::menuTextWidth, 0>(app, ts, at, &w, typed, 0x12);
    return format(app, ts, at, words) && namedAt<text::menuTextWidth, 0>(app, ts, at, &w, text, 0x12);
}

/* The fields' shown and hidden: typing stopped; shown, the function told
 * where the field has a value. */
Result fieldShown(App* app, ThreadState& ts, u32 at, u32 item, u32 type, u32 changed, u32 value, u32 ebx,
                  bool tell)
{
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const int16_t editing = half(app, item + 0x40);
    if (editing == 1)
        setHalf(app, item + 0x40, 0);
    if (!tell)
        return int32_t(r);
    const u32 function = word(app, item + changed);
    if (function == 0 || word(app, item + value) == 0)
        return int32_t(r);
    if (!gameCall(app, ts, at, function, {item, item, (ebx & 0xffff0000) | uint16_t(editing), item}, {}, r))
        return {};
    return int32_t(r);
}

/* The fields' bind: the value's place and the function told of a change. */
Result fieldBound(App* app, ThreadState& ts, u32 at, u32 item, u32 type, u32 value, u32 function,
                  u32 valueAt, u32 functionAt)
{
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    setWord(app, item + functionAt, function);
    setWord(app, item + valueAt, value);
    return 1;
}

/* 0x4580b0's: an edstring drawn: its label, its text -- being typed, the
 * cursor "_" blinking after it. */
Result stringDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot, u32 ebx, u32 ecx)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x14;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const u32 colour = hot != 0 ? 0xffe440 : 0xff649dfd;
    const int32_t y = record<StringField>(app, item).y;
    int32_t x = record<StringField>(app, item).x;
    if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, 0x12, 0x12))
        return {};
    const u32 label = record<StringField>(app, item).label;
    if (label != 0)
    {
        if (!namedAt<text::menuTextWidth, 0>(app, ts, at, &r, label, 0x12))
            return {};
        setWord(app, ebp - 0x14, r);
        if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, label, u32(x), u32(y), 0x12, 0, colour))
            return {};
    }
    x = int16_t(x + int32_t(word(app, ebp - 0x14)) + 0xa);
    const StringField& f = record<StringField>(app, item);
    if (f.target == 0)
        return 0;
    if (f.editing == 0)
    {
        if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, f.target, u32(x), u32(y), 0x12, 0, colour))
            return {};
        return 0;
    }
    if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, f.buffer, u32(x), u32(y), 0x12, 0, colour))
        return {};
    StringField& g = record<StringField>(app, item);
    g.blink = int16_t((int32_t(g.blink) + 1) % 0x1e);
    if (g.blink >= 0xf)
        return 0;
    if (!namedAt<text::menuTextWidth, 0>(app, ts, at - 8, &r, g.buffer, 0x12)
        || !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, kUnderline, u32(x) + r, u32(y), 0x12, 0, colour))
        return {};
    return 0;
}

/* 0x4581f0's: a key on an edstring: Enter copies the text in to type and
 * holds the field; typing, a character added (to its length), Backspace or
 * Delete takes the last off, Enter copies it out and tells the function,
 * Escape lets it go. */
Result stringKey(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    const u32 at = esp - 0x10;
    const uint16_t key = uint16_t(ebx);
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    StringField& f = record<StringField>(app, item);
    const uint16_t editing = uint16_t(f.editing);
    if (editing == 0)
    {
        if (key != kEnter || f.buffer == 0)
            return 0;
        f.editing = 1;
        if (f.target != 0)
        {
            if (!callCore<kStrncpy, watcom::clib::strncpy>(app, ts, at, r,
                                                           f.buffer, f.target, u32(int32_t(f.maxLength) - 1)))
                return {};
        }
        else
            setByte(app, f.buffer, uint8_t(ebx >> 8));
        StringField& g = record<StringField>(app, item);
        setByte(app, g.buffer + u32(int32_t(g.maxLength)) - 1, 0);
        g.state |= 4;
        return 2;
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        f.editing = 0;
        if (f.target != 0
            && !callCore<kStrncpy, watcom::clib::strncpy>(app, ts, at, r,
                                                          f.target, f.buffer, u32(int32_t(f.maxLength))))
            return {};
        if (!told(app, ts, at, record<StringField>(app, item).changed, item, 0, 0))
            return {};
        record<StringField>(app, item).state &= 0xfb;
        return 2;
    }
    if (key == kEscape)
    {
        f.editing = 0;
        f.state &= 0xfb;
        return 2;
    }
    const u32 n = length(app, f.buffer);
    if (key == kBackspace || key == kDelete)
    {
        if (n != 0)
            setByte(app, f.buffer + n - 1, 0);
        return 2;
    }
    if (int32_t(n) < int32_t(f.maxLength))
    {
        setByte(app, f.buffer + n, uint8_t(ebx));
        setByte(app, f.buffer + n + 1, 0);
    }
    return 2;
}

/* 0x4587e0's: an edchar laid out: its box after its label where the value
 * is checked. */
Result charLaidOut(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x14, at = ebp - 8;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    u32 off = 0;
    if (record<CharField>(app, item).checked != 0)
    {
        if (!namedAt<text::menuTextWidth, 0>(app, ts, at, &r, record<CharField>(app, item).label, 0x12))
            return {};
        off = r + 0xa;
    }
    CharField& f = record<CharField>(app, item);
    f.boxX = int16_t(f.x + int32_t(off));
    f.boxY = f.y;
    f.boxHeight = 0x1d;
    u32 labelWidth, w = 0;
    if (!namedAt<text::menuTextWidth, 0>(app, ts, at, &labelWidth, f.label, 0x12))
        return {};
    if (record<CharField>(app, item).value != 0
        && !valueWidth(app, ts, at, item, item + 0x58, ebp - 8,
                       {ebp - 8, kD, byteAt(app, record<CharField>(app, item).value)}, w))
        return {};
    const u32 e = labelWidth + w + 0xa;
    record<CharField>(app, item).boxWidth = int16_t(e);
    return int32_t(e);
}

}  // namespace

/* 0x457ae0 (stdcall 4): key cx typed into the line eax, edx long, at the
 * cursor [ebx]; dots refused where the stack's word. */
nfs::u32 editKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, keyTyped(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* 0x457c40, 0x458480, 0x458940, 0x458e30: an edint, edchar, edfloat, edfixed
 * eax drawn, lit where edx. */
nfs::u32 drawIntField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, intDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 drawCharField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, charDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 drawFloatField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, floatDrawn(app, ts, e.esp, e.eax, e.edx, false));
}

nfs::u32 drawFixedField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, floatDrawn(app, ts, e.esp, e.eax, e.edx, true));
}

/* 0x457df0, 0x458640: "Min Value %d, Max Value %d" of eax and edx (their low
 * bytes for an edchar) put up. */
nfs::u32 intRangeMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pushed(app, e.esp, {e.ebx, e.ecx, e.ebp});
    return e.finish(ts, args, asked(app, ts, e.esp - 0xc, {kDialogText, kIntRange, e.eax, e.edx}));
}

nfs::u32 charRangeMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pushed(app, e.esp, {e.ebx, e.ecx, e.ebp});
    return e.finish(ts, args, asked(app, ts, e.esp - 0xc, {kDialogText, kIntRange, e.eax & 0xff, e.edx & 0xff}));
}

/* 0x458b20 (stdcall 8): "Min Value %f, Max Value %f" of the stack's floats. */
nfs::u32 floatRangeMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Words least = wordsOf(double(Float(fromBits(e.arg(app, 0)))));
    const Words most = wordsOf(double(Float(fromBits(e.arg(app, 1)))));
    return e.finish(ts, args, asked(app, ts, e.esp - 0x10, {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}), 8);
}

/* 0x459020: of the 16.16 numbers eax and edx. */
nfs::u32 fixedRangeMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Float unit = Float(kFixedUnit2(app));
    const Words most = wordsOf(double(fp::mul(Float(nfs::s32(e.edx)), unit)));
    const Words least = wordsOf(double(fp::mul(unit, Float(nfs::s32(e.eax)))));
    return e.finish(ts, args, asked(app, ts, e.esp - 0xc - 8, {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}));
}

/* 0x457e30, 0x458690, 0x458b80, 0x459090: key bx on an edint, edchar, edfloat,
 * edfixed eax. */
nfs::u32 intFieldKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, intKey(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 charFieldKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, charKey(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 floatFieldKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, floatKey(app, ts, e.esp, e.eax, e.ebx, false));
}

nfs::u32 fixedFieldKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, floatKey(app, ts, e.esp, e.eax, e.ebx, true));
}

/* 0x457f70, 0x458340, 0x4587e0, 0x458ce0, 0x459210: the field eax laid out. */
nfs::u32 layoutIntField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 ebp = e.esp - 0x18, at = ebp - 0xc;
    const u32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    r = laidOut(app, ts, at, item, 0xa, e.esi, [&](u32& w) {
                 const IntField& f = record<IntField>(app, item);
                 w = 0;
                 return f.value == 0
                        || valueWidth(app, ts, at, item, item + 0x60, ebp - 0xc, {ebp - 0xc, kD, word(app, f.value)}, w);
             });
    }
    const u32 left = e.finish(ts, args, r);
    if (r)
        popped(app, e.esp, {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp});
    return left;
}

nfs::u32 layoutStringField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 at = e.esp - 0x14;
    const u32 item = e.eax;
    return e.finish(ts, args, laidOut(app, ts, at, item, 9, e.esi, [&](u32& w) {
                 const u32 target = record<StringField>(app, item).target;
                 w = 0;
                 return target == 0 || namedAt<text::menuTextWidth, 0>(app, ts, at, &w, target, 0x12);
             }));
    return args.eax;
}

nfs::u32 layoutCharField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.ebp});
    const Result r = charLaidOut(app, ts, e.esp, e.eax);
    const u32 left = e.finish(ts, args, r);
    if (r)
        popped(app, e.esp, {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.ebp});
    return left;
}

nfs::u32 layoutFloatField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 ebp = e.esp - 0x18, at = ebp - 0x14;
    const u32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    r = laidOut(app, ts, at, item, 0xc, e.esi, [&](u32& w) {
                 const FloatField& f = record<FloatField>(app, item);
                 w = 0;
                 if (f.value == 0)
                     return true;
                 const u32 whole = f.editing != 0 ? 0 : u32(chopped<int32_t>(ts, Float(floatAt(app, f.value))));
                 return valueWidth(app, ts, at, item, item + 0x64, ebp - 0x14, {ebp - 0x14, kF6, whole}, w);
             });
    }
    const u32 left = e.finish(ts, args, r);
    if (r)
        popped(app, e.esp, {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp});
    return left;
}

nfs::u32 layoutFixedField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 ebp = e.esp - 0x18, at = ebp - 0x10;
    const u32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    r = laidOut(app, ts, at, item, 0xd, e.esi, [&](u32& w) {
                 const FloatField& f = record<FloatField>(app, item);
                 w = 0;
                 return f.value == 0
                        || valueWidth(app, ts, at, item, item + 0x64, ebp - 0x10, {ebp - 0x10, kF6, word(app, f.value)},
                                      w);
             });
    }
    const u32 left = e.finish(ts, args, r);
    if (r)
        popped(app, e.esp, {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp});
    return left;
}

/* 0x458010, 0x458050 ...: the field eax shown (its function told), hidden. */
nfs::u32 intFieldShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0x10, e.eax, 0xa, 0x6c, 0x58, e.ebx, true));
}

nfs::u32 intFieldHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0xc, e.eax, 0xa, 0, 0, e.ebx, false));
}

nfs::u32 charFieldShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0x10, e.eax, 0xb, 0x60, 0x50, e.ebx, true));
}

nfs::u32 charFieldHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0xc, e.eax, 0xb, 0, 0, e.ebx, false));
}

nfs::u32 floatFieldShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0x10, e.eax, 0xc, 0x74, 0x58, e.ebx, true));
}

nfs::u32 floatFieldHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0xc, e.eax, 0xc, 0, 0, e.ebx, false));
}

nfs::u32 fixedFieldShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0x10, e.eax, 0xd, 0x74, 0x58, e.ebx, true));
}

nfs::u32 fixedFieldHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldShown(app, ts, e.esp - 0xc, e.eax, 0xd, 0, 0, e.ebx, false));
}

/* 0x458080, 0x458910, 0x458e00, 0x459320: the field eax's value at edx, its
 * function ebx. */
nfs::u32 bindIntField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldBound(app, ts, e.esp - 0xc, e.eax, 0xa, e.edx, e.ebx, 0x58, 0x6c));
}

nfs::u32 bindCharField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldBound(app, ts, e.esp - 0xc, e.eax, 0xb, e.edx, e.ebx, 0x50, 0x60));
}

nfs::u32 bindFloatField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldBound(app, ts, e.esp - 0xc, e.eax, 0xc, e.edx, e.ebx, 0x58, 0x74));
}

nfs::u32 bindFixedField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fieldBound(app, ts, e.esp - 0xc, e.eax, 0xd, e.edx, e.ebx, 0x58, 0x74));
}

/* 0x4580b0: an edstring eax drawn, lit where edx. */
nfs::u32 drawStringField(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stringDrawn(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

/* 0x4581f0: key bx on an edstring eax. */
nfs::u32 stringFieldKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stringKey(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x4583b0: an edstring eax shown: its buffer (`maxLength`, 11 for none, and
 * its ending) allocated. */
nfs::u32 stringFieldShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 at = e.esp - 0x14, item = e.eax;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return args.eax;
    if (r != 0)
    {
        StringField& f = record<StringField>(app, item);
        if (f.editing == 1)
            f.editing = 0;
        if (f.maxLength == 0)
            f.maxLength = 0xb;
        if (!callCore<kAllocate, ea::reserved>(app, ts, at, r,
                                               kNoName, u32(int32_t(f.maxLength) + 1), u32(0)))
            return args.eax;
        record<StringField>(app, item).buffer = r;
    }
    return e.back(ts, args, r);
}

/* 0x458410: an edstring eax hidden: its buffer freed. */
nfs::u32 stringFieldHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 at = e.esp - 0x10, item = e.eax;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return args.eax;
    if (r != 0)
    {
        StringField& f = record<StringField>(app, item);
        if (f.editing == 1)
            f.editing = 0;
        if (f.buffer != 0)
        {
            if (!callCore<kFree, ea::purgeBlock>(app, ts, at, r, f.buffer))
                return args.eax;
            record<StringField>(app, item).buffer = 0;
        }
    }
    return e.back(ts, args, r);
}

/* ======== The list of lines (List) ======== */

namespace
{

ListBox& listBox(App* app, u32 item)
{
    return record<ListBox>(app, item);
}

}  // namespace

/* 0x4d8570's: the triangle (x0, y0), (x1, y1), (x2, y2), a colour at each
 * corner, in the 2D screens' quad's first three corners -- over a race near
 * the eye, its corners put on the screen's own (x2's in its stack word, at
 * `x2At`) --, clipped and drawn. */
Result triangle(App* app, ThreadState& ts, u32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, u32 x2At,
                int32_t y2, u32 c0, u32 c1, u32 c2)
{
    const u32 at = esp - 0xc - 0x14;
    u32 r;
    if (kOverRace(app) != 0)
    {
        for (u32 corner = 0; corner < 4; ++corner)
        {
            setWord(app, kQuad + 0x20 * corner + 8, kNearDepth);
            setWord(app, kQuad + 0x20 * corner + 0xc, kNearW);
        }
        u32 sx0, sx1, sx2, sy0, sy1, sy2;
        if (!plainAt<0x451400, text::screenX>(app, ts, at, &sx0, u32(x0))
            || !plainAt<0x451400, text::screenX>(app, ts, at, &sx1, u32(x1))
            || !plainAt<0x451400, text::screenX>(app, ts, at, &sx2, word(app, x2At)))
            return {};
        setWord(app, x2At, sx2);
        if (!plainAt<0x451440, text::screenY>(app, ts, at, &sy0, u32(y0))
            || !plainAt<0x451440, text::screenY>(app, ts, at, &sy1, u32(y1))
            || !plainAt<0x451440, text::screenY>(app, ts, at, &sy2, u32(y2)))
            return {};
        x0 = int32_t(sx0);
        x1 = int32_t(sx1);
        y0 = int32_t(sy0);
        y1 = int32_t(sy1);
        y2 = int32_t(sy2);
    }
    else
    {
        const float depth = kScreenDepth(app), w = kScreenW(app);
        setFloat(app, kQuad + 0xc, viaX87(w));
        setFloat(app, kQuad + 0x28, viaX87(depth));
        setFloat(app, kQuad + 0x2c, viaX87(w));
        setFloat(app, kQuad + 0x48, viaX87(depth));
        setFloat(app, kQuad + 0x4c, viaX87(w));
        setFloat(app, kQuad + 8, viaX87(depth));
    }
    setWord(app, kQuad + 0x10, c0);
    setFloat(app, kQuad + 0, ofInt(x0));
    setFloat(app, kQuad + 4, ofInt(y0));
    setWord(app, kQuad + 0x30, c1);
    setFloat(app, kQuad + 0x20, ofInt(x1));
    setWord(app, kQuad + 0x50, c2);
    setFloat(app, kQuad + 0x24, ofInt(y1));
    setFloat(app, kQuad + 0x44, ofInt(y2));
    setFloat(app, kQuad + 0x40, ofInt(int32_t(word(app, x2At))));
    if (!callCoreNoEax<kRenderState, graphics::sendState>(app, ts, at, u32(1),
                                                          u32(0)))
        return {};
    const u32 ecx = markCorners(app, 3);
    NFS2_USE(ecx);
    if (!coreAt<kClipTriangle, vertices::triangleClippedAt, 0>(app, ts, at, &r, kQuad, kQuad + 0x20, kQuad + 0x40))
        return {};
    return int32_t(r);
}

namespace
{

/* A call of the triangle as the list makes it: its five stack words below
 * `at`, x2 among them. */
Result listTriangle(App* app, ThreadState& ts, u32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2,
                    int32_t y2, u32 colour)
{
    const u32 words = at - 0x14;
    setWord(app, words, u32(x2));
    setWord(app, words + 4, u32(y2));
    for (u32 i = 2; i < 5; ++i)
        setWord(app, words + 4 * i, colour);
    return triangle(app, ts, words - 4, x0, y0, x1, y1, words, y2, colour, colour, colour);
}

/* 0x465500's: the list drawn: its rows' background, a line above and below,
 * its lines from the first shown (the chosen lit, a marked one in <> and
 * white); unless it shows one row, its scroll bar and arrows, bright where
 * there is more to scroll to. */
Result listBoxDrawn(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x14, at = ebp - 0x118, text = ebp - 0x118;
    u32 r;
    if (!plainAt<kTextExtent, frontend::wata::textMeasure>(app, ts, at, &r, kAg, ebp - 0x18, ebp - 0x18, ebp - 0x18,
                                                           0x678a40))
        return {};
    const auto bottom = [&] {
        return int32_t(u32(int32_t(listBox(app, item).y))
                       + u32(int32_t(listBox(app, item).rows)) * u32(kLineHeight(app)));
    };
    const auto right = [&] { return int32_t(listBox(app, item).x) + listBox(app, item).width; };
    const auto gradient = [&](int32_t a, int32_t b, int32_t c, int32_t d, u32 from, u32 to) {
        return namedAt<draw2d::gradient, 2>(app, ts, at, &r, u32(a), u32(b), u32(c), u32(d), from, to);
    };
    if (!gradient(listBox(app, item).x, listBox(app, item).y, right(), bottom(), 0xff0000aa, 0xff00aa00))
        return {};
    int16_t lineY = listBox(app, item).y;
    if (!gradient(listBox(app, item).x, lineY, right(), int32_t(lineY) + 1, 0xffaa0000, 0xffaa0000))
        return {};
    for (int16_t i = 0;; ++i)
    {
        const ListBox& l = listBox(app, item);
        if (i >= l.rows)
            break;
        const int32_t n = int32_t(i) + l.first;
        if (n > int32_t(l.count) - 1)
            break;
        const u32 line = word(app, l.lines + u32(n) * 8);
        const uint8_t mark = byteAt(app, line);
        setByte(app, line, uint8_t(mark & 0x7f));
        if (!format(app, ts, at, {text, (mark & 0x80) != 0 ? kMarked : kPlain, line}))
            return {};
        const u32 colour = int32_t(i) + listBox(app, item).first == listBox(app, item).chosen ? 0xffe440
                             : (mark & 0x80) != 0                                               ? 0xffffff
                                                                                                : 0x908080;
        if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, text, u32(int32_t(listBox(app, item).x)), u32(int32_t(lineY)), 0x12, 0, colour))
            return {};
        setByte(app, line, mark);
        lineY = int16_t(lineY + int16_t(kLineHeight(app)));
    }
    if (!gradient(listBox(app, item).x, bottom(), right(), bottom() + 1, 0xffaa0000, 0xffaa0000))
        return {};
    if (listBox(app, item).rows == 1)
        return 0;
    const ListBox& l = listBox(app, item);
    const bool more = l.first != 0 || int32_t(l.count) > int32_t(l.rows) + l.first;
    u32 colour = more ? 0xffaa00aa : 0xff505050;
    if (!gradient(right() - 0x1e, listBox(app, item).y + 1, right(), bottom(), colour, colour))
        return {};
    colour = more ? 0xffaa5000 : 0xff000000;
    if (!gradient(right() - 0x1e, listBox(app, item).y + 1, right(), listBox(app, item).y + 0x19, colour, colour)
        || !gradient(right() - 0x1e, bottom(), right(), bottom() - 0x19, colour, colour))
        return {};
    colour = more ? 0xffaaaaaa : 0xff505050;
    const int32_t y = listBox(app, item).y;
    if (!listTriangle(app, ts, at, right() - 0xf, y + 4, right() - 4, y + 0x15, right() - 6, y + 0x15, colour)
        || !listTriangle(app, ts, at, right() - 0xf, y + 4, right() - 0x18, y + 0x15, right() - 0x1a, y + 0x15,
                         colour)
        || !listTriangle(app, ts, at, right() - 0xf, bottom() - 4, right() - 4, bottom() - 0x15, right() - 6,
                         bottom() - 0x15, colour)
        || !listTriangle(app, ts, at, right() - 0xf, bottom() - 4, right() - 0x18, bottom() - 0x15, right() - 0x1a,
                         bottom() - 0x15, colour))
        return {};
    return 0;
}

/* 0x465880's: the pointer put at the chosen line's right end (left of the
 * scroll bar where there is one). */
Result pointedAtLine(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const ListBox& l = listBox(app, item);
    int32_t x = int32_t(l.boxWidth) + l.boxX - 2;
    const int32_t shown = int32_t(l.chosen) - l.first;
    const int32_t y = int32_t(u32(kLineHeight(app)) * u32(shown) + u32(int32_t(l.boxY)) + 2);
    kPointerY.set(app, y);
    if (l.rows > 1)
        x -= 0x1e;
    kPointerX.set(app, x);
    u32 r;
    if (!callCore<0x49bfc0, frontend::screen::setPointer>(
            app, ts, esp - 0x14, r,
            u32(x), u32(kPointerY(app))))
        return {};
    return int32_t(r);
}

/* 0x4658f0's, 0x465950's: the line after or before chosen, the lines
 * scrolled to it, the pointer put on it unless `quiet`; nothing while the
 * list takes no keys. */
Result listMoved(App* app, ThreadState& ts, u32 esp, u32 item, u32 quiet, bool down)
{
    ListBox& l = listBox(app, item);
    if ((l.flags & 4) != 0)
        return int32_t(item);
    if (down)
    {
        l.chosen = int16_t(l.chosen + 1);
        if (l.chosen > int32_t(l.count) - 1)
            l.chosen = int16_t(l.count - 1);
        if (l.chosen > int32_t(l.rows) + l.first - 1)
            l.first = int16_t(l.first + 1);
    }
    else
    {
        l.chosen = int16_t(l.chosen - 1);
        if (l.chosen < 0)
            l.chosen = 0;
        if (l.chosen < l.first)
            l.first = l.chosen;
    }
    if (quiet != 0)
        return int32_t(item);
    return pointedAtLine(app, ts, esp - 8 - 4, item);
}

/* 0x4659a0's: the part of the list at x, y: 0 its lines (or a list of one
 * row), the scroll bar's 1 up arrow, 2 middle, 3 down arrow. */
int32_t partAt(App* app, u32 item, int32_t x, int32_t y)
{
    const ListBox& l = listBox(app, item);
    if (l.rows == 1)
        return 0;
    if (x < int32_t(l.boxWidth) + l.boxX - 0x1e)
        return 0;
    if (y < int32_t(l.boxY) + 0x19)
        return 1;
    if (y < int32_t(l.boxY) + l.boxHeight - 0x19)
        return 2;
    return 3;
}

/* 0x465ca0's: the line at y chosen, the list's function told (eax the item,
 * edx the caller's, ebx the line). */
Result chosenAt(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx, int32_t x, int32_t y)
{
    const u32 ebp = esp - 0xc, at = ebp - 4;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    ListBox& l = listBox(app, item);
    const int32_t line = int32_t(int64_t(int32_t(u32(y) - u32(int32_t(l.boxY)))) / kLineHeight(app));
    l.chosen = int16_t(line + l.first);
    if (l.choose == 0)
        return 0;
    if (!gameCall(app, ts, at, l.choose, {item, edx, u32(int32_t(l.chosen)), 0}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x465a30's: a key on the list: up, down; Enter chooses the line under the
 * pointer -- an arrow of the scroll bar steps instead, quietly --, the one
 * row of a list of one.  2 for a key taken. */
Result listBoxKeyed(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx, u32 ebx)
{
    const u32 ebp = esp - 0x10, at = ebp - 4;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    const uint16_t key = uint16_t(ebx);
    if (key == kUp || key == kDown)
    {
        if (!listMoved(app, ts, at - 4, item, 0, key == kDown))
            return {};
        return 2;
    }
    if (key != kEnter)
        return 0;
    const ListBox& l = listBox(app, item);
    if (l.rows == 1)
    {
        if (!chosenAt(app, ts, at - 4 - 4, item, edx, int32_t(l.x) + l.width - 2, l.y))
            return {};
        return 2;
    }
    if (partAt(app, item, kPointerX(app), kPointerY(app)) == 0)
    {
        if (!chosenAt(app, ts, at - 4 - 4, item, edx, kPointerX(app), kPointerY(app)))
            return {};
        return 2;
    }
    if (listBox(app, item).rows >= listBox(app, item).count)
        return 2;
    const int32_t part = partAt(app, item, kPointerX(app), kPointerY(app));
    if ((part == 1 || part == 3) && !listMoved(app, ts, at - 4, item, 1, part == 3))
        return {};
    return 2;
}

/* 0x465ba0's: the list laid out: the line's height measured, its box at it,
 * its width and rows high.  eax the product as the code makes it. */
Result listBoxLaidOut(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x14, at = ebp - 4;
    u32 r;
    if (!plainAt<kTextExtent, frontend::wata::textMeasure>(app, ts, at, &r, kAg, ebp - 4, ebp - 4, ebp - 4, 0x678a40)
        || !shownNow(app, ts, at, item, r))
        return {};
    if (r == 0)
        return 0;
    ListBox& l = listBox(app, item);
    l.boxX = l.x;
    l.boxY = l.y;
    l.boxWidth = l.width;
    const u32 e = ((r & 0xffff0000) | uint16_t(l.rows)) * u32(uint16_t(kLineHeight(app)));
    l.boxHeight = int16_t(e);
    return int32_t(e);
}

}  // namespace

/* 0x465500: the list eax drawn. */
nfs::u32 drawListBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listBoxDrawn(app, ts, e.esp, e.eax));
}

/* 0x465880: the pointer on the list eax's chosen line. */
nfs::u32 pointAtListBoxLine(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, pointedAtLine(app, ts, e.esp, e.eax));
}

/* 0x4658f0, 0x465950: the list eax's next, previous line; quietly where ebx. */
nfs::u32 listBoxDown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listMoved(app, ts, e.esp, e.eax, e.ebx, true));
}

nfs::u32 listBoxUp(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listMoved(app, ts, e.esp, e.eax, e.ebx, false));
}

/* 0x4659a0: the part of the list eax at edx, ebx. */
nfs::u32 listBoxPartAt(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(partAt(app, e.eax, int32_t(e.edx), int32_t(e.ebx))));
}

/* 0x465a10: whether edx, ebx is on the list eax's scroll bar. */
nfs::u32 listBoxArrowAt(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, partAt(app, e.eax, int32_t(e.edx), int32_t(e.ebx)) != 0 ? 1 : 0);
}

/* 0x465a30: key bx on the list eax (edx handed to its function). */
nfs::u32 listBoxKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listBoxKeyed(app, ts, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x465ba0: the list eax laid out. */
nfs::u32 layoutListBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, listBoxLaidOut(app, ts, e.esp, e.eax));
}

/* 0x465c10: the list eax shown: from its first line, 3 rows and 0x118 wide
 * for none. */
nfs::u32 listBoxShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 item = e.eax, at = e.esp - 0x10;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return args.eax;
    if (r != 0)
    {
        ListBox& l = listBox(app, item);
        l.first = 0;
        l.chosen = 0;
        if (l.rows == 0)
            l.rows = 3;
        if (l.width == 0)
            l.width = 0x118;
    }
    return e.back(ts, args, r);
}

/* 0x465c60: the list eax hidden: its lines freed. */
nfs::u32 listBoxHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 item = e.eax, at = e.esp - 0x10;
    u32 r;
    if (!shownNow(app, ts, at, item, r))
        return args.eax;
    if (r != 0)
    {
        const u32 lines = listBox(app, item).lines;
        if (lines != 0 && !callCore<kFree, ea::purgeBlock>(app, ts, at, r, lines))
            return args.eax;
        listBox(app, item).lines = 0;
    }
    return e.back(ts, args, r);
}

/* 0x465ca0 (stdcall 4): the list eax's line at ecx chosen, its function
 * handed edx. */
nfs::u32 listBoxChooseAt(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, chosenAt(app, ts, e.esp, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx)), 4);
}

/* 0x4d8570 (stdcall 0x14): the triangle eax, edx; ebx, ecx; the stack's
 * corner and its three colours. */
nfs::u32 drawShadedTriangle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             triangle(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.esp + 4,
                      int32_t(e.arg(app, 1)), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4)),
             0x14);
}

/* ======== The sliders ======== */

namespace
{

#pragma pack(push, 1)
/* A slider's part: the rectangle it is drawn in. */
struct SliderPart
{
    int16_t y, x, width, height;
};

/* slider: a value of 0 to 1 dragged along a track (kind 0, its parts the
 * thumb, the track, the arrows) or a two-axis one (kind 2: four thumbs across
 * and up an area, a bar's thumb, the area); kind 1 none. */
struct Slider : Item
{
    uint8_t percent;      // +0x3c: 1 its value shown as a percentage
    uint8_t ownPlace;     // +0x3d: placed by its row of the menus' table
    uint8_t alphaOn;      // +0x3e
    uint8_t alpha;        // +0x3f
    int16_t row;          // +0x40
    int16_t kind;         // +0x42
    uint32_t step;        // +0x44: a float's bits
    uint32_t arrowSteps[3];  // +0x48
    uint32_t values[3];   // +0x54
    u32 title;          // +0x60
    int16_t titleX, titleY;  // +0x64
    u32 labels[3];      // +0x68
    uint8_t labelsBelow;  // +0x74
    uint8_t alignment;    // +0x75
    uint8_t unknown76[2];
    u32 changed;        // +0x78
    SliderPart parts[6];  // +0x7c
    int32_t repeatClock;  // +0xac
    int16_t held;         // +0xb0
    int16_t shownScreen;  // +0xb2
    uint8_t repeat;       // +0xb4
    uint8_t unknownb5;
    int16_t slide;        // +0xb6
};
#pragma pack(pop)
static_assert(sizeof(Slider) == 0xb8, "a slider is 0xb8 bytes");

Slider& slider(App* app, u32 item)
{
    return record<Slider>(app, item);
}

// The sliders' kinds: how many parts (a word each), their parts' art
// (shapes A and B, 0x18 bytes a kind, 4 a part) and the art's names, the
// rectangles' sizes and colours without art (0x30 a kind, 8 a part), the 3D
// card's quads' texture places (0xc0 a kind, 0x20 a part; lit ones apart).
constexpr u32 kSliderParts = 0x55a4e0, kPartShapes = 0x678ab0;
constexpr u32 kPartNamesA = 0x55af9c, kPartNamesB = 0x55af0c;
constexpr u32 kPartSizes = 0x55adf0, kPartColours = 0x55adec;
constexpr u32 kPartQuads = 0x55a4ec, kHotPartQuads = 0x55a96c;
constexpr Var<u32> kArtTexture{0x749a38};
// The menus' rows a slider may be placed by; the screens sliding in; the
// screen's size; the menus' state bits; the clock; the renderer's flags.
constexpr u32 kRowsX = 0x559c6c, kRowsY = 0x559c94;
constexpr Var<int32_t> kScreenWidth{0x7cdac8}, kScreenHeight{0x7cdacc};
constexpr Var<uint8_t> kMenuBits2{0x55eb2e};
constexpr Var<int32_t> kClock{0x79c1a4};
constexpr Var<uint8_t> kRenderFlags{0x7a3a58};
constexpr Var<int32_t> kMenuScreen{0x66296c};
// Their constants: a half, 1/640 and 1/480 (a slider's, a two-axis one's),
// the percentages' scales and roundings, their formats.
constexpr Var<float> kHalf{0x53ae50};
constexpr Var<double> kPerWidth{0x53ae68}, kPerHeight{0x53ae60}, kPerWidth2{0x53aea0}, kPerHeight2{0x53ae98};
constexpr Var<float> kPercentFrom{0x53ae70}, kPercentScale{0x53ae74}, kPercentScale2{0x53ae78};
constexpr Var<float> kRoundDown{0x53ae7c}, kRoundUp{0x53ae80};
constexpr Var<float> kTwoAxisScale{0x53aea8}, kTwoAxisDown{0x53aeac}, kTwoAxisUp{0x53aeb0};
constexpr u32 kPercent = 0x53ae54, kTwoPercents = 0x53ae84;
constexpr u32 kOne = 0x3f800000, kHundredth = 0x3c23d70a;
// The code's blanks it copies into its frames (a slider's, a two-axis one's).
constexpr u32 kSliderBlank = 0x469020, kTwoAxisBlank = 0x469040;
// What they call: the clip (graphics'), floor, EA's shape found by name, the
// mouse holding a control, GetKeyState's slot; the port's.
constexpr u32 kSetClip = 0x4bebe0, kFloor = 0x4f1c30, kLocateShape = 0x4ef270, kMouseHolds = 0x49c080;
constexpr u32 kArtShapes = 0x8b48c0, kStrncmp = 0x4eb390;
constexpr Var<int32_t> kArtShapeCount{0x8b7628};
constexpr Var<u32> kArtFile{0x559228};
constexpr u32 kTexturedQuad = 0x4db000, kShadedQuad = 0x4db380;
constexpr u32 kViewport = 0x4beb80;
constexpr u32 kShapeSize = 0x4d66f0, kArtTextures = 0x4d6390;
constexpr u32 kDrawArt = 0x4d7160;

int32_t partsOf(App* app, int32_t kind)
{
    return half(app, kSliderParts + 2 * u32(kind));
}

/* Art shape A or B of part `part` of kind `kind` (-1 none). */
u32 shapeAt(int32_t kind, int32_t part, int b)
{
    return kPartShapes + 0x18 * u32(kind) + 4 * u32(part) + 2 * u32(b);
}

Float floatOf(uint32_t bits)
{
    return Float(fromBits(bits));
}

/* Words written below `at` as the code pushes them, the first the lowest:
 * the esp of the function then called. */
u32 argsBelow(App* app, u32 at, std::initializer_list<u32> words)
{
    u32 slot = at - 4 * u32(words.size());
    const u32 callee = slot - 4;
    for (const u32 w : words)
    {
        setWord(app, slot, w);
        slot += 4;
    }
    return callee;
}

/* The 640 by 480 measure's place `v` on the screen's own, as the code works
 * it out: the product (the screen's size times v) by 1/640 or 1/480,
 * truncated. */
u32 onScreen(ThreadState& ts, u32 product, double per)
{
    return u32(chopped<int32_t>(ts, fp::mul(Float(nfs::s32(product)), Float(per))));
}

/* ---- Their parts and values ---- */

/* 0x4692c0's: the part of the slider under the pointer, or -1. */
int32_t partUnder(App* app, u32 item)
{
    for (int32_t i = 0; i < partsOf(app, slider(app, item).kind); ++i)
    {
        const SliderPart& p = slider(app, item).parts[i];
        if (p.x < kPointerX(app) && p.x + p.width > kPointerX(app) && p.y < kPointerY(app)
            && p.y + p.height > kPointerY(app))
            return i;
    }
    return -1;
}

/* 0x469340's: where the slider's thumb may go: its track's start (the
 * track's left and half the thumb) to the word at `startAt`, its end to
 * `endAt`, the length between to `lengthAt`, its place, the length times its
 * value, to `placeAt` (and back). */
int32_t thumbPlaced(App* app, ThreadState& ts, u32 item, u32 startAt, u32 endAt, u32 lengthAt, u32 placeAt)
{
    const Slider& s = slider(app, item);
    const int32_t halfThumb = halved(s.parts[0].width);
    setHalf(app, startAt, halfThumb + s.parts[1].x);
    const int32_t end = s.parts[1].x + (s.parts[1].width - halfThumb);
    setHalf(app, endAt, end);
    setHalf(app, lengthAt, end - half(app, startAt));
    const int32_t place =
        chopped<int32_t>(ts, fp::mul(Float(int16_t(half(app, lengthAt))), floatOf(slider(app, item).values[0])));
    setHalf(app, placeAt, place);
    return place;
}

/* 0x4693c0's: the value at [esp+8] snapped to the step at [esp+4] -- to the
 * step below it, or the one above where it is past a half of the way -- and
 * kept within 0 and 1: a step past 1 is 1, one below 0 none (written back
 * to their words).  On the x87's stack as the code leaves it. */
std::optional<Float> snapped(App* app, ThreadState& ts, u32 esp)
{
    FpuStatus st;
    const u32 ebp = esp - 0xc, at = ebp - 0x10, stepAt = esp + 4, valueAt = esp + 8;
    bool snap = true;
    if (int32_t(word(app, stepAt)) > int32_t(kOne))
        setWord(app, stepAt, kOne);
    else
    {
        st.set(Float(0.0), Float(floatAt(app, valueAt)));
        if (!st.c0() && !st.c3())
            setWord(app, stepAt, 0);
        snap = (word(app, stepAt) & 0x7fffffff) != 0;
    }
    if (snap)
    {
        const Float steps = fp::div(Float(floatAt(app, valueAt)), Float(floatAt(app, stepAt)));
        setFloat(app, ebp - 0x10, float(steps));
        // 0x4f1c30's floor of the double: exact, as the host's.
        NFS2_USE(at);
        setFloat(app, ebp - 8, float(std::floor(double(steps))));
        const Float rest = fp::sub(Float(floatAt(app, ebp - 0x10)), Float(floatAt(app, ebp - 8)));
        setFloat(app, ebp - 0xc, float(fp::abs(rest)));
        if ((word(app, ebp - 0xc) & 0x7fffffff) != 0)
        {
            st.set(Float(floatAt(app, ebp - 0xc)), Float(kHalf(app)));
            const Float whole = !st.c0() ? fp::add(Float(1.0), Float(floatAt(app, ebp - 8)))
                                                   : Float(floatAt(app, ebp - 8));
            setFloat(app, ebp - 4, float(fp::mul(whole, Float(floatAt(app, stepAt)))));
        }
        else
            setWord(app, ebp - 4, word(app, valueAt));
    }
    else
        setWord(app, ebp - 4, word(app, valueAt));
    if (int32_t(word(app, ebp - 4)) > int32_t(kOne))
        setWord(app, ebp - 4, kOne);
    else
    {
        st.set(Float(0.0), Float(floatAt(app, ebp - 4)));
        if (!st.c0() && !st.c3())
            setWord(app, ebp - 4, 0);
    }
    return Float(floatAt(app, ebp - 4));
}

/* snapped() as the code calls it: the value and the step pushed below `at`. */
std::optional<Float> snappedAt(App* app, ThreadState& ts, u32 at, u32 step, u32 value)
{
    return snapped(app, ts, argsBelow(app, at, {step, value}));
}

/* The slider's function told of a change; eax else. */
Result changeTold(App* app, ThreadState& ts, u32 at, u32 item, u32 eax, u32 edx)
{
    const u32 function = slider(app, item).changed;
    if (function == 0)
        return int32_t(eax);
    u32 r;
    if (!gameCall(app, ts, at, function, {item, edx, function, 0}, {}, r))
        return {};
    return int32_t(r);
}

/* The thumb (part `left`'s x, its width after it) moved to its track's start
 * (the word at `startAt`) and its place (at `placeAt`) less half its width:
 * that half, as eax has it. */
u32 thumbMoved(App* app, u32 item, u32 left, u32 startAt, u32 placeAt)
{
    const int32_t h = halved(half(app, item + left + 2));
    setHalf(app, item + left, int32_t(half(app, placeAt)) + half(app, startAt) - h);
    return u32(h);
}

/* 0x469480's: the slider set to the value at [esp+4] -- within 0 and 1 and
 * snapped to its step, written back there --, its thumb moved there, its
 * function told. */
Result valueSet(App* app, ThreadState& ts, u32 esp, u32 item)
{
    FpuStatus st;
    const u32 ebp = esp - 0x14, at = ebp - 0x18, valueAt = esp + 4;
    const double value = double(Float(floatAt(app, valueAt)));
    app->getMemory<double>(ebp - 0x18) = value;
    st.set(Float(1.0), Float(value));
    if (st.c0())
        setWord(app, valueAt, kOne);
    else
    {
        st.set(Float(0.0), Float(value));
        if (!st.c0() && !st.c3())
            setWord(app, valueAt, 0);
    }
    const std::optional<Float> r = snappedAt(app, ts, at, slider(app, item).step, word(app, valueAt));
    if (!r)
        return {};
    setFloat(app, valueAt, float(*r));
    slider(app, item).values[0] = word(app, valueAt);
    thumbPlaced(app, ts, item, ebp - 0x10, ebp - 0xc, ebp - 8, ebp - 4);
    const u32 h = thumbMoved(app, item, 0x7e, ebp - 0x10, ebp - 4);
    return changeTold(app, ts, at, item, h, u32(int32_t(slider(app, item).parts[0].x)));
}

/* 0x469530's, 0x46afe0's: the arrows' step of axis `axis`: the stack's,
 * 1 past 1, a hundredth for none or less.  eax as fnstsw leaves it. */
u32 arrowStepSet(App* app, ThreadState& ts, u32 item, u32 step, int32_t axis, u32 eax)
{
    FpuStatus st;
    const u32 to = item + u32(axis) * 4 + 0x48;
    const Float value = Float(double(floatOf(step)));
    st.set(Float(1.0), value);
    if (st.c0())
    {
        setWord(app, to, kOne);
        return (eax & 0xffff0000) | st.word;
    }
    st.set(Float(0.0), value);
    setWord(app, to, st.c0() ? step : kHundredth);
    return st.c0() ? step : (eax & 0xffff0000) | st.word;
}

/* 0x469590's: the slider dragged to `place` along its track: its value that
 * far of the track's length, snapped to its step, its thumb moved there, its
 * function told. */
Result draggedTo(App* app, ThreadState& ts, u32 esp, u32 item, int16_t place)
{
    const u32 ebp = esp - 0x10, at = ebp - 0x1c;
    thumbPlaced(app, ts, item, ebp - 0x14, ebp - 8, ebp - 0x18, ebp - 4);
    if (place < 0)
        setHalf(app, ebp - 4, 0);
    else if (int32_t(place) > half(app, ebp - 0x18))
        setHalf(app, ebp - 4, half(app, ebp - 0x18));
    else
        setHalf(app, ebp - 4, place);
    setWord(app, ebp - 0x10, word(app, ebp - 4));
    setWord(app, ebp - 0xc, word(app, ebp - 0x18));
    setFloat(app, ebp - 0x1c,
             float(fp::div(Float(int16_t(half(app, ebp - 0x10))), Float(int16_t(half(app, ebp - 0xc))))));
    const std::optional<Float> r = snappedAt(app, ts, at, slider(app, item).step, word(app, ebp - 0x1c));
    if (!r)
        return {};
    setFloat(app, ebp - 0x1c, float(*r));
    slider(app, item).values[0] = word(app, ebp - 0x1c);
    thumbPlaced(app, ts, item, ebp - 0x14, ebp - 8, ebp - 0x18, ebp - 4);
    const u32 h = thumbMoved(app, item, 0x7e, ebp - 0x14, ebp - 4);
    return changeTold(app, ts, at, item, h, u32(int32_t(slider(app, item).parts[0].x)));
}

/* 0x46ac40's: where thumb `part` of the two-axis slider may go along axis
 * `axis` -- axes 0 and 1 parts 0 and 1 from its area's left, 2 and 3 from its
 * middle; another axis part 4 along its bar --: its start to `startAt`, its
 * end to `endAt`, the length between to `lengthAt`, its place (the length
 * times the axis's value) to `placeAt`.  eax: the place, or the slider for a
 * part the axis has not. */
int32_t twoAxisPlaced(App* app, ThreadState& ts, u32 item, int32_t axis, u32 part, u32 startAt, u32 endAt,
                      u32 lengthAt, u32 placeAt)
{
    const Slider& s = slider(app, item);
    const SliderPart& area = s.parts[5];
    if (axis == 0 || axis == 1)
    {
        if (part > 3)
            return int32_t(item);
        const int32_t h = halved(s.parts[part].width);
        int32_t end;
        if (part < 2)
        {
            setHalf(app, startAt, area.x + h);
            end = area.x + (halved(area.width) - h);
        }
        else
        {
            setHalf(app, startAt, area.x + halved(area.width) + h);
            end = area.x + (area.width - h);
        }
        setHalf(app, endAt, end);
        setHalf(app, lengthAt, end - half(app, startAt));
    }
    else
    {
        if (part != 4)
            return int32_t(item);
        setHalf(app, startAt, uint16_t(area.x) + uint16_t(s.parts[0].width));
        const int32_t end = uint16_t(area.x) + uint16_t(area.width) - uint16_t(s.parts[2].width);
        setHalf(app, endAt, end);
        setHalf(app, lengthAt, end - uint16_t(half(app, startAt)));
    }
    const int32_t place = chopped<int32_t>(
        ts, fp::mul(Float(int16_t(half(app, lengthAt))), floatOf(word(app, item + 0x54 + 4 * u32(axis)))));
    setHalf(app, placeAt, place);
    return place;
}

/* The place from the track's far end: the length (at `lengthAt`) less it. */
void fromFarEnd(App* app, u32 lengthAt, u32 placeAt)
{
    setHalf(app, placeAt, int32_t(word(app, lengthAt) - word(app, placeAt)));
}

/* 0x46add0's: axis [esp+8] of the two-axis slider set to the value at
 * [esp+4] -- within 0 and 1 and snapped to its step, written back --, its
 * thumbs moved there: across parts 0 and 2 (the second from the far end), up
 * 1 (from the far end) and 3, for axis 2 part 4; its function told. */
Result twoAxisValueSet(App* app, ThreadState& ts, u32 esp, u32 item, u32 eax)
{
    FpuStatus st;
    const u32 ebp = esp - 0x18, at = ebp - 0x10, valueAt = esp + 4;
    const int32_t axis = int32_t(word(app, esp + 8));
    if (int32_t(word(app, valueAt)) > int32_t(kOne))
        setWord(app, valueAt, kOne);
    else
    {
        st.set(Float(0.0), Float(floatAt(app, valueAt)));
        eax = (eax & 0xffff0000) | st.word;
        if (!st.c0() && !st.c3())
            setWord(app, valueAt, 0);
    }
    const std::optional<Float> r = snappedAt(app, ts, at, slider(app, item).step, word(app, valueAt));
    if (!r)
        return {};
    setFloat(app, valueAt, float(*r));
    setWord(app, item + 0x54 + 4 * u32(axis), word(app, valueAt));
    const u32 start = ebp - 0xc, end = ebp - 8, length = ebp - 0x10, place = ebp - 4;
    const auto placed = [&](u32 part) {
        twoAxisPlaced(app, ts, item, axis, part, start, end, length, place);
    };
    if (axis == 0)
    {
        placed(0);
        thumbMoved(app, item, 0x7e, start, place);
        placed(2);
        fromFarEnd(app, length, place);
        eax = thumbMoved(app, item, 0x8e, start, place);
    }
    else if (axis == 1)
    {
        placed(1);
        fromFarEnd(app, length, place);
        thumbMoved(app, item, 0x86, start, place);
        placed(3);
        eax = thumbMoved(app, item, 0x96, start, place);
    }
    else if (axis == 2)
    {
        placed(4);
        fromFarEnd(app, length, place);
        const int32_t h = int32_t(slider(app, item).parts[4].width) / axis;
        slider(app, item).parts[4].x = int16_t(half(app, place) + half(app, start) - h);
        eax = u32(h);
    }
    return changeTold(app, ts, at, item, eax, 0);
}

/* 0x46b040's: axis `axis` (0 across, 1 up) of the two-axis slider dragged to
 * `to` along its track: each of its thumbs on the axis placed, the value that
 * far of the track's length snapped to its step, the thumb moved there (the
 * second from the far end), its function told. */
Result twoAxisDragged(App* app, ThreadState& ts, u32 esp, u32 item, int16_t to, int32_t axis)
{
    const u32 ebp = esp - 0x10, at = ebp - 0x20;
    setHalf(app, ebp - 8, to);
    const u32 start = ebp - 0x1c, end = ebp - 0x14, length = ebp - 0x18, place = ebp - 0x10;
    const auto placed = [&](u32 part) {
        twoAxisPlaced(app, ts, item, axis, part, start, end, length, place);
    };
    // The place dragged to, within 0 and the track's length.
    const auto clamp = [&] {
        if (half(app, ebp - 8) < 0)
            setHalf(app, place, 0);
        else if (half(app, ebp - 8) > half(app, length))
            setHalf(app, place, half(app, length));
        else
            setHalf(app, place, half(app, ebp - 8));
    };
    const auto value = [&](u32 off) {
        setWord(app, ebp - 4, word(app, place));
        setWord(app, ebp - 0xc, word(app, length));
        setFloat(app, ebp - 0x20,
                 float(fp::div(Float(int16_t(half(app, ebp - 4))), Float(int16_t(half(app, ebp - 0xc))))));
        const std::optional<Float> r = snappedAt(app, ts, at, slider(app, item).step, word(app, ebp - 0x20));
        if (!r)
            return false;
        setFloat(app, ebp - 0x20, float(*r));
        setWord(app, item + off, word(app, ebp - 0x20));
        return true;
    };
    u32 eax = item;
    if (axis == 0)
    {
        placed(0);
        clamp();
        if (!value(0x54))
            return {};
        placed(0);
        thumbMoved(app, item, 0x7e, start, place);
        placed(2);
        clamp();
        if (!value(0x54))
            return {};
        placed(2);
        fromFarEnd(app, length, place);
        eax = thumbMoved(app, item, 0x8e, start, place);
    }
    else if (axis == 1)
    {
        placed(1);
        clamp();
        if (!value(0x58))
            return {};
        placed(1);
        fromFarEnd(app, length, place);
        thumbMoved(app, item, 0x86, start, place);
        placed(3);
        clamp();
        if (!value(0x58))
            return {};
        placed(3);
        eax = thumbMoved(app, item, 0x96, start, place);
    }
    return changeTold(app, ts, at, item, eax, 0);
}

/* ---- The mouse and the keys ---- */

/* 0x46a5f0's: the slider as the mouse works it, while it holds the control:
 * the part under the pointer hot -- the thumb taken up, an arrow starting
 * its repeat -- the thumb taken up dragged to the pointer, an arrow held
 * stepping the value every 0x20 ticks by its step (or the arrows' where that
 * is not within 0 and 1).  Let go, the thumb put down, the repeat stopped. */
Result sliderTracked(App* app, ThreadState& ts, u32 esp, u32 item)
{
    FpuStatus st;
    const u32 ebp = esp - 0x18, at = ebp - 0x14;
    u32 e;
    if (!callCore<kMouseHolds, frontend::glue::mouseKey>(app, ts, at, e))
        return {};
    Slider& s = slider(app, item);
    if (e == 0)
    {
        s.held = 0;
        s.repeat = 0xff;
        s.state &= 0xfb;
        return 0;
    }
    const int32_t part = partUnder(app, item);
    e = u32(part);
    if (part == 0)
    {
        s.held = 1;
        s.state |= 4;
        e = (e & 0xffff00ff) | u32(s.state) << 8;
    }
    else if (part == 2 && s.repeat != 2)
    {
        e = u32(kClock(app));
        s.repeat = 2;
        s.repeatClock = int32_t(e);
        s.state |= 4;
    }
    else if (part == 3 && s.repeat != 3)
    {
        e = u32(kClock(app));
        s.repeat = 3;
        s.repeatClock = int32_t(e);
        s.state |= 4;
    }
    if (slider(app, item).held != 0)
    {
        if (part >= 0 && part < partsOf(app, slider(app, item).kind))
        {
            e = u32(thumbPlaced(app, ts, item, ebp - 0x10, ebp - 8, ebp - 0xc, ebp - 4));
            const int32_t x = kPointerX(app);
            if (half(app, ebp - 8) < x)
                e = word(app, ebp - 0xc);
            else if (half(app, ebp - 0x10) > x)
                e = 0;
            else
                e = ((u32(int32_t(half(app, ebp - 0x10))) & 0xffff0000) | uint16_t(x)) - word(app, ebp - 0x10);
            if (uint16_t(e) != uint16_t(half(app, ebp - 4)))
            {
                const Result r = draggedTo(app, ts, at - 4, item, int16_t(e));
                if (!r)
                    return {};
                e = u32(*r);
            }
        }
        else
            slider(app, item).held = 0;
    }
    // The step: its own, or the arrows' where it is not within 0 and 1.
    st.set(Float(0.0), floatOf(slider(app, item).step));
    e = (e & 0xffff0000) | st.word;
    u32 step;
    if (st.c0() && int32_t(slider(app, item).step) <= int32_t(kOne))
        step = slider(app, item).step;
    else
        e = step = slider(app, item).arrowSteps[0];
    setWord(app, ebp - 0x14, step);
    const uint8_t repeat = slider(app, item).repeat;
    if ((repeat == 2 && part == 2) || (repeat == 3 && part == 3))
    {
        e = u32(kClock(app)) - u32(slider(app, item).repeatClock);
        if (e < 0x20)
            return int32_t(e);
        Float v = floatOf(slider(app, item).values[0]);
        if (repeat == 2)
        {
            v = fp::sub(v, floatOf(step));
            setFloat(app, item + 0x54, float(v));
            st.set(Float(0.0), v);
            if (!st.c0() && !st.c3())
                slider(app, item).values[0] = 0;
        }
        else
        {
            v = fp::add(v, floatOf(step));
            setFloat(app, item + 0x54, float(v));
            st.set(Float(1.0), v);
            if (st.c0())
                slider(app, item).values[0] = kOne;
        }
        if (!valueSet(app, ts, argsBelow(app, at, {slider(app, item).values[0], 0}), item))
            return {};
        slider(app, item).repeatClock = kClock(app);
        return kClock(app);
    }
    slider(app, item).repeat = 0xff;
    return int32_t(e);
}

/* 0x46c320's: the two-axis slider as the mouse works it, while it holds the
 * control: the part under the pointer taken up and hot, then dragged to the
 * pointer along its axis (parts 0 and 2 across, 1 and 3 up; 1 and 2 from the
 * far end).  Let go, it is put down. */
Result twoAxisTracked(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x18, at = ebp - 0x10;
    u32 e;
    if (!callCore<kMouseHolds, frontend::glue::mouseKey>(app, ts, at, e))
        return {};
    Slider& s = slider(app, item);
    if (e == 0)
    {
        if (uint16_t(s.held) == 0xffff)
            return 0;
        s.held = -1;
        s.state &= 0xfb;
        return int32_t(u32(s.state) << 8);
    }
    if (uint16_t(s.held) == 0xffff)
    {
        e = u32(partUnder(app, item));
        if (e <= 3)
        {
            slider(app, item).held = int16_t(e);
            slider(app, item).state |= 4;
        }
    }
    e = (e & 0xffff0000) | uint16_t(slider(app, item).held);
    if (slider(app, item).held < 0)
        return int32_t(e);
    e = u32(partsOf(app, slider(app, item).kind));
    if (slider(app, item).held >= int32_t(e))
        return int32_t(e);
    const uint16_t part = uint16_t(slider(app, item).held);
    e = (e & 0xffff0000) | part;
    if (part > 3)
        return int32_t(e);
    const int32_t axis = part & 1;
    e = u32(twoAxisPlaced(app, ts, item, axis, u32(int32_t(slider(app, item).held)), ebp - 4, ebp - 0x10,
                            ebp - 0xc, ebp - 8));
    const int32_t x = kPointerX(app);
    if (half(app, ebp - 0x10) < x)
        e = word(app, ebp - 0xc);
    else if (half(app, ebp - 4) > x)
        e = 0;
    else
        e = ((u32(int32_t(half(app, ebp - 4))) & 0xffff0000) | uint16_t(x)) - word(app, ebp - 4);
    if (uint16_t(e) == uint16_t(half(app, ebp - 8)))
        return int32_t(e);
    const uint16_t heldPart = uint16_t(slider(app, item).held);
    const int16_t to = heldPart == 2 || heldPart == 1 ? int16_t(word(app, ebp - 0xc) - e) : int16_t(e);
    return twoAxisDragged(app, ts, at - 4, item, to, axis);
}

/* 0x46a860's: a key on a slider (none while its thumb is held): left steps
 * its value down, right up, Enter on an arrow as that arrow -- by its step,
 * or the arrows' where that is not within 0 and 1.  2 for a key taken. */
Result sliderKeyed(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    FpuStatus st;
    const u32 at = esp - 0x10 - 4;
    const uint16_t key = uint16_t(ebx);
    st.set(Float(0.0), floatOf(slider(app, item).step));
    const u32 step = st.c0() && int32_t(slider(app, item).step) <= int32_t(kOne)
                           ? slider(app, item).step
                           : slider(app, item).arrowSteps[0];
    if (slider(app, item).held != 0)
        return 0;
    int direction = 0;
    if (key == kLeft)
        direction = -1;
    else if (key == kRight)
        direction = 1;
    else if (key == kEnter)
    {
        const int32_t part = partUnder(app, item);
        direction = part == 2 ? -1 : part == 3 ? 1 : 0;
    }
    if (direction == 0)
        return 0;
    Float v = floatOf(slider(app, item).values[0]);
    if (direction < 0)
    {
        v = fp::sub(v, floatOf(step));
        setFloat(app, item + 0x54, float(v));
        st.set(Float(0.0), v);
        if (!st.c0() && !st.c3())
            slider(app, item).values[0] = 0;
    }
    else
    {
        v = fp::add(v, floatOf(step));
        setFloat(app, item + 0x54, float(v));
        st.set(Float(1.0), v);
        if (st.c0())
            slider(app, item).values[0] = kOne;
    }
    if (!valueSet(app, ts, argsBelow(app, at, {slider(app, item).values[0], 0}), item))
        return {};
    return 2;
}

/* 0x46c4a0's: a key on a two-axis slider (none while a thumb is held): left
 * steps it down, right up -- across with Shift held (GetKeyState), up else
 * -- by its step, or the axis's arrows' where that is not within 0 and 1.
 * 2 for a key taken. */
Result twoAxisKeyed(App* app, ThreadState& ts, u32 esp, u32 item, u32 ebx)
{
    FpuStatus st;
    const u32 ebp = esp - 0x10, at = ebp - 0x10;
    const uint16_t key = uint16_t(ebx);
    if (uint16_t(slider(app, item).held) != 0xffff)
        return 0;
    if (key != kLeft && key != kRight)
        return 0;
    // Whether Shift is down, esp as the code has it.
    ts.sp = at;
    const int32_t axis = os::window::keyDown(0x10) ? 0 : 1;
    st.set(Float(0.0), floatOf(slider(app, item).step));
    const u32 step = st.c0() && int32_t(slider(app, item).step) <= int32_t(kOne)
                           ? slider(app, item).step
                           : slider(app, item).arrowSteps[axis];
    const u32 valueAt = item + 0x54 + 4 * u32(axis);
    Float v = floatOf(word(app, valueAt));
    if (key == kLeft)
    {
        v = fp::sub(v, floatOf(step));
        setFloat(app, valueAt, float(v));
        st.set(Float(0.0), v);
        if (!st.c0() && !st.c3())
            setWord(app, valueAt, 0);
    }
    else
    {
        v = fp::add(v, floatOf(step));
        setFloat(app, valueAt, float(v));
        st.set(Float(1.0), v);
        if (st.c0())
            setWord(app, valueAt, kOne);
    }
    if (!twoAxisValueSet(app, ts, argsBelow(app, at, {word(app, valueAt), u32(axis)}), item, item))
        return {};
    return 2;
}

/* ---- Their layout ---- */

/* 0x469080's: each part's art -- found by name or made (sliderShape), none
 * over a race -- and its size: the art's, or the kind's own where there is no
 * art or it is empty.  eax the kind. */
Result partSizes(App* app, ThreadState& ts, u32 esp, u32 item);

/* 0x4d5e40's: the menus' shape named `name` (4 characters): among those
 * known, or made from the menus' art (EA's shape of that name, made textures,
 * named); -1 where the art has none. */
Result shapeOf(App* app, ThreadState& ts, u32 esp, u32 name, u32 ebx)
{
    const u32 at = esp - 0x14;
    u32 r;
    for (int32_t i = 0; i < kArtShapeCount(app); ++i)
    {
        ebx = 4;
        if (!callCore<kStrncmp, watcom::clib::strncmp>(app, ts, at,
                                                       r, kArtShapes + u32(i) * 0x14 + 0x10, name, u32(4)))
            return {};
        if (r == 0)
            return i;
    }
    u32 shape, made;
    if (!plainAt<kLocateShape, ea::shapeLocated>(app, ts, at, &shape, kArtFile(app), name))
        return {};
    if (shape == 0)
        return -1;
    if (!coreAt<0x4d6390, shapes::artTexturesOf, 0>(app, ts, at, &made, shape, 0, name))
        return {};
    if (made != 0
        && !callCore<kStrncpy, watcom::clib::strncpy>(app, ts, at, r,
                                                      kArtShapes + made * 0x14 + 0x10, name, u32(4)))
        return {};
    return int32_t(made);
}

Result partSizes(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 at = esp - 0x18 - 4;
    for (int32_t i = 0;; ++i)
    {
        const int32_t kind = slider(app, item).kind;
        if (i >= partsOf(app, kind))
            return kind;
        const u32 a = shapeAt(kind, i, 0), b = shapeAt(kind, i, 1);
        const u32 sizes = kPartSizes + 0x30 * u32(kind) + 8 * u32(i);
        SliderPart& p = slider(app, item).parts[i];
        if (kOverRace(app) != 0)
        {
            setHalf(app, a, -1);
            setHalf(app, b, -1);
            p.width = half(app, sizes);
            p.height = half(app, sizes + 2);
        }
        else
        {
            const u32 nameB = word(app, kPartNamesB + 0x18 * u32(kind) + 4 * u32(i));
            if (nameB != 0)
            {
                const Result r = shapeOf(app, ts, at - 4, nameB, nameB);
                if (!r)
                    return {};
                setHalf(app, b, *r);
            }
            else
                setHalf(app, b, -1);
            const u32 ebxA = nameB != 0 ? u32(int32_t(half(app, b))) : 0;
            const u32 nameA = word(app, kPartNamesA + 0x18 * u32(kind) + 4 * u32(i));
            if (nameA != 0)
            {
                const Result r = shapeOf(app, ts, at - 4, nameA, ebxA);
                if (!r)
                    return {};
                setHalf(app, a, *r);
            }
            else
                setHalf(app, a, -1);
        }
        const u32 partAt = item + 0x7c + 8 * u32(i);
        u32 r;
        if (half(app, b) != -1
            && !namedPlainAt<shapes::shapeSizeOf>(app, ts, at, &r, u32(int32_t(half(app, b))), partAt + 4, partAt + 6))
            return {};
        SliderPart& q = slider(app, item).parts[i];
        if (q.height <= 0 || q.width <= 0)
        {
            setHalf(app, b, -1);
            setHalf(app, a, -1);
            q.width = half(app, sizes);
            q.height = half(app, sizes + 2);
        }
    }
}

/* 0x46a980's: a slider's parts placed: the arrows, the track and the thumb
 * in a row about its middle -- the down arrow left of the track, the up arrow
 * right of it, the thumb on the track's start --, each as high as the
 * highest's middle.  eax: half the up arrow's height. */
u32 sliderPlaced(App* app, u32 item)
{
    Slider& s = slider(app, item);
    const int16_t width = int16_t(s.parts[1].width + s.parts[2].width + s.parts[3].width);
    int16_t tallest = s.parts[0].height;
    for (int p = 1; p < 4; ++p)
        if (tallest < s.parts[p].height)
            tallest = s.parts[p].height;
    const int32_t middleX = int16_t(halved(width) + s.x), middleY = int16_t(halved(tallest) + s.y);
    s.parts[0].x = int16_t(middleX - halved(s.parts[0].width));
    s.parts[0].y = int16_t(middleY - halved(s.parts[0].height));
    s.parts[1].x = int16_t(middleX - halved(s.parts[1].width));
    s.parts[1].y = int16_t(middleY - halved(s.parts[1].height));
    s.parts[2].x = int16_t(middleX - halved(s.parts[1].width) - s.parts[2].width);
    s.parts[2].y = int16_t(middleY - halved(s.parts[2].height));
    s.parts[3].x = int16_t(halved(s.parts[1].width) + middleX);
    const int32_t h3 = halved(s.parts[3].height);
    s.parts[3].y = int16_t(middleY - h3);
    return u32(h3);
}

/* 0x46c640's: a two-axis slider's parts placed about its area's middle: the
 * thumbs across on the area's middle, those up and the bar's on its top. */
u32 twoAxisPlacedParts(App* app, u32 item)
{
    Slider& s = slider(app, item);
    int16_t tallest = s.parts[0].height;
    for (int p = 1; p < 6; ++p)
        if (tallest < s.parts[p].height)
            tallest = s.parts[p].height;
    const int16_t middleX = int16_t(halved(s.parts[5].width) + s.x);
    const int32_t middleY = halved(tallest) + s.y;
    s.parts[5].x = int16_t(middleX - halved(s.parts[5].width));
    s.parts[5].y = int16_t(middleY - halved(s.parts[5].height));
    s.parts[0].x = int16_t(middleX - halved(s.parts[0].width));
    s.parts[0].y = int16_t(middleY - s.parts[0].height);
    s.parts[1].x = int16_t(middleX - halved(s.parts[1].width));
    s.parts[2].x = int16_t(middleX - halved(s.parts[2].width));
    s.parts[2].y = int16_t(middleY - s.parts[2].height);
    s.parts[3].x = int16_t(middleX - halved(s.parts[3].width));
    s.parts[4].x = int16_t(middleX - halved(s.parts[4].width));
    const int32_t h4 = halved(s.parts[4].height);
    s.parts[1].y = int16_t(middleY);
    s.parts[3].y = int16_t(middleY);
    s.parts[4].y = int16_t(middleY - h4);
    return u32(h4);
}

/* Placed by its row of the menus' table where it asks for it. */
void placedByRow(App* app, u32 item)
{
    Slider& s = slider(app, item);
    if (s.ownPlace != 0)
    {
        s.x = int16_t(half(app, kRowsX + 4 * u32(int32_t(s.row))) + 0x94);
        s.y = half(app, kRowsY + 4 * u32(int32_t(s.row)));
    }
}

/* 0x46ab10's: a slider set up: placed, its arrows' step and its value set,
 * its screen kept (and moved on two, to slide in, on the 2D screens). */
Result sliderSetUp(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 at = esp - 0x10;
    placedByRow(app, item);
    sliderPlaced(app, item);
    arrowStepSet(app, ts, item, slider(app, item).arrowSteps[0], 0, item);
    const Result r = valueSet(app, ts, argsBelow(app, at, {slider(app, item).values[0], 0}), item);
    if (!r)
        return {};
    Slider& s = slider(app, item);
    u32 e = (u32(*r) & 0xffff0000) | uint16_t(s.screen);
    s.shownScreen = s.screen;
    if (kOverRace(app) == 0)
    {
        s.screen = int16_t(e + 2);
        if (kMenuScreen(app) != 9 && s.slide == 0)
        {
            e = ((e & 0xffff0000) | uint16_t(s.y)) - 0x46;
            s.slide = int16_t(e);
        }
    }
    return int32_t(e);
}

/* 0x46c820's: a two-axis slider set up: placed, each axis's arrows' step and
 * value set. */
Result twoAxisSetUp(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 at = esp - 0x10;
    placedByRow(app, item);
    twoAxisPlacedParts(app, item);
    u32 e = item;
    for (int32_t axis = 0; axis < 3; ++axis)
        e = arrowStepSet(app, ts, item, slider(app, item).arrowSteps[axis], axis, e);
    Result r;
    for (int32_t axis = 0; axis < 3; ++axis)
    {
        r = twoAxisValueSet(app, ts, argsBelow(app, at, {slider(app, item).values[axis], u32(axis)}), item, item);
        if (!r)
            return {};
    }
    return r;
}

/* 0x46abb0's, 0x46c8b0's: the box the pointer finds it in: at it, as wide as
 * its track and arrows (a two-axis one's area), as high as its highest part.
 * eax as the code leaves it. */
u32 sliderBoxed(App* app, u32 item)
{
    Slider& s = slider(app, item);
    s.boxX = s.x;
    s.boxY = s.y;
    s.boxWidth = int16_t(s.parts[1].width + s.parts[2].width + s.parts[3].width);
    int32_t tallest = s.parts[0].height;
    for (int p = 1; p < 4; ++p)
        if (tallest < s.parts[p].height)
            tallest = s.parts[p].height;
    s.boxHeight = int16_t(tallest);
    return u32(int32_t(s.parts[3].height));
}

u32 twoAxisBoxed(App* app, u32 item)
{
    Slider& s = slider(app, item);
    s.boxX = s.x;
    s.boxY = s.y;
    s.boxWidth = s.parts[5].width;
    int32_t tallest = s.parts[0].height;
    for (int p = 1; p < 6; ++p)
        if (tallest < s.parts[p].height)
            tallest = s.parts[p].height;
    s.boxHeight = int16_t(tallest);
    return u32(tallest);
}

/* ---- Their drawing ---- */

/* The menus' greying: a slider not kept from it (+0x18) greyed (+4 bit 0)
 * where the menus grey their controls. */
void greyed(App* app, u32 item)
{
    const uint8_t menus = kMenuBits2(app);
    if ((menus & 0x20) != 0 && (byteAt(app, item + 0x18) & 1) != 0)
        return;
    if ((menus & 0x10) != 0 && half(app, item + 0x18) != 0)
        return;
    if ((kMenuBits2(app) & 0x30) != 0)
        slider(app, item).flags |= 1;
}

/* A quad of the art's texture, its corners from (x0, y0) round, its texture's
 * places the 8 words at `places`, in `colour`. */
bool artQuad(App* app, ThreadState& ts, u32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
             int32_t x3, int32_t y3, u32 places, u32 colour)
{
    const auto t = [&](u32 i) { return word(app, places + 4 * i); };
    return coreAt<0x4db000, draw2d::texturedQuad, 14>(
        app, ts, at, nullptr,
        draw2d::ArtQuad{{x0, x1, x2, x3}, {y0, y1, y2, y3}, {t(0), t(2), t(4), t(6)}, {t(1), t(3), t(5), t(7)},
                        kArtTexture(app), colour});
}

/* A quad of four colours, its corners from (x0, y0) round. */
bool colourQuad(App* app, ThreadState& ts, u32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                int32_t x3, int32_t y3, u32 c0, u32 c1, u32 c2, u32 c3)
{
    return coreAt<0x4db380, draw2d::shadedQuad, 8>(app, ts, at, nullptr,
                                                   draw2d::ShadedQuad{{x0, x1, x2, x3}, {y0, y1, y2, y3}, {c0, c1, c2, c3}});
}

/* The clip set to a rectangle of the screen's own. */
bool clipped(App* app, ThreadState& ts, u32 at, u32 left, u32 top, u32 right, u32 bottom)
{
    return plainAt<kSetClip, graphics::clipSet>(app, ts, at, nullptr, nfs::s32(left), nfs::s32(top),
                                                nfs::s32(right), nfs::s32(bottom));
}

/* The screen's viewport kept in the frame at `kept` (left, top, width,
 * height: eax's, edx's, ebx's, ecx's words) and put back. */
bool viewportKept(App* app, ThreadState& ts, u32 at, u32 left, u32 top, u32 width, u32 height)
{
    return plainAt<0x4beb80, text::viewportOf>(app, ts, at, nullptr, left, top, width, height);
}

/* 0x469680's: a slider drawn. */
Result sliderDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    FpuStatus st;
    const u32 ebp = esp - 0x14, at = ebp - 0xc4, text = ebp - 0xc4;
    for (u32 i = 0; i < 0x20; i += 4)
        setWord(app, text + i, word(app, kSliderBlank + i));
    // The viewport kept: its left, top, width and height.
    const u32 keptLeft = ebp - 0xa0, keptTop = ebp - 0x94, keptWidth = ebp - 0x9c, keptHeight = ebp - 0x98;
    const auto restore = [&] {
        return clipped(app, ts, at, word(app, keptLeft), word(app, keptTop), word(app, keptWidth),
                       word(app, keptHeight));
    };
    const int32_t textHeight = 0xe;
    bool labelsShown = false;
    int32_t under = -1;
    const bool sliding = kScreensSliding(app) != 0 && kOverRace(app) == 0;
    greyed(app, item);
    u32 colour;
    if ((slider(app, item).flags & 1) != 0)
        colour = 0x908080;
    else if (hot != 0)
    {
        under = partUnder(app, item);
        colour = 0xffe440;
    }
    else
        colour = 0xff649dfd;
    u32 r;
    const auto drawPart = [&](int32_t part) -> bool {
        const int32_t kind = slider(app, item).kind;
        const SliderPart p = slider(app, item).parts[part];
        if (half(app, shapeAt(kind, part, 1)) == -1 && kOverRace(app) == 0)
        {
            // No art: a rectangle in its kind's colour, the hot arrow or thumb framed.
            const u32 fill = word(app, kPartColours + 0x30 * u32(kind) + 8 * u32(part)) | 0xff000000;
            if (!namedAt<draw2d::filled, 1>(app, ts, at, &r, u32(int32_t(p.x)), u32(int32_t(p.y)), u32(int32_t(p.width)), u32(int32_t(p.height)), fill))
                return false;
            if (hot == 0 || part != under || (part != 0 && part != 2 && part != 3))
                return true;
            return namedAt<draw2d::outline, 1>(app, ts, at, &r, u32(int32_t(p.x)), u32(int32_t(p.y)), u32(int32_t(p.width)), u32(int32_t(p.height)), 0xffffe440);
        }
        const int32_t x = p.x, y = p.y, w = p.width, h = p.height;
        const u32 alpha = slider(app, item).alphaOn != 0 && (kRenderFlags(app) & 2) == 0
                                ? u32(slider(app, item).alpha) << 24 | 0xffffff
                                : 0xffffffff;
        if (part > 3)
            return true;
        const u32 places = kPartQuads + 0xc0 * u32(kind) + 0x20 * u32(part);
        if (part == 1)
        {
            // The track.
            const SliderPart& track = slider(app, item).parts[1];
            const SliderPart& thumb = slider(app, item).parts[0];
            const int32_t left = track.x + 0x10, top = track.y + 2, right = track.x + track.width - 1;
            const int32_t inner = track.x + track.width - 0x10, bottom = track.height + track.y - 2;
            if (kOverRace(app) != 0)
            {
                const int32_t hw = halved(w), middle = x + hw, foot = y + h;
                if (!artQuad(app, ts, at, x, y, middle, y, middle, foot, x, foot, places, alpha)
                    || !artQuad(app, ts, at, middle + hw, y, middle, y, middle, foot, middle + hw, foot, places, alpha))
                    return false;
                u32 light, dark;
                if (slider(app, item).alphaOn != 0 && (kRenderFlags(app) & 2) == 0)
                {
                    const u32 a = u32(slider(app, item).alpha) << 24;
                    dark = a | 0x15052d;
                    light = a | 0xff40;
                }
                else
                {
                    light = 0xff00ff40;
                    dark = 0xff15052d;
                }
                if (!viewportKept(app, ts, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const SliderPart& t1 = slider(app, item).parts[1];
                const u32 W = u32(kScreenWidth(app)), H = u32(kScreenHeight(app));
                const u32 clipRight = onScreen(ts, (u32(t0.x + halved(t0.width)) - u32(x)) * W, kPerWidth(app));
                const u32 clipBottom = onScreen(ts, u32(t1.height + 4) * H, kPerHeight(app));
                const u32 clipTop = onScreen(ts, u32(y - 2) * H, kPerHeight(app));
                const u32 clipLeft = onScreen(ts, W * u32(x), kPerWidth(app));
                if (!clipped(app, ts, at, clipLeft, clipTop, clipRight, clipBottom)
                    || !colourQuad(app, ts, at, left, top, inner, top, right, bottom, track.x + 1, bottom, dark,
                                   light, light, dark))
                    return false;
                (void)thumb;
                return restore();
            }
            if ((kRenderFlags(app) & 2) != 0)
            {
                if (half(app, shapeAt(kind, 1, 1)) == -1)
                    return true;
                if (!namedAt<shapes::drawArt, 0>(app, ts, at, &r, u32(int32_t(half(app, shapeAt(kind, 1, 1)))), u32(x), u32(y), alpha)
                    || !viewportKept(app, ts, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const u32 W = u32(kScreenWidth(app)), H = u32(kScreenHeight(app));
                const u32 clipRight = onScreen(ts, (u32(t0.x + halved(t0.width)) - u32(x)) * W, kPerWidth(app));
                const u32 clipBottom =
                    onScreen(ts, u32(slider(app, item).parts[1].height + 4) * H, kPerHeight(app));
                const u32 clipTop = onScreen(ts, u32(y - 2) * H, kPerHeight(app));
                const u32 clipLeft = onScreen(ts, W * u32(x), kPerWidth(app));
                return clipped(app, ts, at, clipLeft, clipTop, clipRight, clipBottom)
                       && colourQuad(app, ts, at, left, top, inner, top, right, bottom, track.x + 1, bottom,
                                     0xff15052d, 0xff00ff40, 0xff00ff40, 0xff15052d)
                       && restore();
            }
            if (half(app, shapeAt(kind, 1, 0)) != -1)
            {
                if (!viewportKept(app, ts, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const u32 W = u32(kScreenWidth(app)), H = u32(kScreenHeight(app));
                const u32 clipBottom =
                    onScreen(ts, u32(slider(app, item).parts[part].height + 4) * H, kPerHeight(app));
                const u32 clipRight = onScreen(ts, (u32(t0.x + halved(t0.width)) - u32(x)) * W, kPerWidth(app));
                const u32 clipTop = onScreen(ts, u32(y - 2) * H, kPerHeight(app));
                const u32 clipLeft = onScreen(ts, W * u32(x), kPerWidth(app));
                if (!clipped(app, ts, at, clipLeft, clipTop, clipRight, clipBottom)
                    || !namedAt<shapes::drawArt, 0>(app, ts, at, &r, u32(int32_t(half(app, shapeAt(kind, 1, 0)))), u32(x), u32(y), alpha)
                    || !restore())
                    return false;
            }
            if (half(app, shapeAt(kind, part, 1)) == -1)
                return true;
            if (!viewportKept(app, ts, at, keptLeft, keptTop, keptWidth, keptHeight))
                return false;
            const SliderPart& t0 = slider(app, item).parts[0];
            const SliderPart& t1 = slider(app, item).parts[1];
            const int32_t thumbMiddle = t0.x + halved(t0.width);
            const u32 W = u32(kScreenWidth(app)), H = u32(kScreenHeight(app));
            const u32 clipBottom =
                onScreen(ts, u32(slider(app, item).parts[part].height + 4) * H, kPerHeight(app));
            const u32 clipRight = onScreen(ts, W * u32(t1.x + t1.width - thumbMiddle - 1), kPerWidth(app));
            const u32 clipTop = onScreen(ts, u32(y - 2) * H, kPerHeight(app));
            const u32 clipLeft = onScreen(ts, u32(thumbMiddle) * W, kPerWidth(app));
            if (!clipped(app, ts, at, clipLeft, clipTop, clipRight, clipBottom)
                || !namedAt<shapes::drawArt, 0>(app, ts, at, &r, u32(int32_t(half(app, shapeAt(kind, part, 1)))), u32(x), u32(y), alpha))
                return false;
            return restore();
        }
        // An arrow or the thumb: the down arrow (3) mirrored.
        int32_t x0 = x, x1 = x + w, x2 = x + w, x3 = x;
        if (part == 3)
        {
            x0 = x + w;
            x1 = x;
            x2 = x;
            x3 = x + w;
        }
        if (part == under && hot != 0)
        {
            if (kOverRace(app) != 0)
                return artQuad(app, ts, at, x0, y, x1, y, x2, y + h, x3, y + h,
                               kHotPartQuads + 0xc0 * u32(kind) + 0x20 * u32(part), alpha);
            if (half(app, shapeAt(kind, part, 0)) == -1)
                return true;
            return namedAt<shapes::drawArt, 0>(app, ts, at, &r, u32(int32_t(half(app, shapeAt(kind, part, 0)))), u32(x), u32(y), alpha);
        }
        if (kOverRace(app) != 0)
            return artQuad(app, ts, at, x0, y, x1, y, x2, y + h, x3, y + h, places, alpha);
        if (half(app, shapeAt(kind, part, 1)) == -1)
            return true;
        return namedAt<shapes::drawArt, 0>(app, ts, at, &r, u32(int32_t(half(app, shapeAt(kind, part, 1)))), u32(x), u32(y), alpha);
    };
    if (!sliding && slider(app, item).shownScreen == slider(app, item).screen)
        for (int32_t part = partsOf(app, slider(app, item).kind) - 1; part >= 0; --part)
            if (!drawPart(part))
                return {};
    // Its texts: its labels by it, its title by its alignment, its value as a
    // percentage.
    const Slider& s = slider(app, item);
    const int32_t boxX = s.boxX, boxY = s.boxY, boxWidth = s.boxWidth, boxHeight = s.boxHeight;
    const int32_t middleX = boxX + halved(boxWidth), middleY = boxY + halved(boxHeight);
    if (!sliding && (s.labels[0] != 0 || s.labels[1] != 0 || s.labels[2] != 0))
    {
        if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, 0x12, 0x12))
            return {};
        labelsShown = true;
        const int32_t y = slider(app, item).labelsBelow == 1 ? boxY + boxHeight : boxY - textHeight;
        const Slider& t = slider(app, item);
        if (t.labels[0] != 0
            && !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, t.labels[0], u32(int32_t(t.parts[2].x)), u32(y), 0x12, 0, colour))
            return {};
        if (slider(app, item).labels[1] != 0
            && !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, slider(app, item).labels[1], u32(middleX), u32(y), 0x12, 2, colour))
            return {};
        const Slider& u = slider(app, item);
        if (u.labels[2] != 0
            && !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, u.labels[2], u32(int32_t(u.parts[3].x) + u.parts[3].width), u32(y), 0x12, 1, colour))
            return {};
    }
    if (slider(app, item).title != 0)
    {
        if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, 0x12, 0x12))
            return {};
        const uint8_t alignment = slider(app, item).alignment <= 4 ? slider(app, item).alignment : 0;
        int32_t x, y, align;
        if (alignment == 3)
        {
            y = boxY + boxHeight;
            if (labelsShown && slider(app, item).labelsBelow == 1)
                y += textHeight;
            align = 2;
            x = middleX;
        }
        else if (alignment == 2)
        {
            y = boxY - textHeight;
            if (labelsShown && slider(app, item).labelsBelow == 0)
                y -= textHeight;
            align = 2;
            x = middleX;
        }
        else if (alignment == 1)
        {
            y = middleY - halved(textHeight);
            x = boxX + boxWidth + 0xa;
            align = 0;
        }
        else if (alignment == 4)
        {
            x = slider(app, item).titleX;
            y = slider(app, item).titleY;
            align = 0;
        }
        else if (kOverRace(app) == 0 && slider(app, item).ownPlace != 0)
        {
            const int32_t row = slider(app, item).row;
            y = int32_t(word(app, kRowsY + 4 * u32(row)));
            x = int32_t(word(app, kRowsX + 4 * u32(row)) + 0xb);
            align = 0;
        }
        else
        {
            y = middleY - halved(textHeight);
            x = boxX - 0xa;
            align = 1;
        }
        Slider& t = slider(app, item);
        t.screen = t.shownScreen;
        if (!sliding || t.alignment == 0)
        {
            int32_t at_ = x;
            if (kOverRace(app) == 0 && t.alignment == 0)
            {
                if (kScreensSliding(app) != 0)
                {
                    // Slid in from the right as its screen comes in.
                    const int32_t by = int32_t(u32(int32_t(t.y)) - word(app, kRowsY) + u32(kScreensSliding(app)) - 0x12c);
                    if (by > 0x64)
                        at_ = 0x280;
                    else if (by > 0)
                        at_ = x + by;
                    t.screen = int16_t(t.shownScreen + 2);
                }
                else if (t.slide != 0)
                {
                    // Slid in by its own count, 0x14 a frame.
                    t.screen = int16_t(t.shownScreen + 2);
                    if ((kRenderFlags(app) & 2) != 0)
                        t.slide = 0;
                    t.slide = int16_t(t.slide - 0x14);
                    if (t.slide <= 0)
                    {
                        if (t.slide < 0)
                            t.slide = 0;
                        t.screen = t.shownScreen;
                    }
                    at_ += t.slide < 0x64 ? int32_t(t.slide) : 0x64;
                }
            }
            if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, slider(app, item).title, u32(at_), u32(y), 0x12, u32(align), colour))
                return {};
        }
    }
    if (!sliding && slider(app, item).percent == 1)
    {
        const int32_t y = middleY - halved(textHeight);
        const std::optional<Float> v = snappedAt(app, ts, at, kHundredth, slider(app, item).values[0]);
        if (!v)
            return {};
        Float p = fp::sub(Float(kPercentFrom(app)), *v);
        p = -p;
        p = fp::mul(p, Float(kPercentScale(app)));
        p = fp::mul(p, Float(kPercentScale2(app)));
        const float scaled = float(p);
        st.set(Float(0.0), Float(scaled));
        const float rounded =
            float(fp::add(Float(scaled), Float(st.c0() || st.c3() ? kRoundUp(app) : kRoundDown(app))));
        const int32_t n = chopped<int32_t>(ts, Float(rounded));
        if (!format(app, ts, at, {text, kPercent, u32(n), 0x25})
            || !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, text, u32(boxX + boxWidth + 0xa), u32(y), 0x12, 0, colour))
            return {};
    }
    return 0;
}

/* 0x46b3a0's: a two-axis slider drawn: its parts from the last -- its thumbs
 * across and up and its bar's thumb as quads of the art on the 3D card or
 * shapes on a 2D screen, the part under the pointer lit where `hot` asks for
 * it; its area a frame of shaded quads, its art clipped at each corner on a
 * 2D screen --, its title by its alignment, its values as percentages. */
Result twoAxisDrawn(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    FpuStatus st;
    const u32 ebp = esp - 0x14, at = ebp - 0xa0, text = ebp - 0xa0;
    for (u32 i = 0; i < 0x40; i += 4)
        setWord(app, text + i, word(app, kTwoAxisBlank + i));
    // The frame's words the code keeps a part's place and size in, which its
    // percentages are placed by when it has no title (as the last part left
    // them); the viewport kept.
    const u32 placeX = ebp - 0x20, placeY = ebp - 0x1c, placeWidth = ebp - 0x30;
    const u32 keptLeft = ebp - 0x50, keptTop = ebp - 0x5c, keptWidth = ebp - 0x60, keptHeight = ebp - 0x4c;
    const int32_t textHeight = 0xe;
    greyed(app, item);
    int32_t under = -1;
    u32 colour = 0xff649dfd;
    if (hot != 0)
    {
        under = partUnder(app, item);
        colour = 0xffe440;
    }
    u32 r;
    const auto quadsOf = [&](int32_t part, bool lit) {
        return (lit ? kHotPartQuads : kPartQuads) + 0xc0 * u32(int32_t(slider(app, item).kind)) + 0x20 * u32(part);
    };
    const auto shapeDrawn = [&](int32_t shape) {
        return coreAt<0x4d7990, shapes::drawShapeOf, 0>(app, ts, at, &r, u32(shape), word(app, placeX), word(app, placeY));
    };
    const u32 W = u32(kScreenWidth(app)), H = u32(kScreenHeight(app));
    const auto drawPart = [&](int32_t part) -> bool {
        const SliderPart p = slider(app, item).parts[part];
        setWord(app, placeX, u32(int32_t(p.x)));
        setWord(app, placeY, u32(int32_t(p.y)));
        setWord(app, placeWidth, u32(int32_t(p.width)));
        const int32_t x = p.x, y = p.y, w = p.width, h = p.height;
        const int32_t kind = slider(app, item).kind;
        if (u32(part) > 5)
            return true;
        if (part < 4)
        {
            // A thumb across (0, 1) or up (2, 3; mirrored).
            const bool lit = part == under;
            if (kOverRace(app) != 0)
            {
                if (part < 2)
                    return artQuad(app, ts, at, x, y, x + w, y, x + w, y + h, x, y + h, quadsOf(part, lit), 0xffffffff);
                return artQuad(app, ts, at, x + w, y, x, y, x, y + h, x + w, y + h, quadsOf(part, lit), 0xffffffff);
            }
            const int16_t shape = half(app, shapeAt(kind, part, lit ? 0 : 1));
            if (shape == -1)
                return true;
            return shapeDrawn(shape);
        }
        if (part == 4)
        {
            // The bar's thumb.
            if (kOverRace(app) != 0)
                return artQuad(app, ts, at, x, y, x + w, y, x + w, y + h, x, y + h, quadsOf(4, false), 0xffffffff);
            const int16_t shape = half(app, shapeAt(kind, 4, 1));
            if (shape == -1)
                return true;
            return shapeDrawn(shape);
        }
        // The area: its art, a frame of shaded quads round it.
        const Slider& s = slider(app, item);
        const SliderPart& area = s.parts[5];
        if (kOverRace(app) != 0)
        {
            const int32_t hw = halved(w), middle = x + hw;
            if (!artQuad(app, ts, at, x, y, middle, y, middle, y + h, x, y + h, quadsOf(5, false), 0xffffffff)
                || !artQuad(app, ts, at, middle + hw, y, middle, y, middle, y + h, middle + hw, y + h,
                            quadsOf(5, false), 0xffffffff))
                return false;
            const u32 a = 0x98900010, b = 0x98186a02;
            const int32_t top = slider(app, item).parts[5].y + 4;
            const int32_t foot = slider(app, item).parts[5].y + slider(app, item).parts[5].height - 4;
            const Slider& t = slider(app, item);
            const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width - 3;
            const int32_t e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
            const int32_t areaLeft = t.parts[5].x + 0x10, areaRight = t.parts[5].x + t.parts[5].width - 0x10;
            return colourQuad(app, ts, at, areaLeft, top, e0, top, e0, foot, areaLeft, foot, a, a, a, a)
                   && colourQuad(app, ts, at, e0, top, e1, top, e1, foot, e0, foot, b, b, b, b)
                   && colourQuad(app, ts, at, e1, top, e3, top, e3, foot, e1, foot, a, a, a, a)
                   && colourQuad(app, ts, at, e3, top, e2, top, e2, foot, e3, foot, b, b, b, b)
                   && colourQuad(app, ts, at, e2, top, areaRight, top, areaRight, foot, e2, foot, a, a, a, a);
        }
        if ((kRenderFlags(app) & 2) != 0)
        {
            const int16_t shape = half(app, shapeAt(kind, 5, 1));
            if (shape != -1 && !shapeDrawn(shape))
                return false;
            const u32 a = 0xff900010, b = 0xff186a02;
            const Slider& t = slider(app, item);
            const int32_t top = t.parts[5].y + 3, foot = t.parts[5].y + t.parts[5].height - 4;
            const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width - 3;
            const int32_t e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
            const int32_t areaLeft = t.parts[5].x + 0x10, areaRight = t.parts[5].x + t.parts[5].width - 0x11;
            return colourQuad(app, ts, at, areaLeft, top, e0, top, e0, foot, areaLeft, foot, a, a, a, a)
                   && colourQuad(app, ts, at, e0, top, e1, top, e1, foot, e0, foot, b, b, b, b)
                   && colourQuad(app, ts, at, e1, top, e3, top, e3, foot, e1, foot, a, a, a, a)
                   && colourQuad(app, ts, at, e3, top, e2, top, e2, foot, e3, foot, b, b, b, b)
                   && colourQuad(app, ts, at, e2, top, areaRight, top, areaRight, foot, e2, foot, a, a, a, a);
        }
        const int16_t shapeB = half(app, shapeAt(kind, 5, 1)), shapeA = half(app, shapeAt(kind, 5, 0));
        if (shapeB == -1 || shapeA == -1)
            return true;
        if (!viewportKept(app, ts, at, keptLeft, keptTop, keptWidth, keptHeight))
            return false;
        // The art clipped to each stretch of the area's top between the thumbs.
        const int32_t tall = h + 4, top = y - 2;
        (void)area;
        const auto stretch = [&](int32_t from, int32_t width, int16_t shape) {
            return clipped(app, ts, at, onScreen(ts, u32(from) * W, kPerWidth2(app)),
                           onScreen(ts, H * u32(top), kPerHeight2(app)),
                           onScreen(ts, u32(width) * W, kPerWidth2(app)),
                           onScreen(ts, u32(tall) * H, kPerHeight2(app)))
                   && shapeDrawn(shape);
        };
        const Slider& t = slider(app, item);
        const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width + 3;
        const int32_t e1l = t.parts[1].x + t.parts[1].width - 3, e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
        return stretch(x, e0 - x, shapeA) && stretch(e0, e1 - e0, shapeB) && stretch(e1l, e3 - e1l, shapeA)
               && stretch(e3, e2 - e3, shapeB) && stretch(e2, x + w - e2, shapeA)
               && clipped(app, ts, at, word(app, keptLeft), word(app, keptTop), word(app, keptWidth),
                          word(app, keptHeight));
    };
    for (int32_t part = partsOf(app, slider(app, item).kind) - 1; part >= 0; --part)
        if (!drawPart(part))
            return {};
    // Its title.
    if (slider(app, item).title != 0)
    {
        const Slider& s = slider(app, item);
        setWord(app, placeX, u32(int32_t(s.boxX)));
        setWord(app, placeY, u32(int32_t(s.boxY)));
        setWord(app, placeWidth, u32(int32_t(s.boxWidth)));
        const int32_t boxX = s.boxX, boxY = s.boxY, boxWidth = s.boxWidth, boxHeight = s.boxHeight;
        const int32_t middleX = boxX + halved(boxWidth), middleY = boxY + halved(boxHeight);
        if (!keepNamedAt<text::selectFontOf>(app, ts, at, &r, 0x12, 0x12))
            return {};
        const uint8_t alignment = slider(app, item).alignment <= 4 ? slider(app, item).alignment : 0;
        int32_t x, y, align;
        if (alignment == 3)
        {
            y = boxY + boxHeight;
            x = middleX;
            align = 2;
        }
        else if (alignment == 2)
        {
            y = boxY - textHeight;
            x = middleX;
            align = 2;
        }
        else if (alignment == 4)
        {
            x = slider(app, item).titleX;
            y = slider(app, item).titleY;
            align = 0;
        }
        else
        {
            y = middleY - halved(textHeight);
            x = boxX - 0xa;
            align = 1;
        }
        if (!namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, slider(app, item).title, u32(x), u32(y), 0x12, u32(align), colour))
            return {};
    }
    // Its values, as percentages.
    float percents[2];
    for (int axis = 0; axis < 2; ++axis)
    {
        const std::optional<Float> v = snappedAt(app, ts, at, kHundredth, slider(app, item).values[axis]);
        if (!v)
            return {};
        const float scaled = float(fp::mul(*v, Float(kTwoAxisScale(app))));
        st.set(Float(0.0), Float(scaled));
        percents[axis] = float(fp::add(
            Float(scaled), Float(st.c0() || st.c3() ? kTwoAxisUp(app) : kTwoAxisDown(app))));
    }
    const int32_t across = chopped<int32_t>(ts, Float(percents[0]));
    const int32_t up = chopped<int32_t>(ts, Float(percents[1]));
    if (!format(app, ts, at, {text, kTwoPercents, u32(up), 0x25, u32(across), 0x25})
        || !namedAt<text::menuTextDrawn, 2>(app, ts, at, &r, text, word(app, placeX) + word(app, placeWidth) + 0xa, word(app, placeY), 0x12, 0, colour))
        return {};
    return 0;
}

}  // namespace

/* 0x46c950's: the slider `item` set to `value` -- a two-axis one's axis
 * `axis`; none of kind 1. */
Result sliderSetValueOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 value, u32 axis)
{
    const u32 at = esp - 8;
    const uint16_t kind = uint16_t(slider(app, item).kind);
    Result r;
    if (kind == 1)
        r = int32_t(item);
    else if (kind == 2)
        r = twoAxisValueSet(app, ts, argsBelow(app, at, {value, axis}), item, item);
    else
        r = valueSet(app, ts, argsBelow(app, at, {value, 0}), item);
    return r;
}

/* 0x46cae0's: the slider `item` drawn, worked by the mouse first where `hot`:
 * kind 2 a two-axis one, kind 1 none.  Greyed as the menus grey their
 * controls. */
Result sliderDrawOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot)
{
    const u32 at = esp - 0x10;
    greyed(app, item);
    const uint16_t kind = uint16_t(slider(app, item).kind);
    Result r = 0;
    if (kind == 2)
    {
        if (hot != 0 && !twoAxisTracked(app, ts, at - 4, item))
            return {};
        r = twoAxisDrawn(app, ts, at - 4, item, hot);
    }
    else if (kind != 1)
    {
        if (hot != 0 && !sliderTracked(app, ts, at - 4, item))
            return {};
        r = sliderDrawn(app, ts, at - 4, item, hot);
    }
    return r;
}

/* ---- The thunks ---- */

/* 0x4692c0: the part of the slider eax under the pointer, or -1. */
nfs::u32 sliderPartUnderPointer(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(partUnder(app, e.eax)));
}

/* 0x469340 (stdcall 8): where the slider eax's thumb may go: the start to
 * [ebx], the end to [ecx], the length to the stack's first pointer, the place
 * to its second. */
nfs::u32 sliderThumbPlace(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(thumbPlaced(app, ts, e.eax, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1))), 8);
}

/* 0x4693c0 (stdcall 8): the stack's value snapped to the stack's step, onto
 * the x87's stack. */
nfs::u32 snapToStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const std::optional<Float> r = snapped(app, ts, e.esp);
    if (!r)
        return args.eax;
    args.floatSet = true;
    args.floatResult = *r;
    return e.back(ts, args, e.eax, 8);
}

/* 0x469480 (stdcall 8): the slider eax set to the stack's value. */
nfs::u32 setSliderValue(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, valueSet(app, ts, e.esp, e.eax), 8);
}

/* 0x469530, 0x46afe0 (stdcall 8): the slider eax's arrows' step, of axis
 * [esp+8] for a two-axis one. */
nfs::u32 setSliderStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, arrowStepSet(app, ts, e.eax, e.arg(app, 0), 0, e.eax), 8);
}

nfs::u32 setTwoAxisStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, arrowStepSet(app, ts, e.eax, e.arg(app, 0), int32_t(e.arg(app, 1)), e.eax), 8);
}

/* 0x469590: the slider eax dragged to dx along its track. */
nfs::u32 dragSliderTo(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, draggedTo(app, ts, e.esp, e.eax, int16_t(e.edx)));
}

/* 0x469680, 0x46b3a0: the slider, the two-axis slider eax drawn; the part
 * under the pointer lit where edx. */
nfs::u32 drawSlider(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderDrawn(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 drawTwoAxis(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisDrawn(app, ts, e.esp, e.eax, e.edx));
}

/* 0x46a5f0, 0x46c320: the slider, the two-axis slider eax as the mouse works
 * it. */
nfs::u32 trackSlider(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderTracked(app, ts, e.esp, e.eax));
}

nfs::u32 trackTwoAxis(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisTracked(app, ts, e.esp, e.eax));
}

/* 0x46a860, 0x46c4a0: key bx on the slider, the two-axis slider eax. */
nfs::u32 sliderKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderKeyed(app, ts, e.esp, e.eax, e.ebx));
}

nfs::u32 twoAxisKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisKeyed(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x46a980, 0x46c640: the slider, the two-axis slider eax's parts placed. */
nfs::u32 placeSliderParts(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, sliderPlaced(app, e.eax));
}

nfs::u32 placeTwoAxisParts(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, twoAxisPlacedParts(app, e.eax));
}

/* 0x46ab10, 0x46c820: the slider, the two-axis slider eax set up. */
nfs::u32 setUpSlider(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderSetUp(app, ts, e.esp, e.eax));
}

nfs::u32 setUpTwoAxis(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisSetUp(app, ts, e.esp, e.eax));
}

/* 0x46abb0, 0x46c8b0: the box the pointer finds the slider, the two-axis
 * slider eax in. */
nfs::u32 sliderBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, sliderBoxed(app, e.eax));
}

nfs::u32 twoAxisBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, twoAxisBoxed(app, e.eax));
}

/* 0x46ac40 (stdcall 0xc): where thumb ebx of the two-axis slider eax may go
 * along axis edx: the start to [ecx], the end, the length and the place to
 * the stack's pointers. */
nfs::u32 twoAxisThumbPlace(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args,
           u32(twoAxisPlaced(app, ts, e.eax, int32_t(e.edx), e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1),
                               e.arg(app, 2))),
           0xc);
}

/* 0x46add0 (stdcall 8): axis [esp+8] of the two-axis slider eax set to the
 * stack's value. */
nfs::u32 setTwoAxisValue(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisValueSet(app, ts, e.esp, e.eax, e.eax), 8);
}

/* 0x46b040: axis ebx of the two-axis slider eax dragged to dx. */
nfs::u32 dragTwoAxisTo(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, twoAxisDragged(app, ts, e.esp, e.eax, int16_t(e.edx), int32_t(e.ebx)));
}

/* 0x469080: the slider eax's parts' art and sizes. */
nfs::u32 sliderPartSizes(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, partSizes(app, ts, e.esp, e.eax));
}

/* 0x4d5e40: the menus' shape named eax, found or made. */
nfs::u32 sliderShape(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, shapeOf(app, ts, e.esp, e.eax, e.ebx));
}

/* 0x46c950 (stdcall 8): the slider eax set to the stack's value -- a
 * two-axis one's axis the stack's second word; none of kind 1. */
nfs::u32 sliderSetValue(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderSetValueOf(app, ts, e.esp, e.eax, e.arg(app, 0), e.arg(app, 1)), 8);
}

/* 0x46cae0: the slider eax drawn, worked by the mouse first where edx: kind 2
 * a two-axis one, kind 1 none.  Greyed as the menus grey their controls. */
nfs::u32 sliderDraw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sliderDrawOf(app, ts, e.esp, e.eax, e.edx));
}

/* 0x46cb80: key cx on the slider eax. */
nfs::u32 sliderWidgetKey(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 item = e.eax, at = e.esp - 0xc;
    const uint16_t kind = uint16_t(slider(app, item).kind);
    const u32 key = u32(int32_t(int16_t(e.ecx)));
    Result r = 0;
    if (kind == 2)
        r = twoAxisKeyed(app, ts, at - 4, item, key);
    else if (kind != 1)
        r = sliderKeyed(app, ts, at - 4, item, key);
    return e.finish(ts, args, r);
}

/* 0x46cbe0: the slider eax laid out. */
nfs::u32 sliderWidgetLayout(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const uint16_t kind = uint16_t(slider(app, e.eax).kind);
    return e.back(ts, args, kind == 2 ? twoAxisBoxed(app, e.eax) : kind == 1 ? e.eax : sliderBoxed(app, e.eax));
}

/* 0x46cc10: the slider eax hidden: its screen put back. */
nfs::u32 sliderWidgetHidden(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Slider& s = slider(app, e.eax);
    const uint16_t kind = uint16_t(s.kind);
    u32 eax = (e.eax & 0xffff0000) | kind;
    if (kind != 1 && kind != 2)
    {
        s.screen = s.shownScreen;
        eax = (e.eax & 0xffff0000) | uint16_t(s.shownScreen);
    }
    return e.back(ts, args, eax);
}

/* 0x46cc50: the slider eax shown: its parts' art and sizes, set up (a kind
 * past 5 made a slider's), its value set. */
nfs::u32 sliderWidgetShown(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 item = e.eax, at = e.esp - 8;
    const uint16_t kind = uint16_t(slider(app, item).kind);
    if (kind > 5)
        slider(app, item).kind = 0;
    if (kind == 2)
    {
        if (!partSizes(app, ts, at - 4, item) || !twoAxisSetUp(app, ts, at - 4, item))
            return args.eax;
    }
    else if (kind != 1)
    {
        if (!partSizes(app, ts, at - 4, item) || !sliderSetUp(app, ts, at - 4, item))
            return args.eax;
    }
    const u32 value = slider(app, item).values[0];
    const u32 callee = argsBelow(app, at, {value, 0});
    const uint16_t now = uint16_t(slider(app, item).kind);
    Result r;
    if (now == 1)
        r = int32_t(item);
    else if (now == 2)
        r = twoAxisValueSet(app, ts, argsBelow(app, callee - 8, {value, 0}), item, item);
    else
        r = valueSet(app, ts, argsBelow(app, callee - 8, {value, 0}), item);
    return e.finish(ts, args, r);
}

}  // namespace fields
}  // namespace port
}  // namespace nfs3hp
