// game_checks.cpp's parts for EA's memory manager and its memmove (native_ea_memory.cpp), included in each
// table with the macro naming the part.
#if defined(EA_WINDOWS)
// {from, to}, -- what the functions write, compared
// The class table (0x9ef5c0), where an error is from (0x552190), the default
// class and memmove's choice (0x5643ac to 0x56440c) and the cases' classes and
// copies (0x61d000 on, the harness's) are windows already.
#elif defined(EA_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
#elif defined(EA_EFFECT_CASES)
// case kSomething: ... break;
#elif defined(EA_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// None of its own: reservememadr and purgememadr (game_checks.cpp's), the
// locks made and freed (sound_lib.inc's), sprintf; Error, the game's fill and
// the MMX copy are tested natives, called as they are.
#elif defined(EA_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// EnterCriticalSection and LeaveCriticalSection: the game's own (kPointers).
#elif defined(EA_PROMISES)
// {"ea::name", watcom, result, arguments, fpu, heap, liveOnly},
{"ea::findFreeBlock", true, true, 0},
{"ea::findLargestFreeBlock", true, true, 0},
{"ea::linkFreeBlock", true, true, 0},
{"ea::unlinkFreeBlock", true, true, 0},
{"ea::clearMemClasses", true, true, 0},
{"ea::markMemBlock", true, true, 0},
{"ea::memClassOverhead", true, true, 0},
{"ea::memBlockOverhead", true, true, 0},
{"ea::creatememclass", true, true, 0},
{"ea::releasememclass", true, true, 0},
{"ea::getfreememclass", true, true, 0},
{"ea::largestmem", true, true, 0},
{"ea::largestmemclass", true, true, 0},
{"ea::memBlockSize", true, true, 0},
{"ea::memBlockName", true, true, 0},
{"ea::memBlockFlags", true, true, 0},
{"ea::reservememadr", true, true, 0},
{"ea::purgememadr", true, true, 0},
{"ea::purgememclass", true, true, 0},
{"ea::reservememStdcall", true, true, 0},
{"ea::purgememStdcall", true, true, 0},
// The copies through the x87's stack: its words compared too, and its
// registers in use (none, after).
{"ea::memmove", true, true, 0, true, kNoHeap, true},
#elif defined(EA_ARGUMENTS)
/* Where the memory cases keep their own: two classes' memory, a block marked
 * on its own, bytes copied (all of it, for the copies). */
constexpr x86::reg32 kEaMemClassA = 0x61d000, kEaMemClassB = 0x61e400, kEaMemClassesEnd = 0x61f000;
constexpr x86::reg32 kEaMemMarked = 0x61f000, kEaMemCopies = 0x61d000, kEaMemEnd = 0x620000;
constexpr x86::reg32 kEaMemTable = 0x9ef5c0, kEaMemDefault = 0x5643f4, kEaMemReport = 0x56440c;

/* A class made up as memstd.c keeps one: its blocks in memory, the free
 * ones in its free list. */
struct EaMemClass
{
    x86::reg32 index = 0, base = 0, header = 0, high = 0;
    std::vector<x86::reg32> blocks;  // the blocks between LOW and HIGH
    std::vector<bool> free;
};

/* A word made up with no "MB" in it, at either of its halves -- what a
 * pointer into a class's memory that is not a block's finds there. */
x86::reg32 eaNoBlockWord()
{
    x86::reg32 w = random32();
    if ((w & 0xffff) == 0x424d)
        w ^= 1;
    if ((w >> 16) == 0x424d)
        w ^= 0x10000;
    if ((w >> 8 & 0xffff) == 0x424d)
        w ^= 0x100;
    return w;
}

x86::reg16& eaHalf(CheckApp& g, x86::reg32 at)
{
    return reinterpret_cast<x86::reg16&>(*&g.getMemory<x86::reg8>(at));
}

/* Class `index` over [from, to): LOW and its header, blocks of sizes as the
 * class aligns them -- free ones in the free list by address, their sizes as
 * the blocks lie (`exact`) or now and then not -- and HIGH; its flags, its
 * overhead, its lock or none. */
EaMemClass makeEaMemClass(CheckApp& g, x86::reg32 index, x86::reg32 from, x86::reg32 to, bool exact)
{
    EaMemClass c;
    c.index = index;
    c.base = from + 4 * below(0x10);
    c.header = c.base + 0x10;
    static const x86::reg32 aligns[] = {4, 8, 0x10, 0x20};
    const x86::reg32 align = chance(85) ? aligns[below(4)] : 1u << below(7);
    const x86::reg32 overhead = chance(40) ? 0 : chance(80) ? 4 * below(8) : below(0x20);
    x86::reg32 flags = index | (chance(50) ? 0x100 : 0) | (chance(40) ? 0x200 : 0) | (chance(40) ? 0x400 : 0)
                       | (chance(25) ? 0x800 : 0);
    if (chance(10))
        flags = random32();
    const x86::reg32 start = c.base + 0x60 + below(0x20);
    x86::reg32 at = (start + 0xf) & ~0xfu;
    const unsigned count = 1 + below(8);
    for (unsigned i = 0; i < count; ++i)
    {
        x86::reg32 span = 0x10 + (((chance(70) ? 0x10 + below(0x100) : below(0x300)) + align - 1) & ~(align - 1));
        if (span < 0x18)
            span = 0x18;
        if (at + span + 0x100 > to)
            break;
        c.blocks.push_back(at);
        c.free.push_back(chance(45));
        at += span;
    }
    c.high = at;
    // LOW, the header's block; the header.
    eaHalf(g, c.base) = 0x424d;
    eaHalf(g, c.base + 2) = x86::reg16(index | 0x80 | (flags & 0x700));
    g.word(c.base + 4) = 0x40;
    g.word(c.base + 8) = c.blocks.empty() ? c.high : c.blocks[0];
    g.word(c.base + 0xc) = 0;
    g.text(c.header, below(8), random32());
    g.word(c.header + 8) = c.base;
    g.word(c.header + 0xc) = c.high;
    const x86::reg32 sentinel = c.header + 0x10;
    eaHalf(g, sentinel) = 0x4253;
    eaHalf(g, sentinel + 2) = x86::reg16(random32() & 0xbfff);
    g.word(sentinel + 4) = 0x7fffffff;
    g.word(sentinel + 8) = random32();
    g.word(sentinel + 0xc) = random32();
    g.word(c.header + 0x28) = align;
    g.word(c.header + 0x2c) = chance(80) ? aligns[below(4)] : random32();
    g.word(c.header + 0x30) = overhead;
    g.word(c.header + 0x34) = flags;
    g.word(c.header + 0x38) = chance(50) ? 0 : 0x630000 + 0x10 * below(0x100);
    g.word(c.header + 0x3c) = random32();
    // The blocks, their bodies made up.
    std::vector<x86::reg32> list;
    for (size_t i = 0; i < c.blocks.size(); ++i)
    {
        const x86::reg32 b = c.blocks[i];
        const x86::reg32 next = i + 1 < c.blocks.size() ? c.blocks[i + 1] : c.high;
        for (x86::reg32 k = b + 0x10; k < next; k += 4)
            g.word(k) = eaNoBlockWord();
        g.word(b + 8) = next;
        g.word(b + 0xc) = i == 0 ? c.base : c.blocks[i - 1];
        if (c.free[i])
        {
            eaHalf(g, b) = 0x4246;
            eaHalf(g, b + 2) = x86::reg16((chance(80) ? index | (random32() & 0x8f00) : random32()) | 0x4000);
            g.word(b + 4) = exact || chance(90) ? next - b - 0x10 : below(0x400);
            list.push_back(b);
        }
        else
        {
            eaHalf(g, b) = 0x424d;
            eaHalf(g, b + 2) = x86::reg16(chance(90) ? index | (flags & 0x700) | (chance(50) ? 0x100 : 0)
                                                     : (index | random32()) & 0xbfff);
            const x86::reg32 room = next - b - 0x10;
            g.word(b + 4) = chance(90) ? below(room + 1) : random32();
        }
    }
    // HIGH.
    eaHalf(g, c.high) = 0x424d;
    eaHalf(g, c.high + 2) = x86::reg16(index | 0x8010 | (flags & 0x700));
    g.word(c.high + 4) = 0;
    g.word(c.high + 8) = 0;
    g.word(c.high + 0xc) = c.blocks.empty() ? c.base : c.blocks.back();
    // The free list, by address -- as linkFreeBlock keeps it: out of order,
    // its walk from the start may never stop.
    list.insert(list.begin(), sentinel);
    for (size_t i = 0; i < list.size(); ++i)
    {
        g.word(list[i] + 0x10) = list[(i + 1) % list.size()];
        g.word(list[i] + 0x14) = list[(i + list.size() - 1) % list.size()];
    }
    return c;
}

/* A block of the class's: in use or free as `free` asks, if it has one. */
x86::reg32 anEaBlock(const EaMemClass& c, int free)
{
    std::vector<x86::reg32> found;
    for (size_t i = 0; i < c.blocks.size(); ++i)
        if (free < 0 || c.free[i] == (free != 0))
            found.push_back(c.blocks[i]);
    if (found.empty())
        return c.blocks.empty() ? c.high : c.blocks[below(x86::reg32(c.blocks.size()))];
    return found[below(x86::reg32(found.size()))];
}

/* A class number: of the two classes mostly, any other now and then, the
 * high bits anything. */
x86::reg32 anEaClass(const EaMemClass& a, const EaMemClass& b)
{
    const x86::reg32 index = chance(45) ? a.index : chance(80) ? b.index : below(16);
    return chance(70) ? index : (random32() & ~0xfu) | index;
}

bool makeEaMemoryArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "findFreeBlock", "findLargestFreeBlock", "linkFreeBlock", "unlinkFreeBlock", "clearMemClasses",
        "markMemBlock", "memClassOverhead", "memBlockOverhead", "creatememclass", "releasememclass",
        "getfreememclass", "largestmem", "largestmemclass", "memBlockSize", "memBlockName", "memBlockFlags",
        "reservememadr", "purgememadr", "purgememclass", "reservememStdcall", "purgememStdcall", "memmove",
        "fpuCopySwapped", "fpuCopy", "dwordCopySwapped", "dwordCopy"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    g.word(kEaMemReport) = chance(70) ? 1 : chance(50) ? 0 : random32();
    g.word(kEaMemDefault) = random32();
    // What a block's neighbour at 0 -- LOW's before it, HIGH's after -- has:
    // not free.
    g.word(0) = 0;

    // The copies: every byte of their memory made up, where they go and from.
    if (name == "memmove" || name == "fpuCopySwapped" || name == "fpuCopy" || name == "dwordCopySwapped"
        || name == "dwordCopy")
    {
        for (x86::reg32 at = kEaMemCopies; at < kEaMemEnd; at += 4)
            g.word(at) = random32();
        const x86::reg32 from = kEaMemCopies + below(0x1400);
        x86::reg32 to = chance(40) ? from + below(0x40) - 0x20 : kEaMemCopies + below(0x1400);
        const x86::reg32 count = chance(80) ? below(0x200) : chance(90) ? below(0x800) : 0x100 * below(8);
        if (name == "memmove")
        {
            // Now and then just past the source: no overlap, forwards.
            if (chance(5))
                to = from + count;
            g.word(0x5643b4) = chance(30) ? (chance(80) ? 1 : random32()) : 0;
            g.word(0x5643ac) = chance(50) ? (chance(80) ? 1 : random32()) : 0;
            cpu.eax = from;
            cpu.edx = to;
            cpu.ebx = count;
        }
        else
        {
            const bool swapped = name == "fpuCopySwapped" || name == "dwordCopySwapped";
            cpu.eax = swapped ? from : to;
            cpu.edx = swapped ? to : from;
            // Less than nothing now and then: a dword, a word, a byte copied.
            cpu.ebx = chance(95) ? count : x86::reg32(-1 - x86::sreg32(below(0x100)));
        }
        return true;
    }

    // Two classes over memory with no blocks of the cases before -- no "MB"
    // anywhere but where a block is -- the table naming them -- the others too, mostly, or none.
    for (x86::reg32 at = kEaMemClassA; at < kEaMemEnd; at += 4)
        g.word(at) = eaNoBlockWord();
    for (x86::reg32 at = kEaMemClassA + 3; at < kEaMemEnd; at += 4)
        if (g.getMemory<x86::reg8>(at) == 0x4d && g.getMemory<x86::reg8>(at + 1) == 0x42)
            g.getMemory<x86::reg8>(at) = 0x4c;
    x86::reg32 ia = below(16), ib = below(15);
    if (ib >= ia)
        ++ib;
    // A free block larger than it lies, split, puts the rest over the blocks
    // after it -- over the free list's links: reservememadr's blocks as
    // they lie.
    const bool exact = name == "reservememadr";
    const EaMemClass a = makeEaMemClass(g, ia, kEaMemClassA, kEaMemClassB, exact);
    const EaMemClass b = makeEaMemClass(g, ib, kEaMemClassB, kEaMemClassesEnd, exact);
    // Those that read a class the table does not name -- 0 -- only where it
    // goes no further than its first words.
    const bool noneSafe = name == "clearMemClasses" || name == "memClassOverhead" || name == "memBlockOverhead"
                          || name == "releasememclass" || name == "getfreememclass" || name == "memBlockName"
                          || name == "creatememclass";
    const bool fewer = chance(50);
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(kEaMemTable + 4 * i) = i == ia ? a.header
                                    : i == ib ? b.header
                                    : noneSafe && (fewer ? chance(60) : chance(5)) ? 0
                                    : chance(50) ? a.header : b.header;
    if (name == "getfreememclass" && chance(20))
        for (x86::reg32 i = 1; i < 16; ++i)
            g.word(kEaMemTable + 4 * i) = a.header;
    const EaMemClass& c = chance(50) ? a : b;
    // A name: one of the strings, or none.
    const auto aName = [] { return chance(25) ? 0 : aString(); };
    // A size asked for: small mostly, under 8, as large as any free block or
    // larger, less than nothing.
    const auto aSize = [] {
        return chance(70) ? below(0x180) : chance(40) ? below(8) : chance(60) ? 0x100 + below(0x300)
               : chance(50) ? x86::reg32(-1 - x86::sreg32(below(0x100))) : random32();
    };

    if (name == "findFreeBlock" || name == "findLargestFreeBlock")
    {
        cpu.eax = c.header;
        cpu.edx = chance(85) ? aSize() : (random32() & 0x7fffffff);
        if (name == "findLargestFreeBlock" && cpu.edx == 0x80000000)
            cpu.edx = 0;
        cpu.ebx = chance(50) ? 0 : chance(50) ? 0x10 : random32();
    }
    else if (name == "linkFreeBlock")
    {
        cpu.eax = c.header;
        cpu.edx = chance(90) ? anEaBlock(c, 0) : chance(50) ? c.high : c.base;
    }
    else if (name == "unlinkFreeBlock")
    {
        cpu.eax = chance(90) ? c.header : random32();
        cpu.edx = chance(95) ? anEaBlock(c, 1) : c.header + 0x10;
        // A block that is not free has its body for links: the class's own blocks.
        if (!(g.getMemory<x86::reg8>(cpu.edx + 3) & 0x40) && cpu.edx != c.header + 0x10)
        {
            g.word(cpu.edx + 0x10) = c.header + 0x10;
            g.word(cpu.edx + 0x14) = c.header + 0x10;
        }
    }
    else if (name == "markMemBlock")
    {
        for (x86::reg32 at = kEaMemMarked; at < kEaMemEnd; at += 4)
            g.word(at) = random32();
        cpu.eax = kEaMemMarked + 4 * below(0x40) + (chance(90) ? 0 : below(4));
        cpu.ebx = chance(90) ? below(0x200) : below(0x400) - 0x100;
        cpu.ecx = chance(50) ? 0 : chance(80) ? 4 * below(0x10) : below(0x40);
        cpu.edx = aName();
        stack[0] = chance(70) ? (random32() & 0xff0f) | below(16) : random32();
        stack[1] = chance(80) ? anEaBlock(c, -1) : random32();
        stack[2] = chance(80) ? anEaBlock(c, -1) : random32();
    }
    else if (name == "memClassOverhead")
        cpu.eax = chance(80) ? anEaClass(a, b) : random32();
    else if (name == "memBlockOverhead")
    {
        cpu.eax = aName();
        cpu.edx = chance(80) ? anEaClass(a, b) : random32();
    }
    else if (name == "creatememclass")
    {
        static const x86::reg32 outside[] = {0xffffffff, 0x10, 0x11, 0x80000000};
        cpu.eax = chance(85) ? below(16) : chance(50) ? outside[below(4)] : random32();
        cpu.edx = aString();
        cpu.ebx = kEaMemClassA + 4 * below(0x100) + (chance(90) ? 0 : below(4));
        cpu.ecx = chance(85) ? 0x100 + below(0x1000) : below(0x100);
        static const x86::reg32 aligns[] = {1, 2, 4, 8, 0x10, 0x20, 0x40, 0x100};
        stack[0] = chance(90) ? aligns[below(8)] : random32();
        stack[1] = chance(90) ? aligns[below(8)] : 1 + below(0x100);
        stack[2] = chance(40) ? 0 : chance(80) ? 4 * below(0x10) : below(0x40);
        for (x86::reg32 i = 3; i < 7; ++i)
            stack[i] = chance(50) ? 0 : chance(80) ? 1 : random32();
        // The game's sprintf keeps edx, which the LOW block's mark takes its
        // class's bytes from; its stand-in may not (kScratch), and a name
        // copied that far on goes out of the guest's memory.  Blocks named
        // (the fifth word) only where the stand-in's first call -- the case's
        // first, sprintf's, logged as it is worked out -- keeps edx, as the
        // stand-in works it out.
        const x86::reg32 more = x86::reg32(mix(g_seed ^ (std::uint64_t(1) << 32) ^ (0x4df690u ^ 0xabcd)));
        if ((more >> 4) % 4 != 0)
            stack[5] = 0;
    }
    else if (name == "releasememclass" || name == "largestmemclass" || name == "purgememclass")
    {
        cpu.eax = anEaClass(a, b);
        if (name == "purgememclass" && chance(5))
            g.word(g.word(kEaMemTable + 4 * (cpu.eax & 0xf)) + 8) = 0;
    }
    else if (name == "largestmem")
        g.word(kEaMemDefault) = anEaClass(a, b);
    else if (name == "memBlockSize" || name == "memBlockName" || name == "memBlockFlags")
    {
        const x86::reg32 block = chance(90) ? anEaBlock(c, chance(80) ? 0 : -1) : chance(50) ? c.base : c.high;
        cpu.eax = chance(95) ? block + 0x10 : kEaMemClassA + 0x10 + below(0x1f00);
    }
    else if (name == "reservememadr")
    {
        cpu.eax = aName();
        // Not so large that what it takes wraps round: a block found for it,
        // split far out of memory.
        cpu.edx = aSize();
        if (x86::sreg32(cpu.edx) > 0x10000000)
            cpu.edx &= 0x0fffffff;
        cpu.ebx = (chance(80) ? c.index : anEaClass(a, b) & 0xf)
                  | (chance(70) ? (chance(40) ? 0x10 : 0) | (chance(30) ? 0x20 : 0) | (chance(40) ? 0x100 : 0)
                                      | (chance(20) ? 0x200 : 0) | (chance(20) ? 0x400 : 0) | (chance(20) ? 0x800 : 0)
                                : random32() & 0xbff0);
    }
    else if (name == "purgememadr")
    {
        const x86::reg32 block = chance(85) ? anEaBlock(c, 0) : chance(40) ? anEaBlock(c, 1)
                                 : chance(50) ? (chance(50) ? c.base : c.high) : kEaMemClassA + 0x10 + below(0x1f00);
        cpu.eax = chance(95) ? block + 0x10 : 0;
    }
    else if (name == "reservememStdcall")
        stack[0] = aSize();
    else if (name == "purgememStdcall")
        stack[0] = chance(90) ? anEaBlock(c, 0) + 0x10 : random32();
    return true;
}
#endif
