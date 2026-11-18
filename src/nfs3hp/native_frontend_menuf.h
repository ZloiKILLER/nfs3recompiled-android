#ifndef NFS3HP_NATIVE_FRONTEND_MENUF_H
#define NFS3HP_NATIVE_FRONTEND_MENUF_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menuf.cpp's typed functions (the HUD editor's element, the front end's start, a
 * screen's effects, a pause's and a race's small ones), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menuf
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45b940: the grip the pointer has on the HUD element under it, and the cursor to show.
reg32 pickResizeCursor(App* app, CPU& cpu, reg32 element);
// The game's 0x45bcb0: a HUD element each frame of the editor: its drag, its contents, its frame.
reg32 drawElement(App* app, CPU& cpu, reg32 element, reg32 dragging);
// The game's 0x45bd20: a HUD element's keys (eax the element, edx the screen, bx the key).
reg32 elementKey(App* app, CPU& cpu, reg32 element, reg32 menu, std::uint16_t key);
// The game's 0x45c9d0: where a HUD element stands in pixels, to its layout record's 640 by 480 units.
reg32 elementToRecord(App* app, CPU& cpu, reg32 element);
// The game's 0x4730d0: the front end's start: the title, a new player's settings, the menu loop.
reg32 frontEndStart(App* app, CPU& cpu);
// The game's 0x478000: one of the things a race's end calls, empty: eax as it came.
reg32 raceEndEmpty(App* app, CPU& cpu);
// The game's 0x4a4a10: an effects item as its screen opens: the band's colour, the item's texture.
reg32 startScreenEffects(App* app, CPU& cpu, reg32 item, reg32 keepArt);
// The game's 0x4bc530: the race's mouse read afresh as a pause ends.
reg32 resetRaceMouse(App* app, CPU& cpu);
// The game's 0x4bc660: a race's view pass, pausing again: the pause menu.
reg32 pauseMenuAgain(App* app, CPU& cpu);

}  // namespace menuf
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
