#include "game_settings.h"
#include "native_frontend_loada.h"
#include "port_snd_calls.h"
#include "port_snd_play.h"
#include "port_snd_voices.h"
#include "port_sound_effects.h"
#include "port_sound_slice.h"
#include "port_sound_racesetup.h"

#include <cstdint>

/* The race's sound set up and ended, rewritten (port.h says how): the slots
 * emptied, the engines started, the race's banks and its music's state reset
 * as a race starts; the master volume faded out over the game's own delay
 * loop as it ends, everything stopped and the menus' volumes back; a cop
 * car's sounds stopped as it leaves. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Var<int32_t> kEffectsOn{0x55290c};
constexpr Var<int32_t> kCopsPlaying{0x552608}, kEffectsKept{0x552920}, kEngineKept{0x552924};
constexpr Var<int32_t> kMusicOn{0x552910}, kMusicOnKept{0x552918};
constexpr Var<uint8_t> kBusUsed{0x552930};

// One of the race's sound slots (0x570de8): the sound playing (-1 none) and its effect.
struct Slot
{
    int32_t sound;
    int32_t effect;
};
constexpr Table<Slot> kSlots{0x570de8};
constexpr int32_t kSlotCount = 0x27;

// The game's functions the race's sound calls: its fellow rewrites (as C++), what has no typed function yet (by
// address).
constexpr reg32 kSlotStop = 0x416f60, kCopSirenStop = 0x411ca0, kCopSourceReset = 0x411620;
constexpr reg32 kEngineSoundsStart = 0x414830, kCarsReset = 0x4c8370, kCarsRestore = 0x4c8440;
constexpr reg32 kSetMasterVolume = 0x4e73d0, kStopAllSounds = 0x4e9010, kRemoveBanks = 0x4e8c1c;
constexpr reg32 kBusDown = 0x418790;

/* The slots emptied: 1..0x27, the sound -1 and its effect -1. */
void slotsEmptied(App* app)
{
    for (int32_t slot = 0; slot < kSlotCount; ++slot)
        kSlots.set(app, slot, {-1, -1});
}

}  // namespace

/* 0x4121b0's: cop car `car`'s sounds stopped (its slot 4 on too, unless
 * `keep`), its source reset for car `keep`.  eax as the reset leaves it. */
Result copRemoved(App* app, CPU& cpu, reg32 esp, reg32 car, reg32 keep)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 source = car * 56 + 0x570b78;  // the car's source, 56 bytes a car
    reg32 eax = car;
    const auto slotStop = [&](reg32 slot) {
        cpu.esp = frame;
        return tookEax(guest::direct<kSlotStop, slotStopped>(app, cpu, frame - 4, int32_t(slot)), eax);
    };
    if (app->getMemory<reg32>(car * 56 + 0x570b78) != 0)
    {
        if (keep == 0 && !slotStop(car + 4))
            return {};
        cpu.esp = frame;
        if (!tookEax(guest::direct<kCopSirenStop, copSirenStopped>(app, cpu, frame - 4, int32_t(car)), eax)
            || !slotStop(car + 0x16) || !slotStop(car + 0x1b))
            return {};
        const int32_t cops = int32_t(reg32(kCopsPlaying(app)) - 1);  // wrapping, as the code's dec
        kCopsPlaying.set(app, cops);
        if (cops < 0)
            kCopsPlaying.set(app, 0);
    }
    cpu.esp = frame;
    eax = guest::direct<kCopSourceReset, sourceResetTo>(app, cpu, source, keep);
    if (cpu.terminate)
        return {};
    return int32_t(eax);
}

/* 0x4121b0: cop car eax's sounds stopped (its slot 4 on too, unless edx),
 * its source reset for car edx. */
x86::reg32 copRemove(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copRemoved(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x4127b0: car eax's +0x624 compared with 0, the flags its caller tests --
 * which no caller does now: nothing. */
x86::reg32 carFlagTest(App*, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, e.eax);
}

/* 0x4129a0: the race's 0x6fd2b4. */
x86::reg32 raceValue(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(gameSettings(app).recordsHolder));
}

/* 0x4129b0's: nothing: eax as it came. */
reg32 eaxKept(App*, reg32 eax)
{
    return eax;
}

/* 0x4129b0: nothing. */
x86::reg32 soundNothing(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, eaxKept(app, e.eax));
}

/* 0x414800: the slots 1..0x27 emptied.  eax as the loop leaves it: 0x27. */
x86::reg32 slotsEmpty(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    slotsEmptied(app);
    return e.back(cpu, args, kSlotCount);
}

/* 0x4136e0's: the race's first bank (<audio dir> and 0x536a20's name) loaded
 * into slot 0, with 0x10 for the load, the game's code's frame (6 words
 * pushed) below `esp`. */
Result raceBankLoadOf(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 6 * 4;
    const reg32 name = frame - 0x64;
    copyText(app, name, 0x7a2ab4);
    appendText(app, name, 0x536a20);
    return bankLoaded(app, cpu, name - 4, name, 0, 0x10, name);
}

/* 0x4136e0: the race's first bank (<audio dir> and 0x536a20's name) loaded
 * into slot 0, with 0x10 for the load. */
x86::reg32 raceBankLoad(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceBankLoadOf(app, cpu, e.esp));
}

/* 0x4129c0's: the race's sound set up: the slots emptied, the engines started,
 * the cops' radio's banks picked (by the car, the second set for 0x6fd4c0),
 * the speech reset; for each of 8 cars (0x4a33d0 between) its sounds'
 * state cleared; the music's state, a random pick of 0x6fd4b4, the volumes
 * kept for the end, the master volume at 0x78.  `eax` as the code has it. */
Result raceSoundsSetUp(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 6 * 4;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    if (kEffectsOn(app) != 0)
    {
        app->getMemory<uint8_t>(0x552858) = 4;
        slotsEmptied(app);
        g.atCall();
        if (!tookEax(guest::direct<kEngineSoundsStart, engineSoundsStarted>(app, cpu, frame - 4, reg32(kSlotCount)), eax))
            return {};
        const int32_t car = gameSettings(app).track;
        if (car < 0xa)
        {
            // The cops' radio's banks of this car, in the first set or the second.
            const reg32 set = gameSettings(app).backwards == 0 ? 0x552864 : 0x5528b4;
            app->getMemory<reg32>(0x571240) = app->getMemory<reg32>(set + reg32(car) * 8);
            app->getMemory<reg32>(0x571238) = app->getMemory<reg32>(set + reg32(car) * 8 + 4);
        }
        else
        {
            app->getMemory<reg32>(0x571238) = 0xffffffff;
            app->getMemory<reg32>(0x571240) = 0xffffffff;
        }
        eax = 0;
        app->getMemory<reg32>(0x552860) = 0;
        app->getMemory<reg32>(0x55285c) = 0;
    }
    g.atCall();
    if (!tookEax(guest::direct<kCarsReset, speechSetUp>(app, cpu, frame - 4), eax))
        return {};
    // The 8 cars' sounds cleared (1..8 in the tables, the code's).
    for (reg32 car = 1; car <= 8; ++car)
    {
        g.atCall();
        eax = guest::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
        if (cpu.terminate)
            return {};
        for (const reg32 table : {0x570f47u, 0x570ddfu, 0x570f3fu, 0x571097u, 0x57106fu, 0x5710c7u})
            app->getMemory<uint8_t>(car + table) = 0;
        app->getMemory<reg32>(car * 4 + 0x5710ec) = 0;
        app->getMemory<reg32>(car * 4 + 0x571074) = 0;
        app->getMemory<reg32>(car * 4 + 0x5710cc) = 0x200;
    }
    // The slots' pans (the code writes the one byte each time).
    app->getMemory<uint8_t>(0x57126b) = 0x40;
    app->getMemory<uint8_t>(0x552930) = 0;
    app->getMemory<uint8_t>(0x552859) = 0;
    app->getMemory<uint8_t>(0x55285a) = 0;
    app->getMemory<reg32>(0x552904) = reg32(gameSettings(app).recordsHolder);
    app->getMemory<uint8_t>(0x552834) = 4;
    app->getMemory<reg32>(0x552838) = 0xffffffff;
    app->getMemory<uint8_t>(0x552850) = 0;
    app->getMemory<reg32>(0x55283c) = 0;
    app->getMemory<reg32>(0x552840) = 0;
    app->getMemory<uint8_t>(0x552908) = 1;
    app->getMemory<uint8_t>(0x552909) = 1;
    // A pick among the cars at random.
    const reg32 random = nextRandom(app);
    app->getMemory<reg32>(0x552c34) = random % reg32(gameSettings(app).laps);
    app->getMemory<reg32>(0x552920) = reg32(gameSettings(app).effectsVolume);
    app->getMemory<reg32>(0x552924) = reg32(gameSettings(app).engineVolume);
    app->getMemory<reg32>(0x552c38) = 0;
    g.atCall();
    if (!tookEax(guest::direct<kSetMasterVolume, setMasterVolume>(app, cpu, g.esp - 4, 0x78), eax))
        return {};
    return int32_t(eax);
}

/* 0x4129c0's core: the race's sound set up.  The eax the code has at the call is read by nothing: the
 * slots' work and the speech reset each put a result in it before it is used. */
Result raceSoundInitOf(App* app, CPU& cpu, reg32 esp)
{
    return raceSoundsSetUp(app, cpu, esp, 0);
}

/* 0x4129c0: the race's sound set up. */
x86::reg32 raceSoundInit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceSoundsSetUp(app, cpu, e.esp, e.eax));
}

/* 0x412b50: the race's sound ended: the master volume faded out a step at a
 * time over the game's own delay (0x2fffd turns of a loop a step, which the
 * code burns as it did), the sounds stopped, the banks removed and their
 * copies freed, the bus down, the music stopped, the master volume back,
 * the speech ended, the menus' volumes and music setting back. */
Result raceSoundEndOf(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 2 * 4;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    reg32 eax = 0;
    for (int32_t level = 0x7f;;)
    {
        // The delay: the game's code's loop, a safepoint a turn.
        volatile reg32 turns = 1;
        while (turns < 0x2fffd)
        {
            safepoint(app, cpu);
            turns = turns + 1;
        }
        g.atCall();
        if (!tookEax(guest::direct<kSetMasterVolume, setMasterVolume>(app, cpu, g.esp - 4, level), eax))
            return std::nullopt;
        if (--level < 0)
            break;
    }
    if (kEffectsOn(app) != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kStopAllSounds, stopAllSounds>(app, cpu, g.esp - 4), eax))
            return std::nullopt;
        g.atCall();
        if (!tookEax(guest::direct<kRemoveBanks, removeEveryBank>(app, cpu, g.esp - 4, -1), eax))
            return std::nullopt;
        const Result freed = banksFreed(app, cpu, frame - 4);
        if (!freed)
            return std::nullopt;
        eax = reg32(*freed);
    }
    if (kBusUsed(app) != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kBusDown, busLevelDown>(app, cpu, g.esp - 4), eax))
            return std::nullopt;
    }
    const Result stopped = musicStopped(app, cpu, frame - 4, eax);
    if (!stopped)
        return std::nullopt;
    g.atCall();
    if (!tookEax(guest::direct<kSetMasterVolume, setMasterVolume>(app, cpu, g.esp - 4, 0x7f), eax))
        return std::nullopt;
    g.atCall();
    if (!tookEax(guest::direct<kCarsRestore, speechScriptFreed>(app, cpu, g.esp - 4, eax), eax))
        return std::nullopt;
    gameSettings(app).effectsVolume = kEffectsKept(app);
    gameSettings(app).engineVolume = kEngineKept(app);
    kMusicOn.set(app, kMusicOnKept(app));
    return kMusicOnKept(app);
}

x86::reg32 raceSoundEnd(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceSoundEndOf(app, cpu, e.esp));
}

}  // namespace port
}  // namespace nfs3hp
