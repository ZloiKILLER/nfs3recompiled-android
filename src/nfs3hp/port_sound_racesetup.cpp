#include "game_settings.h"
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

// The game's functions the race's sound calls.
constexpr reg32 kSlotStop = 0x416f60, kCopSirenStop = 0x411ca0, kCopSourceReset = 0x411620;
constexpr reg32 kEngineSoundsStart = 0x414830, kCarsReset = 0x4c8370, kCarsRestore = 0x4c8440;
constexpr reg32 kPending = 0x4a33d0, kSetMasterVolume = 0x4e73d0, kStopAllSounds = 0x4e9010, kRemoveBanks = 0x4e8c1c;
constexpr reg32 kBusDown = 0x418790;

/* The slots emptied: 1..0x27, the sound -1 and its effect -1. */
void slotsEmptied(App* app)
{
    for (int32_t slot = 0; slot < kSlotCount; ++slot)
        kSlots.set(app, slot, {-1, -1});
}

}  // namespace

/* 0x4121b0: cop car eax's sounds stopped (its slot 4 on too, unless edx),
 * its source reset for car edx.  eax as the reset leaves it. */
void copRemove(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(3);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    const reg32 car = e.eax;
    const reg32 source = car * 56 + 0x570b78;  // the car's source, 56 bytes a car
    reg32 eax = e.eax;
    if (app->getMemory<reg32>(car * 56 + 0x570b78) != 0)
    {
        if (e.edx == 0 && !g.call(kSlotStop, {car + 4, e.edx, car * 8, car}, eax))
            return;
        if (!g.call(kCopSirenStop, {car, e.edx, car * 8, car}, eax)
            || !g.call(kSlotStop, {car + 0x16, e.edx, car * 8, car}, eax)
            || !g.call(kSlotStop, {car + 0x1b, e.edx, car * 8, car}, eax))
            return;
        const int32_t cops = int32_t(reg32(kCopsPlaying(app)) - 1);  // wrapping, as the code's dec
        g.esi = reg32(cops);
        kCopsPlaying.set(app, cops);
        if (cops < 0)
            kCopsPlaying.set(app, 0);
    }
    if (!g.call(kCopSourceReset, {source, e.edx, car * 8, car * 56}, eax))
        return;
    e.back(cpu, eax);
}

/* 0x4127b0: car eax's +0x624 compared with 0, the flags its caller tests --
 * which no caller does now: nothing. */
void carFlagTest(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

/* 0x4129a0: the race's 0x6fd2b4. */
void raceValue(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(gameSettings(app).recordsHolder));
}

/* 0x4129b0: nothing. */
void soundNothing(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

/* 0x414800: the slots 1..0x27 emptied.  eax as the loop leaves it: 0x27. */
void slotsEmpty(App* app, CPU& cpu)
{
    const Entry e(cpu);
    slotsEmptied(app);
    e.back(cpu, kSlotCount);
}

/* 0x4136e0's: the race's first bank (<audio dir> and 0x536a20's name) loaded
 * into slot 0, with 0x10 for the load, the game's code's frame (6 words
 * pushed) below `esp`. */
Result raceBankLoadOf(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 6 * 4;
    const reg32 name = frame - 0x64;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, name);
    g.esi = copyText(app, name, 0x7a2ab4);
    appendText(app, name, 0x536a20, &g.esi);
    g.edi = name;
    g.sync();
    return bankLoaded(app, cpu, name - 4, name, 0, 0x10, name);
}

/* 0x4136e0: the race's first bank (<audio dir> and 0x536a20's name) loaded
 * into slot 0, with 0x10 for the load. */
void raceBankLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, raceBankLoadOf(app, cpu, e.esp));
}

/* 0x4129c0: the race's sound set up: the slots emptied, the engines started,
 * the cops' radio's banks picked (by the car, the second set for 0x6fd4c0),
 * the speech reset; for each of 8 cars (0x4a33d0 between) its sounds'
 * state cleared; the music's state, a random pick of 0x6fd4b4, the volumes
 * kept for the end, the master volume at 0x78. */
void raceSoundInit(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(6);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    reg32 eax = e.eax;
    if (kEffectsOn(app) != 0)
    {
        app->getMemory<uint8_t>(0x552858) = 4;
        slotsEmptied(app);
        if (!g.call(kEngineSoundsStart, {kSlotCount, e.edx, e.ebx, e.ecx}, eax))
            return;
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
            g.esi = 0xffffffff;
            app->getMemory<reg32>(0x571238) = 0xffffffff;
            app->getMemory<reg32>(0x571240) = 0xffffffff;
        }
        eax = 0;
        app->getMemory<reg32>(0x552860) = 0;
        app->getMemory<reg32>(0x55285c) = 0;
    }
    if (!g.call(kCarsReset, {eax}, eax))
        return;
    // The 8 cars' sounds cleared (1..8 in the tables, the code's).
    for (reg32 car = 1; car <= 8; ++car)
    {
        if (!g.call(kPending, {eax}, eax))
            return;
        g.esi = 0x200;
        for (const reg32 table : {0x570f47u, 0x570ddfu, 0x570f3fu, 0x571097u, 0x57106fu, 0x5710c7u})
            app->getMemory<uint8_t>(car + table) = 0;
        app->getMemory<reg32>(car * 4 + 0x5710ec) = 0;
        app->getMemory<reg32>(car * 4 + 0x571074) = 0;
        app->getMemory<reg32>(car * 4 + 0x5710cc) = 0x200;
    }
    // The slots' pans (the code writes the one byte each time).
    app->getMemory<uint8_t>(0x57126b) = 0x40;
    g.edi = 0xffffffff;
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
    if (!g.call(kSetMasterVolume, {0x78}, eax))
        return;
    e.back(cpu, eax);
}

/* 0x412b50: the race's sound ended: the master volume faded out a step at a
 * time over the game's own delay (0x2fffd turns of a loop a step, which the
 * code burns as it did), the sounds stopped, the banks removed and their
 * copies freed, the bus down, the music stopped, the master volume back,
 * the speech ended, the menus' volumes and music setting back. */
void raceSoundEnd(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(2);
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    reg32 eax = 0;
    for (int32_t level = 0x7f;;)
    {
        // The delay: the generated code's loop, a safepoint a turn.
        volatile reg32 turns = 1;
        while (turns < 0x2fffd)
        {
            safepoint(app, cpu);
            turns = turns + 1;
        }
        if (!g.call(kSetMasterVolume, {reg32(level)}, eax))
            return;
        if (--level < 0)
            break;
    }
    if (kEffectsOn(app) != 0)
    {
        if (!g.call(kStopAllSounds, {eax}, eax) || !g.call(kRemoveBanks, {0xffffffff}, eax))
            return;
        const Result freed = banksFreed(app, cpu, frame - 4);
        if (!freed)
            return;
        eax = reg32(*freed);
    }
    if (kBusUsed(app) != 0 && !g.call(kBusDown, {eax}, eax))
        return;
    const Result stopped = musicStopped(app, cpu, frame - 4, eax);
    if (!stopped)
        return;
    if (!g.call(kSetMasterVolume, {0x7f}, eax) || !g.call(kCarsRestore, {eax}, eax))
        return;
    gameSettings(app).effectsVolume = kEffectsKept(app);
    gameSettings(app).engineVolume = kEngineKept(app);
    kMusicOn.set(app, kMusicOnKept(app));
    e.back(cpu, reg32(kMusicOnKept(app)));
}

}  // namespace port
}  // namespace nfs3hp
