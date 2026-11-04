#ifndef NFS3HP_PORT_VIEW_H
#define NFS3HP_PORT_VIEW_H

#include "port.h"

/* The pan-and-zoom view over the front end's and the pause screens' maps,
 * rewritten (port.h says how; port_view.cpp): the floats of a participant's
 * record that the HUD reads, the pan's base, the lap-metric popup over the
 * race's results, and the view's own pan, drawn grid and keys.
 *
 * This is the first of the x87-free layers: the arithmetic is the host's
 * doubles, rounded to a float where the game stores a float, not cpu.fpu (the
 * game's single-precision race is not followed: only what is drawn sees it).
 * An adapter that gives back a float pushes it on the x87's stack as the game's
 * code does, since the game's callers still read it there.
 *
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_VIEW_NATIVES): the generated code calls them by name.  The adapter
 * takes the arguments from the registers and the stack and gives back what the
 * game's code does -- the result in eax or on the x87's stack, every other
 * register as the caller had it (but where the code leaves one otherwise,
 * which is said at the function).  tools/game_checks holds each against the
 * game's code for it (view.inc). */
namespace nfs3hp
{
namespace port
{
namespace view
{

/* The adapters, under the game's functions' names. */
void storeIncremented(App* app, CPU& cpu);    // 0x4646e0
void sectionAheadIsTwo(App* app, CPU& cpu);   // 0x420ff0
void edgeTriangles(App* app, CPU& cpu);       // 0x474db0
void glyphResource(App* app, CPU& cpu);        // 0x475240
void colourNearest(App* app, CPU& cpu);       // 0x4733c0
void gridDrawn(App* app, CPU& cpu);           // 0x464840
void viewFrame(App* app, CPU& cpu);           // 0x464fd0
void panKey(App* app, CPU& cpu);              // 0x4651e0
void recordFloat(App* app, CPU& cpu);         // 0x469580
void recordFloatAt(App* app, CPU& cpu);       // 0x46b030
void recordFloatByKind(App* app, CPU& cpu);   // 0x46ca70
void metricPopup(App* app, CPU& cpu);         // 0x46cf30

}  // namespace view
}  // namespace port
}  // namespace nfs3hp

#endif
