#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <vector>
#ifdef _WIN32
# include <windows.h>
#else
# include <sys/mman.h>
# include <unistd.h>
#endif
#include <lib/mutex.h>
#include <cstdlib>
#include <new>
#include <algorithm>

namespace win32
{

static const x86::reg32 s_simulatedMemory = 128*1024*1024;

x86::reg8* MemMap::s_memory = nullptr;
x86::reg8* MemMap::s_blocks = nullptr;
x86::reg32 MemMap::s_blockCount = 0;
x86::reg32 MemMap::s_blockSize = 0;
x86::reg32 MemMap::s_sectionSize = 0;
x86::reg32 MemMap::s_addressOffset = 0;
Mutex*     MemMap::s_lock = nullptr;
std::vector<MemMap*> MemMap::s_memMaps;

x86::reg32 MemMap::pageSize()
{
    static const x86::reg32 s_pageSize = []() -> x86::reg32 {
#ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return x86::reg32(si.dwPageSize);
#else
        long v = sysconf(_SC_PAGESIZE);
        return v > 0 ? x86::reg32(v) : 4096u;
#endif
    }();
    return s_pageSize;
}

x86::reg32 MemMap::granularity()
{
    const x86::reg32 p = pageSize();
    return p > 4096u ? p : 4096u;
}


static void* alloc(x86::reg32 size)
{
#ifdef _WIN32
    void* p = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!p) throw std::bad_alloc();
    return p;
#else
    void* p = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (p == MAP_FAILED) throw std::bad_alloc();
    return p;
#endif
}

static void dealloc(void* mem, x86::reg32 size)
{
#ifdef _WIN32
    NFS2_USE(size);
    VirtualFree(mem, 0, MEM_RELEASE);
#else
    munmap(mem, size);
#endif
}

static bool protect(void* mem, x86::reg32 size, bool read, bool write)
{
#ifdef _WIN32
    NFS2_USE(mem);
    NFS2_USE(size);
    NFS2_USE(read);
    NFS2_USE(write);
    return true;
#else
    int protection = PROT_NONE;
    if (read)
        protection |= PROT_READ;
    if (write)
        protection |= PROT_WRITE;
    uintptr_t addr = reinterpret_cast<uintptr_t>(mem);
    const uintptr_t pageMask = uintptr_t(MemMap::pageSize()) - 1;
    uintptr_t alignedAddr = addr & ~pageMask;
    x86::reg32 alignedSize = x86::reg32(((addr - alignedAddr) + size + pageMask) & ~pageMask);
    int ret = mprotect(reinterpret_cast<void*>(alignedAddr), alignedSize, protection);
    return ret == 0;
#endif
}

MemMap::MemMap(x86::reg32 size)
    :   m_block(x86::reg32(-1))
    ,   m_blockCount(0)
{
    const x86::reg32 g = granularity();
    // Validate before rounding or subtracting unsigned sizes.
    if (!size || size > s_simulatedMemory) throw std::bad_alloc();
    m_blockCount = (size - 1) / g + 1;
    s_lock->lock();
    for (x86::reg32 b = 0; b <= s_blockCount - m_blockCount;)
    {
        x86::reg32 end = b;
        while (end < b + m_blockCount && !s_blocks[end]) ++end;
        if (end == b + m_blockCount) { m_block = b; break; }
        b = end + 1;
    }
    if (m_block == x86::reg32(-1))
    {
        s_lock->unlock();
        throw std::bad_alloc();
    }
    if (!protect(s_memory + getBlockStart(), getBlockSize(), true, true))
    {
        s_lock->unlock();
        throw std::bad_alloc();
    }
    try { s_memMaps.push_back(this); }
    catch (...)
    {
        protect(s_memory + getBlockStart(), getBlockSize(), false, false);
        s_lock->unlock();
        throw;
    }
    std::fill(s_blocks + m_block, s_blocks + m_block + m_blockCount, 1);
    s_lock->unlock();
}

MemMap::~MemMap()
{
    protect(s_memory + s_sectionSize + s_blockSize + m_block * granularity(), m_blockCount * granularity(), false, false);
    s_lock->lock();
    for (std::vector<MemMap*>::iterator it = s_memMaps.begin(); it != s_memMaps.end(); ++it)
    {
        if (*it == this)
        {
            s_memMaps.erase(it);
            break;
        }
    }
    for (x86::reg32 s = m_block; s < m_block + m_blockCount; ++s)
    {
        s_blocks[s] = 0;
    }
    s_lock->unlock();
}

MemMap* MemMap::findBlock(x86::reg32 memIndex)
{
    NFS2_ASSERT(memIndex >= s_addressOffset);
    NFS2_ASSERT(memIndex < s_addressOffset + s_simulatedMemory);
    MemMap* result = nullptr;
    s_lock->lock();
    for (std::vector<MemMap*>::iterator it = s_memMaps.begin(); it != s_memMaps.end(); ++it)
    {
        if ((*it)->getBlockStart() <= memIndex && (*it)->getBlockStart() + (*it)->getBlockSize() > memIndex)
        {
            NFS2_ASSERT(!result);
            result = *it;
            break;
        }
    }
    s_lock->unlock();
    return result;
}

x86::reg8* MemMap::init(x86::reg32 baseAddress, const std::vector<Section>& sections)
{
    const x86::reg32 g = granularity();
    s_blockCount = s_simulatedMemory / g;
    s_blockSize = (s_blockCount + g - 1) & ~(g - 1);
    x86::reg32 maxEnd = 0;
    for (std::vector<Section>::const_iterator it = sections.begin(); it != sections.end(); ++it)
    {
        x86::reg32 end = it->baseAddress + it->size;
        if (end > maxEnd)
            maxEnd = end;
    }
    s_sectionSize = (maxEnd + g - 1) & ~(g - 1);
    s_memory = reinterpret_cast<x86::reg8*>(alloc(s_simulatedMemory+s_blockSize+s_sectionSize));
    s_blocks = s_memory + s_sectionSize;
    s_addressOffset = s_sectionSize + s_blockSize;
    auto enable = [](void* address, x86::reg32 size) {
        if (!protect(address, size, true, true)) {
            dealloc(s_memory, s_simulatedMemory+s_blockSize+s_sectionSize);
            s_memory = nullptr;
            throw std::bad_alloc();
        }
    };
    enable(s_blocks, s_blockCount);
    for (std::vector<Section>::const_iterator it = sections.begin(); it != sections.end(); ++it)
    {
        NFS2_ASSERT(it->baseAddress >= baseAddress);
        NFS2_ASSERT(it->baseAddress + it->size <= s_sectionSize);
        enable(s_memory + it->baseAddress, it->size);
        if (it->data)
        {
            memcpy(s_memory + it->baseAddress, it->data, it->size);
        }
    }
    s_lock = new Mutex("MemMap");
    return s_memory;
}

void MemMap::fini()
{
    std::vector<MemMap*> memMapsCopy(s_memMaps);
    for (std::vector<MemMap*>::iterator it = memMapsCopy.begin(); it != memMapsCopy.end(); ++it)
    {
        delete *it;
    }
    NFS2_ASSERT(s_memMaps.empty());
    s_blocks = nullptr;
    s_blockCount = 0;
    s_blockSize = 0;
    s_addressOffset = 0;
    delete s_lock;
    s_lock = nullptr;
    dealloc(s_memory, s_simulatedMemory + s_blockSize + s_sectionSize);
}

void MemMap::fillDebugGraph(x86::reg16* graph)
{
    s_lock->lock();
    const x86::reg32 blocksPerPixel = (s_blockCount+639)/640;
    for (x86::reg32 b = 0; b < s_blockCount; b += blocksPerPixel)
    {
        x86::reg16 c = 0;
        for (x86::reg32 b1 = b; b1 < (std::min)(b+blocksPerPixel, s_blockCount); ++b1)
        {
            if (s_blocks[b1])
                c++;
        }
        x86::reg16 r = x86::reg16((0x1f*c/blocksPerPixel) << (5+6));
        x86::reg16 g = x86::reg16((0x3f*(blocksPerPixel-c)/blocksPerPixel) << 5);
        graph[b/blocksPerPixel + 640*0] = r|g;
        graph[b/blocksPerPixel + 640*1] = r|g;
        graph[b/blocksPerPixel + 640*2] = r|g;
    }
    s_lock->unlock();
}

void MemMap::fillDebugGraph(x86::reg32* graph)
{
    s_lock->lock();
    const x86::reg32 blocksPerPixel = (s_blockCount+639)/640;
    for (x86::reg32 b = 0; b < s_blockCount; b += blocksPerPixel)
    {
        x86::reg16 c = 0;
        for (x86::reg32 b1 = b; b1 < (std::min)(b+blocksPerPixel, s_blockCount); ++b1)
        {
            if (s_blocks[b1])
                c++;
        }
        x86::reg32 r = (0xff*c/blocksPerPixel) << 24;
        x86::reg32 g = (0xff*(blocksPerPixel-c)/blocksPerPixel) << 16;
        graph[b/blocksPerPixel + 640*0] = r|g;
        graph[b/blocksPerPixel + 640*1] = r|g;
        graph[b/blocksPerPixel + 640*2] = r|g;
    }
    s_lock->unlock();
}

}
