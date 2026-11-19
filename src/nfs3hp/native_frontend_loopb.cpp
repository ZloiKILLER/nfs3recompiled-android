#include "native_controls_queue.h"
#include "native_controls_race.h"
#include "native_frontend_loopb.h"
#include "native_game.h"
#include "native_load_saves.h"
#include "native_load_track.h"
#include "native_raceload.h"
#include "native_aiplan.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_collide.h"
#include "native_ea_memory.h"
#include "native_controls_input.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_connect.h"
#include "native_frontend_inpa.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_menub.h"
#include "native_frontend_menud.h"
#include "native_frontend_menuf.h"
#include "native_frontend_mova.h"
#include "native_frontend_pause.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_screen.h"
#include "native_frontend_simc.h"
#include "native_frontend_tail.h"
#include "native_graphics.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstdint>
#include <cstring>

/* The race's loop around the simulation, decompiled:
 *
 *   laps and standings  who is ahead (a qsort of the cars by state, progress and the
 *                       tick of it), the place each car gets, a car's lap, its times
 *                       and top speeds, the performance times 0-60 and the like, and
 *                       made-up times for the laps a car did not finish;
 *   the end of a race   when everyone that counts is home, the short times cleared,
 *                       the states closed; asking to leave and what is taken down;
 *   the replay's record each car's four control bytes for 32 ticks, packed by runs into
 *                       0x7a5f48, unpacked back, and the table of what the television
 *                       camera finds interesting, one row per 120 ticks;
 *   the views           the rectangles of the race's views, set from the screen's mode,
 *                       and the projection a view sets;
 *   the pause's frame   the race drawn again under the pause menu, the pointer shown
 *                       while the mouse moves and hidden after a while.
 *
 * All of the game's state stays in guest memory.  What each function leaves in eax and
 * in the registers it does not save is kept, the x87's status word in ax too on the
 * ways out that follow a `fnstsw ax`.
 *
 * Plan step 5: each function a typed core in frontend::loopb (native_frontend_loopb.h), in a
 * frame of its own where the game's function has one and calls anything (fe::Frame: what it calls
 * runs at the stack addresses it did), and a thunk under the game's name for the calls by
 * address.  A core of this file calls another, and native_frontend_loopa.cpp's, as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game still
 * by address.  Before such a call the registers the call by address would load are set where what
 * follows meets them; a callee whose leftovers the function leaves stays by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* the port's widescreen change's two changes to the views, in nfs3hp_main.cpp. */
nfs::u32 widescreenHalfAngle(nfs::App* app, nfs::u32 half, nfs::u32 vertical);
nfs::u32 letterboxHeight(nfs::u32 width, nfs::u32 height, nfs::u32 half);

namespace
{

using fe::args;
using fe::Guest;
using fx::Arith;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

Float floatIn(const Guest& guest, u32 address)
{
    return Float(guest.floatAt(address));
}

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* A float's bits, as fstp dword [esp] hands an argument over. */
u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* What a jump reads after `fcom a, b; fnstsw ax; sahf`.  Unordered sets every flag. */
bool jumpsBelow(u16 status)
{
    return (status & 0x100) != 0;
}

bool jumpsBelowOrEqual(u16 status)
{
    return (status & 0x4100) != 0;
}

/* eax after `fnstsw ax`. */
u32 withStatus(u32 eax, u16 status)
{
    return (eax & 0xffff0000u) | status;
}

/* `test r, r; jge/jg; neg r`: the magnitude, the lowest integer as it is. */
u32 magnitude(u32 value)
{
    return s32(value) < 0 ? 0 - value : value;
}

/* A number divided by 2 to the power `bits`, towards zero: sar edx, 31, then sub or
 * shl and sbb, then sar. */
u32 dividedDown(u32 value, unsigned bits)
{
    const u32 towardsZero = s32(value) < 0 ? value + ((u32(1) << bits) - 1) : value;
    return u32(s32(towardsZero) >> bits);
}

/* rep movsd forwards, then rep movsb for the bytes left: the inlined memcpy. */
void copyBytes(Guest& guest, u32 to, u32 from, u32 count)
{
    u32 at = 0;
    for (u32 i = 0; i < count >> 2; ++i, at += 4)
        guest.setDword(to + at, guest.dword(from + at));
    for (u32 i = 0; i < (count & 3); ++i, ++at)
        guest.setByte(to + at, guest.byte(from + at));
}

// The game's functions, by address.
constexpr u32 kFreeSpeedTable = 0x40ec40;
constexpr u32 kMasterSilent = 0x418610;
constexpr u32 kNearestSpot = 0x423d90;       // eax a player: -1 for none
constexpr u32 kStatesForPictures = 0x431af0;
constexpr u32 kRaceTextFrame = 0x434990;
constexpr u32 kCountFrame = 0x434a70;
constexpr u32 kRaceStartDrawing = 0x434b00;
constexpr u32 kRaceStartNothing = 0x434b10;
constexpr u32 kDrawPointer = 0x434b60;
constexpr u32 kMessagesNothing = 0x4371c0;
constexpr u32 kForEachTask = 0x4be2f0;       // eax a list, edx a task's function, ebx 0
constexpr u32 kFillZero = 0x4e070c;          // eax, edx the bytes
constexpr u32 kMemmove = 0x4ea4f0;           // eax from, edx to, ebx the bytes
constexpr u32 kRand = 0x4eb1fc;
constexpr u32 kQsort = 0x4ee4f8;             // eax the base, edx the count, ebx the size, ecx the comparison
constexpr u32 kAtoi = 0x4f0430;

// This file's own.
constexpr u32 kCompareCars = 0x4cdd30;

// The race's state.
constexpr u32 kTick = 0x7d3684;
constexpr u32 kPaused = 0x7a3d10;
constexpr u32 kPrecisionUnset = 0x7a3a70;    // nonzero: the x87 is left alone
constexpr u32 kRaceControl = 0x79f3ec;
constexpr u32 kMenuControl = 0x79f3f4;
constexpr u32 kNoMenuDraw = 0x5600fc;
constexpr u32 kWideCockpit = 0x559264;
constexpr u32 kFormSize = 0x1a4;
constexpr u32 kLoadingOff = 0x55e88c;
constexpr u32 kSlicesPerLap = 0x5dd940;

// The whole screen's view, the race's, and those of each half.
constexpr u32 kViewScreen = 0x7cdac0;
constexpr u32 kViewRace = 0x7cdb88;
constexpr u32 kViewUpper = 0x7cda80;
constexpr u32 kViewLower = 0x7cdaa0;
constexpr u32 kViewUpperHud = 0x7cdb08;
constexpr u32 kViewLowerHud = 0x7cdb28;
constexpr u32 kViewMirror = 0x7cdb48;
constexpr u32 kViewMenu = 0x7cdb68;

// The lists of cars, and how many each holds.
constexpr u32 kCarsAll = 0x5efa48, kCarsAllCount = 0x5efda4;
constexpr u32 kCarsOrdered = 0x5efac8, kCarsOrderedCount = 0x5efd9c;
constexpr u32 kCarsRacing = 0x5efb08, kCarsRacingCount = 0x5efd98;
constexpr u32 kCarsAi = 0x5efb88, kCarsAiCount = 0x5efd94;
constexpr u32 kCarsChased = 0x5efc08, kCarsChasedCount = 0x553078;
constexpr u32 kCarsPlaced = 0x5efc48, kCarsPlacedCount = 0x5efda0;
constexpr u32 kCarsChasing = 0x5efd08, kCarsChasingCount = 0x553074;

// A car's record.
constexpr u32 kCarSlice = 0x1c;              // where on the road
constexpr u32 kCarWheelLoad = 0x164;         // uint16
constexpr u32 kCarDriver = 0x1f0;            // which entry of kDriversGone
constexpr u32 kCarPlayer = 0x1f4;
constexpr u32 kCarFlags = 0x200;             // byte
constexpr u32 kCarLapDone = 0x204;           // the lap the track's position says
constexpr u32 kCarLapStart = 0x208;
constexpr u32 kCarProgress = 0x224;          // slices driven, for the standings
constexpr u32 kCarProgressTick = 0x228;      // and when
constexpr u32 kCarLastSlice = 0x22c;
constexpr u32 kCarLap = 0x234;
constexpr u32 kCarLapStarted = 0x238;
constexpr u32 kCarBehindLeader = 0x23c;
constexpr u32 kCarLapTimes = 0x240;          // int[], by lap
constexpr u32 kCarRunSlice = 0x260;          // where the performance run began
constexpr u32 kCarRunTick = 0x264;           // and when
constexpr u32 kCarLapSpeeds = 0x298;         // float[], by lap
constexpr u32 kCarTopSpeed = 0x2b8;          // float
constexpr u32 kCarPlace = 0x2bc;
constexpr u32 kCarState = 0x2c4;             // kStateFinished, kStateOut...
constexpr u32 kCarSlot = 0x2cc;
constexpr u32 kCarSpeed = 0x4f4;             // float
constexpr u32 kCarAtStart = 0x50b;           // byte
constexpr u32 kCarStats = 0x528;             // a record: its float kStatsTopSpeed
constexpr u32 kCarLapChanged = 0x8d4;
constexpr u32 kStatsTopSpeed = 0x1cc;

// The performance summary's times, in a car's record: reaching a speed, and two of each
// for a distance.
constexpr u32 kTime60Mph = 0x268, kTime100Kmh = 0x26c, kTime100Mph = 0x270, kTime160Kmh = 0x274;
constexpr u32 kTimeQuarter = 0x278, kTimeQuarterToo = 0x27c, kTimeMile = 0x280, kTimeMileToo = 0x284;
constexpr u32 kTimeLast = 0x288, kTimeEnd = 0x28c;

constexpr std::uint8_t kFlagRacing = 0x01, kFlagHuman = 0x04, kFlagCapped = 0x08;
constexpr std::uint8_t kFlagGone = 0x10, kFlagWinner = 0x20, kFlagOut = 0x80;
constexpr u32 kStateRunning = 1, kStateFinished = 3, kStateOut = 4;

constexpr u32 kDriversGone = 0x7a22ca;       // uint16, by driver: left the network's race
constexpr u32 kFinishDelay = 0x1c0;          // ticks a car shows its finish before it counts

bool driverGone(const Guest& guest, u32 car)
{
    return guest.word(kDriversGone + 2 * guest.dword(car + kCarDriver)) != 0;
}

/* Whether the car has come home long enough ago. */
bool finishShown(const Guest& guest, u32 car)
{
    return s32(guest.dword(car + kCarProgressTick) + kFinishDelay) < s32(guest.dword(kTick));
}

/* ---------------------------------------------------------------------------
 * The pause's frame: sub_4bc560, sub_4bc680
 * ------------------------------------------------------------------------- */

constexpr u32 kPointerX = 0x7a3cf8, kPointerY = 0x7a3cfc, kPointerButtons = 0x7a3d00;
constexpr u32 kPointerShownUntil = 0x55fe3c;
constexpr u32 kRealTick = 0x79c1a4;
constexpr u32 kPointerHoldTicks = 0x280;

}  // namespace

/* sub_4bc560: the pointer stays up 640 ticks past the last move. */
nfs::u32 frontend::loopb::pointerIdle(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0xc);
    const u32 x = guest.frame(), y = guest.frame() + 4, buttons = guest.frame() + 8;
    // The game's code hands it x and y in edx and ebx too, which it does not read.
    guest::direct<0x4bed70, useClip>(app, ts, kViewScreen);
    fe::direct<0x435310, controls::mouseRead>(app, ts, buttons, x, y, 0);
    const u32 movedX = magnitude(guest.dword(x) - guest.dword(kPointerX));
    u32 eax = magnitude(guest.dword(y) - guest.dword(kPointerY));
    if (s32(movedX) > 1 || s32(eax) > 1 || guest.dword(buttons) != guest.dword(kPointerButtons))
    {
        guest.setDword(kPointerShownUntil, guest.dword(kRealTick) + kPointerHoldTicks);
        guest.setDword(kPointerX, guest.dword(x));
        guest.setDword(kPointerY, guest.dword(y));
        eax = guest.dword(buttons);
        guest.setDword(kPointerButtons, eax);
    }
    else if (guest.dword(kPointerShownUntil) != 0)
    {
        eax = guest.dword(kRealTick);
        if (s32(eax) >= s32(guest.dword(kPointerShownUntil)))
        {
            eax = 0;
            guest.setDword(kPointerShownUntil, 0);
        }
    }
    if (guest.dword(kPointerShownUntil) != 0)
    {
        guest.setDword(0x563d3c, 0);
        return fx::callAt(guest, kDrawPointer);
    }
    return eax;
}

/* sub_4bc680: the frame of a paused race: the view passes again, the menu over them if `menu`. */
nfs::u32 frontend::loopb::pauseDraw(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    fe::direct<0x431c30, collide::secondLayerNothing>(app, ts, 3);
    fe::direct<0x431b40, collide::hudStates>(app, ts);
    const bool onePlayer = gameSettings(guest).playMode == 1;
    guest::direct<0x4bc620, endFrame>(app, ts, guest.dword(kPaused));
    // ebx and ecx as the call by address loads them (edx as it is): sub_4bc470 below reads them.
    guest::direct<0x4bed70, useClip>(app, ts, kViewScreen);
    fe::direct<0x4be8e0, graphics::viewSurround>(app, ts);
    fe::direct<0x4bc470, frontend::simc::viewPass>(app, ts, 0, 0, 0, 0, 0);
    if (onePlayer)
    {
        fe::direct<0x4bc470, frontend::simc::viewPass>(app, ts, 0, 1, 0, 0, 0);
    }
    else if (guest.dword(kPaused) == 0 || guest.dword(kWideCockpit) != 0)
    {
        const u32 target = fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, 0);
        const u32 form = (guest.byte(target + kCarFlags) & 0x20) != 0 ? 1 : 0;
        bool cockpit = false;
        if (gameSettings(guest).hudAt(form * kFormSize).rearView != 0)
            cockpit = fe::direct<0x422aa0, carstate::insideView>(app, ts, 0);
        if (cockpit || guest.dword(kWideCockpit) != 0)
            fe::direct<0x4bc470, frontend::simc::viewPass>(app, ts, 1, 0, 0, 0, 0);
    }
    guest::direct<0x4bc650, textAndCount>(app, ts);
    if (menu != 0)
    {
        if (guest.dword(kPaused) == 0)
        {
            guest.setDword(kPaused, 1);
            fe::direct<0x4bc660, frontend::menuf::pauseMenuAgain>(app, ts);
        }
        fe::direct<0x434b20, graphics::windowSynced>(app, ts);
        fe::direct<kStatesForPictures, collide::picturesStated>(guest.app(), guest.ts());
        if (guest.dword(kNoMenuDraw) == 0)
            fe::direct<0x44bdd0, frontend::menub::menuDrawPass>(app, ts);
        fe::direct<0x434b40, graphics::flushFlip>(app, ts);
        fe::direct<0x431b40, collide::hudStates>(app, ts);
    }
    else if (guest.dword(kPaused) != 0)
    {
        guest.setDword(kPaused, 0);
        fe::direct<0x4bc670, frontend::menud::endPause>(app, ts);
    }
    if (guest.dword(kPaused) == 0)
        guest::direct<0x4bc560, pointerIdle>(app, ts);
    fe::direct<0x431c40, collide::firstLayerNothing>(app, ts, 3);
    if (guest.dword(kPrecisionUnset) != 0)
        return 3;  // eax as the call by address leaves it: the layer it was handed
    // The word an fninit leaves kept as the menus', and its precision cleared for the race.
    guest.setWord(kRaceControl, 0x7f);  // fninit's control word as fnstcw stores it; fldcw changes nothing here
    const u32 word = guest.dword(kRaceControl);
    guest.setDword(kMenuControl, word);
    guest.setDword(kRaceControl, word & 0xfffff0ffu);
    return word & 0xfffff0ffu;
}

/* ---------------------------------------------------------------------------
 * The replay's record: sub_4bc9b0 to sub_4bda00
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kReplayMode = 0x7cda58;        // 0, 1 recording; 2, 3, 4 playing back
constexpr u32 kReplayWritten = 0x7cda50;     // bytes in kReplayData
constexpr u32 kReplayRead = 0x7cda54;
constexpr u32 kReplayData = 0x7a5f48;
constexpr u32 kReplaySize = 0x1e000;
constexpr u32 kReplayStopTick = 0x55fe40;
constexpr u32 kBlockFilled = 0x7a3e30;       // int, by player: of the current block's 32 ticks
constexpr u32 kPlayerBlocks = 0x7cd548;      // by player: 32 bytes of each of four controls
constexpr u32 kPlayerBlockSize = 0x80;
constexpr u32 kRowBytes = 0x20;
constexpr u32 kControls = 0x79c4a8;          // the four control bytes of this tick
constexpr u32 kPackedRow = 0x7a3d90;
constexpr u32 kUnpackedRow = 0x7a3e10;
constexpr u32 kPendingEvents = 0x7a3dd0;     // int, by car: the worth of what just happened to it
constexpr u32 kEventRows = 0x7c3f48;         // per 120 ticks 64 bytes, an int a car
constexpr u32 kEventRowCount = 0x258;
constexpr u32 kEventRowBytes = 0x40;
constexpr u32 kEventRowsPlayed = 0x12b;
constexpr u32 kEventTicks = 0x78;
constexpr u32 kEventFirstTick = 0x200;
constexpr u32 kTvRecords = 0x7cd968;         // two, 116 bytes each
constexpr u32 kTvRecordSize = 116;
constexpr u32 kTvOn = 0x7d3680;

/* The worth of what is happening to a car, for the television: the first found of a car
 * that has finished, one near a spot, what kPendingEvents holds, a lap changed, a load
 * on its wheels; 0 for none. */
u32 eventWorth(Guest& guest, u32 index, u32 car)
{
    if ((guest.byte(car + kCarFlags) & kFlagRacing) != 0 && guest.dword(car + kCarState) == kStateFinished)
        return 10;
    if (*fe::direct<kNearestSpot, carstate::spotNearestTo>(guest.app(), guest.ts(), guest.dword(car + kCarPlayer) & 1) != u32(-1))
        return 9;
    const u32 pending = guest.dword(kPendingEvents + 4 * index);
    if (pending != 0)
    {
        guest.setDword(kPendingEvents + 4 * index, 0);
        return pending;
    }
    if (guest.dword(car + kCarLapChanged) != 0)
        return 7;
    if (guest.word(car + kCarWheelLoad) > 0x20)
        return 6;
    return 0;
}

}  // namespace

/* sub_4bc9b0: 32 bytes packed: a positive count and a byte repeated, or a negative count and
 * that many bytes as they are, the last byte alone as -1 and itself.  The packed length to
 * *length; the packed row's address back. */
nfs::u32 frontend::loopb::packRow(App* app, ThreadState& ts, u32 row, u32 length)
{
    Guest guest(app, ts);
    const auto at = [&](u32 i) { return guest.byte(row + i); };
    u32 written = 0;
    const auto put = [&](u8 value) { guest.setByte(kPackedRow + written++, value); };
    u32 start = 0, next = 1;
    do
    {
        if (at(start) == at(next))
        {
            do
                ++next;
            while (next < kRowBytes && at(start) == at(next));
            put(u8(next - start));
            put(at(start));
            start = next;
        }
        else
        {
            do
                ++next;
            while (next < kRowBytes && at(next - 1) != at(next));
            const u32 count = next - start - 1;
            put(u8(0 - count));
            for (u32 i = 0; i < count; ++i)
                put(at(start++));
        }
        next = start + 1;
    } while (next < kRowBytes);
    if (next == kRowBytes)
    {
        put(0xff);
        put(at(start));
    }
    guest.setDword(length, written);
    return kPackedRow;
}

/* sub_4bca80: and back: the bytes read to *length; the row's address back. */
nfs::u32 frontend::loopb::unpackRow(App* app, ThreadState& ts, u32 packed, u32 length)
{
    Guest guest(app, ts);
    u32 read = 0, written = 0;
    do
    {
        const std::int8_t count = std::int8_t(guest.byte(packed + read++));
        if (count > 0)
        {
            for (std::int32_t i = 0; i < count; ++i)
                guest.setByte(kUnpackedRow + written++, guest.byte(packed + read));
            ++read;
        }
        else
        {
            for (std::int32_t i = 0; i < -std::int32_t(count); ++i)
                guest.setByte(kUnpackedRow + written++, guest.byte(packed + read++));
        }
    } while (written < kRowBytes);
    guest.setDword(length, read);
    return kUnpackedRow;
}

/* sub_4bcb90: the race's start: recording cleared, or playback from the top. */
nfs::u32 frontend::loopb::replayStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const u32 mode = guest.dword(kReplayMode);
    if (mode == 0 || mode == 1)
    {
        for (u32 i = 0; i < kReplaySize; ++i)
            guest.setByte(kReplayData + i, 0);
        for (u32 i = 0; i < 16; ++i)
            guest.setDword(kPendingEvents + 4 * i, 0);
        guest.setDword(kReplayRead, 0);
        guest.setDword(kReplayWritten, 0);
        for (u32 i = 0; i < 16; ++i)
            guest.setDword(kEventRows + 4 * i, 0xb);
        for (u32 row = 1; row < kEventRowCount; ++row)
            for (u32 i = 0; i < 16; ++i)
                guest.setDword(kEventRows + row * kEventRowBytes + 4 * i, 0);
    }
    else if (s32(mode) >= 2)
    {
        guest.setDword(0x7cd948, 0);
        guest.setDword(0x7cd950, 0);
        guest.setDword(0x7cd958, 0);
        guest.setDword(0x7cd95c, 0);
        guest.setDword(0x7cd960, 0);
        guest.setDword(kReplayRead, 0);
        if (mode == 4)
        {
            guest.setDword(0x7cd960, guest.dword(0x7cd954) != 0 ? 1 : 0);
        }
        else
        {
            guest.setDword(0x7cd964, 0);
            guest.setDword(0x7cd954, 0);
            guest.setDword(0x7cd94c, 2);
        }
    }
    for (u32 tv = 0; tv < 2; ++tv)
    {
        const u32 record = kTvRecords + kTvRecordSize * tv;
        guest.setDword(record + 0x00, 1);
        guest.setDword(record + 0x04, 0);
        guest.setDword(record + 0x08, 0);
        guest.setDword(record + 0x0c, 0);
        for (u32 i = 0; i < 25; ++i)
            guest.setDword(record + 0x10 + 4 * i, 1);
    }
    for (u32 i = 0; i < 8; ++i)
        guest.setDword(kBlockFilled + 4 * i, 0);
    guest.setDword(kReplayMode, mode);
    return 8;
}

/* sub_4bcd60: the race's end: the replay saved while it was recording, a network's race over if
 * not. */
nfs::u32 frontend::loopb::replayEnd(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    u32 eax = 0;
    if (guest.dword(kLoadingOff) == 0)
    {
        if (s32(guest.dword(kReplayMode)) <= 1)
            eax = *fe::direct<0x4bd1e0, load::replaySaved>(app, ts);
        if (s32(guest.dword(kReplayMode)) > 1 && s32(gameSettings(guest).playMode) >= 2)
            gameSettings(guest).playMode = 0;
    }
    return eax;
}

/* sub_4bcda0: a block's four rows packed onto the record (the game's code hands the block by
 * value).  Full, the mode goes to 1 and the tick is kept. */
nfs::u32 frontend::loopb::storeBlock(App* app, ThreadState& ts, u32 block)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    if (s32(guest.dword(kReplayWritten) + kPlayerBlockSize) >= s32(kReplaySize))
    {
        const u32 tick = guest.dword(kTick);
        guest.setDword(kReplayStopTick, tick);
        guest.setDword(kReplayMode, 1);
        return tick;
    }
    const u32 length = guest.frame();
    u32 count = 0;
    for (u32 row = 0; row < 4; ++row)
    {
        const u32 packed = guest::direct<0x4bc9b0, packRow>(app, ts, block + row * kRowBytes, length);
        count = guest.dword(length);
        copyBytes(guest, kReplayData + guest.dword(kReplayWritten), packed, count);
        guest.setDword(kReplayWritten, guest.dword(kReplayWritten) + count);
    }
    return count;
}

/* sub_4bcee0: a block's four rows taken off the record into `block`.  The rows are made up in
 * the frame, then copied. */
nfs::u32 frontend::loopb::takeBlock(App* app, ThreadState& ts, u32 block)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x84);
    const u32 rows = guest.frame(), length = guest.frame() + 0x80;
    for (u32 row = 0; row < 4; ++row)
    {
        const u32 unpacked =
            guest::direct<0x4bca80, unpackRow>(app, ts, kReplayData + guest.dword(kReplayRead), length);
        copyBytes(guest, rows + row * kRowBytes, unpacked, kRowBytes);
        guest.setDword(kReplayRead, guest.dword(kReplayRead) + guest.dword(length));
    }
    copyBytes(guest, block, rows, 0x80);
    return block;
}

/* sub_4bd000: this tick's controls of a player onto his block; 32 of them packed away. */
nfs::u32 frontend::loopb::recordControls(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    fe::direct<0x48f6a0, controls::carControlsTaken>(app, ts, player);
    const u32 filled = kBlockFilled + 4 * player;
    const u32 block = kPlayerBlocks + player * kPlayerBlockSize;
    const u32 count = guest.dword(filled);
    guest.setByte(block + count + 0 * kRowBytes, guest.byte(kControls + 0));
    guest.setByte(block + count + 1 * kRowBytes, guest.byte(kControls + 1));
    guest.setByte(block + count + 2 * kRowBytes, guest.byte(kControls + 2));
    guest.setDword(filled, count + 1);
    guest.setByte(block + count + 3 * kRowBytes, guest.byte(kControls + 3));
    if (count + 1 != kRowBytes)
        return 4 * player;
    guest.setDword(filled, 0);
    // The block goes by value, more than a call's arguments hold: by address its copy on the stack.
    return guest::direct<0x4bcda0, storeBlock>(app, ts, block);
}

/* sub_4bd090: a player's controls for this tick off the record, and the television told. */
nfs::u32 frontend::loopb::playControls(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x80);
    const u32 filled = kBlockFilled + 4 * player;
    const u32 block = kPlayerBlocks + player * kPlayerBlockSize;
    if (guest.dword(filled) == 0)
    {
        if (guest.byte(kReplayData + guest.dword(kReplayRead)) == 0)
        {
            guest.setDword(kReplayMode, 4);
        }
        else
        {
            guest::direct<0x4bcee0, takeBlock>(app, ts, guest.frame());
            copyBytes(guest, block, guest.frame(), kPlayerBlockSize);
        }
    }
    const u32 mode = guest.dword(kReplayMode);
    if (mode == 2)
    {
        const u32 at = block + guest.dword(filled);
        guest.setByte(kControls + 0, guest.byte(at + 0 * kRowBytes));
        guest.setByte(kControls + 1, guest.byte(at + 1 * kRowBytes));
        const u8 third = guest.byte(at + 2 * kRowBytes);
        const u8 fourth = guest.byte(at + 3 * kRowBytes);
        guest.setByte(kControls + 2, third);
        guest.setByte(kControls + 3, fourth);
    }
    else if (mode == 3)
    {
        guest.setByte(kControls + 1, 0);
        guest.setByte(kControls + 2, 0);
        guest.setByte(kControls + 3, 0);
        guest.setByte(kControls + 0, 0);
    }
    guest.setDword(filled, guest.dword(filled) + 1);
    const s32 players = s32(gameSettings(guest).playMode);
    if ((players >= 2 && player == gameSettings(guest).localPlayer) || players == 0)
    {
        if (guest.dword(kTvRecords) != 0 && guest.dword(kTvOn) != 0)
            fe::direct<0x4bdd70, frontend::simc::tvDirect>(app, ts, 0);
    }
    else if (players == 1)
    {
        if (guest.dword(kTvRecords + kTvRecordSize * player) != 0 && guest.dword(kTvOn) != 0)
            fe::direct<0x4bdd70, frontend::simc::tvDirect>(app, ts, player);
    }
    if (guest.dword(filled) == kRowBytes)
        guest.setDword(filled, 0);
    return 4 * player;
}

/* sub_4bd380: a car's task each tick: the ghost of the player's own, then the record or its
 * playback. */
nfs::u32 frontend::loopb::replayCarTask(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 ours = guest.dword(kCarsAll + 4 * gameSettings(guest).localPlayer);
    if (car == ours && gameSettings(guest).replayMode == 0)
        guest::direct<0x477c20, loopa::recordCar>(app, ts, ours);
    const u32 mode = guest.dword(kReplayMode);
    if (mode == 0 || mode == 1)
    {
        guest::direct<0x4bd000, recordControls>(app, ts, guest.dword(car + kCarPlayer));
        return guest::direct<0x4bda00, recordEvents>(app, ts);
    }
    return guest::direct<0x4bd090, playControls>(app, ts, guest.dword(car + kCarPlayer));
}

/* sub_4bda00: each car's worth into this moment's row, the most of them kept. */
nfs::u32 frontend::loopb::recordEvents(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const u32 tick = guest.dword(kTick);
    if (guest.dword(kReplayMode) != 0)
        return tick;
    const s32 sinceFirst = s32(tick - kEventFirstTick);
    const s32 row = (sinceFirst < 0 ? 0 : sinceFirst) / s32(kEventTicks);
    u32 eax = u32(row);
    if (row >= s32(kEventRowsPlayed))
        return eax;
    for (u32 index = 0; s32(index) < s32(guest.dword(kCarsOrderedCount)); ++index)
    {
        const u32 car = guest.dword(kCarsOrdered + 4 * index);
        const u32 worth = eventWorth(guest, index, car);
        eax = 4 * index + (u32(row) << 6);
        if (s32(worth) > s32(guest.dword(eax + kEventRows)))
            guest.setDword(eax + kEventRows, worth);
    }
    return eax;
}

/* sub_4bd9d0: the worth a game's event gives a car, the most kept. */
nfs::u32 frontend::loopb::noteEvent(App* app, ThreadState& ts, u32 worth, u32 index)
{
    Guest guest(app, ts);
    const u32 at = (index & 0xff) << 2;
    if (s32(worth) > s32(guest.dword(at + kPendingEvents)))
        guest.setDword(at + kPendingEvents, worth);
    return worth;
}

/* ---------------------------------------------------------------------------
 * The pointer on a strip of four, and a number in a text: sub_4bd3f0, sub_4be020
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kStripLeft = 0x7497d8, kStripTop = 0x7497dc, kStripRight = 0x7497e0, kStripBottom = 0x7497e4;

}  // namespace

/* sub_4bd3f0: the strip cut 32%, 63%, 72% along; the pointer in the middle of the choice's part. */
nfs::u32 frontend::loopb::pointerAtChoice(App* app, ThreadState& ts, u32 choice)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x2c);
    const Float half = floatIn(guest, 0x540178);
    const Float rawY = fp::mul(Float(s32(guest.dword(kStripTop) + guest.dword(kStripBottom))), half);
    const Float right = Float(s32(guest.dword(kStripRight)));
    const float leftEdge = float(Float(s32(guest.dword(kStripLeft))));
    const float rightEdge = float(right);
    const Float width = fp::sub(right, Float(leftEdge));
    const Float at32 = fp::mul(width, doubleAt(guest, 0x540180));
    const Float at63 = fp::mul(width, doubleAt(guest, 0x540188));
    const Float at72 = fp::mul(width, doubleAt(guest, 0x540190));
    const float y = float(rawY);
    const Float left = Float(s32(guest.dword(kStripLeft)));
    const float first = float(fp::add(at32, left));
    const float second = float(fp::add(at63, left));
    const float third = float(fp::add(left, at72));
    float x = guest.floatAt(guest.frame() + 0x24);  // none chosen: what the stack held
    if (choice <= 3)
    {
        Float sum = 0.0;
        switch (choice)
        {
        case 0:
            sum = fp::add(Float(first), Float(leftEdge));
            break;
        case 1:
            sum = fp::add(Float(second), Float(first));
            break;
        case 2:
            sum = fp::add(Float(third), Float(second));
            break;
        default:
            sum = fp::add(Float(rightEdge), Float(third));
            break;
        }
        x = float(fp::mul(sum, half));
    }
    Arith x87(guest);
    const std::int32_t pointerY = x87.truncated(Float(y));
    const std::int32_t pointerX = x87.truncated(Float(x));
    return fe::direct<0x49bfc0, frontend::screen::setPointer>(app, ts, u32(pointerX), u32(pointerY));
}

/* sub_4be020: the next number of a text at *cursor, past what is not one and past /comments/:
 * the cursor moved on, and its value back. */
nfs::u32 frontend::loopb::readNumber(App* app, ThreadState& ts, u32 cursor)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x100);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
    u32 at = guest.dword(cursor);
    const u32 digits = guest.frame();
    const auto isNumber = [&](u32 where) {
        return fe::direct<0x4be000, frontend::simc::isNumberCharacter>(app, ts, guest.byte(where));
    };
    while ((isNumber(at) & 0xff) == 0)
    {
        if (guest.byte(at++) == '/')
            while (guest.byte(at++) != '/')
            {
            }
    }
    u32 count = 0;
    u32 answer = isNumber(at);
    while ((answer & 0xff) != 0)
    {
        guest.setByte(digits + count, guest.byte(at));
        ++count;
        ++at;
        answer = isNumber(at);
    }
    guest.setByte(digits + count, u8(answer));
    const u32 value = fe::direct<kAtoi, watcom::clib::atoi>(guest.app(), guest.ts(), digits);
    guest.setDword(cursor, at);
    return value;
}

/* ---------------------------------------------------------------------------
 * The views: sub_4be5e0, sub_4bed20 to sub_4bed70, sub_4dbce0
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kMode = 0x55fec4;
constexpr u32 kViewPasses = 0x7cdba8;

/* A view's two rectangles.  The first is what it projects through, the second what it
 * draws into. */
void rectFirst(nfs::App* app, nfs::ThreadState& ts, u32 view, u32 x, u32 y, u32 width, u32 height)
{
    fe::direct<0x4beb00, frontend::pause::setRectFirst>(app, ts, view, x, y, width, height);
}

void rectSecond(nfs::App* app, nfs::ThreadState& ts, u32 view, u32 x, u32 y, u32 width, u32 height)
{
    fe::direct<0x4beb40, frontend::pause::setRectSecond>(app, ts, view, x, y, width, height);
}

}  // namespace

/* sub_4be5e0: all the race's views from the mode's size, the form's mirror among them. */
nfs::u32 frontend::loopb::viewsLayout(App* app, ThreadState& ts, u32 form)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x28);
    const u32 xAt = guest.frame(), yAt = guest.frame() + 4, widthAt = guest.frame() + 8, heightAt = guest.frame() + 0xc;
    fe::direct<0x4be570, graphics::modeSized>(app, ts, guest.dword(kMode), xAt, yAt, widthAt, heightAt);
    const u32 x = guest.dword(xAt), y = guest.dword(yAt), width = guest.dword(widthAt), height = guest.dword(heightAt);
    if (gameSettings(guest).wideScreen != 0 && gameSettings(guest).playMode != 1)
    {
        // Bars above and below (the port's: any screen's, see letterboxHeight).
        const u32 barred = letterboxHeight(width, height, dividedDown(width << 3, 4));
        const u32 barTop = dividedDown(height - barred, 1);
        rectSecond(app, ts, kViewRace, x, barTop, width, barred);
        rectFirst(app, ts, kViewRace, x, dividedDown(0 - barTop, 1), width, height);
    }
    else
    {
        rectFirst(app, ts, kViewRace, x, y, width, height);
        rectSecond(app, ts, kViewRace, x, y, width, height);
    }
    guest.setDword(kViewPasses, 2);
    const u32 quarter = dividedDown(height, 2), half = dividedDown(height, 1);
    rectFirst(app, ts, kViewUpper, x, y - quarter, width, height);
    rectSecond(app, ts, kViewUpper, x, y, width, half);
    rectFirst(app, ts, kViewLower, x, quarter + y, width, height);
    rectSecond(app, ts, kViewLower, x, half + y, width, half);
    rectFirst(app, ts, kViewUpperHud, x, y - quarter, width, height);
    rectSecond(app, ts, kViewUpperHud, x, y, width, half);
    rectFirst(app, ts, kViewLowerHud, x, quarter + y, width, height);
    rectSecond(app, ts, kViewLowerHud, x, half + y, width, half);

    const u32 at = form * kFormSize;
    const Float h = Float(s32(height));
    const Float top = fp::mul(h, Float(gameSettings(guest).hudAt(at).rects[9].top));
    const Float w = Float(s32(width));
    const Float right = fp::mul(w, Float(gameSettings(guest).hudAt(at).rects[9].right));
    const Float bottom = fp::mul(h, Float(gameSettings(guest).hudAt(at).rects[9].bottom));
    const Float left = fp::mul(w, Float(gameSettings(guest).hudAt(at).rects[9].left));
    Arith x87(guest);
    const u32 leftEdge = u32(x87.truncated(left));
    const u32 rightEdge = u32(x87.truncated(right));
    const u32 mirrorWidth = rightEdge - leftEdge + 1;
    const u32 raise = u32(s32(height * 5) / 60);
    const u32 topEdge = u32(x87.truncated(top));
    const u32 bottomEdge = u32(x87.truncated(bottom));
    rectFirst(app, ts, kViewMirror, leftEdge, topEdge - raise, mirrorWidth, quarter);
    rectSecond(app, ts, kViewMirror, leftEdge, topEdge, mirrorWidth, bottomEdge - topEdge + 1);
    rectFirst(app, ts, kViewMenu, 0, 0, 0x280, 0x1e0);
    rectSecond(app, ts, kViewMenu, 0, 0, 0x280, 0x1e0);
    return fe::direct<0x4be3d0, frontend::pause::fillViewQuads>(app, ts);
}

/* sub_4dbce0: the projection a view asks for.  The horizontal half-angle goes through the
 * port's widescreenHalfAngle; the vertical one is 13/16 of what was asked. */
nfs::u32 frontend::loopb::setProjection(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 angle,
                                          u32 last)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x18);
    const u32 asked = dividedDown(angle, 1);
    const u32 vertical = dividedDown(asked + 12 * asked, 4);
    const u32 across = widescreenHalfAngle(app, asked, vertical);
    fe::direct<0x4fd280, frontend::mova::projectionCentre>(app, ts, x, y);
    // The width and height in ebx and ecx, where the game's code has them from its own call.
    fe::direct<0x4fd3f0, frontend::mova::projectionViewDegrees>(app, ts, across, vertical, width, height);
    const auto asFloat = [](u32 value) { return bitsOf(float(Float(s32(value)))); };
    // The game's code hands it x and `last` in eax and edx too, which it does not take.
    return fe::direct<0x4bf260, frontend::rendd::setProjection>(app, ts, asFloat(x), asFloat(y), asFloat(width),
                                                                 asFloat(height), asFloat(across), asFloat(vertical),
                                                                 last);
}

/* sub_4bed20: a view's projection and clip. */
nfs::u32 frontend::loopb::useWithProjection(App* app, ThreadState& ts, u32 view, u32 angle, u32 last)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest::direct<0x4dbce0, setProjection>(app, ts, guest.dword(view), guest.dword(view + 4), guest.dword(view + 8),
                                           guest.dword(view + 0xc), angle, last);
    return *fe::direct<0x4bebe0, graphics::clipSet>(app, ts, guest.dword(view + 0x10), guest.dword(view + 0x14), guest.dword(view + 0x18), guest.dword(view + 0x1c));
}

/* sub_4bed50: only its projection.  What sub_4dbce0 leaves in ebx, ecx and edx, which the game's
 * code leaves on from here, its callers do not read: sub_4b6020 sets ebx again and sub_4cc6d0
 * changes ecx and edx, sub_4a35d0 sets them before what it calls next reads them. */
nfs::u32 frontend::loopb::projectionOnly(App* app, ThreadState& ts, u32 view, u32 angle, u32 last)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return guest::direct<0x4dbce0, setProjection>(app, ts, guest.dword(view), guest.dword(view + 4), guest.dword(view + 8),
                                                  guest.dword(view + 0xc), angle, last);
}

/* sub_4bed70: only its clip. */
nfs::u32 frontend::loopb::useClip(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return *fe::direct<0x4bebe0, graphics::clipSet>(app, ts, guest.dword(view + 0x10), guest.dword(view + 0x14), guest.dword(view + 0x18), guest.dword(view + 0x1c));
}

/* sub_4bc100: the view a player's race HUD is drawn in. */
nfs::u32 frontend::loopb::viewForHud(App* app, ThreadState& ts, u32 form)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 view = gameSettings(guest).playMode == 1 ? kViewUpperHud + (guest.dword(form + 4) << 5) : kViewScreen;
    return guest::direct<0x4bed70, useClip>(app, ts, view);
}

/* sub_4bc130: the view a player's scene is drawn in. */
nfs::u32 frontend::loopb::viewForScene(App* app, ThreadState& ts, u32 form)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 view = gameSettings(guest).playMode == 1 ? kViewUpper + (guest.dword(form + 4) << 5) : kViewRace;
    return guest::direct<0x4bed70, useClip>(app, ts, view);
}

/* sub_4bc620: the frame ended, told whether it is the first under a pause. */
nfs::u32 frontend::loopb::endFrame(App* app, ThreadState& ts, u32 paused)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const bool first = paused == 0;
    fe::direct<0x434a00, graphics::frameEnd>(app, ts, first, first);
    return 0;  // the eax it left, which no caller reads
}

/* sub_4bc650. */
nfs::u32 frontend::loopb::textAndCount(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 text = *fe::direct<kRaceTextFrame, collide::raceTextOfFrame>(guest.app(), guest.ts());
    fe::direct<kCountFrame, collide::frameCounted>(guest.app(), guest.ts());  // (leaves eax as it was)
    return text;
}

/* sub_4bc820: nothing. */
nfs::u32 frontend::loopb::nothing(App*, ThreadState&)
{
    return 0;
}

/* sub_4bc980. */
nfs::u32 frontend::loopb::drawStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<kRaceStartDrawing, collide::raceStartPrepared>(guest.app(), guest.ts());
    return fe::direct<0x4bbd80, frontend::rendd::arenaCreate>(app, ts);
}

/* sub_4bc990. */
nfs::u32 frontend::loopb::drawEnd(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    // Its result in eax, as the call by address leaves it: sub_434b10 hands it back.
    const u32 freed = fe::direct<0x4bbdc0, frontend::rendd::arenaFree>(app, ts);
    return freed;  // sub_434b10 is an empty function: it hands eax back
}

/* sub_4bc9a0. */
nfs::u32 frontend::loopb::replayModeOff(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kReplayMode, 0);
    return 0;
}

/* ---------------------------------------------------------------------------
 * The end of a race: sub_4c4820, sub_4c4830, sub_4c4d80, sub_4c4de0, sub_4c9d90
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kFormsChanged = 0x7d3694;
constexpr u32 kEndList = 0x7d368c, kEndListB = 0x7d3690;
constexpr u32 kAskedToLeave = 0x7d3698;
constexpr u32 kStopTick = 0x7d36a4, kStopping = 0x7d36a0;

}  // namespace

/* sub_4c4820. */
nfs::u32 frontend::loopb::saveConfig(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return *fe::direct<0x472820, load::configSaved>(app, ts);
}

/* sub_4c4830: everything the race holds, let go. */
nfs::u32 frontend::loopb::teardown(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (guest.dword(kFormsChanged) != 0)
    {
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, SETTINGS_ADDRESS(hud), SETTINGS_ADDRESS(hud[6]), 0x1a4);
        guest.setDword(kFormsChanged, 0);
    }
    fe::direct<0x477a90, load::ghostSaved>(app, ts);
    fe::direct<0x471070, controls::forcesStoppedOrActuatorsOff>(app, ts, 0u);
    fe::direct<0x4c64e0, frontend::inpa::inputLocksFree>(app, ts);
    const u32 list = guest.dword(kEndList);
    if (list != 0)
        fe::direct<0x4be2f0, frontend::loada::listRemove>(app, ts, list, 0x426680, 0);
    fe::direct<0x42cab0, raceload::carTasksRemoved>(app, ts, 0u);
    fe::direct<0x4b7d00, frontend::rendd::freePlayerModel>(app, ts);
    fe::direct<kFreeSpeedTable, aiplan::freeSpeeds>(guest.app(), guest.ts(), 0);
    fe::direct<0x493fb0, frontend::rendc::conesFree>(app, ts);
    guest::direct<0x4c9d90, clearRecords>(app, ts);
    // (sub_4371c0 is an empty function)
    guest::direct<0x475b10, loopa::freeLoadedBlock>(app, ts);
    fe::direct<0x4be240, frontend::tail::blockFree>(app, ts, guest.dword(kEndListB));
    fe::direct<0x4be240, frontend::tail::blockFree>(app, ts, guest.dword(kEndList));
    guest::direct<0x4bc9a0, replayModeOff>(app, ts);
    guest::direct<0x4c4820, saveConfig>(app, ts);
    return fe::direct<0x431bc0, collide::screenStates>(app, ts) ? 1 : 0;
}

/* sub_4c4d80: the race told to stop. */
nfs::u32 frontend::loopb::stop(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    fe::direct<0x4d5cc0, graphics::drawingBegun>(app, ts, 0, 0);
    const u32 extra = s32(gameSettings(guest).playMode) >= 2 ? 0x20 : 0x10;
    guest.setDword(kStopTick, guest.dword(0x5efdfc) + extra);
    fx::callAt(guest, kMasterSilent);
    guest.setDword(0x560108, 1);
    guest.setDword(kStopping, 1);
    fe::direct<0x470f40, controls::forcesStopped>(app, ts, 0u);
    return fe::direct<0x451940, frontend::connect::flushKeys>(app, ts, 0x1b);
}

/* sub_4c4de0: whether a player's asked and the race may stop now: past its first ticks, and
 * past 320 of them unless a network holds it. */
nfs::u32 frontend::loopb::asked(App* app, ThreadState& ts, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (guest.dword(kAskedToLeave) != 0)
        return 0;
    const s32 tick = s32(guest.dword(kTick));
    if (tick < 0x20)
        return 0;
    if (tick < 0x140 && s32(gameSettings(guest).replayMode) <= 1)
        return 0;
    // Whether it may ask in eax, as the code before leaves it: the key's, or the pressed answer.
    u32 mayAsk = key;
    if (key != 0)
        mayAsk = fe::direct<0x48f7b0, controls::functionKeyHeld>(app, ts, 1, 1);
    if (fe::direct<0x4c55a0, frontend::inpa::networkHold>(app, ts, mayAsk) == 0)
        return 0;
    guest::direct<0x4c4d80, stop>(app, ts);
    return 1;
}

/* sub_4c9d90: the records at 0x7dd850 cleared. */
nfs::u32 frontend::loopb::clearRecords(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), 0x7dd850, 0x280);
}

/* ---------------------------------------------------------------------------
 * Laps and standings: sub_4cdd00 to sub_4ceb50
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kStandings = 0x5612b0;         // the cars by place, first to last
constexpr u32 kRacers = 0x5612f0;

/* The performance summary's times of a car: the speeds reached, how far it took a
 * quarter and a mile. */
constexpr u32 kTimesShown = 0x7f74a0;        // their lines shown: 100 mph, 160 km/h, 60 mph, 100 km/h

/* How far along the road a car is from where its run began, over the line if it must. */
u32 runDistance(const Guest& guest, u32 car, u32 slice, u32 slices, u32 distance)
{
    const u32 start = guest.dword(car + kCarRunSlice);
    if (s32(distance) > s32(start) && s32(slice) > s32(slices - distance) && s32(slice) < s32(slices))
        return magnitude(slice - (start + slices));
    return magnitude(slice - start);
}

/* What the fastest of a lap may be when the car's speed is capped: its stats' top speed,
 * less a tenth a driver's number. */
constexpr u32 kHandicap = 0x5404a4;

/* A random bit under the stats' top speed: rand() / 65536 * 3. */
Float underTopSpeed(const Guest& guest, u32 car, u32 random)
{
    const Float taken = fp::mul(fp::mul(Float(s32(random)), doubleAt(guest, 0x5404a8)), doubleAt(guest, 0x5404b0));
    return fp::sub(floatIn(guest, guest.dword(car + kCarStats) + kStatsTopSpeed), taken);
}

/* A car the race ended before it was home: an order at the line from its pace, and made
 * up times, a bit random, for the laps it did not finish.  The game's own 16-bit random
 * numbers, at 0x554f10. */
constexpr u32 kPaceSeed = 0x554f10, kPaceProduct = 0x554f14, kPaceFactor = 0x554f18;

/* Each tick the race is checked for its end. */
constexpr u32 kRaceEnded = 0x7d369c;

}  // namespace

/* sub_4cdd30: the qsort's comparison: the cars at *a and *b.  Who is home comes first, the
 * earlier lap among them; then the further on, then who got there first. */
nfs::u32 frontend::loopb::compareCars(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    const u32 first = guest.dword(a), second = guest.dword(b);
    const bool firstHome = (guest.byte(first + kCarFlags) & kFlagRacing) != 0 && guest.dword(first + kCarState) == kStateFinished;
    const bool secondHome = (guest.byte(second + kCarFlags) & kFlagRacing) != 0 && guest.dword(second + kCarState) == kStateFinished;
    if (firstHome && secondHome)
        return s32(guest.dword(second + kCarLapStarted)) > s32(guest.dword(first + kCarLapStarted)) ? u32(-1) : 1;
    if (firstHome)
        return u32(-1);
    if (secondHome)
        return 1;
    const s32 firstOn = s32(guest.dword(first + kCarProgress)), secondOn = s32(guest.dword(second + kCarProgress));
    if (firstOn > secondOn)
        return u32(-1);
    if (firstOn != secondOn)
        return 1;
    return s32(guest.dword(second + kCarProgressTick)) > s32(guest.dword(first + kCarProgressTick)) ? u32(-1) : 1;
}

/* sub_4cde00: the places: those still in it, sorted; 0 for the others. */
nfs::u32 frontend::loopb::standingsUpdate(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    u32 eax = 0;
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsPlacedCount)); ++i)
    {
        guest.setDword(guest.dword(kCarsPlaced + 4 * i) + kCarPlace, 0);
        eax = i + 1;
    }
    u32 count = 0;
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsPlacedCount)); ++i)
    {
        eax = guest.dword(kCarsPlaced + 4 * i);
        const u32 car = eax;
        const u8 flags = guest.byte(car + kCarFlags);
        if ((flags & kFlagOut) != 0)
            continue;
        if ((flags & kFlagRacing) != 0 && guest.dword(car + kCarState) == kStateOut)
            continue;
        if ((guest.byte(car + kCarFlags) & kFlagHuman) != 0 && s32(gameSettings(guest).playMode) > 1 && driverGone(guest, car))
            continue;
        guest.setDword(kStandings + 4 * count, car);
        ++count;
    }
    if (count != 0)
    {
        guest.setDword(kRacers, count);
        fe::direct<kQsort, watcom::clib::qsort>(guest.app(), guest.ts(), kStandings, count, 4, kCompareCars);
        count = guest.dword(kRacers);
        eax = 0;
        for (u32 i = 0; s32(i) < s32(count); ++i)
        {
            guest.setDword(guest.dword(kStandings + 4 * i) + kCarPlace, i + 1);
            eax = i + 1;
        }
    }
    guest.setDword(kRacers, count);
    return eax;
}

/* sub_4cdd00: the race's start: the standings emptied, then made. */
nfs::u32 frontend::loopb::standingsStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsPlacedCount)); ++i)
        guest.setDword(kStandings + 4 * i, 0);
    guest.setDword(kRacers, 0);
    return guest::direct<0x4cde00, standingsUpdate>(app, ts);
}

/* sub_4cded0: a car's place. */
nfs::u32 frontend::loopb::placeOf(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    return guest.dword(car + kCarPlace);
}

/* sub_4cdee0: how many a place is among. */
nfs::u32 frontend::loopb::racerCount(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kRacers);
}

/* sub_4cdef0: the car one place ahead of `place` from 2 on, for the first nobody. */
nfs::u32 frontend::loopb::carInPlace(App* app, ThreadState& ts, u32 place)
{
    Guest guest(app, ts);
    return s32(place) > 1 ? guest.dword(kStandings - 8 + 4 * place) : u32(0);
}

/* sub_4cdf10: the performance summary's times of a car. */
nfs::u32 frontend::loopb::performanceTimes(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const u32 slices = guest.dword(kSlicesPerLap);
    bool stopped = false;
    if (guest.byte(car + kCarAtStart) == 1)
    {
        // The speed's size: fchs for what fcom finds negative, the bits themselves otherwise.
        const u32 speedAt = car + kCarSpeed;
        const bool negative = !jumpsBelowOrEqual(fp::compare(0.0, floatIn(guest, speedAt)));
        const float size = negative ? float(-floatIn(guest, speedAt)) : guest.floatAt(speedAt);
        stopped = jumpsBelow(fp::compare(Float(size), floatIn(guest, 0x5404a0)));
    }
    if (stopped)
    {
        for (const u32 time : { kTime60Mph, kTime100Mph, kTimeQuarter, kTimeMile, kTimeLast, kTime100Kmh, kTime160Kmh, kTimeQuarterToo, kTimeMileToo, kTimeEnd })
            guest.setDword(car + time, 0);
        guest.setDword(kTimesShown + 0xc, u32(-1));
        guest.setDword(kTimesShown + 8, u32(-1));
        guest.setDword(kTimesShown + 4, u32(-1));
        guest.setDword(car + kCarRunSlice, guest.dword(car + kCarSlice));
        const u32 tick = guest.dword(kTick);
        guest.setDword(kTimesShown + 0, u32(-1));
        guest.setDword(car + kCarRunTick, tick);
        return tick;
    }
    const auto sinceRun = [&] { return guest.dword(kTick) - guest.dword(car + kCarRunTick); };
    // The speeds' bits, as the game compares them.
    const auto passed = [&](u32 time, s32 speedBits, u32 shownAt) {
        if (s32(guest.dword(car + kCarSpeed)) >= speedBits && guest.dword(car + time) == 0)
        {
            const u32 taken = sinceRun();
            guest.setDword(kTimesShown + shownAt, 0);
            guest.setDword(car + time, taken);
        }
    };
    passed(kTime60Mph, 0x41d651ec, 8);
    passed(kTime100Kmh, 0x41dde76d, 0xc);
    passed(kTime100Mph, 0x42328f5c, 0);
    passed(kTime160Kmh, 0x4231851f, 4);

    const u32 mode = gameSettings(guest).hud[0].speedometerForm;
    const u32 quarter = mode == 1 || mode == 2 ? 0x42 : 0x43;
    const u32 quarterGone = runDistance(guest, car, guest.dword(car + kCarSlice), slices, quarter);
    if (s32(quarterGone) >= s32(quarter) && guest.dword(car + kTimeQuarter) == 0)
        guest.setDword(car + kTimeQuarter, sinceRun());
    if (s32(quarterGone) >= s32(quarter) && guest.dword(car + kTimeQuarterToo) == 0)
        guest.setDword(car + kTimeQuarterToo, sinceRun());

    const u32 modeNow = gameSettings(guest).hud[0].speedometerForm;
    const u32 mile = modeNow == 1 || modeNow == 2 ? 0x10c : 0xa7;
    const u32 mileGone = runDistance(guest, car, guest.dword(car + kCarSlice), slices, mile);
    if (s32(mileGone) >= s32(mile) && guest.dword(car + kTimeMile) == 0)
    {
        const u32 taken = sinceRun();
        guest.setDword(car + kTimeMile, taken);
        if (s32(taken) < 0x580)
            guest.setDword(car + kTimeMile, 0);
    }
    if (s32(mileGone) >= s32(mile) && guest.dword(car + kTimeMileToo) == 0)
    {
        const u32 taken = sinceRun();
        guest.setDword(car + kTimeMileToo, taken);
        if (s32(taken) < 0x400)
            guest.setDword(car + kTimeMileToo, 0);
        return taken;
    }
    return mileGone;
}

/* sub_4ce280: a car's task each tick: its lap's top speed, the lap's end and time, and how far on. */
nfs::u32 frontend::loopb::carTask(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x24);
    const u32 speedAt = car + kCarSpeed;
    u32 eax = car;
    const s32 lap = s32(guest.dword(car + kCarLap));
    if (lap < 8)
    {
        eax = u32(lap) * 4;
        const u32 record = car + eax + kCarLapSpeeds;
        const u16 beaten = fp::compare(floatIn(guest, speedAt), floatIn(guest, record));
        eax = withStatus(eax, beaten);
        if (!jumpsBelowOrEqual(beaten))
        {
            if ((guest.byte(car + kCarFlags) & kFlagCapped) == 0)
            {
                guest.setDword(record, guest.dword(speedAt));
            }
            else if (s32(gameSettings(guest).playMode) >= 2)
            {
                // On a network: the same for everyone, by the driver's number.
                const float driver = float(Float(s32(guest.dword(car + kCarDriver))));
                const float less = float(fp::mul(Float(driver), floatIn(guest, kHandicap)));
                const Float top = floatIn(guest, guest.dword(car + kCarStats) + kStatsTopSpeed);
                const float cap = float(fp::sub(top, Float(less)));
                const u16 over = fp::compare(floatIn(guest, speedAt), Float(cap));
                const u32 kept = !jumpsBelowOrEqual(over) ? bitsOf(cap) : guest.dword(speedAt);
                guest.setDword(car + guest.dword(car + kCarLap) * 4 + kCarLapSpeeds, kept);
            }
            else
            {
                const Float cap = underTopSpeed(guest, car, fe::direct<kRand, watcom::clib::rand>(guest.app(), guest.ts()));
                const Float speed = floatIn(guest, speedAt);
                const u16 under = fp::compare(cap, speed);
                eax = withStatus(guest.dword(car + kCarStats), under);
                const Float kept = jumpsBelow(under) ? underTopSpeed(guest, car, fe::direct<kRand, watcom::clib::rand>(guest.app(), guest.ts())) : speed;
                guest.setFloat(car + guest.dword(car + kCarLap) * 4 + kCarLapSpeeds, float(kept));
            }
            eax = guest.dword(car + kCarLap) << 2;
            const u32 now = car + eax + kCarLapSpeeds;
            const u16 best = fp::compare(floatIn(guest, now), floatIn(guest, car + kCarTopSpeed));
            eax = withStatus(eax, best);
            if (!jumpsBelowOrEqual(best))
            {
                eax = guest.dword(now);
                guest.setDword(car + kCarTopSpeed, eax);
            }
        }
    }
    if ((guest.byte(car + kCarFlags) & kFlagRacing) != 0 && guest.dword(car + kCarLapDone) != guest.dword(car + kCarLap)
        && guest.dword(car + kCarState) != kStateFinished)
    {
        // A lap's ended: its time, and how far behind the leader's the same lap.
        guest.setDword(car + guest.dword(car + kCarLap) * 4 + kCarLapTimes, guest.dword(kTick) - guest.dword(car + kCarLapStarted));
        guest.setDword(car + kCarLapStarted, guest.dword(kTick));
        const u32 cars = gameSettings(guest).opponentCount + gameSettings(guest).humanCount;
        u32 leaders = 0;
        for (u32 i = 0; s32(i) < s32(cars); ++i)
        {
            const u32 other = guest.dword(kCarsOrdered + 4 * i);
            if (guest.dword(other + kCarPlace) == 1)
                leaders = guest.dword(other + guest.dword(car + kCarLap) * 4 + kCarLapTimes);
        }
        guest.setDword(car + kCarBehindLeader, guest.dword(car + guest.dword(car + kCarLap) * 4 + kCarLapTimes) - leaders);
        eax = guest.dword(car + kCarLapDone);
        const u32 laps = gameSettings(guest).laps;
        guest.setDword(car + kCarLap, eax);
        if (eax == laps && (guest.byte(car + kCarFlags) & kFlagOut) == 0 && guest.dword(car + kCarState) != kStateOut)
        {
            // Home.  The lap's start, less the finish's show, is the order at the line.
            guest.setDword(car + kCarProgressTick, guest.dword(kTick));
            eax = laps * guest.dword(kSlicesPerLap);
            guest.setDword(car + kCarState, kStateFinished);
            guest.setDword(car + kCarLapStarted, guest.dword(car + kCarLapStarted) - 0x200);
            guest.setDword(car + kCarProgress, eax);
        }
    }
    const u8 flags = guest.byte(car + kCarFlags);
    if ((flags & kFlagWinner) != 0 && (flags & kFlagHuman) != 0 && gameSettings(guest).pursuitWinCount == guest.dword(car + kCarSlot)
        && guest.dword(car + kCarState) != kStateFinished)
    {
        guest.setDword(car + kCarProgress, 0);
        eax = guest.dword(kTick);
        guest.setDword(car + kCarState, kStateFinished);
        guest.setDword(car + kCarProgressTick, eax);
        guest.setDword(car + kCarLapStarted, eax);
    }
    const u32 slice = guest.dword(car + kCarSlice);
    if (slice != guest.dword(car + kCarLastSlice))
    {
        const u32 state = guest.dword(car + kCarState);
        guest.setDword(car + kCarLastSlice, slice);
        if (state != kStateFinished && state != kStateOut)
        {
            guest.setDword(car + kCarProgressTick, guest.dword(kTick));
            const u32 along = gameSettings(guest).backwards != 0 ? guest.dword(kSlicesPerLap) - guest.dword(car + kCarLastSlice)
                                                             : guest.dword(car + kCarLastSlice);
            eax = (guest.dword(car + kCarLap) - guest.dword(car + kCarLapStart)) * guest.dword(kSlicesPerLap) + along;
            guest.setDword(car + kCarProgress, eax);
        }
    }
    return eax;
}

/* sub_4ce5a0: a car the race ended before it was home: its order at the line and made-up times. */
nfs::u32 frontend::loopb::estimateRest(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    u32 rest = 0x2a3000;
    const u32 tickOfIt = guest.dword(car + kCarProgressTick);
    if (s32(guest.dword(car + kCarProgress)) > 0 && s32(tickOfIt) > 0x200)
    {
        const u32 slicesInAll = guest.dword(kSlicesPerLap) * gameSettings(guest).laps;
        rest = u32(s32((tickOfIt - 0x200) * slicesInAll) / s32(guest.dword(car + kCarProgress)));
    }
    u32 missing = 0;
    guest.setDword(car + kCarLapStarted, rest);
    for (u32 lap = 0; s32(lap) < s32(gameSettings(guest).laps); ++lap)
    {
        const u32 time = guest.dword(car + lap * 4 + kCarLapTimes);
        rest -= time;
        if (time == 0)
            ++missing;
    }
    if (missing != 0 && s32(rest) > 0)
    {
        for (u32 lap = 0; s32(lap) < s32(gameSettings(guest).laps); ++lap)
        {
            const u32 slot = car + lap * 4;
            if (guest.dword(slot + kCarLapTimes) != 0)
                continue;
            const u32 product = guest.dword(kPaceSeed) * guest.dword(kPaceFactor);
            guest.setDword(kPaceProduct, product);
            guest.setDword(kPaceSeed, product & 0xffff);
            const u32 pick = (guest.dword(kPaceProduct) & 0xffff00) >> 8;
            const u32 share = (u32(s32(rest) / s32(missing * 20)) * pick) >> 16;
            guest.setDword(car + kCarLapStarted, guest.dword(car + kCarLapStarted) + share);
            guest.setDword(slot + kCarLapTimes, u32(s32(rest) / s32(missing)) + share);
        }
    }
    const float reach = float(fp::mul(Float(s32(guest.dword(kSlicesPerLap))), doubleAt(guest, 0x5404b8)));
    guest.setFloat(car + kCarTopSpeed, float(floatIn(guest, car + kCarLapSpeeds)));
    // eax: 0 with no laps, and what the last lap's comparison leaves there.
    u32 eax = 0;
    for (u32 lap = 0; s32(lap) < s32(gameSettings(guest).laps); ++lap)
    {
        const u32 slot = car + lap * 4;
        if ((guest.dword(slot + kCarLapSpeeds) & 0x7fffffffu) == 0 && guest.dword(slot + kCarLapTimes) != 0)
        {
            // A lap of no speed known: its average, in the game's units.
            const Float time = Float(s32(guest.dword(slot + kCarLapTimes)));
            guest.setFloat(slot + kCarLapSpeeds, float(fp::div(fp::mul(Float(reach), doubleAt(guest, 0x5404c0)), time)));
        }
        const u16 status = fp::compare(floatIn(guest, car + kCarTopSpeed), floatIn(guest, slot + kCarLapSpeeds));
        eax = withStatus(lap * 4, status);
        if (jumpsBelow(status))
        {
            eax = guest.dword(slot + kCarLapSpeeds);
            guest.setDword(car + kCarTopSpeed, eax);
        }
    }
    return eax;
}

/* sub_4ce750: the race over: those out told so, those in it 1 or the state asked; then those
 * still racing get their order made up, and 3 when nobody is left running. */
nfs::u32 frontend::loopb::closeRace(App* app, ThreadState& ts, u32 state)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    bool nobodyRunning = true;
    u32 eax = 0;
    for (; s32(eax) < s32(guest.dword(kCarsAllCount)); ++eax)
    {
        const u32 car = guest.dword(kCarsAll + 4 * eax);
        if ((guest.byte(car + kCarFlags) & kFlagOut) != 0)
        {
            guest.setDword(car + kCarState, state);
        }
        else if (guest.dword(car + kCarState) != kStateFinished && guest.dword(car + kCarState) != kStateOut)
        {
            guest.setDword(car + kCarState, kStateRunning);
            nobodyRunning = false;
        }
    }
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsRacingCount)); ++i)
    {
        const u32 car = guest.dword(kCarsRacing + 4 * i);
        const u8 flags = guest.byte(car + kCarFlags);
        eax = (eax & 0xffffff00u) | flags;
        if ((flags & kFlagGone) != 0)
            continue;
        if (guest.dword(car + kCarState) != kStateFinished)
            guest::direct<0x4ce5a0, estimateRest>(app, ts, car);
        eax = guest.dword(car + kCarState);
        if (eax == kStateFinished || eax == kStateOut)
            continue;
        guest.setDword(car + kCarState, nobodyRunning ? kStateFinished : state);
    }
    return eax;
}

/* sub_4ce800: a list's cars, but those home or out, to a state. */
nfs::u32 frontend::loopb::setStates(App* app, ThreadState& ts, u32 list, u32 count, u32 state)
{
    Guest guest(app, ts);
    u32 i = 0;
    for (; s32(i) < s32(count); ++i)
    {
        const u32 car = guest.dword(list + 4 * i);
        const u32 now = guest.dword(car + kCarState);
        if (now != kStateFinished && now != kStateOut)
            guest.setDword(car + kCarState, state);
    }
    return i;
}

/* sub_4ce840: whether everyone not out is home long enough, or gone from a four-player race. */
nfs::u32 frontend::loopb::allFinishedKnockout(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 done = 0, out = 0;
    s32 count = 0;
    for (u32 i = 0; s32(i) < (count = s32(guest.dword(kCarsAllCount))); ++i)
    {
        const u32 car = guest.dword(kCarsAll + 4 * i);
        if ((guest.byte(car + kCarFlags) & kFlagOut) != 0)
        {
            ++out;
            continue;
        }
        if ((guest.byte(car + kCarFlags) & kFlagRacing) != 0 && guest.dword(car + kCarState) == kStateFinished && finishShown(guest, car))
            ++done;
        else if (gameSettings(guest).playMode == 4 && driverGone(guest, car))
            ++done;
    }
    const u32 racing = u32(count) - out;
    return done == racing && s32(racing) > 0 ? 1 : 0;
}

/* sub_4ce8e0: the same, out or not; and the short times cleared when so. */
nfs::u32 frontend::loopb::allFinishedTimed(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    u32 done = 0;
    s32 count = 0;
    for (u32 i = 0; s32(i) < (count = s32(guest.dword(kCarsAllCount))); ++i)
    {
        const u32 car = guest.dword(kCarsAll + 4 * i);
        if ((guest.byte(car + kCarFlags) & kFlagRacing) != 0 && guest.dword(car + kCarState) == kStateFinished && finishShown(guest, car))
            ++done;
        else if (gameSettings(guest).playMode == 4 && driverGone(guest, car))
            ++done;
    }
    if (done != u32(count) || count <= 0)
        return 0;
    guest::direct<0x4ceb50, clearShortTimes>(app, ts);
    return 1;
}

/* sub_4ce960: knockout's sides: the chasers all home or gone and the chased too, or the
 * chasers and the computer's home with some chased left, who are then told they are home too. */
nfs::u32 frontend::loopb::allFinishedSides(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    const auto homeOrOut = [&](u32 car) {
        const bool racing = (guest.byte(car + kCarFlags) & kFlagRacing) != 0;
        const u32 state = guest.dword(car + kCarState);
        return racing && (state == kStateFinished || state == kStateOut);
    };
    u32 chasing = guest.dword(kCarsChasingCount);
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsChasingCount)); ++i)
    {
        const u32 car = guest.dword(kCarsChasing + 4 * i);
        if (homeOrOut(car) && finishShown(guest, car))
            --chasing;
        else if (gameSettings(guest).playMode == 4 && driverGone(guest, car))
            --chasing;
    }
    u32 computer = guest.dword(kCarsAiCount);
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsAiCount)); ++i)
    {
        const u32 car = guest.dword(kCarsAi + 4 * i);
        if (homeOrOut(car) && finishShown(guest, car))
            --computer;
    }
    u32 chased = guest.dword(kCarsChasedCount);
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsChasedCount)); ++i)
    {
        const u32 car = guest.dword(kCarsChased + 4 * i);
        if (guest.dword(car + kCarState) == kStateFinished && s32(guest.dword(kTick)) > s32(guest.dword(car + kCarProgressTick) + kFinishDelay))
            --chased;
        else if (gameSettings(guest).playMode == 4 && driverGone(guest, car))
            --chased;
    }
    if (chasing == 0 && chased == 0)
        return 1;
    if (chasing == 0 && computer == 0 && chased != 0)
    {
        guest::direct<0x4ce800, setStates>(app, ts, kCarsChased, guest.dword(kCarsChasedCount), kStateFinished);
        return 1;
    }
    return 0;
}

/* sub_4ceae0: each tick: by the kind of race, whether it has ended; then it is closed. */
nfs::u32 frontend::loopb::checkRaceEnd(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (guest.dword(kRaceEnded) != 0)
        return 0;
    u32 ended = 0;
    switch (gameSettings(guest).raceType)
    {
    case 0:
    case 1:
        ended = guest::direct<0x4ce8e0, allFinishedTimed>(app, ts);
        break;
    case 2:
        ended = guest::direct<0x4ce840, allFinishedKnockout>(app, ts);
        break;
    case 3:
        ended = guest::direct<0x4ce960, allFinishedSides>(app, ts);
        break;
    default:
        break;
    }
    if (ended == 0)
        return 0;
    guest.setDword(kRaceEnded, 1);
    const u32 result = guest::direct<0x4ce750, closeRace>(app, ts, kStateFinished);
    if (gameSettings(guest).replayMode == 0)
        guest.setDword(0x7d36b4, 0);
    return result;
}

/* sub_4ceb50: times too short to be true: gone. */
nfs::u32 frontend::loopb::clearShortTimes(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const auto clearIfShort = [&](u32 car, u32 time, s32 least) {
        const u32 value = guest.dword(car + time);
        if (value != 0 && s32(value) < least)
            guest.setDword(car + time, 0);
    };
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarsAllCount)); ++i)
    {
        eax = guest.dword(kCarsAll + 4 * i);
        clearIfShort(eax, kTimeQuarter, 0x2c0);
        clearIfShort(eax, kTimeMile, 0x640);
        clearIfShort(eax, kTimeQuarterToo, 0x2c0);
        clearIfShort(eax, kTimeMileToo, 0x480);
    }
    return eax;
}

/* sub_4d5d10. */
nfs::u32 frontend::loopb::clearDrawnFlag(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (guest.dword(0x563b30) != 0)
        guest.setDword(0x563b30, 0);
    return 0;
}

/* The thunks, under the game's names. */

#define LOOPB_PLAIN(thunk, name, core, restores)                                  \
    nfs::u32 frontend::thunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args) \
    {                                                                               \
        static fe::Site site{ name, 0, 0, nullptr, 0, restores };                   \
        return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::core(app, ts); }); \
    }

#define LOOPB_EAX(thunk, name, core, restores)                                         \
    nfs::u32 frontend::thunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args) \
    {                                                                                    \
        static fe::Site site{ name, 0, 0, nullptr, 0, restores };                        \
        const u32 eax = args.eax;                                                       \
        return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::core(app, ts, eax); }); \
    }

LOOPB_EAX(raceLoopViewForHud, "sub_4bc100", viewForHud, 0)
LOOPB_EAX(raceLoopViewForScene, "sub_4bc130", viewForScene, 0)
LOOPB_PLAIN(raceLoopPointerIdle, "sub_4bc560", pointerIdle, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_EAX(raceLoopEndFrame, "sub_4bc620", endFrame, fe::kRestoreEdx)
LOOPB_PLAIN(raceLoopTextAndCount, "sub_4bc650", textAndCount, 0)
LOOPB_EAX(raceLoopPauseDraw, "sub_4bc680", pauseDraw, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi)
LOOPB_PLAIN(raceLoopNothing, "sub_4bc820", nothing, 0)
LOOPB_PLAIN(raceLoopDrawStart, "sub_4bc980", drawStart, 0)
LOOPB_PLAIN(raceLoopDrawEnd, "sub_4bc990", drawEnd, 0)
LOOPB_PLAIN(replayModeOff, "sub_4bc9a0", replayModeOff, fe::kRestoreEdx)

nfs::u32 frontend::replayPackRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc9b0", 0, 0, nullptr, 0 };
    const u32 row = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::packRow(app, ts, row, length); });
}

nfs::u32 frontend::replayUnpackRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bca80", 0, 0, nullptr, 0 };
    const u32 packed = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::unpackRow(app, ts, packed, length); });
}

LOOPB_PLAIN(replayStart, "sub_4bcb90", replayStart, fe::kRestoreAll)
LOOPB_PLAIN(replayEnd, "sub_4bcd60", replayEnd, fe::kRestoreEdi)

nfs::u32 frontend::replayStoreBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bcda0", 0x80, 0, nullptr, 0 };
    const u32 block = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::storeBlock(app, ts, block); });
}

nfs::u32 frontend::replayTakeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bcee0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 block = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::takeBlock(app, ts, block); });
}

LOOPB_EAX(replayRecordControls, "sub_4bd000", recordControls, fe::kRestoreAll)
LOOPB_EAX(replayPlayControls, "sub_4bd090", playControls, fe::kRestoreAll)
LOOPB_EAX(replayCarTask, "sub_4bd380", replayCarTask, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi)
LOOPB_EAX(raceLoopPointerAtChoice, "sub_4bd3f0", pointerAtChoice, fe::kRestoreEcx | fe::kRestoreEdx)

nfs::u32 frontend::replayNoteEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bd9d0", 0, 0, nullptr, 0, 0 };
    const u32 worth = args.eax, index = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::noteEvent(app, ts, worth, index); });
}

LOOPB_PLAIN(replayRecordEvents, "sub_4bda00", recordEvents, fe::kRestoreAll)
LOOPB_EAX(raceLoopReadNumber, "sub_4be020", readNumber, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_EAX(viewsLayout, "sub_4be5e0", viewsLayout, fe::kRestoreAll)

nfs::u32 frontend::viewsUseWithProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bed20", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 view = args.eax, angle = args.edx, last = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::useWithProjection(app, ts, view, angle, last); });
}

nfs::u32 frontend::viewsProjectionOnly(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bed50", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 view = args.eax, angle = args.edx, last = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::projectionOnly(app, ts, view, angle, last); });
}

LOOPB_EAX(viewsUseClip, "sub_4bed70", useClip, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_PLAIN(raceEndSaveConfig, "sub_4c4820", saveConfig, 0)
LOOPB_PLAIN(raceEndTeardown, "sub_4c4830", teardown, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_PLAIN(raceEndStop, "sub_4c4d80", stop, fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_EAX(raceEndAsked, "sub_4c4de0", asked, fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_PLAIN(raceEndClearRecords, "sub_4c9d90", clearRecords, fe::kRestoreEdx)
LOOPB_PLAIN(lapsStandingsStart, "sub_4cdd00", standingsStart, fe::kRestoreEbx | fe::kRestoreEcx)

nfs::u32 frontend::lapsCompareCars(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdd30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::compareCars(app, ts, a, b); });
}

LOOPB_PLAIN(lapsStandingsUpdate, "sub_4cde00", standingsUpdate, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi)
LOOPB_EAX(lapsPlaceOf, "sub_4cded0", placeOf, 0)
LOOPB_PLAIN(lapsRacerCount, "sub_4cdee0", racerCount, 0)
LOOPB_EAX(lapsCarInPlace, "sub_4cdef0", carInPlace, 0)
LOOPB_EAX(lapsPerformanceTimes, "sub_4cdf10", performanceTimes, fe::kRestoreAll)
LOOPB_EAX(lapsCarTask, "sub_4ce280", carTask, fe::kRestoreAll)
LOOPB_EAX(lapsEstimateRest, "sub_4ce5a0", estimateRest, fe::kRestoreAll)
LOOPB_EAX(lapsCloseRace, "sub_4ce750", closeRace, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi)

nfs::u32 frontend::lapsSetStates(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ce800", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 list = args.eax, count = args.edx, state = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::setStates(app, ts, list, count, state); });
}

LOOPB_PLAIN(lapsAllFinishedKnockout, "sub_4ce840", allFinishedKnockout, fe::kRestoreAll)
LOOPB_PLAIN(lapsAllFinishedTimed, "sub_4ce8e0", allFinishedTimed, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi)
LOOPB_PLAIN(lapsAllFinishedSides, "sub_4ce960", allFinishedSides, fe::kRestoreAll)
LOOPB_PLAIN(lapsCheckRaceEnd, "sub_4ceae0", checkRaceEnd, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx)
LOOPB_PLAIN(lapsClearShortTimes, "sub_4ceb50", clearShortTimes, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEdi)
LOOPB_PLAIN(raceLoopClearDrawnFlag, "sub_4d5d10", clearDrawnFlag, fe::kRestoreEcx)

nfs::u32 frontend::viewsSetProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dbce0", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x = args.eax, y = args.edx, width = args.ebx, height = args.ecx;
    const u32 angle = stackArgument(app, ts, 0), last = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopb::setProjection(app, ts, x, y, width, height, angle, last); });
}

#undef LOOPB_PLAIN
#undef LOOPB_EAX

}
