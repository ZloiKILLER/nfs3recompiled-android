#include "native_frontend_screen.h"
#include "game_settings.h"
#include "native_controls_host.h"
#include "native_ea_memory.h"
#include "native_ea_memory.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "port_shapes.h"
#include "port_text.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>

/* What the menus' screens do around their items: the pointer put where the
 * keys moved the selection, the screens' art loaded.  Each address
 * is bound to the function of that name here (FRONTEND_NATIVES).
 *
 * Written as their code is, as native_frontend_text.cpp's: each push and pop
 * to the guest's stack, each call with the return address's slot, the
 * registers as the code leaves them.  The art's and the memory's functions
 * they call as C++ (guest::direct: every register but eax kept, as those
 * keep them), the shown screen's name by address; what they ask of the mouse
 * they ask of the port (controls::directInput).
 *
 * Plan step 5: each function a typed core in frontend::screen (native_frontend_screen.h) -- its
 * arguments into the registers, the code run, the ret left to the thunk under the game's name,
 * which the calls by address enter. */
namespace nfs3hp
{
namespace frontend
{

namespace
{

using guest::at;
using guest::put;
using x86::reg32;

constexpr reg32 kInRace = 0x559234;          // the menus are a race's pause screens
// The pointer.
constexpr reg32 kMouseX = 0x55e5e4, kMouseY = 0x55e5e8;   // where the menus have it, on a 640 by 480 screen
constexpr reg32 kMouse = 0x554ee8;           // the mouse's handle, or none
constexpr reg32 kGetDeviceData = 0x28;       // GetDeviceData: what the mouse has buffered
constexpr reg32 kPointerLeft = 0x5f38e0, kPointerTop = 0x5f38e4;
constexpr reg32 kPointerRight = 0x5f38e8, kPointerBottom = 0x5f38ec;
constexpr reg32 kPointerX = 0x5f38f0, kPointerY = 0x5f38f4;
// The screens' art.
constexpr reg32 kCurrentMenuName = 0x44ae00;  // the shown screen's name, or none
constexpr reg32 kNoTag = 0x537830;            // ""
constexpr reg32 kPauseName = 0x53793c, kMainMultName = 0x537944, kPostGameName = 0x537950;
constexpr reg32 kNetworkScreen = 0x555748;    // the screen is MAINMULT's, or a race's pause over the network
constexpr reg32 kArtLeft = 0x5f3fc8, kArtTop = 0x5f3fcc;
constexpr reg32 kItemLines = 0x5f3fdc, kItemLinesBytes = 0x64;
constexpr reg32 kItemText = 0x5f3fd0, kItemTextBytes = 0x384;
constexpr reg32 kDetail = 0x7a3a58;           // bit 2: no glow

/* The shapes the screens draw their items with: each its name, where its
 * number goes (a word). */
struct Shape
{
    reg32 name;
    reg32 to;
};
constexpr Shape kShapes[] = {
    {0x53795c, 0x5556cc},  // netw
    {0x537964, 0x5f4002},  // icon
    {0x53796c, 0x5f3ffe},  // icol
    {0x537974, 0x5f3ff8},  // rlit
    {0x53797c, 0x5f3ff4},  // glit
    {0x537984, 0x5f3ffc},  // ccba
    {0x53798c, 0x5f4000},  // ccya
    {0x537994, 0x5f3ffa},  // badg
};
constexpr Shape kGlows[] = {
    {0x53799c, 0x5f4004},  // gglo
    {0x5379a4, 0x5f3ff6},  // rglo
};

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

/* ret, the thunks' end: the return address popped, unless the game stopped
 * in the function. */
void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

using guest::EspKept;

}  // namespace

/* 0x435230: the pointer put at eax, edx, inside its bounds, what the mouse
 * has buffered taken first.  The eax it leaves. */
reg32 screen::placePointer(App* app, CPU& cpu, reg32 x, reg32 y)
{
    // ebx, ecx, esi, edi and ebp pushed by the code (ebp its base), and the local at [ebp - 4]: the
    // frame; the registers they restore are kept across the host call that reads and spoils them.
    const EspKept kept(cpu);
    cpu.esp -= 4 * 5;
    const reg32 ebp = cpu.esp;
    cpu.esp -= 4;
    reg32 eax = x;
    const reg32 esi = x, ebx = y;
    put(app, ebp - 4, 0xffffffff);
    const reg32 device = at(app, kMouse);
    if (device != 0)
    {
        // GetDeviceData(the device, 0x10, none, &(INFINITE), 0): the events dropped.
        push(app, cpu, 0);
        push(app, cpu, ebp - 4);
        push(app, cpu, 0);
        push(app, cpu, 0x10);
        push(app, cpu, device);
        const auto hostResult = controls::directInputResult(app, cpu, controls::Object::Device, kGetDeviceData);
        if (!hostResult)
            return eax;
        eax = *hostResult;  // the host's result
        const reg32 left = at(app, kPointerLeft);
        if (x86::sreg32(esi) < x86::sreg32(left))
            put(app, kPointerX, left);
        else
        {
            eax = at(app, kPointerRight);
            put(app, kPointerX, x86::sreg32(esi) > x86::sreg32(eax) ? eax : esi);
        }
        const reg32 top = at(app, kPointerTop);
        if (x86::sreg32(ebx) < x86::sreg32(top))
            put(app, kPointerY, top);
        else
        {
            const reg32 bottom = at(app, kPointerBottom);
            put(app, kPointerY, x86::sreg32(ebx) > x86::sreg32(bottom) ? bottom : ebx);
        }
    }
    return eax;
}

/* 0x49bfc0: the menus' pointer to eax, edx on a 640 by 480 screen, and the
 * mouse's to there on the screen's own when the 3D card draws the menus.
 * The eax it leaves (placePointer's). */
reg32 screen::setPointer(App* app, CPU& cpu, reg32 x, reg32 y)
{
    cpu.esp -= 4 * 3;  // ebx, ecx and ebp pushed by the code
    reg32 pointerX = x, pointerY = y;
    put(app, kMouseY, y);
    put(app, kMouseX, x);
    if (at(app, kInRace) != 0)
    {
        pointerX = reg32(port::text::screenX(app, x86::sreg32(x)));
        pointerY = reg32(port::text::screenY(app, x86::sreg32(y)));
    }
    const reg32 eax = guest::direct<0x435230, screen::placePointer>(app, cpu, pointerX, pointerY);
    if (cpu.terminate)
        return eax;
    cpu.esp += 4 * 3;
    return eax;
}

/* 0x440970: the screens' art for the one shown -- whether it is the network
 * game's, the shapes its items are drawn with (on the menus, not a race's
 * pause screens) -- and the items' buffers made anew, cleared.  The eax it leaves. */
reg32 screen::loadScreenArt(App* app, CPU& cpu)
{
    // ebx, ecx, edx, esi, edi and ebp pushed by the code: the frame the calls are made in.
    cpu.esp -= 4 * 6;
    put(app, kArtTop, 0x251);
    put(app, kArtLeft, 0x87);
    // 0x44ae00's: the shown screen's name, or none (a plain read of the loop's data: that file is not
    // in the function checks).
    const reg32 menu = at(app, 0x559230);
    const reg32 name = menu != 0 ? at(app, menu + 0x14) : 0;
    reg32 eax = name;
    if (at(app, kInRace) != 0)
    {
        if (name != 0)
        {
            eax = guest::direct<0x4ee310, watcom::clib::strcmp>(app, cpu, name, kPauseName);
            if (cpu.terminate)
                return eax;
            put(app, kNetworkScreen, eax == 0 ? 1 : 0);
        }
    }
    else if (name != 0)
    {
        eax = guest::direct<0x4ee310, watcom::clib::strcmp>(app, cpu, name, kMainMultName);
        if (cpu.terminate)
            return eax;
        if (eax == 0)
            put(app, kNetworkScreen, 1);
        eax = guest::direct<0x4ee310, watcom::clib::strcmp>(app, cpu, name, kPostGameName);
        if (cpu.terminate)
            return eax;
        if (eax == 0)
            put(app, kNetworkScreen, eax);
    }
    if (at(app, kInRace) == 0)
    {
        // A shape of the menus' art by its name (port_shapes.cpp's).
        auto load = [&](const Shape& shape) {
            const port::Result found = guest::direct<0x4d5db0, port::shapes::shapeNamed>(app, cpu, cpu.esp - 4, shape.name);
            if (!found)
                return false;
            eax = reg32(*found);
            app->getMemory<x86::reg16>(shape.to) = x86::reg16(eax);
            return true;
        };
        for (const Shape& shape : kShapes)
            if (!load(shape))
                return eax;
        if (!(guest::byte(app, kDetail) & 2))
            for (const Shape& shape : kGlows)
                if (!load(shape))
                    return eax;
    }
    // The items' buffers given back and made anew: the memory's (native_ea_memory.cpp's), a tag, the
    // bytes, the class.
    const reg32 oldLines = at(app, kItemLines);
    if (oldLines != 0)
    {
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, oldLines);
        if (!freed)
            return eax;
        eax = *freed;
    }
    const std::optional<reg32> lines = guest::direct<0x4e1620, ea::reserved>(app, cpu, kNoTag, kItemLinesBytes, 0);
    if (!lines)
        return eax;
    eax = *lines;
    put(app, kItemLines, eax);
    if (eax != 0)
    {
        eax = guest::direct<0x4e0640, watcom::clib::memset>(app, cpu, eax, 0, kItemLinesBytes);
        if (cpu.terminate)
            return eax;
    }
    const reg32 oldText = at(app, kItemText);
    if (oldText != 0)
    {
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, oldText);
        if (!freed)
            return eax;
        eax = *freed;
    }
    const std::optional<reg32> text = guest::direct<0x4e1620, ea::reserved>(app, cpu, kNoTag, kItemTextBytes, 0);
    if (!text)
        return eax;
    eax = *text;
    put(app, kItemText, eax);
    eax = guest::direct<0x4e0640, watcom::clib::memset>(app, cpu, eax, 0, kItemTextBytes);
    if (cpu.terminate)
        return eax;
    if (at(app, kInRace) == 0)
        gameSettings(app).networkLobby = 0;
    cpu.esp += 4 * 6;
    return eax;
}

/* The thunks under the game's names, which the addresses are bound to: the
 * arguments from the registers, the core, the ret. */
x86::reg32 placePointer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = screen::placePointer(app, cpu, args.eax, args.edx);
    ret(cpu);
    return args.eax;
}

x86::reg32 setPointer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = screen::setPointer(app, cpu, args.eax, args.edx);
    ret(cpu);
    return args.eax;
}

x86::reg32 loadScreenArt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = screen::loadScreenArt(app, cpu);
    ret(cpu);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
