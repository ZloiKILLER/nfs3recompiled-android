#include "game_settings.h"
#include "native_ea_support.h"
#include "port_snd_sources.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"

#include <algorithm>
#include <cstdint>

/* The race's sound sources, rewritten (port.h says how): the players'
 * listeners and the four cars the cops' sounds follow, reset as a race starts;
 * a cop car's siren and engine played from its place; how close the cop cars
 * are; and the sound of the track's zones, as loud as a car is into its zone.
 *
 * A source is the car it follows, a distance (0x15f9: none) and a chase.  The
 * players have two (0x50 bytes each, the source at +0x18), the cops four (0x38
 * bytes each, the source first and the chase after it); the listener's place
 * is the game's.  The sounds play in the race's slots, through slotPlay and
 * slotStop (port_sound_effects.cpp) and the sirens (port_sound_siren.cpp). */
namespace nfs3hp
{
namespace port
{

namespace
{

using sources::quotient;
using sources::shiftedToZero;

#pragma pack(push, 1)

// A source of sound: the car it follows (0 none) and where it is.
struct Source
{
    reg32 car;
    int32_t last4, last8;  // both -1 when reset
    int32_t other0c;
    int32_t other10;
    int32_t distance;      // from the listener; 0x15f9 for none
    int32_t other18;
    int32_t other1c;
    int32_t chase;         // 0 until the car chases
};
static_assert(sizeof(Source) == 0x24, "a source");

// A player's: the source after three words and 12 bytes the game keeps.
struct PlayerSource
{
    int32_t first, second, third;
    uint8_t other0c[0xc];
    Source source;
    uint8_t other3c[0x14];
};
static_assert(sizeof(PlayerSource) == 0x50, "a player's source");

// A cop car's: the source, then its chase.
struct CopSource
{
    Source source;
    int32_t chaseKept;     // the chase again, once the car chases
    int32_t closeness;     // minus the distance it was set chasing at
    reg32 target;          // what it chases
    uint8_t other30[8];
};
static_assert(sizeof(CopSource) == 0x38, "a cop car's source");

// One of a track's zones, 6 words: where it starts, is full, stops being full
// and ends along the track, its effect and its side.
struct Zone
{
    int32_t start, fullStart, fullEnd, end;
    int32_t effect, side;
};

#pragma pack(pop)

constexpr Var<int32_t> kSourceCount{0x552608};  // the sources with a car, 4 at most
constexpr Var<uint8_t> kSourceLimit{0x552858};
constexpr Records<PlayerSource> kPlayers{0x570ad8};
constexpr Records<CopSource> kCops{0x570b78};
constexpr Table<int32_t> kCarEngineBanks{0x552744};
constexpr Var<int32_t> kListenerEngineBank{0x570dd0}, kListenerChase{0x570dcc};
constexpr Table<reg32> kCars{0x5efa48};
constexpr Var<int32_t> kCarCount{0x5efda4};
constexpr Table<uint8_t> kModelPitch{0x55276c};  // each model's engine pitch, the models from 0x32

// The listener's place, and what the sources call: the distance from it (port_sound_3d.cpp), the slots' play and
// stop (port_sound_effects.cpp), and the game's trace (compiled out).
constexpr reg32 kListener = 0x570a78;
constexpr reg32 kSourceDistance = 0x410cb0, kSlotPlay = 0x417080, kSlotStop = 0x416f60, kGameTrace = 0x42d510;
constexpr reg32 kTraceText = 0x536b98;

// A cop car's slots start here (its siren's and its engine's).
constexpr int32_t kCopSlots = 0x1b;

// The distance of no car, and of nothing heard.
constexpr int32_t kFar = 0x15f9;

/* 0x411620's: source `source` reset, for car `car` (0: none). */
void resetSource(App* app, Source& source, reg32 car)
{
    int32_t count = kSourceCount(app);
    source.last4 = -1;
    source.last8 = -1;
    source.other10 = 0;
    source.chase = 0;
    source.distance = kFar;
    source.other18 = 0;
    source.other1c = -1;
    source.car = car;
    if (car != 0)
        count = int32_t(uint32_t(count) + 1);
    kSourceCount.set(app, std::min(count, 4));
}

}  // namespace

/* 0x411db0's: cop car `cop`'s distance from the listener, squared (0x15f9 with
 * no car), measured in arithmetic `a` (the x87's precision there). */
Result copDistanceSquared(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, int32_t cop)
{
    const reg32 frame = esp - 2 * 4;  // ebp and edx pushed
    const reg32 at = frame - 0xc;     // and a point of three words, then a word, on the stack
    const reg32 car = kCops(app, cop).source.car;
    if (car == 0)
        return kFar;
    store<reg32>(app, at, load<reg32>(app, car + 0x98));
    store<reg32>(app, at + 4, load<reg32>(app, car + 0x9c));
    store<reg32>(app, at + 8, load<reg32>(app, car + 0xa0));
    cpu.esp = at;
    const Result distance = guest::direct<kSourceDistance, world::distanceAt>(app, cpu, a, kListener, at);
    if (!distance)
        return {};
    const int32_t whole = shiftedToZero(*distance, 16);
    return imul(whole, whole);
}

/* 0x411c50's: cop car `cop`'s siren at distance `distance`, pan `pan`, pitch
 * `pitch`: on, or its tick. */
Result copSirenPlayed(App* app, CPU& cpu, reg32 esp, int32_t siren, int32_t distance, reg32 pan, int32_t pitch)
{
    const reg32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    // As loud as the distance is short of 0x15f9, none below 0.
    const int32_t volume =
        std::max(quotient(imul(int32_t(uint32_t(kFar) - uint32_t(distance)), 127), kFar), 0);
    if (sources::sirenActive(app, siren) == 0)
        return sources::sirenSwitchedOn(app, cpu, frame - 4, siren, false);
    return sources::sirenTicked(app, cpu, frame - 4, siren, volume, pitch, pan);
}

/* 0x411ca0's: siren `siren` stopped. */
Result copSirenStopped(App* app, CPU& cpu, reg32 esp, int32_t siren)
{
    return sources::sirenSwitchedOff(app, cpu, esp - 4 - 4, siren);
}

/* 0x411620's: the source at `source` reset, for car `car` (0: none): the source. */
reg32 sourceResetTo(App* app, reg32 source, reg32 car)
{
    resetSource(app, record<Source>(app, source), car);
    return source;
}

/* 0x411cb0's: cop car `cop`'s engine at distance `distance`, pitch `pitch`, pan
 * `pan`, in slot 0x1b on: effect 3, or the cops' own (10, or 11 for three
 * models) with the model's pitch. */
Result copEnginePlayed(App* app, CPU& cpu, reg32 esp, int32_t cop, int32_t distance, int32_t pitch, reg32 pan)
{
    const reg32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    const reg32 at = frame - 4;       // and a local
    const reg32 car = kCops(app, cop).source.car;
    const int32_t model = load<int32_t>(app, load<reg32>(app, car + 0x220) + 0x14);
    int32_t bend;
    int32_t effect;
    if ((load<uint8_t>(app, car + 0x200) & 0x10) == 0)
    {
        bend = shiftedToZero(int32_t(uint32_t(pitch) << 6), 16);
        effect = 3;
    }
    else
    {
        bend = shiftedToZero(imul(pitch, kModelPitch(app, model - 0x32)), 16);
        effect = model == 0x35 || model == 0x3e || model == 0x40 ? 0xb : 0xa;
    }
    const int32_t volume = quotient(imul(int32_t(uint32_t(kFar) - uint32_t(distance)), 127), kFar);
    const int32_t slot = cop + kCopSlots;
    cpu.esp = at;
    return guest::direct<kSlotPlay, slotPlayed>(app, cpu, at - 8, slot, effect, volume, std::clamp(bend, 0, 0x7f),
                                                pan);
}

/* 0x411e20's: how close cop cars `from` to `from + count - 1` are, all told:
 * each's 0x15f9 less its distance (squared), at most; measured in `a`. */
Result copsClosenessTold(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, int32_t count, int32_t from)
{
    const reg32 frame = esp - 5 * 4;  // ebx, ecx, esi, edi and ebp pushed
    int32_t sum = 0;
    const int32_t end = int32_t(uint32_t(from) + uint32_t(count));
    for (int32_t cop = from; cop < end; ++cop)
    {
        const Result distance = copDistanceSquared(app, cpu, frame - 4, a, cop);
        if (!distance)
            return {};
        const int32_t close = kFar - std::min(*distance, kFar);
        sum = int32_t(uint32_t(sum) + uint32_t(close));
    }
    return sum;
}

/* 0x415050's: car `car`'s sound of the track's zones `zones` in slot `slot`: as
 * loud as the car is into its zone, from its side (turned in the mirror)
 * offset by `offset`, or the player's side in the split screen.  1 when one
 * plays, 0 when none does or the view is not the player's. */
Result zoneSoundPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 car, reg32 zones, reg32 offset)
{
    const reg32 frame = esp - 3 * 4;  // esi, edi and ebp pushed
    const reg32 at = frame - 0xc;     // and three locals
    if (zones == 0)
        return 0;
    const int32_t view = gameSettings(app).viewedPlayer;
    if (view != gameSettings(app).localPlayer)
    {
        // The game's trace, compiled out (its words, 0x64, 2 and kTraceText, the caller's to push): nothing.
        cpu.esp = at;
        guest::direct<kGameTrace, ea::traceOff>(app, cpu);
        if (cpu.terminate)
            return {};
        return 0;
    }
    // The first zone the car is in, along the track.
    reg32 zone = zones;
    for (;; zone += sizeof(Zone))
    {
        const Zone& z = record<Zone>(app, zone);
        if (z.start <= -1)
            return 0;
        const int32_t along = load<int32_t>(app, car + 0x1c);
        if (along < z.start)
            continue;
        if (along > z.end)
            continue;
        break;
    }
    const Zone& z = record<Zone>(app, zone);
    const int32_t along = load<int32_t>(app, car + 0x1c);
    // As loud as the car is into its zone, up to full and down from it.
    int32_t volume;
    if (along < z.fullStart)
    {
        const int32_t length = int32_t(uint32_t(z.fullStart) - uint32_t(z.start));
        volume = quotient(imul(int32_t(uint32_t(along) - uint32_t(z.start)), 127), length);
    }
    else if (along > z.fullEnd)
    {
        const int32_t length = int32_t(uint32_t(z.end) - uint32_t(z.fullEnd));
        volume = quotient(imul(int32_t(uint32_t(z.end) - uint32_t(along)), 127), length);
    }
    else
        volume = 0x7f;
    int32_t side = z.side;
    if (gameSettings(app).mirrored != 0)
        side = negated(side);
    reg32 pan;
    if (gameSettings(app).playMode == 1)
        pan = load<int32_t>(app, car + 0x1f0) != 0 ? 0xc000 : 0x3fff;
    else
    {
        const reg32 base = reg32(volume) << 8;
        const bool loud = volume >= 0x3f;
        reg32 sides = base - 0x3f00;
        if (side < 0)
        {
            if (along < z.fullStart)
            {
                if (!loud)
                    sides = 0;
                else
                    sides = 0x10000 - sides;
            }
            else if (along > z.fullEnd)
                sides = loud ? base + 0x4100 : 0x8000;
            else
                sides = 0xc000;
            pan = (sides + offset) & 0xffff;
        }
        else if (side > 0)
        {
            if (along < z.fullStart)
            {
                if (!loud)
                    sides = 0;
            }
            else
            {
                if (along > z.fullEnd)
                {
                    if (!loud)
                        sides = 0x8000;
                    else
                        sides = 0x8000 - sides;
                }
                else
                    sides = 0x3fff;
            }
            pan = (sides + offset) & 0xffff;
        }
        else
            pan = 0;
    }
    cpu.esp = at;
    if (!guest::direct<kSlotPlay, slotPlayed>(app, cpu, at - 8, slot, z.effect, volume, 0x40, pan))
        return {};
    return 1;
}

/* The game's entries -- each its thunk, the registers and the stack in, the
 * result in eax out. */

/* 0x411620: source eax reset, for car edx (0: none). */
x86::reg32 sourceReset(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, sourceResetTo(app, e.eax, e.edx));
}

/* 0x411680: the sources reset as a race starts: the players' (each with its
 * car -- the first's, or each player's own in the split screen) and the cop
 * cars'.  eax 0. */
Result sourcesResetOf(App* app)
{
    kSourceLimit.set(app, 4);
    const int32_t engineBank = kCarEngineBanks(app, std::int32_t(gameSettings(app).track));
    kSourceCount.set(app, 0);
    kListenerEngineBank.set(app, engineBank);
    for (int32_t player = 0; player < 2; ++player)
    {
        PlayerSource& p = kPlayers(app, player);
        p.first = -1;
        p.second = 0;
        p.third = 0;
        resetSource(app, p.source, 0);
        if (std::int32_t(gameSettings(app).playMode) <= 1)
        {
            if (player < kCarCount(app))
                p.source.car = kCars(app, player);
        }
        else if (player == 0)
        {
            const int32_t car = gameSettings(app).localPlayer;
            if (car < kCarCount(app))
                p.source.car = kCars(app, car);
        }
    }
    for (int32_t cop = 0; cop < 4; ++cop)
        resetSource(app, kCops(app, cop).source, 0);
    kListenerChase.set(app, 0);
    return 0;
}

x86::reg32 sourcesReset(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, *sourcesResetOf(app));
}

/* 0x411750: the cop car of car eax, not chasing yet, set chasing -- ebx its
 * chase, edx its target, ecx its distance (kept minus).  eax as the code
 * leaves it: the end of the cop cars' table. */
x86::reg32 copChaseSet(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, copChaseSetOf(app, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx)));
}

/* 0x411750's core: copChaseSet for `car`, its `target`, `chase` and `distance`; eax as the code leaves it. */
reg32 copChaseSetOf(App* app, reg32 car, reg32 target, int32_t chase, int32_t distance)
{
    for (int32_t cop = 0; cop < 4; ++cop)
    {
        CopSource& c = kCops(app, cop);
        if (c.source.car != car || c.source.chase != 0)
            continue;
        c.chaseKept = chase;
        c.target = target;
        c.source.chase = chase;
        c.closeness = negated(distance);
    }
    return kCops.address(4);
}

/* 0x411c50: cop car eax's siren at distance ecx, pan edx, pitch ebx: on, or
 * its tick. */
x86::reg32 copSiren(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copSirenPlayed(app, cpu, e.esp, int32_t(e.eax), int32_t(e.ecx), e.edx, int32_t(e.ebx)));
}

/* 0x411ca0: siren eax stopped. */
x86::reg32 copSirenStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copSirenStopped(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x411cb0: cop car eax's engine at distance ecx, pitch ebx, pan edx, in
 * slot 0x1b on: effect 3, or the cops' own (10, or 11 for three models) with
 * the model's pitch. */
x86::reg32 copEngine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copEnginePlayed(app, cpu, e.esp, int32_t(e.eax), int32_t(e.ecx), int32_t(e.ebx), e.edx));
}

/* 0x411da0's: cop car `cop`'s engine stopped. */
Result copEngineStopped(App* app, CPU& cpu, reg32 esp, int32_t cop)
{
    const reg32 frame = esp - 4;
    cpu.esp = frame;
    return guest::direct<kSlotStop, slotStopped>(app, cpu, frame - 4, cop + kCopSlots);
}

/* 0x411da0: cop car eax's engine stopped. */
x86::reg32 copEngineStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copEngineStopped(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x411db0: cop car eax's distance from the listener, squared (0x15f9 with
 * no car). */
x86::reg32 copDistance(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copDistanceSquared(app, cpu, e.esp, world::arithmeticOf(cpu.fenv.rounding), int32_t(e.eax)));
}

/* 0x411e20: how close eax cop cars from edx on are, all told: each's 0x15f9
 * less its distance (squared), at most. */
x86::reg32 copsCloseness(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copsClosenessTold(app, cpu, e.esp, world::arithmeticOf(cpu.fenv.rounding), int32_t(e.eax),
                                    int32_t(e.edx)));
    return args.eax;
}

/* 0x415050: car edx's sound of the track's zones (ecx: their list, 6 words
 * each -- where it starts, is full, stops being full, ends; its effect; its
 * side -- to one below 0) in slot eax: as loud as the car is into its zone,
 * from its side (turned in the mirror) offset by ebx, or the player's side
 * in the split screen.  1 when one plays; 0, or with the view not the
 * player's (0x6fd4f8). */
x86::reg32 zoneSound(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, zoneSoundPlayed(app, cpu, e.esp, int32_t(e.eax), e.edx, e.ecx, e.ebx));
}

}  // namespace port
}  // namespace nfs3hp
