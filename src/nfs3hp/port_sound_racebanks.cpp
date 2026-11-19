#include "game_settings.h"
#include "native_frontend_loada.h"
#include "native_watcom.h"
#include "port_snd_speech.h"
#include "port_sound_calls.h"

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
 * At the track's name, before the last car's bank, ebx and ecx are whatever the
 * code has in them -- the loads before leave them, the names' strcat sets ecx, a
 * bank's load ebx -- so those two follow the code (`ebx`, `ecx`, from ebx as
 * the code is entered) into what has no typed function yet, called by its
 * address as the game's code calls it.  The banks' loads and the sprintf are
 * called as C++. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr u32 kLoadBank = 0x410b50, kCarBank = 0x4138e0, kSprintf = 0x4df690;
constexpr u32 kPathEnd = 0x4529a0, kTrackName = 0x4392b0;
constexpr u32 kAudioDir = 0x7a2ab4, kCarsDir = 0x7a28b4;
constexpr u32 kUnitNamesAt = 0x41296c;   // 7 words in the code: the units' names
constexpr Var<int32_t> kSurround{0x5525a4};
constexpr Var<uint8_t> kRaceFlags{0x55eb2e}, kRaceFlagsMore{0x55eb2f};
constexpr u32 kRacerSize = 0x6c;  // the racers' records, the model first

/* A typed core's answer as the code has it in eax: false when the game stopped. */
bool took(const Result& result, u32& eax)
{
    if (!result)
        return false;
    eax = u32(*result);
    return true;
}

/* The game's inlined strcat: what it leaves in ecx, past the end of the name it ran along
 * (the length and 2 under 0), then the copy. */
u32 appendName(App* app, u32 to, u32 from)
{
    u32 length = 0;
    while (load<uint8_t>(app, to + length) != 0)
        ++length;
    copyText(app, to + length, from);
    return u32(-2 - int32_t(length));
}

/* The bank file's ending for car model `model` (as the menus number them). */
u32 modelEnding(int32_t model)
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
    const int32_t model = int32_t(gameSettings(app).entryAt(u32(racer) * kRacerSize).car);
    return model < 0 || model > 0x31 ? 0 : model;
}

}  // namespace

/* 0x413bc0's: the race's banks loaded; `ebx` as the code is entered.  eax as
 * the code leaves it: the loading bar's, after the last. */
static Result raceBanksLoaded(App* app, ThreadState& ts, u32 esp, u32 ebx)
{
    // The six registers the game's code pushes first lie in its frame (where the units' names it reads past the
    // seven run into them): its depth kept, their values not written.
    const u32 frame = esp - 6 * 4;
    // The code keeps ebp at a base in its frame, its locals at offsets from it.
    const u32 b = frame - 0x9a;
    const u32 name = b - 0x4a, units = b + 0x1a, unitNames = b + 0x7e, sp = frame - 0xe4;
    u32 r = 0, ecx = 0;
    // What has no typed function yet, by its address: ebx and ecx as the code has them.
    const auto game = [&](u32 target, u32 eax, u32 edx, std::initializer_list<u32> stack = {}) {
        return pointerGame(app, ts, sp, target, Args{eax, edx, ebx, ecx}, stack, r);
    };
    const auto copy = [&](u32 from) { copyText(app, name, from); };
    const auto append = [&](u32 from) { ecx = appendName(app, name, from); };
    // The bank `name` names loaded into slot `slot`; the loading bar turned.
    const auto loadBank = [&](u32 slot) {
        ebx = 0;
        ts.sp = sp;
        return took(guest::direct<kLoadBank, bankLoaded>(app, ts, sp - 4, name, int32_t(slot), 0u, name), r);
    };
    // The loading bar turned (0x4a33d0): its eax into r, every other register kept.
    const auto bar = [&]() {
        ts.sp = sp;
        r = guest::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
        return !ts.terminate;
    };
    const auto sprintf = [&](u32 format, u32 folder, u32 value) {
        ts.sp = sp;
        r = guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, format, guest::Words{folder, value});
        return !ts.terminate;
    };
    // The car of model `model` from its archive ("<cars><model name>\" and the bank's
    // file), loaded into slot `slot`.
    const auto carArchive = [&](int32_t model, u32 slot) {
        copy(kCarsDir);
        if (!game(kPathEnd, name, u32(model)))
            return false;
        if (!game(kTrackName, u32(model), u32(model)))
            return false;
        append(r);
        if (!game(kPathEnd, name, slot))
            return false;
        append(0x536ac0);
        ts.sp = sp;
        return took(guest::direct<kCarBank, carBankLoaded>(app, ts, sp - 4, name, int32_t(slot)), r);
    };
    // The units' names, the folder of the units in use.
    for (u32 i = 0; i < 7; ++i)
        store<u32>(app, unitNames + 4 * i, load<u32>(app, kUnitNamesAt + 4 * i));
    const u32 unit = u32(gameSettings(app).language);
    if (unit <= 5)
        copyText(app, units, kSpeechUnitFolders[unit]);
    int32_t other;
    if (gameSettings(app).playMode == 1)
    {
        // The split screen: each player's car, into banks 0 and 0x16; the third's
        // (a cop's) next.
        for (const auto& [entry, slot] : {std::pair<u32, u32>{0, 0}, {1, 0x16}})
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
        const int32_t model = int32_t(gameSettings(app).entryAt(u32(gameSettings(app).localPlayer) * kRacerSize).car);
        if ((std::uint8_t(gameSettings(app).raceFlags) & 2) != 0)
        {
            copy(kAudioDir);
            const u32 cop = u32(gameSettings(app).cheatTrack) + 0x32;
            ecx = cop;
            u32 ending;
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
        if (!loadBank(5) || !bar())
            return {};
        copy(units);
        append(0x536af8);
        if (!loadBank(8) || !bar())
            return {};
    }
    // The crashes, the race's voices.
    copy(kAudioDir);
    append(kSurround(app) != 0 ? 0x536b00 : 0x536b08);
    if (!loadBank(2) || !bar())
        return {};
    const u32 voiceUnit = u32(gameSettings(app).language);
    copy(units);
    append(0x536b0c);
    append(load<u32>(app, unitNames + voiceUnit * 4));
    if (!loadBank(6) || !bar())
        return {};
    copy(units);
    append(0x536b10);
    append(load<u32>(app, unitNames + u32(gameSettings(app).language) * 4));
    if (!loadBank(7) || !bar())
        return {};
    // The track's ambience: by its number, its weather and night.
    const int32_t track = gameSettings(app).track;
    if (track < 9)
    {
        if (!sprintf(kSurround(app) != 0 ? 0x536b14u : 0x536b20u, kAudioDir, u32(track)))
            return {};
        u32 more = 0;
        if (gameSettings(app).weather != 0 && gameSettings(app).night != 0)
            more = 0x536b2c;
        else if (gameSettings(app).weather != 0)
            more = 0x536b30;
        else if (gameSettings(app).night != 0)
            more = 0x536b34;
        if (more != 0)
            append(more);
    }
    else if (!sprintf(0x536b38, kAudioDir, u32(track)))
        return {};
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
    const auto trackSpeech = [&](u32 format, u32 slot, bool audioPast7) {
        const int32_t number = gameSettings(app).track;
        u32 folder = audioPast7 ? kAudioDir : units, value = u32(number);
        if (number <= 7)
        {
            folder = units;
            value = u32(number % 4);
        }
        return sprintf(format, folder, value) && loadBank(slot) && bar();
    };
    const uint8_t mode = kRaceFlags(app);
    if (((mode & 0x10) != 0 || (mode & 0x20) != 0) && gameSettings(app).raceType == 3)
    {
        if (!trackSpeech(0x536b4c, 0xb, true) || !trackSpeech(0x536b58, 0xc, true) || !trackSpeech(0x536b64, 0xd, true)
            || !trackSpeech(0x536b70, 0xe, true))
            return {};
        return int32_t(r);
    }
    copy(units);
    append(0x536b7c);
    if (!loadBank(0xa) || !bar())
        return {};
    if (gameSettings(app).raceOptionCopRadio != 0)
    {
        // The co-driver's.
        copy(units);
        append(0x536b84);
        append(load<u32>(app, unitNames + u32(gameSettings(app).language) * 4));
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
nfs::u32 raceBanksLoad(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, raceBanksLoaded(app, ts, e.esp, e.ebx));
}

}  // namespace port
}  // namespace nfs3hp
