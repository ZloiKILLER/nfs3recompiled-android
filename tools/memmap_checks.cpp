#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <memory>
#include <new>
#include <stdexcept>
#include <cstdio>
using win32::MemMap;
static void check(bool ok) { if (!ok) throw std::runtime_error("memory check failed"); }
static void reject(unsigned size) {
    bool failed = false;
    try { MemMap block(size); } catch (const std::bad_alloc&) { failed = true; }
    check(failed);
}
int main() {
    try {
        check(SDL_Init(0));
        auto* memory = MemMap::init(0, {});
        constexpr unsigned pool = 128u*1024u*1024u;
        reject(0); reject(pool+1); reject(0xffffffffu);
        {
            MemMap whole(pool);
            memory[whole.getBlockStart()] = 21;
            memory[whole.getBlockStart()+whole.getBlockSize()-1] = 42;
            reject(1);
        }
        {
            MemMap heap(64u*1024u*1024u);
            MemMap remainder(64u*1024u*1024u);
            check(remainder.getBlockStart() == heap.getBlockStart()+heap.getBlockSize());
            reject(1);
        }
        {
            auto first = std::unique_ptr<MemMap>(new MemMap(1));
            auto middle = std::unique_ptr<MemMap>(new MemMap(1));
            MemMap tail(pool - 2*MemMap::granularity());
            auto address = middle->getBlockStart();
            middle.reset();
            MemMap reused(1);
            check(reused.getBlockStart() == address);
            memory[tail.getBlockStart()] = 7;
            check(memory[tail.getBlockStart()] == 7);
        }
        MemMap::fini(); SDL_Quit();
        std::puts("PASS: full pool, 64 MiB heap, exhaustion, overflow, reuse");
        return 0;
    } catch (const std::exception& e) { std::fprintf(stderr,"%s\n",e.what()); return 1; }
}
