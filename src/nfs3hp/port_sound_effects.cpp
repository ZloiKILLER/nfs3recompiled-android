#include "game_settings.h"
#include "port_snd.h"
#include "port_snd_calls.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"
#include "port_sound_music.h"

#include <cstdint>
#include <utility>

/* The game's sound effects, rewritten (port.h says how): an effect of a bank
 * played at the effects' (or engine's, or speech's) volume, panned or placed
 * as the sound mode says; the race's sounds kept in slots; the master and the
 * music's volume; the effect bus; SND's angle of a vector.
 *
 * Each function's integer arithmetic is the game's to the bit (its
 * multiplications by 55 and 63, its shifts by 14, its divisions toward 0).
 * It calls EA's SND library (port_snd.h) and its fellow effects as C++, the
 * game's music and the effect bus, which are not rewritten yet, as the game's
 * code does.  The game's code keeps its frame pointer in ebp, where SND reads
 * an angle and a sample's place it never set: so do the rewrites (HeldEbp). */
namespace nfs3hp
{
namespace port
{

namespace
{

// One of the race's sound slots: the sound playing (-1 none) and its effect.
struct Slot
{
    int32_t sound;
    int32_t effect;
};

// A bank of the game's: its SND handle first.
struct Bank
{
    int32_t handle;
    int32_t other[2];
};

// The game's sound settings and state.
constexpr Var<int32_t> kEffectsOn{0x55290c}, kMenuSoundsOff{0x559234};
constexpr Var<int32_t> kSurround{0x5525a4}, kStereo{0x55292c}, kPlaying{0x552928};
constexpr Var<int32_t> kTechVersion2{0x552854};
constexpr Var<uint8_t> kMusicUnmuted{0x552908}, kBusUsed{0x552930};
constexpr Table<int32_t> kCarMusicShare{0x5525d0};
constexpr Table<Bank> kGameBanks{0x571124};
constexpr Table<Slot> kSlots{0x570de8};
constexpr Table<uint8_t> kSlotBanks{0x552798, 4};

// The banks the code names by their place.
enum BankIndex : int32_t
{
    kMenuBank = 0,       // 0x571124
    kSceneryBank = 1,    // 0x571130
    kCarBank = 2,        // 0x57113c
    kCrashBank = 3,      // 0x571148
    kBank4 = 4,          // 0x571154
    kBank5 = 5,          // 0x571160
    kSpeechBank6 = 6,    // 0x57116c
    kSpeechBank7 = 7,    // 0x571178
    kBank9 = 9,          // 0x571190
    kSpeechBank10 = 10,  // 0x57119c
};

// What the effects call: the streamed music's volume and the effect bus (SND's, by address), the game's own
// music and race sound (as C++).
constexpr reg32 kMusicLevel = 0x4e8b60, kBusInit = 0x4e9c70, kSetBusLevel = 0x4e9dfc;
constexpr reg32 kMusicStop = 0x4109d0, kRaceSoundInit = 0x4129c0;

int32_t bankHandle(App* app, int32_t i) { return kGameBanks(app, i).handle; }

/* A level of 0..127 scaled by a setting as the game's code does it: the
 * setting times `factor`, times the level, over 2^14. */
int32_t scaled(int32_t setting, int32_t factor, int32_t level)
{
    const int32_t s = imul(setting, factor);
    return imul(s, level) >> 14;
}

/* A pan (0..0xffff around) as stereo's (0..127, past it for a pan of none of
 * those), as the game's code makes it. */
uint8_t stereoPan(uint32_t pan)
{
    const uint32_t a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return uint8_t(int32_t(a) >> 8);
}

/* The options' place by the sound mode: 3D or surround by the angle (in
 * surround only with stereo on, else straight ahead), plain stereo by the
 * pan (the middle with stereo off). */
void place(App* app, PlayOptions& o, uint32_t pan)
{
    if (gameSettings(app).soundSetup == 2)
    {
        o.placed = 1;
        o.position = uint16_t(pan);
    }
    else if (kSurround(app) != 0)
    {
        o.placed = 1;
        o.position = kStereo(app) != 0 ? uint16_t(pan) : 0;
    }
    else
    {
        o.placed = 0;
        o.pan = int8_t(kStereo(app) != 0 ? stereoPan(pan) : 0x40);
    }
}

/* The music's volume, less the car's own share for the tech version. */
int32_t musicVolume(App* app)
{
    if (gameSettings(app).musicMode == 1 || kTechVersion2(app) == 1)
    {
        const int32_t v = std::int32_t(gameSettings(app).raceMusicVolume) - kCarMusicShare(app, std::int32_t(gameSettings(app).track));
        return v < 0 ? 0 : v;
    }
    return std::int32_t(gameSettings(app).raceMusicVolume);
}

/* Whether bank `bank` is a speech bank for effect `effect` now. */
bool isSpeech(App* app, int32_t bank, int32_t effect)
{
    const auto is = [&](int32_t i) { return bank == bankHandle(app, i); };
    if (is(kSpeechBank7) || is(kSpeechBank6) || is(kSpeechBank10))
        return true;
    if (is(kBank5) && gameSettings(app).raceType == 3 && effect >= 0x20 && effect < 0x29)
        return true;
    if (gameSettings(app).raceType == 3)
        for (const int32_t i : {11, 14, 12, 13, 8, 15, 16, 17, 18, 19, 20, 21})
            if (is(i))
                return true;
    return is(kBank9) && gameSettings(app).raceOptionCopRadio != 0;
}

/* A sound played: options from SND's defaults, filled by `fill`, where the
 * game's code has them on its stack at `at` (esp there).  Its handle, the
 * effects playing one more. */
template <class Fill>
Result play(App* app, CPU& cpu, reg32 at, Fill fill)
{
    PlayOptions o;
    playDefaults(o);
    fill(o);
    const Result handle = playSound(app, cpu, at - 4, o);
    if (handle)
        kPlaying.set(app, kPlaying(app) + 1);
    return handle;
}

}  // namespace

/* 0x4182b0's: effect `effect` of bank `bank` at `volume`, pan `pan`, pitch
 * `pitch` -- the volume scaled by its kind's (the engine's for bank 0's
 * effect 2, the speech's for the speech's banks, the effects' else).  -1 with
 * the effects off. */
Result bankEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                  uint8_t pitch)
{
    const HeldEbp ebp(cpu, esp - 0xc);
    if (kEffectsOn(app) == 0)
        return -1;
    int32_t level;
    if (bank == 0 && effect == 2)
        level = int32_t(uint32_t(volume) * uint32_t(gameSettings(app).engineVolume)) / 0x7f;
    else if (isSpeech(app, bank, effect))
        level = scaled(std::int32_t(gameSettings(app).speechVolume), 0x7f, volume);
    else if ((bank == bankHandle(app, kBank4) && (effect == 6 || effect == 7))
             || (bank == bankHandle(app, kCarBank) && effect == 0xf))
        level = int32_t(uint32_t(volume) * uint32_t(gameSettings(app).effectsVolume)) / 0x7f;
    else
        level = scaled(std::int32_t(gameSettings(app).effectsVolume), 55, volume);
    return play(app, cpu, esp - 0xc - 0x14, [&](PlayOptions& o) {
        o.bank = int8_t(bank);
        o.patch = effect;
        o.volume = int8_t(level);
        o.bend = int8_t(pitch);
        place(app, o, pan);
    });
}

/* 0x418790's: the effect bus's level down (0). */
Result busLevelDown(App* app, CPU& cpu, reg32 esp)
{
    cpu.esp = esp - 8;
    return guest::direct<kSetBusLevel, setBusMaster>(app, cpu, esp - 8 - 4, 0, 0);
}

/* 0x418770's: the effect bus's level up (127). */
Result busLevelUp(App* app, CPU& cpu, reg32 esp)
{
    cpu.esp = esp - 8;
    return guest::direct<kSetBusLevel, setBusMaster>(app, cpu, esp - 8 - 4, 0, 0x7f);
}

/* Silence -- the music and every sound stopped -- then the master volume
 * brought up a step at a time below `to`, the bus down when it is used.  The
 * code's ebp is `esp`, which SND's master volume reads. */
static Result silenceAndRaise(App* app, CPU& cpu, reg32 esp, int32_t to)
{
    if (!masterVolumeFrom(app, cpu, esp - 4, 0, esp))
        return {};
    cpu.esp = esp;
    if (!guest::direct<kMusicStop, musicStopped>(app, cpu, esp - 4, 0u))
        return {};
    Result r = stopAllSounds(app, cpu, esp - 4);
    for (int32_t level = 0; r && level < to; ++level)
        r = masterVolumeFrom(app, cpu, esp - 4, level, esp);
    if (r && kBusUsed(app) != 0)
        r = busLevelDown(app, cpu, esp - 4);
    return r;
}

/* 0x416f60's: slot `slot`'s sound faded out (5 ticks) and the slot empty; the
 * effects playing, one less (that count back; else the slot as given). */
Result slotStopped(App* app, CPU& cpu, reg32 esp, int32_t slot)
{
    if (kEffectsOn(app) == 0 || slot == -1 || kSlots(app, slot).sound == -1)
        return slot;
    if (!fadeSound(app, cpu, esp - 0x14 - 4, reg32(kSlots(app, slot).sound), 5, -1))
        return {};
    kSlots.set(app, slot, {-1, -1});
    const int32_t playing = kPlaying(app) - 1;
    kPlaying.set(app, playing);
    return playing;
}

/* 0x417080's: slot `slot` plays `effect` at `volume`, `pitch` (each kept to
 * 0..127), `pan`: started -- from the crashes' bank for the crash slots, by
 * the effect for some, else from the slot's own bank -- the slot's old sound
 * stopped; or, the slot playing that effect already, its volume, pitch and
 * place changed.  The last call's answer. */
Result slotPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, int32_t effect, int32_t volume, int32_t pitch,
                  reg32 pan)
{
    const reg32 frame = esp - 0xc;  // ebp as the code has it
    const reg32 at = frame - 8;
    const auto clamp = [](int32_t v) { return v < 0 ? 0 : v > 0x7f ? 0x7f : v; };
    pitch = clamp(pitch);
    volume = clamp(volume);
    int32_t bank;
    bool crash = false;
    for (const int32_t s : {0x12, 0x13, 0x1a, 0x20, 0x21, 0x24, 0x14, 0x15, 0x22, 0x23})
        crash = crash || slot == s;
    if (crash)
        bank = bankHandle(app, kCrashBank);
    else if (effect == 0)
        bank = bankHandle(app, kSceneryBank);
    else if (effect == 2)
        bank = bankHandle(app, kMenuBank);
    else if (effect == 0xa || effect == 0xb)
        bank = bankHandle(app, kCarBank);
    else if (effect == 0xc)
    {
        bank = bankHandle(app, kBank5);
        effect = 1;
    }
    else
        bank = bankHandle(app, kSlotBanks(app, slot));
    const auto start = [&](int32_t level, uint8_t pitchOf) {
        return bankEffect(app, cpu, at - 8, bank, effect, level, pan, pitchOf);
    };
    if (slot == 0x1f || slot == 0x13 || slot == 0x21)
    {
        const Result r = start(volume, 0x40);
        if (!r)
            return {};
        kSlots.set(app, slot, {*r, effect});
        kPlaying.set(app, kPlaying(app) - 1);
        return r;
    }
    const bool engine = slot >= 8 && slot <= 0xb;
    const Slot old = kSlots(app, slot);
    if (effect != old.effect)
    {
        if (old.sound != -1 && slot > 3)
        {
            if (!stopSound(app, cpu, at - 4, reg32(old.sound)))
                return {};
            kPlaying.set(app, kPlaying(app) - 1);
        }
        const Result r = start(engine ? 0 : volume, uint8_t(pitch));
        if (!r)
            return {};
        kSlots.set(app, slot, {*r, effect});
        return r;
    }
    int32_t level;
    if (engine)
        level = scaled(std::int32_t(gameSettings(app).engineVolume), 63, volume);
    else if (slot == 0x10 || slot == 0x11)
        level = scaled(std::int32_t(gameSettings(app).effectsVolume), 80, volume);
    else
        level = scaled(std::int32_t(gameSettings(app).effectsVolume), 55, volume);
    const reg32 sound = reg32(old.sound);
    Result r = setVolumeFrom(app, cpu, at - 4, sound, level, frame);
    // SND's bend's error text reads ebx, esi and edi as the code has them: the slot's offset (8 a slot), the slot,
    // the effect.
    if (r)
        r = setBendFrom(app, cpu, at - 4, sound, pitch, reg32(slot) * 8, reg32(slot), reg32(effect));
    if (r && gameSettings(app).soundSetup == 2)
        r = placeSound(app, cpu, at - 4, sound, pan, 0);
    if (r && kSurround(app) != 0)
        return placeSound(app, cpu, at - 4, sound, kStereo(app) != 0 ? pan : 0, 0);
    if (r)
        r = panSound(app, cpu, at - 4, sound, kStereo(app) != 0 ? stereoPan(pan) : 0x40);
    return r;
}

/* 0x416fc0's: slot `slot` plays effect `effect`, pitch `ecx` times `ebx` (16.16,
 * toward 0), `volume`, `pan`, for `player` -- in the split screen the player's
 * side instead of the pan (the second player's slots 14 on) -- the horn (4) of
 * the first car left out half the time by the random (its quotient by 100
 * back then). */
Result slotEffectPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, int32_t effect, reg32 ecx, reg32 ebx,
                        reg32 volume, reg32 pan, reg32 player)
{
    if (gameSettings(app).playMode == 1)
    {
        pan = player != 0 ? 0xc000 : 0x3fff;
        if (player != 0)
            slot += 0xe;
    }
    const int32_t product = imul(int32_t(ecx), int32_t(ebx));
    const int32_t pitch = int32_t(uint32_t(product) + (product < 0 ? 0xffffu : 0u)) >> 16;
    if (gameSettings(app).track == 0 && effect == 4)
    {
        const reg32 n = nextRandom(app);
        if (n % 0x64 > 0x32)
            return int32_t(n / 0x64);
    }
    return slotPlayed(app, cpu, esp - 3 * 4 - 4 - 8, slot, effect, int32_t(volume), pitch, pan);
}

/* 0x418280's: effect 5 of the car's bank at full volume, in the middle. */
Result carEffectFive(App* app, CPU& cpu, reg32 esp)
{
    return bankEffect(app, cpu, esp - 4 * 4 - 8, bankHandle(app, kCarBank), 5, 0x7f, 0, 0x40);
}

/* 0x4184f0's: effect `effect` of bank `bank` at `volume` scaled by the speech's,
 * pan `pan`, pitch `pitch`.  -1 with the effects off. */
Result speechEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                    uint8_t pitch)
{
    const reg32 frame = esp - 2 * 4;
    // SND's play reads ebp for a sample's place it never set: the code's frame pointer.
    const HeldEbp ebp(cpu, frame);
    if (kEffectsOn(app) == 0)
        return -1;
    return play(app, cpu, frame - 0x14, [&](PlayOptions& o) {
        o.bank = int8_t(bank);
        o.patch = effect;
        o.volume = int8_t(scaled(std::int32_t(gameSettings(app).speechVolume), 0x7f, volume));
        o.bend = int8_t(pitch);
        place(app, o, pan);
    });
}

/* 0x4e8f28's: the angle of the vector (`along`, `across`), 0x400 a turn: the
 * smaller over the larger looked up in the arctangents at 0x5678b8, then put
 * in its octant. */
reg32 vectorAngle(App* app, int32_t along, int32_t across)
{
    reg32 octant = 0;
    if (across < 0)
    {
        octant |= 8;
        across = int32_t(0u - uint32_t(across));
    }
    if (along < 0)
    {
        octant |= 0x10;
        along = int32_t(0u - uint32_t(along));
    }
    reg32 a;
    if (along == across)
        a = 0x80;
    else
    {
        if (along > across)
        {
            std::swap(along, across);
            octant |= 4;
        }
        const reg32 q = reg32((uint64_t(uint32_t(along)) << 32) / uint32_t(across));
        a = app->getMemory<uint8_t>(((q >> 24) + ((q >> 23) & 1)) + 0x5678b8);
    }
    switch (octant)
    {
    case 4: a = 0x100 - a; break;
    case 8: a = 0x200 - a; break;
    case 0xc: a += 0x100; break;
    case 0x10: a = 0u - a; break;
    case 0x14: a -= 0x100; break;
    case 0x18: a -= 0x200; break;
    case 0x1c: a = 0u - a - 0x100; break;
    default: break;
    }
    return a;
}

/* The effects -- each its thunk (the game's registers and stack in, the
 * result in eax out) around what it does. */

/* 0x4181d0's: menu effect `effect` of the first bank at `volume`, the game's
 * code's frame (4 words pushed) below `esp`.  SNDplay's handle (the effect
 * itself, as the code leaves eax, with sounds off). */
Result playMenuEffectOf(App* app, CPU& cpu, reg32 esp, int32_t effect, int32_t volume)
{
    const reg32 frame = esp - 4 * 4;
    const HeldEbp ebp(cpu, frame);
    Result handle = effect;
    if (kMenuSoundsOff(app) == 0 && kEffectsOn(app) != 0)
        handle = play(app, cpu, frame - 0x10, [&](PlayOptions& o) {
            o.bank = int8_t(bankHandle(app, kMenuBank));
            o.volume = int8_t(scaled(std::int32_t(gameSettings(app).effectsVolume), 55, volume));
            o.patch = effect;
            o.bend = 0x40;
            if (gameSettings(app).soundSetup == 2 || kSurround(app) != 0)
            {
                o.placed = 1;
                o.position = 0;
            }
            else
            {
                o.placed = 0;
                o.pan = 0x40;
            }
        });
    return handle;
}

/* 0x4181d0: menu effect eax of the first bank at volume edx.  SNDplay's
 * handle (the effect itself, as the code leaves eax, with sounds off). */
x86::reg32 playMenuEffect(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, playMenuEffectOf(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

/* 0x4182b0 (stdcall 4): effect edx of bank eax at volume ecx, pan ebx, pitch
 * the stack's word. */
x86::reg32 playEffect(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             bankEffect(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ecx), e.ebx,
                        uint8_t(e.arg(app, 0))),
             4);
    return args.eax;
}

/* 0x418280: effect 5 of the car's bank at full volume, in the middle. */
x86::reg32 playCarEffectFive(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, carEffectFive(app, cpu, e.esp));
}

/* 0x4184f0 (stdcall 4): effect edx of bank eax at volume ecx scaled by the
 * speech's, pan ebx, pitch the stack's word.  -1 with the effects off. */
x86::reg32 playSpeechEffect(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             speechEffect(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ecx), e.ebx,
                          uint8_t(e.arg(app, 0))),
             4);
    return args.eax;
}

/* 0x4185e0: a side's angle eax (0..127; edx above 0 for the right) as a pan
 * (0..0xffff). */
x86::reg32 panFromSide(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, panFromSideOf(app, int32_t(e.eax), int32_t(e.edx)));
}

/* 0x4185e0's core: the pan of side angle `angle` (edx `side`: above 0 for the right). */
reg32 panFromSideOf(App*, int32_t angle, int32_t side)
{
    if (side <= 0)
        angle = 0x7f - angle + 0x80;
    int32_t pan = int32_t(uint32_t(angle) << 8) - 0x3ffe;
    if (pan < 0)
        pan += 0x10000;
    return reg32(pan);
}

/* 0x418610: the master volume to nothing. */
x86::reg32 masterSilent(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, masterVolumeFrom(app, cpu, e.frame(1) - 4, 0, e.frame(1)));
}

/* 0x418620: the master volume back (0x78), the music's too when it is not
 * muted. */
x86::reg32 masterBack(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    // The stream's volume (0x4e8b60, by its address) reads ebp: the code's frame.
    const HeldEbp ebp(cpu, e.frame(2));
    const Result r = masterVolumeFrom(app, cpu, e.frame(2) - 4, 0x78, e.frame(2));
    if (!r || kMusicUnmuted(app) == 0)
        return e.finish(cpu, args, r);
    reg32 music;
    if (!pointerGame(app, cpu, e.frame(2), kMusicLevel, {reg32(musicVolume(app))}, {}, music))
        return args.eax;
    return e.back(cpu, args, music);
}

/* 0x4186a0: silence, then the master volume back up to 127. */
x86::reg32 silenceForMenus(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    const HeldEbp ebp(cpu, e.frame(2));
    return e.finish(cpu, args, silenceAndRaise(app, cpu, e.frame(2), 0x80));
}

/* 0x4186f0: silence, then the master volume back up to 0x77, and the race's
 * sounds reset. */
x86::reg32 silenceForRace(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    const HeldEbp ebp(cpu, e.frame(2));
    if (!silenceAndRaise(app, cpu, e.frame(2), 0x78))
        return args.eax;
    cpu.esp = e.frame(2);
    return e.finish(cpu, args, guest::direct<kRaceSoundInit, raceSoundsSetUp>(app, cpu, e.frame(2) - 4, 0u));
}

/* 0x418740: the effect bus set to type 0x14, its level 0. */
x86::reg32 busInit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, busInitOf(app, cpu, e.esp));
}

/* 0x418740's core: the effect bus set to type 0x14, its level 0; esp the frame base. */
Result busInitOf(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 4 * 4;
    cpu.esp = frame;
    if (!guest::direct<kBusInit, initEffectBus>(app, cpu, frame - 4 - 4, 0, 0x14u, -1, -1, 0x7f))
        return std::nullopt;
    cpu.esp = frame;
    return guest::direct<kSetBusLevel, setBusMaster>(app, cpu, frame - 4, 0, 0);
}

/* 0x418770, 0x418790: the bus's level up (127) and down (0). */
x86::reg32 busUp(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, busLevelUp(app, cpu, e.esp));
}

x86::reg32 busDown(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, busLevelDown(app, cpu, e.esp));
}

/* 0x4187a0: the race's slots stopped: 4..11 and 14..36. */
x86::reg32 slotsStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    Result r;
    for (int32_t slot = 4; slot <= 0xb; ++slot)
        if (!(r = slotStopped(app, cpu, e.frame(2) - 4, slot)))
            return args.eax;
    for (int32_t slot = 0xe; slot < 0x25; ++slot)
        if (!(r = slotStopped(app, cpu, e.frame(2) - 4, slot)))
            return args.eax;
    return e.finish(cpu, args, r);
}

/* 0x4187e0: the music muted, or back at its volume. */
x86::reg32 musicMuteToggle(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    reg32 r;
    const bool unmuted = kMusicUnmuted(app) != 0;
    if (!pointerGame(app, cpu, e.frame(2), kMusicLevel, {unmuted ? 0 : reg32(musicVolume(app)), 0}, {}, r))
        return args.eax;
    // Muted, the code writes dl as the call leaves it (0 it set before).
    kMusicUnmuted.set(app, unmuted ? uint8_t(args.edx) : 1);
    return e.back(cpu, args, r);
}

/* 0x416f60: slot eax stopped. */
x86::reg32 slotStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, slotStopped(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x416fc0 (stdcall 12): slot eax plays effect edx, pitch ecx times ebx, volume,
 * pan and player the stack's words. */
x86::reg32 slotEffect(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             slotEffectPlayed(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), e.ecx, e.ebx, e.arg(app, 0),
                              e.arg(app, 1), e.arg(app, 2)),
             0xc);
    return args.eax;
}

/* 0x417080 (stdcall 4): slot eax plays effect edx at volume ecx, pitch ebx,
 * pan the stack's word. */
x86::reg32 slotPlay(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             slotPlayed(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ecx), int32_t(e.ebx),
                        e.arg(app, 0)),
             4);
    return args.eax;
}

/* 0x4e8f28: the angle of the vector (eax, edx), 0x400 a turn. */
x86::reg32 SNDangle(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, vectorAngle(app, int32_t(e.eax), int32_t(e.edx)));
}

}  // namespace port
}  // namespace nfs3hp
