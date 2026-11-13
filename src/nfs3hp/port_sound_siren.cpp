#include "game_settings.h"
#include "port_snd_sources.h"

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
 * the race's (the player's 0x10, the cops' 0x1b on).  The game's code keeps
 * esi and edi for its own use around the calls into SND, which read them where
 * a play fails, so these set them the same way (sources::HeldEsiEdi). */
namespace nfs3hp
{
namespace port
{

namespace
{

using sources::callGameResult;
using sources::HeldEsiEdi;
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

// What is not rewritten here: the play of a slot, its stop, and an effect of a bank.
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
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    held.setEsi(reg32(siren));
    kSirenTone.set(app, siren, tone);
    const Result handle =
        callGameResult(app, cpu, frame, kPlayEffect, {reg32(kCopBank(app)), reg32(effect), 0, 0}, {0x40});
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
    const HeldEbp ebp(cpu, frame);
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
    const HeldEbp ebp(cpu, frame);
    if (kSirenOn(app, siren) != 0)
        return siren;
    HeldEsiEdi held(cpu);
    held.setEdi(1);
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
    held.setEdi(0);
    kSirenTone.set(app, siren, 0);
    kSirenFirst.set(app, siren, ticks);
    return toneStarted(app, cpu, frame - 4, siren, 0, 0);
}

/* 0x417f60's. */
Result sirenSwitchedOff(App* app, CPU& cpu, reg32 esp, int32_t siren)
{
    const reg32 frame = esp - 6 * 4;
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    held.setEsi(0xf0);
    held.setEdi(0x78);
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
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    held.setEsi(reg32(siren));
    held.setEdi(pan);
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
    Result r = setBend(app, cpu, at - 4, kSlotSounds(app, slot), bend);
    if (!r)
        return {};
    // The effects' volume times 63 (steady) or 85 (warbling), the siren's volume of it.
    const int32_t effects = gameSettings(app).effectsVolume;
    const int32_t scale = kSirenSecond(app, siren) == -1 ? imul(effects, 63) : imul(effects, 85);
    r = setVolume(app, cpu, at - 4, kSlotSounds(app, slot), imul(volume, scale) >> 14);
    if (!r)
        return {};
    const int32_t mode = gameSettings(app).soundSetup;
    held.setEsi(reg32(mode));
    // 3D (or surround): placed by the angle, the middle without stereo; else panned by the stereo's side.
    if (mode == 2)
        return placeSound(app, cpu, at - 4, kSlotSounds(app, slot), pan, 0);
    if (kSurround(app) != 0)
        return placeSound(app, cpu, at - 4, kSlotSounds(app, slot), kStereo(app) != 0 ? pan : 0, 0);
    return panSound(app, cpu, at - 4, kSlotSounds(app, slot),
                    kStereo(app) != 0 ? int32_t(stereoPan(pan)) : 0x40);
}

}  // namespace sources

/* The game's entries -- each its adapter, the registers and the stack in, the
 * result in eax out. */

/* 0x417b80 (stdcall 8): the noise ecx of slot eax, effect edx, as loud as the
 * car is near -- [esp+4] its distance (0x15f9 and on: silent), [esp+8] its pan
 * -- or the slot stopped.  The noise is a car's knock (port_sound_crash.cpp)
 * at a 16th of ecx's force.  eax as the code leaves it. */
void noiseByDistance(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(3);
    const reg32 at = frame - 4;  // and a local
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    const int32_t slot = int32_t(e.eax);
    held.setEsi(e.eax);
    held.setEdi(e.edx);
    if (kEffectsOn(app) == 0)
        return e.back(cpu, e.eax, 8);
    const int32_t distance = int32_t(e.arg(app, 0));
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
            return e.finish(cpu, callGameResult(app, cpu, at, kSlotStop, {e.eax}), 8);
        return e.back(cpu, reg32(eax), 8);
    }
    e.finish(cpu,
             sources::knockPlayed(app, cpu, at - 8 - 4, slot, e.edx, e.ebx, int32_t(e.ecx) >> 4, distance,
                                  e.arg(app, 1)),
             8);
}

/* 0x417c00: the player's noise (slot 0x10, the second player's 0x11 by eax in
 * the split screen), effect 3, at pitch ebx, as loud as edx is short of 0x15f9,
 * pan ecx. */
void playerNoise(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(3);
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    held.setEsi(e.eax);
    if (kEffectsOn(app) == 0)
        return e.back(cpu, e.eax);
    const int32_t distance = int32_t(e.edx);
    if (distance < kFar)
        held.setEdi(kFar);
    const int32_t slot = e.eax != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    const reg32 volume = reg32(nearness(distance, kFar));
    e.finish(cpu, callGameResult(app, cpu, frame, kSlotPlay, {reg32(slot), 3, e.ebx, volume}, {e.ecx}));
}

/* 0x417c70: the player's noise stopped (the second player's by eax in the
 * split screen). */
void playerNoiseStop(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(1);
    const HeldEbp ebp(cpu, frame);
    if (kEffectsOn(app) == 0)
        return e.back(cpu, e.eax);
    const reg32 slot = e.eax != 0 && gameSettings(app).playMode == 1 ? 0x11 : kPlayerSirenSlot;
    e.finish(cpu, callGameResult(app, cpu, frame, kSlotStop, {slot}));
}

/* 0x417ca0: whether siren eax is on. */
void sirenIsOn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(sources::sirenActive(app, int32_t(e.eax))));
}

/* 0x417cb0: siren eax's first tone (effect 0). */
void sirenFirstTone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, toneStarted(app, cpu, e.esp, int32_t(e.eax), 0, 0));
}

/* 0x417db0: siren eax's second tone (effect 1). */
void sirenSecondTone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, toneStarted(app, cpu, e.esp, int32_t(e.eax), 1, 1));
}

/* 0x417e00: siren eax's one tone for ever (effect 9). */
void sirenSteadyTone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, toneStarted(app, cpu, e.esp, int32_t(e.eax), 1, 9));
}

/* 0x417d00: siren eax's first tone faded out, the second's turn. */
void sirenFirstDone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, toneFaded(app, cpu, e.esp, 4, int32_t(e.eax), 1));
}

/* 0x417e60: siren eax's second tone faded out, the first's turn. */
void sirenSecondDone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, toneFaded(app, cpu, e.esp, 3, int32_t(e.eax), 0));
}

/* 0x417eb0: siren eax on, if it is not: steady (edx not 0), or warbling --
 * from either tone at random, for a random while. */
void sirenStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sources::sirenSwitchedOn(app, cpu, e.esp, int32_t(e.eax), e.edx != 0));
}

/* 0x417f60: siren eax off, faded out, its state as it starts. */
void sirenStop(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sources::sirenSwitchedOff(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x417fd0: siren eax's tick: its tones turned when their ticks are up, its
 * volume edx, pitch ebx (16.10) and pan ecx set. */
void sirenTick(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sources::sirenTicked(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

/* 0x4179d0 (stdcall 12): slot eax's noise edx -- 1 a car's, 0 the road's,
 * else another's -- at pitch ecx times ebx (16.16), as loud as [esp+4] (its
 * distance) is short of 0x15f (a car's) or 0x15f9, and as [esp+0xc] (its
 * speed) says; pan [esp+8].  Silent: the slot stopped.  eax as the code
 * leaves it: the last quotient it worked out when nothing plays. */
void noiseBySpeed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(3);
    const reg32 at = frame - 8;  // and two locals
    const HeldEbp ebp(cpu, frame);
    HeldEsiEdi held(cpu);
    const int32_t slot = int32_t(e.eax);
    const reg32 kind = e.edx;
    held.setEsi(e.eax);
    held.setEdi(kind);
    if (kEffectsOn(app) == 0)
        return e.back(cpu, e.eax, 0xc);
    const int32_t distance = int32_t(e.arg(app, 0));
    const reg32 pan = e.arg(app, 1);
    const int32_t speed = int32_t(e.arg(app, 2));
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
        eax = shiftedToZero(imul(int32_t(e.ebx + 0x28), nearness(distance, kFar)), 7);
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
            return e.finish(cpu, callGameResult(app, cpu, at, kSlotStop, {e.eax}), 0xc);
        return e.back(cpu, reg32(eax), 0xc);
    }
    const int32_t pitch = shiftedToZero(imul(int32_t(e.ecx), int32_t(e.ebx)), 16);
    e.finish(cpu, callGameResult(app, cpu, at, kSlotPlay, {e.eax, kind, reg32(pitch), reg32(volume)}, {pan}), 0xc);
}

}  // namespace port
}  // namespace nfs3hp
