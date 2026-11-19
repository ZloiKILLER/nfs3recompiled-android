#include "game_settings.h"
#include "native_carstate.h"
#include "port_snd_3d.h"
#include "port_sound_calls.h"

#include <algorithm>
#include <array>
#include <cstdint>

/* The race's sound tick, rewritten (port.h says how): the listener put where
 * the player's camera is (0x4229c0) and its move worked out, then each kind of
 * the race's sounds updated in turn -- the cars about, the player's own car,
 * the cop cars' closeness, their sirens and engines -- for each player in the
 * split screen, or from the replay's camera (0x55fe30) where one is used.
 *
 * The game's state: 0x570a78 the listener (its place, then its axes), 0x570ae4
 * and 0x570b34 each player's last place, 0x570af0 and 0x570b40 each player's
 * car, 0x570dcc the listener's direction, 0x55260c the cop cars' closeness,
 * 0x570b78 the cop sources (0x38 bytes each).  The arithmetic is the x87's as
 * the game's code has it (port_snd_3d.h). */
namespace nfs3hp
{
namespace port
{
namespace world
{

namespace
{

#pragma pack(push, 1)
// A cop source (0x570b78 on): the car it follows and what its sounds keep.
struct CopSource
{
    u32 car;             // 0 none, -1 let go
    u32 other4[2];
    int32_t speed;         // the noise of its speed
    u32 other10;
    int32_t nearDistance;  // its distance, kept to 0x15f9
    int32_t volume;
    u32 other1c;
    int32_t ticks;         // a horn: > 0 sounding, < 0 waiting; 0 none
    int32_t soundLength;
    int32_t waitLength;
    int32_t times;
    u32 other30[2];
};
static_assert(sizeof(CopSource) == 0x38, "a cop source");
#pragma pack(pop)

constexpr Records<CopSource> kSources{0x570b78};
constexpr Table<u32> kRoadCars{0x5efac8};
constexpr Var<int32_t> kRoadCarCount{0x5efd9c};
constexpr Var<int32_t> kReplayCamera{0x55fe30}, kCopsCloseness{0x55260c};
constexpr Table<u32> kPlayerCars{0x570af0, 0x50};
constexpr Var<double> kRoadScale{0x536958}, kSpeedScale{0x536950};
constexpr u32 kMoved = 0x570dc0, kReplayListener = 0x7a3d1c, kLastPlaces = 0x570ae4;
// What it calls: the camera (by address, with no typed function yet), the sound rewrites (as C++).
constexpr u32 kCameraView = 0x4229c0, kCopRemove = 0x4121b0, kPlayerCarSound = 0x4117a0;
constexpr u32 kCopsClosenessAt = 0x411e20, kCopDistance = 0x411db0, kNoiseBySpeed = 0x4179d0;
constexpr u32 kNoiseByDistance = 0x417b80, kCopSiren = 0x411c50, kCopSirenStop = 0x411ca0;
constexpr u32 kCopEngine = 0x411cb0, kCopEngineStop = 0x411da0, kRadioTick = 0x4c8810;

int32_t absolute(int32_t v) { return v > 0 ? v : negated(v); }
int32_t sum(int32_t a, int32_t b) { return int32_t(uint32_t(a) + uint32_t(b)); }

/* movsd, `count` times. */
void moveWords(App* app, u32& esi, u32& edi, u32 count)
{
    for (; count > 0; --count, esi += 4, edi += 4)
        guest::put(app, edi, guest::at(app, esi));
}

/* 0x412220: the `count` cop cars nearest the listener (of the cars on the
 * road, 0x5efac8's, the cops' only -- in the split screen those of player
 * `player` -- within 0x4b0000) kept in cop sources `first` on: those no longer
 * near let go, the new ones taken where a source is free.  eax as the code
 * leaves it. */
template <class Real>
Result copSourcesIn(App* app, ThreadState& ts, u32 entryEsp, int32_t player, int32_t first, int32_t count)
{
    // Four pushes, and the nearest pairs and the rest on a frame of 0x54.
    const u32 frame = entryEsp - 4 * 4, below = frame - 0x54;
    struct Near
    {
        u32 car;
        int32_t distance;
    };
    std::array<Near, 4> nearest;
    const int32_t n = std::min(count, 4);
    for (int32_t i = 0; i < n; ++i)
        nearest[i] = {0, 0x4b0000};
    const int32_t cars = kRoadCarCount(app);
    u32 eax = cars > 0 ? u32(cars) : 0;
    for (int32_t i = 0; i < cars; ++i)
    {
        if (n <= 0)
            break;
        const u32 car = kRoadCars(app, i);
        if ((app->getMemory<uint8_t>(car + 0x200) & 4) != 0)
        {
            if (std::int32_t(gameSettings(app).playMode) <= 1)
                continue;
            if (gameSettings(app).localPlayer == guest::at(app, car + 0x1f0))
                continue;
        }
        if ((app->getMemory<uint8_t>(car + 0x200) & 0x40) != 0 || app->getMemory<uint8_t>(car + 0x89) == 0)
            continue;
        const double scale = kRoadScale(app);
        const Vec3 place = vectorAt(app, car + 0x98), listener = vectorAt(app, kListener);
        const int32_t dx = scaledInteger<Real>(Real(place.x) - Real(listener.x), scale);
        const int32_t dy = scaledInteger<Real>(Real(place.y) - Real(listener.y), scale);
        const int32_t dz = scaledInteger<Real>(Real(place.z) - Real(listener.z), scale);
        int32_t x = dx <= 0 ? negated(dx) : dx;
        int32_t y = absolute(dy);
        int32_t z = dz <= 0 ? negated(dz) : dz;
        if (z < x)
            z >>= 2;
        else
            x >>= 2;
        int32_t distance = sum(z, x);
        if (distance >= 0x4b0000)
            continue;
        if (y > distance)
            distance >>= 2;
        else
            y >>= 2;
        distance = sum(distance, y);
        if (distance >= 0x4b0000)
            continue;
        for (int32_t k = 0; k < n; ++k)
        {
            if (distance >= nearest[k].distance)
                continue;
            // Moved down one to make room.
            for (int32_t at = n - 1; at > k; --at)
                nearest[at] = nearest[at - 1];
            nearest[k] = {car, distance};
            break;
        }
    }
    if (n <= 0)
        return eax;
    // Sources whose car is not among the nearest: let go (-1); the nearest a
    // source has already: off the list.
    for (int32_t s = 0; s < n; ++s)
    {
        CopSource& source = kSources(app, first + s);
        if (source.car == 0)
            continue;
        int32_t k = 0;
        for (; k < n; ++k)
            if (nearest[k].car == source.car)
            {
                nearest[k].car = 0;
                break;
            }
        if (k == n)
            source.car = 0xffffffff;
    }
    // The new ones into the free sources.
    for (int32_t k = 0; k < n; ++k)
    {
        if (nearest[k].car == 0)
            continue;
        for (int32_t s = 0; s < n; ++s)
        {
            const u32 held = kSources(app, first + s).car;
            if (held != 0 && held != 0xffffffff)
                continue;
            ts.sp = below;
            if (!guest::direct<kCopRemove, copRemoved>(app, ts, below - 4, u32(first + s), nearest[k].car))
                return {};
            break;
        }
    }
    // The ones let go and not taken: reset.
    for (int32_t s = 0; s < n; ++s)
    {
        if (kSources(app, first + s).car != 0xffffffff)
        {
            eax = u32(s) * 56;
            continue;
        }
        ts.sp = below;
        const Result removed = guest::direct<kCopRemove, copRemoved>(app, ts, below - 4, u32(first + s), 0u);
        if (!removed)
            return {};
        eax = u32(*removed);
    }
    return int32_t(eax);
}

/* 0x411e60: cop source `index`'s sounds for player `player`: from its car's
 * place, angle and distance -- the noise of its speed (by the gap in their
 * +0x7c0 with the player's car, or its own +0x7c4), its knocks and scrapes,
 * and then its siren and radio (a cop), its engine (a racer), or a car's horn
 * now and then.  eax as the code leaves it. */
template <class Real>
Result copSoundIn(App* app, ThreadState& ts, u32 entryEsp, int32_t player, int32_t index)
{
    // Five pushes, and a frame of 0x30.
    const u32 frame = entryEsp - 5 * 4, below = frame - 0x30;
    CopSource& source = kSources(app, index);
    const u32 car = source.car;
    if (car == 0)
        return int32_t(u32(index) * 56);
    const Vec3 place = vectorAt(app, car + 0x98);
    const Result ahead = aheadAlongTrack(app, ts, below - 4, arithmeticFor<Real>(), place, car + 0xa4);
    if (!ahead)
        return {};
    if (*ahead < 0)
        return *ahead;
    int32_t angle;
    if (gameSettings(app).playMode == 1)
        angle = player != 0 ? 0xc000 : 0x3fff;
    else
    {
        const Result r = angleToPlace(app, ts, below - 4, arithmeticFor<Real>(), kListener, place);
        if (!r)
            return {};
        angle = *r;
    }
    ts.sp = below;
    const Result distanceNow =
        guest::direct<kCopDistance, copDistanceSquared>(app, ts, below - 4, arithmeticFor<Real>(), index);
    if (!distanceNow)
        return {};
    const int32_t distance = *distanceNow;
    const int32_t nearDistance = distance < 0x15f9 ? distance : 0x15f9;
    // The noise of its speed: by the gap between the two cars' +0x7c0 when the
    // listener has a direction, else its own +0x7c4.
    float noise;
    if (kListenerDirection(app) != 0)
    {
        const u32 playerCar = kPlayerCars(app, player);
        const float gap = float(Real(floatAt(app, car + 0x7c0)) - Real(floatAt(app, playerCar + 0x7c0)));
        // fldz; fcomp: the gap as it is where 0 is below it (or unordered).
        noise = gap <= 0.0f ? -gap : gap;
    }
    else
        noise = floatAt(app, car + 0x7c4);
    source.speed = scaledInteger<Real>(Real(noise), kSpeedScale(app));
    int32_t topSpeed = int32_t(guest::at(app, car + 0x590));
    if (topSpeed > 0x1770)
        topSpeed = 0x1770;
    const uint8_t flags = app->getMemory<uint8_t>(car + 0x200);
    int32_t volume;
    if ((flags & 0x10) != 0)
        volume = 0x54;
    else
    {
        const int32_t limit = sum(int32_t(guest::at(app, guest::at(app, car + 0x528) + 0x1c4)), 0x7d0);
        volume = limit == 0 ? 0 : idiv(imul(topSpeed, 127), limit);
    }
    volume = volume - 0xa;
    if (volume < 0)
        volume = 0;
    source.nearDistance = nearDistance;
    source.volume = volume;
    ts.sp = below;
    if (!guest::direct<kNoiseBySpeed, noiseBySpeedPlayed>(app, ts, below - 4 - 3 * 4, index + 4,
                                                         (flags & 0x10) != 0 ? 1u : 0u, u32(*ahead), u32(volume),
                                                         distance, u32(angle), source.speed))
        return {};
    // The car's knocks and scrapes, from the last.
    for (int32_t k = int32_t(guest::at(app, car + 0x7dc)) - 1; k >= 0; --k)
    {
        const u32 knock = car + u32(k) * 24;
        const u32 who = int32_t(guest::at(app, knock + 0x7e4)) > -1 ? u32(index) + 0x16 : 0xffffffff;
        ts.sp = below;
        if (!guest::direct<kNoiseByDistance, noiseByDistancePlayed>(
                app, ts, below - 4 - 2 * 4, int32_t(who), guest::at(app, knock + 0x7e8), guest::at(app, knock + 0x7ec),
                guest::at(app, knock + 0x7f0), distance, u32(angle)))
            return {};
    }
    const u32 current = source.car;
    const uint8_t kind = app->getMemory<uint8_t>(current + 0x200);
    // The engine's (a cop's) sound and its stop, as C++.
    const auto engine = [&]() {
        ts.sp = below;
        return guest::direct<kCopEngine, copEnginePlayed>(app, ts, below - 4, index, distance, *ahead, u32(angle));
    };
    const auto engineStop = [&]() {
        ts.sp = below;
        return guest::direct<kCopEngineStop, copEngineStopped>(app, ts, below - 4, index);
    };
    u32 r;
    if ((kind & 0x20) != 0)
    {
        if ((app->getMemory<uint8_t>(current + 0x710) & 0x10) != 0)
        {
            ts.sp = below;
            if (!guest::direct<kCopSiren, copSirenPlayed>(app, ts, below - 4, index, distance, u32(angle), *ahead))
                return {};
            source.ticks = 1;
        }
        else if (source.ticks != 0)
        {
            ts.sp = below;
            if (!guest::direct<kCopSirenStop, copSirenStopped>(app, ts, below - 4, index))
                return {};
            source.ticks = 0;
        }
        ts.sp = below;
        return guest::direct<kRadioTick, radioSoundTicked>(app, ts, below - 4 - 4, car, u32(angle), *ahead, distance,
                                                           player);
    }
    if ((kind & 4) != 0)
    {
        r = current;
        if (app->getMemory<uint8_t>(car + 0x50e) != 0)
        {
            const Result played = engine();
            if (!played)
                return {};
            r = u32(*played);
            source.ticks = 1;
        }
        else if (source.ticks != 0)
        {
            const Result stopped = engineStop();
            if (!stopped)
                return {};
            r = u32(*stopped);
            source.ticks = 0;
        }
        return int32_t(r);
    }
    // A car's horn: sounding while it counts down, then waiting.
    const int32_t ticks = source.ticks;
    if (ticks > 0)
    {
        if (source.soundLength > 0)
            source.ticks = ticks - 1;
        if (source.ticks == 0)
        {
            app->getMemory<uint8_t>(car + 0x50e) = 0;
            const Result stopped = engineStop();
            if (!stopped)
                return {};
            r = u32(*stopped);
            const int32_t left = source.times - 1;
            source.times = left;
            if (left != 0)
            {
                source.ticks = source.waitLength;
                return source.waitLength;
            }
            return int32_t(r);
        }
        app->getMemory<uint8_t>(car + 0x50e) = 1;
        return engine();
    }
    if (ticks < 0)
    {
        source.ticks = ticks + 1;
        if (source.ticks == 0)
        {
            source.ticks = source.soundLength;
            return source.soundLength;
        }
    }
    return int32_t(current);
}

}  // namespace

Result copSourcesNearest(App* app, ThreadState& ts, u32 esp, Arithmetic a, int32_t player, int32_t first, int32_t count)
{
    return withArithmetic(a, [&](auto real) { return copSourcesIn<decltype(real)>(app, ts, esp, player, first, count); });
}

Result copSourceSound(App* app, ThreadState& ts, u32 esp, Arithmetic a, int32_t player, int32_t index)
{
    return withArithmetic(a, [&](auto real) { return copSoundIn<decltype(real)>(app, ts, esp, player, index); });
}

namespace
{

/* The listener at player `player`'s camera, and its move since; its direction
 * that of the car at `carSlot`'s (+0xa4), or the moved one's with the camera
 * off it (0x6fd3a0 2 and on). */
bool listenerAt(Calls& game, Arithmetic a, int32_t player, u32 carSlot)
{
    App* app = game.app;
    game.atCall();
    guest::direct<kCameraView, carstate::viewCopied>(app, game.ts, u32(player), kListener);
    const u32 car = guest::at(app, carSlot);
    moveListener(app, a, player, kListener);
    guest::put(app, u32(kListenerDirection.at), std::int32_t(gameSettings(app).replayMode) >= 2 ? kMoved : car + 0xa4);
    return true;
}

}  // namespace

}  // namespace world

using namespace world;

/* 0x412220: the ebx cop cars nearest the listener kept in cop sources edx on,
 * for player eax. */
nfs::u32 copSourcesPick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, copSourcesNearest(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), int32_t(e.eax), int32_t(e.edx),
                                    int32_t(e.ebx)));
    return args.eax;
}

/* 0x411e60: cop source edx's sounds for player eax. */
nfs::u32 copCarSound(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, copSourceSound(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), int32_t(e.eax), int32_t(e.edx)));
}

/* 0x411410: the race's sound tick. */
nfs::u32 raceSoundTick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Arithmetic a = arithmeticOf(ts.fenv.rounding);
    // Six pushes, no frame: the code's esp and ebp both.
    const u32 frame = e.frame(6);
    const HeldEbp ebp(ts, frame);
    Calls game{app, ts, frame};
    // The code's esi and edi, which its moves of the listener's place copy with.
    u32 esi = e.esi, edi = e.edi;
    const auto done = [&](const Result& r) { return e.finish(ts, args, r); };
    const auto cops = [&](int32_t player, int32_t first, int32_t count) {
        return copSourcesNearest(app, ts, frame - 4, a, player, first, count);
    };
    const auto track = [&](int32_t player) { return trackSoundsFor(app, ts, frame - 4, a, player); };
    const auto closeness = [&](int32_t count, int32_t from) {
        ts.sp = frame;
        const Result r = guest::direct<kCopsClosenessAt, copsClosenessTold>(app, ts, frame - 4, a, count, from);
        if (!r)
            return false;
        kCopsCloseness.set(app, *r);
        return true;
    };
    const auto playerCar = [&](u32 player) {
        ts.sp = frame;
        return bool(guest::direct<kPlayerCarSound, playerCarSounded>(app, ts, frame - 4, a, player));
    };
    Result r;
    if (gameSettings(app).playMode == 1)
    {
        if (!listenerAt(game, a, 0, 0x570af0) || !cops(0, 0, 1))
            return args.eax;
        edi = kLastPlaces;
        if (!track(0) || !playerCar(0))
            return args.eax;
        esi = kListener;
        if (!closeness(1, 0))
            return args.eax;
        if (!(r = copSourceSound(app, ts, frame - 4, a, 0, 0)))
            return args.eax;
        // The listener kept as player 0's last place.
        moveWords(app, esi, edi, 3);
        if (!listenerAt(game, a, 1, 0x570b40))
            return args.eax;
        if (!playerCar(1))
            return args.eax;
        if (!track(1) || !cops(1, 1, 1))
            return args.eax;
        edi = 0x570b34;
        if (!closeness(1, 1))
            return args.eax;
        esi = kListener;
        if (!(r = copSourceSound(app, ts, frame - 4, a, 1, 1)))
            return args.eax;
        moveWords(app, esi, edi, 3);
        return done(r);
    }
    if (kReplayCamera(app) == 0)
    {
        if (!listenerAt(game, a, 0, 0x570af0))
            return args.eax;
    }
    else
    {
        esi = kReplayListener;
        edi = kListener;
        moveWords(app, esi, edi, 0x18);
        edi = 0;
        kListenerDirection.set(app, 0);
    }
    if (!cops(0, 0, 4) || !track(0) || !closeness(4, 0) || !playerCar(0))
        return args.eax;
    for (int32_t cop = 0; cop < 4; ++cop)
    {
        if (cop == 2)
            edi = kLastPlaces;
        if (cop == 3)
            esi = kListener;
        if (!(r = copSourceSound(app, ts, frame - 4, a, 0, cop)))
            return args.eax;
    }
    moveWords(app, esi, edi, 3);
    done(r);
    return args.eax;
}

}  // namespace port
}  // namespace nfs3hp
