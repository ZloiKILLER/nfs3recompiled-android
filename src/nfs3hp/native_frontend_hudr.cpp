#include "native_frontend_hudr.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_hudx.h"
#include "native_frontend_hudz.h"
#include "native_frontend_loopa.h"
#include "native_frontend_loopb.h"
#include "native_frontend_rendb.h"
#include "native_graphics.h"
#include "port_hud.h"
#include "port_text.h"
#include "native_vertices.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <cstring>
#include <initializer_list>
#include <iterator>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
x86::reg32 hudDrawElement(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudRebuildElement(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudStepMapPanes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudOrdinalLetters(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudFormatTime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawTime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawPercentAndCount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawLaps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawRatio(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawPlaceAmong(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawTriangle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudDrawLine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudFlushLines(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 hudPrepareLines(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}
}

/* The race's HUD above the port's own drawing (port_hud.cpp), decompiled:
 *
 *   the elements     one instrument of the HUD drawn, by the kind of element it
 *                    is, as the pause screens' editor shows it, each element an
 *                    item there (sub_45ab30, from the widget sub_45bcb0), and one
 *                    made again for the rectangle it now has (sub_4853c0);
 *   the map's pane   stepped on in a split screen until both players' agree
 *                    (sub_48e920);
 *   the HUD's text   a time, a percentage and a count, "Lap 2/3", a place among
 *                    so many and an ordinal's letters, formatted and handed to the
 *                    aligned text at 0x4897f0;
 *   the HUD's lines  a triangle and a line of the markers', with their clip
 *                    codes, and the line mesh's two ends.
 *
 * The HUD's state stays in guest memory, where the port's functions read it, and
 * what they do is called by address.
 *
 * Plan step 5: each function a typed core in frontend::hudr (native_frontend_hudr.h), in a frame of
 * its own where the game's function has one, and a thunk under the game's name for the calls by
 * address.  A core of this file calls another, and those of the HUD's other files done so
 * (native_frontend_hudx.cpp, _hudz.cpp), as C++ (guest::direct), and the typed cores of other files
 * too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using x86::Float;
using x86::reg32;
using x86::sreg32;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline reg32 callAt(Guest& guest, reg32 function, const fe::Arguments& arguments = fe::Arguments(), reg32 esi = 0,
                    double* floatResult = nullptr)
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<reg32> result = guest::pointerCall(guest.app(), cpu, function, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

/* fild, fld and sub_4dfd56 with fistp (fe::FloatMath's) as plain values: no x87 behind them. */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}
    static Float integer(std::int32_t value) { return Float(sreg32(value)); }
    Float load(reg32 address) const { return Float(m_guest.floatAt(address)); }
    std::int32_t truncated(const Float& value)
    {
        const double chopped = guest::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.cpu(), double(value));
        return fp::toInteger<sreg32>(chopped);  // fistp
    }

private:
    Guest& m_guest;
};

/* A game function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* The registers a function's pushes keep; the rest it leaves as they fall. */
constexpr std::uint8_t kKeepsEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kKeepsEcxEsiEdi = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsEbxEcxEsiEdi = fe::kRestoreEbx | kKeepsEcxEsiEdi;

// The game's functions, by address.
constexpr reg32 kSprintf = 0x4df690;

// The numbers the elements show.
constexpr reg32 kLapTime = 0x72522c;
constexpr reg32 kPosition = 0x725234;

// A car, as sub_422bd0 gives it.
constexpr reg32 kCarFlags = 0x200;             // byte; kCarIsCop: the cop's form of the HUD
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr reg32 kCarRatioA = 0x2c8;
constexpr reg32 kCarRatioB = 0x2cc;
constexpr reg32 kCarCount = 0x2d8;             // the count under a percentage
constexpr reg32 kCarGood = 0x2dc;              // the percentage's part
constexpr reg32 kCarOther = 0x2e0;             // and the rest of its whole
constexpr reg32 kCarGear = 0x50b;              // byte
constexpr reg32 kCarData = 0x528;              // its numbers: kDataGears, kDataRevLimit
constexpr reg32 kCarRevs = 0x590;
constexpr reg32 kDataGears = 8;
constexpr reg32 kDataRevLimit = 0x1c4;

// The HUD's records (native_frontend_hudedit.cpp), by seat and by form.
constexpr reg32 kRecordsPerSeat = 0x348;
constexpr reg32 kRecordsPerForm = 0x1a4;
constexpr reg32 kRectangleBytes = 16;

// The kinds of element, a rectangle each, by what they draw.
enum Kind : reg32
{
    kSpeedFigures = 0,
    kSpeedDial = 1,
    kRevFigures = 2,
    kRevDial = 3,
    kLaps = 4,
    kLapTimeText = 5,
    kGrid = 6,
    kPercentAndCount = 7,
    kStartLights = 8,
    kKind9 = 9,
    kPlace = 10,
    kTimeZero = 11,
    kLabelLine = 12,
    kStandings = 13,
    kRatioFirst = 14,
    kKeyLine = 15,
    kSegmentMeter = 16,
    kMap = 17,
    kMarkers = 18,
    kRatioSecond = 19,
    kLabelRows = 20,
    kPanel = 21,
    kCopMap = 22,
};

/* The car a view looks at (sub_422bd0, as C++). */
reg32 cameraTarget(Guest& guest, x86::CPU& cpu, reg32 view)
{
    return fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), cpu, view);
}

/* A render state set (sub_431900, as C++): its answer, which it leaves in eax. */
reg32 renderState(Guest& guest, x86::CPU& cpu, reg32 state, reg32 value)
{
    return *fe::direct<0x431900, graphics::stateAnswer>(guest.app(), cpu, state, value);
}

/* Line `number` of the language's text (sub_4d1850, as C++; edx 0: only a name past
 * the lines reads it). */
reg32 languageLine(Guest& guest, x86::CPU& cpu, std::uint32_t number)
{
    return reg32(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), cpu, cpu.esp - 4, std::int32_t(number), 0));
}

/* Whether a car is a cop's, as 0 or 1: the form of the HUD its view has. */
reg32 formOf(const Guest& guest, reg32 car)
{
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

/* The queue's quads drawn: what the code leaves in eax. */
reg32 flushQueue(Guest& guest, reg32 queue)
{
    return reg32(*fe::direct<0x49be40, port::text::quadsFlushed>(guest.app(), guest.cpu(), guest.cpu().esp - 4, queue));
}

/* The queue's quad of the four corners from `first`. */
reg32 queueQuad(Guest& guest, reg32 queue, reg32 first)
{
    return reg32(*fe::direct<0x49bd30, port::text::quadQueued>(guest.app(), guest.cpu(), guest.cpu().esp - 8, queue, first, first + 1, first + 2, first + 3));
}

reg32 unqueueQuad(Guest& guest, reg32 queue, reg32 first)
{
    return reg32(*fe::direct<0x49bdc0, port::text::quadUnqueued>(guest.app(), guest.cpu(), guest.cpu().esp - 8, queue, first, first + 1, first + 2, first + 3));
}

/* ---------------------------------------------------------------------------
 * An element made again: sub_4853c0
 * ------------------------------------------------------------------------- */

constexpr reg32 kViewWidth = 0x7cdac8;
constexpr reg32 kViewHeight = 0x7cdacc;
constexpr reg32 kBuildMapViews = 0x48d110;     // eax a view, edx and ebx its size

/* The start lights stand in one place on both players' screens: the rectangle
 * just set, by seat 1 or seat 2, copied to the other's and made again there. */
void shareStartLights(Guest& guest, x86::CPU& cpu, reg32 seat, reg32 form, reg32 width, reg32 height)
{
    if (seat != 1 && seat != 2)
        return;
    const reg32 otherSeat = 3 - seat;
    const reg32 otherView = seat == 1 ? 1 : 0;
    const reg32 otherForm = formOf(guest, cameraTarget(guest, cpu, otherView));
    gameSettings(guest).hudOf(otherSeat, otherForm).rects[kStartLights] = gameSettings(guest).hudOf(seat, form).rects[kStartLights];
    fe::direct<0x480910, render::hudRectPlaced>(guest.app(), cpu, otherView, kStartLights, width, height);
    guest::direct<0x480a00, frontend::hudz::setAlignments>(guest.app(), cpu, otherView);
}

}  // namespace

/* sub_4853c0: one element of a view's HUD made again for the rectangle it now
 * has: eax the view, edx the element.  A dial's band comes with the forms that
 * have one; a kind with nothing of its own goes without. */
x86::reg32 frontend::hudr::rebuildElement(App* app, CPU& cpu, reg32 view, reg32 element)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x18);
    const reg32 width = guest.dword(kViewWidth), height = guest.dword(kViewHeight);
    const reg32 seat = view + (gameSettings(guest).playMode == 1 ? 1 : 0);
    const reg32 form = formOf(guest, cameraTarget(guest, cpu, view));
    if (element == kStartLights)
        shareStartLights(guest, cpu, seat, form, width, height);
    fe::direct<0x480910, render::hudRectPlaced>(app, cpu, view, element, width, height);
    const reg32 placed = guest::direct<0x480a00, hudz::setAlignments>(app, cpu, view);
    const reg32 seatBytes = seat * kRecordsPerSeat, formBytes = form * kRecordsPerForm;
    const reg32 recordBytes = seatBytes + formBytes;
    switch (element)
    {
    case kSpeedFigures:
        return *fe::direct<0x480fc0, render::hudPicturePlaced>(app, cpu, view);
    case kSpeedDial:
    {
        fe::direct<0x480fc0, render::hudPicturePlaced>(app, cpu, view);
        const reg32 shown = gameSettings(guest).hudAt(recordBytes).speedometerForm;
        if (shown == 4 || shown == 2)
            return *fe::direct<0x4800a0, render::hudBandBuilt>(app, cpu, view);
        return recordBytes;
    }
    case kRevFigures:
        return *fe::direct<0x4812e0, render::hudDialPlaced>(app, cpu, view);
    case kRevDial:
    {
        fe::direct<0x4812e0, render::hudDialPlaced>(app, cpu, view);
        const reg32 shown = gameSettings(guest).hudAt(recordBytes).tachometerForm;
        if (shown == 2 || shown == 3)
            return *fe::direct<0x4800a0, render::hudBandBuilt>(app, cpu, view);
        return recordBytes;
    }
    case kGrid:
        return guest::direct<0x4818b0, hudz::layGrid>(app, cpu, view);
    case kStartLights:
        return guest::direct<0x4815d0, hudz::layReplayBar>(app, cpu, view);
    case kKind9:
        fe::direct<0x4be5e0, loopb::viewsLayout>(app, cpu, form);
        return fe::direct<0x47fd80, rendb::fitFrameToScreen>(app, cpu);
    case kStandings:
    case kLabelRows:
        return *fe::direct<0x481c50, render::hudTablePlaced>(app, cpu, view);
    case kSegmentMeter:
        return guest::direct<0x481570, hudz::layNothing>(app, cpu, view);
    case kMap:
    case kCopMap:
        // By address: the function goes out with the edx sub_48d110 leaves.
        callAt(guest, kBuildMapViews, args({ view, width, height }));
        return *fe::direct<0x481aa0, render::hudPanelPlaced>(app, cpu, view);
    case kMarkers:
        return guest::direct<0x481290, hudz::layMarkersFrame>(app, cpu, view, width);
    case kPanel:
        return guest::direct<0x481510, hudz::layPanel>(app, cpu, view);
    default:
        // A kind with nothing of its own has the jump table's index left in eax;
        // one past the table what placing it left.
        return element <= kCopMap ? element : placed;
    }
}

namespace
{

/* ---------------------------------------------------------------------------
 * An element drawn: sub_45ab30
 * ------------------------------------------------------------------------- */

// An element's item.
constexpr reg32 kItemKind = 0x3c;              // its kind; the second view's from kKindsPerView on
constexpr reg32 kItemLeftAlone = 0x70;         // nonzero: the editor's pointer is not on it
constexpr std::int32_t kKindsPerView = 23;

// The rectangles the elements are drawn in, by view and kind.
constexpr reg32 kBoxes = 0x749758;
constexpr reg32 kBoxesPerView = 0x170;
constexpr reg32 kBoxLeft = 0, kBoxTop = 4, kBoxRight = 8, kBoxBottom = 0xc;
// How each kind's text is aligned, by view and kind.
constexpr reg32 kAlignments = 0x725690;
constexpr reg32 kAlignmentsPerView = 0x5c;

constexpr reg32 kPulse = 0x79c1a4;             // counts on, frame by frame
constexpr reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kCardWithAlpha = 2;
constexpr reg32 kStateShape = 0x2bf;           // set around what is drawn apart on a card with alpha

constexpr reg32 kColourGrey = 0xffa0a0a0;
constexpr reg32 kColourWhite = 0xffffffff;
constexpr reg32 kColourRow = 0xff707070;
constexpr reg32 kColourNoAlpha = 0x7fffffff;

// The text's scales.
constexpr reg32 kScaleSmall = 0x791b3c;
constexpr reg32 kScaleRows = 0x791b4c;
constexpr reg32 kScaleLabel = 0x791b50;
constexpr reg32 kScaleLarge = 0x791b54;
constexpr reg32 kFullScaleSpeed = 0x42b2d1ec;  // a float, 89.41: the speed at which a speedometer is full

// The vertex groups and queues.
constexpr reg32 kBackQueue = 0x725748;         // the panels behind the instruments
constexpr reg32 kBackVertices = 0x725758;      // their corners, 0x20 each: x and y first
constexpr reg32 kDialQueue = 0x749a38;
constexpr reg32 kOverQueue = 0x76da48;
constexpr reg32 kBackQuads = 0x791adc;         // the first of the back queue's corners
constexpr reg32 kBackGroup = 0x791ad8;
constexpr reg32 kSpeedGroup = 0x791ac4;
constexpr reg32 kSpeedQuads = 0x791ad0;
constexpr reg32 kSpeedVertices = 0x791b38;
constexpr reg32 kRevGroup = 0x791b30;
constexpr reg32 kRevQuads = 0x791b24;
constexpr reg32 kRevVertices = 0x791b2c;
constexpr reg32 kMapGroup = 0x791b48;
constexpr reg32 kMapQuads = 0x791b20;
constexpr reg32 kPanelQuads = 0x791acc;
constexpr reg32 kLightsA = 0x749a40;
constexpr reg32 kLightsB = 0x749a44;

constexpr reg32 kListWidths = 0x724780;        // by view

// The port's functions the elements end in.

/* What sub_45ab30 works out for every kind. */
struct Element
{
    Guest& guest;
    x86::CPU& cpu;
    reg32 item;
    reg32 view;       // 0 or 1
    reg32 kind;
    reg32 car;        // the car the view looks at
    reg32 colour;
    reg32 top;        // the box's top and the text's alignment, as they were at the start
    reg32 alignment;

    reg32 box(reg32 field) const { return guest.dword(kBoxes + view * kBoxesPerView + kind * kRectangleBytes + field); }
    bool leftAlone() const { return guest.dword(item + kItemLeftAlone) != 0; }
    bool cardWithAlpha() const { return (guest.byte(kDisplayFlags) & kCardWithAlpha) != 0; }

    /* A text's arguments: its scale, colour, box and alignment on the stack. */
    fe::Arguments text(std::initializer_list<reg32> registers, reg32 scale) const
    {
        const reg32 left = box(kBoxLeft), right = box(kBoxRight);
        return args(registers, { scale, colour, left, right, top, alignment });
    }
};

/* The HUD's text drawn aligned (port_hud's), as C++: what it gives back in eax. */
reg32 alignedText(Guest& guest, reg32 text, reg32 scale, reg32 colour, reg32 x, reg32 dx, reg32 y, reg32 mode)
{
    return reg32(*fe::direct<0x4897f0, port::hud::textAlignedOf>(guest.app(), guest.cpu(), guest.cpu().esp - 4 - 4 * 6, text,
                                                                 scale, colour, x, dx, y, mode));
}

/* A line of the language's text at its place, the box read before the line. */
reg32 drawLanguageLine(Guest& guest, x86::CPU& cpu, fe::Arguments place, std::uint32_t line)
{
    const reg32 text = languageLine(guest, cpu, line);
    return alignedText(guest, text, place.stack[0], place.stack[1], place.stack[2], place.stack[3], place.stack[4],
                       place.stack[5]);
}

/* The gears' list down a speedometer's side: the chosen one and the last in
 * colours of their own when the editor's pointer is elsewhere. */
reg32 drawGearRows(const Element& e, reg32 left, reg32 right, reg32 bottom)
{
    Guest& guest = e.guest;
    const bool leftAlone = e.leftAlone();
    const reg32 scale = guest.dword(kScaleLarge);
    const reg32 gear = guest.byte(e.car + kCarGear);
    const reg32 gears = guest.dword(guest.dword(e.car + kCarData) + kDataGears) - 2;
    const reg32 drawn = leftAlone
        ? reg32(*fe::direct<0x489b30, port::hud::listRowsOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 7, gears, gear, 0u, scale,
                                                             kColourRow, kColourGrey, 0u, left, right, bottom))
        : reg32(*fe::direct<0x489b30, port::hud::listRowsOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 7, gears, gear, 0u, scale,
                                                             e.colour, e.colour, e.colour, left, right, bottom));
    // edx and ebx as the call by address leaves them.
    return drawn;
}

/* The frame around the element's item, for the one the editor's pointer is on. */
reg32 frameItem(const Element& e)
{
    return fe::direct<0x459410, frontend::loopa::itemFrame>(e.guest.app(), e.cpu, e.item, e.colour & kColourNoAlpha);
}

/* Kind 0: the speed in figures, and the gears' rows in the box's right part. */
reg32 drawSpeedFigures(const Element& e)
{
    constexpr reg32 kRowsShare = 0x539bfc;     // a double, 0.75
    Guest& guest = e.guest;
    const reg32 seat = (gameSettings(guest).playMode == 1 ? 1 : 0) + e.view;
    const reg32 form = gameSettings(guest).hudOf(seat, formOf(guest, e.car)).speedometerForm;
    const reg32 firstUnits = form == 1 || form == 2 ? 1 : 0;
    fe::direct<0x489d70, port::hud::percentTextOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 6, kFullScaleSpeed, firstUnits,
                                                   guest.dword(kScaleLabel), e.colour, e.box(kBoxRight), e.top);
    FloatMath math(guest);
    const Float left = FloatMath::integer(std::int32_t(e.box(kBoxLeft)));
    const Float right = FloatMath::integer(std::int32_t(e.box(kBoxRight)));
    const float rightStored = float(right);
    const float rows = float(fp::mul(fp::sub(right, left), doubleAt(guest, kRowsShare)));
    const float rowsLeft = float(fp::sub(Float(rightStored), Float(rows)));
    const reg32 rowsRight = reg32(math.truncated(Float(rightStored)));
    const reg32 rowsStart = reg32(math.truncated(Float(rowsLeft)));
    return drawGearRows(e, rowsStart, rowsRight, e.box(kBoxBottom));
}

/* Kind 1: the speedometer's dial, its needle and the gears' rows. */
reg32 drawSpeedDial(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 view = e.view;
    const reg32 seat = view + (gameSettings(guest).playMode == 1 ? 1 : 0);
    const reg32 form = formOf(guest, e.car);
    fe::direct<0x48a220, port::hud::indicatorGeometryOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 2, view, kFullScaleSpeed,
                                                         e.colour);
    guest::direct<0x481170, frontend::hudz::placeDialArt>(guest.app(), e.cpu, seat, form,
                                                         guest.dword(kSpeedVertices) + (view << 8));
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.cpu, guest.dword(kSpeedGroup) + (view << 5));
    const reg32 quads = guest.dword(kSpeedQuads) + view * 8;
    queueQuad(guest, kDialQueue, quads);
    queueQuad(guest, kDialQueue, quads + 4);
    drawGearRows(e, e.box(kBoxLeft), e.box(kBoxRight), e.box(kBoxBottom));
    const reg32 flushed = flushQueue(guest, kDialQueue);
    if (!e.cardWithAlpha())
        return flushed;
    if (e.leftAlone())
        return e.item;
    unqueueQuad(guest, kDialQueue, quads);
    return frameItem(e);
}

/* Kind 2: the revs in figures. */
reg32 drawRevFigures(const Element& e)
{
    Guest& guest = e.guest;
    return reg32(*fe::direct<0x489c40, port::hud::timeTextOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 5,
                                                              guest.dword(e.car + kCarRevs), guest.dword(kScaleLabel),
                                                              guest.dword(kScaleLarge), e.colour, e.box(kBoxRight), e.top));
}

/* Kind 3: the tachometer: its corners' texture coordinates for the size of its
 * art, half a texel in, and moved across to this view's part of it; its centre
 * and scales; its quad and its needle. */
reg32 drawRevDial(const Element& e)
{
    constexpr reg32 kArtWidth = 0x791ae8, kArtHeight = 0x791aea;  // words
    constexpr reg32 kDoubled = 0x539c04;       // a float, 2.0
    constexpr reg32 kArtExtent = 0x539c08;     // a float, 64.0
    constexpr reg32 kFirstViewShift = 0x539c0c, kSecondViewShift = 0x539bfc;  // doubles, 0.5 and 0.75
    constexpr reg32 kHalf = 0x539c14;          // a float
    constexpr reg32 kToTextScale = 0x539c18, kToNeedleScale = 0x539c1c;  // floats, 4.0 and 21.33
    constexpr reg32 kDialScale = 0x724760;     // by view
    constexpr reg32 kNeedleScale = 0x724758;   // by view
    constexpr reg32 kTextScale = 0x724778;     // by view
    constexpr reg32 kDialCentre = 0x724768;    // by view, x and y
    constexpr reg32 kViewVertices = 0xe0;
    constexpr reg32 kU0 = 0x18, kV0 = 0x1c, kU1 = 0x38, kV1 = 0x3c, kU2 = 0x58, kV2 = 0x5c, kU3 = 0x78, kV3 = 0x7c;
    constexpr reg32 kFirstX = 0, kFirstY = 4, kThirdX = 0x40, kThirdY = 0x44;

    Guest& guest = e.guest;
    const reg32 view = e.view;
    const reg32 corners = guest.dword(kRevVertices) + view * kViewVertices;

    const Float one(1.0);
    const Float doubled(guest.floatAt(kDoubled));
    const Float extent(guest.floatAt(kArtExtent));
    const Float artWidth = FloatMath::integer(guest.word(kArtWidth));
    const Float artHeight = FloatMath::integer(guest.word(kArtHeight));
    const float halfU = float(fp::div(one, fp::mul(artWidth, doubled)));
    const float halfV = float(fp::div(one, fp::mul(doubled, artHeight)));
    const float endU = float(fp::sub(fp::div(extent, artWidth), Float(halfU)));
    const float endV = float(fp::sub(fp::div(extent, artHeight), Float(halfV)));
    guest.setFloat(corners + kU0, halfU);
    guest.setFloat(corners + kV0, halfV);
    guest.setFloat(corners + kU1, endU);
    guest.setFloat(corners + kV1, halfV);
    guest.setFloat(corners + kU2, endU);
    guest.setFloat(corners + kV2, endV);
    guest.setFloat(corners + kU3, halfU);
    guest.setFloat(corners + kV3, endV);

    const Float shift = doubleAt(guest, view != 0 ? kSecondViewShift : kFirstViewShift);
    for (const reg32 u : { kU0, kU1, kU2, kU3 })
        guest.setFloat(corners + u, float(fp::add(Float(guest.floatAt(corners + u)), shift)));

    const Float scale(guest.floatAt(kDialScale + view * 4));
    const Float half(guest.floatAt(kHalf));
    const Float sumX = fp::add(Float(guest.floatAt(corners + kFirstX)), Float(guest.floatAt(corners + kThirdX)));
    const Float sumY = fp::add(Float(guest.floatAt(corners + kFirstY)), Float(guest.floatAt(corners + kThirdY)));
    guest.setFloat(kDialCentre + view * 8, float(fp::mul(sumX, half)));
    guest.setFloat(kDialCentre + view * 8 + 4, float(fp::mul(half, sumY)));
    guest.setFloat(kTextScale + view * 4, float(fp::mul(scale, Float(guest.floatAt(kToTextScale)))));
    guest.setFloat(kNeedleScale + view * 4, float(fp::mul(scale, Float(guest.floatAt(kToNeedleScale)))));

    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.cpu, guest.dword(kRevGroup) + (view << 5));
    const reg32 quad = guest.dword(kRevQuads) + view * 7;
    queueQuad(guest, kDialQueue, quad);
    const reg32 limit = guest.dword(guest.dword(e.car + kCarData) + kDataRevLimit);
    const reg32 revs = guest.dword(e.car + kCarRevs);
    fe::direct<0x489e00, port::hud::indicatorQuadOf>(guest.app(), e.cpu, e.cpu.esp - 4, view, revs, limit, e.colour);
    // edx, ebx and ecx as the call by address leaves them.
    flushQueue(guest, kDialQueue);
    const reg32 unqueued = unqueueQuad(guest, kDialQueue, quad);
    if (!e.cardWithAlpha())
        return unqueued;
    if (e.leftAlone())
        return e.item;
    return frameItem(e);
}

/* Kind 7: what share a car's two numbers make, in percent, and a count. */
reg32 drawPercent(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 part = guest.dword(e.car + kCarGood);
    const reg32 whole = part + guest.dword(e.car + kCarOther);
    // idiv, its dividend sign extended; the quotient's low half.
    const reg32 percent = whole != 0 ? reg32(std::int64_t(sreg32(part * 100)) / sreg32(whole)) : 100;
    return guest::direct<0x489900, frontend::hudr::drawPercentAndCount>(
        guest.app(), e.cpu, percent, guest.dword(e.car + kCarCount), guest.dword(kScaleSmall), e.colour, e.box(kBoxLeft),
        e.box(kBoxRight), e.top, e.alignment);
}

/* Kind 10: the place among the racers; in the first language with its letters,
 * for which it takes the large scale too. */
reg32 drawPlace(const Element& e)
{
    Guest& guest = e.guest;
    const bool plain = gameSettings(guest).language != 0;
    const reg32 left = e.box(kBoxLeft), right = e.box(kBoxRight);
    const reg32 small = guest.dword(kScaleSmall);
    if (plain)
    {
        const reg32 among = fe::direct<0x4cdee0, frontend::loopb::racerCount>(guest.app(), e.cpu);
        return guest::direct<0x48a320, frontend::hudr::drawPlaceAmong>(guest.app(), e.cpu, guest.dword(kPosition), among, small,
                                                                        e.colour, left, right, e.top, e.alignment);
    }
    const reg32 large = guest.dword(kScaleLarge);
    const reg32 among = fe::direct<0x4cdee0, frontend::loopb::racerCount>(guest.app(), e.cpu);
    const reg32 drawn = reg32(*fe::direct<0x48a3a0, port::hud::rankTextOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 7,
                                                                          guest.dword(kPosition), among, large, small, e.colour,
                                                                          left, right, e.top, e.alignment));
    return drawn;
}

/* The panel behind a list: its quad queued, on a card without alpha. */
void queueListPanel(const Element& e)
{
    Guest& guest = e.guest;
    if (e.cardWithAlpha())
        return;
    queueQuad(guest, kBackQueue, guest.dword(kBackQuads) + e.view * 4);
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.cpu, guest.dword(kBackGroup) + (e.view << 5));
}

/* The list's panel, its first and third corners, as a flat rectangle in the
 * colour without its alpha: for the element the editor's pointer is on. */
reg32 frameListPanel(const Element& e)
{
    constexpr reg32 kFirstX = 0, kFirstY = 4, kThirdX = 0x40, kThirdY = 0x44;
    Guest& guest = e.guest;
    if (e.leftAlone())
        return e.item;
    const reg32 corner = kBackVertices + ((guest.dword(kBackQuads) + e.view * 4) << 5);
    FloatMath math(guest);
    const reg32 bottom = reg32(math.truncated(math.load(corner + kThirdY)));
    const reg32 right = reg32(math.truncated(math.load(corner + kThirdX)));
    const reg32 top = reg32(math.truncated(math.load(corner + kFirstY)));
    const reg32 left = reg32(math.truncated(math.load(corner + kFirstX)));
    return *fe::direct<0x459350, graphics::flatRectangle>(guest.app(), e.cpu, left, top, right, bottom, e.colour & kColourNoAlpha);
}

/* Kind 13: the standings: its rectangle's element made again first, as they grow. */
reg32 drawStandings(const Element& e)
{
    Guest& guest = e.guest;
    // The view the game's code hands it in eax is not read.
    fe::direct<0x47ee80, frontend::rendb::prepareStandings>(guest.app(), e.cpu, e.car);
    guest::direct<0x4853c0, frontend::hudr::rebuildElement>(guest.app(), e.cpu, e.view, kStandings);
    queueListPanel(e);
    fe::direct<0x48ae70, port::hud::standingsOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 4, guest.dword(kScaleRows),
                                                 e.box(kBoxLeft), guest.dword(kListWidths + e.view * 4), e.view);
    return frameListPanel(e);
}

/* Kind 20: the list of labels. */
reg32 drawLabelRows(const Element& e)
{
    Guest& guest = e.guest;
    queueListPanel(e);
    fe::direct<0x48a660, port::hud::labelRowsOf>(guest.app(), e.cpu, e.cpu.esp - 4 - 4 * 4, e.view, guest.dword(kScaleRows),
                                                 e.box(kBoxLeft), e.box(kBoxRight), guest.dword(kListWidths + e.view * 4));
    return frameListPanel(e);
}

/* Kinds 17 and 22: the track's map and what is on it (its panel on a card without
 * alpha), then the mesh of its lines and the queues, the last under kStateShape on
 * a card with alpha. */
reg32 drawMap(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 view = e.view;
    fe::direct<0x48d4c0, port::hud::mapPointsOf>(guest.app(), e.cpu, e.cpu.esp - 4, view, e.car, 0, 0);
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.cpu, guest.dword(kMapGroup) + (view << 5));
    if (!e.cardWithAlpha())
    {
        const reg32 quad = guest.dword(kMapQuads) + view * 4;
        queueQuad(guest, kBackQueue, quad);
        flushQueue(guest, kBackQueue);
        unqueueQuad(guest, kBackQueue, quad);
    }
    guest::direct<0x48e770, frontend::hudr::flushLines>(guest.app(), e.cpu);
    flushQueue(guest, kDialQueue);
    if (e.cardWithAlpha())
        renderState(guest, e.cpu, kStateShape, 1);
    const reg32 flushed = flushQueue(guest, kOverQueue);
    if (!e.cardWithAlpha())
        return flushed;
    return renderState(guest, e.cpu, kStateShape, 0);
}

/* Kind 8: the fonts and the views' text flushed, the lights lit, the bands drawn. */
reg32 drawLights(const Element& e)
{
    Guest& guest = e.guest;
    fe::direct<0x4d1340, port::text::fontsDrawn>(guest.app(), e.cpu, e.cpu.esp - 4);
    guest::direct<0x4808b0, frontend::hudz::unqueueViewQuads>(guest.app(), e.cpu, 0);
    if (gameSettings(guest).playMode == 1)
        guest::direct<0x4808b0, frontend::hudz::unqueueViewQuads>(guest.app(), e.cpu, 1);
    guest.setDword(kLightsB, 0);
    guest.setDword(kLightsA, 0);
    fe::direct<0x48c7c0, render::startLightsDrawn>(guest.app(), e.cpu, 0xc, 5, 8, 3, 0);
    flushQueue(guest, kDialQueue);
    const reg32 band = *fe::direct<0x4800a0, render::hudBandBuilt>(guest.app(), e.cpu, 0);
    if (gameSettings(guest).playMode != 1)
        return band;
    return *fe::direct<0x4800a0, render::hudBandBuilt>(guest.app(), e.cpu, 1);
}

/* Kind 6: the grid in the colour, or none when the editor's pointer is elsewhere,
 * and its second pass under kStateShape on a card with alpha. */
reg32 drawGrid(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 result =
        guest::direct<0x482030, frontend::hudz::setGridHighlight>(guest.app(), e.cpu, 0, e.view, e.leftAlone() ? 0 : e.colour);
    if (!e.cardWithAlpha())
        return result;
    renderState(guest, e.cpu, kStateShape, 1);
    guest::direct<0x482190, frontend::hudz::drawGridHighlight>(guest.app(), e.cpu, e.view);
    return renderState(guest, e.cpu, kStateShape, 0);
}

/* An element's colour: white or grey, by its kind, when the editor's pointer is
 * elsewhere; on it, a pulse of the frame's counter. */
reg32 elementColour(Guest& guest, x86::CPU& cpu, reg32 item, reg32 kind)
{
    if (guest.dword(item + kItemLeftAlone) != 0)
        return fe::direct<0x47bc20, frontend::rendb::kindIsText>(guest.app(), cpu, kind) != 0 ? kColourGrey : kColourWhite;
    std::uint8_t pulse = std::uint8_t((guest.dword(kPulse) & 0x7f) * 2);
    if (pulse > 0x7f)
        pulse = std::uint8_t(0xff - pulse);
    const std::uint8_t rest = std::uint8_t(0x80 - pulse);
    const reg32 low = rest > 0x40 ? 0x40 : rest;
    return 0xff000000 | ((pulse + 0x80u) << 16) | (low << 8) | low;
}

}  // namespace

/* sub_45ab30: an element of the HUD drawn for its view: eax its item.  What it
 * hands back is what its last call left in eax, or what a kind that ends without
 * a call had there: the item, the kind. */
x86::reg32 frontend::hudr::drawElement(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x5c);
    constexpr std::uint32_t kLabelLineId = 0x5fd, kKeyLineId = 0x151;
    constexpr reg32 kKeyAlignment = 4;
    constexpr reg32 kKeyColour = 0x9ffafa00, kMeterColour = 0xff151f1f, kMarkersColour = 0x8f004f00;

    const std::int32_t number = std::int32_t(guest.dword(item + kItemKind));
    const reg32 view = number >= kKindsPerView ? 1 : 0;
    const reg32 kind = reg32(view != 0 ? number - kKindsPerView : number);
    const reg32 top = guest.dword(kBoxes + view * kBoxesPerView + kind * kRectangleBytes + kBoxTop);
    const reg32 alignment = guest.dword(kAlignments + view * kAlignmentsPerView + kind * 4);
    const reg32 car = cameraTarget(guest, cpu, view);
    const reg32 colour = elementColour(guest, cpu, item, kind);
    const Element e{ guest, cpu, item, view, kind, car, colour, top, alignment };
    const reg32 small = guest.dword(kScaleSmall);
    switch (kind)
    {
    case kSpeedFigures:
        return drawSpeedFigures(e);
    case kSpeedDial:
        return drawSpeedDial(e);
    case kRevFigures:
        return drawRevFigures(e);
    case kRevDial:
        return drawRevDial(e);
    case kLaps:
        return guest::direct<0x4899d0, drawLaps>(app, cpu, 0, gameSettings(guest).laps, small, colour, e.box(kBoxLeft),
                                                 e.box(kBoxRight), top, alignment);
    case kLapTimeText:
        return guest::direct<0x4898b0, drawTime>(app, cpu, guest.dword(kLapTime), small, colour, e.box(kBoxLeft),
                                                 e.box(kBoxRight), top, alignment);
    case kGrid:
        return drawGrid(e);
    case kPercentAndCount:
        return drawPercent(e);
    case kStartLights:
        return drawLights(e);
    case kKind9:
        guest::direct<0x484030, hudz::flushFrameQueue>(app, cpu);
        return e.leftAlone() ? item : frameItem(e);
    case kPlace:
        return drawPlace(e);
    case kTimeZero:
        return guest::direct<0x4898b0, drawTime>(app, cpu, 0, small, colour, e.box(kBoxLeft), e.box(kBoxRight), top,
                                                 alignment);
    case kLabelLine:
        return drawLanguageLine(guest, cpu, e.text({ 0 }, guest.dword(kScaleLarge)), kLabelLineId);
    case kStandings:
        return drawStandings(e);
    case kRatioFirst:
        return guest::direct<0x489a50, drawRatio>(app, cpu, guest.dword(car + kCarRatioA), gameSettings(guest).pursuitOutCount,
                                                  small, colour, e.box(kBoxLeft), e.box(kBoxRight), top, alignment);
    case kKeyLine:
    {
        Element line = e;
        line.colour = e.leftAlone() ? kKeyColour : colour;
        line.top = e.box(kBoxTop);
        line.alignment = kKeyAlignment;
        return drawLanguageLine(guest, cpu, line.text({ 0 }, small), kKeyLineId);
    }
    case kSegmentMeter:
    {
        const reg32 meterColour = e.leftAlone() ? kMeterColour : colour;
        const reg32 left = e.box(kBoxLeft), meterTop = e.box(kBoxTop);
        return *fe::direct<0x48c0e0, render::meterDrawn>(guest.app(), e.cpu, view, left, meterTop, e.box(kBoxRight) - left,
                                                         e.box(kBoxBottom) - meterTop, reg32(-1), meterColour);
    }
    case kMap:
    case kCopMap:
        return drawMap(e);
    case kMarkers:
    {
        const reg32 markerColour = e.leftAlone() ? kMarkersColour : colour;
        const reg32 set = reg32(*fe::direct<0x48b5f0, port::hud::markersOf>(app, cpu, cpu.esp - 4, view, car, car, markerColour));
        // edx, ebx and ecx as the call by address leaves them.
        return set;
    }
    case kRatioSecond:
        return guest::direct<0x489a50, drawRatio>(app, cpu, guest.dword(car + kCarRatioB), gameSettings(guest).pursuitWinCount,
                                                  small, colour, e.box(kBoxLeft), e.box(kBoxRight), top, alignment);
    case kLabelRows:
        return drawLabelRows(e);
    case kPanel:
        queueQuad(guest, kDialQueue, guest.dword(kPanelQuads) + view * 4);
        return flushQueue(guest, kDialQueue);
    default:
        return kind;
    }
}

namespace
{

/* ---------------------------------------------------------------------------
 * The map's pane: sub_48e920
 * ------------------------------------------------------------------------- */


}  // namespace

/* sub_48e920: the first view's map stepped on and, in a split screen, the second's
 * until both are of one style and either both or neither at the top of their
 * rectangles (the kind of map each has by its form). */
x86::reg32 frontend::hudr::stepMapPanes(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const reg32 stepped = reg32(*fe::direct<0x48e790, port::hud::paneStepOf>(app, cpu, cpu.esp - 4, 0u));
    if (gameSettings(guest).playMode != 1)
        return stepped;
    const reg32 firstForm = formOf(guest, cameraTarget(guest, cpu, 0));
    const reg32 secondForm = formOf(guest, cameraTarget(guest, cpu, 1));
    const HudRecord& first = gameSettings(guest).hudOf(1, firstForm);
    const HudRecord& second = gameSettings(guest).hudOf(2, secondForm);
    const HudRect& firstBox = first.rects[firstForm != 0 ? kCopMap : kMap];
    const HudRect& secondBox = second.rects[secondForm != 0 ? kCopMap : kMap];
    const auto atTop = [](const HudRect& box) -> reg32 { return (box.leftBits & 0x7fffffff) == 0 ? 1 : 0; };
    for (;;)
    {
        if (first.mapStyle == second.mapStyle)
        {
            const reg32 firstAtTop = atTop(firstBox);
            if (firstAtTop == atTop(secondBox))
                return firstAtTop;
        }
        fe::direct<0x48e790, port::hud::paneStepOf>(app, cpu, cpu.esp - 4, 1u);
    }
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's text
 * ------------------------------------------------------------------------- */

constexpr GuestString kLettersTh{ 0x53b7cc, "th" };
constexpr GuestString kLettersSt{ 0x53b7d0, "st" };
constexpr GuestString kLettersNd{ 0x53b7d4, "nd" };
constexpr GuestString kLettersRd{ 0x53b7d8, "rd" };
constexpr GuestString kLettersStrings[] = { kLettersTh, kLettersSt, kLettersNd, kLettersRd };

}  // namespace

/* sub_4896f0: the letters after a place: "st", "nd", "rd", or "th" for 4 to 20
 * and all the rest. */
x86::reg32 frontend::hudr::ordinalLetters(App*, CPU&, reg32 number)
{
    const std::int32_t place = std::int32_t(number);
    if (place > 3 && place < 21)
        return kLettersTh.address;
    switch (place % 10)
    {
    case 1:
        return kLettersSt.address;
    case 2:
        return kLettersNd.address;
    case 3:
        return kLettersRd.address;
    default:
        return kLettersTh.address;
    }
}

namespace
{

/* A time's formats, by language. */
constexpr reg32 kTimeFormats = 0x55d6cc;

}  // namespace

/* sub_489750: a time in 64ths of a second, written without its sign into `buffer`
 * as minutes, seconds and hundredths, in the language's format.  edx is left 0:
 * the sign of the hundredths, which have none. */
x86::reg32 frontend::hudr::formatTime(App* app, CPU& cpu, reg32 buffer, reg32 timeWord)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 4);
    const std::int32_t time = std::int32_t(timeWord);
    constexpr std::int32_t kTicksPerSecond = 64, kSecondsPerMinute = 60;
    const std::int32_t magnitude = time > 0 ? time : std::int32_t(reg32(0) - reg32(time));
    const std::int32_t seconds = magnitude / kTicksPerSecond;
    const std::int32_t hundredths = (magnitude % kTicksPerSecond) * 100 / kTicksPerSecond;
    const reg32 format = guest.dword(kTimeFormats + gameSettings(guest).language * 4);
    const reg32 written = fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, format, guest::Words{reg32(seconds / kSecondsPerMinute), reg32(seconds % kSecondsPerMinute), reg32(hundredths)});
    return written;
}

namespace
{

constexpr reg32 kTextBufferBytes = 0x100;

/* What sub_4897f0 takes on the stack, and what the functions over it pass on. */
struct TextPlace
{
    reg32 scale, colour, left, right, top, how;
};

TextPlace textPlaceOf(win32::WinApplication* app, const x86::CPU& cpu)
{
    return { stackArgument(app, cpu, 0), stackArgument(app, cpu, 1), stackArgument(app, cpu, 2),
             stackArgument(app, cpu, 3), stackArgument(app, cpu, 4), stackArgument(app, cpu, 5) };
}

reg32 drawPlaced(Guest& guest, reg32 buffer, const TextPlace& place)
{
    return alignedText(guest, buffer, place.scale, place.colour, place.left, place.right, place.top, place.how);
}

}  // namespace

/* sub_4898b0: a time at its place. */
x86::reg32 frontend::hudr::drawTime(App* app, CPU& cpu, reg32 time, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 how)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const reg32 buffer = guest.frame();
    guest::direct<0x489750, formatTime>(app, cpu, buffer, time);
    return drawPlaced(guest, buffer, place);
}

namespace
{

constexpr GuestString kPercentFormat{ 0x53b7ec, "%d%% %s" };
constexpr GuestString kCountFormat{ 0x53b7f4, "%d %s" };
constexpr GuestString kPercentStrings[] = { kPercentFormat, kCountFormat };
constexpr std::uint32_t kPercentLine = 0x152, kCountOfOneLine = 0x153, kCountLine = 0x154;
constexpr reg32 kPercentSign = '%';

}  // namespace

/* sub_489900: "85% ..." at its place and, a percent sign's height below it, how
 * many: one has a line of its own. */
x86::reg32 frontend::hudr::drawPercentAndCount(App* app, CPU& cpu, reg32 percent, reg32 count, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 how)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const reg32 buffer = guest.frame();
    const reg32 line = languageLine(guest, cpu, kPercentLine);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kPercentFormat.address, guest::Words{percent, line});
    drawPlaced(guest, buffer, place);
    if (count == 1)
    {
        const reg32 one = languageLine(guest, cpu, kCountOfOneLine);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, one, guest::Words({}));
    }
    else
    {
        const reg32 many = languageLine(guest, cpu, kCountLine);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kCountFormat.address, guest::Words{count, many});
    }
    TextPlace below = place;
    below.top = place.top + fe::direct<0x4d1540, port::text::characterHeight>(guest.app(), guest.cpu(), kPercentSign);
    const reg32 drawn = drawPlaced(guest, buffer, below);
    return drawn;
}

namespace
{

constexpr GuestString kLapsWordFirst{ 0x53b7fc, "%s %d/%d" };
constexpr GuestString kLapsWordLast{ 0x53b808, "%d/%d %s" };
constexpr GuestString kLapsStrings[] = { kLapsWordFirst, kLapsWordLast };
constexpr std::uint32_t kLapsLine = 0x155;
constexpr reg32 kLanguageWordFirst = 2;

}  // namespace

/* sub_4899d0: a count of a total with its word, first in one language and last
 * in the rest. */
x86::reg32 frontend::hudr::drawLaps(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 how)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const reg32 buffer = guest.frame();
    const reg32 line = languageLine(guest, cpu, kLapsLine);
    if (gameSettings(guest).language == kLanguageWordFirst)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kLapsWordFirst.address, guest::Words{line, done, total});
    else
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kLapsWordLast.address, guest::Words{done, total, line});
    const reg32 drawn = drawPlaced(guest, buffer, place);
    return drawn;
}

namespace
{

constexpr GuestString kRatioFormat{ 0x53b814, "%s: %d/%d" };
constexpr GuestString kRatioStrings[] = { kRatioFormat };
constexpr std::uint32_t kRatioLine = 0x14d;

}  // namespace

/* sub_489a50: a line of the language's with a count of a total. */
x86::reg32 frontend::hudr::drawRatio(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 how)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const reg32 buffer = guest.frame();
    const reg32 line = languageLine(guest, cpu, kRatioLine);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kRatioFormat.address, guest::Words{ line, done, total });
    const reg32 drawn = drawPlaced(guest, buffer, place);
    return drawn;
}

namespace
{

constexpr GuestString kPlaceFormatPos{ 0x53b890, "Pos. %d/%d" };
constexpr GuestString kPlaceFormatOf{ 0x53b89c, "%d\xba de %d" };
constexpr GuestString kPlaceFormat{ 0x53b8a8, "%d/%d" };
constexpr GuestString kPlaceStrings[] = { kPlaceFormatPos, kPlaceFormatOf, kPlaceFormat };
constexpr reg32 kLanguagePos = 2, kLanguageOf = 3;

}  // namespace

/* sub_48a320: a place among so many, as the language writes it. */
x86::reg32 frontend::hudr::drawPlaceAmong(App* app, CPU& cpu, reg32 place, reg32 among, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 how)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + kTextBufferBytes);
    const TextPlace at{ scale, colour, left, right, top, how };
    const reg32 buffer = guest.frame();
    const reg32 language = gameSettings(guest).language;
    const reg32 format = language == kLanguagePos ? kPlaceFormatPos.address
        : language == kLanguageOf ? kPlaceFormatOf.address : kPlaceFormat.address;
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, format, guest::Words{ place, among });
    const reg32 drawn = drawPlaced(guest, buffer, at);
    return drawn;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's lines
 * ------------------------------------------------------------------------- */

// The bounds a line's vertex is clipped by.
constexpr reg32 kClipDepth = 0x7d34f8;
constexpr reg32 kClipLeft = 0x7d34fc;
constexpr reg32 kClipBottom = 0x7d3500;
constexpr reg32 kClipTop = 0x7d3504;
constexpr reg32 kClipRight = 0x7d350c;
constexpr std::uint8_t kPastLeft = 1, kPastRight = 2, kPastBottom = 4, kPastTop = 8, kPastDepth = 0x10;

// A vertex on the line layer: x and y as integers.
constexpr reg32 kVertexBytes = 0x20;
constexpr reg32 kVertexX = 0, kVertexY = 4, kVertexDepth = 0xc, kVertexColour = 0x10, kVertexClip = 0x14;
constexpr reg32 kBlankVerticesBytes = 3 * kVertexBytes;
constexpr reg32 kTriangleBlanks = 0x489630;    // what the triangle's three vertices start from
constexpr reg32 kLineBlanks = 0x489690;        // the line's two, and an empty one
constexpr reg32 kStateTexture = 1;              // set to none

/* A vertex's clip code.  A negative number is past its low bound whatever the
 * bound. */
std::uint8_t clipCode(const Guest& guest, reg32 vertex)
{
    const auto before = [&](reg32 value, reg32 bound) { return sreg32(value) < 0 || sreg32(value) < sreg32(guest.dword(bound)); };
    const reg32 depth = guest.dword(vertex + kVertexDepth);
    if (sreg32(depth) < 0 || sreg32(depth) >= sreg32(guest.dword(kClipDepth)))
        return kPastDepth;
    std::uint8_t code = 0;
    const reg32 y = guest.dword(vertex + kVertexY);
    if (before(y, kClipTop))
        code |= kPastTop;
    else if (sreg32(y) > sreg32(guest.dword(kClipBottom)))
        code |= kPastBottom;
    const reg32 x = guest.dword(vertex + kVertexX);
    if (before(x, kClipLeft))
        code |= kPastLeft;
    else if (sreg32(x) > sreg32(guest.dword(kClipRight)))
        code |= kPastRight;
    return code;
}

/* The frame's vertices from the game's blank ones, the points' x and y and the
 * colour on them, and their clip codes. */
void placeVertices(Guest& guest, reg32 blanks, std::initializer_list<reg32> coordinates, reg32 colour)
{
    const reg32 frame = guest.frame();
    for (reg32 at = 0; at < kBlankVerticesBytes; at += 4)
        guest.setDword(frame + at, guest.dword(blanks + at));
    reg32 vertex = frame;
    for (const reg32* coordinate = coordinates.begin(); coordinate != coordinates.end(); coordinate += 2, vertex += kVertexBytes)
    {
        guest.setDword(vertex + kVertexX, coordinate[0]);
        guest.setDword(vertex + kVertexY, coordinate[1]);
        guest.setDword(vertex + kVertexColour, colour);
    }
    for (reg32 placed = frame; placed != vertex; placed += kVertexBytes)
        guest.setByte(placed + kVertexClip, clipCode(guest, placed));
}

}  // namespace

/* sub_48b2a0: a triangle in a colour, filled or as three lines. */
x86::reg32 frontend::hudr::drawTriangle(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 filled, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x60);
    placeVertices(guest, kTriangleBlanks, { x0, y0, x1, y1, x2, y2 }, colour);
    const reg32 first = guest.frame(), second = first + kVertexBytes, third = second + kVertexBytes;
    renderState(guest, cpu, kStateTexture, 0);
    if (filled != 0)
        return fe::direct<0x4c11b0, vertices::triangleClipped>(guest.app(), guest.cpu(), first, second, third);
    fe::direct<0x4c0b20, render::lineDrawn>(app, cpu, first, second);
    fe::direct<0x4c0b20, render::lineDrawn>(app, cpu, second, third);
    return *fe::direct<0x4c0b20, render::lineDrawn>(app, cpu, third, first);
}

/* sub_48b4a0: a line in a colour. */
x86::reg32 frontend::hudr::drawLine(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x60);
    placeVertices(guest, kLineBlanks, { x0, y0, x1, y1 }, colour);
    const reg32 first = guest.frame();
    renderState(guest, cpu, kStateTexture, 0);
    return *fe::direct<0x4c0b20, render::lineDrawn>(app, cpu, first, first + kVertexBytes);
}

namespace
{

constexpr reg32 kLineMesh = 0x793108;

}

/* sub_48e770: the HUD's line mesh flushed. */
x86::reg32 frontend::hudr::flushLines(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return *fe::direct<0x49bf60, graphics::lineMeshFlushed>(app, cpu, kLineMesh);
}

/* sub_48e780: the HUD's line mesh made ready. */
x86::reg32 frontend::hudr::prepareLines(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest::direct<0x49bef0, hudx::resetSurface>(app, cpu, kLineMesh);
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

x86::reg32 frontend::hudDrawElement(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45ab30", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::drawElement(app, cpu, item); });
}

x86::reg32 frontend::hudRebuildElement(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4853c0", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 view = args.eax, element = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::rebuildElement(app, cpu, view, element); });
}

x86::reg32 frontend::hudStepMapPanes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48e920", 0, 0, nullptr, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::stepMapPanes(app, cpu); });
}

x86::reg32 frontend::hudOrdinalLetters(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4896f0", 0, 0, kLettersStrings, std::size(kLettersStrings), kKeepsEcxEdx };
    const reg32 place = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::ordinalLetters(app, cpu, place); });
}

x86::reg32 frontend::hudFormatTime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_489750", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 buffer = args.eax, time = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::formatTime(app, cpu, buffer, time); });
}

x86::reg32 frontend::hudDrawTime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4898b0", 0x18, 0, nullptr, 0 };
    const reg32 time = args.eax;
    const TextPlace p = textPlaceOf(app, cpu);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return hudr::drawTime(app, cpu, time, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

x86::reg32 frontend::hudDrawPercentAndCount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_489900", 0x18, 0, kPercentStrings, std::size(kPercentStrings), kKeepsEbxEcxEsiEdi };
    const reg32 percent = args.eax, count = args.edx;
    const TextPlace p = textPlaceOf(app, cpu);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return hudr::drawPercentAndCount(app, cpu, percent, count, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

x86::reg32 frontend::hudDrawLaps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4899d0", 0x18, 0, kLapsStrings, std::size(kLapsStrings), kKeepsEcxEsiEdi };
    const reg32 done = args.eax, total = args.edx;
    const TextPlace p = textPlaceOf(app, cpu);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return hudr::drawLaps(app, cpu, done, total, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

x86::reg32 frontend::hudDrawRatio(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_489a50", 0x18, 0, kRatioStrings, std::size(kRatioStrings), kKeepsEbxEcxEsiEdi };
    const reg32 done = args.eax, total = args.edx;
    const TextPlace p = textPlaceOf(app, cpu);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return hudr::drawRatio(app, cpu, done, total, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

x86::reg32 frontend::hudDrawPlaceAmong(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48a320", 0x18, 0, kPlaceStrings, std::size(kPlaceStrings), kKeepsEcxEsiEdi };
    const reg32 placeNumber = args.eax, among = args.edx;
    const TextPlace p = textPlaceOf(app, cpu);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return hudr::drawPlaceAmong(app, cpu, placeNumber, among, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

x86::reg32 frontend::hudDrawTriangle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48b2a0", 0x20, 0, nullptr, 0 };
    const reg32 x0 = stackArgument(app, cpu, 0), y0 = stackArgument(app, cpu, 1), x1 = stackArgument(app, cpu, 2);
    const reg32 y1 = stackArgument(app, cpu, 3), x2 = stackArgument(app, cpu, 4), y2 = stackArgument(app, cpu, 5);
    const reg32 filled = stackArgument(app, cpu, 6), colour = stackArgument(app, cpu, 7);
    return fe::runThunk(app, cpu, args, site,
                 [&](Guest&) { return hudr::drawTriangle(app, cpu, x0, y0, x1, y1, x2, y2, filled, colour); });
}

x86::reg32 frontend::hudDrawLine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48b4a0", 0x14, 0, nullptr, 0 };
    const reg32 x0 = stackArgument(app, cpu, 0), y0 = stackArgument(app, cpu, 1), x1 = stackArgument(app, cpu, 2);
    const reg32 y1 = stackArgument(app, cpu, 3), colour = stackArgument(app, cpu, 4);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::drawLine(app, cpu, x0, y0, x1, y1, colour); });
}

x86::reg32 frontend::hudFlushLines(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48e770", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::flushLines(app, cpu); });
}

x86::reg32 frontend::hudPrepareLines(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_48e780", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return hudr::prepareLines(app, cpu); });
}

}
