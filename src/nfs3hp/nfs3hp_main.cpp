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

/* Whether the game is waiting for a name to be typed: the dialog that takes one
 * is up (tools/apply_text_entry.py).  Read once a frame by textEntryTick, which
 * is what raises and lowers the phone's keyboard by it. */
static bool s_takingText = false;

void textEntry(bool taking)
{
    s_takingText = taking;
}

bool takingText()
{
    return s_takingText;
}

/* The depth a Screen Size entry reads (tools/apply_full_colour.py): the mode
 * table's 16 is what the game keeps, but a race is drawn in full colour and,
 * with the game's 32-bit textures taken, in full colour throughout. */
x86::reg32 shownDepth(x86::reg32 depth)
{
    return depth == 16 && win32::glide2x::fullColourTextures() ? 32 : depth;
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

/* A camera's horizontal half field of view, in whole degrees, for the view it
 * draws into (tools/apply_widescreen.py, in sub_4dbce0).
 *
 * The projection takes both angles and works a focal length out of each --
 * half the width over the tangent of the horizontal, half the height over the
 * tangent of the vertical (sub_4fd2b0, through the game's own sine table).
 * Nothing is distorted only while those two come out equal, which asks that the
 * tangents stand in the ratio of the view's sides.  The game's own rule, the
 * vertical angle being 13/16 of the horizontal one in whole degrees, gives that
 * ratio at a single angle and drifts either side of it: about 1.27 at a
 * horizontal half angle of 30 degrees, 1.35 at 45, 1.52 at 60, where a 4:3
 * screen asks for 1.33.  So the original squeezed its picture at close angles
 * and stretched it at wide ones by a few percent -- little enough to pass on a
 * 4:3 monitor, plain enough to see through the wide cinema cameras of a replay
 * or a finish.
 *
 * The vertical angle stays exactly what the game made it, and the horizontal
 * one becomes the one that view's shape asks for.  On a wide screen that is
 * Hor+ -- the vertical view unchanged, more of the world at the sides -- and on
 * any screen it is the game's proportions put right, at every camera angle,
 * which is what a round wheel wants.  The view is the rectangle being drawn
 * into ([0x7cdba0]), so split screen's halves each get their own shape. */
x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half, x86::reg32 vertical)
{
    const x86::sreg32 width = x86::sreg32(app->getMemory<x86::reg32>(0x7cdba0));
    const x86::sreg32 height = x86::sreg32(app->getMemory<x86::reg32>(0x7cdba4));
    const x86::sreg32 down = x86::sreg32(vertical);
    if (width <= 0 || height <= 0 || down <= 0 || down >= 90 || x86::sreg32(half) <= 0 || half >= 90)
        return half;
    const double across = SDL_atan(SDL_tan(double(down) * SDL_PI_D / 180.0) * double(width) / double(height))
                          * 180.0 / SDL_PI_D;
    const x86::sreg32 rounded = x86::sreg32(SDL_lround(across));
    return rounded > 0 && rounded < 90 ? x86::reg32(rounded) : half;
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
/* Per player, per layout -- the racer's and the cop's -- and per element.  The
 * editor names an element by its place in the pair of layouts, 0 to 45, which
 * is the layout and the element within it; the player it is editing for is what
 * its test is handed (hudLayoutsDrawn), and is kept here for the measuring,
 * which is told only the element.  Split screen edits one player's HUD at a
 * time, and a cop and a racer never share a screen, so one set per player is
 * enough -- but the racer's and the cop's own layouts are different HUDs, and
 * measurements of one said nothing about the other. */
static HudDrawn s_hudDrawn[2][2][23];
static x86::reg32 s_hudPlayer = 0;

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
    HudDrawn& drawn = s_hudDrawn[s_hudPlayer][index / 23][index % 23];
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
    /* Said once for each new shape, not for every step of a drag.  The
     * rectangle the game itself drew the element into is said with it: that is
     * what a line of text is centred between (sub_4897f0), and it is the
     * element's layout in pixels (sub_480910) as sub_4800a0 leaves it, not the
     * box the editor moves. */
    const x86::reg32 pixels = 0x749758 + s_hudPlayer * 23 * 16 + (index % 23) * 16;
    if (!drawn.swap || x0 - left != drawn.x0 - drawn.left || y0 - top != drawn.y0 - drawn.top
        || x1 - right != drawn.x1 - drawn.right || y1 - bottom != drawn.y1 - drawn.bottom)
        SDL_Log("[HUDEDIT] element %u drawn at %d,%d-%d,%d in its box %d,%d-%d,%d, the game's %d,%d-%d,%d",
                unsigned(index % 23), x0, y0, x1, y1, int(left), int(top), int(right), int(bottom),
                int(x86::sreg32(app->getMemory<x86::reg32>(pixels))),
                int(x86::sreg32(app->getMemory<x86::reg32>(pixels + 4))),
                int(x86::sreg32(app->getMemory<x86::reg32>(pixels + 8))),
                int(x86::sreg32(app->getMemory<x86::reg32>(pixels + 12))));
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
 * then.  0 for an element not measured, which leaves the game's own rule.
 *
 * What made this shake near the left edge was not the slack but what the game
 * did underneath it: it re-decided every frame whether an element's text was
 * centred in it or put against one of its edges, and near an edge a pixel of
 * movement flipped the answer.  It does not any more -- an element's text is
 * always centred in it (tools/apply_hud_editor.py) -- so what an element draws
 * stands in the same place in its box wherever the box goes, which is what
 * makes this measurement worth anything. */
x86::sreg32 hudDragSlack(win32::WinApplication* app, x86::reg32 object, x86::reg32 side)
{
    const x86::reg32 index = app->getMemory<x86::reg32>(object + 0x3c);
    if (index >= 46 || !hudDrawnFresh(s_hudDrawn[s_hudPlayer][index / 23][index % 23]))
        return 0;
    const HudDrawn& measured = s_hudDrawn[s_hudPlayer][index / 23][index % 23];
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
    if (index >= 46 || !hudDrawnFresh(s_hudDrawn[s_hudPlayer][index / 23][index % 23]))
        return;
    const HudDrawn& measured = s_hudDrawn[s_hudPlayer][index / 23][index % 23];
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

    s_hudPlayer = player < 2 ? player : 0;
    /* Which of the player's two layouts is being edited -- the racer's or the
     * cop's.  The test is handed the element's place within one of them, 0 to
     * 22, while the item it belongs to names both: its index runs 0 to 45, the
     * racer's elements and then the cop's, and the rectangle being tried is
     * that item's own (object + 0x40).  An element measured in one layout says
     * nothing about the other: they are different HUDs. */
    const x86::reg32 named = app->getMemory<x86::reg32>(s_rect - 0x40 + 0x3c);
    const x86::reg32 variant = named < 46 ? named / 23 : 0;
    if (measurable && s_element < kElements && hudDrawnFresh(s_hudDrawn[player][variant][s_element]))
    {
        for (x86::reg32 edge = 0; edge < 4; ++edge)
            s_savedTried[edge] = app->getMemory<float>(s_rect + edge * 4);
        const HudDrawn& measured = s_hudDrawn[player][variant][s_element];
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
            if (measurable && !(layout == variant && element == s_element)
                && hudDrawnFresh(s_hudDrawn[player][layout][element]))
            {
                const HudDrawn& measured = s_hudDrawn[player][layout][element];
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

/* A race's music, played from the track's own file rather than from a copy of
 * it (tools/apply_music_stream.py).  The game copied the whole track -- 7 to 14
 * MB -- into a file of its own at every race start and streamed from that; on a
 * phone that is megabytes written to flash and seconds of loading for nothing.
 *
 * The name is the game's own.  sub_4107c0 spells the track out to open its index
 * file -- the audio directory, the track, rock or tech, and ".map" or ".lin" --
 * and then spells the copy's name over it for the stream.  The first is kept
 * here as the game opens it, and put back in place of the second with ".mus"
 * for its extension: the track beside its index.  It goes into the game's own
 * buffer, which held that very name a moment ago and so has the room for it.
 *
 * Anything unexpected -- no name kept, or one with no extension to replace --
 * leaves the game's own name alone, and the stream finds no file where the copy
 * used to be: a race without music rather than a race with the wrong one. */
static char s_musicIndex[256];

void musicIndexOpened(win32::WinApplication* app, x86::reg32 path)
{
    s_musicIndex[0] = '\0';
    for (size_t i = 0; i + 1 < sizeof(s_musicIndex); ++i)
    {
        const char letter = char(app->getMemory<x86::reg8>(path + x86::reg32(i)));
        s_musicIndex[i] = letter;
        s_musicIndex[i + 1] = '\0';
        if (!letter)
            return;
    }
    /* Longer than any name of the game's: keep none rather than half of one. */
    s_musicIndex[0] = '\0';
}

void musicStreamFile(win32::WinApplication* app, x86::reg32 path)
{
    const size_t length = SDL_strlen(s_musicIndex);
    if (length < 5 || s_musicIndex[length - 4] != '.')
        return;
    SDL_memcpy(s_musicIndex + length - 4, ".mus", 5);
    for (size_t i = 0; i <= length; ++i)
        app->getMemory<x86::reg8>(path + x86::reg32(i)) = x86::reg8(s_musicIndex[i]);
    SDL_Log("[MUSIC] streaming %s", s_musicIndex);
}

/* A new player's settings (tools/apply_first_settings.py).  With no settings
 * file of its own -- data copied straight off the disc has none, the game makes
 * it the first time it runs -- or none it would take, the game starts its
 * settings over (sub_472980, sub_4723f0), and the first time the front end
 * comes up it chooses what suits the machine (sub_472d10, View Distance among
 * it).  Right after that the phone's go over them: NFS3Activity.onFirstSettings
 * lays them on the block the game keeps, 0x20f0 bytes at 0x6fbb40 that are the
 * file word for word, as an import writes them into a settings file that came
 * with the data.  Returns whether it did, for the caller to have the game take
 * up the new bindings and save the file. */
bool firstSettings(win32::WinApplication* app)
{
    bool laid = false;
#ifdef __ANDROID__
    const jsize kSize = 0x20f0;
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = env ? static_cast<jobject>(SDL_GetAndroidActivity()) : nullptr;
    if (activity)
    {
        jclass cls = env->GetObjectClass(activity);
        jmethodID method = env->GetMethodID(cls, "onFirstSettings", "([B)Z");
        jbyteArray config = method ? env->NewByteArray(kSize) : nullptr;
        if (config)
        {
            void* block = &app->getMemory<void>(0x6fbb40);
            env->SetByteArrayRegion(config, 0, kSize, static_cast<const jbyte*>(block));
            laid = env->CallBooleanMethod(activity, method, config) == JNI_TRUE && !env->ExceptionCheck();
            if (laid)
                env->GetByteArrayRegion(config, 0, kSize, static_cast<jbyte*>(block));
            env->DeleteLocalRef(config);
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
            laid = false;
        }
        env->DeleteLocalRef(cls);
        env->DeleteLocalRef(activity);
    }
#else
    (void)app;
#endif
    SDL_Log("[SETTINGS] a new player's settings: %s", laid ? "the phone's, over the game's" : "the game's own");
    return laid;
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
    static x86::reg32 s_raced = 0;
    /* Nothing to do where the on-screen controls are bound as the keyboard they
     * are (NFS_TOUCH_DRIVE): the game sees keys steering the car and raises the
     * flag itself, at the start of the race, where a replay raises it again. */
    static const bool s_keys = []() {
        const char* how = SDL_getenv("NFS_TOUCH_DRIVE");
        return how && SDL_strcmp(how, "keys") == 0;
    }();
    if (s_keys)
        return;
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
    /* A replay is the race driven again from what the game recorded of it, so
     * the car is steered the way it was steered then: the flag keeps the value
     * the race left it at rather than following a screen nobody is touching.
     * ([0x6fd3a0] is 2 while a replay runs, as sub_4bcb00 sets it.) */
    const bool replay = app->getMemory<x86::reg32>(0x6fd3a0) == 2;
    const x86::reg32 digital = replay ? s_raced : x86::reg32(win32::Gamepad::touchSteering() ? 1 : 0);
    if (!replay)
        s_raced = digital;
    if (app->getMemory<x86::reg32>(record + 0x24) != digital)
        app->getMemory<x86::reg32>(record + 0x24) = digital;
    if (s_last != int(digital))
    {
        SDL_Log("[STEERING] player %u: %s%s", unsigned((record - 0x6fd52c) / 0x6c),
                digital ? "the game's keyboard steering (touch buttons)" : "analog (pad)",
                replay ? ", as the replay was recorded" : "");
        s_last = int(digital);
    }
}

bool s_traceCarDetail = false;
bool s_tracePointer = false;
bool s_traceTicks = false;
bool s_traceInput = false;

/* NFS_INPUT_TRACE: what player one's car is being driven with and where it has
 * got to, once every eight ticks of the game's clock -- the same eight a replay
 * is recorded by.  A replay is the race run again from what was recorded of it,
 * so a race and its replay should read the same here: the same numbers in the
 * player's record at the same tick, and the car in the same place.  Where the
 * two part company is what makes a replay a different drive, and which of the
 * two columns parts company first says whether it is the input that differs or
 * what the game does with it.  The record is the one at [car+0x220], six of its
 * words from +0x1c: the gearbox, the steering as given, the flag that says
 * whether the car turns towards it, the pedals.  The place is the car's own
 * (+0x98), which is what the ghost recorder writes down too. */
void traceInput(win32::WinApplication* app)
{
    static x86::reg32 s_step = ~x86::reg32(0);
    if (!nfs3hp::raceIsRunning())
    {
        s_step = ~x86::reg32(0);
        return;
    }
    const x86::reg32 step = app->getMemory<x86::reg32>(0x7d3684) >> 3;
    if (step == s_step)
        return;
    s_step = step;
    const x86::reg32 car = app->getMemory<x86::reg32>(0x678b54);
    if (!car)
        return;
    const x86::reg32 record = app->getMemory<x86::reg32>(car + 0x220);
    if (record < 0x6fd52c || record >= 0x6fd52c + 16 * 0x6c || (record - 0x6fd52c) % 0x6c)
        return;
    SDL_Log("[INPUT] step %u mode %u rec %08x %08x %08x %08x %08x %08x at %.1f %.1f %.1f",
            unsigned(step), unsigned(app->getMemory<x86::reg32>(0x6fd3a0)),
            unsigned(app->getMemory<x86::reg32>(record + 0x1c)),
            unsigned(app->getMemory<x86::reg32>(record + 0x20)),
            unsigned(app->getMemory<x86::reg32>(record + 0x24)),
            unsigned(app->getMemory<x86::reg32>(record + 0x28)),
            unsigned(app->getMemory<x86::reg32>(record + 0x2c)),
            unsigned(app->getMemory<x86::reg32>(record + 0x30)),
            double(app->getMemory<float>(car + 0x98)), double(app->getMemory<float>(car + 0x9c)),
            double(app->getMemory<float>(car + 0xa0)));
}

/* NFS_TICK_TRACE: once a second, the game's own clock against real time and
 * against the frames drawn.  [0x7d3684] is that clock -- zeroed as a race is
 * set up (sub_4c4740) and read all over the game, the replay's recorder among
 * them: sub_477c20 keeps one sample of a car for every eight of these, so how
 * fast this counts is how finely a replay remembers a drive, and how fast a
 * replay plays back.  What the trace says is ticks in the second, frames in the
 * second, and the two divided: if the clock counts frames rather than time,
 * that quotient sits at 1 and everything timed by it follows the frame rate. */
struct TickTrace
{
    x86::reg32 clock = 0;
    x86::reg32 swaps = 0;
    Uint64     at = 0;
};
TickTrace s_ticks;

void traceTicks(win32::WinApplication* app)
{
    const Uint64 now = SDL_GetTicks();
    const x86::reg32 clock = app->getMemory<x86::reg32>(0x7d3684);
    if (!s_ticks.at || clock < s_ticks.clock)
    {
        s_ticks.clock = clock;
        s_ticks.swaps = nfs3hp::s_swaps;
        s_ticks.at = now;
        return;
    }
    const Uint64 span = now - s_ticks.at;
    if (span < 1000)
        return;
    const x86::reg32 ticks = clock - s_ticks.clock;
    const x86::reg32 frames = nfs3hp::s_swaps - s_ticks.swaps;
    SDL_Log("[TICKS] %u in %llu ms (%.1f/s), %u frames (%.1f/s), %.2f ticks a frame, clock %u, mode %u",
            unsigned(ticks), (unsigned long long)span, double(ticks) * 1000.0 / double(span),
            unsigned(frames), double(frames) * 1000.0 / double(span),
            frames ? double(ticks) / double(frames) : 0.0, unsigned(clock),
            unsigned(app->getMemory<x86::reg32>(0x6fd3a0)));
    s_ticks.clock = clock;
    s_ticks.swaps = nfs3hp::s_swaps;
    s_ticks.at = now;
}

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
    int   source;   // kSourceTouch, kSourceMouse, kSourceTouchpad
    int   action;   // a finger: kTouchDown, kTouchMove, kTouchUp
    float x, y;     // window pixels; for the touchpad, how far the finger moved
    int   buttons;  // a mouse: bit 0 left, 1 right, 2 middle; the touchpad: bit 0
};
/* The touchpad is the other way a finger can work the pointer, picked in the
 * launcher (TouchpadPointer.java): the finger moves the cursor by as far as it
 * slides, wherever it is on the screen, rather than putting it under itself --
 * one pixel of the game's cursor for each pixel of the screen, the way it went
 * when it reached the game as a relative mouse, before 0.74.  Its moves add up
 * in the queue, and only the whole pixels go out; the rest waits for the next. */
const int kSourceTouch = 0, kSourceMouse = 1, kSourceTouchpad = 2;
float s_touchpadRestX, s_touchpadRestY;
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

/* The player is driving: a race is up, its menu is not open over it
 * (sub_4bc680 keeps [0x7a3d10] set while it is), and what is on the screen is
 * not a replay.  A replay is a race as far as the game is concerned -- the same
 * loop, the same cars, the same HUD -- but nobody is driving: the player's
 * hands are for the replay's own bar, which is worked with the pointer, and for
 * the camera.  The game says which it is in [0x6fd3a0]: 0 a race, 1 split
 * screen, 2 a replay, as sub_4bcb00 sets it while it plays one back. */
bool playerDriving(win32::WinApplication* app)
{
    return nfs3hp::raceIsRunning() && app->getMemory<x86::reg32>(0x7a3d10) == 0
        && app->getMemory<x86::reg32>(0x6fd3a0) != 2;
}

bool guestNameIs(win32::WinApplication* app, x86::reg32 at, const char* name)
{
    for (x86::reg32 i = 0;; ++i)
    {
        const char c = char(app->getMemory<x86::reg8>(at + i));
        if (c != name[i])
            return false;
        if (!c)
            return true;
    }
}

/* An item of the menu on the screen, by the codelink its menu file names it
 * with, or 0.  The menu showing is the one at [0x559230], which the front end
 * polls every frame (sub_44b230); its items are a run of the front end's own
 * array -- 0xbc bytes each from 0x604ee0 -- that starts at the index in the
 * menu's word at +0x18 and ends at an item whose type (+0) is 0.  That is how
 * the game looks an item up itself (sub_442a40).  Of an item the port reads the
 * flags at +4 (0x1301 keeps the game's own selection off it, sub_449f30), the
 * place its menu file gave it at +6 and +8, in the 640x480 the menus are laid
 * out in, and the codelink at +0x10 -- where the loader's table at 0x557c00
 * puts x, y and codelink. */
x86::reg32 menuItem(win32::WinApplication* app, const char* codelink)
{
    const x86::reg32 menu = app->getMemory<x86::reg32>(0x559230);
    if (!menu)
        return 0;
    const x86::sreg32 first = x86::sreg16(app->getMemory<x86::reg16>(menu + 0x18));
    if (first < 0)
        return 0;
    for (x86::reg32 i = x86::reg32(first); i < x86::reg32(first) + 256; ++i)
    {
        const x86::reg32 item = 0x604ee0 + i * 0xbc;
        if (app->getMemory<x86::reg32>(item) == 0)
            break;
        const x86::reg32 name = app->getMemory<x86::reg32>(item + 0x10);
        if (name && guestNameIs(app, name, codelink))
            return item;
    }
    return 0;
}

/* The player's name on the main menu looks like the field it is typed into,
 * but it is a line of text beside the Player Name tab: a [text] item,
 * SHOW_PLAYERNAME1, next to the [button] SET_PLAYERNAME1 (MAIN.MNU, and the
 * same pair for player two on the split screen's and the records' screens).
 * The game takes no click on text, so a finger that lands on the name presses
 * the tab beside it instead, and the game opens its name box as it does for
 * the tab -- and with the box comes the phone's keyboard (textEntryTick).
 *
 * The name's line runs from where the text starts to past the ten letters a
 * name can have (sub_45cef0 allows ten), and is as tall as the row of tabs it
 * sits in, which stand 35 apart; the tab is pressed a little way in from its
 * corner.  Where the name stands, the menu files also put the opponent choice
 * and the assists line, but those are the ones shown when the name is not; the
 * tab being one the game would let the player choose says which it is.  No
 * other box may be up already. */
bool nameTapTarget(win32::WinApplication* app, x86::sreg32 x, x86::sreg32 y,
                   x86::sreg32& tabX, x86::sreg32& tabY)
{
    if (nfs3hp::raceIsRunning() || app->getMemory<x86::reg8>(0x558b10) != 0)
        return false;
    static const char* const kTabs[] = { "SET_PLAYERNAME1", "SET_PLAYERNAME2" };
    static const char* const kNames[] = { "SHOW_PLAYERNAME1", "SHOW_PLAYERNAME2" };
    for (int player = 0; player < 2; ++player)
    {
        const x86::reg32 tab = menuItem(app, kTabs[player]);
        const x86::reg32 name = menuItem(app, kNames[player]);
        if (!tab || !name || (app->getMemory<x86::reg16>(tab + 4) & 0x1301))
            continue;
        const x86::sreg32 atX = x86::sreg16(app->getMemory<x86::reg16>(tab + 6));
        const x86::sreg32 atY = x86::sreg16(app->getMemory<x86::reg16>(tab + 8));
        const x86::sreg32 nameX = x86::sreg16(app->getMemory<x86::reg16>(name + 6));
        if (x >= nameX - 8 && x < nameX + 200 && y >= atY - 2 && y < atY + 32)
        {
            tabX = atX + 24;
            tabY = atY + 12;
            return true;
        }
    }
    return false;
}

/* A finger that went down on a player's name, and the tab it presses instead,
 * until the finger comes up. */
bool        s_nameTab;
x86::sreg32 s_nameTabX, s_nameTabY;

/* NFS3Activity's say in something, by the name of its method taking a boolean;
 * from the game thread, which SDL has attached to the VM.  False while there
 * is no activity to tell yet. */
bool tellActivity(const char* method, bool value)
{
#ifdef __ANDROID__
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity)
        return false;
    jclass cls = env->GetObjectClass(activity);
    jmethodID id = env->GetMethodID(cls, method, "(Z)V");
    if (id)
        env->CallVoidMethod(activity, id, jboolean(value ? JNI_TRUE : JNI_FALSE));
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#else
    (void)method;
    (void)value;
#endif
    return true;
}

bool frontEndTakingText(win32::WinApplication* app);

/* The line the game's name box types into, in the 640x480 its menus are laid
 * out in, as sub_444350 draws its frame.  The box itself is four floats at
 * 0x661050 -- top, left, bottom, right -- that sub_444c20 fits around its text
 * as it opens it; the line is centred across it, as wide as [0x661070] (the
 * widest letter times the letters a name can have, and the cursor, 240 at
 * most), and 20 tall from one above the text, which sits 90 above the box's
 * bottom (the -30 and -60 at 0x538b08).  A finger is no cursor, so the line
 * reaches a little way beyond itself -- short of the prompt above it and of
 * Ok and Cancel below. */
bool textFieldRect(win32::WinApplication* app, x86::sreg32& left, x86::sreg32& top,
                   x86::sreg32& right, x86::sreg32& bottom)
{
    const float boxLeft = app->getMemory<float>(0x661054), boxRight = app->getMemory<float>(0x66105c);
    const float boxBottom = app->getMemory<float>(0x661058);
    const x86::sreg32 width = x86::sreg32(app->getMemory<x86::reg32>(0x661070));
    if (!(boxRight > boxLeft) || width <= 0 || width > 640)
        return false;
    left = x86::sreg32(boxLeft) + (x86::sreg32(boxRight - boxLeft) - width) / 2;
    right = left + width;
    top = x86::sreg32(boxBottom - 90) - 1;
    bottom = top + 20;
    return true;
}

bool inTextField(win32::WinApplication* app, x86::sreg32 x, x86::sreg32 y)
{
    x86::sreg32 left, top, right, bottom;
    return textFieldRect(app, left, top, right, bottom)
        && x >= left - 10 && x <= right + 10 && y >= top - 10 && y <= bottom + 10;
}

/* A press on the picture while the name box is up: on its line the phone's
 * keyboard comes up, anywhere else it goes away (NFS3Activity.onTextFieldTap).
 * The press goes on to the game either way. */
void textFieldPress(win32::WinApplication* app, x86::sreg32 x, x86::sreg32 y)
{
    if (!frontEndTakingText(app))
        return;
    const bool inside = inTextField(app, x, y);
    SDL_Log("[TEXT] press at %d,%d %s the name's line", int(x), int(y), inside ? "on" : "off");
    tellActivity("onTextFieldTap", inside);
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
    if (driving)
        s_nameTab = false;
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
        if (input.source == kSourceTouchpad)
        {
            if (driving)
                continue;
            s_touchpadRestX += input.x;
            s_touchpadRestY += input.y;
            const x86::sreg32 moveX = x86::sreg32(s_touchpadRestX), moveY = x86::sreg32(s_touchpadRestY);
            s_touchpadRestX -= float(moveX);
            s_touchpadRestY -= float(moveY);
            const x86::sreg32 toX = clampAxis(atX + moveX, lowX, highX), toY = clampAxis(atY + moveY, lowY, highY);
            win32::Mouse::movePointer(toX - atX, toY - atY);
            atX = toX;
            atY = toY;
            const bool down = (input.buttons & 1) != 0;
            if (s_tracePointer)
                SDL_Log("[POINTER] touchpad %+.0f,%+.0f button %s -> cursor %d,%d of %dx%d", input.x, input.y,
                        down ? "down" : "up", int(toX), int(toY), int(highX), int(highY));
            if (down && !s_touchHeld)
            {
                textFieldPress(app, toX, toY);
                win32::Mouse::pressPointer(0, true);
                s_touchHeld = true;
                ++taken;
                break;  // as for a finger: the release waits for the next frame
            }
            if (!down && s_touchHeld)
            {
                win32::Mouse::pressPointer(0, false);
                s_touchHeld = false;
            }
            continue;
        }
        const bool touch = input.source == kSourceTouch;
        if (touch && driving)
            continue;
        x86::sreg32 toX = clampAxis(
            lowX + x86::sreg32(SDL_lround(double(input.x - rectX) * double(highX - lowX) / double(rectW))), lowX, highX);
        x86::sreg32 toY = clampAxis(
            lowY + x86::sreg32(SDL_lround(double(input.y - rectY) * double(highY - lowY) / double(rectH))), lowY, highY);
        if (touch && input.action == kTouchDown && !s_touchHeld)
        {
            textFieldPress(app, toX, toY);
            s_nameTab = nameTapTarget(app, toX, toY, s_nameTabX, s_nameTabY);
            if (s_nameTab)
                SDL_Log("[POINTER] finger on the player's name at %d,%d: the Player Name tab is pressed at %d,%d",
                        int(toX), int(toY), int(s_nameTabX), int(s_nameTabY));
        }
        if (touch && s_nameTab)
        {
            toX = s_nameTabX;
            toY = s_nameTabY;
        }
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
            if (input.action == kTouchUp)
                s_nameTab = false;
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
    if (input.source == kSourceTouchpad && s_pointerCount > 0)
    {
        /* The touchpad counts how far, so its moves add up instead. */
        PointerInput& last = s_pointerQueue[s_pointerCount - 1];
        if (last.source == kSourceTouchpad && last.buttons == input.buttons)
        {
            last.x += input.x;
            last.y += input.y;
            SDL_UnlockMutex(s_touchMutex);
            return;
        }
    }
    else if (s_pointerCount > 0)
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

/* How far the finger slid on the touchpad, in window pixels, and whether it
 * holds the button. */
void queueTouchpad(float dx, float dy, int buttons)
{
    PointerInput input = {};
    input.source = kSourceTouchpad;
    input.action = kTouchMove;
    input.x = dx;
    input.y = dy;
    input.buttons = buttons & 1;
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
    /* The pads follow the same signal as the on-screen controls: their buttons
     * are the racing ones while a race is being driven, and the menu's -- one
     * to confirm, one to go back, the D-pad to move -- everywhere else, a
     * replay included.  With the menu open over a race the button that opened
     * it closes it too (Gamepad::Context::Pause). */
    const bool paused = nfs3hp::raceIsRunning() && app->getMemory<x86::reg32>(0x7a3d10) != 0;
    win32::Gamepad::context(driving ? win32::Gamepad::Context::Race
                            : paused ? win32::Gamepad::Context::Pause : win32::Gamepad::Context::Menu);
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

/* Which of the racing layout's buttons player one's car has any use for: the
 * gears only with a manual gearbox, the spike strip only in a police car.  The
 * car is the one the game's first view follows (sub_422bd0: [[0x5e10b0]+8]).
 * Its gearbox is its player's record's at +0x1c -- 1 automatic, 0 manual, the
 * car screen's Transmission list ("trans", 0x5561da) as kept for player one at
 * [0x6fd2d4], copied into the record for the race (0x49ad7c) and read by the
 * car when it shifts (0x43149a).  A police car carries bit 0x20 of its flags
 * byte at +0x200: the bit the game tells the siren from the horn by (0x43154e)
 * and picks the pause screen by (0x44d4fb). */
void raceControlsTick(win32::WinApplication* app)
{
    static int s_last = -1;
    if (!nfs3hp::raceIsRunning())
    {
        s_last = -1;
        return;
    }
    const x86::reg32 view = app->getMemory<x86::reg32>(0x5e10b0);
    const x86::reg32 car = view ? x86::reg32(app->getMemory<x86::reg32>(view + 8)) : 0;
    if (!car)
        return;
    const x86::reg32 record = app->getMemory<x86::reg32>(car + 0x220);
    if (record < 0x6fd52c || record >= 0x6fd52c + 16 * 0x6c || (record - 0x6fd52c) % 0x6c)
        return;
    const bool gears = app->getMemory<x86::reg32>(record + 0x1c) != 1;
    const bool spikes = (app->getMemory<x86::reg8>(car + 0x200) & 0x20) != 0;
    const int state = int(gears) | int(spikes) << 1;
    if (state == s_last)
        return;
#ifdef __ANDROID__
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity)
        return;  // asked again next frame
    jclass cls = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(cls, "onRaceControls", "(ZZ)V");
    if (method)
        env->CallVoidMethod(activity, method, jboolean(gears ? JNI_TRUE : JNI_FALSE),
                            jboolean(spikes ? JNI_TRUE : JNI_FALSE));
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#endif
    SDL_Log("[CONTROLS] gears %s, spike strip %s", gears ? "shown (manual)" : "hidden (automatic)",
            spikes ? "shown (police car)" : "hidden");
    s_last = state;
}

/* The phone's keyboard, raised by the game rather than by a button.
 *
 * The game puts up a little box to take a name -- a line of text with Ok and
 * Cancel -- and nothing in it takes focus: on a keyboard you simply type.  So
 * the box itself is the signal that typing is wanted, and where the keyboard
 * comes up from is the player's hands: from a pad at once, from a finger with
 * a tap on the line the box keeps (textFieldPress), never beside a real
 * keyboard (NFS3Activity).  Whichever way, the keyboard goes when the box
 * does, and a race lowers it whatever else is true -- nothing types while
 * driving, and a keyboard left up over one would be a keyboard nobody could
 * put away.  The car screen's own box (tools/apply_text_entry.py) counts as
 * typing too.
 *
 * Android is asked on its own thread, which is where SDL's text input has to be
 * started and stopped from (nativeToggleKeyboard). */
/* Whether the front end has its string box up -- "Please enter your name", a
 * line to type in and Ok and Cancel under it.
 *
 * Every screen that asks for a string asks through one function, sub_444c20:
 * it is handed the buffer, how long a string may be and what to call
 * afterwards, and it puts the box up.  What it sets is what this reads: a byte
 * at [0x558b10] that says a box of some kind is up, and [0x558b14] for which
 * kind -- 2 and 4 are the boxes that only ask a question, 3 is the one that
 * takes typing.  So the answer holds wherever the box was opened from: the
 * player's name in the race setup, the same name on the car screen, anything
 * else that asks for a word.  The items themselves were the wrong place to look
 * -- each screen hands its own handler out, and the first two the port watched
 * were the car screen's, which a player who renames themselves from the main
 * menu never touches. */
bool frontEndTakingText(win32::WinApplication* app)
{
    return app->getMemory<x86::reg8>(0x558b10) != 0
        && app->getMemory<x86::reg32>(0x558b14) == 3;
}

void textEntryTick(win32::WinApplication* app)
{
    static int last = -1;
    const bool racing = nfs3hp::raceIsRunning();
    const int typing = !racing && (nfs3hp::takingText() || frontEndTakingText(app)) ? 1 : 0;
    if (typing == last)
        return;
    last = typing;
    /* Whether the keyboard comes up now is NFS3Activity's to say, by what the
     * box was opened with: at once from a pad, otherwise with a tap on the
     * line (textFieldPress). */
    if (!tellActivity("onTextEntry", typing != 0))
    {
        last = -1;  // asked again next frame
        return;
    }
    x86::sreg32 left, top, right, bottom;
    if (typing && frontEndTakingText(app) && textFieldRect(app, left, top, right, bottom))
        SDL_Log("[TEXT] a name is asked for; its line runs %d..%d across, %d..%d down",
                int(left), int(right), int(top), int(bottom));
    else
        SDL_Log("[TEXT] %s", typing ? "a name is asked for" : "typing over: keyboard down");
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
 * controls should be showing and which of the race's buttons, whether the pads'
 * keys go out, the phone's cornering, how the touch buttons steer, and the car
 * detail trace when asked for (NFS_CAR_DETAIL_TRACE). */
void onSwap(win32::WinApplication* app)
{
    ++nfs3hp::s_swaps;
#ifndef __ANDROID__
    scriptTick();
#endif
    pointerTick(app);
    layoutTick(app);
    raceControlsTick(app);
    textEntryTick(app);
    padKeysTick(app);
    phoneTick(app);
    steeringTick(app);
    if (s_traceCarDetail)
        traceCarDetail(app);
    if (s_traceTicks)
        traceTicks(app);
    if (s_traceInput)
        traceInput(app);
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
        const char* tickTrace = SDL_getenv("NFS_TICK_TRACE");
        s_traceTicks = tickTrace && *tickTrace && *tickTrace != '0';
        const char* inputTrace = SDL_getenv("NFS_INPUT_TRACE");
        s_traceInput = inputTrace && *inputTrace && *inputTrace != '0';
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
/* A finger on the touchpad (TouchpadPointer.java): how far it slid, in window
 * pixels, and whether it holds the button, bit 0.  Called on the Android UI
 * thread. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeTouchpad(JNIEnv*, jclass, jfloat dx, jfloat dy, jint buttons)
{
    queueTouchpad(float(dx), float(dy), int(buttons));
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
