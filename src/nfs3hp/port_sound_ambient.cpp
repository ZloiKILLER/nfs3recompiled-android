#include "port_snd_3d.h"
#include "native_aiplan.h"
#include "native_track.h"
#include "port_snd_voices.h"
#include "port_sound_effects.h"

#include <cstdint>

/* The track's own sounds, rewritten (port.h says how): of the sound objects
 * the track's sections hold near the listener (0x552600's lists, 0x570dd0's
 * sound table, 18 bytes an object), the two nearest -- kept in the order they
 * had (0x570dd4, 0x570dd8) -- each played in its slot (0x14 on, the second
 * player's 0x22) at the volume its distance and range give, from its place;
 * some only within a stretch of the road, some every so many laps, some fading
 * in and out over their stretch.  The arithmetic is the x87's as the game's
 * code has it (port_snd_3d.h).
 *
 * Where the game's code keeps a local that a call of the game's fills (the
 * animation's place and the two numbers beside it), the local is where the
 * code has it on the game's stack, and read from there. */
namespace nfs3hp
{
namespace port
{
namespace world
{

namespace
{

#pragma pack(push, 1)
// A sound object of the sound table (0x570dd0), 18 bytes.
struct SoundObject
{
    int16_t range;        // how far it is heard, in whole units
    int16_t clock;        // added to the animation clock for its place a tick on
    int16_t level;        // its sound's level; below 0 the object is not in use
    int16_t start, end;   // the stretch it is heard in (the lap, say)
    int16_t tailFrom, tailTo;  // and the one from the stretch's end
    int16_t startEffect;  // its effect in the first stretch; -2 per lap, -3 every so many laps
    int16_t tailEffect;   // and in the tail
};
static_assert(sizeof(SoundObject) == 18, "a sound object");

// One of the race's sound slots: the sound playing (-1 none) and its effect.
struct Slot
{
    int32_t sound;
    int32_t effect;
};

// One of the two nearest objects found: it is 0x20 bytes on the game's stack.
struct Candidate
{
    u32 object;
    int32_t place[3];
    int32_t distance;
    int32_t lap, laps;           // the animation place's two numbers: the lap, and how many
    int32_t index;               // in the sound table
};
static_assert(sizeof(Candidate) == 0x20, "a candidate");
#pragma pack(pop)

constexpr u32 kFar = 0x27100000;
constexpr u32 kSections = 0x552600;
constexpr Var<u32> kSoundTable{0x570dd0};
constexpr Var<int32_t> kKept0{0x570dd4}, kKept1{0x570dd8};
constexpr Var<int32_t> kClock{0x7d3684};
constexpr Table<Slot> kSlots{0x570de8};
constexpr Var<double> kPlaceScale{0x536930}, kUnit{0x536938};
// What it calls: the game's functions with no typed one yet (by address), the sound rewrites (as C++).
constexpr u32 kAmbientWanted = 0x41bc20, kAnimationPlace = 0x40f6d0;
constexpr u32 kSlotStop = 0x416f60, kSlotEffect = 0x416fc0, kSoundStatus = 0x4e8fd0;

/* The code's signed shift towards 0 by 16. */
int32_t shiftedTowardZero(int32_t n) { return n < 0 ? int32_t(uint32_t(n) + 0xffff) >> 16 : n >> 16; }

int32_t absolute(int32_t v) { return v > 0 ? v : negated(v); }

/* idiv's remainder of n by `by` is 0 (the code faults on 0). */
bool remainderIsZero(int32_t n, int32_t by) { return by == 0 ? false : by == -1 ? true : n % by == 0; }

/* A float of the game's, scaled to whole units: fld; fmul; fistp. */
template <class Real>
int32_t wholeUnits(App* app, u32 at)
{
    return nearestInteger(double(times<Real>(double(Real(floatAt(app, at))), kPlaceScale(app))));
}

/* The game's whole units as a float: fild; fmul; fstp. */
template <class Real>
float unitsAsFloat(App* app, int32_t whole)
{
    return float(times<Real>(double(whole), kUnit(app)));
}

template <class Real>
Result trackSoundsIn(App* app, ThreadState& ts, u32 entryEsp, int32_t player)
{
    // Six pushes, a frame of 0xd8 with ebp 0x82 below the pushes.
    const u32 pushed = entryEsp - 6 * 4;
    const u32 b = pushed - 0x82, below = pushed - 0xd8;
    Calls game{app, ts, below};
    const auto slotStop = [&](int32_t slot) {
        game.atCall();
        return bool(guest::direct<kSlotStop, slotStopped>(app, ts, below - 4, slot));
    };
    // Slot `slot`'s effect at `volume` and `pan`, the pitch 0x40 times `ecx` (16.16).
    const auto slotEffect = [&](u32 slot, int32_t effect, u32 ecx, u32 volume, u32 pan) {
        game.atCall();
        return bool(guest::direct<kSlotEffect, slotEffectPlayed>(app, ts, below - 4 - 3 * 4, int32_t(slot), effect,
                                                                 ecx, 0x40u, volume, pan, u32(player << 5)));
    };
    // The animation place's outputs, where the code has them.
    const u32 placeAt = b + 0x2e, lapAt = b + 0x3e, lapsAt = b + 0x3a;
    const u32 entries = b - 0x56;
    const auto candidate = [&](int32_t i) -> Candidate& { return record<Candidate>(app, entries + 0x20 * u32(i)); };
    const auto object = [&](int32_t index) -> SoundObject& {
        return record<SoundObject>(app, kSoundTable(app) + u32(index) * 18);
    };
    const u32 table = kSoundTable(app);
    if (table == 0)
        return player;
    const u32 wanted = guest::direct<kAmbientWanted, track::ambientPlays>(app, ts) ? 1u : 0u;
    if (wanted == 0)
        return 0;
    const int32_t count = 2;
    for (int32_t i = 0; i < count; ++i)
    {
        candidate(i).index = -1;
        candidate(i).distance = int32_t(kFar);
    }
    // The sections' lists of objects, each object a record (its length a
    // word first, its kind at +2: 3 a sound's).
    int32_t soundIndex = 0;
    for (u32 list = kSections;; list += 4)
    {
        const u32 section = guest::at(app, list);
        if (section == 0)
            break;
        const u32 objects = guest::at(app, section);
        if (objects == 0)
            continue;
        u32 cursor = objects + 8;
        for (uint32_t left = app->getMemory<uint16_t>(objects + 6); left-- != 0;)
        {
            if (app->getMemory<uint8_t>(cursor + 2) == 3)
            {
                const SoundObject& rec = object(soundIndex);
                if (rec.level > -1)
                {
                    // Its place (0x40f6d0's, at this tick), how near.
                    u32 time = u32(kClock(app));
                    time = (time & ~0xffu) | (uint8_t(time) & 0xfe);
                    game.atCall();
                    if (!guest::direct<kAnimationPlace, aiplan::placeAt>(app, ts, cursor, 1u,
                                                                         int32_t(time + u32(int32_t(rec.clock))), placeAt,
                                                                         lapAt, lapsAt))
                        return {};
                    const int32_t x = int32_t(guest::at(app, placeAt)), y = int32_t(guest::at(app, placeAt + 4)),
                                  z = int32_t(guest::at(app, placeAt + 8));
                    const int32_t dx = x - wholeUnits<Real>(app, kListener);
                    const int32_t dz = z - wholeUnits<Real>(app, kListener + 8);
                    const int32_t reach = int32_t(uint32_t(int32_t(rec.range)) << 16);
                    int32_t ax = absolute(dx), az = absolute(dz);
                    if (ax > az)
                        az >>= 2;
                    else
                        ax >>= 2;
                    int32_t distance = int32_t(uint32_t(az) + uint32_t(ax));
                    bool near = distance < reach;
                    if (near)
                    {
                        const int32_t dy = y - wholeUnits<Real>(app, kListener + 4);
                        int32_t ay = absolute(dy);
                        if (ay > distance)
                            distance >>= 2;
                        else
                            ay >>= 2;
                        distance = int32_t(uint32_t(ay) + uint32_t(distance));
                        near = distance < reach;
                    }
                    if (near)
                    {
                        // Among the nearest two, by distance.
                        for (int32_t slot = 0; slot < count; ++slot)
                        {
                            if (distance >= candidate(slot).distance)
                                continue;
                            for (int32_t at = count - 1; at > slot; --at)
                                candidate(at) = candidate(at - 1);
                            Candidate& c = candidate(slot);
                            c.distance = distance;
                            c.object = cursor;
                            c.place[0] = x;
                            c.place[1] = y;
                            c.place[2] = z;
                            c.lap = int32_t(guest::at(app, lapAt));
                            c.laps = int32_t(guest::at(app, lapsAt));
                            c.index = soundIndex;
                            break;
                        }
                    }
                }
                ++soundIndex;
            }
            cursor += app->getMemory<uint16_t>(cursor);
        }
    }
    // Kept in the slots they had.
    if (candidate(1).index == kKept0(app) || candidate(0).index == kKept1(app))
    {
        // The code swaps them through a third place above the second.
        const Candidate first = candidate(0), second = candidate(1);
        candidate(0) = second;
        candidate(1) = first;
        record<Candidate>(app, b - 0x16) = first;
    }
    kKept0.set(app, candidate(0).index);
    kKept1.set(app, candidate(1).index);
    for (int32_t i = 0; i < count; ++i)
    {
        const Candidate& entry = candidate(i);
        const int32_t slot = i + (player != 0 ? 0x22 : 0x14);
        if (int32_t(entry.distance) >= int32_t(kFar))
        {
            if (!slotStop(slot))
                return {};
            continue;
        }
        // The object's sound: its range (a word), its effect, its stretch.
        const SoundObject& rec = object(entry.index);
        const u32 recAt = kSoundTable(app) + u32(entry.index) * 18;
        const int32_t reachedBy = shiftedTowardZero(entry.distance);
        const int32_t range2 = imul(rec.range, rec.range);
        const int32_t outside = range2 - imul(reachedBy, reachedBy);
        int32_t volume = range2 == 0 ? 0 : idiv(imul(outside, 127), range2);
        const Vec3 place{unitsAsFloat<Real>(app, entry.place[0]), unitsAsFloat<Real>(app, entry.place[1]),
                         unitsAsFloat<Real>(app, entry.place[2])};
        const Result angle = angleToPlace(app, ts, below - 4, arithmeticFor<Real>(), kListener, place);
        if (!angle)
            return {};
        // Its direction: from its place a tick on, to its own.
        u32 time = u32(kClock(app)) - 0x20 + u32(int32_t(rec.clock));
        game.atCall();
        if (!guest::direct<kAnimationPlace, aiplan::placeAt>(app, ts, entry.object, 1u, int32_t(time), placeAt, lapAt,
                                                             lapsAt))
            return {};
        // Its move since a tick before, the direction the track's ahead is taken along.
        const u32 moveAt = b + 0x22;
        for (u32 k = 0; k < 3; ++k)
        {
            const int32_t moved = int32_t((uint32_t(entry.place[k]) - guest::at(app, placeAt + 4 * k)) * 2u);
            store<float>(app, moveAt + 4 * k, unitsAsFloat<Real>(app, moved));
        }
        const Result ahead = aheadAlongTrack(app, ts, below - 4, arithmeticFor<Real>(), place, moveAt);
        if (!ahead)
            return {};
        int32_t level = rec.level;
        const int32_t pan = *angle;
        // Plays at the volume, ecx its ahead unless silent.
        const auto play = [&](u32 slotNumber, int32_t effect) {
            return slotEffect(slotNumber, effect, volume != 0 ? u32(*ahead) : 0, u32(volume), u32(pan));
        };
        const int32_t lap = entry.lap, laps = entry.laps;
        const int32_t playing = kSlots(app, slot).effect;
        bool done = false;
        if (playing == rec.startEffect || playing == rec.tailEffect)
        {
            // The slot already playing one of its other effects: on with it.
            game.atCall();
            const Result status =
                guest::direct<kSoundStatus, soundStatus>(app, ts, below - 4, u32(kSlots(app, slot).sound));
            if (!status)
                return {};
            if (*status == 0)
            {
                if (!slotEffect(u32(i + 0x14), kSlots(app, slot).effect, u32(*ahead), u32(volume), u32(pan)))
                    return {};
                continue;
            }
        }
        if (uint16_t(rec.startEffect) == 0xfffe)
        {
            // Once at these laps' stretches.
            if (rec.end == lap || rec.tailFrom == lap || rec.tailTo == lap)
            {
                if (!play(0x13, int32_t(0u - uint32_t(int32_t(rec.tailEffect)))))
                    return {};
                done = true;
            }
        }
        if (!done && uint16_t(rec.startEffect) == 0xfffd)
        {
            // Every so many laps.
            if (remainderIsZero(lap, rec.end))
            {
                if (!play(0x13, int32_t(0u - uint32_t(int32_t(rec.tailEffect)))))
                    return {};
                done = true;
            }
        }
        if (!done)
        {
            // Within its stretch, its own effect; fading in and out at its ends.
            bool set = false;
            if (rec.startEffect > -1 && rec.start <= lap && rec.end > lap)
            {
                level = rec.startEffect;
                set = true;
            }
            if (!set && rec.tailEffect > -1 && laps - rec.tailFrom <= lap && laps - rec.tailTo > lap)
            {
                level = rec.tailEffect;
                set = true;
            }
            if (!set && rec.start > -1)
            {
                if (rec.start >= lap)
                    level = -1;
                else
                {
                    int32_t numerator = 0, divisor = 0;
                    bool divide = false;
                    if (rec.end > lap)
                    {
                        numerator = imul(lap - rec.start, volume);
                        divisor = rec.end - rec.start;
                        divide = true;
                    }
                    else
                    {
                        const int32_t fromEnd = laps - rec.tailTo;
                        if (fromEnd <= lap)
                            level = -1;
                        else if (laps - rec.tailFrom < lap)
                        {
                            numerator = imul(volume, fromEnd - lap);
                            divisor = rec.tailFrom - rec.tailTo;
                            divide = true;
                        }
                    }
                    if (divide)
                        volume = divisor == 0 ? 0 : idiv(numerator, divisor);
                }
            }
        }
        // (Even after a lap's call the code goes on here, with the level as it was.)
        if (level < 0)
        {
            if (!slotStop(slot))
                return {};
            continue;
        }
        if (!play(u32(i + 0x14), level))
            return {};
    }
    return count;
}

}  // namespace

Result trackSoundsFor(App* app, ThreadState& ts, u32 esp, Arithmetic a, int32_t player)
{
    return withArithmetic(a, [&](auto real) { return trackSoundsIn<decltype(real)>(app, ts, esp, player); });
}

}  // namespace world

using namespace world;

/* 0x410e30: player eax's track sounds.  eax as the code leaves it: the count
 * of objects it played (2), the player's own where it has none to play, 0 if
 * the track has none wanted. */
nfs::u32 trackSounds(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, trackSoundsFor(app, ts, e.esp, arithmeticOf(ts.fenv.rounding), int32_t(e.eax)));
}

}  // namespace port
}  // namespace nfs3hp
