#ifndef NFS3HP_NATIVE_GUEST_H
#define NFS3HP_NATIVE_GUEST_H

#include <lib/winapp.h>
#include <initializer_list>
#include <optional>
#include <type_traits>

/* What the game's own functions that are native (native_game.cpp,
 * native_watcom.cpp) need of the guest: its memory, and its code called as
 * the game's code calls it. */
namespace nfs3hp
{
namespace guest
{

inline x86::reg32 at(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg32>(address);
}

inline void put(win32::WinApplication* app, x86::reg32 address, x86::reg32 value)
{
    app->getMemory<x86::reg32>(address) = value;
}

inline x86::reg8 byte(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg8>(address);
}

inline void putByte(win32::WinApplication* app, x86::reg32 address, x86::reg8 value)
{
    app->getMemory<x86::reg8>(address) = value;
}

typedef void (*Native)(win32::WinApplication*, x86::CPU&);

/* A call as the game's code makes one: `arguments` pushed below esp, the
 * return address's slot below them, and esp back where it was after it --
 * where a stdcall callee's ret leaves it, and a cdecl one's caller puts it.
 * False when the game stopped in it, with everything left as it is. */
template <typename Target>
bool through(win32::WinApplication* app, x86::CPU& cpu, Target target, std::initializer_list<x86::reg32> arguments)
{
    const x86::reg32 esp = cpu.esp;
    x86::reg32 slot = esp - 4 * x86::reg32(arguments.size());
    cpu.esp = slot - 4;
    for (const x86::reg32 argument : arguments)
    {
        put(app, slot, argument);
        slot += 4;
    }
    if constexpr (std::is_same_v<Target, Native>)
        target(app, cpu);
    else
        app->dynamic_call(target, cpu);
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    return true;
}

inline bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                 std::initializer_list<x86::reg32> arguments = {})
{
    return through(app, cpu, target, arguments);
}

inline bool call(win32::WinApplication* app, x86::CPU& cpu, Native target,
                 std::initializer_list<x86::reg32> arguments = {})
{
    return through(app, cpu, target, arguments);
}

/* What a caller's frame keeps across a call by address, as Watcom's code does: every register but
 * eax, and esp. */
class KeptRegisters
{
public:
    explicit KeptRegisters(x86::CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp),
          esp_(cpu.esp)
    {
    }
    ~KeptRegisters()
    {
        cpu_.ebx = ebx_;
        cpu_.ecx = ecx_;
        cpu_.edx = edx_;
        cpu_.esi = esi_;
        cpu_.edi = edi_;
        cpu_.ebp = ebp_;
        cpu_.esp = esp_;
    }
    KeptRegisters(const KeptRegisters&) = delete;
    KeptRegisters& operator=(const KeptRegisters&) = delete;

private:
    x86::CPU& cpu_;
    x86::reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_, esp_;
};

/* A Watcom function of the game's by address as its code calls it: `registers` into eax, edx, ebx
 * and ecx in turn, every other register kept.  watcom: false when the game stopped in it;
 * watcomEax: its eax back; watcomFloat: what it leaves on the x87's stack, taken off. */
inline bool watcom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                   std::initializer_list<x86::reg32> registers)
{
    const KeptRegisters kept(cpu);
    x86::reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : registers)
        *into[i++] = value;
    return call(app, cpu, address);
}

inline std::optional<x86::reg32> watcomEax(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                                           std::initializer_list<x86::reg32> registers)
{
    if (!watcom(app, cpu, address, registers))
        return std::nullopt;
    return cpu.eax;
}

inline std::optional<x86::Float> watcomFloat(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                                             std::initializer_list<x86::reg32> registers)
{
    if (!watcom(app, cpu, address, registers))
        return std::nullopt;
    return cpu.fpu.take();
}

/* Another file's typed function called as C++ (that file's header, native_<file>.h); `Address` is
 * the game's function it is.  In the game a plain call.  In tools/game_checks (NFS_GAME_CHECKS) a
 * plain call too where the check runs that function's own code for the game's on the other side;
 * where it stands in for it there, the call goes by address as the game's code makes it -- through
 * the callee's ByAddress<Core>, in its header -- for the stand-in to take, as the game's code's
 * call does.  The address at the call is what the check reads to tell which (generate.py,
 * named_addresses). */
template <auto Core>
struct ByAddress;

#ifdef NFS_GAME_CHECKS
bool standsIn(x86::reg32 address);  // tools/game_checks/game_checks.cpp's
#endif

template <x86::reg32 Address, auto Core, typename... Args>
inline auto direct(win32::WinApplication* app, x86::CPU& cpu, Args... args)
{
#ifdef NFS_FRONTEND_CHECKS
    /* tools/frontend_checks: always by address, decided here, as every call of a function it
     * tests goes to a stand-in or through the dispatch -- and the core itself is never named, so
     * the menu files need not link one another there. */
#ifdef NFS_FRONTEND_STRICT
    /* Its strict build (FRONTEND_STRICT=1): the registers as the game's direct call leaves them --
     * every one but eax kept, eax spoilt -- so code that still counts on what a callee left in a
     * register, or on eax after a direct call, differs from the game's code. */
    if constexpr (std::is_void_v<decltype(ByAddress<Core>::template call<Address>(app, cpu, args...))>)
    {
        {
            const KeptRegisters kept(cpu);
            ByAddress<Core>::template call<Address>(app, cpu, args...);
        }
        cpu.eax = 0x5b0115edu;
    }
    else
    {
        auto result = [&] {
            const KeptRegisters kept(cpu);
            return ByAddress<Core>::template call<Address>(app, cpu, args...);
        }();
        cpu.eax = 0x5b0115edu;
        return result;
    }
#else
    return ByAddress<Core>::template call<Address>(app, cpu, args...);
#endif
#else
#ifdef NFS_GAME_CHECKS
    if (standsIn(Address))
        return ByAddress<Core>::template call<Address>(app, cpu, args...);
#endif
    // Every register but eax kept across it, as the call by address keeps them: what the caller
    // holds in them is the same in the game as in the check.  And the return address's slot under
    // esp while it runs, as the call's: a callee's frames on the guest's stack where the game's are.
    const KeptRegisters kept(cpu);
    cpu.esp -= 4;
    if constexpr (std::is_invocable_v<decltype(Core), win32::WinApplication*, x86::CPU&, Args...>)
        return Core(app, cpu, args...);
    else
        return Core(app, args...);
#endif
}

}  // namespace guest
}  // namespace nfs3hp

#endif
