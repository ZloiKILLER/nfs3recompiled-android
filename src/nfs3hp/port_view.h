#ifndef NFS3HP_PORT_VIEW_H
#define NFS3HP_PORT_VIEW_H

#include "native_guest.h"
#include "port.h"

/* The pan-and-zoom view over the front end's and the pause screens' maps,
 * rewritten (port.h says how; port_view.cpp): the floats of a participant's
 * record that the HUD reads, the pan's base, the lap-metric popup over the
 * race's results, and the view's own pan, drawn grid and keys.
 *
 * This is the first of the x87-free layers: the arithmetic is the host's
 * doubles, rounded to a float where the game stores a float, not the x87 stack (the
 * game's single-precision race is not followed: only what is drawn sees it).
 * A thunk that gives back a float pushes it on the x87's stack as the game's
 * code does, since the game's callers still read it there.
 *
 * Each address is bound to the thunk of that name here
 * (PORT_VIEW_NATIVES): the game's code calls them by name.  The thunk
 * takes the arguments from the registers and the stack and gives back what the
 * game's code does -- the result in eax or on the x87's stack, every other
 * register as the caller had it (but where the code leaves one otherwise,
 * which is said at the function). */
namespace nfs3hp
{
namespace port
{
namespace view
{

/* The thunks, under the game's functions' names. */
x86::reg32 storeIncremented(App* app, CPU& cpu, x86::Args& args);    // 0x4646e0
x86::reg32 sectionAheadIsTwo(App* app, CPU& cpu, x86::Args& args);   // 0x420ff0
x86::reg32 edgeTriangles(App* app, CPU& cpu, x86::Args& args);       // 0x474db0
x86::reg32 glyphResource(App* app, CPU& cpu, x86::Args& args);        // 0x475240
x86::reg32 colourNearest(App* app, CPU& cpu, x86::Args& args);       // 0x4733c0
x86::reg32 gridDrawn(App* app, CPU& cpu, x86::Args& args);           // 0x464840
x86::reg32 viewFrame(App* app, CPU& cpu, x86::Args& args);           // 0x464fd0
x86::reg32 panKey(App* app, CPU& cpu, x86::Args& args);              // 0x4651e0
x86::reg32 recordFloat(App* app, CPU& cpu, x86::Args& args);         // 0x469580
x86::reg32 recordFloatAt(App* app, CPU& cpu, x86::Args& args);       // 0x46b030
x86::reg32 recordFloatByKind(App* app, CPU& cpu, x86::Args& args);   // 0x46ca70
x86::reg32 metricPopup(App* app, CPU& cpu, x86::Args& args);         // 0x46cf30

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x4646e0: the pan's base set to `value` plus one.
void storedIncremented(App* app, float value);
// The game's 0x46cf30: the lap-metric popup over the race's results for the record at `record` (eax back as it
// came); esp the frame base.
Result metricPopupOf(App* app, CPU& cpu, reg32 esp, reg32 record);
// The game's 0x474db0: the glyphs' mesh's two triangles for a face's edge (the entries in the low bytes of `eax`,
// `edx`, `ebx`, `ecx`) when the adjacency grid still marks it, the eight words its code is handed at `words`;
// false when the game stopped.  esp the frame base.
bool edgeTrianglesEmitted(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx,
                          const reg32* words);
// The game's 0x464840: the view's grid drawn in the box at x, y of width by height, the map's offset at `mapX`
// and `mapY`, its scale at `scale`; false when the game stopped.  esp the frame base.
bool gridDrawnAt(App* app, CPU& cpu, reg32 esp, reg32 x, reg32 y, reg32 width, reg32 height, reg32 mapX, reg32 mapY,
                 reg32 scale);

}  // namespace view
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
