#ifndef glide2x_H_
#define glide2x_H_

#include <x86.h>
#include <winapi/types.h>

namespace win32
{

class WinApplication;
class Library;

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

/* The whole picture black, whatever clip window the game has set: the bars
 * beside a 4:3 rectangle, before it is drawn. */
void clearPicture(WinApplication* app, x86::CPU& cpu);

/* Texture atlas size for the next Glide window: 2048, what the game's own
 * textures were made for, or 4096 for a game layer that asks for more of them
 * at full size.  The renderer still takes 2048 where the GPU cannot do 4096. */
void setPreferredAtlasSize(x86::reg32 size);

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

}}

#endif
