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
constexpr Var<u32> kCells{0x5dd948};
constexpr Var<uint8_t> kBusUsed{0x552930}, kGearCounter{0x552d8c};
constexpr Table<int32_t> kLoads{0x571078};
constexpr Table<uint8_t> kLoadCurve{0x552a38};
constexpr Table<u32> kPlayerViews{0x5e10b8}, kPlayerLooks{0x5e10b0};
constexpr u32 kListener = 0x570a78;  // the listener: where the sounds are heard from
constexpr u32 kSources = 0x570ad8;   // 0x50 a player's source: the car's pointer at +0x18, a flag at +0x38
constexpr Var<double> kFastSpeed{0x536bb0}, kFastHeight{0x536bb8};

// What the car's sound calls of the game's.
constexpr u32 kDotProduct = 0x4e01f0, kCarEffectFive = 0x418280, kPlaceAhead = 0x4124d0;
constexpr u32 kSourceAngle = 0x410d30, kSourceDistance = 0x410cb0, kCarAngle = 0x410dc0, kCarKnock = 0x416d90;
constexpr u32 kSlotStop = 0x416f60, kMusicEventResume = 0x418c60, kCopSiren = 0x411c50, kCopSirenStop = 0x411ca0;
constexpr u32 kRadioTick = 0x4c8810, kPlayerNoise = 0x417c00, kPlayerNoiseStop = 0x417c70, kTrace = 0x42d510;
constexpr u32 kCoDriverTick = 0x4c8d10, kRaceAnnouncer = 0x412d60, kSceneryTick = 0x415210;
constexpr u32 kFarEffectTick = 0x414f60, kInTunnel = 0x420bf0, kSurfaceUnder = 0x420840;
constexpr u32 kBusUp = 0x418770, kBusDown = 0x418790, kSoundVolume = 0x4e95fc, kPitchBend = 0x4e97e4;
constexpr u32 kPan = 0x4e9950;

u32 wordAt(App* app, u32 at) { return app->getMemory<u32>(at); }
uint8_t byteAt(App* app, u32 at) { return app->getMemory<uint8_t>(at); }
uint16_t halfAt(App* app, u32 at) { return app->getMemory<uint16_t>(at); }
float floatAt(App* app, u32 at) { return app->getMemory<float>(at); }

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
u32 stereoPanOf(u32 pan)
{
    const u32 a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return u32(int32_t(a) >> 8);
}

}  // namespace

/* 0x417390's: car `car`'s engine -- at distance squared `distanceSquared`,
 * `speedShare` times `ahead` (16.16) of throttle, pan `pan`, angle `angle`:
 * the radio (0x4c8d10), the cop chase (0x412d60) and the cars about
 * (0x415210, in arithmetic `a`) first, the far-off effect's tick; then its
 * revs (+0x4f4) as the three engine sounds' (in slots 0, 1 and 12, the second
 * player's 2, 3 and 13) volume, pitch and pan -- louder in a tunnel (the bus
 * on), the gear change's effect now and then.  eax as the code leaves it: the
 * last call's. */
Result engineSounded(App* app, ThreadState& ts, u32 esp, world::Arithmetic a, u32 car, int32_t distanceSquared,
                     int32_t speedShare, int32_t ahead, u32 pan, u32 angle)
{
    const u32 frame = esp - 3 * 4;
    const HeldEbp ebp(ts, frame);
    Game g(app, ts, frame - 0x40);
    const int32_t split = gameSettings(app).playMode;
    const uint8_t driver = byteAt(app, car + 0x1f0);
    const u32 playerSlot = wordAt(app, car + 0x1f0);
    u32 eax;
    g.atCall();
    if (!tookEax(guest::direct<kCoDriverTick, coDriverTicked>(app, ts, g.esp - 4, split == 1 ? int32_t(driver) : 0),
                 eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kRaceAnnouncer, raceAnnouncerTicked>(app, ts, g.esp - 4, car), eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kSceneryTick, scenerySoundsOf>(app, ts, g.esp - 4, a, car, angle), eax))
        return {};
    if (split != 1 || driver == 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kFarEffectTick, farEffectTicked>(app, ts, g.esp - 4, eax), eax))
            return {};
    }
    if (kEffectsOn(app) == 0)
        return int32_t(eax);
    // How close the car is: 127 at the sound's, none from 0x15f9 squared on.
    int32_t near = 0;
    if (distanceSquared < 0x15f9)
        near = int32_t((0x15f9u - u32(distanceSquared)) * 127) / 0x15f9;
    // The revs, cut towards 0: twice their size is the volume, to 127 for the
    // gear change's.
    const int32_t revs = truncated(floatAt(app, car + 0x4f4));
    u32 volume = 2u * u32(revs > 0 ? revs : negated(revs));
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
    // In a tunnel (0x420bf0's, of the car's place and cell): the bus up.  By
    // address, as the scenery's: the code's eax here is the place's y (edx its x, ecx the cell).
    const u32 cell = wordAt(app, car + 0x1c);
    const u32 x = wordAt(app, car + 0x98), y = wordAt(app, car + 0x9c), z = wordAt(app, car + 0xa0);
    g.atCall();
    const std::optional<u32> inTunnel = guest::direct<kInTunnel, trackdata::placeInTunnel>(app, ts, car + 0x98, cell);
    if (!inTunnel)
        return {};
    eax = *inTunnel;
    if (eax != 0)
    {
        if (kBusUsed(app) == 0 && split != 1)
        {
            g.atCall();
            if (!tookEax(guest::direct<kBusUp, busLevelUp>(app, ts, g.esp - 4), eax))
                return {};
            kBusUsed.set(app, 1);
        }
        volume += 0x14;
        pitch = 0x5e;
    }
    else if (kBusUsed(app) != 0 && split != 1)
    {
        g.atCall();
        if (!tookEax(guest::direct<kBusDown, busLevelDown>(app, ts, g.esp - 4), eax))
            return {};
        kBusUsed.set(app, 0);
    }
    // In the split screen the player's side.
    const bool inSplit = split == 1;
    const u32 side = playerSlot != 0 ? 0xc000 : 0x3fff;
    // The ground's kind (0x420840's, else the car's +0x18c): some lower the
    // pitch, some (now and then) the gear change's effect.
    g.atCall();
    if (!tookEax(ts, guest::direct<kSurfaceUnder, trackdata::groundUnder>(app, ts, nfs::s32(cell),
                                                                            nfs::s32(wordAt(app, car + 0x7c))),
                 eax))
        return {};
    const u32 kind = eax != 0 ? eax : wordAt(app, car + 0x18c);
    const u32 ground = kind - 2;
    const bool lower = ground == 0 || ground == 8 || ground == 9 || ground == 0xa || ground == 0xb;
    const bool gearKind = ground == 5 || ground == 0xa || ground == 0xe || ground == 0xf;
    if (lower)
    {
        volume += u32(towardZero(int32_t(volume), 2));
        pitch = 0x18;
    }
    // The gear change's effect.
    const auto gearChange = [&](u32 gearPitch, u32 gearVolume) {
        g.atCall();
        return tookEax(guest::direct<kEffectBank, bankEffect>(app, ts, g.esp - 8, kCarBank(app), 0x1d,
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
            const u32 gearPitch = kind == 7 || kind == 0xc ? 0x28 : 0x40;
            if (!gearChange(gearPitch, u32(towardZero(revsVolume, 1))))
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
        if (turn == 0 && int32_t(wordAt(app, car + 0x15c)) < 0x3e4ccccd && !gearChange(0x40, u32(revsVolume)))
            return {};
    }
    if (distanceSquared > 4)
        volume = u32(idiv(imul(int32_t(volume), 6), 10));
    const int32_t shape = distanceSquared < 4 ? 0x41 : distanceSquared < 0x23 ? 0xc : 0;
    if (int32_t(volume) > 0x7f)
        volume = 0x7f;
    // The load: the revs eased towards the car's (+0x508), the three sounds'
    // share of it (0x552a38's curve).
    const int32_t target = byteAt(app, car + 0x508);
    const int32_t was = kLoads(app, int32_t(playerSlot));
    const int32_t change = int32_t(u32(target) - u32(was));
    kLoads.set(app, int32_t(playerSlot),
               int32_t(u32(was) + u32(target >= was ? towardZero(change, 1) : towardZero(change, 3))));
    int32_t load = kLoads(app, int32_t(playerSlot));
    if (load > 0xff)
        load = 0xff;
    load >>= 1;
    const int32_t rising = kLoadCurve(app, load);
    const int32_t falling = kLoadCurve(app, int32_t(0x80u - u32(load)));
    int32_t share = towardZero(imul(throttle + 0x28, near), 7);
    if (share > 0x7f)
        share = 0x7f;
    if (share < 0)
        share = 0;
    const int32_t falls = imul(falling, share) >> 7;
    const int32_t engine = imul(int32_t(volume), share) >> 7;
    const int32_t rises = imul(rising, share) >> 7;
    // The pan: the game's, the player's side in the split screen.
    u32 panned = std::int32_t(gameSettings(app).replayMode) >= 2 ? stereoPanOf(pan) : 0x40;
    if (inSplit)
        panned = kStereoOn(app) == 0 ? 0x40 : (playerSlot != 0 ? 0 : 0x7f);
    const bool second = inSplit && playerSlot != 0;
    const u32 first = second ? 0x570e50 : 0x570e48, low = second ? 0x570df0 : 0x570de8,
                third = second ? 0x570e00 : 0x570df8;
    // The three sounds: their volume, pitch and pan.
    const u32 setting = u32(gameSettings(app).engineVolume);
    const int32_t scale = int32_t((setting + (((setting << 2) - setting) << 3)) * 2);
    const int32_t lead = shape + 0x32;
    // SND's calls as C++.
    const auto soundVolume = [&](u32 handle, int32_t level) {
        g.atCall();
        return tookEax(guest::direct<kSoundVolume, setVolume>(app, ts, g.esp - 4, handle, level), eax);
    };
    const auto pitchBend = [&](u32 handle, int32_t bend) {
        g.atCall();
        return tookEax(guest::direct<kPitchBend, setBend>(app, ts, g.esp - 4, handle, bend), eax);
    };
    const auto soundPan = [&](u32 handle, u32 to) {
        g.atCall();
        return tookEax(guest::direct<kPan, panSound>(app, ts, g.esp - 4, handle, int32_t(to)), eax);
    };
    if (!soundVolume(wordAt(app, first), imul(scale, engine) >> 14))
        return {};
    if (!pitchBend(wordAt(app, first), pitch))
        return {};
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
nfs::u32 carEngine(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             engineSounded(app, ts, e.esp, world::arithmeticOf(ts.fenv.rounding), e.eax, int32_t(e.edx),
                           int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0), e.arg(app, 1)),
             8);
    return args.eax;
}

/* 0x4117a0's: player `player`'s car's sounds, from its source (0x570ad8's):
 * the engine, the car's knocks and scrapes, the cop's siren and radio or the
 * horn's noise, in arithmetic `a`.  eax as the code leaves it. */
Result playerCarSounded(App* app, ThreadState& ts, u32 esp, world::Arithmetic a, u32 player)
{
    const u32 frame = esp - 6 * 4;
    const u32 place = frame - 0x34, up = frame - 0x40;
    Game g(app, ts, up);
    const u32 source = ((player * 5) << 4) + kSources + 0x18;  // the car's pointer in the player's source
    const u32 car = wordAt(app, source);
    if (car == 0)
        return int32_t(source);
    for (u32 k = 0; k < 3; ++k)
        store<u32>(app, place + 4 * k, wordAt(app, car + 0x98 + 4 * k));
    // The cell's up, and how much of it the car's own has: downhill below 0.
    const u32 cell = wordAt(app, car + 0x1c);
    const u32 cells = kCells(app);
    for (u32 k = 0; k < 3; ++k)
        store<float>(app, up + 4 * k, float(int16_t(int8_t(byteAt(app, cells + cell * 9 * 4 + 0x14 + k)))));
    u32 eax;
    g.atCall();
    float down = float(guest::direct<kDotProduct, vertices::dotProduct>(app, ts, car + 0xd8, up));
    if (ts.terminate)
        return {};
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
            if (count >= 0x40 && count < 0x94 && (count & 0x1f) == 0)
            {
                g.atCall();
                if (!tookEax(guest::direct<kCarEffectFive, carEffectFive>(app, ts, g.esp - 4), eax))
                    return {};
            }
        }
    }
    if (reset)
        app->getMemory<int32_t>(car + 0x998) = 0;
    g.atCall();
    if (!tookEax(guest::direct<kPlaceAhead, world::aheadAt>(app, ts, g.esp - 4, a, place, car + 0xa4), eax))
        return {};
    const u32 ahead = eax;
    // The pan: the player's side in the split screen, else by the angle (none
    // for a view of kind 2).
    u32 pan;
    const int32_t split = gameSettings(app).playMode;
    if (split == 1)
        pan = player != 0 ? 0xc000 : 0x3fff;
    else
    {
        const u32 view = kPlayerViews(app, int32_t(player));
        u32 kindOfView;
        if (view != 0)
            kindOfView = wordAt(app, view);
        else
        {
            const u32 look = kPlayerLooks(app, int32_t(player));
            kindOfView = look != 0 ? wordAt(app, look) : 0;
        }
        if (kindOfView == 2)
        {
            pan = 0;
        }
        else
        {
            g.atCall();
            if (!tookEax(guest::direct<kSourceAngle, world::angleAt>(app, ts, g.esp - 4, a, kListener, place), pan))
                return {};
        }
    }
    g.atCall();
    if (!tookEax(guest::direct<kSourceDistance, world::distanceAt>(app, ts, a, kListener, place), eax))
        return {};
    const int32_t whole = towardZero(int32_t(eax), 16);
    const u32 distanceSquared = u32(imul(whole, whole));
    // How loud the car is for its speed: its top (+0x590 of 127 over the
    // engine's own limit, which the car's tyre and gear change).
    const u32 tyre = wordAt(app, wordAt(app, car + 0x220) + 0x1c);
    u32 limit = wordAt(app, wordAt(app, car + 0x528) + 0x1c4) + (tyre == 1 ? 0x3e8 : 0x7d0);
    if (halfAt(app, car + 0x164) > 5)
        limit -= 0x1f4;
    const u32 speed = wordAt(app, car + 0x590);
    const int32_t speedShare = idiv(int32_t((speed << 7) - speed), int32_t(limit));
    u32 angle;
    g.atCall();
    if (!tookEax(guest::direct<kCarAngle, world::carAngleAt>(app, ts, g.esp - 4, a, kListener, car), angle))
        return {};
    if (!engineSounded(app, ts, up - 4 - 8, a, car, int32_t(distanceSquared), speedShare, int32_t(ahead), pan, angle))
        return {};
    // The car's knocks and scrapes: 24 bytes each from +0x7e0; one played at `towards`.
    const auto knock = [&](u32 slot, u32 entry, u32 towards) {
        g.atCall();
        return tookEax(guest::direct<kCarKnock, sources::knockPlayed>(
                           app, ts, g.esp - 4 - 2 * 4, int32_t(slot), wordAt(app, entry + 0x7e8),
                           wordAt(app, entry + 0x7ec), int32_t(wordAt(app, entry + 0x7f0)),
                           int32_t(distanceSquared), towards),
                       eax);
    };
    const int32_t count = int32_t(wordAt(app, car + 0x7dc));
    const bool secondPlayer = player != 0;
    int32_t k = 0;
    for (; k < count; ++k)
    {
        const u32 entry = car + u32(k) * 24;
        const u32 selector = wordAt(app, entry + 0x7e4);
        u32 slot = selector;
        if (selector == 8)
            slot = secondPlayer ? 9 : 8;
        else if (selector == 0xa)
            slot = secondPlayer ? 0xb : 0xa;
        const u32 kindOfSound = wordAt(app, entry + 0x7e0);
        if ((kindOfSound == 5 || kindOfSound == 3) && int32_t(slot) > -1)
        {
            g.atCall();
            if (!tookEax(guest::direct<kSlotStop, slotStopped>(app, ts, g.esp - 4, int32_t(slot)), eax))
                return {};
            continue;
        }
        if (int32_t(slot) < 0 && gameSettings(app).playMode != 1)
        {
            // A new crash: heard from where it hit.
            u32 crashAngle;
            g.atCall();
            if (!tookEax(guest::direct<kSourceAngle, world::angleAt>(app, ts, g.esp - 4, a, kListener, car + 0x178),
                         crashAngle))
                return {};
            if (!knock(slot, entry, crashAngle))
                return {};
            int32_t loudness = idiv(int32_t((wordAt(app, entry + 0x7f0) << 7) - wordAt(app, entry + 0x7f0)), 0xa0000);
            if (loudness > 0x7f)
                loudness = 0x7f;
            eax = u32(loudness);
            if (loudness > 0x7d)
            {
                eax = u32(k) * 3;
                if (wordAt(app, car + u32(k) * 24 + 0x7e8) != 6)
                {
                    g.atCall();
                    if (!tookEax(guest::direct<kMusicEventResume, musicEventResumed>(app, ts, g.esp - 4, eax, 0u), eax))
                        return {};
                }
            }
            // The game's trace, compiled out (its words, 0x64, 1, 0x536940 and the loudness, the
            // caller's to push): nothing.
            g.atCall();
            guest::direct<kTrace, ea::traceOff>(app, ts);
            if (ts.terminate)
                return {};
            continue;
        }
        if (!knock(slot, entry, pan))
            return {};
    }
    eax = u32(k);
    if ((std::uint8_t(gameSettings(app).raceFlags) & 1) != 0)
        return int32_t(eax);
    const u32 flagAt = source;  // the source's flag (+0x20 of the pointer's place: +0x38 of the source)
    if ((byteAt(app, car + 0x200) & 0x20) != 0)
    {
        // A cop's car: its siren, and its radio.
        eax = split == 1 ? wordAt(app, car + 0x1f0) + 2 : 4;
        if ((byteAt(app, car + 0x710) & 0x10) != 0)
        {
            g.atCall();
            if (!tookEax(guest::direct<kCopSiren, copSirenPlayed>(app, ts, g.esp - 4, int32_t(eax),
                                                                  int32_t(distanceSquared), pan, int32_t(ahead)),
                         eax))
                return {};
            app->getMemory<u32>(flagAt + 0x20) = 1;
        }
        else if (wordAt(app, flagAt + 0x20) != 0)
        {
            g.atCall();
            if (!tookEax(guest::direct<kCopSirenStop, copSirenStopped>(app, ts, g.esp - 4, int32_t(eax)), eax))
                return {};
            app->getMemory<u32>(flagAt + 0x20) = 0;
        }
        g.atCall();
        return guest::direct<kRadioTick, radioSoundTicked>(app, ts, g.esp - 4 - 4, car, pan, int32_t(ahead),
                                                           int32_t(distanceSquared), int32_t(player));
    }
    if (byteAt(app, car + 0x50e) != 0)
    {
        // A horn's noise, by the distance (a 64th of the whole ones).
        const int32_t near = towardZero(int32_t(ahead << 6), 16);
        const u32 driver = wordAt(app, car + 0x1f0);
        g.atCall();
        if (!guest::direct<kPlayerNoise, playerNoisePlayed>(app, ts, g.esp - 4, driver, int32_t(distanceSquared),
                                                           u32(near), pan))
            return {};
        app->getMemory<u32>(flagAt + 0x20) = 1;
        return int32_t(flagAt);
    }
    if (wordAt(app, flagAt + 0x20) != 0)
    {
        g.atCall();
        if (!guest::direct<kPlayerNoiseStop, playerNoiseStopped>(app, ts, g.esp - 4, wordAt(app, car + 0x1f0)))
            return {};
        app->getMemory<u32>(flagAt + 0x20) = 0;
    }
    return int32_t(flagAt);
}

/* 0x4117a0: player eax's car's sounds. */
nfs::u32 playerCarSound(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, playerCarSounded(app, ts, e.esp, world::arithmeticOf(ts.fenv.rounding), e.eax));
}

}  // namespace port
}  // namespace nfs3hp
