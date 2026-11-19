#include "game_settings.h"
#include "port_snd_sources.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"

#include <algorithm>
#include <cstdint>

/* The race's sirens and noises by distance, rewritten (port.h says how): a
 * siren of 5 (the cops' four, the fourth the player's own) that warbles -- its
 * two tones taking turns for so many ticks, picked at random -- its volume,
 * pitch and place set as it goes; the noises of a car near by as loud as it is
 * close.
 *
 * A siren's state is a word in each of five tables: on, which tone, the first
 * tone's ticks, the second's (-1: for ever) and the ticks gone.  Its sound is
 * the player's own for siren 4, else the cops' by siren; it plays in a slot of
 * the race's (the player's 0x10, the cops' 0x1b on). */
namespace nfs3hp
{
namespace port
{

namespace
{

using sources::quotient;
using sources::shiftedToZero;

constexpr Var<int32_t> kEffectsOn{0x55290c}, kStereo{0x55292c}, kSurround{0x5525a4};
constexpr Table<int32_t> kSirenOn{0x552d90}, kSirenTone{0x552da4}, kSirenFirst{0x552db8}, kSirenSecond{0x552dcc};
constexpr Table<int32_t> kSirenTicks{0x552de0};
constexpr Var<u32> kPlayerSiren{0x570e68};
constexpr Table<u32> kSirens{0x570ec0, 8};
constexpr Var<int32_t> kCopBank{0x571160};
constexpr Table<u32> kSlotSounds{0x570de8, 8};  // the sound each of the race's slots plays (-1 none)
constexpr Var<int32_t> kSpeedClamp{0x57126c}, kNoiseScale{0x55260c};

// What the sirens call of the effects' (port_sound_effects.cpp): the play of a slot, its stop, and an effect of a
// bank.
constexpr u32 kSlotPlay = 0x417080, kSlotStop = 0x416f60, kPlayEffect = 0x4182b0;

// The slot of the player's siren, and the cops' from this on.
constexpr int32_t kPlayerSirenIndex = 4, kPlayerSirenSlot = 0x10, kCopSirenSlots = 0x1b;

// A noise's loudness is nothing from this distance on (a car's from 0x15f).
constexpr int32_t kFar = 0x15f9, kCarFar = 0x15f;

/* How loud `distance` leaves a sound: its 127ths of `far` short, nothing past it. */
int32_t nearness(int32_t distance, int32_t far)
{
    return distance < far ? quotient(imul(int32_t(uint32_t(far) - uint32_t(distance)), 127), far) : 0;
}

/* A pan (0..0xffff around) as stereo's (0..127), as the game's code makes it. */
u32 stereoPan(uint32_t pan)
{
    const uint32_t a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return u32(int32_t(a) >> 8);
}

/* Siren i's sound. */
u32 sirenSound(App* app, int32_t siren)
{
    return siren == kPlayerSirenIndex ? kPlayerSiren(app) : kSirens(app, siren);
}

/* The slot siren i's sound plays in. */
int32_t sirenSlot(int32_t siren) { return siren == kPlayerSirenIndex ? kPlayerSirenSlot : siren + kCopSirenSlots; }

/* 0x417cb0's, 0x417db0's, 0x417e00's: siren `siren`'s tone `tone` started
 * with effect `effect` of the cops' bank, its sound kept.  The handle. */
Result toneStarted(App* app, ThreadState& ts, u32 esp, int32_t siren, int32_t tone, int32_t effect)
{
    const u32 frame = esp - 5 * 4;  // five pushes
    kSirenTone.set(app, siren, tone);
    ts.sp = frame;
    const Result handle =
        guest::direct<kPlayEffect, bankEffect>(app, ts, frame - 8, kCopBank(app), effect, 0, 0u, uint8_t(0x40));
    if (!handle)
        return {};
    if (siren == kPlayerSirenIndex)
        kPlayerSiren.set(app, u32(*handle));
    else
        kSirens.set(app, siren, u32(*handle));
    return handle;
}

/* 0x417d00's, 0x417e60's: siren `siren`'s tone faded out in 5 ticks, the tone
 * `next`'s turn, the ticks gone none (`pushes`: what the code pushes). */
Result toneFaded(App* app, ThreadState& ts, u32 esp, u32 pushes, int32_t siren, int32_t next)
{
    const u32 frame = esp - pushes * 4;
    kSirenTone.set(app, siren, next);
    kSirenTicks.set(app, siren, 0);
    return fadeSound(app, ts, frame - 4, sirenSound(app, siren), 5, -1);
}

/* The warble's turn: a random number of ticks, `base` and up to `range` more. */
int32_t randomTicks(App* app, uint32_t range, int32_t base) { return int32_t(nextRandom(app) % range) + base; }

}  // namespace

namespace sources
{

int32_t sirenActive(App* app, int32_t siren) { return kSirenOn(app, siren); }

/* 0x417eb0's. */
Result sirenSwitchedOn(App* app, ThreadState& ts, u32 esp, int32_t siren, bool steady)
{
    const u32 frame = esp - 5 * 4;
    if (kSirenOn(app, siren) != 0)
        return siren;
    kSirenOn.set(app, siren, 1);
    if (steady)
    {
        kSirenTone.set(app, siren, 1);
        kSirenSecond.set(app, siren, -1);
        return toneStarted(app, ts, frame - 4, siren, 1, 9);
    }
    // Half the time the second tone first (for 2 to 3 seconds), else the first (for 2.4 to 4.4).
    if (nextRandom(app) % 100 > 0x32)
    {
        kSirenTone.set(app, siren, 1);
        kSirenSecond.set(app, siren, randomTicks(app, 0x3c, 0x78));
        return toneStarted(app, ts, frame - 4, siren, 1, 1);
    }
    const int32_t ticks = randomTicks(app, 0x78, 0xf0);
    kSirenTone.set(app, siren, 0);
    kSirenFirst.set(app, siren, ticks);
    return toneStarted(app, ts, frame - 4, siren, 0, 0);
}

/* 0x417f60's. */
Result sirenSwitchedOff(App* app, ThreadState& ts, u32 esp, int32_t siren)
{
    const u32 frame = esp - 6 * 4;
    kSirenOn.set(app, siren, 0);
    kSirenTone.set(app, siren, 0);
    kSirenFirst.set(app, siren, 0xf0);
    kSirenSecond.set(app, siren, 0x78);
    kSirenTicks.set(app, siren, 0);
    return fadeSound(app, ts, frame - 4, sirenSound(app, siren), 5, -1);
}

/* 0x417fd0's. */
Result sirenTicked(App* app, ThreadState& ts, u32 esp, int32_t siren, int32_t volume, int32_t pitch, u32 pan)
{
    const u32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    const u32 at = frame - 0xc;     // and three locals: the code's esp from here on
    if (kSirenOn(app, siren) == 0)
        return siren;
    if (kSirenTone(app, siren) == 0)
    {
        // The first tone: up when its ticks are, then the second's turn.
        const int32_t ticks = int32_t(uint32_t(kSirenTicks(app, siren)) + 1);
        kSirenTicks.set(app, siren, ticks);
        if (ticks == kSirenFirst(app, siren))
        {
            if (!toneFaded(app, ts, at - 4, 4, siren, 1))
                return {};
            kSirenSecond.set(app, siren, randomTicks(app, 0x3c, 0x78));
            if (!toneStarted(app, ts, at - 4, siren, 1, 1))
                return {};
        }
    }
    else
    {
        const int32_t ticks = int32_t(uint32_t(kSirenTicks(app, siren)) + 1);
        kSirenTicks.set(app, siren, ticks);
        if (ticks == kSirenSecond(app, siren))
        {
            if (!toneFaded(app, ts, at - 4, 3, siren, 0))
                return {};
            kSirenFirst.set(app, siren, randomTicks(app, 0x78, 0xf0));
            if (!toneStarted(app, ts, at - 4, siren, 0, 0))
                return {};
        }
    }
    // The pitch (16.10) as a bend of 0..127.
    const int32_t bend = std::clamp(shiftedToZero(int32_t(uint32_t(pitch) << 6), 16), 0, 0x7f);
    const int32_t slot = sirenSlot(siren);
    // SND's bend's error text reads ebx, esi and edi as the code has them: the slot, the siren, the pan.
    Result r = setBendFrom(app, ts, at - 4, kSlotSounds(app, slot), bend, u32(slot), u32(siren), pan);
    if (!r)
        return {};
    // The effects' volume times 63 (steady) or 85 (warbling), the siren's volume of it.
    const int32_t effects = gameSettings(app).effectsVolume;
    const int32_t scale = kSirenSecond(app, siren) == -1 ? imul(effects, 63) : imul(effects, 85);
    r = setVolumeFrom(app, ts, at - 4, kSlotSounds(app, slot), imul(volume, scale) >> 14, frame);
    if (!r)
        return {};
    const int32_t mode = gameSettings(app).soundSetup;
    // 3D (or surround): placed by the angle, the middle without stereo; else panned by the stereo's side.
    if (mode == 2)
        return placeSound(app, ts, at - 4, kSlotSounds(app, slot), pan, 0);
    if (kSurround(app) != 0)
        return placeSound(app, ts, at - 4, kSlotSounds(app, slot), kStereo(app) != 0 ? pan : 0, 0);
    return panSound(app, ts, at - 4, kSlotSounds(app, slot),
                    kStereo(app) != 0 ? int32_t(stereoPan(pan)) : 0x40);
}

}  // namespace sources

/* The game's entries -- each its thunk, the registers and the stack in, the
 * result in eax out. */

/* 0x417b80's: the noise `force` of slot `slot`, effect `effect`, as loud as
 * the car is near -- `distance` (0x15f9 and on: silent), `pan` -- or the slot
 * stopped.  The noise is a car's knock (port_sound_crash.cpp) at a 16th of
 * the force.  eax as the code leaves it. */
Result noiseByDistancePlayed(App* app, ThreadState& ts, u32 esp, int32_t slot, u32 effect, u32 material, u32 force,
                             int32_t distance, u32 pan)
{
    const u32 frame = esp - 3 * 4;
    const u32 at = frame - 4;  // and a local
    if (kEffectsOn(app) == 0)
        return slot;
    int32_t eax = slot;
    bool audible = false;
    if (distance < kFar)
    {
        eax = nearness(distance, kFar);
        audible = eax != 0;
    }
    if (!audible && slot != -1)
    {
        if (kSlotSounds(app, slot) != 0xffffffffu)
        {
            ts.sp = at;
            return guest::direct<kSlotStop, slotStopped>(app, ts, at - 4, slot);
        }
        return eax;
    }
    return sources::knockPlayed(app, ts, at - 8 - 4, slot, effect, material, int32_t(force) >> 4, distance, pan);
}

/* 0x417b80 (stdcall 8): the noise ecx of slot eax, effect edx, material ebx,
 * [esp+4] its distance, [esp+8] its pan. */
nfs::u32 noiseByDistance(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             noiseByDistancePlayed(app, ts, e.esp, int32_t(e.eax), e.edx, e.ebx, e.ecx, int32_t(e.arg(app, 0)),
                                   e.arg(app, 1)),
             8);
    return args.eax;
}

/* 0x417c00's: the player's noise (slot 0x10, the second player's 0x11 by
 * `player` in the split screen), effect 3, at pitch `pitch`, as loud as
 * `distance` is short of 0x15f9, pan `pan`.  `player` with the effects off. */
Result playerNoisePlayed(App* app, ThreadState& ts, u32 esp, u32 player, int32_t distance, u32 pitch, u32 pan)
{
    const u32 frame = esp - 3 * 4;
    if (kEffectsOn(app) == 0)
        return int32_t(player);
    const int32_t slot = player != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    const int32_t volume = nearness(distance, kFar);
    ts.sp = frame;
    return guest::direct<kSlotPlay, slotPlayed>(app, ts, frame - 8, slot, 3, volume, int32_t(pitch), pan);
}

/* 0x417c70's: the player's noise stopped (the second player's by `player` in
 * the split screen).  `player` with the effects off. */
Result playerNoiseStopped(App* app, ThreadState& ts, u32 esp, u32 player)
{
    const u32 frame = esp - 4;
    if (kEffectsOn(app) == 0)
        return int32_t(player);
    const int32_t slot = player != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    ts.sp = frame;
    return guest::direct<kSlotStop, slotStopped>(app, ts, frame - 4, slot);
}

/* 0x417c00: the player's noise, eax the player, edx the distance, ebx the
 * pitch, ecx the pan. */
nfs::u32 playerNoise(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, playerNoisePlayed(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx));
}

/* 0x417c70: the player's noise stopped, eax the player. */
nfs::u32 playerNoiseStop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, playerNoiseStopped(app, ts, e.esp, e.eax));
}

/* 0x417ca0: whether siren eax is on. */
nfs::u32 sirenIsOn(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(sources::sirenActive(app, int32_t(e.eax))));
}

/* 0x417cb0: siren eax's first tone (effect 0). */
nfs::u32 sirenFirstTone(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, toneStarted(app, ts, e.esp, int32_t(e.eax), 0, 0));
}

/* 0x417db0: siren eax's second tone (effect 1). */
nfs::u32 sirenSecondTone(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, toneStarted(app, ts, e.esp, int32_t(e.eax), 1, 1));
}

/* 0x417e00: siren eax's one tone for ever (effect 9). */
nfs::u32 sirenSteadyTone(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, toneStarted(app, ts, e.esp, int32_t(e.eax), 1, 9));
}

/* 0x417d00: siren eax's first tone faded out, the second's turn. */
nfs::u32 sirenFirstDone(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, toneFaded(app, ts, e.esp, 4, int32_t(e.eax), 1));
}

/* 0x417e60: siren eax's second tone faded out, the first's turn. */
nfs::u32 sirenSecondDone(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, toneFaded(app, ts, e.esp, 3, int32_t(e.eax), 0));
}

/* 0x417eb0: siren eax on, if it is not: steady (edx not 0), or warbling --
 * from either tone at random, for a random while. */
nfs::u32 sirenStart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sources::sirenSwitchedOn(app, ts, e.esp, int32_t(e.eax), e.edx != 0));
}

/* 0x417f60: siren eax off, faded out, its state as it starts. */
nfs::u32 sirenStop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sources::sirenSwitchedOff(app, ts, e.esp, int32_t(e.eax)));
}

/* 0x417fd0: siren eax's tick: its tones turned when their ticks are up, its
 * volume edx, pitch ebx (16.10) and pan ecx set. */
nfs::u32 sirenTick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sources::sirenTicked(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

/* 0x4179d0's: slot `slot`'s noise `kind` -- 1 a car's, 0 the road's, else
 * another's -- at pitch `ecx` times `ebx` (16.16), as loud as `distance` is
 * short of 0x15f (a car's) or 0x15f9, and as `speed` says; pan `pan`.  Silent:
 * the slot stopped.  eax as the code leaves it: the last quotient it worked
 * out when nothing plays. */
Result noiseBySpeedPlayed(App* app, ThreadState& ts, u32 esp, int32_t slot, u32 kind, u32 ecx, u32 ebx,
                          int32_t distance, u32 pan, int32_t speed)
{
    const u32 frame = esp - 3 * 4;
    const u32 at = frame - 8;  // and two locals
    if (kEffectsOn(app) == 0)
        return slot;
    int32_t eax = slot;  // as the code has it as it goes
    int32_t loud;
    if (kind == 1)
    {
        // A car's: as loud as it is near, within 0x15f.
        if (distance < kCarFar)
            loud = eax = nearness(distance, kCarFar);
        else
            loud = 0;
    }
    else
    {
        // Else the effects' 40ths of it again, at most 127.
        eax = shiftedToZero(imul(int32_t(ebx + 0x28), nearness(distance, kFar)), 7);
        eax = std::min(eax, 0x7f);
        eax = std::max(eax, 0);
        loud = eax;
    }
    // How fast it goes, in whole units, 40 at most.
    eax = shiftedToZero(speed > 0 ? speed : int32_t(0u - uint32_t(speed)), 16);
    kSpeedClamp.set(app, std::min(eax, 0x28));
    int32_t volume = loud;
    if (kind == 1)
        volume = eax = quotient(imul(quotient(imul(kSpeedClamp(app), loud), 0x28), 120), 100);
    if (kind == 0)
    {
        // The road's: its distance's share of the scale the game keeps, 115 of it at most.
        const int32_t scale = kNoiseScale(app);
        const int32_t share =
            scale != 0 ? quotient(imul(int32_t(uint32_t(kFar) - uint32_t(distance)), loud), scale) : 0;
        volume = eax = quotient(imul(int32_t(uint32_t(share) << 3), 15), 100);
        if (eax > 0x73)
            volume = 0x73;
    }
    volume = std::min(volume, 0x7f);
    if (volume == 0 && slot != -1)
    {
        if (kSlotSounds(app, slot) != 0xffffffffu)
        {
            ts.sp = at;
            return guest::direct<kSlotStop, slotStopped>(app, ts, at - 4, slot);
        }
        return eax;
    }
    const int32_t pitch = shiftedToZero(imul(int32_t(ecx), int32_t(ebx)), 16);
    ts.sp = at;
    return guest::direct<kSlotPlay, slotPlayed>(app, ts, at - 8, slot, int32_t(kind), volume, pitch, pan);
}

/* 0x4179d0 (stdcall 12): slot eax's noise edx at pitch ecx times ebx,
 * [esp+4] its distance, [esp+8] its pan, [esp+0xc] its speed. */
nfs::u32 noiseBySpeed(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             noiseBySpeedPlayed(app, ts, e.esp, int32_t(e.eax), e.edx, e.ecx, e.ebx, int32_t(e.arg(app, 0)),
                                e.arg(app, 1), int32_t(e.arg(app, 2))),
             0xc);
    return args.eax;
}

}  // namespace port
}  // namespace nfs3hp
