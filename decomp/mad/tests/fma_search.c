/* Find IDCT rows on which rounding the product separately (what the x87 at
 * 53-bit precision does) and fusing it with the sum (an FMA) give different
 * results, so the differential test can check the decompiled IDCT on exactly
 * those.  Prints "in1 in3 in5 in7" lines. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC optimize("fp-contract=off")

static float f(uint32_t b) { float v; memcpy(&v, &b, 4); return v; }
static uint32_t lo(double v) { uint64_t b; memcpy(&b, &v, 8); return (uint32_t)b; }

int main(int argc, char **argv)
{
    int want = argc > 1 ? atoi(argv[1]) : 40;
    const double c0 = f(0x3fa73d75), c1 = f(0x3f0a8bd4), c2 = f(0x3ec3ef15), m = f(0x59c00000);
    uint64_t s = 88172645463325252ull;
    int found = 0;
    for (uint64_t n = 0; found < want && n < 4000000000ull; n++)
    {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        int32_t i1 = (int32_t)(s & 0xffffffff), i3 = (int32_t)(s >> 32);
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        int32_t i5 = (int32_t)(s & 0xffffffff), i7 = (int32_t)(s >> 32);
        int shift = (int)(n % 7);  /* also smaller magnitudes */
        i1 >>= shift; i3 >>= shift; i5 >>= shift; i7 >>= shift;
        double z = (double)(int32_t)((uint32_t)i5 - (uint32_t)i3);
        double y = (double)(int32_t)((uint32_t)i1 - (uint32_t)i7);
        double yz = y + z;
        double sep_a = (z * c1 + yz * c2) + m;
        double fus_a = fma(yz, c2, z * c1) + m;
        double sep_b = (y * c0 - yz * c2) + m;
        double fus_b = fma(-yz, c2, y * c0) + m;
        if (lo(sep_a) != lo(fus_a) || lo(sep_b) != lo(fus_b))
        {
            printf("%d %d %d %d\n", i1, i3, i5, i7);
            found++;
        }
    }
    return 0;
}
