// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static constexpr x86::reg32 kBlit = 0x7e0300, kConverter = 0x7e0310, kQueueTest = 0x7e0320, kKeyHandler = 0x7e0330;
    constexpr x86::reg32 kRows = 0xbf0000, kColumns = 0xbf4000, kPixels = 0xc00000;
    static constexpr x86::reg32 kClut = 0xb84000;
    constexpr x86::reg32 kShapes = 0xb80000, kPalette = 0xb88000, kRowSpace = 0xba0000;
    constexpr x86::reg32 kScratch = 0xb00000, kFontTable = 0xbc0000, kLinks = 0xbd0000;
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x50dc20].kind = Kind::Bool;
        specs[0x4f90e0].kind = Kind::Bool;
        specs[0x4fd4e0] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        // The palette in use, below the converters' tables: what a type that is none finds there.
        for (const x86::reg32 clut : { 0xa0a814u, 0xa0ac14u, kClut + 0x10 })
            specs[clut] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[kBlit] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[kConverter] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[kQueueTest] = { EAX | EBX | EDX, 0, 0, EDX, Kind::Bool };
        specs[kKeyHandler] = { EAX | EBX | ECX | EDX | ESI | EDI, 0, 0, EBX | ECX, Kind::Plain };
        specs[word32(0x5344b8)] = { 0, 1, 4, 0, Kind::NoEax };
        specs[word32(0x534588)] = { 0, 1, 4, 0, Kind::NoEax };
        g_methods[0x401010] = &nfs3hp::watbStandInLog;
        g_methods[0x4f7900] = &nfs3hp::watbStandInMalloc;
        g_methods[0x4e1620] = &nfs3hp::watbStandInReserve;
        g_methods[0x5236c0] = &nfs3hp::watbStandInStride;
        g_methods[0x52448c] = &nfs3hp::watbStandInClass;
        g_methods[kConverter] = &nfs3hp::watbStandInConverter;
        g_methods[0x5042c4] = &nfs3hp::watbStandInSwapRedBlue;
        g_methods[0x51df60] = &nfs3hp::watbStandInExpand24;
        g_methods[0x51de60] = &nfs3hp::watbStandInExpand18;
        return true;
    }();
    (void)standInsAdded;

    // The tables they read, as the exe has them; the converters' are stand-ins.
    const auto fromImage = [&](x86::reg32 at, std::size_t bytes) {
        std::memcpy(g_memory + at, g_image.data() + (at - 0x534000), bytes);
    };
    fromImage(0x566d54, 0x80);
    if (random(4) != 0)
        fromImage(0x568594, 0x40);
    for (x86::reg32 at = 0x568394 - 0x40; at < 0x568594; at += 4)
        setWord32(at, kConverter);

    // A window: its clip inside its tables, a depth a table of blitters has.
    static const x86::reg8 kDepths[] = { 4, 8, 15, 16, 24, 32, 15, 16, 16, 8 };
    static const x86::reg32 kTypes[] = { 0x40, 0x41, 0x42, 0x43, 0x6d, 0x78, 0x7a, 0x7b, 0x7d, 0x7e, 0x7f };
    const x86::reg32 clipLeft = random(8), clipTop = random(8);
    setWord32(0x565000, clipLeft);
    setWord32(0x565004, clipTop);
    setWord32(0x565008, clipLeft + 20 + random(40));
    setWord32(0x56500c, clipTop + 20 + random(40));
    const x86::reg8 depth = kDepths[random(std::size(kDepths))];
    g_memory[0x565010] = depth;
    g_memory[0x565011] = x86::reg8(kTypes[random(std::size(kTypes))]);
    g_memory[0x565012] = x86::reg8(random(5) == 0);
    setWord32(0x565014, kPixels);
    setWord32(0x56501c, random(2) ? 0x400 : 0x40 * ((depth + 1u) & ~1u) / 8);
    setWord32(0x565020, kRows);
    setWord32(0x565024, kColumns);
    setWord32(0x565028, random(3) == 0);
    const x86::reg32 pixelBytes = (depth + 1u) / 8 ? (depth + 1u) / 8 : 1;
    for (x86::reg32 i = 0; i < 128; ++i)
    {
        setWord32(kRows + 4 * i, i * word32(0x56501c));
        setWord32(kColumns + 4 * i, i * pixelBytes);
    }
    fillRandom(kPixels, kPixels + 0x20000);

    // Shapes, each of a type the converters know, and a palette some of them lead to.
    static const x86::reg32 kClutTypes[] = { 0x22, 0x23, 0x24, 0x29, 0x2a, 0x2c, 0x2d, 0x2e, 0x30 };
    for (x86::reg32 i = 0; i < 6; ++i)
    {
        const x86::reg32 shape = kShapes + i * 0x400;
        fillRandom(shape, shape + 0x400);
        const x86::reg32 next = random(3) == 0 ? 0 : kClut - shape;
        setWord32(shape, kTypes[random(std::size(kTypes))] | (next << 8));
        g_memory[shape + 4] = x86::reg8(1 + random(12));
        g_memory[shape + 5] = 0;
        g_memory[shape + 6] = x86::reg8(1 + random(12));
        g_memory[shape + 7] = 0;
    }
    fillRandom(kClut, kClut + 0x800);
    setWord32(kClut, kClutTypes[random(std::size(kClutTypes))]);
    g_memory[kClut + 4] = x86::reg8(1 + random(16));
    g_memory[kClut + 5] = 0;

    // Palettes and spans.
    setWord32(0x568390, random(2) ? 0xa0a814 : kPalette);
    fillRandom(kPalette, kPalette + 0x1000);
    fillRandom(kScratch, kScratch + 0x6000);
    setWord32(0x5650c0, 0);
    setWord32(0x9f09b0, 0xa0a814);
    setWord32(0xa0b020, kPalette);
    setWord32(0xa0b018, kConverter);
    setWord32(0xa0b014, kBlit);
    setWord32(0x5643ec, 1 + random(40));
    setWord32(0x5643f4, random(4));

    // The blitters' tables.
    for (const x86::reg32 table : { 0x5676b4u, 0x5688a8u })
    {
        for (x86::reg32 i = 0; i < 6; ++i)
            setWord32(table + 4 * i, kBlit);
        setWord32(table + 0x18, random(3) == 0 ? kBlit : 0);
        setWord32(table + 0x1c, random(3));
        setWord32(table + 0x20, random(3) == 0 ? kTypes[random(std::size(kTypes))] : 0);
    }
    setWord32(0x5676b0, random(2));

    // The row space: runs of a count, the uses, the offsets.
    x86::reg32 run = kRowSpace;
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 size = 2 + random(24);
        setWord32(run, size);
        setWord32(run + 4, random(2) ? 0 : 1 + random(2));
        for (x86::reg32 k = 0; k < size; ++k)
            setWord32(run + 8 + 4 * k, random(40));
        run += (size + 2) * 4;
    }
    setWord32(0x568380, kRowSpace);
    setWord32(0xa0a810, run);
    setWord32(0x56837c, random(3) ? 0x100 + random(16) : 0x40);
    setWord32(0x56438c, random(2) ? 0 : 16);

    // The font's.
    setWord32(0x567280, kFontTable);
    fillRandom(kFontTable, kFontTable + 0x1100);
    for (x86::reg32 k = 0; k < 16; ++k)
    {
        static const x86::reg8 kAlphas[] = { 0, 0x08, 0x20, 0x80, 0xfb, 0xfc, 0xff };
        g_memory[kFontTable + 4 * k + 3] = kAlphas[random(std::size(kAlphas))];
    }
    setWord32(0x567220, random(4) == 0 ? random(0x300) : random(256));
    setWord32(0x567208, random(5));
    setWord32(0x56720c, random(2));
    setWord32(0x5672a4, random(2));

    // The keys'.
    setWord32(0x56879a, random(4) == 0 ? 2 : 1);
    for (x86::reg32 i = 0; i < 0x85 + 0x80; ++i)
        g_memory[0x568694 + i] = x86::reg8(random(3) == 0 ? 1 + random(4) : 0);
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x56879e + 4 * i, kKeyHandler);

    // Streams: the open ones' links, the closed ones', __iob in use.
    const x86::reg32 open = random(5);
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 link = kLinks + 0x40 * i;
        setWord32(link, i + 1 < open ? link + 0x40 : 0);
        const x86::reg32 file = random(2) ? 0x566f58 + 0x1a * random(6) : kLinks + 0x800 + 0x20 * i;
        setWord32(link + 4, file);
        g_memory[file + 0xd] = x86::reg8(random(3) == 0 ? random(256) : 0);
    }
    setWord32(0x9f38a0, open != 0 ? kLinks : 0);
    const x86::reg32 closed = random(4);
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(kLinks + 0x400 + 0x40 * i, i + 1 < closed ? kLinks + 0x400 + 0x40 * (i + 1) : 0);
    setWord32(0x9f38a4, closed != 0 ? kLinks + 0x400 : 0);
    const x86::reg32 inUse = random(6);
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(0x566f58 + 0x1a * i + 0xc, i < inUse ? 1 + random(100) : 0);

    // The command line.
    static const char kLetters[] = "  \t\"\\\\ab";
    const x86::reg32 line = kLinks + 0x1000;
    const x86::reg32 length = random(28);
    g_memory[line - 1] = random(2) ? '\\' : 'x';
    for (x86::reg32 i = 0; i < length; ++i)
        g_memory[line + i] = x86::reg8(kLetters[random(8)]);
    g_memory[line + length] = 0;
    setWord32(0x5677fc, line);
    setWord32(0x567800, random(0));
    setWord32(0xa18218, random(3) == 0);

    // The start-up's.
    g_memory[0x5644c0] = x86::reg8(random(3) == 0 ? 1 + random(3) : 0);
    g_memory[0x5644c1] = x86::reg8(random(2));
    g_memory[0x567834] = x86::reg8(random(3) == 0);
}
