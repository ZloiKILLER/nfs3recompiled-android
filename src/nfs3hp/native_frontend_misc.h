#ifndef NFS3HP_NATIVE_FRONTEND_MISC_H
#define NFS3HP_NATIVE_FRONTEND_MISC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_misc.cpp's typed functions (racetype, level, setup, the race's results, records,
 * postgame, loadsave, credits and credits1), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each calls
 * the game's other functions by address on `ts` and throws fe::Terminated when the game stopped
 * in one, as a call by address does; its result is what the game's function leaves in eax.  An
 * argument the code hands that the function does not read is named all the same: a call by
 * address hands it. */
namespace nfs3hp
{
namespace frontend
{
namespace misc
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x45d630: level as it opens.
u32 levelOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x452f80: setup as it opens: the network shut if the race left one; a developer's start.
u32 setupOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x452f60: setup on every pass: the attract-mode movie, when it is time.
u32 setupOnFrame(App* app, ThreadState& ts);
// The game's 0x452ac0: the race dialog answered.
u32 setupRaceAnswered(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x452bd0: the race dialog: its buttons one fewer on the network, each with its line of help; the last.
u32 setupAskRace(App* app, ThreadState& ts, u32 canCancel);
// The game's 0x452d40: two players side by side.
u32 setupTwoPlayersClicked(App* app, ThreadState& ts);
// The game's 0x452f20: one player.
u32 setupOnePlayerClicked(App* app, ThreadState& ts);
// The game's 0x452d80: the web site's "not possible" answered: the dialog closes.
u32 setupWebSiteRefused(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x452db0: whether the screen up is setup itself.
u32 setupIsCurrent(App* app, ThreadState& ts);
// The game's 0x452de0: the web site's question answered: Yes leaves the menus for it.
u32 setupWebSiteAnswered(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x452e60: the web site's button: asked first, or refused if the machine has no way to dial out.
u32 setupWebSiteClicked(App* app, ThreadState& ts);
// The game's 0x4554e0: a group's participant by its id, or 0.
u32 resultsParticipantById(App* app, ThreadState& ts, u32 group, u32 id);
// The game's 0x455530: its car's two ranks, into *a and *b, if there is one.
u32 resultsParticipantRanks(App* app, ThreadState& ts, u32 found, u32 a, u32 b);
// The game's 0x4557b0: whether it is what the mask asks: 4 and 8 flags it has, 1 and 2 flags it has not.
u32 resultsParticipantMatches(App* app, ThreadState& ts, u32 found, u32 mask, u32 state);
// The game's 0x455840: for qsort: two participants in the order the sort asked for by kSortKey (0 none, 1 rank.
u32 resultsCompare(App* app, ThreadState& ts, u32 aAt, u32 bAt);
// The game's 0x455c10: a group sorted, by a key and a way: the order's table filled, -1 past its participants.
u32 resultsSort(App* app, ThreadState& ts, u32 group, u32 key, u32 descending);
// The game's 0x455cd0: the index-th in the order that matches a mask (0: any), or 0.
u32 resultsNth(App* app, ThreadState& ts, u32 group, u32 index, u32 mask, u32 state);
// The game's 0x455dd0: the same for an order made for the occasion, the group's kept as it was.
u32 resultsNthSorted(App* app, ThreadState& ts, u32 group, u32 index, u32 key, u32 descending, u32 mask,
                       u32 state);
// The game's 0x44f670: a track's and a direction's records chosen.
u32 recordsSelect(App* app, ThreadState& ts, u32 track, u32 kind);
// The game's 0x44fa90: the holder of the chosen record: whether there is one, and its names, into three buffers.
u32 recordsRead(App* app, ThreadState& ts, u32 first, u32 second, u32 third);
// The game's 0x44fbc0: the chosen records wiped.
u32 recordsClear(App* app, ThreadState& ts);
// The game's 0x450f70: records as it opens.
u32 recordsOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x450f20: records on every pass: delete is open while there is something to delete.
u32 recordsOnFrame(App* app, ThreadState& ts);
// The game's 0x450eb0: delete's click: asked first.
u32 recordsDeleteClicked(App* app, ThreadState& ts);
// The game's 0x450e50: the answer: Yes wipes them.
u32 recordsDeleteAnswered(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x4cef00: whether a track has a record's holder, the records' file loaded for the moment if it is not.
u32 recordsHolder(App* app, ThreadState& ts, u32 track);
// The game's 0x442b60: a bonus track open from now on.
u32 postgameUnlockTrack(App* app, ThreadState& ts, u32 track);
// The game's 0x44e170: a line of an award.
u32 postgameAwardMessage(App* app, ThreadState& ts, u32 id);
// The game's 0x44e1a0: what the race just won opens: a bonus track or car, and the line that says so.
u32 postgameCheckAwards(App* app, ThreadState& ts);
// The game's 0x44e4c0: a network race left, by whichever way this machine has.
u32 postgameLeaveNetwork(App* app, ThreadState& ts);
// The game's 0x44e510: "leave?" answered.
u32 postgameLeaveAnswered(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x44e550: "leave?" with Yes and No.
u32 postgameAskLeave(App* app, ThreadState& ts, u32 lineId);
// The game's 0x44e5b0: Main Menu's click: asked first if a series has more to race.
u32 postgameMainMenuClicked(App* app, ThreadState& ts);
// The game's 0x44e600: the same in a network race.
u32 postgameNetworkMainMenuClicked(App* app, ThreadState& ts);
// The game's 0x44e6c0: why a replay of this race cannot be kept.
u32 postgameCannotSaveReplay(App* app, ThreadState& ts);
// The game's 0x44e710: View Replay's click: the kept replay, the one just raced.
u32 postgameViewReplayClicked(App* app, ThreadState& ts);
// The game's 0x44e7a0: the toggle's click: the next sheet in the race's cycle.
u32 postgameToggleSheets(App* app, ThreadState& ts);
// The game's 0x44efd0: postgame as it opens.
u32 postgameOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44eda0: postgame on every pass: the network's end told; the sheet shown coming up.
u32 postgameOnFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x4cef80: whether anybody finished the race, as the developer's flag lets it.
u32 postgameFinishers(App* app, ThreadState& ts);
// The game's 0x4cf000: how many are still in.
u32 postgameFinishersCount(App* app, ThreadState& ts);
// The game's 0x4cf040: whether a machine in the network race cannot keep a replay.
u32 postgameReplayExcluded(App* app, ThreadState& ts);
// The game's 0x4cf090: whether a series goes on with another race.
u32 postgameRaceAgain(App* app, ThreadState& ts);
// The game's 0x4461d0: a kept knockout's description.
u32 loadSaveKnockoutInfo(App* app, ThreadState& ts, u32 path);
// The game's 0x4462e0: a kept tournament's description.
u32 loadSaveTournamentInfo(App* app, ThreadState& ts, u32 path);
// The game's 0x446420: a kept ghost's description.
u32 loadSaveGhostInfo(App* app, ThreadState& ts, u32 path);
// The game's 0x4465e0: a kept knockout loaded.
u32 loadSaveLoadKnockout(App* app, ThreadState& ts, u32 path);
// The game's 0x446800: a kept tournament loaded.
u32 loadSaveLoadTournament(App* app, ThreadState& ts, u32 path);
// The game's 0x4466a0: a kept ghost loaded, and the track's record's holder looked up.
u32 loadSaveLoadGhost(App* app, ThreadState& ts, u32 path);
// The game's 0x446ad0: the kept tournaments shown.
u32 loadSaveShowTournaments(App* app, ThreadState& ts);
// The game's 0x446b10: the kept knockouts shown.
u32 loadSaveShowKnockouts(App* app, ThreadState& ts);
// The game's 0x446b60: the kept replays shown.
u32 loadSaveShowReplays(App* app, ThreadState& ts);
// The game's 0x446ba0: the kept ghosts shown.
u32 loadSaveShowGhosts(App* app, ThreadState& ts);
// The game's 0x446bf0: the file cannot be loaded.
u32 loadSaveLoadFailed(App* app, ThreadState& ts);
// The game's 0x446c50: Load Now's click: what the list has selected, loaded.
u32 loadSaveLoadNow(App* app, ThreadState& ts);
// The game's 0x446ea0: the file selected deleted, the list made anew.
u32 loadSaveDeleteSelected(App* app, ThreadState& ts);
// The game's 0x446ff0: "delete?" answered.
u32 loadSaveDeleteAnswered(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x447030: Delete's click: asked first, naming the file.
u32 loadSaveDeleteClicked(App* app, ThreadState& ts);
// The game's 0x4470e0: the description's box drawn: its title and its lines, those it has.
u32 loadSaveDrawDescription(App* app, ThreadState& ts, u32 item);
// The game's 0x4471a0: the list's selection moved: the description of the one selected.
u32 loadSaveSelectionMoved(App* app, ThreadState& ts, u32 list, u32 selected);
// The game's 0x447470: loadsave as it opens: the replays listed first.
u32 loadSaveOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x447370: loadsave on every pass: the box and the buttons as the list has it, and the list's title.
u32 loadSaveOnFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x439ce0: whether the bonus car is installed.
u32 creditsHaveBonusCar(App* app, ThreadState& ts);
// The game's 0x441cb0: the page asked for, drawn into a canvas made into a texture, `previous`'s reused.
u32 creditsRenderPage(App* app, ThreadState& ts, u32 previous);
// The game's 0x442370: next's click: on to the next page, once it is still; round to the first after the last.
u32 creditsNextPage(App* app, ThreadState& ts, u32 item);
// The game's 0x4423e0: previous's click: back a page, round to the last from the first.
u32 creditsPreviousPage(App* app, ThreadState& ts);
// The game's 0x442850: credits as it opens: the slide's start, the fonts.
u32 creditsOnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x442980: credits as it closes.
u32 creditsOnExit(App* app, ThreadState& ts);
// The game's 0x442580: credits on every pass: the page's slide.
u32 creditsOnFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x454d60: the page's title, and on the button the next's, with "..." but in the second language.
u32 credits1ShowPage(App* app, ThreadState& ts, u32 nextButton);
// The game's 0x454e40: next's click.
u32 credits1NextClicked(App* app, ThreadState& ts, u32 item);
// The game's 0x454ea0: credits1 as it opens: from the first page, the one it was on kept.
u32 credits1OnEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x454e80: credits1 as it closes: the page it was on.
u32 credits1OnExit(App* app, ThreadState& ts);
// The game's 0x45dab0: racetype as it closes: the item selected is the race's type, of six.
u32 raceTypeOnExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x44f5d0: whether the records' file is in memory.
u32 recordsLoaded(App* app, ThreadState& ts);
// The game's 0x44e460: Save's click, the dialog that asks for a knockout's name.
u32 postgameSaveKnockoutClicked(App* app, ThreadState& ts);
// The game's 0x44e480: Save's click, the dialog that asks for a tournament's name.
u32 postgameSaveTournamentClicked(App* app, ThreadState& ts);
// The game's 0x44e4a0: Save's click, the dialog that asks for a ghost's name.
u32 postgameSaveGhostClicked(App* app, ThreadState& ts);
// The game's 0x44e6a0: Save's click, the dialog that asks for the replay's name.
u32 postgameSaveReplayClicked(App* app, ThreadState& ts);
// The game's 0x44e690: the menu loop told to quit.
u32 postgameQuitClicked(App* app, ThreadState& ts);
// The game's 0x44f500: postgame as it closes: the words spoken over the sheets stopped.
u32 postgameOnExit(App* app, ThreadState& ts);

}  // namespace misc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
