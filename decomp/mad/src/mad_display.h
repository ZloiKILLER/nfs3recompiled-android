/*
 * The MAD player's software display path of Need for Speed III, decompiled
 * from nfs3.exe: packed 7-bit YUV 4:2:2 (what mad_decoder.c writes) to the
 * 15- or 16-bit screen, at 1x or 2x, with the original's interpolation.
 *
 * Colour: BT.601 (JPEG) with 7-bit components --
 *     Y8 = 2*y + 1,  c = chroma - 64 + 0.5
 *     R8 = clamp(2*(y + round(1.402 c_r)) + 1)
 *     G8 = clamp(2*(y + round(-0.3441 c_b) + round(-0.7141 c_r)) + 1)
 *     B8 = clamp(2*(y + round(1.772 c_b)) + 1)
 * then to 5/6 bits, rounded for the left pixel of each pair and truncated
 * for the right one (a two-pixel dither).  Chroma byte 1 of a word is Cb,
 * byte 0 is Cr.
 *
 * In the game the player always uses this path (sub_4df1f0 is called with
 * its "software" flag set); the mode comes from the measured CPU speed:
 * < 133 MHz -> 1, < 200 MHz -> 2, otherwise 3.  The port reports a 2 GHz CPU,
 * so mode 3.
 */
#ifndef MAD_DISPLAY_H
#define MAD_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Colour tables, 0x9f4fb0..0x9f641c in the game, in this order. */
typedef struct MadRgbTables
{
    uint32_t blue[354];   /* 0x9f4fb0  9-bit blue field  -> pixel bits, rounded | truncated << 16 */
    uint32_t green[262];  /* 0x9f5538  9-bit green field */
    uint32_t red[306];    /* 0x9f5950  9-bit red field */
    uint32_t luma[128];   /* 0x9f5e18  y  -> R:9 @23 | G:9 @11 | B:9 @0, with offsets */
    uint32_t cb[128];     /* 0x9f6018  Cb -> G @11 | B @0 */
    uint32_t cr[128];     /* 0x9f6218  Cr -> R @23 | G @11 */
    int32_t  depth;       /* 0x9f6418  depth the tables are built for (0: none) */
} MadRgbTables;

/* The screen as the player sees it: the game's surface description,
 * 0x564ff4..0x56502c (only the fields the converter reads). */
typedef struct MadSurface
{
    int32_t clip_left;          /* 0x565000 */
    int32_t clip_top;           /* 0x565004 */
    int32_t clip_right;         /* 0x565008 */
    int32_t clip_bottom;        /* 0x56500c */
    uint8_t depth;              /* 0x565010  15 or 16 */
    uint8_t *base;              /* 0x565014 */
    const int32_t *row_offset;  /* 0x565020  byte offset of each row */
    const int32_t *x_offset;    /* 0x565024  byte offset of each column */
    /* sub_4f90e0, asked before the second pass over a picture larger than
     * 0x25800 pixels; nonzero to go on.  NULL: go on. */
    int32_t (*large_picture_ok)(void *user);
    void *user;
} MadSurface;

/* sub_4fda20: build the colour tables for a 15- or 16-bit screen. */
void mad_build_rgb_tables(MadRgbTables *t, int32_t depth);

/* sub_4fdc90: show a frame of `w` x `h` pixels (packed 4:2:2, 2 bytes a
 * pixel) at (x, y), clipped to the surface.  mode 0: 1x; 1: 2x, odd rows
 * left as they are; 2: 2x, odd rows the average of their neighbours;
 * 3: 2x interpolated across and down.  Returns 0, -1 (mode), -2 (depth),
 * what large_picture_ok said when it said no, or -3 (not in the original)
 * for a mode-3 row wider than the original's 640-dword buffer. */
int32_t mad_show_frame(MadRgbTables *t, const MadSurface *s, int32_t x, int32_t y,
                       const uint8_t *frame, int32_t w, int32_t h, int32_t mode);

/* Row converters, exported for the tests.  Words are packed 4:2:2. */
void mad_row_1x(const MadRgbTables *t, const uint32_t *src, uint32_t *dst, int32_t words);            /* sub_4fd61c */
void mad_row_2x(const MadRgbTables *t, const uint32_t *src, uint32_t *dst, int32_t words);            /* sub_4fd6ef */
void mad_row_2x_between(const MadRgbTables *t, const uint32_t *a, const uint32_t *b,
                        uint32_t *dst, int32_t words);                                                 /* sub_4fd7b6 */
void mad_row_widen(const uint32_t *src, uint32_t *dst, int32_t words);                                /* sub_4fd892 */
void mad_row_widen_between(const uint32_t *a, const uint32_t *b, uint32_t *dst, int32_t words);       /* sub_4fd8ee */

#ifdef __cplusplus
}
#endif

#endif
