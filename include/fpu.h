#ifndef FPU_H_
#define FPU_H_

#include <x86.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

//#define NFS2SE_X87_DEBUG 1

#if defined(_MSC_VER) && !defined(__clang__)
#define FPU_NOINLINE __declspec(noinline)
#else
#define FPU_NOINLINE __attribute__((noinline))
#endif

namespace x86
{

#ifdef WITH_PEDANTIC_FPU
    struct Float;
#endif

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

#ifdef WITH_PEDANTIC_FPU
        inline void operator=(const Float &value);
#endif

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
    extern "C" x86::reg32 add80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 sub80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 mul80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 div80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 cmp80(const IEEEf80Data *f1, const IEEEf80Data *f2);
    extern "C" x86::reg32 chs80(IEEEf80Data *result);
    extern "C" x86::reg32 sin80(IEEEf80Data *result);
    extern "C" x86::reg32 cos80(IEEEf80Data *result);
    extern "C" x86::reg32 sqrt80(IEEEf80Data *result);
    extern "C" x86::reg32 tan80(IEEEf80Data *result);
    extern "C" x86::reg32 log280(IEEEf80Data *result);
    extern "C" x86::reg32 atan80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 abs80(IEEEf80Data *result);
    extern "C" x86::reg32 round80(IEEEf80Data *result, x86::reg32 mode);
    extern "C" x86::reg32 f2xm180(IEEEf80Data *result);
    extern "C" x86::reg32 scale80(IEEEf80Data *result, const IEEEf80Data *operand);
    extern "C" x86::reg32 rem80(IEEEf80Data *result, const IEEEf80Data *operand);

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

#ifdef WITH_PEDANTIC_FPU
    struct Float
    {
        IEEEf80 f80value;
#ifdef NFS2SE_X87_DEBUG
        float dbgValue;
#endif

        Float()
            : f80value()
#ifdef NFS2SE_X87_DEBUG
              ,
              dbgValue(0)
#endif
        {
        }
        Float(const IEEEf80 &value)
            : f80value(value)
#ifdef NFS2SE_X87_DEBUG
              ,
              dbgValue(0)
#endif
        {
        }
        Float(float value)
        {
            convert32x80(&value, &f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
        }

        Float(double value)
        {
            convert64x80(&value, &f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
        }

        Float(sreg16 value)
        {
            converti16x80(&value, &f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
        }

        Float(sreg32 value)
        {
            converti32x80(&value, &f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
        }
        Float(sreg64 value)
        {
            converti64x80(&value, &f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
        }

        operator IEEEf80() const
        {
            return f80value;
        }

        operator double() const
        {
            double result;
            convert80x64(&f80value.data, &result);
            return result;
        }
        operator float() const
        {
            float result;
            convert80x32(&f80value.data, &result);
            return result;
        }
        operator sreg16() const
        {
            sreg16 result;
            convert80xi16(&f80value.data, &result);
            return result;
        }
        operator sreg32() const
        {
            sreg32 result;
            convert80xi32(&f80value.data, &result);
            return result;
        }
        operator sreg64() const
        {
            sreg64 result;
            convert80xi64(&f80value.data, &result);
            return result;
        }

        Float &operator+=(const Float &other)
        {
            add80(&f80value.data, &other.f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
            return *this;
        }

        Float &operator-=(const Float &other)
        {
            sub80(&f80value.data, &other.f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
            return *this;
        }

        Float &operator*=(const Float &other)
        {
            mul80(&f80value.data, &other.f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
            return *this;
        }

        Float &operator/=(const Float &other)
        {
            div80(&f80value.data, &other.f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&f80value.data, &dbgValue);
#endif
            return *this;
        }
        Float operator-() const
        {
            Float result = *this;
            chs80(&result.f80value.data);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
        }
    };

    static inline Float operator+(const Float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result += arg2;
        return result;
    }

    static inline Float operator-(const Float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result -= arg2;
        return result;
    }

    static inline Float operator*(const Float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result *= arg2;
        return result;
    }

    static inline Float operator/(const Float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result /= arg2;
        return result;
    }

    static inline Float operator+(const float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result += arg2;
        return result;
    }

    static inline Float operator-(const float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result -= arg2;
        return result;
    }

    static inline Float operator*(const float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result *= arg2;
        return result;
    }

    static inline Float operator/(const float &arg1, const Float &arg2)
    {
        Float result = arg1;
        result /= arg2;
        return result;
    }
    static inline Float operator+(const double &arg1, const Float &arg2)
    {
        Float result = arg1;
        result += arg2;
        return result;
    }

    static inline Float operator-(const double &arg1, const Float &arg2)
    {
        Float result = arg1;
        result -= arg2;
        return result;
    }

    static inline Float operator*(const double &arg1, const Float &arg2)
    {
        Float result = arg1;
        result *= arg2;
        return result;
    }

    static inline Float operator/(const double &arg1, const Float &arg2)
    {
        Float result = arg1;
        result /= arg2;
        return result;
    }
#elif defined(WITH_WIDE_FPU)
    /* An experiment (WITH_WIDE_FPU): every value on the x87 stack as a long
     * double -- IEEE quad on arm64, 113 bits of significand, done in software
     * and slow -- and never rounded to the race's single precision.  That is
     * what Modern Patch does on a PC since 1.6.0 ("FPU always uses extended
     * precision"): its race loop keeps the fninit and drops the fldcw at
     * 0x4a3ed3.  Only to see whether the opponents drive better with more
     * precision than a double gives. */
    typedef long double Float;
#else
    typedef double Float;
#endif

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

        Float &st(int index)
        {
            Float &result = regs[(count - index) & 7];
            return result;
        }

        inline void init()
        {
            status.word = 0;
            count = 0;
            setControl(0x37f);
        }

        /* fldcw and init.  Nothing else writes the control word, so what it
         * asks of the arithmetic below is worked out here, once, rather than
         * pulled out of its bit fields at every fadd. */
        inline void setControl(reg16 word)
        {
            control.word = word;
#if defined(WITH_WIDE_FPU)
            rounding = kRoundNone;   // as Modern Patch: the race's single precision is never taken
#elif !defined(WITH_PEDANTIC_FPU)
            rounding = control.pc != s_singlePrecision ? kRoundNone
                     : control.rc == 0 ? kRoundSingleNearest : kRoundSingleOther;
#endif
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
#ifdef WITH_PEDANTIC_FPU
        inline Float add(const Float &a, const Float &b) { return a + b; }
        inline Float sub(const Float &a, const Float &b) { return a - b; }
        inline Float mul(const Float &a, const Float &b) { return a * b; }
        inline Float div(const Float &a, const Float &b) { return a / b; }
#else
        /* Which operation a rounding came from, for the rare rounding that has
         * to know the exact result more closely than the double says. */
        enum class Rounded { Add, Sub, Mul, Div, Sqrt };

        /* The precision control that means single precision: 0, as the x87
         * has it -- or 4, which two bits never hold, for a run without the
         * rounding (NFS_FPU_SINGLE=0), to compare frame rates by. */
#ifdef WITH_WIDE_FPU
        // No race is ever in single precision, so the native loops stay off too.
        static inline x86::reg8 s_singlePrecision = 4;
#else
        static inline x86::reg8 s_singlePrecision = 0;
#endif

        /* What setControl found the control word to ask for. */
        static constexpr x86::reg8 kRoundNone = 0;           // 53 or 64 bits: the double as it is
        static constexpr x86::reg8 kRoundSingleNearest = 1;  // a race
        static constexpr x86::reg8 kRoundSingleOther = 2;    // single, a directed mode
        x86::reg8 rounding = kRoundNone;

        inline Float add(const Float &a, const Float &b)
        {
#ifdef WITH_WIDE_FPU
            return a + b;
#else
            const double sum = a + b;
            return rounding == kRoundNone ? sum : toSingle(sum, Rounded::Add, a, b);
#endif
        }

        inline Float sub(const Float &a, const Float &b)
        {
#ifdef WITH_WIDE_FPU
            return a - b;
#else
            const double difference = a - b;
            return rounding == kRoundNone ? difference : toSingle(difference, Rounded::Sub, a, b);
#endif
        }

        inline Float mul(const Float &a, const Float &b)
        {
#ifdef WITH_WIDE_FPU
            return a * b;
#else
            const double product = a * b;
            return rounding == kRoundNone ? product : toSingle(product, Rounded::Mul, a, b);
#endif
        }

        inline Float div(const Float &a, const Float &b)
        {
#ifdef WITH_WIDE_FPU
            return a / b;
#else
            const double quotient = a / b;
            return rounding == kRoundNone ? quotient : toSingle(quotient, Rounded::Div, a, b);
#endif
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
#endif

        inline void compare(const Float& val1, const Float& val2)
        {
#ifdef WITH_PEDANTIC_FPU
            x86::reg32 r = cmp80(&val1.f80value.data, &val2.f80value.data) & 0x4700;
            status.word &= ~0x4700;
            status.word |= r;
#else
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
#endif
        }

        inline Float log2(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = log280(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::log2(value);
#endif
        }

        inline Float sin(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = sin80(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::sin(value);
#endif
        }

        inline Float cos(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = cos80(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::cos(value);
#endif
        }

        inline Float tan(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = tan80(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::tan(value);
#endif
        }

        inline Float abs(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = abs80(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::fabs(value);
#endif
        }

        inline Float sqrt(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = sqrt80(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
#ifdef WITH_WIDE_FPU
            return std::sqrt(value);
#else
            const double root = ::sqrt(value);
            return rounding == kRoundNone ? root : toSingle(root, Rounded::Sqrt, value, 0.0);
#endif
#endif
        }

        inline Float atan(const Float &value, const Float &operand)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = atan80(&result.f80value.data, &operand.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::atan2(operand, value);
#endif
        }

        inline Float rem(const Float &val1, const Float &val2)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = val1;
            x86::reg32 r = rem80(&result.f80value.data, &val2.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            status.c2 = 0;
            return std::fmod(val1, val2);
#endif
        }

        inline Float scale(const Float &val1, const Float &val2)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = val1;
            x86::reg32 r = scale80(&result.f80value.data, &val2.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return val1 * std::pow(Float(2), std::trunc(val2));
#endif
        }

        inline Float f2xm1(const Float &value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            x86::reg32 r = f2xm180(&result.f80value.data);
            status.word &= ~0x4700;
            status.word |= r & 0x4700;
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
            return std::pow(Float(2), value) - Float(1);
#endif
        }

        /* st(0) rounded to an integer in the control word's mode -- frndint,
         * and fistp before the store.  The value is passed in, so the code
         * generator can hand over a value it keeps in a local. */
        Float rndint(const Float& value)
        {
#ifdef WITH_PEDANTIC_FPU
            Float result = value;
            round80(&result.f80value.data, control.rc);
#ifdef NFS2SE_X87_DEBUG
            convert80x32(&result.f80value.data, &result.dbgValue);
#endif
            return result;
#else
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
#endif
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
#ifdef WITH_PEDANTIC_FPU
            return Int(rndint(value));
#else
            const Float rounded = rndint(value);
            const Float lowest = Float(std::numeric_limits<Int>::min());
            if (!(rounded >= lowest && rounded < -lowest))
            {
                ++s_indefinite;
                return std::numeric_limits<Int>::min();
            }
            return Int(rounded);
#endif
        }
    };

#ifdef WITH_PEDANTIC_FPU
    void IEEEf80::operator=(const Float &value)
    {
        *this = value.f80value;
    }
#endif

}

#endif /* !FPU_H_ */
