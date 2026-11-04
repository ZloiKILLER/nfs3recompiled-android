#include "port_dialogs.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

/* The dialogs over a screen, rewritten (port.h says how): a question and its
 * buttons, a message that goes away by itself, a line to type in, a list to
 * tick entries of.
 *
 * One dialog is up at a time (kShown), of a kind (kKind), with its lines of
 * text, its buttons and the button highlighted (kFocus).  While it is up the
 * menu loop hands it each pass (0x445f50) and each key (0x445e60).  An answer
 * -- Enter, Escape, the message's time run out -- goes to the dialog's
 * callback, with a byte the callback clears to keep the dialog up; what the
 * callback returns goes back to the menu loop.  The box fades in over a dozen
 * passes and the keys wait for it.  The pointer highlights the button it is
 * on, and rests long enough on one with a line of help for the help box to
 * show (0x445ac0).
 *
 * The layout is worked out as the dialog opens (native_frontend_dialogs.cpp,
 * native whole), in floats on the 640x480 screen: a box round the lines, one
 * round the buttons, put together and centred (kRect: top, left, bottom,
 * right), the buttons placed below the lines in a row or one above the other
 * (kButtonBoxes).  What is rewritten here draws it, in the x87's own
 * arithmetic (through cpu.fpu, as the code orders it): in a race's pause
 * screens it rounds to a float's precision, and the dialog lands where the
 * game puts it.  There the box and the buttons are quads of the pause
 * screens' art; in the menus they are shapes of the front end's own
 * (0x445de0).
 *
 * Each function is a typed core and an adapter under the game's name.  The
 * cores call each other as C++, each given the esp its game's code is entered
 * at; what is not rewritten here -- the layout, the text and the 2D drawing --
 * is called by address.  The game's code leaves ebx, ecx and edx otherwise
 * than it got them in some of these (the scratch of a Watcom function); the
 * adapters give them back as the caller had them, as no caller reads them
 * after (by liveness on nfs3.exe).  tools/game_checks holds each adapter
 * against the game's code for it (dialogs.inc). */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

// nfs3hp_main.cpp: the box's emblem kept round on a wide screen (tools/apply_widescreen.py).
void dialogEmblem(bool drawing);

namespace port
{
namespace dialogs
{

namespace
{

using x86::Float;

/* ---- The game's variables ---- */

// The dialog up.
constexpr Var<uint8_t> kShown{0x558b10};
constexpr Var<reg32> kKind{0x558b14};
constexpr reg32 kQuestion = 1, kMessage = 2, kEdit = 3, kList = 4;
constexpr Var<reg32> kCallback{0x558b18};          // (answer, &keep up)
constexpr Var<reg32> kLineCount{0x558b1c};
constexpr Var<reg32> kLines{0x558b20};             // char*[kLineCount]
constexpr Var<reg32> kButtonCount{0x558b30};
constexpr Var<reg32> kButtons{0x558b34};           // char*[kButtonCount]
constexpr Var<reg32> kButtonHelp{0x558b38};        // char*[kButtonCount], or 0
constexpr Var<reg32> kFocus{0x558b3c};             // the button highlighted, -1 for none
constexpr Var<reg32> kButtonsX{0x558b40};
// A message's.
constexpr Var<reg32> kMessageOpened{0x558b44};     // the tick it came up at
constexpr Var<reg32> kMessageTime{0x558b48};       // the ticks it stays up, 0 for as long as it takes
constexpr Var<uint8_t> kMessageKeys{0x558b4c};     // the keys answer it
// The line typed in.
constexpr Var<reg32> kReplaceOnKey{0x558b2c};
constexpr Var<reg32> kEditLength{0x558b50};
constexpr Var<reg32> kEditMax{0x558b54};
constexpr reg32 kEditText = 0x558b58;
constexpr reg32 kEditSize = 0x100;
constexpr Var<reg32> kEditMode{0x558c5c};
constexpr Var<reg32> kEditWidth{0x661070};
constexpr Var<reg32> kCursorBlink{0x558d08};       // 0 to 24: the cursor shows past 12
// A list's.
constexpr Var<reg32> kListFocus{0x558c60};         // the entry highlighted, -1 for none
constexpr Var<uint8_t> kListCount{0x558c64};
constexpr Var<reg32> kListNames{0x558c68};         // char*[kListCount]
constexpr Var<reg32> kListTicks{0x558c6c};         // int*[kListCount]: an entry's tick, Enter sets it
constexpr Var<reg32> kListHelp{0x558c70};          // char*[kListCount], or 0
constexpr Var<reg32> kListX{0x558c74};
constexpr Var<reg32> kListY{0x558c78};
constexpr int32_t kMaxButtons = 10;
constexpr int32_t kMaxEntries = 6;

// Where it is drawn: the box (floats), the buttons' and the entries' boxes
// (8 bytes each, int16), and how far it has faded in.
constexpr reg32 kRect = 0x661050;
constexpr reg32 kTop = 0x0, kLeft = 0x4, kBottom = 0x8, kRight = 0xc;
constexpr reg32 kButtonBoxes = 0x661000;
constexpr reg32 kEntryBoxes = 0x660fd0;
constexpr Var<reg32> kFade{0x661064};              // 0 to 255

// Its art: the front end's shapes, and in a race's pause screens parts of
// their art, a texture.
constexpr Var<reg32> kBoxShape{0x66108c};
constexpr Var<reg32> kButtonShape{0x661068};
constexpr Var<reg32> kFocusShape{0x661060};
constexpr Var<reg32> kTickedShape{0x558b24};       // a green light
constexpr Var<reg32> kUntickedShape{0x558b28};     // a red one
constexpr Var<reg32> kYel1Shape{0x66106c};
constexpr Var<reg32> kPauseArt{0x749a38};

// The help box over a dialog, for a button or an entry.
constexpr Var<reg32> kHelpOver{0x558afc};          // 0 a button, 1 an entry, -1 neither
constexpr Var<reg32> kHelpLastX{0x558b00};
constexpr Var<reg32> kHelpLastY{0x558b04};
constexpr Var<reg32> kHelpFor{0x558b08};           // the button or entry, -1 for none
constexpr Var<reg32> kHelpText{0x558b0c};
constexpr Var<reg32> kHelpHeight{0x661074};
constexpr Var<reg32> kHelpY{0x661078};
constexpr Var<reg32> kHelpX{0x66107c};
constexpr Var<reg32> kHelpWidth{0x661080};
constexpr Var<reg32> kHelpDue{0x661084};
constexpr Var<reg32> kHelpFade{0x661088};          // 0 to 255
constexpr reg32 kHelpWrap = 0x14a;
constexpr reg32 kHelpBoxColour = 0x4b6cad;
constexpr reg32 kHelpTextColour = 0x488cfd;
constexpr reg32 kHelpDepth = 0xffff9c;             // -100, with the alpha in the top byte
constexpr Var<reg32> kHelpAlpha{0x559450};         // float, 0.9 while a dialog's help may show
constexpr reg32 kFloatOne = 0x3f800000;
constexpr reg32 kFloatPoint9 = 0x3f666666;

// The menus'.
constexpr Var<reg32> kInRace{0x559234};
constexpr Var<reg32> kMouseX{0x55e5e4};
constexpr Var<reg32> kMouseY{0x55e5e8};
constexpr Var<reg32> kTicks{0x79c1a4};
constexpr Var<uint8_t> kDisplayFlags{0x7a3a58};
constexpr uint8_t kNoTranslucency = 0x02;
constexpr Var<reg32> kDrawColour{0x563aa4};
constexpr reg32 kOpaque = 0xff000000;
constexpr Var<reg32> kDepthNear{0x563aa8};
constexpr Var<reg32> kDepthFar{0x563aac};
constexpr reg32 kFloatAlmostOne = 0x3f7fff00;
constexpr reg32 kFloatTiny = 0x37800080;

// Text.
constexpr reg32 kFont = 0x12;
constexpr reg32 kAlignLeft = 0, kAlignCentre = 2;
constexpr reg32 kTextColour = 0x649dfd;
constexpr reg32 kFocusColour = 0xffe440;
constexpr reg32 kEdgeColour = 0x727eb3;
constexpr reg32 kFocusEdgeColour = 0xffd84b;
constexpr reg32 kWhite = 0xffffff;

// Keys, as 0x451960 gives them, and the answers they make.
constexpr uint16_t kKeyBackspace = 0x08, kKeyEnter = 0x0d, kKeyEscape = 0x1b, kKeySpace = 0x20;
constexpr uint16_t kKeyUp = 0x4800, kKeyLeft = 0x4b00, kKeyRight = 0x4d00, kKeyDown = 0x5000, kKeyDelete = 0x5300;
constexpr reg32 kAnswerEnter = 1, kAnswerSpace = 2, kAnswerEscape = 3, kAnswerTimedOut = 4;
constexpr reg32 kHandled = 2;                      // the callback's: taken, and 0 to the loop

// Sounds, 0x4181d0(sound, volume).
constexpr reg32 kSoundMove = 0, kVolumeTick = 0x2d;

// The strings.
constexpr reg32 kCursor = 0x538b04;                // "_"
constexpr reg32 kPopu = 0x538b84, kBpop = 0x538b8c, kYpop = 0x538b94, kGlit = 0x538b9c, kRlit = 0x538ba4;
constexpr reg32 kYel1 = 0x538bac;

/* ---- What they call ---- */

// By address: the game's code, or another part's rewrite held on its own.
constexpr reg32 kPlaySound = 0x4181d0;
constexpr reg32 kSetFont = 0x452100;
constexpr reg32 kTextWidth = 0x452020;             // (text, font)
constexpr reg32 kDrawText = 0x452190;              // (text, x, y, font, align, colour)
constexpr reg32 kTextBox = 0x451ff0;
constexpr reg32 kDrawTextWrapped = 0x451fc0;
constexpr reg32 kFillBox = 0x4d8550;
constexpr reg32 kBoxBorder = 0x4d8930;
constexpr reg32 kShadedBox = 0x4d8860;             // (x, y, w, h, four corners' colours)
constexpr reg32 kFieldBox = 0x4d8ac0;
constexpr reg32 kFillRect = 0x4d8910;
constexpr reg32 kDrawQuad = 0x4db000;
constexpr reg32 kDrawShape = 0x4d7160;             // (shape, x, y, colour)
constexpr reg32 kShapeSize = 0x4d66f0;             // (shape, &w, &h): int16 each
constexpr reg32 kFindShape = 0x4d5db0;
constexpr reg32 kSetClip = 0x4bebe0;
constexpr reg32 kMemset = 0x4e0640;
constexpr reg32 kSetMousePosition = 0x49bfc0;
constexpr reg32 kCharAllowed = 0x443810;
constexpr reg32 kPointAtButton = 0x444570;
constexpr reg32 kLinesBox = 0x443a90;
constexpr reg32 kButtonRowBox = 0x443c00;
constexpr reg32 kPlaceButtons = 0x443d20;
constexpr reg32 kTextAllowed = 0x443860;
constexpr reg32 kStrncpy = 0x4e0e30;

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

float floatAt(App* app, reg32 at)
{
    return app->getMemory<float>(at);
}

void setFloat(App* app, reg32 at, float value)
{
    app->getMemory<float>(at) = value;
}

constexpr reg32 entered(reg32 at)
{
    return at - 4;
}

int32_t sdword(App* app, reg32 at)
{
    return int32_t(word(app, at));
}

/* The game's code being run: its calls, and what eax is as the game's code
 * would have it at this point. */
struct Game
{
    App* app;
    CPU& cpu;
    reg32 eax;
    bool call(reg32 esp, reg32 target, Args a = {}, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, a, stack, eax);
    }
};

#define GAME(...)                 \
    do                            \
    {                             \
        if (!g.call(__VA_ARGS__)) \
            return {};            \
    } while (0)

#define NEEDED(variable, expression) \
    const Result variable = (expression); \
    if (!variable)                   \
        return {}

bool inRace(App* app)
{
    return kInRace(app) != 0;
}

/* The x87 as the code uses it: a sum or a product rounded as the control word
 * asks, a comparison as fcom leaves the status word. */
bool above(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return !cpu.fpu.status.c0 && !cpu.fpu.status.c3;
}

bool below(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.c0;
}

Float integer(int32_t value)
{
    return Float(x86::sreg32(value));
}

Float load(App* app, reg32 at)
{
    return Float(floatAt(app, at));
}

int32_t chop(CPU& cpu, const Float& value)
{
    return chopped<int32_t>(cpu, value);
}

bool setFont(Game& g, reg32 at)
{
    return g.call(at, kSetFont, {kFont});
}

reg32 fadeAlpha(App* app)
{
    return kFade(app) << 24;
}

/* A button's box or a list entry's. */
struct Box
{
    int32_t y, x, width, height;
};

reg32 boxAt(reg32 boxes, int32_t index)
{
    return boxes + reg32(index) * 8;
}

Box box(App* app, reg32 boxes, int32_t index)
{
    const reg32 at = boxAt(boxes, index);
    return {app->getMemory<int16_t>(at), app->getMemory<int16_t>(at + 2), app->getMemory<int16_t>(at + 4),
            app->getMemory<int16_t>(at + 6)};
}

void setBoxField(App* app, reg32 boxes, int32_t index, reg32 field, int32_t value)
{
    app->getMemory<uint16_t>(boxAt(boxes, index) + field) = uint16_t(value);
}

/* A part of the pause screens' art, by its corners' texture coordinates (float bits). */
struct ArtPart
{
    reg32 left, top, right, bottom;
};
constexpr ArtPart kBoxArt{0x3f000000, 0x3e800000, 0x3f7f0000, 0x3efe0000};
constexpr ArtPart kButtonArt{0x3ef80000, 0x3f140000, 0x3f0c0000, 0x3f270000};
constexpr ArtPart kFocusArt{0x3f0d0000, 0x3f140000, 0x3f1d0000, 0x3f270000};

/* 0x4db000: a part of the pause screens' art, (left, top) to (right, bottom) on the screen. */
bool drawPauseArt(Game& g, reg32 at, int32_t left, int32_t top, int32_t right, int32_t bottom, const ArtPart& part,
                  reg32 colour)
{
    return g.call(at, kDrawQuad, {reg32(left), reg32(top), reg32(right), reg32(top)},
                  {reg32(right), reg32(bottom), reg32(left), reg32(bottom), part.left, part.top, part.right, part.top,
                   part.right, part.bottom, part.left, part.bottom, kPauseArt(g.app), colour});
}

/* 0x4d8860: a box shaded between its corners' colours; a line, one pixel high. */
bool drawShaded(Game& g, reg32 at, int32_t x, int32_t y, int32_t width, int32_t height, reg32 c0, reg32 c1, reg32 c2,
                reg32 c3)
{
    return g.call(at, kShadedBox, {reg32(x), reg32(y), reg32(width), reg32(height)}, {c0, c1, c2, c3});
}

bool playSound(Game& g, reg32 at, reg32 sound, reg32 volume)
{
    return g.call(at, kPlaySound, {sound, volume});
}

/* ---- The state ---- */

/* 0x445f10's: the dialog closed. */
Result closeDialogCore(Game& g)
{
    App* app = g.app;
    kCallback.set(app, 0);
    kShown.set(app, 0);
    kKind.set(app, 0);
    kHelpText.set(app, 0);
    return int32_t(g.eax & ~0xff00u);  // ah cleared on the way
}

/* ---- Drawing ---- */

/* 0x4438d0's: the dialog's box at `rect`: a frame shaded from corner to
 * corner, fading in with the dialog, and the box's art 14 pixels out to the
 * left and 7 up -- a quad of the pause screens' art in a race. */
Result drawBoxCore(Game& g, reg32 esp, reg32 rect)
{
    App* app = g.app;
    CPU& cpu = g.cpu;
    const reg32 at = esp - 24 - 0x20;
    const int32_t x = chop(cpu, load(app, rect + kLeft));
    const int32_t y = chop(cpu, load(app, rect + kTop));
    const int32_t width = chop(cpu, fp::sub(load(app, rect + kRight), load(app, rect + kLeft)));
    const int32_t height = chop(cpu, fp::sub(load(app, rect + kBottom), load(app, rect + kTop)));
    // The corners' alphas, faded in -- one of them never reaches the end of
    // its fade, which jumps from 16 to 160 as the fade completes.
    const int32_t fade = sdword(app, 0x661064);
    reg32 corners[4] = {0xa0000060, 0xa0000050, 0xd0000020, 0xd0000010};
    if (fade < 255)
    {
        const reg32 f = reg32(fade);
        corners[0] = (reg32(int32_t(f * 160) / 255) << 24) + 0x60;
        corners[1] = (reg32(int32_t(f * 16) / 255) << 24) + 0x50;
        const reg32 alpha = reg32(int32_t(f * 208) / 255) << 24;
        corners[2] = alpha + 0x20;
        corners[3] = alpha + 0x10;
    }
    if (!drawShaded(g, at, x, y, width, height, corners[0], corners[1], corners[2], corners[3]))
        return {};
    const reg32 colour = fadeAlpha(app) + kWhite;
    if (inRace(app))
    {
        const int32_t bottom = chop(cpu, fp::add(integer(y - 7), Float(63.0)));
        const int32_t right = chop(cpu, fp::add(integer(x - 14), Float(127.0)));
        // The emblem on the art narrowed for a wide screen, for this one draw.
        dialogEmblem(true);
        const bool drawn = drawPauseArt(g, at, x - 14, y - 7, right, bottom, kBoxArt, colour);
        dialogEmblem(false);
        if (!drawn)
            return {};
        return int32_t(g.eax);
    }
    GAME(at, kDrawShape, {kBoxShape(app), reg32(x - 14), reg32(y - 7), colour});
    return int32_t(g.eax);
}

/* 0x443f40's: the dialog's lines, centred, from 19 pixels below the top of its
 * box at `rect`, 30 pixels apart. */
Result drawLinesCore(Game& g, reg32 esp, int32_t count, reg32 lines, reg32 rect)
{
    App* app = g.app;
    CPU& cpu = g.cpu;
    const reg32 at = esp - 16 - 0x18;
    const Float halfWidth = fp::mul(fp::sub(load(app, rect + kRight), load(app, rect + kLeft)), Float(0.5));
    const int32_t top = chop(cpu, load(app, rect + kTop)) + 19;
    const int32_t middle = chop(cpu, fp::add(halfWidth, load(app, rect + kLeft)));
    if (!setFont(g, at))
        return {};
    for (int32_t i = 0; i < count; ++i)
    {
        const reg32 line = word(app, lines + 4 * reg32(i));
        if (line == 0)
            continue;
        int32_t x = middle + 25;
        reg32 colour = kTextColour;
        if (inRace(app))
        {
            x = chop(cpu, fp::add(integer(middle), Float(31.75)));
            colour = kOpaque | kTextColour;
        }
        GAME(at, kDrawText, {line, reg32(x), reg32(top + 30 * i), kFont}, {kAlignCentre, colour});
    }
    return int32_t(g.eax);
}

/* 0x444000's: the dialog's buttons in their boxes, the one highlighted in
 * yellow: the button's art 17 pixels to the left of its text, and a line
 * along its top and its bottom. */
Result drawButtonsCore(Game& g, reg32 esp, int32_t focus)
{
    App* app = g.app;
    CPU& cpu = g.cpu;
    const reg32 at = esp - 20 - 0x3c;
    if (!setFont(g, at))
        return {};
    for (int32_t i = 0; i < sdword(app, 0x558b30); ++i)
    {
        const reg32 text = word(app, kButtons(app) + 4 * reg32(i));
        if (text == 0)
            continue;
        const Box b = box(app, kButtonBoxes, i);
        const bool focused = i == focus;
        const reg32 alpha = fadeAlpha(app);
        const reg32 white = alpha + kWhite;
        const reg32 solid = focused ? kFocusEdgeColour : kEdgeColour;
        const reg32 edge = alpha + solid;
        const int32_t artX = b.x - 17, artY = b.y + 1;
        if (inRace(app))
        {
            const int32_t artBottom = chop(cpu, fp::add(integer(artY), Float(19.0)));
            const int32_t artRight = chop(cpu, fp::add(integer(artX), Float(16.0)));
            if (!drawPauseArt(g, at, artX, artY, artRight, artBottom, focused ? kFocusArt : kButtonArt, white)
                || !drawShaded(g, at, b.x - 3, b.y + 1, b.width + 8, 1, edge, solid, solid, edge))
                return {};
            const int32_t bottom = chop(cpu, fp::add(integer(b.y), Float(19.0)));
            if (!drawShaded(g, at, b.x - 7, bottom, b.width + 8, 1, edge, solid, solid, edge))
                return {};
        }
        else
        {
            if (!g.call(at, kDrawShape, {focused ? kFocusShape(app) : kButtonShape(app), reg32(artX), reg32(artY), white})
                || !drawShaded(g, at, b.x - 3, b.y + 1, b.width + 8, 1, edge, solid, solid, edge)
                || !drawShaded(g, at, b.x - 3, b.y + 16, b.width + 5, 1, edge, solid, solid, edge))
                return {};
        }
        GAME(at, kDrawText, {text, reg32(b.x), reg32(b.y), kFont}, {kAlignLeft, focused ? kFocusColour : kTextColour});
    }
    return int32_t(g.eax);
}

/* 0x444350's: the line typed in, centred 90 pixels above the bottom of the
 * dialog's box at `rect`, with a blinking cursor after it: in the menus on a
 * field of its own (once the dialog is faded in), its end shown when it is
 * too long for the field. */
Result drawEditFieldCore(Game& g, reg32 esp, reg32 rect)
{
    App* app = g.app;
    CPU& cpu = g.cpu;
    const reg32 at = esp - 24 - 0x14;
    GAME(at, kTextWidth, {kEditText, kFont});
    const int32_t width = int32_t(g.eax);
    const int32_t middle = chop(cpu, fp::add(load(app, rect + kLeft), load(app, rect + kRight))) / 2;
    const int32_t y = chop(cpu, fp::add(fp::add(load(app, rect + kBottom), Float(-30.0)), Float(-60.0)));
    int32_t x = middle - width / 2 - 3;
    if (!inRace(app))
    {
        const int32_t span = chop(cpu, fp::sub(load(app, rect + kRight), load(app, rect + kLeft)));
        const int32_t fieldWidth = sdword(app, 0x661070);
        const int32_t fieldX = chop(cpu, load(app, rect + kLeft)) + (span - fieldWidth) / 2;
        const int32_t fade = sdword(app, 0x661064);
        const reg32 colour = (reg32(int32_t(reg32(fade) * 60) / 255) << 24) + 0x20;
        if (fade >= 255)
            GAME(at, kFieldBox, {reg32(fieldX), reg32(y - 1), reg32(fieldWidth), 20}, {colour, colour, colour, colour});
    }
    if (!setFont(g, at))
        return {};
    GAME(at, kTextWidth, {kEditText, kFont});
    const int32_t shownWidth = int32_t(g.eax);
    const int32_t fieldWidth = sdword(app, 0x661070);
    if (shownWidth > fieldWidth)
    {
        // Its end in the field, what goes past the field's left edge clipped.
        const int32_t fieldMiddle = chop(cpu, fp::add(load(app, rect + kLeft), load(app, rect + kRight))) / 2;
        const int32_t clip = fieldMiddle - fieldWidth / 2;
        x = fieldWidth / 2 + fieldMiddle - shownWidth - 7;
        if (!inRace(app))
            GAME(at, kSetClip, {reg32(clip), 0, reg32(640 - clip), 480});
    }
    GAME(at, kDrawText, {kEditText, reg32(x), reg32(y), kFont}, {kAlignLeft, kFocusColour});
    const int32_t blink = int32_t(kCursorBlink(app) + 1) % 25;
    kCursorBlink.set(app, reg32(blink));
    if (blink > 12)
    {
        const int32_t cursorX = width + x;
        if (inRace(app))
            GAME(at, kDrawText, {kCursor, reg32(cursorX), reg32(y), kFont}, {kAlignLeft, kFocusColour});
        else
        {
            const reg32 colour = fadeAlpha(app) + kFocusColour;
            if (!drawShaded(g, at, cursorX, y + 13, 7, 1, colour, colour, colour, colour))
                return {};
        }
    }
    if (!inRace(app))
        GAME(at, kSetClip, {0, 0, 640, 480});
    return int32_t(g.eax);
}

/* 0x445380's: the list's entries, each with its light: green ticked, red not
 * -- in a race a square of colour -- and the name after it, yellow the one
 * highlighted. */
Result drawListCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 24 - 0x14;
    // The light's size, as the art last said: 10 by 10 until it does.
    const reg32 lightWidth = at + 0x10, lightHeight = at + 0xc;  // [ebp - 4], [ebp - 8]
    app->getMemory<uint16_t>(lightWidth) = 10;
    app->getMemory<uint16_t>(lightHeight) = 10;
    if (!setFont(g, at))
        return {};
    for (int32_t i = 0; i < int32_t(kListCount(app)); ++i)
    {
        const reg32 name = word(app, kListNames(app) + 4 * reg32(i));
        if (name == 0)
            continue;
        reg32 colour = kFocusColour;
        if (i != sdword(app, 0x558c60))
            colour = inRace(app) ? kOpaque | kTextColour : kTextColour;
        const Box b = box(app, kEntryBoxes, i);
        const reg32 tick = word(app, kListTicks(app) + 4 * reg32(i));
        if (inRace(app))
        {
            GAME(at, kFillRect,
                 {reg32(b.x), reg32(b.y), reg32(app->getMemory<int16_t>(lightWidth)),
                  reg32(app->getMemory<int16_t>(lightHeight))},
                 {tick != 0 && word(app, tick) != 0 ? 0x8000u : 0x800000u});
        }
        else
        {
            const reg32 white = fadeAlpha(app) + kWhite;
            if (tick != 0 && word(app, tick) != 0)
                GAME(at, kDrawShape, {kTickedShape(app), reg32(b.x - 1), reg32(b.y + 3), white});
            else
                GAME(at, kDrawShape, {kUntickedShape(app), reg32(b.x), reg32(tick != 0 ? b.y + 2 : b.y), white});
            GAME(at, kShapeSize, {kTickedShape(app), lightWidth, lightHeight, colour});
        }
        GAME(at, kDrawText, {name, reg32(b.x + app->getMemory<int16_t>(lightWidth)), reg32(b.y), kFont},
             {kAlignLeft, colour});
    }
    return int32_t(g.eax);
}

/* ---- The pointer ---- */

/* 0x4445d0's, 0x4451d0's: the button or entry under the pointer, -1 for none.
 * In the menus a box reaches 15 pixels further left and 10 right. */
Result underMouseCore(Game& g, reg32 boxes, int32_t count)
{
    App* app = g.app;
    const int32_t x = sdword(app, 0x55e5e4), y = sdword(app, 0x55e5e8);
    for (int32_t i = 0; i < count; ++i)
    {
        const Box b = box(app, boxes, i);
        const bool hit = inRace(app) ? y > b.y && y < b.y + b.height && x > b.x && x < b.x + b.width
                                     : y > b.y - 1 && y < b.y + b.height && x > b.x - 15 && x < b.x + b.width + 10;
        if (hit)
            return i;
    }
    return -1;
}

Result buttonUnderMouseCore(Game& g)
{
    return underMouseCore(g, kButtonBoxes, sdword(g.app, 0x558b30));
}

Result entryUnderMouseCore(Game& g)
{
    return underMouseCore(g, kEntryBoxes, int32_t(kListCount(g.app)));
}

/* 0x445160's: the pointer moved onto the bottom right of an entry's box. */
Result pointAtEntryCore(Game& g, reg32 esp, int32_t index)
{
    App* app = g.app;
    if (index < 0 || index >= int32_t(kListCount(app)))
        return index;
    const Box b = box(app, kEntryBoxes, index);
    kMouseX.set(app, reg32(b.x + b.width - 2));
    kMouseY.set(app, reg32(b.y + b.height - 2));
    GAME(esp - 12, kSetMousePosition, {kMouseX(app), kMouseY(app)});
    return int32_t(g.eax);
}

/* The button under the pointer highlighted, with a tick when it changes; none
 * when the pointer is on no button. */
Result followPointer(Game& g, reg32 at)
{
    App* app = g.app;
    NEEDED(found, buttonUnderMouseCore(g));
    g.eax = reg32(*found);
    const int32_t under = *found;
    if (under < 0 || under >= sdword(app, 0x558b30))
    {
        kFocus.set(app, reg32(-1));
        return 0;
    }
    if (under != sdword(app, 0x558b3c))
        GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
    kFocus.set(app, reg32(under));
    return 0;
}

/* 0x445ac0's: the help box over a dialog, for the button or list entry the
 * pointer has rested on for 100 ticks: its line of help, beside the pointer
 * (kept on the 640x480 screen), wrapped at 330 pixels, fading in and out. */
Result drawDialogHelpCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 24 - 8;
    if (kButtonHelp(app) == 0 && kListHelp(app) == 0)
        return int32_t(g.eax);
    NEEDED(button, buttonUnderMouseCore(g));
    g.eax = reg32(*button);
    int32_t under = *button;
    int32_t over = 0;
    if (under == -1)
    {
        NEEDED(entry, entryUnderMouseCore(g));
        g.eax = reg32(*entry);
        under = *entry;
        over = under != -1 ? 1 : -1;
    }
    kHelpOver.set(app, reg32(over));
    if (under == -1 || under != sdword(app, 0x558b08) || over == -1)
        kHelpFor.set(app, reg32(-1));
    kHelpAlpha.set(app, kFloatPoint9);
    if (sdword(app, 0x558b08) == -1)
    {
        if (kHelpLastX(app) != kMouseX(app) || kHelpLastY(app) != kMouseY(app))
        {
            kHelpLastX.set(app, kMouseX(app));
            kHelpLastY.set(app, kMouseY(app));
            kHelpDue.set(app, kTicks(app) + 100);
        }
        else if (int32_t(kTicks(app)) > int32_t(kHelpDue(app)) && under != -1)
        {
            kHelpFor.set(app, reg32(under));
            kHelpFade.set(app, 0);
            const reg32 help = over == 0 ? kButtonHelp(app) : kListHelp(app);
            kHelpText.set(app, 0);
            if (help != 0)
                kHelpText.set(app, word(app, help + 4 * reg32(under)));
            // The box: the text's size, wrapped, and a margin.
            const reg32 width = at, height = at + 4;  // [ebp - 8], [ebp - 4]
            GAME(at, kTextBox, {kHelpText(app), kFont, kHelpWrap, width}, {height, 0});
            const int32_t w = sdword(app, width) + 7, h = sdword(app, height) + 3;
            kHelpX.set(app, kMouseX(app));
            kHelpWidth.set(app, reg32(w));
            kHelpHeight.set(app, reg32(h));
            if (int32_t(kMouseX(app)) + w > 630)
                kHelpX.set(app, reg32(630 - w));
            kHelpY.set(app, kMouseY(app) + 22);
            if (int32_t(kMouseY(app)) + 22 + h > 460)
                kHelpY.set(app, reg32(int32_t(kMouseY(app)) - h - 8));
        }
    }

    if (kHelpText(app) != 0)
    {
        if (sdword(app, 0x558b08) == -1)
        {
            if (int32_t(kHelpFade(app)) > 0)
                kHelpFade.set(app, kHelpFade(app) - 15);
            if (int32_t(kHelpFade(app)) <= 0)
                kHelpText.set(app, 0);
        }
        else
        {
            if (int32_t(kHelpFade(app)) < 255)
                kHelpFade.set(app, kHelpFade(app) + 10);
            if (int32_t(kHelpFade(app)) > 255)
                kHelpFade.set(app, 255);
        }
    }
    if (kHelpText(app) != 0)
    {
        const reg32 alpha = kHelpFade(app) << 24;
        kDrawColour.set(app, alpha);
        const reg32 x = kHelpX(app), y = kHelpY(app);
        const reg32 w = kHelpWidth(app), h = kHelpHeight(app);
        GAME(at, kFillBox, {x, y, x + w, y + h}, {alpha + kHelpDepth, alpha + kHelpBoxColour});
        GAME(at, kBoxBorder, {x, y, w, h}, {alpha});
        if (inRace(app))
            GAME(at, kDrawText, {kHelpText(app), x, y, kFont}, {kAlignLeft, kHelpTextColour});
        else
            GAME(at, kDrawTextWrapped, {kHelpText(app), kFont, 8, x + 7}, {y, kHelpWrap, 0, 0, 0, 0});
        kDrawColour.set(app, kOpaque);
    }
    kHelpAlpha.set(app, kFloatOne);
    return int32_t(g.eax);
}

/* ---- A question ---- */

/* 0x444910's: a key for a question: Enter or Escape answers it, the arrows
 * move the highlight round its buttons. */
Result questionKeyCore(Game& g, reg32 esp, uint16_t key)
{
    App* app = g.app;
    const reg32 at = esp - 12;
    const int32_t last = sdword(app, 0x558b30) - 1;
    int32_t focus = sdword(app, 0x558b3c);
    switch (key)
    {
    case kKeyEnter:
        return kAnswerEnter;
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyUp:
    case kKeyLeft:
        focus = int32_t(reg32(focus) - 1);
        if (focus < 0)
            focus = last;
        break;
    case kKeyDown:
    case kKeyRight:
        focus = int32_t(reg32(focus) + 1);
        if (focus > last)
            focus = 0;
        break;
    default:
        return 0;
    }
    kFocus.set(app, reg32(focus));
    GAME(at, kPointAtButton, {reg32(focus)});
    return 0;
}

/* 0x4449d0's: a question's pass: the button under the pointer highlighted,
 * the box, the lines and the buttons drawn. */
Result questionPassCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    kListFocus.set(app, reg32(-1));
    NEEDED(followed, followPointer(g, at));
    NEEDED(box, drawBoxCore(g, entered(at), kRect));
    g.eax = reg32(*box);
    NEEDED(lines, drawLinesCore(g, entered(at), sdword(app, 0x558b1c), kLines(app), kRect));
    g.eax = reg32(*lines);
    NEEDED(buttons, drawButtonsCore(g, entered(at), sdword(app, 0x558b3c)));
    g.eax = reg32(*buttons);
    return 0;
}

/* ---- A message ---- */

/* 0x444a60's: the message to go on the next pass. */
Result endMessageCore(Game& g)
{
    kMessageTime.set(g.app, 1);
    kMessageOpened.set(g.app, 0);
    return int32_t(g.eax);
}

/* 0x444b20's: a key for a message, when keys answer it: Enter, Escape, or the
 * space bar. */
Result messageKeyCore(Game& g, uint16_t key)
{
    if (kMessageKeys(g.app) == 0)
        return 0;
    switch (key)
    {
    case kKeyEnter:
        return kAnswerEnter;
    case kKeyEscape:
        return kAnswerEscape;
    case kKeySpace:
        return kAnswerSpace;
    default:
        return 0;
    }
}

/* 0x444b60's: a message's pass: the box and the lines drawn, and the answer
 * kAnswerTimedOut once its time has gone by. */
Result messagePassCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    kListFocus.set(app, reg32(-1));
    NEEDED(box, drawBoxCore(g, entered(at), kRect));
    g.eax = reg32(*box);
    NEEDED(lines, drawLinesCore(g, entered(at), sdword(app, 0x558b1c), kLines(app), kRect));
    g.eax = reg32(*lines);
    const int32_t time = sdword(app, 0x558b48);
    if (time <= 0)
        return 0;
    reg32 elapsed = kTicks(app) - kMessageOpened(app);
    if (int32_t(elapsed) <= 0)
        elapsed = 0 - elapsed;
    return int32_t(elapsed) >= time ? kAnswerTimedOut : 0;
}

/* ---- A line to type in ---- */

/* 0x444f50's: a key for the line typed in: Enter or Escape answers, Left and
 * Right move the highlight round the buttons, Backspace takes the last
 * character off and Delete the lot; any other character the mode allows goes
 * on the end while there is room (0x443810). */
Result editKeyCore(Game& g, reg32 esp, reg32 key)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    const int32_t last = sdword(app, 0x558b30) - 1;
    int32_t focus = sdword(app, 0x558b3c);
    switch (uint16_t(key))
    {
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyEnter:
        return kAnswerEnter;
    case kKeyBackspace:
        if (sdword(app, 0x558b50) > 0)
        {
            const reg32 length = kEditLength(app) - 1;
            kEditLength.set(app, length);
            setByte(app, kEditText + length, 0);
        }
        return 0;
    case kKeyLeft:
        focus = int32_t(reg32(focus) - 1);
        if (focus < 0)
            focus = last;
        kFocus.set(app, reg32(focus));
        GAME(at, kPointAtButton, {reg32(focus)});
        return 0;
    case kKeyRight:
        focus = int32_t(reg32(focus) + 1);
        if (last < focus)
            focus = 0;
        kFocus.set(app, reg32(focus));
        GAME(at, kPointAtButton, {reg32(focus)});
        return 0;
    case kKeyDelete:
        GAME(at, kMemset, {kEditText, 0, kEditSize});
        kEditLength.set(app, 0);
        return 0;
    default:
        break;
    }
    if (sdword(app, 0x558b54) <= sdword(app, 0x558b50) && kReplaceOnKey(app) == 0)
        return 0;
    GAME(at, kCharAllowed, {key & 0xff, kEditMode(app)});
    if ((g.eax & 0xff) == 0)
        return 0;
    if (kReplaceOnKey(app) != 0)
    {
        kReplaceOnKey.set(app, 0);
        GAME(at, kMemset, {kEditText, 0, kEditSize});
        kEditLength.set(app, 0);
    }
    const reg32 length = kEditLength(app) + 1;
    setByte(app, kEditText + length - 1, uint8_t(key));
    kEditLength.set(app, length);
    return 0;
}

/* 0x4450c0's: the pass of a dialog to type in: as a question's, and the field. */
Result editPassCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 16;
    kListFocus.set(app, reg32(-1));
    NEEDED(followed, followPointer(g, at));
    NEEDED(box, drawBoxCore(g, entered(at), kRect));
    g.eax = reg32(*box);
    NEEDED(lines, drawLinesCore(g, entered(at), sdword(app, 0x558b1c), kLines(app), kRect));
    g.eax = reg32(*lines);
    NEEDED(field, drawEditFieldCore(g, entered(at), kRect));
    g.eax = reg32(*field);
    NEEDED(buttons, drawButtonsCore(g, entered(at), sdword(app, 0x558b3c)));
    g.eax = reg32(*buttons);
    return 0;
}

/* ---- A list ---- */

/* 0x4452d0's: the list's entries' boxes, from (kListX, kListY) down, 18 pixels
 * each (30 in a race).  Six entries or more get none. */
Result placeEntriesCore(Game& g, reg32 esp, int32_t count, reg32 names)
{
    App* app = g.app;
    const reg32 at = esp - 16 - 8;
    if (!setFont(g, at))
        return {};
    const int32_t x = sdword(app, 0x558c74);
    int32_t y = sdword(app, 0x558c78);
    for (int32_t i = 0; i < count && count < kMaxEntries; ++i)
    {
        const reg32 name = word(app, names + 4 * reg32(i));
        int32_t width = 0;
        if (name != 0)
        {
            GAME(at, kTextWidth, {name, kFont});
            width = int32_t(g.eax);
        }
        setBoxField(app, kEntryBoxes, i, 4, width);
        setBoxField(app, kEntryBoxes, i, 2, x);
        setBoxField(app, kEntryBoxes, i, 0, y);
        setBoxField(app, kEntryBoxes, i, 6, inRace(app) ? 30 : 18);
        y += box(app, kEntryBoxes, i).height;
    }
    return int32_t(g.eax);
}

/* 0x445750's: a list opened: lines, the entries to tick below them, and
 * buttons below those, in a row or, with `column`, one above the other.
 * Nothing without lines, entries, ticks or buttons, with ten buttons or more,
 * or six entries or more. */
Result openListCore(Game& g, reg32 esp, reg32 lineCount, reg32 lines, int32_t entryCount, reg32 names, reg32 ticks,
                    reg32 entryHelp, int32_t buttonCount, reg32 buttons, reg32 buttonHelp, int32_t focus,
                    reg32 column, reg32 callback)
{
    App* app = g.app;
    CPU& cpu = g.cpu;
    const reg32 at = esp - 12 - 0x68;
    kListHelp.set(app, 0);
    kHelpText.set(app, 0);
    kButtonHelp.set(app, 0);
    if (lineCount == 0 || lines == 0 || entryCount == 0 || names == 0 || ticks == 0 || buttonCount == 0 || buttons == 0
        || buttonCount >= kMaxButtons || entryCount >= kMaxEntries)
        return int32_t(g.eax);
    kLineCount.set(app, lineCount);
    kKind.set(app, kList);
    kShown.set(app, 1);
    setByte(app, 0x558c64, uint8_t(entryCount));
    kLines.set(app, lines);
    kListTicks.set(app, ticks);
    kListNames.set(app, names);
    kListHelp.set(app, entryHelp);
    kButtonCount.set(app, reg32(buttonCount));
    kButtonHelp.set(app, buttonHelp);
    kButtons.set(app, buttons);
    kListFocus.set(app, reg32(-1));
    // The button highlighted first: the first for a negative one, the last for one past them.
    if (focus < 0)
        focus = 0;
    else if (buttonCount - 1 < focus)
        focus = buttonCount - 1;
    kFocus.set(app, reg32(focus));

    const reg32 linesRect = at, buttonsRect = at + 0x10;  // [ebp - 0x68], [ebp - 0x58]
    const reg32 listRect = at + 0x20;                     // [ebp - 0x48]
    if (column != 0)
    {
        GAME(at, kLinesBox, {kLineCount(app), kLines(app), 0, 0}, {linesRect});
        GAME(at, kLinesBox, {kButtonCount(app), kButtons(app), 0, 0}, {buttonsRect});
    }
    else
    {
        GAME(at, kLinesBox, {kLineCount(app), kLines(app), 1, 0}, {linesRect});
        GAME(at, kButtonRowBox, {kButtonCount(app), kButtons(app), buttonsRect});
    }
    GAME(at, kLinesBox, {reg32(entryCount), names, 0, 0}, {listRect});

    // The three boxes put together round the lines' middle, as wide as the
    // widest; the entries 18 pixels each and a margin.
    x86::FPU& f = cpu.fpu;
    const Float linesWidth = f.sub(load(app, linesRect + kRight), load(app, linesRect + kLeft));
    const Float linesHeight = f.sub(load(app, linesRect + kBottom), load(app, linesRect + kTop));
    const float linesWidthF = float(linesWidth);
    const Float halfLinesWidth = f.mul(linesWidth, Float(0.5));
    const float linesHeightF = float(linesHeight);
    const Float halfLinesHeight = f.mul(Float(0.5), linesHeight);
    const float listWidthF = float(f.sub(load(app, listRect + kRight), load(app, listRect + kLeft)));
    const float buttonsWidthF = float(f.sub(load(app, buttonsRect + kRight), load(app, buttonsRect + kLeft)));
    const float buttonsHeightF = float(f.sub(load(app, buttonsRect + kBottom), load(app, buttonsRect + kTop)));
    const float middleXF = float(f.add(load(app, linesRect + kLeft), halfLinesWidth));
    const float listHeightF = float(integer(18 * entryCount + 20));
    const float middleYF = float(f.add(load(app, linesRect + kTop), halfLinesHeight));
    float width, height;
    if (column != 0)
    {
        height = float(f.add(f.add(f.add(Float(linesHeightF), Float(buttonsHeightF)), Float(listHeightF)), Float(-15.0)));
        width = above(cpu, Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        if (below(cpu, Float(width), Float(listWidthF)))
            width = listWidthF;
        kButtonsX.set(app, reg32(chop(cpu, Float(middleXF))));
    }
    else
    {
        const Float halfRow = f.mul(f.add(Float(buttonsWidthF), Float(-30.0)), Float(0.5));
        kButtonsX.set(app, reg32(chop(cpu, f.sub(Float(middleXF), halfRow))));
        width = above(cpu, Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        if (below(cpu, Float(width), Float(listWidthF)))
            width = listWidthF;
        height = float(f.add(Float(linesHeightF), Float(listHeightF)));
    }
    const Float halfListWidth = f.mul(Float(listWidthF), Float(0.5));
    const Float middleX = Float(middleXF), middleY = Float(middleYF);
    const Float halfWidth = f.mul(Float(0.5), Float(width)), halfHeight = f.mul(Float(height), Float(0.5));
    setFloat(app, kRect + kTop, float(f.sub(middleY, halfHeight)));
    setFloat(app, kRect + kLeft, float(f.sub(middleX, halfWidth)));
    setFloat(app, kRect + kRight, float(f.add(halfWidth, middleX)));
    setFloat(app, kRect + kBottom, float(f.add(halfHeight, middleY)));
    // The entries: 15 pixels in from the list's own box, 35 below the lines'
    // top and 30 a line.
    const Float listLeft = f.sub(middleX, halfListWidth);
    kListX.set(app, reg32(chop(cpu, f.add(listLeft, Float(15.0)))));
    const Float top = Float(floatAt(app, kRect + kTop));
    kListY.set(app, reg32(chop(cpu, f.add(integer(30 * int32_t(kLineCount(app))), f.add(top, Float(35.0))))));

    NEEDED(placed, placeEntriesCore(g, entered(at), int32_t(byteAt(app, 0x558c64)), kListNames(app)));
    g.eax = reg32(*placed);
    GAME(at, kPlaceButtons, {kButtonCount(app), kButtons(app), column, kRect});
    GAME(at, kPointAtButton, {kFocus(app)});
    kCallback.set(app, callback);
    return int32_t(g.eax);
}

/* 0x445520's: a key for a list: Up and Down move the highlight round its
 * entries, Left and Right round its buttons; Enter ticks the entry
 * highlighted and answers, as Escape does. */
Result listKeyCore(Game& g, reg32 esp, uint16_t key)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    const int32_t entries = byteAt(app, 0x558c64);
    const int32_t buttons = sdword(app, 0x558b30);
    int32_t entry = sdword(app, 0x558c60), focus = sdword(app, 0x558b3c);
    switch (key)
    {
    case kKeyEnter:
        if (entry >= 0 && entry < entries)
        {
            const reg32 tick = word(app, kListTicks(app) + 4 * reg32(entry));
            if (tick != 0)
                setWord(app, tick, 1);
        }
        return kAnswerEnter;
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyUp:
        entry = int32_t(reg32(entry) - 1);
        if (entry < 0)
            entry = entries - 1;
        kListFocus.set(app, reg32(entry));
        {
            g.eax = reg32(entry);
            NEEDED(pointed, pointAtEntryCore(g, entered(at), entry));
            g.eax = reg32(*pointed);
        }
        return 0;
    case kKeyDown:
        entry = int32_t(reg32(entry) + 1);
        if (entries - 1 < entry)
            entry = 0;
        kListFocus.set(app, reg32(entry));
        {
            g.eax = reg32(entry);
            NEEDED(pointed, pointAtEntryCore(g, entered(at), entry));
            g.eax = reg32(*pointed);
        }
        return 0;
    case kKeyLeft:
        focus = int32_t(reg32(focus) - 1);
        if (focus < 0)
            focus = buttons - 1;
        kFocus.set(app, reg32(focus));
        GAME(at, kPointAtButton, {reg32(focus)});
        return 0;
    case kKeyRight:
        focus = int32_t(reg32(focus) + 1);
        if (buttons - 1 < focus)
            focus = 0;
        kFocus.set(app, reg32(focus));
        GAME(at, kPointAtButton, {reg32(focus)});
        return 0;
    default:
        return 0;
    }
}

/* 0x445680's: a list's pass: the button or the entry under the pointer
 * highlighted (the other not), the box, the lines, the entries and the
 * buttons drawn. */
Result listPassCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 24;
    NEEDED(buttonFound, buttonUnderMouseCore(g));
    g.eax = reg32(*buttonFound);
    NEEDED(entryFound, entryUnderMouseCore(g));
    g.eax = reg32(*entryFound);
    const int32_t button = *buttonFound, entry = *entryFound;
    if (button >= 0 && button < sdword(app, 0x558b30))
    {
        if (button != sdword(app, 0x558b3c))
            GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
        kFocus.set(app, reg32(button));
        kListFocus.set(app, reg32(-1));
    }
    else if (entry >= 0 && entry < int32_t(byteAt(app, 0x558c64)))
    {
        if (entry != sdword(app, 0x558c60))
            GAME(at, kPlaySound, {kSoundMove, kVolumeTick});
        kListFocus.set(app, reg32(entry));
        kFocus.set(app, reg32(-1));
    }
    else
    {
        kFocus.set(app, reg32(-1));
        kListFocus.set(app, reg32(-1));
    }
    NEEDED(box, drawBoxCore(g, entered(at), kRect));
    g.eax = reg32(*box);
    NEEDED(lines, drawLinesCore(g, entered(at), sdword(app, 0x558b1c), kLines(app), kRect));
    g.eax = reg32(*lines);
    NEEDED(list, drawListCore(g, entered(at)));
    g.eax = reg32(*list);
    NEEDED(buttons, drawButtonsCore(g, entered(at), sdword(app, 0x558b3c)));
    g.eax = reg32(*buttons);
    return 0;
}

/* ---- The dialog up, whatever its kind ---- */

/* 0x445de0's: the dialogs' shapes found among the front end's. */
Result loadDialogArtCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 4;
    GAME(at, kFindShape, {kPopu});
    kBoxShape.set(app, g.eax);
    GAME(at, kFindShape, {kBpop});
    kButtonShape.set(app, g.eax);
    GAME(at, kFindShape, {kYpop});
    kFocusShape.set(app, g.eax);
    GAME(at, kFindShape, {kGlit});
    kTickedShape.set(app, g.eax);
    GAME(at, kFindShape, {kRlit});
    kUntickedShape.set(app, g.eax);
    if ((kDisplayFlags(app) & kNoTranslucency) == 0)
    {
        GAME(at, kFindShape, {kYel1});
        kYel1Shape.set(app, g.eax);
    }
    return int32_t(g.eax);
}

/* An answer handed to the dialog's callback, with the byte at `keepUp`, 1,
 * that the callback clears to keep the dialog up; what the callback returns
 * is for the menu loop. */
Result handOn(Game& g, reg32 at, reg32 answer, reg32 keepUp)
{
    App* app = g.app;
    reg32 result = 0;
    if (answer == 0)
        return 0;
    if (kCallback(app) != 0)
    {
        GAME(at, kCallback(app), {answer, keepUp});
        result = g.eax;
    }
    if (byteAt(app, keepUp) != 0)
    {
        NEEDED(closed, closeDialogCore(g));
        g.eax = reg32(*closed);
    }
    return int32_t(result);
}

/* 0x445e60's: a key for the dialog up, once it has faded in (at once in a race). */
Result dialogKeyCore(Game& g, reg32 esp, reg32 key)
{
    App* app = g.app;
    const reg32 at = esp - 16 - 4;
    const reg32 keepUp = at;  // [ebp - 4]
    setByte(app, keepUp, 1);
    if (!inRace(app) && int32_t(kFade(app)) < 255)
        return 0;
    const reg32 code = reg32(int32_t(int16_t(key)));
    reg32 answer = 0;
    switch (kKind(app))
    {
    case kQuestion:
    {
        NEEDED(r, questionKeyCore(g, entered(at), uint16_t(code)));
        answer = reg32(*r);
        break;
    }
    case kMessage:
    {
        NEEDED(r, messageKeyCore(g, uint16_t(code)));
        answer = reg32(*r);
        break;
    }
    case kEdit:
    {
        NEEDED(r, editKeyCore(g, entered(at), code));
        answer = reg32(*r);
        break;
    }
    case kList:
    {
        NEEDED(r, listKeyCore(g, entered(at), uint16_t(code)));
        answer = reg32(*r);
        break;
    }
    default:
        break;
    }
    g.eax = answer;
    NEEDED(result, handOn(g, at, answer, keepUp));
    return *result == int32_t(kHandled) ? 0 : *result;
}

/* 0x445f50's: the dialog up, a pass of it: faded in a step, drawn, its answer
 * if the pass made one, and its help box. */
Result dialogPassCore(Game& g, reg32 esp)
{
    App* app = g.app;
    const reg32 at = esp - 24 - 4;
    const reg32 keepUp = at;  // [ebp - 4]
    setByte(app, keepUp, 1);
    kDepthNear.set(app, 0);
    kDepthFar.set(app, kFloatAlmostOne);
    if (int32_t(kFade(app)) < 255)
    {
        kFade.set(app, reg32(std::min(int32_t(kFade(app)) + 20, 255)));
        kDrawColour.set(app, fadeAlpha(app));
    }
    reg32 answer = 0;
    switch (kKind(app))
    {
    case kQuestion:
    {
        NEEDED(r, questionPassCore(g, entered(at)));
        answer = reg32(*r);
        break;
    }
    case kMessage:
    {
        NEEDED(r, messagePassCore(g, entered(at)));
        answer = reg32(*r);
        break;
    }
    case kEdit:
    {
        NEEDED(r, editPassCore(g, entered(at)));
        answer = reg32(*r);
        break;
    }
    case kList:
    {
        NEEDED(r, listPassCore(g, entered(at)));
        answer = reg32(*r);
        break;
    }
    default:
        break;
    }
    g.eax = answer;
    NEEDED(handed, handOn(g, at, answer, keepUp));
    reg32 result = reg32(*handed);
    if (!inRace(app))
        kDrawColour.set(app, kOpaque);
    if (result == kHandled)
        result = 0;
    kDepthNear.set(app, 0);
    kDepthFar.set(app, kFloatAlmostOne);
    NEEDED(helped, drawDialogHelpCore(g, entered(at)));
    g.eax = reg32(*helped);
    kDepthNear.set(app, kFloatAlmostOne);
    kDepthFar.set(app, kFloatTiny);
    return int32_t(result);
}

}  // namespace

/* ---- The adapters ---- */

/* 0x4438b0: whether a dialog is up, in al. */
void dialogShown(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, (e.eax & ~0xffu) | kShown(app));
}

/* 0x4438c0: the kind of dialog up, 0 for none. */
void dialogKind(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, kKind(app));
}

/* 0x4438d0: the dialog's box at eax drawn. */
void drawBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawBoxCore(g, e.esp, e.eax));
}

/* 0x443f40: the lines at edx (eax of them) drawn in the box at ebx. */
void drawLines(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawLinesCore(g, e.esp, int32_t(e.eax), e.edx, e.ebx));
}

/* 0x444000: the buttons drawn, the one edx highlighted. */
void drawButtons(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawButtonsCore(g, e.esp, int32_t(e.edx)));
}

/* 0x444350: the field typed in, in the box at eax. */
void drawEditField(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawEditFieldCore(g, e.esp, e.eax));
}

/* 0x4445d0: the button under the pointer. */
void buttonUnderMouse(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, buttonUnderMouseCore(g));
}

/* 0x444910: the key eax for a question. */
void questionKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, questionKeyCore(g, e.esp, uint16_t(e.eax)));
}

/* 0x4449d0: a question's pass. */
void questionPass(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, questionPassCore(g, e.esp));
}

/* 0x444a60: the message to go on the next pass. */
void endMessage(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, endMessageCore(g));
}

/* 0x444b20: the key eax for a message. */
void messageKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, messageKeyCore(g, uint16_t(e.eax)));
}

/* 0x444b60: a message's pass. */
void messagePass(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, messagePassCore(g, e.esp));
}

/* 0x444f50: the key eax for a line typed in. */
void editKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, editKeyCore(g, e.esp, e.eax));
}

/* 0x4450c0: the pass of a dialog to type in. */
void editPass(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, editPassCore(g, e.esp));
}

/* 0x445150: the list entry highlighted. */
void listFocus(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, kListFocus(app));
}

/* 0x445160: the pointer moved onto the bottom right of the entry eax's box. */
void pointAtEntry(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, pointAtEntryCore(g, e.esp, int32_t(e.eax)));
}

/* 0x4451d0: the entry under the pointer. */
void entryUnderMouse(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, entryUnderMouseCore(g));
}

/* 0x4452d0: eax entries' boxes laid out for the names at edx. */
void placeEntries(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, placeEntriesCore(g, e.esp, int32_t(e.eax), e.edx));
}

/* 0x445380: the list's entries drawn. */
void drawList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawListCore(g, e.esp));
}

/* 0x445520: the key eax for a list. */
void listKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, listKeyCore(g, e.esp, uint16_t(e.eax)));
}

/* 0x445680: a list's pass. */
void listPass(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, listPassCore(g, e.esp));
}

/* 0x445750 (stdcall 0x20): a list opened. */
void openList(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu,
             openListCore(g, e.esp, e.eax, e.edx, int32_t(e.ebx), e.ecx, e.arg(app, 0), e.arg(app, 1),
                          int32_t(e.arg(app, 2)), e.arg(app, 3), e.arg(app, 4), int32_t(e.arg(app, 5)), e.arg(app, 6),
                          e.arg(app, 7)),
             0x20);
}

/* 0x445ac0: the help box over the dialog. */
void drawDialogHelp(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, drawDialogHelpCore(g, e.esp));
}

/* 0x445de0: the dialogs' shapes found. */
void loadDialogArt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, loadDialogArtCore(g, e.esp));
}

/* 0x445e60: the key eax for the dialog up. */
void dialogKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, dialogKeyCore(g, e.esp, e.eax));
}

/* 0x445f10: the dialog closed. */
void closeDialog(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, closeDialogCore(g));
}

/* 0x445f50: a pass of the dialog up. */
void dialogPass(App* app, CPU& cpu)
{
    const Entry e(cpu);
    Game g{app, cpu, e.eax};
    e.finish(cpu, dialogPassCore(g, e.esp));
}

}  // namespace dialogs
}  // namespace port
}  // namespace nfs3hp
