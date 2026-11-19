#ifndef NFS3HP_NATIVE_TRACK_H
#define NFS3HP_NATIVE_TRACK_H

#include "native_guest.h"

/* native_track.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one;
 * one without `ts` calls nothing, and by address leaves ts.terminate to say it. */
namespace nfs3hp
{
namespace track
{

// The game's 0x419b60: the arena allocated and none of it used; its address.
std::optional<nfs::u32> startArena(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41a100: the track's geometry loaded, its colours tinted (or grey); eax as the code leaves it.
std::optional<nfs::u32> loadTrackGeometry(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41aaf0: view `index` the one drawn for the camera, its distances worked out; the camera's group.
std::optional<nfs::u32> viewOnCameraBlock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 camera,
                                            nfs::u32 index);
// The game's 0x41b8c0: the list at `list` put between two state records of the drawing's.
bool bracketOrdered(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list);
// The game's 0x41cdc0: the track's blocks of the list at `list` drawn for the camera into the buckets.
bool drawTrackBlocks(nfs::App* app, nfs::ThreadState& ts, nfs::u32 camera, nfs::u32 list,
                     nfs::u32 buckets);
// The game's 0x41d310: view `index` the one drawn; the view's offset.
nfs::u32 chooseView(nfs::App* app, nfs::u32 index);
// The game's 0x41d350: object `index` of the car's block set moving: -1 set moving, 0 moving already, 2 one
// that falls, else 1.
std::optional<nfs::u32> setObjectMoving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car,
                                          nfs::u32 index);
// setObjectMoving, under the name the game's callers take it by.
inline std::optional<nfs::u32> objectMoving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car,
                                              nfs::u32 index)
{
    return setObjectMoving(app, ts, car, index);
}
// The game's 0x41d610: View Distance set (0 Full .. 3 Close).
void chooseViewDistance(nfs::App* app, nfs::u32 distance);
// The game's 0x41d900: the models unlimited on the drivers that want it; the last block cleared, if one was.
std::optional<nfs::u32> unlimitedModels(nfs::App* app, const nfs::ThreadState& ts);
// unlimitedModels as the code leaves eax: `eax`, the one it is called with, where it cleared no block.
inline nfs::u32 modelsUnlimitedLeaving(nfs::App* app, const nfs::ThreadState& ts, nfs::u32 eax)
{
    return unlimitedModels(app, ts).value_or(eax);
}
// The game's 0x41da60: the track let go of at the race's end; eax as the code leaves it.
std::optional<nfs::u32> letGoOfTrack(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41dae0: the objects' pool started; its records' address.
std::optional<nfs::u32> startObjectPool(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41dc60: the opened textures taken by the driver and the file let go of; eax as the code leaves it.
std::optional<nfs::u32> takeTrackTextures(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41dcc0: the track's .col file named `name` loaded, if it is there; eax as the code leaves it.
std::optional<nfs::u32> loadCollisionFile(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// The game's 0x41de40: the objects' states all still.
bool stillObjects(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41deb0: the views' serial started over and the four views on no block.
bool startViewsOver(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41e070: whether block `index` is drawn for the camera.
std::optional<bool> blockDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 camera, nfs::u32 index);

// The game's 0x418cc0: the file or address at `file` opened with the shell; false when the game stopped (eax 1).
bool openWithShell(nfs::App* app, nfs::ThreadState& ts, nfs::u32 file);
// The game's 0x41bb50: the ambient sounds around the camera; eax as the code leaves it.
std::optional<nfs::u32> ambientAround(nfs::App* app, nfs::ThreadState& ts, nfs::u32 camera);
// The game's 0x41bc20: whether the track's ambient sounds play (eax 1 or 0).
bool ambientPlays(nfs::App* app);
// The game's 0x41d870: View Distance the next of its four, the views' distances worked out again; eax as it leaves it.
std::optional<nfs::u32> viewDistanceOnward(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41de80: the views as a race starts; eax as the code leaves it.
std::optional<nfs::u32> startTrackViews(nfs::App* app, nfs::ThreadState& ts);
}  // namespace track

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
