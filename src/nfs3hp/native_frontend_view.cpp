#include "native_frontend.h"
#include <nfs3hp.h>
#include <iterator>

/* The pan-and-zoom widget's tiles and its state, decompiled: what is left of it
 * outside src/nfs3hp/port_view.cpp, which draws the tiles and moves the view.
 *
 *   the tiles     five pictures, 0000 to 0004, of one .fsh: their records made
 *                 (sub_464700, the first load), or the textures those records
 *                 hold filled again (sub_4647a0, the next picture on the same
 *                 textures);
 *   the interior  the picture of the car the showcase has asked for, loaded when
 *                 the car changes (sub_464eb0, every frame from frameOfView);
 *   its state     what the widget's table (0x557a28) answers at +0x14 (sub_4652f0).
 *
 * All of the game's state stays in guest memory.  What each function leaves in
 * eax and in the registers it does not save is kept: Watcom's callers may go on
 * with them. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using x86::reg32;

constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kStrcmp = 0x4ee310;
constexpr reg32 kLoadFile = 0x4e0ec0;              // (name, flags): the file in a block, or 0
constexpr reg32 kFreeBlock = 0x4e1890;
constexpr reg32 kShapeInFile = 0x4ef1e0;           // (file, name): the .fsh's shape of that name
constexpr reg32 kNewTile = 0x4d4080;               // (file, name, side, record), format: a tile's texture made
constexpr reg32 kShapeIntoTexture = 0x4d5ed0;      // (texture, shape, side, format)
constexpr reg32 kCarFolder = 0x4392b0;
constexpr reg32 kCarFileStem = 0x439290;
constexpr reg32 kCarFlag4 = 0x4395c0;              // the car has a folder of its own
constexpr reg32 kCurrentMenuName = 0x44ae00;
constexpr reg32 kFillTiles = 0x4647a0;

/* ---------------------------------------------------------------------------
 * The tiles
 * ------------------------------------------------------------------------- */

constexpr GuestString kPathFormat = { 0x53ab0c, "%s%s" };
constexpr GuestString kTileNameFormat = { 0x53ab14, "%04d" };
constexpr GuestString kTilesStrings[] = { kPathFormat, kTileNameFormat };

constexpr reg32 kTiles = 0x67895c;                 // the widget's records: port_view's grid draws them
constexpr reg32 kTileBytes = 44;
constexpr reg32 kTileTexture = 4;                  // the texture's handle in a record
constexpr reg32 kTileCount = 5;
constexpr reg32 kTileSide = 0x100;                 // pixels
constexpr reg32 kTileFormat = 3;
// The block's flags.  The game sets edx to them before the sprintf, which keeps it, for the load after.
constexpr reg32 kLoadFlags = 0x10;

/* The frame of both: ebx, ecx, esi, edi, ebp, then one buffer for the file's
 * path and each tile's name. */
constexpr reg32 kNameBytes = 0x100;
constexpr reg32 kTilesFrame = 20 + kNameBytes;

/* The file of the tiles, its path in the frame's buffer. */
reg32 loadTileFile(Guest& guest, reg32 folder, reg32 fileName)
{
    const reg32 name = guest.frame();
    guest.call(kSprintf, args({}, { name, kPathFormat.address, folder, fileName }));
    return guest.call(kLoadFile, args({ name, kLoadFlags }));
}

/* sub_464700: the tiles' records made from folder + fileName, then the file freed. */
reg32 makeTiles(Guest& guest, reg32 folder, reg32 fileName)
{
    const reg32 name = guest.frame();
    const reg32 file = loadTileFile(guest, folder, fileName);
    for (reg32 tile = 0; tile < kTileCount; ++tile)
    {
        guest.call(kSprintf, args({}, { name, kTileNameFormat.address, tile }));
        guest.call(kNewTile, args({ file, name, kTileSide, kTiles + kTileBytes * tile }, { kTileFormat }));
    }
    return guest.call(kFreeBlock, args({ file }));
}

/* sub_4647a0: the same records' textures filled from folder + fileName, then the
 * file freed. */
reg32 fillTiles(Guest& guest, reg32 folder, reg32 fileName)
{
    const reg32 name = guest.frame();
    const reg32 file = loadTileFile(guest, folder, fileName);
    for (reg32 tile = 0; tile < kTileCount; ++tile)
    {
        guest.call(kSprintf, args({}, { name, kTileNameFormat.address, tile }));
        const reg32 shape = guest.call(kShapeInFile, args({ file, name }));
        const reg32 texture = guest.dword(kTiles + kTileBytes * tile + kTileTexture);
        guest.call(kShapeIntoTexture, args({ texture, shape, kTileSide, kTileFormat }));
    }
    return guest.call(kFreeBlock, args({ file }));
}

/* ---------------------------------------------------------------------------
 * The interior
 * ------------------------------------------------------------------------- */

constexpr GuestString kInteriorFormat = { 0x53ab3c, "IN_%s.fsh" };
constexpr GuestString kCredits = { 0x53ab48, "credits1" };
constexpr GuestString kCreditsFormat = { 0x53ab54, "IN_%d.fsh" };
constexpr GuestString kCarFolderFormat = { 0x53ab60, "%s\\%s\\" };
constexpr GuestString kInteriorStrings[] = { kInteriorFormat, kCredits, kCreditsFormat, kCarFolderFormat };

constexpr reg32 kScreenFade = 0x559c60;            // a screen's fade out: nothing is loaded under it
constexpr reg32 kCar = 0x664794;                   // the car to show
constexpr reg32 kShownCar = 0x664798;              // the car shown
constexpr reg32 kInteriorCar = 0x678940;           // the car whose interior the tiles hold
constexpr reg32 kCarRootOwn = 0x7a28b4;            // the folder of the cars' own folders
constexpr reg32 kCarRootCommon = 0x7a3234;         // the folder of the others' art

/* The frame: six registers, then two buffers of 300 bytes. */
constexpr reg32 kPathBytes = 300;
constexpr reg32 kInteriorFrame = 24 + 2 * kPathBytes;

/* sub_464eb0: the car's interior on the tiles, when the car is not the one they
 * hold.  Under a fade eax stays as it came; with the same car it is the car. */
reg32 loadInterior(Guest& guest)
{
    if (guest.dword(kScreenFade) != 0)
        return guest.eax();
    const reg32 car = guest.dword(kCar);
    if (car == guest.dword(kInteriorCar))
        return car;
    guest.setDword(kShownCar, car);
    guest.setDword(kInteriorCar, car);

    const reg32 picture = guest.frame();
    const reg32 folder = guest.frame() + kPathBytes;
    const reg32 carFolder = guest.call(kCarFolder, args({ car }));
    const reg32 stem = guest.call(kCarFileStem, args({ guest.dword(kShownCar) }));
    guest.call(kSprintf, args({}, { picture, kInteriorFormat.address, stem }));

    const reg32 screen = guest.call(kCurrentMenuName);
    if (guest.call(kStrcmp, args({ screen, kCredits.address })) == 0)
    {
        // By the car's number there.  The folder copied is the game's own dead store:
        // both ways on write over it.
        guest.call(kSprintf, args({}, { picture, kCreditsFormat.address, guest.dword(kShownCar) }));
        guest.copyString(folder, kCarRootCommon);
    }

    if (guest.call(kCarFlag4, args({ guest.dword(kShownCar) })) != 0)
        guest.call(kSprintf, args({}, { folder, kCarFolderFormat.address, kCarRootOwn, carFolder }));
    else
        guest.copyString(folder, kCarRootCommon);
    return guest.call(kFillTiles, args({ picture, folder }));
}

/* ---------------------------------------------------------------------------
 * The widget's state
 * ------------------------------------------------------------------------- */

constexpr reg32 kDragging = 0x67894c;              // port_view's: the mouse drags the map
constexpr reg32 kPanActive = 0x678950;             // port_view's: the map is panning
constexpr reg32 kStateDragging = 2, kStatePanning = 1;
constexpr reg32 kStateIdle = 0xfffffffd;           // -3

/* sub_4652f0: the table's +0x14. */
reg32 widgetState(const Guest& guest)
{
    if (guest.dword(kDragging) != 0)
        return kStateDragging;
    return guest.dword(kPanActive) != 0 ? kStatePanning : kStateIdle;
}

constexpr std::uint8_t kRestoreBcSiDi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

}

void frontend::viewMakeTiles(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_464700", 0, kTilesFrame, kTilesStrings, std::size(kTilesStrings), kRestoreBcSiDi };
    const reg32 fileName = cpu.eax, folder = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return makeTiles(guest, folder, fileName); });
}

void frontend::viewFillTiles(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4647a0", 0, kTilesFrame, kTilesStrings, std::size(kTilesStrings), kRestoreBcSiDi };
    const reg32 fileName = cpu.eax, folder = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return fillTiles(guest, folder, fileName); });
}

void frontend::viewLoadInterior(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_464eb0", 0, kInteriorFrame, kInteriorStrings, std::size(kInteriorStrings), fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return loadInterior(guest); });
}

void frontend::viewWidgetState(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4652f0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return widgetState(guest); });
}

}
