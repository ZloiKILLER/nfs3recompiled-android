#ifndef LIB_MEMMAP_H
#define LIB_MEMMAP_H

#include <x86.h>
#include <vector>

namespace win32
{

class Mutex;

struct Section
{
    const x86::reg8*    data;
    x86::reg32          baseAddress;
    x86::reg32          size;
};

class MemMap
{
public:
    MemMap(x86::reg32 size);
    ~MemMap();

    x86::reg32 getBlockStart() const  { return s_addressOffset + m_block * granularity(); }
    x86::reg32 getBlockSize() const   { return m_blockCount * granularity(); }

    /* Allocation granularity of one guest block.  It must be at least the host
     * page size: mprotect() rounds to whole pages, so a smaller block would let
     * freeing one block revoke access to its live neighbours.  Android 15+ can
     * run a 16 KB page kernel, where the historical hardcoded 4096 also made
     * mprotect() fail outright with EINVAL. */
    static x86::reg32 granularity();
    static x86::reg32 pageSize();

    static x86::reg8* init(x86::reg32 baseAddress, const std::vector<Section>& sections);
    static void fini();
    static MemMap* findBlock(x86::reg32 memIndex);

    static void fillDebugGraph(x86::reg16* graph);
    static void fillDebugGraph(x86::reg32* graph);
private:
    x86::reg32 m_block;
    x86::reg32 m_blockCount;

private:
    static x86::reg8*           s_memory;
    static x86::reg8*           s_blocks;
    static x86::reg32           s_blockCount;
    static x86::reg32           s_blockSize;
    static x86::reg32           s_sectionSize;
    static x86::reg32           s_addressOffset;
    static Mutex*               s_lock;
    static std::vector<MemMap*> s_memMaps;
};

}

#endif // MEMMAP_H
