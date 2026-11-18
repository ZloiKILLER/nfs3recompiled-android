#include "native_frontend_draw.h"
#include "native_guest.h"
#include "port_text.h"

#include <iterator>
#include <utility>

/* What is left here of the menus' drawing: a slider's value and a two-axis
 * slider's, onto the x87's stack (0x469520, 0x46afd0), and a .qfs loaded by
 * its name (0x4956f0).  The rest is the port's own: the sliders and the
 * other widgets that hold a value port_fields.cpp's, the buttons, lists and
 * the menu items' layout port_buttons.cpp's, the text and the fonts
 * port_text.cpp's, the 2D drawing port_draw2d.cpp's, the menus' shapes
 * port_shapes.cpp's.
 *
 * Each address is bound to the function of that name here
 * (FRONTEND_DRAW_NATIVES), and the game's code calls it by name.  Written
 * as their code is: each push and pop to the guest's stack, each call with
 * the return address's slot, the frame's locals where the code keeps them,
 * the registers as the code leaves them; the x87's stack pushed and popped a
 * value at a time.  What the menus' other code and the graphics' natives do
 * is called by address.
 *
 * Plan step 5: each function a typed core in frontend::draw (native_frontend_draw.h) -- its
 * arguments into the registers, the code run, eax back, the ret left to the thunk under the
 * game's name, which the calls by address enter. */
namespace nfs3hp
{
namespace frontend
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg32;
using x86::sreg32;

// The pointer, on a 640 by 480 screen.
constexpr reg32 kPointerX = 0x55e5e4, kPointerY = 0x55e5e8;
// The menus' clock, in ticks.
constexpr reg32 kClock = 0x79c1a4;
// The sliders' kinds: their parts' count (a word each, at 0x55a4e0 + 2 * kind),
// their parts' colours and those of a part under the pointer (0xc0 bytes a
// kind, 0x20 a part), their parts' shapes (0x18 bytes a kind, 4 a part).
constexpr reg32 kSliderParts = 0x55a4de;
constexpr reg32 kPartColours = 0x55a4ec, kHotPartColours = 0x55a96c;
constexpr reg32 kPartShapes = 0x678aae;
// What the menus' code and the graphics' natives do.
constexpr reg32 kChop = 0x4dfd56;            // __CHP: st(0) rounded toward 0
constexpr reg32 kFloor = 0x4f1c30;           // floor of the double on the stack
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kViewport = 0x4beb80, kSetClip = 0x4bebe0;
constexpr reg32 kSelectFont = 0x452100;
// The menus' code of frontend_glue's and the formats': a list's line, a text
// scale for the 2D screens, a 2D screen's text, the quads of a font drawn,
// whether the mouse holds a control, a shape of an .fsh by its name, a .qfs
// file loaded.
constexpr reg32 kScreenText = 0x452190;
constexpr reg32 kMouseHolds = 0x49c080;
constexpr reg32 kLoadQfsFile = 0x4f2e20;
// This file's functions that are called by address.
constexpr reg32 kSetSliderValue = 0x469480, kShapeByName = 0x4d5db0;
// The screen's size.
constexpr reg32 kScreenWidth = 0x7cdac8, kScreenHeight = 0x7cdacc;
// The renderer's state bits: 2 for a 3D card's alpha.
constexpr reg32 kRenderFlags = 0x7a3a58;
// The menus' text in the fonts' quads rather than on the 2D screens (not 0).
constexpr reg32 kMenu3d = 0x559234;

x86::Float loadFloat(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

/* The string at `from` copied to `to`, its 0 too. */
void copyString(win32::WinApplication* app, reg32 to, reg32 from)
{
    for (;; ++to, ++from)
    {
        const x86::reg8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
}

reg32 stringLength(win32::WinApplication* app, reg32 string)
{
    reg32 length = 0;
    while (byte(app, string + length) != 0)
        ++length;
    return length;
}

}  // namespace

/* 0x469520: the value of the slider at eax (its thunk leaves it on the x87's stack). */
double draw::sliderStoredValue(App* app, CPU& cpu, reg32 slider)
{
    NFS2_USE(cpu);  // ebp pushed and popped by the code: no value kept
    return loadFloat(app, slider + 0x54);
}

/* 0x46afd0: the value of the slider at eax, end edx's. */
double draw::twoAxisValue(App* app, CPU& cpu, reg32 slider, reg32 end)
{
    NFS2_USE(cpu);
    return loadFloat(app, slider + end * 4 + 0x54);
}

/* 0x4956f0: the .qfs file named at eax loaded (the formats' 0x4f2e20,
 * eax its result): the name copied into the frame, [0x55e544]'s extension put
 * after it where it has none of its own.   edx holds the loader's flags. */
x86::reg32 draw::loadQfsFlags(App* app, CPU& cpu, reg32 fileName, reg32 flags)
{
    // ebx, ecx, esi, edi and ebp pushed by the code, then 0x200 bytes: the frame, whose bottom the
    // name is built at.
    cpu.esp -= 4 * 5 + 0x200;
    const reg32 name = cpu.esp;
    copyString(app, name, fileName);
    reg32 at_ = name + stringLength(app, name);
    // Back from its end to a '.', ':', '/' or '\\', or its start.
    for (;;)
    {
        at_ -= 1;
        if (at_ <= name)
            break;
        const x86::reg8 c = byte(app, at_);
        if (c == '.' || c == ':' || c == '/' || c == '\\')
            break;
    }
    if (byte(app, at_) != '.')
        copyString(app, name + stringLength(app, name), at(app, 0x55e544));
    x86::Args loader;
    loader.eax = name;
    loader.edx = flags;
    const std::optional<reg32> block = guest::pointerCall(app, cpu, kLoadQfsFile, loader);
    if (!block)
        return 0;
    cpu.esp += 4 * 5 + 0x200;
    return *block;
}

x86::reg32 sliderStoredValue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.floatSet = true;
    args.floatResult = draw::sliderStoredValue(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 twoAxisValue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.floatSet = true;
    args.floatResult = draw::twoAxisValue(app, cpu, args.eax, args.edx);
    ret(cpu);
    return args.eax;
}

x86::reg32 loadQfs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 block = draw::loadQfsFlags(app, cpu, args.eax, args.edx);
    args.eax = block;
    ret(cpu);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
