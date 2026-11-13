#include "native_frontend_screen.h"
#include "game_settings.h"
#include "native_controls_host.h"
#include "native_ea_memory.h"
#include "native_guest.h"
#include "port_shapes.h"
#include "port_text.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>

/* What the menus' screens do around their items: the pointer put where the
 * keys moved the selection, the screens' art loaded.  disassemble_nfs3hp.py
 * binds each address to the function of that name here (FRONTEND_NATIVES).
 *
 * Written as their code is, as native_frontend_text.cpp's: each push and pop
 * to the guest's stack, each call with the return address's slot, the
 * registers as the code leaves them.  The art's and the memory's functions
 * they call as C++ (guest::direct: every register but eax kept, as those
 * keep them), the shown screen's name by address; what they ask of the mouse
 * they ask of the port (controls::directInput).  tools/game_checks holds each
 * against its generated code.
 *
 * Plan step 5: each function a typed core in frontend::screen (native_frontend_screen.h) -- its
 * arguments into the registers, the code run, the ret left to the adapter under the game's name,
 * which the calls by address enter. */
namespace nfs3hp
{
namespace watcom
{
void memset(win32::WinApplication* app, x86::CPU& cpu);
void strcmp(win32::WinApplication* app, x86::CPU& cpu);
}

namespace frontend
{

void placePointer(win32::WinApplication* app, x86::CPU& cpu);

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

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

void enter(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 locals)
{
    for (reg32* r : saved)
        push(app, cpu, *r);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= locals;
}

/* The registers popped, the ret left to the adapter (ret). */
void leave(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved)
{
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    for (auto r = std::rbegin(saved); r != std::rend(saved); ++r)
        **r = pop(app, cpu);
}

/* ret, the adapters' end: the return address popped, unless the game stopped
 * in the function. */
void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

}  // namespace

/* 0x435230: the pointer put at eax, edx, inside its bounds, what the mouse
 * has buffered taken first. */
void screen::placePointer(App* app, CPU& cpu, reg32 x, reg32 y)
{
    cpu.eax = x;
    cpu.edx = y;
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.esi, &cpu.edi}, 4);
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.edx = 0xffffffff;
    cpu.ecx = at(app, kMouse);
    put(app, ebp - 4, cpu.edx);
    if (cpu.ecx != 0)
    {
        // GetDeviceData(the device, 0x10, none, &(INFINITE), 0): the events dropped.
        push(app, cpu, 0);
        cpu.eax = cpu.ecx;
        cpu.edx = at(app, cpu.ecx);
        cpu.ecx = ebp - 4;
        push(app, cpu, cpu.ecx);
        push(app, cpu, 0);
        push(app, cpu, 0x10);
        push(app, cpu, cpu.eax);
        if (!controls::directInput(app, cpu, controls::Object::Device, kGetDeviceData))
            return;
        cpu.edi = at(app, kPointerLeft);
        if (x86::sreg32(cpu.esi) < x86::sreg32(cpu.edi))
            put(app, kPointerX, cpu.edi);
        else
        {
            cpu.eax = at(app, kPointerRight);
            put(app, kPointerX, x86::sreg32(cpu.esi) > x86::sreg32(cpu.eax) ? cpu.eax : cpu.esi);
        }
        cpu.edx = at(app, kPointerTop);
        if (x86::sreg32(cpu.ebx) < x86::sreg32(cpu.edx))
            put(app, kPointerY, cpu.edx);
        else
        {
            cpu.ecx = at(app, kPointerBottom);
            put(app, kPointerY, x86::sreg32(cpu.ebx) > x86::sreg32(cpu.ecx) ? cpu.ecx : cpu.ebx);
        }
    }
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.esi, &cpu.edi});
}

/* 0x49bfc0: the menus' pointer to eax, edx on a 640 by 480 screen, and the
 * mouse's to there on the screen's own when the 3D card draws the menus. */
void screen::setPointer(App* app, CPU& cpu, reg32 x, reg32 y)
{
    cpu.eax = x;
    cpu.edx = y;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.ebx = at(app, kInRace);
    put(app, kMouseY, cpu.edx);
    put(app, kMouseX, cpu.eax);
    if (cpu.ebx != 0)
    {
        if (!direct(app, cpu, port::text::toScreenX))
            return;
        cpu.ecx = cpu.eax;
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, port::text::toScreenY))
            return;
        cpu.edx = cpu.eax;
    }
    cpu.eax = cpu.ecx;
    // The adapter, not guest::direct: setPointer leaves edx as placePointer leaves it.
    if (!direct(app, cpu, frontend::placePointer))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
}

/* 0x440970: the screens' art for the one shown -- whether it is the network
 * game's, the shapes its items are drawn with (on the menus, not a race's
 * pause screens) -- and the items' buffers made anew, cleared. */
void screen::loadScreenArt(App* app, CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0);
    cpu.edx = 0x251;
    cpu.ecx = 0x87;
    put(app, kArtTop, cpu.edx);
    put(app, kArtLeft, cpu.ecx);
    if (!byAddress(app, cpu, kCurrentMenuName))
        return;
    cpu.ebx = at(app, kInRace);
    cpu.ecx = cpu.eax;
    if (cpu.ebx != 0)
    {
        if (cpu.eax != 0)
        {
            cpu.edx = kPauseName;
            if (!direct(app, cpu, watcom::strcmp))
                return;
            if (cpu.eax == 0)
                put(app, kNetworkScreen, 1);
            else
            {
                cpu.edi = 0;
                put(app, kNetworkScreen, cpu.edi);
            }
        }
    }
    else if (cpu.eax != 0)
    {
        cpu.edx = kMainMultName;
        if (!direct(app, cpu, watcom::strcmp))
            return;
        if (cpu.eax == 0)
            put(app, kNetworkScreen, 1);
        cpu.edx = kPostGameName;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, watcom::strcmp))
            return;
        if (cpu.eax == 0)
            put(app, kNetworkScreen, cpu.eax);
    }
    if (at(app, kInRace) == 0)
    {
        // A shape of the menus' art by its name (port_shapes.cpp's).
        auto load = [&](const Shape& shape) {
            cpu.eax = shape.name;
            const port::Result found = guest::direct<0x4d5db0, port::shapes::shapeNamed>(app, cpu, cpu.esp - 4, cpu.eax);
            if (!found)
                return false;
            cpu.eax = reg32(*found);
            app->getMemory<x86::reg16>(shape.to) = cpu.ax;
            return true;
        };
        for (const Shape& shape : kShapes)
            if (!load(shape))
                return;
        if (!(guest::byte(app, kDetail) & 2))
            for (const Shape& shape : kGlows)
                if (!load(shape))
                    return;
    }
    // The items' buffers given back and made anew: the memory's (native_ea_memory.cpp's), eax a tag,
    // edx bytes, ebx the class.
    cpu.ebx = at(app, kItemLines);
    if (cpu.ebx != 0)
    {
        cpu.eax = cpu.ebx;
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, cpu.eax);
        if (!freed)
            return;
        cpu.eax = *freed;
    }
    cpu.edx = kItemLinesBytes;
    cpu.eax = kNoTag;
    cpu.ebx = 0;
    const std::optional<reg32> lines = guest::direct<0x4e1620, ea::reserved>(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!lines)
        return;
    cpu.eax = *lines;
    put(app, kItemLines, cpu.eax);
    if (cpu.eax != 0)
    {
        cpu.ebx = kItemLinesBytes;
        cpu.edx = 0;
        if (!direct(app, cpu, watcom::memset))
            return;
    }
    cpu.esi = at(app, kItemText);
    if (cpu.esi != 0)
    {
        cpu.eax = cpu.esi;
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, cpu.eax);
        if (!freed)
            return;
        cpu.eax = *freed;
    }
    cpu.edx = kItemTextBytes;
    cpu.eax = kNoTag;
    cpu.ebx = 0;
    const std::optional<reg32> text = guest::direct<0x4e1620, ea::reserved>(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!text)
        return;
    cpu.eax = *text;
    cpu.ebx = kItemTextBytes;
    cpu.edx = 0;
    put(app, kItemText, cpu.eax);
    if (!direct(app, cpu, watcom::memset))
        return;
    cpu.edi = at(app, kInRace);
    if (cpu.edi == 0)
        gameSettings(app).networkLobby = cpu.edi;
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi});
}

/* The adapters under the game's names, which disassemble_nfs3hp.py binds the addresses to: the
 * arguments from the registers, the core, the ret. */
void placePointer(win32::WinApplication* app, x86::CPU& cpu)
{
    screen::placePointer(app, cpu, cpu.eax, cpu.edx);
    ret(cpu);
}

void setPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    screen::setPointer(app, cpu, cpu.eax, cpu.edx);
    ret(cpu);
}

void loadScreenArt(win32::WinApplication* app, x86::CPU& cpu)
{
    screen::loadScreenArt(app, cpu);
    ret(cpu);
}

}  // namespace frontend
}  // namespace nfs3hp
