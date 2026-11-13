#ifndef NFS3HP_PORT_SND_STREAM_CALLS_H
#define NFS3HP_PORT_SND_STREAM_CALLS_H

#include "native_guest.h"
#include "port.h"

/* port_snd_stream.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress below, for the checks' stand-ins).  Not in port_snd_stream.h, which names the
 * game's addresses the stream files call (a header's numbers count as named by every file including it:
 * tools/game_checks/generate.py).  A Result is nullopt when the game stopped in a call.  A core's `esp` is the
 * frame base its adapter hands it: the game's esp on entry, at the return address.  A caller through
 * guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the adapter hands
 * it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{
namespace stream
{

// The game's 0x4e7470 (SNDstreamautovol): the stream's voice to volume `volume` over `ticks`; esp the frame base.
Result fadeStream(App* app, CPU& cpu, reg32 esp, int32_t ticks, int32_t volume);
// The game's 0x4e7520 (addsystemtask): `function` run every `period` ticks, the first `delay` ticks from now; the
// tick it runs first; esp the frame base.
Result scheduleTask(App* app, CPU& cpu, reg32 esp, reg32 function, int32_t period, reg32 delay);
// The game's 0x4e75e0: `function` no longer a task; its slot's offset, 0x100 for none.
reg32 unscheduleTask(App* app, reg32 function);
// The game's 0x4e7630: the tasks due run, each with `argument` (and the caller's ebp, as the code's tasks find it,
// when the tasks are not timed); what they gave back, or-ed; esp the frame base.
Result runDueTasks(App* app, CPU& cpu, reg32 esp, reg32 argument);

}  // namespace stream
}  // namespace port

namespace guest
{

// The ticks in eax, the volume in edx; eax back.
template <>
struct ByAddress<port::stream::fadeStream>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t ticks,
                             std::int32_t volume)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(ticks), x86::reg32(volume)});
    }
};

// The function in eax, the period in edx, the delay in ebx; the tick in eax.
template <>
struct ByAddress<port::stream::scheduleTask>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 function,
                             std::int32_t period, x86::reg32 delay)
    {
        return watcomEax(app, cpu, Address, {function, x86::reg32(period), delay});
    }
};

// The function in eax; the offset in eax.
template <>
struct ByAddress<port::stream::unscheduleTask>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 function)
    {
        watcom(app, cpu, Address, {function});
        return cpu.eax;
    }
};

// The argument in eax; eax back.
template <>
struct ByAddress<port::stream::runDueTasks>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 argument)
    {
        return watcomEax(app, cpu, Address, {argument});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
