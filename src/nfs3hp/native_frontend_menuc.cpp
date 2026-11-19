#include "native_frontend_menuc.h"
#include "native_load_saves.h"
#include "native_watcom.h"
#include "native_frontend_menub.h"
#include "game_settings.h"
#include "native_collide.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_connect.h"
#include "native_frontend_draw.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_loop.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menud.h"
#include "native_frontend_menue.h"
#include "native_frontend_misc.h"
#include "native_frontend_screen.h"
#include "native_frontend_wata.h"
#include "native_graphics.h"
#include "port_draw2d.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_shapes.h"
#include "port_sound_effects.h"
#include "port_sound_music.h"
#include "port_sound_racesetup.h"
#include "port_sound_speech.h"
#include "port_text.h"
#include "port_view.h"
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
 *                        sub_473080) and the screens after a race (sub_4732b0).
 *
 * Plan step 5: each function a typed core in frontend::menuc (native_frontend_menuc.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the game's
 * name for the calls by address.  A core calls another of this file's, or of
 * native_frontend_menub.cpp's, as C++ (guest::direct), and the typed cores of the other menu files,
 * the port's and the libraries' too (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using nfs::u32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The menu engine's.
constexpr u32 kItemShownNow = 0x44a660;      // eax the item, edx its widget's type
constexpr u32 kPointerX = 0x55e5e4;
constexpr u32 kPointerY = 0x55e5e8;
constexpr std::uint32_t kClick = 1, kClickVolume = 0x7f;

// Text and drawing.
constexpr u32 kSprintf = 0x4df690;
constexpr std::uint32_t kFont = 0x12;

// The alignments the menus' text takes.
constexpr std::uint32_t kAlignLeft = 3, kAlignRight = 4, kAlignCentre = 5;

// Their colours.
constexpr std::uint32_t kGold = 0xffd59428, kBlue = 0xff0000aa, kBlack = 0xff000000;
constexpr std::uint32_t kRowText = 0xff649dfd, kPlayerText = 0x00ffe440, kHotText = 0x00ffe440;
constexpr std::uint32_t kDimmedText = 0x00004a96, kOutText = 0x00800000;
constexpr std::uint32_t kHotFrame = 0xffffe440;
constexpr std::uint32_t kBestLap = 0xffff0000, kOtherLap = 0xff007fff;

// The results, as the screens after a race read them.
constexpr u32 kResultsCount = 0x677d30;      // the racers
constexpr u32 kRoundsCount = 0x677d34;
constexpr u32 kRacerStatus = 0x2c4;          // 2 and 4: did not finish
constexpr u32 kRacerPlayer = 0x1f0;
constexpr u32 kRacerLapsDone = 0x2c8;
constexpr u32 kRacerTime = 0x238;
constexpr u32 kRacerLaps = 0x240;            // the time of each lap
constexpr u32 kRacerRoundsDone = 0x2cc;
constexpr u32 kRacerRoundsTime = 0x2d4;
constexpr std::uint32_t kStatusOut = 2, kStatusWrecked = 4;

constexpr std::uint32_t kRaceDuel = 3;

// The lines of text.
constexpr std::uint32_t kLineLaps = 0x193, kLineNoTime = 0xeb, kLineNoPlace = 0xed, kLineWrecked = 0xf5;
constexpr std::uint32_t kLineSortBy = 0x199, kLineLapTimes = 5, kLineWinner = 0x194;

// The game's formats.
constexpr u32 kFormatLapNumber = 0x53aeb4;   // "%d)"
constexpr u32 kFormatNumber = 0x53aeb8;      // "%d"
constexpr u32 kFormatOfTotal = 0x53aebc;     // "%d/%d"
constexpr u32 kFormatNumberAndName = 0x53aee4; // "%d (%s)"
constexpr u32 kFormatOfTotalAndName = 0x53aeec; // "%d/%d (%s)"
constexpr u32 kNoValue = 0x53aef8;

// The arrows' art.
constexpr u32 kArtUp = 0x53aec4, kArtDown = 0x53aecc, kArtUpHot = 0x53aed4, kArtDownHot = 0x53aedc;

// The tables slide in and out: 20 pixels a pass, out past the screen's edge.
constexpr int kSlideStep = 0x14;
constexpr int kSlidOut = 0x2a8;
enum SlideState { kSlideNone = 0, kSlideIn = 1, kSlideOut = 2, kSlideGone = 3 };

constexpr u32 kItemX = 6;
constexpr u32 kItemY = 8;
constexpr u32 kItemBoundTop = 0x1a, kItemBoundLeft = 0x1c, kItemBoundWidth = 0x1e, kItemBoundHeight = 0x20;

constexpr int kArrowSize = 0xc;

constexpr std::uint32_t kResultsType = 0x17, kStandingsType = 0x19, kRoundsType = 0x1a, kSideBySideType = 0x1b;

/* Half, towards zero. */
std::int32_t half(std::int32_t value)
{
    return (value - (value >> 31)) >> 1;
}

std::int32_t s16(const Guest& guest, u32 address)
{
    return std::int16_t(guest.word(address));
}

/* The low word of a register set, the high one kept: what a `mov ax, ...`
 * leaves, for what a function leaves in a register it does not save. */
u32 withLow(u32 value, std::uint32_t low)
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
    u32 rowHeight;
    u32 rows;         // how many it shows at once
    u32 boxes;        // a box for each column's title: top, left, width, height
    u32 up, down;     // the arrows' boxes
    u32 shapes;       // up, down, up under the pointer, down under the pointer, the two kept
    u32 scroll;       // the first racer shown
    u32 sort;         // the column it is sorted by
    u32 state;        // SlideState
    u32 x;            // where it stands while it slides
};

constexpr u32 kColumns = 0x3c;               // int16 each: the columns' widths
constexpr Fields kResultsFields{ 0x44, 0x46, 0x48, 0x68, 0x70, 0x78, 0x84, 0x86, 0x88, 0x8a };
constexpr Fields kStandingsFields{ 0x42, 0x44, 0x46, 0x5e, 0x66, 0x6e, 0x7a, 0x7c, 0x7e, 0x80 };

/* A table on the screen, for its functions. */
class Table
{
public:
    Table(App* app, ThreadState& ts, Guest& guest, u32 item, const Fields& fields, std::uint32_t type)
        : app(app), ts(ts), g(guest), item(item), f(fields), type(type)
    {
    }

    // By address, as the shown-nows below: the type in edx, which the core does not take, is part of
    // the call this group's check sees.
    bool shown() const { return fe::direct<kItemShownNow, frontend::loop::shownNow>(g.app(), g.ts(), item) != 0; }

    std::int32_t x() const { return s16(g, item + f.x); }
    std::int32_t y() const { return s16(g, item + kItemY); }
    std::int32_t width(int column) const { return s16(g, item + kColumns + 2 * column); }
    std::int32_t rowHeight() const { return s16(g, item + f.rowHeight); }
    std::int32_t rows() const { return s16(g, item + f.rows); }
    std::int32_t scroll() const { return s16(g, item + f.scroll); }
    std::uint16_t state() const { return g.word(item + f.state); }
    u32 box(int column) const { return item + f.boxes + 8 * column; }

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
        const std::uint32_t colours[] = { c1, c2, c3, c4 };
        fe::direct<0x4d8860, port::draw2d::shadedBox>(app, ts, ts.sp - 20, bx, by, w, h, colours);
    }

    void text(u32 string, std::int32_t tx, std::int32_t ty, std::uint32_t align, std::uint32_t colour) const
    {
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, string, u32(tx), u32(ty), kFont,
                                                        align, colour);
    }

    void fitted(u32 string, std::int32_t tx, std::int32_t ty, std::uint32_t colour, std::int32_t room) const
    {
        guest::direct<0x452120, frontend::menub::textDrawFitted>(app, ts, string, u32(tx), u32(ty), kFont,
                                                                 kAlignLeft, colour, u32(room));
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
    void title(int column, u32 string, std::uint32_t align, bool canPoint, bool dimmed, std::int32_t inset) const
    {
        const u32 at = box(column);
        const std::int32_t left = s16(g, at + 2);
        const std::int32_t titleX = align == kAlignCentre ? left + half(s16(g, at + 4)) : left + 10;
        const std::int32_t titleY = half(s16(g, at + 6)) + s16(g, at) - 5;
        const bool hot = canPoint && fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, at) != 0;
        if (hot)
            fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8, u32(left + inset), u32(s16(g, at)),
                                                        u32(s16(g, at + 4) - inset), u32(s16(g, at + 6) - inset),
                                                        kHotFrame);
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

    bool isThisPlayer(u32 record) const
    {
        const u32 player = g.dword(record + kRacerPlayer);
        return player == gameSettings(g).localPlayer || (gameSettings(g).playMode == 1 && player == 1);
    }

    /* The six shapes named, in their places: the last, what the calls leave in eax. */
    u32 loadArrows() const
    {
        static constexpr u32 kShapes[] = { kArtUp, kArtDown, kArtUpHot, kArtDownHot, kArtDown, kArtUp };
        u32 shape = 0;
        for (int i = 0; i < 6; ++i)
        {
            shape = u32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4, kShapes[i]));
            g.setWord(item + f.shapes + 2 * i, std::uint16_t(shape));
        }
        return shape;
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
    u32 scrollToPlayer(std::int32_t count, u32 lastEax) const
    {
        for (std::int32_t i = 0; i < count; ++i)
        {
            lastEax = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(i), 0, 0);
            if (lastEax == 0)
                return 0;
            if (isThisPlayer(lastEax))
            {
                if (i >= rows())
                {
                    const u32 first = withLow(u32(i), std::uint32_t(i) - g.word(item + f.rows)) + 1;
                    g.setWord(item + f.scroll, std::uint16_t(first));
                    return first;
                }
                lastEax = u32(rows());
            }
        }
        return lastEax;
    }

    App* const app;
    ThreadState& ts;
    Guest& g;
    const u32 item;
    const Fields& f;
    const std::uint32_t type;

private:
    void arrow(u32 boxField, int shape, std::int32_t nudge) const
    {
        const u32 at = item + boxField;
        const bool hot = fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, at) != 0;
        const u32 art = item + f.shapes + (hot ? 4 + shape : shape);
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, u32(s16(g, art)),
                                                        u32(s16(g, at + 2)), u32(s16(g, at) + nudge));
    }
};

/* The pointer put on a box, three quarters along it, half way down. */
void setPointer(App* app, ThreadState& ts, Guest& guest, std::int32_t x, std::int32_t y)
{
    guest.setDword(kPointerX, u32(x));
    guest.setDword(kPointerY, u32(y));
    fe::direct<0x49bfc0, frontend::screen::setPointer>(app, ts, u32(x), u32(y));
}

void pointAtBox(App* app, ThreadState& ts, Guest& guest, u32 box)
{
    setPointer(app, ts, guest, s16(guest, box + 2) + (3 * s16(guest, box + 4)) / 4,
               s16(guest, box) + half(s16(guest, box + 6)));
}

/* ---------------------------------------------------------------------------
 * The little mark over a table's sorted column
 * ------------------------------------------------------------------------- */

/* sub_46cce0: "sort by" and an arrow over a column's title, over two lines as
 * wide as the words.  On the stack: the column's box, its top and left, then
 * its width and height, then the arrow's shape. */
u32 drawSortMark(App* app, ThreadState& ts, Guest& guest, u32 topLeft, u32 widthHeight, u32 shape)
{
    const std::int32_t top = std::int16_t(topLeft);
    const std::int32_t left = std::int16_t(topLeft >> 16);
    const std::int32_t halfWidth = half(std::int16_t(widthHeight));
    const u32 measured = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineSortBy,
                                                                             0));
    const std::int32_t textWidth = std::int32_t(u32(*fe::direct<0x452020, port::text::menuTextWidth>(
        app, ts, ts.sp - 4, measured, kFont)));
    const std::int32_t centre = left + halfWidth;
    const u32 line = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineSortBy, 0));
    fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, line, u32(centre), u32(top - 0x1e),
                                                    kFont, kAlignCentre, kRowText);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, shape, u32(left - 6 + halfWidth),
                                                    u32(top - 0xe));
    const std::int32_t halfText = half(textWidth);
    const u32 from = u32(centre - halfText), to = u32(centre + halfText);
    fe::direct<0x4d8ed0, port::draw2d::drawLineOf>(app, ts, ts.sp - 12, from, u32(top - 0xd), to, u32(top - 0xd),
                                                   0xff5f78a9, 0xff5f78a9);
    return u32(*fe::direct<0x4d8ed0, port::draw2d::drawLineOf>(app, ts, ts.sp - 12, from, u32(top - 0xc), to,
                                                                 u32(top - 0xc), 0xff738cbd, 0xff738cbd));
}

/* sub_46cdc0, sub_46e660, sub_46f4e0: that mark, over the table's own column,
 * which they leave in edx. */
u32 drawTableSortMark(App* app, ThreadState& ts, Guest& guest, u32 item, u32 boxes, u32 sort, u32 shape)
{
    const u32 column = u32(s16(guest, item + sort));
    const u32 at = item + boxes + 8 * column;
    const u32 result = guest::direct<0x46cce0, frontend::menuc::drawColumnSortMark>(
        app, ts, guest.dword(at), guest.dword(at + 4), u32(s16(guest, item + shape)));
    return result;
}

/* ---------------------------------------------------------------------------
 * The results, sorted by a column
 * ------------------------------------------------------------------------- */

/* sub_46cdf0: which column of a row the pointer is in, for its popups: 1 the
 * lap times', 2 the metric's, 0 neither.  A wrecked racer has no laps. */
u32 resultsPopupUnderPointer(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
    const u32 box = guest.frame();
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
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, box) != 0)
        {
            const u32 record =
                fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(row + t.scroll()), 0, 0);
            if (record != 0 && guest.dword(record + kRacerStatus) != kStatusWrecked)
                return 1;
            break;
        }
        nextRow();
    }
    if (gameSettings(guest).raceType != kRaceDuel)
    {
        makeBox(firstLeft + guest.word(item + 0x40), guest.word(item + 0x42));
        for (std::int32_t row = 0; row < t.rows(); ++row)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, box) != 0)
            {
                if (fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(row + t.scroll()), 0, 0) != 0)
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
u32 drawLapTimesPopup(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
    const u32 buffer = guest.frame();               // char[0x100]
    const u32 box = guest.frame() + 0x100;
    guest.setWord(box + 2, std::uint16_t(std::uint32_t(guest.word(item + 0x3c)) + guest.word(item + 0x8a) + guest.word(item + 0x3e)));
    guest.setWord(box, std::uint16_t(guest.word(item + kItemY) + guest.word(item + 0x44)));
    guest.setWord(box + 4, std::uint16_t(guest.word(item + 0x40) + 1));
    guest.setWord(box + 6, std::uint16_t(guest.word(item + 0x44) + 1));
    for (std::int32_t row = 0; row < t.rows(); ++row)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, box) == 0)
        {
            guest.setWord(box, std::uint16_t(guest.word(box) + guest.word(item + 0x44)));
            continue;
        }
        const u32 record =
            fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(row + t.scroll()), 0, 0);
        if (record == 0)
            return 0;
        if (guest.dword(record + kRacerStatus) == kStatusWrecked)
            return record;
        const std::int32_t laps = std::int32_t(gameSettings(guest).laps);
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
        static constexpr std::uint32_t kPopupColours[] = { 0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222 };
        fe::direct<0x4d8ac0, port::draw2d::drawFieldBoxOf>(app, ts, ts.sp - 20, left + 2, top + 2, width - 3,
                                                           15 * laps + 0x20, kPopupColours);
        const u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineLapTimes,
                                                                              0));
        t.text(title, left + half(width), top + 8, kAlignCentre, kHotText);
        std::int32_t lineY = top + 0x18;
        u32 last = u32(top);
        for (std::int32_t lap = 0; lap < laps; ++lap)
        {
            const std::uint32_t colour = lap == best ? kBestLap : kOtherLap;
            fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, guest.dword(record + kRacerLaps + 4 * lap),
                                                            buffer);
            t.text(buffer, left + width - 8, lineY, kAlignRight, colour);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatLapNumber, guest::Words{u32(lap + 1)});
            last = u32(*fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, buffer,
                                                                          u32(left + 8), u32(lineY), kFont,
                                                                          kAlignLeft, colour));
            lineY += 0xf;
        }
        return last;
    }
    return u32(t.rows());
}

/* The last column: the laps done in a duel, the result otherwise. */
void lastColumnText(App* app, ThreadState& ts, const Table& t, u32 record, u32 buffer)
{
    Guest& guest = t.g;
    if (gameSettings(guest).raceType == kRaceDuel)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatNumber, guest::Words{guest.dword(record + kRacerLapsDone)});
    else
        guest::direct<0x455720, frontend::menub::resultsTopSpeedText>(app, ts, record, buffer);
}

/* One racer's row in the results' plain form. */
void drawResultsRowPlain(App* app, ThreadState& ts, const Table& t, u32 record, u32 buffer, std::int32_t rowY, bool dimmed)
{
    const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    const u32 place = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoPlace,
                                                                          0));
    t.text(place, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest::direct<0x455570, frontend::menub::resultsName>(app, ts, record, buffer);
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest::direct<0x455600, frontend::menub::resultsCarName>(app, ts, record, buffer);
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    const u32 out = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoTime, 0));
    t.text(out, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
    x += t.width(2);
    lastColumnText(app, ts, t, record, buffer);
    t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
}

/* One racer's row in the results; `popup`, a column of it dimmed under its popup. */
void drawResultsRow(App* app, ThreadState& ts, const Table& t, u32 record, u32 buffer, std::int32_t rowY, bool dimmed,
                    u32 popup)
{
    Guest& guest = t.g;
    const std::uint32_t status = guest.dword(record + kRacerStatus);
    const bool wrecked = status == kStatusWrecked;
    const std::uint32_t colour = dimmed ? kDimmedText : (status == kStatusOut || wrecked) ? kOutText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    if (wrecked)
        guest.copyString(buffer, u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoPlace,
                                                                                   0)));
    else
        guest::direct<0x4555c0, frontend::menub::resultsPlaceText>(app, ts, record, buffer);
    t.text(buffer, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest::direct<0x455570, frontend::menub::resultsName>(app, ts, record, buffer);
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest::direct<0x455600, frontend::menub::resultsCarName>(app, ts, record, buffer);
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    if (wrecked)
    {
        const u32 line = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineWrecked,
                                                                             0));
        t.text(line, x + half(t.width(2)), rowY, kAlignCentre, colour);
        x += t.width(2);
        lastColumnText(app, ts, t, record, buffer);
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
        return;
    }
    fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, guest.dword(record + kRacerTime), buffer);
    t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, popup == 1 ? kDimmedText : colour);
    x += t.width(2);
    if (gameSettings(guest).raceType == kRaceDuel)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatOfTotal, guest::Words{guest.dword(record + kRacerLapsDone), gameSettings(guest).pursuitOutCount});
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
    }
    else
    {
        guest::direct<0x455720, frontend::menub::resultsTopSpeedText>(app, ts, record, buffer);
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, popup == 2 ? kDimmedText : colour);
    }
}

/* sub_46d3c0: the results table: sliding, sorted as chosen, its box and lines,
 * the titles, a row for each racer shown, the popups, the arrows. */
void drawResults(App* app, ThreadState& ts, Guest& guest, u32 item, bool pointing)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
    const u32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46e0d0, frontend::menuc::resultsTableLayout>(app, ts, item);
    const bool dimmed = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    switch (guest.word(item + 0x86))
    {
    case 0:
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 8, 0);
        break;
    case 3:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0,
                                                          gameSettings(guest).raceType == kRaceDuel ? 6u : 2u, 1);
        break;
    default:
        break;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const std::int32_t shownRows = smaller(t.rows(), count);
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2) + t.width(3);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const bool canPoint = pointing && !plain;
    const u32 popup = canPoint ? guest::direct<0x46cdf0, frontend::menuc::resultsPopupColumn>(app, ts, item) : 0;
    const std::int32_t x = t.x(), y = t.y();
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    if (t.rows() < count)
        fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8, u32(s16(guest, item + 0x6a) - 3),
                                                    u32(s16(guest, item + 0x68) - 1), 0x12, u32(height + 4), kGold);
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height + 2, 3);
    if (t.state() == kSlideNone && count > 1 && !plain)
        guest::direct<0x46cdc0, frontend::menuc::resultsSortMark>(app, ts, item);

    static constexpr std::uint32_t kTitles[] = { 227, 200, 40, 41 };
    static constexpr std::uint32_t kAligns[] = { kAlignCentre, kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 4; ++column)
    {
        u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kTitles[column],
                                                                        0));
        if (gameSettings(guest).raceType == kRaceDuel && column == 3)
            title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineLaps, 0));
        t.title(column, title, kAligns[column], canPoint, dimmed, 1);
    }

    std::int32_t rowY = y + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const u32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(place), 0, 0);
        if (record == 0)
            break;
        if (plain)
            drawResultsRowPlain(app, ts, t, record, buffer, rowY, dimmed);
        else
            drawResultsRow(app, ts, t, record, buffer, rowY, dimmed, popup);
        rowY += t.rowHeight();
    }
    if (canPoint)
    {
        guest::direct<0x46d180, frontend::menuc::resultsLapTimesPopup>(app, ts, item);
        fe::direct<0x46cf30, port::view::metricPopupOf>(app, ts, ts.sp - 4, item);
    }
    t.arrows(count, 1);
}

/* sub_46df90: the results as their screen opens: the arrows' art named, sorted
 * by place, the player's row in view. */
u32 resultsOnShow(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    t.placeIfNew();
    guest.setWord(item + 0x86, 0);
    guest.setWord(item + 0x84, 0);
    const u32 sorted = fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 0);
    return t.scrollToPlayer(count, sorted);
}

/* sub_46ef90: the same for the standings, sorted by their third column. */
u32 standingsOnShow(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kStandingsFields, kStandingsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    guest.setWord(item + 0x7c, 2);
    guest.setWord(item + 0x7a, 0);
    t.placeIfNew();
    const u32 sorted = fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 4, 1);
    return t.scrollToPlayer(count, sorted);
}

/* sub_46fe30: the same for the second list, which its draw sorts. */
u32 roundsOnShow(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kStandingsFields, kRoundsType);
    if (!t.shown())
        return 0;
    const u32 lastShape = t.loadArrows();
    guest.setWord(item + 0x7c, 2);
    const bool placed = t.placeIfNew();
    guest.setWord(item + 0x7a, 0);
    return placed ? withLow(lastShape, guest.word(item + kItemX)) : lastShape;
}

/* sub_46e0d0: the results' boxes as the table stands now: a title's for each
 * column, the whole table's, the arrows'.  What is left in eax is the last sum
 * they were made with. */
u32 layoutResults(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
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
        const u32 box = t.box(column);
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
    const u32 first = withLow(u32(tableWidth + 0x20), x) + u32(tableWidth) + 0x14;
    const u32 second = withLow(first, x) + u32(tableWidth) + 0x14;
    return withLow(second, top) + u32(height) - kArrowSize;
}

/* sub_46f0c0, sub_46ff00: the same for the three columns of the others. */
u32 layoutStandings(App* app, ThreadState& ts, Guest& guest, u32 item, std::uint32_t type, u32 countAt)
{
    const Table t(app, ts, guest, item, kStandingsFields, type);
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
        const u32 box = t.box(column);
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
    const u32 first = withLow(u32(tableWidth + 0x20), x) + u32(tableWidth) + 0x14;
    const u32 second = u32(tableWidth) + withLow(first, x) + 0x14;
    return withLow(second, top) + u32(height) - kArrowSize;
}

constexpr std::uint16_t kEnter = 0x0d, kKeyRight = 0x4d00, kKeyLeft = 0x4b00, kKeyPageUp = 0x4900, kKeyPageDown = 0x5100;

void click(nfs::App* app, nfs::ThreadState& ts)
{
    fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kClick, kClickVolume);
}

/* sub_46e270: the results' keys: Enter on a title sorts by it, left and right
 * go along the titles, the arrows scroll.  2 when a key was used; it leaves
 * that in edx too, and in ebx where it has got to. */
u32 resultsKey(App* app, ThreadState& ts, Guest& guest, u32 item, std::uint16_t key)
{
    const Table t(app, ts, guest, item, kResultsFields, kResultsType);
    u32 ebx = 0xffffffff;
    if (!t.shown())
    {
        return 0;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const bool sortable = count > 1 && !plain;
    for (int column = 0; column < 4; ++column)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, t.box(column)) != 0)
        {
            ebx = u32(column);
            break;
        }
    }
    bool used = false;
    if (key == kEnter && sortable && ebx <= 3)
    {
        guest.setWord(item + 0x86, std::uint16_t(ebx));
        used = true;
        click(app, ts);
    }
    if (sortable && (key == kKeyRight || key == kKeyLeft))
    {
        std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
        column = column < 0 ? 0 : column > 3 ? 3 : column;
        ebx = u32(column);
        pointAtBox(app, ts, guest, t.box(column));
        used = true;
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x84) != 0)
        {
            click(app, ts);
            ebx = withLow(ebx, guest.word(item + 0x84)) - 1;
            guest.setWord(item + 0x84, std::uint16_t(ebx));
            used = true;
            pointAtBox(app, ts, guest, item + 0x68);
        }
        if (key == kKeyPageDown)
        {
            ebx = u32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(app, ts);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
                pointAtBox(app, ts, guest, item + 0x70);
            }
        }
        if (key == kEnter)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + 0x68) != 0
                && guest.word(item + 0x84) != 0)
            {
                click(app, ts);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) - 1));
                used = true;
            }
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + 0x70) != 0
                && t.rows() + t.scroll() < count)
            {
                click(app, ts);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
            }
        }
    }
    const u32 result = used ? 2 : 0;
    return result;
}

/* sub_46f200, sub_470040: the keys of the others.  sub_470040 takes left and
 * right both as the next title, and points with that one's width and height
 * at the top and left of the one after it: the game's own slip. */
u32 standingsKey(App* app, ThreadState& ts, Guest& guest, u32 item, std::uint16_t key, std::uint32_t type, u32 countAt)
{
    const bool rounds = type == kRoundsType;
    const Table t(app, ts, guest, item, kStandingsFields, type);
    u32 ebx = 0xffffffff;
    if (!t.shown())
    {
        return 0;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) != 0;
    const std::int32_t count = std::int32_t(guest.dword(countAt));
    for (int column = 0; column < 3; ++column)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, t.box(column)) != 0)
        {
            ebx = u32(column);
            break;
        }
    }
    bool used = false;
    if (key == kEnter && !plain && std::int32_t(ebx) >= 0)
    {
        guest.setWord(item + 0x7c, std::uint16_t(ebx));
        used = true;
        click(app, ts);
    }
    if (!plain && (key == kKeyRight || key == kKeyLeft))
    {
        used = true;
        if (rounds)
        {
            ebx = u32((std::int32_t(ebx) + 1) % 3);
            const u32 box = t.box(int(ebx));
            setPointer(app, ts, guest, s16(guest, box + 10) + (3 * s16(guest, box + 4)) / 4,
                       s16(guest, box + 8) + half(s16(guest, box + 6)));
        }
        else
        {
            std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
            column = column < 0 ? 0 : column > 2 ? 2 : column;
            ebx = u32(column) << 3;
            pointAtBox(app, ts, guest, t.box(column));
        }
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x7a) != 0)
        {
            click(app, ts);
            ebx = withLow(ebx, guest.word(item + 0x7a)) - 1;
            guest.setWord(item + 0x7a, std::uint16_t(ebx));
            used = true;
            pointAtBox(app, ts, guest, item + 0x5e);
        }
        if (key == kKeyPageDown)
        {
            ebx = u32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(app, ts);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) + 1));
                used = true;
                pointAtBox(app, ts, guest, item + 0x66);
            }
        }
        if (key == kEnter)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + 0x5e) != 0
                && guest.word(item + 0x7a) != 0)
            {
                click(app, ts);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) - 1));
                used = true;
            }
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + 0x66) != 0
                && t.rows() + t.scroll() < count)
            {
                click(app, ts);
                const std::uint16_t scroll = guest.word(item + 0x7a);
                if (!rounds)
                    ebx = withLow(ebx, scroll) + 1;
                guest.setWord(item + 0x7a, std::uint16_t(scroll + 1));
                used = true;
            }
        }
    }
    const u32 result = used ? 2 : 0;
    return result;
}

/* The standings' box: its background and lines, as sub_46e690 and sub_46f510
 * both draw it. */
void drawStandingsBox(const Table& t, std::int32_t shownRows, std::int32_t tableWidth, std::int32_t height, std::int32_t count,
                      int columnLines)
{
    Guest& guest = t.g;
    App* const app = t.app;
    ThreadState& ts = t.ts;
    const std::int32_t x = t.x(), y = t.y();
    fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8, u32(x), u32(y), u32(tableWidth + 2),
                                                u32(height + 2), kBlack);
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    if (t.rows() < count)
        fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8, u32(s16(guest, t.item + 0x60) - 3),
                                                    u32(s16(guest, t.item + 0x5e) - 3), 0x12, u32(height + 6),
                                                    kGold);
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height, columnLines);
}

/* sub_46e690: a series' standings: each racer's name and car, his rank and
 * place, and his other rank. */
void drawStandings(App* app, ThreadState& ts, Guest& guest, u32 item, bool pointing)
{
    const Table t(app, ts, guest, item, kStandingsFields, kStandingsType);
    const u32 buffer = guest.frame();               // char[0x100]
    const u32 placeName = guest.frame() + 0x100;    // char[0x100]
    const u32 firstRank = guest.frame() + 0x210;
    const u32 secondRank = guest.frame() + 0x214;
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46f0c0, frontend::menuc::standingsTableLayout>(app, ts, item);
    const bool dimmed = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 8, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 3, 1);
        break;
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 4, 1);
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
        guest::direct<0x46e660, frontend::menuc::standingsSortMark>(app, ts, item);

    static constexpr std::uint32_t kTitles[] = { 200, 239, 240 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kTitles[column],
                                                                              0));
        t.title(column, title, kAligns[column], !plain && pointing, dimmed, 0);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const u32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(place), 0, 0);
        if (record == 0)
            break;
        const bool out = guest.dword(record + kRacerStatus) == kStatusOut;
        fe::direct<0x455530, frontend::misc::resultsParticipantRanks>(app, ts, record, firstRank, secondRank);
        const std::uint32_t colour = dimmed ? kDimmedText : (!plain && out) ? kOutText : kRowText;
        const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
        std::int32_t x = t.x();
        guest::direct<0x455570, frontend::menub::resultsName>(app, ts, record, buffer);
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
        guest::direct<0x455600, frontend::menub::resultsCarName>(app, ts, record, buffer);
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const u32 line = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoTime,
                                                                                 0));
            t.text(line, x + half(t.width(1)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            guest::direct<0x4555c0, frontend::menub::resultsPlaceText>(app, ts, record, placeName);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatNumberAndName, guest::Words{guest.dword(firstRank), placeName});
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
        }
        x += t.width(1);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatNumber, guest::Words{guest.dword(secondRank)});
        t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
        rowY += t.rowHeight();
    }
    t.arrows(count, 0);
}

/* sub_46f510: the second list of racers: each one's name and car, a count of
 * his against the count at 0x6fd4b8, his time once he has reached it. */
void drawRounds(App* app, ThreadState& ts, Guest& guest, u32 item, bool pointing)
{
    const Table t(app, ts, guest, item, kStandingsFields, kRoundsType);
    const u32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46ff00, frontend::menuc::roundsTableLayout>(app, ts, item);
    const bool dimmed = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 1, 8, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 1, 5, 1);
        break;
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 1, 7, 0);
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
        guest::direct<0x46f4e0, frontend::menuc::roundsSortMark>(app, ts, item);

    static constexpr std::uint32_t kTitles[] = { 200, 247, 405 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kTitles[column],
                                                                              0));
        t.title(column, title, kAligns[column], pointing && !plain, dimmed, 1);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const u32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 1, u32(place), 0, 0);
        if (record == 0)
            break;
        const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
        std::int32_t x = t.x();
        guest::direct<0x455570, frontend::menub::resultsName>(app, ts, record, buffer);
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, colour);
        guest::direct<0x455600, frontend::menub::resultsCarName>(app, ts, record, buffer);
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const u32 none = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoPlace,
                                                                                 0));
            t.text(none, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            const u32 line = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kLineNoTime,
                                                                                 0));
            t.text(line, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            const std::int32_t won = std::int32_t(guest.dword(record + kRacerRoundsDone));
            const std::int32_t toWin = std::int32_t(gameSettings(guest).pursuitWinCount);
            if (won >= toWin)
            {
                const u32 winner = u32(*fe::direct<0x4d1850, port::text::lineText>(
                    app, ts, ts.sp - 4, kLineWinner, 0));
                fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatOfTotalAndName, guest::Words{u32(won), u32(toWin), winner});
            }
            else
            {
                fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kFormatOfTotal, guest::Words{u32(won), u32(toWin)});
            }
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            if (won >= toWin)
            {
                fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, guest.dword(record + kRacerRoundsTime),
                                                                buffer);
                t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
            }
            else
            {
                const u32 none = u32(*fe::direct<0x4d1850, port::text::lineText>(
                    app, ts, ts.sp - 4, kLineNoPlace, 0));
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

constexpr u32 kSideState = 0x6c, kSideX = 0x6e;
constexpr u32 kSideRowHeight = 0x42;
constexpr u32 kSideTitles = 0x44;            // two texts for each of five rows
constexpr int kSideRows = 5;
constexpr u32 kRacerBests[] = { 0x268, 0x270, 0x278, 0x280 };

/* sub_470360: where the table stands, the first time. */
u32 sideBySideOnShow(Guest& guest, u32 item)
{
    const u32 shown = fe::direct<kItemShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item);
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
void drawSideBySide(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const u32 buffer = guest.frame();               // char[0x100]
    if (fe::direct<kItemShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
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
    const bool dimmed = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    const std::int32_t lastPlayer = gameSettings(guest).playMode == 1 ? 1 : 0;
    const std::uint32_t kind = gameSettings(guest).hud[0].speedometerForm;
    const u32 second = kind == 3 || kind == 4 ? 1 : 0;
    const std::int32_t titles = s16(guest, item + kColumns);
    const std::int32_t rowHeight = s16(guest, item + kSideRowHeight);
    const std::int32_t tableWidth = titles + s16(guest, item + kColumns + 2) + (lastPlayer ? s16(guest, item + kColumns + 4) : 0);
    const std::int32_t height = 6 * rowHeight;
    const std::int32_t x = s16(guest, item + kSideX), y = s16(guest, item + kItemY);
    auto shaded = [&](std::int32_t bx, std::int32_t by, std::int32_t w, std::int32_t h, std::uint32_t c1, std::uint32_t c2,
                      std::uint32_t c3, std::uint32_t c4) {
        const std::uint32_t colours[] = { c1, c2, c3, c4 };
        fe::direct<0x4d8860, port::draw2d::shadedBox>(app, ts, ts.sp - 20, bx, by, w, h, colours);
    };
    fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8, u32(x), u32(y), u32(tableWidth + 2),
                                                u32(height + 2), kBlack);
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
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12,
                                                        guest.dword(item + kSideTitles + 8 * i + 4 * second),
                                                        u32(x + 10), u32(titleY - 10), kFont, kAlignLeft,
                                                        titleColour);
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
        u32 record = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(
            app, ts, 0, gameSettings(guest).localPlayer + player);
        if (record == 0)
            record = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(
                app, ts, 1, gameSettings(guest).localPlayer + player);
        guest::direct<0x455570, frontend::menub::resultsName>(app, ts, record, buffer);
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, buffer, u32(centre), u32(rowY),
                                                        kFont, kAlignCentre, nameColour);
        rowY += rowHeight;
        guest::direct<0x455660, frontend::menub::resultsCarLine0>(app, ts, record, buffer);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, ts, buffer, u32(centre), u32(rowY - 10), kFont, kAlignCentre, colour, u32(room));
        guest::direct<0x4556c0, frontend::menub::resultsCarLine1>(app, ts, record, buffer);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, ts, buffer, u32(centre), u32(rowY + 5), kFont, kAlignCentre, colour, u32(room));
        rowY += rowHeight;
        for (const u32 field : kRacerBests)
        {
            const u32 value = guest.dword(record + field + 4 * second);
            if (value != 0)
            {
                fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, value, buffer);
                fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, buffer, u32(centre),
                                                                u32(rowY), kFont, kAlignCentre, colour);
            }
            else
            {
                fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, kNoValue, u32(centre),
                                                                u32(rowY), kFont, kAlignCentre, kDimmedText);
            }
            rowY += rowHeight;
        }
        columnX += room;
    }
}

/* ---------------------------------------------------------------------------
 * The menus' art
 * ------------------------------------------------------------------------- */

constexpr u32 kChromeTexture = 0x55a4d0;
constexpr u32 kChromeBuffer = 0x55a4d4;
constexpr u32 kChromeBufferSize = 0x55a4d8;
constexpr u32 kGlowTexture = 0x678a54;
constexpr u32 kGlowTransform = 0x678a58;
constexpr u32 kGlowMode = 0x678a78;
constexpr u32 kDisplayFlags = 0x7a3a58;
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr u32 kArtChrome = 0x53abb8, kArtGlow = 0x53abc0;
constexpr std::uint32_t kOne = 0x3f800000;

/* sub_465d60: no chrome, no glow. */
void clearMenuArt(Guest& guest)
{
    guest.setDword(kGlowTexture, 0);
    guest.setDword(kChromeBuffer, 0);
    guest.setDword(kChromeTexture, 0);
}

/* A shape if it is square, of the size it should be. */
bool isSquareOf(const Guest& guest, u32 shape, std::uint16_t size)
{
    return guest.word(shape + 4) == size && guest.word(shape + 6) == size;
}

/* sub_465d80: the chrome and the glow, from the screen's art: textures of 256
 * and 32 pixels, or none if the art has none of that size.  The glow's
 * corners are the texture's. */
u32 loadMenuArt(App* app, ThreadState& ts, Guest& guest, u32 art)
{
    clearMenuArt(guest);
    if (art == 0)
        return 0;
    const u32 glowTexture = guest.frame();
    guest.setDword(kChromeBufferSize, 4);
    guest.setDword(kChromeBuffer, *fe::direct<0x4d5f40, graphics::artTexture>(app, ts, 0x100, kChromeBufferSize, 0));
    const u32 chrome = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, kArtChrome);
    if (chrome != 0 && isSquareOf(guest, chrome, 0x100))
    {
        const std::uint32_t format = (guest.byte(kDisplayFlags) & kNoTranslucency) ? 4 : 0;
        fe::direct<0x4d6070, port::shapes::textureOf>(app, ts, ts.sp - 4, kChromeTexture, chrome, 0x100, format);
    }
    else
    {
        guest.setDword(kChromeTexture, 0);
    }
    const u32 glow = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, kArtGlow);
    if (glow != 0 && isSquareOf(guest, glow, 0x20))
    {
        fe::direct<0x4d6070, port::shapes::textureOf>(app, ts, ts.sp - 4, glowTexture, glow, 0x20, 0);
        const u32 texture = guest.dword(glowTexture);
        guest.setDword(kGlowTexture, texture);
        static constexpr std::uint32_t kCorners[] = { 0, 0, kOne, 0, kOne, kOne, 0, kOne };
        for (std::size_t i = 0; i < std::size(kCorners); ++i)
            guest.setDword(kGlowTransform + 4 * u32(i), kCorners[i]);
        guest.setByte(kGlowMode, 0xc);
        return texture;
    }
    guest.setDword(kGlowTexture, 0);
    return glow;
}

/* ---------------------------------------------------------------------------
 * The front end's ends
 * ------------------------------------------------------------------------- */

constexpr u32 kLanguageIndex = 0x7a3a50;
constexpr u32 kArtFolder = 0x7a2db4;
constexpr u32 kLanguageCodes = 0x4721b8;     // char[7], one a language
constexpr u32 kFormatTitle = 0x53b030;       // "%stitle_%c.qfs"
constexpr u32 kBack = 0x53b040;
constexpr u32 kReadFile = 0x4956f0;
constexpr std::uint32_t kLoadFlags = 0x10;
constexpr u32 kShapeTextures = 0x4d6390;
constexpr u32 kTitleTexture = 0x8b7638;
constexpr u32 kBeginDraw = 0x4d9120, kEndDraw = 0x4d9140;
constexpr std::int32_t kTitleTicks = 0x180;
constexpr u32 kInputMode = 0x56438c;
constexpr u32 kInputFlags = 0x564390;
constexpr u32 kSavedInputMode = 0x679008;
constexpr u32 kSavedInputFlags = 0x679004;
constexpr u32 kAfterRace = 0x8b7630;
constexpr u32 kLoadFlag = 0x55d000;

/* sub_472f20: the title's picture, in the language's. */
u32 drawTitle(App* app, ThreadState& ts, Guest& guest)
{
    const u32 name = guest.frame();                 // char[0x12c]
    const u32 codes = guest.frame() + 0x12c;
    for (u32 i = 0; i < 7; ++i)
        guest.setByte(codes + i, guest.byte(kLanguageCodes + i));
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, ts, ts.sp - 4,
                                                     port::Args{});
    const std::uint32_t code = guest.byte(codes + guest.dword(kLanguageIndex));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kFormatTitle, guest::Words{kArtFolder, code});
        const u32 file = fe::direct<kReadFile, frontend::draw::loadQfsFlags>(app, ts, name, kLoadFlags);
    if (file == 0)
        return 0;
    const u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, file, kBack);
        guest.setDword(kTitleTexture,
                   u32(*fe::direct<kShapeTextures, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, 0, 0)));
    fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, ts);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, guest.dword(kTitleTexture), 0, 0);
    fe::direct<0x4d9140, graphics::flipAndWait>(app, ts);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

/* sub_472fc0: the title on the screen until a key, or until its time is up. */
u32 waitOnTitle(App* app, ThreadState& ts, Guest& guest)
{
    if (gameSettings(guest).installIsRemote == 0)
        guest::direct<0x472f20, frontend::menuc::titleDraw>(app, ts);
    fe::direct<0x4f0650, frontend::wata::timerSinceLast>(app, ts);
    fe::direct<0x451940, frontend::connect::flushKeys>(app, ts, 0);
    std::int32_t elapsed = 0;
    for (;;)
    {
        const u32 key = u32(*fe::direct<0x451960, port::text::menuKeyOf>(app, ts, ts.sp - 4, 1));
        if (key != 0 || elapsed >= kTitleTicks)
            return key;
        fe::direct<0x4c9860, port::descriptionServed>(app, ts, ts.sp - 4);
        const u32 ticks = fe::direct<0x4f0650, frontend::wata::timerSinceLast>(app, ts);
        elapsed += std::int32_t(ticks);
        if (elapsed > kTitleTicks)
            return ticks;
    }
}

/* sub_473020: into the menus: the input's mode kept and the menus', the sound
 * and the screens taken up. */
u32 enterMenus(App* app, ThreadState& ts, Guest& guest)
{
    fe::direct<0x431bc0, collide::screenStates>(app, ts);
    fe::direct<0x431900, graphics::sendState>(app, ts, 3, 0);
    fe::direct<0x4d5cc0, graphics::drawingBegun>(app, ts, 1, 0);
    const u32 mode = guest.dword(kInputMode);
    guest.setDword(kSavedInputMode, mode);
    guest.setDword(kInputMode, 0x10);
    fe::direct<0x4f0230, frontend::eab::inputStateClose>(app, ts);
    const u32 flags = guest.dword(kInputFlags);
    guest.setDword(kSavedInputFlags, flags);
    guest.setByte(kInputFlags, std::uint8_t(flags | 4));
    fe::direct<0x4136e0, port::raceBankLoadOf>(app, ts, ts.sp - 4);   // the game's code: the flags in eax, unread
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, ts, ts.sp - 4,
                                                     port::Args{});
    return fe::direct<0x49c140, frontend::menud::loadPointerArt>(app, ts);
}

/* sub_473080: out of them, all of it undone. */
u32 leaveMenus(App* app, ThreadState& ts, Guest& guest)
{
    // By address: the stop's ebx is the one the function leaves, and the 0 in edx the removal is handed,
    // which its core does not take, is part of the call this group's check sees.
    fe::direct<0x4109d0, port::musicStopped>(app, ts, ts.sp - 4, 0);
    fe::direct<0x410b20, port::banksRemoveOf>(app, ts, ts.sp - 4);
    guest.setDword(0x55b034, 0);
    fe::direct<0x49c2a0, frontend::menud::freePointerArt>(app, ts, 0);  // eax: its result is not read
    guest.setDword(kInputMode, guest.dword(kSavedInputMode));
    const u32 flags = guest.dword(kSavedInputFlags);
    guest.setDword(kInputFlags, flags);
    fe::direct<0x472360, port::fields::customColoursRefreshed>(app, ts, ts.sp - 4, flags);
    fe::direct<0x472820, load::configSaved>(app, ts);
    fe::direct<0x4d5c40, port::shapes::texturesLetGo>(app, ts, ts.sp - 4);
    return fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, ts);
}

/* sub_4732b0: the screens after a race: the voices its kind has, the menu loop
 * on postgame, its answer. */
u32 runPostgame(App* app, ThreadState& ts, Guest& guest)
{
    constexpr u32 kPostgame = 0x53b054;
    guest::direct<0x473020, frontend::menuc::menusEnter>(app, ts);
    gameSettings(guest).replayMode = 0;
    guest.setDword(kAfterRace, 1);
    switch (gameSettings(guest).raceType)
    {
    case 1:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, ts, ts.sp - 4,
                                                        gameSettings(guest).seriesVariant == 0 ? 2u : 3u);
        break;
    case 2:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, ts, ts.sp - 4, 1);
        break;
    default:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, ts, ts.sp - 4, 4);
        break;
    }
    fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, ts, ts.sp - 4, 0);
    const u32 answer = u32(*fe::direct<0x44b230, port::loop::menuLoopOf>(app, ts, ts.sp - 4, kPostgame, 0));
    fe::direct<0x4c9a00, port::raceVoiceBanksFreed>(app, ts, ts.sp - 4);
    guest::direct<0x44e7d0, frontend::menub::awardsClear>(app, ts);
    guest.setDword(kAfterRace, 0);
    guest::direct<0x473080, frontend::menuc::menusLeave>(app, ts);
    return answer;
}

constexpr std::uint8_t kResultsSaves = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeySaves = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kSortMarkSaves = fe::kRestoreEbx | fe::kRestoreEcx;

}

/* ---------------------------------------------------------------------------
 * The typed cores (native_frontend_menuc.h) and the thunks under the game's names
 *
 * Each core makes its own frame where the game's function has one: the thunk's Site leaves
 * none (frameBytes 0), and what the core calls runs at the stack addresses it did.
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::menuc::resultsTablePass(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
}

nfs::u32 frontend::resultsTablePass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e0b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTablePass(app, ts, item); });
}
nfs::u32 frontend::menuc::standingsTablePass(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
}

nfs::u32 frontend::standingsTablePass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f0a0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTablePass(app, ts, item); });
}
nfs::u32 frontend::menuc::roundsTablePass(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
}

nfs::u32 frontend::roundsTablePass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46fee0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTablePass(app, ts, item); });
}
nfs::u32 frontend::menuc::resultsTableHidden(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
    return u32(0);
}

nfs::u32 frontend::resultsTableHidden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e640", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTableHidden(app, ts, item); });
}
nfs::u32 frontend::menuc::standingsTableHidden(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
    return u32(0);
}

nfs::u32 frontend::standingsTableHidden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f4c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTableHidden(app, ts, item); });
}
nfs::u32 frontend::menuc::roundsTableHidden(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, ts, item);
    return u32(0);
}

nfs::u32 frontend::roundsTableHidden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470340", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTableHidden(app, ts, item); });
}
nfs::u32 frontend::menuc::resultsTableDraw(App* app, ThreadState& ts, u32 item, u32 pointing)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x174);
    drawResults(app, ts, guest, item, pointing != 0);
    return u32(0);
}

nfs::u32 frontend::resultsTableDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46d3c0", 0, 0, nullptr, 0, kResultsSaves };
    const u32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTableDraw(app, ts, item, pointing); });
}
nfs::u32 frontend::menuc::standingsTableDraw(App* app, ThreadState& ts, u32 item, u32 pointing)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x270);
    drawStandings(app, ts, guest, item, pointing != 0);
    return u32(0);
}

nfs::u32 frontend::standingsTableDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e690", 0, 0, nullptr, 0, kResultsSaves };
    const u32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTableDraw(app, ts, item, pointing); });
}
nfs::u32 frontend::menuc::roundsTableDraw(App* app, ThreadState& ts, u32 item, u32 pointing)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x16c);
    drawRounds(app, ts, guest, item, pointing != 0);
    return u32(0);
}

nfs::u32 frontend::roundsTableDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f510", 0, 0, nullptr, 0, kResultsSaves };
    const u32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTableDraw(app, ts, item, pointing); });
}
nfs::u32 frontend::menuc::resultsPopupColumn(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    return resultsPopupUnderPointer(app, ts, guest, item);
}

nfs::u32 frontend::resultsPopupColumn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46cdf0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsPopupColumn(app, ts, item); });
}
nfs::u32 frontend::menuc::resultsLapTimesPopup(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x114);
    return drawLapTimesPopup(app, ts, guest, item);
}

nfs::u32 frontend::resultsLapTimesPopup(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46d180", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsLapTimesPopup(app, ts, item); });
}
nfs::u32 frontend::menuc::resultsTableShown(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    return resultsOnShow(app, ts, guest, item);
}

nfs::u32 frontend::resultsTableShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46df90", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTableShown(app, ts, item); });
}
nfs::u32 frontend::menuc::standingsTableShown(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    return standingsOnShow(app, ts, guest, item);
}

nfs::u32 frontend::standingsTableShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46ef90", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTableShown(app, ts, item); });
}
nfs::u32 frontend::menuc::resultsTableLayout(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return layoutResults(app, ts, guest, item);
}

nfs::u32 frontend::resultsTableLayout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e0d0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTableLayout(app, ts, item); });
}
nfs::u32 frontend::menuc::loadMenuArtSlots(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    return loadMenuArt(app, ts, guest, item);
}

nfs::u32 frontend::loadMenuArtSlots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_465d80", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::loadMenuArtSlots(app, ts, item); });
}
nfs::u32 frontend::menuc::titleDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x134);
    return drawTitle(app, ts, guest);
}

nfs::u32 frontend::titleDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_472f20", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::titleDraw(app, ts); });
}
nfs::u32 frontend::menuc::titleWait(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return waitOnTitle(app, ts, guest);
}

nfs::u32 frontend::titleWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_472fc0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::titleWait(app, ts); });
}
nfs::u32 frontend::menuc::menusEnter(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return enterMenus(app, ts, guest);
}

nfs::u32 frontend::menusEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_473020", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::menusEnter(app, ts); });
}
nfs::u32 frontend::menuc::menusLeave(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return leaveMenus(app, ts, guest);
}

nfs::u32 frontend::menusLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_473080", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::menusLeave(app, ts); });
}
nfs::u32 frontend::menuc::postgameRun(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return runPostgame(app, ts, guest);
}

nfs::u32 frontend::postgameRun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4732b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::postgameRun(app, ts); });
}

/* sub_45cb50: the HUD editor's element, hidden: nothing to do. */
void frontend::menuc::hudElementHidden(nfs::App* , nfs::ThreadState& )
{
}

/* sub_470390: the side by side table has nothing to do on a pass. */
void frontend::menuc::headToHeadPass(nfs::App* , nfs::ThreadState& )
{
}

/* sub_473010: a comparison with the intro's flag, for no one. */
void frontend::menuc::skipIntroCompare(nfs::App* , nfs::ThreadState& )
{
}

/* sub_460760: the buttons' pulse starts again from its brightest, as the
 * selected item changes. */
void frontend::menuc::resetButtonPulse(nfs::App* app, nfs::ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x559fdc, 0xff);
}

/* sub_465d60: no chrome, no glow. */
void frontend::menuc::clearMenuArtSlots(nfs::App* app, nfs::ThreadState& ts)
{
    Guest guest(app, ts);
    clearMenuArt(guest);
}

nfs::u32 frontend::hudElementHidden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45cb50", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuc::hudElementHidden(app, ts);
        return args.eax;
    });
}
nfs::u32 frontend::headToHeadPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470390", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuc::headToHeadPass(app, ts);
        return args.eax;
    });
}
nfs::u32 frontend::skipIntroCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_473010", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuc::skipIntroCompare(app, ts);
        return args.eax;
    });
}
nfs::u32 frontend::resetButtonPulse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_460760", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuc::resetButtonPulse(app, ts);
        return args.eax;
    });
}
nfs::u32 frontend::clearMenuArtSlots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_465d60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menuc::clearMenuArtSlots(app, ts);
        return args.eax;
    });
}

/* sub_4654c0: the two values on the stack kept, in the slots at 0x678954.  What
 * is left in eax is the first. */
nfs::u32 frontend::menuc::setSliderComponents(App* app, ThreadState& ts, u32 first, u32 second)
{
    Guest guest(app, ts);
    guest.setDword(0x678958, second);
    guest.setDword(0x678954, first);
    return first;
}

nfs::u32 frontend::setSliderComponents(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4654c0", 8, 0, nullptr, 0, 0 };
    const u32 first = app->getMemory<u32>(ts.sp + 4), second = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::setSliderComponents(app, ts, first, second); });
}

nfs::u32 frontend::menuc::drawColumnSortMark(App* app, ThreadState& ts, u32 topLeft, u32 widthHeight, u32 shape)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 36);
    return drawSortMark(app, ts, guest, topLeft, widthHeight, shape);
}

nfs::u32 frontend::drawColumnSortMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46cce0", 12, 0, nullptr, 0 };
    const u32 arguments = ts.sp + 4;
    const u32 topLeft = app->getMemory<u32>(arguments), widthHeight = app->getMemory<u32>(arguments + 4);
    const u32 shape = app->getMemory<u32>(arguments + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menuc::drawColumnSortMark(app, ts, topLeft, widthHeight, shape);
    });
}

nfs::u32 frontend::menuc::resultsSortMark(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return drawTableSortMark(app, ts, guest, item, 0x48, 0x86, 0x80);
}

nfs::u32 frontend::resultsSortMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46cdc0", 0, 0, nullptr, 0, kSortMarkSaves };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsSortMark(app, ts, item); });
}

nfs::u32 frontend::menuc::standingsSortMark(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return drawTableSortMark(app, ts, guest, item, 0x46, 0x7c, 0x76);
}

nfs::u32 frontend::standingsSortMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e660", 0, 0, nullptr, 0, kSortMarkSaves };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsSortMark(app, ts, item); });
}

nfs::u32 frontend::menuc::roundsSortMark(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return drawTableSortMark(app, ts, guest, item, 0x46, 0x7c, 0x76);
}

nfs::u32 frontend::roundsSortMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f4e0", 0, 0, nullptr, 0, kSortMarkSaves };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsSortMark(app, ts, item); });
}

nfs::u32 frontend::menuc::resultsTableKey(App* app, ThreadState& ts, u32 item, u32, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0xc);
    return resultsKey(app, ts, guest, item, std::uint16_t(key));
}

nfs::u32 frontend::resultsTableKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46e270", 0, 0, nullptr, 0, kKeySaves };
    const u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::resultsTableKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::menuc::standingsTableLayout(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return layoutStandings(app, ts, guest, item, kStandingsType, kResultsCount);
}

nfs::u32 frontend::standingsTableLayout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f0c0", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTableLayout(app, ts, item); });
}

nfs::u32 frontend::menuc::standingsTableKey(App* app, ThreadState& ts, u32 item, u32, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0xc);
    return standingsKey(app, ts, guest, item, std::uint16_t(key), kStandingsType, kResultsCount);
}

nfs::u32 frontend::standingsTableKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46f200", 0, 0, nullptr, 0, kKeySaves };
    const u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::standingsTableKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::menuc::roundsTableShown(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return roundsOnShow(app, ts, guest, item);
}

nfs::u32 frontend::roundsTableShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46fe30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTableShown(app, ts, item); });
}

nfs::u32 frontend::menuc::roundsTableLayout(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return layoutStandings(app, ts, guest, item, kRoundsType, kRoundsCount);
}

nfs::u32 frontend::roundsTableLayout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_46ff00", 0, 0, nullptr, 0 };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTableLayout(app, ts, item); });
}

nfs::u32 frontend::menuc::roundsTableKey(App* app, ThreadState& ts, u32 item, u32, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0xc);
    return standingsKey(app, ts, guest, item, std::uint16_t(key), kRoundsType, kRoundsCount);
}

nfs::u32 frontend::roundsTableKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470040", 0, 0, nullptr, 0, kKeySaves };
    const u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::roundsTableKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::menuc::headToHeadShown(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return sideBySideOnShow(guest, item);
}

nfs::u32 frontend::headToHeadShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470360", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::headToHeadShown(app, ts, item); });
}

nfs::u32 frontend::menuc::headToHeadDraw(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x154);
    drawSideBySide(app, ts, guest, item);
    return 0;
}

nfs::u32 frontend::headToHeadDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4703a0", 0, 0, nullptr, 0, kResultsSaves };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::headToHeadDraw(app, ts, item); });
}

/* sub_477c00: the flag at 0x55d000 down. */
nfs::u32 frontend::menuc::clearLoadFlag(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kLoadFlag, 0);
    return 0;
}

nfs::u32 frontend::clearLoadFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477c00", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menuc::clearLoadFlag(app, ts); });
}


}
