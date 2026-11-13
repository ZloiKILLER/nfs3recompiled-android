#include "game_settings.h"
#include "port_snd.h"

#include <cmath>
#include <cstdint>

/* The race's own sounds that play by themselves, rewritten (port.h says how):
 * the engines' sounds started in their slots as the race begins (the second
 * player's too in the split screen); a far-off effect of the crowd's bank
 * aimed by where the car is and played after a while; an effect of that bank
 * at random; the car bank's effect 15.
 *
 * Each plays through bankEffect (port_sound_effects.cpp) as the game's code
 * calls 0x4182b0.  That code reads esi and edi, which its callers here set
 * (a pan, a flag) for their own use and nobody for it, so these rewrites set
 * them the same way before each effect. */
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
constexpr Table<reg32> kCars{0x5efa48};
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
int32_t scaled(App* app, reg32 at)
{
    return nearestInteger(double(app->getMemory<float>(at)) * kUnitsToWhole(app));
}

int32_t bankHandle(App* app, int32_t i) { return kBankHandles(app, i); }

/* An effect of the game's played at `volume`, `pan`, pitch 0x40, with esi and
 * edi as the code that plays it has them; `esp` is the code's after its
 * pushes. */
Result startEffect(App* app, CPU& cpu, reg32 esp, reg32 esi, reg32 edi, int32_t bank, int32_t effect,
                   int32_t volume, uint32_t pan)
{
    cpu.esi = esi;
    cpu.edi = edi;
    return bankEffect(app, cpu, esp - 8, bank, effect, volume, pan, 0x40);
}

/* 0x414ef0's: the far-off effect played, when the race has the crowd, panned
 * at random to its side (more than 7: the effect 10 further on is the
 * same one, behind).  eax as it leaves, the call's handle once it played. */
Result farEffectPlayed(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const HeldEbp ebp(cpu, esp - 5 * 4);
    if (gameSettings(app).weather == 0)
        return int32_t(eax);
    reg32 pan = nextRandom(app) & 0x7fff;
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
    return startEffect(app, cpu, esp - 5 * 4, reg32(bank), cpu.edi, bank, kFarEffect(app), 0x7f, pan);
}

}  // namespace

/* 0x414830: the engines' sounds as the race starts, where their slots are
 * empty: effects 0 and 1 of the first bank and 0 of the car's bank in slots
 * 0, 2 and 12 -- and the second player's (bank 22, on the right) in 1, 3 and
 * 13 in the split screen.  eax as the code leaves it: 0 after the first
 * slots, the last effect's handle after the second player's. */
void engineSoundsStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(6);
    const HeldEbp ebp(cpu, frame);
    reg32 eax = e.eax, edi = e.edi;
    const bool split = gameSettings(app).playMode == 1;
    if (kSlots(app, 0).sound == -1)
    {
        const reg32 pan = split ? kLeft : 0;
        const int32_t menu = bankHandle(app, kMenuBank), car = bankHandle(app, kCarBank);
        Result r = startEffect(app, cpu, frame, pan, edi, menu, 0, 0, pan);
        if (!r)
            return;
        kSlots.set(app, 0, {*r, 0});
        edi = 1;
        if (!(r = startEffect(app, cpu, frame, pan, edi, menu, 1, 0, pan)))
            return;
        kSlots.set(app, 2, {*r, 1});
        if (!(r = startEffect(app, cpu, frame, pan, edi, car, 0, 0, pan)))
            return;
        kSlots.set(app, 12, {*r, 0});
        eax = 0;
    }
    if (split && kSlots(app, 1).sound == -1)
    {
        const int32_t second = bankHandle(app, kSecondPlayerBank), car = bankHandle(app, kCarBank);
        Result r = startEffect(app, cpu, frame, 1, edi, second, 0, 0, kRight);
        if (!r)
            return;
        kSlots.set(app, 1, {*r, 0});
        edi = 0;
        if (!(r = startEffect(app, cpu, frame, 1, edi, second, 1, 0, kRight)))
            return;
        kSlots.set(app, 3, {*r, 1});
        if (!(r = startEffect(app, cpu, frame, 1, edi, car, 0, 0, kRight)))
            return;
        kSlots.set(app, 13, {*r, 0});
        eax = reg32(*r);
    }
    e.back(cpu, eax);
}

/* 0x414e10: the far-off effect aimed, if none is: from the player's car
 * (its place at +0x98 and +0xa0, scaled) to the point (eax, edx) -- 6 when
 * far, 7 when near, 10 more when the point is behind -- to play in half the
 * distance's ticks.  eax as the code leaves it: the car's last coordinate
 * scaled. */
void farEffectAim(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (kFarTicks(app) != -1)
        return e.back(cpu, e.eax);
    const reg32 car = kCars(app, std::int32_t(gameSettings(app).localPlayer));
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
    e.back(cpu, reg32(z));
}

/* 0x414ef0: the far-off effect played. */
void farEffectPlay(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, farEffectPlayed(app, cpu, e.esp, e.eax));
}

/* 0x414f60: the far-off effect's tick: played when its ticks are up. */
void farEffectTick(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const HeldEbp ebp(cpu, e.frame(2));
    int32_t ticks = kFarTicks(app);
    Result r = int32_t(e.eax);
    if (ticks > 0)
        --ticks;
    else if (ticks == 0)
    {
        if (!(r = farEffectPlayed(app, cpu, e.frame(2) - 4, e.eax)))
            return;
        ticks = -1;
    }
    kFarTicks.set(app, ticks);
    e.finish(cpu, r);
}

/* 0x414f90: effect edx of the crowd's bank (below 0: one of 4 at random, at a
 * random volume of 0x5a on), at random pan -- the player's side in the split
 * screen, by eax. */
void crowdEffect(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(5);
    const HeldEbp ebp(cpu, frame);
    int32_t effect = int32_t(e.edx);
    const bool split = gameSettings(app).playMode == 1;
    reg32 side = 0;
    if (split)
        side = e.eax != 0 ? kRight : kLeft;
    int32_t volume = 0x77;
    if (effect < 0)
    {
        const reg32 first = nextRandom(app);
        const reg32 second = nextRandom(app);
        effect = int32_t(first & 3);
        volume = int32_t(second % 0x1e) + 0x5a;
    }
    const reg32 pan = split ? side : nextRandom(app) % 0xffff;
    e.finish(cpu, startEffect(app, cpu, frame, reg32(effect), split ? side : e.edi, bankHandle(app, kCrowdBank),
                              effect, volume, pan));
}

/* 0x415010: the car bank's effect 15 at full volume -- the player's side in
 * the split screen, by eax. */
void carEffectFifteen(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(4);
    const HeldEbp ebp(cpu, frame);
    const reg32 pan = gameSettings(app).playMode == 1 ? (e.eax != 0 ? kRight : kLeft) : 0;
    e.finish(cpu, startEffect(app, cpu, frame, e.esi, e.edi, bankHandle(app, kCarBank), 0xf, 0x7f, pan));
}

}  // namespace port
}  // namespace nfs3hp
