#ifndef NFS3HP_PORT_SND_3D_H
#define NFS3HP_PORT_SND_3D_H

#include "port_snd.h"

#include <cmath>
#include <cstdint>
#include <cstring>

/* What the game's world sounds (port_sound_3d.cpp, _tick, _ambient and
 * _scenery) share: the race's arithmetic on floats, and the sound's space --
 * the listener, how far and at what angle a source is from it.
 *
 * The game's code does this arithmetic on the x87, whose precision control the
 * race sets to 24 bits: each sum, difference and product is rounded to a
 * float's significand, a product of a float and a double constant included
 * (the exact product rounded once).  Out of a race the control word asks for
 * more, and the port's x87 then works in double.  The functions here are
 * written once for either, as templates on the type the arithmetic is done in
 * (float or double), and entered with the Arithmetic that the control word at
 * the call asks for -- the race's, in the game; both, in the checks. */
namespace nfs3hp
{
namespace port
{
// The sound rewrites this group calls, as the game's code calls their addresses
// (port_sound_effects.cpp, port_snd_voices.cpp): 0x4182b0, 0x417080, 0x416f60,
// 0x416fc0, 0x4e8f28, 0x4e8fd0, 0x4e9420 and 0x4e95fc.
void playEffect(App* app, CPU& cpu);
void slotPlay(App* app, CPU& cpu);
void slotStop(App* app, CPU& cpu);
void slotEffect(App* app, CPU& cpu);
void SNDangle(App* app, CPU& cpu);
void SNDstatus(App* app, CPU& cpu);
void SNDautovol(App* app, CPU& cpu);
void SNDvol(App* app, CPU& cpu);

namespace world
{

enum class Arithmetic
{
    Double,
    Single
};

inline Arithmetic arithmeticOf(const CPU& cpu)
{
    return cpu.fpu.rounding == x86::FPU::kRoundNone ? Arithmetic::Double : Arithmetic::Single;
}

template <class Real>
constexpr Arithmetic arithmeticFor()
{
    return sizeof(Real) == sizeof(float) ? Arithmetic::Single : Arithmetic::Double;
}

/* Calls f(Real()) with the type the arithmetic is done in. */
template <class F>
auto withArithmetic(Arithmetic a, F&& f)
{
    return a == Arithmetic::Single ? f(float()) : f(double());
}

/* fistp: `value` to the nearest integer (ties to even), the x87's "integer
 * indefinite" past the range. */
inline int32_t nearestInteger(double value)
{
    const double r = std::nearbyint(value);
    return r >= -2147483648.0 && r <= 2147483647.0 ? int32_t(r) : INT32_MIN;
}

/* `a` times the double `c`, rounded to the precision of Real.  A float's is
 * the x87's single-precision product: the exact one rounded once, which the
 * product in double (rounded already) can get wrong only where it lies exactly
 * half way between two floats -- then the error the fma finds says which way
 * the exact product went. */
template <class Real>
Real times(double a, double c);

template <>
inline double times<double>(double a, double c)
{
    return a * c;
}

template <>
inline float times<float>(double a, double c)
{
    double product = a * c;
    const double lost = std::fma(a, c, -product);
    std::uint64_t bits;
    std::memcpy(&bits, &product, sizeof bits);
    if (lost != 0 && (bits & 0x1fffffff) == 0x10000000)
        product = std::nextafter(product, lost > 0 ? HUGE_VAL : -HUGE_VAL);
    return float(product);
}

/* `value` times the double `scale`, to the nearest integer: fmul; fistp. */
template <class Real>
int32_t scaledInteger(Real value, double scale)
{
    return nearestInteger(double(times<Real>(double(value), scale)));
}

/* A vector of floats. */
struct Vec3
{
    float x, y, z;
};

inline float floatAt(App* app, reg32 at) { return app->getMemory<float>(at); }

inline Vec3 vectorAt(App* app, reg32 at) { return {floatAt(app, at), floatAt(app, at + 4), floatAt(app, at + 8)}; }

/* The game's dot product of the floats at `a` and `b` (0x4e01f0, vertices'),
 * not rewritten yet: called as the game's code does, and its result -- left on
 * the x87's stack -- taken from there.  The double is the x87's value as the
 * dot left it (a float's in the race). */
inline bool gameDot(App* app, CPU& cpu, reg32 esp, reg32 a, reg32 b, double& result)
{
    if (!callGame(app, cpu, esp, 0x4e01f0, {a, b}))
        return false;
    result = double(cpu.fpu.st(0));
    cpu.fpu.pop();
    return true;
}

/* The code's 16.16 division -- cdq; idiv; the remainder shifted up 16 and
 * divided again; the quotients added -- of `n` by `by`. */
inline int32_t fixedQuotient(int32_t n, int32_t by)
{
    if (by == 0)
        return 0;
    const int32_t whole = idiv(n, by);
    const int64_t rest = by == -1 ? 0 : int64_t(n % by);
    const int32_t fraction = int32_t((rest * 0x10000) / by);
    return int32_t(uint32_t(fraction) + (uint32_t(whole) << 16));
}

/* A call of the game's code from a rewrite whose original keeps esi and edi
 * in use: they are what the original has there, for the game's code below to
 * read where it reads what it never set (port.h).  `esp` is the code's before
 * the call.  The registers of `Args` are the original's as well, all four:
 * a stand-in of the checks may give back the eax it was called with. */
struct Calls
{
    App* app;
    CPU& cpu;
    reg32 esp;
    reg32 esi, edi;
    bool operator()(reg32 target, Args a, std::initializer_list<reg32> stack, reg32& result)
    {
        cpu.esi = esi;
        cpu.edi = edi;
        return callGame(app, cpu, esp, target, a, stack, result);
    }
    bool operator()(reg32 target, Args a, std::initializer_list<reg32> stack = {})
    {
        reg32 ignored;
        return (*this)(target, a, stack, ignored);
    }
    // Another rewrite's adapter (port_sound_effects.cpp, port_snd_voices.cpp), as
    // the game's code calls its address: run as C++, so that the checks hold it
    // against the game's own code and not against a stand-in.
    bool operator()(guest::Native target, Args a, std::initializer_list<reg32> stack, reg32& result)
    {
        cpu.esi = esi;
        cpu.edi = edi;
        return callGame(app, cpu, esp, target, a, stack, result);
    }
    bool operator()(guest::Native target, Args a, std::initializer_list<reg32> stack = {})
    {
        reg32 ignored;
        return (*this)(target, a, stack, ignored);
    }
};

// The game's state the sound's space is in.
constexpr reg32 kListener = 0x570a78;      // the listener: its place, then its axes (+0xc, +0x24)
constexpr Var<int32_t> kMirror{0x6fd4c4};  // the angle's side turned
constexpr Var<reg32> kListenerDirection{0x570dcc};

/* The sound's space (port_sound_3d.cpp).  `esp` is the game's code's on entering
 * the function. */

/* 0x410c40: listener `listener`'s move from its last place to the one at
 * `place`, scaled and kept at 0x570dc0; the place kept. */
void moveListener(App* app, Arithmetic a, int32_t listener, reg32 place);

/* 0x410cb0: the distance of `to` from `from`: each axis's difference scaled
 * and the largest of them plus a quarter of the others, as the code goes about
 * it. */
int32_t distanceBetween(App* app, Arithmetic a, const Vec3& from, const Vec3& to);

/* 0x410d30: the angle of `place` from the listener at `listener` (its place,
 * then its axes at +0xc and +0x24), a 16-bit turn. */
Result angleToPlace(App* app, CPU& cpu, reg32 esp, Arithmetic a, reg32 listener, const Vec3& place);

/* 0x4124d0: how far `place` is ahead (16.16, of 0xdc0000) along the track: its
 * cell (0x4970c0's) by the direction at `direction` (3 floats; none if 0),
 * less the listener's own. */
Result aheadAlongTrack(App* app, CPU& cpu, reg32 esp, Arithmetic a, const Vec3& place, reg32 direction);

/* 0x410e30: player `player`'s track sounds, esi and edi as the code has them on
 * entering (port_sound_ambient.cpp). */
Result trackSoundsFor(App* app, CPU& cpu, reg32 esp, reg32 esi, reg32 edi, Arithmetic a, int32_t player);

}  // namespace world
}  // namespace port
}  // namespace nfs3hp

#endif
