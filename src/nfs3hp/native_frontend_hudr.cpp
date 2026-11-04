#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstring>
#include <initializer_list>
#include <iterator>

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
 * what they do is called by address. */
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
using fe::X87;
using x86::Float;
using x86::reg32;
using x86::sreg32;

/* A generated function's stack argument, by its place, before its frame. */
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
constexpr reg32 kCameraTarget = 0x422bd0;      // eax a view: the car it looks at
constexpr reg32 kSetRenderState = 0x431900;    // eax the state, edx its value
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kLanguageText = 0x4d1850;      // eax a line's number
constexpr reg32 kCharacterHeight = 0x4d1540;   // eax a character
constexpr reg32 kAlignedText = 0x4897f0;       // eax the text; the stack a TextPlace
constexpr reg32 kFormatTime = 0x489750;
constexpr reg32 kDrawTime = 0x4898b0;
constexpr reg32 kDrawPercentAndCount = 0x489900;
constexpr reg32 kDrawLaps = 0x4899d0;
constexpr reg32 kDrawRatio = 0x489a50;
constexpr reg32 kDrawPlace = 0x48a320;
constexpr reg32 kDrawPlaceWithLetters = 0x48a3a0;
constexpr reg32 kFlushHudLines = 0x48e770;
constexpr reg32 kRebuildElement = 0x4853c0;
constexpr reg32 kQueueQuad = 0x49bd30;         // eax a queue, edx, ebx, ecx and the stack its corners
constexpr reg32 kUnqueueQuad = 0x49bdc0;       // likewise
constexpr reg32 kFlushQueue = 0x49be40;        // eax a queue
constexpr reg32 kFlushFonts = 0x4d1340;
constexpr reg32 kFlushBand = 0x4800a0;         // eax a view
constexpr reg32 kDrawVertexGroup = 0x47fbe0;   // eax a group's vertices
constexpr reg32 kFlatRectangle = 0x459350;     // eax, edx, ebx, ecx its sides; the stack its colour
constexpr reg32 kFrameItem = 0x459410;         // eax the item, edx the colour

// The numbers the elements show.
constexpr reg32 kSplitScreen = 0x6fd3b0;       // 1 when two views share the screen
constexpr reg32 kLanguage = 0x6fbb4c;
constexpr reg32 kTotalLaps = 0x6fd4b4;
constexpr reg32 kRatioTotalA = 0x6fd4bc;
constexpr reg32 kRatioTotalB = 0x6fd4b8;
constexpr reg32 kLapTime = 0x72522c;
constexpr reg32 kPosition = 0x725234;
constexpr reg32 kRacerCount = 0x4cdee0;        // how many a place is among

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
constexpr reg32 kRecords = 0x6fbc50;
constexpr reg32 kRecordsPerSeat = 0x348;
constexpr reg32 kRecordsPerForm = 0x1a4;
constexpr reg32 kSpeedometerForm = 0x18;
constexpr reg32 kTachometerForm = 0x1c;
constexpr reg32 kMapStyle = 0x20;
constexpr reg32 kRectangles = 0x24;            // 23 of 16 bytes
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

/* Whether a car is a cop's, as 0 or 1: the form of the HUD its view has. */
reg32 formOf(const Guest& guest, reg32 car)
{
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

/* The record of a seat in a form. */
constexpr reg32 record(reg32 seat, reg32 form)
{
    return kRecords + seat * kRecordsPerSeat + form * kRecordsPerForm;
}

/* The queue's quad of the four corners from `first`. */
reg32 queueQuad(Guest& guest, reg32 queue, reg32 first)
{
    return guest.call(kQueueQuad, args({ queue, first, first + 1, first + 2 }, { first + 3 }));
}

reg32 unqueueQuad(Guest& guest, reg32 queue, reg32 first)
{
    return guest.call(kUnqueueQuad, args({ queue, first, first + 1, first + 2 }, { first + 3 }));
}

/* ---------------------------------------------------------------------------
 * An element made again: sub_4853c0
 * ------------------------------------------------------------------------- */

constexpr reg32 kViewWidth = 0x7cdac8;
constexpr reg32 kViewHeight = 0x7cdacc;
constexpr reg32 kHudRect = 0x480910;           // eax a view, edx the element, ebx and ecx the view's size
constexpr reg32 kPlaceElement = 0x480a00;      // eax a view
constexpr reg32 kBuildPicture = 0x480fc0;      // eax a view
constexpr reg32 kBuildDial = 0x4812e0;         // eax a view
constexpr reg32 kBuildMapViews = 0x48d110;     // eax a view, edx and ebx its size
constexpr reg32 kBuildMapPanel = 0x481aa0;     // eax a view
constexpr reg32 kBuildTable = 0x481c50;        // eax a view
constexpr reg32 kBuildMarkers = 0x481290;      // eax a view, edx its width
constexpr reg32 kBuildPanel = 0x481510;        // eax a view
constexpr reg32 kBuildMeter = 0x481570;        // eax a view; an empty function
constexpr reg32 kBuildLights = 0x4815d0;       // eax a view
constexpr reg32 kBuildGrid = 0x4818b0;         // eax a view
constexpr reg32 kBuildKind9 = 0x4be5e0;        // eax a form
constexpr reg32 kBuildKind9Rest = 0x47fd80;

/* The start lights stand in one place on both players' screens: the rectangle
 * just set, by seat 1 or seat 2, copied to the other's and made again there. */
void shareStartLights(Guest& guest, reg32 seat, reg32 form, reg32 width, reg32 height)
{
    if (seat != 1 && seat != 2)
        return;
    const reg32 otherSeat = 3 - seat;
    const reg32 otherView = seat == 1 ? 1 : 0;
    const reg32 otherForm = formOf(guest, guest.call(kCameraTarget, args({ otherView })));
    const reg32 from = record(seat, form) + kRectangles + kStartLights * kRectangleBytes;
    const reg32 to = record(otherSeat, otherForm) + kRectangles + kStartLights * kRectangleBytes;
    for (reg32 at = 0; at < kRectangleBytes; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
    guest.call(kHudRect, args({ otherView, kStartLights, width, height }));
    guest.call(kPlaceElement, args({ otherView }));
}

/* sub_4853c0: one element of a view's HUD made again for the rectangle it now
 * has: eax the view, edx the element.  A dial's band comes with the forms that
 * have one; a kind with nothing of its own goes without. */
reg32 rebuildElement(Guest& guest, reg32 view, reg32 element)
{
    const reg32 width = guest.dword(kViewWidth), height = guest.dword(kViewHeight);
    const reg32 seat = view + (guest.dword(kSplitScreen) == 1 ? 1 : 0);
    const reg32 form = formOf(guest, guest.call(kCameraTarget, args({ view })));
    if (element == kStartLights)
        shareStartLights(guest, seat, form, width, height);
    guest.call(kHudRect, args({ view, element, width, height }));
    guest.call(kPlaceElement, args({ view }));
    const reg32 seatBytes = seat * kRecordsPerSeat, formBytes = form * kRecordsPerForm;
    const reg32 recordBytes = seatBytes + formBytes;
    switch (element)
    {
    case kSpeedFigures:
        return guest.call(kBuildPicture, args({ view }));
    case kSpeedDial:
    {
        guest.call(kBuildPicture, args({ view }));
        const reg32 shown = guest.dword(kRecords + recordBytes + kSpeedometerForm);
        guest.leaveEdx(seatBytes);
        if (shown == 4 || shown == 2)
            return guest.call(kFlushBand, args({ view }));
        return recordBytes;
    }
    case kRevFigures:
        return guest.call(kBuildDial, args({ view }));
    case kRevDial:
    {
        guest.call(kBuildDial, args({ view }));
        const reg32 shown = guest.dword(kRecords + recordBytes + kTachometerForm);
        guest.leaveEdx(formBytes);
        if (shown == 2 || shown == 3)
            return guest.call(kFlushBand, args({ view }));
        return recordBytes;
    }
    case kGrid:
        return guest.call(kBuildGrid, args({ view }));
    case kStartLights:
        return guest.call(kBuildLights, args({ view }));
    case kKind9:
        guest.call(kBuildKind9, args({ form }));
        return guest.call(kBuildKind9Rest);
    case kStandings:
    case kLabelRows:
        return guest.call(kBuildTable, args({ view }));
    case kSegmentMeter:
        return guest.call(kBuildMeter, args({ view }));
    case kMap:
    case kCopMap:
        guest.call(kBuildMapViews, args({ view, width, height }));
        return guest.call(kBuildMapPanel, args({ view }));
    case kMarkers:
        return guest.call(kBuildMarkers, args({ view, width }));
    case kPanel:
        return guest.call(kBuildPanel, args({ view }));
    default:
        // A kind with nothing of its own has the jump table's index left in eax;
        // one past the table what placing it left.
        return element <= kCopMap ? element : guest.eax();
    }
}

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

constexpr reg32 kKindIsGrey = 0x47bc20;        // eax a kind
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
constexpr reg32 kDrawSpeedText = 0x489d70;
constexpr reg32 kDrawListRows = 0x489b30;      // eax how many, edx the one chosen, ebx 0
constexpr reg32 kUpdateNeedle = 0x48a220;
constexpr reg32 kPlaceDialArt = 0x481170;      // eax a seat, edx a form, ebx the vertices
constexpr reg32 kDrawRevText = 0x489c40;
constexpr reg32 kSetRevNeedle = 0x489e00;      // eax a view, edx the revs, ebx their limit, ecx the colour
constexpr reg32 kPrepareStandings = 0x47ee80;  // eax a view, edx the car
constexpr reg32 kDrawStandings = 0x48ae70;
constexpr reg32 kDrawLabelRows = 0x48a660;
constexpr reg32 kDrawMapPoints = 0x48d4c0;     // eax a view, edx the car
constexpr reg32 kSetMarkers = 0x48b5f0;        // eax a view, edx and ebx the car, ecx the colour
constexpr reg32 kDrawMeter = 0x48c0e0;
constexpr reg32 kDrawLights = 0x48c7c0;
constexpr reg32 kFlushViewFont = 0x4808b0;     // eax a view
constexpr reg32 kDrawGridLines = 0x482030;     // eax 0, edx a view, ebx the colour
constexpr reg32 kDrawGridSecond = 0x482190;    // eax a view
constexpr reg32 kDrawKind9 = 0x484030;         // the queue at 0x700248 flushed

/* What sub_45ab30 works out for every kind. */
struct Element
{
    Guest& guest;
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

/* A line of the language's text at its place, the box read before the line. */
reg32 drawLanguageLine(Guest& guest, fe::Arguments place, std::uint32_t line)
{
    place.registers[0] = guest.call(kLanguageText, args({ line }));
    place.registerCount = 1;
    return guest.call(kAlignedText, place);
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
    if (leftAlone)
        return guest.call(kDrawListRows, args({ gears, gear, 0 }, { scale, kColourRow, kColourGrey, 0, left, right, bottom }));
    return guest.call(kDrawListRows, args({ gears, gear, 0 }, { scale, e.colour, e.colour, e.colour, left, right, bottom }));
}

/* The frame around the element's item, for the one the editor's pointer is on. */
reg32 frameItem(const Element& e)
{
    return e.guest.call(kFrameItem, args({ e.item, e.colour & kColourNoAlpha }));
}

/* Kind 0: the speed in figures, and the gears' rows in the box's right part. */
reg32 drawSpeedFigures(const Element& e)
{
    constexpr reg32 kRowsShare = 0x539bfc;     // a double, 0.75
    Guest& guest = e.guest;
    const reg32 seat = (guest.dword(kSplitScreen) == 1 ? 1 : 0) + e.view;
    const reg32 form = guest.dword(record(seat, formOf(guest, e.car)) + kSpeedometerForm);
    const reg32 firstUnits = form == 1 || form == 2 ? 1 : 0;
    guest.call(kDrawSpeedText, args({}, { kFullScaleSpeed, firstUnits, guest.dword(kScaleLabel), e.colour, e.box(kBoxRight), e.top }));
    X87 x87(guest);
    const Float left = X87::integer(std::int32_t(e.box(kBoxLeft)));
    const Float right = X87::integer(std::int32_t(e.box(kBoxRight)));
    const float rightStored = float(right);
    const float rows = float(fp::mul(fp::sub(right, left), doubleAt(guest, kRowsShare)));
    const float rowsLeft = float(fp::sub(Float(rightStored), Float(rows)));
    const reg32 rowsRight = reg32(x87.truncated(Float(rightStored)));
    const reg32 rowsStart = reg32(x87.truncated(Float(rowsLeft)));
    return drawGearRows(e, rowsStart, rowsRight, e.box(kBoxBottom));
}

/* Kind 1: the speedometer's dial, its needle and the gears' rows. */
reg32 drawSpeedDial(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 view = e.view;
    const reg32 seat = view + (guest.dword(kSplitScreen) == 1 ? 1 : 0);
    const reg32 form = formOf(guest, e.car);
    guest.call(kUpdateNeedle, args({ view }, { kFullScaleSpeed, e.colour }));
    guest.call(kPlaceDialArt, args({ seat, form, guest.dword(kSpeedVertices) + (view << 8) }));
    guest.call(kDrawVertexGroup, args({ guest.dword(kSpeedGroup) + (view << 5) }));
    const reg32 quads = guest.dword(kSpeedQuads) + view * 8;
    queueQuad(guest, kDialQueue, quads);
    queueQuad(guest, kDialQueue, quads + 4);
    drawGearRows(e, e.box(kBoxLeft), e.box(kBoxRight), e.box(kBoxBottom));
    const reg32 flushed = guest.call(kFlushQueue, args({ kDialQueue }));
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
    return guest.call(kDrawRevText, args({ guest.dword(e.car + kCarRevs) },
                                         { guest.dword(kScaleLabel), guest.dword(kScaleLarge), e.colour, e.box(kBoxRight), e.top }));
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
    const Float artWidth = X87::integer(guest.word(kArtWidth));
    const Float artHeight = X87::integer(guest.word(kArtHeight));
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

    guest.call(kDrawVertexGroup, args({ guest.dword(kRevGroup) + (view << 5) }));
    const reg32 quad = guest.dword(kRevQuads) + view * 7;
    queueQuad(guest, kDialQueue, quad);
    const reg32 limit = guest.dword(guest.dword(e.car + kCarData) + kDataRevLimit);
    guest.call(kSetRevNeedle, args({ view, guest.dword(e.car + kCarRevs), limit, e.colour }));
    guest.call(kFlushQueue, args({ kDialQueue }));
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
    return guest.call(kDrawPercentAndCount, e.text({ percent, guest.dword(e.car + kCarCount) }, guest.dword(kScaleSmall)));
}

/* Kind 10: the place among the racers; in the first language with its letters,
 * for which it takes the large scale too. */
reg32 drawPlace(const Element& e)
{
    Guest& guest = e.guest;
    const bool plain = guest.dword(kLanguage) != 0;
    const reg32 left = e.box(kBoxLeft), right = e.box(kBoxRight);
    const reg32 small = guest.dword(kScaleSmall);
    if (plain)
    {
        const reg32 among = guest.call(kRacerCount);
        return guest.call(kDrawPlace, args({ guest.dword(kPosition), among }, { small, e.colour, left, right, e.top, e.alignment }));
    }
    const reg32 large = guest.dword(kScaleLarge);
    const reg32 among = guest.call(kRacerCount);
    return guest.call(kDrawPlaceWithLetters, args({ guest.dword(kPosition), among }, { large, small, e.colour, left, right, e.top, e.alignment }));
}

/* The panel behind a list: its quad queued, on a card without alpha. */
void queueListPanel(const Element& e)
{
    Guest& guest = e.guest;
    if (e.cardWithAlpha())
        return;
    queueQuad(guest, kBackQueue, guest.dword(kBackQuads) + e.view * 4);
    guest.call(kDrawVertexGroup, args({ guest.dword(kBackGroup) + (e.view << 5) }));
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
    X87 x87(guest);
    const reg32 bottom = reg32(x87.truncated(x87.load(corner + kThirdY)));
    const reg32 right = reg32(x87.truncated(x87.load(corner + kThirdX)));
    const reg32 top = reg32(x87.truncated(x87.load(corner + kFirstY)));
    const reg32 left = reg32(x87.truncated(x87.load(corner + kFirstX)));
    return guest.call(kFlatRectangle, args({ left, top, right, bottom }, { e.colour & kColourNoAlpha }));
}

/* Kind 13: the standings: its rectangle's element made again first, as they grow. */
reg32 drawStandings(const Element& e)
{
    Guest& guest = e.guest;
    guest.call(kPrepareStandings, args({ e.view, e.car }));
    guest.call(kRebuildElement, args({ e.view, kStandings }));
    queueListPanel(e);
    guest.call(kDrawStandings, args({}, { guest.dword(kScaleRows), e.box(kBoxLeft), guest.dword(kListWidths + e.view * 4), e.view }));
    return frameListPanel(e);
}

/* Kind 20: the list of labels. */
reg32 drawLabelRows(const Element& e)
{
    Guest& guest = e.guest;
    queueListPanel(e);
    guest.call(kDrawLabelRows, args({ e.view }, { guest.dword(kScaleRows), e.box(kBoxLeft), e.box(kBoxRight), guest.dword(kListWidths + e.view * 4) }));
    return frameListPanel(e);
}

/* Kinds 17 and 22: the track's map and what is on it (its panel on a card without
 * alpha), then the mesh of its lines and the queues, the last under kStateShape on
 * a card with alpha. */
reg32 drawMap(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 view = e.view;
    guest.call(kDrawMapPoints, args({ view, e.car }));
    guest.call(kDrawVertexGroup, args({ guest.dword(kMapGroup) + (view << 5) }));
    if (!e.cardWithAlpha())
    {
        const reg32 quad = guest.dword(kMapQuads) + view * 4;
        queueQuad(guest, kBackQueue, quad);
        guest.call(kFlushQueue, args({ kBackQueue }));
        unqueueQuad(guest, kBackQueue, quad);
    }
    guest.call(kFlushHudLines);
    guest.call(kFlushQueue, args({ kDialQueue }));
    if (e.cardWithAlpha())
        guest.call(kSetRenderState, args({ kStateShape, 1 }));
    const reg32 flushed = guest.call(kFlushQueue, args({ kOverQueue }));
    if (!e.cardWithAlpha())
        return flushed;
    return guest.call(kSetRenderState, args({ kStateShape, 0 }));
}

/* Kind 8: the fonts and the views' text flushed, the lights lit, the bands drawn. */
reg32 drawLights(const Element& e)
{
    Guest& guest = e.guest;
    guest.call(kFlushFonts);
    guest.call(kFlushViewFont, args({ 0 }));
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kFlushViewFont, args({ 1 }));
    guest.setDword(kLightsB, 0);
    guest.setDword(kLightsA, 0);
    guest.call(kDrawLights, args({ 0xc, 5, 8, 3 }, { 0 }));
    guest.call(kFlushQueue, args({ kDialQueue }));
    const reg32 band = guest.call(kFlushBand, args({ 0 }));
    if (guest.dword(kSplitScreen) != 1)
        return band;
    return guest.call(kFlushBand, args({ 1 }));
}

/* Kind 6: the grid in the colour, or none when the editor's pointer is elsewhere,
 * and its second pass under kStateShape on a card with alpha. */
reg32 drawGrid(const Element& e)
{
    Guest& guest = e.guest;
    const reg32 result = guest.call(kDrawGridLines, args({ 0, e.view, e.leftAlone() ? 0 : e.colour }));
    if (!e.cardWithAlpha())
        return result;
    guest.call(kSetRenderState, args({ kStateShape, 1 }));
    guest.call(kDrawGridSecond, args({ e.view }));
    return guest.call(kSetRenderState, args({ kStateShape, 0 }));
}

/* An element's colour: white or grey, by its kind, when the editor's pointer is
 * elsewhere; on it, a pulse of the frame's counter. */
reg32 elementColour(Guest& guest, reg32 item, reg32 kind)
{
    if (guest.dword(item + kItemLeftAlone) != 0)
        return guest.call(kKindIsGrey, args({ kind })) != 0 ? kColourGrey : kColourWhite;
    std::uint8_t pulse = std::uint8_t((guest.dword(kPulse) & 0x7f) * 2);
    if (pulse > 0x7f)
        pulse = std::uint8_t(0xff - pulse);
    const std::uint8_t rest = std::uint8_t(0x80 - pulse);
    const reg32 low = rest > 0x40 ? 0x40 : rest;
    return 0xff000000 | ((pulse + 0x80u) << 16) | (low << 8) | low;
}

/* sub_45ab30: an element of the HUD drawn for its view: eax its item.  What it
 * hands back is what its last call left in eax, or what a kind that ends without
 * a call had there: the item, the kind. */
reg32 drawElement(Guest& guest, reg32 item)
{
    constexpr std::uint32_t kLabelLineId = 0x5fd, kKeyLineId = 0x151;
    constexpr reg32 kKeyAlignment = 4;
    constexpr reg32 kKeyColour = 0x9ffafa00, kMeterColour = 0xff151f1f, kMarkersColour = 0x8f004f00;

    const std::int32_t number = std::int32_t(guest.dword(item + kItemKind));
    const reg32 view = number >= kKindsPerView ? 1 : 0;
    const reg32 kind = reg32(view != 0 ? number - kKindsPerView : number);
    const reg32 top = guest.dword(kBoxes + view * kBoxesPerView + kind * kRectangleBytes + kBoxTop);
    const reg32 alignment = guest.dword(kAlignments + view * kAlignmentsPerView + kind * 4);
    const reg32 car = guest.call(kCameraTarget, args({ view }));
    const reg32 colour = elementColour(guest, item, kind);
    const Element e{ guest, item, view, kind, car, colour, top, alignment };
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
        return guest.call(kDrawLaps, e.text({ 0, guest.dword(kTotalLaps) }, small));
    case kLapTimeText:
        return guest.call(kDrawTime, e.text({ guest.dword(kLapTime) }, small));
    case kGrid:
        return drawGrid(e);
    case kPercentAndCount:
        return drawPercent(e);
    case kStartLights:
        return drawLights(e);
    case kKind9:
        guest.call(kDrawKind9);
        return e.leftAlone() ? item : frameItem(e);
    case kPlace:
        return drawPlace(e);
    case kTimeZero:
        return guest.call(kDrawTime, e.text({ 0 }, small));
    case kLabelLine:
        return drawLanguageLine(guest, e.text({ 0 }, guest.dword(kScaleLarge)), kLabelLineId);
    case kStandings:
        return drawStandings(e);
    case kRatioFirst:
        return guest.call(kDrawRatio, e.text({ guest.dword(car + kCarRatioA), guest.dword(kRatioTotalA) }, small));
    case kKeyLine:
    {
        Element line = e;
        line.colour = e.leftAlone() ? kKeyColour : colour;
        line.top = e.box(kBoxTop);
        line.alignment = kKeyAlignment;
        return drawLanguageLine(guest, line.text({ 0 }, small), kKeyLineId);
    }
    case kSegmentMeter:
    {
        const reg32 meterColour = e.leftAlone() ? kMeterColour : colour;
        const reg32 left = e.box(kBoxLeft), meterTop = e.box(kBoxTop);
        return guest.call(kDrawMeter, args({ view, left, meterTop, e.box(kBoxRight) - left },
                                           { e.box(kBoxBottom) - meterTop, reg32(-1), meterColour }));
    }
    case kMap:
    case kCopMap:
        return drawMap(e);
    case kMarkers:
        return guest.call(kSetMarkers, args({ view, car, car, e.leftAlone() ? kMarkersColour : colour }));
    case kRatioSecond:
        return guest.call(kDrawRatio, e.text({ guest.dword(car + kCarRatioB), guest.dword(kRatioTotalB) }, small));
    case kLabelRows:
        return drawLabelRows(e);
    case kPanel:
        queueQuad(guest, kDialQueue, guest.dword(kPanelQuads) + view * 4);
        return guest.call(kFlushQueue, args({ kDialQueue }));
    default:
        return kind;
    }
}

/* ---------------------------------------------------------------------------
 * The map's pane: sub_48e920
 * ------------------------------------------------------------------------- */

constexpr reg32 kPaneStep = 0x48e790;          // eax a view

/* sub_48e920: the first view's map stepped on and, in a split screen, the second's
 * until both are of one style and either both or neither at the top of their
 * rectangles (the kind of map each has by its form). */
reg32 stepMapPanes(Guest& guest)
{
    const reg32 stepped = guest.call(kPaneStep, args({ 0 }));
    if (guest.dword(kSplitScreen) != 1)
        return stepped;
    const reg32 firstForm = formOf(guest, guest.call(kCameraTarget, args({ 0 })));
    const reg32 secondForm = formOf(guest, guest.call(kCameraTarget, args({ 1 })));
    const reg32 first = record(1, firstForm), second = record(2, secondForm);
    const reg32 firstTop = first + kRectangles + (firstForm != 0 ? kCopMap : kMap) * kRectangleBytes + kBoxTop;
    const reg32 secondTop = second + kRectangles + (secondForm != 0 ? kCopMap : kMap) * kRectangleBytes + kBoxTop;
    const auto atTop = [&](reg32 topAt) -> reg32 { return (guest.dword(topAt) & 0x7fffffff) == 0 ? 1 : 0; };
    for (;;)
    {
        if (guest.dword(first + kMapStyle) == guest.dword(second + kMapStyle))
        {
            const reg32 firstAtTop = atTop(firstTop);
            if (firstAtTop == atTop(secondTop))
                return firstAtTop;
        }
        guest.call(kPaneStep, args({ 1 }));
    }
}

/* ---------------------------------------------------------------------------
 * The HUD's text
 * ------------------------------------------------------------------------- */

constexpr GuestString kLettersTh{ 0x53b7cc, "th" };
constexpr GuestString kLettersSt{ 0x53b7d0, "st" };
constexpr GuestString kLettersNd{ 0x53b7d4, "nd" };
constexpr GuestString kLettersRd{ 0x53b7d8, "rd" };
constexpr GuestString kLettersStrings[] = { kLettersTh, kLettersSt, kLettersNd, kLettersRd };

/* sub_4896f0: the letters after a place: "st", "nd", "rd", or "th" for 4 to 20
 * and all the rest. */
reg32 ordinalLetters(std::int32_t place)
{
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

/* A time's formats, by language. */
constexpr reg32 kTimeFormats = 0x55d6cc;

/* sub_489750: a time in 64ths of a second, written without its sign into `buffer`
 * as minutes, seconds and hundredths, in the language's format.  edx is left 0:
 * the sign of the hundredths, which have none. */
reg32 formatTime(Guest& guest, reg32 buffer, std::int32_t time)
{
    constexpr std::int32_t kTicksPerSecond = 64, kSecondsPerMinute = 60;
    const std::int32_t magnitude = time > 0 ? time : std::int32_t(reg32(0) - reg32(time));
    const std::int32_t seconds = magnitude / kTicksPerSecond;
    const std::int32_t hundredths = (magnitude % kTicksPerSecond) * 100 / kTicksPerSecond;
    const reg32 format = guest.dword(kTimeFormats + guest.dword(kLanguage) * 4);
    const reg32 written = guest.call(kSprintf, args({}, { buffer, format, reg32(seconds / kSecondsPerMinute),
                                                          reg32(seconds % kSecondsPerMinute), reg32(hundredths) }));
    guest.leaveEdx(0);
    return written;
}

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
    return guest.call(kAlignedText, args({ buffer }, { place.scale, place.colour, place.left, place.right, place.top, place.how }));
}

/* sub_4898b0: a time at its place. */
reg32 drawTime(Guest& guest, reg32 time, const TextPlace& place)
{
    const reg32 buffer = guest.frame();
    guest.call(kFormatTime, args({ buffer, time }));
    return drawPlaced(guest, buffer, place);
}

constexpr GuestString kPercentFormat{ 0x53b7ec, "%d%% %s" };
constexpr GuestString kCountFormat{ 0x53b7f4, "%d %s" };
constexpr GuestString kPercentStrings[] = { kPercentFormat, kCountFormat };
constexpr std::uint32_t kPercentLine = 0x152, kCountOfOneLine = 0x153, kCountLine = 0x154;
constexpr reg32 kPercentSign = '%';

/* sub_489900: "85% ..." at its place and, a percent sign's height below it, how
 * many: one has a line of its own. */
reg32 drawPercentAndCount(Guest& guest, reg32 percent, reg32 count, const TextPlace& place)
{
    const reg32 buffer = guest.frame();
    const reg32 line = guest.call(kLanguageText, args({ kPercentLine }));
    guest.call(kSprintf, args({}, { buffer, kPercentFormat.address, percent, line }));
    drawPlaced(guest, buffer, place);
    if (count == 1)
    {
        const reg32 one = guest.call(kLanguageText, args({ kCountOfOneLine }));
        guest.call(kSprintf, args({}, { buffer, one }));
    }
    else
    {
        const reg32 many = guest.call(kLanguageText, args({ kCountLine }));
        guest.call(kSprintf, args({}, { buffer, kCountFormat.address, count, many }));
    }
    TextPlace below = place;
    below.top = place.top + guest.call(kCharacterHeight, args({ kPercentSign }));
    const reg32 drawn = drawPlaced(guest, buffer, below);
    guest.leaveEdx(place.how);
    return drawn;
}

constexpr GuestString kLapsWordFirst{ 0x53b7fc, "%s %d/%d" };
constexpr GuestString kLapsWordLast{ 0x53b808, "%d/%d %s" };
constexpr GuestString kLapsStrings[] = { kLapsWordFirst, kLapsWordLast };
constexpr std::uint32_t kLapsLine = 0x155;
constexpr reg32 kLanguageWordFirst = 2;

/* sub_4899d0: a count of a total with its word, first in one language and last
 * in the rest. */
reg32 drawLaps(Guest& guest, reg32 done, reg32 total, const TextPlace& place)
{
    const reg32 buffer = guest.frame();
    const reg32 line = guest.call(kLanguageText, args({ kLapsLine }));
    if (guest.dword(kLanguage) == kLanguageWordFirst)
        guest.call(kSprintf, args({}, { buffer, kLapsWordFirst.address, line, done, total }));
    else
        guest.call(kSprintf, args({}, { buffer, kLapsWordLast.address, done, total, line }));
    const reg32 drawn = drawPlaced(guest, buffer, place);
    guest.leaveEdx(place.left);
    return drawn;
}

constexpr GuestString kRatioFormat{ 0x53b814, "%s: %d/%d" };
constexpr GuestString kRatioStrings[] = { kRatioFormat };
constexpr std::uint32_t kRatioLine = 0x14d;

/* sub_489a50: a line of the language's with a count of a total. */
reg32 drawRatio(Guest& guest, reg32 done, reg32 total, const TextPlace& place)
{
    const reg32 buffer = guest.frame();
    const reg32 line = guest.call(kLanguageText, args({ kRatioLine }));
    guest.call(kSprintf, args({}, { buffer, kRatioFormat.address, line, done, total }));
    const reg32 drawn = drawPlaced(guest, buffer, place);
    guest.leaveEdx(place.how);
    return drawn;
}

constexpr GuestString kPlaceFormatPos{ 0x53b890, "Pos. %d/%d" };
constexpr GuestString kPlaceFormatOf{ 0x53b89c, "%d\xba de %d" };
constexpr GuestString kPlaceFormat{ 0x53b8a8, "%d/%d" };
constexpr GuestString kPlaceStrings[] = { kPlaceFormatPos, kPlaceFormatOf, kPlaceFormat };
constexpr reg32 kLanguagePos = 2, kLanguageOf = 3;

/* sub_48a320: a place among so many, as the language writes it. */
reg32 drawPlaceAmong(Guest& guest, reg32 place, reg32 among, const TextPlace& at)
{
    const reg32 buffer = guest.frame();
    const reg32 language = guest.dword(kLanguage);
    const reg32 format = language == kLanguagePos ? kPlaceFormatPos.address
        : language == kLanguageOf ? kPlaceFormatOf.address : kPlaceFormat.address;
    guest.call(kSprintf, args({}, { buffer, format, place, among }));
    const reg32 drawn = drawPlaced(guest, buffer, at);
    guest.leaveEdx(at.left);
    return drawn;
}

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
constexpr reg32 kDrawSegment = 0x4c0b20;       // eax and edx two vertices
constexpr reg32 kDrawTriangleClipped = 0x4c11b0;  // eax, edx and ebx three vertices
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

/* sub_48b2a0: a triangle in a colour, filled or as three lines. */
reg32 drawTriangle(Guest& guest, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 filled, reg32 colour)
{
    placeVertices(guest, kTriangleBlanks, { x0, y0, x1, y1, x2, y2 }, colour);
    const reg32 first = guest.frame(), second = first + kVertexBytes, third = second + kVertexBytes;
    guest.call(kSetRenderState, args({ kStateTexture, 0 }));
    if (filled != 0)
        return guest.call(kDrawTriangleClipped, args({ first, second, third }));
    guest.call(kDrawSegment, args({ first, second }));
    guest.call(kDrawSegment, args({ second, third }));
    return guest.call(kDrawSegment, args({ third, first }));
}

/* sub_48b4a0: a line in a colour. */
reg32 drawLine(Guest& guest, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 colour)
{
    placeVertices(guest, kLineBlanks, { x0, y0, x1, y1 }, colour);
    const reg32 first = guest.frame();
    guest.call(kSetRenderState, args({ kStateTexture, 0 }));
    return guest.call(kDrawSegment, args({ first, first + kVertexBytes }));
}

constexpr reg32 kLineMesh = 0x793108;
constexpr reg32 kFlushMesh = 0x49bf60, kPrepareMesh = 0x49bef0;

}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

void frontend::hudDrawElement(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45ab30", 0, 24 + 0x5c, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawElement(guest, item); });
}

void frontend::hudRebuildElement(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4853c0", 0, 20 + 0x18, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 view = cpu.eax, element = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return rebuildElement(guest, view, element); });
}

void frontend::hudStepMapPanes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48e920", 0, 24 + 8, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return stepMapPanes(guest); });
}

void frontend::hudOrdinalLetters(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4896f0", 0, 12, kLettersStrings, std::size(kLettersStrings), kKeepsEcxEdx };
    const std::int32_t place = std::int32_t(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest&) { return ordinalLetters(place); });
}

void frontend::hudFormatTime(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_489750", 0, 20 + 4, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 buffer = cpu.eax;
    const std::int32_t time = std::int32_t(cpu.edx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return formatTime(guest, buffer, time); });
}

void frontend::hudDrawTime(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4898b0", 0x18, 24 + kTextBufferBytes, nullptr, 0 };
    const reg32 time = cpu.eax;
    const TextPlace place = textPlaceOf(app, cpu);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTime(guest, time, place); });
}

void frontend::hudDrawPercentAndCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_489900", 0x18, 20 + kTextBufferBytes, kPercentStrings, std::size(kPercentStrings), kKeepsEbxEcxEsiEdi };
    const reg32 percent = cpu.eax, count = cpu.edx;
    const TextPlace place = textPlaceOf(app, cpu);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawPercentAndCount(guest, percent, count, place); });
}

void frontend::hudDrawLaps(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4899d0", 0x18, 16 + kTextBufferBytes, kLapsStrings, std::size(kLapsStrings), kKeepsEcxEsiEdi };
    const reg32 done = cpu.eax, total = cpu.edx;
    const TextPlace place = textPlaceOf(app, cpu);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawLaps(guest, done, total, place); });
}

void frontend::hudDrawRatio(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_489a50", 0x18, 20 + kTextBufferBytes, kRatioStrings, std::size(kRatioStrings), kKeepsEbxEcxEsiEdi };
    const reg32 done = cpu.eax, total = cpu.edx;
    const TextPlace place = textPlaceOf(app, cpu);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawRatio(guest, done, total, place); });
}

void frontend::hudDrawPlaceAmong(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48a320", 0x18, 16 + kTextBufferBytes, kPlaceStrings, std::size(kPlaceStrings), kKeepsEcxEsiEdi };
    const reg32 placeNumber = cpu.eax, among = cpu.edx;
    const TextPlace place = textPlaceOf(app, cpu);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawPlaceAmong(guest, placeNumber, among, place); });
}

void frontend::hudDrawTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48b2a0", 0x20, 24 + 0x60, nullptr, 0 };
    const reg32 x0 = stackArgument(app, cpu, 0), y0 = stackArgument(app, cpu, 1), x1 = stackArgument(app, cpu, 2);
    const reg32 y1 = stackArgument(app, cpu, 3), x2 = stackArgument(app, cpu, 4), y2 = stackArgument(app, cpu, 5);
    const reg32 filled = stackArgument(app, cpu, 6), colour = stackArgument(app, cpu, 7);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTriangle(guest, x0, y0, x1, y1, x2, y2, filled, colour); });
}

void frontend::hudDrawLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48b4a0", 0x14, 24 + 0x60, nullptr, 0 };
    const reg32 x0 = stackArgument(app, cpu, 0), y0 = stackArgument(app, cpu, 1), x1 = stackArgument(app, cpu, 2);
    const reg32 y1 = stackArgument(app, cpu, 3), colour = stackArgument(app, cpu, 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawLine(guest, x0, y0, x1, y1, colour); });
}

void frontend::hudFlushLines(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48e770", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kFlushMesh, args({ kLineMesh })); });
}

void frontend::hudPrepareLines(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48e780", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kPrepareMesh, args({ kLineMesh })); });
}

}
