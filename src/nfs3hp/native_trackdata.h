#ifndef NFS3HP_NATIVE_TRACKDATA_H
#define NFS3HP_NATIVE_TRACKDATA_H

#include "native_guest.h"

/* native_trackdata.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  A place on the track is
 * its record of 0x78 bytes (native_trackdata.cpp says what it holds), a point three 16.16 words.
 * Each may call the game's other functions by address on `cpu`, and is false or nullopt when the
 * game stopped in one; one without `cpu` calls nothing, and through ByAddress leaves cpu.terminate
 * to say it. */
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
bool bringPlace(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 place, x86::reg32 followed);
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

}  // namespace trackdata

namespace guest
{

// The header in eax.
template <>
struct ByAddress<trackdata::openTrack>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 header)
    {
        return watcom(app, cpu, Address, {header});
    }
};

template <>
struct ByAddress<trackdata::closeTrack>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// The point in eax, the place in edx.
template <>
struct ByAddress<trackdata::followToNearest>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 place)
    {
        watcom(app, cpu, Address, {point, place});
        return cpu.eax;
    }
};

// The node in eax, the place in edx.
template <>
struct ByAddress<trackdata::resetPlace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 place)
    {
        watcom(app, cpu, Address, {node, place});
        return cpu.eax;
    }
};

// The point in eax, the place in edx; found in eax, the rest in edx.
template <>
struct ByAddress<trackdata::searchQuad>
{
    template <x86::reg32 Address>
    static std::optional<trackdata::Searched> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point,
                                                    x86::reg32 place)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = point;
        cpu.edx = place;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return trackdata::Searched{cpu.eax, cpu.edx};
    }
};

// The point in eax, the place in edx, followed in ebx.
template <>
struct ByAddress<trackdata::bringPlace>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 place,
                     x86::reg32 followed)
    {
        return watcom(app, cpu, Address, {point, place, followed});
    }
};

// The way in eax, from in edx, to in ebx; found in eax, the rest in edx.
template <>
struct ByAddress<trackdata::matchQuad>
{
    template <x86::reg32 Address>
    static trackdata::Matched call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 way, x86::reg32 from,
                                   x86::reg32 to)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = way;
        cpu.edx = from;
        cpu.ebx = to;
        guest::call(app, cpu, Address);
        return trackdata::Matched{cpu.eax, cpu.edx};
    }
};

// The point in eax, the other in edx, the place in ebx, the height's address in ecx; the flags in
// eax (and edx and ebx), the rest in ecx.
template <>
struct ByAddress<trackdata::edgesPast>
{
    template <x86::reg32 Address>
    static trackdata::Edges call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 other,
                                 x86::reg32 place, x86::reg32 height)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = point;
        cpu.edx = other;
        cpu.ebx = place;
        cpu.ecx = height;
        guest::call(app, cpu, Address);
        return trackdata::Edges{cpu.eax, cpu.ecx};
    }
};

// The place in eax.
template <>
struct ByAddress<trackdata::itemCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place)
    {
        watcom(app, cpu, Address, {place});
        return cpu.eax;
    }
};

// The place in eax, the index in edx, to in ebx, the size's address in ecx; the result in eax, and
// for an item taken the size in bx and the items in edx.
template <>
struct ByAddress<trackdata::takeItem>
{
    template <x86::reg32 Address>
    static trackdata::ItemTaken call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 index,
                                     x86::reg32 to, x86::reg32 size)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = place;
        cpu.edx = index;
        cpu.ebx = to;
        cpu.ecx = size;
        guest::call(app, cpu, Address);
        if (cpu.eax != 0)
            return trackdata::ItemTaken{cpu.eax, 0, 0};
        return trackdata::ItemTaken{cpu.eax, cpu.bx, cpu.edx};
    }
};

// The shift in eax, the span in edx.
template <>
struct ByAddress<trackdata::setLanes>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 span, x86::reg32 shift)
    {
        watcom(app, cpu, Address, {shift, span});
    }
};

// The place in eax, the axes in edx, the corner in esi; the result in edx.
template <>
struct ByAddress<trackdata::takeFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 axes,
                           x86::reg32 corner)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = place;
        cpu.edx = axes;
        cpu.esi = corner;
        guest::call(app, cpu, Address);
        return cpu.edx;
    }
};

// The place in eax, followed in edx, the node in ebx, out in esi.
template <>
struct ByAddress<trackdata::snapPlace>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 out,
                     x86::reg32 followed, x86::reg32 node)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = place;
        cpu.edx = followed;
        cpu.ebx = node;
        cpu.esi = out;
        return guest::call(app, cpu, Address);
    }
};

// The point in eax, followed in edx, the node in ebx, out in esi.
template <>
struct ByAddress<trackdata::placeAt>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point, x86::reg32 followed,
                     x86::reg32 node, x86::reg32 out)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = point;
        cpu.edx = followed;
        cpu.ebx = node;
        cpu.esi = out;
        return guest::call(app, cpu, Address);
    }
};

// The place in eax, followed in edx.
template <>
struct ByAddress<trackdata::placeColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 followed)
    {
        watcom(app, cpu, Address, {place, followed});
        return cpu.eax;
    }
};

// The place in eax, followed in edx.
template <>
struct ByAddress<trackdata::placeSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 followed)
    {
        watcom(app, cpu, Address, {place, followed});
        return cpu.eax;
    }
};

// The node in eax, the lane in edx.
template <>
struct ByAddress<trackdata::groundUnder>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::sreg32 node, x86::sreg32 lane)
    {
        watcom(app, cpu, Address, {x86::reg32(node), x86::reg32(lane)});
        return cpu.eax;
    }
};

// Stdcall: the place's three words and the node pushed.
template <>
struct ByAddress<trackdata::placeInTunnel>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place,
                                          x86::reg32 node)
    {
        const KeptRegisters kept(cpu);
        if (!guest::call(app, cpu, Address, {at(app, place), at(app, place + 4), at(app, place + 8), node}))
            return std::nullopt;
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
