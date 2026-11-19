#ifndef NFS3HP_NATIVE_LOAD_CARDB_H
#define NFS3HP_NATIVE_LOAD_CARDB_H

#include "native_guest.h"

/* native_load_cardb.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace load
{

// The game's 0x4385a0: the first "P%03d" folder not there in the carmodel folder, its name to `name`.
bool freePlayerFolder(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// The game's 0x438ad0: the id of the number `number` into the 16 bytes at `id`.
bool makeId(nfs::App* app, nfs::ThreadState& ts, nfs::u32 id, nfs::u32 number);
// The game's 0x439250: the address of the car's name.
std::optional<nfs::u32> nameOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4392b0: the address of the car's folder's name, or 0.
std::optional<nfs::u32> folderOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439300: the address of the prefix of the car's files.
std::optional<nfs::u32> prefixOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439520: the address of the car's id.
std::optional<nfs::u32> idOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439660: the number of the id at `id`, -1 if it is not one.
nfs::u32 numberOf(nfs::App* app, nfs::u32 id);
// The game's 0x4396b0: the car's police flag (0x10 of its flags).
std::optional<nfs::u32> policeFlag(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4396e0: the car's Ferrari-or-Mercedes flag (0x20 of its flags).
std::optional<nfs::u32> makerFlag(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439990: the place in the list of the car whose id is the 16 bytes at `id`, -1 if none.
nfs::u32 placeById(nfs::App* app, nfs::u32 id);
// The game's 0x439a50: the record of the car at `place` in the list, 0 for none.
nfs::u32 recordAt(nfs::App* app, nfs::u32 place);
// The game's 0x439d90: the cars' list loaded, sorted and checked; 0, or -1 where memory fails.
std::optional<nfs::u32> loadCars(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x43baa0: the carp.txt beside the .viv at `vivPath` appended to it; fclose's answer.
std::optional<nfs::u32> appendCarp(nfs::App* app, nfs::ThreadState& ts, nfs::u32 vivPath);
// The game's 0x43bc00: the 16 bytes at `id` made the last car loaded's id; the count copied, or 0.
nfs::u32 setLastId(nfs::App* app, nfs::u32 id);
// The game's 0x43bc40: the last car loaded's name printed with `format`: sprintf's answer, or 0.
std::optional<nfs::u32> nameLastCar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 format);
// The game's 0x43bc70: `flags` made the last car loaded's flags; the car.
nfs::u32 setLastFlags(nfs::App* app, nfs::u32 flags);
// The game's 0x43bc90: the last car loaded given flag 8 (after the others); the car.
nfs::u32 markLastAfterOthers(nfs::App* app);

// The game's 0x438340: the path at `path` ended with a backslash; false when the game stopped.
bool slashAdded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
// The game's 0x4389d0: the folder at `folder` emptied and removed; rmdir's answer.
std::optional<nfs::u32> folderRemoved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 folder);
// The game's 0x4386b0: the name of the first folder in `folder`, or 0.
std::optional<nfs::u32> firstSubfolderIn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 folder);
// The game's 0x4387e0: the next folder of that search, or 0.
std::optional<nfs::u32> nextSubfolderFound(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x438870: the name of the first file in `folder`, or 0.
std::optional<nfs::u32> firstFileIn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 folder);
// The game's 0x438970: the next file of that search, or 0.
std::optional<nfs::u32> nextFileFound(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x438af0: the first car of the cars' list, or 0.
nfs::u32 firstListedCar(nfs::App* app);
// The game's 0x438b10: the next car of that walk, or 0.
nfs::u32 nextListedCar(nfs::App* app);
// The game's 0x438bd0: how many cars the list holds.
nfs::u32 carsListed(nfs::App* app);
// The game's 0x439730: the class of the car at `car` (0, 1 or 2).
std::optional<nfs::u32> classOfCar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439610: whether the car's id is one of the game's own (1 or 0).
std::optional<nfs::u32> numericIdOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439570: the car's flag 0x40.
std::optional<nfs::u32> flag40Of(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4395c0: the car's flag 4 (a folder of its own).
std::optional<nfs::u32> flag4Of(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x439680: the car's bonus flag.
std::optional<nfs::u32> bonusFlagOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x43a580: the cars' class lists built; the count.
std::optional<nfs::u32> classListsBuilt(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x439c10: whether the bonus car's record at `record` is locked.
nfs::u32 bonusLocked(nfs::App* app, nfs::u32 record);
// The game's 0x439af0: the text `index` of the texts at `texts`, 0 past the count.
nfs::u32 fecTextOf(nfs::App* app, nfs::u32 texts, nfs::u32 index);
// The game's 0x43b680: the car's .fec path into `path`; what the code leaves in eax.
std::optional<nfs::u32> fecPathOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 path);
// The game's 0x43b4f0: the record's .FEC written to the file at `path`.
std::optional<nfs::u32> fecWritten(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 record);
// The game's 0x43ba20: the placeholder car's car.viv path.
nfs::u32 placeholderViv(nfs::App* app);

}  // namespace load
}  // namespace nfs3hp

#endif
