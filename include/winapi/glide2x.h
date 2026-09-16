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

/* The texture atlas: free texels, whole 256x256 tiles among them, and its size
 * in texels.  False while no Glide window is open. */
bool atlasFreeSpace(x86::reg32& freeTexels, x86::reg32& freeTiles, x86::reg32& totalTexels);

/* Texture atlas size for the next Glide window: 2048, what the game's own
 * textures were made for, or 4096 for a game layer that asks for more of them
 * at full size.  The renderer still takes 2048 where the GPU cannot do 4096. */
void setPreferredAtlasSize(x86::reg32 size);

/* A screen resolution Glide never had, 1280x720: the id a game layer puts in
 * its driver's mode table, and grSstWinOpen opens.  Above every
 * GR_RESOLUTION_* of Glide 2 and 3. */
static const x86::reg32 kResolution1280x720 = 0x80;

}}

#endif
