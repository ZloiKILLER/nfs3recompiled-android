#ifndef NFS3HP_OS_VIDEO_H
#define NFS3HP_OS_VIDEO_H

#include "os_system.h"

#include <game_types.h>
#include <lib/app.h>
#include <lib/palette.h>

#include <array>
#include <cstdint>
#include <optional>

/* The port's display, as the game's drawing code needs it (EA's display library, the menus and the
 * movies): a display shown in the game's window, the modes it tells of and the one it is in, its
 * screen and the buffer behind it to lock and draw into, and a palette.  Each is a port object the
 * game holds by its handle (os::Handle) where it held DirectDraw's objects; os::video::release lets
 * one go.  Plain C++ on src/lib's Renderer, Surface and Palette. */
namespace nfs3hp
{
namespace os
{
namespace video
{

/* A picture's layout: its size, the bytes a row, the bits a pixel and, at 16 bits, where red,
 * green and blue are in one (all 0 at 8 bits, where a pixel is a palette's entry), and how often
 * the screen shows it.  Every mode the port tells of has all of these. */
struct Format
{
    nfs::u32 width;
    nfs::u32 height;
    std::int32_t pitch;
    nfs::u32 bits;
    nfs::u32 redMask;
    nfs::u32 greenMask;
    nfs::u32 blueMask;
    nfs::u32 refresh;
};

/* The modes the display tells of, in its order: 640x480 at 16 bits and at 8, then both again (the
 * 800x600 ones it lists have always been told as these). */
const std::array<Format, 4>& modes();

/* The mode the display is in: one for the whole port, 640x480 at 16 bits until a mode is set.
 * Every surface of the display is laid out as it. */
Format mode();

/* A display, showing nothing yet: its handle. */
Handle openDisplay(App* app);

/* The display `display` shown in the window `window` (os::window's); nothing for window 0. */
void showIn(App* app, Handle display, Handle window);

/* The display put in a mode of `bits` a pixel, `width` by `height`: what mode() tells is 640x480
 * at those bits, whatever the size asked. */
void setMode(App* app, Handle display, nfs::u32 width, nfs::u32 height, nfs::u32 bits);

/* A surface of the display, whatever is asked of it: the one picture the display shows. */
Handle makeSurface(App* app, Handle display);

/* The buffer behind the surface `surface`, drawn into while it shows. */
Handle backBuffer(App* app, Handle surface);

/* A surface locked to draw into: where its pixels are in the game's memory, and their layout. */
struct Locked
{
    nfs::u32 pixels;
    Format format;
};

/* The surface `surface` locked.  The port's surfaces are never lost: a lock always comes, and
 * nullopt is only for a handle that is no surface of the display's. */
std::optional<Locked> lock(App* app, Handle surface);

/* The surface `surface` unlocked: what was drawn is the port's to show.  The guest context is let
 * go meanwhile. */
void unlock(App* app, nfs::ThreadState& ts, Handle surface);

/* A palette's colour, as the game keeps it: src/lib's. */
using Colour = nfs::Colour;

/* A palette of the display's of 256 colours, the first from `colours`: its handle. */
Handle makePalette(App* app, Handle display, const Colour* colours);

/* `count` of the palette's colours from `first` on set from `colours`. */
void setPalette(App* app, Handle palette, nfs::u32 first, nfs::u32 count, const Colour* colours);

/* A display, a surface or a palette let go; 0, or one let go already, is none. */
void release(App* app, Handle handle);

}  // namespace video
}  // namespace os
}  // namespace nfs3hp

#endif
