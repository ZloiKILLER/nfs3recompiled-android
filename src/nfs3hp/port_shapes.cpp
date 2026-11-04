#include "port_shapes.h"
#include "port_thrash.h"

#include <cstdint>

/* The resizing's steps and places are the game's x87 arithmetic, each sum,
 * product and quotient rounded as the control word has it (cpu.fpu) and kept
 * as a float where the code stores one: nothing here may be fused. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The game's shapes, rewritten (port_shapes.h): the FSH shapes of the menus'
 * art and of a race's textures turned into pixels, the menus' art made
 * textures a tile at a time, found by name and drawn.
 *
 * An FSH shape is a record of its own: its format byte, its width and height
 * as words at +4 and +6, its pixels from +0x10 -- 4 bits a pixel (0x7a), 8
 * through a palette (0x7b), 1555 (0x7e), 565 (0x78), 4444 (0x6d) or 8888
 * (0x7d).  What it becomes is the format the driver is asked for: 2 (8 bits),
 * 3 or 11 (1555), 4 (565), 6 (8888) or 7 (4444); a converter for each pair
 * the game knows copies it over, most of them resized to the texture's size
 * as they go, and says in a word whether the pixels have alpha.
 *
 * The integer arithmetic is the game's to the bit -- its packing of the
 * colours, its pixels left as they come out of a register's low half -- and
 * the x87's is too, operation by operation (__CHP's chop toward 0 where the
 * code calls it, the control word's rounding where it stores straight).  The
 * records stay where the game keeps them: the art's shapes at 0x8b48c0, its
 * tiles at 0x8b5090 and the room the tiles take at 0x8b7a74, the quad the
 * tiles are drawn with at 0x563ab0.
 *
 * tools/game_checks holds each adapter against the game's code for it, made
 * afresh from nfs3.exe (tools/game_checks/shapes.inc). */
namespace nfs3hp
{
namespace port
{
namespace shapes
{

namespace
{

using F = x86::Float;

uint8_t byteAt(App* app, reg32 at) { return app->getMemory<uint8_t>(at); }
uint16_t wordAt(App* app, reg32 at) { return app->getMemory<uint16_t>(at); }
reg32 dwordAt(App* app, reg32 at) { return app->getMemory<reg32>(at); }
float floatAt(App* app, reg32 at) { return app->getMemory<float>(at); }
void putByte(App* app, reg32 at, uint8_t value) { app->getMemory<uint8_t>(at) = value; }
void putWord(App* app, reg32 at, uint16_t value) { app->getMemory<uint16_t>(at) = value; }
void putDword(App* app, reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; }
void putFloat(App* app, reg32 at, float value) { app->getMemory<float>(at) = value; }

/* A word of a record, signed, as the code reads one (a dword two bytes
 * before it, shifted down). */
int32_t sword(App* app, reg32 at) { return int16_t(wordAt(app, at)); }

/* a + b and a times b, wrapping, as the registers have them. */
int32_t plus(int32_t a, int32_t b) { return int32_t(reg32(a) + reg32(b)); }
int32_t times(int32_t a, int32_t b) { return int32_t(reg32(a) * reg32(b)); }

/* v halved toward 0 (mov eax, v; sar edx, 0x1f; sub eax, edx; sar eax, 1). */
int32_t halved(int32_t v) { return int32_t(reg32(v) - reg32(v >> 31)) >> 1; }

// An FSH shape's header.
constexpr reg32 kShapeWidth = 4, kShapeHeight = 6, kShapePixels = 0x10;

// The game's variables these read and write.
constexpr reg32 kArtPalette = 0x563a88;      // the palette of an 8-bit shape being converted
constexpr reg32 kKeyBlack = 0x7a3a6c;        // the driver keys black: a pixel of none is kept apart
constexpr reg32 kDriverFlags = 0x7a3a58;     // 1 << the driver's type: 2 softtria, 0x40 Direct3D
constexpr reg32 kInRace = 0x559234;          // the menus are a race's pause screens
constexpr reg32 kMenuArt = 0x559228;         // the menus' art, an FSH
constexpr reg32 kArtShapes = 0x8b48c0;       // 100 of 0x14: size, tiles across and down, offset, first tile, end
constexpr reg32 kArtShapeCount = 0x8b7628;
constexpr reg32 kArtTiles = 0x8b5090;        // 0x320 of 0xc: the texture, its size, its column and row
constexpr reg32 kArtTileCount = 0x8b762c;
constexpr reg32 kTileRoom = 0x8b7a74;        // 20 columns of 15 tiles of 32 pixels: room free (1) or taken
constexpr reg32 kArtTextures = 0x563b34;     // 13 rows of 10 of the menus' textures
constexpr reg32 kArtTexturesUsed = 0x563d44;
constexpr reg32 kCorners = 0x563ab0;         // the tiles' quad: 4 corners of 0x20 -- x, y, z, w, colour, clip code
constexpr reg32 kCornerBytes = 0x20;
constexpr reg32 kShapeDepth = 0x563aa8, kShapeW = 0x563aac;  // each corner's z and w, floats
constexpr reg32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504;
constexpr reg32 kClipRight = 0x7d350c;       // the clip's rectangle: a float's bits, compared as integers

// The game's pointers to the driver's functions (THRASH_opendll fills them).
constexpr reg32 kThrashTReset = 0x9ef918, kThrashDrawQuad = 0x9ef94c, kThrashTUpdate = 0x9ef964;
constexpr reg32 kThrashTAlloc = 0x9ef968, kThrashSetState = 0x9ef96c, kThrashAbout = 0x9ef980;
constexpr reg32 kStateTextureFilter = 7;     // THRASH_setstate's: filtered (1) or not

// What is not theirs: the render states, the clipper, EA's allocator, its
// shapes and keys, the runtime's strings, the clock's wait, the trace, Error.
constexpr reg32 kSetRenderState = 0x431900, kClipQuad = 0x4c12e0, kTrace = 0x42d500, kError = 0x401010;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kLocateShape = 0x4ef270, kInkey = 0x4f0120;
constexpr reg32 kShapePalette = 0x4fd1c0, kStrncmp = 0x4eb390, kStrncpy = 0x4e0e30, kTickWait = 0x4c9860;
constexpr reg32 kShapeBufferTag = 0x5495c4, kNoArtTiles = 0x549630;
constexpr reg32 kNoPalette = 0x5495ec, kBadShapeFormat = 0x5495f8, kTooManyTiles = 0x54960c;

reg32 corner(reg32 i) { return kCorners + i * kCornerBytes; }

/* ---- The x87's arithmetic of the resizing ---- */

/* fild a; fild b; fdivp: a over b, kept as a float (fstp). */
float ratio(CPU& cpu, int32_t a, int32_t b) { return float(fp::div(F(a), F(b))); }

/* fild n; fmul [step]; __CHP; fistp: n steps along, toward 0. */
int32_t stepped(CPU& cpu, int32_t n, float step) { return chopped<int32_t>(cpu, fp::mul(F(n), F(step))); }

/* fld [place]; __CHP; fistp. */
int32_t whole(CPU& cpu, float place) { return chopped<int32_t>(cpu, F(place)); }

/* fld [place]; fadd [step]; fstp [place]. */
float advanced(CPU& cpu, float place, float step) { return float(fp::add(F(place), F(step))); }

/* fild a; fstp [f]; fild b; fdivr [f]; fistp: a as a float over b, rounded
 * as the control word says. */
int32_t wholeRatio(CPU& cpu, int32_t a, int32_t b)
{
    return fp::toInteger<int32_t>(fp::div(F(float(F(a))), F(b)));
}

/* fild c; fild d; fdivp; fmul [f]; fistp: c over d, times a as a float. */
int32_t wholeRatioTimes(CPU& cpu, int32_t c, int32_t d, int32_t a)
{
    return fp::toInteger<int32_t>(fp::mul(fp::div(F(c), F(d)), F(float(F(a)))));
}

/* ---- The alpha scans: 0 every pixel opaque, 1 some fully clear (keyed), 2
 * some in between ---- */

// 0x4d18e0, 4444: the top nibble.
int32_t alphaOf4444(App* app, reg32 at, int32_t outer, int32_t inner)
{
    bool keyed = false;
    for (int32_t i = 0; i < outer; ++i)
        for (int32_t j = 0; j < inner; ++j, at += 2)
        {
            const reg32 alpha = wordAt(app, at) & 0xf000;
            if (alpha == 0xf000)
                continue;
            if (alpha != 0)
                return 2;
            keyed = true;
        }
    return keyed ? 1 : 0;
}

// 0x4d1950, 8888: the top byte.
int32_t alphaOf8888(App* app, reg32 at, int32_t outer, int32_t inner)
{
    bool keyed = false;
    for (int32_t i = 0; i < outer; ++i)
        for (int32_t j = 0; j < inner; ++j, at += 4)
        {
            const reg32 alpha = dwordAt(app, at) & 0xff000000;
            if (alpha == 0xff000000)
                continue;
            if (alpha != 0)
                return 2;
            keyed = true;
        }
    return keyed ? 1 : 0;
}

/* 0x4d19b0, 4 bits: a pixel of 0 is a keyed shape.  The row's loop never
 * counts its columns -- it goes on to the first such pixel however far that
 * is, and only a width of 0 ends a row. */
int32_t alphaOfNibbles(App* app, reg32 at, int32_t width, int32_t rows)
{
    for (int32_t row = 0; row < rows; ++row)
        for (reg32 column = 0;; column += 2, ++at)
        {
            const reg32 end = int32_t(column) < int32_t(width & 1) ? reg32(width) + 1 : reg32(width);
            if (end == 0)
                break;
            const uint8_t pair = byteAt(app, at);
            if (!(pair & 0x0f) || !(pair & 0xf0))
                return 1;
        }
    return 0;
}

/* 0x4d1a00, 8 bits through the palette: index 0xff, or a colour of alpha 0,
 * is a keyed pixel; with no palette, 2. */
int32_t alphaOfPaletted(App* app, reg32 at, int32_t outer, int32_t inner)
{
    if (dwordAt(app, kArtPalette) == 0)
        return 2;
    bool keyed = false;
    for (int32_t i = 0; i < outer; ++i)
        for (int32_t j = 0; j < inner; ++j, ++at)
        {
            const uint8_t index = byteAt(app, at);
            if (index == 0xff)
            {
                keyed = true;
                continue;
            }
            const uint8_t alpha = byteAt(app, dwordAt(app, kArtPalette) + reg32(index) * 4 + 3);
            if (alpha == 0)
                keyed = true;
            else if (alpha < 0xff)
                return 2;
        }
    return keyed ? 1 : 0;
}

// 0x4d1a90, 1555: a pixel without its top bit.
int32_t alphaOf1555(App* app, reg32 at, int32_t outer, int32_t inner)
{
    for (int32_t i = 0; i < outer; ++i)
        for (int32_t j = 0; j < inner; ++j, at += 2)
            if (!(byteAt(app, at + 1) & 0x80))
                return 1;
    return 0;
}

// 0x4d1ad0, 565: the key colours 0x7c0 and 0x7e0.
int32_t alphaOf565(App* app, reg32 at, int32_t outer, int32_t inner)
{
    for (int32_t i = 0; i < outer; ++i)
        for (int32_t j = 0; j < inner; ++j, at += 2)
        {
            const uint16_t pixel = wordAt(app, at);
            if (pixel == 0x7c0 || pixel == 0x7e0)
                return 1;
        }
    return 0;
}

/* ---- The converters: shape pixels `from` (srcW by srcH) into texture
 * pixels `to` (dstW by dstH); `flag` the word that says whether they have
 * alpha ---- */

typedef void (*Converter)(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW,
                          int32_t dstH, reg32 flag);

/* The copy most converters make: down `dstH` rows and across `dstW`, each
 * pixel the source's at the sizes' ratios times its place, toward 0 -- the
 * column's a float stepped along the row.  `pixel` converts the source's
 * pixel at `at` into `out` and says how many bytes it wrote. */
template <class Pixel>
void resized(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 bytes, Pixel pixel)
{
    const float across = ratio(cpu, srcW, dstW);
    const float down = ratio(cpu, srcH, dstH);
    for (int32_t y = 0; y < dstH; ++y)
    {
        const reg32 row = from + reg32(stepped(cpu, y, down)) * reg32(srcW) * bytes;
        float x = 0.0f;
        for (int32_t i = 0; i < dstW; ++i)
        {
            to += pixel(row + reg32(whole(cpu, x)) * bytes, to);
            x = advanced(cpu, x, across);
        }
    }
}

/* A row of 4-bit pixels' bytes: half its width, rounded up. */
int32_t nibbleRow(int32_t width)
{
    return halved(int32_t((width & 1) ? reg32(width) + 1 : reg32(width)));
}

/* Pixel x of a row of 4-bit ones: the low half of byte x / 2 for an odd x,
 * the high half for an even one. */
uint8_t nibbleAt(App* app, reg32 row, int32_t x)
{
    const uint8_t pair = byteAt(app, row + reg32(halved(x)));
    return (x & 1) ? pair & 0xf : pair >> 4;
}

/* The 8888 key colours, a shape's transparent pixels. */
bool keyed8888(reg32 pixel) { return pixel == 0xff00f800 || pixel == 0xff00fb00; }
bool keyed565(uint16_t pixel) { return pixel == 0x7c0 || pixel == 0x7e0; }
bool keyed1555(uint16_t pixel) { return pixel == 0x83e0 || pixel == 0x83c0; }

// 0x4d1b20: 4444 as it is, resized; alpha where a pixel's is 0.
void same4444(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
              reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        if (!(byteAt(app, at + 1) & 0xf0))
            putDword(app, flag, 1);
        putWord(app, out, wordAt(app, at));
        return 2u;
    });
}

/* 0x4d1c00: 8888 as it is, resized; alpha with any pixel at all (the code
 * compares the top byte, in place, with 0xff). */
void same8888(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
              reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 4, [&](reg32 at, reg32 out) {
        if ((dwordAt(app, at) & 0xff000000) != 0xff)
            putDword(app, flag, 1);
        putDword(app, out, dwordAt(app, at));
        return 4u;
    });
}

// 0x4d1ce0: 8 bits as they are, resized; alpha with index 0xff.
void same8(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
           reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 1, [&](reg32 at, reg32 out) {
        if (byteAt(app, at) == 0xff)
            putDword(app, flag, 1);
        putByte(app, out, byteAt(app, at));
        return 1u;
    });
}

// 0x4d1db0: 565 as it is, resized; no alpha.
void same565(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        putWord(app, out, wordAt(app, at));
        return 2u;
    });
}

/* 0x4d1e60: 1555 as it is, resized; alpha where a pixel's bit is clear --
 * but for a driver that keys black, which takes 0x8000 as 0x8001 and says
 * nothing of alpha. */
void same1555(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
              reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        if (dwordAt(app, kKeyBlack) != 0)
        {
            const uint16_t pixel = wordAt(app, at);
            putWord(app, out, pixel);
            if (pixel == 0x8000)
                putWord(app, out, 0x8001);
        }
        else
        {
            if (!(byteAt(app, at + 1) & 0x80))
                putDword(app, flag, 1);
            putWord(app, out, wordAt(app, at));
        }
        return 2u;
    });
}

/* 0x4d1f70: 4 bits to 1555, each nibble doubled into the three colours, 0
 * transparent (and alpha); not resized -- as much of each row as both
 * widths have, the texture's row's rest left. */
void nibbles1555(App* app, CPU&, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t,
                 reg32 flag)
{
    const int32_t pitch = nibbleRow(srcW);
    putDword(app, flag, 0);
    for (int32_t y = 0; y < srcH; ++y)
    {
        const reg32 row = from + reg32(times(pitch, y));
        for (int32_t x = 0; x < (srcW < dstW ? srcW : dstW); ++x, to += 2)
        {
            const uint8_t v = nibbleAt(app, row, x);
            if (v != 0)
            {
                const reg32 c = uint8_t(v + v);
                putWord(app, to, uint16_t(c << 10 | 0x8000 | c << 5 | c));
            }
            else
            {
                putDword(app, flag, 1);
                putWord(app, to, 0);
            }
        }
        if (srcW < dstW)
            to += reg32(dstW - srcW) * 2;
    }
}

/* 0x4d2050: 4 bits to 565, the rows resized, the columns not; 0
 * transparent. */
void nibbles565(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                reg32 flag)
{
    const int32_t pitch = nibbleRow(srcW);
    putDword(app, flag, 0);
    const float down = ratio(cpu, srcH, dstH);
    for (int32_t y = 0; y < dstH; ++y)
    {
        const reg32 row = from + reg32(times(stepped(cpu, y, down), pitch));
        for (int32_t x = 0; x < dstW; ++x, to += 2)
        {
            const uint8_t v = nibbleAt(app, row, x);
            if (v != 0)
            {
                const reg32 c = uint8_t(v + v), g = uint8_t(v << 2);
                putWord(app, to, uint16_t(c << 11 | g << 5 | c));
            }
            else
            {
                putDword(app, flag, 1);
                putWord(app, to, 0);
            }
        }
    }
}

// 0x4d2150: 4 bits to 4444, the nibble as alpha over white; alpha always.
void nibbles4444(App* app, CPU&, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t, int32_t, reg32 flag)
{
    const int32_t pitch = nibbleRow(srcW);
    putDword(app, flag, 1);
    for (int32_t y = 0; y < srcH; ++y)
    {
        const reg32 row = from + reg32(times(pitch, y));
        for (int32_t x = 0; x < srcW; ++x, to += 2)
        {
            const uint8_t v = nibbleAt(app, row, x);
            putWord(app, to, v != 0 ? uint16_t(reg32(v) << 12 | 0xfff) : 0);
        }
    }
}

// 0x4d21f0: 4 bits to 8888, the nibble as alpha over white; alpha always.
void nibbles8888(App* app, CPU&, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t, int32_t, reg32 flag)
{
    const int32_t pitch = nibbleRow(srcW);
    putDword(app, flag, 1);
    for (int32_t y = 0; y < srcH; ++y)
    {
        const reg32 row = from + reg32(times(pitch, y));
        for (int32_t x = 0; x < srcW; ++x, to += 4)
        {
            const uint8_t v = nibbleAt(app, row, x);
            putDword(app, to, v != 0 ? reg32(uint8_t(v << 4 | v)) << 24 | 0xffffff : 0);
        }
    }
}

/* 0x4d22b0: 8 bits through the palette to 1555, resized in whole steps --
 * the width's ratio, and the height's times the width, each rounded as the
 * control word says; alpha where a colour's top bit is set. */
void palette1555(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                 reg32 flag)
{
    putDword(app, flag, 0);
    const reg32 palette = dwordAt(app, kArtPalette);
    if (palette == 0)
        return;
    const int32_t across = wholeRatio(cpu, srcW, dstW);
    const int32_t down = wholeRatioTimes(cpu, srcH, dstH, srcW);
    for (int32_t y = 0; y < dstH; ++y, from += reg32(down))
    {
        reg32 at = from;
        for (int32_t x = 0; x < dstW; ++x, at += reg32(across), to += 2)
        {
            const reg32 colour = palette + reg32(byteAt(app, at)) * 4;
            const reg32 a = (byteAt(app, colour + 3) >> 7) & 1, r = (byteAt(app, colour + 2) >> 3) & 0x1f;
            const reg32 g = (byteAt(app, colour + 1) >> 3) & 0x1f, b = (byteAt(app, colour) >> 3) & 0x1f;
            putWord(app, to, uint16_t(a << 15 | r << 10 | g << 5 | b));
            if (a != 0)
                putDword(app, flag, 1);
        }
    }
}

// 0x4d23f0: 8 bits through the palette to 565, resized; no alpha.
void palette565(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                reg32 flag)
{
    putDword(app, flag, 0);
    const reg32 palette = dwordAt(app, kArtPalette);
    if (palette == 0)
        return;
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 1, [&](reg32 at, reg32 out) {
        const reg32 colour = palette + reg32(byteAt(app, at)) * 4;
        const reg32 r = byteAt(app, colour + 2) >> 3, g = byteAt(app, colour + 1) >> 2, b = byteAt(app, colour) >> 3;
        putWord(app, out, uint16_t(r << 11 | g << 5 | b));
        return 2u;
    });
}

/* 0x4d2510: 8 bits through the palette to 4444, resized; index 0xff
 * transparent; alpha with that or a colour not opaque. */
void palette4444(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                 reg32 flag)
{
    putDword(app, flag, 0);
    const reg32 palette = dwordAt(app, kArtPalette);
    if (palette == 0)
        return;
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 1, [&](reg32 at, reg32 out) {
        const uint8_t index = byteAt(app, at);
        if (index == 0xff)
        {
            putDword(app, flag, 1);
            putWord(app, out, 0);
            return 2u;
        }
        const reg32 colour = palette + reg32(index) * 4;
        const reg32 r = byteAt(app, colour + 2) >> 4, g = byteAt(app, colour + 1) >> 4, b = byteAt(app, colour) >> 4;
        const reg32 a = byteAt(app, colour + 3) >> 4;
        putWord(app, out, uint16_t(a << 12 | r << 8 | g << 4 | b));
        if (a != 0xf)
            putDword(app, flag, 1);
        return 2u;
    });
}

/* 0x4d2690: 8 bits through the palette to 8888 (the palette's colour as it
 * is), resized; index 0xff transparent; alpha with that or a colour not
 * opaque. */
void palette8888(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                 reg32 flag)
{
    putDword(app, flag, 0);
    const reg32 palette = dwordAt(app, kArtPalette);
    if (palette == 0)
        return;
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 1, [&](reg32 at, reg32 out) {
        const uint8_t index = byteAt(app, at);
        if (index == 0xff)
        {
            putDword(app, flag, 1);
            putDword(app, out, 0);
            return 4u;
        }
        const reg32 colour = palette + reg32(index) * 4;
        const reg32 a = byteAt(app, colour + 3);
        putDword(app, out, a << 24 | reg32(byteAt(app, colour + 2)) << 16 | reg32(byteAt(app, colour + 1)) << 8
                               | byteAt(app, colour));
        if (a != 0xff)
            putDword(app, flag, 1);
        return 4u;
    });
}

/* 0x4d27e0: 8888 to 4444, resized in whole steps; the key colours
 * transparent; a driver that keys black gets a pixel of alpha but no colour
 * with blue 1.  Alpha with a key or a pixel not opaque. */
void argb4444(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
              reg32 flag)
{
    putDword(app, flag, 0);
    const int32_t across = wholeRatio(cpu, srcW, dstW);
    const int32_t down = wholeRatioTimes(cpu, srcH, dstH, srcW);
    for (int32_t y = 0; y < dstH; ++y, from += reg32(down) * 4)
    {
        reg32 at = from;
        for (int32_t x = 0; x < dstW; ++x, at += reg32(across) * 4, to += 2)
        {
            const reg32 pixel = dwordAt(app, at);
            if (keyed8888(pixel))
            {
                putDword(app, flag, 1);
                putWord(app, to, 0);
                continue;
            }
            const reg32 a = (pixel & 0xf0000000) >> 16, r = (pixel & 0xf00000) >> 12, g = (pixel & 0xf000) >> 8;
            reg32 b = (pixel & 0xf0) >> 4;
            if (dwordAt(app, kKeyBlack) != 0 && a != 0 && r == 0 && g == 0 && b == 0)
                b = 1;
            putWord(app, to, uint16_t(a | r | g | b));
            if (a != 0xf000)
                putDword(app, flag, 1);
        }
    }
}

/* 0x4d2940: 8888 to 565, resized, each colour times the alpha; the key
 * colours black.  Alpha with a key. */
void argb565(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 4, [&](reg32 at, reg32 out) {
        const reg32 pixel = dwordAt(app, at);
        if (keyed8888(pixel))
        {
            putDword(app, flag, 1);
            putWord(app, out, 0);
            return 2u;
        }
        const reg32 a = pixel >> 24;
        const reg32 r = ((pixel & 0xff0000) * a) >> 0x1b, g = ((pixel & 0xff00) * a) >> 0x12;
        const reg32 b = ((pixel & 0xff) * a) >> 0xb;
        putWord(app, out, uint16_t(r << 11 | g << 5 | b));
        return 2u;
    });
}

/* 0x4d2a80: 8888 to 1555, resized; the key colours, and a pixel without the
 * alpha's top bit, transparent.  Alpha with either. */
void argb1555(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
              reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 4, [&](reg32 at, reg32 out) {
        const reg32 pixel = dwordAt(app, at);
        if (keyed8888(pixel))
        {
            putDword(app, flag, 1);
            putWord(app, out, 0);
            return 2u;
        }
        const reg32 a = pixel >> 31, r = (pixel & 0xf80000) >> 0x13, g = (pixel & 0xf800) >> 0xb;
        const reg32 b = (pixel & 0xf8) >> 3;
        putWord(app, out, uint16_t(a << 15 | r << 10 | g << 5 | b));
        if (a == 0)
        {
            putWord(app, out, 0);
            putDword(app, flag, 1);
        }
        return 2u;
    });
}

// 0x4d2be0: 565 to 1555, resized; the key colours transparent (and alpha).
void rgb1555(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed565(pixel))
        {
            putWord(app, out, 0);
            putDword(app, flag, 1);
            return 2u;
        }
        const reg32 r = (pixel & 0xf800) >> 0xb, g = (pixel & 0x7c0) >> 6, b = pixel & 0x1f;
        putWord(app, out, uint16_t(r << 10 | 0x8000 | g << 5 | b));
        return 2u;
    });
}

// 0x4d2d20: 565 to 4444, resized; the key colours transparent (and alpha).
void rgb4444(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed565(pixel))
        {
            putWord(app, out, 0);
            putDword(app, flag, 1);
            return 2u;
        }
        const reg32 r = (pixel & 0x7800) >> 0xb, g = (pixel & 0x3c0) >> 6, b = pixel & 0xf;
        putWord(app, out, uint16_t(r << 8 | 0xf000 | g << 4 | b));
        return 2u;
    });
}

/* 0x4d2e60: 565 to 8888, resized (green's bits taken from 0xfc0, one too
 * high); the key colours transparent (and alpha). */
void rgb8888(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
             reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed565(pixel))
        {
            putDword(app, out, 0);
            putDword(app, flag, 1);
            return 4u;
        }
        const reg32 r = (pixel & 0xf800) >> 8, g = (pixel & 0xfc0) >> 3, b = (pixel & 0x1f) << 3;
        putDword(app, out, 0xff000000 | r << 16 | g << 8 | b);
        return 4u;
    });
}

// 0x4d2fb0: 1555 to 565, resized; the key colours transparent (and alpha).
void argb1555To565(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                   reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed1555(pixel))
        {
            putWord(app, out, 0);
            putDword(app, flag, 1);
            return 2u;
        }
        const reg32 r = (pixel & 0x7c00) >> 0xa, g = (pixel & 0x3e0) >> 4, b = pixel & 0x1f;
        putWord(app, out, uint16_t(r << 11 | g << 5 | b));
        return 2u;
    });
}

// 0x4d30f0: 1555 to 4444, resized; the key colours transparent (and alpha).
void argb1555To4444(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW,
                    int32_t dstH, reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed1555(pixel))
        {
            putWord(app, out, 0);
            putDword(app, flag, 1);
            return 2u;
        }
        const reg32 r = (pixel & 0x7c00) >> 0xb, g = (pixel & 0x3e0) >> 6, b = (pixel & 0x1f) >> 1;
        putWord(app, out, uint16_t(r << 8 | 0xf000 | g << 4 | b));
        return 2u;
    });
}

// 0x4d3230: 1555 to 8888, resized; the key colours transparent (and alpha).
void argb1555To8888(App* app, CPU& cpu, reg32 to, reg32 from, int32_t srcW, int32_t srcH, int32_t dstW,
                    int32_t dstH, reg32 flag)
{
    putDword(app, flag, 0);
    resized(app, cpu, to, from, srcW, srcH, dstW, dstH, 2, [&](reg32 at, reg32 out) {
        const uint16_t pixel = wordAt(app, at);
        if (keyed1555(pixel))
        {
            putDword(app, out, 0);
            putDword(app, flag, 1);
            return 4u;
        }
        const reg32 r = (pixel & 0x7c00) >> 7, g = (pixel & 0x3e0) >> 2, b = (pixel & 0x1f) << 3;
        putDword(app, out, 0xff000000 | r << 16 | g << 8 | b);
        return 4u;
    });
}

/* The converter for a shape's kind (1 the 4 bits', 2 the palette's, 3 1555,
 * 4 565, 6 8888, 7 4444) and the texture's format: the two levels of jmp
 * [eax*4 + table] at 0x4d36c4 on.  None for a pair the game does not know. */
Converter converterFor(int32_t kind, reg32 format)
{
    switch (kind)
    {
    case 1:
        switch (format)
        {
        case 3: case 11: return nibbles1555;
        case 4: return nibbles565;
        case 6: return nibbles8888;
        case 7: return nibbles4444;
        }
        break;
    case 2:
        switch (format)
        {
        case 2: return same8;
        case 3: case 11: return palette1555;
        case 4: return palette565;
        case 6: return palette8888;
        case 7: return palette4444;
        }
        break;
    case 3:
        switch (format)
        {
        case 3: case 11: return same1555;
        case 4: return argb1555To565;
        case 6: return argb1555To8888;
        case 7: return argb1555To4444;
        }
        break;
    case 4:
        switch (format)
        {
        case 3: case 11: return rgb1555;
        case 4: return same565;
        case 6: return rgb8888;
        case 7: return rgb4444;
        }
        break;
    case 6:
        switch (format)
        {
        case 3: case 11: return argb1555;
        case 4: return argb565;
        case 6: return same8888;
        case 7: return argb4444;
        }
        break;
    case 7:
        if (format == 7)
            return same4444;
        break;
    }
    return nullptr;
}

/* ---- The menus' art's tiles ---- */

/* 0x4d6110: the square of `size` pixels at (left, top) of the 1555 shape
 * `from` copied out into `to`, 0 past the shape.  top + size back. */
int32_t cutTileOf(App* app, reg32 from, reg32 to, int32_t size, int32_t left, int32_t top)
{
    const int32_t width = sword(app, from + kShapeWidth), height = sword(app, from + kShapeHeight);
    const reg32 pixels = from + kShapePixels;
    for (int32_t y = top; y < plus(top, size); ++y)
        for (int32_t x = left; x < plus(left, size); ++x, to += 2)
            putWord(app, to, x < width && y < height ? wordAt(app, pixels + reg32(plus(times(width, y), x)) * 2) : 0);
    return plus(top, size);
}

/* 0x4d6190: a 565 shape made 1555 in place: green's lowest bit dropped, the
 * top bit set but for black; its format byte 0x7e then. */
void shape565To1555Of(App* app, reg32 shape)
{
    const int32_t count = times(sword(app, shape + kShapeWidth), sword(app, shape + kShapeHeight));
    reg32 at = shape + kShapePixels;
    for (int32_t i = 0; i < count; ++i, at += 2)
    {
        const reg32 pixel = wordAt(app, at);
        uint16_t made = uint16_t(((pixel >> 11) & 0x1f) << 10 | (((pixel >> 5) & 0x3f) >> 1) << 5 | (pixel & 0x1f));
        if (made != 0)
            made |= 0x8000;
        putWord(app, at, made);
    }
    putByte(app, shape, 0x7e);
}

/* 0x4d6210: with `start`, the room of the tiles made `across` by `down`
 * (each 32 pixels a side, the room 20 by 15 of them), free; 0.  Otherwise
 * the largest square of tiles -- 8 a side, then 4, 2, 1 -- free at column
 * `across`, row `down`, taken: its side in pixels, 0 for none. */
int32_t tileRoomOf(App* app, int32_t across, int32_t down, bool start)
{
    const auto room = [&](int32_t column, int32_t row) { return kTileRoom + reg32(plus(times(column, 15), row)); };
    if (start)
    {
        for (int32_t column = 0; column < 20; ++column)
            for (int32_t row = 0; row < 15; ++row)
                putByte(app, room(column, row), column < across && row < down ? 1 : 0);
        return 0;
    }
    if (reg32(times(down, 15)) + kTileRoom == 0)
        return 0;
    for (int32_t side = 8; side != 0; side = halved(side))
    {
        if (plus(across, side) > 20 || plus(down, side) > 15)
            continue;
        bool free = true;
        for (int32_t column = across; column < plus(across, side); ++column)
            for (int32_t row = down; row < plus(down, side); ++row)
                if (byteAt(app, room(column, row)) == 0)
                    free = false;
        if (!free)
            continue;
        for (int32_t column = across; column < plus(across, side); ++column)
            for (int32_t row = down; row < plus(down, side); ++row)
                putByte(app, room(column, row), 0);
        return int32_t(reg32(side) << 5);
    }
    return 0;
}

/* A corner's clip code into its +0x14, as the menus' quads work it out: 0x10
 * for a w behind the far edge, else 8 above, 4 below, 1 left of, 2 right of
 * the rectangle -- each a float's bits compared as integers, a negative one
 * counting as short. */
void clipCode(App* app, reg32 at)
{
    const reg32 w = dwordAt(app, at + 0xc);
    uint8_t code = 0;
    if ((w & 0x80000000) || int32_t(w) >= int32_t(dwordAt(app, kClipFar)))
        code = 0x10;
    else
    {
        const reg32 y = dwordAt(app, at + 4);
        if ((y & 0x80000000) || int32_t(y) < int32_t(dwordAt(app, kClipTop)))
            code |= 8;
        else if (int32_t(y) > int32_t(dwordAt(app, kClipBottom)))
            code |= 4;
        const reg32 x = dwordAt(app, at);
        if ((x & 0x80000000) || int32_t(x) < int32_t(dwordAt(app, kClipLeft)))
            code |= 1;
        else if (int32_t(x) > int32_t(dwordAt(app, kClipRight)))
            code |= 2;
    }
    putByte(app, at + 0x14, code);
}

/* 0x4d70f0: the tiles' quad drawn: on softtria as one quad of the driver's
 * under state 0x2bf, otherwise through the clipper. */
Result drawTileQuad(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 0x10;
    reg32 r;
    if (byteAt(app, kDriverFlags) & 2)
    {
        if (!callGame(app, cpu, frame, kSetRenderState, {0x2bf, 2})
            || !callGame(app, cpu, frame, dwordAt(app, kThrashDrawQuad), {}, {corner(0), corner(1), corner(2), corner(3)})
            || !callGame(app, cpu, frame, kSetRenderState, {0x2bf, 0}, {}, r))
            return {};
        return int32_t(r);
    }
    if (!callGame(app, cpu, frame, kClipQuad, {corner(0), corner(1), corner(2), corner(3)}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4d6390: the menus' art shape `shape` cut into tiles of 32 pixels and
 * each made a texture and filled: the shape turned into pixels first, 8888
 * as 4444, 565 as 1555, 1555 as it is, palettised through its own palette.
 * A new record (`again` 0) gets its size, its tiles across and down, where
 * the shape sits in them and its first tile, and a texture for each; record
 * `again` has its textures filled afresh.  `ecx` what the first allocation
 * is handed there.  The record, or 0 past 0x320 tiles (an Error). */
Result artTexturesOf(App* app, CPU& cpu, reg32 esp, reg32 shape, int32_t again, reg32 ecx)
{
    const reg32 frame = esp - 0x14 - 0x34;
    const reg32 flagAt = frame, formatAt = frame + 4;
    const reg32 palette = dwordAt(app, kArtPalette);
    const reg32 tilesBefore = dwordAt(app, kArtTileCount);
    reg32 pixels, cut;
    if (!callGame(app, cpu, frame, kAllocate, {kShapeBufferTag, 0x12c014, 0x10, ecx}, {}, pixels)
        || !callGame(app, cpu, frame, kAllocate, {kShapeBufferTag, 0x40000, 0x10, 0}, {}, cut))
        return {};
    reg32 format = 0;
    const reg32 kind = reg32(byteAt(app, shape)) - 0x78;
    switch (kind)
    {
    case 0x7d - 0x78:
        format = 7;
        break;
    case 0x7b - 0x78:
    {
        reg32 own;
        if (!callGame(app, cpu, frame, kShapePalette, {shape, 0x40000, 0x10, 0}, {}, own))
            return {};
        if (own == 0 && !callGame(app, cpu, frame, kError, {0, 0, 0x10, 0}, {kNoPalette}))
            return {};
        format = 3;
        putDword(app, kArtPalette, own + 0x10);
        break;
    }
    case 0x7e - 0x78:
        format = 3;
        break;
    case 0x78 - 0x78:
        shape565To1555Of(app, shape);
        format = 3;
        break;
    default:
        if (!callGame(app, cpu, frame, kError, {kind, 0x40000, 0x10, 0}, {kBadShapeFormat}))
            return {};
        break;
    }
    if (again < 0)
    {
        format = 3;
        again = 0;
    }
    putDword(app, formatAt, format);
    if (!shapeToPixelsOf(app, cpu, frame - 8, pixels, shape, flagAt, 0, formatAt))
        return {};
    putWord(app, pixels + kShapeHeight, wordAt(app, shape + kShapeHeight));
    putWord(app, pixels + kShapeWidth, wordAt(app, shape + kShapeWidth));
    int32_t index = again;
    if (again == 0)
    {
        // A new record: its size, its tiles across and down, where the shape
        // sits in them -- half the room left over each way -- its first tile.
        index = int32_t(dwordAt(app, kArtShapeCount));
        const reg32 record = kArtShapes + reg32(index) * 0x14;
        putWord(app, record, wordAt(app, shape + kShapeWidth));
        putWord(app, record + 2, wordAt(app, shape + kShapeHeight));
        putDword(app, kArtShapeCount, reg32(index) + 1);
        const auto tiles = [](int32_t pixels) {
            const int32_t v = plus(pixels, 0x1f);
            return plus(v, v < 0 ? 0x1f : 0) >> 5;
        };
        const int32_t across = tiles(sword(app, shape + kShapeWidth)), down = tiles(sword(app, shape + kShapeHeight));
        putWord(app, record + 4, uint16_t(across));
        putWord(app, record + 6, uint16_t(down));
        putWord(app, record + 0xc, wordAt(app, kArtTileCount));
        putWord(app, record + 8, uint16_t(halved(int32_t((reg32(across) << 5) - reg32(sword(app, shape + kShapeWidth))))));
        putWord(app, record + 0xa, uint16_t(halved(int32_t((reg32(down) << 5) - reg32(sword(app, shape + kShapeHeight))))));
    }
    const reg32 record = kArtShapes + reg32(index) * 0x14;
    const int32_t across = sword(app, record + 4), down = sword(app, record + 6);
    int32_t tile = sword(app, record + 0xc);
    tileRoomOf(app, across, down, true);
    for (int32_t column = 0; column < across; ++column)
        for (int32_t row = 0; row < down; ++row)
        {
            const int32_t size = tileRoomOf(app, column, row, false);
            if (size == 0)
                continue;
            cutTileOf(app, pixels, cut, size, int32_t(reg32(column) << 5), int32_t(reg32(row) << 5));
            const reg32 at = kArtTiles + reg32(tile) * 0xc;
            if (again == 0)
            {
                const reg32 kept = dwordAt(app, formatAt);
                reg32 texture;
                if (!callGame(app, cpu, frame, dwordAt(app, kThrashTAlloc), {reg32(size), cut, kept, 0},
                              {reg32(size), reg32(size), kept, 0, 0}, texture))
                    return {};
                putDword(app, at, texture);
                putWord(app, at + 8, uint16_t(row));
                putWord(app, at + 4, uint16_t(size));
                putWord(app, at + 6, uint16_t(column));
            }
            const reg32 texture = dwordAt(app, at);
            if (!callGame(app, cpu, frame, dwordAt(app, kThrashTUpdate), {reg32(tile) * 3, reg32(tile), texture, cut},
                          {texture, cut, 0}))
                return {};
            if (tile < 0x31f)
            {
                ++tile;
                continue;
            }
            if (!callGame(app, cpu, frame, kError, {reg32(tile), reg32(tile), texture, cut}, {kTooManyTiles}))
                return {};
            putDword(app, kArtTileCount, tilesBefore);
            if (!callGame(app, cpu, frame, kFree, {cut}) || !callGame(app, cpu, frame, kFree, {pixels}))
                return {};
            return 0;
        }
    putWord(app, record + 0xe, uint16_t(tile));
    if (again == 0)
        putDword(app, kArtTileCount, reg32(tile));
    putDword(app, kArtPalette, palette);
    if (!callGame(app, cpu, frame, kFree, {cut}) || !callGame(app, cpu, frame, kFree, {pixels}))
        return {};
    return index;
}

/* 0x4d6070: shape `shape` made a texture of its own, at most `size` pixels
 * a side: turned into pixels in a buffer of its own, a texture of their size
 * allocated into [out] and, when there is one, filled.  The format, asked
 * for and given back. */
Result textureOf(App* app, CPU& cpu, reg32 esp, reg32 out, reg32 shape, int32_t size, reg32 format)
{
    const reg32 frame = esp - 0xc - 0x14;
    const reg32 formatAt = frame, flagAt = frame + 4;
    putDword(app, formatAt, format);
    // port: a shape of 512 a side or more at 512 (port_thrash.h)
    size = driver::texture512Side(size, sword(app, shape + kShapeWidth));
    reg32 pixels;
    if (!callGame(app, cpu, frame, kAllocate, {kShapeBufferTag, reg32(times(size, size)) * 4 + 0x14, 0x10, reg32(size)},
                  {}, pixels))
        return {};
    if (!shapeToPixelsOf(app, cpu, frame - 8, pixels, shape, flagAt, size, formatAt))
        return {};
    const reg32 kept = dwordAt(app, formatAt);
    const reg32 width = reg32(sword(app, pixels + kShapeWidth)), height = reg32(sword(app, pixels + kShapeHeight));
    reg32 texture;
    if (!callGame(app, cpu, frame, dwordAt(app, kThrashTAlloc), {width, kept}, {width, height, kept, 0, 0}, texture))
        return {};
    putDword(app, out, texture);
    if (texture != 0
        && !callGame(app, cpu, frame, dwordAt(app, kThrashTUpdate), {texture, pixels + kShapePixels},
                     {texture, pixels + kShapePixels, 0}))
        return {};
    if (!callGame(app, cpu, frame, kFree, {pixels}))
        return {};
    return int32_t(dwordAt(app, formatAt));
}

/* 0x4d5c40: every texture let go of (THRASH_treset) and the menus' art's
 * table of them emptied. */
Result texturesLetGo(App* app, CPU& cpu, reg32 esp)
{
    if (!callGame(app, cpu, esp - 0x18, dwordAt(app, kThrashTReset), {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx}))
        return {};
    for (reg32 row = 0; row < 0xd; ++row)
        for (reg32 column = 0; column < 0xa; ++column)
            putDword(app, kArtTextures + row * 0x28 + column * 4, 0);
    putDword(app, kArtTexturesUsed, 0);
    return 0x30;
}

/* 0x4d66f0: record `record`'s width and height into the words at
 * `widthAt` and `heightAt` -- 0 for one outside the 100.  What the code
 * leaves in eax: the record, or the height in five times its low half. */
reg32 shapeSizeOf(App* app, int32_t record, reg32 widthAt, reg32 heightAt)
{
    if (record < 0 || record >= 100)
    {
        putWord(app, widthAt, 0);
        putWord(app, heightAt, 0);
        return reg32(record);
    }
    const reg32 at = kArtShapes + reg32(record) * 0x14;
    putWord(app, widthAt, wordAt(app, at));
    const uint16_t height = wordAt(app, at + 2);
    putWord(app, heightAt, height);
    return (reg32(record) * 5 & 0xffff0000) | height;
}

}  // namespace

/* 0x4d37a0: what the driver takes (THRASH_about's record: its sizes at +0x10
 * on, its formats at +0x30, its texture memory at +0x70; without one, sizes
 * of 2 to 256 in a record of the code's own on its stack, the rest of it as
 * the stack has it) decides the size -- at most `limit`, halved while that is
 * wider than the shape -- and the format: the one asked for if the driver
 * takes it and it holds the shape's alpha (the scans say which), else 8888
 * for 32-bit art within the driver's texture memory, else by the alpha among
 * 1555, 4444, 565 and the others the driver takes, else the first it takes
 * at all.  Then the converter for the shape's kind and the format.  The
 * kind and alpha of a shape of no known format are what the code's stack
 * held there. */
Result shapeToPixelsOf(App* app, CPU& cpu, reg32 esp, reg32 target, reg32 shape, reg32 flag, int32_t limit,
                       reg32 format)
{
    const reg32 frame = esp - 0xc - 0x9c;
    const reg32 base = esp - 0xc - 0x7e;  // ebp as the code has it: its locals about it
    reg32 about;
    if (!callGame(app, cpu, frame, dwordAt(app, kThrashAbout), {target, shape, reg32(limit), reg32(limit)}, {}, about))
        return {};
    const int32_t shapeWidth = sword(app, shape + kShapeWidth);
    const reg32 asked = dwordAt(app, format);
    if (asked != 3 && asked != 4 && asked != 7 && asked != 6 && asked != 0)
        putDword(app, format, 0);
    reg32 record = about;
    if (record == 0)
    {
        record = frame;
        putDword(app, record + 0x1c, 2);
        putDword(app, record + 0x10, 2);
        putDword(app, record + 0x20, 0x100);
        putDword(app, record + 0x14, 0x100);
    }
    const auto field = [&](reg32 offset) { return int32_t(dwordAt(app, record + offset)); };
    int32_t width, height;
    if (limit != 0)
    {
        int32_t least = field(0x10);
        if (least <= field(0x1c))
            least = field(0x1c);
        int32_t most = field(0x14);
        if (most >= field(0x20))
            most = field(0x20);
        if (most >= limit)
            most = limit;
        width = most;
        while (shapeWidth < width && least <= width)
            width >>= 1;
        if (least > width)
            width = least;
        height = width;
    }
    else
    {
        width = sword(app, shape + kShapeWidth);
        height = sword(app, shape + kShapeHeight);
    }
    const auto formats = [&]() { return dwordAt(app, record + 0x30); };
    if (dwordAt(app, format) != 0 && dwordAt(app, formats() + dwordAt(app, format) * 4) == 0)
        putDword(app, format, 0);
    // The shape's kind and its alpha, by its format.
    int32_t kind = int32_t(dwordAt(app, base + 0x66)), alpha = int32_t(dwordAt(app, base + 0x6a));
    const reg32 pixels = shape + kShapePixels;
    const int32_t w = sword(app, shape + kShapeWidth), h = sword(app, shape + kShapeHeight);
    switch (byteAt(app, shape))
    {
    case 0x7a: kind = 1; alpha = alphaOfNibbles(app, pixels, w, h); break;
    case 0x7b: kind = 2; alpha = alphaOfPaletted(app, pixels, w, h); break;
    case 0x7d: kind = 6; alpha = alphaOf8888(app, pixels, w, h); break;
    case 0x6d: kind = 7; alpha = alphaOf4444(app, pixels, w, h); break;
    case 0x7e: kind = 3; alpha = alphaOf1555(app, pixels, w, h); break;
    case 0x78: kind = 4; alpha = alphaOf565(app, pixels, w, h); break;
    default: break;
    }
    // A format the driver takes and that keeps alpha: bit 2 of its word says it would not.
    const auto takes = [&](reg32 f) { return dwordAt(app, formats() + f * 4) != 0; };
    const auto keeps = [&](reg32 f) { return takes(f) && !(byteAt(app, formats() + f * 4) & 4); };
    const auto offers = [&](reg32 f) { return (byteAt(app, formats() + f * 4) & 1) != 0; };
    bool choose = dwordAt(app, format) == 0;
    if (!choose && alpha != 0 && (byteAt(app, formats() + dwordAt(app, format) * 4) & 4))
    {
        putDword(app, format, 0);
        choose = true;
    }
    if (choose)
    {
        if (kind == 6 && offers(6)
            && ((byteAt(app, record + 0x75) & 0x80) || ((byteAt(app, record + 0x74) & 1) && field(0x70) > 0x200000)
                || field(0x70) > 0x400000))
            putDword(app, format, 6);
        else
        {
            if (kind == 2 && takes(2))
                putDword(app, format, 2);
            else if (kind == 7)
                putDword(app, format, 7);
            else if (alpha == 0)
            {
                if (offers(4))
                    putDword(app, format, 4);
                else if (offers(0xb))
                    putDword(app, format, 0xb);
                else if (offers(3))
                    putDword(app, format, 3);
                else if (offers(7))
                    putDword(app, format, 7);
            }
            else if (alpha == 1)
            {
                if (keeps(3))
                    putDword(app, format, 3);
                else if (keeps(7))
                    putDword(app, format, 7);
                else if (keeps(6))
                    putDword(app, format, 6);
            }
            else if (alpha == 2)
            {
                if (keeps(7))
                    putDword(app, format, 7);
                else if (keeps(6))
                    putDword(app, format, 6);
            }
            // Still none: the first the driver takes at all.
            if (dwordAt(app, format) == 0)
            {
                for (const reg32 f : {4u, 0xbu, 3u, 7u, 6u})
                    if (takes(f))
                    {
                        putDword(app, format, f);
                        break;
                    }
            }
        }
    }
    if (reg32(kind) - 1 <= 6)
        if (const Converter convert = converterFor(kind, dwordAt(app, format)))
            convert(app, cpu, target + kShapePixels, pixels, w, h, width, height, flag);
    putWord(app, target + kShapeWidth, uint16_t(width));
    putWord(app, target + kShapeHeight, uint16_t(height));
    return height;
}

/* 0x4d5db0: the shapes known searched by name (strncmp, as the code
 * compares), else -- after a tick -- the shape found in the menus' art and
 * made textures, its name kept.  The registers the clock's wait is handed
 * are those the last comparison left, the entry's without one. */
Result shapeNamed(App* app, CPU& cpu, reg32 esp, reg32 name)
{
    const reg32 frame = esp - 0x18;
    for (int32_t i = 0; i < int32_t(dwordAt(app, kArtShapeCount)); ++i)
    {
        reg32 different;
        if (!callGame(app, cpu, frame, kStrncmp, {kArtShapes + reg32(i) * 0x14 + 0x10, name, 4, name}, {}, different))
            return {};
        if (different == 0)
            return i;
    }
    reg32 shape;
    if (!callGame(app, cpu, frame, kTickWait, {cpu.eax, cpu.edx, cpu.ebx, name})
        || !callGame(app, cpu, frame, kLocateShape, {dwordAt(app, kMenuArt), name, cpu.ebx, name}, {}, shape))
        return {};
    if (shape == 0)
        return 0;
    const Result record = artTexturesOf(app, cpu, frame - 4, shape, 0, name);
    if (!record)
        return {};
    if (*record != 0
        && !callGame(app, cpu, frame, kStrncpy, {kArtShapes + reg32(*record) * 0x14 + 0x10, name, 4, name}))
        return {};
    return record;
}

/* 0x4d7160: not on a race's pause screens; a trace line for a record
 * without tiles.  Each tile its texture, a quad of its size where it sits
 * from (x, y), at the menus' depth, clipped and drawn.  Direct3D draws a
 * record of more than 10 tiles unfiltered. */
Result drawArt(App* app, CPU& cpu, reg32 esp, int32_t record, int32_t x, int32_t y, reg32 colour)
{
    const reg32 frame = esp - 0xc - 0x20;
    if (dwordAt(app, kInRace) != 0)
        return record;
    const reg32 at = kArtShapes + reg32(record) * 0x14;
    int32_t tile = sword(app, at + 0xc);
    if (tile == -1)
    {
        if (!callGame(app, cpu, frame, kTrace, {reg32(record) * 0x14, reg32(tile), 0, colour}, {kNoArtTiles, reg32(record)}))
            return {};
        reg32 r;
        if (!callGame(app, cpu, frame, kInkey, {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx}, {}, r))
            return {};
        return int32_t(r);
    }
    // Each corner's depth and w, and the colour.
    const F depth = F(floatAt(app, kShapeDepth)), w = F(floatAt(app, kShapeW));
    putFloat(app, corner(3) + 8, float(depth));
    putFloat(app, corner(3) + 0xc, float(w));
    for (reg32 c = 3; c < 4; --c)
        putDword(app, corner(c) + 0x10, colour);
    const F lastW = F(floatAt(app, corner(3) + 0xc));
    for (reg32 c = 2; c < 3; --c)
    {
        putDword(app, corner(c) + 8, dwordAt(app, corner(c + 1) + 8));
        putFloat(app, corner(c) + 0xc, float(lastW));
    }
    bool unfiltered = false;
    if (byteAt(app, kDriverFlags) & 0x40)
    {
        const reg32 apart = reg32(tile - sword(app, at + 0xe));
        if (int32_t(int32_t(apart) > 0 ? apart : 0u - apart) > 0xa)
        {
            if (!callGame(app, cpu, frame, dwordAt(app, kThrashSetState), {apart, apart}, {kStateTextureFilter, 0}))
                return {};
            unfiltered = true;
        }
    }
    for (int32_t i = sword(app, at + 0xc);; ++i)
    {
        const int32_t end = sword(app, at + 0xe);
        if (end <= i)
        {
            if (!unfiltered)
                return end;
            reg32 r;
            if (!callGame(app, cpu, frame, dwordAt(app, kThrashSetState), {reg32(end), reg32(i)},
                          {kStateTextureFilter, 1}, r))
                return {};
            return int32_t(r);
        }
        // This tile: its texture, its size, where it sits in the record.
        const reg32 t = kArtTiles + reg32(tile) * 0xc;
        ++tile;
        const F size = F(x86::sreg16(wordAt(app, t + 4)));
        const F left = fp::add(F(int32_t(reg32(sword(app, t + 6)) << 5)), F(x));
        putFloat(app, corner(3), float(left));
        putFloat(app, corner(0), float(left));
        const int32_t top = int32_t((reg32(sword(app, t + 8)) << 5) + reg32(y));
        const F right = fp::add(F(floatAt(app, corner(0))), size);
        putFloat(app, corner(2), float(right));
        putFloat(app, corner(1) + 4, float(F(top)));
        const reg32 texture = dwordAt(app, t);
        const F rightAgain = F(floatAt(app, corner(2))), topAgain = F(floatAt(app, corner(1) + 4));
        putFloat(app, corner(1), float(rightAgain));
        const F bottom = fp::add(size, topAgain);
        putFloat(app, corner(0) + 4, float(topAgain));
        putFloat(app, corner(3) + 4, float(bottom));
        putFloat(app, corner(2) + 4, float(bottom));
        if (!callGame(app, cpu, frame, kSetRenderState, {1, texture}))
            return {};
        for (reg32 c = 0; c < 4; ++c)
            clipCode(app, corner(c));
        if (!drawTileQuad(app, cpu, frame - 4))
            return {};
    }
}

/* The game's entries: each its adapter (the game's registers and stack in,
 * the result in eax out) around what it does. */

namespace
{

/* A scan's entry: the pixels at eax, edx and ebx their counts. */
void scanEntry(App* app, CPU& cpu, int32_t (*scan)(App*, reg32, int32_t, int32_t))
{
    const Entry e(cpu);
    e.back(cpu, reg32(scan(app, e.eax, int32_t(e.edx), int32_t(e.ebx))));
}

/* A converter's entry (stdcall 0xc): the texture's pixels at eax, the
 * shape's at edx, ebx by ecx of them, the texture's size and the flag's word
 * on the stack.  What the code leaves in eax no caller reads. */
void convertEntry(App* app, CPU& cpu, Converter convert)
{
    const Entry e(cpu);
    convert(app, cpu, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx), int32_t(e.arg(app, 0)), int32_t(e.arg(app, 1)),
            e.arg(app, 2));
    e.back(cpu, e.eax, 0xc);
}

}  // namespace

// 0x4d18e0 .. 0x4d1ad0: the alpha scans.
void scan4444(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOf4444); }
void scan8888(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOf8888); }
void scanNibbles(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOfNibbles); }
void scanPaletted(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOfPaletted); }
void scan1555(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOf1555); }
void scan565(App* app, CPU& cpu) { scanEntry(app, cpu, alphaOf565); }

// 0x4d1b20 .. 0x4d3230: the converters.
void convert4444(App* app, CPU& cpu) { convertEntry(app, cpu, same4444); }
void convert8888(App* app, CPU& cpu) { convertEntry(app, cpu, same8888); }
void convert8(App* app, CPU& cpu) { convertEntry(app, cpu, same8); }
void convert565(App* app, CPU& cpu) { convertEntry(app, cpu, same565); }
void convert1555(App* app, CPU& cpu) { convertEntry(app, cpu, same1555); }
void nibblesTo1555(App* app, CPU& cpu) { convertEntry(app, cpu, nibbles1555); }
void nibblesTo565(App* app, CPU& cpu) { convertEntry(app, cpu, nibbles565); }
void nibblesTo4444(App* app, CPU& cpu) { convertEntry(app, cpu, nibbles4444); }
void nibblesTo8888(App* app, CPU& cpu) { convertEntry(app, cpu, nibbles8888); }
void paletteTo1555(App* app, CPU& cpu) { convertEntry(app, cpu, palette1555); }
void paletteTo565(App* app, CPU& cpu) { convertEntry(app, cpu, palette565); }
void paletteTo4444(App* app, CPU& cpu) { convertEntry(app, cpu, palette4444); }
void paletteTo8888(App* app, CPU& cpu) { convertEntry(app, cpu, palette8888); }
void argb8888To4444(App* app, CPU& cpu) { convertEntry(app, cpu, argb4444); }
void argb8888To565(App* app, CPU& cpu) { convertEntry(app, cpu, argb565); }
void argb8888To1555(App* app, CPU& cpu) { convertEntry(app, cpu, argb1555); }
void rgb565To1555(App* app, CPU& cpu) { convertEntry(app, cpu, rgb1555); }
void rgb565To4444(App* app, CPU& cpu) { convertEntry(app, cpu, rgb4444); }
void rgb565To8888(App* app, CPU& cpu) { convertEntry(app, cpu, rgb8888); }
void argb1555To565(App* app, CPU& cpu) { convertEntry(app, cpu, argb1555To565); }
void argb1555To4444(App* app, CPU& cpu) { convertEntry(app, cpu, argb1555To4444); }
void argb1555To8888(App* app, CPU& cpu) { convertEntry(app, cpu, argb1555To8888); }

/* 0x4d37a0 (stdcall 4): shape edx into pixels at eax + 0x10, ebx the flag's
 * word, ecx the most a side, the format's word on the stack. */
void shapeToPixels(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, shapeToPixelsOf(app, cpu, e.esp, e.eax, e.edx, e.ebx, int32_t(e.ecx), e.arg(app, 0)), 4);
}

/* 0x4d5c40: the textures let go of. */
void resetTextures(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, texturesLetGo(app, cpu, e.esp));
}

/* 0x4d5db0: the menus' shape named at eax. */
void shapeByName(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, shapeNamed(app, cpu, e.esp, e.eax));
}

/* 0x4d6070: shape edx a texture into [eax], ebx the most a side, ecx the
 * format asked for. */
void shapeTexture(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, textureOf(app, cpu, e.esp, e.eax, e.edx, int32_t(e.ebx), e.ecx));
}

/* 0x4d6110 (stdcall 4): the tile of ebx pixels at (ecx, the stack's word)
 * of shape eax into edx. */
void cutTile(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(cutTileOf(app, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx), int32_t(e.arg(app, 0)))), 4);
}

/* 0x4d6190: shape eax made 1555; eax as it was. */
void shape565To1555(App* app, CPU& cpu)
{
    const Entry e(cpu);
    shape565To1555Of(app, e.eax);
    e.back(cpu, e.eax);
}

/* 0x4d6210: the tiles' room at column eax, row edx; ebx not 0 to start it. */
void tileRoom(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(tileRoomOf(app, int32_t(e.eax), int32_t(e.edx), e.ebx != 0)));
}

/* 0x4d6390: the art shape eax made textures, edx the record to fill again
 * (0 a new one). */
void artShapeTextures(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, artTexturesOf(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ecx));
}

/* 0x4d66f0: record eax's size into the words at edx and ebx. */
void shapeSize(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, shapeSizeOf(app, int32_t(e.eax), e.edx, e.ebx));
}

/* 0x4d70f0: the tiles' quad drawn. */
void drawShapeQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, drawTileQuad(app, cpu, e.esp));
}

/* 0x4d7160: art record eax drawn at edx, ebx in colour ecx. */
void drawArtShape(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, drawArt(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

/* 0x4d7990: art record eax drawn at edx, ebx, opaque. */
void drawShape(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, drawArt(app, cpu, e.esp - 0xc, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), 0xffffffff));
}

}  // namespace shapes
}  // namespace port
}  // namespace nfs3hp
