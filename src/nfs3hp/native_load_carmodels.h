#ifndef NFS3HP_NATIVE_LOAD_CARMODELS_H
#define NFS3HP_NATIVE_LOAD_CARMODELS_H

#include "native_guest.h"

/* native_load_carmodels.cpp's typed functions that other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each may call
 * the game's other functions by address on `ts`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace load
{

/* What 0x49c700 leaves in eax, ebx, ecx and edx (it keeps none of them). */
struct ColorsLeft
{
    nfs::u32 eax, ebx, ecx, edx;
};

/* What one of the textures' functions leaves in eax, ebx, ecx and edx (it keeps none of them). */
struct Left
{
    nfs::u32 eax, ebx, ecx, edx;
};

// The game's 0x49c700: the colours of car.fce in the .viv named `name`: their count to [count],
// the first `wanted` of them to `colors`, its 12 bytes to `twelve`.
std::optional<ColorsLeft> readColors(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 colors,
                                     nfs::u32 wanted, nfs::u32 count, nfs::u32 twelve);
// readColors for a caller that asks only whether the game went on.
inline bool coloursRead(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 colors,
                        nfs::u32 wanted, nfs::u32 count, nfs::u32 twelve)
{
    return readColors(app, ts, name, colors, wanted, count, twelve).has_value();
}
// The game's 0x4b7a60: car's model named `name` loaded from the .viv at `viv`, its texture `most`
// at most each way, in the colour at `color` when there is one; in eax the model's block.
std::optional<Left> loadVivModel(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 name,
                                 nfs::u32 most, nfs::u32 color, nfs::u32 viv);
// loadVivModel's block.
inline std::optional<nfs::u32> vivModelLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car,
                                                nfs::u32 name, nfs::u32 most, nfs::u32 color,
                                                nfs::u32 viv)
{
    const std::optional<Left> left = loadVivModel(app, ts, car, name, most, color, viv);
    if (!left)
        return std::nullopt;
    return left->eax;
}
// The game's 0x4b7ba0: another of the model named `name` in the .viv at `viv`, given car's
// model's texture; the block, what the copy leaves in ebx to `copyLeft`.
std::optional<nfs::u32> copyVivModel(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 name,
                                       nfs::u32 viv, nfs::u32& copyLeft);
// copyVivModel's block, as the code's thunk calls it (ebx the .viv on entry).
inline std::optional<nfs::u32> vivModelCopied(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car,
                                                nfs::u32 name, nfs::u32 viv)
{
    nfs::u32 copyLeft = viv;
    return copyVivModel(app, ts, car, name, viv, copyLeft);
}
// The game's 0x4dc940: player's specs loaded (or made anew and saved); eax as the code leaves it.
std::optional<nfs::u32> playerSpecs(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player);

// The game's 0x49c880: the colour at `rgb` as hue, saturation and brightness to `hsv`; the brightness stored.
nfs::u32 colourToHsv(nfs::App* app, nfs::u32 rgb, nfs::u32 hsv);
// The game's 0x49ca50: the colour at `hsv` made red, green and blue to `rgb`; the blue stored.
std::optional<nfs::u32> hsvToColour(nfs::App* app, nfs::ThreadState& ts, nfs::u32 hsv, nfs::u32 rgb);
// The game's 0x49d320: the .tga at `tga` loaded as the model's texture, `most` at most, the texture at `texture`,
// the colour `color`, whether to upload `upload`, where to copy `copyTo`; in eax what the code leaves.
std::optional<nfs::u32> tgaLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 model, nfs::u32 tga,
                                    nfs::u32 most, nfs::u32 texture, nfs::u32 color, nfs::u32 upload,
                                    nfs::u32 copyTo);
// The game's 0x49d450: the model named `name` loaded, `most` at most each way, the colour `color`; in eax the model.
std::optional<nfs::u32> modelLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 most,
                                      nfs::u32 color);
// The game's 0x4dc490: the place in the player's specs of the car with the id at `id`, -1 for none.
nfs::u32 specIndexOf(nfs::App* app, nfs::u32 player, nfs::u32 id);
// The game's 0x4dca50: the player's specs saved.
std::optional<nfs::u32> specsSavedOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player);
// The game's 0x49b540: the car file's receiving begun for the record `record`, `bytes` bytes into `buffer`.
std::optional<nfs::u32> carpReceiveBegun(nfs::App* app, nfs::ThreadState& ts, nfs::u32 record,
                                           nfs::u32 bytes, nfs::u32 buffer);
// The game's 0x49b5f0: `count` bytes at `data` received into the record `record`'s file.
std::optional<nfs::u32> carpChunkReceived(nfs::App* app, nfs::ThreadState& ts, nfs::u32 record,
                                            nfs::u32 data, nfs::u32 count);
// The game's 0x49b800: the car `car`'s file's sending begun with the header `header`; the file.
std::optional<nfs::u32> carpSendBegun(nfs::App* app, nfs::ThreadState& ts, nfs::u32 header,
                                        nfs::u32 car);

}  // namespace load

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
