#ifndef NFS3HP_PORT_SOUND_SPEECH_H
#define NFS3HP_PORT_SOUND_SPEECH_H

#include "native_guest.h"
#include "port.h"

/* port_sound_speech.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a
 * call.  A core's `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.
 * A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp
 * the adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4c9490: the description, if it is spoken, stopped (`eax` back with nothing to stop); esp the frame
// base.
Result descriptionStopped(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4c95f0: a description spoken -- track `track` of kind `kind` (0x32 the menus', 0x34 the cars', 0x33 a
// part of the one loaded) in language `language`; esp the frame base.
Result descriptionStarted(App* app, CPU& cpu, reg32 esp, reg32 track, reg32 kind, reg32 language);
// The game's 0x4c9860: the system tasks run, then the description's tick; esp the frame base.
Result descriptionServed(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c9890: the race voice bank `index` loaded; esp the frame base.
Result raceVoiceBankLoaded(App* app, CPU& cpu, reg32 esp, reg32 index);
// The game's 0x4c9990: effect `group` * 16 + `index` of the car or the scenery bank spoken: its handle; esp the frame
// base.
Result carLineSpoken(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 group);
// The game's 0x4c99d0: the spoken line faded out; esp the frame base.
Result spokenStopOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c99f0 (and 0x4c95e0): the spoken line's status; esp the frame base.
Result spokenStatusOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c9a00: the race voice banks removed and their header copies freed; esp the frame base.
Result raceVoiceBanksFreed(App* app, CPU& cpu, reg32 esp);

}  // namespace port

namespace guest
{

// `eax` in eax; eax back.
template <>
struct ByAddress<port::descriptionStopped>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// The track in eax, the kind in edx, the language in ebx; eax back.
template <>
struct ByAddress<port::descriptionStarted>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 track,
                             x86::reg32 kind, x86::reg32 language)
    {
        return watcomEax(app, cpu, Address, {track, kind, language});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::descriptionServed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The bank in eax; eax back.
template <>
struct ByAddress<port::raceVoiceBankLoaded>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 index)
    {
        return watcomEax(app, cpu, Address, {index});
    }
};

// The index in eax, the group in edx; the handle in eax.
template <>
struct ByAddress<port::carLineSpoken>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 index,
                             x86::reg32 group)
    {
        return watcomEax(app, cpu, Address, {index, group});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::spokenStopOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// Nothing handed over; the status in eax.
template <>
struct ByAddress<port::spokenStatusOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::raceVoiceBanksFreed>
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
