#ifndef NFS3HP_PORT_THRASH_H
#define NFS3HP_PORT_THRASH_H

#include "port.h"

#include <optional>
#include <vector>

/* The THRASH driver, the Voodoo2's (voodoo2a.dll), as the port's own C++
 * (port_thrash.cpp): the game's 34 THRASH_* calls, drawing straight through ThrashRenderer
 * (lib/thrashrenderer.h) -- no Glide under it, no DirectDraw, no Win32.  What
 * the game sees of the driver stays where voodoo2a has it: its descriptor and
 * mode table, its records and its variables, at their addresses in the DLL's
 * data.
 *
 * Here, besides, what the game layer (nfs3hp_main.cpp) asks of the picture
 * the driver draws. */
namespace nfs3hp
{
namespace port
{
namespace driver
{

/* A vertex as Glide had it: what the game hands the driver (native_vertices.cpp) -- the fields
 * voodoo2a set, the others 0. */
struct GrTmuVertex
{
    float sow;
    float tow;
    float oow;
};

struct GrVertex
{
    float x, y, z;
    float r, g, b;
    float ooz;
    float a;
    float oow;
    GrTmuVertex tmuvtx[2];
};

/* While set, every triangle the driver draws is also appended here, three
 * vertices each, as the game's vertices made them; a line as its two ends and
 * the second again. */
void traceTriangles(std::vector<GrVertex>* trace);
/* Whether a trace is being taken: a check nested in another must not take it
 * over. */
bool tracingTriangles();

/* Called on the game thread after every buffer swap, with the guest context
 * held, so the game layer can look at the game's state once a frame.  One
 * observer; null removes it. */
using SwapObserver = void (*)(App* app);
void setSwapObserver(SwapObserver observer);

/* Where the game's picture sits inside the window, in window pixels, and the
 * size of the picture itself: the letterbox rectangle the frame is shown in,
 * which turns a touch on the screen into a pixel of the game.  False while no
 * window is open. */
bool screenRect(int& x, int& y, int& w, int& h, int& pictureWidth, int& pictureHeight);

/* 4:3 in the middle of a wider picture: while on, everything the game draws is
 * squeezed sideways into the 4:3 rectangle centred on the picture, as it would
 * have been drawn on a 4:3 screen.  For a screen the game only knows how to
 * stretch.  No effect on a picture no wider than 4:3. */
void fitFourThree(bool on);

/* While on, every triangle is narrowed by the same factor towards its own left
 * corner: a picture of its own, like a dialog's emblem, keeps its shape on a
 * wide screen while everything around it stays as the game stretched it. */
void squeezeToLeft(bool on);

/* The whole picture black, whatever clip window the game has set: the bars
 * beside a 4:3 rectangle, before it is drawn. */
void clearPicture(App* app, ThreadState& ts);

/* Screen resolutions the Voodoo2 never had: the ids the game layer puts in the
 * driver's mode table, beside Glide's GR_RESOLUTION_* ids, and
 * THRASH_setvideomode opens.  Above every one of Glide 2's and 3's. */
constexpr u32 kResolution1280x720 = 0x80;
constexpr u32 kResolution1600x900 = 0x81;
constexpr u32 kResolution1920x1080 = 0x82;
/* Dynamic Fullscreen: a size the game layer works out from the screen it runs
 * on and sets (setDynamicResolution) before the window opens. */
constexpr u32 kResolutionDynamic = 0x83;
void setDynamicResolution(u32 width, u32 height);

/* A texture format the Voodoo2 never had: eight bits a channel, numbered as
 * Glide 3 numbers it for the Voodoo 4 and 5 (GR_TEXFMT_ARGB_8888), each texel
 * a little-endian 0xAARRGGBB.  THRASH_about maps the game's 32-bit format to
 * it (the port's full colour change), and the texels reach the renderer as they
 * are. */
constexpr u32 kTexFmtArgb8888 = 0x12;

/* A texture side the Voodoo2 never had: 512, past Glide 2's GR_LOD_256 (0).
 * THRASH_about offers it, its LOD is one less than 256's (-1), and the
 * renderer takes a negative LOD as a side of 256 doubled.  A track's
 * textures are asked for at 512, not the game's 128 (native_track.cpp), so
 * each keeps its own size up to it.  The game's other limits (256 for a car)
 * stand for art of up to 256; a shape or picture of 512 a side or more is
 * made a texture of 512 (texture512Side). */
constexpr u32 kMaxTextureSide = 0x200;
inline int32_t texture512Side(int32_t limit, int32_t side)
{
    return limit != 0 && limit < int32_t(kMaxTextureSide) && side >= int32_t(kMaxTextureSide)
               ? int32_t(kMaxTextureSide) : limit;
}
/* A texture record, Glide's GrTexInfo and voodoo2a's three words after it. */
constexpr u32 kRecordSmallLod = 0x00, kRecordLargeLod = 0x04, kRecordAspect = 0x08, kRecordFormat = 0x0c;
constexpr u32 kRecordTexels = 0x10, kRecordStart = 0x14, kRecordHalves = 0x18, kRecordBefore = 0x1c;
constexpr u32 kRecordBytes = 0x20;
/* voodoo2a's maps to Glide's numbers: an LOD by a size's log2, an aspect by
 * the two logs' difference (from -3, at kAspects - 3). */
constexpr u32 kLods = 0xa92338;
constexpr u32 kAspects = 0xa92344;

/* What of the textures touches only the game's memory, in
 * port_thrash_records.cpp, without the renderer. */

/* log2 of a power of two, from the exponent of the float it converts to. */
int32_t exponentOf(u32 value);
/* Glide's LOD of a side of 2 to the `log`: voodoo2a's table, and the port's
 * 512 (log 9) past it, one LOD under 256's. */
u32 lodOf(App* app, int32_t log);
/* THRASH texture record `record`, of one level, made `side` a side: a
 * texture allocated before its picture was fitted (the car screens' one,
 * 256 a side, which a .tga of 512 fills at 512).  A record of more levels is
 * left as it is. */
void resizeTexture(App* app, u32 record, u32 side);

/* The 34 exports as C++ (port_thrash.cpp): each one's words as its
 * parameters, in their order (thrash.h), and its result -- what voodoo2a
 * leaves in eax.  The THRASH_* thunks the game's pointers lead to take the
 * words off the stack, call these and pop the words; native code calls them
 * through the game's pointers, guest::driver (thrash_calls.h).
 *
 * An export that gives nothing back (Glide's void ones) leaves eax as its
 * caller had it: its thunk returns args.eax.  Those that call the game's
 * own code -- its lambdas, voodoo2a's malloc and free -- give nullopt when
 * the game stopped in it.  The registers a caller left that voodoo2a reads
 * besides (ebx for setVideoMode and restore, esp for is) are read from ts as
 * the export finds them. */

u32 about(App* app, ThreadState& ts);
u32 clearWindow(App* app, ThreadState& ts);
u32 clip(App* app, ThreadState& ts, u32 minX, u32 minY, u32 maxX, u32 maxY);
void drawLine(App* app, ThreadState& ts, u32 a, u32 b);
void drawLineMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 lines);
void drawLineStrip(App* app, ThreadState& ts, u32 count, u32 vertices);
void drawPoint(App* app, ThreadState& ts, u32 vertex);
void drawPointMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 points);
void drawPointStrip(App* app, ThreadState& ts, u32 count, u32 vertices);
void drawQuad(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d);
void drawQuadMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 quads);
void drawTri(App* app, ThreadState& ts, u32 a, u32 b, u32 c);
void drawTriFan(App* app, ThreadState& ts, u32 count, u32 vertices);
void drawTriMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 triangles);
/* The strip's argument voodoo2a writes back (port_thrash.cpp) is the
 * thunk's: there is none here. */
void drawTriStrip(App* app, ThreadState& ts, u32 count, u32 vertices);
void flushWindow(App* app, ThreadState& ts);
void idle(App* app, ThreadState& ts);
u32 init(App* app, ThreadState& ts);
u32 is(App* app, ThreadState& ts);
std::optional<u32> lockWindow(App* app, ThreadState& ts);
void pageFlip(App* app, ThreadState& ts);
std::optional<u32> readRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 pixels);
std::optional<u32> restore(App* app, ThreadState& ts);
u32 selectDisplay(App* app, ThreadState& ts, u32 board);
/* The argument voodoo2a writes back for thrash::kMipLinear is the
 * thunk's, as with drawTriStrip. */
u32 setState(App* app, ThreadState& ts, u32 state, u32 value);
u32 setTexture(App* app, ThreadState& ts, u32 texture);
std::optional<u32> setVideoMode(App* app, ThreadState& ts, u32 mode, u32 buffers, u32 auxiliary);
/* With the ebx the export finds (what it gives back for no buffers): the thunk's. */
std::optional<u32> setVideoModeEbx(App* app, ThreadState& ts, u32 ebx, u32 mode, u32 buffers, u32 auxiliary);
u32 sync(App* app, ThreadState& ts, u32 what);
std::optional<u32> tAlloc(App* app, ThreadState& ts, u32 width, u32 height, u32 format, u32 palette,
                            u32 levels);
std::optional<u32> tReset(App* app, ThreadState& ts);
u32 tUpdate(App* app, ThreadState& ts, u32 texture, u32 texels, u32 palette);
std::optional<u32> unlockWindow(App* app, ThreadState& ts, u32 lock);
u32 window(App* app, ThreadState& ts, u32 buffer);
std::optional<u32> writeRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 pixels);

}  // namespace driver
}  // namespace port
}  // namespace nfs3hp

#endif
