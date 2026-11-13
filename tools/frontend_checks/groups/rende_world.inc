// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x496f20].kind = Kind::Bool;                                // a distance within: 0 or 1
        specs[0x4e0820].kind = Kind::Bool;                                // strstr: found or not, not read through
        specs[0x4f24e0].kind = Kind::Bool;                                // a shape's comment or none
        specs[0x4e0ec0].kind = Kind::Bool;                                // the file, or none
        specs[0x4972f0] = { EAX, 0, 0, 0, Kind::SliderValue };            // leaves the length on the x87
        specs[0x4eb230] = { 0, 0, 0, 0, Kind::SliderValue };              // leaves the fraction on the x87
        // The driver's quad and triangle: stdcall, ebx and the rest kept.
        specs[0x7e0200] = { 0, 4, 16, ECX | EDX, Kind::Plain };
        specs[0x7e0210] = { 0, 3, 12, ECX | EDX, Kind::Plain };
        // Stand-ins that do more than kSpecs' kinds: rende_hooks.inc.
        g_methods[0x4bbde0] = &nfs3hp::rendeAllocate;
        g_methods[0x4bbf80] = &nfs3hp::rendeAllocateListed;
        g_methods[0x4bbe70] = &nfs3hp::rendeEmptyList;
        g_methods[0x4c2cd0] = &nfs3hp::rendeClip;
        g_methods[0x4c6c70] = &nfs3hp::rendePutSegment;
        g_methods[0x4d3f50] = &nfs3hp::rendeSquareTexture;
        g_methods[0x423b00] = &nfs3hp::rendeCameraMode;
        g_methods[0x4e0160] = &nfs3hp::rendeCross;
        g_methods[0x4bf4c0] = &nfs3hp::rendeProject;
        return true;
    }();
    (void)standInsAdded;

    // Noise at null, so that a write through a pointer that should be none shows.
    setWord32(0, 0x5a5a5a5a);
    setWord32(0x9ef94c, 0x7e0200);
    setWord32(0x9ef974, 0x7e0210);

    // Which way the frame draws: on the card or sorted here; how many passes.
    setWord32(0x7a3a60, random(2));
    g_memory[0x7a3a58] = x86::reg8(random(2) ? 0x10 : 0x06);
    setWord32(0x6fbc30, random(5) == 0 ? x86::reg32(-1) : random(3));
    setWord32(0x6fd4c8, random(2));
    setWord32(0x6fbc40, random(4));
    setWord32(0x7d3684, random(1000));
    setWord32(0x7d36a0, random(6) == 0);

    // The stack's noise as places of the arena: what a stand-in leaves unwritten in a frame
    // (a block's address, should another group's check take the allocator's stand-in) is read through.
    for (x86::reg32 at = kStack - 0x8000; at < kStack + 0x100; at += 4)
        setWord32(at, 0x920000 + 0x20 * random(0x400));

    // The buckets: empty; a case's own lists go into them (rende_entry.inc).
    std::memset(g_memory + 0x7d5f54, 0, 8);
    std::memset(g_memory + 0x7d5f5c, 0, 0x3e80);
    std::memset(g_memory + 0x7d9ddc, 0, 0x1900);
    std::memset(g_memory + 0x7db6dc, 0, 0x1900);

    // Strips: 64 of them, where sub_4c6cb0 and sub_4c6f00 find them.
    setWord32(0x560110, 0x900000);
    setWord32(0x7d5758, random(4) == 0 ? 0x40 : random(6));
    setWord32(0x7d5754, random(0x40));
    for (x86::reg32 s = 0; s < 0x40; ++s)
    {
        const x86::reg32 strip = 0x900000 + s * 0x340;
        setWord32(strip + 0xc, random(3) == 0 ? 0x10 : random(4) == 0 ? 0 : random(0x10));
        for (x86::reg32 at = 0; at < 12; at += 4)
            setFloat(strip + at, float(x86::sreg32(random(2000)) - 1000));
    }
    setFloat(0x7dcfdc, float(random(100)));
    setFloat(0x8ca280, float(random(64)) / 8.0f);

    // Particles: lives never zero at their kinds, as the art's table has them.
    for (x86::reg32 k = 0; k < 16; ++k)
    {
        setWord32(0x560bcc + 0x80 * k, 1 + random(200));
        setWord32(0x560b64 + 0x80 * k, random(2));
        setWord32(0x560bc8 + 0x80 * k, random(2));
    }
}
