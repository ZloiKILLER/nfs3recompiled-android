#include "game_settings.h"
#include "port_snd_speech.h"

#include <cstdint>
#include <cstring>

/* The race's announcer and its music, rewritten (port.h says how): per car of
 * a player each tick -- the keys that ask for a gap, a position or a lap time
 * spoken; the lap's lines (a new lap, the best one, the last lap, the lap
 * number, the position at the end) queued on the race voice (a line of 9
 * words); the speech's tick; then the race music's intensity by the position
 * and the laps left (Pathfinder's control), its event by where on the track
 * the car is (the section's kind), traced.
 *
 * The game's state: each player's last lap (a byte), the lap's start and its
 * time, the best lap, the laps for the music (two), its intensity, its event
 * and the long wait's mark.  eax is what the code leaves in it, which the
 * rewrite follows (the tick has several ways out, and its callers see eax). */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Var<int32_t> kRaceTime{0x7d3684}, kRaceClock{0x7d36b0};
constexpr Var<int32_t> kMusicOn{0x552794}, kBestLap{0x552904};
constexpr Table<uint8_t> kLastLap{0x5710c8, 1}, kEventKind{0x5710a0, 1};
constexpr Table<int32_t> kLapStart{0x5710d0}, kLapTime{0x5710f0};
constexpr Table<reg32> kRaceOrder{0x5efac8};
constexpr Var<int32_t> kMusicLap{0x552860}, kMusicLapNow{0x55285c}, kMusicAhead{0x55284c};
constexpr Var<uint8_t> kMusicLevel{0x552850}, kMusicEvent{0x552834}, kWaitMark{0x552d7c};
constexpr Var<int32_t> kMusicBusy{0x552838};
constexpr Var<int32_t> kMusicEnabled{0x552910}, kMusicReady{0x552914};
constexpr Var<uint8_t> kMusicMuted{0x552590};
constexpr Var<int32_t> kLineStart{0x571238}, kLineEnd{0x571240};
constexpr Var<float> kMetricScale{0x536a18}, kImperialScale{0x536a1c};

// What is not rewritten yet, or what the checks put stand-ins in the place of.
constexpr reg32 kTrace = 0x42d510, kKeyPressed = 0x48f7b0, kSectionKind = 0x4211c0;
constexpr reg32 kPathControl = 0x4e705c, kPathInterrupt = 0x4e7208, kPathEvent = 0x4e70fc;
constexpr reg32 kPrepare = 0x40a330, kSettle = 0x4129b0, kPursuitSounds = 0x4188a0;
constexpr reg32 kTraceLaps = 0x5369e8, kTraceWait = 0x5369d8;

/* The tick of one car: the announcer for its player, then the music. */
struct Tick
{
    App* app;
    CPU& cpu;
    reg32 sp;       // esp where the code calls from
    reg32 car;
    uint8_t own;    // the car's player
    reg32 side;     // the player's side in the split screen, else 0

    reg32 pan() const { return gameSettings(app).playMode == 1 ? (side != 0 ? kSideRight : kSideLeft) : 0; }

    /* The race voice's line `effect` queued at full volume (kind 0), the
     * player's side. */
    Result say(reg32 effect) const
    {
        const reg32 line = sp;
        SpeechLine& l = record<SpeechLine>(app, line);
        l.volume = 0x7f;
        l.pitch = 0x40;
        l.pan = pan();
        l.bank = kSpeechBankHandles(app, kSpeechBankRaceVoice);
        l.effect = int32_t(effect);
        l.player = int32_t(side);
        l.kind = 0;
        return speechLineSaid(app, cpu, sp - 4, line);
    }
};

/* The unsigned revs of a car, scaled by `scale` and chopped: seconds of a lap
 * time. */
int32_t scaledRevs(App* app, reg32 car, float scale)
{
    const float revs = load<float>(app, car + 0x4f4);
    const float unsignedRevs = 0.0 < revs || revs != revs ? revs : -revs;
    return truncatedInteger(double(unsignedRevs) * double(scale));
}

}  // namespace

/* 0x412d60's: car `car`'s announcer and music tick.  eax as the code leaves
 * it. */
static Result raceAnnouncerTicked(App* app, CPU& cpu, reg32 esp, reg32 car)
{
    const reg32 frame = esp - 6 * 4;
    const HeldEbp ebp(cpu, frame);
    const reg32 sp = frame - 0x3c;
    const int32_t split = gameSettings(app).playMode;
    const uint8_t own = load<uint8_t>(app, car + 0x1f0);
    const Tick t{app, cpu, sp, car, own, split == 1 ? own : 0u};
    const reg32 place = load<reg32>(app, kRaceOrder(app, own) + 0x2bc);
    // The car's kind of driver, kept by its player.
    const reg32 driver = load<reg32>(app, car + 0x2c);
    kEventKind.set(app, int32_t(load<reg32>(app, car + 0x1f0)), driver != 0 ? load<uint8_t>(app, driver + 3) : uint8_t(0));
    const auto call = [&](reg32 target, Args a) { return callResult(app, cpu, sp, target, a, {}, cpu.esi, cpu.edi); };
    const auto keyPressed = [&](reg32 key) { return call(kKeyPressed, Args{key, 1, 0, 0}); };
    if (!call(kPrepare, {}) || !call(kSettle, {}))
        return {};
    // The keys: 0x13 the music on and off.
    Result key = keyPressed(0x13);
    if (!key)
        return {};
    if (*key != 0 && t.side == 0)
        kMusicOn.set(app, kMusicOn(app) == 0 ? 1 : 0);
    if (gameSettings(app).raceType == 3 && !call(kPursuitSounds, Args{t.side, 0, 0, 0}))
        return {};
    const uint8_t flags = load<uint8_t>(app, car + 0x200);
    if (split != 1 && !((flags & 1) != 0 && load<int32_t>(app, car + 0x2c4) == 3) && gameSettings(app).opponentCount != 0)
    {
        if ((flags & 0x20) == 0)
        {
            if (!(key = keyPressed(2)))
                return {};
            // The gap to the leader, to be said (the code has the car in ecx).
            if (*key != 0 && !gapLineQueued(app, cpu, sp - 4, car, 0, 1, car))
                return {};
            if (!(key = keyPressed(3)))
                return {};
            if (*key != 0 && !raceEventLineQueued(app, cpu, sp - 4, 0, place))
                return {};
        }
        if (!(key = keyPressed(4)))
            return {};
        if (*key != 0)
        {
            // The lap time: the revs' counter, unsigned, scaled for the voice's units.
            const int32_t language = gameSettings(app).hud[0].speedometerForm;
            const bool metric = language == 1 || language == 2 || (language == 0 && gameSettings(app).language == 0);
            const int32_t seconds = scaledRevs(app, car, metric ? kMetricScale(app) : kImperialScale(app));
            if (!lapTimeLineQueued(app, cpu, sp - 4, 0, seconds))
                return {};
        }
    }
    const int32_t lap = load<int32_t>(app, car + 0x204);
    const int32_t laps = gameSettings(app).laps;
    const reg32 lastLap = reg32(kLastLap(app, own));
    if (gameSettings(app).raceType != 3)
    {
        // A new lap: the lap's line, the best lap's.
        bool said = false;
        if (lastLap != reg32(lap) && lap > 0)
        {
            const int32_t time = int32_t(reg32(kRaceTime(app)) - reg32(kLapStart(app, own)));
            if (time < kBestLap(app) || kBestLap(app) == 0)
            {
                if (!t.say((nextRandom(app) & 1) + 3))
                    return {};
                const int32_t best = int32_t(reg32(kRaceTime(app)) - reg32(kLapStart(app, own)));
                kBestLap.set(app, best);
                kLapTime.set(app, own, best);
                said = true;
            }
        }
        if (!said && lastLap != reg32(lap) && lap > 1)
        {
            const int32_t time = int32_t(reg32(kRaceTime(app)) - reg32(kLapStart(app, own)));
            if (time < kLapTime(app, own))
            {
                // Faster than the last lap.
                if (!t.say(nextRandom(app) % 3))
                    return {};
                kLapTime.set(app, own, int32_t(reg32(kRaceTime(app)) - reg32(kLapStart(app, own))));
            }
        }
        // The race over: the position.
        if (lastLap != reg32(lap) && lap == laps && !(gameSettings(app).networkGame != 0 && split > 1)
            && !(gameSettings(app).opponentCount == 0 && split == 0))
        {
            const Result radioLine = copLineChosen(app, cpu, sp - 4, int32_t(place));
            if (!radioLine || !t.say(reg32(*radioLine) + (t.side << 4)))
                return {};
        }
    }
    if (gameSettings(app).raceType != 3)
    {
        // The last lap, or the lap's number.
        bool queued = false;
        if (laps - 1 == lap && lastLap != reg32(lap))
        {
            if (!t.say((nextRandom(app) & 1) + 5))
                return {};
            queued = true;
        }
        if (!queued && lastLap != reg32(lap) && lap != 0 && laps - 1 > lap)
        {
            if (!t.say(reg32(lap) + 6))
                return {};
        }
    }
    // The lap kept.
    if (lastLap != reg32(lap))
    {
        if (uint8_t(lastLap) == 0)
            kLapTime.set(app, own, int32_t(reg32(kRaceTime(app)) - reg32(kLapStart(app, own))));
        kLastLap.set(app, own, uint8_t(lap));
        kLapStart.set(app, own, load<int32_t>(app, car + 0x238));
        kMusicLap.set(app, lap);
        kMusicLapNow.set(app, lap);
    }
    Result ticked = playerSpeechTicked(app, cpu, sp - 4, int32_t(t.side));
    if (!ticked)
        return {};
    // The music: player one's (in the split screen, the one ahead's).
    bool ahead = false;
    reg32 eax = reg32(*ticked);
    if (split == 1)
    {
        const reg32 first = kRaceOrder(app, 0), second = kRaceOrder(app, 1);
        eax = load<reg32>(app, first + 0x2bc);
        ahead = load<int32_t>(app, first + 0x2bc) >= load<int32_t>(app, second + 0x2bc);
    }
    eax = (eax & 0xffffff00u) | (ahead ? 1u : 0u);
    if (kMusicEnabled(app) == 0 || gameSettings(app).installIsRemote != 0 || gameSettings(app).raceMusicVolume == 0 || kMusicMuted(app) != 0
        || kMusicReady(app) == 0)
        return int32_t(eax);
    if (split == 1 && uint8_t(eax) != uint8_t(t.side))
        return int32_t(eax);
    const reg32 rival = load<reg32>(app, car + 0x628);
    kMusicAhead.set(app, rival != 0 && load<int32_t>(app, rival + 0x2c) > 0 ? 1 : 0);
    // The intensity: by the laps left and the race's laps.
    const int32_t furthest = lap > kMusicLap(app) ? lap : kMusicLap(app);
    uint8_t level;
    if (furthest == laps - 1 || (furthest > 5 && laps == 8))
        level = 0x64;
    else if ((furthest == 0 && laps != 2) || (furthest == 1 && laps == 8))
        level = 0;
    else
        level = 0x32;
    const int32_t mode = gameSettings(app).opponentCount;
    if (mode != 0)
    {
        if (mode == 1)
        {
            if (reg32(furthest) <= 7)
            {
                if (place == 1)
                    switch (furthest)
                    {
                    case 0: level = laps == 2 ? 0x32 : 0; break;
                    case 1: level = laps == 2 ? 0x64 : 0x32; break;
                    case 2:
                    case 3: level = laps == 4 ? 0x64 : 0x32; break;
                    default: level = 0x64; break;
                    }
                else
                    switch (furthest)
                    {
                    case 0: level = 0; break;
                    case 1: level = laps == 2 ? 0x32 : 0; break;
                    case 2: level = laps == 4 ? 0x32 : 0; break;
                    case 3: level = laps == 4 ? 0x64 : 0; break;
                    case 7: level = 0x64; break;
                    default: level = 0x32; break;
                    }
            }
        }
        else if (int32_t(place) < 2 && level != 0x64)
            level = level + 0x64 > 0x64 ? 0x64 : uint8_t(level + 0x64);
        else if (int32_t(place) < 4 && int32_t(place) >= 2)
            level = level + 0x32 > 0x64 ? 0x64 : uint8_t(level + 0x32);
    }
    // Calm when the car is slow.
    const int32_t chopped = truncatedInteger(double(load<float>(app, car + 0x4f4)));
    const reg32 speed = (chopped <= 0 ? 0u - reg32(chopped) : reg32(chopped)) * 2;
    if (int32_t(speed) < 0x32)
        level = 0;
    if (level != kMusicLevel(app) && kMusicBusy(app) == -1)
    {
        if (!call(kPathControl, Args{level, 0, 0, 0}))
            return {};
        kMusicLevel.set(app, level);
    }
    // The long wait (over 600): the music's event resumed once.
    const int32_t waiting = load<int32_t>(app, car + 0x8a0);
    reg32 traced;
    if (!callWith(app, cpu, sp, kTrace, Args{}, {0x64, 2, kTraceWait, reg32(waiting)}, reg32(waiting), cpu.edi, traced))
        return {};
    if (load<int32_t>(app, car + 0x8a0) > 0x258 && kWaitMark(app) == 0)
    {
        kWaitMark.set(app, 1);
        if (!call(kPathInterrupt, Args{0, 0, 0, 0}))
            return {};
        const reg32 event = kMusicBusy(app) != -1 ? 1u : reg32(kMusicEvent(app));
        if (!call(kPathEvent, Args{event, 0, 0, 0}))
            return {};
    }
    else if (load<int32_t>(app, car + 0x8a0) == 0 && kWaitMark(app) != 0)
        kWaitMark.set(app, 0);
    // The section's event.
    const Result section = call(kSectionKind, Args{car, 0, 0, 0});
    if (!section)
        return {};
    eax = reg32(*section);
    const uint8_t event = uint8_t(eax), was = kMusicEvent(app);
    if (event == was)
        return int32_t(eax);
    if (kRaceClock(app) > 0x258)
    {
        const reg32 backwards = load<reg32>(app, car + 0x998);
        bool over = false;
        if (backwards == 0)
        {
            if (reg32(was) == kLineStart(app))
            {
                eax &= 0xff;
                over = eax == reg32(kLineEnd(app));
            }
        }
        if (!over && backwards != 0)
        {
            eax = reg32(kMusicEvent(app));
            if (eax == reg32(kLineEnd(app)))
            {
                eax = event;
                over = eax == reg32(kLineStart(app));
            }
        }
        if (over)
        {
            // Over the line: a lap on (or back, going backwards).
            const int32_t now = int32_t(reg32(kMusicLapNow(app)) + (backwards != 0 ? 0xffffffffu : 1u));
            kMusicLapNow.set(app, now);
            if (backwards == 0)
                kMusicLap.set(app, now);
            if (kMusicLap(app) < 0)
                kMusicLap.set(app, 0);
            if (laps == kMusicLap(app))
                kMusicLap.set(app, laps - 1);
            eax = reg32(kMusicLapNow(app));
            if (!callWith(app, cpu, sp, kTrace, Args{},
                          {0x64, 1, kTraceLaps, reg32(lap), reg32(kMusicLap(app)), reg32(kMusicLapNow(app))},
                          reg32(lap), cpu.edi, traced))
                return {};
        }
    }
    const int32_t busy = kMusicBusy(app);
    kMusicEvent.set(app, event);
    if (busy != -1)
        return int32_t(eax);
    return call(kPathEvent, Args{event, 0, 0, 0});
}

/* 0x412d60: car eax's announcer and music tick. */
void raceAnnouncer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, raceAnnouncerTicked(app, cpu, e.esp, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
