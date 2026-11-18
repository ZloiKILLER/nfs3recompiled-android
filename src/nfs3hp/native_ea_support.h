#ifndef NFS3HP_NATIVE_EA_SUPPORT_H
#define NFS3HP_NATIVE_EA_SUPPORT_H

#include "native_guest.h"

/* native_ea_support.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one.  EA's
 * trace (0x4e08f0) is not one: its format's arguments follow it on the stack, as many as it asks. */
namespace nfs3hp
{
namespace ea
{

/* Where a look-up in an archive's entries ended: the entry it stopped at,
 * whether it is the one, and that entry's size's word as it read it. */
struct VivSearch
{
    x86::reg32 entry;
    bool found;
    x86::reg32 sizeWord;
};

// The game's 0x42d510: the game's own trace, compiled out: nothing.
void traceOff(win32::WinApplication* app);
// The game's 0x43b830: the archive named `name` opened, its FILE to [file]: its header's block, or 0.
std::optional<x86::reg32> vivOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 file);
// The game's 0x43b9b0: the archive's block `block` given back and the FILE at [file] closed.
std::optional<x86::reg32> vivClosed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block, x86::reg32 file);
// The game's 0x43b9d0: the file named `path` looked for in the archive's header `header`: found, its
// offset to [offsetTo] and its size to [sizeTo].
std::optional<VivSearch> vivFile(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 header, x86::reg32 path,
                                 x86::reg32 offsetTo, x86::reg32 sizeTo);
// vivFile for a caller that asks only whether it was found.
inline std::optional<bool> vivFileFound(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 header, x86::reg32 path,
                                        x86::reg32 offsetTo, x86::reg32 sizeTo)
{
    const std::optional<VivSearch> search = vivFile(app, cpu, header, path, offsetTo, sizeTo);
    if (!search)
        return std::nullopt;
    return search->found;
}
// The game's 0x4e0980: EA's trace at column `column` and row `row` (the cursor moved there, if they
// are on the page), printf's way, `format` with `words` -- while the trace's level is 2 or more,
// under the trace's lock: what the code leaves in eax (`eax` as it is handed with the trace off).
std::optional<x86::reg32> monoPrintedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32 column,
                                        x86::reg32 row, x86::reg32 format, guest::Words words);
// The game's 0x4e08f0: EA's trace, printf's way, `format` with `words`, while the trace's level is 2 or more:
// what the code leaves in eax (`eax` as it is handed with the trace off).
std::optional<x86::reg32> monoPrinted(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32 format,
                                      guest::Words words);
// The game's 0x4e0ca0: the monitor's page cleared under the trace's lock, while the trace's level is
// 2 or more: what the code leaves in eax (`eax` as it is handed with the trace off).
std::optional<x86::reg32> monoCleared(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax);
// The game's 0x4e0d50: the attribute the monitor's text is written in, `attribute`'s low byte: eax
// as it is handed (`attribute`).
x86::reg32 monoAttributeSet(win32::WinApplication* app, x86::reg32 attribute);
// The game's 0x4e0dee: the random numbers seeded from `seed`: the last word.
x86::reg32 randomSeeded(win32::WinApplication* app, x86::reg32 seed);
// The game's 0x4e0e60: whether the file named `name` is there.
std::optional<bool> fileThere(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x4e0ec0, loadfileadr: the file named `name` loaded whole, flags `flags`: the block, or
// 0.
std::optional<x86::reg32> loadWhole(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 flags);
// The game's 0x4e0fe0: loadWhole with the complaint at a missing file off: the block, or 0.
std::optional<x86::reg32> loadQuietly(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 flags);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
