#ifndef NFS3HP_NATIVE_FRONTEND_MENUB_H
#define NFS3HP_NATIVE_FRONTEND_MENUB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menub.cpp's typed functions (the menu loop's pass, the race's awards, the records'
 * tables and their sheet, text in a column, the codes, the showcase's car sheets, the race's
 * results as the menus keep them, the direction pad), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax, and a void one leaves eax as it came. */
namespace nfs3hp
{
namespace frontend
{
namespace menub
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x44bdd0: a pass over the shown screen: its items drawn, a dialog's over them, the pointer on top.
u32 menuDrawPass(App* app, ThreadState& ts);
// The game's 0x44be20: out of the menus, and any dialog closed.
u32 menuLeave(App* app, ThreadState& ts, u32 eax);
// The game's 0x44e7d0: no one has won anything.
u32 awardsClear(App* app, ThreadState& ts);
// The game's 0x44e7f0: an award to a player; the award back, edx left the player's offset.
u32 awardsGrant(App* app, ThreadState& ts, u32 award, u32 player);
// The game's 0x44e820: what a player won in this race, by the kind of race.
u32 awardsForRace(App* app, ThreadState& ts, u32 player);
// The game's 0x44ea60: the next line of this kind a player has won said, its flag spent.
u32 awardsSpeak(App* app, ThreadState& ts, u32 kind);
// The game's 0x44ed30: the lines of the postgame sheet shown.
u32 awardsSpeakForSheet(App* app, ThreadState& ts);
// The game's 0x44f6e0: qsort's: two records' places by the key asked for, then by the time; edx left the order.
u32 recordsCompare(App* app, ThreadState& ts, u32 slotA, u32 slotB);
// The game's 0x44f830: the records' ranks by a key; the count ranked back.
u32 recordsSortBy(App* app, ThreadState& ts, u32 key);
// The game's 0x44f8d0: how many records a track's list has, for a class of laps.
u32 recordsCount(App* app, ThreadState& ts, u32 track, u32 lapsClass);
// The game's 0x44f910: the record at a rank, or 0.
u32 recordByRank(App* app, ThreadState& ts, u32 rank);
// The game's 0x44f960: whether a time gets into the chosen list.
u32 recordsTimeQualifies(App* app, ThreadState& ts, u32 time);
// The game's 0x44f9b0: no record of any track or laps is this race's any more.
u32 recordsClearMarks(App* app, ThreadState& ts);
// The game's 0x44fa10: a record (by value on the stack; newRecord where it is) into the chosen list, its ranks written.
u32 recordsInsert(App* app, ThreadState& ts, u32 newRecord);
// The game's 0x44fb20: whether the holder of the best lap is this race's.
u32 recordsHolderMark(App* app, ThreadState& ts);
// The game's 0x44fb30: a new holder of the best lap; ecx left with the mark.
u32 recordsSetHolder(App* app, ThreadState& ts, u32 driver, u32 line0, u32 line1, u32 time, u32 mark);
// The game's 0x44fc00: the RECORDS sheet's mark on the column it is sorted by.
u32 recordsSheetSortMark(App* app, ThreadState& ts, u32 item);
// The game's 0x44fc70: the RECORDS sheet drawn, on every pass.
u32 recordsSheetDraw(App* app, ThreadState& ts, u32 item, u32 hovering);
// The game's 0x450770: the RECORDS sheet as its screen opens.
u32 recordsSheetLoad(App* app, ThreadState& ts, u32 item);
// The game's 0x450950: the RECORDS sheet's records saved as the screen closes.
u32 recordsSheetSave(App* app, ThreadState& ts, u32 item);
// The game's 0x450970: the RECORDS sheet's boxes, from its columns and where it has slid to.
u32 recordsSheetLayout(App* app, ThreadState& ts, u32 item);
// The game's 0x450a90: the RECORDS sheet's keys; edx left the answer.
u32 recordsSheetKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x450e30: the RECORDS sheet's slot with nothing to do: 0.
u32 recordsSheetIdle(App* app, ThreadState& ts, u32 item);
// The game's 0x450fe0: a participant's race into the chosen list and its best lap: which it got.
u32 recordsEnterResult(App* app, ThreadState& ts, u32 found);
// The game's 0x4512f0: the players' races entered, and what they got turned into awards.
u32 recordsEnterPlayers(App* app, ThreadState& ts);
// The game's 0x452090: a text cut to a width in a font, its end made "..." when cut.
u32 textFit(App* app, ThreadState& ts, u32 text, u32 most, u32 font);
// The game's 0x452120: a copy of a text, fitted to a width, drawn.
u32 textDrawFitted(App* app, ThreadState& ts, u32 text, u32 x, u32 y, u32 font, u32 align, u32 colour,
                     u32 most);
// The game's 0x452410: the screen's picture taken.
u32 menuCaptureScreen(App* app, ThreadState& ts);
// The game's 0x452420: a key against a code: 1 when the code is done; ebx left at the next letter.
u32 cheatAdvance(App* app, ThreadState& ts, u32 key, u32 progress, u32 code);
// The game's 0x4524e0: a key typed on any screen: "GO" and a track, and the codes.
u32 cheatKey(App* app, ThreadState& ts, u32 key);
// The game's 0x4530c0: whether a car's showcase art exists.
u32 carArtExists(App* app, ThreadState& ts, u32 car);
// The game's 0x4534d0: the screen left remembered for the car screen's way back.
u32 carViewRemember(App* app, ThreadState& ts, u32 name);
// The game's 0x453940: a page of the showcase car's statistics.
u32 carStatsDraw(App* app, ThreadState& ts);
// The game's 0x453f60: the showcase car's specifications, the one selected lit.
u32 carSpecsDraw(App* app, ThreadState& ts);
// The game's 0x454730: whether a car is in the class shown.
u32 carPassesFilter(App* app, ThreadState& ts, u32 car);
// The game's 0x454780: the list of the class's cars with a bar of the statistic chosen.
u32 carListDraw(App* app, ThreadState& ts);
// The game's 0x4549c0: two cars' statistics compared.
u32 carCompareDraw(App* app, ThreadState& ts, u32 item);
// The game's 0x454bf0: the comparison's bars' art loaded.
u32 carCompareLoadArt(App* app, ThreadState& ts, u32 item);
// The game's 0x455020: whether a network player's car has dropped out of the results.
u32 resultsHiddenRemote(App* app, ThreadState& ts, u32 car);
// The game's 0x455060: nothing: what a network game does more here (eax left as it came).
void resultsNetworkNothing(App* app, ThreadState& ts);
// The game's 0x455070: a tournament's race scored.
u32 tournamentScore(App* app, ThreadState& ts);
// The game's 0x455140: a knockout's round: the last still in is out.
u32 knockoutEliminate(App* app, ThreadState& ts);
// The game's 0x455210: the race's finishers copied into a table of results.
u32 resultsCollectFinishers(App* app, ThreadState& ts, u32 table, u32 count);
// The game's 0x455280: the race's cars with a lap result copied into a table of results.
u32 resultsCollectLaps(App* app, ThreadState& ts, u32 table, u32 count);
// The game's 0x4552f0: the results settled: those out marked, the lapped timed, the places.
u32 resultsAssign(App* app, ThreadState& ts);
// The game's 0x455410: the race's results made as the race ends.
u32 resultsBuild(App* app, ThreadState& ts);
// The game's 0x455570: a participant's name into a buffer.
u32 resultsName(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x4555c0: a participant's place, in words, into a buffer.
u32 resultsPlaceText(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x455600: a participant's car's name into a buffer.
u32 resultsCarName(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x455660: the first line of a participant's car's text into a buffer.
u32 resultsCarLine0(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x4556c0: the second line of a participant's car's text into a buffer.
u32 resultsCarLine1(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x455720: a participant's top speed written out into a buffer.
u32 resultsTopSpeedText(App* app, ThreadState& ts, u32 found, u32 buffer);
// The game's 0x457270: the direction pad's arrow the pointer is on, or -1.
u32 dirPadUnderPointer(App* app, ThreadState& ts);
// The game's 0x4572d0: whether the pointer is on the direction pad's centre.
u32 dirPadCentreUnderPointer(App* app, ThreadState& ts);
// The game's 0x457340: an arrow chosen: the direction it stands for handed on.
u32 dirPadChoose(App* app, ThreadState& ts, u32 arrow);
// The game's 0x457460: the direction pad drawn.
u32 dirPadDraw(App* app, ThreadState& ts, u32 item, u32 active);
// The game's 0x4575a0: the direction pad's keys.
u32 dirPadKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x457620: the direction pad's slot that always answers -3.
u32 dirPadAnswer(App* app, ThreadState& ts);
// The game's 0x457630: the direction pad's box round the item.
u32 dirPadLayout(App* app, ThreadState& ts, u32 item);
// The game's 0x457670: the direction pad's slot with nothing in it (eax left as it came).
void dirPadNothing(App* app, ThreadState& ts);
// The game's 0x457680: the direction pad's shapes by name.
u32 dirPadLoadShapes(App* app, ThreadState& ts);

}  // namespace menub
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
