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
nfs::u32 hudDrawElement(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudRebuildElement(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudStepMapPanes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudOrdinalLetters(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudFormatTime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawTime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawPercentAndCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawLaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawRatio(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawPlaceAmong(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudFlushLines(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudPrepareLines(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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
using nfs::Float;
using nfs::u32;
using nfs::s32;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline u32 callAt(Guest& guest, u32 function, const fe::Arguments& arguments = fe::Arguments(), u32 esi = 0,
                    double* floatResult = nullptr)
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<u32> result = guest::pointerCall(guest.app(), ts, function, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

/* fild, fld and sub_4dfd56 with fistp (fe::FloatMath's) as plain values: no x87 behind them. */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}
    static Float integer(std::int32_t value) { return Float(s32(value)); }
    Float load(u32 address) const { return Float(m_guest.floatAt(address)); }
    std::int32_t truncated(const Float& value)
    {
        const double chopped = guest::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<s32>(chopped);  // fistp
    }

private:
    Guest& m_guest;
};

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
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
constexpr u32 kSprintf = 0x4df690;

// The numbers the elements show.
constexpr u32 kLapTime = 0x72522c;
constexpr u32 kPosition = 0x725234;

// A car, as sub_422bd0 gives it.
constexpr u32 kCarFlags = 0x200;             // byte; kCarIsCop: the cop's form of the HUD
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr u32 kCarRatioA = 0x2c8;
constexpr u32 kCarRatioB = 0x2cc;
constexpr u32 kCarCount = 0x2d8;             // the count under a percentage
constexpr u32 kCarGood = 0x2dc;              // the percentage's part
constexpr u32 kCarOther = 0x2e0;             // and the rest of its whole
constexpr u32 kCarGear = 0x50b;              // byte
constexpr u32 kCarData = 0x528;              // its numbers: kDataGears, kDataRevLimit
constexpr u32 kCarRevs = 0x590;
constexpr u32 kDataGears = 8;
constexpr u32 kDataRevLimit = 0x1c4;

// The HUD's records (native_frontend_hudedit.cpp), by seat and by form.
constexpr u32 kRecordsPerSeat = 0x348;
constexpr u32 kRecordsPerForm = 0x1a4;
constexpr u32 kRectangleBytes = 16;

// The kinds of element, a rectangle each, by what they draw.
enum Kind : u32
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
u32 cameraTarget(Guest& guest, nfs::ThreadState& ts, u32 view)
{
    return fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), ts, view);
}

/* A render state set (sub_431900, as C++): its answer, which it leaves in eax. */
u32 renderState(Guest& guest, nfs::ThreadState& ts, u32 state, u32 value)
{
    return *fe::direct<0x431900, graphics::stateAnswer>(guest.app(), ts, state, value);
}

/* Line `number` of the language's text (sub_4d1850, as C++; edx 0: only a name past
 * the lines reads it). */
u32 languageLine(Guest& guest, nfs::ThreadState& ts, std::uint32_t number)
{
    return u32(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), ts, ts.sp - 4, std::int32_t(number), 0));
}

/* Whether a car is a cop's, as 0 or 1: the form of the HUD its view has. */
u32 formOf(const Guest& guest, u32 car)
{
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

/* The queue's quads drawn: what the code leaves in eax. */
u32 flushQueue(Guest& guest, u32 queue)
{
    return u32(*fe::direct<0x49be40, port::text::quadsFlushed>(guest.app(), guest.ts(), guest.ts().sp - 4, queue));
}

/* The queue's quad of the four corners from `first`. */
u32 queueQuad(Guest& guest, u32 queue, u32 first)
{
    return u32(*fe::direct<0x49bd30, port::text::quadQueued>(guest.app(), guest.ts(), guest.ts().sp - 8, queue, first, first + 1, first + 2, first + 3));
}

u32 unqueueQuad(Guest& guest, u32 queue, u32 first)
{
    return u32(*fe::direct<0x49bdc0, port::text::quadUnqueued>(guest.app(), guest.ts(), guest.ts().sp - 8, queue, first, first + 1, first + 2, first + 3));
}

/* ---------------------------------------------------------------------------
 * An element made again: sub_4853c0
 * ------------------------------------------------------------------------- */

constexpr u32 kViewWidth = 0x7cdac8;
constexpr u32 kViewHeight = 0x7cdacc;
constexpr u32 kBuildMapViews = 0x48d110;     // eax a view, edx and ebx its size

/* The start lights stand in one place on both players' screens: the rectangle
 * just set, by seat 1 or seat 2, copied to the other's and made again there. */
void shareStartLights(Guest& guest, nfs::ThreadState& ts, u32 seat, u32 form, u32 width, u32 height)
{
    if (seat != 1 && seat != 2)
        return;
    const u32 otherSeat = 3 - seat;
    const u32 otherView = seat == 1 ? 1 : 0;
    const u32 otherForm = formOf(guest, cameraTarget(guest, ts, otherView));
    gameSettings(guest).hudOf(otherSeat, otherForm).rects[kStartLights] = gameSettings(guest).hudOf(seat, form).rects[kStartLights];
    fe::direct<0x480910, render::hudRectPlaced>(guest.app(), ts, otherView, kStartLights, width, height);
    guest::direct<0x480a00, frontend::hudz::setAlignments>(guest.app(), ts, otherView);
}

}  // namespace

/* sub_4853c0: one element of a view's HUD made again for the rectangle it now
 * has: eax the view, edx the element.  A dial's band comes with the forms that
 * have one; a kind with nothing of its own goes without. */
nfs::u32 frontend::hudr::rebuildElement(App* app, ThreadState& ts, u32 view, u32 element)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x18);
    const u32 width = guest.dword(kViewWidth), height = guest.dword(kViewHeight);
    const u32 seat = view + (gameSettings(guest).playMode == 1 ? 1 : 0);
    const u32 form = formOf(guest, cameraTarget(guest, ts, view));
    if (element == kStartLights)
        shareStartLights(guest, ts, seat, form, width, height);
    fe::direct<0x480910, render::hudRectPlaced>(app, ts, view, element, width, height);
    const u32 placed = guest::direct<0x480a00, hudz::setAlignments>(app, ts, view);
    const u32 seatBytes = seat * kRecordsPerSeat, formBytes = form * kRecordsPerForm;
    const u32 recordBytes = seatBytes + formBytes;
    switch (element)
    {
    case kSpeedFigures:
        return *fe::direct<0x480fc0, render::hudPicturePlaced>(app, ts, view);
    case kSpeedDial:
    {
        fe::direct<0x480fc0, render::hudPicturePlaced>(app, ts, view);
        const u32 shown = gameSettings(guest).hudAt(recordBytes).speedometerForm;
        if (shown == 4 || shown == 2)
            return *fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, view);
        return recordBytes;
    }
    case kRevFigures:
        return *fe::direct<0x4812e0, render::hudDialPlaced>(app, ts, view);
    case kRevDial:
    {
        fe::direct<0x4812e0, render::hudDialPlaced>(app, ts, view);
        const u32 shown = gameSettings(guest).hudAt(recordBytes).tachometerForm;
        if (shown == 2 || shown == 3)
            return *fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, view);
        return recordBytes;
    }
    case kGrid:
        return guest::direct<0x4818b0, hudz::layGrid>(app, ts, view);
    case kStartLights:
        return guest::direct<0x4815d0, hudz::layReplayBar>(app, ts, view);
    case kKind9:
        fe::direct<0x4be5e0, loopb::viewsLayout>(app, ts, form);
        return fe::direct<0x47fd80, rendb::fitFrameToScreen>(app, ts);
    case kStandings:
    case kLabelRows:
        return *fe::direct<0x481c50, render::hudTablePlaced>(app, ts, view);
    case kSegmentMeter:
        return guest::direct<0x481570, hudz::layNothing>(app, ts, view);
    case kMap:
    case kCopMap:
        // By address: the function goes out with the edx sub_48d110 leaves.
        callAt(guest, kBuildMapViews, args({ view, width, height }));
        return *fe::direct<0x481aa0, render::hudPanelPlaced>(app, ts, view);
    case kMarkers:
        return guest::direct<0x481290, hudz::layMarkersFrame>(app, ts, view, width);
    case kPanel:
        return guest::direct<0x481510, hudz::layPanel>(app, ts, view);
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
constexpr u32 kItemKind = 0x3c;              // its kind; the second view's from kKindsPerView on
constexpr u32 kItemLeftAlone = 0x70;         // nonzero: the editor's pointer is not on it
constexpr std::int32_t kKindsPerView = 23;

// The rectangles the elements are drawn in, by view and kind.
constexpr u32 kBoxes = 0x749758;
constexpr u32 kBoxesPerView = 0x170;
constexpr u32 kBoxLeft = 0, kBoxTop = 4, kBoxRight = 8, kBoxBottom = 0xc;
// How each kind's text is aligned, by view and kind.
constexpr u32 kAlignments = 0x725690;
constexpr u32 kAlignmentsPerView = 0x5c;

constexpr u32 kPulse = 0x79c1a4;             // counts on, frame by frame
constexpr u32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kCardWithAlpha = 2;
constexpr u32 kStateShape = 0x2bf;           // set around what is drawn apart on a card with alpha

constexpr u32 kColourGrey = 0xffa0a0a0;
constexpr u32 kColourWhite = 0xffffffff;
constexpr u32 kColourRow = 0xff707070;
constexpr u32 kColourNoAlpha = 0x7fffffff;

// The text's scales.
constexpr u32 kScaleSmall = 0x791b3c;
constexpr u32 kScaleRows = 0x791b4c;
constexpr u32 kScaleLabel = 0x791b50;
constexpr u32 kScaleLarge = 0x791b54;
constexpr u32 kFullScaleSpeed = 0x42b2d1ec;  // a float, 89.41: the speed at which a speedometer is full

// The vertex groups and queues.
constexpr u32 kBackQueue = 0x725748;         // the panels behind the instruments
constexpr u32 kBackVertices = 0x725758;      // their corners, 0x20 each: x and y first
constexpr u32 kDialQueue = 0x749a38;
constexpr u32 kOverQueue = 0x76da48;
constexpr u32 kBackQuads = 0x791adc;         // the first of the back queue's corners
constexpr u32 kBackGroup = 0x791ad8;
constexpr u32 kSpeedGroup = 0x791ac4;
constexpr u32 kSpeedQuads = 0x791ad0;
constexpr u32 kSpeedVertices = 0x791b38;
constexpr u32 kRevGroup = 0x791b30;
constexpr u32 kRevQuads = 0x791b24;
constexpr u32 kRevVertices = 0x791b2c;
constexpr u32 kMapGroup = 0x791b48;
constexpr u32 kMapQuads = 0x791b20;
constexpr u32 kPanelQuads = 0x791acc;
constexpr u32 kLightsA = 0x749a40;
constexpr u32 kLightsB = 0x749a44;

constexpr u32 kListWidths = 0x724780;        // by view

// The port's functions the elements end in.

/* What sub_45ab30 works out for every kind. */
struct Element
{
    Guest& guest;
    nfs::ThreadState& ts;
    u32 item;
    u32 view;       // 0 or 1
    u32 kind;
    u32 car;        // the car the view looks at
    u32 colour;
    u32 top;        // the box's top and the text's alignment, as they were at the start
    u32 alignment;

    u32 box(u32 field) const { return guest.dword(kBoxes + view * kBoxesPerView + kind * kRectangleBytes + field); }
    bool leftAlone() const { return guest.dword(item + kItemLeftAlone) != 0; }
    bool cardWithAlpha() const { return (guest.byte(kDisplayFlags) & kCardWithAlpha) != 0; }

    /* A text's arguments: its scale, colour, box and alignment on the stack. */
    fe::Arguments text(std::initializer_list<u32> registers, u32 scale) const
    {
        const u32 left = box(kBoxLeft), right = box(kBoxRight);
        return args(registers, { scale, colour, left, right, top, alignment });
    }
};

/* The HUD's text drawn aligned (port_hud's), as C++: what it gives back in eax. */
u32 alignedText(Guest& guest, u32 text, u32 scale, u32 colour, u32 x, u32 dx, u32 y, u32 mode)
{
    return u32(*fe::direct<0x4897f0, port::hud::textAlignedOf>(guest.app(), guest.ts(), guest.ts().sp - 4 - 4 * 6, text,
                                                                 scale, colour, x, dx, y, mode));
}

/* A line of the language's text at its place, the box read before the line. */
u32 drawLanguageLine(Guest& guest, nfs::ThreadState& ts, fe::Arguments place, std::uint32_t line)
{
    const u32 text = languageLine(guest, ts, line);
    return alignedText(guest, text, place.stack[0], place.stack[1], place.stack[2], place.stack[3], place.stack[4],
                       place.stack[5]);
}

/* The gears' list down a speedometer's side: the chosen one and the last in
 * colours of their own when the editor's pointer is elsewhere. */
u32 drawGearRows(const Element& e, u32 left, u32 right, u32 bottom)
{
    Guest& guest = e.guest;
    const bool leftAlone = e.leftAlone();
    const u32 scale = guest.dword(kScaleLarge);
    const u32 gear = guest.byte(e.car + kCarGear);
    const u32 gears = guest.dword(guest.dword(e.car + kCarData) + kDataGears) - 2;
    const u32 drawn = leftAlone
        ? u32(*fe::direct<0x489b30, port::hud::listRowsOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 7, gears, gear, 0u, scale,
                                                             kColourRow, kColourGrey, 0u, left, right, bottom))
        : u32(*fe::direct<0x489b30, port::hud::listRowsOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 7, gears, gear, 0u, scale,
                                                             e.colour, e.colour, e.colour, left, right, bottom));
    // edx and ebx as the call by address leaves them.
    return drawn;
}

/* The frame around the element's item, for the one the editor's pointer is on. */
u32 frameItem(const Element& e)
{
    return fe::direct<0x459410, frontend::loopa::itemFrame>(e.guest.app(), e.ts, e.item, e.colour & kColourNoAlpha);
}

/* Kind 0: the speed in figures, and the gears' rows in the box's right part. */
u32 drawSpeedFigures(const Element& e)
{
    constexpr u32 kRowsShare = 0x539bfc;     // a double, 0.75
    Guest& guest = e.guest;
    const u32 seat = (gameSettings(guest).playMode == 1 ? 1 : 0) + e.view;
    const u32 form = gameSettings(guest).hudOf(seat, formOf(guest, e.car)).speedometerForm;
    const u32 firstUnits = form == 1 || form == 2 ? 1 : 0;
    fe::direct<0x489d70, port::hud::percentTextOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 6, kFullScaleSpeed, firstUnits,
                                                   guest.dword(kScaleLabel), e.colour, e.box(kBoxRight), e.top);
    FloatMath math(guest);
    const Float left = FloatMath::integer(std::int32_t(e.box(kBoxLeft)));
    const Float right = FloatMath::integer(std::int32_t(e.box(kBoxRight)));
    const float rightStored = float(right);
    const float rows = float(fp::mul(fp::sub(right, left), doubleAt(guest, kRowsShare)));
    const float rowsLeft = float(fp::sub(Float(rightStored), Float(rows)));
    const u32 rowsRight = u32(math.truncated(Float(rightStored)));
    const u32 rowsStart = u32(math.truncated(Float(rowsLeft)));
    return drawGearRows(e, rowsStart, rowsRight, e.box(kBoxBottom));
}

/* Kind 1: the speedometer's dial, its needle and the gears' rows. */
u32 drawSpeedDial(const Element& e)
{
    Guest& guest = e.guest;
    const u32 view = e.view;
    const u32 seat = view + (gameSettings(guest).playMode == 1 ? 1 : 0);
    const u32 form = formOf(guest, e.car);
    fe::direct<0x48a220, port::hud::indicatorGeometryOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 2, view, kFullScaleSpeed,
                                                         e.colour);
    guest::direct<0x481170, frontend::hudz::placeDialArt>(guest.app(), e.ts, seat, form,
                                                         guest.dword(kSpeedVertices) + (view << 8));
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.ts, guest.dword(kSpeedGroup) + (view << 5));
    const u32 quads = guest.dword(kSpeedQuads) + view * 8;
    queueQuad(guest, kDialQueue, quads);
    queueQuad(guest, kDialQueue, quads + 4);
    drawGearRows(e, e.box(kBoxLeft), e.box(kBoxRight), e.box(kBoxBottom));
    const u32 flushed = flushQueue(guest, kDialQueue);
    if (!e.cardWithAlpha())
        return flushed;
    if (e.leftAlone())
        return e.item;
    unqueueQuad(guest, kDialQueue, quads);
    return frameItem(e);
}

/* Kind 2: the revs in figures. */
u32 drawRevFigures(const Element& e)
{
    Guest& guest = e.guest;
    return u32(*fe::direct<0x489c40, port::hud::timeTextOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 5,
                                                              guest.dword(e.car + kCarRevs), guest.dword(kScaleLabel),
                                                              guest.dword(kScaleLarge), e.colour, e.box(kBoxRight), e.top));
}

/* Kind 3: the tachometer: its corners' texture coordinates for the size of its
 * art, half a texel in, and moved across to this view's part of it; its centre
 * and scales; its quad and its needle. */
u32 drawRevDial(const Element& e)
{
    constexpr u32 kArtWidth = 0x791ae8, kArtHeight = 0x791aea;  // words
    constexpr u32 kDoubled = 0x539c04;       // a float, 2.0
    constexpr u32 kArtExtent = 0x539c08;     // a float, 64.0
    constexpr u32 kFirstViewShift = 0x539c0c, kSecondViewShift = 0x539bfc;  // doubles, 0.5 and 0.75
    constexpr u32 kHalf = 0x539c14;          // a float
    constexpr u32 kToTextScale = 0x539c18, kToNeedleScale = 0x539c1c;  // floats, 4.0 and 21.33
    constexpr u32 kDialScale = 0x724760;     // by view
    constexpr u32 kNeedleScale = 0x724758;   // by view
    constexpr u32 kTextScale = 0x724778;     // by view
    constexpr u32 kDialCentre = 0x724768;    // by view, x and y
    constexpr u32 kViewVertices = 0xe0;
    constexpr u32 kU0 = 0x18, kV0 = 0x1c, kU1 = 0x38, kV1 = 0x3c, kU2 = 0x58, kV2 = 0x5c, kU3 = 0x78, kV3 = 0x7c;
    constexpr u32 kFirstX = 0, kFirstY = 4, kThirdX = 0x40, kThirdY = 0x44;

    Guest& guest = e.guest;
    const u32 view = e.view;
    const u32 corners = guest.dword(kRevVertices) + view * kViewVertices;

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
    for (const u32 u : { kU0, kU1, kU2, kU3 })
        guest.setFloat(corners + u, float(fp::add(Float(guest.floatAt(corners + u)), shift)));

    const Float scale(guest.floatAt(kDialScale + view * 4));
    const Float half(guest.floatAt(kHalf));
    const Float sumX = fp::add(Float(guest.floatAt(corners + kFirstX)), Float(guest.floatAt(corners + kThirdX)));
    const Float sumY = fp::add(Float(guest.floatAt(corners + kFirstY)), Float(guest.floatAt(corners + kThirdY)));
    guest.setFloat(kDialCentre + view * 8, float(fp::mul(sumX, half)));
    guest.setFloat(kDialCentre + view * 8 + 4, float(fp::mul(half, sumY)));
    guest.setFloat(kTextScale + view * 4, float(fp::mul(scale, Float(guest.floatAt(kToTextScale)))));
    guest.setFloat(kNeedleScale + view * 4, float(fp::mul(scale, Float(guest.floatAt(kToNeedleScale)))));

    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.ts, guest.dword(kRevGroup) + (view << 5));
    const u32 quad = guest.dword(kRevQuads) + view * 7;
    queueQuad(guest, kDialQueue, quad);
    const u32 limit = guest.dword(guest.dword(e.car + kCarData) + kDataRevLimit);
    const u32 revs = guest.dword(e.car + kCarRevs);
    fe::direct<0x489e00, port::hud::indicatorQuadOf>(guest.app(), e.ts, e.ts.sp - 4, view, revs, limit, e.colour);
    // edx, ebx and ecx as the call by address leaves them.
    flushQueue(guest, kDialQueue);
    const u32 unqueued = unqueueQuad(guest, kDialQueue, quad);
    if (!e.cardWithAlpha())
        return unqueued;
    if (e.leftAlone())
        return e.item;
    return frameItem(e);
}

/* Kind 7: what share a car's two numbers make, in percent, and a count. */
u32 drawPercent(const Element& e)
{
    Guest& guest = e.guest;
    const u32 part = guest.dword(e.car + kCarGood);
    const u32 whole = part + guest.dword(e.car + kCarOther);
    // idiv, its dividend sign extended; the quotient's low half.
    const u32 percent = whole != 0 ? u32(std::int64_t(s32(part * 100)) / s32(whole)) : 100;
    return guest::direct<0x489900, frontend::hudr::drawPercentAndCount>(
        guest.app(), e.ts, percent, guest.dword(e.car + kCarCount), guest.dword(kScaleSmall), e.colour, e.box(kBoxLeft),
        e.box(kBoxRight), e.top, e.alignment);
}

/* Kind 10: the place among the racers; in the first language with its letters,
 * for which it takes the large scale too. */
u32 drawPlace(const Element& e)
{
    Guest& guest = e.guest;
    const bool plain = gameSettings(guest).language != 0;
    const u32 left = e.box(kBoxLeft), right = e.box(kBoxRight);
    const u32 small = guest.dword(kScaleSmall);
    if (plain)
    {
        const u32 among = fe::direct<0x4cdee0, frontend::loopb::racerCount>(guest.app(), e.ts);
        return guest::direct<0x48a320, frontend::hudr::drawPlaceAmong>(guest.app(), e.ts, guest.dword(kPosition), among, small,
                                                                        e.colour, left, right, e.top, e.alignment);
    }
    const u32 large = guest.dword(kScaleLarge);
    const u32 among = fe::direct<0x4cdee0, frontend::loopb::racerCount>(guest.app(), e.ts);
    const u32 drawn = u32(*fe::direct<0x48a3a0, port::hud::rankTextOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 7,
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
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.ts, guest.dword(kBackGroup) + (e.view << 5));
}

/* The list's panel, its first and third corners, as a flat rectangle in the
 * colour without its alpha: for the element the editor's pointer is on. */
u32 frameListPanel(const Element& e)
{
    constexpr u32 kFirstX = 0, kFirstY = 4, kThirdX = 0x40, kThirdY = 0x44;
    Guest& guest = e.guest;
    if (e.leftAlone())
        return e.item;
    const u32 corner = kBackVertices + ((guest.dword(kBackQuads) + e.view * 4) << 5);
    FloatMath math(guest);
    const u32 bottom = u32(math.truncated(math.load(corner + kThirdY)));
    const u32 right = u32(math.truncated(math.load(corner + kThirdX)));
    const u32 top = u32(math.truncated(math.load(corner + kFirstY)));
    const u32 left = u32(math.truncated(math.load(corner + kFirstX)));
    return *fe::direct<0x459350, graphics::flatRectangle>(guest.app(), e.ts, left, top, right, bottom, e.colour & kColourNoAlpha);
}

/* Kind 13: the standings: its rectangle's element made again first, as they grow. */
u32 drawStandings(const Element& e)
{
    Guest& guest = e.guest;
    // The view the game's code hands it in eax is not read.
    fe::direct<0x47ee80, frontend::rendb::prepareStandings>(guest.app(), e.ts, e.car);
    guest::direct<0x4853c0, frontend::hudr::rebuildElement>(guest.app(), e.ts, e.view, kStandings);
    queueListPanel(e);
    fe::direct<0x48ae70, port::hud::standingsOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 4, guest.dword(kScaleRows),
                                                 e.box(kBoxLeft), guest.dword(kListWidths + e.view * 4), e.view);
    return frameListPanel(e);
}

/* Kind 20: the list of labels. */
u32 drawLabelRows(const Element& e)
{
    Guest& guest = e.guest;
    queueListPanel(e);
    fe::direct<0x48a660, port::hud::labelRowsOf>(guest.app(), e.ts, e.ts.sp - 4 - 4 * 4, e.view, guest.dword(kScaleRows),
                                                 e.box(kBoxLeft), e.box(kBoxRight), guest.dword(kListWidths + e.view * 4));
    return frameListPanel(e);
}

/* Kinds 17 and 22: the track's map and what is on it (its panel on a card without
 * alpha), then the mesh of its lines and the queues, the last under kStateShape on
 * a card with alpha. */
u32 drawMap(const Element& e)
{
    Guest& guest = e.guest;
    const u32 view = e.view;
    fe::direct<0x48d4c0, port::hud::mapPointsOf>(guest.app(), e.ts, e.ts.sp - 4, view, e.car, 0, 0);
    fe::direct<0x47fbe0, frontend::rendb::queueEightQuads>(guest.app(), e.ts, guest.dword(kMapGroup) + (view << 5));
    if (!e.cardWithAlpha())
    {
        const u32 quad = guest.dword(kMapQuads) + view * 4;
        queueQuad(guest, kBackQueue, quad);
        flushQueue(guest, kBackQueue);
        unqueueQuad(guest, kBackQueue, quad);
    }
    guest::direct<0x48e770, frontend::hudr::flushLines>(guest.app(), e.ts);
    flushQueue(guest, kDialQueue);
    if (e.cardWithAlpha())
        renderState(guest, e.ts, kStateShape, 1);
    const u32 flushed = flushQueue(guest, kOverQueue);
    if (!e.cardWithAlpha())
        return flushed;
    return renderState(guest, e.ts, kStateShape, 0);
}

/* Kind 8: the fonts and the views' text flushed, the lights lit, the bands drawn. */
u32 drawLights(const Element& e)
{
    Guest& guest = e.guest;
    fe::direct<0x4d1340, port::text::fontsDrawn>(guest.app(), e.ts, e.ts.sp - 4);
    guest::direct<0x4808b0, frontend::hudz::unqueueViewQuads>(guest.app(), e.ts, 0);
    if (gameSettings(guest).playMode == 1)
        guest::direct<0x4808b0, frontend::hudz::unqueueViewQuads>(guest.app(), e.ts, 1);
    guest.setDword(kLightsB, 0);
    guest.setDword(kLightsA, 0);
    fe::direct<0x48c7c0, render::startLightsDrawn>(guest.app(), e.ts, 0xc, 5, 8, 3, 0);
    flushQueue(guest, kDialQueue);
    const u32 band = *fe::direct<0x4800a0, render::hudBandBuilt>(guest.app(), e.ts, 0);
    if (gameSettings(guest).playMode != 1)
        return band;
    return *fe::direct<0x4800a0, render::hudBandBuilt>(guest.app(), e.ts, 1);
}

/* Kind 6: the grid in the colour, or none when the editor's pointer is elsewhere,
 * and its second pass under kStateShape on a card with alpha. */
u32 drawGrid(const Element& e)
{
    Guest& guest = e.guest;
    const u32 result =
        guest::direct<0x482030, frontend::hudz::setGridHighlight>(guest.app(), e.ts, 0, e.view, e.leftAlone() ? 0 : e.colour);
    if (!e.cardWithAlpha())
        return result;
    renderState(guest, e.ts, kStateShape, 1);
    guest::direct<0x482190, frontend::hudz::drawGridHighlight>(guest.app(), e.ts, e.view);
    return renderState(guest, e.ts, kStateShape, 0);
}

/* An element's colour: white or grey, by its kind, when the editor's pointer is
 * elsewhere; on it, a pulse of the frame's counter. */
u32 elementColour(Guest& guest, nfs::ThreadState& ts, u32 item, u32 kind)
{
    if (guest.dword(item + kItemLeftAlone) != 0)
        return fe::direct<0x47bc20, frontend::rendb::kindIsText>(guest.app(), ts, kind) != 0 ? kColourGrey : kColourWhite;
    std::uint8_t pulse = std::uint8_t((guest.dword(kPulse) & 0x7f) * 2);
    if (pulse > 0x7f)
        pulse = std::uint8_t(0xff - pulse);
    const std::uint8_t rest = std::uint8_t(0x80 - pulse);
    const u32 low = rest > 0x40 ? 0x40 : rest;
    return 0xff000000 | ((pulse + 0x80u) << 16) | (low << 8) | low;
}

}  // namespace

/* sub_45ab30: an element of the HUD drawn for its view: eax its item.  What it
 * hands back is what its last call left in eax, or what a kind that ends without
 * a call had there: the item, the kind. */
nfs::u32 frontend::hudr::drawElement(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x5c);
    constexpr std::uint32_t kLabelLineId = 0x5fd, kKeyLineId = 0x151;
    constexpr u32 kKeyAlignment = 4;
    constexpr u32 kKeyColour = 0x9ffafa00, kMeterColour = 0xff151f1f, kMarkersColour = 0x8f004f00;

    const std::int32_t number = std::int32_t(guest.dword(item + kItemKind));
    const u32 view = number >= kKindsPerView ? 1 : 0;
    const u32 kind = u32(view != 0 ? number - kKindsPerView : number);
    const u32 top = guest.dword(kBoxes + view * kBoxesPerView + kind * kRectangleBytes + kBoxTop);
    const u32 alignment = guest.dword(kAlignments + view * kAlignmentsPerView + kind * 4);
    const u32 car = cameraTarget(guest, ts, view);
    const u32 colour = elementColour(guest, ts, item, kind);
    const Element e{ guest, ts, item, view, kind, car, colour, top, alignment };
    const u32 small = guest.dword(kScaleSmall);
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
        return guest::direct<0x4899d0, drawLaps>(app, ts, 0, gameSettings(guest).laps, small, colour, e.box(kBoxLeft),
                                                 e.box(kBoxRight), top, alignment);
    case kLapTimeText:
        return guest::direct<0x4898b0, drawTime>(app, ts, guest.dword(kLapTime), small, colour, e.box(kBoxLeft),
                                                 e.box(kBoxRight), top, alignment);
    case kGrid:
        return drawGrid(e);
    case kPercentAndCount:
        return drawPercent(e);
    case kStartLights:
        return drawLights(e);
    case kKind9:
        guest::direct<0x484030, hudz::flushFrameQueue>(app, ts);
        return e.leftAlone() ? item : frameItem(e);
    case kPlace:
        return drawPlace(e);
    case kTimeZero:
        return guest::direct<0x4898b0, drawTime>(app, ts, 0, small, colour, e.box(kBoxLeft), e.box(kBoxRight), top,
                                                 alignment);
    case kLabelLine:
        return drawLanguageLine(guest, ts, e.text({ 0 }, guest.dword(kScaleLarge)), kLabelLineId);
    case kStandings:
        return drawStandings(e);
    case kRatioFirst:
        return guest::direct<0x489a50, drawRatio>(app, ts, guest.dword(car + kCarRatioA), gameSettings(guest).pursuitOutCount,
                                                  small, colour, e.box(kBoxLeft), e.box(kBoxRight), top, alignment);
    case kKeyLine:
    {
        Element line = e;
        line.colour = e.leftAlone() ? kKeyColour : colour;
        line.top = e.box(kBoxTop);
        line.alignment = kKeyAlignment;
        return drawLanguageLine(guest, ts, line.text({ 0 }, small), kKeyLineId);
    }
    case kSegmentMeter:
    {
        const u32 meterColour = e.leftAlone() ? kMeterColour : colour;
        const u32 left = e.box(kBoxLeft), meterTop = e.box(kBoxTop);
        return *fe::direct<0x48c0e0, render::meterDrawn>(guest.app(), e.ts, view, left, meterTop, e.box(kBoxRight) - left,
                                                         e.box(kBoxBottom) - meterTop, u32(-1), meterColour);
    }
    case kMap:
    case kCopMap:
        return drawMap(e);
    case kMarkers:
    {
        const u32 markerColour = e.leftAlone() ? kMarkersColour : colour;
        const u32 set = u32(*fe::direct<0x48b5f0, port::hud::markersOf>(app, ts, ts.sp - 4, view, car, car, markerColour));
        // edx, ebx and ecx as the call by address leaves them.
        return set;
    }
    case kRatioSecond:
        return guest::direct<0x489a50, drawRatio>(app, ts, guest.dword(car + kCarRatioB), gameSettings(guest).pursuitWinCount,
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
nfs::u32 frontend::hudr::stepMapPanes(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const u32 stepped = u32(*fe::direct<0x48e790, port::hud::paneStepOf>(app, ts, ts.sp - 4, 0u));
    if (gameSettings(guest).playMode != 1)
        return stepped;
    const u32 firstForm = formOf(guest, cameraTarget(guest, ts, 0));
    const u32 secondForm = formOf(guest, cameraTarget(guest, ts, 1));
    const HudRecord& first = gameSettings(guest).hudOf(1, firstForm);
    const HudRecord& second = gameSettings(guest).hudOf(2, secondForm);
    const HudRect& firstBox = first.rects[firstForm != 0 ? kCopMap : kMap];
    const HudRect& secondBox = second.rects[secondForm != 0 ? kCopMap : kMap];
    const auto atTop = [](const HudRect& box) -> u32 { return (box.leftBits & 0x7fffffff) == 0 ? 1 : 0; };
    for (;;)
    {
        if (first.mapStyle == second.mapStyle)
        {
            const u32 firstAtTop = atTop(firstBox);
            if (firstAtTop == atTop(secondBox))
                return firstAtTop;
        }
        fe::direct<0x48e790, port::hud::paneStepOf>(app, ts, ts.sp - 4, 1u);
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
nfs::u32 frontend::hudr::ordinalLetters(App*, ThreadState&, u32 number)
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
constexpr u32 kTimeFormats = 0x55d6cc;

}  // namespace

/* sub_489750: a time in 64ths of a second, written without its sign into `buffer`
 * as minutes, seconds and hundredths, in the language's format.  edx is left 0:
 * the sign of the hundredths, which have none. */
nfs::u32 frontend::hudr::formatTime(App* app, ThreadState& ts, u32 buffer, u32 timeWord)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 4);
    const std::int32_t time = std::int32_t(timeWord);
    constexpr std::int32_t kTicksPerSecond = 64, kSecondsPerMinute = 60;
    const std::int32_t magnitude = time > 0 ? time : std::int32_t(u32(0) - u32(time));
    const std::int32_t seconds = magnitude / kTicksPerSecond;
    const std::int32_t hundredths = (magnitude % kTicksPerSecond) * 100 / kTicksPerSecond;
    const u32 format = guest.dword(kTimeFormats + gameSettings(guest).language * 4);
    const u32 written = fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, format, guest::Words{u32(seconds / kSecondsPerMinute), u32(seconds % kSecondsPerMinute), u32(hundredths)});
    return written;
}

namespace
{

constexpr u32 kTextBufferBytes = 0x100;

/* What sub_4897f0 takes on the stack, and what the functions over it pass on. */
struct TextPlace
{
    u32 scale, colour, left, right, top, how;
};

TextPlace textPlaceOf(nfs::App* app, const nfs::ThreadState& ts)
{
    return { stackArgument(app, ts, 0), stackArgument(app, ts, 1), stackArgument(app, ts, 2),
             stackArgument(app, ts, 3), stackArgument(app, ts, 4), stackArgument(app, ts, 5) };
}

u32 drawPlaced(Guest& guest, u32 buffer, const TextPlace& place)
{
    return alignedText(guest, buffer, place.scale, place.colour, place.left, place.right, place.top, place.how);
}

}  // namespace

/* sub_4898b0: a time at its place. */
nfs::u32 frontend::hudr::drawTime(App* app, ThreadState& ts, u32 time, u32 scale, u32 colour, u32 left, u32 right, u32 top, u32 how)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const u32 buffer = guest.frame();
    guest::direct<0x489750, formatTime>(app, ts, buffer, time);
    return drawPlaced(guest, buffer, place);
}

namespace
{

constexpr GuestString kPercentFormat{ 0x53b7ec, "%d%% %s" };
constexpr GuestString kCountFormat{ 0x53b7f4, "%d %s" };
constexpr GuestString kPercentStrings[] = { kPercentFormat, kCountFormat };
constexpr std::uint32_t kPercentLine = 0x152, kCountOfOneLine = 0x153, kCountLine = 0x154;
constexpr u32 kPercentSign = '%';

}  // namespace

/* sub_489900: "85% ..." at its place and, a percent sign's height below it, how
 * many: one has a line of its own. */
nfs::u32 frontend::hudr::drawPercentAndCount(App* app, ThreadState& ts, u32 percent, u32 count, u32 scale, u32 colour, u32 left, u32 right, u32 top, u32 how)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const u32 buffer = guest.frame();
    const u32 line = languageLine(guest, ts, kPercentLine);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kPercentFormat.address, guest::Words{percent, line});
    drawPlaced(guest, buffer, place);
    if (count == 1)
    {
        const u32 one = languageLine(guest, ts, kCountOfOneLine);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, one, guest::Words({}));
    }
    else
    {
        const u32 many = languageLine(guest, ts, kCountLine);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kCountFormat.address, guest::Words{count, many});
    }
    TextPlace below = place;
    below.top = place.top + fe::direct<0x4d1540, port::text::characterHeight>(guest.app(), guest.ts(), kPercentSign);
    const u32 drawn = drawPlaced(guest, buffer, below);
    return drawn;
}

namespace
{

constexpr GuestString kLapsWordFirst{ 0x53b7fc, "%s %d/%d" };
constexpr GuestString kLapsWordLast{ 0x53b808, "%d/%d %s" };
constexpr GuestString kLapsStrings[] = { kLapsWordFirst, kLapsWordLast };
constexpr std::uint32_t kLapsLine = 0x155;
constexpr u32 kLanguageWordFirst = 2;

}  // namespace

/* sub_4899d0: a count of a total with its word, first in one language and last
 * in the rest. */
nfs::u32 frontend::hudr::drawLaps(App* app, ThreadState& ts, u32 done, u32 total, u32 scale, u32 colour, u32 left, u32 right, u32 top, u32 how)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const u32 buffer = guest.frame();
    const u32 line = languageLine(guest, ts, kLapsLine);
    if (gameSettings(guest).language == kLanguageWordFirst)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kLapsWordFirst.address, guest::Words{line, done, total});
    else
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kLapsWordLast.address, guest::Words{done, total, line});
    const u32 drawn = drawPlaced(guest, buffer, place);
    return drawn;
}

namespace
{

constexpr GuestString kRatioFormat{ 0x53b814, "%s: %d/%d" };
constexpr GuestString kRatioStrings[] = { kRatioFormat };
constexpr std::uint32_t kRatioLine = 0x14d;

}  // namespace

/* sub_489a50: a line of the language's with a count of a total. */
nfs::u32 frontend::hudr::drawRatio(App* app, ThreadState& ts, u32 done, u32 total, u32 scale, u32 colour, u32 left, u32 right, u32 top, u32 how)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + kTextBufferBytes);
    const TextPlace place{ scale, colour, left, right, top, how };
    const u32 buffer = guest.frame();
    const u32 line = languageLine(guest, ts, kRatioLine);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kRatioFormat.address, guest::Words{ line, done, total });
    const u32 drawn = drawPlaced(guest, buffer, place);
    return drawn;
}

namespace
{

constexpr GuestString kPlaceFormatPos{ 0x53b890, "Pos. %d/%d" };
constexpr GuestString kPlaceFormatOf{ 0x53b89c, "%d\xba de %d" };
constexpr GuestString kPlaceFormat{ 0x53b8a8, "%d/%d" };
constexpr GuestString kPlaceStrings[] = { kPlaceFormatPos, kPlaceFormatOf, kPlaceFormat };
constexpr u32 kLanguagePos = 2, kLanguageOf = 3;

}  // namespace

/* sub_48a320: a place among so many, as the language writes it. */
nfs::u32 frontend::hudr::drawPlaceAmong(App* app, ThreadState& ts, u32 place, u32 among, u32 scale, u32 colour, u32 left, u32 right, u32 top, u32 how)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + kTextBufferBytes);
    const TextPlace at{ scale, colour, left, right, top, how };
    const u32 buffer = guest.frame();
    const u32 language = gameSettings(guest).language;
    const u32 format = language == kLanguagePos ? kPlaceFormatPos.address
        : language == kLanguageOf ? kPlaceFormatOf.address : kPlaceFormat.address;
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, format, guest::Words{ place, among });
    const u32 drawn = drawPlaced(guest, buffer, at);
    return drawn;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's lines
 * ------------------------------------------------------------------------- */

// The bounds a line's vertex is clipped by.
constexpr u32 kClipDepth = 0x7d34f8;
constexpr u32 kClipLeft = 0x7d34fc;
constexpr u32 kClipBottom = 0x7d3500;
constexpr u32 kClipTop = 0x7d3504;
constexpr u32 kClipRight = 0x7d350c;
constexpr std::uint8_t kPastLeft = 1, kPastRight = 2, kPastBottom = 4, kPastTop = 8, kPastDepth = 0x10;

// A vertex on the line layer: x and y as integers.
constexpr u32 kVertexBytes = 0x20;
constexpr u32 kVertexX = 0, kVertexY = 4, kVertexDepth = 0xc, kVertexColour = 0x10, kVertexClip = 0x14;
constexpr u32 kBlankVerticesBytes = 3 * kVertexBytes;
constexpr u32 kTriangleBlanks = 0x489630;    // what the triangle's three vertices start from
constexpr u32 kLineBlanks = 0x489690;        // the line's two, and an empty one
constexpr u32 kStateTexture = 1;              // set to none

/* A vertex's clip code.  A negative number is past its low bound whatever the
 * bound. */
std::uint8_t clipCode(const Guest& guest, u32 vertex)
{
    const auto before = [&](u32 value, u32 bound) { return s32(value) < 0 || s32(value) < s32(guest.dword(bound)); };
    const u32 depth = guest.dword(vertex + kVertexDepth);
    if (s32(depth) < 0 || s32(depth) >= s32(guest.dword(kClipDepth)))
        return kPastDepth;
    std::uint8_t code = 0;
    const u32 y = guest.dword(vertex + kVertexY);
    if (before(y, kClipTop))
        code |= kPastTop;
    else if (s32(y) > s32(guest.dword(kClipBottom)))
        code |= kPastBottom;
    const u32 x = guest.dword(vertex + kVertexX);
    if (before(x, kClipLeft))
        code |= kPastLeft;
    else if (s32(x) > s32(guest.dword(kClipRight)))
        code |= kPastRight;
    return code;
}

/* The frame's vertices from the game's blank ones, the points' x and y and the
 * colour on them, and their clip codes. */
void placeVertices(Guest& guest, u32 blanks, std::initializer_list<u32> coordinates, u32 colour)
{
    const u32 frame = guest.frame();
    for (u32 at = 0; at < kBlankVerticesBytes; at += 4)
        guest.setDword(frame + at, guest.dword(blanks + at));
    u32 vertex = frame;
    for (const u32* coordinate = coordinates.begin(); coordinate != coordinates.end(); coordinate += 2, vertex += kVertexBytes)
    {
        guest.setDword(vertex + kVertexX, coordinate[0]);
        guest.setDword(vertex + kVertexY, coordinate[1]);
        guest.setDword(vertex + kVertexColour, colour);
    }
    for (u32 placed = frame; placed != vertex; placed += kVertexBytes)
        guest.setByte(placed + kVertexClip, clipCode(guest, placed));
}

}  // namespace

/* sub_48b2a0: a triangle in a colour, filled or as three lines. */
nfs::u32 frontend::hudr::drawTriangle(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 filled, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x60);
    placeVertices(guest, kTriangleBlanks, { x0, y0, x1, y1, x2, y2 }, colour);
    const u32 first = guest.frame(), second = first + kVertexBytes, third = second + kVertexBytes;
    renderState(guest, ts, kStateTexture, 0);
    if (filled != 0)
        return fe::direct<0x4c11b0, vertices::triangleClipped>(guest.app(), guest.ts(), first, second, third);
    fe::direct<0x4c0b20, render::lineDrawn>(app, ts, first, second);
    fe::direct<0x4c0b20, render::lineDrawn>(app, ts, second, third);
    return *fe::direct<0x4c0b20, render::lineDrawn>(app, ts, third, first);
}

/* sub_48b4a0: a line in a colour. */
nfs::u32 frontend::hudr::drawLine(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x60);
    placeVertices(guest, kLineBlanks, { x0, y0, x1, y1 }, colour);
    const u32 first = guest.frame();
    renderState(guest, ts, kStateTexture, 0);
    return *fe::direct<0x4c0b20, render::lineDrawn>(app, ts, first, first + kVertexBytes);
}

namespace
{

constexpr u32 kLineMesh = 0x793108;

}

/* sub_48e770: the HUD's line mesh flushed. */
nfs::u32 frontend::hudr::flushLines(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return *fe::direct<0x49bf60, graphics::lineMeshFlushed>(app, ts, kLineMesh);
}

/* sub_48e780: the HUD's line mesh made ready. */
nfs::u32 frontend::hudr::prepareLines(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x49bef0, hudx::resetSurface>(app, ts, kLineMesh);
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::hudDrawElement(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45ab30", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::drawElement(app, ts, item); });
}

nfs::u32 frontend::hudRebuildElement(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4853c0", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const u32 view = args.eax, element = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::rebuildElement(app, ts, view, element); });
}

nfs::u32 frontend::hudStepMapPanes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48e920", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::stepMapPanes(app, ts); });
}

nfs::u32 frontend::hudOrdinalLetters(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4896f0", 0, 0, kLettersStrings, std::size(kLettersStrings), kKeepsEcxEdx };
    const u32 place = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::ordinalLetters(app, ts, place); });
}

nfs::u32 frontend::hudFormatTime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_489750", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const u32 buffer = args.eax, time = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::formatTime(app, ts, buffer, time); });
}

nfs::u32 frontend::hudDrawTime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4898b0", 0x18, 0, nullptr, 0 };
    const u32 time = args.eax;
    const TextPlace p = textPlaceOf(app, ts);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudr::drawTime(app, ts, time, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

nfs::u32 frontend::hudDrawPercentAndCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_489900", 0x18, 0, kPercentStrings, std::size(kPercentStrings), kKeepsEbxEcxEsiEdi };
    const u32 percent = args.eax, count = args.edx;
    const TextPlace p = textPlaceOf(app, ts);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudr::drawPercentAndCount(app, ts, percent, count, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

nfs::u32 frontend::hudDrawLaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4899d0", 0x18, 0, kLapsStrings, std::size(kLapsStrings), kKeepsEcxEsiEdi };
    const u32 done = args.eax, total = args.edx;
    const TextPlace p = textPlaceOf(app, ts);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudr::drawLaps(app, ts, done, total, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

nfs::u32 frontend::hudDrawRatio(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_489a50", 0x18, 0, kRatioStrings, std::size(kRatioStrings), kKeepsEbxEcxEsiEdi };
    const u32 done = args.eax, total = args.edx;
    const TextPlace p = textPlaceOf(app, ts);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudr::drawRatio(app, ts, done, total, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

nfs::u32 frontend::hudDrawPlaceAmong(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48a320", 0x18, 0, kPlaceStrings, std::size(kPlaceStrings), kKeepsEcxEsiEdi };
    const u32 placeNumber = args.eax, among = args.edx;
    const TextPlace p = textPlaceOf(app, ts);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudr::drawPlaceAmong(app, ts, placeNumber, among, p.scale, p.colour, p.left, p.right, p.top, p.how);
    });
}

nfs::u32 frontend::hudDrawTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48b2a0", 0x20, 0, nullptr, 0 };
    const u32 x0 = stackArgument(app, ts, 0), y0 = stackArgument(app, ts, 1), x1 = stackArgument(app, ts, 2);
    const u32 y1 = stackArgument(app, ts, 3), x2 = stackArgument(app, ts, 4), y2 = stackArgument(app, ts, 5);
    const u32 filled = stackArgument(app, ts, 6), colour = stackArgument(app, ts, 7);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return hudr::drawTriangle(app, ts, x0, y0, x1, y1, x2, y2, filled, colour); });
}

nfs::u32 frontend::hudDrawLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48b4a0", 0x14, 0, nullptr, 0 };
    const u32 x0 = stackArgument(app, ts, 0), y0 = stackArgument(app, ts, 1), x1 = stackArgument(app, ts, 2);
    const u32 y1 = stackArgument(app, ts, 3), colour = stackArgument(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::drawLine(app, ts, x0, y0, x1, y1, colour); });
}

nfs::u32 frontend::hudFlushLines(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48e770", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::flushLines(app, ts); });
}

nfs::u32 frontend::hudPrepareLines(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48e780", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudr::prepareLines(app, ts); });
}

}
