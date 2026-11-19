#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <lib/settings.h>
#include <lib/gamepad.h>
#include <lib/memmap.h>
#include <lib/timer.h>
#include <nfs3hp.h>
#include "os_system.h"
#include <lib/thrashrenderer.h>
#include "game_settings.h"
#include "native_thrash.h"
#include "port_thrash.h"
#include "dynamic_fullscreen.h"
#include "func_log.h"
#include "diag_hooks.h"
#include "replay_trace.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#include <fcntl.h>
#include <jni.h>
#include <unistd.h>
#endif

using nfs3hp::gameSettings;

namespace nfs3hp
{
// menu_data.cpp: the port's versions of menu files, read in place of the game's.
void installMenuData();
// port_display.cpp: gamma, brightness and contrast on the game's graphics screens.
void installDisplaySliders(nfs::App* app);
// port_connect.cpp: the TCP/IP port and this phone's address on the connection screen.
void installConnectScreen(nfs::App* app);

/* The MAD player runs on the game thread. Android's UI thread only requests
 * that its normal cleanup path run at the next frame boundary. */
static std::atomic_bool s_moviePlaying{false};
static std::atomic_bool s_movieSkip{false};

void moviePlaying(bool playing)
{
    s_movieSkip.store(false, std::memory_order_release);
    s_moviePlaying.store(playing, std::memory_order_release);
}

/* NFS_AUTO_REPLAY=1 (tools/replay_run.py): no movies, and from the first menu
 * straight to the last race's replay, once. */
static bool autoReplayAsked()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_AUTO_REPLAY");
        return value && *value && *value != '0';
    }();
    return on;
}

static bool takeAutoReplay()
{
    static bool s_taken = false;
    if (s_taken || !autoReplayAsked())
        return false;
    s_taken = true;
    return true;
}

bool movieSkipRequested()
{
    return autoReplayAsked() || s_movieSkip.load(std::memory_order_acquire);
}

static bool requestMovieSkip()
{
    if (!s_moviePlaying.load(std::memory_order_acquire)) return false;
    s_movieSkip.store(true, std::memory_order_release);
    return true;
}

/* The TCP/IP port a race is hosted and joined on (the port's network port change):
 * NFS_NET_PORT to start with, 9803 as the Modern Patch has it unless the player
 * chose 1030, the original's, and the connection screen moves it from there
 * (port_connect.cpp).  Every player of one race needs the same. */
static std::atomic<nfs::u32>& networkPortChosen()
{
    static std::atomic<nfs::u32> port{ []() {
        const char* value = SDL_getenv("NFS_NET_PORT");
        const int chosen = value ? SDL_atoi(value) : 0;
        return nfs::u32(chosen > 0 && chosen < 65536 ? chosen : 9803);
    }() };
    return port;
}

nfs::u32 networkPort()
{
    return networkPortChosen().load(std::memory_order_relaxed);
}

void setNetworkPort(nfs::u32 port)
{
    networkPortChosen().store(port, std::memory_order_relaxed);
}

/* Whether the player is driving.  A race is one call of the game's own -- set
 * up by sub_4a3400, taken down again by sub_4a35d0, each called from one place
 * only -- and both are marked (the port's race state change the first, the
 * native graphics::takeDownRace the second), so this follows it exactly
 * rather than guessing from the screen or from what the car is doing. */
static bool s_raceRunning = false;
/* The race's loading screen is up: the race is running, but nobody drives it
 * yet, so the touch controls stay hidden (layoutTick). */
static bool s_loadingScreen = false;

void raceRunning(bool running)
{
    s_raceRunning = running;
    /* A load cut short leaves the race without finishing its setup, and so
     * without the end of the loading screen (loadingScreenFit). */
    if (!running)
    {
        nfs3hp::port::driver::fitFourThree(false);
        s_loadingScreen = false;
    }
}

/* The race taken down: the replay trace closed. */
static void raceDownStep(nfs::App* app)
{
    endReplayTrace(app);
}

/* The loading screen at 4:3 in the middle of a wide screen, as the Modern Patch
 * shows it (the port's loading screen change): from the start of the loading
 * picture, the whole picture black and everything drawn squeezed into the 4:3
 * rectangle in its middle; once the race is loaded, the whole screen again. */
void loadingScreenFit(nfs::App* app, nfs::ThreadState& ts, bool on)
{
    if (on)
        nfs3hp::port::driver::clearPicture(app, ts);
    nfs3hp::port::driver::fitFourThree(on);
    s_loadingScreen = on;
}

bool loadingScreenUp()
{
    return s_loadingScreen;
}

/* The NFS emblem of a dialog box over a race -- the pause menu's "Quit to:" and
 * the like (the port's widescreen change, around its draw in sub_4438d0).  The
 * game lays the box out for 640x480 and stretches it over the whole picture,
 * which the box and its buttons bear; the round emblem comes out an oval.  So
 * for its one draw each triangle narrows by the 4:3 factor towards its own left
 * edge, and the emblem is round again where it stands, at the box's corner --
 * the box as the game stretches it, as the Modern Patch has it. */
void dialogEmblem(bool drawing)
{
    nfs3hp::port::driver::squeezeToLeft(drawing);
}

bool raceIsRunning()
{
    return s_raceRunning;
}

/* Whether the game is waiting for a name to be typed: the dialog that takes one
 * is up (the port's text entry change).  Read once a frame by textEntryTick, which
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

/* The depth a Screen Size entry reads (the port's full colour change): the mode
 * table's 16 is what the game keeps, but a race is drawn in full colour and,
 * with the game's 32-bit textures taken, in full colour throughout. */
nfs::u32 shownDepth(nfs::u32 depth)
{
    return depth == 16 ? 32 : depth;
}

/* View Distance at Full: the player's choice in the Graphics menu, [0x6fbc28],
 * Full to Close as 0 to 3.  At Full the game already draws the track at full
 * detail out to its far distance; the port's track detail change has the ones below
 * take that distance to 500 wherever the game held it shorter, and past it. */
static bool viewDistanceFull(nfs::App* app)
{
    return gameSettings(app).viewDistance == 0;
}

/* Which table of multipliers sub_41d620 scales the distances by: 0x41a8bc, whose
 * widest row leaves the far distance whole, or 0x41a8b0, whose widest row keeps
 * three quarters of it.  `reduced` is the game's own choice, [0x6fd4c8]; at Full
 * the distance is whole. */
nfs::u32 viewDistanceReduced(nfs::App* app, nfs::u32 reduced)
{
    return viewDistanceFull(app) ? 0u : reduced;
}

/* How far View Distance Full reaches, in 16.16: past the game's 500 (and the
 * Modern Patch's), because a wide screen shows more of the track at its sides,
 * where the blocks coming into view were out of a 4:3 picture.  The track's
 * own lists of the blocks seen from each still bound it (2026-10-22: 600 on
 * trial, better; 800 no better than 600 until the track lists grew
 * (native_loading.cpp, extendNeighbours, which reaches as far); with them 700). */
constexpr nfs::u32 kFullFarDistance = nfs::u32(700) << 16;

/* The far distance of each split-screen view, in 16.16: the game's 440, or at
 * Full the reach a single view has. */
nfs::u32 splitFarDistance(nfs::App* app, nfs::u32 distance)
{
    return viewDistanceFull(app) ? kFullFarDistance : distance;
}

/* A view's 500 -- the main view's far distance, the cull distance of the main
 * and split views -- in 16.16, or at Full kFullFarDistance. */
nfs::u32 fullFarDistance(nfs::App* app, nfs::u32 distance)
{
    return viewDistanceFull(app) ? kFullFarDistance : distance;
}

/* The rear-view mirror at the main view's distances.  A single-screen race has
 * two views (sub_41d960): the main one from sub_41df10's first template, the
 * mirror from its fourth -- polygons culled beyond 150, the far distance 100,
 * the track's detailed model only to 40 and its middle one to 80, six blocks
 * of track -- so the mirror showed little behind the car, and the low model's
 * holes.  sub_41d620 works out the main view's distances from View Distance;
 * here, before it turns them into floats for every view, the mirror takes the
 * main view's whole set, +0x20 to +0x4c of its 84-byte record (the camera's
 * own record at +0x1c stays).  Split screen has no mirror and is left alone. */
void mirrorFollowsMain(nfs::App* app)
{
    if (gameSettings(app).playMode == 1 || app->getMemory<nfs::u32>(0x5dd0dc) != 2)
        return;
    const nfs::u32 mainView = 0x5dd0e0;
    const nfs::u32 mirror = mainView + 0x54;
    for (nfs::u32 at = 0x20; at < 0x50; at += 4)
        app->getMemory<nfs::u32>(mirror + at) = app->getMemory<nfs::u32>(mainView + at);
}

/* A view pass's kind ([0x7a3d14], 1 for the mirror) where the game asks it to
 * leave something out of the mirror: the main view's 0, the mirror drawn in
 * full (the port's mirror detail change). */
nfs::u32 mirrorAsMain(nfs::u32 kind)
{
    return kind == 1 ? 0 : kind;
}

/* Wheel spin, per car.  The game turns a car's wheels by the frames elapsed
 * since one stamp shared by every car and every view (0x55fe28), written as
 * each view pass ends, so a car first drawn in a later pass of the same frame
 * -- the second split-screen view, the rear-view mirror -- saw no time pass and
 * its wheels stood still.  the port's car detail change routes the three reads of
 * that stamp here: sub_4b9140 and sub_4b9280 for the two wheel angles, and
 * sub_4b90e0, two ways, for the body.  Each keeps its own stamp per car, so a
 * car still advances once per frame, whichever view draws it first. */
nfs::u32 carWheelStamp(nfs::App* app, nfs::u32 car, nfs::u32 which)
{
    static std::unordered_map<nfs::u32, std::array<nfs::u32, 4>> s_stamps;
    const nfs::u32 frame = app->getMemory<nfs::u32>(0x7d3684);
    auto entry = s_stamps.try_emplace(car);
    nfs::u32& stamp = entry.first->second[which & 3];
    nfs::u32 last = entry.second ? app->getMemory<nfs::u32>(0x55fe28) : stamp;
    /* A stamp from an earlier race -- the frame count starts over and car
     * records are reused -- would freeze the wheels until the count caught up,
     * and one from long ago would spin them through hundreds of turns at once. */
    if (last > frame || frame - last > 4)
        last = frame - 1;
    stamp = frame;
    return last;
}

/* Dynamic Fullscreen: a race on the whole screen, whatever its proportions --
 * 16:9, 18:9, 19.5:9, 20:9, a tablet's 16:10 or 4:3, a foldable open or shut.
 * One mode of the Voodoo2 driver's table (kDynamicEntry, the last, so the
 * Graphics menu lists it last) is the port's: its size is worked out from the
 * screen, the height the screen's own up to 1080 and the width the screen's
 * proportion of it, and the driver opens it under kResolutionDynamic.  Nothing of
 * the game's art is redrawn for it: every camera already widens its view to
 * the screen's shape (widescreenHalfAngle) and the HUD keeps its proportions
 * (widescreenHudRect).
 *
 * The size is measured again as each race opens its mode (screenModeNative,
 * sub_4bef50), so a foldable opened or shut between races gets its new shape
 * at the next one; during a race the picture keeps its shape and the
 * presenter puts it between bars.  The Graphics menu shows the mode as
 * "Dynamic Fullscreen", and the settings keep kDynamicFullscreenWidth for it
 * in place of a width, which sub_4bee80 (the port's widescreen change) turns
 * back into the mode at start, on any screen. */
constexpr nfs::u32 kModeTable = 0xa92018;       // voodoo2a's modes, 40 bytes each
constexpr nfs::u32 kModeResolutions = 0xa922e8; // each mode's resolution id
constexpr nfs::u32 kDynamicEntry = 16;
constexpr nfs::u32 kDynamicMaxHeight = 1080;
static nfs::u32 s_dynamicWidth = 0;
static nfs::u32 s_dynamicHeight = 0;

/* The size of the screen the game is drawn on, in pixels: its window's, or
 * before there is one, the display's. */
static bool screenPixels(int& w, int& h)
{
    w = h = 0;
    int count = 0;
    if (SDL_Window** windows = SDL_GetWindows(&count))
    {
        if (count > 0)
            SDL_GetWindowSizeInPixels(windows[0], &w, &h);
        SDL_free(windows);
    }
    if (w <= 0 || h <= 0)
    {
        if (const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()))
        {
            w = mode->w;
            h = mode->h;
        }
    }
    return w > 0 && h > 0;
}

/* Dynamic Fullscreen's size for the screen as it is now, landscape: the
 * screen's height up to 1080, the width in its proportion, a multiple of 8
 * and no narrower than 4:3. */
static void dynamicFullscreenSize(nfs::u32& width, nfs::u32& height)
{
    int w = 0, h = 0;
    if (!screenPixels(w, h))
    {
        width = 1920;
        height = 1080;
        return;
    }
    const nfs::u32 across = nfs::u32(SDL_max(w, h)), down = nfs::u32(SDL_min(w, h));
    height = SDL_min(down, kDynamicMaxHeight) & ~1u;
    width = ((across * height / down + 4) / 8) * 8;
    width = SDL_max(width, (height * 4 / 3 + 7) / 8 * 8);
}

bool dynamicFullscreenMode(nfs::u32 mode)
{
    return mode == kDynamicEntry && s_dynamicWidth != 0;
}

/* Dynamic Fullscreen's mode measured again and put in the driver's table and
 * its size for the id.  True when its size changed. */
bool dynamicFullscreenRefresh(nfs::App* app)
{
    if (s_dynamicWidth == 0)
        return false;
    nfs::u32 width, height;
    dynamicFullscreenSize(width, height);
    if (width == s_dynamicWidth && height == s_dynamicHeight)
        return false;
    SDL_Log("[SCREEN] Dynamic Fullscreen %ux%u -> %ux%u", unsigned(s_dynamicWidth), unsigned(s_dynamicHeight),
            unsigned(width), unsigned(height));
    s_dynamicWidth = width;
    s_dynamicHeight = height;
    app->getMemory<nfs::u32>(kModeTable + kDynamicEntry * 40) = width;
    app->getMemory<nfs::u32>(kModeTable + kDynamicEntry * 40 + 4) = height;
    nfs3hp::port::driver::setDynamicResolution(width, height);
    return true;
}

/* View Angle ([0x6fbc40]) in the Graphics menu: Normal, 0 -- the game's Wide,
 * 90 degrees -- Narrow, 1, 75, and Wide, 2, 105, the Modern Patch's widest
 * (the port's View Angle change). */
constexpr nfs::u32 kViewAngleList = 0x55675c;   // the settings record's (0x556758) list of its choices
constexpr nfs::u16 kNormalText = 0xaf, kNarrowText = 0x113, kWideText = 0x112;

nfs::u32 viewAngleFor(nfs::App* app, nfs::u32 angle)
{
    return gameSettings(app).viewAngle == 2 ? 105u : angle;
}

/* The menu's three choices, a list of text lines ending at a zero, in a guest
 * block of the port's own (the game's list has no room for a third). */
void viewAngleStart(nfs::App* app)
{
    static nfs::MemMap* const block = new nfs::MemMap(nfs::MemMap::granularity());
    const nfs::u32 list = block->getBlockStart();
    const nfs::u16 lines[] = {kNormalText, kNarrowText, kWideText, 0};
    for (nfs::u32 i = 0; i < 4; ++i)
        app->getMemory<nfs::u16>(list + i * 2) = lines[i];
    app->getMemory<nfs::u32>(kViewAngleList) = list;
}

/* Dynamic Fullscreen put in the driver's table at start (main), measured
 * against the display. */
void dynamicFullscreenStart(nfs::App* app)
{
    nfs::u32 width, height;
    dynamicFullscreenSize(width, height);
    s_dynamicWidth = width;
    s_dynamicHeight = height;
    const nfs::u32 mode = kModeTable + kDynamicEntry * 40;
    app->getMemory<nfs::u32>(mode) = width;
    app->getMemory<nfs::u32>(mode + 4) = height;
    app->getMemory<nfs::u32>(mode + 16) = 1;
    app->getMemory<nfs::u32>(kModeResolutions + kDynamicEntry * 4) = nfs3hp::port::driver::kResolutionDynamic;
    nfs3hp::port::driver::setDynamicResolution(width, height);
    SDL_Log("[SCREEN] Dynamic Fullscreen %ux%u", unsigned(width), unsigned(height));
}

/* The Graphics menu's line for a mode as sub_448210 has just written it (a
 * list entry: the mode at +0, its line from +0x14 on, 0x100 bytes of room):
 * Dynamic Fullscreen's by its name (the port's widescreen change, 0x448291). */
void dynamicFullscreenLabel(nfs::App* app, nfs::u32 entry)
{
    if (!dynamicFullscreenMode(app->getMemory<nfs::u32>(entry)))
        return;
    char* line = &app->getMemory<char>(entry + 0x14);
    SDL_strlcpy(line, kDynamicFullscreenName, 0x100);
}

/* sub_4bee80's answer for the saved screen size (the port's widescreen change, at
 * 0x4bef35): Dynamic Fullscreen's mode where the settings keep its marker,
 * the nearest mode of that width otherwise. */
nfs::u32 dynamicSavedMode(nfs::u32 found, nfs::u32 width)
{
    return width == kDynamicFullscreenWidth && s_dynamicWidth != 0 ? kDynamicEntry : found;
}

/* Screen sizes the Graphics menu may list.  sub_4bed90 passes the menu only
 * driver modes whose width is exactly 4/3 of their height, and
 * the port's widescreen change has it ask here too: an exact 16:9 is taken as
 * well, which is the 1280x720 main() adds to the Voodoo2 driver's table, and
 * Dynamic Fullscreen's size, whatever its shape.  The driver's own 856x480 is
 * not quite 16:9 and stays unlisted. */
bool widescreenMode(nfs::u32 width, nfs::u32 height)
{
    return width * 9 == height * 16 || (s_dynamicWidth != 0 && width == s_dynamicWidth && height == s_dynamicHeight);
}

/* Wide Screen in the Graphics menu ([0x6fbc4c]) has a race of one screen drawn
 * between bars: a view less high than the screen ([0x7cdba4] against
 * [0x7cdae4]). */
static bool letterboxed(nfs::App* app)
{
    return raceIsRunning() && gameSettings(app).wideScreen != 0 && gameSettings(app).playMode != 1
           && nfs::s32(app->getMemory<nfs::u32>(0x7cdba4)) < nfs::s32(app->getMemory<nfs::u32>(0x7cdae4));
}

/* A camera's horizontal half field of view, in whole degrees, for the view it
 * draws into (the port's widescreen change, in sub_4dbce0).
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
nfs::u32 widescreenHalfAngle(nfs::App* app, nfs::u32 half, nfs::u32 vertical)
{
    /* Outside a race the view rectangle is stale: a race leaves its own there
     * (1920x1080 on a 1080p race) and the menus never set it again, so the
     * Player Car screen, drawn at 640x480, got Hor+ after the first race --
     * even one quit on its loading screen.  There the screen is the view. */
    const nfs::u32 at = raceIsRunning() ? 0x7cdba0 : 0x7cdae0;
    const nfs::s32 width = nfs::s32(app->getMemory<nfs::u32>(at));
    /* Behind Wide Screen's bars the game projects through the whole screen's
     * height (sub_4beb00's rectangle) and draws only into the bars' (sub_4beb40's,
     * [0x7cdb98]): the angle is the whole screen's. */
    const nfs::s32 height = nfs::s32(app->getMemory<nfs::u32>(letterboxed(app) ? 0x7cdae4 : at + 4));
    const nfs::s32 down = nfs::s32(vertical);
    if (width <= 0 || height <= 0 || down <= 0 || down >= 90 || nfs::s32(half) <= 0 || half >= 90)
        return half;
    const double across = SDL_atan(SDL_tan(double(down) * SDL_PI_D / 180.0) * double(width) / double(height))
                          * 180.0 / SDL_PI_D;
    const nfs::s32 rounded = nfs::s32(SDL_lround(across));
    return rounded > 0 && rounded < 90 ? nfs::u32(rounded) : half;
}

/* Wide Screen's view height (the port's widescreen change, in sub_4be5e0): the
 * game's half of the width -- a 2:1 view -- on a screen narrower than 2:1, where
 * that leaves bars above and below; on a wider one, where 2:1 was taller than
 * the screen, kLetterbox's -- 2.76:1, Ultra Panavision's. */
constexpr double kLetterbox = 2.76;

nfs::u32 letterboxHeight(nfs::u32 width, nfs::u32 height, nfs::u32 half)
{
    if (nfs::s32(width) <= 0 || nfs::s32(height) <= 0 || width < height * 2)
        return half;
    return nfs::u32(SDL_min(nfs::s32(height), nfs::s32(SDL_lround(double(width) / kLetterbox))));
}

/* The half width a zoomed camera's x scale is made of (the port's widescreen change,
 * in sub_4dbde0).  The replay's and the finish's television cameras set their
 * projection from their zoom rather than an angle: x scale half the view's width
 * times the zoom, y scale half its height times the zoom and times 1.34834
 * ([0x54972c]), the game's 4:3.  Those agree on a 4:3 view; on a 16:9 one the y
 * scale is a quarter short and the cars come out squashed.  On a view wider than
 * 4:3 the x scale takes the y scale's size, half the height times 1.34834, so
 * pixels are square again, the vertical view is the game's and more of the
 * world shows at the sides.  Only an active race gets Hor+; a 4:3 race keeps
 * the game's half width.  (The Player Car screen does not come through here, as
 * a log showed: it takes the angle projection, see widescreenHalfAngle.) */
float zoomHalfWidth(nfs::App* app)
{
    const float halfWidth = app->getMemory<float>(0x5600ac);
    const float halfHeight = app->getMemory<float>(0x5600b0);
    if (!raceIsRunning() || !(halfWidth * 3.0f > halfHeight * 4.0f))
        return halfWidth;
    return float(double(halfHeight) * app->getMemory<double>(0x54972c));
}

/* The rectangle the in-car cabin is drawn in (the port's cabin fit change): field
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
nfs::u32 cabinRect(nfs::App* app, nfs::u32 field)
{
    const nfs::s32 x = nfs::s32(app->getMemory<nfs::u32>(0x7cdb98));
    const nfs::s32 y = nfs::s32(app->getMemory<nfs::u32>(0x7cdb9c));
    const nfs::s32 width = nfs::s32(app->getMemory<nfs::u32>(0x7cdba0));
    const nfs::s32 height = nfs::s32(app->getMemory<nfs::u32>(0x7cdba4));
    const nfs::u32 screenWidth = app->getMemory<nfs::u32>(0x7cdae0);
    const nfs::u32 screenHeight = app->getMemory<nfs::u32>(0x7cdae4);
    nfs::s32 fitHeight = height;
    if (height > 0 && screenHeight && screenWidth * 3 > screenHeight * 4)
        fitHeight = nfs::s32(SDL_lround(double(height) * double(screenWidth) * 3.0 / (double(screenHeight) * 4.0)));
    switch (field)
    {
    case 0: return nfs::u32(x);
    case 1: return nfs::u32(y + (height - fitHeight) / 2);
    case 2: return nfs::u32(width);
    default: return nfs::u32(fitHeight);
    }
}

/* The size the HUD works its sizes out from (the port's HUD scale change).  The
 * game takes them from the screen it draws on -- the frame around each element
 * and its insets, the gap beside it, the text -- as a fraction of its width,
 * which on a 4:3 screen is the same as a fraction of its height.  Any other
 * shape is not, so what those reads get instead is the 4:3 screen that fits in
 * this one: on a wider screen the width that height would have, on a taller one
 * the height that width would have.  A border then stays the thickness it was
 * drawn with and text the size.  Where an element sits is a separate question,
 * answered in widescreenHudRect; these only say how big its parts are. */
nfs::u32 hudReferenceWidth(nfs::App* app)
{
    const nfs::u32 width = app->getMemory<nfs::u32>(0x7cdac8);
    const nfs::u32 height = app->getMemory<nfs::u32>(0x7cdacc);
    if (!height || width * 3 <= height * 4)
        return width;
    return (height * 4) / 3;
}

nfs::u32 hudReferenceHeight(nfs::App* app)
{
    const nfs::u32 width = app->getMemory<nfs::u32>(0x7cdac8);
    const nfs::u32 height = app->getMemory<nfs::u32>(0x7cdacc);
    if (!width || height * 4 <= width * 3)
        return height;
    return (width * 3) / 4;
}

/* How a HUD element is drawn on the screen in use against its layout: whether
 * its left and right edges move or its top and bottom, and by what factor about
 * their centre.  False for an element drawn as laid out -- all of them on a
 * screen no wider than 4:3.  See widescreenHudRect. */
static bool widescreenHudScale(nfs::App* app, nfs::u32 element, bool& across, double& scale)
{
    const nfs::u32 width = app->getMemory<nfs::u32>(0x7cdae0);
    const nfs::u32 height = app->getMemory<nfs::u32>(0x7cdae4);
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
 * (the port's widescreen change, at the end of sub_480910).  The game lays every
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
void widescreenHudRect(nfs::App* app, nfs::u32 slot)
{
    bool across;
    double scale;
    if (!widescreenHudScale(app, (slot % (23 * 16)) / 16, across, scale))
        return;
    const nfs::u32 rect = 0x749758 + slot;
    const nfs::u32 first = across ? 0 : 4, second = across ? 8 : 12;  // x0 and x1, or y0 and y1
    const double a = double(nfs::s32(app->getMemory<nfs::u32>(rect + first)));
    const double b = double(nfs::s32(app->getMemory<nfs::u32>(rect + second)));
    const double centre = (a + b) / 2.0, half = (b - a) * scale / 2.0;
    app->getMemory<nfs::u32>(rect + first) = nfs::u32(nfs::s32(SDL_lround(centre - half)));
    app->getMemory<nfs::u32>(rect + second) = nfs::u32(nfs::s32(SDL_lround(centre + half)));
}

/* The same change on an element's layout, the fractions of the screen the game
 * keeps it at: four floats, top, left, bottom and right (the port's HUD editor change).
 * By `scale` for as it is drawn, by its inverse for as it is laid out. */
static void scaleHudLayout(nfs::App* app, nfs::u32 rect, bool across, double scale)
{
    const nfs::u32 first = across ? 4 : 0, second = across ? 12 : 8;  // left and right, or top and bottom
    const double a = app->getMemory<float>(rect + first), b = app->getMemory<float>(rect + second);
    const double centre = (a + b) / 2.0, half = (b - a) * scale / 2.0;
    app->getMemory<float>(rect + first) = float(centre - half);
    app->getMemory<float>(rect + second) = float(centre + half);
}

/* The HUD editor writing an element's place into its layout, from the rectangle
 * it moves -- the one drawn: put back as laid out, or every edit would narrow a
 * gauge that drawing then narrows again.  `rect` is the layout's four floats. */
void hudLayoutDesigned(nfs::App* app, nfs::u32 rect, nfs::u32 element)
{
    bool across;
    double scale;
    if (widescreenHudScale(app, element, across, scale))
        scaleHudLayout(app, rect, across, 1.0 / scale);
}

/* The standings table's rectangle made the panel it is drawn as
 * (the port's HUD editor change, at the end of sub_481c50).  The table's layout is
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
void hudTablePanel(nfs::App* app, nfs::u32 player, bool otherLayout)
{
    const nfs::s32 width = nfs::s32(app->getMemory<nfs::u32>(otherLayout ? 0x7925cc : 0x7925c8));
    const nfs::s32 top = nfs::s32(app->getMemory<nfs::u32>(0x724780 + player * 4));
    const nfs::s32 bottom = nfs::s32(app->getMemory<nfs::u32>(0x724788 + player * 4));
    if (width <= 0 || bottom <= top || player > 1)
        return;
    const nfs::u32 rect = 0x749758 + player * 23 * 16 + (otherLayout ? 20 : 13) * 16;
    const nfs::s32 left = nfs::s32(app->getMemory<nfs::u32>(rect));
    app->getMemory<nfs::u32>(rect + 4) = nfs::u32(top);
    app->getMemory<nfs::u32>(rect + 8) = nfs::u32(left + width);
    app->getMemory<nfs::u32>(rect + 12) = nfs::u32(bottom);
}

/* Buffer swaps so far, counted in onSwap: how old a measurement is. */
static nfs::u32 s_swaps = 0;

/* Where each HUD element really is on the screen, as the HUD editor draws it
 * (the port's HUD editor change, around the call sub_45bcb0 makes to sub_45ab30 for
 * every element, every frame).  An element's layout is a box, and what the game
 * draws in it is often far smaller -- a line of text at one side of it, a panel
 * at one corner -- yet the editor framed the box and measured every other element
 * against it, so an element blinked red over empty space.  What is drawn is
 * measured here instead, from every quad the element queues meanwhile
 * (hudQueueQuad): the game's 2D is queued, text glyph by glyph, and drawn
 * later in the frame, all of it together, so what reaches the driver during an
 * element's turn is somebody else's.  Kept with the element's rectangle at the
 * time, so the same measurement still holds once the element has moved: an
 * element draws the same thing wherever it stands.  An element that queues
 * nothing, or something far bigger than its box -- a strip across the whole
 * screen is not what the player means by the element -- keeps its box, as the
 * game had it. */
struct HudDrawn
{
    nfs::u32  swap = 0;       // s_swaps when measured; 0 is never
    nfs::s32 x0, y0, x1, y1; // what was drawn, pixels
    nfs::s32 left, top, right, bottom;  // the element's rectangle then
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
static nfs::u32 s_hudPlayer = 0;

static bool hudDrawnFresh(const HudDrawn& drawn)
{
    return drawn.swap && s_swaps - drawn.swap <= 2;
}

/* The quads queued while an element draws: none while nobody measures. */
static bool s_hudMeasuring = false;
static float s_hudX0, s_hudY0, s_hudX1, s_hudY1;

/* One quad into one of the game's 2D queues (the port's HUD editor change, at the
 * start of sub_49bd30): four vertex numbers into the queue's vertices, 32 bytes
 * each from list+0x10, x and y the first two floats. */
void hudQueueQuad(nfs::App* app, nfs::u32 list, nfs::u32 a, nfs::u32 b, nfs::u32 c, nfs::u32 d)
{
    if (!s_hudMeasuring)
        return;
    for (nfs::u32 vertex : { a, b, c, d })
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

void hudEditorDraw(nfs::App* app, nfs::u32 object, bool begin)
{
    if (begin)
    {
        s_hudMeasuring = true;
        s_hudX0 = s_hudY0 = 1e9f;
        s_hudX1 = s_hudY1 = -1e9f;
        return;
    }
    s_hudMeasuring = false;
    const nfs::u32 index = app->getMemory<nfs::u32>(object + 0x3c);
    if (index >= 46)
        return;
    HudDrawn& drawn = s_hudDrawn[s_hudPlayer][index / 23][index % 23];
    const nfs::s32 left = nfs::s16(app->getMemory<nfs::u16>(object + 6));
    const nfs::s32 top = nfs::s16(app->getMemory<nfs::u16>(object + 8));
    const nfs::s32 right = nfs::s32(app->getMemory<nfs::u32>(object + 0x60));
    const nfs::s32 bottom = nfs::s32(app->getMemory<nfs::u32>(object + 0x64));
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
    const nfs::u32 pixels = 0x749758 + s_hudPlayer * 23 * 16 + (index % 23) * 16;
    if (!drawn.swap || x0 - left != drawn.x0 - drawn.left || y0 - top != drawn.y0 - drawn.top
        || x1 - right != drawn.x1 - drawn.right || y1 - bottom != drawn.y1 - drawn.bottom)
        SDL_Log("[HUDEDIT] element %u drawn at %d,%d-%d,%d in its box %d,%d-%d,%d, the game's %d,%d-%d,%d",
                unsigned(index % 23), x0, y0, x1, y1, int(left), int(top), int(right), int(bottom),
                int(nfs::s32(app->getMemory<nfs::u32>(pixels))),
                int(nfs::s32(app->getMemory<nfs::u32>(pixels + 4))),
                int(nfs::s32(app->getMemory<nfs::u32>(pixels + 8))),
                int(nfs::s32(app->getMemory<nfs::u32>(pixels + 12))));
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
 * (the port's HUD editor change, sub_45a120's four clamps), so a line of text in
 * the middle of a box half the screen wide could never come near the edge.
 * Negative where the drawing spills out of the box: it stays on the screen
 * then.  0 for an element not measured, which leaves the game's own rule.
 *
 * What made this shake near the left edge was not the slack but what the game
 * did underneath it: it re-decided every frame whether an element's text was
 * centred in it or put against one of its edges, and near an edge a pixel of
 * movement flipped the answer.  It does not any more -- an element's text is
 * always centred in it (the port's HUD editor change) -- so what an element draws
 * stands in the same place in its box wherever the box goes, which is what
 * makes this measurement worth anything. */
nfs::s32 hudDragSlack(nfs::App* app, nfs::u32 object, nfs::u32 side)
{
    const nfs::u32 index = app->getMemory<nfs::u32>(object + 0x3c);
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
void hudEditorFrame(nfs::App* app, nfs::u32 object, bool drawn)
{
    static nfs::u32 s_object = 0;
    static nfs::u16 s_left, s_top;
    static nfs::u32 s_right, s_bottom;
    if (!drawn)
    {
        if (s_object != object)
            return;
        app->getMemory<nfs::u16>(object + 6) = s_left;
        app->getMemory<nfs::u16>(object + 8) = s_top;
        app->getMemory<nfs::u32>(object + 0x60) = s_right;
        app->getMemory<nfs::u32>(object + 0x64) = s_bottom;
        s_object = 0;
        return;
    }
    const nfs::u32 index = app->getMemory<nfs::u32>(object + 0x3c);
    if (index >= 46 || !hudDrawnFresh(s_hudDrawn[s_hudPlayer][index / 23][index % 23]))
        return;
    const HudDrawn& measured = s_hudDrawn[s_hudPlayer][index / 23][index % 23];
    s_object = object;
    s_left = app->getMemory<nfs::u16>(object + 6);
    s_top = app->getMemory<nfs::u16>(object + 8);
    s_right = app->getMemory<nfs::u32>(object + 0x60);
    s_bottom = app->getMemory<nfs::u32>(object + 0x64);
    app->getMemory<nfs::u16>(object + 6) = nfs::u16(measured.x0);
    app->getMemory<nfs::u16>(object + 8) = nfs::u16(measured.y0);
    app->getMemory<nfs::u32>(object + 0x60) = nfs::u32(measured.x1);
    app->getMemory<nfs::u32>(object + 0x64) = nfs::u32(measured.y1);
}

/* What the HUD editor's test answered, in the log whenever the answer changes
 * ([HUDEDIT]): the element being moved and where to, as fractions of the screen,
 * and the element in its way.  The test hands that one back through the local
 * at [ebp-0x10] of sub_459db0, and only when it found one: an element off the
 * screen, or one refused for split screen's half, is said as such.  Read while
 * the layouts are still as the test saw them. */
static void traceHudLayoutTest(nfs::App* app, nfs::ThreadState& ts, nfs::u32 element, nfs::u32 rect,
                               nfs::u32 mode, nfs::u32 base, bool answered, nfs::u32 blockerAt)
{
    static int s_last = -2;
    static nfs::u32 s_lastElement = ~0u;
    const bool fits = answered;
    const float y0 = app->getMemory<float>(rect), x0 = app->getMemory<float>(rect + 4);
    const float y1 = app->getMemory<float>(rect + 8), x1 = app->getMemory<float>(rect + 12);
    int blocker = -1;
    nfs::u32 variant = 0;
    if (!fits && y0 >= 0 && x0 >= 0 && y1 <= 1 && x1 <= 1)
    {
        /* One of the front end's items, 0xbc bytes each from 0x604ee0 (the
         * array sub_459670 searches), or the local was never written. */
        const nfs::u32 other = app->getMemory<nfs::u32>(blockerAt);
        if (other >= 0x604ee0 && other < 0x604ee0 + 1024 * 0xbc && (other - 0x604ee0) % 0xbc == 0)
            blocker = int(app->getMemory<nfs::u32>(other + 0x3c) % 23);
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
            const nfs::u32 in = base + variant * 0x1a4 + nfs::u32(blocker) * 16;
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
 * Before the test it is handed its arguments (the rectangle being tried, the
 * player, the element and the editor's mode); after it, its answer and the
 * address of the local it hands the item in the way through. */
void hudLayoutsDrawn(nfs::App* app, nfs::ThreadState& ts, bool drawn, nfs::u32 rect, nfs::u32 playerIn,
                     nfs::u32 elementIn, nfs::u32 modeIn, nfs::u32 fits, nfs::u32 blockerAt)
{
    const nfs::u32 kElements = 23;
    const float kSlack = 0.01f;
    static float s_saved[2][kElements][4];
    static float s_savedTried[4];
    static bool s_triedMeasured = false;
    static nfs::u32 s_base = 0, s_element = 0, s_rect = 0, s_mode = 0;
    if (!drawn)
    {
        if (!s_base)
            return;
        traceHudLayoutTest(app, ts, s_element, s_rect, s_mode, s_base, fits != 0, blockerAt);
        for (nfs::u32 layout = 0; layout < 2; ++layout)
            for (nfs::u32 element = 0; element < kElements; ++element)
                for (nfs::u32 edge = 0; edge < 4; ++edge)
                    app->getMemory<float>(s_base + layout * 0x1a4 + element * 16 + edge * 4) = s_saved[layout][element][edge];
        if (s_triedMeasured)
            for (nfs::u32 edge = 0; edge < 4; ++edge)
                app->getMemory<float>(s_rect + edge * 4) = s_savedTried[edge];
        s_triedMeasured = false;
        s_base = 0;
        return;
    }
    const nfs::u32 player = playerIn;
    s_element = elementIn;
    s_rect = rect;
    s_mode = modeIn;
    const nfs::u32 slot = player + (gameSettings(app).playMode == 1 ? 1 : 0);
    s_base = SETTINGS_ADDRESS(hud[0].rects) + slot * 0x348;
    const nfs::u32 hudWidth = app->getMemory<nfs::u32>(0x7cdac8);
    const nfs::u32 hudHeight = app->getMemory<nfs::u32>(0x7cdacc);
    const bool measurable = player < 2 && hudWidth && hudHeight;
    auto putFractions = [&](nfs::u32 at, nfs::s32 left, nfs::s32 top, nfs::s32 right, nfs::s32 bottom) {
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
    const nfs::u32 named = app->getMemory<nfs::u32>(s_rect - 0x40 + 0x3c);
    const nfs::u32 variant = named < 46 ? named / 23 : 0;
    if (measurable && s_element < kElements && hudDrawnFresh(s_hudDrawn[player][variant][s_element]))
    {
        for (nfs::u32 edge = 0; edge < 4; ++edge)
            s_savedTried[edge] = app->getMemory<float>(s_rect + edge * 4);
        const HudDrawn& measured = s_hudDrawn[player][variant][s_element];
        const nfs::u32 object = s_rect - 0x40;
        const nfs::s32 left = nfs::s16(app->getMemory<nfs::u16>(object + 6));
        const nfs::s32 top = nfs::s16(app->getMemory<nfs::u16>(object + 8));
        const nfs::s32 right = nfs::s32(app->getMemory<nfs::u32>(object + 0x60));
        const nfs::s32 bottom = nfs::s32(app->getMemory<nfs::u32>(object + 0x64));
        putFractions(s_rect, left + measured.x0 - measured.left, top + measured.y0 - measured.top,
                     right + measured.x1 - measured.right, bottom + measured.y1 - measured.bottom);
        s_triedMeasured = true;
    }

    for (nfs::u32 layout = 0; layout < 2; ++layout)
        for (nfs::u32 element = 0; element < kElements; ++element)
        {
            const nfs::u32 rect = s_base + layout * 0x1a4 + element * 16;
            for (nfs::u32 edge = 0; edge < 4; ++edge)
                s_saved[layout][element][edge] = app->getMemory<float>(rect + edge * 4);
            if (measurable && !(layout == variant && element == s_element)
                && hudDrawnFresh(s_hudDrawn[player][layout][element]))
            {
                const HudDrawn& measured = s_hudDrawn[player][layout][element];
                putFractions(rect, measured.x0, measured.y0, measured.x1, measured.y1);
            }
            else if (element == (layout ? 20u : 13u) && measurable)
            {
                const nfs::u32 panel = 0x749758 + player * 23 * 16 + element * 16;
                const nfs::s32 panelLeft = nfs::s32(app->getMemory<nfs::u32>(panel));
                const nfs::s32 panelTop = nfs::s32(app->getMemory<nfs::u32>(panel + 4));
                const nfs::s32 panelRight = nfs::s32(app->getMemory<nfs::u32>(panel + 8));
                const nfs::s32 panelBottom = nfs::s32(app->getMemory<nfs::u32>(panel + 12));
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
 * it (the port's music stream change).  The game copied the whole track -- 7 to 14
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

void musicIndexOpened(nfs::App* app, nfs::u32 path)
{
    s_musicIndex[0] = '\0';
    for (size_t i = 0; i + 1 < sizeof(s_musicIndex); ++i)
    {
        const char letter = char(app->getMemory<nfs::u8>(path + nfs::u32(i)));
        s_musicIndex[i] = letter;
        s_musicIndex[i + 1] = '\0';
        if (!letter)
            return;
    }
    /* Longer than any name of the game's: keep none rather than half of one. */
    s_musicIndex[0] = '\0';
}

void musicStreamFile(nfs::App* app, nfs::u32 path)
{
    const size_t length = SDL_strlen(s_musicIndex);
    if (length < 5 || s_musicIndex[length - 4] != '.')
        return;
    SDL_memcpy(s_musicIndex + length - 4, ".mus", 5);
    for (size_t i = 0; i <= length; ++i)
        app->getMemory<nfs::u8>(path + nfs::u32(i)) = nfs::u8(s_musicIndex[i]);
    SDL_Log("[MUSIC] streaming %s", s_musicIndex);
}

/* A new player's settings (the port's first settings change).  With no settings
 * file of its own -- data copied straight off the disc has none, the game makes
 * it the first time it runs -- or none it would take, the game starts its
 * settings over (sub_472980, sub_4723f0), and the first time the front end
 * comes up it chooses what suits the machine (sub_472d10, View Distance among
 * it).  Right after that the phone's go over them: NFS3Activity.onFirstSettings
 * lays them on the block the game keeps, 0x20f0 bytes at 0x6fbb40 that are the
 * file word for word, as an import writes them into a settings file that came
 * with the data.  Returns whether it did, for the caller to have the game take
 * up the new bindings and save the file. */
bool firstSettings(nfs::App* app)
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
            void* block = &gameSettings(app);
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

// The most of Render_GetTm's buffer a view pass used since the last call
// (native_vertices.cpp, at sub_4bbd70).
nfs::u32 takeArenaPeak();
}

namespace
{
/* How hard player one's car corners, for the phone's own vibration, about every
 * 40 ms while a race is drawn.  It is the load the game's Body Roll force
 * feedback is made of (sub_4716e0): [car+0x924] against the scale at 0x5efde4,
 * full where the game clamps that effect.  Read from the car rather than taken
 * from the effect, which the game only computes while its Body Roll slider is
 * above zero -- a pad's setting the phone should not depend on.  The car is the
 * one force feedback follows (0x678b54); while its flag at +0x164 is set or
 * +0x15c is negative the game stops its effects, and there is no cornering to
 * feel either. */
void phoneTick(nfs::App* app)
{
    static Uint64 last = 0;
    const Uint64 now = SDL_GetTicks();
    if (now - last < 40)
        return;
    last = now;
    float turn = 0;
    const nfs::u32 car = app->getMemory<nfs::u32>(0x678b54);
    const float scale = app->getMemory<float>(0x5efde4);
    if (car && scale > 0 && app->getMemory<nfs::u16>(car + 0x164) == 0
        && app->getMemory<float>(car + 0x15c) >= 0)
        turn = SDL_min(1.0f, SDL_fabsf(app->getMemory<float>(car + 0x924)) / scale);
    nfs::Gamepad::phoneTick(turn);
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
void steeringTick(nfs::App* app)
{
    static int s_last = -1;
    static nfs::u32 s_raced = 0;
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
    /* Nor in a race against another machine ([0x6fd3b0] 2 modem, 3 serial, 4
     * network): every machine drives every car from the players' inputs alone,
     * and each learned this flag from the others as the race began (message 9,
     * sub_43da60), so a flag changed here and nowhere else drives this car
     * differently on this machine than on the rest. */
    if (gameSettings(app).playMode >= 2)
        return;
    const nfs::u32 car = app->getMemory<nfs::u32>(0x678b54);
    if (!car)
        return;
    const nfs::u32 record = app->getMemory<nfs::u32>(car + 0x220);
    if (record < SETTINGS_ADDRESS(entries) || record >= SETTINGS_ADDRESS(entries) + 16 * 0x6c || (record - SETTINGS_ADDRESS(entries)) % 0x6c)
        return;
    /* A replay is the race driven again from what the game recorded of it, so
     * the car is steered the way it was steered then: the flag keeps the value
     * the race left it at rather than following a screen nobody is touching.
     * ([0x6fd3a0] is 2 while a replay runs, as sub_4bcb00 sets it.) */
    const bool replay = gameSettings(app).replayMode == 2;
    const nfs::u32 digital = replay ? s_raced : nfs::u32(nfs::Gamepad::touchSteering() ? 1 : 0);
    if (!replay)
        s_raced = digital;
    if (app->getMemory<nfs::u32>(record + 0x24) != digital)
        app->getMemory<nfs::u32>(record + 0x24) = digital;
    if (s_last != int(digital))
    {
        SDL_Log("[STEERING] player %u: %s%s", unsigned((record - SETTINGS_ADDRESS(entries)) / 0x6c),
                digital ? "the game's keyboard steering (touch buttons)" : "analog (pad)",
                replay ? ", as the replay was recorded" : "");
        s_last = int(digital);
    }
}

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
void traceInput(nfs::App* app)
{
    static nfs::u32 s_step = ~nfs::u32(0);
    if (!nfs3hp::raceIsRunning())
    {
        s_step = ~nfs::u32(0);
        return;
    }
    const nfs::u32 step = app->getMemory<nfs::u32>(0x7d3684) >> 3;
    if (step == s_step)
        return;
    s_step = step;
    const nfs::u32 car = app->getMemory<nfs::u32>(0x678b54);
    if (!car)
        return;
    const nfs::u32 record = app->getMemory<nfs::u32>(car + 0x220);
    if (record < SETTINGS_ADDRESS(entries) || record >= SETTINGS_ADDRESS(entries) + 16 * 0x6c || (record - SETTINGS_ADDRESS(entries)) % 0x6c)
        return;
    SDL_Log("[INPUT] step %u mode %u rec %08x %08x %08x %08x %08x %08x at %.1f %.1f %.1f",
            unsigned(step), unsigned(gameSettings(app).replayMode),
            unsigned(app->getMemory<nfs::u32>(record + 0x1c)),
            unsigned(app->getMemory<nfs::u32>(record + 0x20)),
            unsigned(app->getMemory<nfs::u32>(record + 0x24)),
            unsigned(app->getMemory<nfs::u32>(record + 0x28)),
            unsigned(app->getMemory<nfs::u32>(record + 0x2c)),
            unsigned(app->getMemory<nfs::u32>(record + 0x30)),
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
/* [VIEW], every two seconds of a race: what drawing further than View
 * Distance's 500 would meet (2026-09-26).
 *   far      the far distance of a single view and of split screen's halves
 *            ([0x5dd108], [0x5dd15c], 16.16)
 *   fog      where the track's vertex fog starts and ends ([0x552e04],
 *            [0x552e08]): past its end the track is the fog's colour alone
 *   buffer   the most of Render_GetTm's buffer one view pass took, against its
 *            size ([0x7a3d04]): past it the rest of the pass is not drawn
 * and once a race, how far the track's own lists of what each block can see
 * reach (block records of 0x5c0 bytes at 0x571370, the centre at +0x78, the
 * list at +0xb4, four bytes an entry, -1 ending it; [0x5dd83c] the last block):
 * a block no list names is never drawn, however far the view reaches. */
bool playerDriving(nfs::App* app);

void viewTick(nfs::App* app)
{
    static bool reachLogged = false;
    static Uint64 last = 0;
    if (!nfs3hp::raceIsRunning())
    {
        reachLogged = false;
        return;
    }
    if (!reachLogged && playerDriving(app))
    {
        reachLogged = true;
        const nfs::s32 lastBlock = nfs::s32(app->getMemory<nfs::u32>(0x5dd83c));
        std::vector<float> reach;
        unsigned longest = 0;
        for (nfs::s32 block = 0; block <= lastBlock && block < 2000; ++block)
        {
            const nfs::u32 record = 0x571370 + nfs::u32(block) * 0x5c0;
            const float x = app->getMemory<float>(record + 0x78);
            const float z = app->getMemory<float>(record + 0x80);
            float farthest = 0.0f;
            unsigned listed = 0;
            for (; listed < 300; ++listed)
            {
                const nfs::s32 other = nfs::s16(app->getMemory<nfs::u16>(record + 0xb4 + listed * 4));
                if (other < 0 || other > lastBlock)
                    break;
                const nfs::u32 at = 0x571370 + nfs::u32(other) * 0x5c0;
                const float dx = app->getMemory<float>(at + 0x78) - x;
                const float dz = app->getMemory<float>(at + 0x80) - z;
                farthest = std::max(farthest, std::sqrt(dx * dx + dz * dz));
            }
            longest = std::max(longest, listed);
            reach.push_back(farthest);
        }
        if (!reach.empty())
        {
            std::sort(reach.begin(), reach.end());
            const std::size_t n = reach.size();
            unsigned under = 0;
            for (float r : reach)
                under += r < 500.0f;
            SDL_Log("[VIEW] %u blocks; what a block's list reaches: least %.0f, a tenth under %.0f, median %.0f, "
                    "most %.0f; %u blocks reach less than 500; longest list %u",
                    unsigned(n), double(reach.front()), double(reach[n / 10]), double(reach[n / 2]),
                    double(reach.back()), under, longest);
        }
    }
    const Uint64 now = SDL_GetTicks();
    if (now - last < 2000)
        return;
    last = now;
    const nfs::u32 peak = nfs3hp::takeArenaPeak();
    SDL_Log("[VIEW] far %.0f, split %.0f; fog %.0f to %.0f; buffer %u of %u KB",
            double(nfs::s32(app->getMemory<nfs::u32>(0x5dd108))) / 65536.0,
            double(nfs::s32(app->getMemory<nfs::u32>(0x5dd15c))) / 65536.0,
            double(app->getMemory<float>(0x552e04)), double(app->getMemory<float>(0x552e08)),
            unsigned(peak >> 10), unsigned(app->getMemory<nfs::u32>(0x7a3d04) >> 10));
}

struct TickTrace
{
    nfs::u32 clock = 0;
    nfs::u32 swaps = 0;
    Uint64     at = 0;
    // The GPU, looked at every frame: load in percent times clock in MHz.
    double     gpuLoad = 0;
    double     gpuClock = 0;
    double     gpuBusy = 0;
    unsigned   gpuLooks = 0;
    bool       gpuHidden = false;
    /* How many of the game's ticks each frame took, 0 to 4 and more: the
     * race's clock follows a timer the game re-arms itself, and a late timer
     * shows up here as frames that run several ticks at once. */
    nfs::u32 frameClock = 0;
    unsigned   ticksAFrame[5] = {};
};
TickTrace s_ticks;

/* A number the kernel shows in a file of its own, such as "  85%"; -1 where
 * there is no such file or an app may not read it. */
int readKernelNumber(const char* path)
{
#ifdef __ANDROID__
    const int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return -1;
    char text[32];
    const ssize_t length = read(fd, text, sizeof text - 1);
    close(fd);
    if (length <= 0)
        return -1;
    text[length] = 0;
    return int(SDL_strtol(text, nullptr, 10));
#else
    (void)path;
    return -1;
#endif
}

/* How busy the GPU is and how fast the governor runs it, as Samsung's kernels
 * show them to any app (/sys/kernel/gpu); the clock is in kHz there. */
void lookAtGpu()
{
    if (s_ticks.gpuHidden)
        return;
    const int load = readKernelNumber("/sys/kernel/gpu/gpu_busy");
    const int clock = readKernelNumber("/sys/kernel/gpu/gpu_clock");
    if (load < 0 || clock < 0)
    {
        s_ticks.gpuHidden = true;
        return;
    }
    const double megahertz = clock >= 10000 ? clock / 1000.0 : double(clock);
    s_ticks.gpuLoad += load;
    s_ticks.gpuClock += megahertz;
    s_ticks.gpuBusy += load / 100.0 * megahertz;
    ++s_ticks.gpuLooks;
}

void traceTicks(nfs::App* app)
{
    const Uint64 now = SDL_GetTicks();
    const nfs::u32 clock = app->getMemory<nfs::u32>(0x7d3684);
    lookAtGpu();
    if (clock >= s_ticks.frameClock)
        ++s_ticks.ticksAFrame[SDL_min(clock - s_ticks.frameClock, nfs::u32(4))];
    s_ticks.frameClock = clock;
    if (!s_ticks.at || clock < s_ticks.clock)
    {
        s_ticks.clock = clock;
        s_ticks.swaps = nfs3hp::s_swaps;
        s_ticks.at = now;
        s_ticks.gpuLoad = s_ticks.gpuClock = s_ticks.gpuBusy = 0;
        s_ticks.gpuLooks = 0;
        return;
    }
    const Uint64 span = now - s_ticks.at;
    if (span < 1000)
        return;
    const nfs::u32 ticks = clock - s_ticks.clock;
    const nfs::u32 frames = nfs3hp::s_swaps - s_ticks.swaps;
    const nfs::DrawCounters drawn = nfs::ThrashRenderer::takeCounters();
    SDL_Log("[TICKS] %u in %llu ms (%.1f/s), %u frames (%.1f/s), %.2f ticks a frame, clock %u, mode %u; "
            "%u triangles a frame, %u fogged, %u fog tables, %u batches (%u empty), %u cut",
            unsigned(ticks), (unsigned long long)span, double(ticks) * 1000.0 / double(span),
            unsigned(frames), double(frames) * 1000.0 / double(span),
            frames ? double(ticks) / double(frames) : 0.0, unsigned(clock),
            unsigned(gameSettings(app).replayMode),
            frames ? drawn.triangles / unsigned(frames) : 0u, frames ? drawn.fogged / unsigned(frames) : 0u,
            drawn.fogTables, frames ? drawn.drawCalls / unsigned(frames) : 0u,
            frames ? drawn.emptyCalls / unsigned(frames) : 0u, frames ? drawn.cut / unsigned(frames) : 0u);
    /* The GPU's side of the same second: its work is the busy share of its
     * clock, so M cycles a frame compares seconds the governor ran at
     * different clocks; the cap is the highest clock it may take just now.
     * depth16 counts the frames drawn with the Voodoo's 16-bit depth steps. */
    if (s_ticks.gpuLooks && frames)
    {
        const int cap = readKernelNumber("/sys/kernel/gpu/gpu_max_clock");
        SDL_Log("[GPU] %.0f%% at %.0f MHz, cap %d MHz, %.2f Mcycles a frame; depth16 %u of %u frames; "
                "night %u, split %u",
                s_ticks.gpuLoad / s_ticks.gpuLooks, s_ticks.gpuClock / s_ticks.gpuLooks,
                cap >= 10000 ? cap / 1000 : cap,
                s_ticks.gpuBusy / s_ticks.gpuLooks / (double(frames) * 1000.0 / double(span)),
                drawn.depth16Frames, drawn.frames,
                unsigned(gameSettings(app).night), unsigned(gameSettings(app).playMode));
    }
    /* And the x87's special cases this second, all threads together: compares
     * that met a NaN, and fistp with no integer to store. */
    static unsigned s_unordered = 0, s_indefinite = 0;
    const unsigned unordered = nfs::FloatEnv::s_unordered, indefinite = nfs::FloatEnv::s_indefinite;
    /* The C runtime's rand() (sub_4eb1fc) keeps its seed at +0xc of the
     * thread's data, which it finds through the pointer at [0x567764]:
     * 0x4ff0ec returns the main thread's block [0x9f642c]; once a thread has
     * been started, 0x51e7a8 looks it up with TlsGetValue([0x567760]). */
    const nfs::u32 threadData = app->getMemory<nfs::u32>(0x9f642c);
    if (ticks)
        SDL_Log("[STEP] frames with 0/1/2/3/4+ ticks: %u/%u/%u/%u/%u; NaN compares %u, fistp indefinite %u; "
                "thread data by %08x, tls %08x, main seed %08x",
                s_ticks.ticksAFrame[0], s_ticks.ticksAFrame[1], s_ticks.ticksAFrame[2], s_ticks.ticksAFrame[3],
                s_ticks.ticksAFrame[4], unordered - s_unordered, indefinite - s_indefinite,
                unsigned(app->getMemory<nfs::u32>(0x567764)), unsigned(app->getMemory<nfs::u32>(0x567760)),
                threadData ? unsigned(app->getMemory<nfs::u32>(threadData + 0xc)) : 0u);
    s_unordered = unordered;
    s_indefinite = indefinite;
    SDL_memset(s_ticks.ticksAFrame, 0, sizeof s_ticks.ticksAFrame);
    s_ticks.gpuLoad = s_ticks.gpuClock = s_ticks.gpuBusy = 0;
    s_ticks.gpuLooks = 0;
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

nfs::s32 clampAxis(nfs::s32 value, nfs::s32 low, nfs::s32 high)
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
bool playerDriving(nfs::App* app)
{
    return nfs3hp::raceIsRunning() && app->getMemory<nfs::u32>(0x7a3d10) == 0
        && gameSettings(app).replayMode != 2;
}

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

bool frontEndTakingText(nfs::App* app);

/* The line the game's name box types into, in the 640x480 its menus are laid
 * out in, as sub_444350 draws its frame.  The box itself is four floats at
 * 0x661050 -- top, left, bottom, right -- that sub_444c20 fits around its text
 * as it opens it; the line is centred across it, as wide as [0x661070] (the
 * widest letter times the letters a name can have, and the cursor, 240 at
 * most), and 20 tall from one above the text, which sits 90 above the box's
 * bottom (the -30 and -60 at 0x538b08).  A finger is no cursor, so the line
 * reaches a little way beyond itself -- short of the prompt above it and of
 * Ok and Cancel below. */
bool textFieldRect(nfs::App* app, nfs::s32& left, nfs::s32& top,
                   nfs::s32& right, nfs::s32& bottom)
{
    const float boxLeft = app->getMemory<float>(0x661054), boxRight = app->getMemory<float>(0x66105c);
    const float boxBottom = app->getMemory<float>(0x661058);
    const nfs::s32 width = nfs::s32(app->getMemory<nfs::u32>(0x661070));
    if (!(boxRight > boxLeft) || width <= 0 || width > 640)
        return false;
    left = nfs::s32(boxLeft) + (nfs::s32(boxRight - boxLeft) - width) / 2;
    right = left + width;
    top = nfs::s32(boxBottom - 90) - 1;
    bottom = top + 20;
    return true;
}

bool inTextField(nfs::App* app, nfs::s32 x, nfs::s32 y)
{
    nfs::s32 left, top, right, bottom;
    return textFieldRect(app, left, top, right, bottom)
        && x >= left - 10 && x <= right + 10 && y >= top - 10 && y <= bottom + 10;
}

/* A press on the picture while the name box is up: on its line the phone's
 * keyboard comes up, anywhere else it goes away (NFS3Activity.onTextFieldTap).
 * The press goes on to the game either way. */
void textFieldPress(nfs::App* app, nfs::s32 x, nfs::s32 y)
{
    if (!frontEndTakingText(app))
        return;
    const bool inside = inTextField(app, x, y);
    SDL_Log("[TEXT] press at %d,%d %s the name's line", int(x), int(y), inside ? "on" : "off");
    tellActivity("onTextFieldTap", inside);
}

void pointerTick(nfs::App* app)
{
    if (!s_touchMutex)
        return;
    const bool driving = playerDriving(app);
    /* On the race's loading screen a tap is a click and nothing more: it hurries the loading bar
     * (graphics::loadingProgress), and is not handed to the game, whose queue would keep it for
     * the race to find. */
    const bool loading = nfs3hp::loadingScreenUp();
    if (driving && s_touchHeld)
    {
        nfs::Mouse::pressPointer(0, false);
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
    if (!nfs3hp::port::driver::screenRect(rectX, rectY, rectW, rectH, pictureW, pictureH))
        return;
    (void)pictureW;
    (void)pictureH;
    /* The cursor counts in whatever space the game gave it, which is the whole
     * of the picture in a race and the 640x480 its menus are laid out in --
     * sub_4352d0 sets the bounds either way.  So a touch is placed in those
     * bounds rather than in pixels of the picture, and follows the menus
     * wherever they are drawn. */
    const nfs::s32 lowX = nfs::s32(app->getMemory<nfs::u32>(0x5f38e0));
    const nfs::s32 lowY = nfs::s32(app->getMemory<nfs::u32>(0x5f38e4));
    const nfs::s32 highX = nfs::s32(app->getMemory<nfs::u32>(0x5f38e8));
    const nfs::s32 highY = nfs::s32(app->getMemory<nfs::u32>(0x5f38ec));
    if (highX <= lowX || highY <= lowY)
        return;
    /* Where the game has the cursor now; from here on it is followed in step
     * with what it is told, since it only catches up when the game next reads
     * the queue -- once for everything handed over this frame. */
    nfs::s32 atX = clampAxis(nfs::s32(app->getMemory<nfs::u32>(0x5f38f0)), lowX, highX);
    nfs::s32 atY = clampAxis(nfs::s32(app->getMemory<nfs::u32>(0x5f38f4)), lowY, highY);

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
            const nfs::s32 moveX = nfs::s32(s_touchpadRestX), moveY = nfs::s32(s_touchpadRestY);
            s_touchpadRestX -= float(moveX);
            s_touchpadRestY -= float(moveY);
            const nfs::s32 toX = clampAxis(atX + moveX, lowX, highX), toY = clampAxis(atY + moveY, lowY, highY);
            nfs::Mouse::movePointer(toX - atX, toY - atY);
            atX = toX;
            atY = toY;
            const bool down = (input.buttons & 1) != 0;
            if (s_tracePointer)
                SDL_Log("[POINTER] touchpad %+.0f,%+.0f button %s -> cursor %d,%d of %dx%d", input.x, input.y,
                        down ? "down" : "up", int(toX), int(toY), int(highX), int(highY));
            if (down && !s_touchHeld)
            {
                textFieldPress(app, toX, toY);
                nfs::Mouse::pressPointer(0, true);
                s_touchHeld = true;
                ++taken;
                break;  // as for a finger: the release waits for the next frame
            }
            if (!down && s_touchHeld)
            {
                nfs::Mouse::pressPointer(0, false);
                s_touchHeld = false;
            }
            continue;
        }
        const bool touch = input.source == kSourceTouch;
        if (touch && loading)
        {
            if (input.action == kTouchDown)
                nfs::Mouse::notePress();
            continue;
        }
        if (touch && driving)
            continue;
        nfs::s32 toX = clampAxis(
            lowX + nfs::s32(SDL_lround(double(input.x - rectX) * double(highX - lowX) / double(rectW))), lowX, highX);
        nfs::s32 toY = clampAxis(
            lowY + nfs::s32(SDL_lround(double(input.y - rectY) * double(highY - lowY) / double(rectH))), lowY, highY);
        if (touch && input.action == kTouchDown && !s_touchHeld)
        {
            textFieldPress(app, toX, toY);
        }
        // Preserve the actual hit point: name text can be covered by a dropdown.
        nfs::Mouse::movePointer(toX - atX, toY - atY);
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
                nfs::Mouse::pressPointer(0, true);
                s_touchHeld = true;
                ++taken;
                break;  // the release waits for the next frame, so the press is its own poll
            }
            if (input.action == kTouchUp && s_touchHeld)
            {
                nfs::Mouse::pressPointer(0, false);
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
                nfs::Mouse::pressPointer(nfs::u32(button), down);
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
    if (!nfs3hp::port::driver::screenRect(rectX, rectY, rectW, rectH, pictureW, pictureH))
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
void layoutTick(nfs::App* app)
{
    static int last = -1;
    // Not while the loading screen is still up: nothing to drive yet.
    const bool driving = playerDriving(app) && !nfs3hp::loadingScreenUp();
    /* The pads follow the same signal as the on-screen controls: their buttons
     * are the racing ones while a race is being driven, and the menu's -- one
     * to confirm, one to go back, the D-pad to move -- everywhere else, a
     * replay included.  With the menu open over a race the button that opened
     * it closes it too (Gamepad::Context::Pause). */
    const bool paused = nfs3hp::raceIsRunning() && app->getMemory<nfs::u32>(0x7a3d10) != 0;
    nfs::Gamepad::context(driving ? nfs::Gamepad::Context::Race
                            : paused ? nfs::Gamepad::Context::Pause : nfs::Gamepad::Context::Menu);
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
            unsigned(app->getMemory<nfs::u32>(0x7cdae0)), unsigned(app->getMemory<nfs::u32>(0x7cdae4)));
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
void raceControlsTick(nfs::App* app)
{
    static int s_last = -1;
    if (!nfs3hp::raceIsRunning())
    {
        s_last = -1;
        return;
    }
    const nfs::u32 view = app->getMemory<nfs::u32>(0x5e10b0);
    const nfs::u32 car = view ? nfs::u32(app->getMemory<nfs::u32>(view + 8)) : 0;
    if (!car)
        return;
    const nfs::u32 record = app->getMemory<nfs::u32>(car + 0x220);
    if (record < SETTINGS_ADDRESS(entries) || record >= SETTINGS_ADDRESS(entries) + 16 * 0x6c || (record - SETTINGS_ADDRESS(entries)) % 0x6c)
        return;
    const bool gears = app->getMemory<nfs::u32>(record + 0x1c) != 1;
    const bool spikes = (app->getMemory<nfs::u8>(car + 0x200) & 0x20) != 0;
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
 * put away.  The car screen's own box (the port's text entry change) counts as
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
bool frontEndTakingText(nfs::App* app)
{
    return app->getMemory<nfs::u8>(0x558b10) != 0
        && app->getMemory<nfs::u32>(0x558b14) == 3;
}

void textEntryTick(nfs::App* app)
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
    nfs::s32 left, top, right, bottom;
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
void padKeysTick(nfs::App* app)
{
    bool muted = false;
    if (playerDriving(app) && gameSettings(app).playMode == 1)
    {
        muted = true;
        for (nfs::u32 i = 0; i < 3 * 13 && muted; ++i)
            muted = gameSettings(app).controls[0][i] == app->getMemory<nfs::u32>(0x490430 + i * 4);
    }
    static int last = -1;
    if (last != int(muted))
    {
        SDL_Log("[PADKEYS] %s", muted ? "muted: split screen on the game's keyboard controls" : "on");
        last = int(muted);
    }
    nfs::Gamepad::muteGameKeys(muted);
}

/* Every buffer swap: the finger and the mouse on the picture, which layout the
 * controls should be showing and which of the race's buttons, whether the pads'
 * keys go out, the phone's cornering and how the touch buttons steer. */
void onSwap(nfs::App* app)
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
    viewTick(app);
    if (s_traceTicks)
        traceTicks(app);
    if (s_traceInput)
        traceInput(app);
}

/* The app to the background and back, on the thread that pumps the events.
 *
 * The game's time stands still in the background (nfs::suspendTime): before,
 * its timer went on counting while the game itself was stopped, and on the
 * player's return the race made up for every tick at once -- the opponents had
 * driven on meanwhile.  And a race the player was driving comes back paused,
 * as a PC game does when it loses the screen: the game's own pause menu, opened
 * by the game's own key.  Not over a race's loading screen, where Escape would
 * abandon the load, not over a replay or the pause menu itself, and not in a
 * network race ([0x6fd3b0] 2 and more), which no one player may stop. */
bool s_pauseOnReturn = false;

void pushEscape(bool down)
{
    SDL_Event event;
    SDL_zero(event);
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.timestamp = SDL_GetTicksNS();
    event.key.scancode = SDL_SCANCODE_ESCAPE;
    event.key.key = SDLK_ESCAPE;
    event.key.down = down;
    SDL_PushEvent(&event);
}

/* Held a little while, as the back gesture holds it (NFS3Activity): the game
 * reads the key once a frame and a press shorter than that can go unseen.
 * From SDL's timer thread, outside the event watch that asks for it. */
Uint32 SDLCALL releaseEscape(void*, SDL_TimerID, Uint32)
{
    pushEscape(false);
    return 0;
}

Uint32 SDLCALL pressEscape(void*, SDL_TimerID, Uint32)
{
    pushEscape(true);
    SDL_AddTimer(150, releaseEscape, nullptr);
    return 0;
}

bool SDLCALL backgroundWatch(void* userdata, SDL_Event* event)
{
    nfs::App* app = static_cast<nfs::App*>(userdata);
    if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND)
    {
        s_pauseOnReturn = playerDriving(app) && !nfs3hp::loadingScreenUp()
                       && gameSettings(app).playMode < 2;
        nfs::suspendTime(true);
        SDL_Log("[TIME] to the background%s", s_pauseOnReturn ? "; the race comes back paused" : "");
    }
    else if (event->type == SDL_EVENT_DID_ENTER_FOREGROUND)
    {
        nfs::suspendTime(false);
        if (s_pauseOnReturn)
        {
            s_pauseOnReturn = false;
            SDL_AddTimer(50, pressEscape, nullptr);
        }
    }
    return true;
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
    nfs::File::setDataDirectory(dataPath);
    nfs::File::setCdDirectory(dataPath);
#else
    if (argc >= 3)
    {
        nfs::File::setDataDirectory(argv[1]);
        nfs::File::setCdDirectory(argv[2]);
    }
    else if (argc == 2)
    {
        nfs::File::setDataDirectory(argv[1]);
        nfs::File::setCdDirectory(argv[1]);
    }
    else
    {
        std::string exeDir = getExeDirectory(argv[0]);
        nfs::File::setDataDirectory(exeDir.c_str());
        nfs::File::setCdDirectory(exeDir.c_str());
    }
#endif
    nfs3hp::installMenuData();
    /* The game pumps its message queue from a guest thread, which is not
     * the thread SDL considers "main" on Android.  SDL only warns about
     * it, but the warning is a modal dialog that has to be dismissed on
     * every single launch.  Suppress the dialog rather than the check:
     * nothing downstream depends on it, and a real fix means moving the
     * pump onto the SDL main thread, which is a much larger change. */
    SDL_SetHint(SDL_HINT_ASSERT, "always_ignore");
    /* No poll sentinels.  On Android SDL_WaitEvent spins with them: each round
     * of its wait pumps events, the pump puts a sentinel on the queue, and
     * anything put on the queue wakes the waiter -- which is itself.  The
     * game's window thread waits in SDL_WaitEvent for good (GetMessageA), so a
     * profile showed that thread taking most of a core, polling joysticks and
     * the clock without end, and the heat of it pulling the big cores' clock
     * down.  Nothing in the port asks for sentinels: SDL_PollEvent is not
     * used, and the pump wants real events only.  The pads are updated once a
     * frame instead (Gamepad::update). */
    SDL_SetHint(SDL_HINT_POLL_SENTINEL, "0");
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
    /* Debug mode off (NFS_LOG=0, the launcher's Debug screen) writes nothing
     * to the log: every line below critical -- the port's diagnostics and
     * SDL's own -- is dropped before it is formatted. */
    if (const char* log = SDL_getenv("NFS_LOG"); log && SDL_strcmp(log, "0") == 0)
        SDL_SetLogPriorities(SDL_LOG_PRIORITY_CRITICAL);
    /* Debug mode's "Live functions log": which functions run, sampled, to
     * functions.bin (func_log.cpp). */
#ifdef __ANDROID__
    if (const char* funcLog = SDL_getenv("NFS_FUNC_LOG"); funcLog && *funcLog && *funcLog != '0')
        nfs3hp::startFunctionLog();
    if (const char* trace = SDL_getenv("NFS_REPLAY_TRACE"); trace && *trace && *trace != '0')
        nfs3hp::startReplayTrace();
#endif
    nfs3hp::diag::takeAutoReplay = nfs3hp::takeAutoReplay;
    nfs3hp::diag::raceDown = nfs3hp::raceDownStep;
    SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK|SDL_INIT_GAMEPAD);
    /* The window's wait (os::window) consumes key, text, quit and the port's own
     * events; everything else falls through its switch and is thrown away.  The
     * pad is read by polling its state (Gamepad::update), never from
     * events.  Leaving those event types enabled meant a connected controller,
     * reporting at its native rate, kept the queue non-empty -- so SDL_WaitEvent
     * never actually blocked and the window's wait ran SDL_PumpEvents thousands
     * of times a second for nothing.  A simpleperf profile showed that thread
     * burning most of a core inside SDL with zero game code on it. */
    SDL_SetJoystickEventsEnabled(false);
    SDL_SetGamepadEventsEnabled(false);
    {
        /* nfs3.exe's own version resource (StringFileInfo 040904b0,
         * FileVersion): the version a network race checks between machines. */
        nfs3hp::os::setFileVersion("27, 2.0");
        nfs3hp::Application app("nfs3.exe");
        nfs3hp::installDisplaySliders(&app);
        nfs3hp::installConnectScreen(&app);
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Device Description", new nfs::SettingValue("3Dfx Voodoo 2"));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Card", new nfs::SettingValue("3Dfx Voodoo 2"));
        //app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new nfs::SettingValue("softtri"));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new nfs::SettingValue("voodoo2"));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Group", new nfs::SettingValue("3Dfx"));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "D3D Device", new nfs::SettingValue(nfs::u32(0)));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Triple Buffer", new nfs::SettingValue(nfs::u32(0)));
        //app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new nfs::SettingValue(nfs::u32(0)));
        app.addSetting(nfs::SettingsRoot::localMachine, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new nfs::SettingValue(nfs::u32(1)));

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
         * match the player's.  It costs CPU rather than GPU -- THRASH
         * takes screen-space vertices, so the game transforms all that geometry
         * itself.  Only the High row is touched: Medium and Low stay as they
         * were, and the cull distance itself is left alone so the draw
         * distance does not change. */
        {
            /* Twice the cull distance rather than the distance itself: split
             * screen scales the three switch distances by 0.75, and the rear-view
             * mirror scales them and the cull distance by 0.6 (sub_4bb5d0), both
             * at once in a split-screen mirror.  Twice the cull distance stays at
             * or beyond the cull distance under all of those, so a car is either
             * on the detailed model or not drawn at all, in every view.  Held at
             * just the cull distance, split screen drew every car beyond 225 as
             * the tiny box. */
            const nfs::u32 highRow = 0x55fdf4;
            const float cull = app.getMemory<float>(highRow + nfs::u32(12));
            for (nfs::u32 level = 0; level < 3; ++level)
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
            const nfs::u32 budgetTable = 0x4b7a50;
            app.getMemory<nfs::u32>(budgetTable) = 0x7fffffff;
        }
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
        nfs3hp::port::driver::setSwapObserver(onSwap);
        SDL_AddEventWatch(backgroundWatch, static_cast<nfs::App*>(&app));

        /* The screen sizes the Graphics menu offers.  The Voodoo2 driver's mode
         * table (0xa92018, 40 bytes a mode: width, height, depth, LFB format,
         * available, then the colour and aux buffer counts sub_a83ac0 works out
         * from the card's memory) holds entries the card never offered, and the
         * last three are sizes no card could open -- 1280x1200, 1600x1200,
         * 400x300 -- so they and the unused 512x256 before them carry the
         * widescreen modes instead, each under a resolution id of the port's
         * own in the table THRASH_setvideomode opens the window by (0xa922e8):
         * 1280x720, 1600x900, 1920x1080, then Dynamic Fullscreen last, the
         * order the menu lists them in (dynamicFullscreenStart).  The Graphics menu lists them once
         * the port's widescreen change lets a 16:9 mode through sub_4bed90, which
         * still applies every other test of its own: 16 bits, at least two
         * colour buffers, and within the driver's memory limit, which at 8 MB
         * leaves 1920x1080 about fifty kilobytes to spare.  The driver's data is
         * in place from construction on -- LoadLibrary does not copy it again --
         * so this sticks. */
        {
            static const struct { nfs::u32 entry, width, height, resolution; } kWide[] = {
                { 12, 1280, 720, nfs3hp::port::driver::kResolution1280x720 },
                { 14, 1600, 900, nfs3hp::port::driver::kResolution1600x900 },
                { 15, 1920, 1080, nfs3hp::port::driver::kResolution1920x1080 },
            };
            for (const auto& wide : kWide)
            {
                const nfs::u32 mode = 0xa92018 + wide.entry * 40;
                app.getMemory<nfs::u32>(mode) = wide.width;
                app.getMemory<nfs::u32>(mode + 4) = wide.height;
                app.getMemory<nfs::u32>(mode + 16) = 1;
                app.getMemory<nfs::u32>(0xa922e8 + wide.entry * 4) = wide.resolution;
            }
            nfs3hp::dynamicFullscreenStart(&app);
        }
        /* 512x384, the one size the list held below the game's own 640x480.  A
         * phone has pixels to spare and every menu is drawn for 640x480, so it
         * leaves the table the same way the widescreen modes join it: by the
         * available flag. */
        app.getMemory<nfs::u32>(0xa92018 + 6 * 40 + 16) = 0;
        nfs3hp::viewAngleStart(&app);

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
extern "C" JNIEXPORT jboolean JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeMoviePlaying(JNIEnv*, jclass)
{
    return nfs3hp::s_moviePlaying.load(std::memory_order_acquire) ? JNI_TRUE : JNI_FALSE;
}
extern "C" JNIEXPORT jboolean JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeSkipMovie(JNIEnv*, jclass)
{
    return nfs3hp::requestMovieSkip() ? JNI_TRUE : JNI_FALSE;
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
    nfs::Gamepad::setSlotDescriptors(a, b);
    if (a)
        env->ReleaseStringUTFChars(first, a);
    if (b)
        env->ReleaseStringUTFChars(second, b);
}
#endif
