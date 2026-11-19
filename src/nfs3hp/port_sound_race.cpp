#include "game_settings.h"
#include "port_snd.h"
#include "port_sound_calls.h"

#include <cmath>
#include <cstdint>

/* The race's own sounds that play by themselves, rewritten (port.h says how):
 * the engines' sounds started in their slots as the race begins (the second
 * player's too in the split screen); a far-off effect of the crowd's bank
 * aimed by where the car is and played after a while; an effect of that bank
 * at random; the car bank's effect 15.
 *
 * Each plays through bankEffect (port_sound_effects.cpp) as the game's code
 * calls 0x4182b0. */
namespace nfs3hp
{
namespace port
{

namespace
{

struct Slot
{
    int32_t sound;
    int32_t effect;
};

constexpr Var<int32_t> kFarEffect{0x571118}, kFarTicks{0x552934};
constexpr Table<int32_t> kBankHandles{0x571124, 12};
constexpr Table<Slot> kSlots{0x570de8};
constexpr Table<u32> kCars{0x5efa48};
constexpr Var<double> kUnitsToWhole{0x536b90};

// The banks the code names by their place in kBankHandles.
enum BankIndex : int32_t
{
    kMenuBank = 0,
    kCarBank = 2,
    kCrowdBank = 4,
    kSecondPlayerBank = 22,  // 0x57122c
};

// The pan of the left and right of the split screen.
constexpr uint32_t kLeft = 0x3fff, kRight = 0xc000;

/* fistp: `value` to the nearest integer (even at a tie), the x87's "integer
 * indefinite" past the range. */
int32_t nearestInteger(double value)
{
    const double r = std::nearbyint(value);
    return r >= -2147483648.0 && r <= 2147483647.0 ? int32_t(r) : INT32_MIN;
}

/* A float of the game scaled to a whole number as its code does it. */
int32_t scaled(App* app, u32 at)
{
    return nearestInteger(double(app->getMemory<float>(at)) * kUnitsToWhole(app));
}

int32_t bankHandle(App* app, int32_t i) { return kBankHandles(app, i); }

/* An effect of the game's played at `volume`, `pan`, pitch 0x40; `esp` is the
 * code's after its pushes. */
Result startEffect(App* app, ThreadState& ts, u32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan)
{
    return bankEffect(app, ts, esp - 8, bank, effect, volume, pan, 0x40);
}

/* 0x414ef0's: the far-off effect played, when the race has the crowd, panned
 * at random to its side (more than 7: the effect 10 further on is the
 * same one, behind).  eax as it leaves, the call's handle once it played. */
Result farEffectPlayed(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    if (gameSettings(app).weather == 0)
        return int32_t(eax);
    u32 pan = nextRandom(app) & 0x7fff;
    const int32_t effect = kFarEffect(app);
    if (effect > 7)
    {
        pan += 0x4000;
        kFarEffect.set(app, effect - 0xa);
    }
    else
    {
        pan -= 0x4000;
        if (int32_t(pan) <= 0)
            pan = 0u - pan;
    }
    const int32_t bank = bankHandle(app, kCrowdBank);
    return startEffect(app, ts, esp - 5 * 4, bank, kFarEffect(app), 0x7f, pan);
}

}  // namespace

/* 0x414830's: the engines' sounds as the race starts, where their slots are
 * empty: effects 0 and 1 of the first bank and 0 of the car's bank in slots
 * 0, 2 and 12 -- and the second player's (bank 22, on the right) in 1, 3 and
 * 13 in the split screen.  eax as the code leaves it: 0 after the first
 * slots, the last effect's handle after the second player's, else `eax`. */
Result engineSoundsStarted(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    const u32 frame = esp - 6 * 4;
    const bool split = gameSettings(app).playMode == 1;
    if (kSlots(app, 0).sound == -1)
    {
        const u32 pan = split ? kLeft : 0;
        const int32_t menu = bankHandle(app, kMenuBank), car = bankHandle(app, kCarBank);
        Result r = startEffect(app, ts, frame, menu, 0, 0, pan);
        if (!r)
            return {};
        kSlots.set(app, 0, {*r, 0});
        if (!(r = startEffect(app, ts, frame, menu, 1, 0, pan)))
            return {};
        kSlots.set(app, 2, {*r, 1});
        if (!(r = startEffect(app, ts, frame, car, 0, 0, pan)))
            return {};
        kSlots.set(app, 12, {*r, 0});
        eax = 0;
    }
    if (split && kSlots(app, 1).sound == -1)
    {
        const int32_t second = bankHandle(app, kSecondPlayerBank), car = bankHandle(app, kCarBank);
        Result r = startEffect(app, ts, frame, second, 0, 0, kRight);
        if (!r)
            return {};
        kSlots.set(app, 1, {*r, 0});
        if (!(r = startEffect(app, ts, frame, second, 1, 0, kRight)))
            return {};
        kSlots.set(app, 3, {*r, 1});
        if (!(r = startEffect(app, ts, frame, car, 0, 0, kRight)))
            return {};
        kSlots.set(app, 13, {*r, 0});
        eax = u32(*r);
    }
    return int32_t(eax);
}

/* 0x414830: the engines' sounds as the race starts. */
nfs::u32 engineSoundsStart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, engineSoundsStarted(app, ts, e.esp, e.eax));
}

/* 0x414e10: the far-off effect aimed, if none is: from the player's car
 * (its place at +0x98 and +0xa0, scaled) to the point (eax, edx) -- 6 when
 * far, 7 when near, 10 more when the point is behind -- to play in half the
 * distance's ticks.  eax as the code leaves it: the car's last coordinate
 * scaled. */
nfs::u32 farEffectAim(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    if (kFarTicks(app) != -1)
        return e.back(ts, args, e.eax);
    const u32 car = kCars(app, std::int32_t(gameSettings(app).localPlayer));
    const int32_t acrossFrom = int32_t(e.eax), alongFrom = int32_t(e.edx);
    const int32_t z = scaled(app, car + 0xa0);
    const int32_t dx = scaled(app, car + 0x98) - acrossFrom, dz = z - alongFrom;
    const auto magnitude = [](int32_t v) { return v > 0 ? v : negated(v); };
    int32_t nearer = magnitude(dx), farther = magnitude(dz);
    if (nearer > farther)
        farther >>= 2;
    else
        nearer >>= 2;
    const int32_t sum = int32_t(uint32_t(farther) + uint32_t(nearer));
    // The code's division of the sum by 2^16 toward 0.
    const int32_t distance = sum < 0 ? int32_t((sum + 0xffff) >> 16) : sum >> 16;
    int32_t effect = distance > 0x2ee ? 6 : 7;
    if (z > alongFrom)
        effect += 0xa;
    kFarEffect.set(app, effect);
    kFarTicks.set(app, distance >> 1);
    return e.back(ts, args, u32(z));
}

/* 0x414ef0: the far-off effect played. */
nfs::u32 farEffectPlay(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, farEffectPlayed(app, ts, e.esp, e.eax));
}

/* 0x414f60's: the far-off effect's tick: played when its ticks are up.  `eax`
 * as the code has it. */
Result farEffectTicked(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    int32_t ticks = kFarTicks(app);
    Result r = int32_t(eax);
    if (ticks > 0)
        --ticks;
    else if (ticks == 0)
    {
        if (!(r = farEffectPlayed(app, ts, esp - 2 * 4 - 4, eax)))
            return {};
        ticks = -1;
    }
    kFarTicks.set(app, ticks);
    return r;
}

/* 0x414f60: the far-off effect's tick. */
nfs::u32 farEffectTick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, farEffectTicked(app, ts, e.esp, e.eax));
}

/* 0x414f90: effect edx of the crowd's bank (below 0: one of 4 at random, at a
 * random volume of 0x5a on), at random pan -- the player's side in the split
 * screen, by eax. */
Result crowdEffectOf(App* app, ThreadState& ts, u32 esp, u32 sideWanted, int32_t effect)
{
    const u32 frame = esp - 4 * 5;
    const HeldEbp ebp(ts, frame);
    const bool split = gameSettings(app).playMode == 1;
    u32 side = 0;
    if (split)
        side = sideWanted != 0 ? kRight : kLeft;
    int32_t volume = 0x77;
    if (effect < 0)
    {
        const u32 first = nextRandom(app);
        const u32 second = nextRandom(app);
        effect = int32_t(first & 3);
        volume = int32_t(second % 0x1e) + 0x5a;
    }
    const u32 pan = split ? side : nextRandom(app) % 0xffff;
    return startEffect(app, ts, frame, bankHandle(app, kCrowdBank), effect, volume, pan);
}

nfs::u32 crowdEffect(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, crowdEffectOf(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* 0x415010: the car bank's effect 15 at full volume -- the player's side in
 * the split screen, by eax. */
nfs::u32 carEffectFifteen(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 frame = e.frame(4);
    const HeldEbp ebp(ts, frame);
    const u32 pan = gameSettings(app).playMode == 1 ? (e.eax != 0 ? kRight : kLeft) : 0;
    return e.finish(ts, args, startEffect(app, ts, frame, bankHandle(app, kCarBank), 0xf, 0x7f, pan));
}

}  // namespace port
}  // namespace nfs3hp
