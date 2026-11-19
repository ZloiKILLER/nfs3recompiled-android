#ifndef NFS3HP_GAME_SETTINGS_H
#define NFS3HP_GAME_SETTINGS_H

#include <lib/app.h>
#include <cstddef>
#include <cstdint>

/* The game's settings, config.dat as it is in memory: 0x20f0 bytes at 0x6fbb40
 * that the game saves whole (sub_472820), loads whole (sub_472980), and copies
 * whole or in stretches into saves, replays and ghosts.  The block also holds
 * the race's setup and the race's cast, so not all of it is a preference.
 *
 * This is a view of the data where the game has it: code that reads it by address still
 * shares it (the network's and the replays' functions above all, sub_477fd0,
 * sub_4dece0), the game's functions get addresses in it, cars keep theirs of
 * a RaceEntry, and the menu lists' table (0x5563e8) points at 66 fields.
 * Every read and write by native code goes through gameSettings(app), and
 * SETTINGS_ADDRESS marks the places that hand an address on or copy a stretch:
 * stage 2 (the block out of guest memory) starts from gameSettings() and those
 * places.
 *
 * Dwords are unsigned as the guest's memory reads are; a native that compares
 * one as signed says so at the place.  Names are from how the natives use the
 * fields; what is not known is an unknownXXX array. */
namespace nfs3hp
{

#pragma pack(push, 1)

/* A rectangle of the HUD, fractions of the view; compared, zeroed and copied as bits too. */
struct HudRect
{
    union
    {
        float top;
        nfs::u32 topBits;
    };
    union
    {
        float left;
        nfs::u32 leftBits;
    };
    union
    {
        float bottom;
        nfs::u32 bottomBits;
    };
    union
    {
        float right;
        nfs::u32 rightBits;
    };
};

/* The HUD of a seat in a form (racer or cop). */
struct HudRecord
{
    nfs::u32 shown;  // the HUD is on at all
    nfs::u32 opponentName;  // the name over the car ahead
    nfs::u32 opponentList;  // the list of the race's cars
    nfs::u32 rearView;  // the mirror in the band
    union
    {
        float mapScale;  // the track map's zoom, 1.0 to 4.0
        nfs::u32 mapScaleBits;
    };
    nfs::u32 splitTimeForm;  // 0 none, else its form from 1; the lap clock's kind
    nfs::u32 speedometerForm;  // 4 and 2 digital, 3 and 1 the dial; 1 mph (English's default), 3 km/h
    nfs::u32 tachometerForm;  // 1 digital, else the dial
    nfs::u32 mapStyle;  // 0 no map, else its form from 1
    HudRect rects[0x18];  // the elements' places; 9 the mirror, 0x11 and 0x16 the map, 23 by language
};

/* A game controller as config.dat remembers it. */
struct DeviceRecord
{
    nfs::u32 kind;  // the first byte: 1/2/4 a DirectInput subtype (2 a wheel or joystick, 4 a game pad), 8 force feedback; 0 none
    char name[0x34];
    nfs::u32 buttonMask;  // a bit for each button
    nfs::u32 povMask;
    nfs::u32 axisMask;  // a bit for each of the eight axes
    nfs::u32 gain;  // force feedback, 0 to 100 (default 75)
    nfs::u32 axisRange[8][2];  // each axis's two ends, 0 to 100
};

/* What a player chose on this machine; a RaceEntry is made of it. */
struct PlayerSettings
{
    nfs::u32 car;  // the car's number
    char carId[0x10];  // the car's id, as a string
    nfs::u32 carClass;  // the class of the car (0x4396b0)
    nfs::u32 transmission;  // 1 automatic, 0 manual
    nfs::u32 unknown1c;  // written by the defaults only
    nfs::u32 digital[3];  // whether steering, throttle and brake are bound to keys (sub_43c040)
    std::uint8_t unknown2c[0x14];
    char name[0xb];
    std::uint8_t colour[4];  // the car's colour bytes
    std::uint8_t unknown4f[1];
    float tuning[7];  // the car's tuning (sub_4dcbc0), else the defaults at 0x55d024
};

/* An entry of the race's cast, 0x6c bytes; cars keep its address at +0x220. */
struct RaceEntry
{
    std::uint8_t unknown00[0x10];
    nfs::u32 place;  // its place on the grid
    nfs::u32 car;  // the car's number (the model)
    nfs::u32 kind;  // bits: 1 a person, 2 an opponent, 4 traffic, 8 a cop
    nfs::u32 transmission;  // 1 automatic, 0 manual; a person's starts as 1
    std::uint8_t colour[4];
    nfs::u32 digital[3];  // PlayerSettings::digital; network message 9 writes them too
    nfs::u32 driver;  // the opponent or cop driver picked
    char name[0xb];
    std::uint8_t unknown3f[1];
    float tuning[7];
    nfs::u32 extras[3];  // from the car, three dwords
    nfs::u32 flag68;  // 0x4395a0 of the car; a network race's machine is left out of the results while it is set
};

/* The knockout series' state (raceType 2). */
struct KnockoutState
{
    std::int16_t stage;  // the race it is at
    std::int16_t places[0x10];  // for each player
    std::int16_t stillIn[0x10];  // for each player
    std::int16_t tracks[7];  // by stage
};

/* The tournament series' state (raceType 1). */
struct TournamentState
{
    std::int16_t stage;
    std::int16_t places[0x10];  // for each player
    std::int16_t raceScores[0x10];  // for each player
    std::int16_t totals[0x10];  // for each player
};

struct GameSettings
{
    nfs::u32 fileSize;  // config.dat's own size, 0x20f0: a file that does not start so is not taken
    nfs::u32 defaultsStamp[2];  // two dwords the defaults copy from 0x55e8b8
    nfs::u32 language;  // by the language's name (sub_472250): 0 English, 1 to 5 German, French, Spanish, Italian, Swedish
    nfs::u32 recordsHolderKept;  // recordsHolder as the last cast or ghost set it
    nfs::u32 installIsRemote;  // sub_4b62a0: the install is "remote": no sound, movies, modem or game of its own
    nfs::u32 comPort;  // from 0, of four
    nfs::u32 chosenModem;
    char phoneNumber[0x1e];
    char tcpAddress[0x1f];
    char ipxGameName[0xb];
    nfs::u32 cameraViewsChosen[4];  // the camera's four views as the menu chose them
    nfs::u32 cameraMode[2];  // the camera mode each player last had
    nfs::u32 cameraViews[4];  // cameraViewsChosen made the race's view numbers (sub_472300)
    std::uint8_t unknown090[8];  // first of the sound's sixteen dwords the options screen keeps whole
    nfs::u32 soundFlag98;  // set when musicMode is not (fitRaceSettings); default 1
    nfs::u32 soundSurround;  // the speakers
    nfs::u32 soundRateKind;  // SNDsetrate: 0, or -50 unless this is set
    nfs::u32 musicMode;  // above 2 the race's music in its other form; default 2
    nfs::u32 menuMusicVolume;
    nfs::u32 menuMusicAsked;  // the menu tune asked for; none when kNoMenuMusic
    nfs::u32 raceMusicVolume;
    nfs::u32 effectsVolume;
    nfs::u32 engineVolume;
    nfs::u32 speechVolume;
    nfs::u32 soundSetup;  // 2 taken as 0, 3 the 3D setup
    nfs::u32 feedKeys[3];  // audio feed: the keys of the time, the position and the speed, 0 to 9
    nfs::u32 configVersion;  // against [0x7a3a58]: another version resets the settings
    nfs::u32 screenMode;  // the mode's index in the list
    nfs::u32 screenWidth;  // the chosen mode's second dword; default 640
    nfs::u32 detailLevel;  // car detail, 0 to 2
    nfs::u32 useSpecular;
    nfs::u32 useLights;
    nfs::u32 viewDistance;  // 0 Full .. 3 Close
    nfs::u32 skyDetail;  // 1 no sky dome pieces, 2 no sky's rest
    nfs::u32 effectsLevel;  // 0 all, 1 the major ones, 2 none; sub_4dece0 reads it
    nfs::u32 projectedText;  // sub_4483a0/sub_448330's choice; the projected text item
    union
    {
        float alphaIntensity;  // the effects' intensity and the sun's glare
        nfs::u32 alphaIntensityBits;
    };
    nfs::u32 detailFlagFc;  // one of the machine-speed tables; polygons with a texture's bit 2 are drawn when 1
    nfs::u32 viewAngle;  // 0 Normal; the port's 2 is Wide
    nfs::u32 detailFlag104;  // set with useSpecular by sub_472d10; read in the race frame
    nfs::u32 nearPolygons;  // polygons near the eye get a kind bit, on a driver with [0x7a3a58] & 6
    nfs::u32 wideScreen;  // Wide Screen in the Graphics menu: the view between bars
    HudRecord hud[7];  // [seat * 2 + form]: three seats, racer and cop; [6] a kept copy of [0] while another car is watched
    std::uint8_t unknownC8c[0x1a4];  // one record's worth past hud[6]; no native reaches it
    DeviceRecord devices[0x10];
    nfs::u32 forceCollision;  // force feedback strengths, 0 off to 100; the jolts
    nfs::u32 forceRoad;
    nfs::u32 forceEngine;
    nfs::u32 forceBodyRoll;  // never with forceRoadGrip
    nfs::u32 forceRoadGrip;
    nfs::u32 deviceCount;
    nfs::u32 forceDeviceCount;
    nfs::u32 controls[3][0xd];  // 13 bindings: [0] one player's, [1] and [2] split screen's
    union
    {
        nfs::u32 unlockedCars;  // bits; used as a byte
        std::uint8_t unlockedCarsByte;
    };
    nfs::u32 unlockedAll;  // the bonus track open too
    std::int16_t duelsWon;  // a bit for each track
    std::int16_t duelsLost;
    nfs::u32 recordsHolder;  // of the track's records (0x4cef00)
    std::uint8_t unknown1778[4];
    PlayerSettings players[2];
    std::uint8_t playersDone[8];  // cleared as a series starts
    std::uint8_t unknown185c[4];
    nfs::u32 replayMode;  // 0 a race, 1 split screen, 2 a replay (sub_4bcb00 sets it while one plays back): no ghost, no forces, no driving
    std::uint8_t unknown1864[4];
    nfs::u32 unknown1868;  // written 2 by the defaults and sub_472d10; no native reads it
    nfs::u32 lastTickRaced;  // the race tick last run past the start (sub_4c48d0), so 0: nothing to replay; cleared as the cast is built
    nfs::u32 playMode;  // 0 one player, 1 split screen, 2 modem, 3 serial, 4 network
    nfs::u32 raceSeed;
    nfs::u32 raceType;  // 0 a single race, 1 a tournament, 2 a knockout, 3 Hot Pursuit
    KnockoutState knockout;
    TournamentState tournament;
    std::uint8_t unknown192e[2];
    nfs::u32 seriesVariant;  // a series' variant, 0 or 1: sides chosen, harder opponents
    nfs::u32 chosenOptions[6];  // the single race's options, copied to the race's
    nfs::u32 pursuitOption;  // Hot Pursuit's, goes to raceOptionPursuit
    nfs::u32 raceOptionComputerDrives;  // the race's options, 7 dwords: the player's car driven for him, the setup's grip goes by it
    nfs::u32 raceOptionLaneVotes;
    nfs::u32 raceOptionCones;  // set out as a race loads; also shown on the HUD
    nfs::u32 raceOptionCopRadio;  // the co-driver's or the cops' radio
    nfs::u32 raceOptionHazard;  // a car leans over too far: the hazard race
    nfs::u32 raceOptionSimulation;  // the tyres' slip and the handling as a simulation's
    nfs::u32 raceOptionPursuit;  // from pursuitOption
    nfs::u32 track;
    nfs::u32 lapsClass;  // 0x4cf160 of the laps: which of three tables of records
    nfs::u32 laps;
    nfs::u32 pursuitWinCount;  // laps or tickets to win; the HUD's ratio; the speeding limit
    nfs::u32 pursuitOutCount;  // tickets that take a car out; the HUD's ratio
    nfs::u32 backwards;  // the track run the other way. With the next three the "sides" a series chooses
    nfs::u32 mirrored;  // the track mirrored: steering and calls swapped, own textures
    nfs::u32 night;
    nfs::u32 weather;  // rain or snow; lights on, the crowd
    nfs::u32 trafficLevel;  // 0 to 2; 2 none
    nfs::u32 opponentClass;  // 0 to 3 a class of cars, past that one car
    std::uint8_t unknown1998[0x10];
    nfs::u32 noTraffic;
    nfs::u32 unknown19ac;  // default 1; no native reads it
    nfs::u32 seriesRace;  // set as a series or pursuit builds its cast; 1 cuts the racers' levels
    nfs::u32 localPlayer;  // the player at this machine
    nfs::u32 viewedPlayer;  // the player the views follow
    union
    {
        nfs::u32 raceFlags;  // used as a byte: 1 and 0x20 the handling a simulation's, 2 start records, 4, 8 knock-out, 0x10; the menu's cheats
        std::uint8_t raceFlagsByte;
    };
    nfs::u32 cheatTrack;  // a cheat's track or cop car
    nfs::u32 setting504;  // a race setting, 0 to 3; 3 not in a two-player series
    nfs::u32 networkLobby;  // a network race's lobby: the machine ready, the sub-mode
    nfs::u32 networkGame;  // a network game is set up; only this machine's player queued here
    nfs::u32 entryCount;  // everyone in the cast
    nfs::u32 racerCount;
    nfs::u32 humanCount;
    nfs::u32 copCount;
    nfs::u32 opponentCount;
    nfs::u32 pursuerCount;
    nfs::u32 trafficCount;
    RaceEntry entries[0x10];
    nfs::u32 gridSlots[0x11];  // an entry's place on the grid

    /* A table's record by the byte offset a native has in a register: what
     * index * size computes, wrapped as the game's address arithmetic wraps. */
    HudRecord& hudAt(nfs::u32 byteOffset) { return *reinterpret_cast<HudRecord*>(reinterpret_cast<char*>(hud) + nfs::s32(byteOffset)); }
    DeviceRecord& deviceAt(nfs::u32 byteOffset) { return *reinterpret_cast<DeviceRecord*>(reinterpret_cast<char*>(devices) + nfs::s32(byteOffset)); }
    RaceEntry& entryAt(nfs::u32 byteOffset) { return *reinterpret_cast<RaceEntry*>(reinterpret_cast<char*>(entries) + nfs::s32(byteOffset)); }
    /* A seat's record in a form (0 racer, 1 cop). */
    HudRecord& hudOf(nfs::u32 seat, nfs::u32 form) { return hud[seat * 2 + form]; }
};

#pragma pack(pop)

static_assert(sizeof(HudRect) == 0x10);
static_assert(offsetof(HudRect, top) == 0x0);
static_assert(offsetof(HudRect, left) == 0x4);
static_assert(offsetof(HudRect, bottom) == 0x8);
static_assert(offsetof(HudRect, right) == 0xc);
static_assert(sizeof(HudRecord) == 0x1a4);
static_assert(offsetof(HudRecord, shown) == 0x0);
static_assert(offsetof(HudRecord, opponentName) == 0x4);
static_assert(offsetof(HudRecord, opponentList) == 0x8);
static_assert(offsetof(HudRecord, rearView) == 0xc);
static_assert(offsetof(HudRecord, mapScale) == 0x10);
static_assert(offsetof(HudRecord, splitTimeForm) == 0x14);
static_assert(offsetof(HudRecord, speedometerForm) == 0x18);
static_assert(offsetof(HudRecord, tachometerForm) == 0x1c);
static_assert(offsetof(HudRecord, mapStyle) == 0x20);
static_assert(offsetof(HudRecord, rects) == 0x24);
static_assert(sizeof(DeviceRecord) == 0x88);
static_assert(offsetof(DeviceRecord, kind) == 0x0);
static_assert(offsetof(DeviceRecord, name) == 0x4);
static_assert(offsetof(DeviceRecord, buttonMask) == 0x38);
static_assert(offsetof(DeviceRecord, povMask) == 0x3c);
static_assert(offsetof(DeviceRecord, axisMask) == 0x40);
static_assert(offsetof(DeviceRecord, gain) == 0x44);
static_assert(offsetof(DeviceRecord, axisRange) == 0x48);
static_assert(sizeof(PlayerSettings) == 0x6c);
static_assert(offsetof(PlayerSettings, car) == 0x0);
static_assert(offsetof(PlayerSettings, carId) == 0x4);
static_assert(offsetof(PlayerSettings, carClass) == 0x14);
static_assert(offsetof(PlayerSettings, transmission) == 0x18);
static_assert(offsetof(PlayerSettings, unknown1c) == 0x1c);
static_assert(offsetof(PlayerSettings, digital) == 0x20);
static_assert(offsetof(PlayerSettings, unknown2c) == 0x2c);
static_assert(offsetof(PlayerSettings, name) == 0x40);
static_assert(offsetof(PlayerSettings, colour) == 0x4b);
static_assert(offsetof(PlayerSettings, unknown4f) == 0x4f);
static_assert(offsetof(PlayerSettings, tuning) == 0x50);
static_assert(sizeof(RaceEntry) == 0x6c);
static_assert(offsetof(RaceEntry, unknown00) == 0x0);
static_assert(offsetof(RaceEntry, place) == 0x10);
static_assert(offsetof(RaceEntry, car) == 0x14);
static_assert(offsetof(RaceEntry, kind) == 0x18);
static_assert(offsetof(RaceEntry, transmission) == 0x1c);
static_assert(offsetof(RaceEntry, colour) == 0x20);
static_assert(offsetof(RaceEntry, digital) == 0x24);
static_assert(offsetof(RaceEntry, driver) == 0x30);
static_assert(offsetof(RaceEntry, name) == 0x34);
static_assert(offsetof(RaceEntry, unknown3f) == 0x3f);
static_assert(offsetof(RaceEntry, tuning) == 0x40);
static_assert(offsetof(RaceEntry, extras) == 0x5c);
static_assert(offsetof(RaceEntry, flag68) == 0x68);
static_assert(sizeof(KnockoutState) == 0x50);
static_assert(offsetof(KnockoutState, stage) == 0x0);
static_assert(offsetof(KnockoutState, places) == 0x2);
static_assert(offsetof(KnockoutState, stillIn) == 0x22);
static_assert(offsetof(KnockoutState, tracks) == 0x42);
static_assert(sizeof(TournamentState) == 0x62);
static_assert(offsetof(TournamentState, stage) == 0x0);
static_assert(offsetof(TournamentState, places) == 0x2);
static_assert(offsetof(TournamentState, raceScores) == 0x22);
static_assert(offsetof(TournamentState, totals) == 0x42);
static_assert(sizeof(GameSettings) == 0x20f0);
static_assert(offsetof(GameSettings, fileSize) == 0x0);
static_assert(offsetof(GameSettings, defaultsStamp) == 0x4);
static_assert(offsetof(GameSettings, language) == 0xc);
static_assert(offsetof(GameSettings, recordsHolderKept) == 0x10);
static_assert(offsetof(GameSettings, installIsRemote) == 0x14);
static_assert(offsetof(GameSettings, comPort) == 0x18);
static_assert(offsetof(GameSettings, chosenModem) == 0x1c);
static_assert(offsetof(GameSettings, phoneNumber) == 0x20);
static_assert(offsetof(GameSettings, tcpAddress) == 0x3e);
static_assert(offsetof(GameSettings, ipxGameName) == 0x5d);
static_assert(offsetof(GameSettings, cameraViewsChosen) == 0x68);
static_assert(offsetof(GameSettings, cameraMode) == 0x78);
static_assert(offsetof(GameSettings, cameraViews) == 0x80);
static_assert(offsetof(GameSettings, unknown090) == 0x90);
static_assert(offsetof(GameSettings, soundFlag98) == 0x98);
static_assert(offsetof(GameSettings, soundSurround) == 0x9c);
static_assert(offsetof(GameSettings, soundRateKind) == 0xa0);
static_assert(offsetof(GameSettings, musicMode) == 0xa4);
static_assert(offsetof(GameSettings, menuMusicVolume) == 0xa8);
static_assert(offsetof(GameSettings, menuMusicAsked) == 0xac);
static_assert(offsetof(GameSettings, raceMusicVolume) == 0xb0);
static_assert(offsetof(GameSettings, effectsVolume) == 0xb4);
static_assert(offsetof(GameSettings, engineVolume) == 0xb8);
static_assert(offsetof(GameSettings, speechVolume) == 0xbc);
static_assert(offsetof(GameSettings, soundSetup) == 0xc0);
static_assert(offsetof(GameSettings, feedKeys) == 0xc4);
static_assert(offsetof(GameSettings, configVersion) == 0xd0);
static_assert(offsetof(GameSettings, screenMode) == 0xd4);
static_assert(offsetof(GameSettings, screenWidth) == 0xd8);
static_assert(offsetof(GameSettings, detailLevel) == 0xdc);
static_assert(offsetof(GameSettings, useSpecular) == 0xe0);
static_assert(offsetof(GameSettings, useLights) == 0xe4);
static_assert(offsetof(GameSettings, viewDistance) == 0xe8);
static_assert(offsetof(GameSettings, skyDetail) == 0xec);
static_assert(offsetof(GameSettings, effectsLevel) == 0xf0);
static_assert(offsetof(GameSettings, projectedText) == 0xf4);
static_assert(offsetof(GameSettings, alphaIntensity) == 0xf8);
static_assert(offsetof(GameSettings, detailFlagFc) == 0xfc);
static_assert(offsetof(GameSettings, viewAngle) == 0x100);
static_assert(offsetof(GameSettings, detailFlag104) == 0x104);
static_assert(offsetof(GameSettings, nearPolygons) == 0x108);
static_assert(offsetof(GameSettings, wideScreen) == 0x10c);
static_assert(offsetof(GameSettings, hud) == 0x110);
static_assert(offsetof(GameSettings, unknownC8c) == 0xc8c);
static_assert(offsetof(GameSettings, devices) == 0xe30);
static_assert(offsetof(GameSettings, forceCollision) == 0x16b0);
static_assert(offsetof(GameSettings, forceRoad) == 0x16b4);
static_assert(offsetof(GameSettings, forceEngine) == 0x16b8);
static_assert(offsetof(GameSettings, forceBodyRoll) == 0x16bc);
static_assert(offsetof(GameSettings, forceRoadGrip) == 0x16c0);
static_assert(offsetof(GameSettings, deviceCount) == 0x16c4);
static_assert(offsetof(GameSettings, forceDeviceCount) == 0x16c8);
static_assert(offsetof(GameSettings, controls) == 0x16cc);
static_assert(offsetof(GameSettings, unlockedCars) == 0x1768);
static_assert(offsetof(GameSettings, unlockedAll) == 0x176c);
static_assert(offsetof(GameSettings, duelsWon) == 0x1770);
static_assert(offsetof(GameSettings, duelsLost) == 0x1772);
static_assert(offsetof(GameSettings, recordsHolder) == 0x1774);
static_assert(offsetof(GameSettings, unknown1778) == 0x1778);
static_assert(offsetof(GameSettings, players) == 0x177c);
static_assert(offsetof(GameSettings, playersDone) == 0x1854);
static_assert(offsetof(GameSettings, unknown185c) == 0x185c);
static_assert(offsetof(GameSettings, replayMode) == 0x1860);
static_assert(offsetof(GameSettings, unknown1864) == 0x1864);
static_assert(offsetof(GameSettings, unknown1868) == 0x1868);
static_assert(offsetof(GameSettings, lastTickRaced) == 0x186c);
static_assert(offsetof(GameSettings, playMode) == 0x1870);
static_assert(offsetof(GameSettings, raceSeed) == 0x1874);
static_assert(offsetof(GameSettings, raceType) == 0x1878);
static_assert(offsetof(GameSettings, knockout) == 0x187c);
static_assert(offsetof(GameSettings, tournament) == 0x18cc);
static_assert(offsetof(GameSettings, unknown192e) == 0x192e);
static_assert(offsetof(GameSettings, seriesVariant) == 0x1930);
static_assert(offsetof(GameSettings, chosenOptions) == 0x1934);
static_assert(offsetof(GameSettings, pursuitOption) == 0x194c);
static_assert(offsetof(GameSettings, raceOptionComputerDrives) == 0x1950);
static_assert(offsetof(GameSettings, raceOptionLaneVotes) == 0x1954);
static_assert(offsetof(GameSettings, raceOptionCones) == 0x1958);
static_assert(offsetof(GameSettings, raceOptionCopRadio) == 0x195c);
static_assert(offsetof(GameSettings, raceOptionHazard) == 0x1960);
static_assert(offsetof(GameSettings, raceOptionSimulation) == 0x1964);
static_assert(offsetof(GameSettings, raceOptionPursuit) == 0x1968);
static_assert(offsetof(GameSettings, track) == 0x196c);
static_assert(offsetof(GameSettings, lapsClass) == 0x1970);
static_assert(offsetof(GameSettings, laps) == 0x1974);
static_assert(offsetof(GameSettings, pursuitWinCount) == 0x1978);
static_assert(offsetof(GameSettings, pursuitOutCount) == 0x197c);
static_assert(offsetof(GameSettings, backwards) == 0x1980);
static_assert(offsetof(GameSettings, mirrored) == 0x1984);
static_assert(offsetof(GameSettings, night) == 0x1988);
static_assert(offsetof(GameSettings, weather) == 0x198c);
static_assert(offsetof(GameSettings, trafficLevel) == 0x1990);
static_assert(offsetof(GameSettings, opponentClass) == 0x1994);
static_assert(offsetof(GameSettings, unknown1998) == 0x1998);
static_assert(offsetof(GameSettings, noTraffic) == 0x19a8);
static_assert(offsetof(GameSettings, unknown19ac) == 0x19ac);
static_assert(offsetof(GameSettings, seriesRace) == 0x19b0);
static_assert(offsetof(GameSettings, localPlayer) == 0x19b4);
static_assert(offsetof(GameSettings, viewedPlayer) == 0x19b8);
static_assert(offsetof(GameSettings, raceFlags) == 0x19bc);
static_assert(offsetof(GameSettings, cheatTrack) == 0x19c0);
static_assert(offsetof(GameSettings, setting504) == 0x19c4);
static_assert(offsetof(GameSettings, networkLobby) == 0x19c8);
static_assert(offsetof(GameSettings, networkGame) == 0x19cc);
static_assert(offsetof(GameSettings, entryCount) == 0x19d0);
static_assert(offsetof(GameSettings, racerCount) == 0x19d4);
static_assert(offsetof(GameSettings, humanCount) == 0x19d8);
static_assert(offsetof(GameSettings, copCount) == 0x19dc);
static_assert(offsetof(GameSettings, opponentCount) == 0x19e0);
static_assert(offsetof(GameSettings, pursuerCount) == 0x19e4);
static_assert(offsetof(GameSettings, trafficCount) == 0x19e8);
static_assert(offsetof(GameSettings, entries) == 0x19ec);
static_assert(offsetof(GameSettings, gridSlots) == 0x20ac);

/* Where the block is in the guest's memory, and the one place that says so. */
constexpr nfs::u32 kGameSettingsAddress = 0x6fbb40;

inline GameSettings& gameSettings(nfs::App* app)
{
    return *reinterpret_cast<GameSettings*>(&app->getMemory<nfs::u8>(kGameSettingsAddress));
}

/* A field's guest address, for what cannot go through a field's value: an address
 * handed on to a game function, or a stretch of the block copied.  `field` as
 * offsetof takes it: players[1].name, hud, devices[0].gain.  A constant expression. */
#define SETTINGS_ADDRESS(field) (::nfs3hp::kGameSettingsAddress + nfs::u32(offsetof(::nfs3hp::GameSettings, field)))

/* A seat's HUD record by address, for what copies rectangles or hands them on. */
constexpr nfs::u32 hudRecordAddress(nfs::u32 seat, nfs::u32 form)
{
    return SETTINGS_ADDRESS(hud) + (seat * 2 + form) * nfs::u32(sizeof(HudRecord));
}

}  // namespace nfs3hp

#endif
