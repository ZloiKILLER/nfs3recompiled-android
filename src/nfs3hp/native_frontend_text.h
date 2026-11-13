#ifndef NFS3HP_NATIVE_FRONTEND_TEXT_H
#define NFS3HP_NATIVE_FRONTEND_TEXT_H

#include "native_guest.h"

/* native_frontend_text.cpp's typed functions (a display mode's or a controller's name, the checks
 * of a player's name and of a line typed in), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address (ByAddress, for the checks' stand-ins).
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in, runs on `cpu` as the code does, and returns what
 * it leaves in eax.  When the game stopped in a call they return at once with cpu.terminate set,
 * as the code does -- they do not throw -- and so do their calls by address here. */
namespace nfs3hp
{
namespace frontend
{
namespace text
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x448400: display mode `mode` (counted from 0x7a8)'s name.
reg32 displayModeName(App* app, CPU& cpu, reg32 mode);
// The game's 0x43d9f0: controller `controller`'s name, its row of the menu shown in its colour.
reg32 controllerName(App* app, CPU& cpu, reg32 controller);
// The game's 0x4721f0: whether `name` will do for a player's: 1 to 10 characters.
reg32 nameValid(App* app, CPU& cpu, reg32 name);
// The game's 0x443630: whether the character in al may be in a short name; ah left al + 1.
reg32 shortNameCharacter(App* app, CPU& cpu, reg32 character);
// The game's 0x443660: whether `line` is a short name: 0 to 8 of those characters.
reg32 shortNameValid(App* app, CPU& cpu, reg32 line);
// The game's 0x443700: whether the language takes `line` as a player's name.
reg32 languageNameValid(App* app, CPU& cpu, reg32 line);
// The game's 0x443780: whether the language takes `line` as a file's name.
reg32 languageFileNameValid(App* app, CPU& cpu, reg32 line);
// The game's 0x443860: whether `line` will do in mode `mode` (0 a short name, 1 and 2 the language's).
reg32 lineAllowed(App* app, CPU& cpu, reg32 line, reg32 mode);

}  // namespace text
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax and edx, every other register
 * as the caller has it; eax back.  With cpu.terminate set when the game stopped in it. */
template <>
struct ByAddress<frontend::text::displayModeName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
    {
        cpu.eax = mode;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::controllerName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 controller)
    {
        cpu.eax = controller;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::nameValid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        cpu.eax = name;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::shortNameCharacter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        cpu.eax = character;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::shortNameValid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line)
    {
        cpu.eax = line;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::languageNameValid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line)
    {
        cpu.eax = line;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::languageFileNameValid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line)
    {
        cpu.eax = line;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::text::lineAllowed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 mode)
    {
        cpu.eax = line;
        cpu.edx = mode;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
