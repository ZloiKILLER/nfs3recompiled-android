#ifndef NFS3HP_NATIVE_LOAD_CARDB_H
#define NFS3HP_NATIVE_LOAD_CARDB_H

#include "native_guest.h"

/* native_load_cardb.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
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

}  // namespace load

namespace guest
{

// The name's buffer in eax.
template <>
struct ByAddress<load::freePlayerFolder>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return watcom(app, cpu, Address, {name});
    }
};

// The id's 16 bytes in eax, the number in edx.
template <>
struct ByAddress<load::makeId>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id, x86::reg32 number)
    {
        return watcom(app, cpu, Address, {id, number});
    }
};

// The car's lookups: the car in eax; eax the answer.
struct CardbCarLookup
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<load::nameOf> : CardbCarLookup
{
};

template <>
struct ByAddress<load::folderOf> : CardbCarLookup
{
};

template <>
struct ByAddress<load::prefixOf> : CardbCarLookup
{
};

template <>
struct ByAddress<load::idOf> : CardbCarLookup
{
};

template <>
struct ByAddress<load::policeFlag> : CardbCarLookup
{
};

template <>
struct ByAddress<load::makerFlag> : CardbCarLookup
{
};

// Those that call nothing: their one argument in eax, eax the answer.
struct CardbPlainCall
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 argument)
    {
        watcom(app, cpu, Address, {argument});
        return cpu.eax;
    }
};

template <>
struct ByAddress<load::numberOf> : CardbPlainCall
{
};

template <>
struct ByAddress<load::placeById> : CardbPlainCall
{
};

template <>
struct ByAddress<load::recordAt> : CardbPlainCall
{
};

template <>
struct ByAddress<load::setLastId> : CardbPlainCall
{
};

template <>
struct ByAddress<load::setLastFlags> : CardbPlainCall
{
};

// Eax the car.
template <>
struct ByAddress<load::markLastAfterOthers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// Eax the answer.
template <>
struct ByAddress<load::loadCars>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The .viv's path in eax; eax the answer.
template <>
struct ByAddress<load::appendCarp>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vivPath)
    {
        return watcomEax(app, cpu, Address, {vivPath});
    }
};

// The format in eax; eax the answer.
template <>
struct ByAddress<load::nameLastCar>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format)
    {
        return watcomEax(app, cpu, Address, {format});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
