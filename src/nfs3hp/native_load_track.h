#ifndef NFS3HP_NATIVE_LOAD_TRACK_H
#define NFS3HP_NATIVE_LOAD_TRACK_H

#include "native_guest.h"

/* native_load_track.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace load
{

/* What packCache leaves: the buffer (or 0), and what its code leaves in edx, which no caller
 * reads: where the writing stopped (writeBig's file), the buffer's size, the header's size or
 * the size's place. */
struct Packed
{
    x86::reg32 buffer, left;
};

// The game's 0x40e000: the opponents' speeds loaded; the second half's index, or 0.
std::optional<x86::reg32> loadSpeeds(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x40f230: the track's cameras cleared.
bool clearCameras(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x40f280: the track's cameras loaded, their files' path starting `path`; how many.
std::optional<x86::reg32> loadCameras(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path);
// The game's 0x418fe0: the node of the file named `name` in the cache, or 0.
std::optional<x86::reg32> findCached(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cache, x86::reg32 name);
// The game's 0x419460: the cache written as a BIGF archive into a block of its own, its size to
// [sizeTo].
std::optional<Packed> packCache(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cache, x86::reg32 sizeTo);
// packCache's buffer (or 0).
inline std::optional<x86::reg32> packedCache(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cache,
                                             x86::reg32 sizeTo)
{
    const std::optional<Packed> packed = packCache(app, cpu, cache, sizeTo);
    if (!packed)
        return std::nullopt;
    return packed->buffer;
}
// The game's 0x4195f0: a cache made of the BIGF archive at `data`, or 0.
std::optional<x86::reg32> openCache(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data);
// The game's 0x41d8a0: the path of the track's file named by its end `end`.
std::optional<x86::reg32> trackFile(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 end);
// The game's 0x477ac0: the ghost written to the stream `stream` at `offset`.
bool writeGhostTo(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 offset);
// The game's 0x477b00: the ghost read from the handle `handle`.
bool readGhostFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);

}  // namespace load

namespace guest
{

// Eax the answer.
template <>
struct ByAddress<load::loadSpeeds>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<load::clearCameras>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// Those of one argument in eax, eax the answer.
struct TrackOneArgument
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 argument)
    {
        return watcomEax(app, cpu, Address, {argument});
    }
};

template <>
struct ByAddress<load::loadCameras> : TrackOneArgument
{
};

template <>
struct ByAddress<load::openCache> : TrackOneArgument
{
};

template <>
struct ByAddress<load::trackFile> : TrackOneArgument
{
};

// The cache in eax, the name in edx; eax the node.
template <>
struct ByAddress<load::findCached>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cache,
                                          x86::reg32 name)
    {
        return watcomEax(app, cpu, Address, {cache, name});
    }
};

// The cache in eax, the size's address in edx; eax the buffer.
template <>
struct ByAddress<load::packedCache>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cache,
                                          x86::reg32 sizeTo)
    {
        return watcomEax(app, cpu, Address, {cache, sizeTo});
    }
};

// The stream in eax, the offset in edx.
template <>
struct ByAddress<load::writeGhostTo>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 offset)
    {
        return watcom(app, cpu, Address, {stream, offset});
    }
};

// The handle in eax.
template <>
struct ByAddress<load::readGhostFrom>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return watcom(app, cpu, Address, {handle});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
