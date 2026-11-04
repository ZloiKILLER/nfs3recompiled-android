#include "native_frontend.h"
#include <nfs3hp.h>
#include <initializer_list>
#include <iterator>

/* What the menus lean on past their screens' handlers, decompiled:
 *
 *   the results tables   the widgets of the screens after a race: the race's
 *                        results with a column to sort by (type 0x17, sub_46d3c0
 *                        and its handlers), two lists of three columns (types
 *                        0x19 and 0x1a, sub_46e690 and sub_46f510, here the
 *                        standings and the rounds, from what they print) and the
 *                        players side by side (type 0x1b, sub_4703a0), which keep
 *                        their state in their items;
 *   the menu art         the chrome and the glow of the buttons (sub_465d80);
 *   the front end's ends the title's art and its wait (sub_472f20, sub_472fc0),
 *                        going in and coming out of the menus (sub_473020,
 *                        sub_473080) and the screens after a race (sub_4732b0). */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using x86::reg32;

// The menu engine's.
constexpr reg32 kItemShownNow = 0x44a660;      // eax the item, edx its widget's type
constexpr reg32 kKeysTaken = 0x449df0;         // nonzero: the dialog has the keys, the screen is dimmed
constexpr reg32 kPointerInBox = 0x49c1f0;      // eax a box: top, left, width, height, int16 each
constexpr reg32 kSetPointer = 0x49bfc0;
constexpr reg32 kPointerX = 0x55e5e4;
constexpr reg32 kPointerY = 0x55e5e8;
constexpr reg32 kMenuEffect = 0x4181d0;        // eax the effect, edx its volume
constexpr std::uint32_t kClick = 1, kClickVolume = 0x7f;

// Text and drawing.
constexpr reg32 kLanguageText = 0x4d1850;
constexpr reg32 kTextWidth = 0x452020;
constexpr reg32 kDrawText = 0x452190;          // eax text, edx x, ebx y, ecx font; alignment, colour
constexpr reg32 kDrawTextFitted = 0x452120;    // the same; alignment, colour, the width it may take
constexpr reg32 kShadedBox = 0x4d8860;         // eax x, edx y, ebx width, ecx height; four colours
constexpr reg32 kFilledBox = 0x4d8930;         // the same; a colour
constexpr reg32 kFieldBox = 0x4d8ac0;          // the same; four colours, blended
constexpr reg32 kDrawLine = 0x4d8ed0;          // eax x1, edx y1, ebx x2, ecx y2; two colours
constexpr reg32 kDrawShape = 0x4d7990;         // eax the shape, edx x, ebx y
constexpr reg32 kShapeByName = 0x4d5db0;
constexpr reg32 kSprintf = 0x4df690;
constexpr std::uint32_t kFont = 0x12;

// The alignments kDrawText takes.
constexpr std::uint32_t kAlignLeft = 3, kAlignRight = 4, kAlignCentre = 5;

// Their colours.
constexpr std::uint32_t kGold = 0xffd59428, kBlue = 0xff0000aa, kBlack = 0xff000000;
constexpr std::uint32_t kRowText = 0xff649dfd, kPlayerText = 0x00ffe440, kHotText = 0x00ffe440;
constexpr std::uint32_t kDimmedText = 0x00004a96, kOutText = 0x00800000;
constexpr std::uint32_t kHotFrame = 0xffffe440;
constexpr std::uint32_t kBestLap = 0xffff0000, kOtherLap = 0xff007fff;

// The results, as the screens after a race read them.
constexpr reg32 kSortResults = 0x455c10;       // eax which, edx the key, ebx descending
constexpr reg32 kNthResult = 0x455cd0;         // eax which, edx the place: a racer's record or 0
constexpr reg32 kPlayerResult = 0x4554e0;      // eax which, edx the player: the same
constexpr reg32 kFinishers = 0x4cef80;         // nonzero (sub_4cef80's answer): a table's plain form, no sorting, no popups
constexpr reg32 kRacerRanks = 0x455530;        // eax the record, edx and ebx two ints written
constexpr reg32 kRacerName = 0x455570;         // eax the record, edx a buffer filled
constexpr reg32 kRacerPlace = 0x4555c0;
constexpr reg32 kRacerCar = 0x455600;
constexpr reg32 kRacerFirstLine = 0x455660;
constexpr reg32 kRacerSecondLine = 0x4556c0;
constexpr reg32 kRacerResult = 0x455720;
constexpr reg32 kFormatTime = 0x4cee40;        // eax a time, edx a buffer filled
constexpr reg32 kResultsCount = 0x677d30;      // the racers
constexpr reg32 kRoundsCount = 0x677d34;
constexpr reg32 kRacerStatus = 0x2c4;          // 2 and 4: did not finish
constexpr reg32 kRacerPlayer = 0x1f0;
constexpr reg32 kRacerLapsDone = 0x2c8;
constexpr reg32 kRacerTime = 0x238;
constexpr reg32 kRacerLaps = 0x240;            // the time of each lap
constexpr reg32 kRacerRoundsDone = 0x2cc;
constexpr reg32 kRacerRoundsTime = 0x2d4;
constexpr std::uint32_t kStatusOut = 2, kStatusWrecked = 4;

constexpr reg32 kRaceType = 0x6fd3b8;          // 0 a single race, 1 a tournament, 2 a knockout, 3 a duel
constexpr std::uint32_t kRaceDuel = 3;
constexpr reg32 kThisPlayer = 0x6fd4f4;
constexpr reg32 kPlayers = 0x6fd3b0;
constexpr reg32 kLaps = 0x6fd4b4;
constexpr reg32 kRaceLaps = 0x6fd4bc;
constexpr reg32 kRoundsToWin = 0x6fd4b8;

// The lines of text.
constexpr std::uint32_t kLineLaps = 0x193, kLineNoTime = 0xeb, kLineNoPlace = 0xed, kLineWrecked = 0xf5;
constexpr std::uint32_t kLineSortBy = 0x199, kLineLapTimes = 5, kLineWinner = 0x194;

// The game's formats.
constexpr reg32 kFormatLapNumber = 0x53aeb4;   // "%d)"
constexpr reg32 kFormatNumber = 0x53aeb8;      // "%d"
constexpr reg32 kFormatOfTotal = 0x53aebc;     // "%d/%d"
constexpr reg32 kFormatNumberAndName = 0x53aee4; // "%d (%s)"
constexpr reg32 kFormatOfTotalAndName = 0x53aeec; // "%d/%d (%s)"
constexpr reg32 kNoValue = 0x53aef8;

// The arrows' art.
constexpr reg32 kArtUp = 0x53aec4, kArtDown = 0x53aecc, kArtUpHot = 0x53aed4, kArtDownHot = 0x53aedc;

// The tables slide in and out: 20 pixels a pass, out past the screen's edge.
constexpr int kSlideStep = 0x14;
constexpr int kSlidOut = 0x2a8;
enum SlideState { kSlideNone = 0, kSlideIn = 1, kSlideOut = 2, kSlideGone = 3 };

constexpr reg32 kItemX = 6;
constexpr reg32 kItemY = 8;
constexpr reg32 kItemBoundTop = 0x1a, kItemBoundLeft = 0x1c, kItemBoundWidth = 0x1e, kItemBoundHeight = 0x20;

constexpr int kArrowSize = 0xc;

constexpr std::uint32_t kResultsType = 0x17, kStandingsType = 0x19, kRoundsType = 0x1a, kSideBySideType = 0x1b;

/* Half, towards zero. */
std::int32_t half(std::int32_t value)
{
    return (value - (value >> 31)) >> 1;
}

std::int32_t s16(const Guest& guest, reg32 address)
{
    return std::int16_t(guest.word(address));
}

/* The low word of a register set, the high one kept: what a `mov ax, ...`
 * leaves, for what a function leaves in a register it does not save. */
reg32 withLow(reg32 value, std::uint32_t low)
{
    return (value & 0xffff0000) | (low & 0xffff);
}

std::int32_t smaller(std::int32_t a, std::int32_t b)
{
    return a < b ? a : b;
}

/* A table's fields in its item: the same, at different places in the types. */
struct Fields
{
    reg32 rowHeight;
    reg32 rows;         // how many it shows at once
    reg32 boxes;        // a box for each column's title: top, left, width, height
    reg32 up, down;     // the arrows' boxes
    reg32 shapes;       // up, down, up under the pointer, down under the pointer, the two kept
    reg32 scroll;       // the first racer shown
    reg32 sort;         // the column it is sorted by
    reg32 state;        // SlideState
    reg32 x;            // where it stands while it slides
};

constexpr reg32 kColumns = 0x3c;               // int16 each: the columns' widths
constexpr Fields kResultsFields{ 0x44, 0x46, 0x48, 0x68, 0x70, 0x78, 0x84, 0x86, 0x88, 0x8a };
constexpr Fields kStandingsFields{ 0x42, 0x44, 0x46, 0x5e, 0x66, 0x6e, 0x7a, 0x7c, 0x7e, 0x80 };

/* A table on the screen, for its functions. */
class Table
{
public:
    Table(Guest& guest, reg32 item, const Fields& fields, std::uint32_t type)
        : g(guest), item(item), f(fields), type(type)
    {
    }

    bool shown() const { return g.call(kItemShownNow, fe::args({ item, type })) != 0; }

    std::int32_t x() const { return s16(g, item + f.x); }
    std::int32_t y() const { return s16(g, item + kItemY); }
    std::int32_t width(int column) const { return s16(g, item + kColumns + 2 * column); }
    std::int32_t rowHeight() const { return s16(g, item + f.rowHeight); }
    std::int32_t rows() const { return s16(g, item + f.rows); }
    std::int32_t scroll() const { return s16(g, item + f.scroll); }
    std::uint16_t state() const { return g.word(item + f.state); }
    reg32 box(int column) const { return item + f.boxes + 8 * column; }

    /* The table a step further on its way in or out. */
    void slide() const
    {
        if (state() == kSlideOut)
        {
            const std::int16_t moved = std::int16_t(g.word(item + f.x) + kSlideStep);
            g.setWord(item + f.x, std::uint16_t(moved));
            if (moved >= kSlidOut)
            {
                g.setWord(item + f.x, kSlidOut);
                g.setWord(item + f.state, kSlideGone);
            }
        }
        if (state() == kSlideIn)
        {
            g.setWord(item + f.x, std::uint16_t(g.word(item + f.x) - kSlideStep));
            if (x() <= s16(g, item + kItemX))
            {
                g.setWord(item + f.state, kSlideNone);
                g.setWord(item + f.x, g.word(item + kItemX));
            }
        }
    }

    void shaded(std::int32_t bx, std::int32_t by, std::int32_t w, std::int32_t h, std::uint32_t c1, std::uint32_t c2,
                std::uint32_t c3, std::uint32_t c4) const
    {
        g.call(kShadedBox, fe::args({ reg32(bx), reg32(by), reg32(w), reg32(h) }, { c1, c2, c3, c4 }));
    }

    void text(reg32 string, std::int32_t tx, std::int32_t ty, std::uint32_t align, std::uint32_t colour) const
    {
        g.call(kDrawText, fe::args({ string, reg32(tx), reg32(ty), kFont }, { align, colour }));
    }

    void fitted(reg32 string, std::int32_t tx, std::int32_t ty, std::uint32_t colour, std::int32_t room) const
    {
        g.call(kDrawTextFitted, fe::args({ string, reg32(tx), reg32(ty), kFont }, { kAlignLeft, colour, reg32(room) }));
    }

    /* The lines between the rows, then the columns'. */
    void rowLines(std::int32_t shownRows, std::int32_t tableWidth) const
    {
        std::int32_t lineY = y() + rowHeight();
        for (std::int32_t i = 0; i < shownRows; ++i)
        {
            shaded(x(), lineY, tableWidth, 1, kBlue, kGold, kGold, kBlue);
            lineY += rowHeight();
        }
    }

    void columnLines(std::int32_t height, int lines) const
    {
        std::int32_t lineX = x() + width(0);
        for (int column = 1; column <= lines; ++column)
        {
            shaded(lineX, y(), 1, height, kBlue, kBlue, kGold, kGold);
            lineX += width(column);
        }
    }

    /* A column's title: centred or a little in, framed and lit under the
     * pointer, where Enter sorts by it. */
    void title(int column, reg32 string, std::uint32_t align, bool canPoint, bool dimmed, std::int32_t inset) const
    {
        const reg32 at = box(column);
        const std::int32_t left = s16(g, at + 2);
        const std::int32_t titleX = align == kAlignCentre ? left + half(s16(g, at + 4)) : left + 10;
        const std::int32_t titleY = half(s16(g, at + 6)) + s16(g, at) - 5;
        const bool hot = canPoint && g.call(kPointerInBox, fe::args({ at })) != 0;
        if (hot)
            g.call(kFilledBox, fe::args({ reg32(left + inset), reg32(s16(g, at)), reg32(s16(g, at + 4) - inset),
                                          reg32(s16(g, at + 6) - inset) }, { kHotFrame }));
        text(string, titleX, titleY, align, dimmed ? kDimmedText : hot ? kHotText : kGold);
    }

    /* The two arrows, while there are more racers than rows. */
    void arrows(std::int32_t count, std::int32_t upNudge) const
    {
        if (rows() >= count)
            return;
        if (g.word(item + f.scroll) != 0)
            arrow(f.up, 0, upNudge);
        if (rows() + scroll() < count)
            arrow(f.down, 2, 0);
    }

    bool isThisPlayer(reg32 record) const
    {
        const reg32 player = g.dword(record + kRacerPlayer);
        return player == g.dword(kThisPlayer) || (g.dword(kPlayers) == 1 && player == 1);
    }

    /* The six shapes named, in their places. */
    void loadArrows() const
    {
        static constexpr reg32 kShapes[] = { kArtUp, kArtDown, kArtUpHot, kArtDownHot, kArtDown, kArtUp };
        for (int i = 0; i < 6; ++i)
            g.setWord(item + f.shapes + 2 * i, std::uint16_t(g.call(kShapeByName, fe::args({ kShapes[i] }))));
    }

    /* Not yet placed: it stands where its item does. */
    bool placeIfNew() const
    {
        if (g.word(item + f.x) != 0)
            return false;
        g.setWord(item + f.state, kSlideNone);
        g.setWord(item + f.x, g.word(item + kItemX));
        return true;
    }

    /* Whoever is this player, scrolled into view.  What is left in eax is what
     * it stopped at. */
    reg32 scrollToPlayer(std::int32_t count, reg32 lastEax) const
    {
        for (std::int32_t i = 0; i < count; ++i)
        {
            lastEax = g.call(kNthResult, fe::args({ 0, reg32(i), 0, 0 }));
            if (lastEax == 0)
                return 0;
            if (isThisPlayer(lastEax))
            {
                if (i >= rows())
                {
                    const reg32 first = withLow(reg32(i), std::uint32_t(i) - g.word(item + f.rows)) + 1;
                    g.setWord(item + f.scroll, std::uint16_t(first));
                    return first;
                }
                lastEax = reg32(rows());
            }
        }
        return lastEax;
    }

    Guest& g;
    const reg32 item;
    const Fields& f;
    const std::uint32_t type;

private:
    void arrow(reg32 boxField, int shape, std::int32_t nudge) const
    {
        const reg32 at = item + boxField;
        const bool hot = g.call(kPointerInBox, fe::args({ at })) != 0;
        const reg32 art = item + f.shapes + (hot ? 4 + shape : shape);
        g.call(kDrawShape, fe::args({ reg32(s16(g, art)), reg32(s16(g, at + 2)), reg32(s16(g, at) + nudge) }));
    }
};

/* The pointer put on a box, three quarters along it, half way down. */
void setPointer(Guest& guest, std::int32_t x, std::int32_t y)
{
    guest.setDword(kPointerX, reg32(x));
    guest.setDword(kPointerY, reg32(y));
    guest.call(kSetPointer, fe::args({ reg32(x), reg32(y) }));
}

void pointAtBox(Guest& guest, reg32 box)
{
    setPointer(guest, s16(guest, box + 2) + (3 * s16(guest, box + 4)) / 4, s16(guest, box) + half(s16(guest, box + 6)));
}

/* ---------------------------------------------------------------------------
 * The little mark over a table's sorted column
 * ------------------------------------------------------------------------- */

/* sub_46cce0: "sort by" and an arrow over a column's title, over two lines as
 * wide as the words.  On the stack: the column's box, its top and left, then
 * its width and height, then the arrow's shape. */
reg32 drawSortMark(Guest& guest, reg32 topLeft, reg32 widthHeight, reg32 shape)
{
    const std::int32_t top = std::int16_t(topLeft);
    const std::int32_t left = std::int16_t(topLeft >> 16);
    const std::int32_t halfWidth = half(std::int16_t(widthHeight));
    const reg32 measured = guest.call(kLanguageText, fe::args({ kLineSortBy }));
    const std::int32_t textWidth = std::int32_t(guest.call(kTextWidth, fe::args({ measured, kFont })));
    const std::int32_t centre = left + halfWidth;
    const reg32 line = guest.call(kLanguageText, fe::args({ kLineSortBy }));
    guest.call(kDrawText, fe::args({ line, reg32(centre), reg32(top - 0x1e), kFont }, { kAlignCentre, kRowText }));
    guest.call(kDrawShape, fe::args({ shape, reg32(left - 6 + halfWidth), reg32(top - 0xe) }));
    const std::int32_t halfText = half(textWidth);
    const reg32 from = reg32(centre - halfText), to = reg32(centre + halfText);
    guest.call(kDrawLine, fe::args({ from, reg32(top - 0xd), to, reg32(top - 0xd) }, { 0xff5f78a9, 0xff5f78a9 }));
    return guest.call(kDrawLine, fe::args({ from, reg32(top - 0xc), to, reg32(top - 0xc) }, { 0xff738cbd, 0xff738cbd }));
}

/* sub_46cdc0, sub_46e660, sub_46f4e0: that mark, over the table's own column,
 * which they leave in edx. */
reg32 drawTableSortMark(Guest& guest, reg32 item, reg32 boxes, reg32 sort, reg32 shape)
{
    const reg32 column = reg32(s16(guest, item + sort));
    const reg32 at = item + boxes + 8 * column;
    const reg32 result = guest.call(0x46cce0, fe::args({}, { guest.dword(at), guest.dword(at + 4),
                                                              reg32(s16(guest, item + shape)) }));
    guest.leaveEdx(column);
    return result;
}

/* ---------------------------------------------------------------------------
 * The results, sorted by a column
 * ------------------------------------------------------------------------- */

/* sub_46cdf0: which column of a row the pointer is in, for its popups: 1 the
 * lap times', 2 the metric's, 0 neither.  A wrecked racer has no laps. */
reg32 resultsPopupUnderPointer(Guest& guest, reg32 item)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    const reg32 box = guest.frame();
    auto makeBox = [&](std::uint32_t left, std::uint32_t boxWidth) {
        guest.setWord(box + 2, std::uint16_t(left));
        guest.setWord(box, std::uint16_t(guest.word(item + kItemY) + guest.word(item + 0x44)));
        guest.setWord(box + 4, std::uint16_t(boxWidth + 1));
        guest.setWord(box + 6, std::uint16_t(guest.word(item + 0x44) + 1));
    };
    auto nextRow = [&] { guest.setWord(box, std::uint16_t(guest.word(box) + guest.word(item + 0x44))); };
    const std::uint32_t firstLeft = std::uint32_t(guest.word(item + 0x3c)) + guest.word(item + 0x8a) + guest.word(item + 0x3e);
    makeBox(firstLeft, guest.word(item + 0x40));
    for (std::int32_t row = 0; row < t.rows(); ++row)
    {
        if (guest.call(kPointerInBox, fe::args({ box })) != 0)
        {
            const reg32 record = guest.call(kNthResult, fe::args({ 0, reg32(row + t.scroll()), 0, 0 }));
            if (record != 0 && guest.dword(record + kRacerStatus) != kStatusWrecked)
                return 1;
            break;
        }
        nextRow();
    }
    if (guest.dword(kRaceType) != kRaceDuel)
    {
        makeBox(firstLeft + guest.word(item + 0x40), guest.word(item + 0x42));
        for (std::int32_t row = 0; row < t.rows(); ++row)
        {
            if (guest.call(kPointerInBox, fe::args({ box })) != 0)
            {
                if (guest.call(kNthResult, fe::args({ 0, reg32(row + t.scroll()), 0, 0 })) != 0)
                    return 2;
                break;
            }
            nextRow();
        }
    }
    return 0;
}

/* sub_46d180: the popup with a racer's lap times, the best one lit, over the
 * row under the pointer.  What is left in eax is what it stopped at. */
reg32 drawLapTimesPopup(Guest& guest, reg32 item)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    const reg32 box = guest.frame() + 0x100;
    guest.setWord(box + 2, std::uint16_t(std::uint32_t(guest.word(item + 0x3c)) + guest.word(item + 0x8a) + guest.word(item + 0x3e)));
    guest.setWord(box, std::uint16_t(guest.word(item + kItemY) + guest.word(item + 0x44)));
    guest.setWord(box + 4, std::uint16_t(guest.word(item + 0x40) + 1));
    guest.setWord(box + 6, std::uint16_t(guest.word(item + 0x44) + 1));
    for (std::int32_t row = 0; row < t.rows(); ++row)
    {
        if (guest.call(kPointerInBox, fe::args({ box })) == 0)
        {
            guest.setWord(box, std::uint16_t(guest.word(box) + guest.word(item + 0x44)));
            continue;
        }
        const reg32 record = guest.call(kNthResult, fe::args({ 0, reg32(row + t.scroll()), 0, 0 }));
        if (record == 0)
            return 0;
        if (guest.dword(record + kRacerStatus) == kStatusWrecked)
            return record;
        const std::int32_t laps = std::int32_t(guest.dword(kLaps));
        std::int32_t bestTime = std::int32_t(guest.dword(record + kRacerLaps));
        std::int32_t best = 0;
        for (std::int32_t lap = 0; lap < laps; ++lap)
        {
            const std::int32_t time = std::int32_t(guest.dword(record + kRacerLaps + 4 * lap));
            if (bestTime > time)
            {
                bestTime = time;
                best = lap;
            }
        }
        const std::int32_t top = s16(guest, box), left = s16(guest, box + 2), width = s16(guest, box + 4);
        guest.call(kFieldBox, fe::args({ reg32(left + 2), reg32(top + 2), reg32(width - 3), reg32(15 * laps + 0x20) },
                                       { 0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222 }));
        const reg32 title = guest.call(kLanguageText, fe::args({ kLineLapTimes }));
        t.text(title, left + half(width), top + 8, kAlignCentre, kHotText);
        std::int32_t lineY = top + 0x18;
        reg32 last = reg32(top);
        for (std::int32_t lap = 0; lap < laps; ++lap)
        {
            const std::uint32_t colour = lap == best ? kBestLap : kOtherLap;
            guest.call(kFormatTime, fe::args({ guest.dword(record + kRacerLaps + 4 * lap), buffer }));
            t.text(buffer, left + width - 8, lineY, kAlignRight, colour);
            guest.call(kSprintf, fe::args({}, { buffer, kFormatLapNumber, reg32(lap + 1) }));
            last = guest.call(kDrawText, fe::args({ buffer, reg32(left + 8), reg32(lineY), kFont }, { kAlignLeft, colour }));
            lineY += 0xf;
        }
        return last;
    }
    return reg32(t.rows());
}

/* The last column: the laps done in a duel, the result otherwise. */
void lastColumnText(const Table& t, reg32 record, reg32 buffer)
{
    Guest& guest = t.g;
    if (guest.dword(kRaceType) == kRaceDuel)
        guest.call(kSprintf, fe::args({}, { buffer, kFormatNumber, guest.dword(record + kRacerLapsDone) }));
    else
        guest.call(kRacerResult, fe::args({ record, buffer }));
}

/* One racer's row in the results' plain form. */
void drawResultsRowPlain(const Table& t, reg32 record, reg32 buffer, std::int32_t rowY, bool dimmed)
{
    Guest& guest = t.g;
    const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    const reg32 place = guest.call(kLanguageText, fe::args({ kLineNoPlace }));
    t.text(place, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest.call(kRacerName, fe::args({ record, buffer }));
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest.call(kRacerCar, fe::args({ record, buffer }));
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    const reg32 out = guest.call(kLanguageText, fe::args({ kLineNoTime }));
    t.text(out, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
    x += t.width(2);
    lastColumnText(t, record, buffer);
    t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
}

/* One racer's row in the results; `popup`, a column of it dimmed under its popup. */
void drawResultsRow(const Table& t, reg32 record, reg32 buffer, std::int32_t rowY, bool dimmed, reg32 popup)
{
    Guest& guest = t.g;
    const std::uint32_t status = guest.dword(record + kRacerStatus);
    const bool wrecked = status == kStatusWrecked;
    const std::uint32_t colour = dimmed ? kDimmedText : (status == kStatusOut || wrecked) ? kOutText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    if (wrecked)
        guest.copyString(buffer, guest.call(kLanguageText, fe::args({ kLineNoPlace })));
    else
        guest.call(kRacerPlace, fe::args({ record, buffer }));
    t.text(buffer, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest.call(kRacerName, fe::args({ record, buffer }));
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest.call(kRacerCar, fe::args({ record, buffer }));
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    if (wrecked)
    {
        const reg32 line = guest.call(kLanguageText, fe::args({ kLineWrecked }));
        t.text(line, x + half(t.width(2)), rowY, kAlignCentre, colour);
        x += t.width(2);
        lastColumnText(t, record, buffer);
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
        return;
    }
    guest.call(kFormatTime, fe::args({ guest.dword(record + kRacerTime), buffer }));
    t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, popup == 1 ? kDimmedText : colour);
    x += t.width(2);
    if (guest.dword(kRaceType) == kRaceDuel)
    {
        guest.call(kSprintf, fe::args({}, { buffer, kFormatOfTotal, guest.dword(record + kRacerLapsDone), guest.dword(kRaceLaps) }));
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
    }
    else
    {
        guest.call(kRacerResult, fe::args({ record, buffer }));
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, popup == 2 ? kDimmedText : colour);
    }
}

/* sub_46d3c0: the results table: sliding, sorted as chosen, its box and lines,
 * the titles, a row for each racer shown, the popups, the arrows. */
void drawResults(Guest& guest, reg32 item, bool pointing)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest.call(0x46e0d0, fe::args({ item }));
    const bool dimmed = guest.call(kKeysTaken) != 0;
    switch (guest.word(item + 0x86))
    {
    case 0:
    case 2:
        guest.call(kSortResults, fe::args({ 0, 1, 0 }));
        break;
    case 1:
        guest.call(kSortResults, fe::args({ 0, 8, 0 }));
        break;
    case 3:
        guest.call(kSortResults, fe::args({ 0, guest.dword(kRaceType) == kRaceDuel ? 6u : 2u, 1 }));
        break;
    default:
        break;
    }
    const bool plain = guest.call(kFinishers) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const std::int32_t shownRows = smaller(t.rows(), count);
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2) + t.width(3);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const bool canPoint = pointing && !plain;
    const reg32 popup = canPoint ? guest.call(0x46cdf0, fe::args({ item })) : 0;
    const std::int32_t x = t.x(), y = t.y();
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    if (t.rows() < count)
        guest.call(kFilledBox, fe::args({ reg32(s16(guest, item + 0x6a) - 3), reg32(s16(guest, item + 0x68) - 1), 0x12, reg32(height + 4) }, { kGold }));
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height + 2, 3);
    if (t.state() == kSlideNone && count > 1 && !plain)
        guest.call(0x46cdc0, fe::args({ item }));

    static constexpr std::uint32_t kTitles[] = { 227, 200, 40, 41 };
    static constexpr std::uint32_t kAligns[] = { kAlignCentre, kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 4; ++column)
    {
        reg32 title = guest.call(kLanguageText, fe::args({ kTitles[column] }));
        if (guest.dword(kRaceType) == kRaceDuel && column == 3)
            title = guest.call(kLanguageText, fe::args({ kLineLaps }));
        t.title(column, title, kAligns[column], canPoint, dimmed, 1);
    }

    std::int32_t rowY = y + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = guest.call(kNthResult, fe::args({ 0, reg32(place), 0, 0 }));
        if (record == 0)
            break;
        if (plain)
            drawResultsRowPlain(t, record, buffer, rowY, dimmed);
        else
            drawResultsRow(t, record, buffer, rowY, dimmed, popup);
        rowY += t.rowHeight();
    }
    if (canPoint)
    {
        guest.call(0x46d180, fe::args({ item }));
        guest.call(0x46cf30, fe::args({ item }));
    }
    t.arrows(count, 1);
}

/* sub_46df90: the results as their screen opens: the arrows' art named, sorted
 * by place, the player's row in view. */
reg32 resultsOnShow(Guest& guest, reg32 item)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    t.placeIfNew();
    guest.setWord(item + 0x86, 0);
    guest.setWord(item + 0x84, 0);
    const reg32 sorted = guest.call(kSortResults, fe::args({ 0, 1, 0 }));
    return t.scrollToPlayer(count, sorted);
}

/* sub_46ef90: the same for the standings, sorted by their third column. */
reg32 standingsOnShow(Guest& guest, reg32 item)
{
    const Table t(guest, item, kStandingsFields, kStandingsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    guest.setWord(item + 0x7c, 2);
    guest.setWord(item + 0x7a, 0);
    t.placeIfNew();
    const reg32 sorted = guest.call(kSortResults, fe::args({ 0, 4, 1 }));
    return t.scrollToPlayer(count, sorted);
}

/* sub_46fe30: the same for the second list, which its draw sorts. */
reg32 roundsOnShow(Guest& guest, reg32 item)
{
    const Table t(guest, item, kStandingsFields, kRoundsType);
    if (!t.shown())
        return 0;
    t.loadArrows();
    const reg32 lastShape = guest.eax();
    guest.setWord(item + 0x7c, 2);
    const bool placed = t.placeIfNew();
    guest.setWord(item + 0x7a, 0);
    return placed ? withLow(lastShape, guest.word(item + kItemX)) : lastShape;
}

/* sub_46e0d0: the results' boxes as the table stands now: a title's for each
 * column, the whole table's, the arrows'.  What is left in eax is the last sum
 * they were made with. */
reg32 layoutResults(Guest& guest, reg32 item)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    if (!t.shown())
        return 0;
    const std::int32_t shownRows = smaller(t.rows(), std::int32_t(guest.dword(kResultsCount)));
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2) + t.width(3);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const std::uint16_t top = guest.word(item + kItemY);
    const std::uint16_t x = guest.word(item + 0x8a);
    std::uint16_t left = x;
    for (int column = 0; column < 4; ++column)
    {
        const reg32 box = t.box(column);
        guest.setWord(box, top);
        guest.setWord(box + 2, left);
        guest.setWord(box + 4, guest.word(item + kColumns + 2 * column));
        guest.setWord(box + 6, guest.word(item + 0x44));
        left = std::uint16_t(left + guest.word(item + kColumns + 2 * column));
    }
    guest.setWord(item + kItemBoundLeft, x);
    guest.setWord(item + kItemBoundTop, top);
    guest.setWord(item + kItemBoundWidth, std::uint16_t(tableWidth + 0x20));
    guest.setWord(item + kItemBoundHeight, std::uint16_t(height));
    const std::uint16_t arrowLeft = std::uint16_t(x + tableWidth + 0x14);
    guest.setWord(item + 0x68, top);
    guest.setWord(item + 0x6a, arrowLeft);
    guest.setWord(item + 0x6c, kArrowSize);
    guest.setWord(item + 0x6e, kArrowSize);
    guest.setWord(item + 0x70, std::uint16_t(top + height - kArrowSize));
    guest.setWord(item + 0x72, arrowLeft);
    guest.setWord(item + 0x74, kArrowSize);
    guest.setWord(item + 0x76, kArrowSize);
    const reg32 first = withLow(reg32(tableWidth + 0x20), x) + reg32(tableWidth) + 0x14;
    const reg32 second = withLow(first, x) + reg32(tableWidth) + 0x14;
    return withLow(second, top) + reg32(height) - kArrowSize;
}

/* sub_46f0c0, sub_46ff00: the same for the three columns of the others. */
reg32 layoutStandings(Guest& guest, reg32 item, std::uint32_t type, reg32 countAt)
{
    const Table t(guest, item, kStandingsFields, type);
    if (!t.shown())
        return 0;
    const std::int32_t shownRows = smaller(t.rows(), std::int32_t(guest.dword(countAt)));
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2);
    const std::int32_t height = (shownRows + 1) * t.rowHeight();
    const std::uint16_t top = guest.word(item + kItemY);
    const std::uint16_t x = guest.word(item + 0x80);
    std::uint16_t left = x;
    for (int column = 0; column < 3; ++column)
    {
        const reg32 box = t.box(column);
        guest.setWord(box, top);
        guest.setWord(box + 2, left);
        guest.setWord(box + 4, guest.word(item + kColumns + 2 * column));
        guest.setWord(box + 6, guest.word(item + 0x42));
        left = std::uint16_t(left + guest.word(item + kColumns + 2 * column));
    }
    guest.setWord(item + kItemBoundLeft, x);
    guest.setWord(item + kItemBoundTop, top);
    guest.setWord(item + kItemBoundWidth, std::uint16_t(tableWidth + 0x20));
    guest.setWord(item + kItemBoundHeight, std::uint16_t(height));
    const std::uint16_t arrowLeft = std::uint16_t(x + tableWidth + 0x14);
    guest.setWord(item + 0x5e, top);
    guest.setWord(item + 0x60, arrowLeft);
    guest.setWord(item + 0x62, kArrowSize);
    guest.setWord(item + 0x64, kArrowSize);
    guest.setWord(item + 0x66, std::uint16_t(top + height - kArrowSize));
    guest.setWord(item + 0x68, arrowLeft);
    guest.setWord(item + 0x6a, kArrowSize);
    guest.setWord(item + 0x6c, kArrowSize);
    const reg32 first = withLow(reg32(tableWidth + 0x20), x) + reg32(tableWidth) + 0x14;
    const reg32 second = reg32(tableWidth) + withLow(first, x) + 0x14;
    return withLow(second, top) + reg32(height) - kArrowSize;
}

constexpr std::uint16_t kEnter = 0x0d, kKeyRight = 0x4d00, kKeyLeft = 0x4b00, kKeyPageUp = 0x4900, kKeyPageDown = 0x5100;

void click(Guest& guest)
{
    guest.call(kMenuEffect, fe::args({ kClick, kClickVolume }));
}

/* sub_46e270: the results' keys: Enter on a title sorts by it, left and right
 * go along the titles, the arrows scroll.  2 when a key was used; it leaves
 * that in edx too, and in ebx where it has got to. */
reg32 resultsKey(Guest& guest, reg32 item, std::uint16_t key)
{
    const Table t(guest, item, kResultsFields, kResultsType);
    reg32 ebx = 0xffffffff;
    guest.leaveEbx(ebx);
    if (!t.shown())
    {
        guest.leaveEdx(0);
        return 0;
    }
    const bool plain = guest.call(kFinishers) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const bool sortable = count > 1 && !plain;
    for (int column = 0; column < 4; ++column)
    {
        if (guest.call(kPointerInBox, fe::args({ t.box(column) })) != 0)
        {
            ebx = reg32(column);
            break;
        }
    }
    bool used = false;
    if (key == kEnter && sortable && ebx <= 3)
    {
        guest.setWord(item + 0x86, std::uint16_t(ebx));
        used = true;
        click(guest);
    }
    if (sortable && (key == kKeyRight || key == kKeyLeft))
    {
        std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
        column = column < 0 ? 0 : column > 3 ? 3 : column;
        ebx = reg32(column);
        pointAtBox(guest, t.box(column));
        used = true;
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x84) != 0)
        {
            click(guest);
            ebx = withLow(ebx, guest.word(item + 0x84)) - 1;
            guest.setWord(item + 0x84, std::uint16_t(ebx));
            used = true;
            pointAtBox(guest, item + 0x68);
        }
        if (key == kKeyPageDown)
        {
            ebx = reg32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(guest);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
                pointAtBox(guest, item + 0x70);
            }
        }
        if (key == kEnter)
        {
            if (guest.call(kPointerInBox, fe::args({ item + 0x68 })) != 0 && guest.word(item + 0x84) != 0)
            {
                click(guest);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) - 1));
                used = true;
            }
            if (guest.call(kPointerInBox, fe::args({ item + 0x70 })) != 0 && t.rows() + t.scroll() < count)
            {
                click(guest);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
            }
        }
    }
    const reg32 result = used ? 2 : 0;
    guest.leaveEbx(ebx);
    guest.leaveEdx(result);
    return result;
}

/* sub_46f200, sub_470040: the keys of the others.  sub_470040 takes left and
 * right both as the next title, and points with that one's width and height
 * at the top and left of the one after it: the game's own slip. */
reg32 standingsKey(Guest& guest, reg32 item, std::uint16_t key, std::uint32_t type, reg32 countAt)
{
    const bool rounds = type == kRoundsType;
    const Table t(guest, item, kStandingsFields, type);
    reg32 ebx = 0xffffffff;
    guest.leaveEbx(ebx);
    if (!t.shown())
    {
        guest.leaveEdx(0);
        return 0;
    }
    const bool plain = guest.call(kFinishers) != 0;
    const std::int32_t count = std::int32_t(guest.dword(countAt));
    for (int column = 0; column < 3; ++column)
    {
        if (guest.call(kPointerInBox, fe::args({ t.box(column) })) != 0)
        {
            ebx = reg32(column);
            break;
        }
    }
    bool used = false;
    if (key == kEnter && !plain && std::int32_t(ebx) >= 0)
    {
        guest.setWord(item + 0x7c, std::uint16_t(ebx));
        used = true;
        click(guest);
    }
    if (!plain && (key == kKeyRight || key == kKeyLeft))
    {
        used = true;
        if (rounds)
        {
            ebx = reg32((std::int32_t(ebx) + 1) % 3);
            const reg32 box = t.box(int(ebx));
            setPointer(guest, s16(guest, box + 10) + (3 * s16(guest, box + 4)) / 4, s16(guest, box + 8) + half(s16(guest, box + 6)));
        }
        else
        {
            std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
            column = column < 0 ? 0 : column > 2 ? 2 : column;
            ebx = reg32(column) << 3;
            pointAtBox(guest, t.box(column));
        }
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x7a) != 0)
        {
            click(guest);
            ebx = withLow(ebx, guest.word(item + 0x7a)) - 1;
            guest.setWord(item + 0x7a, std::uint16_t(ebx));
            used = true;
            pointAtBox(guest, item + 0x5e);
        }
        if (key == kKeyPageDown)
        {
            ebx = reg32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(guest);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) + 1));
                used = true;
                pointAtBox(guest, item + 0x66);
            }
        }
        if (key == kEnter)
        {
            if (guest.call(kPointerInBox, fe::args({ item + 0x5e })) != 0 && guest.word(item + 0x7a) != 0)
            {
                click(guest);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) - 1));
                used = true;
            }
            if (guest.call(kPointerInBox, fe::args({ item + 0x66 })) != 0 && t.rows() + t.scroll() < count)
            {
                click(guest);
                const std::uint16_t scroll = guest.word(item + 0x7a);
                if (!rounds)
                    ebx = withLow(ebx, scroll) + 1;
                guest.setWord(item + 0x7a, std::uint16_t(scroll + 1));
                used = true;
            }
        }
    }
    const reg32 result = used ? 2 : 0;
    guest.leaveEbx(ebx);
    guest.leaveEdx(result);
    return result;
}

/* The standings' box: its background and lines, as sub_46e690 and sub_46f510
 * both draw it. */
void drawStandingsBox(const Table& t, std::int32_t shownRows, std::int32_t tableWidth, std::int32_t height, std::int32_t count,
                      int columnLines)
{
    Guest& guest = t.g;
    const std::int32_t x = t.x(), y = t.y();
    guest.call(kFilledBox, fe::args({ reg32(x), reg32(y), reg32(tableWidth + 2), reg32(height + 2) }, { kBlack }));
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    if (t.rows() < count)
        guest.call(kFilledBox, fe::args({ reg32(s16(guest, t.item + 0x60) - 3), reg32(s16(guest, t.item + 0x5e) - 3), 0x12, reg32(height + 6) }, { kGold }));
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height, columnLines);
}

/* sub_46e690: a series' standings: each racer's name and car, his rank and
 * place, and his other rank. */
void drawStandings(Guest& guest, reg32 item, bool pointing)
{
    const Table t(guest, item, kStandingsFields, kStandingsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    const reg32 placeName = guest.frame() + 0x100;    // char[0x100]
    const reg32 firstRank = guest.frame() + 0x210;
    const reg32 secondRank = guest.frame() + 0x214;
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest.call(0x46f0c0, fe::args({ item }));
    const bool dimmed = guest.call(kKeysTaken) != 0;
    const bool plain = guest.call(kFinishers) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        guest.call(kSortResults, fe::args({ 0, 8, 0 }));
        break;
    case 1:
        guest.call(kSortResults, fe::args({ 0, 3, 1 }));
        break;
    case 2:
        guest.call(kSortResults, fe::args({ 0, 4, 1 }));
        break;
    default:
        break;
    }
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const std::int32_t shownRows = smaller(t.rows(), count);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2);
    drawStandingsBox(t, shownRows, tableWidth, height, count, 3);
    if (!plain && t.state() == kSlideNone)
        guest.call(0x46e660, fe::args({ item }));

    static constexpr std::uint32_t kTitles[] = { 200, 239, 240 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const reg32 title = guest.call(kLanguageText, fe::args({ kTitles[column] }));
        t.title(column, title, kAligns[column], !plain && pointing, dimmed, 0);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = guest.call(kNthResult, fe::args({ 0, reg32(place), 0, 0 }));
        if (record == 0)
            break;
        const bool out = guest.dword(record + kRacerStatus) == kStatusOut;
        guest.call(kRacerRanks, fe::args({ record, firstRank, secondRank }));
        const std::uint32_t colour = dimmed ? kDimmedText : (!plain && out) ? kOutText : kRowText;
        const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
        std::int32_t x = t.x();
        guest.call(kRacerName, fe::args({ record, buffer }));
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
        guest.call(kRacerCar, fe::args({ record, buffer }));
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const reg32 line = guest.call(kLanguageText, fe::args({ kLineNoTime }));
            t.text(line, x + half(t.width(1)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            guest.call(kRacerPlace, fe::args({ record, placeName }));
            guest.call(kSprintf, fe::args({}, { buffer, kFormatNumberAndName, guest.dword(firstRank), placeName }));
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
        }
        x += t.width(1);
        guest.call(kSprintf, fe::args({}, { buffer, kFormatNumber, guest.dword(secondRank) }));
        t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
        rowY += t.rowHeight();
    }
    t.arrows(count, 0);
}

/* sub_46f510: the second list of racers: each one's name and car, a count of
 * his against the count at 0x6fd4b8, his time once he has reached it. */
void drawRounds(Guest& guest, reg32 item, bool pointing)
{
    const Table t(guest, item, kStandingsFields, kRoundsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest.call(0x46ff00, fe::args({ item }));
    const bool dimmed = guest.call(kKeysTaken) != 0;
    const bool plain = guest.call(kFinishers) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        guest.call(kSortResults, fe::args({ 1, 8, 0 }));
        break;
    case 1:
        guest.call(kSortResults, fe::args({ 1, 5, 1 }));
        break;
    case 2:
        guest.call(kSortResults, fe::args({ 1, 7, 0 }));
        break;
    default:
        break;
    }
    const std::int32_t count = std::int32_t(guest.dword(kRoundsCount));
    const std::int32_t shownRows = smaller(t.rows(), count);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2);
    drawStandingsBox(t, shownRows, tableWidth, height, count, 2);
    if (!plain && t.state() == kSlideNone)
        guest.call(0x46f4e0, fe::args({ item }));

    static constexpr std::uint32_t kTitles[] = { 200, 247, 405 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const reg32 title = guest.call(kLanguageText, fe::args({ kTitles[column] }));
        t.title(column, title, kAligns[column], pointing && !plain, dimmed, 1);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = guest.call(kNthResult, fe::args({ 1, reg32(place), 0, 0 }));
        if (record == 0)
            break;
        const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
        std::int32_t x = t.x();
        guest.call(kRacerName, fe::args({ record, buffer }));
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, colour);
        guest.call(kRacerCar, fe::args({ record, buffer }));
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const reg32 none = guest.call(kLanguageText, fe::args({ kLineNoPlace }));
            t.text(none, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            const reg32 line = guest.call(kLanguageText, fe::args({ kLineNoTime }));
            t.text(line, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            const std::int32_t won = std::int32_t(guest.dword(record + kRacerRoundsDone));
            const std::int32_t toWin = std::int32_t(guest.dword(kRoundsToWin));
            if (won >= toWin)
            {
                const reg32 winner = guest.call(kLanguageText, fe::args({ kLineWinner }));
                guest.call(kSprintf, fe::args({}, { buffer, kFormatOfTotalAndName, reg32(won), reg32(toWin), winner }));
            }
            else
            {
                guest.call(kSprintf, fe::args({}, { buffer, kFormatOfTotal, reg32(won), reg32(toWin) }));
            }
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            if (won >= toWin)
            {
                guest.call(kFormatTime, fe::args({ guest.dword(record + kRacerRoundsTime), buffer }));
                t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
            }
            else
            {
                const reg32 none = guest.call(kLanguageText, fe::args({ kLineNoPlace }));
                t.text(none, x + half(t.width(2)), rowY, kAlignCentre, colour);
            }
        }
        rowY += t.rowHeight();
    }
    t.arrows(count, 0);
}

/* ---------------------------------------------------------------------------
 * The players side by side
 * ------------------------------------------------------------------------- */

constexpr reg32 kSideState = 0x6c, kSideX = 0x6e;
constexpr reg32 kSideRowHeight = 0x42;
constexpr reg32 kSideTitles = 0x44;            // two texts for each of five rows
constexpr reg32 kTrackKind = 0x6fbc68;         // 3 and 4 take a row's second text and values
constexpr int kSideRows = 5;
constexpr reg32 kRacerBests[] = { 0x268, 0x270, 0x278, 0x280 };

/* sub_470360: where the table stands, the first time. */
reg32 sideBySideOnShow(Guest& guest, reg32 item)
{
    const reg32 shown = guest.call(kItemShownNow, fe::args({ item, kSideBySideType }));
    if (shown != 0 && guest.word(item + kSideX) == 0)
    {
        guest.setWord(item + kSideState, 0);
        const std::uint16_t x = guest.word(item + kItemX);
        guest.setWord(item + kSideX, x);
        return withLow(shown, x);
    }
    return shown;
}

/* sub_4703a0: a column for this player, and in split screen one for the other:
 * his name, two lines of his car's, four times, or kNoValue for one he has not. */
void drawSideBySide(Guest& guest, reg32 item)
{
    const reg32 buffer = guest.frame();               // char[0x100]
    if (guest.call(kItemShownNow, fe::args({ item, kSideBySideType })) == 0)
        return;
    if (guest.word(item + kSideState) == kSlideOut)
    {
        const std::int16_t moved = std::int16_t(guest.word(item + kSideX) + kSlideStep);
        guest.setWord(item + kSideX, std::uint16_t(moved));
        if (moved >= kSlidOut)
        {
            guest.setWord(item + kSideX, kSlidOut);
            guest.setWord(item + kSideState, kSlideGone);
        }
    }
    if (guest.word(item + kSideState) == kSlideIn)
    {
        guest.setWord(item + kSideX, std::uint16_t(guest.word(item + kSideX) - kSlideStep));
        if (s16(guest, item + kSideX) <= s16(guest, item + kItemX))
        {
            guest.setWord(item + kSideState, kSlideNone);
            guest.setWord(item + kSideX, guest.word(item + kItemX));
        }
    }
    const bool dimmed = guest.call(kKeysTaken) != 0;
    const std::int32_t lastPlayer = guest.dword(kPlayers) == 1 ? 1 : 0;
    const std::uint32_t kind = guest.dword(kTrackKind);
    const reg32 second = kind == 3 || kind == 4 ? 1 : 0;
    const std::int32_t titles = s16(guest, item + kColumns);
    const std::int32_t rowHeight = s16(guest, item + kSideRowHeight);
    const std::int32_t tableWidth = titles + s16(guest, item + kColumns + 2) + (lastPlayer ? s16(guest, item + kColumns + 4) : 0);
    const std::int32_t height = 6 * rowHeight;
    const std::int32_t x = s16(guest, item + kSideX), y = s16(guest, item + kItemY);
    auto shaded = [&](std::int32_t bx, std::int32_t by, std::int32_t w, std::int32_t h, std::uint32_t c1, std::uint32_t c2,
                      std::uint32_t c3, std::uint32_t c4) {
        guest.call(kShadedBox, fe::args({ reg32(bx), reg32(by), reg32(w), reg32(h) }, { c1, c2, c3, c4 }));
    };
    guest.call(kFilledBox, fe::args({ reg32(x), reg32(y), reg32(tableWidth + 2), reg32(height + 2) }, { kBlack }));
    shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    shaded(x, y + rowHeight - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    std::int32_t lineX = x;
    for (std::int32_t i = 0; i <= lastPlayer; ++i)
    {
        lineX += s16(guest, item + kColumns + 2 * i);
        shaded(lineX, y, 1, height + 2, kBlue, kBlue, kGold, kGold);
    }
    std::int32_t lineY = y + rowHeight;
    for (int i = 0; i < kSideRows; ++i)
    {
        shaded(x, lineY, tableWidth, 1, kBlue, kGold, kGold, kBlue);
        lineY += rowHeight;
    }

    const std::uint32_t titleColour = dimmed ? kDimmedText : kGold;
    std::int32_t titleY = y + rowHeight + half(rowHeight);
    for (int i = 0; i < kSideRows; ++i)
    {
        guest.call(kDrawText, fe::args({ guest.dword(item + kSideTitles + 8 * i + 4 * second), reg32(x + 10),
                                         reg32(titleY - 10), kFont }, { kAlignLeft, titleColour }));
        titleY += rowHeight;
    }

    std::int32_t columnX = x + titles;
    const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
    const std::uint32_t nameColour = dimmed ? kDimmedText : kPlayerText;
    for (std::int32_t player = 0; player <= lastPlayer; ++player)
    {
        const std::int32_t room = s16(guest, item + kColumns + 2 + 2 * player);
        const std::int32_t centre = columnX + half(room);
        std::int32_t rowY = half(rowHeight) + y - 5;
        reg32 record = guest.call(kPlayerResult, fe::args({ 0, guest.dword(kThisPlayer) + player }));
        if (record == 0)
            record = guest.call(kPlayerResult, fe::args({ 1, guest.dword(kThisPlayer) + player }));
        guest.call(kRacerName, fe::args({ record, buffer }));
        guest.call(kDrawText, fe::args({ buffer, reg32(centre), reg32(rowY), kFont }, { kAlignCentre, nameColour }));
        rowY += rowHeight;
        guest.call(kRacerFirstLine, fe::args({ record, buffer }));
        guest.call(kDrawTextFitted, fe::args({ buffer, reg32(centre), reg32(rowY - 10), kFont }, { kAlignCentre, colour, reg32(room) }));
        guest.call(kRacerSecondLine, fe::args({ record, buffer }));
        guest.call(kDrawTextFitted, fe::args({ buffer, reg32(centre), reg32(rowY + 5), kFont }, { kAlignCentre, colour, reg32(room) }));
        rowY += rowHeight;
        for (const reg32 field : kRacerBests)
        {
            const reg32 value = guest.dword(record + field + 4 * second);
            if (value != 0)
            {
                guest.call(kFormatTime, fe::args({ value, buffer }));
                guest.call(kDrawText, fe::args({ buffer, reg32(centre), reg32(rowY), kFont }, { kAlignCentre, colour }));
            }
            else
            {
                guest.call(kDrawText, fe::args({ kNoValue, reg32(centre), reg32(rowY), kFont }, { kAlignCentre, kDimmedText }));
            }
            rowY += rowHeight;
        }
        columnX += room;
    }
}

/* ---------------------------------------------------------------------------
 * The menus' art
 * ------------------------------------------------------------------------- */

constexpr reg32 kChromeTexture = 0x55a4d0;
constexpr reg32 kChromeBuffer = 0x55a4d4;
constexpr reg32 kChromeBufferSize = 0x55a4d8;
constexpr reg32 kGlowTexture = 0x678a54;
constexpr reg32 kGlowTransform = 0x678a58;
constexpr reg32 kGlowMode = 0x678a78;
constexpr reg32 kAllocateVram = 0x4d5f40;
constexpr reg32 kFindShape = 0x4ef1e0;
constexpr reg32 kShapeTexture = 0x4d6070;      // eax where, edx the shape, ebx its size, ecx its format
constexpr reg32 kDisplayFlags = 0x7a3a58;
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr reg32 kArtChrome = 0x53abb8, kArtGlow = 0x53abc0;
constexpr std::uint32_t kOne = 0x3f800000;

/* sub_465d60: no chrome, no glow. */
void clearMenuArt(Guest& guest)
{
    guest.setDword(kGlowTexture, 0);
    guest.setDword(kChromeBuffer, 0);
    guest.setDword(kChromeTexture, 0);
}

/* A shape if it is square, of the size it should be. */
bool isSquareOf(const Guest& guest, reg32 shape, std::uint16_t size)
{
    return guest.word(shape + 4) == size && guest.word(shape + 6) == size;
}

/* sub_465d80: the chrome and the glow, from the screen's art: textures of 256
 * and 32 pixels, or none if the art has none of that size.  The glow's
 * corners are the texture's. */
reg32 loadMenuArt(Guest& guest, reg32 art)
{
    clearMenuArt(guest);
    if (art == 0)
        return 0;
    const reg32 glowTexture = guest.frame();
    guest.setDword(kChromeBufferSize, 4);
    guest.setDword(kChromeBuffer, guest.call(kAllocateVram, fe::args({ 0x100, kChromeBufferSize, 0 })));
    const reg32 chrome = guest.call(kFindShape, fe::args({ art, kArtChrome }));
    if (chrome != 0 && isSquareOf(guest, chrome, 0x100))
    {
        const std::uint32_t format = (guest.byte(kDisplayFlags) & kNoTranslucency) ? 4 : 0;
        guest.call(kShapeTexture, fe::args({ kChromeTexture, chrome, 0x100, format }));
    }
    else
    {
        guest.setDword(kChromeTexture, 0);
    }
    const reg32 glow = guest.call(kFindShape, fe::args({ art, kArtGlow }));
    if (glow != 0 && isSquareOf(guest, glow, 0x20))
    {
        guest.call(kShapeTexture, fe::args({ glowTexture, glow, 0x20, 0 }));
        const reg32 texture = guest.dword(glowTexture);
        guest.setDword(kGlowTexture, texture);
        static constexpr std::uint32_t kCorners[] = { 0, 0, kOne, 0, kOne, kOne, 0, kOne };
        for (std::size_t i = 0; i < std::size(kCorners); ++i)
            guest.setDword(kGlowTransform + 4 * reg32(i), kCorners[i]);
        guest.setByte(kGlowMode, 0xc);
        return texture;
    }
    guest.setDword(kGlowTexture, 0);
    return glow;
}

/* ---------------------------------------------------------------------------
 * The front end's ends
 * ------------------------------------------------------------------------- */

constexpr reg32 kStartScreens = 0x4daf80;
constexpr reg32 kSkipIntro = 0x6fbb54;
constexpr reg32 kLanguageIndex = 0x7a3a50;
constexpr reg32 kArtFolder = 0x7a2db4;
constexpr reg32 kLanguageCodes = 0x4721b8;     // char[7], one a language
constexpr reg32 kFormatTitle = 0x53b030;       // "%stitle_%c.qfs"
constexpr reg32 kBack = 0x53b040;
constexpr reg32 kReadFile = 0x4956f0;
constexpr std::uint32_t kLoadFlags = 0x10;
constexpr reg32 kShapeTextures = 0x4d6390;
constexpr reg32 kTitleTexture = 0x8b7638;
constexpr reg32 kBeginDraw = 0x4d9120, kEndDraw = 0x4d9140;
constexpr reg32 kFree = 0x4e1890;
constexpr reg32 kElapsed = 0x4f0650;
constexpr reg32 kPollKeysOff = 0x451940;
constexpr reg32 kMenuKey = 0x451960;
constexpr reg32 kDescriptionService = 0x4c9860;
constexpr std::int32_t kTitleTicks = 0x180;
constexpr reg32 kInputMode = 0x56438c;
constexpr reg32 kInputFlags = 0x564390;
constexpr reg32 kSavedInputMode = 0x679008;
constexpr reg32 kSavedInputFlags = 0x679004;
constexpr reg32 kAfterRace = 0x8b7630;
constexpr reg32 kLoadFlag = 0x55d000;

/* sub_472f20: the title's picture, in the language's. */
reg32 drawTitle(Guest& guest)
{
    const reg32 name = guest.frame();                 // char[0x12c]
    const reg32 codes = guest.frame() + 0x12c;
    for (reg32 i = 0; i < 7; ++i)
        guest.setByte(codes + i, guest.byte(kLanguageCodes + i));
    guest.call(kStartScreens);
    const std::uint32_t code = guest.byte(codes + guest.dword(kLanguageIndex));
    guest.call(kSprintf, fe::args({}, { name, kFormatTitle, kArtFolder, code }));
    const reg32 file = guest.call(kReadFile, fe::args({ name, kLoadFlags }));
    if (file == 0)
        return 0;
    const reg32 shape = guest.call(kFindShape, fe::args({ file, kBack }));
    guest.setDword(kTitleTexture, guest.call(kShapeTextures, fe::args({ shape, 0, 0 })));
    guest.call(kBeginDraw);
    guest.call(kDrawShape, fe::args({ guest.dword(kTitleTexture), 0 }));
    guest.call(kEndDraw);
    return guest.call(kFree, fe::args({ file }));
}

/* sub_472fc0: the title on the screen until a key, or until its time is up. */
reg32 waitOnTitle(Guest& guest)
{
    if (guest.dword(kSkipIntro) == 0)
        guest.call(0x472f20);
    guest.call(kElapsed);
    guest.call(kPollKeysOff, fe::args({ 0 }));
    std::int32_t elapsed = 0;
    for (;;)
    {
        const reg32 key = guest.call(kMenuKey, fe::args({ 1 }));
        if (key != 0 || elapsed >= kTitleTicks)
            return key;
        guest.call(kDescriptionService);
        const reg32 ticks = guest.call(kElapsed);
        elapsed += std::int32_t(ticks);
        if (elapsed > kTitleTicks)
            return ticks;
    }
}

/* sub_473020: into the menus: the input's mode kept and the menus', the sound
 * and the screens taken up. */
reg32 enterMenus(Guest& guest)
{
    guest.call(0x431bc0);
    guest.call(0x431900, fe::args({ 3, 0 }));
    guest.call(0x4d5cc0, fe::args({ 1, 0 }));
    const reg32 mode = guest.dword(kInputMode);
    guest.setDword(kSavedInputMode, mode);
    guest.setDword(kInputMode, 0x10);
    guest.call(0x4f0230, fe::args({ mode, 0x10 }));
    const reg32 flags = guest.dword(kInputFlags);
    guest.setDword(kSavedInputFlags, flags);
    guest.setByte(kInputFlags, std::uint8_t(flags | 4));
    guest.call(0x4136e0, fe::args({ flags }));
    guest.call(kStartScreens);
    return guest.call(0x49c140);
}

/* sub_473080: out of them, all of it undone. */
reg32 leaveMenus(Guest& guest)
{
    guest.call(0x4109d0);
    guest.call(0x410b20, fe::args({ guest.eax(), 0 }));
    guest.setDword(0x55b034, 0);
    guest.call(0x49c2a0);
    guest.setDword(kInputMode, guest.dword(kSavedInputMode));
    const reg32 flags = guest.dword(kSavedInputFlags);
    guest.setDword(kInputFlags, flags);
    guest.call(0x472360, fe::args({ flags }));
    guest.call(0x472820);
    guest.call(0x4d5c40);
    return guest.call(0x4d5d10);
}

/* sub_4732b0: the screens after a race: the voices its kind has, the menu loop
 * on postgame, its answer. */
reg32 runPostgame(Guest& guest)
{
    constexpr reg32 kVoiceLoad = 0x4c9890;
    constexpr reg32 kPostgame = 0x53b054;
    guest.call(0x473020);
    guest.setDword(0x6fd3a0, 0);
    guest.setDword(kAfterRace, 1);
    switch (guest.dword(kRaceType))
    {
    case 1:
        guest.call(kVoiceLoad, fe::args({ guest.dword(0x6fd470) == 0 ? 2u : 3u }));
        break;
    case 2:
        guest.call(kVoiceLoad, fe::args({ 1 }));
        break;
    default:
        guest.call(kVoiceLoad, fe::args({ 4 }));
        break;
    }
    guest.call(kVoiceLoad, fe::args({ 0 }));
    const reg32 answer = guest.call(0x44b230, fe::args({ kPostgame, 0 }));
    guest.call(0x4c9a00);
    guest.call(0x44e7d0);
    guest.setDword(kAfterRace, 0);
    guest.call(0x473080);
    return answer;
}

constexpr std::uint8_t kResultsSaves = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeySaves = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kSortMarkSaves = fe::kRestoreEbx | fe::kRestoreEcx;

}

/* A table's handler that only asks whether it is shown, and answers that. */
#define MENUC_SHOWN(name, address, type)                                                 \
    void frontend::name(win32::WinApplication* app, x86::CPU& cpu)                       \
    {                                                                                    \
        static fe::Site site{ address, 0, 8, nullptr, 0, fe::kRestoreEdx };              \
        const reg32 item = cpu.eax;                                                      \
        fe::runWhole(app, cpu, site, [&](Guest& guest) {                                 \
            return guest.call(kItemShownNow, fe::args({ item, type }));                  \
        });                                                                              \
    }

/* The same, whose answer is always 0. */
#define MENUC_NOTHING(name, address, type)                                               \
    void frontend::name(win32::WinApplication* app, x86::CPU& cpu)                       \
    {                                                                                    \
        static fe::Site site{ address, 0, 8, nullptr, 0, fe::kRestoreEdx };              \
        const reg32 item = cpu.eax;                                                      \
        fe::runWhole(app, cpu, site, [&](Guest& guest) {                                 \
            guest.call(kItemShownNow, fe::args({ item, type }));                         \
            return reg32(0);                                                             \
        });                                                                              \
    }

/* A table's draw: the item, whether the pointer counts; edx and eax 0 after. */
#define MENUC_DRAW(name, address, locals, draw)                                          \
    void frontend::name(win32::WinApplication* app, x86::CPU& cpu)                       \
    {                                                                                    \
        static fe::Site site{ address, 0, 20 + locals, nullptr, 0, kResultsSaves };      \
        const reg32 item = cpu.eax;                                                      \
        const bool pointing = cpu.edx != 0;                                              \
        fe::runWhole(app, cpu, site, [&](Guest& guest) {                                 \
            draw(guest, item, pointing);                                                 \
            guest.leaveEdx(0);                                                           \
            return reg32(0);                                                             \
        });                                                                              \
    }

/* A function of the item alone, in a frame of its size. */
#define MENUC_OF_ITEM(name, address, frameBytes, body)                                   \
    void frontend::name(win32::WinApplication* app, x86::CPU& cpu)                       \
    {                                                                                    \
        static fe::Site site{ address, 0, frameBytes, nullptr, 0 };                      \
        const reg32 item = cpu.eax;                                                      \
        fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, item); });   \
    }

/* A function of no arguments and the registers it saves. */
#define MENUC_PLAIN(name, address, frameBytes, saves, body)                              \
    void frontend::name(win32::WinApplication* app, x86::CPU& cpu)                       \
    {                                                                                    \
        static fe::Site site{ address, 0, frameBytes, nullptr, 0, saves };               \
        fe::runWhole(app, cpu, site, [](Guest& guest) { return body(guest); });          \
    }

MENUC_SHOWN(resultsTablePass, "sub_46e0b0", kResultsType)
MENUC_SHOWN(standingsTablePass, "sub_46f0a0", kStandingsType)
MENUC_SHOWN(roundsTablePass, "sub_46fee0", kRoundsType)
MENUC_NOTHING(resultsTableHidden, "sub_46e640", kResultsType)
MENUC_NOTHING(standingsTableHidden, "sub_46f4c0", kStandingsType)
MENUC_NOTHING(roundsTableHidden, "sub_470340", kRoundsType)
MENUC_DRAW(resultsTableDraw, "sub_46d3c0", 0x174, drawResults)
MENUC_DRAW(standingsTableDraw, "sub_46e690", 0x270, drawStandings)
MENUC_DRAW(roundsTableDraw, "sub_46f510", 0x16c, drawRounds)
MENUC_OF_ITEM(resultsPopupColumn, "sub_46cdf0", 24 + 8, resultsPopupUnderPointer)
MENUC_OF_ITEM(resultsLapTimesPopup, "sub_46d180", 24 + 0x114, drawLapTimesPopup)
MENUC_OF_ITEM(resultsTableShown, "sub_46df90", 24 + 4, resultsOnShow)
MENUC_OF_ITEM(standingsTableShown, "sub_46ef90", 24 + 4, standingsOnShow)
MENUC_OF_ITEM(resultsTableLayout, "sub_46e0d0", 24, layoutResults)
MENUC_OF_ITEM(loadMenuArtSlots, "sub_465d80", 24 + 4, loadMenuArt)
MENUC_PLAIN(titleDraw, "sub_472f20", 24 + 0x134, fe::kRestoreAll, drawTitle)
MENUC_PLAIN(titleWait, "sub_472fc0", 8, fe::kRestoreEdx, waitOnTitle)
MENUC_PLAIN(menusEnter, "sub_473020", 8, fe::kRestoreEdx, enterMenus)
MENUC_PLAIN(menusLeave, "sub_473080", 8, fe::kRestoreEdx, leaveMenus)
MENUC_PLAIN(postgameRun, "sub_4732b0", 16, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi, runPostgame)

/* sub_45cb50: the HUD editor's element, hidden: nothing to do. */
void frontend::hudElementHidden(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45cb50", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.eax(); });
}

/* sub_470390: the side by side table has nothing to do on a pass. */
void frontend::headToHeadPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470390", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.eax(); });
}

/* sub_473010: a comparison with the intro's flag, for no one. */
void frontend::skipIntroCompare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_473010", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.eax(); });
}

/* sub_460760: the buttons' pulse starts again from its brightest, as the
 * selected item changes. */
void frontend::resetButtonPulse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_460760", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.setDword(0x559fdc, 0xff);
        return guest.eax();
    });
}

/* sub_4654c0: the two values on the stack kept, in the slots at 0x678954.  What
 * is left in eax is the first. */
void frontend::setSliderComponents(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4654c0", 8, 4, nullptr, 0, 0 };
    const reg32 first = cpu.esp + 4, second = cpu.esp + 8;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        guest.setDword(0x678958, guest.dword(second));
        guest.setDword(0x678954, guest.dword(first));
        return guest.dword(first);
    });
}

void frontend::clearMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_465d60", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        clearMenuArt(guest);
        return guest.eax();
    });
}

void frontend::drawColumnSortMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46cce0", 12, 36, nullptr, 0 };
    const reg32 arguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return drawSortMark(guest, guest.dword(arguments), guest.dword(arguments + 4), guest.dword(arguments + 8));
    });
}

void frontend::resultsSortMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46cdc0", 0, 12, nullptr, 0, kSortMarkSaves };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTableSortMark(guest, item, 0x48, 0x86, 0x80); });
}

void frontend::standingsSortMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46e660", 0, 12, nullptr, 0, kSortMarkSaves };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTableSortMark(guest, item, 0x46, 0x7c, 0x76); });
}

void frontend::roundsSortMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46f4e0", 0, 12, nullptr, 0, kSortMarkSaves };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTableSortMark(guest, item, 0x46, 0x7c, 0x76); });
}

void frontend::resultsTableKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46e270", 0, 16 + 0xc, nullptr, 0, kKeySaves };
    const reg32 item = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsKey(guest, item, key); });
}

void frontend::standingsTableLayout(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46f0c0", 0, 24, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return layoutStandings(guest, item, kStandingsType, kResultsCount); });
}

void frontend::standingsTableKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46f200", 0, 16 + 0xc, nullptr, 0, kKeySaves };
    const reg32 item = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return standingsKey(guest, item, key, kStandingsType, kResultsCount); });
}

void frontend::roundsTableShown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46fe30", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return roundsOnShow(guest, item); });
}

void frontend::roundsTableLayout(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_46ff00", 0, 24, nullptr, 0 };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return layoutStandings(guest, item, kRoundsType, kRoundsCount); });
}

void frontend::roundsTableKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470040", 0, 16 + 0xc, nullptr, 0, kKeySaves };
    const reg32 item = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return standingsKey(guest, item, key, kRoundsType, kRoundsCount); });
}

void frontend::headToHeadShown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470360", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return sideBySideOnShow(guest, item); });
}

void frontend::headToHeadDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4703a0", 0, 20 + 0x154, nullptr, 0, kResultsSaves };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        drawSideBySide(guest, item);
        guest.leaveEdx(0);
        return reg32(0);
    });
}

/* sub_477c00: the flag at 0x55d000 down. */
void frontend::clearLoadFlag(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477c00", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.setDword(kLoadFlag, 0);
        return reg32(0);
    });
}

#undef MENUC_SHOWN
#undef MENUC_NOTHING
#undef MENUC_DRAW
#undef MENUC_OF_ITEM
#undef MENUC_PLAIN

}
