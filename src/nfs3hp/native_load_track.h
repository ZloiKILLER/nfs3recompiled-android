#ifndef NFS3HP_NATIVE_LOAD_TRACK_H
#define NFS3HP_NATIVE_LOAD_TRACK_H

#include "native_guest.h"

/* native_load_track.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
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

// The game's 0x41dbc0: the track's textures loaded; false when the game stopped.
bool trackTexturesLoaded(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x477a90: the race's ghost saved; false when the game stopped.
bool ghostSaved(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x478340: the horizon of the track `track` read; what the code leaves in eax.
std::optional<x86::reg32> horizonRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 track);
// The game's 0x4dcd20: the weather's shapes loaded; false when the game stopped.
bool weatherLoaded(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace load
}  // namespace nfs3hp

#endif
