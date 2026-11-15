#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_watcom.h"
#include "port_snd_play.h"
#include "port_snd_stream_calls.h"
#include "port_sound_slice.h"
#include "port_sound_music.h"

#include <cstdint>

/* The game's music, rewritten (port.h says how): the CD stream it plays
 * through, the tune for the menus ("show") and for a race (the track's rock or
 * tech version) started on Pathfinder with its map, faded in and out, and the
 * effects' banks loaded and freed.  The port's change to the race music
 * (tools/apply_music_stream.py's, before these were native) is here: the
 * track is played from its own file, not from a copy made at every race.
 *
 * The game's state: 0x6fbb54 set when there is no sound (no music either),
 * 0x552910 the music on, 0x552598 the music's CD stream, 0x570a50 the
 * Pathfinder map, 0x5525a0 the music playing, 0x55258c the menu tune
 * playing, 0x5525fc the menu tune picked, 0x552590 the race's own tune,
 * 0x552914 whether the menus' music plays, 0x552854 tech (1) or rock,
 * 0x5525f8 the race's tune, 0x5525a8 the tunes' names, 0x7a2a34 the audio
 * directory, 0x7a2734 the temp directory, 0x6fbbe4 the track, 0x6fbbe8 the
 * music's volume, 0x6fbbec the menu music asked for.
 *
 * The functions call the game's streams and Pathfinder, which are not
 * rewritten yet, by their addresses, with the registers the game's code has
 * at each call (the stand-ins of tools/game_checks read the ones their
 * function takes, and a few take more than the code gives them). */
namespace nfs3hp
{
namespace port
{

namespace
{

// The game's state.
constexpr Var<int32_t> kEffectsOn{0x55290c}, kMusicOn{0x552910}, kMusicStream{0x552598};
constexpr Var<int32_t> kMusicMap{0x570a50}, kMusicPlaying{0x5525a0}, kMenuTune{0x55258c}, kMenuMusic{0x552914};
constexpr Var<int32_t> kTech{0x552854}, kRaceTune{0x5525f8};
constexpr Var<int32_t> kRaceStarted{0x55e88c};
constexpr Var<int32_t> kMenuSixAsked{0x552594};
constexpr Var<uint8_t> kMenuPick{0x5525fc}, kOwnTune{0x552590}, kMenuBack{0x552591};
constexpr Table<reg32> kTuneNames{0x5525a8};
constexpr Table<int32_t> kCarMusicShare{0x5525d0};

// A bank of the game's: its SND handle, then at +8 the memory its header's
// copy is in (0x571124 and 0x57112c).
#pragma pack(push, 1)
struct GameBank
{
    int32_t handle;
    int32_t other;
    reg32 copy;
};
#pragma pack(pop)
constexpr Records<GameBank> kGameBanks{0x571124};
constexpr int32_t kBankCopyCount = 0x17;

// The menus' tunes the code names.
constexpr int32_t kMenuTunePicked = 0x16, kMenuTuneSecond = 0x17, kMenuTuneSix = 0x18, kNoTune = 0x63;

// The ends of a tune's file name: rock and tech, the track's index, its stream.
constexpr reg32 kRockEnd = 0x5368d8, kTechEnd = 0x5368e0, kIndexOwn = 0x5368e8, kIndexMenu = 0x5368f0;
constexpr reg32 kStreamEnd = 0x5368f8;

/* The race's tune for the track the game is on: by its number (3 to 12),
 * at random of 5 for the others. */
int32_t raceTune(App* app)
{
    static const int32_t tunes[] = {0, 0, 1, 1, 2, 2, 3, 3, 8, 8};
    const uint32_t t = uint32_t(gameSettings(app).musicMode) - 3;
    return t <= 9 ? tunes[t] : int32_t(nextRandom(app) % 5);
}

/* Whether the race's music is the tech version: not while the game's mode
 * says rock; the track 1's always, else when the tech is to be (track 0
 * never). */
bool techMusic(App* app)
{
    const int32_t t = gameSettings(app).musicMode;
    return !rockMode(app) && (t == 1 || (t != 0 && kTech(app) == 1));
}

}  // namespace

/* 0x4109d0's: the music stopped, if it plays: faded out and waited for, the
 * stream purged, Pathfinder ended and its map freed.  eax as the code leaves
 * it: the last call's, else as it came. */
Result musicStopped(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    Game g(app, cpu, esp - 4 * 4);
    if (gameSettings(app).installIsRemote == 0 && kMusicPlaying(app) != 0 && kMusicOn(app) != 0)
    {
        // edx holds the flag just tested: 0.
        g.atCall();
        if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 0x64, 0), eax))
            return {};
        for (;;)
        {
            safepoint(app, cpu);
            if (!g.call(kStreamGetVolume, {eax}, eax))
                return {};
            if (int32_t(eax) <= 0)
                break;
            g.atCall();
            if (!tookEax(guest::direct<kRunSystemTasks, stream::runDueTasks>(app, cpu, g.esp - 4, 0u), eax))
                return {};
        }
        g.esi = 0;
        if (!g.call(kStreamPurge, {eax}, eax))
            return {};
        kMusicPlaying.set(app, 0);
        if (!g.call(kPathRestore, {eax}, eax))
            return {};
        const reg32 map = reg32(kMusicMap(app));
        g.edi = map;
        if (map != 0)
        {
            g.atCall();
            if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, map), eax))
                return {};
            kMusicMap.set(app, 0);
        }
    }
    return int32_t(eax);
}

/* 0x4104f0's: tune `tune` started at `volume` -- a menu's (0x16 the one
 * picked, 0x17 and 0x18 the sixth) or a race's (below 10: rock or tech, from
 * its .lin index when `own` says so, else its .map) -- on Pathfinder, faded
 * in.  eax as the code leaves it: the last call's, or on the ways out that
 * come first, what it holds there. */
Result musicStarted(App* app, CPU& cpu, reg32 esp, int32_t tune, int32_t volume, int32_t own)
{
    const reg32 frame = esp - 4 * 4 - 0x28a;  // ebp as the code has it
    const reg32 locals = esp - 4 * 4 - 0x20c;
    const reg32 name = locals, path = locals + 0x100;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, locals);
    if (tune == kNoTune)
    {
        tune = 0;
        g.esi = 0;
    }
    g.edi = kNoTune;
    kOwnTune.set(app, 0);
    reg32 eax = reg32(gameSettings(app).installIsRemote);
    kMenuTune.set(app, kNoTune);
    if (eax != 0 || kMusicOn(app) == 0 || tune > kMenuTuneSix)
        return int32_t(eax);
    if (own != 0 && tune < 0xa)
        kOwnTune.set(app, 1);
    if (own == 0)
        kMenuMusic.set(app, 1);
    if (gameSettings(app).menuMusicAsked == 0)
    {
        // A tune of the menus' 6 at random (the sixth is the second): the
        // code divides EA's random number by 7 and takes the rest.
        g.edi = 7;
        const reg32 random = nextRandom(app);
        eax = random / 7;
        uint8_t pick = uint8_t(random % 7 + 1);
        if (pick == 6)
            pick = 2;
        kMenuPick.set(app, pick);
    }
    else
    {
        eax = uint8_t(std::int32_t(gameSettings(app).menuMusicAsked));
        kMenuPick.set(app, uint8_t(std::int32_t(gameSettings(app).menuMusicAsked)));
    }
    // ecx for the calls below, as the code has it: the tune with the mode in
    // its second byte (6 for the sixth).
    reg32 ecx = (reg32(tune) & 0xffff00ff) | reg32(gameMode(app)) << 8;
    if (rockMode(app))
        kMenuPick.set(app, 1);
    if (tune == kMenuTuneSix)
    {
        ecx = 6;
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x5368c4, guest::Words{kAudioDir}), eax))
            return {};
        kMenuTune.set(app, 6);
    }
    else if (tune == kMenuTunePicked)
    {
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x5368cc, guest::Words{kAudioDir, kMenuPick(app)}), eax))
            return {};
        eax = kMenuPick(app);
        kMenuTune.set(app, int32_t(eax));
    }
    else if (tune == kMenuTuneSecond)
    {
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x5368c4, guest::Words{kAudioDir}), eax))
            return {};
    }
    else
    {
        if (tune >= 0xa)
            return int32_t(eax);
        const int32_t pick = std::int32_t(gameSettings(app).musicMode) >= 3 ? kRaceTune(app) : tune;
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, pick)}), eax))
            return {};
        ecx = appendText(app, name, !rockMode(app) && kTech(app) == 1 ? kTechEnd : kRockEnd);
        eax &= 0xffffff00;
    }
    const reg32 index = own != 0 && tune < 0xa ? kIndexOwn : kIndexMenu;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, path, kFormatTwo, guest::Words{name, index}), eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kLoadFile, ea::loadWhole>(app, cpu, path, 0x10), eax))
        return {};
    kMusicMap.set(app, int32_t(eax));
    if (!g.call(kPathInit, {eax}, eax))
        return {};
    constexpr reg32 kRing = 0x5dc;  // the stream's ring, in blocks
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, path, kFormatTwo, guest::Words{name, kStreamEnd}), eax))
        return {};
    // 0x4e73d0 (SNDmastervol) with the path in edx, which the stream start
    // below takes from it.
    if (!g.call(0x4e73d0, {0, path, kRing, ecx}, eax))
        return {};
    if (!g.call(kPathStartStream, {reg32(kMusicStream(app)), path, kRing, ecx}, eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 1, 0), eax))
        return {};
    if (!g.call(0x4e73d0, {0x7f, reg32(volume), kRing, ecx}, eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 0x64, volume), eax))
        return {};
    kMusicPlaying.set(app, 1);
    return int32_t(eax);
}

/* The music, as the game's code calls the functions of it. */

/* 0x40feb0's: SND ended at the game's exit, with the sound or the music on;
 * `in` the caller's registers, handed on. */
Result soundExitOf(App* app, CPU& cpu, reg32 esp, const Args& in)
{
    reg32 eax = in.eax;
    if (kEffectsOn(app) != 0 || kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 4);
        if (!g.call(kSoundRestore, {eax, in.edx, in.ebx, in.ecx}, eax))
            return {};
    }
    return int32_t(eax);
}

/* 0x40feb0: SND ended at the game's exit, with the sound or the music on. */
void soundExit(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, soundExitOf(app, cpu, e.esp, {e.eax, e.edx, e.ebx, e.ecx}));
}

/* 0x40fed0: the music's old copy (<temp>temp.mus) made writable and deleted. */
void deleteMusicCopy(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(3);
    const reg32 name = frame - 0x80;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, name);
    reg32 eax = e.eax;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536880, guest::Words{kTempDir}), eax))
        return;
    g.atCall();
    if (!tookEax(guest::direct<kFileExists, ea::fileThere>(app, cpu, name), eax))
        return;
    if (eax != 0)
    {
        // The import at 0x5345ec (SetFileAttributesA): the attributes of the file set to archive (0x20).
        g.sync();
        if (!callWindows<0x5345ec, &host::setFileAttributes>(app, cpu, g.esp, {eax}, eax, name, 0x20)
            || !g.call(kUnlink, {name}, eax))
            return;
    }
    e.back(cpu, eax);
}

/* 0x40ffd0: the race's tune for the track. */
void raceTuneOf(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(raceTune(app)));
}

/* 0x410030: the race's music picked for track eax -- its tune, rock or tech
 * -- before the race.  The game then copied the track to <temp>temp.mus,
 * turning the loading bar; the port plays the track where it lies, so that
 * part is not done (nor the buffer it copied through taken, nor the file's
 * name made up from the tune's, which only that copy used).  eax as the code
 * leaves it (the file name's making clears its low byte, and the second one
 * has the mode). */
void raceMusicPrepare(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(6) - 0x82;  // ebp as the code has it
    const reg32 name = frame - 0xaa;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, e.frame(6, 0x12c));
    const int32_t track = int32_t(e.eax);
    reg32 eax = e.eax;
    const int32_t quiet = gameSettings(app).installIsRemote;
    g.edi = reg32(quiet);
    if (quiet != 0 || track >= 0xa)
        return e.back(cpu, eax);
    if (!g.call(0x4a33d0, {eax, 0x80000, 0, e.eax}, eax))
        return;
    if (kRaceStarted(app) != 0)
        return e.back(cpu, eax);
    if (std::int32_t(gameSettings(app).musicMode) < 3)
    {
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, track)}), eax))
            return;
        eax = (eax & 0xffff0000) | reg32(gameMode(app)) << 8;
    }
    else
    {
        const int32_t tune = raceTune(app);
        kRaceTune.set(app, tune);
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, tune)}), eax))
            return;
        const int32_t t = gameSettings(app).musicMode;
        if (t == 0xb)
        {
            // Tech or rock at random, the track 11's.
            const bool tech = nextRandom(app) % 100 <= 0x32;
            kTech.set(app, tech ? 1 : 0);
            eax = 0;
        }
        else
        {
            kTech.set(app, (t & 1) != 0 ? 0 : 1);
            eax &= 0xffffff00;
        }
    }
    e.back(cpu, eax);
}

/* 0x410330's: the music's CD stream made (300 KB of ring, 16 KB blocks), when
 * the music is on; `ecx` the caller's, handed on.  eax as the code leaves it:
 * the stream, else as it came. */
Result musicStreamMade(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 3 * 4);
        if (!g.call(kStreamMake, {0x4b000, 0x4000, 0, ecx}, eax))
            return {};
        kMusicStream.set(app, int32_t(eax));
    }
    return int32_t(eax);
}

/* 0x410330: the music's CD stream made (300 KB of ring, 16 KB blocks), when
 * the music is on. */
void musicStreamMake(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicStreamMade(app, cpu, e.esp, e.eax, e.ecx));
}

/* 0x410370: the same, eax KB of ring. */
Result musicStreamOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 3 * 4);
        if (!g.call(kStreamMake, {eax << 10, 0x4000, 0, ecx}, eax))
            return std::nullopt;
        kMusicStream.set(app, int32_t(eax));
    }
    return int32_t(eax);
}

void musicStreamMakeOf(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicStreamOf(app, cpu, e.esp, e.eax, e.ecx));
}

/* 0x4103a0: the music after a change of setting eax (edx: the menus' tune
 * back): the menu's tune, or the race's for a track (3..12, rock or tech by
 * eax's evenness) -- restarted. */
void musicAfterSetting(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 esp = e.frame(4);
    const int32_t setting = int32_t(e.eax);
    // The menu's tune started again.
    const auto menu = [&]() -> Result {
        const Result stopped = musicStopped(app, cpu, esp - 4, e.eax);
        if (!stopped)
            return stopped;
        const Result started = musicStarted(app, cpu, esp - 4, kMenuTunePicked, std::int32_t(gameSettings(app).menuMusicVolume), 0);
        if (started)
            kMenuBack.set(app, 1);
        return started;
    };
    if (e.edx != 0)
        return e.finish(cpu, menu());
    const int32_t track = gameSettings(app).musicMode;
    if (track < 3 || track > 0xc)
    {
        if (kMenuBack(app) != 0)
        {
            kMenuBack.set(app, 1);
            return e.back(cpu, e.eax);
        }
        return e.finish(cpu, menu());
    }
    kTech.set(app, setting % 2 != 0 ? 0 : 1);
    const int32_t tune = raceTune(app);
    kRaceTune.set(app, tune);
    Result r = musicStopped(app, cpu, esp - 4, reg32(tune));
    if (!r)
        return;
    r = musicStarted(app, cpu, esp - 4, 0, std::int32_t(gameSettings(app).raceMusicVolume), 1);
    if (!r)
        return;
    kMenuBack.set(app, 0);
    e.finish(cpu, r);
}

/* 0x410460's: the menus' tune 0x18 while there is no other asked for (6),
 * the game's code's frame (4 words pushed) below `esp`.  eax as the code
 * leaves it: the last call's, else as it came. */
Result musicMenuSixOf(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 4 * 4;
    const HeldEbp ebp(cpu, frame);
    kMenuSixAsked.set(app, 1);
    Result r = int32_t(eax);
    if (gameSettings(app).menuMusicAsked != 6 && kMenuTune(app) != 6)
    {
        if (!(r = musicStopped(app, cpu, frame - 4, eax)))
            return {};
        if (!(r = musicStarted(app, cpu, frame - 4, kMenuTuneSix, std::int32_t(gameSettings(app).menuMusicVolume), 0)))
            return {};
    }
    return r;
}

/* 0x410460: the menus' tune 0x18 while there is no other asked for (6). */
void musicMenuSix(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicMenuSixOf(app, cpu, e.esp, e.eax));
}

/* 0x4104f0: tune eax started at volume edx (ebx: its own). */
void musicStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicStarted(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x4107c0: a race's music started -- tune eax (the race's when ebx says
 * so), rock or tech, at volume edx less the car's own share for the tech --
 * from the track's own file, by the port's change. */
void raceMusicStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(4) - 0x186;  // ebp as the code has it
    const reg32 locals = e.frame(4, 0x108);
    const reg32 name = locals;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, locals);
    int32_t tune = int32_t(e.eax);
    const reg32 own = e.ebx, volume = e.edx;
    constexpr reg32 kRing = 0x5dc;
    if (tune == kNoTune)
        tune = 0;
    kOwnTune.set(app, 0);
    const int32_t quiet = gameSettings(app).installIsRemote;
    if (quiet != 0 || kMusicOn(app) == 0 || tune >= 0xa)
        return e.back(cpu, reg32(tune));
    reg32 eax;
    reg32 pick, ecx;
    if (own != 0)
    {
        kMenuMusic.set(app, quiet);
        kOwnTune.set(app, 1);
        pick = reg32(kRaceTune(app));
        ecx = (kTuneNames(app, int32_t(pick)) & 0xffffff00) | gameMode(app);
        eax = pick;
    }
    else
    {
        kMenuMusic.set(app, 1);
        pick = reg32(tune);
        g.esi = kTuneNames(app, tune);
        ecx = gameMode(app);
        eax = reg32(tune);
    }
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, int32_t(pick))}), eax))
        return;
    if (!rockMode(app))
        g.edi = reg32(gameSettings(app).musicMode);
    appendText(app, name, techMusic(app) ? kTechEnd : kRockEnd);
    ecx = appendText(app, name, own != 0 ? kIndexOwn : kIndexMenu);
    musicIndexOpened(app, name);
    g.atCall();
    if (!tookEax(guest::direct<kLoadFile, ea::loadWhole>(app, cpu, name, 0x10), eax))
        return;
    kMusicMap.set(app, int32_t(eax));
    if (!g.call(kPathInit, {eax}, eax))
        return;
    g.esi = kStreamEnd;
    g.edi = name;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536900, guest::Words{kTempDir}), eax))
        return;
    ecx = appendText(app, name, kStreamEnd);
    musicStreamFile(app, name);
    if (!g.call(kPathStartStream, {reg32(kMusicStream(app)), name, kRing, ecx}, eax))
        return;
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 1, 0), eax))
        return;
    reg32 level = volume;
    if (gameSettings(app).musicMode == 1 || kTech(app) == 1)
    {
        // The tech's music less the car's own share of it.
        level = volume - reg32(kCarMusicShare(app, std::int32_t(gameSettings(app).track)));
        if (int32_t(level) < 0)
            level = 0;
    }
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 0x64, int32_t(level)),
                 eax))
        return;
    kMusicPlaying.set(app, 1);
    e.back(cpu, eax);
}

/* 0x4109d0: the music stopped. */
void musicStop(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicStopped(app, cpu, e.esp, e.eax));
}

/* 0x410aa0's: the music's stream ended: the streamer's restore, the stream
 * deleted; `ebx` and `ecx` the caller's, handed on.  eax as the code leaves
 * it: the last call's, else as it came. */
Result musicStreamEndOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ebx, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 2 * 4);
        if (!g.call(kStreamRestore, {eax}, eax) || !g.call(kStreamDelete, {reg32(kMusicStream(app)), 0, ebx, ecx}, eax))
            return {};
        // The stream is gone: edx holds the flag just tested: 0.
        kMusicStream.set(app, 0);
    }
    return int32_t(eax);
}

/* 0x410aa0: the music's stream ended: the streamer's restore, the stream
 * deleted. */
void musicStreamEnd(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, musicStreamEndOf(app, cpu, e.esp, e.eax, e.ebx, e.ecx));
}

/* 0x410ad0's: the 23 banks' header copies freed.  eax as the loop leaves it:
 * the last bank's free, or its place in the table. */
Result banksFreed(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 5 * 4;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, frame);
    reg32 eax = 0;
    for (int32_t i = 0; i < kBankCopyCount; ++i)
    {
        eax = reg32(i) * 3;
        GameBank& bank = kGameBanks(app, i);
        const reg32 copy = bank.copy;
        if (copy == 0)
            continue;
        g.esi = 0;
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, copy), eax))
            return {};
        bank.copy = 0;
    }
    return int32_t(eax);
}

void banksFree(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, banksFreed(app, cpu, e.esp));
}

/* 0x410b20's: every bank removed from SND, the first's copy freed.  eax as
 * the code leaves it: the last call's. */
Result banksRemoveOf(App* app, CPU& cpu, reg32 esp)
{
    Game g(app, cpu, esp - 3 * 4);
    reg32 eax;
    g.atCall();
    if (!tookEax(guest::direct<0x4e8c1c, removeEveryBank>(app, cpu, g.esp - 4, -1), eax))
        return {};
    GameBank& first = kGameBanks(app, 0);
    if (first.copy != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, first.copy), eax))
            return {};
        first.copy = 0;
    }
    return int32_t(eax);
}

/* 0x410b20: every bank removed from SND, the first's copy freed. */
void banksRemove(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, banksRemoveOf(app, cpu, e.esp));
}

/* 0x410b50's: bank file `file` (".bnk" added) loaded into slot `slot` and
 * added to SND (`flags` for the load): kept whole, or (SND asks for it, 7)
 * its header copied out and the file freed.  Its handle, as the code leaves
 * it in eax; with the effects off, the eax it came with. */
Result bankLoaded(App* app, CPU& cpu, reg32 esp, reg32 file, int32_t slot, reg32 flags, reg32 eax)
{
    const reg32 frame = esp - 4 * 4;  // ebp as the code has it
    const reg32 buffer = frame - 0x108;
    const reg32 handleAt = frame - 8;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, buffer);
    if (kEffectsOn(app) == 0)
        return int32_t(eax);
    copyText(app, buffer, file);
    appendText(app, buffer, 0x536908);
    g.atCall();
    if (!tookEax(guest::direct<kLoadFile, ea::loadWhole>(app, cpu, buffer, flags), eax))
        return {};
    const reg32 data = eax;
    g.edi = data;
    if (!g.call(0x4e8d20, {handleAt, data, flags, data}, eax))
        return {};
    reg32 kept = data;
    if (eax == 7)
    {
        const reg32 handle = load<reg32>(app, handleAt);
        if (!g.call(0x4e8db0, {handle, data, flags, data}, eax))
            return {};
        const reg32 size = eax;
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, 0x53688c, size, flags), eax))
            return {};
        const reg32 copy = eax;
        if (!g.call(0x4e8e20, {copy, handle, copy, data}, eax))
            return {};
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, data), eax))
            return {};
        kept = copy;
        g.edi = copy;
    }
    GameBank& bank = kGameBanks(app, slot);
    bank.copy = kept;
    const reg32 handle = load<reg32>(app, handleAt);
    bank.handle = int32_t(handle);
    return int32_t(handle);
}

void bankLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, bankLoaded(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
