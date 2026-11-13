#ifndef NFS3HP_NATIVE_AIDRIVE_H
#define NFS3HP_NATIVE_AIDRIVE_H

#include "native_guest.h"

#include <optional>

/* native_aidrive.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Those that take `cpu`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one.  Where one answers with "what the code leaves in eax", that is the eax its
 * adapter leaves. */
namespace nfs3hp
{
namespace aidrive
{

// The game's 0x404d30: a cop's start in Hot Pursuit: the slice behind it to [sliceOut], its place to [placeOut].
bool placeCopAtStart(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, x86::reg32 sliceOut,
                     x86::reg32 placeOut);
// The game's 0x405220: cop put back behind the race's leader (on the road where `putBack` asks); what the code leaves in eax.
std::optional<x86::reg32> jumpBehind(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, bool putBack);
// The game's 0x405740: a cop's driver, by its state; what the code leaves in eax.
std::optional<x86::reg32> driveCop(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x405c80: the first human's car within 50 slices of slice, or 0.
std::optional<x86::reg32> humanNearSliceOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice);
// The game's 0x405cd0: whether a traffic car on the road is out of every human's sight.
bool unwatched(win32::WinApplication* app, x86::reg32 car);
// The game's 0x405d40: an extra car brought onto the road, its life a random share of traffic's; its life.
std::optional<x86::reg32> bringOn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// bringOn for a caller that asks only whether the game went on.
inline bool trafficStartDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
{
    return bringOn(app, cpu, car).has_value();
}
// The game's 0x405dc0: an extra car taken off the road (a traffic car's and a cop's own leaving); what the code leaves in eax.
std::optional<x86::reg32> takeOff(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x405e20: an extra car's step (a traffic car's, a cop's, 0 for any other); what it gives.
std::optional<x86::reg32> stepExtraCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x405fd0: car's reversing set by `how` for `frames`; what its code leaves in edx.
x86::reg32 setReversing(win32::WinApplication* app, x86::reg32 car, x86::reg32 how, x86::reg32 frames);
// The game's 0x406120: car, moving and slower than 10, turns its reversing over; whether it did.
bool reverseIfSlow(win32::WinApplication* app, x86::reg32 car);
// reverseIfSlow for a caller that asks only whether the game went on (it cannot stop it).
inline bool reverseWhenSlowDone(win32::WinApplication* app, x86::reg32 car)
{
    reverseIfSlow(app, car);
    return true;
}
// The game's 0x406160: as reverseIfSlow, where its nose also points down along the vector at `down`.
bool reverseIfTipped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 down);
// The game's 0x4088f0: the computer's car's step, each frame; what the code leaves in eax.
std::optional<x86::reg32> stepCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);

}  // namespace aidrive

namespace guest
{

// The cop in eax, the slice's address in edx, the place's in ebx.
template <>
struct ByAddress<aidrive::placeCopAtStart>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, x86::reg32 sliceOut,
                     x86::reg32 placeOut)
    {
        return watcom(app, cpu, Address, {cop, sliceOut, placeOut});
    }
};

// The cop in eax, `putBack` in edx (1 or 0); eax back.
template <>
struct ByAddress<aidrive::jumpBehind>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, bool putBack)
    {
        return watcomEax(app, cpu, Address, {cop, putBack ? 1u : 0u});
    }
};

// The cop in eax; eax back.
template <>
struct ByAddress<aidrive::driveCop>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        return watcomEax(app, cpu, Address, {cop});
    }
};

// The slice in eax; the car in eax.
template <>
struct ByAddress<aidrive::humanNearSliceOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice)
    {
        return watcomEax(app, cpu, Address, {slice});
    }
};

// The car in eax; 1 or 0 in eax.
template <>
struct ByAddress<aidrive::unwatched>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax != 0;
    }
};

// The car in eax.
template <>
struct ByAddress<aidrive::trafficStartDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The car in eax; eax back.
template <>
struct ByAddress<aidrive::takeOff>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aidrive::stepExtraCar>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

// The car in eax, `how` in edx, the frames in ebx; edx back.
template <>
struct ByAddress<aidrive::setReversing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 how,
                           x86::reg32 frames)
    {
        x86::reg32 edx;
        {
            const KeptRegisters kept(cpu);
            cpu.eax = car;
            cpu.edx = how;
            cpu.ebx = frames;
            guest::call(app, cpu, Address);
            edx = cpu.edx;
        }
        return edx;
    }
};

// The car in eax.
template <>
struct ByAddress<aidrive::reverseWhenSlowDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The car in eax, the vector's address in edx.
template <>
struct ByAddress<aidrive::reverseIfTipped>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 down)
    {
        return watcom(app, cpu, Address, {car, down});
    }
};

// The car in eax; eax back.
template <>
struct ByAddress<aidrive::stepCar>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
