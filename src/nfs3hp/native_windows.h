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
 *     guest::windows<kCreateFileA, &host::createFile>(app, cpu, name, access, ...)
 *
 * The words are what the game's code pushes, in its order.  Each becomes the
 * function's parameter as win32::Wrapper makes it of the same word on the stack (an address a
 * pointer into the game's memory, 0 a null one), and the function is called; what it gives back is
 * eax.  nullopt when the game stopped in it (ExitProcess). */
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
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the Windows function takes");
        if constexpr (std::is_void_v<R>)
        {
            Fn(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...);
            return 0;
        }
        else
            return x86::reg32(Fn(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

/* The port's own host functions (src/nfs3hp/host_*.h, plan step 6) in the import's place: one
 * that needs neither the CPU (it never waits, so never lets the guest context go) nor the
 * application, or the application alone.  The words become its parameters the same way. */
template <typename R, typename... T>
struct WindowsFunction<R (*)(win32::WinApplication*, T...)>
{
    template <R (*Fn)(win32::WinApplication*, T...), typename... W>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the host function takes");
        if constexpr (std::is_void_v<R>)
        {
            Fn(app, WindowsWord<T>::of(app, x86::reg32(words))...);
            return 0;
        }
        else
            return x86::reg32(Fn(app, WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

template <typename R, typename... T>
struct WindowsFunction<R (*)(T...)>
{
    template <R (*Fn)(T...), typename... W>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the host function takes");
        if constexpr (std::is_void_v<R>)
        {
            Fn(WindowsWord<T>::of(app, x86::reg32(words))...);
            return 0;
        }
        else
            return x86::reg32(Fn(WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> windows(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
    const x86::reg32 eax = WindowsFunction<decltype(Fn)>::template call<Fn>(app, cpu, words...);
    if (cpu.terminate)
        return std::nullopt;
    return eax;
}

template <typename F>
struct ReturnsVoid;
template <typename R, typename... T>
struct ReturnsVoid<R (*)(T...)> : std::is_void<R>
{
};

/* guest::windows where the code goes on with eax after a function that gives nothing back: `eax`,
 * what it has there at the call. */
template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> windowsKeeping(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, W... words)
{
    const x86::reg32 r = WindowsFunction<decltype(Fn)>::template call<Fn>(app, cpu, words...);
    if (cpu.terminate)
        return std::nullopt;
    return ReturnsVoid<decltype(Fn)>::value ? eax : r;
}

/* A Windows function the game's code calls through its import thunk (a jmp to the slot):
 * guest::windows. */
template <x86::reg32 Thunk, auto Fn, typename... W>
inline std::optional<x86::reg32> thunked(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
    const x86::reg32 eax = WindowsFunction<decltype(Fn)>::template call<Fn>(app, cpu, words...);
    if (cpu.terminate)
        return std::nullopt;
    return eax;
}

/* A call of Windows the port answers with its own host code (src/nfs3hp/host_*.h, plan step 6),
 * in host terms rather than the import's words:
 *
 *     guest::host<kGetTickCount>(app, cpu, {}, [&] { return host::ticks(); })
 *
 * `body` runs and what it gives back, if anything, is eax.  hostThunk: the same for a call
 * through the import thunk's address.  nullopt when the game stopped in it. */
template <typename Body>
inline std::optional<x86::reg32> hostBody(x86::CPU& cpu, Body& body)
{
    x86::reg32 eax = 0;
    if constexpr (std::is_void_v<decltype(body())>)
        body();
    else
        eax = x86::reg32(body());
    if (cpu.terminate)
        return std::nullopt;
    return eax;
}

template <x86::reg32 Slot, typename Body>
inline std::optional<x86::reg32> host(win32::WinApplication* app, x86::CPU& cpu,
                                      std::initializer_list<x86::reg32> words, Body&& body)
{
    (void)app;
    (void)words;
    return hostBody(cpu, body);
}

template <x86::reg32 Thunk, typename Body>
inline std::optional<x86::reg32> hostThunk(win32::WinApplication* app, x86::CPU& cpu,
                                           std::initializer_list<x86::reg32> words, Body&& body)
{
    (void)app;
    (void)words;
    return hostBody(cpu, body);
}

template <typename F>
struct WindowsMethod;

template <typename R, typename Owner, typename... T>
struct WindowsMethod<R (Owner::*)(win32::WinApplication*, x86::CPU&, T...)>
{
    template <R (Owner::*Fn)(win32::WinApplication*, x86::CPU&, T...), typename... W>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object, W... words)
    {
        static_assert(sizeof...(T) == sizeof...(W), "every word the method takes, after the object");
        Owner* const owner = &app->getMemory<Owner>(object);
        if constexpr (std::is_void_v<R>)
        {
            (owner->*Fn)(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...);
            return 0;
        }
        else
            return x86::reg32((owner->*Fn)(app, cpu, WindowsWord<T>::of(app, x86::reg32(words))...));
    }
};

/* A method of a COM object of the port's Windows (DirectDraw's, src/lib/winapi/ddraw), the
 * object `object` in the game's memory and the method at `Offset` in its table:
 *
 *     guest::method<0x18, &win32::IDirectDraw2::CreateSurface>(app, cpu, object, words...)
 *
 * Where the object's table has that very method there (what a call through it
 * would call is Method's Wrapper), the method is called as C++ on the object, as Wrapper calls it;
 * anywhere else -- another object's table -- the call goes through the table as the
 * game's code makes it (stdcall: the object, then the words). */
template <x86::reg32 Offset, auto Method, typename... W>
inline std::optional<x86::reg32> method(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object, W... words)
{
    const x86::reg32 target = at(app, at(app, object) + Offset);
    if (object != 0 && app->thunkAt(target) == &win32::Wrapper<decltype(Method), Method>::stdcall)
    {
        const x86::reg32 eax = WindowsMethod<decltype(Method)>::template call<Method>(app, cpu, object, words...);
        if (cpu.terminate)
            return std::nullopt;
        return eax;
    }
    x86::Args args;
    return pointerCall(app, cpu, target, args, {object, x86::reg32(words)...});
}

}  // namespace guest
}  // namespace nfs3hp

#endif
