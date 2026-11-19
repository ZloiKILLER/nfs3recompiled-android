#include "native_frontend_view.h"
#include "native_watcom.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_loop.h"
#include "native_frontend_rende.h"
#include "native_frontend_showcase.h"
#include "native_load_cardb.h"
#include "native_graphics.h"
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
 * with them.
 *
 * Plan step 5: each function a typed core in frontend::view (native_frontend_view.h), in a frame
 * of its own where the game's function has one (fe::Frame), and a thunk under the game's name
 * for the calls by address.  A core of this file calls another as C++ (guest::direct), and the
 * typed cores of other files too (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using nfs::u32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

constexpr u32 kSprintf = 0x4df690;
constexpr u32 kStrcmp = 0x4ee310;
constexpr u32 kCarFlag4 = 0x4395c0;              // the car has a folder of its own
constexpr u32 kShapeIntoTexture = 0x4d5ed0;      // (texture, shape, side, format)

/* ---------------------------------------------------------------------------
 * The tiles
 * ------------------------------------------------------------------------- */

constexpr GuestString kPathFormat = { 0x53ab0c, "%s%s" };
constexpr GuestString kTileNameFormat = { 0x53ab14, "%04d" };
constexpr GuestString kTilesStrings[] = { kPathFormat, kTileNameFormat };

constexpr u32 kTiles = 0x67895c;                 // the widget's records: port_view's grid draws them
constexpr u32 kTileBytes = 44;
constexpr u32 kTileTexture = 4;                  // the texture's handle in a record
constexpr u32 kTileCount = 5;
constexpr u32 kTileSide = 0x100;                 // pixels
constexpr u32 kTileFormat = 3;
// The block's flags.  The game sets edx to them before the sprintf, which keeps it, for the load after.
constexpr u32 kLoadFlags = 0x10;

/* The frame of both: ebx, ecx, esi, edi, ebp, then one buffer for the file's
 * path and each tile's name. */
constexpr u32 kNameBytes = 0x100;
constexpr u32 kTilesFrame = 20 + kNameBytes;

/* The file of the tiles, its path in the frame's buffer. */
u32 loadTileFile(App* app, ThreadState& ts, Guest& guest, u32 folder, u32 fileName)
{
    const u32 name = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kPathFormat.address, guest::Words{folder, fileName});
    return *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, kLoadFlags);
}

}  // namespace

/* sub_464700: the tiles' records made from folder + fileName, then the file freed. */
nfs::u32 frontend::view::makeTiles(App* app, ThreadState& ts, u32 fileName, u32 folder)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kTilesFrame);
    const u32 name = guest.frame();
    const u32 file = loadTileFile(app, ts, guest, folder, fileName);
    for (u32 tile = 0; tile < kTileCount; ++tile)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kTileNameFormat.address, guest::Words{tile});
        // The tile's texture made: (file, name, side, record), the format on the stack.
        fe::direct<0x4d4080, rende::loadShapeTexture>(app, ts, file, name, kTileSide, kTiles + kTileBytes * tile,
                                                      kTileFormat);
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

/* sub_4647a0: the same records' textures filled from folder + fileName, then the
 * file freed. */
nfs::u32 frontend::view::fillTiles(App* app, ThreadState& ts, u32 fileName, u32 folder)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kTilesFrame);
    const u32 name = guest.frame();
    const u32 file = loadTileFile(app, ts, guest, folder, fileName);
    for (u32 tile = 0; tile < kTileCount; ++tile)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kTileNameFormat.address, guest::Words{tile});
        const u32 shape = fe::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, file, name);
        const u32 texture = guest.dword(kTiles + kTileBytes * tile + kTileTexture);
        fe::direct<0x4d5ed0, graphics::shapeIntoTextureOf>(app, ts, texture, shape, kTileSide, kTileFormat);
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

/* ---------------------------------------------------------------------------
 * The interior
 * ------------------------------------------------------------------------- */

namespace
{

constexpr GuestString kInteriorFormat = { 0x53ab3c, "IN_%s.fsh" };
constexpr GuestString kCredits = { 0x53ab48, "credits1" };
constexpr GuestString kCreditsFormat = { 0x53ab54, "IN_%d.fsh" };
constexpr GuestString kCarFolderFormat = { 0x53ab60, "%s\\%s\\" };
constexpr GuestString kInteriorStrings[] = { kInteriorFormat, kCredits, kCreditsFormat, kCarFolderFormat };

constexpr u32 kScreenFade = 0x559c60;            // a screen's fade out: nothing is loaded under it
constexpr u32 kCar = 0x664794;                   // the car to show
constexpr u32 kShownCar = 0x664798;              // the car shown
constexpr u32 kInteriorCar = 0x678940;           // the car whose interior the tiles hold
constexpr u32 kCarRootOwn = 0x7a28b4;            // the folder of the cars' own folders
constexpr u32 kCarRootCommon = 0x7a3234;         // the folder of the others' art

/* The frame: six registers, then two buffers of 300 bytes. */
constexpr u32 kPathBytes = 300;
constexpr u32 kInteriorFrame = 24 + 2 * kPathBytes;

}  // namespace

/* sub_464eb0: the car's interior on the tiles, when the car is not the one they
 * hold.  Under a fade eax stays as it came; with the same car it is the car. */
nfs::u32 frontend::view::loadInterior(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kInteriorFrame);
    if (guest.dword(kScreenFade) != 0)
        return eax;
    const u32 car = guest.dword(kCar);
    if (car == guest.dword(kInteriorCar))
        return car;
    guest.setDword(kShownCar, car);
    guest.setDword(kInteriorCar, car);

    const u32 picture = guest.frame();
    const u32 folder = guest.frame() + kPathBytes;
    const u32 carFolder = *fe::direct<0x4392b0, load::folderOf>(app, ts, car);
    const u32 stem = fe::direct<0x439290, frontend::showcase::carFileStem>(app, ts, guest.dword(kShownCar));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), picture, kInteriorFormat.address, guest::Words{stem});

    const u32 screen = fe::direct<0x44ae00, frontend::loop::currentMenuName>(app, ts);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), screen, kCredits.address) == 0)
    {
        // By the car's number there.  The folder copied is the game's own dead store:
        // both ways on write over it.
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), picture, kCreditsFormat.address, guest::Words{guest.dword(kShownCar)});
        guest.copyString(folder, kCarRootCommon);
    }

    if (*fe::direct<0x4395c0, load::flag4Of>(guest.app(), guest.ts(), guest.dword(kShownCar)) != 0)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), folder, kCarFolderFormat.address, guest::Words{kCarRootOwn, carFolder});
    else
        guest.copyString(folder, kCarRootCommon);
    return guest::direct<0x4647a0, view::fillTiles>(app, ts, picture, folder);
}

/* ---------------------------------------------------------------------------
 * The widget's state
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kDragging = 0x67894c;              // port_view's: the mouse drags the map
constexpr u32 kPanActive = 0x678950;             // port_view's: the map is panning
constexpr u32 kStateDragging = 2, kStatePanning = 1;
constexpr u32 kStateIdle = 0xfffffffd;           // -3

constexpr std::uint8_t kRestoreBcSiDi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

}  // namespace

/* sub_4652f0: the table's +0x14. */
nfs::u32 frontend::view::widgetState(App* app, ThreadState& ts)
{
    const Guest guest(app, ts);
    if (guest.dword(kDragging) != 0)
        return kStateDragging;
    return guest.dword(kPanActive) != 0 ? kStatePanning : kStateIdle;
}

nfs::u32 frontend::viewMakeTiles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_464700", 0, 0, kTilesStrings, std::size(kTilesStrings), kRestoreBcSiDi };
    const u32 fileName = args.eax, folder = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return view::makeTiles(app, ts, fileName, folder); });
}

nfs::u32 frontend::viewFillTiles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4647a0", 0, 0, kTilesStrings, std::size(kTilesStrings), kRestoreBcSiDi };
    const u32 fileName = args.eax, folder = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return view::fillTiles(app, ts, fileName, folder); });
}

nfs::u32 frontend::viewLoadInterior(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_464eb0", 0, 0, kInteriorStrings, std::size(kInteriorStrings), fe::kRestoreAll };
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return view::loadInterior(app, ts, eax); });
}

nfs::u32 frontend::viewWidgetState(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4652f0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return view::widgetState(app, ts); });
}

}
