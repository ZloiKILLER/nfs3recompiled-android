#ifndef glide2x_H_
#define glide2x_H_

#include <x86.h>
#include <winapi/types.h>
#include <vector>

namespace win32
{

class WinApplication;
class Library;
struct GrVertex;
class ThrashRenderer;

}

namespace win32 { namespace glide2x
{

extern Library* s_glide2xRegistry;

/* Called on the game thread after every buffer swap, with the guest context
 * held, so a game-specific layer can look at the game's state once a frame
 * without this one knowing anything about it.  One observer; null removes it. */
using SwapObserver = void (*)(WinApplication* app);
void setSwapObserver(SwapObserver observer);

/* Where the game's picture sits inside the window, in window pixels, and the
 * size of the picture itself.  This is the letterbox rectangle the renderer
 * blits into, so it is what turns a touch on the screen into a pixel of the
 * game.  False while no Glide window is open. */
bool screenRect(int& x, int& y, int& w, int& h, int& pictureWidth, int& pictureHeight);

/* The texture atlas: free texels, whole 256x256 tiles among them, and its size
 * in texels.  False while no Glide window is open. */
bool atlasFreeSpace(x86::reg32& freeTexels, x86::reg32& freeTiles, x86::reg32& totalTexels);

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
void clearPicture(WinApplication* app, x86::CPU& cpu);

/* Texture atlas size for the next Glide window: 2048, what the game's own
 * textures were made for, or 4096 for a game layer that asks for more of them
 * at full size.  The renderer still takes 2048 where the GPU cannot do 4096. */
void setPreferredAtlasSize(x86::reg32 size);

/* The next Glide window draws through ThrashRenderer, the native THRASH
 * driver's own renderer, rather than the Voodoo2 emulated over an atlas.
 * Only for a game whose driver calls are all native: the generated driver
 * code would reach Glide functions ThrashRenderer does not stand behind. */
void useThrashRenderer(bool on);

/* Screen resolutions Glide never had: the ids a game layer puts in its
 * driver's mode table, and grSstWinOpen opens.  Above every
 * GR_RESOLUTION_* of Glide 2 and 3. */
static const x86::reg32 kResolution1280x720 = 0x80;
static const x86::reg32 kResolution1600x900 = 0x81;
static const x86::reg32 kResolution1920x1080 = 0x82;

/* A texture format Glide 2 never had: eight bits a channel, numbered as Glide 3
 * numbers it for the Voodoo 4 and 5 (GR_TEXFMT_ARGB_8888), each texel a
 * little-endian 0xAARRGGBB.  A game layer maps its driver's 32-bit format to
 * it, and the texels reach the atlas as they are.  Offered unless
 * NFS_TEXTURES32 is 0 (fullColourTextures). */
static const x86::reg32 kTexFmtArgb8888 = 0x12;
bool fullColourTextures();

/* grDrawTriangle for a native stand-in that builds the vertices itself (the
 * game's THRASH_drawtri): the same fitting, the same renderer.  Nothing when no
 * Glide window is open. */
void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c);

/* grDrawLine for a native stand-in, likewise (the game's THRASH_drawline). */
void drawLine(const GrVertex* a, const GrVertex* b);

/* NFS_NATIVE_CHECK: while set, every triangle either path hands over is also
 * appended here, three vertices each, as the caller passed them; a line as its
 * two ends and the second again. */
void traceTriangles(std::vector<GrVertex>* trace);
/* Whether a trace is being taken: a check nested in another must not take it over. */
bool tracingTriangles();

/* NFS_NATIVE_CHECK: while set, every Glide state call either path makes is
 * appended here: its offset in voodoo2a's table of Glide functions (0x60
 * grColorCombine, ...), its arguments, then 0xffffffff. */
void traceCalls(std::vector<x86::reg32>* trace);

/* Glide's state calls for a native stand-in (native_thrash.cpp), made straight
 * rather than through a guest call: the same functions voodoo2a reaches
 * through its table. */
namespace direct
{
void chromakeyValue(WinApplication* app, x86::CPU& cpu, x86::reg32 color);
void colorCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 function, x86::reg32 factor, x86::reg32 local,
                  x86::reg32 other, x86::reg32 invert);
void alphaCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 function, x86::reg32 factor, x86::reg32 local,
                  x86::reg32 other, x86::reg32 invert);
void alphaBlendFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 rgbSource, x86::reg32 rgbDestination,
                        x86::reg32 alphaSource, x86::reg32 alphaDestination);
void cullMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
void texFilterMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 minification,
                   x86::reg32 magnification);
void ditherMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
void chromakeyMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
void alphaTestFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 function);
void depthBufferMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
void depthBufferFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 function);
void depthMask(WinApplication* app, x86::CPU& cpu, x86::reg32 mask);
void fogColorValue(WinApplication* app, x86::CPU& cpu, x86::reg32 colour);
void fogMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
void fogTable(WinApplication* app, x86::CPU& cpu, const x86::reg8* table);
void gammaCorrectionValue(WinApplication* app, x86::CPU& cpu, float gamma);
void depthBiasLevel(WinApplication* app, x86::CPU& cpu, x86::sreg16 bias);
void texMipMapMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 mode, x86::reg32 lodBlend);
void texCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 rgbFunction, x86::reg32 rgbFactor,
                x86::reg32 alphaFunction, x86::reg32 alphaFactor, x86::reg32 rgbInvert, x86::reg32 alphaInvert);
void texClampMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 s, x86::reg32 t);
void texLodBiasValue(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, float bias);
x86::reg32 texCalcMemRequired(WinApplication* app, x86::CPU& cpu, x86::reg32 smallLod, x86::reg32 largeLod,
                              x86::reg32 aspect, x86::reg32 format);
x86::reg32 texMinAddress(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu);
x86::reg32 texMaxAddress(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu);
/* `info`: the guest address of a GrTexInfo, as voodoo2a passes its records. */
void texSource(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 start, x86::reg32 evenOdd, x86::reg32 info);
void texDownloadMipMap(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 start, x86::reg32 evenOdd,
                       x86::reg32 info);
void texDownloadTable(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 type, x86::reg32 data);
void renderBuffer(WinApplication* app, x86::CPU& cpu, x86::reg32 buffer);
void bufferClear(WinApplication* app, x86::CPU& cpu, x86::reg32 colour, x86::reg8 alpha, x86::reg16 depth);
void bufferSwap(WinApplication* app, x86::CPU& cpu, x86::reg32 interval);
x86::sreg32 bufferNumPending(WinApplication* app, x86::CPU& cpu);
x86::reg32 sstStatus(WinApplication* app, x86::CPU& cpu);
x86::reg32 sstVRetraceOn(WinApplication* app, x86::CPU& cpu);
void sstIdle(WinApplication* app, x86::CPU& cpu);
void sstIsBusy(WinApplication* app, x86::CPU& cpu);
void clipWindow(WinApplication* app, x86::CPU& cpu, x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY);
/* `info` and `data`: guest addresses. */
x86::reg32 lfbLock(WinApplication* app, x86::CPU& cpu, x86::reg32 type, x86::reg32 buffer, x86::reg32 writeMode,
                   x86::reg32 origin, x86::reg32 pixelPipeline, x86::reg32 info);
x86::reg32 lfbUnlock(WinApplication* app, x86::CPU& cpu, x86::reg32 type, x86::reg32 buffer);
x86::reg32 lfbReadRegion(WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 x, x86::reg32 y,
                         x86::reg32 width, x86::reg32 height, x86::reg32 stride, x86::reg32 data);
/* THRASH_treset: the renderer gives up every texture (ThrashRenderer). */
void resetTextures();
/* The ThrashRenderer, when a triangle may go to it as it is -- drawn by it,
 * not traced, not narrowed or fitted to the screen -- and so straight from
 * the game's vertices (ThrashRenderer::drawCorners); null otherwise. */
ThrashRenderer* thrashRenderer();
x86::reg32 sstControl(WinApplication* app, x86::CPU& cpu, x86::reg32 code);
}

}}

#endif
