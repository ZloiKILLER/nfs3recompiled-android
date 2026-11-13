#ifndef NFS3HP_NATIVE_THRASH_H
#define NFS3HP_NATIVE_THRASH_H

#include <x86.h>

namespace win32
{
class WinApplication;
}

/* What the game's natives share around the THRASH driver (port_thrash.cpp). */
namespace nfs3hp
{

/* voodoo2a's malloc and free (native_watcom_heap.cpp): Watcom's near heap on
 * voodoo2a's variables, growing by VirtualAlloc through its imports -- what
 * the driver's texture and lock records come from.  After a call,
 * cpu.terminate says whether the game stopped in one. */
x86::reg32 thrashMalloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 amount);
void thrashFree(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pointer);

/* From native_vertices.cpp, for the natives of the other files that keep
 * checks of their own (native_loading.cpp, native_movie.cpp,
 * ): NFS_NATIVE_CHECK=1 and NFS_NATIVES=0 as read once.
 * Since phase 2 (2026-10-27) the vertex and drawing natives are whole, with
 * no generated code behind them and no check of their own in the game:
 * nativeOriginalRunning (a check of theirs running the generated code, when
 * every other native stepped aside) and nativeWholeChecking (one of theirs
 * checking a call over all of memory, when a nested check ran the native
 * alone) are false. */
bool nativeChecking();
bool nativesEnabled();
bool nativeOriginalRunning();
bool nativeWholeChecking();

}

#endif
