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
using x86::reg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The menu engine's.
constexpr reg32 kItemShownNow = 0x44a660;      // eax the item, edx its widget's type
constexpr reg32 kPointerX = 0x55e5e4;
constexpr reg32 kPointerY = 0x55e5e8;
constexpr std::uint32_t kClick = 1, kClickVolume = 0x7f;

// Text and drawing.
constexpr reg32 kSprintf = 0x4df690;
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

constexpr std::uint32_t kRaceDuel = 3;

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
    Table(App* app, CPU& cpu, Guest& guest, reg32 item, const Fields& fields, std::uint32_t type)
        : app(app), cpu(cpu), g(guest), item(item), f(fields), type(type)
    {
    }

    // By address, as the shown-nows below: the type in edx, which the core does not take, is part of
    // the call this group's check sees.
    bool shown() const { return fe::direct<kItemShownNow, frontend::loop::shownNow>(g.app(), g.cpu(), item) != 0; }

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
        const std::uint32_t colours[] = { c1, c2, c3, c4 };
        fe::direct<0x4d8860, port::draw2d::shadedBox>(app, cpu, cpu.esp - 20, bx, by, w, h, colours);
    }

    void text(reg32 string, std::int32_t tx, std::int32_t ty, std::uint32_t align, std::uint32_t colour) const
    {
        fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, string, reg32(tx), reg32(ty), kFont,
                                                        align, colour);
    }

    void fitted(reg32 string, std::int32_t tx, std::int32_t ty, std::uint32_t colour, std::int32_t room) const
    {
        guest::direct<0x452120, frontend::menub::textDrawFitted>(app, cpu, string, reg32(tx), reg32(ty), kFont,
                                                                 kAlignLeft, colour, reg32(room));
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
        const bool hot = canPoint && fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, at) != 0;
        if (hot)
            fe::direct<0x4d8930, port::draw2d::outline>(app, cpu, cpu.esp - 8, reg32(left + inset), reg32(s16(g, at)),
                                                        reg32(s16(g, at + 4) - inset), reg32(s16(g, at + 6) - inset),
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

    bool isThisPlayer(reg32 record) const
    {
        const reg32 player = g.dword(record + kRacerPlayer);
        return player == gameSettings(g).localPlayer || (gameSettings(g).playMode == 1 && player == 1);
    }

    /* The six shapes named, in their places: the last, what the calls leave in eax. */
    reg32 loadArrows() const
    {
        static constexpr reg32 kShapes[] = { kArtUp, kArtDown, kArtUpHot, kArtDownHot, kArtDown, kArtUp };
        reg32 shape = 0;
        for (int i = 0; i < 6; ++i)
        {
            shape = reg32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, cpu, cpu.esp - 4, kShapes[i]));
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
    reg32 scrollToPlayer(std::int32_t count, reg32 lastEax) const
    {
        for (std::int32_t i = 0; i < count; ++i)
        {
            lastEax = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(i), 0, 0);
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

    App* const app;
    CPU& cpu;
    Guest& g;
    const reg32 item;
    const Fields& f;
    const std::uint32_t type;

private:
    void arrow(reg32 boxField, int shape, std::int32_t nudge) const
    {
        const reg32 at = item + boxField;
        const bool hot = fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, at) != 0;
        const reg32 art = item + f.shapes + (hot ? 4 + shape : shape);
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, cpu, cpu.esp - 4, reg32(s16(g, art)),
                                                        reg32(s16(g, at + 2)), reg32(s16(g, at) + nudge));
    }
};

/* The pointer put on a box, three quarters along it, half way down. */
void setPointer(App* app, CPU& cpu, Guest& guest, std::int32_t x, std::int32_t y)
{
    guest.setDword(kPointerX, reg32(x));
    guest.setDword(kPointerY, reg32(y));
    fe::direct<0x49bfc0, frontend::screen::setPointer>(app, cpu, reg32(x), reg32(y));
}

void pointAtBox(App* app, CPU& cpu, Guest& guest, reg32 box)
{
    setPointer(app, cpu, guest, s16(guest, box + 2) + (3 * s16(guest, box + 4)) / 4,
               s16(guest, box) + half(s16(guest, box + 6)));
}

/* ---------------------------------------------------------------------------
 * The little mark over a table's sorted column
 * ------------------------------------------------------------------------- */

/* sub_46cce0: "sort by" and an arrow over a column's title, over two lines as
 * wide as the words.  On the stack: the column's box, its top and left, then
 * its width and height, then the arrow's shape. */
reg32 drawSortMark(App* app, CPU& cpu, Guest& guest, reg32 topLeft, reg32 widthHeight, reg32 shape)
{
    const std::int32_t top = std::int16_t(topLeft);
    const std::int32_t left = std::int16_t(topLeft >> 16);
    const std::int32_t halfWidth = half(std::int16_t(widthHeight));
    const reg32 measured = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineSortBy,
                                                                             0));
    const std::int32_t textWidth = std::int32_t(reg32(*fe::direct<0x452020, port::text::menuTextWidth>(
        app, cpu, cpu.esp - 4, measured, kFont)));
    const std::int32_t centre = left + halfWidth;
    const reg32 line = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineSortBy, 0));
    fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, line, reg32(centre), reg32(top - 0x1e),
                                                    kFont, kAlignCentre, kRowText);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, cpu, cpu.esp - 4, shape, reg32(left - 6 + halfWidth),
                                                    reg32(top - 0xe));
    const std::int32_t halfText = half(textWidth);
    const reg32 from = reg32(centre - halfText), to = reg32(centre + halfText);
    fe::direct<0x4d8ed0, port::draw2d::drawLineOf>(app, cpu, cpu.esp - 12, from, reg32(top - 0xd), to, reg32(top - 0xd),
                                                   0xff5f78a9, 0xff5f78a9);
    return reg32(*fe::direct<0x4d8ed0, port::draw2d::drawLineOf>(app, cpu, cpu.esp - 12, from, reg32(top - 0xc), to,
                                                                 reg32(top - 0xc), 0xff738cbd, 0xff738cbd));
}

/* sub_46cdc0, sub_46e660, sub_46f4e0: that mark, over the table's own column,
 * which they leave in edx. */
reg32 drawTableSortMark(App* app, CPU& cpu, Guest& guest, reg32 item, reg32 boxes, reg32 sort, reg32 shape)
{
    const reg32 column = reg32(s16(guest, item + sort));
    const reg32 at = item + boxes + 8 * column;
    const reg32 result = guest::direct<0x46cce0, frontend::menuc::drawColumnSortMark>(
        app, cpu, guest.dword(at), guest.dword(at + 4), reg32(s16(guest, item + shape)));
    return result;
}

/* ---------------------------------------------------------------------------
 * The results, sorted by a column
 * ------------------------------------------------------------------------- */

/* sub_46cdf0: which column of a row the pointer is in, for its popups: 1 the
 * lap times', 2 the metric's, 0 neither.  A wrecked racer has no laps. */
reg32 resultsPopupUnderPointer(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
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
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, box) != 0)
        {
            const reg32 record =
                fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(row + t.scroll()), 0, 0);
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
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, box) != 0)
            {
                if (fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(row + t.scroll()), 0, 0) != 0)
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
reg32 drawLapTimesPopup(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    const reg32 box = guest.frame() + 0x100;
    guest.setWord(box + 2, std::uint16_t(std::uint32_t(guest.word(item + 0x3c)) + guest.word(item + 0x8a) + guest.word(item + 0x3e)));
    guest.setWord(box, std::uint16_t(guest.word(item + kItemY) + guest.word(item + 0x44)));
    guest.setWord(box + 4, std::uint16_t(guest.word(item + 0x40) + 1));
    guest.setWord(box + 6, std::uint16_t(guest.word(item + 0x44) + 1));
    for (std::int32_t row = 0; row < t.rows(); ++row)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, box) == 0)
        {
            guest.setWord(box, std::uint16_t(guest.word(box) + guest.word(item + 0x44)));
            continue;
        }
        const reg32 record =
            fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(row + t.scroll()), 0, 0);
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
        fe::direct<0x4d8ac0, port::draw2d::drawFieldBoxOf>(app, cpu, cpu.esp - 20, left + 2, top + 2, width - 3,
                                                           15 * laps + 0x20, kPopupColours);
        const reg32 title = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineLapTimes,
                                                                              0));
        t.text(title, left + half(width), top + 8, kAlignCentre, kHotText);
        std::int32_t lineY = top + 0x18;
        reg32 last = reg32(top);
        for (std::int32_t lap = 0; lap < laps; ++lap)
        {
            const std::uint32_t colour = lap == best ? kBestLap : kOtherLap;
            fe::direct<0x4cee40, frontend::menue::timeText>(app, cpu, guest.dword(record + kRacerLaps + 4 * lap),
                                                            buffer);
            t.text(buffer, left + width - 8, lineY, kAlignRight, colour);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatLapNumber, guest::Words{reg32(lap + 1)});
            last = reg32(*fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, buffer,
                                                                          reg32(left + 8), reg32(lineY), kFont,
                                                                          kAlignLeft, colour));
            lineY += 0xf;
        }
        return last;
    }
    return reg32(t.rows());
}

/* The last column: the laps done in a duel, the result otherwise. */
void lastColumnText(App* app, CPU& cpu, const Table& t, reg32 record, reg32 buffer)
{
    Guest& guest = t.g;
    if (gameSettings(guest).raceType == kRaceDuel)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatNumber, guest::Words{guest.dword(record + kRacerLapsDone)});
    else
        guest::direct<0x455720, frontend::menub::resultsTopSpeedText>(app, cpu, record, buffer);
}

/* One racer's row in the results' plain form. */
void drawResultsRowPlain(App* app, CPU& cpu, const Table& t, reg32 record, reg32 buffer, std::int32_t rowY, bool dimmed)
{
    const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    const reg32 place = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoPlace,
                                                                          0));
    t.text(place, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest::direct<0x455570, frontend::menub::resultsName>(app, cpu, record, buffer);
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest::direct<0x455600, frontend::menub::resultsCarName>(app, cpu, record, buffer);
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    const reg32 out = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoTime, 0));
    t.text(out, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
    x += t.width(2);
    lastColumnText(app, cpu, t, record, buffer);
    t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
}

/* One racer's row in the results; `popup`, a column of it dimmed under its popup. */
void drawResultsRow(App* app, CPU& cpu, const Table& t, reg32 record, reg32 buffer, std::int32_t rowY, bool dimmed,
                    reg32 popup)
{
    Guest& guest = t.g;
    const std::uint32_t status = guest.dword(record + kRacerStatus);
    const bool wrecked = status == kStatusWrecked;
    const std::uint32_t colour = dimmed ? kDimmedText : (status == kStatusOut || wrecked) ? kOutText : kRowText;
    const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
    std::int32_t x = t.x();
    if (wrecked)
        guest.copyString(buffer, reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoPlace,
                                                                                   0)));
    else
        guest::direct<0x4555c0, frontend::menub::resultsPlaceText>(app, cpu, record, buffer);
    t.text(buffer, x + half(t.width(0)), rowY, kAlignCentre, colour);
    x += t.width(0);
    guest::direct<0x455570, frontend::menub::resultsName>(app, cpu, record, buffer);
    t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
    guest::direct<0x455600, frontend::menub::resultsCarName>(app, cpu, record, buffer);
    t.fitted(buffer, x + 10, rowY + 5, colour, t.width(1));
    x += t.width(1);
    if (wrecked)
    {
        const reg32 line = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineWrecked,
                                                                             0));
        t.text(line, x + half(t.width(2)), rowY, kAlignCentre, colour);
        x += t.width(2);
        lastColumnText(app, cpu, t, record, buffer);
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
        return;
    }
    fe::direct<0x4cee40, frontend::menue::timeText>(app, cpu, guest.dword(record + kRacerTime), buffer);
    t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, popup == 1 ? kDimmedText : colour);
    x += t.width(2);
    if (gameSettings(guest).raceType == kRaceDuel)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatOfTotal, guest::Words{guest.dword(record + kRacerLapsDone), gameSettings(guest).pursuitOutCount});
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, colour);
    }
    else
    {
        guest::direct<0x455720, frontend::menub::resultsTopSpeedText>(app, cpu, record, buffer);
        t.text(buffer, x + half(t.width(3)), rowY, kAlignCentre, popup == 2 ? kDimmedText : colour);
    }
}

/* sub_46d3c0: the results table: sliding, sorted as chosen, its box and lines,
 * the titles, a row for each racer shown, the popups, the arrows. */
void drawResults(App* app, CPU& cpu, Guest& guest, reg32 item, bool pointing)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46e0d0, frontend::menuc::resultsTableLayout>(app, cpu, item);
    const bool dimmed = reg32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, cpu, cpu.esp - 4, 0)) != 0;
    switch (guest.word(item + 0x86))
    {
    case 0:
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 1, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 8, 0);
        break;
    case 3:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0,
                                                          gameSettings(guest).raceType == kRaceDuel ? 6u : 2u, 1);
        break;
    default:
        break;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, cpu) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const std::int32_t shownRows = smaller(t.rows(), count);
    const std::int32_t tableWidth = t.width(0) + t.width(1) + t.width(2) + t.width(3);
    const std::int32_t height = t.rowHeight() * (shownRows + 1);
    const bool canPoint = pointing && !plain;
    const reg32 popup = canPoint ? guest::direct<0x46cdf0, frontend::menuc::resultsPopupColumn>(app, cpu, item) : 0;
    const std::int32_t x = t.x(), y = t.y();
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    if (t.rows() < count)
        fe::direct<0x4d8930, port::draw2d::outline>(app, cpu, cpu.esp - 8, reg32(s16(guest, item + 0x6a) - 3),
                                                    reg32(s16(guest, item + 0x68) - 1), 0x12, reg32(height + 4), kGold);
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height + 2, 3);
    if (t.state() == kSlideNone && count > 1 && !plain)
        guest::direct<0x46cdc0, frontend::menuc::resultsSortMark>(app, cpu, item);

    static constexpr std::uint32_t kTitles[] = { 227, 200, 40, 41 };
    static constexpr std::uint32_t kAligns[] = { kAlignCentre, kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 4; ++column)
    {
        reg32 title = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kTitles[column],
                                                                        0));
        if (gameSettings(guest).raceType == kRaceDuel && column == 3)
            title = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineLaps, 0));
        t.title(column, title, kAligns[column], canPoint, dimmed, 1);
    }

    std::int32_t rowY = y + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(place), 0, 0);
        if (record == 0)
            break;
        if (plain)
            drawResultsRowPlain(app, cpu, t, record, buffer, rowY, dimmed);
        else
            drawResultsRow(app, cpu, t, record, buffer, rowY, dimmed, popup);
        rowY += t.rowHeight();
    }
    if (canPoint)
    {
        guest::direct<0x46d180, frontend::menuc::resultsLapTimesPopup>(app, cpu, item);
        fe::direct<0x46cf30, port::view::metricPopupOf>(app, cpu, cpu.esp - 4, item);
    }
    t.arrows(count, 1);
}

/* sub_46df90: the results as their screen opens: the arrows' art named, sorted
 * by place, the player's row in view. */
reg32 resultsOnShow(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    t.placeIfNew();
    guest.setWord(item + 0x86, 0);
    guest.setWord(item + 0x84, 0);
    const reg32 sorted = fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 1, 0);
    return t.scrollToPlayer(count, sorted);
}

/* sub_46ef90: the same for the standings, sorted by their third column. */
reg32 standingsOnShow(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kStandingsFields, kStandingsType);
    if (!t.shown())
        return 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    t.loadArrows();
    guest.setWord(item + 0x7c, 2);
    guest.setWord(item + 0x7a, 0);
    t.placeIfNew();
    const reg32 sorted = fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 4, 1);
    return t.scrollToPlayer(count, sorted);
}

/* sub_46fe30: the same for the second list, which its draw sorts. */
reg32 roundsOnShow(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kStandingsFields, kRoundsType);
    if (!t.shown())
        return 0;
    const reg32 lastShape = t.loadArrows();
    guest.setWord(item + 0x7c, 2);
    const bool placed = t.placeIfNew();
    guest.setWord(item + 0x7a, 0);
    return placed ? withLow(lastShape, guest.word(item + kItemX)) : lastShape;
}

/* sub_46e0d0: the results' boxes as the table stands now: a title's for each
 * column, the whole table's, the arrows'.  What is left in eax is the last sum
 * they were made with. */
reg32 layoutResults(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
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
reg32 layoutStandings(App* app, CPU& cpu, Guest& guest, reg32 item, std::uint32_t type, reg32 countAt)
{
    const Table t(app, cpu, guest, item, kStandingsFields, type);
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

void click(win32::WinApplication* app, x86::CPU& cpu)
{
    fe::direct<0x4181d0, port::playMenuEffectOf>(app, cpu, cpu.esp - 4, kClick, kClickVolume);
}

/* sub_46e270: the results' keys: Enter on a title sorts by it, left and right
 * go along the titles, the arrows scroll.  2 when a key was used; it leaves
 * that in edx too, and in ebx where it has got to. */
reg32 resultsKey(App* app, CPU& cpu, Guest& guest, reg32 item, std::uint16_t key)
{
    const Table t(app, cpu, guest, item, kResultsFields, kResultsType);
    reg32 ebx = 0xffffffff;
    if (!t.shown())
    {
        return 0;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, cpu) != 0;
    const std::int32_t count = std::int32_t(guest.dword(kResultsCount));
    const bool sortable = count > 1 && !plain;
    for (int column = 0; column < 4; ++column)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, t.box(column)) != 0)
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
        click(app, cpu);
    }
    if (sortable && (key == kKeyRight || key == kKeyLeft))
    {
        std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
        column = column < 0 ? 0 : column > 3 ? 3 : column;
        ebx = reg32(column);
        pointAtBox(app, cpu, guest, t.box(column));
        used = true;
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x84) != 0)
        {
            click(app, cpu);
            ebx = withLow(ebx, guest.word(item + 0x84)) - 1;
            guest.setWord(item + 0x84, std::uint16_t(ebx));
            used = true;
            pointAtBox(app, cpu, guest, item + 0x68);
        }
        if (key == kKeyPageDown)
        {
            ebx = reg32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(app, cpu);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
                pointAtBox(app, cpu, guest, item + 0x70);
            }
        }
        if (key == kEnter)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, item + 0x68) != 0
                && guest.word(item + 0x84) != 0)
            {
                click(app, cpu);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) - 1));
                used = true;
            }
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, item + 0x70) != 0
                && t.rows() + t.scroll() < count)
            {
                click(app, cpu);
                guest.setWord(item + 0x84, std::uint16_t(guest.word(item + 0x84) + 1));
                used = true;
            }
        }
    }
    const reg32 result = used ? 2 : 0;
    return result;
}

/* sub_46f200, sub_470040: the keys of the others.  sub_470040 takes left and
 * right both as the next title, and points with that one's width and height
 * at the top and left of the one after it: the game's own slip. */
reg32 standingsKey(App* app, CPU& cpu, Guest& guest, reg32 item, std::uint16_t key, std::uint32_t type, reg32 countAt)
{
    const bool rounds = type == kRoundsType;
    const Table t(app, cpu, guest, item, kStandingsFields, type);
    reg32 ebx = 0xffffffff;
    if (!t.shown())
    {
        return 0;
    }
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, cpu) != 0;
    const std::int32_t count = std::int32_t(guest.dword(countAt));
    for (int column = 0; column < 3; ++column)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, t.box(column)) != 0)
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
        click(app, cpu);
    }
    if (!plain && (key == kKeyRight || key == kKeyLeft))
    {
        used = true;
        if (rounds)
        {
            ebx = reg32((std::int32_t(ebx) + 1) % 3);
            const reg32 box = t.box(int(ebx));
            setPointer(app, cpu, guest, s16(guest, box + 10) + (3 * s16(guest, box + 4)) / 4,
                       s16(guest, box + 8) + half(s16(guest, box + 6)));
        }
        else
        {
            std::int32_t column = std::int32_t(ebx) + (key == kKeyRight ? 1 : -1);
            column = column < 0 ? 0 : column > 2 ? 2 : column;
            ebx = reg32(column) << 3;
            pointAtBox(app, cpu, guest, t.box(column));
        }
    }
    if (t.rows() < count)
    {
        if (key == kKeyPageUp && guest.word(item + 0x7a) != 0)
        {
            click(app, cpu);
            ebx = withLow(ebx, guest.word(item + 0x7a)) - 1;
            guest.setWord(item + 0x7a, std::uint16_t(ebx));
            used = true;
            pointAtBox(app, cpu, guest, item + 0x5e);
        }
        if (key == kKeyPageDown)
        {
            ebx = reg32(count);
            if (t.rows() + t.scroll() < count)
            {
                click(app, cpu);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) + 1));
                used = true;
                pointAtBox(app, cpu, guest, item + 0x66);
            }
        }
        if (key == kEnter)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, item + 0x5e) != 0
                && guest.word(item + 0x7a) != 0)
            {
                click(app, cpu);
                guest.setWord(item + 0x7a, std::uint16_t(guest.word(item + 0x7a) - 1));
                used = true;
            }
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, cpu, item + 0x66) != 0
                && t.rows() + t.scroll() < count)
            {
                click(app, cpu);
                const std::uint16_t scroll = guest.word(item + 0x7a);
                if (!rounds)
                    ebx = withLow(ebx, scroll) + 1;
                guest.setWord(item + 0x7a, std::uint16_t(scroll + 1));
                used = true;
            }
        }
    }
    const reg32 result = used ? 2 : 0;
    return result;
}

/* The standings' box: its background and lines, as sub_46e690 and sub_46f510
 * both draw it. */
void drawStandingsBox(const Table& t, std::int32_t shownRows, std::int32_t tableWidth, std::int32_t height, std::int32_t count,
                      int columnLines)
{
    Guest& guest = t.g;
    App* const app = t.app;
    CPU& cpu = t.cpu;
    const std::int32_t x = t.x(), y = t.y();
    fe::direct<0x4d8930, port::draw2d::outline>(app, cpu, cpu.esp - 8, reg32(x), reg32(y), reg32(tableWidth + 2),
                                                reg32(height + 2), kBlack);
    t.shaded(x - 2, y - 2, tableWidth + 4, 2, kBlue, kGold, kGold, kBlue);
    t.shaded(x - 2, y + height + 2, tableWidth + 4, 2, kGold, kGold, kGold, kGold);
    t.shaded(x - 2, y - 2, 2, height + 4, kBlue, kBlue, kGold, kGold);
    t.shaded(x + tableWidth, y - 2, 2, height + 4, kGold, kGold, kGold, kGold);
    if (t.rows() < count)
        fe::direct<0x4d8930, port::draw2d::outline>(app, cpu, cpu.esp - 8, reg32(s16(guest, t.item + 0x60) - 3),
                                                    reg32(s16(guest, t.item + 0x5e) - 3), 0x12, reg32(height + 6),
                                                    kGold);
    t.shaded(x, y + t.rowHeight() - 1, tableWidth, 2, kBlue, kGold, kGold, kBlue);
    t.rowLines(shownRows, tableWidth);
    t.columnLines(height, columnLines);
}

/* sub_46e690: a series' standings: each racer's name and car, his rank and
 * place, and his other rank. */
void drawStandings(App* app, CPU& cpu, Guest& guest, reg32 item, bool pointing)
{
    const Table t(app, cpu, guest, item, kStandingsFields, kStandingsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    const reg32 placeName = guest.frame() + 0x100;    // char[0x100]
    const reg32 firstRank = guest.frame() + 0x210;
    const reg32 secondRank = guest.frame() + 0x214;
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46f0c0, frontend::menuc::standingsTableLayout>(app, cpu, item);
    const bool dimmed = reg32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, cpu, cpu.esp - 4, 0)) != 0;
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, cpu) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 8, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 3, 1);
        break;
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 0, 4, 1);
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
        guest::direct<0x46e660, frontend::menuc::standingsSortMark>(app, cpu, item);

    static constexpr std::uint32_t kTitles[] = { 200, 239, 240 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const reg32 title = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kTitles[column],
                                                                              0));
        t.title(column, title, kAligns[column], !plain && pointing, dimmed, 0);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 0, reg32(place), 0, 0);
        if (record == 0)
            break;
        const bool out = guest.dword(record + kRacerStatus) == kStatusOut;
        fe::direct<0x455530, frontend::misc::resultsParticipantRanks>(app, cpu, record, firstRank, secondRank);
        const std::uint32_t colour = dimmed ? kDimmedText : (!plain && out) ? kOutText : kRowText;
        const std::uint32_t nameColour = t.isThisPlayer(record) ? kPlayerText : colour;
        std::int32_t x = t.x();
        guest::direct<0x455570, frontend::menub::resultsName>(app, cpu, record, buffer);
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, nameColour);
        guest::direct<0x455600, frontend::menub::resultsCarName>(app, cpu, record, buffer);
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const reg32 line = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoTime,
                                                                                 0));
            t.text(line, x + half(t.width(1)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            guest::direct<0x4555c0, frontend::menub::resultsPlaceText>(app, cpu, record, placeName);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatNumberAndName, guest::Words{guest.dword(firstRank), placeName});
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
        }
        x += t.width(1);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatNumber, guest::Words{guest.dword(secondRank)});
        t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
        rowY += t.rowHeight();
    }
    t.arrows(count, 0);
}

/* sub_46f510: the second list of racers: each one's name and car, a count of
 * his against the count at 0x6fd4b8, his time once he has reached it. */
void drawRounds(App* app, CPU& cpu, Guest& guest, reg32 item, bool pointing)
{
    const Table t(app, cpu, guest, item, kStandingsFields, kRoundsType);
    const reg32 buffer = guest.frame();               // char[0x100]
    if (!t.shown())
        return;
    t.slide();
    if (t.state() == kSlideGone)
        return;
    guest::direct<0x46ff00, frontend::menuc::roundsTableLayout>(app, cpu, item);
    const bool dimmed = reg32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, cpu, cpu.esp - 4, 0)) != 0;
    const bool plain = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, cpu) != 0;
    switch (guest.word(item + 0x7c))
    {
    case 0:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 1, 8, 0);
        break;
    case 1:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 1, 5, 1);
        break;
    case 2:
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, cpu, 1, 7, 0);
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
        guest::direct<0x46f4e0, frontend::menuc::roundsSortMark>(app, cpu, item);

    static constexpr std::uint32_t kTitles[] = { 200, 247, 405 };
    static constexpr std::uint32_t kAligns[] = { kAlignLeft, kAlignCentre, kAlignCentre };
    for (int column = 0; column < 3; ++column)
    {
        const reg32 title = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kTitles[column],
                                                                              0));
        t.title(column, title, kAligns[column], pointing && !plain, dimmed, 1);
    }

    std::int32_t rowY = t.y() + t.rowHeight() + half(t.rowHeight()) - 5;
    std::int32_t place = t.scroll();
    for (std::int32_t row = 0; row < shownRows && place < count; ++row, ++place)
    {
        const reg32 record = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, cpu, 1, reg32(place), 0, 0);
        if (record == 0)
            break;
        const std::uint32_t colour = dimmed ? kDimmedText : kRowText;
        std::int32_t x = t.x();
        guest::direct<0x455570, frontend::menub::resultsName>(app, cpu, record, buffer);
        t.text(buffer, x + 10, rowY - 10, kAlignLeft, colour);
        guest::direct<0x455600, frontend::menub::resultsCarName>(app, cpu, record, buffer);
        t.fitted(buffer, x + 10, rowY + 5, colour, t.width(0));
        x += t.width(0);
        if (plain)
        {
            const reg32 none = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoPlace,
                                                                                 0));
            t.text(none, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            const reg32 line = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, kLineNoTime,
                                                                                 0));
            t.text(line, x + half(t.width(2)), rowY, kAlignCentre, dimmed ? kDimmedText : kOutText);
        }
        else
        {
            const std::int32_t won = std::int32_t(guest.dword(record + kRacerRoundsDone));
            const std::int32_t toWin = std::int32_t(gameSettings(guest).pursuitWinCount);
            if (won >= toWin)
            {
                const reg32 winner = reg32(*fe::direct<0x4d1850, port::text::lineText>(
                    app, cpu, cpu.esp - 4, kLineWinner, 0));
                fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatOfTotalAndName, guest::Words{reg32(won), reg32(toWin), winner});
            }
            else
            {
                fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, kFormatOfTotal, guest::Words{reg32(won), reg32(toWin)});
            }
            t.text(buffer, x + half(t.width(1)), rowY, kAlignCentre, colour);
            x += t.width(1);
            if (won >= toWin)
            {
                fe::direct<0x4cee40, frontend::menue::timeText>(app, cpu, guest.dword(record + kRacerRoundsTime),
                                                                buffer);
                t.text(buffer, x + half(t.width(2)), rowY, kAlignCentre, colour);
            }
            else
            {
                const reg32 none = reg32(*fe::direct<0x4d1850, port::text::lineText>(
                    app, cpu, cpu.esp - 4, kLineNoPlace, 0));
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
constexpr int kSideRows = 5;
constexpr reg32 kRacerBests[] = { 0x268, 0x270, 0x278, 0x280 };

/* sub_470360: where the table stands, the first time. */
reg32 sideBySideOnShow(Guest& guest, reg32 item)
{
    const reg32 shown = fe::direct<kItemShownNow, frontend::loop::shownNow>(guest.app(), guest.cpu(), item);
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
void drawSideBySide(App* app, CPU& cpu, Guest& guest, reg32 item)
{
    const reg32 buffer = guest.frame();               // char[0x100]
    if (fe::direct<kItemShownNow, frontend::loop::shownNow>(guest.app(), guest.cpu(), item) == 0)
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
    const bool dimmed = reg32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, cpu, cpu.esp - 4, 0)) != 0;
    const std::int32_t lastPlayer = gameSettings(guest).playMode == 1 ? 1 : 0;
    const std::uint32_t kind = gameSettings(guest).hud[0].speedometerForm;
    const reg32 second = kind == 3 || kind == 4 ? 1 : 0;
    const std::int32_t titles = s16(guest, item + kColumns);
    const std::int32_t rowHeight = s16(guest, item + kSideRowHeight);
    const std::int32_t tableWidth = titles + s16(guest, item + kColumns + 2) + (lastPlayer ? s16(guest, item + kColumns + 4) : 0);
    const std::int32_t height = 6 * rowHeight;
    const std::int32_t x = s16(guest, item + kSideX), y = s16(guest, item + kItemY);
    auto shaded = [&](std::int32_t bx, std::int32_t by, std::int32_t w, std::int32_t h, std::uint32_t c1, std::uint32_t c2,
                      std::uint32_t c3, std::uint32_t c4) {
        const std::uint32_t colours[] = { c1, c2, c3, c4 };
        fe::direct<0x4d8860, port::draw2d::shadedBox>(app, cpu, cpu.esp - 20, bx, by, w, h, colours);
    };
    fe::direct<0x4d8930, port::draw2d::outline>(app, cpu, cpu.esp - 8, reg32(x), reg32(y), reg32(tableWidth + 2),
                                                reg32(height + 2), kBlack);
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
        fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12,
                                                        guest.dword(item + kSideTitles + 8 * i + 4 * second),
                                                        reg32(x + 10), reg32(titleY - 10), kFont, kAlignLeft,
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
        reg32 record = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(
            app, cpu, 0, gameSettings(guest).localPlayer + player);
        if (record == 0)
            record = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(
                app, cpu, 1, gameSettings(guest).localPlayer + player);
        guest::direct<0x455570, frontend::menub::resultsName>(app, cpu, record, buffer);
        fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, buffer, reg32(centre), reg32(rowY),
                                                        kFont, kAlignCentre, nameColour);
        rowY += rowHeight;
        guest::direct<0x455660, frontend::menub::resultsCarLine0>(app, cpu, record, buffer);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, cpu, buffer, reg32(centre), reg32(rowY - 10), kFont, kAlignCentre, colour, reg32(room));
        guest::direct<0x4556c0, frontend::menub::resultsCarLine1>(app, cpu, record, buffer);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, cpu, buffer, reg32(centre), reg32(rowY + 5), kFont, kAlignCentre, colour, reg32(room));
        rowY += rowHeight;
        for (const reg32 field : kRacerBests)
        {
            const reg32 value = guest.dword(record + field + 4 * second);
            if (value != 0)
            {
                fe::direct<0x4cee40, frontend::menue::timeText>(app, cpu, value, buffer);
                fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, buffer, reg32(centre),
                                                                reg32(rowY), kFont, kAlignCentre, colour);
            }
            else
            {
                fe::direct<0x452190, port::text::menuTextDrawn>(app, cpu, cpu.esp - 12, kNoValue, reg32(centre),
                                                                reg32(rowY), kFont, kAlignCentre, kDimmedText);
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
reg32 loadMenuArt(App* app, CPU& cpu, Guest& guest, reg32 art)
{
    clearMenuArt(guest);
    if (art == 0)
        return 0;
    const reg32 glowTexture = guest.frame();
    guest.setDword(kChromeBufferSize, 4);
    guest.setDword(kChromeBuffer, *fe::direct<0x4d5f40, graphics::artTexture>(app, cpu, 0x100, kChromeBufferSize, 0));
    const reg32 chrome = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, art, kArtChrome);
    if (chrome != 0 && isSquareOf(guest, chrome, 0x100))
    {
        const std::uint32_t format = (guest.byte(kDisplayFlags) & kNoTranslucency) ? 4 : 0;
        fe::direct<0x4d6070, port::shapes::textureOf>(app, cpu, cpu.esp - 4, kChromeTexture, chrome, 0x100, format);
    }
    else
    {
        guest.setDword(kChromeTexture, 0);
    }
    const reg32 glow = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, art, kArtGlow);
    if (glow != 0 && isSquareOf(guest, glow, 0x20))
    {
        fe::direct<0x4d6070, port::shapes::textureOf>(app, cpu, cpu.esp - 4, glowTexture, glow, 0x20, 0);
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
constexpr std::int32_t kTitleTicks = 0x180;
constexpr reg32 kInputMode = 0x56438c;
constexpr reg32 kInputFlags = 0x564390;
constexpr reg32 kSavedInputMode = 0x679008;
constexpr reg32 kSavedInputFlags = 0x679004;
constexpr reg32 kAfterRace = 0x8b7630;
constexpr reg32 kLoadFlag = 0x55d000;

/* sub_472f20: the title's picture, in the language's. */
reg32 drawTitle(App* app, CPU& cpu, Guest& guest)
{
    const reg32 name = guest.frame();                 // char[0x12c]
    const reg32 codes = guest.frame() + 0x12c;
    for (reg32 i = 0; i < 7; ++i)
        guest.setByte(codes + i, guest.byte(kLanguageCodes + i));
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, cpu, cpu.esp - 4,
                                                     port::Args{});
    const std::uint32_t code = guest.byte(codes + guest.dword(kLanguageIndex));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kFormatTitle, guest::Words{kArtFolder, code});
        const reg32 file = fe::direct<kReadFile, frontend::draw::loadQfsFlags>(app, cpu, name, kLoadFlags);
    if (file == 0)
        return 0;
    const reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, kBack);
        guest.setDword(kTitleTexture,
                   reg32(*fe::direct<kShapeTextures, port::shapes::artTexturesOf>(app, cpu, cpu.esp - 4, shape, 0, 0)));
    fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, cpu);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, cpu, cpu.esp - 4, guest.dword(kTitleTexture), 0, 0);
    fe::direct<0x4d9140, graphics::flipAndWait>(app, cpu);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, file);
}

/* sub_472fc0: the title on the screen until a key, or until its time is up. */
reg32 waitOnTitle(App* app, CPU& cpu, Guest& guest)
{
    if (gameSettings(guest).installIsRemote == 0)
        guest::direct<0x472f20, frontend::menuc::titleDraw>(app, cpu);
    fe::direct<0x4f0650, frontend::wata::timerSinceLast>(app, cpu);
    fe::direct<0x451940, frontend::connect::flushKeys>(app, cpu, 0);
    std::int32_t elapsed = 0;
    for (;;)
    {
        const reg32 key = reg32(*fe::direct<0x451960, port::text::menuKeyOf>(app, cpu, cpu.esp - 4, 1));
        if (key != 0 || elapsed >= kTitleTicks)
            return key;
        fe::direct<0x4c9860, port::descriptionServed>(app, cpu, cpu.esp - 4);
        const reg32 ticks = fe::direct<0x4f0650, frontend::wata::timerSinceLast>(app, cpu);
        elapsed += std::int32_t(ticks);
        if (elapsed > kTitleTicks)
            return ticks;
    }
}

/* sub_473020: into the menus: the input's mode kept and the menus', the sound
 * and the screens taken up. */
reg32 enterMenus(App* app, CPU& cpu, Guest& guest)
{
    fe::direct<0x431bc0, collide::screenStates>(app, cpu);
    fe::direct<0x431900, graphics::sendState>(app, cpu, 3, 0);
    fe::direct<0x4d5cc0, graphics::drawingBegun>(app, cpu, 1, 0);
    const reg32 mode = guest.dword(kInputMode);
    guest.setDword(kSavedInputMode, mode);
    guest.setDword(kInputMode, 0x10);
    fe::direct<0x4f0230, frontend::eab::inputStateClose>(app, cpu);
    const reg32 flags = guest.dword(kInputFlags);
    guest.setDword(kSavedInputFlags, flags);
    guest.setByte(kInputFlags, std::uint8_t(flags | 4));
    fe::direct<0x4136e0, port::raceBankLoadOf>(app, cpu, cpu.esp - 4);   // the game's code: the flags in eax, unread
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, cpu, cpu.esp - 4,
                                                     port::Args{});
    return fe::direct<0x49c140, frontend::menud::loadPointerArt>(app, cpu);
}

/* sub_473080: out of them, all of it undone. */
reg32 leaveMenus(App* app, CPU& cpu, Guest& guest)
{
    // By address: the stop's ebx is the one the function leaves, and the 0 in edx the removal is handed,
    // which its core does not take, is part of the call this group's check sees.
    fe::direct<0x4109d0, port::musicStopped>(app, cpu, cpu.esp - 4, 0);
    fe::direct<0x410b20, port::banksRemoveOf>(app, cpu, cpu.esp - 4);
    guest.setDword(0x55b034, 0);
    fe::direct<0x49c2a0, frontend::menud::freePointerArt>(app, cpu, 0);  // eax: its result is not read
    guest.setDword(kInputMode, guest.dword(kSavedInputMode));
    const reg32 flags = guest.dword(kSavedInputFlags);
    guest.setDword(kInputFlags, flags);
    fe::direct<0x472360, port::fields::customColoursRefreshed>(app, cpu, cpu.esp - 4, flags);
    fe::direct<0x472820, load::configSaved>(app, cpu);
    fe::direct<0x4d5c40, port::shapes::texturesLetGo>(app, cpu, cpu.esp - 4);
    return fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, cpu);
}

/* sub_4732b0: the screens after a race: the voices its kind has, the menu loop
 * on postgame, its answer. */
reg32 runPostgame(App* app, CPU& cpu, Guest& guest)
{
    constexpr reg32 kPostgame = 0x53b054;
    guest::direct<0x473020, frontend::menuc::menusEnter>(app, cpu);
    gameSettings(guest).replayMode = 0;
    guest.setDword(kAfterRace, 1);
    switch (gameSettings(guest).raceType)
    {
    case 1:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, cpu, cpu.esp - 4,
                                                        gameSettings(guest).seriesVariant == 0 ? 2u : 3u);
        break;
    case 2:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, cpu, cpu.esp - 4, 1);
        break;
    default:
        fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, cpu, cpu.esp - 4, 4);
        break;
    }
    fe::direct<0x4c9890, port::raceVoiceBankLoaded>(app, cpu, cpu.esp - 4, 0);
    const reg32 answer = reg32(*fe::direct<0x44b230, port::loop::menuLoopOf>(app, cpu, cpu.esp - 4, kPostgame, 0));
    fe::direct<0x4c9a00, port::raceVoiceBanksFreed>(app, cpu, cpu.esp - 4);
    guest::direct<0x44e7d0, frontend::menub::awardsClear>(app, cpu);
    guest.setDword(kAfterRace, 0);
    guest::direct<0x473080, frontend::menuc::menusLeave>(app, cpu);
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

x86::reg32 frontend::menuc::resultsTablePass(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
}

x86::reg32 frontend::resultsTablePass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e0b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTablePass(app, cpu, item); });
}
x86::reg32 frontend::menuc::standingsTablePass(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
}

x86::reg32 frontend::standingsTablePass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f0a0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTablePass(app, cpu, item); });
}
x86::reg32 frontend::menuc::roundsTablePass(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
}

x86::reg32 frontend::roundsTablePass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46fee0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTablePass(app, cpu, item); });
}
x86::reg32 frontend::menuc::resultsTableHidden(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
    return reg32(0);
}

x86::reg32 frontend::resultsTableHidden(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e640", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTableHidden(app, cpu, item); });
}
x86::reg32 frontend::menuc::standingsTableHidden(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
    return reg32(0);
}

x86::reg32 frontend::standingsTableHidden(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f4c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTableHidden(app, cpu, item); });
}
x86::reg32 frontend::menuc::roundsTableHidden(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    fe::direct<kItemShownNow, frontend::loop::shownNow>(app, cpu, item);
    return reg32(0);
}

x86::reg32 frontend::roundsTableHidden(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_470340", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTableHidden(app, cpu, item); });
}
x86::reg32 frontend::menuc::resultsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x174);
    drawResults(app, cpu, guest, item, pointing != 0);
    return reg32(0);
}

x86::reg32 frontend::resultsTableDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46d3c0", 0, 0, nullptr, 0, kResultsSaves };
    const reg32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTableDraw(app, cpu, item, pointing); });
}
x86::reg32 frontend::menuc::standingsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x270);
    drawStandings(app, cpu, guest, item, pointing != 0);
    return reg32(0);
}

x86::reg32 frontend::standingsTableDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e690", 0, 0, nullptr, 0, kResultsSaves };
    const reg32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTableDraw(app, cpu, item, pointing); });
}
x86::reg32 frontend::menuc::roundsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x16c);
    drawRounds(app, cpu, guest, item, pointing != 0);
    return reg32(0);
}

x86::reg32 frontend::roundsTableDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f510", 0, 0, nullptr, 0, kResultsSaves };
    const reg32 item = args.eax, pointing = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTableDraw(app, cpu, item, pointing); });
}
x86::reg32 frontend::menuc::resultsPopupColumn(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    return resultsPopupUnderPointer(app, cpu, guest, item);
}

x86::reg32 frontend::resultsPopupColumn(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46cdf0", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsPopupColumn(app, cpu, item); });
}
x86::reg32 frontend::menuc::resultsLapTimesPopup(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x114);
    return drawLapTimesPopup(app, cpu, guest, item);
}

x86::reg32 frontend::resultsLapTimesPopup(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46d180", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsLapTimesPopup(app, cpu, item); });
}
x86::reg32 frontend::menuc::resultsTableShown(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 4);
    return resultsOnShow(app, cpu, guest, item);
}

x86::reg32 frontend::resultsTableShown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46df90", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTableShown(app, cpu, item); });
}
x86::reg32 frontend::menuc::standingsTableShown(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 4);
    return standingsOnShow(app, cpu, guest, item);
}

x86::reg32 frontend::standingsTableShown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46ef90", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTableShown(app, cpu, item); });
}
x86::reg32 frontend::menuc::resultsTableLayout(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return layoutResults(app, cpu, guest, item);
}

x86::reg32 frontend::resultsTableLayout(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e0d0", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTableLayout(app, cpu, item); });
}
x86::reg32 frontend::menuc::loadMenuArtSlots(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 4);
    return loadMenuArt(app, cpu, guest, item);
}

x86::reg32 frontend::loadMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_465d80", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::loadMenuArtSlots(app, cpu, item); });
}
x86::reg32 frontend::menuc::titleDraw(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x134);
    return drawTitle(app, cpu, guest);
}

x86::reg32 frontend::titleDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_472f20", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::titleDraw(app, cpu); });
}
x86::reg32 frontend::menuc::titleWait(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return waitOnTitle(app, cpu, guest);
}

x86::reg32 frontend::titleWait(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_472fc0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::titleWait(app, cpu); });
}
x86::reg32 frontend::menuc::menusEnter(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return enterMenus(app, cpu, guest);
}

x86::reg32 frontend::menusEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473020", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::menusEnter(app, cpu); });
}
x86::reg32 frontend::menuc::menusLeave(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return leaveMenus(app, cpu, guest);
}

x86::reg32 frontend::menusLeave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473080", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::menusLeave(app, cpu); });
}
x86::reg32 frontend::menuc::postgameRun(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    return runPostgame(app, cpu, guest);
}

x86::reg32 frontend::postgameRun(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4732b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::postgameRun(app, cpu); });
}

/* sub_45cb50: the HUD editor's element, hidden: nothing to do. */
void frontend::menuc::hudElementHidden(win32::WinApplication* , x86::CPU& )
{
}

/* sub_470390: the side by side table has nothing to do on a pass. */
void frontend::menuc::headToHeadPass(win32::WinApplication* , x86::CPU& )
{
}

/* sub_473010: a comparison with the intro's flag, for no one. */
void frontend::menuc::skipIntroCompare(win32::WinApplication* , x86::CPU& )
{
}

/* sub_460760: the buttons' pulse starts again from its brightest, as the
 * selected item changes. */
void frontend::menuc::resetButtonPulse(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x559fdc, 0xff);
}

/* sub_465d60: no chrome, no glow. */
void frontend::menuc::clearMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    clearMenuArt(guest);
}

x86::reg32 frontend::hudElementHidden(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45cb50", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        menuc::hudElementHidden(app, cpu);
        return args.eax;
    });
}
x86::reg32 frontend::headToHeadPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_470390", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        menuc::headToHeadPass(app, cpu);
        return args.eax;
    });
}
x86::reg32 frontend::skipIntroCompare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473010", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        menuc::skipIntroCompare(app, cpu);
        return args.eax;
    });
}
x86::reg32 frontend::resetButtonPulse(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_460760", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        menuc::resetButtonPulse(app, cpu);
        return args.eax;
    });
}
x86::reg32 frontend::clearMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_465d60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        menuc::clearMenuArtSlots(app, cpu);
        return args.eax;
    });
}

/* sub_4654c0: the two values on the stack kept, in the slots at 0x678954.  What
 * is left in eax is the first. */
x86::reg32 frontend::menuc::setSliderComponents(App* app, CPU& cpu, reg32 first, reg32 second)
{
    Guest guest(app, cpu);
    guest.setDword(0x678958, second);
    guest.setDword(0x678954, first);
    return first;
}

x86::reg32 frontend::setSliderComponents(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4654c0", 8, 0, nullptr, 0, 0 };
    const reg32 first = app->getMemory<reg32>(cpu.esp + 4), second = app->getMemory<reg32>(cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::setSliderComponents(app, cpu, first, second); });
}

x86::reg32 frontend::menuc::drawColumnSortMark(App* app, CPU& cpu, reg32 topLeft, reg32 widthHeight, reg32 shape)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 36);
    return drawSortMark(app, cpu, guest, topLeft, widthHeight, shape);
}

x86::reg32 frontend::drawColumnSortMark(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46cce0", 12, 0, nullptr, 0 };
    const reg32 arguments = cpu.esp + 4;
    const reg32 topLeft = app->getMemory<reg32>(arguments), widthHeight = app->getMemory<reg32>(arguments + 4);
    const reg32 shape = app->getMemory<reg32>(arguments + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return menuc::drawColumnSortMark(app, cpu, topLeft, widthHeight, shape);
    });
}

x86::reg32 frontend::menuc::resultsSortMark(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return drawTableSortMark(app, cpu, guest, item, 0x48, 0x86, 0x80);
}

x86::reg32 frontend::resultsSortMark(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46cdc0", 0, 0, nullptr, 0, kSortMarkSaves };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsSortMark(app, cpu, item); });
}

x86::reg32 frontend::menuc::standingsSortMark(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return drawTableSortMark(app, cpu, guest, item, 0x46, 0x7c, 0x76);
}

x86::reg32 frontend::standingsSortMark(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e660", 0, 0, nullptr, 0, kSortMarkSaves };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsSortMark(app, cpu, item); });
}

x86::reg32 frontend::menuc::roundsSortMark(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return drawTableSortMark(app, cpu, guest, item, 0x46, 0x7c, 0x76);
}

x86::reg32 frontend::roundsSortMark(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f4e0", 0, 0, nullptr, 0, kSortMarkSaves };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsSortMark(app, cpu, item); });
}

x86::reg32 frontend::menuc::resultsTableKey(App* app, CPU& cpu, reg32 item, reg32, reg32 key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0xc);
    return resultsKey(app, cpu, guest, item, std::uint16_t(key));
}

x86::reg32 frontend::resultsTableKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46e270", 0, 0, nullptr, 0, kKeySaves };
    const reg32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::resultsTableKey(app, cpu, item, menu, key); });
}

x86::reg32 frontend::menuc::standingsTableLayout(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return layoutStandings(app, cpu, guest, item, kStandingsType, kResultsCount);
}

x86::reg32 frontend::standingsTableLayout(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f0c0", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTableLayout(app, cpu, item); });
}

x86::reg32 frontend::menuc::standingsTableKey(App* app, CPU& cpu, reg32 item, reg32, reg32 key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0xc);
    return standingsKey(app, cpu, guest, item, std::uint16_t(key), kStandingsType, kResultsCount);
}

x86::reg32 frontend::standingsTableKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46f200", 0, 0, nullptr, 0, kKeySaves };
    const reg32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::standingsTableKey(app, cpu, item, menu, key); });
}

x86::reg32 frontend::menuc::roundsTableShown(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    return roundsOnShow(app, cpu, guest, item);
}

x86::reg32 frontend::roundsTableShown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46fe30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTableShown(app, cpu, item); });
}

x86::reg32 frontend::menuc::roundsTableLayout(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return layoutStandings(app, cpu, guest, item, kRoundsType, kRoundsCount);
}

x86::reg32 frontend::roundsTableLayout(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_46ff00", 0, 0, nullptr, 0 };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTableLayout(app, cpu, item); });
}

x86::reg32 frontend::menuc::roundsTableKey(App* app, CPU& cpu, reg32 item, reg32, reg32 key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0xc);
    return standingsKey(app, cpu, guest, item, std::uint16_t(key), kRoundsType, kRoundsCount);
}

x86::reg32 frontend::roundsTableKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_470040", 0, 0, nullptr, 0, kKeySaves };
    const reg32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::roundsTableKey(app, cpu, item, menu, key); });
}

x86::reg32 frontend::menuc::headToHeadShown(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return sideBySideOnShow(guest, item);
}

x86::reg32 frontend::headToHeadShown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_470360", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::headToHeadShown(app, cpu, item); });
}

x86::reg32 frontend::menuc::headToHeadDraw(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x154);
    drawSideBySide(app, cpu, guest, item);
    return 0;
}

x86::reg32 frontend::headToHeadDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4703a0", 0, 0, nullptr, 0, kResultsSaves };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::headToHeadDraw(app, cpu, item); });
}

/* sub_477c00: the flag at 0x55d000 down. */
x86::reg32 frontend::menuc::clearLoadFlag(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kLoadFlag, 0);
    return 0;
}

x86::reg32 frontend::clearLoadFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_477c00", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menuc::clearLoadFlag(app, cpu); });
}


}
