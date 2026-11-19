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
nfs::u32 playMenuEffect(App* app, ThreadState& ts, nfs::Args& args);  // 0x4181d0
namespace shapes
{
nfs::u32 shapeByName(App* app, ThreadState& ts, nfs::Args& args);     // 0x4d5db0
nfs::u32 shapeSize(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d66f0
nfs::u32 drawArtShape(App* app, ThreadState& ts, nfs::Args& args);    // 0x4d7160
nfs::u32 drawShape(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d7990
}  // namespace shapes

namespace buttons
{

/* The thunks, under the game's functions' names. */
nfs::u32 joinListedGame(App* app, ThreadState& ts, nfs::Args& args);       // 0x43fdd0
nfs::u32 drawReadyMarks(App* app, ThreadState& ts, nfs::Args& args);       // 0x440c40
nfs::u32 networkLeft(App* app, ThreadState& ts, nfs::Args& args);    // 0x440d20
nfs::u32 drawLicence(App* app, ThreadState& ts, nfs::Args& args);          // 0x442450
nfs::u32 listColours(App* app, ThreadState& ts, nfs::Args& args);          // 0x442b90
nfs::u32 listAllowed(App* app, ThreadState& ts, nfs::Args& args);          // 0x442bf0
nfs::u32 listLength(App* app, ThreadState& ts, nfs::Args& args);           // 0x442c50
nfs::u32 findList(App* app, ThreadState& ts, nfs::Args& args);             // 0x442c80
nfs::u32 menuEdited(App* app, ThreadState& ts, nfs::Args& args);           // 0x442d00
nfs::u32 menuRecordEdited(App* app, ThreadState& ts, nfs::Args& args);          // 0x442d20
nfs::u32 loadShapes(App* app, ThreadState& ts, nfs::Args& args);           // 0x44ac90
nfs::u32 redrawMenu(App* app, ThreadState& ts, nfs::Args& args);           // 0x44b170
nfs::u32 textBoxDraw(App* app, ThreadState& ts, nfs::Args& args);          // 0x455e30
nfs::u32 textBoxKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x456020
nfs::u32 textBoxUp(App* app, ThreadState& ts, nfs::Args& args);            // 0x456110
nfs::u32 textBoxShown(App* app, ThreadState& ts, nfs::Args& args);         // 0x456170
nfs::u32 textBoxHidden(App* app, ThreadState& ts, nfs::Args& args);        // 0x4561d0
nfs::u32 slideTrackMap(App* app, ThreadState& ts, nfs::Args& args);        // 0x456320
nfs::u32 drawTrackFacts(App* app, ThreadState& ts, nfs::Args& args);       // 0x456590
nfs::u32 drawGlow(App* app, ThreadState& ts, nfs::Args& args);             // 0x456670
nfs::u32 trackMapShape(App* app, ThreadState& ts, nfs::Args& args);        // 0x456800
nfs::u32 rectDraw(App* app, ThreadState& ts, nfs::Args& args);             // 0x45fae0
nfs::u32 speechDraw(App* app, ThreadState& ts, nfs::Args& args);           // 0x45fc90
nfs::u32 speechShown(App* app, ThreadState& ts, nfs::Args& args);          // 0x45fca0
nfs::u32 speechHidden(App* app, ThreadState& ts, nfs::Args& args);         // 0x45fcb0
nfs::u32 textDraw(App* app, ThreadState& ts, nfs::Args& args);             // 0x45fcd0
nfs::u32 textUp(App* app, ThreadState& ts, nfs::Args& args);               // 0x460070
nfs::u32 textShown(App* app, ThreadState& ts, nfs::Args& args);            // 0x4600b0
nfs::u32 imageDraw(App* app, ThreadState& ts, nfs::Args& args);            // 0x460100
nfs::u32 imageUp(App* app, ThreadState& ts, nfs::Args& args);              // 0x460410
nfs::u32 imageShown(App* app, ThreadState& ts, nfs::Args& args);           // 0x460440
nfs::u32 pulseOn(App* app, ThreadState& ts, nfs::Args& args);              // 0x460700
nfs::u32 pulseColour(App* app, ThreadState& ts, nfs::Args& args);          // 0x460770
nfs::u32 buttonDraw(App* app, ThreadState& ts, nfs::Args& args);           // 0x4607c0
nfs::u32 buttonKey(App* app, ThreadState& ts, nfs::Args& args);            // 0x461270
nfs::u32 layoutMenuItem(App* app, ThreadState& ts, nfs::Args& args);       // 0x4613e0
nfs::u32 buttonShown(App* app, ThreadState& ts, nfs::Args& args);          // 0x461900
nfs::u32 cascadeLayoutClosed(App* app, ThreadState& ts, nfs::Args& args);  // 0x461a70
nfs::u32 cascadeLineText(App* app, ThreadState& ts, nfs::Args& args);      // 0x461aa0
nfs::u32 cascadeDraw(App* app, ThreadState& ts, nfs::Args& args);          // 0x461d00
nfs::u32 cascadePass(App* app, ThreadState& ts, nfs::Args& args);          // 0x462320
nfs::u32 cascadeShown(App* app, ThreadState& ts, nfs::Args& args);         // 0x4623a0
nfs::u32 cascadeKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x4624a0
nfs::u32 cascadeUp(App* app, ThreadState& ts, nfs::Args& args);            // 0x4628d0
nfs::u32 checkboxDraw(App* app, ThreadState& ts, nfs::Args& args);         // 0x4629c0
nfs::u32 checkboxKey(App* app, ThreadState& ts, nfs::Args& args);          // 0x462ad0
nfs::u32 checkboxShown(App* app, ThreadState& ts, nfs::Args& args);        // 0x462b20
nfs::u32 checkboxUp(App* app, ThreadState& ts, nfs::Args& args);           // 0x462b30
nfs::u32 boxOverlayDraw(App* app, ThreadState& ts, nfs::Args& args);       // 0x462b90
nfs::u32 selectListShown(App* app, ThreadState& ts, nfs::Args& args);      // 0x462c30
nfs::u32 selectListHidden(App* app, ThreadState& ts, nfs::Args& args);     // 0x462ce0
nfs::u32 selectListDraw(App* app, ThreadState& ts, nfs::Args& args);       // 0x462d20
nfs::u32 selectListPass(App* app, ThreadState& ts, nfs::Args& args);       // 0x462f90
nfs::u32 selectListKey(App* app, ThreadState& ts, nfs::Args& args);        // 0x462fa0
nfs::u32 selectListUp(App* app, ThreadState& ts, nfs::Args& args);         // 0x4633d0
nfs::u32 dropListDraw(App* app, ThreadState& ts, nfs::Args& args);         // 0x4634d0
nfs::u32 dropListPass(App* app, ThreadState& ts, nfs::Args& args);         // 0x463de0
nfs::u32 dropListShown(App* app, ThreadState& ts, nfs::Args& args);        // 0x463e60
nfs::u32 listKey(App* app, ThreadState& ts, nfs::Args& args);              // 0x463ff0
nfs::u32 layoutList(App* app, ThreadState& ts, nfs::Args& args);           // 0x464560
nfs::u32 drawTurnedShape(App* app, ThreadState& ts, nfs::Args& args);      // 0x4d6730
nfs::u32 drawColouredRectangle(App* app, ThreadState& ts, nfs::Args& args);  // 0x4d88f0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x44ac90: the HUD's shapes of the .fsh named at `name` loaded and kept (`ebx` handed on); esp the frame base.
Result shapesLoaded(App* app, ThreadState& ts, u32 esp, u32 name, u32 ebx);
// The game's 0x44b170: the screen drawn again and shown, `in` the caller's registers handed on; esp the frame base.
Result redrawn(App* app, ThreadState& ts, u32 esp, const Args& in);
// The game's 0x456800: the track map's tile `index` made shape `shape`'s textures: their record; esp the frame base.
Result mapShape(App* app, ThreadState& ts, u32 esp, u32 shape, int32_t index);
// The game's 0x461270: key `key` for the button item at `at`: its answer; esp the frame base.
Result buttonKeyed(App* app, ThreadState& ts, u32 esp, u32 at, u32 key);
// The game's 0x4633d0: the select list item at `at` laid out.
void selectLaidOut(App* app, u32 at);
// The game's 0x4d6730: menu shape `shape` drawn turned by `angle`, scaled by `scaleBits` (a float's bits) about x, y,
// `nudge` to the left; esp the frame base.
Result turnedShape(App* app, ThreadState& ts, u32 esp, int32_t shape, int32_t x, int32_t y, int32_t nudge, int32_t angle,
                   u32 scaleBits);
// The game's 0x4d88f0: the rectangle a.eax, a.edx to a.ebx, a.ecx, `colours` at its corners; esp the frame base.
Result colouredRectangle(App* app, ThreadState& ts, u32 esp, const Args& a, const u32 colours[4]);

}  // namespace buttons
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
