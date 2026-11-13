#ifndef NFS3HP_NATIVE_TRACK_H
#define NFS3HP_NATIVE_TRACK_H

#include "native_guest.h"

/* native_track.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * one without `cpu` calls nothing, and through ByAddress leaves cpu.terminate to say it. */
namespace nfs3hp
{
namespace track
{

/* What objectAnimation leaves in eax and edx. */
struct MovedLeft
{
    x86::reg32 result;
    // edx where no path was started: the object's record, or its state over the path's.
    std::optional<x86::reg32> record;
};

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
// The game's 0x41d350: object `index` of the car's block set moving; what the code leaves in eax and edx.
std::optional<MovedLeft> setObjectMoving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                         x86::reg32 index);
// setObjectMoving for a caller that reads eax alone: -1 set moving, 0 moving already, 2 one that falls, else 1.
inline std::optional<x86::reg32> objectMoving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                              x86::reg32 index)
{
    const std::optional<MovedLeft> left = setObjectMoving(app, cpu, car, index);
    if (!left)
        return std::nullopt;
    return left->result;
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
// The game's 0x41dcc0: the track's .col file named `name` loaded, if it is there.
bool loadCollisionFile(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x41de40: the objects' states all still.
bool stillObjects(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41deb0: the views' serial started over and the four views on no block.
bool startViewsOver(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41e070: whether block `index` is drawn for the camera.
std::optional<bool> blockDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 index);

}  // namespace track

namespace guest
{

template <>
struct ByAddress<track::startArena>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<track::loadTrackGeometry>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The camera in eax, the view in edx; the group left in edx (eax the view drawn).
template <>
struct ByAddress<track::viewOnCameraBlock>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera,
                                          x86::reg32 index)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = camera;
        cpu.edx = index;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return cpu.edx;
    }
};

// The list in eax.
template <>
struct ByAddress<track::bracketOrdered>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list)
    {
        return watcom(app, cpu, Address, {list});
    }
};

// The camera in eax, the list in edx, the buckets in ebx.
template <>
struct ByAddress<track::drawTrackBlocks>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 list,
                     x86::reg32 buckets)
    {
        return watcom(app, cpu, Address, {camera, list, buckets});
    }
};

// The view in eax.
template <>
struct ByAddress<track::chooseView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        watcom(app, cpu, Address, {index});
        return cpu.eax;
    }
};

// The car in eax, the object in edx.
template <>
struct ByAddress<track::objectMoving>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 index)
    {
        return watcomEax(app, cpu, Address, {car, index});
    }
};

// The distance in eax.
template <>
struct ByAddress<track::chooseViewDistance>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 distance)
    {
        watcom(app, cpu, Address, {distance});
    }
};

// Called with `eax`, which it leaves where it clears no block.
template <>
struct ByAddress<track::modelsUnlimitedLeaving>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        watcom(app, cpu, Address, {eax});
        return cpu.eax;
    }
};

template <>
struct ByAddress<track::letGoOfTrack>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<track::startObjectPool>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<track::takeTrackTextures>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The name in eax.
template <>
struct ByAddress<track::loadCollisionFile>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return watcom(app, cpu, Address, {name});
    }
};

template <>
struct ByAddress<track::stillObjects>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<track::startViewsOver>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// The camera in eax, the block in edx; 1 or 0 in eax.
template <>
struct ByAddress<track::blockDrawn>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 index)
    {
        const std::optional<x86::reg32> drawn = watcomEax(app, cpu, Address, {camera, index});
        if (!drawn)
            return std::nullopt;
        return *drawn != 0;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
