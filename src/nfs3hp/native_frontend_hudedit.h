#ifndef NFS3HP_NATIVE_FRONTEND_HUDEDIT_H
#define NFS3HP_NATIVE_FRONTEND_HUDEDIT_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudedit.cpp's typed functions (the HUD's screens: hud, and the race's pause
 * screens that drag it about), which other files call as C++ (guest::direct), and how the game's
 * code calls each by address.  Each calls the game's other
 * functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudedit
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x44de00: done's key handler: Enter keeps the places as they are.
reg32 doneKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x44de20: reset's key handler: Enter takes the defaults from disc.
reg32 resetKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x44de80: a pause screen of the HUD as it opens.
reg32 layoutEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44dfd0: a pause screen of the HUD as it closes.
reg32 layoutExit(App* app, CPU& cpu);
// The game's 0x45ca90: an element's item put where its rectangle has it.
reg32 placeElement(App* app, CPU& cpu, reg32 item);
// The game's 0x45dae0: the speedometer on hud's miniature, in its form.
reg32 showSpeedometer(App* app, CPU& cpu, reg32 player, reg32 cop, reg32 menu);
// The game's 0x45deb0: the tachometer on hud's miniature, in its form.
reg32 showTachometer(App* app, CPU& cpu, reg32 player, reg32 cop, reg32 menu);
// The game's 0x45e160: the rear view's buttons and lists greyed out.
reg32 greyMirrors(App* app, CPU& cpu, reg32 menu);
// The game's 0x45e210: a cop's opponents' names and split times greyed out.
reg32 greyOpponentNames(App* app, CPU& cpu, reg32 menu);
// The game's 0x45e310: the images in their places on hud's miniature.
reg32 placeImages(App* app, CPU& cpu, reg32 menu);
// The game's 0x45e3d0: only this player's and cop's controls shown, and their images.
reg32 showScreenItems(App* app, CPU& cpu, reg32 menu, reg32 player, reg32 cop);
// The game's 0x45e960: hud as it opens: the records kept, the miniature, the lists.
reg32 hudEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x45f160: hud as it closes: the records back, or the lists into them.
reg32 hudExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x45f300: the miniature's art by whose HUD is on.
reg32 setScreenArt(App* app, CPU& cpu, reg32 menu, reg32 player, reg32 cop);
// The game's 0x45f4c0: hud's every pass: the items for what has changed.
reg32 hudFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x485680: the rectangles' defaults for this language, from disc.
reg32 loadDefaults(App* app, CPU& cpu);

}  // namespace hudedit
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
