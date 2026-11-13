#include "game_settings.h"
#include "port_sound_slice.h"

#include <cstdint>
#include <optional>

/* The pursuit's sound, rewritten (port.h says how): the music's intensity
 * following the chase's (Pathfinder's events and its control, each change
 * traced), the cops' warning beeps counted down, an effect of the cops' bank.
 *
 * The game's state: 0x552844 each player's chase level, 0x55283c the beeps
 * left, 0x571110 the beep's sound, 0x552838 the music's level (-1: none
 * asked for, else 0 or 100), 0x552834 the event the music plays outside a
 * chase, 0x55284c the chase music on. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Table<int32_t> kChaseLevels{0x552844}, kBeeps{0x55283c}, kBeepSounds{0x571110};
constexpr Var<int32_t> kMusicLevel{0x552838}, kChaseMusic{0x55284c}, kCopBank{0x571160};
constexpr Var<int32_t> kMusicOn{0x552910}, kMenuMusic{0x552914};
constexpr Var<uint8_t> kCalmEvent{0x552834}, kOwnTune{0x552590};
constexpr reg32 kTrace = 0x42d510, kSoundStatus = 0x4e8fd0, kBeepStart = 0x418930;

// The pan of the player's side in the split screen, else the middle.
reg32 sideOf(App* app, reg32 player)
{
    if (gameSettings(app).playMode != 1)
        return 0;
    return player != 0 ? 0xc000 : 0x3fff;
}

}  // namespace

/* 0x4188a0: player eax's beeps: when the last is done and there are more,
 * the next -- the high one (0x31) while below the chase's level.  eax as the
 * last call leaves it. */
void copBeepTick(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(6);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    const int32_t player = int32_t(e.eax);
    const reg32 side = sideOf(app, e.eax);
    g.edi = e.eax;
    g.esi = e.eax * 4;
    reg32 eax = e.eax;
    if (kBeeps(app, player) != 0)
    {
        if (!g.call(kSoundStatus, {reg32(kBeepSounds(app, player)), e.edx, side, e.ecx}, eax))
            return;
        if (eax != 0)
        {
            const reg32 effect = kChaseLevels(app, player) < kBeeps(app, player) ? 0x31 : 0x30;
            if (!g.call(kEffectBank, {reg32(kCopBank(app)), effect, side, 0x7f}, {0x40}, eax))
                return;
            kBeepSounds.set(app, player, int32_t(eax));
            kBeeps.set(app, player, int32_t(reg32(kBeeps(app, player)) - 1));
        }
    }
    e.back(cpu, eax);
}

/* 0x418930: player edx's beeps started: eax of them, the first high. */
void copBeepStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(4);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    const int32_t player = int32_t(e.edx);
    g.esi = e.edx * 4;
    kBeeps.set(app, player, int32_t(e.eax));
    reg32 eax;
    if (!g.call(kEffectBank, {reg32(kCopBank(app)), 0x31, sideOf(app, e.edx), 0x7f}, {0x40}, eax))
        return;
    kBeepSounds.set(app, player, int32_t(eax));
    e.back(cpu, eax);
}

/* 0x418c30: effect eax of the cops' bank, at full volume. */
void copEffect(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(5);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    g.esi = reg32(kCopBank(app));
    reg32 eax;
    if (!g.call(kEffectBank, {g.esi, e.eax, 0, 0x7f}, {0x40}, eax))
        return;
    e.back(cpu, eax);
}

/* 0x418c60: the music's event back after an interruption (the race's own
 * music on, playing, not muted): the chase's, or the calm one. */
void musicEventResume(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(2);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    reg32 eax = e.eax;
    if (kMenuMusic(app) != 0 && kMusicOn(app) != 0 && gameSettings(app).installIsRemote == 0 && gameSettings(app).raceMusicVolume != 0
        && kOwnTune(app) == 0)
    {
        if (!g.call(kPathEventInterrupt, {0, e.edx}, eax))
            return;
        // The chase's event while the music has a level of its own, else the calm one.
        const reg32 event = kMusicLevel(app) != -1 ? 1 : reg32(kCalmEvent(app));
        if (!g.call(kPathEvent, {event, 0}, eax))
            return;
    }
    e.back(cpu, eax);
}

/* 0x418990: player edx's chase at level eax: the beeps when it rises; the
 * race's music (player one's, its own music on) following -- the chase's
 * event when the chase starts, its level up at 4 and on, down at 2, the
 * calm event back when it ends.  eax as the code leaves it. */
void chaseLevel(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(5);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    const int32_t level = int32_t(e.eax);
    const int32_t player = int32_t(e.edx);
    reg32 eax = e.eax;
    const reg32 ebx = e.edx, ecx = e.eax;  // the player and the level, as the code keeps them
    const auto trace = [&](reg32 text) { return g.call(kTrace, {eax, e.edx, ebx, ecx}, {0x64, 1, text}, eax); };
    const auto path = [&](reg32 target, reg32 first, reg32 second) {
        return g.call(target, {first, second, ebx, ecx}, eax);
    };
    const auto event = [&](reg32 first, reg32 second) { return path(kPathEvent, first, second); };
    const auto control = [&](reg32 first) { return path(kPathControl, first, 0); };
    /* The beeps for a rise: false when the level is as it was (nothing more
     * to do); `stopped` when the game stopped. */
    bool stopped = false;
    const auto rise = [&]() {
        const int32_t was = kChaseLevels(app, player);
        eax = reg32(player) * 4;
        if (level == was)
            return false;
        if (level > was && !g.call(kBeepStart, {reg32(level), reg32(player), ebx, ecx}, eax))
            stopped = true;
        return true;
    };
    const auto keep = [&] {
        kChaseLevels.set(app, player, level);
        e.back(cpu, eax);
    };
    const bool music = kMusicOn(app) != 0 && gameSettings(app).installIsRemote == 0 && gameSettings(app).raceMusicVolume != 0 && kOwnTune(app) == 0
                       && player != 1;
    if (!music)
    {
        if (!rise())
            return e.back(cpu, eax);
        if (stopped)
            return;
        return keep();
    }
    if (kMenuMusic(app) != 0)
    {
        if (level != 0 && kMusicLevel(app) != -1 && kChaseMusic(app) == 0)
        {
            // The chase is on again without the music having heard: the calm event, its level none.
            g.esi = 0xffffffff;
            if (!trace(0x536bc0))
                return;
            eax = kCalmEvent(app);
            kMusicLevel.set(app, -1);
            if (!event(eax, e.edx))
                return;
            if (level == kChaseLevels(app, player))
                return e.back(cpu, eax);
        }
        if (level != 0 && kMusicLevel(app) == -1 && kChaseMusic(app) != 0)
        {
            // The chase's music started: its level 100 from the level 4, else 0.
            if (level >= 4)
            {
                g.esi = 0x64;
                if (!trace(0x536bd0))
                    return;
                kMusicLevel.set(app, 0x64);
            }
            else
            {
                g.edi = 0;
                if (!trace(0x536be4))
                    return;
                kMusicLevel.set(app, 0);
            }
            if (!event(1, 0) || !control(reg32(kMusicLevel(app))))
                return;
            if (level == kChaseLevels(app, player))
                return e.back(cpu, eax);
        }
    }
    if (!rise())
        return e.back(cpu, eax);
    if (stopped)
        return;
    if (kMenuMusic(app) == 0)
        return keep();
    if (level > 1 && kMusicLevel(app) == -1 && kChaseMusic(app) != 0)
    {
        g.esi = 0;
        if (!trace(0x536bfc))
            return;
        kMusicLevel.set(app, 0);
        if (!event(1, 0) || !control(0))
            return;
        return keep();
    }
    if (level == 2 && kMusicLevel(app) != 0 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536c10))
            return;
        kMusicLevel.set(app, 0);
        if (!control(0))
            return;
        return keep();
    }
    if (level == 4 && kMusicLevel(app) != 0x64 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536c20))
            return;
        kMusicLevel.set(app, 0x64);
        if (!control(0x64))
            return;
        return keep();
    }
    if (level == 0 && kMusicLevel(app) != -1 && kChaseMusic(app) != 0)
    {
        g.edi = 0xffffffff;
        if (!trace(0x536bc0))
            return;
        kMusicLevel.set(app, -1);
        if (!event(kCalmEvent(app), 0))
            return;
    }
    keep();
}

}  // namespace port
}  // namespace nfs3hp
