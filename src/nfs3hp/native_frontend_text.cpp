#include "native_frontend_text.h"
#include "game_settings.h"
#include "native_guest.h"
#include "port_text.h"

/* The menus' text: a display mode's or a controller's name -- names the
 * language's text (port_text.cpp) gives past its own lines -- and the checks
 * of a player's name and of a line typed in.  The menus' decompiled screens call them
 * by address.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the registers as the code leaves them --
 * every one kept but eax, but where noted.
 *
 * Plan step 5: each function a typed core in frontend::text (native_frontend_text.h) -- its
 * arguments into the registers, the code run, eax back, the ret left to the thunk under the
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

void putHalf(win32::WinApplication* app, reg32 address, x86::reg16 value)
{
    app->getMemory<x86::reg16>(address) = value;
}

/* ret, the thunks' end: the return address popped, unless the game stopped
 * in the function. */
void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

/* repne scasb for the 0: the string's length. */
reg32 stringLength(win32::WinApplication* app, reg32 string)
{
    reg32 length = 0;
    while (byte(app, string + length) != 0)
        ++length;
    return length;
}

/* A register's low byte replaced. */
reg32 withLow(reg32 value, x86::reg8 low)
{
    return (value & ~reg32(0xff)) | low;
}

}  // namespace

/* 0x448400: display mode eax - 0x7a8's name. */
x86::reg32 text::displayModeName(App* app, CPU& cpu, reg32 mode)
{
    // The code pushes and pops edx and ebp: frames, no values kept here.
    NFS2_USE(app);
    NFS2_USE(cpu);
    return (mode - 0x7a8) * kModeBytes + kDisplayModes + kModeName;
}

/* 0x43d9f0: controller eax's name -- "none" past the controllers there are,
 * a line of the language's for one without a name of its own -- its row of
 * the menu shown, in its colour. */
x86::reg32 text::controllerName(App* app, CPU& cpu, reg32 controller)
{
    // ecx, edx and ebp pushed by the code: the frame, not their values.
    cpu.esp -= 4 * 3;
    const reg32 frame = cpu.esp;
    reg32 eax = controller;
    if (x86::sreg32(controller) >= x86::sreg32(gameSettings(app).deviceCount))
    {
        putHalf(app, kControllerRowShown + controller * 2, 0);
        const port::Result text = port::text::lineText(
            app, cpu, cpu.esp - 4, std::int32_t(kNoControllerText), controller);
        if (cpu.terminate)
            return 0;
        cpu.esp = frame + 4 * 3;
        return reg32(*text);
    }
    eax = controller * kControllerBytes;
    if (std::uint8_t(gameSettings(app).deviceAt(eax).name[0]) != 0)
        eax += SETTINGS_ADDRESS(devices[0].name);
    else
    {
        const port::Result text = port::text::lineText(
            app, cpu, cpu.esp - 4, std::int32_t(kUnnamedControllerText), controller);
        if (cpu.terminate)
            return 0;
        cpu.esp = frame;
        eax = reg32(*text);
    }
    putHalf(app, kControllerRowShown + controller * 2, 1);
    put(app, kControllerRowColour + controller * 4, kControllerColour);
    cpu.esp += 4 * 3;
    return eax;
}

/* 0x4721f0: whether the name at eax will do for a player's: 1 to 10
 * characters.  0 for none. */
x86::reg32 text::nameValid(App* app, CPU& cpu, reg32 name)
{
    // ecx, edx, edi and ebp pushed by the code: no call inside, so no frame is kept here.
    NFS2_USE(cpu);
    if (name == 0)
        return name;
    bool valid = false;
    if (stringLength(app, name) >= 1)
        valid = stringLength(app, name) <= kNameLength;
    return valid ? 1 : 0;
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
 * pointer; 1 when it has none.  Its eax; the pointer call keeps edx and ecx as it finds them. */
reg32 languageCheck(win32::WinApplication* app, x86::CPU& cpu, reg32 line, reg32 offset)
{
    cpu.esp -= 4 * 3;  // ecx, edx and ebp pushed by the code
    const reg32 row = gameSettings(app).language * 5 << 2;
    reg32 eax = row;
    if (at(app, row + kLanguageChecks + offset) == 0)
        eax = withLow(eax, 1);
    else
    {
        x86::Args check;
        check.eax = line;
        check.edx = line;
        check.ecx = row;
        const std::optional<reg32> checked =
            guest::pointerCall(app, cpu, at(app, row + kLanguageChecks + offset), check);
        if (!checked)
            return 0;
        eax = *checked;
    }
    cpu.esp += 4 * 3;
    return eax;
}

}  // namespace

/* 0x443630: whether the character in al may be in a short name: a letter or
 * a digit, '_' or '-'.  ah is left al + 1. */
x86::reg32 text::shortNameCharacter(App* app, CPU& cpu, reg32 character)
{
    // edx and ebp pushed by the code: no call inside, so no frame is kept here.
    NFS2_USE(cpu);
    const x86::reg8 al = x86::reg8(character);
    const x86::reg8 ah = x86::reg8(al + 1);
    x86::reg8 result;
    if (byte(app, reg32(ah) + kCharacterKinds) & 0xe0)
        result = 1;
    else
        result = al == '_' || al == '-' ? 1 : 0;
    return (character & 0xffff0000) | (reg32(ah) << 8) | result;
}

/* 0x443660: whether the text at eax is a short name: 0 to 8 of those. */
x86::reg32 text::shortNameValid(App* app, CPU& cpu, reg32 line)
{
    // ebx, ecx, edx, edi and ebp pushed by the code: the frame the calls are made in.
    if (line == 0)
        return 0;
    const reg32 length = stringLength(app, line);
    if (x86::sreg32(length) < 0 || x86::sreg32(length) > x86::sreg32(kShortName))
        return 0;
    cpu.esp -= 4 * 5;
    reg32 eax = 0;
    for (reg32 i = 0; x86::sreg32(i) < x86::sreg32(length); ++i)
    {
        eax = guest::direct<0x443630, shortNameCharacter>(app, cpu, reg32(byte(app, line + i)));
        if (cpu.terminate)
            return eax;
        if (x86::reg8(eax) == 0)
        {
            cpu.esp += 4 * 5;
            return eax;
        }
    }
    cpu.esp += 4 * 5;
    return withLow(eax, 1);
}

/* 0x443700, 0x443780: whether the language takes the line at eax as a
 * player's name (mode 1), as a file's (mode 2). */
x86::reg32 text::languageNameValid(App* app, CPU& cpu, reg32 line)
{
    return languageCheck(app, cpu, line, 0);
}

x86::reg32 text::languageFileNameValid(App* app, CPU& cpu, reg32 line)
{
    return languageCheck(app, cpu, line, 8);
}

/* 0x443860: whether the line at eax will do in mode edx: 0 a short name, 1
 * and 2 the language's checks; never an empty one, nor in another mode. */
x86::reg32 text::lineAllowed(App* app, CPU& cpu, reg32 line, reg32 mode)
{
    // ebx and ebp pushed by the code: the frame the calls are made in.
    if (line == 0 || byte(app, line) == 0)
        return withLow(line, 0);
    x86::reg8 bl = 0;
    reg32 eax = line;
    if (mode <= 2)
    {
        cpu.esp -= 4 * 2;
        if (mode == 0)
            eax = guest::direct<0x443660, shortNameValid>(app, cpu, line);
        else if (mode == 1)
            eax = guest::direct<0x443700, languageNameValid>(app, cpu, line);
        else
            eax = guest::direct<0x443780, languageFileNameValid>(app, cpu, line);
        if (cpu.terminate)
            return eax;
        cpu.esp += 4 * 2;
        bl = x86::reg8(eax);
    }
    return withLow(eax, bl);
}

/* The thunks under the game's names, the
 * arguments from the registers, the core, the ret. */
x86::reg32 displayModeName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::displayModeName(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 controllerName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::controllerName(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 nameValid(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::nameValid(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 shortNameCharacter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::shortNameCharacter(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 shortNameValid(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::shortNameValid(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 languageNameValid(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::languageNameValid(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 languageFileNameValid(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::languageFileNameValid(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 lineAllowed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = text::lineAllowed(app, cpu, args.eax, args.edx);
    ret(cpu);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
