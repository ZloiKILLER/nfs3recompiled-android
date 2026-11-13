#include "native_frontend_hudz.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_hudr.h"
#include "native_frontend_rendb.h"
#include "native_graphics.h"
#include "native_raceload.h"
#include "port_loop.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <cstdint>
#include <cstring>
#include <iterator>

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
 * its own where the game's function has one, and an adapter under the game's name for the calls by
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

/* The port's own (nfs3hp_main.cpp), from tools/apply_hud_editor.py: the layout as laid
 * out, not as a wide screen draws it, and the layouts as measured drawn for the length
 * of the test whether an element fits, cpu holding the test's arguments. */
void hudLayoutDesigned(win32::WinApplication* app, x86::reg32 rect, x86::reg32 element);
void hudLayoutsDrawn(win32::WinApplication* app, x86::CPU& cpu, bool drawn);

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg16;
using x86::sreg32;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }  // ja; false unordered
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }  // jb; true unordered

bool isAbove(Float a, Float b) { return jumpsAbove(fp::compare(a, b)); }
bool isBelow(Float a, Float b) { return jumpsBelow(fp::compare(a, b)); }

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, reg32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, reg32 address) { return Float(sreg32(guest.dword(address))); }

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The game's functions, by address.
constexpr reg32 kBuildFrame = 0x47f650;        // eax the corners, ebx, edx, stack, ecx its inside
constexpr reg32 kFlushBand = 0x4800a0;         // eax a view
constexpr reg32 kDrawLights = 0x48c7c0;        // eax, edx, ebx, ecx quads of the bar, the stack the button
constexpr reg32 kQueueQuad = 0x49bd30;         // eax a queue, edx, ebx, ecx and the stack its corners
constexpr reg32 kUnqueueQuad = 0x49bdc0;       // likewise
constexpr reg32 kFlushQueue = 0x49be40;        // eax a queue
constexpr reg32 kClipQuad = 0x4c12e0;          // eax, edx, ebx, ecx its four vertices

// The numbers of the screen.
constexpr reg32 kViewWidth = 0x7cdac8, kViewHeight = 0x7cdacc;
constexpr reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kCardWithAlpha = 2;     // not voodoo2a's

// A car, as sub_422bd0 gives it.
constexpr reg32 kCarFlags = 0x200;             // byte; kCarIsCop: the cop's form of the HUD
constexpr std::uint8_t kCarIsCop = 0x20;

// The HUD's records, by seat and by form; a seat is a view, one on in a split screen.
constexpr reg32 kRecordsPerSeat = 0x348;
constexpr reg32 kRecordsPerForm = 0x1a4;
constexpr reg32 kRectangles = 0x24;            // 23 of 16 bytes, fractions of the screen
constexpr reg32 kRectangleBytes = 16;
constexpr reg32 kRectangleTop = 0, kRectangleLeft = 4, kRectangleBottom = 8, kRectangleRight = 12;

// The kinds of element these name, a rectangle each.
constexpr reg32 kElements = 23;
constexpr reg32 kSpeedFigures = 0;
constexpr reg32 kSpeedDial = 1;
constexpr reg32 kRevFigures = 2;
constexpr reg32 kRevDial = 3;
constexpr reg32 kLapTimeText = 5;
constexpr reg32 kGrid = 6;
constexpr reg32 kReplayBar = 8;
constexpr reg32 kKind9 = 9;
constexpr reg32 kFirstOfModes = 10;            // from here to kMarkers one mode's, past it the other's
constexpr reg32 kStandings = 13;
constexpr reg32 kMap = 17;
constexpr reg32 kMarkers = 18;
constexpr reg32 kLabelRows = 20;
constexpr reg32 kPanel = 21;
constexpr reg32 kCopMap = 22;
// After the 23, the map's place while it is the small form (sub_459db0 keeps it).
constexpr reg32 kSmallMapRectangle = kRectangles + kElements * kRectangleBytes;

// The pixel boxes of a view's elements: left, top, right, bottom.
constexpr reg32 kBoxes = 0x749758;
constexpr reg32 kBoxBytes = 16;
constexpr reg32 kBoxesPerView = kElements * kBoxBytes;

// How a view's texts are aligned (sub_4897f0), 23 to a view.
constexpr reg32 kAlignments = 0x725690;
constexpr reg32 kAlignmentsPerView = kElements * 4;
constexpr reg32 kCentred = 8;
constexpr reg32 kCentredToo = 0x10;

// Vertices: 0x20 bytes, four to a quad.
constexpr reg32 kCornerBytes = 0x20;
constexpr reg32 kQuadBytes = 4 * kCornerBytes;
constexpr reg32 kCornerX = 0, kCornerY = 4, kCornerDepth = 0xc, kCornerColour = 0x10, kCornerClip = 0x14;
constexpr reg32 kCornerU = 0x18, kCornerV = 0x1c;

// The queues of quads and the batches of corners in them (native_frontend_rendb.cpp).
constexpr reg32 kDialQueue = 0x749a38;
constexpr reg32 kFrameQueue = 0x700248;
constexpr reg32 kViewQuadCounts = 0x700240;    // by view: the quads it has queued
constexpr reg32 kViewCorners = 0x791a88;       // the queue's numbers of them, 7 quads to a view
constexpr reg32 kViewCornersEach = 7 * 4;
constexpr reg32 kPanelQuads = 0x791ae0;        // a quad to a view
constexpr reg32 kFrameQuads = 0x791ae4;        // 8 quads to a view
constexpr reg32 kFrameQuadsEach = 8 * kQuadBytes;
constexpr reg32 kGridQuads = 0x791a58;         // 13 quads to a view
constexpr reg32 kGridCorners = 0x791a84;       // the queue's numbers of them
constexpr reg32 kGridQuadsEach = 13;

/* Whether a car is a cop's, as 0 or 1: the form of the HUD its view has. */
reg32 formOf(const Guest& guest, reg32 car)
{
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

/* The seat of a view, by the screen's split. */
reg32 seatOf(const Guest& guest, reg32 view)
{
    const reg32 players = gameSettings(guest).playMode;
    return view + (players == 1 ? players : 0);
}

/* The offset of a seat's form in the records. */
constexpr reg32 formOffset(reg32 seat, reg32 form)
{
    return seat * kRecordsPerSeat + form * kRecordsPerForm;
}

/* A view's box for a kind of element. */
constexpr reg32 boxOf(reg32 view, reg32 kind)
{
    return kBoxes + view * kBoxesPerView + kind * kBoxBytes;
}

reg32 queueQuad(Guest& guest, reg32 first)
{
    return guest.call(kQueueQuad, args({ kDialQueue, first, first + 1, first + 2 }, { first + 3 }));
}

reg32 unqueueQuad(Guest& guest, reg32 first)
{
    return guest.call(kUnqueueQuad, args({ kDialQueue, first, first + 1, first + 2 }, { first + 3 }));
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

Fractions fractionsAt(const Guest& guest, reg32 at)
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
    constexpr reg32 kShareOfWidth = 0x539bac;
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
bool countsAgainst(reg32 moved, reg32 other, reg32 players, reg32 mode)
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
x86::reg32 frontend::hudz::elementFits(App* app, CPU& cpu, reg32 tried, reg32 view, reg32 moved, reg32 mode, reg32 blockerOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x100);
    constexpr sreg32 kHalfBits = 0x3f000000, kOneBits = 0x3f800000;
    constexpr reg32 kItemOfElement = 0x1f;     // the widget of an element on the editor's screen
    constexpr reg32 kItemKind = 0x3c;
    constexpr reg32 kItemsOfSecondView = 23;   // what its items' kinds count on from
    // Two locals of the frame the test reads on a pass with nothing of its own to weigh.
    constexpr reg32 kStaleBottom = 0xf4, kStaleRight = 0xf8;

    const reg32 players = gameSettings(guest).playMode;
    const reg32 onSplit = players == 1 ? players : 0;
    guest.leaveEcx(players);
    guest.leaveEbx(onSplit);
    const reg32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, view));
    const reg32 seat = onSplit + view;
    reg32 leftInEcx = cpu.ecx;

    const auto leave = [&](reg32 ebx, reg32 ecx, reg32 answer) {
        guest.leaveEbx(ebx);
        guest.leaveEcx(ecx);
        guest.leaveEdx(answer);
        return answer;
    };

    // Wholly on the screen.  The upper limits are compared as the bits they are.
    const Fractions wanted = fractionsAt(guest, tried);
    if (isAbove(0.0, wanted.top) || isAbove(0.0, wanted.bottom) || isAbove(0.0, wanted.left) || isAbove(0.0, wanted.right))
        return leave(seat, leftInEcx, 0);
    const auto bitsAt = [&](reg32 edge) { return sreg32(guest.dword(tried + edge)); };
    if (bitsAt(kRectangleTop) > kOneBits || bitsAt(kRectangleBottom) > kOneBits || bitsAt(kRectangleLeft) > kOneBits
        || bitsAt(kRectangleRight) > kOneBits)
        return leave(seat, leftInEcx, 0);

    // In its half of a split screen, but for the replay bar.
    if (gameSettings(guest).playMode == 1)
    {
        const bool outside = seat == 1 ? bitsAt(kRectangleBottom) > kHalfBits : bitsAt(kRectangleTop) < kHalfBits;
        if (outside && moved != kReplayBar)
            return leave(seat, leftInEcx, 0);
    }

    // Nothing is in the way of the maps.
    if (moved == kMap || moved == kCopMap)
        return leave(seat, leftInEcx, 1);

    const reg32 layout = formOffset(seat, form);
    const auto fractionsOf = [&](reg32 kind) {
        return fractionsAt(gameSettings(guest).hudAt(layout).rects[kind]);
    };
    // What the last pass weighed of the second parts, its bottom and its right; on a first
    // pass, what the stack holds there.
    float staleBottom = guest.floatAt(guest.frame() + kStaleBottom);
    float staleRight = guest.floatAt(guest.frame() + kStaleRight);
    for (reg32 other = 0; other < kElements; ++other)
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

        // What ecx went on with: the rectangle's offset for the two, else the layout's.
        leftInEcx = moved == kSpeedFigures || moved == kSpeedDial ? layout + other * kRectangleBytes : layout;

        if (!hasSize(shared) && !hasSize(second))
            continue;

        // In the way: which item of the screen shows it, if any.
        const reg32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, cpu);
        const reg32 itsKind = seat == 2 ? other + kItemsOfSecondView : other;
        for (sreg32 number = sreg16(guest.word(menu + fe::menu::kFirstItem));; ++number)
        {
            const reg32 offset = reg32(number) * fe::item::kSize;
            if (guest.dword(fe::item::kTable + offset) == 0)
                return leave(reg32(number), offset, 0);
            const reg32 item = fe::item::kTable + offset;
            if (item != 0 && guest.dword(item) == kItemOfElement && guest.dword(item + kItemKind) == itsKind)
            {
                guest.setDword(blockerOut, item);
                return leave(reg32(number), item, 0);
            }
        }
    }
    return leave(seat, leftInEcx, 1);
}

namespace
{

/* ---------------------------------------------------------------------------
 * A drag put into the layout: sub_459db0
 * ------------------------------------------------------------------------- */

/* The item of an element on the editor's screen. */
namespace elementItem
{
constexpr reg32 kPixelX = 0x06;                // word
constexpr reg32 kPixelY = 0x08;                // word
constexpr reg32 kKind = 0x3c;                  // from 23 on, the second view's
constexpr reg32 kPlace = 0x40;                 // its rectangle, fractions of the screen
constexpr reg32 kLastGood = 0x50;              // the last place that fitted
constexpr reg32 kPixelRight = 0x60;
constexpr reg32 kPixelBottom = 0x64;
constexpr reg32 kClear = 0x70;                 // 0 on the item in another's way
constexpr reg32 kBlocker = 0x74;               // the item in this one's way
constexpr reg32 kKindsOfView = 23;
}

/* The item's pixels as fractions of the screen. */
void placeFromPixels(Guest& guest, reg32 item)
{
    const Float x = Float(sreg16(guest.word(item + elementItem::kPixelX)));
    const Float perWidth = fp::div(1.0, integerFrom(guest, kViewWidth));
    const Float y = Float(sreg16(guest.word(item + elementItem::kPixelY)));
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
x86::reg32 frontend::hudz::elementMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x10);
    constexpr reg32 kEditModes = 0x677df0;     // by view
    constexpr reg32 kMostOfScreen = 0x539bb4;

    const reg32 kind = guest.dword(item + elementItem::kKind);
    const reg32 view = sreg32(kind) >= sreg32(elementItem::kKindsOfView) ? 1 : 0;
    const reg32 element = view ? kind - elementItem::kKindsOfView : kind;
    const reg32 place = item + elementItem::kPlace;

    const reg32 players = gameSettings(guest).playMode;
    placeFromPixels(guest, item);
    reg32 layout;
    if (players == 1)
        layout = formOffset(view + 1, formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, view)));
    else
        layout = formOffset(0, formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, reg32(0))));

    const reg32 rectangle = SETTINGS_ADDRESS(hud) + kRectangles + layout + element * kRectangleBytes;
    copyDwords(guest, rectangle, place, 4);
    hudLayoutDesigned(app, rectangle, element); /* port: the layout as laid out, not as drawn */

    guest::direct<0x4853c0, hudr::rebuildElement>(app, cpu, view, element);
    guest.call(kFlushBand, args({ view }));

    if ((element == kMap || element == kCopMap)
        && isBelow(fp::sub(floatFrom(guest, place + kRectangleRight), floatFrom(guest, place + kRectangleLeft)), floatFrom(guest, kMostOfScreen))
        && isBelow(fp::sub(floatFrom(guest, place + kRectangleBottom), floatFrom(guest, place + kRectangleTop)), floatFrom(guest, kMostOfScreen)))
    {
        const reg32 seat = seatOf(guest, view);
        const reg32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, view));
        copyDwords(guest, SETTINGS_ADDRESS(hud) + kSmallMapRectangle + formOffset(seat, form), place, 4);
    }

    // The test hands the item in the way to the bottom of the frame, and writes nothing there
    // when no item is: the original then goes on with what the stack held.
    const reg32 blockerAt = guest.frame();
    const reg32 mode = guest.dword(kEditModes + 4 * view);
    cpu.eax = place;
    cpu.edx = view;
    cpu.ebx = element;
    cpu.ecx = mode;
    hudLayoutsDrawn(app, cpu, true); /* port: measured against what is drawn */
    const reg32 fits = guest::direct<0x459670, elementFits>(app, cpu, place, view, element, mode, blockerAt);
    cpu.eax = fits;  // the answer in eax, as the call by address leaves it, for the port's trace
    hudLayoutsDrawn(app, cpu, false);

    const reg32 blocker = guest.dword(item + elementItem::kBlocker);
    if (blocker != 0)
        guest.setDword(blocker + elementItem::kClear, 1);
    if (fits != 0)
    {
        if (blocker != 0)
            guest.setDword(item + elementItem::kBlocker, 0);
        copyDwords(guest, item + elementItem::kLastGood, place, 4);
        return 1;
    }
    const reg32 newBlocker = guest.dword(blockerAt);
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
 * (tools/apply_hud_editor.py: the text jumped the width of its margin as an element was
 * dragged), and what those comparisons came to is read nowhere else, so they are gone;
 * what stays of them is their asking sub_47bc20 about each of the others, twice.  The
 * standings' two layouts keep theirs, 8 or 16, both centred as sub_4897f0 reads them: 16
 * when what the table leaves below it is less than where its top stands.  eax is left the
 * last rectangle's offset, the comparison's status word in its low half. */
x86::reg32 frontend::hudz::setAlignments(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x98);
    constexpr reg32 kHalfScreen = 0x53b510, kLessHalfScreen = 0x53b514;

    const reg32 seat = seatOf(guest, view);
    const reg32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, view));
    const reg32 alignments = kAlignments + view * kAlignmentsPerView;

    for (reg32 kind = 0; kind < kElements; ++kind)
    {
        if (fe::direct<0x47bc20, rendb::kindIsText>(app, cpu, kind) == 0)
            continue;
        for (reg32 pass = 0; pass < 2; ++pass)
            for (reg32 other = 0; other < kElements; ++other)
                if (other != kind)
                    fe::direct<0x47bc20, rendb::kindIsText>(app, cpu, other);
        guest.setDword(alignments + kind * 4, kCentred); /* port: an element's text stays centred in it, was the edge it stood nearest */
    }

    reg32 result = 0;
    for (const reg32 kind : { kStandings, kLabelRows })
    {
        const reg32 offset = formOffset(seat, form) + kind * kRectangleBytes;
        result = offset;
        const HudRect& rectangle = gameSettings(guest).hudAt(formOffset(seat, form)).rects[kind];
        const Float top = Float(rectangle.top);
        const Float bottom = Float(rectangle.bottom);
        reg16 status = 0;
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
void placeQuad(Guest& guest, reg32 quad, float left, float top, float right, float bottom)
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
x86::reg32 frontend::hudz::placeDialArt(App* app, CPU& cpu, reg32 seat, reg32 form, reg32 corners)
{
    Guest guest(app, cpu);
    constexpr reg32 kEdge = 0x3b000000, kFirstEnd = 0x3e7a0000, kSecondStart = 0x3e810000, kSecondEnd = 0x3efd0000;

    const reg32 offset = formOffset(seat, form);
    const reg32 dialForm = gameSettings(guest).hudAt(offset).speedometerForm;
    guest.leaveEdx(dialForm);
    const bool upper = dialForm == 1 || dialForm == 2;
    const reg32 top = upper ? kEdge : kSecondStart, bottom = upper ? kFirstEnd : kSecondEnd;
    const bool second = gameSettings(guest).night != 0;
    const reg32 left = second ? kSecondStart : kEdge, right = second ? kSecondEnd : kFirstEnd;

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
x86::reg32 frontend::hudz::layMarkersFrame(App* app, CPU& cpu, reg32 view, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 box = boxOf(view, kMarkers);
    const reg32 left = guest.dword(box), top = guest.dword(box + 4), right = guest.dword(box + 8), bottom = guest.dword(box + 12);
    const reg32 quads = view * kFrameQuadsEach + guest.dword(kFrameQuads);
    return guest.call(kBuildFrame, args({ quads, top, left, bottom }, { right }));
}


/* The panel's quad over its box. */
x86::reg32 frontend::hudz::layPanel(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const reg32 quad = guest.dword(kPanelQuads) + view * kQuadBytes;
    const reg32 box = boxOf(view, kPanel);
    placeQuad(guest, quad, float(integerFrom(guest, box)), float(integerFrom(guest, box + 4)), float(integerFrom(guest, box + 8)),
              float(integerFrom(guest, box + 12)));
    return quad;
}

namespace
{

/* The replay bar: its fixed quads at their shares of the box's width and height. */
constexpr reg32 kFixedQuads = 0x724790;
constexpr reg32 kBarQuads = 21;

struct ReplayBar
{
    float left, top, width, height;
};

/* A quad all of the bar's height, `start` of its width in and `span` wide. */
void placeBarColumn(Guest& guest, const ReplayBar& bar, reg32 quad, reg32 start, reg32 span)
{
    const Float left = fp::add(fp::mul(bar.width, floatFrom(guest, start)), bar.left);
    const Float right = fp::add(fp::mul(bar.width, floatFrom(guest, span)), left);
    placeQuad(guest, quad, float(left), bar.top, float(right), float(fp::add(bar.top, bar.height)));
}

/* A quad by four shares, floats of the image: where it starts across and down, how wide and
 * how high. */
struct BarShares
{
    reg32 across, down, wide, high;
};

void placeBarPart(Guest& guest, const ReplayBar& bar, reg32 quad, const BarShares& shares)
{
    const float left = float(fp::add(fp::mul(bar.width, floatFrom(guest, shares.across)), bar.left));
    const float top = float(fp::add(fp::mul(bar.height, floatFrom(guest, shares.down)), bar.top));
    const float right = float(fp::add(fp::mul(bar.width, floatFrom(guest, shares.wide)), Float(left)));
    const float bottom = float(fp::add(fp::mul(bar.height, floatFrom(guest, shares.high)), Float(top)));
    placeQuad(guest, quad, left, top, right, bottom);
}

}  // namespace

/* eax the view: the first view's only.  Another's leaves eax its boxes' offset. */
x86::reg32 frontend::hudz::layReplayBar(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    constexpr reg32 kFirstColumnEnd = 0x53b530, kMiddleColumn = 0x53b534, kLastColumn = 0x53b538;
    constexpr reg32 kUpperDown = 0x53b540, kUpperWide = 0x53b544, kUpperHigh = 0x53b548;
    constexpr reg32 kLowerDown = 0x53b558, kLowerWide = 0x53b55c, kLowerHigh = 0x53b560;
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
    const reg32 box = boxOf(view, kReplayBar);
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
    constexpr reg32 kFirstPart = 3;
    for (reg32 number = kFirstPart; number < kBarQuads; ++number)
    {
        const reg32 listed = number - kFirstPart;
        placeBarPart(guest, bar, kFixedQuads + number * kQuadBytes, listed < std::size(kParts) ? kParts[listed] : kCameraPart);
    }
    return kFixedQuads + (kBarQuads - 1) * kQuadBytes;
}

namespace
{

/* The grid's parts over its box: the first quad of a view's 13 the whole of it, then the
 * eight parts by the game's tables of offsets and sizes in texels, then four of them again
 * from the tenth quad on. */
void placeGridParts(Guest& guest, reg32 quads, reg32 firstQuad, reg32 count, float width, float height)
{
    constexpr reg32 kWholeWidth = 0x55d3b8, kPartWidths = 0x55d3bc;
    constexpr reg32 kWholeHeight = 0x55d3dc, kPartHeights = 0x55d3e0;
    constexpr reg32 kPartLefts = 0x55d400, kPartTops = 0x55d420;

    const Float across = width, down = height;
    for (reg32 i = 0; i < count; ++i)
    {
        const reg32 quad = quads + firstQuad + i * kQuadBytes;
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

x86::reg32 frontend::hudz::layGrid(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    constexpr reg32 kParts = 8, kPartsAgain = 4;

    const reg32 quads = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
    const reg32 box = boxOf(view, kGrid);
    const reg32 left = guest.dword(box), top = guest.dword(box + 4), right = guest.dword(box + 8), bottom = guest.dword(box + 12);
    const float width = float(Float(sreg32(right - left)));
    const float height = float(Float(sreg32(bottom - top)));
    placeQuad(guest, quads, float(Float(sreg32(left))), float(Float(sreg32(top))), float(Float(sreg32(right))), float(Float(sreg32(bottom))));
    placeGridParts(guest, quads, kQuadBytes, kParts, width, height);
    placeGridParts(guest, quads, (1 + kParts) * kQuadBytes, kPartsAgain, width, height);
    return quads;
}

/* ---------------------------------------------------------------------------
 * Small ones
 * ------------------------------------------------------------------------- */

/* The map's small form: a map at all, and its rectangle at most half the screen each way. */
x86::reg32 frontend::hudz::mapPaneIsSmall(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    constexpr reg32 kHalfScreen = 0x53b5c0;
    const reg32 seat = seatOf(guest, view);
    const reg32 form = formOf(guest, fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, view));
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
constexpr reg32 kGridHighlights = 0x55d338;
constexpr reg32 kGridColours = 0x55d340;

}  // namespace

/* eax the part to highlight, edx the view, ebx a colour, 0 for the part's own.  The grid's
 * whole and that part leave the queue and come back, on the cards that queue them (voodoo2a
 * among them); its debug line calls it "speech set".  ebx is left the part, edx what it last
 * held, eax what it last held too: sub_42d500 leaves them alone. */
x86::reg32 frontend::hudz::setGridHighlight(App* app, CPU& cpu, reg32 part, reg32 view, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 8);
    constexpr reg32 kCornersEach = kGridQuadsEach * 4;

    reg32 eax = part;
    const reg32 highlight = kGridHighlights + view * 4;
    const sreg32 previous = sreg32(guest.dword(highlight));
    if (previous >= 0 && (guest.byte(kDisplayFlags) & kCardWithAlpha) == 0)
    {
        const reg32 corners = guest.dword(kGridCorners) + view * kCornersEach;
        unqueueQuad(guest, corners);
        eax = unqueueQuad(guest, corners + (reg32(previous) + 1) * 4);
    }
    guest.setDword(highlight, part);
    guest.leaveEdx(part);
    if (sreg32(part) >= 0)
    {
        const reg32 first = guest.dword(kGridCorners);
        const reg32 corners = first + view * kCornersEach;
        const reg32 partCorners = corners + (part + 1) * 4;
        const reg32 whole = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
        const reg32 itsQuad = whole + (partCorners - first) * kCornerBytes;
        const reg32 shown = colour != 0 ? colour : guest.dword(kGridColours + part * 4);
        for (reg32 corner = 0; corner < 4; ++corner)
            guest.setDword(whole + corner * kCornerBytes + kCornerColour, shown);
        for (reg32 corner = 0; corner < 4; ++corner)
            guest.setDword(itsQuad + corner * kCornerBytes + kCornerColour, shown);
        guest.leaveEdx(itsQuad);
        eax = whole;
        if ((guest.byte(kDisplayFlags) & kCardWithAlpha) == 0)
        {
            queueQuad(guest, corners);
            eax = queueQuad(guest, partCorners);
        }
    }
    guest.leaveEbx(part);
    // The game's code hands it "speech set %d\n" (0x53b5c4) and the part on the stack and the eax it
    // goes out with, which it does not read: it does nothing, eax left as it is.
    fe::direct<0x42d500, raceload::noHook>(app, cpu);
    return eax;
}

namespace
{

/* A corner's clip code as sub_4c12e0 wants it: the screen's limits are compared with the
 * float's bits as an integer, a negative one being off that side.  16 past the far plane. */
std::uint8_t clipCodeOf(const Guest& guest, reg32 corner)
{
    constexpr reg32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504, kClipRight = 0x7d350c;
    const sreg32 depth = sreg32(guest.dword(corner + kCornerDepth));
    if (depth < 0 || depth >= sreg32(guest.dword(kClipFar)))
        return 0x10;
    std::uint8_t code = 0;
    const sreg32 y = sreg32(guest.dword(corner + kCornerY));
    if (y < 0 || y < sreg32(guest.dword(kClipTop)))
        code |= 8;
    else if (y > sreg32(guest.dword(kClipBottom)))
        code |= 4;
    const sreg32 x = sreg32(guest.dword(corner + kCornerX));
    if (x < 0 || x < sreg32(guest.dword(kClipLeft)))
        code |= 1;
    else if (x > sreg32(guest.dword(kClipRight)))
        code |= 2;
    return code;
}

}  // namespace

/* The grid's whole and its highlighted part, if one is: their clip codes, and the two quads
 * to the clipper.  eax is left the view when none is. */
x86::reg32 frontend::hudz::drawGridHighlight(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x20);
    constexpr reg32 kTextureState = 1;
    const sreg32 part = sreg32(guest.dword(kGridHighlights + view * 4));
    if (part < 0)
        return view;
    const reg32 whole = guest.dword(kGridQuads) + view * kGridQuadsEach * kQuadBytes;
    const reg32 itsQuad = whole + (reg32(part) + 1) * kQuadBytes;
    fe::direct<0x431900, graphics::sendState>(app, cpu, kTextureState, guest.dword(kDialQueue));
    for (const reg32 quad : { whole, itsQuad })
        for (reg32 corner = 0; corner < 4; ++corner)
        {
            const reg32 at = quad + corner * kCornerBytes;
            guest.setByte(at + kCornerClip, clipCodeOf(guest, at));
        }
    guest.call(kClipQuad, args({ whole, whole + kCornerBytes, whole + 2 * kCornerBytes, whole + 3 * kCornerBytes }));
    return guest.call(kClipQuad, args({ itsQuad, itsQuad + kCornerBytes, itsQuad + 2 * kCornerBytes, itsQuad + 3 * kCornerBytes }));
}


/* The replay bar's lit quads by its state: the camera, paused or not, the speed, the third
 * of the fixed ones; the button highlighted.  A camera or a speed past the bar's hands on
 * what eax or ebx held. */
x86::reg32 frontend::hudz::drawReplayBar(App* app, CPU& cpu, reg32 eax, reg32 ebx)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    constexpr reg32 kPanelPaused = 0x7cd948;
    constexpr reg32 kPanelSpeed = 0x7cd94c;
    constexpr reg32 kPanelCamera = 0x7cd954;
    constexpr reg32 kPanelButton = 0x7cd958;
    constexpr reg32 kFirstCameraQuad = 12, kCameraCount = 9;
    constexpr reg32 kFirstSpeedQuad = 6, kSpeedCount = 4;
    constexpr reg32 kPlayQuad = 4, kPausedQuad = 5, kFixedQuad = 3;

    const reg32 camera = guest.dword(kPanelCamera);
    const reg32 cameraQuad = camera < kCameraCount ? kFirstCameraQuad + camera : eax;
    const reg32 playQuad = guest.dword(kPanelPaused) != 0 ? kPausedQuad : kPlayQuad;
    const reg32 speed = guest.dword(kPanelSpeed);
    const reg32 speedQuad = speed < kSpeedCount ? kFirstSpeedQuad + speed : ebx;
    return guest.call(kDrawLights, args({ cameraQuad, playQuad, speedQuad, kFixedQuad }, { guest.dword(kPanelButton) }));
}

namespace
{

/* A queue's layout: its first word its texture. */
constexpr reg32 kQueueTaken = 0x08;
constexpr reg32 kQueueFirstVertex = 0x10;

}  // namespace

/* A queue made ready: its 4096 vertices at their depth, nothing taken.  edx is left their
 * count. */
x86::reg32 frontend::hudz::queueStart(App* app, CPU& cpu, reg32 queue, reg32 texture)
{
    Guest guest(app, cpu);
    constexpr reg32 kQueueMost = 0x04, kQueueQueued = 0x0c;
    constexpr reg32 kVertices = 0x1000;
    constexpr reg32 kVertexZ = 0x08, kVertexW = 0x0c, kVertexFlags = 0x14;
    constexpr reg32 kFlatZ = 0x37800080, kFlatW = 0x3f7fff00;

    guest.setDword(queue + kQueueTaken, 0);
    guest.setDword(queue + kQueueQueued, 0);
    guest.setDword(queue + kQueueMost, kVertices - 1);
    guest.setDword(queue, texture);
    for (reg32 i = 0; i < kVertices; ++i)
    {
        const reg32 vertex = queue + kQueueFirstVertex + i * kCornerBytes;
        guest.setDword(vertex + kVertexZ, kFlatZ);
        guest.setDword(vertex + kVertexW, kFlatW);
        guest.setDword(vertex + kVertexFlags, 0);
    }
    guest.leaveEdx(kVertices);
    return queue;
}


/* eax a queue, edx how many corners.  Handed back: their number in the queue through ecx,
 * and where they are through ebx.  ecx is left their offset. */
x86::reg32 frontend::hudz::queueTakeCorners(App* app, CPU& cpu, reg32 queue, reg32 count, reg32 cornersOut, reg32 numberOut)
{
    Guest guest(app, cpu);
    const reg32 number = guest.dword(queue + kQueueTaken);
    guest.setDword(numberOut, number);
    const reg32 offset = number * kCornerBytes;
    guest.setDword(cornersOut, queue + kQueueFirstVertex + offset);
    guest.setDword(queue + kQueueTaken, guest.dword(queue + kQueueTaken) + count);
    guest.leaveEcx(offset);
    return queue;
}


/* The quads a view has queued, each out of the dials' queue, the count run down. */
x86::reg32 frontend::hudz::unqueueViewQuads(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const reg32 count = kViewQuadCounts + view * 4;
    reg32 corner = guest.dword(kViewCorners) + view * kViewCornersEach;
    for (reg32 left = guest.dword(count); left != 0; left = guest.dword(count))
    {
        guest.setDword(count, left - 1);
        unqueueQuad(guest, corner);
        corner += 4;
    }
    return view * 4;
}

namespace
{

/* A generated function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

}

/* ---------------------------------------------------------------------------
 * The stand-ins
 * ------------------------------------------------------------------------- */

/* sub_481570: a form of the HUD's laying out with nothing to lay; the view left in eax. */
x86::reg32 frontend::hudz::layNothing(App*, CPU&, reg32 view)
{
    return view;
}

/* sub_484030: the frames' queue flushed. */
x86::reg32 frontend::hudz::flushFrameQueue(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest.call(kFlushQueue, args({ kFrameQueue }));
}

void frontend::hudEditElementFits(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_459670", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 tried = cpu.eax, view = cpu.edx, moved = cpu.ebx, mode = cpu.ecx;
    const reg32 blockerOut = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return hudz::elementFits(app, cpu, tried, view, moved, mode, blockerOut);
    });
}

void frontend::hudEditElementMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_459db0", 0, 0, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::elementMoved(app, cpu, item); });
}

void frontend::hudUnqueueViewQuads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4808b0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::unqueueViewQuads(app, cpu, view); });
}

void frontend::hudSetAlignments(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_480a00", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::setAlignments(app, cpu, view); });
}

void frontend::hudPlaceDialArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_481170", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 seat = cpu.eax, form = cpu.edx, corners = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::placeDialArt(app, cpu, seat, form, corners); });
}

void frontend::hudLayMarkersFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_481290", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 view = cpu.eax, width = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::layMarkersFrame(app, cpu, view, width); });
}

void frontend::hudLayPanel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_481510", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::layPanel(app, cpu, view); });
}

void frontend::hudLayNothing(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_481570", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::layNothing(app, cpu, view); });
}

void frontend::hudLayReplayBar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4815d0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::layReplayBar(app, cpu, view); });
}

void frontend::hudLayGrid(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4818b0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::layGrid(app, cpu, view); });
}

void frontend::hudMapPaneIsSmall(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_481f90", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::mapPaneIsSmall(app, cpu, view); });
}

void frontend::hudSetGridHighlight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_482030", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 part = cpu.eax, view = cpu.edx, colour = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::setGridHighlight(app, cpu, part, view, colour); });
}

void frontend::hudDrawGridHighlight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_482190", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::drawGridHighlight(app, cpu, view); });
}

void frontend::hudDrawReplayBar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_482880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 eax = cpu.eax, ebx = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::drawReplayBar(app, cpu, eax, ebx); });
}

void frontend::hudFlushFrameQueue(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484030", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::flushFrameQueue(app, cpu); });
}

void frontend::hudQueueStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bc80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 queue = cpu.eax, texture = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudz::queueStart(app, cpu, queue, texture); });
}

void frontend::hudQueueTakeCorners(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bd10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 queue = cpu.eax, count = cpu.edx, cornersOut = cpu.ebx, numberOut = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return hudz::queueTakeCorners(app, cpu, queue, count, cornersOut, numberOut);
    });
}

}
