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

/* The words a variadic function's caller pushes after the arguments it names
 * (sprintf's after the buffer and the format), as one value. */
struct Words
{
    static constexpr unsigned kMost = 16;
    x86::reg32 word[kMost] = {};
    unsigned count = 0;

    Words(std::initializer_list<x86::reg32> words)
    {
        for (const x86::reg32 w : words)
            if (count < kMost)
                word[count++] = w;
    }
};

/* ebp as the game's code that a rewrite stands for keeps it -- its frame pointer -- for what it
 * calls that reads ebp unset (port::HeldEbp sets it): handed on by the calls below that are not
 * given an ebp of their own. */
inline x86::reg32& heldEbp()
{
    static thread_local x86::reg32 held = 0;
    return held;
}

/* The registers between the calls of the code that still works the game's way with them (native_render.cpp,
 * native_graphics.cpp): one image a guest thread, shared by those files.  Defined in native_render.cpp. */
x86::Args& registerImage();

/* A call through a pointer (a table's, a callback, a handler the game's data holds, an import): `args`
 * its register arguments by Watcom's convention, and what the callee leaves in them after it; `words`
 * and then `more` pushed below esp, the return address's slot under them, esp back after it (where a
 * stdcall callee's ret leaves it, and a cdecl one's caller puts it).  Its eax, nullopt when the game
 * stopped. */
inline std::optional<x86::reg32> pointerCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                                             x86::Args& args, std::initializer_list<x86::reg32> words = {},
                                             const Words& more = Words({}))
{
    const x86::reg32 esp = cpu.esp;
    x86::reg32 slot = esp - 4 * x86::reg32(words.size() + more.count);
    cpu.esp = slot - 4;
    for (const x86::reg32 word : words)
    {
        put(app, slot, word);
        slot += 4;
    }
    for (unsigned i = 0; i < more.count; ++i)
    {
        put(app, slot, more.word[i]);
        slot += 4;
    }
    const x86::reg32 eax = app->callAddress(target, cpu, args);
    if (cpu.terminate)
        return std::nullopt;
    cpu.esp = esp;
    return eax;
}

/* pointerCall with only the register arguments given (eax, edx, ebx, ecx in turn). */
inline std::optional<x86::reg32> pointerCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                                             std::initializer_list<x86::reg32> registers,
                                             std::initializer_list<x86::reg32> words = {},
                                             const Words& more = Words({}))
{
    x86::Args args;
    args.ebp = heldEbp();
    x86::reg32* const into[] = {&args.eax, &args.edx, &args.ebx, &args.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : registers)
        if (i < 4)
            *into[i++] = value;
    return pointerCall(app, cpu, target, args, words, more);
}

/* A call with its arguments on the stack only (stdcall, cdecl): `arguments` and then `more` pushed.
 * False when the game stopped in it. */
inline bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                 std::initializer_list<x86::reg32> arguments = {}, const Words& more = Words({}))
{
    x86::Args args;
    args.ebp = heldEbp();
    return pointerCall(app, cpu, target, args, arguments, more).has_value();
}

/* esp kept across a call, as the caller's frame keeps it, and back on the way out. */
class EspKept
{
public:
    explicit EspKept(x86::CPU& cpu) : cpu_(cpu), esp_(cpu.esp) {}
    ~EspKept() { cpu_.esp = esp_; }
    EspKept(const EspKept&) = delete;
    EspKept& operator=(const EspKept&) = delete;

private:
    x86::CPU& cpu_;
    x86::reg32 esp_;
};

/* A Watcom function of the game's by address as its code calls it: `registers` into eax, edx, ebx
 * and ecx in turn.  watcom: false when the game stopped in it; watcomEax: its eax back;
 * watcomFloat: the float it gives back. */
inline bool watcom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                   std::initializer_list<x86::reg32> registers)
{
    return pointerCall(app, cpu, address, registers).has_value();
}

inline std::optional<x86::reg32> watcomEax(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                                           std::initializer_list<x86::reg32> registers)
{
    return pointerCall(app, cpu, address, registers);
}

inline std::optional<x86::Float> watcomFloat(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                                             std::initializer_list<x86::reg32> registers)
{
    x86::Args args;
    args.ebp = heldEbp();
    x86::reg32* const into[] = {&args.eax, &args.edx, &args.ebx, &args.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : registers)
        if (i < 4)
            *into[i++] = value;
    if (!pointerCall(app, cpu, address, args))
        return std::nullopt;
    return x86::Float(args.floatResult);
}

/* Another file's typed function called as C++; `Address` is the game's function it is. */
template <x86::reg32 Address, auto Core, typename... Args>
inline auto direct(win32::WinApplication* app, x86::CPU& cpu, Args... args)
{
    // The return address's slot under esp while it runs, as the call's: a callee's frames on the
    // guest's stack where the game's are.
    const EspKept kept(cpu);
    cpu.esp -= 4;
    if constexpr (std::is_invocable_v<decltype(Core), win32::WinApplication*, x86::CPU&, Args...>)
        return Core(app, cpu, args...);
    else
        return Core(app, args...);
}

}  // namespace guest
}  // namespace nfs3hp

#endif
