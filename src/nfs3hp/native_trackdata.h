#ifndef NFS3HP_NATIVE_TRACKDATA_H
#define NFS3HP_NATIVE_TRACKDATA_H

#include "native_guest.h"

/* native_trackdata.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  A place on the track is
 * its record of 0x78 bytes (native_trackdata.cpp says what it holds), a point three 16.16 words.
 * Each may call the game's other functions by address on `cpu`, and is false or nullopt when the
 * game stopped in one; one without `cpu` calls nothing. */
namespace nfs3hp
{
namespace trackdata
{

/* What a search for the road's quad leaves: 1 found or 0, and edx as the code leaves it. */
struct Searched
{
    x86::reg32 found;
    x86::reg32 scratch;
};

/* What 0x41f870 leaves: 1 when found on a polygon with a height, else 0; and edx as the code
 * leaves it. */
struct Matched
{
    x86::reg32 found;
    x86::reg32 scratch;
};

/* What 0x41fd60 leaves: the flags, and ecx as the code leaves it (the polygon where they are the
 * same, else the first two corners' y's sum). */
struct Edges
{
    x86::reg32 flags;
    x86::reg32 scratch;
};

/* What 0x41ff20 leaves: 0, or -1 past the items; the item's size as it went through bx (halved,
 * not doubled), and the items, which the code leaves in edx -- for an item taken. */
struct ItemTaken
{
    x86::reg32 result;
    x86::reg16 size;
    x86::reg32 items;
};

// The game's 0x41e370: the track's nodes from its header, the .ccm and simhts.dat loaded.
bool openTrack(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 header);
// The game's 0x41e3b0: the .ccm and simhts.dat freed.
bool closeTrack(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x41e420: the place moved to the node nearest the point; 1 where its node changed, else 0.
x86::reg32 followToNearest(win32::WinApplication* app, x86::reg32 point, x86::reg32 place);
// The game's 0x41e940: the place at node `node`, nothing found on it; the node's entry's offset.
x86::reg32 resetPlace(win32::WinApplication* app, x86::reg32 node, x86::reg32 place);
// The game's 0x41e990: the road's quad under the point looked for about the place's node.
std::optional<Searched> searchQuad(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 place);
// The game's 0x41f6e0: the place brought to the point, followed or not.
std::optional<x86::reg32> bringPlace(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 place,
                                     x86::reg32 followed);
// The game's 0x41f870: the place `from` copied to `to` and moved by `way` to the quad that matches.
Matched matchQuad(win32::WinApplication* app, x86::reg32 way, x86::reg32 from, x86::reg32 to);
// The game's 0x41fd60: which edges of `other`'s quad the point is past, its height to [height].
Edges edgesPast(win32::WinApplication* app, x86::reg32 point, x86::reg32 other, x86::reg32 place, x86::reg32 height);
// The game's 0x41ff10: the items of the place's block.
x86::reg32 itemCount(win32::WinApplication* app, x86::reg32 place);
// The game's 0x41ff20: item `index` of the place's block to `to`, its size's word to [size].
ItemTaken takeItem(win32::WinApplication* app, x86::reg32 place, x86::reg32 index, x86::reg32 to, x86::reg32 size);
// The game's 0x41ffa0: the lanes' span and shift.
void setLanes(win32::WinApplication* app, x86::reg32 span, x86::reg32 shift);
// The game's 0x420230: the place's frame to `axes` (nine words), its corner to `corner`; edx as the code leaves it.
x86::reg32 takeFrame(win32::WinApplication* app, x86::reg32 place, x86::reg32 axes, x86::reg32 corner);
// The game's 0x4205a0: the place (three floats) snapped to the road, the place on the track to `out`.
bool snapPlace(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 out, x86::reg32 followed,
               x86::reg32 node);
// The game's 0x420650: the point put on the track near node `node`, the place on the track to `out`.
bool placeAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 followed, x86::reg32 node,
             x86::reg32 out);
// The game's 0x4206f0: the light where the place is.
x86::reg32 placeColour(win32::WinApplication* app, x86::reg32 place, x86::reg32 followed);
// The game's 0x4207e0: the kind of the polygon the place is on.
x86::reg32 placeSurface(win32::WinApplication* app, x86::reg32 place, x86::reg32 followed);
// The game's 0x420840: the special ground under a car at node `node` across lane `lane`, or 0.
x86::reg32 groundUnder(win32::WinApplication* app, x86::sreg32 node, x86::sreg32 lane);
// The game's 0x420bf0: 1 where the place (three words at `place`) is in a tunnel by node `node`, else 0.
std::optional<x86::reg32> placeInTunnel(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 node);

// The game's 0x41ffd0: a place in a lane to `out` (16.16), from the place on the track, on the right where `right`; the lane's node.
x86::reg32 laneNodePlaced(win32::WinApplication* app, x86::reg32 place, x86::reg32 out, x86::reg32 right);
// The game's 0x420d60: whether the car is in a tunnel two nodes on (or back) from its node (1 or 0).
std::optional<x86::reg32> carAgainstTrack(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4211c0: the kind of the track at the car's node.
x86::reg32 trackKindUnderCar(win32::WinApplication* app, x86::reg32 car);

}  // namespace trackdata

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
