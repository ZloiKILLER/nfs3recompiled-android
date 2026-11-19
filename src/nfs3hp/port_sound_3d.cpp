#include "game_settings.h"
#include "native_frontend_simd.h"
#include "port_snd_3d.h"
#include "port_sound_effects.h"

/* The race's sound in space, rewritten (port.h says how): where a sound's
 * source is from the listener -- its distance (the code's cheap one, the
 * largest of three plus a quarter of the others) and its angle -- how far the
 * listener moved since the last frame, and how far along the track a place is.
 * The arithmetic is the x87's as the game's code has it (port_snd_3d.h): the
 * float type of the race's precision, or double.
 *
 * The game's state: 0x570a60 each listener's last place (3 floats, 12 bytes
 * apart), 0x570dc0 its move since, scaled; 0x6fd4c4 the mirror (the angle's
 * side turned). */
namespace nfs3hp
{
namespace port
{
namespace world
{

namespace
{

constexpr Var<float> kMoveScale{0x536910};
constexpr Var<double> kDistanceScale{0x536918}, kAngleScale{0x536920}, kCarAngleScale{0x536928};
constexpr Var<double> kAheadScale{0x536960}, kCellUnit{0x536968};
constexpr Var<double> kAcrossScale{0x536970}, kAcrossUnit{0x536978};
constexpr u32 kLastPlaces = 0x570a60, kMoved = 0x570dc0;
constexpr u32 kAngle = 0x4e8f28;      // SNDangle (port_sound_effects.cpp's vectorAngle)

int32_t sum(int32_t a, int32_t b) { return int32_t(uint32_t(a) + uint32_t(b)); }

template <class Real>
void moveListenerIn(App* app, int32_t listener, u32 place)
{
    // The code's index: listener * 3, scaled by the floats' size.
    const u32 last = kLastPlaces + u32(listener) * 12;
    const Real scale = Real(kMoveScale(app));
    const Real dx = (Real(floatAt(app, place)) - Real(floatAt(app, last))) * scale;
    const Real dy = (Real(floatAt(app, place + 4)) - Real(floatAt(app, last + 4))) * scale;
    const Real dz = scale * (Real(floatAt(app, place + 8)) - Real(floatAt(app, last + 8)));
    // The place kept (as words, the code's moves) around the move's stores.
    guest::put(app, last, guest::at(app, place));
    guest::put(app, last + 4, guest::at(app, place + 4));
    app->getMemory<float>(kMoved) = float(dx);
    app->getMemory<float>(kMoved + 4) = float(dy);
    app->getMemory<float>(kMoved + 8) = float(dz);
    guest::put(app, last + 8, guest::at(app, place + 8));
}

template <class Real>
int32_t distanceIn(App* app, const Vec3& from, const Vec3& to)
{
    const double scale = kDistanceScale(app);
    int32_t x = scaledInteger<Real>(Real(to.x) - Real(from.x), scale);
    int32_t y = scaledInteger<Real>(Real(to.y) - Real(from.y), scale);
    int32_t z = scaledInteger<Real>(Real(to.z) - Real(from.z), scale);
    if (x <= 0)
        x = negated(x);
    y = y > 0 ? y : negated(y);
    if (z <= 0)
        z = negated(z);
    // The largest of the three plus a quarter of each of the others, twice
    // over: the larger of x and z, then of that sum and y.
    if (x > z)
        z >>= 2;
    else
        x >>= 2;
    z = sum(z, x);
    if (y > z)
        z >>= 2;
    else
        y >>= 2;
    return sum(y, z);
}

/* The dot product's result, on the x87's stack as the game's code has it,
 * scaled to a whole number: fmul; fistp. */
template <class Real>
int32_t scaledTop(double top, double scale)
{
    return nearestInteger(double(times<Real>(top, scale)));
}

/* An angle from the listener's axes: the vector at `vector` projected on the
 * axes at +0xc and +0x24 (scaled by `scale`), the first turned in the mirror,
 * each shifted down by 8 and SNDangle'd, as a 16-bit turn.  `esp` is the
 * code's at its calls. */
template <class Real>
Result angleFromAxes(App* app, ThreadState& ts, u32 esp, u32 listener, u32 vector, double scale)
{
    double top;
    if (!gameDot(app, ts, esp, vector, listener + 0xc, top))
        return {};
    int32_t first = scaledTop<Real>(top, scale);
    if (!gameDot(app, ts, esp, vector, listener + 0x24, top))
        return {};
    const int32_t second = scaledTop<Real>(top, scale);
    if (gameSettings(app).mirrored != 0)
        first = negated(first);
    ts.sp = esp;
    const u32 angle = guest::direct<kAngle, vectorAngle>(app, ts, first >> 8, second >> 8);
    if (ts.terminate)
        return {};
    return int32_t((angle << 6) & 0xffff);
}

template <class Real>
Result angleToPlaceIn(App* app, ThreadState& ts, u32 esp, u32 listener, const Vec3& place)
{
    const Vec3 from = vectorAt(app, listener);
    // Four pushes and a frame of 0xc, where the difference is kept as floats.
    const u32 frame = esp - 4 * 4, below = frame - 0xc;
    store<float>(app, below, float(Real(place.x) - Real(from.x)));
    store<float>(app, below + 4, float(Real(place.y) - Real(from.y)));
    store<float>(app, below + 8, float(Real(place.z) - Real(from.z)));
    return angleFromAxes<Real>(app, ts, below, listener, below, kAngleScale(app));
}

template <class Real>
Result aheadIn(App* app, ThreadState& ts, u32 esp, const Vec3& place, u32 record)
{
    // Three pushes, a frame of 0x28: the cell's three ints at ebp - 0x1c, the
    // place of the cell at ebp - 0x10, the track's direction at ebp - 0x28.
    const u32 ebp = esp - 3 * 4, below = ebp - 0x28, cellAt = ebp - 0x1c, vectorOnStack = ebp - 0x10;
    const Vec3 listener = vectorAt(app, kListener);
    const double scale = kAheadScale(app);
    const int32_t cell[3] = {scaledInteger<Real>(Real(place.x) - Real(listener.x), scale) >> 8,
                             scaledInteger<Real>(Real(place.y) - Real(listener.y), scale) >> 8,
                             scaledInteger<Real>(Real(place.z) - Real(listener.z), scale) >> 8};
    // The listener's direction as the code read it, before the cell's lookup.
    const u32 direction = kListenerDirection(app);
    for (int k = 0; k < 3; ++k)
        store<int32_t>(app, cellAt + 4 * k, cell[k]);
    // (0x4970c0, simd's fixedNormalise, which reads only the vector in eax.)
    ts.sp = below;
    guest::direct<0x4970c0, frontend::simd::fixedNormalise>(app, ts, cellAt);
    if (ts.terminate)
        return {};
    // The cell as a place again, in the track's units, kept as floats.
    const double unit = kCellUnit(app);
    const auto storeCellPlace = [&] {
        for (int k = 0; k < 3; ++k)
            store<float>(app, vectorOnStack + 4 * k, float(times<Real>(double(cell[k]), unit)));
    };
    int32_t ahead = 0;
    double top;
    if (record != 0)
    {
        storeCellPlace();
        for (u32 k = 0; k < 12; k += 4)
            guest::put(app, ebp - 0x28 + k, guest::at(app, record + k));
        if (!gameDot(app, ts, below, vectorOnStack, ebp - 0x28, top))
            return {};
        ahead = scaledTop<Real>(top, scale);
    }
    if (direction != 0)
    {
        storeCellPlace();
        if (!gameDot(app, ts, below, vectorOnStack, direction, top))
            return {};
        ahead = sum(ahead, negated(scaledTop<Real>(top, scale)));
    }
    return fixedQuotient(0xdc0000, sum(ahead, 0xdc0000));
}

/* The clamp the code gives an across: a 16.16 -1..1. */
int32_t clampedAcross(int32_t v) { return v > 0x10000 ? 0x10000 : v < -0x10000 ? -0x10000 : v; }

template <class Real>
Result acrossIn(App* app, ThreadState& ts, u32 esp, u32 listener, u32 place, u32 across)
{
    // Four pushes and a frame of 0x1c, where the place from the listener is kept
    // as floats.
    const u32 below = esp - 4 * 4 - 0x1c;
    const double scale = kAcrossScale(app);
    const Vec3 at = vectorAt(app, listener);
    const int32_t whole[3] = {int32_t(guest::at(app, place)), int32_t(guest::at(app, place + 4)),
                              int32_t(guest::at(app, place + 8))};
    // The place from the listener in whole units, and the cheap distance.
    int32_t x = sum(whole[0], negated(scaledInteger<Real>(Real(at.x), scale)));
    int32_t y = sum(whole[1], negated(scaledInteger<Real>(Real(at.y), scale)));
    int32_t z = sum(whole[2], negated(scaledInteger<Real>(Real(at.z), scale)));
    x = x > 0 ? x : negated(x);
    int32_t ay = y > 0 ? y : negated(y);
    if (z <= 0)
        z = negated(z);
    if (z < x)
        z >>= 2;
    else
        x >>= 2;
    z = sum(z, x);
    if (ay > z)
        z >>= 2;
    else
        ay >>= 2;
    const int32_t distance = sum(z, ay);
    // The place less the listener's, in floats, on the axes.
    const double unit = kAcrossUnit(app);
    store<float>(app, below, float(times<Real>(double(whole[0]), unit) - Real(at.x)));
    store<float>(app, below + 4, float(times<Real>(double(whole[1]), unit) - Real(at.y)));
    store<float>(app, below + 8, float(times<Real>(double(whole[2]), unit) - Real(at.z)));
    double top;
    if (!gameDot(app, ts, below, below, listener + 0xc, top))
        return {};
    const int32_t first = scaledTop<Real>(top, scale);
    if (!gameDot(app, ts, below, below, listener + 0x24, top))
        return {};
    const int32_t second = scaledTop<Real>(top, scale);
    if (distance == 0)
    {
        guest::put(app, across + 4, 0);
        guest::put(app, across, 0);
    }
    else
    {
        guest::put(app, across, u32(clampedAcross(fixedQuotient(first, distance))));
        guest::put(app, across + 4, u32(clampedAcross(fixedQuotient(second, distance))));
    }
    return distance;
}

}  // namespace

void moveListener(App* app, Arithmetic a, int32_t listener, u32 place)
{
    withArithmetic(a, [&](auto real) { moveListenerIn<decltype(real)>(app, listener, place); });
}

int32_t distanceBetween(App* app, Arithmetic a, const Vec3& from, const Vec3& to)
{
    return withArithmetic(a, [&](auto real) { return distanceIn<decltype(real)>(app, from, to); });
}

Result angleToPlace(App* app, ThreadState& ts, u32 esp, Arithmetic a, u32 listener, const Vec3& place)
{
    return withArithmetic(a, [&](auto real) {
        return angleToPlaceIn<decltype(real)>(app, ts, esp, listener, place);
    });
}

Result aheadAlongTrack(App* app, ThreadState& ts, u32 esp, Arithmetic a, const Vec3& place, u32 direction)
{
    return withArithmetic(a, [&](auto real) { return aheadIn<decltype(real)>(app, ts, esp, place, direction); });
}

Result distanceAt(App* app, Arithmetic a, u32 from, u32 to)
{
    return distanceBetween(app, a, vectorAt(app, from), vectorAt(app, to));
}

Result angleAt(App* app, ThreadState& ts, u32 esp, Arithmetic a, u32 listener, u32 place)
{
    return angleToPlace(app, ts, esp, a, listener, vectorAt(app, place));
}

Result carAngleAt(App* app, ThreadState& ts, u32 esp, Arithmetic a, u32 listener, u32 car)
{
    // Five pushes, no frame.
    return withArithmetic(a, [&](auto real) {
        return angleFromAxes<decltype(real)>(app, ts, esp - 5 * 4, listener, car + 0x13c, kCarAngleScale(app));
    });
}

Result aheadAt(App* app, ThreadState& ts, u32 esp, Arithmetic a, u32 place, u32 direction)
{
    return aheadAlongTrack(app, ts, esp, a, vectorAt(app, place), direction);
}

}  // namespace world

using namespace world;

/* The game's entries -- each its thunk around the function above. */

/* 0x410c40: listener eax's move to the place at edx.  eax left * 3. */
nfs::u32 listenerMove(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    moveListener(app, arithmeticOf(ts.fenv.rounding), int32_t(e.eax), e.edx);
    return e.back(ts, args, e.eax * 3);
}

/* 0x410cb0: the distance from the place at eax to the one at edx. */
nfs::u32 sourceDistance(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, distanceAt(app, arithmeticOf(ts.fenv.rounding), e.eax, e.edx));
}

/* 0x410d30: the angle of the place at edx from listener eax. */
nfs::u32 sourceAngle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, angleAt(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), e.eax, e.edx));
}

/* 0x410dc0: the angle of the car at edx (its direction at +0x13c) from
 * listener eax. */
nfs::u32 carAngle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, carAngleAt(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), e.eax, e.edx));
}

/* 0x4124d0: how far the place at eax is ahead along the track. */
nfs::u32 placeAhead(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, aheadAt(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), e.eax, e.edx));
}

/* 0x412620: the place at edx (ints) from the listener at eax: its distance in
 * eax, and across each axis over that (16.16, -1..1) at ebx and ebx + 4. */
nfs::u32 placeAcross(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, placeAcrossOf(app, ts, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x412620's core: placeAcross for the listener, the place and the pair `across` is written to; esp the
 * frame base. */
Result placeAcrossOf(App* app, ThreadState& ts, u32 esp, u32 listener, u32 place, u32 across)
{
    return withArithmetic(arithmeticOf(ts.fenv.rounding),
                          [&](auto real) { return acrossIn<decltype(real)>(app, ts, esp, listener, place, across); });
}

}  // namespace port
}  // namespace nfs3hp
