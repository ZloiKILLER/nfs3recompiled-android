/* The x87's extended precision, worked out exactly in portable C++.
 *
 * WITH_PEDANTIC_FPU keeps every value on the x87 stack as the x87 does: 80
 * bits, a 64-bit significand with its integer bit, a 15-bit exponent
 * (include/fpu.h, x86::Float).  The helpers it calls were x87.asm, which runs
 * them on the host's own x87 -- so only on an x86 PC, and always at the host's
 * precision control.  These are the same helpers for any host, an arm64 phone
 * first: the four operations and the square root rounded to the precision the
 * game's control word asks for (24, 53 or 64 bits, FPU::setControl tells
 * control80), in its rounding mode, with the x87's 15-bit exponent and its
 * denormals; conversions, comparisons, frndint, fscale and fprem exact as
 * well.  The rounding follows Berkeley SoftFloat's floatx80 (roundAndPack and
 * the add/sub/mul paths), which is the x87's; the quotient and the root are
 * worked out by long division and an integer square root with the sticky bit
 * carried, so they need no correction step.  tools/x87soft_checks.cpp compares
 * every helper with a real x87, bit for bit.
 *
 * fsin, fcos, fptan, fpatan, fyl2x and f2xm1 are left to the host's long
 * double: an arm64 long double has 113 bits, and its result rounded once to
 * 64 is at most a last bit away from what the x87's own microcode gives --
 * the x87's are not correctly rounded either. */
#include <fpu.h>
#include <cmath>
#include <math.h>
#include <cfloat>
#include <cstdint>
#include <cstring>

namespace x86
{
namespace
{

typedef unsigned __int128 u128;

/* The control word of the x87 this thread runs: FPU::setControl and init. */
thread_local reg16 t_control = 0x037f;

enum Rounding { kNearest = 0, kDown = 1, kUp = 2, kTowardZero = 3 };

inline int roundingMode() { return (t_control >> 10) & 3; }

/* SoftFloat's rounding precisions: 80 (a 64-bit significand), 64 (53 bits),
 * 32 (24 bits).  Precision control 00 is single, 10 double, 11 extended; 01
 * is reserved and works as extended. */
inline int precision()
{
    switch ((t_control >> 8) & 3)
    {
    case 0: return 32;
    case 2: return 64;
    default: return 80;
    }
}

struct X
{
    bool sign;
    int32_t exp;    // the exponent field, 0..0x7fff
    uint64_t sig;   // the significand with its integer bit
};

inline X load(const IEEEf80Data* p)
{
    X x;
    x.sig = uint64_t(p->data1) | uint64_t(p->data2) << 16 | uint64_t(p->data3) << 32 | uint64_t(p->data4) << 48;
    x.exp = p->data5 & 0x7fff;
    x.sign = (p->data5 >> 15) != 0;
    return x;
}

inline void store(IEEEf80Data* p, const X& x)
{
    p->data1 = reg16(x.sig);
    p->data2 = reg16(x.sig >> 16);
    p->data3 = reg16(x.sig >> 32);
    p->data4 = reg16(x.sig >> 48);
    p->data5 = reg16((x.sign ? 0x8000 : 0) | (x.exp & 0x7fff));
}

inline X pack(bool sign, int32_t exp, uint64_t sig) { return X{ sign, exp, sig }; }

const uint64_t kTop = uint64_t(1) << 63;

inline bool isNaN(const X& x) { return x.exp == 0x7fff && (x.sig << 1) != 0; }
inline bool isInf(const X& x) { return x.exp == 0x7fff && (x.sig << 1) == 0; }
inline bool isZero(const X& x) { return x.exp == 0 && x.sig == 0; }

// The x87's "real indefinite", what an invalid operation gives.
inline X indefinite() { return pack(true, 0x7fff, 0xc000000000000000ull); }

inline X quiet(X x)
{
    x.sig |= 0x4000000000000000ull;
    return x;
}

/* Which NaN comes out, by the x87's table: of a quiet and a signalling one,
 * the quiet one; of two of a kind, the larger significand; quiet either way. */
inline X propagateNaN(const X& a, const X& b)
{
    const bool an = isNaN(a), bn = isNaN(b);
    if (an && bn)
    {
        const bool aq = (a.sig & 0x4000000000000000ull) != 0, bq = (b.sig & 0x4000000000000000ull) != 0;
        if (aq != bq)
            return quiet(aq ? a : b);
        if (a.sig != b.sig)
            return quiet(a.sig > b.sig ? a : b);
        return quiet(a.sign ? b : a);
    }
    return quiet(an ? a : b);
}

inline int clz64(uint64_t v) { return v ? __builtin_clzll(v) : 64; }

/* A denormal's significand shifted up to its integer bit, its exponent below 1. */
inline void normaliseSubnormal(uint64_t sig, int32_t& exp, uint64_t& out)
{
    const int shift = clz64(sig);
    out = sig << shift;
    exp = 1 - shift;
}

inline void shift64RightJamming(uint64_t a, int32_t count, uint64_t& z)
{
    if (count == 0)
        z = a;
    else if (count < 64)
        z = (a >> count) | uint64_t((a << ((-count) & 63)) != 0);
    else
        z = uint64_t(a != 0);
}

inline void shift64ExtraRightJamming(uint64_t a0, uint64_t a1, int32_t count, uint64_t& z0, uint64_t& z1)
{
    if (count == 0)
    {
        z1 = a1;
        z0 = a0;
    }
    else if (count < 64)
    {
        z1 = (a0 << ((-count) & 63)) | uint64_t(a1 != 0);
        z0 = a0 >> count;
    }
    else
    {
        if (count == 64)
            z1 = a0 | uint64_t(a1 != 0);
        else
            z1 = uint64_t((a0 | a1) != 0);
        z0 = 0;
    }
}

inline void shift128RightJamming(uint64_t a0, uint64_t a1, int32_t count, uint64_t& z0, uint64_t& z1)
{
    if (count == 0)
    {
        z1 = a1;
        z0 = a0;
    }
    else if (count < 64)
    {
        const int neg = (-count) & 63;
        z1 = (a0 << neg) | (a1 >> count) | uint64_t((a1 << neg) != 0);
        z0 = a0 >> count;
    }
    else
    {
        if (count == 64)
            z1 = a0 | uint64_t(a1 != 0);
        else if (count < 128)
            z1 = (a0 >> (count & 63)) | uint64_t(((a0 << ((-count) & 63)) | a1) != 0);
        else
            z1 = uint64_t((a0 | a1) != 0);
        z0 = 0;
    }
}

/* SoftFloat's roundAndPackFloatx80: the significand zSig0:zSig1 (zSig0's top
 * bit the integer bit, unless the exponent is that of a denormal) rounded to
 * the precision and in the mode asked for, past the largest exponent to
 * infinity or the largest number, below the smallest to a denormal. */
X roundAndPack(int roundingPrecision, bool zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1, int mode)
{
    const bool nearestEven = mode == kNearest;
    uint64_t roundIncrement, roundMask, roundBits;
    bool increment;
    if (roundingPrecision == 64)
    {
        roundIncrement = 0x0000000000000400ull;
        roundMask = 0x00000000000007ffull;
    }
    else if (roundingPrecision == 32)
    {
        roundIncrement = 0x0000008000000000ull;
        roundMask = 0x000000ffffffffffull;
    }
    else
    {
        goto precision80;
    }
    zSig0 |= uint64_t(zSig1 != 0);
    if (!nearestEven)
    {
        if (mode == kTowardZero)
            roundIncrement = 0;
        else
        {
            roundIncrement = roundMask;
            if (zSign)
            {
                if (mode == kUp)
                    roundIncrement = 0;
            }
            else
            {
                if (mode == kDown)
                    roundIncrement = 0;
            }
        }
    }
    roundBits = zSig0 & roundMask;
    if (0x7ffd <= uint32_t(zExp - 1))
    {
        if (0x7ffe < zExp || (zExp == 0x7ffe && zSig0 + roundIncrement < zSig0))
            goto overflow;
        if (zExp <= 0)
        {
            shift64RightJamming(zSig0, 1 - zExp, zSig0);
            zExp = 0;
            roundBits = zSig0 & roundMask;
            zSig0 += roundIncrement;
            if (int64_t(zSig0) < 0)
                zExp = 1;
            roundIncrement = roundMask + 1;
            if (nearestEven && (roundBits << 1) == roundIncrement)
                roundMask |= roundIncrement;
            zSig0 &= ~roundMask;
            return pack(zSign, zExp, zSig0);
        }
    }
    zSig0 += roundIncrement;
    if (zSig0 < roundIncrement)
    {
        ++zExp;
        zSig0 = kTop;
    }
    roundIncrement = roundMask + 1;
    if (nearestEven && (roundBits << 1) == roundIncrement)
        roundMask |= roundIncrement;
    zSig0 &= ~roundMask;
    if (zSig0 == 0)
        zExp = 0;
    return pack(zSign, zExp, zSig0);

precision80:
    increment = int64_t(zSig1) < 0;
    if (!nearestEven)
    {
        if (mode == kTowardZero)
            increment = false;
        else if (zSign)
            increment = mode == kDown && zSig1;
        else
            increment = mode == kUp && zSig1;
    }
    if (0x7ffd <= uint32_t(zExp - 1))
    {
        if (0x7ffe < zExp || (zExp == 0x7ffe && zSig0 == ~uint64_t(0) && increment))
        {
            roundMask = 0;
        overflow:
            if (mode == kTowardZero || (zSign && mode == kUp) || (!zSign && mode == kDown))
                return pack(zSign, 0x7ffe, ~roundMask);
            return pack(zSign, 0x7fff, kTop);
        }
        if (zExp <= 0)
        {
            shift64ExtraRightJamming(zSig0, zSig1, 1 - zExp, zSig0, zSig1);
            zExp = 0;
            if (nearestEven)
                increment = int64_t(zSig1) < 0;
            else if (mode == kTowardZero)
                increment = false;
            else if (zSign)
                increment = mode == kDown && zSig1;
            else
                increment = mode == kUp && zSig1;
            if (increment)
            {
                ++zSig0;
                zSig0 &= ~uint64_t((zSig1 << 1) == 0 && nearestEven);
                if (int64_t(zSig0) < 0)
                    zExp = 1;
            }
            return pack(zSign, zExp, zSig0);
        }
    }
    if (increment)
    {
        ++zSig0;
        if (zSig0 == 0)
        {
            ++zExp;
            zSig0 = kTop;
        }
        else
        {
            zSig0 &= ~uint64_t((zSig1 << 1) == 0 && nearestEven);
        }
    }
    else if (zSig0 == 0)
    {
        zExp = 0;
    }
    return pack(zSign, zExp, zSig0);
}

X normaliseRoundAndPack(int roundingPrecision, bool zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1, int mode)
{
    if (zSig0 == 0)
    {
        if (zSig1 == 0)
            return pack(zSign, 0, 0);
        zSig0 = zSig1;
        zSig1 = 0;
        zExp -= 64;
    }
    const int shift = clz64(zSig0);
    if (shift)
    {
        zSig0 = (zSig0 << shift) | (zSig1 >> (64 - shift));
        zSig1 <<= shift;
    }
    zExp -= shift;
    return roundAndPack(roundingPrecision, zSign, zExp, zSig0, zSig1, mode);
}

/* Magnitudes of a and b added, both of the sign zSign. */
X addSigs(X a, X b, bool zSign, int prec, int mode)
{
    int32_t aExp = a.exp, bExp = b.exp, zExp;
    uint64_t aSig = a.sig, bSig = b.sig, zSig0, zSig1;
    int32_t expDiff = aExp - bExp;
    if (expDiff > 0)
    {
        if (aExp == 0x7fff)
        {
            if (aSig << 1)
                return propagateNaN(a, b);
            return a;
        }
        if (bExp == 0)
            --expDiff;
        shift64ExtraRightJamming(bSig, 0, expDiff, bSig, zSig1);
        zExp = aExp;
    }
    else if (expDiff < 0)
    {
        if (bExp == 0x7fff)
        {
            if (bSig << 1)
                return propagateNaN(a, b);
            return pack(zSign, 0x7fff, kTop);
        }
        if (aExp == 0)
            ++expDiff;
        shift64ExtraRightJamming(aSig, 0, -expDiff, aSig, zSig1);
        zExp = bExp;
    }
    else
    {
        if (aExp == 0x7fff)
        {
            if ((aSig | bSig) << 1)
                return propagateNaN(a, b);
            return a;
        }
        zSig1 = 0;
        zSig0 = aSig + bSig;
        if (aExp == 0)
        {
            if (zSig0 == 0 && aSig == 0)
                return pack(zSign, 0, 0);
            if (zSig0 < aSig)  // two pseudo-denormals carried out
            {
                zExp = 1;
                goto shiftRight1;
            }
            normaliseSubnormal(zSig0, zExp, zSig0);
            goto roundAndPackIt;
        }
        zExp = aExp;
        goto shiftRight1;
    }
    zSig0 = aSig + bSig;
    if (int64_t(zSig0) < 0)
        goto roundAndPackIt;
shiftRight1:
    shift64ExtraRightJamming(zSig0, zSig1, 1, zSig0, zSig1);
    zSig0 |= kTop;
    ++zExp;
roundAndPackIt:
    return roundAndPack(prec, zSign, zExp, zSig0, zSig1, mode);
}

/* Magnitude of b taken from that of a, the result of the sign zSign. */
X subSigs(X a, X b, bool zSign, int prec, int mode)
{
    int32_t aExp = a.exp, bExp = b.exp, zExp;
    uint64_t aSig = a.sig, bSig = b.sig, zSig0, zSig1;
    int32_t expDiff = aExp - bExp;
    if (expDiff > 0)
        goto aExpBigger;
    if (expDiff < 0)
        goto bExpBigger;
    if (aExp == 0x7fff)
    {
        if ((aSig | bSig) << 1)
            return propagateNaN(a, b);
        return indefinite();
    }
    if (aExp == 0)
    {
        aExp = 1;
        bExp = 1;
    }
    zSig1 = 0;
    if (bSig < aSig)
        goto aBigger;
    if (aSig < bSig)
        goto bBigger;
    return pack(mode == kDown, 0, 0);
bExpBigger:
    if (bExp == 0x7fff)
    {
        if (bSig << 1)
            return propagateNaN(a, b);
        return pack(!zSign, 0x7fff, kTop);
    }
    if (aExp == 0)
        ++expDiff;
    shift128RightJamming(aSig, 0, -expDiff, aSig, zSig1);
bBigger:
    {
        const u128 r = ((u128(bSig) << 64)) - ((u128(aSig) << 64) | zSig1);
        zSig0 = uint64_t(r >> 64);
        zSig1 = uint64_t(r);
    }
    zExp = bExp;
    zSign = !zSign;
    return normaliseRoundAndPack(prec, zSign, zExp, zSig0, zSig1, mode);
aExpBigger:
    if (aExp == 0x7fff)
    {
        if (aSig << 1)
            return propagateNaN(a, b);
        return a;
    }
    if (bExp == 0)
        --expDiff;
    shift128RightJamming(bSig, 0, expDiff, bSig, zSig1);
aBigger:
    {
        const u128 r = ((u128(aSig) << 64)) - ((u128(bSig) << 64) | zSig1);
        zSig0 = uint64_t(r >> 64);
        zSig1 = uint64_t(r);
    }
    zExp = aExp;
    return normaliseRoundAndPack(prec, zSign, zExp, zSig0, zSig1, mode);
}

X add(X a, X b, int prec, int mode)
{
    if (a.sign == b.sign)
        return addSigs(a, b, a.sign, prec, mode);
    return subSigs(a, b, a.sign, prec, mode);
}

X sub(X a, X b, int prec, int mode)
{
    if (a.sign == b.sign)
        return subSigs(a, b, a.sign, prec, mode);
    return addSigs(a, b, a.sign, prec, mode);
}

X mul(X a, X b, int prec, int mode)
{
    const bool zSign = a.sign != b.sign;
    int32_t aExp = a.exp, bExp = b.exp;
    uint64_t aSig = a.sig, bSig = b.sig;
    if (aExp == 0x7fff)
    {
        if ((aSig << 1) || (bExp == 0x7fff && (bSig << 1)))
            return propagateNaN(a, b);
        if (bExp == 0 && bSig == 0)
            return indefinite();
        return pack(zSign, 0x7fff, kTop);
    }
    if (bExp == 0x7fff)
    {
        if (bSig << 1)
            return propagateNaN(a, b);
        if (aExp == 0 && aSig == 0)
            return indefinite();
        return pack(zSign, 0x7fff, kTop);
    }
    if (aExp == 0)
    {
        if (aSig == 0)
            return pack(zSign, 0, 0);
        normaliseSubnormal(aSig, aExp, aSig);
    }
    if (bExp == 0)
    {
        if (bSig == 0)
            return pack(zSign, 0, 0);
        normaliseSubnormal(bSig, bExp, bSig);
    }
    int32_t zExp = aExp + bExp - 0x3ffe;
    const u128 p = u128(aSig) * bSig;
    uint64_t zSig0 = uint64_t(p >> 64), zSig1 = uint64_t(p);
    if (int64_t(zSig0) >= 0)
    {
        zSig0 = (zSig0 << 1) | (zSig1 >> 63);
        zSig1 <<= 1;
        --zExp;
    }
    return roundAndPack(prec, zSign, zExp, zSig0, zSig1, mode);
}

X div(X a, X b, int prec, int mode)
{
    const bool zSign = a.sign != b.sign;
    int32_t aExp = a.exp, bExp = b.exp;
    uint64_t aSig = a.sig, bSig = b.sig;
    if (aExp == 0x7fff)
    {
        if (aSig << 1)
            return propagateNaN(a, b);
        if (bExp == 0x7fff)
        {
            if (bSig << 1)
                return propagateNaN(a, b);
            return indefinite();
        }
        return pack(zSign, 0x7fff, kTop);
    }
    if (bExp == 0x7fff)
    {
        if (bSig << 1)
            return propagateNaN(a, b);
        return pack(zSign, 0, 0);
    }
    if (bExp == 0)
    {
        if (bSig == 0)
        {
            if (aExp == 0 && aSig == 0)
                return indefinite();
            return pack(zSign, 0x7fff, kTop);  // divide by zero
        }
        normaliseSubnormal(bSig, bExp, bSig);
    }
    if (aExp == 0)
    {
        if (aSig == 0)
            return pack(zSign, 0, 0);
        normaliseSubnormal(aSig, aExp, aSig);
    }
    /* The quotient by long division: 64 bits, then 64 more, and whatever
     * remains as the sticky bit. */
    int32_t zExp;
    u128 n;
    if (aSig >= bSig)
    {
        n = u128(aSig) << 63;
        zExp = aExp - bExp + 0x3fff;
    }
    else
    {
        n = u128(aSig) << 64;
        zExp = aExp - bExp + 0x3ffe;
    }
    const uint64_t q1 = uint64_t(n / bSig);
    const uint64_t r1 = uint64_t(n % bSig);
    const u128 n2 = u128(r1) << 64;
    const uint64_t q2 = uint64_t(n2 / bSig);
    const uint64_t r2 = uint64_t(n2 % bSig);
    return roundAndPack(prec, zSign, zExp, q1, q2 | uint64_t(r2 != 0), mode);
}

/* floor(sqrt(m)) for m < 2^128. */
uint64_t isqrt128(u128 m)
{
    const double estimate = std::sqrt(double(m));
    uint64_t r = estimate >= 18446744073709549568.0 ? ~uint64_t(0) : uint64_t(estimate);
    if (r == 0)
        r = 1;
    for (int i = 0; i < 3; ++i)
    {
        const u128 next = (u128(r) + m / r) >> 1;
        r = next > ~uint64_t(0) ? ~uint64_t(0) : uint64_t(next);
    }
    while (u128(r) * r > m)
        --r;
    while (r != ~uint64_t(0) && u128(r + 1) * (r + 1) <= m)
        ++r;
    return r;
}

X sqrt(X a, int prec, int mode)
{
    int32_t aExp = a.exp;
    uint64_t aSig = a.sig;
    if (aExp == 0x7fff)
    {
        if (aSig << 1)
            return propagateNaN(a, a);
        if (!a.sign)
            return a;
        return indefinite();
    }
    if (aExp == 0 && aSig == 0)
        return a;
    if (a.sign)
        return indefinite();
    if (aExp == 0)
        normaliseSubnormal(aSig, aExp, aSig);
    /* a = aSig * 2^(e - 63) with e unbiased; the root has 64 bits taken from
     * aSig shifted up so that its integer square root is 2^63 or more. */
    const int32_t e = aExp - 0x3fff;
    const u128 m = (e & 1) ? (u128(aSig) << 64) : (u128(aSig) << 63);
    const int32_t zExp = (e >> 1) + 0x3fff;
    const uint64_t r = isqrt128(m);
    const u128 rem = m - u128(r) * r;
    /* No root lies half way between two of them: past r + 1/2 exactly when
     * the remainder is more than r. */
    uint64_t zSig1 = 0;
    if (rem != 0)
        zSig1 = rem > r ? 0xc000000000000000ull : 0x4000000000000000ull;
    return roundAndPack(prec, false, zExp, r, zSig1, mode);
}

/* The value as a significand with its integer bit and an unbiased exponent,
 * denormals normalised; false for a zero. */
inline bool normalised(const X& x, int32_t& e, uint64_t& sig)
{
    if (x.sig == 0)
        return false;
    int32_t exp = x.exp;
    sig = x.sig;
    if (exp == 0)
        normaliseSubnormal(sig, exp, sig);
    else if (!(sig & kTop))
    {
        // an unnormal: the x87 refuses it, but give it its value
        const int shift = clz64(sig);
        sig <<= shift;
        exp -= shift;
    }
    e = exp - 0x3fff;
    return true;
}

/* Magnitudes compared: -1, 0, 1. */
int compareMagnitude(const X& a, const X& b)
{
    int32_t ae = 0, be = 0;
    uint64_t as = 0, bs = 0;
    const bool an = normalised(a, ae, as), bn = normalised(b, be, bs);
    if (!an || !bn)
        return an ? 1 : bn ? -1 : 0;
    if (a.exp == 0x7fff || b.exp == 0x7fff)
    {
        // infinities
        if (a.exp == b.exp)
            return 0;
        return a.exp == 0x7fff ? 1 : -1;
    }
    if (ae != be)
        return ae > be ? 1 : -1;
    if (as != bs)
        return as > bs ? 1 : -1;
    return 0;
}

/* The x87 integer of a value, rounded in the mode, to `bits` bits; false
 * where it does not fit (or is no number at all). */
bool toInt(const X& x, int bits, int mode, int64_t& out)
{
    if (x.exp == 0x7fff)
        return false;
    int32_t e;
    uint64_t sig;
    if (!normalised(x, e, sig))
    {
        out = 0;
        return true;
    }
    uint64_t mag;  // the magnitude rounded
    if (e >= 64)
        return false;
    if (e == 63)
    {
        mag = sig;
    }
    else if (e < 0)
    {
        // |x| < 1
        bool up;
        switch (mode)
        {
        case kNearest: up = e == -1 && sig > kTop; break;  // more than one half
        case kDown: up = x.sign; break;
        case kUp: up = !x.sign; break;
        default: up = false; break;
        }
        mag = up ? 1 : 0;
    }
    else
    {
        const int fraction = 63 - e;
        mag = sig >> fraction;
        const uint64_t rest = sig & ((uint64_t(1) << fraction) - 1);
        const uint64_t half = uint64_t(1) << (fraction - 1);
        bool up;
        switch (mode)
        {
        case kNearest: up = rest > half || (rest == half && (mag & 1)); break;
        case kDown: up = x.sign && rest; break;
        case kUp: up = !x.sign && rest; break;
        default: up = false; break;
        }
        if (up)
            ++mag;
    }
    const uint64_t limit = uint64_t(1) << (bits - 1);
    if (x.sign)
    {
        if (mag > limit)
            return false;
        out = int64_t(0 - mag);
    }
    else
    {
        if (mag >= limit)
            return false;
        out = int64_t(mag);
    }
    return true;
}

/* An integer as the x87 holds it: exact. */
X fromInt(int64_t v)
{
    if (v == 0)
        return pack(false, 0, 0);
    const bool sign = v < 0;
    const uint64_t mag = sign ? 0 - uint64_t(v) : uint64_t(v);
    const int shift = clz64(mag);
    return pack(sign, 0x3fff + 63 - shift, mag << shift);
}

/* The value rounded to an IEEE format of `bits` significand bits (with the
 * hidden one) and `exponentBits` of exponent, in the mode: the bits of it. */
uint64_t toIEEE(const X& x, int bits, int exponentBits, int mode)
{
    const uint64_t signBit = uint64_t(x.sign) << (bits - 1 + exponentBits);
    const int32_t bias = (1 << (exponentBits - 1)) - 1;
    const uint64_t maxExp = (uint64_t(1) << exponentBits) - 1;
    const int fractionBits = bits - 1;
    if (x.exp == 0x7fff)
    {
        if ((x.sig << 1) == 0)
            return signBit | (maxExp << fractionBits);
        // a NaN keeps the top of its payload, quiet
        const uint64_t payload = (x.sig << 1) >> (64 - fractionBits);
        return signBit | (maxExp << fractionBits) | payload | (uint64_t(1) << (fractionBits - 1));
    }
    int32_t e;
    uint64_t sig;
    if (!normalised(x, e, sig))
        return signBit;
    int32_t biased = e + bias;
    // keep `bits` bits of sig (below its integer bit for a normal result)
    int drop = 64 - bits;
    if (biased <= 0)
        drop += 1 - biased;  // a denormal keeps fewer
    uint64_t kept, rest, half;
    bool sticky;
    if (drop >= 64)
    {
        kept = 0;
        rest = drop == 64 ? sig : 0;
        sticky = drop == 64 ? false : sig != 0;
        half = drop == 64 ? kTop : 0;
        if (drop > 64)
        {
            // far below: only the sticky matters
            rest = 0;
        }
    }
    else
    {
        kept = sig >> drop;
        rest = sig & ((uint64_t(1) << drop) - 1);
        half = uint64_t(1) << (drop - 1);
        sticky = false;
    }
    bool up;
    const bool inexact = rest != 0 || sticky;
    switch (mode)
    {
    case kNearest:
        up = drop <= 64 && (rest > half || (rest == half && (sticky || (kept & 1))));
        break;
    case kDown: up = x.sign && inexact; break;
    case kUp: up = !x.sign && inexact; break;
    default: up = false; break;
    }
    if (up)
        ++kept;
    if (biased <= 0)
    {
        // denormal, or rounded up to the smallest normal: the hidden bit
        // lands in the exponent field by itself
        return signBit | kept;
    }
    if (kept >> bits)
    {
        kept >>= 1;
        ++biased;
    }
    if (uint64_t(biased) >= maxExp)
    {
        const bool toMax = mode == kTowardZero || (x.sign && mode == kUp) || (!x.sign && mode == kDown);
        if (toMax)
            return signBit | ((maxExp - 1) << fractionBits) | ((uint64_t(1) << fractionBits) - 1);
        return signBit | (maxExp << fractionBits);
    }
    return signBit | (uint64_t(biased) << fractionBits) | (kept & ((uint64_t(1) << fractionBits) - 1));
}

X fromIEEE(uint64_t bitsValue, int bits, int exponentBits)
{
    const int fractionBits = bits - 1;
    const bool sign = (bitsValue >> (fractionBits + exponentBits)) & 1;
    const uint64_t maxExp = (uint64_t(1) << exponentBits) - 1;
    const int32_t bias = (1 << (exponentBits - 1)) - 1;
    const uint64_t field = (bitsValue >> fractionBits) & maxExp;
    const uint64_t fraction = bitsValue & ((uint64_t(1) << fractionBits) - 1);
    if (field == maxExp)
    {
        if (fraction == 0)
            return pack(sign, 0x7fff, kTop);
        // the x87 loads a signalling NaN quiet
        return pack(sign, 0x7fff, kTop | 0x4000000000000000ull | (fraction << (63 - fractionBits)));
    }
    if (field == 0)
    {
        if (fraction == 0)
            return pack(sign, 0, 0);
        const int shift = clz64(fraction);
        // value = fraction * 2^(1 - bias - fractionBits)
        return pack(sign, 0x3fff + (1 - bias - fractionBits) + 63 - shift, fraction << shift);
    }
    return pack(sign, int32_t(field) - bias + 0x3fff, kTop | (fraction << (63 - fractionBits)));
}

/* frndint: to an integer in the mode, the exponent kept. */
X roundToInteger(const X& x, int mode)
{
    if (x.exp == 0x7fff)
        return isNaN(x) ? quiet(x) : x;
    int32_t e;
    uint64_t sig;
    if (!normalised(x, e, sig))
        return x;
    if (e >= 63)
        return x;
    if (e < 0)
    {
        bool one;
        switch (mode)
        {
        case kNearest: one = e == -1 && sig > kTop; break;
        case kDown: one = x.sign; break;
        case kUp: one = !x.sign; break;
        default: one = false; break;
        }
        return one ? pack(x.sign, 0x3fff, kTop) : pack(x.sign, 0, 0);
    }
    const int fraction = 63 - e;
    const uint64_t mask = (uint64_t(1) << fraction) - 1;
    const uint64_t rest = sig & mask;
    uint64_t whole = sig & ~mask;
    const uint64_t half = uint64_t(1) << (fraction - 1);
    bool up;
    switch (mode)
    {
    case kNearest: up = rest > half || (rest == half && ((whole >> fraction) & 1)); break;
    case kDown: up = x.sign && rest; break;
    case kUp: up = !x.sign && rest; break;
    default: up = false; break;
    }
    int32_t exp = e + 0x3fff;
    if (up)
    {
        whole += uint64_t(1) << fraction;
        if (whole == 0)
        {
            whole = kTop;
            ++exp;
        }
    }
    return pack(x.sign, exp, whole);
}

/* fscale: x times 2 to the power of y truncated. */
X scale(const X& x, const X& y)
{
    if (isNaN(x) || isNaN(y))
        return propagateNaN(x, y);
    if (isInf(y))
    {
        if (y.sign)
        {
            if (isInf(x))
                return indefinite();
            return pack(x.sign, 0, 0);
        }
        if (isZero(x))
            return indefinite();
        return pack(x.sign, 0x7fff, kTop);
    }
    if (isInf(x) || isZero(x))
        return x;
    int64_t n;
    if (!toInt(y, 64, kTowardZero, n))
        n = y.sign ? INT64_MIN : INT64_MAX;
    if (n > 100000)
        n = 100000;
    if (n < -100000)
        n = -100000;
    int32_t e = 0;
    uint64_t sig = 0;
    normalised(x, e, sig);
    const int64_t exp = int64_t(e) + 0x3fff + n;
    return roundAndPack(80, x.sign, int32_t(exp), sig, 0, roundingMode());
}

/* fprem: x less y times the quotient truncated, exact.  Within 64 binary
 * places of each other the x87 finishes in one step, C2 clear and the
 * quotient's three lowest bits in C0, C3, C1.  Further apart it takes a
 * partial step and sets C2 for the caller to go round again: it reduces by
 * y * 2^(D - N) with N = 32 + D mod 32 (D the difference of the exponents),
 * which is what an Intel x87 does (found by experiment; the manual says only
 * "between 32 and 63"), and leaves C0, C3, C1 clear. */
X remainder(const X& x, const X& y, reg32& status)
{
    status = 0;
    if (isNaN(x) || isNaN(y))
        return propagateNaN(x, y);
    if (isInf(x) || isZero(y))
        return indefinite();
    if (isInf(y) || isZero(x))
        return x;
    int32_t xe = 0, ye = 0;
    uint64_t xs = 0, ys = 0;
    normalised(x, xe, xs);
    normalised(y, ye, ys);
    const int32_t d = xe - ye;
    if (d < 0)
        return x;
    uint64_t rem;
    int32_t scale = ye;  // the remainder counts in units of 2^(scale - 63)
    if (d < 64)
    {
        const u128 n = u128(xs) << d;
        const uint64_t q = uint64_t(n / ys);
        rem = uint64_t(n % ys);
        status = ((q & 4) ? 0x0100 : 0) | ((q & 2) ? 0x4000 : 0) | ((q & 1) ? 0x0200 : 0);
    }
    else
    {
        const int32_t n = 32 + (d & 31);
        rem = uint64_t((u128(xs) << n) % ys);
        scale = ye + (d - n);
        status = 0x0400;
    }
    if (rem == 0)
        return pack(x.sign, 0, 0);
    const int shift = clz64(rem);
    const int64_t exp = int64_t(scale) - shift + 0x3fff;
    return roundAndPack(80, x.sign, int32_t(exp), rem << shift, 0, kNearest);
}

/* --- the transcendentals ------------------------------------------------- */

#if (defined(__x86_64__) || defined(__i386__)) && !defined(X87SOFT_QUAD)
/* An x86 host has the x87 itself: the same microcode, the same last bit. */
inline long double toLong(const X& x)
{
    long double v = 0;
    IEEEf80Data d;
    store(&d, x);
    std::memcpy(&v, &d, 10);
    return v;
}

inline X fromLong(long double v)
{
    IEEEf80Data d;
    std::memcpy(&d, &v, 10);
    return load(&d);
}

inline X tSin(const X& x) { long double v = toLong(x); asm("fsin" : "+t"(v)); return fromLong(v); }
inline X tCos(const X& x) { long double v = toLong(x); asm("fcos" : "+t"(v)); return fromLong(v); }
inline X tTan(const X& x) { long double v = toLong(x); asm("fptan\n\tfstp %%st(0)" : "+t"(v)); return fromLong(v); }
inline X tF2xm1(const X& x) { long double v = toLong(x); asm("f2xm1" : "+t"(v)); return fromLong(v); }

inline X tAtan2(const X& y, const X& x)
{
    long double r;
    asm("fpatan" : "=t"(r) : "0"(toLong(x)), "u"(toLong(y)) : "st(1)");
    return fromLong(r);
}

inline X tLog2(const X& x)
{
    long double r;
    asm("fyl2x" : "=t"(r) : "0"(toLong(x)), "u"(1.0L) : "st(1)");
    return fromLong(r);
}
#else
#if defined(X87SOFT_QUAD)
/* Only for checking the arm64 path on an x86 PC: GCC's __float128. */
}  // namespace
}  // namespace x86
#include <quadmath.h>
namespace x86
{
namespace
{
typedef __float128 Quad;
inline Quad qsin(Quad v) { return sinq(v); }
inline Quad qcos(Quad v) { return cosq(v); }
inline Quad qatan2(Quad y, Quad x) { return atan2q(y, x); }
inline Quad qlog2(Quad v) { return log2q(v); }
inline Quad qexpm1(Quad v) { return expm1q(v); }
inline Quad qfma(Quad a, Quad b, Quad c) { return fmaq(a, b, c); }
inline Quad qround(Quad v) { return roundq(v); }
const Quad kLn2 = 0.693147180559945309417232121458176568Q;
#define X87SOFT_HAVE_QUAD 1
#elif LDBL_MANT_DIG == 113
typedef long double Quad;
inline Quad qsin(Quad v) { return sinl(v); }
inline Quad qcos(Quad v) { return cosl(v); }
inline Quad qatan2(Quad y, Quad x) { return atan2l(y, x); }
inline Quad qlog2(Quad v) { return log2l(v); }
inline Quad qexpm1(Quad v) { return expm1l(v); }
inline Quad qfma(Quad a, Quad b, Quad c) { return fmal(a, b, c); }
inline Quad qround(Quad v) { return roundl(v); }
const Quad kLn2 = 0.693147180559945309417232121458176568L;
#define X87SOFT_HAVE_QUAD 1
#endif

#ifdef X87SOFT_HAVE_QUAD
/* IEEE quad (an arm64 long double): the same 15-bit exponent and bias, 112
 * bits of fraction -- every x87 value fits it exactly, and a result comes back
 * rounded once from 113 bits to 64. */
inline Quad toQuad(const X& x)
{
    u128 bits = u128(x.sign) << 127;
    if (x.exp == 0x7fff)
    {
        bits |= u128(0x7fff) << 112 | (u128(x.sig << 1) << 48);
    }
    else if (x.sig != 0)
    {
        if (x.exp == 0 && !(x.sig & kTop))
            bits |= u128(x.sig) << 49;  // an x87 denormal is a quad denormal: sig * 2^(-16382 - 63)
        else
            bits |= u128(uint32_t(x.exp ? x.exp : 1)) << 112 | (u128(x.sig << 1) << 48);
    }
    Quad v;
    std::memcpy(&v, &bits, sizeof v);
    return v;
}

inline X fromQuad(Quad v)
{
    u128 bits;
    std::memcpy(&bits, &v, sizeof bits);
    const bool sign = (bits >> 127) != 0;
    const int32_t exp = int32_t((bits >> 112) & 0x7fff);
    const u128 fraction = bits & ((u128(1) << 112) - 1);
    if (exp == 0x7fff)
    {
        if (fraction == 0)
            return pack(sign, 0x7fff, kTop);
        return pack(sign, 0x7fff, kTop | 0x4000000000000000ull | uint64_t(fraction >> 49));
    }
    if (exp == 0 && fraction == 0)
        return pack(sign, 0, 0);
    const u128 sig = exp ? (fraction | (u128(1) << 112)) : fraction;
    const uint64_t hi = uint64_t(sig >> 49);
    const uint64_t lo = uint64_t(sig << 15);  // the 49 bits below, at the top
    if (exp == 0)
        return normaliseRoundAndPack(80, sign, 1, hi, lo, kNearest);
    return roundAndPack(80, sign, exp, hi, lo, kNearest);
}

/* fsin, fcos and fptan reduce their argument by multiples of pi/2 the way the
 * x87 does: with pi to 66 bits, 0xc90fdaa22168c234c * 2^-66, not the true
 * pi -- most of how far the x87 lands from the correct result near a multiple
 * of pi/2 (Bruce Dawson, "Intel Underestimates Error Bounds by 1.3
 * quintillion", 2014).  So reduced, then worked out in quad, 99.4% of sines
 * and cosines come out as an x87's to the bit and the rest a last bit apart;
 * of tangents 94% and the rest a last bit apart (tools/x87soft_checks.cpp). */
inline Quad reduce(const X& x, int& quadrant)
{
    const Quad halfPi = (toQuad(pack(false, 0x4000, 0xc90fdaa22168c234ull))
                         + toQuad(pack(false, 0x3fc0, 0xc000000000000000ull))) / 2;
    const Quad v = toQuad(x);
    const Quad k = qround(v / halfPi);
    quadrant = int(int64_t(k) & 3);
    return qfma(-k, halfPi, v);
}

inline X tSin(const X& x)
{
    int q;
    const Quad r = reduce(x, q);
    const Quad v = (q & 1) ? qcos(r) : qsin(r);
    return fromQuad(q & 2 ? -v : v);
}

inline X tCos(const X& x)
{
    int q;
    const Quad r = reduce(x, q);
    const Quad v = (q & 1) ? qsin(r) : qcos(r);
    return fromQuad((q == 1 || q == 2) ? -v : v);
}

inline X tTan(const X& x)
{
    int q;
    const Quad r = reduce(x, q);
    return fromQuad((q & 1) ? -qcos(r) / qsin(r) : qsin(r) / qcos(r));
}
inline X tAtan2(const X& y, const X& x) { return fromQuad(qatan2(toQuad(y), toQuad(x))); }
inline X tLog2(const X& x) { return fromQuad(qlog2(toQuad(x))); }
// 2^x - 1 as expm1(x ln 2), so that a small x keeps its digits
inline X tF2xm1(const X& x)
{
    return fromQuad(qexpm1(toQuad(x) * kLn2));
}
#else
/* long double is only a double here (MSVC): 53 bits for the transcendentals. */
inline double toDouble(const X& x)
{
    const uint64_t bits = toIEEE(x, 53, 11, kNearest);
    double d;
    std::memcpy(&d, &bits, 8);
    return d;
}

inline X fromDouble(double d)
{
    uint64_t bits;
    std::memcpy(&bits, &d, 8);
    return fromIEEE(bits, 53, 11);
}

inline X tSin(const X& x) { return fromDouble(std::sin(toDouble(x))); }
inline X tCos(const X& x) { return fromDouble(std::cos(toDouble(x))); }
inline X tTan(const X& x) { return fromDouble(std::tan(toDouble(x))); }
inline X tAtan2(const X& y, const X& x) { return fromDouble(std::atan2(toDouble(y), toDouble(x))); }
inline X tLog2(const X& x) { return fromDouble(std::log2(toDouble(x))); }
inline X tF2xm1(const X& x) { return fromDouble(std::expm1(toDouble(x) * 0.69314718055994530942)); }
#endif
#endif

}  // namespace

extern "C" {

void control80(reg16 word)
{
    t_control = word;
}

void convert80x64(const IEEEf80Data* fp80, double* fp64)
{
    const uint64_t bits = toIEEE(load(fp80), 53, 11, roundingMode());
    std::memcpy(fp64, &bits, 8);
}

void convert80x32(const IEEEf80Data* fp80, float* fp32)
{
    const uint32_t bits = uint32_t(toIEEE(load(fp80), 24, 8, roundingMode()));
    std::memcpy(fp32, &bits, 4);
}

void convert64x80(const double* fp64, IEEEf80Data* fp80)
{
    uint64_t bits;
    std::memcpy(&bits, fp64, 8);
    store(fp80, fromIEEE(bits, 53, 11));
}

void convert32x80(const float* fp32, IEEEf80Data* fp80)
{
    uint32_t bits;
    std::memcpy(&bits, fp32, 4);
    store(fp80, fromIEEE(bits, 24, 8));
}

void converti16x80(const sreg16* int16, IEEEf80Data* fp80) { store(fp80, fromInt(*int16)); }
void converti32x80(const sreg32* int32, IEEEf80Data* fp80) { store(fp80, fromInt(*int32)); }
void converti64x80(const sreg64* int64, IEEEf80Data* fp80) { store(fp80, fromInt(*int64)); }

// fistp: the integer indefinite, the lowest one, where the value does not fit
void convert80xi16(const IEEEf80Data* fp80, sreg16* int16)
{
    int64_t v;
    *int16 = toInt(load(fp80), 16, roundingMode(), v) ? sreg16(v) : sreg16(-32768);
}

void convert80xi32(const IEEEf80Data* fp80, sreg32* int32)
{
    int64_t v;
    *int32 = toInt(load(fp80), 32, roundingMode(), v) ? sreg32(v) : sreg32(INT32_MIN);
}

void convert80xi64(const IEEEf80Data* fp80, sreg64* int64)
{
    int64_t v;
    *int64 = toInt(load(fp80), 64, roundingMode(), v) ? sreg64(v) : sreg64(INT64_MIN);
}

reg32 add80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    store(result, add(load(result), load(operand), precision(), roundingMode()));
    return 0;
}

reg32 sub80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    store(result, sub(load(result), load(operand), precision(), roundingMode()));
    return 0;
}

reg32 mul80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    store(result, mul(load(result), load(operand), precision(), roundingMode()));
    return 0;
}

reg32 div80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    store(result, div(load(result), load(operand), precision(), roundingMode()));
    return 0;
}

reg32 sqrt80(IEEEf80Data* result)
{
    store(result, sqrt(load(result), precision(), roundingMode()));
    return 0;
}

// fcom: C3, C2, C0 -- all three where either is no number
reg32 cmp80(const IEEEf80Data* f1, const IEEEf80Data* f2)
{
    const X a = load(f1), b = load(f2);
    if (isNaN(a) || isNaN(b))
        return 0x4500;
    if (isZero(a) && isZero(b))
        return 0x4000;
    int c;
    if (a.sign != b.sign)
    {
        if (isZero(a) && isZero(b))
            c = 0;
        else
            c = a.sign ? -1 : 1;
    }
    else
    {
        c = compareMagnitude(a, b);
        if (a.sign)
            c = -c;
    }
    return c == 0 ? 0x4000 : c < 0 ? 0x0100 : 0;
}

reg32 chs80(IEEEf80Data* result)
{
    X x = load(result);
    x.sign = !x.sign;
    store(result, x);
    return 0;
}

reg32 abs80(IEEEf80Data* result)
{
    X x = load(result);
    x.sign = false;
    store(result, x);
    return 0;
}

reg32 round80(IEEEf80Data* result, reg32 mode)
{
    store(result, roundToInteger(load(result), int(mode & 3)));
    return 0;
}

reg32 scale80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    store(result, scale(load(result), load(operand)));
    return 0;
}

reg32 rem80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    reg32 status;
    store(result, remainder(load(result), load(operand), status));
    return status;
}

// fsin, fcos, fptan: C2 and the operand unchanged where |x| is 2^63 or more
static bool outOfRange(const X& x)
{
    return x.exp == 0x7fff || (x.exp >= 0x3fff + 63);
}

reg32 sin80(IEEEf80Data* result)
{
    const X x = load(result);
    if (isNaN(x))
    {
        store(result, quiet(x));
        return 0;
    }
    if (isInf(x))
    {
        store(result, indefinite());
        return 0;
    }
    if (outOfRange(x))
        return 0x0400;
    if (isZero(x))
        return 0;
    store(result, tSin(x));
    return 0;
}

reg32 cos80(IEEEf80Data* result)
{
    const X x = load(result);
    if (isNaN(x))
    {
        store(result, quiet(x));
        return 0;
    }
    if (isInf(x))
    {
        store(result, indefinite());
        return 0;
    }
    if (outOfRange(x))
        return 0x0400;
    store(result, tCos(x));
    return 0;
}

reg32 tan80(IEEEf80Data* result)
{
    const X x = load(result);
    if (isNaN(x))
    {
        store(result, quiet(x));
        return 0;
    }
    if (isInf(x))
    {
        store(result, indefinite());
        return 0;
    }
    if (outOfRange(x))
        return 0x0400;
    if (isZero(x))
        return 0;
    store(result, tTan(x));
    return 0;
}

// fpatan as the helper is called: result = atan2(operand, result)
reg32 atan80(IEEEf80Data* result, const IEEEf80Data* operand)
{
    const X x = load(result), y = load(operand);
    if (isNaN(x) || isNaN(y))
    {
        store(result, propagateNaN(x, y));
        return 0;
    }
    store(result, tAtan2(y, x));
    return 0;
}

// fyl2x with 1: log2
reg32 log280(IEEEf80Data* result)
{
    const X x = load(result);
    if (isNaN(x))
    {
        store(result, quiet(x));
        return 0;
    }
    if (x.sign && !isZero(x))
    {
        store(result, indefinite());
        return 0;
    }
    if (isZero(x))
    {
        store(result, pack(true, 0x7fff, kTop));
        return 0;
    }
    store(result, tLog2(x));
    return 0;
}

// f2xm1: 2^x - 1, worked out as expm1(x ln 2) so that a small x keeps its digits
reg32 f2xm180(IEEEf80Data* result)
{
    const X x = load(result);
    if (isNaN(x))
    {
        store(result, quiet(x));
        return 0;
    }
    if (isZero(x))
        return 0;
    if (isInf(x))
    {
        store(result, x.sign ? pack(true, 0x3fff, kTop) : x);
        return 0;
    }
    store(result, tF2xm1(x));
    return 0;
}

}  // extern "C"

}  // namespace x86
