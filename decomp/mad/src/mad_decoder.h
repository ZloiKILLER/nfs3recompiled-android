/*
 * EA MAD ("Madcow") video decoder of Need for Speed III -- decompiled from
 * nfs3.exe (1998, Watcom C).  Every function names the address of the
 * original it replaces; tools/../tests check them against the original
 * machine code, bit for bit.
 *
 * Picture format the decoder writes (what the player keeps in its frame
 * buffers): packed 4:2:2, two pixels per 32-bit word, little-endian bytes
 *     byte 0 = chroma plane "B" (block 5, 0x9f0f38)
 *     byte 1 = chroma plane "A" (block 4, 0x9f0e38)
 *     byte 2 = luma of the right pixel
 *     byte 3 = luma of the left pixel
 * every component 7 bits wide (0..127).  Chroma is coded 8x8 per 16x16
 * macroblock (4:2:0) and repeated on both rows of each pair when written.
 * Which of A/B is Cb and which Cr is decided by the converter that shows the
 * frame (sub_4fdc90), not by the decoder.
 */
#ifndef MAD_DECODER_H
#define MAD_DECODER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The decoder's state.  In the game all of it is global data; the first
 * block is one contiguous range of the BSS, 0x9f0c38..0x9f254c, in this
 * order, the last two arrays live in DGROUP. */
typedef struct MadDecoder
{
    int32_t  qmat[64];         /* 0x9f0c38  dequantiser, quant * matrix * AAN scale     */
    uint8_t  clamp[256];       /* 0x9f0d38  signed byte -> 7-bit sample (see build)     */
    int32_t  blk_a[64];        /* 0x9f0e38  chroma block A, 8x8 (byte 1 of the output)  */
    int32_t  blk_b[64];        /* 0x9f0f38  chroma block B, 8x8 (byte 0 of the output)  */
    uint32_t vlc_mv[64];       /* 0x9f1038  motion/offset code, 6-bit index             */
    uint32_t vlc_after9[256];  /* 0x9f1138  AC code after nine zero bits, 8-bit index   */
    uint32_t vlc_after6[256];  /* 0x9f1538  AC code after six zero bits, 8-bit index    */
    uint32_t vlc_first9[512];  /* 0x9f1938  AC code, first nine bits                    */
    int32_t  luma[256];        /* 0x9f2138  luma macroblock, 16x16                      */
    int32_t  tables_ready;     /* 0x9f2538 */
    uint32_t bits;             /* 0x9f253c  bit buffer, next bit in bit 31              */
    int32_t  inter;            /* 0x9f2540  0 for a key frame (MADk), 1 otherwise       */
    int32_t  nbits;            /* 0x9f2544  valid bits in `bits`                        */
    const uint8_t *ptr;        /* 0x9f2548  next 16-bit word of the bitstream           */

    int32_t  coef[64];         /* 0x56cf2c  coefficients of the block being decoded     */
    int32_t  idct_tmp[72];     /* 0x56ce0c  IDCT between passes, 8 columns of 9 dwords  */

    /* Not in the original.  Bit 0: a corrupt stream made the original write
     * outside the coefficient block (it would corrupt other data; this
     * version stops the block instead).  Bit 1: a coefficient past the end
     * of the scan, which stayed inside the block and is reproduced. */
    int32_t  overflow;
} MadDecoder;

/* sub_4f30d0: build the code tables and the clamp table (done once). */
void mad_build_tables(MadDecoder *d);

/* sub_4f3560: start a frame.  `data` is the MAD chunk payload (chunk + 0x18),
 * `inter` 0 for a MADk key frame, `quant` the byte at chunk + 0x15. */
void mad_begin_frame(MadDecoder *d, const uint8_t *data, int32_t inter, uint32_t quant);

/* sub_4f3600: decode one 16x16 macroblock into `cur`, predicting from `ref`
 * (both point at the macroblock's top-left word, rows `width` pixels apart,
 * 2 bytes a pixel).  `ref` may be read outside the picture by a motion vector,
 * exactly as the original does. */
void mad_decode_macroblock(MadDecoder *d, const uint8_t *ref, uint8_t *cur, int32_t width);

/* Lower-level pieces, exported for the tests. */
void     mad_skip_bits(MadDecoder *d, int32_t n);                         /* sub_4f3350 */
int32_t  mad_read_small(MadDecoder *d);                                   /* sub_4f33b0 */
void     mad_fill_dc(MadDecoder *d, int32_t *blk, int32_t stride);        /* sub_4f33e0 */
void     mad_predict_luma(const uint8_t *ref, int32_t odd, int32_t wdw,
                          int32_t *blk, int32_t offset);                  /* sub_4f3420 */
void     mad_predict_chroma(const uint8_t *ref, int32_t plane, int32_t wdw,
                            int32_t *blk, int32_t offset);                /* sub_4f34f0 */
int32_t  mad_decode_block(MadDecoder *d);                                 /* sub_510e48 */
void     mad_idct(MadDecoder *d, int32_t *dst, int32_t stride);           /* sub_510cbd */
void     mad_idct_row(const int32_t *in, int32_t *out);                   /* sub_510a50 */
void     mad_idct_col(const int32_t *in, int32_t *out);                   /* sub_510bb3 */
void     mad_put_row(const MadDecoder *d, const int32_t *luma_row,
                     const int32_t *chroma_a_row, const int32_t *chroma_b_row,
                     uint32_t *dst);                                       /* sub_510ff8 */

#ifdef __cplusplus
}
#endif

#endif
