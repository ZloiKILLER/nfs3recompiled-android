#ifndef NFS3HP_NATIVE_CONTROLS_QUEUE_H
#define NFS3HP_NATIVE_CONTROLS_QUEUE_H

#include "native_guest.h"

/* native_controls_queue.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace controls
{

/* An answer, and what the code leaves in edx with it -- no caller reads it. */
struct Answered
{
    x86::reg32 answer;
    x86::reg32 edxLeft;
};

// The game's 0x48f6e0: choice `choice` of `count` stepped by the first player's buttons.
x86::reg8 stepChoice(win32::WinApplication* app, x86::reg8 choice, x86::reg32 count);
// The game's 0x48f710: choice `choice` of three stepped by the first player's buttons.
x86::reg8 stepChoiceOfThree(win32::WinApplication* app, x86::reg8 choice);
// The game's 0x4c60c0: this machine's four bytes at `actions` in a race over the network, queued and
// sent; eax as the code leaves it.
std::optional<x86::reg32> sendInput(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 actions);
// The game's 0x4c6530: player's four bytes at `actions` queued for its next frame: 1 if they were,
// else 0.
Answered queuePlayerInput(win32::WinApplication* app, x86::reg32 player, x86::reg32 actions);

}  // namespace controls

namespace guest
{

// The choice in al, the count in edx; the choice back in al.
template <>
struct ByAddress<controls::stepChoice>
{
    template <x86::reg32 Address>
    static x86::reg8 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg8 choice, x86::reg32 count)
    {
        return x86::reg8(watcomEax(app, cpu, Address, {choice, count}).value_or(0));
    }
};

// The choice in al; the choice back in al.
template <>
struct ByAddress<controls::stepChoiceOfThree>
{
    template <x86::reg32 Address>
    static x86::reg8 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg8 choice)
    {
        return x86::reg8(watcomEax(app, cpu, Address, {choice}).value_or(0));
    }
};

// The four bytes' address in eax; eax back.
template <>
struct ByAddress<controls::sendInput>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 actions)
    {
        return watcomEax(app, cpu, Address, {actions});
    }
};

// The player in eax, the four bytes' address in edx; eax and edx back.
template <>
struct ByAddress<controls::queuePlayerInput>
{
    template <x86::reg32 Address>
    static controls::Answered call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 actions)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = player;
        cpu.edx = actions;
        guest::call(app, cpu, Address);
        return controls::Answered{cpu.eax, cpu.edx};
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
