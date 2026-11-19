#include "native_watcom_heap.h"
#include "native_guest.h"
#include "native_thrash.h"
#include "os_system.h"

/* Watcom's near heap, as the runtime's code keeps it, on its variables in
 * guest memory (WatcomHeap): the game's -- whose functions native_watcom.cpp
 * binds -- and voodoo2a's, whose malloc and free THRASH_talloc and
 * THRASH_lockwindow take their records from, so they lie where voodoo2a's code
 * would have put them.  The two runtimes are Watcom 11's and have the same
 * heap; the game's takes the heap's lock around it, voodoo2a's would take its
 * own, a bare ret, which is not called here.
 *
 * The heap is a list of "miniheaps", each a block of the port's memory
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

const WatcomHeap kGameHeap = {
    0x566f4c, 0x566f50, 0x566f54, 0x9f3800, 0xa0b140, 0x56af68, 0x5677f8, 0x56af6c, 0x567780, 0x567788,
};

const WatcomHeap kVoodoo2aHeap = {
    0xa9267c, 0xa92680, 0xa92684, 0xa93850, 0xa93eb0, 0xa92830, 0xa9273c, 0xa92834, 0, 0,
};

namespace watcomheap
{

namespace
{

nfs::u32 word(nfs::App* app, nfs::u32 address)
{
    return app->getMemory<nfs::u32>(address);
}

using guest::put;

/* The heap's lock taken, or given back, through its pointer: false when the game stopped; what the
 * call leaves in eax into `eax`, if asked, which is as it was for no lock. */
bool lock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 slot, nfs::u32* eax = nullptr)
{
    if (slot == 0)
        return true;
    const auto left = guest::pointerCall(app, ts, word(app, slot), {});
    if (!left)
        return false;
    if (eax != nullptr)
        *eax = *left;
    return true;
}

}  // namespace

/* __MemAllocator (game 0x515300, voodoo2a sub_a88100). */
nfs::u32 allocateIn(nfs::App* app, nfs::u32 heap, nfs::u32 amount)
{
    if (amount == 0)
        return 0;
    nfs::u32 size = amount + 0xb;
    if (size < amount)
        return 0;
    size &= ~nfs::u32(7);
    if (size < 0x10)
        size = 0x10;
    if (size > word(app, heap + 0x14))
        return 0;
    nfs::u32 largest = word(app, heap + 0x10);
    nfs::u32 block = word(app, heap + 0xc);
    if (size <= largest)
    {
        block = word(app, heap + 0x28);
        largest = 0;
    }
    const nfs::u32 head = heap + 0x20;
    nfs::u32 length;
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
    const nfs::u32 left = length - size;
    if (left >= 0x10)
    {
        const nfs::u32 rest = block + size;
        put(app, heap + 0xc, rest);
        put(app, rest, left);
        put(app, block, size);
        const nfs::u32 before = word(app, block + 4);
        put(app, rest + 4, before);
        const nfs::u32 after = word(app, block + 8);
        put(app, rest + 8, after);
        put(app, before + 8, rest);
        put(app, after + 4, rest);
    }
    else
    {
        put(app, heap + 0x1c, word(app, heap + 0x1c) - 1);
        const nfs::u32 before = word(app, block + 4);
        put(app, heap + 0xc, before);
        const nfs::u32 after = word(app, block + 8);
        put(app, before + 8, after);
        put(app, after + 4, before);
    }
    app->getMemory<nfs::u8>(block) |= 1;
    return block + 4;
}

/* __MemFree (game 0x5153b0, voodoo2a sub_a881b0): joined to the free blocks
 * beside it. */
void freeIn(nfs::App* app, nfs::u32 heap, nfs::u32 pointer)
{
    if (pointer == 0)
        return;
    nfs::u32 block = pointer - 4;
    if (!(app->getMemory<nfs::u8>(block) & 1))
        return;
    nfs::u32 length = word(app, block) & ~nfs::u32(1);
    nfs::u32 next = block + length;
    nfs::u32 at;  // the free block `block` goes before
    if (!(app->getMemory<nfs::u8>(next) & 1))
    {
        // The one after is free: the two become one, which takes its place.
        length += word(app, next);
        put(app, block, length);
        if (next == word(app, heap + 0xc))
            put(app, heap + 0xc, block);
        const nfs::u32 before = word(app, next + 4);
        at = word(app, next + 8);
        put(app, before + 8, at);
        put(app, at + 4, before);
        put(app, heap + 0x1c, word(app, heap + 0x1c) - 1);
    }
    else
    {
        put(app, block, length);
        bool search = false;
        const nfs::u32 rover = word(app, heap + 0xc);
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
            const nfs::u32 free = word(app, heap + 0x1c);
            const nfs::u32 used = word(app, heap + 0x18);
            nfs::u32 tries = used / (free + 1);
            bool found = false;
            if (tries < free)
            {
                tries += tries;
                if (used - free <= free)
                    tries = 0xffffffff;
                nfs::u32 ahead = word(app, block) + block;
                while (true)
                {
                    const nfs::u32 header = word(app, ahead);
                    if (!(header & 1))
                    {
                        at = ahead;
                        found = true;
                        break;
                    }
                    if (header == 0xffffffff)
                        break;
                    ahead += header & ~nfs::u32(1);
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
    const nfs::u32 before = word(app, at + 4);
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
nfs::u32 link(nfs::App* app, const WatcomHeap& h, nfs::u32 heap)
{
    nfs::u32 before = 0;
    nfs::u32 after = word(app, h.first);
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
    const nfs::u32 head = heap + 0x20;
    const nfs::u32 first = heap + 0x2c;
    put(app, heap + 0x20, 0);
    put(app, heap + 0x10, 0);
    put(app, heap + 0x18, 0);
    put(app, heap + 0x1c, 0);
    put(app, heap + 0x24, head);
    const nfs::u32 length = word(app, heap) - 0x2c;
    put(app, heap + 0x28, head);
    put(app, heap + 0xc, head);
    put(app, first, length);
    put(app, first + length, 0xffffffff);
    return first;
}

/* __AdjustAmount (game 0x515628): rounded to 8, room for the headers, at least
 * _amblksiz, whole pages; false where that is 0 or past 4 GB -- `amount`
 * untouched for 0, as far as it got otherwise. */
bool adjust(nfs::App* app, const WatcomHeap& h, nfs::u32& amount)
{
    nfs::u32 size = (amount + 7) & ~nfs::u32(7);
    if (size == 0)
        return false;
    amount = size;
    size += 0x3c;
    if (size < amount)
        return false;
    if (size < word(app, h.blockSize))
        size = word(app, h.blockSize) & ~nfs::u32(1);
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
bool giveBack(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 heap, nfs::u32& result)
{
    NFS2_USE(ts);
    const nfs::u32 after = word(app, heap + 8);
    os::memory::release(heap);
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
bool release(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 heap, nfs::u32& result)
{
    const nfs::u32 before = word(app, heap + 4);
    const nfs::u32 after = word(app, heap + 8);
    if (!giveBack(app, ts, h, heap, result))
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
bool shrink(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h)
{
    if (!lock(app, ts, h.access))
        return false;
    for (nfs::u32 heap = word(app, h.first); heap != 0;)
    {
        const nfs::u32 after = word(app, heap + 8);
        nfs::u32 result;
        if (word(app, heap) - 0x2c == word(app, word(app, heap + 0x24)) && !release(app, ts, h, heap, result))
            return false;
        heap = after;
    }
    return lock(app, ts, h.release);
}

/* __CreateNewNHeap (game 0x515558, voodoo2a in sub_a88414): a miniheap of the
 * port's memory, linked in, its one block handed to free. */
bool create(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result)
{
    result = 0;
    if (word(app, h.enabled) == 0 || word(app, h.brk) == 0xfffffffe || !adjust(app, h, amount))
        return true;
    const nfs::u32 heap = os::memory::allocate(app, amount);
    if (heap == 0 || amount - 4 > amount || amount - 4 < 0x38)
        return true;
    put(app, heap, amount - 4);
    const nfs::u32 block = link(app, h, heap);
    put(app, block, word(app, block) | 1);
    put(app, heap + 0x14, 0);
    put(app, heap + 0x18, word(app, heap + 0x18) + 1);
    if (!free(app, ts, h, block + 4))
        return false;
    result = 1;
    return true;
}

/* __ExpandDGROUP (game 0x515614, voodoo2a sub_a88414). */
bool expand(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result)
{
    return shrink(app, ts, h) && create(app, ts, h, amount, result);
}

/* _nfree (game 0x4f79f0, voodoo2a sub_a86da0): the miniheap that holds the
 * block found -- the last free's, those beside it; the rover's, those beside
 * it; then all of them -- and the block given back to it. */
bool free(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 pointer, nfs::u32* eax)
{
    if (eax != nullptr)
        *eax = pointer;  // the block, which the caller's code hands in eax
    if (pointer == 0)
        return true;
    if (!lock(app, ts, h.access, eax))
        return false;
    auto holds = [app, pointer](nfs::u32 heap) {
        return heap != 0 && heap <= pointer && word(app, heap) + heap > pointer;
    };
    nfs::u32 heap = 0;
    for (const nfs::u32 rover : {h.freeRover, h.rover})
    {
        const nfs::u32 first = word(app, rover);
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
    for (nfs::u32 each = word(app, h.first); heap == 0 && each != 0; each = word(app, each + 8))
        if (holds(each))
            heap = each;
    if (heap != 0)
    {
        freeIn(app, heap, pointer);
        put(app, h.freeRover, heap);
        if (heap < word(app, h.rover) && word(app, heap + 0x14) > word(app, h.largestBefore))
            put(app, h.largestBefore, word(app, heap + 0x14));
        app->getMemory<nfs::u8>(h.clean) = 0;
    }
    return lock(app, ts, h.release, eax);
}

/* _nmalloc (game 0x4f7900, voodoo2a sub_a86cb0): from the rover's miniheap on,
 * or from the first when the largest block before the rover may do; failing
 * that the heap grown once, and the search again.  (__nmemneed, which would
 * let the caller give memory back, says no.) */
bool allocate(nfs::App* app, nfs::ThreadState& ts, const WatcomHeap& h, nfs::u32 amount, nfs::u32& result)
{
    result = 0;
    if (amount == 0 || amount > 0xffffffd4)
        return true;
    nfs::u32 size = (amount + 0xb) & ~nfs::u32(7);
    if (size < 0x10)
        size = 0x10;
    if (!lock(app, ts, h.access))
        return false;
    for (bool grown = false;;)
    {
        nfs::u32 heap;
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
            const nfs::u32 largest = word(app, heap + 0x14);
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
        nfs::u32 expanded;
        if (!expand(app, ts, h, amount, expanded))
            return false;
        if (expanded == 0)
            break;
        grown = true;
    }
    app->getMemory<nfs::u8>(h.clean) = 0;
    return lock(app, ts, h.release);
}

}  // namespace watcomheap

/* voodoo2a's malloc and free, for THRASH's records: 0 when the game stopped. */
nfs::u32 thrashMalloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 amount)
{
    nfs::u32 result;
    return watcomheap::allocate(app, ts, kVoodoo2aHeap, amount, result) ? result : 0;
}

void thrashFree(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pointer)
{
    watcomheap::free(app, ts, kVoodoo2aHeap, pointer);
}

}  // namespace nfs3hp
