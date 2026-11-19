#ifndef NFS3HP_NATIVE_THRASH_H
#define NFS3HP_NATIVE_THRASH_H

#include <game_types.h>

namespace nfs
{
class App;
}

/* What the game's natives share around the THRASH driver (port_thrash.cpp). */
namespace nfs3hp
{

/* voodoo2a's malloc and free (native_watcom_heap.cpp): Watcom's near heap on
 * voodoo2a's variables, growing by VirtualAlloc through its imports -- what
 * the driver's texture and lock records come from.  After a call,
 * ts.terminate says whether the game stopped in one. */
nfs::u32 thrashMalloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 amount);
void thrashFree(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pointer);

}

#endif
