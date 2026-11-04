#include "port_fields.h"
#include "port_draw2d.h"
#include "port_text.h"

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
 * Each function is a typed core under its own name and an adapter under the
 * game's.  The adapter takes the arguments from the registers and the stack
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
 * the menu's click as the C++ adapters they are (as the game's code calls
 * their addresses); what is not rewritten -- the menu loop's, the dialogs',
 * the car records', the pointer's, the runtime's, EA's, the render state and
 * the clipper -- by address, from where the game's code had its stack, its
 * frame's words where the game's code points them.  tools/game_checks holds
 * each adapter against the game's code (fields.inc).
 *
 * The x87's arithmetic is the game's to the bit: each product and sum through
 * cpu.fpu under the control word as it is, a value stored as a float where
 * the code stores one, the compares leaving the status word as the code's. */
namespace nfs3hp
{
namespace port
{
/* Other files' adapters, declared by their type: tools/game_checks/generate.py
 * finds a native's file by its `void name(` definition, which a declaration
 * written so would be taken for. */
using Adapter = void(App* app, CPU& cpu);
/* port_sound_effects.cpp's menu click (0x4181d0). */
Adapter playMenuEffect;

namespace shapes
{
/* port_shapes.cpp's, called as the game's code calls them: a menu shape by
 * name, its textures made, its size, an art record drawn, a shape drawn. */
Adapter shapeByName, artShapeTextures, shapeSize, drawArtShape, drawShape;  // 0x4d5db0, 0x4d6390, 0x4d66f0, 0x4d7160, 0x4d7990
}  // namespace shapes

namespace fields
{

namespace
{

using Float = x86::Float;

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
    reg32 label;        // +0x3c
    int16_t player;     // +0x40: 0 or 1
    int16_t unknown42;
    int32_t chosen;     // +0x44
    int32_t highlighted;  // +0x48
    int32_t count;      // +0x4c
    uint32_t colour;    // +0x50: the chosen one's, ARGB
    reg32 car;          // +0x54: the player's car's variable
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
    reg32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t unknown43;
    int32_t least, most;  // +0x44
    uint8_t unitsShown;   // +0x4c
    uint8_t unknown4d[3];
    reg32 unit, units;  // +0x50
    reg32 value;        // +0x58: where it is kept
    int32_t cursor;     // +0x5c
    char typed[12];     // +0x60
    reg32 changed;      // +0x6c: told when it changes
};

/* edstring: a text typed into a buffer of its own (`maxLength`), copied
 * where it is kept when Enter takes it; its cursor blinks. */
struct StringField : Item
{
    reg32 label;        // +0x3c
    int16_t editing;    // +0x40
    int16_t blink;      // +0x42
    reg32 target;       // +0x44
    reg32 buffer;       // +0x48
    int16_t maxLength;  // +0x4c
    int16_t unknown4e;
    reg32 changed;      // +0x50
};

/* edchar: a byte. */
struct CharField : Item
{
    reg32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t least, most;  // +0x43
    uint8_t unitsShown;   // +0x45
    uint8_t unknown46[2];
    reg32 unit, units;  // +0x48
    reg32 value;        // +0x50
    int32_t cursor;     // +0x54
    char typed[8];      // +0x58
    reg32 changed;      // +0x60
};

/* edfloat and edfixed: a float, a 16.16 number; shown with `decimals`. */
struct FloatField : Item
{
    reg32 label;        // +0x3c
    int16_t editing;    // +0x40
    uint8_t checked;    // +0x42
    uint8_t unknown43;
    reg32 least, most;  // +0x44: a float's bits (edfloat), an integer (edfixed)
    uint8_t unitsShown;   // +0x4c
    uint8_t unknown4d[3];
    reg32 unit, units;  // +0x50
    reg32 value;        // +0x58
    int32_t cursor;     // +0x5c
    int32_t decimals;   // +0x60
    char typed[16];     // +0x64
    reg32 changed;      // +0x74
};

/* List: `rows` of its lines shown from `first`, `chosen` lit; a line 8 bytes
 * (its text first, its first byte's top bit marking it). */
struct ListBox : Item
{
    int16_t width;      // +0x3c
    int16_t rows;       // +0x3e
    int16_t chosen;     // +0x40
    int16_t first;      // +0x42
    reg32 choose;       // +0x44: told of a line chosen by the pointer
    reg32 lines;        // +0x48
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
// The players' cars, the custom colours kept as they are (not 0).
constexpr reg32 kFirstCar = 0x6fd2bc, kSecondCar = 0x6fd328;
constexpr Var<int32_t> kKeepCustomColours{0x6fd3a0};
// A car's record's colours: the chosen (a dword a player), how many less
// one, the colours (4 bytes each), the custom one (a player's).
constexpr reg32 kChosenColours = 0x32c, kLastColour = 0x334, kColours = 0x338, kCustomColours = 0x378;
// The pointer.
constexpr Var<int32_t> kPointerX{0x55e5e4}, kPointerY{0x55e5e8};
// The cursor's colour: the 3D card's font's, the 2D screens'.
constexpr Var<reg32> kCursorColour3d{0x561b38}, kCursorColour{0x559438};
// The 3D card's font: its colour, whether only measured, which font; its
// characters' metrics by number.
constexpr Var<reg32> kTextColour{0x561b34};
constexpr Var<reg32> kMeasuring{0x561b3c};
constexpr Var<uint8_t> kFont3d{0x840184};
constexpr reg32 kGlyphWidth3d = 0x83fc40, kGlyphHeight3d = 0x83fe00, kTextureY3d = 0x83f700, kTextureX3d = 0x83f8c0;
// The 2D screens' font: which (0x24 the accented one), its colour, its
// texture, the nudge, its characters' squares.
constexpr Var<int32_t> kScreenFont{0x664650};
constexpr Var<reg32> kScreenTextColour{0x559434}, kFontTexture{0x8b7624};
constexpr Var<int32_t> kNudge{0x563aa0};
constexpr reg32 kGlyphU = 0x8b7858, kGlyphV = 0x8b774a, kGlyphWidth = 0x8b7966, kGlyphHeight = 0x8b763c;
// The 2D screens' quad (0x20 bytes a corner), its depth and w.
constexpr reg32 kQuad = 0x563ab0;
constexpr Var<float> kScreenDepth{0x563aa8}, kScreenW{0x563aac};
constexpr reg32 kNearDepth = 0x37800080, kNearW = 0x3f7fff00;
// The clip's bounds, compared as the floats' bits: w, left, bottom, top, right.
constexpr Var<int32_t> kClipFar{0x7d34f8}, kClipLeft{0x7d34fc}, kClipBottom{0x7d3500}, kClipTop{0x7d3504};
constexpr Var<int32_t> kClipRight{0x7d350c};
// A list's line's height.
constexpr Var<int32_t> kLineHeight{0x678a40};
// The dialog's text and its lines and buttons.
constexpr reg32 kDialogText = 0x559894, kDialogLines = 0x559994, kDialogButtons = 0x55999c;
// The fields' data the code copies into its frame (four blocks of 0x100).
constexpr reg32 kIntBlank = 0x4576e0, kCharBlank = 0x4577e0, kFloatBlank = 0x4578e0, kFixedBlank = 0x4579e0;
// Their constants: 0.4, 1/65536 twice, 65536.
constexpr Var<double> kSpacing{0x5395f8}, kFixedUnit{0x539b7c}, kFixedUnit2{0x539b84}, kFixedScale{0x539b8c};

// The texts: "%s", "%d %s", "%d", the ranges', "_", "", "%.*f %s", "%.*f",
// "%f", "%.6f", "%s...", "%c ", "Ag", "<%s>"; the arrows' shapes' names; the
// screens' items.
constexpr reg32 kS = 0x539b0c, kDS = 0x539b10, kD = 0x539b18, kIntRange = 0x539b1c, kUnderline = 0x539b38;
constexpr reg32 kNoName = 0x539b3c, kFS = 0x539b40, kF = 0x539b48, kFloatRange = 0x539b50, kScanF = 0x539b6c;
constexpr reg32 kF6 = 0x539b70, kEllipsis = 0x539ab0, kCharSpace = 0x5395f4, kAg = 0x53aba8;
constexpr reg32 kMarked = 0x53abac, kPlain = 0x53abb4;
constexpr reg32 kArrowName = 0x539ab8, kBoxName = 0x539ac0;
constexpr reg32 kFirstCarItem = 0x539ac8, kSecondCarItem = 0x539adc, kColourEdit = 0x539af0;
constexpr reg32 kNoCarText = 0x537690;

/* What is not rewritten, called by address: the menu loop's (whether an item
 * is on the screen shown now, a screen's item by name, a screen by name, an
 * item changed, the colour set's state), the dialog's question, the pointer
 * (placed, in a box, the line under it), the car records', the runtime's,
 * EA's memory, the text's height, the render state, the clippers. */
constexpr reg32 kShownNow = 0x44a660, kFindItem = 0x442a40, kMenuNamed = 0x44aeb0, kItemChanged = 0x44a470;
constexpr reg32 kColourSetState = 0x449df0, kQuestion = 0x4446d0;
constexpr reg32 kSetPointer = 0x49bfc0, kPointerInBox = 0x49c1f0, kLineAtPointer = 0x49c1c0;
constexpr reg32 kCarRecord = 0x439a50, kFecText = 0x439af0, kCarFecPath = 0x43b680, kWriteCarFec = 0x43b4f0;
constexpr reg32 kRgbToHsv = 0x49c880, kSaveSettings = 0x4dcbc0;
constexpr reg32 kSprintf = 0x4df690, kSscanf = 0x4f0548, kStrncpy = 0x4e0e30, kMemset = 0x4e0640;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890;
constexpr reg32 kFontHeight = 0x4f0840, kTextExtent = 0x4f0c90, kCursorQuad = 0x4d1120;
constexpr reg32 kRenderState = 0x431900, kClipTriangle = 0x4c11b0, kClipQuad = 0x4c12e0;

/* The port's own, called as the game's code calls their addresses. */
constexpr guest::Native kSelectFont = text::selectFont, kTextWidth = text::textWidth;         // 0x452100, 0x452020
constexpr guest::Native kDrawMenuText = text::drawMenuText, kDrawTextAt = text::drawTextAt;   // 0x452190, 0x451590
constexpr guest::Native kSetTextColour = text::setTextColour, kLanguageText = text::languageText;  // 0x4513c0, 0x4d1850
constexpr guest::Native kFlushFonts = text::flushFonts, kDrawCharacter = text::drawCharacter;  // 0x4d1340, 0x4d06f0
constexpr guest::Native kToScreenX = text::toScreenX, kToScreenY = text::toScreenY;  // 0x451400, 0x451440
constexpr guest::Native kGradient = draw2d::gradientRectangle, kFilled = draw2d::filledRectangle;  // 0x4d8550, 0x4d8910
constexpr guest::Native kFrame = draw2d::frame, kFieldBox = draw2d::drawFieldBox;  // 0x4d8930, 0x4d8ac0
constexpr guest::Native kAccented = draw2d::accentedCharacter, kScreenCharacter = draw2d::drawScreenCharacter;
constexpr guest::Native kShapeByName = shapes::shapeByName, kDrawShape = shapes::drawShape;  // 0x4d5db0, 0x4d7990
constexpr guest::Native kMenuClick = playMenuEffect;  // 0x4181d0

constexpr reg32 kEnter = 0xd, kEscape = 0x1b, kBackspace = 8, kDelete = 0x5300;
constexpr reg32 kUp = 0x4800, kDown = 0x5000, kLeft = 0x4b00, kRight = 0x4d00;

reg32 word(App* app, reg32 at)
{
    return app->getMemory<reg32>(at);
}

void setWord(App* app, reg32 at, reg32 value)
{
    app->getMemory<reg32>(at) = value;
}

int16_t half(App* app, reg32 at)
{
    return app->getMemory<int16_t>(at);
}

void setHalf(App* app, reg32 at, int32_t value)
{
    app->getMemory<uint16_t>(at) = uint16_t(value);
}

uint8_t byteAt(App* app, reg32 at)
{
    return app->getMemory<uint8_t>(at);
}

void setByte(App* app, reg32 at, uint8_t value)
{
    app->getMemory<uint8_t>(at) = value;
}

float floatAt(App* app, reg32 at)
{
    return app->getMemory<float>(at);
}

void setFloat(App* app, reg32 at, float value)
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
    reg32 lo, hi;
};

Words wordsOf(double d)
{
    uint64_t b;
    std::memcpy(&b, &d, 8);
    return {reg32(b), reg32(b >> 32)};
}

/* sar 0x1f, sub, sar 1: halved toward 0. */
int32_t halved(int32_t v)
{
    return int32_t(uint32_t(v) - uint32_t(v >> 31)) >> 1;
}

/* repne scasb's length of the text at `s`. */
reg32 length(App* app, reg32 s)
{
    reg32 n = 0;
    while (byteAt(app, s + n) != 0)
        ++n;
    return n;
}

/* The registers a function pushes on entry, written below `esp` in the order
 * it pushes them -- what is read of its frame past its locals (a format's
 * words logged, a text written past a buffer) finds them there --, and popped
 * back from there on its way out. */
void pushed(App* app, reg32 esp, std::initializer_list<reg32> values)
{
    for (const reg32 value : values)
        setWord(app, esp -= 4, value);
}

void popped(App* app, reg32 esp, std::initializer_list<reg32*> registers)
{
    for (reg32* r : registers)
        *r = word(app, esp -= 4);
}

/* The menu loop's: whether `item`, of widget `type`, is on the screen shown. */
bool shownNow(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 type, reg32& r, reg32 ebx = 0, reg32 ecx = 0)
{
    return callGame(app, cpu, esp, kShownNow, {item, type, ebx, ecx}, {}, r);
}

/* sprintf into `buffer`, its format's words after it. */
bool format(App* app, CPU& cpu, reg32 esp, std::initializer_list<reg32> words)
{
    return callGame(app, cpu, esp, kSprintf, {}, words);
}

/* The dialog's question, its text at kDialogText: an OK. */
bool question(App* app, CPU& cpu, reg32 esp, reg32& r)
{
    return callGame(app, cpu, esp, kQuestion, {2, kDialogLines, 1, kDialogButtons}, {0, 1, 0, 0}, r);
}

/* The clip code of the corner at `at`: 0x10 behind the eye or past the far
 * clip, else 8 above the clip, 4 below, 1 left of it, 2 right of it.  Where
 * the code compared w alone, `farOnly`. */
uint8_t clipCode(App* app, reg32 at, bool& farOnly)
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
reg32 markCorners(App* app, int count)
{
    reg32 ecx = 0;
    for (int corner = 0; corner < count; ++corner)
    {
        bool farOnly;
        const reg32 at = kQuad + 0x20 * reg32(corner);
        setByte(app, at + 0x14, clipCode(app, at, farOnly));
        ecx = reg32(farOnly ? kClipFar(app) : kClipLeft(app));
    }
    return ecx;
}

}  // namespace

/* ======== The car records' colours ======== */

namespace
{

Result colourCount(App* app, CPU& cpu, reg32 esp, reg32 car);
Result colourRgb(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t colour, int32_t player, reg32 out0, reg32 out1,
                 reg32 out2, reg32 out3);
Result customColoursRefreshed(App* app, CPU& cpu, reg32 esp, reg32 eax);

bool carRecordOf(App* app, CPU& cpu, reg32 esp, reg32 car, reg32& rec)
{
    return callGame(app, cpu, esp, kCarRecord, {car}, {}, rec);
}

/* 0x439760's: how many colours car `car` has, its custom one counted. */
Result colourCount(App* app, CPU& cpu, reg32 esp, reg32 car)
{
    const reg32 at = esp - 4;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    return int32_t(word(app, rec + kLastColour) + 1);
}

/* 0x439780's: the number of its custom colour, its last. */
Result customColour(App* app, CPU& cpu, reg32 esp, reg32 car)
{
    const Result n = colourCount(app, cpu, esp - 4 - 4, car);
    if (!n)
        return {};
    return int32_t(reg32(*n) - 1);
}

/* 0x439790's: the colour player `player` chose for it, -1 for no player. */
Result chosenColour(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player)
{
    const reg32 at = esp - 4;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    if (player == 0 || player == 1)
        return int32_t(word(app, rec + reg32(player) * 4 + kChosenColours));
    return -1;
}

/* 0x439ab0's: text `n` of car `car`'s, "" past them or without its record. */
Result carTextOf(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t n)
{
    const reg32 at = esp - 8;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    if (rec == 0)
        return int32_t(kNoCarText);
    const int32_t last = int32_t(half(app, rec + 4)) - 1;
    if (n > last)
        return int32_t(kNoCarText);
    reg32 r;
    if (!callGame(app, cpu, at, kFecText, {rec + 4, reg32(n), 0, reg32(last)}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4397c0's: the name of colour `colour` (its text 0x1e on). */
Result colourName(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t colour)
{
    return carTextOf(app, cpu, esp - 4 - 4, car, int32_t(reg32(colour) + 0x1e));
}

/* 0x43b7f0's: car `car`'s record written to its .fec file: 0, or -1. */
Result colourSaved(App* app, CPU& cpu, reg32 esp, reg32 car)
{
    const reg32 ebp = esp - 0x10, at = ebp - 0x12c, path = ebp - 0x12c;
    HeldEbp held(cpu, ebp);
    reg32 rec, r;
    if (!carRecordOf(app, cpu, at, car, rec) || !callGame(app, cpu, at, kCarFecPath, {car, path, rec, car}, {}, r)
        || !callGame(app, cpu, at, kWriteCarFec, {path, rec, rec, car}, {}, r))
        return {};
    return r == 0 ? 0 : -1;
}

/* 0x4397d0's: player `player`'s colour of car `car` set to `colour`, the
 * record saved and the custom colours taken afresh: 0, or -1. */
Result colourSet(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player, int32_t colour)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    if (player != 0 && player != 1)
        return -1;
    setWord(app, rec + reg32(player) * 4 + kChosenColours, reg32(colour));
    const Result saved = colourSaved(app, cpu, at - 4, car);
    if (!saved || !customColoursRefreshed(app, cpu, at - 4, reg32(*saved)))
        return {};
    return *saved == 0 ? 0 : -1;
}

/* 0x439810's: colour `colour` of car `car` -- its custom one, the player's,
 * for the last -- its four bytes to `out0` to `out3`: 0, or -1 past them. */
Result colourRgb(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t colour, int32_t player, reg32 out0, reg32 out1,
                 reg32 out2, reg32 out3)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    const Result count = colourCount(app, cpu, at - 4, car);
    if (!count)
        return {};
    if (colour > *count)
        return -1;
    reg32 from;
    if (colour == int32_t(reg32(*count) - 1))
    {
        if (player < 0 || player > 1)
            return -1;
        from = rec + reg32(player) * 4 + kCustomColours;
    }
    else
        from = rec + reg32(colour) * 4 + kColours;
    setByte(app, out0, byteAt(app, from));
    setByte(app, out1, byteAt(app, from + 1));
    setByte(app, out2, byteAt(app, from + 2));
    setByte(app, out3, byteAt(app, from + 3));
    return 0;
}

/* 0x4398c0's: player `player`'s custom colour of car `car` set, saved: 0 or
 * -1. */
Result customColourSet(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player, uint8_t b0, uint8_t b1, uint8_t b2,
                       uint8_t b3)
{
    const reg32 at = esp - 8;
    HeldEbp held(cpu, at);
    reg32 rec;
    if (!carRecordOf(app, cpu, at, car, rec))
        return {};
    if (player < 0 || player > 1)
        return -1;
    const reg32 to = rec + reg32(player) * 4 + kCustomColours;
    setByte(app, to, b0);
    setByte(app, to + 1, b1);
    setByte(app, to + 2, b2);
    setByte(app, to + 3, b3);
    const Result saved = colourSaved(app, cpu, at - 4, car);
    if (!saved)
        return {};
    if (*saved != 0)
        return -1;
    if (!customColoursRefreshed(app, cpu, at - 4, 0))
        return {};
    return 0;
}

/* 0x472360's: unless they are kept as they are, both players' cars' chosen
 * colours' bytes taken into the settings (0x6fd307 on, 0x6fd373 on) and the
 * settings saved; eax as it was otherwise. */
Result customColoursRefreshed(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    if (kKeepCustomColours(app) != 0)
        return int32_t(eax);
    const reg32 at = esp - 0x14, inner = at - 0xc - 4;
    HeldEbp held(cpu, at);
    const Result c0 = chosenColour(app, cpu, inner, word(app, kFirstCar), 0);
    if (!c0 || !colourRgb(app, cpu, inner, word(app, kFirstCar), *c0, 0, 0x6fd307, 0x6fd308, 0x6fd309, 0x6fd30a))
        return {};
    const Result c1 = chosenColour(app, cpu, inner, word(app, kSecondCar), 1);
    if (!c1)
        return {};
    const Result r1 = colourRgb(app, cpu, inner, word(app, kSecondCar), *c1, 1, 0x6fd373, 0x6fd374, 0x6fd375, 0x6fd376);
    if (!r1)
        return {};
    reg32 r;
    if (!callGame(app, cpu, at, kSaveSettings, {reg32(*r1), reg32(*c1), 1, 0x6fd373}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x43dbe0's: the colour wheel `item`'s colours (hue, saturation, value and
 * 0xff, twice) from its player's car's custom colour. */
Result wheelFromCar(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x18, at = ebp - 0x30;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x13, r))
        return {};
    if (r == 0)
        return 0;
    const int32_t player = int32_t(word(app, item + 0x60));
    const reg32 car = player == 0 ? word(app, kFirstCar) : word(app, kSecondCar);
    const Result colour = customColour(app, cpu, at - 4, car);
    if (!colour || !colourRgb(app, cpu, at - 0xc - 4, car, *colour, player, ebp - 4, ebp - 0x10, ebp - 0xc, ebp - 8))
        return {};
    setWord(app, ebp - 0x30, byteAt(app, ebp - 4));
    setWord(app, ebp - 0x2c, byteAt(app, ebp - 0x10));
    setWord(app, ebp - 0x28, byteAt(app, ebp - 0xc));
    setWord(app, ebp - 0x24, 0xff);
    if (!callGame(app, cpu, at, kRgbToHsv, {ebp - 0x30, ebp - 0x20, reg32(player), 0xff}, {}, r))
        return {};
    for (reg32 i = 0; i < 3; ++i)
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
Result wheelGiven(App* app, CPU& cpu, reg32 esp, reg32 item, int32_t player)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x13, r, reg32(player), item))
        return {};
    if (r == 0)
        return 0;
    setWord(app, item + 0x60, reg32(player));
    if (!wheelFromCar(app, cpu, at - 4, item)
        || !callGame(app, cpu, at, kItemChanged, {item, reg32(player), reg32(player), item}, {}, r))
        return {};
    const uint8_t state = uint8_t(byteAt(app, item + 5) & 0xeb);
    setByte(app, item + 5, uint8_t(state | 4));
    return int32_t((r & 0xffff00ffu) | reg32(state) << 8);
}

}  // namespace

/* 0x439760: eax a car: how many colours it has. */
void carColourCount(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, colourCount(app, cpu, e.esp, e.eax));
}

/* 0x439780: eax a car: its custom colour's number. */
void carCustomColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, customColour(app, cpu, e.esp, e.eax));
}

/* 0x439790: eax a car, edx a player: the colour the player chose. */
void carColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, chosenColour(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x4397c0: eax a car, edx a colour: its name. */
void carColourName(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, colourName(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x4397d0: eax a car, edx a player, ebx a colour: set and saved. */
void setCarColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, colourSet(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x439810 (stdcall 0xc): eax a car, edx a colour, ebx a player: its bytes to
 * [ecx] and the stack's three pointers. */
void carColourRgb(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             colourRgb(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, e.arg(app, 0), e.arg(app, 1),
                       e.arg(app, 2)),
             0xc);
}

/* 0x4398c0 (stdcall 8): eax a car, edx a player: its custom colour bl, cl and
 * the stack's two bytes. */
void setCarCustomColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             customColourSet(app, cpu, e.esp, e.eax, int32_t(e.edx), uint8_t(e.ebx), uint8_t(e.ecx),
                             uint8_t(e.arg(app, 0)), uint8_t(e.arg(app, 1))),
             8);
}

/* 0x439ab0: eax a car, edx a number: its text. */
void carText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, carTextOf(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x43b7f0: eax a car: its record saved. */
void saveCarColours(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, colourSaved(app, cpu, e.esp, e.eax));
}

/* 0x43dbe0: the colour wheel eax from its car. */
void wheelFromCarColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, wheelFromCar(app, cpu, e.esp, e.eax));
}

/* 0x43dd50: the colour wheel eax given to player edx. */
void wheelForPlayer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, wheelGiven(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x472360: the custom colours taken into the settings. */
void refreshCustomColours(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, customColoursRefreshed(app, cpu, e.esp, e.eax));
}

/* ======== A field's text and cursor ======== */

namespace
{

/* 0x4513e0's: the cursor's colour, the 3D card's font's over a race. */
void cursorColourSet(App* app, reg32 colour)
{
    (kOverRace(app) != 0 ? kCursorColour3d : kCursorColour).set(app, colour);
}

/* 0x4d11f0's cursor: a quad (0x4d1120) at x, y in the 3D card's font, of
 * `shape` (its float the stack's word). */
bool cursor3d(App* app, CPU& cpu, reg32 at, float x, int32_t y, int32_t shape, reg32 counted)
{
    const int32_t whole = chopped<int32_t>(cpu, Float(x));
    return callGame(app, cpu, at, kCursorQuad, {reg32(whole), reg32(y), 0, counted}, {bitsOf(ofInt(shape))});
}

/* 0x4d11f0's: text `text` in the 3D card's font at x, y, the cursor before
 * character `cursor` where `show` -- of `shape` --: how wide. */
Result cursorText3d(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, int32_t cursor, int32_t shape,
                    reg32 show)
{
    const reg32 ebp = esp - 0xc, at = ebp - 0x1c;
    HeldEbp held(cpu, ebp);
    const reg32 colour = kTextColour(app) | 0xff000000;
    float place = ofInt(x);
    int32_t i = 0;
    for (reg32 s = text;; ++s, ++i)
    {
        const uint8_t c = byteAt(app, s);
        if (c == 0)
            break;
        const uint8_t index = uint8_t(c - 0x20 + uint8_t(kFont3d(app) * 0xe0));
        const reg32 width = byteAt(app, kGlyphWidth3d + index);
        if (i == cursor && show != 0 && !cursor3d(app, cpu, at, place, y, shape, reg32(i)))
            return {};
        if (!callGame(app, cpu, at, kDrawCharacter,
                      {index, byteAt(app, kTextureX3d + index), byteAt(app, kTextureY3d + index), reg32(i)},
                      {bitsOf(place), reg32(y), width, reg32(byteAt(app, kGlyphHeight3d + index)) + 1, colour,
                       kMeasuring(app)}))
            return {};
        const Float advance = cpu.fpu.pop();
        place = float(fp::add(advance, Float(place)));
    }
    if (i == cursor && show != 0 && !cursor3d(app, cpu, at, place, y, shape, reg32(i)))
        return {};
    return chopped<int32_t>(cpu, fp::sub(Float(place), Float(x)));
}

/* 0x4da560's: the 2D screens' cursor, a quad at x, y as high as the font's
 * characters and `width` wide, `left` its left side's colour and `right` its
 * right's. */
Result cursorDrawn(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, reg32 left, reg32 right)
{
    if (kOverRace(app) != 0)
        return x;
    const reg32 at = esp - 0xc - 0x10;
    HeldEbp held(cpu, esp - 0xc);
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
    const float right0 = float(fp::add(Float(x86::sreg32(width)), Float(left0)));
    const float bottom = float(fp::add(Float(top), Float(x86::sreg32(size))));
    setFloat(app, kQuad + 0x40, right0);
    setFloat(app, kQuad + 0x64, bottom);
    setWord(app, kQuad + 0x20, word(app, kQuad + 0x40));
    setFloat(app, kQuad + 0x44, bottom);
    reg32 r;
    if (!callGame(app, cpu, at, kRenderState, {1, 0, reg32(width), word(app, kQuad + 0x40)}, {}, r))
        return {};
    markCorners(app, 4);
    if (!callGame(app, cpu, at, kClipQuad, {kQuad, kQuad + 0x20, kQuad + 0x40, kQuad + 0x60}, {}, r))
        return {};
    for (const reg32 colour : {0x70u, 0x10u, 0x50u, 0x30u})
        setWord(app, kQuad + colour, 0xffffffff);
    return -1;
}

/* 0x4da8c0's: text `text` on a 2D screen at x, y (`measure`: measured only),
 * the cursor before character `cursor` where `show`, `width` wide: how
 * wide; 0 over a race. */
Result cursorText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 measure, int32_t cursor,
                  int32_t width, reg32 show)
{
    const reg32 ebp = esp - 0xc, at = ebp - 0x24;
    if (kOverRace(app) != 0)
    {
        kNudge.set(app, 0);
        return 0;
    }
    HeldEbp held(cpu, ebp);
    const reg32 colour = kScreenTextColour(app) | 0xff000000;
    const reg32 cursorColour = kCursorColour(app) | 0xff000000;
    reg32 r;
    if (!callGame(app, cpu, at, kRenderState, {1, kFontTexture(app), reg32(y), 0}, {}, r))
        return {};
    int32_t place = x, i = 0;
    for (reg32 s = text;; ++s, ++i)
    {
        const uint8_t c = byteAt(app, s);
        if (c == 0)
            break;
        reg32 index;
        if (kScreenFont(app) == 0x24)
        {
            if (!callGame(app, cpu, at, kAccented, {c, c, reg32(y), 0}, {}, index))
                return {};
        }
        else
            index = reg32(c) - 0x20;
        const reg32 w = byteAt(app, kGlyphWidth + index);
        if (i == cursor && show != 0
            && !cursorDrawn(app, cpu, at - 4 - 4, place, y, width, cursorColour, cursorColour))
            return {};
        if (!callGame(app, cpu, at, kScreenCharacter,
                      {byteAt(app, kGlyphU + index), byteAt(app, kGlyphV + index), reg32(place), reg32(y)},
                      {w, byteAt(app, kGlyphHeight + index), colour, c, measure}, r))
            return {};
        place = int32_t(reg32(place) + r);
    }
    if (i == cursor && show != 0 && !cursorDrawn(app, cpu, at - 4 - 4, place, y, width, cursorColour, cursorColour))
        return {};
    kNudge.set(app, 0);
    return int32_t(reg32(place) - reg32(x));
}

/* 0x4515e0's: a field's text at x, y of the menus' screen with its cursor
 * before character `cursor` (`width` wide, where `show`): by the 3D card's
 * font at its place on the screen's own over a race -- the fonts drawn at
 * once where the menus ask --, by the 2D screens' otherwise. */
Result fieldText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, int32_t cursor, int32_t width,
                 reg32 show)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    if (kOverRace(app) != 0)
    {
        reg32 sx, sy;
        if (!callGame(app, cpu, at, kToScreenX, {reg32(x), reg32(x), reg32(y), reg32(cursor)}, {}, sx)
            || !callGame(app, cpu, at, kToScreenY, {reg32(y), sx, reg32(y), reg32(cursor)}, {}, sy))
            return {};
        const Result r = cursorText3d(app, cpu, at - 8 - 4, text, int32_t(sx), int32_t(sy), cursor, width, show);
        if (!r)
            return {};
        if ((kMenuBits1(app) & 0x20) == 0)
            return r;
        reg32 f;
        if (!callGame(app, cpu, at, kFlushFonts, {reg32(*r), sx, sy, reg32(cursor)}, {}, f))
            return {};
        return int32_t(f);
    }
    return cursorText(app, cpu, at - 0xc - 4, text, x, y, 0, cursor, width, show);
}

/* 0x452220's: a field's line `text` in font `font` at x, y -- its right
 * there with alignment 1 or 4, its middle with 2 or 5; past 2 with a shadow
 * in colour 8 first -- in `colour`, its cursor before character `cursor`,
 * `width` wide and in `cursorColour`, where `show`. */
Result fieldLine(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 font, int32_t align,
                 reg32 colour, int32_t cursor, int32_t width, reg32 cursorColour, reg32 show)
{
    const reg32 at = esp - 0xc - 4;
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    if (!callGame(app, cpu, at, kSelectFont, {font, font, reg32(y), reg32(align)}, {}, r))
        return {};
    if (align == 1 || align == 4 || align == 2 || align == 5)
    {
        if (!callGame(app, cpu, at, kTextWidth, {text, font, reg32(y), reg32(align)}, {}, r))
            return {};
        x = int32_t(reg32(x) - reg32(align == 1 || align == 4 ? int32_t(r) : halved(int32_t(r))));
    }
    if (align > 2)
    {
        if (!callGame(app, cpu, at, kSetTextColour, {8, font, reg32(y), reg32(align)}, {}, r)
            || !callGame(app, cpu, at, kDrawTextAt, {text, reg32(x) + 1, reg32(y) + 1, reg32(align)}, {}, r))
            return {};
    }
    cursorColourSet(app, cursorColour);
    if (!callGame(app, cpu, at, kSetTextColour, {colour, show, reg32(y), reg32(width)}, {}, r))
        return {};
    return fieldText(app, cpu, at - 8 - 4, text, x, y, cursor, width, show);
}

/* 0x4522c0's: text `text` in font `font` a character a line, down from y
 * (aligned by `align` as a menu text), each line its character's middle at x:
 * in `colour`, past 2 with a shadow.  How many characters. */
Result spacedText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, int32_t font, reg32 edx)
{
    const reg32 ebp = esp - 0xc, at = ebp - 0x20, character = ebp - 0x10;
    HeldEbp held(cpu, ebp);
    setWord(app, ebp - 0x18, text);
    setHalf(app, ebp - 0xc, x);
    setHalf(app, ebp - 8, font);
    const int32_t font16 = half(app, ebp - 8);
    reg32 r;
    if (!callGame(app, cpu, at, kSelectFont, {reg32(font16), edx, reg32(y), reg32(font)}, {}, r)
        || !callGame(app, cpu, at, kFontHeight, {reg32(font16), edx, reg32(y), reg32(font)}, {}, r))
        return {};
    const int32_t spacing = chopped<int32_t>(cpu, fp::mul(Float(x86::sreg32(r)), Float(kSpacing(app))));
    setWord(app, ebp - 0x14, r);
    setWord(app, ebp - 0x1c, reg32(spacing));
    const int16_t align16 = half(app, esp + 4);
    setHalf(app, ebp - 4, int32_t(length(app, text)));
    if (align16 == 1 || align16 == 4)
        y = int32_t(reg32(y) - reg32(spacing * int32_t(word(app, ebp - 4))));
    else if (align16 == 2 || align16 == 5)
        y = int32_t(int16_t(y)) - halved(int32_t(half(app, ebp - 4)) * spacing);
    for (int16_t i = 0; int32_t(i) < int32_t(half(app, ebp - 4)); ++i)
    {
        if (!callGame(app, cpu, at, kStrncpy, {character, word(app, ebp - 0x18), 1, reg32(i)}, {}, r))
            return {};
        // The font as the code takes it, before the character is written.
        const reg32 fontNow = reg32(int32_t(half(app, ebp - 8)));
        if (!format(app, cpu, at, {character, kCharSpace, byteAt(app, word(app, ebp - 0x18) + reg32(int32_t(i)))})
            || !callGame(app, cpu, at, kTextWidth, {character, fontNow, 1, reg32(i)}, {}, r))
            return {};
        const int32_t left = int32_t(half(app, ebp - 0xc)) - halved(int32_t(r));
        setWord(app, ebp - 0x20, reg32(left));
        if (half(app, esp + 4) > 2)
        {
            const int32_t down = spacing * int32_t(i);
            if (!callGame(app, cpu, at, kSetTextColour, {8, reg32(down), reg32(spacing), reg32(i)}, {}, r)
                || !callGame(app, cpu, at, kDrawTextAt,
                             {character, reg32(left) + 1, reg32(int32_t(int16_t(y)) + down) + 1, reg32(i)}, {}, r))
                return {};
        }
        if (!callGame(app, cpu, at, kSetTextColour, {word(app, esp + 8), reg32(left), reg32(spacing), reg32(i)}, {}, r)
            || !callGame(app, cpu, at, kDrawTextAt,
                         {character, reg32(left), reg32(int32_t(int16_t(y)) + int32_t(i) * spacing), reg32(i)}, {},
                         r))
            return {};
    }
    return half(app, ebp - 4);
}

}  // namespace

/* 0x4513e0: the cursor's colour, eax. */
void setCursorColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cursorColourSet(app, e.eax);
    e.back(cpu, e.eax);
}

/* 0x4515e0 (stdcall 8): text eax at edx, ebx, the cursor before character
 * ecx, its width and whether it shows the stack's words. */
void drawFieldText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             fieldText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                       int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x452220 (stdcall 0x18): text eax in font ecx at edx, ebx; the stack's its
 * alignment, colour, cursor, the cursor's width, its colour, whether it
 * shows. */
void drawFieldLine(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             fieldLine(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, int32_t(e.arg(app, 0)),
                       e.arg(app, 1), int32_t(e.arg(app, 2)), int32_t(e.arg(app, 3)), e.arg(app, 4), e.arg(app, 5)),
             0x18);
}

/* 0x4522c0 (stdcall 8): text eax in font cx a character a line at dx, ebx;
 * its alignment (a word) and colour the stack's. */
void drawSpacedText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    pushed(app, e.esp, {e.esi, e.edi, e.ebp});
    const Result r = spacedText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.edx);
    e.finish(cpu, r, 8);
    if (r)
        popped(app, e.esp, {&cpu.esi, &cpu.edi, &cpu.ebp});
}

/* 0x4d11f0 (stdcall 8): text eax in the 3D card's font at edx, ebx, the
 * cursor before character ecx where the stack's second word, of the first. */
void drawCursorText3d(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             cursorText3d(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                          int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x4da560 (stdcall 4): the 2D screens' cursor at eax, edx, ebx wide, in ecx
 * and the stack's colour. */
void drawCursor(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, cursorDrawn(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx, e.arg(app, 0)),
             4);
}

/* 0x4da8c0 (stdcall 0xc): text eax on a 2D screen at edx, ebx (ecx: only
 * measured), the cursor before the stack's character, its width, whether
 * it shows. */
void drawCursorText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             cursorText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, int32_t(e.arg(app, 0)),
                        int32_t(e.arg(app, 1)), e.arg(app, 2)),
             0xc);
}

/* ======== The car's colours (carclrselect) ======== */

namespace
{

ColourList& colourList(App* app, reg32 item)
{
    return record<ColourList>(app, item);
}

Result listLaidOut(App* app, CPU& cpu, reg32 esp, reg32 item);

/* 0x4569a0's: the list's width: its widest colour's name, or "custom...",
 * and the swatch's.  The last width measured. */
Result listWidth(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 edx)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x100, text = ebp - 0x100;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!callGame(app, cpu, at, kLanguageText, {0x2b, edx, 0, item}, {}, r) || !format(app, cpu, at, {text, kEllipsis, r})
        || !callGame(app, cpu, at, kTextWidth, {text, 0x12, 0, item}, {}, r))
        return {};
    colourList(app, item).boxWidth = int16_t(r);
    for (int32_t i = 0; i < colourList(app, item).count; ++i)
    {
        const Result name = colourName(app, cpu, at - 4, word(app, colourList(app, item).car), i);
        if (!name || !callGame(app, cpu, at, kTextWidth, {reg32(*name), 0x12, reg32(i), item}, {}, r))
            return {};
        if (int32_t(r) > colourList(app, item).boxWidth)
            colourList(app, item).boxWidth = int16_t(r);
    }
    colourList(app, item).boxWidth = int16_t(colourList(app, item).boxWidth + 0x2d);
    return int32_t(r);
}

/* 0x456a20's: the list's player's car, its colours, the chosen one's. */
Result listRefreshed(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x18, at = ebp - 0x14;
    setByte(app, ebp - 8, 0xff);
    if (item == 0)
        return 0;
    HeldEbp held(cpu, ebp);
    const int32_t player = colourList(app, item).player != 0 ? 1 : 0;
    const reg32 car = player != 0 ? kSecondCar : kFirstCar;
    reg32 rec;
    if (!carRecordOf(app, cpu, at, word(app, car), rec))
        return {};
    if (rec == 0)
        return 0;
    colourList(app, item).car = car;
    colourList(app, item).shownCar = int32_t(word(app, car));
    const Result chosen = chosenColour(app, cpu, at - 4, word(app, car), player);
    if (!chosen)
        return {};
    colourList(app, item).chosen = *chosen;
    colourList(app, item).highlighted = *chosen;
    if (!colourRgb(app, cpu, at - 0xc - 4, word(app, car), *chosen, player, ebp - 0x14, ebp - 4, ebp - 0x10,
                   ebp - 0xc))
        return {};
    const Result count = colourCount(app, cpu, at - 4, word(app, colourList(app, item).car));
    if (!count)
        return {};
    colourList(app, item).count = *count;
    colourList(app, item).colour = reg32(byteAt(app, ebp - 8)) << 24 | reg32(byteAt(app, ebp - 0x14)) << 16
                                   | reg32(byteAt(app, ebp - 4)) << 8 | byteAt(app, ebp - 0x10);
    return byteAt(app, ebp - 0x10);
}

/* 0x4d8aa0's: a frame around the rectangle in `colour`. */
Result framed(App* app, CPU& cpu, reg32 esp, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour)
{
    reg32 r;
    if (!callGame(app, cpu, esp - 8, kFrame, {x, y, width, height}, {colour}, r))
        return {};
    return int32_t(r);
}

/* 0x456b50's: colour `colour`'s line of the list at x, y -- its swatch first
 * where `swatch` (framed lit where `lit`) --: its name, "custom..." for the
 * custom one. */
Result colourLine(App* app, CPU& cpu, reg32 esp, reg32 item, int32_t colour, int32_t x, int32_t y, reg32 lit,
                  reg32 swatch)
{
    const reg32 ebp = esp - 0xc, at = ebp - 0x11c, text = ebp - 0x11c;
    HeldEbp held(cpu, ebp);
    reg32 r, textColour;
    if (swatch != 0)
    {
        if (!colourRgb(app, cpu, at - 0xc - 4, word(app, colourList(app, item).car), colour,
                       colourList(app, item).player, ebp - 0xc, ebp - 8, ebp - 4, ebp - 0x10))
            return {};
        const reg32 argb = 0xff000000 | reg32(byteAt(app, ebp - 0xc)) << 16 | reg32(byteAt(app, ebp - 8)) << 8
                           | byteAt(app, ebp - 4);
        if (!callGame(app, cpu, at, kFilled, {reg32(x), reg32(y) + 1, 0x10, 0xd}, {argb}, r))
            return {};
        const Result f = framed(app, cpu, at - 4 - 4, reg32(x), reg32(y) + 1, 0x10, 0xc, lit != 0 ? 0xffffe440 : 0xff7b8ab5);
        if (!f)
            return {};
        r = reg32(*f);
        x += 0x13;
        textColour = lit != 0 ? 0xffe440 : 0xff649dfd;
    }
    else
    {
        if (!callGame(app, cpu, at, kColourSetState, {item, reg32(colour), reg32(x), reg32(y)}, {}, r))
            return {};
        textColour = r != 0 ? 0x4a96 : 0xffe440;
    }
    if ((kMenuBits3(app) & 0x40) != 0)
        return int32_t(r);
    const reg32 car = word(app, colourList(app, item).car);
    const Result custom = customColour(app, cpu, at - 4, car);
    if (!custom)
        return {};
    reg32 name;
    if (*custom == colour)
    {
        if (swatch != 0)
        {
            if (!callGame(app, cpu, at, kLanguageText, {0x2b, textColour, swatch, reg32(colour)}, {}, name)
                || !format(app, cpu, at, {text, kEllipsis, name}))
                return {};
            name = text;
        }
        else if (!callGame(app, cpu, at - 8, kLanguageText, {0x2b, reg32(x), reg32(y), 0x12}, {}, name))
            return {};
    }
    else
    {
        const Result n = colourName(app, cpu, at - 8 - 4, car, colour);
        if (!n)
            return {};
        name = reg32(*n);
    }
    if (!callGame(app, cpu, at, kDrawMenuText, {name, reg32(x), reg32(y), 0x12}, {0, textColour}, r))
        return {};
    return int32_t(r);
}

/* 0x456ce0's: the car's colours: the chosen one's line beside the arrow,
 * shut; open, the colours' box -- the one under the pointer lit where `hot`
 * --, the arrow and its box, the label. */
Result listDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot)
{
    const reg32 ebp = esp - 0x14, at = ebp - 8;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x12, r, hot, item))
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
        && !listRefreshed(app, cpu, at - 4, item))
        return {};
    if (hot != 0)
    {
        if (colourList(app, item).open != 0)
        {
            if (!callGame(app, cpu, at, kPointerInBox, {item + 0x1a, colourList(app, item).car, hot, item}, {}, r))
                return {};
            if (r != 0)
            {
                if (!callGame(app, cpu, at, kLineAtPointer, {reg32(int32_t(colourList(app, item).boxY)), 0x10, hot, item},
                              {}, r))
                    return {};
                const int32_t line = int32_t(r);
                if (line >= 0 && line < colourList(app, item).count && line != colourList(app, item).highlighted)
                    colourList(app, item).highlighted = line;
            }
            const ColourList& l = colourList(app, item);
            if (!callGame(app, cpu, at, kFieldBox,
                          {reg32(int32_t(l.boxX)), reg32(int32_t(l.boxY)), reg32(int32_t(l.boxWidth)),
                           reg32(int32_t(l.boxHeight))},
                          {0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222}, r))
                return {};
            int16_t lineY = int16_t(colourList(app, item).boxY + 5);
            const int16_t lineX = int16_t(colourList(app, item).boxX + 0xf);
            for (int32_t i = 0; i < colourList(app, item).count; ++i)
            {
                if (!colourLine(app, cpu, at - 8 - 4, item, i, lineX, lineY,
                                i == colourList(app, item).highlighted ? 1 : 0, 1))
                    return {};
                lineY = int16_t(lineY + 0x10);
            }
            const int32_t x = colourList(app, item).x, y = colourList(app, item).y;
            if (!callGame(app, cpu, at, kDrawShape, {reg32(int32_t(colourList(app, item).arrow)), reg32(x), reg32(y + 0xd), reg32(y)}, {}, r)
                || !callGame(app, cpu, at, kDrawShape, {reg32(int32_t(colourList(app, item).box)), reg32(x + 0xc), reg32(y), reg32(y)}, {}, r)
                || !callGame(app, cpu, at, kDrawMenuText,
                             {colourList(app, item).label, reg32(int32_t(colourList(app, item).boxX) + 0xf),
                              reg32(y - 1), 0x12},
                             {0, 0xffe440}, r))
                return {};
        }
        else
        {
            const int32_t x = colourList(app, item).x, y = colourList(app, item).y;
            if (!callGame(app, cpu, at, kDrawShape, {reg32(int32_t(colourList(app, item).arrow)), reg32(x), reg32(y + 0xd), reg32(y)}, {}, r))
                return {};
        }
    }
    if (colourList(app, item).open == 0 && kScreensSliding(app) == 0
        && !colourLine(app, cpu, at - 8 - 4, item, colourList(app, item).chosen,
                       int32_t(colourList(app, item).x) + 0x19, colourList(app, item).y, 0, 0))
        return {};
    return 0;
}

/* 0x456ea0's: the list shown: shut, its shapes found, its colours taken. */
Result listShown(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x12, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    colourList(app, item).open = 0;
    if (!callGame(app, cpu, at, kShapeByName, {kArrowName, 0x12, ebx, item}, {}, r))
        return {};
    colourList(app, item).arrow = int16_t(r);
    if (!callGame(app, cpu, at, kShapeByName, {kBoxName, 0x12, ebx, item}, {}, r))
        return {};
    ColourList& l = colourList(app, item);
    l.chosen = 0;
    l.count = 0;
    l.car = 0;
    l.colour = 0;
    l.box = int16_t(r);
    l.highlighted = 0;
    if (!listRefreshed(app, cpu, at - 4, item))
        return {};
    return listWidth(app, cpu, at - 4, item, 0x12);
}

/* 0x456f10's: the pointer put on the lit line. */
Result pointedAtColour(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const ColourList& l = colourList(app, item);
    const reg32 y = (reg32(l.highlighted) << 4) + reg32(int32_t(l.boxY)) + 0x10;
    const reg32 x = reg32(halved(l.boxWidth) + l.boxX);
    reg32 r;
    if (!callGame(app, cpu, esp - 0x10, kSetPointer, {x, y, y, item}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x456f50's, 0x456f70's: the list opened (the chosen one lit) or shut. */
Result listOpened(App* app, CPU& cpu, reg32 esp, reg32 item, int16_t open)
{
    colourList(app, item).open = open;
    colourList(app, item).highlighted = colourList(app, item).chosen;
    return listLaidOut(app, cpu, esp - 8 - 4, item);
}

/* 0x456f90's: the lit colour chosen: set for the car and saved, the list
 * shut; its player's 3D car on the screen told to change. */
Result colourChosen(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 at = esp - 0x18;
    HeldEbp held(cpu, at);
    ColourList& l = colourList(app, item);
    const bool changed = l.chosen != l.highlighted;
    l.chosen = l.highlighted;
    l.open = 0;
    if (!colourSet(app, cpu, at - 4, word(app, l.car), l.player, l.highlighted))
        return {};
    Result r = listLaidOut(app, cpu, at - 4, item);
    if (!r || !changed)
        return r;
    reg32 eax = reg32(*r), menu;
    if (colourList(app, item).player == 0)
    {
        if (!callGame(app, cpu, at, kMenuNamed, {eax, kFirstCarItem}, {}, menu)
            || !callGame(app, cpu, at, kFindItem, {menu, kFirstCarItem}, {}, eax))
            return {};
        if (eax != 0)
            setByte(app, eax + 0xb3, 1);
    }
    if (colourList(app, item).player == 1)
    {
        if (!callGame(app, cpu, at, kMenuNamed, {eax, kSecondCarItem}, {}, menu)
            || !callGame(app, cpu, at, kFindItem, {menu, kSecondCarItem}, {}, eax))
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
Result listKey(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 at = esp - 0x10;
    HeldEbp held(cpu, at);
    const uint16_t key = uint16_t(ebx);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x12, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    if (colourList(app, item).car == 0)
        return 0;
    const auto click = [&] { return callGame(app, cpu, at, kMenuClick, {1, 0x7f, ebx, item}, {}, r); };
    const auto shut = [&]() -> Result {
        if (!click() || !listOpened(app, cpu, at - 4, item, 0))
            return {};
        return 2;
    };
    if (colourList(app, item).open == 0)
    {
        if (key != kEnter && key != kRight)
            return 0;
        if (!listOpened(app, cpu, at - 4, item, 1) || !click())
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
        if (!pointedAtColour(app, cpu, at - 4, item) || !click())
            return {};
        return 2;
    }
    if (key == kEscape)
        return shut();
    if (key != kEnter && key != kLeft)
        return 0;
    if (!callGame(app, cpu, at, kPointerInBox, {item + 0x1a, 0x7f, ebx, item}, {}, r))
        return {};
    if (r == 0)
        return shut();
    if (!click() || !colourChosen(app, cpu, at - 4, item))
        return {};
    const Result custom = customColour(app, cpu, at - 4, word(app, colourList(app, item).car));
    if (!custom)
        return {};
    if (*custom != colourList(app, item).highlighted)
        return 2;
    reg32 menu, wheel;
    if (!callGame(app, cpu, at, kMenuNamed, {reg32(*custom), kColourEdit}, {}, menu)
        || !callGame(app, cpu, at, kFindItem, {menu, kColourEdit}, {}, wheel))
        return {};
    if (wheel != 0 && !wheelGiven(app, cpu, at - 4, wheel, colourList(app, item).player))
        return {};
    return 2;
}

/* 0x4571e0's: the box the pointer finds the list in: its colours' open, the
 * arrow's shut.  eax as the code leaves it, the box's top in its low half. */
Result listLaidOut(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0x12, r, 0, item))
        return {};
    if (r == 0)
        return 0;
    reg32 e;
    if (colourList(app, item).open != 0)
    {
        const Result w = listWidth(app, cpu, at - 4, item, 0x12);
        if (!w)
            return {};
        ColourList& l = colourList(app, item);
        e = ((reg32(*w) & 0xffff0000) | uint16_t(l.count)) << 4;
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
void colourListWidth(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listWidth(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x456a20: the colours' list eax's colours taken from its car. */
void colourListRefresh(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listRefreshed(app, cpu, e.esp, e.eax));
}

/* 0x456b50 (stdcall 8): colour edx's line of the list eax at ebx, ecx; lit
 * and with its swatch the stack's words. */
void drawColourLine(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             colourLine(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0),
                        e.arg(app, 1)),
             8);
}

/* 0x456ce0: the colours' list eax drawn, the pointer's line lit where edx. */
void drawColourList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listDrawn(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x456ea0: the colours' list eax shown. */
void colourListShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listShown(app, cpu, e.esp, e.eax, e.ebx));
}

/* 0x456f10: the pointer on the list eax's lit line. */
void pointAtColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, pointedAtColour(app, cpu, e.esp, e.eax));
}

/* 0x456f50, 0x456f70: the list eax opened, shut. */
void openColourList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listOpened(app, cpu, e.esp, e.eax, 1));
}

void closeColourList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listOpened(app, cpu, e.esp, e.eax, 0));
}

/* 0x456f90: the list eax's lit colour chosen. */
void chooseColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, colourChosen(app, cpu, e.esp, e.eax));
}

/* 0x457030: key bx on the list eax. */
void colourListKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listKey(app, cpu, e.esp, e.eax, e.ebx));
}

/* 0x4571e0: the list eax laid out. */
void layoutColourList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listLaidOut(app, cpu, e.esp, e.eax));
}

/* 0x457250: the list hidden: nothing. */
void colourListHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

/* 0x4d8aa0 (stdcall 4): a frame around eax, edx, ebx wide, ecx high in the
 * stack's colour. */
void drawFrameAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, framed(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* ======== The fields typed into ======== */

namespace
{

/* 0x457ae0's: key `key` typed into the field's line `typed`, `size` bytes,
 * at the cursor (the word at `cursor`): a digit, or a dot (none where
 * `noDots`, one only), kept to the room a sign and its ending leave; a minus
 * first; Backspace or Delete clears it.  2 for a key taken, 0 else. */
Result keyTyped(App* app, CPU& cpu, reg32 esp, reg32 typed, int32_t size, reg32 cursor, reg32 ecx, reg32 noDots)
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
        reg32 r;
        if (!callGame(app, cpu, esp - 0xc - 8, kMemset, {typed, 0, reg32(size), ecx}, {}, r))
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
            setWord(app, cursor, reg32(size - 2));
        return 2;
    }
    if (!digit && key != '.')
        return 0;
    if (key == '.' && noDots != 0)
        return 2;
    bool dot = false;
    for (int32_t i = 0; i < size; ++i)
        if (byteAt(app, typed + reg32(i)) == '.')
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
    setWord(app, cursor, reg32(next));
    if (limit < next)
        setWord(app, cursor, reg32(limit));
    return 2;
}

/* What a field's draw shares: its label, the place its value goes. */
struct Labelled
{
    int32_t x, y;
    reg32 colour, labelWidth;
};

/* The field drawn up to its value: the 0x100 bytes of its blank copied into
 * the frame's line, the font, the label (its width to [ebp-0x10], which the
 * code reads as it is without one).  False when the game stopped; `shown`
 * false (and eax 0) when the item is not on the screen. */
bool labelled(App* app, CPU& cpu, reg32 ebp, reg32 item, reg32 hot, reg32 type, reg32 blank, bool& shown,
              Labelled& l)
{
    const reg32 at = ebp - 0x114;
    for (reg32 i = 0; i < 0x100; i += 4)
        setWord(app, at + i, word(app, blank + i));
    reg32 r;
    if (!shownNow(app, cpu, at, item, type, r, item, 0))
        return false;
    shown = r != 0;
    if (!shown)
        return true;
    const Item& it = record<Item>(app, item);
    l.colour = hot != 0 ? 0xffe440 : 0xff649dfd;
    l.x = it.x;
    l.y = it.y;
    if (!callGame(app, cpu, at, kSelectFont, {0x12, type, item, 0}, {}, r))
        return false;
    const reg32 label = word(app, item + 0x3c);
    if (label != 0)
    {
        if (!callGame(app, cpu, at, kTextWidth, {label, 0x12, reg32(l.y), 0}, {}, r))
            return false;
        setWord(app, ebp - 0x10, r);
        if (!callGame(app, cpu, at, kDrawMenuText, {label, reg32(l.x), reg32(l.y), 0x12}, {0, l.colour}, r))
            return false;
    }
    l.labelWidth = word(app, ebp - 0x10);
    return true;
}

/* A field's line being typed, its cursor at `cursor`. */
bool typedLine(App* app, CPU& cpu, reg32 ebp, reg32 typed, const Labelled& l, int32_t cursor)
{
    const reg32 at = ebp - 0x114, text = ebp - 0x114;
    return format(app, cpu, at, {text, kS, typed})
           && fieldLine(app, cpu, at - 0x18 - 4, text, int32_t(reg32(l.x) + l.labelWidth), l.y, 0x12, 0, 0x800000,
                        cursor, 3, 0x800000, 1);
}

/* A field's value as the frame's line has it, drawn. */
bool valueLine(App* app, CPU& cpu, reg32 ebp, const Labelled& l)
{
    reg32 r;
    return callGame(app, cpu, ebp - 0x114, kDrawMenuText, {ebp - 0x114, reg32(l.x) + l.labelWidth, reg32(l.y), 0x12},
                    {0, l.colour}, r);
}

/* A field's unit's name, its value's format's last word: 0 for none. */
reg32 unitOf(uint8_t shown, reg32 unit, reg32 units)
{
    return shown == 1 && unit != 0 ? unit : shown == 2 && units != 0 ? units : 0;
}

/* 0x457c40's, 0x458480's: an edint's, an edchar's drawing. */
Result intDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    HeldEbp held(cpu, ebp);
    bool shown;
    Labelled l;
    if (!labelled(app, cpu, ebp, item, hot, 0xa, kIntBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const IntField& f = record<IntField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, cpu, ebp, item + 0x60, l, f.cursor) ? Result(0) : Result();
    const reg32 value = word(app, f.value);
    const reg32 unit = unitOf(f.unitsShown, f.unit, f.units);
    if (!(unit != 0 ? format(app, cpu, at, {text, kDS, value, unit}) : format(app, cpu, at, {text, kD, value}))
        || !valueLine(app, cpu, ebp, l))
        return {};
    return 0;
}

Result charDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    HeldEbp held(cpu, ebp);
    bool shown;
    Labelled l;
    if (!labelled(app, cpu, ebp, item, hot, 0xb, kCharBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const CharField& f = record<CharField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, cpu, ebp, item + 0x58, l, f.cursor) ? Result(0) : Result();
    const reg32 value = byteAt(app, f.value);
    const reg32 unit = unitOf(f.unitsShown, f.unit, f.units);
    if (!(unit != 0 ? format(app, cpu, at, {text, kDS, value, unit}) : format(app, cpu, at, {text, kD, value}))
        || !valueLine(app, cpu, ebp, l))
        return {};
    return 0;
}

/* 0x458940's, 0x458e30's: an edfloat's, an edfixed's: its value with its
 * decimals (1 to 6, else 6) as a double. */
Result floatDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot, bool fixed)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x114, text = ebp - 0x114;
    HeldEbp held(cpu, ebp);
    bool shown;
    Labelled l;
    if (!labelled(app, cpu, ebp, item, hot, fixed ? 0xd : 0xc, fixed ? kFixedBlank : kFloatBlank, shown, l))
        return {};
    if (!shown)
        return 0;
    const FloatField& f = record<FloatField>(app, item);
    if (f.value == 0)
        return 0;
    if (f.editing == 1)
        return typedLine(app, cpu, ebp, item + 0x64, l, f.cursor) ? Result(0) : Result();
    const reg32 decimals = f.decimals > 0 && f.decimals <= 6 ? reg32(f.decimals) : 6;
    const reg32 unit = unitOf(f.unitsShown, f.unit, f.units);
    const Float value = fixed ? fp::mul(Float(x86::sreg32(word(app, f.value))), Float(kFixedUnit(app)))
                              : Float(floatAt(app, f.value));
    const Words d = wordsOf(double(value));
    if (!(unit != 0 ? format(app, cpu, at, {text, kFS, decimals, d.lo, d.hi, unit})
                    : format(app, cpu, at, {text, kF, decimals, d.lo, d.hi}))
        || !valueLine(app, cpu, ebp, l))
        return {};
    return 0;
}

/* The range messages: the dialog's text written, the question put. */
Result asked(App* app, CPU& cpu, reg32 at, std::initializer_list<reg32> words)
{
    reg32 r;
    if (!format(app, cpu, at, words) || !question(app, cpu, at, r))
        return {};
    return int32_t(r);
}

/* What the key handlers share: Enter starts typing (the line cleared, the
 * cursor at its start, the field held) where the field has a value. */
Result typingStarted(App* app, CPU& cpu, reg32 at, reg32 item, reg32 typed, reg32 size, reg32 cursor)
{
    setHalf(app, item + 0x40, 1);
    setWord(app, cursor, 0);
    reg32 r;
    if (!callGame(app, cpu, at, kMemset, {typed, 0, size, item}, {}, r))
        return {};
    record<Item>(app, item).state |= 4;
    return 2;
}

/* A field's function told of its change, eax the item. */
bool told(App* app, CPU& cpu, reg32 at, reg32 function, reg32 item, reg32 edx, reg32 ebx)
{
    reg32 r;
    return function == 0 || callGame(app, cpu, at, function, {item, edx, ebx, item}, {}, r);
}

/* 0x457e30's: a key on an edint. */
Result intKey(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 ebp = esp - 0x10, at = ebp - 8;
    HeldEbp held(cpu, ebp);
    const uint16_t key = uint16_t(ebx);
    setWord(app, ebp - 8, 0);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0xa, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<IntField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<IntField>(app, item).value == 0)
            return 0;
        return typingStarted(app, cpu, at, item, item + 0x60, 0xc, item + 0x5c);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<IntField>(app, item).editing = 0;
        record<IntField>(app, item).cursor = 0;
        reg32 n;
        if (!callGame(app, cpu, at, kSscanf, {}, {item + 0x60, kD, ebp - 8}, n))
            return {};
        const IntField& f = record<IntField>(app, item);
        if (f.checked == 1)
        {
            const int32_t v = int32_t(word(app, ebp - 8));
            if (v < f.least || v > f.most)
            {
                pushed(app, at - 4, {item, item, ebp});
                if (!asked(app, cpu, at - 4 - 0xc, {kDialogText, kIntRange, reg32(f.least), reg32(f.most)}))
                    return {};
                n = 0;
            }
        }
        if (n == 1)
        {
            setWord(app, record<IntField>(app, item).value, word(app, ebp - 8));
            if (!told(app, cpu, at, record<IntField>(app, item).changed, item, n, item))
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
    if (!keyTyped(app, cpu, at - 4 - 4, item + 0x60, 0xc, item + 0x5c, reg32(int32_t(int16_t(key))), 1))
        return {};
    return 2;
}

/* 0x458690's: a key on an edchar. */
Result charKey(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 ebp = esp - 0x10, at = ebp - 8;
    HeldEbp held(cpu, ebp);
    const uint16_t key = uint16_t(ebx);
    setByte(app, ebp - 4, 0);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0xb, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<CharField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<CharField>(app, item).value == 0)
            return 0;
        return typingStarted(app, cpu, at, item, item + 0x58, 5, item + 0x54);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<CharField>(app, item).editing = 0;
        record<CharField>(app, item).cursor = 0;
        reg32 n;
        if (!callGame(app, cpu, at, kSscanf, {}, {item + 0x58, kD, ebp - 4}, n))
            return {};
        const CharField& f = record<CharField>(app, item);
        if (f.checked == 1)
        {
            const uint8_t v = byteAt(app, ebp - 4);
            if (v < f.least || v > f.most)
            {
                pushed(app, at - 4, {item, item, ebp});
                if (!asked(app, cpu, at - 4 - 0xc, {kDialogText, kIntRange, f.least, f.most}))
                    return {};
                n = 0;
            }
        }
        if (n == 1)
        {
            setByte(app, record<CharField>(app, item).value, byteAt(app, ebp - 4));
            if (!told(app, cpu, at, record<CharField>(app, item).changed, item, n, item))
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
        if (!told(app, cpu, at, f.changed, item, 0xb, item))
            return {};
        return 2;
    }
    if (!keyTyped(app, cpu, at - 4 - 4, item + 0x58, 5, item + 0x54, reg32(int32_t(int16_t(key))), 1))
        return {};
    return 2;
}

/* 0x458b80's, 0x459090's: a key on an edfloat, an edfixed. */
Result floatKey(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx, bool fixed)
{
    const reg32 ebp = esp - 0x10, at = ebp - 8;
    HeldEbp held(cpu, ebp);
    const uint16_t key = uint16_t(ebx);
    setWord(app, ebp - 8, 0);
    reg32 r;
    if (!shownNow(app, cpu, at, item, fixed ? 0xd : 0xc, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    const uint16_t editing = uint16_t(record<FloatField>(app, item).editing);
    if (editing == 0)
    {
        if (key != kEnter || record<FloatField>(app, item).value == 0)
            return 0;
        return typingStarted(app, cpu, at, item, item + 0x64, 0xf, item + 0x5c);
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        record<FloatField>(app, item).editing = 0;
        record<FloatField>(app, item).cursor = 0;
        reg32 n;
        if (!callGame(app, cpu, at, kSscanf, {}, {item + 0x64, kScanF, ebp - 8}, n))
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
                cpu.fpu.compare(Float(floatAt(app, ebp - 8)), Float(fromBits(f.least)));
                out = cpu.fpu.status.c0;
                if (!out)
                {
                    cpu.fpu.compare(Float(floatAt(app, ebp - 8)), Float(fromBits(f.most)));
                    out = !cpu.fpu.status.c0 && !cpu.fpu.status.c3;
                }
            }
            if (out)
            {
                if (fixed)
                {
                    const Float unit = Float(kFixedUnit2(app));
                    const Words most = wordsOf(double(fp::mul(Float(x86::sreg32(f.most)), unit)));
                    const Words least = wordsOf(double(fp::mul(unit, Float(x86::sreg32(f.least)))));
                    if (!asked(app, cpu, at - 4 - 0xc - 8,
                               {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}))
                        return {};
                }
                else
                {
                    const Words least = wordsOf(double(Float(fromBits(f.least))));
                    const Words most = wordsOf(double(Float(fromBits(f.most))));
                    if (!asked(app, cpu, at - 8 - 4 - 0x10,
                               {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}))
                        return {};
                }
                n = 0;
            }
        }
        if (n == 1)
        {
            FloatField& g = record<FloatField>(app, item);
            setWord(app, g.value, fixed ? reg32(scaled()) : word(app, ebp - 8));
            if (!told(app, cpu, at, g.changed, item, item, g.value))
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
    if (!keyTyped(app, cpu, at - 4 - 4, item + 0x64, 0xf, item + 0x5c, reg32(int32_t(int16_t(key))), 0))
        return {};
    return 2;
}

/* The fields' layout: the box at the field, 0x1d high and as wide as its
 * label and its value (`valueWidth`) and 10 between; esi's width without a
 * label, as the code leaves it. */
Result laidOut(App* app, CPU& cpu, reg32 at, reg32 item, reg32 type, reg32 esi,
               const std::function<bool(reg32&)>& valueWidth)
{
    reg32 r;
    if (!shownNow(app, cpu, at, item, type, r, 0, item))
        return {};
    if (r == 0)
        return 0;
    Item& it = record<Item>(app, item);
    it.boxHeight = 0x1d;
    it.boxX = it.x;
    it.boxY = it.y;
    reg32 labelWidth = esi;
    const reg32 label = word(app, item + 0x3c);
    if (label != 0 && !callGame(app, cpu, at, kTextWidth, {label, 0x12, item, item}, {}, labelWidth))
        return {};
    reg32 w;
    if (!valueWidth(w))
        return {};
    const reg32 e = labelWidth + w + 0xa;
    record<Item>(app, item).boxWidth = int16_t(e);
    return int32_t(e);
}

/* A value's width: its typed line's while it is edited, else as `format`
 * writes it into the frame at `text`. */
bool valueWidth(App* app, CPU& cpu, reg32 at, reg32 item, reg32 typed, reg32 text,
                std::initializer_list<reg32> words, reg32& w)
{
    if (record<IntField>(app, item).editing != 0)
        return callGame(app, cpu, at, kTextWidth, {typed, 0x12, item, item}, {}, w);
    return format(app, cpu, at, words) && callGame(app, cpu, at, kTextWidth, {text, 0x12, item, item}, {}, w);
}

/* The fields' shown and hidden: typing stopped; shown, the function told
 * where the field has a value. */
Result fieldShown(App* app, CPU& cpu, reg32 at, reg32 item, reg32 type, reg32 changed, reg32 value, reg32 ebx,
                  bool tell)
{
    reg32 r;
    if (!shownNow(app, cpu, at, item, type, r, ebx, item))
        return {};
    if (r == 0)
        return 0;
    const int16_t editing = half(app, item + 0x40);
    if (editing == 1)
        setHalf(app, item + 0x40, 0);
    if (!tell)
        return int32_t(r);
    const reg32 function = word(app, item + changed);
    if (function == 0 || word(app, item + value) == 0)
        return int32_t(r);
    if (!callGame(app, cpu, at, function, {item, item, (ebx & 0xffff0000) | uint16_t(editing), item}, {}, r))
        return {};
    return int32_t(r);
}

/* The fields' bind: the value's place and the function told of a change. */
Result fieldBound(App* app, CPU& cpu, reg32 at, reg32 item, reg32 type, reg32 value, reg32 function,
                  reg32 valueAt, reg32 functionAt)
{
    reg32 r;
    if (!shownNow(app, cpu, at, item, type, r, function, item))
        return {};
    if (r == 0)
        return 0;
    setWord(app, item + functionAt, function);
    setWord(app, item + valueAt, value);
    return 1;
}

/* 0x4580b0's: an edstring drawn: its label, its text -- being typed, the
 * cursor "_" blinking after it. */
Result stringDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot, reg32 ebx, reg32 ecx)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x14;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 9, r, ebx, ecx))
        return {};
    if (r == 0)
        return 0;
    const reg32 colour = hot != 0 ? 0xffe440 : 0xff649dfd;
    const int32_t y = record<StringField>(app, item).y;
    int32_t x = record<StringField>(app, item).x;
    if (!callGame(app, cpu, at, kSelectFont, {0x12, 9, 0, ecx}, {}, r))
        return {};
    const reg32 label = record<StringField>(app, item).label;
    if (label != 0)
    {
        if (!callGame(app, cpu, at, kTextWidth, {label, 0x12, 0, 0x12}, {}, r))
            return {};
        setWord(app, ebp - 0x14, r);
        if (!callGame(app, cpu, at, kDrawMenuText, {label, reg32(x), reg32(y), 0x12}, {0, colour}, r))
            return {};
    }
    x = int16_t(x + int32_t(word(app, ebp - 0x14)) + 0xa);
    const StringField& f = record<StringField>(app, item);
    if (f.target == 0)
        return 0;
    if (f.editing == 0)
    {
        if (!callGame(app, cpu, at, kDrawMenuText, {f.target, reg32(x), reg32(y), 0x12}, {0, colour}, r))
            return {};
        return 0;
    }
    if (!callGame(app, cpu, at, kDrawMenuText, {f.buffer, reg32(x), reg32(y), 0x12}, {0, colour}, r))
        return {};
    StringField& g = record<StringField>(app, item);
    g.blink = int16_t((int32_t(g.blink) + 1) % 0x1e);
    if (g.blink >= 0xf)
        return 0;
    if (!callGame(app, cpu, at - 8, kTextWidth, {g.buffer, 0x12, reg32(y), 0x12}, {}, r)
        || !callGame(app, cpu, at, kDrawMenuText, {kUnderline, reg32(x) + r, reg32(y), 0x12}, {0, colour}, r))
        return {};
    return 0;
}

/* 0x4581f0's: a key on an edstring: Enter copies the text in to type and
 * holds the field; typing, a character added (to its length), Backspace or
 * Delete takes the last off, Enter copies it out and tells the function,
 * Escape lets it go. */
Result stringKey(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 at = esp - 0x10;
    HeldEbp held(cpu, at);
    const uint16_t key = uint16_t(ebx);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 9, r, ebx, item))
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
            if (!callGame(app, cpu, at, kStrncpy, {f.buffer, f.target, reg32(int32_t(f.maxLength) - 1), item}, {}, r))
                return {};
        }
        else
            setByte(app, f.buffer, uint8_t(ebx >> 8));
        StringField& g = record<StringField>(app, item);
        setByte(app, g.buffer + reg32(int32_t(g.maxLength)) - 1, 0);
        g.state |= 4;
        return 2;
    }
    if (editing != 1)
        return 0;
    if (key == kEnter)
    {
        f.editing = 0;
        if (f.target != 0
            && !callGame(app, cpu, at, kStrncpy, {f.target, f.buffer, reg32(int32_t(f.maxLength)), item}, {}, r))
            return {};
        if (!told(app, cpu, at, record<StringField>(app, item).changed, item, 0, 0))
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
    const reg32 n = length(app, f.buffer);
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
Result charLaidOut(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x14, at = ebp - 8;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 0xb, r, item, 0))
        return {};
    if (r == 0)
        return 0;
    reg32 off = 0;
    if (record<CharField>(app, item).checked != 0)
    {
        if (!callGame(app, cpu, at, kTextWidth, {record<CharField>(app, item).label, 0x12, item, item}, {}, r))
            return {};
        off = r + 0xa;
    }
    CharField& f = record<CharField>(app, item);
    f.boxX = int16_t(f.x + int32_t(off));
    f.boxY = f.y;
    f.boxHeight = 0x1d;
    reg32 labelWidth, w = 0;
    if (!callGame(app, cpu, at, kTextWidth, {f.label, 0x12, item, item}, {}, labelWidth))
        return {};
    if (record<CharField>(app, item).value != 0
        && !valueWidth(app, cpu, at, item, item + 0x58, ebp - 8,
                       {ebp - 8, kD, byteAt(app, record<CharField>(app, item).value)}, w))
        return {};
    const reg32 e = labelWidth + w + 0xa;
    record<CharField>(app, item).boxWidth = int16_t(e);
    return int32_t(e);
}

}  // namespace

/* 0x457ae0 (stdcall 4): key cx typed into the line eax, edx long, at the
 * cursor [ebx]; dots refused where the stack's word. */
void editKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, keyTyped(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* 0x457c40, 0x458480, 0x458940, 0x458e30: an edint, edchar, edfloat, edfixed
 * eax drawn, lit where edx. */
void drawIntField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, intDrawn(app, cpu, e.esp, e.eax, e.edx));
}

void drawCharField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, charDrawn(app, cpu, e.esp, e.eax, e.edx));
}

void drawFloatField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, floatDrawn(app, cpu, e.esp, e.eax, e.edx, false));
}

void drawFixedField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, floatDrawn(app, cpu, e.esp, e.eax, e.edx, true));
}

/* 0x457df0, 0x458640: "Min Value %d, Max Value %d" of eax and edx (their low
 * bytes for an edchar) put up. */
void intRangeMessage(App* app, CPU& cpu)
{
    const Entry e(cpu);
    HeldEbp held(cpu, e.esp - 0xc);
    pushed(app, e.esp, {e.ebx, e.ecx, e.ebp});
    e.finish(cpu, asked(app, cpu, e.esp - 0xc, {kDialogText, kIntRange, e.eax, e.edx}));
}

void charRangeMessage(App* app, CPU& cpu)
{
    const Entry e(cpu);
    HeldEbp held(cpu, e.esp - 0xc);
    pushed(app, e.esp, {e.ebx, e.ecx, e.ebp});
    e.finish(cpu, asked(app, cpu, e.esp - 0xc, {kDialogText, kIntRange, e.eax & 0xff, e.edx & 0xff}));
}

/* 0x458b20 (stdcall 8): "Min Value %f, Max Value %f" of the stack's floats. */
void floatRangeMessage(App* app, CPU& cpu)
{
    const Entry e(cpu);
    HeldEbp held(cpu, e.esp - 0x10);
    const Words least = wordsOf(double(Float(fromBits(e.arg(app, 0)))));
    const Words most = wordsOf(double(Float(fromBits(e.arg(app, 1)))));
    e.finish(cpu, asked(app, cpu, e.esp - 0x10, {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}), 8);
}

/* 0x459020: of the 16.16 numbers eax and edx. */
void fixedRangeMessage(App* app, CPU& cpu)
{
    const Entry e(cpu);
    HeldEbp held(cpu, e.esp - 0xc);
    const Float unit = Float(kFixedUnit2(app));
    const Words most = wordsOf(double(fp::mul(Float(x86::sreg32(e.edx)), unit)));
    const Words least = wordsOf(double(fp::mul(unit, Float(x86::sreg32(e.eax)))));
    e.finish(cpu, asked(app, cpu, e.esp - 0xc - 8, {kDialogText, kFloatRange, least.lo, least.hi, most.lo, most.hi}));
}

/* 0x457e30, 0x458690, 0x458b80, 0x459090: key bx on an edint, edchar, edfloat,
 * edfixed eax. */
void intFieldKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, intKey(app, cpu, e.esp, e.eax, e.ebx));
}

void charFieldKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, charKey(app, cpu, e.esp, e.eax, e.ebx));
}

void floatFieldKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, floatKey(app, cpu, e.esp, e.eax, e.ebx, false));
}

void fixedFieldKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, floatKey(app, cpu, e.esp, e.eax, e.ebx, true));
}

/* 0x457f70, 0x458340, 0x4587e0, 0x458ce0, 0x459210: the field eax laid out. */
void layoutIntField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 ebp = e.esp - 0x18, at = ebp - 0xc;
    const reg32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    HeldEbp held(cpu, ebp);
    r = laidOut(app, cpu, at, item, 0xa, e.esi, [&](reg32& w) {
                 const IntField& f = record<IntField>(app, item);
                 w = 0;
                 return f.value == 0
                        || valueWidth(app, cpu, at, item, item + 0x60, ebp - 0xc, {ebp - 0xc, kD, word(app, f.value)}, w);
             });
    }
    e.finish(cpu, r);
    if (r)
        popped(app, e.esp, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp});
}

void layoutStringField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 at = e.esp - 0x14;
    HeldEbp held(cpu, at);
    const reg32 item = e.eax;
    e.finish(cpu, laidOut(app, cpu, at, item, 9, e.esi, [&](reg32& w) {
                 const reg32 target = record<StringField>(app, item).target;
                 w = 0;
                 return target == 0 || callGame(app, cpu, at, kTextWidth, {target, 0x12, item, target}, {}, w);
             }));
}

void layoutCharField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.ebp});
    const Result r = charLaidOut(app, cpu, e.esp, e.eax);
    e.finish(cpu, r);
    if (r)
        popped(app, e.esp, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.ebp});
}

void layoutFloatField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 ebp = e.esp - 0x18, at = ebp - 0x14;
    const reg32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    HeldEbp held(cpu, ebp);
    r = laidOut(app, cpu, at, item, 0xc, e.esi, [&](reg32& w) {
                 const FloatField& f = record<FloatField>(app, item);
                 w = 0;
                 if (f.value == 0)
                     return true;
                 const reg32 whole = f.editing != 0 ? 0 : reg32(chopped<int32_t>(cpu, Float(floatAt(app, f.value))));
                 return valueWidth(app, cpu, at, item, item + 0x64, ebp - 0x14, {ebp - 0x14, kF6, whole}, w);
             });
    }
    e.finish(cpu, r);
    if (r)
        popped(app, e.esp, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp});
}

void layoutFixedField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 ebp = e.esp - 0x18, at = ebp - 0x10;
    const reg32 item = e.eax;
    pushed(app, e.esp, {e.ebx, e.ecx, e.edx, e.esi, e.edi, e.ebp});
    Result r;
    {
    HeldEbp held(cpu, ebp);
    r = laidOut(app, cpu, at, item, 0xd, e.esi, [&](reg32& w) {
                 const FloatField& f = record<FloatField>(app, item);
                 w = 0;
                 return f.value == 0
                        || valueWidth(app, cpu, at, item, item + 0x64, ebp - 0x10, {ebp - 0x10, kF6, word(app, f.value)},
                                      w);
             });
    }
    e.finish(cpu, r);
    if (r)
        popped(app, e.esp, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp});
}

/* 0x458010, 0x458050 ...: the field eax shown (its function told), hidden. */
void intFieldShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0x10, e.eax, 0xa, 0x6c, 0x58, e.ebx, true));
}

void intFieldHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0xc, e.eax, 0xa, 0, 0, e.ebx, false));
}

void charFieldShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0x10, e.eax, 0xb, 0x60, 0x50, e.ebx, true));
}

void charFieldHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0xc, e.eax, 0xb, 0, 0, e.ebx, false));
}

void floatFieldShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0x10, e.eax, 0xc, 0x74, 0x58, e.ebx, true));
}

void floatFieldHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0xc, e.eax, 0xc, 0, 0, e.ebx, false));
}

void fixedFieldShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0x10, e.eax, 0xd, 0x74, 0x58, e.ebx, true));
}

void fixedFieldHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldShown(app, cpu, e.esp - 0xc, e.eax, 0xd, 0, 0, e.ebx, false));
}

/* 0x458080, 0x458910, 0x458e00, 0x459320: the field eax's value at edx, its
 * function ebx. */
void bindIntField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldBound(app, cpu, e.esp - 0xc, e.eax, 0xa, e.edx, e.ebx, 0x58, 0x6c));
}

void bindCharField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldBound(app, cpu, e.esp - 0xc, e.eax, 0xb, e.edx, e.ebx, 0x50, 0x60));
}

void bindFloatField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldBound(app, cpu, e.esp - 0xc, e.eax, 0xc, e.edx, e.ebx, 0x58, 0x74));
}

void bindFixedField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fieldBound(app, cpu, e.esp - 0xc, e.eax, 0xd, e.edx, e.ebx, 0x58, 0x74));
}

/* 0x4580b0: an edstring eax drawn, lit where edx. */
void drawStringField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stringDrawn(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

/* 0x4581f0: key bx on an edstring eax. */
void stringFieldKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stringKey(app, cpu, e.esp, e.eax, e.ebx));
}

/* 0x4583b0: an edstring eax shown: its buffer (`maxLength`, 11 for none, and
 * its ending) allocated. */
void stringFieldShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 at = e.esp - 0x14, item = e.eax;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 9, r, e.ebx, item))
        return;
    if (r != 0)
    {
        StringField& f = record<StringField>(app, item);
        if (f.editing == 1)
            f.editing = 0;
        if (f.maxLength == 0)
            f.maxLength = 0xb;
        if (!callGame(app, cpu, at, kAllocate, {kNoName, reg32(int32_t(f.maxLength) + 1), 0, item}, {}, r))
            return;
        record<StringField>(app, item).buffer = r;
    }
    e.back(cpu, r);
}

/* 0x458410: an edstring eax hidden: its buffer freed. */
void stringFieldHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 at = e.esp - 0x10, item = e.eax;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 9, r, item, item))
        return;
    if (r != 0)
    {
        StringField& f = record<StringField>(app, item);
        if (f.editing == 1)
            f.editing = 0;
        if (f.buffer != 0)
        {
            if (!callGame(app, cpu, at, kFree, {f.buffer, f.buffer, item, item}, {}, r))
                return;
            record<StringField>(app, item).buffer = 0;
        }
    }
    e.back(cpu, r);
}

/* ======== The list of lines (List) ======== */

namespace
{

ListBox& listBox(App* app, reg32 item)
{
    return record<ListBox>(app, item);
}

/* 0x4d8570's: the triangle (x0, y0), (x1, y1), (x2, y2), a colour at each
 * corner, in the 2D screens' quad's first three corners -- over a race near
 * the eye, its corners put on the screen's own (x2's in its stack word, at
 * `x2At`) --, clipped and drawn. */
Result triangle(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, reg32 x2At,
                int32_t y2, reg32 c0, reg32 c1, reg32 c2)
{
    const reg32 at = esp - 0xc - 0x14;
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    if (kOverRace(app) != 0)
    {
        for (reg32 corner = 0; corner < 4; ++corner)
        {
            setWord(app, kQuad + 0x20 * corner + 8, kNearDepth);
            setWord(app, kQuad + 0x20 * corner + 0xc, kNearW);
        }
        reg32 sx0, sx1, sx2, sy0, sy1, sy2;
        if (!callGame(app, cpu, at, kToScreenX, {reg32(x0), reg32(y0), reg32(x1), reg32(y1)}, {}, sx0)
            || !callGame(app, cpu, at, kToScreenX, {reg32(x1), reg32(y0), reg32(x1), reg32(y1)}, {}, sx1)
            || !callGame(app, cpu, at, kToScreenX, {word(app, x2At), reg32(y0), sx1, reg32(y1)}, {}, sx2))
            return {};
        setWord(app, x2At, sx2);
        if (!callGame(app, cpu, at, kToScreenY, {reg32(y0), reg32(y0), sx1, reg32(y1)}, {}, sy0)
            || !callGame(app, cpu, at, kToScreenY, {reg32(y1), sy0, sx1, reg32(y1)}, {}, sy1)
            || !callGame(app, cpu, at, kToScreenY, {reg32(y2), sy0, sx1, sy1}, {}, sy2))
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
    if (!callGame(app, cpu, at, kRenderState, {1, 0, reg32(x1), reg32(y1)}, {}, r))
        return {};
    const reg32 ecx = markCorners(app, 3);
    if (!callGame(app, cpu, at, kClipTriangle, {kQuad, kQuad + 0x20, kQuad + 0x40, ecx}, {}, r))
        return {};
    return int32_t(r);
}

/* A call of the triangle as the list makes it: its five stack words below
 * `at`, x2 among them. */
Result listTriangle(App* app, CPU& cpu, reg32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2,
                    int32_t y2, reg32 colour)
{
    const reg32 words = at - 0x14;
    setWord(app, words, reg32(x2));
    setWord(app, words + 4, reg32(y2));
    for (reg32 i = 2; i < 5; ++i)
        setWord(app, words + 4 * i, colour);
    return triangle(app, cpu, words - 4, x0, y0, x1, y1, words, y2, colour, colour, colour);
}

/* 0x465500's: the list drawn: its rows' background, a line above and below,
 * its lines from the first shown (the chosen lit, a marked one in <> and
 * white); unless it shows one row, its scroll bar and arrows, bright where
 * there is more to scroll to. */
Result listBoxDrawn(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x118, text = ebp - 0x118;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!callGame(app, cpu, at, kTextExtent, {kAg, ebp - 0x18, ebp - 0x18, ebp - 0x18}, {0x678a40}, r))
        return {};
    const auto bottom = [&] {
        return int32_t(reg32(int32_t(listBox(app, item).y))
                       + reg32(int32_t(listBox(app, item).rows)) * reg32(kLineHeight(app)));
    };
    const auto right = [&] { return int32_t(listBox(app, item).x) + listBox(app, item).width; };
    const auto gradient = [&](int32_t a, int32_t b, int32_t c, int32_t d, reg32 from, reg32 to) {
        return callGame(app, cpu, at, kGradient, {reg32(a), reg32(b), reg32(c), reg32(d)}, {from, to}, r);
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
        const reg32 line = word(app, l.lines + reg32(n) * 8);
        const uint8_t mark = byteAt(app, line);
        setByte(app, line, uint8_t(mark & 0x7f));
        if (!format(app, cpu, at, {text, (mark & 0x80) != 0 ? kMarked : kPlain, line}))
            return {};
        const reg32 colour = int32_t(i) + listBox(app, item).first == listBox(app, item).chosen ? 0xffe440
                             : (mark & 0x80) != 0                                               ? 0xffffff
                                                                                                : 0x908080;
        if (!callGame(app, cpu, at, kDrawMenuText,
                      {text, reg32(int32_t(listBox(app, item).x)), reg32(int32_t(lineY)), 0x12}, {0, colour}, r))
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
    reg32 colour = more ? 0xffaa00aa : 0xff505050;
    if (!gradient(right() - 0x1e, listBox(app, item).y + 1, right(), bottom(), colour, colour))
        return {};
    colour = more ? 0xffaa5000 : 0xff000000;
    if (!gradient(right() - 0x1e, listBox(app, item).y + 1, right(), listBox(app, item).y + 0x19, colour, colour)
        || !gradient(right() - 0x1e, bottom(), right(), bottom() - 0x19, colour, colour))
        return {};
    colour = more ? 0xffaaaaaa : 0xff505050;
    const int32_t y = listBox(app, item).y;
    if (!listTriangle(app, cpu, at, right() - 0xf, y + 4, right() - 4, y + 0x15, right() - 6, y + 0x15, colour)
        || !listTriangle(app, cpu, at, right() - 0xf, y + 4, right() - 0x18, y + 0x15, right() - 0x1a, y + 0x15,
                         colour)
        || !listTriangle(app, cpu, at, right() - 0xf, bottom() - 4, right() - 4, bottom() - 0x15, right() - 6,
                         bottom() - 0x15, colour)
        || !listTriangle(app, cpu, at, right() - 0xf, bottom() - 4, right() - 0x18, bottom() - 0x15, right() - 0x1a,
                         bottom() - 0x15, colour))
        return {};
    return 0;
}

/* 0x465880's: the pointer put at the chosen line's right end (left of the
 * scroll bar where there is one). */
Result pointedAtLine(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const ListBox& l = listBox(app, item);
    int32_t x = int32_t(l.boxWidth) + l.boxX - 2;
    const int32_t shown = int32_t(l.chosen) - l.first;
    const int32_t y = int32_t(reg32(kLineHeight(app)) * reg32(shown) + reg32(int32_t(l.boxY)) + 2);
    kPointerY.set(app, y);
    if (l.rows > 1)
        x -= 0x1e;
    kPointerX.set(app, x);
    reg32 r;
    if (!callGame(app, cpu, esp - 0x14, kSetPointer,
                  {reg32(x), reg32(kPointerY(app)), reg32(shown), reg32(int32_t(l.boxY))}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4658f0's, 0x465950's: the line after or before chosen, the lines
 * scrolled to it, the pointer put on it unless `quiet`; nothing while the
 * list takes no keys. */
Result listMoved(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 quiet, bool down)
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
    return pointedAtLine(app, cpu, esp - 8 - 4, item);
}

/* 0x4659a0's: the part of the list at x, y: 0 its lines (or a list of one
 * row), the scroll bar's 1 up arrow, 2 middle, 3 down arrow. */
int32_t partAt(App* app, reg32 item, int32_t x, int32_t y)
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
Result chosenAt(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 edx, int32_t x, int32_t y)
{
    const reg32 ebp = esp - 0xc, at = ebp - 4;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 7, r, reg32(x), reg32(y)))
        return {};
    if (r == 0)
        return 0;
    ListBox& l = listBox(app, item);
    const int32_t line = int32_t(int64_t(int32_t(reg32(y) - reg32(int32_t(l.boxY)))) / kLineHeight(app));
    l.chosen = int16_t(line + l.first);
    if (l.choose == 0)
        return 0;
    if (!callGame(app, cpu, at, l.choose, {item, edx, reg32(int32_t(l.chosen)), 0}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x465a30's: a key on the list: up, down; Enter chooses the line under the
 * pointer -- an arrow of the scroll bar steps instead, quietly --, the one
 * row of a list of one.  2 for a key taken. */
Result listBoxKeyed(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 edx, reg32 ebx)
{
    const reg32 ebp = esp - 0x10, at = ebp - 4;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 7, r, ebx, 0))
        return {};
    if (r == 0)
        return 0;
    const uint16_t key = uint16_t(ebx);
    if (key == kUp || key == kDown)
    {
        if (!listMoved(app, cpu, at - 4, item, 0, key == kDown))
            return {};
        return 2;
    }
    if (key != kEnter)
        return 0;
    const ListBox& l = listBox(app, item);
    if (l.rows == 1)
    {
        if (!chosenAt(app, cpu, at - 4 - 4, item, edx, int32_t(l.x) + l.width - 2, l.y))
            return {};
        return 2;
    }
    if (partAt(app, item, kPointerX(app), kPointerY(app)) == 0)
    {
        if (!chosenAt(app, cpu, at - 4 - 4, item, edx, kPointerX(app), kPointerY(app)))
            return {};
        return 2;
    }
    if (listBox(app, item).rows >= listBox(app, item).count)
        return 2;
    const int32_t part = partAt(app, item, kPointerX(app), kPointerY(app));
    if ((part == 1 || part == 3) && !listMoved(app, cpu, at - 4, item, 1, part == 3))
        return {};
    return 2;
}

/* 0x465ba0's: the list laid out: the line's height measured, its box at it,
 * its width and rows high.  eax the product as the code makes it. */
Result listBoxLaidOut(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x14, at = ebp - 4;
    HeldEbp held(cpu, ebp);
    reg32 r;
    if (!callGame(app, cpu, at, kTextExtent, {kAg, ebp - 4, ebp - 4, ebp - 4}, {0x678a40}, r)
        || !shownNow(app, cpu, at, item, 7, r, ebp - 4, ebp - 4))
        return {};
    if (r == 0)
        return 0;
    ListBox& l = listBox(app, item);
    l.boxX = l.x;
    l.boxY = l.y;
    l.boxWidth = l.width;
    const reg32 e = ((r & 0xffff0000) | uint16_t(l.rows)) * reg32(uint16_t(kLineHeight(app)));
    l.boxHeight = int16_t(e);
    return int32_t(e);
}

}  // namespace

/* 0x465500: the list eax drawn. */
void drawListBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listBoxDrawn(app, cpu, e.esp, e.eax));
}

/* 0x465880: the pointer on the list eax's chosen line. */
void pointAtListBoxLine(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, pointedAtLine(app, cpu, e.esp, e.eax));
}

/* 0x4658f0, 0x465950: the list eax's next, previous line; quietly where ebx. */
void listBoxDown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listMoved(app, cpu, e.esp, e.eax, e.ebx, true));
}

void listBoxUp(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listMoved(app, cpu, e.esp, e.eax, e.ebx, false));
}

/* 0x4659a0: the part of the list eax at edx, ebx. */
void listBoxPartAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(partAt(app, e.eax, int32_t(e.edx), int32_t(e.ebx))));
}

/* 0x465a10: whether edx, ebx is on the list eax's scroll bar. */
void listBoxArrowAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, partAt(app, e.eax, int32_t(e.edx), int32_t(e.ebx)) != 0 ? 1 : 0);
}

/* 0x465a30: key bx on the list eax (edx handed to its function). */
void listBoxKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listBoxKeyed(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x465ba0: the list eax laid out. */
void layoutListBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, listBoxLaidOut(app, cpu, e.esp, e.eax));
}

/* 0x465c10: the list eax shown: from its first line, 3 rows and 0x118 wide
 * for none. */
void listBoxShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 0x10;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 7, r, item, item))
        return;
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
    e.back(cpu, r);
}

/* 0x465c60: the list eax hidden: its lines freed. */
void listBoxHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 0x10;
    HeldEbp held(cpu, at);
    reg32 r;
    if (!shownNow(app, cpu, at, item, 7, r, item, item))
        return;
    if (r != 0)
    {
        const reg32 lines = listBox(app, item).lines;
        if (lines != 0 && !callGame(app, cpu, at, kFree, {lines, lines, item, item}, {}, r))
            return;
        listBox(app, item).lines = 0;
    }
    e.back(cpu, r);
}

/* 0x465ca0 (stdcall 4): the list eax's line at ecx chosen, its function
 * handed edx. */
void listBoxChooseAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, chosenAt(app, cpu, e.esp, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx)), 4);
}

/* 0x4d8570 (stdcall 0x14): the triangle eax, edx; ebx, ecx; the stack's
 * corner and its three colours. */
void drawShadedTriangle(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             triangle(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.esp + 4,
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
    reg32 title;          // +0x60
    int16_t titleX, titleY;  // +0x64
    reg32 labels[3];      // +0x68
    uint8_t labelsBelow;  // +0x74
    uint8_t alignment;    // +0x75
    uint8_t unknown76[2];
    reg32 changed;        // +0x78
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

Slider& slider(App* app, reg32 item)
{
    return record<Slider>(app, item);
}

// The sliders' kinds: how many parts (a word each), their parts' art
// (shapes A and B, 0x18 bytes a kind, 4 a part) and the art's names, the
// rectangles' sizes and colours without art (0x30 a kind, 8 a part), the 3D
// card's quads' texture places (0xc0 a kind, 0x20 a part; lit ones apart).
constexpr reg32 kSliderParts = 0x55a4e0, kPartShapes = 0x678ab0;
constexpr reg32 kPartNamesA = 0x55af9c, kPartNamesB = 0x55af0c;
constexpr reg32 kPartSizes = 0x55adf0, kPartColours = 0x55adec;
constexpr reg32 kPartQuads = 0x55a4ec, kHotPartQuads = 0x55a96c;
constexpr Var<reg32> kArtTexture{0x749a38};
// The menus' rows a slider may be placed by; the screens sliding in; the
// screen's size; the menus' state bits; the clock; the renderer's flags.
constexpr reg32 kRowsX = 0x559c6c, kRowsY = 0x559c94;
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
constexpr reg32 kPercent = 0x53ae54, kTwoPercents = 0x53ae84;
constexpr reg32 kOne = 0x3f800000, kHundredth = 0x3c23d70a;
// The code's blanks it copies into its frames (a slider's, a two-axis one's).
constexpr reg32 kSliderBlank = 0x469020, kTwoAxisBlank = 0x469040;
// What they call: the clip (graphics'), floor, EA's shape found by name, the
// mouse holding a control, GetKeyState's slot; the port's.
constexpr reg32 kSetClip = 0x4bebe0, kFloor = 0x4f1c30, kLocateShape = 0x4ef270, kMouseHolds = 0x49c080;
constexpr reg32 kGetKeyStateSlot = 0x534738, kArtShapes = 0x8b48c0, kStrncmp = 0x4eb390;
constexpr Var<int32_t> kArtShapeCount{0x8b7628};
constexpr Var<reg32> kArtFile{0x559228};
constexpr guest::Native kTexturedQuad = draw2d::drawTexturedQuad, kShadedQuad = draw2d::drawShadedQuad;
constexpr guest::Native kViewport = text::viewport;
constexpr guest::Native kShapeSize = shapes::shapeSize, kArtTextures = shapes::artShapeTextures;
constexpr guest::Native kDrawArt = shapes::drawArtShape;

int32_t partsOf(App* app, int32_t kind)
{
    return half(app, kSliderParts + 2 * reg32(kind));
}

/* Art shape A or B of part `part` of kind `kind` (-1 none). */
reg32 shapeAt(int32_t kind, int32_t part, int b)
{
    return kPartShapes + 0x18 * reg32(kind) + 4 * reg32(part) + 2 * reg32(b);
}

Float floatOf(uint32_t bits)
{
    return Float(fromBits(bits));
}

/* Words written below `at` as the code pushes them, the first the lowest:
 * the esp of the function then called. */
reg32 argsBelow(App* app, reg32 at, std::initializer_list<reg32> words)
{
    reg32 slot = at - 4 * reg32(words.size());
    const reg32 callee = slot - 4;
    for (const reg32 w : words)
    {
        setWord(app, slot, w);
        slot += 4;
    }
    return callee;
}

/* The 640 by 480 measure's place `v` on the screen's own, as the code works
 * it out: the product (the screen's size times v) by 1/640 or 1/480,
 * truncated. */
reg32 onScreen(CPU& cpu, reg32 product, double per)
{
    return reg32(chopped<int32_t>(cpu, fp::mul(Float(x86::sreg32(product)), Float(per))));
}

/* ---- Their parts and values ---- */

/* 0x4692c0's: the part of the slider under the pointer, or -1. */
int32_t partUnder(App* app, reg32 item)
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
int32_t thumbPlaced(App* app, CPU& cpu, reg32 item, reg32 startAt, reg32 endAt, reg32 lengthAt, reg32 placeAt)
{
    const Slider& s = slider(app, item);
    const int32_t halfThumb = halved(s.parts[0].width);
    setHalf(app, startAt, halfThumb + s.parts[1].x);
    const int32_t end = s.parts[1].x + (s.parts[1].width - halfThumb);
    setHalf(app, endAt, end);
    setHalf(app, lengthAt, end - half(app, startAt));
    const int32_t place =
        chopped<int32_t>(cpu, fp::mul(Float(int16_t(half(app, lengthAt))), floatOf(slider(app, item).values[0])));
    setHalf(app, placeAt, place);
    return place;
}

/* 0x4693c0's: the value at [esp+8] snapped to the step at [esp+4] -- to the
 * step below it, or the one above where it is past a half of the way -- and
 * kept within 0 and 1: a step past 1 is 1, one below 0 none (written back
 * to their words).  On the x87's stack as the code leaves it. */
std::optional<Float> snapped(App* app, CPU& cpu, reg32 esp)
{
    const reg32 ebp = esp - 0xc, at = ebp - 0x10, stepAt = esp + 4, valueAt = esp + 8;
    HeldEbp held(cpu, ebp);
    bool snap = true;
    if (int32_t(word(app, stepAt)) > int32_t(kOne))
        setWord(app, stepAt, kOne);
    else
    {
        cpu.fpu.compare(Float(0.0), Float(floatAt(app, valueAt)));
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            setWord(app, stepAt, 0);
        snap = (word(app, stepAt) & 0x7fffffff) != 0;
    }
    if (snap)
    {
        const Float steps = fp::div(Float(floatAt(app, valueAt)), Float(floatAt(app, stepAt)));
        setFloat(app, ebp - 0x10, float(steps));
        const Words d = wordsOf(double(steps));
        if (!callGame(app, cpu, at, kFloor, {}, {d.lo, d.hi}))
            return {};
        setFloat(app, ebp - 8, float(cpu.fpu.pop()));
        const Float rest = fp::sub(Float(floatAt(app, ebp - 0x10)), Float(floatAt(app, ebp - 8)));
        setFloat(app, ebp - 0xc, float(fp::abs(rest)));
        if ((word(app, ebp - 0xc) & 0x7fffffff) != 0)
        {
            cpu.fpu.compare(Float(floatAt(app, ebp - 0xc)), Float(kHalf(app)));
            const Float whole = !cpu.fpu.status.c0 ? fp::add(Float(1.0), Float(floatAt(app, ebp - 8)))
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
        cpu.fpu.compare(Float(0.0), Float(floatAt(app, ebp - 4)));
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            setWord(app, ebp - 4, 0);
    }
    return Float(floatAt(app, ebp - 4));
}

/* snapped() as the code calls it: the value and the step pushed below `at`. */
std::optional<Float> snappedAt(App* app, CPU& cpu, reg32 at, reg32 step, reg32 value)
{
    return snapped(app, cpu, argsBelow(app, at, {step, value}));
}

/* The slider's function told of a change; eax else. */
Result changeTold(App* app, CPU& cpu, reg32 at, reg32 item, reg32 eax, reg32 edx)
{
    const reg32 function = slider(app, item).changed;
    if (function == 0)
        return int32_t(eax);
    reg32 r;
    if (!callGame(app, cpu, at, function, {item, edx, function, 0}, {}, r))
        return {};
    return int32_t(r);
}

/* The thumb (part `left`'s x, its width after it) moved to its track's start
 * (the word at `startAt`) and its place (at `placeAt`) less half its width:
 * that half, as eax has it. */
reg32 thumbMoved(App* app, reg32 item, reg32 left, reg32 startAt, reg32 placeAt)
{
    const int32_t h = halved(half(app, item + left + 2));
    setHalf(app, item + left, int32_t(half(app, placeAt)) + half(app, startAt) - h);
    return reg32(h);
}

/* 0x469480's: the slider set to the value at [esp+4] -- within 0 and 1 and
 * snapped to its step, written back there --, its thumb moved there, its
 * function told. */
Result valueSet(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0x18, valueAt = esp + 4;
    HeldEbp held(cpu, ebp);
    const double value = double(Float(floatAt(app, valueAt)));
    app->getMemory<double>(ebp - 0x18) = value;
    cpu.fpu.compare(Float(1.0), Float(value));
    if (cpu.fpu.status.c0)
        setWord(app, valueAt, kOne);
    else
    {
        cpu.fpu.compare(Float(0.0), Float(value));
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            setWord(app, valueAt, 0);
    }
    const std::optional<Float> r = snappedAt(app, cpu, at, slider(app, item).step, word(app, valueAt));
    if (!r)
        return {};
    setFloat(app, valueAt, float(*r));
    slider(app, item).values[0] = word(app, valueAt);
    thumbPlaced(app, cpu, item, ebp - 0x10, ebp - 0xc, ebp - 8, ebp - 4);
    const reg32 h = thumbMoved(app, item, 0x7e, ebp - 0x10, ebp - 4);
    return changeTold(app, cpu, at, item, h, reg32(int32_t(slider(app, item).parts[0].x)));
}

/* 0x469530's, 0x46afe0's: the arrows' step of axis `axis`: the stack's,
 * 1 past 1, a hundredth for none or less.  eax as fnstsw leaves it. */
reg32 arrowStepSet(App* app, CPU& cpu, reg32 item, reg32 step, int32_t axis, reg32 eax)
{
    const reg32 to = item + reg32(axis) * 4 + 0x48;
    const Float value = Float(double(floatOf(step)));
    cpu.fpu.compare(Float(1.0), value);
    if (cpu.fpu.status.c0)
    {
        setWord(app, to, kOne);
        return (eax & 0xffff0000) | cpu.fpu.status.word;
    }
    cpu.fpu.compare(Float(0.0), value);
    setWord(app, to, cpu.fpu.status.c0 ? step : kHundredth);
    return cpu.fpu.status.c0 ? step : (eax & 0xffff0000) | cpu.fpu.status.word;
}

/* 0x469590's: the slider dragged to `place` along its track: its value that
 * far of the track's length, snapped to its step, its thumb moved there, its
 * function told. */
Result draggedTo(App* app, CPU& cpu, reg32 esp, reg32 item, int16_t place)
{
    const reg32 ebp = esp - 0x10, at = ebp - 0x1c;
    HeldEbp held(cpu, ebp);
    thumbPlaced(app, cpu, item, ebp - 0x14, ebp - 8, ebp - 0x18, ebp - 4);
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
    const std::optional<Float> r = snappedAt(app, cpu, at, slider(app, item).step, word(app, ebp - 0x1c));
    if (!r)
        return {};
    setFloat(app, ebp - 0x1c, float(*r));
    slider(app, item).values[0] = word(app, ebp - 0x1c);
    thumbPlaced(app, cpu, item, ebp - 0x14, ebp - 8, ebp - 0x18, ebp - 4);
    const reg32 h = thumbMoved(app, item, 0x7e, ebp - 0x14, ebp - 4);
    return changeTold(app, cpu, at, item, h, reg32(int32_t(slider(app, item).parts[0].x)));
}

/* 0x46ac40's: where thumb `part` of the two-axis slider may go along axis
 * `axis` -- axes 0 and 1 parts 0 and 1 from its area's left, 2 and 3 from its
 * middle; another axis part 4 along its bar --: its start to `startAt`, its
 * end to `endAt`, the length between to `lengthAt`, its place (the length
 * times the axis's value) to `placeAt`.  eax: the place, or the slider for a
 * part the axis has not. */
int32_t twoAxisPlaced(App* app, CPU& cpu, reg32 item, int32_t axis, reg32 part, reg32 startAt, reg32 endAt,
                      reg32 lengthAt, reg32 placeAt)
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
        cpu, fp::mul(Float(int16_t(half(app, lengthAt))), floatOf(word(app, item + 0x54 + 4 * reg32(axis)))));
    setHalf(app, placeAt, place);
    return place;
}

/* The place from the track's far end: the length (at `lengthAt`) less it. */
void fromFarEnd(App* app, reg32 lengthAt, reg32 placeAt)
{
    setHalf(app, placeAt, int32_t(word(app, lengthAt) - word(app, placeAt)));
}

/* 0x46add0's: axis [esp+8] of the two-axis slider set to the value at
 * [esp+4] -- within 0 and 1 and snapped to its step, written back --, its
 * thumbs moved there: across parts 0 and 2 (the second from the far end), up
 * 1 (from the far end) and 3, for axis 2 part 4; its function told. */
Result twoAxisValueSet(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 eax)
{
    const reg32 ebp = esp - 0x18, at = ebp - 0x10, valueAt = esp + 4;
    HeldEbp held(cpu, ebp);
    const int32_t axis = int32_t(word(app, esp + 8));
    if (int32_t(word(app, valueAt)) > int32_t(kOne))
        setWord(app, valueAt, kOne);
    else
    {
        cpu.fpu.compare(Float(0.0), Float(floatAt(app, valueAt)));
        eax = (eax & 0xffff0000) | cpu.fpu.status.word;
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            setWord(app, valueAt, 0);
    }
    const std::optional<Float> r = snappedAt(app, cpu, at, slider(app, item).step, word(app, valueAt));
    if (!r)
        return {};
    setFloat(app, valueAt, float(*r));
    setWord(app, item + 0x54 + 4 * reg32(axis), word(app, valueAt));
    const reg32 start = ebp - 0xc, end = ebp - 8, length = ebp - 0x10, place = ebp - 4;
    const auto placed = [&](reg32 part) {
        twoAxisPlaced(app, cpu, item, axis, part, start, end, length, place);
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
        eax = reg32(h);
    }
    return changeTold(app, cpu, at, item, eax, 0);
}

/* 0x46b040's: axis `axis` (0 across, 1 up) of the two-axis slider dragged to
 * `to` along its track: each of its thumbs on the axis placed, the value that
 * far of the track's length snapped to its step, the thumb moved there (the
 * second from the far end), its function told. */
Result twoAxisDragged(App* app, CPU& cpu, reg32 esp, reg32 item, int16_t to, int32_t axis)
{
    const reg32 ebp = esp - 0x10, at = ebp - 0x20;
    HeldEbp held(cpu, ebp);
    setHalf(app, ebp - 8, to);
    const reg32 start = ebp - 0x1c, end = ebp - 0x14, length = ebp - 0x18, place = ebp - 0x10;
    const auto placed = [&](reg32 part) {
        twoAxisPlaced(app, cpu, item, axis, part, start, end, length, place);
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
    const auto value = [&](reg32 off) {
        setWord(app, ebp - 4, word(app, place));
        setWord(app, ebp - 0xc, word(app, length));
        setFloat(app, ebp - 0x20,
                 float(fp::div(Float(int16_t(half(app, ebp - 4))), Float(int16_t(half(app, ebp - 0xc))))));
        const std::optional<Float> r = snappedAt(app, cpu, at, slider(app, item).step, word(app, ebp - 0x20));
        if (!r)
            return false;
        setFloat(app, ebp - 0x20, float(*r));
        setWord(app, item + off, word(app, ebp - 0x20));
        return true;
    };
    reg32 eax = item;
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
    return changeTold(app, cpu, at, item, eax, 0);
}

/* ---- The mouse and the keys ---- */

/* 0x46a5f0's: the slider as the mouse works it, while it holds the control:
 * the part under the pointer hot -- the thumb taken up, an arrow starting
 * its repeat -- the thumb taken up dragged to the pointer, an arrow held
 * stepping the value every 0x20 ticks by its step (or the arrows' where that
 * is not within 0 and 1).  Let go, the thumb put down, the repeat stopped. */
Result sliderTracked(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x18, at = ebp - 0x14;
    HeldEbp held(cpu, ebp);
    reg32 e;
    if (!callGame(app, cpu, at, kMouseHolds, {item, item}, {}, e))
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
    e = reg32(part);
    if (part == 0)
    {
        s.held = 1;
        s.state |= 4;
        e = (e & 0xffff00ff) | reg32(s.state) << 8;
    }
    else if (part == 2 && s.repeat != 2)
    {
        e = reg32(kClock(app));
        s.repeat = 2;
        s.repeatClock = int32_t(e);
        s.state |= 4;
    }
    else if (part == 3 && s.repeat != 3)
    {
        e = reg32(kClock(app));
        s.repeat = 3;
        s.repeatClock = int32_t(e);
        s.state |= 4;
    }
    if (slider(app, item).held != 0)
    {
        if (part >= 0 && part < partsOf(app, slider(app, item).kind))
        {
            e = reg32(thumbPlaced(app, cpu, item, ebp - 0x10, ebp - 8, ebp - 0xc, ebp - 4));
            const int32_t x = kPointerX(app);
            if (half(app, ebp - 8) < x)
                e = word(app, ebp - 0xc);
            else if (half(app, ebp - 0x10) > x)
                e = 0;
            else
                e = ((reg32(int32_t(half(app, ebp - 0x10))) & 0xffff0000) | uint16_t(x)) - word(app, ebp - 0x10);
            if (uint16_t(e) != uint16_t(half(app, ebp - 4)))
            {
                const Result r = draggedTo(app, cpu, at - 4, item, int16_t(e));
                if (!r)
                    return {};
                e = reg32(*r);
            }
        }
        else
            slider(app, item).held = 0;
    }
    // The step: its own, or the arrows' where it is not within 0 and 1.
    cpu.fpu.compare(Float(0.0), floatOf(slider(app, item).step));
    e = (e & 0xffff0000) | cpu.fpu.status.word;
    reg32 step;
    if (cpu.fpu.status.c0 && int32_t(slider(app, item).step) <= int32_t(kOne))
        step = slider(app, item).step;
    else
        e = step = slider(app, item).arrowSteps[0];
    setWord(app, ebp - 0x14, step);
    const uint8_t repeat = slider(app, item).repeat;
    if ((repeat == 2 && part == 2) || (repeat == 3 && part == 3))
    {
        e = reg32(kClock(app)) - reg32(slider(app, item).repeatClock);
        if (e < 0x20)
            return int32_t(e);
        Float v = floatOf(slider(app, item).values[0]);
        if (repeat == 2)
        {
            v = fp::sub(v, floatOf(step));
            setFloat(app, item + 0x54, float(v));
            cpu.fpu.compare(Float(0.0), v);
            if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
                slider(app, item).values[0] = 0;
        }
        else
        {
            v = fp::add(v, floatOf(step));
            setFloat(app, item + 0x54, float(v));
            cpu.fpu.compare(Float(1.0), v);
            if (cpu.fpu.status.c0)
                slider(app, item).values[0] = kOne;
        }
        if (!valueSet(app, cpu, argsBelow(app, at, {slider(app, item).values[0], 0}), item))
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
Result twoAxisTracked(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 ebp = esp - 0x18, at = ebp - 0x10;
    HeldEbp held(cpu, ebp);
    reg32 e;
    if (!callGame(app, cpu, at, kMouseHolds, {item, item}, {}, e))
        return {};
    Slider& s = slider(app, item);
    if (e == 0)
    {
        if (uint16_t(s.held) == 0xffff)
            return 0;
        s.held = -1;
        s.state &= 0xfb;
        return int32_t(reg32(s.state) << 8);
    }
    if (uint16_t(s.held) == 0xffff)
    {
        e = reg32(partUnder(app, item));
        if (e <= 3)
        {
            slider(app, item).held = int16_t(e);
            slider(app, item).state |= 4;
        }
    }
    e = (e & 0xffff0000) | uint16_t(slider(app, item).held);
    if (slider(app, item).held < 0)
        return int32_t(e);
    e = reg32(partsOf(app, slider(app, item).kind));
    if (slider(app, item).held >= int32_t(e))
        return int32_t(e);
    const uint16_t part = uint16_t(slider(app, item).held);
    e = (e & 0xffff0000) | part;
    if (part > 3)
        return int32_t(e);
    const int32_t axis = part & 1;
    e = reg32(twoAxisPlaced(app, cpu, item, axis, reg32(int32_t(slider(app, item).held)), ebp - 4, ebp - 0x10,
                            ebp - 0xc, ebp - 8));
    const int32_t x = kPointerX(app);
    if (half(app, ebp - 0x10) < x)
        e = word(app, ebp - 0xc);
    else if (half(app, ebp - 4) > x)
        e = 0;
    else
        e = ((reg32(int32_t(half(app, ebp - 4))) & 0xffff0000) | uint16_t(x)) - word(app, ebp - 4);
    if (uint16_t(e) == uint16_t(half(app, ebp - 8)))
        return int32_t(e);
    const uint16_t heldPart = uint16_t(slider(app, item).held);
    const int16_t to = heldPart == 2 || heldPart == 1 ? int16_t(word(app, ebp - 0xc) - e) : int16_t(e);
    return twoAxisDragged(app, cpu, at - 4, item, to, axis);
}

/* 0x46a860's: a key on a slider (none while its thumb is held): left steps
 * its value down, right up, Enter on an arrow as that arrow -- by its step,
 * or the arrows' where that is not within 0 and 1.  2 for a key taken. */
Result sliderKeyed(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 at = esp - 0x10 - 4;
    HeldEbp held(cpu, esp - 0x10);
    const uint16_t key = uint16_t(ebx);
    cpu.fpu.compare(Float(0.0), floatOf(slider(app, item).step));
    const reg32 step = cpu.fpu.status.c0 && int32_t(slider(app, item).step) <= int32_t(kOne)
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
        cpu.fpu.compare(Float(0.0), v);
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            slider(app, item).values[0] = 0;
    }
    else
    {
        v = fp::add(v, floatOf(step));
        setFloat(app, item + 0x54, float(v));
        cpu.fpu.compare(Float(1.0), v);
        if (cpu.fpu.status.c0)
            slider(app, item).values[0] = kOne;
    }
    if (!valueSet(app, cpu, argsBelow(app, at, {slider(app, item).values[0], 0}), item))
        return {};
    return 2;
}

/* 0x46c4a0's: a key on a two-axis slider (none while a thumb is held): left
 * steps it down, right up -- across with Shift held (GetKeyState), up else
 * -- by its step, or the axis's arrows' where that is not within 0 and 1.
 * 2 for a key taken. */
Result twoAxisKeyed(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 ebx)
{
    const reg32 ebp = esp - 0x10, at = ebp - 0x10;
    HeldEbp held(cpu, ebp);
    const uint16_t key = uint16_t(ebx);
    if (uint16_t(slider(app, item).held) != 0xffff)
        return 0;
    if (key != kLeft && key != kRight)
        return 0;
    reg32 r;
    if (!callGame(app, cpu, at, word(app, kGetKeyStateSlot), {ebx, 0, item, uint16_t(slider(app, item).held)}, {0x10}, r))
        return {};
    const int32_t axis = (r & 0x8000) != 0 ? 0 : 1;
    cpu.fpu.compare(Float(0.0), floatOf(slider(app, item).step));
    const reg32 step = cpu.fpu.status.c0 && int32_t(slider(app, item).step) <= int32_t(kOne)
                           ? slider(app, item).step
                           : slider(app, item).arrowSteps[axis];
    const reg32 valueAt = item + 0x54 + 4 * reg32(axis);
    Float v = floatOf(word(app, valueAt));
    if (key == kLeft)
    {
        v = fp::sub(v, floatOf(step));
        setFloat(app, valueAt, float(v));
        cpu.fpu.compare(Float(0.0), v);
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
            setWord(app, valueAt, 0);
    }
    else
    {
        v = fp::add(v, floatOf(step));
        setFloat(app, valueAt, float(v));
        cpu.fpu.compare(Float(1.0), v);
        if (cpu.fpu.status.c0)
            setWord(app, valueAt, kOne);
    }
    if (!twoAxisValueSet(app, cpu, argsBelow(app, at, {word(app, valueAt), reg32(axis)}), item, item))
        return {};
    return 2;
}

/* ---- Their layout ---- */

/* 0x469080's: each part's art -- found by name or made (sliderShape), none
 * over a race -- and its size: the art's, or the kind's own where there is no
 * art or it is empty.  eax the kind. */
Result partSizes(App* app, CPU& cpu, reg32 esp, reg32 item);

/* 0x4d5e40's: the menus' shape named `name` (4 characters): among those
 * known, or made from the menus' art (EA's shape of that name, made textures,
 * named); -1 where the art has none. */
Result shapeOf(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 ebx)
{
    const reg32 at = esp - 0x14;
    HeldEbp held(cpu, at);
    reg32 r;
    for (int32_t i = 0; i < kArtShapeCount(app); ++i)
    {
        ebx = 4;
        if (!callGame(app, cpu, at, kStrncmp, {kArtShapes + reg32(i) * 0x14 + 0x10, name, 4, name}, {}, r))
            return {};
        if (r == 0)
            return i;
    }
    reg32 shape, made;
    if (!callGame(app, cpu, at, kLocateShape, {kArtFile(app), name, ebx, name}, {}, shape))
        return {};
    if (shape == 0)
        return -1;
    if (!callGame(app, cpu, at, kArtTextures, {shape, 0, ebx, name}, {}, made))
        return {};
    if (made != 0 && !callGame(app, cpu, at, kStrncpy, {kArtShapes + made * 0x14 + 0x10, name, 4, name}, {}, r))
        return {};
    return int32_t(made);
}

Result partSizes(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 at = esp - 0x18 - 4;
    HeldEbp held(cpu, esp - 0x18);
    for (int32_t i = 0;; ++i)
    {
        const int32_t kind = slider(app, item).kind;
        if (i >= partsOf(app, kind))
            return kind;
        const reg32 a = shapeAt(kind, i, 0), b = shapeAt(kind, i, 1);
        const reg32 sizes = kPartSizes + 0x30 * reg32(kind) + 8 * reg32(i);
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
            const reg32 nameB = word(app, kPartNamesB + 0x18 * reg32(kind) + 4 * reg32(i));
            if (nameB != 0)
            {
                const Result r = shapeOf(app, cpu, at - 4, nameB, nameB);
                if (!r)
                    return {};
                setHalf(app, b, *r);
            }
            else
                setHalf(app, b, -1);
            const reg32 ebxA = nameB != 0 ? reg32(int32_t(half(app, b))) : 0;
            const reg32 nameA = word(app, kPartNamesA + 0x18 * reg32(kind) + 4 * reg32(i));
            if (nameA != 0)
            {
                const Result r = shapeOf(app, cpu, at - 4, nameA, ebxA);
                if (!r)
                    return {};
                setHalf(app, a, *r);
            }
            else
                setHalf(app, a, -1);
        }
        const reg32 partAt = item + 0x7c + 8 * reg32(i);
        reg32 r;
        if (half(app, b) != -1
            && !callGame(app, cpu, at, kShapeSize, {reg32(int32_t(half(app, b))), partAt + 4, partAt + 6, item}, {}, r))
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
reg32 sliderPlaced(App* app, reg32 item)
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
    return reg32(h3);
}

/* 0x46c640's: a two-axis slider's parts placed about its area's middle: the
 * thumbs across on the area's middle, those up and the bar's on its top. */
reg32 twoAxisPlacedParts(App* app, reg32 item)
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
    return reg32(h4);
}

/* Placed by its row of the menus' table where it asks for it. */
void placedByRow(App* app, reg32 item)
{
    Slider& s = slider(app, item);
    if (s.ownPlace != 0)
    {
        s.x = int16_t(half(app, kRowsX + 4 * reg32(int32_t(s.row))) + 0x94);
        s.y = half(app, kRowsY + 4 * reg32(int32_t(s.row)));
    }
}

/* 0x46ab10's: a slider set up: placed, its arrows' step and its value set,
 * its screen kept (and moved on two, to slide in, on the 2D screens). */
Result sliderSetUp(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 at = esp - 0x10;
    HeldEbp held(cpu, at);
    placedByRow(app, item);
    sliderPlaced(app, item);
    arrowStepSet(app, cpu, item, slider(app, item).arrowSteps[0], 0, item);
    const Result r = valueSet(app, cpu, argsBelow(app, at, {slider(app, item).values[0], 0}), item);
    if (!r)
        return {};
    Slider& s = slider(app, item);
    reg32 e = (reg32(*r) & 0xffff0000) | uint16_t(s.screen);
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
Result twoAxisSetUp(App* app, CPU& cpu, reg32 esp, reg32 item)
{
    const reg32 at = esp - 0x10;
    HeldEbp held(cpu, at);
    placedByRow(app, item);
    twoAxisPlacedParts(app, item);
    reg32 e = item;
    for (int32_t axis = 0; axis < 3; ++axis)
        e = arrowStepSet(app, cpu, item, slider(app, item).arrowSteps[axis], axis, e);
    Result r;
    for (int32_t axis = 0; axis < 3; ++axis)
    {
        r = twoAxisValueSet(app, cpu, argsBelow(app, at, {slider(app, item).values[axis], reg32(axis)}), item, item);
        if (!r)
            return {};
    }
    return r;
}

/* 0x46abb0's, 0x46c8b0's: the box the pointer finds it in: at it, as wide as
 * its track and arrows (a two-axis one's area), as high as its highest part.
 * eax as the code leaves it. */
reg32 sliderBoxed(App* app, reg32 item)
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
    return reg32(int32_t(s.parts[3].height));
}

reg32 twoAxisBoxed(App* app, reg32 item)
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
    return reg32(tallest);
}

/* ---- Their drawing ---- */

/* The menus' greying: a slider not kept from it (+0x18) greyed (+4 bit 0)
 * where the menus grey their controls. */
void greyed(App* app, reg32 item)
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
bool artQuad(App* app, CPU& cpu, reg32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
             int32_t x3, int32_t y3, reg32 places, reg32 colour)
{
    const auto t = [&](reg32 i) { return word(app, places + 4 * i); };
    return callGame(app, cpu, at, kTexturedQuad, {reg32(x0), reg32(y0), reg32(x1), reg32(y1)},
                    {reg32(x2), reg32(y2), reg32(x3), reg32(y3), t(0), t(1), t(2), t(3), t(4), t(5), t(6), t(7),
                     kArtTexture(app), colour});
}

/* A quad of four colours, its corners from (x0, y0) round. */
bool colourQuad(App* app, CPU& cpu, reg32 at, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                int32_t x3, int32_t y3, reg32 c0, reg32 c1, reg32 c2, reg32 c3)
{
    return callGame(app, cpu, at, kShadedQuad, {reg32(x0), reg32(y0), reg32(x1), reg32(y1)},
                    {reg32(x2), reg32(y2), reg32(x3), reg32(y3), c0, c1, c2, c3});
}

/* The clip set to a rectangle of the screen's own. */
bool clipped(App* app, CPU& cpu, reg32 at, reg32 left, reg32 top, reg32 right, reg32 bottom)
{
    return callGame(app, cpu, at, kSetClip, {left, top, right, bottom});
}

/* The screen's viewport kept in the frame at `kept` (left, top, width,
 * height: eax's, edx's, ebx's, ecx's words) and put back. */
bool viewportKept(App* app, CPU& cpu, reg32 at, reg32 left, reg32 top, reg32 width, reg32 height)
{
    return callGame(app, cpu, at, kViewport, {left, top, width, height});
}

/* 0x469680's: a slider drawn. */
Result sliderDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0xc4, text = ebp - 0xc4;
    HeldEbp held(cpu, ebp);
    for (reg32 i = 0; i < 0x20; i += 4)
        setWord(app, text + i, word(app, kSliderBlank + i));
    // The viewport kept: its left, top, width and height.
    const reg32 keptLeft = ebp - 0xa0, keptTop = ebp - 0x94, keptWidth = ebp - 0x9c, keptHeight = ebp - 0x98;
    const auto restore = [&] {
        return clipped(app, cpu, at, word(app, keptLeft), word(app, keptTop), word(app, keptWidth),
                       word(app, keptHeight));
    };
    const int32_t textHeight = 0xe;
    bool labelsShown = false;
    int32_t under = -1;
    const bool sliding = kScreensSliding(app) != 0 && kOverRace(app) == 0;
    greyed(app, item);
    reg32 colour;
    if ((slider(app, item).flags & 1) != 0)
        colour = 0x908080;
    else if (hot != 0)
    {
        under = partUnder(app, item);
        colour = 0xffe440;
    }
    else
        colour = 0xff649dfd;
    reg32 r;
    const auto drawPart = [&](int32_t part) -> bool {
        const int32_t kind = slider(app, item).kind;
        const SliderPart p = slider(app, item).parts[part];
        if (half(app, shapeAt(kind, part, 1)) == -1 && kOverRace(app) == 0)
        {
            // No art: a rectangle in its kind's colour, the hot arrow or thumb framed.
            const reg32 fill = word(app, kPartColours + 0x30 * reg32(kind) + 8 * reg32(part)) | 0xff000000;
            if (!callGame(app, cpu, at, kFilled, {reg32(int32_t(p.x)), reg32(int32_t(p.y)), reg32(int32_t(p.width)),
                                                  reg32(int32_t(p.height))},
                          {fill}, r))
                return false;
            if (hot == 0 || part != under || (part != 0 && part != 2 && part != 3))
                return true;
            return callGame(app, cpu, at, kFrame, {reg32(int32_t(p.x)), reg32(int32_t(p.y)), reg32(int32_t(p.width)),
                                                   reg32(int32_t(p.height))},
                            {0xffffe440}, r);
        }
        const int32_t x = p.x, y = p.y, w = p.width, h = p.height;
        const reg32 alpha = slider(app, item).alphaOn != 0 && (kRenderFlags(app) & 2) == 0
                                ? reg32(slider(app, item).alpha) << 24 | 0xffffff
                                : 0xffffffff;
        if (part > 3)
            return true;
        const reg32 places = kPartQuads + 0xc0 * reg32(kind) + 0x20 * reg32(part);
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
                if (!artQuad(app, cpu, at, x, y, middle, y, middle, foot, x, foot, places, alpha)
                    || !artQuad(app, cpu, at, middle + hw, y, middle, y, middle, foot, middle + hw, foot, places, alpha))
                    return false;
                reg32 light, dark;
                if (slider(app, item).alphaOn != 0 && (kRenderFlags(app) & 2) == 0)
                {
                    const reg32 a = reg32(slider(app, item).alpha) << 24;
                    dark = a | 0x15052d;
                    light = a | 0xff40;
                }
                else
                {
                    light = 0xff00ff40;
                    dark = 0xff15052d;
                }
                if (!viewportKept(app, cpu, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const SliderPart& t1 = slider(app, item).parts[1];
                const reg32 W = reg32(kScreenWidth(app)), H = reg32(kScreenHeight(app));
                const reg32 clipRight = onScreen(cpu, (reg32(t0.x + halved(t0.width)) - reg32(x)) * W, kPerWidth(app));
                const reg32 clipBottom = onScreen(cpu, reg32(t1.height + 4) * H, kPerHeight(app));
                const reg32 clipTop = onScreen(cpu, reg32(y - 2) * H, kPerHeight(app));
                const reg32 clipLeft = onScreen(cpu, W * reg32(x), kPerWidth(app));
                if (!clipped(app, cpu, at, clipLeft, clipTop, clipRight, clipBottom)
                    || !colourQuad(app, cpu, at, left, top, inner, top, right, bottom, track.x + 1, bottom, dark,
                                   light, light, dark))
                    return false;
                (void)thumb;
                return restore();
            }
            if ((kRenderFlags(app) & 2) != 0)
            {
                if (half(app, shapeAt(kind, 1, 1)) == -1)
                    return true;
                if (!callGame(app, cpu, at, kDrawArt, {reg32(int32_t(half(app, shapeAt(kind, 1, 1)))), reg32(x),
                                                       reg32(y), alpha},
                              {}, r)
                    || !viewportKept(app, cpu, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const reg32 W = reg32(kScreenWidth(app)), H = reg32(kScreenHeight(app));
                const reg32 clipRight = onScreen(cpu, (reg32(t0.x + halved(t0.width)) - reg32(x)) * W, kPerWidth(app));
                const reg32 clipBottom =
                    onScreen(cpu, reg32(slider(app, item).parts[1].height + 4) * H, kPerHeight(app));
                const reg32 clipTop = onScreen(cpu, reg32(y - 2) * H, kPerHeight(app));
                const reg32 clipLeft = onScreen(cpu, W * reg32(x), kPerWidth(app));
                return clipped(app, cpu, at, clipLeft, clipTop, clipRight, clipBottom)
                       && colourQuad(app, cpu, at, left, top, inner, top, right, bottom, track.x + 1, bottom,
                                     0xff15052d, 0xff00ff40, 0xff00ff40, 0xff15052d)
                       && restore();
            }
            if (half(app, shapeAt(kind, 1, 0)) != -1)
            {
                if (!viewportKept(app, cpu, at, keptLeft, keptTop, keptWidth, keptHeight))
                    return false;
                const SliderPart& t0 = slider(app, item).parts[0];
                const reg32 W = reg32(kScreenWidth(app)), H = reg32(kScreenHeight(app));
                const reg32 clipBottom =
                    onScreen(cpu, reg32(slider(app, item).parts[part].height + 4) * H, kPerHeight(app));
                const reg32 clipRight = onScreen(cpu, (reg32(t0.x + halved(t0.width)) - reg32(x)) * W, kPerWidth(app));
                const reg32 clipTop = onScreen(cpu, reg32(y - 2) * H, kPerHeight(app));
                const reg32 clipLeft = onScreen(cpu, W * reg32(x), kPerWidth(app));
                if (!clipped(app, cpu, at, clipLeft, clipTop, clipRight, clipBottom)
                    || !callGame(app, cpu, at, kDrawArt, {reg32(int32_t(half(app, shapeAt(kind, 1, 0)))), reg32(x),
                                                          reg32(y), alpha},
                                 {}, r)
                    || !restore())
                    return false;
            }
            if (half(app, shapeAt(kind, part, 1)) == -1)
                return true;
            if (!viewportKept(app, cpu, at, keptLeft, keptTop, keptWidth, keptHeight))
                return false;
            const SliderPart& t0 = slider(app, item).parts[0];
            const SliderPart& t1 = slider(app, item).parts[1];
            const int32_t thumbMiddle = t0.x + halved(t0.width);
            const reg32 W = reg32(kScreenWidth(app)), H = reg32(kScreenHeight(app));
            const reg32 clipBottom =
                onScreen(cpu, reg32(slider(app, item).parts[part].height + 4) * H, kPerHeight(app));
            const reg32 clipRight = onScreen(cpu, W * reg32(t1.x + t1.width - thumbMiddle - 1), kPerWidth(app));
            const reg32 clipTop = onScreen(cpu, reg32(y - 2) * H, kPerHeight(app));
            const reg32 clipLeft = onScreen(cpu, reg32(thumbMiddle) * W, kPerWidth(app));
            if (!clipped(app, cpu, at, clipLeft, clipTop, clipRight, clipBottom)
                || !callGame(app, cpu, at, kDrawArt, {reg32(int32_t(half(app, shapeAt(kind, part, 1)))), reg32(x),
                                                      reg32(y), alpha},
                             {}, r))
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
                return artQuad(app, cpu, at, x0, y, x1, y, x2, y + h, x3, y + h,
                               kHotPartQuads + 0xc0 * reg32(kind) + 0x20 * reg32(part), alpha);
            if (half(app, shapeAt(kind, part, 0)) == -1)
                return true;
            return callGame(app, cpu, at, kDrawArt, {reg32(int32_t(half(app, shapeAt(kind, part, 0)))), reg32(x),
                                                     reg32(y), alpha},
                            {}, r);
        }
        if (kOverRace(app) != 0)
            return artQuad(app, cpu, at, x0, y, x1, y, x2, y + h, x3, y + h, places, alpha);
        if (half(app, shapeAt(kind, part, 1)) == -1)
            return true;
        return callGame(app, cpu, at, kDrawArt, {reg32(int32_t(half(app, shapeAt(kind, part, 1)))), reg32(x),
                                                 reg32(y), alpha},
                        {}, r);
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
        if (!callGame(app, cpu, at, kSelectFont, {0x12}, {}, r))
            return {};
        labelsShown = true;
        const int32_t y = slider(app, item).labelsBelow == 1 ? boxY + boxHeight : boxY - textHeight;
        const Slider& t = slider(app, item);
        if (t.labels[0] != 0
            && !callGame(app, cpu, at, kDrawMenuText, {t.labels[0], reg32(int32_t(t.parts[2].x)), reg32(y), 0x12},
                         {0, colour}, r))
            return {};
        if (slider(app, item).labels[1] != 0
            && !callGame(app, cpu, at, kDrawMenuText, {slider(app, item).labels[1], reg32(middleX), reg32(y), 0x12},
                         {2, colour}, r))
            return {};
        const Slider& u = slider(app, item);
        if (u.labels[2] != 0
            && !callGame(app, cpu, at, kDrawMenuText,
                         {u.labels[2], reg32(int32_t(u.parts[3].x) + u.parts[3].width), reg32(y), 0x12}, {1, colour},
                         r))
            return {};
    }
    if (slider(app, item).title != 0)
    {
        if (!callGame(app, cpu, at, kSelectFont, {0x12}, {}, r))
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
            y = int32_t(word(app, kRowsY + 4 * reg32(row)));
            x = int32_t(word(app, kRowsX + 4 * reg32(row)) + 0xb);
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
                    const int32_t by = int32_t(reg32(int32_t(t.y)) - word(app, kRowsY) + reg32(kScreensSliding(app)) - 0x12c);
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
            if (!callGame(app, cpu, at, kDrawMenuText, {slider(app, item).title, reg32(at_), reg32(y), 0x12},
                          {reg32(align), colour}, r))
                return {};
        }
    }
    if (!sliding && slider(app, item).percent == 1)
    {
        const int32_t y = middleY - halved(textHeight);
        const std::optional<Float> v = snappedAt(app, cpu, at, kHundredth, slider(app, item).values[0]);
        if (!v)
            return {};
        Float p = fp::sub(Float(kPercentFrom(app)), *v);
        p = -p;
        p = fp::mul(p, Float(kPercentScale(app)));
        p = fp::mul(p, Float(kPercentScale2(app)));
        const float scaled = float(p);
        cpu.fpu.compare(Float(0.0), Float(scaled));
        const float rounded =
            float(fp::add(Float(scaled), Float(cpu.fpu.status.c0 || cpu.fpu.status.c3 ? kRoundUp(app) : kRoundDown(app))));
        const int32_t n = chopped<int32_t>(cpu, Float(rounded));
        if (!format(app, cpu, at, {text, kPercent, reg32(n), 0x25})
            || !callGame(app, cpu, at, kDrawMenuText, {text, reg32(boxX + boxWidth + 0xa), reg32(y), 0x12},
                         {0, colour}, r))
            return {};
    }
    return 0;
}

/* 0x46b3a0's: a two-axis slider drawn: its parts from the last -- its thumbs
 * across and up and its bar's thumb as quads of the art on the 3D card or
 * shapes on a 2D screen, the part under the pointer lit where `hot` asks for
 * it; its area a frame of shaded quads, its art clipped at each corner on a
 * 2D screen --, its title by its alignment, its values as percentages. */
Result twoAxisDrawn(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot)
{
    const reg32 ebp = esp - 0x14, at = ebp - 0xa0, text = ebp - 0xa0;
    HeldEbp held(cpu, ebp);
    for (reg32 i = 0; i < 0x40; i += 4)
        setWord(app, text + i, word(app, kTwoAxisBlank + i));
    // The frame's words the code keeps a part's place and size in, which its
    // percentages are placed by when it has no title (as the last part left
    // them); the viewport kept.
    const reg32 placeX = ebp - 0x20, placeY = ebp - 0x1c, placeWidth = ebp - 0x30;
    const reg32 keptLeft = ebp - 0x50, keptTop = ebp - 0x5c, keptWidth = ebp - 0x60, keptHeight = ebp - 0x4c;
    const int32_t textHeight = 0xe;
    greyed(app, item);
    int32_t under = -1;
    reg32 colour = 0xff649dfd;
    if (hot != 0)
    {
        under = partUnder(app, item);
        colour = 0xffe440;
    }
    reg32 r;
    const auto quadsOf = [&](int32_t part, bool lit) {
        return (lit ? kHotPartQuads : kPartQuads) + 0xc0 * reg32(int32_t(slider(app, item).kind)) + 0x20 * reg32(part);
    };
    const auto shapeDrawn = [&](int32_t shape) {
        return callGame(app, cpu, at, kDrawShape,
                        {reg32(shape), word(app, placeX), word(app, placeY), 0}, {}, r);
    };
    const reg32 W = reg32(kScreenWidth(app)), H = reg32(kScreenHeight(app));
    const auto drawPart = [&](int32_t part) -> bool {
        const SliderPart p = slider(app, item).parts[part];
        setWord(app, placeX, reg32(int32_t(p.x)));
        setWord(app, placeY, reg32(int32_t(p.y)));
        setWord(app, placeWidth, reg32(int32_t(p.width)));
        const int32_t x = p.x, y = p.y, w = p.width, h = p.height;
        const int32_t kind = slider(app, item).kind;
        if (reg32(part) > 5)
            return true;
        if (part < 4)
        {
            // A thumb across (0, 1) or up (2, 3; mirrored).
            const bool lit = part == under;
            if (kOverRace(app) != 0)
            {
                if (part < 2)
                    return artQuad(app, cpu, at, x, y, x + w, y, x + w, y + h, x, y + h, quadsOf(part, lit), 0xffffffff);
                return artQuad(app, cpu, at, x + w, y, x, y, x, y + h, x + w, y + h, quadsOf(part, lit), 0xffffffff);
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
                return artQuad(app, cpu, at, x, y, x + w, y, x + w, y + h, x, y + h, quadsOf(4, false), 0xffffffff);
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
            if (!artQuad(app, cpu, at, x, y, middle, y, middle, y + h, x, y + h, quadsOf(5, false), 0xffffffff)
                || !artQuad(app, cpu, at, middle + hw, y, middle, y, middle, y + h, middle + hw, y + h,
                            quadsOf(5, false), 0xffffffff))
                return false;
            const reg32 a = 0x98900010, b = 0x98186a02;
            const int32_t top = slider(app, item).parts[5].y + 4;
            const int32_t foot = slider(app, item).parts[5].y + slider(app, item).parts[5].height - 4;
            const Slider& t = slider(app, item);
            const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width - 3;
            const int32_t e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
            const int32_t areaLeft = t.parts[5].x + 0x10, areaRight = t.parts[5].x + t.parts[5].width - 0x10;
            return colourQuad(app, cpu, at, areaLeft, top, e0, top, e0, foot, areaLeft, foot, a, a, a, a)
                   && colourQuad(app, cpu, at, e0, top, e1, top, e1, foot, e0, foot, b, b, b, b)
                   && colourQuad(app, cpu, at, e1, top, e3, top, e3, foot, e1, foot, a, a, a, a)
                   && colourQuad(app, cpu, at, e3, top, e2, top, e2, foot, e3, foot, b, b, b, b)
                   && colourQuad(app, cpu, at, e2, top, areaRight, top, areaRight, foot, e2, foot, a, a, a, a);
        }
        if ((kRenderFlags(app) & 2) != 0)
        {
            const int16_t shape = half(app, shapeAt(kind, 5, 1));
            if (shape != -1 && !shapeDrawn(shape))
                return false;
            const reg32 a = 0xff900010, b = 0xff186a02;
            const Slider& t = slider(app, item);
            const int32_t top = t.parts[5].y + 3, foot = t.parts[5].y + t.parts[5].height - 4;
            const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width - 3;
            const int32_t e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
            const int32_t areaLeft = t.parts[5].x + 0x10, areaRight = t.parts[5].x + t.parts[5].width - 0x11;
            return colourQuad(app, cpu, at, areaLeft, top, e0, top, e0, foot, areaLeft, foot, a, a, a, a)
                   && colourQuad(app, cpu, at, e0, top, e1, top, e1, foot, e0, foot, b, b, b, b)
                   && colourQuad(app, cpu, at, e1, top, e3, top, e3, foot, e1, foot, a, a, a, a)
                   && colourQuad(app, cpu, at, e3, top, e2, top, e2, foot, e3, foot, b, b, b, b)
                   && colourQuad(app, cpu, at, e2, top, areaRight, top, areaRight, foot, e2, foot, a, a, a, a);
        }
        const int16_t shapeB = half(app, shapeAt(kind, 5, 1)), shapeA = half(app, shapeAt(kind, 5, 0));
        if (shapeB == -1 || shapeA == -1)
            return true;
        if (!viewportKept(app, cpu, at, keptLeft, keptTop, keptWidth, keptHeight))
            return false;
        // The art clipped to each stretch of the area's top between the thumbs.
        const int32_t tall = h + 4, top = y - 2;
        (void)area;
        const auto stretch = [&](int32_t from, int32_t width, int16_t shape) {
            return clipped(app, cpu, at, onScreen(cpu, reg32(from) * W, kPerWidth2(app)),
                           onScreen(cpu, H * reg32(top), kPerHeight2(app)),
                           onScreen(cpu, reg32(width) * W, kPerWidth2(app)),
                           onScreen(cpu, reg32(tall) * H, kPerHeight2(app)))
                   && shapeDrawn(shape);
        };
        const Slider& t = slider(app, item);
        const int32_t e0 = t.parts[0].x + t.parts[0].width - 3, e1 = t.parts[1].x + t.parts[1].width + 3;
        const int32_t e1l = t.parts[1].x + t.parts[1].width - 3, e3 = t.parts[3].x + 3, e2 = t.parts[2].x + 3;
        return stretch(x, e0 - x, shapeA) && stretch(e0, e1 - e0, shapeB) && stretch(e1l, e3 - e1l, shapeA)
               && stretch(e3, e2 - e3, shapeB) && stretch(e2, x + w - e2, shapeA)
               && clipped(app, cpu, at, word(app, keptLeft), word(app, keptTop), word(app, keptWidth),
                          word(app, keptHeight));
    };
    for (int32_t part = partsOf(app, slider(app, item).kind) - 1; part >= 0; --part)
        if (!drawPart(part))
            return {};
    // Its title.
    if (slider(app, item).title != 0)
    {
        const Slider& s = slider(app, item);
        setWord(app, placeX, reg32(int32_t(s.boxX)));
        setWord(app, placeY, reg32(int32_t(s.boxY)));
        setWord(app, placeWidth, reg32(int32_t(s.boxWidth)));
        const int32_t boxX = s.boxX, boxY = s.boxY, boxWidth = s.boxWidth, boxHeight = s.boxHeight;
        const int32_t middleX = boxX + halved(boxWidth), middleY = boxY + halved(boxHeight);
        if (!callGame(app, cpu, at, kSelectFont, {0x12}, {}, r))
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
        if (!callGame(app, cpu, at, kDrawMenuText, {slider(app, item).title, reg32(x), reg32(y), 0x12},
                      {reg32(align), colour}, r))
            return {};
    }
    // Its values, as percentages.
    float percents[2];
    for (int axis = 0; axis < 2; ++axis)
    {
        const std::optional<Float> v = snappedAt(app, cpu, at, kHundredth, slider(app, item).values[axis]);
        if (!v)
            return {};
        const float scaled = float(fp::mul(*v, Float(kTwoAxisScale(app))));
        cpu.fpu.compare(Float(0.0), Float(scaled));
        percents[axis] = float(fp::add(
            Float(scaled), Float(cpu.fpu.status.c0 || cpu.fpu.status.c3 ? kTwoAxisUp(app) : kTwoAxisDown(app))));
    }
    const int32_t across = chopped<int32_t>(cpu, Float(percents[0]));
    const int32_t up = chopped<int32_t>(cpu, Float(percents[1]));
    if (!format(app, cpu, at, {text, kTwoPercents, reg32(up), 0x25, reg32(across), 0x25})
        || !callGame(app, cpu, at, kDrawMenuText,
                     {text, word(app, placeX) + word(app, placeWidth) + 0xa, word(app, placeY), 0x12}, {0, colour}, r))
        return {};
    return 0;
}

}  // namespace

/* ---- The adapters ---- */

/* 0x4692c0: the part of the slider eax under the pointer, or -1. */
void sliderPartUnderPointer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(partUnder(app, e.eax)));
}

/* 0x469340 (stdcall 8): where the slider eax's thumb may go: the start to
 * [ebx], the end to [ecx], the length to the stack's first pointer, the place
 * to its second. */
void sliderThumbPlace(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(thumbPlaced(app, cpu, e.eax, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1))), 8);
}

/* 0x4693c0 (stdcall 8): the stack's value snapped to the stack's step, onto
 * the x87's stack. */
void snapToStep(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const std::optional<Float> r = snapped(app, cpu, e.esp);
    if (!r)
        return;
    cpu.fpu.push(*r);
    e.back(cpu, e.eax, 8);
}

/* 0x469480 (stdcall 8): the slider eax set to the stack's value. */
void setSliderValue(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, valueSet(app, cpu, e.esp, e.eax), 8);
}

/* 0x469530, 0x46afe0 (stdcall 8): the slider eax's arrows' step, of axis
 * [esp+8] for a two-axis one. */
void setSliderStep(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, arrowStepSet(app, cpu, e.eax, e.arg(app, 0), 0, e.eax), 8);
}

void setTwoAxisStep(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, arrowStepSet(app, cpu, e.eax, e.arg(app, 0), int32_t(e.arg(app, 1)), e.eax), 8);
}

/* 0x469590: the slider eax dragged to dx along its track. */
void dragSliderTo(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, draggedTo(app, cpu, e.esp, e.eax, int16_t(e.edx)));
}

/* 0x469680, 0x46b3a0: the slider, the two-axis slider eax drawn; the part
 * under the pointer lit where edx. */
void drawSlider(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sliderDrawn(app, cpu, e.esp, e.eax, e.edx));
}

void drawTwoAxis(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisDrawn(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x46a5f0, 0x46c320: the slider, the two-axis slider eax as the mouse works
 * it. */
void trackSlider(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sliderTracked(app, cpu, e.esp, e.eax));
}

void trackTwoAxis(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisTracked(app, cpu, e.esp, e.eax));
}

/* 0x46a860, 0x46c4a0: key bx on the slider, the two-axis slider eax. */
void sliderKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sliderKeyed(app, cpu, e.esp, e.eax, e.ebx));
}

void twoAxisKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisKeyed(app, cpu, e.esp, e.eax, e.ebx));
}

/* 0x46a980, 0x46c640: the slider, the two-axis slider eax's parts placed. */
void placeSliderParts(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, sliderPlaced(app, e.eax));
}

void placeTwoAxisParts(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, twoAxisPlacedParts(app, e.eax));
}

/* 0x46ab10, 0x46c820: the slider, the two-axis slider eax set up. */
void setUpSlider(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sliderSetUp(app, cpu, e.esp, e.eax));
}

void setUpTwoAxis(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisSetUp(app, cpu, e.esp, e.eax));
}

/* 0x46abb0, 0x46c8b0: the box the pointer finds the slider, the two-axis
 * slider eax in. */
void sliderBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, sliderBoxed(app, e.eax));
}

void twoAxisBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, twoAxisBoxed(app, e.eax));
}

/* 0x46ac40 (stdcall 0xc): where thumb ebx of the two-axis slider eax may go
 * along axis edx: the start to [ecx], the end, the length and the place to
 * the stack's pointers. */
void twoAxisThumbPlace(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu,
           reg32(twoAxisPlaced(app, cpu, e.eax, int32_t(e.edx), e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1),
                               e.arg(app, 2))),
           0xc);
}

/* 0x46add0 (stdcall 8): axis [esp+8] of the two-axis slider eax set to the
 * stack's value. */
void setTwoAxisValue(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisValueSet(app, cpu, e.esp, e.eax, e.eax), 8);
}

/* 0x46b040: axis ebx of the two-axis slider eax dragged to dx. */
void dragTwoAxisTo(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, twoAxisDragged(app, cpu, e.esp, e.eax, int16_t(e.edx), int32_t(e.ebx)));
}

/* 0x469080: the slider eax's parts' art and sizes. */
void sliderPartSizes(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, partSizes(app, cpu, e.esp, e.eax));
}

/* 0x4d5e40: the menus' shape named eax, found or made. */
void sliderShape(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, shapeOf(app, cpu, e.esp, e.eax, e.ebx));
}

/* 0x46c950 (stdcall 8): the slider eax set to the stack's value -- a
 * two-axis one's axis the stack's second word; none of kind 1. */
void sliderSetValue(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 8;
    HeldEbp held(cpu, at);
    const uint16_t kind = uint16_t(slider(app, item).kind);
    Result r;
    if (kind == 1)
        r = int32_t(item);
    else if (kind == 2)
        r = twoAxisValueSet(app, cpu, argsBelow(app, at, {e.arg(app, 0), e.arg(app, 1)}), item, item);
    else
        r = valueSet(app, cpu, argsBelow(app, at, {e.arg(app, 0), 0}), item);
    e.finish(cpu, r, 8);
}

/* 0x46cae0: the slider eax drawn, worked by the mouse first where edx: kind 2
 * a two-axis one, kind 1 none.  Greyed as the menus grey their controls. */
void sliderDraw(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 0x10;
    HeldEbp held(cpu, at);
    greyed(app, item);
    const uint16_t kind = uint16_t(slider(app, item).kind);
    Result r = 0;
    if (kind == 2)
    {
        if (e.edx != 0 && !twoAxisTracked(app, cpu, at - 4, item))
            return;
        r = twoAxisDrawn(app, cpu, at - 4, item, e.edx);
    }
    else if (kind != 1)
    {
        if (e.edx != 0 && !sliderTracked(app, cpu, at - 4, item))
            return;
        r = sliderDrawn(app, cpu, at - 4, item, e.edx);
    }
    e.finish(cpu, r);
}

/* 0x46cb80: key cx on the slider eax. */
void sliderWidgetKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 0xc;
    const uint16_t kind = uint16_t(slider(app, item).kind);
    const reg32 key = reg32(int32_t(int16_t(e.ecx)));
    Result r = 0;
    if (kind == 2)
        r = twoAxisKeyed(app, cpu, at - 4, item, key);
    else if (kind != 1)
        r = sliderKeyed(app, cpu, at - 4, item, key);
    e.finish(cpu, r);
}

/* 0x46cbe0: the slider eax laid out. */
void sliderWidgetLayout(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const uint16_t kind = uint16_t(slider(app, e.eax).kind);
    e.back(cpu, kind == 2 ? twoAxisBoxed(app, e.eax) : kind == 1 ? e.eax : sliderBoxed(app, e.eax));
}

/* 0x46cc10: the slider eax hidden: its screen put back. */
void sliderWidgetHidden(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Slider& s = slider(app, e.eax);
    const uint16_t kind = uint16_t(s.kind);
    reg32 eax = (e.eax & 0xffff0000) | kind;
    if (kind != 1 && kind != 2)
    {
        s.screen = s.shownScreen;
        eax = (e.eax & 0xffff0000) | uint16_t(s.shownScreen);
    }
    e.back(cpu, eax);
}

/* 0x46cc50: the slider eax shown: its parts' art and sizes, set up (a kind
 * past 5 made a slider's), its value set. */
void sliderWidgetShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 item = e.eax, at = e.esp - 8;
    HeldEbp held(cpu, at);
    const uint16_t kind = uint16_t(slider(app, item).kind);
    if (kind > 5)
        slider(app, item).kind = 0;
    if (kind == 2)
    {
        if (!partSizes(app, cpu, at - 4, item) || !twoAxisSetUp(app, cpu, at - 4, item))
            return;
    }
    else if (kind != 1)
    {
        if (!partSizes(app, cpu, at - 4, item) || !sliderSetUp(app, cpu, at - 4, item))
            return;
    }
    const reg32 value = slider(app, item).values[0];
    const reg32 callee = argsBelow(app, at, {value, 0});
    const uint16_t now = uint16_t(slider(app, item).kind);
    Result r;
    if (now == 1)
        r = int32_t(item);
    else if (now == 2)
        r = twoAxisValueSet(app, cpu, argsBelow(app, callee - 8, {value, 0}), item, item);
    else
        r = valueSet(app, cpu, argsBelow(app, callee - 8, {value, 0}), item);
    e.finish(cpu, r);
}

}  // namespace fields
}  // namespace port
}  // namespace nfs3hp
