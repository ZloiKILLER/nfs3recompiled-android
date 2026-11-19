#ifndef NFS3HP_NATIVE_FRONTEND_LOADA_H
#define NFS3HP_NATIVE_FRONTEND_LOADA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loada.cpp's typed functions (a race's loading around the track and the cars' files: the
 * cast and its roster, the cars in the race, the car list, kept files, the horizon, the loader's steps),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's function
 * leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loada
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x473340: nothing chosen yet.
u32 rosterClearUsed(App* app, ThreadState& ts);
// The game's 0x473360: the least chosen of `span` from the race's number at `index` on.
u32 rosterPickLeastUsed(App* app, ThreadState& ts, u32 kind, u32 index, u32 span);
// The game's 0x4733b0: the choice `pick` of the kind marked chosen.
u32 rosterMarkUsed(App* app, ThreadState& ts, u32 kind, u32 pick);
// The game's 0x4734d0: an opponent's car.
u32 rosterPickOpponentCar(App* app, ThreadState& ts, u32 index);
// The game's 0x473550: an opponent's driver, of seven.
u32 rosterPickOpponentDriver(App* app, ThreadState& ts, u32 index);
// The game's 0x473570: a cop's driver, of eight after the seven.
u32 rosterPickCopDriver(App* app, ThreadState& ts, u32 index);
// The game's 0x4735c0: a car of the traffic on this track.
u32 rosterPickTrafficCar(App* app, ThreadState& ts, u32 index);
// The game's 0x4736c0: a tournament's finalist in the car its winner has.
u32 rosterEntryFinalist(App* app, ThreadState& ts, u32 entry);
// The game's 0x473710: a person's entry.
u32 rosterEntryHuman(App* app, ThreadState& ts, u32 entry, u32 place, u32 player);
// The game's 0x473890: a cop's entry.
u32 rosterEntryCop(App* app, ThreadState& ts, u32 entry, u32 index, u32 pursuit);
// The game's 0x473960: an opponent's entry.
u32 rosterEntryOpponent(App* app, ThreadState& ts, u32 entry, u32 place);
// The game's 0x473a70: a traffic car's entry.
u32 rosterEntryTraffic(App* app, ThreadState& ts, u32 entry, u32 index);
// The game's 0x473ac0: a tournament's seven tracks in the race's order.
u32 rosterTournamentTracks(App* app, ThreadState& ts);
// The game's 0x473b70: the grid: each entry's rank by `ranks`, or as they stand.
u32 rosterGridOrder(App* app, ThreadState& ts, u32 count, u32 ranks);
// The game's 0x473bf0: BuildRegularRaceRoster: the people, the opponents, the traffic.
u32 rosterBuildRegular(App* app, ThreadState& ts);
// The game's 0x473ee0: the tournament's stage: its people, the opponents, its track.
u32 rosterBuildTournament(App* app, ThreadState& ts);
// The game's 0x474310: knockout: the stage's track, the people, the opponents.
u32 rosterBuildKnockout(App* app, ThreadState& ts);
// The game's 0x474640: hot pursuit: the people, the opponents, the cops, the traffic.
u32 rosterBuildHotPursuit(App* app, ThreadState& ts);
// The game's 0x474c60: the race's number at `index` on from its seed, 0 to 255.
u32 rosterSeedByte(App* app, ThreadState& ts, u32 index);
// The game's 0x474ca0: the race's cast by the kind of race, seeded by the race's seed.
u32 rosterBuild(App* app, ThreadState& ts);
// The game's 0x4283d0: the race's cars loaded, a stop looked for around each step.
u32 raceCarSteps(App* app, ThreadState& ts);
// The game's 0x4284e0: an event added to a car.
u32 carAddEvent(App* app, ThreadState& ts, u32 car, u32 a, u32 b, u32 c, u32 d, u32 time, u32 e);
// The game's 0x4285f0: a car's block of driving state cleared.
u32 carClearDriving(App* app, ThreadState& ts, u32 car);
// The game's 0x428700: a car put on the race's lists its entry's kind says.
u32 carJoinLists(App* app, ThreadState& ts, u32 car);
// The game's 0x428970: a car's state for the start of a race, or after a reset.
u32 carReset(App* app, ThreadState& ts, u32 car);
// The game's 0x428ad0: whether the car can be set down on the slice there.
u32 carPlaceIsClear(App* app, ThreadState& ts, u32 car, u32 slice, u32 across, u32 up, u32 right);
// The game's 0x428d60: the car set down where there is room near the slice.
u32 carPlaceOnRoad(App* app, ThreadState& ts, u32 car, u32 wanted);
// The game's 0x438b70: the car whose next is `next`, or 0.
u32 carBefore(App* app, ThreadState& ts, u32 next);
// The game's 0x438f60: one of a car's four stats.
u32 carStat(App* app, ThreadState& ts, u32 number, u32 which);
// The game's 0x439510: the three words at +0x394 of a car's record.
u32 carExtras(App* app, ThreadState& ts, u32 number);
// The game's 0x439530: 1 for a car with neither of two marks.
u32 carIdIsPlain(App* app, ThreadState& ts, u32 id);
// The game's 0x4395a0: a car's kept mark, 8 or 0.
u32 carIsKept(App* app, ThreadState& ts, u32 number);
// The game's 0x4395e0: a car's mark 2.
u32 carFlag2(App* app, ThreadState& ts, u32 number);
// The game's 0x439710: a byte of a car's record, at +0x3a0.
u32 carByte3a0(App* app, ThreadState& ts, u32 number);
// The game's 0x439a10: the first player in the car at `index` on the list at 0x554f84.
u32 playerToListedCar(App* app, ThreadState& ts, u32 index);
// The game's 0x439b70: every kept car freed.
u32 freeCars(App* app, ThreadState& ts);
// The game's 0x439bb0: a car's block freed: -1 for no car.
u32 carFreeData(App* app, ThreadState& ts, u32 car);
// The game's 0x439bf0: the block at the holder's +4 freed, if there is one.
u32 carFreeBlock(App* app, ThreadState& ts, u32 holder);
// The game's 0x43a4d0: qsort's comparison of two cars by their names.
u32 carCompareNames(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x43a6b0: the number of the car at `index` in the class; 0 for none.
u32 carInClass(App* app, ThreadState& ts, u32 index, u32 wantedClass);
// The game's 0x43a6e0: which cars the car lists offer.
u32 carFilterLists(App* app, ThreadState& ts, u32 flags, u32 pick);
// The game's 0x43b2a0: a kept car's folder removed.
u32 carRemoveFolder(App* app, ThreadState& ts, u32 car);
// The game's 0x43b390: every kept car off the list and freed, and the list read again.
u32 removeKeptCars(App* app, ThreadState& ts);
// The game's 0x4516a0: whether the current drive has `wanted` bytes free.
u32 diskHasRoom(App* app, ThreadState& ts, u32 wanted);
// The game's 0x452a00: a copy of a file in pieces of 512 bytes.
u32 copyFile(App* app, ThreadState& ts, u32 to, u32 from);
// The game's 0x478090: a kept race's players' cars found by their ids.
u32 savedResolvePlayers(App* app, ThreadState& ts, u32 saved);
// The game's 0x478100: a kept race's entries' cars found by their ids.
u32 savedResolveEntries(App* app, ThreadState& ts, u32 saved);
// The game's 0x4781b0: a race's entries' cars' ids written beside them.
u32 savedNameEntries(App* app, ThreadState& ts, u32 saved);
// The game's 0x4bcb00: a replay's file read, the game's settings kept but the replay's own.
u32 readReplayKeepingSettings(App* app, ThreadState& ts);
// The game's 0x4be2f0: the record of the two keys taken out of the list.
u32 listRemove(App* app, ThreadState& ts, u32 list, u32 keyA, u32 keyB);
// The game's 0x4dc550: the player's tuning of the car of `id`, or 0.
u32 tuningOf(App* app, ThreadState& ts, u32 player, u32 id);
// The game's 0x4dcbc0: both players' tunings copied in.
u32 tuningForPlayers(App* app, ThreadState& ts);
// The game's 0x478280: the Screen Size chosen.
u32 screenSizeChosen(App* app, ThreadState& ts);
// The game's 0x478290: the number of the Screen Size chosen.
u32 screenSizeNumber(App* app, ThreadState& ts);
// The game's 0x4782b0: the number of the Screen Size `size`.
u32 screenSizeNumberOf(App* app, ThreadState& ts, u32 size);
// The game's 0x4782c0: on to the horizon's setting up.
u32 horizonStartCall(App* app, ThreadState& ts);
// The game's 0x4782d0: on to the horizon's freeing.
u32 horizonFreeCall(App* app, ThreadState& ts);
// The game's 0x4782e0: a view's two words to `out`.
u32 skyViewTo(App* app, ThreadState& ts, u32 view, u32 out);
// The game's 0x478320: a piece's shape's width and height to theirs.
u32 shapeSize(App* app, ThreadState& ts, u32 shape, u32 width, u32 height);
// The game's 0x478b70: the horizon's eight pieces.
u32 horizonSetPieces(App* app, ThreadState& ts, u32 pieces);
// The game's 0x478bb0: the horizon's sixteen pieces and how high each is.
u32 horizonMeasure(App* app, ThreadState& ts);
// The game's 0x478c70: the horizon's setting up.
u32 horizonStart(App* app, ThreadState& ts);
// The game's 0x478c80: the horizon's block freed, and a render state set back.
u32 horizonFree(App* app, ThreadState& ts);
// The game's 0x4a33d0: whether to stop loading: 1 when asked for.
u32 stopCheck(App* app, ThreadState& ts);
// The game's 0x4b7c20: the cop's light kept.
u32 copKeepLight(App* app, ThreadState& ts);
// The game's 0x4b7c60: the track's cop's model loaded.
u32 copLoadModel(App* app, ThreadState& ts);
// The game's 0x4bc830: the race's things loaded, a stop looked for after each.
u32 raceSteps(App* app, ThreadState& ts);
// The game's 0x4bc870: the same for the race's resetting.
u32 raceResetSteps(App* app, ThreadState& ts);

}  // namespace loada
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
