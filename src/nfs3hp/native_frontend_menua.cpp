#include "native_frontend_menua.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_frontend_menud.h"
#include "native_frontend_menue.h"
#include "native_controls_devices.h"
#include "native_controls_input.h"
#include "native_ea_memory.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_glue.h"
#include "native_frontend_loop.h"
#include "native_frontend_neta.h"
#include "native_frontend_pause.h"
#include "native_frontend_screen.h"
#include "native_frontend_text.h"
#include "native_frontend_wina.h"
#include "native_graphics.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_draw2d.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_menus.h"
#include "port_shapes.h"
#include "port_snd_stream_calls.h"
#include "port_sound_effects.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <iterator>

/* What the menus' screens lean on, native whole:
 *
 *   the controls' list        CONTROLS: a binding's label, the list that draws
 *                             them, its keys, and what a joystick, a key or the
 *                             axes' rest positions give it;
 *   the colour picker         a car's colour: the triangle, the hue bar, their
 *                             markers and buttons, mouse and keys;
 *   the network lobby's panel the chat panel beside a network screen and over
 *                             the race's pause, its messages and its line typed;
 *   what may be typed         the character classes of the dialogs' edit lines;
 *   the screen modes' list    the Graphics menu's Screen Size;
 *   the race's pause menu     its start and its pass of input and transitions.
 *
 * Everything else is called: the typed cores as C++, the rest at its address.
 *
 * Plan step 5: each function a typed core in frontend::menua (native_frontend_menua.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and an adapter under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

// The port's own, in nfs3hp_main.cpp.
x86::reg32 shownDepth(x86::reg32 depth);
void dynamicFullscreenLabel(win32::WinApplication* app, x86::reg32 entry);


namespace
{

using fe::Guest;
using fe::GuestString;
using x86::reg32;
using s32 = std::int32_t;

/* ---------------------------------------------------------------------------
 * The game's, by address
 * ------------------------------------------------------------------------- */

constexpr reg32 kDrawText = 0x452190;
constexpr reg32 kDrawShape = 0x4d7990;
constexpr reg32 kDrawArtShape = 0x4d7160;
constexpr reg32 kDrawTurnedShape = 0x4d6730;
constexpr reg32 kDrawTexturedQuad = 0x4db000;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kPlayEffect = 0x4181d0;
constexpr reg32 kSetPointer = 0x49bfc0;
constexpr reg32 kHsvToRgb = 0x49ca50;
constexpr reg32 kSetRenderState = 0x431900;

/* A float of 1, as the turned shapes' scale goes on the stack. */
constexpr reg32 kFloatOne = 0x3f800000;

constexpr reg32 kInRace = 0x559234;
constexpr reg32 kPointerX = 0x55e5e4;
constexpr reg32 kPointerY = 0x55e5e8;
constexpr reg32 kDisplayFlags = 0x7a3a58;        // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr reg32 kFromMouse = fe::kInputFromMouse;

constexpr reg32 kMenuFont = 0x12;
constexpr reg32 kColourChosen = 0xffe440;
constexpr reg32 kColourPlain = 0xff649dfd;
constexpr std::uint32_t kEffectVolume = 0x7f;

constexpr reg32 u(s32 value) { return reg32(value); }

s32 s16At(const Guest& guest, reg32 address) { return std::int16_t(guest.word(address)); }
s32 s32At(const Guest& guest, reg32 address) { return s32(guest.dword(address)); }

/* The sign-extended low byte: the kind of a binding, as the game tests it. */
s32 lowByte(reg32 value) { return std::int8_t(value & 0xff); }

/* A value halved towards zero. */
s32 halved(s32 value) { return s32(reg32(value) - reg32(value >> 31)) >> 1; }

s32 times(s32 a, s32 b) { return s32(reg32(a) * reg32(b)); }
s32 minus(s32 a, s32 b) { return s32(reg32(a) - reg32(b)); }

reg32 withLowByte(reg32 value, std::uint8_t low) { return (value & 0xffffff00) | low; }
reg32 withLowWord(reg32 value, std::uint16_t low) { return (value & 0xffff0000) | low; }

/* The game's functions the screens here call most, as C++ (guest::direct); a core of the port's
 * that takes `esp` gets the esp it would have from the game's call: below the arguments pushed and
 * the return address. */

/* Whether an item is shown.  The game's code has the widget's kind in edx, which the function does
 * not read and the code after it does: it is left there. */
reg32 shownNow(Guest& guest, x86::CPU& cpu, reg32 item, reg32 widget)
{
    guest.leaveEdx(widget);
    return fe::direct<0x44a660, frontend::loop::shownNow>(guest.app(), cpu, item);
}

/* Line `number` of the language's text (edx, which only a name past the lines reads, as it is). */
reg32 languageText(Guest& guest, x86::CPU& cpu, reg32 number)
{
    return u(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), cpu, cpu.esp - 4, s32(number), cpu.edx));
}

reg32 drawMenuText(Guest& guest, x86::CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 align, reg32 colour)
{
    return u(*fe::direct<0x452190, port::text::menuTextDrawn>(guest.app(), cpu, cpu.esp - 4 - 4 * 2, text, s32(x),
                                                              s32(y), kMenuFont, s32(align), colour));
}

reg32 drawText(Guest& guest, x86::CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 colour)
{
    return drawMenuText(guest, cpu, text, x, y, 0, colour);
}

reg32 drawShape(Guest& guest, x86::CPU& cpu, s32 shape, reg32 x, reg32 y)
{
    return u(*fe::direct<0x4d7990, port::shapes::drawShapeOf>(guest.app(), cpu, cpu.esp - 4, shape, s32(x), s32(y)));
}

reg32 drawTurnedShape(Guest& guest, x86::CPU& cpu, s32 shape, reg32 x, reg32 y, reg32 angle)
{
    return u(*fe::direct<0x4d6730, port::buttons::turnedShape>(guest.app(), cpu, cpu.esp - 4 - 4 * 2, shape, s32(x),
                                                               s32(y), 0, s32(angle), kFloatOne));
}

reg32 playEffect(Guest& guest, x86::CPU& cpu, reg32 effect)
{
    return u(*fe::direct<0x4181d0, port::playMenuEffectOf>(guest.app(), cpu, cpu.esp - 4, s32(effect),
                                                           s32(kEffectVolume)));
}

void setPointer(Guest& guest, x86::CPU& cpu, reg32 x, reg32 y)
{
    fe::direct<0x49bfc0, frontend::screen::setPointer>(guest.app(), cpu, x, y);
}

/* The rectangle from colourA at its top to colourB at its bottom. */
reg32 shadedRectangle(Guest& guest, x86::CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 colourA,
                      reg32 colourB)
{
    const std::uint32_t colours[4] = { colourA, colourA, colourB, colourB };
    return u(*fe::direct<0x4d81b0, port::draw2d::shadedRectangle>(guest.app(), cpu, cpu.esp - 4 - 4 * 4, s32(left),
                                                                  s32(top), s32(right), s32(bottom), colours));
}

reg32 shadedRectangle(Guest& guest, x86::CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 colour)
{
    return shadedRectangle(guest, cpu, left, top, right, bottom, colour, colour);
}

reg32 filledRectangle(Guest& guest, x86::CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour)
{
    return u(*fe::direct<0x4d8910, port::draw2d::filled>(guest.app(), cpu, cpu.esp - 4 - 4, s32(x), s32(y), s32(width),
                                                         s32(height), colour));
}

reg32 frameRectangle(Guest& guest, x86::CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour)
{
    return u(*fe::direct<0x4d8930, port::draw2d::outline>(guest.app(), cpu, cpu.esp - 4 - 4, s32(x), s32(y),
                                                          s32(width), s32(height), colour));
}

reg32 shapeNamed(Guest& guest, x86::CPU& cpu, reg32 name)
{
    return u(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(guest.app(), cpu, cpu.esp - 4, name));
}

void renderState(Guest& guest, x86::CPU& cpu, reg32 state, reg32 value)
{
    fe::direct<0x431900, graphics::sendState>(guest.app(), cpu, state, value);
}

bool dialogShown(Guest& guest, x86::CPU& cpu)
{
    return fe::direct<0x4438b0, port::dialogs::dialogShownOf>(guest.app(), cpu) != 0;
}

reg32 readJoystick(Guest& guest, x86::CPU& cpu, reg32 device, reg32 state)
{
    return *fe::direct<0x4358f0, controls::readState>(guest.app(), cpu, device, state);
}

reg32 runSystemTasks(Guest& guest, x86::CPU& cpu)
{
    return u(*fe::direct<0x4e7630, port::stream::runDueTasks>(guest.app(), cpu, cpu.esp - 4, 0u));
}

reg32 controlsFor(Guest& guest, x86::CPU& cpu)
{
    return fe::direct<0x4904d0, controls::controlSets>(guest.app(), cpu, gameSettings(guest).playMode);
}

/* The same by address, for the last call of a function that leaves the registers as the call
 * leaves them: its arguments' registers, and what the callee changes in them. */
reg32 drawTextByAddress(Guest& guest, reg32 text, reg32 x, reg32 y, reg32 colour)
{
    return guest.call(kDrawText, fe::args({ text, x, y, kMenuFont }, { 0, colour }));
}

reg32 drawShapeByAddress(Guest& guest, s32 shape, reg32 x, reg32 y)
{
    return guest.call(kDrawShape, fe::args({ u(shape), x, y }));
}

reg32 playEffectByAddress(Guest& guest, reg32 effect)
{
    return guest.call(kPlayEffect, fe::args({ effect, kEffectVolume }));
}

reg32 setPointerByAddress(Guest& guest, reg32 x, reg32 y)
{
    return guest.call(kSetPointer, fe::args({ x, y }));
}

/* ---------------------------------------------------------------------------
 * The controls' list
 * ------------------------------------------------------------------------- */

/* A joystick as sub_4358f0 reads it into a record of the caller's. */
namespace joystick
{
constexpr reg32 kButtons = 0x30;   // a byte each, 32
constexpr reg32 kAxes = 0x50;      // int32 each, 8
constexpr reg32 kAnyHeld = 0x70;
constexpr reg32 kHats = 0x74;      // int32 each, 4
}

// The devices: a record of 0x88 for each, its first word nonzero when it is on,
// which of its axes count at +0x40.
constexpr reg32 kAxisRest = 0x5f3d50;            // int32[8] for each device

// The list's layout, from sub_43d310.
constexpr reg32 kRowHeight = 0x555608;
constexpr reg32 kPaneGap = 0x5f3da0;             // from one player's column to the other's
constexpr reg32 kColumnLeft = 0x5f3d9c;
constexpr reg32 kBindingWidth = 0x5f3d98;        // from a binding's label to what it is
constexpr reg32 kPointerPlaced = 0x555640;       // the keys have moved the pointer already
constexpr reg32 kHeardBinding = 0x5f3d94;        // a binding the joystick gave, not yet taken
constexpr reg32 kBindingBefore = 0x5f3d90;       // the binding being changed, for Escape
constexpr reg32 kBlink = 0x555644;
constexpr reg32 kLabelGaps = 0x55560c;           // int32 for each row: from a row's left to its label
constexpr reg32 kRowShape = 0x5f3daa;            // int16, con2
constexpr reg32 kRowLitShape = 0x5f3da8;         // int16, con1
constexpr reg32 kBindingIcons = 0x5f3d38;        // int16 for each icon
constexpr reg32 kHatArrow = 0x5f3d46;            // int16
constexpr reg32 kButtonsTexture = 0x749a38;

// What the list's item keeps: the box the pointer has it by, the row a binding
// is being changed in (-1, none).
constexpr reg32 kListWidget = 0x14;
constexpr reg32 kItemFlags = 0x05;
constexpr std::uint8_t kCapturingKeys = 0x04;
constexpr std::uint8_t kChanged = 0x10;
constexpr reg32 kItemX = 0x06;
constexpr reg32 kItemY = 0x08;
constexpr reg32 kItemBoxY = 0x1a;                // int16; the rows from here
constexpr reg32 kItemBoxX = 0x1c;
constexpr reg32 kItemBoxHeight = 0x1e;           // int16: the second column's distance, with one player
constexpr reg32 kItemBoxWidth = 0x20;
constexpr reg32 kItemChanging = 0x3c;            // int16

constexpr std::int32_t kRowsEachPlayer = 13;

// Keys.
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr std::uint16_t kKeyEscape = 0x1b;
constexpr std::uint16_t kKeyTab = 0x09;
constexpr std::uint16_t kKeyBackspace = 0x08;
constexpr std::uint16_t kKeyDelete = 0x5300;
constexpr std::uint16_t kKeyUp = 0x4800;
constexpr std::uint16_t kKeyPageUp = 0x4900;
constexpr std::uint16_t kKeyLeft = 0x4b00;
constexpr std::uint16_t kKeyRight = 0x4d00;
constexpr std::uint16_t kKeyDown = 0x5000;
constexpr std::uint16_t kKeyPageDown = 0x5100;

// A binding: its kind in the low byte, the device in the top four bits.
constexpr s32 kBindAxis = 1;
constexpr s32 kBindButton = 2;
constexpr s32 kBindHat = 3;
constexpr s32 kBindKey = 4;


}  // namespace

/* sub_43be40: what a joystick is doing, as a binding, for the list waiting for
 * one: a button pressed (waited for to come up), a hat pushed, an axis moved
 * far from where it rested; 0 for nothing.  The first device with something. */
x86::reg32 frontend::menua::pollJoystickBinding(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x98);
    const reg32 state = guest.frame();
    reg32 binding = 0;
    for (reg32 device = 0; s32(device) < s32(gameSettings(guest).deviceCount) && lowByte(binding) == 0; ++device)
    {
        if (readJoystick(guest, cpu, device, state) == 0 || gameSettings(guest).devices[device].kind == 0)
            continue;
        if (guest.dword(state + joystick::kAnyHeld) != 0)
        {
            for (reg32 button = 0; button < 32; ++button)
            {
                if (guest.byte(state + joystick::kButtons + button) != 0)
                {
                    binding = (button << 8) + (device << 28) + kBindButton;
                    break;
                }
            }
            // (The game's code has the state in edx for the tasks too, which they do not read; it is still there
            // for the joystick.)
            do
                runSystemTasks(guest, cpu);
            while (readJoystick(guest, cpu, device, state) != 0 && guest.dword(state + joystick::kAnyHeld) != 0);
        }
        for (reg32 hat = 0; hat < 4 && lowByte(binding) == 0; ++hat)
        {
            const reg32 direction = guest.dword(state + joystick::kHats + 4 * hat);
            if (direction != 0)
                binding = (hat << 20) + (device << 28) + (direction << 8) + kBindHat;
        }
        for (reg32 axis = 0; axis < 8 && lowByte(binding) == 0; ++axis)
        {
            if ((gameSettings(guest).devices[device].axisMask & (reg32(1) << axis)) == 0)
                continue;
            s32 position = s32At(guest, state + joystick::kAxes + 4 * axis) >> 8;
            if (position < 0)
                position = 0;
            // (The table has room for two devices; past them it is the list's other numbers.)
            const s32 rest = s32At(guest, kAxisRest + (device << 5) + 4 * axis);
            const s32 moved = minus(position, rest);
            const reg32 base = (axis << 24) + (device << 28);
            if (moved > 0x40)
                binding = base + (rest < 0x40 ? 0xff01 : 0x80ff01);
            else if (moved < -0x40)
                binding = base + (rest > 0xc0 ? 0xff0001 : 0x7f0001);
        }
    }
    return binding;
}

namespace
{

/* An icon of the race's buttons' texture, for what an axis is bound to: its
 * size on the screen and where it is, as floats' bits. */
struct AxisIcon
{
    reg32 width, height;
    reg32 left, top, right, bottom;
};
constexpr AxisIcon kAxisIcons[] = {
    { 0x0c, 0x09, 0x3f2c0000, 0x3f2e0000, 0x3f380000, 0x3f370000 },
    { 0x0c, 0x09, 0x3f390000, 0x3f2e0000, 0x3f450000, 0x3f370000 },
    { 0x12, 0x09, 0x3f590000, 0x3f2e0000, 0x3f6b0000, 0x3f370000 },
    { 0x12, 0x09, 0x3f460000, 0x3f2e0000, 0x3f580000, 0x3f370000 },
    { 0x09, 0x12, 0x3f760000, 0x3f250000, 0x3f7f0000, 0x3f370000 },
    { 0x09, 0x12, 0x3f6c0000, 0x3f250000, 0x3f750000, 0x3f370000 },
    { 0x09, 0x0c, 0x3f6c0000, 0x3f180000, 0x3f750000, 0x3f240000 },
    { 0x09, 0x0c, 0x3f760000, 0x3f180000, 0x3f7f0000, 0x3f240000 },
    { 0x13, 0x16, 0x3f300000, 0x3f170000, 0x3f430000, 0x3f2d0000 },
    { 0x13, 0x16, 0x3f440000, 0x3f170000, 0x3f570000, 0x3f2d0000 },
    { 0x13, 0x16, 0x3f580000, 0x3f170000, 0x3f6b0000, 0x3f2d0000 },
    { 0x13, 0x16, 0x3f6c0000, 0x3f010000, 0x3f7f0000, 0x3f170000 },
};

constexpr GuestString kFormatDevice{ 0x537748, "%s %d" };
constexpr GuestString kFormatCharacter{ 0x537750, "%s %c" };
constexpr GuestString kDegrees{ 0x537758, "deg" };
constexpr GuestString kFormatHat{ 0x53775c, "%s %c  %d %s" };
constexpr GuestString kFormatPlayer1{ 0x53776c, "%s 1" };
constexpr GuestString kFormatPlayer2{ 0x537774, "%s 2" };
constexpr GuestString kRowShapeName{ 0x53777c, "con2" };
constexpr GuestString kRowLitShapeName{ 0x537784, "con1" };
constexpr GuestString kFormatKey{ 0x5377d8, "%c" };

/* Which of the icons an axis binding shows: by the way it moves (the middle and
 * the ends of the axis), then the axis.  Axes 0 and 6 are side to side. */
std::uint32_t axisIcon(reg32 binding, reg32 axis)
{
    const bool sideways = axis == 0 || axis == 6;
    const reg32 from = binding & 0xff0000;
    std::uint32_t icon;
    if (from == 0x800000 && (binding & 0xff00) == 0xff00)
        icon = sideways ? 0 : 6;
    else if (from == 0x7f0000 && (binding & 0xff00) == 0)
        icon = sideways ? 1 : 7;
    else if (from == 0)
        icon = sideways ? 2 : 4;
    else if (from == 0xff0000)
        icon = sideways ? 3 : 5;
    else
        icon = from == 0x800000 ? 0 : 1;
    if (axis >= 3 && axis <= 5 && icon <= 3)
    {
        // (The last two are never reached: icons 2 and 3 are the sideways axes'.)
        static const std::uint32_t kOtherAxes[] = { 9, 8, 10, 11 };
        icon = kOtherAxes[icon];
    }
    return icon;
}

}  // namespace

/* sub_43c110: a binding's two lines at a place of the list: which device, and
 * what of it.  An axis has its picture: from the race's texture in a race, a
 * shape in the menus; a hat's direction is an arrow in the menus, words in a
 * race. */
x86::reg32 frontend::menua::drawBinding(App* app, CPU& cpu, reg32, reg32 binding, reg32 left, reg32 y, reg32 chosen)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x64);
    const reg32 line = guest.frame();   // char[0x64]
    const reg32 colour = chosen != 0 ? kColourChosen : kColourPlain;
    reg32 x = left + guest.dword(kColumnLeft);
    const auto deviceLine = [&] {
        const reg32 device = (binding >> 28) + 1;
        const reg32 name = languageText(guest, cpu, 0x54);
        guest.call(kSprintf, fe::args({}, { line, kFormatDevice.address, name, device }));
        drawText(guest, cpu, line, x, y, colour);
    };
    switch (lowByte(binding))
    {
    case kBindAxis:
    {
        deviceLine();
        x += guest.dword(kBindingWidth);
        const reg32 axis = (binding << 4) >> 28;
        const std::uint32_t icon = axisIcon(binding, axis);
        const reg32 letter = axis % 3 + 0x58;
        if (guest.dword(kInRace) != 0)
        {
            const AxisIcon& picture = kAxisIcons[icon];
            const reg32 right = x + picture.width, bottom = y + picture.height;
            // By address, as the function's other last calls: what they leave in ebx, ecx and edx, it leaves.
            return guest.call(kDrawTexturedQuad,
                              fe::args({ x, y, right, y },
                                       { right, bottom, x, bottom, picture.left, picture.top, picture.right,
                                         picture.top, picture.right, picture.bottom, picture.left, picture.bottom,
                                         guest.dword(kButtonsTexture), 0xffffffff }));
        }
        const reg32 name = languageText(guest, cpu, 0x57);
        guest.call(kSprintf, fe::args({}, { line, kFormatCharacter.address, name, letter }));
        drawTextByAddress(guest, line, x, y, colour);  // by address too: the last call leaves its ecx
        return drawShapeByAddress(guest, s16At(guest, kBindingIcons + 2 * icon), x + guest.dword(kBindingWidth), y);
    }
    case kBindButton:
    {
        deviceLine();
        const reg32 button = ((binding << 4) >> 12) + 0x41;
        const reg32 width = guest.dword(kBindingWidth);
        const reg32 name = languageText(guest, cpu, 0x55);
        guest.call(kSprintf, fe::args({}, { line, kFormatCharacter.address, name, button }));
        return drawTextByAddress(guest, line, x + width, y, colour);
    }
    case kBindHat:
    {
        deviceLine();
        const reg32 eighth = ((binding << 12) >> 20) - 1;
        x += guest.dword(kBindingWidth);
        const reg32 degrees = eighth * 45 != 0 ? eighth * 45 : 360;
        const reg32 hat = ((binding << 4) >> 24) + 0x31;
        const reg32 name = languageText(guest, cpu, 0x56);
        if (guest.dword(kInRace) != 0)
        {
            guest.call(kSprintf, fe::args({}, { line, kFormatHat.address, name, hat, degrees, kDegrees.address }));
            return drawTextByAddress(guest, line, x, y, colour);
        }
        guest.call(kSprintf, fe::args({}, { line, kFormatCharacter.address, name, hat }));
        drawText(guest, cpu, line, x, y, colour);
        x += guest.dword(kBindingWidth);
        return guest.call(kDrawTurnedShape, fe::args({ u(s16At(guest, kHatArrow)), x + 0xc, y + 0xc, 0 },
                                                     { degrees, kFloatOne }));
    }
    case kBindKey:
    {
        drawText(guest, cpu, languageText(guest, cpu, 0x53), x, y, colour);
        const reg32 width = guest.dword(kBindingWidth);
        guest::direct<0x43d7f0, bindingKeyName>(app, cpu, binding, line);
        return drawTextByAddress(guest, line, x + width, y, colour);
    }
    default:
        return drawTextByAddress(guest, languageText(guest, cpu, 0x52), x, y, colour);
    }
}

namespace
{

// The rows' labels, top to bottom.
constexpr std::uint32_t kRowLabels[kRowsEachPlayer] = { 0x47, 0x46, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
                                                        0x4d, 0x4f, 0x4e, 0x51, 0x1bc, 0x50 };

/* Where the pointer goes across for the list's second column with one player,
 * or, with two, where the item has it: for the keys that move it there. */
reg32 columnX(Guest& guest, reg32 item, bool toSecond)
{
    if (gameSettings(guest).playMode == 1)
    {
        reg32 x = u(s16At(guest, item + kItemX)) + guest.dword(kColumnLeft) + guest.dword(kBindingWidth);
        if (toSecond)
            x += guest.dword(kPaneGap);
        return x - 0xa;
    }
    return u(s16At(guest, item + kItemX) + s16At(guest, item + kItemBoxHeight)) - 0xa;
}

/* The pointer put there, `first` down. */
void pointerAtColumn(Guest& guest, x86::CPU& cpu, reg32 item, reg32 first, bool toSecond)
{
    setPointer(guest, cpu, columnX(guest, item, toSecond), first);
}

/* sub_43c900: the list on every pass.  Its first pass after the arrow keys it
 * puts the pointer on the row they should reach; while a binding is being
 * changed, it asks the joystick and says so at the top; each row with its
 * bindings -- two columns with one player, for players 1 and 2 -- and its label. */
void drawControlsList(Guest& guest, x86::CPU& cpu, reg32 item, reg32 pointerWanted)
{
    const reg32 line = guest.frame();   // char[0x4c]
    s32 y = s16At(guest, item + kItemY);
    if (pointerWanted != 0)
    {
        if (guest.dword(kPointerPlaced) == 0)
        {
            const reg32 key = guest.dword(0x66464c);
            if (key == kKeyUp || key == kKeyLeft)
            {
                const s32 height = s32At(guest, kRowHeight);
                pointerAtColumn(guest, cpu, item, u(times(height, 12) + s16At(guest, item + kItemY) + halved(height)), true);
            }
            const reg32 again = guest.dword(0x66464c);
            if (again == kKeyDown || again == kKeyRight)
                pointerAtColumn(guest, cpu, item, u(s16At(guest, item + kItemY) + halved(s32At(guest, kRowHeight))), false);
        }
        guest.setDword(kPointerPlaced, 1);
    }
    else
    {
        guest.setDword(kPointerPlaced, 0);
    }

    s32 chosenRow;
    if (std::int16_t(guest.word(item + kItemChanging)) != -1)
    {
        if (lowByte(guest.dword(kHeardBinding)) == 0)
        {
            const reg32 heard = guest::direct<0x43be40, frontend::menua::pollJoystickBinding>(guest.app(), cpu);
            guest.setDword(kHeardBinding, heard);
            if (lowByte(heard) != 0)
                fe::direct<0x4cc8e0, frontend::wina::pushKey>(guest.app(), cpu, 0xd, 0x1c, 0);
        }
        const reg32 promptY = guest.dword(kInRace) != 0 ? 0x41 : 0x55;
        const reg32 prompt = languageText(guest, cpu, 0x45);
        drawMenuText(guest, cpu, prompt, 0x140, promptY, 2, kColourChosen);
        chosenRow = s16At(guest, item + kItemChanging);
    }
    else if (pointerWanted != 0)
    {
        chosenRow = s32(guest::direct<0x49c1c0, frontend::menud::pointerListLine>(guest.app(), cpu, s16At(guest, item + kItemBoxY),
                                                                                  s32At(guest, kRowHeight)));
        if (gameSettings(guest).playMode == 1)
        {
            const s32 edge = s32(u(s16At(guest, item + kItemX)) + guest.dword(kColumnLeft) + guest.dword(kBindingWidth) + 0x28);
            if (edge < s32At(guest, kPointerX))
                chosenRow += kRowsEachPlayer;
        }
    }
    else
    {
        chosenRow = -1;
    }

    const reg32 controls = controlsFor(guest, cpu);
    const reg32 title = languageText(guest, cpu, 0xc8);
    guest.call(kSprintf, fe::args({}, { line, kFormatPlayer1.address, title }));
    drawText(guest, cpu, line, u(s16At(guest, item + kItemX)) + guest.dword(kColumnLeft),
             u(s16At(guest, item + kItemY)) - 0x1e, kColourPlain);
    if (gameSettings(guest).playMode == 1)
    {
        const reg32 second = languageText(guest, cpu, 0xc8);
        guest.call(kSprintf, fe::args({}, { line, kFormatPlayer2.address, second }));
        drawText(guest, cpu, line, guest.dword(kColumnLeft) + u(s16At(guest, item + kItemX)) + guest.dword(kPaneGap),
                 u(s16At(guest, item + kItemY)) - 0x1e, kColourPlain);
    }

    for (s32 row = 0; row < kRowsEachPlayer; ++row)
    {
        const std::uint32_t label = kRowLabels[row];
        if (guest.dword(kInRace) == 0)
        {
            const reg32 shapeX = guest.dword(kLabelGaps + 4 * u(row)) + u(s16At(guest, item + kItemX));
            drawShape(guest, cpu, s16At(guest, kRowShape), shapeX, u(y));
            if (s16At(guest, item + kItemChanging) == row)
            {
                const s32 blink = s32(guest.dword(kBlink) + 1);
                guest.setDword(kBlink, u(blink));
                if (blink > 0x1e)
                    guest.setDword(kBlink, 0);
                else if (blink > 0xf)
                    drawShape(guest, cpu, s16At(guest, kRowLitShape), shapeX, u(y));
            }
        }
        reg32 labelX = u(s16At(guest, item + kItemX));
        guest::direct<0x43c110, frontend::menua::drawBinding>(guest.app(), cpu, label, guest.dword(controls + 4 * u(row)), labelX,
                                                              u(y), row == chosenRow ? 1u : 0u);
        if (gameSettings(guest).playMode == 1)
        {
            guest::direct<0x43c110, frontend::menua::drawBinding>(guest.app(), cpu, label, guest.dword(controls + 4 * u(row) + 0x34),
                                                                  labelX + guest.dword(kPaneGap), u(y),
                                                                  row + kRowsEachPlayer == chosenRow ? 1u : 0u);
        }
        if (guest.dword(kInRace) == 0)
            labelX += guest.dword(kLabelGaps + 4 * u(row)) + 0xe;
        const reg32 name = languageText(guest, cpu, label);
        drawText(guest, cpu, name, labelX, u(y), kColourPlain);
        y = s32(u(y) + guest.dword(kRowHeight));
    }
}

}  // namespace

/* sub_43c900: the list, while it is shown, its edx left 0. */
x86::reg32 frontend::menua::controlsList(App* app, CPU& cpu, reg32 item, reg32 pointerWanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x4c);
    if (shownNow(guest, cpu, item, kListWidget) != 0)
        drawControlsList(guest, cpu, item, pointerWanted);
    guest.leaveEdx(0);
    return 0;
}

/* sub_43cda0: the key just pressed as a binding, or what the joystick gave; 0
 * for none, and the keys that are the menus' own or the game's are none.  One
 * that is taken has a sound and others bound to it cleared. */
x86::reg32 frontend::menua::keyAsBinding(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    constexpr reg32 kScanCode = 0x66465c;
    constexpr reg32 kCharacterFlags = 0x564ef0;
    const s32 scan = s32At(guest, kScanCode);
    reg32 binding = 0;
    const bool reserved = scan == 0x4a || scan == 0x4e || scan == 0x37 || scan == 0x35 || (scan >= 2 && scan <= 0xb)
        || (scan >= 0x3b && scan <= 0x44) || scan == -1;
    if (!reserved && s32(character) >= 0x20
        && (guest.byte(kCharacterFlags + ((character + 1) & 0xff)) & 0x20) == 0)
    {
        const reg32 upper = guest.call(0x4eede0, fe::args({ character }));
        binding = (upper << 24) + (u(scan) << 8) + kBindKey;
    }
    if (lowByte(guest.dword(kHeardBinding)) != 0)
    {
        binding = guest.dword(kHeardBinding);
        guest.setDword(kHeardBinding, 0);
    }
    if (lowByte(binding) != 0)
    {
        playEffect(guest, cpu, 1);
        guest::direct<0x43d6a0, clearConflictingBindings>(app, cpu, binding);
    }
    return binding;
}

/* sub_43ce80: in a race, one kind of binding (digital or not) for the steering
 * and the two pedals; a binding of the other kind has a message, as the race
 * has read the kind already. */
x86::reg32 frontend::menua::warnDigitalBinding(App* app, CPU& cpu, reg32 slot, reg32 binding)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    constexpr reg32 kMessageLines = 0x5f3dac;
    if (guest.dword(kInRace) == 0)
        return binding;
    const s32 row = s32(slot) < kRowsEachPlayer ? s32(slot) : minus(s32(slot), kRowsEachPlayer);
    guest.leaveEdx(u(row));
    if (reg32(row) > 3)
        return binding;
    static const std::uint8_t kDigitalFlags[4] = { 0, 0, 1, 2 };
    const reg32& flag = gameSettings(guest).players[s32(slot) < kRowsEachPlayer ? 0 : 1].digital[kDigitalFlags[row]];
    const reg32 digital = fe::direct<0x43c000, glue::bindingIsDigital>(app, cpu, binding);
    if (digital == flag)
        return digital;
    guest.setDword(kMessageLines, fe::text(guest, 0x1f0));
    guest.setDword(kMessageLines + 4, fe::text(guest, 0x1f1));
    guest.setDword(kMessageLines + 8, fe::text(guest, 0x1f2));
    return fe::direct<0x444a80, dialogs::openMessage>(app, cpu, 3u, kMessageLines, 0x3e8u, 1u, 0u);
}

namespace
{

/* A binding being changed: Escape puts it back, any other key is the new one.
 * What the generated code leaves in edx's second byte and in ebx goes with it. */
void listKeyChanging(Guest& guest, x86::CPU& cpu, reg32 item, reg32 controls, std::uint16_t key)
{
    const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kCapturingKeys);
    const reg32 slot = controls + 4 * u(s16At(guest, item + kItemChanging));
    guest.setByte(item + kItemFlags, flags);
    guest.leaveEdx((guest.edx() & 0xffff00ff) | (reg32(flags) << 8));
    if (key == kKeyEscape)
    {
        guest.leaveEbx(withLowByte(controls, std::uint8_t(flags & ~kCapturingKeys)));
        guest.setByte(item + kItemFlags, std::uint8_t(flags & ~kCapturingKeys));
        guest.setDword(slot, guest.dword(kBindingBefore));
    }
    else
    {
        const reg32 binding = guest::direct<0x43cda0, frontend::menua::keyAsBinding>(guest.app(), cpu, u(std::int16_t(key)));
        guest.leaveEbx(binding);
        if (binding == 0)
            return;
        guest::direct<0x43ce80, frontend::menua::warnDigitalBinding>(guest.app(), cpu, u(s16At(guest, item + kItemChanging)),
                                                                     binding);
        guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
        guest.setDword(slot, binding);
    }
    guest.setWord(item + kItemChanging, 0xffff);
}

/* The arrows over the rows: the pointer put on the row they reach; whether they
 * moved it.  With one player, two columns of rows.  Its calls by address: what
 * the last leaves in edx, sub_43cf80 leaves. */
bool listKeyArrows(Guest& guest, reg32 item, std::uint16_t key, s32& row)
{
    bool moved = false;
    if (gameSettings(guest).playMode == 1)
    {
        const s32 secondColumn = s32(u(s16At(guest, item + kItemX)) + guest.dword(kPaneGap) + guest.dword(kColumnLeft));
        const reg32 pointerX = guest.dword(kPointerX);
        guest.leaveEdx(pointerX);
        if (secondColumn < s32(pointerX))
            row += kRowsEachPlayer;
        if (key == kKeyLeft && row >= kRowsEachPlayer)
        {
            playEffectByAddress(guest, 0);
            row -= kRowsEachPlayer;
            moved = true;
        }
        if (key == kKeyRight && row < kRowsEachPlayer)
        {
            playEffectByAddress(guest, 0);
            row += kRowsEachPlayer;
            moved = true;
        }
        if (key == kKeyDown)
        {
            ++row;
            playEffectByAddress(guest, 0);
            if (row < kRowsEachPlayer * 2)
                moved = true;
        }
        if (key == kKeyUp)
        {
            playEffectByAddress(guest, 0);
            if (row != 0)
            {
                --row;
                moved = true;
            }
        }
        if (moved)
        {
            const s32 height = s32At(guest, kRowHeight);
            const bool second = row >= kRowsEachPlayer;
            const reg32 columnRow = u(second ? row - kRowsEachPlayer : row);
            setPointerByAddress(guest, columnX(guest, item, second),
                                columnRow * u(height) + u(s16At(guest, item + kItemY)) + u(halved(height)));
        }
        return moved;
    }
    const auto pointAtRow = [&] {
        const s32 height = s32At(guest, kRowHeight);
        setPointerByAddress(guest, columnX(guest, item, false),
                            u(height) * u(row) + u(s16At(guest, item + kItemY)) + u(halved(height)));
    };
    if (key == kKeyDown)
    {
        ++row;
        playEffectByAddress(guest, 0);
        if (row < kRowsEachPlayer)
        {
            moved = true;
            pointAtRow();
        }
    }
    if (key == kKeyUp)
    {
        playEffectByAddress(guest, 0);
        if (row != 0)
        {
            --row;
            moved = true;
            pointAtRow();
        }
    }
    return moved;
}

}  // namespace

/* sub_43cf80: the list's keys.  Nothing being changed: the arrows move the
 * pointer over the rows and Enter starts changing a binding. */
x86::reg32 frontend::menua::listKey(App* app, CPU& cpu, reg32 item, reg32, std::uint16_t key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0xc);
    guest.leaveEdx(0);
    const reg32 controls = controlsFor(guest, cpu);
    guest.leaveEbx(controls);
    if (shownNow(guest, cpu, item, kListWidget) == 0)
        return 0;
    if (std::int16_t(guest.word(item + kItemChanging)) != -1)
    {
        listKeyChanging(guest, cpu, item, controls, key);
        return 2;
    }

    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
    s32 row = s32(guest::direct<0x49c1c0, menud::pointerListLine>(app, cpu, s16At(guest, item + kItemBoxY),
                                                              s32At(guest, kRowHeight)));
    reg32 result = listKeyArrows(guest, item, key, row) ? 2 : 0;
    if (key == kKeyEnter)
    {
        result = 2;
        const reg32 slot = controls + 4 * u(row);
        guest.leaveEdx(slot);
        if (row < kRowsEachPlayer || (gameSettings(guest).playMode == 1 && row < kRowsEachPlayer * 2))
        {
            guest::direct<0x43d920, sampleAxesAtRest>(app, cpu);
            const reg32 before = guest.dword(slot);
            guest.setDword(slot, 0);
            guest.setDword(kBindingBefore, before);
            const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kCapturingKeys);
            guest.setWord(item + kItemChanging, std::uint16_t(row));
            guest.setDword(kHeardBinding, 0);
            guest.setByte(item + kItemFlags, flags);
            guest.leaveEdx(withLowByte(slot, flags));
        }
    }
    guest.leaveEbx(u(row));
    return result;
}

/* sub_43d2b0: the box the list takes the pointer in, for the screen's layout. */
x86::reg32 frontend::menua::listFitBox(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 shown = shownNow(guest, cpu, item, kListWidget);
    if (shown == 0)
        return shown;
    guest.setWord(item + kItemBoxX, guest.word(item + kItemX));
    guest.setWord(item + kItemBoxY, guest.word(item + kItemY));
    // As wide as the generated code's registers have it: only the low words are stored.
    const reg32 tall = withLowWord(shown, guest.word(kRowHeight)) * 0xd;
    guest.setWord(item + kItemBoxWidth, std::uint16_t(tall));
    const reg32 across = withLowWord(tall, guest.word(kColumnLeft)) + withLowWord(guest.edx(), guest.word(kBindingWidth))
        + (gameSettings(guest).playMode == 1 ? 0xfa : 0x28);
    guest.setWord(item + kItemBoxHeight, std::uint16_t(across));
    return across;
}

/* sub_43d310: the list as its screen opens: its layout, in a race or in the
 * menus, and the joysticks waited for to let go. */
x86::reg32 frontend::menua::listPrepare(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const reg32 shown = shownNow(guest, cpu, item, kListWidget);
    if (shown == 0)
        return shown;
    if (guest.dword(kInRace) != 0)
    {
        guest.setDword(kPaneGap, 0xc8);
        if (gameSettings(guest).playMode == 1)
        {
            guest.setWord(item + kItemX, 0x3c);
            guest.setDword(kColumnLeft, 0x6e);
        }
        else
        {
            guest.setWord(item + kItemX, 0x96);
            guest.setDword(kColumnLeft, 0x96);
        }
        guest.setDword(kRowHeight, 0x16);
        guest.setDword(kBindingWidth, 0x7d);
    }
    else
    {
        guest.setDword(kPaneGap, 0xa5);
        guest.setDword(kColumnLeft, 0x6e);
        guest.setDword(kBindingWidth, 0x46);
        guest.setDword(kRowHeight, 0x19);
        guest.setWord(kRowShape, std::uint16_t(shapeNamed(guest, cpu, kRowShapeName.address)));
        guest.setWord(kRowLitShape, std::uint16_t(shapeNamed(guest, cpu, kRowLitShapeName.address)));
    }
    guest.setWord(item + kItemChanging, 0xffff);
    return guest::direct<0x43d650, waitJoystickRelease>(app, cpu);
}

/* sub_43d650: waiting until the joysticks are let go.  (The game's code asks of
 * them with its "any held" flag for the device number: of the first two only.) */
x86::reg32 frontend::menua::waitJoystickRelease(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x84);
    const reg32 state = guest.frame();
    bool held;
    reg32 left;
    do
    {
        held = false;
        for (s32 i = 0; i < s32(gameSettings(guest).deviceCount); ++i)
        {
            if (readJoystick(guest, cpu, held ? 1u : 0u, state) != 0 && guest.dword(state + joystick::kAnyHeld) != 0)
                held = true;
        }
        left = runSystemTasks(guest, cpu);
    } while (held);
    return left;
}

/* sub_43d6a0: a binding taken away from the rows that had it: a key by its code,
 * an axis also where the two would share an end of its travel. */
x86::reg32 frontend::menua::clearConflictingBindings(App* app, CPU& cpu, reg32 binding)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    if (binding == 0)
        return 0;
    const s32 count = gameSettings(guest).playMode == 1 ? kRowsEachPlayer * 2 : kRowsEachPlayer;
    const reg32 controls = controlsFor(guest, cpu);
    if (lowByte(binding) == kBindKey)
    {
        for (s32 row = 0; row < count; ++row)
        {
            const reg32 at = controls + 4 * u(row);
            if (((guest.dword(at) << 8) >> 16) == ((binding << 8) >> 16))
                guest.setDword(at, 0);
        }
        return u(count);
    }
    reg32 at = 0;
    for (s32 row = 0; row < count; ++row)
    {
        at = controls + 4 * u(row);
        const reg32 other = guest.dword(at);
        bool clear = binding == other;
        if (!clear && lowByte(binding) == kBindAxis && lowByte(other) == kBindAxis && (other >> 28) == (binding >> 28)
            && ((other << 4) >> 28) == ((binding << 4) >> 28))
        {
            const reg32 otherLow = (other << 16) >> 24, bindingHigh = (binding << 8) >> 24;
            const reg32 otherHigh = (other << 8) >> 24, bindingLow = (binding << 16) >> 24;
            clear = otherLow == bindingHigh || bindingLow == otherHigh || otherHigh == bindingHigh || bindingLow == otherLow;
        }
        if (clear)
            guest.setDword(at, 0);
    }
    return at;
}

/* sub_43d7f0: a key binding's name: a line of the language's for the keys with
 * one, the character for the rest. */
x86::reg32 frontend::menua::bindingKeyName(App* app, CPU& cpu, reg32 binding, reg32 buffer)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    struct NamedKey
    {
        reg32 scan;
        std::uint32_t text;
    };
    static const NamedKey kNamedKeys[] = {
        { 0x39, 0x58 }, { 0x1c, 0x59 }, { 0x52, 0x5a }, { 0x53, 0x5b }, { 0x47, 0x5c }, { 0x4f, 0x5d }, { 0x49, 0x5e }, { 0x51, 0x5f },
        { 0x48, 0x60 }, { 0x50, 0x61 }, { 0x4b, 0x62 }, { 0x4d, 0x63 }, { 0x0e, 0x64 }, { 0x0f, 0x65 }, { 0x4c, 0x44 },
    };
    const reg32 scan = (binding << 8) >> 16;
    guest.leaveEdx(scan);
    for (const NamedKey& key : kNamedKeys)
    {
        if (key.scan == scan)
        {
            const reg32 name = languageText(guest, cpu, key.text);
            guest.copyString(buffer, name);
            return name & 0xffffff00;
        }
    }
    const reg32 upper = guest.call(0x4eede0, fe::args({ binding >> 24 }));
    return guest.call(kSprintf, fe::args({}, { buffer, kFormatKey.address, upper }));
}

/* sub_43d920: where each axis of the devices that are on rests now, for
 * sub_43be40 to tell them moved. */
x86::reg32 frontend::menua::sampleAxesAtRest(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x84);
    const reg32 state = guest.frame();
    reg32 left = guest.eax();
    for (reg32 device = 0; s32(device) < s32(gameSettings(guest).deviceCount); ++device)
    {
        left = device * 17;
        if (gameSettings(guest).devices[device].kind == 0)
            continue;
        left = readJoystick(guest, cpu, device, state);
        if (left == 0)
            continue;
        for (reg32 axis = 0; axis < 8; ++axis)
        {
            left = reg32(1) << axis;
            if (gameSettings(guest).devices[device].axisMask & left)
                guest.setDword(kAxisRest + (device << 5) + 4 * axis, u(s32At(guest, state + joystick::kAxes + 4 * axis) >> 8));
        }
    }
    return left;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The colour picker
 * ------------------------------------------------------------------------- */

/* Its item: the colour being chosen and the one to go back to (hue, saturation,
 * value, and a fourth), the areas of the triangle, the bar, the swatch and the
 * two buttons, each its top, left, width and height, and the shapes. */
namespace picker
{
constexpr reg32 kWidget = 0x13;
constexpr reg32 kBoxTop = 0x1a;
constexpr reg32 kBoxX = 0x1c;
constexpr reg32 kBoxHeight = 0x1e;
constexpr reg32 kBoxWidth = 0x20;
constexpr reg32 kHeight = 0x3c;
constexpr reg32 kWidth = 0x3e;
constexpr reg32 kHue = 0x40;
constexpr reg32 kSaturation = 0x44;
constexpr reg32 kValue = 0x48;
constexpr reg32 kFourth = 0x4c;
constexpr reg32 kSavedColour = 0x50;
constexpr reg32 kPlayer = 0x60;
constexpr reg32 kTriangleTop = 0x64;
constexpr reg32 kTriangleLeft = 0x66;
constexpr reg32 kTriangleWidth = 0x68;
constexpr reg32 kTriangleHeight = 0x6a;
constexpr reg32 kBarTop = 0x6c;
constexpr reg32 kBarLeft = 0x6e;
constexpr reg32 kBarWidth = 0x70;
constexpr reg32 kBarHeight = 0x72;
constexpr reg32 kSwatchTop = 0x74;
constexpr reg32 kSwatchLeft = 0x76;
constexpr reg32 kSwatchWidth = 0x78;
constexpr reg32 kSwatchHeight = 0x7a;
constexpr reg32 kOkTop = 0x7c;
constexpr reg32 kOkLeft = 0x7e;
constexpr reg32 kOkWidth = 0x80;
constexpr reg32 kOkHeight = 0x82;
constexpr reg32 kCancelTop = 0x84;
constexpr reg32 kCancelLeft = 0x86;
constexpr reg32 kCancelWidth = 0x88;
constexpr reg32 kCancelHeight = 0x8a;
constexpr reg32 kUnderPointer = 0x8c;     // int16, an area of Hit
constexpr reg32 kPressed = 0x8e;          // int16
constexpr reg32 kBackShape = 0x90;        // int16 each
constexpr reg32 kFrameShape = 0x92;
constexpr reg32 kPanelShape = 0x94;
constexpr reg32 kCancelLitShape = 0x96;
constexpr reg32 kOkLitShape = 0x98;
constexpr reg32 kTitle = 0x9c;            // char*
constexpr reg32 kOkText = 0xa0;
constexpr reg32 kCancelText = 0xa4;

enum Hit : s32
{
    kHitNothing = 0,
    kHitOk = 1,
    kHitCancel = 2,
    kHitTriangle = 3,
    kHitBar = 4,
    kHitOutside = 5,
};
}

constexpr reg32 kHueRampTexture = 0x5f3dd0;
constexpr GuestString kNoName{ 0x5377dc, "" };
constexpr GuestString kCarSelect1{ 0x5377e0, "CAR_SELECT_OBJ3DP1" };
constexpr GuestString kCarSelect2{ 0x5377f4, "CAR_SELECT_OBJ3DP2" };
constexpr GuestString kPickerShapes[] = { { 0x537808, "yar1" }, { 0x537810, "bdd2" }, { 0x537818, "ccdd" },
                                          { 0x537820, "ccht" }, { 0x537828, "ccbt" } };
constexpr std::uint32_t kColourMarked = 0xb3;        // byte of the car's item: to be drawn again

}  // namespace

/* sub_43db20: the bar's texture, the hues from top to bottom. */
x86::reg32 frontend::menua::makeHueRamp(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    constexpr reg32 kSide = 0x100;
    const reg32 hsv = guest.frame();
    const reg32 argb = guest.frame() + 0x10;
    const reg32 shape = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kNoName.address, 0x40014u, 0x10u);
    guest.setByte(shape, 0);
    guest.setWord(shape + 4, kSide);
    guest.setWord(shape + 6, kSide);
    guest.setByte(shape, 0x7d);
    reg32 pixel = shape + 0x10;
    for (reg32 hue = 0; hue < kSide; ++hue)
    {
        guest.setDword(hsv + 4, 0xff);
        guest.setDword(hsv + 8, 0xff);
        guest.setDword(hsv + 0xc, 0xff);
        guest.setDword(hsv, hue);
        guest::direct<0x43de20, hsvToArgb>(app, cpu, hsv, argb);
        const reg32 colour = guest.dword(argb) | 0xff000000;
        for (reg32 x = 0; x < kSide; ++x, pixel += 4)
            guest.setDword(pixel, colour);
    }
    fe::direct<0x4d6070, port::shapes::textureOf>(app, cpu, cpu.esp - 4, kHueRampTexture, shape, 0x40, 0u);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, shape);
}

/* sub_43dc90: the colour on the player's car, and the car's picture marked to be
 * drawn again. */
x86::reg32 frontend::menua::applyColourToCar(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x20);
    const reg32 shown = shownNow(guest, cpu, item, picker::kWidget);
    if (shown == 0)
        return shown;
    const reg32 car = gameSettings(guest).players[guest.dword(item + picker::kPlayer) == 0 ? 0 : 1].car;
    const reg32 hsv = guest.frame();
    const reg32 rgb = guest.frame() + 0x10;
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(hsv + 4 * i, guest.dword(item + picker::kHue + 4 * i));
    guest.call(kHsvToRgb, fe::args({ hsv, rgb }));
    reg32 left = u(*fe::direct<0x4398c0, port::fields::customColourSet>(
        app, cpu, cpu.esp - 4 - 4 * 2, car, s32(guest.dword(item + picker::kPlayer)), guest.byte(rgb),
        guest.byte(rgb + 4), guest.byte(rgb + 8), std::uint8_t(0x80)));
    if (guest.dword(item + picker::kPlayer) == 0)
    {
        const reg32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, cpu);
        fe::Item found = fe::findItem(guest, menu, kCarSelect1);
        left = found.address();
        if (found)
            guest.setByte(found.address() + kColourMarked, 1);
    }
    if (guest.dword(item + picker::kPlayer) == 1)
    {
        const reg32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, cpu);
        fe::Item found = fe::findItem(guest, menu, kCarSelect2);
        left = found.address();
        if (found)
            guest.setByte(found.address() + kColourMarked, 1);
    }
    return left;
}

/* sub_43dd90: the colour taken, the picker gone. */
x86::reg32 frontend::menua::acceptColour(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 shown = shownNow(guest, cpu, item, picker::kWidget);
    if (shown == 0)
        return shown;
    const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kChanged);
    guest.setByte(item + kItemFlags, flags);
    guest.setByte(item + kItemFlags, std::uint8_t(flags & ~kCapturingKeys));
    return guest::direct<0x43dc90, applyColourToCar>(app, cpu, item);
}

/* sub_43ddd0: the colour as it was, the picker gone. */
x86::reg32 frontend::menua::revertColour(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 shown = shownNow(guest, cpu, item, picker::kWidget);
    if (shown == 0)
        return shown;
    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) | kChanged));
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(item + picker::kHue + 4 * i, guest.dword(item + picker::kSavedColour + 4 * i));
    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
    return guest::direct<0x43dc90, applyColourToCar>(app, cpu, item);
}

/* sub_43de20: a colour of hue, saturation and value as opaque ARGB. */
x86::reg32 frontend::menua::hsvToArgb(App* app, CPU& cpu, reg32 hsv, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8 + 0x10);
    const reg32 rgb = guest.frame();
    guest.call(kHsvToRgb, fe::args({ hsv, rgb }));
    const reg32 blue = guest.dword(rgb + 8) & 0xff;
    const reg32 argb = 0xff000000 | ((guest.dword(rgb) & 0xff) << 16) | ((guest.dword(rgb + 4) & 0xff) << 8) | blue;
    guest.setDword(out, argb);
    guest.leaveEdx(argb);
    return blue;
}

/* sub_43de60: where a colour puts its marker in the triangle of that width and
 * height: the row by one of its numbers, from the tip, the place in the row by
 * the other, inside the row's ends. */
x86::reg32 frontend::menua::triangleFromColour(App* app, CPU& cpu, s32 width, s32 height, s32 across, s32 down, reg32 outX, reg32 outY)
{
    Guest guest(app, cpu);
    const s32 half = halved(width);
    guest.setDword(outY, u(times(minus(0xff, down), height) / 0xff));
    const s32 reach = times(down, half) / 0xff;
    const s32 low = half - reach, high = half + reach;
    const s32 product = times(high - low, minus(0xff, across));
    const s32 x = product / 0xff + low;
    const s32 clamped = low > x ? low : high < x ? high : x;
    guest.setDword(outX, u(clamped));
    guest.leaveEdx(u(product % 0xff));
    guest.leaveEbx(u(low));
    guest.leaveEcx(u(clamped));
    return u(x);
}

/* sub_43def0: back from a place in the triangle to the two numbers (the middle's,
 * at the tip where the row has no width). */
x86::reg32 frontend::menua::colourAtPlace(App* app, CPU& cpu, s32 width, s32 height, reg32 outAcross, reg32 outDown, s32 x, s32 y)
{
    Guest guest(app, cpu);
    const s32 half = halved(width);
    const s32 row = y < 0 ? 0 : height < y ? height : y;
    const s32 down = 0xff - times(row, 0xff) / height;
    guest.setDword(outDown, u(down));
    const s32 reach = times(down, half) / 0xff;
    const s32 low = half - reach, high = half + reach;
    const s32 span = high - low;
    const s32 clamped = low > x ? low : high < x ? high : x;
    guest.leaveEcx(u(clamped));
    if (span == 0)
    {
        guest.setDword(outAcross, 0x80);
        guest.leaveEdx(u(reach));
        return u(low);
    }
    const s32 fraction = times(clamped - low, 0xff) / span;
    guest.setDword(outAcross, u(0xff - fraction));
    guest.leaveEdx(u(0xff - fraction));
    return u(fraction);
}

/* sub_43dfb0: the picker's back, frame and title. */
x86::reg32 frontend::menua::drawPickerFrame(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    drawShape(guest, cpu, s16At(guest, item + picker::kPanelShape), u(s16At(guest, item + kItemX)), u(s16At(guest, item + kItemY)));
    const s32 y = s16At(guest, item + kItemY);
    const s32 x = s16At(guest, item + kItemX);
    drawShape(guest, cpu, s16At(guest, item + picker::kBackShape), u(x - 0x23), u(y + 0xc));
    drawShape(guest, cpu, s16At(guest, item + picker::kFrameShape), u(x - 0x17), u(y - 1));
    return drawText(guest, cpu, guest.dword(item + picker::kTitle), u(s16At(guest, item + kItemX) + 0xc), u(y - 1), kColourChosen);
}

/* sub_43e040: the swatch of the colour, with its shadow. */
x86::reg32 frontend::menua::drawColourSwatch(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const reg32 argb = guest.frame();
    guest::direct<0x43de20, hsvToArgb>(app, cpu, item + picker::kHue, argb);
    const s32 left = s16At(guest, item + kItemX) + s16At(guest, item + picker::kSwatchLeft);
    const s32 top = s16At(guest, item + kItemY) + s16At(guest, item + picker::kSwatchTop);
    const reg32 width = u(s16At(guest, item + picker::kSwatchWidth));
    const reg32 height = u(s16At(guest, item + picker::kSwatchHeight));
    filledRectangle(guest, cpu, u(left + 3), u(top + 3), width, height, 0xff000000);
    return filledRectangle(guest, cpu, u(left), u(top), width, height, guest.dword(argb));
}

/* sub_43e0d0: the two buttons, the one under the pointer lit. */
x86::reg32 frontend::menua::drawPickerButtons(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    const auto button = [&](std::uint16_t which, reg32 litShape, reg32 left, reg32 top, reg32 text) {
        const s32 x = s16At(guest, item + kItemX) + s16At(guest, item + left);
        const s32 y = s16At(guest, item + kItemY) + s16At(guest, item + top);
        if (guest.word(item + picker::kUnderPointer) == which)
            drawShape(guest, cpu, s16At(guest, item + litShape), u(x), u(y));
        const reg32 colour = guest.word(item + picker::kUnderPointer) == which ? kColourChosen : kColourPlain;
        return drawText(guest, cpu, guest.dword(item + text), u(x + 0xf), u(y), colour);
    };
    button(picker::kHitCancel, picker::kCancelLitShape, picker::kCancelLeft, picker::kCancelTop, picker::kCancelText);
    return button(picker::kHitOk, picker::kOkLitShape, picker::kOkLeft, picker::kOkTop, picker::kOkText);
}

/* sub_43e1f0: the markers of the hue on the bar and of the colour in the
 * triangle, each with its shadow. */
x86::reg32 frontend::menua::drawColourMarkers(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0xc);
    constexpr reg32 kShadow = 0x7f000000;
    constexpr reg32 kMarker = 0xff7f7fff;
    const reg32 markerX = guest.frame(), markerY = guest.frame() + 4;
    const reg32 across = guest.dword(item + picker::kSaturation);
    const reg32 down = guest.dword(item + picker::kValue);
    const s32 hueRow = times(s32At(guest, item + picker::kHue), s16At(guest, item + picker::kBarHeight)) / 0xff;
    const reg32 y = u(hueRow + s16At(guest, item + picker::kBarTop) + s16At(guest, item + kItemY) - 3);
    const reg32 x = u(s16At(guest, item + picker::kBarLeft) + s16At(guest, item + kItemX) - 3);
    const reg32 barWidth = u(s16At(guest, item + picker::kBarWidth) + 6);
    frameRectangle(guest, cpu, x + 1, y + 1, barWidth, 6, kShadow);
    frameRectangle(guest, cpu, x, y, u(s16At(guest, item + picker::kBarWidth) + 6), 6, kMarker);
    guest::direct<0x43de60, triangleFromColour>(app, cpu, s16At(guest, item + picker::kTriangleWidth),
                                                s16At(guest, item + picker::kTriangleHeight), s32(across), s32(down), markerX,
                                                markerY);
    const reg32 colourX = guest.dword(markerX) + u(s16At(guest, item + kItemX) + s16At(guest, item + picker::kTriangleLeft) - 3);
    const reg32 colourY = guest.dword(markerY) + u(s16At(guest, item + kItemY) + s16At(guest, item + picker::kTriangleTop) - 3);
    frameRectangle(guest, cpu, colourX + 1, colourY + 1, 6, 6, kShadow);
    return frameRectangle(guest, cpu, colourX, colourY, 6, 6, kMarker);
}

/* sub_43e310: the triangle of the colour's hue, over its shadow and a translucent
 * edge. */
x86::reg32 frontend::menua::drawColourTriangle(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x28);
    const reg32 hsv = guest.frame();
    const reg32 rgb = guest.frame() + 0x10;
    guest.setDword(hsv, guest.dword(item + picker::kHue));
    guest.setDword(hsv + 4, 0xff);
    guest.setDword(hsv + 8, 0xff);
    guest.setDword(hsv + 0xc, 0xff);
    guest.call(kHsvToRgb, fe::args({ hsv, rgb }));
    const reg32 colour = ((guest.dword(rgb) & 0xff) << 16) | ((guest.dword(rgb + 4) & 0xff) << 8) | (guest.dword(rgb + 8) & 0xff);
    const s32 top = s16At(guest, item + kItemY) + s16At(guest, item + picker::kTriangleTop);
    const s32 left = s16At(guest, item + kItemX) + s16At(guest, item + picker::kTriangleLeft);
    const s32 width = s16At(guest, item + picker::kTriangleWidth);
    const s32 bottom = top + s16At(guest, item + picker::kTriangleHeight);
    const s32 tip = left + halved(width);
    guest::direct<0x4d88c0, menue::shadedTriangle>(app, cpu, u(left + 3), u(top + 3), u(left + width + 3), u(top + 3),
                                                   u(tip + 3), u(bottom + 3), 0xff000000u, 0xff000000u, 0xff000000u);
    guest::direct<0x4d88c0, menue::shadedTriangle>(app, cpu, u(left - 1), u(top - 1), u(left + width + 1), u(top - 1),
                                                   u(tip), u(bottom + 1), colour | 0xaf000000, 0xafffffffu, 0xaf000000u);
    return guest::direct<0x4d88c0, menue::shadedTriangle>(app, cpu, u(left), u(top), u(left + width), u(top), u(tip),
                                                          u(bottom), colour | 0xff000000, 0xffffffffu, 0xff000000u);
}

namespace
{

/* A vertex of the bar's quad, as sub_4c12e0 clips it: x and y as floats, depth
 * and w, its colour, where it falls as flags, its place in the texture. */
namespace vertex
{
constexpr reg32 kX = 0x00;
constexpr reg32 kY = 0x04;
constexpr reg32 kZ = 0x08;
constexpr reg32 kW = 0x0c;
constexpr reg32 kColour = 0x10;
constexpr reg32 kOutcode = 0x14;
constexpr reg32 kU = 0x18;
constexpr reg32 kV = 0x1c;
constexpr reg32 kSize = 0x20;
}

// What the renderer clips to, compared as the game compares them: the floats'
// bits as integers.
constexpr reg32 kClipW = 0x7d34f8;
constexpr reg32 kClipLeft = 0x7d34fc;
constexpr reg32 kClipRight = 0x7d350c;
constexpr reg32 kClipTop = 0x7d3504;
constexpr reg32 kClipBottom = 0x7d3500;

std::uint8_t vertexOutcode(const Guest& guest, reg32 corner)
{
    const s32 w = s32At(guest, corner + vertex::kW);
    if (w < 0 || w >= s32At(guest, kClipW))
        return 0x10;
    std::uint8_t code = 0;
    const s32 y = s32At(guest, corner + vertex::kY);
    if (y < 0 || y < s32At(guest, kClipTop))
        code |= 8;
    else if (y > s32At(guest, kClipBottom))
        code |= 4;
    const s32 x = s32At(guest, corner + vertex::kX);
    if (x < 0 || x < s32At(guest, kClipLeft))
        code |= 1;
    else if (x > s32At(guest, kClipRight))
        code |= 2;
    return code;
}

}  // namespace

/* sub_43e490: the bar: its shadow, and a quad with the hues' texture. */
x86::reg32 frontend::menua::drawValueBar(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x90);
    const s32 x = s16At(guest, item + kItemX);
    const s32 y = s16At(guest, item + kItemY);
    const s32 left = s16At(guest, item + picker::kBarLeft) + x;
    const s32 width = s16At(guest, item + picker::kBarWidth);
    const s32 right = s16At(guest, item + picker::kBarLeft) + width + x;
    const s32 height = s16At(guest, item + picker::kBarHeight);
    const s32 top = s16At(guest, item + picker::kBarTop) + y;
    const s32 bottom = s16At(guest, item + picker::kBarTop) + height + y;
    filledRectangle(guest, cpu, u(left + 3), u(top + 3), u(width), u(height), 0xff000000);

    // The four corners, clockwise from the top left, each at a depth of the
    // game's and in white.
    const reg32 corners = guest.frame();
    const reg32 depth = guest.dword(0x563aa8), w = guest.dword(0x563aac);
    const float leftAt = float(fe::X87::integer(left)), rightAt = float(fe::X87::integer(right));
    const float topAt = float(fe::X87::integer(top)), bottomAt = float(fe::X87::integer(bottom));
    const struct
    {
        float x, y;
        float across, down;
    } kCorners[4] = { { leftAt, topAt, 0.0f, 0.0f },
                      { rightAt, topAt, 1.0f, 0.0f },
                      { rightAt, bottomAt, 1.0f, 1.0f },
                      { leftAt, bottomAt, 0.0f, 1.0f } };
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = corners + vertex::kSize * i;
        guest.setFloat(corner + vertex::kX, kCorners[i].x);
        guest.setFloat(corner + vertex::kY, kCorners[i].y);
        guest.setDword(corner + vertex::kZ, depth);
        guest.setDword(corner + vertex::kW, w);
        guest.setDword(corner + vertex::kColour, 0xffffffff);
        guest.setFloat(corner + vertex::kU, kCorners[i].across);
        guest.setFloat(corner + vertex::kV, kCorners[i].down);
    }
    renderState(guest, cpu, 1, guest.dword(kHueRampTexture));
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = corners + vertex::kSize * i;
        guest.setByte(corner + vertex::kOutcode, vertexOutcode(guest, corner));
    }
    return guest.call(0x4c12e0, fe::args({ corners, corners + vertex::kSize, corners + vertex::kSize * 2, corners + vertex::kSize * 3 }));
}

namespace
{

s32 clampByte(s32 value)
{
    return value < 0 ? 0 : value > 0xff ? 0xff : value;
}

}  // namespace

/* sub_43e7a0: the area pressed followed by the pointer: the triangle's place,
 * the bar's hue. */
x86::reg32 frontend::menua::pickerDragMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x10);
    const reg32 acrossAt = guest.frame() + 4, downAt = guest.frame() + 8;
    s32 hue = s32At(guest, item + picker::kHue);
    guest.setDword(acrossAt, guest.dword(item + picker::kSaturation));
    guest.setDword(downAt, guest.dword(item + picker::kValue));
    const std::uint16_t pressed = guest.word(item + picker::kPressed);
    if (pressed == picker::kHitTriangle)
    {
        const reg32 x = guest.dword(kPointerX) - u(s16At(guest, item + kItemX)) - u(s16At(guest, item + picker::kTriangleLeft));
        const reg32 y = guest.dword(kPointerY) - u(s16At(guest, item + kItemY)) - u(s16At(guest, item + picker::kTriangleTop));
        guest::direct<0x43def0, colourAtPlace>(app, cpu, s16At(guest, item + picker::kTriangleWidth),
                                               s16At(guest, item + picker::kTriangleHeight), acrossAt, downAt, s32(x), s32(y));
    }
    else if (pressed == picker::kHitBar)
    {
        const s32 y = s32(guest.dword(kPointerY) - u(s16At(guest, item + kItemY)) - u(s16At(guest, item + picker::kBarTop)));
        hue = times(y, 0xff) / s16At(guest, item + picker::kBarHeight);
    }
    else
    {
        return withLowWord(guest.dword(item + picker::kValue), pressed);
    }
    const s32 value = clampByte(s32At(guest, downAt));
    guest.setDword(item + picker::kHue, u(clampByte(hue)));
    guest.setDword(item + picker::kSaturation, u(clampByte(s32At(guest, acrossAt))));
    guest.setDword(item + picker::kValue, u(value));
    return u(value);
}

/* sub_43e8e0: which of the picker's areas the pointer is on; the triangle only
 * between its sides. */
x86::reg32 frontend::menua::pickerHitRegion(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    const s32 x = s32(guest.dword(kPointerX) - u(s16At(guest, item + kItemX)));
    const s32 y = s32(guest.dword(kPointerY) - u(s16At(guest, item + kItemY)));
    if (fe::direct<0x49c1f0, glue::pointerInBox>(app, cpu, item + picker::kBoxTop) == 0)
        return picker::kHitOutside;
    const auto inArea = [&](reg32 left, reg32 width, reg32 top, reg32 height) {
        const s32 l = s16At(guest, item + left), t = s16At(guest, item + top);
        return x > l && x < l + s16At(guest, item + width) && y > t && y < t + s16At(guest, item + height);
    };
    if (inArea(picker::kOkLeft, picker::kOkWidth, picker::kOkTop, picker::kOkHeight))
        return picker::kHitOk;
    if (inArea(picker::kCancelLeft, picker::kCancelWidth, picker::kCancelTop, picker::kCancelHeight))
        return picker::kHitCancel;
    if (inArea(picker::kTriangleLeft, picker::kTriangleWidth, picker::kTriangleTop, picker::kTriangleHeight))
    {
        const s32 left = s16At(guest, item + picker::kTriangleLeft);
        const s32 half = halved(s16At(guest, item + picker::kTriangleWidth));
        const s32 height = s16At(guest, item + picker::kTriangleHeight);
        const s32 row = y - s16At(guest, item + picker::kTriangleTop);
        const s32 reach = (0xff - times(row > height ? height : row, 0xff) / height) * half / 0xff;
        if (x >= half - reach + left && x <= half + reach + left)
            return picker::kHitTriangle;
    }
    if (inArea(picker::kBarLeft, picker::kBarWidth, picker::kBarTop, picker::kBarHeight))
        return picker::kHitBar;
    return picker::kHitNothing;
}

/* sub_43ead0: the mouse on the picker: an area pressed, followed, and let go of
 * on the same area. */
x86::reg32 frontend::menua::pickerMouse(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    reg32 left = item;  // eax as it came
    if (guest.word(item + picker::kPressed) == 0)
    {
        left = guest::direct<0x43e8e0, pickerHitRegion>(app, cpu, item);
        guest.setWord(item + picker::kUnderPointer, std::uint16_t(left));
    }
    if (guest.word(item + picker::kUnderPointer) != 0)
    {
        left = fe::direct<0x49c080, glue::mouseKey>(app, cpu);
        if (left == kKeyEnter)
            guest.setWord(item + picker::kPressed, guest.word(item + picker::kUnderPointer));
    }
    if (guest.word(item + picker::kPressed) == 0)
        return left;
    if (fe::direct<0x49c080, glue::mouseKey>(app, cpu) == kKeyEnter)
        return guest::direct<0x43e7a0, pickerDragMoved>(app, cpu, item);
    const reg32 at = guest::direct<0x43e8e0, pickerHitRegion>(app, cpu, item);
    left = at;
    if (at == u(s16At(guest, item + picker::kPressed)))
    {
        const std::uint16_t pressed = guest.word(item + picker::kPressed);
        if (pressed == picker::kHitOk || pressed == picker::kHitCancel || pressed == picker::kHitOutside)
        {
            playEffect(guest, cpu, 0);
            if (pressed == picker::kHitOk)
                left = guest::direct<0x43dd90, acceptColour>(app, cpu, item);
            else if (pressed == picker::kHitCancel)
                left = guest::direct<0x43dc90, applyColourToCar>(app, cpu, item);
            else
                left = guest::direct<0x43ddd0, revertColour>(app, cpu, item);
        }
        else
        {
            left = withLowWord(at, pressed);
        }
    }
    guest.setWord(item + picker::kPressed, 0);
    return left;
}

/* sub_43ebc0: the picker on every pass, while it is up. */
x86::reg32 frontend::menua::drawColourPicker(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (shownNow(guest, cpu, item, picker::kWidget) == 0)
        return 0;
    guest::direct<0x43ead0, pickerMouse>(app, cpu, item);
    guest::direct<0x43dfb0, drawPickerFrame>(app, cpu, item);
    guest::direct<0x43e040, drawColourSwatch>(app, cpu, item);
    if (guest.byte(kDisplayFlags) & kNoTranslucency)
        renderState(guest, cpu, 6, 1);
    guest::direct<0x43e310, drawColourTriangle>(app, cpu, item);
    if (guest.byte(kDisplayFlags) & kNoTranslucency)
        guest.call(kSetRenderState, fe::args({ 6, 0 }));  // by address: the 0 in edx, the function leaves there
    guest::direct<0x43e490, drawValueBar>(app, cpu, item);
    guest::direct<0x43e1f0, drawColourMarkers>(app, cpu, item);
    guest::direct<0x43e0d0, drawPickerButtons>(app, cpu, item);
    return 0;
}

namespace
{

/* A step of a colour's number by the keys. */
void stepColour(Guest& guest, reg32 at, s32 step)
{
    guest.setDword(at, u(clampByte(s32(guest.dword(at) + u(step)))));
}

}  // namespace

/* sub_43ec40: the picker's keys: the arrows move the colour, Tab goes from
 * button to button, Enter is the button's, Escape takes back. */
x86::reg32 frontend::menua::pickerKey(App* app, CPU& cpu, reg32 item, reg32, std::uint16_t key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest.leaveEbx(item);
    if (shownNow(guest, cpu, item, picker::kWidget) == 0)
        return 0;
    switch (key)
    {
    case kKeyEscape:
        // By address, as at Enter and the pointer's move at Tab: what they leave in edx, the function leaves.
        playEffectByAddress(guest, 0);
        guest::direct<0x43ddd0, revertColour>(app, cpu, item);
        return 2;
    case kKeyUp:
        stepColour(guest, item + picker::kValue, 10);
        return 2;
    case kKeyDown:
        stepColour(guest, item + picker::kValue, -10);
        return 2;
    case kKeyLeft:
        stepColour(guest, item + picker::kSaturation, 10);
        return 2;
    case kKeyRight:
        stepColour(guest, item + picker::kSaturation, -10);
        return 2;
    case kKeyPageUp:
        stepColour(guest, item + picker::kHue, -10);
        return 2;
    case kKeyPageDown:
        stepColour(guest, item + picker::kHue, 10);
        return 2;
    case kKeyTab:
    {
        const std::uint16_t under = guest.word(item + picker::kUnderPointer);
        guest.setWord(item + picker::kUnderPointer, under == picker::kHitOk ? picker::kHitCancel : picker::kHitOk);
        const bool ok = guest.word(item + picker::kUnderPointer) == picker::kHitOk;
        const reg32 left = ok ? picker::kOkLeft : picker::kCancelLeft;
        const reg32 width = ok ? picker::kOkWidth : picker::kCancelWidth;
        const reg32 top = ok ? picker::kOkTop : picker::kCancelTop;
        const reg32 height = ok ? picker::kOkHeight : picker::kCancelHeight;
        guest.setDword(kPointerX, u(s16At(guest, item + kItemX) + s16At(guest, item + left) + halved(s16At(guest, item + width))));
        guest.setDword(kPointerY, u(s16At(guest, item + kItemY) + s16At(guest, item + top) + halved(s16At(guest, item + height))));
        playEffect(guest, cpu, 1);
        setPointerByAddress(guest, guest.dword(kPointerX), guest.dword(kPointerY));
        return 2;
    }
    case kKeyEnter:
    {
        const std::uint16_t under = guest.word(item + picker::kUnderPointer);
        if (under != picker::kHitOk && under != picker::kHitCancel)
            return 0;
        playEffectByAddress(guest, 0);
        if (under == picker::kHitOk)
            guest::direct<0x43dd90, acceptColour>(app, cpu, item);
        else
            guest::direct<0x43dc90, applyColourToCar>(app, cpu, item);
        return 2;
    }
    default:
        return 0;
    }
}

/* sub_43ef80: what the picker has the pointer show over it: 2 while the bar is
 * being dragged, 5 the triangle, 1 and 5 over them, 0 otherwise. */
x86::reg32 frontend::menua::pickerPointerKind(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (shownNow(guest, cpu, item, picker::kWidget) == 0)
        return 0;
    const std::uint16_t pressed = guest.word(item + picker::kPressed);
    if (pressed == picker::kHitBar)
        return 2;
    if (pressed == picker::kHitTriangle)
        return 5;
    const std::uint16_t under = guest.word(item + picker::kUnderPointer);
    if (under == picker::kHitBar)
        return 1;
    if (under == picker::kHitTriangle)
        return 5;
    return 0;
}

/* sub_43eff0: the box the picker takes the pointer in. */
x86::reg32 frontend::menua::pickerFitBox(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 shown = shownNow(guest, cpu, item, picker::kWidget);
    if (shown == 0)
        return shown;
    guest.setWord(item + picker::kBoxX, guest.word(item + kItemX));
    guest.setWord(item + picker::kBoxTop, guest.word(item + kItemY));
    guest.setWord(item + picker::kBoxWidth, guest.word(item + picker::kWidth));
    guest.setWord(item + picker::kBoxHeight, guest.word(item + picker::kHeight));
    return withLowWord(shown, guest.word(item + picker::kHeight));
}

/* sub_43f030: the picker as the screen comes up, its colour a default, its
 * shapes found, its buttons as wide as their text, hidden until a colour is
 * asked for. */
x86::reg32 frontend::menua::pickerPrepare(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 shown = shownNow(guest, cpu, item, picker::kWidget);
    if (shown == 0)
        return shown;
    if (guest.word(item + picker::kWidth) == 0)
        guest.setWord(item + picker::kWidth, 0x50);
    if (guest.word(item + picker::kHeight) == 0)
        guest.setWord(item + picker::kHeight, 0x50);
    guest.setDword(item + picker::kHue, 0x7f);
    guest.setDword(item + picker::kSaturation, 0xdf);
    guest.setDword(item + picker::kValue, 0x4f);
    guest.setDword(item + picker::kFourth, 0x7f);
    guest.setDword(item + picker::kPlayer, 0);
    guest.setWord(item + picker::kBackShape, std::uint16_t(shapeNamed(guest, cpu, kPickerShapes[0].address)));
    guest.setWord(item + picker::kFrameShape, std::uint16_t(shapeNamed(guest, cpu, kPickerShapes[1].address)));
    guest.setWord(item + picker::kPanelShape, std::uint16_t(shapeNamed(guest, cpu, kPickerShapes[2].address)));
    guest.setWord(item + picker::kCancelLitShape, std::uint16_t(shapeNamed(guest, cpu, kPickerShapes[3].address)));
    const reg32 lit = shapeNamed(guest, cpu, kPickerShapes[4].address);
    guest.setWord(item + picker::kUnderPointer, 0);
    guest.setWord(item + picker::kOkLitShape, std::uint16_t(lit));
    guest.setWord(item + picker::kPressed, 0);
    guest.setWord(item + picker::kOkWidth, std::uint16_t(fe::textWidth(guest, guest.dword(item + picker::kOkText), kMenuFont) + 0xf));
    guest.setWord(item + picker::kOkHeight, 0x12);
    guest.setWord(item + picker::kCancelWidth, std::uint16_t(fe::textWidth(guest, guest.dword(item + picker::kCancelText), kMenuFont) + 0xf));
    const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kChanged);
    guest.setWord(item + picker::kCancelHeight, 0x12);
    guest.setByte(item + kItemFlags, flags);
    guest.setByte(item + kItemFlags, std::uint8_t(flags & ~kCapturingKeys));
    return guest::direct<0x43db20, makeHueRamp>(app, cpu);
}

/* sub_43f160: whether the picker is shown now. */
x86::reg32 frontend::menua::pickerShown(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return shownNow(guest, cpu, item, picker::kWidget);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The network lobby's panel
 * ------------------------------------------------------------------------- */

// Whether the panel is open, whether the pointer is on it, where it is sliding.
constexpr reg32 kPanelOpen = 0x555748;
constexpr reg32 kPanelActive = 0x5556d0;
constexpr reg32 kPanelTop = 0x5f3fc8;
constexpr reg32 kPanelLeft = 0x5f3fcc;
constexpr reg32 kPanelShapeId = 0x5556cc;        // int16: none, no panel
constexpr reg32 kPanelSliding = 0x5f3fd8;
constexpr reg32 kPanelSpin = 0x555750;
// The rectangle of its messages.
constexpr reg32 kBoxLeft = 0x5f3fbc;
constexpr reg32 kBoxRight = 0x5f3fb8;
constexpr reg32 kBoxTop = 0x5f3fc4;
constexpr reg32 kBoxBottom = 0x5f3fc0;
// The line being typed and the other buffer of messages.
constexpr reg32 kTyping = 0x5f3fdc;
constexpr reg32 kMessageBuffer = 0x5f3fd0;
constexpr reg32 kCursorBlink = 0x5f40bc;
constexpr reg32 kKeyPressed = 0x664654;

// The network's state.
constexpr reg32 kNetState = 0x7a229c;            // word
constexpr reg32 kNetClosing = 0x7a22da;          // word
constexpr reg32 kNetLeft = 0x7a22dc;             // word
constexpr reg32 kNetConnected = 0x7a22e0;

// Its shapes, int16 each.
constexpr reg32 kIconPlain = 0x5f3ffa;
constexpr reg32 kIconLit = 0x5f3ff8;
constexpr reg32 kIconChosen = 0x5f3ff4;
constexpr reg32 kGlowPlain = 0x5f3ff6;
constexpr reg32 kGlowChosen = 0x5f4004;
constexpr reg32 kButtonOpenPlain = 0x5f3ffe;
constexpr reg32 kButtonOpenBack = 0x5f4000;
constexpr reg32 kButtonClosedPlain = 0x5f3ffc;
constexpr reg32 kButtonClosedBack = 0x5f4002;

constexpr GuestString kCursor{ 0x5379ac, "_" };
constexpr GuestString kNameSeparator{ 0x537834, ": " };

bool networkGone(const Guest& guest)
{
    return guest.word(kNetLeft) != 0 || guest.dword(kNetConnected) == 0;
}

void closePanel(Guest& guest)
{
    guest.setDword(kPanelOpen, 0);
    guest.setDword(kPanelActive, 0);
}

}  // namespace

/* sub_440b30: the icon of a player taking part. */
x86::reg32 frontend::menua::drawPlayerIcon(App* app, CPU& cpu, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return drawShapeByAddress(guest, s16At(guest, kIconPlain), x, y);
}

/* sub_440b50: the icon of a player of the list, chosen or not, in a glow that
 * pulses and goes along the list with the time. */
x86::reg32 frontend::menua::drawPulsingIcon(App* app, CPU& cpu, reg32 chosen, reg32 index, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0xc);
    constexpr s32 kPulse = 0x200;
    const s32 phase = s32(guest.dword(0x79c1a4) * 2) % kPulse;
    const s32 at = s32(u(phase) + ((7 - index) << 7)) % kPulse;
    const s32 glow = at >= 0x100 ? 0x1ff - at : at;
    const reg32 colour = (u(glow) << 24) + 0xffffff;
    reg32 result = colour;
    guest.leaveEbx(y - 5);
    guest.leaveEdx(x - 6);
    guest.leaveEcx(chosen);
    const reg32 glowShape = chosen != 0 ? kGlowChosen : kGlowPlain;
    // Each call may be the last, by address: what it leaves in ebx, ecx and edx, the function leaves.
    if (guest.word(glowShape) != 0)
        result = guest.call(kDrawArtShape, fe::args({ u(s16At(guest, glowShape)), x - 6, y - 5, colour }));
    if (chosen != 0)
    {
        if (guest.word(kIconChosen) != 0)
            result = drawShapeByAddress(guest, s16At(guest, kIconChosen), x - 1, y + 1);
    }
    else if (guest.word(kIconLit) != 0)
    {
        result = drawShapeByAddress(guest, s16At(guest, kIconLit), x, y);
    }
    return result;
}

/* sub_440d60: the pointer put on the panel's messages. */
x86::reg32 frontend::menua::pointAtPanel(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    // By address: what the function leaves in eax is this one's.
    return setPointerByAddress(guest, guest.dword(kBoxLeft) + 0xa, guest.dword(kBoxTop) + 0xa);
}

namespace
{

/* The lines round the box of messages, of a colour for the pointer on it. */
void drawRacePanelLines(Guest& guest, x86::CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 colour)
{
    shadedRectangle(guest, cpu, left - 1, top + 2, right - 2, top + 4, colour);
    shadedRectangle(guest, cpu, left - 2, top + 2, left, bottom, colour);
    shadedRectangle(guest, cpu, right - 4, top + 2, right - 2, bottom, colour);
    shadedRectangle(guest, cpu, left - 1, bottom - 0x12, right - 2, bottom - 0x10, colour);
    shadedRectangle(guest, cpu, left - 2, bottom, right - 2, bottom + 2, colour);
}

}  // namespace

/* sub_440d80: the panel over a network race's pause: at the view's side, if it
 * is open; 4 when the network has gone. */
x86::reg32 frontend::menua::drawRacePanel(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x2c);
    constexpr reg32 kBack = 0x80222240;
    const reg32 rectTop = guest.frame(), rectRight = guest.frame() + 4;
    guest.setDword(rectTop, 0);
    guest.setDword(rectRight, 0);
    fe::direct<0x4beb20, pause::readRect>(app, cpu, 0x7cdac0u, rectTop, rectTop, rectRight, rectTop);
    const bool wideView = s32At(guest, rectRight) >= 0x280;
    guest.setDword(kBoxTop, 0x118);
    guest.setDword(kBoxRight, wideView ? 0x201 : 0x240);
    guest.setDword(kBoxBottom, 0x1a6);
    guest.setDword(kBoxLeft, wideView ? 0x80 : 0x40);
    const reg32 left = guest.dword(kBoxLeft), top = guest.dword(kBoxTop);
    const reg32 right = guest.dword(kBoxRight), bottom = guest.dword(kBoxBottom);
    if (guest.dword(kPanelOpen) != 0)
    {
        shadedRectangle(guest, cpu, left - 1, top + 2, right - 2, bottom + 2, kBack);
        const reg32 over = guest::direct<0x49c240, menud::pointerOverRect>(app, cpu, left, top, right - left - 4, bottom - top - 3);
        if (over != 0 && !dialogShown(guest, cpu))
        {
            if (networkGone(guest))
                closePanel(guest);
            else
                guest.setDword(kPanelActive, 1);
            drawRacePanelLines(guest, cpu, left, top, right, bottom, 0xff807020);
        }
        else
        {
            guest.setDword(kPanelActive, 0);
            drawRacePanelLines(guest, cpu, left, top, right, bottom, 0xff000040);
        }
        const reg32 scale = fe::direct<0x4d1400, port::text::keepTextScaleOf>(app, cpu);
        // The shade where the game's code pushes it, for the function to rewrite on a race's pause screens; the
        // scale kept in eax, as the call before leaves it.
        const reg32 shadeAt = guest.frame() - 4;
        guest.setDword(shadeAt, 0x3f19999a);
        fe::direct<0x451530, port::text::dimmed>(app, cpu, shadeAt, scale);
        guest::direct<0x441950, drawPanelMessages>(app, cpu, left + 1, top + 3);
        fe::direct<0x4d1410, port::text::restoreTextScaleOf>(app, cpu);
    }
    else
    {
        guest.setDword(kPanelActive, 0);
    }
    if (networkGone(guest))
    {
        closePanel(guest);
        return 4;
    }
    return 0;
}

/* sub_441040: the panel beside a network screen on every pass: sliding in and
 * out, its parts, the button that opens it (turning, without translucency a
 * plain shape), its messages.  It returns what ends the screen: the lobby's
 * race started (0xb), the screen's network done with (6), the network gone (4). */
x86::reg32 frontend::menua::panelFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x24);
    constexpr reg32 kEdge = 0xff1c1a73, kInner = 0xff0f0f4a;
    constexpr reg32 kPaneDark = 0xff05053a, kPaneLight = 0xff141358;
    constexpr s32 kOpenLeft = 0x110, kClosedLeft = 0x251, kSlide = 0x28;
    constexpr reg32 kPanelMost = 0x1b0;
    if (networkGone(guest))
        closePanel(guest);
    if (guest.dword(kInRace) != 0)
        return guest::direct<0x440d80, drawRacePanel>(app, cpu);
    if (guest.word(kPanelShapeId) == 0)
        return 0;

    reg32 buttonTurn;
    if (guest.dword(kPanelOpen) != 0)
    {
        buttonTurn = 0;
        const s32 at = s32At(guest, kPanelLeft);
        if (at > kOpenLeft)
        {
            guest.setDword(kPanelSliding, 0);
            guest.setDword(kPanelLeft, u(at - kSlide));
        }
        if (s32At(guest, kPanelLeft) < kOpenLeft)
            guest.setDword(kPanelLeft, kOpenLeft);
    }
    else
    {
        buttonTurn = 0xb4;
        const s32 at = s32At(guest, kPanelLeft);
        if (at < kClosedLeft)
            guest.setDword(kPanelLeft, u(at + kSlide));
        if (s32At(guest, kPanelLeft) > kClosedLeft)
            guest.setDword(kPanelLeft, kClosedLeft);
    }

    drawShape(guest, cpu, s16At(guest, kPanelShapeId), guest.dword(kPanelLeft), guest.dword(kPanelTop));
    const reg32 panelLeft = guest.dword(kPanelLeft);
    const reg32 left = panelLeft + 0x2f;
    const reg32 right = panelLeft + 0x170;
    const reg32 inner = right - 4;
    shadedRectangle(guest, cpu, left, 0x87, panelLeft + 0x31, kPanelMost, kInner);
    shadedRectangle(guest, cpu, inner, 0x87, right, kPanelMost, kEdge);
    shadedRectangle(guest, cpu, left, 0x87, right, 0x8b, kEdge);
    shadedRectangle(guest, cpu, left, 0x11d, right, 0x121, kEdge);
    shadedRectangle(guest, cpu, left, 0x199, right, 0x19d, kEdge);
    shadedRectangle(guest, cpu, left, 0x1ac, right, kPanelMost, kEdge);
    shadedRectangle(guest, cpu, left, 0x8b, inner, 0x8d, kInner);
    shadedRectangle(guest, cpu, left, 0x121, inner, 0x123, kInner);
    shadedRectangle(guest, cpu, left, 0x19d, inner, 0x19f, kInner);
    shadedRectangle(guest, cpu, left + 2, 0x8d, inner, 0x11d, kPaneLight, kPaneDark);
    shadedRectangle(guest, cpu, left + 2, 0x123, inner, 0x199, kPaneDark, kPaneLight);
    shadedRectangle(guest, cpu, left + 2, 0x19f, inner, 0x1ac, kPaneLight, kPaneDark);

    const reg32 buttonX = guest.dword(kPanelLeft) + 0xd;
    const bool overButton = guest::direct<0x49c240, menud::pointerOverRect>(app, cpu, buttonX, 0x18bu, 0x20u, 0x20u) != 0;
    const reg32 turnedX = buttonX + 0x10;
    const bool plainShapes = (guest.byte(kDisplayFlags) & kNoTranslucency) != 0;
    if (overButton)
    {
        if (plainShapes)
        {
            drawShape(guest, cpu, s16At(guest, kButtonOpenPlain), buttonX, 0x18b);
        }
        else
        {
            drawTurnedShape(guest, cpu, s16At(guest, kButtonOpenBack), turnedX, 0x19b, buttonTurn);
            const reg32 spin = guest.dword(kPanelSpin) + 0xa;
            guest.setDword(kPanelSpin, spin);
            drawTurnedShape(guest, cpu, s16At(guest, kButtonOpenPlain), turnedX, 0x19b, spin);
        }
    }
    else if (plainShapes)
    {
        drawShape(guest, cpu, s16At(guest, kButtonClosedBack), buttonX, 0x18b);
    }
    else
    {
        drawTurnedShape(guest, cpu, s16At(guest, kButtonClosedPlain), turnedX, 0x19b, buttonTurn);
        const reg32 spin = guest.dword(kPanelSpin) + 5;
        guest.setDword(kPanelSpin, spin);
        drawTurnedShape(guest, cpu, s16At(guest, kButtonClosedBack), turnedX, 0x19b, spin);
    }

    guest::direct<0x49ae30, menud::drawLobbyPlayers>(app, cpu, guest.dword(kPanelLeft) + 0x2f, guest.dword(kPanelTop));
    const reg32 width = right - left - 4;
    guest.setDword(kBoxLeft, left);
    guest.setDword(kBoxTop, 0x120);
    guest.setDword(kBoxRight, left + width);
    guest.setDword(kBoxBottom, 0x1ac);
    if (guest::direct<0x49c240, menud::pointerOverRect>(app, cpu, left, 0x120u, width, 0x8cu) != 0
        && !dialogShown(guest, cpu))
    {
        if (networkGone(guest))
            closePanel(guest);
        else
            guest.setDword(kPanelActive, 1);
        constexpr reg32 kLit = 0xff807020;
        shadedRectangle(guest, cpu, left - 1, 0x11f, right - 2, 0x121, kLit);
        shadedRectangle(guest, cpu, left - 2, 0x11f, left, 0x1ac, kLit);
        shadedRectangle(guest, cpu, right - 4, 0x11f, right - 2, 0x1ac, kLit);
        shadedRectangle(guest, cpu, left - 1, 0x19a, right - 2, 0x19c, kLit);
        shadedRectangle(guest, cpu, left - 2, 0x1ac, right - 2, 0x1ae, kLit);
    }
    else
    {
        guest.setDword(kPanelActive, 0);
    }
    guest::direct<0x441950, drawPanelMessages>(app, cpu, guest.dword(kPanelLeft) + 0x30, 0x11fu);

    if (guest.word(kNetClosing) == 0 && guest.word(kNetLeft) == 0 && guest.word(kNetState) == 0x21)
    {
        const reg32 met = fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x28, 0x21);
        fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, cpu, met);  // eax as the call before left it
        return 0xb;
    }
    if (guest.dword(kTyping) != 0 && fe::direct<0x49a520, frontend::neta::lobbyChanged>(app, cpu) == 0
        && fe::direct<0x49abd0, frontend::neta::allReady>(app, cpu) != 0)
    {
        if (const reg32 typing = guest.dword(kTyping))
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, typing);
            guest.setDword(kTyping, 0);
        }
        if (const reg32 messages = guest.dword(kMessageBuffer))
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, messages);
            guest.setDword(kMessageBuffer, 0);
        }
        fe::direct<0x49acd0, frontend::neta::playersToRace>(app, cpu);
        return 6;
    }
    if (guest.word(kNetLeft) != 0)
    {
        guest.setDword(kPanelOpen, 0);
        guest.setDword(kPanelLeft, kClosedLeft);
        return 4;
    }
    return 0;
}

namespace
{

reg32 overPanelButton(Guest& guest, x86::CPU& cpu)
{
    return guest::direct<0x49c240, frontend::menud::pointerOverRect>(guest.app(), cpu, guest.dword(kPanelLeft) + 0xd, 0x18bu,
                                                                      0x20u, 0x20u);
}

}  // namespace

/* sub_441680: whether the panel is to have the keys and the pointer: over its
 * button, or active. */
x86::reg32 frontend::menua::panelBusy(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (networkGone(guest))
    {
        closePanel(guest);
        return 0;
    }
    if (guest.dword(kInRace) != 0)
        return 0;
    if (overPanelButton(guest, cpu) != 0)
        return 1;
    return guest.dword(kPanelActive) != 0 ? 1 : 0;
}

/* sub_441700: a key for the panel: Enter over its button opens and closes it;
 * while it is active or open the rest are the typed line's, an arrow or so
 * leaving it open.  2 when the key is spent. */
x86::reg32 frontend::menua::panelKey(App* app, CPU& cpu, std::uint16_t key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    reg32 result = 0;
    if (networkGone(guest))
        closePanel(guest);
    if (guest.word(kPanelShapeId) == 0)
        return 0;
    if (key == kKeyEnter && guest.dword(kFromMouse) != 0 && guest.dword(kInRace) == 0 && overPanelButton(guest, cpu) != 0)
    {
        result = 2;
        guest.setDword(kPanelOpen, guest.dword(kPanelOpen) == 0 ? 1 : 0);
    }
    if (guest.dword(kPanelActive) == 0)
    {
        if (guest.dword(kPanelOpen) == 0)
            return result;
        const reg32 pressed = guest.dword(kKeyPressed);
        if (pressed == kKeyUp || pressed == kKeyRight || pressed == kKeyDown || pressed == kKeyLeft
            || pressed == kKeyEnter || pressed == kKeyEscape)
            return result;
        guest::direct<0x440d60, pointAtPanel>(app, cpu);
    }
    guest::direct<0x441b20, typeInPanel>(app, cpu, guest.dword(kKeyPressed));
    return 2;
}

/* sub_4418e0: how wide a message may be, in the box's width. */
x86::reg32 frontend::menua::panelLineWidth(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const s32 across = s32(guest.dword(kBoxRight) - guest.dword(kBoxLeft));
    const reg32 width = across > 0 ? u(across) : reg32(0) - u(across);
    return width - 0x14;
}

/* sub_441900: whether a message fits a line, in the panel's font. */
x86::reg32 frontend::menua::panelLineFits(App* app, CPU& cpu, reg32 text)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (text == 0 || guest.byte(text) == 0)
        return 1;
    fe::direct<0x452100, port::text::selectFontOf>(app, cpu, kMenuFont);
    const s32 width = s32(guest::direct<0x4418e0, panelLineWidth>(app, cpu));
    return fe::textWidth(guest, text, kMenuFont) <= width ? 1 : 0;
}

/* sub_441950: the panel's messages, each cut until it fits its line, and under
 * them the line being typed, as much of its end as fits, and the cursor. */
x86::reg32 frontend::menua::drawPanelMessages(App* app, CPU& cpu, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x7c);
    constexpr reg32 kNothing = 0x43f180;
    constexpr reg32 kPrompt = 0x23;
    constexpr reg32 kLineLength = 0x64;
    constexpr reg32 kMessageSize = 0x65;
    const reg32 line = guest.frame();
    for (reg32 i = 0; i < kLineLength; i += 4)
        guest.setDword(line + i, guest.dword(kNothing + i));
    guest.leaveEdx(0);
    if (guest.dword(kTyping) == 0 || guest.dword(kMessageBuffer) == 0)
        return x;
    guest.call(0x42d9b0);
    const reg32 messages = guest.call(0x437080, fe::args({ 0 }));
    reg32 lineY = y;
    for (s32 i = 0;; ++i)
    {
        guest.leaveEdx(messages);
        if (i >= s32At(guest, messages))
            break;
        guest.call(0x4e0e30, fe::args({ line, messages + 4 + u(i) * kMessageSize, kLineLength }));
        while (guest::direct<0x441900, panelLineFits>(app, cpu, line) == 0 && guest.byte(line) != 0)
            guest.setByte(line + guest.length(line) - 1, 0);
        drawText(guest, cpu, line, x, lineY, kColourChosen);
        lineY += 0xd;
    }
    fe::direct<0x452100, port::text::selectFontOf>(app, cpu, kMenuFont);
    if (guest.dword(kTyping) == 0)
        return kMenuFont;  // eax as sub_452100 leaves it: the font
    const reg32 typedY = y + 0x7c;
    const s32 room = s32(guest::direct<0x4418e0, panelLineWidth>(app, cpu));
    const s32 promptWidth = fe::textWidth(guest, fe::text(guest, kPrompt), kMenuFont);
    s32 fit = minus(room, promptWidth);
    if (fit <= 0)
        fit = minus(0, fit);
    reg32 skipped = 0;
    while (fe::textWidth(guest, guest.dword(kTyping) + skipped, kMenuFont) > fit)
        ++skipped;
    drawText(guest, cpu, fe::text(guest, kPrompt), x, typedY, kColourPlain);
    const reg32 typedX = x + u(promptWidth);
    drawText(guest, cpu, guest.dword(kTyping) + skipped, typedX, typedY, kColourChosen);
    const s32 blink = s32(guest.dword(kCursorBlink) + 1);
    const s32 phase = blink % 0x1e;
    guest.setDword(kCursorBlink, u(phase));
    if (phase >= 0xf)
    {
        guest.leaveEdx(u(phase));
        return u(blink / 0x1e);
    }
    const reg32 typedWidth = u(fe::textWidth(guest, guest.dword(kTyping) + skipped, kMenuFont));
    // By address, the last call: what it leaves in edx, the function leaves.
    return drawTextByAddress(guest, kCursor.address, typedX + typedWidth, typedY, kColourChosen);
}

/* sub_441b20: a key for the line being typed: Enter sends it with the player's
 * name before it, Backspace and Delete take the last character off, Escape
 * nothing, what may be typed goes on its end, up to 0x56 of them.  2 when the key
 * is spent. */
x86::reg32 frontend::menua::typeInPanel(App* app, CPU& cpu, reg32 pressed)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x64);
    const std::uint16_t key = std::uint16_t(pressed);
    constexpr reg32 kTypedMost = 0x56;
    const reg32 typing = guest.dword(kTyping);
    if (typing == 0 || guest.dword(kPanelOpen) == 0 || guest.dword(kFromMouse) != 0)
        return 0;
    if (key == kKeyEnter)
    {
        const reg32 message = guest.frame();   // char[0x64]
        guest.copyString(message, SETTINGS_ADDRESS(players[0].name));
        guest.copyString(message + guest.length(message), kNameSeparator.address);
        guest.call(0x4eedf0, fe::args({ message, guest.dword(kTyping), 0x57 }));
        guest.call(0x42d810, fe::args({ 0x100, message, 0x64 }));
        guest.call(0x4e0640, fe::args({ guest.dword(kTyping), 0, 0x64 }));
    }
    else if (key == kKeyBackspace || key == kKeyDelete)
    {
        const std::uint32_t length = guest.length(guest.dword(kTyping));
        if (length != 0)
            guest.setByte(guest.dword(kTyping) + length - 1, 0);
    }
    else if (key != kKeyEscape)
    {
        if ((guest::direct<0x443810, characterAllowed>(app, cpu, key & 0xffu, 3u) & 0xff) != 0)
        {
            const reg32 text = guest.dword(kTyping);
            const std::uint32_t length = guest.length(text);
            if (length < kTypedMost)
            {
                guest.setByte(text + length + 1, 0);
                guest.setByte(text + length, std::uint8_t(key));
            }
        }
    }
    return 2;
}

namespace
{

/* ---------------------------------------------------------------------------
 * What may be typed
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_4434f0: whether a character may be typed at all. */
x86::reg32 frontend::menua::characterInRange(App*, CPU&, reg32 character)
{
    const bool typable = s32(character) >= 0x20 && s32(character) <= 0xff;
    return withLowByte(character, typable ? 1 : 0);
}

/* sub_443510 and sub_443620: the same for a byte. */
x86::reg32 frontend::menua::byteInRange(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest::direct<0x4434f0, characterInRange>(app, cpu, character & 0xff);
}

/* sub_443620: the same. */
x86::reg32 frontend::menua::byteInRangeAgain(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest::direct<0x4434f0, characterInRange>(app, cpu, character & 0xff);
}

/* sub_4435c0: the characters of a number or a network address. */
x86::reg32 frontend::menua::characterOfAddress(App*, CPU&, reg32 character)
{
    static const std::uint8_t kAllowed[26] = { 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const std::uint8_t index = std::uint8_t((character & 0xff) - 0x20);
    const std::uint8_t allowed = index < std::size(kAllowed) ? kAllowed[index] : 0;
    return (character & 0xffff0000) | (reg32(index) << 8) | allowed;
}

namespace
{

/* sub_443520 and sub_4435e0: whether a whole text passes a character's test;
 * none does that is empty. */
template <typename Test>
reg32 wholeTextPasses(Guest& guest, reg32 text, Test test)
{
    if (text == 0 || guest.byte(text) == 0)
        return text & 0xffffff00;
    reg32 left = text;
    for (reg32 at = text;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (c == 0)
            return withLowByte(left, 1);
        left = test(c);
        if ((left & 0xff) == 0)
            return withLowByte(left, 0);
    }
}

}  // namespace

/* sub_443520: whether every byte of a text may be typed (sub_443510). */
x86::reg32 frontend::menua::textInRange(App* app, CPU& cpu, reg32 text)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return wholeTextPasses(guest, text, [&](std::uint8_t c) { return guest::direct<0x443510, byteInRange>(app, cpu, c); });
}

/* sub_4435e0: whether a whole text is a number or a network address (sub_4435c0). */
x86::reg32 frontend::menua::textOfAddress(App* app, CPU& cpu, reg32 text)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return wholeTextPasses(guest, text,
                           [&](std::uint8_t c) { return guest::direct<0x4435c0, characterOfAddress>(app, cpu, c); });
}

namespace
{

/* sub_4436c0, sub_443740, sub_4437c0: a character by the test of the language's,
 * of a table of three; any without one. */
reg32 languageCharacterTest(Guest& guest, reg32 character, reg32 table)
{
    const reg32 row = gameSettings(guest).language * 20;
    const reg32 test = guest.dword(table + row);
    if (test != 0)
        return guest.call(test, fe::args({ character & 0xff }));
    return withLowByte(row, 1);
}

}  // namespace

/* sub_4436c0, sub_443740, sub_4437c0: by the first, second and third tests of the table. */
x86::reg32 frontend::menua::languageTestFirst(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return languageCharacterTest(guest, character, 0x558c7c);
}

x86::reg32 frontend::menua::languageTestSecond(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return languageCharacterTest(guest, character, 0x558c84);
}

x86::reg32 frontend::menua::languageTestThird(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return languageCharacterTest(guest, character, 0x558c8c);
}

/* sub_443810: whether a character may be typed in a line of a kind (a name,
 * then the language's three). */
x86::reg32 frontend::menua::characterAllowed(App* app, CPU& cpu, reg32 character, reg32 kind)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    switch (kind)
    {
    case 0:
        return fe::direct<0x443630, text::shortNameCharacter>(app, cpu, character & 0xff);
    case 1:
        return guest::direct<0x4436c0, languageTestFirst>(app, cpu, character & 0xff);
    case 2:
        return guest::direct<0x443740, languageTestSecond>(app, cpu, character & 0xff);
    case 3:
        return guest::direct<0x4437c0, languageTestThird>(app, cpu, character & 0xff);
    default:
        break;
    }
    return (character & 0xffff0000) | ((character & 0xff) << 8);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The screen modes' list
 * ------------------------------------------------------------------------- */

constexpr reg32 kModeCount = 0x559218;
constexpr reg32 kModeLines = 0x5591ec;           // int16 for each: its line of the list
constexpr reg32 kModes = 0x661220;
constexpr reg32 kModeSize = 0x114;
constexpr reg32 kModeHeight = 4;

}  // namespace

/* sub_4482c0: the list made afresh from the driver, by way of sub_448210. */
x86::reg32 frontend::menua::listModes(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest.setDword(kModeCount, 0);
    guest.call(0x4e070c, fe::args({ kModeLines, 0x2a }));
    return guest.call(0x4bed90, fe::args({ 0x448210 }));
}

/* sub_4482f0: the list made, the mode chosen still in it, its width. */
x86::reg32 frontend::menua::chosenModeWidth(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest::direct<0x4482c0, listModes>(app, cpu);
    if (s32(gameSettings(guest).screenMode) >= s32At(guest, kModeCount))
        gameSettings(guest).screenMode = 0;
    return guest.dword(kModes + kModeSize * gameSettings(guest).screenMode);
}

/* sub_448330: the mode chosen, copied by way of the stack as the game's code
 * has it. */
x86::reg32 frontend::menua::copyChosenMode(App* app, CPU& cpu, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x114);
    const reg32 copy = guest.frame();
    if (s32At(guest, kModeCount) <= s32(gameSettings(guest).screenMode))
        gameSettings(guest).screenMode = 0;
    const reg32 mode = kModes + kModeSize * gameSettings(guest).screenMode;
    for (reg32 i = 0; i < kModeSize; i += 4)
        guest.setDword(copy + i, guest.dword(mode + i));
    for (reg32 i = 0; i < kModeSize; i += 4)
        guest.setDword(out + i, guest.dword(copy + i));
    guest.setEsi(copy + kModeSize);
    return out;
}

/* sub_4483a0: the list made, the mode of a width chosen; the first if none is. */
x86::reg32 frontend::menua::chooseModeOfWidth(App* app, CPU& cpu, reg32 width)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest::direct<0x4482c0, listModes>(app, cpu);
    for (s32 i = 0; i < s32At(guest, kModeCount); ++i)
    {
        const reg32 mode = kModes + kModeSize * u(i);
        if (width == guest.dword(mode))
        {
            const reg32 height = guest.dword(mode + kModeHeight);
            gameSettings(guest).screenMode = u(i);
            gameSettings(guest).screenWidth = height;
            return height;
        }
    }
    const reg32 height = guest.dword(kModes + kModeHeight);
    gameSettings(guest).screenWidth = height;
    gameSettings(guest).screenMode = 0;
    return height;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The race's pause menu
 * ------------------------------------------------------------------------- */

constexpr reg32 kPauseNextAnswer = 0x559254;     // the answer to take next, set from outside
constexpr reg32 kPauseDone = 0x559228;
constexpr reg32 kCurrentMenu = 0x559230;
constexpr reg32 kFirstMenu = 0x6040d0;
constexpr reg32 kLastAnswer = 0x66296c;
constexpr reg32 kPauseActive = 0x8b7630;
constexpr reg32 kScreenStack = 0x6627fc;         // [1] to [20]
constexpr reg32 kMenuCount = 0x660bb0;
constexpr reg32 kItemShownBefore = 0x1c;         // of the screen: int16, the item selected before

constexpr GuestString kPauseFile{ 0x53905c, "pause" };
constexpr GuestString kPauseMultiFile{ 0x539054, "mspause" };
constexpr GuestString kPauseAdditionsFile{ 0x539064, "ad" };
constexpr GuestString kContinueItem{ 0x539068, "PSE_CONTINUE" };
constexpr GuestString kRestartItem{ 0x539078, "PSE_RESTART" };

void clearScreenStack(Guest& guest)
{
    for (reg32 i = 1; i <= 0x14; ++i)
        guest.setDword(kScreenStack + 4 * i, 0);
}

reg32 enterScreen(Guest& guest, x86::CPU& cpu, reg32 menu)
{
    return u(*fe::direct<0x44ab40, port::loop::enterScreenOf>(guest.app(), cpu, cpu.esp - 4, menu));
}

reg32 leaveScreen(Guest& guest, x86::CPU& cpu, reg32 menu, reg32 action)
{
    return u(*fe::direct<0x44aa30, port::loop::leaveScreenOf>(guest.app(), cpu, cpu.esp - 4, menu, action));
}

}  // namespace

/* sub_44b950: the pause menu as a race pauses for the first time: its text and
 * its screens loaded, its first screen open. */
x86::reg32 frontend::menua::racePauseMenuBegin(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    constexpr reg32 kCommandLine = 0x55eb2e;     // byte
    constexpr std::uint8_t kAdditions = 0x20;
    constexpr reg32 kPlayerState = 0x5efac8;
    guest.setDword(kInRace, 1);
    guest.setDword(kPauseNextAnswer, 0);
    guest.setDword(kPauseDone, 0);
    fe::direct<0x4d1750, port::text::loadedLanguageText>(app, cpu, cpu.esp - 4, s32(gameSettings(guest).language));
    clearScreenStack(guest);
    guest.setDword(kLastAnswer, 5);
    bool multi = false;
    if (guest.byte(kCommandLine) & kAdditions)
    {
        const reg32 player = guest.dword(kPlayerState);
        multi = (guest.byte(player + 0x200) & 1) != 0 && (guest.dword(player + 0x2c4) == 3 || guest.dword(player + 0x2c4) == 4);
    }
    // ebx and ecx as they are handed on.
    const auto load = [&](reg32 name, reg32 reset) {
        fe::direct<0x449510, port::menus::menusLoaded>(app, cpu, cpu.esp - 4, name, reset,
                                                       port::Args{ 0, 0, cpu.ebx, cpu.ecx });
    };
    if (multi)
    {
        load(kPauseMultiFile.address, 1);
    }
    else
    {
        load(kPauseFile.address, 1);
        if (guest.byte(kCommandLine) & kAdditions)
            load(kPauseAdditionsFile.address, 0);
    }
    guest.setDword(kCurrentMenu, kFirstMenu);
    const reg32 entered = enterScreen(guest, cpu, kFirstMenu);
    guest.setDword(kPauseActive, 1);
    return entered;
}

/* sub_44ba30: the pause menu as a race pauses again, its screens loaded. */
x86::reg32 frontend::menua::racePauseMenuAgain(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    guest.setDword(kPauseNextAnswer, 0);
    guest.setDword(kInRace, 1);
    clearScreenStack(guest);
    guest.setDword(kLastAnswer, 5);
    guest.setDword(kCurrentMenu, kFirstMenu);
    const reg32 entered = enterScreen(guest, cpu, kFirstMenu);
    guest.setDword(kPauseActive, 1);
    return entered;
}

namespace
{

reg32 selectedItemOf(const Guest& guest, reg32 menu)
{
    const s32 index = s16At(guest, menu + fe::menu::kFirstItem) + s16At(guest, menu + fe::menu::kSelected);
    return fe::item::kTable + u(index) * fe::item::kSize;
}

/* What the pointer is on and the key, for sub_44bac0: the answer of the dialog
 * or of the screen, 0 for none. */
reg32 racePauseMenuInput(Guest& guest, x86::CPU& cpu)
{
    win32::WinApplication* const app = guest.app();
    const reg32 under = u(*fe::direct<0x44a6b0, port::loop::itemUnderMouseOf>(app, cpu, cpu.esp - 4,
                                                                              guest.dword(kCurrentMenu)));
    const reg32 menu = guest.dword(kCurrentMenu);
    const s32 selected = s16At(guest, menu + fe::menu::kSelected);
    guest.leaveEdx(menu);   // the screen in edx, as the game's code has it (sub_449e50 takes it there)
    if (u(selected) != under
        && *fe::direct<0x449e50, port::loop::selectableOf>(app, cpu, cpu.esp - 4, selected, menu) != 0)
    {
        const reg32 current = guest.dword(kCurrentMenu);
        guest.setWord(current + kItemShownBefore, guest.word(current + fe::menu::kSelected));
    }
    guest.setWord(guest.dword(kCurrentMenu) + fe::menu::kSelected, under == 0xffffffff ? 0x64 : std::uint16_t(under));
    const reg32 key = u(*fe::direct<0x451960, port::text::menuKeyOf>(app, cpu, cpu.esp - 4, 1u));
    if ((key & 0xffff) == 0)
        return 0;
    const reg32 asWord = u(std::int16_t(key));
    if (dialogShown(guest, cpu))
        return u(*fe::direct<0x445e60, port::dialogs::dialogKeyOf>(app, cpu, cpu.esp - 4, asWord));
    return u(*fe::direct<0x44a2d0, port::loop::handleKeyOf>(app, cpu, cpu.esp - 4, guest.dword(kCurrentMenu), asWord));
}

/* The selected button followed to its next screen, for the answer 5 of sub_44bac0:
 * 5 when it is opened, 3 to go back when there is none, or the button is the
 * screen's Escape. */
reg32 racePauseFollowButton(Guest& guest, x86::CPU& cpu)
{
    const reg32 menu = guest.dword(kCurrentMenu);
    const reg32 selected = selectedItemOf(guest, menu);
    guest.setDword(guest.frame(), 0);
    if (u(*fe::direct<0x44a240, port::loop::escapeButtonOf>(guest.app(), cpu, cpu.esp - 4, menu, guest.frame())) == selected)
        return 3;
    const reg32 next = guest.dword(selected) == 5 ? guest.dword(selected + fe::item::kNextMenu) : 0;
    if (next == 0)
        return 3;
    reg32 index = 0;
    while (s32(index) < s32At(guest, kMenuCount)
           && guest.call(0x4ee310, fe::args({ next, guest.dword(kFirstMenu + fe::menu::kName + index * fe::menu::kSize) })) != 0)
        ++index;
    if (s32(index) >= s32At(guest, kMenuCount))
        return 3;
    fe::direct<0x44af10, port::loop::pushMenuOf>(guest.app(), cpu, guest.eax());  // eax as strcmp left it
    leaveScreen(guest, cpu, guest.dword(kCurrentMenu), 5);
    const reg32 to = kFirstMenu + index * fe::menu::kSize;
    guest.setDword(kCurrentMenu, to);
    enterScreen(guest, cpu, to);
    return 5;
}

}  // namespace

/* sub_44bac0: a pass of the pause menu: what the pointer is on, the key, the
 * answer it gives (6, 10, 11, 12 close the screen, 9 reopens it, 5 follows the
 * selected button to its next screen, 3 and 1 go back, the first screen
 * closing).  The screen open then has its pass; otherwise the answer says what
 * the race is to do: 3, 4, 5 by the answer, 1 and 2 by the selected item. */
x86::reg32 frontend::menua::racePauseMenuFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0xc);
    bool screenStaysOpen = true;
    reg32 answer = guest.dword(kPauseNextAnswer);
    if (answer != 0)
        guest.setDword(kPauseNextAnswer, 0);
    else
        answer = racePauseMenuInput(guest, cpu);
    if (answer != 0)
    {
        guest.setDword(kLastAnswer, answer);
        switch (answer)
        {
        case 5:
            answer = racePauseFollowButton(guest, cpu);
            break;
        case 6:
        case 10:
        case 11:
        case 12:
            leaveScreen(guest, cpu, guest.dword(kCurrentMenu), answer);
            screenStaysOpen = false;
            break;
        case 9:
            leaveScreen(guest, cpu, guest.dword(kCurrentMenu), answer);
            enterScreen(guest, cpu, guest.dword(kCurrentMenu));
            break;
        default:
            break;
        }
        if (answer == 3 || answer == 1)
        {
            const reg32 menu = guest.dword(kCurrentMenu);
            if (menu == kFirstMenu)
            {
                screenStaysOpen = false;
            }
            else
            {
                const reg32 left = leaveScreen(guest, cpu, menu, answer);
                fe::direct<0x44aec0, port::loop::popMenuOf>(app, cpu, left);  // eax as the call before left it
                enterScreen(guest, cpu, guest.dword(kCurrentMenu));
            }
        }
    }
    if (screenStaysOpen)
    {
        const reg32 menu = guest.dword(kCurrentMenu);
        const reg32 handler = guest.dword(menu + fe::menu::kOnFrame);
        if (handler != 0)
            guest.call(handler, fe::args({ menu, menu }));
        return 0;
    }
    if (answer == 12)
        return 3;
    if (answer == 11)
        return 4;
    if (answer == 10)
        return 5;
    const reg32 selected = selectedItemOf(guest, guest.dword(kCurrentMenu));
    if (selected == 0 || guest.dword(selected + fe::item::kCodelink) == 0)
        return 0;
    // ebx as it is, handed on.
    const auto names = [&](const GuestString& name) {
        return *fe::direct<0x4429c0, port::menus::codelinkMatched>(app, cpu, cpu.esp - 4,
                                                                   guest.dword(selected + fe::item::kCodelink),
                                                                   name.address, cpu.ebx) != 0;
    };
    if (names(kContinueItem))
        return 1;
    if (names(kRestartItem))
        return 2;
    return 0;
}

namespace
{

const GuestString kBindingStrings[] = { kFormatDevice, kFormatCharacter, kDegrees, kFormatHat };
const GuestString kListStrings[] = { kFormatPlayer1, kFormatPlayer2 };
const GuestString kPrepareStrings[] = { kRowShapeName, kRowLitShapeName };
const GuestString kKeyNameStrings[] = { kFormatKey };
const GuestString kApplyStrings[] = { kCarSelect1, kCarSelect2 };
const GuestString kPickerStrings[] = { kPickerShapes[0], kPickerShapes[1], kPickerShapes[2], kPickerShapes[3], kPickerShapes[4] };
const GuestString kMessagesStrings[] = { kCursor };
const GuestString kTypeStrings[] = { kNameSeparator };
const GuestString kBeginStrings[] = { kPauseFile, kPauseMultiFile, kPauseAdditionsFile };
const GuestString kFrameStrings[] = { kContinueItem, kRestartItem };

constexpr std::uint8_t kEbxEcx = fe::kRestoreEbx | fe::kRestoreEcx;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEdx = fe::kRestoreEbx | fe::kRestoreEdx;
constexpr std::uint8_t kEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kNoRestore = 0;

}

/* The stand-ins: the frames and registers of the generated functions. */

/* sub_448210, the driver's list of modes handed to it one by one: a mode for the Screen
 * Size list, up to 20, with its line, "640 x 480 x 16", " (z)" when it has a depth
 * buffer.  The depth shown is the one a race is drawn in (tools/apply_full_colour.py's
 * reason), and Dynamic Fullscreen's line its name (tools/apply_widescreen.py's). */
static constexpr GuestString kModeFormat{ 0x538d48, "%d x %d x %d" };
static constexpr GuestString kModeHasDepthBuffer{ 0x538d58, " (z)" };
static const GuestString kModeStrings[] = { kModeFormat, kModeHasDepthBuffer };

x86::reg32 frontend::menua::addScreenMode(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 depth,
                                          reg32 depthBuffer)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    constexpr reg32 kModeCount = 0x559218, kModes = 0x661220, kModeSize = 0x114;
    constexpr reg32 kModeLineIds = 0x5591ec, kFirstLineId = 0x7a8;
    constexpr std::int32_t kMostModes = 20;
    const reg32 count = guest.dword(kModeCount);
    const reg32 entry = kModes + count * kModeSize;
    if (std::int32_t(count) >= kMostModes)
    {
        guest.leaveEdx(entry);
        return count * kModeSize;
    }
    const reg32 shown = shownDepth(depth);
    guest.setDword(entry, width);
    guest.setDword(entry + 4, height);
    guest.setDword(entry + 8, bits);
    guest.setDword(entry + 0xc, depth);
    guest.setDword(entry + 0x10, depthBuffer);
    const reg32 line = entry + 0x14;
    const reg32 answer = guest.call(0x4df690, fe::args({ depthBuffer, entry }, { line, kModeFormat.address, height, bits, shown }));
    // What is left in ecx: the buffer flag, or what looking for the line's end made of it.
    reg32 ecx = guest.dword(entry + 0x10);
    if (ecx != 0)
    {
        const reg32 length = guest.length(line);
        guest.copyString(line + length, kModeHasDepthBuffer.address);
        ecx = 0xffffffff - (length + 1);
    }
    dynamicFullscreenLabel(app, entry);
    const reg32 index = guest.dword(kModeCount);
    const reg32 id = ((answer & 0xffff0000) | guest.word(kModeCount)) + kFirstLineId;
    guest.setDword(kModeCount, index + 1);
    guest.setWord(kModeLineIds + index * 2, std::uint16_t(id));
    guest.leaveEbx(index + 1);
    guest.leaveEcx(ecx);
    guest.leaveEdx(index);
    return id;
}

void frontend::controlsPollJoystickBinding(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43be40", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pollJoystickBinding(app, cpu); });
}

void frontend::screenModeListAdd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448210", 4, 0, kModeStrings, std::size(kModeStrings), kEsiEdi };
    const reg32 width = cpu.eax, height = cpu.edx, bits = cpu.ebx, depth = cpu.ecx;
    const reg32 depthBuffer = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::addScreenMode(app, cpu, width, height, bits, depth, depthBuffer); });
}

void frontend::controlsDrawBinding(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43c110", 4, 0, kBindingStrings, std::size(kBindingStrings), kEsiEdi };
    const reg32 label = cpu.eax, binding = cpu.edx, x = cpu.ebx, y = cpu.ecx;
    const reg32 chosen = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawBinding(app, cpu, label, binding, x, y, chosen); });
}

void frontend::controlsDrawList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43c900", 0, 0, kListStrings, std::size(kListStrings), kEbxEcx | kEsiEdi };
    const reg32 item = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::controlsList(app, cpu, item, wanted); });
}

void frontend::controlsKeyAsBinding(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43cda0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::keyAsBinding(app, cpu, character); });
}

void frontend::controlsWarnDigitalBinding(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ce80", 0, 0, nullptr, 0, kEbxEcx };
    const reg32 slot = cpu.eax, binding = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::warnDigitalBinding(app, cpu, slot, binding); });
}

void frontend::controlsListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43cf80", 0, 0, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const reg32 item = cpu.eax, menu = cpu.edx;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::listKey(app, cpu, item, menu, key); });
}

void frontend::controlsListFitBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d2b0", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::listFitBox(app, cpu, item); });
}

void frontend::controlsListPrepare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d310", 0, 0, kPrepareStrings, std::size(kPrepareStrings) };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::listPrepare(app, cpu, item); });
}

void frontend::controlsWaitJoystickRelease(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d650", 0, 0, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::waitJoystickRelease(app, cpu); });
}

void frontend::controlsClearConflicts(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d6a0", 0, 0, nullptr, 0 };
    const reg32 binding = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::clearConflictingBindings(app, cpu, binding); });
}

void frontend::controlsBindingKeyName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d7f0", 0, 0, kKeyNameStrings, std::size(kKeyNameStrings), kEsiEdi };
    const reg32 binding = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::bindingKeyName(app, cpu, binding, buffer); });
}

void frontend::controlsSampleAxes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d920", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::sampleAxesAtRest(app, cpu); });
}

void frontend::colourMakeHueRamp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43db20", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::makeHueRamp(app, cpu); });
}

void frontend::colourApplyToCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43dc90", 0, 0, kApplyStrings, std::size(kApplyStrings) };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::applyColourToCar(app, cpu, item); });
}

void frontend::colourAccept(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43dd90", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::acceptColour(app, cpu, item); });
}

void frontend::colourRevert(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ddd0", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::revertColour(app, cpu, item); });
}

void frontend::colourHsvToArgb(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43de20", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 hsv = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::hsvToArgb(app, cpu, hsv, out); });
}

void frontend::colourTriangleFromColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43de60", 8, 0, nullptr, 0, kEsiEdi };
    const s32 width = s32(cpu.eax), height = s32(cpu.edx), across = s32(cpu.ebx), down = s32(cpu.ecx);
    const reg32 outX = app->getMemory<reg32>(cpu.esp + 4), outY = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::triangleFromColour(app, cpu, width, height, across, down, outX, outY); });
}

void frontend::colourFromTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43def0", 8, 0, nullptr, 0, kEsiEdi };
    const s32 width = s32(cpu.eax), height = s32(cpu.edx);
    const reg32 outAcross = cpu.ebx, outDown = cpu.ecx;
    const s32 x = s32(app->getMemory<reg32>(cpu.esp + 4)), y = s32(app->getMemory<reg32>(cpu.esp + 8));
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::colourAtPlace(app, cpu, width, height, outAcross, outDown, x, y); });
}

void frontend::colourDrawFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43dfb0", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawPickerFrame(app, cpu, item); });
}

void frontend::colourDrawSwatch(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e040", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawColourSwatch(app, cpu, item); });
}

void frontend::colourDrawButtons(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e0d0", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawPickerButtons(app, cpu, item); });
}

void frontend::colourDrawMarkers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e1f0", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawColourMarkers(app, cpu, item); });
}

void frontend::colourDrawTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e310", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawColourTriangle(app, cpu, item); });
}

void frontend::colourDrawValueBar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e490", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawValueBar(app, cpu, item); });
}

void frontend::colourDragMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e7a0", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerDragMoved(app, cpu, item); });
}

void frontend::colourHitRegion(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43e8e0", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerHitRegion(app, cpu, item); });
}

void frontend::colourMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ead0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerMouse(app, cpu, item); });
}

void frontend::colourDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ebc0", 0, 0, nullptr, 0, kEbxEcx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawColourPicker(app, cpu, item); });
}

void frontend::colourKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ec40", 0, 0, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const reg32 item = cpu.eax, menu = cpu.edx;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerKey(app, cpu, item, menu, key); });
}

void frontend::colourPointerKind(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43ef80", 0, 0, nullptr, 0, kEcxEdx | fe::kRestoreEsi };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerPointerKind(app, cpu, item); });
}

void frontend::colourFitBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43eff0", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerFitBox(app, cpu, item); });
}

void frontend::colourPrepare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43f030", 0, 0, kPickerStrings, std::size(kPickerStrings), kEbxEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerPrepare(app, cpu, item); });
}

void frontend::colourShown(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_43f160: whether the picker is shown now.
    static fe::Site site{ "sub_43f160", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pickerShown(app, cpu, item); });
}

void frontend::lobbyDrawPlayerIcon(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440b30", 0, 0, nullptr, 0, kEbxEcx };
    const reg32 x = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawPlayerIcon(app, cpu, x, y); });
}

void frontend::lobbyDrawPulsingIcon(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440b50", 0, 0, nullptr, 0, kEsiEdi };
    const reg32 chosen = cpu.eax, index = cpu.edx, x = cpu.ebx, y = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawPulsingIcon(app, cpu, chosen, index, x, y); });
}

void frontend::lobbyPointAtPanel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440d60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::pointAtPanel(app, cpu); });
}

void frontend::lobbyDrawRacePanel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440d80", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawRacePanel(app, cpu); });
}

void frontend::lobbyPanelFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441040", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::panelFrame(app, cpu); });
}

void frontend::lobbyPanelBusy(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441680", 0, 0, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::panelBusy(app, cpu); });
}

void frontend::lobbyPanelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441700", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    const std::uint16_t key = std::uint16_t(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::panelKey(app, cpu, key); });
}

void frontend::lobbyPanelLineWidth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4418e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::panelLineWidth(app, cpu); });
}

void frontend::lobbyPanelLineFits(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441900", 0, 0, nullptr, 0, kEbxEcxEdx };
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::panelLineFits(app, cpu, text); });
}

void frontend::lobbyDrawMessages(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441950", 0, 0, kMessagesStrings, std::size(kMessagesStrings), kEbxEcx | kEsiEdi };
    const reg32 x = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::drawPanelMessages(app, cpu, x, y); });
}

void frontend::lobbyTypeInPanel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441b20", 0, 0, kTypeStrings, std::size(kTypeStrings) };
    const reg32 pressed = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::typeInPanel(app, cpu, pressed); });
}

void frontend::typedCharacterInRange(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4434f0", 0, 0, nullptr, 0, kNoRestore };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::characterInRange(app, cpu, character); });
}

void frontend::typedByteInRange(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443510", 0, 0, nullptr, 0, kNoRestore };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::byteInRange(app, cpu, character); });
}

void frontend::typedTextInRange(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443520", 0, 0, nullptr, 0, kEbxEdx };
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::textInRange(app, cpu, text); });
}

void frontend::typedCharacterOfAddress(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4435c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::characterOfAddress(app, cpu, character); });
}

void frontend::typedTextOfAddress(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4435e0", 0, 0, nullptr, 0, kEbxEdx };
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::textOfAddress(app, cpu, text); });
}

void frontend::typedByteInRangeAgain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443620", 0, 0, nullptr, 0, kNoRestore };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::byteInRangeAgain(app, cpu, character); });
}

void frontend::typedLanguageTestFirst(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4436c0", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::languageTestFirst(app, cpu, character); });
}

void frontend::typedLanguageTestSecond(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443740", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::languageTestSecond(app, cpu, character); });
}

void frontend::typedLanguageTestThird(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4437c0", 0, 0, nullptr, 0, kEcxEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::languageTestThird(app, cpu, character); });
}

void frontend::typedCharacterAllowed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443810", 0, 0, nullptr, 0, kNoRestore };
    const reg32 character = cpu.eax, kind = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::characterAllowed(app, cpu, character, kind); });
}

void frontend::modesList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4482c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::listModes(app, cpu); });
}

void frontend::modesChosenWidth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4482f0", 0, 0, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::chosenModeWidth(app, cpu); });
}

void frontend::modesCopyChosen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448330", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEdi };
    const reg32 out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::copyChosenMode(app, cpu, out); });
}

void frontend::modesChooseByWidth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4483a0", 0, 0, nullptr, 0, kEbxEdx | fe::kRestoreEdi };
    const reg32 width = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::chooseModeOfWidth(app, cpu, width); });
}

void frontend::racePauseBegin(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44b950", 0, 0, kBeginStrings, std::size(kBeginStrings) };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::racePauseMenuBegin(app, cpu); });
}

void frontend::racePauseAgain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ba30", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::racePauseMenuAgain(app, cpu); });
}

void frontend::racePauseFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44bac0", 0, 0, kFrameStrings, std::size(kFrameStrings) };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menua::racePauseMenuFrame(app, cpu); });
}

}
