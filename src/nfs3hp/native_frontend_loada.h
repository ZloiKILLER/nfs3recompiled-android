#ifndef NFS3HP_NATIVE_FRONTEND_LOADA_H
#define NFS3HP_NATIVE_FRONTEND_LOADA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loada.cpp's typed functions (a race's loading around the track and the cars' files: the
 * cast and its roster, the cars in the race, the car list, kept files, the horizon, the loader's steps),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address (ByAddress,
 * for the checks' stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's function
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

template <>
struct ByAddress<frontend::loada::rosterClearUsed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// kind in eax, index in edx, span in ebx.
template <>
struct ByAddress<frontend::loada::rosterPickLeastUsed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 index,
                           x86::reg32 span)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind, index, span }));
    }
};

// kind in eax, pick in edx.
template <>
struct ByAddress<frontend::loada::rosterMarkUsed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 pick)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind, pick }));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::rosterPickOpponentCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::rosterPickOpponentDriver>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::rosterPickCopDriver>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::rosterPickTrafficCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// entry in eax.
template <>
struct ByAddress<frontend::loada::rosterEntryFinalist>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entry }));
    }
};

// entry in eax, place in edx, player in ebx.
template <>
struct ByAddress<frontend::loada::rosterEntryHuman>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 place,
                           x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entry, place, player }));
    }
};

// entry in eax, index in edx, pursuit in ebx.
template <>
struct ByAddress<frontend::loada::rosterEntryCop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 index,
                           x86::reg32 pursuit)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entry, index, pursuit }));
    }
};

// entry in eax, place in edx.
template <>
struct ByAddress<frontend::loada::rosterEntryOpponent>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entry, place }));
    }
};

// entry in eax, index in edx.
template <>
struct ByAddress<frontend::loada::rosterEntryTraffic>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entry, index }));
    }
};

template <>
struct ByAddress<frontend::loada::rosterTournamentTracks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// count in eax, ranks in edx.
template <>
struct ByAddress<frontend::loada::rosterGridOrder>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 ranks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, ranks }));
    }
};

template <>
struct ByAddress<frontend::loada::rosterBuildRegular>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::rosterBuildTournament>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::rosterBuildKnockout>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::rosterBuildHotPursuit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::rosterSeedByte>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

template <>
struct ByAddress<frontend::loada::rosterBuild>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::raceCarSteps>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// car in eax, a in edx, b in ebx, c in ecx, d, time and e on the stack.
template <>
struct ByAddress<frontend::loada::carAddEvent>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 a, x86::reg32 b,
                           x86::reg32 c, x86::reg32 d, x86::reg32 time, x86::reg32 e)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, a, b, c }, { d, time, e }));
    }
};

// car in eax.
template <>
struct ByAddress<frontend::loada::carClearDriving>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// car in eax.
template <>
struct ByAddress<frontend::loada::carJoinLists>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// car in eax.
template <>
struct ByAddress<frontend::loada::carReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// car in eax, slice in edx, across, up and right on the stack.
template <>
struct ByAddress<frontend::loada::carPlaceIsClear>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 slice,
                           x86::reg32 across, x86::reg32 up, x86::reg32 right)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, slice }, { across, up, right }));
    }
};

// car in eax, wanted in edx.
template <>
struct ByAddress<frontend::loada::carPlaceOnRoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, wanted }));
    }
};

// next in eax.
template <>
struct ByAddress<frontend::loada::carBefore>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 next)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ next }));
    }
};

// number in eax, which in edx.
template <>
struct ByAddress<frontend::loada::carStat>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 number, x86::reg32 which)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ number, which }));
    }
};

// number in eax.
template <>
struct ByAddress<frontend::loada::carExtras>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 number)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ number }));
    }
};

// id in eax.
template <>
struct ByAddress<frontend::loada::carIdIsPlain>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

// number in eax.
template <>
struct ByAddress<frontend::loada::carIsKept>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 number)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ number }));
    }
};

// number in eax.
template <>
struct ByAddress<frontend::loada::carFlag2>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 number)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ number }));
    }
};

// number in eax.
template <>
struct ByAddress<frontend::loada::carByte3a0>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 number)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ number }));
    }
};

// index in eax.
template <>
struct ByAddress<frontend::loada::playerToListedCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

template <>
struct ByAddress<frontend::loada::freeCars>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// car in eax.
template <>
struct ByAddress<frontend::loada::carFreeData>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// holder in eax.
template <>
struct ByAddress<frontend::loada::carFreeBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 holder)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ holder }));
    }
};

// a in eax, b in edx.
template <>
struct ByAddress<frontend::loada::carCompareNames>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// index in eax, wantedClass in edx.
template <>
struct ByAddress<frontend::loada::carInClass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index, x86::reg32 wantedClass)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index, wantedClass }));
    }
};

// flags in eax, pick in edx.
template <>
struct ByAddress<frontend::loada::carFilterLists>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 flags, x86::reg32 pick)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ flags, pick }));
    }
};

// car in eax.
template <>
struct ByAddress<frontend::loada::carRemoveFolder>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loada::removeKeptCars>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// wanted in eax.
template <>
struct ByAddress<frontend::loada::diskHasRoom>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ wanted }));
    }
};

// to in eax, from in edx.
template <>
struct ByAddress<frontend::loada::copyFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 from)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ to, from }));
    }
};

// saved in eax.
template <>
struct ByAddress<frontend::loada::savedResolvePlayers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 saved)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ saved }));
    }
};

// saved in eax.
template <>
struct ByAddress<frontend::loada::savedResolveEntries>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 saved)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ saved }));
    }
};

// saved in eax.
template <>
struct ByAddress<frontend::loada::savedNameEntries>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 saved)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ saved }));
    }
};

template <>
struct ByAddress<frontend::loada::readReplayKeepingSettings>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// list in eax, keyA in edx, keyB in ebx.
template <>
struct ByAddress<frontend::loada::listRemove>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 keyA,
                           x86::reg32 keyB)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list, keyA, keyB }));
    }
};

// player in eax, id in edx.
template <>
struct ByAddress<frontend::loada::tuningOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player, id }));
    }
};

template <>
struct ByAddress<frontend::loada::tuningForPlayers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::screenSizeChosen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::screenSizeNumber>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// size in eax.
template <>
struct ByAddress<frontend::loada::screenSizeNumberOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ size }));
    }
};

template <>
struct ByAddress<frontend::loada::horizonStartCall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::horizonFreeCall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// view in eax, out in esi.
template <>
struct ByAddress<frontend::loada::skyViewTo>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// shape in eax, width in edx, height in ebx.
template <>
struct ByAddress<frontend::loada::shapeSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape, x86::reg32 width,
                           x86::reg32 height)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape, width, height }));
    }
};

// pieces in eax.
template <>
struct ByAddress<frontend::loada::horizonSetPieces>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pieces)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pieces }));
    }
};

template <>
struct ByAddress<frontend::loada::horizonMeasure>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::horizonStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::horizonFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::stopCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::copKeepLight>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::copLoadModel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::raceSteps>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loada::raceResetSteps>
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
