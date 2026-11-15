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

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */

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

// Stdcall: the float's bits pushed (ret 4); eax left as it came.
template <>
struct ByAddress<port::view::storedIncremented>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, float value)
    {
        x86::reg32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        const KeptRegisters kept(cpu);
        guest::call(app, cpu, Address, {bits});
    }
};

// The entries in eax, edx, ebx and ecx, the eight words pushed (ret 0x20).
template <>
struct ByAddress<port::view::edgeTrianglesEmitted>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx,
                     x86::reg32 ebx, x86::reg32 ecx, const x86::reg32* words)
    {
        cpu.eax = eax;
        cpu.edx = edx;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        return guest::call(app, cpu, Address,
                           {words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7]});
    }
};

// The box in eax, edx, ebx and ecx, the three addresses pushed (ret 0xc).
template <>
struct ByAddress<port::view::gridDrawnAt>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 x, x86::reg32 y,
                     x86::reg32 width, x86::reg32 height, x86::reg32 mapX, x86::reg32 mapY, x86::reg32 scale)
    {
        cpu.eax = x;
        cpu.edx = y;
        cpu.ebx = width;
        cpu.ecx = height;
        return guest::call(app, cpu, Address, {mapX, mapY, scale});
    }
};

// The record in eax; eax back.
template <>
struct ByAddress<port::view::metricPopupOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 record)
    {
        return watcomEax(app, cpu, Address, {record});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
