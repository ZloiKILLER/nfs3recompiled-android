#ifndef NFS3HP_NATIVE_FRONTEND_MENUF_H
#define NFS3HP_NATIVE_FRONTEND_MENUF_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menuf.cpp's typed functions (the HUD editor's element, the front end's start, a
 * screen's effects, a pause's and a race's small ones), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menuf
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x45b940: the grip the pointer has on the HUD element under it, and the cursor to show.
u32 pickResizeCursor(App* app, ThreadState& ts, u32 element);
// The game's 0x45bcb0: a HUD element each frame of the editor: its drag, its contents, its frame.
u32 drawElement(App* app, ThreadState& ts, u32 element, u32 dragging);
// The game's 0x45bd20: a HUD element's keys (eax the element, edx the screen, bx the key).
u32 elementKey(App* app, ThreadState& ts, u32 element, u32 menu, std::uint16_t key);
// The game's 0x45c9d0: where a HUD element stands in pixels, to its layout record's 640 by 480 units.
u32 elementToRecord(App* app, ThreadState& ts, u32 element);
// The game's 0x4730d0: the front end's start: the title, a new player's settings, the menu loop.
u32 frontEndStart(App* app, ThreadState& ts);
// The game's 0x478000: one of the things a race's end calls, empty: eax as it came.
u32 raceEndEmpty(App* app, ThreadState& ts);
// The game's 0x4a4a10: an effects item as its screen opens: the band's colour, the item's texture.
u32 startScreenEffects(App* app, ThreadState& ts, u32 item, u32 keepArt);
// The game's 0x4bc530: the race's mouse read afresh as a pause ends.
u32 resetRaceMouse(App* app, ThreadState& ts);
// The game's 0x4bc660: a race's view pass, pausing again: the pause menu.
u32 pauseMenuAgain(App* app, ThreadState& ts);

}  // namespace menuf
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
