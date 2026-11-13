#ifndef NFS3HP_NATIVE_FRONTEND_MENUB_H
#define NFS3HP_NATIVE_FRONTEND_MENUB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menub.cpp's typed functions (the menu loop's pass, the race's awards, the records'
 * tables and their sheet, text in a column, the codes, the showcase's car sheets, the race's
 * results as the menus keep them, the direction pad), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax, and a void one leaves eax as it came. */
namespace nfs3hp
{
namespace frontend
{
namespace menub
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x44bdd0: a pass over the shown screen: its items drawn, a dialog's over them, the pointer on top.
reg32 menuDrawPass(App* app, CPU& cpu);
// The game's 0x44be20: out of the menus, and any dialog closed.
reg32 menuLeave(App* app, CPU& cpu);
// The game's 0x44e7d0: no one has won anything.
reg32 awardsClear(App* app, CPU& cpu);
// The game's 0x44e7f0: an award to a player; the award back, edx left the player's offset.
reg32 awardsGrant(App* app, CPU& cpu, reg32 award, reg32 player);
// The game's 0x44e820: what a player won in this race, by the kind of race.
reg32 awardsForRace(App* app, CPU& cpu, reg32 player);
// The game's 0x44ea60: the next line of this kind a player has won said, its flag spent.
reg32 awardsSpeak(App* app, CPU& cpu, reg32 kind);
// The game's 0x44ed30: the lines of the postgame sheet shown.
reg32 awardsSpeakForSheet(App* app, CPU& cpu);
// The game's 0x44f6e0: qsort's: two records' places by the key asked for, then by the time; edx left the order.
reg32 recordsCompare(App* app, CPU& cpu, reg32 slotA, reg32 slotB);
// The game's 0x44f830: the records' ranks by a key; the count ranked back.
reg32 recordsSortBy(App* app, CPU& cpu, reg32 key);
// The game's 0x44f8d0: how many records a track's list has, for a class of laps.
reg32 recordsCount(App* app, CPU& cpu, reg32 track, reg32 lapsClass);
// The game's 0x44f910: the record at a rank, or 0.
reg32 recordByRank(App* app, CPU& cpu, reg32 rank);
// The game's 0x44f960: whether a time gets into the chosen list.
reg32 recordsTimeQualifies(App* app, CPU& cpu, reg32 time);
// The game's 0x44f9b0: no record of any track or laps is this race's any more.
reg32 recordsClearMarks(App* app, CPU& cpu);
// The game's 0x44fa10: a record (by value on the stack; newRecord where it is) into the chosen list, its ranks written.
reg32 recordsInsert(App* app, CPU& cpu, reg32 newRecord);
// The game's 0x44fb20: whether the holder of the best lap is this race's.
reg32 recordsHolderMark(App* app, CPU& cpu);
// The game's 0x44fb30: a new holder of the best lap; ecx left with the mark.
reg32 recordsSetHolder(App* app, CPU& cpu, reg32 driver, reg32 line0, reg32 line1, reg32 time, reg32 mark);
// The game's 0x44fc00: the RECORDS sheet's mark on the column it is sorted by.
reg32 recordsSheetSortMark(App* app, CPU& cpu, reg32 item);
// The game's 0x44fc70: the RECORDS sheet drawn, on every pass.
reg32 recordsSheetDraw(App* app, CPU& cpu, reg32 item, reg32 hovering);
// The game's 0x450770: the RECORDS sheet as its screen opens.
reg32 recordsSheetLoad(App* app, CPU& cpu, reg32 item);
// The game's 0x450950: the RECORDS sheet's records saved as the screen closes.
reg32 recordsSheetSave(App* app, CPU& cpu, reg32 item);
// The game's 0x450970: the RECORDS sheet's boxes, from its columns and where it has slid to.
reg32 recordsSheetLayout(App* app, CPU& cpu, reg32 item);
// The game's 0x450a90: the RECORDS sheet's keys; edx left the answer.
reg32 recordsSheetKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x450e30: the RECORDS sheet's slot with nothing to do: 0.
reg32 recordsSheetIdle(App* app, CPU& cpu, reg32 item);
// The game's 0x450fe0: a participant's race into the chosen list and its best lap: which it got.
reg32 recordsEnterResult(App* app, CPU& cpu, reg32 found);
// The game's 0x4512f0: the players' races entered, and what they got turned into awards.
reg32 recordsEnterPlayers(App* app, CPU& cpu);
// The game's 0x452090: a text cut to a width in a font, its end made "..." when cut.
reg32 textFit(App* app, CPU& cpu, reg32 text, reg32 most, reg32 font);
// The game's 0x452120: a copy of a text, fitted to a width, drawn.
reg32 textDrawFitted(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 font, reg32 align, reg32 colour,
                     reg32 most);
// The game's 0x452410: the screen's picture taken.
reg32 menuCaptureScreen(App* app, CPU& cpu);
// The game's 0x452420: a key against a code: 1 when the code is done; ebx left at the next letter.
reg32 cheatAdvance(App* app, CPU& cpu, reg32 key, reg32 progress, reg32 code);
// The game's 0x4524e0: a key typed on any screen: "GO" and a track, and the codes.
reg32 cheatKey(App* app, CPU& cpu, reg32 key);
// The game's 0x4530c0: whether a car's showcase art exists.
reg32 carArtExists(App* app, CPU& cpu, reg32 car);
// The game's 0x4534d0: the screen left remembered for the car screen's way back.
reg32 carViewRemember(App* app, CPU& cpu, reg32 name);
// The game's 0x453940: a page of the showcase car's statistics.
reg32 carStatsDraw(App* app, CPU& cpu);
// The game's 0x453f60: the showcase car's specifications, the one selected lit.
reg32 carSpecsDraw(App* app, CPU& cpu);
// The game's 0x454730: whether a car is in the class shown.
reg32 carPassesFilter(App* app, CPU& cpu, reg32 car);
// The game's 0x454780: the list of the class's cars with a bar of the statistic chosen.
reg32 carListDraw(App* app, CPU& cpu);
// The game's 0x4549c0: two cars' statistics compared.
reg32 carCompareDraw(App* app, CPU& cpu, reg32 item);
// The game's 0x454bf0: the comparison's bars' art loaded.
reg32 carCompareLoadArt(App* app, CPU& cpu, reg32 item);
// The game's 0x455020: whether a network player's car has dropped out of the results.
reg32 resultsHiddenRemote(App* app, CPU& cpu, reg32 car);
// The game's 0x455060: nothing: what a network game does more here (eax left as it came).
void resultsNetworkNothing(App* app, CPU& cpu);
// The game's 0x455070: a tournament's race scored.
reg32 tournamentScore(App* app, CPU& cpu);
// The game's 0x455140: a knockout's round: the last still in is out.
reg32 knockoutEliminate(App* app, CPU& cpu);
// The game's 0x455210: the race's finishers copied into a table of results.
reg32 resultsCollectFinishers(App* app, CPU& cpu, reg32 table, reg32 count);
// The game's 0x455280: the race's cars with a lap result copied into a table of results.
reg32 resultsCollectLaps(App* app, CPU& cpu, reg32 table, reg32 count);
// The game's 0x4552f0: the results settled: those out marked, the lapped timed, the places.
reg32 resultsAssign(App* app, CPU& cpu);
// The game's 0x455410: the race's results made as the race ends.
reg32 resultsBuild(App* app, CPU& cpu);
// The game's 0x455570: a participant's name into a buffer.
reg32 resultsName(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x4555c0: a participant's place, in words, into a buffer.
reg32 resultsPlaceText(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x455600: a participant's car's name into a buffer.
reg32 resultsCarName(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x455660: the first line of a participant's car's text into a buffer.
reg32 resultsCarLine0(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x4556c0: the second line of a participant's car's text into a buffer.
reg32 resultsCarLine1(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x455720: a participant's top speed written out into a buffer.
reg32 resultsTopSpeedText(App* app, CPU& cpu, reg32 found, reg32 buffer);
// The game's 0x457270: the direction pad's arrow the pointer is on, or -1.
reg32 dirPadUnderPointer(App* app, CPU& cpu);
// The game's 0x4572d0: whether the pointer is on the direction pad's centre.
reg32 dirPadCentreUnderPointer(App* app, CPU& cpu);
// The game's 0x457340: an arrow chosen: the direction it stands for handed on.
reg32 dirPadChoose(App* app, CPU& cpu, reg32 arrow);
// The game's 0x457460: the direction pad drawn.
reg32 dirPadDraw(App* app, CPU& cpu, reg32 item, reg32 active);
// The game's 0x4575a0: the direction pad's keys.
reg32 dirPadKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x457620: the direction pad's slot that always answers -3.
reg32 dirPadAnswer(App* app, CPU& cpu);
// The game's 0x457630: the direction pad's box round the item.
reg32 dirPadLayout(App* app, CPU& cpu, reg32 item);
// The game's 0x457670: the direction pad's slot with nothing in it (eax left as it came).
void dirPadNothing(App* app, CPU& cpu);
// The game's 0x457680: the direction pad's shapes by name.
reg32 dirPadLoadShapes(App* app, CPU& cpu);

}  // namespace menub
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::menub::menuDrawPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::menuLeave>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::awardsClear>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The award in eax, the player in edx.
template <>
struct ByAddress<frontend::menub::awardsGrant>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 award, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ award, player }));
    }
};

// The player in eax.
template <>
struct ByAddress<frontend::menub::awardsForRace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// The kind in eax.
template <>
struct ByAddress<frontend::menub::awardsSpeak>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind }));
    }
};

template <>
struct ByAddress<frontend::menub::awardsSpeakForSheet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The two slots in eax and edx.
template <>
struct ByAddress<frontend::menub::recordsCompare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slotA, x86::reg32 slotB)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ slotA, slotB }));
    }
};

// The key in eax.
template <>
struct ByAddress<frontend::menub::recordsSortBy>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key }));
    }
};

// The track in eax, the class of laps in edx.
template <>
struct ByAddress<frontend::menub::recordsCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 track, x86::reg32 lapsClass)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ track, lapsClass }));
    }
};

// The rank in eax.
template <>
struct ByAddress<frontend::menub::recordByRank>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 rank)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ rank }));
    }
};

// The time in eax.
template <>
struct ByAddress<frontend::menub::recordsTimeQualifies>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 time)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ time }));
    }
};

template <>
struct ByAddress<frontend::menub::recordsClearMarks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The record by value on the stack, 796 bytes (0x31c), popped: copied there from newRecord.
template <>
struct ByAddress<frontend::menub::recordsInsert>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 newRecord)
    {
        constexpr x86::reg32 kRecordBytes = 796;
        cpu.esp -= kRecordBytes;
        for (x86::reg32 i = 0; i < kRecordBytes; i += 4)
            app->getMemory<x86::reg32>(cpu.esp + i) = app->getMemory<x86::reg32>(newRecord + i);
        const x86::reg32 result = fe::callByAddress(app, cpu, Address, fe::args({}));
        cpu.esp += kRecordBytes;
        return result;
    }
};

template <>
struct ByAddress<frontend::menub::recordsHolderMark>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The names in eax, edx, ebx, the time in ecx, the mark on the stack, popped.
template <>
struct ByAddress<frontend::menub::recordsSetHolder>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 driver, x86::reg32 line0,
                           x86::reg32 line1, x86::reg32 time, x86::reg32 mark)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ driver, line0, line1, time }, { mark }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::recordsSheetSortMark>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax, whether the pointer counts in edx.
template <>
struct ByAddress<frontend::menub::recordsSheetDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 hovering)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, hovering }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::recordsSheetLoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::recordsSheetSave>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::recordsSheetLayout>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::menub::recordsSheetKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 menu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::recordsSheetIdle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The participant in eax.
template <>
struct ByAddress<frontend::menub::recordsEnterResult>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found }));
    }
};

template <>
struct ByAddress<frontend::menub::recordsEnterPlayers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The text in eax, the width in edx, the font in ebx.
template <>
struct ByAddress<frontend::menub::textFit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 most, x86::reg32 font)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, most, font }));
    }
};

// The text, x, y and font in eax, edx, ebx, ecx; the alignment, colour and width on the stack, popped.
template <>
struct ByAddress<frontend::menub::textDrawFitted>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 x, x86::reg32 y,
                           x86::reg32 font, x86::reg32 align, x86::reg32 colour, x86::reg32 most)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, x, y, font }, { align, colour, most }));
    }
};

template <>
struct ByAddress<frontend::menub::menuCaptureScreen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The key, the progress and the code in eax, edx, ebx.
template <>
struct ByAddress<frontend::menub::cheatAdvance>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key, x86::reg32 progress,
                           x86::reg32 code)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key, progress, code }));
    }
};

// The key in eax.
template <>
struct ByAddress<frontend::menub::cheatKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key }));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::menub::carArtExists>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The screen's name in eax.
template <>
struct ByAddress<frontend::menub::carViewRemember>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

template <>
struct ByAddress<frontend::menub::carStatsDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::carSpecsDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::menub::carPassesFilter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::menub::carListDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::carCompareDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::carCompareLoadArt>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::menub::resultsHiddenRemote>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::menub::resultsNetworkNothing>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::tournamentScore>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::knockoutEliminate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The table in eax, its count's address in edx.
template <>
struct ByAddress<frontend::menub::resultsCollectFinishers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 table, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ table, count }));
    }
};

// The table in eax, its count's address in edx.
template <>
struct ByAddress<frontend::menub::resultsCollectLaps>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 table, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ table, count }));
    }
};

template <>
struct ByAddress<frontend::menub::resultsAssign>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::resultsBuild>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsPlaceText>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsCarName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsCarLine0>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsCarLine1>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

// The participant in eax, the buffer in edx.
template <>
struct ByAddress<frontend::menub::resultsTopSpeedText>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 found, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ found, buffer }));
    }
};

template <>
struct ByAddress<frontend::menub::dirPadUnderPointer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::dirPadCentreUnderPointer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The arrow in eax.
template <>
struct ByAddress<frontend::menub::dirPadChoose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 arrow)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ arrow }));
    }
};

// The item in eax, whether it is active in edx.
template <>
struct ByAddress<frontend::menub::dirPadDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 active)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, active }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::menub::dirPadKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 menu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

template <>
struct ByAddress<frontend::menub::dirPadAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::menub::dirPadLayout>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menub::dirPadNothing>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menub::dirPadLoadShapes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
