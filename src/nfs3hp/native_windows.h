#ifndef NFS3HP_NATIVE_WINDOWS_H
#define NFS3HP_NATIVE_WINDOWS_H

#include "native_guest.h"

#include <winapi/wrapper.h>

#include <cstring>
#include <optional>
#include <type_traits>

/* The game's calls of Windows -- its imports -- as C++ calls of the port's Windows
 * (src/lib/winapi): plan step 6's first part.
 *
 *     guest::windows<kCreateFileA, &win32::kernel32::CreateFileA>(app, cpu, name, access, ...)
 *
 * The words are what the game's code pushes, in its order.  In the game each becomes the
 * function's parameter as win32::Wrapper makes it of the same word on the stack (an address a
 * pointer into the game's memory, 0 a null one), and the function is called; what it gives back is
 * eax.  In tools/game_checks (NFS_GAME_CHECKS) the call goes through the import's slot as the
 * game's code makes it (stdcall: the words pushed, the callee takes them off), for the check's
 * stand-in.  The slot's address at the call is what the check reads (generate.py,
 * named_addresses).  Either way nullopt when the game stopped in it (ExitProcess, a stand-in's
 * end). */
namespace nfs3hp
{
namespace guest
{

template <typename T>
struct WindowsWord
{
    static T of(win32::WinApplication*, x86::reg32 word) { return static_cast<T>(word); }
};

template <typename T>
struct WindowsWord<T*>
{
    static T* of(win32::WinApplication* app, x86::reg32 word)
    {
        return word == 0 ? nullptr : &app->getMemory<T>(word);
    }
};

template <>
struct WindowsWord<float>
{
    static float of(win32::WinApplication*, x86::reg32 word)
    {
        float value;
        std::memcpy(&value, &word, sizeof value);
        return value;
    }
};

template <typename T>
struct WindowsWord<win32::Packed<T>>
{
    static win32::Packed<T> of(win32::WinApplication*, x86::reg32 word) { return win32::Packed<T>(word); }
};

template <typename F>
struct WindowsFunction;

template <typename R, typename... T>
struct WindowsFunction<R (*)(win32::WinApplication*, x86::CPU&, T...)>
{
    template <R (*Fn)(win32::WinApplication*, x86::CPU&, T...), typename... W>
    static void call(win32::WinApplication* app, x86::CPU& cpu, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the Windows function takes");
        if constexpr (std::is_void_v<R>)
            Fn(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...);
        else
            cpu.eax = x86::reg32(Fn(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> windows(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
#ifdef NFS_GAME_CHECKS
    if (!call(app, cpu, at(app, Slot), {x86::reg32(words)...}))
        return std::nullopt;
#else
    WindowsFunction<decltype(Fn)>::template call<Fn>(app, cpu, words...);
    if (cpu.terminate)
        return std::nullopt;
#endif
    return cpu.eax;
}

/* A Windows function the game's code calls through its import thunk (a jmp to the slot):
 * guest::windows, but in the check by the thunk's address, which the checks stand in for. */
template <x86::reg32 Thunk, auto Fn, typename... W>
inline std::optional<x86::reg32> thunked(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
#ifdef NFS_GAME_CHECKS
    if (!call(app, cpu, Thunk, {x86::reg32(words)...}))
        return std::nullopt;
#else
    WindowsFunction<decltype(Fn)>::template call<Fn>(app, cpu, words...);
    if (cpu.terminate)
        return std::nullopt;
#endif
    return cpu.eax;
}

template <typename F>
struct WindowsMethod;

template <typename R, typename Owner, typename... T>
struct WindowsMethod<R (Owner::*)(win32::WinApplication*, x86::CPU&, T...)>
{
    template <R (Owner::*Fn)(win32::WinApplication*, x86::CPU&, T...), typename... W>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the method takes, after the object");
        Owner* const owner = &app->getMemory<Owner>(object);
        if constexpr (std::is_void_v<R>)
            (owner->*Fn)(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...);
        else
            cpu.eax = x86::reg32((owner->*Fn)(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

/* A method of a COM object of the port's Windows (DirectDraw's, src/lib/winapi/ddraw), the
 * object `object` in the game's memory and the method at `Offset` in its table:
 *
 *     guest::method<0x18, &win32::IDirectDraw2::CreateSurface>(app, cpu, object, words...)
 *
 * In the game, where the object's table has that very method there (what dynamic_call would
 * call is Method's Wrapper), the method is called as C++ on the object, as Wrapper calls it;
 * anywhere else -- another object's table, the check -- the call goes through the table as the
 * game's code makes it (stdcall: the object, then the words). */
template <x86::reg32 Offset, auto Method, typename... W>
inline std::optional<x86::reg32> method(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object, W... words)
{
    const x86::reg32 target = at(app, at(app, object) + Offset);
#ifndef NFS_GAME_CHECKS
    if (object != 0 && app->methodAt(target) == &win32::Wrapper<decltype(Method), Method>::stdcall)
    {
        WindowsMethod<decltype(Method)>::template call<Method>(app, cpu, object, words...);
        if (cpu.terminate)
            return std::nullopt;
        return cpu.eax;
    }
#endif
    if (!call(app, cpu, target, {object, x86::reg32(words)...}))
        return std::nullopt;
    return cpu.eax;
}

}  // namespace guest
}  // namespace nfs3hp

#endif
