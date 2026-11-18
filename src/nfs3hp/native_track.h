#ifndef NFS3HP_NATIVE_TRACK_H
#define NFS3HP_NATIVE_TRACK_H

#include "native_guest.h"

/* native_track.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * one without `cpu` calls nothing, and by address leaves cpu.terminate to say it. */
namespace nfs3hp
{
namespace track
{

// The game's 0x419b60: the arena allocated and none of it used; its address.
std::optional<x86::reg32> startArena(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41a100: the track's geometry loaded, its colours tinted (or grey); eax as the code leaves it.
std::optional<x86::reg32> loadTrackGeometry(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41aaf0: view `index` the one drawn for the camera, its distances worked out; the camera's group.
std::optional<x86::reg32> viewOnCameraBlock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera,
                                            x86::reg32 index);
// The game's 0x41b8c0: the list at `list` put between two state records of the drawing's.
bool bracketOrdered(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list);
// The game's 0x41cdc0: the track's blocks of the list at `list` drawn for the camera into the buckets.
bool drawTrackBlocks(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 list,
                     x86::reg32 buckets);
// The game's 0x41d310: view `index` the one drawn; the view's offset.
x86::reg32 chooseView(win32::WinApplication* app, x86::reg32 index);
// The game's 0x41d350: object `index` of the car's block set moving: -1 set moving, 0 moving already, 2 one
// that falls, else 1.
std::optional<x86::reg32> setObjectMoving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 index);
// setObjectMoving, under the name the game's callers take it by.
inline std::optional<x86::reg32> objectMoving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                              x86::reg32 index)
{
    return setObjectMoving(app, cpu, car, index);
}
// The game's 0x41d610: View Distance set (0 Full .. 3 Close).
void chooseViewDistance(win32::WinApplication* app, x86::reg32 distance);
// The game's 0x41d900: the models unlimited on the drivers that want it; the last block cleared, if one was.
std::optional<x86::reg32> unlimitedModels(win32::WinApplication* app, const x86::CPU& cpu);
// unlimitedModels as the code leaves eax: `eax`, the one it is called with, where it cleared no block.
inline x86::reg32 modelsUnlimitedLeaving(win32::WinApplication* app, const x86::CPU& cpu, x86::reg32 eax)
{
    return unlimitedModels(app, cpu).value_or(eax);
}
// The game's 0x41da60: the track let go of at the race's end; eax as the code leaves it.
std::optional<x86::reg32> letGoOfTrack(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41dae0: the objects' pool started; its records' address.
std::optional<x86::reg32> startObjectPool(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41dc60: the opened textures taken by the driver and the file let go of; eax as the code leaves it.
std::optional<x86::reg32> takeTrackTextures(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41dcc0: the track's .col file named `name` loaded, if it is there; eax as the code leaves it.
std::optional<x86::reg32> loadCollisionFile(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x41de40: the objects' states all still.
bool stillObjects(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41deb0: the views' serial started over and the four views on no block.
bool startViewsOver(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41e070: whether block `index` is drawn for the camera.
std::optional<bool> blockDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 index);

// The game's 0x418cc0: the file or address at `file` opened with the shell; false when the game stopped (eax 1).
bool openWithShell(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 file);
// The game's 0x41bb50: the ambient sounds around the camera; eax as the code leaves it.
std::optional<x86::reg32> ambientAround(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera);
// The game's 0x41bc20: whether the track's ambient sounds play (eax 1 or 0).
bool ambientPlays(win32::WinApplication* app);
// The game's 0x41d870: View Distance the next of its four, the views' distances worked out again; eax as it leaves it.
std::optional<x86::reg32> viewDistanceOnward(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41de80: the views as a race starts; eax as the code leaves it.
std::optional<x86::reg32> startTrackViews(win32::WinApplication* app, x86::CPU& cpu);
}  // namespace track

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
