#include "native_frontend_menub.h"
#include "native_load_saves.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_loada.h"
#include "native_frontend_loop.h"
#include "native_frontend_loopa.h"
#include "native_frontend_menuc.h"
#include "native_frontend_menud.h"
#include "native_frontend_menue.h"
#include "native_frontend_misc.h"
#include "native_frontend_neta.h"
#include "native_frontend_screen.h"
#include "native_frontend_showcase.h"
#include "native_graphics.h"
#include "native_load_cardb.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_draw2d.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_shapes.h"
#include "port_sound_effects.h"
#include "port_sound_speech.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <cstring>
#include <iterator>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

/* What the menus' screens lean on between 0x44bdd0 and 0x457680:
 *
 *   the menu loop's pass over the shown screen;
 *   the race's awards, won as the race's results are built and spoken as the
 *   postgame sheets go by;
 *   the records' tables: sorted, inserted into, and the RECORDS sheet's list
 *   (an item's own widget, 0x18 in the registry), its drawing and keys;
 *   the text that fits a column;
 *   the keys typed on any screen that unlock things (the game's codes);
 *   the showcase's car sheets (statistics, specifications, the list of a class
 *   and the comparison of two);
 *   the race's results as the menus keep them: copied from the race's cars,
 *   scored, and written out for the sheets;
 *   the direction pad widget (0x22 in the registry).
 *
 * Plan step 5: each function a typed core in frontend::menub (native_frontend_menub.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the game's
 * name for the calls by address.  A core calls another of this file's, or of
 * native_frontend_menuc.cpp's, as C++ (guest::direct), and the typed cores of the other menu files,
 * the port's and the libraries' too (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using nfs::u32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

constexpr std::uint8_t kEbxEcx = fe::kRestoreEbx | fe::kRestoreEcx;
constexpr std::uint8_t kEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = kEbxEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEcxEsiEdi = fe::kRestoreEcx | kEsiEdi;
constexpr std::uint8_t kEbxEcxEsiEdi = kEbxEcx | kEsiEdi;
constexpr std::uint8_t kEbxEcxEdxEsi = kEbxEcxEdx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEdxEdi = kEbxEcxEdx | fe::kRestoreEdi;

// The game's state.
constexpr u32 kInRace = 0x559234;
constexpr u32 kShownMenu = 0x559230;
constexpr u32 kPointerX = 0x55e5e4;
constexpr u32 kPointerY = 0x55e5e8;
constexpr u32 kPointerPressed = 0x664658;     // the last input came from the mouse: a press
constexpr u32 kGraphicsFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kHardware3D = 0x02;
constexpr u32 kTicks = 0x79c1a4;

// The game's functions.
constexpr u32 kFillZero = 0x4e070c;           // eax, edx bytes
constexpr u32 kMenuShownNow = 0x44a660;       // eax the item, edx its widget
constexpr u32 kDrawMenuText = 0x452190;       // eax text, edx x, ebx y, ecx font; align and colour on the stack
constexpr u32 kTextWidth = 0x452020;
constexpr u32 kDrawShape = 0x4d7990;          // eax the shape, edx x, ebx y
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kStricmp = 0x4e9ec0;
constexpr u32 kStrcmp = 0x4ee310;
constexpr u32 kToUpper = 0x4eede0;
constexpr u32 kQsort = 0x4ee4f8;
constexpr u32 kCarText = 0x439ab0;            // eax the car, edx the line of its text

// This file's own by address: qsort's comparison, handed to it.
constexpr u32 kRecordsCompare = 0x44f6e0;

std::int32_t signedWord(const Guest& guest, u32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t signedDword(const Guest& guest, u32 address)
{
    return std::int32_t(guest.dword(address));
}

/* What an inline strcpy leaves in eax: the terminator in al, over what was there. */
constexpr u32 afterInlineCopy(u32 eax)
{
    return eax & 0xffffff00;
}

void copyDwords(Guest& guest, u32 to, u32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* ---------------------------------------------------------------------------
 * The menu loop's pass
 * ------------------------------------------------------------------------- */

constexpr u32 kSkipPump = 0x559264;           // nonzero: not on this pass
constexpr u32 kDialogShown = 0x4438b0;
constexpr u32 kDrawPointer = 0x434b60;
constexpr u32 kPassResult = 0x559254;
constexpr u32 kCaptureScreen = 0x4c4490;

/* sub_44bdd0: a pass over the shown screen: its items drawn, and what they
 * answered, or a dialog's over them; the fonts flushed, the pointer on top, the
 * items told. */
u32 menuDrawPassBody(App* app, ThreadState& ts, Guest& guest)
{
    if (guest.dword(kSkipPump) == 0)
        fe::direct<0x4a75e0, frontend::loopa::panelDraw>(app, ts);
    u32 result =
        u32(*fe::direct<0x44a4f0, port::loop::drawItemsOf>(app, ts, ts.sp - 4, guest.dword(kShownMenu)));
        // The code hands all of eax it leaves (al its answer, the rest the items') on to the dialog's pass, and the
    // core gives al only.
    const u32 shown = (result & ~0xffu) | guest::direct<kDialogShown, port::dialogs::dialogShownOf>(app, ts);
    if ((shown & 0xff) != 0)
        result = u32(*fe::direct<0x445f50, port::dialogs::dialogPassOf>(app, ts, ts.sp - 4, shown));
    fe::direct<0x4d1340, port::text::fontsDrawn>(app, ts, ts.sp - 4);
    callAt(guest, kDrawPointer);
    fe::direct<0x44a8b0, port::loop::passItemsOf>(app, ts, ts.sp - 4, guest.dword(kShownMenu));
    guest.setDword(kPassResult, result);
    return result;
}

/* sub_44be20: out of the menus, and any dialog closed. */
u32 menuLeaveBody(App* app, ThreadState& ts, Guest& guest, u32 eax)
{
    guest.setDword(kInRace, 0);
    return u32(*fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, ts, eax));
}

/* ---------------------------------------------------------------------------
 * The race's awards: for each award a flag for each player, spoken as the
 * sheets go by.
 * ------------------------------------------------------------------------- */

constexpr u32 kAwards = 0x664440;
constexpr std::uint32_t kAwardsBytes = 0x68;

constexpr u32 kResultCount = 0x677d30;        // the race's group
constexpr u32 kAwardTrack = 0x55d020;         // the track a tournament's awards are for
constexpr u32 kSheet = 0x55936c;              // which of the postgame's sheets
constexpr u32 kAwardedCar = 0x6fdc30;

namespace participant
{
constexpr u32 kDriver = 0x1f0;                // the index in the drivers' table
constexpr u32 kFlags = 0x200;
constexpr std::uint8_t kFinisher = 0x01;
constexpr std::uint8_t kRemoteSide = 0x04;
constexpr std::uint8_t kInKnockoutTable = 0x08;
constexpr std::uint8_t kLapResult = 0x20;
constexpr std::uint8_t kOut = 0x80;
constexpr u32 kCar = 0x220;                   // its car's record
constexpr u32 kTime = 0x238;
constexpr u32 kLapTimes = 0x240;              // an int per lap
constexpr u32 kTopSpeed = 0x2b8;              // float
constexpr u32 kPlace = 0x2bc;
constexpr u32 kState = 0x2c4;
constexpr u32 kDuelsBeaten = 0x2cc;
constexpr std::uint32_t kBytes = 0x9ac;
}
constexpr u32 kCarSlot = 0x10;                // a car's record: the player's slot
constexpr u32 kCarIdent = 0x14;
constexpr std::uint32_t kStateDisqualified = 2;
constexpr std::uint32_t kStateDidNotFinish = 4;

u32 awardFlag(u32 award, u32 player)
{
    return kAwards + award * 8 + player * 4;
}

/* sub_44e7d0: no one has won anything. */
u32 awardsClearBody(Guest& guest)
{
        return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kAwards, kAwardsBytes);
}

/* sub_44e7f0.  Edx left multiplied, as the code has it. */
u32 awardsGrantBody(Guest& guest, u32 award, u32 player)
{
    const u32 offset = player * 4;
    guest.setDword(kAwards + offset + award * 8, 1);
    return award;   // eax as it came
}

/* sub_44e820: what a player won in this race, by the kind of race.  Each award
 * is the first place of a sort, but for a duel's and a knockout's. */
u32 awardsForRaceBody(App* app, ThreadState& ts, Guest& guest, u32 player)
{
    const u32 id = player == 1 ? player : gameSettings(guest).localPlayer;
    u32 found = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(app, ts, 0, id);
    if (found == 0)
    {
        found = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(app, ts, 1, id);
        if (found == 0)
            return 0;
    }
    const auto grant = [&](u32 award) {
        return guest::direct<0x44e7f0, frontend::menub::awardsGrant>(app, ts, award, player);
    };
    const auto sorted = [&](u32 key, u32 descending) {
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, key, descending);
    };
    // What a lookup leaves in eax is its answer, which the code goes on with.
    const auto nth = [&](u32 index) {
        return fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, index, 0, 0);
    };

    const u32 raceType = gameSettings(guest).raceType;
    switch (raceType)
    {
    case 0:
    {
        // A single race of more than one: the winner's.
        if (signedDword(guest, kResultCount) <= 1)
            return raceType;
        sorted(1, 0);
        const u32 first = nth(0);
        if (first != found)
            return first;
        return grant(0xa);
    }
    case 1:
    {
        // A tournament: on its track, the first three.
        if (guest.dword(kAwardTrack) != gameSettings(guest).track)
            return raceType;
        sorted(4, 1);
        if (nth(0) == found)
            return grant(7);
        if (nth(1) == found)
            return grant(8);
        const u32 third = nth(2);
        if (third == found)
            return grant(9);
        return third;
    }
    case 2:
    {
        // A knockout, but for the car that was awarded already.
        const u32 car = guest.dword(found + participant::kCar);
        if (guest.dword(kAwardedCar) == guest.dword(car + kCarIdent))
            return car;
        const u32 track = gameSettings(guest).track;
        if (track == 8)
        {
            sorted(1, 0);
            return grant(nth(0) == found ? 6 : 3);
        }
        if (guest.dword(found + participant::kState) == kStateDisqualified)
            return grant(3);
        return grant(u32(std::int32_t(gameSettings(guest).knockout.tracks[5])) == track ? 5 : 4);
    }
    case 3:
        // A duel.
        if ((guest.byte(found + participant::kFlags) & participant::kLapResult) != 0)
        {
            if (std::int32_t(gameSettings(guest).pursuitWinCount) > signedDword(guest, found + participant::kDuelsBeaten))
                return raceType;
            return grant(0xb);
        }
        if (guest.dword(found + participant::kState) == kStateDidNotFinish)
            return raceType;
        sorted(1, 0);
        if (const u32 first = nth(0); first != found)
            return first;
        return grant(0xc);
    default:
        return raceType;
    }
}

/* An award and the line a car says for it. */
struct AwardLine
{
    u32 award;
    u32 line;
};

constexpr AwardLine kFirstLines[] = { { 2, 1 }, { 1, 0 }, { 0, 2 } };
constexpr AwardLine kRaceLines[] = { { 0xa, 0xa }, { 0xc, 9 } };
constexpr AwardLine kTournamentLines[] = { { 7, 5 }, { 8, 6 }, { 9, 7 } };
constexpr AwardLine kKnockoutLines[] = { { 3, 0xf }, { 6, 0xe }, { 4, 0xc }, { 5, 0xd } };
constexpr AwardLine kDuelLines[] = { { 0xb, 8 } };

/* sub_44ea60: the next line of this kind that a player has won said, one a pass,
 * and its flag spent.  Awards 1 and 2 with award 0 are one line. */
u32 awardsSpeakBody(App* app, ThreadState& ts, Guest& guest, u32 kind)
{
    const u32 players = gameSettings(guest).playMode == 1 ? 2 : 1;
    const u32 status = u32(*fe::direct<0x4c99f0, port::spokenStatusOf>(app, ts, ts.sp - 4));
    if (status == 0)
        return status;
    const auto sayLines = [&](u32 player, const AwardLine* lines, std::size_t count, u32& said) {
        for (std::size_t i = 0; i < count; ++i)
        {
            if (guest.dword(awardFlag(lines[i].award, player)) != 0)
            {
                said = u32(*fe::direct<0x4c9990, port::carLineSpoken>(app, ts, ts.sp - 4, lines[i].line, player));
                guest.setDword(awardFlag(lines[i].award, player), 0);
                return true;
            }
        }
        return false;
    };
    for (u32 player = 0; player < players; ++player)
    {
        u32 said = 0;
        switch (kind)
        {
        case 0:
            for (const AwardLine both : { AwardLine{ 1, 3 }, AwardLine{ 2, 4 } })
            {
                if (guest.dword(awardFlag(both.award, player)) != 0 && guest.dword(awardFlag(0, player)) != 0)
                {
                    fe::direct<0x4c9990, port::carLineSpoken>(app, ts, ts.sp - 4, both.line, player);
                    guest.setDword(awardFlag(both.award, player), 0);
                    guest.setDword(awardFlag(0, player), 0);
                    return 0;
                }
            }
            if (sayLines(player, kFirstLines, std::size(kFirstLines), said))
                return said;
            break;
        case 1:
            if (sayLines(player, kRaceLines, std::size(kRaceLines), said))
                return said;
            break;
        case 2:
            if (sayLines(player, kTournamentLines, std::size(kTournamentLines), said))
                return said;
            break;
        case 3:
            if (sayLines(player, kKnockoutLines, std::size(kKnockoutLines), said))
                return said;
            break;
        case 4:
            if (sayLines(player, kDuelLines, std::size(kDuelLines), said))
                return said;
            break;
        default:
            break;
        }
    }
    return players;
}

/* sub_44ed30: the lines of the sheet shown. */
u32 awardsSpeakForSheetBody(App* app, ThreadState& ts, Guest& guest)
{
    const u32 raceType = gameSettings(guest).raceType;
    switch (guest.dword(kSheet))
    {
    case 1:
        guest::direct<0x44ea60, frontend::menub::awardsSpeak>(app, ts, 0);
        break;
    case 2:
        guest::direct<0x44ea60, frontend::menub::awardsSpeak>(app, ts, u32(raceType == 2 ? 3 : 1));
        break;
    case 3:
        if (raceType == 1)
            guest::direct<0x44ea60, frontend::menub::awardsSpeak>(app, ts, 2);
        else if (raceType == 3)
            guest::direct<0x44ea60, frontend::menub::awardsSpeak>(app, ts, 4);
        break;
    default:
        break;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * The records' tables.  sub_44f670 chooses a track's and a number of laps': its
 * ten records, in the order they came, and the holder of its best lap; a table
 * of shorts ranks them by the key asked for.
 * ------------------------------------------------------------------------- */

constexpr u32 kRecordsFile = 0x559408;
constexpr u32 kRecordsList = 0x55940c;        // the chosen list: its count, then its records
constexpr u32 kRecordHolder = 0x559410;
constexpr u32 kRecordsSortKey = 0x559414;
constexpr u32 kRecordsRanks = 0x6645d0;       // int16[10]: which record is at each rank

constexpr std::uint32_t kRecordBytes = 0x31c;
constexpr std::uint32_t kMostRecords = 10;
constexpr std::uint32_t kRecordsByLapsClass = 0x15634;
constexpr std::uint32_t kRecordsByTrack = 0x1f1c;
constexpr std::uint32_t kRecordsLists = 0x16e0; // the lists in the records' file

namespace record
{
constexpr u32 kRank = 0x00;
constexpr u32 kDriver = 0x04;                 // char[11]
constexpr u32 kCarName = 0x0f;
constexpr u32 kCarLine0 = 0x10f;
constexpr u32 kCarLine1 = 0x20f;
constexpr u32 kTime = 0x310;                  // int
constexpr u32 kSpeed = 0x314;                 // float
constexpr u32 kAids = 0x318;                  // int16: whether any aid was on
constexpr u32 kNew = 0x31a;                   // int16; bit 0, this race's
}

// The holder of a list's best lap: the time, whether it is this race's, the names.
constexpr u32 kHolderIsNew = 0x04;
constexpr u32 kHolderDriver = 0x08;
constexpr u32 kHolderCarLine0 = 0x13;
constexpr u32 kHolderCarLine1 = 0x113;

constexpr std::uint32_t kSortByTime = 4;

/* What fp::compare's status word says, as sahf hands it to a jump. */
constexpr std::uint16_t kStatusBelow = 0x0100;  // c0: below, or unordered
constexpr std::uint16_t kStatusEqual = 0x4000;  // c3: equal, or unordered

/* sub_44f6e0, qsort's: two records' places (eax and edx their slots) by the key
 * at 0x559414, and by the time when that key finds them the same. */
u32 recordsCompareBody(Guest& guest, u32 slotA, u32 slotB)
{
    const u32 a = guest.dword(slotA);
    const u32 b = guest.dword(slotB);
    const auto byTime = [&]() -> std::int32_t {
        const std::int32_t timeA = signedDword(guest, a + record::kTime);
        const std::int32_t timeB = signedDword(guest, b + record::kTime);
        return timeB > timeA ? -1 : timeB < timeA ? 1 : 0;
    };
    std::int32_t order = 0;
    switch (guest.dword(kRecordsSortKey))
    {
    case 1:
    case 4:
        order = byTime();
        break;
    case 2:
        order = std::int32_t(fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), a + record::kDriver, b + record::kDriver));
        break;
    case 3:
        order = std::int32_t(fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), a + record::kCarName, b + record::kCarName));
        break;
    case 5:
    {
        // The fastest first; unordered, the same.
        const std::uint16_t status = fp::compare(nfs::Float(guest.floatAt(a + record::kSpeed)),
                                                 nfs::Float(guest.floatAt(b + record::kSpeed)));
        if (status & kStatusBelow)
            order = 1;
        if (!(status & (kStatusBelow | kStatusEqual)))
            order = -1;
        if (status & kStatusEqual)
            order = 0;
        break;
    }
    case 6:
    {
        const std::int32_t aidsA = signedWord(guest, a + record::kAids);
        const std::int32_t aidsB = signedWord(guest, b + record::kAids);
        order = aidsA < aidsB ? 1 : aidsA > aidsB ? -1 : 0;
        break;
    }
    default:
        break;
    }
    if (order == 0)
        order = byTime();
    return u32(order);
}

/* sub_44f830: the ranks by a key.  The list's records in a frame's array,
 * sorted by qsort, and each one's number found again. */
u32 recordsSortByBody(Guest& guest, u32 key)
{
    const u32 places = guest.frame();
    const u32 list = guest.dword(kRecordsList);
    for (u32 i = 0; i < kMostRecords; ++i)
    {
        guest.setWord(kRecordsRanks + 2 * i, 0xffff);
        guest.setDword(places + 4 * i, list + 4 + i * kRecordBytes);
    }
    guest.setDword(kRecordsSortKey, key);
    fe::direct<kQsort, watcom::clib::qsort>(guest.app(), guest.ts(), places, guest.dword(list), 4, kRecordsCompare);
    std::int32_t rank = 0;
    for (; rank < signedDword(guest, guest.dword(kRecordsList)); ++rank)
    {
        const u32 place = guest.dword(places + 4 * rank);
        const u32 first = guest.dword(kRecordsList);
        for (std::int32_t number = 0; number < signedDword(guest, first); ++number)
        {
            if (first + 4 + u32(number) * kRecordBytes == place)
                guest.setWord(kRecordsRanks + 2 * rank, std::uint16_t(number));
        }
    }
    return u32(rank);
}

/* sub_44f8d0: how many records a track's list has, for a class of laps of three. */
u32 recordsCountBody(Guest& guest, u32 track, u32 lapsClass)
{
    const u32 ofClass = u32(std::int32_t(lapsClass) % 3) * kRecordsByLapsClass + guest.dword(kRecordsFile);
    return guest.dword(ofClass + track * kRecordsByTrack + kRecordsLists);
}

/* sub_44f910: the record at a rank, if there is one. */
u32 recordByRankBody(Guest& guest, u32 rank)
{
    if (guest.dword(kRecordsFile) == 0 || std::int32_t(rank) > std::int32_t(kMostRecords))
        return 0;
    const std::int32_t number = signedWord(guest, kRecordsRanks + rank * 2);
    if (number < 0)
        return 0;
    const u32 list = guest.dword(kRecordsList);
    if (number > signedDword(guest, list))
        return 0;
    return list + 4 + u32(number) * kRecordBytes;
}

/* sub_44f960: whether a time gets into the list: there is room, or it beats the
 * last. */
u32 recordsTimeQualifiesBody(App* app, ThreadState& ts, Guest& guest, u32 time)
{
    if (signedDword(guest, guest.dword(kRecordsList)) < std::int32_t(kMostRecords))
        return 1;
    guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, kSortByTime);
    const u32 last =
        guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, guest.dword(guest.dword(kRecordsList)) - 1);
    if (last == 0 || std::int32_t(time) < signedDword(guest, last + record::kTime))
        return 1;
    return 0;
}

/* sub_44f9b0: no record of any track or laps is this race's any more. */
u32 recordsClearMarksBody(App* app, ThreadState& ts, Guest& guest)
{
    u32 found = 0;
    for (u32 lapsClass = 0; lapsClass < 3; ++lapsClass)
    {
        for (u32 track = 0; track < 11; ++track)
        {
            fe::direct<0x44f670, frontend::misc::recordsSelect>(app, ts, track, lapsClass);
            guest.setDword(guest.dword(kRecordHolder) + kHolderIsNew, 0);
            guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, kSortByTime);   // and edx 0, unread
            for (u32 rank = 0; rank < kMostRecords; ++rank)
            {
                found = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, rank);
                if (found != 0)
                    guest.setWord(found + record::kNew, 0);
            }
        }
    }
    return found;
}

/* sub_44fa10: a record, by value on the stack, into the list: at its end, or
 * over the slowest when it is full; then each one's rank written into it. */
u32 recordsInsertBody(App* app, ThreadState& ts, Guest& guest, u32 newRecord)
{
    guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, kSortByTime);
    const u32 list = guest.dword(kRecordsList);
    const u32 count = guest.dword(list);
    if (count == kMostRecords)
    {
        const u32 slowest = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, kMostRecords - 1);
        copyDwords(guest, slowest, newRecord, kRecordBytes / 4);
    }
    else
    {
        copyDwords(guest, list + 4 + count * kRecordBytes, newRecord, kRecordBytes / 4);
        guest.setDword(list, guest.dword(list) + 1);
    }
    guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, kSortByTime);
    u32 found = 0;
    for (u32 rank = 0; rank < kMostRecords; ++rank)
    {
        found = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, rank);
        if (found == 0)
            break;
        guest.setDword(found + record::kRank, rank);
    }
    return found;
}

/* sub_44fb20: whether the holder of the best lap is this race's. */
u32 recordsHolderMarkBody(Guest& guest)
{
    return guest.dword(guest.dword(kRecordHolder) + kHolderIsNew);
}

/* sub_44fb30: a new holder of the best lap.  Ecx left with the mark. */
u32 recordsSetHolderBody(Guest& guest, u32 driver, u32 line0, u32 line1, u32 time, u32 mark)
{
    const u32 holder = guest.dword(kRecordHolder);
    guest.setDword(holder, time);
    guest.setDword(holder + kHolderIsNew, mark);
    guest.copyString(holder + kHolderDriver, driver);
    guest.copyString(guest.dword(kRecordHolder) + kHolderCarLine0, line0);
    guest.copyString(guest.dword(kRecordHolder) + kHolderCarLine1, line1);
    return afterInlineCopy(guest.dword(kRecordHolder));
}

/* ---------------------------------------------------------------------------
 * The RECORDS sheet: an item (widget 0x18), its columns and rows from the .MNU.
 * ------------------------------------------------------------------------- */

constexpr u32 kRecordsWidget = 0x18;
constexpr u32 kSheetTrack = 0x559418;
constexpr u32 kSheetLapsClass = 0x55941c;
constexpr u32 kRecordsAnimated = 0x662968;    // after a race: this race's records stand out

namespace sheet
{
constexpr u32 kX = 0x06;
constexpr u32 kY = 0x08;
constexpr u32 kHeaderBox = 0x1a;              // y, x, width, height
constexpr u32 kBoxHeight = 0x20;
constexpr u32 kColumnWidths = 0x3c;           // int16[6]
constexpr u32 kRowHeight = 0x48;              // int16
constexpr u32 kRows = 0x4a;                   // int16
constexpr u32 kColumnBoxes = 0x4c;            // y, x, width, height, for each column
constexpr std::uint32_t kColumns = 6;
constexpr u32 kUpBox = 0x7c;
constexpr u32 kDownBox = 0x84;
constexpr u32 kTitles = 0x8c;                 // char*[6]
constexpr u32 kShownTrack = 0xa4;             // int16
constexpr u32 kShownLapsClass = 0xa6;         // int16
constexpr u32 kUpShape = 0xa8;                // int16
constexpr u32 kDownShape = 0xaa;              // int16
constexpr u32 kUpLitShape = 0xac;             // int16
constexpr u32 kDownLitShape = 0xae;           // int16
constexpr u32 kSortMarkShape = 0xb0;          // int16
constexpr u32 kSortUpShape = 0xb2;            // int16
constexpr u32 kFirstRow = 0xb4;               // int16, the scroll
constexpr u32 kSortKey = 0xb6;                // int16, a column's, 1 to 6
constexpr u32 kSlide = 0xb8;                  // int16: 0 at rest, 1 coming in, 2 going out, 3 out
constexpr u32 kSlideX = 0xba;                 // int16
constexpr std::int32_t kOutAt = 0x2a8;
constexpr std::int32_t kSlideStep = 0x14;
}

constexpr std::uint32_t kGreyedOut = 0x4a96;    // a colour: while keys are being read
constexpr std::uint32_t kWhite = 0xffffff;
constexpr std::uint32_t kYellow = 0xffe440;
constexpr std::uint32_t kLineLight = 0xffd59428;
constexpr std::uint32_t kLineDark = 0xff0000aa;
constexpr std::uint32_t kHeaderFrame = 0xffffe440;
constexpr std::uint32_t kRowColour = 0xff649dfd;
constexpr std::uint32_t kMenuFont = 0x12;

/* sub_44fc00: the mark on the column the list is sorted by. */
u32 recordsSheetSortMarkBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const std::uint16_t chosen = std::uint16_t(guest.word(item + sheet::kSortKey) - 1);
    const u32 column = chosen <= 5 ? chosen : 0;
    const u32 box = item + sheet::kColumnBoxes + column * 8;
    const u32 place = guest.dword(box);
    const u32 size = guest.dword(box + 4);
    return fe::direct<0x46cce0, frontend::menuc::drawColumnSortMark>(
        app, ts, place, size, u32(signedWord(guest, item + sheet::kSortMarkShape)));
}

/* sub_44fc70: the sheet, on every pass: sliding in or out, its lines, its
 * headers with the pointer's lit, a row for each record shown (place, driver,
 * car, time, speed, aids), the holder of the best lap under them, the scrolling
 * arrows.  Edx left as the code leaves it. */
u32 recordsSheetDrawBody(App* app, ThreadState& ts, Guest& guest, u32 item, u32 hovering)
{
    // By address: the widget's type it hands in edx, which the core does not take, is the edx the
    // function leaves when the sheet is not shown.
    if (fe::direct<kMenuShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;

    // Going out, coming in.
    if (signedWord(guest, item + sheet::kSlide) == 2)
    {
        const std::uint16_t x = std::uint16_t(guest.word(item + sheet::kSlideX) + sheet::kSlideStep);
        guest.setWord(item + sheet::kSlideX, x);
        if (std::int16_t(x) >= sheet::kOutAt)
        {
            guest.setWord(item + sheet::kSlideX, sheet::kOutAt);
            guest.setWord(item + sheet::kSlide, 3);
        }
    }
    if (signedWord(guest, item + sheet::kSlide) == 1)
    {
        guest.setWord(item + sheet::kSlideX, std::uint16_t(guest.word(item + sheet::kSlideX) - sheet::kSlideStep));
        if (signedWord(guest, item + sheet::kSlideX) <= signedWord(guest, item + sheet::kX))
        {
            guest.setWord(item + sheet::kSlide, 0);
            guest.setWord(item + sheet::kSlideX, guest.word(item + sheet::kX));
        }
    }
    if (signedWord(guest, item + sheet::kSlide) == 3)
        return 0;

    const u32 grey = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0));
    const auto ifNotGrey = [&](std::uint32_t colour) { return grey != 0 ? kGreyedOut : colour; };

    // Another track or number of laps: from the top.
    if (u32(signedWord(guest, item + sheet::kShownTrack)) != guest.dword(kSheetTrack)
        || u32(signedWord(guest, item + sheet::kShownLapsClass)) != guest.dword(kSheetLapsClass))
    {
        guest.setWord(item + sheet::kFirstRow, 0);
    }
    guest.setWord(item + sheet::kShownTrack, guest.word(kSheetTrack));
    guest.setWord(item + sheet::kShownLapsClass, guest.word(kSheetLapsClass));

    guest::direct<0x450970, frontend::menub::recordsSheetLayout>(app, ts, item);
    fe::direct<0x44f670, frontend::misc::recordsSelect>(app, ts, guest.dword(kSheetTrack),
                                                        guest.dword(kSheetLapsClass));
    guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, u32(signedWord(guest, item + sheet::kSortKey)));

    std::int32_t width = 0;
    for (u32 column = 0; column < sheet::kColumns; ++column)
        width += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
    const std::int32_t height = signedWord(guest, item + sheet::kRowHeight) * (signedWord(guest, item + sheet::kRows) + 1);
    const std::int32_t records = std::int32_t(
        guest::direct<0x44f8d0, frontend::menub::recordsCount>(app, ts, guest.dword(kSheetTrack),
                                                               guest.dword(kSheetLapsClass)));
    const bool scrolls = records > 1;

    const auto slideX = [&]() { return signedWord(guest, item + sheet::kSlideX); };
    const auto topY = [&]() { return signedWord(guest, item + sheet::kY); };
    const auto rowHeight = [&]() { return signedWord(guest, item + sheet::kRowHeight); };
    const auto rows = [&]() { return signedWord(guest, item + sheet::kRows); };
    const auto box = [&](std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t c0,
                         std::uint32_t c1, std::uint32_t c2, std::uint32_t c3) {
        const std::uint32_t colours[] = { c0, c1, c2, c3 };
        fe::direct<0x4d8860, port::draw2d::shadedBox>(app, ts, ts.sp - 20, x, y, w, h, colours);
    };

    // The frame.
    box(slideX() - 2, topY() - 2, width + 4, 2, kLineDark, kLineLight, kLineLight, kLineDark);
    box(slideX() - 2, topY() + height + 2, width + 4, 2, kLineLight, kLineLight, kLineLight, kLineLight);
    box(slideX() - 2, topY() - 2, 2, height + 4, kLineDark, kLineDark, kLineLight, kLineLight);
    box(slideX() + width, topY() - 2, 2, height + 4, kLineLight, kLineLight, kLineLight, kLineLight);
    box(slideX(), topY() + rowHeight() - 1, width, 2, kLineDark, kLineLight, kLineLight, kLineDark);
    if (rows() < records)
    {
        fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8,
                                                    u32(signedWord(guest, item + sheet::kUpBox + 2) - 3),
                                                    u32(signedWord(guest, item + sheet::kUpBox) - 1), 0x12,
                                                    u32(height + 4), kLineLight);
    }
    // Between the rows and the columns.
    std::int32_t lineY = topY() + rowHeight();
    for (std::int32_t row = 0; row < rows(); ++row)
    {
        box(slideX(), lineY, width, 1, kLineDark, kLineLight, kLineLight, kLineDark);
        lineY += rowHeight();
    }
    std::int32_t lineX = slideX() + signedWord(guest, item + sheet::kColumnWidths);
    for (u32 column = 1; column < sheet::kColumns; ++column)
    {
        box(lineX, topY(), 1, height + 2, kLineDark, kLineDark, kLineLight, kLineLight);
        lineX += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
    }
    if (signedWord(guest, item + sheet::kSlide) == 0 && scrolls)
        guest::direct<0x44fc00, frontend::menub::recordsSheetSortMark>(app, ts, item);   // edx the height, unread

    // The columns: their alignments, and each one's text from its left.
    constexpr std::int32_t kAlign[sheet::kColumns] = { 5, 3, 3, 5, 5, 5 };
    const std::int32_t textX[sheet::kColumns] = {
        signedWord(guest, item + sheet::kColumnWidths) / 2,
        3,
        3,
        signedWord(guest, item + sheet::kColumnWidths + 6) / 2,
        signedWord(guest, item + sheet::kColumnWidths + 8) / 2,
        signedWord(guest, item + sheet::kColumnWidths + 10) / 2,
    };

    // The headers.
    for (u32 column = 0; column < sheet::kColumns; ++column)
    {
        const u32 columnBox = item + sheet::kColumnBoxes + column * 8;
        const std::int32_t x = textX[column] + signedWord(guest, columnBox + 2);
        const std::int32_t y = signedWord(guest, columnBox + 6) / 2 + signedWord(guest, columnBox) - 5;
        const u32 title = guest.dword(item + column * 4 + sheet::kTitles);
        const bool lit = scrolls && hovering != 0
            && fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, columnBox) != 0;
        if (lit)
        {
            fe::direct<0x4d8930, port::draw2d::outline>(app, ts, ts.sp - 8,
                                                        u32(signedWord(guest, columnBox + 2) + 1),
                                                        u32(signedWord(guest, columnBox)),
                                                        u32(signedWord(guest, columnBox + 4) - 1),
                                                        u32(signedWord(guest, columnBox + 6) - 1), kHeaderFrame);
        }
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, title, u32(x), u32(y), kMenuFont,
                                                        u32(kAlign[column]), ifNotGrey(lit ? kYellow : kLineLight));
    }

    // The rows.
    const u32 buffer = guest.frame() + 0x200;
    const auto columnsTo = [&](u32 columns) {
        std::int32_t x = slideX();
        for (u32 column = 0; column < columns; ++column)
            x += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
        return x;
    };
    std::int32_t rowY = topY() + rowHeight() + rowHeight() / 2 - 5;
    std::int32_t rank = signedWord(guest, item + sheet::kFirstRow);
    for (std::int32_t shown = 0; rows() > shown && rank < std::int32_t(kMostRecords); ++shown)
    {
        const u32 found = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, u32(rank));
        if (found == 0)
            break;
        // The place.
        const u32 place = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4,
                                                                              guest.dword(found + record::kRank) + 0x30, 0));
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, place, u32(slideX() + textX[0]),
                                                        u32(rowY), kMenuFont, u32(kAlign[0]),
                                                        ifNotGrey(kRowColour));
        // The driver, this race's lit.
        const std::int32_t driverX = columnsTo(1) + textX[1];
        const bool isNew = guest.dword(kRecordsAnimated) != 0 && guest.word(found + record::kNew) != 0;
        const u32 driverWidth = u32(signedWord(guest, item + sheet::kColumnWidths + 2) - 3);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, ts, found + record::kDriver, u32(driverX), u32(rowY), kMenuFont, u32(kAlign[1]),
            ifNotGrey(isNew ? kYellow : kRowColour), driverWidth);
        // The car, on two lines.
        const std::int32_t carX = columnsTo(2) + textX[2];
        const u32 carWidth = u32(signedWord(guest, item + sheet::kColumnWidths + 4) - 3);
        guest.copyString(buffer, found + record::kCarLine0);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, ts, buffer, u32(carX), u32(rowY - 10), kMenuFont, u32(kAlign[2]), ifNotGrey(kRowColour), carWidth);
        guest.copyString(buffer, found + record::kCarLine1);
        guest::direct<0x452120, frontend::menub::textDrawFitted>(
            app, ts, buffer, u32(carX), u32(rowY + 5), kMenuFont, u32(kAlign[2]), ifNotGrey(kRowColour), carWidth);
        // The time.
        fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, guest.dword(found + record::kTime), buffer);
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, buffer, u32(columnsTo(3) + textX[3]),
                                                        u32(rowY), kMenuFont, u32(kAlign[3]),
                                                        ifNotGrey(kRowColour));
        // The speed.
        fe::direct<0x4cedd0, frontend::menue::metricText>(app, ts, guest.floatAt(found + record::kSpeed), buffer);
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, buffer, u32(columnsTo(4) + textX[4]),
                                                        u32(rowY), kMenuFont, u32(kAlign[4]),
                                                        ifNotGrey(kRowColour));
        // The aids.
        const u32 aids = u32(*fe::direct<0x4d1850, port::text::lineText>(
            app, ts, ts.sp - 4, u32(guest.word(found + record::kAids) != 0 ? 0xc4 : 0xc5), 0));
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, aids,
                                                        u32(columnsTo(5) + textX[5] + kAlign[5]), u32(rowY),
                                                        kMenuFont, u32(kAlign[5]), ifNotGrey(kRowColour));
        rowY += rowHeight();
        ++rank;
    }

    // The holder of the best lap.
    const u32 holderLine = guest.frame();
    const u32 lapDriver = guest.frame() + 0x300;
    const u32 lapLine0 = guest.frame() + 0x100;
    const u32 lapLine1 = guest.frame() + 0x400;
    const u32 lap = fe::direct<0x44fa90, frontend::misc::recordsRead>(app, ts, lapDriver, lapLine0, lapLine1);
    if (lap != 0)
    {
        fe::direct<0x4cee40, frontend::menue::timeText>(app, ts, lap, buffer);
        const u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, 0xea, 0));
        constexpr u32 kHolderFormat = 0x53958c;   // "%s:  %s   %s   %s"
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), holderLine, kHolderFormat, guest::Words{title, lapDriver, lapLine1, buffer});
        const std::int32_t x = width + slideX();
        const std::int32_t y = topY() + height + 0x18;
        const bool blink = guest.dword(kRecordsAnimated) != 0
            && (guest::direct<0x44fb20, frontend::menub::recordsHolderMark>(app, ts) & 1) != 0
            && signedDword(guest, kTicks) % 0x80 < 0x40;
                fe::direct<kDrawMenuText, port::text::menuTextDrawn>(app, ts, ts.sp - 12, holderLine, u32(x), u32(y),
                                                             kMenuFont, 4, ifNotGrey(blink ? kYellow : kLineLight));
    }

    // The arrows.
    if (rows() >= records)
        return 0;
    if (guest.word(item + sheet::kFirstRow) != 0)
    {
        const u32 x = u32(signedWord(guest, item + sheet::kUpBox + 2));
        const u32 y = u32(signedWord(guest, item + sheet::kUpBox) + 1);
        const bool lit = fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + sheet::kUpBox) != 0;
        const u32 shape = u32(signedWord(guest, item + (lit ? sheet::kUpLitShape : sheet::kUpShape)));
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, shape, x, y);
    }
    const std::int32_t firstRow = signedWord(guest, item + sheet::kFirstRow);
    if (rows() + firstRow >= records)
    {
        return 0;
    }
    const u32 x = u32(signedWord(guest, item + sheet::kDownBox + 2));
    const u32 y = u32(signedWord(guest, item + sheet::kDownBox));
    const bool lit = fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + sheet::kDownBox) != 0;
    const u32 shape = u32(signedWord(guest, item + (lit ? sheet::kDownLitShape : sheet::kDownShape)));
        fe::direct<kDrawShape, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, shape, x, y);
    return 0;
}

/* sub_450770: the sheet as its screen opens: the records loaded, the track and
 * laps raced chosen, its titles and arrows, its slide; and after a race,
 * scrolled to this race's first record. */
constexpr GuestString kArrowUp{ 0x5395a0, "bup1" };
constexpr GuestString kArrowDown{ 0x5395a8, "bdn1" };
constexpr GuestString kArrowUpLit{ 0x5395b0, "yup1" };
constexpr GuestString kArrowDownLit{ 0x5395b8, "ydn1" };

u32 recordsSheetLoadBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) == 0)
        return 0;
    fe::direct<0x44f510, load::recordsTableLoaded>(app, ts);
    guest.setDword(kSheetTrack, gameSettings(guest).track);
    const u32 lapsClass = u32(std::int16_t(fe::direct<0x4cf160, frontend::menue::lapsToOption>(
        app, ts, u32(std::int32_t(std::int16_t(gameSettings(guest).laps))))));
    guest.setDword(kSheetLapsClass, lapsClass);
    fe::direct<0x44f670, frontend::misc::recordsSelect>(app, ts, guest.dword(kSheetTrack), lapsClass);
    guest.setWord(item + sheet::kShownTrack, guest.word(kSheetTrack));
    guest.setWord(item + sheet::kShownLapsClass, guest.word(kSheetLapsClass));
    if (signedWord(guest, item + sheet::kRows) >= 10)
        guest.setWord(item + sheet::kRows, 10);
    static constexpr std::uint32_t kTitleTexts[sheet::kColumns] = { 0xe3, 0xc3, 2, 0x28, 0x29, 0x192 };
    for (u32 column = 0; column < sheet::kColumns; ++column)
    {
        const auto title =
            fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, kTitleTexts[column], 0);
        guest.setDword(item + sheet::kTitles + column * 4, u32(*title));
    }
    const auto shape = [&](const GuestString& name) {
        return u32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4, name.address));
    };
    guest.setWord(item + sheet::kUpShape, std::uint16_t(shape(kArrowUp)));
    guest.setWord(item + sheet::kDownShape, std::uint16_t(shape(kArrowDown)));
    guest.setWord(item + sheet::kUpLitShape, std::uint16_t(shape(kArrowUpLit)));
    guest.setWord(item + sheet::kDownLitShape, std::uint16_t(shape(kArrowDownLit)));
    guest.setWord(item + sheet::kSortMarkShape, std::uint16_t(shape(kArrowDown)));
    u32 left = shape(kArrowUp);
    guest.setWord(item + sheet::kSortUpShape, std::uint16_t(left));
    guest.setWord(item + sheet::kFirstRow, 0);
    guest.setWord(item + sheet::kSortKey, 1);
    if (guest.word(item + sheet::kSlideX) == 0)
    {
        const std::uint16_t x = guest.word(item + sheet::kX);
        guest.setWord(item + sheet::kSlide, 0);
        guest.setWord(item + sheet::kSlideX, x);
        left = (left & 0xffff0000) | x;
    }
    if (guest.dword(kRecordsAnimated) == 0)
        return left;

    const std::int32_t records = std::int32_t(
        guest::direct<0x44f8d0, frontend::menub::recordsCount>(app, ts, guest.dword(kSheetTrack),
                                                               guest.dword(kSheetLapsClass)));
    left = guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts,
                                                                   u32(signedWord(guest, item + sheet::kSortKey)));
    for (std::int32_t rank = 0; rank < records; ++rank)
    {
        const u32 found = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, u32(rank));
        if (found == 0)
            return 0;
        left = found;
        if ((guest.byte(found + record::kNew) & 1) != 0)
        {
            left = u32(signedWord(guest, item + sheet::kRows));
            if (rank >= std::int32_t(left))
            {
                const std::uint16_t first = std::uint16_t(rank - guest.word(item + sheet::kRows));
                const u32 result = ((u32(rank) & 0xffff0000) | first) + 1;
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(result));
                return result;
            }
        }
    }
    return left;
}

/* sub_450950: its records saved as the screen closes. */
u32 recordsSheetSaveBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    const u32 shown = fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item);
    if (shown != 0)
        return *fe::direct<0x44f5f0, load::recordsTableSaved>(app, ts);
    return shown;
}

/* sub_450970: the columns' boxes, and the arrows', from the column widths and
 * where the sheet has slid to.  Eax left as the code's sums leave it. */
u32 recordsSheetLayoutBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) == 0)
        return 0;
    std::int32_t x = signedWord(guest, item + sheet::kSlideX);
    u32 total = 0;
    for (u32 column = 0; column < sheet::kColumns; ++column)
    {
        const u32 box = item + sheet::kColumnBoxes + column * 8;
        guest.setWord(box + 2, std::uint16_t(x));
        guest.setWord(box, guest.word(item + sheet::kY));
        guest.setWord(box + 4, guest.word(item + sheet::kColumnWidths + column * 2));
        const std::int32_t width = signedWord(guest, box + 4);
        guest.setWord(box + 6, guest.word(item + sheet::kRowHeight));
        x += width;
        total += u32(width);
    }
    const u32 height = u32((signedWord(guest, item + sheet::kRows) + 1) * signedWord(guest, item + sheet::kRowHeight));

    const u32 header = item + sheet::kHeaderBox;
    guest.setWord(header + 2, guest.word(item + sheet::kSlideX));
    guest.setWord(header, guest.word(item + sheet::kY));
    guest.setWord(header + 4, std::uint16_t(total + 0x16));

    const auto withLowWord = [](u32 value, std::uint16_t low) { return (value & 0xffff0000) | low; };
    const u32 upX = withLowWord(total + 0x16, guest.word(item + sheet::kSlideX)) + total + 0xa;
    guest.setWord(item + sheet::kUpBox + 4, 0xc);
    guest.setWord(item + sheet::kUpBox + 2, std::uint16_t(upX));
    guest.setWord(item + sheet::kUpBox, guest.word(item + sheet::kY));
    guest.setWord(item + sheet::kUpBox + 6, 0xc);
    const u32 downX = total + withLowWord(upX, guest.word(item + sheet::kSlideX)) + 0xa;
    guest.setWord(item + sheet::kDownBox + 4, 0xc);
    guest.setWord(item + sheet::kDownBox + 2, std::uint16_t(downX));
    guest.setWord(item + sheet::kDownBox + 6, 0xc);
    guest.setWord(item + sheet::kBoxHeight, std::uint16_t(height));
    const u32 downY = withLowWord(downX, guest.word(item + sheet::kY)) + height - 0xc;
    guest.setWord(item + sheet::kDownBox, std::uint16_t(downY));
    return downY;
}

/* The pointer moved into a box: three quarters across, half way down. */
void pointerIntoBox(App* app, ThreadState& ts, Guest& guest, u32 box)
{
    const std::int32_t x = signedWord(guest, box + 2) + signedWord(guest, box + 4) * 3 / 4;
    const std::int32_t y = signedWord(guest, box) + signedWord(guest, box + 6) / 2;
    guest.setDword(kPointerX, u32(x));
    guest.setDword(kPointerY, u32(y));
    fe::direct<0x49bfc0, frontend::screen::setPointer>(app, ts, u32(x), u32(y));
}

/* sub_450a90: the sheet's keys: Enter on a column sorts by it, the arrows go
 * across the columns, Page Up and Page Down and clicks on the arrows scroll.
 * What it answers stays in edx, ebx what the code leaves there. */
u32 recordsSheetKeyBody(App* app, ThreadState& ts, Guest& guest, u32 item, std::uint16_t key)
{
    constexpr std::uint16_t kEnter = 0xd, kRight = 0x4d00, kLeft = 0x4b00, kPageUp = 0x4900, kPageDown = 0x5100;
    constexpr std::uint32_t kClick = 1, kVolume = 0x7f;

    std::int32_t column = -1;
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) == 0)
    {
        return 0;
    }
    const std::int32_t records = std::int32_t(
        guest::direct<0x44f8d0, frontend::menub::recordsCount>(app, ts, guest.dword(kSheetTrack),
                                                               guest.dword(kSheetLapsClass)));
    if (records < 2)
    {
        return 0;
    }
    for (u32 i = 0; i < sheet::kColumns; ++i)
    {
        if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + sheet::kColumnBoxes + i * 8) != 0)
        {
            column = std::int32_t(i);
            break;
        }
    }
    const auto clamped = [](std::int32_t value) { return value < 0 ? 0 : value > 5 ? 5 : value; };
    const auto canScrollDown = [&]() {
        return signedWord(guest, item + sheet::kRows) + signedWord(guest, item + sheet::kFirstRow) < records;
    };

    u32 answer = 0;
    if (key == kEnter)
    {
        if (column >= 0)
            guest.setWord(item + sheet::kSortKey, std::uint16_t(column + 1));
        answer = 2;
    }
    if (key == kRight || key == kLeft)
    {
        column = clamped(column + (key == kRight ? 1 : -1));
        pointerIntoBox(app, ts, guest, item + sheet::kColumnBoxes + u32(column) * 8);
        answer = 2;
    }
    if (signedWord(guest, item + sheet::kRows) < records)
    {
        if (key == kPageUp && guest.word(item + sheet::kFirstRow) != 0)
        {
            fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kClick, kVolume);
            const std::uint16_t first = guest.word(item + sheet::kFirstRow);
            guest.setWord(item + sheet::kFirstRow, std::uint16_t(first - 1));
            pointerIntoBox(app, ts, guest, item + sheet::kUpBox);
            answer = 2;
        }
        if (key == kPageDown)
        {
            if (canScrollDown())
            {
                fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kClick, kVolume);
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) + 1));
                pointerIntoBox(app, ts, guest, item + sheet::kDownBox);
                answer = 2;
            }
        }
        if (key == kEnter)
        {
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + sheet::kUpBox) != 0
                && guest.word(item + sheet::kFirstRow) != 0)
            {
                fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kClick, kVolume);
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) - 1));
                answer = 2;
            }
            if (fe::direct<0x49c1f0, frontend::glue::pointerInBox>(app, ts, item + sheet::kDownBox) != 0
                && canScrollDown())
            {
                fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kClick, kVolume);
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) + 1));
                answer = 2;
            }
        }
    }
    return answer;
}

/* sub_450e30: a widget's slot with nothing to do. */
u32 recordsSheetIdleBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item);
    return 0;
}

constexpr std::uint32_t kDriverBytes = 0x6c;
constexpr u32 kDriverCar = 0x14;
constexpr u32 kDriverName = 0x34;

/* The aids a race had on, which a record keeps together. */
constexpr u32 GameSettings::* kAidsOn[] = { &GameSettings::raceOptionComputerDrives, &GameSettings::raceOptionLaneVotes,
                                              &GameSettings::raceOptionCones, &GameSettings::raceOptionCopRadio,
                                              &GameSettings::raceOptionHazard, &GameSettings::raceOptionSimulation };

constexpr std::uint32_t kEnteredBestLap = 1;
constexpr std::uint32_t kEnteredBestTime = 2;
constexpr std::uint32_t kEnteredTime = 4;

/* sub_450fe0: a participant's race into the chosen list and its best lap, for
 * the awards: which it got.  The record built in the frame, and handed by value. */
u32 recordsEnterResultBody(App* app, ThreadState& ts, Guest& guest, u32 found)
{
    const u32 entry = guest.dword(found + participant::kDriver);
    const u32 driver = SETTINGS_ADDRESS(entries) + entry * kDriverBytes;
    const u32 car = gameSettings(guest).entries[entry].car;
    const u32 newRecord = guest.frame();
    const u32 ignored = guest.frame() + 0x51c;
    std::uint32_t entered = 0;

    const u32 holderLap = fe::direct<0x44fa90, frontend::misc::recordsRead>(app, ts, ignored, ignored, ignored);
    const u32 ownCar = guest.dword(guest.dword(found + participant::kCar) + kCarIdent);
    if (ownCar == guest.dword(kAwardedCar))
        return 0;
    if ((guest.byte(found + participant::kFlags) & participant::kFinisher) == 0)
        return 0;

    const u32 raceTime = guest.dword(found + participant::kTime);
    if (guest::direct<0x44f960, frontend::menub::recordsTimeQualifies>(app, ts, raceTime) != 0)
    {
        guest.copyString(newRecord + record::kDriver, driver + kDriverName);
        guest.copyString(newRecord + record::kCarName, *fe::direct<0x439250, load::nameOf>(app, ts, car));
        const auto carLine = [&](std::int32_t n) {
            return u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4, car, n));
        };
        guest.copyString(newRecord + record::kCarLine0, carLine(0));
        guest.copyString(newRecord + record::kCarLine1, carLine(1));
        guest.setDword(newRecord + record::kTime, guest.dword(found + participant::kTime));
        guest.setDword(newRecord + record::kSpeed, guest.dword(found + participant::kTopSpeed));
        std::uint16_t aids = 0;
        for (u32 GameSettings::*aid : kAidsOn)
            aids = std::uint16_t(aids | std::uint16_t(gameSettings(guest).*aid));
        guest.setWord(newRecord + record::kAids, aids);
        guest.setWord(newRecord + record::kNew, 1);

        guest::direct<0x44f830, frontend::menub::recordsSortBy>(app, ts, kSortByTime);
        const u32 best = guest::direct<0x44f910, frontend::menub::recordByRank>(app, ts, 0);
        if (best != 0
            && signedDword(guest, best + record::kTime) > signedDword(guest, found + participant::kTime))
            entered = kEnteredBestTime;
        else
            entered |= kEnteredTime;

        // By value on the stack in the game's code; the core reads it where it is.
        fe::direct<0x44fa10, frontend::menub::recordsInsert>(app, ts, newRecord);
    }

    // Its best lap.
    std::int32_t bestLap = 0;
    for (std::int32_t lap = 0; lap < std::int32_t(gameSettings(guest).laps); ++lap)
    {
        const std::int32_t time = signedDword(guest, found + participant::kLapTimes + 4 * lap);
        if (time != 0 && (time < bestLap || bestLap == 0))
            bestLap = time;
    }
    if (bestLap != 0 && (std::int32_t(holderLap) > bestLap || holderLap == 0))
    {
        const u32 holderDriver = guest.frame() + 0x31c;
        const u32 holderLine0 = guest.frame() + 0x61c;
        const u32 holderLine1 = guest.frame() + 0x41c;
        guest.copyString(holderDriver, driver + kDriverName);
        const auto carLine = [&](std::int32_t n) {
            return u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4, car, n));
        };
        guest.copyString(holderLine0, carLine(0));
        guest.copyString(holderLine1, carLine(1));
        guest::direct<0x44fb30, frontend::menub::recordsSetHolder>(app, ts, holderDriver, holderLine0, holderLine1,
                                                                   u32(bestLap), 1);
        entered |= kEnteredBestLap;
    }
    return entered;
}

/* sub_4512f0: the player's race, and a second player's beside it, entered; and
 * what they got turned into awards. */
u32 recordsEnterPlayersBody(App* app, ThreadState& ts, Guest& guest)
{
    const u32 lapsClass = u32(std::int16_t(fe::direct<0x4cf160, frontend::menue::lapsToOption>(
        app, ts, u32(std::int32_t(std::int16_t(gameSettings(guest).laps))))));
    fe::direct<0x44f670, frontend::misc::recordsSelect>(app, ts, gameSettings(guest).track, lapsClass);
    // What the calls leave in eax, the last one's: the cores called as C++ hand theirs back.
    u32 left = 0;
    const auto enter = [&](u32 id, u32 player) {
        const u32 found = left = fe::direct<0x4554e0, frontend::misc::resultsParticipantById>(app, ts, 0, id);
        if (found == 0 || guest.dword(found + participant::kState) == kStateDidNotFinish)
            return;
        const u32 entered = left =
            guest::direct<0x450fe0, frontend::menub::recordsEnterResult>(app, ts, found);
        if (entered & kEnteredBestLap)
            left = guest::direct<0x44e7f0, frontend::menub::awardsGrant>(app, ts, 0, player);
        if (entered & kEnteredTime)
            left = guest::direct<0x44e7f0, frontend::menub::awardsGrant>(app, ts, 2, player);
        if (entered & kEnteredBestTime)
            left = guest::direct<0x44e7f0, frontend::menub::awardsGrant>(app, ts, 1, player);
    };
    enter(gameSettings(guest).localPlayer, 0);
    if (gameSettings(guest).playMode == 1)
        enter(1, 1);
    return left;
}

/* ---------------------------------------------------------------------------
 * Text in a column
 * ------------------------------------------------------------------------- */

/* sub_452090: a text cut to a width in a font, its end made "..." when it was
 * cut.  The text is changed where it is. */
u32 textFitBody(App* app, ThreadState& ts, Guest& guest, u32 text, std::int32_t most, u32 font)
{
    std::uint32_t length = guest.length(text);
    fe::direct<0x452100, port::text::selectFontOf>(app, ts, font);   // the game's code: the font in edx too, unread
        u32 left = u32(*fe::direct<kTextWidth, port::text::menuTextWidth>(app, ts, ts.sp - 4, text, font));
    if (std::int32_t(left) <= most)
        return left;
    while (length != 0)
    {
                left = u32(*fe::direct<kTextWidth, port::text::menuTextWidth>(app, ts, ts.sp - 4, text, font));
        if (std::int32_t(left) <= most)
            break;
        --length;
        left = text + length;
        guest.setByte(left, 0);
    }
    if (std::int32_t(length) > 3)
    {
        guest.setByte(text + length - 1, '.');
        guest.setByte(text + length - 2, '.');
        left = text + length - 3;
        guest.setByte(left, '.');
    }
    return left;
}

/* sub_452120: a copy of a text, fitted to a width, drawn. */
u32 textDrawFittedBody(App* app, ThreadState& ts, Guest& guest, u32 text, u32 x, u32 y, u32 font, u32 align,
                         u32 colour, u32 most)
{
    const u32 copy = guest.frame();
    guest.copyString(copy, text);
    guest::direct<0x452090, frontend::menub::textFit>(app, ts, copy, most, font);
        return u32(*fe::direct<kDrawMenuText, port::text::menuTextDrawn>(app, ts, ts.sp - 12, copy, x, y, font, align,
                                                                       colour));
}

/* ---------------------------------------------------------------------------
 * The codes typed on any screen
 * ------------------------------------------------------------------------- */

constexpr u32 kCodes = 0x5594e4;              // char*[21], each letter 9 past its own
constexpr u32 kCodeProgress = 0x6645f0;       // int[21]
constexpr std::uint32_t kCodeCount = 21;
constexpr std::int32_t kCodeLetterShift = 9;
constexpr u32 kJumpState = 0x559538;          // "GO" and two digits: a track at once
constexpr u32 kJumpTrack = 0x55953c;
constexpr std::int32_t kLastTrackJump = 0x15;
constexpr std::uint32_t kCodeEffect = 5, kCodeVolume = 0x7f;

/* sub_452420: a key against a code: its place in the code moved on, or back to
 * its start.  1 when the code is done.  Ebx left at the next letter. */
u32 cheatAdvanceBody(Guest& guest, u32 key, u32 progress, u32 code)
{
    const std::int32_t letter = std::int32_t(fe::direct<kToUpper, watcom::clib::toupper>(guest.app(), guest.ts(), key)) + kCodeLetterShift;
    if (letter != std::int32_t(guest.byte(code + guest.dword(progress))))
    {
        guest.setDword(progress, 0);
        return 0;
    }
    const u32 next = guest.dword(progress) + 1;
    guest.setDword(progress, next);
    return guest.byte(code + next) == 0 ? 1 : 0;
}

/* sub_4524e0: a key typed on any screen, in a game of one or two at a single
 * race or a duel: "GO" and a number, a track; and the codes, each its own
 * unlock.  Those of a car have it chosen, in the kinds of race it is for. */
u32 cheatKeyBody(App* app, ThreadState& ts, Guest& guest, u32 key)
{
    if (key == 0)
        return key;
    const u32 players = gameSettings(guest).playMode;
    if (players != 0 && players != 1)
        return key;
    const u32 raceType = gameSettings(guest).raceType;
    if (raceType != 3 && raceType != 0)
        return key;

    // What the calls leave in eax, the last one's: the core called as C++ hands its back.
    u32 left = fe::direct<kToUpper, watcom::clib::toupper>(guest.app(), guest.ts(), key);
    const std::int32_t letter = std::int32_t(left);
    const auto effect = [&]() {
        left = u32(*fe::direct<0x4181d0, port::playMenuEffectOf>(app, ts, ts.sp - 4, kCodeEffect, kCodeVolume));
    };
    const auto isDigit = [&]() { return letter >= '0' && letter <= '9'; };
    const u32 state = guest.dword(kJumpState);
    switch (state)
    {
    case 0:
        if (letter == 'G')
            guest.setDword(kJumpState, state + 1);
        break;
    case 1:
        guest.setDword(kJumpState, letter == 'O' ? state + 1 : 0);
        break;
    case 2:
        if (isDigit())
        {
            guest.setDword(kJumpState, state + 1);
            guest.setDword(kJumpTrack, u32((letter - '0') * 10));
        }
        else
        {
            guest.setDword(kJumpState, 0);
        }
        break;
    case 3:
        if (isDigit())
        {
            const std::int32_t track = signedDword(guest, kJumpTrack) + letter - '0';
            guest.setDword(kJumpTrack, u32(track));
            if (track >= 0 && track <= kLastTrackJump)
            {
                effect();
                gameSettings(guest).raceFlagsByte = std::uint8_t(gameSettings(guest).raceFlags) | 2;
                gameSettings(guest).cheatTrack = guest.dword(kJumpTrack);
            }
        }
        guest.setDword(kJumpState, 0);
        break;
    default:
        break;
    }

    u32 carGroup = 0;
    const auto unlockCar = [&](std::uint8_t bit, u32 group) {
        if ((std::uint8_t(gameSettings(guest).unlockedCars) & bit) != 0)
            return;
        effect();
        gameSettings(guest).unlockedCarsByte = std::uint8_t(gameSettings(guest).unlockedCars) | bit;
        carGroup = group;
    };
    const auto choose = [&](std::uint8_t bit) {
        effect();
        gameSettings(guest).raceFlagsByte = std::uint8_t(gameSettings(guest).raceFlags) | bit;
    };
    for (u32 i = 0; i < kCodeCount; ++i)
    {
        left = guest::direct<0x452420, frontend::menub::cheatAdvance>(app, ts, u32(letter), kCodeProgress + 4 * i,
                                                                      guest.dword(kCodes + 4 * i));
        if (left == 0)
            continue;
        switch (i)
        {
        case 0: unlockCar(0x01, 1); break;
        case 1: unlockCar(0x02, 2); break;
        case 2: unlockCar(0x04, 3); break;
        case 3: unlockCar(0x08, 4); break;
        case 4: unlockCar(0x10, 5); break;
        case 5:
            effect();
            gameSettings(guest).unlockedCarsByte = std::uint8_t(gameSettings(guest).unlockedCars) | 0x1f;
            carGroup = 1;
            break;
        case 6:
            effect();
            gameSettings(guest).unlockedAll = 1;
            gameSettings(guest).track = 8;
            left = fe::direct<0x442b60, frontend::misc::postgameUnlockTrack>(app, ts, 8);
            break;
        case 7: choose(0x01); break;
        case 9: choose(0x02); break;
        case 10: choose(0x04); break;
        case 13: choose(0x08); break;
        case 19: choose(0x10); break;
        case 20: choose(0x20); break;
        default:
            break;
        }
    }
    if (carGroup == 0)
        return left;

    fe::direct<0x439d90, load::loadCars>(app, ts);
    const bool duel = gameSettings(guest).raceType == 3;
    u32 carNumber = 0;
    bool taken = false;
    switch (carGroup)
    {
    case 1: taken = true; carNumber = 0x27; break;
    case 2: taken = !duel; carNumber = 0x12; break;
    case 3: taken = true; carNumber = 0x11; break;
    case 4: taken = duel; carNumber = 0x24; break;
    case 5: taken = duel; carNumber = 0x31; break;
    default: break;
    }
    const u32 id = guest.frame();
    fe::direct<0x438ad0, load::makeId>(app, ts, id, carNumber);
    const u32 car = fe::direct<0x439990, load::placeById>(app, ts, id);
    if (taken)
        gameSettings(guest).players[0].car = car;
    return car;
}

/* ---------------------------------------------------------------------------
 * The showcase's car sheets
 * ------------------------------------------------------------------------- */

constexpr u32 kArtFolder = 0x7a2db4;
constexpr u32 kShowcaseCar = 0x664798;
constexpr u32 kStatsPage = 0x55957c;
constexpr u32 kSpecsSelected = 0x55958c;
constexpr u32 kCarViewMode = 0x559578;
constexpr std::int32_t kStatTitleX = 0xb4;
constexpr std::int32_t kStatValueX = 0x1d6;
constexpr std::int32_t kRowStep = 0xf;

/* sub_4530c0: whether a car has its art: its file exists, in the showcase's
 * folder or the cars'. */
u32 carArtExistsBody(App* app, ThreadState& ts, Guest& guest, u32 car)
{
    constexpr u32 kOwnFolderFormat = 0x539684;    // "%s%s\\SH%s.qfs"
    constexpr u32 kCarsFormat = 0x539694;         // "%sSH%s.qfs"
    constexpr u32 kShowcaseFolder = 0x7a28b4;
    constexpr u32 kCarsFolder = 0x7a3234;
    const u32 folder = *fe::direct<0x4392b0, load::folderOf>(app, ts, car);
    if (gameSettings(guest).installIsRemote != 0 || folder == 0)
        return 0;
    const u32 path = guest.frame();
    if (*fe::direct<0x4395c0, load::flag4Of>(app, ts, car) != 0)
    {
        const u32 stem = fe::direct<0x439290, frontend::showcase::carFileStem>(app, ts, car);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kOwnFolderFormat, guest::Words{kShowcaseFolder, folder, stem});
    }
    else
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kCarsFormat, guest::Words{kCarsFolder, folder});
    }
    return *fe::direct<0x4e0e60, ea::fileThere>(app, ts, path) ? 1 : 0;
}

/* sub_4534d0: the screen left remembered for the car screen's way back, but for
 * the comparison's two. */
constexpr GuestString kCompareNames[] = { { 0x539728, "compare" }, { 0x539730, "compare2" } };

u32 carViewRememberBody(Guest& guest, u32 name)
{
    const u32 first = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kCompareNames[0].address);
    if (first == 0)
        return 0;
    const u32 second = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kCompareNames[1].address);
    if (second == 0)
        return 0;
    guest.setDword(kCarViewMode, name);
    return second;
}

/* A statistic's row: its title from the language's, its value from the car's text.  What the
 * last call leaves in eax. */
u32 statRow(App* app, ThreadState& ts, Guest& guest, bool grey, std::int32_t y, u32 title, u32 line)
{
    const std::uint32_t colour = grey ? kGreyedOut : kWhite;
    const u32 titleText = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, title, 0));
    fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, titleText, u32(kStatTitleX), u32(y),
                                                    kMenuFont, 0, colour);
    const u32 value = u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4,
                                                                             guest.dword(kShowcaseCar), line));
    return u32(*fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, value, u32(kStatValueX),
                                                                  u32(y), kMenuFont, 1, colour));
}

/* sub_453940: a page of the car's statistics. */
u32 carStatsDrawBody(App* app, ThreadState& ts, Guest& guest)
{
    const bool grey = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        fe::direct<0x431900, graphics::sendState>(app, ts, 6, 1);
    std::int32_t y = 0x9a;
    // What the calls leave in eax, the last one's: the cores hand theirs back.
    u32 left = 0;
    const auto rows = [&](u32 count, u32 firstTitle, u32 firstLine) {
        for (u32 i = 0; i < count; ++i)
        {
            left = statRow(app, ts, guest, grey, y, firstTitle + i, firstLine + i);
            y += kRowStep;
        }
    };
    const u32 page = guest.dword(kStatsPage);
    switch (page)
    {
    case 0:
        rows(7, 0x160, 3);
        break;
    case 1:
    {
        rows(3, 0x168, 0x11);
        const u32 header = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, 0x16b, 0));
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, header, 0xa2, u32(y), kMenuFont, 0,
                                                        grey ? kGreyedOut : kYellow);
        y += kRowStep;
        rows(2, 0x16c, 0x14);
        break;
    }
    case 2:
        rows(7, 0x16f, 0xa);
        return 7;
    default:
        return page;
    }
    return left;
}

/* sub_453f60: the car's specifications, the one selected lit. */
u32 carSpecsDrawBody(App* app, ThreadState& ts, Guest& guest)
{
    const bool grey = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        fe::direct<0x431900, graphics::sendState>(app, ts, 6, 1);
    std::int32_t y = 0x87;
    // What the calls leave in eax, the last one's: the core hands its back.
    u32 left = 0;
    for (u32 i = 0; i < 7; ++i)
    {
        const std::uint32_t colour = grey ? kGreyedOut : i == guest.dword(kSpecsSelected) ? kYellow : kWhite;
        const u32 value = u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4,
                                                                                 guest.dword(kShowcaseCar), 0x17 + i));
        left = u32(*fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, value, u32(kStatValueX),
                                                                      u32(y), kMenuFont, 1, colour));
        y += kRowStep;
    }
    return left;
}

constexpr u32 kCarFilter = 0x559570;          // 0, 1 all; 2, 3, 4 a class
constexpr u32 kCarListScroll = 0x5595d4;
constexpr u32 kExtraCars = 0x554f50;
constexpr u32 kCarListStat = 0x559574;
constexpr u32 kCarStatIds = 0x559598;         // int[4]
constexpr u32 kBarLong = 0x5595b4;
constexpr u32 kBarMiddle = 0x5595b8;
constexpr u32 kBarShort = 0x5595bc;
constexpr u32 kBarLeast = 0x5595c0;
constexpr u32 kCompareTitles = 0x5595c4;      // int[4]: the language's
constexpr u32 kCompareLeftEmblem = 0x5595ac;
constexpr u32 kCompareRightEmblem = 0x5595b0;
constexpr u32 kCompareLeftCar = 0x664788;
constexpr u32 kCompareRightCar = 0x664784;
constexpr std::int32_t kStatMost = 0x14;
constexpr std::int32_t kListRows = 0x12;
constexpr std::int32_t kClipBottom = 0x1e0;
constexpr std::int32_t kClipRight = 0x280;
constexpr u32 kArrowLeftOff = 0x66479c;       // int16s: the list's arrows
constexpr u32 kArrowLeftOn = 0x6647a0;
constexpr u32 kArrowRightOff = 0x6647a2;
constexpr u32 kArrowRightOn = 0x66479e;

/* sub_454730: whether a car is in the class shown (sub_439730's, of the car in eax as it came). */
u32 carPassesFilterBody(Guest& guest, u32 car)
{
    switch (guest.dword(kCarFilter))
    {
    case 0:
    case 1:
        return 1;
    case 2:
        return *fe::direct<0x439730, load::classOfCar>(guest.app(), guest.ts(), car) == 0 ? 1 : 0;
    case 3:
        return *fe::direct<0x439730, load::classOfCar>(guest.app(), guest.ts(), car) == 1 ? 1 : 0;
    case 4:
        return *fe::direct<0x439730, load::classOfCar>(guest.app(), guest.ts(), car) == 2 ? 1 : 0;
    default:
        return 0;
    }
}

/* sub_454780: the list of the class's cars, each with a bar of the statistic
 * chosen, scrolled by two arrows pressed with the pointer. */
u32 carListDrawBody(App* app, ThreadState& ts, Guest& guest)
{
    const bool grey = u32(*fe::direct<0x449df0, port::loop::keysTakenOf>(app, ts, ts.sp - 4, 0)) != 0;
    const std::uint32_t colour = grey ? kGreyedOut : kYellow;
    const std::int32_t cars = std::int32_t(fe::direct<0x438bd0, load::carsListed>(app, ts) - guest.dword(kExtraCars));
    std::int32_t inClass = 0;
    for (std::int32_t car = 0; car < cars; ++car)
    {
        if (guest::direct<0x454730, frontend::menub::carPassesFilter>(app, ts, u32(car)) != 0)
            ++inClass;
    }
    // An arrow, and what its last call leaves in eax.
    const auto arrow = [&](u32 off, u32 on, u32 y, std::int32_t step) {
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, u32(signedWord(guest, off)), 0xb9, y);
        const u32 over = fe::direct<0x49c240, frontend::menud::pointerOverRect>(app, ts, 0xb7, y - 2, 0xf, 0xf);
        if (over == 0)
            return over;
        const auto drawn = fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4,
                                                                           u32(signedWord(guest, on)), 0xb9, y);
        if (guest.dword(kPointerPressed) != 0)
            guest.setDword(kCarListScroll, guest.dword(kCarListScroll) + u32(step));
        return u32(*drawn);
    };
    if (signedDword(guest, kCarListScroll) + inClass < kListRows)
        guest.setDword(kCarListScroll, 0);
    else if (signedDword(guest, kCarListScroll) > 0)
        arrow(kArrowLeftOff, kArrowLeftOn, 0x9d, -1);
    u32 left = guest.dword(kCarListScroll) + kListRows;
    if (std::int32_t(left) < inClass)
        left = arrow(kArrowRightOff, kArrowRightOn, 0x19c, 1);

    std::int32_t shown = 0;
    std::int32_t y = 0x9b;
    for (std::int32_t car = signedDword(guest, kCarListScroll); car < cars; ++car)
    {
        left = guest::direct<0x454730, frontend::menub::carPassesFilter>(app, ts, u32(car));
        if (left == 0 || shown >= kListRows)
            continue;
        ++shown;
        const u32 name = u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4, u32(car), 2));
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, name, 0xc8, u32(y), kMenuFont, 0,
                                                        colour);
        const u32 statistic = guest.dword(kCarStatIds + guest.dword(kCarListStat) * 4);
        std::int32_t stat = std::int32_t(fe::direct<0x438f60, frontend::loada::carStat>(app, ts, u32(car), statistic));
        if (stat > kStatMost)
            stat = kStatMost;
        std::int32_t right, x;
        if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        {
            right = std::int32_t(u32(stat) * 0x9a) / kStatMost + 0x18c;
            x = 0x159;
        }
        else
        {
            right = std::int32_t(u32(stat) * 0xbd) / kStatMost + 0x184;
            x = 0x145;
        }
        fe::direct<0x4bebe0, graphics::clipSet>(app, ts, 0, 0, u32(right), u32(kClipBottom));
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, guest.dword(kBarMiddle), u32(x),
                                                        u32(y));
        left = *fe::direct<0x4bebe0, graphics::clipSet>(app, ts, 0, 0, u32(kClipRight), u32(kClipBottom));
        y += kRowStep;
    }
    return left;
}

/* sub_4549c0: two cars' statistics compared: four rows of two bars, growing
 * from the middle, the better one's bright. */
u32 carCompareDrawBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    // By address: the widget's type it hands in edx, which the core does not take, is the edx the
    // function leaves when the comparison is not shown.
    if (fe::direct<kMenuShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;
    std::int32_t left[4], right[4];
    for (u32 i = 0; i < 4; ++i)
    {
        const auto stat = [&](u32 car) {
            const std::int32_t value = std::int32_t(fe::direct<0x438f60, frontend::loada::carStat>(
                app, ts, guest.dword(car), guest.dword(kCarStatIds + 4 * i)));
            return value > kStatMost ? kStatMost : value;
        };
        left[i] = stat(kCompareLeftCar);
        right[i] = stat(kCompareRightCar);
    }
    const auto barLength = [](std::int32_t stat) { return std::int32_t(u32(stat) * 100) / kStatMost; };
    for (u32 i = 0; i < 4; ++i)
    {
        const bool hardware = (guest.byte(kGraphicsFlags) & kHardware3D) != 0;
        const std::int32_t rowY = std::int32_t(0x127 + i * 30);
        const u32 title = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4,
                                                                              guest.dword(kCompareTitles + 4 * i), 0));
        fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 12, title, 0x17a,
                                                        u32(rowY - (hardware ? 3 : 0)), kMenuFont, 2, kYellow);
        u32 leftBar = guest.dword(kBarMiddle);
        u32 rightBar = guest.dword(kBarLeast);
        if (right[i] > left[i])
            leftBar = guest.dword(kBarLong);
        else if (right[i] < left[i])
            rightBar = guest.dword(kBarShort);

        const std::int32_t leftEdge = 0x147 - barLength(left[i]);
        fe::direct<0x4bebe0, graphics::clipSet>(app, ts, u32(leftEdge), 0, u32(kClipRight - leftEdge), u32(kClipBottom));
        const u32 barY = u32((guest.byte(kGraphicsFlags) & kHardware3D) != 0 ? rowY + 0xe : rowY);
        fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, leftBar, 0xd6, barY);
        fe::direct<0x4bebe0, graphics::clipSet>(app, ts, 0, 0, u32(barLength(right[i]) + 0x1af), u32(kClipBottom));
        if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
            fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, rightBar, 0x181, u32(rowY + 0xe));
        else
            fe::direct<0x4d7560, frontend::menue::drawArtTiles>(app, ts, rightBar, 0x154, u32(rowY));
        fe::direct<0x4bebe0, graphics::clipSet>(app, ts, 0, 0, u32(kClipRight), u32(kClipBottom));
    }
        const u32 leftEmblem = guest.dword(kCompareLeftEmblem);
    if (leftEmblem != 0)
        fe::direct<kDrawShape, port::shapes::drawShapeOf>(guest.app(), guest.ts(), guest.ts().sp - 4, leftEmblem, 0x7a,
                                                          0x4e);
    const u32 rightEmblem = guest.dword(kCompareRightEmblem);
    if (rightEmblem != 0)
        fe::direct<kDrawShape, port::shapes::drawShapeOf>(guest.app(), guest.ts(), guest.ts().sp - 4, rightEmblem,
                                                          0x163, 0x4e);
    return 0;
}

/* sub_454bf0: the bars' art loaded as the comparison opens: four shapes on 3D
 * hardware, two without. */
constexpr GuestString kBarNames[] = {
    { 0x5398a4, "lfrd" }, { 0x5398ac, "lfgr" }, { 0x5398b4, "gtrd" },
    { 0x5398bc, "gtgr" }, { 0x5398c4, "redb" }, { 0x5398cc, "grnb" },
};

u32 carCompareLoadArtBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) == 0)
        return 0;
    constexpr u32 kArtFormat = 0x539864;          // "%scompare.fsh"
    const u32 path = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kArtFormat, guest::Words{kArtFolder});
    const u32 art = u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0));
    const auto bar = [&](const GuestString& name) {
        const u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, name.address);
        return u32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, 0, 0));
    };
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
    {
        guest.setDword(kBarLong, bar(kBarNames[0]));
        guest.setDword(kBarMiddle, bar(kBarNames[1]));
        guest.setDword(kBarShort, bar(kBarNames[2]));
        const u32 least = bar(kBarNames[3]);
        guest.setDword(kBarLeast, least);
        return least;
    }
    guest.setDword(kBarLong, bar(kBarNames[4]));
    const u32 green = bar(kBarNames[5]);
    guest.setDword(kBarMiddle, green);
    guest.setDword(kBarLeast, green);
    guest.setDword(kBarShort, guest.dword(kBarLong));
    return green;
}

/* ---------------------------------------------------------------------------
 * The race's results as the menus keep them
 * ------------------------------------------------------------------------- */

constexpr u32 kRaceCars = 0x5efac8;           // the race's cars' records
constexpr u32 kRaceCarCount = 0x5efd9c;
constexpr u32 kNetworkDropped = 0x7a22ca;     // int16 for each driver
constexpr u32 kResultsOfRace = 0x6647b0;
constexpr u32 kResultsOfLaps = 0x66e270;
constexpr u32 kLapResultCount = 0x677d34;
constexpr u32 kTournamentPointsTable = 0x454f10;  // int16 for each place, in the code's own data
constexpr std::uint32_t kPlayerWords = 0x20;

/* sub_455020: a network player's car that has dropped out: left out of the results. */
u32 resultsHiddenRemoteBody(Guest& guest, u32 car)
{
    if (car != 0 && (guest.byte(car + participant::kFlags) & participant::kRemoteSide) != 0
        && std::int32_t(gameSettings(guest).playMode) > 1
        && guest.word(kNetworkDropped + 2 * guest.dword(car + participant::kDriver)) != 0)
        return 1;
    return 0;
}

/* sub_455070: a tournament's race scored: a place's points for each player and
 * the total, then the order of the totals.  Not once the finishers have gone. */
u32 tournamentScoreBody(App* app, ThreadState& ts, Guest& guest)
{
    u32 left = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts);
    if (left != 0)
        return left;
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 0);
    const auto slotOf = [&](u32 found) {
        return guest.dword(guest.dword(found + participant::kCar) + kCarSlot);
    };
    for (std::int32_t place = 0; place < signedDword(guest, kResultCount); ++place)
    {
        const u32 found = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(place), 0, 0);
        if (found == 0)
            continue;
        const std::uint16_t points = guest.word(kTournamentPointsTable + 2 * place);
        gameSettings(guest).tournament.raceScores[slotOf(found)] = points;
        std::int16_t& total = gameSettings(guest).tournament.totals[slotOf(found)];
        total = std::int16_t(std::uint16_t(std::uint16_t(total) + points));
    }
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), SETTINGS_ADDRESS(tournament.places), kPlayerWords);
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 1);
    std::uint16_t place = 0;
    for (u32 i = 0; i <= 8; ++i)
    {
        left = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, i, 0, 0);
        if (left != 0)
        {
            left = slotOf(left);
            gameSettings(guest).tournament.places[left] = place++;
        }
    }
    gameSettings(guest).tournament.stage = std::uint16_t(std::uint16_t(gameSettings(guest).tournament.stage) + 1);
    return left;
}

/* sub_455140: a knockout's round: the last of those still in is out, and the
 * order of those left written down. */
u32 knockoutEliminateBody(App* app, ThreadState& ts, Guest& guest)
{
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 1);   // ecx 2 too: the next call's
    const u32 last = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, 0, 0x20, 2);
    const auto slotOf = [&](u32 found) {
        return guest.dword(guest.dword(found + participant::kCar) + kCarSlot);
    };
    if (last != 0 && signedDword(guest, kResultCount) > 1)
    {
        gameSettings(guest).knockout.stillIn[slotOf(last)] = 0;
        guest.setDword(last + participant::kState, kStateDisqualified);
    }
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), SETTINGS_ADDRESS(knockout.places), kPlayerWords);
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 1);
    u32 left = 0;
    std::uint16_t place = 0;
    for (u32 i = 0; i <= 8; ++i)
    {
        const u32 found = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, i, 0, 0);
        left = found;
        if (found == 0)
            continue;
        const bool inTable = (guest.byte(found + participant::kFlags) & participant::kInKnockoutTable) != 0;
        left = slotOf(found);
        if (inTable && gameSettings(guest).knockout.stillIn[left] == 0)
            continue;
        gameSettings(guest).knockout.places[left] = place++;
    }
    gameSettings(guest).knockout.stage = std::uint16_t(std::uint16_t(gameSettings(guest).knockout.stage) + 1);
    return left;
}

/* sub_455210, sub_455280: the race's cars with a flag copied whole into a table
 * of the results, but for those that have dropped out. */
u32 resultsCollectBody(App* app, ThreadState& ts, Guest& guest, u32 table, u32 count, std::uint8_t flag)
{
    guest.setDword(count, 0);
    u32 left = table;
    for (std::int32_t i = 0; i < signedDword(guest, kRaceCarCount); ++i)
    {
        const u32 car = guest.dword(kRaceCars + 4 * i);
        left = car;
        if ((guest.byte(car + participant::kFlags) & flag) == 0)
            continue;
        left = guest::direct<0x455020, frontend::menub::resultsHiddenRemote>(app, ts, car);
        if (left != 0)
            continue;
        copyDwords(guest, table + guest.dword(count) * participant::kBytes, guest.dword(kRaceCars + 4 * i),
                   participant::kBytes / 4);
        left = participant::kBytes;
        guest.setDword(count, guest.dword(count) + 1);
    }
    return left;
}

/* sub_4552f0: the results settled: in a knockout those out are marked out; those
 * lapped get their times after the last's; then everyone's place. */
u32 resultsAssignBody(App* app, ThreadState& ts, Guest& guest)
{
    constexpr std::uint32_t kPlaceFlagLapped = 0x10;
    if (gameSettings(guest).raceType == 2)
    {
        fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 1);
        for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
        {
            const u32 found = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(i), 0, 0);
            if (found != 0 && (guest.byte(found + participant::kFlags) & participant::kOut) != 0)
                guest.setDword(found + participant::kState, kStateDisqualified);
        }
    }
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 1);   // ecx 2 too: the next call's
    const u32 lastIn = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, 0, 0x20, 2);
    if (lastIn != 0)
    {
        u32 time = guest.dword(lastIn + participant::kTime) + 0x3c;
        for (const u32 state : { 2u, 4u })
        {
            for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
            {
                const u32 found = fe::direct<0x455cd0, frontend::misc::resultsNth>(
                    app, ts, 0, u32(i), kPlaceFlagLapped, state);
                if (found != 0)
                {
                    ++time;
                    guest.setDword(found + participant::kTime, time);
                }
            }
        }
    }
    // What the calls leave in eax, the last one's: the cores hand theirs back.
    u32 left = fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 1, 0);
    for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
    {
        const u32 found = left = fe::direct<0x455cd0, frontend::misc::resultsNth>(app, ts, 0, u32(i), 0, 0);
        if (found != 0)
            guest.setDword(found + participant::kPlace, u32(i + 1));
    }
    return left;
}

/* sub_455410: the race's results made, as the race ends: the awards cleared,
 * the results copied and sorted, a tournament's and a knockout's scored, the
 * records entered and saved. */
u32 resultsBuildBody(App* app, ThreadState& ts, Guest& guest)
{
    if (gameSettings(guest).replayMode != 0)
        return 0;   // eax as the caller had it, which none reads
    fe::direct<0x4c7ea0, frontend::neta::sendRaceResults>(app, ts);
    guest::direct<0x44e7d0, frontend::menub::awardsClear>(app, ts);
    fe::direct<0x44f510, load::recordsTableLoaded>(app, ts);
    guest::direct<0x455210, frontend::menub::resultsCollectFinishers>(app, ts, kResultsOfRace, kResultCount);
    // The game's code hands it 0 in ebx too, which it does not read.
    guest::direct<0x455280, frontend::menub::resultsCollectLaps>(app, ts, kResultsOfLaps, kLapResultCount);
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 0, 0, 0);
    fe::direct<0x455c10, frontend::misc::resultsSort>(app, ts, 1, 0, 0);
    guest::direct<0x4552f0, frontend::menub::resultsAssign>(app, ts);
    if (gameSettings(guest).raceType == 1)
        guest::direct<0x455070, frontend::menub::tournamentScore>(app, ts);
    if (gameSettings(guest).raceType == 2)
        guest::direct<0x455140, frontend::menub::knockoutEliminate>(app, ts);
    guest::direct<0x44f9b0, frontend::menub::recordsClearMarks>(app, ts);
    if (fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts) == 0)
    {
        guest::direct<0x4512f0, frontend::menub::recordsEnterPlayers>(app, ts);
        guest::direct<0x44e820, frontend::menub::awardsForRace>(app, ts, 0);
        if (gameSettings(guest).playMode == 1)
            guest::direct<0x44e820, frontend::menub::awardsForRace>(app, ts, 1);
    }
    // What the calls leave in eax, the last one's: the cores hand theirs back, the network's nothing
    // leaves it as it came.
    u32 left = *fe::direct<0x44f5f0, load::recordsTableSaved>(app, ts);
    if (std::int32_t(gameSettings(guest).playMode) > 1)
    {
        left = fe::direct<0x4cef80, frontend::misc::postgameFinishers>(app, ts);
        if (left == 0)
            fe::direct<0x455060, frontend::menub::resultsNetworkNothing>(app, ts);
    }
    return left;
}

/* The participant's driver, with what the code leaves in edx on the way. */
u32 driverRecord(Guest& guest, u32 found)
{
    const u32 driver = guest.dword(found + participant::kDriver);
    return SETTINGS_ADDRESS(entries) + driver * kDriverBytes;
}

/* sub_455570: a participant's name into a buffer; empty for none. */
u32 resultsNameBody(Guest& guest, u32 found, u32 buffer)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const u32 driver = driverRecord(guest, found);
    guest.copyString(buffer, driver + kDriverName);
    return afterInlineCopy(driver);
}

/* sub_4555c0: a participant's place, in words. */
u32 resultsPlaceTextBody(App* app, ThreadState& ts, Guest& guest, u32 found, u32 buffer)
{
    const u32 text = u32(*fe::direct<0x4d1850, port::text::lineText>(
        app, ts, ts.sp - 4, guest.dword(found + participant::kPlace) + 0x2f, 0));
    guest.copyString(buffer, text);
    return afterInlineCopy(text);
}

/* sub_455600: a participant's car, by name. */
u32 resultsCarNameBody(App* app, ThreadState& ts, Guest& guest, u32 found, u32 buffer)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const u32 driver = driverRecord(guest, found);
    const u32 name = *fe::direct<0x439250, load::nameOf>(app, ts, guest.dword(driver + kDriverCar));
    guest.copyString(buffer, name);
    return afterInlineCopy(name);
}

/* sub_455660, sub_4556c0: a line of a participant's car's text. */
u32 resultsCarLineBody(Guest& guest, u32 found, u32 buffer, u32 line)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const u32 driver = driverRecord(guest, found);
        const u32 text = u32(*fe::direct<kCarText, port::fields::carTextOf>(guest.app(), guest.ts(), guest.ts().sp - 4,
                                                                            guest.dword(driver + kDriverCar), line));
    guest.copyString(buffer, text);
    return afterInlineCopy(text);
}

/* sub_455720: a participant's top speed written out, in the units chosen; "---"
 * when it has none. */
u32 resultsTopSpeedTextBody(App* app, ThreadState& ts, Guest& guest, u32 found, u32 buffer)
{
    constexpr u32 kToKmh = 0x539910, kToMph = 0x539914;       // floats
    constexpr u32 kSpeedFormat = 0x539900;                    // "%5.2f %s"
    constexpr u32 kNoSpeed = 0x53990c;                        // "---"
    const float speed = guest.floatAt(found + participant::kTopSpeed);
    const std::uint16_t status = fp::compare(nfs::Float(1.0), nfs::Float(speed));
    if (!(status & kStatusBelow))
        return fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kNoSpeed, guest::Words({}));
    const u32 units = gameSettings(guest).hud[0].speedometerForm;
    const bool metric = units == 3 || units == 4;
    const u32 unitName = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4,
                                                                             u32(metric ? 0x157 : 0x156), 0));
    const double value = double(fp::mul(nfs::Float(speed), nfs::Float(guest.floatAt(metric ? kToKmh : kToMph))));
    u32 words[2];
    std::memcpy(words, &value, sizeof words);
    return fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kSpeedFormat, guest::Words{words[0], words[1], unitName});
}

/* ---------------------------------------------------------------------------
 * The direction pad: an item (widget 0x22), eight arrows round a centre.
 * ------------------------------------------------------------------------- */

constexpr u32 kDirPadWidget = 0x22;
constexpr u32 kDirPadActive = 0x677da0;
constexpr u32 kDirPadX = 0x559874;            // int16[8], where the arrows are
constexpr u32 kDirPadY = 0x559884;            // int16[8]
constexpr u32 kDirPadShapes = 0x677da4;       // int[9]: the base of each, the centre last
constexpr u32 kDirPadLitShapes = 0x677dc8;    // int[9]
constexpr u32 kBaseX = 0x55982c;              // int16[9]
constexpr u32 kBaseY = 0x55983e;              // int16[9]
constexpr u32 kLitX = 0x559850;               // int16[9]
constexpr u32 kLitY = 0x559862;               // int16[9]
constexpr std::int32_t kPadCentreX = 0x22f;
constexpr std::int32_t kPadCentreY = 0x11b;
constexpr std::int32_t kNear = 0x10;
constexpr std::int32_t kPadOriginX = 0x226;
constexpr std::int32_t kPadOriginY = 0x112;

std::int32_t distanceBetween(std::int32_t from, std::int32_t to)
{
    const std::int32_t by = std::int32_t(u32(from) - u32(to));
    return by > 0 ? by : std::int32_t(0 - u32(by));
}

/* sub_457270: the arrow the pointer is on, or -1. */
u32 dirPadUnderPointerBody(Guest& guest)
{
    for (u32 i = 0; i < 8; ++i)
    {
        const std::int32_t across = distanceBetween(signedDword(guest, kPointerX), signedWord(guest, kDirPadX + 2 * i));
        const std::int32_t down = distanceBetween(signedDword(guest, kPointerY), signedWord(guest, kDirPadY + 2 * i));
        if (std::int32_t(u32(across) + u32(down)) < kNear)
            return i;
    }
    return u32(-1);
}

/* sub_4572d0: whether the pointer is on the centre. */
u32 dirPadCentreUnderPointerBody(Guest& guest)
{
    const std::int32_t across = distanceBetween(signedDword(guest, kPointerX), kPadCentreX);
    const std::int32_t down = distanceBetween(signedDword(guest, kPointerY), kPadCentreY);
    return std::int32_t(u32(across) + u32(down)) < kNear ? 1 : 0;
}

/* sub_457340: an arrow chosen: the direction it stands for handed on; -1 none.
 * Past them the code hands on its locals as they stand. */
u32 dirPadChooseBody(App* app, ThreadState& ts, Guest& guest, u32 arrow)
{
    constexpr u32 kOne = 0x3f800000, kMinusOne = 0xbf800000;
    struct Direction
    {
        u32 x, y;
    };
    static constexpr Direction kDirections[9] = {
        { 0, 0 },               { 0, kOne },        { kMinusOne, kOne },
        { kMinusOne, 0 },       { kMinusOne, kMinusOne }, { 0, kMinusOne },
        { kOne, kMinusOne },    { kOne, 0 },        { kOne, kOne },
    };
    const u32 index = arrow + 1;
    const Direction direction = index <= 8 ? kDirections[index]
                                           : Direction{ guest.dword(guest.frame()), guest.dword(guest.frame() + 4) };
    return guest::direct<0x4654c0, frontend::menuc::setSliderComponents>(app, ts, direction.x, direction.y);
}

/* One of the pad's shapes, where the item is.  By address: the pad's last call, whose edx the
 * function leaves. */
void drawDirPadShape(Guest& guest, u32 item, u32 shape, u32 xTable, u32 yTable, u32 index)
{
    const std::int32_t y = signedWord(guest, item + sheet::kY) - (kPadOriginY - signedWord(guest, yTable + 2 * index));
    const std::int32_t x = signedWord(guest, item + sheet::kX) - (kPadOriginX - signedWord(guest, xTable + 2 * index));
        fe::direct<kDrawShape, port::shapes::drawShapeOf>(guest.app(), guest.ts(), guest.ts().sp - 4,
                                                      guest.dword(shape + 4 * index), u32(x), u32(y));
}

/* sub_457460: the pad drawn: only what the pointer is on, base and lit. */
u32 dirPadDrawBody(App* app, ThreadState& ts, Guest& guest, u32 item, u32 active)
{
    guest.setDword(kDirPadActive, active);
    // By address: the widget's type it hands in edx, which the core does not take, is the edx the
    // function leaves when the pad is not shown.
    if (fe::direct<kMenuShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;
    constexpr u32 kCentre = 8;
    const u32 arrow = guest::direct<0x457270, frontend::menub::dirPadUnderPointer>(app, ts);
    if (arrow != u32(-1))
    {
        drawDirPadShape(guest, item, kDirPadShapes, kBaseX, kBaseY, arrow);
        drawDirPadShape(guest, item, kDirPadLitShapes, kLitX, kLitY, arrow);
    }
    else if (guest::direct<0x4572d0, frontend::menub::dirPadCentreUnderPointer>(app, ts) != 0)
    {
        drawDirPadShape(guest, item, kDirPadShapes, kBaseX, kBaseY, kCentre);
        drawDirPadShape(guest, item, kDirPadLitShapes, kLitX, kLitY, kCentre);
    }
    return 0;
}

/* sub_4575a0: its keys: Enter on an arrow chooses it, on the centre stops. */
u32 dirPadKeyBody(App* app, ThreadState& ts, Guest& guest, u32 item, std::uint16_t key)
{
    constexpr std::uint16_t kEnter = 0xd, kLeft = 0x4b00, kRight = 0x4d00, kSpace = 0x20;
    // By address: the widget's type it hands in edx, which the core does not take, is the edx the
    // function leaves when the pad is not shown or not active.
    if (fe::direct<kMenuShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;
    if (guest.dword(kDirPadActive) == 0)
        return 0;
    const u32 answer = key == kLeft || key == kRight || key == kSpace ? 2 : 0;
    if (key == kEnter)
    {
        const u32 arrow = guest::direct<0x457270, frontend::menub::dirPadUnderPointer>(app, ts);
        if (arrow != u32(-1))
            guest::direct<0x457340, frontend::menub::dirPadChoose>(app, ts, arrow);
        else if (guest::direct<0x4572d0, frontend::menub::dirPadCentreUnderPointer>(app, ts) != 0)
            guest::direct<0x4654c0, frontend::menuc::setSliderComponents>(app, ts, 0, 0);
    }
    return answer;
}

/* sub_457630: its box round the item. */
u32 dirPadLayoutBody(App* app, ThreadState& ts, Guest& guest, u32 item)
{
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) != 0)
    {
        constexpr std::uint16_t kSide = 0x3d;
        guest.setWord(item + 0x1e, kSide);
        guest.setWord(item + 0x1c, std::uint16_t(guest.word(item + sheet::kX) - 0x15));
        guest.setWord(item + 0x20, kSide);
        guest.setWord(item + 0x1a, std::uint16_t(guest.word(item + sheet::kY) - 0x16));
    }
    return 0;
}

/* sub_457680: its shapes by name: "int1" to "int9" and "inta" to "inti". */
u32 dirPadLoadShapesBody(App* app, ThreadState& ts, Guest& guest)
{
    const u32 name = guest.frame();
    guest.setByte(name, 'i');
    guest.setByte(name + 1, 'n');
    guest.setByte(name + 2, 't');
    guest.setByte(name + 4, 0);
    u32 left = 0;
    for (u32 i = 0; i < 9; ++i)
    {
        guest.setByte(name + 3, std::uint8_t('1' + i));
        const auto base = fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4, name);
        guest.setDword(kDirPadShapes + 4 * i, u32(*base));
        guest.setByte(name + 3, std::uint8_t('a' + i));
        left = u32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4, name));
        guest.setDword(kDirPadLitShapes + 4 * i, left);
    }
    return left;
}

}

nfs::u32 frontend::menub::menuDrawPass(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return menuDrawPassBody(app, ts, guest);
}

nfs::u32 frontend::menuDrawPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44bdd0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::menuDrawPass(app, ts); });
}

nfs::u32 frontend::menub::menuLeave(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return menuLeaveBody(app, ts, guest, eax);
}

nfs::u32 frontend::menuLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44be20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::menuLeave(app, ts, eax); });
}

nfs::u32 frontend::menub::awardsClear(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return awardsClearBody(guest);
}

nfs::u32 frontend::awardsClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e7d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::awardsClear(app, ts); });
}

nfs::u32 frontend::menub::awardsGrant(App* app, ThreadState& ts, u32 award, u32 player)
{
    Guest guest(app, ts);
    return awardsGrantBody(guest, award, player);
}

nfs::u32 frontend::awardsGrant(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e7f0", 0, 0, nullptr, 0, 0 };
    const u32 award = args.eax, player = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::awardsGrant(app, ts, award, player); });
}

nfs::u32 frontend::menub::awardsForRace(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return awardsForRaceBody(app, ts, guest, player);
}

nfs::u32 frontend::awardsForRace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e820", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::awardsForRace(app, ts, player); });
}

nfs::u32 frontend::menub::awardsSpeak(App* app, ThreadState& ts, u32 kind)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return awardsSpeakBody(app, ts, guest, kind);
}

nfs::u32 frontend::awardsSpeak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ea60", 0, 0, nullptr, 0, kEbxEcxEdxEdi };
    const u32 kind = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::awardsSpeak(app, ts, kind); });
}

nfs::u32 frontend::menub::awardsSpeakForSheet(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return awardsSpeakForSheetBody(app, ts, guest);
}

nfs::u32 frontend::awardsSpeakForSheet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ed30", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::awardsSpeakForSheet(app, ts); });
}

nfs::u32 frontend::menub::recordsCompare(App* app, ThreadState& ts, u32 slotA, u32 slotB)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return recordsCompareBody(guest, slotA, slotB);
}

nfs::u32 frontend::recordsCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f6e0", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 slotA = args.eax, slotB = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsCompare(app, ts, slotA, slotB); });
}

nfs::u32 frontend::menub::recordsSortBy(App* app, ThreadState& ts, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x28);
    return recordsSortByBody(guest, key);
}

nfs::u32 frontend::recordsSortBy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f830", 0, 0, nullptr, 0, kEbxEcxEdxEsi };
    const u32 key = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSortBy(app, ts, key); });
}

nfs::u32 frontend::menub::recordsCount(App* app, ThreadState& ts, u32 track, u32 lapsClass)
{
    Guest guest(app, ts);
    return recordsCountBody(guest, track, lapsClass);
}

nfs::u32 frontend::recordsCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f8d0", 0, 0, nullptr, 0, kEbxEcx };
    const u32 track = args.eax, lapsClass = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsCount(app, ts, track, lapsClass); });
}

nfs::u32 frontend::menub::recordByRank(App* app, ThreadState& ts, u32 rank)
{
    Guest guest(app, ts);
    return recordByRankBody(guest, rank);
}

nfs::u32 frontend::recordByRank(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f910", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 rank = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordByRank(app, ts, rank); });
}

nfs::u32 frontend::menub::recordsTimeQualifies(App* app, ThreadState& ts, u32 time)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return recordsTimeQualifiesBody(app, ts, guest, time);
}

nfs::u32 frontend::recordsTimeQualifies(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f960", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 time = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsTimeQualifies(app, ts, time); });
}

nfs::u32 frontend::menub::recordsClearMarks(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return recordsClearMarksBody(app, ts, guest);
}

nfs::u32 frontend::recordsClearMarks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f9b0", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsClearMarks(app, ts); });
}

nfs::u32 frontend::menub::recordsInsert(App* app, ThreadState& ts, u32 newRecord)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return recordsInsertBody(app, ts, guest, newRecord);
}

nfs::u32 frontend::recordsInsert(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fa10", kRecordBytes, 0, nullptr, 0, kEcxEdx | kEsiEdi };
    const u32 newRecord = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsInsert(app, ts, newRecord); });
}

nfs::u32 frontend::menub::recordsHolderMark(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return recordsHolderMarkBody(guest);
}

nfs::u32 frontend::recordsHolderMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fb20", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsHolderMark(app, ts); });
}

nfs::u32 frontend::menub::recordsSetHolder(App* app, ThreadState& ts, u32 driver, u32 line0, u32 line1, u32 time,
                                             u32 mark)
{
    Guest guest(app, ts);
    return recordsSetHolderBody(guest, driver, line0, line1, time, mark);
}

nfs::u32 frontend::recordsSetHolder(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fb30", 4, 0, nullptr, 0, kEsiEdi };
    const u32 driver = args.eax, line0 = args.edx, line1 = args.ebx;
    const u32 time = args.ecx, mark = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menub::recordsSetHolder(app, ts, driver, line0, line1, time, mark);
    });
}

nfs::u32 frontend::menub::recordsSheetSortMark(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return recordsSheetSortMarkBody(app, ts, guest, item);
}

nfs::u32 frontend::recordsSheetSortMark(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fc00", 0, 0, nullptr, 0, kEbxEcx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetSortMark(app, ts, item); });
}

nfs::u32 frontend::menub::recordsSheetDraw(App* app, ThreadState& ts, u32 item, u32 hovering)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x580);
    return recordsSheetDrawBody(app, ts, guest, item, hovering);
}

nfs::u32 frontend::recordsSheetDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fc70", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 item = args.eax, hovering = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetDraw(app, ts, item, hovering); });
}

nfs::u32 frontend::menub::recordsSheetLoad(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return recordsSheetLoadBody(app, ts, guest, item);
}

nfs::u32 frontend::recordsSheetLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static constexpr GuestString kNames[] = { kArrowUp, kArrowDown, kArrowUpLit, kArrowDownLit };
    static fe::Site site{ "sub_450770", 0, 0, kNames, std::size(kNames), kEbxEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetLoad(app, ts, item); });
}

nfs::u32 frontend::menub::recordsSheetSave(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return recordsSheetSaveBody(app, ts, guest, item);
}

nfs::u32 frontend::recordsSheetSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450950", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetSave(app, ts, item); });
}

nfs::u32 frontend::menub::recordsSheetLayout(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return recordsSheetLayoutBody(app, ts, guest, item);
}

nfs::u32 frontend::recordsSheetLayout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450970", 0, 0, nullptr, 0, kEbxEcxEdxEsi };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetLayout(app, ts, item); });
}

nfs::u32 frontend::menub::recordsSheetKey(App* app, ThreadState& ts, u32 item, u32, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return recordsSheetKeyBody(app, ts, guest, item, std::uint16_t(key));
}

nfs::u32 frontend::recordsSheetKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450a90", 0, 0, nullptr, 0, kEcxEsiEdi };
    const u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::menub::recordsSheetIdle(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return recordsSheetIdleBody(app, ts, guest, item);
}

nfs::u32 frontend::recordsSheetIdle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450e30", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsSheetIdle(app, ts, item); });
}

nfs::u32 frontend::menub::recordsEnterResult(App* app, ThreadState& ts, u32 found)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x730);
    return recordsEnterResultBody(app, ts, guest, found);
}

nfs::u32 frontend::recordsEnterResult(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450fe0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 found = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsEnterResult(app, ts, found); });
}

nfs::u32 frontend::menub::recordsEnterPlayers(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return recordsEnterPlayersBody(app, ts, guest);
}

nfs::u32 frontend::recordsEnterPlayers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4512f0", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::recordsEnterPlayers(app, ts); });
}

nfs::u32 frontend::menub::textFit(App* app, ThreadState& ts, u32 text, u32 most, u32 font)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return textFitBody(app, ts, guest, text, std::int32_t(most), font);
}

nfs::u32 frontend::textFit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452090", 0, 0, nullptr, 0, kEcxEsiEdi };
    const u32 text = args.eax, most = args.edx, font = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::textFit(app, ts, text, most, font); });
}

nfs::u32 frontend::menub::textDrawFitted(App* app, ThreadState& ts, u32 text, u32 x, u32 y, u32 font, u32 align,
                                           u32 colour, u32 most)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x108);
    return textDrawFittedBody(app, ts, guest, text, x, y, font, align, colour, most);
}

nfs::u32 frontend::textDrawFitted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452120", 12, 0, nullptr, 0, kEsiEdi };
    const u32 text = args.eax, x = args.edx, y = args.ebx, font = args.ecx;
    const u32 align = app->getMemory<u32>(ts.sp + 4), colour = app->getMemory<u32>(ts.sp + 8);
    const u32 most = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menub::textDrawFitted(app, ts, text, x, y, font, align, colour, most);
    });
}

nfs::u32 frontend::menub::menuCaptureScreen(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return callAt(guest, kCaptureScreen);
}

nfs::u32 frontend::menuCaptureScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_452410: the screen's picture taken, the key's work.
    static fe::Site site{ "sub_452410", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::menuCaptureScreen(app, ts); });
}

nfs::u32 frontend::menub::cheatAdvance(App* app, ThreadState& ts, u32 key, u32 progress, u32 code)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return cheatAdvanceBody(guest, key, progress, code);
}

nfs::u32 frontend::cheatAdvance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452420", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 key = args.eax, progress = args.edx, code = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::cheatAdvance(app, ts, key, progress, code); });
}

nfs::u32 frontend::menub::cheatKey(App* app, ThreadState& ts, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x18);
    return cheatKeyBody(app, ts, guest, key);
}

nfs::u32 frontend::cheatKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4524e0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 key = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::cheatKey(app, ts, key); });
}

nfs::u32 frontend::menub::carArtExists(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x100);
    return carArtExistsBody(app, ts, guest, car);
}

nfs::u32 frontend::carArtExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4530c0", 0, 0, nullptr, 0, kEbxEcxEdxEsi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carArtExists(app, ts, car); });
}

nfs::u32 frontend::menub::carViewRemember(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return carViewRememberBody(guest, name);
}

nfs::u32 frontend::carViewRemember(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4534d0", 0, 0, kCompareNames, std::size(kCompareNames), kEcxEdx };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carViewRemember(app, ts, name); });
}

nfs::u32 frontend::menub::carStatsDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x10);
    return carStatsDrawBody(app, ts, guest);
}

nfs::u32 frontend::carStatsDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453940", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carStatsDraw(app, ts); });
}

nfs::u32 frontend::menub::carSpecsDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    return carSpecsDrawBody(app, ts, guest);
}

nfs::u32 frontend::carSpecsDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453f60", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carSpecsDraw(app, ts); });
}

nfs::u32 frontend::menub::carPassesFilter(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return carPassesFilterBody(guest, car);
}

nfs::u32 frontend::carPassesFilter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454730", 0, 0, nullptr, 0, kEcxEdx };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carPassesFilter(app, ts, car); });
}

nfs::u32 frontend::menub::carListDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc);
    return carListDrawBody(app, ts, guest);
}

nfs::u32 frontend::carListDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454780", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carListDraw(app, ts); });
}

nfs::u32 frontend::menub::carCompareDraw(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x2c);
    return carCompareDrawBody(app, ts, guest, item);
}

nfs::u32 frontend::carCompareDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4549c0", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carCompareDraw(app, ts, item); });
}

nfs::u32 frontend::menub::carCompareLoadArt(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x100);
    return carCompareLoadArtBody(app, ts, guest, item);
}

nfs::u32 frontend::carCompareLoadArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454bf0", 0, 0, kBarNames, std::size(kBarNames), kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::carCompareLoadArt(app, ts, item); });
}

nfs::u32 frontend::menub::resultsHiddenRemote(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    return resultsHiddenRemoteBody(guest, car);
}

nfs::u32 frontend::resultsHiddenRemote(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455020", 0, 0, nullptr, 0, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsHiddenRemote(app, ts, car); });
}

void frontend::menub::resultsNetworkNothing(nfs::App* , nfs::ThreadState& )
{
}

nfs::u32 frontend::resultsNetworkNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_455060: nothing: what a network game does more at this point in the
    // results, in the original.
    static fe::Site site{ "sub_455060", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menub::resultsNetworkNothing(app, ts);
        return args.eax;
    });
}

nfs::u32 frontend::menub::tournamentScore(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return tournamentScoreBody(app, ts, guest);
}

nfs::u32 frontend::tournamentScore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455070", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::tournamentScore(app, ts); });
}

nfs::u32 frontend::menub::knockoutEliminate(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return knockoutEliminateBody(app, ts, guest);
}

nfs::u32 frontend::knockoutEliminate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455140", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::knockoutEliminate(app, ts); });
}

nfs::u32 frontend::menub::resultsCollectFinishers(App* app, ThreadState& ts, u32 table, u32 count)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return resultsCollectBody(app, ts, guest, table, count, participant::kFinisher);
}

nfs::u32 frontend::resultsCollectFinishers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455210", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 table = args.eax, count = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsCollectFinishers(app, ts, table, count); });
}

nfs::u32 frontend::menub::resultsCollectLaps(App* app, ThreadState& ts, u32 table, u32 count)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return resultsCollectBody(app, ts, guest, table, count, participant::kLapResult);
}

nfs::u32 frontend::resultsCollectLaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455280", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 table = args.eax, count = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsCollectLaps(app, ts, table, count); });
}

nfs::u32 frontend::menub::resultsAssign(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return resultsAssignBody(app, ts, guest);
}

nfs::u32 frontend::resultsAssign(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4552f0", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsAssign(app, ts); });
}

nfs::u32 frontend::menub::resultsBuild(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return resultsBuildBody(app, ts, guest);
}

nfs::u32 frontend::resultsBuild(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455410", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsBuild(app, ts); });
}

nfs::u32 frontend::menub::resultsName(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    return resultsNameBody(guest, found, buffer);
}

nfs::u32 frontend::resultsName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455570", 0, 0, nullptr, 0, kEsiEdi };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsName(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::resultsPlaceText(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return resultsPlaceTextBody(app, ts, guest, found, buffer);
}

nfs::u32 frontend::resultsPlaceText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4555c0", 0, 0, nullptr, 0, kEsiEdi };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsPlaceText(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::resultsCarName(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return resultsCarNameBody(app, ts, guest, found, buffer);
}

nfs::u32 frontend::resultsCarName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455600", 0, 0, nullptr, 0, kEsiEdi };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsCarName(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::resultsCarLine0(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return resultsCarLineBody(guest, found, buffer, 0);
}

nfs::u32 frontend::resultsCarLine0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455660", 0, 0, nullptr, 0, kEsiEdi };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsCarLine0(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::resultsCarLine1(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return resultsCarLineBody(guest, found, buffer, 1);
}

nfs::u32 frontend::resultsCarLine1(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4556c0", 0, 0, nullptr, 0, kEsiEdi };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsCarLine1(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::resultsTopSpeedText(App* app, ThreadState& ts, u32 found, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return resultsTopSpeedTextBody(app, ts, guest, found, buffer);
}

nfs::u32 frontend::resultsTopSpeedText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455720", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 found = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::resultsTopSpeedText(app, ts, found, buffer); });
}

nfs::u32 frontend::menub::dirPadUnderPointer(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return dirPadUnderPointerBody(guest);
}

nfs::u32 frontend::dirPadUnderPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_457270", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadUnderPointer(app, ts); });
}

nfs::u32 frontend::menub::dirPadCentreUnderPointer(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return dirPadCentreUnderPointerBody(guest);
}

nfs::u32 frontend::dirPadCentreUnderPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4572d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadCentreUnderPointer(app, ts); });
}

nfs::u32 frontend::menub::dirPadChoose(App* app, ThreadState& ts, u32 arrow)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    return dirPadChooseBody(app, ts, guest, arrow);
}

nfs::u32 frontend::dirPadChoose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_457340", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 arrow = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadChoose(app, ts, arrow); });
}

nfs::u32 frontend::menub::dirPadDraw(App* app, ThreadState& ts, u32 item, u32 active)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return dirPadDrawBody(app, ts, guest, item, active);
}

nfs::u32 frontend::dirPadDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_457460", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const u32 item = args.eax, active = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadDraw(app, ts, item, active); });
}

nfs::u32 frontend::menub::dirPadKey(App* app, ThreadState& ts, u32 item, u32, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return dirPadKeyBody(app, ts, guest, item, std::uint16_t(key));
}

nfs::u32 frontend::dirPadKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4575a0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::menub::dirPadAnswer(App*, ThreadState&)
{
    return u32(-3);
}

nfs::u32 frontend::dirPadAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_457620: a widget's slot: what it answers, always.
    static fe::Site site{ "sub_457620", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadAnswer(app, ts); });
}

nfs::u32 frontend::menub::dirPadLayout(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return dirPadLayoutBody(app, ts, guest, item);
}

nfs::u32 frontend::dirPadLayout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_457630", 0, 0, nullptr, 0, kEcxEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadLayout(app, ts, item); });
}

void frontend::menub::dirPadNothing(nfs::App* , nfs::ThreadState& )
{
}

nfs::u32 frontend::dirPadNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_457670: a widget's slot with nothing in it.
    static fe::Site site{ "sub_457670", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        menub::dirPadNothing(app, ts);
        return args.eax;
    });
}

nfs::u32 frontend::menub::dirPadLoadShapes(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 8);
    return dirPadLoadShapesBody(app, ts, guest);
}

nfs::u32 frontend::dirPadLoadShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_457680", 0, 0, nullptr, 0, kEsiEdi | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menub::dirPadLoadShapes(app, ts); });
}

}
