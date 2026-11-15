#include "game_settings.h"
#include "native_ea_support.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "port_snd_voices.h"
#include "port_sound_effects.h"
#include "port_sound_slice.h"

#include <cstdint>

/* A player's own car's sound, rewritten (port.h says how): its engine
 * (0x417390), its knocks and scrapes as its collision list has them, the
 * engine's warning (effect 5 of the car's bank) going downhill backwards now
 * and then, the cop's siren and radio for a cop's car, or the horn's noise --
 * each from the car's place, distance and angle.
 *
 * The x87 as the game's code has it: the car's numbers are floats (single),
 * a comparison with a limit is against the double that lies in the game's
 * data, the revs' integer is the float cut towards 0 (Watcom's __CHP and a
 * fistp).  The dot product of the car's vector with its cell's up comes from
 * the game's own 0x4e01f0 (vertices::dotProduct), called as C++. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Var<int32_t> kEffectsOn{0x55290c};
constexpr Var<int32_t> kStereoOn{0x55292c}, kCarBank{0x57113c};
constexpr Var<reg32> kCells{0x5dd948};
constexpr Var<uint8_t> kBusUsed{0x552930}, kGearCounter{0x552d8c};
constexpr Table<int32_t> kLoads{0x571078};
constexpr Table<uint8_t> kLoadCurve{0x552a38};
constexpr Table<reg32> kPlayerViews{0x5e10b8}, kPlayerLooks{0x5e10b0};
constexpr reg32 kListener = 0x570a78;  // the listener: where the sounds are heard from
constexpr reg32 kSources = 0x570ad8;   // 0x50 a player's source: the car's pointer at +0x18, a flag at +0x38
constexpr Var<double> kFastSpeed{0x536bb0}, kFastHeight{0x536bb8};

// What the car's sound calls of the game's.
constexpr reg32 kDotProduct = 0x4e01f0, kCarEffectFive = 0x418280, kPlaceAhead = 0x4124d0;
constexpr reg32 kSourceAngle = 0x410d30, kSourceDistance = 0x410cb0, kCarAngle = 0x410dc0, kCarKnock = 0x416d90;
constexpr reg32 kSlotStop = 0x416f60, kMusicEventResume = 0x418c60, kCopSiren = 0x411c50, kCopSirenStop = 0x411ca0;
constexpr reg32 kRadioTick = 0x4c8810, kPlayerNoise = 0x417c00, kPlayerNoiseStop = 0x417c70, kTrace = 0x42d510;
constexpr reg32 kCoDriverTick = 0x4c8d10, kRaceAnnouncer = 0x412d60, kSceneryTick = 0x415210;
constexpr reg32 kFarEffectTick = 0x414f60, kInTunnel = 0x420bf0, kSurfaceUnder = 0x420840;
constexpr reg32 kBusUp = 0x418770, kBusDown = 0x418790, kSoundVolume = 0x4e95fc, kPitchBend = 0x4e97e4;
constexpr reg32 kPan = 0x4e9950;

reg32 wordAt(App* app, reg32 at) { return app->getMemory<reg32>(at); }
uint8_t byteAt(App* app, reg32 at) { return app->getMemory<uint8_t>(at); }
uint16_t halfAt(App* app, reg32 at) { return app->getMemory<uint16_t>(at); }
float floatAt(App* app, reg32 at) { return app->getMemory<float>(at); }

/* The code's signed shift towards 0 (sar, rounded up for a negative): `x`
 * over 2^k, truncated. */
int32_t towardZero(int32_t x, unsigned k) { return x < 0 ? int32_t(uint32_t(x) + ((1u << k) - 1)) >> k : x >> k; }

/* fistp of a float cut by __CHP: towards 0, the x87's "integer indefinite"
 * past the range (and for a number that is not one). */
int32_t truncated(float value)
{
    if (!(value > -2147483649.0f && value < 2147483648.0f))
        return INT32_MIN;
    return int32_t(value);
}

/* A pan (0..0xffff around) as stereo's (0..127), as the game's code makes it. */
reg32 stereoPanOf(reg32 pan)
{
    const reg32 a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return reg32(int32_t(a) >> 8);
}

}  // namespace

/* 0x417390's: car `car`'s engine -- at distance squared `distanceSquared`,
 * `speedShare` times `ahead` (16.16) of throttle, pan `pan`, angle `angle`:
 * the radio (0x4c8d10), the cop chase (0x412d60) and the cars about
 * (0x415210) first, the far-off effect's tick; then its revs (+0x4f4) as the
 * three engine sounds' (in slots 0, 1 and 12, the second player's 2, 3 and
 * 13) volume, pitch and pan -- louder in a tunnel (the bus on), the gear
 * change's effect now and then.  eax as the code leaves it: the last call's. */
Result engineSounded(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t distanceSquared, int32_t speedShare,
                     int32_t ahead, reg32 pan, reg32 angle)
{
    const reg32 frame = esp - 3 * 4;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame - 0x40);
    const int32_t split = gameSettings(app).playMode;
    const uint8_t driver = byteAt(app, car + 0x1f0);
    const reg32 playerSlot = wordAt(app, car + 0x1f0);
    g.esi = car;
    g.edi = reg32(split);
    reg32 eax;
    if (!g.call(kCoDriverTick, {split == 1 ? reg32(driver) : 0, angle, (reg32(speedShare) & 0xffffff00) | driver, reg32(ahead)}, eax)
        || !g.call(kRaceAnnouncer, {car, angle}, eax) || !g.call(kSceneryTick, {car, angle}, eax))
        return {};
    if ((split != 1 || driver == 0) && !g.call(kFarEffectTick, {eax}, eax))
        return {};
    if (kEffectsOn(app) == 0)
        return int32_t(eax);
    // How close the car is: 127 at the sound's, none from 0x15f9 squared on.
    int32_t near = 0;
    if (distanceSquared < 0x15f9)
        near = int32_t((0x15f9u - reg32(distanceSquared)) * 127) / 0x15f9;
    // The revs, cut towards 0: twice their size is the volume, to 127 for the
    // gear change's.
    const int32_t revs = truncated(floatAt(app, car + 0x4f4));
    reg32 volume = 2u * reg32(revs > 0 ? revs : negated(revs));
    g.edi = volume;
    const int32_t revsVolume = int32_t(volume) > 0x7f ? 0x7f : int32_t(volume);
    int32_t gearEvery = 8 - towardZero(revsVolume, 4);  // turns between the gear change's effect
    if (gearEvery < 3)
        gearEvery = 3;
    int32_t pitch = 0x40;
    int32_t throttle = towardZero(imul(speedShare, ahead), 16);
    if (throttle > 0x64)
        throttle = 0x64;
    else if (throttle < 0)
        throttle = 0;
    // In a tunnel (0x420bf0's, of the car's place and cell): the bus up.
    const reg32 cell = wordAt(app, car + 0x1c);
    g.atCall();
    if (!tookEax(guest::direct<kInTunnel, trackdata::placeInTunnel>(app, cpu, car + 0x98, cell), eax))
        return {};
    if (eax != 0)
    {
        if (kBusUsed(app) == 0 && split != 1)
        {
            if (!g.call(kBusUp, {eax}, eax))
                return {};
            kBusUsed.set(app, 1);
        }
        volume += 0x14;
        pitch = 0x5e;
    }
    else if (kBusUsed(app) != 0 && split != 1)
    {
        if (!g.call(kBusDown, {eax}, eax))
            return {};
        kBusUsed.set(app, 0);
    }
    // In the split screen the player's side.
    const bool inSplit = split == 1;
    const reg32 side = playerSlot != 0 ? 0xc000 : 0x3fff;
    // The ground's kind (0x420840's, else the car's +0x18c): some lower the
    // pitch, some (now and then) the gear change's effect.
    g.edi = volume;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSurfaceUnder, trackdata::groundUnder>(app, cpu, x86::sreg32(cell),
                                                                            x86::sreg32(wordAt(app, car + 0x7c))),
                 eax))
        return {};
    const reg32 kind = eax != 0 ? eax : wordAt(app, car + 0x18c);
    const reg32 ground = kind - 2;
    const bool lower = ground == 0 || ground == 8 || ground == 9 || ground == 0xa || ground == 0xb;
    const bool gearKind = ground == 5 || ground == 0xa || ground == 0xe || ground == 0xf;
    if (lower)
    {
        volume += reg32(towardZero(int32_t(volume), 2));
        g.edi = volume;
        pitch = 0x18;
    }
    // The gear change's effect.
    const auto gearChange = [&](reg32 gearPitch, reg32 gearVolume) {
        g.atCall();
        return tookEax(guest::direct<kEffectBank, bankEffect>(app, cpu, g.esp - 8, kCarBank(app), 0x1d,
                                                               int32_t(gearVolume), inSplit ? side : 0u,
                                                               uint8_t(gearPitch)),
                       eax);
    };
    // The turn of the gear change's counter: false when the game stopped.
    if (gearKind)
    {
        const uint8_t counter = uint8_t(kGearCounter(app) + 1);
        const uint8_t turn = uint8_t(int32_t(counter) % gearEvery);
        kGearCounter.set(app, turn);
        if (turn == 0 && int32_t(wordAt(app, car + 0x15c)) < 0x3e4ccccd && gameSettings(app).track != 8)
        {
            const reg32 gearPitch = kind == 7 || kind == 0xc ? 0x28 : 0x40;
            if (!gearChange(gearPitch, reg32(towardZero(revsVolume, 1))))
                return {};
        }
    }
    // Going fast off the ground: the effect too.
    if (wordAt(app, car + 0x99c) != 0 && halfAt(app, car + 0x164) == 0
        && double(floatAt(app, car + 0x10c)) > kFastSpeed(app) && double(floatAt(app, car + 0xb8)) > kFastHeight(app))
    {
        const uint8_t counter = uint8_t(kGearCounter(app) + 1);
        const uint8_t turn = uint8_t(int32_t(counter) % gearEvery);
        kGearCounter.set(app, turn);
        if (turn == 0 && int32_t(wordAt(app, car + 0x15c)) < 0x3e4ccccd && !gearChange(0x40, reg32(revsVolume)))
            return {};
    }
    if (distanceSquared > 4)
        volume = reg32(idiv(imul(int32_t(volume), 6), 10));
    const int32_t shape = distanceSquared < 4 ? 0x41 : distanceSquared < 0x23 ? 0xc : 0;
    if (int32_t(volume) > 0x7f)
        volume = 0x7f;
    // The load: the revs eased towards the car's (+0x508), the three sounds'
    // share of it (0x552a38's curve).
    const int32_t target = byteAt(app, car + 0x508);
    const int32_t was = kLoads(app, int32_t(playerSlot));
    const int32_t change = int32_t(reg32(target) - reg32(was));
    kLoads.set(app, int32_t(playerSlot),
               int32_t(reg32(was) + reg32(target >= was ? towardZero(change, 1) : towardZero(change, 3))));
    int32_t load = kLoads(app, int32_t(playerSlot));
    if (load > 0xff)
        load = 0xff;
    load >>= 1;
    const int32_t rising = kLoadCurve(app, load);
    const int32_t falling = kLoadCurve(app, int32_t(0x80u - reg32(load)));
    int32_t share = towardZero(imul(throttle + 0x28, near), 7);
    if (share > 0x7f)
        share = 0x7f;
    if (share < 0)
        share = 0;
    const int32_t falls = imul(falling, share) >> 7;
    const int32_t engine = imul(int32_t(volume), share) >> 7;
    const int32_t rises = imul(rising, share) >> 7;
    g.edi = reg32(engine);
    // The pan: the game's, the player's side in the split screen.
    reg32 panned = std::int32_t(gameSettings(app).replayMode) >= 2 ? stereoPanOf(pan) : 0x40;
    if (inSplit)
        panned = kStereoOn(app) == 0 ? 0x40 : (playerSlot != 0 ? 0 : 0x7f);
    const bool second = inSplit && playerSlot != 0;
    const reg32 first = second ? 0x570e50 : 0x570e48, low = second ? 0x570df0 : 0x570de8,
                third = second ? 0x570e00 : 0x570df8;
    // The three sounds: their volume, pitch and pan.
    const reg32 setting = reg32(gameSettings(app).engineVolume);
    const int32_t scale = int32_t((setting + (((setting << 2) - setting) << 3)) * 2);
    const int32_t lead = shape + 0x32;
    // SND's calls as C++; SNDpitchbend's error text reads ebx as the code has it (0, its
    // calls' loading of none).
    const auto soundVolume = [&](reg32 handle, int32_t level) {
        g.atCall();
        return tookEax(guest::direct<kSoundVolume, setVolume>(app, cpu, g.esp - 4, handle, level), eax);
    };
    const auto pitchBend = [&](reg32 handle, int32_t bend) {
        g.atCall();
        cpu.ebx = 0;
        return tookEax(guest::direct<kPitchBend, setBend>(app, cpu, g.esp - 4, handle, bend), eax);
    };
    const auto soundPan = [&](reg32 handle, reg32 to) {
        g.atCall();
        return tookEax(guest::direct<kPan, panSound>(app, cpu, g.esp - 4, handle, int32_t(to)), eax);
    };
    if (!soundVolume(wordAt(app, first), imul(scale, engine) >> 14))
        return {};
    g.esi = reg32(shape);
    if (!pitchBend(wordAt(app, first), pitch))
        return {};
    g.esi = reg32(lead);
    if (!soundPan(wordAt(app, first), panned)
        || !soundVolume(wordAt(app, low), imul(imul(int32_t(setting), lead), rises) >> 14)
        || !pitchBend(wordAt(app, low), throttle) || !soundPan(wordAt(app, low), panned)
        || !soundVolume(wordAt(app, third), imul(imul(int32_t(setting), lead), falls) >> 14)
        || !pitchBend(wordAt(app, third), throttle)
        || !soundPan(wordAt(app, third), panned))
        return {};
    return int32_t(eax);
}

/* 0x417390 (stdcall 8): car eax's engine -- at distance edx, throttle ebx
 * times ecx (16.16), pan [esp+4], angle [esp+8]. */
void carEngine(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             engineSounded(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0),
                           e.arg(app, 1)),
             8);
}

/* 0x4117a0: player eax's car's sounds, from its source (0x570ad8's): the
 * engine, the car's knocks and scrapes, the cop's siren and radio or the
 * horn's noise.  eax as the code leaves it. */
void playerCarSound(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(6);
    const reg32 place = frame - 0x34, up = frame - 0x40;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, up);
    const reg32 player = e.eax;
    const reg32 source = ((player * 5) << 4) + kSources + 0x18;  // the car's pointer in the player's source
    const reg32 car = wordAt(app, source);
    g.esi = car;
    if (car == 0)
        return e.back(cpu, source);
    for (reg32 k = 0; k < 3; ++k)
        store<reg32>(app, place + 4 * k, wordAt(app, car + 0x98 + 4 * k));
    // The cell's up, and how much of it the car's own has: downhill below 0.
    const reg32 cell = wordAt(app, car + 0x1c);
    const reg32 cells = kCells(app);
    for (reg32 k = 0; k < 3; ++k)
        store<float>(app, up + 4 * k, float(int16_t(int8_t(byteAt(app, cells + cell * 9 * 4 + 0x14 + k)))));
    reg32 eax;
    float down = float(guest::direct<kDotProduct, vertices::dotProduct>(app, cpu, car + 0xd8, up));
    if (cpu.terminate)
        return;
    const int32_t second = gameSettings(app).backwards;
    if (second != 0)
        down = -down;
    // The warning of the engine going backwards downhill, a counter's turn
    // now and then; any other time the counter is reset.
    bool reset = (byteAt(app, car + 0x200) & 1) != 0 && wordAt(app, car + 0x2c4) == 3;
    if (!reset)
    {
        reset = !(down < 0.0f);
        if (!reset && halfAt(app, car + 0x164) == 0 && wordAt(app, car + 0x8d4) == 0
            && (byteAt(app, car + 0x200) & 0x20) == 0)
        {
            const int32_t count = int32_t(wordAt(app, car + 0x998) + 1);
            app->getMemory<int32_t>(car + 0x998) = count;
            if (count >= 0x40 && count < 0x94 && (count & 0x1f) == 0
                && !g.call(kCarEffectFive, {reg32(count), up, e.ebx, reg32(second)}, eax))
                return;
        }
    }
    if (reset)
        app->getMemory<int32_t>(car + 0x998) = 0;
    if (!g.call(kPlaceAhead, {place, car + 0xa4, e.ebx, reg32(second)}, eax))
        return;
    const reg32 ahead = eax;
    // The pan: the player's side in the split screen, else by the angle (none
    // for a view of kind 2).
    reg32 pan, looking = reg32(gameSettings(app).playMode);  // looking: ebx as the code leaves it
    const int32_t split = gameSettings(app).playMode;
    if (split == 1)
        pan = player != 0 ? 0xc000 : 0x3fff;
    else
    {
        const reg32 view = kPlayerViews(app, int32_t(player));
        reg32 kindOfView;
        if (view != 0)
            kindOfView = wordAt(app, view);
        else
        {
            const reg32 look = kPlayerLooks(app, int32_t(player));
            looking = look;
            kindOfView = look != 0 ? wordAt(app, look) : 0;
        }
        if (kindOfView == 2)
        {
            pan = 0;
            g.edi = 0;
        }
        else if (!g.call(kSourceAngle, {kListener, place, looking, reg32(second)}, pan))
            return;
    }
    if (!g.call(kSourceDistance, {kListener, place, looking, reg32(second)}, eax))
        return;
    const int32_t whole = towardZero(int32_t(eax), 16);
    const reg32 distanceSquared = reg32(imul(whole, whole));
    g.edi = distanceSquared;
    // How loud the car is for its speed: its top (+0x590 of 127 over the
    // engine's own limit, which the car's tyre and gear change).
    const reg32 tyre = wordAt(app, wordAt(app, car + 0x220) + 0x1c);
    reg32 limit = wordAt(app, wordAt(app, car + 0x528) + 0x1c4) + (tyre == 1 ? 0x3e8 : 0x7d0);
    if (halfAt(app, car + 0x164) > 5)
        limit -= 0x1f4;
    const reg32 speed = wordAt(app, car + 0x590);
    const int32_t speedShare = idiv(int32_t((speed << 7) - speed), int32_t(limit));
    reg32 angle;
    if (!g.call(kCarAngle, {kListener, car, reg32(speedShare), limit}, angle))
        return;
    g.sync();
    if (!engineSounded(app, cpu, up - 4 - 8, car, int32_t(distanceSquared), speedShare, int32_t(ahead), pan, angle))
        return;
    // The car's knocks and scrapes: 24 bytes each from +0x7e0.
    const int32_t count = int32_t(wordAt(app, car + 0x7dc));
    const bool secondPlayer = player != 0;
    int32_t k = 0;
    for (; k < count; ++k)
    {
        const reg32 entry = car + reg32(k) * 24;
        const reg32 selector = wordAt(app, entry + 0x7e4);
        reg32 slot = selector;
        if (selector == 8)
            slot = secondPlayer ? 9 : 8;
        else if (selector == 0xa)
            slot = secondPlayer ? 0xb : 0xa;
        const reg32 kindOfSound = wordAt(app, entry + 0x7e0);
        if ((kindOfSound == 5 || kindOfSound == 3) && int32_t(slot) > -1)
        {
            if (!g.call(kSlotStop, {slot, reg32(k), reg32(k), slot}, eax))
                return;
            continue;
        }
        if (int32_t(slot) < 0 && gameSettings(app).playMode != 1)
        {
            // A new crash: heard from where it hit.
            reg32 crashAngle;
            if (!g.call(kSourceAngle, {kListener, car + 0x178, kindOfSound, selector}, crashAngle))
                return;
            if (!g.call(kCarKnock, {slot, wordAt(app, entry + 0x7e8), wordAt(app, entry + 0x7ec), wordAt(app, entry + 0x7f0)},
                        {distanceSquared, crashAngle}, eax))
                return;
            int32_t loudness = idiv(int32_t((wordAt(app, entry + 0x7f0) << 7) - wordAt(app, entry + 0x7f0)), 0xa0000);
            if (loudness > 0x7f)
                loudness = 0x7f;
            eax = reg32(loudness);
            if (loudness > 0x7d)
            {
                eax = reg32(k) * 3;
                if (wordAt(app, car + reg32(k) * 24 + 0x7e8) != 6)
                {
                    g.atCall();
                    if (!tookEax(guest::direct<kMusicEventResume, musicEventResumed>(app, cpu, g.esp - 4, eax, 0u), eax))
                        return;
                }
            }
            // The game's trace, compiled out (its words, 0x64, 1, 0x536940 and the loudness, the
            // caller's to push): nothing.
            g.atCall();
            guest::direct<kTrace, ea::traceOff>(app, cpu);
            if (cpu.terminate)
                return;
            continue;
        }
        if (!g.call(kCarKnock, {slot, wordAt(app, entry + 0x7e8), wordAt(app, entry + 0x7ec), wordAt(app, entry + 0x7f0)},
                    {distanceSquared, pan}, eax))
            return;
    }
    eax = reg32(k);
    if ((std::uint8_t(gameSettings(app).raceFlags) & 1) != 0)
        return e.back(cpu, eax);
    const reg32 flagAt = source;  // the source's flag (+0x20 of the pointer's place: +0x38 of the source)
    if ((byteAt(app, car + 0x200) & 0x20) != 0)
    {
        // A cop's car: its siren, and its radio.
        eax = split == 1 ? wordAt(app, car + 0x1f0) + 2 : 4;
        if ((byteAt(app, car + 0x710) & 0x10) != 0)
        {
            if (!g.call(kCopSiren, {eax, pan, ahead, distanceSquared}, eax))
                return;
            app->getMemory<reg32>(flagAt + 0x20) = 1;
        }
        else if (wordAt(app, flagAt + 0x20) != 0)
        {
            if (!g.call(kCopSirenStop, {eax, flagAt}, eax))
                return;
            app->getMemory<reg32>(flagAt + 0x20) = 0;
        }
        if (!g.call(kRadioTick, {car, pan, ahead, distanceSquared}, {player}, eax))
            return;
        return e.back(cpu, eax);
    }
    if (byteAt(app, car + 0x50e) != 0)
    {
        // A horn's noise, by the distance (a 64th of the whole ones).
        const int32_t near = towardZero(int32_t(ahead << 6), 16);
        const reg32 driver = wordAt(app, car + 0x1f0);
        g.esi = driver;
        if (!g.call(kPlayerNoise, {driver, distanceSquared, reg32(near), pan}, eax))
            return;
        app->getMemory<reg32>(flagAt + 0x20) = 1;
        return e.back(cpu, flagAt);
    }
    if (wordAt(app, flagAt + 0x20) != 0)
    {
        if (!g.call(kPlayerNoiseStop, {wordAt(app, car + 0x1f0)}, eax))
            return;
        app->getMemory<reg32>(flagAt + 0x20) = 0;
    }
    e.back(cpu, flagAt);
}

}  // namespace port
}  // namespace nfs3hp
