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
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (FRONTEND_DRAW_NATIVES), and the generated code calls it by name.  Written
 * as their code is: each push and pop to the guest's stack, each call with
 * the return address's slot, the frame's locals where the code keeps them,
 * the registers as the code leaves them; the x87's stack pushed and popped a
 * value at a time.  What the menus' other code and the graphics' natives do
 * is called by address, and so is what of this file has a stand-in in
 * tools/game_checks, which holds each against its generated code. */
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
// This file's functions with a stand-in of their own: called by address.
constexpr reg32 kSetSliderValue = 0x469480, kShapeByName = 0x4d5db0;
// The screen's size.
constexpr reg32 kScreenWidth = 0x7cdac8, kScreenHeight = 0x7cdacc;
// The renderer's state bits: 2 for a 3D card's alpha.
constexpr reg32 kRenderFlags = 0x7a3a58;
// The menus' text in the fonts' quads rather than on the 2D screens (not 0).
constexpr reg32 kMenu3d = 0x559234;

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

x86::reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<x86::reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, x86::reg16 value)
{
    app->getMemory<x86::reg16>(address) = value;
}

/* mov r, [address]; sar r, 0x10: the word at address + 2, signed. */
reg32 high(win32::WinApplication* app, reg32 address)
{
    return reg32(sreg32(at(app, address)) >> 16);
}

/* mov eax, v; sar edx, 0x1f; sub eax, edx; sar eax, 1 -- with v in edx: v
 * halved toward 0 in eax, its sign in edx. */
void halve(x86::CPU& cpu, reg32 v)
{
    cpu.edx = reg32(sreg32(v) >> 31);
    cpu.eax = reg32(sreg32(v - cpu.edx) >> 1);
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
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

x86::Float loadFloat(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

x86::Float loadDouble(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

void storeFloat(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

void storeDouble(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<double>(address) = double(value);
}

/* fild of a dword, of a word. */
x86::Float integer(win32::WinApplication* app, reg32 address)
{
    return x86::Float(sreg32(at(app, address)));
}

x86::Float halfInteger(win32::WinApplication* app, reg32 address)
{
    return x86::Float(x86::sreg16(half(app, address)));
}

/* fistp: st(0) as the control word rounds it, popped. */
void storeInteger(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    put(app, address, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.count -= 1;
}

void exchange(x86::CPU& cpu, int i)
{
    std::swap(cpu.fpu.st(0), cpu.fpu.st(i));
}

/* fdivp st(i): st(i) = st(i) / st(0), popped. */
void divPop(x86::CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::div(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.count -= 1;
}

/* fcomp with `with`, fnstsw ax, sahf: popped. */
void comparePop(x86::CPU& cpu, const x86::Float& with)
{
    cpu.fpu.compare(cpu.fpu.st(0), with);
    cpu.ax = cpu.fpu.status.word;
    cpu.fpu.count -= 1;
}

/* fcompp, fnstsw ax, sahf. */
void compareTwo(x86::CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.ax = cpu.fpu.status.word;
    cpu.fpu.count -= 2;
}

/* What sahf makes of the compare: jb, jbe. */
bool below(const x86::CPU& cpu)
{
    return cpu.fpu.status.c0;
}

bool belowOrEqual(const x86::CPU& cpu)
{
    return cpu.fpu.status.c0 || cpu.fpu.status.c3;
}

/* push ebp, mov ebp, esp after the registers named, and sub esp, `locals`;
 * leave it the other way round, ret `popped`. */
void enter(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 locals)
{
    for (reg32* r : saved)
        push(app, cpu, *r);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= locals;
}

void leave(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 popped = 0)
{
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    for (auto r = std::rbegin(saved); r != std::rend(saved); ++r)
        **r = pop(app, cpu);
    cpu.esp += 4 + popped;
}

/* Signed division of edx:eax by `divisor`, as idiv. */
void idiv(x86::CPU& cpu, reg32 divisor)
{
    const std::int64_t dividend = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
    cpu.eax = reg32(dividend / sreg32(divisor));
    cpu.edx = reg32(dividend % sreg32(divisor));
}

/* rep movsd: ecx dwords from esi to edi. */
void copyDwords(win32::WinApplication* app, x86::CPU& cpu)
{
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
}

/* repne scasb: edi past the first al within ecx bytes, ecx counted down. */
void scanBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    while (cpu.ecx != 0)
    {
        const x86::reg8 b = byte(app, cpu.ees + cpu.edi);
        cpu.edi += 1;
        --cpu.ecx;
        if (b == cpu.al)
            break;
    }
}

/* The code's own strcpy of esi to edi, two bytes a turn: esi and edi past
 * the 0's pair, al the last byte copied. */
void copyPairs(win32::WinApplication* app, x86::CPU& cpu)
{
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            break;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            break;
    }
}

/* That copy with edi kept across it (push edi ... pop edi). */
void inlineCopy(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    copyPairs(app, cpu);
    cpu.edi = pop(app, cpu);
}

/* And its strcat of esi onto the text at edi: push edi; sub ecx, ecx; dec ecx;
 * mov al, 0; repne scasb; dec edi -- the copy from there, pop edi. */
void inlineAppend(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    cpu.ecx = 0xffffffff;
    cpu.al = 0;
    scanBytes(app, cpu);
    cpu.edi -= 1;
    copyPairs(app, cpu);
    cpu.edi = pop(app, cpu);
}

}  // namespace

/* 0x469520: the value of the slider at eax, onto the x87's stack. */
void sliderStoredValue(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.fpu.push(loadFloat(app, cpu.eax + 0x54));
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x46afd0: value edx of the two-axis slider at eax, onto the x87's stack. */
void twoAxisValue(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.fpu.push(loadFloat(app, cpu.eax + cpu.edx * 4 + 0x54));
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4956f0: the .qfs file named at eax loaded (the formats' 0x4f2e20,
 * eax its result): the name copied into the frame, [0x55e544]'s extension put
 * after it where it has none of its own. */
void loadQfs(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.esi, &cpu.edi}, 0x200);
    cpu.ebp -= 0x27e;
    const reg32 name = cpu.ebp + 0x7e;
    cpu.edi = name;
    cpu.esi = cpu.eax;
    inlineCopy(app, cpu);
    cpu.edi = name;
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    scanBytes(app, cpu);
    cpu.ecx = ~cpu.ecx - 1;
    cpu.eax = name + cpu.ecx;
    // Back from its end to a '.', ':', '/' or '\\', or its start.
    for (;;)
    {
        cpu.eax -= 1;
        cpu.ecx = name;
        if (cpu.eax <= cpu.ecx)
            break;
        cpu.bl = byte(app, cpu.eax);
        if (cpu.bl == '.' || cpu.bl == ':' || cpu.bl == '/' || cpu.bl == '\\')
            break;
    }
    if (byte(app, cpu.eax) != '.')
    {
        cpu.edi = name;
        cpu.esi = at(app, 0x55e544);
        inlineAppend(app, cpu);
    }
    cpu.eax = name;
    if (!byAddress(app, cpu, kLoadQfsFile))
        return;
    cpu.esp = cpu.ebp + 0x27e;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace frontend
}  // namespace nfs3hp
