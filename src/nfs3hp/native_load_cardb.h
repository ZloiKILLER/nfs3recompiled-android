#ifndef NFS3HP_NATIVE_LOAD_CARDB_H
#define NFS3HP_NATIVE_LOAD_CARDB_H

#include "native_guest.h"

/* native_load_cardb.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace load
{

// The game's 0x4385a0: the first "P%03d" folder not there in the carmodel folder, its name to `name`.
bool freePlayerFolder(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x438ad0: the id of the number `number` into the 16 bytes at `id`.
bool makeId(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id, x86::reg32 number);
// The game's 0x439250: the address of the car's name.
std::optional<x86::reg32> nameOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4392b0: the address of the car's folder's name, or 0.
std::optional<x86::reg32> folderOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439300: the address of the prefix of the car's files.
std::optional<x86::reg32> prefixOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439520: the address of the car's id.
std::optional<x86::reg32> idOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439660: the number of the id at `id`, -1 if it is not one.
x86::reg32 numberOf(win32::WinApplication* app, x86::reg32 id);
// The game's 0x4396b0: the car's police flag (0x10 of its flags).
std::optional<x86::reg32> policeFlag(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4396e0: the car's Ferrari-or-Mercedes flag (0x20 of its flags).
std::optional<x86::reg32> makerFlag(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439990: the place in the list of the car whose id is the 16 bytes at `id`, -1 if none.
x86::reg32 placeById(win32::WinApplication* app, x86::reg32 id);
// The game's 0x439a50: the record of the car at `place` in the list, 0 for none.
x86::reg32 recordAt(win32::WinApplication* app, x86::reg32 place);
// The game's 0x439d90: the cars' list loaded, sorted and checked; 0, or -1 where memory fails.
std::optional<x86::reg32> loadCars(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x43baa0: the carp.txt beside the .viv at `vivPath` appended to it; fclose's answer.
std::optional<x86::reg32> appendCarp(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vivPath);
// The game's 0x43bc00: the 16 bytes at `id` made the last car loaded's id; the count copied, or 0.
x86::reg32 setLastId(win32::WinApplication* app, x86::reg32 id);
// The game's 0x43bc40: the last car loaded's name printed with `format`: sprintf's answer, or 0.
std::optional<x86::reg32> nameLastCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format);
// The game's 0x43bc70: `flags` made the last car loaded's flags; the car.
x86::reg32 setLastFlags(win32::WinApplication* app, x86::reg32 flags);
// The game's 0x43bc90: the last car loaded given flag 8 (after the others); the car.
x86::reg32 markLastAfterOthers(win32::WinApplication* app);

// The game's 0x438340: the path at `path` ended with a backslash; false when the game stopped.
bool slashAdded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path);
// The game's 0x4389d0: the folder at `folder` emptied and removed; rmdir's answer.
std::optional<x86::reg32> folderRemoved(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 folder);
// The game's 0x4386b0: the name of the first folder in `folder`, or 0.
std::optional<x86::reg32> firstSubfolderIn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 folder);
// The game's 0x4387e0: the next folder of that search, or 0.
std::optional<x86::reg32> nextSubfolderFound(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x438870: the name of the first file in `folder`, or 0.
std::optional<x86::reg32> firstFileIn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 folder);
// The game's 0x438970: the next file of that search, or 0.
std::optional<x86::reg32> nextFileFound(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x438af0: the first car of the cars' list, or 0.
x86::reg32 firstListedCar(win32::WinApplication* app);
// The game's 0x438b10: the next car of that walk, or 0.
x86::reg32 nextListedCar(win32::WinApplication* app);
// The game's 0x438bd0: how many cars the list holds.
x86::reg32 carsListed(win32::WinApplication* app);
// The game's 0x439730: the class of the car at `car` (0, 1 or 2).
std::optional<x86::reg32> classOfCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439610: whether the car's id is one of the game's own (1 or 0).
std::optional<x86::reg32> numericIdOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439570: the car's flag 0x40.
std::optional<x86::reg32> flag40Of(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4395c0: the car's flag 4 (a folder of its own).
std::optional<x86::reg32> flag4Of(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x439680: the car's bonus flag.
std::optional<x86::reg32> bonusFlagOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x43a580: the cars' class lists built; the count.
std::optional<x86::reg32> classListsBuilt(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x439c10: whether the bonus car's record at `record` is locked.
x86::reg32 bonusLocked(win32::WinApplication* app, x86::reg32 record);
// The game's 0x439af0: the text `index` of the texts at `texts`, 0 past the count.
x86::reg32 fecTextOf(win32::WinApplication* app, x86::reg32 texts, x86::reg32 index);
// The game's 0x43b680: the car's .fec path into `path`; what the code leaves in eax.
std::optional<x86::reg32> fecPathOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 path);
// The game's 0x43b4f0: the record's .FEC written to the file at `path`.
std::optional<x86::reg32> fecWritten(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 record);
// The game's 0x43ba20: the placeholder car's car.viv path.
x86::reg32 placeholderViv(win32::WinApplication* app);

}  // namespace load
}  // namespace nfs3hp

#endif
