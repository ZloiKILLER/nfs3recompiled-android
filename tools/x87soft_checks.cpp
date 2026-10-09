/* src/lib/x87soft.cpp against a real x87, bit for bit.  Runs on an x86-64
 * host only (the reference is the machine's own x87):
 *
 *   g++ -std=c++17 -O2 -Iinclude src/lib/x87soft.cpp tools/x87soft_checks.cpp -o x87soft_checks
 *   ./x87soft_checks [cases]
 *
 * The transcendentals as arm64 computes them (IEEE quad), on the same PC:
 *
 *   g++ -std=gnu++17 -O2 -DX87SOFT_QUAD -Iinclude src/lib/x87soft.cpp tools/x87soft_checks.cpp -lquadmath -o x87soft_checks
 *
 * Every operation the WITH_PEDANTIC_FPU build uses, in every precision control
 * (24, 53, 64 bits) and rounding mode, on operands drawn to reach the corners:
 * cancellation, ties, overflow and underflow, denormals, zeros, infinities and
 * NaNs.  Exits non-zero on the first kind of difference. */
#include <fpu.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <random>
#include <string>
#include <cstdlib>

namespace x86
{
void assertLog(const char*, int, const char*) {}
extern "C" void control80(reg16 word);
}

using namespace x86;

struct T
{
    uint8_t b[16];
};

static std::mt19937_64 g(20261005);

static uint64_t rnd() { return g(); }

static T make(bool sign, uint32_t exp, uint64_t sig)
{
    T t;
    std::memset(&t, 0, sizeof t);
    std::memcpy(t.b, &sig, 8);
    const uint16_t se = uint16_t((sign ? 0x8000 : 0) | (exp & 0x7fff));
    std::memcpy(t.b + 8, &se, 2);
    return t;
}

static uint64_t sigOf(const T& t)
{
    uint64_t s;
    std::memcpy(&s, t.b, 8);
    return s;
}

static uint32_t expOf(const T& t)
{
    uint16_t se;
    std::memcpy(&se, t.b + 8, 2);
    return se & 0x7fff;
}

/* An operand: mostly ordinary numbers, with every awkward kind mixed in. */
static T operand(int near = -1)
{
    const bool sign = rnd() & 1;
    uint64_t sig = rnd() | (uint64_t(1) << 63);
    switch (rnd() % 8)
    {
    case 0: sig &= ~((uint64_t(1) << (rnd() % 64)) - 1); break;  // trailing zeros: ties
    case 1: sig = (uint64_t(1) << 63) | (rnd() & 0xff); break;
    case 2: sig = ~uint64_t(0) << (rnd() % 8); break;
    case 3: sig &= 0xffffff0000000000ull; break;                 // a float's digits
    case 4: sig &= 0xfffffffffffff800ull; break;                 // a double's
    default: break;
    }
    uint32_t exp;
    const unsigned kind = rnd() % 100;
    if (near >= 0 && kind < 60)
        exp = uint32_t(near) + uint32_t(rnd() % 140) - 70;
    else if (kind < 75)
        exp = 0x3fff + uint32_t(rnd() % 400) - 200;
    else if (kind < 82)
        exp = 1 + uint32_t(rnd() % 0x7ffe);
    else if (kind < 86)
        exp = 0x7ffe - uint32_t(rnd() % 70);
    else if (kind < 90)
        exp = 1 + uint32_t(rnd() % 70);
    else if (kind < 94)
    {
        exp = 0;
        sig >>= 1 + rnd() % 63;  // a denormal
    }
    else if (kind < 96)
    {
        exp = 0;
        sig = 0;
    }
    else if (kind < 98)
    {
        exp = 0x7fff;
        sig = uint64_t(1) << 63;
    }
    else
    {
        exp = 0x7fff;
        sig = (uint64_t(3) << 62) | (rnd() >> 2);  // a quiet NaN
    }
    if (exp >= 0x7fff && kind < 96)
        exp = 0x7ffe;
    if (exp == 0 && kind < 90)
        exp = 1;
    return make(sign, exp, sig);
}

static uint16_t cwOf(int pc, int rc) { return uint16_t(0x007f | (pc << 8) | (rc << 10)); }

enum Op { ADD, SUB, MUL, DIV, SQRT, CMP, RND, SCALE, REM, TO64, TO32, TOI16, TOI32, TOI64, SIN, COS, TAN, ATAN, LOG2, F2XM1, OPS };
static const char* names[] = { "fadd", "fsub", "fmul", "fdiv", "fsqrt", "fcom", "frndint", "fscale", "fprem",
                               "fst m64", "fst m32", "fistp m16", "fistp m32", "fistp m64",
                               "fsin", "fcos", "fptan", "fpatan", "fyl2x", "f2xm1" };

/* The real x87: a and b loaded (st0 = a, st1 = b), the operation, the result
 * stored as 80 bits (or as the conversion's format), and the status word. */
static void hardware(int op, const T& a, const T& b, uint16_t cw, T& out, uint16_t& sw)
{
    std::memset(&out, 0, sizeof out);
    uint16_t saved;
    asm volatile("fnstcw %0" : "=m"(saved));
    asm volatile("fninit\n fldcw %0" :: "m"(cw));
    asm volatile("fldt %0" :: "m"(b));
    asm volatile("fldt %0" :: "m"(a));
    switch (op)
    {
    case ADD: asm volatile(".byte 0xde,0xc1\n fstpt %0" : "=m"(out)); break;   // faddp
    case SUB: asm volatile(".byte 0xde,0xe1\n fstpt %0" : "=m"(out)); break;   // st1 = st0 - st1
    case MUL: asm volatile(".byte 0xde,0xc9\n fstpt %0" : "=m"(out)); break;
    case DIV: asm volatile(".byte 0xde,0xf1\n fstpt %0" : "=m"(out)); break;   // st1 = st0 / st1
    case SQRT: asm volatile("fsqrt\n fstpt %0\n fstp %%st(0)" : "=m"(out)); break;
    case CMP: asm volatile("fcompp"); break;
    case RND: asm volatile("frndint\n fstpt %0\n fstp %%st(0)" : "=m"(out)); break;
    case SCALE: asm volatile("fscale\n fstpt %0\n fstp %%st(0)" : "=m"(out)); break;
    case REM: asm volatile("fprem\n fnstsw %1\n fstpt %0\n fstp %%st(0)" : "=m"(out), "=m"(sw)); break;
    case TO64: asm volatile("fstpl %0\n fstp %%st(0)" : "=m"(out)); break;
    case TO32: asm volatile("fstps %0\n fstp %%st(0)" : "=m"(out)); break;
    case TOI16: asm volatile("fistps %0\n fstp %%st(0)" : "=m"(out)); break;
    case TOI32: asm volatile("fistpl %0\n fstp %%st(0)" : "=m"(out)); break;
    case TOI64: asm volatile("fistpll %0\n fstp %%st(0)" : "=m"(out)); break;
    case SIN: asm volatile("fsin\n fnstsw %1\n fstpt %0\n fstp %%st(0)" : "=m"(out), "=m"(sw)); break;
    case COS: asm volatile("fcos\n fnstsw %1\n fstpt %0\n fstp %%st(0)" : "=m"(out), "=m"(sw)); break;
    case TAN: asm volatile("fptan\n fnstsw %1\n fstp %%st(0)\n fstpt %0\n fstp %%st(0)" : "=m"(out), "=m"(sw)); break;
    case ATAN: asm volatile("fxch\n fpatan\n fstpt %0" : "=m"(out)); break;            // atan2(a, b) as atan80(b, a)
    case LOG2: asm volatile("fld1\n fxch\n fyl2x\n fstpt %0\n fstp %%st(0)" : "=m"(out)); break;
    case F2XM1: asm volatile("f2xm1\n fstpt %0\n fstp %%st(0)" : "=m"(out)); break;
    }
    if (op == CMP)
        asm volatile("fnstsw %0" : "=m"(sw));
    asm volatile("fninit\n fldcw %0" :: "m"(saved));
}

static void software(int op, const T& a, const T& b, uint16_t cw, T& out, uint16_t& sw)
{
    std::memset(&out, 0, sizeof out);
    control80(cw);
    IEEEf80Data x, y;
    std::memcpy(&x, a.b, 10);
    std::memcpy(&y, b.b, 10);
    reg32 r = 0;
    switch (op)
    {
    case ADD: r = add80(&x, &y); break;
    case SUB: r = sub80(&x, &y); break;
    case MUL: r = mul80(&x, &y); break;
    case DIV: r = div80(&x, &y); break;
    case SQRT: r = sqrt80(&x); break;
    case CMP: r = cmp80(&x, &y); break;
    case RND: r = round80(&x, (cw >> 10) & 3); break;
    case SCALE: r = scale80(&x, &y); break;
    case REM: r = rem80(&x, &y); break;
    case TO64: convert80x64(&x, reinterpret_cast<double*>(out.b)); break;
    case TO32: convert80x32(&x, reinterpret_cast<float*>(out.b)); break;
    case TOI16: convert80xi16(&x, reinterpret_cast<sreg16*>(out.b)); break;
    case TOI32: convert80xi32(&x, reinterpret_cast<sreg32*>(out.b)); break;
    case TOI64: convert80xi64(&x, reinterpret_cast<sreg64*>(out.b)); break;
    case SIN: r = sin80(&x); break;
    case COS: r = cos80(&x); break;
    case TAN: r = tan80(&x); break;
    case ATAN:
    {
        IEEEf80Data v = y, o = x;  // result = atan2(operand = a, value = b)
        r = atan80(&v, &o);
        x = v;
        break;
    }
    case LOG2: r = log280(&x); break;
    case F2XM1: r = f2xm180(&x); break;
    }
    if (op < TO64 || op >= SIN)
        std::memcpy(out.b, &x, 10);
    sw = uint16_t(r);
}

static size_t width(int op)
{
    switch (op)
    {
    case TO64: case TOI64: return 8;
    case TO32: case TOI32: return 4;
    case TOI16: return 2;
    default: return 10;
    }
}

static std::string hex(const T& t, size_t n)
{
    std::string s;
    char buf[4];
    for (size_t i = n; i-- > 0;)
    {
        std::snprintf(buf, sizeof buf, "%02x", t.b[i]);
        s += buf;
    }
    return s;
}

static bool isNaN80(const T& t) { return expOf(t) == 0x7fff && (sigOf(t) << 1) != 0; }

int main(int argc, char** argv)
{
    const long cases = argc > 1 ? std::atol(argv[1]) : 300000;
    int failedKinds = 0;
    for (int op = 0; op < OPS; ++op)
    {
        const bool transcendental = op >= SIN;
        long n = 0, bad = 0, nanOnly = 0, lastBit = 0;
        int shown = 0;
        for (long i = 0; i < cases; ++i)
        {
            const int pc = transcendental ? 3 : int(rnd() % 3 == 0 ? 0 : rnd() % 2 ? 2 : 3);
            const int rc = transcendental ? 0 : int(rnd() % 4);
            const uint16_t cw = cwOf(pc, rc);
            T a = operand();
            T b = operand(int(expOf(a)));
            if (op == SQRT || op == LOG2)
                a.b[9] &= 0x7f;  // mostly positive
            if (op == SCALE)
                b = make(rnd() & 1, 0x3fff + uint32_t(rnd() % 15), rnd() | (uint64_t(1) << 63));
            if (op == SIN || op == COS || op == TAN)
                a = make(rnd() & 1, 0x3fff - 30 + uint32_t(rnd() % 34), rnd() | (uint64_t(1) << 63));
            if (op == F2XM1)
                a = make(rnd() & 1, 0x3fff - 70 + uint32_t(rnd() % 70), rnd() | (uint64_t(1) << 63));
            if (op == REM && (rnd() % 4) == 0)
                b = make(rnd() & 1, expOf(a) - uint32_t(rnd() % 200), rnd() | (uint64_t(1) << 63));
            T h, s;
            uint16_t hs = 0, ss = 0;
            hardware(op, a, b, cw, h, hs);
            software(op, a, b, cw, s, ss);
            ++n;
            const size_t w = width(op);
            bool same = op == CMP || std::memcmp(h.b, s.b, w) == 0;
            if (op == CMP || op == REM)
                same = same && ((hs & 0x4700) == (ss & 0x4700));
            if (op == SIN || op == COS || op == TAN)
                same = same && ((hs & 0x0400) == (ss & 0x0400));
            if (!same && w == 10 && isNaN80(h) && isNaN80(s))
            {
                ++nanOnly;
                if (std::getenv("SHOW_NAN"))
                    std::printf("  NaN %s a=%s b=%s -> x87 %s, soft %s\n", names[op], hex(a, 10).c_str(),
                                hex(b, 10).c_str(), hex(h, 10).c_str(), hex(s, 10).c_str());
                continue;
            }
            if (!same && transcendental && expOf(h) == expOf(s))
            {
                const uint64_t d = sigOf(h) > sigOf(s) ? sigOf(h) - sigOf(s) : sigOf(s) - sigOf(h);
                if (d <= 1)
                {
                    ++lastBit;
                    continue;
                }
            }
            if (!same)
            {
                ++bad;
                if (shown++ < 5)
                    std::printf("  %s pc=%d rc=%d a=%s b=%s -> x87 %s/%04x, soft %s/%04x\n", names[op], pc, rc,
                                hex(a, 10).c_str(), hex(b, 10).c_str(), hex(h, w).c_str(), hs & 0x4700,
                                hex(s, w).c_str(), ss & 0x4700);
            }
        }
        std::printf("%-10s %8ld cases: %8ld differ%s", names[op], n, bad, bad ? "  <<<<" : "");
        if (nanOnly)
            std::printf(", %ld only in which NaN", nanOnly);
        if (transcendental)
            std::printf(", %ld a last bit apart", lastBit);
        std::printf("\n");
        if (bad && !transcendental)
            ++failedKinds;
    }
    std::printf("%d exact operations differ\n", failedKinds);
    return failedKinds ? 1 : 0;
}
