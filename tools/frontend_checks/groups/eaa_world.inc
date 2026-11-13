// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kTableAt = 0xa00000, kVtables = 0xa00100, kDirectDraw = 0xa00500, kSurfaceA = 0xa00510;
    constexpr x86::reg32 kSurfaceB = 0xa00520, kPaletteObject = 0xa00530, kClipperObject = 0xa00540;
    constexpr x86::reg32 kPool = 0xa00600, kBlockList = 0xa01000, kBlocks = 0xa02000, kNames = 0xa04000;
    constexpr x86::reg32 kOtherDisplay = 0xa05000, kModesList = 0xa06000, kBitmapHere = 0xa03200;
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // The console: Watcom's registers, every register kept.
        specs[0x7e0400] = { 0, 0, 0, 0, Kind::Plain };                     // begin
        specs[0x7e0404] = { 0, 0, 0, 0, Kind::Plain };                     // end
        specs[0x7e0408] = { EAX, 0, 0, 0, Kind::Plain };                   // a metric
        specs[0x7e040c] = { 0, 2, 0, 0, Kind::Plain };                     // print
        specs[0x7e0414] = { EAX | EDX | EBX | ECX, 0, 0, 0, Kind::Plain }; // draw a shape
        specs[0x7e0418] = { 0, 0, 0, 0, Kind::Plain };                     // a key
        specs[0x7e041c] = { 0, 0, 0, 0, Kind::Plain };                     // init
        specs[0x7e0420] = { EAX | EDX | EBX | ECX, 1, 4, 0, Kind::Plain }; // clear
        specs[0x7e0424] = { EAX | EDX, 0, 0, 0, Kind::Plain };             // colours
        g_methods[0x7e040c] = &nfs3hp::eaaConsolePrint;
        g_methods[0x7e0418] = &nfs3hp::eaaKey;
        // A menu entry's function, the drawing callback, the display's.
        specs[0x7e0480] = { EAX, 0, 0, 0, Kind::Bool };                    // may fail: the stand-in bitmap's path
        specs[0x7e0484] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x7e0488] = { 0, 0, 0, 0, Kind::Plain };
        // COM: stdcall, ebx, esi and edi kept.
        const auto com = [&](x86::reg32 table, x86::reg32 slot, std::uint8_t words, Kind kind) {
            specs[0x7e0500 + table * 0x100 + slot] = { 0, words, std::uint8_t(words * 4), ECX | EDX, kind };
        };
        com(1, 0x00, 3, Kind::Plain);  // DirectDraw: QueryInterface
        com(1, 0x08, 1, Kind::Plain);  // Release
        com(1, 0x10, 4, Kind::Plain);  // CreateClipper
        com(1, 0x14, 5, Kind::Plain);  // CreatePalette
        com(1, 0x20, 5, Kind::Bool);   // EnumDisplayModes
        com(1, 0x28, 1, Kind::Plain);  // FlipToGDISurface
        com(1, 0x2c, 3, Kind::Plain);  // GetCaps
        com(1, 0x50, 3, Kind::Bool);   // SetCooperativeLevel
        com(2, 0x58, 2, Kind::Plain);  // a surface: GetSurfaceDesc
        com(2, 0x60, 1, Kind::Bool);   // IsLost
        com(2, 0x64, 5, Kind::Plain);  // Lock
        com(2, 0x6c, 1, Kind::Plain);  // Restore
        com(2, 0x70, 2, Kind::Plain);  // SetClipper
        com(2, 0x7c, 2, Kind::Plain);  // SetPalette
        com(3, 0x18, 5, Kind::Plain);  // a palette: SetEntries
        com(4, 0x20, 3, Kind::Plain);  // a clipper: SetHWnd
        g_methods[0x7e0600] = &nfs3hp::eaaQueryInterface;
        g_methods[0x7e0610] = &nfs3hp::eaaCreateClipper;
        g_methods[0x7e062c] = &nfs3hp::eaaGetCaps;
        g_methods[0x7e0764] = &nfs3hp::eaaLock;
        // Windows' critical sections and the window's call, through their slots.
        specs[word32(0x5344b8)] = { 0, 1, 4, ECX | EDX, Kind::NoEax };
        specs[word32(0x534588)] = { 0, 1, 4, ECX | EDX, Kind::NoEax };
        specs[word32(0x534788)] = { 0, 1, 4, ECX | EDX, Kind::Plain };
        // What frees and makes surfaces goes through COM, which keeps these; the callers read them after.
        specs[0x4ed730] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4ed770] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4ed2a0] = { EAX, 0, 0, 0, Kind::Plain };
        g_methods[0x4ed770] = &nfs3hp::eaaBackSurface;
        // What is read through.
        specs[0x4f47c0].kind = Kind::Focus;   // a block's owner's kind: 0 to 2
        specs[0x4efa70].kind = Kind::Shape;   // a glyph, or none
        specs[0x4e1620].kind = Kind::Shape;   // memory, or none
        specs[0x4e19d0].kind = Kind::Shape;
        specs[0x4ef190].kind = Kind::Shape;   // a bitmap
        specs[0x508c70].kind = Kind::Shape;   // a window
        specs[0x4f4a80].kind = Kind::Bool;
        specs[0x4ed0f0].kind = Kind::Bool;    // the bitmap could not be had, or could
        specs[0x4e0e60].kind = Kind::Bool;    // a file exists
        specs[0x4edfe8].kind = Kind::Bool;    // fopen
        specs[0x4e2de0].kind = Kind::Bool;    // read
        specs[0x4f9120].kind = Kind::Bool;
        g_methods[0x4df690] = &nfs3hp::eaaSprintf;
        g_methods[0x501050] = &nfs3hp::eaaReport;
        g_methods[0x4f4b80] = &nfs3hp::eaaFprintf;
        g_methods[0x4f46b0] = &nfs3hp::eaaBlockArgument;
        g_methods[0x4e15e0] = &nfs3hp::eaaBlockName;
        g_methods[0x4ea654] = &nfs3hp::eaaPackCount;
        g_methods[0x4ed480] = &nfs3hp::eaaMakeSurface;
        g_methods[0x533e86] = &nfs3hp::eaaDirectDrawCreate;
        return true;
    }();
    (void)standInsAdded;
    nfs3hp::eaaCapsClear = false;

    // The console's table.
    setWord32(0x79f5a4, kTableAt);
    for (x86::reg32 slot = 0; slot < 0x28; slot += 4)
        setWord32(kTableAt + slot, 0x7e0400 + slot);

    // The COM objects: a table of stand-ins each.
    for (x86::reg32 table = 1; table <= 4; ++table)
        for (x86::reg32 slot = 0; slot < 0x100; slot += 4)
            setWord32(kVtables + (table - 1) * 0x100 + slot, 0x7e0500 + table * 0x100 + slot);
    setWord32(kDirectDraw, kVtables);
    setWord32(kSurfaceA, kVtables + 0x100);
    setWord32(kSurfaceB, kVtables + 0x100);
    setWord32(kPaletteObject, kVtables + 0x200);
    setWord32(kClipperObject, kVtables + 0x300);

    // Names of the lengths the status line cuts.
    static const char* const kBlockNames[] = { "A", "TRACK.BIN", "A_LONG_FILE_NAME_OF_A_BLOCK", "" };
    for (x86::reg32 i = 0; i < 4; ++i)
        std::strcpy(reinterpret_cast<char*>(g_memory + kNames + i * 0x40), kBlockNames[i]);
    std::strcpy(reinterpret_cast<char*>(g_memory + 0xa03100), "MEMMAP??.TXT");

    // What bits a pixel a shape's type has.
    for (x86::reg32 type = 0; type < 0x80; ++type)
    {
        static const std::uint8_t kBits[] = { 0, 4, 8, 16, 24, 32 };
        g_memory[0x566d54 + type] = kBits[random(6)];
    }

    // The blocks of the pools, and the table of them.
    constexpr x86::reg32 kBlockSize = 0x80, kBlockTotal = 24;
    for (x86::reg32 i = 0; i < kBlockTotal; ++i)
    {
        const x86::reg32 block = kBlocks + i * kBlockSize;
        const x86::reg32 data = block + 0x10;
        const std::uint16_t tag = random(3) == 0 ? 0x4246 : random(4) == 0 ? 0x1234 : 0x424d;
        std::memcpy(g_memory + block, &tag, 2);
        const x86::reg32 flags = random(4) == 0 ? random(0) : random(0) & 0x0fff;
        std::memcpy(g_memory + block + 2, &flags, 2);
        setWord32(block + 4, random(5) == 0 ? 0 : random(0x1000));
        setWord32(block + 8, i + 1 == kBlockTotal ? 0 : block + kBlockSize);
        switch (i % 5)
        {
        case 0:
            std::memcpy(g_memory + data, random(2) ? "SHPI" : "SHPF", 4);
            break;
        case 1:
            setWord32(data, 0x57494e44);  // a window
            setWord32(data + 0x20, kBlocks + random(kBlockTotal) * kBlockSize + 0x10);
            break;
        case 2:
        {
            // A bitmap that is the block's size.
            const x86::reg32 w = random(6), h = random(6);
            const x86::reg32 type = random(0x80);
            setWord32(data, type | (random(0) & 0xffffff00));
            std::memcpy(g_memory + data + 4, &w, 2);
            std::memcpy(g_memory + data + 6, &h, 2);
            if (random(4) != 0)
                setWord32(block + 4, w * h * (g_memory[0x566d54 + type] >> 3) + 0x10);
            break;
        }
        default:
            break;
        }
    }
    for (x86::reg32 i = 0; i < 0x80; ++i)
    {
        setWord32(kBlockList + i * 8, kBlocks + random(kBlockTotal) * kBlockSize);
        setWord32(kBlockList + i * 8 + 4, random(0));
    }
    std::strcpy(reinterpret_cast<char*>(g_memory + kPool), "POOL");
    setWord32(kPool + 8, kBlocks);
    setWord32(kPool + 0xc, 0xa03000);
    std::strcpy(reinterpret_cast<char*>(g_memory + kPool + 0x20), "OTHER");
    setWord32(kPool + 0x28, kBlocks + 0x80 * random(kBlockTotal));
    setWord32(kPool + 0x2c, 0xa03000);
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord32(0x9ef5c0 + i * 4, random(4) == 0 ? (random(2) ? kPool : kPool + 0x20) : 0);
    const x86::reg32 poolShown = random(16);
    setWord32(0x9ef5c0 + poolShown * 4, random(2) ? kPool : kPool + 0x20);
    setWord32(0x55e55c, poolShown);

    // The viewer's state: a list and a screen of sensible size.
    setWord32(0x79f59c, 3 + random(25));
    setWord32(0x79f5c0, random(4) == 0 ? 0x40 : 0x46 + random(0x20));
    setWord32(0x79f5a0, random(3) == 0 ? 0 : kBlocks + random(kBlockTotal) * kBlockSize);
    setWord32(0x79f5a8, random(2));
    setWord32(0x79f5ac, random(24));
    setWord32(0x79f5b0, 2 + random(8));
    setWord32(0x79f5b4, random(2));
    setWord32(0x79f5b8, random(5));
    setWord32(0x79f5bc, random(5));
    setWord32(0x79f5c4, kBlockList);
    setWord32(0x79f5c8, random(6));
    setWord32(0x79f5cc, random(8));
    setWord32(0x79f5d0, random(16));
    setWord32(0x79f5d4, random(16));
    setWord32(0x79f5d8, random(16));
    setWord32(0x79f5dc, random(4) == 0 ? 0 : kBlockList);
    setWord32(0x9f25a0, random(2));
    setWord32(0x55e558, random(16));
    setWord32(0x55e560, random(2) ? 0 : random(5));
    setWord32(0x55e564, random(60));
    setWord32(0x55e568, random(5) == 0 ? random(3) : 2 + random(5));
    for (x86::reg32 i = 0; i < 9; ++i)
        setWord32(0x55e56c + i * 4, random(20));
    setWord32(0x55e590, random(2));
    setWord32(0x55e594, random(2));
    setWord32(0x55e598, random(60));
    setWord32(0x55e59c, random(30));
    setWord32(0x55e5a0, random(2));
    setWord32(0x55e5a4, random(10) == 0 ? 7 + random(4) : random(7));
    setWord32(0x55e5a8, random(2));
    setWord32(0x55e5ac, random(2));
    setWord32(0x55e5b0, random(2));

    // The menu of what to do with a block.
    setWord32(0x5662e0, random(4));
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        std::memcpy(g_memory + 0x79f524 + i * 12, random(2) ? "VIEW\0\0\0\0" : "HEX DUMP", 8);
        setWord32(0x79f524 + i * 12 + 8, 0x7e0488);
    }

    // The shapes Kind::Shape hands out: every kind of type.
    for (x86::reg32 k = 0; k < 8; ++k)
    {
        static const std::uint8_t kTypes[] = { 0x7c, 0x6e, 0x6f, 0x22, 0x2a, 0x01, 0x82, 0x03 };
        g_memory[0x5f3e00 + 4 * k] = kTypes[random(8)];
    }

    // Fonts.
    setWord32(0x7cdac8, random(2) ? 640 : 320);
    g_memory[0x7a29b4 + random(8)] = 0;

    // Drawing surfaces: a small one.
    setWord32(0x565000, random(4));
    setWord32(0x565004, random(4));
    setWord32(0x565008, word32(0x565000) + random(5));
    setWord32(0x56500c, word32(0x565004) + random(5));
    {
        static const std::uint8_t kBits[] = { 1, 4, 8, 15, 16, 17, 24, 25, 32 };
        g_memory[0x565010] = kBits[random(9)];
    }
    setWord32(0x567dcc, random(4) == 0 ? 0x7e0480 : 0);
    setWord32(0x5643f0, random(2) ? 0 : kBitmapHere);
    setWord32(0x567184, random(2) ? 0 : 0x7e0484);

    // DirectDraw's displays.
    const auto display = [&](x86::reg32 at) {
        setWord32(at, kDirectDraw);
        setWord32(at + 4, random(2) ? kClipperObject : 0);
        setWord32(at + 0xc, random(3) == 0 ? x86::reg32(-1) : random(8));
        setWord32(at + 0x10, random(3) == 0 ? 0 : kModesList);
        setWord32(at + 0x24, random(2) ? kPaletteObject : 0);
        setWord32(at + 0x440, random(4) == 0);
        setWord32(at + 0x444, random(3) ? kSurfaceA : 0);
        setWord32(at + 0x448, random(2) ? kSurfaceB : 0);
        setWord32(at + 0x44c, random(2) ? kBitmapHere : 0);
        setWord32(at + 0x450, random(2) ? kBitmapHere : 0);
        setWord32(at + 0x454, random(4) ? random(0) : 0);
        for (const x86::reg32 flag : { 0x434u, 0x435u, 0x436u, 0x45du, 0x45eu, 0x45fu, 0x460u, 0x462u })
            g_memory[at + flag] = x86::reg8(random(3) == 0 ? random(256) : 0);
        g_memory[at + 0x437] = x86::reg8(random(4));
        g_memory[at + 0x463] = x86::reg8(random(4));
    };
    display(0x9f38d0);
    display(kOtherDisplay);
    setWord32(0x564480, random(4) == 0 ? 0 : random(0));
    if (random(3) == 0)
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord32(0x9f0990 + i * 4, 0);
}
