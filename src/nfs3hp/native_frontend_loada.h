#ifndef NFS3HP_NATIVE_FRONTEND_LOADA_H
#define NFS3HP_NATIVE_FRONTEND_LOADA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loada.cpp's typed functions (a race's loading around the track and the cars' files: the
 * cast and its roster, the cars in the race, the car list, kept files, the horizon, the loader's steps),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's function
 * leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loada
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x473340: nothing chosen yet.
reg32 rosterClearUsed(App* app, CPU& cpu);
// The game's 0x473360: the least chosen of `span` from the race's number at `index` on.
reg32 rosterPickLeastUsed(App* app, CPU& cpu, reg32 kind, reg32 index, reg32 span);
// The game's 0x4733b0: the choice `pick` of the kind marked chosen.
reg32 rosterMarkUsed(App* app, CPU& cpu, reg32 kind, reg32 pick);
// The game's 0x4734d0: an opponent's car.
reg32 rosterPickOpponentCar(App* app, CPU& cpu, reg32 index);
// The game's 0x473550: an opponent's driver, of seven.
reg32 rosterPickOpponentDriver(App* app, CPU& cpu, reg32 index);
// The game's 0x473570: a cop's driver, of eight after the seven.
reg32 rosterPickCopDriver(App* app, CPU& cpu, reg32 index);
// The game's 0x4735c0: a car of the traffic on this track.
reg32 rosterPickTrafficCar(App* app, CPU& cpu, reg32 index);
// The game's 0x4736c0: a tournament's finalist in the car its winner has.
reg32 rosterEntryFinalist(App* app, CPU& cpu, reg32 entry);
// The game's 0x473710: a person's entry.
reg32 rosterEntryHuman(App* app, CPU& cpu, reg32 entry, reg32 place, reg32 player);
// The game's 0x473890: a cop's entry.
reg32 rosterEntryCop(App* app, CPU& cpu, reg32 entry, reg32 index, reg32 pursuit);
// The game's 0x473960: an opponent's entry.
reg32 rosterEntryOpponent(App* app, CPU& cpu, reg32 entry, reg32 place);
// The game's 0x473a70: a traffic car's entry.
reg32 rosterEntryTraffic(App* app, CPU& cpu, reg32 entry, reg32 index);
// The game's 0x473ac0: a tournament's seven tracks in the race's order.
reg32 rosterTournamentTracks(App* app, CPU& cpu);
// The game's 0x473b70: the grid: each entry's rank by `ranks`, or as they stand.
reg32 rosterGridOrder(App* app, CPU& cpu, reg32 count, reg32 ranks);
// The game's 0x473bf0: BuildRegularRaceRoster: the people, the opponents, the traffic.
reg32 rosterBuildRegular(App* app, CPU& cpu);
// The game's 0x473ee0: the tournament's stage: its people, the opponents, its track.
reg32 rosterBuildTournament(App* app, CPU& cpu);
// The game's 0x474310: knockout: the stage's track, the people, the opponents.
reg32 rosterBuildKnockout(App* app, CPU& cpu);
// The game's 0x474640: hot pursuit: the people, the opponents, the cops, the traffic.
reg32 rosterBuildHotPursuit(App* app, CPU& cpu);
// The game's 0x474c60: the race's number at `index` on from its seed, 0 to 255.
reg32 rosterSeedByte(App* app, CPU& cpu, reg32 index);
// The game's 0x474ca0: the race's cast by the kind of race, seeded by the race's seed.
reg32 rosterBuild(App* app, CPU& cpu);
// The game's 0x4283d0: the race's cars loaded, a stop looked for around each step.
reg32 raceCarSteps(App* app, CPU& cpu);
// The game's 0x4284e0: an event added to a car.
reg32 carAddEvent(App* app, CPU& cpu, reg32 car, reg32 a, reg32 b, reg32 c, reg32 d, reg32 time, reg32 e);
// The game's 0x4285f0: a car's block of driving state cleared.
reg32 carClearDriving(App* app, CPU& cpu, reg32 car);
// The game's 0x428700: a car put on the race's lists its entry's kind says.
reg32 carJoinLists(App* app, CPU& cpu, reg32 car);
// The game's 0x428970: a car's state for the start of a race, or after a reset.
reg32 carReset(App* app, CPU& cpu, reg32 car);
// The game's 0x428ad0: whether the car can be set down on the slice there.
reg32 carPlaceIsClear(App* app, CPU& cpu, reg32 car, reg32 slice, reg32 across, reg32 up, reg32 right);
// The game's 0x428d60: the car set down where there is room near the slice.
reg32 carPlaceOnRoad(App* app, CPU& cpu, reg32 car, reg32 wanted);
// The game's 0x438b70: the car whose next is `next`, or 0.
reg32 carBefore(App* app, CPU& cpu, reg32 next);
// The game's 0x438f60: one of a car's four stats.
reg32 carStat(App* app, CPU& cpu, reg32 number, reg32 which);
// The game's 0x439510: the three words at +0x394 of a car's record.
reg32 carExtras(App* app, CPU& cpu, reg32 number);
// The game's 0x439530: 1 for a car with neither of two marks.
reg32 carIdIsPlain(App* app, CPU& cpu, reg32 id);
// The game's 0x4395a0: a car's kept mark, 8 or 0.
reg32 carIsKept(App* app, CPU& cpu, reg32 number);
// The game's 0x4395e0: a car's mark 2.
reg32 carFlag2(App* app, CPU& cpu, reg32 number);
// The game's 0x439710: a byte of a car's record, at +0x3a0.
reg32 carByte3a0(App* app, CPU& cpu, reg32 number);
// The game's 0x439a10: the first player in the car at `index` on the list at 0x554f84.
reg32 playerToListedCar(App* app, CPU& cpu, reg32 index);
// The game's 0x439b70: every kept car freed.
reg32 freeCars(App* app, CPU& cpu);
// The game's 0x439bb0: a car's block freed: -1 for no car.
reg32 carFreeData(App* app, CPU& cpu, reg32 car);
// The game's 0x439bf0: the block at the holder's +4 freed, if there is one.
reg32 carFreeBlock(App* app, CPU& cpu, reg32 holder);
// The game's 0x43a4d0: qsort's comparison of two cars by their names.
reg32 carCompareNames(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x43a6b0: the number of the car at `index` in the class; 0 for none.
reg32 carInClass(App* app, CPU& cpu, reg32 index, reg32 wantedClass);
// The game's 0x43a6e0: which cars the car lists offer.
reg32 carFilterLists(App* app, CPU& cpu, reg32 flags, reg32 pick);
// The game's 0x43b2a0: a kept car's folder removed.
reg32 carRemoveFolder(App* app, CPU& cpu, reg32 car);
// The game's 0x43b390: every kept car off the list and freed, and the list read again.
reg32 removeKeptCars(App* app, CPU& cpu);
// The game's 0x4516a0: whether the current drive has `wanted` bytes free.
reg32 diskHasRoom(App* app, CPU& cpu, reg32 wanted);
// The game's 0x452a00: a copy of a file in pieces of 512 bytes.
reg32 copyFile(App* app, CPU& cpu, reg32 to, reg32 from);
// The game's 0x478090: a kept race's players' cars found by their ids.
reg32 savedResolvePlayers(App* app, CPU& cpu, reg32 saved);
// The game's 0x478100: a kept race's entries' cars found by their ids.
reg32 savedResolveEntries(App* app, CPU& cpu, reg32 saved);
// The game's 0x4781b0: a race's entries' cars' ids written beside them.
reg32 savedNameEntries(App* app, CPU& cpu, reg32 saved);
// The game's 0x4bcb00: a replay's file read, the game's settings kept but the replay's own.
reg32 readReplayKeepingSettings(App* app, CPU& cpu);
// The game's 0x4be2f0: the record of the two keys taken out of the list.
reg32 listRemove(App* app, CPU& cpu, reg32 list, reg32 keyA, reg32 keyB);
// The game's 0x4dc550: the player's tuning of the car of `id`, or 0.
reg32 tuningOf(App* app, CPU& cpu, reg32 player, reg32 id);
// The game's 0x4dcbc0: both players' tunings copied in.
reg32 tuningForPlayers(App* app, CPU& cpu);
// The game's 0x478280: the Screen Size chosen.
reg32 screenSizeChosen(App* app, CPU& cpu);
// The game's 0x478290: the number of the Screen Size chosen.
reg32 screenSizeNumber(App* app, CPU& cpu);
// The game's 0x4782b0: the number of the Screen Size `size`.
reg32 screenSizeNumberOf(App* app, CPU& cpu, reg32 size);
// The game's 0x4782c0: on to the horizon's setting up.
reg32 horizonStartCall(App* app, CPU& cpu);
// The game's 0x4782d0: on to the horizon's freeing.
reg32 horizonFreeCall(App* app, CPU& cpu);
// The game's 0x4782e0: a view's two words to `out`.
reg32 skyViewTo(App* app, CPU& cpu, reg32 view, reg32 out);
// The game's 0x478320: a piece's shape's width and height to theirs.
reg32 shapeSize(App* app, CPU& cpu, reg32 shape, reg32 width, reg32 height);
// The game's 0x478b70: the horizon's eight pieces.
reg32 horizonSetPieces(App* app, CPU& cpu, reg32 pieces);
// The game's 0x478bb0: the horizon's sixteen pieces and how high each is.
reg32 horizonMeasure(App* app, CPU& cpu);
// The game's 0x478c70: the horizon's setting up.
reg32 horizonStart(App* app, CPU& cpu);
// The game's 0x478c80: the horizon's block freed, and a render state set back.
reg32 horizonFree(App* app, CPU& cpu);
// The game's 0x4a33d0: whether to stop loading: 1 when asked for.
reg32 stopCheck(App* app, CPU& cpu);
// The game's 0x4b7c20: the cop's light kept.
reg32 copKeepLight(App* app, CPU& cpu);
// The game's 0x4b7c60: the track's cop's model loaded.
reg32 copLoadModel(App* app, CPU& cpu);
// The game's 0x4bc830: the race's things loaded, a stop looked for after each.
reg32 raceSteps(App* app, CPU& cpu);
// The game's 0x4bc870: the same for the race's resetting.
reg32 raceResetSteps(App* app, CPU& cpu);

}  // namespace loada
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
