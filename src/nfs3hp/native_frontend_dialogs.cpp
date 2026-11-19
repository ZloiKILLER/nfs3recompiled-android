#include "native_frontend_dialogs.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_screen.h"
#include "native_frontend_text.h"
#include "port_text.h"
#include <algorithm>
#include <iterator>

/* What the dialogs (port::dialogs, src/nfs3hp/port_dialogs.cpp) leave to the
 * menus' code: native whole, a question, a message and a line to
 * type in as the main menu opens them, their layout -- the box round the
 * lines, the one round the buttons, the buttons placed -- and the pointer put
 * on a button.
 *
 * One dialog is up at a time (kShown), of a kind (kKind), with its lines of
 * text, its buttons and the button highlighted (kFocus).  The layout is worked
 * out as the dialog opens, in floats on the 640x480 screen: a box round the
 * lines, one round the buttons, put together and centred (kRect: top, left,
 * bottom, right), the buttons placed below the lines in a row or one above the
 * other (kButtonBoxes).  The arithmetic is the x87's (fe::Arith): in a race's
 * pause screens it rounds to a float's precision, and the dialog lands where
 * the game puts it.
 *
 * Plan step 5: each function a typed core in frontend::dialogs (native_frontend_dialogs.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the game's
 * name for the calls by address.  A core of this file calls another as C++ (guest::direct) but
 * where its caller goes on with the registers the call leaves: sub_443d20's (the dialogs' callers
 * read them) and sub_443a90's from sub_444a80 (the screens that open a message go on with them).
 * The port's text (the font, a width, a line of the language's) and sub_443860 as C++ too
 * (fe::direct); the pointer by address, its eax and edx left for sub_444570's callers. */
namespace nfs3hp
{

namespace watcom
{
}

namespace
{

using fe::Guest;
using fx::Arith;
using nfs::Float;

// The dialog up.
constexpr nfs::u32 kShown = 0x558b10;            // byte
constexpr nfs::u32 kKind = 0x558b14;
constexpr std::uint32_t kQuestion = 1;              // sub_4446d0
constexpr std::uint32_t kMessage = 2;               // sub_444a80
constexpr std::uint32_t kEdit = 3;                  // sub_444c20
constexpr nfs::u32 kCallback = 0x558b18;          // (answer, &keep up)
constexpr nfs::u32 kLineCount = 0x558b1c;
constexpr nfs::u32 kLines = 0x558b20;             // char*[kLineCount]
constexpr nfs::u32 kButtonCount = 0x558b30;
constexpr nfs::u32 kButtons = 0x558b34;           // char*[kButtonCount]
constexpr nfs::u32 kButtonHelp = 0x558b38;        // char*[kButtonCount], or 0
constexpr nfs::u32 kFocus = 0x558b3c;             // the button highlighted, -1 for none
constexpr nfs::u32 kButtonsX = 0x558b40;          // a row's left, a column's middle
constexpr std::int32_t kMaxButtons = 10;
// A message's.
constexpr nfs::u32 kMessageOpened = 0x558b44;     // the tick it came up at
constexpr nfs::u32 kMessageTime = 0x558b48;       // the ticks it stays up, 0 for as long as it takes
constexpr nfs::u32 kMessageKeys = 0x558b4c;       // byte: the keys answer it
constexpr nfs::u32 kMessageLine = 0x661090;       // sub_444be0's one line
constexpr std::uint32_t kMessageTicks = 0x226;
// The line typed in.
constexpr nfs::u32 kReplaceOnKey = 0x558b2c;      // the first character typed replaces the text given
constexpr nfs::u32 kEditLength = 0x558b50;
constexpr nfs::u32 kEditMax = 0x558b54;           // the most characters
constexpr nfs::u32 kEditText = 0x558b58;
constexpr std::uint32_t kEditSize = 0x100;
constexpr nfs::u32 kEditSource = 0x558c58;        // the text it started from
constexpr nfs::u32 kEditMode = 0x558c5c;          // what may be typed (sub_443810), below 4
constexpr nfs::u32 kEditWidth = 0x661070;         // the field's, in pixels
// A list's.
constexpr nfs::u32 kListCount = 0x558c64;         // byte

// Where it is drawn: the box (floats), the buttons' and the entries' boxes
// (8 bytes each, int16), and how far it has faded in.
constexpr nfs::u32 kRect = 0x661050;
constexpr nfs::u32 kTop = 0x0;
constexpr nfs::u32 kLeft = 0x4;
constexpr nfs::u32 kBottom = 0x8;
constexpr nfs::u32 kRight = 0xc;
constexpr nfs::u32 kButtonBoxes = 0x661000;
constexpr nfs::u32 kBoxY = 0;
constexpr nfs::u32 kBoxX = 2;
constexpr nfs::u32 kBoxWidth = 4;
constexpr nfs::u32 kBoxHeight = 6;
constexpr nfs::u32 kBoxSize = 8;
constexpr nfs::u32 kFade = 0x661064;              // 0 to 255

// Its art: the front end's shapes, by name (sub_4d5db0), and in a race's
// pause screens parts of their art, a texture.

// The help box over a dialog (sub_445ac0), for a button or an entry.
constexpr nfs::u32 kHelpText = 0x558b0c;

// The menus'.
constexpr nfs::u32 kInRace = 0x559234;            // the menus are a race's pause screens
constexpr nfs::u32 kMouseX = 0x55e5e4;
constexpr nfs::u32 kMouseY = 0x55e5e8;
constexpr nfs::u32 kTicks = 0x79c1a4;             // the game's clock
constexpr nfs::u32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kNoTranslucency = 0x02;      // no fades
// The depth range 2D is drawn in.

// Text.
constexpr std::uint32_t kFont = 0x12;

// Keys, as sub_451960 gives them, and the answers they make.

// Sounds, sub_4181d0(sound, volume).

// The game's.
constexpr nfs::u32 kMinus15 = 0x538ae8;           // float -15
constexpr nfs::u32 kMinus30 = 0x538aec;           // float -30
constexpr nfs::u32 kMemset = 0x4e0640;
constexpr nfs::u32 kStrncpy = 0x4e0e30;
constexpr nfs::u32 kSetMousePosition = 0x49bfc0;

// This file's own, called by address where the caller goes on with the registers they leave.
constexpr nfs::u32 kLinesBox = 0x443a90;
constexpr nfs::u32 kPlaceButtons = 0x443d20;

const fe::GuestString kWidest{ 0x538b2c, "W" };
const fe::GuestString kCursor{ 0x538b04, "_" };
const fe::GuestString kPopu{ 0x538b84, "popu" };
const fe::GuestString kBpop{ 0x538b8c, "bpop" };
const fe::GuestString kYpop{ 0x538b94, "ypop" };
const fe::GuestString kGlit{ 0x538b9c, "glit" };
const fe::GuestString kRlit{ 0x538ba4, "rlit" };
const fe::GuestString kYel1{ 0x538bac, "yel1" };

nfs::u32 reg(std::int32_t value)
{
    return nfs::u32(value);
}

std::int32_t sdword(const Guest& guest, nfs::u32 address)
{
    return std::int32_t(guest.dword(address));
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

std::int32_t textWidth(Guest& guest, nfs::u32 text)
{
    return fe::textWidth(guest, text, kFont);
}

void setFont(Guest& guest, nfs::ThreadState& ts)
{
    fe::direct<0x452100, port::text::selectFontOf>(guest.app(), ts, kFont);
}

/* A button's box or a list entry's. */
struct Box
{
    std::int32_t y, x, width, height;
};

nfs::u32 boxAt(nfs::u32 boxes, std::int32_t index)
{
    return boxes + nfs::u32(index) * kBoxSize;
}

Box box(const Guest& guest, nfs::u32 boxes, std::int32_t index)
{
    const nfs::u32 at = boxAt(boxes, index);
    return { std::int16_t(guest.word(at + kBoxY)), std::int16_t(guest.word(at + kBoxX)),
             std::int16_t(guest.word(at + kBoxWidth)), std::int16_t(guest.word(at + kBoxHeight)) };
}

/* ---------------------------------------------------------------------------
 * The layout
 * ------------------------------------------------------------------------- */

/* The dialog's box, kRect, round its middle. */
void setDialogRect(Guest& guest, Arith& x87, const Float& middleX, const Float& middleY, const Float& halfWidth,
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
nfs::u32 pointAt(Guest& guest, nfs::u32 boxes, std::int32_t count, std::int32_t index)
{
    if (index < 0 || index >= count)
        return reg(index);
    const Box b = box(guest, boxes, index);
    guest.setDword(kMouseX, reg(b.x + b.width - 2));
    guest.setDword(kMouseY, reg(b.y + b.height - 2));
    return fe::direct<kSetMousePosition, frontend::screen::setPointer>(guest.app(), guest.ts(), guest.dword(kMouseX),
                                                                       guest.dword(kMouseY));
}

/* ---------------------------------------------------------------------------
 * The dialog up, whatever its kind
 * ------------------------------------------------------------------------- */

// A thunk's stack argument, counted from the first.
nfs::u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, nfs::u32 index)
{
    return app->getMemory<nfs::u32>(ts.sp + 4 + 4 * index);
}

const fe::GuestString kEditStrings[] = { kWidest, kCursor };

}

/* ---------------------------------------------------------------------------
 * The state
 * ------------------------------------------------------------------------- */

/* sub_4446c0: the button highlighted. */
nfs::u32 frontend::dialogs::focusedButton(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kFocus);
}

/* sub_444c10: the line typed in. */
nfs::u32 frontend::dialogs::editText(App*, ThreadState&)
{
    return kEditText;
}

/* sub_445f00: the dialog shown at once, faded in. */
nfs::u32 frontend::dialogs::fadedIn(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kFade, 255);
    return 0;  // eax is left as it came: the thunk's
}

/* ---------------------------------------------------------------------------
 * The layout
 * ------------------------------------------------------------------------- */

/* sub_443a90: the box round `count` lines of text, centred on the screen, in
 * floats at `rect`: as wide as the widest line with a margin (190 pixels at
 * least, but in a race), 30 pixels a line and a margin, 45 more for a row of
 * buttons below and 45 for a field to type in.  The dialog starts to fade
 * in. */
nfs::u32 frontend::dialogs::linesBox(App* app, ThreadState& ts, u32 count, u32 lines, u32 rowBelow,
                                       u32 fieldBelow, u32 rect)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x24);
    setFont(guest, ts);
    std::int32_t width = 0;
    for (std::int32_t i = 0; i < std::int32_t(count); ++i)
        width = std::max(width, textWidth(guest, guest.dword(lines + 4 * nfs::u32(i))));
    Arith x87(guest);
    if (inRace(guest))
        width = x87.truncated(x87.add(Arith::integer(width), Float(63.5)));
    else
        width += 51;
    width += 30;
    std::int32_t height = 30 * std::int32_t(count) + 30;
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
    guest.setFloat(rect + kTop, float(Arith::integer(y)));
    guest.setFloat(rect + kLeft, float(Arith::integer(x)));
    guest.setFloat(rect + kRight, float(Arith::integer(x + width)));
    guest.setFloat(rect + kBottom, float(Arith::integer(y + height)));
    guest.setDword(kFade, 0);
    if ((guest.byte(kDisplayFlags) & kNoTranslucency) != 0)
        guest::direct<0x445f00, fadedIn>(app, ts);
    return (nfs::u32(y + height) & ~0xff00u) | (nfs::u32(guest.byte(kDisplayFlags)) << 8);
}

/* sub_443c00: the box round a row of buttons, centred on the screen, in
 * floats at `rect`: the buttons' texts 15 pixels apart and 60 high, 190
 * pixels wide at least but in a race.  The dialog starts to fade in. */
nfs::u32 frontend::dialogs::buttonRowBox(App* app, ThreadState& ts, u32 count, u32 buttons, u32 rect)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x18);
    setFont(guest, ts);
    std::int32_t width = 15;
    for (std::int32_t i = 0; i < std::int32_t(count); ++i)
    {
        const nfs::u32 text = guest.dword(buttons + 4 * nfs::u32(i));
        if (text != 0)
            width += textWidth(guest, text) + 15;
    }
    const std::int32_t height = 60;
    if (width >= 640)
        width = 639;
    if (!inRace(guest) && width < 190)
        width = 190;
    const std::int32_t x = 320 - width / 2, y = 240 - height / 2;
    guest.setFloat(rect + kLeft, float(Arith::integer(x)));
    guest.setFloat(rect + kRight, float(Arith::integer(x + width)));
    guest.setFloat(rect + kTop, float(Arith::integer(y)));
    guest.setFloat(rect + kBottom, float(Arith::integer(y + height)));
    guest.setDword(kFade, (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? 255 : 0);
    // Its eax: the box's bottom with the display's flags in ah.
    return (reg(y + height) & ~0xff00u) | (nfs::u32(guest.byte(kDisplayFlags)) << 8);
}

/* sub_443d20: the buttons' boxes (kButtonBoxes) in the dialog's box at
 * `rect`: in a column, up from its bottom 30 pixels a button, centred on
 * kButtonsX and then lined up on the leftmost; or in a row 45 pixels above
 * its bottom, spread evenly across it from kButtonsX.  Ten buttons or more
 * get no boxes.
 *
 * Written as its code is, the registers as it leaves them, for its callers'
 * sake: each push and pop to the guest's stack, the truncations by __CHP
 * (watcom::clib::chop, every register kept, as __CHP keeps them), the text
 * width and the font the port's (as C++, every register but eax kept as the
 * game's functions keep them); what its code keeps on the x87's stack in
 * locals.  The arguments go
 * into the registers its code reads them from; its result is the eax it
 * leaves. */
nfs::u32 frontend::dialogs::placeButtons(App* app, ThreadState& ts, u32 count, u32 buttons, u32 column, u32 rect)
{
    using nfs::s32;
    auto at = [](nfs::App* app, u32 address) { return app->getMemory<u32>(address); };
    auto put = [](nfs::App* app, u32 address, u32 value) { app->getMemory<u32>(address) = value; };
    auto chop = [&](Float value) {
        // __CHP keeps every register: nothing to keep here.
        return fe::direct<0x4dfd56, watcom::clib::chop>(app, ts, double(value));
    };
    auto setWord = [&](u32 address, u32 value) { app->getMemory<nfs::u16>(address) = nfs::u16(value); };
    auto high = [&](u32 address) { return u32(s32(at(app, address)) >> 16); };  // mov, sar 0x10
    auto loadFloat = [&](u32 address) { return Float(app->getMemory<float>(address)); };
    auto fistp = [&](u32 address, double value) { put(app, address, u32(fp::toInteger<s32>(value))); };
    auto half = [](u32 value) { return u32((s32(value) - (s32(value) >> 31)) >> 1); };  // cdq, sub, sar 1

    u32 eax = count, edx = buttons, ebx = column, ecx = rect, esi = 0, edi = 0;
    // esi, edi and ebp pushed by the code: the frame (ebp is its base), not their values.
    ts.sp -= 4 * 3;
    const u32 ebp = ts.sp;
    ts.sp -= 0x14;
    esi = eax;
    put(app, ebp - 4, edx);
    edx = ebx;
    ebx = ecx;
    fe::direct<0x452100, port::text::selectFontOf>(app, ts, kFont);
    eax = at(app, kButtonsX);
    put(app, ebp - 0x10, eax);
    if (edx != 0)
    {
        // A column, up from the box's bottom.
        Float bottom = loadFloat(ecx + kBottom);
        edi = 0x280;
        bottom = fp::add(bottom, loadFloat(kMinus15));
        ecx = esi - 1;
        fistp(ebp - 8, chop(bottom));
        for (; s32(ecx) >= 0; --ecx)
        {
            if (s32(esi) >= kMaxButtons)
                continue;
            edx = at(app, ebp - 4);
            eax = at(app, ecx * 4 + edx);
            if (eax == 0)
                continue;
            ebx = at(app, ebp - 8);
            edx = kFont;
            ebx -= 0x1e;
            eax = u32(*fe::direct<0x452020, port::text::menuTextWidth>(app, ts, ts.sp - 4, eax, edx));
            put(app, ebp - 8, ebx);
            ebx = eax + 5;
            edx = u32(s32(ebx) >> 31);
            eax = half(ebx);
            edx = at(app, ebp - 0x10) - eax + 8;
            setWord(ecx * 8 + kButtonBoxes + kBoxX, edx);
            eax = high(ecx * 8 + kButtonBoxes);
            if (s32(eax) < s32(edi))
                edi = eax;
            eax = ecx * 8;
            edx = at(app, kInRace);
            setWord(eax + kButtonBoxes + kBoxWidth, ebx);
            setWord(eax + kButtonBoxes + kBoxHeight, edx != 0 ? 0x1e : 0x11);
            eax = at(app, ebp - 8);
            setWord(ecx * 8 + kButtonBoxes + kBoxY, eax);
        }
        // Lined up on the leftmost.
        for (ecx = esi - 1; s32(ecx) >= 0; --ecx)
        {
            if (s32(esi) >= kMaxButtons)
                continue;
            ebx = at(app, ebp - 4);
            eax = at(app, ecx * 4 + ebx);
            if (eax != 0)
                setWord(ecx * 8 + kButtonBoxes + kBoxX, edi);
        }
    }
    else
    {
        // A row, spread evenly across the box.
        fistp(ebp - 0x14, chop(fp::sub(loadFloat(ebx + kRight), loadFloat(ecx + kLeft))));
        eax = at(app, ebp - 0x14);
        Float bottom = loadFloat(ecx + kBottom);
        edx = u32(s32(eax) >> 31);
        ecx = esi + 1;
        {
            const s32 dividend = s32(eax), divisor = s32(ecx);  // idiv ecx
            eax = u32(dividend / divisor);
            edx = u32(dividend % divisor);
        }
        bottom = fp::add(bottom, loadFloat(kMinus15));
        bottom = fp::add(bottom, loadFloat(kMinus30));
        fistp(ebp - 8, chop(bottom));
        put(app, ebp - 0xc, eax);
        for (ecx = 0; s32(ecx) < s32(esi); ++ecx)
        {
            if (s32(esi) >= kMaxButtons)
                continue;
            edi = at(app, ebp - 4);
            eax = at(app, ecx * 4 + edi);
            if (eax == 0)
                continue;
            edx = kFont;
            eax = u32(*fe::direct<0x452020, port::text::menuTextWidth>(app, ts, ts.sp - 4, eax, edx));
            setWord(ecx * 8 + kButtonBoxes + kBoxWidth, eax);
            edx = at(app, ebp - 0xc);
            fistp(ebp - 0x14, chop(loadFloat(ebx + kLeft)));
            put(app, ebp - 0x14, at(app, ebp - 0x14) + edx);
            eax = edx * ecx;  // imul: its low 32 bits
            put(app, ebp - 0x14, at(app, ebp - 0x14) + eax);
            edx = high(ecx * 8 + kButtonBoxes + kBoxX);
            eax = half(edx);
            edx = u32(s32(edx) >> 31);
            edx = at(app, ebp - 0x14) - eax;
            edi = 0x1e;
            setWord(ecx * 8 + kButtonBoxes + kBoxHeight, edi);
            eax = edx + 8;
            setWord(ecx * 8 + kButtonBoxes + kBoxX, eax);
            eax = at(app, ebp - 8);
            setWord(ecx * 8 + kButtonBoxes + kBoxY, eax);
        }
        if (gameSettings(app).language == 3 && esi == 2)
        {
            // Two buttons five pixels further apart each.
            edx = (edx & 0xffff0000u) | app->getMemory<nfs::u16>(kButtonBoxes + kBoxSize + kBoxX);
            eax = (eax & 0xffff0000u) | app->getMemory<nfs::u16>(kButtonBoxes + kBoxX);
            edx += 5;
            eax -= 5;
            setWord(kButtonBoxes + kBoxSize + kBoxX, edx);
            setWord(kButtonBoxes + kBoxX, eax);
        }
    }
    ts.sp = ebp + 4 * 3;
    return eax;
}

/* ---------------------------------------------------------------------------
 * The pointer
 * ------------------------------------------------------------------------- */

/* sub_444570: the pointer on a button. */
nfs::u32 frontend::dialogs::pointAtButton(App* app, ThreadState& ts, u32 index)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return pointAt(guest, kButtonBoxes, sdword(guest, kButtonCount), std::int32_t(index));
}

/* ---------------------------------------------------------------------------
 * A question
 * ------------------------------------------------------------------------- */

/* sub_4446d0: a question opened: its lines over its buttons, the buttons in
 * a row or, with `column`, one above the other; the box round both, the
 * pointer on the button highlighted.  Nothing without lines or buttons, or
 * with ten buttons or more. */
nfs::u32 frontend::dialogs::openQuestion(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 buttonCount,
                                           u32 buttons, u32 buttonHelp, u32 focus, u32 column,
                                           u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x48);
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0 || buttonCount == 0 || buttons == 0 || std::int32_t(buttonCount) >= kMaxButtons)
        return lineCount;  // eax as it came
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kLines, lines);
    guest.setDword(kKind, kQuestion);
    guest.setDword(kButtonCount, buttonCount);
    guest.setByte(kShown, 1);
    guest.setDword(kButtons, buttons);
    guest.setDword(kButtonHelp, buttonHelp);
    setFirstFocus(guest, std::int32_t(focus), std::int32_t(buttonCount));

    const nfs::u32 linesRect = guest.frame() + 0x10, buttonsRect = guest.frame();  // [ebp - 0x38], [ebp - 0x48]
    if (column != 0)
    {
        guest::direct<0x443a90, linesBox>(app, ts, guest.dword(kLineCount), guest.dword(kLines), 0, 0, linesRect);
        guest::direct<0x443a90, linesBox>(app, ts, guest.dword(kButtonCount), guest.dword(kButtons), 0, 0, buttonsRect);
    }
    else
    {
        guest::direct<0x443a90, linesBox>(app, ts, guest.dword(kLineCount), guest.dword(kLines), 1, 0, linesRect);
        guest::direct<0x443c00, buttonRowBox>(app, ts, guest.dword(kButtonCount), guest.dword(kButtons), buttonsRect);
    }

    // The two boxes put together round the lines' middle, as wide as the wider.
    Arith x87(guest);
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

    guest::direct<0x443d20, placeButtons>(app, ts, guest.dword(kButtonCount), guest.dword(kButtons), column, kRect);
    guest::direct<0x444570, pointAtButton>(app, ts, guest.dword(kFocus));
    guest.setDword(kCallback, callback);
    return 0;
}

/* ---------------------------------------------------------------------------
 * A message
 * ------------------------------------------------------------------------- */

/* sub_444a80: a message opened: lines and no buttons, up for `time` ticks (a
 * negative time as long; 0 until a key answers it, whatever `keys` says). */
nfs::u32 frontend::dialogs::openMessage(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 time, u32 keys,
                                          u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0)
        return lineCount;  // eax as it came
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kKind, kMessage);
    guest.setDword(kLines, lines);
    guest.setByte(kShown, 1);
    guest.setDword(kMessageOpened, guest.dword(kTicks));
    if (std::int32_t(time) <= 0)
        time = 0 - time;
    guest.setDword(kMessageTime, time);
    guest.setByte(kMessageKeys, std::int32_t(time) > 0 ? std::uint8_t(keys) : 1);
    const nfs::u32 result =
        guest::direct<0x443a90, linesBox>(app, ts, guest.dword(kLineCount), guest.dword(kLines), 0, 0, kRect);
    guest.setDword(kCallback, callback);
    return result;
}

/* sub_444be0: a message of one line of the language's text, up for 550
 * ticks or until a key answers it. */
nfs::u32 frontend::dialogs::messageById(App* app, ThreadState& ts, u32 id)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setDword(kMessageLine,
                   u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, std::int32_t(id), 0)));
    return guest::direct<0x444a80, openMessage>(app, ts, 1, kMessageLine, kMessageTicks, 1, 0);
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
nfs::u32 frontend::dialogs::openEdit(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 buttonCount,
                                       u32 buttons, u32 focus, u32 source, u32 maxLength, u32 mode,
                                       u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x5c);
    guest.setDword(kHelpText, 0);
    guest.setDword(kButtonHelp, 0);
    if (lineCount == 0 || lines == 0 || sdword(guest, kEditMode) >= 4 || buttonCount == 0 || buttons == 0
        || std::int32_t(buttonCount) >= kMaxButtons)
        return lineCount;  // eax as it came
    guest.setDword(kLineCount, lineCount);
    guest.setDword(kLines, lines);
    guest.setDword(kButtonCount, buttonCount);
    guest.setDword(kButtons, buttons);
    guest.setDword(kKind, kEdit);
    guest.setByte(kShown, 1);
    guest.setDword(kReplaceOnKey, 1);
    setFirstFocus(guest, std::int32_t(focus), std::int32_t(buttonCount));

    const nfs::u32 linesRect = guest.frame() + 0x10, buttonsRect = guest.frame();  // [ebp - 0x4c], [ebp - 0x5c]
    guest::direct<0x443a90, linesBox>(app, ts, guest.dword(kLineCount), guest.dword(kLines), 1, 1, linesRect);
    guest::direct<0x443c00, buttonRowBox>(app, ts, guest.dword(kButtonCount), guest.dword(kButtons), buttonsRect);

    Arith x87(guest);
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

    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kEditText, 0, kEditSize);
    guest.setDword(kEditSource, source);
    guest.setDword(kEditMode, mode);
    guest.setDword(kEditMax, maxLength != 0 && maxLength < kEditSize ? maxLength : kEditSize - 1);
    const nfs::u32 allowed =
        fe::direct<0x443860, frontend::text::lineAllowed>(app, ts, guest.dword(kEditSource), guest.dword(kEditMode));
    if ((allowed & 0xff) != 0)
    {
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), kEditText, guest.dword(kEditSource), guest.dword(kEditMax));
        guest.setDword(kEditLength, guest.length(kEditText));
    }
    else
        guest.setDword(kEditLength, 0);

    // The field: the most characters, the widest of them, and the cursor.
    setFont(guest, ts);
    const float textWidthF = float(x87.mul(Arith::integer(textWidth(guest, kWidest.address)),
                                           Arith::integer(sdword(guest, kEditMax))));
    const Float room = x87.add(x87.add(Arith::integer(textWidth(guest, kCursor.address)), Float(textWidthF)), Float(5.0));
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
        if (!x87.below(Float(boxWidth), Arith::integer(480)))
            boxWidth = float(x87.add(Arith::integer(480), Float(-1.0)));
        const Float halfWidth = x87.mul(Float(boxWidth), Float(0.5));
        guest.setFloat(kRect + kLeft, float(x87.sub(Float(middleXF), halfWidth)));
        guest.setFloat(kRect + kRight, float(x87.add(halfWidth, Float(middleXF))));
    }

    guest.setDword(kCallback, callback);
    guest::direct<0x443d20, placeButtons>(app, ts, guest.dword(kButtonCount), guest.dword(kButtons), 0, kRect);
    return guest::direct<0x444570, pointAtButton>(app, ts, guest.dword(kFocus));
}

/* Each thunk: the game's function, what its ret pops, its frame (the
 * registers it pushes, its locals), the strings it hands to the game, and the
 * registers it restores. */
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::kRestoreEdi;
constexpr std::uint8_t kRestoreNone = 0;

/* Native whole: a question, a message, a line to type in as the main menu
 * opens them, and what they open with. */
namespace frontend
{

/* The button highlighted, native whole: the main menu's question asks it. */
nfs::u32 dialogFocusedButton(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4446c0", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::focusedButton(app, ts); });
}

nfs::u32 dialogLinesBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443a90", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const nfs::u32 count = args.eax, lines = args.edx, rowBelow = args.ebx, fieldBelow = args.ecx;
    const nfs::u32 rect = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return dialogs::linesBox(app, ts, count, lines, rowBelow, fieldBelow, rect); });
}

nfs::u32 dialogButtonRowBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443c00", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const nfs::u32 count = args.eax, buttons = args.edx, rect = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::buttonRowBox(app, ts, count, buttons, rect); });
}

/* sub_443d20 as its code is (dialogs::placeButtons): its ret pops the return address. */
nfs::u32 dialogPlaceButtons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    try
    {
        args.eax = dialogs::placeButtons(app, ts, args.eax, args.edx, args.ebx, args.ecx);
        ts.sp += 4;
    }
    catch (const fe::Terminated&)
    {
    }
    return args.eax;
}

nfs::u32 dialogPointAtButton(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_444570", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const nfs::u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::pointAtButton(app, ts, index); });
}

nfs::u32 dialogOpenQuestion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4446d0", 0x10, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const nfs::u32 lineCount = args.eax, lines = args.edx, buttonCount = args.ebx, buttons = args.ecx;
    const nfs::u32 buttonHelp = stackArgument(app, ts, 0), column = stackArgument(app, ts, 2);
    const nfs::u32 focus = stackArgument(app, ts, 1);
    const nfs::u32 callback = stackArgument(app, ts, 3);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return dialogs::openQuestion(app, ts, lineCount, lines, buttonCount, buttons, buttonHelp, focus, column,
                                     callback);
    });
}

nfs::u32 dialogOpenMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_444a80", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const nfs::u32 lineCount = args.eax, lines = args.edx, time = args.ebx, keys = args.ecx;
    const nfs::u32 callback = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return dialogs::openMessage(app, ts, lineCount, lines, time, keys, callback); });
}

nfs::u32 dialogMessageById(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_444be0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const nfs::u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::messageById(app, ts, id); });
}

nfs::u32 dialogEditText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_444c10", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::editText(app, ts); });
}

nfs::u32 dialogOpenEdit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_444c20", 0x14, 0, kEditStrings, std::size(kEditStrings), kRestoreEsi | kRestoreEdi };
    const nfs::u32 lineCount = args.eax, lines = args.edx, buttonCount = args.ebx, buttons = args.ecx;
    const nfs::u32 focus = stackArgument(app, ts, 0);
    const nfs::u32 source = stackArgument(app, ts, 1), maxLength = stackArgument(app, ts, 2);
    const nfs::u32 mode = stackArgument(app, ts, 3), callback = stackArgument(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return dialogs::openEdit(app, ts, lineCount, lines, buttonCount, buttons, focus, source, maxLength, mode,
                                 callback);
    });
}

nfs::u32 dialogFadedIn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_445f00", 0, 0, nullptr, 0, kRestoreNone };
    const nfs::u32 eax = args.eax;  // left as it came
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return dialogs::fadedIn(app, ts), eax; });
}

}  // namespace frontend

}
