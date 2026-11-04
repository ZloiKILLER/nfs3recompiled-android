#include "native_guest.h"
#include "port_text.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <utility>

/* What the main menu calls of the game's own: a race started or the game
 * left, the attract mode's movies, the sliders' entries, the mouse and the
 * keyboard as keys.  disassemble_nfs3hp.py binds each address to the function
 * of that name here (FRONTEND_GLUE_NATIVES), and the generated code calls it
 * by name.  tools/game_checks holds each against its generated code.
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
 * functions are called through their imports' slots, every function of the
 * game's that is not this file's -- and those of this file's that the checks
 * stand in for, for the other files' natives -- by address. */
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
// The controls.
constexpr reg32 kGameMode = 0x6fd3b0;         // 1: two players on one screen
constexpr reg32 kControllers = 0x6fc970;      // 0x88 bytes each, the first byte's 4 a game pad
constexpr reg32 kFirstDigital = 0x6fd2dc, kSecondDigital = 0x6fd348;
// The attract mode and the movies.
constexpr reg32 kMoviesOff = 0x7a1fdc, kMovieFolder = 0x7a32b4;
constexpr reg32 kClock = 0x79c1a4, kAttractSince = 0x55e548, kAttractMovie = 0x55e54c;
constexpr reg32 kMouseBeforeX = 0x55e550, kMouseBeforeY = 0x55e554;
constexpr reg32 kAttractOff = 0x6fbb54, kMusicVolume = 0x6fbbe8, kAttractTicks = 0x1400;
constexpr reg32 kMovies[] = {0x53beac, 0x53beb8, 0x53bec4, 0x53bed0};   // titleav, demoav1-3.mad
// The network's locks.
constexpr reg32 kNetworkLock = 0x55e5b8, kNetworkLock2 = 0x55ecb4, kNetworkLock3 = 0x55ecb8;
constexpr reg32 kNetworkRecord = 0x79f918;
// The keyboard's ring.
constexpr reg32 kKeyRing = 0x7f6fe0, kKeyRead = 0x7f7474, kKeyLock = 0x7f7478;
constexpr reg32 kKeyWrite = 0x7f747c, kKeyRingSize = 0x7f7480;
// What is called by address.
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kMusicStreamMake = 0x410330, kMusicStart = 0x4104f0, kMusicStop = 0x4109d0;
constexpr reg32 kMusicStreamEnd = 0x410aa0;
constexpr reg32 kDialogShown = 0x4438b0;
constexpr reg32 kSliderSingle = 0x469520, kSliderRange = 0x46afd0;
constexpr reg32 kPlayMovieFile = 0x495bc0;
constexpr reg32 kNetworkSlot = 0x4a6690, kClearBuffers = 0x4d5d30;
constexpr reg32 kClearBuffersReset = 0x4d5d70, kFillZero = 0x4e070c;
constexpr reg32 kFileExists = 0x4e0e60;
constexpr reg32 kFreeLock = 0x4eb360;
constexpr reg32 kKeyCharacter = 0x4f0010, kFlushKeys = 0x4f0240;

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

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

/* mov r, [address]; sar r, 0x10: the word at address + 2, signed. */
reg32 high(win32::WinApplication* app, reg32 address)
{
    return reg32(sreg32(at(app, address)) >> 16);
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

/* A Windows function, through its import's slot. */
bool windows(win32::WinApplication* app, x86::CPU& cpu, reg32 slot)
{
    return byAddress(app, cpu, at(app, slot));
}

/* push ebp, mov ebp, esp after the registers named, and sub esp, `locals`;
 * mov esp, ebp, pop ebp, the registers popped and ret the other way. */
void enter(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 locals)
{
    for (reg32* r : saved)
        push(app, cpu, *r);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= locals;
}

void leave(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved)
{
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    for (auto r = std::rbegin(saved); r != std::rend(saved); ++r)
        **r = pop(app, cpu);
    cpu.esp += 4;
}

/* Two bytes a turn from esi to edi to the string's end: esi and al as the
 * copy leaves them, edi past what it wrote. */
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

/* push edi; the string at esi copied to edi; pop edi. */
void copyString(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    copyPairs(app, cpu);
    cpu.edi = pop(app, cpu);
}

/* cwde: ax sign-extended into eax. */
void cwde(x86::CPU& cpu)
{
    cpu.eax = reg32(sreg32(sreg16(cpu.ax)));
}

}  // namespace

void bindingIsDigital(win32::WinApplication* app, x86::CPU& cpu);
void noteDigitalControls(win32::WinApplication* app, x86::CPU& cpu);
void sliderValue(win32::WinApplication* app, x86::CPU& cpu);
void sliderSetOnMove(win32::WinApplication* app, x86::CPU& cpu);
void sliderSetColour(win32::WinApplication* app, x86::CPU& cpu);
void playMovie(win32::WinApplication* app, x86::CPU& cpu);
void resetAttractTimer(win32::WinApplication* app, x86::CPU& cpu);
void attractMode(win32::WinApplication* app, x86::CPU& cpu);
void freeNetworkLocks(win32::WinApplication* app, x86::CPU& cpu);
void allPlayersReady(win32::WinApplication* app, x86::CPU& cpu);
void mouseKey(win32::WinApplication* app, x86::CPU& cpu);
void pointerInBox(win32::WinApplication* app, x86::CPU& cpu);
void nextKey(win32::WinApplication* app, x86::CPU& cpu);

/* 0x43c000: whether binding eax is digital: 1 unless it is a controller's
 * (its low byte 1), then whether that controller (its top four bits) is a
 * game pad (4 in its record's first byte). */
void bindingIsDigital(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.edx <<= 24;
    cpu.edx = reg32(sreg32(cpu.edx) >> 24);
    if (cpu.edx != 1)
        cpu.eax = 1;
    else
    {
        cpu.eax >>= 28;
        cpu.edx = cpu.eax;
        cpu.eax <<= 4;
        cpu.eax += cpu.edx;
        cpu.eax = (byte(app, cpu.eax * 8 + kControllers) & 4) != 0 ? 1 : 0;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x43c040: the game mode's control set (0x4904d0) looked at: whether its
 * steering, throttle and brake are digital (0x6fd2dc on), and the second
 * player's (0x6fd348 on) with two on one screen. */
void noteDigitalControls(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kGameMode);
    if (!byAddress(app, cpu, 0x4904d0))
        return;
    cpu.ebx = at(app, kGameMode);
    cpu.edx = cpu.eax;
    cpu.ecx = cpu.eax;
    if (cpu.ebx == 1)
    {
        for (reg32 i = 0; i < 3; ++i)
        {
            cpu.eax = at(app, cpu.edx + 0x38 + 4 * i);
            if (!direct(app, cpu, bindingIsDigital))
                return;
            put(app, kSecondDigital + 4 * i, cpu.eax);
        }
    }
    for (reg32 i = 0; i < 3; ++i)
    {
        cpu.eax = at(app, cpu.ecx + 4 + 4 * i);
        if (!direct(app, cpu, bindingIsDigital))
            return;
        put(app, kFirstDigital + 4 * i, cpu.eax);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x46c9b0: slider eax's value onto the x87's stack -- a range's (kind 2)
 * end edx (0x46afd0), 0 for kind 1, a single one's (0x469520) else -- as a
 * float. */
void sliderValue(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx}, 4);
    const reg32 ebp = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.ebx = 0;
    cpu.cx = half(app, cpu.eax + 0x42);
    put(app, ebp - 4, cpu.ebx);
    // The table at 0x46c990: 0x46c9e0, 0x46c9ea, 0x46c9d6, 0x46c9e0, 0x46c9e0, 0x46c9e0.
    if (cpu.cx <= 5)
        cpu.ecx &= 0xffff;
    if (cpu.cx == 2)
    {
        if (!byAddress(app, cpu, kSliderRange))
            return;
        app->getMemory<float>(ebp - 4) = float(cpu.fpu.pop());
    }
    else if (cpu.cx != 1)
    {
        cpu.edx = 0;
        if (!byAddress(app, cpu, kSliderSingle))
            return;
        app->getMemory<float>(ebp - 4) = float(cpu.fpu.pop());
    }
    cpu.fpu.push(x86::Float(app->getMemory<float>(ebp - 4)));
    leave(app, cpu, {&cpu.ebx, &cpu.ecx});
}

/* 0x46cac0: the function slider eax's moves go to, edx (+0x78). */
void sliderSetOnMove(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, cpu.eax + 0x78, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x46cca0: slider eax's bar's colours, bl and dl (+0x3e, +0x3f). */
void sliderSetColour(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    putByte(app, cpu.eax + 0x3e, cpu.bl);
    putByte(app, cpu.eax + 0x3f, cpu.dl);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4960e0: movie eax played from the movies' folder (0x7a32b4), when movies
 * are on and the file is there (0x4e0e60): the keys flushed (0x4f0240), the
 * buffers cleared, the music's stream ended; the movie (0x495bc0) at the
 * quality the processor's speed allows (1 under 133 MHz, 2 under 200, 3),
 * told the processor is slow under 133; the buffers cleared again, the keys
 * flushed, the music's stream made again.  1. */
void playMovie(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x100;
    cpu.ebp -= 0x82;
    const reg32 ebp = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, kMoviesOff);
    if (cpu.edx == 0)
    {
        cpu.esi = kMovieFolder;
        cpu.edi = ebp - 0x7e;
        copyString(app, cpu);
        // strcat: the name after the folder's.
        cpu.edi = ebp - 0x7e;
        cpu.esi = cpu.ecx;
        push(app, cpu, cpu.edi);
        cpu.ecx = 0;
        --cpu.ecx;
        cpu.al = 0;
        while (cpu.ecx != 0)
        {
            --cpu.ecx;
            const reg8 c = byte(app, cpu.edi);
            ++cpu.edi;
            if (c == cpu.al)
                break;
        }
        --cpu.edi;
        copyPairs(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.eax = ebp - 0x7e;
        if (!byAddress(app, cpu, kFileExists))
            return;
        if (cpu.eax != 0)
        {
            if (!byAddress(app, cpu, kFlushKeys) || !byAddress(app, cpu, kClearBuffers)
                || !byAddress(app, cpu, kMusicStreamEnd))
                return;
            // The movies' best quality, every frame: no phone is too slow for them.
            put(app, ebp + 0x7e, cpu.edx);
            cpu.ecx = 3;
            push(app, cpu, 1);
            cpu.edx = ebp + 0x7e;
            cpu.eax = ebp - 0x7e;
            cpu.ebx = 0;
            if (!byAddress(app, cpu, kPlayMovieFile) || !byAddress(app, cpu, kClearBuffersReset)
                || !byAddress(app, cpu, kFlushKeys) || !byAddress(app, cpu, kMusicStreamMake))
                return;
        }
    }
    cpu.eax = 1;
    cpu.esp = ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4961e0: the attract mode's wait started again, from the clock now. */
void resetAttractTimer(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kClock);
    put(app, kAttractSince, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x496200: the main menu's attract mode, every pass: with movies on and it
 * not off (0x6fbb54), its wait started again while a dialog is up or as the
 * mouse moves (0); 0x1400 ticks on, the music stopped, the next of its four
 * movies played (0x55e54c, 0 to 3), the menus' tune started again, 1. */
void attractMode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (at(app, kMoviesOff) != 0 || at(app, kAttractOff) != 0)
    {
        cpu.eax = 0;
        return done();
    }
    if (!byAddress(app, cpu, kDialogShown))
        return;
    if (cpu.al != 0 && !direct(app, cpu, resetAttractTimer))
        return;
    cpu.ebx = at(app, kMouseX);
    bool moved = cpu.ebx != at(app, kMouseBeforeX);
    if (!moved)
    {
        cpu.esi = at(app, kMouseY);
        moved = cpu.esi != at(app, kMouseBeforeY);
    }
    if (moved)
    {
        cpu.eax = at(app, kMouseX);
        put(app, kMouseBeforeX, cpu.eax);
        cpu.eax = at(app, kMouseY);
        put(app, kMouseBeforeY, cpu.eax);
        if (!direct(app, cpu, resetAttractTimer))
            return;
        cpu.eax = 0;
        return done();
    }
    cpu.eax = at(app, kClock);
    cpu.eax -= at(app, kAttractSince);
    if (sreg32(cpu.eax) < sreg32(kAttractTicks))
    {
        cpu.eax = 0;
        return done();
    }
    if (!byAddress(app, cpu, kMusicStop))
        return;
    cpu.eax = at(app, kAttractMovie);
    // The table at 0x4961f0: 0x4962a3, 0x4962aa, 0x4962b1, 0x4962b8.
    if (cpu.eax <= 3)
    {
        cpu.eax = kMovies[cpu.eax];
        if (!direct(app, cpu, playMovie))
            return;
    }
    cpu.eax = 0x16;
    cpu.edx = at(app, kMusicVolume);
    cpu.ebx = 0;
    if (!byAddress(app, cpu, kMusicStart))
        return;
    cpu.edx = at(app, kAttractMovie);
    ++cpu.edx;
    cpu.ecx = 4;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    const std::int64_t dividend = std::int64_t(sreg32(cpu.eax));
    cpu.eax = reg32(sreg32(dividend / 4));
    cpu.edx = reg32(sreg32(dividend % 4));
    put(app, kAttractMovie, cpu.edx);
    if (!direct(app, cpu, resetAttractTimer))
        return;
    cpu.eax = 1;
    done();
}

/* 0x49a4d0: the network's three locks freed (0x4eb360) and forgotten, the
 * record at 0x79f918 cleared. */
void freeNetworkLocks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kNetworkLock);
    if (!byAddress(app, cpu, kFreeLock))
        return;
    cpu.edx = 0;
    cpu.eax = at(app, kNetworkLock2);
    put(app, kNetworkLock, cpu.edx);
    if (!byAddress(app, cpu, kFreeLock))
        return;
    cpu.eax = at(app, kNetworkLock3);
    put(app, kNetworkLock2, cpu.edx);
    if (!byAddress(app, cpu, kFreeLock))
        return;
    cpu.eax = kNetworkRecord;
    put(app, kNetworkLock3, cpu.edx);
    cpu.edx = kNetworkPlayerBytes;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49ab60: 1 when in a network race of two machines or more every other
 * machine's player (0x4a6690) is ready (0x79f5f8, 0x60 a player); else 0. */
void allPlayersReady(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.eax = 0;
    if (at(app, kNetworkRace) == 0 || sreg16(half(app, kNetworkPlayers + 2)) < 2)
        return done();
    cpu.edx = 1;
    for (;;)
    {
        cpu.eax = high(app, kNetworkPlayers);
        if (sreg32(cpu.edx) >= sreg32(cpu.eax))
            break;
        cpu.eax = reg32(sreg32(sreg16(cpu.dx)));
        if (!byAddress(app, cpu, kNetworkSlot))
            return;
        cwde(cpu);
        if (cpu.eax == 0xffffffff)
        {
            cpu.eax = 0;
            return done();
        }
        cpu.ecx = cpu.eax;
        cpu.eax <<= 2;
        cpu.eax -= cpu.ecx;
        cpu.eax <<= 5;
        if (byte(app, cpu.eax + kNetworkPlayerReady) == 0)
        {
            cpu.eax = 0;
            return done();
        }
        ++cpu.edx;
    }
    cpu.eax = 1;
    done();
}

/* 0x49c080: the mouse read into the menus' (0x55e5f4 its buttons, 0x55e5e4
 * and 0x55e5e8 the pointer, 0x55e5f8 the wheel) as a key: a button pressed,
 * the click forgotten (0x55e5ec) and Enter (0xd) for the left, Escape (0x1b)
 * for another; none, 0. */
void mouseKey(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = kMouseWheel;
    cpu.ebx = kMouseY;
    cpu.edx = kMouseX;
    cpu.eax = kMouseButtons;
    if (!direct(app, cpu, port::text::readMenuMouse))
        return;
    cpu.ah = byte(app, kMouseButtons);
    if ((cpu.ah & 0xf) != 0)
    {
        cpu.edx = 0;
        put(app, kMouseClick, cpu.edx);
        cpu.eax = (cpu.ah & 1) != 0 ? 0xd : 0x1b;
    }
    else
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49c1f0: whether the menus' pointer is inside box eax (y, x, width,
 * height, words), edges out. */
void pointerInBox(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    bool inside = false;
    cpu.edx = at(app, cpu.eax);
    cpu.ecx = at(app, kMouseX);
    cpu.edx = reg32(sreg32(cpu.edx) >> 16);
    if (sreg32(cpu.edx) < sreg32(cpu.ecx))
    {
        cpu.ecx = high(app, cpu.eax + 2);
        cpu.ebx = at(app, kMouseX);
        cpu.edx += cpu.ecx;
        if (sreg32(cpu.edx) > sreg32(cpu.ebx))
        {
            cpu.esi = at(app, kMouseY);
            cpu.edx = reg32(sreg32(sreg16(half(app, cpu.eax))));
            if (sreg32(cpu.edx) < sreg32(cpu.esi))
            {
                cpu.eax = high(app, cpu.eax + 4);
                cpu.eax += cpu.edx;
                inside = sreg32(cpu.eax) > sreg32(cpu.esi);
            }
        }
    }
    cpu.eax = inside ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4cc970: the next key off the keyboard's ring, under its lock: its scan
 * code in the high word, its character -- the ring's, or the scan code's
 * with the shifts of its low byte (0x4f0010) -- in the low; keys without one
 * passed over; none, 0. */
void nextKey(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 4);
    const reg32 ebp = cpu.ebp;
    cpu.edx = at(app, kKeyLock);
    push(app, cpu, cpu.edx);
    cpu.esi = 0;
    cpu.ebx = 0;
    if (!windows(app, cpu, kEnterCriticalSection))
        return;
    for (;;)
    {
        cpu.ecx = at(app, kKeyWrite);
        cpu.eax = at(app, kKeyRead);
        if (cpu.eax == cpu.ecx || cpu.ebx != 0)
            break;
        cpu.eax = at(app, cpu.eax * 4 + kKeyRing);
        put(app, ebp - 4, cpu.eax);
        cpu.ebx = cpu.eax;
        cpu.esi = byte(app, ebp - 3);
        cpu.ebx = reg32(sreg32(cpu.ebx) >> 16);
        if (cpu.ebx == 0)
        {
            cpu.edx = 0;
            cpu.eax = cpu.esi;
            cpu.dl = byte(app, ebp - 4);
            if (!byAddress(app, cpu, kKeyCharacter))
                return;
            cpu.ebx = cpu.eax;
        }
        cpu.edx = at(app, kKeyRead);
        ++cpu.edx;
        cpu.ecx = at(app, kKeyRingSize);
        cpu.eax = cpu.edx;
        cpu.edx = reg32(sreg32(cpu.edx) >> 31);
        const std::int64_t dividend = std::int64_t(sreg32(cpu.eax));
        cpu.eax = reg32(sreg32(dividend / sreg32(cpu.ecx)));
        cpu.edx = reg32(sreg32(dividend % sreg32(cpu.ecx)));
        put(app, kKeyRead, cpu.edx);
    }
    cpu.edi = at(app, kKeyLock);
    push(app, cpu, cpu.edi);
    if (!windows(app, cpu, kLeaveCriticalSection))
        return;
    cpu.eax = cpu.esi;
    cpu.eax <<= 16;
    cpu.eax |= cpu.ebx;
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi});
}

}  // namespace frontend
}  // namespace nfs3hp
