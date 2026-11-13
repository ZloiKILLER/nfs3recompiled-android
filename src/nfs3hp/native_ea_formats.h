#ifndef NFS3HP_NATIVE_EA_FORMATS_H
#define NFS3HP_NATIVE_EA_FORMATS_H

#include "native_guest.h"

/* native_ea_formats.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4f0120, inkey: the game's tasks run until the key hook gives a key: the key.
std::optional<x86::reg32> waitKey(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4f2620, unpack: the packed data at `data` unpacked to `to`, an Error for a code not
// known: the size unpacked, or 0.
std::optional<x86::reg32> unpackChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data, x86::reg32 to);
// The game's 0x4f2710, unpacksize: the size the packed data at `data` unpacks to, an Error for a code
// not known.
std::optional<x86::reg32> packedSizeChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data);
// The game's 0x4f29b0: the secondary timer's `callback` removed; eax as the code leaves it.
std::optional<x86::reg32> removeTimer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 callback);

}  // namespace ea

namespace guest
{

// Nothing handed; the key back in eax.
template <>
struct ByAddress<ea::waitKey>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The data in eax, where to in edx; the size back in eax.
template <>
struct ByAddress<ea::unpackChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data, x86::reg32 to)
    {
        return watcomEax(app, cpu, Address, {data, to});
    }
};

// The data in eax; the size back in eax.
template <>
struct ByAddress<ea::packedSizeChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data)
    {
        return watcomEax(app, cpu, Address, {data});
    }
};

// The callback in eax; eax back.
template <>
struct ByAddress<ea::removeTimer>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 callback)
    {
        return watcomEax(app, cpu, Address, {callback});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
