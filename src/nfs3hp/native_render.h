#ifndef NFS3HP_NATIVE_RENDER_H
#define NFS3HP_NATIVE_RENDER_H

#include "native_guest.h"

/* native_render.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  The file's other
 * functions other files call are still the code's registers line by line, with no typed core:
 * those stay calls by address. */
namespace nfs3hp
{
namespace render
{

// The game's 0x434870: the polygon lists' drawers chosen for the driver's abilities.
void choosePolygonDrawers(win32::WinApplication* app);

}  // namespace render

namespace guest
{

template <>
struct ByAddress<render::choosePolygonDrawers>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
