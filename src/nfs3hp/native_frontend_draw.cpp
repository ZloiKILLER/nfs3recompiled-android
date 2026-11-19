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
using nfs::u32;
using nfs::s32;

// The pointer, on a 640 by 480 screen.
constexpr u32 kPointerX = 0x55e5e4, kPointerY = 0x55e5e8;
// The menus' clock, in ticks.
constexpr u32 kClock = 0x79c1a4;
// The sliders' kinds: their parts' count (a word each, at 0x55a4e0 + 2 * kind),
// their parts' colours and those of a part under the pointer (0xc0 bytes a
// kind, 0x20 a part), their parts' shapes (0x18 bytes a kind, 4 a part).
constexpr u32 kSliderParts = 0x55a4de;
constexpr u32 kPartColours = 0x55a4ec, kHotPartColours = 0x55a96c;
constexpr u32 kPartShapes = 0x678aae;
// What the menus' code and the graphics' natives do.
constexpr u32 kChop = 0x4dfd56;            // __CHP: st(0) rounded toward 0
constexpr u32 kFloor = 0x4f1c30;           // floor of the double on the stack
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kViewport = 0x4beb80, kSetClip = 0x4bebe0;
constexpr u32 kSelectFont = 0x452100;
// The menus' code of frontend_glue's and the formats': a list's line, a text
// scale for the 2D screens, a 2D screen's text, the quads of a font drawn,
// whether the mouse holds a control, a shape of an .fsh by its name, a .qfs
// file loaded.
constexpr u32 kScreenText = 0x452190;
constexpr u32 kMouseHolds = 0x49c080;
constexpr u32 kLoadQfsFile = 0x4f2e20;
// This file's functions that are called by address.
constexpr u32 kSetSliderValue = 0x469480, kShapeByName = 0x4d5db0;
// The screen's size.
constexpr u32 kScreenWidth = 0x7cdac8, kScreenHeight = 0x7cdacc;
// The renderer's state bits: 2 for a 3D card's alpha.
constexpr u32 kRenderFlags = 0x7a3a58;
// The menus' text in the fonts' quads rather than on the 2D screens (not 0).
constexpr u32 kMenu3d = 0x559234;

nfs::Float loadFloat(nfs::App* app, u32 address)
{
    return nfs::Float(app->getMemory<float>(address));
}

void ret(nfs::ThreadState& ts)
{
    if (!ts.terminate)
        ts.sp += 4;
}

/* The string at `from` copied to `to`, its 0 too. */
void copyString(nfs::App* app, u32 to, u32 from)
{
    for (;; ++to, ++from)
    {
        const nfs::u8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
}

u32 stringLength(nfs::App* app, u32 string)
{
    u32 length = 0;
    while (byte(app, string + length) != 0)
        ++length;
    return length;
}

}  // namespace

/* 0x469520: the value of the slider at eax (its thunk leaves it on the x87's stack). */
double draw::sliderStoredValue(App* app, ThreadState& ts, u32 slider)
{
    NFS2_USE(ts);  // ebp pushed and popped by the code: no value kept
    return loadFloat(app, slider + 0x54);
}

/* 0x46afd0: the value of the slider at eax, end edx's. */
double draw::twoAxisValue(App* app, ThreadState& ts, u32 slider, u32 end)
{
    NFS2_USE(ts);
    return loadFloat(app, slider + end * 4 + 0x54);
}

/* 0x4956f0: the .qfs file named at eax loaded (the formats' 0x4f2e20,
 * eax its result): the name copied into the frame, [0x55e544]'s extension put
 * after it where it has none of its own.   edx holds the loader's flags. */
nfs::u32 draw::loadQfsFlags(App* app, ThreadState& ts, u32 fileName, u32 flags)
{
    // ebx, ecx, esi, edi and ebp pushed by the code, then 0x200 bytes: the frame, whose bottom the
    // name is built at.
    ts.sp -= 4 * 5 + 0x200;
    const u32 name = ts.sp;
    copyString(app, name, fileName);
    u32 at_ = name + stringLength(app, name);
    // Back from its end to a '.', ':', '/' or '\\', or its start.
    for (;;)
    {
        at_ -= 1;
        if (at_ <= name)
            break;
        const nfs::u8 c = byte(app, at_);
        if (c == '.' || c == ':' || c == '/' || c == '\\')
            break;
    }
    if (byte(app, at_) != '.')
        copyString(app, name + stringLength(app, name), at(app, 0x55e544));
    nfs::Args loader;
    loader.eax = name;
    loader.edx = flags;
    const std::optional<u32> block = guest::pointerCall(app, ts, kLoadQfsFile, loader);
    if (!block)
        return 0;
    ts.sp += 4 * 5 + 0x200;
    return *block;
}

nfs::u32 sliderStoredValue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = draw::sliderStoredValue(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 twoAxisValue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = draw::twoAxisValue(app, ts, args.eax, args.edx);
    ret(ts);
    return args.eax;
}

nfs::u32 loadQfs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 block = draw::loadQfsFlags(app, ts, args.eax, args.edx);
    args.eax = block;
    ret(ts);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
