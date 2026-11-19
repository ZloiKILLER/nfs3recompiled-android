#include "native_carstate.h"
#include "port_snd_sources.h"
#include "port_sound_effects.h"

#include <algorithm>
#include <cstdint>

/* The race's knocks and scrapes, rewritten (port.h says how): what a car hit
 * sounds like -- by what it hit and how hard, now and then at random, not the
 * same twice running where the game says so -- and how loud and high it plays,
 * near or far.
 *
 * A crash plays in slot 0x1f (a big one with its echo, effect 0x23); a scrape
 * plays in the car's own slot, its pitch kept per slot by crashPitch.  The
 * game's code plays them through slotPlay and playEffect (port_sound_effects.cpp). */
namespace nfs3hp
{
namespace port
{

namespace
{

using sources::quotient;

constexpr Var<int32_t> kLastCrash{0x552d80}, kEffectsOn{0x55290c};
constexpr Table<uint8_t> kSlotPitches{0x571244};     // each slot's last pitch
constexpr Table<int32_t> kSlotEffects{0x570dec, 8};  // the effect each of the race's sound slots plays
constexpr Var<int32_t> kCrashBank{0x571148};

// What the knock calls: the play of a slot, its stop, an effect of a bank
// (port_sound_effects.cpp), and the camera (a crash with no effect plays only in
// the camera view that asks for it).
constexpr u32 kSlotPlay = 0x417080, kSlotStop = 0x416f60, kPlayEffect = 0x4182b0, kCameraMode = 0x423b00;

// The slot a crash plays in, and its echo's effect.
constexpr int32_t kCrashSlot = 0x1f, kEchoEffect = 0x23;

/* A force as a level, 0..127: its 127ths of 0xa0000 (the code's shift, subtract
 * and divide), at most 127. */
int32_t forceLevel(int32_t force) { return std::min(0x7f, quotient(imul(force, 127), 0xa0000)); }

/* 0x414940's: slot `slot`'s pitch for effect `effect` at force `force`: the
 * force's level for the engines' slots (8, 9); else that level kept as the
 * slot's, or the one it has when the slot plays that effect already. */
int32_t slotPitch(App* app, int32_t slot, int32_t effect, int32_t force)
{
    if (slot == 8 || slot == 9)
        return forceLevel(force);
    if (effect == kSlotEffects(app, slot))
        return kSlotPitches(app, slot);
    const int32_t level = forceLevel(force);
    kSlotPitches.set(app, slot, uint8_t(level));
    return level;
}

/* 0x4149e0's: the crash effect for a hit of kind `kind` on material
 * `material` at force `force` (0..127): 2 for none.  Any other is kept as the
 * last, and not the same again but where the code allows (then by the random's
 * low bit). */
int32_t crashEffectFor(App* app, int32_t force, uint32_t kind, uint32_t material)
{
    const auto pick = [&](int32_t effect) {
        if (effect != 2)
            kLastCrash.set(app, effect);
        return effect;
    };
    const auto bit = [&] { return (nextRandom(app) & 1) != 0; };
    if (kind == 0xd)
        return 2;
    if (kind == 6 && material == 0x13)
        return pick(0x12);
    if (kind == 6 && material == 1)
        return pick(0x1f);
    if (kind == 5)
    {
        if (material == 0x10)
            return pick(0x1e);
        if (material == 0x11 || material == 0xa)
            return pick(0x1d);
        if (material == 8)
        {
            if (force > 0x6e)
                return pick(0x21);
            return pick(bit() ? 0x1e : 0x20);
        }
        if (material == 0x12)
        {
            static const int32_t picks[] = {0x23, 0x22, 0x20, 0x25};
            return pick(picks[nextRandom(app) & 3]);
        }
        if (material == 9)
            return pick(int32_t(nextRandom(app) & 1) + 0x18);
        if (material == 0xb)
            return pick(0x63);
        if (material == 1 || material == 7)
        {
            int32_t effect = 0x1d;
            if (force > 0x50)
                effect = 0x21;
            else if (force > 0x16)
                effect = 0x22;
            else if (force > 0x10)
                effect = 0x20;
            const uint32_t last = uint32_t(kLastCrash(app));
            // Not the same as the last, but for the loudest.
            if (uint32_t(effect) != last || last == 0x21)
                return pick(effect);
            if (last < 0x20)
            {
                if (last != 0x1d)
                    return pick(effect);
                return pick(bit() ? 0x22 : 0x20);
            }
            if (last == 0x20)
                return pick(bit() ? 0x1d : 0x22);
            if (last != 0x22)
                return pick(effect);
            return pick(bit() ? 0x1d : 0x20);
        }
    }
    int32_t effect;
    if (force > 0x5a)
        effect = 0x21;
    else if (force > 0x46)
        effect = 0x20;
    else if (force > 0x32)
        effect = 0x25;
    else if (force > 0x19)
        effect = 0x22;
    else
        effect = 0x1e;
    const uint32_t last = uint32_t(kLastCrash(app));
    if (uint32_t(effect) != last || last == 0x21)
        return pick(effect);
    switch (last - 0x1e)
    {
    case 0:  // 0x1e
        return pick(0x22);
    case 2:  // 0x20
        return pick(bit() ? 0x25 : 0x22);
    case 4:  // 0x22
        return pick(bit() ? 0x1e : 0x25);
    case 7:  // 0x25
        return pick(bit() ? 0x1e : 0x22);
    default:
        return pick(effect);
    }
}

/* 0x414d70's: the scrape effect for a hit of kind `kind` on material
 * `material`. */
int32_t scrapeEffectFor(uint32_t kind, uint32_t material)
{
    int32_t effect = 0x2c;
    if (kind == 6)
    {
        switch (material)
        {
        case 1: effect = 0x2a; break;
        case 2: effect = 0x2f; break;
        case 3: effect = 0x2d; break;
        case 4: effect = 0x2e; break;
        case 7: effect = 0x29; break;
        case 0xe: effect = 0x2b; break;
        case 0xf: effect = 0x2c; break;
        default: break;
        }
    }
    else if (kind == 5)
    {
        if (material == 1 || material == 7 || material == 0xe)
            effect = 0x28;
        else if (material == 0xf)
            effect = 0x2c;
    }
    return effect;
}

}  // namespace

namespace sources
{

/* 0x416d90's: it calls the crash's effect and the scrape's, the play and stop
 * of the slots and an effect of a bank (port_sound_effects.cpp) as C++. */
Result knockPlayed(App* app, ThreadState& ts, u32 esp, int32_t slot, u32 kind, u32 material, int32_t force,
                   int32_t distance, u32 pan)
{
    const u32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    const u32 at = frame - 8;       // and two locals: the code's esp from here on
    if (kEffectsOn(app) == 0)
        return distance;
    // How loud the distance leaves it: nothing from 0x15f9 on, twice as quick.
    const int32_t doubled = int32_t(uint32_t(distance) * 2);
    const int32_t loud = doubled < 0x15f9 ? quotient(imul(int32_t(0x15f9u - uint32_t(doubled)), 127), 0x15f9) : 0;
    const auto slotPlay = [&](int32_t s, int32_t effect, int32_t pitch, int32_t volume) {
        ts.sp = at;
        return guest::direct<kSlotPlay, slotPlayed>(app, ts, at - 8, s, effect, volume, pitch, pan);
    };
    if (slot < 0)
    {
        const int32_t level = forceLevel(force);
        const int32_t effect = crashEffectFor(app, level, kind, material);
        if (effect == 2)
        {
            // No effect of its own: a camera that asks for one plays effect 2.
            ts.sp = at;
            const u32 camera = guest::direct<kCameraMode, carstate::modeOfCamera>(app, ts, 0u);
            if (ts.terminate)
                return {};
            if (camera != 1)
                return int32_t(camera);
        }
        if (effect == 0x63)
        {
            ts.sp = at;
            return guest::direct<kPlayEffect, bankEffect>(app, ts, at - 8, kCrashBank(app), 1, loud, pan,
                                                          uint8_t(0x40));
        }
        if (effect == kCrashSlot)
        {
            // The crash's echo, as loud as the level and the distance say, at most 127.
            const int32_t doubledEcho = int32_t(uint32_t(quotient(imul(loud, level), 0x7f)) * 2);
            const int32_t echo = std::min(doubledEcho, 0x7f);
            if (!slotPlay(kCrashSlot, kCrashSlot, 0x40, loud))
                return {};
            return slotPlay(kCrashSlot, kEchoEffect, 0x40, echo);
        }
        return slotPlay(kCrashSlot, effect, 0x40, loud);
    }
    if (force <= 0)
    {
        ts.sp = at;
        return guest::direct<kSlotStop, slotStopped>(app, ts, at - 4, slot);
    }
    const int32_t scrape = scrapeEffectFor(kind, material);
    int32_t pitch = slotPitch(app, slot, scrape, force);
    int32_t volume = loud;
    // The engines' slots and the two heavy scrapes follow the force as well.
    if (slot == 8 || slot == 9 || scrape == 0x2f || scrape == 0x2e)
    {
        volume = quotient(imul(loud, forceLevel(force)), 0x7f);
        if (scrape == 0x2e)
            pitch = std::min(volume, 0x40);
    }
    if (scrape == 0x2c)
        pitch = std::max(int32_t(uint32_t(pitch) - 0x3c), 0);
    return slotPlay(slot, scrape, pitch, volume);
}

}  // namespace sources

/* The game's entries -- each its thunk, the registers and the stack in, the
 * result in eax out. */

/* 0x414940: slot eax's pitch for effect edx at force ebx. */
nfs::u32 crashPitch(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(slotPitch(app, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx))));
}

/* 0x4149e0: the crash effect for a hit of kind edx on material ebx at force
 * eax (0..127): 2 for none. */
nfs::u32 crashEffect(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(crashEffectFor(app, int32_t(e.eax), e.edx, e.ebx)));
}

/* 0x414d70: the scrape effect for a hit of kind edx on material ebx. */
nfs::u32 scrapeEffect(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(scrapeEffectFor(e.edx, e.ebx)));
}

/* 0x416d90 (stdcall 8): a car's knock in slot eax (below 0: a crash) of kind
 * edx on material ebx at force ecx, [esp+4] its distance, [esp+8] its pan.
 * With the effects off eax is the distance, as the code leaves it. */
nfs::u32 carKnock(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             sources::knockPlayed(app, ts, e.esp, int32_t(e.eax), e.edx, e.ebx, int32_t(e.ecx),
                                  int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
    return args.eax;
}

}  // namespace port
}  // namespace nfs3hp
