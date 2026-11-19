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
using nfs::u32;

// Display modes (sub_448400): 0x114 bytes each, the name at 0x14.
constexpr u32 kDisplayModes = 0x661220;
constexpr u32 kModeBytes = 0x114;
constexpr u32 kModeName = 0x14;
// Controllers: how many there are, 0x88 bytes each from 0x6fc970, a name at 4
// unless its byte is 0; the menu's rows of them, whether shown and their
// colour.
constexpr u32 kControllerBytes = 0x88;
constexpr u32 kControllerRowShown = 0x555668;  // int16 each
constexpr u32 kControllerRowColour = 0x555688;
constexpr u32 kControllerColour = 0xff649dfd;
constexpr u32 kUnnamedControllerText = 0x54, kNoControllerText = 0x27;
constexpr u32 kNameLength = 10;  // a player's name: 1 to 10 characters

void putHalf(nfs::App* app, u32 address, nfs::u16 value)
{
    app->getMemory<nfs::u16>(address) = value;
}

/* ret, the thunks' end: the return address popped, unless the game stopped
 * in the function. */
void ret(nfs::ThreadState& ts)
{
    if (!ts.terminate)
        ts.sp += 4;
}

/* repne scasb for the 0: the string's length. */
u32 stringLength(nfs::App* app, u32 string)
{
    u32 length = 0;
    while (byte(app, string + length) != 0)
        ++length;
    return length;
}

/* A register's low byte replaced. */
u32 withLow(u32 value, nfs::u8 low)
{
    return (value & ~u32(0xff)) | low;
}

}  // namespace

/* 0x448400: display mode eax - 0x7a8's name. */
nfs::u32 text::displayModeName(App* app, ThreadState& ts, u32 mode)
{
    // The code pushes and pops edx and ebp: frames, no values kept here.
    NFS2_USE(app);
    NFS2_USE(ts);
    return (mode - 0x7a8) * kModeBytes + kDisplayModes + kModeName;
}

/* 0x43d9f0: controller eax's name -- "none" past the controllers there are,
 * a line of the language's for one without a name of its own -- its row of
 * the menu shown, in its colour. */
nfs::u32 text::controllerName(App* app, ThreadState& ts, u32 controller)
{
    // ecx, edx and ebp pushed by the code: the frame, not their values.
    ts.sp -= 4 * 3;
    const u32 frame = ts.sp;
    u32 eax = controller;
    if (nfs::s32(controller) >= nfs::s32(gameSettings(app).deviceCount))
    {
        putHalf(app, kControllerRowShown + controller * 2, 0);
        const port::Result text = port::text::lineText(
            app, ts, ts.sp - 4, std::int32_t(kNoControllerText), controller);
        if (ts.terminate)
            return 0;
        ts.sp = frame + 4 * 3;
        return u32(*text);
    }
    eax = controller * kControllerBytes;
    if (std::uint8_t(gameSettings(app).deviceAt(eax).name[0]) != 0)
        eax += SETTINGS_ADDRESS(devices[0].name);
    else
    {
        const port::Result text = port::text::lineText(
            app, ts, ts.sp - 4, std::int32_t(kUnnamedControllerText), controller);
        if (ts.terminate)
            return 0;
        ts.sp = frame;
        eax = u32(*text);
    }
    putHalf(app, kControllerRowShown + controller * 2, 1);
    put(app, kControllerRowColour + controller * 4, kControllerColour);
    ts.sp += 4 * 3;
    return eax;
}

/* 0x4721f0: whether the name at eax will do for a player's: 1 to 10
 * characters.  0 for none. */
nfs::u32 text::nameValid(App* app, ThreadState& ts, u32 name)
{
    // ecx, edx, edi and ebp pushed by the code: no call inside, so no frame is kept here.
    NFS2_USE(ts);
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

constexpr u32 kCharacterKinds = 0x564ef0;  // the runtime's table of each byte's kind, from -1
constexpr u32 kLanguageChecks = 0x558c80;  // 0x14 bytes a language: its checks of a line, by mode
constexpr u32 kLanguageCheckBytes = 0x14;
constexpr u32 kShortName = 8;

/* The language's check of a line at `offset` of its row, through its
 * pointer; 1 when it has none.  Its eax; the pointer call keeps edx and ecx as it finds them. */
u32 languageCheck(nfs::App* app, nfs::ThreadState& ts, u32 line, u32 offset)
{
    ts.sp -= 4 * 3;  // ecx, edx and ebp pushed by the code
    const u32 row = gameSettings(app).language * 5 << 2;
    u32 eax = row;
    if (at(app, row + kLanguageChecks + offset) == 0)
        eax = withLow(eax, 1);
    else
    {
        nfs::Args check;
        check.eax = line;
        check.edx = line;
        check.ecx = row;
        const std::optional<u32> checked =
            guest::pointerCall(app, ts, at(app, row + kLanguageChecks + offset), check);
        if (!checked)
            return 0;
        eax = *checked;
    }
    ts.sp += 4 * 3;
    return eax;
}

}  // namespace

/* 0x443630: whether the character in al may be in a short name: a letter or
 * a digit, '_' or '-'.  ah is left al + 1. */
nfs::u32 text::shortNameCharacter(App* app, ThreadState& ts, u32 character)
{
    // edx and ebp pushed by the code: no call inside, so no frame is kept here.
    NFS2_USE(ts);
    const nfs::u8 al = nfs::u8(character);
    const nfs::u8 ah = nfs::u8(al + 1);
    nfs::u8 result;
    if (byte(app, u32(ah) + kCharacterKinds) & 0xe0)
        result = 1;
    else
        result = al == '_' || al == '-' ? 1 : 0;
    return (character & 0xffff0000) | (u32(ah) << 8) | result;
}

/* 0x443660: whether the text at eax is a short name: 0 to 8 of those. */
nfs::u32 text::shortNameValid(App* app, ThreadState& ts, u32 line)
{
    // ebx, ecx, edx, edi and ebp pushed by the code: the frame the calls are made in.
    if (line == 0)
        return 0;
    const u32 length = stringLength(app, line);
    if (nfs::s32(length) < 0 || nfs::s32(length) > nfs::s32(kShortName))
        return 0;
    ts.sp -= 4 * 5;
    u32 eax = 0;
    for (u32 i = 0; nfs::s32(i) < nfs::s32(length); ++i)
    {
        eax = guest::direct<0x443630, shortNameCharacter>(app, ts, u32(byte(app, line + i)));
        if (ts.terminate)
            return eax;
        if (nfs::u8(eax) == 0)
        {
            ts.sp += 4 * 5;
            return eax;
        }
    }
    ts.sp += 4 * 5;
    return withLow(eax, 1);
}

/* 0x443700, 0x443780: whether the language takes the line at eax as a
 * player's name (mode 1), as a file's (mode 2). */
nfs::u32 text::languageNameValid(App* app, ThreadState& ts, u32 line)
{
    return languageCheck(app, ts, line, 0);
}

nfs::u32 text::languageFileNameValid(App* app, ThreadState& ts, u32 line)
{
    return languageCheck(app, ts, line, 8);
}

/* 0x443860: whether the line at eax will do in mode edx: 0 a short name, 1
 * and 2 the language's checks; never an empty one, nor in another mode. */
nfs::u32 text::lineAllowed(App* app, ThreadState& ts, u32 line, u32 mode)
{
    // ebx and ebp pushed by the code: the frame the calls are made in.
    if (line == 0 || byte(app, line) == 0)
        return withLow(line, 0);
    nfs::u8 bl = 0;
    u32 eax = line;
    if (mode <= 2)
    {
        ts.sp -= 4 * 2;
        if (mode == 0)
            eax = guest::direct<0x443660, shortNameValid>(app, ts, line);
        else if (mode == 1)
            eax = guest::direct<0x443700, languageNameValid>(app, ts, line);
        else
            eax = guest::direct<0x443780, languageFileNameValid>(app, ts, line);
        if (ts.terminate)
            return eax;
        ts.sp += 4 * 2;
        bl = nfs::u8(eax);
    }
    return withLow(eax, bl);
}

/* The thunks under the game's names, the
 * arguments from the registers, the core, the ret. */
nfs::u32 displayModeName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::displayModeName(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 controllerName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::controllerName(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 nameValid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::nameValid(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 shortNameCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::shortNameCharacter(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 shortNameValid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::shortNameValid(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 languageNameValid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::languageNameValid(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 languageFileNameValid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::languageFileNameValid(app, ts, args.eax);
    ret(ts);
    return args.eax;
}

nfs::u32 lineAllowed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = text::lineAllowed(app, ts, args.eax, args.edx);
    ret(ts);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
