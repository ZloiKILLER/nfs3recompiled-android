#ifndef NFS3HP_NATIVE_WATCOM_HEAP_H
#define NFS3HP_NATIVE_WATCOM_HEAP_H

#include <lib/app.h>

/* Watcom's near heap (native_watcom_heap.cpp), for each runtime that has one
 * in guest memory: the game's (native_watcom.cpp binds its functions) and
 * voodoo2a's (port_thrash.cpp).  Each heap is where its runtime keeps it --
 * these say where -- and the same heap afterwards as the runtime's code would
 * have left it. */
namespace nfs3hp
{

struct WatcomHeap
{
    nfs::u32 first;          // __nheapbeg: the first miniheap
    nfs::u32 rover;          // __MiniHeapRover: the one a search starts from
    nfs::u32 largestBefore;  // __LargestSizeB4MiniHeapRover
    nfs::u32 freeRover;      // __MiniHeapFreeRover: the one the last free found
    nfs::u32 clean;          // __nheap_clean, a byte: 0 once the heap changed
    nfs::u32 enabled;        // __heap_enabled: 0 and the heap does not grow
    nfs::u32 brk;            // _curbrk: -2 and it does not grow either
    nfs::u32 blockSize;      // _amblksiz: what it grows by at least
    nfs::u32 access;         // _AccessNHeap and _ReleaseNHeap, the heap's lock: the
    nfs::u32 release;        // pointers to them, or 0 where it takes none
};

extern const WatcomHeap kGameHeap;
extern const WatcomHeap kVoodoo2aHeap;

namespace watcomheap
{

/* What the runtime's heap functions do.  Those that call out -- the lock --
 * are false when the game stopped in a call, with
 * everything left as it is; what they give back is in `result`. */

// _nmalloc: a block of `amount` bytes, or 0.
bool allocate(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result);
// _nfree: a block given back; 0 and pointers in no miniheap are nothing.  What its code leaves in eax
// into `eax`, if asked: what the heap's lock's giving back leaves there, the block for none.
bool free(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 pointer,
          nfs::u32* eax = nullptr);
// __MemAllocator: a block of `amount` from the miniheap `heap`, or 0.
nfs::u32 allocateIn(nfs::App* app, nfs::u32 heap, nfs::u32 amount);
// __MemFree: a block given back to its miniheap.
void freeIn(nfs::App* app, nfs::u32 heap, nfs::u32 pointer);
// __LinkUpNewNHeap: a new miniheap into the list; its first block.
nfs::u32 link(nfs::App* app, const WatcomHeap& h, nfs::u32 heap);
// __AdjustAmount: `amount` made what a miniheap for it takes; false if none can.
bool adjust(nfs::App* app, const WatcomHeap& h, nfs::u32& amount);
// __CreateNewNHeap: a miniheap with room for `amount`; 1, or 0.
bool create(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result);
// __ExpandDGROUP: the empty miniheaps given back, then a new one; 1, or 0.
bool expand(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result);
// _nheapshrink: every empty miniheap given back to the system.
bool shrink(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h);
// __ReturnMemToSystem: a miniheap to VirtualFree, the rovers off it; 0, or -1.
bool giveBack(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 heap, nfs::u32& result);
// The same, and the miniheap off the list if it went; 0, or -1.
bool release(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 heap, nfs::u32& result);

}  // namespace watcomheap
}  // namespace nfs3hp

#endif
