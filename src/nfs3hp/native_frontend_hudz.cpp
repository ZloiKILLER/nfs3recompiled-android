#include "native_frontend_hudz.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_hudr.h"
#include "native_frontend_rendb.h"
#include "native_graphics.h"
#include "native_raceload.h"
#include "port_loop.h"
#include "port_text.h"
#include "native_vertices.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstdint>
#include <cstring>
#include <iterator>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
nfs::u32 hudEditElementFits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudEditElementMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudUnqueueViewQuads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudSetAlignments(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudPlaceDialArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudLayMarkersFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudLayPanel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudLayNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudLayReplayBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudLayGrid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudMapPaneIsSmall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudSetGridHighlight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawGridHighlight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudDrawReplayBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudFlushFrameQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudQueueStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudQueueTakeCorners(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}
}

/* The race's HUD, the parts that lay it out, decompiled:
 *
 *   the HUD editor   whether an element may stand where it is being dragged, with
 *                    which other element's item in its way (sub_459670), and the
 *                    drag put into the layout (sub_459db0);
 *   the elements     the alignments of a view's texts (sub_480a00) and whether its
 *                    map is the small form (sub_481f90);
 *   their vertices   laid over the boxes at 0x749758: the frame around the markers
 *                    (sub_481290), the panel (sub_481510), the replay bar's 21 quads
 *                    (sub_4815d0), the grid's 13 quads (sub_4818b0), the art of a
 *                    dial (sub_481170);
 *   drawing          the grid's highlighted part, set and drawn (sub_482030,
 *                    sub_482190), the replay bar (sub_482880), the quads a view has
 *                    queued taken back (sub_4808b0), the frames' queue flushed
 *                    (sub_484030);
 *   the queues       one started, and corners taken from it (sub_49bc80, sub_49bd10).
 *
 * The HUD's state stays in guest memory, where port_hud.cpp reads it; what these
 * call is called by address.  What each function leaves in eax and in the registers
 * it does not save is kept, the x87's status word in ax too where sub_480a00 goes out
 * after a `fnstsw ax`.
 *
 * Plan step 5: each function a typed core in frontend::hudz (native_frontend_hudz.h), in a frame of
 * its own where the game's function has one, and a thunk under the game's name for the calls by
 * address.  A core of this file calls another, and those of the HUD's other files done so
 * (native_frontend_hudr.cpp, _hudx.cpp), as C++ (guest::direct), and the typed cores of other files
 * too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* The port's own (nfs3hp_main.cpp), from the port's HUD editor change: the layout as laid
 * out, not as a wide screen draws it, and the layouts as measured drawn for the length
 * of the test whether an element fits, ts holding the test's arguments. */
void hudLayoutDesigned(nfs::App* app, nfs::u32 rect, nfs::u32 element);
/* nfs3hp_main.cpp's: with `drawn`, the test's arguments in `registers` (eax the rectangle tried, edx the player, ebx the
 * element, ecx the editor's mode); without, its answer in registers.eax.  (The overload that took them from ts is gone.) */
void hudLayoutsDrawn(nfs::App* app, nfs::ThreadState& ts, bool drawn, nfs::u32 rect, nfs::u32 playerIn, nfs::u32 elementIn, nfs::u32 modeIn, nfs::u32 fits, nfs::u32 blockerAt);

namespace
{

using fe::args;
using fe::Guest;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::s16;
using nfs::s32;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(u16 status) { return (status & 0x4100) == 0; }  // ja; false unordered
bool jumpsBelow(u16 status) { return (status & 0x0100) != 0; }  // jb; true unordered

bool isAbove(Float a, Float b) { return jumpsAbove(fp::compare(a, b)); }
bool isBelow(Float a, Float b) { return jumpsBelow(fp::compare(a, b)); }

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, u32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, u32 address) { return Float(s32(guest.dword(address))); }

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The game's functions, by address.

// The numbers of the screen.
constexpr u32 kViewWidth = 0x7cdac8, kViewHeight = 0x7cdacc;
constexpr u32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kCardWithAlpha = 2;     // not voodoo2a's

// A car, as sub_422bd0 gives it.
constexpr u32 kCarFlags = 0x200;             // byte; kCarIsCop: the cop's form of the HUD
constexpr std::uint8_t kCarIsCop = 0x20;

// The HUD's records, by seat and by form; a seat is a view, one on in a split screen.
constexpr u32 kRecordsPerSeat = 0x348;
constexpr u32 kRecordsPerForm = 0x1a4;
constexpr u32 kRectangles = 0x24;            // 23 of 16 bytes, fractions of the screen
constexpr u32 kRectangleBytes = 16;
constexpr u32 kRectangleTop = 0, kRectangleLeft = 4, kRectangleBottom = 8, kRectangleRight = 12;

// The kinds of element these name, a rectangle each.
constexpr u32 kElements = 23;
constexpr u32 kSpeedFigures = 0;
constexpr u32 kSpeedDial = 1;
constexpr u32 kRevFigures = 2;
constexpr u32 kRevDial = 3;
constexpr u32 kLapTimeText = 5;
constexpr u32 kGrid = 6;
constexpr u32 kReplayBar = 8;
constexpr u32 kKind9 = 9;
constexpr u32 kFirstOfModes = 10;            // from here to kMarkers one mode's, past it the other's
constexpr u32 kStandings = 13;
constexpr u32 kMap = 17;
constexpr u32 kMarkers = 18;
constexpr u32 kLabelRows = 20;
constexpr u32 kPanel = 21;
constexpr u32 kCopMap = 22;
// After the 23, the map's place while it is the small form (sub_459db0 keeps it).
constexpr u32 kSmallMapRectangle = kRectangles + kElements * kRectangleBytes;

// The pixel boxes of a view's elements: left, top, right, bottom.
constexpr u32 kBoxes = 0x749758;
constexpr u32 kBoxBytes = 16;
constexpr u32 kBoxesPerView = kElements * kBoxBytes;

// How a view's texts are aligned (sub_4897f0), 23 to a view.
constexpr u32 kAlignments = 0x725690;
constexpr u32 kAlignmentsPerView = kElements * 4;
constexpr u32 kCentred = 8;
constexpr u32 kCentredToo = 0x10;

// Vertices: 0x20 bytes, four to a quad.
constexpr u32 kCornerBytes = 0x20;
constexpr u32 kQuadBytes = 4 * kCornerBytes;
constexpr u32 kCornerX = 0, kCornerY = 4, kCornerDepth = 0xc, kCornerColour = 0x10, kCornerClip = 0x14;
constexpr u32 kCornerU = 0x18, kCornerV = 0x1c;

// The queues of quads and the batches of corners in them (native_frontend_rendb.cpp).
constexpr u32 kDialQueue = 0x749a38;
constexpr u32 kFrameQueue = 0x700248;
constexpr u32 kViewQuadCounts = 0x700240;    // by view: the quads it has queued
constexpr u32 kViewCorners = 0x791a88;       // the queue's numbers of them, 7 quads to a view
constexpr u32 kViewCornersEach = 7 * 4;
constexpr u32 kPanelQuads = 0x791ae0;        // a quad to a view
constexpr u32 kFrameQuads = 0x791ae4;        // 8 quads to a view
constexpr u32 kFrameQuadsEach = 8 * kQuadBytes;
constexpr u32 kGridQuads = 0x791a58;         // 13 quads to a view
constexpr u32 kGridCorners = 0x791a84;       // the queue's numbers of them
constexpr u32 kGridQuadsEach = 13;

/* Whether a car is a cop's, as 0 or 1: the form of the HUD its view has. */
u32 formOf(const Guest& guest, u32 car)
{
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

/* The seat of a view, by the screen's split. */
u32 seatOf(const Guest& guest, u32 view)
{
    const u32 players = gameSettings(guest).playMode;
    return view + (players == 1 ? players : 0);
}

/* The offset of a seat's form in the records. */
constexpr u32 formOffset(u32 seat, u32 form)
{
    return seat * kRecordsPerSeat + form * kRecordsPerForm;
}

/* A view's box for a kind of element. */
constexpr u32 boxOf(u32 view, u32 kind)
{
    return kBoxes + view * kBoxesPerView + kind * kBoxBytes;
}

u32 queueQuad(Guest& guest, u32 first)
{
    return u32(*fe::direct<0x49bd30, port::text::quadQueued>(guest.app(), guest.ts(), guest.ts().sp - 8, kDialQueue, first, first + 1, first + 2, first + 3));
}

u32 unqueueQuad(Guest& guest, u32 first)
{
    return u32(*fe::direct<0x49bdc0, port::text::quadUnqueued>(guest.app(), guest.ts(), guest.ts().sp - 8, kDialQueue, first, first + 1, first + 2, first + 3));
}

/* ---------------------------------------------------------------------------
 * The HUD editor's test: sub_459670
 * ------------------------------------------------------------------------- */

struct Fractions
{
    float top, left, bottom, right;
};

Fractions fractionsAt(const HudRect& rectangle)
{
    return { rectangle.top, rectangle.left, rectangle.bottom, rectangle.right };
}

Fractions fractionsAt(const Guest& guest, u32 at)
{
    return { guest.floatAt(at + kRectangleTop), guest.floatAt(at + kRectangleLeft), guest.floatAt(at + kRectangleBottom),
             guest.floatAt(at + kRectangleRight) };
}

/* What two rectangles have in common, as the test works it out for each pair.  A NaN
 * takes `b`'s edge for the top and the left, `a`'s for the bottom and the right. */
Fractions common(const Fractions& a, const Fractions& b)
{
    return { isAbove(a.top, b.top) ? a.top : b.top, isAbove(a.left, b.left) ? a.left : b.left,
             isBelow(a.bottom, b.bottom) ? a.bottom : b.bottom, isBelow(a.right, b.right) ? a.right : b.right };
}

/* Whether what two rectangles have in common has any size at all. */
bool hasSize(const Fractions& shared)
{
    return isBelow(shared.top, shared.bottom) && isBelow(shared.left, shared.right);
}

/* A speed's figures and dial have a second part below them, as high as the lap time's
 * element, which the test weighs as well. */
float lowerPartBottom(const Fractions& above, const Fractions& lapTime)
{
    return float(fp::sub(fp::add(above.bottom, lapTime.bottom), lapTime.top));
}

/* The figures': [0x539bac] of their width, at the right. */
Fractions speedFiguresLowerPart(const Guest& guest, const Fractions& figures, const Fractions& lapTime)
{
    constexpr u32 kShareOfWidth = 0x539bac;
    const float share = float(fp::mul(fp::sub(figures.right, figures.left), doubleAt(guest, kShareOfWidth)));
    return { figures.bottom, float(fp::sub(figures.right, share)), lowerPartBottom(figures, lapTime), figures.right };
}

Fractions speedDialLowerPart(const Fractions& dial, const Fractions& lapTime)
{
    return { dial.bottom, dial.left, lowerPartBottom(dial, lapTime), dial.right };
}

/* Whether another kind of element counts against the one being moved.  An element and
 * what belongs with it are not compared, nor the replay bar or the cop's map; of the kinds
 * from 10 up, the editor's mode has either those below the markers or those from them on. */
bool countsAgainst(u32 moved, u32 other, u32 players, u32 mode)
{
    if (other == kCopMap || moved == kReplayBar || other == kReplayBar)
        return false;
    if (other == kKind9 && players == 1)
        return false;
    if ((moved == kSpeedFigures && other == kSpeedDial) || (other == kSpeedFigures && moved == kSpeedDial))
        return false;
    if ((moved == kRevFigures && other == kRevDial) || (other == kRevFigures && moved == kRevDial))
        return false;
    if (other >= kFirstOfModes)
        return (mode != 0) == (other < kMarkers);
    return true;
}

}  // namespace

/* eax the rectangle the element being moved would have, edx the view, ebx the kind of
 * element, ecx the editor's mode, the stack where the item in its way goes.  1 when it
 * fits: wholly on the screen, in its half of a split screen, and clear of the others of
 * the layout.  0 when not; if an other in its way has an item on the screen, that item is
 * handed back.  ebx, ecx and edx are left as they fall: ebx the seat or the item's number,
 * ecx what the last pass worked with, edx the answer. */
nfs::u32 frontend::hudz::elementFits(App* app, ThreadState& ts, u32 tried, u32 view, u32 moved, u32 mode, u32 blockerOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x100);
    constexpr s32 kHalfBits = 0x3f000000, kOneBits = 0x3f800000;
    constexpr u32 kItemOfElement = 0x1f;     // the widget of an element on the editor's screen
    constexpr u32 kItemKind = 0x3c;
    constexpr u32 kItemsOfSecondView = 23;   // what its items' kinds count on from
    // Two locals of the frame the test reads on a pass with nothing of its own to weigh.
    constexpr u32 kStaleBottom = 0xf4, kStaleRight = 0xf8;

    const u32 players = gameSettings(guest).playMode;
    const u32 onSplit = players == 1 ? players : 0;
    const u32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, view));
    const u32 seat = onSplit + view;

    // Wholly on the screen.  The upper limits are compared as the bits they are.
    const Fractions wanted = fractionsAt(guest, tried);
    if (isAbove(0.0, wanted.top) || isAbove(0.0, wanted.bottom) || isAbove(0.0, wanted.left) || isAbove(0.0, wanted.right))
        return 0;
    const auto bitsAt = [&](u32 edge) { return s32(guest.dword(tried + edge)); };
    if (bitsAt(kRectangleTop) > kOneBits || bitsAt(kRectangleBottom) > kOneBits || bitsAt(kRectangleLeft) > kOneBits
        || bitsAt(kRectangleRight) > kOneBits)
        return 0;

    // In its half of a split screen, but for the replay bar.
    if (gameSettings(guest).playMode == 1)
    {
        const bool outside = seat == 1 ? bitsAt(kRectangleBottom) > kHalfBits : bitsAt(kRectangleTop) < kHalfBits;
        if (outside && moved != kReplayBar)
            return 0;
    }

    // Nothing is in the way of the maps.
    if (moved == kMap || moved == kCopMap)
        return 1;

    const u32 layout = formOffset(seat, form);
    const auto fractionsOf = [&](u32 kind) {
        return fractionsAt(gameSettings(guest).hudAt(layout).rects[kind]);
    };
    // What the last pass weighed of the second parts, its bottom and its right; on a first
    // pass, what the stack holds there.
    float staleBottom = guest.floatAt(guest.frame() + kStaleBottom);
    float staleRight = guest.floatAt(guest.frame() + kStaleRight);
    for (u32 other = 0; other < kElements; ++other)
    {
        if (other == moved || other == kMap || !countsAgainst(moved, other, gameSettings(guest).playMode, mode))
            continue;

        const Fractions against = fractionsOf(other);
        const Fractions shared = common(wanted, against);
        const Fractions lapTime = fractionsOf(kLapTimeText);
        Fractions second;
        if (moved == kSpeedFigures)
            second = common(speedFiguresLowerPart(guest, fractionsOf(kSpeedFigures), lapTime), against);
        else if (other == kSpeedFigures)
            second = common(wanted, speedFiguresLowerPart(guest, fractionsOf(kSpeedFigures), lapTime));
        else if (moved == kSpeedDial)
            second = common(speedDialLowerPart(fractionsOf(kSpeedDial), lapTime), against);
        else if (other == kSpeedDial)
            second = common(wanted, speedDialLowerPart(fractionsOf(kSpeedDial), lapTime));
        else
            // Nothing to weigh: the original compares the bottom and the right it last had,
            // each with itself, which has no size unless one of them is a NaN.
            second = { staleBottom, staleRight, staleBottom, staleRight };
        staleBottom = second.bottom;
        staleRight = second.right;

        if (!hasSize(shared) && !hasSize(second))
            continue;

        // In the way: which item of the screen shows it, if any.
        const u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
        const u32 itsKind = seat == 2 ? other + kItemsOfSecondView : other;
        for (s32 number = s16(guest.word(menu + fe::menu::kFirstItem));; ++number)
        {
            const u32 offset = u32(number) * fe::item::kSize;
            if (guest.dword(fe::item::kTable + offset) == 0)
                return 0;
            const u32 item = fe::item::kTable + offset;
            if (item != 0 && guest.dword(item) == kItemOfElement && guest.dword(item + kItemKind) == itsKind)
            {
                guest.setDword(blockerOut, item);
                return 0;
            }
        }
    }
    return 1;
}

namespace
{

/* ---------------------------------------------------------------------------
 * A drag put into the layout: sub_459db0
 * ------------------------------------------------------------------------- */

/* The item of an element on the editor's screen. */
namespace elementItem
{
constexpr u32 kPixelX = 0x06;                // word
constexpr u32 kPixelY = 0x08;                // word
constexpr u32 kKind = 0x3c;                  // from 23 on, the second view's
constexpr u32 kPlace = 0x40;                 // its rectangle, fractions of the screen
constexpr u32 kLastGood = 0x50;              // the last place that fitted
constexpr u32 kPixelRight = 0x60;
constexpr u32 kPixelBottom = 0x64;
constexpr u32 kClear = 0x70;                 // 0 on the item in another's way
constexpr u32 kBlocker = 0x74;               // the item in this one's way
constexpr u32 kKindsOfView = 23;
}

/* The item's pixels as fractions of the screen. */
void placeFromPixels(Guest& guest, u32 item)
{
    const Float x = Float(s16(guest.word(item + elementItem::kPixelX)));
    const Float perWidth = fp::div(1.0, integerFrom(guest, kViewWidth));
    const Float y = Float(s16(guest.word(item + elementItem::kPixelY)));
    const Float perHeight = fp::div(1.0, integerFrom(guest, kViewHeight));
    const Float left = fp::mul(x, perWidth);
    const Float right = fp::mul(perWidth, integerFrom(guest, item + elementItem::kPixelRight));
    const Float top = fp::mul(y, perHeight);
    guest.setFloat(item + elementItem::kPlace + kRectangleLeft, float(left));
    const Float bottom = fp::mul(perHeight, integerFrom(guest, item + elementItem::kPixelBottom));
    guest.setFloat(item + elementItem::kPlace + kRectangleRight, float(right));
    guest.setFloat(item + elementItem::kPlace + kRectangleTop, float(top));
    guest.setFloat(item + elementItem::kPlace + kRectangleBottom, float(bottom));
}

}  // namespace

/* eax the item moved.  Its pixels go into its rectangle and the layout, the element is
 * made again, and the test says whether it may stand there: 1, and its place is the last
 * good one; 0, and the item in its way is marked. */
nfs::u32 frontend::hudz::elementMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x10);
    constexpr u32 kEditModes = 0x677df0;     // by view
    constexpr u32 kMostOfScreen = 0x539bb4;

    const u32 kind = guest.dword(item + elementItem::kKind);
    const u32 view = s32(kind) >= s32(elementItem::kKindsOfView) ? 1 : 0;
    const u32 element = view ? kind - elementItem::kKindsOfView : kind;
    const u32 place = item + elementItem::kPlace;

    const u32 players = gameSettings(guest).playMode;
    placeFromPixels(guest, item);
    u32 layout;
    if (players == 1)
        layout = formOffset(view + 1, formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, view)));
    else
        layout = formOffset(0, formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, u32(0))));

    const u32 rectangle = SETTINGS_ADDRESS(hud) + kRectangles + layout + element * kRectangleBytes;
    copyDwords(guest, rectangle, place, 4);
    hudLayoutDesigned(app, rectangle, element); /* port: the layout as laid out, not as drawn */

    guest::direct<0x4853c0, hudr::rebuildElement>(app, ts, view, element);
    fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, view);

    if ((element == kMap || element == kCopMap)
        && isBelow(fp::sub(floatFrom(guest, place + kRectangleRight), floatFrom(guest, place + kRectangleLeft)), floatFrom(guest, kMostOfScreen))
        && isBelow(fp::sub(floatFrom(guest, place + kRectangleBottom), floatFrom(guest, place + kRectangleTop)), floatFrom(guest, kMostOfScreen)))
    {
        const u32 seat = seatOf(guest, view);
        const u32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, view));
        copyDwords(guest, SETTINGS_ADDRESS(hud) + kSmallMapRectangle + formOffset(seat, form), place, 4);
    }

    // The test hands the item in the way to the bottom of the frame, and writes nothing there
    // when no item is: the original then goes on with what the stack held.
    const u32 blockerAt = guest.frame();
    const u32 mode = guest.dword(kEditModes + 4 * view);
    nfs::Args handed;
    handed.eax = place;
    handed.edx = view;
    handed.ebx = element;
    handed.ecx = mode;
    hudLayoutsDrawn(app, ts, true, place, view, element, mode, 0, blockerAt); /* port: measured against what is drawn */
    const u32 fits = guest::direct<0x459670, elementFits>(app, ts, place, view, element, mode, blockerAt);
    handed.eax = fits;  // the answer in eax, as the call by address leaves it, for the port's trace
    hudLayoutsDrawn(app, ts, false, place, view, element, mode, fits, blockerAt);

    const u32 blocker = guest.dword(item + elementItem::kBlocker);
    if (blocker != 0)
        guest.setDword(blocker + elementItem::kClear, 1);
    if (fits != 0)
    {
        if (blocker != 0)
            guest.setDword(item + elementItem::kBlocker, 0);
        copyDwords(guest, item + elementItem::kLastGood, place, 4);
        return 1;
    }
    const u32 newBlocker = guest.dword(blockerAt);
    guest.setDword(item + elementItem::kBlocker, newBlocker);
    guest.setDword(newBlocker + elementItem::kClear, 0);
    return 0;
}

/* ---------------------------------------------------------------------------
 * The alignments of a view's texts: sub_480a00
 * ------------------------------------------------------------------------- */

/* eax the view.  The original gave each element whose kind is text (sub_47bc20) the edge
 * of the screen it stood nearest, unless another element stood between: 1 for the left, 2
 * for the right, 4 for neither.  The port has every one of them centred instead
 * (the port's HUD editor change: the text jumped the width of its margin as an element was
 * dragged), and what those comparisons came to is read nowhere else, so they are gone;
 * what stays of them is their asking sub_47bc20 about each of the others, twice.  The
 * standings' two layouts keep theirs, 8 or 16, both centred as sub_4897f0 reads them: 16
 * when what the table leaves below it is less than where its top stands.  eax is left the
 * last rectangle's offset, the comparison's status word in its low half. */
nfs::u32 frontend::hudz::setAlignments(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x98);
    constexpr u32 kHalfScreen = 0x53b510, kLessHalfScreen = 0x53b514;

    const u32 seat = seatOf(guest, view);
    const u32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, view));
    const u32 alignments = kAlignments + view * kAlignmentsPerView;

    for (u32 kind = 0; kind < kElements; ++kind)
    {
        if (fe::direct<0x47bc20, rendb::kindIsText>(app, ts, kind) == 0)
            continue;
        for (u32 pass = 0; pass < 2; ++pass)
            for (u32 other = 0; other < kElements; ++other)
                if (other != kind)
                    fe::direct<0x47bc20, rendb::kindIsText>(app, ts, other);
        guest.setDword(alignments + kind * 4, kCentred); /* port: an element's text stays centred in it, was the edge it stood nearest */
    }

    u32 result = 0;
    for (const u32 kind : { kStandings, kLabelRows })
    {
        const u32 offset = formOffset(seat, form) + kind * kRectangleBytes;
        result = offset;
        const HudRect& rectangle = gameSettings(guest).hudAt(formOffset(seat, form)).rects[kind];
        const Float top = Float(rectangle.top);
        const Float bottom = Float(rectangle.bottom);
        u16 status = 0;
        bool second = false;
        if (seat == 0)
        {
            status = fp::compare(fp::sub(1.0, bottom), top);
            second = jumpsBelow(status);
        }
        else if (seat == 1)
        {
            status = fp::compare(fp::sub(floatFrom(guest, kHalfScreen), bottom), top);
            second = jumpsBelow(status);
        }
        else if (seat == 2)
        {
            const Float below = fp::sub(1.0, bottom);
            status = fp::compare(fp::add(top, floatFrom(guest, kLessHalfScreen)), below);
            second = jumpsAbove(status);
        }
        else
            continue;
        guest.setDword(alignments + kind * 4, second ? kCentredToo : kCentred);
        result = (offset & 0xffff0000) | status;
    }
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The vertices over the boxes
 * ------------------------------------------------------------------------- */

/* The corners of a rectangle, clockwise from its top left. */
void placeQuad(Guest& guest, u32 quad, float left, float top, float right, float bottom)
{
    guest.setFloat(quad + 0 * kCornerBytes + kCornerX, left);
    guest.setFloat(quad + 0 * kCornerBytes + kCornerY, top);
    guest.setFloat(quad + 1 * kCornerBytes + kCornerX, right);
    guest.setFloat(quad + 1 * kCornerBytes + kCornerY, top);
    guest.setFloat(quad + 2 * kCornerBytes + kCornerX, right);
    guest.setFloat(quad + 2 * kCornerBytes + kCornerY, bottom);
    guest.setFloat(quad + 3 * kCornerBytes + kCornerX, left);
    guest.setFloat(quad + 3 * kCornerBytes + kCornerY, bottom);
}

}  // namespace

/* The art of a round dial: the upper or the lower half of the sheet by the dial's form, the
 * left or the right half by [0x6fd4c8].  eax a seat, edx a form, ebx the dial's four
 * corners.  edx is left the dial's form. */
nfs::u32 frontend::hudz::placeDialArt(App* app, ThreadState& ts, u32 seat, u32 form, u32 corners)
{
    Guest guest(app, ts);
    constexpr u32 kEdge = 0x3b000000, kFirstEnd = 0x3e7a0000, kSecondStart = 0x3e810000, kSecondEnd = 0x3efd0000;

    const u32 offset = formOffset(seat, form);
    const u32 dialForm = gameSettings(guest).hudAt(offset).speedometerForm;
    const bool upper = dialForm == 1 || dialForm == 2;
    const u32 top = upper ? kEdge : kSecondStart, bottom = upper ? kFirstEnd : kSecondEnd;
    const bool second = gameSettings(guest).night != 0;
    const u32 left = second ? kSecondStart : kEdge, right = second ? kSecondEnd : kFirstEnd;

    guest.setDword(corners + 0 * kCornerBytes + kCornerU, left);
    guest.setDword(corners + 0 * kCornerBytes + kCornerV, top);
    guest.setDword(corners + 1 * kCornerBytes + kCornerU, right);
    guest.setDword(corners + 1 * kCornerBytes + kCornerV, top);
    guest.setDword(corners + 2 * kCornerBytes + kCornerU, right);
    guest.setDword(corners + 2 * kCornerBytes + kCornerV, bottom);
    guest.setDword(corners + 3 * kCornerBytes + kCornerU, left);
    guest.setDword(corners + 3 * kCornerBytes + kCornerV, bottom);
    return offset;
}


/* The frame's eight quads round the markers' box. */
nfs::u32 frontend::hudz::layMarkersFrame(App* app, ThreadState& ts, u32 view, u32)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 box = boxOf(view, kMarkers);
    const u32 left = guest.dword(box), top = guest.dword(box + 4), right = guest.dword(box + 8), bottom = guest.dword(box + 12);
    const u32 quads = view * kFrameQuadsEach + guest.dword(kFrameQuads);
    return *fe::direct<0x47f650, render::hudFrameBuilt>(app, ts, quads, top, left, bottom, right);
}


/* The panel's quad over its box. */
nfs::u32 frontend::hudz::layPanel(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const u32 quad = guest.dword(kPanelQuads) + view * kQuadBytes;
    const u32 box = boxOf(view, kPanel);
    placeQuad(guest, quad, float(integerFrom(guest, box)), float(integerFrom(guest, box + 4)), float(integerFrom(guest, box + 8)),
              float(integerFrom(guest, box + 12)));
    return quad;
}

namespace
{

/* The replay bar: its fixed quads at their shares of the box's width and height. */
constexpr u32 kFixedQuads = 0x724790;
constexpr u32 kBarQuads = 21;

struct ReplayBar
{
    float left, top, width, height;
};

/* A quad all of the bar's height, `start` of its width in and `span` wide. */
void placeBarColumn(Guest& guest, const ReplayBar& bar, u32 quad, u32 start, u32 span)
{
    const Float left = fp::add(fp::mul(bar.width, floatFrom(guest, start)), bar.left);
    const Float right = fp::add(fp::mul(bar.width, floatFrom(guest, span)), left);
    placeQuad(guest, quad, float(left), bar.top, float(right), float(fp::add(bar.top, bar.height)));
}

/* A quad by four shares, floats of the image: where it starts across and down, how wide and
 * how high. */
struct BarShares
{
    u32 across, down, wide, high;
};

void placeBarPart(Guest& guest, const ReplayBar& bar, u32 quad, const BarShares& shares)
{
    const float left = float(fp::add(fp::mul(bar.width, floatFrom(guest, shares.across)), bar.left));
    const float top = float(fp::add(fp::mul(bar.height, floatFrom(guest, shares.down)), bar.top));
    const float right = float(fp::add(fp::mul(bar.width, floatFrom(guest, shares.wide)), Float(left)));
    const float bottom = float(fp::add(fp::mul(bar.height, floatFrom(guest, shares.high)), Float(top)));
    placeQuad(guest, quad, left, top, right, bottom);
}

}  // namespace

/* eax the view: the first view's only.  Another's leaves eax its boxes' offset. */
nfs::u32 frontend::hudz::layReplayBar(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    constexpr u32 kFirstColumnEnd = 0x53b530, kMiddleColumn = 0x53b534, kLastColumn = 0x53b538;
    constexpr u32 kUpperDown = 0x53b540, kUpperWide = 0x53b544, kUpperHigh = 0x53b548;
    constexpr u32 kLowerDown = 0x53b558, kLowerWide = 0x53b55c, kLowerHigh = 0x53b560;
    constexpr BarShares kParts[] = {
        { 0x53b53c, kUpperDown, kUpperWide, kUpperHigh },       // 3
        { 0x53b54c, kUpperDown, 0x53b550, kUpperHigh },         // 4, 5: pause and play
        { 0x53b54c, kUpperDown, 0x53b550, kUpperHigh },
        { 0x53b554, kLowerDown, kLowerWide, kLowerHigh },       // 6 to 9: the speeds
        { 0x53b554, kLowerDown, kLowerWide, kLowerHigh },
        { 0x53b564, kLowerDown, 0x53b568, kLowerHigh },
        { 0x53b56c, kLowerDown, 0x53b570, kLowerHigh },
        { 0x53b574, kUpperDown, kUpperWide, kUpperHigh },       // 10
        { kFirstColumnEnd, 0x53b578, 0x53b57c, 0x53b580 },      // 11
    };
    constexpr BarShares kCameraPart = { kLowerWide, kUpperDown, 0x53b584, 0x53b588 };  // 12 on: the cameras

    if (view != 0)
        return view * kBoxesPerView;
    const u32 box = boxOf(view, kReplayBar);
    const float left = float(integerFrom(guest, box));
    const float top = float(integerFrom(guest, box + 4));
    const float width = float(fp::sub(integerFrom(guest, box + 8), Float(left)));
    const float height = float(fp::sub(integerFrom(guest, box + 12), Float(top)));
    const ReplayBar bar = { left, top, width, height };

    placeBarColumn(guest, bar, kFixedQuads, kFirstColumnEnd, kMiddleColumn);
    placeQuad(guest, kFixedQuads + kQuadBytes, left, top,
              float(fp::add(fp::mul(bar.width, floatFrom(guest, kFirstColumnEnd)), Float(left))),
              float(fp::add(Float(top), Float(height))));
    placeBarColumn(guest, bar, kFixedQuads + 2 * kQuadBytes, kMiddleColumn, kLastColumn);
    constexpr u32 kFirstPart = 3;
    for (u32 number = kFirstPart; number < kBarQuads; ++number)
    {
        const u32 listed = number - kFirstPart;
        placeBarPart(guest, bar, kFixedQuads + number * kQuadBytes, listed < std::size(kParts) ? kParts[listed] : kCameraPart);
    }
    return kFixedQuads + (kBarQuads - 1) * kQuadBytes;
}

namespace
{

/* The grid's parts over its box: the first quad of a view's 13 the whole of it, then the
 * eight parts by the game's tables of offsets and sizes in texels, then four of them again
 * from the tenth quad on. */
void placeGridParts(Guest& guest, u32 quads, u32 firstQuad, u32 count, float width, float height)
{
    constexpr u32 kWholeWidth = 0x55d3b8, kPartWidths = 0x55d3bc;
    constexpr u32 kWholeHeight = 0x55d3dc, kPartHeights = 0x55d3e0;
    constexpr u32 kPartLefts = 0x55d400, kPartTops = 0x55d420;

    const Float across = width, down = height;
    for (u32 i = 0; i < count; ++i)
    {
        const u32 quad = quads + firstQuad + i * kQuadBytes;
        const float left = float(fp::add(fp::div(fp::mul(floatFrom(guest, kPartLefts + 4 * i), across), floatFrom(guest, kWholeWidth)),
                                         floatFrom(guest, quads + kCornerX)));
        guest.setFloat(quad + 3 * kCornerBytes + kCornerX, left);
        guest.setFloat(quad + 0 * kCornerBytes + kCornerX, left);
        const float top = float(fp::add(fp::div(fp::mul(floatFrom(guest, kPartTops + 4 * i), down), floatFrom(guest, kWholeHeight)),
                                        floatFrom(guest, quads + kCornerY)));
        guest.setFloat(quad + 1 * kCornerBytes + kCornerY, top);
        guest.setFloat(quad + 0 * kCornerBytes + kCornerY, top);
        const Float toRight = fp::add(floatFrom(guest, kPartLefts + 4 * i), floatFrom(guest, kPartWidths + 4 * i));
        const float right = float(fp::add(fp::div(fp::mul(across, toRight), floatFrom(guest, kWholeWidth)), floatFrom(guest, quads + kCornerX)));
        guest.setFloat(quad + 2 * kCornerBytes + kCornerX, right);
        guest.setFloat(quad + 1 * kCornerBytes + kCornerX, right);
        const Float toBottom = fp::add(floatFrom(guest, kPartTops + 4 * i), floatFrom(guest, kPartHeights + 4 * i));
        const float bottom = float(fp::add(fp::div(fp::mul(down, toBottom), floatFrom(guest, kWholeHeight)), floatFrom(guest, quads + kCornerY)));
        guest.setFloat(quad + 3 * kCornerBytes + kCornerY, bottom);
        guest.setFloat(quad + 2 * kCornerBytes + kCornerY, bottom);
    }
}

}  // namespace

nfs::u32 frontend::hudz::layGrid(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    constexpr u32 kParts = 8, kPartsAgain = 4;

    const u32 quads = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
    const u32 box = boxOf(view, kGrid);
    const u32 left = guest.dword(box), top = guest.dword(box + 4), right = guest.dword(box + 8), bottom = guest.dword(box + 12);
    const float width = float(Float(s32(right - left)));
    const float height = float(Float(s32(bottom - top)));
    placeQuad(guest, quads, float(Float(s32(left))), float(Float(s32(top))), float(Float(s32(right))), float(Float(s32(bottom))));
    placeGridParts(guest, quads, kQuadBytes, kParts, width, height);
    placeGridParts(guest, quads, (1 + kParts) * kQuadBytes, kPartsAgain, width, height);
    return quads;
}

/* ---------------------------------------------------------------------------
 * Small ones
 * ------------------------------------------------------------------------- */

/* The map's small form: a map at all, and its rectangle at most half the screen each way. */
nfs::u32 frontend::hudz::mapPaneIsSmall(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr u32 kHalfScreen = 0x53b5c0;
    const u32 seat = seatOf(guest, view);
    const u32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, view));
    const HudRecord& record = gameSettings(guest).hudAt(formOffset(seat, form));
    if (record.mapStyle == 0)
        return 0;
    const HudRect& rectangle = record.rects[form == 0 ? kMap : kCopMap];
    if (isAbove(fp::sub(Float(rectangle.right), Float(rectangle.left)), floatFrom(guest, kHalfScreen)))
        return 0;
    if (isAbove(fp::sub(Float(rectangle.bottom), Float(rectangle.top)), floatFrom(guest, kHalfScreen)))
        return 0;
    return 1;
}

namespace
{

/* The grid's parts: which one is highlighted, by view, -1 none; their colours. */
constexpr u32 kGridHighlights = 0x55d338;
constexpr u32 kGridColours = 0x55d340;

}  // namespace

/* eax the part to highlight, edx the view, ebx a colour, 0 for the part's own.  The grid's
 * whole and that part leave the queue and come back, on the cards that queue them (voodoo2a
 * among them); its debug line calls it "speech set".  ebx is left the part, edx what it last
 * held, eax what it last held too: sub_42d500 leaves them alone. */
nfs::u32 frontend::hudz::setGridHighlight(App* app, ThreadState& ts, u32 part, u32 view, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 8);
    constexpr u32 kCornersEach = kGridQuadsEach * 4;

    u32 eax = part;
    const u32 highlight = kGridHighlights + view * 4;
    const s32 previous = s32(guest.dword(highlight));
    if (previous >= 0 && (guest.byte(kDisplayFlags) & kCardWithAlpha) == 0)
    {
        const u32 corners = guest.dword(kGridCorners) + view * kCornersEach;
        unqueueQuad(guest, corners);
        eax = unqueueQuad(guest, corners + (u32(previous) + 1) * 4);
    }
    guest.setDword(highlight, part);
    if (s32(part) >= 0)
    {
        const u32 first = guest.dword(kGridCorners);
        const u32 corners = first + view * kCornersEach;
        const u32 partCorners = corners + (part + 1) * 4;
        const u32 whole = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
        const u32 itsQuad = whole + (partCorners - first) * kCornerBytes;
        const u32 shown = colour != 0 ? colour : guest.dword(kGridColours + part * 4);
        for (u32 corner = 0; corner < 4; ++corner)
            guest.setDword(whole + corner * kCornerBytes + kCornerColour, shown);
        for (u32 corner = 0; corner < 4; ++corner)
            guest.setDword(itsQuad + corner * kCornerBytes + kCornerColour, shown);
        eax = whole;
        if ((guest.byte(kDisplayFlags) & kCardWithAlpha) == 0)
        {
            queueQuad(guest, corners);
            eax = queueQuad(guest, partCorners);
        }
    }
    // The game's code hands it "speech set %d\n" (0x53b5c4) and the part on the stack and the eax it
    // goes out with, which it does not read: it does nothing, eax left as it is.
    fe::direct<0x42d500, raceload::noHook>(app, ts);
    return eax;
}

namespace
{

/* A corner's clip code as sub_4c12e0 wants it: the screen's limits are compared with the
 * float's bits as an integer, a negative one being off that side.  16 past the far plane. */
std::uint8_t clipCodeOf(const Guest& guest, u32 corner)
{
    constexpr u32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504, kClipRight = 0x7d350c;
    const s32 depth = s32(guest.dword(corner + kCornerDepth));
    if (depth < 0 || depth >= s32(guest.dword(kClipFar)))
        return 0x10;
    std::uint8_t code = 0;
    const s32 y = s32(guest.dword(corner + kCornerY));
    if (y < 0 || y < s32(guest.dword(kClipTop)))
        code |= 8;
    else if (y > s32(guest.dword(kClipBottom)))
        code |= 4;
    const s32 x = s32(guest.dword(corner + kCornerX));
    if (x < 0 || x < s32(guest.dword(kClipLeft)))
        code |= 1;
    else if (x > s32(guest.dword(kClipRight)))
        code |= 2;
    return code;
}

}  // namespace

/* The grid's whole and its highlighted part, if one is: their clip codes, and the two quads
 * to the clipper.  eax is left the view when none is. */
nfs::u32 frontend::hudz::drawGridHighlight(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x20);
    constexpr u32 kTextureState = 1;
    const s32 part = s32(guest.dword(kGridHighlights + view * 4));
    if (part < 0)
        return view;
    const u32 whole = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
    const u32 itsQuad = whole + (u32(part) + 1) * kQuadBytes;
    fe::direct<0x431900, graphics::sendState>(app, ts, kTextureState, guest.dword(kDialQueue));
    for (const u32 quad : { whole, itsQuad })
        for (u32 corner = 0; corner < 4; ++corner)
        {
            const u32 at = quad + corner * kCornerBytes;
            guest.setByte(at + kCornerClip, clipCodeOf(guest, at));
        }
    fe::direct<0x4c12e0, vertices::quadClipped>(guest.app(), guest.ts(), whole, whole + kCornerBytes, whole + 2 * kCornerBytes, whole + 3 * kCornerBytes);
    return fe::direct<0x4c12e0, vertices::quadClipped>(guest.app(), guest.ts(), itsQuad, itsQuad + kCornerBytes, itsQuad + 2 * kCornerBytes, itsQuad + 3 * kCornerBytes);
}


/* The replay bar's lit quads by its state: the camera, paused or not, the speed, the third
 * of the fixed ones; the button highlighted.  A camera or a speed past the bar's hands on
 * what eax or ebx held. */
nfs::u32 frontend::hudz::drawReplayBar(App* app, ThreadState& ts, u32 eax, u32 ebx)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    constexpr u32 kPanelPaused = 0x7cd948;
    constexpr u32 kPanelSpeed = 0x7cd94c;
    constexpr u32 kPanelCamera = 0x7cd954;
    constexpr u32 kPanelButton = 0x7cd958;
    constexpr u32 kFirstCameraQuad = 12, kCameraCount = 9;
    constexpr u32 kFirstSpeedQuad = 6, kSpeedCount = 4;
    constexpr u32 kPlayQuad = 4, kPausedQuad = 5, kFixedQuad = 3;

    const u32 camera = guest.dword(kPanelCamera);
    const u32 cameraQuad = camera < kCameraCount ? kFirstCameraQuad + camera : eax;
    const u32 playQuad = guest.dword(kPanelPaused) != 0 ? kPausedQuad : kPlayQuad;
    const u32 speed = guest.dword(kPanelSpeed);
    const u32 speedQuad = speed < kSpeedCount ? kFirstSpeedQuad + speed : ebx;
    return *fe::direct<0x48c7c0, render::startLightsDrawn>(app, ts, cameraQuad, playQuad, speedQuad, kFixedQuad, guest.dword(kPanelButton));
}

namespace
{

/* A queue's layout: its first word its texture. */
constexpr u32 kQueueTaken = 0x08;
constexpr u32 kQueueFirstVertex = 0x10;

}  // namespace

/* A queue made ready: its 4096 vertices at their depth, nothing taken.  edx is left their
 * count. */
nfs::u32 frontend::hudz::queueStart(App* app, ThreadState& ts, u32 queue, u32 texture)
{
    Guest guest(app, ts);
    constexpr u32 kQueueMost = 0x04, kQueueQueued = 0x0c;
    constexpr u32 kVertices = 0x1000;
    constexpr u32 kVertexZ = 0x08, kVertexW = 0x0c, kVertexFlags = 0x14;
    constexpr u32 kFlatZ = 0x37800080, kFlatW = 0x3f7fff00;

    guest.setDword(queue + kQueueTaken, 0);
    guest.setDword(queue + kQueueQueued, 0);
    guest.setDword(queue + kQueueMost, kVertices - 1);
    guest.setDword(queue, texture);
    for (u32 i = 0; i < kVertices; ++i)
    {
        const u32 vertex = queue + kQueueFirstVertex + i * kCornerBytes;
        guest.setDword(vertex + kVertexZ, kFlatZ);
        guest.setDword(vertex + kVertexW, kFlatW);
        guest.setDword(vertex + kVertexFlags, 0);
    }
    return queue;
}


/* eax a queue, edx how many corners.  Handed back: their number in the queue through ecx,
 * and where they are through ebx.  ecx is left their offset. */
nfs::u32 frontend::hudz::queueTakeCorners(App* app, ThreadState& ts, u32 queue, u32 count, u32 cornersOut, u32 numberOut)
{
    Guest guest(app, ts);
    const u32 number = guest.dword(queue + kQueueTaken);
    guest.setDword(numberOut, number);
    const u32 offset = number * kCornerBytes;
    guest.setDword(cornersOut, queue + kQueueFirstVertex + offset);
    guest.setDword(queue + kQueueTaken, guest.dword(queue + kQueueTaken) + count);
    return queue;
}


/* The quads a view has queued, each out of the dials' queue, the count run down. */
nfs::u32 frontend::hudz::unqueueViewQuads(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const u32 count = kViewQuadCounts + view * 4;
    u32 corner = guest.dword(kViewCorners) + view * kViewCornersEach;
    for (u32 left = guest.dword(count); left != 0; left = guest.dword(count))
    {
        guest.setDword(count, left - 1);
        unqueueQuad(guest, corner);
        corner += 4;
    }
    return view * 4;
}

namespace
{

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

}

/* ---------------------------------------------------------------------------
 * The port's functions
 * ------------------------------------------------------------------------- */

/* sub_481570: a form of the HUD's laying out with nothing to lay; the view left in eax. */
nfs::u32 frontend::hudz::layNothing(App*, ThreadState&, u32 view)
{
    return view;
}

/* sub_484030: the frames' queue flushed. */
nfs::u32 frontend::hudz::flushFrameQueue(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return u32(*fe::direct<0x49be40, port::text::quadsFlushed>(app, ts, ts.sp - 4, kFrameQueue));
}

nfs::u32 frontend::hudEditElementFits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_459670", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 tried = args.eax, view = args.edx, moved = args.ebx, mode = args.ecx;
    const u32 blockerOut = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudz::elementFits(app, ts, tried, view, moved, mode, blockerOut);
    });
}

nfs::u32 frontend::hudEditElementMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_459db0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::elementMoved(app, ts, item); });
}

nfs::u32 frontend::hudUnqueueViewQuads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4808b0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::unqueueViewQuads(app, ts, view); });
}

nfs::u32 frontend::hudSetAlignments(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_480a00", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::setAlignments(app, ts, view); });
}

nfs::u32 frontend::hudPlaceDialArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_481170", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 seat = args.eax, form = args.edx, corners = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::placeDialArt(app, ts, seat, form, corners); });
}

nfs::u32 frontend::hudLayMarkersFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_481290", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 view = args.eax, width = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::layMarkersFrame(app, ts, view, width); });
}

nfs::u32 frontend::hudLayPanel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_481510", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::layPanel(app, ts, view); });
}

nfs::u32 frontend::hudLayNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_481570", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::layNothing(app, ts, view); });
}

nfs::u32 frontend::hudLayReplayBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4815d0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::layReplayBar(app, ts, view); });
}

nfs::u32 frontend::hudLayGrid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4818b0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::layGrid(app, ts, view); });
}

nfs::u32 frontend::hudMapPaneIsSmall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_481f90", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::mapPaneIsSmall(app, ts, view); });
}

nfs::u32 frontend::hudSetGridHighlight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_482030", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 part = args.eax, view = args.edx, colour = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::setGridHighlight(app, ts, part, view, colour); });
}

nfs::u32 frontend::hudDrawGridHighlight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_482190", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::drawGridHighlight(app, ts, view); });
}

nfs::u32 frontend::hudDrawReplayBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_482880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 eax = args.eax, ebx = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::drawReplayBar(app, ts, eax, ebx); });
}

nfs::u32 frontend::hudFlushFrameQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_484030", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::flushFrameQueue(app, ts); });
}

nfs::u32 frontend::hudQueueStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bc80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 queue = args.eax, texture = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudz::queueStart(app, ts, queue, texture); });
}

nfs::u32 frontend::hudQueueTakeCorners(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bd10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 queue = args.eax, count = args.edx, cornersOut = args.ebx, numberOut = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return hudz::queueTakeCorners(app, ts, queue, count, cornersOut, numberOut);
    });
}

}
