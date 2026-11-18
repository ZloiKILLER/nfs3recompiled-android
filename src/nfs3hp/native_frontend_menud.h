#ifndef NFS3HP_NATIVE_FRONTEND_MENUD_H
#define NFS3HP_NATIVE_FRONTEND_MENUD_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menud.cpp's typed functions (the menus' pointer, the effects behind the buttons,
 * the lobby's table of players, a pause's end), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each calls the
 * game's other functions by address on `cpu` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menud
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x49ae30: the network screen's table of players at (x, y).
reg32 drawLobbyPlayers(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x49c140: as the menus start, mouse.fsh made the pointer's textures.
reg32 loadPointerArt(App* app, CPU& cpu);
// The game's 0x49c1c0: the line of a list under the pointer, the list's top and its lines' height given.
reg32 pointerListLine(App* app, CPU& cpu, std::int32_t top, std::int32_t lineHeight);
// The game's 0x49c240: whether the pointer is inside the rectangle, its edges not counted.
reg32 pointerOverRect(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height);
// The game's 0x49c2a0: as the menus end, the pointer's file let go.
reg32 freePointerArt(App* app, CPU& cpu, reg32 eax);
// The game's 0x4a4390: logo.qfs made a texture.
reg32 startLogoTexture(App* app, CPU& cpu);
// The game's 0x4a47c0: the band of moving lines behind a list.
reg32 drawBand(App* app, CPU& cpu);
// The game's 0x4a49c0: a screen's own animation, every pass, by the screen's name.
reg32 drawScreenEffect(App* app, CPU& cpu);
// The game's 0x4a4ba0: the logo's four pictures and its glint for the screen shown.
reg32 loadLogoPictures(App* app, CPU& cpu);
// The game's 0x4a4c20: a logo item at (x, y), its pictures crossing over and its glint.
reg32 drawLogo(App* app, CPU& cpu, reg32 item, reg32 x, reg32 y);
// The game's 0x4bc670: what the race's view pass does as a pause ends.
reg32 endPause(App* app, CPU& cpu);

}  // namespace menud
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
