#ifndef NFS3HP_NATIVE_COLLIDE_H
#define NFS3HP_NATIVE_COLLIDE_H

#include "native_guest.h"

/* native_collide.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `cpu`
 * call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one. */
namespace nfs3hp
{
namespace collide
{

// The game's 0x4311d0 (a car's +0x618): car's collisions for the frame; what the code leaves in eax.
std::optional<x86::reg32> frameCollisions(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x431280: of the points it searches, the value of the one nearest (x, y, z).
x86::reg32 nearest(win32::WinApplication* app, x86::reg32 x, x86::reg32 y, x86::reg32 z);
// nearest, under the name the game's callers take it by.
inline x86::reg32 nearestValue(win32::WinApplication* app, x86::reg32 x, x86::reg32 y, x86::reg32 z)
{
    return nearest(app, x, y, z);
}
// The game's 0x431320: the track's COLL file named at `name` opened and its header checked: its handle, or 0.
std::optional<x86::reg32> openColl(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x4313b0: the COLL file `handle` closed; what the close gives back.
std::optional<x86::reg32> closeColl(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle);
// The game's 0x4313d0: the COLL file's size, its header's.
x86::reg32 collSize(win32::WinApplication* app);
// The game's 0x4313e0: the COLL file `handle` past its header read into `to`.
bool readColl(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 to);
// The game's 0x431410: the COLL file's chunks' count, its header's.
x86::reg32 collCount(win32::WinApplication* app);
// The game's 0x431420: chunk `chunk` of the COLL file's data at `data`.
x86::reg32 collChunkAt(win32::WinApplication* app, x86::reg32 data, x86::reg32 chunk);
// The game's 0x431430 (a car's +0x600): the player's car's controls for the frame; what the code leaves in eax.
std::optional<x86::reg32> controls(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4318c0: the cached render states forgotten.
void forgetStates(win32::WinApplication* app);
// The game's 0x431aa0: the render states for 2D text.
bool textStates(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x431b40: the render states for the race's HUD.
bool hudStates(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x431bc0: the render states for the 2D screens.
bool screenStates(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x431c20: a view pass's layer that draws nothing (count quads, of layer `layer`).
void layerNothing(win32::WinApplication* app, x86::reg32 count, x86::reg32 layer);
// The game's 0x431c30: the main pass's second layer: nothing.
void secondLayerNothing(win32::WinApplication* app, x86::reg32 layer);
// The game's 0x431c40: the main pass's first layer: nothing.
void firstLayerNothing(win32::WinApplication* app, x86::reg32 layer);
// The game's 0x431c50: the software renderer's blend tables; the last quotient, as the code leaves it.
x86::reg32 fillBlendTables(win32::WinApplication* app);
// The game's 0x431d50: the span at `span` blended into the screen.
void blendInto(win32::WinApplication* app, x86::reg32 span);
// The game's 0x432690: how finely the quad of corners a, b, c, d is subdivided (0, 1 or 2).
x86::reg32 subdivisions(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d);
// The game's 0x433530: the textured triangles from `record` on drawn; the next record.
std::optional<x86::reg32> texturedTriangles(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x433660 and 0x4339d0: the flat triangle at `record` drawn; the next record.
std::optional<x86::reg32> flatTriangle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x4336e0: the triangles from `record` on drawn, by z for their flag; the next record.
std::optional<x86::reg32> trianglesByZ(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x433840: the quads from `record` on drawn, by z for their flag; the next record.
std::optional<x86::reg32> quadsByZ(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x4347b0: the triangles with colours of their own from `record` on drawn; the next record.
std::optional<x86::reg32> ownColourTriangles(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x434850: the render state of the record at `record` set; the next record.
std::optional<x86::reg32> stateFromRecord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x434a90: the race's fog colour set.
bool fogColour(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x436c40: the race's messages opened from the settings at `settings`; the copy's block.
std::optional<x86::reg32> openChannels(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 settings);
// The game's 0x436cf0: the race's messages closed, their blocks freed; what the code leaves in eax
// (`eax` where there is nothing to free).
std::optional<x86::reg32> closeChannels(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax);
// The game's 0x436d40: every channel of the messages cleared; the messages' block.
x86::reg32 clearAllChannels(win32::WinApplication* app);
// The game's 0x436e10: the text at `text` added to channel `channel`: 1, or 0.
x86::reg32 addText(win32::WinApplication* app, x86::reg32 channel, x86::reg32 text);
// addText, under the name the game's callers take it by.
inline x86::reg32 addTextResult(win32::WinApplication* app, x86::reg32 channel, x86::reg32 text)
{
    return addText(app, channel, text);
}

// The game's 0x431af0: the render states for pictures.
bool picturesStated(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x437080: channel's lines copied; the copy.
std::optional<x86::reg32> messageLinesCopied(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 channel);
// The game's 0x434990: the race's text for the frame; eax as it leaves it.
std::optional<x86::reg32> raceTextOfFrame(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x434a70: the frames drawn counted (eax is left as it was).
void frameCounted(win32::WinApplication* app);
// The game's 0x434b00: the drawing made ready at the race's start; false when the game stopped.
bool raceStartPrepared(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x430cd0: no car tested yet this frame.
void carListCleared(win32::WinApplication* app);
// The game's 0x4349a0: whether the view is drawn mirrored, kept; that.
x86::reg32 setViewMirrored(win32::WinApplication* app, x86::reg32 view);
// The game's 0x4349d0: setViewMirrored for the view, then the polygon list at `list` drawn.
bool listDrawnMirrored(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 list);
// The game's 0x4371d0: the hash's seed from `seed`; the odd three times it.
x86::reg32 seedTheHash(win32::WinApplication* app, x86::reg32 seed);
}  // namespace collide

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
