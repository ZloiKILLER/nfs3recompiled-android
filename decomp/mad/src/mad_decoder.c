/*
 * EA MAD video decoder of Need for Speed III, decompiled from nfs3.exe.
 * See mad_decoder.h for the picture format and the state layout.
 *
 * Arithmetic is kept exactly as the original does it: 32-bit wrap-around
 * (done in uint32_t to stay defined in C), the bit reader's word order, the
 * x87 part of the IDCT in double precision in the original order.  Build
 * without FMA contraction (-ffp-contract=off): a fused multiply-add rounds
 * once where the original rounds twice.
 *
 * Little-endian hosts only (x86, arm64): the decoder addresses single bytes
 * inside 32-bit words, as the original does.
 */
#include "mad_decoder.h"
#include "mad_tables.h"

#include <string.h>

#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(__GNUC__)
#pragma GCC optimize("fp-contract=off")
#endif

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

static float bits_to_float(uint32_t bits)
{
    float f;
    memcpy(&f, &bits, sizeof f);
    return f;
}

static uint32_t rd16(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8);
}

/* High dword of a signed 32x32 product -- `imul` leaves it in edx. */
static uint32_t mul_high(uint32_t a, uint32_t b)
{
    return (uint32_t)((uint64_t)((int64_t)(int32_t)a * (int64_t)(int32_t)b) >> 32);
}

/* Q16 multiply rounded to nearest, as sub_4f3560 does it:
 * shr eax,16 / adc eax,edx<<16 -- the carry is bit 15 of the low dword. */
static uint32_t fixmul_round(uint32_t a, uint32_t b)
{
    int64_t p = (int64_t)(int32_t)a * (int64_t)(int32_t)b;
    uint32_t lo = (uint32_t)p, hi = (uint32_t)((uint64_t)p >> 32);
    return (hi << 16) + (lo >> 16) + ((lo >> 15) & 1u);
}

/* Low dword of the double's bit pattern: the "magic number" conversion the
 * IDCT uses (value + 1.5*2^52 stored as a qword). */
static uint32_t low_dword(double v)
{
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    return (uint32_t)b;
}

/* ------------------------------------------------------------------ */
/* sub_4f30d0: tables                                                  */
/* ------------------------------------------------------------------ */

static void fill(uint32_t *table, int32_t start, int32_t count, uint32_t entry)
{
    /* `test edi,edi / jle`: a count that came out of an oversized shift is
     * negative and fills nothing. */
    for (int32_t i = 0; i < count; i++)
        table[start + i] = entry;
}

void mad_build_tables(MadDecoder *d)
{
    /* Clamp: a signed byte -128..126 to -64..63, then +64.  Index 0x7f (127)
     * is never written and keeps the BSS's 0: a sample whose integer part
     * comes out exactly 127 is shown black.  An original quirk. */
    for (int32_t v = -128; v < 127; v++)
    {
        int32_t c = v < -64 ? -64 : (v > 63 ? 63 : v);
        d->clamp[(uint8_t)v] = (uint8_t)(c + 64);
    }

    /* First-nine-bits table: what its low byte means (sub_510e48):
     *   <= 9   code length, the entry is the code
     *   0x0f   000000000: nine more zero bits, look up vlc_after9
     *   0x1f   0000000xx: six zero bits, look up vlc_after6
     *   0x2f   000001xxx: escape, 16 bits of level:10 run:6 follow
     *   0x3f   10xxxxxxx: end of block */
    d->vlc_first9[0] = 0x0f;
    for (int32_t i = 1; i < 8; i++)
        d->vlc_first9[i] = 0x1f;
    for (int32_t i = 8; i < 16; i++)
        d->vlc_first9[i] = 0x2f;
    for (int32_t i = 256; i < 384; i++)
        d->vlc_first9[i] = 0x3f;

    /* Table A, records 1..94 (record 0 is end of block).  An entry is
     * level(signed 10 bits) << 22 | run << 16 | length. */
    for (int32_t k = 1; k < 95; k++)
    {
        const uint32_t len = (uint32_t)MAD_VLC_A[k][0];
        const uint32_t rl = (uint32_t)MAD_VLC_A[k][1];
        const uint32_t code = (uint32_t)MAD_VLC_A[k][3];
        uint32_t e = ((rl & 0x3ffu) << 22) | ((rl & 0xfc00u) << 6);
        if ((code & 0xfc00u) == 0)
        {
            /* Starts with six zero bits: indexed by the eight after them. */
            const uint32_t rest = len - 6;
            e |= rest;
            fill(d->vlc_after6, (int32_t)code >> 2, (int32_t)(1u << ((8 - rest) & 31)), e);
        }
        else
        {
            e |= len;
            fill(d->vlc_first9, (int32_t)code >> 7, (int32_t)(1u << ((9 - len) & 31)), e);
        }
    }

    /* Table B, records 0..127: codes that start with eight zero bits. */
    for (int32_t k = 0; k < 128; k++)
    {
        const uint32_t len = (uint32_t)MAD_VLC_B[k][0] + 8;
        const uint32_t rl = (uint32_t)MAD_VLC_B[k][1];
        const uint32_t code = (uint32_t)MAD_VLC_B[k][3];
        uint32_t e = ((rl & 0x3ffu) << 22) | ((rl & 0xfc00u) << 6);
        if (code & 0x8000u)
        {
            /* 000000001...: the eight bits after six zeros start "001". */
            const uint32_t rest = len - 6;
            e |= rest;
            fill(d->vlc_after6, (int32_t)code >> 10, (int32_t)(1u << ((8 - rest) & 31)), e);
        }
        else
        {
            /* 000000000...: the eight bits after nine zeros. */
            const uint32_t rest = len - 9;
            e |= rest;
            fill(d->vlc_after9, (int32_t)code >> 7, (int32_t)(1u << ((8 - rest) & 31)), e);
        }
    }

    /* Small-number code (motion vectors and block offsets), top six bits:
     *   0.....  -> 0, one bit
     *   10xxxx  -> xxxx + 1   (1..16), six bits
     *   11xxxx  -> xxxx - 16  (-16..-1), six bits */
    for (int32_t i = 0; i < 32; i++)
        d->vlc_mv[i] = 1;
    for (int32_t k = 1; k <= 16; k++)
        d->vlc_mv[31 + k] = ((uint32_t)k << 22) | 6;
    for (int32_t j = 0; j < 16; j++)
        d->vlc_mv[48 + j] = ((uint32_t)(-16 + j) << 22) | 6;

    d->tables_ready = 1;
}

/* ------------------------------------------------------------------ */
/* bit reader                                                          */
/* ------------------------------------------------------------------ */

/* sub_4f3350: drop n bits, pull in one 16-bit word when fewer than 16 are
 * left.  The stream is little-endian 16-bit words, most significant bit
 * first. */
void mad_skip_bits(MadDecoder *d, int32_t n)
{
    uint32_t bits = d->bits << (n & 31);
    int32_t left = d->nbits - n;
    if (left < 16)
    {
        bits |= rd16(d->ptr) << ((16 - left) & 31);
        d->ptr += 2;
        left += 16;
    }
    d->bits = bits;
    d->nbits = left;
}

/* sub_4f33b0: a motion vector component or a block's brightness offset. */
int32_t mad_read_small(MadDecoder *d)
{
    const uint32_t e = d->vlc_mv[d->bits >> 26];
    mad_skip_bits(d, (int32_t)(e & 0xff));
    return (int32_t)e >> 22;
}

/* ------------------------------------------------------------------ */
/* sub_4f3560: frame start                                             */
/* ------------------------------------------------------------------ */

void mad_begin_frame(MadDecoder *d, const uint8_t *data, int32_t inter, uint32_t quant)
{
    if (!d->tables_ready)
        mad_build_tables(d);
    d->inter = inter;
    d->nbits = 32;
    d->bits = (rd16(data) << 16) | rd16(data + 2);
    d->ptr = data + 4;
    /* DC: matrix << 15, not scaled by the quantiser. */
    d->qmat[0] = (int32_t)fixmul_round((uint32_t)MAD_QUANT_MATRIX[0] << 15, (uint32_t)MAD_AAN_SCALE[0]);
    for (int32_t i = 1; i < 64; i++)
        d->qmat[i] = (int32_t)fixmul_round(((uint32_t)MAD_QUANT_MATRIX[i] * quant) << 12,
                                           (uint32_t)MAD_AAN_SCALE[i]);
}

/* ------------------------------------------------------------------ */
/* sub_510e48: one block's coefficients                                */
/* ------------------------------------------------------------------ */

/* Store coefficient `i` of the scan, at the byte offset the scan table gives,
 * dequantised by the matrix entry at the same offset.  Past the end of the
 * scan the original reads whatever follows the table and writes wherever that
 * points; inside the block that is reproduced, outside it is flagged. */
static void put_coef(MadDecoder *d, uint32_t i, int32_t level)
{
    if (i >= sizeof MAD_SCAN_OFFSET / sizeof MAD_SCAN_OFFSET[0])
    {
        d->overflow |= 1;
        return;
    }
    const uint32_t off = (uint32_t)MAD_SCAN_OFFSET[i];
    if (off > sizeof d->coef - 4)
    {
        d->overflow |= 1;
        return;
    }
    int32_t q;
    memcpy(&q, (const uint8_t *)d->qmat + off, 4);
    const uint32_t v = (uint32_t)level * (uint32_t)q;
    memcpy((uint8_t *)d->coef + off, &v, 4);
    if (i > 63)
        d->overflow |= 2;  /* reproduced, but not a valid stream */
}

int32_t mad_decode_block(MadDecoder *d)
{
    const uint8_t *p = d->ptr;
    uint32_t bits = d->bits;
    int32_t n = d->nbits;

#define REFILL()                                                   \
    do                                                             \
    {                                                              \
        if (n < 16)                                                \
        {                                                          \
            bits |= rd16(p) << ((16 - n) & 31);                    \
            p += 2;                                                \
            n += 16;                                               \
        }                                                          \
    } while (0)

    /* DC: eight bits, signed, times qmat[0]. */
    d->coef[0] = (int32_t)((uint32_t)((int32_t)bits >> 24) * (uint32_t)d->qmat[0]);
    bits <<= 8;
    n -= 8;
    REFILL();
    memset(&d->coef[1], 0, 63 * sizeof d->coef[0]);

    uint32_t i = 1;
    for (;;)
    {
        uint32_t e = d->vlc_first9[bits >> 23];
        const uint8_t tag = (uint8_t)e;
        if ((int8_t)tag > 9)
        {
            if (tag >= 0x20)
            {
                if (tag < 0x30)
                {
                    /* Escape: 6 bits, then level:10 run:6 taken as a code of
                     * length 16. */
                    bits <<= 6;
                    n -= 6;
                    REFILL();
                    e = (bits & 0xffff0000u) | 0x10;
                }
                else
                {
                    /* End of block: 2 bits. */
                    bits <<= 2;
                    n -= 2;
                    REFILL();
                    break;
                }
            }
            else if (tag >= 0x10)
            {
                bits <<= 6;
                n -= 6;
                REFILL();
                e = d->vlc_after6[bits >> 24];
            }
            else
            {
                bits <<= 9;
                n -= 9;
                REFILL();
                e = d->vlc_after9[bits >> 24];
            }
        }
        const uint32_t run = (e >> 16) & 0x3f;
        const uint32_t len = e & 0xff;
        n -= (int32_t)len;
        i += run;
        bits <<= (len & 31);
        REFILL();
        put_coef(d, i, (int32_t)e >> 22);
        i++;
        if (d->overflow & 1)
            break;  /* not in the original, which would go on writing */
    }
#undef REFILL

    d->ptr = p;
    d->bits = bits;
    d->nbits = n;
    return (int32_t)i;
}

/* sub_4f33e0: a block with only its DC coefficient is that value everywhere. */
void mad_fill_dc(MadDecoder *d, int32_t *blk, int32_t stride)
{
    const int32_t v = d->coef[0];
    for (int32_t r = 0; r < 8; r++)
        for (int32_t c = 0; c < 8; c++)
            blk[r * stride + c] = v;
}

/* ------------------------------------------------------------------ */
/* IDCT (AAN), sub_510a50 / sub_510bb3 / sub_510cbd                    */
/* ------------------------------------------------------------------ */

/* The odd half, shared by both passes.  The x87 part in double precision, in
 * the original's order:
 *   z = in5 - in3, y = in1 - in7
 *   A = z*c1 + (y+z)*c2,   B = y*c0 - (y+z)*c2,  each + 1.5*2^52 -> int
 * The rest in 32-bit integers, K = sqrt(1/2) in Q31, `imul` + `shl edx,1`. */
typedef struct
{
    uint32_t v, vt, ut, us;
} OddHalf;

static OddHalf odd_half(const int32_t *in)
{
    const uint32_t i1 = (uint32_t)in[1], i3 = (uint32_t)in[3];
    const uint32_t i5 = (uint32_t)in[5], i7 = (uint32_t)in[7];
    const uint32_t s53 = i5 + i3, d53 = i5 - i3;
    const uint32_t s17 = i1 + i7, d17 = i1 - i7;
    const uint32_t sum4 = s17 + s53;
    const uint32_t dif4 = s17 - s53;

    const double c0 = (double)bits_to_float(MAD_IDCT_C0_BITS);
    const double c1 = (double)bits_to_float(MAD_IDCT_C1_BITS);
    const double c2 = (double)bits_to_float(MAD_IDCT_C2_BITS);
    const double magic = (double)bits_to_float(MAD_IDCT_MAGIC_BITS);

    const double z = (double)(int32_t)d53;  /* fild [0x56cde8] */
    const double y = (double)(int32_t)d17;  /* fild [0x56cdec] */
    const double yc0 = y * c0;
    const double yz = y + z;
    const double zc1 = z * c1;
    const double yzc2 = yz * c2;
    const double a = (zc1 + yzc2) + magic;
    const double b = (yc0 - yzc2) + magic;

    const uint32_t t = mul_high(dif4, MAD_IDCT_K) << 1;
    const uint32_t u = low_dword(b);
    OddHalf h;
    h.v = low_dword(a);
    h.vt = h.v + t;
    h.ut = t + u;
    h.us = u + sum4;
    return h;
}

/* Outputs 0..7 in the order the original stores them. */
static void even_and_out(const int32_t *in, OddHalf h, uint32_t out[8])
{
    const uint32_t i0 = (uint32_t)in[0], i2 = (uint32_t)in[2];
    const uint32_t i4 = (uint32_t)in[4], i6 = (uint32_t)in[6];
    uint32_t s26 = i2 + i6;
    const uint32_t d26 = i2 - i6;
    const uint32_t s04 = i0 + i4, d04 = i0 - i4;
    const uint32_t t2 = mul_high(d26, MAD_IDCT_K) << 1;
    s26 += t2;
    const uint32_t e0 = d04 + t2, e1 = d04 - t2;
    const uint32_t f0 = s04 + s26, f1 = s04 - s26;
    out[0] = f0 + h.us;
    out[1] = e0 + h.ut;
    out[2] = e1 + h.vt;
    out[3] = f1 + h.v;
    out[4] = f1 - h.v;
    out[5] = e1 - h.vt;
    out[6] = e0 - h.ut;
    out[7] = f0 - h.us;
}

/* sub_510a50: a row of the coefficient block into a column of idct_tmp
 * (outputs 9 dwords apart).  All-zero AC: every output is the DC. */
void mad_idct_row(const int32_t *in, int32_t *out)
{
    if ((in[5] | in[3] | in[1] | in[7] | in[2] | in[6] | in[4]) == 0)
    {
        for (int32_t j = 0; j < 8; j++)
            out[j * 9] = in[0];
        return;
    }
    uint32_t o[8];
    even_and_out(in, odd_half(in), o);
    for (int32_t j = 0; j < 8; j++)
        out[j * 9] = (int32_t)o[j];
}

/* sub_510bb3: a column (a row of idct_tmp) into a row of the output. */
void mad_idct_col(const int32_t *in, int32_t *out)
{
    uint32_t o[8];
    even_and_out(in, odd_half(in), o);
    for (int32_t j = 0; j < 8; j++)
        out[j] = (int32_t)o[j];
}

/* sub_510cbd: the block in coef[] to `dst`, rows `stride` dwords apart.  The
 * scan table is transposed (0, 8, 1, 2, 9, ...), so the first pass runs over
 * what are the picture's columns and the result comes out in raster order.
 * The samples are 16.16 fixed point: byte 2 of each dword is the value. */
void mad_idct(MadDecoder *d, int32_t *dst, int32_t stride)
{
    for (int32_t r = 0; r < 8; r++)
        mad_idct_row(&d->coef[r * 8], &d->idct_tmp[r]);
    for (int32_t k = 0; k < 8; k++)
        mad_idct_col(&d->idct_tmp[k * 9], dst + k * stride);
}

/* ------------------------------------------------------------------ */
/* prediction from the previous frame                                  */
/* ------------------------------------------------------------------ */

/* sub_4f3420: an 8x8 luma block from the reference picture, plus `offset`
 * (byte arithmetic, wraps).  `odd` starts one pixel later.  Only byte 2 of
 * each dword of the block is written -- the byte mad_put_row reads; the
 * other three keep whatever they held. */
void mad_predict_luma(const uint8_t *ref, int32_t odd, int32_t wdw, int32_t *blk, int32_t offset)
{
    static const uint8_t even_src[8] = { 3, 2, 7, 6, 11, 10, 15, 14 };
    static const uint8_t odd_src[8] = { 2, 7, 6, 11, 10, 15, 14, 19 };
    const uint8_t *src_index = odd ? odd_src : even_src;
    const int32_t stride = wdw * 4;
    for (int32_t r = 0; r < 8; r++)
    {
        const uint8_t *s = ref + r * stride;
        uint8_t *b = (uint8_t *)(blk + r * 16);
        for (int32_t j = 0; j < 8; j++)
            b[2 + 4 * j] = (uint8_t)(s[src_index[j]] + (uint8_t)offset);
    }
}

/* sub_4f34f0: an 8x8 chroma block -- byte `plane` (1 = A, 0 = B) of every
 * word on every other row -- plus `offset`.  No half-pixel step. */
void mad_predict_chroma(const uint8_t *ref, int32_t plane, int32_t wdw, int32_t *blk, int32_t offset)
{
    const int32_t stride2 = wdw * 8;
    const uint8_t *s = ref + plane;
    for (int32_t r = 0; r < 8; r++)
    {
        uint8_t *b = (uint8_t *)(blk + r * 8);
        for (int32_t j = 0; j < 8; j++)
            b[2 + 4 * j] = (uint8_t)(s[r * stride2 + 4 * j] + (uint8_t)offset);
    }
}

/* ------------------------------------------------------------------ */
/* output                                                              */
/* ------------------------------------------------------------------ */

/* sub_510ff8: one row of 16 pixels.  The original takes one chroma pointer
 * into blk_a and finds the same row of blk_b 0x100 bytes further on. */
void mad_put_row(const MadDecoder *d, const int32_t *luma_row, const int32_t *chroma_a_row,
                 const int32_t *chroma_b_row, uint32_t *dst)
{
    for (int32_t i = 0; i < 8; i++)
    {
        const uint32_t left = d->clamp[(uint8_t)((uint32_t)luma_row[2 * i] >> 16)];
        const uint32_t right = d->clamp[(uint8_t)((uint32_t)luma_row[2 * i + 1] >> 16)];
        const uint32_t a = d->clamp[(uint8_t)((uint32_t)chroma_a_row[i] >> 16)];
        const uint32_t b = d->clamp[(uint8_t)((uint32_t)chroma_b_row[i] >> 16)];
        dst[i] = (left << 24) | (right << 16) | (a << 8) | b;
    }
}

/* ------------------------------------------------------------------ */
/* sub_4f3600: one macroblock                                          */
/* ------------------------------------------------------------------ */

/* A block coded in the stream: coefficients, then a flat fill or the IDCT. */
static void coded_block(MadDecoder *d, int32_t *blk, int32_t stride)
{
    if (mad_decode_block(d) == 1)
        mad_fill_dc(d, blk, stride);
    else
        mad_idct(d, blk, stride);
}

void mad_decode_macroblock(MadDecoder *d, const uint8_t *ref, uint8_t *cur, int32_t width)
{
    const int32_t wdw = width >> 1;  /* words a row */
    uint32_t mask = 0;               /* bit set: block predicted, not coded */
    int32_t odd = 0;                 /* left uninitialised by the original when unused */

    if (d->inter)
    {
        const uint32_t top = d->bits >> 24;
        if ((top & 0xc0) == 0)
        {
            /* "00": intra macroblock in a predicted frame. */
            mad_skip_bits(d, 2);
            mask = 0;
        }
        else
        {
            if (top & 0x80)
            {
                /* "1": every block predicted. */
                mad_skip_bits(d, 1);
                mask = 0x3ff;
            }
            else
            {
                /* "01" + six bits: which blocks are predicted. */
                mask = d->bits >> 24;
                mad_skip_bits(d, 8);
            }
            const int32_t dx = mad_read_small(d);
            const int32_t dy = mad_read_small(d);
            /* (dx sar 1) + dy * words-a-row, in words, wrapping as 32-bit
             * arithmetic does; may be negative. */
            ref += (int32_t)(((uint32_t)(dx >> 1) + (uint32_t)dy * (uint32_t)wdw) * 4u);
            odd = dx & 1;
        }
    }

    /* Luma: four 8x8 blocks into the 16x16 buffer, rows 16 dwords apart. */
    if (mask & 0x01)
        mad_predict_luma(ref, odd, wdw, d->luma, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->luma, 16);
    if (mask & 0x02)
        mad_predict_luma(ref + 0x10, odd, wdw, d->luma + 8, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->luma + 8, 16);
    if (mask & 0x04)
        mad_predict_luma(ref + wdw * 32, odd, wdw, d->luma + 128, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->luma + 128, 16);
    if (mask & 0x08)
        mad_predict_luma(ref + wdw * 32 + 0x10, odd, wdw, d->luma + 136, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->luma + 136, 16);

    /* Chroma: two 8x8 blocks, rows 8 dwords apart. */
    if (mask & 0x10)
        mad_predict_chroma(ref, 1, wdw, d->blk_a, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->blk_a, 8);
    if (mask & 0x20)
        mad_predict_chroma(ref, 0, wdw, d->blk_b, mad_read_small(d) - 0x40);
    else
        coded_block(d, d->blk_b, 8);

    /* Sixteen rows out; each chroma row serves two. */
    for (int32_t row = 0; row < 16; row++)
        mad_put_row(d, d->luma + row * 16, d->blk_a + (row >> 1) * 8, d->blk_b + (row >> 1) * 8,
                    (uint32_t *)(cur + row * wdw * 4));
}
