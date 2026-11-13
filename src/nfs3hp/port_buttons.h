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
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_BUTTONS_NATIVES): the widgets' table (0x557a84) and the game's code
 * reach them through their addresses, and the generated code calls them by
 * name.  The adapter takes the arguments from the registers and the stack and
 * gives back the result in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
/* What the widgets call of the port's own that its headers do not declare,
 * called as the game's code calls it -- declared here, not in
 * port_buttons.cpp, as tools/game_checks/generate.py finds a function's
 * source file by its declaration: port_sound_effects.cpp's menu effect,
 * port_shapes.cpp's adapters. */
void playMenuEffect(App* app, CPU& cpu);  // 0x4181d0
namespace shapes
{
void shapeByName(App* app, CPU& cpu);     // 0x4d5db0
void shapeSize(App* app, CPU& cpu);       // 0x4d66f0
void drawArtShape(App* app, CPU& cpu);    // 0x4d7160
void drawShape(App* app, CPU& cpu);       // 0x4d7990
}  // namespace shapes

namespace buttons
{

/* The adapters, under the game's functions' names. */
void joinListedGame(App* app, CPU& cpu);       // 0x43fdd0
void drawReadyMarks(App* app, CPU& cpu);       // 0x440c40
void networkLeft(App* app, CPU& cpu);    // 0x440d20
void drawLicence(App* app, CPU& cpu);          // 0x442450
void listColours(App* app, CPU& cpu);          // 0x442b90
void listAllowed(App* app, CPU& cpu);          // 0x442bf0
void listLength(App* app, CPU& cpu);           // 0x442c50
void findList(App* app, CPU& cpu);             // 0x442c80
void menuEdited(App* app, CPU& cpu);           // 0x442d00
void menuRecordEdited(App* app, CPU& cpu);          // 0x442d20
void loadShapes(App* app, CPU& cpu);           // 0x44ac90
void redrawMenu(App* app, CPU& cpu);           // 0x44b170
void textBoxDraw(App* app, CPU& cpu);          // 0x455e30
void textBoxKey(App* app, CPU& cpu);           // 0x456020
void textBoxUp(App* app, CPU& cpu);            // 0x456110
void textBoxShown(App* app, CPU& cpu);         // 0x456170
void textBoxHidden(App* app, CPU& cpu);        // 0x4561d0
void slideTrackMap(App* app, CPU& cpu);        // 0x456320
void drawTrackFacts(App* app, CPU& cpu);       // 0x456590
void drawGlow(App* app, CPU& cpu);             // 0x456670
void trackMapShape(App* app, CPU& cpu);        // 0x456800
void rectDraw(App* app, CPU& cpu);             // 0x45fae0
void speechDraw(App* app, CPU& cpu);           // 0x45fc90
void speechShown(App* app, CPU& cpu);          // 0x45fca0
void speechHidden(App* app, CPU& cpu);         // 0x45fcb0
void textDraw(App* app, CPU& cpu);             // 0x45fcd0
void textUp(App* app, CPU& cpu);               // 0x460070
void textShown(App* app, CPU& cpu);            // 0x4600b0
void imageDraw(App* app, CPU& cpu);            // 0x460100
void imageUp(App* app, CPU& cpu);              // 0x460410
void imageShown(App* app, CPU& cpu);           // 0x460440
void pulseOn(App* app, CPU& cpu);              // 0x460700
void pulseColour(App* app, CPU& cpu);          // 0x460770
void buttonDraw(App* app, CPU& cpu);           // 0x4607c0
void buttonKey(App* app, CPU& cpu);            // 0x461270
void layoutMenuItem(App* app, CPU& cpu);       // 0x4613e0
void buttonShown(App* app, CPU& cpu);          // 0x461900
void cascadeLayoutClosed(App* app, CPU& cpu);  // 0x461a70
void cascadeLineText(App* app, CPU& cpu);      // 0x461aa0
void cascadeDraw(App* app, CPU& cpu);          // 0x461d00
void cascadePass(App* app, CPU& cpu);          // 0x462320
void cascadeShown(App* app, CPU& cpu);         // 0x4623a0
void cascadeKey(App* app, CPU& cpu);           // 0x4624a0
void cascadeUp(App* app, CPU& cpu);            // 0x4628d0
void checkboxDraw(App* app, CPU& cpu);         // 0x4629c0
void checkboxKey(App* app, CPU& cpu);          // 0x462ad0
void checkboxShown(App* app, CPU& cpu);        // 0x462b20
void checkboxUp(App* app, CPU& cpu);           // 0x462b30
void boxOverlayDraw(App* app, CPU& cpu);       // 0x462b90
void selectListShown(App* app, CPU& cpu);      // 0x462c30
void selectListHidden(App* app, CPU& cpu);     // 0x462ce0
void selectListDraw(App* app, CPU& cpu);       // 0x462d20
void selectListPass(App* app, CPU& cpu);       // 0x462f90
void selectListKey(App* app, CPU& cpu);        // 0x462fa0
void selectListUp(App* app, CPU& cpu);         // 0x4633d0
void dropListDraw(App* app, CPU& cpu);         // 0x4634d0
void dropListPass(App* app, CPU& cpu);         // 0x463de0
void dropListShown(App* app, CPU& cpu);        // 0x463e60
void listKey(App* app, CPU& cpu);              // 0x463ff0
void layoutList(App* app, CPU& cpu);           // 0x464560
void drawTurnedShape(App* app, CPU& cpu);      // 0x4d6730
void drawColouredRectangle(App* app, CPU& cpu);  // 0x4d88f0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */

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

// The name in eax, `ebx` in ebx (edx left as it is); the shapes in eax.
template <>
struct ByAddress<port::buttons::shapesLoaded>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 name, x86::reg32 ebx)
    {
        return watcomEax(app, cpu, Address, {name, cpu.edx, ebx});
    }
};

// `in` in eax, edx, ebx and ecx; eax back.
template <>
struct ByAddress<port::buttons::redrawn>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::Args& in)
    {
        return watcomEax(app, cpu, Address, {in.eax, in.edx, in.ebx, in.ecx});
    }
};

// The shape in eax, the index in edx; the record in eax.
template <>
struct ByAddress<port::buttons::mapShape>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 shape,
                             std::int32_t index)
    {
        return watcomEax(app, cpu, Address, {shape, x86::reg32(index)});
    }
};

// The item in eax, the key in ebx (edx left as it is); the answer in eax.
template <>
struct ByAddress<port::buttons::buttonKeyed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 at, x86::reg32 key)
    {
        return watcomEax(app, cpu, Address, {at, cpu.edx, key});
    }
};

// The item in eax (eax left as it came).
template <>
struct ByAddress<port::buttons::selectLaidOut>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 at)
    {
        watcom(app, cpu, Address, {at});
    }
};

// Stdcall: the shape in eax, x in edx, y in ebx, the nudge in ecx; the angle and the scale pushed (ret 8).
template <>
struct ByAddress<port::buttons::turnedShape>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t shape,
                             std::int32_t x, std::int32_t y, std::int32_t nudge, std::int32_t angle,
                             x86::reg32 scaleBits)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(shape);
        cpu.edx = x86::reg32(x);
        cpu.ebx = x86::reg32(y);
        cpu.ecx = x86::reg32(nudge);
        if (!guest::call(app, cpu, Address, {x86::reg32(angle), scaleBits}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Stdcall: `a` in eax, edx, ebx and ecx; the four colours pushed (ret 0x10).
template <>
struct ByAddress<port::buttons::colouredRectangle>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::Args& a,
                             const x86::reg32 colours[4])
    {
        const KeptRegisters kept(cpu);
        cpu.eax = a.eax;
        cpu.edx = a.edx;
        cpu.ebx = a.ebx;
        cpu.ecx = a.ecx;
        if (!guest::call(app, cpu, Address, {colours[0], colours[1], colours[2], colours[3]}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
