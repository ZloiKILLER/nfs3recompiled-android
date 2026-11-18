#include "native_frontend_loada.h"
#include "port_fields.h"
#include "native_load_carmodels.h"
#include "native_load_saves.h"
#include "native_ai_race.h"
#include "native_watcom.h"
#include "native_ea_net.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_aiplan.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_eaa.h"
#include "native_frontend_menub.h"
#include "native_frontend_menuf.h"
#include "native_frontend_misc.h"
#include "native_frontend_neta.h"
#include "native_frontend_rendd.h"
#include "native_frontend_showcase.h"
#include "native_frontend_simb.h"
#include "native_frontend_sime.h"
#include "native_handling.h"
#include "native_load_cardb.h"
#include "native_graphics.h"
#include "native_raceload.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <fpu.h>
// What lib/thread.h holds pointers to, declared here.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <cstddef>
#include <cstdint>
#include <cstring>

/* What a race's loading does around the track and the cars' files, decompiled:
 *
 *   the cast         who is in the race: the players, the opponents, the cops and the
 *                    traffic, each an entry of the table at 0x6fd52c, chosen from the
 *                    race's seed so that every machine of a network game agrees;
 *   the cars in it   a car's lists, its reset, the free place beside the road;
 *   the car list     the lists the car menus filter, kept cars freed and removed;
 *   kept files       a saved race's cars by their ids, the replay read with the
 *                    settings kept, a copy of a file, room on the disk;
 *   the horizon      the sizes of the sixteen pieces of the sky's horizon;
 *   small steps      the loader's calls in their order, each followed by the check for
 *                    a stop asked for.
 *
 * All of the game's state stays in guest memory.  What each function leaves in eax and
 * in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::loada (native_frontend_loada.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * this round's three files (rendd, rende, loada) calls another as C++ (guest::direct), and the
 * typed cores of other files too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg32;
using x86::sreg32;

// The game's functions, by address.
constexpr reg32 kFillZero = 0x4e070c;          // eax, edx bytes
constexpr reg32 kMemset = 0x4e0640;            // eax, the byte in edx, ebx bytes
constexpr reg32 kStrncpy = 0x4e0e30;           // eax from edx, ebx at most
constexpr reg32 kRand = 0x4eb1fc;
constexpr reg32 kSliceHasLane = 0x40e220;      // eax a slice, edx a lane
constexpr reg32 kSnapToTrack = 0x4205a0;       // eax a point, edx 1, ebx the slice, esi the place's record
constexpr reg32 kCarColourRgb = 0x439810;      // eax a car, edx a colour, ebx 0, ecx and the stack four bytes' places
constexpr reg32 kDriverNameOf = 0x4047f0;      // eax an entry: a name to copy
constexpr reg32 kDriverColourOf = 0x404810;    // eax an entry: a colour
constexpr reg32 kNearestColour = 0x4733c0;     // eax an entry, edx a colour
constexpr reg32 kPlayerMachine = 0x4a6690;     // eax a network player: its machine, -1 for none, in ax

// The cast, each player's settings and the kinds of race.
constexpr reg32 kNetworkCounts = 0x7a2274;     // the words at +2, the players, and +4, the first's seat
constexpr reg32 kDefaultTuning = 0x55d024;     // seven floats
constexpr reg32 kNoExtras = 0x56448c;          // three words
constexpr reg32 kRaceFlags = 0x55eb2e;         // kRaceFlagAllCops, the developer's all cops race
constexpr std::uint8_t kRaceFlagAllCops = 0x20;
constexpr reg32 kLastRandomColour = 0x679000;


/* An entry of the cast. */
constexpr reg32 kEntrySize = 0x6c;
constexpr reg32 kEntryPlace = 0x10;
constexpr reg32 kEntryCar = 0x14;
constexpr reg32 kEntryKind = 0x18;             // bits: 1 a person, 2 an opponent, 4 traffic, 8 a cop
constexpr reg32 kEntryKindOfDriver = 0x1c;
constexpr reg32 kEntryColour = 0x20;           // four bytes
constexpr reg32 kEntryName = 0x34;             // 11 characters
constexpr reg32 kEntryTuning = 0x40;           // seven floats
constexpr reg32 kEntryExtras = 0x5c;           // three words
constexpr reg32 kEntryFlag8 = 0x68;
constexpr std::uint8_t kEntryIsCop = 8;

reg32 entryAt(reg32 place)
{
    return SETTINGS_ADDRESS(entries) + place * kEntrySize;
}

/* A game function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

Float floatIn(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* What a jump after `fcom a, b; fnstsw ax; sahf` reads: jb, and jbe.  Unordered sets
 * every flag, so it counts as both. */
bool isBelow(x86::reg16 status)
{
    return (status & 0x100) != 0;
}

bool isBelowOrEqual(x86::reg16 status)
{
    return (status & 0x4100) != 0;
}

/* rep movsd: forwards, a dword at a time. */
void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A word in guest memory, sign extended: what `mov eax, [address - 2]; sar eax, 16` gives. */
sreg32 signedWord(const Guest& guest, reg32 address)
{
    return sreg32(std::int16_t(guest.word(address)));
}

/* The loader's way of going on: each step, then the check for a stop; what the last
 * check returned stays in eax. */
template <std::size_t N>
reg32 stepsWithChecks(win32::WinApplication* app, x86::CPU& cpu, Guest& guest, const reg32 (&steps)[N])
{
    reg32 left = 0;
    for (const reg32 step : steps)
    {
        fx::callAt(guest, step);
        left = guest::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    }
    return left;
}

/* ---------------------------------------------------------------------------
 * The cast
 * ------------------------------------------------------------------------- */

constexpr reg32 kUsedCounts = 0x678c00;        // per kind of choice, 256 bytes: how often each was chosen
constexpr reg32 kSeedBytes = 0x552938;         // 256 bytes, the race's numbers
constexpr reg32 kClassSizes = 0x5555f8;        // how many cars in each of four classes
constexpr reg32 kClassCars = 0x5552d8;         // [class][50] of records, by class
constexpr reg32 kClassCarIdOffset = 0x31c;



/* A network player's entry, its car set by the network. */
void entryFromNetwork(win32::WinApplication* app, x86::CPU& cpu, Guest& guest, reg32 entry)
{
    guest.setDword(entry + kEntryKind, 1);
    copyDwords(guest, entry + kEntryTuning, kDefaultTuning, 7);
    const reg32 flag = guest::direct<0x4395a0, frontend::loada::carIsKept>(app, cpu, guest.dword(entry + kEntryCar));
    guest.setDword(entry + kEntryFlag8, flag);
}

/* The four sides, from the race's numbers at the stage's four places. */
constexpr reg32 GameSettings::* kSides[4] = { &GameSettings::backwards, &GameSettings::mirrored, &GameSettings::night, &GameSettings::weather };

void chooseSides(win32::WinApplication* app, x86::CPU& cpu, Guest& guest, sreg32 stage)
{
    const reg32 base = reg32(stage) * 4;
    for (reg32 i = 0; i < 4; ++i)
        gameSettings(guest).*kSides[i] = reg32(sreg32(guest::direct<0x474c60, frontend::loada::rosterSeedByte>(app, cpu, base + i)) % 2);
}

void noSides(Guest& guest)
{
    for (reg32 i = 0; i < 4; ++i)
        gameSettings(guest).*kSides[i] = 0;
}

void clearCounts(Guest& guest)
{
    for (reg32 GameSettings::*count : { &GameSettings::racerCount, &GameSettings::humanCount, &GameSettings::copCount,
                                        &GameSettings::opponentCount, &GameSettings::pursuerCount, &GameSettings::trafficCount,
                                        &GameSettings::entryCount })
        gameSettings(guest).*count = 0;
}

/* The default tuning and no extras, as the cast's computer-driven entries have them. */
void copyDefaultsAndExtras(Guest& guest, reg32 entry)
{
    copyDwords(guest, entry + kEntryTuning, kDefaultTuning, 7);
    copyDwords(guest, entry + kEntryExtras, kNoExtras, 3);
}

/* A random colour number, as the all-cops race has it: never 0, 2 or 3. */
reg32 allCopsColour(Guest& guest)
{
    for (;;)
    {
        const reg32 random = fe::direct<kRand, watcom::clib::rand>(guest.app(), guest.cpu());
        const reg32 colour = random & 7;
        if (colour != 0 && colour != 2 && colour != 3)
            return colour;
    }
}

}

/* sub_473340: nothing chosen yet. */
x86::reg32 frontend::loada::rosterClearUsed(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kUsedCounts, 0x400);
}

x86::reg32 frontend::loadingRosterClearUsed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473340", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterClearUsed(app, cpu); });
}

/* sub_473360: eax the kind of choice, edx which of the race's numbers, ebx how many there
 * are: the least chosen of them from that number on, marked chosen; in edx as well. */
x86::reg32 frontend::loada::rosterPickLeastUsed(App* app, CPU& cpu, reg32 kind, reg32 index, reg32 spanWord)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const sreg32 span = sreg32(spanWord);
    const sreg32 start = sreg32(guest::direct<0x474c60, rosterSeedByte>(app, cpu, index)) % span;
    const reg32 counts = kUsedCounts + (kind << 8);
    sreg32 pick = start;
    sreg32 level = 0;
    while (level < sreg32(guest.byte(counts + reg32(pick))))
    {
        pick = (pick + 1) % span;
        if (pick == start)
            ++level;
    }
    guest.setByte(counts + reg32(pick), std::uint8_t(guest.byte(counts + reg32(pick)) + 1));
    return reg32(pick);
}

x86::reg32 frontend::loadingRosterPickLeastUsed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473360", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 kind = args.eax, index = args.edx;
    const sreg32 span = sreg32(args.ebx);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterPickLeastUsed(app, cpu, kind, index, span); });
}

/* sub_4733b0: eax the kind of choice, edx the one to mark. */
x86::reg32 frontend::loada::rosterMarkUsed(App* app, CPU& cpu, reg32 kind, reg32 pick)
{
    Guest guest(app, cpu);
    const reg32 row = kind << 8;
    guest.setByte(pick + row + kUsedCounts, std::uint8_t(guest.byte(pick + row + kUsedCounts) + 1));
    return row;
}

x86::reg32 frontend::loadingRosterMarkUsed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4733b0", 0, 0, nullptr, 0, 0 };
    const reg32 kind = args.eax, pick = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterMarkUsed(app, cpu, kind, pick); });
}

/* sub_4734d0: an opponent's car: a car of the class chosen, or the one car chosen. */
x86::reg32 frontend::loada::rosterPickOpponentCar(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const reg32 chosen = gameSettings(guest).opponentClass;
    if (sreg32(chosen) >= 4)
        return chosen - 4;
    const reg32 pick = guest::direct<0x473360, rosterPickLeastUsed>(app, cpu, 1u, index, guest.dword(kClassSizes + 4 * chosen));
    const reg32 classNow = gameSettings(guest).opponentClass;
    const sreg32 inClass = sreg32(guest.dword(kClassSizes + 4 * classNow));
    const reg32 car = guest::direct<0x43a6b0, carInClass>(app, cpu, reg32(sreg32(pick) % inClass), classNow);
    const sreg32 inClassNow = sreg32(guest.dword(kClassSizes + 4 * gameSettings(guest).opponentClass));
    guest::direct<0x4733b0, rosterMarkUsed>(app, cpu, 1u, reg32(sreg32(pick) % inClassNow));
    return car;
}

x86::reg32 frontend::loadingRosterPickOpponentCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4734d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterPickOpponentCar(app, cpu, index); });
}

/* sub_473550: an opponent's driver, of seven. */
x86::reg32 frontend::loada::rosterPickOpponentDriver(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    return guest::direct<0x473360, rosterPickLeastUsed>(app, cpu, 2u, index, 7u);
}

x86::reg32 frontend::loadingRosterPickOpponentDriver(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterPickOpponentDriver(app, cpu, index); });
}

/* sub_473570: a cop's driver, of eight after the seven. */
x86::reg32 frontend::loada::rosterPickCopDriver(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    return guest::direct<0x473360, rosterPickLeastUsed>(app, cpu, 3u, index, 8u) + 7;
}

x86::reg32 frontend::loadingRosterPickCopDriver(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473570", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterPickCopDriver(app, cpu, index); });
}

/* sub_4735c0: a car of the traffic on this track: a table of a count and the cars,
 * track by track; 50 past the tracks. */
x86::reg32 frontend::loada::rosterPickTrafficCar(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    static const reg32 kTables[11] = { 0x55b040, 0x55b04c, 0x55b064, 0x55b06f, 0x55b040, 0x55b04c,
                                       0x55b064, 0x55b06f, 0x55b058, 0x55b080, 0x55b078 };
    const reg32 track = gameSettings(guest).track;
    if (track > 10)
        return 0x32;
    const reg32 table = kTables[track];
    const reg32 pick = guest::direct<0x473360, rosterPickLeastUsed>(app, cpu, 0u, index, reg32(guest.byte(table)));
    return guest.byte(pick + table + 1);
}

x86::reg32 frontend::loadingRosterPickTrafficCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4735c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterPickTrafficCar(app, cpu, index); });
}

/* sub_4736c0: a tournament's finalist in the car its winner has. */
x86::reg32 frontend::loada::rosterEntryFinalist(App* app, CPU& cpu, reg32 entry)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    constexpr reg32 kWinnersCar = 0x6fdc30;
    guest.setDword(entry + kEntryCar, guest.dword(kWinnersCar));
    guest.setDword(entry + kEntryKindOfDriver, 1);
    const reg32 extras = guest::direct<0x439510, carExtras>(app, cpu, guest.dword(entry + kEntryCar));
    copyDwords(guest, entry + kEntryExtras, extras, 3);
    guest.setDword(entry + kEntryFlag8, 0);
    copyDwords(guest, entry + kEntryTuning, kDefaultTuning, 7);
    return extras;
}

x86::reg32 frontend::loadingRosterEntryFinalist(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4736c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 entry = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterEntryFinalist(app, cpu, entry); });
}

/* sub_473710: a person's entry: eax the entry, edx its place, ebx which player. */
x86::reg32 frontend::loada::rosterEntryHuman(App* app, CPU& cpu, reg32 entry, reg32 place, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x34);
    // Past the two players, what esi held: no caller has more.
    const reg32 settings = player == 0 ? SETTINGS_ADDRESS(players[0].car) : player == 1 ? SETTINGS_ADDRESS(players[1].car) : 0;
    guest.setDword(entry + kEntryFlag8, 0);
    guest.setDword(entry + kEntryCar, guest.dword(settings));
    guest.setDword(entry + kEntryKindOfDriver, guest.dword(settings + 0x18));
    if (guest.byte(kRaceFlags) & kRaceFlagAllCops)
    {
        const reg32 frame = guest.frame();
        fe::direct<0x438ad0, load::makeId>(app, cpu, frame, 0xfu);
        const reg32 copCar = fe::direct<0x439990, load::placeById>(app, cpu, frame);
        guest.setDword(entry + kEntryCar, copCar);
        const reg32 colour = allCopsColour(guest);
        guest.setDword(kLastRandomColour, colour);
        fe::direct<kCarColourRgb, port::fields::carColourBytes>(app, cpu, cpu.esp - 4 - 4 * 3, guest.dword(entry + kEntryCar),
                                                                 sreg32(colour), 0, frame + 0x20, frame + 0x18, frame + 0x1c,
                                                                 frame + 0x14);
        guest.setByte(entry + kEntryColour, guest.byte(frame + 0x20));
        guest.setByte(entry + kEntryColour + 1, guest.byte(frame + 0x18));
        guest.setByte(entry + kEntryColour + 2, guest.byte(frame + 0x1c));
        guest.setByte(entry + kEntryColour + 3, guest.byte(frame + 0x14));
    }
    else
    {
        for (reg32 i = 0; i < 4; ++i)
            guest.setByte(entry + kEntryColour + i, guest.byte(settings + 0x4b + i));
    }
    guest.setDword(entry + kEntryKind, 1);
    copyDwords(guest, entry + 0x24, settings + 0x20, 3);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), entry + kEntryName, settings + 0x40, 0xb);
    guest.setDword(entry + kEntryPlace, place);
    const bool ownTuning = sreg32(gameSettings(guest).playMode) <= 1 && gameSettings(guest).raceType != 1 && gameSettings(guest).raceType != 2;
    copyDwords(guest, entry + kEntryTuning, ownTuning ? settings + 0x50 : kDefaultTuning, 7);
    const reg32 extras = guest::direct<0x439510, carExtras>(app, cpu, guest.dword(entry + kEntryCar));
    copyDwords(guest, entry + kEntryExtras, extras, 3);
    return extras;
}

x86::reg32 frontend::loadingRosterEntryHuman(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473710", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 entry = args.eax, place = args.edx, player = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterEntryHuman(app, cpu, entry, place, player); });
}

/* sub_473890: a cop's entry: eax the entry, edx the race's number for it, ebx nonzero for
 * the pursuit's own car, else the track's. */
x86::reg32 frontend::loada::rosterEntryCop(App* app, CPU& cpu, reg32 entry, reg32 index, reg32 pursuit)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    if (pursuit != 0)
        guest.setDword(entry + kEntryCar, guest.byte(kRaceFlags) & kRaceFlagAllCops ? 0x1f9 : 0x1f8);
    else
    {
        static const reg32 kCarOfTrack[9] = { 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x1f4 };
        const reg32 track = gameSettings(guest).track;
        guest.setDword(entry + kEntryCar, track > 8 ? 0x1f4 : kCarOfTrack[track]);
    }
    guest.setDword(entry + kEntryKindOfDriver, 1);
    guest.setDword(entry + kEntryKind, 8);
    guest.setDword(entry + 0x30, guest::direct<0x473570, rosterPickCopDriver>(app, cpu, index));
    const reg32 name = fe::direct<kDriverNameOf, airace::classValueOf>(guest.app(), guest.cpu(), entry);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), entry + kEntryName, name, 0xb);
    copyDefaultsAndExtras(guest, entry);
    return entry;
}

x86::reg32 frontend::loadingRosterEntryCop(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473890", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 entry = args.eax, index = args.edx, pursuit = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterEntryCop(app, cpu, entry, index, pursuit); });
}

/* sub_473960: an opponent's entry: eax the entry, edx its place, which is also which of
 * the race's numbers it goes by. */
x86::reg32 frontend::loada::rosterEntryOpponent(App* app, CPU& cpu, reg32 entry, reg32 place)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x38);
    const reg32 car = guest::direct<0x4734d0, rosterPickOpponentCar>(app, cpu, place);
    guest.setDword(entry + kEntryKindOfDriver, 1);
    guest.setDword(entry + kEntryFlag8, 0);
    guest.setDword(entry + kEntryCar, car);
    guest.setDword(entry + kEntryKind, 2);
    guest.setDword(entry + 0x30, guest::direct<0x473550, rosterPickOpponentDriver>(app, cpu, place));
    guest.setDword(entry + kEntryPlace, place);
    const reg32 name = fe::direct<kDriverNameOf, airace::classValueOf>(guest.app(), guest.cpu(), entry);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), entry + kEntryName, name, 0xb);
    if (guest.byte(kRaceFlags) & kRaceFlagAllCops)
    {
        const reg32 frame = guest.frame();
        fe::direct<0x438ad0, load::makeId>(app, cpu, frame, 0xfu);
        const reg32 copCar = fe::direct<0x439990, load::placeById>(app, cpu, frame);
        guest.setDword(entry + kEntryCar, copCar);
        reg32 colour;
        do
            colour = allCopsColour(guest);
        while (colour == guest.dword(kLastRandomColour));
        fe::direct<kCarColourRgb, port::fields::carColourBytes>(app, cpu, cpu.esp - 4 - 4 * 3, guest.dword(entry + kEntryCar),
                                                                 sreg32(colour), 0, frame + 0x14, frame + 0x18, frame + 0x1c,
                                                                 frame + 0x20);
        guest.setByte(entry + kEntryColour, guest.byte(frame + 0x14));
        guest.setByte(entry + kEntryColour + 1, guest.byte(frame + 0x18));
        guest.setByte(entry + kEntryColour + 2, guest.byte(frame + 0x1c));
        guest.setByte(entry + kEntryColour + 3, guest.byte(frame + 0x20));
    }
    else
    {
        const reg32 colour = fx::callAt(guest, kDriverColourOf, args({ entry }));
        fx::callAt(guest, kNearestColour, args({ entry, colour }));
    }
    copyDefaultsAndExtras(guest, entry);
    return entry;
}

x86::reg32 frontend::loadingRosterEntryOpponent(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473960", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 entry = args.eax, place = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterEntryOpponent(app, cpu, entry, place); });
}

/* sub_473a70: a traffic car's entry: eax the entry, edx which of the race's numbers. */
x86::reg32 frontend::loada::rosterEntryTraffic(App* app, CPU& cpu, reg32 entry, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const reg32 car = guest::direct<0x4735c0, rosterPickTrafficCar>(app, cpu, index);
    guest.setDword(entry + kEntryKind, 4);
    guest.setDword(entry + kEntryKindOfDriver, 1);
    guest.setByte(entry + kEntryName, 0);
    guest.setDword(entry + kEntryFlag8, 0);
    guest.setDword(entry + kEntryCar, car);
    copyDefaultsAndExtras(guest, entry);
    return car;
}

x86::reg32 frontend::loadingRosterEntryTraffic(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473a70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 entry = args.eax, index = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterEntryTraffic(app, cpu, entry, index); });
}

/* sub_473ac0: a tournament's seven tracks in the race's order: of nine, two left out,
 * from the first four when [0x6fd470] is set, else from the next four. */
x86::reg32 frontend::loada::rosterTournamentTracks(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x38);
    bool present[9];
    for (bool& one : present)
        one = true;
    reg32 index = 0x138;
    for (reg32 toRemove = 2; toRemove != 0;)
    {
        const bool first = gameSettings(guest).seriesVariant != 0;
        const sreg32 number = sreg32(guest::direct<0x474c60, rosterSeedByte>(app, cpu, index));
        ++index;
        const sreg32 track = first ? number % 4 : number % 4 + 4;
        if (present[track])
        {
            present[track] = false;
            --toRemove;
        }
    }
    reg32 track = 0;
    for (reg32 stage = 0; stage < 7; ++track)
    {
        if (!present[track])
            continue;
        gameSettings(guest).knockout.tracks[stage] = std::uint16_t(track);
        ++stage;
    }
    return track;
}

x86::reg32 frontend::loadingRosterTournamentTracks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473ac0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterTournamentTracks(app, cpu); });
}

/* sub_473b70: the grid: eax entries, edx each one's rank by a table of 16, or 0 for as
 * they stand. */
x86::reg32 frontend::loada::rosterGridOrder(App* app, CPU& cpu, reg32 countWord, reg32 ranks)
{
    Guest guest(app, cpu);
    const sreg32 count = sreg32(countWord);
    sreg32 i = 0;
    if (ranks == 0)
    {
        for (; i < count; ++i)
            gameSettings(guest).gridSlots[reg32(i)] = reg32(i);
        return reg32(i);
    }
    reg32 eax = reg32(count);
    reg32 next = 0;
    for (sreg32 rank = 0; rank < 0x10; ++rank)
    {
        for (i = 0; i < count; ++i)
        {
            const reg32 who = guest.dword(entryAt(reg32(i)) + kEntryPlace);
            const sreg32 hers = signedWord(guest, ranks + who * 2);
            eax = reg32(hers);
            if (hers == rank)
                gameSettings(guest).gridSlots[reg32(i)] = next++;
        }
    }
    return eax;
}

x86::reg32 frontend::loadingRosterGridOrder(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473b70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const sreg32 count = sreg32(args.eax);
    const reg32 ranks = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterGridOrder(app, cpu, count, ranks); });
}

/* sub_473bf0: BuildRegularRaceRoster: the people, then the opponents, then the traffic. */
x86::reg32 frontend::loada::rosterBuildRegular(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    clearCounts(guest);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(playersDone), 8);
    copyDwords(guest, SETTINGS_ADDRESS(raceOptionComputerDrives), SETTINGS_ADDRESS(chosenOptions), 6);
    gameSettings(guest).raceOptionPursuit = 0;
    reg32 place = 0;
    const reg32 mode = gameSettings(guest).playMode;
    if (mode == 0)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, entryAt(place), place, 0u);
        ++place;
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
    }
    else if (mode == 1)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, reg32(SETTINGS_ADDRESS(entries)), 0u, 0u);
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, reg32(SETTINGS_ADDRESS(entries) + kEntrySize), 1u, 1u);
        place = 2;
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
    }
    else if (mode <= 4)
    {
        const reg32 view = reg32(signedWord(guest, kNetworkCounts + 4));
        gameSettings(guest).localPlayer = view;
        gameSettings(guest).viewedPlayer = view;
        for (sreg32 i = 0; i < signedWord(guest, kNetworkCounts + 2); ++i)
        {
            entryFromNetwork(app, cpu, guest, entryAt(place));
            ++place;
            gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        }
    }
    const reg32 strength = gameSettings(guest).trafficLevel;
    if (sreg32(strength) < 2)
        gameSettings(guest).opponentCount = strength != 0 ? 1 : 0;
    else
        gameSettings(guest).opponentCount = 8 - gameSettings(guest).humanCount;
    for (sreg32 i = 0; i < sreg32(gameSettings(guest).opponentCount); ++i)
    {
        guest::direct<0x473960, rosterEntryOpponent>(app, cpu, entryAt(place), place);
        ++place;
    }
    if (gameSettings(guest).noTraffic == 0)
    {
        const sreg32 room = sreg32(8 - (gameSettings(guest).opponentCount + gameSettings(guest).humanCount));
        gameSettings(guest).trafficCount = room < 0 ? 0 : reg32(room);
        for (sreg32 i = 0; i < sreg32(gameSettings(guest).trafficCount); ++i)
        {
            guest::direct<0x473a70, rosterEntryTraffic>(app, cpu, entryAt(place), place);
            ++place;
        }
    }
    const reg32 humans = gameSettings(guest).humanCount;
    gameSettings(guest).racerCount = humans;
    gameSettings(guest).entryCount = gameSettings(guest).opponentCount + humans + gameSettings(guest).trafficCount;
    const reg32 result = guest::direct<0x473b70, rosterGridOrder>(app, cpu, humans + gameSettings(guest).opponentCount, 0u);
    if (gameSettings(guest).playMode != 0)
    {
        gameSettings(guest).cheatTrack = 0;
        gameSettings(guest).raceFlags = 0;
    }
    return result;
}

x86::reg32 frontend::loadingRosterBuildRegular(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473bf0", 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterBuildRegular(app, cpu); });
}

/* sub_473ee0: the tournament's stage: its people, those of them out by now in the winner's
 * car, the opponents; the stage's track. */
x86::reg32 frontend::loada::rosterBuildTournament(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x24);
    gameSettings(guest).seriesRace = 1;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(raceOptionComputerDrives), 0x1c);
    if (gameSettings(guest).knockout.stage == 0)
    {
        guest::direct<0x473ac0, rosterTournamentTracks>(app, cpu);
        for (reg32 i = 0; i < 0x10; ++i)
            gameSettings(guest).knockout.places[i] = std::uint16_t(i);
        gameSettings(guest).knockout.stillIn[0] = 0x10;  // the game's loop goes one word past the table
        for (reg32 i = 0; i < 0x10; ++i)
            gameSettings(guest).knockout.stillIn[i] = 1;
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(playersDone), 8);
    }
    if (gameSettings(guest).seriesVariant == 1)
        chooseSides(app, cpu, guest, std::int32_t(gameSettings(guest).knockout.stage));
    else
        noSides(guest);
    clearCounts(guest);
    gameSettings(guest).laps = 2;

    reg32 place = 0;     // where in the table
    reg32 who = 0;       // whose place on the tournament's table
    sreg32 outOf = 0;    // how many of those are out
    const reg32 mode = gameSettings(guest).playMode;
    const auto person = [&](reg32 player) {
        const reg32 entry = entryAt(place);
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, entry, who, player);
        guest.setDword(entry + kEntryPlace, who);
        if (gameSettings(guest).knockout.stillIn[who] == 0)
        {
            ++outOf;
            guest::direct<0x4736c0, rosterEntryFinalist>(app, cpu, entry);
        }
    };
    if (mode == 0)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        person(0);
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        ++place;
        ++who;
    }
    else if (mode == 1)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        person(0);
        place = 1;
        who = 1;
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        person(1);
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        place = 2;
        who = 2;
    }
    else if (mode <= 4)
    {
        const reg32 view = reg32(signedWord(guest, kNetworkCounts + 4));
        gameSettings(guest).localPlayer = view;
        gameSettings(guest).viewedPlayer = view;
        for (sreg32 i = 0; i < signedWord(guest, kNetworkCounts + 2); ++i)
        {
            const sreg32 machine = sreg32(std::int16_t(fe::direct<kPlayerMachine, ea::machineOfPlayerNumber>(guest.app(), guest.cpu(), reg32(i))));
            if (machine == -1)
                continue;
            const reg32 entry = entryAt(place);
            guest.setDword(entry + kEntryKind, 1);
            guest.setDword(entry + kEntryPlace, reg32(machine));
            copyDwords(guest, entry + kEntryTuning, kDefaultTuning, 7);
            const reg32 flag = guest::direct<0x4395a0, carIsKept>(app, cpu, guest.dword(entry + kEntryCar));
            guest.setDword(entry + kEntryFlag8, flag);
            if (gameSettings(guest).knockout.stillIn[reg32(machine)] == 0)
            {
                ++outOf;
                guest::direct<0x4736c0, rosterEntryFinalist>(app, cpu, entry);
            }
            gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
            ++place;
        }
        who = 8;
    }
    const sreg32 others = sreg32(8 - gameSettings(guest).humanCount);
    for (sreg32 i = 0; i < others; ++i)
    {
        const bool still = gameSettings(guest).knockout.stillIn[who] != 0;
        guest::direct<0x473960, rosterEntryOpponent>(app, cpu, entryAt(place), who);
        ++who;
        if (still)
        {
            ++place;
            gameSettings(guest).opponentCount = gameSettings(guest).opponentCount + 1;
        }
        else
            ++outOf;
    }
    while (std::int32_t(gameSettings(guest).knockout.stage) > outOf)
    {
        gameSettings(guest).opponentCount = gameSettings(guest).opponentCount - 1;
        ++outOf;
    }
    const reg32 humans = gameSettings(guest).humanCount;
    gameSettings(guest).racerCount = humans;
    const reg32 racing = humans + gameSettings(guest).opponentCount;
    gameSettings(guest).entryCount = racing;
    guest::direct<0x473b70, rosterGridOrder>(app, cpu, racing, reg32(SETTINGS_ADDRESS(knockout.places)));
    const reg32 track = reg32(std::int32_t(gameSettings(guest).knockout.tracks[reg32(std::int32_t(gameSettings(guest).knockout.stage))]));
    gameSettings(guest).raceFlags = 0;
    gameSettings(guest).cheatTrack = 0;
    gameSettings(guest).track = track;
    return track;
}

x86::reg32 frontend::loadingRosterBuildTournament(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_473ee0", 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterBuildTournament(app, cpu); });
}

/* sub_474310: knockout: the stage's track, the people, the opponents. */
x86::reg32 frontend::loada::rosterBuildKnockout(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    constexpr reg32 kStageTrack = 0x55d004;
    gameSettings(guest).seriesRace = 1;
    gameSettings(guest).track = guest.dword(kStageTrack + 4 * reg32(std::int32_t(gameSettings(guest).tournament.stage)));
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(raceOptionComputerDrives), 0x1c);
    if (gameSettings(guest).tournament.stage == 0)
    {
        for (reg32 i = 0; i < 0x10; ++i)
            gameSettings(guest).tournament.places[i] = std::uint16_t(i);
        gameSettings(guest).tournament.raceScores[0] = 0x10;  // the game's loop goes one word past the table
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(playersDone), 8);
    }
    clearCounts(guest);
    gameSettings(guest).laps = sreg32(gameSettings(guest).playMode) > 1 ? 2 : 4;
    if (gameSettings(guest).seriesVariant == 1)
        chooseSides(app, cpu, guest, std::int32_t(gameSettings(guest).tournament.stage));
    else
        noSides(guest);

    reg32 place = 0;
    reg32 who = 0;
    const reg32 mode = gameSettings(guest).playMode;
    if (mode == 0)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, entryAt(place), who, 0u);
        ++who;
        ++place;
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
    }
    else if (mode == 1)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, reg32(SETTINGS_ADDRESS(entries)), 0u, 0u);
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, reg32(SETTINGS_ADDRESS(entries) + kEntrySize), 1u, 1u);
        who = 2;
        place = 2;
        gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
    }
    else if (mode <= 4)
    {
        const reg32 view = reg32(signedWord(guest, kNetworkCounts + 4));
        gameSettings(guest).localPlayer = view;
        gameSettings(guest).viewedPlayer = view;
        for (sreg32 i = 0; i < signedWord(guest, kNetworkCounts + 2); ++i)
        {
            const sreg32 machine = sreg32(std::int16_t(fe::direct<kPlayerMachine, ea::machineOfPlayerNumber>(guest.app(), guest.cpu(), reg32(i))));
            if (machine == -1)
                continue;
            const reg32 entry = entryAt(place);
            guest.setDword(entry + kEntryPlace, reg32(machine));
            entryFromNetwork(app, cpu, guest, entry);
            ++place;
            gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        }
        who = 8;
    }
    const sreg32 others = sreg32(8 - gameSettings(guest).humanCount);
    for (sreg32 i = 0; i < others; ++i)
    {
        guest::direct<0x473960, rosterEntryOpponent>(app, cpu, entryAt(place), who);
        gameSettings(guest).opponentCount = gameSettings(guest).opponentCount + 1;
        ++place;
        ++who;
    }
    const reg32 humans = gameSettings(guest).humanCount;
    gameSettings(guest).racerCount = humans;
    const reg32 racing = humans + gameSettings(guest).opponentCount;
    gameSettings(guest).entryCount = racing;
    const reg32 result = guest::direct<0x473b70, rosterGridOrder>(app, cpu, racing, reg32(SETTINGS_ADDRESS(tournament.places)));
    gameSettings(guest).raceFlags = 0;
    gameSettings(guest).cheatTrack = 0;
    return result;
}

x86::reg32 frontend::loadingRosterBuildKnockout(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_474310", 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterBuildKnockout(app, cpu); });
}

/* sub_474640: BuildHotPursuitRoster: who drives a police car decides who the rest are. */
x86::reg32 frontend::loada::rosterBuildHotPursuit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    clearCounts(guest);
    if (guest.byte(kRaceFlags) & kRaceFlagAllCops)
    {
        const reg32 now = fe::direct<0x4f22b0, watcom::clib::time>(app, cpu, 0u);   // time
        fe::direct<0x4eb220, watcom::clib::srand>(app, cpu, now);   // srand, of it
        gameSettings(guest).seriesRace = 1;
    }
    gameSettings(guest).raceOptionComputerDrives = 0;
    gameSettings(guest).raceOptionLaneVotes = 0;
    gameSettings(guest).raceOptionCones = 0;
    gameSettings(guest).raceOptionCopRadio = 0;
    gameSettings(guest).raceOptionHazard = 0;
    gameSettings(guest).raceOptionPursuit = gameSettings(guest).pursuitOption;
    gameSettings(guest).raceOptionSimulation = 0;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), SETTINGS_ADDRESS(playersDone), 8);

    const auto countKind = [&](bool police) {
        reg32& count = police ? gameSettings(guest).copCount : gameSettings(guest).humanCount;
        count = count + 1;
    };
    const auto markCop = [&](reg32 entry) { guest.setByte(entry + kEntryKind, guest.byte(entry + kEntryKind) | kEntryIsCop); };
    reg32 place = 0;
    const reg32 mode = gameSettings(guest).playMode;
    if (mode == 0)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        const reg32 entry = entryAt(0);
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, entry, 0u, 0u);
        const bool police = *fe::direct<0x4396b0, load::policeFlag>(app, cpu, gameSettings(guest).players[0].car) != 0;
        if (police)
        {
            markCop(entry);
            gameSettings(guest).pursuerCount = 0;
            gameSettings(guest).opponentCount = 6;
            gameSettings(guest).trafficCount = 1;
            gameSettings(guest).copCount = gameSettings(guest).copCount + 1;
        }
        else
        {
            gameSettings(guest).pursuerCount = 6;
            gameSettings(guest).trafficCount = 4;
            gameSettings(guest).opponentCount = 1;
            gameSettings(guest).humanCount = gameSettings(guest).humanCount + 1;
        }
        if (guest.byte(kRaceFlags) & kRaceFlagAllCops)
        {
            gameSettings(guest).trafficCount = 3;
            gameSettings(guest).noTraffic = 0;
            gameSettings(guest).pursuerCount = 2;
        }
        if (police)
        {
            gameSettings(guest).pursuitWinCount = gameSettings(guest).opponentCount;
            gameSettings(guest).pursuitOutCount = 1;
        }
        else
        {
            gameSettings(guest).pursuitWinCount = 0x64;
            gameSettings(guest).pursuitOutCount = gameSettings(guest).laps + 1;
        }
        place = 1;
    }
    else if (mode == 1)
    {
        gameSettings(guest).localPlayer = 0;
        gameSettings(guest).viewedPlayer = 0;
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, reg32(SETTINGS_ADDRESS(entries)), 0u, 0u);
        if (*fe::direct<0x4396b0, load::policeFlag>(app, cpu, gameSettings(guest).players[0].car) != 0)
        {
            countKind(true);
            markCop(SETTINGS_ADDRESS(entries));
        }
        else
            countKind(false);
        const reg32 second = entryAt(1);
        guest::direct<0x473710, rosterEntryHuman>(app, cpu, second, 1u, 1u);
        if (*fe::direct<0x4396b0, load::policeFlag>(app, cpu, gameSettings(guest).players[1].car) != 0)
        {
            markCop(second);
            countKind(true);
        }
        else
            countKind(false);
        place = 2;
        const reg32 skill = gameSettings(guest).laps + 1;
        if (gameSettings(guest).humanCount == 2)
        {
            gameSettings(guest).pursuitOutCount = skill;
            gameSettings(guest).opponentCount = 0;
            gameSettings(guest).trafficCount = 4;
            gameSettings(guest).pursuitWinCount = 0x64;
            gameSettings(guest).pursuerCount = 6;
        }
        else if (gameSettings(guest).copCount == 2)
        {
            gameSettings(guest).opponentCount = 6;
            gameSettings(guest).trafficCount = 0;
            gameSettings(guest).pursuitWinCount = 3;
            gameSettings(guest).pursuitOutCount = 1;
            gameSettings(guest).pursuerCount = 0;
        }
        else
        {
            gameSettings(guest).pursuitWinCount = skill;
            gameSettings(guest).pursuitOutCount = skill;
            gameSettings(guest).opponentCount = 0;
            gameSettings(guest).trafficCount = 6;
            gameSettings(guest).pursuerCount = 0;
        }
    }
    else if (mode <= 4)
    {
        const reg32 view = reg32(signedWord(guest, kNetworkCounts + 4));
        gameSettings(guest).localPlayer = view;
        gameSettings(guest).viewedPlayer = view;
        for (sreg32 i = 0; i < signedWord(guest, kNetworkCounts + 2); ++i)
        {
            const reg32 entry = entryAt(place);
            entryFromNetwork(app, cpu, guest, entry);
            if (*fe::direct<0x4396b0, load::policeFlag>(app, cpu, guest.dword(entry + kEntryCar)) != 0)
            {
                markCop(entry);
                countKind(true);
            }
            else
                countKind(false);
            ++place;
        }
        const reg32 cops = gameSettings(guest).copCount;
        const reg32 humans = gameSettings(guest).humanCount;
        if (cops == 0)
        {
            gameSettings(guest).trafficCount = gameSettings(guest).noTraffic == 0 && sreg32(humans) < 4 ? 1 : 0;
            const sreg32 room = sreg32(8 - gameSettings(guest).trafficCount - gameSettings(guest).humanCount);
            const sreg32 capped = room > 6 ? 6 : room;
            gameSettings(guest).pursuerCount = capped > 0 ? reg32(capped) : 0;
        }
        else if (humans == 0)
        {
            gameSettings(guest).trafficCount = gameSettings(guest).noTraffic == 0 && sreg32(cops) < 6 ? 1 : 0;
            const sreg32 room = sreg32(8 - gameSettings(guest).trafficCount - gameSettings(guest).copCount);
            const sreg32 capped = room > 6 ? 6 : room;
            gameSettings(guest).opponentCount = capped > 1 ? reg32(capped) : 1;
        }
        else if (gameSettings(guest).noTraffic == 0)
        {
            const sreg32 room = sreg32(8 - humans - cops);
            const sreg32 capped = room > 4 ? 4 : room;
            gameSettings(guest).trafficCount = capped > 0 ? reg32(capped) : 0;
        }
        const reg32 skill = gameSettings(guest).laps + 1;
        gameSettings(guest).pursuitWinCount = skill;
        gameSettings(guest).pursuitOutCount = skill;
    }
    for (sreg32 i = 0; i < sreg32(gameSettings(guest).opponentCount); ++i)
    {
        guest::direct<0x473960, rosterEntryOpponent>(app, cpu, entryAt(place), place);
        ++place;
    }
    for (sreg32 i = 0; i < sreg32(gameSettings(guest).pursuerCount); ++i)
    {
        const bool chasing = (guest.byte(kRaceFlags) & kRaceFlagAllCops) || (i & 3) == 0;
        guest::direct<0x473890, rosterEntryCop>(app, cpu, entryAt(place), place, chasing ? 1u : 0u);
        ++place;
    }
    if (gameSettings(guest).noTraffic == 0)
    {
        for (sreg32 i = 0; i < sreg32(gameSettings(guest).trafficCount); ++i)
        {
            guest::direct<0x473a70, rosterEntryTraffic>(app, cpu, entryAt(place), place);
            ++place;
        }
    }
    else
        gameSettings(guest).trafficCount = 0;
    const reg32 humans = gameSettings(guest).humanCount;
    const reg32 opponents = gameSettings(guest).opponentCount;
    const reg32 racers = humans + gameSettings(guest).copCount;
    gameSettings(guest).racerCount = racers;
    gameSettings(guest).entryCount = racers + gameSettings(guest).pursuerCount + opponents + gameSettings(guest).trafficCount;
    guest::direct<0x473b70, rosterGridOrder>(app, cpu, humans + opponents, 0u);
    gameSettings(guest).cheatTrack = 0;
    gameSettings(guest).raceFlags = gameSettings(guest).raceFlags & 0x10;
    return reg32(0);
}

x86::reg32 frontend::loadingRosterBuildHotPursuit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_474640", 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterBuildHotPursuit(app, cpu); });
}

/* sub_474c60: the race's number at eax on from its seed, 0 to 255. */
x86::reg32 frontend::loada::rosterSeedByte(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    return guest.byte(kSeedBytes + reg32(sreg32(gameSettings(guest).raceSeed + index) % 0x100));
}

x86::reg32 frontend::loadingRosterSeedByte(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_474c60", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterSeedByte(app, cpu, index); });
}

/* sub_474ca0: the race's cast, by the kind of race, seeded by the race's seed; the track's
 * records, the cars' ids. */
x86::reg32 frontend::loada::rosterBuild(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x3cu, 0xdu);
    fe::direct<0x43a580, load::classListsBuilt>(app, cpu);
    guest::direct<0x473340, rosterClearUsed>(app, cpu);
    fe::direct<0x4e0dee, ea::randomSeeded>(app, cpu, gameSettings(guest).raceSeed);
    gameSettings(guest).lastTickRaced = 0;
    static const reg32 kBuilders[4] = { 0x473bf0, 0x474310, 0x473ee0, 0x474640 };
    const reg32 kind = gameSettings(guest).raceType;
    if (kind <= 3)
        fx::callAt(guest, kBuilders[kind]);
    const sreg32 holder = sreg32(std::int16_t(
        fe::direct<0x4cef00, misc::recordsHolder>(app, cpu, reg32(std::int32_t(std::int16_t(gameSettings(guest).track))))));
    gameSettings(guest).recordsHolder = reg32(holder);
    gameSettings(guest).recordsHolderKept = reg32(holder);
    return guest::direct<0x4781b0, savedNameEntries>(app, cpu, reg32(SETTINGS_ADDRESS(fileSize)));
}

x86::reg32 frontend::loadingRosterBuild(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_474ca0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::rosterBuild(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The cars in a race
 * ------------------------------------------------------------------------- */

namespace
{

constexpr reg32 kCarEvents = 0x7dc;            // how many of up to 8, then 0x18 bytes each
constexpr reg32 kCarOfRoster = 0x220;          // the car's entry of the cast
constexpr reg32 kCarListFlags = 0x200;         // byte: which lists it joined
constexpr reg32 kCarSlice = 0x1c;
constexpr reg32 kCarLane = 0x638;

constexpr reg32 kRaceCarCount = 0x5efd9c;
constexpr reg32 kRaceCars = 0x5efac8;
constexpr reg32 kSliceCount = 0x5dd940;
constexpr reg32 kSlices = 0x5dd948;
constexpr reg32 kSliceSize = 36;

}

/* sub_4283d0: the race's cars loaded, a stop looked for around each step: 1 when stopped. */
x86::reg32 frontend::loada::raceCarSteps(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    constexpr reg32 kStopAsked = 0x55e88c;
    static const reg32 kSteps[4] = { 0x4b7c60, 0x42c590, 0x42c700, 0x42c7f0 };
    guest::direct<0x4a33d0, stopCheck>(app, cpu);
    if (guest.dword(kStopAsked) != 0)
        return 1;
    for (const reg32 step : kSteps)
    {
        fx::callAt(guest, step);
        guest::direct<0x4a33d0, stopCheck>(app, cpu);
        if (guest.dword(kStopAsked) != 0)
            return 1;
    }
    return 0;
}

x86::reg32 frontend::loadingRaceCarSteps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4283d0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::raceCarSteps(app, cpu); });
}

/* sub_4284e0: an event added to a car, while it has room for one: eax the car; edx, ebx,
 * ecx and the stack's first its fields, the second a time of 0 to 10 turned to 16.16, the
 * third the last field. */
x86::reg32 frontend::loada::carAddEvent(App* app, CPU& cpu, reg32 car, reg32 a, reg32 b, reg32 c, reg32 d, reg32 time, reg32 e)
{
    Guest guest(app, cpu);
    const reg32 count = guest.dword(car + kCarEvents);
    if (sreg32(count) >= 8)
        return car;
    const Float given = Float(floatOf(time));
    Float clamped;
    if (!isBelowOrEqual(fp::compare(0.0, given)))
        clamped = 0.0;
    else if (!isBelowOrEqual(fp::compare(given, doubleAt(guest, 0x537198))))
        clamped = 10.0;
    else
        clamped = given;
    const reg32 event = car + 0x7e0 + count * 0x18;
    guest.setDword(event, a);
    guest.setDword(event + 4, b);
    guest.setDword(event + 8, c);
    guest.setDword(event + 0xc, d);
    guest.setDword(event + 0x10, reg32(fp::toInteger<sreg32>(fp::mul(clamped, doubleAt(guest, 0x5371a0)))));
    guest.setDword(event + 0x14, e);
    guest.setDword(car + kCarEvents, count + 1);
    return count * 3;
}

x86::reg32 frontend::loadingCarAddEvent(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4284e0", 0xc, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax, a = args.edx, b = args.ebx, c = args.ecx;
    const reg32 d = stackArgument(app, cpu, 0), time = stackArgument(app, cpu, 1), e = stackArgument(app, cpu, 2);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carAddEvent(app, cpu, car, a, b, c, d, time, e); });
}

/* sub_4285f0: a car's block of driving state at +0x224 cleared. */
x86::reg32 frontend::loada::carClearDriving(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const reg32 block = car + 0x224;
    guest.setDword(block + 4, 0);
    guest.setDword(block + 8, 0);
    guest.setDword(block + 0xc, 0);
    guest.setDword(block + 0x10, 0);
    guest.setDword(block + 0x14, 0x200);
    for (reg32 at = 0x3c; at <= 0x68; at += 4)
        guest.setDword(block + at, 0);
    guest.setDword(block, 0);
    for (reg32 i = 1; i <= 8; ++i)
    {
        guest.setDword(block + 0x18 + 4 * i, 0);
        guest.setDword(block + 0x70 + 4 * i, 0);
    }
    guest.setDword(block + 0x94, 0);
    guest.setDword(block + 0x98, 0);
    guest.setDword(block + 0x9c, 0);
    guest.setDword(block + 0xa0, 0);
    for (reg32 at = 0xa8; at <= 0xbc; at += 4)
        guest.setDword(block + at, 0);
    return block;
}

x86::reg32 frontend::loadingCarClearDriving(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4285f0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carClearDriving(app, cpu, car); });
}

/* sub_428700: a car put on the race's lists its entry's kind says. */
x86::reg32 frontend::loada::carJoinLists(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    constexpr reg32 kAllB = 0x5efcc8, kAllC = 0x5efc88;
    constexpr reg32 kHumansList = 0x5efa48, kHumansCount = 0x5efda4;
    constexpr reg32 kCopsListA = 0x5efa88, kCopsCount = 0x5efd88;
    constexpr reg32 kCopsListB = 0x5efc08, kCopsCountB = 0x553078;
    constexpr reg32 kRacersList = 0x5efc48, kRacersCount = 0x5efda0;
    constexpr reg32 kPeopleList = 0x5efd08, kPeopleCount = 0x553074;
    constexpr reg32 kOthersList = 0x5efb08, kOthersCount = 0x5efd98;
    constexpr reg32 kAiList = 0x5efb88, kAiCount = 0x5efd94;
    constexpr reg32 kTrafficList = 0x5efb48, kTrafficCount = 0x5efd8c;
    constexpr reg32 kCarName = 0x1e4, kCarNumber = 0x1f0, kCarHumanNumber = 0x1f4, kCarRacerNumber = 0x1f8;
    const auto setFlag = [&](std::uint8_t flag) {
        guest.setByte(car + kCarListFlags, guest.byte(car + kCarListFlags) | flag);
    };
    const auto addTo = [&](reg32 list, reg32 count) {
        const reg32 n = guest.dword(count);
        guest.setDword(list + 4 * n, car);
        guest.setDword(count, n + 1);
        return n;
    };

    const std::uint32_t kinds = guest.dword(guest.dword(car + kCarOfRoster) + kEntryKind);
    const reg32 number = guest.dword(kRaceCarCount);
    guest.setDword(kRaceCars + 4 * number, car);
    guest.setDword(kAllB + 4 * number, car);
    guest.setDword(kAllC + 4 * number, car);
    guest.setDword(car + kCarListFlags, 0);
    guest.setDword(car + kCarHumanNumber, 0xffffffffu);
    guest.setDword(car + kCarNumber, number);
    guest.setDword(car + kCarRacerNumber, 0xffffffffu);
    guest.setDword(kRaceCarCount, number + 1);
    const reg32 folder = *fe::direct<0x4392b0, load::folderOf>(app, cpu, guest.dword(guest.dword(car + kCarOfRoster) + kEntryCar));
    guest.copyString(car + kCarName, folder);

    if (kinds & 1)
    {
        const reg32 human = guest.dword(kHumansCount);
        guest.setDword(car + kCarHumanNumber, human);
        guest.setDword(kHumansList + 4 * human, car);
        guest.setDword(kHumansCount, human + 1);
        setFlag(4);
        if (kinds & 8)
        {
            addTo(kCopsListA, kCopsCount);
            addTo(kCopsListB, kCopsCountB);
            setFlag(0x20);
        }
        else
        {
            guest.setDword(car + kCarRacerNumber, addTo(kRacersList, kRacersCount));
            addTo(kPeopleList, kPeopleCount);
            setFlag(1);
        }
    }
    if (kinds & 2)
    {
        addTo(kOthersList, kOthersCount);
        setFlag(2);
        addTo(kAiList, kAiCount);
        setFlag(8);
        guest.setDword(car + kCarRacerNumber, addTo(kRacersList, kRacersCount));
        setFlag(1);
    }
    if (kinds & 4)
    {
        addTo(kOthersList, kOthersCount);
        setFlag(2);
        addTo(kTrafficList, kTrafficCount);
        setFlag(0x10);
    }
    if ((kinds & 8) && !(kinds & 1))
    {
        addTo(kOthersList, kOthersCount);
        setFlag(2);
        addTo(kCopsListA, kCopsCount);
        setFlag(0x20);
    }
    const reg32 roster = guest.dword(car + kCarOfRoster);
    if (guest.dword(0x6fdc30) == guest.dword(roster + kEntryCar) && gameSettings(guest).raceType == 2)
        setFlag(0x80);
    return roster;
}

x86::reg32 frontend::loadingCarJoinLists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428700", 0, 0 };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carJoinLists(app, cpu, car); });
}

/* sub_428970: a car's state for the start of a race, or after a reset. */
x86::reg32 frontend::loada::carReset(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    for (const reg32 at : { 0xe4u, 0xe8u, 0xecu, 0x4f8u, 0x4fcu, 0x500u, 0x4e0u, 0x4e4u, 0x4e8u, 0x524u })
        guest.setDword(car + at, 0);
    // The game's code hands it 0x20 in edx too, which it does not read.
    guest.setDword(car + kCarLane,
                   fe::direct<0x40e2b0, aiplan::roadSlice>(app, cpu, guest.dword(car + kCarSlice), floatOf(guest.dword(car + 0xbc))));
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), car + 0x168, 0x20);
    guest.setWord(car + 0x164, 1);
    guest.setDword(car + 0x168, 0);
    guest.setDword(car + 0x16c, 0);
    guest.setByte(car + 0x8b, 0);
    guest.setDword(car + 0x184, 0);
    for (const reg32 at : { 0x99cu, 0x9a0u, 0x8d4u, 0x8d0u, 0x91cu, 0x920u, 0x924u, 0x928u, 0x92cu, 0x8c8u })
        guest.setDword(car + at, 0);
    guest.setDword(car + 0x174, guest.dword(0x5efdc8));
    guest.setByte(car + 0x621, guest.byte(car + 0x621) & 0xbf);
    fe::direct<0x4aa79f, handling::resetDrive>(app, cpu, car);
    fe::direct<0x4015b0, airace::carStoppedIfCopNear>(guest.app(), guest.cpu(), car);
    fe::direct<0x42cc20, raceload::lapsCounted>(app, cpu, car);
    fe::direct<0x42ce10, raceload::setRaceDistance>(app, cpu, car);
    return 0;
}

x86::reg32 frontend::loadingCarReset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428970", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carReset(app, cpu, car); });
}

/* sub_428ad0: eax a car, edx a slice, the stack three numbers of which the first and third
 * are how far along each of the slice's two vectors: whether the car can be set down there.
 * What it hands on through its frame is laid out where the game's code has it. */
x86::reg32 frontend::loada::carPlaceIsClear(App* app, CPU& cpu, reg32 car, reg32 slice, reg32 across, reg32 up, reg32 right)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc4);
    static_cast<void>(up);   // the second of the three: not read
    const reg32 frame = guest.frame();
    const reg32 place = frame;   // the place's record: its slice at +0x10, what it hit at +0x14, +0x18 and +0x1c
    const reg32 sum = frame + 0x78;
    const reg32 forward = frame + 0x84;
    const reg32 centre = frame + 0x90;
    const reg32 side = frame + 0x9c;

    const reg32 lane = fe::direct<0x40e2b0, aiplan::roadSlice>(app, cpu, slice, floatOf(across));
    if (fe::direct<kSliceHasLane, aiplan::hasLane>(guest.app(), guest.cpu(), slice, lane) == 0)
        return 0;
    for (sreg32 i = 0; i < sreg32(guest.dword(kRaceCarCount)); ++i)
    {
        const reg32 other = guest.dword(kRaceCars + 4 * reg32(i));
        const reg32 otherSlice = guest.dword(other + kCarSlice);
        const reg32 otherLane = guest.dword(other + kCarLane);
        if (other == car || guest.byte(other + 0x88) != 0 || guest.byte(other + 0x89) == 0)
            continue;
        if (otherLane != lane && lane != otherLane - 1 && otherLane + 1 != lane)
            continue;
        sreg32 before = sreg32(otherSlice) - 1;
        if (before < 0)
            before += sreg32(guest.dword(kSliceCount));
        sreg32 after = sreg32(otherSlice) + 1;
        if (after >= sreg32(guest.dword(kSliceCount)))
            after -= sreg32(guest.dword(kSliceCount));
        if (slice == otherSlice || sreg32(slice) == before || sreg32(slice) == after)
        {
            return 0;
        }
    }
    // The slice's own numbers, the signed top byte of three dwords each, scaled up.
    const reg32 record = guest.dword(kSlices) + slice * kSliceSize;
    const auto byteAt = [&](reg32 at) { return Float(sreg32((reg32(sreg32(guest.dword(record + at)) >> 24)) << 9)); };
    const Float scale = doubleAt(guest, 0x5371a8);
    guest.setFloat(forward, float(fp::mul(byteAt(0x15), scale)));
    guest.setFloat(forward + 4, float(fp::mul(byteAt(0x16), scale)));
    guest.setFloat(forward + 8, float(fp::mul(byteAt(0x17), scale)));
    guest.setFloat(side, float(fp::mul(byteAt(0x11), scale)));
    guest.setFloat(side + 4, float(fp::mul(byteAt(0x12), scale)));
    guest.setFloat(side + 8, float(fp::mul(scale, byteAt(0x13))));
    guest.setFloat(centre, float(fp::mul(Float(sreg32(guest.dword(record))), scale)));
    guest.setFloat(centre + 4, float(fp::mul(Float(sreg32(guest.dword(record + 4))), scale)));
    guest.setFloat(centre + 8, float(fp::mul(Float(sreg32(guest.dword(record + 8))), scale)));
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, forward, floatOf(across), forward);
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, side, floatOf(right), side);
    const Float x = fp::add(fp::add(floatIn(guest, forward), floatIn(guest, side)), floatIn(guest, centre));
    const Float y = fp::add(fp::add(floatIn(guest, forward + 4), floatIn(guest, side + 4)), floatIn(guest, centre + 4));
    const Float z = fp::add(fp::add(floatIn(guest, forward + 8), floatIn(guest, side + 8)), floatIn(guest, centre + 8));
    guest.setFloat(sum, float(x));
    guest.setFloat(sum + 4, float(y));
    guest.setFloat(sum + 8, float(z));
    fe::direct<0x4205a0, trackdata::snapPlace>(app, cpu, sum, place, 1u, slice);
    if (guest.dword(place + 0x14) == 0 || guest.dword(place + 0x18) == 0)
        return 0;
    const reg32 surface = fe::direct<0x4207e0, trackdata::placeSurface>(app, cpu, place, 1u);
    if ((surface & 0x40) || (surface & 0x80) || !(surface & 0x3f) || (surface & 0x3f) == 0xe)
        return 0;
    reg32 height;
    if (guest.dword(place + 0x14) != 0)
        height = reg32(sreg32(guest.dword(guest.dword(place + 0x1c))) >> 0x10) * 2;
    else
    {
        const reg32 slices = guest.dword(kSlices);
        height = reg32(sreg32(guest.dword(slices + guest.dword(place + 0x10) * kSliceSize + 0xe)) >> 0x18) << 9;
    }
    return isBelow(fp::compare(Float(sreg32(height)), doubleAt(guest, 0x5371b0))) ? 0 : 1;
}

x86::reg32 frontend::loadingCarPlaceIsClear(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428ad0", 0xc, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax, slice = args.edx;
    const reg32 across = stackArgument(app, cpu, 0), right = stackArgument(app, cpu, 2);
    const reg32 up = stackArgument(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carPlaceIsClear(app, cpu, car, slice, across, up, right); });
}

/* sub_428d60: eax a car, edx a slice: the car set down where there is room near it: first
 * on the slice as it is, then for ten slices on, in or against the way of the race, each
 * tried lane by lane on both sides. */
x86::reg32 frontend::loada::carPlaceOnRoad(App* app, CPU& cpu, reg32 car, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x44);
    const reg32 where = guest.frame();
    const Float toFixed = doubleAt(guest, 0x5371c8);
    // The first try's three numbers: twelve zero bytes kept in the code's own section.
    reg32 up = guest.dword(0x428464), along = guest.dword(0x428468);

    const auto putDown = [&](reg32 slice, reg32 x, reg32 y, reg32 z) {
        guest.setDword(where, x);
        guest.setDword(where + 4, y);
        guest.setDword(where + 8, z);
        return fe::direct<0x4a0aa0, frontend::simb::placeOnRoad>(app, cpu, car, slice, where,
                                                                 gameSettings(guest).backwards);
    };
    const auto fixedOf = [&](reg32 bits) { return reg32(fp::toInteger<sreg32>(fp::mul(Float(floatOf(bits)), toFixed))); };
    const auto foundAt = [&](reg32 slice, reg32 across) {
        const reg32 x = fixedOf(across);
        const reg32 y = fixedOf(up);
        const reg32 z = fixedOf(along);
        return putDown(slice, x, y, z);
    };

    if (guest::direct<0x428ad0, carPlaceIsClear>(app, cpu, car, wanted, guest.dword(0x428460), up, along) == 0)
    {
        reg32 slice = wanted;
        for (sreg32 tries = 9; tries != -1; --tries)
        {
            const reg32 record = guest.dword(kSlices) + slice * kSliceSize;
            const reg32 widths = guest.byte(record + 0x13);
            const sreg32 lanesLeft = sreg32((widths >> 4) & 0xf);
            const sreg32 lanesRight = sreg32(widths & 0xf);
            const Float toMetres = doubleAt(guest, 0x5371b8);
            const float laneLeft = float(fp::mul(Float(sreg32(reg32(guest.byte(record + 0x17)) << 0xf)), toMetres));
            const float laneRight = float(fp::mul(toMetres, Float(sreg32(reg32(guest.byte(record + 0x1b)) << 0xf))));
            const Float half = doubleAt(guest, 0x5371c0);
            const sreg32 lanes = lanesLeft > lanesRight ? lanesLeft : lanesRight;
            for (sreg32 i = 0; i < lanes; ++i)
            {
                if (i <= lanesRight)
                {
                    const Float width = Float(laneRight);
                    const reg32 across = bitsOf(float(fp::add(fp::mul(Float(i), width), fp::mul(width, half))));
                    if (guest::direct<0x428ad0, carPlaceIsClear>(app, cpu, car, slice, across, up, along) != 0)
                        return foundAt(slice, across);
                }
                if (i <= lanesLeft)
                {
                    const Float width = Float(laneLeft);
                    const reg32 across = bitsOf(float(fp::sub(fp::mul(-Float(i), width), fp::mul(width, half))));
                    if (guest::direct<0x428ad0, carPlaceIsClear>(app, cpu, car, slice, across, up, along) != 0)
                        return foundAt(slice, across);
                }
            }
            sreg32 next;
            if (gameSettings(guest).backwards != 0)
            {
                next = sreg32(slice) - 1;
                if (next < 0)
                    next += sreg32(guest.dword(kSliceCount));
            }
            else
            {
                next = sreg32(slice) + 1;
                if (next >= sreg32(guest.dword(kSliceCount)))
                    next -= sreg32(guest.dword(kSliceCount));
            }
            slice = reg32(next);
            up = 0;
            along = 0;
        }
    }
    return putDown(wanted, 0, 0, 0);
}

x86::reg32 frontend::loadingCarPlaceOnRoad(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428d60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax, wanted = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carPlaceOnRoad(app, cpu, car, wanted); });
}

/* ---------------------------------------------------------------------------
 * The car list
 * ------------------------------------------------------------------------- */

namespace
{

constexpr reg32 kCarListHead = 0x554f70;       // the kept cars: each record's first word the next
constexpr reg32 kCarFlags = 0x380;
constexpr std::uint8_t kCarKept = 8;

}

/* sub_438b70: the car whose next is eax, or 0. */
x86::reg32 frontend::loada::carBefore(App* app, CPU& cpu, reg32 next)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    reg32 car = fe::direct<0x438af0, load::firstListedCar>(app, cpu);
    while (car != 0 && next != guest.dword(car))
        car = fe::direct<0x438b10, load::nextListedCar>(app, cpu);
    return car;
}

x86::reg32 frontend::loadingCarBefore(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_438b70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 next = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carBefore(app, cpu, next); });
}

/* sub_438f60: eax a car's number, edx which of its four stats, a byte each; 0 past them. */
x86::reg32 frontend::loada::carStat(App* app, CPU& cpu, reg32 number, reg32 which)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, number);
    if (record == 0 || which > 3)
        return 0;
    return guest.byte(record + 0x38f + which);
}

x86::reg32 frontend::loadingCarStat(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_438f60", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 number = args.eax, which = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carStat(app, cpu, number, which); });
}

/* sub_439510: the three words at +0x394 of a car's record. */
x86::reg32 frontend::loada::carExtras(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return fe::direct<0x439a50, load::recordAt>(app, cpu, number) + 0x394;
}

x86::reg32 frontend::loadingCarExtras(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439510", 0, 0, nullptr, 0, 0 };
    const reg32 number = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carExtras(app, cpu, number); });
}

/* sub_439530: eax an id's text: 1 for a car with neither of two marks. */
x86::reg32 frontend::loada::carIdIsPlain(App* app, CPU& cpu, reg32 id)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 number = fe::direct<0x439990, load::placeById>(app, cpu, id);
    if (*fe::direct<0x439570, load::flag40Of>(app, cpu, number) != 0)
        return 0;
    if (*fe::direct<0x439680, load::bonusFlagOf>(app, cpu, number) != 0)
        return 0;
    return 1;
}

x86::reg32 frontend::loadingCarIdIsPlain(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439530", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 id = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carIdIsPlain(app, cpu, id); });
}

/* sub_4395a0: a car's kept mark, 8 or 0. */
x86::reg32 frontend::loada::carIsKept(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest.dword(fe::direct<0x439a50, load::recordAt>(app, cpu, number) + kCarFlags) & 8;
}

x86::reg32 frontend::loadingCarIsKept(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4395a0", 0, 0, nullptr, 0, 0 };
    const reg32 number = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carIsKept(app, cpu, number); });
}

/* sub_4395e0: a car's mark 2. */
x86::reg32 frontend::loada::carFlag2(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest.dword(fe::direct<0x439a50, load::recordAt>(app, cpu, number) + kCarFlags) & 2;
}

x86::reg32 frontend::loadingCarFlag2(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4395e0", 0, 0, nullptr, 0, 0 };
    const reg32 number = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carFlag2(app, cpu, number); });
}

/* sub_439710: a byte of a car's record, at +0x3a0. */
x86::reg32 frontend::loada::carByte3a0(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest.byte(fe::direct<0x439a50, load::recordAt>(app, cpu, number) + 0x3a0);
}

x86::reg32 frontend::loadingCarByte3a0(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439710", 0, 0, nullptr, 0, 0 };
    const reg32 number = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carByte3a0(app, cpu, number); });
}

/* sub_439a10: the first player in the car at eax on the list at 0x554f84. */
x86::reg32 frontend::loada::playerToListedCar(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    fe::direct<0x438ad0, load::makeId>(app, cpu, reg32(SETTINGS_ADDRESS(players[0].carId)), guest.dword(0x554f84 + 4 * index));
    const reg32 number = fe::direct<0x439990, load::placeById>(app, cpu, reg32(SETTINGS_ADDRESS(players[0].carId)));
    gameSettings(guest).players[0].car = number;
    const reg32 kind = *fe::direct<0x439730, load::classOfCar>(app, cpu, number);
    gameSettings(guest).players[0].carClass = kind;
    return kind;
}

x86::reg32 frontend::loadingPlayerToListedCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439a10", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::playerToListedCar(app, cpu, index); });
}

/* sub_439b70: every kept car freed, its folder removed first when it says so. */
x86::reg32 frontend::loada::freeCars(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    for (reg32 car = guest.dword(kCarListHead); car != 0; car = guest.dword(kCarListHead))
    {
        guest.setDword(kCarListHead, guest.dword(car));
        if (guest.byte(car + kCarFlags) & kCarKept)
            guest::direct<0x43b2a0, carRemoveFolder>(app, cpu, car);
        guest::direct<0x439bb0, carFreeData>(app, cpu, car);
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, car);
    }
    guest.setDword(kCarListHead, 0);
    return 0;
}

x86::reg32 frontend::loadingFreeCars(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439b70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::freeCars(app, cpu); });
}

/* sub_439bb0: a car's block freed: -1 for no car. */
x86::reg32 frontend::loada::carFreeData(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    if (car == 0)
        return 0xffffffffu;
    guest::direct<0x439bf0, carFreeBlock>(app, cpu, car + 4);
    return 0;
}

x86::reg32 frontend::loadingCarFreeData(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439bb0", 0, 0, nullptr, 0, 0 };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carFreeData(app, cpu, car); });
}

/* sub_439bf0: the block at eax's +4 freed, if there is one. */
x86::reg32 frontend::loada::carFreeBlock(App* app, CPU& cpu, reg32 holder)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 block = guest.dword(holder + 4);
    if (block != 0)
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    return 0;
}

x86::reg32 frontend::loadingCarFreeBlock(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_439bf0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 holder = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carFreeBlock(app, cpu, holder); });
}

/* sub_43a4d0: qsort's comparison of two cars by their names (strcmp's answer). */
x86::reg32 frontend::loada::carCompareNames(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    const reg32 second = guest.dword(b) + 0x10;
    return fe::direct<0x4ee310, watcom::clib::strcmp>(guest.app(), guest.cpu(), guest.dword(a) + 0x10, second);
}

x86::reg32 frontend::loadingCarCompareNames(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43a4d0", 0, 0, nullptr, 0, 0 };
    const reg32 a = args.eax, b = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carCompareNames(app, cpu, a, b); });
}

/* sub_43a6b0: the number of the car at eax in the class at edx; 0 for none.  Leaves edx at
 * the class's row. */
x86::reg32 frontend::loada::carInClass(App* app, CPU& cpu, reg32 index, reg32 wantedClass)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 row = wantedClass * 200;
    const reg32 record = guest.dword(row + kClassCars + index * 4);
    if (record == 0)
        return reg32(0);
    return fe::direct<0x439990, load::placeById>(app, cpu, record + kClassCarIdOffset);
}

x86::reg32 frontend::loadingCarInClass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43a6b0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 index = args.eax, wantedClass = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carInClass(app, cpu, index, wantedClass); });
}

namespace
{

/* sub_43a6e0's lists: which cars each of the two choosers may show, a word each. */
constexpr reg32 kOpponentShown = 0x55519a;     // four classes' words, then a car's
constexpr reg32 kCarsOfOpponent = 0x5551a2;
constexpr reg32 kCarsShown = 0x55500a;
constexpr reg32 kCarLockedColourA = 0x55506c;
constexpr reg32 kCarLockedColourB = 0x555214;
constexpr std::uint32_t kLockedColour = 0x908080;

/* What the filter goes on with: its registers, the ones no save keeps. */
struct FilterLeft
{
    reg32 eax;
    reg32 edx;
};

}

/* sub_43a6e0: eax which cars the car lists offer, edx nonzero to move the choices now to one
 * that is offered.  What the last loop or call left in eax and edx stays there. */
x86::reg32 frontend::loada::carFilterLists(App* app, CPU& cpu, reg32 flags, reg32 pick)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const sreg32 count = sreg32(fe::direct<0x438bd0, load::carsListed>(app, cpu) - guest.dword(0x554f50));
    FilterLeft left{ reg32(count) * 2 + 8, guest.dword(0x554f50) };
    const auto finish = [&]() {
        return left.eax;
    };
    const auto memset1 = [&](reg32 at, reg32 bytes) {
        left.eax = fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), at, 1, bytes);
        };
    if (flags & 2)
    {
        if (flags & 4)
            memset1(kOpponentShown, reg32(count) * 2 + 8);
        if (flags & 8)
            memset1(kCarsShown, reg32(count) * 2);
        return finish();
    }
    if (flags & 1)
    {
        if (flags & 4)
            memset1(kOpponentShown, reg32(count) * 2 + 8);
        if (flags & 8)
            memset1(kCarsShown, reg32(count) * 2);
        left.edx = 0;
        for (sreg32 i = 0; i < count; ++i, left.edx = reg32(i))
        {
            const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, reg32(i));
            left.eax = fe::direct<0x439c10, load::bonusLocked>(app, cpu, record);   // of the record, in eax
            if (left.eax != 0)
            {
                guest.setWord(kCarsShown + 2 * reg32(i), 0);
                guest.setDword(kCarLockedColourA + 4 * reg32(i), kLockedColour);
                guest.setWord(kCarsOfOpponent + 2 * reg32(i), 0);
                guest.setDword(kCarLockedColourB + 4 * reg32(i), kLockedColour);
            }
        }
        return finish();
    }
    // The car's police flag.
    const auto isPolice = [&](reg32 car) { return *fe::direct<0x4396b0, load::policeFlag>(app, cpu, car); };
    /* One pass over the cars for each of the two lists a flag asks for: `off` on a car's
     * number says it goes; on the second list, `alsoIfNotPolice` asks for that as well. */
    const auto filter = [&](std::uint32_t flag, auto off, bool alsoIfNotPolice) {
        if (!(flags & flag))
            return;
        if (flags & 4)
        {
            left.edx = 0;
            for (sreg32 i = 0; i < count; ++i, left.edx = reg32(i))
                if (off(reg32(i)))
                    guest.setWord(kCarsOfOpponent + 2 * reg32(i), 0);
        }
        if (flags & 8)
        {
            left.edx = 0;
            for (sreg32 i = 0; i < count; ++i, left.edx = reg32(i))
            {
                if (!off(reg32(i)))
                    continue;
                if (alsoIfNotPolice)
                {
                    left.eax = isPolice(reg32(i));
                    if (left.eax != 0)
                        continue;
                }
                guest.setWord(kCarsShown + 2 * reg32(i), 0);
                left.eax = 0;
            }
        }
    };
    const auto whenZero = [&](auto ask) {
        return [&, ask](reg32 car) {
            left.eax = ask(car);
            return left.eax == 0;
        };
    };
    const auto whenSet = [&](auto ask) {
        return [&, ask](reg32 car) {
            left.eax = ask(car);
            return left.eax != 0;
        };
    };
    const auto ofClass = [&](reg32 wanted) {
        return [&, wanted](reg32 car) {
            left.eax = *fe::direct<0x439730, load::classOfCar>(app, cpu, car);
            return left.eax == wanted;
        };
    };
    filter(0x10, ofClass(0), true);
    filter(0x20, ofClass(1), true);
    filter(0x40, ofClass(2), true);
    filter(0x40000, [&](reg32 car) {
        left.eax = *fe::direct<0x4395c0, load::flag4Of>(app, cpu, car);
        if (left.eax != 0)
            return true;
        left.eax = *fe::direct<0x439610, load::numericIdOf>(app, cpu, car);
        return left.eax == 0;
    }, false);
    const auto artExists = [&](reg32 car) { return fe::direct<0x4530c0, frontend::menub::carArtExists>(app, cpu, car); };
    const auto hasSlides = [&](reg32 car) { return fe::direct<0x453200, frontend::showcase::hasSlides>(app, cpu, car); };
    const auto hasHistory = [&](reg32 car) { return fe::direct<0x453290, frontend::showcase::hasHistory>(app, cpu, car); };
    const auto isBonus = [&](reg32 car) { return *fe::direct<0x439680, load::bonusFlagOf>(app, cpu, car); };
    const auto hasNumeric = [&](reg32 car) { return *fe::direct<0x439610, load::numericIdOf>(app, cpu, car); };
    const auto isMaker = [&](reg32 car) { return *fe::direct<0x4396e0, load::makerFlag>(app, cpu, car); };
    const auto flag2 = [&](reg32 car) { return guest::direct<0x4395e0, carFlag2>(app, cpu, car); };
    filter(0x80000, whenZero(artExists), false);
    filter(0x100000, whenZero(hasSlides), false);
    filter(0x200000, whenZero(hasHistory), false);
    filter(0x1000, whenSet(isBonus), false);
    filter(0x2000, whenSet(isMaker), false);
    filter(0x4000, whenSet(isPolice), false);
    filter(0x8000, whenZero(isPolice), false);
    filter(0x10000, whenZero(hasNumeric), false);
    filter(0x20000, whenZero(flag2), false);

    if (flags & 0x80)
    {
        left.edx = 0;
        guest.setWord(kOpponentShown, 0);
    }
    if (flags & 0x100)
        guest.setWord(kOpponentShown + 2, 0);
    if (flags & 0x200)
    {
        left.eax = 0;
        guest.setWord(kOpponentShown + 4, 0);
    }
    if (flags & 0x400)
    {
        left.edx = 0;
        guest.setWord(kOpponentShown + 6, 0);
    }
    if (flags & 0x800)
    {
        left.edx = 0;
        while (sreg32(left.edx) < count)
        {
            ++left.edx;
            guest.setWord(kCarsOfOpponent - 2 + 2 * left.edx, 0);
        }
    }
    if (pick == 0)
        return finish();

    /* Each choice moved on to the next that is shown, or back round to itself. */
    const auto moveOn = [&](reg32& choice, reg32 shown, sreg32 span, bool yields) {
        const reg32 start = choice;
        for (;;)
        {
            if (yields)
            {
                // The port's yield at these two loops.
                app->unlockContext(cpu);
                win32::Thread::sleep(0);
                app->lockContext(cpu);
            }
            left.eax = choice;
            if (guest.word(left.eax * 2 + shown) != 0)
                return;
            const sreg32 next = sreg32(left.eax + 1);
            left.eax = reg32(next / span);
            left.edx = reg32(next % span);
            choice = left.edx;
            if (start == left.edx)
                return;
        }
    };
    moveOn(gameSettings(guest).opponentClass, kOpponentShown, count + 4, false);
    moveOn(gameSettings(guest).players[0].car, kCarsShown, count, true);
    moveOn(gameSettings(guest).players[1].car, kCarsShown, count, true);
    return finish();
}

x86::reg32 frontend::loadingCarFilterLists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43a6e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const std::uint32_t flags = args.eax;
    const reg32 pick = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carFilterLists(app, cpu, flags, pick); });
}

/* sub_43b2a0: a kept car's folder removed: its name under the kept cars' folder. */
x86::reg32 frontend::loada::carRemoveFolder(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x140);
    const reg32 path = guest.frame();
    guest.copyString(path, 0x7a28b4);
    fe::direct<0x438340, load::slashAdded>(app, cpu, path);
    guest.copyString(path + guest.length(path), car + 0x110);
    fe::direct<0x4389d0, load::folderRemoved>(app, cpu, path);
    return 0;
}

x86::reg32 frontend::loadingCarRemoveFolder(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43b2a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::carRemoveFolder(app, cpu, car); });
}

/* sub_43b390: every kept car off the list and freed, and the list read again. */
x86::reg32 frontend::loada::removeKeptCars(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    reg32 car = fe::direct<0x438af0, load::firstListedCar>(app, cpu);
    while (car != 0)
    {
        if (!(guest.byte(car + kCarFlags) & kCarKept))
        {
            car = fe::direct<0x438b10, load::nextListedCar>(app, cpu);
            continue;
        }
        const reg32 before = guest::direct<0x438b70, carBefore>(app, cpu, car);
        if (before != 0)
            guest.setDword(before, guest.dword(car));
        else
            guest.setDword(kCarListHead, guest.dword(car));
        guest::direct<0x439bb0, carFreeData>(app, cpu, car);
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, car);
        car = fe::direct<0x438af0, load::firstListedCar>(app, cpu);
    }
    fe::direct<0x43bcf0, frontend::sime::removeNumberedFolders>(app, cpu);
    fe::direct<0x439d90, load::loadCars>(app, cpu);
    return 0;
}

x86::reg32 frontend::loadingRemoveKeptCars(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43b390", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::removeKeptCars(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * Kept files
 * ------------------------------------------------------------------------- */

/* sub_4516a0: whether the current drive has eax bytes free. */
x86::reg32 frontend::loada::diskHasRoom(App* app, CPU& cpu, reg32 wantedWord)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x118);
    const sreg32 wanted = sreg32(wantedWord);
    const reg32 path = guest.frame();
    const reg32 free = path + 0xfc;   // what getdiskfree fills: three words multiplied at +4, +8, +0xc
    fe::direct<0x4f0730, watcom::clib::getcwd>(app, cpu, path, 0xfau);
    if (fe::direct<0x4f07d0, watcom::clib::getdiskfree>(app, cpu, reg32(guest.byte(path)) - 0x40, free) != 0)
        return 0;
    const sreg32 room = sreg32(guest.dword(free + 4) * guest.dword(free + 0xc) * guest.dword(free + 8));
    return room < wanted ? 0 : 1;
}

x86::reg32 frontend::loadingDiskHasRoom(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4516a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const sreg32 wanted = sreg32(args.eax);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::diskHasRoom(app, cpu, wanted); });
}

/* sub_452a00: eax to, edx from: a copy of a file in pieces of 512 bytes.  -1 for a file not
 * opened (the other left open, as the game has it), 1 for a piece not written, else 0. */
x86::reg32 frontend::loada::copyFile(App* app, CPU& cpu, reg32 to, reg32 from)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x21c);
    constexpr reg32 kFopen = 0x4edfe8;
    constexpr reg32 kRead = 0x539604, kWrite = 0x539608;   // "rb", "wb"
    const reg32 buffer = guest.frame();
    const reg32 source = fe::direct<kFopen, watcom::clib::fopen>(guest.app(), guest.cpu(), from, kRead);
    const reg32 target = fe::direct<kFopen, watcom::clib::fopen>(guest.app(), guest.cpu(), to, kWrite);
    if (source == 0 || target == 0)
        return 0xffffffffu;
    reg32 failed = 0;
    for (;;)
    {
        const reg32 got = fe::direct<0x4e9200, watcom::clib::fread>(guest.app(), guest.cpu(), buffer, 1, 0x200, source);
        if (got == 0)
            break;
        if (got != fe::direct<0x4eebd0, watcom::clib::fwrite>(guest.app(), guest.cpu(), buffer, 1, got, target))
        {
            failed = 1;
            break;
        }
    }
    fe::direct<0x4ee100, watcom::clib::fclose>(guest.app(), guest.cpu(), source);
    fe::direct<0x4ee100, watcom::clib::fclose>(guest.app(), guest.cpu(), target);
    return failed;
}

x86::reg32 frontend::loadingCopyFile(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_452a00", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 to = args.eax, from = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::copyFile(app, cpu, to, from); });
}

namespace
{

/* A saved race: the game's settings from here on, so the same offsets as from 0x6fbb40. */
constexpr reg32 kSavedMagic = 0x20f0;

}

/* sub_478090: a kept race's players' cars found by their ids: 0 if one is not there. */
x86::reg32 frontend::loada::savedResolvePlayers(App* app, CPU& cpu, reg32 saved)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    if (guest.dword(saved) != kSavedMagic)
        return 0;
    reg32 number = fe::direct<0x439990, load::placeById>(app, cpu, saved + 0x1780);
    if (sreg32(number) < 0)
        return 0;
    guest.setDword(saved + 0x177c, number);
    number = fe::direct<0x439990, load::placeById>(app, cpu, saved + 0x17ec);
    if (sreg32(number) < 0)
        return 0;
    guest.setDword(saved + 0x17e8, number);
    if (sreg32(guest.dword(saved + 0x1994)) >= 4)
    {
        number = fe::direct<0x439990, load::placeById>(app, cpu, saved + 0x1998);
        if (sreg32(number) < 0)
            return 0;
        guest.setDword(saved + 0x1994, number + 4);
    }
    return 1;
}

x86::reg32 frontend::loadingSavedResolvePlayers(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478090", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 saved = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::savedResolvePlayers(app, cpu, saved); });
}

/* sub_478100: a kept race's entries' cars found by their ids; a car not found is fine for
 * the traffic's and the cops' numbers. */
x86::reg32 frontend::loada::savedResolveEntries(App* app, CPU& cpu, reg32 saved)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    if (guest.dword(saved) != kSavedMagic)
        return 0;
    for (sreg32 i = 0; i < sreg32(guest.dword(saved + 0x19d0)); ++i)
    {
        const reg32 entry = saved + reg32(i) * kEntrySize;
        const reg32 number = fe::direct<0x439990, load::placeById>(app, cpu, entry + 0x19ec);
        if (sreg32(number) >= 0)
            guest.setDword(entry + 0x1a00, number);
        else
        {
            const sreg32 car = sreg32(guest.dword(entry + 0x1a00));
            if (!(car >= 0x32 && car <= 0x1f3) && !(car >= 0x1f4 && car <= 0x257))
                return 0;
        }
    }
    return 1;
}

x86::reg32 frontend::loadingSavedResolveEntries(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478100", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 saved = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::savedResolveEntries(app, cpu, saved); });
}

/* sub_4781b0: a race's entries' cars' ids written beside them, for a kept race. */
x86::reg32 frontend::loada::savedNameEntries(App* app, CPU& cpu, reg32 saved)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    if (guest.dword(saved) != kSavedMagic)
        return 0;
    for (sreg32 i = 0; i < sreg32(guest.dword(saved + 0x19d0)); ++i)
    {
        const reg32 entry = saved + reg32(i) * kEntrySize;
        const sreg32 car = sreg32(guest.dword(entry + 0x1a00));
        if (car < 0 || car > 0x31)
        {
            fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), entry + 0x19ec, 0xff, 0x10);
            continue;
        }
        const reg32 id = *fe::direct<0x439520, load::idOf>(app, cpu, reg32(car));
        if (id == 0)
            return 0;
        copyDwords(guest, entry + 0x19ec, id, 4);
    }
    return 1;
}

x86::reg32 frontend::loadingSavedNameEntries(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4781b0", 0, 0 };
    const reg32 saved = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::savedNameEntries(app, cpu, saved); });
}

/* sub_4bcb00: a replay's file read, every setting of the game kept as it was but the
 * replay's own: 1 if read. */
x86::reg32 frontend::loada::readReplayKeepingSettings(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1780);
    constexpr reg32 kSettingsWords = 0x5db;
    constexpr reg32 kReplayState = 0x7a3e50, kStateWords = 0x83c;
    constexpr reg32 kReadingB = 0x7cda58;
    const reg32 kept = guest.frame();
    gameSettings(guest).replayMode = 2;
    guest.setDword(kReadingB, 2);
    copyDwords(guest, kept, SETTINGS_ADDRESS(language), kSettingsWords);
    const reg32 read = *fe::direct<0x4bd2e0, load::replayRead>(app, cpu) != 0 ? 1 : 0;
    copyDwords(guest, SETTINGS_ADDRESS(fileSize), kReplayState, kStateWords);
    copyDwords(guest, SETTINGS_ADDRESS(language), kept, kSettingsWords);
    guest.setDword(0x55fe40, guest.dword(0x7a5f44));
    guest.setDword(0x7cda54, 0);
    if (read == 0)
    {
        guest.setDword(kReadingB, 0);
        gameSettings(guest).replayMode = 0;
    }
    return read;
}

x86::reg32 frontend::loadingReadReplayKeepingSettings(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bcb00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::readReplayKeepingSettings(app, cpu); });
}

/* sub_4be2f0: eax a list, edx and ebx a record's keys: that record taken out, and the last
 * slot marked free, whether it was found or not.  Records of 16 bytes from +8, a count at +4. */
x86::reg32 frontend::loada::listRemove(App* app, CPU& cpu, reg32 list, reg32 keyA, reg32 keyB)
{
    Guest guest(app, cpu);
    if (list == 0 || guest.dword(list + 4) == 0)
        return list;
    const sreg32 count = sreg32(guest.dword(list + 4));
    const reg32 records = list + 8;
    for (sreg32 i = 0; i < count; ++i)
    {
        const reg32 record = records + reg32(i) * 0x10;
        if (keyA != guest.dword(record + 4) || keyB != guest.dword(record + 8))
            continue;
        sreg32 at = i;
        for (; at < count - 1; ++at)
            copyDwords(guest, records + reg32(at) * 0x10, records + reg32(at) * 0x10 + 0x10, 4);
        break;
    }
    const reg32 end = reg32(count) * 0x10 + list;
    guest.setDword(end + 8, 0x7ffe);
    guest.setDword(list + 4, reg32(count) - 1);
    return end;
}

x86::reg32 frontend::loadingListRemove(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be2f0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 list = args.eax, keyA = args.edx, keyB = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::listRemove(app, cpu, list, keyA, keyB); });
}

/* sub_4dc550: eax which player's tunings, edx a car's id: its tuning there, or 0. */
x86::reg32 frontend::loada::tuningOf(App* app, CPU& cpu, reg32 player, reg32 id)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    const reg32 tunings = player == 0 ? 0x8b9200 : player == 1 ? 0x8b88a0 : 0;
    if (tunings == 0 || id == 0)
        return 0;
    const reg32 index = fe::direct<0x4dc490, load::specIndexOf>(app, cpu, player, id);
    if (index == 0xffffffffu)
        return 0;
    return index * 0x30 + tunings + 0x14;
}

x86::reg32 frontend::loadingTuningOf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dc550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 player = args.eax, id = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::tuningOf(app, cpu, player, id); });
}

/* sub_4dcbc0: both players' tunings copied in: the car's own edited one, or the default. */
x86::reg32 frontend::loada::tuningForPlayers(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const auto tuningFor = [&](reg32 which, reg32 to) {
        reg32 from = kDefaultTuning;
        const reg32& player = gameSettings(guest).players[which].car;
        if (fe::direct<0x439a50, load::recordAt>(app, cpu, player) != 0)
        {
            const reg32 id = *fe::direct<0x439520, load::idOf>(app, cpu, player);
            if (id != 0)
            {
                const reg32 own = guest::direct<0x4dc550, tuningOf>(app, cpu, which, id);
                if (own != 0)
                    from = own;
            }
        }
        copyDwords(guest, to, from, 7);
    };
    tuningFor(0, SETTINGS_ADDRESS(players[0].tuning));
    tuningFor(1, SETTINGS_ADDRESS(players[1].tuning));
    return 0x1c;   // the byte count, which rep movs's setting up left in eax
}

x86::reg32 frontend::loadingTuningForPlayers(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dcbc0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::tuningForPlayers(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The horizon and the small ones
 * ------------------------------------------------------------------------- */

/* sub_478280: the Screen Size chosen. */
x86::reg32 frontend::loada::screenSizeChosen(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return gameSettings(guest).language;
}

x86::reg32 frontend::loadingScreenSizeChosen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478280", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::screenSizeChosen(app, cpu); });
}

/* sub_478290: the number of the Screen Size chosen. */
x86::reg32 frontend::loada::screenSizeNumber(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest.dword(guest::direct<0x478280, screenSizeChosen>(app, cpu) * 4 + 0x55d12c);
}

x86::reg32 frontend::loadingScreenSizeNumber(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478290", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::screenSizeNumber(app, cpu); });
}

/* sub_4782b0: the number of the Screen Size at eax. */
x86::reg32 frontend::loada::screenSizeNumberOf(App* app, CPU& cpu, reg32 size)
{
    Guest guest(app, cpu);
    return guest.dword(size * 4 + 0x55d12c);
}

x86::reg32 frontend::loadingScreenSizeNumberOf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4782b0", 0, 0, nullptr, 0, 0 };
    const reg32 size = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::screenSizeNumberOf(app, cpu, size); });
}

/* sub_4782c0: on to the horizon's setting up. */
x86::reg32 frontend::loada::horizonStartCall(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest::direct<0x478c70, horizonStart>(app, cpu);
}

x86::reg32 frontend::loadingHorizonStartCall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4782c0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonStartCall(app, cpu); });
}

/* sub_4782d0: on to the horizon's freeing. */
x86::reg32 frontend::loada::horizonFreeCall(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest::direct<0x478c80, horizonFree>(app, cpu);
}

x86::reg32 frontend::loadingHorizonFreeCall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4782d0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonFreeCall(app, cpu); });
}

/* sub_4782e0: eax a view, esi where its two words go, from sub_47a880 through the frame. */
x86::reg32 frontend::loada::skyViewTo(App* app, CPU& cpu, reg32 view, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    const reg32 words = guest.frame();
    fe::direct<0x47a880, render::viewSkyMade>(app, cpu, view, words);
    guest.setDword(out, guest.dword(words));
    guest.setDword(out + 4, guest.dword(words + 4));
    return out;
}

x86::reg32 frontend::loadingSkyViewTo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4782e0", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 view = args.eax, out = args.esi;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::skyViewTo(app, cpu, view, out); });
}

/* sub_478320: eax a piece's shape, edx and ebx where its width and height go. */
x86::reg32 frontend::loada::shapeSize(App* app, CPU& cpu, reg32 shape, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    guest.setDword(width, guest.word(shape));
    const reg32 tall = guest.word(shape + 2);
    guest.setDword(height, tall);
    return tall;
}

x86::reg32 frontend::loadingShapeSize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478320", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 shape = args.eax, width = args.edx, height = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::shapeSize(app, cpu, shape, width, height); });
}

/* sub_478b70: the horizon's eight pieces, of 44 bytes each from eax. */
x86::reg32 frontend::loada::horizonSetPieces(App* app, CPU& cpu, reg32 pieces)
{
    Guest guest(app, cpu);
    constexpr reg32 kPieceSize = 44;
    for (reg32 i = 0; i < 8; ++i)
        guest.setDword(0x55d1d0 + 4 * i, pieces + i * kPieceSize);
    return 7 * kPieceSize;
}

x86::reg32 frontend::loadingHorizonSetPieces(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478b70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 pieces = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonSetPieces(app, cpu, pieces); });
}

/* sub_478bb0: the horizon's sixteen pieces, the second eight the first again, mirrored
 * when [0x55d15c] says so, and how high each is against the highest.  The seventeenth
 * height is the frame's word past the sixteen, unset: the game's own slip, kept. */
x86::reg32 frontend::loada::horizonMeasure(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x64);
    constexpr reg32 kPieces = 0x55d1d0;
    constexpr reg32 kUsed = 0x6ff95c;
    constexpr reg32 kHeights = 0x7001e8;
    const reg32 heights = guest.frame();
    const reg32 width = heights + 0x44;
    guest::direct<0x4a33d0, stopCheck>(app, cpu);
    sreg32 highest = 1;
    for (reg32 i = 0; i < 0x10; ++i)
    {
        guest::direct<0x4a33d0, stopCheck>(app, cpu);
        const reg32 from = guest.dword(0x55d15c) != 0 && i > 7 ? 0xf - i : i;
        guest.setDword(kUsed + 4 * i, guest.dword(kPieces + 4 * (from % 8)));
        guest::direct<0x478320, shapeSize>(app, cpu, guest.dword(kUsed + 4 * i), width, heights + 4 * i);
        const sreg32 height = sreg32(guest.dword(heights + 4 * i));
        if (highest < height)
            highest = height;
    }
    reg32 last = 0;
    for (reg32 i = 0; i <= 0x10; ++i)
    {
        last = guest::direct<0x4a33d0, stopCheck>(app, cpu);
        guest.setFloat(kHeights + 4 * i, float(fp::div(Float(sreg32(guest.dword(heights + 4 * i))), Float(highest))));
    }
    return last;
}

x86::reg32 frontend::loadingHorizonMeasure(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478bb0", 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonMeasure(app, cpu); });
}

/* sub_478c70: the horizon's setting up. */
x86::reg32 frontend::loada::horizonStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    return guest::direct<0x478bb0, horizonMeasure>(app, cpu);
}

x86::reg32 frontend::loadingHorizonStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478c70", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonStart(app, cpu); });
}

/* sub_478c80: the horizon's block freed, and a render state set back. */
x86::reg32 frontend::loada::horizonFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    constexpr reg32 kBlock = 0x55d278;
    const reg32 block = guest.dword(kBlock);
    if (block != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
        guest.setDword(kBlock, 0);
    }
    return fe::direct<0x431900, graphics::sendState>(app, cpu, 0x15, 0) ? 1 : 0;
}

x86::reg32 frontend::loadingHorizonFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478c80", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::horizonFree(app, cpu); });
}

/* sub_4a33d0: whether to stop loading: asked for, [0x55e88c] is set and the screen says so. */
x86::reg32 frontend::loada::stopCheck(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    constexpr reg32 kStopAsked = 0x55e88c;
    if (fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(app, cpu, 1u) == 0)
        return 0;
    guest.setDword(kStopAsked, 1);
    fe::direct<0x494bd0, graphics::loadingAborted>(app, cpu);
    return 1;
}

x86::reg32 frontend::loadingStopCheck(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4a33d0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::stopCheck(app, cpu); });
}

/* sub_4b7c20: the cop's light kept. */
x86::reg32 frontend::loada::copKeepLight(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const reg32 light = guest.dword(0x7d3684);
    guest.setDword(0x55fe28, light);
    return light;
}

x86::reg32 frontend::loadingCopKeepLight(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7c20", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::copKeepLight(app, cpu); });
}

/* sub_4b7c60: the track's cop's model loaded, by the track: "%scopN". */
x86::reg32 frontend::loada::copLoadModel(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x8c);
    constexpr reg32 kCarsFolder = 0x7a2934;
    static const reg32 kFormats[9] = { 0x53ff88, 0x53ff90, 0x53ff98, 0x53ffa0, 0x53ff88,
                                       0x53ff90, 0x53ff98, 0x53ffa0, 0x53ffa8 };
    const reg32 name = guest.frame();
    fx::callAt(guest, 0x49d640);
    guest.setDword(0x55fe28, guest.dword(0x7d3684));
    const reg32 track = gameSettings(guest).track;
    const reg32 format = track > 8 ? 0x53ff90 : kFormats[track];
    fe::direct<0x4df690, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, format, guest::Words{kCarsFolder});
    const reg32 model = *fe::direct<0x49d450, load::modelLoaded>(app, cpu, name, 0x20, 0);
    guest.setDword(0x55fe2c, model);
    return model;
}

x86::reg32 frontend::loadingCopLoadModel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7c60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::copLoadModel(app, cpu); });
}

/* sub_4bc830: the race's things loaded, a stop looked for after each. */
x86::reg32 frontend::loada::raceSteps(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    static const reg32 kSteps[5] = { 0x421360, 0x4c70f0, 0x4dbba0, 0x484070, 0x4cb670 };
    return stepsWithChecks(app, cpu, guest, kSteps);
}

x86::reg32 frontend::loadingRaceSteps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bc830", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::raceSteps(app, cpu); });
}

/* sub_4bc870: the same for the race's resetting; the last two without a check. */
x86::reg32 frontend::loada::raceResetSteps(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    static const reg32 kSteps[4] = { 0x434a80, 0x47b350, 0x4dcdf0, 0x4c7b10 };
    stepsWithChecks(app, cpu, guest, kSteps);
    fe::direct<0x4bc530, menuf::resetRaceMouse>(app, cpu);
    return guest::direct<0x4bbd70, rendd::arenaRewind>(app, cpu);
}

x86::reg32 frontend::loadingRaceResetSteps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bc870", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return loada::raceResetSteps(app, cpu); });
}

}
