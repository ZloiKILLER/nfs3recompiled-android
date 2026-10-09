#include "native_frontend.h"
#include <algorithm>
#include <iterator>

/* The dialogs over a screen: a question and its buttons, a message that goes
 * away by itself, a line to type in, a list to tick entries of.
 *
 * One dialog is up at a time (kShown), of a kind (kKind), with its lines of
 * text, its buttons and the button highlighted (kFocus).  While it is up the
 * menu loop hands it each pass (sub_445f50) and each key (sub_445e60).  An
 * answer -- Enter, Escape, the message's time run out -- goes to the
 * dialog's callback, with a byte the callback clears to keep the dialog up;
 * what the callback returns goes back to the menu loop.  The box fades in
 * over a dozen passes and the keys wait for it.  The pointer highlights the
 * button it is on, and rests long enough on one with a line of help for the
 * help box to show (sub_445ac0).
 *
 * The layout is worked out as the dialog opens, in floats on the 640x480
 * screen: a box round the lines, one round the buttons, put together and
 * centred (kRect: top, left, bottom, right), the buttons placed below the
 * lines in a row or one above the other (kButtonBoxes).  The arithmetic is
 * the x87's (fe::X87): in a race's pause screens it rounds to a float's
 * precision, and the dialog lands where the game puts it.  There the box and
 * the buttons are quads of the pause screens' art; in the menus they are
 * shapes of the front end's own (sub_445de0). */
namespace nfs3hp
{

// nfs3hp_main.cpp: the box's emblem kept round on a wide screen (tools/apply_widescreen.py).
void dialogEmblem(bool drawing);

namespace
{

using fe::Guest;
using fe::X87;
using x86::Float;

// The dialog up.
constexpr x86::reg32 kShown = 0x558b10;            // byte
constexpr x86::reg32 kKind = 0x558b14;
constexpr std::uint32_t kQuestion = 1;              // sub_4446d0
constexpr std::uint32_t kMessage = 2;               // sub_444a80
constexpr std::uint32_t kEdit = 3;                  // sub_444c20
constexpr std::uint32_t kList = 4;                  // sub_445750
constexpr x86::reg32 kCallback = 0x558b18;          // (answer, &keep up)
constexpr x86::reg32 kLineCount = 0x558b1c;
constexpr x86::reg32 kLines = 0x558b20;             // char*[kLineCount]
constexpr x86::reg32 kButtonCount = 0x558b30;
constexpr x86::reg32 kButtons = 0x558b34;           // char*[kButtonCount]
constexpr x86::reg32 kButtonHelp = 0x558b38;        // char*[kButtonCount], or 0
constexpr x86::reg32 kFocus = 0x558b3c;             // the button highlighted, -1 for none
constexpr x86::reg32 kButtonsX = 0x558b40;          // a row's left, a column's middle
constexpr std::int32_t kMaxButtons = 10;
// A message's.
constexpr x86::reg32 kMessageOpened = 0x558b44;     // the tick it came up at
constexpr x86::reg32 kMessageTime = 0x558b48;       // the ticks it stays up, 0 for as long as it takes
constexpr x86::reg32 kMessageKeys = 0x558b4c;       // byte: the keys answer it
constexpr x86::reg32 kMessageLine = 0x661090;       // sub_444be0's one line
constexpr std::uint32_t kMessageTicks = 0x226;
// The line typed in.
constexpr x86::reg32 kReplaceOnKey = 0x558b2c;      // the first character typed replaces the text given
constexpr x86::reg32 kEditLength = 0x558b50;
constexpr x86::reg32 kEditMax = 0x558b54;           // the most characters
constexpr x86::reg32 kEditText = 0x558b58;
constexpr std::uint32_t kEditSize = 0x100;
constexpr x86::reg32 kEditSource = 0x558c58;        // the text it started from
constexpr x86::reg32 kEditMode = 0x558c5c;          // what may be typed (sub_443810), below 4
constexpr x86::reg32 kEditWidth = 0x661070;         // the field's, in pixels
constexpr x86::reg32 kCursorBlink = 0x558d08;       // 0 to 24: the cursor shows past 12
// A list's.
constexpr x86::reg32 kListFocus = 0x558c60;         // the entry highlighted, -1 for none
constexpr x86::reg32 kListCount = 0x558c64;         // byte
constexpr x86::reg32 kListNames = 0x558c68;         // char*[kListCount]
constexpr x86::reg32 kListTicks = 0x558c6c;         // int*[kListCount]: an entry's tick, Enter sets it
constexpr x86::reg32 kListHelp = 0x558c70;          // char*[kListCount], or 0
constexpr x86::reg32 kListX = 0x558c74;
constexpr x86::reg32 kListY = 0x558c78;
constexpr std::int32_t kMaxEntries = 6;

// Where it is drawn: the box (floats), the buttons' and the entries' boxes
// (8 bytes each, int16), and how far it has faded in.
constexpr x86::reg32 kRect = 0x661050;
constexpr x86::reg32 kTop = 0x0;
constexpr x86::reg32 kLeft = 0x4;
constexpr x86::reg32 kBottom = 0x8;
constexpr x86::reg32 kRight = 0xc;
constexpr x86::reg32 kButtonBoxes = 0x661000;
constexpr x86::reg32 kEntryBoxes = 0x660fd0;
constexpr x86::reg32 kBoxY = 0;
constexpr x86::reg32 kBoxX = 2;
constexpr x86::reg32 kBoxWidth = 4;
constexpr x86::reg32 kBoxHeight = 6;
constexpr x86::reg32 kBoxSize = 8;
constexpr x86::reg32 kFade = 0x661064;              // 0 to 255

// Its art: the front end's shapes, by name (sub_4d5db0), and in a race's
// pause screens parts of their art, a texture.
constexpr x86::reg32 kBoxShape = 0x66108c;          // "popu"
constexpr x86::reg32 kButtonShape = 0x661068;       // "bpop"
constexpr x86::reg32 kFocusShape = 0x661060;        // "ypop"
constexpr x86::reg32 kTickedShape = 0x558b24;       // "glit", a green light
constexpr x86::reg32 kUntickedShape = 0x558b28;     // "rlit", a red one
constexpr x86::reg32 kYel1Shape = 0x66106c;         // "yel1", not when translucency is off
constexpr x86::reg32 kPauseArt = 0x749a38;

// The help box over a dialog (sub_445ac0), for a button or an entry.
constexpr x86::reg32 kHelpOver = 0x558afc;          // 0 a button, 1 an entry, -1 neither
constexpr x86::reg32 kHelpLastX = 0x558b00;         // where the pointer was
constexpr x86::reg32 kHelpLastY = 0x558b04;
constexpr x86::reg32 kHelpFor = 0x558b08;           // the button or entry, -1 for none
constexpr x86::reg32 kHelpText = 0x558b0c;
constexpr x86::reg32 kHelpHeight = 0x661074;
constexpr x86::reg32 kHelpY = 0x661078;
constexpr x86::reg32 kHelpX = 0x66107c;
constexpr x86::reg32 kHelpWidth = 0x661080;
constexpr x86::reg32 kHelpDue = 0x661084;           // the tick it shows at
constexpr x86::reg32 kHelpFade = 0x661088;          // 0 to 255
constexpr std::uint32_t kHelpWrap = 0x14a;
constexpr std::uint32_t kHelpBoxColour = 0x4b6cad;
constexpr std::uint32_t kHelpTextColour = 0x488cfd;
constexpr std::uint32_t kHelpDepth = 0xffff9c;      // -100, with the alpha in the top byte
constexpr x86::reg32 kHelpAlpha = 0x559450;         // float, 0.9 while a dialog's help may show
constexpr std::uint32_t kFloatOne = 0x3f800000;
constexpr std::uint32_t kFloatPoint9 = 0x3f666666;

// The menus'.
constexpr x86::reg32 kInRace = 0x559234;            // the menus are a race's pause screens
constexpr x86::reg32 kMouseX = 0x55e5e4;
constexpr x86::reg32 kMouseY = 0x55e5e8;
constexpr x86::reg32 kTicks = 0x79c1a4;             // the game's clock
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kNoTranslucency = 0x02;      // no fades
constexpr x86::reg32 kDisplayChoice = 0x6fbb4c;
constexpr x86::reg32 kDrawColour = 0x563aa4;        // what 2D is drawn with, alpha on top
constexpr std::uint32_t kOpaque = 0xff000000;
// The depth range 2D is drawn in.
constexpr x86::reg32 kDepthNear = 0x563aa8;
constexpr x86::reg32 kDepthFar = 0x563aac;
constexpr std::uint32_t kFloatAlmostOne = 0x3f7fff00;
constexpr std::uint32_t kFloatTiny = 0x37800080;

// Text.
constexpr std::uint32_t kFont = 0x12;
constexpr std::uint32_t kAlignLeft = 0;             // sub_452190's
constexpr std::uint32_t kAlignCentre = 2;
constexpr std::uint32_t kTextColour = 0x649dfd;
constexpr std::uint32_t kFocusColour = 0xffe440;
constexpr std::uint32_t kEdgeColour = 0x727eb3;     // a button's edges
constexpr std::uint32_t kFocusEdgeColour = 0xffd84b;
constexpr std::uint32_t kWhite = 0xffffff;

// Keys, as sub_451960 gives them, and the answers they make.
constexpr std::uint16_t kKeyBackspace = 0x08;
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr std::uint16_t kKeyEscape = 0x1b;
constexpr std::uint16_t kKeySpace = 0x20;
constexpr std::uint16_t kKeyUp = 0x4800;
constexpr std::uint16_t kKeyLeft = 0x4b00;
constexpr std::uint16_t kKeyRight = 0x4d00;
constexpr std::uint16_t kKeyDown = 0x5000;
constexpr std::uint16_t kKeyDelete = 0x5300;
constexpr x86::reg32 kAnswerEnter = fe::kDialogEnter;
constexpr x86::reg32 kAnswerSpace = 2;              // a message's
constexpr x86::reg32 kAnswerEscape = fe::kDialogEscape;
constexpr x86::reg32 kAnswerTimedOut = 4;           // a message's time ran out

// Sounds, sub_4181d0(sound, volume).
constexpr x86::reg32 kPlaySound = 0x4181d0;
constexpr std::uint32_t kSoundMove = 0;
constexpr std::uint32_t kVolumeTick = 0x2d;

// The game's.
constexpr x86::reg32 kText = 0x4d1850;
constexpr x86::reg32 kSetFont = 0x452100;
constexpr x86::reg32 kDrawText = 0x452190;          // (text, x, y, font, align, colour)
constexpr x86::reg32 kTextBox = 0x451ff0;           // a text's size, wrapped: (text, font, wrap, &w, &h, 0)
constexpr x86::reg32 kDrawTextWrapped = 0x451fc0;
constexpr x86::reg32 kFillBox = 0x4d8550;
constexpr x86::reg32 kBoxBorder = 0x4d8930;
constexpr x86::reg32 kShadedBox = 0x4d8860;         // (x, y, w, h, four corners' colours)
constexpr x86::reg32 kFieldBox = 0x4d8ac0;          // the same, for a field to type in
constexpr x86::reg32 kFillRect = 0x4d8910;          // (x, y, w, h, colour)
constexpr x86::reg32 kDrawQuad = 0x4db000;          // a textured quad: four corners, their u and v, texture, colour
constexpr x86::reg32 kDrawShape = 0x4d7160;         // (shape, x, y, colour)
constexpr x86::reg32 kShapeSize = 0x4d66f0;         // (shape, &w, &h): int16 each
constexpr x86::reg32 kFindShape = 0x4d5db0;         // by its name
constexpr x86::reg32 kSetClip = 0x4bebe0;           // (x, y, w, h)
constexpr x86::reg32 kMemset = 0x4e0640;
constexpr x86::reg32 kStrncpy = 0x4e0e30;
constexpr x86::reg32 kSetMousePosition = 0x49bfc0;
constexpr x86::reg32 kCharAllowed = 0x443810;       // (character, mode)
constexpr x86::reg32 kTextAllowed = 0x443860;       // (text, mode)

// This file's own, called where they are so each runs as itself.
constexpr x86::reg32 kDrawBox = 0x4438d0;
constexpr x86::reg32 kLinesBox = 0x443a90;
constexpr x86::reg32 kButtonRowBox = 0x443c00;
constexpr x86::reg32 kPlaceButtons = 0x443d20;
constexpr x86::reg32 kDrawLines = 0x443f40;
constexpr x86::reg32 kDrawButtons = 0x444000;
constexpr x86::reg32 kDrawEditField = 0x444350;
constexpr x86::reg32 kPointAtButton = 0x444570;
constexpr x86::reg32 kButtonUnderMouse = 0x4445d0;
constexpr x86::reg32 kQuestionKey = 0x444910;
constexpr x86::reg32 kQuestionPass = 0x4449d0;
constexpr x86::reg32 kOpenMessage = 0x444a80;
constexpr x86::reg32 kMessageKey = 0x444b20;
constexpr x86::reg32 kMessagePass = 0x444b60;
constexpr x86::reg32 kEditKey = 0x444f50;
constexpr x86::reg32 kEditPass = 0x4450c0;
constexpr x86::reg32 kPointAtEntry = 0x445160;
constexpr x86::reg32 kEntryUnderMouse = 0x4451d0;
constexpr x86::reg32 kPlaceEntries = 0x4452d0;
constexpr x86::reg32 kDrawList = 0x445380;
constexpr x86::reg32 kListKey = 0x445520;
constexpr x86::reg32 kListPass = 0x445680;
constexpr x86::reg32 kDrawDialogHelp = 0x445ac0;
constexpr x86::reg32 kFadedIn = 0x445f00;
constexpr x86::reg32 kClose = 0x445f10;

const fe::GuestString kWidest{ 0x538b2c, "W" };
const fe::GuestString kCursor{ 0x538b04, "_" };
const fe::GuestString kPopu{ 0x538b84, "popu" };
const fe::GuestString kBpop{ 0x538b8c, "bpop" };
const fe::GuestString kYpop{ 0x538b94, "ypop" };
const fe::GuestString kGlit{ 0x538b9c, "glit" };
const fe::GuestString kRlit{ 0x538ba4, "rlit" };
const fe::GuestString kYel1{ 0x538bac, "yel1" };

x86::reg32 reg(std::int32_t value)
{
    return x86::reg32(value);
}

std::int32_t sdword(const Guest& guest, x86::reg32 address)
{
    return std::int32_t(guest.dword(address));
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

std::int32_t textWidth(Guest& guest, x86::reg32 text)
{
    return fe::textWidth(guest, text, kFont);
}

void setFont(Guest& guest)
{
    guest.call(kSetFont, fe::args({ kFont }));
}

x86::reg32 fadeAlpha(const Guest& guest)
{
    return guest.dword(kFade) << 24;
}

/* A button's box or a list entry's. */
struct Box
{
    std::int32_t y, x, width, height;
};

x86::reg32 boxAt(x86::reg32 boxes, std::int32_t index)
{
    return boxes + x86::reg32(index) * kBoxSize;
}

Box box(const Guest& guest, x86::reg32 boxes, std::int32_t index)
{
    const x86::reg32 at = boxAt(boxes, index);
    return { std::int16_t(guest.word(at + kBoxY)), std::int16_t(guest.word(at + kBoxX)),
             std::int16_t(guest.word(at + kBoxWidth)), std::int16_t(guest.word(at + kBoxHeight)) };
}

void setBoxField(Guest& guest, x86::reg32 boxes, std::int32_t index, x86::reg32 field, std::int32_t value)
{
    guest.setWord(boxAt(boxes, index) + field, std::uint16_t(value));
}

/* A part of the pause screens' art, by its corners' texture coordinates
 * (float bits). */
struct ArtPart
{
    x86::reg32 left, top, right, bottom;
};
const ArtPart kBoxArt{ 0x3f000000, 0x3e800000, 0x3f7f0000, 0x3efe0000 };     // 0.5, 0.25 to 0.996, 0.496
const ArtPart kButtonArt{ 0x3ef80000, 0x3f140000, 0x3f0c0000, 0x3f270000 };  // 0.484, 0.578 to 0.547, 0.652
const ArtPart kFocusArt{ 0x3f0d0000, 0x3f140000, 0x3f1d0000, 0x3f270000 };   // 0.551, 0.578 to 0.613, 0.652

/* sub_4db000: a part of the pause screens' art, (left, top) to (right,
 * bottom) on the screen. */
x86::reg32 drawPauseArt(Guest& guest, std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom,
                        const ArtPart& part, x86::reg32 colour)
{
    return guest.call(kDrawQuad, fe::args({ reg(left), reg(top), reg(right), reg(top) },
                                          { reg(right), reg(bottom), reg(left), reg(bottom), part.left, part.top,
                                            part.right, part.top, part.right, part.bottom, part.left, part.bottom,
                                            guest.dword(kPauseArt), colour }));
}

/* sub_4d8860: a box shaded between its corners' colours; a line, one pixel high. */
void drawShaded(Guest& guest, std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height,
                std::initializer_list<x86::reg32> corners)
{
    const x86::reg32* c = corners.begin();
    guest.call(kShadedBox, fe::args({ reg(x), reg(y), reg(width), reg(height) }, { c[0], c[1], c[2], c[3] }));
}

void playSound(Guest& guest, std::uint32_t sound, std::uint32_t volume)
{
    guest.call(kPlaySound, fe::args({ sound, volume }));
}

/* ---------------------------------------------------------------------------
 * The state
 * ------------------------------------------------------------------------- */

/* sub_4438b0: whether a dialog is up, in al. */
x86::reg32 dialogShown(Guest& guest)
{
    return (guest.eax() & ~0xffu) | guest.byte(kShown);
}

/* sub_4438c0: the kind of dialog up, 0 for none. */
x86::reg32 dialogKind(Guest& guest)
{
    return guest.dword(kKind);
}

/* sub_4446c0: the button highlighted. */
x86::reg32 focusedButton(Guest& guest)
{
    return guest.dword(kFocus);
}

/* sub_444c10: the line typed in. */
x86::reg32 editText()
{
    return kEditText;
}

/* sub_445150: the list entry highlighted. */
x86::reg32 listFocus(Guest& guest)
{
    return guest.dword(kListFocus);
}

/* sub_445f00: the dialog shown at once, faded in. */
x86::reg32 fadedIn(Guest& guest)
{
    guest.setDword(kFade, 255);
    return guest.eax();
}

/* sub_445f10: the dialog closed. */
x86::reg32 closeDialog(Guest& guest)
{
    guest.setDword(kCallback, 0);
    guest.setByte(kShown, 0);
    guest.setDword(kKind, 0);
    guest.setDword(kHelpText, 0);
    return guest.eax() & ~0xff00u;  // ah cleared on the way, as the generated code leaves it
}

/* ---------------------------------------------------------------------------
 * The layout
 * ------------------------------------------------------------------------- */

/* sub_443a90: the box round `count` lines of text, centred on the screen, in
 * floats at `rect`: as wide as the widest line with a margin (190 pixels at
 * least, but in a race), 30 pixels a line and a margin, 45 more for a row of
 * buttons below and 45 for a field to type in.  The dialog starts to fade
 * in. */
x86::reg32 linesBox(Guest& guest, std::int32_t count, x86::reg32 lines, x86::reg32 rowBelow, x86::reg32 fieldBelow,
                    x86::reg32 rect)
{
    setFont(guest);
    std::int32_t width = 0;
    for (std::int32_t i = 0; i < count; ++i)
        width = std::max(width, textWidth(guest, guest.dword(lines + 4 * x86::reg32(i))));
    X87 x87(guest);
    if (inRace(guest))
        width = x87.truncated(x87.add(X87::integer(width), Float(63.5)));
    else
        width += 51;
    width += 30;
    std::int32_t height = 30 * count + 30;
    if (rowBelow != 0)
        height += 45;
    if (fieldBelow != 0)
        height += 45;
    if (width >= 640)
        width = 639;
    if (height >= 480)
        height = 479;
    if (!inRace(guest) && width < 190)
        width = 190;
    const std::int32_t x = 320 - width / 2, y = 240 - height / 2;
    guest.setFloat(rect + kTop, float(X87::integer(y)));
    guest.setFloat(rect + kLeft, float(X87::integer(x)));
    guest.setFloat(rect + kRight, float(X87::integer(x + width)));
    guest.setFloat(rect + kBottom, float(X87::integer(y + height)));
    guest.setDword(kFade, 0);
    if ((guest.byte(kDisplayFlags) & kNoTranslucency) != 0)
        guest.call(kFadedIn);
    // The registers as the generated function leaves them: a message hands
    // them on (sub_444a80), and the screens that open one go on with them --
    // some return its eax as their own result.
    guest.leaveEbx(rect);
    guest.leaveEcx(0);
    guest.leaveEdx(height < 0 ? ~0u : 0);
    return (x86::reg32(y + height) & ~0xff00u) | (x86::reg32(guest.byte(kDisplayFlags)) << 8);
}

/* sub_443c00: the box round a row of buttons, centred on the screen, in
 * floats at `rect`: the buttons' texts 15 pixels apart and 60 high, 190
 * pixels wide at least but in a race.  The dialog starts to fade in. */
x86::reg32 buttonRowBox(Guest& guest, std::int32_t count, x86::reg32 buttons, x86::reg32 rect)
{
    setFont(guest);
    std::int32_t width = 15;
    for (std::int32_t i = 0; i < count; ++i)
    {
        const x86::reg32 text = guest.dword(buttons + 4 * x86::reg32(i));
        if (text != 0)
            width += textWidth(guest, text) + 15;
    }
    const std::int32_t height = 60;
    if (width >= 640)
        width = 639;
    if (!inRace(guest) && width < 190)
        width = 190;
    const std::int32_t x = 320 - width / 2, y = 240 - height / 2;
    guest.setFloat(rect + kLeft, float(X87::integer(x)));
    guest.setFloat(rect + kRight, float(X87::integer(x + width)));
    guest.setFloat(rect + kTop, float(X87::integer(y)));
    guest.setFloat(rect + kBottom, float(X87::integer(y + height)));
    guest.setDword(kFade, (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? 255 : 0);
    return guest.eax();
}

/* sub_443d20: the buttons' boxes (kButtonBoxes) in the dialog's box at
 * `rect`: in a column, up from its bottom 30 pixels a button, centred on
 * kButtonsX and then lined up on the leftmost; or in a row 45 pixels above
 * its bottom, spread evenly across it from kButtonsX.  Ten buttons or more
 * get no boxes. */
x86::reg32 placeButtons(Guest& guest, std::int32_t count, x86::reg32 buttons, x86::reg32 column, x86::reg32 rect)
{
    setFont(guest);
    X87 x87(guest);
    const std::int32_t middle = sdword(guest, kButtonsX);
    auto text = [&](std::int32_t i) { return guest.dword(buttons + 4 * x86::reg32(i)); };
    if (column != 0)
    {
        std::int32_t y = x87.truncated(x87.add(x87.load(rect + kBottom), Float(-15.0)));
        std::int32_t left = 640;
        for (std::int32_t i = count - 1; i >= 0; --i)
        {
            if (count >= kMaxButtons || text(i) == 0)
                continue;
            y -= 30;
            const std::int32_t width = textWidth(guest, text(i)) + 5;
            const std::int32_t x = middle - width / 2 + 8;
            setBoxField(guest, kButtonBoxes, i, kBoxX, x);
            left = std::min(left, std::int32_t(std::int16_t(x)));
            setBoxField(guest, kButtonBoxes, i, kBoxWidth, width);
            setBoxField(guest, kButtonBoxes, i, kBoxHeight, inRace(guest) ? 30 : 17);
            setBoxField(guest, kButtonBoxes, i, kBoxY, y);
        }
        for (std::int32_t i = count - 1; i >= 0; --i)
        {
            if (count < kMaxButtons && text(i) != 0)
                setBoxField(guest, kButtonBoxes, i, kBoxX, left);
        }
        return guest.eax();
    }

    const std::int32_t spacing = x87.truncated(x87.sub(x87.load(rect + kRight), x87.load(rect + kLeft))) / (count + 1);
    const std::int32_t y = x87.truncated(x87.add(x87.add(x87.load(rect + kBottom), Float(-15.0)), Float(-30.0)));
    for (std::int32_t i = 0; i < count; ++i)
    {
        if (count >= kMaxButtons || text(i) == 0)
            continue;
        setBoxField(guest, kButtonBoxes, i, kBoxWidth, textWidth(guest, text(i)));
        const std::int32_t at = x87.truncated(x87.load(rect + kLeft)) + spacing + spacing * i;
        setBoxField(guest, kButtonBoxes, i, kBoxHeight, 30);
        setBoxField(guest, kButtonBoxes, i, kBoxX, at - box(guest, kButtonBoxes, i).width / 2 + 8);
        setBoxField(guest, kButtonBoxes, i, kBoxY, y);
    }
    if (guest.dword(kDisplayChoice) == 3 && count == 2)
    {
        // Two buttons five pixels further apart each.
        setBoxField(guest, kButtonBoxes, 1, kBoxX, box(guest, kButtonBoxes, 1).x + 5);
        setBoxField(guest, kButtonBoxes, 0, kBoxX, box(guest, kButtonBoxes, 0).x - 5);
    }
    return guest.eax();
}

/* The dialog's box, kRect, round its middle. */
void setDialogRect(Guest& guest, X87& x87, const Float& middleX, const Float& middleY, const Float& halfWidth,
                   const Float& halfHeight)
{
    guest.setFloat(kRect + kTop, float(x87.sub(middleY, halfHeight)));
    guest.setFloat(kRect + kLeft, float(x87.sub(middleX, halfWidth)));
    guest.setFloat(kRect + kRight, float(x87.add(halfWidth, middleX)));
    guest.setFloat(kRect + kBottom, float(x87.add(halfHeight, middleY)));
}

/* The button highlighted first: the first for a negative one, the last
 * for one past them. */
void setFirstFocus(Guest& guest, std::int32_t focus, std::int32_t buttonCount)
{
    if (focus < 0)
        focus = 0;
    else if (buttonCount - 1 < focus)
        focus = buttonCount - 1;
    guest.setDword(kFocus, reg(focus));
}

/* ---------------------------------------------------------------------------
 * Drawing
 * ------------------------------------------------------------------------- */

/* sub_4438d0: the dialog's box at `rect`: a frame shaded from corner to
 * corner, fading in with the dialog, and the box's art 14 pixels out to the
 * left and 7 up -- a quad of the pause screens' art in a race. */
x86::reg32 drawBox(Guest& guest, x86::reg32 rect)
{
    X87 x87(guest);
    const std::int32_t x = x87.truncated(x87.load(rect + kLeft));
    const std::int32_t y = x87.truncated(x87.load(rect + kTop));
    const std::int32_t width = x87.truncated(x87.sub(x87.load(rect + kRight), x87.load(rect + kLeft)));
    const std::int32_t height = x87.truncated(x87.sub(x87.load(rect + kBottom), x87.load(rect + kTop)));
    // The corners' alphas, faded in -- one of them never reaches the end of
    // its fade, which jumps from 16 to 160 as the fade completes.
    const std::int32_t fade = sdword(guest, kFade);
    x86::reg32 corners[4] = { 0xa0000060, 0xa0000050, 0xd0000020, 0xd0000010 };
    if (fade < 255)
    {
        const x86::reg32 f = x86::reg32(fade);
        corners[0] = (x86::reg32(std::int32_t(f * 160) / 255) << 24) + 0x60;
        corners[1] = (x86::reg32(std::int32_t(f * 16) / 255) << 24) + 0x50;
        const x86::reg32 alpha = x86::reg32(std::int32_t(f * 208) / 255) << 24;
        corners[2] = alpha + 0x20;
        corners[3] = alpha + 0x10;
    }
    drawShaded(guest, x, y, width, height, { corners[0], corners[1], corners[2], corners[3] });
    const x86::reg32 colour = fadeAlpha(guest) + kWhite;
    if (inRace(guest))
    {
        const std::int32_t bottom = x87.truncated(x87.add(X87::integer(y - 7), Float(63.0)));
        const std::int32_t right = x87.truncated(x87.add(X87::integer(x - 14), Float(127.0)));
        // The emblem on the art narrowed for a wide screen, for this one draw.
        struct Emblem
        {
            Emblem() { dialogEmblem(true); }
            ~Emblem() { dialogEmblem(false); }
        } emblem;
        return drawPauseArt(guest, x - 14, y - 7, right, bottom, kBoxArt, colour);
    }
    return guest.call(kDrawShape, fe::args({ guest.dword(kBoxShape), reg(x - 14), reg(y - 7), colour }));
}

/* sub_443f40: the dialog's lines, centred, from 19 pixels below the top of
 * its box at `rect`, 30 pixels apart. */
x86::reg32 drawLines(Guest& guest, std::int32_t count, x86::reg32 lines, x86::reg32 rect)
{
    X87 x87(guest);
    const Float halfWidth = x87.mul(x87.sub(x87.load(rect + kRight), x87.load(rect + kLeft)), Float(0.5));
    const std::int32_t top = x87.truncated(x87.load(rect + kTop)) + 19;
    const std::int32_t middle = x87.truncated(x87.add(halfWidth, x87.load(rect + kLeft)));
    setFont(guest);
    for (std::int32_t i = 0; i < count; ++i)
    {
        const x86::reg32 line = guest.dword(lines + 4 * x86::reg32(i));
        if (line == 0)
            continue;
        std::int32_t x = middle + 25;
        x86::reg32 colour = kTextColour;
        if (inRace(guest))
        {
            x = x87.truncated(x87.add(X87::integer(middle), Float(31.75)));
            colour = kOpaque | kTextColour;
        }
        guest.call(kDrawText, fe::args({ line, reg(x), reg(top + 30 * i), kFont }, { kAlignCentre, colour }));
    }
    return guest.eax();
}

/* sub_444000: the dialog's buttons in their boxes, the one highlighted in
 * yellow: the button's art 17 pixels to the left of its text, and a line
 * along its top and its bottom. */
x86::reg32 drawButtons(Guest& guest, std::int32_t focus)
{
    setFont(guest);
    X87 x87(guest);
    for (std::int32_t i = 0; i < sdword(guest, kButtonCount); ++i)
    {
        const x86::reg32 text = guest.dword(guest.dword(kButtons) + 4 * x86::reg32(i));
        if (text == 0)
            continue;
        const Box b = box(guest, kButtonBoxes, i);
        const bool focused = i == focus;
        const x86::reg32 alpha = fadeAlpha(guest);
        const x86::reg32 white = alpha + kWhite;
        const x86::reg32 solid = focused ? kFocusEdgeColour : kEdgeColour;
        const x86::reg32 edge = alpha + solid;
        const std::int32_t artX = b.x - 17, artY = b.y + 1;
        if (inRace(guest))
        {
            const std::int32_t artBottom = x87.truncated(x87.add(X87::integer(artY), Float(19.0)));
            const std::int32_t artRight = x87.truncated(x87.add(X87::integer(artX), Float(16.0)));
            drawPauseArt(guest, artX, artY, artRight, artBottom, focused ? kFocusArt : kButtonArt, white);
            drawShaded(guest, b.x - 3, b.y + 1, b.width + 8, 1, { edge, solid, solid, edge });
            const std::int32_t bottom = x87.truncated(x87.add(X87::integer(b.y), Float(19.0)));
            drawShaded(guest, b.x - 7, bottom, b.width + 8, 1, { edge, solid, solid, edge });
        }
        else
        {
            guest.call(kDrawShape, fe::args({ guest.dword(focused ? kFocusShape : kButtonShape), reg(artX), reg(artY),
                                              white }));
            drawShaded(guest, b.x - 3, b.y + 1, b.width + 8, 1, { edge, solid, solid, edge });
            drawShaded(guest, b.x - 3, b.y + 16, b.width + 5, 1, { edge, solid, solid, edge });
        }
        guest.call(kDrawText, fe::args({ text, reg(b.x), reg(b.y), kFont },
                                       { kAlignLeft, focused ? kFocusColour : kTextColour }));
    }
    return guest.eax();
}

/* sub_444350: the line typed in, centred 90 pixels above the bottom of the
 * dialog's box at `rect`, with a blinking cursor after it: in the menus on a
 * field of its own (once the dialog is faded in), its end shown when it is
 * too long for the field. */
x86::reg32 drawEditField(Guest& guest, x86::reg32 rect)
{
    X87 x87(guest);
    const std::int32_t width = textWidth(guest, kEditText);
    const std::int32_t middle = x87.truncated(x87.add(x87.load(rect + kLeft), x87.load(rect + kRight))) / 2;
    const std::int32_t y = x87.truncated(x87.add(x87.add(x87.load(rect + kBottom), Float(-30.0)), Float(-60.0)));
    std::int32_t x = middle - width / 2 - 3;
    if (!inRace(guest))
    {
        const std::int32_t span = x87.truncated(x87.sub(x87.load(rect + kRight), x87.load(rect + kLeft)));
        const std::int32_t fieldWidth = sdword(guest, kEditWidth);
        const std::int32_t fieldX = x87.truncated(x87.load(rect + kLeft)) + (span - fieldWidth) / 2;
        const std::int32_t fade = sdword(guest, kFade);
        const x86::reg32 colour = (x86::reg32(std::int32_t(x86::reg32(fade) * 60) / 255) << 24) + 0x20;
        if (fade >= 255)
            guest.call(kFieldBox, fe::args({ reg(fieldX), reg(y - 1), reg(fieldWidth), 20 }, { colour, colour, colour, colour }));
    }
    setFont(guest);
    const std::int32_t shownWidth = textWidth(guest, kEditText);
    const std::int32_t fieldWidth = sdword(guest, kEditWidth);
    if (shownWidth > fieldWidth)
    {
        // Its end in the field, what goes past the field's left edge clipped.
        const std::int32_t fieldMiddle = x87.truncated(x87.add(x87.load(rect + kLeft), x87.load(rect + kRight))) / 2;
        const std::int32_t clip = fieldMiddle - fieldWidth / 2;
        x = fieldWidth / 2 + fieldMiddle - shownWidth - 7;
        if (!inRace(guest))
            guest.call(kSetClip, fe::args({ reg(clip), 0, reg(640 - clip), 480 }));
    }
    guest.call(kDrawText, fe::args({ kEditText, reg(x), reg(y), kFont }, { kAlignLeft, kFocusColour }));
    const std::int32_t blink = std::int32_t(guest.dword(kCursorBlink) + 1) % 25;
    guest.setDword(kCursorBlink, reg(blink));
    if (blink > 12)
    {
        const std::int32_t cursorX = width + x;
        if (inRace(guest))
            guest.call(kDrawText, fe::args({ kCursor.address, reg(cursorX), reg(y), kFont }, { kAlignLeft, kFocusColour }));
        else
        {
            const x86::reg32 colour = fadeAlpha(guest) + kFocusColour;
            drawShaded(guest, cursorX, y + 13, 7, 1, { colour, colour, colour, colour });
        }
    }
    if (!inRace(guest))
        guest.call(kSetClip, fe::args({ 0, 0, 640, 480 }));
    return guest.eax();
}

/* sub_445380: the list's entries, each with its light: green ticked, red
 * not -- in a race a square of colour -- and the name after it, yellow the
 * one highlighted. */
x86::reg32 drawList(Guest& guest)
{
    // The light's size, as the art last said: 10 by 10 until it does.
    const x86::reg32 lightWidth = guest.frame() + 0x10, lightHeight = guest.frame() + 0xc;  // [ebp - 4], [ebp - 8]
    guest.setWord(lightWidth, 10);
    guest.setWord(lightHeight, 10);
    setFont(guest);
    for (std::int32_t i = 0; i < std::int32_t(guest.byte(kListCount)); ++i)
    {
        const x86::reg32 name = guest.dword(guest.dword(kListNames) + 4 * x86::reg32(i));
        if (name == 0)
            continue;
        x86::reg32 colour = kFocusColour;
        if (i != sdword(guest, kListFocus))
            colour = inRace(guest) ? kOpaque | kTextColour : kTextColour;
        const Box b = box(guest, kEntryBoxes, i);
        const x86::reg32 tick = guest.dword(guest.dword(kListTicks) + 4 * x86::reg32(i));
        if (inRace(guest))
        {
            guest.call(kFillRect, fe::args({ reg(b.x), reg(b.y), reg(std::int16_t(guest.word(lightWidth))),
                                             reg(std::int16_t(guest.word(lightHeight))) },
                                           { tick != 0 && guest.dword(tick) != 0 ? 0x8000u : 0x800000u }));
        }
        else
        {
            const x86::reg32 white = fadeAlpha(guest) + kWhite;
            if (tick != 0 && guest.dword(tick) != 0)
                guest.call(kDrawShape, fe::args({ guest.dword(kTickedShape), reg(b.x - 1), reg(b.y + 3), white }));
            else
                guest.call(kDrawShape, fe::args({ guest.dword(kUntickedShape), reg(b.x), reg(tick != 0 ? b.y + 2 : b.y),
                                                  white }));
            guest.call(kShapeSize, fe::args({ guest.dword(kTickedShape), lightWidth, lightHeight, colour }));
        }
        guest.call(kDrawText, fe::args({ name, reg(b.x + std::int16_t(guest.word(lightWidth))), reg(b.y), kFont },
                                       { kAlignLeft, colour }));
    }
    return guest.eax();
}

/* sub_445ac0: the help box over a dialog, for the button or list entry the
 * pointer has rested on for 100 ticks: its line of help, beside the pointer
 * (kept on the 640x480 screen), wrapped at 330 pixels, fading in and out. */
x86::reg32 drawDialogHelp(Guest& guest)
{
    if (guest.dword(kButtonHelp) == 0 && guest.dword(kListHelp) == 0)
        return guest.eax();
    std::int32_t under = std::int32_t(guest.call(kButtonUnderMouse));
    std::int32_t over = 0;
    if (under == -1)
    {
        under = std::int32_t(guest.call(kEntryUnderMouse));
        over = under != -1 ? 1 : -1;
    }
    guest.setDword(kHelpOver, reg(over));
    if (under == -1 || under != sdword(guest, kHelpFor) || over == -1)
        guest.setDword(kHelpFor, reg(-1));
    guest.setDword(kHelpAlpha, kFloatPoint9);
    if (sdword(guest, kHelpFor) == -1)
    {
        if (guest.dword(kHelpLastX) != guest.dword(kMouseX) || guest.dword(kHelpLastY) != guest.dword(kMouseY))
        {
            guest.setDword(kHelpLastX, guest.dword(kMouseX));
            guest.setDword(kHelpLastY, guest.dword(kMouseY));
            guest.setDword(kHelpDue, guest.dword(kTicks) + 100);
        }
        else if (sdword(guest, kTicks) > sdword(guest, kHelpDue) && under != -1)
        {
            guest.setDword(kHelpFor, reg(under));
            guest.setDword(kHelpFade, 0);
            const x86::reg32 help = over == 0 ? guest.dword(kButtonHelp) : guest.dword(kListHelp);
            guest.setDword(kHelpText, 0);
            if (help != 0)
                guest.setDword(kHelpText, guest.dword(help + 4 * x86::reg32(under)));
            // The box: the text's size, wrapped, and a margin.
            const x86::reg32 width = guest.frame(), height = guest.frame() + 4;  // [ebp - 8], [ebp - 4]
            guest.call(kTextBox, fe::args({ guest.dword(kHelpText), kFont, kHelpWrap, width }, { height, 0 }));
            const std::int32_t w = sdword(guest, width) + 7, h = sdword(guest, height) + 3;
            guest.setDword(kHelpX, guest.dword(kMouseX));
            guest.setDword(kHelpWidth, reg(w));
            guest.setDword(kHelpHeight, reg(h));
            if (sdword(guest, kMouseX) + w > 630)
                guest.setDword(kHelpX, reg(630 - w));
            guest.setDword(kHelpY, guest.dword(kMouseY) + 22);
            if (sdword(guest, kMouseY) + 22 + h > 460)
                guest.setDword(kHelpY, reg(sdword(guest, kMouseY) - h - 8));
        }
    }

    if (guest.dword(kHelpText) != 0)
    {
        if (sdword(guest, kHelpFor) == -1)
        {
            if (sdword(guest, kHelpFade) > 0)
                guest.setDword(kHelpFade, guest.dword(kHelpFade) - 15);
            if (sdword(guest, kHelpFade) <= 0)
                guest.setDword(kHelpText, 0);
        }
        else
        {
            if (sdword(guest, kHelpFade) < 255)
                guest.setDword(kHelpFade, guest.dword(kHelpFade) + 10);
            if (sdword(guest, kHelpFade) > 255)
                guest.setDword(kHelpFade, 255);
        }
    }
    if (guest.dword(kHelpText) != 0)
    {
        const x86::reg32 alpha = guest.dword(kHelpFade) << 24;
        guest.setDword(kDrawColour, alpha);
        const x86::reg32 x = guest.dword(kHelpX), y = guest.dword(kHelpY);
        const x86::reg32 w = guest.dword(kHelpWidth), h = guest.dword(kHelpHeight);
        guest.call(kFillBox, fe::args({ x, y, x + w, y + h }, { alpha + kHelpDepth, alpha + kHelpBoxColour }));
        guest.call(kBoxBorder, fe::args({ x, y, w, h }, { alpha }));
        if (inRace(guest))
            guest.call(kDrawText, fe::args({ guest.dword(kHelpText), x, y, kFont }, { kAlignLeft, kHelpTextColour }));
        else
            guest.call(kDrawTextWrapped, fe::args({ guest.dword(kHelpText), kFont, 8, x + 7 }, { y, kHelpWrap, 0, 0, 0, 0 }));
        guest.setDword(kDrawColour, kOpaque);
    }
    guest.setDword(kHelpAlpha, kFloatOne);
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * The pointer
 * ------------------------------------------------------------------------- */

/* sub_444570, sub_445160: the pointer moved onto the bottom right of a
 * button's or an entry's box. */
x86::reg32 pointAt(Guest& guest, x86::reg32 boxes, std::int32_t count, std::int32_t index)
{
    if (index < 0 || index >= count)
        return reg(index);
    const Box b = box(guest, boxes, index);
    guest.setDword(kMouseX, reg(b.x + b.width - 2));
    guest.setDword(kMouseY, reg(b.y + b.height - 2));
    return guest.call(kSetMousePosition, fe::args({ guest.dword(kMouseX), guest.dword(kMouseY) }));
}

/* sub_4445d0, sub_4451d0: the button or entry under the pointer, -1 for
 * none.  In the menus a box reaches 15 pixels further left and 10 right. */
x86::reg32 underMouse(const Guest& guest, x86::reg32 boxes, std::int32_t count)
{
    const std::int32_t x = sdword(guest, kMouseX), y = sdword(guest, kMouseY);
    for (std::int32_t i = 0; i < count; ++i)
    {
        const Box b = box(guest, boxes, i);
        const bool hit = inRace(guest)
                             ? y > b.y && y < b.y + b.height && x > b.x && x < b.x + b.width
                             : y > b.y - 1 && y < b.y + b.height && x > b.x - 15 && x < b.x + b.width + 10;
        if (hit)
            return reg(i);
    }
    return reg(-1);
}

/* The button under the pointer highlighted, with a tick when it changes; none
 * when the pointer is on no button. */
void followPointer(Guest& guest)
{
    const std::int32_t under = std::int32_t(guest.call(kButtonUnderMouse));
    if (under < 0 || under >= sdword(guest, kButtonCount))
    {
        guest.setDword(kFocus, reg(-1));
        return;
    }
    if (under != sdword(guest, kFocus))
        playSound(guest, kSoundMove, kVolumeTick);
    guest.setDword(kFocus, reg(under));
}

/* ---------------------------------------------------------------------------
 * A question
 * ------------------------------------------------------------------------- */

/* sub_4446d0: a question opened: its lines over its buttons, the buttons in
 * a row or, with `column`, one above the other; the box round both, the
 * pointer on the button highlighted.  Nothing without lines or buttons, or
 * with ten buttons or more. */
x86::reg32 openQuestion(Guest& guest, x86::reg32 lineCount, x86::reg32 lines, std::int32_t buttonCount,
                        x86::reg32 buttons, x86::reg32 buttonHelp, std::int32_t focus, x86::reg32 column,
                        x86::reg32 callback)
{
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0 || buttonCount == 0 || buttons == 0 || buttonCount >= kMaxButtons)
        return guest.eax();
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kLines, lines);
    guest.setDword(kKind, kQuestion);
    guest.setDword(kButtonCount, reg(buttonCount));
    guest.setByte(kShown, 1);
    guest.setDword(kButtons, buttons);
    guest.setDword(kButtonHelp, buttonHelp);
    setFirstFocus(guest, focus, buttonCount);

    const x86::reg32 linesRect = guest.frame() + 0x10, buttonsRect = guest.frame();  // [ebp - 0x38], [ebp - 0x48]
    if (column != 0)
    {
        guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 0, 0 }, { linesRect }));
        guest.call(kLinesBox, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), 0, 0 }, { buttonsRect }));
    }
    else
    {
        guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 1, 0 }, { linesRect }));
        guest.call(kButtonRowBox, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), buttonsRect }));
    }

    // The two boxes put together round the lines' middle, as wide as the wider.
    X87 x87(guest);
    const Float linesWidth = x87.sub(x87.load(linesRect + kRight), x87.load(linesRect + kLeft));
    const Float linesHeight = x87.sub(x87.load(linesRect + kBottom), x87.load(linesRect + kTop));
    const float linesWidthF = float(linesWidth), linesHeightF = float(linesHeight);
    const Float halfLinesWidth = x87.mul(linesWidth, Float(0.5));
    const Float halfLinesHeight = x87.mul(Float(0.5), linesHeight);
    const float buttonsWidthF = float(x87.sub(x87.load(buttonsRect + kRight), x87.load(buttonsRect + kLeft)));
    const float buttonsHeightF = float(x87.sub(x87.load(buttonsRect + kBottom), x87.load(buttonsRect + kTop)));
    const float middleXF = float(x87.add(x87.load(linesRect + kLeft), halfLinesWidth));
    const float middleYF = float(x87.add(x87.load(linesRect + kTop), halfLinesHeight));
    float width, height;
    if (column != 0)
    {
        height = float(x87.add(x87.add(Float(linesHeightF), Float(buttonsHeightF)), Float(-15.0)));
        width = x87.above(Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        guest.setDword(kButtonsX, reg(x87.truncated(Float(middleXF))));
    }
    else
    {
        const Float halfRow = x87.mul(x87.add(Float(buttonsWidthF), Float(-30.0)), Float(0.5));
        guest.setDword(kButtonsX, reg(x87.truncated(x87.sub(Float(middleXF), halfRow))));
        width = x87.above(Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        height = linesHeightF;
    }
    setDialogRect(guest, x87, Float(middleXF), Float(middleYF), x87.mul(Float(0.5), Float(width)),
                  x87.mul(Float(height), Float(0.5)));

    guest.call(kPlaceButtons, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), column, kRect }));
    guest.call(kPointAtButton, fe::args({ guest.dword(kFocus) }));
    guest.setDword(kCallback, callback);
    return 0;
}

/* sub_444910: a key for a question: Enter or Escape answers it, the arrows
 * move the highlight round its buttons. */
x86::reg32 questionKey(Guest& guest, std::uint16_t key)
{
    const std::int32_t last = sdword(guest, kButtonCount) - 1;
    std::int32_t focus = sdword(guest, kFocus);
    switch (key)
    {
    case kKeyEnter:
        return kAnswerEnter;
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyUp:
    case kKeyLeft:
        focus = std::int32_t(x86::reg32(focus) - 1);
        if (focus < 0)
            focus = last;
        break;
    case kKeyDown:
    case kKeyRight:
        focus = std::int32_t(x86::reg32(focus) + 1);
        if (focus > last)
            focus = 0;
        break;
    default:
        return 0;
    }
    guest.setDword(kFocus, reg(focus));
    guest.call(kPointAtButton, fe::args({ reg(focus) }));
    return 0;
}

/* sub_4449d0: a question's pass: the button under the pointer highlighted,
 * the box, the lines and the buttons drawn. */
x86::reg32 questionPass(Guest& guest)
{
    guest.setDword(kListFocus, reg(-1));
    followPointer(guest);
    guest.call(kDrawBox, fe::args({ kRect }));
    guest.call(kDrawLines, fe::args({ guest.dword(kLineCount), guest.dword(kLines), kRect }));
    guest.call(kDrawButtons, fe::args({ kRect, guest.dword(kFocus) }));
    return 0;
}

/* ---------------------------------------------------------------------------
 * A message
 * ------------------------------------------------------------------------- */

/* sub_444a60: the message to go on the next pass. */
x86::reg32 endMessage(Guest& guest)
{
    guest.setDword(kMessageTime, 1);
    guest.setDword(kMessageOpened, 0);
    return guest.eax();
}

/* sub_444a80: a message opened: lines and no buttons, up for `time` ticks (a
 * negative time as long; 0 until a key answers it, whatever `keys` says). */
x86::reg32 openMessage(Guest& guest, x86::reg32 lineCount, x86::reg32 lines, x86::reg32 time, std::uint8_t keys,
                       x86::reg32 callback)
{
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0)
        return guest.eax();
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kKind, kMessage);
    guest.setDword(kLines, lines);
    guest.setByte(kShown, 1);
    guest.setDword(kMessageOpened, guest.dword(kTicks));
    if (std::int32_t(time) <= 0)
        time = 0 - time;
    guest.setDword(kMessageTime, time);
    guest.setByte(kMessageKeys, std::int32_t(time) > 0 ? keys : 1);
    const x86::reg32 result = guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 0, 0 }, { kRect }));
    guest.setDword(kCallback, callback);
    guest.leaveEdx(0);  // as the generated function leaves it (see sub_443a90)
    return result;
}

/* sub_444b20: a key for a message, when keys answer it: Enter, Escape, or
 * the space bar. */
x86::reg32 messageKey(Guest& guest, std::uint16_t key)
{
    if (guest.byte(kMessageKeys) == 0)
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

/* sub_444b60: a message's pass: the box and the lines drawn, and the answer
 * kAnswerTimedOut once its time has gone by. */
x86::reg32 messagePass(Guest& guest)
{
    guest.setDword(kListFocus, reg(-1));
    guest.call(kDrawBox, fe::args({ kRect }));
    guest.call(kDrawLines, fe::args({ guest.dword(kLineCount), guest.dword(kLines), kRect }));
    const std::int32_t time = sdword(guest, kMessageTime);
    if (time <= 0)
        return 0;
    x86::reg32 elapsed = guest.dword(kTicks) - guest.dword(kMessageOpened);
    if (std::int32_t(elapsed) <= 0)
        elapsed = 0 - elapsed;
    return std::int32_t(elapsed) >= time ? kAnswerTimedOut : 0;
}

/* sub_444be0: a message of one line of the language's text, up for 550
 * ticks or until a key answers it. */
x86::reg32 messageById(Guest& guest, std::uint32_t id)
{
    guest.setDword(kMessageLine, guest.call(kText, fe::args({ id })));
    return guest.call(kOpenMessage, fe::args({ 1, kMessageLine, kMessageTicks, 1 }, { 0 }));
}

/* ---------------------------------------------------------------------------
 * A line to type in
 * ------------------------------------------------------------------------- */

/* sub_444c20: a dialog opened to type a line in: its lines over the field,
 * a row of buttons below.  The text starts as `source` when it is one the
 * mode allows (sub_443860), and the first character typed replaces it.
 * The field is wide enough for `maxLength` of the widest character, 240
 * pixels at most; the box widens for it.  Nothing without lines or buttons,
 * with ten buttons or more, or when the mode the last such dialog was opened
 * with is past 3 (the mode is checked before the new one is kept). */
x86::reg32 openEdit(Guest& guest, x86::reg32 lineCount, x86::reg32 lines, std::int32_t buttonCount,
                    x86::reg32 buttons, std::int32_t focus, x86::reg32 source, x86::reg32 maxLength, x86::reg32 mode,
                    x86::reg32 callback)
{
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0 || sdword(guest, kEditMode) >= 4 || buttonCount == 0 || buttons == 0
        || buttonCount >= kMaxButtons)
        return guest.eax();
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kLines, lines);
    guest.setDword(kButtonCount, reg(buttonCount));
    guest.setDword(kButtons, buttons);
    guest.setDword(kKind, kEdit);
    guest.setByte(kShown, 1);
    guest.setDword(kReplaceOnKey, 1);
    setFirstFocus(guest, focus, buttonCount);

    const x86::reg32 linesRect = guest.frame() + 0x10, buttonsRect = guest.frame();  // [ebp - 0x4c], [ebp - 0x5c]
    guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 1, 1 }, { linesRect }));
    guest.call(kButtonRowBox, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), buttonsRect }));

    X87 x87(guest);
    const Float linesWidth = x87.sub(x87.load(linesRect + kRight), x87.load(linesRect + kLeft));
    const float linesWidthF = float(linesWidth);
    const Float halfLinesWidth = x87.mul(linesWidth, Float(0.5));
    const Float linesHeight = x87.sub(x87.load(linesRect + kBottom), x87.load(linesRect + kTop));
    const Float buttonsWidth = x87.sub(x87.load(buttonsRect + kRight), x87.load(buttonsRect + kLeft));
    const float linesHeightF = float(linesHeight);
    const Float halfLinesHeight = x87.mul(linesHeight, Float(0.5));
    const float buttonsWidthF = float(buttonsWidth);
    const Float halfRow = x87.mul(Float(0.5), x87.add(buttonsWidth, Float(-30.0)));
    const float middleXF = float(x87.add(x87.load(linesRect + kLeft), halfLinesWidth));
    const float middleYF = float(x87.add(x87.load(linesRect + kTop), halfLinesHeight));
    guest.setDword(kButtonsX, reg(x87.truncated(x87.sub(Float(middleXF), halfRow))));
    const float width = x87.above(Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
    setDialogRect(guest, x87, Float(middleXF), Float(middleYF), x87.mul(Float(0.5), Float(width)),
                  x87.mul(Float(linesHeightF), Float(0.5)));

    guest.call(kMemset, fe::args({ kEditText, 0, kEditSize }));
    guest.setDword(kEditSource, source);
    guest.setDword(kEditMode, mode);
    guest.setDword(kEditMax, maxLength != 0 && maxLength < kEditSize ? maxLength : kEditSize - 1);
    if ((guest.call(kTextAllowed, fe::args({ guest.dword(kEditSource), guest.dword(kEditMode) })) & 0xff) != 0)
    {
        guest.call(kStrncpy, fe::args({ kEditText, guest.dword(kEditSource), guest.dword(kEditMax) }));
        guest.setDword(kEditLength, guest.length(kEditText));
    }
    else
        guest.setDword(kEditLength, 0);

    // The field: the most characters, the widest of them, and the cursor.
    setFont(guest);
    const float textWidthF = float(x87.mul(X87::integer(textWidth(guest, kWidest.address)),
                                           X87::integer(sdword(guest, kEditMax))));
    const Float room = x87.add(x87.add(X87::integer(textWidth(guest, kCursor.address)), Float(textWidthF)), Float(5.0));
    const Float field = x87.above(room, Float(240.0)) ? Float(240.0) : Float(float(room));
    guest.setDword(kEditWidth, reg(x87.truncated(field)));
    const Float withMargin = x87.add(field, Float(30.0));
    if (x87.above(withMargin, Float(width)))
    {
        // Wider than the lines and the buttons: the box widened round it,
        // short of the screen's width -- and of its height.
        float boxWidth = float(withMargin);
        if (!x87.below(Float(boxWidth), Float(640.0)))
            boxWidth = 639.0f;
        if (!x87.below(Float(boxWidth), X87::integer(480)))
            boxWidth = float(x87.add(X87::integer(480), Float(-1.0)));
        const Float halfWidth = x87.mul(Float(boxWidth), Float(0.5));
        guest.setFloat(kRect + kLeft, float(x87.sub(Float(middleXF), halfWidth)));
        guest.setFloat(kRect + kRight, float(x87.add(halfWidth, Float(middleXF))));
    }

    guest.setDword(kCallback, callback);
    guest.call(kPlaceButtons, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), 0, kRect }));
    return guest.call(kPointAtButton, fe::args({ guest.dword(kFocus) }));
}

/* sub_444f50: a key for the line typed in: Enter or Escape answers, Left and
 * Right move the highlight round the buttons, Backspace takes the last
 * character off and Delete the lot; any other character the mode allows
 * goes on the end while there is room (sub_443810). */
x86::reg32 editKey(Guest& guest, x86::reg32 key)
{
    const std::int32_t last = sdword(guest, kButtonCount) - 1;
    std::int32_t focus = sdword(guest, kFocus);
    switch (std::uint16_t(key))
    {
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyEnter:
        return kAnswerEnter;
    case kKeyBackspace:
        if (sdword(guest, kEditLength) > 0)
        {
            const x86::reg32 length = guest.dword(kEditLength) - 1;
            guest.setDword(kEditLength, length);
            guest.setByte(kEditText + length, 0);
        }
        return 0;
    case kKeyLeft:
        focus = std::int32_t(x86::reg32(focus) - 1);
        if (focus < 0)
            focus = last;
        guest.setDword(kFocus, reg(focus));
        guest.call(kPointAtButton, fe::args({ reg(focus) }));
        return 0;
    case kKeyRight:
        focus = std::int32_t(x86::reg32(focus) + 1);
        if (last < focus)
            focus = 0;
        guest.setDword(kFocus, reg(focus));
        guest.call(kPointAtButton, fe::args({ reg(focus) }));
        return 0;
    case kKeyDelete:
        guest.call(kMemset, fe::args({ kEditText, 0, kEditSize }));
        guest.setDword(kEditLength, 0);
        return 0;
    default:
        break;
    }
    if (sdword(guest, kEditMax) <= sdword(guest, kEditLength) && guest.dword(kReplaceOnKey) == 0)
        return 0;
    if ((guest.call(kCharAllowed, fe::args({ key & 0xff, guest.dword(kEditMode) })) & 0xff) == 0)
        return 0;
    if (guest.dword(kReplaceOnKey) != 0)
    {
        guest.setDword(kReplaceOnKey, 0);
        guest.call(kMemset, fe::args({ kEditText, 0, kEditSize }));
        guest.setDword(kEditLength, 0);
    }
    const x86::reg32 length = guest.dword(kEditLength) + 1;
    guest.setByte(kEditText + length - 1, std::uint8_t(key));
    guest.setDword(kEditLength, length);
    return 0;
}

/* sub_4450c0: the pass of a dialog to type in: as a question's, and the
 * field. */
x86::reg32 editPass(Guest& guest)
{
    guest.setDword(kListFocus, reg(-1));
    followPointer(guest);
    guest.call(kDrawBox, fe::args({ kRect }));
    guest.call(kDrawLines, fe::args({ guest.dword(kLineCount), guest.dword(kLines), kRect }));
    guest.call(kDrawEditField, fe::args({ kRect }));
    guest.call(kDrawButtons, fe::args({ kRect, guest.dword(kFocus) }));
    return 0;
}

/* ---------------------------------------------------------------------------
 * A list
 * ------------------------------------------------------------------------- */

/* sub_4452d0: the list's entries' boxes, from (kListX, kListY) down, 18
 * pixels each (30 in a race).  Six entries or more get none. */
x86::reg32 placeEntries(Guest& guest, std::int32_t count, x86::reg32 names)
{
    setFont(guest);
    const std::int32_t x = sdword(guest, kListX);
    std::int32_t y = sdword(guest, kListY);
    for (std::int32_t i = 0; i < count && count < kMaxEntries; ++i)
    {
        const x86::reg32 name = guest.dword(names + 4 * x86::reg32(i));
        setBoxField(guest, kEntryBoxes, i, kBoxWidth, name != 0 ? textWidth(guest, name) : 0);
        setBoxField(guest, kEntryBoxes, i, kBoxX, x);
        setBoxField(guest, kEntryBoxes, i, kBoxY, y);
        setBoxField(guest, kEntryBoxes, i, kBoxHeight, inRace(guest) ? 30 : 18);
        y += box(guest, kEntryBoxes, i).height;
    }
    return guest.eax();
}

/* sub_445750: a list opened: lines, the entries to tick below them, and
 * buttons below those, in a row or, with `column`, one above the other.
 * Nothing without lines, entries, ticks or buttons, with ten buttons or
 * more, or six entries or more. */
x86::reg32 openList(Guest& guest, x86::reg32 lineCount, x86::reg32 lines, std::int32_t entryCount, x86::reg32 names,
                    x86::reg32 ticks, x86::reg32 entryHelp, std::int32_t buttonCount, x86::reg32 buttons,
                    x86::reg32 buttonHelp, std::int32_t focus, x86::reg32 column, x86::reg32 callback)
{
    guest.setDword(kListHelp, 0);
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0 || entryCount == 0 || names == 0 || ticks == 0 || buttonCount == 0
        || buttons == 0 || buttonCount >= kMaxButtons || entryCount >= kMaxEntries)
        return guest.eax();
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kKind, kList);
    guest.setByte(kShown, 1);
    guest.setByte(kListCount, std::uint8_t(entryCount));
    guest.setDword(kLines, lines);
    guest.setDword(kListTicks, ticks);
    guest.setDword(kListNames, names);
    guest.setDword(kListHelp, entryHelp);
    guest.setDword(kButtonCount, reg(buttonCount));
    guest.setDword(kButtonHelp, buttonHelp);
    guest.setDword(kButtons, buttons);
    guest.setDword(kListFocus, reg(-1));
    setFirstFocus(guest, focus, buttonCount);

    const x86::reg32 linesRect = guest.frame(), buttonsRect = guest.frame() + 0x10;  // [ebp - 0x68], [ebp - 0x58]
    const x86::reg32 listRect = guest.frame() + 0x20;                                // [ebp - 0x48]
    if (column != 0)
    {
        guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 0, 0 }, { linesRect }));
        guest.call(kLinesBox, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), 0, 0 }, { buttonsRect }));
    }
    else
    {
        guest.call(kLinesBox, fe::args({ guest.dword(kLineCount), guest.dword(kLines), 1, 0 }, { linesRect }));
        guest.call(kButtonRowBox, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), buttonsRect }));
    }
    guest.call(kLinesBox, fe::args({ reg(entryCount), names, 0, 0 }, { listRect }));

    // The three boxes put together round the lines' middle, as wide as the
    // widest; the entries 18 pixels each and a margin.
    X87 x87(guest);
    const Float linesWidth = x87.sub(x87.load(linesRect + kRight), x87.load(linesRect + kLeft));
    const Float linesHeight = x87.sub(x87.load(linesRect + kBottom), x87.load(linesRect + kTop));
    const float linesWidthF = float(linesWidth);
    const Float halfLinesWidth = x87.mul(linesWidth, Float(0.5));
    const float linesHeightF = float(linesHeight);
    const Float halfLinesHeight = x87.mul(Float(0.5), linesHeight);
    const float listWidthF = float(x87.sub(x87.load(listRect + kRight), x87.load(listRect + kLeft)));
    const float buttonsWidthF = float(x87.sub(x87.load(buttonsRect + kRight), x87.load(buttonsRect + kLeft)));
    const float buttonsHeightF = float(x87.sub(x87.load(buttonsRect + kBottom), x87.load(buttonsRect + kTop)));
    const float middleXF = float(x87.add(x87.load(linesRect + kLeft), halfLinesWidth));
    const float listHeightF = float(X87::integer(18 * entryCount + 20));
    const float middleYF = float(x87.add(x87.load(linesRect + kTop), halfLinesHeight));
    float width, height;
    if (column != 0)
    {
        height = float(x87.add(x87.add(x87.add(Float(linesHeightF), Float(buttonsHeightF)), Float(listHeightF)),
                               Float(-15.0)));
        width = x87.above(Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        if (x87.below(Float(width), Float(listWidthF)))
            width = listWidthF;
        guest.setDword(kButtonsX, reg(x87.truncated(Float(middleXF))));
    }
    else
    {
        const Float halfRow = x87.mul(x87.add(Float(buttonsWidthF), Float(-30.0)), Float(0.5));
        guest.setDword(kButtonsX, reg(x87.truncated(x87.sub(Float(middleXF), halfRow))));
        width = x87.above(Float(buttonsWidthF), Float(linesWidthF)) ? buttonsWidthF : linesWidthF;
        if (x87.below(Float(width), Float(listWidthF)))
            width = listWidthF;
        height = float(x87.add(Float(linesHeightF), Float(listHeightF)));
    }
    const Float halfListWidth = x87.mul(Float(listWidthF), Float(0.5));
    const Float middleX = Float(middleXF), middleY = Float(middleYF);
    const Float halfWidth = x87.mul(Float(0.5), Float(width)), halfHeight = x87.mul(Float(height), Float(0.5));
    setDialogRect(guest, x87, middleX, middleY, halfWidth, halfHeight);
    // The entries: 15 pixels in from the list's own box, 35 below the lines'
    // top and 30 a line.
    const Float listLeft = x87.sub(middleX, halfListWidth);
    guest.setDword(kListX, reg(x87.truncated(x87.add(listLeft, Float(15.0)))));
    const Float top = Float(guest.floatAt(kRect + kTop));
    guest.setDword(kListY, reg(x87.truncated(x87.add(X87::integer(30 * std::int32_t(guest.dword(kLineCount))),
                                                     x87.add(top, Float(35.0))))));

    guest.call(kPlaceEntries, fe::args({ guest.byte(kListCount), guest.dword(kListNames) }));
    guest.call(kPlaceButtons, fe::args({ guest.dword(kButtonCount), guest.dword(kButtons), column, kRect }));
    guest.call(kPointAtButton, fe::args({ guest.dword(kFocus) }));
    guest.setDword(kCallback, callback);
    return guest.eax();
}

/* sub_445520: a key for a list: Up and Down move the highlight round its
 * entries, Left and Right round its buttons; Enter ticks the entry
 * highlighted and answers, as Escape does. */
x86::reg32 listKey(Guest& guest, std::uint16_t key)
{
    const std::int32_t entries = guest.byte(kListCount);
    const std::int32_t buttons = sdword(guest, kButtonCount);
    std::int32_t entry = sdword(guest, kListFocus), focus = sdword(guest, kFocus);
    switch (key)
    {
    case kKeyEnter:
        if (entry >= 0 && entry < entries)
        {
            const x86::reg32 tick = guest.dword(guest.dword(kListTicks) + 4 * x86::reg32(entry));
            if (tick != 0)
                guest.setDword(tick, 1);
        }
        return kAnswerEnter;
    case kKeyEscape:
        return kAnswerEscape;
    case kKeyUp:
        entry = std::int32_t(x86::reg32(entry) - 1);
        if (entry < 0)
            entry = entries - 1;
        guest.setDword(kListFocus, reg(entry));
        guest.call(kPointAtEntry, fe::args({ reg(entry) }));
        return 0;
    case kKeyDown:
        entry = std::int32_t(x86::reg32(entry) + 1);
        if (entries - 1 < entry)
            entry = 0;
        guest.setDword(kListFocus, reg(entry));
        guest.call(kPointAtEntry, fe::args({ reg(entry) }));
        return 0;
    case kKeyLeft:
        focus = std::int32_t(x86::reg32(focus) - 1);
        if (focus < 0)
            focus = buttons - 1;
        guest.setDword(kFocus, reg(focus));
        guest.call(kPointAtButton, fe::args({ reg(focus) }));
        return 0;
    case kKeyRight:
        focus = std::int32_t(x86::reg32(focus) + 1);
        if (buttons - 1 < focus)
            focus = 0;
        guest.setDword(kFocus, reg(focus));
        guest.call(kPointAtButton, fe::args({ reg(focus) }));
        return 0;
    default:
        return 0;
    }
}

/* sub_445680: a list's pass: the button or the entry under the pointer
 * highlighted (the other not), the box, the lines, the entries and the
 * buttons drawn. */
x86::reg32 listPass(Guest& guest)
{
    const std::int32_t button = std::int32_t(guest.call(kButtonUnderMouse));
    const std::int32_t entry = std::int32_t(guest.call(kEntryUnderMouse));
    if (button >= 0 && button < sdword(guest, kButtonCount))
    {
        if (button != sdword(guest, kFocus))
            playSound(guest, kSoundMove, kVolumeTick);
        guest.setDword(kFocus, reg(button));
        guest.setDword(kListFocus, reg(-1));
    }
    else if (entry >= 0 && entry < std::int32_t(guest.byte(kListCount)))
    {
        if (entry != sdword(guest, kListFocus))
            playSound(guest, kSoundMove, kVolumeTick);
        guest.setDword(kListFocus, reg(entry));
        guest.setDword(kFocus, reg(-1));
    }
    else
    {
        guest.setDword(kFocus, reg(-1));
        guest.setDword(kListFocus, reg(-1));
    }
    guest.call(kDrawBox, fe::args({ kRect }));
    guest.call(kDrawLines, fe::args({ guest.dword(kLineCount), guest.dword(kLines), kRect }));
    guest.call(kDrawList);
    guest.call(kDrawButtons, fe::args({ kRect, guest.dword(kFocus) }));
    return 0;
}

/* ---------------------------------------------------------------------------
 * The dialog up, whatever its kind
 * ------------------------------------------------------------------------- */

/* sub_445de0: the dialogs' shapes found among the front end's. */
x86::reg32 loadDialogArt(Guest& guest)
{
    auto find = [&](const fe::GuestString& name) { return guest.call(kFindShape, fe::args({ name.address })); };
    guest.setDword(kBoxShape, find(kPopu));
    guest.setDword(kButtonShape, find(kBpop));
    guest.setDword(kFocusShape, find(kYpop));
    guest.setDword(kTickedShape, find(kGlit));
    guest.setDword(kUntickedShape, find(kRlit));
    if ((guest.byte(kDisplayFlags) & kNoTranslucency) == 0)
        guest.setDword(kYel1Shape, find(kYel1));
    return guest.eax();
}

/* An answer handed to the dialog's callback, with the byte at `keepUp`, 1,
 * that the callback clears to keep the dialog up; what the callback returns
 * is for the menu loop. */
x86::reg32 handOn(Guest& guest, x86::reg32 answer, x86::reg32 keepUp)
{
    x86::reg32 result = 0;
    if (answer == 0)
        return result;
    if (guest.dword(kCallback) != 0)
        result = guest.call(guest.dword(kCallback), fe::args({ answer, keepUp }));
    if (guest.byte(keepUp) != 0)
        guest.call(kClose);
    return result;
}

/* sub_445e60: a key for the dialog up, once it has faded in (at once in a
 * race). */
x86::reg32 dialogKey(Guest& guest, x86::reg32 key)
{
    const x86::reg32 keepUp = guest.frame();  // [ebp - 4]
    guest.setByte(keepUp, 1);
    if (!inRace(guest) && sdword(guest, kFade) < 255)
        return 0;
    const x86::reg32 code = reg(std::int16_t(key));
    x86::reg32 answer = 0;
    switch (guest.dword(kKind))
    {
    case kQuestion:
        answer = guest.call(kQuestionKey, fe::args({ code }));
        break;
    case kMessage:
        answer = guest.call(kMessageKey, fe::args({ code }));
        break;
    case kEdit:
        answer = guest.call(kEditKey, fe::args({ code }));
        break;
    case kList:
        answer = guest.call(kListKey, fe::args({ code }));
        break;
    default:
        break;
    }
    const x86::reg32 result = handOn(guest, answer, keepUp);
    return result == fe::kDialogHandled ? 0 : result;
}

/* sub_445f50: the dialog up, a pass of it: faded in a step, drawn, its
 * answer if the pass made one, and its help box. */
x86::reg32 dialogPass(Guest& guest)
{
    const x86::reg32 keepUp = guest.frame();  // [ebp - 4]
    guest.setByte(keepUp, 1);
    guest.setDword(kDepthNear, 0);
    guest.setDword(kDepthFar, kFloatAlmostOne);
    if (sdword(guest, kFade) < 255)
    {
        guest.setDword(kFade, reg(std::min(sdword(guest, kFade) + 20, 255)));
        guest.setDword(kDrawColour, fadeAlpha(guest));
    }
    x86::reg32 answer = 0;
    switch (guest.dword(kKind))
    {
    case kQuestion:
        answer = guest.call(kQuestionPass);
        break;
    case kMessage:
        answer = guest.call(kMessagePass);
        break;
    case kEdit:
        answer = guest.call(kEditPass);
        break;
    case kList:
        answer = guest.call(kListPass);
        break;
    default:
        break;
    }
    x86::reg32 result = handOn(guest, answer, keepUp);
    if (!inRace(guest))
        guest.setDword(kDrawColour, kOpaque);
    if (result == fe::kDialogHandled)
        result = 0;
    guest.setDword(kDepthNear, 0);
    guest.setDword(kDepthFar, kFloatAlmostOne);
    guest.call(kDrawDialogHelp);
    guest.setDword(kDepthNear, kFloatAlmostOne);
    guest.setDword(kDepthFar, kFloatTiny);
    return result;
}

// A stand-in's stack argument, counted from the first.
x86::reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, x86::reg32 index)
{
    return app->getMemory<x86::reg32>(cpu.esp + 4 + 4 * index);
}

const fe::GuestString kEditStrings[] = { kWidest, kCursor };
const fe::GuestString kFieldStrings[] = { kCursor };
const fe::GuestString kArtStrings[] = { kPopu, kBpop, kYpop, kGlit, kRlit, kYel1 };

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

bool dialogShownStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4438b0", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return dialogShown(guest); });
}

bool dialogKindStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4438c0", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return dialogKind(guest); });
}

bool dialogDrawBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4438d0", 0, 24 + 0x20, nullptr, 0 };
    const x86::reg32 rect = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawBox(guest, rect); });
}

bool dialogLinesBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443a90", 4, 12 + 0x24, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 lines = cpu.edx, rowBelow = cpu.ebx, fieldBelow = cpu.ecx;
    const x86::reg32 rect = stackArgument(app, cpu, 0);
    return fe::run(app, cpu, site, [&](Guest& guest) { return linesBox(guest, count, lines, rowBelow, fieldBelow, rect); });
}

bool dialogButtonRowBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443c00", 0, 16 + 0x18, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 buttons = cpu.edx, rect = cpu.ebx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return buttonRowBox(guest, count, buttons, rect); });
}

bool dialogPlaceButtons(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443d20", 0, 12 + 0x14, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 buttons = cpu.edx, column = cpu.ebx, rect = cpu.ecx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return placeButtons(guest, count, buttons, column, rect); });
}

bool dialogDrawLines(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443f40", 0, 16 + 0x18, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 lines = cpu.edx, rect = cpu.ebx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawLines(guest, count, lines, rect); });
}

bool dialogDrawButtons(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444000", 0, 20 + 0x3c, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const std::int32_t focus = std::int32_t(cpu.edx);
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawButtons(guest, focus); });
}

bool dialogDrawEditField(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444350", 0, 24 + 0x14, kFieldStrings, std::size(kFieldStrings) };
    const x86::reg32 rect = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawEditField(guest, rect); });
}

bool dialogPointAtButton(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444570", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const std::int32_t index = std::int32_t(cpu.eax);
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return pointAt(guest, kButtonBoxes, sdword(guest, kButtonCount), index);
    });
}

bool dialogButtonUnderMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4445d0", 0, 24 + 8, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return underMouse(guest, kButtonBoxes, sdword(guest, kButtonCount));
    });
}

bool dialogFocusStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4446c0", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return focusedButton(guest); });
}

bool dialogOpenQuestion(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4446d0", 0x10, 12 + 0x48, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, buttons = cpu.ecx;
    const std::int32_t buttonCount = std::int32_t(cpu.ebx);
    const x86::reg32 buttonHelp = stackArgument(app, cpu, 0), column = stackArgument(app, cpu, 2);
    const std::int32_t focus = std::int32_t(stackArgument(app, cpu, 1));
    const x86::reg32 callback = stackArgument(app, cpu, 3);
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return openQuestion(guest, lineCount, lines, buttonCount, buttons, buttonHelp, focus, column, callback);
    });
}

bool dialogQuestionKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444910", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const std::uint16_t key = std::uint16_t(cpu.eax);
    return fe::run(app, cpu, site, [&](Guest& guest) { return questionKey(guest, key); });
}

bool dialogQuestionPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4449d0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return questionPass(guest); });
}

bool dialogEndMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444a60", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return endMessage(guest); });
}

bool dialogOpenMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444a80", 4, 12, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, time = cpu.ebx;
    const std::uint8_t keys = std::uint8_t(cpu.ecx);
    const x86::reg32 callback = stackArgument(app, cpu, 0);
    return fe::run(app, cpu, site, [&](Guest& guest) { return openMessage(guest, lineCount, lines, time, keys, callback); });
}

bool dialogMessageKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444b20", 0, 12, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const std::uint16_t key = std::uint16_t(cpu.eax);
    return fe::run(app, cpu, site, [&](Guest& guest) { return messageKey(guest, key); });
}

bool dialogMessagePass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444b60", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return messagePass(guest); });
}

bool dialogMessageById(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444be0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const std::uint32_t id = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return messageById(guest, id); });
}

bool dialogEditText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444c10", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest&) { return editText(); });
}

bool dialogOpenEdit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444c20", 0x14, 12 + 0x5c, kEditStrings, std::size(kEditStrings), kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, buttons = cpu.ecx;
    const std::int32_t buttonCount = std::int32_t(cpu.ebx);
    const std::int32_t focus = std::int32_t(stackArgument(app, cpu, 0));
    const x86::reg32 source = stackArgument(app, cpu, 1), maxLength = stackArgument(app, cpu, 2);
    const x86::reg32 mode = stackArgument(app, cpu, 3), callback = stackArgument(app, cpu, 4);
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return openEdit(guest, lineCount, lines, buttonCount, buttons, focus, source, maxLength, mode, callback);
    });
}

bool dialogEditKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444f50", 0, 24, nullptr, 0 };
    const x86::reg32 key = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return editKey(guest, key); });
}

bool dialogEditPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4450c0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return editPass(guest); });
}

bool dialogListFocus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445150", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return listFocus(guest); });
}

bool dialogPointAtEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445160", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const std::int32_t index = std::int32_t(cpu.eax);
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return pointAt(guest, kEntryBoxes, guest.byte(kListCount), index);
    });
}

bool dialogEntryUnderMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4451d0", 0, 20 + 8, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return underMouse(guest, kEntryBoxes, guest.byte(kListCount));
    });
}

bool dialogPlaceEntries(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4452d0", 0, 16 + 8, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 names = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return placeEntries(guest, count, names); });
}

bool dialogDrawList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445380", 0, 24 + 0x14, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawList(guest); });
}

bool dialogListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445520", 0, 24, nullptr, 0 };
    const std::uint16_t key = std::uint16_t(cpu.eax);
    return fe::run(app, cpu, site, [&](Guest& guest) { return listKey(guest, key); });
}

bool dialogListPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445680", 0, 24, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) { return listPass(guest); });
}

bool dialogOpenList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445750", 0x20, 12 + 0x68, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, names = cpu.ecx;
    const std::int32_t entryCount = std::int32_t(cpu.ebx);
    x86::reg32 stack[8];
    for (x86::reg32 i = 0; i < 8; ++i)
        stack[i] = stackArgument(app, cpu, i);
    return fe::run(app, cpu, site, [&](Guest& guest) {
        return openList(guest, lineCount, lines, entryCount, names, stack[0], stack[1], std::int32_t(stack[2]), stack[3],
                        stack[4], std::int32_t(stack[5]), stack[6], stack[7]);
    });
}

bool dialogDrawHelp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445ac0", 0, 24 + 8, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawDialogHelp(guest); });
}

bool dialogLoadArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445de0", 0, 4, kArtStrings, std::size(kArtStrings), kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return loadDialogArt(guest); });
}

bool dialogKeyStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445e60", 0, 16 + 4, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 key = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return dialogKey(guest, key); });
}

bool dialogFadedIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445f00", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return fadedIn(guest); });
}

bool dialogClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445f10", 0, 8, nullptr, 0, kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return closeDialog(guest); });
}

bool dialogPassStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445f50", 0, 24 + 4, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) { return dialogPass(guest); });
}

}
