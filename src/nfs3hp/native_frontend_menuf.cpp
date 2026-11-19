#include "native_frontend_menuf.h"
#include "native_load_saves.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_frontend_menua.h"
#include "native_controls_input.h"
#include "native_frontend_connect.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudz.h"
#include "native_frontend_loada.h"
#include "native_frontend_loop.h"
#include "native_frontend_menuc.h"
#include "native_frontend_neta.h"
#include "native_frontend_rendb.h"
#include "native_frontend_screen.h"
#include "native_frontend_wata.h"
#include "native_frontend_wina.h"
#include "native_graphics.h"
#include "os_window.h"
#include "port_buttons.h"
#include "port_loop.h"
#include "port_shapes.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include "native_guest.h"

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

/* The last of the menus' functions, decompiled:
 *
 *   the HUD editor's element   an element of a race's HUD, dragged about and
 *              resized over the frozen race (the pause screens themselves are
 *              in native_frontend_hudedit.cpp);
 *   the front end's start      sub_4730d0, what the game's state machine runs
 *              to enter the menus, the port's first settings in it;
 *   a screen's effects         sub_4a4a10, as an effects item's screen opens;
 *   three small ones           at a pause's and a race's end.
 *
 * An element's item is the 188 bytes of any item, with the fields below past
 * those fe::item names.  It stands in the race's pixels at kLeft, kTop, kRight
 * and kBottom, and in its layout record's 640 by 480 units at kPlaced..., the
 * four int16 the record keeps.  Drawing an element and judging its place stay
 * with the rest of the editor: sub_45a120, sub_45ab30, sub_459db0 and
 * sub_459fe0 are called where they are.
 *
 * Plan step 5: each function a typed core in frontend::menuf (native_frontend_menuf.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

// The port's own, in nfs3hp_main.cpp.  What sub_45bcb0 measures of what an
// element really draws, and the frame about that (the port's HUD editor change).
void hudEditorDraw(nfs::App* app, nfs::u32 object, bool begin);
void hudEditorFrame(nfs::App* app, nfs::u32 object, bool drawn);
// The phone's settings over a new player's (the port's first settings change):
// whether it laid them.
bool firstSettings(nfs::App* app);

namespace
{

using fe::Guest;
using fe::GuestString;
using nfs::Float;
using nfs::u32;

/* ---------------------------------------------------------------------------
 * The HUD editor's element
 * ------------------------------------------------------------------------- */

// An element's item, past fe::item's fields.
constexpr u32 kFlags = 0x05;              // byte; kWithKeys while the keys hold it
constexpr u32 kLeft = 0x06;               // int16, pixels
constexpr u32 kTop = 0x08;                // int16, pixels
constexpr u32 kPlacedTop = 0x1a;          // int16 each, on the 640 by 480 screen: its layout record's rectangle
constexpr u32 kPlacedLeft = 0x1c;
constexpr u32 kPlacedWidth = 0x1e;
constexpr u32 kPlacedHeight = 0x20;
constexpr u32 kElementNumber = 0x3c;      // past kElements, the second player's
constexpr u32 kRight = 0x60;              // pixels
constexpr u32 kBottom = 0x64;             // pixels
constexpr u32 kPlaceGood = 0x70;          // nonzero: it may stand where it stands; its frame's colour says so
constexpr u32 kGrip = 0x78;               // below
constexpr u32 kDragged = 0x7c;            // int16: the mouse holds it
constexpr u32 kHeld = 0x7e;               // int16: the keys hold it
constexpr std::uint8_t kWithKeys = 0x04;

constexpr std::int32_t kElements = 23;
constexpr std::int32_t kResizedFirst = 17, kResizedSecond = 22;  // these two elements alone have edges to grip

// What the pointer or the keys have hold of: in the high byte the edges -- 1
// the top, 2 the bottom, 4 the left, 8 the right -- in the low the cursor.
constexpr u32 kGripWhole = 0;
constexpr u32 kGripLeft = 0x409;
constexpr u32 kGripRight = 0x809;
constexpr u32 kGripTop = 0x108;
constexpr u32 kGripBottom = 0x208;
constexpr u32 kGripTopLeft = 0x50a;
constexpr u32 kGripBottomLeft = 0x60b;
constexpr u32 kGripTopRight = 0x90b;
constexpr u32 kGripBottomRight = 0xa0a;

constexpr u32 kPointerX = 0x55e5e4, kPointerY = 0x55e5e8;     // on the 640 by 480 screen
constexpr u32 kModeWidth = 0x7cdac8, kModeHeight = 0x7cdacc;  // the race's mode, in pixels
constexpr u32 kLastGoodPlace = 0x677e08;  // the 8 bytes at kPlacedTop, when last it could stand there
constexpr u32 kRoomTop = 0x677e10;        // what the keys have to move it in, down the screen, in pixels
constexpr u32 kRoomBottom = 0x677e14;

// The game's doubles.
constexpr u32 kCursorPerWide = 0x539c24;  // 1/640
constexpr u32 kCursorPerHigh = 0x539c34;  // 1/480
constexpr u32 kGripReach = 0x539c2c;      // 8: how near an edge the pointer grips it, in pixels
constexpr u32 kWideUnits = 0x539c3c;      // 640
constexpr u32 kPerWideUnit = 0x539c44;    // 1/640
constexpr u32 kHighUnits = 0x539c4c;      // 480
constexpr u32 kPerHighUnit = 0x539c54;    // 1/480
constexpr u32 kLeastWide = 0x539c5c;      // 16640: over the mode's width, the least it may be wide
constexpr u32 kLeastHigh = 0x539c6c;      // 12480
constexpr u32 kShrinkMargin = 0x539c64;   // -26: what stays of its size in pixels when a stroke takes the rest
constexpr u32 kRecordWideUnits = 0x539c74;  // 640 and 480, sub_45c9d0's own
constexpr u32 kRecordHighUnits = 0x539c7c;

// The game's functions.
constexpr std::uint32_t kVkShift = 0x10, kVkControl = 0x11;

constexpr std::uint32_t kFrameGood = 0x7fffffff, kFrameBad = 0xffff0000;

constexpr std::uint16_t kKeyEnter = 0x0d, kKeyUp = 0x4800, kKeyLeft = 0x4b00, kKeyRight = 0x4d00, kKeyDown = 0x5000;

constexpr std::int32_t kStrokesAcross = 80;  // a stroke of the keys is this part of the mode's width
constexpr u32 kKeySpent = 2;               // to the menu loop

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

std::int32_t sdword(const Guest& guest, u32 address)
{
    return std::int32_t(guest.dword(address));
}

std::int32_t sword(const Guest& guest, u32 address)
{
    return std::int16_t(guest.word(address));
}

// fild
Float integer(std::int32_t value)
{
    return FloatMath::integer(value);
}

/* What a jump after fcom goes by: jb, below or unordered, and ja, above and
 * ordered. */
bool belowOrUnordered(const Float& a, const Float& b)
{
    return (fp::compare(a, b) & 0x100) != 0;
}

bool above(const Float& a, const Float& b)
{
    return (fp::compare(a, b) & 0x4100) == 0;
}

/* The pointer's place in the race's pixels, off an edge of the element: how
 * near it is, positive.  fabs as the game writes it: a NaN stays, 0 goes -0. */
Float distanceFromEdge(const Guest& guest, u32 pointer, u32 mode, u32 perUnit, std::int32_t edge)
{
    const std::int32_t scaled = std::int32_t(guest.dword(pointer) * guest.dword(mode));
    const Float value = fp::sub(fp::mul(integer(scaled), doubleAt(guest, perUnit)), integer(edge));
    return belowOrUnordered(0.0, value) ? value : -value;
}

}  // namespace

/* sub_45b940: the grip the pointer has on the element under it -- none unless
 * it is one of the two that resize -- kept in the element, and the resize
 * cursor to show.  While it is held the grip stays what it was. */
nfs::u32 frontend::menuf::pickResizeCursor(App* app, ThreadState& ts, u32 element)
{
    Guest guest(app, ts);
    if (guest.word(element + kDragged) == 0 && guest.word(element + kHeld) == 0)
    {
        guest.setDword(element + kGrip, kGripWhole);
        const std::int32_t number = sdword(guest, element + kElementNumber);
        const std::int32_t own = number >= kElements ? number - kElements : number;
        if (own == kResizedFirst || own == kResizedSecond)
        {
            const Float reach = doubleAt(guest, kGripReach);
            const auto atSide = [&](std::int32_t edge) {
                return belowOrUnordered(distanceFromEdge(guest, kPointerX, kModeWidth, kCursorPerWide, edge), reach);
            };
            const auto atRow = [&](std::int32_t edge) {
                return belowOrUnordered(distanceFromEdge(guest, kPointerY, kModeHeight, kCursorPerHigh, edge), reach);
            };
            const bool left = atSide(sword(guest, element + kLeft));
            const bool right = !left && atSide(sdword(guest, element + kRight));
            const bool top = atRow(sword(guest, element + kTop));
            const bool bottom = !top && atRow(sdword(guest, element + kBottom));
            if (left)
                guest.setDword(element + kGrip, top ? kGripTopLeft : bottom ? kGripBottomLeft : kGripLeft);
            else if (right)
                guest.setDword(element + kGrip, top ? kGripTopRight : bottom ? kGripBottomRight : kGripRight);
            else if (top)
                guest.setDword(element + kGrip, kGripTop);
            else if (bottom)
                guest.setDword(element + kGrip, kGripBottom);
        }
    }
    return guest.dword(element + kGrip) & 0xff;
}

/* sub_45bcb0: an element, each frame of the editor: its drag when it is asked
 * for, what the race draws for it, measured by the port, and, on those that
 * have one, the frame about that, red where it may not stand. */
nfs::u32 frontend::menuf::drawElement(App* app, ThreadState& ts, u32 element, u32 dragging)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr std::int32_t kAlsoFramed[] = { 20, 13, 15, 6 };
    if (dragging != 0)
        fe::direct<0x45a120, frontend::wata::hudEditorDrag>(app, ts, element);
    hudEditorDraw(app, element, true);
    fe::direct<0x45ab30, frontend::hudr::drawElement>(app, ts, element);
    hudEditorDraw(app, element, false);
    const std::int32_t own = sdword(guest, element + kElementNumber) % kElements;
    bool framed = fe::direct<0x47bc20, frontend::rendb::kindIsText>(app, ts, u32(own)) != 0;
    for (const std::int32_t also : kAlsoFramed)
        framed = framed || own == also;
    if (framed)
    {
        const u32 colour = guest.dword(element + kPlaceGood) != 0 ? kFrameGood : kFrameBad;
        hudEditorFrame(app, element, true);
        fe::direct<0x459440, graphics::editorFrame>(app, ts, element, colour);
        hudEditorFrame(app, element, false);
    }
    return 0;
}

namespace
{

/* One way across the element: what the keys do across the screen and what they
 * do down it are the same code with these. */
struct Axis
{
    u32 start;        // kLeft or kTop
    u32 end;          // kRight or kBottom
    u32 placedStart;  // kPlacedLeft or kPlacedTop
    u32 placedSize;   // kPlacedWidth or kPlacedHeight
    u32 mode;         // kModeWidth or kModeHeight
    u32 units;        // 640 or 480: the record's screen
    u32 perUnit;      // its inverse
    u32 least;        // kLeastWide or kLeastHigh
    u32 pointer;      // kPointerX or kPointerY
    bool across;
};

constexpr Axis kAcross{ kLeft, kRight, kPlacedLeft, kPlacedWidth, kModeWidth, kWideUnits, kPerWideUnit,
                        kLeastWide, kPointerX, true };
constexpr Axis kDown{ kTop, kBottom, kPlacedTop, kPlacedHeight, kModeHeight, kHighUnits, kPerHighUnit,
                      kLeastHigh, kPointerY, false };

/* A stroke of the keys on an element they hold: by m_step pixels, less where
 * the room or its size says so.  What each leaves in edx is as the game's
 * function leaves it: no caller reads it. */
class ElementStroke
{
public:
    ElementStroke(Guest& guest, nfs::ThreadState& ts, u32 element, std::int32_t step)
        : m_guest(guest), m_ts(ts), m_element(element), m_step(step)
    {
    }

    void moveBack(const Axis& axis)
    {
        if (!mayGoBack(axis))
            return;
        if (axis.across)
            {}
        shortenToRoom(axis);
        moveStart(axis, -m_step);
        placeStart(axis);
        moveEnd(axis, -m_step);
        if (!axis.across)
            {}
        judgeAndPointAtMenu();
    }

    void moveForward(const Axis& axis)
    {
        const std::int32_t limit = highest(axis);
        if (!above(integer(limit), fp::add(sizeInPixels(axis), integer(start(axis)))))
            return;
        const std::int32_t room = std::int32_t(u32(limit) - u32(start(axis)));
        const Float space = fp::sub(integer(room), sizeInPixels(axis));
        if (above(integer(m_step), space))
            m_step = FloatMath(m_guest).truncated(space);
        moveStart(axis, m_step);
        placeStart(axis);
        moveEnd(axis, m_step);
        judgeAndPointAtMenu();
    }

    /* The start edge going back: bigger. */
    void growAtStart(const Axis& axis)
    {
        if (!mayGoBack(axis))
            return;
        shortenToRoom(axis);
        moveStart(axis, -m_step);
        const Float stroke = placeStartByInverse(axis);
        setPlacedSize(axis, FloatMath(m_guest).truncated(fp::add(stroke, placedSize(axis))));
        judgePlace();
        pointerTo(axis, sword(m_guest, m_element + axis.placedStart));
    }

    /* The start edge coming on: smaller, down to the least it may be. */
    void shrinkAtStart(const Axis& axis)
    {
        if (!mayShrink(axis))
            return;
        shortenToSize(axis);
        moveStart(axis, m_step);
        const Float stroke = placeStartByInverse(axis);
        setPlacedSize(axis, FloatMath(m_guest).truncated(fp::sub(placedSize(axis), stroke)));
        judgePlace();
        pointerTo(axis, sword(m_guest, m_element + axis.placedStart));
    }

    /* The end edge going back: smaller. */
    void shrinkAtEnd(const Axis& axis)
    {
        if (!mayShrink(axis))
            return;
        shortenToSize(axis);
        setPlacedSize(axis, FloatMath(m_guest).truncated(fp::sub(placedSize(axis), inUnits(axis, m_step))));
        moveEnd(axis, -m_step);
        judgePlace();
        pointerToEnd(axis);
    }

    /* The end edge going on: bigger, up to a pixel short of the room's end. */
    void growAtEnd(const Axis& axis)
    {
        const std::int32_t limit = highest(axis);
        const std::int32_t end = sdword(m_guest, m_element + axis.end);
        if (limit <= end)
            return;
        const std::int32_t room = std::int32_t(u32(limit) - u32(end));
        if (room <= m_step)
            m_step = room - 1;
        setPlacedSize(axis, FloatMath(m_guest).truncated(fp::add(inUnits(axis, m_step), placedSize(axis))));
        moveEnd(axis, m_step);
        judgePlace();
        pointerToEnd(axis);
    }

private:
    std::int32_t start(const Axis& axis) const { return sword(m_guest, m_element + axis.start); }
    Float placedSize(const Axis& axis) const { return integer(sword(m_guest, m_element + axis.placedSize)); }
    std::int32_t modeSize(const Axis& axis) const { return sdword(m_guest, axis.mode); }
    std::int32_t highest(const Axis& axis) const { return axis.across ? modeSize(axis) : sdword(m_guest, kRoomBottom); }

    /* Pixels to units by a product and a quotient.  placeStartByInverse has
     * the other way the game goes about it: the two round differently. */
    Float inUnits(const Axis& axis, std::int32_t pixels) const
    {
        return fp::div(fp::mul(integer(pixels), doubleAt(m_guest, axis.units)), integer(modeSize(axis)));
    }

    /* The placed size on the mode's screen, in pixels. */
    Float sizeInPixels(const Axis& axis) const
    {
        const u32 scaled = u32(sword(m_guest, m_element + axis.placedSize)) * u32(modeSize(axis));
        return fp::mul(integer(std::int32_t(scaled)), doubleAt(m_guest, axis.perUnit));
    }

    void setPlacedSize(const Axis& axis, std::int32_t size)
    {
        m_guest.setWord(m_element + axis.placedSize, std::uint16_t(size));
    }

    void moveStart(const Axis& axis, std::int32_t by)
    {
        m_guest.setWord(m_element + axis.start, std::uint16_t(m_guest.word(m_element + axis.start) + by));
    }

    void moveEnd(const Axis& axis, std::int32_t by)
    {
        m_guest.setDword(m_element + axis.end, m_guest.dword(m_element + axis.end) + u32(by));
    }

    void placeStart(const Axis& axis)
    {
        const std::int32_t place = FloatMath(m_guest).truncated(inUnits(axis, start(axis)));
        m_guest.setWord(m_element + axis.placedStart, std::uint16_t(place));
    }

    /* The start placed by the mode's inverse; the stroke in units, for the
     * size. */
    Float placeStartByInverse(const Axis& axis)
    {
        const Float units = doubleAt(m_guest, axis.units);
        const Float product = fp::mul(integer(start(axis)), units);
        const Float inverse = fp::div(1.0, integer(modeSize(axis)));
        const std::int32_t place = FloatMath(m_guest).truncated(fp::mul(product, inverse));
        m_guest.setWord(m_element + axis.placedStart, std::uint16_t(place));
        return fp::mul(fp::mul(units, integer(m_step)), inverse);
    }

    /* Whether the start has anywhere to go: the screen's left, or the top of
     * the room. */
    bool mayGoBack(const Axis& axis)
    {
        if (axis.across)
            return start(axis) > 0;
        return start(axis) > sdword(m_guest, kRoomTop);
    }

    void shortenToRoom(const Axis& axis)
    {
        const std::int32_t room = axis.across ? start(axis) : std::int32_t(u32(start(axis)) - m_guest.dword(kRoomTop));
        if (room < m_step)
            m_step = room;
    }

    bool mayShrink(const Axis& axis) const
    {
        return above(placedSize(axis), fp::div(doubleAt(m_guest, axis.least), integer(modeSize(axis))));
    }

    void shortenToSize(const Axis& axis)
    {
        const Float most = fp::add(sizeInPixels(axis), doubleAt(m_guest, kShrinkMargin));
        if (above(integer(m_step), most))
            m_step = FloatMath(m_guest).truncated(most);
    }

    /* sub_459db0's answer kept, and while it says yes the place remembered. */
    void judgePlace()
    {
        const u32 good = fe::direct<0x459db0, frontend::hudz::elementMoved>(m_guest.app(), m_ts, m_element);
        m_guest.setDword(m_element + kPlaceGood, good);
        if (good != 0)
        {
            m_guest.setDword(kLastGoodPlace, m_guest.dword(m_element + kPlacedTop));
            m_guest.setDword(kLastGoodPlace + 4, m_guest.dword(m_element + kPlacedTop + 4));
        }
    }

    void judgeAndPointAtMenu()
    {
        judgePlace();
        const u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(m_guest.app(), m_ts);
        fe::direct<0x449ea0, port::loop::pointAtSelectedOf>(m_guest.app(), m_ts, m_ts.sp - 4, menu);
    }

    /* The pointer goes with the edge. */
    void pointerTo(const Axis& axis, std::int32_t place)
    {
        m_guest.setDword(axis.pointer, u32(place));
        fe::direct<0x49bfc0, frontend::screen::setPointer>(m_guest.app(), m_ts, m_guest.dword(kPointerX),
                                                           m_guest.dword(kPointerY));
    }

    void pointerToEnd(const Axis& axis)
    {
        pointerTo(axis, FloatMath(m_guest).truncated(inUnits(axis, sdword(m_guest, m_element + axis.end))));
    }

    Guest& m_guest;
    nfs::ThreadState& m_ts;
    u32 m_element;
    std::int32_t m_step;
};


/* Shift and an arrow take hold of the whole element, to move it; Ctrl and an
 * arrow, on those that resize, of its edge on that side, the pointer going
 * there. */
void takeHold(Guest& guest, nfs::ThreadState& ts, u32 element, std::int32_t own, std::uint16_t key)
{
    const auto down = [&](std::uint32_t virtualKey) {
        return os::window::keyDown(virtualKey);
    };
    const auto hold = [&](u32 grip) {
        guest.setWord(element + kHeld, 1);
        guest.setDword(element + kGrip, grip);
        guest.setByte(element + kFlags, std::uint8_t(guest.byte(element + kFlags) | kWithKeys));
    };
    const auto pointerTo = [&](u32 pointer, std::int32_t place) {
        guest.setDword(pointer, u32(place));
        fe::direct<0x49bfc0, frontend::screen::setPointer>(guest.app(), ts, guest.dword(kPointerX),
                                                           guest.dword(kPointerY));
    };
    if (down(kVkShift) && (key == kKeyLeft || key == kKeyRight || key == kKeyUp || key == kKeyDown))
    {
        hold(kGripWhole);
        return;
    }
    if (!down(kVkControl) || (own != kResizedFirst && own != kResizedSecond))
        return;
    const std::int32_t left = sword(guest, element + kPlacedLeft), top = sword(guest, element + kPlacedTop);
    switch (key)
    {
    case kKeyLeft:
        hold(kGripLeft);
        pointerTo(kPointerX, left);
        break;
    case kKeyRight:
        hold(kGripRight);
        pointerTo(kPointerX, left + sword(guest, element + kPlacedWidth));
        break;
    case kKeyUp:
        hold(kGripTop);
        pointerTo(kPointerY, top);
        break;
    case kKeyDown:
        hold(kGripBottom);
        pointerTo(kPointerY, top + sword(guest, element + kPlacedHeight));
        break;
    default:
        break;
    }
}

/* The room the keys have, taken hold of: in split screen a player's half of
 * the screen.  The game's test is > 23, though element 23 is the second
 * player's too. */
void setRoom(Guest& guest, u32 element)
{
    const std::int32_t height = sdword(guest, kModeHeight);
    if (gameSettings(guest).playMode == 1 && sdword(guest, element + kElementNumber) <= kElements)
    {
        guest.setDword(kRoomTop, 0);
        guest.setDword(kRoomBottom, u32(height / 2));
    }
    else
    {
        guest.setDword(kRoomTop, gameSettings(guest).playMode == 1 ? u32(height / 2 + 1) : 0);
        guest.setDword(kRoomBottom, u32(height));
    }
}

}  // namespace

/* sub_45bd20: an element's keys: taken hold of, then the arrows a stroke at a
 * time, by what the keys have hold of; Enter lets go, puts it back where it
 * last could stand and gives the pointer back. */
nfs::u32 frontend::menuf::elementKey(App* app, ThreadState& ts, u32 element, u32, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x20);
    const std::int32_t own = sdword(guest, element + kElementNumber) % kElements;
    if (guest.word(element + kHeld) == 0)
    {
        takeHold(guest, ts, element, own, key);
        if (guest.word(element + kHeld) == 0)
            return 0;
        setRoom(guest, element);
    }

    const std::int32_t width = sdword(guest, kModeWidth);
    if (key == kKeyEnter)
    {
        guest.setByte(element + kFlags, std::uint8_t(guest.byte(element + kFlags) & ~kWithKeys));
        guest.setWord(element + kHeld, 0);
        fe::direct<0x459fe0, frontend::wata::hudEditorDrop>(app, ts, element, kLastGoodPlace);
        fe::direct<0x49c0e0, connect::setPointer>(app, ts, 0);  // the mouse has the pointer again
        const u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
        fe::direct<0x449ea0, port::loop::pointAtSelectedOf>(app, ts, ts.sp - 4, menu);
        return kKeySpent;
    }

    ElementStroke stroke(guest, ts, element, width / kStrokesAcross);
    switch (guest.dword(element + kGrip))
    {
    case kGripWhole:
        if (key == kKeyLeft)
            stroke.moveBack(kAcross);
        else if (key == kKeyRight)
            stroke.moveForward(kAcross);
        else if (key == kKeyUp)
            stroke.moveBack(kDown);
        else if (key == kKeyDown)
            stroke.moveForward(kDown);
        else
            return 0;
        return kKeySpent;
    case kGripLeft:
        if (key == kKeyLeft)
            stroke.growAtStart(kAcross);
        else if (key == kKeyRight)
            stroke.shrinkAtStart(kAcross);
        else
            return 0;
        return kKeySpent;
    case kGripRight:
        if (key == kKeyLeft)
            stroke.shrinkAtEnd(kAcross);
        else if (key == kKeyRight)
        {
            stroke.growAtEnd(kAcross);
        }
        else
            return 0;
        return kKeySpent;
    case kGripTop:
        if (key == kKeyUp)
            stroke.growAtStart(kDown);
        else if (key == kKeyDown)
            stroke.shrinkAtStart(kDown);
        else
            return 0;
        return kKeySpent;
    case kGripBottom:
        if (key == kKeyUp)
            stroke.shrinkAtEnd(kDown);
        else if (key == kKeyDown)
            stroke.growAtEnd(kDown);
        else
            return 0;
        return kKeySpent;
    default:
        return 0;
    }
}

/* sub_45c9d0: where an element stands in pixels, to its layout record's 640
 * by 480 rectangle.  eax stays the element. */
nfs::u32 frontend::menuf::elementToRecord(App* app, ThreadState& ts, u32 element)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0xc);
    FloatMath x87(guest);
    const Float wide = doubleAt(guest, kRecordWideUnits), high = doubleAt(guest, kRecordHighUnits);
    const Float perWide = fp::div(1.0, integer(sdword(guest, kModeWidth)));
    const Float perHigh = fp::div(1.0, integer(sdword(guest, kModeHeight)));
    const std::int32_t left = sword(guest, element + kLeft), top = sword(guest, element + kTop);
    guest.setWord(element + kPlacedLeft, std::uint16_t(x87.truncated(fp::mul(fp::mul(integer(left), wide), perWide))));
    guest.setWord(element + kPlacedTop, std::uint16_t(x87.truncated(fp::mul(fp::mul(integer(top), high), perHigh))));
    const std::int32_t width = std::int32_t(guest.dword(element + kRight) - u32(left) + 1);
    guest.setWord(element + kPlacedWidth, std::uint16_t(x87.truncated(fp::mul(perWide, fp::mul(integer(width), wide)))));
    const std::int32_t height = std::int32_t(guest.dword(element + kBottom) - u32(top) + 1);
    guest.setWord(element + kPlacedHeight, std::uint16_t(x87.truncated(fp::mul(perHigh, fp::mul(high, integer(height))))));
    return element;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The front end's start
 * ------------------------------------------------------------------------- */

constexpr u32 kFirstTime = 0x55b03c;         // the game has just started: the title, the settings
constexpr u32 kLoadingOff = 0x55e88c;
constexpr u32 kMenuBits1 = 0x55eb2d;         // byte
constexpr std::uint8_t kNoTitleMovie = 0x08;
constexpr u32 kAttractMovie = 0x55e54c;      // the attract movies played
constexpr u32 kConfigMadeAfresh = 0x67900c;  // no settings file: a new player
constexpr u32 kButtonHandler = 0x56446c;
constexpr u32 kGameButtonHandler = 0x49bfb0;
constexpr u32 kOnlyWrittenA = 0x559428, kOnlyWrittenB = 0x55942c;  // nothing reads these by address
constexpr u32 kCarsBytes = 0x50;
constexpr u32 kOpponentsBytes = 0x62;
constexpr u32 kNetworkAsked = 0x7a1fa0;      // a string: not empty, a network game to start at once
constexpr u32 kNetworkWays = 0x7a1ffc;
constexpr u32 kNetworkConnected = 0x7a22e0;
constexpr u32 kConnectionLost = 0x7a22dc;    // int16

// The screens the menu loop goes through first (setup's next), as char*[3] in
// the code segment, copied to the frame as the original's locals are: by how
// many are playing.
constexpr u32 kNextScreens = 0x4721c0, kNextScreensBytes = 12;
constexpr u32 kNextOfMain = 0, kNextOfSplit = kNextScreensBytes, kNextOfNetwork = 2 * kNextScreensBytes;

constexpr GuestString kTitleMovie{ 0x53b048, "titleav.mad" };
constexpr GuestString kSetup{ 0x53afb8, "setup" };
constexpr GuestString kStartStrings[] = { kTitleMovie, kSetup };

constexpr u32 kSaveConfig = 0x472820;
constexpr u32 kDetailSettings = 0x472d10;
constexpr u32 kFlushKeys = 0x4f0240;
constexpr u32 kFillZero = 0x4e070c;
constexpr u32 kNetworkFailed = 10;           // to the state machine

}  // namespace

/* sub_4730d0: the front end's start: the title movie and picture the first
 * time, a new player's settings chosen, the phone's laid over them and saved,
 * the race's choices forgotten, and the menu loop.  What the state machine does
 * next is what it hands back. */
nfs::u32 frontend::menuf::frontEndStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x24);
    const u32 lists = guest.frame();
    for (u32 at = 0; at < 3 * kNextScreensBytes; at += 4)
        guest.setDword(lists + at, guest.dword(kNextScreens + at));

    fe::direct<0x43b390, frontend::loada::removeKeptCars>(app, ts);
    guest.setDword(kButtonHandler, kGameButtonHandler);
    gameSettings(guest).raceFlags = 0;
    gameSettings(guest).cheatTrack = 0;
    guest.setDword(kOnlyWrittenA, 0);
    guest.setDword(kOnlyWrittenB, 0);
    fe::direct<0x473010, menuc::skipIntroCompare>(app, ts);
    if (gameSettings(guest).installIsRemote == 0 && guest.dword(kLoadingOff) == 0 && (guest.byte(kMenuBits1) & kNoTitleMovie) == 0
        && guest.dword(kFirstTime) != 0)
    {
        fe::direct<0x4960e0, glue::playMovie>(app, ts, kTitleMovie.address);
        fe::direct<0x4961e0, glue::resetAttractTimer>(app, ts);
        guest.setDword(kAttractMovie, 1);
    }
    fe::direct<0x473020, menuc::menusEnter>(app, ts);
    if (guest.dword(kFirstTime) != 0 && guest.dword(kLoadingOff) == 0)
    {
        fe::direct<0x472fc0, menuc::titleWait>(app, ts);
        if (guest.dword(kConfigMadeAfresh) != 0)
        {
            guest.setDword(kConfigMadeAfresh, 0);
            fe::direct<0x472d10, render::detailSettingsSet>(app, ts);
            // Port: the phone's settings over the new player's.  The game takes
            // up the new bindings and saves them, as it does leaving the front end.
            if (firstSettings(app))
            {
                fe::direct<0x43c040, glue::noteDigitalControls>(app, ts);
                fe::direct<0x472820, load::configSaved>(app, ts);
            }
        }
    }
    callAt(guest, kFlushKeys);
    gameSettings(guest).replayMode = 0;
    gameSettings(guest).lastTickRaced = 0;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), SETTINGS_ADDRESS(knockout), kCarsBytes);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), SETTINGS_ADDRESS(tournament), kOpponentsBytes);
    fe::direct<0x477c00, menuc::clearLoadFlag>(app, ts);

    u32 next = 0;
    if (guest.dword(kFirstTime) == 0)
    {
        const u32 players = gameSettings(guest).playMode;
        if (players == 0)
            next = lists + kNextOfMain;
        else if (players == 1)
            next = lists + kNextOfSplit;
        else
        {
            if (guest.dword(kNetworkConnected) != 0 && guest.word(kConnectionLost) == 0)
                next = lists + kNextOfNetwork;
            fe::direct<0x49ede0, frontend::neta::lobbyOpen>(app, ts);
        }
    }
    else if (guest.length(kNetworkAsked) != 0)
    {
        next = lists + kNextOfNetwork;
        const u32 started = std::int32_t(guest.dword(kNetworkWays)) > 1
                                  ? fe::direct<0x49e220, frontend::neta::joinRaceNet>(app, ts)
                                  : fe::direct<0x49e2e0, frontend::neta::hostRaceNet>(app, ts);
        if (started == 0)
        {
            fe::direct<0x473080, menuc::menusLeave>(app, ts);
            return kNetworkFailed;
        }
    }

    guest.setDword(kFirstTime, 0);
    gameSettings(guest).raceSeed = fe::direct<0x4b56d0, frontend::wina::tickCount>(app, ts);
    const u32 found = *fe::direct<0x4bee80, graphics::screenModeFor>(app, ts, gameSettings(guest).screenWidth);
    guest::direct<0x4483a0, menua::chooseModeOfWidth>(app, ts, found);  // the width in eax as sub_4bee80 leaves it
    const u32 answer = u32(*fe::direct<0x44b230, port::loop::menuLoopOf>(app, ts, ts.sp - 4, kSetup.address, next));
    fe::direct<0x473080, menuc::menusLeave>(app, ts);
    return answer;
}

namespace
{

/* ---------------------------------------------------------------------------
 * A screen's effects
 * ------------------------------------------------------------------------- */

constexpr u32 kBandColour = 0x7a1b8c;
constexpr u32 kDefaultBandColour = 0xff4060a0;
constexpr u32 kTrackColours = 0x55eae0;      // by track
constexpr u32 kTrackArtNames = 0x559620;     // char*, by track
constexpr u32 kTrackArtFolder = 0x7a31b4;
constexpr u32 kMenuArtFolder = 0x7a2db4;
constexpr u32 kMenuShapeFile = 0x559228;
constexpr u32 kBandClosing = 0x55eb10, kBandBottom = 0x55eb0c;
constexpr u32 kBandTallest = 0x1df;

constexpr u32 kItemShapeName = 0x3c;         // char*, in its art
constexpr u32 kItemArtFile = 0x40;           // char*, its own file of art, or 0
constexpr u32 kItemTexture = 0x44;           // int16

constexpr GuestString kLocation{ 0x53cf98, "location" };
constexpr GuestString kTrackInfo{ 0x53cfa4, "trkinfo" };
constexpr GuestString kRecords{ 0x53cfac, "records" };
constexpr GuestString kCar1{ 0x53cfb4, "car1" };
constexpr GuestString kCar2{ 0x53cfbc, "car2" };
constexpr GuestString kNoName{ 0x53cfc4, "" };
constexpr GuestString kTrackArtFormat{ 0x53cfc8, "%s%s.qfs" };
constexpr GuestString kBackShape{ 0x53cfd4, "back" };
constexpr GuestString kArtFormat{ 0x53cf18, "%s%s" };
constexpr GuestString kEffectsStrings[] = { kLocation, kTrackInfo, kRecords, kCar1, kCar2, kNoName,
                                            kTrackArtFormat, kBackShape, kArtFormat };

constexpr u32 kStrcmp = 0x4ee310;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kLocateShape = 0x4ef270;       // eax the file, edx a name
constexpr u32 kShapeTexture = 0x4d6390;      // eax a shape, edx the old texture

}  // namespace

/* sub_4a4a10: an effects item as its screen opens.  The band behind the lists
 * takes its colour -- by the track on the track's screens, by the car on the
 * car screens -- and, unless told not to, the item its shape's texture: on
 * trkinfo the track's own picture, otherwise its shape, from its own art or the
 * screen's.  The band starts tall, once.  What is left in eax is as the
 * game's function leaves it: the last call's answer. */
nfs::u32 frontend::menuf::startScreenEffects(App* app, ThreadState& ts, u32 item, u32 keepArt)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x54);
    const u32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, ts);
    u32 left = name;
    const u32 shown = name != 0 ? name : kNoName.address;
    guest.setDword(kBandColour, kDefaultBandColour);
    const auto is = [&](const GuestString& screen) {
        left = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), shown, screen.address);
        return left == 0;
    };
    const auto bandByCar = [&](u32 player) {
        left = fe::direct<0x4a46f0, frontend::wata::carBadgeColour>(app, ts);
        guest.setDword(kBandColour, left);
    };
    if (name != 0)
    {
        if (is(kLocation) || is(kTrackInfo) || is(kRecords))
            guest.setDword(kBandColour, guest.dword(kTrackColours + 4 * gameSettings(guest).track));
        if (is(kCar1))
            bandByCar(1);
        if (is(kCar2))
            bandByCar(2);
    }
    if (keepArt == 0)
    {
        const u32 path = guest.frame();
        const u32 file = guest.dword(item + kItemArtFile);
        const u32 shapeName = guest.dword(item + kItemShapeName);
        u32 shape = 0;
        if (is(kTrackInfo))
        {
            const u32 trackArt = guest.dword(kTrackArtNames + 4 * gameSettings(guest).track);
            fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kTrackArtFormat.address,
                                                        guest::Words{kTrackArtFolder, trackArt});
            // The item in ebx handed on (the game's code has the shape's name in edx too, which it does not read).
            const u32 art = u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0));
            shape = left = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, kBackShape.address);
        }
        else if (file != 0)
        {
            fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kArtFormat.address,
                                                        guest::Words{kMenuArtFolder, file});
            left = u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0));
            if (shapeName != 0)
                shape = left = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, left, shapeName);
        }
        else if (shapeName != 0)
            shape = left = callAt(guest, kLocateShape, fe::args({ guest.dword(kMenuShapeFile), shapeName }));
        else
            left = 0;
        if (shape != 0)
        {
                        left = u32(*fe::direct<kShapeTexture, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, 0, 0));
            guest.setWord(item + kItemTexture, std::uint16_t(left));
        }
    }
    if (guest.dword(kBandClosing) == 0)
    {
        guest.setDword(kBandBottom, kBandTallest);
        guest.setDword(kBandClosing, 1);
        left = 1;
    }
    return left;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The small ones
 * ------------------------------------------------------------------------- */

constexpr u32 kLabelsOn = 0x55fe3c;
constexpr u32 kRaceMouseButtons = 0x7a3d00, kRaceMouseX = 0x7a3cf8, kRaceMouseY = 0x7a3cfc;

}  // namespace

/* sub_478000: one of the things a race's end calls (native_graphics.cpp's
 * kRaceEnds), empty. */
nfs::u32 frontend::menuf::raceEndEmpty(App*, ThreadState&)
{
    return 0;   // it leaves eax as it came, which its thunk does; no caller reads it
}

/* sub_4bc530: the race's mouse read afresh as a pause ends, what it did in the
 * menus forgotten. */
nfs::u32 frontend::menuf::resetRaceMouse(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setDword(kLabelsOn, 0);
    return *fe::direct<0x435310, controls::mouseRead>(app, ts, kRaceMouseButtons, kRaceMouseX, kRaceMouseY, 0);
}

/* sub_4bc660: a race's view pass, pausing again: the pause menu (sub_44ba30). */
nfs::u32 frontend::menuf::pauseMenuAgain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x44ba30, menua::racePauseMenuAgain>(app, ts);
}

nfs::u32 frontend::hudElementPickCursor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45b940", 0, 0, nullptr, 0 };
    const u32 element = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::pickResizeCursor(app, ts, element); });
}

nfs::u32 frontend::hudElementDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45bcb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 element = args.eax, dragging = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::drawElement(app, ts, element, dragging); });
}

nfs::u32 frontend::hudElementKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45bd20", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 element = args.eax, menu = args.edx;
    const std::uint16_t key = std::uint16_t(args.ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::elementKey(app, ts, element, menu, key); });
}

nfs::u32 frontend::hudElementToRecord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45c9d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 element = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::elementToRecord(app, ts, element); });
}

nfs::u32 frontend::frontEndFirstStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4730d0", 0, 0, kStartStrings, std::size(kStartStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::frontEndStart(app, ts); });
}

nfs::u32 frontend::raceEndEmpty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_478000", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuf::raceEndEmpty(app, ts);
        return args.eax;
    });
}

nfs::u32 frontend::effectsScreenStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4a10", 0, 0, kEffectsStrings, std::size(kEffectsStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 item = args.eax, keepArt = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::startScreenEffects(app, ts, item, keepArt); });
}

nfs::u32 frontend::pauseResetRaceMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc530", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::resetRaceMouse(app, ts); });
}

nfs::u32 frontend::pauseMenuAgain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc660", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuf::pauseMenuAgain(app, ts); });
}

}
