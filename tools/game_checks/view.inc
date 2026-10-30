// game_checks.cpp's parts for the pan-and-zoom view, rewritten
// (src/nfs3hp/port_view.cpp, port::view), included in each table with the
// macro naming the part.  The functions compute in the host's doubles, rounded
// to a float where the game stores one; the x87 of the checks is therefore at
// the extended precision (the build's default), any rounding mode.
#if defined(VIEW_WINDOWS)
// {from, to}, -- what the functions write, compared (and the world made for the cases)
// The records, the tracks' data and the sections the view's cases make: 0x30f400 to 0x322000; the sections' count.
{0x30e000, 0x322000}, {0x5dd000, 0x5de000}, {0x7000, 0x9000}, {0x1f0000, 0x240000}, {0x800000, 0x880000},
#elif defined(VIEW_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kViewEntry,     // a participant's entry: the scratch one, or none
kViewColours,   // how many colours a car has: none to eight
#elif defined(VIEW_EFFECT_CASES)
// case kSomething: ... break;
case kViewEntry:
    result = r % 6 == 0 ? 0 : 0x310000;
    break;
case kViewColours:
    result = r % 11 == 0 ? 0 : 1 + (r >> 8) % 8;
    break;
#elif defined(VIEW_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// What the view's popup calls that is not held on its own: a participant's
// entry, a metric written, the view prepared (the 2D drawing, the text,
// sprintf and the natives -- the pointer in a box, the render state -- are
// the real ones on both sides, over the worlds of the loop and the 2D drawing).
{0x455cd0, "a participant's entry", kWatcom, 4, 0, kEax, -1, 0, kViewEntry},
{0x4cedd0, "a metric written", kStdcall, 0, 2, kEax, -1, 0, kNothing},
{0x464eb0, "the view prepared", kWatcom, 4, 0, kEax | kEcx | kEdx, -1, 0, kNothing},
{0x464840, "the grid drawn", kStdcall, 4, 3, kEax, -1, 0, kNothing},
{0x474db0, "the edges' triangles", kStdcall, 4, 8, kEax, -1, 0, kNothing},
{0x439760, "a car's colours", kWatcom, 1, 0, kEax | kEbx | kEcx, -1, 0, kViewColours},
#elif defined(VIEW_PROMISES)
// {"port::view::name", watcom, result, arguments, fpu, heap, liveOnly},
// eax and the registers the port keeps; the x87's stack, its live registers
// and its words where a float goes up on it.  Not where the game's code only
// compares (panKey): it leaves the status word of its last compare, which no
// caller reads (each compare is followed by its own fnstsw), and the port's
// code has no compare to leave it.
{"port::view::storeIncremented", true, true, 0},
{"port::view::glyphResource", true, true, 2},
{"port::view::edgeTriangles", true, false, 4},
{"port::view::colourNearest", true, false, 2},
{"port::view::sectionAheadIsTwo", true, true, 1},
{"port::view::gridDrawn", true, false, 4},
{"port::view::viewFrame", true, true, 2},
{"port::view::panKey", true, true, 3},
{"port::view::metricPopup", true, false, 1},
{"port::view::recordFloat", true, true, 1, true, kNoHeap, true},
{"port::view::recordFloatAt", true, true, 2, true, kNoHeap, true},
{"port::view::recordFloatByKind", true, true, 2, true, kNoHeap, true},
#elif defined(VIEW_ARGUMENTS)
/* Where the view's cases keep their records: a participant's, its kind and
 * its floats, in the menus' scratch above the dialogs'. */
constexpr x86::reg32 kViewRecord = 0x30f400;
constexpr x86::reg32 kViewEntryAt = 0x310000;      // the participant's entry the popup is given

/* The x87 as a view case finds it: the extended precision (what the build
 * computes in doubles), any rounding, a few values on the stack. */
void makeViewX87(x86::CPU& cpu, x86::reg32 most)
{
    cpu.fpu.setControl(x86::reg16(0x37f | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(most);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
}

/* A float of the view's: a place on the screen mostly, any bits now and then. */
x86::reg32 aViewFloat()
{
    if (chance(15))
        return anyFloatBits((std::uint64_t(random32()) << 32) | random32());
    const float f = float(int(below(4000)) - 2000) / (chance(50) ? 1.0f : 8.0f);
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

void makeViewWorld(CheckApp& g, x86::CPU& cpu)
{
    for (x86::reg32 at = kViewRecord; at < kViewRecord + 0x100; at += 4)
        g.word(at) = chance(90) ? aViewFloat() : random32();
    // The record's kind (a word at +0x42): mostly one of the six, now and then past them.
    static const x86::reg32 kinds[] = {0, 1, 2, 3, 4, 5, 6, 0xffff};
    g.getMemory<x86::reg16>(kViewRecord + 0x42) = x86::reg16(chance(92) ? kinds[below(8)] : random32());
    makeViewX87(cpu, 6);
}

bool makeViewArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* a)
{
    makeViewWorld(g, cpu);
    if (name == "storeIncremented")
    {
        cpu.eax = random32();
        a[0] = aViewFloat();
    }
    else if (name == "panKey")
    {
        static const x86::reg32 keys[] = {0x4b00, 0x4d00, 0x20, 0x20, 0x1c0d, 0};
        cpu.eax = random32();
        cpu.edx = random32();
        cpu.ebx = (chance(30) ? random32() & 0xffff0000 : 0) | (chance(90) ? keys[below(6)] : random32() & 0xffff);
        // Whether the view is up, and where it is panned to: near its edge now and then.
        g.word(0x678950) = chance(85) ? 1 : chance(50) ? 0 : random32();
        const auto aPan = [] {
            if (chance(20))
                return aViewFloat();
            const float f = float(int(below(61)) - 30) / (chance(50) ? 1.0f : 4.0f);
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        g.word(0x678954) = aPan();
        g.word(0x678958) = aPan();
    }
    else if (name == "metricPopup")
    {
        // The popup: the language's text (the loop's world), a record whose box is stepped
        // down a few rows, and the entries' metrics.
        makeLoopWorld(g, cpu);
        makeViewWorld(g, cpu);
        cpu.eax = kViewRecord;
        for (const x86::reg32 off : {0x8u, 0x3cu, 0x3eu, 0x40u, 0x8au})
            g.getMemory<x86::reg16>(kViewRecord + off) = x86::reg16(below(60));
        g.getMemory<x86::reg16>(kViewRecord + 0x42) = x86::reg16(20 + below(80));
        g.word(0x55e5e4) = below(260);
        g.word(0x55e5e8) = below(260);
        g.getMemory<x86::reg16>(kViewRecord + 0x44) = x86::reg16(chance(90) ? 5 + below(35) : random32());
        g.getMemory<x86::reg16>(kViewRecord + 0x46) = x86::reg16(chance(90) ? below(7) : random32());
        g.getMemory<x86::reg16>(kViewRecord + 0x84) = x86::reg16(chance(90) ? below(6) : random32());
        g.word(0x6fd3b8) = chance(10) ? 3 : below(3);
        g.word(0x6fd4b4) = chance(90) ? below(8) : below(13);
        for (x86::reg32 j = 0; j < 12; ++j)
        {
            static const float values[] = {0.0f, -1.0f, 1.0f, 2.5f, 7.0f, 7.0f, 120.25f, -0.0f};
            float f = chance(15) ? float(int(below(9)) - 4) : values[below(8)];
            if (chance(3))
                f = NAN;
            std::memcpy(&g.getMemory<char>(kViewEntryAt + 0x298 + 4 * j), &f, 4);
        }
        g.word(kViewEntryAt + 0x2c4) = chance(15) ? 4 : random32();
    }
    else if (name == "glyphResource")
    {
        // The font's name (a string), the flags; the font's file or the cache file is the stand-in's
        // 0x77x0 -- a pattern of small bytes with the period of its steps, so that any of them parses
        // as a font of a few small glyphs, or as a baked resource of a few records.
        makeFeDrawWorld(g, cpu);
        makeViewX87(cpu, 4);
        cpu.eax = 0x30f800;
        cpu.edx = random32();
        std::memcpy(&g.getMemory<char>(0x30f800), "glyphs", 7);
        static const x86::reg8 pattern[16] = {1, 2, 2, 1, 3, 1, 2, 2, 1, 3, 2, 1, 2, 1, 3, 2};
        for (x86::reg32 i = 0; i < 0x1000; ++i)
            g.getMemory<x86::reg8>(0x7700 + i) = pattern[(i + (chance(1) ? 1 : 0)) % 16];
        g.word(0x679010) = random32();
        g.word(0x30e000) = 0x57454956;        // game_checks.cpp's allocate hands out blocks that follow one another
        g.word(0x30e004) = 0x800000;
    }
    else if (name == "edgeTriangles")
    {
        // Four offsets within a face (0 to 2); the flags' grid, the triangle list, the vertices, the normals,
        // the width, the second face's number and the first's, the counter -- by their addresses.
        const x86::reg32 width = 1 + below(20);
        cpu.eax = below(3);
        cpu.edx = below(3);
        cpu.ebx = below(3);
        cpu.ecx = below(3);
        a[0] = 0x31a000;
        a[1] = 0x31b000;
        a[2] = 0x31c000;
        a[3] = 0x31d000;
        a[4] = width;
        a[5] = below(6);
        a[6] = below(6);
        a[7] = 0x31e000;
        g.word(0x31e000) = 4 + below(40);
        for (x86::reg32 i = 0; i < 0x400; ++i)
            g.getMemory<x86::reg8>(0x31a000 + i) = x86::reg8(chance(60) ? 1 : 0);
        for (x86::reg32 i = 0; i < 0x400; ++i)
            g.getMemory<x86::reg8>(0x31b000 + i) = x86::reg8(below(std::min<x86::reg32>(width, 32)));
        for (x86::reg32 i = 0; i < 12 * 32; i += 4)
        {
            const float f = float(int(below(2000)) - 1000) / 8.0f;
            std::memcpy(&g.getMemory<char>(0x31c000 + i), &f, 4);
        }
    }
    else if (name == "colourNearest")
    {
        // The object (the car at +0x14), the target colour; the colours' bytes are the stack's own.
        cpu.eax = kViewRecord;
        cpu.edx = random32();
        g.word(kViewRecord + 0x14) = random32();
    }
    else if (name == "sectionAheadIsTwo")
    {
        // The record's position (floats at +8, +0xc, +0x10), its track (at +4), the sections.  The
        // x87 at the extended precision and the nearest rounding, as the game runs it.
        cpu.fpu.setControl(0x37f);
        cpu.eax = kViewRecord;
        g.word(kViewRecord + 4) = chance(95) ? below(3) : below(5);
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            const float f = chance(90) ? float(int(below(60000)) - 30000) / 8.0f : float(int(below(4000000)) - 2000000);
            std::memcpy(&g.getMemory<char>(kViewRecord + 8 + 4 * i), &f, 4);
        }
        const x86::reg32 count = 1 + below(chance(90) ? 20 : 60);
        g.word(0x5dd940) = count;
        g.word(0x5dd948) = 0x320000;
        for (x86::reg32 i = 0; i < 5; ++i)
        {
            const x86::reg32 track = 0x311000 + 0x1000 * i;
            g.word(0x5efa48 + 4 * i) = track;
            g.word(track + 0x1c) = chance(95) ? below(count) : random32() % 200;
            g.word(track + 0x998) = chance(50) ? 0 : 1;
        }
        for (x86::reg32 at = 0x320000; at < 0x320000 + 36 * 120; at += 4)
            g.word(at) = chance(70) ? random32() : below(8000) - 4000;
        for (x86::reg32 i = 0; i < 120; ++i)
        {
            // The kind byte: nibbles of 2, 0 and others.
            static const x86::reg8 nibbles[] = {0, 2, 2, 1, 3, 15};
            g.getMemory<x86::reg8>(0x320000 + 36 * i + 0xe) = x86::reg8(nibbles[below(6)] | nibbles[below(6)] << 4);
        }
    }
    else if (name == "gridDrawn")
    {
        // The clip is set by the game's own code on both sides: the 2D drawing's world, its THRASH slots.
        makeFeDrawWorld(g, cpu);
        makeViewX87(cpu, 4);
        // The box, the map's offset and the zoom (by their addresses, on the stack), the clip's limits
        // as float bits, the viewport, the tiles' textures.
        cpu.eax = chance(90) ? below(320) : random32() % 700;
        cpu.edx = chance(90) ? below(240) : random32() % 500;
        cpu.ebx = chance(85) ? 0x140 : chance(50) ? below(700) : random32();
        cpu.ecx = chance(85) ? 0xc0 : chance(50) ? below(500) : random32();
        a[0] = 0x55a43c;
        a[1] = 0x55a440;
        a[2] = 0x55a438;
        g.word(0x55a43c) = chance(90) ? below(6000) - 3000 : random32();
        g.word(0x55a440) = chance(90) ? below(600) - 300 : random32();
        {
            const float scale = chance(5) ? NAN : chance(10) ? 0.5f : float(1 + below(300)) / 100.0f;
            std::memcpy(&g.getMemory<char>(0x55a438), &scale, 4);
        }
        g.getMemory<x86::reg8>(0x7a3a58) = x86::reg8(random32());
        const float limits[] = {512.0f, 0.0f, 480.0f, 0.0f, 640.0f};
        const x86::reg32 at[] = {0x7d34f8, 0x7d34fc, 0x7d3500, 0x7d3504, 0x7d350c};
        for (int i = 0; i < 5; ++i)
        {
            float f = limits[i] + (chance(30) ? float(int(below(200)) - 100) : 0.0f);
            std::memcpy(&g.getMemory<char>(at[i]), &f, 4);
        }
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x7cda70 + 4 * i) = below(chance(80) ? 700 : 100000);
        for (x86::reg32 i = 0; i < 5; ++i)
            g.word(0x678960 + 44 * i) = random32();
    }
    else if (name == "viewFrame")
    {
        makeFeDrawWorld(g, cpu);
        makeViewX87(cpu, 4);
        // The object (its position at +6 and +8, a flag byte at +5), the mouse and the pan.
        cpu.eax = kViewRecord;
        cpu.edx = random32();
        g.getMemory<x86::reg16>(kViewRecord + 6) = x86::reg16(chance(90) ? below(400) : random32());
        g.getMemory<x86::reg16>(kViewRecord + 8) = x86::reg16(chance(90) ? below(300) : random32());
        g.getMemory<x86::reg8>(kViewRecord + 5) = x86::reg8(random32());
        g.word(0x55a43c) = chance(90) ? below(2000) - 1000 : random32();
        g.word(0x55a440) = chance(90) ? below(2000) - 1000 : random32();
        g.word(0x678950) = random32();
        g.word(0x67894c) = chance(50) ? 0 : chance(80) ? 1 : random32();
        g.word(0x678948) = chance(90) ? below(640) : random32();
        g.word(0x678944) = chance(90) ? below(480) : random32();
        g.word(0x55e5e4) = chance(90) ? below(640) : random32();
        g.word(0x55e5e8) = chance(90) ? below(480) : random32();
        g.word(0x678954) = aViewFloat();
        g.word(0x678958) = aViewFloat();
        g.getMemory<x86::reg8>(0x7a3a58) = x86::reg8(random32());
    }
    else if (name == "recordFloat")
        cpu.eax = kViewRecord;
    else if (name == "recordFloatAt" || name == "recordFloatByKind")
    {
        cpu.eax = kViewRecord;
        cpu.edx = chance(90) ? below(40) : chance(50) ? x86::reg32(-int(1 + below(4))) : below(60);
    }
    return true;
}
#endif
