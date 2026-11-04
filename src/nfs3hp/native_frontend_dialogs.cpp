#include "native_frontend.h"
#include <algorithm>
#include <iterator>

/* What the dialogs (port::dialogs, src/nfs3hp/port_dialogs.cpp) leave to the
 * menus' code: native whole, with no generated code behind them
 * (disassemble_nfs3hp.py's MENU_NATIVES), a question, a message and a line to
 * type in as the main menu opens them, their layout -- the box round the
 * lines, the one round the buttons, the buttons placed -- and the pointer put
 * on a button.
 *
 * One dialog is up at a time (kShown), of a kind (kKind), with its lines of
 * text, its buttons and the button highlighted (kFocus).  The layout is worked
 * out as the dialog opens, in floats on the 640x480 screen: a box round the
 * lines, one round the buttons, put together and centred (kRect: top, left,
 * bottom, right), the buttons placed below the lines in a row or one above the
 * other (kButtonBoxes).  The arithmetic is the x87's (fe::X87): in a race's
 * pause screens it rounds to a float's precision, and the dialog lands where
 * the game puts it. */
namespace nfs3hp
{

namespace watcom
{
void CHP(win32::WinApplication* app, x86::CPU& cpu);  // __CHP, native_watcom.cpp
}

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
// A list's.
constexpr x86::reg32 kListCount = 0x558c64;         // byte

// Where it is drawn: the box (floats), the buttons' and the entries' boxes
// (8 bytes each, int16), and how far it has faded in.
constexpr x86::reg32 kRect = 0x661050;
constexpr x86::reg32 kTop = 0x0;
constexpr x86::reg32 kLeft = 0x4;
constexpr x86::reg32 kBottom = 0x8;
constexpr x86::reg32 kRight = 0xc;
constexpr x86::reg32 kButtonBoxes = 0x661000;
constexpr x86::reg32 kBoxY = 0;
constexpr x86::reg32 kBoxX = 2;
constexpr x86::reg32 kBoxWidth = 4;
constexpr x86::reg32 kBoxHeight = 6;
constexpr x86::reg32 kBoxSize = 8;
constexpr x86::reg32 kFade = 0x661064;              // 0 to 255

// Its art: the front end's shapes, by name (sub_4d5db0), and in a race's
// pause screens parts of their art, a texture.

// The help box over a dialog (sub_445ac0), for a button or an entry.
constexpr x86::reg32 kHelpText = 0x558b0c;

// The menus'.
constexpr x86::reg32 kInRace = 0x559234;            // the menus are a race's pause screens
constexpr x86::reg32 kMouseX = 0x55e5e4;
constexpr x86::reg32 kMouseY = 0x55e5e8;
constexpr x86::reg32 kTicks = 0x79c1a4;             // the game's clock
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kNoTranslucency = 0x02;      // no fades
constexpr x86::reg32 kDisplayChoice = 0x6fbb4c;
// The depth range 2D is drawn in.

// Text.
constexpr std::uint32_t kFont = 0x12;

// Keys, as sub_451960 gives them, and the answers they make.

// Sounds, sub_4181d0(sound, volume).

// The game's.
constexpr x86::reg32 kText = 0x4d1850;
constexpr x86::reg32 kSetFont = 0x452100;
constexpr x86::reg32 kTextWidth = 0x452020;         // (text, font)
constexpr x86::reg32 kMinus15 = 0x538ae8;           // float -15
constexpr x86::reg32 kMinus30 = 0x538aec;           // float -30
constexpr x86::reg32 kMemset = 0x4e0640;
constexpr x86::reg32 kStrncpy = 0x4e0e30;
constexpr x86::reg32 kSetMousePosition = 0x49bfc0;
constexpr x86::reg32 kTextAllowed = 0x443860;       // (text, mode)

// This file's own, called where they are so each runs as itself.
constexpr x86::reg32 kLinesBox = 0x443a90;
constexpr x86::reg32 kButtonRowBox = 0x443c00;
constexpr x86::reg32 kPlaceButtons = 0x443d20;
constexpr x86::reg32 kPointAtButton = 0x444570;
constexpr x86::reg32 kOpenMessage = 0x444a80;
constexpr x86::reg32 kFadedIn = 0x445f00;

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

/* ---------------------------------------------------------------------------
 * The state
 * ------------------------------------------------------------------------- */

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

/* sub_445f00: the dialog shown at once, faded in. */
x86::reg32 fadedIn(Guest& guest)
{
    guest.setDword(kFade, 255);
    return guest.eax();
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
    // The registers as the generated function leaves them: the box's right
    // edge in ebx, its bottom in eax with the display's flags in ah.
    guest.leaveEbx(reg(x + width));
    guest.leaveEdx(0);
    return (reg(y + height) & ~0xff00u) | (x86::reg32(guest.byte(kDisplayFlags)) << 8);
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
    guest.leaveEdx(callback);  // as the generated function leaves it
    return 0;
}

/* ---------------------------------------------------------------------------
 * A message
 * ------------------------------------------------------------------------- */

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

/* ---------------------------------------------------------------------------
 * A list
 * ------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * The dialog up, whatever its kind
 * ------------------------------------------------------------------------- */

// A stand-in's stack argument, counted from the first.
x86::reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, x86::reg32 index)
{
    return app->getMemory<x86::reg32>(cpu.esp + 4 + 4 * index);
}

const fe::GuestString kEditStrings[] = { kWidest, kCursor };

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

/* Native whole, no generated code behind them (disassemble_nfs3hp.py's
 * MENU_NATIVES): a question, a message, a line to type in as the main menu
 * opens them, and what they open with. */
namespace frontend
{

/* The button highlighted, native whole: the main menu's question asks it. */
void dialogFocusedButton(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4446c0", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return focusedButton(guest); });
}

void dialogLinesBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443a90", 4, 12 + 0x24, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 lines = cpu.edx, rowBelow = cpu.ebx, fieldBelow = cpu.ecx;
    const x86::reg32 rect = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return linesBox(guest, count, lines, rowBelow, fieldBelow, rect); });
}

void dialogButtonRowBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443c00", 0, 16 + 0x18, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const std::int32_t count = std::int32_t(cpu.eax);
    const x86::reg32 buttons = cpu.edx, rect = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return buttonRowBox(guest, count, buttons, rect); });
}

/* sub_443d20: the buttons' boxes (kButtonBoxes) in the dialog's box at
 * `rect`: in a column, up from its bottom 30 pixels a button, centred on
 * kButtonsX and then lined up on the leftmost; or in a row 45 pixels above
 * its bottom, spread evenly across it from kButtonsX.  Ten buttons or more
 * get no boxes.
 *
 * Written as its code is, the registers and the x87 as it leaves them, for
 * its callers' sake: each push and pop to the guest's stack, the truncations
 * by __CHP, the game's text width and font by address. */
void dialogPlaceButtons(win32::WinApplication* app, x86::CPU& cpu)
{
    using x86::reg32;
    using x86::sreg32;
    auto at = [](win32::WinApplication* app, reg32 address) { return app->getMemory<reg32>(address); };
    auto put = [](win32::WinApplication* app, reg32 address, reg32 value) { app->getMemory<reg32>(address) = value; };
    auto push = [&](reg32 value) {
        cpu.esp -= 4;
        put(app, cpu.esp, value);
    };
    auto pop = [&] {
        const reg32 value = at(app, cpu.esp);
        cpu.esp += 4;
        return value;
    };
    auto call = [&](reg32 target) {
        cpu.esp -= 4;
        app->dynamic_call(target, cpu);
        return !cpu.terminate;
    };
    auto chop = [&] {
        cpu.esp -= 4;
        watcom::CHP(app, cpu);
        return !cpu.terminate;
    };
    auto setWord = [&](reg32 address, reg32 value) { app->getMemory<x86::reg16>(address) = x86::reg16(value); };
    auto high = [&](reg32 address) { return reg32(sreg32(at(app, address)) >> 16); };  // mov, sar 0x10
    auto loadFloat = [&](reg32 address) { return Float(app->getMemory<float>(address)); };
    auto fistp = [&](reg32 address) {
        put(app, address, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
        cpu.fpu.count -= 1;
    };
    auto half = [](reg32 value) { return reg32((sreg32(value) - (sreg32(value) >> 31)) >> 1); };  // cdq, sub, sar 1

    push(cpu.esi);
    push(cpu.edi);
    push(cpu.ebp);
    cpu.ebp = cpu.esp;
    const reg32 ebp = cpu.ebp;
    cpu.esp -= 0x14;
    cpu.esi = cpu.eax;
    put(app, ebp - 4, cpu.edx);
    cpu.edx = cpu.ebx;
    cpu.ebx = cpu.ecx;
    cpu.eax = kFont;
    if (!call(kSetFont))
        return;
    cpu.eax = at(app, kButtonsX);
    put(app, ebp - 0x10, cpu.eax);
    if (cpu.edx != 0)
    {
        // A column, up from the box's bottom.
        cpu.fpu.push(loadFloat(cpu.ecx + kBottom));
        cpu.edi = 0x280;
        cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), loadFloat(kMinus15));
        cpu.ecx = cpu.esi - 1;
        if (!chop())
            return;
        fistp(ebp - 8);
        for (; sreg32(cpu.ecx) >= 0; --cpu.ecx)
        {
            if (sreg32(cpu.esi) >= kMaxButtons)
                continue;
            cpu.edx = at(app, ebp - 4);
            cpu.eax = at(app, cpu.ecx * 4 + cpu.edx);
            if (cpu.eax == 0)
                continue;
            cpu.ebx = at(app, ebp - 8);
            cpu.edx = kFont;
            cpu.ebx -= 0x1e;
            if (!call(kTextWidth))
                return;
            put(app, ebp - 8, cpu.ebx);
            cpu.ebx = cpu.eax + 5;
            cpu.edx = reg32(sreg32(cpu.ebx) >> 31);
            cpu.eax = half(cpu.ebx);
            cpu.edx = at(app, ebp - 0x10) - cpu.eax + 8;
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxX, cpu.edx);
            cpu.eax = high(cpu.ecx * 8 + kButtonBoxes);
            if (sreg32(cpu.eax) < sreg32(cpu.edi))
                cpu.edi = cpu.eax;
            cpu.eax = cpu.ecx * 8;
            cpu.edx = at(app, kInRace);
            setWord(cpu.eax + kButtonBoxes + kBoxWidth, cpu.ebx);
            setWord(cpu.eax + kButtonBoxes + kBoxHeight, cpu.edx != 0 ? 0x1e : 0x11);
            cpu.eax = at(app, ebp - 8);
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxY, cpu.eax);
        }
        // Lined up on the leftmost.
        for (cpu.ecx = cpu.esi - 1; sreg32(cpu.ecx) >= 0; --cpu.ecx)
        {
            if (sreg32(cpu.esi) >= kMaxButtons)
                continue;
            cpu.ebx = at(app, ebp - 4);
            cpu.eax = at(app, cpu.ecx * 4 + cpu.ebx);
            if (cpu.eax != 0)
                setWord(cpu.ecx * 8 + kButtonBoxes + kBoxX, cpu.edi);
        }
    }
    else
    {
        // A row, spread evenly across the box.
        cpu.fpu.push(loadFloat(cpu.ebx + kRight));
        cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), loadFloat(cpu.ecx + kLeft));
        if (!chop())
            return;
        fistp(ebp - 0x14);
        cpu.eax = at(app, ebp - 0x14);
        cpu.fpu.push(loadFloat(cpu.ecx + kBottom));
        cpu.edx = reg32(sreg32(cpu.eax) >> 31);
        cpu.ecx = cpu.esi + 1;
        {
            const sreg32 dividend = sreg32(cpu.eax), divisor = sreg32(cpu.ecx);  // idiv ecx
            cpu.eax = reg32(dividend / divisor);
            cpu.edx = reg32(dividend % divisor);
        }
        cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), loadFloat(kMinus15));
        cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), loadFloat(kMinus30));
        if (!chop())
            return;
        fistp(ebp - 8);
        put(app, ebp - 0xc, cpu.eax);
        for (cpu.ecx = 0; sreg32(cpu.ecx) < sreg32(cpu.esi); ++cpu.ecx)
        {
            if (sreg32(cpu.esi) >= kMaxButtons)
                continue;
            cpu.edi = at(app, ebp - 4);
            cpu.eax = at(app, cpu.ecx * 4 + cpu.edi);
            if (cpu.eax == 0)
                continue;
            cpu.edx = kFont;
            if (!call(kTextWidth))
                return;
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxWidth, cpu.eax);
            cpu.edx = at(app, ebp - 0xc);
            cpu.fpu.push(loadFloat(cpu.ebx + kLeft));
            if (!chop())
                return;
            fistp(ebp - 0x14);
            put(app, ebp - 0x14, at(app, ebp - 0x14) + cpu.edx);
            cpu.eax = cpu.edx * cpu.ecx;  // imul: its low 32 bits
            put(app, ebp - 0x14, at(app, ebp - 0x14) + cpu.eax);
            cpu.edx = high(cpu.ecx * 8 + kButtonBoxes + kBoxX);
            cpu.eax = half(cpu.edx);
            cpu.edx = reg32(sreg32(cpu.edx) >> 31);
            cpu.edx = at(app, ebp - 0x14) - cpu.eax;
            cpu.edi = 0x1e;
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxHeight, cpu.edi);
            cpu.eax = cpu.edx + 8;
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxX, cpu.eax);
            cpu.eax = at(app, ebp - 8);
            setWord(cpu.ecx * 8 + kButtonBoxes + kBoxY, cpu.eax);
        }
        if (at(app, kDisplayChoice) == 3 && cpu.esi == 2)
        {
            // Two buttons five pixels further apart each.
            cpu.dx = app->getMemory<x86::reg16>(kButtonBoxes + kBoxSize + kBoxX);
            cpu.ax = app->getMemory<x86::reg16>(kButtonBoxes + kBoxX);
            cpu.edx += 5;
            cpu.eax -= 5;
            setWord(kButtonBoxes + kBoxSize + kBoxX, cpu.edx);
            setWord(kButtonBoxes + kBoxX, cpu.eax);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop();
    cpu.edi = pop();
    cpu.esi = pop();
    cpu.esp += 4;
}

void dialogPointAtButton(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444570", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const std::int32_t index = std::int32_t(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return pointAt(guest, kButtonBoxes, sdword(guest, kButtonCount), index);
    });
}

void dialogOpenQuestion(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4446d0", 0x10, 12 + 0x48, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, buttons = cpu.ecx;
    const std::int32_t buttonCount = std::int32_t(cpu.ebx);
    const x86::reg32 buttonHelp = stackArgument(app, cpu, 0), column = stackArgument(app, cpu, 2);
    const std::int32_t focus = std::int32_t(stackArgument(app, cpu, 1));
    const x86::reg32 callback = stackArgument(app, cpu, 3);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return openQuestion(guest, lineCount, lines, buttonCount, buttons, buttonHelp, focus, column, callback);
    });
}

void dialogOpenMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444a80", 4, 12, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, time = cpu.ebx;
    const std::uint8_t keys = std::uint8_t(cpu.ecx);
    const x86::reg32 callback = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return openMessage(guest, lineCount, lines, time, keys, callback); });
}

void dialogMessageById(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444be0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const std::uint32_t id = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return messageById(guest, id); });
}

void dialogEditText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444c10", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return editText(); });
}

void dialogOpenEdit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_444c20", 0x14, 12 + 0x5c, kEditStrings, std::size(kEditStrings), kRestoreEsi | kRestoreEdi };
    const x86::reg32 lineCount = cpu.eax, lines = cpu.edx, buttons = cpu.ecx;
    const std::int32_t buttonCount = std::int32_t(cpu.ebx);
    const std::int32_t focus = std::int32_t(stackArgument(app, cpu, 0));
    const x86::reg32 source = stackArgument(app, cpu, 1), maxLength = stackArgument(app, cpu, 2);
    const x86::reg32 mode = stackArgument(app, cpu, 3), callback = stackArgument(app, cpu, 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return openEdit(guest, lineCount, lines, buttonCount, buttons, focus, source, maxLength, mode, callback);
    });
}

void dialogFadedIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_445f00", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return fadedIn(guest); });
}

}  // namespace frontend

}
