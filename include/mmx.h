#ifndef MMX_H_
#define MMX_H_

#include <x86.h>
#include <cstring>

/* The generated disassembly emits SSE2 intrinsics (_mm_add_epi16, _mm_madd_epi16,
 * _mm_packus_epi16, ...) unconditionally -- there is no #ifdef around them -- so a
 * translation layer is required on ARM even when WITH_MMX is off and the game's
 * CPUID reports no MMX.  sse2neon covers every one of the 25 intrinsics the
 * nfs3hp disassembly actually uses. */
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
# include <immintrin.h>
#else
# include <sse2neon.h>
#endif

namespace x86
{

struct regmmx
{
    __m128i reg;
    operator x86::reg64() const
    {
        return _mm_cvtsi128_si64(reg);
    }
    operator __m128i() const
    {
        return reg;
    }
};

static inline regmmx from_reg64(const x86::reg64& value)
{
    regmmx result;
    result.reg = _mm_cvtsi64_si128(value);
    return result;
}

struct MMX
{
    regmmx mm0;
    regmmx mm1;
    regmmx mm2;
    regmmx mm3;
    regmmx mm4;
    regmmx mm5;
    regmmx mm6;
    regmmx mm7;
    void init()
    {
        memset(this, 0, sizeof(*this));
    }
};

}

#endif /* !MMX_H_ */
