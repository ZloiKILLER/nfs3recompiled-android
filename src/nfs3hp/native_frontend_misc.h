#ifndef NFS3HP_NATIVE_FRONTEND_MISC_H
#define NFS3HP_NATIVE_FRONTEND_MISC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_misc.cpp's typed functions (racetype, level, setup, the race's results, records,
 * postgame, loadsave, credits and credits1), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each calls
 * the game's other functions by address on `cpu` and throws fe::Terminated when the game stopped
 * in one, as a call by address does; its result is what the game's function leaves in eax.  An
 * argument the code hands that the function does not read is named all the same: a call by
 * address hands it. */
namespace nfs3hp
{
namespace frontend
{
namespace misc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45d630: level as it opens.
reg32 levelOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x452f80: setup as it opens: the network shut if the race left one; a developer's start.
reg32 setupOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x452f60: setup on every pass: the attract-mode movie, when it is time.
reg32 setupOnFrame(App* app, CPU& cpu);
// The game's 0x452ac0: the race dialog answered.
reg32 setupRaceAnswered(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x452bd0: the race dialog: its buttons one fewer on the network, each with its line of help; the last.
reg32 setupAskRace(App* app, CPU& cpu, reg32 canCancel);
// The game's 0x452d40: two players side by side.
reg32 setupTwoPlayersClicked(App* app, CPU& cpu);
// The game's 0x452f20: one player.
reg32 setupOnePlayerClicked(App* app, CPU& cpu);
// The game's 0x452d80: the web site's "not possible" answered: the dialog closes.
reg32 setupWebSiteRefused(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x452db0: whether the screen up is setup itself.
reg32 setupIsCurrent(App* app, CPU& cpu);
// The game's 0x452de0: the web site's question answered: Yes leaves the menus for it.
reg32 setupWebSiteAnswered(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x452e60: the web site's button: asked first, or refused if the machine has no way to dial out.
reg32 setupWebSiteClicked(App* app, CPU& cpu);
// The game's 0x4554e0: a group's participant by its id, or 0.
reg32 resultsParticipantById(App* app, CPU& cpu, reg32 group, reg32 id);
// The game's 0x455530: its car's two ranks, into *a and *b, if there is one.
reg32 resultsParticipantRanks(App* app, CPU& cpu, reg32 found, reg32 a, reg32 b);
// The game's 0x4557b0: whether it is what the mask asks: 4 and 8 flags it has, 1 and 2 flags it has not.
reg32 resultsParticipantMatches(App* app, CPU& cpu, reg32 found, reg32 mask, reg32 state);
// The game's 0x455840: for qsort: two participants in the order the sort asked for by kSortKey (0 none, 1 rank.
reg32 resultsCompare(App* app, CPU& cpu, reg32 aAt, reg32 bAt);
// The game's 0x455c10: a group sorted, by a key and a way: the order's table filled, -1 past its participants.
reg32 resultsSort(App* app, CPU& cpu, reg32 group, reg32 key, reg32 descending);
// The game's 0x455cd0: the index-th in the order that matches a mask (0: any), or 0.
reg32 resultsNth(App* app, CPU& cpu, reg32 group, reg32 index, reg32 mask, reg32 state);
// The game's 0x455dd0: the same for an order made for the occasion, the group's kept as it was.
reg32 resultsNthSorted(App* app, CPU& cpu, reg32 group, reg32 index, reg32 key, reg32 descending, reg32 mask,
                       reg32 state);
// The game's 0x44f670: a track's and a direction's records chosen.
reg32 recordsSelect(App* app, CPU& cpu, reg32 track, reg32 kind);
// The game's 0x44fa90: the holder of the chosen record: whether there is one, and its names, into three buffers.
reg32 recordsRead(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third);
// The game's 0x44fbc0: the chosen records wiped.
reg32 recordsClear(App* app, CPU& cpu);
// The game's 0x450f70: records as it opens.
reg32 recordsOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x450f20: records on every pass: delete is open while there is something to delete.
reg32 recordsOnFrame(App* app, CPU& cpu);
// The game's 0x450eb0: delete's click: asked first.
reg32 recordsDeleteClicked(App* app, CPU& cpu);
// The game's 0x450e50: the answer: Yes wipes them.
reg32 recordsDeleteAnswered(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x4cef00: whether a track has a record's holder, the records' file loaded for the moment if it is not.
reg32 recordsHolder(App* app, CPU& cpu, reg32 track);
// The game's 0x442b60: a bonus track open from now on.
reg32 postgameUnlockTrack(App* app, CPU& cpu, reg32 track);
// The game's 0x44e170: a line of an award.
reg32 postgameAwardMessage(App* app, CPU& cpu, reg32 id);
// The game's 0x44e1a0: what the race just won opens: a bonus track or car, and the line that says so.
reg32 postgameCheckAwards(App* app, CPU& cpu);
// The game's 0x44e4c0: a network race left, by whichever way this machine has.
reg32 postgameLeaveNetwork(App* app, CPU& cpu);
// The game's 0x44e510: "leave?" answered.
reg32 postgameLeaveAnswered(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x44e550: "leave?" with Yes and No.
reg32 postgameAskLeave(App* app, CPU& cpu, reg32 lineId);
// The game's 0x44e5b0: Main Menu's click: asked first if a series has more to race.
reg32 postgameMainMenuClicked(App* app, CPU& cpu);
// The game's 0x44e600: the same in a network race.
reg32 postgameNetworkMainMenuClicked(App* app, CPU& cpu);
// The game's 0x44e6c0: why a replay of this race cannot be kept.
reg32 postgameCannotSaveReplay(App* app, CPU& cpu);
// The game's 0x44e710: View Replay's click: the kept replay, the one just raced.
reg32 postgameViewReplayClicked(App* app, CPU& cpu);
// The game's 0x44e7a0: the toggle's click: the next sheet in the race's cycle.
reg32 postgameToggleSheets(App* app, CPU& cpu);
// The game's 0x44efd0: postgame as it opens.
reg32 postgameOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44eda0: postgame on every pass: the network's end told; the sheet shown coming up.
reg32 postgameOnFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x4cef80: whether anybody finished the race, as the developer's flag lets it.
reg32 postgameFinishers(App* app, CPU& cpu);
// The game's 0x4cf000: how many are still in.
reg32 postgameFinishersCount(App* app, CPU& cpu);
// The game's 0x4cf040: whether a machine in the network race cannot keep a replay.
reg32 postgameReplayExcluded(App* app, CPU& cpu);
// The game's 0x4cf090: whether a series goes on with another race.
reg32 postgameRaceAgain(App* app, CPU& cpu);
// The game's 0x4461d0: a kept knockout's description.
reg32 loadSaveKnockoutInfo(App* app, CPU& cpu, reg32 path);
// The game's 0x4462e0: a kept tournament's description.
reg32 loadSaveTournamentInfo(App* app, CPU& cpu, reg32 path);
// The game's 0x446420: a kept ghost's description.
reg32 loadSaveGhostInfo(App* app, CPU& cpu, reg32 path);
// The game's 0x4465e0: a kept knockout loaded.
reg32 loadSaveLoadKnockout(App* app, CPU& cpu, reg32 path);
// The game's 0x446800: a kept tournament loaded.
reg32 loadSaveLoadTournament(App* app, CPU& cpu, reg32 path);
// The game's 0x4466a0: a kept ghost loaded, and the track's record's holder looked up.
reg32 loadSaveLoadGhost(App* app, CPU& cpu, reg32 path);
// The game's 0x446ad0: the kept tournaments shown.
reg32 loadSaveShowTournaments(App* app, CPU& cpu);
// The game's 0x446b10: the kept knockouts shown.
reg32 loadSaveShowKnockouts(App* app, CPU& cpu);
// The game's 0x446b60: the kept replays shown.
reg32 loadSaveShowReplays(App* app, CPU& cpu);
// The game's 0x446ba0: the kept ghosts shown.
reg32 loadSaveShowGhosts(App* app, CPU& cpu);
// The game's 0x446bf0: the file cannot be loaded.
reg32 loadSaveLoadFailed(App* app, CPU& cpu);
// The game's 0x446c50: Load Now's click: what the list has selected, loaded.
reg32 loadSaveLoadNow(App* app, CPU& cpu);
// The game's 0x446ea0: the file selected deleted, the list made anew.
reg32 loadSaveDeleteSelected(App* app, CPU& cpu);
// The game's 0x446ff0: "delete?" answered.
reg32 loadSaveDeleteAnswered(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x447030: Delete's click: asked first, naming the file.
reg32 loadSaveDeleteClicked(App* app, CPU& cpu);
// The game's 0x4470e0: the description's box drawn: its title and its lines, those it has.
reg32 loadSaveDrawDescription(App* app, CPU& cpu, reg32 item);
// The game's 0x4471a0: the list's selection moved: the description of the one selected.
reg32 loadSaveSelectionMoved(App* app, CPU& cpu, reg32 list, reg32 selected);
// The game's 0x447470: loadsave as it opens: the replays listed first.
reg32 loadSaveOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x447370: loadsave on every pass: the box and the buttons as the list has it, and the list's title.
reg32 loadSaveOnFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x439ce0: whether the bonus car is installed.
reg32 creditsHaveBonusCar(App* app, CPU& cpu);
// The game's 0x441cb0: the page asked for, drawn into a canvas made into a texture, `previous`'s reused.
reg32 creditsRenderPage(App* app, CPU& cpu, reg32 previous);
// The game's 0x442370: next's click: on to the next page, once it is still; round to the first after the last.
reg32 creditsNextPage(App* app, CPU& cpu, reg32 item);
// The game's 0x4423e0: previous's click: back a page, round to the last from the first.
reg32 creditsPreviousPage(App* app, CPU& cpu);
// The game's 0x442850: credits as it opens: the slide's start, the fonts.
reg32 creditsOnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x442980: credits as it closes.
reg32 creditsOnExit(App* app, CPU& cpu);
// The game's 0x442580: credits on every pass: the page's slide.
reg32 creditsOnFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x454d60: the page's title, and on the button the next's, with "..." but in the second language.
reg32 credits1ShowPage(App* app, CPU& cpu, reg32 nextButton);
// The game's 0x454e40: next's click.
reg32 credits1NextClicked(App* app, CPU& cpu, reg32 item);
// The game's 0x454ea0: credits1 as it opens: from the first page, the one it was on kept.
reg32 credits1OnEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x454e80: credits1 as it closes: the page it was on.
reg32 credits1OnExit(App* app, CPU& cpu);
// The game's 0x45dab0: racetype as it closes: the item selected is the race's type, of six.
reg32 raceTypeOnExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x44f5d0: whether the records' file is in memory.
reg32 recordsLoaded(App* app, CPU& cpu);
// The game's 0x44e460: Save's click, the dialog that asks for a knockout's name.
reg32 postgameSaveKnockoutClicked(App* app, CPU& cpu);
// The game's 0x44e480: Save's click, the dialog that asks for a tournament's name.
reg32 postgameSaveTournamentClicked(App* app, CPU& cpu);
// The game's 0x44e4a0: Save's click, the dialog that asks for a ghost's name.
reg32 postgameSaveGhostClicked(App* app, CPU& cpu);
// The game's 0x44e6a0: Save's click, the dialog that asks for the replay's name.
reg32 postgameSaveReplayClicked(App* app, CPU& cpu);
// The game's 0x44e690: the menu loop told to quit.
reg32 postgameQuitClicked(App* app, CPU& cpu);
// The game's 0x44f500: postgame as it closes: the words spoken over the sheets stopped.
reg32 postgameOnExit(App* app, CPU& cpu);

}  // namespace misc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
