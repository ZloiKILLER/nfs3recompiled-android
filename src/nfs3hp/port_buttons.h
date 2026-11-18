#ifndef NFS3HP_PORT_BUTTONS_H
#define NFS3HP_PORT_BUTTONS_H

#include "native_guest.h"
#include "port.h"

/* The menus' widgets, rewritten (port.h says how; port_buttons.cpp): what a
 * screen's text, image, button, cascade, drop list, select list, check box,
 * box overlay, speech, rectangle and text box do as they are shown, drawn,
 * keyed, laid out and hidden -- and what those call of their own: the lists
 * by name, the HUD's art loaded, the ready marks of a network race, the
 * track's map and its sliding glow, a shape drawn turned.
 *
 * Each function here is a thunk under the game's name: the widgets' table (0x557a84) and the game's code
 * reach them through their addresses, and the game's code calls them by
 * name.  The thunk takes the arguments from the registers and the stack and
 * gives back the result in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
/* What the widgets call of the port's own that its headers do not declare,
 * called as the game's code calls it -- declared here, not in
 * port_buttons.cpp: port_sound_effects.cpp's menu effect,
 * port_shapes.cpp's thunks. */
x86::reg32 playMenuEffect(App* app, CPU& cpu, x86::Args& args);  // 0x4181d0
namespace shapes
{
x86::reg32 shapeByName(App* app, CPU& cpu, x86::Args& args);     // 0x4d5db0
x86::reg32 shapeSize(App* app, CPU& cpu, x86::Args& args);       // 0x4d66f0
x86::reg32 drawArtShape(App* app, CPU& cpu, x86::Args& args);    // 0x4d7160
x86::reg32 drawShape(App* app, CPU& cpu, x86::Args& args);       // 0x4d7990
}  // namespace shapes

namespace buttons
{

/* The thunks, under the game's functions' names. */
x86::reg32 joinListedGame(App* app, CPU& cpu, x86::Args& args);       // 0x43fdd0
x86::reg32 drawReadyMarks(App* app, CPU& cpu, x86::Args& args);       // 0x440c40
x86::reg32 networkLeft(App* app, CPU& cpu, x86::Args& args);    // 0x440d20
x86::reg32 drawLicence(App* app, CPU& cpu, x86::Args& args);          // 0x442450
x86::reg32 listColours(App* app, CPU& cpu, x86::Args& args);          // 0x442b90
x86::reg32 listAllowed(App* app, CPU& cpu, x86::Args& args);          // 0x442bf0
x86::reg32 listLength(App* app, CPU& cpu, x86::Args& args);           // 0x442c50
x86::reg32 findList(App* app, CPU& cpu, x86::Args& args);             // 0x442c80
x86::reg32 menuEdited(App* app, CPU& cpu, x86::Args& args);           // 0x442d00
x86::reg32 menuRecordEdited(App* app, CPU& cpu, x86::Args& args);          // 0x442d20
x86::reg32 loadShapes(App* app, CPU& cpu, x86::Args& args);           // 0x44ac90
x86::reg32 redrawMenu(App* app, CPU& cpu, x86::Args& args);           // 0x44b170
x86::reg32 textBoxDraw(App* app, CPU& cpu, x86::Args& args);          // 0x455e30
x86::reg32 textBoxKey(App* app, CPU& cpu, x86::Args& args);           // 0x456020
x86::reg32 textBoxUp(App* app, CPU& cpu, x86::Args& args);            // 0x456110
x86::reg32 textBoxShown(App* app, CPU& cpu, x86::Args& args);         // 0x456170
x86::reg32 textBoxHidden(App* app, CPU& cpu, x86::Args& args);        // 0x4561d0
x86::reg32 slideTrackMap(App* app, CPU& cpu, x86::Args& args);        // 0x456320
x86::reg32 drawTrackFacts(App* app, CPU& cpu, x86::Args& args);       // 0x456590
x86::reg32 drawGlow(App* app, CPU& cpu, x86::Args& args);             // 0x456670
x86::reg32 trackMapShape(App* app, CPU& cpu, x86::Args& args);        // 0x456800
x86::reg32 rectDraw(App* app, CPU& cpu, x86::Args& args);             // 0x45fae0
x86::reg32 speechDraw(App* app, CPU& cpu, x86::Args& args);           // 0x45fc90
x86::reg32 speechShown(App* app, CPU& cpu, x86::Args& args);          // 0x45fca0
x86::reg32 speechHidden(App* app, CPU& cpu, x86::Args& args);         // 0x45fcb0
x86::reg32 textDraw(App* app, CPU& cpu, x86::Args& args);             // 0x45fcd0
x86::reg32 textUp(App* app, CPU& cpu, x86::Args& args);               // 0x460070
x86::reg32 textShown(App* app, CPU& cpu, x86::Args& args);            // 0x4600b0
x86::reg32 imageDraw(App* app, CPU& cpu, x86::Args& args);            // 0x460100
x86::reg32 imageUp(App* app, CPU& cpu, x86::Args& args);              // 0x460410
x86::reg32 imageShown(App* app, CPU& cpu, x86::Args& args);           // 0x460440
x86::reg32 pulseOn(App* app, CPU& cpu, x86::Args& args);              // 0x460700
x86::reg32 pulseColour(App* app, CPU& cpu, x86::Args& args);          // 0x460770
x86::reg32 buttonDraw(App* app, CPU& cpu, x86::Args& args);           // 0x4607c0
x86::reg32 buttonKey(App* app, CPU& cpu, x86::Args& args);            // 0x461270
x86::reg32 layoutMenuItem(App* app, CPU& cpu, x86::Args& args);       // 0x4613e0
x86::reg32 buttonShown(App* app, CPU& cpu, x86::Args& args);          // 0x461900
x86::reg32 cascadeLayoutClosed(App* app, CPU& cpu, x86::Args& args);  // 0x461a70
x86::reg32 cascadeLineText(App* app, CPU& cpu, x86::Args& args);      // 0x461aa0
x86::reg32 cascadeDraw(App* app, CPU& cpu, x86::Args& args);          // 0x461d00
x86::reg32 cascadePass(App* app, CPU& cpu, x86::Args& args);          // 0x462320
x86::reg32 cascadeShown(App* app, CPU& cpu, x86::Args& args);         // 0x4623a0
x86::reg32 cascadeKey(App* app, CPU& cpu, x86::Args& args);           // 0x4624a0
x86::reg32 cascadeUp(App* app, CPU& cpu, x86::Args& args);            // 0x4628d0
x86::reg32 checkboxDraw(App* app, CPU& cpu, x86::Args& args);         // 0x4629c0
x86::reg32 checkboxKey(App* app, CPU& cpu, x86::Args& args);          // 0x462ad0
x86::reg32 checkboxShown(App* app, CPU& cpu, x86::Args& args);        // 0x462b20
x86::reg32 checkboxUp(App* app, CPU& cpu, x86::Args& args);           // 0x462b30
x86::reg32 boxOverlayDraw(App* app, CPU& cpu, x86::Args& args);       // 0x462b90
x86::reg32 selectListShown(App* app, CPU& cpu, x86::Args& args);      // 0x462c30
x86::reg32 selectListHidden(App* app, CPU& cpu, x86::Args& args);     // 0x462ce0
x86::reg32 selectListDraw(App* app, CPU& cpu, x86::Args& args);       // 0x462d20
x86::reg32 selectListPass(App* app, CPU& cpu, x86::Args& args);       // 0x462f90
x86::reg32 selectListKey(App* app, CPU& cpu, x86::Args& args);        // 0x462fa0
x86::reg32 selectListUp(App* app, CPU& cpu, x86::Args& args);         // 0x4633d0
x86::reg32 dropListDraw(App* app, CPU& cpu, x86::Args& args);         // 0x4634d0
x86::reg32 dropListPass(App* app, CPU& cpu, x86::Args& args);         // 0x463de0
x86::reg32 dropListShown(App* app, CPU& cpu, x86::Args& args);        // 0x463e60
x86::reg32 listKey(App* app, CPU& cpu, x86::Args& args);              // 0x463ff0
x86::reg32 layoutList(App* app, CPU& cpu, x86::Args& args);           // 0x464560
x86::reg32 drawTurnedShape(App* app, CPU& cpu, x86::Args& args);      // 0x4d6730
x86::reg32 drawColouredRectangle(App* app, CPU& cpu, x86::Args& args);  // 0x4d88f0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x44ac90: the HUD's shapes of the .fsh named at `name` loaded and kept (`ebx` handed on); esp the frame base.
Result shapesLoaded(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 ebx);
// The game's 0x44b170: the screen drawn again and shown, `in` the caller's registers handed on; esp the frame base.
Result redrawn(App* app, CPU& cpu, reg32 esp, const Args& in);
// The game's 0x456800: the track map's tile `index` made shape `shape`'s textures: their record; esp the frame base.
Result mapShape(App* app, CPU& cpu, reg32 esp, reg32 shape, int32_t index);
// The game's 0x461270: key `key` for the button item at `at`: its answer; esp the frame base.
Result buttonKeyed(App* app, CPU& cpu, reg32 esp, reg32 at, reg32 key);
// The game's 0x4633d0: the select list item at `at` laid out.
void selectLaidOut(App* app, reg32 at);
// The game's 0x4d6730: menu shape `shape` drawn turned by `angle`, scaled by `scaleBits` (a float's bits) about x, y,
// `nudge` to the left; esp the frame base.
Result turnedShape(App* app, CPU& cpu, reg32 esp, int32_t shape, int32_t x, int32_t y, int32_t nudge, int32_t angle,
                   reg32 scaleBits);
// The game's 0x4d88f0: the rectangle a.eax, a.edx to a.ebx, a.ecx, `colours` at its corners; esp the frame base.
Result colouredRectangle(App* app, CPU& cpu, reg32 esp, const Args& a, const reg32 colours[4]);

}  // namespace buttons
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
