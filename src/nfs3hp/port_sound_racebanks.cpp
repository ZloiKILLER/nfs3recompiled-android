#include "game_settings.h"
#include "port_snd_speech.h"

#include <cstdint>
#include <utility>

/* The race's banks loaded, rewritten (port.h says how): the players' cars'
 * (from the car's archive, or by its model the audio folder's file), the
 * race's own -- crashes, the cops' and their radio in the units' folder, the
 * track's ambience by its number (and the weather), the speech's, the
 * co-driver's -- the loading bar turned after each.  The names are built as
 * the code builds them, on its stack, with its inlined strcpy and strcat and
 * sprintf.
 *
 * Before the last car's bank the checks' stand-ins log ebx and ecx at the
 * track's name (they take four registers), which are whatever the code has in
 * them there -- the loads before leave them, and the names' strcat sets ecx --
 * so those two follow the code through its calls (cpu's).  What is not
 * rewritten yet is called as the game's code calls it. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr reg32 kLoadBank = 0x410b50, kCarBank = 0x4138e0, kLoadingBar = 0x4a33d0, kSprintf = 0x4df690;
constexpr reg32 kPathEnd = 0x4529a0, kTrackName = 0x4392b0;
constexpr reg32 kAudioDir = 0x7a2ab4, kCarsDir = 0x7a28b4;
constexpr reg32 kUnitNamesAt = 0x41296c;   // 7 words in the code: the units' names
constexpr Var<int32_t> kSurround{0x5525a4};
constexpr Var<uint8_t> kRaceFlags{0x55eb2e}, kRaceFlagsMore{0x55eb2f};
constexpr reg32 kRacerSize = 0x6c;  // the racers' records, the model first

/* The game's inlined strcpy of `from` to `to`: esi is left where the copy
 * stopped reading, which the callees that follow may see. */
void copyName(App* app, CPU& cpu, reg32 to, reg32 from)
{
    reg32 length = 0;
    while (load<uint8_t>(app, from + length) != 0)
        ++length;
    copyText(app, to, from);
    cpu.esi = from + length + (length & 1);
}

/* Its strcat: ecx is left past the end of the name it ran along (the length
 * and 2 under 0), then the copy. */
void appendName(App* app, CPU& cpu, reg32 to, reg32 from)
{
    reg32 length = 0;
    while (load<uint8_t>(app, to + length) != 0)
        ++length;
    cpu.ecx = reg32(-2 - int32_t(length));
    copyName(app, cpu, to + length, from);
}

/* The bank file's ending for car model `model` (as the menus number them). */
reg32 modelEnding(int32_t model)
{
    if (model == 0x25b)
        return 0x536aa0;
    if (model >= 0x258)
        return 0x536aa8;
    if (model == 0x3e || model == 0x35)
        return 0x536ab0;
    return 0x536ab8;
}

/* A racer's car model (in the 0..0x31 the archives have, else 0). */
int32_t modelOf(App* app, int32_t racer)
{
    const int32_t model = int32_t(gameSettings(app).entryAt(reg32(racer) * kRacerSize).car);
    return model < 0 || model > 0x31 ? 0 : model;
}

}  // namespace

/* 0x413bc0's: the race's banks loaded.  eax as the code leaves it: the
 * loading bar's, after the last. */
static Result raceBanksLoaded(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 6 * 4;
    // The registers the game's code pushes first lie in its frame, where the
    // units' names it reads past the seven run into them.
    store<reg32>(app, frame, cpu.ebp);
    store<reg32>(app, frame + 4, cpu.edi);
    store<reg32>(app, frame + 8, cpu.esi);
    store<reg32>(app, frame + 12, cpu.edx);
    store<reg32>(app, frame + 16, cpu.ecx);
    store<reg32>(app, frame + 20, cpu.ebx);
    // The code keeps ebp at a base in its frame, its locals at offsets from it.
    const reg32 b = frame - 0x9a;
    const HeldEbp ebp(cpu, b);
    const reg32 name = b - 0x4a, units = b + 0x1a, unitNames = b + 0x7e, sp = frame - 0xe4;
    reg32 r = 0;
    const auto game = [&](reg32 target, reg32 eax, reg32 edx, std::initializer_list<reg32> stack = {}) {
        return callGame(app, cpu, sp, target, Args{eax, edx, cpu.ebx, cpu.ecx}, stack, r);
    };
    const auto copy = [&](reg32 from) {
        copyName(app, cpu, name, from);
        cpu.edi = name;
    };
    const auto append = [&](reg32 from) {
        appendName(app, cpu, name, from);
        cpu.edi = name;
    };
    // The bank `name` names loaded into slot `slot`; the loading bar turned.
    const auto loadBank = [&](reg32 slot) {
        cpu.edi = name;
        cpu.ebx = 0;
        return game(kLoadBank, name, slot);
    };
    const auto bar = [&]() { return game(kLoadingBar, r, 0); };
    // The car of model `model` from its archive ("<cars><model name>\" and the bank's
    // file), loaded into slot `slot`.
    const auto carArchive = [&](int32_t model, reg32 slot) {
        copy(kCarsDir);
        if (!game(kPathEnd, name, reg32(model)))
            return false;
        cpu.edi = name;
        if (!game(kTrackName, reg32(model), reg32(model)))
            return false;
        cpu.esi = r;
        append(r);
        cpu.esi = 0x536ac0;
        cpu.edi = name;
        if (!game(kPathEnd, name, slot))
            return false;
        append(0x536ac0);
        return game(kCarBank, name, slot);
    };
    // The units' names, the folder of the units in use.
    for (reg32 i = 0; i < 7; ++i)
        store<reg32>(app, unitNames + 4 * i, load<reg32>(app, kUnitNamesAt + 4 * i));
    cpu.ecx = 0;
    cpu.esi = kUnitNamesAt + 28;
    cpu.edi = unitNames + 28;
    const reg32 unit = reg32(gameSettings(app).language);
    if (unit <= 5)
    {
        copyName(app, cpu, units, kSpeechUnitFolders[unit]);
        cpu.edi = units;
    }
    int32_t other;
    if (gameSettings(app).playMode == 1)
    {
        // The split screen: each player's car, into banks 0 and 0x16; the third's
        // (a cop's) next.
        for (const auto& [entry, slot] : {std::pair<reg32, reg32>{0, 0}, {1, 0x16}})
        {
            const int32_t model = int32_t(gameSettings(app).entries[entry].car);
            if (model < 0 || model > 0x31)
            {
                copy(kAudioDir);
                append(modelEnding(model));
                if (!loadBank(slot))
                    return {};
            }
            else if (!carArchive(model, slot))
                return {};
        }
        other = gameSettings(app).opponentCount != 0 ? modelOf(app, 3) : 0;
    }
    else
    {
        // The player's car: a cop's (the flags' bit 2) by its own number, else by
        // its model.
        const int32_t model = int32_t(gameSettings(app).entryAt(reg32(gameSettings(app).localPlayer) * kRacerSize).car);
        if ((std::uint8_t(gameSettings(app).raceFlags) & 2) != 0)
        {
            copy(kAudioDir);
            const reg32 cop = reg32(gameSettings(app).cheatTrack) + 0x32;
            cpu.ecx = cop;
            reg32 ending;
            if (model == 0x25b)
                ending = 0x536ac8;
            else if (model >= 0x258)
                ending = 0x536ad0;
            else if (cop == 0x3e || cop == 0x35 || gameSettings(app).cheatTrack == 0x15)
                ending = 0x536ad8;
            else
                ending = 0x536ae0;
            append(ending);
            if (!loadBank(0))
                return {};
        }
        else if (!carArchive(model, 0))
            return {};
        // The other car: the opponent's.
        other = modelOf(app, gameSettings(app).localPlayer == 0 ? 1 : 0);
    }
    // The other car's (or the cop's) bank, as player 1's.
    if (!carArchive(other, 1) || !bar())
        return {};
    if (gameSettings(app).raceType == 3)
    {
        // The pursuit: the cops' sounds and their radio.
        copy(kAudioDir);
        append(kSurround(app) != 0 ? 0x536ae8 : 0x536af0);
        cpu.esi = units;
        if (!loadBank(5) || !bar())
            return {};
        cpu.esi = units;
        copy(units);
        append(0x536af8);
        if (!loadBank(8) || !bar())
            return {};
    }
    // The crashes, the race's voices.
    copy(kAudioDir);
    append(kSurround(app) != 0 ? 0x536b00 : 0x536b08);
    cpu.esi = units;
    if (!loadBank(2) || !bar())
        return {};
    const reg32 voiceUnit = reg32(gameSettings(app).language);
    copy(units);
    append(0x536b0c);
    append(load<reg32>(app, unitNames + voiceUnit * 4));
    cpu.esi = units;
    if (!loadBank(6) || !bar())
        return {};
    copy(units);
    append(0x536b10);
    append(load<reg32>(app, unitNames + reg32(gameSettings(app).language) * 4));
    if (!loadBank(7) || !bar())
        return {};
    // The track's ambience: by its number, its weather and night.
    const int32_t track = gameSettings(app).track;
    if (track < 9)
    {
        if (!game(kSprintf, 0, 0, {name, kSurround(app) != 0 ? 0x536b14u : 0x536b20u, kAudioDir, reg32(track)}))
            return {};
        reg32 more = 0;
        if (gameSettings(app).weather != 0 && gameSettings(app).night != 0)
            more = 0x536b2c;
        else if (gameSettings(app).weather != 0)
            more = 0x536b30;
        else if (gameSettings(app).night != 0)
            more = 0x536b34;
        if (more != 0)
        {
            cpu.esi = more;
            append(more);
        }
    }
    else if (!game(kSprintf, 0, 0, {name, 0x536b38, kAudioDir, reg32(track)}))
        return {};
    cpu.esi = kAudioDir;
    if (!loadBank(3) || !bar())
        return {};
    copy(kAudioDir);
    append(0x536b44);
    if (!loadBank(4) || !bar())
        return {};
    if ((kRaceFlagsMore(app) & 1) != 0)
        return int32_t(r);
    // The speech of the track: its four banks, by the track's place in the four
    // (in the units' folder, or the audio one past 7).
    const auto trackSpeech = [&](reg32 format, reg32 slot, bool audioPast7) {
        const int32_t number = gameSettings(app).track;
        reg32 folder = audioPast7 ? kAudioDir : units, value = reg32(number);
        if (number <= 7)
        {
            folder = units;
            value = reg32(number % 4);
        }
        return game(kSprintf, 0, 0, {name, format, folder, value}) && loadBank(slot) && bar();
    };
    const uint8_t mode = kRaceFlags(app);
    if (((mode & 0x10) != 0 || (mode & 0x20) != 0) && gameSettings(app).raceType == 3)
    {
        if (!trackSpeech(0x536b4c, 0xb, true) || !trackSpeech(0x536b58, 0xc, true) || !trackSpeech(0x536b64, 0xd, true)
            || !trackSpeech(0x536b70, 0xe, true))
            return {};
        return int32_t(r);
    }
    cpu.esi = units;
    copy(units);
    append(0x536b7c);
    if (!loadBank(0xa) || !bar())
        return {};
    if (gameSettings(app).raceOptionCopRadio != 0)
    {
        // The co-driver's.
        copy(units);
        append(0x536b84);
        append(load<reg32>(app, unitNames + reg32(gameSettings(app).language) * 4));
        if (!loadBank(9) || !bar())
            return {};
    }
    if (gameSettings(app).raceType != 3)
        return int32_t(r);
    if (!trackSpeech(0x536b4c, 0xb, false) || !trackSpeech(0x536b58, 0xc, false) || !trackSpeech(0x536b64, 0xd, false)
        || !trackSpeech(0x536b70, 0xe, false))
        return {};
    return int32_t(r);
}

/* 0x413bc0: the race's banks loaded. */
void raceBanksLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, raceBanksLoaded(app, cpu, e.esp));
}

}  // namespace port
}  // namespace nfs3hp
