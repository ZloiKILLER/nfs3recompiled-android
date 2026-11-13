#ifndef NFS3HP_NATIVE_LOAD_SAVES_H
#define NFS3HP_NATIVE_LOAD_SAVES_H

#include "native_guest.h"

/* native_load_saves.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace load
{

/* What findBigEntry finds, and where it stops, as its code leaves ebx and
 * ecx: the entry it was at (the index asked for where it walked none), and
 * the archive's end, the last name's length walked past, the offset's
 * pointer written through, or the shift of the size's big-endian read. */
struct BigEntry
{
    x86::reg32 name;  // the entry's name's address, 0 for none
    x86::reg32 entry;
    x86::reg32 last;
};

// The game's 0x472940: 1 if the file named `name` was written before the one named `other`, else 0.
std::optional<x86::reg32> writtenBefore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                        x86::reg32 other);
// The game's 0x4ea040, EA's findbigentry: the entry of the BIG archive named `name`, or with no
// name the index'th; its offset to [offsetTo] and its size to [sizeTo].
std::optional<BigEntry> findEntry(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 archive, x86::reg32 name,
                                  x86::reg32 index, x86::reg32 offsetTo, x86::reg32 sizeTo);
// The game's 0x4ea290, EA's locatebigentry: findEntry's, and an Error for an entry not found.
std::optional<BigEntry> locateEntry(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 archive,
                                    x86::reg32 name, x86::reg32 index, x86::reg32 offsetTo, x86::reg32 sizeTo);
// locateEntry's name's address (0 for none): what its code leaves in eax, and in ecx.
inline std::optional<x86::reg32> locatedEntry(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 archive,
                                              x86::reg32 name, x86::reg32 index, x86::reg32 offsetTo,
                                              x86::reg32 sizeTo)
{
    const std::optional<BigEntry> found = locateEntry(app, cpu, archive, name, index, offsetTo, sizeTo);
    if (!found)
        return std::nullopt;
    return found->name;
}
// The game's 0x4eebb0: savefileadr with its Errors off; how many bytes it wrote.
std::optional<x86::reg32> saveBlockQuietly(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                           x86::reg32 bytes, x86::reg32 count);
// The game's 0x4f2380: the sine of the angle (a turn 65536), 16.16.
x86::reg32 sineOf(win32::WinApplication* app, x86::reg32 angle);
// The game's 0x4f23f0: the cosine of the angle, as sineOf.
x86::reg32 cosineOf(win32::WinApplication* app, x86::reg32 angle);

}  // namespace load

namespace guest
{

// The names in eax and edx; eax the answer.
template <>
struct ByAddress<load::writtenBefore>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                          x86::reg32 other)
    {
        return watcomEax(app, cpu, Address, {name, other});
    }
};

// The archive in eax, the name in edx, the index in ebx, the offset's address in ecx, the size's
// on the stack (ret 4); the entry's name back in eax, where it stopped in ebx and ecx.
template <>
struct ByAddress<load::findEntry>
{
    template <x86::reg32 Address>
    static std::optional<load::BigEntry> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 archive,
                                              x86::reg32 name, x86::reg32 index, x86::reg32 offsetTo,
                                              x86::reg32 sizeTo)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = archive;
        cpu.edx = name;
        cpu.ebx = index;
        cpu.ecx = offsetTo;
        if (!guest::call(app, cpu, Address, {sizeTo}))
            return std::nullopt;
        return load::BigEntry{cpu.eax, cpu.ebx, cpu.ecx};
    }
};

// As findEntry's; eax the entry's name.
template <>
struct ByAddress<load::locatedEntry>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 archive,
                                          x86::reg32 name, x86::reg32 index, x86::reg32 offsetTo,
                                          x86::reg32 sizeTo)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = archive;
        cpu.edx = name;
        cpu.ebx = index;
        cpu.ecx = offsetTo;
        if (!guest::call(app, cpu, Address, {sizeTo}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The name in eax, the bytes in edx, the count in ebx; eax the answer.
template <>
struct ByAddress<load::saveBlockQuietly>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                          x86::reg32 bytes, x86::reg32 count)
    {
        return watcomEax(app, cpu, Address, {name, bytes, count});
    }
};

// The angle in eax; eax the answer.
struct SavesAngle
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 angle)
    {
        watcom(app, cpu, Address, {angle});
        return cpu.eax;
    }
};

template <>
struct ByAddress<load::sineOf> : SavesAngle
{
};

template <>
struct ByAddress<load::cosineOf> : SavesAngle
{
};

}  // namespace guest
}  // namespace nfs3hp

#endif
