#ifndef NFS3HP_NATIVE_EA_FORMATS_H
#define NFS3HP_NATIVE_EA_FORMATS_H

#include "native_guest.h"

/* native_ea_formats.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4ef270: the shape named by the four characters at `name` in the shape file at `file`, or 0.
x86::reg32 shapeLocated(win32::WinApplication* app, x86::reg32 file, x86::reg32 name);
// The game's 0x4f0010: the character of scan code `code` as the keys held say (`callerEcx` the ecx the code hands it).
std::optional<x86::reg32> scanCodeCharacter(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 code,
                                            x86::reg32 callerEcx);
// The game's 0x4f0100: the key hook's next key, 0 for none.
std::optional<x86::reg32> hookedKey(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4f0240: the menus' keys taken until there are none: 0.
std::optional<x86::reg32> menuKeysFlushed(win32::WinApplication* app, x86::CPU& cpu);
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

// The game's 0x4ef270: the shape named `name` in the file at `file`: it, or 0.
x86::reg32 shapeLocated(win32::WinApplication* app, x86::reg32 file, x86::reg32 name);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
