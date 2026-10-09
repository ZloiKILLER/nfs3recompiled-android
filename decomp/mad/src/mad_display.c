/*
 * MAD player software display path, decompiled from nfs3.exe.  See
 * mad_display.h.  32-bit arithmetic is done in uint32_t, as the original
 * wraps; the table builder's few floating-point products in double, as the
 * x87 gives them at 53-bit precision.
 */
#include "mad_display.h"

#include <math.h>
#include <string.h>

#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(__GNUC__)
#pragma GCC optimize("fp-contract=off")
#endif

/* x87 fistp in the default rounding mode: to nearest, ties to even. */
static int32_t fistp(double v)
{
    return (int32_t)nearbyint(v);
}

/* ------------------------------------------------------------------ */
/* sub_4fda20                                                          */
/* ------------------------------------------------------------------ */

void mad_build_rgb_tables(MadRgbTables *t, int32_t depth)
{
    /* Constants at 0x54e098..0x54e0b8. */
    const double cb_to_g = -0.3441, cb_to_b = 1.772, cr_to_r = 1.402, cr_to_g = -0.7141, half = 0.5;

    for (int32_t k = 0; k < 128; k++)
    {
        /* Luma in all three fields, each with its own offset so that the
         * chroma terms added later never take a field below zero:
         * R = 89 + y, G = 67 + y, B = 113 + y. */
        t->luma[k] = 0x2c800000u + 0x21800u + 0x71u + (uint32_t)k * 0x800801u;

        const double c = (double)(k - 64) + half;
        const int32_t g_u = fistp(c * cb_to_g);
        const int32_t b_u = fistp(c * cb_to_b);
        t->cb[k] = (((uint32_t)g_u & 0x1ffu) << 11) + ((uint32_t)b_u & 0x1ffu);
        const int32_t r_v = fistp(c * cr_to_r);
        const int32_t g_v = fistp(c * cr_to_g);
        t->cr[k] = (((uint32_t)r_v & 0x1ffu) << 23) + (((uint32_t)g_v & 0x1ffu) << 11);
    }

    /* Field -> pixel bits.  The 8-bit value is 2 * field - offset, which
     * undoes the field offset and doubles the 7-bit scale; the low half gets
     * it rounded, the high half truncated. */
    for (int32_t e = -153; e < 153; e++)
    {
        int32_t v = 2 * e + 0x81;
        v = v < 0 ? 0 : (v > 0xff ? 0xff : v);
        int32_t r = (v + 4) >> 3;
        const int32_t trunc = v >> 3;
        if (r > 0x1f)
            r = 0x1f;
        t->red[e + 153] = depth == 15 ? ((uint32_t)r << 10) + ((uint32_t)trunc << 26)
                                      : ((uint32_t)r << 11) + ((uint32_t)trunc << 27);
    }
    for (int32_t e = -131; e < 131; e++)
    {
        int32_t v = 2 * e + 0x81;
        v = v < 0 ? 0 : (v > 0xff ? 0xff : v);
        int32_t g, trunc;
        if (depth == 15)
        {
            g = (v + 4) >> 3;
            trunc = v >> 3;
            if (g > 0x1f)
                g = 0x1f;
        }
        else
        {
            g = (v + 2) >> 2;
            trunc = v >> 2;
            if (g > 0x3f)
                g = 0x3f;
        }
        t->green[e + 131] = ((uint32_t)g << 5) + ((uint32_t)trunc << 21);
    }
    for (int32_t e = -177; e < 177; e++)
    {
        int32_t v = 2 * e + 0x81;
        v = v < 0 ? 0 : (v > 0xff ? 0xff : v);
        int32_t b = (v + 4) >> 3;
        const int32_t trunc = v >> 3;
        if (b > 0x1f)
            b = 0x1f;
        t->blue[e + 177] = (uint32_t)b + ((uint32_t)trunc << 16);
    }
}

/* ------------------------------------------------------------------ */
/* row converters                                                      */
/* ------------------------------------------------------------------ */

/* The three fields of a sum to pixel bits (both halves). */
static uint32_t to_pixels(const MadRgbTables *t, uint32_t sum)
{
    return t->red[sum >> 23] + t->green[(sum >> 11) & 0x1ff] + t->blue[sum & 0x1ff];
}

/* sub_4fd61c: a row at 1x, two pixels a word in, two a dword out. */
void mad_row_1x(const MadRgbTables *t, const uint32_t *src, uint32_t *dst, int32_t words)
{
    do
    {
        const uint32_t w = *src++;
        const uint32_t chroma = t->cr[w & 0xff] + t->cb[(w >> 8) & 0xff];
        const uint32_t left = to_pixels(t, t->luma[w >> 24] + chroma);
        const uint32_t right = to_pixels(t, t->luma[(w >> 16) & 0xff] + chroma);
        *dst++ = (right & 0xffff0000u) | (left & 0xffffu);
    } while (--words != 0);
}

/* sub_4fd6ef: a row at 2x across -- every pixel twice. */
void mad_row_2x(const MadRgbTables *t, const uint32_t *src, uint32_t *dst, int32_t words)
{
    do
    {
        const uint32_t w = *src++;
        const uint32_t chroma = t->cr[w & 0xff] + t->cb[(w >> 8) & 0xff];
        dst[0] = to_pixels(t, t->luma[w >> 24] + chroma);
        dst[1] = to_pixels(t, t->luma[(w >> 16) & 0xff] + chroma);
        dst += 2;
    } while (--words != 0);
}

/* Average of two words, byte by byte, as the original does it in one add. */
static uint32_t average(uint32_t a, uint32_t b)
{
    return ((a + b) >> 1) & 0x7f7f7f7fu;
}

/* sub_4fd7b6: the average of two rows, at 2x across. */
void mad_row_2x_between(const MadRgbTables *t, const uint32_t *a, const uint32_t *b, uint32_t *dst, int32_t words)
{
    do
    {
        const uint32_t w = average(*a++, *b++);
        const uint32_t chroma = t->cr[w & 0xff] + t->cb[(w >> 8) & 0xff];
        dst[0] = to_pixels(t, t->luma[w >> 24] + chroma);
        dst[1] = to_pixels(t, t->luma[(w >> 16) & 0xff] + chroma);
        dst += 2;
    } while (--words != 0);
}

/* One word to two, interpolating towards the next word `n`:
 *   first:  left, avg(left, right), own chroma
 *   second: right, avg(right, n.left), chroma averaged with n's */
static void widen(uint32_t s, uint32_t n, uint32_t *out)
{
    const uint32_t right_up = (s << 8) & 0x7f000000u;     /* s.right in the left slot  */
    const uint32_t next_left = (n >> 8) & 0x007f0000u;    /* n.left in the right slot  */
    const uint32_t mixed = (((n & 0x7f7fu) | right_up | next_left) + s) >> 1;
    out[1] = (mixed & 0x007f7f7fu) | right_up;
    out[0] = (s & 0x7f007f7fu) | ((mixed >> 8) & 0x007f0000u);
}

/* sub_4fd892: a row widened to twice its words (reads one word past). */
void mad_row_widen(const uint32_t *src, uint32_t *dst, int32_t words)
{
    do
    {
        widen(src[0], src[1], dst);
        src++;
        dst += 2;
    } while (--words != 0);
}

/* sub_4fd8ee: the average of two rows, widened. */
void mad_row_widen_between(const uint32_t *a, const uint32_t *b, uint32_t *dst, int32_t words)
{
    uint32_t s = average(*a++, *b++);
    do
    {
        const uint32_t n = average(*a++, *b++);
        widen(s, n, dst);
        dst += 2;
        s = n;
    } while (--words != 0);
}

/* ------------------------------------------------------------------ */
/* sub_4fdc90                                                          */
/* ------------------------------------------------------------------ */

static uint32_t *row_at(const MadSurface *s, int32_t x, int32_t row)
{
    return (uint32_t *)(s->base + s->x_offset[x] + s->row_offset[row]);
}

/* Watcom's idiv: truncating. */
static int32_t idiv(int32_t a, int32_t b)
{
    return a / b;
}

int32_t mad_show_frame(MadRgbTables *t, const MadSurface *s, int32_t x, int32_t y,
                       const uint8_t *frame, int32_t w, int32_t h, int32_t mode)
{
    int32_t scale;
    if (mode == 0)
        scale = 1;
    else if (mode == 1 || mode == 2 || mode == 3)
        scale = 2;
    else
        return -1;

    const int32_t depth = s->depth;
    if (depth != 15 && depth != 16)
        return -2;
    if (depth != t->depth)
    {
        mad_build_rgb_tables(t, depth);
        t->depth = depth;
    }

    /* Start on an even pixel, or an odd one if the surface's origin is
     * two bytes off a dword. */
    const uint32_t misalign = ((uint32_t)(uintptr_t)s->base + (uint32_t)s->x_offset[0] + (uint32_t)s->row_offset[0]) & 3u;
    x = (int32_t)(((uint32_t)x & ~1u) | (misalign >> 1));

    int32_t x_end = x + scale * w;
    int32_t y_end = y + h * scale;
    const int32_t row_words = w / 2;
    const uint8_t *src = frame;

    if (x < s->clip_left)
    {
        const int32_t step = scale * 2;
        const int32_t n = idiv(s->clip_left - x + step - 1, step);
        x += n * step;
        src += n * 4;
    }
    if (x_end > s->clip_right)
        x_end = s->clip_right;
    if (y < s->clip_top)
    {
        const int32_t n = idiv(s->clip_top - y + scale - 1, scale);
        y += n * scale;
        src += n * row_words * 4;
    }
    if (y_end > s->clip_bottom)
        y_end = s->clip_bottom;

    const int32_t words = idiv(x_end - x, scale * 2);
    if (words < 2)
        return 0;
    const int32_t row_bytes = row_words * 4;
    uint32_t tmp[640];  /* the original's 0xa00-byte stack buffer */
    if (mode == 3 && 2 * words - 2 > 640)
        return -3;      /* not in the original, which overruns its stack */

    if (mode == 0)
    {
        for (int32_t row = y; row < y_end; row++)
        {
            mad_row_1x(t, (const uint32_t *)src, row_at(s, x, row), words);
            src += row_bytes;
        }
        return 0;
    }

    if (mode == 3)
    {
        const uint8_t *p = src;
        for (int32_t row = y; row < y_end; row += 2)
        {
            mad_row_widen((const uint32_t *)p, tmp, words - 1);
            mad_row_1x(t, tmp, row_at(s, x, row), 2 * words - 2);
            p += row_bytes;
        }
        if ((y_end - y) * (x_end - x) > 0x25800 && s->large_picture_ok)
        {
            const int32_t ok = s->large_picture_ok(s->user);
            if (!ok)
                return ok;
        }
        p = src;
        for (int32_t row = y + 1; row < y_end - 1; row += 2)
        {
            const uint8_t *next = p + row_bytes;
            mad_row_widen_between((const uint32_t *)p, (const uint32_t *)next, tmp, words - 1);
            mad_row_1x(t, tmp, row_at(s, x, row), 2 * words - 2);
            p = next;
        }
        return 0;
    }

    /* modes 1 and 2 */
    {
        const uint8_t *p = src;
        for (int32_t row = y; row < y_end; row += 2)
        {
            mad_row_2x(t, (const uint32_t *)p, row_at(s, x, row), words);
            p += row_bytes;
        }
    }
    if (mode != 2)
        return 0;
    if ((y_end - y) * (x_end - x) > 0x25800 && s->large_picture_ok)
    {
        const int32_t ok = s->large_picture_ok(s->user);
        if (!ok)
            return ok;
    }
    {
        const uint8_t *p = src;
        for (int32_t row = y + 1; row < y_end - 1; row += 2)
        {
            const uint8_t *next = p + row_bytes;
            mad_row_2x_between(t, (const uint32_t *)p, (const uint32_t *)next, row_at(s, x, row), words);
            p = next;
        }
    }
    return 0;
}
