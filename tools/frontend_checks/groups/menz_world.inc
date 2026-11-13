// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kDisplay = 0xa0f000, kDirectDraw = 0xa00500, kSurface = 0xa00510, kVtable = 0xa00100;
    constexpr x86::reg32 kDriver = 0xa00200, kFontRecord = 0xa05000, kGlyphs = 0xa06000, kFontData = 0xa04000;
    constexpr x86::reg32 kDevices = 0xa0a000, kOps = 0xa0c000, kBlocks = 0xa0d000, kText = 0xa01f00;
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // COM: stdcall, ebx, esi and edi kept.
        const auto com = [&](x86::reg32 slot, std::uint8_t words) {
            specs[0x7e0500 + slot] = { 0, words, std::uint8_t(words * 4), ECX | EDX, Kind::Plain };
        };
        com(0x08, 1);  // Release
        com(0x18, 4);  // CreateSurface
        com(0x30, 3);  // GetAttachedSurface
        com(0x54, 2);  // GetPixelFormat
        com(0x58, 2);  // GetSurfaceDesc
        com(0x74, 3);  // SetColorKey
        com(0x7c, 2);  // SetPalette
        com(0x80, 2);  // Unlock
        g_methods[0x7e0500 + 0x18] = &nfs3hp::menzCreateSurface;
        g_methods[0x7e0500 + 0x58] = &nfs3hp::menzSurfaceDesc;
        // Windows' functions, through their slots.
        const auto import = [&](x86::reg32 slot, std::uint8_t words, Kind kind) {
            specs[word32(slot)] = { 0, words, std::uint8_t(words * 4), ECX | EDX, kind };
        };
        import(0x5344b8, 1, Kind::NoEax);  // EnterCriticalSection
        import(0x534588, 1, Kind::NoEax);  // LeaveCriticalSection
        import(0x534568, 0, Kind::Plain);  // GetTickCount
        import(0x5344a0, 3, Kind::Plain);  // CreateMutexA
        import(0x534534, 0, Kind::Plain);  // GetLastError
        import(0x534734, 2, Kind::Bool);   // FindWindowA
        import(0x534798, 2, Kind::Plain);  // ShowWindow
        import(0x534788, 1, Kind::Plain);  // SetForegroundWindow
        import(0x534748, 1, Kind::Plain);  // GetKeyboardType
        g_methods[word32(0x534534)] = &nfs3hp::menzGetLastError;
        g_methods[word32(0x534748)] = &nfs3hp::menzGetKeyboardType;
        // The graphics driver's table, the video driver's, through memory.
        specs[0x7e0614] = { EAX, 0, 0, EBX | ECX | EDX, Kind::Plain };
        g_methods[0x7e0614] = &nfs3hp::menzAllocate;
        for (x86::reg32 slot = 0x7e0700; slot < 0x7e0710; slot += 4)
            specs[slot] = { 0, 0, 0, 0, Kind::Plain };
        // What is read through.
        specs[0x4ed2a0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4ef190].kind = Kind::Shape;
        specs[0x4e1620].kind = Kind::Bool;
        specs[0x509a30].kind = Kind::Bool;
        specs[0x4df7a0].kind = Kind::Bool;    // the thread, or none
        g_methods[0x4eb5e0] = &nfs3hp::menzLock;
        g_methods[0x50e3f0] = &nfs3hp::menzListFind;
        g_methods[0x50e5a0] = &nfs3hp::menzListFetch;
        g_methods[0x4eb880] = &nfs3hp::menzCountBits;
        g_methods[0x4df8e0] = &nfs3hp::menzSleep;
        g_methods[0x4dfc60] = &nfs3hp::menzWaitEvent;
        return true;
    }();
    (void)standInsAdded;

    // The COM objects.
    for (x86::reg32 slot = 0; slot < 0x100; slot += 4)
        setWord32(kVtable + slot, 0x7e0500 + slot);
    setWord32(kDirectDraw, kVtable);
    setWord32(kSurface, kVtable);
    // The display.
    setWord32(kDisplay, kDirectDraw);
    setWord32(kDisplay + 0x43c, random(4) == 0 ? 0 : kSurface);
    setWord32(kDisplay + 0x444, kSurface);
    g_memory[kDisplay + 0x45f] = x86::reg8(random(2));
    setWord32(0x9f38d0 + 0x43c, random(2) ? kSurface : 0);
    // The list of modes.
    setWord32(0x9f09a0, random(4));
    setWord32(0xa01000 + 0x54, 8 + 8 * random(4) - random(2));
    setWord32(0xa01100 + 8, random(4));
    setWord32(0xa01100 + 0x10, random(3));

    // The drivers.
    setWord32(0x56718c, kDriver);
    setWord32(kDriver + 0x14, 0x7e0614);
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x56445c + 4 * i, 0x7e0700 + 4 * i);
    // What says which way.
    setWord32(0x568f30, random(2));
    setWord32(0x569058, random(3) == 0);
    setWord32(0x5612a4, random(2) ? 0 : kText);
    setWord32(0x567188, random(2));
    setWord32(0x9f38c8, 0xa01f80);
    setWord32(0xa01f80, kText);
    static const x86::reg8 kFpuKinds[] = { 0, 0, 2, 3, 3, 4 };
    g_memory[0x5644c1] = kFpuKinds[random(6)];
    g_memory[0x567858] = x86::reg8(random(4) == 0 ? 1 : 0);

    // A font: its record, its glyphs in order, and a .FNT.
    setWord32(kFontRecord + 0x20, random(40));
    setWord32(kFontRecord + 0x28, random(5) == 0 ? 0x63 : 0x64);
    setWord32(kFontRecord + 0x30, kGlyphs);
    for (x86::reg32 i = 0; i < 0x60; ++i)
    {
        const std::uint16_t character = std::uint16_t(0x20 + i * (random(4) ? 1 : 2));
        std::memcpy(g_memory + kGlyphs + 11 * i, &character, 2);
    }
    std::memcpy(g_memory + kFontData, random(6) ? "FNTx" : "XXXX", 4);
    {
        const std::uint16_t count = std::uint16_t(random(40));
        std::memcpy(g_memory + kFontData + 0xa, &count, 2);
        const std::uint16_t glyphCount = std::uint16_t(random(2) ? random(0x100) : 0x100 + random(0x100));
        std::memcpy(g_memory + kFontData + 0x20, &glyphCount, 2);
    }
    setWord32(kFontData + 0x1c, 0x100);
    setWord32(kFontData + 0x14, 0x200);
    setWord32(kFontData + 0xc, random(0));
    static const x86::reg8 kTypes[] = { 1, 3, 0x40, 0x44, 0x4f, 0x50, 0x55, 0x56, 0x57, 0x60, 0x62, 0x63, 0x78, 0x79, 0x7a, 0x7b, 0x90 };
    g_memory[kFontData + 0x100] = kTypes[random(17)];
    g_memory[0xa07000] = kTypes[random(17)];

    // The file devices and their operations.
    setWord32(0x9f4f60, kDevices);
    for (x86::reg32 i = 0; i < 32; ++i)
    {
        const x86::reg32 device = kDevices + i * 0x68;
        setWord32(device, random(2));
        setWord32(device + 0x18, random(3) == 0 ? kOps + 0x40 * random(8) : 0);
    }
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 op = kOps + 0x40 * i;
        setWord32(op + 4, random(0x40));
        setWord32(op + 8, random(12));
        setWord32(op + 0xc, random(4));
        g_memory[op + 0x10] = x86::reg8(random(3));
        setWord32(op + 0x18, random(3) ? random(0) : 0);
        setWord32(op + 0x2c, random(3) ? kBlocks + 0x20 * random(4) : 0);
        setWord32(kBlocks + 0x20 * i + 0xc, random(2) ? random(0) : 0);
    }
}
