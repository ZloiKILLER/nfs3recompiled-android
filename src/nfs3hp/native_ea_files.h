#ifndef NFS3HP_NATIVE_EA_FILES_H
#define NFS3HP_NATIVE_EA_FILES_H

#include "native_guest.h"

/* native_ea_files.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4e28d0, EA's openhandle: the file named `name` opened to read, its handle to
// [handleOut], where it starts to [startOut], its length to [lengthOut]: 1 if opened, else 0.
std::optional<x86::reg32> openToRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                     x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut);
// The game's 0x4e2ae0, EA's openhandlew: the file named `name` made anew to write, its handle to
// [handleOut], 0 to [startOut], its length to [lengthOut]: 1 if opened, else 0.
std::optional<x86::reg32> openToWrite(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                      x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut);
// The game's 0x4e2da0, EA's closehandle: handle `handle` closed: 1 if it was open.
std::optional<x86::reg32> closeChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);
// The game's 0x4e2de0, EA's readhandle: `count` bytes of handle `handle` read to `buffer`: how many.
std::optional<x86::reg32> readChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer,
                                      x86::reg32 count);
// The game's 0x4e2e20, EA's writehandle: `count` bytes at `buffer` written to handle `handle`: how
// many.
std::optional<x86::reg32> writeChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                       x86::reg32 buffer, x86::reg32 count);
// The game's 0x4e2e60, EA's seekhandle: handle `handle` sought to `to`: where it is then, or 0.
std::optional<x86::reg32> seekChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 to);
// The game's 0x4e33a0, EA's getdirectory: the current directory to `buffer`: its length, or an
// Error's answer.
std::optional<x86::reg32> currentDirectory(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer);
// The game's 0x501010: 0 -- after an Error naming function `name` -- on the main thread with the
// check on and a surface locked; else 1.
std::optional<x86::reg32> surfaceUnlocked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x501110: Windows' pages for the bytes at `bytes` (rounded up there): their address, or
// 0.
std::optional<x86::reg32> allocatePages(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bytes);
// The game's 0x501180: Windows' pages at `pages` given back: VirtualFree's answer.
std::optional<x86::reg32> pagesFreed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pages);
// The game's 0x501050, cdecl: a report at `level` -- nothing when above the threshold at `threshold`
// (0: none) -- of `format` with `words`, sent the way the game set: what the way leaves in eax.
std::optional<x86::reg32> reportedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 threshold,
                                     x86::reg32 level, x86::reg32 format, guest::Words words);

}  // namespace ea

namespace guest
{

// The name in eax, the three addresses in edx, ebx and ecx; eax back.
template <>
struct ByAddress<ea::openToRead>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                          x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut)
    {
        return watcomEax(app, cpu, Address, {name, handleOut, startOut, lengthOut});
    }
};

// The name in eax, the three addresses in edx, ebx and ecx; eax back.
template <>
struct ByAddress<ea::openToWrite>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                          x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut)
    {
        return watcomEax(app, cpu, Address, {name, handleOut, startOut, lengthOut});
    }
};

// The handle in eax; eax back.
template <>
struct ByAddress<ea::closeChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return watcomEax(app, cpu, Address, {handle});
    }
};

// The handle in eax, the buffer in edx, the count in ebx; eax back.
template <>
struct ByAddress<ea::readChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                          x86::reg32 buffer, x86::reg32 count)
    {
        return watcomEax(app, cpu, Address, {handle, buffer, count});
    }
};

// The handle in eax, the buffer in edx, the count in ebx; eax back.
template <>
struct ByAddress<ea::writeChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                          x86::reg32 buffer, x86::reg32 count)
    {
        return watcomEax(app, cpu, Address, {handle, buffer, count});
    }
};

// The handle in eax, where to in edx; eax back.
template <>
struct ByAddress<ea::seekChecked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 to)
    {
        return watcomEax(app, cpu, Address, {handle, to});
    }
};

// The buffer in eax; eax back.
template <>
struct ByAddress<ea::currentDirectory>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer)
    {
        return watcomEax(app, cpu, Address, {buffer});
    }
};

// The function's name in eax; eax back.
template <>
struct ByAddress<ea::surfaceUnlocked>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return watcomEax(app, cpu, Address, {name});
    }
};

// The bytes' address in eax; the pages back in eax.
template <>
struct ByAddress<ea::allocatePages>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bytes)
    {
        return watcomEax(app, cpu, Address, {bytes});
    }
};

// The pages in eax; VirtualFree's answer back in eax, every other register kept.
template <>
struct ByAddress<ea::pagesFreed>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pages)
    {
        return watcomEax(app, cpu, Address, {pages});
    }
};

// The threshold, the level, the format and the words pushed (the caller takes them off); eax back.
template <>
struct ByAddress<ea::reportedAt>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 threshold,
                                          x86::reg32 level, x86::reg32 format, Words words)
    {
        if (!guest::call(app, cpu, Address, {threshold, level, format}, words))
            return std::nullopt;
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
