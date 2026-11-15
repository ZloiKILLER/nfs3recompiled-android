#ifndef NFS3HP_PORT_THRASH_H
#define NFS3HP_PORT_THRASH_H

#include "port.h"

#include <optional>
#include <vector>

/* The THRASH driver, the Voodoo2's (voodoo2a.dll), as the port's own C++
 * (port_thrash.cpp): the game's 34 THRASH_* calls, which
 * disassemble_nfs3hp.py binds, drawing straight through ThrashRenderer
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

/* A vertex as Glide had it: what the native vertex checks (NFS_NATIVE_CHECK,
 * native_vertices.cpp) trace of what the game hands the driver -- the fields
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
void clearPicture(App* app, CPU& cpu);

/* Screen resolutions the Voodoo2 never had: the ids the game layer puts in the
 * driver's mode table, beside Glide's GR_RESOLUTION_* ids, and
 * THRASH_setvideomode opens.  Above every one of Glide 2's and 3's. */
constexpr reg32 kResolution1280x720 = 0x80;
constexpr reg32 kResolution1600x900 = 0x81;
constexpr reg32 kResolution1920x1080 = 0x82;
/* Dynamic Fullscreen: a size the game layer works out from the screen it runs
 * on and sets (setDynamicResolution) before the window opens. */
constexpr reg32 kResolutionDynamic = 0x83;
void setDynamicResolution(reg32 width, reg32 height);

/* A texture format the Voodoo2 never had: eight bits a channel, numbered as
 * Glide 3 numbers it for the Voodoo 4 and 5 (GR_TEXFMT_ARGB_8888), each texel
 * a little-endian 0xAARRGGBB.  THRASH_about maps the game's 32-bit format to
 * it (tools/apply_full_colour.py), and the texels reach the renderer as they
 * are.  Offered unless NFS_TEXTURES32 is 0 (fullColourTextures). */
constexpr reg32 kTexFmtArgb8888 = 0x12;
bool fullColourTextures();

/* A texture side the Voodoo2 never had: 512, past Glide 2's GR_LOD_256 (0).
 * THRASH_about offers it, its LOD is one less than 256's (-1), and the
 * renderer takes a negative LOD as a side of 256 doubled.  A track's
 * textures are asked for at 512, not the game's 128 (native_track.cpp), so
 * each keeps its own size up to it.  The game's other limits (256 for a car)
 * stand for art of up to 256; a shape or picture of 512 a side or more is
 * made a texture of 512 (texture512Side). */
constexpr reg32 kMaxTextureSide = 0x200;
inline int32_t texture512Side(int32_t limit, int32_t side)
{
    return limit != 0 && limit < int32_t(kMaxTextureSide) && side >= int32_t(kMaxTextureSide)
               ? int32_t(kMaxTextureSide) : limit;
}
/* A texture record, Glide's GrTexInfo and voodoo2a's three words after it. */
constexpr reg32 kRecordSmallLod = 0x00, kRecordLargeLod = 0x04, kRecordAspect = 0x08, kRecordFormat = 0x0c;
constexpr reg32 kRecordTexels = 0x10, kRecordStart = 0x14, kRecordHalves = 0x18, kRecordBefore = 0x1c;
constexpr reg32 kRecordBytes = 0x20;
/* voodoo2a's maps to Glide's numbers: an LOD by a size's log2, an aspect by
 * the two logs' difference (from -3, at kAspects - 3). */
constexpr reg32 kLods = 0xa92338;
constexpr reg32 kAspects = 0xa92344;

/* What of the textures touches only the game's memory, in
 * port_thrash_records.cpp, without the renderer, so that tools/game_checks
 * builds it with the natives that call it. */

/* log2 of a power of two, from the exponent of the float it converts to. */
int32_t exponentOf(reg32 value);
/* Glide's LOD of a side of 2 to the `log`: voodoo2a's table, and the port's
 * 512 (log 9) past it, one LOD under 256's. */
reg32 lodOf(App* app, int32_t log);
/* THRASH texture record `record`, of one level, made `side` a side: a
 * texture allocated before its picture was fitted (the car screens' one,
 * 256 a side, which a .tga of 512 fills at 512).  A record of more levels is
 * left as it is. */
void resizeTexture(App* app, reg32 record, reg32 side);

/* The 34 exports as C++ (port_thrash.cpp): each one's words as its
 * parameters, in their order (thrash.h), and its result -- what voodoo2a
 * leaves in eax.  The THRASH_* adapters the game's pointers lead to take the
 * words off the stack, call these and pop the words; native code calls them
 * through the game's pointers, guest::driver (thrash_calls.h).
 *
 * An export that gives nothing back (Glide's void ones) leaves eax as its
 * caller had it: its function returns cpu.eax.  Those that call the game's
 * own code -- its lambdas, voodoo2a's malloc and free -- give nullopt when
 * the game stopped in it.  The registers a caller left that voodoo2a reads
 * besides (ebx for setVideoMode and restore, esp for is) are read from cpu as
 * the export finds them. */

reg32 about(App* app, CPU& cpu);
reg32 clearWindow(App* app, CPU& cpu);
reg32 clip(App* app, CPU& cpu, reg32 minX, reg32 minY, reg32 maxX, reg32 maxY);
reg32 drawLine(App* app, CPU& cpu, reg32 a, reg32 b);
reg32 drawLineMesh(App* app, CPU& cpu, reg32 count, reg32 vertices, reg32 lines);
reg32 drawLineStrip(App* app, CPU& cpu, reg32 count, reg32 vertices);
reg32 drawPoint(App* app, CPU& cpu, reg32 vertex);
reg32 drawPointMesh(App* app, CPU& cpu, reg32 count, reg32 vertices, reg32 points);
reg32 drawPointStrip(App* app, CPU& cpu, reg32 count, reg32 vertices);
reg32 drawQuad(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c, reg32 d);
reg32 drawQuadMesh(App* app, CPU& cpu, reg32 count, reg32 vertices, reg32 quads);
reg32 drawTri(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c);
reg32 drawTriFan(App* app, CPU& cpu, reg32 count, reg32 vertices);
reg32 drawTriMesh(App* app, CPU& cpu, reg32 count, reg32 vertices, reg32 triangles);
/* The strip's argument voodoo2a writes back (port_thrash.cpp) is the
 * adapter's: there is none here. */
reg32 drawTriStrip(App* app, CPU& cpu, reg32 count, reg32 vertices);
reg32 flushWindow(App* app, CPU& cpu);
reg32 idle(App* app, CPU& cpu);
reg32 init(App* app, CPU& cpu);
reg32 is(App* app, CPU& cpu);
std::optional<reg32> lockWindow(App* app, CPU& cpu);
reg32 pageFlip(App* app, CPU& cpu);
std::optional<reg32> readRect(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 pixels);
std::optional<reg32> restore(App* app, CPU& cpu);
reg32 selectDisplay(App* app, CPU& cpu, reg32 board);
/* The argument voodoo2a writes back for thrash::kMipLinear is the
 * adapter's, as with drawTriStrip. */
reg32 setState(App* app, CPU& cpu, reg32 state, reg32 value);
reg32 setTexture(App* app, CPU& cpu, reg32 texture);
std::optional<reg32> setVideoMode(App* app, CPU& cpu, reg32 mode, reg32 buffers, reg32 auxiliary);
reg32 sync(App* app, CPU& cpu, reg32 what);
std::optional<reg32> tAlloc(App* app, CPU& cpu, reg32 width, reg32 height, reg32 format, reg32 palette,
                            reg32 levels);
std::optional<reg32> tReset(App* app, CPU& cpu);
reg32 tUpdate(App* app, CPU& cpu, reg32 texture, reg32 texels, reg32 palette);
std::optional<reg32> unlockWindow(App* app, CPU& cpu, reg32 lock);
reg32 window(App* app, CPU& cpu, reg32 buffer);
std::optional<reg32> writeRect(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 pixels);

}  // namespace driver
}  // namespace port
}  // namespace nfs3hp

#endif
