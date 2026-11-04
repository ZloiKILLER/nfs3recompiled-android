#include "port_text.h"
#include "port_draw2d.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>

/* The menus' text and the 2D drawing under it, rewritten (port.h says how).
 *
 * A text of the 3D card's font is drawn a character at a time, each a quad of
 * the font's texture queued on its font's list (0x7f74c0, 0x81b4d0: the
 * texture's state, the next free vertex, the quads' vertex numbers four by
 * four) and drawn with the others when the fonts are flushed; the 2D
 * screens' font is the game's code, called by address.  Their sizes are a
 * 640 by 480 screen's, the menus', scaled to the screen's own.  A long text
 * is wrapped into a box a word at a time.  The menus' key comes from the
 * mouse, the keyboard or (in the PC game) a joystick, held keys repeating.
 *
 * The x87's arithmetic is the game's to the bit: each sum, product and
 * quotient as the x87 rounds it under the control word the game has
 * (cpu.fpu's), each value stored to a float where the game's code stores it,
 * the integers truncated as __CHP and fistp make them; a width that the
 * game's code leaves on the x87's stack is left there, and the compares the
 * game's code makes leave the status word as they do.  What these call of the
 * game's that is not rewritten -- the 2D screens' text, the render state, the
 * quads' clipping and indices, memmove, the mouse, the keyboard, the files --
 * they call by address, from where the game's code has its stack; the menu's
 * click sound (port::playMenuEffect) as the C++ it is.  A function of this
 * file calls another of it as C++.
 *
 * The game's callers of these read no register but eax after them (by
 * liveness on nfs3.exe), so each adapter gives back every other register as
 * the caller had it, though the game's code leaves edx and ebx changed in
 * drawText, drawCharacter, takeQuad, queueQuad, unqueueQuad, drawTextAt,
 * drawMenuText, nextWord and the wrapped text's.  tools/game_checks holds
 * each adapter against the game's code for it (text.inc). */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{
/* nfs3hp_main.cpp's HUD editor measuring each quad queued (at sub_49bd30's
 * start: tools/apply_hud_editor.py). */
void hudQueueQuad(win32::WinApplication* app, x86::reg32 list, x86::reg32 a, x86::reg32 b, x86::reg32 c,
                  x86::reg32 d);

namespace port
{
/* port_sound_effects.cpp's menu effect (0x4181d0), declared by its type:
 * tools/game_checks/generate.py would take `void playMenuEffect(` for its
 * definition. */
using Adapter = void(App* app, CPU& cpu);
Adapter playMenuEffect;

namespace text
{

namespace
{

using Float = x86::Float;

// The menus: in a race's pause screens the 3D card's font draws them.
constexpr Var<int32_t> kInRace{0x559234};
// The 2D screens' text: its font, its colour, its size (1) and line (14).
constexpr Var<reg32> kScreenFont{0x664650}, kScreenTextColour{0x559434};
constexpr Var<float> kScreenTextSize{0x559450}, kScreenLineHeight{0x5395ec};
constexpr Var<reg32> kScreenTextShade{0x563aa4};
// The 3D card's font: its colour, whether a text is only measured, which of
// the two fonts, whether the fonts are scaled to the screen, how big a
// character is drawn, the text scale (a float's bits) and the one kept.
constexpr Var<reg32> kTextColour{0x561b34};
constexpr Var<int32_t> kMeasuring{0x561b3c};
constexpr Var<int32_t> kFont{0x840184};
constexpr Var<int32_t> kScaledToScreen{0x840188};
constexpr Var<float> kCharacterSize{0x84018c};
constexpr Var<reg32> kCharacterSizeBits{0x84018c};
constexpr Var<reg32> kTextScale{0x840190}, kKeptTextScale{0x561b40};
constexpr Var<float> kMenuWidth{0x5395f0};     // 640
constexpr Var<float> kScreenScale{0x540bc8};   // 1/320: a character's size by the screen's width
constexpr Var<double> kMinusOne{0x540bc0};
// The screen: its width, its viewport (left, top, width, height).
constexpr Var<int32_t> kScreenWidth{0x7cdac8};
constexpr Var<int32_t> kViewportLeft{0x7cda70}, kViewportTop{0x7cda74};
constexpr Var<int32_t> kViewportWidth{0x7cda78}, kViewportHeight{0x7cda7c};
constexpr int32_t kMenuWidthPixels = 0x280, kMenuHeightPixels = 0x1e0;
// A character's metrics, by its number (0xe0 a font, from the space).
constexpr int32_t kCharacters = 0xe0;
constexpr Table<int8_t> kAdvance{0x83f4e0};     // how far the next one goes
constexpr Table<uint8_t> kTextureY{0x83f700}, kTextureX{0x83f8c0};
constexpr Table<uint8_t> kGlyphWidth{0x83fc40};
constexpr Table<uint8_t> kGlyphHeight{0x83fe00};  // less one
constexpr Table<int8_t> kOffsetX{0x83ffc0}, kOffsetY{0x83fa80};
// A font's texture, 44 bytes a font: its width and height first.
struct FontTexture
{
    uint16_t width, height;
};
constexpr Table<FontTexture> kFontTextures{0x83f6a8, 44};
// The fonts' lists of quads, 0x24010 bytes a font: the texture's state, the
// next free vertex (counting down), how many vertex numbers are queued, the
// vertices (0x20 bytes each: x, y, colour at 0x10, u and v at 0x18), the
// numbers.
constexpr reg32 kFontQuads = 0x7f74c0, kFontQuadsBytes = 0x24010;
constexpr reg32 kListState = 0, kListNextVertex = 4, kListQueued = 0xc, kListVertices = 0x10;
constexpr reg32 kListNumbers = 0x20010;
constexpr reg32 kVertexX = 0, kVertexY = 4, kVertexColour = 0x10, kVertexU = 0x18, kVertexV = 0x1c;
// The language's text: offsets from its block's start, then the lines; the
// lines past its own are names.
constexpr Var<reg32> kLanguageTexts{0x563a80};
constexpr Table<reg32> kLanguageFiles{0x563a68};  // text.eng, text.ger ...
constexpr int32_t kLanguageLines = 0x7a7;
constexpr reg32 kNoText = 0x549430;               // ""
constexpr reg32 kTwoStrings = 0x549428, kGameFolder = 0x7a2eb4;  // "%s%s", the game's folder
constexpr reg32 kPoliceCarText = 0x1e3, kCarText = 0x1e4;
// The renderer: its state bits (2 alpha), the 2D screens' detail shades.
constexpr Var<uint8_t> kRenderFlags{0x7a3a58};
constexpr Var<reg32> kShadeFull{0x791b50}, kShadeMost{0x791b54}, kShadeLess{0x791b4c};
constexpr Var<double> kSixTenths{0x5395e0};
constexpr Var<reg32> kTabText{0x5395e8};          // "   ": a tab, as the box shows it
// The menus' key: the clock; the mouse's button word, its click, its wheel,
// whether it is there; the key now, its scan code, the last one, whether it
// came from the mouse; the key held, when it repeats first and next; when the
// joystick is looked at next.
constexpr Var<int32_t> kClock{0x79c1a4};
constexpr reg32 kMouseButtons = 0x55e5f4, kMouseX = 0x55e5e4, kMouseY = 0x55e5e8, kMouseWheel = 0x55e5f8;
constexpr Var<reg32> kMouseClick{0x55e5ec};
constexpr Var<uint8_t> kMouseClickButtons{0x55e5ed};
constexpr Var<int32_t> kMouseWheelTurn{0x55e5f8};
constexpr Var<int32_t> kMouseOn{0x55e600};
constexpr Var<reg32> kKey{0x664654}, kKeyScanCode{0x66465c}, kLastKey{0x66464c}, kKeyFromMouse{0x664658};
constexpr Var<reg32> kHeldKey{0x664648};
constexpr Var<int32_t> kRepeatFirst{0x664644}, kRepeatNext{0x559440}, kJoystickNext{0x55943c};
constexpr Var<reg32> kRepeatNone{0x664650};
constexpr reg32 kEnter = 0xd, kEscape = 0x1b;
// The 2D drawing's (port_draw2d.h), called as the game's code calls them.
constexpr guest::Native kDrawScreenText = draw2d::drawScreenText;  // 0x4da4a0: a 2D screen's text (eax, x edx, y ebx; ecx 1 only measured): its width
constexpr guest::Native kScreenTextLine = draw2d::drawTurnedText;  // 0x4da120: a line of it at a size (stdcall: size, 0, colour)
// What is not rewritten: called by address.
constexpr reg32 kRenderState = 0x431900;     // render state eax to edx
constexpr reg32 kQuadOutcodes = 0x4d5840, kQuadMesh = 0x4c1e20;
constexpr reg32 kMemmove = 0x4ea4f0;
constexpr reg32 kReadMouse = 0x435310;
constexpr reg32 kKeyDown = 0x4eb1c0, kScreenCapture = 0x452410;
constexpr reg32 kNextKey = 0x4cc970, kKeyFeedback = 0x449a60, kResetAttractTimer = 0x4961e0;
constexpr reg32 kGetKeyState = 0x534738;      // GetKeyState's import
constexpr reg32 kDisplayModeName = 0x448400, kControllerName = 0x43d9f0;
constexpr reg32 kCarKnown = 0x439cc0, kCarIsPolice = 0x4396b0, kCarName = 0x439250;
constexpr reg32 kFree = 0x4e1890, kSprintf = 0x4df690, kLoadFile = 0x4e0ec0;
constexpr int32_t kAltScanCode = 0x38, kPScanCode = 0x19;

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

float floatAt(App* app, reg32 at)
{
    return app->getMemory<float>(at);
}

void setFloat(App* app, reg32 at, float value)
{
    app->getMemory<float>(at) = value;
}

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The game's strings: how long, one appended, one copied (its own code's
 * strcat and strcpy, a byte after another to the 0). */
reg32 length(App* app, reg32 s)
{
    reg32 n = 0;
    while (byteAt(app, s + n) != 0)
        ++n;
    return n;
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

void appendString(App* app, reg32 to, reg32 from)
{
    copyString(app, to + length(app, to), from);
}

/* A call of the port's own adapter, as the game's code calls the address:
 * from `esp`, its eax. */
bool callPort(App* app, CPU& cpu, reg32 esp, guest::Native target, Args a, reg32& result)
{
    cpu.esp = esp;
    cpu.eax = a.eax;
    cpu.edx = a.edx;
    cpu.ebx = a.ebx;
    cpu.ecx = a.ecx;
    if (!guest::call(app, cpu, target))
        return false;
    result = cpu.eax;
    return true;
}

/* What leaves a value on the x87's stack: eax as the game's code leaves it,
 * and the value. */
struct Width
{
    reg32 eax;
    Float value;
};
using WidthResult = std::optional<Width>;

/* ---- The screen ---- */

struct Rectangle
{
    int32_t left, top, width, height;
};

/* 0x4beb80's: the screen's viewport. */
Rectangle screenViewport(App* app)
{
    return {kViewportLeft(app), kViewportTop(app), kViewportWidth(app), kViewportHeight(app)};
}

/* idiv: a 32-bit dividend, sign-extended, by `by`; the quotient's low half. */
int32_t quotient(int32_t dividend, int32_t by)
{
    return int32_t(int64_t(dividend) / int64_t(by));
}

/* 0x451400's, 0x451440's: x on the 640 wide screen of the menus, y on the 480
 * high one, on the screen's own (the product's low 32 bits). */
int32_t onScreenX(App* app, int32_t x)
{
    return quotient(int32_t(reg32(x) * reg32(screenViewport(app).width)), kMenuWidthPixels);
}

int32_t onScreenY(App* app, int32_t y)
{
    return quotient(int32_t(reg32(y) * reg32(screenViewport(app).height)), kMenuHeightPixels);
}

/* 0x451480's, 0x4514c0's: and the other way. */
int32_t onMenuX(App* app, int32_t x)
{
    return quotient(int32_t(reg32(x) * 0x280), screenViewport(app).width);
}

int32_t onMenuY(App* app, int32_t y)
{
    return quotient(int32_t(reg32(y) * 0x1e0), screenViewport(app).height);
}

/* ---- The language's text ---- */

Result lineText(App* app, CPU& cpu, reg32 esp, int32_t number, reg32 edx);

/* 0x4d17b0's: the text of a number past the language's lines: a display
 * mode's name (0x7a8 to 0x7bc), a car's (0x7bd to 0x7ee: its name, or "police
 * car" or "car" for one the game knows only so), a controller's (0x855 to
 * 0x865), else "". */
Result nameText(App* app, CPU& cpu, reg32 esp, int32_t number, reg32 edx)
{
    const reg32 at = esp - 8;
    reg32 r;
    if (number >= 0x7a8 && number <= 0x7bc)
    {
        if (!callGame(app, cpu, at, kDisplayModeName, {reg32(number), edx}, {}, r))
            return {};
        return int32_t(r);
    }
    if (number >= 0x7bd && number <= 0x7ee)
    {
        const reg32 car = reg32(number - 0x7bd);
        if (!callGame(app, cpu, at, kCarKnown, {car, car}, {}, r))
            return {};
        if (r != 0)
        {
            if (!callGame(app, cpu, at, kCarIsPolice, {car, car}, {}, r))
                return {};
            return lineText(app, cpu, at - 4, r != 0 ? kPoliceCarText : kCarText, car);
        }
        if (!callGame(app, cpu, at, kCarName, {car, car}, {}, r))
            return {};
        return int32_t(r);
    }
    if (!(number >= 0x7ef && number <= 0x821) && number >= 0x855 && number <= 0x865)
    {
        if (!callGame(app, cpu, at, kControllerName, {reg32(number - 0x855), edx}, {}, r))
            return {};
        return int32_t(r);
    }
    return int32_t(kNoText);
}

/* 0x4d1850's: line `number` of the language's text, or a name past them
 * (edx as the caller had it, which those that are names hand on). */
Result lineText(App* app, CPU& cpu, reg32 esp, int32_t number, reg32 edx)
{
    if (number >= kLanguageLines)
        return nameText(app, cpu, esp - 8 - 4, number, edx);
    const reg32 texts = kLanguageTexts(app);
    return int32_t(texts + word(app, reg32(number) * 4 + texts));
}

/* 0x4d1750's: the language's text loaded from its file (text.eng ...), the
 * one before freed -- for a language of 0 to 6; the block, or the language
 * past them. */
Result loadedLanguageText(App* app, CPU& cpu, reg32 esp, int32_t language)
{
    if (language > 6)
        return language;
    const reg32 old = kLanguageTexts(app);
    const reg32 at = esp - 0x10 - 0xfc;  // the file's name is built there
    reg32 r = reg32(language);
    if (old != 0 && !callGame(app, cpu, at, kFree, {old, reg32(language), 0, old}, {}, r))
        return {};
    const reg32 file = kLanguageFiles(app, language);
    if (!callGame(app, cpu, at, kSprintf, {r, reg32(language), file, old}, {at, kTwoStrings, kGameFolder, file}, r)
        || !callGame(app, cpu, at, kLoadFile, {at, 0, file, old}, {}, r))
        return {};
    kLanguageTexts.set(app, r);
    return int32_t(r);
}

/* ---- The fonts' quads ---- */

reg32 fontList(App* app)
{
    return kFontQuads + reg32(kFont(app)) * kFontQuadsBytes;
}

/* 0x49bcd0's: the next free vertex of list `list`, from its end: its number
 * and its address; the list one shorter. */
struct Taken
{
    reg32 number;
    reg32 vertex;
};

Taken nextQuad(App* app, reg32 list)
{
    const reg32 n = word(app, list + kListNextVertex);
    setWord(app, list + kListNextVertex, n - 1);
    return {n, list + kListVertices + (n << 5)};
}

/* 0x49bdc0's: the quad of vertices a, b, c, d taken off list `list`'s queue,
 * the first one queued so -- those after it moved down; the memmove's eax, or
 * where the search ended. */
Result unqueued(App* app, CPU& cpu, reg32 esp, reg32 list, reg32 a, reg32 b, reg32 c, reg32 d)
{
    int32_t i = 0;
    for (; i < int32_t(word(app, list + kListQueued)); i += 4)
    {
        const reg32 at = list + reg32(i) * 4 + kListNumbers;
        if (word(app, at) == a && word(app, at + 4) == b && word(app, at + 8) == c && word(app, at + 0xc) == d)
        {
            const reg32 bytes = (word(app, list + kListQueued) - reg32(i)) * 4 - 0x10;
            reg32 r;
            if (!callGame(app, cpu, esp - 0x14, kMemmove, {at + 0x10, at, bytes, c}, {}, r))
                return {};
            setWord(app, list + kListQueued, word(app, list + kListQueued) - 4);
            return int32_t(r);
        }
    }
    return i;
}

/* 0x49bd30's: the quad of vertices a, b, c, d queued on list `list` -- taken
 * off first where it may be there already (a vertex below the list's next
 * free one).  d. */
Result queued(App* app, CPU& cpu, reg32 esp, reg32 list, reg32 a, reg32 b, reg32 c, reg32 d)
{
    hudQueueQuad(app, list, a, b, c, d);
    if (int32_t(a) < int32_t(word(app, list + kListNextVertex))
        && !unqueued(app, cpu, esp - 0x14 - 4 - 4, list, a, b, c, d))
        return {};
    for (const reg32 n : {a, b, c, d})
    {
        const reg32 queued = word(app, list + kListQueued);
        setWord(app, list + kListQueued, queued + 1);
        setWord(app, list + queued * 4 + kListNumbers, n);
    }
    return int32_t(d);
}

/* 0x49be40's: list `list`'s quads drawn -- its texture's state set, their
 * outcodes, their indices -- and then those queued for a later frame than the
 * list's kept, the others taken out; the list's frame 0xfff.  The last call's
 * eax (the list, when nothing is queued). */
Result quadsDrawn(App* app, CPU& cpu, reg32 esp, reg32 list)
{
    if (word(app, list + kListQueued) == 0)
        return int32_t(list);
    const reg32 at = esp - 0x18 - 4;
    const reg32 vertices = list + kListVertices, numbers = list + kListNumbers;
    reg32 r;
    if (!callGame(app, cpu, at, kRenderState, {1, word(app, list + kListState)}, {}, r)
        || !callGame(app, cpu, at, kQuadOutcodes,
                     {reg32(int32_t(word(app, list + kListQueued)) >> 2), vertices, numbers, list}, {}, r)
        || !callGame(app, cpu, at, kQuadMesh, {reg32(int32_t(word(app, list + kListQueued)) >> 2), vertices, numbers, list},
                     {}, r))
        return {};
    reg32 eax = r;
    for (int32_t i = 0;;)
    {
        const reg32 queued = word(app, list + kListQueued);
        if (i >= int32_t(queued))
            break;
        eax = reg32(i) * 4;
        if (int32_t(word(app, list + kListNextVertex)) >= int32_t(word(app, list + eax + kListNumbers)))
        {
            i += 4;
            continue;
        }
        const reg32 from = numbers + eax;
        if (!callGame(app, cpu, at, kMemmove, {from + 0x10, from, (queued - reg32(i)) * 4 - 0x10, list}, {}, r))
            return {};
        eax = r;
        setWord(app, list + kListQueued, word(app, list + kListQueued) - 4);
    }
    setWord(app, list + kListNextVertex, 0xfff);
    return int32_t(eax);
}

/* 0x4d1340's: both fonts' quads drawn, render state 0x2bf on around them on
 * a card with alpha. */
Result fontsDrawn(App* app, CPU& cpu, reg32 esp)
{
    const reg32 at = esp - 8;
    const bool alpha = (kRenderFlags(app) & 2) != 0;
    reg32 r;
    if (alpha && !callGame(app, cpu, at, kRenderState, {0x2bf, 1}, {}, r))
        return {};
    Result drawn = quadsDrawn(app, cpu, at - 4, kFontQuads);
    if (drawn)
        drawn = quadsDrawn(app, cpu, at - 4, kFontQuads + kFontQuadsBytes);
    if (!drawn)
        return {};
    if ((kRenderFlags(app) & 2) != 0)
    {
        if (!callGame(app, cpu, at, kRenderState, {0x2bf, 0}, {}, r))
            return {};
        return int32_t(r);
    }
    return drawn;
}

/* ---- The 3D card's font ---- */

/* 0x4d1390's: the text scale set to `scale` (a float's bits), the one before
 * back.  Where the fonts are scaled to the screen, a character's size is the
 * screen's width over 320 -- the small font below a scale of 1 --, else the
 * scale itself. */
reg32 scaleSet(App* app, CPU& cpu, reg32 scale)
{
    const reg32 before = kTextScale(app);
    kTextScale.set(app, scale);
    if (kScaledToScreen(app) != 0)
    {
        kFont.set(app, int32_t(scale) < 0x3f800000 ? 1 : 0);
        kCharacterSize.set(app, float(fp::mul(Float(x86::sreg32(kScreenWidth(app))), Float(kScreenScale(app)))));
    }
    else
        kCharacterSizeBits.set(app, scale);
    return before;
}

/* 0x4d1540's, 0x4d1590's: character `c`'s height (less one) and its offset
 * down in the font, times a character's size, whole pixels toward 0. */
int32_t characterHeightOf(App* app, CPU& cpu, uint8_t c)
{
    const reg32 i = reg32(c) + (reg32(kFont(app)) << 5) * 7;
    const Float height = Float(x86::sreg16(byteAt(app, 0x83fde0 + i)));
    return chopped<int32_t>(cpu, fp::mul(height, Float(kCharacterSize(app))));
}

int32_t characterDropOf(App* app, CPU& cpu, uint8_t c)
{
    const reg32 i = reg32(c) + (reg32(kFont(app)) << 5) * 7;
    const Float drop = Float(x86::sreg16(int8_t(byteAt(app, 0x83fa60 + i))));
    return chopped<int32_t>(cpu, fp::mul(drop, Float(kCharacterSize(app))));
}

/* What 0x4d06f0 is handed: the character's number (0xe0 a font, from the
 * space), its square of the font's texture, where it goes (x a float), its
 * colour, whether it is only measured. */
struct Character
{
    reg32 number;
    int32_t textureX, textureY;
    float x;
    int32_t y;
    int32_t width, height;
    reg32 colour;
    int32_t measured;
};

/* 0x4d06f0's: a character of the fonts: its quad, unless it is a space, or
 * it is only measured, or it would be no pixel wide or high -- its corners
 * and their texture's, in its colour, queued on its font's list.  x, the
 * argument on the stack at esp + 4, moves to the quad's left edge.  How far
 * the next character goes. */
WidthResult drawnCharacter(App* app, CPU& cpu, reg32 esp, const Character& ch)
{
    auto& f = cpu.fpu;
    const int64_t number = x86::sreg32(ch.number);
    reg32 eax = reg32(int32_t(number / kCharacters));
    if (number % kCharacters != 0 && ch.measured == 0)
    {
        const FontTexture texture = kFontTextures(app, kFont(app));
        const Float perWidth = f.div(Float(1.0), Float(x86::sreg32(texture.width)));
        const Float width = Float(x86::sreg32(ch.width));
        const float widthF = float(width);
        const Float size = Float(kCharacterSize(app));
        const Float wide = f.mul(width, size);
        const Float height = Float(x86::sreg32(ch.height));
        const float heightF = float(height);
        const Float high = f.mul(height, size);
        const Float offsetX = f.mul(Float(x86::sreg16(kOffsetX(app, int32_t(ch.number)))), size);
        const Float perHeight = f.div(Float(1.0), Float(x86::sreg32(texture.height)));
        const float y = float(Float(x86::sreg32(ch.y)));
        const Float offsetY = f.mul(size, Float(x86::sreg16(kOffsetY(app, int32_t(ch.number)))));
        const float textureX = float(Float(x86::sreg32(ch.textureX)));
        const float textureY = float(Float(x86::sreg32(ch.textureY)));
        const float wideF = float(wide);
        const Float left = f.add(offsetX, Float(ch.x));
        const float highF = float(high);
        setFloat(app, esp + 4, float(left));
        const int32_t leftPixel = chopped<int32_t>(cpu, left);
        const float top = float(f.add(offsetY, Float(y)));
        const Float right = f.add(left, Float(wideF));
        const float perWidthF = float(perWidth);
        const float rightF = float(right);
        const int32_t rightPixel = chopped<int32_t>(cpu, right);
        const float perHeightF = float(perHeight);
        eax = reg32(leftPixel);
        if (leftPixel != rightPixel)
        {
            const int32_t topPixel = chopped<int32_t>(cpu, Float(top));
            const Float bottom = f.add(Float(top), Float(highF));
            eax = reg32(topPixel);
            const float bottomF = float(bottom);
            if (topPixel != chopped<int32_t>(cpu, bottom))
            {
                // Four vertices from the font's list: 0 the top left, 1 the
                // top right, 2 the bottom right, 3 the bottom left.
                const reg32 list = fontList(app);
                Taken q[4];
                for (Taken& t : q)
                    t = nextQuad(app, list);
                const reg32 p0 = q[0].vertex, p1 = q[1].vertex, p2 = q[2].vertex, p3 = q[3].vertex;
                setWord(app, p3 + kVertexColour, ch.colour);
                setWord(app, p2 + kVertexColour, word(app, p3 + kVertexColour));
                setWord(app, p1 + kVertexColour, word(app, p2 + kVertexColour));
                setWord(app, p0 + kVertexColour, word(app, p1 + kVertexColour));
                setWord(app, p3 + kVertexX, word(app, esp + 4));
                setFloat(app, p0 + kVertexX, float(Float(floatAt(app, p3 + kVertexX))));
                setWord(app, p2 + kVertexX, bitsOf(rightF));
                setFloat(app, p1 + kVertexX, float(Float(floatAt(app, p2 + kVertexX))));
                setWord(app, p1 + kVertexY, bitsOf(top));
                setFloat(app, p0 + kVertexY, float(Float(floatAt(app, p1 + kVertexY))));
                setWord(app, p3 + kVertexY, bitsOf(bottomF));
                const float bottomAgain = floatAt(app, p3 + kVertexY);
                const Float u0 = f.mul(Float(textureX), Float(perWidthF));
                setFloat(app, p2 + kVertexY, float(Float(bottomAgain)));
                setFloat(app, p3 + kVertexU, float(u0));
                const Float textureRight = f.add(Float(textureX), Float(widthF));
                const float u0Again = floatAt(app, p3 + kVertexU);
                const Float u1 = f.mul(Float(perWidthF), textureRight);
                setFloat(app, p0 + kVertexU, float(Float(u0Again)));
                setFloat(app, p2 + kVertexU, float(u1));
                const float u1Again = floatAt(app, p2 + kVertexU);
                const Float v0 = f.mul(Float(textureY), Float(perHeightF));
                setFloat(app, p1 + kVertexU, float(Float(u1Again)));
                setFloat(app, p1 + kVertexV, float(v0));
                setFloat(app, p0 + kVertexV, float(Float(floatAt(app, p1 + kVertexV))));
                const Float v1 = f.mul(f.add(Float(textureY), Float(heightF)), Float(perHeightF));
                setFloat(app, p3 + kVertexV, float(v1));
                setFloat(app, p2 + kVertexV, float(Float(floatAt(app, p3 + kVertexV))));
                const Result r = queued(app, cpu, esp - 0x10 - 0x5c - 4 - 4, fontList(app), q[0].number, q[1].number,
                                        q[2].number, q[3].number);
                if (!r)
                    return {};
                eax = reg32(*r);
            }
        }
    }
    const x86::sreg16 advance = kAdvance(app, int32_t(ch.number));
    eax = (eax & 0xffff0000) | x86::reg16(advance);
    return Width{eax, f.mul(Float(advance), Float(kCharacterSize(app)))};
}

WidthResult measured(App* app, CPU& cpu, reg32 esp, reg32 text);

/* 0x4d1480's: text `text` measured, and its last character's own width past
 * where the next would go. */
WidthResult measuredWithOverhang(App* app, CPU& cpu, reg32 esp, reg32 text)
{
    const WidthResult m = measured(app, cpu, esp - 0x18 - 4, text);
    if (!m)
        return {};
    const float width = float(m->value);
    const uint8_t last = byteAt(app, text + length(app, text) - 1);
    const uint8_t i = uint8_t(uint8_t(uint32_t(uint8_t(kFont(app))) * kCharacters) + uint8_t(last - 0x20));
    const int32_t advance = kAdvance(app, i);
    const int32_t overhang = int32_t(kGlyphWidth(app, i)) + kOffsetX(app, i) - advance;
    return Width{reg32(advance),
                 fp::add(fp::mul(Float(x86::sreg32(overhang)), Float(kCharacterSize(app))), Float(width))};
}

/* 0x4d1000's: text `text` at x, y in the font's colour, opaque -- its last
 * character no further right than the screen's edge -- or only measured.
 * How wide. */
WidthResult drawnText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y)
{
    auto& f = cpu.fpu;
    const reg32 at = esp - 0x10 - 0x18;
    const reg32 colour = kTextColour(app) | 0xff000000;
    float place = float(Float(x86::sreg32(x)));
    float start = place;
    if (kMeasuring(app) == 0)
    {
        const WidthResult w = measuredWithOverhang(app, cpu, at - 4, text);
        if (!w)
            return {};
        const float width = float(w->value);
        const Float end = f.add(Float(place), Float(width));
        const float screen = float(Float(x86::sreg32(kScreenWidth(app))));
        f.compare(end, Float(screen));
        if (!f.status.c0)
        {
            // Past the edge: moved back to end a pixel short of it.
            place = float(f.add(f.sub(Float(screen), Float(width)), Float(kMinusOne(app))));
            start = place;
        }
    }
    const reg32 font = (reg32(kFont(app)) << 5) * 7;
    for (reg32 p = text; byteAt(app, p) != 0; ++p)
    {
        const reg32 n = reg32(byteAt(app, p)) - 0x20 + font;
        const int32_t measuring = kMeasuring(app);
        const Character ch{n, kTextureX(app, int32_t(n)), kTextureY(app, int32_t(n)), place, y,
                           kGlyphWidth(app, int32_t(n)), kGlyphHeight(app, int32_t(n)) + 1, colour, measuring};
        const WidthResult advance = drawnCharacter(app, cpu, at - 0x18 - 4, ch);
        if (!advance)
            return {};
        place = float(f.add(advance->value, Float(place)));
    }
    return Width{0, f.sub(Float(place), Float(start))};
}

/* 0x4d1430's: how wide text `text` is, in whole pixels. */
WidthResult measured(App* app, CPU& cpu, reg32 esp, reg32 text)
{
    const int32_t was = kMeasuring(app);
    kMeasuring.set(app, 1);
    const WidthResult w = drawnText(app, cpu, esp - 0x10 - 8 - 4, text, 0, 0);
    if (!w)
        return {};
    const int32_t whole = chopped<int32_t>(cpu, w->value);
    kMeasuring.set(app, was);
    return Width{reg32(whole), Float(x86::sreg32(whole))};
}

/* ---- The menus' text ---- */

/* 0x452020's: how wide text `text` is in font `font`: on the 640 wide screen
 * of the menus when the 3D card draws them, else as the 2D screens draw it;
 * 5 for none. */
Result menuTextWidth(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 font)
{
    if (text == 0)
        return 5;
    kScreenFont.set(app, font);
    const reg32 at = esp - 0x10 - 4;
    if (kInRace(app) != 0)
    {
        const WidthResult m = measured(app, cpu, at - 4, text);
        if (!m)
            return {};
        const Float wide = fp::mul(m->value, Float(kMenuWidth(app)));
        return chopped<int32_t>(cpu, fp::div(wide, Float(x86::sreg32(kScreenWidth(app)))));
    }
    reg32 r;
    if (!callGame(app, cpu, at, kDrawScreenText, {text, 0, 0, 1}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4513c0's: the menus' text's colour: the 3D card's font's in a race's
 * pause screens, the 2D screens' otherwise. */
void textColourSet(App* app, reg32 colour)
{
    (kInRace(app) != 0 ? kTextColour : kScreenTextColour).set(app, colour);
}

/* 0x451590's: text `text` at x, y of the menus' screen: by the 3D card's
 * font at its place on the screen's own in a race's pause screens, the fonts
 * drawn at once -- by the 2D screens' otherwise. */
Result textDrawnAt(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y)
{
    const reg32 at = esp - 0xc;
    if (kInRace(app) != 0)
    {
        if (!drawnText(app, cpu, at - 4, text, onScreenX(app, x), onScreenY(app, y)))
            return {};
        return fontsDrawn(app, cpu, at - 4);
    }
    reg32 r;
    if (!callGame(app, cpu, at, kDrawScreenText, {text, reg32(x), reg32(y), 0}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x452190's: text `text` (none: nothing) in font `font` at x, y -- its right
 * there with alignment 1 or 4, its middle with 2 or 5; past 2 with a shadow
 * in colour 8 a pixel down and right first -- in `colour`. */
Result menuTextDrawn(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 font, int32_t align,
                     reg32 colour)
{
    if (text == 0)
        return 0;
    kScreenFont.set(app, font);
    const reg32 at = esp - 0xc - 4 - 4;
    if (align == 1 || align == 4 || align == 2 || align == 5)
    {
        const Result w = menuTextWidth(app, cpu, at, text, font);
        if (!w)
            return {};
        const int32_t by = align == 1 || align == 4 ? *w : (*w - (*w >> 31)) >> 1;
        x = int32_t(reg32(x) - reg32(by));
    }
    if (align > 2)
    {
        textColourSet(app, 8);
        if (!textDrawnAt(app, cpu, at, text, int32_t(reg32(x) + 1), int32_t(reg32(y) + 1)))
            return {};
    }
    textColourSet(app, colour);
    return textDrawnAt(app, cpu, at, text, x, y);
}

/* 0x451530's: the screen behind the menus dimmed by `shade` (the text scale
 * set to it) -- in a race's pause screens by one of three shades instead: 1
 * or more [0x791b50], 0.6 or more [0x791b54], less [0x791b4c], put in the
 * argument's place (`slot`).  eax: that shade, else as it was. */
reg32 dimmed(App* app, CPU& cpu, reg32 slot, reg32 eax)
{
    if (kInRace(app) != 0)
    {
        const double shade = double(Float(floatAt(app, slot)));
        cpu.fpu.compare(Float(1.0), Float(shade));
        if (!cpu.fpu.status.c0 && !cpu.fpu.status.c3)
        {
            cpu.fpu.compare(Float(shade), Float(kSixTenths(app)));
            eax = cpu.fpu.status.c0 ? kShadeLess(app) : kShadeMost(app);
        }
        else
            eax = kShadeFull(app);
        setWord(app, slot, eax);
    }
    scaleSet(app, cpu, word(app, slot));
    return eax;
}

/* 0x451500's: the mouse read -- its buttons to [buttons], x to [x], y to
 * [y], the wheel to [wheel] -- x and y put on the menus' 640 by 480 screen in
 * a race's pause screens. */
Result menuMouseRead(App* app, CPU& cpu, reg32 esp, reg32 buttons, reg32 x, reg32 y, reg32 wheel)
{
    reg32 r;
    if (!callGame(app, cpu, esp - 0xc, kReadMouse, {buttons, x, y, wheel}, {}, r))
        return {};
    if (kInRace(app) != 0)
    {
        setWord(app, x, reg32(onMenuX(app, int32_t(word(app, x)))));
        r = reg32(onMenuY(app, int32_t(word(app, y))));
        setWord(app, y, r);
    }
    return int32_t(r);
}

/* ---- A text in a box ---- */

/* 0x451b50's: the next word of the text at `from` into `to`: a line's end
 * (\r and what follows it), a run of spaces and tabs, or the characters up
 * to a space, a tab, a line's end or the text's -- its tabs then shown as
 * three spaces.  How much of the text it took. */
int32_t wordTaken(App* app, reg32 from, reg32 to)
{
    int32_t n = 0;
    const uint8_t first = byteAt(app, from);
    if (first == 0xd)
    {
        setByte(app, to, 0xd);
        setByte(app, to + 1, byteAt(app, from + 1));
        n = 2;
        setByte(app, to + 2, 0);
    }
    else if (first == 0x20 || first == 9)
    {
        for (uint8_t c; (c = byteAt(app, from + reg32(n))) == 0x20 || c == 9; ++n)
            setByte(app, to + reg32(n), c);
        setByte(app, to + reg32(n), 0);
    }
    else if (first == 0)
        setByte(app, to, 0);
    else
    {
        for (uint8_t c; (c = byteAt(app, from + reg32(n))) != 0 && c != 0x20 && c != 9 && c != 0xd && c != 0xa; ++n)
            setByte(app, to + reg32(n), c);
        setByte(app, to + reg32(n), 0);
    }
    // Each tab of the word turned into the box's three spaces.
    for (;;)
    {
        reg32 tab = 0;
        while (byteAt(app, to + tab) != 0 && byteAt(app, to + tab) != 9)
            ++tab;
        if (byteAt(app, to + tab) == 0)
            break;
        std::string expanded;
        for (reg32 i = 0; i < tab; ++i)
            expanded += char(byteAt(app, to + i));
        for (reg32 s = kTabText.at; byteAt(app, s) != 0; ++s)
            expanded += char(byteAt(app, s));
        for (reg32 s = to + tab + 1; byteAt(app, s) != 0; ++s)
            expanded += char(byteAt(app, s));
        for (std::size_t i = 0; i <= expanded.size(); ++i)
            setByte(app, to + reg32(i), i < expanded.size() ? uint8_t(expanded[i]) : 0);
    }
    return n;
}

/* What 0x451cd0 is handed: the text, its font, colour and place (y moves a
 * line down as each is drawn, in the argument's place on the stack), how wide
 * a line may be; where to put how wide the widest line is, how high the box,
 * how many lines; whether it is only measured; which lines are drawn, from
 * the first. */
struct Box
{
    reg32 text, font, colour;
    int32_t x, y;
    int32_t width;
    reg32 widest, height, lines;
    int32_t measureOnly;
    int32_t count, first;
};

/* 0x451cd0's: a text wrapped into a box a word at a time, each line drawn by
 * the 2D screens' font at its size [0x559450], 14 of it a line -- in a race's
 * pause screens only measured, as one line.  eax as its code leaves it. */
Result wrapped(App* app, CPU& cpu, reg32 esp, Box b)
{
    auto& f = cpu.fpu;
    const reg32 at = esp - 0xc - 0x490;  // where its calls are made from
    const reg32 line = at;                // the line so far, 0x400 bytes
    const reg32 wordBuffer = esp - 0x9c;  // the word taken
    const reg32 ySlot = esp + 4;
    const reg32 lastWidthSlot = esp - 0x20;  // its local of the last line's width
    if (b.widest != 0)
        setWord(app, b.widest, 5);
    if (b.height != 0)
        setWord(app, b.height,
                reg32(chopped<int32_t>(cpu, f.mul(Float(kScreenTextSize(app)), Float(kScreenLineHeight(app))))));
    kScreenFont.set(app, b.font);
    reg32 eax = b.font;
    if (b.text == 0)
        return int32_t(eax);
    if (kInRace(app) != 0)
    {
        if (b.widest == 0)
            return int32_t(eax);
        const WidthResult w = measuredWithOverhang(app, cpu, at - 4, b.text);
        if (!w)
            return {};
        setWord(app, b.widest, reg32(chopped<int32_t>(cpu, w->value)));
        return int32_t(b.widest);
    }
    const auto drawLine = [&](reg32 shown) {
        reg32 r;
        if (!callGame(app, cpu, at, kScreenTextLine, {line, reg32(b.x), reg32(b.y)},
                      {bitsOf(kScreenTextSize(app)), shown, b.colour + kScreenTextShade(app)}, r))
            return false;
        eax = r;
        return true;
    };
    int32_t widest = 0, lines = 0;
    bool drawn = b.first <= 0 && int32_t(reg32(b.first) + reg32(b.count)) > 0;
    bool measuredLine = false;
    int32_t lastWidth = 0;
    reg32 cursor = b.text;
    setByte(app, line, 0);
    for (;;)
    {
        eax = cursor;
        if (byteAt(app, cursor) == 0)
            break;
        const int32_t taken = wordTaken(app, cursor, wordBuffer);
        bool newLine = false;
        if (byteAt(app, wordBuffer) == 0xd)
        {
            newLine = true;
            setByte(app, wordBuffer, 0);
        }
        const reg32 end = line + length(app, line);
        appendString(app, line, wordBuffer);
        reg32 w;
        if (!callGame(app, cpu, at, kDrawScreenText, {line, 0, 0, 1}, {}, w))
            return {};
        lastWidth = chopped<int32_t>(cpu, f.mul(Float(x86::sreg32(w)), Float(kScreenTextSize(app))));
        measuredLine = true;
        eax = reg32(lastWidth);
        if (b.width < lastWidth || newLine)
        {
            const bool tooWide = lastWidth > b.width;
            if (tooWide)
            {
                // The word goes to the next line: this one without it.
                setByte(app, end, 0);
                if (!callGame(app, cpu, at, kDrawScreenText, {line, 0, 0, 1}, {}, w))
                    return {};
                eax = w;
                if (int32_t(w) > widest)
                    widest = int32_t(w);
            }
            if (b.measureOnly == 0 && drawn && !drawLine(0))
                return {};
            ++lines;
            if (drawn)
            {
                const Float down = f.mul(Float(kScreenTextSize(app)), Float(kScreenLineHeight(app)));
                eax = reg32(b.y);
                b.y = chopped<int32_t>(cpu, f.add(down, Float(x86::sreg32(b.y))));
                setWord(app, ySlot, reg32(b.y));
            }
            eax = reg32(lines);
            drawn = lines >= b.first && int32_t(reg32(b.first) + reg32(b.count)) > lines;
            setByte(app, line, 0);
            if (tooWide)
            {
                eax = reg32(lastWidth);
                const uint8_t c = byteAt(app, wordBuffer);
                if (c != 0x20 && c != 9)
                    copyString(app, line, wordBuffer);
            }
        }
        cursor += reg32(taken);
    }
    if (b.height != 0)
    {
        eax = reg32(lines + 1) * 14;
        setWord(app, b.height,
                reg32(chopped<int32_t>(cpu, f.mul(Float(x86::sreg32(eax)), Float(kScreenTextSize(app))))));
    }
    if (b.widest != 0)
    {
        eax = reg32(widest);
        setWord(app, b.widest, eax);
    }
    if (b.measureOnly == 0 && drawn && !drawLine(reg32(b.measureOnly)))
        return {};
    if (lines == 0 && b.widest != 0)
    {
        // Its local of the width read where it is, unset for an empty text.
        eax = measuredLine ? reg32(lastWidth) : word(app, lastWidthSlot);
        setWord(app, b.widest, eax);
    }
    if (b.lines != 0)
    {
        eax = reg32(lines + 1);
        setWord(app, b.lines, eax);
    }
    return int32_t(eax);
}

/* ---- The menus' key ---- */

/* 0x451840's: the scan code of a menu key -- the arrows, Enter, Escape --,
 * 0 for another; 0x4518c0's the other way. */
reg32 scanCodeOf(reg32 key)
{
    switch (key)
    {
    case 0x4800: return 0x48;
    case 0x5000: return 0x50;
    case 0x4b00: return 0x4b;
    case 0x4d00: return 0x4d;
    case kEnter: return 0x1c;
    case kEscape: return 1;
    default: return 0;
    }
}

reg32 keyOf(reg32 scanCode)
{
    switch (scanCode)
    {
    case 0x48: return 0x4800;
    case 0x50: return 0x5000;
    case 0x4b: return 0x4b00;
    case 0x4d: return 0x4d00;
    case 0x1c: return kEnter;
    case 1: return kEscape;
    default: return 0;
    }
}

/* 0x451810's: Alt+P down: the screen captured. */
Result capturedKeys(App* app, CPU& cpu, reg32 esp)
{
    const reg32 at = esp - 4;
    reg32 r;
    if (!callGame(app, cpu, at, kKeyDown, {kAltScanCode}, {}, r))
        return {};
    if (r == 0)
        return int32_t(r);
    if (!callGame(app, cpu, at, kKeyDown, {kPScanCode}, {}, r))
        return {};
    if (r == 0)
        return int32_t(r);
    if (!callGame(app, cpu, at, kScreenCapture, {r}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x49c000's: the mouse as a menu key: a button clicked Enter (the left) or
 * nothing (another), the click taken; the wheel up and down. */
Result mouseKey(App* app, CPU& cpu, reg32 esp)
{
    if (!menuMouseRead(app, cpu, esp - 0x14 - 4, kMouseButtons, kMouseX, kMouseY, kMouseWheel))
        return {};
    if (kMouseOn(app) == 0)
        return 0;
    const uint8_t buttons = kMouseClickButtons(app);
    if ((buttons & 0xf) != 0)
    {
        kMouseClick.set(app, 0);
        return (buttons & 1) != 0 ? int32_t(kEnter) : 0;
    }
    const int32_t wheel = kMouseWheelTurn(app);
    return wheel == 0 ? 0 : wheel > 0 ? 0x4800 : 0x5000;
}

/* Whether key `key` is held down (GetKeyState's top bit). */
std::optional<bool> keyHeld(App* app, CPU& cpu, reg32 at, reg32 key)
{
    reg32 state;
    if (!callGame(app, cpu, at, word(app, kGetKeyState), {key}, {key}, state))
        return {};
    return (state & 0x8000) != 0;
}

/* The keys queued thrown away. */
bool keysFlushed(App* app, CPU& cpu, reg32 at)
{
    for (reg32 r = 1; r != 0;)
        if (!callGame(app, cpu, at, kNextKey, {}, {}, r))
            return false;
    return true;
}

/* 0x451960's: the menus' key now -- the mouse's, else the keyboard's, else
 * (every 40 ticks) the joystick's -- as a key and its scan code, each made
 * from the other where only one came; Enter's click played.  With `repeating`
 * a key held repeats after 45 ticks every 15, and Enter or Escape still held
 * from the last time gives none, the keys queued thrown away. */
Result menuKeyOf(App* app, CPU& cpu, reg32 esp, reg32 repeating)
{
    const reg32 at = esp - 0x14;
    if (!capturedKeys(app, cpu, at - 4))
        return {};
    const Result mouse = mouseKey(app, cpu, at - 4);
    if (!mouse)
        return {};
    reg32 eax = reg32(*mouse);
    kKeyFromMouse.set(app, 1);
    if (eax == 0)
    {
        kKeyFromMouse.set(app, 0);
        if (!callGame(app, cpu, at, kNextKey, {}, {}, eax))
            return {};
    }
    if (eax == 0 && kClock(app) > kJoystickNext(app))
    {
        kJoystickNext.set(app, int32_t(reg32(kClock(app)) + 0x28));
        eax = 0;  // 0x451700: no joystick in this build
    }
    reg32 scanCode = reg32(int32_t(eax) >> 16), key = eax & 0xffff;
    if ((scanCode == 0x1a && key == 0x1b) || (scanCode == 0xe && key == 0x7f))
        scanCode = key = 0;
    if (key != 0 && scanCode == 0)
        scanCode = eax = scanCodeOf(key);
    if (scanCode != 0 && key == 0)
        key = eax = keyOf(scanCode);
    if (key != 0 && !callGame(app, cpu, at, kKeyFeedback, {eax}, {}, eax))
        return {};
    kKey.set(app, key);
    kKeyScanCode.set(app, scanCode);
    if (key != 0)
        kLastKey.set(app, key);
    if (key == kEnter && !callPort(app, cpu, at, playMenuEffect, {1, 0x7f}, eax))
        return {};
    if (repeating != 0 && kHeldKey(app) != 0)
    {
        for (const reg32 held : {kEnter, kEscape})
        {
            if (kHeldKey(app) != held)
                continue;
            bool flush = key == held;
            if (!flush)
            {
                const std::optional<bool> down = keyHeld(app, cpu, at, held);
                if (!down)
                    return {};
                flush = *down;
            }
            if (flush)
                return keysFlushed(app, cpu, at) ? Result(0) : Result();
        }
        if (key == kHeldKey(app))
        {
            if (kClock(app) < kRepeatFirst(app) || kClock(app) <= kRepeatNext(app))
                return 0;
            kRepeatNext.set(app, int32_t(reg32(kClock(app)) + 0xf));
            return int32_t(key);
        }
    }
    kHeldKey.set(app, key);
    const reg32 first = reg32(kClock(app)) + 0x2d;
    kRepeatFirst.set(app, int32_t(first));
    kRepeatNext.set(app, int32_t(first + 0xf));
    reg32 r;
    if (key != 0 && !callGame(app, cpu, at, kResetAttractTimer, {first + 0xf}, {}, r))
        return {};
    return int32_t(key);
}

}  // namespace

/* ---- The adapters: the game's registers and stack in, the result in eax
 * (a width on the x87's stack too) out. ---- */

namespace
{

void backWith(CPU& cpu, const Entry& e, const WidthResult& w, reg32 popped = 0)
{
    if (!w)
        return;
    e.back(cpu, w->eax, popped);
    cpu.fpu.push(w->value);
}

}  // namespace

/* 0x452020: how wide the text at eax is in font edx. */
void textWidth(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, menuTextWidth(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x452100: the 2D screens' font, eax. */
void selectFont(App* app, CPU& cpu)
{
    const Entry e(cpu);
    kScreenFont.set(app, e.eax);
    e.back(cpu, e.eax);
}

/* 0x4d1430: how wide the text at eax is, in whole pixels, onto the x87's
 * stack. */
void measureText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    backWith(cpu, e, measured(app, cpu, e.esp, e.eax));
}

/* 0x4d1480: the text at eax measured, its last character's overhang added,
 * onto the x87's stack. */
void textWidthWithOverhang(App* app, CPU& cpu)
{
    const Entry e(cpu);
    backWith(cpu, e, measuredWithOverhang(app, cpu, e.esp, e.eax));
}

/* 0x4d1000: the text at eax at x edx, y ebx; how wide onto the x87's stack. */
void drawText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    backWith(cpu, e, drawnText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x4d06f0 (stdcall 0x18): character eax with its texture's square at edx,
 * ebx, x (a float), y, width, height, colour and whether only measured the
 * stack's words; how far the next goes onto the x87's stack. */
void drawCharacter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Character ch{e.eax,
                       int32_t(e.edx),
                       int32_t(e.ebx),
                       floatAt(app, e.esp + 4),
                       int32_t(e.arg(app, 1)),
                       int32_t(e.arg(app, 2)),
                       int32_t(e.arg(app, 3)),
                       e.arg(app, 4),
                       int32_t(e.arg(app, 5))};
    backWith(cpu, e, drawnCharacter(app, cpu, e.esp, ch), 0x18);
}

/* 0x49bcd0: the next free vertex of the list at eax: its number to [ebx], its
 * address to [edx] and eax. */
void takeQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    // As the game's code orders the writes: the number, the list, the address.
    const reg32 n = word(app, e.eax + kListNextVertex);
    setWord(app, e.ebx, n);
    setWord(app, e.eax + kListNextVertex, n - 1);
    const reg32 vertex = e.eax + kListVertices + (n << 5);
    setWord(app, e.edx, vertex);
    e.back(cpu, vertex);
}

/* 0x4beb80: the screen's viewport: its left to [eax], top to [edx], width to
 * [ebx], height to [ecx] and eax. */
void viewport(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Rectangle r = screenViewport(app);
    setWord(app, e.eax, reg32(r.left));
    setWord(app, e.edx, reg32(r.top));
    setWord(app, e.ebx, reg32(r.width));
    setWord(app, e.ecx, reg32(r.height));
    e.back(cpu, reg32(r.height));
}

/* 0x451400, 0x451440: eax on the 640 by 480 screen of the menus, on the
 * screen's own. */
void toScreenX(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(onScreenX(app, int32_t(e.eax))));
}

void toScreenY(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(onScreenY(app, int32_t(e.eax))));
}

/* 0x4d17b0: the text of number eax past the language's lines. */
void specialText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, nameText(app, cpu, e.esp, int32_t(e.eax), e.edx));
}

/* 0x4d1850: line eax of the language's text, or a name past them. */
void languageText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, lineText(app, cpu, e.esp, int32_t(e.eax), e.edx));
}

/* 0x4d1340: both fonts' quads drawn. */
void flushFonts(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, fontsDrawn(app, cpu, e.esp));
}

/* 0x4d1390 (stdcall 4): the text scale set to the float on the stack, the one
 * before onto the x87's stack; eax as it was. */
void setTextScale(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 before = scaleSet(app, cpu, e.arg(app, 0));
    float was;
    std::memcpy(&was, &before, sizeof was);
    e.back(cpu, e.eax, 4);
    cpu.fpu.push(Float(was));
}

/* 0x4d1400: the text scale kept, eax it. */
void keepTextScale(App* app, CPU& cpu)
{
    const Entry e(cpu);
    kKeptTextScale.set(app, kTextScale(app));
    e.back(cpu, kKeptTextScale(app));
}

/* 0x4d1410: the text scale kept put back. */
void restoreTextScale(App* app, CPU& cpu)
{
    const Entry e(cpu);
    scaleSet(app, cpu, kKeptTextScale(app));
    e.back(cpu, e.eax);
}

/* 0x4d1540, 0x4d1590: character al's height (less one) and its offset down,
 * in pixels of the font as it is drawn. */
void characterWidth(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(characterHeightOf(app, cpu, uint8_t(e.eax))));
}

void characterOverhang(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(characterDropOf(app, cpu, uint8_t(e.eax))));
}

/* 0x4513c0: the menus' text's colour, eax. */
void setTextColour(App* app, CPU& cpu)
{
    const Entry e(cpu);
    textColourSet(app, e.eax);
    e.back(cpu, e.eax);
}

/* 0x451480, 0x4514c0: eax on the screen's own, on the menus' 640 by 480. */
void screenToMenuX(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(onMenuX(app, int32_t(e.eax))));
}

void screenToMenuY(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(onMenuY(app, int32_t(e.eax))));
}

/* 0x451500: the mouse read to [eax] (buttons), [edx] (x), [ebx] (y), [ecx]
 * (the wheel). */
void readMenuMouse(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, menuMouseRead(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

/* 0x451530 (stdcall 4): the screen behind the menus dimmed by the float on
 * the stack. */
void dimBackground(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, dimmed(app, cpu, e.esp + 4, e.eax), 4);
}

/* 0x451590: text eax at edx, ebx of the menus' screen. */
void drawTextAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, textDrawnAt(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x452190 (stdcall 8): text eax in font ecx at edx, ebx, its alignment and
 * colour the stack's words. */
void drawMenuText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             menuTextDrawn(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx, int32_t(e.arg(app, 0)),
                           e.arg(app, 1)),
             8);
}

/* 0x49be40: the font's quads of the list at eax drawn. */
void flushQuads(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, quadsDrawn(app, cpu, e.esp, e.eax));
}

/* 0x451700: the joystick as a menu key: none (its code is not in this build
 * of the game). */
void joystickKey(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, 0);
}

/* 0x451810: the screen captured on Alt+P. */
void captureKeys(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, capturedKeys(app, cpu, e.esp));
}

/* 0x451840, 0x4518c0: menu key eax's scan code; scan code eax's key. */
void scanCodeOfKey(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, scanCodeOf(e.eax));
}

void keyOfScanCode(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, keyOf(e.eax));
}

/* 0x451960: the menus' key, repeating with eax set. */
void menuKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, menuKeyOf(app, cpu, e.esp, e.eax));
}

/* 0x451b20: no key held. */
void resetMenuKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    kRepeatFirst.set(app, 0);
    kRepeatNone.set(app, 0xffffffff);
    kHeldKey.set(app, 0);
    e.back(cpu, e.eax);
}

/* 0x451b50: the next word of the text at eax into edx; how much it took. */
void nextWord(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(wordTaken(app, e.eax, e.edx)));
}

/* 0x451cd0 (stdcall 0x20): the text at eax wrapped into a box, in font edx,
 * colour ebx, at x ecx; y, the width, where the widest line, the height and
 * the lines go, whether only measured, how many lines from which drawn, the
 * stack's words. */
void drawWrappedText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Box b{e.eax,
                e.edx,
                e.ebx,
                int32_t(e.ecx),
                int32_t(e.arg(app, 0)),
                int32_t(e.arg(app, 1)),
                e.arg(app, 2),
                e.arg(app, 3),
                e.arg(app, 4),
                int32_t(e.arg(app, 5)),
                int32_t(e.arg(app, 6)),
                int32_t(e.arg(app, 7))};
    e.finish(cpu, wrapped(app, cpu, e.esp, b), 0x20);
}

/* 0x451fc0 (stdcall 0x18): that box, every line drawn. */
void drawTextBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Box b{e.eax,
                e.edx,
                e.ebx,
                int32_t(e.ecx),
                int32_t(e.arg(app, 0)),
                int32_t(e.arg(app, 1)),
                e.arg(app, 2),
                e.arg(app, 3),
                e.arg(app, 4),
                int32_t(e.arg(app, 5)),
                0x400,
                0};
    e.finish(cpu, wrapped(app, cpu, e.esp - 0xc - 0x20 - 4, b), 0x18);
}

/* 0x451ff0 (stdcall 8): the text at eax in font edx measured in a box ebx
 * wide: the widest line to [ecx], the height and the lines to the stack's
 * words. */
void measureTextBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Box b{e.eax, e.edx, 0, 0, 0, int32_t(e.ebx), e.ecx, e.arg(app, 0), e.arg(app, 1), 1, 0x400, 0};
    e.finish(cpu, wrapped(app, cpu, e.esp - 0xc - 0x20 - 4, b), 8);
}

/* 0x452110: the menus' end: nothing to do. */
void menusEnd(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

/* 0x49bd30 (stdcall 4): the quad of vertices edx, ebx, ecx and the stack's
 * word queued on the list at eax. */
void queueQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, queued(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* 0x49bdc0 (stdcall 4): and taken off it. */
void unqueueQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, unqueued(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

/* 0x49c000: the mouse as a menu key. */
void mouseMenuKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, mouseKey(app, cpu, e.esp));
}

/* 0x4d1750: the language eax's text loaded. */
void loadLanguageText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, loadedLanguageText(app, cpu, e.esp, int32_t(e.eax)));
}

}  // namespace text
}  // namespace port
}  // namespace nfs3hp
