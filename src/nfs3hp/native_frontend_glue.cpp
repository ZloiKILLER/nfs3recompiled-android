#include "native_frontend_glue.h"
#include "native_frontend_draw.h"
#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_windows.h"
#include "host_threads.h"
#include "port_dialogs.h"
#include "port_gamecall.h"
#include "port_sound_music.h"
#include "native_ea_formats.h"
#include "port_text.h"
#include "native_graphics.h"
#include <lib/winapp.h>
#include <winapi/types.h>

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <utility>

/* What the main menu calls of the game's own: a race started or the game
 * left, the attract mode's movies, the sliders' entries, the mouse and the
 * keyboard as keys.
 *
 * The menus' widgets -- the lists by name, the buttons' and the lists' keys,
 * the HUD's art loaded -- are port_buttons.cpp's.  A slider (an item of a
 * screen, 188 bytes, the table at 0x604ee0): its kind (0x42), the function
 * its moves go to (0x78).
 *
 * The attract mode: after 0x1400 ticks of the clock (0x79c1a4) with the
 * mouse still and no dialog up, the music stops and the next of titleav.mad,
 * demoav1-3.mad plays from the movies' folder (0x7a32b4), unless movies are
 * off (0x7a1fdc); the keys flushed around it.  And the keyboard's ring
 * (0x7f6fe0, its read and write indexes and size at 0x7f7474, 0x7f747c,
 * 0x7f7480, under the lock at 0x7f7478) taken a key at a time; the network's
 * locks freed; the controls' bindings that are digital noted.  The menus'
 * text and the mouse put on their 640 by 480 screen are port_text.cpp's.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them.  The switches go by the targets
 * nfs3.exe's tables hold (0x46c930, 0x46c990, 0x46cac8, 0x4961f0).  Windows'
 * functions are the port's own, called as C++; the typed cores of the port's and the libraries' --
 * the music's, the dialogs', the files', the locks', the control sets -- as
 * C++ too (guest::direct: every register but eax kept, as they keep them), every other function of the
 * game's by address.
 *
 * Plan step 5: each function a typed core in frontend::glue (native_frontend_glue.h) -- its
 * arguments into the registers, the code run, eax back, the ret left to the thunk under the
 * game's name, which the calls by address enter.  A core of this file calls another, and
 * native_frontend_draw.cpp's, as C++ (guest::direct). */
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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg16;
using x86::sreg32;

// The menus.
constexpr reg32 kNetworkRace = 0x7a22e0;
constexpr reg32 kNetworkPlayers = 0x7a2274;   // the word at 0x7a2276 how many
constexpr reg32 kNetworkPlayerReady = 0x79f5f8, kNetworkPlayerBytes = 0x60;
constexpr reg32 kMouseX = 0x55e5e4, kMouseY = 0x55e5e8, kMouseClick = 0x55e5ec;
constexpr reg32 kMouseButtons = 0x55e5f4, kMouseWheel = 0x55e5f8;
constexpr reg32 kSliderFlags = 0x55eb2e;      // 0x20 a demo's command line, 0x10 ...
// The attract mode and the movies.
constexpr reg32 kMoviesOff = 0x7a1fdc, kMovieFolder = 0x7a32b4;
constexpr reg32 kClock = 0x79c1a4, kAttractSince = 0x55e548, kAttractMovie = 0x55e54c;
constexpr reg32 kMouseBeforeX = 0x55e550, kMouseBeforeY = 0x55e554;
constexpr reg32 kAttractTicks = 0x1400;
constexpr reg32 kMovies[] = {0x53beac, 0x53beb8, 0x53bec4, 0x53bed0};   // titleav, demoav1-3.mad
// The network's locks.
constexpr reg32 kNetworkLock = 0x55e5b8, kNetworkLock2 = 0x55ecb4, kNetworkLock3 = 0x55ecb8;
constexpr reg32 kNetworkRecord = 0x79f918;
// The keyboard's ring.
constexpr reg32 kKeyRing = 0x7f6fe0, kKeyRead = 0x7f7474, kKeyLock = 0x7f7478;
constexpr reg32 kKeyWrite = 0x7f747c, kKeyRingSize = 0x7f7480;
// What is called by address.
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kPlayMovieFile = 0x495bc0;
constexpr reg32 kNetworkSlot = 0x4a6690, kClearBuffers = 0x4d5d30;
constexpr reg32 kClearBuffersReset = 0x4d5d70, kFillZero = 0x4e070c;
constexpr reg32 kKeyCharacter = 0x4f0010, kFlushKeys = 0x4f0240;

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

/* mov r, [address]; sar r, 0x10: the word at address + 2, signed. */
reg32 high(win32::WinApplication* app, reg32 address)
{
    return reg32(sreg32(at(app, address)) >> 16);
}

/* A function of the game's by address, the registers it reads set by the caller: the return
 * address's slot, and esp as the callee leaves it.  False when the game stopped in it. */
bool callAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target, const port::Args& in, reg32& eax,
               std::initializer_list<reg32> stack = {})
{
    return port::gameCall(app, cpu, cpu.esp, target, in, stack, eax);
}

/* A Windows function, its words handed: the port's own, called as C++ (native_windows.h); in the
 * check through its import's slot, as the code calls it.  False when the game stopped in it. */
template <reg32 Slot, auto Function, typename... Words>
bool windows(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    return guest::windows<Slot, Function>(app, cpu, words...).has_value();
}

/* ret, the thunks' end: the return address popped, unless the game stopped
 * in the function. */
void ret(x86::CPU& cpu)
{
    if (!cpu.terminate)
        cpu.esp += 4;
}

/* The string at `from` copied to `to`, its 0 too; the string's length. */
reg32 copyString(win32::WinApplication* app, reg32 to, reg32 from)
{
    reg32 length = 0;
    for (;; ++length)
    {
        const reg8 c = byte(app, from + length);
        putByte(app, to + length, c);
        if (c == 0)
            return length;
    }
}

}  // namespace

x86::reg32 bindingIsDigital(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 noteDigitalControls(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sliderValue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sliderSetOnMove(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sliderSetColour(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 playMovie(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
void resetAttractTimer(win32::WinApplication* app, x86::CPU& cpu);
x86::reg32 attractMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
void freeNetworkLocks(win32::WinApplication* app, x86::CPU& cpu);
x86::reg32 allPlayersReady(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 mouseKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 pointerInBox(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nextKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

/* 0x43c000: whether binding eax is digital: 1 unless it is a controller's
 * (its low byte 1), then whether that controller (its top four bits) is a
 * game pad (4 in its record's first byte). */
x86::reg32 glue::bindingIsDigital(App* app, CPU& cpu, reg32 binding)
{
    // edx and ebp pushed by the code: no call inside, no frame kept here.
    NFS2_USE(cpu);
    if (reg32(sreg32(binding << 24) >> 24) != 1)
        return 1;
    const reg32 controller = binding >> 28;
    return (std::uint8_t(gameSettings(app).deviceAt((controller * 17) * 8).kind) & 4) != 0 ? 1 : 0;
}

/* 0x43c040: the game mode's control set (0x4904d0) looked at: whether its
 * steering, throttle and brake are digital (0x6fd2dc on), and the second
 * player's (0x6fd348 on) with two on one screen.  The eax it leaves: the last one's. */
x86::reg32 glue::noteDigitalControls(App* app, CPU& cpu)
{
    cpu.esp -= 4 * 4;  // ebx, ecx, edx and ebp pushed by the code
    const reg32 set = guest::direct<0x4904d0, controls::controlSets>(app, cpu, gameSettings(app).playMode);
    if (cpu.terminate)
        return set;
    reg32 eax = set;
    if (gameSettings(app).playMode == 1)
    {
        for (reg32 i = 0; i < 3; ++i)
        {
            eax = guest::direct<0x43c000, bindingIsDigital>(app, cpu, at(app, set + 0x38 + 4 * i));
            if (cpu.terminate)
                return eax;
            gameSettings(app).players[1].digital[i] = eax;
        }
    }
    for (reg32 i = 0; i < 3; ++i)
    {
        eax = guest::direct<0x43c000, bindingIsDigital>(app, cpu, at(app, set + 4 + 4 * i));
        if (cpu.terminate)
            return eax;
        gameSettings(app).players[0].digital[i] = eax;
    }
    cpu.esp += 4 * 4;
    return eax;
}

/* 0x46c9b0: slider eax's value -- a range's (kind 2) end edx (0x46afd0), 0
 * for kind 1, a single one's (0x469520) else -- as a float, which its thunk
 * leaves on the x87's stack.  eax stays the slider. */
double glue::sliderValue(App* app, CPU& cpu, reg32 slider, reg32 end)
{
    // ebx, ecx and ebp pushed by the code, then the float's local at [ebp - 4]: the frame.
    cpu.esp -= 4 * 3 + 4;
    const reg32 local = cpu.esp;
    const reg16 kind = half(app, slider + 0x42);
    put(app, local, 0);
    // The table at 0x46c990: 0x46c9e0, 0x46c9ea, 0x46c9d6, 0x46c9e0, 0x46c9e0, 0x46c9e0.
    if (kind == 2)
    {
        const double value = guest::direct<0x46afd0, draw::twoAxisValue>(app, cpu, slider, end);
        if (cpu.terminate)
            return 0.0;
        app->getMemory<float>(local) = float(value);
    }
    else if (kind != 1)
    {
        const double value = guest::direct<0x469520, draw::sliderStoredValue>(app, cpu, slider);
        if (cpu.terminate)
            return 0.0;
        app->getMemory<float>(local) = float(value);
    }
    const x86::Float value = x86::Float(app->getMemory<float>(local));
    cpu.esp += 4 * 3 + 4;
    return value;
}

/* 0x46cac0: the function slider eax's moves go to, edx (+0x78). */
void glue::sliderSetOnMove(App* app, CPU& cpu, reg32 slider, reg32 function)
{
    NFS2_USE(cpu);  // ebp pushed and popped by the code: no value kept
    put(app, slider + 0x78, function);
}

/* 0x46cca0: slider eax's bar's colours, bl and dl (+0x3e, +0x3f). */
void glue::sliderSetColour(App* app, CPU& cpu, reg32 slider, reg32 second, reg32 first)
{
    NFS2_USE(cpu);
    putByte(app, slider + 0x3e, reg8(first));
    putByte(app, slider + 0x3f, reg8(second));
}

/* 0x4960e0: movie eax played from the movies' folder (0x7a32b4), when movies
 * are on and the file is there (0x4e0e60): the keys flushed (0x4f0240), the
 * buffers cleared, the music's stream ended; the movie (0x495bc0) at the
 * quality the processor's speed allows (1 under 133 MHz, 2 under 200, 3),
 * told the processor is slow under 133; the buffers cleared again, the keys
 * flushed, the music's stream made again.  1. */
x86::reg32 glue::playMovie(App* app, CPU& cpu, reg32 movie)
{
    // The functions of the game's called by address read the registers set here and keep the others.
    reg32 eax = movie;  // what the code's eax holds as it goes
    // ebx, ecx, edx, esi, edi and ebp pushed by the code, then 0x100 bytes: the name is built at the
    // frame's bottom, ebp - 0x7e.
    const reg32 ebp = cpu.esp - 4 * 6 - 0x82;
    cpu.esp -= 4 * 6 + 0x100;
    const reg32 name = ebp - 0x7e;
    if (at(app, kMoviesOff) == 0)
    {
        const reg32 folder = copyString(app, name, kMovieFolder);
        copyString(app, name + folder, movie);
        const std::optional<bool> there = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
        if (!there)
            return 0;
        if (*there)
        {
            // None of these read eax: the clears leave it, the flush gives 0.
            const std::optional<reg32> flushed = guest::direct<kFlushKeys, ea::menuKeysFlushed>(app, cpu);
            if (!flushed)
                return 0;
            eax = *flushed;
            const std::optional<reg32> cleared = guest::direct<0x4d5d30, graphics::clearedTwice>(app, cpu);
            if (!cleared)
                return 0;
            eax = *cleared;
            // The stream's end hands ebx and ecx on to a call that reads neither.
            const port::Result ended = guest::direct<0x410aa0, port::musicStreamEndOf>(
                app, cpu, cpu.esp - 4, eax, 0, 0xfffffffeu - folder);
            if (!ended)
                return 0;
            // The movies' best quality, every frame: no phone is too slow for them.
            put(app, ebp + 0x7e, 0);
            if (!callAddress(app, cpu, kPlayMovieFile, {name, ebp + 0x7e, 0, 3}, eax, {1}))
                return 0;
            const std::optional<reg32> reset = guest::direct<0x4d5d70, graphics::clearedTwiceAndReset>(app, cpu);
            if (!reset)
                return 0;
            eax = *reset;
            const std::optional<reg32> flushedAgain = guest::direct<kFlushKeys, ea::menuKeysFlushed>(app, cpu);
            if (!flushedAgain)
                return 0;
            eax = *flushedAgain;
            const port::Result made = guest::direct<0x410330, port::musicStreamMade>(app, cpu, cpu.esp - 4, eax, 3);
            if (!made)
                return 0;
        }
    }
    return 1;
}

/* 0x4961e0: the attract mode's wait started again, from the clock now. */
void glue::resetAttractTimer(App* app, CPU& cpu)
{
    NFS2_USE(cpu);  // ebp pushed and popped by the code: no value kept; no caller reads its eax
    put(app, kAttractSince, at(app, kClock));
}

/* 0x496200: the main menu's attract mode, every pass: with movies on and it
 * not off (0x6fbb54), its wait started again while a dialog is up or as the
 * mouse moves (0); 0x1400 ticks on, the music stopped, the next of its four
 * movies played (0x55e54c, 0 to 3), the menus' tune started again, 1. */
x86::reg32 glue::attractMode(App* app, CPU& cpu)
{
    cpu.esp -= 4 * 5;  // ebx, ecx, edx, esi and ebp pushed by the code
    if (at(app, kMoviesOff) != 0 || gameSettings(app).installIsRemote != 0)
    {
        cpu.esp += 4 * 5;
        return 0;
    }
    // Whether a dialog is up (port_dialogs.cpp's).
    const bool dialogUp = guest::direct<0x4438b0, port::dialogs::dialogShownOf>(app, cpu) != 0;
    if (cpu.terminate)
        return 0;
    if (dialogUp)
    {
        guest::direct<0x4961e0, resetAttractTimer>(app, cpu);
        if (cpu.terminate)
            return 0;
    }
    const bool moved = at(app, kMouseX) != at(app, kMouseBeforeX) || at(app, kMouseY) != at(app, kMouseBeforeY);
    if (moved)
    {
        put(app, kMouseBeforeX, at(app, kMouseX));
        put(app, kMouseBeforeY, at(app, kMouseY));
        guest::direct<0x4961e0, resetAttractTimer>(app, cpu);
        if (cpu.terminate)
            return 0;
        cpu.esp += 4 * 5;
        return 0;
    }
    const reg32 waited = at(app, kClock) - at(app, kAttractSince);
    if (sreg32(waited) < sreg32(kAttractTicks))
    {
        cpu.esp += 4 * 5;
        return 0;
    }
    const port::Result stopped = guest::direct<0x4109d0, port::musicStopped>(app, cpu, cpu.esp - 4, waited);
    if (!stopped)
        return 0;
    const reg32 movie = at(app, kAttractMovie);
    // The table at 0x4961f0: 0x4962a3, 0x4962aa, 0x4962b1, 0x4962b8.
    if (movie <= 3)
    {
        guest::direct<0x4960e0, playMovie>(app, cpu, kMovies[movie]);
        if (cpu.terminate)
            return 0;
    }
    const port::Result started = guest::direct<0x4104f0, port::musicStarted>(
        app, cpu, cpu.esp - 4, std::int32_t(0x16), std::int32_t(gameSettings(app).menuMusicVolume), std::int32_t(0));
    if (!started)
        return 0;
    put(app, kAttractMovie, reg32(sreg32(at(app, kAttractMovie) + 1) % 4));
    guest::direct<0x4961e0, resetAttractTimer>(app, cpu);
    if (cpu.terminate)
        return 0;
    cpu.esp += 4 * 5;
    return 1;
}

/* 0x49a4d0: the network's three locks freed (0x4eb360) and forgotten, the
 * record at 0x79f918 cleared. */
void glue::freeNetworkLocks(App* app, CPU& cpu)
{
    cpu.esp -= 4 * 2;  // edx and ebp pushed by the code
    // Lock `lock` freed (native_ea_threads.cpp's); false when the game stopped.
    const auto freeLock = [&](reg32 lock) { return guest::direct<0x4eb360, ea::lockFreed>(app, cpu, lock).has_value(); };
    if (!freeLock(at(app, kNetworkLock)))
        return;
    put(app, kNetworkLock, 0);
    if (!freeLock(at(app, kNetworkLock2)))
        return;
    put(app, kNetworkLock2, 0);
    if (!freeLock(at(app, kNetworkLock3)))
        return;
    put(app, kNetworkLock3, 0);
    guest::direct<0x4e070c, ea::clearBytes>(app, cpu, kNetworkRecord, kNetworkPlayerBytes);
    if (cpu.terminate)
        return;
    cpu.esp += 4 * 2;
}

/* 0x49ab60: 1 when in a network race of two machines or more every other
 * machine's player (0x4a6690) is ready (0x79f5f8, 0x60 a player); else 0. */
x86::reg32 glue::allPlayersReady(App* app, CPU& cpu)
{
    cpu.esp -= 4 * 3;  // ecx, edx and ebp pushed by the code
    if (at(app, kNetworkRace) == 0 || sreg16(half(app, kNetworkPlayers + 2)) < 2)
        return 0;
    for (reg32 player = 1; sreg32(player) < sreg32(high(app, kNetworkPlayers)); ++player)
    {
        const reg32 slot = reg32(sreg32(sreg16(reg16(guest::direct<0x4a6690, ea::machineOfPlayerNumber>(
            app, cpu, reg32(sreg32(sreg16(reg16(player)))))))));
        if (slot == 0xffffffff)
            return 0;
        if (byte(app, slot * 0x60 + kNetworkPlayerReady) == 0)
            return 0;
    }
    return 1;
}

/* 0x49c080: the mouse read into the menus' (0x55e5f4 its buttons, 0x55e5e4
 * and 0x55e5e8 the pointer, 0x55e5f8 the wheel) as a key: a button pressed,
 * the click forgotten (0x55e5ec) and Enter (0xd) for the left, Escape (0x1b)
 * for another; none, 0. */
x86::reg32 glue::mouseKey(App* app, CPU& cpu)
{
    const reg32 esp = cpu.esp;
    cpu.esp -= 4 * 4;  // ebx, ecx, edx and ebp pushed by the code
    // The port's mouse read (port_text.cpp), as C++, from where the thunk finds the stack.
    const bool read = port::text::mouseRead(app, cpu, cpu.esp - 4, kMouseButtons, kMouseX, kMouseY, kMouseWheel);
    cpu.esp = esp;
    if (!read)
        return 0;
    const reg8 buttons = byte(app, kMouseButtons);
    if ((buttons & 0xf) == 0)
        return 0;
    put(app, kMouseClick, 0);
    return (buttons & 1) != 0 ? 0xd : 0x1b;
}

/* 0x49c1f0: whether the menus' pointer is inside box eax (y, x, width,
 * height, words), edges out. */
x86::reg32 glue::pointerInBox(App* app, CPU& cpu, reg32 box)
{
    // ebx, ecx, edx, esi and ebp pushed by the code: no call inside, no frame kept here.
    NFS2_USE(cpu);
    const reg32 left = high(app, box);
    if (!(sreg32(left) < sreg32(at(app, kMouseX))))
        return 0;
    if (!(sreg32(left + high(app, box + 2)) > sreg32(at(app, kMouseX))))
        return 0;
    const reg32 top = reg32(sreg32(sreg16(half(app, box))));
    if (!(sreg32(top) < sreg32(at(app, kMouseY))))
        return 0;
    return sreg32(high(app, box + 4) + top) > sreg32(at(app, kMouseY)) ? 1 : 0;
}

/* 0x4cc970: the next key off the keyboard's ring, under its lock: its scan
 * code in the high word, its character -- the ring's, or the scan code's
 * with the shifts of its low byte (0x4f0010) -- in the low; keys without one
 * passed over; none, 0. */
x86::reg32 glue::nextKey(App* app, CPU& cpu)
{
    // ebx, ecx, edx, esi, edi and ebp pushed by the code, then the local at [ebp - 4].
    const reg32 ebp = cpu.esp - 4 * 6;
    cpu.esp -= 4 * 6 + 4;
    reg32 scan = 0, character = 0;
    if (!windows<kEnterCriticalSection, &host::enterLock>(app, cpu, at(app, kKeyLock)))
        return 0;
    for (;;)
    {
        const reg32 write = at(app, kKeyWrite), read = at(app, kKeyRead);
        if (read == write || character != 0)
            break;
        const reg32 entry = at(app, read * 4 + kKeyRing);
        put(app, ebp - 4, entry);
        scan = byte(app, ebp - 3);
        character = reg32(sreg32(entry) >> 16);
        if (character == 0)
        {
            // scan in eax and `write` in ecx are what the character function reads.
            const std::optional<reg32> typed = guest::direct<kKeyCharacter, ea::scanCodeCharacter>(app, cpu, scan, write);
            if (!typed)
                return 0;
            character = *typed;
        }
        put(app, kKeyRead, reg32(std::int64_t(sreg32(read + 1)) % sreg32(at(app, kKeyRingSize))));
    }
    if (!windows<kLeaveCriticalSection, &host::leaveLock>(app, cpu, at(app, kKeyLock)))
        return 0;
    return (scan << 16) | character;
}

/* The thunks under the game's names, the
 * arguments from the registers, the core, the ret. */
x86::reg32 bindingIsDigital(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::bindingIsDigital(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 noteDigitalControls(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::noteDigitalControls(app, cpu);
    ret(cpu);
    return args.eax;
}

x86::reg32 sliderValue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 slider = args.eax;
    const double value = glue::sliderValue(app, cpu, slider, args.edx);
    if (!cpu.terminate)
    {
        args.floatSet = true;
        args.floatResult = value;
        // The code zeroes edx for a single slider's value (kinds but 1 and 2): no caller reads it.
        const reg16 kind = app->getMemory<reg16>(slider + 0x42);
        if (kind != 1 && kind != 2)
            args.edx = 0;
    }
    ret(cpu);
    return args.eax;
}

x86::reg32 sliderSetOnMove(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    glue::sliderSetOnMove(app, cpu, args.eax, args.edx);
    ret(cpu);
    return args.eax;
}

x86::reg32 sliderSetColour(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    glue::sliderSetColour(app, cpu, args.eax, args.edx, args.ebx);
    ret(cpu);
    return args.eax;
}

x86::reg32 playMovie(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::playMovie(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 resetAttractTimer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    glue::resetAttractTimer(app, cpu);
    args.eax = guest::at(app, kClock);  // what its code leaves in eax: no caller reads it
    ret(cpu);
    return args.eax;
}

x86::reg32 attractMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::attractMode(app, cpu);
    ret(cpu);
    return args.eax;
}

x86::reg32 freeNetworkLocks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    glue::freeNetworkLocks(app, cpu);
    args.eax = kNetworkRecord + kNetworkPlayerBytes;  // past the record cleared: no caller reads it
    ret(cpu);
    return args.eax;
}

x86::reg32 allPlayersReady(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::allPlayersReady(app, cpu);
    ret(cpu);
    return args.eax;
}

x86::reg32 mouseKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::mouseKey(app, cpu);
    ret(cpu);
    return args.eax;
}

x86::reg32 pointerInBox(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::pointerInBox(app, cpu, args.eax);
    ret(cpu);
    return args.eax;
}

x86::reg32 nextKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = glue::nextKey(app, cpu);
    ret(cpu);
    return args.eax;
}

}  // namespace frontend
}  // namespace nfs3hp
