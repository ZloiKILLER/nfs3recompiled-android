#ifndef FPU_H_
#define FPU_H_

#include <x86.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

#if defined(_MSC_VER) && !defined(__clang__)
#define FPU_NOINLINE __declspec(noinline)
#else
#define FPU_NOINLINE __attribute__((noinline))
#endif

namespace x86
{


    struct IEEEf80Data
    {
        x86::reg16  data1;
        x86::reg16  data2;
        x86::reg16  data3;
        x86::reg16  data4;
        x86::reg16  data5;
    };
    struct IEEEf80
    {
        IEEEf80Data data;

        inline IEEEf80();
        inline IEEEf80(IEEEf80Data data) : data(data) {};
        inline IEEEf80(double value);


        inline operator double() const;
        inline void operator=(double value);
    };

    extern "C" void convert80x64(const IEEEf80Data *fp80, double *fp64);
    extern "C" void convert80x32(const IEEEf80Data *fp80, float *fp32);
    extern "C" void convert64x80(const double *fp64, IEEEf80Data *fp80);
    extern "C" void convert32x80(const float *fp32, IEEEf80Data *fp80);
    extern "C" void converti16x80(const sreg16 *int16, IEEEf80Data *fp80);
    extern "C" void converti32x80(const sreg32 *int32, IEEEf80Data *fp80);
    extern "C" void converti64x80(const sreg64 *int64, IEEEf80Data *fp80);
    extern "C" void convert80xi16(const IEEEf80Data *fp80, sreg16 *int16);
    extern "C" void convert80xi32(const IEEEf80Data *fp80, sreg32 *int32);
    extern "C" void convert80xi64(const IEEEf80Data *fp80, sreg64 *int64);

    inline IEEEf80::IEEEf80()
        : data{0, 0, 0, 0, 0}
    {
    }

    inline IEEEf80::IEEEf80(double value)
    {
        convert64x80(&value, &data);
    }

    IEEEf80::operator double() const
    {
        double result;
        convert80x64(&data, &result);
        return result;
    }

    void IEEEf80::operator=(double value)
    {
        convert64x80(&value, &data);
    }

    typedef double Float;

    struct FPU
    {
        union
        {
            reg16 word;
            struct
            {
                reg8 im : 1;
                reg8 dm : 1;
                reg8 zm : 1;
                reg8 om : 1;
                reg8 um : 1;
                reg8 pm : 1;
                reg8 unused1 : 2;
                reg8 pc : 2;
                reg8 rc : 2;
                reg8 ic : 1;
                reg8 unused2 : 3;
            };
        } control;

        union
        {
            reg16 word;
            struct
            {
                reg8 ie : 1;
                reg8 de : 1;
                reg8 ze : 1;
                reg8 oe : 1;
                reg8 ue : 1;
                reg8 pe : 1;
                reg8 sf : 1;
                reg8 es : 1;
                reg8 c0 : 1;
                reg8 c1 : 1;
                reg8 c2 : 1;
                reg8 top : 3;
                reg8 c3 : 1;
                reg8 b : 1;
            };
        } status;
        x86::reg32 count;

        Float regs[8];
        void push(Float value)
        {
            count++;
            NFS2_ASSERT(count <= 8);
            regs[count & 7] = value;
        }
        Float pop()
        {
            if (count > 0)
            {
                Float result = regs[count & 7];
                count--;
                return result;
            }
            else
            {
                return 0.0;
            }
        }

        /* What a callee left on the stack, as the generated code takes it
         * (codegen/fpu_frame.py): the count moves whatever it was, as a native's
         * does. */
        Float take()
        {
            const Float result = regs[count & 7];
            count--;
            return result;
        }

        Float &st(int index)
        {
            Float &result = regs[(count - index) & 7];
            return result;
        }

        /* fninit.  The word an x87 starts with asks for 64 bits; here it asks
         * for single precision, as the game's races do, so that the menus are in
         * it too and code that reads the word and loads it again (the game's own
         * loading screen does) asks for no other. */
        inline void init()
        {
            status.word = 0;
            count = 0;
            setControl(0x7f);
        }

        /* fldcw and init.  Nothing else writes the control word, so what it
         * asks of the arithmetic below is worked out here, once, rather than
         * pulled out of its bit fields at every fadd. */
        /* Told of every control word as it is set, when something listens:
         * the live functions log (src/nfs3hp/func_log.cpp) notes each
         * thread's precision.  Null, as it nearly always is, it costs a test
         * at each fldcw. */
        static inline void (*s_controlHook)(reg16 word) = nullptr;

        inline void setControl(reg16 word)
        {
            control.word = word;
            if (s_controlHook)
                s_controlHook(word);
            rounding = control.pc != 0 ? kRoundNone : control.rc == 0 ? kRoundSingleNearest : kRoundSingleOther;
        }

        /* The four operations and fsqrt as the x87 rounds them: to the
         * precision its control word asks for.  NFS3 clears the precision
         * control before every race (fldcw at 0x4a3ed3), so on a PC each sum,
         * product and quotient of the race comes out with a float's 24-bit
         * significand, while the exponent keeps the x87's range.  Worked out in
         * double and rounded from there, the result is the x87's bit for bit:
         * the exact result is the double plus an error whose sign is known --
         * TwoSum for a sum, fma for a product, the remainder for a quotient or
         * a root -- and that sign settles the only cases rounding twice could
         * get wrong, a double exactly half way between two floats or exactly on
         * one.  Precision control at 53 or 64 bits leaves the double as it is,
         * as close as the port comes to either.  The disassembler emits these
         * for fadd, fsub, fsubr, fmul, fdiv and fdivr (disasm/codegen/fpu.py). */
        /* Which operation a rounding came from, for the rare rounding that has
         * to know the exact result more closely than the double says. */
        enum class Rounded { Add, Sub, Mul, Div, Sqrt };

        /* What setControl found the control word to ask for. */
        static constexpr x86::reg8 kRoundNone = 0;           // 53 or 64 bits: the double as it is
        static constexpr x86::reg8 kRoundSingleNearest = 1;  // a race
        static constexpr x86::reg8 kRoundSingleOther = 2;    // single, a directed mode
        x86::reg8 rounding = kRoundSingleNearest;

        inline Float add(const Float &a, const Float &b)
        {
            const double sum = a + b;
            return rounding == kRoundNone ? sum : toSingle(sum, Rounded::Add, a, b);
        }

        inline Float sub(const Float &a, const Float &b)
        {
            const double difference = a - b;
            return rounding == kRoundNone ? difference : toSingle(difference, Rounded::Sub, a, b);
        }

        inline Float mul(const Float &a, const Float &b)
        {
            const double product = a * b;
            return rounding == kRoundNone ? product : toSingle(product, Rounded::Mul, a, b);
        }

        inline Float div(const Float &a, const Float &b)
        {
            const double quotient = a / b;
            return rounding == kRoundNone ? quotient : toSingle(quotient, Rounded::Div, a, b);
        }

        static constexpr std::uint64_t kSingleDropped = (std::uint64_t(1) << 29) - 1;
        static constexpr std::uint64_t kSingleHalf = std::uint64_t(1) << 28;
        static constexpr std::uint64_t kSingleStep = std::uint64_t(1) << 29;

        /* `value` rounded to a 24-bit significand in the rounding mode of the
         * control word.  It stands in every one of the game's sums, so only the
         * common case is done here: to nearest, not exactly half way between two
         * floats, and inside a float's normal range short of 2^127 -- where the
         * host's own double -> float -> double rounds exactly as the x87 would
         * (and as masking the bits did before), and no rounding up can reach a
         * float's infinity.  The bits only decide which way to go, so the value
         * itself never leaves the FPU registers; moving it to the integer unit
         * and back on every operation was a fifth of a split-screen race.  A
         * tie, a directed mode, a value past a float's range (the x87 keeps its
         * own wider exponent), an infinity or a NaN goes to roundSingleSlow,
         * which works out from `a` and `b` which side of `value` the exact result
         * lies. */
        inline double toSingle(double value, Rounded op, double a, double b) const
        {
            std::uint64_t bits;
            std::memcpy(&bits, &value, sizeof bits);
            const std::uint64_t exponent = (bits >> 52) & 0x7ff;
            if (rounding == kRoundSingleNearest && (bits & kSingleDropped) != kSingleHalf
                && exponent - 0x381 < 0x47e - 0x381)
                return double(float(value));
            if ((bits << 1) == 0)
                return value;  // either zero
            return roundSingleSlow(value, op, a, b, control.rc);
        }

        static inline int sign(double value)
        {
            return (value > 0) - (value < 0);
        }

        static FPU_NOINLINE double roundSingleSlow(double value, Rounded op, double a, double b, unsigned rc)
        {
            std::uint64_t bits;
            std::memcpy(&bits, &value, sizeof bits);
            if (value == 0 || ((bits >> 52) & 0x7ff) == 0x7ff)
                return value;
            int beyond;  // the sign of the exact result minus `value`
            switch (op)
            {
            case Rounded::Add:
            {
                const double part = value - a;
                beyond = sign((a - (value - part)) + (b - part));
                break;
            }
            case Rounded::Sub:
            {
                const double part = value - a;
                beyond = sign((a - (value - part)) + (-b - part));
                break;
            }
            case Rounded::Mul:
                beyond = sign(std::fma(a, b, -value));
                break;
            case Rounded::Div:
                beyond = sign(std::fma(-value, b, a)) * sign(b);
                break;
            default:  // the square root of `a`
                beyond = sign(std::fma(-value, value, a));
                break;
            }
            const bool negative = value < 0;
            if (negative)
                beyond = -beyond;  // the exact magnitude against |value|
            const std::uint64_t dropped = bits & kSingleDropped;
            if (dropped == 0 && beyond == 0)
                return value;
            std::uint64_t kept = bits & ~kSingleDropped;
            if (rc == 0)
            {
                if (dropped > kSingleHalf || (dropped == kSingleHalf && (beyond > 0 || (beyond == 0 && ((kept >> 29) & 1)))))
                    kept += kSingleStep;
            }
            else
            {
                const bool away = rc != 3 && ((rc == 2) != negative);
                if (away)
                {
                    if (dropped != 0 || beyond > 0)
                        kept += kSingleStep;
                }
                else if (dropped == 0 && beyond < 0)
                {
                    kept -= kSingleStep;
                }
            }
            std::memcpy(&value, &kept, sizeof kept);
            return value;
        }

        inline void compare(const Float& val1, const Float& val2)
        {
            if (val1 == val2)
            {
                status.c3 = true;
                status.c2 = false;
                status.c0 = false;
            }
            else if (val1 > val2)
            {
                status.c3 = false;
                status.c2 = false;
                status.c0 = false;
            }
            else if (val1 < val2)
            {
                status.c3 = false;
                status.c2 = false;
                status.c0 = true;
            }
            else
            {
                ++s_unordered;
                status.c3 = true;
                status.c2 = true;
                status.c0 = true;
            }
        }

        inline Float log2(const Float &value)
        {
            return std::log2(value);
        }

        inline Float sin(const Float &value)
        {
            return std::sin(value);
        }

        inline Float cos(const Float &value)
        {
            return std::cos(value);
        }

        inline Float tan(const Float &value)
        {
            return std::tan(value);
        }

        inline Float abs(const Float &value)
        {
            return std::fabs(value);
        }

        inline Float sqrt(const Float &value)
        {
            const double root = ::sqrt(value);
            return rounding == kRoundNone ? root : toSingle(root, Rounded::Sqrt, value, 0.0);
        }

        inline Float atan(const Float &value, const Float &operand)
        {
            return std::atan2(operand, value);
        }

        inline Float rem(const Float &val1, const Float &val2)
        {
            status.c2 = 0;
            return std::fmod(val1, val2);
        }

        inline Float scale(const Float &val1, const Float &val2)
        {
            return val1 * std::pow(Float(2), std::trunc(val2));
        }

        inline Float f2xm1(const Float &value)
        {
            return std::pow(Float(2), value) - Float(1);
        }

        /* st(0) rounded to an integer in the control word's mode -- frndint,
         * and fistp before the store.  The value is passed in, so the code
         * generator can hand over a value it keeps in a local. */
        Float rndint(const Float& value)
        {
            switch (control.rc)
            {
            case 0:
                return std::nearbyint(value);
            case 1:
                return std::floor(value);
            case 2:
                return std::ceil(value);
            case 3:
                return std::trunc(value);
            default:
                NFS2_ASSERT(false);
                return std::trunc(value);
            }
        }

        Float rndint()
        {
            return rndint(st(0));
        }

        /* How often, since the start, a compare met a NaN and an fistp had no
         * integer to store -- told apart from the game's normal run by the
         * tick trace ([STEP]). */
        static inline unsigned s_unordered = 0;
        static inline unsigned s_indefinite = 0;

        /* fist/fistp: st(0) rounded in the control word's mode and stored as
         * an integer of `Int`'s size.  A NaN, or a value past the integer's
         * range, stores the x87's "integer indefinite", the lowest integer --
         * the host's conversion would be undefined there, and arm64 clamps a
         * value to the nearest end and turns a NaN into 0. */
        template <typename Int>
        Int toInteger(const Float& value)
        {
            const Float rounded = rndint(value);
            const Float lowest = Float(std::numeric_limits<Int>::min());
            if (!(rounded >= lowest && rounded < -lowest))
            {
                ++s_indefinite;
                return std::numeric_limits<Int>::min();
            }
            return Int(rounded);
        }
    };


}

/* The game's arithmetic with nothing behind it: no stack, no control word, no
 * status word.  The sums, products, quotients and roots come out as an x87 at
 * single precision has them, to the bit; codegen/fpu.py writes these for the
 * generated code.  Values stay doubles between operations: a float's 24 bits
 * with the x87's wider exponent, and the full width of what sin and the like
 * return, which the x87 does not round either. */
namespace fp
{
    using x86::Float;

    inline double single(double value, x86::FPU::Rounded op, double a, double b)
    {
        std::uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        const std::uint64_t exponent = (bits >> 52) & 0x7ff;
        if ((bits & x86::FPU::kSingleDropped) != x86::FPU::kSingleHalf && exponent - 0x381 < 0x47e - 0x381)
            return double(float(value));
        if ((bits << 1) == 0)
            return value;  // either zero
        return x86::FPU::roundSingleSlow(value, op, a, b, 0);
    }

    inline double add(double a, double b) { return single(a + b, x86::FPU::Rounded::Add, a, b); }
    inline double sub(double a, double b) { return single(a - b, x86::FPU::Rounded::Sub, a, b); }
    inline double mul(double a, double b) { return single(a * b, x86::FPU::Rounded::Mul, a, b); }
    inline double div(double a, double b) { return single(a / b, x86::FPU::Rounded::Div, a, b); }
    inline double sqrt(double a) { return single(::sqrt(a), x86::FPU::Rounded::Sqrt, a, 0.0); }

    /* fcom: the word fnstsw reads, the x87's c3, c2 and c0. */
    inline x86::reg16 compare(double a, double b)
    {
        if (a == b)
            return 0x4000;
        if (a > b)
            return 0;
        if (a < b)
            return 0x100;
        ++x86::FPU::s_unordered;
        return 0x4500;
    }

    inline double abs(double a) { return std::fabs(a); }
    inline double sin(double a) { return std::sin(a); }
    inline double cos(double a) { return std::cos(a); }
    inline double tan(double a) { return std::tan(a); }
    inline double log2(double a) { return std::log2(a); }
    inline double atan(double a, double b) { return std::atan2(b, a); }
    inline double rem(double a, double b) { return std::fmod(a, b); }
    inline double scale(double a, double b) { return a * std::pow(2.0, std::trunc(b)); }
    inline double f2xm1(double a) { return std::pow(2.0, a) - 1.0; }
    inline double rndint(double a) { return std::nearbyint(a); }

    /* fistp, to nearest: the x87's integer indefinite, the lowest integer, for a
     * NaN or a value out of range, where the host's conversion is undefined. */
    template <typename Int>
    inline Int toInteger(double value)
    {
        const double rounded = std::nearbyint(value);
        const double lowest = double(std::numeric_limits<Int>::min());
        if (!(rounded >= lowest && rounded < -lowest))
        {
            ++x86::FPU::s_indefinite;
            return std::numeric_limits<Int>::min();
        }
        return Int(rounded);
    }
}

#endif /* !FPU_H_ */
