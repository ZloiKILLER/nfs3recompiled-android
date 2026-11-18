#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_load_cardb.h"
#include "native_watcom.h"
#include "port_snd_speech.h"
#include "port_snd_stream_calls.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"
#include "port_sound_speech.h"

#include <cstdint>
#include <cstring>

/* The race's speech, rewritten (port.h says how): what the cops' radio and the
 * race's voices say, each player's (two in the split screen) played in turn
 * from a queue, and faded with the distance of whoever speaks; the co-driver
 * that calls the track's turns as the car comes up to them; the description
 * of a track or a car spoken over the menus' music (but for loading its
 * bank, native_snd_speech.cpp's).
 *
 * Each function is here twice: as C++ with arguments and a result, and as the
 * game's entry, by the name the game's code knows, a thunk around the
 * first (port_snd.h says how).  The integer arithmetic is the game's to the
 * bit (its multiplications by 127 and 63, its divisions toward 0); the
 * co-driver's floating point is the game's too, in doubles, with the
 * integer conversions the x87 makes.  The rewrites of SND and the speech
 * itself are called as C++; what is not rewritten yet (the effects, the
 * music, the files) is called as the game's code calls it, by address -- with
 * esi and edi as the code has them (SND's errors push them).
 *
 * The game's state, per player: kSaid (8 apart) a line said, kToTell one to
 * tell the game of, kPlaying a line playing, kRadioSound the radio's sound,
 * kLineKind the line's kind, kToClear a line to clear, kLineSound the line's
 * sound, kQueued the lines queued (in the two queues, 0x24 bytes a line),
 * kOtherSound another sound, kRadioSpeaker (8 apart) who speaks on the radio,
 * kRadioOn the radio on; the radio's last chatter and the last cop line. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Table<int32_t> kSaid{0x7dd334, 8}, kToTell{0x7dd340}, kPlaying{0x7dd348}, kRadioSound{0x7dd350},
    kLineKind{0x7dd358}, kToClear{0x7dd368}, kLineSound{0x7dd370}, kQueued{0x7dd378}, kRadioDistance{0x7dd388},
    kOtherSound{0x7dd390}, kRadioSpeaker{0x7dd818, 8}, kRadioOn{0x7dd81c, 8};
constexpr reg32 kQueueFirst = 0x7dd398, kQueueSecond = 0x7dd5d8;   // the two queues of lines

constexpr Var<int32_t> kLineLock{0x560118}, kLastChatter{0x560ac4}, kLastCopLine{0x560ac8};
constexpr Var<int32_t> kSurround{0x5525a4}, kStereo{0x55292c};
constexpr Var<int32_t> kRaceTime{0x7d3684};
constexpr Table<reg32> kCars{0x5efa48};
constexpr Table<reg32> kRaceCars{0x5efc48};
constexpr Var<int32_t> kRaceCarCount{0x5efda0};

// The co-driver's: each player's script of calls (a pointer) and its lap, the script's start.
constexpr Table<reg32> kScript{0x7dd380};
constexpr Table<int32_t> kScriptLap{0x7dd360};
constexpr Var<reg32> kScriptStart{0x7dd828};
constexpr Var<reg32> kTrackLength{0x5dd940};
constexpr reg32 kCallCodes = 0x560ae2, kCallEffects = 0x560ac9;   // by call: mirrored, and its effect
constexpr Var<float> kRevsScale{0x540330};
constexpr Var<double> kLeadScale{0x540334};

// The description's: bank, header copy, part, state.
constexpr Var<int32_t> kDescriptionOn{0x7dd830}, kDescriptionLoaded{0x7dd838}, kDescriptionKind{0x7dd83c};
constexpr Var<int32_t> kDescriptionBank{0x7dd844}, kDescriptionPart{0x7dd834}, kDescriptionStopped{0x560af8};
constexpr Var<reg32> kDescriptionHeader{0x7dd84c};
constexpr Var<int32_t> kSpokenSound{0x7dd82c};
constexpr reg32 kCarsFolder = 0x7a28b4;
constexpr Table<reg32> kCarNames{0x560b50}, kMenuTracks{0x560afc}, kCarTracks{0x560b18};

// What is not rewritten yet.
constexpr reg32 kPlayEffect = 0x4182b0, kPlaySpeech = 0x4184f0, kTellGame = 0x482030, kStreamVolume = 0x4e7470;
constexpr reg32 kFree = 0x4e1890, kSprintf = 0x4df690;
constexpr reg32 kPathEnd = 0x4529a0, kTrackName = 0x4392b0, kLoadQuietly = 0x4e0fe0;
constexpr reg32 kLoadBank = 0x410b50, kTimersRun = 0x4e7630, kSettle = 0x418c60;
constexpr reg32 kLeaderGap = 0x485a60, kDescriptionLoad = 0x4c9240;
constexpr reg32 kDescriptionFormat = 0x54030c, kScriptFolder = 0x7a27b4;
constexpr reg32 kSecondPlayerSuffix = 0x540354;

int32_t bankHandle(App* app, int32_t i) { return kSpeechBankHandles(app, i); }

/* A pan (0..0xffff around) as stereo's (0..127), as the game's code makes it. */
int32_t stereoPanOf(uint32_t pan)
{
    const uint32_t a = int32_t(pan) > 0x3fff && int32_t(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return int32_t(a) >> 8;
}

/* A distance's volume: the part of 0x15f9 it leaves, over 0x15f9, times 127. */
int32_t distanceVolume(int32_t distance)
{
    return idiv(imul(int32_t(0x15f9u - uint32_t(distance)), 0x7f), 0x15f9);
}

/* A level of the speech: the setting times 127, times `level`, over 2^14. */
int32_t speechLevel(App* app, int32_t level)
{
    const int32_t setting = gameSettings(app).speechVolume;
    return imul(level, int32_t((uint32_t(setting) << 7) - uint32_t(setting))) >> 14;
}

reg32 queueOf(int32_t player) { return player != 0 ? kQueueSecond : kQueueFirst; }

}  // namespace

/* 0x4c8ac0: each player's line said, when its sound is done: marked over (the
 * line lock cleared).  eax as the code leaves it: the second player's status,
 * 0, or 8. */
static Result linesSaidMarked(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 5 * 4;
    reg32 eax = 0;
    for (int32_t p = 0; p < 2; ++p)
    {
        const Result status = soundStatus(app, cpu, frame - 4, reg32(kOtherSound(app, p)));
        if (!status)
            return {};
        eax = reg32(*status);
        if (eax == 0)
            continue;
        eax = reg32(p * 8);
        if (kSaid(app, p) == 0)
            continue;
        kLineLock.set(app, 0);
        kSaid.set(app, p, 0);
        kToClear.set(app, p, 0);
    }
    return int32_t(eax);
}

/* 0x4c85a0: line `line` said, or queued (16 at most) while the player's line
 * plays; held back while a line is being said (the line lock), silent over the
 * radio for kind 5.  eax as the code leaves it: the player's place in the
 * tables (times 4), the told game's answer, or the line's place in the queue
 * (times 9). */
Result speechLineSaid(App* app, CPU& cpu, reg32 esp, reg32 line)
{
    const reg32 frame = esp - 6 * 4;
    SpeechLine& l = record<SpeechLine>(app, line);
    const int32_t p = l.player > 1 ? 0 : l.player;
    reg32 eax = reg32(p) * 4;
    if (kPlaying(app, p) == 0)
    {
        if (kLineLock(app) != 0)
            return int32_t(eax);
        // Over the radio a line of kind 5 plays at no volume.
        const int32_t volume = kRadioOn(app, 0) != 0 && l.kind == 5 ? 0 : l.volume;
        cpu.esp = frame;
        const Result handle =
            guest::direct<kPlayEffect, bankEffect>(app, cpu, frame - 8, l.bank, l.effect, volume, l.pan, uint8_t(l.pitch));
        if (!handle)
            return {};
        kLineSound.set(app, p, *handle);
        kPlaying.set(app, p, 1);
        if (l.kind == 2)
        {
            kToTell.set(app, p, 1);
            const Result told = callAt(app, cpu, frame, kTellGame, {reg32(l.delay), reg32(p), 0});
            if (!told)
                return {};
            eax = reg32(*told);
        }
        if (l.kind == 4 || l.kind == 3)
            kLineKind.set(app, p, l.kind);
        return int32_t(eax);
    }
    const int32_t count = kQueued(app, p);
    if (count >= 0x10)
        return int32_t(eax);
    kQueued.set(app, p, count + 1);
    const reg32 to = queueOf(l.player) + reg32(count) * 9 * 4;
    for (reg32 k = 0; k < 0x24; k += 4)
        app->getMemory<reg32>(to + k) = app->getMemory<reg32>(line + k);
    return int32_t(reg32(count) * 9);
}

/* 0x4c86d0: player's speech tick: a line done ended (the game told, where it
 * asked), the next of the queue played and the queue moved up.  eax as the
 * code leaves it. */
Result playerSpeechTicked(App* app, CPU& cpu, reg32 esp, int32_t player)
{
    const reg32 frame = esp - 6 * 4;
    if (!linesSaidMarked(app, cpu, frame - 4))
        return {};
    const int32_t p = player > 1 ? 0 : player;
    if (kPlaying(app, p) != 0)
    {
        const Result status = soundStatus(app, cpu, frame - 4, reg32(kLineSound(app, p)));
        if (!status)
            return {};
        if (*status != 0)
        {
            const int32_t toTell = kToTell(app, p);
            kPlaying.set(app, p, 0);
            if (toTell != 0)
            {
                kToTell.set(app, p, 0);
                if (!callAt(app, cpu, frame, kTellGame, {0xffffffff, reg32(p), 0}))
                    return {};
            }
            if (kLineKind(app, p) == 4)
                kLineKind.set(app, p, 0);
        }
    }
    reg32 eax = reg32(p) * 4;
    if (kQueued(app, p) != 0 && kPlaying(app, p) == 0)
    {
        Result said = speechLineSaid(app, cpu, frame - 4, queueOf(p));
        if (!said)
            return {};
        eax = reg32(*said);
        const int32_t left = kQueued(app, p) - 1;
        kQueued.set(app, p, left);
        if (left != 0)
            for (int32_t b = 0; b < kQueued(app, p); ++b)
            {
                const reg32 to = queueOf(p) + reg32(b) * 0x24;
                for (reg32 k = 0; k < 0x24; k += 4)
                    app->getMemory<reg32>(to + k) = app->getMemory<reg32>(to + 0x24 + k);
                eax = to;
            }
    }
    return int32_t(eax);
}

/* 0x4c8810's: the radio's sound of `player` ticked: when it is done, its
 * line's volume back; else, the speaker still the one, at `distance`, `pitch`
 * and `pan`: its volume, pitch and place set. */
Result radioSoundTicked(App* app, CPU& cpu, reg32 esp, reg32 speaker, reg32 pan, int32_t pitch, int32_t distance,
                        int32_t player)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 sp = frame - 8;
    const reg32 sound = reg32(kRadioSound(app, player));
    const Result done = soundStatus(app, cpu, sp - 4, sound);
    if (!done)
        return {};
    if (*done != 0 && kRadioOn(app, player) != 0)
    {
        kRadioOn.set(app, player, 0);
        if (kPlaying(app, player) == 0)
            return player * 8;
        // The setting times 127 times 127 over 2^14.
        const uint32_t setting = uint32_t(gameSettings(app).speechVolume);
        const int32_t volume = int32_t(setting + (((setting << 6) - setting) << 8)) >> 14;
        return setVolumeFrom(app, cpu, sp - 4, reg32(kLineSound(app, player)), volume, frame);
    }
    if (kRadioOn(app, player) == 0 || speaker != reg32(kRadioSpeaker(app, player)))
        return player * 8;
    const int32_t level = distance < 0x15f9 ? distanceVolume(distance) : 0;
    const int32_t volume = imul(level, int32_t((uint32_t(gameSettings(app).speechVolume) << 7) - uint32_t(gameSettings(app).speechVolume))) >> 14;
    if (!setVolumeFrom(app, cpu, sp - 4, sound, volume, frame))
        return {};
    const int32_t bend = int32_t(uint32_t(pitch) << 6) / 0x10000;
    // SND's bend's error text reads ebx, esi and edi as the code has them: the player, the pan, the sound.
    if (!setBendFrom(app, cpu, sp - 4, sound, bend, reg32(player), pan, sound))
        return {};
    if (kSurround(app) != 0)
        return placeSound(app, cpu, sp - 4, sound, kStereo(app) != 0 ? pan : 0, 0);
    return panSound(app, cpu, sp - 4, sound, kStereo(app) != 0 ? stereoPanOf(pan) : 0x40);
}

/* 0x4c89b0's: the radio of `player`, if it is off, on: `speaker` at `distance`,
 * `pitch`, `pan` -- a chatter of 10 at random, not the last one -- the lines
 * playing lowered for it.  The chatter's sound (its handle), or the player. */
Result radioStarted(App* app, CPU& cpu, reg32 esp, reg32 speaker, int32_t distance, int32_t pitch, reg32 pan,
                    int32_t player)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 sp = frame - 8;
    if (kRadioOn(app, player) != 0)
        return player;
    reg32 chatter = nextRandom(app) % 10;
    while (chatter == reg32(kLastChatter(app)))
        chatter = nextRandom(app) % 10;
    kRadioOn.set(app, player, 1);
    kRadioSpeaker.set(app, player, int32_t(speaker));
    const int32_t volume = distanceVolume(distance);
    const int32_t bend = int32_t(uint32_t(pitch) << 6) / 0x10000;
    const int32_t pitchOf = bend > 0x7f ? 0x7f : bend;
    if (kPlaying(app, 0) != 0 && player == 0 && !setVolumeFrom(app, cpu, sp - 4, reg32(kLineSound(app, 0)), 0, frame))
        return {};
    if (kPlaying(app, 1) != 0 && player == 0 && !setVolumeFrom(app, cpu, sp - 4, reg32(kLineSound(app, 1)), 0, frame))
        return {};
    cpu.esp = sp;
    const Result handle = guest::direct<kPlayEffect, bankEffect>(app, cpu, sp - 8, bankHandle(app, kSpeechBankCopRadio),
                                                                 int32_t(chatter), volume, pan, uint8_t(pitchOf));
    if (!handle)
        return {};
    kLastChatter.set(app, int32_t(chatter));
    kRadioSound.set(app, player, *handle);
    return handle;
}

/* 0x4c8b30's: `player`'s cop radio of `kind` (1 a chase, 2, 3, else), from
 * `distance`, unless the race is replayed: a line of its kind at random, not
 * the last one, at the distance's volume -- the lines playing lowered for it.
 * The line (its effect); the distance when replayed. */
Result copRadioStarted(App* app, CPU& cpu, reg32 esp, int32_t player, int32_t kind, int32_t distance)
{
    const reg32 frame = esp - 4 * 4;
    const reg32 sp = frame - 8;
    if (gameSettings(app).replayMode != 0)
        return distance;
    int32_t choices, first;
    switch (kind)
    {
    case 1: choices = 3; first = 0x20; break;
    case 2: choices = 1; first = 0x13; break;
    case 3: choices = 2; first = 0x11; break;
    default: choices = 2; first = 0x10; break;
    }
    const int32_t volume = distanceVolume(distance);
    kRadioDistance.set(app, player, volume);
    int32_t line = first + int32_t(nextRandom(app) % reg32(choices));
    if (choices != 1)
        while (line == kLastCopLine(app))
            line = first + int32_t(nextRandom(app) % reg32(choices));
    const reg32 pan = gameSettings(app).playMode == 1 ? (player != 0 ? kSideRight : kSideLeft) : 0;
    const int32_t playing = kPlaying(app, 0);
    kSaid.set(app, player, 1);
    if (playing != 0 && player == 0 && !setVolumeFrom(app, cpu, sp - 4, reg32(kLineSound(app, 0)), 0, frame))
        return {};
    if (kPlaying(app, 1) != 0 && player == 0 && !setVolumeFrom(app, cpu, sp - 4, reg32(kLineSound(app, 1)), 0, frame))
        return {};
    kLineLock.set(app, 1);
    cpu.esp = sp;
    const Result handle = guest::direct<kPlayEffect, bankEffect>(app, cpu, sp - 8, bankHandle(app, kSpeechBankCopRadio),
                                                                 line, volume, pan, uint8_t(0x40));
    if (!handle)
        return {};
    kOtherSound.set(app, player, *handle);
    if (first == 0x20)
        kToClear.set(app, player, 1);
    cpu.esp = sp;
    if (!guest::direct<kSettle, musicEventResumed>(app, cpu, sp - 4, 0u, 0u))
        return {};
    kLastCopLine.set(app, line);
    return line;
}

/* 0x4c8ca0's: a line of the speech bank queued: `effect`, kind 2, `delay`, for
 * player `player` (its side in the split screen). */
static Result speechLineQueued(App* app, CPU& cpu, reg32 esp, reg32 effect, reg32 delay, reg32 player)
{
    const reg32 frame = esp - 4 * 4;
    const reg32 line = frame - 0x24, sp = line;
    SpeechLine& l = record<SpeechLine>(app, line);
    l.pitch = 0x40;
    l.volume = 0x7f;
    l.pan = gameSettings(app).playMode == 1 ? (player != 0 ? kSideRight : kSideLeft) : 0;
    l.effect = int32_t(effect);
    l.player = int32_t(player);
    l.delay = int32_t(delay);
    l.bank = bankHandle(app, kSpeechBankSpeech);
    l.kind = 2;
    return speechLineSaid(app, cpu, sp - 4, line);
}

/* 0x4c8370's: the speech set up for the race: each player's lines, radio and
 * description cleared, the co-driver's script of the track (by its number, its
 * direction's file) loaded.  eax: the script, as loaded. */
Result speechSetUp(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 5 * 4;
    const reg32 name = frame - 0x64, sp = name;
    for (int32_t p = 0; p < 2; ++p)
    {
        kPlaying.set(app, p, 0);
        kRadioOn.set(app, p, 0);
        kQueued.set(app, p, 0);
        kToClear.set(app, p, 0);
    }
    const reg32 track = reg32(gameSettings(app).track);
    kLineLock.set(app, 0);
    kDescriptionLoaded.set(app, 0);
    kDescriptionOn.set(app, 0);
    cpu.esp = sp;
    guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kDescriptionFormat, guest::Words{kScriptFolder, track, track});
    if (cpu.terminate)
        return {};
    appendText(app, name, gameSettings(app).backwards != 0 ? 0x540320 : 0x540328);
    cpu.esp = sp;
    const std::optional<reg32> loaded = guest::direct<kLoadQuietly, ea::loadQuietly>(app, cpu, name, 0u);
    if (!loaded)
        return {};
    const Result script = int32_t(*loaded);
    kScriptStart.set(app, reg32(*script));
    kScript.set(app, 1, reg32(*script));
    kScript.set(app, 0, reg32(*script));
    kScriptLap.set(app, 1, 0);
    kScriptLap.set(app, 0, 0);
    return script;
}

/* 0x4c8440's: the co-driver's script freed. */
Result speechScriptFreed(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 script = kScriptStart(app);
    if (script == 0)
        return int32_t(eax);
    cpu.esp = frame;
    const std::optional<reg32> r = guest::direct<kFree, ea::purgeBlock>(app, cpu, script);
    if (!r)
        return {};
    kScriptStart.set(app, 0);
    kScript.set(app, 1, 0);
    kScript.set(app, 0, 0);
    return int32_t(*r);
}

/* 0x4c8f90's: a line of the race voice's bank queued -- `effect`, `kind`, for
 * `player` -- once the race is under way (0x200 ticks): the line's kind kept
 * as the player's.  eax as the code leaves it: the line's answer, or the
 * effect as it came. */
Result raceLineQueued(App* app, CPU& cpu, reg32 esp, reg32 effect, reg32 kind, reg32 player)
{
    const reg32 frame = esp - 4 * 4;
    if (kRaceTime(app) < 0x200)
        return int32_t(effect);
    const reg32 line = frame - 0x24;
    SpeechLine& l = record<SpeechLine>(app, line);
    l.pitch = 0x40;
    l.volume = 0x7f;
    l.pan = gameSettings(app).playMode == 1 ? (player != 0 ? kSideRight : kSideLeft) : 0;
    l.effect = int32_t(effect);
    l.player = int32_t(player);
    l.kind = int32_t(kind);
    l.bank = bankHandle(app, kSpeechBankLines);
    kLineKind.set(app, int32_t(player), int32_t(kind));
    return speechLineSaid(app, cpu, line - 4, line);
}

/* 0x4c9010's: `player`'s race voice for `event`, unless one is queued: for 1,
 * line 1 mostly, else 10 or 11; for 8, 8 or 9; else the event's own.  eax as the code leaves it: the event when one is queued. */
Result raceEventLineQueued(App* app, CPU& cpu, reg32 esp, reg32 player, reg32 event)
{
    const reg32 frame = esp - 3 * 4;
    if (kLineKind(app, int32_t(player)) != 0)
        return int32_t(event);
    reg32 effect;
    if (event == 1)
    {
        if (nextRandom(app) % 100 > 0x46)
            effect = (nextRandom(app) & 1) + 0xa;
        else
            effect = 1;
    }
    else if (event == 8)
        effect = (nextRandom(app) & 1) + 8;
    else
        effect = event;
    return raceLineQueued(app, cpu, frame - 4, effect, 4, player);
}

/* 0x4c9090's: the race voice's line for `player` about `car`'s gap to the
 * leader (`toSay`: to be said): the gap (0x485a60's, or 0x380 with none).
 * eax as the code leaves it: the gap; `ecx` is what the code has in it when no
 * car is first. */
Result gapLineQueued(App* app, CPU& cpu, reg32 esp, reg32 car, reg32 player, reg32 toSay, reg32 ecx)
{
    const reg32 frame = esp - 4 * 4;
    const reg32 sp = frame - 4;
    if ((load<uint8_t>(app, car + 0x200) & 1) != 0 && load<reg32>(app, car + 0x2c4) == 3)
        return 0;
    reg32 gap = ecx;
    for (int32_t i = 0; i < kRaceCarCount(app); ++i)
    {
        const reg32 other = kRaceCars(app, i);
        if (load<int32_t>(app, other + 0x2bc) == 1)
        {
            const Result measured = callAt(app, cpu, sp, kLeaderGap, {other, car});
            if (!measured)
                return {};
            gap = reg32(*measured);
            break;
        }
    }
    if (kLineKind(app, int32_t(player)) != 0)
        return int32_t(gap);
    if (load<int32_t>(app, car + 8) == 0)
        gap = 0x380;
    if (toSay == 0)
        return int32_t(gap);
    if (int32_t(gap) < 0x40)
    {
        if (load<int32_t>(app, car + 0x2bc) != 1)
            gap = 0x40;
        else
        {
            if (!raceLineQueued(app, cpu, sp - 4, (nextRandom(app) & 1) + 0xa, 4, player))
                return {};
            return int32_t(gap);
        }
    }
    if (int32_t(gap) > 0x300)
    {
        if (!raceLineQueued(app, cpu, sp - 4, (nextRandom(app) & 1) + 0x1d, 4, player))
            return {};
        return int32_t(gap);
    }
    // The code takes the random number's low bit after a shift by 4: always 0.
    (void)nextRandom(app);
    const reg32 effect = reg32(int32_t(gap) / 64) + 0x11 - 1;
    if (!raceLineQueued(app, cpu, sp - 4, effect, 4, player))
        return {};
    return int32_t(gap);
}

/* 0x4c91a0's: `player`'s race voice for a lap time (`seconds` less 3), unless
 * one is queued: by tens of seconds where the voice has them, else line 0x4a.
 * eax as the code leaves it. */
Result lapTimeLineQueued(App* app, CPU& cpu, reg32 esp, reg32 player, int32_t seconds)
{
    const reg32 frame = esp - 4 * 4;
    seconds = int32_t(uint32_t(seconds) + 3);
    if (kLineKind(app, int32_t(player)) != 0 || seconds < 0x32)
        return int32_t(player);
    const int32_t language = gameSettings(app).hud[0].speedometerForm;
    const bool metric = gameSettings(app).language == 0;
    // By tens of seconds: as the code's two tests of the voice's language
    // and of the units go.
    bool tens;
    if (seconds < 0xd2 && (language == 1 || language == 2 || (language == 0 && metric)))
        tens = true;
    else if (seconds >= 0x136)
        tens = false;
    else if (language == 1 || language == 2)
        tens = false;
    else if (language != 0)
        tens = true;
    else
        tens = !metric;
    const reg32 effect = tens ? reg32(seconds / 10 + 0x2b) : 0x4a;
    return raceLineQueued(app, cpu, frame - 4, effect, 4, player);
}

/* 0x4c8d10's: `player`'s co-driver (the game's option on): along the track's
 * script of calls, as the car comes up to each -- by its speed, a little ahead
 * -- the call queued; the script started over on a new lap, or when the car
 * stops or goes backwards.  The code leaves the compare's bits in ax: the x87's
 * status word, whose other bits nothing sets (0). */
Result coDriverTicked(App* app, CPU& cpu, reg32 esp, int32_t player)
{
    const reg32 frame = esp - 6 * 4;
    const reg32 sp = frame - 0x14;
    const reg32 slot = reg32(player) * 4;
    if (kScript(app, player) == 0 || gameSettings(app).raceOptionCopRadio == 0)
        return int32_t(slot);
    if (player == 0 && std::int32_t(gameSettings(app).laps) <= kScriptLap(app, 0))
        return int32_t(slot);
    const reg32 car = player == 0 && gameSettings(app).playMode != 1 ? kCars(app, std::int32_t(gameSettings(app).localPlayer)) : kCars(app, player);
    uint32_t speedBits = load<uint32_t>(app, car + 0x7c0);
    const int32_t ahead = load<int32_t>(app, car + 0x224);
    const reg32 script = kScript(app, player);
    reg32 length = kTrackLength(app);
    int32_t position = load<int32_t>(app, script);
    int32_t reach = load<int32_t>(app, script + 4);
    // The revs, unsigned (as they are unless 0 is at or above them), scaled and
    // chopped.
    const float revs = load<float>(app, car + 0x4f4);
    const float unsignedRevs = 0.0 < revs || revs != revs ? revs : -revs;
    const int32_t revsScaled = truncatedInteger(double(unsignedRevs) * double(kRevsScale(app)));
    if (reach < 2)
        reach = 2;
    if (gameSettings(app).backwards != 0)
    {
        speedBits ^= 0x80000000u;
        position = int32_t(length) - position;
    }
    float speed;
    std::memcpy(&speed, &speedBits, 4);
    // How far ahead: the speed (fistp'd) times the call's reach (16.16,
    // rounded), over 2^16, a sixth of it.
    const int32_t speedInteger = nearestInteger(double(speed) * kLeadScale(app));
    const int64_t product = int64_t(speedInteger) * reach;
    const reg32 low = reg32(product), high = reg32(uint64_t(product) >> 32);
    const int32_t rounded = int32_t((low >> 16) + (high << 16) + ((low >> 15) & 1));
    int32_t lead = rounded / 0x10000 / 6;
    const uint16_t compared = comparedStatus(0, 0.0, double(speed));
    // The speed below 0 (a NaN is not); or the car goes backwards.
    if (double(speed) < 0.0 || load<int32_t>(app, car + 0x998) != 0)
    {
        kScriptLap.set(app, player, load<int32_t>(app, kCars(app, player) + 0x204));
        kScript.set(app, player, kScriptStart(app));
        return int32_t((reg32(lead) & 0xffff0000u) | compared);
    }
    position = int32_t(reg32(position) + reg32(length) * reg32(kScriptLap(app, player)));
    lead = int32_t(reg32(lead) + reg32(ahead));
    // The script's next word: its pointer moved on by one, read.
    const auto next = [&]() {
        const reg32 at = kScript(app, player) + 4;
        kScript.set(app, player, at);
        return load<int32_t>(app, at);
    };
    reg32 pointer = 0;
    bool passed = false;
    if (position < ahead)
    {
        // Passed: skipped to the end of this entry's calls.
        kScript.set(app, player, kScript(app, player) + 4);
        int32_t word;
        do
            word = next();
        while (word > -1);
        pointer = kScript(app, player);
        passed = true;
    }
    else if (lead >= position)
    {
        kScript.set(app, player, kScript(app, player) + 4);
        for (;;)
        {
            const int32_t word = next();
            pointer = kScript(app, player);
            if (word <= -1)
                break;
            reg32 code = reg32(word);
            if (gameSettings(app).mirrored != 0)
                code = load<uint8_t>(app, reg32(word) + kCallCodes);
            reg32 delay;
            if (code == 0xf)
            {
                reg32 mine = load<int32_t>(app, kScript(app, player) + 4) == 0x11 ? 0xb : 3;
                delay = gameSettings(app).mirrored != 0 ? (mine == 3 ? 0xb : 3) : mine;
            }
            else
                delay = reg32(int32_t(int8_t(load<uint8_t>(app, code + kCallEffects + 3))));
            if (code == 0xc && revsScaled <= 0x3c)
                continue;
            if (!speechLineQueued(app, cpu, sp - 4, code, delay, reg32(player)))
                return {};
        }
        passed = true;
    }
    if (passed)
        kScript.set(app, player, pointer + 4);
    const reg32 after = kScript(app, player);
    if (load<reg32>(app, after) == 0xfffffffe)
    {
        kScript.set(app, player, kScriptStart(app));
        kScriptLap.set(app, player, kScriptLap(app, player) + 1);
    }
    return int32_t(slot);
}

/* 0x4c9340's: the description's part `part` spoken.  The sound's handle. */
static Result descriptionSpoken(App* app, CPU& cpu, reg32 esp, reg32 part)
{
    const reg32 frame = esp - 5 * 4;
    cpu.esp = frame;
    const Result handle = guest::direct<kPlaySpeech, speechEffect>(app, cpu, frame - 8, kDescriptionBank(app),
                                                                   int32_t(part), 0x7f, 0u, uint8_t(0x40));
    if (!handle)
        return {};
    kSpokenSound.set(app, *handle);
    kDescriptionPart.set(app, int32_t(part));
    kDescriptionOn.set(app, 1);
    return handle;
}

/* The music's stream to `volume` (0x4e7470's answer), from `sp`. */
static Result musicTo(App* app, CPU& cpu, reg32 sp, int32_t volume)
{
    cpu.esp = sp;
    return guest::direct<kStreamVolume, stream::fadeStream>(app, cpu, sp - 4, 0x64, volume);
}

/* The music up to its volume again, as the description ends (0x4e7470's
 * answer). */
static Result musicBackUp(App* app, CPU& cpu, reg32 sp)
{
    kDescriptionStopped.set(app, 0);
    return musicTo(app, cpu, sp, gameSettings(app).menuMusicVolume);
}

/* A block of the game's freed (0x4e1890), from `sp`: its answer. */
static Result blockFreed(App* app, CPU& cpu, reg32 sp, reg32 block)
{
    cpu.esp = sp;
    const std::optional<reg32> r = guest::direct<kFree, ea::purgeBlock>(app, cpu, block);
    if (!r)
        return {};
    return int32_t(*r);
}

/* 0x4c9380's: the description's tick: the next part spoken when one is done;
 * when they are (or it is stopped), the bank removed and the music back up.
 * `eax` is what the code has in it when there is nothing to do. */
static Result descriptionTicked(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 5 * 4;
    const HeldEbp ebp(cpu, frame);
    if (kDescriptionOn(app) == 0)
        return int32_t(eax);
    if (kDescriptionKind(app) == 0x33)
    {
        if (kDescriptionStopped(app) == 0)
        {
            const Result status = soundStatus(app, cpu, frame - 4, reg32(kSpokenSound(app)));
            if (!status)
                return {};
            if (*status == 0)
                return 0;
        }
        if (!fadeSound(app, cpu, frame - 4, reg32(kSpokenSound(app)), 0, -1))
            return {};
        kDescriptionOn.set(app, 0);
        return musicBackUp(app, cpu, frame);
    }
    if (kDescriptionStopped(app) != 0)
    {
        if (!fadeSound(app, cpu, frame - 4, reg32(kSpokenSound(app)), 0, -1)
            || !removeEveryBank(app, cpu, frame - 4, kDescriptionBank(app)))
            return {};
        const reg32 header = kDescriptionHeader(app);
        if (header != 0)
        {
            if (!blockFreed(app, cpu, frame, header))
                return {};
            kDescriptionHeader.set(app, 0);
        }
        kDescriptionOn.set(app, 0);
        kDescriptionLoaded.set(app, 0);
        return musicBackUp(app, cpu, frame);
    }
    const Result status = soundStatus(app, cpu, frame - 4, reg32(kSpokenSound(app)));
    if (!status)
        return {};
    if (*status == 0)
        return 0;
    const int32_t part = kDescriptionPart(app) + 1;
    kDescriptionPart.set(app, part);
    const Result has = bankHasPatch(app, cpu, frame - 4, kDescriptionBank(app), part);
    if (!has)
        return {};
    if (*has != 0)
        return descriptionSpoken(app, cpu, frame - 4, reg32(kDescriptionPart(app)));
    kDescriptionStopped.set(app, 1);
    return *has;
}

/* 0x4c9490's: the description, if it is spoken, stopped: its tick once more,
 * the music back up.  `eax` as the code has it with nothing to stop. */
Result descriptionStopped(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 2 * 4;
    if (kDescriptionOn(app) == 0)
        return int32_t(eax);
    kDescriptionStopped.set(app, 1);
    if (!descriptionTicked(app, cpu, frame - 4, eax))
        return {};
    return musicTo(app, cpu, frame, gameSettings(app).menuMusicVolume);
}

/* 0x4c95f0's: a description spoken -- track `track` of the menus' (kind 0x32)
 * or of the cars' (0x34) in language `language`, or (0x33) part `track` of
 * the one loaded -- the music lowered to half for it. */
Result descriptionStarted(App* app, CPU& cpu, reg32 esp, reg32 track, reg32 kind, reg32 language)
{
    const reg32 frame = esp - 4 * 4;
    const reg32 name = frame - 0x104, sp = name;
    reg32 eax = track;
    if (kDescriptionOn(app) != 0)
    {
        const Result stopped = descriptionStopped(app, cpu, sp - 4, eax);
        if (!stopped)
            return {};
        eax = reg32(*stopped);
    }
    const auto musicDown = [&]() {
        const int32_t volume = gameSettings(app).menuMusicVolume;
        const Result r = musicTo(app, cpu, sp, volume / 2);
        if (r)
            kDescriptionKind.set(app, int32_t(kind));
        return r;
    };
    if (kind == 0x33)
    {
        if (kDescriptionLoaded(app) == 0)
            return int32_t(eax);
        if (!musicDown())
            return {};
        return descriptionSpoken(app, cpu, sp - 4, track);
    }
    if ((kind != 0x32 && kind != 0x34) || int32_t(track) < 0 || int32_t(track) > 0x31)
        return int32_t(eax);
    const Table<reg32>& names = kind == 0x32 ? kMenuTracks : kCarTracks;
    copyText(app, name, kCarsFolder);
    // 0x4529a0 (port_menus.cpp's, with no typed function yet) by its address: the name in eax.
    if (!callAt(app, cpu, sp, kPathEnd, {name}))
        return {};
    cpu.esp = sp;
    const std::optional<reg32> trackName = guest::direct<kTrackName, load::folderOf>(app, cpu, track);
    if (!trackName)
        return {};
    appendText(app, name, *trackName);
    if (!callAt(app, cpu, sp, kPathEnd, {name}))
        return {};
    appendText(app, name, load<reg32>(app, names.address(int32_t(language))));
    // The description bank loaded (0x4c9240, the speech's one register-level native, with no typed function: it
    // asks SND to add the bank) by its address, as the code loads the registers.
    const Result loaded = callAt(app, cpu, sp, kDescriptionLoad, {name, language, language, track});
    if (!loaded)
        return {};
    if (*loaded == 0)
        return 0;
    if (!musicDown())
        return {};
    return descriptionSpoken(app, cpu, sp - 4, 0);
}

/* 0x4c9860's: the system tasks run, then the description's tick. */
Result descriptionServed(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 4;
    // The tasks read ebp unset: the code's frame pointer.
    const HeldEbp ebp(cpu, frame);
    cpu.esp = frame;
    const Result tasks = guest::direct<kTimersRun, stream::runDueTasks>(app, cpu, frame - 4, 0u);
    if (!tasks)
        return {};
    return descriptionTicked(app, cpu, frame - 4, reg32(*tasks));
}

/* 0x4c9890's: the race voice bank `index` loaded -- from the units' folder
 * (one of five), "2" more in the split screen -- as bank 2 (the description's
 * loaded mark set) or 1. */
Result raceVoiceBankLoaded(App* app, CPU& cpu, reg32 esp, reg32 index)
{
    const reg32 frame = esp - 6 * 4;
    const reg32 name = frame - 0x100, sp = name;
    const reg32 units = reg32(gameSettings(app).language);
    if (units <= 5)
        copyText(app, name, kSpeechUnitFolders[units]);
    appendText(app, name, load<reg32>(app, kCarNames.address(int32_t(index))));
    if (gameSettings(app).playMode == 1)
        appendText(app, name, kSecondPlayerSuffix);
    const bool second = index != 0;
    cpu.esp = sp;
    const Result r = guest::direct<kLoadBank, bankLoaded>(app, cpu, sp - 4, name, second ? 2 : 1, 0u, name);
    if (!r)
        return {};
    if (second)
        kDescriptionLoaded.set(app, 1);
    return r;
}

/* 0x4c9990's: effect `group` * 16 + `index` of the car bank (index above 4) or the
 * scenery bank, spoken at full volume.  The sound's handle. */
Result carLineSpoken(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 group)
{
    const reg32 frame = esp - 3 * 4;
    const int32_t bank = bankHandle(app, int32_t(index) > 4 ? kSpeechBankCar : kSpeechBankScenery);
    cpu.esp = frame;
    const Result handle = guest::direct<kPlaySpeech, speechEffect>(app, cpu, frame - 8, bank,
                                                                   int32_t((group << 4) + index), 0x7f, 0u,
                                                                   uint8_t(0x40));
    if (!handle)
        return {};
    kSpokenSound.set(app, *handle);
    return handle;
}

/* 0x4c9a00's: the race voice banks removed and their header copies freed. */
Result raceVoiceBanksFreed(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 5 * 4;
    const HeldEbp ebp(cpu, frame);
    const auto removed = [&](int32_t bank) {
        return removeEveryBank(app, cpu, frame - 4, bankHandle(app, bank));
    };
    const auto freed = [&](Var<reg32> pointer) {
        const reg32 block = pointer(app);
        if (block != 0)
        {
            if (!blockFreed(app, cpu, frame, block))
                return false;
            pointer.set(app, 0);
        }
        return true;
    };
    if (!removed(kSpeechBankScenery) || !removed(kSpeechBankCar))
        return {};
    if (!freed(Var<reg32>{0x571138}))
        return {};
    if (kDescriptionLoaded(app) != 0)
    {
        if (!removed(kSpeechBankCar) || !freed(Var<reg32>{0x571144}))
            return {};
    }
    kDescriptionLoaded.set(app, 0);
    return 0;
}

/* The speech's entries -- each its thunk (the game's registers and stack in,
 * the result in eax out) around what it does. */

x86::reg32 speechInit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, speechSetUp(app, cpu, e.esp));
}

x86::reg32 speechFree(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, speechScriptFreed(app, cpu, e.esp, e.eax));
}

x86::reg32 speechLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, speechLineSaid(app, cpu, e.esp, e.eax));
}

/* 0x4c8d10: player eax's co-driver. */
x86::reg32 coDriverTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, coDriverTicked(app, cpu, e.esp, int32_t(e.eax)));
}

x86::reg32 descriptionSpeak(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, descriptionSpoken(app, cpu, e.esp, e.eax));
}

x86::reg32 descriptionTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, descriptionTicked(app, cpu, e.esp, e.eax));
}

x86::reg32 descriptionStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, descriptionStopped(app, cpu, e.esp, e.eax));
}

/* 0x4c95e0's, 0x4c99f0's: the spoken line's status, the game's code's frame
 * (a word pushed) below `esp`. */
Result spokenStatusOf(App* app, CPU& cpu, reg32 esp)
{
    return soundStatus(app, cpu, esp - 4 - 4, reg32(kSpokenSound(app)));
}

/* 0x4c95e0, 0x4c99f0: the spoken line's status. */
x86::reg32 spokenStatus(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, spokenStatusOf(app, cpu, e.esp));
}

x86::reg32 descriptionStart(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, descriptionStarted(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

x86::reg32 descriptionService(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, descriptionServed(app, cpu, e.esp));
}

x86::reg32 raceVoiceLoad(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceVoiceBankLoaded(app, cpu, e.esp, e.eax));
}

x86::reg32 carLineSpeak(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, carLineSpoken(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x4c99d0's: the spoken line faded out, the game's code's frame (3 words
 * pushed) below `esp`. */
Result spokenStopOf(App* app, CPU& cpu, reg32 esp)
{
    return fadeSound(app, cpu, esp - 3 * 4 - 4, reg32(kSpokenSound(app)), 0, -1);
}

/* 0x4c99d0: the spoken line faded out. */
x86::reg32 spokenStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, spokenStopOf(app, cpu, e.esp));
}

x86::reg32 raceVoiceFree(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceVoiceBanksFreed(app, cpu, e.esp));
}

x86::reg32 speechTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, playerSpeechTicked(app, cpu, e.esp, int32_t(e.eax)));
}

/* 0x4c8810 (stdcall 4): player [esp+4]'s radio sound. */
x86::reg32 radioTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             radioSoundTicked(app, cpu, e.esp, e.eax, e.edx, int32_t(e.ebx), int32_t(e.ecx),
                              int32_t(e.arg(app, 0))),
             4);
    return args.eax;
}

/* 0x4c89b0 (stdcall 4): player [esp+4]'s radio started. */
x86::reg32 radioStart(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             radioStarted(app, cpu, e.esp, e.eax, int32_t(e.ecx), int32_t(e.ebx), e.edx, int32_t(e.arg(app, 0))), 4);
    return args.eax;
}

x86::reg32 speechSaid(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, linesSaidMarked(app, cpu, e.esp));
}

x86::reg32 copRadio(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copRadioStarted(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx)));
}

x86::reg32 queueSpeechLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, speechLineQueued(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

x86::reg32 queueRaceLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceLineQueued(app, cpu, e.esp, e.eax, e.ebx, e.edx));
}

x86::reg32 raceEventLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, raceEventLineQueued(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 gapLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, gapLineQueued(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

x86::reg32 lapTimeLine(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, lapTimeLineQueued(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

}  // namespace port
}  // namespace nfs3hp

