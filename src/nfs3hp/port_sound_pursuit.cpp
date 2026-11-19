#include "game_settings.h"
#include "native_ea_support.h"
#include "port_snd_stream_calls.h"
#include "port_snd_voices.h"
#include "port_sound_effects.h"
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
constexpr u32 kTrace = 0x42d510, kSoundStatus = 0x4e8fd0, kBeepStart = 0x418930;

// The pan of the player's side in the split screen, else the middle.
u32 sideOf(App* app, u32 player)
{
    if (gameSettings(app).playMode != 1)
        return 0;
    return player != 0 ? 0xc000 : 0x3fff;
}

}  // namespace

/* 0x4188a0's: player `player`'s beeps: when the last is done and there are
 * more, the next -- the high one (0x31) while below the chase's level.  eax as
 * the last call leaves it (else the player). */
Result copBeepsTicked(App* app, ThreadState& ts, u32 esp, u32 player)
{
    const u32 frame = esp - 6 * 4;
    Game g(app, ts, frame);
    const int32_t p = int32_t(player);
    const u32 side = sideOf(app, player);
    u32 eax = player;
    if (kBeeps(app, p) != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kSoundStatus, soundStatus>(app, ts, g.esp - 4, u32(kBeepSounds(app, p))), eax))
            return {};
        if (eax != 0)
        {
            const u32 effect = kChaseLevels(app, p) < kBeeps(app, p) ? 0x31 : 0x30;
            g.atCall();
            if (!tookEax(guest::direct<kEffectBank, bankEffect>(app, ts, g.esp - 8, kCopBank(app), int32_t(effect),
                                                                 0x7f, side, uint8_t(0x40)),
                         eax))
                return {};
            kBeepSounds.set(app, p, int32_t(eax));
            kBeeps.set(app, p, int32_t(u32(kBeeps(app, p)) - 1));
        }
    }
    return int32_t(eax);
}

/* 0x4188a0: player eax's beeps. */
nfs::u32 copBeepTick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, copBeepsTicked(app, ts, e.esp, e.eax));
}

/* 0x418930: player `player`'s beeps started: `count` of them, the first
 * high.  The beep's handle; esp the frame base. */
Result beepsStarted(App* app, ThreadState& ts, u32 esp, int32_t count, int32_t player)
{
    const u32 frame = esp - 4 * 4;
    Game g(app, ts, frame);
    kBeeps.set(app, player, count);
    g.atCall();
    const Result sound = guest::direct<kEffectBank, bankEffect>(app, ts, g.esp - 8, kCopBank(app), 0x31, 0x7f,
                                                                 sideOf(app, u32(player)), uint8_t(0x40));
    if (!sound)
        return std::nullopt;
    kBeepSounds.set(app, player, *sound);
    return sound;
}

nfs::u32 copBeepStart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, beepsStarted(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

/* 0x418c30: effect eax of the cops' bank, at full volume. */
nfs::u32 copEffect(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, copEffectPlayed(app, ts, e.esp, int32_t(e.eax)));
}

/* 0x418c30's core: copEffect for `effect`; esp the frame base. */
Result copEffectPlayed(App* app, ThreadState& ts, u32 esp, int32_t effect)
{
    const u32 frame = esp - 4 * 5;
    const HeldEbp ebp(ts, frame);
    Game g(app, ts, frame);
    const int32_t bank = kCopBank(app);
    g.atCall();
    return guest::direct<kEffectBank, bankEffect>(app, ts, g.esp - 8, bank, effect, 0x7f, 0u, uint8_t(0x40));
}

/* 0x418c60: the music's event back after an interruption (the race's own
 * music on, playing, not muted): the chase's, or the calm one. */
Result musicEventResumed(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx)
{
    const u32 frame = esp - 2 * 4;
    Game g(app, ts, frame);
    if (kMenuMusic(app) != 0 && kMusicOn(app) != 0 && gameSettings(app).installIsRemote == 0 && gameSettings(app).raceMusicVolume != 0
        && kOwnTune(app) == 0)
    {
        // (`edx` the interruption's end is handed: it reads only the event.)
        (void)edx;
        g.atCall();
        if (!tookEax(guest::direct<kPathEventInterrupt, stream::pathEventInterrupt>(app, ts, g.esp - 4, 0), eax))
            return std::nullopt;
        // The chase's event while the music has a level of its own, else the calm one.
        const u32 event = kMusicLevel(app) != -1 ? 1 : u32(kCalmEvent(app));
        g.atCall();
        if (!tookEax(guest::direct<kPathEvent, stream::pathEvent>(app, ts, g.esp - 4, int32_t(event), 0), eax))
            return std::nullopt;
    }
    return int32_t(eax);
}

nfs::u32 musicEventResume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, musicEventResumed(app, ts, e.esp, e.eax, e.edx));
}

/* 0x418990: player edx's chase at level eax: the beeps when it rises; the
 * race's music (player one's, its own music on) following -- the chase's
 * event when the chase starts, its level up at 4 and on, down at 2, the
 * calm event back when it ends.  eax as the code leaves it. */
nfs::u32 chaseLevel(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 frame = e.frame(5);
    const HeldEbp ebp(ts, frame);
    Game g(app, ts, frame);
    const int32_t level = int32_t(e.eax);
    const int32_t player = int32_t(e.edx);
    u32 eax = e.eax;
    // The game's trace, compiled out (its words, 0x64, 1 and the text, the caller's to push): nothing.
    const auto trace = [&](u32) {
        g.atCall();
        guest::direct<kTrace, ea::traceOff>(app, ts);
        return !ts.terminate;
    };
    // Pathfinder's event and control, as C++ (the player and the level the code keeps in ebx and ecx are not
    // read by them).
    const auto event = [&](u32 first, u32 second) {
        g.atCall();
        return tookEax(guest::direct<kPathEvent, stream::pathEvent>(app, ts, g.esp - 4, int32_t(first),
                                                                    int32_t(second)),
                       eax);
    };
    const auto control = [&](u32 first) {
        g.atCall();
        return tookEax(guest::direct<kPathControl, stream::pathControl>(app, ts, g.esp - 4, int32_t(first)), eax);
    };
    /* The beeps for a rise: false when the level is as it was (nothing more
     * to do); `stopped` when the game stopped. */
    bool stopped = false;
    const auto rise = [&]() {
        const int32_t was = kChaseLevels(app, player);
        eax = u32(player) * 4;
        if (level == was)
            return false;
        if (level > was)
        {
            g.atCall();
            stopped = !tookEax(guest::direct<kBeepStart, beepsStarted>(app, ts, g.esp - 4, level, player), eax);
        }
        return true;
    };
    const auto keep = [&] {
        kChaseLevels.set(app, player, level);
        return e.back(ts, args, eax);
    };
    const bool music = kMusicOn(app) != 0 && gameSettings(app).installIsRemote == 0 && gameSettings(app).raceMusicVolume != 0 && kOwnTune(app) == 0
                       && player != 1;
    if (!music)
    {
        if (!rise())
            return e.back(ts, args, eax);
        if (stopped)
            return args.eax;
        return keep();
    }
    if (kMenuMusic(app) != 0)
    {
        if (level != 0 && kMusicLevel(app) != -1 && kChaseMusic(app) == 0)
        {
            // The chase is on again without the music having heard: the calm event, its level none.
            if (!trace(0x536bc0))
                return args.eax;
            eax = kCalmEvent(app);
            kMusicLevel.set(app, -1);
            if (!event(eax, e.edx))
                return args.eax;
            if (level == kChaseLevels(app, player))
                return e.back(ts, args, eax);
        }
        if (level != 0 && kMusicLevel(app) == -1 && kChaseMusic(app) != 0)
        {
            // The chase's music started: its level 100 from the level 4, else 0.
            if (level >= 4)
            {
                if (!trace(0x536bd0))
                    return args.eax;
                kMusicLevel.set(app, 0x64);
            }
            else
            {
                if (!trace(0x536be4))
                    return args.eax;
                kMusicLevel.set(app, 0);
            }
            if (!event(1, 0) || !control(u32(kMusicLevel(app))))
                return args.eax;
            if (level == kChaseLevels(app, player))
                return e.back(ts, args, eax);
        }
    }
    if (!rise())
        return e.back(ts, args, eax);
    if (stopped)
        return args.eax;
    if (kMenuMusic(app) == 0)
        return keep();
    if (level > 1 && kMusicLevel(app) == -1 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536bfc))
            return args.eax;
        kMusicLevel.set(app, 0);
        if (!event(1, 0) || !control(0))
            return args.eax;
        return keep();
    }
    if (level == 2 && kMusicLevel(app) != 0 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536c10))
            return args.eax;
        kMusicLevel.set(app, 0);
        if (!control(0))
            return args.eax;
        return keep();
    }
    if (level == 4 && kMusicLevel(app) != 0x64 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536c20))
            return args.eax;
        kMusicLevel.set(app, 0x64);
        if (!control(0x64))
            return args.eax;
        return keep();
    }
    if (level == 0 && kMusicLevel(app) != -1 && kChaseMusic(app) != 0)
    {
        if (!trace(0x536bc0))
            return args.eax;
        kMusicLevel.set(app, -1);
        if (!event(kCalmEvent(app), 0))
            return args.eax;
    }
    keep();
    return args.eax;
}

}  // namespace port
}  // namespace nfs3hp
