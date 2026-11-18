#ifndef NFS3HP_NATIVE_LOAD_CARMODELS_H
#define NFS3HP_NATIVE_LOAD_CARMODELS_H

#include "native_guest.h"

/* native_load_carmodels.cpp's typed functions that other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each may call
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
// copyVivModel's block, as the code's thunk calls it (ebx the .viv on entry).
inline std::optional<x86::reg32> vivModelCopied(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                                x86::reg32 name, x86::reg32 viv)
{
    x86::reg32 copyLeft = viv;
    return copyVivModel(app, cpu, car, name, viv, copyLeft);
}
// The game's 0x4dc940: player's specs loaded (or made anew and saved); eax as the code leaves it.
std::optional<x86::reg32> playerSpecs(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);

// The game's 0x49c880: the colour at `rgb` as hue, saturation and brightness to `hsv`; the brightness stored.
x86::reg32 colourToHsv(win32::WinApplication* app, x86::reg32 rgb, x86::reg32 hsv);
// The game's 0x49ca50: the colour at `hsv` made red, green and blue to `rgb`; the blue stored.
std::optional<x86::reg32> hsvToColour(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 hsv, x86::reg32 rgb);
// The game's 0x49d320: the .tga at `tga` loaded as the model's texture, `most` at most, the texture at `texture`,
// the colour `color`, whether to upload `upload`, where to copy `copyTo`; in eax what the code leaves.
std::optional<x86::reg32> tgaLoaded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 model, x86::reg32 tga,
                                    x86::reg32 most, x86::reg32 texture, x86::reg32 color, x86::reg32 upload,
                                    x86::reg32 copyTo);
// The game's 0x49d450: the model named `name` loaded, `most` at most each way, the colour `color`; in eax the model.
std::optional<x86::reg32> modelLoaded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 most,
                                      x86::reg32 color);
// The game's 0x4dc490: the place in the player's specs of the car with the id at `id`, -1 for none.
x86::reg32 specIndexOf(win32::WinApplication* app, x86::reg32 player, x86::reg32 id);
// The game's 0x4dca50: the player's specs saved.
std::optional<x86::reg32> specsSavedOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);
// The game's 0x49b540: the car file's receiving begun for the record `record`, `bytes` bytes into `buffer`.
std::optional<x86::reg32> carpReceiveBegun(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record,
                                           x86::reg32 bytes, x86::reg32 buffer);
// The game's 0x49b5f0: `count` bytes at `data` received into the record `record`'s file.
std::optional<x86::reg32> carpChunkReceived(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record,
                                            x86::reg32 data, x86::reg32 count);
// The game's 0x49b800: the car `car`'s file's sending begun with the header `header`; the file.
std::optional<x86::reg32> carpSendBegun(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 header,
                                        x86::reg32 car);

}  // namespace load

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
