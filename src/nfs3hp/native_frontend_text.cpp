#include "native_frontend_text.h"
#include "game_settings.h"
#include "native_guest.h"
#include "port_text.h"

/* The menus' text: a display mode's or a controller's name -- names the
 * language's text (port_text.cpp) gives past its own lines -- and the checks
 * of a player's name and of a line typed in.  disassemble_nfs3hp.py binds
 * each address to the function of that name here (FRONTEND_NATIVES), and the
 * generated code calls it by name; the menus' decompiled screens call them
 * by address.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the registers as the code leaves them --
 * every one kept but eax, but where noted.  tools/game_checks holds each
 * against its generated code.
 *
 * Plan step 5: each function a typed core in frontend::text (native_frontend_text.h) -- its
 * arguments into the registers, the code run, eax back, the ret left to the adapter under the
 * game's name, which the calls by address enter.  A core of this file calls another as C++
 * (guest::direct). */
namespace nfs3hp
{
namespace frontend
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using x86::reg32;

// Display modes (sub_448400): 0x114 bytes each, the name at 0x14.
constexpr reg32 kDisplayModes = 0x661220;
constexpr reg32 kModeBytes = 0x114;
constexpr reg32 kModeName = 0x14;
// Controllers: how many there are, 0x88 bytes each from 0x6fc970, a name at 4
// unless its byte is 0; the menu's rows of them, whether shown and their
// colour.
constexpr reg32 kControllerBytes = 0x88;
constexpr reg32 kControllerRowShown = 0x555668;  // int16 each
constexpr reg32 kControllerRowColour = 0x555688;
constexpr reg32 kControllerColour = 0xff649dfd;
constexpr reg32 kUnnamedControllerText = 0x54, kNoControllerText = 0x27;
constexpr reg32 kNameLength = 10;  // a player's name: 1 to 10 characters

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

void putHalf(win32::WinApplication* app, reg32 address, x86::reg16 value)
{
    app->getMemory<x86::reg16>(address) = value;
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

/* ret, the adapters' end: the return address popped, unless the game stopped
 * in the function. */
void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

/* repne scasb for the 0 at es:edi from ecx -1, not and dec: the string's
 * length in ecx, edi past its 0. */
void inlineLength(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    while (cpu.ecx != 0)
    {
        const x86::reg8 b = byte(app, cpu.ees + cpu.edi);
        cpu.edi += cpu.flags.df ? reg32(-1) : 1;
        --cpu.ecx;
        if (b == 0)
            break;
    }
    cpu.ecx = ~cpu.ecx - 1;
}

}  // namespace

/* 0x448400: display mode eax - 0x7a8's name. */
x86::reg32 text::displayModeName(App* app, CPU& cpu, reg32 mode)
{
    cpu.eax = mode;
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = (cpu.eax - 0x7a8) * kModeBytes + kDisplayModes + kModeName;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    return cpu.eax;
}

/* 0x43d9f0: controller eax's name -- "none" past the controllers there are,
 * a line of the language's for one without a name of its own -- its row of
 * the menu shown, in its colour. */
x86::reg32 text::controllerName(App* app, CPU& cpu, reg32 controller)
{
    cpu.eax = controller;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        return cpu.eax;
    };
    if (x86::sreg32(cpu.eax) >= x86::sreg32(gameSettings(app).deviceCount))
    {
        cpu.ecx = 0;
        putHalf(app, kControllerRowShown + cpu.eax * 2, x86::reg16(cpu.ecx));
        cpu.eax = kNoControllerText;
        if (!direct(app, cpu, port::text::languageText))
            return cpu.eax;
        return done();
    }
    cpu.eax = cpu.edx * kControllerBytes;
    if (std::uint8_t(gameSettings(app).deviceAt(cpu.eax).name[0]) != 0)
        cpu.eax += SETTINGS_ADDRESS(devices[0].name);
    else
    {
        cpu.eax = kUnnamedControllerText;
        if (!direct(app, cpu, port::text::languageText))
            return cpu.eax;
    }
    putHalf(app, kControllerRowShown + cpu.edx * 2, 1);
    put(app, kControllerRowColour + cpu.edx * 4, kControllerColour);
    return done();
}

/* 0x4721f0: whether the name at eax will do for a player's: 1 to 10
 * characters.  0 for none. */
x86::reg32 text::nameValid(App* app, CPU& cpu, reg32 name)
{
    cpu.eax = name;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.edi = cpu.eax;
        inlineLength(app, cpu);
        bool valid = false;
        if (cpu.ecx >= 1)
        {
            cpu.edi = cpu.edx;
            inlineLength(app, cpu);
            valid = cpu.ecx <= kNameLength;
        }
        cpu.eax = valid ? 1 : 0;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    return cpu.eax;
}

/* ---- What may be typed in a dialog's line: the checks of sub_443860's
 * modes. ---- */
namespace
{

constexpr reg32 kCharacterKinds = 0x564ef0;  // the runtime's table of each byte's kind, from -1
constexpr reg32 kLanguageChecks = 0x558c80;  // 0x14 bytes a language: its checks of a line, by mode
constexpr reg32 kLanguageCheckBytes = 0x14;
constexpr reg32 kShortName = 8;

/* The language's check of a line at `offset` of its row, through its
 * pointer; 1 when it has none.  The ret is the caller's. */
void languageCheck(win32::WinApplication* app, x86::CPU& cpu, reg32 offset)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = gameSettings(app).language;
    cpu.eax = (cpu.ecx * 4 + cpu.ecx) << 2;
    if (at(app, cpu.eax + kLanguageChecks + offset) == 0)
        cpu.al = 1;
    else
    {
        cpu.ecx = cpu.eax;
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, at(app, cpu.ecx + kLanguageChecks + offset)))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
}

}  // namespace

/* 0x443630: whether the character in al may be in a short name: a letter or
 * a digit, '_' or '-'.  ah is left al + 1. */
x86::reg32 text::shortNameCharacter(App* app, CPU& cpu, reg32 character)
{
    cpu.eax = character;
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ah = cpu.al;
    cpu.edx = 0;
    ++cpu.ah;
    cpu.dl = cpu.ah;
    if (byte(app, cpu.edx + kCharacterKinds) & 0xe0)
        cpu.al = 1;
    else
        cpu.al = cpu.al == '_' || cpu.al == '-' ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    return cpu.eax;
}

/* 0x443660: whether the text at eax is a short name: 0 to 8 of those. */
x86::reg32 text::shortNameValid(App* app, CPU& cpu, reg32 line)
{
    cpu.eax = line;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        return cpu.eax;
    };
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
    {
        cpu.al = 0;
        return done();
    }
    cpu.edi = cpu.eax;
    inlineLength(app, cpu);
    cpu.ebx = cpu.ecx;
    if (x86::sreg32(cpu.ecx) < 0 || x86::sreg32(cpu.ecx) > x86::sreg32(kShortName))
    {
        cpu.al = 0;
        return done();
    }
    for (cpu.ecx = 0; x86::sreg32(cpu.ecx) < x86::sreg32(cpu.ebx); ++cpu.ecx)
    {
        cpu.eax = cpu.edx + cpu.ecx;
        cpu.al = byte(app, cpu.eax);
        cpu.eax &= 0xff;
        guest::direct<0x443630, shortNameCharacter>(app, cpu, cpu.eax);
        if (cpu.terminate)
            return cpu.eax;
        if (cpu.al == 0)
            return done();
    }
    cpu.al = 1;
    return done();
}

/* 0x443700, 0x443780: whether the language takes the line at eax as a
 * player's name (mode 1), as a file's (mode 2). */
x86::reg32 text::languageNameValid(App* app, CPU& cpu, reg32 line)
{
    cpu.eax = line;
    languageCheck(app, cpu, 0);
    return cpu.eax;
}

x86::reg32 text::languageFileNameValid(App* app, CPU& cpu, reg32 line)
{
    cpu.eax = line;
    languageCheck(app, cpu, 8);
    return cpu.eax;
}

/* 0x443860: whether the line at eax will do in mode edx: 0 a short name, 1
 * and 2 the language's checks; never an empty one, nor in another mode. */
x86::reg32 text::lineAllowed(App* app, CPU& cpu, reg32 line, reg32 mode)
{
    cpu.eax = line;
    cpu.edx = mode;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.bl = 0;
    if (cpu.eax == 0 || byte(app, cpu.eax) == 0)
        cpu.al = 0;
    else
    {
        if (cpu.edx <= 2)
        {
            if (cpu.edx == 0)
                guest::direct<0x443660, shortNameValid>(app, cpu, cpu.eax);
            else if (cpu.edx == 1)
                guest::direct<0x443700, languageNameValid>(app, cpu, cpu.eax);
            else
                guest::direct<0x443780, languageFileNameValid>(app, cpu, cpu.eax);
            if (cpu.terminate)
                return cpu.eax;
            cpu.bl = cpu.al;
        }
        cpu.al = cpu.bl;
    }
    cpu.ebp = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    return cpu.eax;
}

/* The adapters under the game's names, which disassemble_nfs3hp.py binds the addresses to: the
 * arguments from the registers, the core, the ret. */
void displayModeName(win32::WinApplication* app, x86::CPU& cpu)
{
    text::displayModeName(app, cpu, cpu.eax);
    ret(cpu);
}

void controllerName(win32::WinApplication* app, x86::CPU& cpu)
{
    text::controllerName(app, cpu, cpu.eax);
    ret(cpu);
}

void nameValid(win32::WinApplication* app, x86::CPU& cpu)
{
    text::nameValid(app, cpu, cpu.eax);
    ret(cpu);
}

void shortNameCharacter(win32::WinApplication* app, x86::CPU& cpu)
{
    text::shortNameCharacter(app, cpu, cpu.eax);
    ret(cpu);
}

void shortNameValid(win32::WinApplication* app, x86::CPU& cpu)
{
    text::shortNameValid(app, cpu, cpu.eax);
    ret(cpu);
}

void languageNameValid(win32::WinApplication* app, x86::CPU& cpu)
{
    text::languageNameValid(app, cpu, cpu.eax);
    ret(cpu);
}

void languageFileNameValid(win32::WinApplication* app, x86::CPU& cpu)
{
    text::languageFileNameValid(app, cpu, cpu.eax);
    ret(cpu);
}

void lineAllowed(win32::WinApplication* app, x86::CPU& cpu)
{
    text::lineAllowed(app, cpu, cpu.eax, cpu.edx);
    ret(cpu);
}

}  // namespace frontend
}  // namespace nfs3hp
