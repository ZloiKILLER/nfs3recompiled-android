#ifndef NFS3HP_NATIVE_FRONTEND_MENUD_H
#define NFS3HP_NATIVE_FRONTEND_MENUD_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menud.cpp's typed functions (the menus' pointer, the effects behind the buttons,
 * the lobby's table of players, a pause's end), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each calls the
 * game's other functions by address on `ts` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menud
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x49ae30: the network screen's table of players at (x, y).
u32 drawLobbyPlayers(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x49c140: as the menus start, mouse.fsh made the pointer's textures.
u32 loadPointerArt(App* app, ThreadState& ts);
// The game's 0x49c1c0: the line of a list under the pointer, the list's top and its lines' height given.
u32 pointerListLine(App* app, ThreadState& ts, std::int32_t top, std::int32_t lineHeight);
// The game's 0x49c240: whether the pointer is inside the rectangle, its edges not counted.
u32 pointerOverRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height);
// The game's 0x49c2a0: as the menus end, the pointer's file let go.
u32 freePointerArt(App* app, ThreadState& ts, u32 eax);
// The game's 0x4a4390: logo.qfs made a texture.
u32 startLogoTexture(App* app, ThreadState& ts);
// The game's 0x4a47c0: the band of moving lines behind a list.
u32 drawBand(App* app, ThreadState& ts);
// The game's 0x4a49c0: a screen's own animation, every pass, by the screen's name.
u32 drawScreenEffect(App* app, ThreadState& ts);
// The game's 0x4a4ba0: the logo's four pictures and its glint for the screen shown.
u32 loadLogoPictures(App* app, ThreadState& ts);
// The game's 0x4a4c20: a logo item at (x, y), its pictures crossing over and its glint.
u32 drawLogo(App* app, ThreadState& ts, u32 item, u32 x, u32 y);
// The game's 0x4bc670: what the race's view pass does as a pause ends.
u32 endPause(App* app, ThreadState& ts);

}  // namespace menud
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
