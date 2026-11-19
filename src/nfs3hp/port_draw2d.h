#ifndef NFS3HP_PORT_DRAW2D_H
#define NFS3HP_PORT_DRAW2D_H

#include "native_guest.h"
#include "port.h"

/* The game's 2D drawing, rewritten (port.h says how): what the menus, the
 * race's HUD and its pause screens draw with -- rectangles plain, shaded and
 * framed, lines, the 640 by 480 quads scaled to the screen, the screen quad's
 * clip codes; the 2D screens' font made, its characters and texts drawn
 * (upright, turned and scaled, the titles that sweep in); the menus' pointer.
 *
 * Each function is a typed core under its own name and a thunk under the
 * game's, which is bound to the game's address
 * (PORT_DRAW2D_NATIVES): the thunk takes the arguments from the registers
 * and the stack and gives back what Watcom's convention promises the caller
 * -- the result in eax (what the game's code leaves there), every other
 * register as it was, a stdcall's stack words taken off.  No caller of the
 * game's reads more of what these leave (port_draw2d.cpp says where that was
 * looked into).  The cores call each other as C++, and what is not rewritten
 * -- the render state, the clipped quad and line, the 640 by 480 places, the
 * shapes, EA's bitmaps and files -- by address, from where the game's code
 * had its stack.
 *
 * The x87's arithmetic is the game's to the bit: each product and sum through
 * fp::, in the order and with the operands the code has, at single precision;
 * a value stored as a float where the code stores one. */
namespace nfs3hp
{
namespace port
{
namespace draw2d
{

/* ---- The typed cores ---- */

/* The clip codes of the four corners of each of `count` quads -- four vertex
 * numbers each, from `quads` -- among the vertices at `vertices`: the number
 * of quads done. */
int32_t clipCodes(App* app, int32_t count, u32 vertices, u32 quads);

/* The screen quad: the rectangle from (left, top) to (right, bottom), a
 * colour at each corner from the top left clockwise -- placed from 640 by 480
 * near the eye on a race's pause screens, at the 2D screens' depth otherwise. */
Result shadedRectangle(App* app, ThreadState& ts, u32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                       const uint32_t colours[4]);
/* That rectangle shaded from `from` at its left to `to` at its right. */
Result gradient(App* app, ThreadState& ts, u32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                uint32_t from, uint32_t to);
/* The rectangle at (x, y), `width` by `height`, in one colour. */
Result filled(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
/* A frame a pixel wide around it. */
Result outline(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
/* It with a colour at each corner. */
Result shadedBox(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                 const uint32_t colours[4]);
/* The menus' raised panel: it dark, its top and left edges light, its right
 * and bottom black. */
Result panel(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height);
/* A field to type in: it shaded, its edges sunk in. */
Result fieldBox(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                const uint32_t colours[4]);
/* A line from (x0, y0) to (x1, y1), shaded from `from` to `to`. */
Result line(App* app, ThreadState& ts, u32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t from,
            uint32_t to);
// The game's 0x4d8ac0: fieldBox, but nothing on a race's pause screens (x back); esp the frame base.
Result drawFieldBoxOf(App* app, ThreadState& ts, u32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                      const uint32_t colours[4]);
// The game's 0x4d8ed0: line, but nothing on a race's pause screens (x0 back); esp the frame base.
Result drawLineOf(App* app, ThreadState& ts, u32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t from,
                  uint32_t to);
// The game's 0x4d8b80: tileRectangle, but nothing on a race's pause screens (the shape back); esp the frame base.
Result drawTileRectangleOf(App* app, ThreadState& ts, u32 esp, int32_t shape, int32_t left, int32_t right, int32_t top,
                           int32_t bottom, uint32_t from, uint32_t to);
/* A menu shape's first tile's texture over the rectangle from (left, top) to
 * (right, bottom), shaded from `from` at its left to `to` at its right. */
Result tileRectangle(App* app, ThreadState& ts, u32 esp, int32_t shape, int32_t left, int32_t right, int32_t top,
                     int32_t bottom, uint32_t from, uint32_t to);

/* The place of character `c` among the 2D screens' accented ones (0xe0 on). */
Result accented(App* app, ThreadState& ts, u32 esp, u32 c);
/* A character of the 2D screens' font drawn into its bitmap at the font's
 * next place, its square noted under `index`: how wide it is. */
Result glyph(App* app, ThreadState& ts, u32 esp, u32 text, int32_t index);
/* The 2D screens' font made: its characters drawn into a bitmap from the
 * language's .ffn files and their squares noted, the bitmap a texture. */
Result makeScreenFont(App* app, ThreadState& ts, u32 esp);
/* The 2D screens started afresh: the screens' states, the textures, the
 * font, the menu shapes known. */
Result resetScreens(App* app, ThreadState& ts, u32 esp, const Args& a);

/* A character of the 2D screens' font: its square of the texture at (u, v),
 * `width` by `height`, drawn at (x, y) in `colour` -- unless `measure`.  How
 * far on the next one goes. */
Result screenCharacter(App* app, ThreadState& ts, u32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, u32 measure);
/* It turned by `turn` (a cosine, 0x400 a turn) and scaled by `scale`. */
Result turnedCharacter(App* app, ThreadState& ts, u32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, u32 measure, int32_t turn, uint32_t scale);
/* A text on a 2D screen at (x, y) -- its width, or 0 on a race's pause
 * screens; `measure` only measures it. */
Result screenText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, u32 measure);
/* It turned and scaled, in a colour of its own. */
Result turnedText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, uint32_t scale, int32_t turn,
                  uint32_t colour);
/* A screen's title sweeping in, a character at a time: 1 while it does, 0
 * once it is through and on a card with alpha; `start` starts it again. */
Result titleText(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, u32 start);

/* A quad of the menus' art in 640 by 480 measure, scaled to the screen. */
struct ArtQuad
{
    int32_t x[4], y[4];
    uint32_t u[4], v[4];  // floats' bits
    uint32_t texture, colour;
};
Result texturedQuad(App* app, ThreadState& ts, u32 esp, const ArtQuad& quad);
/* A quad in 640 by 480 measure, a colour at each corner. */
struct ShadedQuad
{
    int32_t x[4], y[4];
    uint32_t colour[4];
};
Result shadedQuad(App* app, ThreadState& ts, u32 esp, const ShadedQuad& quad);
/* The menus' pointer, its picture moved on with the clock. */
Result pointer(App* app, ThreadState& ts, u32 esp);

/* ---- The thunks, under the game's names ---- */

nfs::u32 quadOutcodes(App* app, ThreadState& ts, nfs::Args& args);          // 0x4d5840
nfs::u32 drawShadedRectangle(App* app, ThreadState& ts, nfs::Args& args);   // 0x4d81b0
nfs::u32 gradientRectangle(App* app, ThreadState& ts, nfs::Args& args);     // 0x4d8550
nfs::u32 drawShadedBox(App* app, ThreadState& ts, nfs::Args& args);         // 0x4d8860
nfs::u32 filledRectangle(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d8910
nfs::u32 frame(App* app, ThreadState& ts, nfs::Args& args);                 // 0x4d8930
nfs::u32 drawPanel(App* app, ThreadState& ts, nfs::Args& args);             // 0x4d89f0
nfs::u32 drawFieldBox(App* app, ThreadState& ts, nfs::Args& args);          // 0x4d8ac0
nfs::u32 drawTileRectangle(App* app, ThreadState& ts, nfs::Args& args);     // 0x4d8b80
nfs::u32 drawLine(App* app, ThreadState& ts, nfs::Args& args);              // 0x4d8ed0
nfs::u32 fontCharacter(App* app, ThreadState& ts, nfs::Args& args);         // 0x4d9240
nfs::u32 accentedCharacter(App* app, ThreadState& ts, nfs::Args& args);     // 0x4d9320
nfs::u32 buildScreenFont(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d93a0
nfs::u32 drawScreenCharacter(App* app, ThreadState& ts, nfs::Args& args);   // 0x4d95c0
nfs::u32 drawTurnedCharacter(App* app, ThreadState& ts, nfs::Args& args);   // 0x4d9b90
nfs::u32 drawTurnedText(App* app, ThreadState& ts, nfs::Args& args);        // 0x4da120
nfs::u32 drawTitleText(App* app, ThreadState& ts, nfs::Args& args);         // 0x4da1e0
nfs::u32 drawScreenText(App* app, ThreadState& ts, nfs::Args& args);        // 0x4da4a0
nfs::u32 drawPointer(App* app, ThreadState& ts, nfs::Args& args);           // 0x4dab90
// The game's 0x4dab90: the menus' pointer drawn; esp the frame base (the callee's entry esp).
Result pointerDrawn(App* app, ThreadState& ts, u32 esp);
nfs::u32 startScreens(App* app, ThreadState& ts, nfs::Args& args);          // 0x4daf80
nfs::u32 drawTexturedQuad(App* app, ThreadState& ts, nfs::Args& args);      // 0x4db000
nfs::u32 drawShadedQuad(App* app, ThreadState& ts, nfs::Args& args);        // 0x4db380

}  // namespace draw2d
}  // namespace port

/* How the game's code calls by address the cores the menus call as C++ (guest::direct): shadedRectangle (0x4d81b0), gradient (0x4d8550), shadedBox (0x4d8860), filled (0x4d8910), outline
 * (0x4d8930), drawFieldBoxOf (0x4d8ac0), drawLineOf (0x4d8ed0), resetScreens (0x4daf80), texturedQuad
 * (0x4db000).  A Result is nullopt when the game stopped in a call.  A core's `esp` is the frame base its
 * thunk hands it: the game's esp on entry, at the return address.  A caller through guest::direct passes
 * ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the thunk hands it, so the core's
 * frame lies where the game's does; by address the call makes it. */
namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
