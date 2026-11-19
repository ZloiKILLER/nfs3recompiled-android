#ifndef NFS3HP_NATIVE_LOAD_SAVES_H
#define NFS3HP_NATIVE_LOAD_SAVES_H

#include "native_guest.h"

/* native_load_saves.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in
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
    nfs::u32 name;  // the entry's name's address, 0 for none
    nfs::u32 entry;
    nfs::u32 last;
};

// The game's 0x472940: 1 if the file named `name` was written before the one named `other`, else 0.
std::optional<nfs::u32> writtenBefore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name,
                                        nfs::u32 other);
// The game's 0x4ea040, EA's findbigentry: the entry of the BIG archive named `name`, or with no
// name the index'th; its offset to [offsetTo] and its size to [sizeTo].
std::optional<BigEntry> findEntry(nfs::App* app, nfs::ThreadState& ts, nfs::u32 archive, nfs::u32 name,
                                  nfs::u32 index, nfs::u32 offsetTo, nfs::u32 sizeTo);
// The game's 0x4ea290, EA's locatebigentry: findEntry's, and an Error for an entry not found.
std::optional<BigEntry> locateEntry(nfs::App* app, nfs::ThreadState& ts, nfs::u32 archive,
                                    nfs::u32 name, nfs::u32 index, nfs::u32 offsetTo, nfs::u32 sizeTo);
// locateEntry's name's address (0 for none): what its code leaves in eax, and in ecx.
inline std::optional<nfs::u32> locatedEntry(nfs::App* app, nfs::ThreadState& ts, nfs::u32 archive,
                                              nfs::u32 name, nfs::u32 index, nfs::u32 offsetTo,
                                              nfs::u32 sizeTo)
{
    const std::optional<BigEntry> found = locateEntry(app, ts, archive, name, index, offsetTo, sizeTo);
    if (!found)
        return std::nullopt;
    return found->name;
}
// The game's 0x4eebb0: savefileadr with its Errors off; how many bytes it wrote.
std::optional<nfs::u32> saveBlockQuietly(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name,
                                           nfs::u32 bytes, nfs::u32 count);
// The game's 0x4f2380: the sine of the angle (a turn 65536), 16.16.
nfs::u32 sineOf(nfs::App* app, nfs::u32 angle);
// The game's 0x4f23f0: the cosine of the angle, as sineOf.
nfs::u32 cosineOf(nfs::App* app, nfs::u32 angle);

// The game's 0x44f510: the records' table loaded if it is not; false when the game stopped.
bool recordsTableLoaded(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x44f5f0: the records' table saved and freed; what the code leaves in eax.
std::optional<nfs::u32> recordsTableSaved(nfs::App* app, nfs::ThreadState& ts);
// The same where the caller has `eax` in the register the code's result is left in when it saves nothing.
std::optional<nfs::u32> recordsTableSavedWith(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x446990: the kept replay named `name` made the race's; 1 if it was.
std::optional<nfs::u32> keptReplayLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// The game's 0x446060: the replay named `name`'s information read into the screen's lines.
std::optional<nfs::u32> replayInfoShown(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// The game's 0x4475a0: the replay files of the extension at `extension` listed into the menu's record `list`.
std::optional<nfs::u32> replaysListed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list,
                                        nfs::u32 extension);
// The game's 0x472820: config.dat saved; what the code leaves in eax.
std::optional<nfs::u32> configSaved(nfs::App* app, nfs::ThreadState& ts);
std::optional<nfs::u32> configSavedWith(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x472980: config.dat loaded; false when the game stopped.
bool configLoaded(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4bd1e0: the race's replay saved; what the code leaves in eax.
std::optional<nfs::u32> replaySaved(nfs::App* app, nfs::ThreadState& ts);
std::optional<nfs::u32> replaySavedWith(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x4bd2e0: the race's replay read; its answer.
std::optional<nfs::u32> replayRead(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x472300: the race's camera views made from the ones chosen; the last.
nfs::u32 cameraViewsSet(nfs::App* app);
// The game's 0x4e9f00, EA's typeofbigfile: the kind of the BIG archive at `archive`.
std::optional<nfs::u32> bigFileKindOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 archive);
// The game's 0x4eeae0, EA's savefileadr: `count` bytes at `bytes` written to the file named `name`; how many.
std::optional<nfs::u32> blockSaved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 bytes,
                                     nfs::u32 count);

}  // namespace load
}  // namespace nfs3hp

#endif
