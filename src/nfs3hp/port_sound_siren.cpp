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
constexpr Var<reg32> kPlayerSiren{0x570e68};
constexpr Table<reg32> kSirens{0x570ec0, 8};
constexpr Var<int32_t> kCopBank{0x571160};
constexpr Table<reg32> kSlotSounds{0x570de8, 8};  // the sound each of the race's slots plays (-1 none)
constexpr Var<int32_t> kSpeedClamp{0x57126c}, kNoiseScale{0x55260c};

// What the sirens call of the effects' (port_sound_effects.cpp): the play of a slot, its stop, and an effect of a
// bank.
constexpr reg32 kSlotPlay = 0x417080, kSlotStop = 0x416f60, kPlayEffect = 0x4182b0;

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
reg32 stereoPan(uint32_t pan)
{
    const uint32_t a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return reg32(int32_t(a) >> 8);
}

/* Siren i's sound. */
reg32 sirenSound(App* app, int32_t siren)
{
    return siren == kPlayerSirenIndex ? kPlayerSiren(app) : kSirens(app, siren);
}

/* The slot siren i's sound plays in. */
int32_t sirenSlot(int32_t siren) { return siren == kPlayerSirenIndex ? kPlayerSirenSlot : siren + kCopSirenSlots; }

/* 0x417cb0's, 0x417db0's, 0x417e00's: siren `siren`'s tone `tone` started
 * with effect `effect` of the cops' bank, its sound kept.  The handle. */
Result toneStarted(App* app, CPU& cpu, reg32 esp, int32_t siren, int32_t tone, int32_t effect)
{
    const reg32 frame = esp - 5 * 4;  // five pushes
    kSirenTone.set(app, siren, tone);
    cpu.esp = frame;
    const Result handle =
        guest::direct<kPlayEffect, bankEffect>(app, cpu, frame - 8, kCopBank(app), effect, 0, 0u, uint8_t(0x40));
    if (!handle)
        return {};
    if (siren == kPlayerSirenIndex)
        kPlayerSiren.set(app, reg32(*handle));
    else
        kSirens.set(app, siren, reg32(*handle));
    return handle;
}

/* 0x417d00's, 0x417e60's: siren `siren`'s tone faded out in 5 ticks, the tone
 * `next`'s turn, the ticks gone none (`pushes`: what the code pushes). */
Result toneFaded(App* app, CPU& cpu, reg32 esp, reg32 pushes, int32_t siren, int32_t next)
{
    const reg32 frame = esp - pushes * 4;
    kSirenTone.set(app, siren, next);
    kSirenTicks.set(app, siren, 0);
    return fadeSound(app, cpu, frame - 4, sirenSound(app, siren), 5, -1);
}

/* The warble's turn: a random number of ticks, `base` and up to `range` more. */
int32_t randomTicks(App* app, uint32_t range, int32_t base) { return int32_t(nextRandom(app) % range) + base; }

}  // namespace

namespace sources
{

int32_t sirenActive(App* app, int32_t siren) { return kSirenOn(app, siren); }

/* 0x417eb0's. */
Result sirenSwitchedOn(App* app, CPU& cpu, reg32 esp, int32_t siren, bool steady)
{
    const reg32 frame = esp - 5 * 4;
    if (kSirenOn(app, siren) != 0)
        return siren;
    kSirenOn.set(app, siren, 1);
    if (steady)
    {
        kSirenTone.set(app, siren, 1);
        kSirenSecond.set(app, siren, -1);
        return toneStarted(app, cpu, frame - 4, siren, 1, 9);
    }
    // Half the time the second tone first (for 2 to 3 seconds), else the first (for 2.4 to 4.4).
    if (nextRandom(app) % 100 > 0x32)
    {
        kSirenTone.set(app, siren, 1);
        kSirenSecond.set(app, siren, randomTicks(app, 0x3c, 0x78));
        return toneStarted(app, cpu, frame - 4, siren, 1, 1);
    }
    const int32_t ticks = randomTicks(app, 0x78, 0xf0);
    kSirenTone.set(app, siren, 0);
    kSirenFirst.set(app, siren, ticks);
    return toneStarted(app, cpu, frame - 4, siren, 0, 0);
}

/* 0x417f60's. */
Result sirenSwitchedOff(App* app, CPU& cpu, reg32 esp, int32_t siren)
{
    const reg32 frame = esp - 6 * 4;
    kSirenOn.set(app, siren, 0);
    kSirenTone.set(app, siren, 0);
    kSirenFirst.set(app, siren, 0xf0);
    kSirenSecond.set(app, siren, 0x78);
    kSirenTicks.set(app, siren, 0);
    return fadeSound(app, cpu, frame - 4, sirenSound(app, siren), 5, -1);
}

/* 0x417fd0's. */
Result sirenTicked(App* app, CPU& cpu, reg32 esp, int32_t siren, int32_t volume, int32_t pitch, reg32 pan)
{
    const reg32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    const reg32 at = frame - 0xc;     // and three locals: the code's esp from here on
    if (kSirenOn(app, siren) == 0)
        return siren;
    if (kSirenTone(app, siren) == 0)
    {
        // The first tone: up when its ticks are, then the second's turn.
        const int32_t ticks = int32_t(uint32_t(kSirenTicks(app, siren)) + 1);
        kSirenTicks.set(app, siren, ticks);
        if (ticks == kSirenFirst(app, siren))
        {
            if (!toneFaded(app, cpu, at - 4, 4, siren, 1))
                return {};
            kSirenSecond.set(app, siren, randomTicks(app, 0x3c, 0x78));
            if (!toneStarted(app, cpu, at - 4, siren, 1, 1))
                return {};
        }
    }
    else
    {
        const int32_t ticks = int32_t(uint32_t(kSirenTicks(app, siren)) + 1);
        kSirenTicks.set(app, siren, ticks);
        if (ticks == kSirenSecond(app, siren))
        {
            if (!toneFaded(app, cpu, at - 4, 3, siren, 0))
                return {};
            kSirenFirst.set(app, siren, randomTicks(app, 0x78, 0xf0));
            if (!toneStarted(app, cpu, at - 4, siren, 0, 0))
                return {};
        }
    }
    // The pitch (16.10) as a bend of 0..127.
    const int32_t bend = std::clamp(shiftedToZero(int32_t(uint32_t(pitch) << 6), 16), 0, 0x7f);
    const int32_t slot = sirenSlot(siren);
    // SND's bend's error text reads ebx, esi and edi as the code has them: the slot, the siren, the pan.
    Result r = setBendFrom(app, cpu, at - 4, kSlotSounds(app, slot), bend, reg32(slot), reg32(siren), pan);
    if (!r)
        return {};
    // The effects' volume times 63 (steady) or 85 (warbling), the siren's volume of it.
    const int32_t effects = gameSettings(app).effectsVolume;
    const int32_t scale = kSirenSecond(app, siren) == -1 ? imul(effects, 63) : imul(effects, 85);
    r = setVolumeFrom(app, cpu, at - 4, kSlotSounds(app, slot), imul(volume, scale) >> 14, frame);
    if (!r)
        return {};
    const int32_t mode = gameSettings(app).soundSetup;
    // 3D (or surround): placed by the angle, the middle without stereo; else panned by the stereo's side.
    if (mode == 2)
        return placeSound(app, cpu, at - 4, kSlotSounds(app, slot), pan, 0);
    if (kSurround(app) != 0)
        return placeSound(app, cpu, at - 4, kSlotSounds(app, slot), kStereo(app) != 0 ? pan : 0, 0);
    return panSound(app, cpu, at - 4, kSlotSounds(app, slot),
                    kStereo(app) != 0 ? int32_t(stereoPan(pan)) : 0x40);
}

}  // namespace sources

/* The game's entries -- each its thunk, the registers and the stack in, the
 * result in eax out. */

/* 0x417b80's: the noise `force` of slot `slot`, effect `effect`, as loud as
 * the car is near -- `distance` (0x15f9 and on: silent), `pan` -- or the slot
 * stopped.  The noise is a car's knock (port_sound_crash.cpp) at a 16th of
 * the force.  eax as the code leaves it. */
Result noiseByDistancePlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 effect, reg32 material, reg32 force,
                             int32_t distance, reg32 pan)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 at = frame - 4;  // and a local
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
            cpu.esp = at;
            return guest::direct<kSlotStop, slotStopped>(app, cpu, at - 4, slot);
        }
        return eax;
    }
    return sources::knockPlayed(app, cpu, at - 8 - 4, slot, effect, material, int32_t(force) >> 4, distance, pan);
}

/* 0x417b80 (stdcall 8): the noise ecx of slot eax, effect edx, material ebx,
 * [esp+4] its distance, [esp+8] its pan. */
x86::reg32 noiseByDistance(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             noiseByDistancePlayed(app, cpu, e.esp, int32_t(e.eax), e.edx, e.ebx, e.ecx, int32_t(e.arg(app, 0)),
                                   e.arg(app, 1)),
             8);
    return args.eax;
}

/* 0x417c00's: the player's noise (slot 0x10, the second player's 0x11 by
 * `player` in the split screen), effect 3, at pitch `pitch`, as loud as
 * `distance` is short of 0x15f9, pan `pan`.  `player` with the effects off. */
Result playerNoisePlayed(App* app, CPU& cpu, reg32 esp, reg32 player, int32_t distance, reg32 pitch, reg32 pan)
{
    const reg32 frame = esp - 3 * 4;
    if (kEffectsOn(app) == 0)
        return int32_t(player);
    const int32_t slot = player != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    const int32_t volume = nearness(distance, kFar);
    cpu.esp = frame;
    return guest::direct<kSlotPlay, slotPlayed>(app, cpu, frame - 8, slot, 3, volume, int32_t(pitch), pan);
}

/* 0x417c70's: the player's noise stopped (the second player's by `player` in
 * the split screen).  `player` with the effects off. */
Result playerNoiseStopped(App* app, CPU& cpu, reg32 esp, reg32 player)
{
    const reg32 frame = esp - 4;
    if (kEffectsOn(app) == 0)
        return int32_t(player);
    const int32_t slot = player != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    cpu.esp = frame;
    return guest::direct<kSlotStop, slotStopped>(app, cpu, frame - 4, slot);
}

/* 0x417c00: the player's noise, eax the player, edx the distance, ebx the
 * pitch, ecx the pan. */
x86::reg32 playerNoise(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, playerNoisePlayed(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx));
}

/* 0x417c70: the player's noise stopped, eax the player. */
x86::reg32 playerNoiseStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, playerNoiseStopped(app, cpu, e.esp, e.eax));
}

/* 0x417ca0: whether siren eax is on. */
x86::reg32 sirenIsOn(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(sources::sirenActive(app, int32_t(e.eax))));
}

/* 0x417cb0: siren eax's first tone (effect 0). */
x86::reg32 sirenFirstTone(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, toneStarted(app, cpu, e.esp, int32_t(e.eax), 0, 0));
}

/* 0x417db0: siren eax's second tone (effect 1). */
x86::reg32 sirenSecondTone(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, toneStarted(app, cpu, e.esp, int32_t(e.eax), 1, 1));
}

/* 0x417e00: siren eax's one tone for ever (effect 9). */
x86::reg32 sirenSteadyTone(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, toneStarted(app, cpu, e.esp, int32_t(e.eax), 1, 9));
}

/* 0x417d00: siren eax's first tone faded out, the second's turn. */
x86::reg32 sirenFirstDone(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, toneFaded(app, cpu, e.esp, 4, int32_t(e.eax), 1));
}

/* 0x417e60: siren eax's second tone faded out, the first's turn. */
x86::reg32 sirenSecondDone(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, toneFaded(app, cpu, e.esp, 3, int32_t(e.eax), 0));
}

/* 0x417eb0: siren eax on, if it is not: steady (edx not 0), or warbling --
 * from either tone at random, for a random while. */
x86::reg32 sirenStart(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, sources::sirenSwitchedOn(app, cpu, e.esp, int32_t(e.eax), e.edx != 0));
}

/* 0x417f60: siren eax off, faded out, its state as it starts. */
x86::reg32 sirenStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, sources::sirenSwitchedOff(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x417fd0: siren eax's tick: its tones turned when their ticks are up, its
 * volume edx, pitch ebx (16.10) and pan ecx set. */
x86::reg32 sirenTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, sources::sirenTicked(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

/* 0x4179d0's: slot `slot`'s noise `kind` -- 1 a car's, 0 the road's, else
 * another's -- at pitch `ecx` times `ebx` (16.16), as loud as `distance` is
 * short of 0x15f (a car's) or 0x15f9, and as `speed` says; pan `pan`.  Silent:
 * the slot stopped.  eax as the code leaves it: the last quotient it worked
 * out when nothing plays. */
Result noiseBySpeedPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 kind, reg32 ecx, reg32 ebx,
                          int32_t distance, reg32 pan, int32_t speed)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 at = frame - 8;  // and two locals
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
            cpu.esp = at;
            return guest::direct<kSlotStop, slotStopped>(app, cpu, at - 4, slot);
        }
        return eax;
    }
    const int32_t pitch = shiftedToZero(imul(int32_t(ecx), int32_t(ebx)), 16);
    cpu.esp = at;
    return guest::direct<kSlotPlay, slotPlayed>(app, cpu, at - 8, slot, int32_t(kind), volume, pitch, pan);
}

/* 0x4179d0 (stdcall 12): slot eax's noise edx at pitch ecx times ebx,
 * [esp+4] its distance, [esp+8] its pan, [esp+0xc] its speed. */
x86::reg32 noiseBySpeed(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             noiseBySpeedPlayed(app, cpu, e.esp, int32_t(e.eax), e.edx, e.ecx, e.ebx, int32_t(e.arg(app, 0)),
                                e.arg(app, 1), int32_t(e.arg(app, 2))),
             0xc);
    return args.eax;
}

}  // namespace port
}  // namespace nfs3hp
