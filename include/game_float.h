#ifndef GAME_FLOAT_H_
#define GAME_FLOAT_H_

#include <game_types.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

#if defined(_MSC_VER) && !defined(__clang__)
#define FLOAT_NOINLINE __declspec(noinline)
#else
#define FLOAT_NOINLINE __attribute__((noinline))
#endif

namespace nfs
{


    struct Float80Data
    {
        nfs::u16  data1;
        nfs::u16  data2;
        nfs::u16  data3;
        nfs::u16  data4;
        nfs::u16  data5;
    };
    struct Float80
    {
        Float80Data data;

        inline Float80();
        inline Float80(Float80Data data) : data(data) {};
        inline Float80(double value);


        inline operator double() const;
        inline void operator=(double value);
    };

    extern "C" void convert80x64(const Float80Data *fp80, double *fp64);
    extern "C" void convert64x80(const double *fp64, Float80Data *fp80);

    inline Float80::Float80()
        : data{0, 0, 0, 0, 0}
    {
    }

    inline Float80::Float80(double value)
    {
        convert64x80(&value, &data);
    }

    Float80::operator double() const
    {
        double result;
        convert80x64(&data, &result);
        return result;
    }

    void Float80::operator=(double value)
    {
        convert64x80(&value, &data);
    }

    typedef double Float;

    /* A guest thread's floating-point environment: what the game's runtime reads and sets of the
     * x87 with fstcw, fldcw and fnstsw (its _control87, the math library's checks) -- the control
     * word, and with it the precision the game's arithmetic rounds to, and the status word.  No
     * registers and no stack: the port computes in doubles (fp::, below). */
    struct FloatEnv
    {
        union
        {
            u16 word;
            struct
            {
                u8 im : 1;
                u8 dm : 1;
                u8 zm : 1;
                u8 om : 1;
                u8 um : 1;
                u8 pm : 1;
                u8 unused1 : 2;
                u8 pc : 2;
                u8 rc : 2;
                u8 ic : 1;
                u8 unused2 : 3;
            };
        } control;

        union
        {
            u16 word;
            struct
            {
                u8 ie : 1;
                u8 de : 1;
                u8 ze : 1;
                u8 oe : 1;
                u8 ue : 1;
                u8 pe : 1;
                u8 sf : 1;
                u8 es : 1;
                u8 c0 : 1;
                u8 c1 : 1;
                u8 c2 : 1;
                u8 top : 3;
                u8 c3 : 1;
                u8 b : 1;
            };
        } status;

        /* The word an x87 starts with asks for 64 bits; here it asks for single precision, as
         * the game's races do, so that the menus are in it too and code that reads the word and
         * loads it again (the game's own loading screen does) asks for no other. */
        inline void init()
        {
            status.word = 0;
            setControl(0x7f);
        }

        /* Told of every control word as it is set, when something listens:
         * the live functions log (src/nfs3hp/func_log.cpp) notes each
         * thread's precision.  Null, as it nearly always is, it costs a test
         * at each fldcw. */
        static inline void (*s_controlHook)(u16 word) = nullptr;

        inline void setControl(u16 word)
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
         * as close as the port comes to either. */
        /* Which operation a rounding came from, for the rare rounding that has
         * to know the exact result more closely than the double says. */
        enum class Rounded { Add, Sub, Mul, Div, Sqrt };

        /* What setControl found the control word to ask for. */
        static constexpr nfs::u8 kRoundNone = 0;           // 53 or 64 bits: the double as it is
        static constexpr nfs::u8 kRoundSingleNearest = 1;  // a race
        static constexpr nfs::u8 kRoundSingleOther = 2;    // single, a directed mode
        nfs::u8 rounding = kRoundSingleNearest;

        static constexpr std::uint64_t kSingleDropped = (std::uint64_t(1) << 29) - 1;
        static constexpr std::uint64_t kSingleHalf = std::uint64_t(1) << 28;
        static constexpr std::uint64_t kSingleStep = std::uint64_t(1) << 29;

        static inline int sign(double value)
        {
            return (value > 0) - (value < 0);
        }

        static FLOAT_NOINLINE double roundSingleSlow(double value, Rounded op, double a, double b, unsigned rc)
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

        /* How often, since the start, a compare met a NaN and an fistp had no
         * integer to store -- told apart from the game's normal run by the
         * tick trace ([STEP]). */
        static inline unsigned s_unordered = 0;
        static inline unsigned s_indefinite = 0;

    };


}

/* The game's arithmetic with nothing behind it: no stack, no control word, no
 * status word.  The sums, products, quotients and roots come out as an x87 at
 * single precision has them, to the bit.  Values stay doubles between operations: a float's 24 bits
 * with the x87's wider exponent, and the full width of what sin and the like
 * return, which the x87 does not round either. */
namespace fp
{
    using nfs::Float;

    inline double single(double value, nfs::FloatEnv::Rounded op, double a, double b)
    {
        std::uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        const std::uint64_t exponent = (bits >> 52) & 0x7ff;
        if ((bits & nfs::FloatEnv::kSingleDropped) != nfs::FloatEnv::kSingleHalf && exponent - 0x381 < 0x47e - 0x381)
            return double(float(value));
        if ((bits << 1) == 0)
            return value;  // either zero
        return nfs::FloatEnv::roundSingleSlow(value, op, a, b, 0);
    }

    inline double add(double a, double b) { return single(a + b, nfs::FloatEnv::Rounded::Add, a, b); }
    inline double sub(double a, double b) { return single(a - b, nfs::FloatEnv::Rounded::Sub, a, b); }
    inline double mul(double a, double b) { return single(a * b, nfs::FloatEnv::Rounded::Mul, a, b); }
    inline double div(double a, double b) { return single(a / b, nfs::FloatEnv::Rounded::Div, a, b); }
    inline double sqrt(double a) { return single(::sqrt(a), nfs::FloatEnv::Rounded::Sqrt, a, 0.0); }

    /* fcom: the word fnstsw reads, the x87's c3, c2 and c0. */
    inline nfs::u16 compare(double a, double b)
    {
        if (a == b)
            return 0x4000;
        if (a > b)
            return 0;
        if (a < b)
            return 0x100;
        ++nfs::FloatEnv::s_unordered;
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
            ++nfs::FloatEnv::s_indefinite;
            return std::numeric_limits<Int>::min();
        }
        return Int(rounded);
    }
}

#endif /* !GAME_FLOAT_H_ */
