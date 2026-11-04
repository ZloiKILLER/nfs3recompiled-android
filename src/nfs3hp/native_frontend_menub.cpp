#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstring>
#include <iterator>

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
 * Everything each of them calls is called by address, the other functions of
 * this file too. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using x86::reg32;

constexpr std::uint8_t kEbxEcx = fe::kRestoreEbx | fe::kRestoreEcx;
constexpr std::uint8_t kEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = kEbxEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEcxEsiEdi = fe::kRestoreEcx | kEsiEdi;
constexpr std::uint8_t kEbxEcxEsiEdi = kEbxEcx | kEsiEdi;
constexpr std::uint8_t kEbxEcxEdxEsi = kEbxEcxEdx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEdxEdi = kEbxEcxEdx | fe::kRestoreEdi;

// The game's state.
constexpr reg32 kPlayers = 0x6fd3b0;            // 0, 1 side by side, 2 a network
constexpr reg32 kRaceType = 0x6fd3b8;           // 0 a single race, 1 a tournament, 2 a knockout, 3 a duel
constexpr reg32 kTrack = 0x6fd4ac;
constexpr reg32 kLaps = 0x6fd4b4;
constexpr reg32 kInRace = 0x559234;
constexpr reg32 kShownMenu = 0x559230;
constexpr reg32 kPointerX = 0x55e5e4;
constexpr reg32 kPointerY = 0x55e5e8;
constexpr reg32 kPointerPressed = 0x664658;     // the last input came from the mouse: a press
constexpr reg32 kGraphicsFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kHardware3D = 0x02;
constexpr reg32 kTicks = 0x79c1a4;

// The game's functions.
constexpr reg32 kCarLineSpeak = 0x4c9990;       // eax the line, edx the player
constexpr reg32 kSpokenStatus = 0x4c99f0;
constexpr reg32 kFillZero = 0x4e070c;           // eax, edx bytes
constexpr reg32 kMenuShownNow = 0x44a660;       // eax the item, edx its widget
constexpr reg32 kKeysTaken = 0x449df0;          // a key is being read for the controls: everything drawn grey
constexpr reg32 kDrawMenuText = 0x452190;       // eax text, edx x, ebx y, ecx font; align and colour on the stack
constexpr reg32 kSelectFont = 0x452100;
constexpr reg32 kTextWidth = 0x452020;
constexpr reg32 kLanguageText = 0x4d1850;
constexpr reg32 kDrawShape = 0x4d7990;          // eax the shape, edx x, ebx y
constexpr reg32 kDrawShapeSoft = 0x4d7560;
constexpr reg32 kShadedBox = 0x4d8860;          // eax x, edx y, ebx width, ecx height; four colours on the stack
constexpr reg32 kFrame = 0x4d8930;              // eax x, edx y, ebx width, ecx height; a colour on the stack
constexpr reg32 kPointerInBox = 0x49c1f0;       // eax the box: y, x, width, height, words
constexpr reg32 kPointerInRect = 0x49c240;      // eax y, edx x, ebx height, ecx width
constexpr reg32 kSetPointer = 0x49bfc0;
constexpr reg32 kPlayEffect = 0x4181d0;         // eax the effect, edx its volume
constexpr reg32 kShapeByName = 0x4d5db0;
constexpr reg32 kSetRenderState = 0x431900;
constexpr reg32 kSetClip = 0x4bebe0;            // eax left, edx top, ebx right, ecx bottom
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kStricmp = 0x4e9ec0;
constexpr reg32 kStrcmp = 0x4ee310;
constexpr reg32 kToUpper = 0x4eede0;
constexpr reg32 kQsort = 0x4ee4f8;
constexpr reg32 kFileExists = 0x4e0e60;
constexpr reg32 kLoadShapes = 0x44ac90;
constexpr reg32 kFindArtShape = 0x4ef1e0;       // eax the art, edx the shape's name
constexpr reg32 kArtShapeTextures = 0x4d6390;   // eax the shape, edx 0
constexpr reg32 kCarText = 0x439ab0;            // eax the car, edx the line of its text
constexpr reg32 kCarName = 0x439250;
constexpr reg32 kCarFolder = 0x4392b0;
constexpr reg32 kCarFlag4 = 0x4395c0;
constexpr reg32 kCarFileStem = 0x439290;
constexpr reg32 kCarClass = 0x439730;
constexpr reg32 kCarCount = 0x438bd0;
constexpr reg32 kCarStat = 0x438f60;            // eax the car, edx the statistic
constexpr reg32 kLoadCarList = 0x439d90;
constexpr reg32 kMakeCarId = 0x438ad0;          // eax where to, edx the number
constexpr reg32 kFindCarById = 0x439990;
constexpr reg32 kParticipantById = 0x4554e0;    // eax the group, edx the id
constexpr reg32 kResultsSort = 0x455c10;        // eax the group, edx the key, ebx descending
constexpr reg32 kResultsNth = 0x455cd0;         // eax the group, edx the index, ebx a mask, ecx the state it must have
constexpr reg32 kPostgameFinishers = 0x4cef80;
constexpr reg32 kRecordsSelect = 0x44f670;      // eax the track, edx the class of laps
constexpr reg32 kRecordsRead = 0x44fa90;        // eax, edx, ebx the holder's three names, to these buffers
constexpr reg32 kLoadRecords = 0x44f510;
constexpr reg32 kSaveRecords = 0x44f5f0;
constexpr reg32 kLapsClassOf = 0x4cf160;        // eax the laps: which of three tables of records
constexpr reg32 kFormatTime = 0x4cee40;         // eax the time, edx the buffer
constexpr reg32 kFormatSpeed = 0x4cedd0;        // the speed, the buffer: on the stack
constexpr reg32 kUnlockTrack = 0x442b60;        // eax the track
constexpr reg32 kDrawSortMark = 0x46cce0;
constexpr reg32 kSetDirection = 0x4654c0;       // x and y, floats, on the stack

// This file's own.
constexpr reg32 kAwardsClear = 0x44e7d0;
constexpr reg32 kAwardsGrant = 0x44e7f0;        // eax the award, edx the player
constexpr reg32 kAwardsForRace = 0x44e820;
constexpr reg32 kAwardsSpeak = 0x44ea60;
constexpr reg32 kRecordsCompare = 0x44f6e0;
constexpr reg32 kRecordsSortBy = 0x44f830;
constexpr reg32 kRecordsCount = 0x44f8d0;
constexpr reg32 kRecordByRank = 0x44f910;
constexpr reg32 kRecordsTimeQualifies = 0x44f960;
constexpr reg32 kRecordsClearMarks = 0x44f9b0;
constexpr reg32 kRecordsInsert = 0x44fa10;
constexpr reg32 kRecordsHolderMark = 0x44fb20;
constexpr reg32 kRecordsSetHolder = 0x44fb30;
constexpr reg32 kRecordsSheetSortMark = 0x44fc00;
constexpr reg32 kRecordsSheetLayout = 0x450970;
constexpr reg32 kRecordsEnterResult = 0x450fe0;
constexpr reg32 kRecordsEnterPlayers = 0x4512f0;
constexpr reg32 kTextFit = 0x452090;
constexpr reg32 kCheatAdvance = 0x452420;
constexpr reg32 kTextDrawFitted = 0x452120;
constexpr reg32 kCarPassesFilter = 0x454730;
constexpr reg32 kResultsHiddenRemote = 0x455020;
constexpr reg32 kResultsNetworkNothing = 0x455060;
constexpr reg32 kTournamentScore = 0x455070;
constexpr reg32 kKnockoutEliminate = 0x455140;
constexpr reg32 kResultsCollectFinishers = 0x455210;
constexpr reg32 kResultsCollectLaps = 0x455280;
constexpr reg32 kResultsAssign = 0x4552f0;
constexpr reg32 kDirPadUnderPointer = 0x457270;
constexpr reg32 kDirPadCentreUnderPointer = 0x4572d0;
constexpr reg32 kDirPadChoose = 0x457340;

std::int32_t signedWord(const Guest& guest, reg32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t signedDword(const Guest& guest, reg32 address)
{
    return std::int32_t(guest.dword(address));
}

/* What an inline strcpy leaves in eax: the terminator in al, over what was there. */
constexpr reg32 afterInlineCopy(reg32 eax)
{
    return eax & 0xffffff00;
}

void copyDwords(Guest& guest, reg32 to, reg32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* ---------------------------------------------------------------------------
 * The menu loop's pass
 * ------------------------------------------------------------------------- */

constexpr reg32 kSkipPump = 0x559264;           // nonzero: not on this pass
constexpr reg32 kPump = 0x4a75e0;
constexpr reg32 kDrawItems = 0x44a4f0;
constexpr reg32 kDialogShown = 0x4438b0;
constexpr reg32 kDialogPass = 0x445f50;
constexpr reg32 kFlushFonts = 0x4d1340;
constexpr reg32 kDrawPointer = 0x434b60;
constexpr reg32 kPassItems = 0x44a8b0;
constexpr reg32 kCloseDialog = 0x445f10;
constexpr reg32 kPassResult = 0x559254;
constexpr reg32 kCaptureScreen = 0x4c4490;

/* sub_44bdd0: a pass over the shown screen: its items drawn, and what they
 * answered, or a dialog's over them; the fonts flushed, the pointer on top, the
 * items told. */
reg32 menuDrawPassBody(Guest& guest)
{
    if (guest.dword(kSkipPump) == 0)
        guest.call(kPump);
    reg32 result = guest.call(kDrawItems, fe::args({ guest.dword(kShownMenu) }));
    if ((guest.call(kDialogShown) & 0xff) != 0)
        result = guest.call(kDialogPass);
    guest.call(kFlushFonts);
    guest.call(kDrawPointer);
    guest.call(kPassItems, fe::args({ guest.dword(kShownMenu) }));
    guest.setDword(kPassResult, result);
    return result;
}

/* sub_44be20: out of the menus, and any dialog closed. */
reg32 menuLeaveBody(Guest& guest)
{
    guest.setDword(kInRace, 0);
    return guest.call(kCloseDialog);
}

/* ---------------------------------------------------------------------------
 * The race's awards: for each award a flag for each player, spoken as the
 * sheets go by.
 * ------------------------------------------------------------------------- */

constexpr reg32 kAwards = 0x664440;
constexpr std::uint32_t kAwardsBytes = 0x68;

constexpr reg32 kLocalPlayerId = 0x6fd4f4;
constexpr reg32 kResultCount = 0x677d30;        // the race's group
constexpr reg32 kAwardTrack = 0x55d020;         // the track a tournament's awards are for
constexpr reg32 kDuelsWon = 0x6fd4b8;
constexpr reg32 kSheet = 0x55936c;              // which of the postgame's sheets
constexpr reg32 kAwardedCar = 0x6fdc30;
constexpr reg32 kTournamentTrack = 0x6fd408;    // int16

namespace participant
{
constexpr reg32 kDriver = 0x1f0;                // the index in the drivers' table
constexpr reg32 kFlags = 0x200;
constexpr std::uint8_t kFinisher = 0x01;
constexpr std::uint8_t kRemoteSide = 0x04;
constexpr std::uint8_t kInKnockoutTable = 0x08;
constexpr std::uint8_t kLapResult = 0x20;
constexpr std::uint8_t kOut = 0x80;
constexpr reg32 kCar = 0x220;                   // its car's record
constexpr reg32 kTime = 0x238;
constexpr reg32 kLapTimes = 0x240;              // an int per lap
constexpr reg32 kTopSpeed = 0x2b8;              // float
constexpr reg32 kPlace = 0x2bc;
constexpr reg32 kState = 0x2c4;
constexpr reg32 kDuelsBeaten = 0x2cc;
constexpr std::uint32_t kBytes = 0x9ac;
}
constexpr reg32 kCarSlot = 0x10;                // a car's record: the player's slot
constexpr reg32 kCarIdent = 0x14;
constexpr std::uint32_t kStateDisqualified = 2;
constexpr std::uint32_t kStateDidNotFinish = 4;

reg32 awardFlag(reg32 award, reg32 player)
{
    return kAwards + award * 8 + player * 4;
}

/* sub_44e7d0: no one has won anything. */
reg32 awardsClearBody(Guest& guest)
{
    return guest.call(kFillZero, fe::args({ kAwards, kAwardsBytes }));
}

/* sub_44e7f0.  Edx left multiplied, as the code has it. */
reg32 awardsGrantBody(Guest& guest, reg32 award, reg32 player)
{
    const reg32 offset = player * 4;
    guest.setDword(kAwards + offset + award * 8, 1);
    guest.leaveEdx(offset);
    return guest.eax();
}

/* sub_44e820: what a player won in this race, by the kind of race.  Each award
 * is the first place of a sort, but for a duel's and a knockout's. */
reg32 awardsForRaceBody(Guest& guest, reg32 player)
{
    const reg32 id = player == 1 ? player : guest.dword(kLocalPlayerId);
    reg32 found = guest.call(kParticipantById, fe::args({ 0, id }));
    if (found == 0)
    {
        found = guest.call(kParticipantById, fe::args({ 1, id }));
        if (found == 0)
            return 0;
    }
    const auto grant = [&](reg32 award) { return guest.call(kAwardsGrant, fe::args({ award, player })); };
    const auto sorted = [&](reg32 key, reg32 descending) {
        guest.call(kResultsSort, fe::args({ 0, key, descending }));
    };
    const auto nth = [&](reg32 index) { return guest.call(kResultsNth, fe::args({ 0, index, 0, 0 })); };

    const reg32 raceType = guest.dword(kRaceType);
    switch (raceType)
    {
    case 0:
        // A single race of more than one: the winner's.
        if (signedDword(guest, kResultCount) <= 1)
            return raceType;
        sorted(1, 0);
        if (nth(0) != found)
            return guest.eax();
        return grant(0xa);
    case 1:
        // A tournament: on its track, the first three.
        if (guest.dword(kAwardTrack) != guest.dword(kTrack))
            return raceType;
        sorted(4, 1);
        if (nth(0) == found)
            return grant(7);
        if (nth(1) == found)
            return grant(8);
        if (nth(2) == found)
            return grant(9);
        return guest.eax();
    case 2:
    {
        // A knockout, but for the car that was awarded already.
        const reg32 car = guest.dword(found + participant::kCar);
        if (guest.dword(kAwardedCar) == guest.dword(car + kCarIdent))
            return car;
        const reg32 track = guest.dword(kTrack);
        if (track == 8)
        {
            sorted(1, 0);
            return grant(nth(0) == found ? 6 : 3);
        }
        if (guest.dword(found + participant::kState) == kStateDisqualified)
            return grant(3);
        return grant(reg32(signedWord(guest, kTournamentTrack)) == track ? 5 : 4);
    }
    case 3:
        // A duel.
        if ((guest.byte(found + participant::kFlags) & participant::kLapResult) != 0)
        {
            if (signedDword(guest, kDuelsWon) > signedDword(guest, found + participant::kDuelsBeaten))
                return raceType;
            return grant(0xb);
        }
        if (guest.dword(found + participant::kState) == kStateDidNotFinish)
            return raceType;
        sorted(1, 0);
        if (nth(0) != found)
            return guest.eax();
        return grant(0xc);
    default:
        return raceType;
    }
}

/* An award and the line a car says for it. */
struct AwardLine
{
    reg32 award;
    reg32 line;
};

constexpr AwardLine kFirstLines[] = { { 2, 1 }, { 1, 0 }, { 0, 2 } };
constexpr AwardLine kRaceLines[] = { { 0xa, 0xa }, { 0xc, 9 } };
constexpr AwardLine kTournamentLines[] = { { 7, 5 }, { 8, 6 }, { 9, 7 } };
constexpr AwardLine kKnockoutLines[] = { { 3, 0xf }, { 6, 0xe }, { 4, 0xc }, { 5, 0xd } };
constexpr AwardLine kDuelLines[] = { { 0xb, 8 } };

/* sub_44ea60: the next line of this kind that a player has won said, one a pass,
 * and its flag spent.  Awards 1 and 2 with award 0 are one line. */
reg32 awardsSpeakBody(Guest& guest, reg32 kind)
{
    const reg32 players = guest.dword(kPlayers) == 1 ? 2 : 1;
    const reg32 status = guest.call(kSpokenStatus);
    if (status == 0)
        return status;
    const auto sayLines = [&](reg32 player, const AwardLine* lines, std::size_t count, reg32& said) {
        for (std::size_t i = 0; i < count; ++i)
        {
            if (guest.dword(awardFlag(lines[i].award, player)) != 0)
            {
                said = guest.call(kCarLineSpeak, fe::args({ lines[i].line, player }));
                guest.setDword(awardFlag(lines[i].award, player), 0);
                return true;
            }
        }
        return false;
    };
    for (reg32 player = 0; player < players; ++player)
    {
        reg32 said = 0;
        switch (kind)
        {
        case 0:
            for (const AwardLine both : { AwardLine{ 1, 3 }, AwardLine{ 2, 4 } })
            {
                if (guest.dword(awardFlag(both.award, player)) != 0 && guest.dword(awardFlag(0, player)) != 0)
                {
                    guest.call(kCarLineSpeak, fe::args({ both.line, player }));
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
reg32 awardsSpeakForSheetBody(Guest& guest)
{
    const reg32 raceType = guest.dword(kRaceType);
    switch (guest.dword(kSheet))
    {
    case 1:
        guest.call(kAwardsSpeak, fe::args({ 0 }));
        break;
    case 2:
        guest.call(kAwardsSpeak, fe::args({ reg32(raceType == 2 ? 3 : 1) }));
        break;
    case 3:
        if (raceType == 1)
            guest.call(kAwardsSpeak, fe::args({ 2 }));
        else if (raceType == 3)
            guest.call(kAwardsSpeak, fe::args({ 4 }));
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

constexpr reg32 kRecordsFile = 0x559408;
constexpr reg32 kRecordsList = 0x55940c;        // the chosen list: its count, then its records
constexpr reg32 kRecordHolder = 0x559410;
constexpr reg32 kRecordsSortKey = 0x559414;
constexpr reg32 kRecordsRanks = 0x6645d0;       // int16[10]: which record is at each rank

constexpr std::uint32_t kRecordBytes = 0x31c;
constexpr std::uint32_t kMostRecords = 10;
constexpr std::uint32_t kRecordsByLapsClass = 0x15634;
constexpr std::uint32_t kRecordsByTrack = 0x1f1c;
constexpr std::uint32_t kRecordsLists = 0x16e0; // the lists in the records' file

namespace record
{
constexpr reg32 kRank = 0x00;
constexpr reg32 kDriver = 0x04;                 // char[11]
constexpr reg32 kCarName = 0x0f;
constexpr reg32 kCarLine0 = 0x10f;
constexpr reg32 kCarLine1 = 0x20f;
constexpr reg32 kTime = 0x310;                  // int
constexpr reg32 kSpeed = 0x314;                 // float
constexpr reg32 kAids = 0x318;                  // int16: whether any aid was on
constexpr reg32 kNew = 0x31a;                   // int16; bit 0, this race's
}

// The holder of a list's best lap: the time, whether it is this race's, the names.
constexpr reg32 kHolderIsNew = 0x04;
constexpr reg32 kHolderDriver = 0x08;
constexpr reg32 kHolderCarLine0 = 0x13;
constexpr reg32 kHolderCarLine1 = 0x113;

constexpr std::uint32_t kSortByTime = 4;

/* What fp::compare's status word says, as sahf hands it to a jump. */
constexpr std::uint16_t kStatusBelow = 0x0100;  // c0: below, or unordered
constexpr std::uint16_t kStatusEqual = 0x4000;  // c3: equal, or unordered

/* sub_44f6e0, qsort's: two records' places (eax and edx their slots) by the key
 * at 0x559414, and by the time when that key finds them the same. */
reg32 recordsCompareBody(Guest& guest, reg32 slotA, reg32 slotB)
{
    const reg32 a = guest.dword(slotA);
    const reg32 b = guest.dword(slotB);
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
        order = std::int32_t(guest.call(kStricmp, fe::args({ a + record::kDriver, b + record::kDriver })));
        break;
    case 3:
        order = std::int32_t(guest.call(kStricmp, fe::args({ a + record::kCarName, b + record::kCarName })));
        break;
    case 5:
    {
        // The fastest first; unordered, the same.
        const std::uint16_t status = fp::compare(x86::Float(guest.floatAt(a + record::kSpeed)),
                                                 x86::Float(guest.floatAt(b + record::kSpeed)));
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
    guest.leaveEdx(reg32(order));
    return reg32(order);
}

/* sub_44f830: the ranks by a key.  The list's records in a frame's array,
 * sorted by qsort, and each one's number found again. */
reg32 recordsSortByBody(Guest& guest, reg32 key)
{
    const reg32 places = guest.frame();
    const reg32 list = guest.dword(kRecordsList);
    for (reg32 i = 0; i < kMostRecords; ++i)
    {
        guest.setWord(kRecordsRanks + 2 * i, 0xffff);
        guest.setDword(places + 4 * i, list + 4 + i * kRecordBytes);
    }
    guest.setDword(kRecordsSortKey, key);
    guest.call(kQsort, fe::args({ places, guest.dword(list), 4, kRecordsCompare }));
    std::int32_t rank = 0;
    for (; rank < signedDword(guest, guest.dword(kRecordsList)); ++rank)
    {
        const reg32 place = guest.dword(places + 4 * rank);
        const reg32 first = guest.dword(kRecordsList);
        for (std::int32_t number = 0; number < signedDword(guest, first); ++number)
        {
            if (first + 4 + reg32(number) * kRecordBytes == place)
                guest.setWord(kRecordsRanks + 2 * rank, std::uint16_t(number));
        }
    }
    return reg32(rank);
}

/* sub_44f8d0: how many records a track's list has, for a class of laps of three. */
reg32 recordsCountBody(Guest& guest, reg32 track, reg32 lapsClass)
{
    const reg32 ofClass = reg32(std::int32_t(lapsClass) % 3) * kRecordsByLapsClass + guest.dword(kRecordsFile);
    guest.leaveEdx(ofClass);
    return guest.dword(ofClass + track * kRecordsByTrack + kRecordsLists);
}

/* sub_44f910: the record at a rank, if there is one. */
reg32 recordByRankBody(Guest& guest, reg32 rank)
{
    if (guest.dword(kRecordsFile) == 0 || std::int32_t(rank) > std::int32_t(kMostRecords))
        return 0;
    const std::int32_t number = signedWord(guest, kRecordsRanks + rank * 2);
    if (number < 0)
        return 0;
    const reg32 list = guest.dword(kRecordsList);
    if (number > signedDword(guest, list))
        return 0;
    return list + 4 + reg32(number) * kRecordBytes;
}

/* sub_44f960: whether a time gets into the list: there is room, or it beats the
 * last. */
reg32 recordsTimeQualifiesBody(Guest& guest, reg32 time)
{
    if (signedDword(guest, guest.dword(kRecordsList)) < std::int32_t(kMostRecords))
        return 1;
    guest.call(kRecordsSortBy, fe::args({ kSortByTime }));
    const reg32 last = guest.call(kRecordByRank, fe::args({ guest.dword(guest.dword(kRecordsList)) - 1 }));
    if (last == 0 || std::int32_t(time) < signedDword(guest, last + record::kTime))
        return 1;
    return 0;
}

/* sub_44f9b0: no record of any track or laps is this race's any more. */
reg32 recordsClearMarksBody(Guest& guest)
{
    reg32 found = 0;
    for (reg32 lapsClass = 0; lapsClass < 3; ++lapsClass)
    {
        for (reg32 track = 0; track < 11; ++track)
        {
            guest.call(kRecordsSelect, fe::args({ track, lapsClass }));
            guest.setDword(guest.dword(kRecordHolder) + kHolderIsNew, 0);
            guest.call(kRecordsSortBy, fe::args({ kSortByTime, 0 }));
            for (reg32 rank = 0; rank < kMostRecords; ++rank)
            {
                found = guest.call(kRecordByRank, fe::args({ rank }));
                if (found != 0)
                    guest.setWord(found + record::kNew, 0);
            }
        }
    }
    return found;
}

/* sub_44fa10: a record, by value on the stack, into the list: at its end, or
 * over the slowest when it is full; then each one's rank written into it. */
reg32 recordsInsertBody(Guest& guest, reg32 newRecord)
{
    guest.call(kRecordsSortBy, fe::args({ kSortByTime }));
    const reg32 list = guest.dword(kRecordsList);
    const reg32 count = guest.dword(list);
    if (count == kMostRecords)
    {
        const reg32 slowest = guest.call(kRecordByRank, fe::args({ kMostRecords - 1 }));
        copyDwords(guest, slowest, newRecord, kRecordBytes / 4);
    }
    else
    {
        copyDwords(guest, list + 4 + count * kRecordBytes, newRecord, kRecordBytes / 4);
        guest.setDword(list, guest.dword(list) + 1);
    }
    guest.call(kRecordsSortBy, fe::args({ kSortByTime }));
    reg32 found = 0;
    for (reg32 rank = 0; rank < kMostRecords; ++rank)
    {
        found = guest.call(kRecordByRank, fe::args({ rank }));
        if (found == 0)
            break;
        guest.setDword(found + record::kRank, rank);
    }
    return found;
}

/* sub_44fb20: whether the holder of the best lap is this race's. */
reg32 recordsHolderMarkBody(Guest& guest)
{
    return guest.dword(guest.dword(kRecordHolder) + kHolderIsNew);
}

/* sub_44fb30: a new holder of the best lap.  Ecx left with the mark. */
reg32 recordsSetHolderBody(Guest& guest, reg32 driver, reg32 line0, reg32 line1, reg32 time, reg32 mark)
{
    const reg32 holder = guest.dword(kRecordHolder);
    guest.setDword(holder, time);
    guest.setDword(holder + kHolderIsNew, mark);
    guest.leaveEcx(mark);
    guest.copyString(holder + kHolderDriver, driver);
    guest.copyString(guest.dword(kRecordHolder) + kHolderCarLine0, line0);
    guest.copyString(guest.dword(kRecordHolder) + kHolderCarLine1, line1);
    return afterInlineCopy(guest.dword(kRecordHolder));
}

/* ---------------------------------------------------------------------------
 * The RECORDS sheet: an item (widget 0x18), its columns and rows from the .MNU.
 * ------------------------------------------------------------------------- */

constexpr reg32 kRecordsWidget = 0x18;
constexpr reg32 kSheetTrack = 0x559418;
constexpr reg32 kSheetLapsClass = 0x55941c;
constexpr reg32 kRecordsAnimated = 0x662968;    // after a race: this race's records stand out

namespace sheet
{
constexpr reg32 kX = 0x06;
constexpr reg32 kY = 0x08;
constexpr reg32 kHeaderBox = 0x1a;              // y, x, width, height
constexpr reg32 kBoxHeight = 0x20;
constexpr reg32 kColumnWidths = 0x3c;           // int16[6]
constexpr reg32 kRowHeight = 0x48;              // int16
constexpr reg32 kRows = 0x4a;                   // int16
constexpr reg32 kColumnBoxes = 0x4c;            // y, x, width, height, for each column
constexpr std::uint32_t kColumns = 6;
constexpr reg32 kUpBox = 0x7c;
constexpr reg32 kDownBox = 0x84;
constexpr reg32 kTitles = 0x8c;                 // char*[6]
constexpr reg32 kShownTrack = 0xa4;             // int16
constexpr reg32 kShownLapsClass = 0xa6;         // int16
constexpr reg32 kUpShape = 0xa8;                // int16
constexpr reg32 kDownShape = 0xaa;              // int16
constexpr reg32 kUpLitShape = 0xac;             // int16
constexpr reg32 kDownLitShape = 0xae;           // int16
constexpr reg32 kSortMarkShape = 0xb0;          // int16
constexpr reg32 kSortUpShape = 0xb2;            // int16
constexpr reg32 kFirstRow = 0xb4;               // int16, the scroll
constexpr reg32 kSortKey = 0xb6;                // int16, a column's, 1 to 6
constexpr reg32 kSlide = 0xb8;                  // int16: 0 at rest, 1 coming in, 2 going out, 3 out
constexpr reg32 kSlideX = 0xba;                 // int16
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
reg32 recordsSheetSortMarkBody(Guest& guest, reg32 item)
{
    const std::uint16_t chosen = std::uint16_t(guest.word(item + sheet::kSortKey) - 1);
    const reg32 column = chosen <= 5 ? chosen : 0;
    const reg32 box = item + sheet::kColumnBoxes + column * 8;
    const reg32 place = guest.dword(box);
    const reg32 size = guest.dword(box + 4);
    return guest.call(kDrawSortMark, fe::args({ column, size, place },
                                               { place, size, reg32(signedWord(guest, item + sheet::kSortMarkShape)) }));
}

/* sub_44fc70: the sheet, on every pass: sliding in or out, its lines, its
 * headers with the pointer's lit, a row for each record shown (place, driver,
 * car, time, speed, aids), the holder of the best lap under them, the scrolling
 * arrows.  Edx left as the code leaves it. */
reg32 recordsSheetDrawBody(Guest& guest, reg32 item, reg32 hovering)
{
    if (guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget })) == 0)
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

    const reg32 grey = guest.call(kKeysTaken);
    const auto ifNotGrey = [&](std::uint32_t colour) { return grey != 0 ? kGreyedOut : colour; };

    // Another track or number of laps: from the top.
    if (reg32(signedWord(guest, item + sheet::kShownTrack)) != guest.dword(kSheetTrack)
        || reg32(signedWord(guest, item + sheet::kShownLapsClass)) != guest.dword(kSheetLapsClass))
    {
        guest.setWord(item + sheet::kFirstRow, 0);
    }
    guest.setWord(item + sheet::kShownTrack, guest.word(kSheetTrack));
    guest.setWord(item + sheet::kShownLapsClass, guest.word(kSheetLapsClass));

    guest.call(kRecordsSheetLayout, fe::args({ item }));
    guest.call(kRecordsSelect, fe::args({ guest.dword(kSheetTrack), guest.dword(kSheetLapsClass) }));
    guest.call(kRecordsSortBy, fe::args({ reg32(signedWord(guest, item + sheet::kSortKey)) }));

    std::int32_t width = 0;
    for (reg32 column = 0; column < sheet::kColumns; ++column)
        width += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
    const std::int32_t height = signedWord(guest, item + sheet::kRowHeight) * (signedWord(guest, item + sheet::kRows) + 1);
    const std::int32_t records = std::int32_t(
        guest.call(kRecordsCount, fe::args({ guest.dword(kSheetTrack), guest.dword(kSheetLapsClass) })));
    const bool scrolls = records > 1;

    const auto slideX = [&]() { return signedWord(guest, item + sheet::kSlideX); };
    const auto topY = [&]() { return signedWord(guest, item + sheet::kY); };
    const auto rowHeight = [&]() { return signedWord(guest, item + sheet::kRowHeight); };
    const auto rows = [&]() { return signedWord(guest, item + sheet::kRows); };
    const auto box = [&](std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t c0,
                         std::uint32_t c1, std::uint32_t c2, std::uint32_t c3) {
        guest.call(kShadedBox, fe::args({ reg32(x), reg32(y), reg32(w), reg32(h) }, { c0, c1, c2, c3 }));
    };

    // The frame.
    box(slideX() - 2, topY() - 2, width + 4, 2, kLineDark, kLineLight, kLineLight, kLineDark);
    box(slideX() - 2, topY() + height + 2, width + 4, 2, kLineLight, kLineLight, kLineLight, kLineLight);
    box(slideX() - 2, topY() - 2, 2, height + 4, kLineDark, kLineDark, kLineLight, kLineLight);
    box(slideX() + width, topY() - 2, 2, height + 4, kLineLight, kLineLight, kLineLight, kLineLight);
    box(slideX(), topY() + rowHeight() - 1, width, 2, kLineDark, kLineLight, kLineLight, kLineDark);
    if (rows() < records)
    {
        guest.call(kFrame, fe::args({ reg32(signedWord(guest, item + sheet::kUpBox + 2) - 3),
                                      reg32(signedWord(guest, item + sheet::kUpBox) - 1), 0x12, reg32(height + 4) },
                                    { kLineLight }));
    }
    // Between the rows and the columns.
    std::int32_t lineY = topY() + rowHeight();
    for (std::int32_t row = 0; row < rows(); ++row)
    {
        box(slideX(), lineY, width, 1, kLineDark, kLineLight, kLineLight, kLineDark);
        lineY += rowHeight();
    }
    std::int32_t lineX = slideX() + signedWord(guest, item + sheet::kColumnWidths);
    for (reg32 column = 1; column < sheet::kColumns; ++column)
    {
        box(lineX, topY(), 1, height + 2, kLineDark, kLineDark, kLineLight, kLineLight);
        lineX += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
    }
    if (signedWord(guest, item + sheet::kSlide) == 0 && scrolls)
        guest.call(kRecordsSheetSortMark, fe::args({ item, reg32(height) }));

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
    for (reg32 column = 0; column < sheet::kColumns; ++column)
    {
        const reg32 columnBox = item + sheet::kColumnBoxes + column * 8;
        const std::int32_t x = textX[column] + signedWord(guest, columnBox + 2);
        const std::int32_t y = signedWord(guest, columnBox + 6) / 2 + signedWord(guest, columnBox) - 5;
        const reg32 title = guest.dword(item + column * 4 + sheet::kTitles);
        const bool lit = scrolls && hovering != 0 && guest.call(kPointerInBox, fe::args({ columnBox })) != 0;
        if (lit)
        {
            guest.call(kFrame, fe::args({ reg32(signedWord(guest, columnBox + 2) + 1), reg32(signedWord(guest, columnBox)),
                                          reg32(signedWord(guest, columnBox + 4) - 1),
                                          reg32(signedWord(guest, columnBox + 6) - 1) },
                                        { kHeaderFrame }));
        }
        guest.call(kDrawMenuText, fe::args({ title, reg32(x), reg32(y), kMenuFont },
                                           { reg32(kAlign[column]), ifNotGrey(lit ? kYellow : kLineLight) }));
    }

    // The rows.
    const reg32 buffer = guest.frame() + 0x200;
    const auto columnsTo = [&](reg32 columns) {
        std::int32_t x = slideX();
        for (reg32 column = 0; column < columns; ++column)
            x += signedWord(guest, item + sheet::kColumnWidths + 2 * column);
        return x;
    };
    std::int32_t rowY = topY() + rowHeight() + rowHeight() / 2 - 5;
    std::int32_t rank = signedWord(guest, item + sheet::kFirstRow);
    for (std::int32_t shown = 0; rows() > shown && rank < std::int32_t(kMostRecords); ++shown)
    {
        const reg32 found = guest.call(kRecordByRank, fe::args({ reg32(rank) }));
        if (found == 0)
            break;
        // The place.
        const reg32 place = guest.call(kLanguageText, fe::args({ guest.dword(found + record::kRank) + 0x30 }));
        guest.call(kDrawMenuText, fe::args({ place, reg32(slideX() + textX[0]), reg32(rowY), kMenuFont },
                                           { reg32(kAlign[0]), ifNotGrey(kRowColour) }));
        // The driver, this race's lit.
        const std::int32_t driverX = columnsTo(1) + textX[1];
        const bool isNew = guest.dword(kRecordsAnimated) != 0 && guest.word(found + record::kNew) != 0;
        const reg32 driverWidth = reg32(signedWord(guest, item + sheet::kColumnWidths + 2) - 3);
        guest.call(kTextDrawFitted, fe::args({ found + record::kDriver, reg32(driverX), reg32(rowY), kMenuFont },
                                             { reg32(kAlign[1]), ifNotGrey(isNew ? kYellow : kRowColour), driverWidth }));
        // The car, on two lines.
        const std::int32_t carX = columnsTo(2) + textX[2];
        const reg32 carWidth = reg32(signedWord(guest, item + sheet::kColumnWidths + 4) - 3);
        guest.copyString(buffer, found + record::kCarLine0);
        guest.call(kTextDrawFitted, fe::args({ buffer, reg32(carX), reg32(rowY - 10), kMenuFont },
                                             { reg32(kAlign[2]), ifNotGrey(kRowColour), carWidth }));
        guest.copyString(buffer, found + record::kCarLine1);
        guest.call(kTextDrawFitted, fe::args({ buffer, reg32(carX), reg32(rowY + 5), kMenuFont },
                                             { reg32(kAlign[2]), ifNotGrey(kRowColour), carWidth }));
        // The time.
        guest.call(kFormatTime, fe::args({ guest.dword(found + record::kTime), buffer }));
        guest.call(kDrawMenuText, fe::args({ buffer, reg32(columnsTo(3) + textX[3]), reg32(rowY), kMenuFont },
                                           { reg32(kAlign[3]), ifNotGrey(kRowColour) }));
        // The speed.
        guest.leaveEdx(item);
        guest.call(kFormatSpeed, fe::args({}, { guest.dword(found + record::kSpeed), buffer }));
        guest.call(kDrawMenuText, fe::args({ buffer, reg32(columnsTo(4) + textX[4]), reg32(rowY), kMenuFont },
                                           { reg32(kAlign[4]), ifNotGrey(kRowColour) }));
        // The aids.
        const reg32 aids = guest.call(kLanguageText, fe::args({ reg32(guest.word(found + record::kAids) != 0 ? 0xc4 : 0xc5) }));
        guest.call(kDrawMenuText, fe::args({ aids, reg32(columnsTo(5) + textX[5] + kAlign[5]), reg32(rowY), kMenuFont },
                                           { reg32(kAlign[5]), ifNotGrey(kRowColour) }));
        rowY += rowHeight();
        ++rank;
    }

    // The holder of the best lap.
    const reg32 holderLine = guest.frame();
    const reg32 lapDriver = guest.frame() + 0x300;
    const reg32 lapLine0 = guest.frame() + 0x100;
    const reg32 lapLine1 = guest.frame() + 0x400;
    const reg32 lap = guest.call(kRecordsRead, fe::args({ lapDriver, lapLine0, lapLine1 }));
    if (lap != 0)
    {
        guest.call(kFormatTime, fe::args({ lap, buffer }));
        const reg32 title = guest.call(kLanguageText, fe::args({ 0xea }));
        constexpr reg32 kHolderFormat = 0x53958c;   // "%s:  %s   %s   %s"
        guest.call(kSprintf, fe::args({}, { holderLine, kHolderFormat, title, lapDriver, lapLine1, buffer }));
        const std::int32_t x = width + slideX();
        const std::int32_t y = topY() + height + 0x18;
        const bool blink = guest.dword(kRecordsAnimated) != 0 && (guest.call(kRecordsHolderMark) & 1) != 0
            && signedDword(guest, kTicks) % 0x80 < 0x40;
        guest.call(kDrawMenuText, fe::args({ holderLine, reg32(x), reg32(y), kMenuFont },
                                           { 4, ifNotGrey(blink ? kYellow : kLineLight) }));
    }

    // The arrows.
    if (rows() >= records)
        return 0;
    if (guest.word(item + sheet::kFirstRow) != 0)
    {
        const reg32 x = reg32(signedWord(guest, item + sheet::kUpBox + 2));
        const reg32 y = reg32(signedWord(guest, item + sheet::kUpBox) + 1);
        const bool lit = guest.call(kPointerInBox, fe::args({ item + sheet::kUpBox })) != 0;
        const reg32 shape = reg32(signedWord(guest, item + (lit ? sheet::kUpLitShape : sheet::kUpShape)));
        guest.call(kDrawShape, fe::args({ shape, x, y }));
    }
    const std::int32_t firstRow = signedWord(guest, item + sheet::kFirstRow);
    if (rows() + firstRow >= records)
    {
        guest.leaveEdx(reg32(firstRow));
        return 0;
    }
    const reg32 x = reg32(signedWord(guest, item + sheet::kDownBox + 2));
    const reg32 y = reg32(signedWord(guest, item + sheet::kDownBox));
    const bool lit = guest.call(kPointerInBox, fe::args({ item + sheet::kDownBox })) != 0;
    const reg32 shape = reg32(signedWord(guest, item + (lit ? sheet::kDownLitShape : sheet::kDownShape)));
    guest.call(kDrawShape, fe::args({ shape, x, y }));
    return 0;
}

/* sub_450770: the sheet as its screen opens: the records loaded, the track and
 * laps raced chosen, its titles and arrows, its slide; and after a race,
 * scrolled to this race's first record. */
constexpr GuestString kArrowUp{ 0x5395a0, "bup1" };
constexpr GuestString kArrowDown{ 0x5395a8, "bdn1" };
constexpr GuestString kArrowUpLit{ 0x5395b0, "yup1" };
constexpr GuestString kArrowDownLit{ 0x5395b8, "ydn1" };

reg32 recordsSheetLoadBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget })) == 0)
        return 0;
    guest.call(kLoadRecords);
    guest.setDword(kSheetTrack, guest.dword(kTrack));
    const reg32 lapsClass = reg32(std::int16_t(guest.call(kLapsClassOf, fe::args({ reg32(signedWord(guest, kLaps)) }))));
    guest.setDword(kSheetLapsClass, lapsClass);
    guest.call(kRecordsSelect, fe::args({ guest.dword(kSheetTrack), lapsClass }));
    guest.setWord(item + sheet::kShownTrack, guest.word(kSheetTrack));
    guest.setWord(item + sheet::kShownLapsClass, guest.word(kSheetLapsClass));
    if (signedWord(guest, item + sheet::kRows) >= 10)
        guest.setWord(item + sheet::kRows, 10);
    static constexpr std::uint32_t kTitleTexts[sheet::kColumns] = { 0xe3, 0xc3, 2, 0x28, 0x29, 0x192 };
    for (reg32 column = 0; column < sheet::kColumns; ++column)
        guest.setDword(item + sheet::kTitles + column * 4, guest.call(kLanguageText, fe::args({ kTitleTexts[column] })));
    const auto shape = [&](const GuestString& name) { return guest.call(kShapeByName, fe::args({ name.address })); };
    guest.setWord(item + sheet::kUpShape, std::uint16_t(shape(kArrowUp)));
    guest.setWord(item + sheet::kDownShape, std::uint16_t(shape(kArrowDown)));
    guest.setWord(item + sheet::kUpLitShape, std::uint16_t(shape(kArrowUpLit)));
    guest.setWord(item + sheet::kDownLitShape, std::uint16_t(shape(kArrowDownLit)));
    guest.setWord(item + sheet::kSortMarkShape, std::uint16_t(shape(kArrowDown)));
    reg32 left = shape(kArrowUp);
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
        guest.call(kRecordsCount, fe::args({ guest.dword(kSheetTrack), guest.dword(kSheetLapsClass) })));
    left = guest.call(kRecordsSortBy, fe::args({ reg32(signedWord(guest, item + sheet::kSortKey)) }));
    for (std::int32_t rank = 0; rank < records; ++rank)
    {
        const reg32 found = guest.call(kRecordByRank, fe::args({ reg32(rank) }));
        if (found == 0)
            return 0;
        left = found;
        if ((guest.byte(found + record::kNew) & 1) != 0)
        {
            left = reg32(signedWord(guest, item + sheet::kRows));
            if (rank >= std::int32_t(left))
            {
                const std::uint16_t first = std::uint16_t(rank - guest.word(item + sheet::kRows));
                const reg32 result = ((reg32(rank) & 0xffff0000) | first) + 1;
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(result));
                return result;
            }
        }
    }
    return left;
}

/* sub_450950: its records saved as the screen closes. */
reg32 recordsSheetSaveBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget })) != 0)
        guest.call(kSaveRecords);
    return guest.eax();
}

/* sub_450970: the columns' boxes, and the arrows', from the column widths and
 * where the sheet has slid to.  Eax left as the code's sums leave it. */
reg32 recordsSheetLayoutBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget })) == 0)
        return 0;
    std::int32_t x = signedWord(guest, item + sheet::kSlideX);
    reg32 total = 0;
    for (reg32 column = 0; column < sheet::kColumns; ++column)
    {
        const reg32 box = item + sheet::kColumnBoxes + column * 8;
        guest.setWord(box + 2, std::uint16_t(x));
        guest.setWord(box, guest.word(item + sheet::kY));
        guest.setWord(box + 4, guest.word(item + sheet::kColumnWidths + column * 2));
        const std::int32_t width = signedWord(guest, box + 4);
        guest.setWord(box + 6, guest.word(item + sheet::kRowHeight));
        x += width;
        total += reg32(width);
    }
    const reg32 height = reg32((signedWord(guest, item + sheet::kRows) + 1) * signedWord(guest, item + sheet::kRowHeight));

    const reg32 header = item + sheet::kHeaderBox;
    guest.setWord(header + 2, guest.word(item + sheet::kSlideX));
    guest.setWord(header, guest.word(item + sheet::kY));
    guest.setWord(header + 4, std::uint16_t(total + 0x16));

    const auto withLowWord = [](reg32 value, std::uint16_t low) { return (value & 0xffff0000) | low; };
    const reg32 upX = withLowWord(total + 0x16, guest.word(item + sheet::kSlideX)) + total + 0xa;
    guest.setWord(item + sheet::kUpBox + 4, 0xc);
    guest.setWord(item + sheet::kUpBox + 2, std::uint16_t(upX));
    guest.setWord(item + sheet::kUpBox, guest.word(item + sheet::kY));
    guest.setWord(item + sheet::kUpBox + 6, 0xc);
    const reg32 downX = total + withLowWord(upX, guest.word(item + sheet::kSlideX)) + 0xa;
    guest.setWord(item + sheet::kDownBox + 4, 0xc);
    guest.setWord(item + sheet::kDownBox + 2, std::uint16_t(downX));
    guest.setWord(item + sheet::kDownBox + 6, 0xc);
    guest.setWord(item + sheet::kBoxHeight, std::uint16_t(height));
    const reg32 downY = withLowWord(downX, guest.word(item + sheet::kY)) + height - 0xc;
    guest.setWord(item + sheet::kDownBox, std::uint16_t(downY));
    return downY;
}

/* The pointer moved into a box: three quarters across, half way down. */
void pointerIntoBox(Guest& guest, reg32 box)
{
    const std::int32_t x = signedWord(guest, box + 2) + signedWord(guest, box + 4) * 3 / 4;
    const std::int32_t y = signedWord(guest, box) + signedWord(guest, box + 6) / 2;
    guest.setDword(kPointerX, reg32(x));
    guest.setDword(kPointerY, reg32(y));
    guest.call(kSetPointer, fe::args({ reg32(x), reg32(y) }));
}

/* sub_450a90: the sheet's keys: Enter on a column sorts by it, the arrows go
 * across the columns, Page Up and Page Down and clicks on the arrows scroll.
 * What it answers stays in edx, ebx what the code leaves there. */
reg32 recordsSheetKeyBody(Guest& guest, reg32 item, std::uint16_t key)
{
    constexpr std::uint16_t kEnter = 0xd, kRight = 0x4d00, kLeft = 0x4b00, kPageUp = 0x4900, kPageDown = 0x5100;
    constexpr std::uint32_t kClick = 1, kVolume = 0x7f;

    std::int32_t column = -1;
    guest.leaveEbx(reg32(column));
    if (guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget })) == 0)
    {
        guest.leaveEdx(0);
        return 0;
    }
    const std::int32_t records = std::int32_t(
        guest.call(kRecordsCount, fe::args({ guest.dword(kSheetTrack), guest.dword(kSheetLapsClass) })));
    if (records < 2)
    {
        guest.leaveEdx(0);
        return 0;
    }
    for (reg32 i = 0; i < sheet::kColumns; ++i)
    {
        if (guest.call(kPointerInBox, fe::args({ item + sheet::kColumnBoxes + i * 8 })) != 0)
        {
            column = std::int32_t(i);
            guest.leaveEbx(reg32(column));
            break;
        }
    }
    const auto clamped = [](std::int32_t value) { return value < 0 ? 0 : value > 5 ? 5 : value; };
    const auto canScrollDown = [&]() {
        return signedWord(guest, item + sheet::kRows) + signedWord(guest, item + sheet::kFirstRow) < records;
    };

    reg32 answer = 0;
    if (key == kEnter)
    {
        if (column >= 0)
            guest.setWord(item + sheet::kSortKey, std::uint16_t(column + 1));
        answer = 2;
    }
    if (key == kRight || key == kLeft)
    {
        column = clamped(column + (key == kRight ? 1 : -1));
        guest.leaveEbx(reg32(column));
        pointerIntoBox(guest, item + sheet::kColumnBoxes + reg32(column) * 8);
        answer = 2;
    }
    if (signedWord(guest, item + sheet::kRows) < records)
    {
        if (key == kPageUp && guest.word(item + sheet::kFirstRow) != 0)
        {
            guest.call(kPlayEffect, fe::args({ kClick, kVolume }));
            const std::uint16_t first = guest.word(item + sheet::kFirstRow);
            guest.setWord(item + sheet::kFirstRow, std::uint16_t(first - 1));
            guest.leaveEbx(((reg32(column) & 0xffff0000) | first) - 1);
            pointerIntoBox(guest, item + sheet::kUpBox);
            answer = 2;
        }
        if (key == kPageDown)
        {
            guest.leaveEbx(reg32(records));
            if (canScrollDown())
            {
                guest.call(kPlayEffect, fe::args({ kClick, kVolume }));
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) + 1));
                pointerIntoBox(guest, item + sheet::kDownBox);
                answer = 2;
            }
        }
        if (key == kEnter)
        {
            if (guest.call(kPointerInBox, fe::args({ item + sheet::kUpBox })) != 0
                && guest.word(item + sheet::kFirstRow) != 0)
            {
                guest.call(kPlayEffect, fe::args({ kClick, kVolume }));
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) - 1));
                answer = 2;
            }
            if (guest.call(kPointerInBox, fe::args({ item + sheet::kDownBox })) != 0 && canScrollDown())
            {
                guest.call(kPlayEffect, fe::args({ kClick, kVolume }));
                guest.setWord(item + sheet::kFirstRow, std::uint16_t(guest.word(item + sheet::kFirstRow) + 1));
                answer = 2;
            }
        }
    }
    guest.leaveEdx(answer);
    return answer;
}

/* sub_450e30: a widget's slot with nothing to do. */
reg32 recordsSheetIdleBody(Guest& guest, reg32 item)
{
    guest.call(kMenuShownNow, fe::args({ item, kRecordsWidget }));
    return 0;
}

constexpr reg32 kDrivers = 0x6fd52c;
constexpr std::uint32_t kDriverBytes = 0x6c;
constexpr reg32 kDriverCar = 0x14;
constexpr reg32 kDriverName = 0x34;

/* The aids a race had on, which a record keeps together. */
constexpr reg32 kAidsOn[] = { 0x6fd490, 0x6fd494, 0x6fd498, 0x6fd49c, 0x6fd4a0, 0x6fd4a4 };

constexpr std::uint32_t kEnteredBestLap = 1;
constexpr std::uint32_t kEnteredBestTime = 2;
constexpr std::uint32_t kEnteredTime = 4;

/* sub_450fe0: a participant's race into the chosen list and its best lap, for
 * the awards: which it got.  The record built in the frame, and handed by value. */
reg32 recordsEnterResultBody(Guest& guest, x86::CPU& cpu, reg32 found)
{
    const reg32 driver = kDrivers + guest.dword(found + participant::kDriver) * kDriverBytes;
    const reg32 car = guest.dword(driver + kDriverCar);
    const reg32 newRecord = guest.frame();
    const reg32 ignored = guest.frame() + 0x51c;
    std::uint32_t entered = 0;

    const reg32 holderLap = guest.call(kRecordsRead, fe::args({ ignored, ignored, ignored }));
    const reg32 ownCar = guest.dword(guest.dword(found + participant::kCar) + kCarIdent);
    if (ownCar == guest.dword(kAwardedCar))
        return 0;
    if ((guest.byte(found + participant::kFlags) & participant::kFinisher) == 0)
        return 0;

    if (guest.call(kRecordsTimeQualifies, fe::args({ guest.dword(found + participant::kTime) })) != 0)
    {
        guest.copyString(newRecord + record::kDriver, driver + kDriverName);
        guest.copyString(newRecord + record::kCarName, guest.call(kCarName, fe::args({ car })));
        guest.copyString(newRecord + record::kCarLine0, guest.call(kCarText, fe::args({ car, 0 })));
        guest.copyString(newRecord + record::kCarLine1, guest.call(kCarText, fe::args({ car, 1 })));
        guest.setDword(newRecord + record::kTime, guest.dword(found + participant::kTime));
        guest.setDword(newRecord + record::kSpeed, guest.dword(found + participant::kTopSpeed));
        std::uint16_t aids = 0;
        for (const reg32 aid : kAidsOn)
            aids = std::uint16_t(aids | guest.word(aid));
        guest.setWord(newRecord + record::kAids, aids);
        guest.setWord(newRecord + record::kNew, 1);

        guest.call(kRecordsSortBy, fe::args({ kSortByTime }));
        const reg32 best = guest.call(kRecordByRank, fe::args({ 0 }));
        if (best != 0
            && signedDword(guest, best + record::kTime) > signedDword(guest, found + participant::kTime))
            entered = kEnteredBestTime;
        else
            entered |= kEnteredTime;

        cpu.esp -= kRecordBytes;
        copyDwords(guest, cpu.esp, newRecord, kRecordBytes / 4);
        guest.call(kRecordsInsert);
        cpu.esp += kRecordBytes;
    }

    // Its best lap.
    std::int32_t bestLap = 0;
    for (std::int32_t lap = 0; lap < signedDword(guest, kLaps); ++lap)
    {
        const std::int32_t time = signedDword(guest, found + participant::kLapTimes + 4 * lap);
        if (time != 0 && (time < bestLap || bestLap == 0))
            bestLap = time;
    }
    if (bestLap != 0 && (std::int32_t(holderLap) > bestLap || holderLap == 0))
    {
        const reg32 holderDriver = guest.frame() + 0x31c;
        const reg32 holderLine0 = guest.frame() + 0x61c;
        const reg32 holderLine1 = guest.frame() + 0x41c;
        guest.copyString(holderDriver, driver + kDriverName);
        guest.copyString(holderLine0, guest.call(kCarText, fe::args({ car, 0 })));
        guest.copyString(holderLine1, guest.call(kCarText, fe::args({ car, 1 })));
        guest.call(kRecordsSetHolder, fe::args({ holderDriver, holderLine0, holderLine1, reg32(bestLap) }, { 1 }));
        entered |= kEnteredBestLap;
    }
    return entered;
}

/* sub_4512f0: the player's race, and a second player's beside it, entered; and
 * what they got turned into awards. */
reg32 recordsEnterPlayersBody(Guest& guest)
{
    const reg32 lapsClass = reg32(std::int16_t(guest.call(kLapsClassOf, fe::args({ reg32(signedWord(guest, kLaps)) }))));
    guest.call(kRecordsSelect, fe::args({ guest.dword(kTrack), lapsClass }));
    const auto enter = [&](reg32 id, reg32 player) {
        const reg32 found = guest.call(kParticipantById, fe::args({ 0, id }));
        if (found == 0 || guest.dword(found + participant::kState) == kStateDidNotFinish)
            return;
        const reg32 entered = guest.call(kRecordsEnterResult, fe::args({ found }));
        if (entered & kEnteredBestLap)
            guest.call(kAwardsGrant, fe::args({ 0, player }));
        if (entered & kEnteredTime)
            guest.call(kAwardsGrant, fe::args({ 2, player }));
        if (entered & kEnteredBestTime)
            guest.call(kAwardsGrant, fe::args({ 1, player }));
    };
    enter(guest.dword(kLocalPlayerId), 0);
    if (guest.dword(kPlayers) == 1)
        enter(1, 1);
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * Text in a column
 * ------------------------------------------------------------------------- */

/* sub_452090: a text cut to a width in a font, its end made "..." when it was
 * cut.  The text is changed where it is. */
reg32 textFitBody(Guest& guest, reg32 text, std::int32_t most, reg32 font)
{
    std::uint32_t length = guest.length(text);
    guest.call(kSelectFont, fe::args({ font, font }));
    reg32 left = guest.call(kTextWidth, fe::args({ text, font }));
    if (std::int32_t(left) <= most)
        return left;
    while (length != 0)
    {
        left = guest.call(kTextWidth, fe::args({ text, font }));
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
reg32 textDrawFittedBody(Guest& guest, reg32 text, reg32 x, reg32 y, reg32 font, reg32 align, reg32 colour, reg32 most)
{
    const reg32 copy = guest.frame();
    guest.copyString(copy, text);
    guest.call(kTextFit, fe::args({ copy, most, font }));
    return guest.call(kDrawMenuText, fe::args({ copy, x, y, font }, { align, colour }));
}

/* ---------------------------------------------------------------------------
 * The codes typed on any screen
 * ------------------------------------------------------------------------- */

constexpr reg32 kCodes = 0x5594e4;              // char*[21], each letter 9 past its own
constexpr reg32 kCodeProgress = 0x6645f0;       // int[21]
constexpr std::uint32_t kCodeCount = 21;
constexpr std::int32_t kCodeLetterShift = 9;
constexpr reg32 kJumpState = 0x559538;          // "GO" and two digits: a track at once
constexpr reg32 kJumpTrack = 0x55953c;
constexpr std::int32_t kLastTrackJump = 0x15;
constexpr reg32 kUnlockedCars = 0x6fd2a8;       // byte
constexpr reg32 kUnlockedAll = 0x6fd2ac;
constexpr reg32 kChosenCheats = 0x6fd4fc;       // byte
constexpr reg32 kChosenTrackJump = 0x6fd500;
constexpr reg32 kPlayerCar = 0x6fd2bc;
constexpr std::uint32_t kCodeEffect = 5, kCodeVolume = 0x7f;

/* sub_452420: a key against a code: its place in the code moved on, or back to
 * its start.  1 when the code is done.  Ebx left at the next letter. */
reg32 cheatAdvanceBody(Guest& guest, reg32 key, reg32 progress, reg32 code)
{
    const std::int32_t letter = std::int32_t(guest.call(kToUpper, fe::args({ key }))) + kCodeLetterShift;
    if (letter != std::int32_t(guest.byte(code + guest.dword(progress))))
    {
        guest.setDword(progress, 0);
        return 0;
    }
    const reg32 next = guest.dword(progress) + 1;
    guest.setDword(progress, next);
    guest.leaveEbx(code + next);
    return guest.byte(code + next) == 0 ? 1 : 0;
}

/* sub_4524e0: a key typed on any screen, in a game of one or two at a single
 * race or a duel: "GO" and a number, a track; and the codes, each its own
 * unlock.  Those of a car have it chosen, in the kinds of race it is for. */
reg32 cheatKeyBody(Guest& guest, reg32 key)
{
    if (key == 0)
        return key;
    const reg32 players = guest.dword(kPlayers);
    if (players != 0 && players != 1)
        return key;
    const reg32 raceType = guest.dword(kRaceType);
    if (raceType != 3 && raceType != 0)
        return key;

    const std::int32_t letter = std::int32_t(guest.call(kToUpper, fe::args({ key })));
    const auto effect = [&]() { guest.call(kPlayEffect, fe::args({ kCodeEffect, kCodeVolume })); };
    const auto isDigit = [&]() { return letter >= '0' && letter <= '9'; };
    const reg32 state = guest.dword(kJumpState);
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
            guest.setDword(kJumpTrack, reg32((letter - '0') * 10));
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
            guest.setDword(kJumpTrack, reg32(track));
            if (track >= 0 && track <= kLastTrackJump)
            {
                effect();
                guest.setByte(kChosenCheats, guest.byte(kChosenCheats) | 2);
                guest.setDword(kChosenTrackJump, guest.dword(kJumpTrack));
            }
        }
        guest.setDword(kJumpState, 0);
        break;
    default:
        break;
    }

    reg32 carGroup = 0;
    const auto unlockCar = [&](std::uint8_t bit, reg32 group) {
        if ((guest.byte(kUnlockedCars) & bit) != 0)
            return;
        effect();
        guest.setByte(kUnlockedCars, guest.byte(kUnlockedCars) | bit);
        carGroup = group;
    };
    const auto choose = [&](std::uint8_t bit) {
        effect();
        guest.setByte(kChosenCheats, guest.byte(kChosenCheats) | bit);
    };
    for (reg32 i = 0; i < kCodeCount; ++i)
    {
        if (guest.call(kCheatAdvance, fe::args({ reg32(letter), kCodeProgress + 4 * i, guest.dword(kCodes + 4 * i) })) == 0)
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
            guest.setByte(kUnlockedCars, guest.byte(kUnlockedCars) | 0x1f);
            carGroup = 1;
            break;
        case 6:
            effect();
            guest.setDword(kUnlockedAll, 1);
            guest.setDword(kTrack, 8);
            guest.call(kUnlockTrack, fe::args({ 8 }));
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
        return guest.eax();

    guest.call(kLoadCarList);
    const bool duel = guest.dword(kRaceType) == 3;
    reg32 carNumber = 0;
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
    const reg32 id = guest.frame();
    guest.call(kMakeCarId, fe::args({ id, carNumber }));
    const reg32 car = guest.call(kFindCarById, fe::args({ id }));
    if (taken)
        guest.setDword(kPlayerCar, car);
    return car;
}

/* ---------------------------------------------------------------------------
 * The showcase's car sheets
 * ------------------------------------------------------------------------- */

constexpr reg32 kRacesClosed = 0x6fbb54;
constexpr reg32 kArtFolder = 0x7a2db4;
constexpr reg32 kShowcaseCar = 0x664798;
constexpr reg32 kStatsPage = 0x55957c;
constexpr reg32 kSpecsSelected = 0x55958c;
constexpr reg32 kCarViewMode = 0x559578;
constexpr std::int32_t kStatTitleX = 0xb4;
constexpr std::int32_t kStatValueX = 0x1d6;
constexpr std::int32_t kRowStep = 0xf;

/* sub_4530c0: whether a car has its art: its file exists, in the showcase's
 * folder or the cars'. */
reg32 carArtExistsBody(Guest& guest, reg32 car)
{
    constexpr reg32 kOwnFolderFormat = 0x539684;    // "%s%s\\SH%s.qfs"
    constexpr reg32 kCarsFormat = 0x539694;         // "%sSH%s.qfs"
    constexpr reg32 kShowcaseFolder = 0x7a28b4;
    constexpr reg32 kCarsFolder = 0x7a3234;
    const reg32 folder = guest.call(kCarFolder, fe::args({ car }));
    if (guest.dword(kRacesClosed) != 0 || folder == 0)
        return 0;
    const reg32 path = guest.frame();
    if (guest.call(kCarFlag4, fe::args({ car })) != 0)
    {
        const reg32 stem = guest.call(kCarFileStem, fe::args({ car }));
        guest.call(kSprintf, fe::args({}, { path, kOwnFolderFormat, kShowcaseFolder, folder, stem }));
    }
    else
    {
        guest.call(kSprintf, fe::args({}, { path, kCarsFormat, kCarsFolder, folder }));
    }
    return guest.call(kFileExists, fe::args({ path })) != 0 ? 1 : 0;
}

/* sub_4534d0: the screen left remembered for the car screen's way back, but for
 * the comparison's two. */
constexpr GuestString kCompareNames[] = { { 0x539728, "compare" }, { 0x539730, "compare2" } };

reg32 carViewRememberBody(Guest& guest, reg32 name)
{
    const reg32 first = guest.call(kStrcmp, fe::args({ name, kCompareNames[0].address }));
    if (first == 0)
        return 0;
    const reg32 second = guest.call(kStrcmp, fe::args({ name, kCompareNames[1].address }));
    if (second == 0)
        return 0;
    guest.setDword(kCarViewMode, name);
    return second;
}

/* A statistic's row: its title from the language's, its value from the car's text. */
void statRow(Guest& guest, bool grey, std::int32_t y, reg32 title, reg32 line)
{
    const std::uint32_t colour = grey ? kGreyedOut : kWhite;
    const reg32 titleText = guest.call(kLanguageText, fe::args({ title }));
    guest.call(kDrawMenuText, fe::args({ titleText, reg32(kStatTitleX), reg32(y), kMenuFont }, { 0, colour }));
    const reg32 value = guest.call(kCarText, fe::args({ guest.dword(kShowcaseCar), line }));
    guest.call(kDrawMenuText, fe::args({ value, reg32(kStatValueX), reg32(y), kMenuFont }, { 1, colour }));
}

/* sub_453940: a page of the car's statistics. */
reg32 carStatsDrawBody(Guest& guest)
{
    const bool grey = guest.call(kKeysTaken) != 0;
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        guest.call(kSetRenderState, fe::args({ 6, 1 }));
    std::int32_t y = 0x9a;
    const auto rows = [&](reg32 count, reg32 firstTitle, reg32 firstLine) {
        for (reg32 i = 0; i < count; ++i)
        {
            statRow(guest, grey, y, firstTitle + i, firstLine + i);
            y += kRowStep;
        }
    };
    const reg32 page = guest.dword(kStatsPage);
    switch (page)
    {
    case 0:
        rows(7, 0x160, 3);
        break;
    case 1:
    {
        rows(3, 0x168, 0x11);
        const reg32 header = guest.call(kLanguageText, fe::args({ 0x16b }));
        guest.call(kDrawMenuText, fe::args({ header, 0xa2, reg32(y), kMenuFont }, { 0, grey ? kGreyedOut : kYellow }));
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
    return guest.eax();
}

/* sub_453f60: the car's specifications, the one selected lit. */
reg32 carSpecsDrawBody(Guest& guest)
{
    const bool grey = guest.call(kKeysTaken) != 0;
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        guest.call(kSetRenderState, fe::args({ 6, 1 }));
    std::int32_t y = 0x87;
    for (reg32 i = 0; i < 7; ++i)
    {
        const std::uint32_t colour = grey ? kGreyedOut : i == guest.dword(kSpecsSelected) ? kYellow : kWhite;
        const reg32 value = guest.call(kCarText, fe::args({ guest.dword(kShowcaseCar), 0x17 + i }));
        guest.call(kDrawMenuText, fe::args({ value, reg32(kStatValueX), reg32(y), kMenuFont }, { 1, colour }));
        y += kRowStep;
    }
    return guest.eax();
}

constexpr reg32 kCarFilter = 0x559570;          // 0, 1 all; 2, 3, 4 a class
constexpr reg32 kCarListScroll = 0x5595d4;
constexpr reg32 kExtraCars = 0x554f50;
constexpr reg32 kCarListStat = 0x559574;
constexpr reg32 kCarStatIds = 0x559598;         // int[4]
constexpr reg32 kBarLong = 0x5595b4;
constexpr reg32 kBarMiddle = 0x5595b8;
constexpr reg32 kBarShort = 0x5595bc;
constexpr reg32 kBarLeast = 0x5595c0;
constexpr reg32 kCompareTitles = 0x5595c4;      // int[4]: the language's
constexpr reg32 kCompareLeftEmblem = 0x5595ac;
constexpr reg32 kCompareRightEmblem = 0x5595b0;
constexpr reg32 kCompareLeftCar = 0x664788;
constexpr reg32 kCompareRightCar = 0x664784;
constexpr std::int32_t kStatMost = 0x14;
constexpr std::int32_t kListRows = 0x12;
constexpr std::int32_t kClipBottom = 0x1e0;
constexpr std::int32_t kClipRight = 0x280;
constexpr reg32 kArrowLeftOff = 0x66479c;       // int16s: the list's arrows
constexpr reg32 kArrowLeftOn = 0x6647a0;
constexpr reg32 kArrowRightOff = 0x6647a2;
constexpr reg32 kArrowRightOn = 0x66479e;

/* sub_454730: whether a car (in eax, for sub_439730) is in the class shown. */
reg32 carPassesFilterBody(Guest& guest)
{
    switch (guest.dword(kCarFilter))
    {
    case 0:
    case 1:
        return 1;
    case 2:
        return guest.call(kCarClass) == 0 ? 1 : 0;
    case 3:
        return guest.call(kCarClass) == 1 ? 1 : 0;
    case 4:
        return guest.call(kCarClass) == 2 ? 1 : 0;
    default:
        return 0;
    }
}

/* sub_454780: the list of the class's cars, each with a bar of the statistic
 * chosen, scrolled by two arrows pressed with the pointer. */
reg32 carListDrawBody(Guest& guest)
{
    const std::uint32_t colour = guest.call(kKeysTaken) != 0 ? kGreyedOut : kYellow;
    const std::int32_t cars = std::int32_t(guest.call(kCarCount) - guest.dword(kExtraCars));
    std::int32_t inClass = 0;
    for (std::int32_t car = 0; car < cars; ++car)
    {
        if (guest.call(kCarPassesFilter, fe::args({ reg32(car) })) != 0)
            ++inClass;
    }
    const auto arrow = [&](reg32 off, reg32 on, reg32 y, std::int32_t step) {
        guest.call(kDrawShape, fe::args({ reg32(signedWord(guest, off)), 0xb9, y }));
        if (guest.call(kPointerInRect, fe::args({ 0xb7, y - 2, 0xf, 0xf })) == 0)
            return;
        guest.call(kDrawShape, fe::args({ reg32(signedWord(guest, on)), 0xb9, y }));
        if (guest.dword(kPointerPressed) != 0)
            guest.setDword(kCarListScroll, guest.dword(kCarListScroll) + reg32(step));
    };
    if (signedDword(guest, kCarListScroll) + inClass < kListRows)
        guest.setDword(kCarListScroll, 0);
    else if (signedDword(guest, kCarListScroll) > 0)
        arrow(kArrowLeftOff, kArrowLeftOn, 0x9d, -1);
    reg32 left = guest.dword(kCarListScroll) + kListRows;
    if (std::int32_t(left) < inClass)
    {
        arrow(kArrowRightOff, kArrowRightOn, 0x19c, 1);
        left = guest.eax();
    }

    std::int32_t shown = 0;
    std::int32_t y = 0x9b;
    for (std::int32_t car = signedDword(guest, kCarListScroll); car < cars; ++car)
    {
        left = guest.call(kCarPassesFilter, fe::args({ reg32(car) }));
        if (left == 0 || shown >= kListRows)
            continue;
        ++shown;
        const reg32 name = guest.call(kCarText, fe::args({ reg32(car), 2 }));
        guest.call(kDrawMenuText, fe::args({ name, 0xc8, reg32(y), kMenuFont }, { 0, colour }));
        const reg32 statistic = guest.dword(kCarStatIds + guest.dword(kCarListStat) * 4);
        std::int32_t stat = std::int32_t(guest.call(kCarStat, fe::args({ reg32(car), statistic })));
        if (stat > kStatMost)
            stat = kStatMost;
        std::int32_t right, x;
        if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
        {
            right = std::int32_t(reg32(stat) * 0x9a) / kStatMost + 0x18c;
            x = 0x159;
        }
        else
        {
            right = std::int32_t(reg32(stat) * 0xbd) / kStatMost + 0x184;
            x = 0x145;
        }
        guest.call(kSetClip, fe::args({ 0, 0, reg32(right), reg32(kClipBottom) }));
        guest.call(kDrawShape, fe::args({ guest.dword(kBarMiddle), reg32(x), reg32(y) }));
        left = guest.call(kSetClip, fe::args({ 0, 0, reg32(kClipRight), reg32(kClipBottom) }));
        y += kRowStep;
    }
    return left;
}

/* sub_4549c0: two cars' statistics compared: four rows of two bars, growing
 * from the middle, the better one's bright. */
reg32 carCompareDrawBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, 0x1c })) == 0)
        return 0;
    std::int32_t left[4], right[4];
    for (reg32 i = 0; i < 4; ++i)
    {
        const auto stat = [&](reg32 car) {
            const std::int32_t value = std::int32_t(
                guest.call(kCarStat, fe::args({ guest.dword(car), guest.dword(kCarStatIds + 4 * i) })));
            return value > kStatMost ? kStatMost : value;
        };
        left[i] = stat(kCompareLeftCar);
        right[i] = stat(kCompareRightCar);
    }
    const auto barLength = [](std::int32_t stat) { return std::int32_t(reg32(stat) * 100) / kStatMost; };
    for (reg32 i = 0; i < 4; ++i)
    {
        const bool hardware = (guest.byte(kGraphicsFlags) & kHardware3D) != 0;
        const std::int32_t rowY = std::int32_t(0x127 + i * 30);
        const reg32 title = guest.call(kLanguageText, fe::args({ guest.dword(kCompareTitles + 4 * i) }));
        guest.call(kDrawMenuText, fe::args({ title, 0x17a, reg32(rowY - (hardware ? 3 : 0)), kMenuFont }, { 2, kYellow }));
        reg32 leftBar = guest.dword(kBarMiddle);
        reg32 rightBar = guest.dword(kBarLeast);
        if (right[i] > left[i])
            leftBar = guest.dword(kBarLong);
        else if (right[i] < left[i])
            rightBar = guest.dword(kBarShort);

        const std::int32_t leftEdge = 0x147 - barLength(left[i]);
        guest.call(kSetClip, fe::args({ reg32(leftEdge), 0, reg32(kClipRight - leftEdge), reg32(kClipBottom) }));
        const reg32 barY = reg32((guest.byte(kGraphicsFlags) & kHardware3D) != 0 ? rowY + 0xe : rowY);
        guest.call(kDrawShape, fe::args({ leftBar, 0xd6, barY }));
        guest.call(kSetClip, fe::args({ 0, 0, reg32(barLength(right[i]) + 0x1af), reg32(kClipBottom) }));
        if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
            guest.call(kDrawShape, fe::args({ rightBar, 0x181, reg32(rowY + 0xe) }));
        else
            guest.call(kDrawShapeSoft, fe::args({ rightBar, 0x154, reg32(rowY) }));
        guest.call(kSetClip, fe::args({ 0, 0, reg32(kClipRight), reg32(kClipBottom) }));
    }
    const reg32 leftEmblem = guest.dword(kCompareLeftEmblem);
    if (leftEmblem != 0)
        guest.call(kDrawShape, fe::args({ leftEmblem, 0x7a, 0x4e }));
    const reg32 rightEmblem = guest.dword(kCompareRightEmblem);
    if (rightEmblem != 0)
        guest.call(kDrawShape, fe::args({ rightEmblem, 0x163, 0x4e }));
    return 0;
}

/* sub_454bf0: the bars' art loaded as the comparison opens: four shapes on 3D
 * hardware, two without. */
constexpr GuestString kBarNames[] = {
    { 0x5398a4, "lfrd" }, { 0x5398ac, "lfgr" }, { 0x5398b4, "gtrd" },
    { 0x5398bc, "gtgr" }, { 0x5398c4, "redb" }, { 0x5398cc, "grnb" },
};

reg32 carCompareLoadArtBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, 0x1c })) == 0)
        return 0;
    constexpr reg32 kArtFormat = 0x539864;          // "%scompare.fsh"
    const reg32 path = guest.frame();
    guest.call(kSprintf, fe::args({}, { path, kArtFormat, kArtFolder }));
    const reg32 art = guest.call(kLoadShapes, fe::args({ path }));
    const auto bar = [&](const GuestString& name) {
        const reg32 shape = guest.call(kFindArtShape, fe::args({ art, name.address }));
        return guest.call(kArtShapeTextures, fe::args({ shape, 0 }));
    };
    if ((guest.byte(kGraphicsFlags) & kHardware3D) != 0)
    {
        guest.setDword(kBarLong, bar(kBarNames[0]));
        guest.setDword(kBarMiddle, bar(kBarNames[1]));
        guest.setDword(kBarShort, bar(kBarNames[2]));
        const reg32 least = bar(kBarNames[3]);
        guest.setDword(kBarLeast, least);
        return least;
    }
    guest.setDword(kBarLong, bar(kBarNames[4]));
    const reg32 green = bar(kBarNames[5]);
    guest.setDword(kBarMiddle, green);
    guest.setDword(kBarLeast, green);
    guest.setDword(kBarShort, guest.dword(kBarLong));
    return green;
}

/* ---------------------------------------------------------------------------
 * The race's results as the menus keep them
 * ------------------------------------------------------------------------- */

constexpr reg32 kRaceCars = 0x5efac8;           // the race's cars' records
constexpr reg32 kRaceCarCount = 0x5efd9c;
constexpr reg32 kNetworkDropped = 0x7a22ca;     // int16 for each driver
constexpr reg32 kResultsOfRace = 0x6647b0;
constexpr reg32 kResultsOfLaps = 0x66e270;
constexpr reg32 kLapResultCount = 0x677d34;
constexpr reg32 kReplaying = 0x6fd3a0;
constexpr reg32 kResultsBuildStart = 0x4c7ea0;
constexpr reg32 kTournamentPointsTable = 0x454f10;  // int16 for each place, in the code's own data
constexpr reg32 kTournamentRaceScores = 0x6fd42e;   // int16 for each player
constexpr reg32 kTournamentTotals = 0x6fd44e;       // int16 for each player
constexpr reg32 kTournamentPlaces = 0x6fd40e;       // int16 for each player
constexpr reg32 kTournamentRaces = 0x6fd40c;        // int16
constexpr reg32 kKnockoutIn = 0x6fd3de;             // int16 for each player
constexpr reg32 kKnockoutPlaces = 0x6fd3be;         // int16 for each player
constexpr reg32 kKnockoutRounds = 0x6fd3bc;         // int16
constexpr std::uint32_t kPlayerWords = 0x20;

/* sub_455020: a network player's car that has dropped out: left out of the results. */
reg32 resultsHiddenRemoteBody(Guest& guest, reg32 car)
{
    if (car != 0 && (guest.byte(car + participant::kFlags) & participant::kRemoteSide) != 0
        && signedDword(guest, kPlayers) > 1
        && guest.word(kNetworkDropped + 2 * guest.dword(car + participant::kDriver)) != 0)
        return 1;
    return 0;
}

/* sub_455070: a tournament's race scored: a place's points for each player and
 * the total, then the order of the totals.  Not once the finishers have gone. */
reg32 tournamentScoreBody(Guest& guest)
{
    reg32 left = guest.call(kPostgameFinishers);
    if (left != 0)
        return left;
    guest.call(kResultsSort, fe::args({ 0, 1, 0 }));
    const auto slotOf = [&](reg32 found) {
        return guest.dword(guest.dword(found + participant::kCar) + kCarSlot);
    };
    for (std::int32_t place = 0; place < signedDword(guest, kResultCount); ++place)
    {
        const reg32 found = guest.call(kResultsNth, fe::args({ 0, reg32(place), 0, 0 }));
        if (found == 0)
            continue;
        const std::uint16_t points = guest.word(kTournamentPointsTable + 2 * place);
        guest.setWord(kTournamentRaceScores + slotOf(found) * 2, points);
        const reg32 total = kTournamentTotals + slotOf(found) * 2;
        guest.setWord(total, std::uint16_t(guest.word(total) + points));
    }
    guest.call(kFillZero, fe::args({ kTournamentPlaces, kPlayerWords }));
    guest.call(kResultsSort, fe::args({ 0, 1, 1 }));
    std::uint16_t place = 0;
    for (reg32 i = 0; i <= 8; ++i)
    {
        left = guest.call(kResultsNth, fe::args({ 0, i, 0, 0 }));
        if (left != 0)
        {
            left = slotOf(left);
            guest.setWord(kTournamentPlaces + left * 2, place++);
        }
    }
    guest.setWord(kTournamentRaces, std::uint16_t(guest.word(kTournamentRaces) + 1));
    return left;
}

/* sub_455140: a knockout's round: the last of those still in is out, and the
 * order of those left written down. */
reg32 knockoutEliminateBody(Guest& guest)
{
    guest.call(kResultsSort, fe::args({ 0, 1, 1, 2 }));
    const reg32 last = guest.call(kResultsNth, fe::args({ 0, 0, 0x20, 2 }));
    const auto slotOf = [&](reg32 found) {
        return guest.dword(guest.dword(found + participant::kCar) + kCarSlot);
    };
    if (last != 0 && signedDword(guest, kResultCount) > 1)
    {
        guest.setWord(kKnockoutIn + slotOf(last) * 2, 0);
        guest.setDword(last + participant::kState, kStateDisqualified);
    }
    guest.call(kFillZero, fe::args({ kKnockoutPlaces, kPlayerWords }));
    guest.call(kResultsSort, fe::args({ 0, 1, 1 }));
    reg32 left = 0;
    std::uint16_t place = 0;
    for (reg32 i = 0; i <= 8; ++i)
    {
        const reg32 found = guest.call(kResultsNth, fe::args({ 0, i, 0, 0 }));
        left = found;
        if (found == 0)
            continue;
        const bool inTable = (guest.byte(found + participant::kFlags) & participant::kInKnockoutTable) != 0;
        left = slotOf(found);
        if (inTable && guest.word(kKnockoutIn + left * 2) == 0)
            continue;
        guest.setWord(kKnockoutPlaces + left * 2, place++);
    }
    guest.setWord(kKnockoutRounds, std::uint16_t(guest.word(kKnockoutRounds) + 1));
    return left;
}

/* sub_455210, sub_455280: the race's cars with a flag copied whole into a table
 * of the results, but for those that have dropped out. */
reg32 resultsCollectBody(Guest& guest, reg32 table, reg32 count, std::uint8_t flag)
{
    guest.setDword(count, 0);
    reg32 left = table;
    for (std::int32_t i = 0; i < signedDword(guest, kRaceCarCount); ++i)
    {
        const reg32 car = guest.dword(kRaceCars + 4 * i);
        left = car;
        if ((guest.byte(car + participant::kFlags) & flag) == 0)
            continue;
        left = guest.call(kResultsHiddenRemote, fe::args({ car }));
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
reg32 resultsAssignBody(Guest& guest)
{
    constexpr std::uint32_t kPlaceFlagLapped = 0x10;
    if (guest.dword(kRaceType) == 2)
    {
        guest.call(kResultsSort, fe::args({ 0, 1, 1 }));
        for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
        {
            const reg32 found = guest.call(kResultsNth, fe::args({ 0, reg32(i), 0, 0 }));
            if (found != 0 && (guest.byte(found + participant::kFlags) & participant::kOut) != 0)
                guest.setDword(found + participant::kState, kStateDisqualified);
        }
    }
    guest.call(kResultsSort, fe::args({ 0, 1, 1, 2 }));
    const reg32 lastIn = guest.call(kResultsNth, fe::args({ 0, 0, 0x20, 2 }));
    if (lastIn != 0)
    {
        reg32 time = guest.dword(lastIn + participant::kTime) + 0x3c;
        for (const reg32 state : { 2u, 4u })
        {
            for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
            {
                const reg32 found = guest.call(kResultsNth, fe::args({ 0, reg32(i), kPlaceFlagLapped, state }));
                if (found != 0)
                {
                    ++time;
                    guest.setDword(found + participant::kTime, time);
                }
            }
        }
    }
    guest.call(kResultsSort, fe::args({ 0, 1, 0 }));
    for (std::int32_t i = 0; i < signedDword(guest, kResultCount); ++i)
    {
        const reg32 found = guest.call(kResultsNth, fe::args({ 0, reg32(i), 0, 0 }));
        if (found != 0)
            guest.setDword(found + participant::kPlace, reg32(i + 1));
    }
    return guest.eax();
}

/* sub_455410: the race's results made, as the race ends: the awards cleared,
 * the results copied and sorted, a tournament's and a knockout's scored, the
 * records entered and saved. */
reg32 resultsBuildBody(Guest& guest)
{
    if (guest.dword(kReplaying) != 0)
        return guest.eax();
    guest.call(kResultsBuildStart);
    guest.call(kAwardsClear);
    guest.call(kLoadRecords);
    guest.call(kResultsCollectFinishers, fe::args({ kResultsOfRace, kResultCount }));
    guest.call(kResultsCollectLaps, fe::args({ kResultsOfLaps, kLapResultCount, 0 }));
    guest.call(kResultsSort, fe::args({ 0, 0, 0 }));
    guest.call(kResultsSort, fe::args({ 1, 0, 0 }));
    guest.call(kResultsAssign);
    if (guest.dword(kRaceType) == 1)
        guest.call(kTournamentScore);
    if (guest.dword(kRaceType) == 2)
        guest.call(kKnockoutEliminate);
    guest.call(kRecordsClearMarks);
    if (guest.call(kPostgameFinishers) == 0)
    {
        guest.call(kRecordsEnterPlayers);
        guest.call(kAwardsForRace, fe::args({ 0 }));
        if (guest.dword(kPlayers) == 1)
            guest.call(kAwardsForRace, fe::args({ 1 }));
    }
    guest.call(kSaveRecords);
    if (signedDword(guest, kPlayers) > 1 && guest.call(kPostgameFinishers) == 0)
        guest.call(kResultsNetworkNothing);
    return guest.eax();
}

/* The participant's driver, with what the code leaves in edx on the way. */
reg32 driverRecord(Guest& guest, reg32 found)
{
    const reg32 driver = guest.dword(found + participant::kDriver);
    guest.leaveEdx(driver * 12);
    return kDrivers + driver * kDriverBytes;
}

/* sub_455570: a participant's name into a buffer; empty for none. */
reg32 resultsNameBody(Guest& guest, reg32 found, reg32 buffer)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const reg32 driver = driverRecord(guest, found);
    guest.copyString(buffer, driver + kDriverName);
    return afterInlineCopy(driver);
}

/* sub_4555c0: a participant's place, in words. */
reg32 resultsPlaceTextBody(Guest& guest, reg32 found, reg32 buffer)
{
    const reg32 text = guest.call(kLanguageText, fe::args({ guest.dword(found + participant::kPlace) + 0x2f }));
    guest.copyString(buffer, text);
    return afterInlineCopy(text);
}

/* sub_455600: a participant's car, by name. */
reg32 resultsCarNameBody(Guest& guest, reg32 found, reg32 buffer)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const reg32 driver = driverRecord(guest, found);
    const reg32 name = guest.call(kCarName, fe::args({ guest.dword(driver + kDriverCar) }));
    guest.copyString(buffer, name);
    return afterInlineCopy(name);
}

/* sub_455660, sub_4556c0: a line of a participant's car's text. */
reg32 resultsCarLineBody(Guest& guest, reg32 found, reg32 buffer, reg32 line)
{
    guest.setByte(buffer, 0);
    if (found == 0)
        return 0;
    const reg32 driver = driverRecord(guest, found);
    const reg32 text = guest.call(kCarText, fe::args({ guest.dword(driver + kDriverCar), line }));
    guest.copyString(buffer, text);
    return afterInlineCopy(text);
}

/* sub_455720: a participant's top speed written out, in the units chosen; "---"
 * when it has none. */
reg32 resultsTopSpeedTextBody(Guest& guest, reg32 found, reg32 buffer)
{
    constexpr reg32 kUnits = 0x6fbc68;
    constexpr reg32 kToKmh = 0x539910, kToMph = 0x539914;       // floats
    constexpr reg32 kSpeedFormat = 0x539900;                    // "%5.2f %s"
    constexpr reg32 kNoSpeed = 0x53990c;                        // "---"
    const float speed = guest.floatAt(found + participant::kTopSpeed);
    const std::uint16_t status = fp::compare(x86::Float(1.0), x86::Float(speed));
    if (!(status & kStatusBelow))
        return guest.call(kSprintf, fe::args({}, { buffer, kNoSpeed }));
    const reg32 units = guest.dword(kUnits);
    const bool metric = units == 3 || units == 4;
    const reg32 unitName = guest.call(kLanguageText, fe::args({ reg32(metric ? 0x157 : 0x156) }));
    const double value = double(fp::mul(x86::Float(speed), x86::Float(guest.floatAt(metric ? kToKmh : kToMph))));
    reg32 words[2];
    std::memcpy(words, &value, sizeof words);
    return guest.call(kSprintf, fe::args({}, { buffer, kSpeedFormat, words[0], words[1], unitName }));
}

/* ---------------------------------------------------------------------------
 * The direction pad: an item (widget 0x22), eight arrows round a centre.
 * ------------------------------------------------------------------------- */

constexpr reg32 kDirPadWidget = 0x22;
constexpr reg32 kDirPadActive = 0x677da0;
constexpr reg32 kDirPadX = 0x559874;            // int16[8], where the arrows are
constexpr reg32 kDirPadY = 0x559884;            // int16[8]
constexpr reg32 kDirPadShapes = 0x677da4;       // int[9]: the base of each, the centre last
constexpr reg32 kDirPadLitShapes = 0x677dc8;    // int[9]
constexpr reg32 kBaseX = 0x55982c;              // int16[9]
constexpr reg32 kBaseY = 0x55983e;              // int16[9]
constexpr reg32 kLitX = 0x559850;               // int16[9]
constexpr reg32 kLitY = 0x559862;               // int16[9]
constexpr std::int32_t kPadCentreX = 0x22f;
constexpr std::int32_t kPadCentreY = 0x11b;
constexpr std::int32_t kNear = 0x10;
constexpr std::int32_t kPadOriginX = 0x226;
constexpr std::int32_t kPadOriginY = 0x112;

std::int32_t distanceBetween(std::int32_t from, std::int32_t to)
{
    const std::int32_t by = std::int32_t(reg32(from) - reg32(to));
    return by > 0 ? by : std::int32_t(0 - reg32(by));
}

/* sub_457270: the arrow the pointer is on, or -1. */
reg32 dirPadUnderPointerBody(Guest& guest)
{
    for (reg32 i = 0; i < 8; ++i)
    {
        const std::int32_t across = distanceBetween(signedDword(guest, kPointerX), signedWord(guest, kDirPadX + 2 * i));
        const std::int32_t down = distanceBetween(signedDword(guest, kPointerY), signedWord(guest, kDirPadY + 2 * i));
        if (std::int32_t(reg32(across) + reg32(down)) < kNear)
            return i;
    }
    return reg32(-1);
}

/* sub_4572d0: whether the pointer is on the centre. */
reg32 dirPadCentreUnderPointerBody(Guest& guest)
{
    const std::int32_t across = distanceBetween(signedDword(guest, kPointerX), kPadCentreX);
    const std::int32_t down = distanceBetween(signedDword(guest, kPointerY), kPadCentreY);
    return std::int32_t(reg32(across) + reg32(down)) < kNear ? 1 : 0;
}

/* sub_457340: an arrow chosen: the direction it stands for handed on; -1 none.
 * Past them the code hands on its locals as they stand. */
reg32 dirPadChooseBody(Guest& guest, reg32 arrow)
{
    constexpr reg32 kOne = 0x3f800000, kMinusOne = 0xbf800000;
    struct Direction
    {
        reg32 x, y;
    };
    static constexpr Direction kDirections[9] = {
        { 0, 0 },               { 0, kOne },        { kMinusOne, kOne },
        { kMinusOne, 0 },       { kMinusOne, kMinusOne }, { 0, kMinusOne },
        { kOne, kMinusOne },    { kOne, 0 },        { kOne, kOne },
    };
    const reg32 index = arrow + 1;
    const Direction direction = index <= 8 ? kDirections[index]
                                           : Direction{ guest.dword(guest.frame()), guest.dword(guest.frame() + 4) };
    return guest.call(kSetDirection, fe::args({}, { direction.x, direction.y }));
}

/* One of the pad's shapes, where the item is. */
void drawDirPadShape(Guest& guest, reg32 item, reg32 shape, reg32 xTable, reg32 yTable, reg32 index)
{
    const std::int32_t y = signedWord(guest, item + sheet::kY) - (kPadOriginY - signedWord(guest, yTable + 2 * index));
    const std::int32_t x = signedWord(guest, item + sheet::kX) - (kPadOriginX - signedWord(guest, xTable + 2 * index));
    guest.call(kDrawShape, fe::args({ guest.dword(shape + 4 * index), reg32(x), reg32(y) }));
}

/* sub_457460: the pad drawn: only what the pointer is on, base and lit. */
reg32 dirPadDrawBody(Guest& guest, reg32 item, reg32 active)
{
    guest.setDword(kDirPadActive, active);
    if (guest.call(kMenuShownNow, fe::args({ item, kDirPadWidget })) == 0)
        return 0;
    constexpr reg32 kCentre = 8;
    const reg32 arrow = guest.call(kDirPadUnderPointer);
    if (arrow != reg32(-1))
    {
        drawDirPadShape(guest, item, kDirPadShapes, kBaseX, kBaseY, arrow);
        drawDirPadShape(guest, item, kDirPadLitShapes, kLitX, kLitY, arrow);
    }
    else if (guest.call(kDirPadCentreUnderPointer) != 0)
    {
        drawDirPadShape(guest, item, kDirPadShapes, kBaseX, kBaseY, kCentre);
        drawDirPadShape(guest, item, kDirPadLitShapes, kLitX, kLitY, kCentre);
    }
    return 0;
}

/* sub_4575a0: its keys: Enter on an arrow chooses it, on the centre stops. */
reg32 dirPadKeyBody(Guest& guest, reg32 item, std::uint16_t key)
{
    constexpr std::uint16_t kEnter = 0xd, kLeft = 0x4b00, kRight = 0x4d00, kSpace = 0x20;
    if (guest.call(kMenuShownNow, fe::args({ item, kDirPadWidget })) == 0)
        return 0;
    if (guest.dword(kDirPadActive) == 0)
        return 0;
    const reg32 answer = key == kLeft || key == kRight || key == kSpace ? 2 : 0;
    if (key == kEnter)
    {
        const reg32 arrow = guest.call(kDirPadUnderPointer);
        if (arrow != reg32(-1))
            guest.call(kDirPadChoose, fe::args({ arrow }));
        else if (guest.call(kDirPadCentreUnderPointer) != 0)
            guest.call(kSetDirection, fe::args({}, { 0, 0 }));
    }
    return answer;
}

/* sub_457630: its box round the item. */
reg32 dirPadLayoutBody(Guest& guest, reg32 item)
{
    if (guest.call(kMenuShownNow, fe::args({ item, kDirPadWidget })) != 0)
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
reg32 dirPadLoadShapesBody(Guest& guest)
{
    const reg32 name = guest.frame();
    guest.setByte(name, 'i');
    guest.setByte(name + 1, 'n');
    guest.setByte(name + 2, 't');
    guest.setByte(name + 4, 0);
    reg32 left = 0;
    for (reg32 i = 0; i < 9; ++i)
    {
        guest.setByte(name + 3, std::uint8_t('1' + i));
        guest.setDword(kDirPadShapes + 4 * i, guest.call(kShapeByName, fe::args({ name })));
        guest.setByte(name + 3, std::uint8_t('a' + i));
        left = guest.call(kShapeByName, fe::args({ name }));
        guest.setDword(kDirPadLitShapes + 4 * i, left);
    }
    return left;
}

}

void frontend::menuDrawPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44bdd0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return menuDrawPassBody(guest); });
}

void frontend::menuLeave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44be20", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return menuLeaveBody(guest); });
}

void frontend::awardsClear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e7d0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return awardsClearBody(guest); });
}

void frontend::awardsGrant(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e7f0", 0, 4, nullptr, 0, 0 };
    const reg32 award = cpu.eax, player = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return awardsGrantBody(guest, award, player); });
}

void frontend::awardsForRace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e820", 0, 24, nullptr, 0, fe::kRestoreAll };
    const reg32 player = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return awardsForRaceBody(guest, player); });
}

void frontend::awardsSpeak(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ea60", 0, 20, nullptr, 0, kEbxEcxEdxEdi };
    const reg32 kind = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return awardsSpeakBody(guest, kind); });
}

void frontend::awardsSpeakForSheet(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ed30", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return awardsSpeakForSheetBody(guest); });
}

void frontend::recordsCompare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f6e0", 0, 20, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsCompareBody(guest, a, b); });
}

void frontend::recordsSortBy(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f830", 0, 20 + 0x28, nullptr, 0, kEbxEcxEdxEsi };
    const reg32 key = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSortByBody(guest, key); });
}

void frontend::recordsCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f8d0", 0, 12, nullptr, 0, kEbxEcx };
    const reg32 track = cpu.eax, lapsClass = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsCountBody(guest, track, lapsClass); });
}

void frontend::recordByRank(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f910", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 rank = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordByRankBody(guest, rank); });
}

void frontend::recordsTimeQualifies(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f960", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 time = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsTimeQualifiesBody(guest, time); });
}

void frontend::recordsClearMarks(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f9b0", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsClearMarksBody(guest); });
}

void frontend::recordsInsert(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fa10", kRecordBytes, 20, nullptr, 0, kEcxEdx | kEsiEdi };
    const reg32 newRecord = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsInsertBody(guest, newRecord); });
}

void frontend::recordsHolderMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fb20", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsHolderMarkBody(guest); });
}

void frontend::recordsSetHolder(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fb30", 4, 12, nullptr, 0, kEsiEdi };
    const reg32 driver = cpu.eax, line0 = cpu.edx, line1 = cpu.ebx, time = cpu.ecx;
    const reg32 mark = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return recordsSetHolderBody(guest, driver, line0, line1, time, mark);
    });
}

void frontend::recordsSheetSortMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fc00", 0, 12, nullptr, 0, kEbxEcx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetSortMarkBody(guest, item); });
}

void frontend::recordsSheetDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fc70", 0, 20 + 0x580, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 item = cpu.eax, hovering = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetDrawBody(guest, item, hovering); });
}

void frontend::recordsSheetLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static constexpr GuestString kNames[] = { kArrowUp, kArrowDown, kArrowUpLit, kArrowDownLit };
    static fe::Site site{ "sub_450770", 0, 16, kNames, std::size(kNames), kEbxEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetLoadBody(guest, item); });
}

void frontend::recordsSheetSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450950", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetSaveBody(guest, item); });
}

void frontend::recordsSheetLayout(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450970", 0, 24, nullptr, 0, kEbxEcxEdxEsi };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetLayoutBody(guest, item); });
}

void frontend::recordsSheetKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450a90", 0, 24, nullptr, 0, kEcxEsiEdi };
    const reg32 item = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetKeyBody(guest, item, key); });
}

void frontend::recordsSheetIdle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450e30", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSheetIdleBody(guest, item); });
}

void frontend::recordsEnterResult(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450fe0", 0, 24 + 0x730, nullptr, 0, fe::kRestoreAll };
    const reg32 found = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsEnterResultBody(guest, cpu, found); });
}

void frontend::recordsEnterPlayers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4512f0", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsEnterPlayersBody(guest); });
}

void frontend::textFit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452090", 0, 20, nullptr, 0, kEcxEsiEdi };
    const reg32 text = cpu.eax, most = cpu.edx, font = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return textFitBody(guest, text, std::int32_t(most), font); });
}

void frontend::textDrawFitted(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452120", 12, 12 + 0x108, nullptr, 0, kEsiEdi };
    const reg32 text = cpu.eax, x = cpu.edx, y = cpu.ebx, font = cpu.ecx;
    const reg32 align = app->getMemory<reg32>(cpu.esp + 4), colour = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 most = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return textDrawFittedBody(guest, text, x, y, font, align, colour, most);
    });
}

void frontend::menuCaptureScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_452410: the screen's picture taken, the key's work.
    static fe::Site site{ "sub_452410", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.call(kCaptureScreen); });
}

void frontend::cheatAdvance(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452420", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const reg32 key = cpu.eax, progress = cpu.edx, code = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cheatAdvanceBody(guest, key, progress, code); });
}

void frontend::cheatKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4524e0", 0, 24 + 0x18, nullptr, 0, fe::kRestoreAll };
    const reg32 key = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cheatKeyBody(guest, key); });
}

void frontend::carArtExists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4530c0", 0, 20 + 0x100, nullptr, 0, kEbxEcxEdxEsi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carArtExistsBody(guest, car); });
}

void frontend::carViewRemember(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4534d0", 0, 12, kCompareNames, std::size(kCompareNames), kEcxEdx };
    const reg32 name = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carViewRememberBody(guest, name); });
}

void frontend::carStatsDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453940", 0, 24 + 0x10, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return carStatsDrawBody(guest); });
}

void frontend::carSpecsDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453f60", 0, 24 + 4, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return carSpecsDrawBody(guest); });
}

void frontend::carPassesFilter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454730", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return carPassesFilterBody(guest); });
}

void frontend::carListDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454780", 0, 24 + 0xc, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return carListDrawBody(guest); });
}

void frontend::carCompareDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4549c0", 0, 20 + 0x2c, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carCompareDrawBody(guest, item); });
}

void frontend::carCompareLoadArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454bf0", 0, 12 + 0x100, kBarNames, std::size(kBarNames), kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carCompareLoadArtBody(guest, item); });
}

void frontend::resultsHiddenRemote(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455020", 0, 4, nullptr, 0, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsHiddenRemoteBody(guest, car); });
}

void frontend::resultsNetworkNothing(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_455060: nothing: what a network game does more at this point in the
    // results, in the original.
    static fe::Site site{ "sub_455060", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.eax(); });
}

void frontend::tournamentScore(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455070", 0, 24, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return tournamentScoreBody(guest); });
}

void frontend::knockoutEliminate(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455140", 0, 24, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return knockoutEliminateBody(guest); });
}

void frontend::resultsCollectFinishers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455210", 0, 24, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 table = cpu.eax, count = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return resultsCollectBody(guest, table, count, participant::kFinisher);
    });
}

void frontend::resultsCollectLaps(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455280", 0, 24, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 table = cpu.eax, count = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return resultsCollectBody(guest, table, count, participant::kLapResult);
    });
}

void frontend::resultsAssign(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4552f0", 0, 24, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return resultsAssignBody(guest); });
}

void frontend::resultsBuild(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455410", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return resultsBuildBody(guest); });
}

void frontend::resultsName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455570", 0, 12, nullptr, 0, kEsiEdi };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsNameBody(guest, found, buffer); });
}

void frontend::resultsPlaceText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4555c0", 0, 12, nullptr, 0, kEsiEdi };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsPlaceTextBody(guest, found, buffer); });
}

void frontend::resultsCarName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455600", 0, 12, nullptr, 0, kEsiEdi };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsCarNameBody(guest, found, buffer); });
}

void frontend::resultsCarLine0(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455660", 0, 12, nullptr, 0, kEsiEdi };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsCarLineBody(guest, found, buffer, 0); });
}

void frontend::resultsCarLine1(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4556c0", 0, 12, nullptr, 0, kEsiEdi };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsCarLineBody(guest, found, buffer, 1); });
}

void frontend::resultsTopSpeedText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455720", 0, 12, nullptr, 0, fe::kRestoreEcx };
    const reg32 found = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resultsTopSpeedTextBody(guest, found, buffer); });
}

void frontend::dirPadUnderPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_457270", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return dirPadUnderPointerBody(guest); });
}

void frontend::dirPadCentreUnderPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4572d0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return dirPadCentreUnderPointerBody(guest); });
}

void frontend::dirPadChoose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_457340", 0, 24 + 8, nullptr, 0, fe::kRestoreAll };
    const reg32 arrow = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return dirPadChooseBody(guest, arrow); });
}

void frontend::dirPadDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_457460", 0, 20, nullptr, 0, kEbxEcxEsiEdi };
    const reg32 item = cpu.eax, active = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return dirPadDrawBody(guest, item, active); });
}

void frontend::dirPadKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4575a0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const reg32 item = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return dirPadKeyBody(guest, item, key); });
}

void frontend::dirPadAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_457620: a widget's slot: what it answers, always.
    static fe::Site site{ "sub_457620", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return reg32(-3); });
}

void frontend::dirPadLayout(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_457630", 0, 12, nullptr, 0, kEcxEdx };
    const reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return dirPadLayoutBody(guest, item); });
}

void frontend::dirPadNothing(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_457670: a widget's slot with nothing in it.
    static fe::Site site{ "sub_457670", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return guest.eax(); });
}

void frontend::dirPadLoadShapes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_457680", 0, 16 + 8, nullptr, 0, kEsiEdi | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return dirPadLoadShapesBody(guest); });
}

}
