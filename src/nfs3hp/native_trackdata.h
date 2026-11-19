#ifndef NFS3HP_NATIVE_TRACKDATA_H
#define NFS3HP_NATIVE_TRACKDATA_H

#include "native_guest.h"

/* native_trackdata.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  A place on the track is
 * its record of 0x78 bytes (native_trackdata.cpp says what it holds), a point three 16.16 words.
 * Each may call the game's other functions by address on `ts`, and is false or nullopt when the
 * game stopped in one; one without `ts` calls nothing. */
namespace nfs3hp
{
namespace trackdata
{

/* What a search for the road's quad leaves: 1 found or 0, and edx as the code leaves it. */
struct Searched
{
    nfs::u32 found;
    nfs::u32 scratch;
};

/* What 0x41f870 leaves: 1 when found on a polygon with a height, else 0; and edx as the code
 * leaves it. */
struct Matched
{
    nfs::u32 found;
    nfs::u32 scratch;
};

/* What 0x41fd60 leaves: the flags, and ecx as the code leaves it (the polygon where they are the
 * same, else the first two corners' y's sum). */
struct Edges
{
    nfs::u32 flags;
    nfs::u32 scratch;
};

/* What 0x41ff20 leaves: 0, or -1 past the items; the item's size as it went through bx (halved,
 * not doubled), and the items, which the code leaves in edx -- for an item taken. */
struct ItemTaken
{
    nfs::u32 result;
    nfs::u16 size;
    nfs::u32 items;
};

// The game's 0x41e370: the track's nodes from its header, the .ccm and simhts.dat loaded.
bool openTrack(nfs::App* app, nfs::ThreadState& ts, nfs::u32 header);
// The game's 0x41e3b0: the .ccm and simhts.dat freed.
bool closeTrack(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x41e420: the place moved to the node nearest the point; 1 where its node changed, else 0.
nfs::u32 followToNearest(nfs::App* app, nfs::u32 point, nfs::u32 place);
// The game's 0x41e940: the place at node `node`, nothing found on it; the node's entry's offset.
nfs::u32 resetPlace(nfs::App* app, nfs::u32 node, nfs::u32 place);
// The game's 0x41e990: the road's quad under the point looked for about the place's node.
std::optional<Searched> searchQuad(nfs::App* app, nfs::ThreadState& ts, nfs::u32 point, nfs::u32 place);
// The game's 0x41f6e0: the place brought to the point, followed or not.
std::optional<nfs::u32> bringPlace(nfs::App* app, nfs::ThreadState& ts, nfs::u32 point, nfs::u32 place,
                                     nfs::u32 followed);
// The game's 0x41f870: the place `from` copied to `to` and moved by `way` to the quad that matches.
Matched matchQuad(nfs::App* app, nfs::u32 way, nfs::u32 from, nfs::u32 to);
// The game's 0x41fd60: which edges of `other`'s quad the point is past, its height to [height].
Edges edgesPast(nfs::App* app, nfs::u32 point, nfs::u32 other, nfs::u32 place, nfs::u32 height);
// The game's 0x41ff10: the items of the place's block.
nfs::u32 itemCount(nfs::App* app, nfs::u32 place);
// The game's 0x41ff20: item `index` of the place's block to `to`, its size's word to [size].
ItemTaken takeItem(nfs::App* app, nfs::u32 place, nfs::u32 index, nfs::u32 to, nfs::u32 size);
// The game's 0x41ffa0: the lanes' span and shift.
void setLanes(nfs::App* app, nfs::u32 span, nfs::u32 shift);
// The game's 0x420230: the place's frame to `axes` (nine words), its corner to `corner`; edx as the code leaves it.
nfs::u32 takeFrame(nfs::App* app, nfs::u32 place, nfs::u32 axes, nfs::u32 corner);
// The game's 0x4205a0: the place (three floats) snapped to the road, the place on the track to `out`.
bool snapPlace(nfs::App* app, nfs::ThreadState& ts, nfs::u32 place, nfs::u32 out, nfs::u32 followed,
               nfs::u32 node);
// The game's 0x420650: the point put on the track near node `node`, the place on the track to `out`.
bool placeAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 point, nfs::u32 followed, nfs::u32 node,
             nfs::u32 out);
// The game's 0x4206f0: the light where the place is.
nfs::u32 placeColour(nfs::App* app, nfs::u32 place, nfs::u32 followed);
// The game's 0x4207e0: the kind of the polygon the place is on.
nfs::u32 placeSurface(nfs::App* app, nfs::u32 place, nfs::u32 followed);
// The game's 0x420840: the special ground under a car at node `node` across lane `lane`, or 0.
nfs::u32 groundUnder(nfs::App* app, nfs::s32 node, nfs::s32 lane);
// The game's 0x420bf0: 1 where the place (three words at `place`) is in a tunnel by node `node`, else 0.
std::optional<nfs::u32> placeInTunnel(nfs::App* app, nfs::ThreadState& ts, nfs::u32 place, nfs::u32 node);

// The game's 0x41ffd0: a place in a lane to `out` (16.16), from the place on the track, on the right where `right`; the lane's node.
nfs::u32 laneNodePlaced(nfs::App* app, nfs::u32 place, nfs::u32 out, nfs::u32 right);
// The game's 0x420d60: whether the car is in a tunnel two nodes on (or back) from its node (1 or 0).
std::optional<nfs::u32> carAgainstTrack(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4211c0: the kind of the track at the car's node.
nfs::u32 trackKindUnderCar(nfs::App* app, nfs::u32 car);

}  // namespace trackdata

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
