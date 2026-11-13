#ifndef NFS3HP_NATIVE_LOAD_CARMODELS_H
#define NFS3HP_NATIVE_LOAD_CARMODELS_H

#include "native_guest.h"

/* native_load_carmodels.cpp's typed functions that other files call as C++ (guest::direct), and
 * how the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call
 * the game's other functions by address on `cpu`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace load
{

/* What 0x49c700 leaves in eax, ebx, ecx and edx (it keeps none of them). */
struct ColorsLeft
{
    x86::reg32 eax, ebx, ecx, edx;
};

/* What one of the textures' functions leaves in eax, ebx, ecx and edx (it keeps none of them). */
struct Left
{
    x86::reg32 eax, ebx, ecx, edx;
};

// The game's 0x49c700: the colours of car.fce in the .viv named `name`: their count to [count],
// the first `wanted` of them to `colors`, its 12 bytes to `twelve`.
std::optional<ColorsLeft> readColors(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 colors,
                                     x86::reg32 wanted, x86::reg32 count, x86::reg32 twelve);
// readColors for a caller that asks only whether the game went on.
inline bool coloursRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 colors,
                        x86::reg32 wanted, x86::reg32 count, x86::reg32 twelve)
{
    return readColors(app, cpu, name, colors, wanted, count, twelve).has_value();
}
// The game's 0x4b7a60: car's model named `name` loaded from the .viv at `viv`, its texture `most`
// at most each way, in the colour at `color` when there is one; in eax the model's block.
std::optional<Left> loadVivModel(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 name,
                                 x86::reg32 most, x86::reg32 color, x86::reg32 viv);
// loadVivModel's block.
inline std::optional<x86::reg32> vivModelLoaded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                                x86::reg32 name, x86::reg32 most, x86::reg32 color,
                                                x86::reg32 viv)
{
    const std::optional<Left> left = loadVivModel(app, cpu, car, name, most, color, viv);
    if (!left)
        return std::nullopt;
    return left->eax;
}
// The game's 0x4b7ba0: another of the model named `name` in the .viv at `viv`, given car's
// model's texture; the block, what the copy leaves in ebx to `copyLeft`.
std::optional<x86::reg32> copyVivModel(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 name,
                                       x86::reg32 viv, x86::reg32& copyLeft);
// copyVivModel's block, as the code's adapter calls it (ebx the .viv on entry).
inline std::optional<x86::reg32> vivModelCopied(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                                x86::reg32 name, x86::reg32 viv)
{
    x86::reg32 copyLeft = viv;
    return copyVivModel(app, cpu, car, name, viv, copyLeft);
}
// The game's 0x4dc940: player's specs loaded (or made anew and saved); eax as the code leaves it.
std::optional<x86::reg32> playerSpecs(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);

}  // namespace load

namespace guest
{

// The name in eax, the colours in edx, how many in ebx, the count's address in ecx, the 12 bytes'
// on the stack (ret 4).
template <>
struct ByAddress<load::coloursRead>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 colors,
                     x86::reg32 wanted, x86::reg32 count, x86::reg32 twelve)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = name;
        cpu.edx = colors;
        cpu.ebx = wanted;
        cpu.ecx = count;
        return guest::call(app, cpu, Address, {twelve});
    }
};

// The car in eax, the name in edx, the most in ebx, the colour in ecx, the .viv on the stack
// (ret 4); eax the block.
template <>
struct ByAddress<load::vivModelLoaded>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 name, x86::reg32 most, x86::reg32 color, x86::reg32 viv)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        cpu.edx = name;
        cpu.ebx = most;
        cpu.ecx = color;
        if (!guest::call(app, cpu, Address, {viv}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The car in eax, the name in edx, the .viv in ebx; eax the block.
template <>
struct ByAddress<load::vivModelCopied>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 name, x86::reg32 viv)
    {
        return watcomEax(app, cpu, Address, {car, name, viv});
    }
};

// The player in eax.
template <>
struct ByAddress<load::playerSpecs>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return watcomEax(app, cpu, Address, {player});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
