#ifndef NFS3HP_NATIVE_EA_FILES_H
#define NFS3HP_NATIVE_EA_FILES_H

#include "native_guest.h"

/* native_ea_files.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
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
// The game's 0x4e3330: the drive of the path at `path` by its number, 1 for A:; 0 for none.
std::optional<x86::reg32> driveNumberOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path);
// The game's 0x501050, cdecl: a report at `level` -- nothing when above the threshold at `threshold`
// (0: none) -- of `format` with `words`, sent the way the game set: what the way leaves in eax.
std::optional<x86::reg32> reportedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 threshold,
                                     x86::reg32 level, x86::reg32 format, guest::Words words);

// The game's 0x4e2ab0: openToRead with [0x56440c] (an Error if it fails) set to `errorsAs` for the call.
std::optional<x86::reg32> errorsAsOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                         x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut,
                                         x86::reg32 errorsAs);
// The game's 0x4e3620: the file named `name` opened as `flags` say (block `block`), its handle to [handleOut].
std::optional<x86::reg32> fileHandleOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                           x86::reg32 flags, x86::reg32 block, x86::reg32 handleOut);
// The game's 0x4e3e60: handle `handle` sought to `to`: 1 if it was, else 0.
std::optional<x86::reg32> handleSought(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 to);
// The game's 0x4e3ae0: `count` bytes of handle `handle` read to `buffer`: how many.
std::optional<x86::reg32> bytesRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer,
                                    x86::reg32 count);
// The game's 0x4e3cc0: `count` bytes at `buffer` written to handle `handle`: how many.
std::optional<x86::reg32> bytesWritten(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                       x86::reg32 buffer, x86::reg32 count);
// The game's 0x4e3980: what handle `handle` is: how it was opened to [howOut], its block to [blockOut], its
// length to [lengthOut], the free bytes to [freeOut] (each pointer may be 0).
std::optional<x86::reg32> handleInfoOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                       x86::reg32 howOut, x86::reg32 blockOut, x86::reg32 lengthOut,
                                       x86::reg32 freeOut);
// The game's 0x4e41f0: where handle `handle` is in memory (mapped), or 0.
x86::reg32 handleMemoryOf(win32::WinApplication* app, x86::reg32 handle);
// The game's 0x4e3600: the file named `name` deleted: DeleteFile's answer.
std::optional<x86::reg32> fileRemoved(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x4e3430: the handles' table made anew for `count` handles.
std::optional<x86::reg32> handlesMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count);
// The game's 0x5011a0, EA's touchmempages: the pages of `bytes` from `start` gone over: `start`, or what
// its loops leave in eax.
std::optional<x86::reg32> memoryTouched(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 start,
                                        x86::reg32 bytes);
// The game's 0x501270, EA's openblockhandle: the file `name` opened to read, its handle to [handleOut],
// where it starts to [startOut], its length to [lengthOut], its block to [blockOut]: 1 if opened, else 0.
std::optional<x86::reg32> blockOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name,
                                      x86::reg32 handleOut, x86::reg32 startOut, x86::reg32 lengthOut,
                                      x86::reg32 blockOut);
// The game's 0x4e3fe0 (and 0x5013a0, EA's closeblockhandle, which jumps to it): handle `handle`'s slot closed.
std::optional<x86::reg32> slotClosed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);
// The game's 0x5013b0, EA's readblockhandle: `blocks` blocks of `handle` read to `buffer`: the bytes read.
std::optional<x86::reg32> blocksRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle,
                                     x86::reg32 buffer, x86::reg32 blocks);
// The game's 0x501450, EA's seekblockhandle: `handle` sought to block `to`: where it is then.
std::optional<x86::reg32> blocksSought(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 to);

// The game's 0x4e40a0: handle `handle`'s drive's lock entered: 1, or 0 if it is not open.
std::optional<x86::reg32> handleDriveLocked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);
// The game's 0x4e40f0: handle `handle`'s drive's lock left: 1, or 0 if it is not open.
std::optional<x86::reg32> handleDriveUnlocked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);
// The game's 0x4e4140: `length` bytes at `memory` taken as a file on drive `drive`, read and written by blocks of
// `block`; its handle to [handleOut]: 1 if a slot was free, else 0.
std::optional<x86::reg32> memoryOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 memory, x86::reg32 length,
                                       x86::reg32 drive, x86::reg32 block, x86::reg32 handleOut);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
