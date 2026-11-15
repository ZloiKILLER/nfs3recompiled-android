#ifndef NFS3HP_THRASH_CALLS_H
#define NFS3HP_THRASH_CALLS_H

#include "native_guest.h"
#include "port_thrash.h"
#include "thrash.h"

#include <optional>
#include <type_traits>

/* The game's calls of its renderer -- the THRASH exports, through its
 * pointers to them (thrash.h, kThrash*) -- as C++ calls of the port's driver
 * (port_thrash.h): plan step 4's, for THRASH.
 *
 *     guest::driver<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, a, b, c)
 *
 * The words are what the game's code pushes, in its order.  In the game, when
 * the pointer leads to voodoo2a's export -- the adapter the port registers
 * under its name (disassembly/voodoo2a.cpp), which GetProcAddress gave
 * THRASH_opendll -- the typed function is called with them, the registers
 * but eax kept as the export keeps them; a pointer to anything else (another
 * driver) is called as the game's code calls it.  In tools/game_checks
 * (NFS_GAME_CHECKS) and tools/frontend_checks (NFS_FRONTEND_CHECKS) the call
 * always goes through the pointer, stdcall, the words pushed below esp, for
 * the check's stand-in; neither builds port_thrash.cpp.  What the export
 * leaves in eax, or nullopt when the game stopped in it. */
namespace nfs3hp
{

/* voodoo2a's exports as the game calls them (port_thrash.cpp). */
void THRASH_about(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_clearwindow(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_clip(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawline(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawlinemesh(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawlinestrip(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawpoint(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawpointmesh(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawpointstrip(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawquad(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawquadmesh(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawtri(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawtrifan(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawtrimesh(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_drawtristrip(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_flushwindow(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_idle(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_init(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_is(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_lockwindow(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_pageflip(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_readrect(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_restore(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_selectdisplay(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_setstate(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_settexture(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_setvideomode(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_sync(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_talloc(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_treset(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_tupdate(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_unlockwindow(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_window(win32::WinApplication* app, x86::CPU& cpu);
void THRASH_writerect(win32::WinApplication* app, x86::CPU& cpu);

namespace guest
{

#if defined(NFS_GAME_CHECKS) || defined(NFS_FRONTEND_CHECKS)
#define NFS3HP_THRASH_THROUGH_SLOTS 1
#endif

/* Each pointer's export: its typed function and, in the game, its adapter. */
template <x86::reg32 Slot>
struct DriverExport;

#ifdef NFS3HP_THRASH_THROUGH_SLOTS
#define NFS3HP_DRIVER_EXPORT(slot, typed, adapter)                       \
    template <>                                                          \
    struct DriverExport<thrash::slot>                                    \
    {                                                                    \
        static constexpr auto function = &port::driver::typed;           \
    }
#else
#define NFS3HP_DRIVER_EXPORT(slot, typed, adapter)                       \
    template <>                                                          \
    struct DriverExport<thrash::slot>                                    \
    {                                                                    \
        static constexpr auto function = &port::driver::typed;           \
        static constexpr Native registered = &nfs3hp::adapter;           \
    }
#endif

NFS3HP_DRIVER_EXPORT(kThrashDrawPointMesh, drawPointMesh, THRASH_drawpointmesh);
NFS3HP_DRIVER_EXPORT(kThrashDrawPointStrip, drawPointStrip, THRASH_drawpointstrip);
NFS3HP_DRIVER_EXPORT(kThrashDrawLineStrip, drawLineStrip, THRASH_drawlinestrip);
NFS3HP_DRIVER_EXPORT(kThrashDrawLineMesh, drawLineMesh, THRASH_drawlinemesh);
NFS3HP_DRIVER_EXPORT(kThrashDrawPoint, drawPoint, THRASH_drawpoint);
NFS3HP_DRIVER_EXPORT(kThrashWriteRect, writeRect, THRASH_writerect);
NFS3HP_DRIVER_EXPORT(kThrashTReset, tReset, THRASH_treset);
NFS3HP_DRIVER_EXPORT(kThrashDrawTriStrip, drawTriStrip, THRASH_drawtristrip);
NFS3HP_DRIVER_EXPORT(kThrashReadRect, readRect, THRASH_readrect);
NFS3HP_DRIVER_EXPORT(kThrashDrawTriMesh, drawTriMesh, THRASH_drawtrimesh);
NFS3HP_DRIVER_EXPORT(kThrashDrawLine, drawLine, THRASH_drawline);
NFS3HP_DRIVER_EXPORT(kThrashClip, clip, THRASH_clip);
NFS3HP_DRIVER_EXPORT(kThrashDrawQuadMesh, drawQuadMesh, THRASH_drawquadmesh);
NFS3HP_DRIVER_EXPORT(kThrashSetTexture, setTexture, THRASH_settexture);
NFS3HP_DRIVER_EXPORT(kThrashWindow, window, THRASH_window);
NFS3HP_DRIVER_EXPORT(kThrashIdle, idle, THRASH_idle);
NFS3HP_DRIVER_EXPORT(kThrashPageFlip, pageFlip, THRASH_pageflip);
NFS3HP_DRIVER_EXPORT(kThrashSelectDisplay, selectDisplay, THRASH_selectdisplay);
NFS3HP_DRIVER_EXPORT(kThrashSync, sync, THRASH_sync);
NFS3HP_DRIVER_EXPORT(kThrashDrawQuad, drawQuad, THRASH_drawquad);
NFS3HP_DRIVER_EXPORT(kThrashFlushWindow, flushWindow, THRASH_flushwindow);
NFS3HP_DRIVER_EXPORT(kThrashLockWindow, lockWindow, THRASH_lockwindow);
NFS3HP_DRIVER_EXPORT(kThrashSetVideoMode, setVideoMode, THRASH_setvideomode);
NFS3HP_DRIVER_EXPORT(kThrashUnlockWindow, unlockWindow, THRASH_unlockwindow);
NFS3HP_DRIVER_EXPORT(kThrashDrawTriFan, drawTriFan, THRASH_drawtrifan);
NFS3HP_DRIVER_EXPORT(kThrashTUpdate, tUpdate, THRASH_tupdate);
NFS3HP_DRIVER_EXPORT(kThrashTAlloc, tAlloc, THRASH_talloc);
NFS3HP_DRIVER_EXPORT(kThrashSetState, setState, THRASH_setstate);
NFS3HP_DRIVER_EXPORT(kThrashClearWindow, clearWindow, THRASH_clearwindow);
NFS3HP_DRIVER_EXPORT(kThrashDrawTri, drawTri, THRASH_drawtri);
NFS3HP_DRIVER_EXPORT(kThrashInit, init, THRASH_init);
NFS3HP_DRIVER_EXPORT(kThrashRestore, restore, THRASH_restore);
NFS3HP_DRIVER_EXPORT(kThrashAbout, about, THRASH_about);
NFS3HP_DRIVER_EXPORT(kThrashIs, is, THRASH_is);

#undef NFS3HP_DRIVER_EXPORT

template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> driver(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
    static_assert(Fn == DriverExport<Slot>::function, "the export the pointer is to");
    static_assert(4 * sizeof...(W) == thrash::exportAt(Slot).bytes, "every word the export takes");
#ifndef NFS3HP_THRASH_THROUGH_SLOTS
    if (app->methodAt(at(app, Slot)) == DriverExport<Slot>::registered)
    {
        const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
        const x86::reg32 esp = cpu.esp;
        const auto result = Fn(app, cpu, x86::reg32(words)...);
        x86::reg32 eax;
        if constexpr (std::is_same_v<std::decay_t<decltype(result)>, std::optional<x86::reg32>>)
        {
            if (!result)
                return std::nullopt;
            eax = *result;
        }
        else
        {
            eax = result;
        }
        if (cpu.terminate)
            return std::nullopt;
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        return eax;
    }
#endif
    if (!call(app, cpu, at(app, Slot), {x86::reg32(words)...}))
        return std::nullopt;
    return cpu.eax;
}

/* The same from a rewrite's frame (port.h's callGame): esp first set to
 * `esp`, where the game's code has it at the call, and left there. */
template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> driverFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, W... words)
{
    cpu.esp = esp;
    return driver<Slot, Fn>(app, cpu, words...);
}

}  // namespace guest
}  // namespace nfs3hp

#endif
