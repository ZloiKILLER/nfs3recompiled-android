#ifndef NFS3HP_NATIVE_GUEST_H
#define NFS3HP_NATIVE_GUEST_H

#include <lib/app.h>
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

inline nfs::u32 at(nfs::App* app, nfs::u32 address)
{
    return app->getMemory<nfs::u32>(address);
}

inline void put(nfs::App* app, nfs::u32 address, nfs::u32 value)
{
    app->getMemory<nfs::u32>(address) = value;
}

inline nfs::u8 byte(nfs::App* app, nfs::u32 address)
{
    return app->getMemory<nfs::u8>(address);
}

inline void putByte(nfs::App* app, nfs::u32 address, nfs::u8 value)
{
    app->getMemory<nfs::u8>(address) = value;
}

/* The game's memory at `address` as the port's own functions (os_*.h) take a buffer or a string
 * the game hands them: null for 0. */
inline char* memory(nfs::App* app, nfs::u32 address)
{
    return address == 0 ? nullptr : &app->getMemory<char>(address);
}

inline const char* text(nfs::App* app, nfs::u32 address)
{
    return memory(app, address);
}

/* `text` and its ending to `address` in the game's memory. */
inline void putText(nfs::App* app, nfs::u32 address, const char* text)
{
    for (nfs::u32 i = 0;; ++i)
    {
        app->getMemory<char>(address + i) = text[i];
        if (text[i] == 0)
            return;
    }
}

/* The words a variadic function's caller pushes after the arguments it names
 * (sprintf's after the buffer and the format), as one value. */
struct Words
{
    static constexpr unsigned kMost = 16;
    nfs::u32 word[kMost] = {};
    unsigned count = 0;

    Words(std::initializer_list<nfs::u32> words)
    {
        for (const nfs::u32 w : words)
            if (count < kMost)
                word[count++] = w;
    }
};

/* ebp as the game's code that a rewrite stands for keeps it -- its frame pointer -- for what it
 * calls that reads ebp unset (port::HeldEbp sets it): handed on by the calls below that are not
 * given an ebp of their own. */
inline nfs::u32& heldEbp()
{
    static thread_local nfs::u32 held = 0;
    return held;
}

/* The registers between the calls of the code that still works the game's way with them (native_render.cpp,
 * native_graphics.cpp): one image a guest thread, shared by those files.  Defined in native_render.cpp. */
nfs::Args& registerImage();

/* A call through a pointer (a table's, a callback, a handler the game's data holds, an import): `args`
 * its register arguments by Watcom's convention, and what the callee leaves in them after it; `words`
 * and then `more` pushed below esp, the return address's slot under them, esp back after it (where a
 * stdcall callee's ret leaves it, and a cdecl one's caller puts it).  Its eax, nullopt when the game
 * stopped. */
inline std::optional<nfs::u32> pointerCall(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target,
                                             nfs::Args& args, std::initializer_list<nfs::u32> words = {},
                                             const Words& more = Words({}))
{
    const nfs::u32 esp = ts.sp;
    nfs::u32 slot = esp - 4 * nfs::u32(words.size() + more.count);
    ts.sp = slot - 4;
    for (const nfs::u32 word : words)
    {
        put(app, slot, word);
        slot += 4;
    }
    for (unsigned i = 0; i < more.count; ++i)
    {
        put(app, slot, more.word[i]);
        slot += 4;
    }
    const nfs::u32 eax = app->callAddress(target, ts, args);
    if (ts.terminate)
        return std::nullopt;
    ts.sp = esp;
    return eax;
}

/* pointerCall with only the register arguments given (eax, edx, ebx, ecx in turn). */
inline std::optional<nfs::u32> pointerCall(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target,
                                             std::initializer_list<nfs::u32> registers,
                                             std::initializer_list<nfs::u32> words = {},
                                             const Words& more = Words({}))
{
    nfs::Args args;
    args.ebp = heldEbp();
    nfs::u32* const into[] = {&args.eax, &args.edx, &args.ebx, &args.ecx};
    unsigned i = 0;
    for (const nfs::u32 value : registers)
        if (i < 4)
            *into[i++] = value;
    return pointerCall(app, ts, target, args, words, more);
}

/* A call with its arguments on the stack only (stdcall, cdecl): `arguments` and then `more` pushed.
 * False when the game stopped in it. */
inline bool call(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target,
                 std::initializer_list<nfs::u32> arguments = {}, const Words& more = Words({}))
{
    nfs::Args args;
    args.ebp = heldEbp();
    return pointerCall(app, ts, target, args, arguments, more).has_value();
}

/* esp kept across a call, as the caller's frame keeps it, and back on the way out. */
class EspKept
{
public:
    explicit EspKept(nfs::ThreadState& ts) : ts_(ts), esp_(ts.sp) {}
    ~EspKept() { ts_.sp = esp_; }
    EspKept(const EspKept&) = delete;
    EspKept& operator=(const EspKept&) = delete;

private:
    nfs::ThreadState& ts_;
    nfs::u32 esp_;
};

/* A Watcom function of the game's by address as its code calls it: `registers` into eax, edx, ebx
 * and ecx in turn.  watcom: false when the game stopped in it; watcomEax: its eax back;
 * watcomFloat: the float it gives back. */
inline bool watcom(nfs::App* app, nfs::ThreadState& ts, nfs::u32 address,
                   std::initializer_list<nfs::u32> registers)
{
    return pointerCall(app, ts, address, registers).has_value();
}

inline std::optional<nfs::u32> watcomEax(nfs::App* app, nfs::ThreadState& ts, nfs::u32 address,
                                           std::initializer_list<nfs::u32> registers)
{
    return pointerCall(app, ts, address, registers);
}

inline std::optional<nfs::Float> watcomFloat(nfs::App* app, nfs::ThreadState& ts, nfs::u32 address,
                                             std::initializer_list<nfs::u32> registers)
{
    nfs::Args args;
    args.ebp = heldEbp();
    nfs::u32* const into[] = {&args.eax, &args.edx, &args.ebx, &args.ecx};
    unsigned i = 0;
    for (const nfs::u32 value : registers)
        if (i < 4)
            *into[i++] = value;
    if (!pointerCall(app, ts, address, args))
        return std::nullopt;
    return nfs::Float(args.floatResult);
}

/* Another file's typed function called as C++; `Address` is the game's function it is. */
template <nfs::u32 Address, auto Core, typename... Args>
inline auto direct(nfs::App* app, nfs::ThreadState& ts, Args... args)
{
    // The return address's slot under esp while it runs, as the call's: a callee's frames on the
    // guest's stack where the game's are.
    const EspKept kept(ts);
    ts.sp -= 4;
    if constexpr (std::is_invocable_v<decltype(Core), nfs::App*, nfs::ThreadState&, Args...>)
        return Core(app, ts, args...);
    else
        return Core(app, args...);
}

}  // namespace guest
}  // namespace nfs3hp

#endif
