#ifndef NFS3HP_PORT_SOUND_MUSIC_H
#define NFS3HP_PORT_SOUND_MUSIC_H

#include "native_guest.h"
#include "port.h"

/* port_sound_music.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress below, for the checks' stand-ins); musicStopped and musicStarted are declared in
 * port_sound_slice.h as well, which holds the game's addresses this one may not (a header's numbers count as
 * named by every file including it: tools/game_checks/generate.py).  A Result is nullopt when the game stopped
 * in a call.  A core's `eax`, where it takes one, is what the game's code has in eax at the call (the function
 * gives it back where it leaves eax alone).  A core's `esp` is the frame base its adapter hands it: the game's
 * esp on entry, at the return address.  A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack
 * words the game's call pushes: the esp the adapter hands it, so the core's frame lies where the game's does;
 * by address the call makes it.  Not here: 0x4103a0 (the music after a change of setting) and 0x4107c0 (a
 * race's music started), whose adapters do their work on the registers themselves. */
namespace nfs3hp
{
namespace port
{

// The game's 0x40feb0: SND ended at the game's exit, with the sound or the music on; `in` the caller's registers,
// handed on to SNDrestore; esp the frame base.
Result soundExitOf(App* app, CPU& cpu, reg32 esp, const Args& in);
// The game's 0x410330: the music's CD stream made when the music is on (`ecx` handed on): the stream, else `eax`;
// esp the frame base.
Result musicStreamMade(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx);
// The game's 0x410370: the music's CD stream of `eax` KB made when the music is on (`ecx` handed on): the
// stream, else `eax`; esp the frame base.
Result musicStreamOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ecx);
// The game's 0x410460: the menus' tune 0x18 while there is no other asked for; esp the frame base.
Result musicMenuSixOf(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4104f0: tune `tune` started at `volume` (`own`: a race's own tune); esp the frame base.
Result musicStarted(App* app, CPU& cpu, reg32 esp, int32_t tune, int32_t volume, int32_t own);
// The game's 0x4109d0: the music stopped, if it plays; esp the frame base.
Result musicStopped(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x410aa0: the music's stream ended (`ebx` and `ecx` handed on); esp the frame base.
Result musicStreamEndOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 ebx, reg32 ecx);
// The game's 0x410b20: every bank removed from SND, the first's copy freed; esp the frame base.
Result banksRemoveOf(App* app, CPU& cpu, reg32 esp);

}  // namespace port

namespace guest
{

// `in` in eax, edx, ebx and ecx; eax back.
template <>
struct ByAddress<port::soundExitOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::Args& in)
    {
        return watcomEax(app, cpu, Address, {in.eax, in.edx, in.ebx, in.ecx});
    }
};

// `eax` in eax, `ecx` in ecx (edx and ebx left as they are); eax back.
template <>
struct ByAddress<port::musicStreamMade>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 ecx)
    {
        return watcomEax(app, cpu, Address, {eax, cpu.edx, cpu.ebx, ecx});
    }
};

// `eax` in eax, `ecx` in ecx (edx and ebx left as they are); eax back.
template <>
struct ByAddress<port::musicStreamOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 ecx)
    {
        return watcomEax(app, cpu, Address, {eax, cpu.edx, cpu.ebx, ecx});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::musicMenuSixOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// The tune in eax, the volume in edx, `own` in ebx; eax back.
template <>
struct ByAddress<port::musicStarted>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t tune,
                             std::int32_t volume, std::int32_t own)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(tune), x86::reg32(volume), x86::reg32(own)});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::musicStopped>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// `eax` in eax, `ebx` in ebx, `ecx` in ecx (edx left as it is); eax back.
template <>
struct ByAddress<port::musicStreamEndOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 ebx,
                             x86::reg32 ecx)
    {
        return watcomEax(app, cpu, Address, {eax, cpu.edx, ebx, ecx});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::banksRemoveOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
