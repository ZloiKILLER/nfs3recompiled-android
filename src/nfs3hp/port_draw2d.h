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
 * Each function is a typed core under its own name and an adapter under the
 * game's, which disassemble_nfs3hp.py binds to the game's address
 * (PORT_DRAW2D_NATIVES): the adapter takes the arguments from the registers
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
 * cpu.fpu, in the order and with the operands the code has, under the control
 * word as it is; a value stored as a float where the code stores one. */
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
int32_t clipCodes(App* app, int32_t count, reg32 vertices, reg32 quads);

/* The screen quad: the rectangle from (left, top) to (right, bottom), a
 * colour at each corner from the top left clockwise -- placed from 640 by 480
 * near the eye on a race's pause screens, at the 2D screens' depth otherwise. */
Result shadedRectangle(App* app, CPU& cpu, reg32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                       const uint32_t colours[4]);
/* That rectangle shaded from `from` at its left to `to` at its right. */
Result gradient(App* app, CPU& cpu, reg32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                uint32_t from, uint32_t to);
/* The rectangle at (x, y), `width` by `height`, in one colour. */
Result filled(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
/* A frame a pixel wide around it. */
Result outline(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
/* It with a colour at each corner. */
Result shadedBox(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                 const uint32_t colours[4]);
/* The menus' raised panel: it dark, its top and left edges light, its right
 * and bottom black. */
Result panel(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height);
/* A field to type in: it shaded, its edges sunk in. */
Result fieldBox(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                const uint32_t colours[4]);
/* A line from (x0, y0) to (x1, y1), shaded from `from` to `to`. */
Result line(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t from,
            uint32_t to);
// The game's 0x4d8ac0: fieldBox, but nothing on a race's pause screens (x back); esp the frame base.
Result drawFieldBoxOf(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                      const uint32_t colours[4]);
// The game's 0x4d8ed0: line, but nothing on a race's pause screens (x0 back); esp the frame base.
Result drawLineOf(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t from,
                  uint32_t to);
/* A menu shape's first tile's texture over the rectangle from (left, top) to
 * (right, bottom), shaded from `from` at its left to `to` at its right. */
Result tileRectangle(App* app, CPU& cpu, reg32 esp, int32_t shape, int32_t left, int32_t right, int32_t top,
                     int32_t bottom, uint32_t from, uint32_t to);

/* The place of character `c` among the 2D screens' accented ones (0xe0 on). */
Result accented(App* app, CPU& cpu, reg32 esp, reg32 c);
/* A character of the 2D screens' font drawn into its bitmap at the font's
 * next place, its square noted under `index`: how wide it is.  ebx and ecx
 * as the caller has them, for EA's calls. */
Result glyph(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t index, reg32 ebx, reg32 ecx);
/* The 2D screens' font made: its characters drawn into a bitmap from the
 * language's .ffn files and their squares noted, the bitmap a texture. */
Result makeScreenFont(App* app, CPU& cpu, reg32 esp);
/* The 2D screens started afresh: the screens' states, the textures, the
 * font, the menu shapes known. */
Result resetScreens(App* app, CPU& cpu, reg32 esp, const Args& a);

/* A character of the 2D screens' font: its square of the texture at (u, v),
 * `width` by `height`, drawn at (x, y) in `colour` -- unless `measure`.  How
 * far on the next one goes. */
Result screenCharacter(App* app, CPU& cpu, reg32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, reg32 measure);
/* It turned by `turn` (a cosine, 0x400 a turn) and scaled by `scale`. */
Result turnedCharacter(App* app, CPU& cpu, reg32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, reg32 measure, int32_t turn, uint32_t scale);
/* A text on a 2D screen at (x, y) -- its width, or 0 on a race's pause
 * screens; `measure` only measures it. */
Result screenText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 measure);
/* It turned and scaled, in a colour of its own. */
Result turnedText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, uint32_t scale, int32_t turn,
                  uint32_t colour);
/* A screen's title sweeping in, a character at a time: 1 while it does, 0
 * once it is through and on a card with alpha; `start` starts it again. */
Result titleText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 start);

/* A quad of the menus' art in 640 by 480 measure, scaled to the screen. */
struct ArtQuad
{
    int32_t x[4], y[4];
    uint32_t u[4], v[4];  // floats' bits
    uint32_t texture, colour;
};
Result texturedQuad(App* app, CPU& cpu, reg32 esp, const ArtQuad& quad);
/* A quad in 640 by 480 measure, a colour at each corner. */
struct ShadedQuad
{
    int32_t x[4], y[4];
    uint32_t colour[4];
};
Result shadedQuad(App* app, CPU& cpu, reg32 esp, const ShadedQuad& quad);
/* The menus' pointer, its picture moved on with the clock. */
Result pointer(App* app, CPU& cpu, reg32 esp);

/* ---- The adapters, under the game's names ---- */

void quadOutcodes(App* app, CPU& cpu);          // 0x4d5840
void drawShadedRectangle(App* app, CPU& cpu);   // 0x4d81b0
void gradientRectangle(App* app, CPU& cpu);     // 0x4d8550
void drawShadedBox(App* app, CPU& cpu);         // 0x4d8860
void filledRectangle(App* app, CPU& cpu);       // 0x4d8910
void frame(App* app, CPU& cpu);                 // 0x4d8930
void drawPanel(App* app, CPU& cpu);             // 0x4d89f0
void drawFieldBox(App* app, CPU& cpu);          // 0x4d8ac0
void drawTileRectangle(App* app, CPU& cpu);     // 0x4d8b80
void drawLine(App* app, CPU& cpu);              // 0x4d8ed0
void fontCharacter(App* app, CPU& cpu);         // 0x4d9240
void accentedCharacter(App* app, CPU& cpu);     // 0x4d9320
void buildScreenFont(App* app, CPU& cpu);       // 0x4d93a0
void drawScreenCharacter(App* app, CPU& cpu);   // 0x4d95c0
void drawTurnedCharacter(App* app, CPU& cpu);   // 0x4d9b90
void drawTurnedText(App* app, CPU& cpu);        // 0x4da120
void drawTitleText(App* app, CPU& cpu);         // 0x4da1e0
void drawScreenText(App* app, CPU& cpu);        // 0x4da4a0
void drawPointer(App* app, CPU& cpu);           // 0x4dab90
void startScreens(App* app, CPU& cpu);          // 0x4daf80
void drawTexturedQuad(App* app, CPU& cpu);      // 0x4db000
void drawShadedQuad(App* app, CPU& cpu);        // 0x4db380

}  // namespace draw2d
}  // namespace port

/* How the game's code calls by address the cores the menus call as C++ (guest::direct), for the checks'
 * stand-ins: shadedRectangle (0x4d81b0), gradient (0x4d8550), shadedBox (0x4d8860), filled (0x4d8910), outline
 * (0x4d8930), drawFieldBoxOf (0x4d8ac0), drawLineOf (0x4d8ed0), resetScreens (0x4daf80), texturedQuad
 * (0x4db000).  A Result is nullopt when the game stopped in a call.  A core's `esp` is the frame base its
 * adapter hands it: the game's esp on entry, at the return address.  A caller through guest::direct passes
 * cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the adapter hands it, so the core's
 * frame lies where the game's does; by address the call makes it. */
namespace guest
{

// Stdcall: the rectangle's left in eax, top in edx, right in ebx, bottom in ecx; the four colours pushed (ret 0x10).
template <>
struct ByAddress<port::draw2d::shadedRectangle>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t left,
                             std::int32_t top, std::int32_t right, std::int32_t bottom, const std::uint32_t colours[4])
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(left);
        cpu.edx = x86::reg32(top);
        cpu.ebx = x86::reg32(right);
        cpu.ecx = x86::reg32(bottom);
        if (!guest::call(app, cpu, Address, {colours[0], colours[1], colours[2], colours[3]}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: the rectangle in eax, edx, ebx and ecx; its left's and right's colours pushed (ret 8).
template <>
struct ByAddress<port::draw2d::gradient>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t left,
                             std::int32_t top, std::int32_t right, std::int32_t bottom, std::uint32_t from,
                             std::uint32_t to)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(left);
        cpu.edx = x86::reg32(top);
        cpu.ebx = x86::reg32(right);
        cpu.ecx = x86::reg32(bottom);
        if (!guest::call(app, cpu, Address, {from, to}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: x in eax, y in edx, the width in ebx, the height in ecx; the four colours pushed (ret 0x10).
template <>
struct ByAddress<port::draw2d::shadedBox>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x, std::int32_t y,
                             std::int32_t width, std::int32_t height, const std::uint32_t colours[4])
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(x);
        cpu.edx = x86::reg32(y);
        cpu.ebx = x86::reg32(width);
        cpu.ecx = x86::reg32(height);
        if (!guest::call(app, cpu, Address, {colours[0], colours[1], colours[2], colours[3]}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: x in eax, y in edx, the width in ebx, the height in ecx; the colour pushed (ret 4).
template <>
struct ByAddress<port::draw2d::filled>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x, std::int32_t y,
                             std::int32_t width, std::int32_t height, std::uint32_t colour)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(x);
        cpu.edx = x86::reg32(y);
        cpu.ebx = x86::reg32(width);
        cpu.ecx = x86::reg32(height);
        if (!guest::call(app, cpu, Address, {colour}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: x in eax, y in edx, the width in ebx, the height in ecx; the colour pushed (ret 4).
template <>
struct ByAddress<port::draw2d::outline>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x, std::int32_t y,
                             std::int32_t width, std::int32_t height, std::uint32_t colour)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(x);
        cpu.edx = x86::reg32(y);
        cpu.ebx = x86::reg32(width);
        cpu.ecx = x86::reg32(height);
        if (!guest::call(app, cpu, Address, {colour}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: x in eax, y in edx, the width in ebx, the height in ecx; the four colours pushed (ret 0x10).
template <>
struct ByAddress<port::draw2d::drawFieldBoxOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x, std::int32_t y,
                             std::int32_t width, std::int32_t height, const std::uint32_t colours[4])
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(x);
        cpu.edx = x86::reg32(y);
        cpu.ebx = x86::reg32(width);
        cpu.ecx = x86::reg32(height);
        if (!guest::call(app, cpu, Address, {colours[0], colours[1], colours[2], colours[3]}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: x0 in eax, y0 in edx, x1 in ebx, y1 in ecx; the two colours pushed (ret 8).
template <>
struct ByAddress<port::draw2d::drawLineOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x0,
                             std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t from, std::uint32_t to)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(x0);
        cpu.edx = x86::reg32(y0);
        cpu.ebx = x86::reg32(x1);
        cpu.ecx = x86::reg32(y1);
        if (!guest::call(app, cpu, Address, {from, to}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// `a` in eax, edx, ebx and ecx; eax back.
template <>
struct ByAddress<port::draw2d::resetScreens>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::Args& a)
    {
        return watcomEax(app, cpu, Address, {a.eax, a.edx, a.ebx, a.ecx});
    }
};

// Stdcall: the first two corners in eax, edx, ebx and ecx; the other two, the texture's places, the texture and
// the colour pushed (ret 0x38).
template <>
struct ByAddress<port::draw2d::texturedQuad>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::draw2d::ArtQuad& quad)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(quad.x[0]);
        cpu.edx = x86::reg32(quad.y[0]);
        cpu.ebx = x86::reg32(quad.x[1]);
        cpu.ecx = x86::reg32(quad.y[1]);
        if (!guest::call(app, cpu, Address,
                         {x86::reg32(quad.x[2]), x86::reg32(quad.y[2]), x86::reg32(quad.x[3]), x86::reg32(quad.y[3]),
                          quad.u[0], quad.v[0], quad.u[1], quad.v[1], quad.u[2], quad.v[2], quad.u[3], quad.v[3],
                          quad.texture, quad.colour}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
