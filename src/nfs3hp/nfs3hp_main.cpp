#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <lib/registry.h>
#include <lib/gamepad.h>
#include <nfs3hp.h>
#include <winapi/glide2x.h>
#include <SDL3/SDL.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#include <jni.h>
#endif

namespace nfs3hp
{
/* NFS_CAR_DETAIL_FULL, read once: unset or anything but "0" means on.  The
 * generated code patched by tools/apply_car_detail.py asks it too. */
bool fullCarDetail()
{
    static const bool full = []() {
        const char* value = SDL_getenv("NFS_CAR_DETAIL_FULL");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return full;
}

/* Whether the player is driving.  A race is one call of the game's own -- set
 * up by sub_4a3400, taken down again by sub_4a35d0, each called from one place
 * only -- and tools/apply_race_state.py marks both, so this follows it exactly
 * rather than guessing from the screen or from what the car is doing. */
static bool s_raceRunning = false;

void raceRunning(bool running)
{
    s_raceRunning = running;
    /* A load cut short leaves the race without finishing its setup, and so
     * without the end of the loading screen (loadingScreenFit). */
    if (!running)
        win32::glide2x::fitFourThree(false);
}

/* The loading screen at 4:3 in the middle of a wide screen, as the Modern Patch
 * shows it (tools/apply_loading_screen.py): from the start of the loading
 * picture, the whole picture black and everything drawn squeezed into the 4:3
 * rectangle in its middle; once the race is loaded, the whole screen again. */
void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on)
{
    if (on)
        win32::glide2x::clearPicture(app, cpu);
    win32::glide2x::fitFourThree(on);
}

bool raceIsRunning()
{
    return s_raceRunning;
}

/* View Distance at Full: the player's choice in the Graphics menu, [0x6fbc28],
 * Full to Close as 0 to 3.  At Full the game already draws the track at full
 * detail out to its far distance; tools/apply_track_detail.py has the two below
 * take that distance to 500 wherever the game held it shorter. */
static bool viewDistanceFull(win32::WinApplication* app)
{
    return app->getMemory<x86::reg32>(0x6fbc28) == 0;
}

/* Which table of multipliers sub_41d620 scales the distances by: 0x41a8bc, whose
 * widest row leaves the far distance whole, or 0x41a8b0, whose widest row keeps
 * three quarters of it.  `reduced` is the game's own choice, [0x6fd4c8]; at Full
 * the distance is whole. */
x86::reg32 viewDistanceReduced(win32::WinApplication* app, x86::reg32 reduced)
{
    return viewDistanceFull(app) ? 0u : reduced;
}

/* The far distance of each split-screen view, in 16.16: the game's 440, or at
 * Full the 500 a single view has. */
x86::reg32 splitFarDistance(win32::WinApplication* app, x86::reg32 distance)
{
    return viewDistanceFull(app) ? x86::reg32(500) << 16 : distance;
}

/* Wheel spin, per car.  The game turns a car's wheels by the frames elapsed
 * since one stamp shared by every car and every view (0x55fe28), written as
 * each view pass ends, so a car first drawn in a later pass of the same frame
 * -- the second split-screen view, the rear-view mirror -- saw no time pass and
 * its wheels stood still.  tools/apply_car_detail.py routes the three reads of
 * that stamp here: sub_4b9140 and sub_4b9280 for the two wheel angles, and
 * sub_4b90e0, two ways, for the body.  Each keeps its own stamp per car, so a
 * car still advances once per frame, whichever view draws it first. */
x86::reg32 carWheelStamp(win32::WinApplication* app, x86::reg32 car, x86::reg32 which)
{
    static std::unordered_map<x86::reg32, std::array<x86::reg32, 4>> s_stamps;
    const x86::reg32 frame = app->getMemory<x86::reg32>(0x7d3684);
    auto entry = s_stamps.try_emplace(car);
    x86::reg32& stamp = entry.first->second[which & 3];
    x86::reg32 last = entry.second ? app->getMemory<x86::reg32>(0x55fe28) : stamp;
    /* A stamp from an earlier race -- the frame count starts over and car
     * records are reused -- would freeze the wheels until the count caught up,
     * and one from long ago would spin them through hundreds of turns at once. */
    if (last > frame || frame - last > 4)
        last = frame - 1;
    stamp = frame;
    return last;
}

/* Screen sizes the Graphics menu may list.  sub_4bed90 passes the menu only
 * driver modes whose width is exactly 4/3 of their height, and
 * tools/apply_widescreen.py has it ask here too: an exact 16:9 is taken as
 * well, which is the 1280x720 main() adds to the Voodoo2 driver's table.  The
 * driver's own 856x480 is not quite 16:9 and stays unlisted. */
bool widescreenMode(x86::reg32 width, x86::reg32 height)
{
    return width * 9 == height * 16;
}

/* A camera's horizontal half field of view, in whole degrees, for the screen the
 * race is drawn on (tools/apply_widescreen.py, in sub_4dbce0).  The game makes
 * each view's vertical angle 13/16 of its horizontal one, proportions that hold
 * on a 4:3 screen only, so on anything wider the picture came out stretched
 * sideways.  The vertical angle stays what the game makes it; the horizontal one
 * opens up by as much as the screen is wider than 4:3 -- Hor+ -- for every view
 * alike: the chase and in-car cameras, the mirror, both halves of split screen.
 * 4:3 and narrower screens keep the game's own angle to the degree.  The screen
 * is the mode the race switched to (sub_4bef50 copies it to 0x7cdae0). */
x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdae0);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdae4);
    if (!height || width * 3 <= height * 4 || x86::sreg32(half) <= 0 || half >= 90)
        return half;
    const double wider = double(width) * 3.0 / (double(height) * 4.0);
    const double angle = SDL_atan(SDL_tan(double(half) * SDL_PI_D / 180.0) * wider) * 180.0 / SDL_PI_D;
    return x86::reg32(SDL_lround(angle));
}

/* The rectangle the in-car cabin is drawn in (tools/apply_cabin_fit.py): field
 * 0 to 3 of x, y, width and height.  The game takes the view's own rectangle
 * ([0x7cdb98]), and the cabin picture and the wheel over it are drawn for 4:3, so
 * on a wider screen they stretched with it.  Here the cabin keeps the view's
 * width and takes the height that width has on a 4:3 screen, centred on the view
 * from top to bottom: the dashboard and the wheel keep their proportions and fill
 * the view from side to side, and what no longer fits runs off the top and the
 * bottom alike, where the game's clip window cuts it -- a little of the roof, a
 * little of the dashboard's lower edge.  Standing on the bottom edge instead, the
 * whole of the extra height went upwards and the wheel rose into the road.
 * Centred, a point halfway down the picture stays halfway down the screen, as
 * on 4:3.  How much taller comes from the screen rather than the view, so a
 * split-screen half, which the game squashes to half height on a 4:3 screen too,
 * keeps that look and loses the same share.  A screen no wider than 4:3 keeps the
 * game's own rectangle. */
x86::reg32 cabinRect(win32::WinApplication* app, x86::reg32 field)
{
    const x86::sreg32 x = x86::sreg32(app->getMemory<x86::reg32>(0x7cdb98));
    const x86::sreg32 y = x86::sreg32(app->getMemory<x86::reg32>(0x7cdb9c));
    const x86::sreg32 width = x86::sreg32(app->getMemory<x86::reg32>(0x7cdba0));
    const x86::sreg32 height = x86::sreg32(app->getMemory<x86::reg32>(0x7cdba4));
    const x86::reg32 screenWidth = app->getMemory<x86::reg32>(0x7cdae0);
    const x86::reg32 screenHeight = app->getMemory<x86::reg32>(0x7cdae4);
    x86::sreg32 fitHeight = height;
    if (height > 0 && screenHeight && screenWidth * 3 > screenHeight * 4)
        fitHeight = x86::sreg32(SDL_lround(double(height) * double(screenWidth) * 3.0 / (double(screenHeight) * 4.0)));
    switch (field)
    {
    case 0: return x86::reg32(x);
    case 1: return x86::reg32(y + (height - fitHeight) / 2);
    case 2: return x86::reg32(width);
    default: return x86::reg32(fitHeight);
    }
}

/* The size the HUD works its sizes out from (tools/apply_hud_scale.py).  The
 * game takes them from the screen it draws on -- the frame around each element
 * and its insets, the gap beside it, the text -- as a fraction of its width,
 * which on a 4:3 screen is the same as a fraction of its height.  Any other
 * shape is not, so what those reads get instead is the 4:3 screen that fits in
 * this one: on a wider screen the width that height would have, on a taller one
 * the height that width would have.  A border then stays the thickness it was
 * drawn with and text the size.  Where an element sits is a separate question,
 * answered in widescreenHudRect; these only say how big its parts are. */
x86::reg32 hudReferenceWidth(win32::WinApplication* app)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdac8);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdacc);
    if (!height || width * 3 <= height * 4)
        return width;
    return (height * 4) / 3;
}

x86::reg32 hudReferenceHeight(win32::WinApplication* app)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdac8);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdacc);
    if (!width || height * 4 <= width * 3)
        return height;
    return (width * 3) / 4;
}

/* How a HUD element is drawn on the screen in use against its layout: whether
 * its left and right edges move or its top and bottom, and by what factor about
 * their centre.  False for an element drawn as laid out -- all of them on a
 * screen no wider than 4:3.  See widescreenHudRect. */
static bool widescreenHudScale(win32::WinApplication* app, x86::reg32 element, bool& across, double& scale)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdae0);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdae4);
    if (!height || width * 3 <= height * 4)
        return false;
    const double narrow = double(height) * 4.0 / (double(width) * 3.0);
    if (element == 1 || element == 3 || element == 6 || element == 8)
    {
        across = true;
        scale = narrow;
        return true;
    }
    if (element == 21)
    {
        across = false;
        scale = 1.0 / narrow;
        return true;
    }
    return false;
}

/* HUD elements the game draws as pictures keep their shape on a wide screen
 * (tools/apply_widescreen.py, at the end of sub_480910).  The game lays every
 * element out as a fraction of the screen -- the DashHud .POS files, kept in
 * config.dat -- and sub_480910 turns that into pixels by the screen's width and
 * height, so on 16:9 each came out a third wider than it was drawn.  The analog
 * speedometer and tachometer, the tutorial icon and the replay bar (elements 1,
 * 3, 6 and 8 of the 23 a .POS file lists) are narrowed about their centre by as
 * much as the screen is wider than 4:3, and the spike belt indicator (21) is
 * made taller by as much instead: the elements, and the ways, the Modern Patch
 * settled on.  Everything else keeps its layout, still a fraction of the screen.
 * The layout itself stays as laid out, so a 4:3 screen finds the HUD as it was;
 * the HUD editor is told the difference (hudLayoutDesigned, hudLayoutsDrawn).
 * `slot` is the element's offset in the table of pixel rectangles at 0x749758:
 * 23 per player, four ints each -- x0, y0, x1, y1.  How much wider is taken from
 * the screen mode itself, not from the size sub_480910 was handed, which in
 * split screen is one player's half. */
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot)
{
    bool across;
    double scale;
    if (!widescreenHudScale(app, (slot % (23 * 16)) / 16, across, scale))
        return;
    const x86::reg32 rect = 0x749758 + slot;
    const x86::reg32 first = across ? 0 : 4, second = across ? 8 : 12;  // x0 and x1, or y0 and y1
    const double a = double(x86::sreg32(app->getMemory<x86::reg32>(rect + first)));
    const double b = double(x86::sreg32(app->getMemory<x86::reg32>(rect + second)));
    const double centre = (a + b) / 2.0, half = (b - a) * scale / 2.0;
    app->getMemory<x86::reg32>(rect + first) = x86::reg32(x86::sreg32(SDL_lround(centre - half)));
    app->getMemory<x86::reg32>(rect + second) = x86::reg32(x86::sreg32(SDL_lround(centre + half)));
}

/* The same change on an element's layout, the fractions of the screen the game
 * keeps it at: four floats, top, left, bottom and right (tools/apply_hud_editor.py).
 * By `scale` for as it is drawn, by its inverse for as it is laid out. */
static void scaleHudLayout(win32::WinApplication* app, x86::reg32 rect, bool across, double scale)
{
    const x86::reg32 first = across ? 4 : 0, second = across ? 12 : 8;  // left and right, or top and bottom
    const double a = app->getMemory<float>(rect + first), b = app->getMemory<float>(rect + second);
    const double centre = (a + b) / 2.0, half = (b - a) * scale / 2.0;
    app->getMemory<float>(rect + first) = float(centre - half);
    app->getMemory<float>(rect + second) = float(centre + half);
}

/* The HUD editor writing an element's place into its layout, from the rectangle
 * it moves -- the one drawn: put back as laid out, or every edit would narrow a
 * gauge that drawing then narrows again.  `rect` is the layout's four floats. */
void hudLayoutDesigned(win32::WinApplication* app, x86::reg32 rect, x86::reg32 element)
{
    bool across;
    double scale;
    if (widescreenHudScale(app, element, across, scale))
        scaleHudLayout(app, rect, across, 1.0 / scale);
}

/* The standings table's rectangle made the panel it is drawn as
 * (tools/apply_hud_editor.py, at the end of sub_481c50).  The table's layout is
 * a box half the screen wide and a third high, and sub_481c50 draws the table as
 * a panel only as big as its rows: at the box's left edge, as wide as the widest
 * row ([0x7925c8], or [0x7925cc] for the table's other layout), and from the
 * box's bottom up or its top down, as the table keeps to one or the other -- top
 * and bottom it leaves at [0x724780] and [0x724788] per player.  Nothing else
 * reads the rest of the box, but the HUD editor framed the whole of it and
 * measured every other element against it.  Here the pixel rectangle becomes
 * the panel: the left edge and the edge the panel keeps to do not move, so the
 * table is drawn where it was, and the editor frames what is on the screen.
 * The table is element 13 of the 23 per player at 0x749758, or 20 in its other
 * layout. */
void hudTablePanel(win32::WinApplication* app, x86::reg32 player, bool otherLayout)
{
    const x86::sreg32 width = x86::sreg32(app->getMemory<x86::reg32>(otherLayout ? 0x7925cc : 0x7925c8));
    const x86::sreg32 top = x86::sreg32(app->getMemory<x86::reg32>(0x724780 + player * 4));
    const x86::sreg32 bottom = x86::sreg32(app->getMemory<x86::reg32>(0x724788 + player * 4));
    if (width <= 0 || bottom <= top || player > 1)
        return;
    const x86::reg32 rect = 0x749758 + player * 23 * 16 + (otherLayout ? 20 : 13) * 16;
    const x86::sreg32 left = x86::sreg32(app->getMemory<x86::reg32>(rect));
    app->getMemory<x86::reg32>(rect + 4) = x86::reg32(top);
    app->getMemory<x86::reg32>(rect + 8) = x86::reg32(left + width);
    app->getMemory<x86::reg32>(rect + 12) = x86::reg32(bottom);
}

/* Buffer swaps so far, counted in onSwap: how old a measurement is. */
static x86::reg32 s_swaps = 0;

/* Where each HUD element really is on the screen, as the HUD editor draws it
 * (tools/apply_hud_editor.py, around the call sub_45bcb0 makes to sub_45ab30 for
 * every element, every frame).  An element's layout is a box, and what the game
 * draws in it is often far smaller -- a line of text at one side of it, a panel
 * at one corner -- yet the editor framed the box and measured every other element
 * against it, so an element blinked red over empty space.  What is drawn is
 * measured here instead, from every quad the element queues meanwhile
 * (hudQueueQuad): the game's 2D is queued, text glyph by glyph, and drawn
 * later in the frame, all of it together, so what reaches Glide during an
 * element's turn is somebody else's.  Kept with the element's rectangle at the
 * time, so the same measurement still holds once the element has moved: an
 * element draws the same thing wherever it stands.  An element that queues
 * nothing, or something far bigger than its box -- a strip across the whole
 * screen is not what the player means by the element -- keeps its box, as the
 * game had it. */
struct HudDrawn
{
    x86::reg32  swap = 0;       // s_swaps when measured; 0 is never
    x86::sreg32 x0, y0, x1, y1; // what was drawn, pixels
    x86::sreg32 left, top, right, bottom;  // the element's rectangle then
};
static HudDrawn s_hudDrawn[2][23];

static bool hudDrawnFresh(const HudDrawn& drawn)
{
    return drawn.swap && s_swaps - drawn.swap <= 2;
}

/* The quads queued while an element draws: none while nobody measures. */
static bool s_hudMeasuring = false;
static float s_hudX0, s_hudY0, s_hudX1, s_hudY1;

/* One quad into one of the game's 2D queues (tools/apply_hud_editor.py, at the
 * start of sub_49bd30): four vertex numbers into the queue's vertices, 32 bytes
 * each from list+0x10, x and y the first two floats. */
void hudQueueQuad(win32::WinApplication* app, x86::reg32 list, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d)
{
    if (!s_hudMeasuring)
        return;
    for (x86::reg32 vertex : { a, b, c, d })
    {
        if (vertex >= 0x1000)
            continue;
        const float x = app->getMemory<float>(list + 0x10 + vertex * 32);
        const float y = app->getMemory<float>(list + 0x10 + vertex * 32 + 4);
        s_hudX0 = SDL_min(s_hudX0, x);
        s_hudY0 = SDL_min(s_hudY0, y);
        s_hudX1 = SDL_max(s_hudX1, x);
        s_hudY1 = SDL_max(s_hudY1, y);
    }
}

void hudEditorDraw(win32::WinApplication* app, x86::reg32 object, bool begin)
{
    if (begin)
    {
        s_hudMeasuring = true;
        s_hudX0 = s_hudY0 = 1e9f;
        s_hudX1 = s_hudY1 = -1e9f;
        return;
    }
    s_hudMeasuring = false;
    const x86::reg32 index = app->getMemory<x86::reg32>(object + 0x3c);
    if (index >= 46)
        return;
    HudDrawn& drawn = s_hudDrawn[index / 23][index % 23];
    const x86::sreg32 left = x86::sreg16(app->getMemory<x86::reg16>(object + 6));
    const x86::sreg32 top = x86::sreg16(app->getMemory<x86::reg16>(object + 8));
    const x86::sreg32 right = x86::sreg32(app->getMemory<x86::reg32>(object + 0x60));
    const x86::sreg32 bottom = x86::sreg32(app->getMemory<x86::reg32>(object + 0x64));
    const int x0 = int(SDL_floor(s_hudX0)), y0 = int(SDL_floor(s_hudY0));
    const int x1 = int(SDL_ceil(s_hudX1)), y1 = int(SDL_ceil(s_hudY1));
    if (x1 <= x0 || y1 <= y0 || x1 - x0 > 2 * (right - left) + 64 || y1 - y0 > 2 * (bottom - top) + 64)
    {
        drawn.swap = 0;
        return;
    }
    /* Said once for each new shape, not for every step of a drag. */
    if (!drawn.swap || x0 - left != drawn.x0 - drawn.left || y0 - top != drawn.y0 - drawn.top
        || x1 - right != drawn.x1 - drawn.right || y1 - bottom != drawn.y1 - drawn.bottom)
        SDL_Log("[HUDEDIT] element %u drawn at %d,%d-%d,%d in its box %d,%d-%d,%d",
                unsigned(index % 23), x0, y0, x1, y1, int(left), int(top), int(right), int(bottom));
    drawn.swap = s_swaps ? s_swaps : 1;
    drawn.x0 = x0;
    drawn.y0 = y0;
    drawn.x1 = x1;
    drawn.y1 = y1;
    drawn.left = left;
    drawn.top = top;
    drawn.right = right;
    drawn.bottom = bottom;
}

/* How far an element's box reaches past what it draws, on one side -- 0 left,
 * 1 right, 2 top, 3 bottom -- from its last measurement: what the box may lose
 * off the edge of the screen while the drawing stays on it.  The HUD editor
 * keeps the box itself on the screen while an element is dragged
 * (tools/apply_hud_editor.py, sub_45a120's four clamps), so a line of text in
 * the middle of a box half the screen wide could never come near the edge.
 * Negative where the drawing spills out of the box: it stays on the screen
 * then.  0 for an element not measured, which leaves the game's own rule. */
x86::sreg32 hudDragSlack(win32::WinApplication* app, x86::reg32 object, x86::reg32 side)
{
    const x86::reg32 index = app->getMemory<x86::reg32>(object + 0x3c);
    if (index >= 46 || !hudDrawnFresh(s_hudDrawn[index / 23][index % 23]))
        return 0;
    const HudDrawn& measured = s_hudDrawn[index / 23][index % 23];
    switch (side)
    {
    case 0: return measured.x0 - measured.left;
    case 1: return measured.right - measured.x1;
    case 2: return measured.y0 - measured.top;
    default: return measured.bottom - measured.y1;
    }
}

/* Around the frame sub_45bcb0 draws about an element (sub_459440, from the
 * element's rectangle): for the length of the call the rectangle is what was
 * drawn, measured a moment before, then it is put back.  `drawn` before the
 * call, not after it. */
void hudEditorFrame(win32::WinApplication* app, x86::reg32 object, bool drawn)
{
    static x86::reg32 s_object = 0;
    static x86::reg16 s_left, s_top;
    static x86::reg32 s_right, s_bottom;
    if (!drawn)
    {
        if (s_object != object)
            return;
        app->getMemory<x86::reg16>(object + 6) = s_left;
        app->getMemory<x86::reg16>(object + 8) = s_top;
        app->getMemory<x86::reg32>(object + 0x60) = s_right;
        app->getMemory<x86::reg32>(object + 0x64) = s_bottom;
        s_object = 0;
        return;
    }
    const x86::reg32 index = app->getMemory<x86::reg32>(object + 0x3c);
    if (index >= 46 || !hudDrawnFresh(s_hudDrawn[index / 23][index % 23]))
        return;
    const HudDrawn& measured = s_hudDrawn[index / 23][index % 23];
    s_object = object;
    s_left = app->getMemory<x86::reg16>(object + 6);
    s_top = app->getMemory<x86::reg16>(object + 8);
    s_right = app->getMemory<x86::reg32>(object + 0x60);
    s_bottom = app->getMemory<x86::reg32>(object + 0x64);
    app->getMemory<x86::reg16>(object + 6) = x86::reg16(measured.x0);
    app->getMemory<x86::reg16>(object + 8) = x86::reg16(measured.y0);
    app->getMemory<x86::reg32>(object + 0x60) = x86::reg32(measured.x1);
    app->getMemory<x86::reg32>(object + 0x64) = x86::reg32(measured.y1);
}

/* What the HUD editor's test answered, in the log whenever the answer changes
 * ([HUDEDIT]): the element being moved and where to, as fractions of the screen,
 * and the element in its way.  The test hands that one back through the local
 * at [ebp-0x10] of sub_459db0, and only when it found one: an element off the
 * screen, or one refused for split screen's half, is said as such.  Read while
 * the layouts are still as the test saw them. */
static void traceHudLayoutTest(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 element, x86::reg32 rect,
                               x86::reg32 mode, x86::reg32 base)
{
    static int s_last = -2;
    static x86::reg32 s_lastElement = ~0u;
    const bool fits = cpu.eax != 0;
    const float y0 = app->getMemory<float>(rect), x0 = app->getMemory<float>(rect + 4);
    const float y1 = app->getMemory<float>(rect + 8), x1 = app->getMemory<float>(rect + 12);
    int blocker = -1;
    x86::reg32 variant = 0;
    if (!fits && y0 >= 0 && x0 >= 0 && y1 <= 1 && x1 <= 1)
    {
        /* One of the front end's items, 0xbc bytes each from 0x604ee0 (the
         * array sub_459670 searches), or the local was never written. */
        const x86::reg32 other = app->getMemory<x86::reg32>(cpu.ebp - 0x10);
        if (other >= 0x604ee0 && other < 0x604ee0 + 1024 * 0xbc && (other - 0x604ee0) % 0xbc == 0)
            blocker = int(app->getMemory<x86::reg32>(other + 0x3c) % 23);
    }
    const int answer = fits ? -2 : blocker;
    if (answer == s_last && element == s_lastElement)
        return;
    s_last = answer;
    s_lastElement = element;
    if (fits)
        SDL_Log("[HUDEDIT] element %u (mode %u) at top %.3f left %.3f bottom %.3f right %.3f: free",
                unsigned(element), unsigned(mode), y0, x0, y1, x1);
    else if (blocker < 0)
        SDL_Log("[HUDEDIT] element %u (mode %u) at top %.3f left %.3f bottom %.3f right %.3f: refused, no element in the way",
                unsigned(element), unsigned(mode), y0, x0, y1, x1);
    else
        for (variant = 0; variant < 2; ++variant)
        {
            /* Read into floats first: getMemory hands back an accessor, which
             * a variadic call would pass as it is rather than as its value. */
            const x86::reg32 in = base + variant * 0x1a4 + x86::reg32(blocker) * 16;
            const float top = app->getMemory<float>(in), left = app->getMemory<float>(in + 4);
            const float bottom = app->getMemory<float>(in + 8), right = app->getMemory<float>(in + 12);
            SDL_Log("[HUDEDIT] element %u (mode %u) at top %.3f left %.3f bottom %.3f right %.3f: element %d in the way,"
                    " layout %u top %.3f left %.3f bottom %.3f right %.3f (as tested)",
                    unsigned(element), unsigned(mode), y0, x0, y1, x1, blocker, unsigned(variant),
                    top, left, bottom, right);
        }
}

/* Around the HUD editor's test of whether an element may stand where it is
 * (sub_459670), which reads every other element's layout.  Before the test,
 * `drawn` sets the player's two layouts, 0x1a4 apart, as the test should see
 * them, and the call after it puts back exactly what was there -- the test calls
 * nothing that could come back here, so one copy is enough:
 *  - as drawn.  An element the editor measured a moment ago (hudEditorDraw)
 *    stands for what it drew, over the width and the height of the HUD,
 *    [0x7cdac8] and [0x7cdacc], the way the editor turns pixels into its
 *    fractions -- the element being moved too, at its rectangle now, moved as
 *    far as its drawing reached past it when measured, so the test's first
 *    check, that it is on the screen, asks it of what is drawn: the rest of the
 *    box may hang off the edge (hudDragSlack).  An element not measured falls
 *    back on its layout, the ones a wide screen draws narrowed or taller made
 *    so, and the standings table on its panel (hudTablePanel);
 *  - a little inside their frames.  The test counts any overlap at all, and the
 *    places it compares are fractions of the screen worked back from pixels, so
 *    a finger putting two frames edge to edge met them by a pixel or two and the
 *    element blinked red with nothing in its way.  Every other element is taken
 *    in by kSlack of the screen on each side, a quarter of its size at most, so
 *    two frames may touch, or overlap by that much.
 * Before the test cpu holds its arguments: the rectangle being tried in eax, the
 * player in edx, the element in ebx and the editor's mode in ecx; after it, its
 * answer in eax. */
void hudLayoutsDrawn(win32::WinApplication* app, x86::CPU& cpu, bool drawn)
{
    const x86::reg32 kElements = 23;
    const float kSlack = 0.01f;
    static float s_saved[2][kElements][4];
    static float s_savedTried[4];
    static bool s_triedMeasured = false;
    static x86::reg32 s_base = 0, s_element = 0, s_rect = 0, s_mode = 0;
    if (!drawn)
    {
        if (!s_base)
            return;
        traceHudLayoutTest(app, cpu, s_element, s_rect, s_mode, s_base);
        for (x86::reg32 layout = 0; layout < 2; ++layout)
            for (x86::reg32 element = 0; element < kElements; ++element)
                for (x86::reg32 edge = 0; edge < 4; ++edge)
                    app->getMemory<float>(s_base + layout * 0x1a4 + element * 16 + edge * 4) = s_saved[layout][element][edge];
        if (s_triedMeasured)
            for (x86::reg32 edge = 0; edge < 4; ++edge)
                app->getMemory<float>(s_rect + edge * 4) = s_savedTried[edge];
        s_triedMeasured = false;
        s_base = 0;
        return;
    }
    const x86::reg32 player = cpu.edx;
    s_element = cpu.ebx;
    s_rect = cpu.eax;
    s_mode = cpu.ecx;
    const x86::reg32 slot = player + (app->getMemory<x86::reg32>(0x6fd3b0) == 1 ? 1 : 0);
    s_base = 0x6fbc74 + slot * 0x348;
    const x86::reg32 hudWidth = app->getMemory<x86::reg32>(0x7cdac8);
    const x86::reg32 hudHeight = app->getMemory<x86::reg32>(0x7cdacc);
    const bool measurable = player < 2 && hudWidth && hudHeight;
    auto putFractions = [&](x86::reg32 at, x86::sreg32 left, x86::sreg32 top, x86::sreg32 right, x86::sreg32 bottom) {
        app->getMemory<float>(at) = float(top) / float(hudHeight);
        app->getMemory<float>(at + 4) = float(left) / float(hudWidth);
        app->getMemory<float>(at + 8) = float(bottom) / float(hudHeight);
        app->getMemory<float>(at + 12) = float(right) / float(hudWidth);
    };

    if (measurable && s_element < kElements && hudDrawnFresh(s_hudDrawn[player][s_element]))
    {
        for (x86::reg32 edge = 0; edge < 4; ++edge)
            s_savedTried[edge] = app->getMemory<float>(s_rect + edge * 4);
        const HudDrawn& measured = s_hudDrawn[player][s_element];
        const x86::reg32 object = s_rect - 0x40;
        const x86::sreg32 left = x86::sreg16(app->getMemory<x86::reg16>(object + 6));
        const x86::sreg32 top = x86::sreg16(app->getMemory<x86::reg16>(object + 8));
        const x86::sreg32 right = x86::sreg32(app->getMemory<x86::reg32>(object + 0x60));
        const x86::sreg32 bottom = x86::sreg32(app->getMemory<x86::reg32>(object + 0x64));
        putFractions(s_rect, left + measured.x0 - measured.left, top + measured.y0 - measured.top,
                     right + measured.x1 - measured.right, bottom + measured.y1 - measured.bottom);
        s_triedMeasured = true;
    }

    for (x86::reg32 layout = 0; layout < 2; ++layout)
        for (x86::reg32 element = 0; element < kElements; ++element)
        {
            const x86::reg32 rect = s_base + layout * 0x1a4 + element * 16;
            for (x86::reg32 edge = 0; edge < 4; ++edge)
                s_saved[layout][element][edge] = app->getMemory<float>(rect + edge * 4);
            if (measurable && element != s_element && hudDrawnFresh(s_hudDrawn[player][element]))
            {
                const HudDrawn& measured = s_hudDrawn[player][element];
                putFractions(rect, measured.x0, measured.y0, measured.x1, measured.y1);
            }
            else if (element == (layout ? 20u : 13u) && measurable)
            {
                const x86::reg32 panel = 0x749758 + player * 23 * 16 + element * 16;
                const x86::sreg32 panelLeft = x86::sreg32(app->getMemory<x86::reg32>(panel));
                const x86::sreg32 panelTop = x86::sreg32(app->getMemory<x86::reg32>(panel + 4));
                const x86::sreg32 panelRight = x86::sreg32(app->getMemory<x86::reg32>(panel + 8));
                const x86::sreg32 panelBottom = x86::sreg32(app->getMemory<x86::reg32>(panel + 12));
                if (panelRight > panelLeft && panelBottom > panelTop)
                    putFractions(rect, panelLeft, panelTop, panelRight, panelBottom);
            }
            else
            {
                bool across;
                double scale;
                if (widescreenHudScale(app, element, across, scale))
                    scaleHudLayout(app, rect, across, scale);
            }
            const float top = app->getMemory<float>(rect), left = app->getMemory<float>(rect + 4);
            const float bottom = app->getMemory<float>(rect + 8), right = app->getMemory<float>(rect + 12);
            const float down = SDL_min(kSlack, (bottom - top) / 4), in = SDL_min(kSlack, (right - left) / 4);
            if (down > 0)
            {
                app->getMemory<float>(rect) = top + down;
                app->getMemory<float>(rect + 8) = bottom - down;
            }
            if (in > 0)
            {
                app->getMemory<float>(rect + 4) = left + in;
                app->getMemory<float>(rect + 12) = right - in;
            }
        }
}
}

namespace
{
/* NFS_CAR_DETAIL_TRACE: once a second while a race is drawn, what car detail
 * the game settled on, read back from its own state after each swap.
 *  - The level every car was last drawn at (car+0x8b4, -1 when it was not
 *    drawn) and what that costs against the budget in sub_4bbad0: 4 minus the
 *    level per car, capped per Car Detail setting by the table at 0x4b7a50.
 *  - The transform buffer behind Render_GetTm (sub_4bbde0), a bump allocator:
 *    base 0x55fe34, cursor 0x55fe38, limit 0x7a3d04.  It starts over for every
 *    3D pass and does not take a failed allocation back, so a cursor past the
 *    limit at swap time means the last pass ran out and left something undrawn.
 *  - The texture atlas's free space.
 * Levels and the buffer describe the frame's last 3D pass, which with the
 * rear-view mirror on need not be the main view. */
struct CarDetailTrace
{
    Uint64 lastLog = 0;
    x86::reg32 peakBuffer = 0;
    x86::reg32 overflowFrames = 0;
    x86::reg32 frames = 0;
};
CarDetailTrace s_carDetailTrace;

void traceCarDetail(win32::WinApplication* app)
{
    CarDetailTrace& trace = s_carDetailTrace;
    const x86::reg32 limit = app->getMemory<x86::reg32>(0x7a3d04);
    const x86::reg32 base = app->getMemory<x86::reg32>(0x55fe34);
    const x86::reg32 cursor = app->getMemory<x86::reg32>(0x55fe38);
    if (base && cursor >= base)
    {
        trace.peakBuffer = SDL_max(trace.peakBuffer, cursor - base);
        if (cursor - base > limit)
            ++trace.overflowFrames;
    }
    ++trace.frames;
    const Uint64 now = SDL_GetTicks();
    if (now - trace.lastLog < 1000)
        return;
    trace.lastLog = now;

    const x86::reg32 count = app->getMemory<x86::reg32>(0x5efd9c);
    int levels[4] = {};
    int hidden = 0;
    int points = 0;
    for (x86::reg32 i = 0; i < count && i < 64; ++i)
    {
        const x86::reg32 car = app->getMemory<x86::reg32>(0x5efac8 + i * 4);
        const x86::sreg32 level = car ? x86::sreg32(app->getMemory<x86::reg32>(car + 0x8b4)) : -1;
        if (level < 0 || level > 3)
        {
            ++hidden;
            continue;
        }
        ++levels[level];
        points += 4 - level;
    }
    const x86::reg32 detail = app->getMemory<x86::reg32>(0x6fbc1c);
    const x86::sreg32 budget = detail < 3 ? x86::sreg32(app->getMemory<x86::reg32>(0x4b7a50 + detail * 4)) : 0;
    x86::reg32 freeTexels = 0, freeTiles = 0, totalTexels = 0;
    win32::glide2x::atlasFreeSpace(freeTexels, freeTiles, totalTexels);
    SDL_Log("[CARDETAIL] detail=%u cars=%u L0=%d L1=%d L2=%d L3=%d hidden=%d points=%d budget=%d"
            " buffer peak=%u/%u overflow=%u/%u frames atlas=%u free=%u%% tiles256=%u",
            unsigned(detail), unsigned(count), levels[0], levels[1], levels[2], levels[3], hidden,
            points, int(budget), unsigned(trace.peakBuffer), unsigned(limit),
            unsigned(trace.overflowFrames), unsigned(trace.frames), unsigned(SDL_sqrt(double(totalTexels))),
            totalTexels ? unsigned(100ull * freeTexels / totalTexels) : 0u, unsigned(freeTiles));
    trace.peakBuffer = 0;
    trace.overflowFrames = 0;
    trace.frames = 0;
}

/* How hard player one's car corners, for the phone's own vibration, about every
 * 40 ms while a race is drawn.  It is the load the game's Body Roll force
 * feedback is made of (sub_4716e0): [car+0x924] against the scale at 0x5efde4,
 * full where the game clamps that effect.  Read from the car rather than taken
 * from the effect, which the game only computes while its Body Roll slider is
 * above zero -- a pad's setting the phone should not depend on.  The car is the
 * one force feedback follows (0x678b54); while its flag at +0x164 is set or
 * +0x15c is negative the game stops its effects, and there is no cornering to
 * feel either. */
void phoneTick(win32::WinApplication* app)
{
    static Uint64 last = 0;
    const Uint64 now = SDL_GetTicks();
    if (now - last < 40)
        return;
    last = now;
    float turn = 0;
    const x86::reg32 car = app->getMemory<x86::reg32>(0x678b54);
    const float scale = app->getMemory<float>(0x5efde4);
    if (car && scale > 0 && app->getMemory<x86::reg16>(car + 0x164) == 0
        && app->getMemory<float>(car + 0x15c) >= 0)
        turn = SDL_min(1.0f, SDL_fabsf(app->getMemory<float>(car + 0x924)) / scale);
    win32::Gamepad::phoneTick(turn);
}

/* The on-screen steering buttons steer the way the game steers for a key.  A
 * car takes the steering it is given as it is while its player's steering is
 * analog, and turns towards it -- to full lock at the car's rate at
 * [params+0x240], back to the centre at +0x244 -- while that player's record
 * says it is digital: [[car+0x220]+0x24], checked at 0x4aa26b, one of the
 * records at 0x6fd52c, 0x6c each.  The game fills that flag at the start of a
 * race from the kind of binding steering has (sub_43c040, then 0x49adba); the
 * control profile binds it to the first slot's axis, so it starts analog.
 * While the buttons are what steers player one it is raised, and the pad's
 * stick lowers it again.  Player one's car is the one force feedback follows
 * (0x678b54, as in phoneTick). */
void steeringTick(win32::WinApplication* app)
{
    static int s_last = -1;
    if (!nfs3hp::raceIsRunning())
    {
        s_last = -1;
        return;
    }
    const x86::reg32 car = app->getMemory<x86::reg32>(0x678b54);
    if (!car)
        return;
    const x86::reg32 record = app->getMemory<x86::reg32>(car + 0x220);
    if (record < 0x6fd52c || record >= 0x6fd52c + 16 * 0x6c || (record - 0x6fd52c) % 0x6c)
        return;
    const x86::reg32 digital = win32::Gamepad::touchSteering() ? 1 : 0;
    if (app->getMemory<x86::reg32>(record + 0x24) != digital)
        app->getMemory<x86::reg32>(record + 0x24) = digital;
    if (s_last != int(digital))
    {
        SDL_Log("[STEERING] player %u: %s", unsigned((record - 0x6fd52c) / 0x6c),
                digital ? "the game's keyboard steering (touch buttons)" : "analog (pad)");
        s_last = int(digital);
    }
}

bool s_traceCarDetail = false;
bool s_tracePointer = false;

/* A finger on the picture, or a real mouse, as the game's own pointer.
 *
 * The game has no idea where a pointer is: its DirectInput mouse reports how
 * far it moved, and sub_435310 adds that to the cursor it keeps itself at
 * 0x5f38f0 and 0x5f38f4, clamped to the bounds sub_4352d0 set -- the whole
 * 640x480 of a menu, or the race viewport.  Either way the cursor counts in
 * pixels of the picture, so the movement is worked out here from where the
 * pointer is on the screen: that place, less where the cursor stands, is what
 * the game is told it moved.  It arrives through the same buffered queue a
 * mouse fills, in the same order, so the game applies it the same way -- and
 * the cursor is read again every frame, so the game warping it itself
 * (sub_49bfc0, moving it onto a highlighted item) corrects the next answer
 * rather than throwing it off.  A mouse goes the same way rather than by the
 * distance it moved: that distance counts in pixels of the phone's screen, up
 * to three times the 640 a menu spans, and the game's cursor ran away from the
 * system pointer it was meant to follow.
 *
 * Both queue up on the Android UI thread and are turned into movement here, on
 * the game thread, where the guest context is held.  A press waits a frame
 * before its release can follow, so a click too quick to span a frame is still
 * a press the game sees.  When a finger presses at all is TouchPointer.java's
 * to say: only a tap clicks, and a finger held still presses and drags.
 *
 * While the player drives, the finger is no pointer: the race has nothing to
 * point at, and its standings table sits under the steering buttons, where a
 * thumb that slid off one landed on a name and the view went over to that
 * opponent.  Touches that arrive then are dropped, and a press still held from
 * the menu is let go.  The pause menu and everything after the race take it
 * back.  A mouse keeps working in a race, as it does in the original. */
struct PointerInput
{
    int   source;   // kSourceTouch, kSourceMouse
    int   action;   // a finger: kTouchDown, kTouchMove, kTouchUp
    float x, y;     // window pixels
    int   buttons;  // a mouse: bit 0 left, 1 right, 2 middle
};
const int kSourceTouch = 0, kSourceMouse = 1;
const int kTouchDown = 0, kTouchMove = 1, kTouchUp = 2;
const int kPointerQueue = 32;

SDL_Mutex*   s_touchMutex;
PointerInput s_pointerQueue[kPointerQueue];
int          s_pointerCount;
bool         s_touchHeld;
int          s_mouseButtons;

x86::sreg32 clampAxis(x86::sreg32 value, x86::sreg32 low, x86::sreg32 high)
{
    return value < low ? low : (value > high ? high : value);
}

/* The player is driving: a race is up and its menu is not open over it
 * (sub_4bc680 keeps [0x7a3d10] set while it is). */
bool playerDriving(win32::WinApplication* app)
{
    return nfs3hp::raceIsRunning() && app->getMemory<x86::reg32>(0x7a3d10) == 0;
}

void pointerTick(win32::WinApplication* app)
{
    if (!s_touchMutex)
        return;
    const bool driving = playerDriving(app);
    if (driving && s_touchHeld)
    {
        win32::Mouse::pressPointer(0, false);
        s_touchHeld = false;
    }
    SDL_LockMutex(s_touchMutex);
    const int count = s_pointerCount;
    PointerInput inputs[kPointerQueue];
    if (count)
        SDL_memcpy(inputs, s_pointerQueue, count * sizeof(PointerInput));
    SDL_UnlockMutex(s_touchMutex);
    if (!count)
        return;

    int rectX, rectY, rectW, rectH, pictureW, pictureH;
    if (!win32::glide2x::screenRect(rectX, rectY, rectW, rectH, pictureW, pictureH))
        return;
    (void)pictureW;
    (void)pictureH;
    /* The cursor counts in whatever space the game gave it, which is the whole
     * of the picture in a race and the 640x480 its menus are laid out in --
     * sub_4352d0 sets the bounds either way.  So a touch is placed in those
     * bounds rather than in pixels of the picture, and follows the menus
     * wherever they are drawn. */
    const x86::sreg32 lowX = x86::sreg32(app->getMemory<x86::reg32>(0x5f38e0));
    const x86::sreg32 lowY = x86::sreg32(app->getMemory<x86::reg32>(0x5f38e4));
    const x86::sreg32 highX = x86::sreg32(app->getMemory<x86::reg32>(0x5f38e8));
    const x86::sreg32 highY = x86::sreg32(app->getMemory<x86::reg32>(0x5f38ec));
    if (highX <= lowX || highY <= lowY)
        return;
    /* Where the game has the cursor now; from here on it is followed in step
     * with what it is told, since it only catches up when the game next reads
     * the queue -- once for everything handed over this frame. */
    x86::sreg32 atX = clampAxis(x86::sreg32(app->getMemory<x86::reg32>(0x5f38f0)), lowX, highX);
    x86::sreg32 atY = clampAxis(x86::sreg32(app->getMemory<x86::reg32>(0x5f38f4)), lowY, highY);

    int taken = 0;
    for (; taken < count; ++taken)
    {
        const PointerInput& input = inputs[taken];
        const bool touch = input.source == kSourceTouch;
        if (touch && driving)
            continue;
        const x86::sreg32 toX = clampAxis(
            lowX + x86::sreg32(SDL_lround(double(input.x - rectX) * double(highX - lowX) / double(rectW))), lowX, highX);
        const x86::sreg32 toY = clampAxis(
            lowY + x86::sreg32(SDL_lround(double(input.y - rectY) * double(highY - lowY) / double(rectH))), lowY, highY);
        win32::Mouse::movePointer(toX - atX, toY - atY);
        atX = toX;
        atY = toY;
        if (s_tracePointer)
        {
            if (touch)
                SDL_Log("[POINTER] finger %s window %.0f,%.0f -> cursor %d,%d of %dx%d",
                        input.action == kTouchDown ? "down" : (input.action == kTouchUp ? "up" : "move"),
                        input.x, input.y, int(toX), int(toY), int(highX), int(highY));
            else
                SDL_Log("[POINTER] mouse buttons %d window %.0f,%.0f -> cursor %d,%d of %dx%d",
                        input.buttons, input.x, input.y, int(toX), int(toY), int(highX), int(highY));
        }
        if (touch)
        {
            if (input.action == kTouchDown && !s_touchHeld)
            {
                win32::Mouse::pressPointer(0, true);
                s_touchHeld = true;
                ++taken;
                break;  // the release waits for the next frame, so the press is its own poll
            }
            if (input.action == kTouchUp && s_touchHeld)
            {
                win32::Mouse::pressPointer(0, false);
                s_touchHeld = false;
            }
            continue;
        }
        bool pressed = false;
        for (int button = 0; button < 3; ++button)
        {
            const int bit = 1 << button;
            if ((input.buttons ^ s_mouseButtons) & bit)
            {
                const bool down = (input.buttons & bit) != 0;
                win32::Mouse::pressPointer(x86::reg32(button), down);
                pressed = pressed || down;
            }
        }
        s_mouseButtons = input.buttons;
        if (pressed)
        {
            ++taken;
            break;  // likewise
        }
    }

    SDL_LockMutex(s_touchMutex);
    s_pointerCount -= taken;
    if (s_pointerCount > 0)
        SDL_memmove(s_pointerQueue, s_pointerQueue + taken, s_pointerCount * sizeof(PointerInput));
    SDL_UnlockMutex(s_touchMutex);
}

/* One step of a pointer, from the Android UI thread.  A move that follows
 * another move of the same pointer, with nothing in between, replaces it: only
 * the last place reached matters, and a queue that filled up would drop the
 * release at the end of it. */
void queuePointer(const PointerInput& input)
{
    if (!s_touchMutex)
        return;
    SDL_LockMutex(s_touchMutex);
    if (s_pointerCount > 0)
    {
        const PointerInput& last = s_pointerQueue[s_pointerCount - 1];
        const bool move = input.source == kSourceTouch ? input.action == kTouchMove : true;
        const bool lastMove = last.source == kSourceTouch ? last.action == kTouchMove : true;
        if (move && lastMove && last.source == input.source && last.buttons == input.buttons)
            --s_pointerCount;
    }
    if (s_pointerCount < kPointerQueue)
        s_pointerQueue[s_pointerCount++] = input;
    SDL_UnlockMutex(s_touchMutex);
}

void queueScreenTouch(int action, float x, float y)
{
    PointerInput input = {};
    input.source = kSourceTouch;
    input.action = action;
    input.x = x;
    input.y = y;
    queuePointer(input);
}

/* Where a real mouse is, in window pixels, and which of its buttons are down. */
void queueMouse(float x, float y, int buttons)
{
    PointerInput input = {};
    input.source = kSourceMouse;
    input.action = kTouchMove;
    input.x = x;
    input.y = y;
    input.buttons = buttons & 7;
    queuePointer(input);
}

#ifndef __ANDROID__
/* NFS_TOUCH_SCRIPT, desktop diagnostics: touches on the picture at set times, so
 * a run with no hand on the mouse walks the menus exactly the way a finger does
 * on a phone -- through queueScreenTouch, the same path.  "ms@x,y" taps pixel
 * x,y of the game's picture that many milliseconds after startup, and
 * "ms@key:Name" presses a key by its SDL name.  Steps are separated by ';' and
 * run in order.  Each run is logged as [TOUCHSCRIPT]. */
struct ScriptStep
{
    Uint64      at;
    bool        key;
    float       x, y;
    SDL_Keycode code;
};
std::vector<ScriptStep> s_script;
size_t s_scriptNext = 0;
int s_scriptLift = 0;  // frames until the tap or key in progress lifts
float s_scriptX = 0, s_scriptY = 0;
SDL_Keycode s_scriptKey = SDLK_UNKNOWN;  // the key held, if it is a key

void loadTouchScript()
{
    const char* text = SDL_getenv("NFS_TOUCH_SCRIPT");
    if (!text)
        return;
    std::string all(text);
    size_t start = 0;
    while (start < all.size())
    {
        size_t end = all.find(';', start);
        if (end == std::string::npos)
            end = all.size();
        const std::string step = all.substr(start, end - start);
        start = end + 1;
        const size_t at = step.find('@');
        if (at == std::string::npos)
            continue;
        ScriptStep s = {};
        s.at = Uint64(SDL_strtoull(step.substr(0, at).c_str(), nullptr, 10));
        const std::string what = step.substr(at + 1);
        if (what.compare(0, 4, "key:") == 0)
        {
            s.key = true;
            s.code = SDL_GetKeyFromName(what.substr(4).c_str());
            if (s.code == SDLK_UNKNOWN)
                continue;
        }
        else if (SDL_sscanf(what.c_str(), "%f,%f", &s.x, &s.y) != 2)
            continue;
        s_script.push_back(s);
    }
    SDL_Log("[TOUCHSCRIPT] %d steps", int(s_script.size()));
}

void pushScriptKey(SDL_Keycode code, bool down)
{
    SDL_Event event;
    SDL_zero(event);
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.timestamp = SDL_GetTicksNS();
    event.key.key = code;
    event.key.scancode = SDL_GetScancodeFromKey(code, nullptr);
    event.key.down = down;
    SDL_PushEvent(&event);
}

/* A key is held for a few frames like a tap: a race reads its keys once a frame
 * rather than from messages, and a press let go in the same frame never
 * registered there. */
void scriptTick()
{
    if (s_scriptLift > 0 && --s_scriptLift == 0)
    {
        if (s_scriptKey != SDLK_UNKNOWN)
            pushScriptKey(s_scriptKey, false);
        else
            queueScreenTouch(kTouchUp, s_scriptX, s_scriptY);
        s_scriptKey = SDLK_UNKNOWN;
    }
    if (s_scriptLift > 0 || s_scriptNext >= s_script.size() || SDL_GetTicks() < s_script[s_scriptNext].at)
        return;
    const ScriptStep& step = s_script[s_scriptNext++];
    if (step.key)
    {
        SDL_Log("[TOUCHSCRIPT] key %s", SDL_GetKeyName(step.code));
        pushScriptKey(step.code, true);
        s_scriptKey = step.code;
        s_scriptLift = 3;
        return;
    }
    int rectX, rectY, rectW, rectH, pictureW, pictureH;
    if (!win32::glide2x::screenRect(rectX, rectY, rectW, rectH, pictureW, pictureH))
        return;
    s_scriptX = float(rectX) + step.x * float(rectW) / float(pictureW);
    s_scriptY = float(rectY) + step.y * float(rectH) / float(pictureH);
    SDL_Log("[TOUCHSCRIPT] tap %.0f,%.0f of %dx%d", step.x, step.y, pictureW, pictureH);
    queueScreenTouch(kTouchDown, s_scriptX, s_scriptY);
    s_scriptLift = 3;
}
#endif

/* Which of the two on-screen layouts belongs on the screen, told to the Android
 * side each time it changes.  The player is driving while a race is up and its
 * menu is not open over it.  That menu is sub_4bc680's to open and close, once a
 * frame of the race: it sets [0x7a3d10] as the menu comes up and clears it as it
 * goes, and nothing else writes it.  (Not [0x559234], which reads as if it
 * should do: the game sets that one for its screens drawn at the race's own
 * resolution, the loading screen among them, and it stays set for the whole of a
 * race that was never paused.)  So the controls follow the game instead of the
 * player pressing a button to say which of the two they are looking at. */
void layoutTick(win32::WinApplication* app)
{
    static int last = -1;
    const bool driving = playerDriving(app);
    if (last == int(driving))
        return;
#ifdef __ANDROID__
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity)
        return;  // nothing to tell yet; asked again next frame
    jclass cls = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(cls, "onGameDriving", "(Z)V");
    if (method)
        env->CallVoidMethod(activity, method, jboolean(driving ? JNI_TRUE : JNI_FALSE));
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#endif
    SDL_Log("[LAYOUT] %s, screen %ux%u", driving ? "race" : "menu",
            unsigned(app->getMemory<x86::reg32>(0x7cdae0)), unsigned(app->getMemory<x86::reg32>(0x7cdae4)));
    last = int(driving);
}

/* Whether the pads' keys would drive somebody else's car.  A pad's buttons send
 * the keys the launcher's control profile binds (ControlProfile.java), player
 * one's for the first pad and player two's for the second.  A player who keeps
 * the game's own keyboard controls instead -- its defaults, which "Default in
 * game" writes, are what a keyboard player wants -- has split screen's second
 * player on Space, A, Z and S, the first pad's handbrake, gears and spikes, and
 * its first player steering with the arrows both D-pads send.  So while the two
 * of them drive on exactly those tables, the pads keep quiet but for Escape; a
 * pad is set up for a race with Gamepad ON, and the Gamepads screen says so.
 * One player on the defaults has no one to collide with, and a pad drives that
 * car as it did.  The tables live at 0x6fd20c (one player, then split screen's
 * two), and the defaults the game restores them from at 0x490430 (sub_490c40):
 * three tables of thirteen bindings each, compared as they are, so any binding
 * the player changed in the game's own Controls screen lifts it. */
void padKeysTick(win32::WinApplication* app)
{
    bool muted = false;
    if (playerDriving(app) && app->getMemory<x86::reg32>(0x6fd3b0) == 1)
    {
        muted = true;
        for (x86::reg32 i = 0; i < 3 * 13 && muted; ++i)
            muted = app->getMemory<x86::reg32>(0x6fd20c + i * 4) == app->getMemory<x86::reg32>(0x490430 + i * 4);
    }
    static int last = -1;
    if (last != int(muted))
    {
        SDL_Log("[PADKEYS] %s", muted ? "muted: split screen on the game's keyboard controls" : "on");
        last = int(muted);
    }
    win32::Gamepad::muteGameKeys(muted);
}

/* Every buffer swap: the finger and the mouse on the picture, which layout the
 * controls should be showing, whether the pads' keys go out, the phone's
 * cornering, how the touch buttons steer, and the car detail trace when asked
 * for (NFS_CAR_DETAIL_TRACE). */
void onSwap(win32::WinApplication* app)
{
    ++nfs3hp::s_swaps;
#ifndef __ANDROID__
    scriptTick();
#endif
    pointerTick(app);
    layoutTick(app);
    padKeysTick(app);
    phoneTick(app);
    steeringTick(app);
    if (s_traceCarDetail)
        traceCarDetail(app);
}
}

#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#include <SDL3/SDL.h>
#include <lib/renderer.h>
#include <jni.h>
#endif

static std::string getExeDirectory(const char* argv0)
{
    std::string path(argv0);
    for (auto& c : path)
        if (c == '\\') c = '/';
    auto pos = path.rfind('/');
    if (pos != std::string::npos)
        return path.substr(0, pos + 1);
    return "./";
}

int main(int argc, char* argv[])
{
#ifdef __ANDROID__
    /* There is no argv[1]/argv[2] on Android and argv[0] carries no directory
     * (SDL synthesizes it), so getExeDirectory()'s fallback would resolve to
     * the process cwd, not the app's data.  SDL_GetAndroidExternalStoragePath()
     * is Context.getExternalFilesDir() -- app-private but world-readable, which
     * is where the first-run import screen (M3) copies fedata/gamedata/drivers
     * into.  There is no separate "CD" location on Android: both point here. */
    NFS2_USE(argc);
    NFS2_USE(argv);
    const char* dataPath = SDL_GetAndroidExternalStoragePath();
    NFS2_ASSERT(dataPath);
    win32::File::setDataDirectory(dataPath);
    win32::File::setCdDirectory(dataPath);
#else
    if (argc >= 3)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[2]);
    }
    else if (argc == 2)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[1]);
    }
    else
    {
        std::string exeDir = getExeDirectory(argv[0]);
        win32::File::setDataDirectory(exeDir.c_str());
        win32::File::setCdDirectory(exeDir.c_str());
    }
#endif
    /* The game pumps its message queue from a guest thread, which is not
     * the thread SDL considers "main" on Android.  SDL only warns about
     * it, but the warning is a modal dialog that has to be dismissed on
     * every single launch.  Suppress the dialog rather than the check:
     * nothing downstream depends on it, and a real fix means moving the
     * pump onto the SDL main thread, which is a much larger change. */
    SDL_SetHint(SDL_HINT_ASSERT, "always_ignore");
    /* Without this the game starts in portrait on Android, and the manifest
     * cannot stop it: SDLActivity.setOrientationBis() calls
     * setRequestedOrientation() at runtime and overrides screenOrientation.
     * It derives what to allow from SDL_HINT_ORIENTATIONS, and with the hint
     * unset it decides nothing is explicitly allowed, which for a resizable
     * window means SCREEN_ORIENTATION_FULL_USER -- i.e. whatever the system
     * auto-rotate setting says.  The game is landscape only; NFS_ORIENTATION
     * just picks whether it may turn over: "auto" allows both landscape
     * directions, anything else pins one.  The hint has to agree with the
     * activity's requested orientation, because setOrientationBis derives the
     * request from the hint and would otherwise undo it. */
    {
        /* SDL maps LandscapeLeft to SCREEN_ORIENTATION_LANDSCAPE and
         * LandscapeRight to its reverse, so naming one of them here is what
         * pins that direction; naming both hands the choice to the sensor. */
        const char* orientation = SDL_getenv("NFS_ORIENTATION");
        const bool autoRotate = orientation && SDL_strcasecmp(orientation, "auto") == 0;
        const bool reversed = orientation && SDL_strcasecmp(orientation, "landscape_reverse") == 0;
        SDL_SetHint(SDL_HINT_ORIENTATIONS,
                    autoRotate ? "LandscapeLeft LandscapeRight"
                    : reversed ? "LandscapeRight" : "LandscapeLeft");
    }
    /* SDL_INIT_GAMEPAD is what turns raw joysticks into the mapped
     * SDL_EVENT_GAMEPAD_* stream window.cpp translates into keystrokes. */
    SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK|SDL_INIT_GAMEPAD);
    /* Window::getMessage consumes key, text, quit and its own registered user
     * events; everything else falls through its switch and is thrown away.  The
     * pad is read by polling its state (Gamepad::update), never from
     * events.  Leaving those event types enabled meant a connected controller,
     * reporting at its native rate, kept the queue non-empty -- so SDL_WaitEvent
     * never actually blocked and the message pump ran SDL_PumpEvents thousands
     * of times a second for nothing.  A simpleperf profile showed that thread
     * burning most of a core inside SDL with zero game code on it. */
    SDL_SetJoystickEventsEnabled(false);
    SDL_SetGamepadEventsEnabled(false);
    {
        nfs3hp::Application app("nfs3.exe");
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Device Description", new win32::RegistryValue("3Dfx Voodoo 2"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Card", new win32::RegistryValue("3Dfx Voodoo 2"));
        //app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new win32::RegistryValue("softtri"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new win32::RegistryValue("voodoo2"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Group", new win32::RegistryValue("3Dfx"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "D3D Device", new win32::RegistryValue(x86::reg32(0)));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Triple Buffer", new win32::RegistryValue(x86::reg32(0)));
        //app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new win32::RegistryValue(x86::reg32(0)));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new win32::RegistryValue(x86::reg32(1)));

        /* Car detail is chosen by distance alone -- there is no per-car role
         * anywhere in the selection.  The table at 0x55fdf4 holds four floats
         * per "cardetail" setting: three level-of-detail switch distances and a
         * cull distance, and level 0 (High) switches away from the detailed
         * model at 30 while culling only at 300.  The player's car sits inside
         * 30 under every chase camera, which is the whole reason it alone looks
         * detailed; opponents, traffic and the second player drop a level as
         * soon as they are further away than that.
         *
         * Holding the detailed model out to the cull distance makes every car
         * match the player's.  It costs emulated CPU rather than GPU -- Glide
         * takes screen-space vertices, so the game transforms all that geometry
         * itself -- so NFS_CAR_DETAIL_FULL=0 restores the original distances,
         * budget and texture sizes without a rebuild.  Only the High row is
         * touched: Medium and Low stay as they were, and the cull distance
         * itself is left alone so the draw distance does not change. */
        if (nfs3hp::fullCarDetail())
        {
            /* Twice the cull distance rather than the distance itself: split
             * screen scales the three switch distances by 0.75, and the rear-view
             * mirror scales them and the cull distance by 0.6 (sub_4bb5d0), both
             * at once in a split-screen mirror.  Twice the cull distance stays at
             * or beyond the cull distance under all of those, so a car is either
             * on the detailed model or not drawn at all, in every view.  Held at
             * just the cull distance, split screen drew every car beyond 225 as
             * the tiny box. */
            const x86::reg32 highRow = 0x55fdf4;
            const float cull = app.getMemory<float>(highRow + x86::reg32(12));
            for (x86::reg32 level = 0; level < 3; ++level)
                app.getMemory<float>(highRow + level * 4) = cull * 2.0f;
            /* The distances alone did not do it.  Once every car has its level,
             * sub_4bbad0 charges each car it will draw 4 minus that level against
             * a budget per setting -- 10, 8 and 6 in the table at 0x4b7a50 -- and
             * while over, drops the furthest car at medium to low, or failing that
             * the furthest detailed car to medium.  It never touches the car the
             * view follows and never goes below low.  The player's car spends 4
             * of High's 10, so with three other cars in view all of them end up
             * on the low model, whose wheels are part of the body and do not turn,
             * however close they are: detail that fell with the number of cars
             * and, of all places, up close.  The game copies the table on every
             * call, so lifting High's budget here is enough. */
            const x86::reg32 budgetTable = 0x4b7a50;
            app.getMemory<x86::reg32>(budgetTable) = 0x7fffffff;
            /* Every car loads the player's 256x256 texture as well (the patched
             * sub_4b7d30), up to four times the atlas room each.  A 4096 atlas
             * takes about 90 MB of graphics memory, so only where memory is to
             * spare; elsewhere a texture that does not fit is kept a mip level
             * smaller rather than dropped. */
            if (SDL_GetSystemRAM() >= 3072)
                win32::glide2x::setPreferredAtlasSize(4096);
        }
        const char* detailTrace = SDL_getenv("NFS_CAR_DETAIL_TRACE");
        s_traceCarDetail = detailTrace && *detailTrace && *detailTrace != '0';
        const char* pointerTrace = SDL_getenv("NFS_POINTER_TRACE");
        s_tracePointer = pointerTrace && *pointerTrace && *pointerTrace != '0';
        s_touchMutex = SDL_CreateMutex();
#ifndef __ANDROID__
        loadTouchScript();
#endif
        win32::glide2x::setSwapObserver(onSwap);

        /* The screen sizes the Graphics menu offers.  The Voodoo2 driver's mode
         * table (0xa92018, 40 bytes a mode: width, height, depth, LFB format,
         * available, then the colour and aux buffer counts sub_a83ac0 works out
         * from the card's memory) holds entries the card never offered, and the
         * last three are sizes no card could open -- 1280x1200, 1600x1200,
         * 400x300 -- so they carry the widescreen modes instead, each under a
         * resolution id of the port's own in the table THRASH_setvideomode hands
         * grSstWinOpen (0xa922e8).  The Graphics menu lists them once
         * tools/apply_widescreen.py lets a 16:9 mode through sub_4bed90, which
         * still applies every other test of its own: 16 bits, at least two
         * colour buffers, and within the driver's memory limit, which at 8 MB
         * leaves 1920x1080 about fifty kilobytes to spare.  The driver's data is
         * in place from construction on -- LoadLibrary does not copy it again --
         * so this sticks.  NFS_WIDESCREEN=0 leaves the three entries alone. */
        const char* widescreen = SDL_getenv("NFS_WIDESCREEN");
        if (!widescreen || SDL_strcmp(widescreen, "0") != 0)
        {
            static const struct { x86::reg32 entry, width, height, resolution; } kWide[] = {
                { 14, 1280, 720, win32::glide2x::kResolution1280x720 },
                { 15, 1600, 900, win32::glide2x::kResolution1600x900 },
                { 16, 1920, 1080, win32::glide2x::kResolution1920x1080 },
            };
            for (const auto& wide : kWide)
            {
                const x86::reg32 mode = 0xa92018 + wide.entry * 40;
                app.getMemory<x86::reg32>(mode) = wide.width;
                app.getMemory<x86::reg32>(mode + 4) = wide.height;
                app.getMemory<x86::reg32>(mode + 16) = 1;
                app.getMemory<x86::reg32>(0xa922e8 + wide.entry * 4) = wide.resolution;
            }
        }
        /* 512x384, the one size the list held below the game's own 640x480.  A
         * phone has pixels to spare and every menu is drawn for 640x480, so it
         * leaves the table the same way the widescreen modes join it: by the
         * available flag. */
        app.getMemory<x86::reg32>(0xa92018 + 6 * 40 + 16) = 0;

#ifndef __ANDROID__
        // Headless driver metadata regression: exercise the real recompiled queries.
        if (SDL_getenv("NFS_CHECK_GLIDE_MODES")) {
            for (const auto entry : {0x504e00u, 0xa83c60u}) {
                x86::CPU cpu{};
                app.runThread(cpu, entry);
                const auto table = app.getMemory<x86::reg32>(cpu.eax + 0x40);
                const auto count = app.getMemory<x86::reg32>(cpu.eax + 0x3c);
                if (count != 16) return 2;
                for (unsigned i = 1; i <= count; ++i) {
                    const auto* mode = &app.getMemory<x86::reg32>(table + i*40);
                    if (mode[2] != 32 || mode[3] != 4) return 3;
                    SDL_Log("[OUTPUT32] driver=%x id=%u %ux%ux%u LFB-format=%u available=%u buffers=%u",
                            entry, i, mode[0], mode[1], mode[2], mode[3], mode[4], mode[5]);
                }
                // Repeat query, including the cached descriptor path.
                app.runThread(cpu, entry);
                if (app.getMemory<x86::reg32>(table + 40 + 8) != 32) return 4;
            }
            SDL_Log("[OUTPUT32] PASS: both drivers and cached queries");
        } else
#endif
        app.execute();
    }
    SDL_Quit();
    return 0;
}

#ifdef __ANDROID__
/* The overlay's keyboard button, both ways.  It goes through SDL rather than
 * SDLActivity.showTextInput() because showing the Android keyboard is only half
 * of it: SDL_SendKeyboardText drops every character while text input is
 * inactive, so a keyboard raised behind SDL's back deletes (backspace is a key
 * event, ungated) but types nothing.  Starting and stopping through SDL also
 * raises the shown/hidden events the renderer listens to, so the picture rises
 * and drops on its own.
 *
 * Called on the Android UI thread; so is SDL's own SDL_StopTextInput from
 * SDLDummyEdit.onKeyPreIme when the system back key closes the keyboard. */
static SDL_Window* keyboardWindow()
{
    SDL_Window* window = SDL_GetKeyboardFocus();
    if (!window)
    {
        int count = 0;
        SDL_Window* const* windows = SDL_GetWindows(&count);
        if (count > 0)
            window = windows[0];
    }
    return window;
}

extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeToggleKeyboard(JNIEnv*, jclass)
{
    SDL_Window* window = keyboardWindow();
    if (!window)
        return;

    if (SDL_TextInputActive(window))
        SDL_StopTextInput(window);
    else
        SDL_StartTextInput(window);
}
/* Whether the keyboard the overlay's button raises is up, as SDL has it -- the
 * system back key closes it rather than leaving the screen it types into. */
extern "C" JNIEXPORT jboolean JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeKeyboardActive(JNIEnv*, jclass)
{
    SDL_Window* window = keyboardWindow();
    return window && SDL_TextInputActive(window) ? JNI_TRUE : JNI_FALSE;
}
/* A finger on the game's picture: where it is, in window pixels, and whether it
 * presses, only points, or lets go.  The overlay takes its own buttons first,
 * and TouchPointer.java decides when a finger presses; what reaches here is
 * what the game's pointer does (see pointerTick).  Called on the Android UI
 * thread. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeScreenTouch(JNIEnv*, jclass, jint action, jfloat x, jfloat y)
{
    queueScreenTouch(int(action), float(x), float(y));
}
/* A real mouse, or a touchpad Android drives as one: where its pointer is, in
 * window pixels, and which buttons are down -- bit 0 left, 1 right, 2 middle.
 * Called on the Android UI thread. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeMousePointer(JNIEnv*, jclass, jfloat x, jfloat y, jint buttons)
{
    queueMouse(float(x), float(y), int(buttons));
}
/* Which physical pad is Gamepad 1 and which is Gamepad 2, resolved by the
 * launcher's rules (GamepadSlots.java) and sent again whenever Android attaches
 * or detaches an input device.  Called on the Android UI thread; the slots
 * themselves are only touched on the game thread, on the next frame. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeSetGamepadSlots(JNIEnv* env, jclass, jstring first, jstring second)
{
    const char* a = first ? env->GetStringUTFChars(first, nullptr) : nullptr;
    const char* b = second ? env->GetStringUTFChars(second, nullptr) : nullptr;
    win32::Gamepad::setSlotDescriptors(a, b);
    if (a)
        env->ReleaseStringUTFChars(first, a);
    if (b)
        env->ReleaseStringUTFChars(second, b);
}
#endif
