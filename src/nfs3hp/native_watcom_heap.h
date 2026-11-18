#ifndef NFS3HP_NATIVE_WATCOM_HEAP_H
#define NFS3HP_NATIVE_WATCOM_HEAP_H

#include <lib/winapp.h>

/* Watcom's near heap (native_watcom_heap.cpp), for each runtime that has one
 * in guest memory: the game's (native_watcom.cpp binds its functions) and
 * voodoo2a's (port_thrash.cpp).  Each heap is where its runtime keeps it --
 * these say where -- and the same heap afterwards as the runtime's code would
 * have left it. */
namespace nfs3hp
{

struct WatcomHeap
{
    x86::reg32 first;          // __nheapbeg: the first miniheap
    x86::reg32 rover;          // __MiniHeapRover: the one a search starts from
    x86::reg32 largestBefore;  // __LargestSizeB4MiniHeapRover
    x86::reg32 freeRover;      // __MiniHeapFreeRover: the one the last free found
    x86::reg32 clean;          // __nheap_clean, a byte: 0 once the heap changed
    x86::reg32 enabled;        // __heap_enabled: 0 and the heap does not grow
    x86::reg32 brk;            // _curbrk: -2 and it does not grow either
    x86::reg32 blockSize;      // _amblksiz: what it grows by at least
    x86::reg32 virtualAlloc;   // the runtime's imports
    x86::reg32 virtualFree;
    x86::reg32 access;         // _AccessNHeap and _ReleaseNHeap, the heap's lock: the
    x86::reg32 release;        // pointers to them, or 0 where it takes none
};

extern const WatcomHeap kGameHeap;
extern const WatcomHeap kVoodoo2aHeap;

namespace watcomheap
{

/* What the runtime's heap functions do.  Those that call out -- the lock,
 * VirtualAlloc, VirtualFree -- are false when the game stopped in a call, with
 * everything left as it is; what they give back is in `result`. */

// _nmalloc: a block of `amount` bytes, or 0.
bool allocate(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result);
// _nfree: a block given back; 0 and pointers in no miniheap are nothing.  What its code leaves in eax
// into `eax`, if asked: what the heap's lock's giving back leaves there, the block for none.
bool free(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 pointer,
          x86::reg32* eax = nullptr);
// __MemAllocator: a block of `amount` from the miniheap `heap`, or 0.
x86::reg32 allocateIn(win32::WinApplication* app, x86::reg32 heap, x86::reg32 amount);
// __MemFree: a block given back to its miniheap.
void freeIn(win32::WinApplication* app, x86::reg32 heap, x86::reg32 pointer);
// __LinkUpNewNHeap: a new miniheap into the list; its first block.
x86::reg32 link(win32::WinApplication* app, const WatcomHeap& h, x86::reg32 heap);
// __AdjustAmount: `amount` made what a miniheap for it takes; false if none can.
bool adjust(win32::WinApplication* app, const WatcomHeap& h, x86::reg32& amount);
// __CreateNewNHeap: a miniheap with room for `amount`; 1, or 0.
bool create(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result);
// __ExpandDGROUP: the empty miniheaps given back, then a new one; 1, or 0.
bool expand(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result);
// _nheapshrink: every empty miniheap given back to the system.
bool shrink(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h);
// __ReturnMemToSystem: a miniheap to VirtualFree, the rovers off it; 0, or -1.
bool giveBack(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 heap, x86::reg32& result);
// The same, and the miniheap off the list if it went; 0, or -1.
bool release(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 heap, x86::reg32& result);

}  // namespace watcomheap
}  // namespace nfs3hp

#endif
