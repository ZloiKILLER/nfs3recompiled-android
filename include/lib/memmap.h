#ifndef LIB_MEMMAP_H
#define LIB_MEMMAP_H

#include <game_types.h>
#include <vector>

namespace nfs
{

class Mutex;

struct Section
{
    const nfs::u8*    data;
    nfs::u32          baseAddress;
    nfs::u32          size;
};

class MemMap
{
public:
    MemMap(nfs::u32 size);
    ~MemMap();

    nfs::u32 getBlockStart() const  { return s_addressOffset + m_block * granularity(); }
    nfs::u32 getBlockSize() const   { return m_blockCount * granularity(); }

    /* Allocation granularity of one guest block.  It must be at least the host
     * page size: mprotect() rounds to whole pages, so a smaller block would let
     * freeing one block revoke access to its live neighbours.  Android 15+ can
     * run a 16 KB page kernel, where the historical hardcoded 4096 also made
     * mprotect() fail outright with EINVAL. */
    static nfs::u32 granularity();
    static nfs::u32 pageSize();

    static nfs::u8* init(nfs::u32 baseAddress, const std::vector<Section>& sections);
    static void fini();
    static MemMap* findBlock(nfs::u32 memIndex);

    /* Every range of guest addresses the game can write, as (start, size):
     * the image's sections, then each run of allocated blocks. */
    static void usedRanges(std::vector<std::pair<nfs::u32, nfs::u32>>& ranges);
    /* The allocated block an address lies in, as (start, size); (0, 0) when
     * none does, a section's address included. */
    static std::pair<nfs::u32, nfs::u32> blockOf(nfs::u32 address);

    static void fillDebugGraph(nfs::u16* graph);
    static void fillDebugGraph(nfs::u32* graph);
private:
    nfs::u32 m_block;
    nfs::u32 m_blockCount;

private:
    static nfs::u8*           s_memory;
    static nfs::u8*           s_blocks;
    static nfs::u32           s_blockCount;
    static nfs::u32           s_blockSize;
    static nfs::u32           s_sectionSize;
    static nfs::u32           s_addressOffset;
    static Mutex*               s_lock;
    static std::vector<MemMap*> s_memMaps;
    static std::vector<std::pair<nfs::u32, nfs::u32>> s_sections;
};

}

#endif // MEMMAP_H
