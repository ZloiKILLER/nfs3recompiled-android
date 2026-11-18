#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_loada.h"
#include "native_watcom.h"
#include "port_snd_calls.h"
#include "port_snd_play.h"
#include "port_snd_stream_calls.h"
#include "port_sound_slice.h"
#include "port_sound_music.h"

#include <cstdint>

/* The game's music, rewritten (port.h says how): the CD stream it plays
 * through, the tune for the menus ("show") and for a race (the track's rock or
 * tech version) started on Pathfinder with its map, faded in and out, and the
 * effects' banks loaded and freed.  The port's change to the race music
 * (the port's music stream change's, before these were native) is here: the
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
 * at each call. */
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
            g.atCall();
            if (!tookEax(guest::direct<kStreamGetVolume, stream::streamVolume>(app, cpu, g.esp - 4), eax))
                return {};
            if (int32_t(eax) <= 0)
                break;
            g.atCall();
            if (!tookEax(guest::direct<kRunSystemTasks, stream::runDueTasks>(app, cpu, g.esp - 4, 0u), eax))
                return {};
        }
        g.atCall();
        if (!tookEax(guest::direct<kStreamPurge, stream::purgeStreamer>(app, cpu, g.esp - 4), eax))
            return {};
        kMusicPlaying.set(app, 0);
        g.atCall();
        if (!tookEax(guest::direct<kPathRestore, stream::pathRestore>(app, cpu, g.esp - 4), eax))
            return {};
        const reg32 map = reg32(kMusicMap(app));
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
    }
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
    if (rockMode(app))
        kMenuPick.set(app, 1);
    if (tune == kMenuTuneSix)
    {
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
        appendText(app, name, !rockMode(app) && kTech(app) == 1 ? kTechEnd : kRockEnd);
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
    g.atCall();
    if (!tookEax(guest::direct<kPathInit, stream::pathInit>(app, cpu, g.esp - 4, eax), eax))
        return {};
    constexpr reg32 kRing = 0x5dc;  // the stream's ring, in blocks
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, path, kFormatTwo, guest::Words{name, kStreamEnd}), eax))
        return {};
    // 0x4e73d0 (SNDmastervol) with the path in edx, which the stream start
    // below takes from it.
    g.atCall();
    if (!tookEax(guest::direct<0x4e73d0, setMasterVolume>(app, cpu, g.esp - 4, 0), eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kPathStartStream, stream::startPathStream>(app, cpu, g.esp - 4,
                                                                          reg32(kMusicStream(app)), path, kRing),
                 eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 1, 0), eax))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<0x4e73d0, setMasterVolume>(app, cpu, g.esp - 4, 0x7f), eax))
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
        g.atCall();
        if (!tookEax(guest::direct<kSoundRestore, endLibrary>(app, cpu, g.esp - 4, eax), eax))
            return {};
    }
    return int32_t(eax);
}

/* 0x40feb0: SND ended at the game's exit, with the sound or the music on. */
x86::reg32 soundExit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, soundExitOf(app, cpu, e.esp, {e.eax, e.edx, e.ebx, e.ecx}));
}

/* 0x40fed0: the music's old copy (<temp>temp.mus) made writable and deleted. */
Result musicCopyDeleted(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 4 * 3;
    const reg32 name = frame - 0x80;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, name);
    reg32 eax = 0;  // what the code has in eax at the call: put over by the sprintf's result before it is read
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536880, guest::Words{kTempDir}), eax))
        return std::nullopt;
    g.atCall();
    if (!tookEax(guest::direct<kFileExists, ea::fileThere>(app, cpu, name), eax))
        return std::nullopt;
    if (eax != 0)
    {
        // The import at 0x5345ec (SetFileAttributesA): the attributes of the file set to archive (0x20).
        if (!callWindowsWords<0x5345ec, &host::setFileAttributes>(app, cpu, g.esp, eax, name, 0x20)
            || !g.call(kUnlink, {name}, eax))
            return std::nullopt;
    }
    return std::int32_t(eax);
}

x86::reg32 deleteMusicCopy(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicCopyDeleted(app, cpu, e.esp));
}

/* 0x40ffd0: the race's tune for the track. */
x86::reg32 raceTuneOf(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(raceTune(app)));
}

/* 0x410030: the race's music picked for track eax -- its tune, rock or tech
 * -- before the race.  The game then copied the track to <temp>temp.mus,
 * turning the loading bar; the port plays the track where it lies, so that
 * part is not done (nor the buffer it copied through taken, nor the file's
 * name made up from the tune's, which only that copy used).  eax as the code
 * leaves it (the file name's making clears its low byte, and the second one
 * has the mode). */
x86::reg32 raceMusicPrepare(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    const reg32 frame = e.frame(6) - 0x82;  // ebp as the code has it
    const reg32 name = frame - 0xaa;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, e.frame(6, 0x12c));
    const int32_t track = int32_t(e.eax);
    reg32 eax = e.eax;
    const int32_t quiet = gameSettings(app).installIsRemote;
    if (quiet != 0 || track >= 0xa)
        return e.back(cpu, args, eax);
    g.atCall();
    eax = guest::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    if (cpu.terminate)
        return args.eax;
    if (kRaceStarted(app) != 0)
        return e.back(cpu, args, eax);
    if (std::int32_t(gameSettings(app).musicMode) < 3)
    {
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, track)}), eax))
            return args.eax;
        eax = (eax & 0xffff0000) | reg32(gameMode(app)) << 8;
    }
    else
    {
        const int32_t tune = raceTune(app);
        kRaceTune.set(app, tune);
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, tune)}), eax))
            return args.eax;
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
    return e.back(cpu, args, eax);
}

/* 0x410330's: the music's CD stream made (300 KB of ring, 16 KB blocks), when
 * the music is on; `ecx` the caller's, handed on.  eax as the code leaves it:
 * the stream, else as it came. */
Result musicStreamMade(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 3 * 4);
        g.atCall();
        if (!tookEax(guest::direct<kStreamMake, stream::makeFirstStream>(app, cpu, g.esp - 4, 0x4b000u, 0x4000u, 0u), eax))
            return {};
        kMusicStream.set(app, int32_t(eax));
    }
    return int32_t(eax);
}

/* 0x410330: the music's CD stream made (300 KB of ring, 16 KB blocks), when
 * the music is on. */
x86::reg32 musicStreamMake(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicStreamMade(app, cpu, e.esp, e.eax, e.ecx));
}

/* 0x410370: the same, eax KB of ring. */
Result musicStreamOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 3 * 4);
        g.atCall();
        if (!tookEax(guest::direct<kStreamMake, stream::makeFirstStream>(app, cpu, g.esp - 4, eax << 10, 0x4000u, 0u), eax))
            return std::nullopt;
        kMusicStream.set(app, int32_t(eax));
    }
    return int32_t(eax);
}

x86::reg32 musicStreamMakeOf(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicStreamOf(app, cpu, e.esp, e.eax, e.ecx));
}

/* 0x4103a0: the music after a change of setting eax (edx: the menus' tune
 * back): the menu's tune, or the race's for a track (3..12, rock or tech by
 * eax's evenness) -- restarted. */
x86::reg32 musicAfterSetting(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
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
        return e.finish(cpu, args, menu());
    const int32_t track = gameSettings(app).musicMode;
    if (track < 3 || track > 0xc)
    {
        if (kMenuBack(app) != 0)
        {
            kMenuBack.set(app, 1);
            return e.back(cpu, args, e.eax);
        }
        return e.finish(cpu, args, menu());
    }
    kTech.set(app, setting % 2 != 0 ? 0 : 1);
    const int32_t tune = raceTune(app);
    kRaceTune.set(app, tune);
    Result r = musicStopped(app, cpu, esp - 4, reg32(tune));
    if (!r)
        return args.eax;
    r = musicStarted(app, cpu, esp - 4, 0, std::int32_t(gameSettings(app).raceMusicVolume), 1);
    if (!r)
        return args.eax;
    kMenuBack.set(app, 0);
    return e.finish(cpu, args, r);
}

/* 0x410460's: the menus' tune 0x18 while there is no other asked for (6),
 * the game's code's frame (4 words pushed) below `esp`.  eax as the code
 * leaves it: the last call's, else as it came. */
Result musicMenuSixOf(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 4 * 4;
    // The tasks musicStopped runs read ebp unset: the code's frame pointer.
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
x86::reg32 musicMenuSix(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicMenuSixOf(app, cpu, e.esp, e.eax));
}

/* 0x4104f0: tune eax started at volume edx (ebx: its own). */
x86::reg32 musicStart(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicStarted(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx)));
}

/* 0x4107c0: a race's music started -- tune eax (the race's when ebx says
 * so), rock or tech, at volume edx less the car's own share for the tech --
 * from the track's own file, by the port's change. */
x86::reg32 raceMusicStart(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
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
        return e.back(cpu, args, reg32(tune));
    reg32 eax;
    reg32 pick;
    if (own != 0)
    {
        kMenuMusic.set(app, quiet);
        kOwnTune.set(app, 1);
        pick = reg32(kRaceTune(app));
        eax = pick;
    }
    else
    {
        kMenuMusic.set(app, 1);
        pick = reg32(tune);
        eax = reg32(tune);
    }
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kFormatTwo, guest::Words{kAudioDir, kTuneNames(app, int32_t(pick))}), eax))
        return args.eax;
    appendText(app, name, techMusic(app) ? kTechEnd : kRockEnd);
    appendText(app, name, own != 0 ? kIndexOwn : kIndexMenu);
    musicIndexOpened(app, name);
    g.atCall();
    if (!tookEax(guest::direct<kLoadFile, ea::loadWhole>(app, cpu, name, 0x10), eax))
        return args.eax;
    kMusicMap.set(app, int32_t(eax));
    g.atCall();
    if (!tookEax(guest::direct<kPathInit, stream::pathInit>(app, cpu, g.esp - 4, eax), eax))
        return args.eax;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536900, guest::Words{kTempDir}), eax))
        return args.eax;
    appendText(app, name, kStreamEnd);
    musicStreamFile(app, name);
    g.atCall();
    if (!tookEax(guest::direct<kPathStartStream, stream::startPathStream>(app, cpu, g.esp - 4,
                                                                          reg32(kMusicStream(app)), name, kRing),
                 eax))
        return args.eax;
    g.atCall();
    if (!tookEax(guest::direct<kStreamAutoVolume, stream::fadeStream>(app, cpu, g.esp - 4, 1, 0), eax))
        return args.eax;
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
        return args.eax;
    kMusicPlaying.set(app, 1);
    return e.back(cpu, args, eax);
}

/* 0x4109d0: the music stopped. */
x86::reg32 musicStop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicStopped(app, cpu, e.esp, e.eax));
}

/* 0x410aa0's: the music's stream ended: the streamer's restore, the stream
 * deleted; `ebx` and `ecx` the caller's, handed on.  eax as the code leaves
 * it: the last call's, else as it came. */
Result musicStreamEndOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ebx, reg32 ecx)
{
    if (gameSettings(app).installIsRemote == 0 && kMusicOn(app) != 0)
    {
        Game g(app, cpu, esp - 2 * 4);
        g.atCall();
        if (!tookEax(guest::direct<kStreamRestore, stream::restoreStreamer>(app, cpu, g.esp - 4), eax))
            return {};
        g.atCall();
        if (!tookEax(guest::direct<kStreamDelete, stream::deleteCdStream>(app, cpu, g.esp - 4, reg32(kMusicStream(app)),
                                                                          0u, ebx, ecx),
                     eax))
            return {};
        // The stream is gone: edx holds the flag just tested: 0.
        kMusicStream.set(app, 0);
    }
    return int32_t(eax);
}

/* 0x410aa0: the music's stream ended: the streamer's restore, the stream
 * deleted. */
x86::reg32 musicStreamEnd(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, musicStreamEndOf(app, cpu, e.esp, e.eax, e.ebx, e.ecx));
}

/* 0x410ad0's: the 23 banks' header copies freed.  eax as the loop leaves it:
 * the last bank's free, or its place in the table. */
Result banksFreed(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 5 * 4;
    Game g(app, cpu, frame);
    reg32 eax = 0;
    for (int32_t i = 0; i < kBankCopyCount; ++i)
    {
        eax = reg32(i) * 3;
        GameBank& bank = kGameBanks(app, i);
        const reg32 copy = bank.copy;
        if (copy == 0)
            continue;
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, copy), eax))
            return {};
        bank.copy = 0;
    }
    return int32_t(eax);
}

x86::reg32 banksFree(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, banksFreed(app, cpu, e.esp));
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
x86::reg32 banksRemove(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, banksRemoveOf(app, cpu, e.esp));
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
    Game g(app, cpu, buffer);
    if (kEffectsOn(app) == 0)
        return int32_t(eax);
    copyText(app, buffer, file);
    appendText(app, buffer, 0x536908);
    g.atCall();
    if (!tookEax(guest::direct<kLoadFile, ea::loadWhole>(app, cpu, buffer, flags), eax))
        return {};
    const reg32 data = eax;
    g.atCall();
    if (!tookEax(guest::direct<0x4e8d20, bankAdded>(app, cpu, g.esp - 4, handleAt, data), eax))
        return {};
    reg32 kept = data;
    if (eax == 7)
    {
        const reg32 handle = load<reg32>(app, handleAt);
        g.atCall();
        if (!tookEax(guest::direct<0x4e8db0, bankHeaderSize>(app, cpu, g.esp - 4, int32_t(handle)), eax))
            return {};
        const reg32 size = eax;
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, 0x53688c, size, flags), eax))
            return {};
        const reg32 copy = eax;
        g.atCall();
        if (!tookEax(guest::direct<0x4e8e20, copyBankHeader>(app, cpu, g.esp - 4, copy, int32_t(handle)), eax))
            return {};
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, data), eax))
            return {};
        kept = copy;
    }
    GameBank& bank = kGameBanks(app, slot);
    bank.copy = kept;
    const reg32 handle = load<reg32>(app, handleAt);
    bank.handle = int32_t(handle);
    return int32_t(handle);
}

x86::reg32 bankLoad(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, bankLoaded(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
