#ifndef NFS3HP_NATIVE_EA_FORMATS_H
#define NFS3HP_NATIVE_EA_FORMATS_H

#include "native_guest.h"

/* native_ea_formats.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4ef270: the shape named by the four characters at `name` in the shape file at `file`, or 0.
nfs::u32 shapeLocated(nfs::App* app, nfs::u32 file, nfs::u32 name);
// The game's 0x4f0010: the character of scan code `code` as the keys held say (`callerEcx` the ecx the code hands it).
std::optional<nfs::u32> scanCodeCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::u32 code,
                                            nfs::u32 callerEcx);
// The game's 0x4f0100: the key hook's next key, 0 for none.
std::optional<nfs::u32> hookedKey(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4f0240: the menus' keys taken until there are none: 0.
std::optional<nfs::u32> menuKeysFlushed(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4f0120, inkey: the game's tasks run until the key hook gives a key: the key.
std::optional<nfs::u32> waitKey(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4f2620, unpack: the packed data at `data` unpacked to `to`, an Error for a code not
// known: the size unpacked, or 0.
std::optional<nfs::u32> unpackChecked(nfs::App* app, nfs::ThreadState& ts, nfs::u32 data, nfs::u32 to);
// The game's 0x4f2710, unpacksize: the size the packed data at `data` unpacks to, an Error for a code
// not known.
std::optional<nfs::u32> packedSizeChecked(nfs::App* app, nfs::ThreadState& ts, nfs::u32 data);
// The game's 0x4f29b0: the secondary timer's `callback` removed; eax as the code leaves it.
std::optional<nfs::u32> removeTimer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 callback);

// The game's 0x4ef270: the shape named `name` in the file at `file`: it, or 0.
nfs::u32 shapeLocated(nfs::App* app, nfs::u32 file, nfs::u32 name);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
