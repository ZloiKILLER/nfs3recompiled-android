#include "game_settings.h"
#include "native_trackdata.h"
#include "native_ea_support.h"
#include "port_snd_3d.h"
#include "port_snd_voices.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"

#include <cstdint>

/* The tracks' own scenery sounds, rewritten (port.h says how): for the
 * player's car each tick, by the track (0x6fd4ac) and where on it the car is
 * (its cell, +0x1c) -- the rain and the tunnel's echo; a sound that plays all
 * along a stretch, swelling and fading at random (slot 0x1a, the second
 * player's 0x24); and now and then, at random, the scenery's effects of the
 * ambience bank (0x571148): birds, trains, bells, voices, some repeating a few
 * times over (0x570f70's, 0x1c bytes a player).  Each the code's own way, the
 * random numbers drawn in its order.
 *
 * The game's state, per player: 0x570de0 the rain on (a byte), 0x570f50 its
 * sound, 0x570f48 in a stretch (a byte), 0x570f40 a zone's sound playing (a
 * byte), 0x570f20 the stretch's volume and 0x571050 where it heads, 0x571120
 * and 0x57111c the stretch's pan and where it heads.
 *
 * What the code keeps in esi and edi is kept in Calls' (port_snd_3d.h), for the
 * sounds it starts below to read where they read what the code never set. */
namespace nfs3hp
{
namespace port
{
namespace world
{

namespace
{

#pragma pack(push, 1)
// A repeating call: 0x570f70 on, one a player.
struct Repeat
{
    int32_t times;        // left
    uint32_t pan;
    uint16_t ticks;       // until the next
    uint16_t gap;         // between them
    uint8_t effect;
    uint8_t pitch;
    uint8_t volume;
    uint8_t other0f;
    uint8_t other10[0xc];
};
static_assert(sizeof(Repeat) == 0x1c, "a repeating call");
struct Slot
{
    int32_t sound;
    int32_t effect;
};
#pragma pack(pop)

constexpr Var<u32> kAmbience{0x571148}, kRainBank{0x571154};
constexpr Table<int32_t> kWeather{0x564288};
constexpr Table<uint8_t> kRainOn{0x570de0};
constexpr Table<u32> kRainSound{0x570f50};
constexpr Table<uint8_t> kInStretch{0x570f48}, kZoneSound{0x570f40};
constexpr Table<int32_t> kStretchVolume{0x570f20}, kStretchTarget{0x571050};
constexpr Var<int32_t> kStretchPan{0x571120}, kStretchPanTarget{0x57111c};
constexpr Table<Slot> kSlots{0x570de8};
constexpr Table<int32_t> kZoneTable{0x552d54};
constexpr Var<uint8_t> kRainAlways{0x552909};
constexpr Var<int32_t> kHornSpot{0x552d84}, kCheerSpot{0x552d88}, kBellLap{0x552c34}, kBellState{0x552c38};
constexpr Records<Repeat> kRepeats{0x570f70};
constexpr u32 kTrainPlace = 0x412988;
constexpr Var<double> kTrainUnit{0x536ba8};
// What it calls: the trace (compiled out), the tunnel's test (by address: the function may leave
// the eax it was called with, which its typed function's call does not set), the sound rewrites (as C++).
constexpr u32 kTrace = 0x42d510, kInTunnel = 0x420bf0, kZone = 0x415050;
constexpr u32 kPlayEffect = 0x4182b0, kSlotPlay = 0x417080, kSlotStop = 0x416f60, kSoundFade = 0x4e9420;
constexpr u32 kSoundVolume = 0x4e95fc;

// The pans: a side, and what the code makes of the heading.
constexpr u32 kLeft = 0x3fff, kRight = 0xc000;

int32_t signedAt(App* app, u32 at) { return int32_t(guest::at(app, at)); }

/* The code's signed shift towards 0 by 16. */
int32_t shiftedTowardZero(int32_t n) { return n < 0 ? int32_t(uint32_t(n) + 0xffff) >> 16 : n >> 16; }

struct Roll
{
    u32 quotient, remainder;
};

Result sceneryIn(App* app, ThreadState& ts, u32 entryEsp, Arithmetic arithmetic,
                 u32 car, u32 heading)
{
    // Five pushes, and a frame of 0x44.
    const u32 frame = entryEsp - 5 * 4, below = frame - 0x44;
    const HeldEbp ebp(ts, frame);
    Calls game{app, ts, below};
    const auto player = [&] { return guest::at(app, car + 0x1f0); };
    const auto cell = [&] { return signedAt(app, car + 0x1c); };
    const auto track = [&] { return std::int32_t(gameSettings(app).track); };
    const auto random = [&] { return nextRandom(app); };
    const auto roll = [&](u32 by) {
        const u32 r = random();
        return Roll{r / by, r % by};
    };
    const auto inside = [&](int32_t lo, int32_t hi) { return cell() >= lo && cell() <= hi; };
    const auto bits = [&](u32 at) { return app->getMemory<uint8_t>(at); };

    if (gameSettings(app).viewedPlayer != gameSettings(app).localPlayer)
    {
        // The game's trace, compiled out (its words, 0x64, 2 and the text, the caller's to push): nothing, eax
        // left as the code has it, the car.
        game.atCall();
        guest::direct<kTrace, ea::traceOff>(app, ts);
        if (ts.terminate)
            return {};
        return int32_t(car);
    }
    // In a tunnel (0x420bf0's, of the car's place), the rain (the same with the
    // player's weather, 0x564288).
    const u32 x = guest::at(app, car + 0x98), y = guest::at(app, car + 0x9c), z = guest::at(app, car + 0xa0);
    // (The place's x, y and z are the car's three words from +0x98.)
    game.atCall();
    const std::optional<u32> inTunnel = guest::direct<kInTunnel, trackdata::placeInTunnel>(app, ts, car + 0x98, u32(cell()));
    if (!inTunnel)
        return {};
    const u32 tunnel = *inTunnel;
    const u32 weather = gameSettings(app).playMode == 1 ? player() : 0;
    const u32 rain = guest::at(app, kWeather.address(int32_t(weather)));
    game.atCall();
    const std::optional<u32> inEcho = guest::direct<kInTunnel, trackdata::placeInTunnel>(app, ts, car + 0x98, u32(cell()));
    if (!inEcho)
        return {};
    const u32 echo = *inEcho;
    if (rain != 0)
    {
        int32_t level = int32_t(rain);
        if (level > 0x64)
            level = 0x64;
        const u32 rainNow = guest::at(app, kWeather.address(int32_t(weather)));
        const uint8_t on = kRainOn(app, int32_t(weather));
        if (rainNow != 0 && on == 0)
        {
            const u32 pan = gameSettings(app).playMode == 1 ? (weather != 0 ? kRight : kLeft) : 0;
            const int32_t volume = idiv(imul(level, 127), 100);
            game.atCall();
            const Result sound =
                guest::direct<kPlayEffect, bankEffect>(app, ts, below - 8, kRainBank(app), 4, volume, pan, uint8_t(0x40));
            if (!sound)
                return {};
            guest::put(app, kRainSound.address(int32_t(weather)), u32(*sound));
            app->getMemory<uint8_t>(kRainOn.address(int32_t(weather))) = 1;
        }
        else if (rainNow == 0 && on != 0)
        {
            game.atCall();
            if (!guest::direct<kSoundFade, fadeSound>(app, ts, below - 4, guest::at(app, kRainSound.address(int32_t(weather))),
                                                      5, -1))
                return {};
            app->getMemory<uint8_t>(kRainOn.address(int32_t(weather))) = 0;
        }
        else if (rainNow != 0 && on != 0)
        {
            // Louder with the effects' volume and the rain.
            const int32_t loudness = imul(level, imul(std::int32_t(gameSettings(app).effectsVolume), 63));
            const int32_t volume = idiv(loudness, 100) >> 7;
            game.atCall();
            if (!guest::direct<kSoundVolume, setVolume>(app, ts, below - 4,
                                                        guest::at(app, kRainSound.address(int32_t(weather))), volume))
                return {};
        }
        if (echo != 0 || kRainAlways(app) == 0)
        {
            game.atCall();
            if (!guest::direct<kSoundVolume, setVolume>(app, ts, below - 4,
                                                        guest::at(app, kRainSound.address(int32_t(weather))), 0))
                return {};
        }
    }
    // The stretch's slot; the player's side in the split screen.
    const bool split = gameSettings(app).playMode == 1;
    const u32 sidePan = player() != 0 ? kRight : kLeft;
    const u32 slot = split && player() != 0 ? 0x24 : 0x1a;
    // The pans the effects take: a side, the heading less or more a quarter,
    // or at random.
    const auto pickedPan = [&] { return split ? sidePan : random() % 0xffff; };
    const auto turnedPan = [&](bool mirrorFirst) {
        if (split)
            return sidePan;
        const bool mirror = gameSettings(app).mirrored != 0;
        const u32 add = mirror == mirrorFirst ? 0xc000 : 0x2000;
        return ((random() & 0x3fff) + add + heading) & 0xffff;
    };
    const auto headingPan = [&](bool mirrorFirst) {
        if (split)
            return sidePan;
        const bool mirror = gameSettings(app).mirrored != 0;
        return (heading + (mirror == mirrorFirst ? 0xc000 : 0x3fff)) & 0xffff;
    };
    const auto play = [&](u32 effect, u32 pitch, u32 pan, u32 volume, u32& result) {
        game.atCall();
        const Result r = guest::direct<kPlayEffect, bankEffect>(app, ts, below - 8, kAmbience(app), int32_t(effect),
                                                                int32_t(volume), pan, uint8_t(pitch));
        if (!r)
            return false;
        result = u32(*r);
        return true;
    };
    const auto playOnly = [&](u32 effect, u32 pitch, u32 pan, u32 volume) {
        u32 ignored;
        return play(effect, pitch, pan, volume, ignored);
    };
    const auto slotStop = [&](u32 s) {
        game.atCall();
        return bool(guest::direct<kSlotStop, slotStopped>(app, ts, below - 4, int32_t(s)));
    };
    // The slot's sound played: the stretch's volume at its slot.
    const auto playSlot = [&](u32 effect, u32 pan) {
        const u32 volume = guest::at(app, kStretchVolume.address(int32_t(player())));
        game.atCall();
        return bool(guest::direct<kSlotPlay, slotPlayed>(app, ts, below - 8, int32_t(slot), int32_t(effect),
                                                         int32_t(volume), int32_t(volume), pan));
    };
    // A stretch entered: its sound started (`effect`), the old zone's stopped.
    const auto stretchStart = [&](u32 effect, bool slowPan) {
        const uint8_t zoned = bits(kInStretch.address(int32_t(player())));
        if (bits(kZoneSound.address(int32_t(player()))) != 0)
        {
            if (!slotStop(slot))
                return false;
            app->getMemory<uint8_t>(kZoneSound.address(int32_t(player()))) = zoned;
        }
        guest::put(app, kStretchPan.at, 0);
        guest::put(app, kStretchVolume.address(int32_t(player())), 1);
        guest::put(app, kStretchTarget.address(int32_t(player())), random() & 0x7f);
        guest::put(app, kStretchPanTarget.at, slowPan ? random() & 0x1ff : random() % 0xffff);
        const u32 pan = split ? sidePan : guest::at(app, kStretchPan.at);
        if (!playSlot(effect, pan))
            return false;
        app->getMemory<uint8_t>(kInStretch.address(int32_t(player()))) = 1;
        return true;
    };
    // Out of every stretch of the track: no longer in one.
    const auto stretchLeft = [&] {
        if (bits(kInStretch.address(int32_t(player()))) != 0)
            app->getMemory<uint8_t>(kInStretch.address(int32_t(player()))) = 0;
    };
    // One stretch of a track: from lo to hi, its effect.
    const auto stretch = [&](int32_t lo, int32_t hi, int32_t forTrack, u32 effect, bool slowPan) {
        const bool in = cell() >= lo && cell() <= hi;
        if (in && track() == forTrack && bits(kInStretch.address(int32_t(player()))) == 0)
            return stretchStart(effect, slowPan);
        if (!in && track() == forTrack)
            stretchLeft();
        return true;
    };
    if (!stretch(0x26c, 0x3a0, 8, 8, false) || !stretch(0x28c, 0x3b1, 3, 6, false) || !stretch(0x28a, 0x3ca, 7, 6, true))
        return {};
    if (track() == 4)
    {
        const int32_t c = cell();
        const bool in = (c >= 0x2c6 && c <= 0x2e4) || (c >= 0x34c && c <= 0x35a) || (c >= 0x3b2 && c <= 0x3d8);
        if (in)
        {
            if (bits(kInStretch.address(int32_t(player()))) == 0 && !stretchStart(3, false))
                return {};
        }
        else
            stretchLeft();
    }
    // The stretch's sound: swelling and fading, its pan drifting; faded out in
    // a tunnel; else the zones' sounds (0x415050).
    if (bits(kInStretch.address(int32_t(player()))) != 0)
    {
        const u32 effect = track() == 3 || track() == 7 ? 6 : track() == 4 ? 3 : 8;
        if (tunnel != 0)
        {
            game.atCall();
            if (!guest::direct<kSoundFade, fadeSound>(app, ts, below - 4, u32(kSlots(app, int32_t(slot)).sound), 0x32,
                                                      0))
                return {};
            guest::put(app, kStretchVolume.address(int32_t(player())), 0);
            guest::put(app, kStretchTarget.address(int32_t(player())), 0);
        }
        else if (guest::at(app, kStretchVolume.address(int32_t(player()))) ==
                 guest::at(app, kStretchTarget.address(int32_t(player()))))
            guest::put(app, kStretchTarget.address(int32_t(player())), random() & 0x7f);
        else
        {
            const int32_t target = kStretchPanTarget(app);
            int32_t pan = kStretchPan(app);
            if (pan == target)
                guest::put(app, kStretchPanTarget.at, random() & 0x1ff);
            else
            {
                pan += pan < target ? 1 : -1;
                kStretchPan.set(app, pan);
                if (pan < 0)
                    kStretchPan.set(app, 0);
                else if (pan > 0x1ff)
                    kStretchPan.set(app, 0x1ff);
            }
            const u32 at = kStretchVolume.address(int32_t(player()));
            const int32_t volume = signedAt(app, at);
            guest::put(app, at, u32(volume < signedAt(app, kStretchTarget.address(int32_t(player()))) ? volume + 1
                                                                                                    : volume - 1));
            if (!playSlot(effect, split ? sidePan : u32(kStretchPan(app)) << 7))
                return {};
        }
    }
    else if (guest::at(app, kStretchVolume.address(int32_t(player()))) != 0)
    {
        const u32 effect = track() == 3 || track() == 7 ? 6 : track() == 4 ? 3 : 8;
        const u32 at = kStretchVolume.address(int32_t(player()));
        guest::put(app, at, guest::at(app, at) - 1);
        if (!playSlot(effect, split ? sidePan : u32(kStretchPan(app))))
            return {};
    }
    else
    {
        game.atCall();
        const Result zone = guest::direct<kZone, zoneSoundPlayed>(app, ts, below - 4, int32_t(slot), car,
                                                                  u32(kZoneTable(app, track())), heading);
        if (!zone)
            return {};
        if (*zone != 0)
            app->getMemory<uint8_t>(kZoneSound.address(int32_t(player()))) = 1;
        else if (bits(kZoneSound.address(int32_t(player()))) != 0)
        {
            if (!slotStop(slot))
                return {};
            app->getMemory<uint8_t>(kZoneSound.address(int32_t(player()))) = 0;
        }
    }
    // The scenery's effects, now and then.  Track 8: two kinds of bird.
    if (inside(0x3d4, 0x45b) && track() == 8)
    {
        if (roll(0x1f40).remainder > 0x1f27)
        {
            const u32 pitch = random() & 0x7f;
            const u32 volume = roll(0x28).remainder + 0x32;
            const u32 pan = pickedPan();
            if (!playOnly(0xb, pitch, pan, volume))
                return {};
        }
        if (roll(0x1f40).remainder > 0x1edc)
        {
            const u32 pitch = (random() & 0x1f) + 0x40;
            const u32 volume = roll(0x28).remainder + 0x32;
            const u32 pan = pickedPan();
            if (!playOnly(9, pitch, pan, volume))
                return {};
        }
    }
    // Track 8, away from two stretches: two more.
    {
        const int32_t c = cell();
        const bool away = c > 0x4c2 || c < 0x49 || (c > 0x7e && c < 0xb1);
        if (away && track() == 8)
        {
            if (roll(0x1f40).remainder > 0x1e14)
            {
                const u32 pan = turnedPan(true);
                const u32 pitch = (random() & 0x1f) + 0x40;
                const u32 volume = roll(0x28).remainder + 0x32;
                if (!playOnly(2, pitch, pan, volume))
                    return {};
            }
            if (roll(0x1f40).remainder > 0x1f2c)
            {
                const u32 pan = turnedPan(true);
                const u32 volume = roll(0x28).remainder + 0x1e;
                if (!playOnly(1, 0x40, pan, volume))
                    return {};
            }
        }
    }
    // Track 6: a repeating call.
    if (inside(0x2f0, 0x3de) && track() == 6)
    {
        Repeat& call = kRepeats(app, int32_t(player()));
        const u32 callAt = kRepeats.address(int32_t(player()));
        if (call.times != 0)
        {
            const uint16_t ticks = uint16_t(call.ticks - 1);
            call.ticks = ticks;
            if (ticks == 0)
            {
                const u32 pan = split ? sidePan : (call.pan + heading) & 0xffff;
                if (!playOnly(call.effect, call.pitch, pan, call.volume))
                    return {};
                call.ticks = call.gap;
                --call.times;
            }
        }
        else if (roll(0x1f40).remainder > 0x1ef5)
        {
            const u32 effect = roll(3).remainder + 0x11;
            if (effect == 0x13)
            {
                random();
                call.times = 1;
            }
            else
                call.times = int32_t(roll(5).remainder + 3);
            const uint16_t length = uint16_t((random() & 0xf) + 0x10);
            call.effect = uint8_t(effect);
            call.gap = length;
            call.ticks = length;
            call.pitch = uint8_t((random() & 0x3f) + 0x40);
            call.volume = uint8_t((random() & 0x3f) + 0x40);
            if (split)
                call.pan = sidePan;
            else
            {
                const bool mirror = gameSettings(app).mirrored != 0;
                call.pan = (random() & 0x3fff) + (mirror ? 0xc000 : 0x2000);
            }
        }
    }
    // Track 6, out of tunnels, in three stretches: a gust.
    if (track() == 6 && tunnel == 0)
    {
        const int32_t c = cell();
        if ((c > 0xdf && c < 0x13d) || (c > 0x163 && c < 0x1a4) || (c > 0x311 && c < 0x352))
        {
            if (roll(0xc8).remainder > 0xbe)
            {
                u32 pan;
                if (split)
                    pan = sidePan;
                else
                {
                    const bool mirror = gameSettings(app).mirrored != 0;
                    pan = ((random() & 0x3fff) + (mirror ? 0x2000 : 0xc000) + heading) & 0xffff;
                }
                const u32 pitch = random() & 0x7f;
                const u32 volume = (random() & 0x3f) + 0x40;
                if (!playOnly(8, pitch, pan, volume))
                    return {};
            }
        }
    }
    // Tracks 3 and 7: now and then.
    if (inside(0x28c, 0x3b1))
    {
        if (track() == 3 || track() == 7)
        {
            if (roll(0xfa0).remainder > 0xf96)
            {
                const u32 pitch = (random() & 0x3f) + 0x40;
                const u32 volume = (random() & 0x3f) + 0x40;
                const u32 pan = pickedPan();
                if (!playOnly(3, pitch, pan, volume))
                    return {};
            }
        }
    }
    // Track 4, at a gate when the car brakes: a horn, and the call's count.
    if ((cell() == 0x163 || cell() == 0x164) && track() == 4 && guest::at(app, car + 0x18c) == 0xa &&
        app->getMemory<uint16_t>(car + 0x74) != 0)
    {
        Repeat& call = kRepeats(app, int32_t(player()));
        call.times = 0x140;
        call.ticks = 0x10;
        if (!playOnly(0xa, 0x40, split ? sidePan : 0, 0x7f))
            return {};
    }
    // Track 4: the train, as it passes.
    if (inside(0x12c, 0x1f4) && track() == 4)
    {
        Repeat& call = kRepeats(app, int32_t(player()));
        if (call.times != 0)
        {
            const uint16_t ticks = call.ticks;
            if (ticks != 0)
            {
                call.ticks = uint16_t(ticks - 1);
            }
            else
            {
                const Roll rolled = roll(0x1f4);
                const int32_t threshold = int32_t(0x1f4u - uint32_t(call.times));
                if (int32_t(rolled.remainder) > threshold)
                {
                    // Its place: 0x412988's, scaled.
                    const double unit = kTrainUnit(app);
                    const Vec3 place = withArithmetic(arithmetic, [&](auto real) {
                        using Real = decltype(real);
                        return Vec3{float(times<Real>(double(signedAt(app, kTrainPlace)), unit)),
                                    float(times<Real>(double(signedAt(app, kTrainPlace + 4)), unit)),
                                    float(times<Real>(double(signedAt(app, kTrainPlace + 8)), unit))};
                    });
                    const int32_t distance = distanceBetween(app, arithmetic, vectorAt(app, kListener), place);
                    const int32_t reach = shiftedTowardZero(distance);
                    const int32_t reached = imul(reach, reach);
                    const int32_t volume = reached > 0x57e ? 0 : idiv(imul(0x57e - reached, 127), 0x57e);
                    u32 pan;
                    if (split)
                        pan = sidePan;
                    else
                    {
                        const Result angle = angleToPlace(app, ts, below - 4, arithmetic, kListener, place);
                        if (!angle)
                            return {};
                        pan = u32(*angle);
                    }
                    const u32 effect = (random() & 1) + 6;
                    if (!playOnly(effect, 0x40, pan, u32(volume)))
                        return {};
                }
            }
            --kRepeats(app, int32_t(player())).times;
        }
    }
    // Track 4, in three stretches: two kinds of noise.
    for (int kind = 0; kind < 2; ++kind)
    {
        const int32_t c = cell();
        const bool in = (c >= 0xeb && c <= 0x136) || (c >= 0x19f && c <= 0x24e) || (c >= 0x262 && c <= 0x3fa);
        if (!(in && track() == 4))
            continue;
        if (roll(0x1f40).remainder <= (kind == 0 ? 0x1edcu : 0x1f2cu))
            continue;
        const u32 volume = roll(0x4b).remainder + 5;
        if (kind == 0)
        {
            const u32 pan = pickedPan();
            if (!playOnly(5, 0x40, pan, volume))
                return {};
        }
        else
        {
            const u32 pan = pickedPan();
            const u32 effect = (random() & 1) + 8;
            if (!playOnly(effect, 0x40, pan, volume))
                return {};
        }
    }
    // Track 0, in two stretches.
    {
        const int32_t c = cell();
        if (((c >= 0xeb && c <= 0x1e5) || (c >= 0x244 && c <= 0x352)) && track() == 0)
        {
            if (roll(0x1f40).remainder > 0x1e78)
            {
                const u32 pitch = random() & 0x7f;
                const u32 volume = roll(0x64).remainder + 0x1c;
                const u32 pan = pickedPan();
                const u32 effect = roll(3).remainder + 0xb;
                if (!playOnly(effect, pitch, pan, volume))
                    return {};
            }
        }
    }
    // Track 1, near the end.
    if (inside(0x409, 0x44c) && track() == 1)
    {
        if (roll(0xfa0).remainder > 0xf96)
        {
            u32 pan;
            if (split)
                pan = sidePan;
            else
            {
                const bool mirror = gameSettings(app).mirrored != 0;
                pan = ((random() & 0x3fff) + (mirror ? 0x2000 : 0xc000) + heading) & 0xffff;
            }
            const u32 pitch = (random() & 0x1f) + 0x40;
            const u32 volume = roll(0x28).remainder + 0x32;
            if (!playOnly(0xb, pitch, pan, volume))
                return {};
        }
    }
    // Tracks 1 and 5, braking at a spot: a call.
    if (cell() == 0xa6 || cell() == 0xa7)
    {
        if ((track() == 1 || track() == 5) && guest::at(app, car + 0x18c) == 0xa &&
            app->getMemory<uint16_t>(car + 0x74) != 0)
            if (!playOnly(3, 0x40, split ? sidePan : 0, 0x7f))
                return {};
    }
    // Track 2: a call repeating, louder each time; or a single one.
    if (inside(0x398, 0x42a) && track() == 2)
    {
        Repeat& call = kRepeats(app, int32_t(player()));
        if (call.times != 0)
        {
            const uint16_t ticks = uint16_t(call.ticks - 1);
            call.ticks = ticks;
            if (ticks == 0)
            {
                if (!playOnly(call.effect, call.pitch, call.pan, call.volume))
                    return {};
                call.ticks = call.gap;
                call.pitch = uint8_t(call.pitch + 4);
                if (call.pitch > 0x7f)
                    call.pitch = 0x7f;
                --call.times;
            }
        }
        if (roll(0x1f40).remainder > 0x1edc)
        {
            const u32 effect = roll(5).remainder + 0xb;
            if (int32_t(effect) >= 0xd && call.times == 0)
            {
                call.times = int32_t(roll(7).remainder + 1);
                const uint16_t length = uint16_t((random() & 0xf) + 8 + ((effect - 0xb) << 3));
                call.gap = length;
                call.effect = uint8_t(effect);
                call.ticks = length;
                call.pan = split ? sidePan : random() % 0xffff;
                call.pitch = uint8_t((random() & 0x3f) + 0x40);
                call.volume = uint8_t((random() & 0x3f) + 0x3a);
            }
            else
            {
                const u32 pitch = (random() & 0x3f) + 0x40;
                // The code's ecx stays 0x30 through its division.
                const u32 volume = 0x30 + roll(0x30).remainder;
                const u32 pan = pickedPan();
                if (!playOnly(effect, pitch, pan, volume))
                    return {};
            }
        }
    }
    // Track 5, in a stretch: now and then.
    {
        const int32_t c = cell();
        if (c > 0x1f4 && c < 0x2bc && track() == 5)
        {
            if (roll(0x1388).remainder > 0x1374)
            {
                const u32 volume = roll(0x25).remainder + 0x5a;
                const u32 pan = pickedPan();
                const u32 effect = (random() & 1) + 0xa;
                if (!playOnly(effect, 0x40, pan, volume))
                    return {};
            }
        }
    }
    // Track 1: three more, by stretch.
    const auto rare = [&](bool in, u32 limit, u32 effect) {
        if (!in || track() != 1)
            return true;
        if (roll(0x1388).remainder <= limit)
            return true;
        const u32 pan = pickedPan();
        return playOnly(effect, 0x40, pan, 0x7f);
    };
    {
        const int32_t c = cell();
        if (!rare(c > 0xc8 && c < 0x13b, 0x1356, 0x12))
            return {};
    }
    {
        const int32_t c = cell();
        if (!rare((c > 0x168 && c < 0x1bd) || (c > 0x355 && c < 0x3b6), 0x137e, 0x10))
            return {};
    }
    // Track 1, at the start, on lap 3: a cheer.
    if ((cell() == 0xb || cell() == 0xc) && track() == 1 && guest::at(app, car + 0x638) == 3)
    {
        if (roll(0x1388).remainder > 0x1324)
        {
            const u32 pan = headingPan(true);
            const u32 pitch = roll(0x50).remainder;
            if (!playOnly(0xe, pitch, pan, 0x7f))
                return {};
        }
    }
    // Track 4: once on its stretch after lap 9, once at its spot on lap 2.
    if (cell() > 0x13b && cell() < 0x143 && track() == 4 && signedAt(app, car + 0x638) > 9 && kHornSpot(app) == 0)
    {
        const u32 pan = headingPan(true);
        if (!playOnly(0xd, 0x40, pan, 0x7f))
            return {};
        kHornSpot.set(app, 1);
    }
    if (!(cell() >= 0x13b && cell() <= 0x143) && track() == 4)
        kHornSpot.set(app, 0);
    if (cell() == 0x161 && track() == 4 && guest::at(app, car + 0x638) == 2 && kCheerSpot(app) == 0)
    {
        const u32 pitch = random() & 0x3f;
        if (!playOnly(0xf, pitch, split ? sidePan : 0, 0x7f))
            return {};
        kCheerSpot.set(app, 1);
    }
    if (cell() == 0x189 && guest::at(app, car + 0x638) == 0xb && track() == 4)
    {
        kCheerSpot.set(app, 0);
    }
    // Track 0: two more.
    if (cell() > 0x41 && cell() < 0x55 && track() == 0)
    {
        if (roll(0x1388).remainder > 0x137e)
        {
            const u32 pan = headingPan(true);
            if (!playOnly(0xa, 0x40, pan, 0x7f))
                return {};
        }
    }
    if (cell() > 0x3a7 && cell() < 0x3ac && signedAt(app, car + 0x638) > 0xa && track() == 0)
    {
        if (roll(0x1388).remainder > 0x1374)
        {
            const u32 pan = headingPan(true);
            const u32 effect = (random() & 1) + 0x10;
            if (!playOnly(effect, 0x40, pan, 0x7f))
                return {};
        }
    }
    // Track 3, at either end: a call repeating, the pan moving on, quieter each
    // time; and now and then another.
    const auto ends = [&] {
        const int32_t c = cell();
        return (c >= 0x14 && c <= 0x190) || (c >= 0x578 && c <= 0x5c8);
    };
    if (ends() && track() == 3)
    {
        Repeat& call = kRepeats(app, int32_t(player()));
        if (call.times != 0)
        {
            const uint16_t ticks = uint16_t(call.ticks - 1);
            call.ticks = ticks;
            if (ticks == 0)
            {
                if (!playOnly(call.effect, call.pitch, call.pan, call.volume))
                    return {};
                call.ticks = call.gap;
                const u32 moved = call.pan + 0xfa0;
                call.pan = split ? sidePan : moved & 0xffff;
                --call.times;
                call.volume = uint8_t(call.volume - 0xf);
            }
        }
        if (roll(0x1f40).remainder > 0x1f2c)
        {
            if (call.times == 0)
            {
                call.times = int32_t(roll(3).remainder + 3);
                const uint16_t length = uint16_t((random() & 0xf) + 0x10);
                call.gap = length;
                call.ticks = length;
                call.effect = 0xa;
                const u32 pan = split ? sidePan : random() % 0xffff;
                call.pitch = 0x40;
                call.volume = 0x7f;
                call.pan = pan;
            }
        }
    }
    if (ends() && track() == 3)
    {
        if (roll(0x1388).remainder > 0x136f)
        {
            const u32 volume = roll(0x2d).remainder + 0x50;
            const u32 pan = pickedPan();
            const u32 effect = roll(3).remainder + 0x11;
            if (!playOnly(effect, 0x40, pan, volume))
                return {};
        }
    }
    // Track 3: the bells, once on the lap (0x552c34) before the last, once more
    // after.
    if (inside(0x528, 0x55c) && track() == 3)
    {
        if (roll(0x1388).remainder > 0x1356 && kBellState(app) == 0)
        {
            if (kBellLap(app) <= signedAt(app, car + 0x204))
            {
                const u32 pan = headingPan(false);
                if (!playOnly(7, 0x40, pan, 0x7f))
                    return {};
                kBellState.set(app, 1);
            }
        }
    }
    if (cell() < 0x359 || cell() > 0x388 || track() != 3)
        return int32_t(car);
    const Roll last = roll(0x1388);
    if (last.remainder <= 0x1356 || kBellState(app) != 1)
        return int32_t(last.quotient);
    const u32 pan = headingPan(false);
    u32 r;
    if (!play(8, 0x40, pan, 0x7f, r))
        return {};
    kBellState.set(app, 2);
    return int32_t(r);
}

}  // namespace

}  // namespace world

Result scenerySoundsOf(App* app, ThreadState& ts, u32 esp, world::Arithmetic a, u32 car, u32 heading)
{
    return world::sceneryIn(app, ts, esp, a, car, heading);
}

namespace world
{

}  // namespace world

using namespace world;

/* 0x415210: car eax's scenery sounds, edx its heading (a pan's).  eax as the
 * code leaves it. */
nfs::u32 scenerySounds(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, scenerySoundsOf(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), e.eax, e.edx));
}

}  // namespace port
}  // namespace nfs3hp
