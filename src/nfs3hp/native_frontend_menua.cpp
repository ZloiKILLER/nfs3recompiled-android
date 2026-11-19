#include "native_frontend_menua.h"
#include "native_load_carmodels.h"
#include "native_raceload.h"
#include "native_collide.h"
#include "native_watcom.h"
#include "native_game.h"
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
#include "native_guest.h"
#include "native_vertices.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

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
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

// The port's own, in nfs3hp_main.cpp.
nfs::u32 shownDepth(nfs::u32 depth);
void dynamicFullscreenLabel(nfs::App* app, nfs::u32 entry);


namespace
{

using fe::Guest;
using fe::GuestString;
using nfs::u32;
using s32 = std::int32_t;

/* ---------------------------------------------------------------------------
 * The game's, by address
 * ------------------------------------------------------------------------- */

constexpr u32 kDrawText = 0x452190;
constexpr u32 kDrawShape = 0x4d7990;
constexpr u32 kDrawArtShape = 0x4d7160;
constexpr u32 kDrawTurnedShape = 0x4d6730;
constexpr u32 kDrawTexturedQuad = 0x4db000;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kPlayEffect = 0x4181d0;
constexpr u32 kSetPointer = 0x49bfc0;
constexpr u32 kHsvToRgb = 0x49ca50;
constexpr u32 kSetRenderState = 0x431900;

/* A float of 1, as the turned shapes' scale goes on the stack. */
constexpr u32 kFloatOne = 0x3f800000;

constexpr u32 kInRace = 0x559234;
constexpr u32 kPointerX = 0x55e5e4;
constexpr u32 kPointerY = 0x55e5e8;
constexpr u32 kDisplayFlags = 0x7a3a58;        // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr u32 kFromMouse = fe::kInputFromMouse;

constexpr u32 kMenuFont = 0x12;
constexpr u32 kColourChosen = 0xffe440;
constexpr u32 kColourPlain = 0xff649dfd;
constexpr std::uint32_t kEffectVolume = 0x7f;

constexpr u32 u(s32 value) { return u32(value); }

s32 s16At(const Guest& guest, u32 address) { return std::int16_t(guest.word(address)); }
s32 s32At(const Guest& guest, u32 address) { return s32(guest.dword(address)); }

/* The sign-extended low byte: the kind of a binding, as the game tests it. */
s32 lowByte(u32 value) { return std::int8_t(value & 0xff); }

/* A value halved towards zero. */
s32 halved(s32 value) { return s32(u32(value) - u32(value >> 31)) >> 1; }

s32 times(s32 a, s32 b) { return s32(u32(a) * u32(b)); }
s32 minus(s32 a, s32 b) { return s32(u32(a) - u32(b)); }

u32 withLowByte(u32 value, std::uint8_t low) { return (value & 0xffffff00) | low; }
u32 withLowWord(u32 value, std::uint16_t low) { return (value & 0xffff0000) | low; }

/* The game's functions the screens here call most, as C++ (guest::direct); a core of the port's
 * that takes `esp` gets the esp it would have from the game's call: below the arguments pushed and
 * the return address. */

/* Whether an item is shown.  The game's code has the widget's kind in edx, which the function does
 * not read and the code after it does: it is left there. */
u32 shownNow(Guest& guest, nfs::ThreadState& ts, u32 item, u32 widget)
{
    return fe::direct<0x44a660, frontend::loop::shownNow>(guest.app(), ts, item);
}

/* Line `number` of the language's text (edx, which only a name past the lines reads, as it is). */
u32 languageText(Guest& guest, nfs::ThreadState& ts, u32 number)
{
    return u(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), ts, ts.sp - 4, s32(number), 0));
}

u32 drawMenuText(Guest& guest, nfs::ThreadState& ts, u32 text, u32 x, u32 y, u32 align, u32 colour)
{
    return u(*fe::direct<0x452190, port::text::menuTextDrawn>(guest.app(), ts, ts.sp - 4 - 4 * 2, text, s32(x),
                                                              s32(y), kMenuFont, s32(align), colour));
}

u32 drawText(Guest& guest, nfs::ThreadState& ts, u32 text, u32 x, u32 y, u32 colour)
{
    return drawMenuText(guest, ts, text, x, y, 0, colour);
}

u32 drawShape(Guest& guest, nfs::ThreadState& ts, s32 shape, u32 x, u32 y)
{
    return u(*fe::direct<0x4d7990, port::shapes::drawShapeOf>(guest.app(), ts, ts.sp - 4, shape, s32(x), s32(y)));
}

u32 drawTurnedShape(Guest& guest, nfs::ThreadState& ts, s32 shape, u32 x, u32 y, u32 angle)
{
    return u(*fe::direct<0x4d6730, port::buttons::turnedShape>(guest.app(), ts, ts.sp - 4 - 4 * 2, shape, s32(x),
                                                               s32(y), 0, s32(angle), kFloatOne));
}

u32 playEffect(Guest& guest, nfs::ThreadState& ts, u32 effect)
{
    return u(*fe::direct<0x4181d0, port::playMenuEffectOf>(guest.app(), ts, ts.sp - 4, s32(effect),
                                                           s32(kEffectVolume)));
}

void setPointer(Guest& guest, nfs::ThreadState& ts, u32 x, u32 y)
{
    fe::direct<0x49bfc0, frontend::screen::setPointer>(guest.app(), ts, x, y);
}

/* The rectangle from colourA at its top to colourB at its bottom. */
u32 shadedRectangle(Guest& guest, nfs::ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 colourA,
                      u32 colourB)
{
    const std::uint32_t colours[4] = { colourA, colourA, colourB, colourB };
    return u(*fe::direct<0x4d81b0, port::draw2d::shadedRectangle>(guest.app(), ts, ts.sp - 4 - 4 * 4, s32(left),
                                                                  s32(top), s32(right), s32(bottom), colours));
}

u32 shadedRectangle(Guest& guest, nfs::ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 colour)
{
    return shadedRectangle(guest, ts, left, top, right, bottom, colour, colour);
}

u32 filledRectangle(Guest& guest, nfs::ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 colour)
{
    return u(*fe::direct<0x4d8910, port::draw2d::filled>(guest.app(), ts, ts.sp - 4 - 4, s32(x), s32(y), s32(width),
                                                         s32(height), colour));
}

u32 frameRectangle(Guest& guest, nfs::ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 colour)
{
    return u(*fe::direct<0x4d8930, port::draw2d::outline>(guest.app(), ts, ts.sp - 4 - 4, s32(x), s32(y),
                                                          s32(width), s32(height), colour));
}

u32 shapeNamed(Guest& guest, nfs::ThreadState& ts, u32 name)
{
    return u(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(guest.app(), ts, ts.sp - 4, name));
}

void renderState(Guest& guest, nfs::ThreadState& ts, u32 state, u32 value)
{
    fe::direct<0x431900, graphics::sendState>(guest.app(), ts, state, value);
}

bool dialogShown(Guest& guest, nfs::ThreadState& ts)
{
    return fe::direct<0x4438b0, port::dialogs::dialogShownOf>(guest.app(), ts) != 0;
}

u32 readJoystick(Guest& guest, nfs::ThreadState& ts, u32 device, u32 state)
{
    return *fe::direct<0x4358f0, controls::readState>(guest.app(), ts, device, state);
}

u32 runSystemTasks(Guest& guest, nfs::ThreadState& ts)
{
    return u(*fe::direct<0x4e7630, port::stream::runDueTasks>(guest.app(), ts, ts.sp - 4, 0u));
}

u32 controlsFor(Guest& guest, nfs::ThreadState& ts)
{
    return fe::direct<0x4904d0, controls::controlSets>(guest.app(), ts, gameSettings(guest).playMode);
}

/* The same for the last call of a function, whose result it gives back. */
u32 drawTextLast(Guest& guest, u32 text, u32 x, u32 y, u32 colour)
{
    return drawText(guest, guest.ts(), text, x, y, colour);
}

u32 drawShapeLast(Guest& guest, s32 shape, u32 x, u32 y)
{
    return drawShape(guest, guest.ts(), shape, x, y);
}

u32 playEffectLast(Guest& guest, u32 effect)
{
    return playEffect(guest, guest.ts(), effect);
}

u32 setPointerLast(Guest& guest, u32 x, u32 y)
{
    return fe::direct<kSetPointer, frontend::screen::setPointer>(guest.app(), guest.ts(), x, y);
}

/* ---------------------------------------------------------------------------
 * The controls' list
 * ------------------------------------------------------------------------- */

/* A joystick as sub_4358f0 reads it into a record of the caller's. */
namespace joystick
{
constexpr u32 kButtons = 0x30;   // a byte each, 32
constexpr u32 kAxes = 0x50;      // int32 each, 8
constexpr u32 kAnyHeld = 0x70;
constexpr u32 kHats = 0x74;      // int32 each, 4
}

// The devices: a record of 0x88 for each, its first word nonzero when it is on,
// which of its axes count at +0x40.
constexpr u32 kAxisRest = 0x5f3d50;            // int32[8] for each device

// The list's layout, from sub_43d310.
constexpr u32 kRowHeight = 0x555608;
constexpr u32 kPaneGap = 0x5f3da0;             // from one player's column to the other's
constexpr u32 kColumnLeft = 0x5f3d9c;
constexpr u32 kBindingWidth = 0x5f3d98;        // from a binding's label to what it is
constexpr u32 kPointerPlaced = 0x555640;       // the keys have moved the pointer already
constexpr u32 kHeardBinding = 0x5f3d94;        // a binding the joystick gave, not yet taken
constexpr u32 kBindingBefore = 0x5f3d90;       // the binding being changed, for Escape
constexpr u32 kBlink = 0x555644;
constexpr u32 kLabelGaps = 0x55560c;           // int32 for each row: from a row's left to its label
constexpr u32 kRowShape = 0x5f3daa;            // int16, con2
constexpr u32 kRowLitShape = 0x5f3da8;         // int16, con1
constexpr u32 kBindingIcons = 0x5f3d38;        // int16 for each icon
constexpr u32 kHatArrow = 0x5f3d46;            // int16
constexpr u32 kButtonsTexture = 0x749a38;

// What the list's item keeps: the box the pointer has it by, the row a binding
// is being changed in (-1, none).
constexpr u32 kListWidget = 0x14;
constexpr u32 kItemFlags = 0x05;
constexpr std::uint8_t kCapturingKeys = 0x04;
constexpr std::uint8_t kChanged = 0x10;
constexpr u32 kItemX = 0x06;
constexpr u32 kItemY = 0x08;
constexpr u32 kItemBoxY = 0x1a;                // int16; the rows from here
constexpr u32 kItemBoxX = 0x1c;
constexpr u32 kItemBoxHeight = 0x1e;           // int16: the second column's distance, with one player
constexpr u32 kItemBoxWidth = 0x20;
constexpr u32 kItemChanging = 0x3c;            // int16

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
nfs::u32 frontend::menua::pollJoystickBinding(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x98);
    const u32 state = guest.frame();
    u32 binding = 0;
    for (u32 device = 0; s32(device) < s32(gameSettings(guest).deviceCount) && lowByte(binding) == 0; ++device)
    {
        if (readJoystick(guest, ts, device, state) == 0 || gameSettings(guest).devices[device].kind == 0)
            continue;
        if (guest.dword(state + joystick::kAnyHeld) != 0)
        {
            for (u32 button = 0; button < 32; ++button)
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
                runSystemTasks(guest, ts);
            while (readJoystick(guest, ts, device, state) != 0 && guest.dword(state + joystick::kAnyHeld) != 0);
        }
        for (u32 hat = 0; hat < 4 && lowByte(binding) == 0; ++hat)
        {
            const u32 direction = guest.dword(state + joystick::kHats + 4 * hat);
            if (direction != 0)
                binding = (hat << 20) + (device << 28) + (direction << 8) + kBindHat;
        }
        for (u32 axis = 0; axis < 8 && lowByte(binding) == 0; ++axis)
        {
            if ((gameSettings(guest).devices[device].axisMask & (u32(1) << axis)) == 0)
                continue;
            s32 position = s32At(guest, state + joystick::kAxes + 4 * axis) >> 8;
            if (position < 0)
                position = 0;
            // (The table has room for two devices; past them it is the list's other numbers.)
            const s32 rest = s32At(guest, kAxisRest + (device << 5) + 4 * axis);
            const s32 moved = minus(position, rest);
            const u32 base = (axis << 24) + (device << 28);
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
    u32 width, height;
    u32 left, top, right, bottom;
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
std::uint32_t axisIcon(u32 binding, u32 axis)
{
    const bool sideways = axis == 0 || axis == 6;
    const u32 from = binding & 0xff0000;
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
nfs::u32 frontend::menua::drawBinding(App* app, ThreadState& ts, u32, u32 binding, u32 left, u32 y, u32 chosen)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x64);
    const u32 line = guest.frame();   // char[0x64]
    const u32 colour = chosen != 0 ? kColourChosen : kColourPlain;
    u32 x = left + guest.dword(kColumnLeft);
    const auto deviceLine = [&] {
        const u32 device = (binding >> 28) + 1;
        const u32 name = languageText(guest, ts, 0x54);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatDevice.address, guest::Words{name, device});
        drawText(guest, ts, line, x, y, colour);
    };
    switch (lowByte(binding))
    {
    case kBindAxis:
    {
        deviceLine();
        x += guest.dword(kBindingWidth);
        const u32 axis = (binding << 4) >> 28;
        const std::uint32_t icon = axisIcon(binding, axis);
        const u32 letter = axis % 3 + 0x58;
        if (guest.dword(kInRace) != 0)
        {
            const AxisIcon& picture = kAxisIcons[icon];
            const u32 right = x + picture.width, bottom = y + picture.height;
                        port::draw2d::ArtQuad quad;
            const u32 quadX[4] = { x, right, right, x }, quadY[4] = { y, y, bottom, bottom };
            const u32 quadU[4] = { picture.left, picture.right, picture.right, picture.left };
            const u32 quadV[4] = { picture.top, picture.top, picture.bottom, picture.bottom };
            for (int corner = 0; corner < 4; ++corner)
            {
                quad.x[corner] = s32(quadX[corner]);
                quad.y[corner] = s32(quadY[corner]);
                quad.u[corner] = quadU[corner];
                quad.v[corner] = quadV[corner];
            }
            quad.texture = guest.dword(kButtonsTexture);
            quad.colour = 0xffffffff;
            return u(*fe::direct<kDrawTexturedQuad, port::draw2d::texturedQuad>(guest.app(), ts, ts.sp - 4 - 4 * 14,
                                                                                quad));
        }
        const u32 name = languageText(guest, ts, 0x57);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatCharacter.address, guest::Words{name, letter});
        drawTextLast(guest, line, x, y, colour);  // by address too: the last call leaves its ecx
        return drawShapeLast(guest, s16At(guest, kBindingIcons + 2 * icon), x + guest.dword(kBindingWidth), y);
    }
    case kBindButton:
    {
        deviceLine();
        const u32 button = ((binding << 4) >> 12) + 0x41;
        const u32 width = guest.dword(kBindingWidth);
        const u32 name = languageText(guest, ts, 0x55);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatCharacter.address, guest::Words{name, button});
        return drawTextLast(guest, line, x + width, y, colour);
    }
    case kBindHat:
    {
        deviceLine();
        const u32 eighth = ((binding << 12) >> 20) - 1;
        x += guest.dword(kBindingWidth);
        const u32 degrees = eighth * 45 != 0 ? eighth * 45 : 360;
        const u32 hat = ((binding << 4) >> 24) + 0x31;
        const u32 name = languageText(guest, ts, 0x56);
        if (guest.dword(kInRace) != 0)
        {
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatHat.address, guest::Words{name, hat, degrees, kDegrees.address});
            return drawTextLast(guest, line, x, y, colour);
        }
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatCharacter.address, guest::Words{name, hat});
        drawText(guest, ts, line, x, y, colour);
        x += guest.dword(kBindingWidth);
                return drawTurnedShape(guest, ts, s16At(guest, kHatArrow), x + 0xc, y + 0xc, degrees);
    }
    case kBindKey:
    {
        drawText(guest, ts, languageText(guest, ts, 0x53), x, y, colour);
        const u32 width = guest.dword(kBindingWidth);
        guest::direct<0x43d7f0, bindingKeyName>(app, ts, binding, line);
        return drawTextLast(guest, line, x + width, y, colour);
    }
    default:
        return drawTextLast(guest, languageText(guest, ts, 0x52), x, y, colour);
    }
}

namespace
{

// The rows' labels, top to bottom.
constexpr std::uint32_t kRowLabels[kRowsEachPlayer] = { 0x47, 0x46, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
                                                        0x4d, 0x4f, 0x4e, 0x51, 0x1bc, 0x50 };

/* Where the pointer goes across for the list's second column with one player,
 * or, with two, where the item has it: for the keys that move it there. */
u32 columnX(Guest& guest, u32 item, bool toSecond)
{
    if (gameSettings(guest).playMode == 1)
    {
        u32 x = u(s16At(guest, item + kItemX)) + guest.dword(kColumnLeft) + guest.dword(kBindingWidth);
        if (toSecond)
            x += guest.dword(kPaneGap);
        return x - 0xa;
    }
    return u(s16At(guest, item + kItemX) + s16At(guest, item + kItemBoxHeight)) - 0xa;
}

/* The pointer put there, `first` down. */
void pointerAtColumn(Guest& guest, nfs::ThreadState& ts, u32 item, u32 first, bool toSecond)
{
    setPointer(guest, ts, columnX(guest, item, toSecond), first);
}

/* sub_43c900: the list on every pass.  Its first pass after the arrow keys it
 * puts the pointer on the row they should reach; while a binding is being
 * changed, it asks the joystick and says so at the top; each row with its
 * bindings -- two columns with one player, for players 1 and 2 -- and its label. */
void drawControlsList(Guest& guest, nfs::ThreadState& ts, u32 item, u32 pointerWanted)
{
    const u32 line = guest.frame();   // char[0x4c]
    s32 y = s16At(guest, item + kItemY);
    if (pointerWanted != 0)
    {
        if (guest.dword(kPointerPlaced) == 0)
        {
            const u32 key = guest.dword(0x66464c);
            if (key == kKeyUp || key == kKeyLeft)
            {
                const s32 height = s32At(guest, kRowHeight);
                pointerAtColumn(guest, ts, item, u(times(height, 12) + s16At(guest, item + kItemY) + halved(height)), true);
            }
            const u32 again = guest.dword(0x66464c);
            if (again == kKeyDown || again == kKeyRight)
                pointerAtColumn(guest, ts, item, u(s16At(guest, item + kItemY) + halved(s32At(guest, kRowHeight))), false);
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
            const u32 heard = guest::direct<0x43be40, frontend::menua::pollJoystickBinding>(guest.app(), ts);
            guest.setDword(kHeardBinding, heard);
            if (lowByte(heard) != 0)
                fe::direct<0x4cc8e0, frontend::wina::pushKey>(guest.app(), ts, 0xd, 0x1c, 0);
        }
        const u32 promptY = guest.dword(kInRace) != 0 ? 0x41 : 0x55;
        const u32 prompt = languageText(guest, ts, 0x45);
        drawMenuText(guest, ts, prompt, 0x140, promptY, 2, kColourChosen);
        chosenRow = s16At(guest, item + kItemChanging);
    }
    else if (pointerWanted != 0)
    {
        chosenRow = s32(guest::direct<0x49c1c0, frontend::menud::pointerListLine>(guest.app(), ts, s16At(guest, item + kItemBoxY),
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

    const u32 controls = controlsFor(guest, ts);
    const u32 title = languageText(guest, ts, 0xc8);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatPlayer1.address, guest::Words{title});
    drawText(guest, ts, line, u(s16At(guest, item + kItemX)) + guest.dword(kColumnLeft),
             u(s16At(guest, item + kItemY)) - 0x1e, kColourPlain);
    if (gameSettings(guest).playMode == 1)
    {
        const u32 second = languageText(guest, ts, 0xc8);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), line, kFormatPlayer2.address, guest::Words{second});
        drawText(guest, ts, line, guest.dword(kColumnLeft) + u(s16At(guest, item + kItemX)) + guest.dword(kPaneGap),
                 u(s16At(guest, item + kItemY)) - 0x1e, kColourPlain);
    }

    for (s32 row = 0; row < kRowsEachPlayer; ++row)
    {
        const std::uint32_t label = kRowLabels[row];
        if (guest.dword(kInRace) == 0)
        {
            const u32 shapeX = guest.dword(kLabelGaps + 4 * u(row)) + u(s16At(guest, item + kItemX));
            drawShape(guest, ts, s16At(guest, kRowShape), shapeX, u(y));
            if (s16At(guest, item + kItemChanging) == row)
            {
                const s32 blink = s32(guest.dword(kBlink) + 1);
                guest.setDword(kBlink, u(blink));
                if (blink > 0x1e)
                    guest.setDword(kBlink, 0);
                else if (blink > 0xf)
                    drawShape(guest, ts, s16At(guest, kRowLitShape), shapeX, u(y));
            }
        }
        u32 labelX = u(s16At(guest, item + kItemX));
        guest::direct<0x43c110, frontend::menua::drawBinding>(guest.app(), ts, label, guest.dword(controls + 4 * u(row)), labelX,
                                                              u(y), row == chosenRow ? 1u : 0u);
        if (gameSettings(guest).playMode == 1)
        {
            guest::direct<0x43c110, frontend::menua::drawBinding>(guest.app(), ts, label, guest.dword(controls + 4 * u(row) + 0x34),
                                                                  labelX + guest.dword(kPaneGap), u(y),
                                                                  row + kRowsEachPlayer == chosenRow ? 1u : 0u);
        }
        if (guest.dword(kInRace) == 0)
            labelX += guest.dword(kLabelGaps + 4 * u(row)) + 0xe;
        const u32 name = languageText(guest, ts, label);
        drawText(guest, ts, name, labelX, u(y), kColourPlain);
        y = s32(u(y) + guest.dword(kRowHeight));
    }
}

}  // namespace

/* sub_43c900: the list, while it is shown, its edx left 0. */
nfs::u32 frontend::menua::controlsList(App* app, ThreadState& ts, u32 item, u32 pointerWanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x4c);
    if (shownNow(guest, ts, item, kListWidget) != 0)
        drawControlsList(guest, ts, item, pointerWanted);
    return 0;
}

/* sub_43cda0: the key just pressed as a binding, or what the joystick gave; 0
 * for none, and the keys that are the menus' own or the game's are none.  One
 * that is taken has a sound and others bound to it cleared. */
nfs::u32 frontend::menua::keyAsBinding(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr u32 kScanCode = 0x66465c;
    constexpr u32 kCharacterFlags = 0x564ef0;
    const s32 scan = s32At(guest, kScanCode);
    u32 binding = 0;
    const bool reserved = scan == 0x4a || scan == 0x4e || scan == 0x37 || scan == 0x35 || (scan >= 2 && scan <= 0xb)
        || (scan >= 0x3b && scan <= 0x44) || scan == -1;
    if (!reserved && s32(character) >= 0x20
        && (guest.byte(kCharacterFlags + ((character + 1) & 0xff)) & 0x20) == 0)
    {
        const u32 upper = fe::direct<0x4eede0, watcom::clib::toupper>(guest.app(), guest.ts(), character);
        binding = (upper << 24) + (u(scan) << 8) + kBindKey;
    }
    if (lowByte(guest.dword(kHeardBinding)) != 0)
    {
        binding = guest.dword(kHeardBinding);
        guest.setDword(kHeardBinding, 0);
    }
    if (lowByte(binding) != 0)
    {
        playEffect(guest, ts, 1);
        guest::direct<0x43d6a0, clearConflictingBindings>(app, ts, binding);
    }
    return binding;
}

/* sub_43ce80: in a race, one kind of binding (digital or not) for the steering
 * and the two pedals; a binding of the other kind has a message, as the race
 * has read the kind already. */
nfs::u32 frontend::menua::warnDigitalBinding(App* app, ThreadState& ts, u32 slot, u32 binding)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    constexpr u32 kMessageLines = 0x5f3dac;
    if (guest.dword(kInRace) == 0)
        return binding;
    const s32 row = s32(slot) < kRowsEachPlayer ? s32(slot) : minus(s32(slot), kRowsEachPlayer);
    if (u32(row) > 3)
        return binding;
    static const std::uint8_t kDigitalFlags[4] = { 0, 0, 1, 2 };
    const u32& flag = gameSettings(guest).players[s32(slot) < kRowsEachPlayer ? 0 : 1].digital[kDigitalFlags[row]];
    const u32 digital = fe::direct<0x43c000, glue::bindingIsDigital>(app, ts, binding);
    if (digital == flag)
        return digital;
    guest.setDword(kMessageLines, fe::text(guest, 0x1f0));
    guest.setDword(kMessageLines + 4, fe::text(guest, 0x1f1));
    guest.setDword(kMessageLines + 8, fe::text(guest, 0x1f2));
    return fe::direct<0x444a80, dialogs::openMessage>(app, ts, 3u, kMessageLines, 0x3e8u, 1u, 0u);
}

namespace
{

/* A binding being changed: Escape puts it back, any other key is the new one.
 * What the game's code leaves in edx's second byte and in ebx goes with it. */
void listKeyChanging(Guest& guest, nfs::ThreadState& ts, u32 item, u32 controls, std::uint16_t key)
{
    const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kCapturingKeys);
    const u32 slot = controls + 4 * u(s16At(guest, item + kItemChanging));
    guest.setByte(item + kItemFlags, flags);
    if (key == kKeyEscape)
    {
        guest.setByte(item + kItemFlags, std::uint8_t(flags & ~kCapturingKeys));
        guest.setDword(slot, guest.dword(kBindingBefore));
    }
    else
    {
        const u32 binding = guest::direct<0x43cda0, frontend::menua::keyAsBinding>(guest.app(), ts, u(std::int16_t(key)));
        if (binding == 0)
            return;
        guest::direct<0x43ce80, frontend::menua::warnDigitalBinding>(guest.app(), ts, u(s16At(guest, item + kItemChanging)),
                                                                     binding);
        guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
        guest.setDword(slot, binding);
    }
    guest.setWord(item + kItemChanging, 0xffff);
}

/* The arrows over the rows: the pointer put on the row they reach; whether they
 * moved it.  With one player, two columns of rows.  Its calls by address: what
 * the last leaves in edx, sub_43cf80 leaves. */
bool listKeyArrows(Guest& guest, u32 item, std::uint16_t key, s32& row)
{
    bool moved = false;
    if (gameSettings(guest).playMode == 1)
    {
        const s32 secondColumn = s32(u(s16At(guest, item + kItemX)) + guest.dword(kPaneGap) + guest.dword(kColumnLeft));
        const u32 pointerX = guest.dword(kPointerX);
        if (secondColumn < s32(pointerX))
            row += kRowsEachPlayer;
        if (key == kKeyLeft && row >= kRowsEachPlayer)
        {
            playEffectLast(guest, 0);
            row -= kRowsEachPlayer;
            moved = true;
        }
        if (key == kKeyRight && row < kRowsEachPlayer)
        {
            playEffectLast(guest, 0);
            row += kRowsEachPlayer;
            moved = true;
        }
        if (key == kKeyDown)
        {
            ++row;
            playEffectLast(guest, 0);
            if (row < kRowsEachPlayer * 2)
                moved = true;
        }
        if (key == kKeyUp)
        {
            playEffectLast(guest, 0);
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
            const u32 columnRow = u(second ? row - kRowsEachPlayer : row);
            setPointerLast(guest, columnX(guest, item, second),
                                columnRow * u(height) + u(s16At(guest, item + kItemY)) + u(halved(height)));
        }
        return moved;
    }
    const auto pointAtRow = [&] {
        const s32 height = s32At(guest, kRowHeight);
        setPointerLast(guest, columnX(guest, item, false),
                            u(height) * u(row) + u(s16At(guest, item + kItemY)) + u(halved(height)));
    };
    if (key == kKeyDown)
    {
        ++row;
        playEffectLast(guest, 0);
        if (row < kRowsEachPlayer)
        {
            moved = true;
            pointAtRow();
        }
    }
    if (key == kKeyUp)
    {
        playEffectLast(guest, 0);
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
nfs::u32 frontend::menua::listKey(App* app, ThreadState& ts, u32 item, u32, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0xc);
    const u32 controls = controlsFor(guest, ts);
    if (shownNow(guest, ts, item, kListWidget) == 0)
        return 0;
    if (std::int16_t(guest.word(item + kItemChanging)) != -1)
    {
        listKeyChanging(guest, ts, item, controls, key);
        return 2;
    }

    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
    s32 row = s32(guest::direct<0x49c1c0, menud::pointerListLine>(app, ts, s16At(guest, item + kItemBoxY),
                                                              s32At(guest, kRowHeight)));
    u32 result = listKeyArrows(guest, item, key, row) ? 2 : 0;
    if (key == kKeyEnter)
    {
        result = 2;
        const u32 slot = controls + 4 * u(row);
        if (row < kRowsEachPlayer || (gameSettings(guest).playMode == 1 && row < kRowsEachPlayer * 2))
        {
            guest::direct<0x43d920, sampleAxesAtRest>(app, ts, 0);  // its result is not read
            const u32 before = guest.dword(slot);
            guest.setDword(slot, 0);
            guest.setDword(kBindingBefore, before);
            const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kCapturingKeys);
            guest.setWord(item + kItemChanging, std::uint16_t(row));
            guest.setDword(kHeardBinding, 0);
            guest.setByte(item + kItemFlags, flags);
        }
    }
    return result;
}

/* sub_43d2b0: the box the list takes the pointer in, for the screen's layout. */
nfs::u32 frontend::menua::listFitBox(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 shown = shownNow(guest, ts, item, kListWidget);
    if (shown == 0)
        return shown;
    guest.setWord(item + kItemBoxX, guest.word(item + kItemX));
    guest.setWord(item + kItemBoxY, guest.word(item + kItemY));
    // As wide as the game's code's registers have it: only the low words are stored.
    const u32 tall = withLowWord(shown, guest.word(kRowHeight)) * 0xd;
    guest.setWord(item + kItemBoxWidth, std::uint16_t(tall));
    const u32 across = withLowWord(tall, guest.word(kColumnLeft)) + withLowWord(0, guest.word(kBindingWidth))
        + (gameSettings(guest).playMode == 1 ? 0xfa : 0x28);
    guest.setWord(item + kItemBoxHeight, std::uint16_t(across));
    return across;
}

/* sub_43d310: the list as its screen opens: its layout, in a race or in the
 * menus, and the joysticks waited for to let go. */
nfs::u32 frontend::menua::listPrepare(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const u32 shown = shownNow(guest, ts, item, kListWidget);
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
        guest.setWord(kRowShape, std::uint16_t(shapeNamed(guest, ts, kRowShapeName.address)));
        guest.setWord(kRowLitShape, std::uint16_t(shapeNamed(guest, ts, kRowLitShapeName.address)));
    }
    guest.setWord(item + kItemChanging, 0xffff);
    return guest::direct<0x43d650, waitJoystickRelease>(app, ts);
}

/* sub_43d650: waiting until the joysticks are let go.  (The game's code asks of
 * them with its "any held" flag for the device number: of the first two only.) */
nfs::u32 frontend::menua::waitJoystickRelease(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x84);
    const u32 state = guest.frame();
    bool held;
    u32 left;
    do
    {
        held = false;
        for (s32 i = 0; i < s32(gameSettings(guest).deviceCount); ++i)
        {
            if (readJoystick(guest, ts, held ? 1u : 0u, state) != 0 && guest.dword(state + joystick::kAnyHeld) != 0)
                held = true;
        }
        left = runSystemTasks(guest, ts);
    } while (held);
    return left;
}

/* sub_43d6a0: a binding taken away from the rows that had it: a key by its code,
 * an axis also where the two would share an end of its travel. */
nfs::u32 frontend::menua::clearConflictingBindings(App* app, ThreadState& ts, u32 binding)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    if (binding == 0)
        return 0;
    const s32 count = gameSettings(guest).playMode == 1 ? kRowsEachPlayer * 2 : kRowsEachPlayer;
    const u32 controls = controlsFor(guest, ts);
    if (lowByte(binding) == kBindKey)
    {
        for (s32 row = 0; row < count; ++row)
        {
            const u32 at = controls + 4 * u(row);
            if (((guest.dword(at) << 8) >> 16) == ((binding << 8) >> 16))
                guest.setDword(at, 0);
        }
        return u(count);
    }
    u32 at = 0;
    for (s32 row = 0; row < count; ++row)
    {
        at = controls + 4 * u(row);
        const u32 other = guest.dword(at);
        bool clear = binding == other;
        if (!clear && lowByte(binding) == kBindAxis && lowByte(other) == kBindAxis && (other >> 28) == (binding >> 28)
            && ((other << 4) >> 28) == ((binding << 4) >> 28))
        {
            const u32 otherLow = (other << 16) >> 24, bindingHigh = (binding << 8) >> 24;
            const u32 otherHigh = (other << 8) >> 24, bindingLow = (binding << 16) >> 24;
            clear = otherLow == bindingHigh || bindingLow == otherHigh || otherHigh == bindingHigh || bindingLow == otherLow;
        }
        if (clear)
            guest.setDword(at, 0);
    }
    return at;
}

/* sub_43d7f0: a key binding's name: a line of the language's for the keys with
 * one, the character for the rest. */
nfs::u32 frontend::menua::bindingKeyName(App* app, ThreadState& ts, u32 binding, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    struct NamedKey
    {
        u32 scan;
        std::uint32_t text;
    };
    static const NamedKey kNamedKeys[] = {
        { 0x39, 0x58 }, { 0x1c, 0x59 }, { 0x52, 0x5a }, { 0x53, 0x5b }, { 0x47, 0x5c }, { 0x4f, 0x5d }, { 0x49, 0x5e }, { 0x51, 0x5f },
        { 0x48, 0x60 }, { 0x50, 0x61 }, { 0x4b, 0x62 }, { 0x4d, 0x63 }, { 0x0e, 0x64 }, { 0x0f, 0x65 }, { 0x4c, 0x44 },
    };
    const u32 scan = (binding << 8) >> 16;
    for (const NamedKey& key : kNamedKeys)
    {
        if (key.scan == scan)
        {
            const u32 name = languageText(guest, ts, key.text);
            guest.copyString(buffer, name);
            return name & 0xffffff00;
        }
    }
    const u32 upper = fe::direct<0x4eede0, watcom::clib::toupper>(guest.app(), guest.ts(), binding >> 24);
    return fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatKey.address, guest::Words{upper});
}

/* sub_43d920: where each axis of the devices that are on rests now, for
 * sub_43be40 to tell them moved. */
nfs::u32 frontend::menua::sampleAxesAtRest(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x84);
    const u32 state = guest.frame();
    u32 left = eax;
    for (u32 device = 0; s32(device) < s32(gameSettings(guest).deviceCount); ++device)
    {
        left = device * 17;
        if (gameSettings(guest).devices[device].kind == 0)
            continue;
        left = readJoystick(guest, ts, device, state);
        if (left == 0)
            continue;
        for (u32 axis = 0; axis < 8; ++axis)
        {
            left = u32(1) << axis;
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
constexpr u32 kWidget = 0x13;
constexpr u32 kBoxTop = 0x1a;
constexpr u32 kBoxX = 0x1c;
constexpr u32 kBoxHeight = 0x1e;
constexpr u32 kBoxWidth = 0x20;
constexpr u32 kHeight = 0x3c;
constexpr u32 kWidth = 0x3e;
constexpr u32 kHue = 0x40;
constexpr u32 kSaturation = 0x44;
constexpr u32 kValue = 0x48;
constexpr u32 kFourth = 0x4c;
constexpr u32 kSavedColour = 0x50;
constexpr u32 kPlayer = 0x60;
constexpr u32 kTriangleTop = 0x64;
constexpr u32 kTriangleLeft = 0x66;
constexpr u32 kTriangleWidth = 0x68;
constexpr u32 kTriangleHeight = 0x6a;
constexpr u32 kBarTop = 0x6c;
constexpr u32 kBarLeft = 0x6e;
constexpr u32 kBarWidth = 0x70;
constexpr u32 kBarHeight = 0x72;
constexpr u32 kSwatchTop = 0x74;
constexpr u32 kSwatchLeft = 0x76;
constexpr u32 kSwatchWidth = 0x78;
constexpr u32 kSwatchHeight = 0x7a;
constexpr u32 kOkTop = 0x7c;
constexpr u32 kOkLeft = 0x7e;
constexpr u32 kOkWidth = 0x80;
constexpr u32 kOkHeight = 0x82;
constexpr u32 kCancelTop = 0x84;
constexpr u32 kCancelLeft = 0x86;
constexpr u32 kCancelWidth = 0x88;
constexpr u32 kCancelHeight = 0x8a;
constexpr u32 kUnderPointer = 0x8c;     // int16, an area of Hit
constexpr u32 kPressed = 0x8e;          // int16
constexpr u32 kBackShape = 0x90;        // int16 each
constexpr u32 kFrameShape = 0x92;
constexpr u32 kPanelShape = 0x94;
constexpr u32 kCancelLitShape = 0x96;
constexpr u32 kOkLitShape = 0x98;
constexpr u32 kTitle = 0x9c;            // char*
constexpr u32 kOkText = 0xa0;
constexpr u32 kCancelText = 0xa4;

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

constexpr u32 kHueRampTexture = 0x5f3dd0;
constexpr GuestString kNoName{ 0x5377dc, "" };
constexpr GuestString kCarSelect1{ 0x5377e0, "CAR_SELECT_OBJ3DP1" };
constexpr GuestString kCarSelect2{ 0x5377f4, "CAR_SELECT_OBJ3DP2" };
constexpr GuestString kPickerShapes[] = { { 0x537808, "yar1" }, { 0x537810, "bdd2" }, { 0x537818, "ccdd" },
                                          { 0x537820, "ccht" }, { 0x537828, "ccbt" } };
constexpr std::uint32_t kColourMarked = 0xb3;        // byte of the car's item: to be drawn again

}  // namespace

/* sub_43db20: the bar's texture, the hues from top to bottom. */
nfs::u32 frontend::menua::makeHueRamp(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    constexpr u32 kSide = 0x100;
    const u32 hsv = guest.frame();
    const u32 argb = guest.frame() + 0x10;
    const u32 shape = *fe::direct<0x4e1620, ea::reserved>(app, ts, kNoName.address, 0x40014u, 0x10u);
    guest.setByte(shape, 0);
    guest.setWord(shape + 4, kSide);
    guest.setWord(shape + 6, kSide);
    guest.setByte(shape, 0x7d);
    u32 pixel = shape + 0x10;
    for (u32 hue = 0; hue < kSide; ++hue)
    {
        guest.setDword(hsv + 4, 0xff);
        guest.setDword(hsv + 8, 0xff);
        guest.setDword(hsv + 0xc, 0xff);
        guest.setDword(hsv, hue);
        guest::direct<0x43de20, hsvToArgb>(app, ts, hsv, argb);
        const u32 colour = guest.dword(argb) | 0xff000000;
        for (u32 x = 0; x < kSide; ++x, pixel += 4)
            guest.setDword(pixel, colour);
    }
    fe::direct<0x4d6070, port::shapes::textureOf>(app, ts, ts.sp - 4, kHueRampTexture, shape, 0x40, 0u);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, shape);
}

/* sub_43dc90: the colour on the player's car, and the car's picture marked to be
 * drawn again. */
nfs::u32 frontend::menua::applyColourToCar(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x20);
    const u32 shown = shownNow(guest, ts, item, picker::kWidget);
    if (shown == 0)
        return shown;
    const u32 car = gameSettings(guest).players[guest.dword(item + picker::kPlayer) == 0 ? 0 : 1].car;
    const u32 hsv = guest.frame();
    const u32 rgb = guest.frame() + 0x10;
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(hsv + 4 * i, guest.dword(item + picker::kHue + 4 * i));
    fe::direct<0x49ca50, load::hsvToColour>(guest.app(), guest.ts(), hsv, rgb);
    u32 left = u(*fe::direct<0x4398c0, port::fields::customColourSet>(
        app, ts, ts.sp - 4 - 4 * 2, car, s32(guest.dword(item + picker::kPlayer)), guest.byte(rgb),
        guest.byte(rgb + 4), guest.byte(rgb + 8), std::uint8_t(0x80)));
    if (guest.dword(item + picker::kPlayer) == 0)
    {
        const u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
        fe::Item found = fe::findItem(guest, menu, kCarSelect1);
        left = found.address();
        if (found)
            guest.setByte(found.address() + kColourMarked, 1);
    }
    if (guest.dword(item + picker::kPlayer) == 1)
    {
        const u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
        fe::Item found = fe::findItem(guest, menu, kCarSelect2);
        left = found.address();
        if (found)
            guest.setByte(found.address() + kColourMarked, 1);
    }
    return left;
}

/* sub_43dd90: the colour taken, the picker gone. */
nfs::u32 frontend::menua::acceptColour(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 shown = shownNow(guest, ts, item, picker::kWidget);
    if (shown == 0)
        return shown;
    const std::uint8_t flags = std::uint8_t(guest.byte(item + kItemFlags) | kChanged);
    guest.setByte(item + kItemFlags, flags);
    guest.setByte(item + kItemFlags, std::uint8_t(flags & ~kCapturingKeys));
    return guest::direct<0x43dc90, applyColourToCar>(app, ts, item);
}

/* sub_43ddd0: the colour as it was, the picker gone. */
nfs::u32 frontend::menua::revertColour(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 shown = shownNow(guest, ts, item, picker::kWidget);
    if (shown == 0)
        return shown;
    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) | kChanged));
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(item + picker::kHue + 4 * i, guest.dword(item + picker::kSavedColour + 4 * i));
    guest.setByte(item + kItemFlags, std::uint8_t(guest.byte(item + kItemFlags) & ~kCapturingKeys));
    return guest::direct<0x43dc90, applyColourToCar>(app, ts, item);
}

/* sub_43de20: a colour of hue, saturation and value as opaque ARGB. */
nfs::u32 frontend::menua::hsvToArgb(App* app, ThreadState& ts, u32 hsv, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0x10);
    const u32 rgb = guest.frame();
    fe::direct<0x49ca50, load::hsvToColour>(guest.app(), guest.ts(), hsv, rgb);
    const u32 blue = guest.dword(rgb + 8) & 0xff;
    const u32 argb = 0xff000000 | ((guest.dword(rgb) & 0xff) << 16) | ((guest.dword(rgb + 4) & 0xff) << 8) | blue;
    guest.setDword(out, argb);
    return blue;
}

/* sub_43de60: where a colour puts its marker in the triangle of that width and
 * height: the row by one of its numbers, from the tip, the place in the row by
 * the other, inside the row's ends. */
nfs::u32 frontend::menua::triangleFromColour(App* app, ThreadState& ts, s32 width, s32 height, s32 across, s32 down, u32 outX, u32 outY)
{
    Guest guest(app, ts);
    const s32 half = halved(width);
    guest.setDword(outY, u(times(minus(0xff, down), height) / 0xff));
    const s32 reach = times(down, half) / 0xff;
    const s32 low = half - reach, high = half + reach;
    const s32 product = times(high - low, minus(0xff, across));
    const s32 x = product / 0xff + low;
    const s32 clamped = low > x ? low : high < x ? high : x;
    guest.setDword(outX, u(clamped));
    return u(x);
}

/* sub_43def0: back from a place in the triangle to the two numbers (the middle's,
 * at the tip where the row has no width). */
nfs::u32 frontend::menua::colourAtPlace(App* app, ThreadState& ts, s32 width, s32 height, u32 outAcross, u32 outDown, s32 x, s32 y)
{
    Guest guest(app, ts);
    const s32 half = halved(width);
    const s32 row = y < 0 ? 0 : height < y ? height : y;
    const s32 down = 0xff - times(row, 0xff) / height;
    guest.setDword(outDown, u(down));
    const s32 reach = times(down, half) / 0xff;
    const s32 low = half - reach, high = half + reach;
    const s32 span = high - low;
    const s32 clamped = low > x ? low : high < x ? high : x;
    if (span == 0)
    {
        guest.setDword(outAcross, 0x80);
        return u(low);
    }
    const s32 fraction = times(clamped - low, 0xff) / span;
    guest.setDword(outAcross, u(0xff - fraction));
    return u(fraction);
}

/* sub_43dfb0: the picker's back, frame and title. */
nfs::u32 frontend::menua::drawPickerFrame(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    drawShape(guest, ts, s16At(guest, item + picker::kPanelShape), u(s16At(guest, item + kItemX)), u(s16At(guest, item + kItemY)));
    const s32 y = s16At(guest, item + kItemY);
    const s32 x = s16At(guest, item + kItemX);
    drawShape(guest, ts, s16At(guest, item + picker::kBackShape), u(x - 0x23), u(y + 0xc));
    drawShape(guest, ts, s16At(guest, item + picker::kFrameShape), u(x - 0x17), u(y - 1));
    return drawText(guest, ts, guest.dword(item + picker::kTitle), u(s16At(guest, item + kItemX) + 0xc), u(y - 1), kColourChosen);
}

/* sub_43e040: the swatch of the colour, with its shadow. */
nfs::u32 frontend::menua::drawColourSwatch(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const u32 argb = guest.frame();
    guest::direct<0x43de20, hsvToArgb>(app, ts, item + picker::kHue, argb);
    const s32 left = s16At(guest, item + kItemX) + s16At(guest, item + picker::kSwatchLeft);
    const s32 top = s16At(guest, item + kItemY) + s16At(guest, item + picker::kSwatchTop);
    const u32 width = u(s16At(guest, item + picker::kSwatchWidth));
    const u32 height = u(s16At(guest, item + picker::kSwatchHeight));
    filledRectangle(guest, ts, u(left + 3), u(top + 3), width, height, 0xff000000);
    return filledRectangle(guest, ts, u(left), u(top), width, height, guest.dword(argb));
}

/* sub_43e0d0: the two buttons, the one under the pointer lit. */
nfs::u32 frontend::menua::drawPickerButtons(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    const auto button = [&](std::uint16_t which, u32 litShape, u32 left, u32 top, u32 text) {
        const s32 x = s16At(guest, item + kItemX) + s16At(guest, item + left);
        const s32 y = s16At(guest, item + kItemY) + s16At(guest, item + top);
        if (guest.word(item + picker::kUnderPointer) == which)
            drawShape(guest, ts, s16At(guest, item + litShape), u(x), u(y));
        const u32 colour = guest.word(item + picker::kUnderPointer) == which ? kColourChosen : kColourPlain;
        return drawText(guest, ts, guest.dword(item + text), u(x + 0xf), u(y), colour);
    };
    button(picker::kHitCancel, picker::kCancelLitShape, picker::kCancelLeft, picker::kCancelTop, picker::kCancelText);
    return button(picker::kHitOk, picker::kOkLitShape, picker::kOkLeft, picker::kOkTop, picker::kOkText);
}

/* sub_43e1f0: the markers of the hue on the bar and of the colour in the
 * triangle, each with its shadow. */
nfs::u32 frontend::menua::drawColourMarkers(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc);
    constexpr u32 kShadow = 0x7f000000;
    constexpr u32 kMarker = 0xff7f7fff;
    const u32 markerX = guest.frame(), markerY = guest.frame() + 4;
    const u32 across = guest.dword(item + picker::kSaturation);
    const u32 down = guest.dword(item + picker::kValue);
    const s32 hueRow = times(s32At(guest, item + picker::kHue), s16At(guest, item + picker::kBarHeight)) / 0xff;
    const u32 y = u(hueRow + s16At(guest, item + picker::kBarTop) + s16At(guest, item + kItemY) - 3);
    const u32 x = u(s16At(guest, item + picker::kBarLeft) + s16At(guest, item + kItemX) - 3);
    const u32 barWidth = u(s16At(guest, item + picker::kBarWidth) + 6);
    frameRectangle(guest, ts, x + 1, y + 1, barWidth, 6, kShadow);
    frameRectangle(guest, ts, x, y, u(s16At(guest, item + picker::kBarWidth) + 6), 6, kMarker);
    guest::direct<0x43de60, triangleFromColour>(app, ts, s16At(guest, item + picker::kTriangleWidth),
                                                s16At(guest, item + picker::kTriangleHeight), s32(across), s32(down), markerX,
                                                markerY);
    const u32 colourX = guest.dword(markerX) + u(s16At(guest, item + kItemX) + s16At(guest, item + picker::kTriangleLeft) - 3);
    const u32 colourY = guest.dword(markerY) + u(s16At(guest, item + kItemY) + s16At(guest, item + picker::kTriangleTop) - 3);
    frameRectangle(guest, ts, colourX + 1, colourY + 1, 6, 6, kShadow);
    return frameRectangle(guest, ts, colourX, colourY, 6, 6, kMarker);
}

/* sub_43e310: the triangle of the colour's hue, over its shadow and a translucent
 * edge. */
nfs::u32 frontend::menua::drawColourTriangle(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x28);
    const u32 hsv = guest.frame();
    const u32 rgb = guest.frame() + 0x10;
    guest.setDword(hsv, guest.dword(item + picker::kHue));
    guest.setDword(hsv + 4, 0xff);
    guest.setDword(hsv + 8, 0xff);
    guest.setDword(hsv + 0xc, 0xff);
    fe::direct<0x49ca50, load::hsvToColour>(guest.app(), guest.ts(), hsv, rgb);
    const u32 colour = ((guest.dword(rgb) & 0xff) << 16) | ((guest.dword(rgb + 4) & 0xff) << 8) | (guest.dword(rgb + 8) & 0xff);
    const s32 top = s16At(guest, item + kItemY) + s16At(guest, item + picker::kTriangleTop);
    const s32 left = s16At(guest, item + kItemX) + s16At(guest, item + picker::kTriangleLeft);
    const s32 width = s16At(guest, item + picker::kTriangleWidth);
    const s32 bottom = top + s16At(guest, item + picker::kTriangleHeight);
    const s32 tip = left + halved(width);
    guest::direct<0x4d88c0, menue::shadedTriangle>(app, ts, u(left + 3), u(top + 3), u(left + width + 3), u(top + 3),
                                                   u(tip + 3), u(bottom + 3), 0xff000000u, 0xff000000u, 0xff000000u);
    guest::direct<0x4d88c0, menue::shadedTriangle>(app, ts, u(left - 1), u(top - 1), u(left + width + 1), u(top - 1),
                                                   u(tip), u(bottom + 1), colour | 0xaf000000, 0xafffffffu, 0xaf000000u);
    return guest::direct<0x4d88c0, menue::shadedTriangle>(app, ts, u(left), u(top), u(left + width), u(top), u(tip),
                                                          u(bottom), colour | 0xff000000, 0xffffffffu, 0xff000000u);
}

namespace
{

/* A vertex of the bar's quad, as sub_4c12e0 clips it: x and y as floats, depth
 * and w, its colour, where it falls as flags, its place in the texture. */
namespace vertex
{
constexpr u32 kX = 0x00;
constexpr u32 kY = 0x04;
constexpr u32 kZ = 0x08;
constexpr u32 kW = 0x0c;
constexpr u32 kColour = 0x10;
constexpr u32 kOutcode = 0x14;
constexpr u32 kU = 0x18;
constexpr u32 kV = 0x1c;
constexpr u32 kSize = 0x20;
}

// What the renderer clips to, compared as the game compares them: the floats'
// bits as integers.
constexpr u32 kClipW = 0x7d34f8;
constexpr u32 kClipLeft = 0x7d34fc;
constexpr u32 kClipRight = 0x7d350c;
constexpr u32 kClipTop = 0x7d3504;
constexpr u32 kClipBottom = 0x7d3500;

std::uint8_t vertexOutcode(const Guest& guest, u32 corner)
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
nfs::u32 frontend::menua::drawValueBar(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x90);
    const s32 x = s16At(guest, item + kItemX);
    const s32 y = s16At(guest, item + kItemY);
    const s32 left = s16At(guest, item + picker::kBarLeft) + x;
    const s32 width = s16At(guest, item + picker::kBarWidth);
    const s32 right = s16At(guest, item + picker::kBarLeft) + width + x;
    const s32 height = s16At(guest, item + picker::kBarHeight);
    const s32 top = s16At(guest, item + picker::kBarTop) + y;
    const s32 bottom = s16At(guest, item + picker::kBarTop) + height + y;
    filledRectangle(guest, ts, u(left + 3), u(top + 3), u(width), u(height), 0xff000000);

    // The four corners, clockwise from the top left, each at a depth of the
    // game's and in white.
    const u32 corners = guest.frame();
    const u32 depth = guest.dword(0x563aa8), w = guest.dword(0x563aac);
    const float leftAt = float(FloatMath::integer(left)), rightAt = float(FloatMath::integer(right));
    const float topAt = float(FloatMath::integer(top)), bottomAt = float(FloatMath::integer(bottom));
    const struct
    {
        float x, y;
        float across, down;
    } kCorners[4] = { { leftAt, topAt, 0.0f, 0.0f },
                      { rightAt, topAt, 1.0f, 0.0f },
                      { rightAt, bottomAt, 1.0f, 1.0f },
                      { leftAt, bottomAt, 0.0f, 1.0f } };
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 corner = corners + vertex::kSize * i;
        guest.setFloat(corner + vertex::kX, kCorners[i].x);
        guest.setFloat(corner + vertex::kY, kCorners[i].y);
        guest.setDword(corner + vertex::kZ, depth);
        guest.setDword(corner + vertex::kW, w);
        guest.setDword(corner + vertex::kColour, 0xffffffff);
        guest.setFloat(corner + vertex::kU, kCorners[i].across);
        guest.setFloat(corner + vertex::kV, kCorners[i].down);
    }
    renderState(guest, ts, 1, guest.dword(kHueRampTexture));
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 corner = corners + vertex::kSize * i;
        guest.setByte(corner + vertex::kOutcode, vertexOutcode(guest, corner));
    }
    return fe::direct<0x4c12e0, vertices::quadClipped>(guest.app(), ts, corners, corners + vertex::kSize, corners + vertex::kSize * 2, corners + vertex::kSize * 3);
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
nfs::u32 frontend::menua::pickerDragMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x10);
    const u32 acrossAt = guest.frame() + 4, downAt = guest.frame() + 8;
    s32 hue = s32At(guest, item + picker::kHue);
    guest.setDword(acrossAt, guest.dword(item + picker::kSaturation));
    guest.setDword(downAt, guest.dword(item + picker::kValue));
    const std::uint16_t pressed = guest.word(item + picker::kPressed);
    if (pressed == picker::kHitTriangle)
    {
        const u32 x = guest.dword(kPointerX) - u(s16At(guest, item + kItemX)) - u(s16At(guest, item + picker::kTriangleLeft));
        const u32 y = guest.dword(kPointerY) - u(s16At(guest, item + kItemY)) - u(s16At(guest, item + picker::kTriangleTop));
        guest::direct<0x43def0, colourAtPlace>(app, ts, s16At(guest, item + picker::kTriangleWidth),
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
nfs::u32 frontend::menua::pickerHitRegion(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    const s32 x = s32(guest.dword(kPointerX) - u(s16At(guest, item + kItemX)));
    const s32 y = s32(guest.dword(kPointerY) - u(s16At(guest, item + kItemY)));
    if (fe::direct<0x49c1f0, glue::pointerInBox>(app, ts, item + picker::kBoxTop) == 0)
        return picker::kHitOutside;
    const auto inArea = [&](u32 left, u32 width, u32 top, u32 height) {
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
nfs::u32 frontend::menua::pickerMouse(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    u32 left = item;  // eax as it came
    if (guest.word(item + picker::kPressed) == 0)
    {
        left = guest::direct<0x43e8e0, pickerHitRegion>(app, ts, item);
        guest.setWord(item + picker::kUnderPointer, std::uint16_t(left));
    }
    if (guest.word(item + picker::kUnderPointer) != 0)
    {
        left = fe::direct<0x49c080, glue::mouseKey>(app, ts);
        if (left == kKeyEnter)
            guest.setWord(item + picker::kPressed, guest.word(item + picker::kUnderPointer));
    }
    if (guest.word(item + picker::kPressed) == 0)
        return left;
    if (fe::direct<0x49c080, glue::mouseKey>(app, ts) == kKeyEnter)
        return guest::direct<0x43e7a0, pickerDragMoved>(app, ts, item);
    const u32 at = guest::direct<0x43e8e0, pickerHitRegion>(app, ts, item);
    left = at;
    if (at == u(s16At(guest, item + picker::kPressed)))
    {
        const std::uint16_t pressed = guest.word(item + picker::kPressed);
        if (pressed == picker::kHitOk || pressed == picker::kHitCancel || pressed == picker::kHitOutside)
        {
            playEffect(guest, ts, 0);
            if (pressed == picker::kHitOk)
                left = guest::direct<0x43dd90, acceptColour>(app, ts, item);
            else if (pressed == picker::kHitCancel)
                left = guest::direct<0x43dc90, applyColourToCar>(app, ts, item);
            else
                left = guest::direct<0x43ddd0, revertColour>(app, ts, item);
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
nfs::u32 frontend::menua::drawColourPicker(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (shownNow(guest, ts, item, picker::kWidget) == 0)
        return 0;
    guest::direct<0x43ead0, pickerMouse>(app, ts, item);
    guest::direct<0x43dfb0, drawPickerFrame>(app, ts, item);
    guest::direct<0x43e040, drawColourSwatch>(app, ts, item);
    if (guest.byte(kDisplayFlags) & kNoTranslucency)
        renderState(guest, ts, 6, 1);
    guest::direct<0x43e310, drawColourTriangle>(app, ts, item);
    if (guest.byte(kDisplayFlags) & kNoTranslucency)
        fe::direct<0x431900, graphics::stateAnswer>(app, ts, 6, 0);
    guest::direct<0x43e490, drawValueBar>(app, ts, item);
    guest::direct<0x43e1f0, drawColourMarkers>(app, ts, item);
    guest::direct<0x43e0d0, drawPickerButtons>(app, ts, item);
    return 0;
}

namespace
{

/* A step of a colour's number by the keys. */
void stepColour(Guest& guest, u32 at, s32 step)
{
    guest.setDword(at, u(clampByte(s32(guest.dword(at) + u(step)))));
}

}  // namespace

/* sub_43ec40: the picker's keys: the arrows move the colour, Tab goes from
 * button to button, Enter is the button's, Escape takes back. */
nfs::u32 frontend::menua::pickerKey(App* app, ThreadState& ts, u32 item, u32, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (shownNow(guest, ts, item, picker::kWidget) == 0)
        return 0;
    switch (key)
    {
    case kKeyEscape:
        // By address, as at Enter and the pointer's move at Tab: what they leave in edx, the function leaves.
        playEffectLast(guest, 0);
        guest::direct<0x43ddd0, revertColour>(app, ts, item);
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
        const u32 left = ok ? picker::kOkLeft : picker::kCancelLeft;
        const u32 width = ok ? picker::kOkWidth : picker::kCancelWidth;
        const u32 top = ok ? picker::kOkTop : picker::kCancelTop;
        const u32 height = ok ? picker::kOkHeight : picker::kCancelHeight;
        guest.setDword(kPointerX, u(s16At(guest, item + kItemX) + s16At(guest, item + left) + halved(s16At(guest, item + width))));
        guest.setDword(kPointerY, u(s16At(guest, item + kItemY) + s16At(guest, item + top) + halved(s16At(guest, item + height))));
        playEffect(guest, ts, 1);
        setPointerLast(guest, guest.dword(kPointerX), guest.dword(kPointerY));
        return 2;
    }
    case kKeyEnter:
    {
        const std::uint16_t under = guest.word(item + picker::kUnderPointer);
        if (under != picker::kHitOk && under != picker::kHitCancel)
            return 0;
        playEffectLast(guest, 0);
        if (under == picker::kHitOk)
            guest::direct<0x43dd90, acceptColour>(app, ts, item);
        else
            guest::direct<0x43dc90, applyColourToCar>(app, ts, item);
        return 2;
    }
    default:
        return 0;
    }
}

/* sub_43ef80: what the picker has the pointer show over it: 2 while the bar is
 * being dragged, 5 the triangle, 1 and 5 over them, 0 otherwise. */
nfs::u32 frontend::menua::pickerPointerKind(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (shownNow(guest, ts, item, picker::kWidget) == 0)
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
nfs::u32 frontend::menua::pickerFitBox(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 shown = shownNow(guest, ts, item, picker::kWidget);
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
nfs::u32 frontend::menua::pickerPrepare(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 shown = shownNow(guest, ts, item, picker::kWidget);
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
    guest.setWord(item + picker::kBackShape, std::uint16_t(shapeNamed(guest, ts, kPickerShapes[0].address)));
    guest.setWord(item + picker::kFrameShape, std::uint16_t(shapeNamed(guest, ts, kPickerShapes[1].address)));
    guest.setWord(item + picker::kPanelShape, std::uint16_t(shapeNamed(guest, ts, kPickerShapes[2].address)));
    guest.setWord(item + picker::kCancelLitShape, std::uint16_t(shapeNamed(guest, ts, kPickerShapes[3].address)));
    const u32 lit = shapeNamed(guest, ts, kPickerShapes[4].address);
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
    return guest::direct<0x43db20, makeHueRamp>(app, ts);
}

/* sub_43f160: whether the picker is shown now. */
nfs::u32 frontend::menua::pickerShown(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return shownNow(guest, ts, item, picker::kWidget);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The network lobby's panel
 * ------------------------------------------------------------------------- */

// Whether the panel is open, whether the pointer is on it, where it is sliding.
constexpr u32 kPanelOpen = 0x555748;
constexpr u32 kPanelActive = 0x5556d0;
constexpr u32 kPanelTop = 0x5f3fc8;
constexpr u32 kPanelLeft = 0x5f3fcc;
constexpr u32 kPanelShapeId = 0x5556cc;        // int16: none, no panel
constexpr u32 kPanelSliding = 0x5f3fd8;
constexpr u32 kPanelSpin = 0x555750;
// The rectangle of its messages.
constexpr u32 kBoxLeft = 0x5f3fbc;
constexpr u32 kBoxRight = 0x5f3fb8;
constexpr u32 kBoxTop = 0x5f3fc4;
constexpr u32 kBoxBottom = 0x5f3fc0;
// The line being typed and the other buffer of messages.
constexpr u32 kTyping = 0x5f3fdc;
constexpr u32 kMessageBuffer = 0x5f3fd0;
constexpr u32 kCursorBlink = 0x5f40bc;
constexpr u32 kKeyPressed = 0x664654;

// The network's state.
constexpr u32 kNetState = 0x7a229c;            // word
constexpr u32 kNetClosing = 0x7a22da;          // word
constexpr u32 kNetLeft = 0x7a22dc;             // word
constexpr u32 kNetConnected = 0x7a22e0;

// Its shapes, int16 each.
constexpr u32 kIconPlain = 0x5f3ffa;
constexpr u32 kIconLit = 0x5f3ff8;
constexpr u32 kIconChosen = 0x5f3ff4;
constexpr u32 kGlowPlain = 0x5f3ff6;
constexpr u32 kGlowChosen = 0x5f4004;
constexpr u32 kButtonOpenPlain = 0x5f3ffe;
constexpr u32 kButtonOpenBack = 0x5f4000;
constexpr u32 kButtonClosedPlain = 0x5f3ffc;
constexpr u32 kButtonClosedBack = 0x5f4002;

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
nfs::u32 frontend::menua::drawPlayerIcon(App* app, ThreadState& ts, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return drawShapeLast(guest, s16At(guest, kIconPlain), x, y);
}

/* sub_440b50: the icon of a player of the list, chosen or not, in a glow that
 * pulses and goes along the list with the time. */
nfs::u32 frontend::menua::drawPulsingIcon(App* app, ThreadState& ts, u32 chosen, u32 index, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0xc);
    constexpr s32 kPulse = 0x200;
    const s32 phase = s32(guest.dword(0x79c1a4) * 2) % kPulse;
    const s32 at = s32(u(phase) + ((7 - index) << 7)) % kPulse;
    const s32 glow = at >= 0x100 ? 0x1ff - at : at;
    const u32 colour = (u(glow) << 24) + 0xffffff;
    u32 result = colour;
    const u32 glowShape = chosen != 0 ? kGlowChosen : kGlowPlain;
    // Each call may be the last, by address: what it leaves in ebx, ecx and edx, the function leaves.
    if (guest.word(glowShape) != 0)
                result = u(*fe::direct<kDrawArtShape, port::shapes::drawArt>(guest.app(), ts, ts.sp - 4, s16At(guest, glowShape),
                                                                     s32(x - 6), s32(y - 5), colour));
    if (chosen != 0)
    {
        if (guest.word(kIconChosen) != 0)
            result = drawShapeLast(guest, s16At(guest, kIconChosen), x - 1, y + 1);
    }
    else if (guest.word(kIconLit) != 0)
    {
        result = drawShapeLast(guest, s16At(guest, kIconLit), x, y);
    }
    return result;
}

/* sub_440d60: the pointer put on the panel's messages. */
nfs::u32 frontend::menua::pointAtPanel(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    // By address: what the function leaves in eax is this one's.
    return setPointerLast(guest, guest.dword(kBoxLeft) + 0xa, guest.dword(kBoxTop) + 0xa);
}

namespace
{

/* The lines round the box of messages, of a colour for the pointer on it. */
void drawRacePanelLines(Guest& guest, nfs::ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 colour)
{
    shadedRectangle(guest, ts, left - 1, top + 2, right - 2, top + 4, colour);
    shadedRectangle(guest, ts, left - 2, top + 2, left, bottom, colour);
    shadedRectangle(guest, ts, right - 4, top + 2, right - 2, bottom, colour);
    shadedRectangle(guest, ts, left - 1, bottom - 0x12, right - 2, bottom - 0x10, colour);
    shadedRectangle(guest, ts, left - 2, bottom, right - 2, bottom + 2, colour);
}

}  // namespace

/* sub_440d80: the panel over a network race's pause: at the view's side, if it
 * is open; 4 when the network has gone. */
nfs::u32 frontend::menua::drawRacePanel(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x2c);
    constexpr u32 kBack = 0x80222240;
    const u32 rectTop = guest.frame(), rectRight = guest.frame() + 4;
    guest.setDword(rectTop, 0);
    guest.setDword(rectRight, 0);
    fe::direct<0x4beb20, pause::readRect>(app, ts, 0x7cdac0u, rectTop, rectTop, rectRight, rectTop);
    const bool wideView = s32At(guest, rectRight) >= 0x280;
    guest.setDword(kBoxTop, 0x118);
    guest.setDword(kBoxRight, wideView ? 0x201 : 0x240);
    guest.setDword(kBoxBottom, 0x1a6);
    guest.setDword(kBoxLeft, wideView ? 0x80 : 0x40);
    const u32 left = guest.dword(kBoxLeft), top = guest.dword(kBoxTop);
    const u32 right = guest.dword(kBoxRight), bottom = guest.dword(kBoxBottom);
    if (guest.dword(kPanelOpen) != 0)
    {
        shadedRectangle(guest, ts, left - 1, top + 2, right - 2, bottom + 2, kBack);
        const u32 over = guest::direct<0x49c240, menud::pointerOverRect>(app, ts, left, top, right - left - 4, bottom - top - 3);
        if (over != 0 && !dialogShown(guest, ts))
        {
            if (networkGone(guest))
                closePanel(guest);
            else
                guest.setDword(kPanelActive, 1);
            drawRacePanelLines(guest, ts, left, top, right, bottom, 0xff807020);
        }
        else
        {
            guest.setDword(kPanelActive, 0);
            drawRacePanelLines(guest, ts, left, top, right, bottom, 0xff000040);
        }
        const u32 scale = fe::direct<0x4d1400, port::text::keepTextScaleOf>(app, ts);
        // The shade where the game's code pushes it, for the function to rewrite on a race's pause screens; the
        // scale kept in eax, as the call before leaves it.
        const u32 shadeAt = guest.frame() - 4;
        guest.setDword(shadeAt, 0x3f19999a);
        fe::direct<0x451530, port::text::dimmed>(app, ts, shadeAt, scale);
        guest::direct<0x441950, drawPanelMessages>(app, ts, left + 1, top + 3);
        fe::direct<0x4d1410, port::text::restoreTextScaleOf>(app, ts);
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
nfs::u32 frontend::menua::panelFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x24);
    constexpr u32 kEdge = 0xff1c1a73, kInner = 0xff0f0f4a;
    constexpr u32 kPaneDark = 0xff05053a, kPaneLight = 0xff141358;
    constexpr s32 kOpenLeft = 0x110, kClosedLeft = 0x251, kSlide = 0x28;
    constexpr u32 kPanelMost = 0x1b0;
    if (networkGone(guest))
        closePanel(guest);
    if (guest.dword(kInRace) != 0)
        return guest::direct<0x440d80, drawRacePanel>(app, ts);
    if (guest.word(kPanelShapeId) == 0)
        return 0;

    u32 buttonTurn;
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

    drawShape(guest, ts, s16At(guest, kPanelShapeId), guest.dword(kPanelLeft), guest.dword(kPanelTop));
    const u32 panelLeft = guest.dword(kPanelLeft);
    const u32 left = panelLeft + 0x2f;
    const u32 right = panelLeft + 0x170;
    const u32 inner = right - 4;
    shadedRectangle(guest, ts, left, 0x87, panelLeft + 0x31, kPanelMost, kInner);
    shadedRectangle(guest, ts, inner, 0x87, right, kPanelMost, kEdge);
    shadedRectangle(guest, ts, left, 0x87, right, 0x8b, kEdge);
    shadedRectangle(guest, ts, left, 0x11d, right, 0x121, kEdge);
    shadedRectangle(guest, ts, left, 0x199, right, 0x19d, kEdge);
    shadedRectangle(guest, ts, left, 0x1ac, right, kPanelMost, kEdge);
    shadedRectangle(guest, ts, left, 0x8b, inner, 0x8d, kInner);
    shadedRectangle(guest, ts, left, 0x121, inner, 0x123, kInner);
    shadedRectangle(guest, ts, left, 0x19d, inner, 0x19f, kInner);
    shadedRectangle(guest, ts, left + 2, 0x8d, inner, 0x11d, kPaneLight, kPaneDark);
    shadedRectangle(guest, ts, left + 2, 0x123, inner, 0x199, kPaneDark, kPaneLight);
    shadedRectangle(guest, ts, left + 2, 0x19f, inner, 0x1ac, kPaneLight, kPaneDark);

    const u32 buttonX = guest.dword(kPanelLeft) + 0xd;
    const bool overButton = guest::direct<0x49c240, menud::pointerOverRect>(app, ts, buttonX, 0x18bu, 0x20u, 0x20u) != 0;
    const u32 turnedX = buttonX + 0x10;
    const bool plainShapes = (guest.byte(kDisplayFlags) & kNoTranslucency) != 0;
    if (overButton)
    {
        if (plainShapes)
        {
            drawShape(guest, ts, s16At(guest, kButtonOpenPlain), buttonX, 0x18b);
        }
        else
        {
            drawTurnedShape(guest, ts, s16At(guest, kButtonOpenBack), turnedX, 0x19b, buttonTurn);
            const u32 spin = guest.dword(kPanelSpin) + 0xa;
            guest.setDword(kPanelSpin, spin);
            drawTurnedShape(guest, ts, s16At(guest, kButtonOpenPlain), turnedX, 0x19b, spin);
        }
    }
    else if (plainShapes)
    {
        drawShape(guest, ts, s16At(guest, kButtonClosedBack), buttonX, 0x18b);
    }
    else
    {
        drawTurnedShape(guest, ts, s16At(guest, kButtonClosedPlain), turnedX, 0x19b, buttonTurn);
        const u32 spin = guest.dword(kPanelSpin) + 5;
        guest.setDword(kPanelSpin, spin);
        drawTurnedShape(guest, ts, s16At(guest, kButtonClosedBack), turnedX, 0x19b, spin);
    }

    guest::direct<0x49ae30, menud::drawLobbyPlayers>(app, ts, guest.dword(kPanelLeft) + 0x2f, guest.dword(kPanelTop));
    const u32 width = right - left - 4;
    guest.setDword(kBoxLeft, left);
    guest.setDword(kBoxTop, 0x120);
    guest.setDword(kBoxRight, left + width);
    guest.setDword(kBoxBottom, 0x1ac);
    if (guest::direct<0x49c240, menud::pointerOverRect>(app, ts, left, 0x120u, width, 0x8cu) != 0
        && !dialogShown(guest, ts))
    {
        if (networkGone(guest))
            closePanel(guest);
        else
            guest.setDword(kPanelActive, 1);
        constexpr u32 kLit = 0xff807020;
        shadedRectangle(guest, ts, left - 1, 0x11f, right - 2, 0x121, kLit);
        shadedRectangle(guest, ts, left - 2, 0x11f, left, 0x1ac, kLit);
        shadedRectangle(guest, ts, right - 4, 0x11f, right - 2, 0x1ac, kLit);
        shadedRectangle(guest, ts, left - 1, 0x19a, right - 2, 0x19c, kLit);
        shadedRectangle(guest, ts, left - 2, 0x1ac, right - 2, 0x1ae, kLit);
    }
    else
    {
        guest.setDword(kPanelActive, 0);
    }
    guest::direct<0x441950, drawPanelMessages>(app, ts, guest.dword(kPanelLeft) + 0x30, 0x11fu);

    if (guest.word(kNetClosing) == 0 && guest.word(kNetLeft) == 0 && guest.word(kNetState) == 0x21)
    {
        const u32 met = fe::direct<0x4a6a10, frontend::neta::barrier>(app, ts, 0x28, 0x21);
        fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, ts, met);  // eax as the call before left it
        return 0xb;
    }
    if (guest.dword(kTyping) != 0 && fe::direct<0x49a520, frontend::neta::lobbyChanged>(app, ts) == 0
        && fe::direct<0x49abd0, frontend::neta::allReady>(app, ts) != 0)
    {
        if (const u32 typing = guest.dword(kTyping))
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, typing);
            guest.setDword(kTyping, 0);
        }
        if (const u32 messages = guest.dword(kMessageBuffer))
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, messages);
            guest.setDword(kMessageBuffer, 0);
        }
        fe::direct<0x49acd0, frontend::neta::playersToRace>(app, ts);
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

u32 overPanelButton(Guest& guest, nfs::ThreadState& ts)
{
    return guest::direct<0x49c240, frontend::menud::pointerOverRect>(guest.app(), ts, guest.dword(kPanelLeft) + 0xd, 0x18bu,
                                                                      0x20u, 0x20u);
}

}  // namespace

/* sub_441680: whether the panel is to have the keys and the pointer: over its
 * button, or active. */
nfs::u32 frontend::menua::panelBusy(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (networkGone(guest))
    {
        closePanel(guest);
        return 0;
    }
    if (guest.dword(kInRace) != 0)
        return 0;
    if (overPanelButton(guest, ts) != 0)
        return 1;
    return guest.dword(kPanelActive) != 0 ? 1 : 0;
}

/* sub_441700: a key for the panel: Enter over its button opens and closes it;
 * while it is active or open the rest are the typed line's, an arrow or so
 * leaving it open.  2 when the key is spent. */
nfs::u32 frontend::menua::panelKey(App* app, ThreadState& ts, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    u32 result = 0;
    if (networkGone(guest))
        closePanel(guest);
    if (guest.word(kPanelShapeId) == 0)
        return 0;
    if (key == kKeyEnter && guest.dword(kFromMouse) != 0 && guest.dword(kInRace) == 0 && overPanelButton(guest, ts) != 0)
    {
        result = 2;
        guest.setDword(kPanelOpen, guest.dword(kPanelOpen) == 0 ? 1 : 0);
    }
    if (guest.dword(kPanelActive) == 0)
    {
        if (guest.dword(kPanelOpen) == 0)
            return result;
        const u32 pressed = guest.dword(kKeyPressed);
        if (pressed == kKeyUp || pressed == kKeyRight || pressed == kKeyDown || pressed == kKeyLeft
            || pressed == kKeyEnter || pressed == kKeyEscape)
            return result;
        guest::direct<0x440d60, pointAtPanel>(app, ts);
    }
    guest::direct<0x441b20, typeInPanel>(app, ts, guest.dword(kKeyPressed));
    return 2;
}

/* sub_4418e0: how wide a message may be, in the box's width. */
nfs::u32 frontend::menua::panelLineWidth(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const s32 across = s32(guest.dword(kBoxRight) - guest.dword(kBoxLeft));
    const u32 width = across > 0 ? u(across) : u32(0) - u(across);
    return width - 0x14;
}

/* sub_441900: whether a message fits a line, in the panel's font. */
nfs::u32 frontend::menua::panelLineFits(App* app, ThreadState& ts, u32 text)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (text == 0 || guest.byte(text) == 0)
        return 1;
    fe::direct<0x452100, port::text::selectFontOf>(app, ts, kMenuFont);
    const s32 width = s32(guest::direct<0x4418e0, panelLineWidth>(app, ts));
    return fe::textWidth(guest, text, kMenuFont) <= width ? 1 : 0;
}

/* sub_441950: the panel's messages, each cut until it fits its line, and under
 * them the line being typed, as much of its end as fits, and the cursor. */
nfs::u32 frontend::menua::drawPanelMessages(App* app, ThreadState& ts, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x7c);
    constexpr u32 kNothing = 0x43f180;
    constexpr u32 kPrompt = 0x23;
    constexpr u32 kLineLength = 0x64;
    constexpr u32 kMessageSize = 0x65;
    const u32 line = guest.frame();
    for (u32 i = 0; i < kLineLength; i += 4)
        guest.setDword(line + i, guest.dword(kNothing + i));
    if (guest.dword(kTyping) == 0 || guest.dword(kMessageBuffer) == 0)
        return x;
    fe::direct<0x42d9b0, raceload::messageLinesShown>(app, ts);
    const u32 messages = *fe::direct<0x437080, collide::messageLinesCopied>(guest.app(), guest.ts(), 0);
    u32 lineY = y;
    for (s32 i = 0;; ++i)
    {
        if (i >= s32At(guest, messages))
            break;
        fe::direct<0x4e0e30, watcom::clib::strncpy>(guest.app(), guest.ts(), line, messages + 4 + u(i) * kMessageSize, kLineLength);
        while (guest::direct<0x441900, panelLineFits>(app, ts, line) == 0 && guest.byte(line) != 0)
            guest.setByte(line + guest.length(line) - 1, 0);
        drawText(guest, ts, line, x, lineY, kColourChosen);
        lineY += 0xd;
    }
    fe::direct<0x452100, port::text::selectFontOf>(app, ts, kMenuFont);
    if (guest.dword(kTyping) == 0)
        return kMenuFont;  // eax as sub_452100 leaves it: the font
    const u32 typedY = y + 0x7c;
    const s32 room = s32(guest::direct<0x4418e0, panelLineWidth>(app, ts));
    const s32 promptWidth = fe::textWidth(guest, fe::text(guest, kPrompt), kMenuFont);
    s32 fit = minus(room, promptWidth);
    if (fit <= 0)
        fit = minus(0, fit);
    u32 skipped = 0;
    while (fe::textWidth(guest, guest.dword(kTyping) + skipped, kMenuFont) > fit)
        ++skipped;
    drawText(guest, ts, fe::text(guest, kPrompt), x, typedY, kColourPlain);
    const u32 typedX = x + u(promptWidth);
    drawText(guest, ts, guest.dword(kTyping) + skipped, typedX, typedY, kColourChosen);
    const s32 blink = s32(guest.dword(kCursorBlink) + 1);
    const s32 phase = blink % 0x1e;
    guest.setDword(kCursorBlink, u(phase));
    if (phase >= 0xf)
    {
        return u(blink / 0x1e);
    }
    const u32 typedWidth = u(fe::textWidth(guest, guest.dword(kTyping) + skipped, kMenuFont));
    // By address, the last call: what it leaves in edx, the function leaves.
    return drawTextLast(guest, kCursor.address, typedX + typedWidth, typedY, kColourChosen);
}

/* sub_441b20: a key for the line being typed: Enter sends it with the player's
 * name before it, Backspace and Delete take the last character off, Escape
 * nothing, what may be typed goes on its end, up to 0x56 of them.  2 when the key
 * is spent. */
nfs::u32 frontend::menua::typeInPanel(App* app, ThreadState& ts, u32 pressed)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x64);
    const std::uint16_t key = std::uint16_t(pressed);
    constexpr u32 kTypedMost = 0x56;
    const u32 typing = guest.dword(kTyping);
    if (typing == 0 || guest.dword(kPanelOpen) == 0 || guest.dword(kFromMouse) != 0)
        return 0;
    if (key == kKeyEnter)
    {
        const u32 message = guest.frame();   // char[0x64]
        guest.copyString(message, SETTINGS_ADDRESS(players[0].name));
        guest.copyString(message + guest.length(message), kNameSeparator.address);
        fe::direct<0x4eedf0, watcom::clib::strncat>(guest.app(), guest.ts(), message, guest.dword(kTyping), 0x57);
        fe::direct<0x42d810, raceload::typedMessageSent>(guest.app(), guest.ts(), message);
        fe::direct<0x4e0640, watcom::clib::memset>(guest.app(), guest.ts(), guest.dword(kTyping), 0, 0x64);
    }
    else if (key == kKeyBackspace || key == kKeyDelete)
    {
        const std::uint32_t length = guest.length(guest.dword(kTyping));
        if (length != 0)
            guest.setByte(guest.dword(kTyping) + length - 1, 0);
    }
    else if (key != kKeyEscape)
    {
        if ((guest::direct<0x443810, characterAllowed>(app, ts, key & 0xffu, 3u) & 0xff) != 0)
        {
            const u32 text = guest.dword(kTyping);
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
nfs::u32 frontend::menua::characterInRange(App*, ThreadState&, u32 character)
{
    const bool typable = s32(character) >= 0x20 && s32(character) <= 0xff;
    return withLowByte(character, typable ? 1 : 0);
}

/* sub_443510 and sub_443620: the same for a byte. */
nfs::u32 frontend::menua::byteInRange(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x4434f0, characterInRange>(app, ts, character & 0xff);
}

/* sub_443620: the same. */
nfs::u32 frontend::menua::byteInRangeAgain(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x4434f0, characterInRange>(app, ts, character & 0xff);
}

/* sub_4435c0: the characters of a number or a network address. */
nfs::u32 frontend::menua::characterOfAddress(App*, ThreadState&, u32 character)
{
    static const std::uint8_t kAllowed[26] = { 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const std::uint8_t index = std::uint8_t((character & 0xff) - 0x20);
    const std::uint8_t allowed = index < std::size(kAllowed) ? kAllowed[index] : 0;
    return (character & 0xffff0000) | (u32(index) << 8) | allowed;
}

namespace
{

/* sub_443520 and sub_4435e0: whether a whole text passes a character's test;
 * none does that is empty. */
template <typename Test>
u32 wholeTextPasses(Guest& guest, u32 text, Test test)
{
    if (text == 0 || guest.byte(text) == 0)
        return text & 0xffffff00;
    u32 left = text;
    for (u32 at = text;; ++at)
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
nfs::u32 frontend::menua::textInRange(App* app, ThreadState& ts, u32 text)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return wholeTextPasses(guest, text, [&](std::uint8_t c) { return guest::direct<0x443510, byteInRange>(app, ts, c); });
}

/* sub_4435e0: whether a whole text is a number or a network address (sub_4435c0). */
nfs::u32 frontend::menua::textOfAddress(App* app, ThreadState& ts, u32 text)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return wholeTextPasses(guest, text,
                           [&](std::uint8_t c) { return guest::direct<0x4435c0, characterOfAddress>(app, ts, c); });
}

namespace
{

/* sub_4436c0, sub_443740, sub_4437c0: a character by the test of the language's,
 * of a table of three; any without one. */
u32 languageCharacterTest(Guest& guest, u32 character, u32 table)
{
    const u32 row = gameSettings(guest).language * 20;
    const u32 test = guest.dword(table + row);
    if (test != 0)
        return callAt(guest, test, fe::args({ character & 0xff }));
    return withLowByte(row, 1);
}

}  // namespace

/* sub_4436c0, sub_443740, sub_4437c0: by the first, second and third tests of the table. */
nfs::u32 frontend::menua::languageTestFirst(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return languageCharacterTest(guest, character, 0x558c7c);
}

nfs::u32 frontend::menua::languageTestSecond(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return languageCharacterTest(guest, character, 0x558c84);
}

nfs::u32 frontend::menua::languageTestThird(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return languageCharacterTest(guest, character, 0x558c8c);
}

/* sub_443810: whether a character may be typed in a line of a kind (a name,
 * then the language's three). */
nfs::u32 frontend::menua::characterAllowed(App* app, ThreadState& ts, u32 character, u32 kind)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    switch (kind)
    {
    case 0:
        return fe::direct<0x443630, text::shortNameCharacter>(app, ts, character & 0xff);
    case 1:
        return guest::direct<0x4436c0, languageTestFirst>(app, ts, character & 0xff);
    case 2:
        return guest::direct<0x443740, languageTestSecond>(app, ts, character & 0xff);
    case 3:
        return guest::direct<0x4437c0, languageTestThird>(app, ts, character & 0xff);
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

constexpr u32 kModeCount = 0x559218;
constexpr u32 kModeLines = 0x5591ec;           // int16 for each: its line of the list
constexpr u32 kModes = 0x661220;
constexpr u32 kModeSize = 0x114;
constexpr u32 kModeHeight = 4;

}  // namespace

/* sub_4482c0: the list made afresh from the driver, by way of sub_448210. */
nfs::u32 frontend::menua::listModes(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest.setDword(kModeCount, 0);
    fe::direct<0x4e070c, ea::clearBytes>(guest.app(), guest.ts(), kModeLines, 0x2a);
    fe::direct<0x4bed90, graphics::screenModesListed>(guest.app(), guest.ts(), 0x448210);
    return 0;  // the eax it left, which no caller reads
}

/* sub_4482f0: the list made, the mode chosen still in it, its width. */
nfs::u32 frontend::menua::chosenModeWidth(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest::direct<0x4482c0, listModes>(app, ts);
    if (s32(gameSettings(guest).screenMode) >= s32At(guest, kModeCount))
        gameSettings(guest).screenMode = 0;
    return guest.dword(kModes + kModeSize * gameSettings(guest).screenMode);
}

/* sub_448330: the mode chosen, copied by way of the stack as the game's code
 * has it. */
nfs::u32 frontend::menua::copyChosenMode(App* app, ThreadState& ts, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x114);
    const u32 copy = guest.frame();
    if (s32At(guest, kModeCount) <= s32(gameSettings(guest).screenMode))
        gameSettings(guest).screenMode = 0;
    const u32 mode = kModes + kModeSize * gameSettings(guest).screenMode;
    for (u32 i = 0; i < kModeSize; i += 4)
        guest.setDword(copy + i, guest.dword(mode + i));
    for (u32 i = 0; i < kModeSize; i += 4)
        guest.setDword(out + i, guest.dword(copy + i));
    return out;
}

/* sub_4483a0: the list made, the mode of a width chosen; the first if none is. */
nfs::u32 frontend::menua::chooseModeOfWidth(App* app, ThreadState& ts, u32 width)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest::direct<0x4482c0, listModes>(app, ts);
    for (s32 i = 0; i < s32At(guest, kModeCount); ++i)
    {
        const u32 mode = kModes + kModeSize * u(i);
        if (width == guest.dword(mode))
        {
            const u32 height = guest.dword(mode + kModeHeight);
            gameSettings(guest).screenMode = u(i);
            gameSettings(guest).screenWidth = height;
            return height;
        }
    }
    const u32 height = guest.dword(kModes + kModeHeight);
    gameSettings(guest).screenWidth = height;
    gameSettings(guest).screenMode = 0;
    return height;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The race's pause menu
 * ------------------------------------------------------------------------- */

constexpr u32 kPauseNextAnswer = 0x559254;     // the answer to take next, set from outside
constexpr u32 kPauseDone = 0x559228;
constexpr u32 kCurrentMenu = 0x559230;
constexpr u32 kFirstMenu = 0x6040d0;
constexpr u32 kLastAnswer = 0x66296c;
constexpr u32 kPauseActive = 0x8b7630;
constexpr u32 kScreenStack = 0x6627fc;         // [1] to [20]
constexpr u32 kMenuCount = 0x660bb0;
constexpr u32 kItemShownBefore = 0x1c;         // of the screen: int16, the item selected before

constexpr GuestString kPauseFile{ 0x53905c, "pause" };
constexpr GuestString kPauseMultiFile{ 0x539054, "mspause" };
constexpr GuestString kPauseAdditionsFile{ 0x539064, "ad" };
constexpr GuestString kContinueItem{ 0x539068, "PSE_CONTINUE" };
constexpr GuestString kRestartItem{ 0x539078, "PSE_RESTART" };

void clearScreenStack(Guest& guest)
{
    for (u32 i = 1; i <= 0x14; ++i)
        guest.setDword(kScreenStack + 4 * i, 0);
}

u32 enterScreen(Guest& guest, nfs::ThreadState& ts, u32 menu)
{
    return u(*fe::direct<0x44ab40, port::loop::enterScreenOf>(guest.app(), ts, ts.sp - 4, menu));
}

u32 leaveScreen(Guest& guest, nfs::ThreadState& ts, u32 menu, u32 action)
{
    return u(*fe::direct<0x44aa30, port::loop::leaveScreenOf>(guest.app(), ts, ts.sp - 4, menu, action));
}

}  // namespace

/* sub_44b950: the pause menu as a race pauses for the first time: its text and
 * its screens loaded, its first screen open. */
nfs::u32 frontend::menua::racePauseMenuBegin(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr u32 kCommandLine = 0x55eb2e;     // byte
    constexpr std::uint8_t kAdditions = 0x20;
    constexpr u32 kPlayerState = 0x5efac8;
    guest.setDword(kInRace, 1);
    guest.setDword(kPauseNextAnswer, 0);
    guest.setDword(kPauseDone, 0);
    fe::direct<0x4d1750, port::text::loadedLanguageText>(app, ts, ts.sp - 4, s32(gameSettings(guest).language));
    clearScreenStack(guest);
    guest.setDword(kLastAnswer, 5);
    bool multi = false;
    if (guest.byte(kCommandLine) & kAdditions)
    {
        const u32 player = guest.dword(kPlayerState);
        multi = (guest.byte(player + 0x200) & 1) != 0 && (guest.dword(player + 0x2c4) == 3 || guest.dword(player + 0x2c4) == 4);
    }
    // ebx and ecx as they are handed on.
    const auto load = [&](u32 name, u32 reset) {
        fe::direct<0x449510, port::menus::menusLoaded>(app, ts, ts.sp - 4, name, reset,
                                                       port::Args{ 0, 0, 0, 0 });
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
    const u32 entered = enterScreen(guest, ts, kFirstMenu);
    guest.setDword(kPauseActive, 1);
    return entered;
}

/* sub_44ba30: the pause menu as a race pauses again, its screens loaded. */
nfs::u32 frontend::menua::racePauseMenuAgain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    guest.setDword(kPauseNextAnswer, 0);
    guest.setDword(kInRace, 1);
    clearScreenStack(guest);
    guest.setDword(kLastAnswer, 5);
    guest.setDword(kCurrentMenu, kFirstMenu);
    const u32 entered = enterScreen(guest, ts, kFirstMenu);
    guest.setDword(kPauseActive, 1);
    return entered;
}

namespace
{

u32 selectedItemOf(const Guest& guest, u32 menu)
{
    const s32 index = s16At(guest, menu + fe::menu::kFirstItem) + s16At(guest, menu + fe::menu::kSelected);
    return fe::item::kTable + u(index) * fe::item::kSize;
}

/* What the pointer is on and the key, for sub_44bac0: the answer of the dialog
 * or of the screen, 0 for none. */
u32 racePauseMenuInput(Guest& guest, nfs::ThreadState& ts)
{
    nfs::App* const app = guest.app();
    const u32 under = u(*fe::direct<0x44a6b0, port::loop::itemUnderMouseOf>(app, ts, ts.sp - 4,
                                                                              guest.dword(kCurrentMenu)));
    const u32 menu = guest.dword(kCurrentMenu);
    const s32 selected = s16At(guest, menu + fe::menu::kSelected);
    if (u(selected) != under
        && *fe::direct<0x449e50, port::loop::selectableOf>(app, ts, ts.sp - 4, selected, menu) != 0)
    {
        const u32 current = guest.dword(kCurrentMenu);
        guest.setWord(current + kItemShownBefore, guest.word(current + fe::menu::kSelected));
    }
    guest.setWord(guest.dword(kCurrentMenu) + fe::menu::kSelected, under == 0xffffffff ? 0x64 : std::uint16_t(under));
    const u32 key = u(*fe::direct<0x451960, port::text::menuKeyOf>(app, ts, ts.sp - 4, 1u));
    if ((key & 0xffff) == 0)
        return 0;
    const u32 asWord = u(std::int16_t(key));
    if (dialogShown(guest, ts))
        return u(*fe::direct<0x445e60, port::dialogs::dialogKeyOf>(app, ts, ts.sp - 4, asWord));
    return u(*fe::direct<0x44a2d0, port::loop::handleKeyOf>(app, ts, ts.sp - 4, guest.dword(kCurrentMenu), asWord));
}

/* The selected button followed to its next screen, for the answer 5 of sub_44bac0:
 * 5 when it is opened, 3 to go back when there is none, or the button is the
 * screen's Escape. */
u32 racePauseFollowButton(Guest& guest, nfs::ThreadState& ts)
{
    const u32 menu = guest.dword(kCurrentMenu);
    const u32 selected = selectedItemOf(guest, menu);
    guest.setDword(guest.frame(), 0);
    if (u(*fe::direct<0x44a240, port::loop::escapeButtonOf>(guest.app(), ts, ts.sp - 4, menu, guest.frame())) == selected)
        return 3;
    const u32 next = guest.dword(selected) == 5 ? guest.dword(selected + fe::item::kNextMenu) : 0;
    if (next == 0)
        return 3;
    u32 index = 0;
    while (s32(index) < s32At(guest, kMenuCount)
           && fe::direct<0x4ee310, watcom::clib::strcmp>(guest.app(), guest.ts(), next, guest.dword(kFirstMenu + fe::menu::kName + index * fe::menu::kSize)) != 0)
        ++index;
    if (s32(index) >= s32At(guest, kMenuCount))
        return 3;
    fe::direct<0x44af10, port::loop::pushMenuOf>(guest.app(), ts, 0);  // eax as strcmp left it: its 0, the names equal
    leaveScreen(guest, ts, guest.dword(kCurrentMenu), 5);
    const u32 to = kFirstMenu + index * fe::menu::kSize;
    guest.setDword(kCurrentMenu, to);
    enterScreen(guest, ts, to);
    return 5;
}

}  // namespace

/* sub_44bac0: a pass of the pause menu: what the pointer is on, the key, the
 * answer it gives (6, 10, 11, 12 close the screen, 9 reopens it, 5 follows the
 * selected button to its next screen, 3 and 1 go back, the first screen
 * closing).  The screen open then has its pass; otherwise the answer says what
 * the race is to do: 3, 4, 5 by the answer, 1 and 2 by the selected item. */
nfs::u32 frontend::menua::racePauseMenuFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc);
    bool screenStaysOpen = true;
    u32 answer = guest.dword(kPauseNextAnswer);
    if (answer != 0)
        guest.setDword(kPauseNextAnswer, 0);
    else
        answer = racePauseMenuInput(guest, ts);
    if (answer != 0)
    {
        guest.setDword(kLastAnswer, answer);
        switch (answer)
        {
        case 5:
            answer = racePauseFollowButton(guest, ts);
            break;
        case 6:
        case 10:
        case 11:
        case 12:
            leaveScreen(guest, ts, guest.dword(kCurrentMenu), answer);
            screenStaysOpen = false;
            break;
        case 9:
            leaveScreen(guest, ts, guest.dword(kCurrentMenu), answer);
            enterScreen(guest, ts, guest.dword(kCurrentMenu));
            break;
        default:
            break;
        }
        if (answer == 3 || answer == 1)
        {
            const u32 menu = guest.dword(kCurrentMenu);
            if (menu == kFirstMenu)
            {
                screenStaysOpen = false;
            }
            else
            {
                const u32 left = leaveScreen(guest, ts, menu, answer);
                fe::direct<0x44aec0, port::loop::popMenuOf>(app, ts, left);  // eax as the call before left it
                enterScreen(guest, ts, guest.dword(kCurrentMenu));
            }
        }
    }
    if (screenStaysOpen)
    {
        const u32 menu = guest.dword(kCurrentMenu);
        const u32 handler = guest.dword(menu + fe::menu::kOnFrame);
        if (handler != 0)
            callAt(guest, handler, fe::args({ menu, menu }));
        return 0;
    }
    if (answer == 12)
        return 3;
    if (answer == 11)
        return 4;
    if (answer == 10)
        return 5;
    const u32 selected = selectedItemOf(guest, guest.dword(kCurrentMenu));
    if (selected == 0 || guest.dword(selected + fe::item::kCodelink) == 0)
        return 0;
    // ebx as it is, handed on.
    const auto names = [&](const GuestString& name) {
        return *fe::direct<0x4429c0, port::menus::codelinkMatched>(app, ts, ts.sp - 4,
                                                                   guest.dword(selected + fe::item::kCodelink),
                                                                   name.address, 0) != 0;
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

/* The thunks: the frames and registers of the game's functions. */

/* sub_448210, the driver's list of modes handed to it one by one: a mode for the Screen
 * Size list, up to 20, with its line, "640 x 480 x 16", " (z)" when it has a depth
 * buffer.  The depth shown is the one a race is drawn in (the port's full colour change's
 * reason), and Dynamic Fullscreen's line its name (the port's widescreen change's). */
static constexpr GuestString kModeFormat{ 0x538d48, "%d x %d x %d" };
static constexpr GuestString kModeHasDepthBuffer{ 0x538d58, " (z)" };
static const GuestString kModeStrings[] = { kModeFormat, kModeHasDepthBuffer };

nfs::u32 frontend::menua::addScreenMode(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 depth,
                                          u32 depthBuffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    constexpr u32 kModeCount = 0x559218, kModes = 0x661220, kModeSize = 0x114;
    constexpr u32 kModeLineIds = 0x5591ec, kFirstLineId = 0x7a8;
    constexpr std::int32_t kMostModes = 20;
    const u32 count = guest.dword(kModeCount);
    const u32 entry = kModes + count * kModeSize;
    if (std::int32_t(count) >= kMostModes)
    {
        return count * kModeSize;
    }
    const u32 shown = shownDepth(depth);
    guest.setDword(entry, width);
    guest.setDword(entry + 4, height);
    guest.setDword(entry + 8, bits);
    guest.setDword(entry + 0xc, depth);
    guest.setDword(entry + 0x10, depthBuffer);
    const u32 line = entry + 0x14;
    // The game's code has the buffer flag and the entry in eax and edx, which sprintf does not read.
    const u32 answer = fe::direct<0x4df690, watcom::clib::sprintf>(app, ts, line, kModeFormat.address,
                                                                     guest::Words{height, bits, shown});
    // What is left in ecx: the buffer flag, or what looking for the line's end made of it.
    u32 ecx = guest.dword(entry + 0x10);
    if (ecx != 0)
    {
        const u32 length = guest.length(line);
        guest.copyString(line + length, kModeHasDepthBuffer.address);
        ecx = 0xffffffff - (length + 1);
    }
    dynamicFullscreenLabel(app, entry);
    const u32 index = guest.dword(kModeCount);
    const u32 id = ((answer & 0xffff0000) | guest.word(kModeCount)) + kFirstLineId;
    guest.setDword(kModeCount, index + 1);
    guest.setWord(kModeLineIds + index * 2, std::uint16_t(id));
    return id;
}

nfs::u32 frontend::controlsPollJoystickBinding(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43be40", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pollJoystickBinding(app, ts); });
}

nfs::u32 frontend::screenModeListAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448210", 4, 0, kModeStrings, std::size(kModeStrings), kEsiEdi };
    const u32 width = args.eax, height = args.edx, bits = args.ebx, depth = args.ecx;
    const u32 depthBuffer = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::addScreenMode(app, ts, width, height, bits, depth, depthBuffer); });
}

nfs::u32 frontend::controlsDrawBinding(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43c110", 4, 0, kBindingStrings, std::size(kBindingStrings), kEsiEdi };
    const u32 label = args.eax, binding = args.edx, x = args.ebx, y = args.ecx;
    const u32 chosen = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawBinding(app, ts, label, binding, x, y, chosen); });
}

nfs::u32 frontend::controlsDrawList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43c900", 0, 0, kListStrings, std::size(kListStrings), kEbxEcx | kEsiEdi };
    const u32 item = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::controlsList(app, ts, item, wanted); });
}

nfs::u32 frontend::controlsKeyAsBinding(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43cda0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::keyAsBinding(app, ts, character); });
}

nfs::u32 frontend::controlsWarnDigitalBinding(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ce80", 0, 0, nullptr, 0, kEbxEcx };
    const u32 slot = args.eax, binding = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::warnDigitalBinding(app, ts, slot, binding); });
}

nfs::u32 frontend::controlsListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43cf80", 0, 0, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const u32 item = args.eax, menu = args.edx;
    const std::uint16_t key = std::uint16_t(args.ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::listKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::controlsListFitBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d2b0", 0, 0, nullptr, 0, kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::listFitBox(app, ts, item); });
}

nfs::u32 frontend::controlsListPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d310", 0, 0, kPrepareStrings, std::size(kPrepareStrings) };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::listPrepare(app, ts, item); });
}

nfs::u32 frontend::controlsWaitJoystickRelease(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d650", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::waitJoystickRelease(app, ts); });
}

nfs::u32 frontend::controlsClearConflicts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d6a0", 0, 0, nullptr, 0 };
    const u32 binding = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::clearConflictingBindings(app, ts, binding); });
}

nfs::u32 frontend::controlsBindingKeyName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d7f0", 0, 0, kKeyNameStrings, std::size(kKeyNameStrings), kEsiEdi };
    const u32 binding = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::bindingKeyName(app, ts, binding, buffer); });
}

nfs::u32 frontend::controlsSampleAxes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d920", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::sampleAxesAtRest(app, ts, eax); });
}

nfs::u32 frontend::colourMakeHueRamp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43db20", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::makeHueRamp(app, ts); });
}

nfs::u32 frontend::colourApplyToCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43dc90", 0, 0, kApplyStrings, std::size(kApplyStrings) };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::applyColourToCar(app, ts, item); });
}

nfs::u32 frontend::colourAccept(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43dd90", 0, 0, nullptr, 0, kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::acceptColour(app, ts, item); });
}

nfs::u32 frontend::colourRevert(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ddd0", 0, 0, nullptr, 0, kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::revertColour(app, ts, item); });
}

nfs::u32 frontend::colourHsvToArgb(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43de20", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 hsv = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::hsvToArgb(app, ts, hsv, out); });
}

nfs::u32 frontend::colourTriangleFromColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43de60", 8, 0, nullptr, 0, kEsiEdi };
    const s32 width = s32(args.eax), height = s32(args.edx), across = s32(args.ebx), down = s32(args.ecx);
    const u32 outX = app->getMemory<u32>(ts.sp + 4), outY = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::triangleFromColour(app, ts, width, height, across, down, outX, outY); });
}

nfs::u32 frontend::colourFromTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43def0", 8, 0, nullptr, 0, kEsiEdi };
    const s32 width = s32(args.eax), height = s32(args.edx);
    const u32 outAcross = args.ebx, outDown = args.ecx;
    const s32 x = s32(app->getMemory<u32>(ts.sp + 4)), y = s32(app->getMemory<u32>(ts.sp + 8));
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::colourAtPlace(app, ts, width, height, outAcross, outDown, x, y); });
}

nfs::u32 frontend::colourDrawFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43dfb0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawPickerFrame(app, ts, item); });
}

nfs::u32 frontend::colourDrawSwatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e040", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawColourSwatch(app, ts, item); });
}

nfs::u32 frontend::colourDrawButtons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e0d0", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawPickerButtons(app, ts, item); });
}

nfs::u32 frontend::colourDrawMarkers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e1f0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawColourMarkers(app, ts, item); });
}

nfs::u32 frontend::colourDrawTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e310", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawColourTriangle(app, ts, item); });
}

nfs::u32 frontend::colourDrawValueBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e490", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawValueBar(app, ts, item); });
}

nfs::u32 frontend::colourDragMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e7a0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerDragMoved(app, ts, item); });
}

nfs::u32 frontend::colourHitRegion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43e8e0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerHitRegion(app, ts, item); });
}

nfs::u32 frontend::colourMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ead0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerMouse(app, ts, item); });
}

nfs::u32 frontend::colourDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ebc0", 0, 0, nullptr, 0, kEbxEcx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawColourPicker(app, ts, item); });
}

nfs::u32 frontend::colourKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ec40", 0, 0, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const u32 item = args.eax, menu = args.edx;
    const std::uint16_t key = std::uint16_t(args.ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::colourPointerKind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ef80", 0, 0, nullptr, 0, kEcxEdx | fe::kRestoreEsi };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerPointerKind(app, ts, item); });
}

nfs::u32 frontend::colourFitBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43eff0", 0, 0, nullptr, 0, kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerFitBox(app, ts, item); });
}

nfs::u32 frontend::colourPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f030", 0, 0, kPickerStrings, std::size(kPickerStrings), kEbxEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerPrepare(app, ts, item); });
}

nfs::u32 frontend::colourShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_43f160: whether the picker is shown now.
    static fe::Site site{ "sub_43f160", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pickerShown(app, ts, item); });
}

nfs::u32 frontend::lobbyDrawPlayerIcon(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440b30", 0, 0, nullptr, 0, kEbxEcx };
    const u32 x = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawPlayerIcon(app, ts, x, y); });
}

nfs::u32 frontend::lobbyDrawPulsingIcon(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440b50", 0, 0, nullptr, 0, kEsiEdi };
    const u32 chosen = args.eax, index = args.edx, x = args.ebx, y = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawPulsingIcon(app, ts, chosen, index, x, y); });
}

nfs::u32 frontend::lobbyPointAtPanel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440d60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::pointAtPanel(app, ts); });
}

nfs::u32 frontend::lobbyDrawRacePanel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440d80", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawRacePanel(app, ts); });
}

nfs::u32 frontend::lobbyPanelFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441040", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::panelFrame(app, ts); });
}

nfs::u32 frontend::lobbyPanelBusy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441680", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::panelBusy(app, ts); });
}

nfs::u32 frontend::lobbyPanelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441700", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEsi };
    const std::uint16_t key = std::uint16_t(args.eax);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::panelKey(app, ts, key); });
}

nfs::u32 frontend::lobbyPanelLineWidth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4418e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::panelLineWidth(app, ts); });
}

nfs::u32 frontend::lobbyPanelLineFits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441900", 0, 0, nullptr, 0, kEbxEcxEdx };
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::panelLineFits(app, ts, text); });
}

nfs::u32 frontend::lobbyDrawMessages(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441950", 0, 0, kMessagesStrings, std::size(kMessagesStrings), kEbxEcx | kEsiEdi };
    const u32 x = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::drawPanelMessages(app, ts, x, y); });
}

nfs::u32 frontend::lobbyTypeInPanel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441b20", 0, 0, kTypeStrings, std::size(kTypeStrings) };
    const u32 pressed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::typeInPanel(app, ts, pressed); });
}

nfs::u32 frontend::typedCharacterInRange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4434f0", 0, 0, nullptr, 0, kNoRestore };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::characterInRange(app, ts, character); });
}

nfs::u32 frontend::typedByteInRange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443510", 0, 0, nullptr, 0, kNoRestore };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::byteInRange(app, ts, character); });
}

nfs::u32 frontend::typedTextInRange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443520", 0, 0, nullptr, 0, kEbxEdx };
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::textInRange(app, ts, text); });
}

nfs::u32 frontend::typedCharacterOfAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4435c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::characterOfAddress(app, ts, character); });
}

nfs::u32 frontend::typedTextOfAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4435e0", 0, 0, nullptr, 0, kEbxEdx };
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::textOfAddress(app, ts, text); });
}

nfs::u32 frontend::typedByteInRangeAgain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443620", 0, 0, nullptr, 0, kNoRestore };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::byteInRangeAgain(app, ts, character); });
}

nfs::u32 frontend::typedLanguageTestFirst(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4436c0", 0, 0, nullptr, 0, kEcxEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::languageTestFirst(app, ts, character); });
}

nfs::u32 frontend::typedLanguageTestSecond(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443740", 0, 0, nullptr, 0, kEcxEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::languageTestSecond(app, ts, character); });
}

nfs::u32 frontend::typedLanguageTestThird(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4437c0", 0, 0, nullptr, 0, kEcxEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::languageTestThird(app, ts, character); });
}

nfs::u32 frontend::typedCharacterAllowed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443810", 0, 0, nullptr, 0, kNoRestore };
    const u32 character = args.eax, kind = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::characterAllowed(app, ts, character, kind); });
}

nfs::u32 frontend::modesList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4482c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::listModes(app, ts); });
}

nfs::u32 frontend::modesChosenWidth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4482f0", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::chosenModeWidth(app, ts); });
}

nfs::u32 frontend::modesCopyChosen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448330", 0, 0, nullptr, 0, kEbxEcxEdx | fe::kRestoreEdi };
    const u32 out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::copyChosenMode(app, ts, out); });
}

nfs::u32 frontend::modesChooseByWidth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4483a0", 0, 0, nullptr, 0, kEbxEdx | fe::kRestoreEdi };
    const u32 width = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::chooseModeOfWidth(app, ts, width); });
}

nfs::u32 frontend::racePauseBegin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44b950", 0, 0, kBeginStrings, std::size(kBeginStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::racePauseMenuBegin(app, ts); });
}

nfs::u32 frontend::racePauseAgain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ba30", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::racePauseMenuAgain(app, ts); });
}

nfs::u32 frontend::racePauseFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44bac0", 0, 0, kFrameStrings, std::size(kFrameStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menua::racePauseMenuFrame(app, ts); });
}

}
