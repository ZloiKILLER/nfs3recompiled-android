#include "native_watcom_heap.h"
#include "native_guest.h"
#include "native_thrash.h"
#include "native_windows.h"
#include "host_system.h"

/* Watcom's near heap, as the runtime's code keeps it, on its variables in
 * guest memory (WatcomHeap): the game's -- whose functions native_watcom.cpp
 * binds -- and voodoo2a's, whose malloc and free THRASH_talloc and
 * THRASH_lockwindow take their records from, so they lie where voodoo2a's code
 * would have put them.  The two runtimes are Watcom 11's and have the same
 * heap; the game's takes the heap's lock around it, voodoo2a's would take its
 * own, a bare ret, which is not called here.
 *
 * The heap is a list of "miniheaps", each a block of VirtualAlloc'd memory
 * kept in the order of their addresses:
 *
 *   +0x00  length, the header's 0x2c and the closing -1 included
 *   +0x04  the miniheap before, +0x08 the one after
 *   +0x0c  the rover: the free block a search starts from
 *   +0x10  the largest free block before the rover
 *   +0x14  the largest free block
 *   +0x18  blocks in use, +0x1c free blocks
 *   +0x20  the free list's head: a length of 0, the last free block, the first
 *   +0x2c  the first block: a length, its bit 0 set while in use; a free
 *          block's previous and next free blocks follow it.  The last dword
 *          of the miniheap is -1. */

namespace nfs3hp
{

namespace
{

/* The runtimes' VirtualAlloc and VirtualFree: the slots of the game's imports, and of voodoo2a's. */
constexpr x86::reg32 kGameVirtualAlloc = 0x534624, kGameVirtualFree = 0x534628;
constexpr x86::reg32 kVoodoo2aVirtualAlloc = 0xa9042c, kVoodoo2aVirtualFree = 0xa90430;

}  // namespace

const WatcomHeap kGameHeap = {
    0x566f4c, 0x566f50, 0x566f54, 0x9f3800, 0xa0b140, 0x56af68, 0x5677f8, 0x56af6c, kGameVirtualAlloc,
    kGameVirtualFree, 0x567780, 0x567788,
};

const WatcomHeap kVoodoo2aHeap = {
    0xa9267c, 0xa92680, 0xa92684, 0xa93850, 0xa93eb0, 0xa92830, 0xa9273c, 0xa92834, kVoodoo2aVirtualAlloc,
    kVoodoo2aVirtualFree, 0, 0,
};

namespace watcomheap
{

namespace
{

x86::reg32 word(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg32>(address);
}

using guest::put;

/* The heap's lock taken, or given back, through its pointer: false when the game stopped; what the
 * call leaves in eax into `eax`, if asked, which is as it was for no lock. */
bool lock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32* eax = nullptr)
{
    if (slot == 0)
        return true;
    const auto left = guest::pointerCall(app, cpu, word(app, slot), {});
    if (!left)
        return false;
    if (eax != nullptr)
        *eax = *left;
    return true;
}

/* The runtime's VirtualAlloc and VirtualFree, their words handed: the port's own, called as C++
 * (native_windows.h).  What it gave back; nullopt when the game stopped. */
template <typename... Words>
std::optional<x86::reg32> virtualAlloc(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, Words... words)
{
    return h.virtualAlloc == kGameVirtualAlloc
        ? guest::windows<kGameVirtualAlloc, &host::virtualAlloc>(app, cpu, words...)
        : guest::windows<kVoodoo2aVirtualAlloc, &host::virtualAlloc>(app, cpu, words...);
}

template <typename... Words>
std::optional<x86::reg32> virtualFree(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, Words... words)
{
    return h.virtualFree == kGameVirtualFree
        ? guest::windows<kGameVirtualFree, &host::virtualFree>(app, cpu, words...)
        : guest::windows<kVoodoo2aVirtualFree, &host::virtualFree>(app, cpu, words...);
}

}  // namespace

/* __MemAllocator (game 0x515300, voodoo2a sub_a88100). */
x86::reg32 allocateIn(win32::WinApplication* app, x86::reg32 heap, x86::reg32 amount)
{
    if (amount == 0)
        return 0;
    x86::reg32 size = amount + 0xb;
    if (size < amount)
        return 0;
    size &= ~x86::reg32(7);
    if (size < 0x10)
        size = 0x10;
    if (size > word(app, heap + 0x14))
        return 0;
    x86::reg32 largest = word(app, heap + 0x10);
    x86::reg32 block = word(app, heap + 0xc);
    if (size <= largest)
    {
        block = word(app, heap + 0x28);
        largest = 0;
    }
    const x86::reg32 head = heap + 0x20;
    x86::reg32 length;
    while (true)
    {
        length = word(app, block);
        if (size <= length)
            break;
        if (length > largest)
            largest = length;
        block = word(app, block + 8);
        if (block == head)
        {
            put(app, heap + 0x14, largest);
            return 0;
        }
    }
    put(app, heap + 0x10, largest);
    put(app, heap + 0x18, word(app, heap + 0x18) + 1);
    const x86::reg32 left = length - size;
    if (left >= 0x10)
    {
        const x86::reg32 rest = block + size;
        put(app, heap + 0xc, rest);
        put(app, rest, left);
        put(app, block, size);
        const x86::reg32 before = word(app, block + 4);
        put(app, rest + 4, before);
        const x86::reg32 after = word(app, block + 8);
        put(app, rest + 8, after);
        put(app, before + 8, rest);
        put(app, after + 4, rest);
    }
    else
    {
        put(app, heap + 0x1c, word(app, heap + 0x1c) - 1);
        const x86::reg32 before = word(app, block + 4);
        put(app, heap + 0xc, before);
        const x86::reg32 after = word(app, block + 8);
        put(app, before + 8, after);
        put(app, after + 4, before);
    }
    app->getMemory<x86::reg8>(block) |= 1;
    return block + 4;
}

/* __MemFree (game 0x5153b0, voodoo2a sub_a881b0): joined to the free blocks
 * beside it. */
void freeIn(win32::WinApplication* app, x86::reg32 heap, x86::reg32 pointer)
{
    if (pointer == 0)
        return;
    x86::reg32 block = pointer - 4;
    if (!(app->getMemory<x86::reg8>(block) & 1))
        return;
    x86::reg32 length = word(app, block) & ~x86::reg32(1);
    x86::reg32 next = block + length;
    x86::reg32 at;  // the free block `block` goes before
    if (!(app->getMemory<x86::reg8>(next) & 1))
    {
        // The one after is free: the two become one, which takes its place.
        length += word(app, next);
        put(app, block, length);
        if (next == word(app, heap + 0xc))
            put(app, heap + 0xc, block);
        const x86::reg32 before = word(app, next + 4);
        at = word(app, next + 8);
        put(app, before + 8, at);
        put(app, at + 4, before);
        put(app, heap + 0x1c, word(app, heap + 0x1c) - 1);
    }
    else
    {
        put(app, block, length);
        bool search = false;
        const x86::reg32 rover = word(app, heap + 0xc);
        if (block < rover)
        {
            at = rover;
            if (block <= word(app, rover + 4))
            {
                at = word(app, heap + 0x28);
                search = block >= at;
            }
        }
        else
        {
            at = word(app, rover + 8);
            if (block >= at)
            {
                at = heap + 0x20;
                search = block <= word(app, at + 4);
            }
        }
        if (search)
        {
            // A few blocks onwards first, for a free one; then the free list.
            const x86::reg32 free = word(app, heap + 0x1c);
            const x86::reg32 used = word(app, heap + 0x18);
            x86::reg32 tries = used / (free + 1);
            bool found = false;
            if (tries < free)
            {
                tries += tries;
                if (used - free <= free)
                    tries = 0xffffffff;
                x86::reg32 ahead = word(app, block) + block;
                while (true)
                {
                    const x86::reg32 header = word(app, ahead);
                    if (!(header & 1))
                    {
                        at = ahead;
                        found = true;
                        break;
                    }
                    if (header == 0xffffffff)
                        break;
                    ahead += header & ~x86::reg32(1);
                    if (--tries == 0)
                        break;
                }
            }
            if (!found)
            {
                at = word(app, heap + 0xc);
                if (block < at)
                    at = word(app, heap + 0x28);
                while (block >= at)
                    at = word(app, at + 8);
            }
        }
    }
    const x86::reg32 before = word(app, at + 4);
    length = word(app, block);
    if (word(app, before) + before == block)
    {
        length += word(app, before);
        put(app, before, length);
        if (block == word(app, heap + 0xc))
            put(app, heap + 0xc, before);
        block = before;
    }
    else
    {
        put(app, heap + 0x1c, word(app, heap + 0x1c) + 1);
        put(app, block + 8, at);
        put(app, block + 4, before);
        put(app, before + 8, block);
        put(app, at + 4, block);
    }
    put(app, heap + 0x18, word(app, heap + 0x18) - 1);
    if (block < word(app, heap + 0xc) && length > word(app, heap + 0x10))
        put(app, heap + 0x10, length);
    if (length > word(app, heap + 0x14))
        put(app, heap + 0x14, length);
}

/* __LinkUpNewNHeap (game 0x5154e0, voodoo2a sub_a882e0): a new miniheap of
 * [heap] bytes into the list, in the order of the addresses; its first block,
 * all of it, in use by no one and on no list yet. */
x86::reg32 link(win32::WinApplication* app, const WatcomHeap& h, x86::reg32 heap)
{
    x86::reg32 before = 0;
    x86::reg32 after = word(app, h.first);
    while (after != 0 && heap >= after)
    {
        before = after;
        after = word(app, after + 8);
    }
    put(app, heap + 4, before);
    put(app, heap + 8, after);
    if (before != 0)
        put(app, before + 8, heap);
    else
        put(app, h.first, heap);
    if (after != 0)
        put(app, after + 4, heap);
    const x86::reg32 head = heap + 0x20;
    const x86::reg32 first = heap + 0x2c;
    put(app, heap + 0x20, 0);
    put(app, heap + 0x10, 0);
    put(app, heap + 0x18, 0);
    put(app, heap + 0x1c, 0);
    put(app, heap + 0x24, head);
    const x86::reg32 length = word(app, heap) - 0x2c;
    put(app, heap + 0x28, head);
    put(app, heap + 0xc, head);
    put(app, first, length);
    put(app, first + length, 0xffffffff);
    return first;
}

/* __AdjustAmount (game 0x515628): rounded to 8, room for the headers, at least
 * _amblksiz, whole pages; false where that is 0 or past 4 GB -- `amount`
 * untouched for 0, as far as it got otherwise. */
bool adjust(win32::WinApplication* app, const WatcomHeap& h, x86::reg32& amount)
{
    x86::reg32 size = (amount + 7) & ~x86::reg32(7);
    if (size == 0)
        return false;
    amount = size;
    size += 0x3c;
    if (size < amount)
        return false;
    if (size < word(app, h.blockSize))
        size = word(app, h.blockSize) & ~x86::reg32(1);
    amount = size;
    size += 0xfff;
    if (size < amount)
        return false;
    amount = size & 0xfffff000;
    return amount != 0;
}

/* __ReturnMemToSystem (game 0x5241fc, voodoo2a sub_a8af0c): the rover moved
 * on from the miniheap given back, to the next or else to the first -- the
 * miniheap itself when it was the only one, the runtime's slip, kept. */
bool giveBack(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 heap, x86::reg32& result)
{
    const x86::reg32 after = word(app, heap + 8);
    const auto freed = virtualFree(app, cpu, h, heap, 0, 0x8000);  // MEM_RELEASE
    if (!freed)
        return false;
    if (*freed == 0)
    {
        result = 0xffffffff;
        return true;
    }
    if (heap == word(app, h.rover))
    {
        if (after != 0)
        {
            put(app, h.rover, after);
        }
        else
        {
            put(app, h.largestBefore, 0);
            put(app, h.rover, word(app, h.first));
        }
    }
    if (heap == word(app, h.freeRover))
        put(app, h.freeRover, 0);
    result = 0;
    return true;
}

/* Game 0x52425c, voodoo2a sub_a8aeac. */
bool release(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 heap, x86::reg32& result)
{
    const x86::reg32 before = word(app, heap + 4);
    const x86::reg32 after = word(app, heap + 8);
    if (!giveBack(app, cpu, h, heap, result))
        return false;
    if (result != 0)
        return true;
    if (before == 0)
        put(app, h.first, after);
    else
        put(app, before + 8, after);
    if (after != 0)
        put(app, after + 4, before);
    return true;
}

/* _nheapshrink (game 0x5241c0, voodoo2a sub_a8ae70), under the lock: every
 * miniheap that is one free block, given back. */
bool shrink(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h)
{
    if (!lock(app, cpu, h.access))
        return false;
    for (x86::reg32 heap = word(app, h.first); heap != 0;)
    {
        const x86::reg32 after = word(app, heap + 8);
        x86::reg32 result;
        if (word(app, heap) - 0x2c == word(app, word(app, heap + 0x24)) && !release(app, cpu, h, heap, result))
            return false;
        heap = after;
    }
    return lock(app, cpu, h.release);
}

/* __CreateNewNHeap (game 0x515558, voodoo2a in sub_a88414): a miniheap from
 * VirtualAlloc, linked in, its one block handed to free. */
bool create(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result)
{
    result = 0;
    if (word(app, h.enabled) == 0 || word(app, h.brk) == 0xfffffffe || !adjust(app, h, amount))
        return true;
    const auto made = virtualAlloc(app, cpu, h, 0, amount, 0x1000, 0x40);  // MEM_COMMIT, PAGE_EXECUTE_READWRITE
    if (!made)
        return false;
    const x86::reg32 heap = *made;
    if (heap == 0 || amount - 4 > amount || amount - 4 < 0x38)
        return true;
    put(app, heap, amount - 4);
    const x86::reg32 block = link(app, h, heap);
    put(app, block, word(app, block) | 1);
    put(app, heap + 0x14, 0);
    put(app, heap + 0x18, word(app, heap + 0x18) + 1);
    if (!free(app, cpu, h, block + 4))
        return false;
    result = 1;
    return true;
}

/* __ExpandDGROUP (game 0x515614, voodoo2a sub_a88414). */
bool expand(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result)
{
    return shrink(app, cpu, h) && create(app, cpu, h, amount, result);
}

/* _nfree (game 0x4f79f0, voodoo2a sub_a86da0): the miniheap that holds the
 * block found -- the last free's, those beside it; the rover's, those beside
 * it; then all of them -- and the block given back to it. */
bool free(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 pointer, x86::reg32* eax)
{
    if (eax != nullptr)
        *eax = pointer;  // the block, which the caller's code hands in eax
    if (pointer == 0)
        return true;
    if (!lock(app, cpu, h.access, eax))
        return false;
    auto holds = [app, pointer](x86::reg32 heap) {
        return heap != 0 && heap <= pointer && word(app, heap) + heap > pointer;
    };
    x86::reg32 heap = 0;
    for (const x86::reg32 rover : {h.freeRover, h.rover})
    {
        const x86::reg32 first = word(app, rover);
        if (first == 0)
            continue;
        if (holds(first))
            heap = first;
        else if (holds(word(app, first + 4)))
            heap = word(app, first + 4);
        else if (holds(word(app, first + 8)))
            heap = word(app, first + 8);
        if (heap != 0)
            break;
    }
    for (x86::reg32 each = word(app, h.first); heap == 0 && each != 0; each = word(app, each + 8))
        if (holds(each))
            heap = each;
    if (heap != 0)
    {
        freeIn(app, heap, pointer);
        put(app, h.freeRover, heap);
        if (heap < word(app, h.rover) && word(app, heap + 0x14) > word(app, h.largestBefore))
            put(app, h.largestBefore, word(app, heap + 0x14));
        app->getMemory<x86::reg8>(h.clean) = 0;
    }
    return lock(app, cpu, h.release, eax);
}

/* _nmalloc (game 0x4f7900, voodoo2a sub_a86cb0): from the rover's miniheap on,
 * or from the first when the largest block before the rover may do; failing
 * that the heap grown once, and the search again.  (__nmemneed, which would
 * let the caller give memory back, says no.) */
bool allocate(win32::WinApplication* app, x86::CPU& cpu, const WatcomHeap& h, x86::reg32 amount, x86::reg32& result)
{
    result = 0;
    if (amount == 0 || amount > 0xffffffd4)
        return true;
    x86::reg32 size = (amount + 0xb) & ~x86::reg32(7);
    if (size < 0x10)
        size = 0x10;
    if (!lock(app, cpu, h.access))
        return false;
    for (bool grown = false;;)
    {
        x86::reg32 heap;
        if (size > word(app, h.largestBefore) && word(app, h.rover) != 0)
        {
            heap = word(app, h.rover);
        }
        else
        {
            put(app, h.largestBefore, 0);
            heap = word(app, h.first);
        }
        for (; heap != 0; heap = word(app, heap + 8))
        {
            const x86::reg32 largest = word(app, heap + 0x14);
            put(app, h.rover, heap);
            if (largest >= amount)
            {
                result = allocateIn(app, heap, amount);
                if (result != 0)
                    break;
            }
            if (largest > word(app, h.largestBefore))
                put(app, h.largestBefore, largest);
        }
        if (result != 0 || grown)
            break;
        x86::reg32 expanded;
        if (!expand(app, cpu, h, amount, expanded))
            return false;
        if (expanded == 0)
            break;
        grown = true;
    }
    app->getMemory<x86::reg8>(h.clean) = 0;
    return lock(app, cpu, h.release);
}

}  // namespace watcomheap

/* voodoo2a's malloc and free, for THRASH's records: 0 when the game stopped. */
x86::reg32 thrashMalloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 amount)
{
    x86::reg32 result;
    return watcomheap::allocate(app, cpu, kVoodoo2aHeap, amount, result) ? result : 0;
}

void thrashFree(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pointer)
{
    watcomheap::free(app, cpu, kVoodoo2aHeap, pointer);
}

}  // namespace nfs3hp
