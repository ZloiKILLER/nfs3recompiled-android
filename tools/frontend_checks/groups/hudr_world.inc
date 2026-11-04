// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What these functions call and the check's table lacks: the registers each reads,
    // its stack arguments, what its ret pops, what it changes besides eax (read off nfs3.exe).
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto plain = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0, std::uint8_t clobbers = 0) {
            specs[at] = { in, stack, pops, clobbers, Kind::Plain };
        };
        specs[0x422bd0] = { EAX, 0, 0, 0, Kind::Shape };               // the car a view looks at: memory to read
        specs[0x47bc20] = { EAX, 0, 0, 0, Kind::Bool };
        plain(0x4d1340, 0);
        plain(0x4808b0, EAX);
        plain(0x48c7c0, EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX);
        plain(0x49be40, EAX);
        plain(0x4800a0, EAX);
        plain(0x489d70, 0, 6, 24);
        plain(0x489b30, EAX | EBX | EDX, 7, 28, EBX | EDX);
        plain(0x48a220, EAX, 2, 8);
        plain(0x481170, EAX | EBX | EDX, 0, 0, EDX);
        plain(0x47fbe0, EAX);
        plain(0x49bd30, EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX);
        plain(0x49bdc0, EAX | EBX | ECX | EDX, 1, 4, EBX | EDX);
        plain(0x459410, EAX | EDX, 0, 0, EDX);
        plain(0x489c40, EAX, 5, 20);
        plain(0x489e00, EAX | EBX | ECX | EDX);
        plain(0x48d4c0, EAX | EDX, 0, 0, EDX);
        plain(0x431900, EAX | EDX);
        plain(0x4cdee0, 0);
        plain(0x48a3a0, EAX | EDX, 7, 28, EDX);
        specs[0x4897f0] = { EAX, 6, 24, 0, Kind::Plain, true };        // the text it is given recorded
        plain(0x47ee80, EAX | EDX, 0, 0, EDX);
        plain(0x48ae70, 0, 4, 16);
        plain(0x459350, EAX | EBX | ECX | EDX, 1, 4, EBX | EDX);
        plain(0x48b5f0, EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX);
        plain(0x48a660, EAX, 4, 16);
        plain(0x48c0e0, EAX | EBX | ECX | EDX, 3, 12, EBX | ECX | EDX);
        plain(0x482030, EAX | EBX | EDX, 0, 0, EBX | EDX);
        plain(0x482190, EAX);
        plain(0x484030, 0);
        plain(0x480910, EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX);
        plain(0x480a00, EAX);
        plain(0x480fc0, EAX);
        plain(0x4812e0, EAX);
        plain(0x48d110, EAX | EBX | EDX, 0, 0, EBX | EDX);
        plain(0x481aa0, EAX);
        plain(0x481c50, EAX);
        plain(0x481290, EAX | EDX, 0, 0, EDX);
        plain(0x481510, EAX);
        specs[0x481570] = { EAX, 0, 0, 0, Kind::NoEax };               // an empty function
        plain(0x4815d0, EAX);
        plain(0x4818b0, EAX);
        plain(0x4be5e0, EAX);
        plain(0x47fd80, 0);
        plain(0x49bf60, EAX);
        plain(0x49bef0, EAX);
        plain(0x48e790, EAX);
        plain(0x4d1540, EAX);
        plain(0x4c11b0, EAX | EBX | EDX, 0, 0, EBX | EDX);
        plain(0x4c0b20, EAX | EDX, 0, 0, EDX);
        // Stand-ins that do more than kSpecs' kinds: hudr_hooks.inc.
        g_methods[0x4c0b20] = &nfs3hp::hudrSegment;
        g_methods[0x4c11b0] = &nfs3hp::hudrTriangle;
        g_methods[0x48e790] = &nfs3hp::hudrPaneStep;
        return true;
    }();
    (void)standInsAdded;

    // The language, and the time's format for it, which the world's noise has overwritten.
    setWord32(0x6fbb4c, random(5));
    static const x86::reg32 kTimeFormatsOfLanguages[] = { 0x53b78c, 0x53b79c, 0x53b7ac, 0x53b7ac, 0x53b7ac };
    for (x86::reg32 i = 0; i < 5; ++i)
        setWord32(0x55d6cc + 4 * i, kTimeFormatsOfLanguages[i]);
    setWord32(0x6fd3b0, random(3) == 0 ? random(3) : 1);

    // The elements' boxes and alignments, mostly the screen's numbers.
    for (x86::reg32 at = 0x749758; at < 0x749a38; at += 4)
        setWord32(at, random(5) == 0 ? random(0) : random(640) - 40);
    for (x86::reg32 at = 0x725690; at < 0x725690 + 2 * 0x5c; at += 4)
        setWord32(at, random(5));

    // The HUD's records: the forms of its dials.
    for (x86::reg32 record = 0x6fbc50; record < 0x6fbc50 + 3 * 0x348; record += 0x1a4)
    {
        setWord32(record + 0x18, random(6));
        setWord32(record + 0x1c, random(6));
    }

    // A car's: memory the stand-in for sub_422bd0 hands out one of 16 places of.  Its numbers
    // through kCarData point somewhere; its percentage's two numbers come to no whole now and then.
    for (x86::reg32 k = 0; k < 16; ++k)
    {
        const x86::reg32 car = 0x5f3c00 + 16 * k;
        setWord32(car + 0x528, 0x79f000 + 0x10 * random(0x40));
        if (random(3) == 0)
        {
            const x86::reg32 part = random(3) == 0 ? random(0) : random(200);
            setWord32(car + 0x2dc, part);
            setWord32(car + 0x2e0, random(2) ? 0 - part : random(200));
        }
    }

    // The quads' first corners, as small numbers: some index the back vertices.
    for (const x86::reg32 at : { 0x791adcu, 0x791ad8u, 0x791ac4u, 0x791ad0u, 0x791b38u, 0x791b30u, 0x791b24u, 0x791b48u, 0x791b20u, 0x791accu })
        setWord32(at, random(0x40));
    setWord32(0x791b2c, 0x79d000 + 0x20 * random(0x40));    // the tachometer's corners
    g_memory[0x791ae8] = x86::reg8(random(3) == 0 ? 0 : random(256));   // the art's width and height, as words
    g_memory[0x791ae9] = 0;
    g_memory[0x791aea] = x86::reg8(random(3) == 0 ? 0 : random(256));
    g_memory[0x791aeb] = 0;
    // What the tachometer's corners and the panels' hold: floats, mostly small.
    for (x86::reg32 at = 0x79d000; at < 0x79e000; at += 4)
        if (random(8) != 0)
            setFloat(at, float(x86::sreg32(random(1280)) - 320) / 8.0f);
    for (x86::reg32 at = 0x725758; at < 0x726000; at += 4)
        if (random(8) != 0)
            setFloat(at, float(x86::sreg32(random(1280)) - 320) / 8.0f);
    for (x86::reg32 i = 0; i < 2; ++i)
        setFloat(0x724760 + 4 * i, someValue(g_random));

    // The lines' clip bounds: past a vertex's depth half the time; the points' x and y
    // are small numbers around the others.
    setWord32(0x7d34f8, random(2) ? 0x7f000000 : random(0x40000000));
    setWord32(0x7d34fc, random(4) == 0 ? x86::reg32(-5) : 4);
    setWord32(0x7d350c, 12);
    setWord32(0x7d3504, random(4) == 0 ? x86::reg32(-5) : 4);
    setWord32(0x7d3500, 12);
    // The lines' blank vertices, in the image: noise, to see them copied, and depths around
    // the bound and past it, where the game's own never go.
    for (x86::reg32 at = 0x489630; at < 0x4896f0; at += 4)
        setWord32(at, random(0));
    for (x86::reg32 vertex = 0x489630; vertex < 0x4896f0; vertex += 0x20)
    {
        static const x86::reg32 kDepths[] = { 0x3f7fff00, 0x3f7fff00, 0x80000000, 0x7f000000, 0x10 };
        setWord32(vertex + 0xc, kDepths[random(5)]);
    }

    // The panes sub_48e920 compares: as often as not the same already.
    if (random(2))
    {
        const x86::reg32 style = random(4);
        for (const x86::reg32 at : { 0x6fbfb8u, 0x6fbfb8u + 0x1a4, 0x6fc300u, 0x6fc300u + 0x1a4 })
            setWord32(at, style);
    }
    for (const x86::reg32 record : { 0x6fbf98u, 0x6fbf98u + 0x1a4, 0x6fc2e0u, 0x6fc2e0u + 0x1a4 })
    {
        // Each map's top: at 0 or -0 (the sign alone), or not quite.
        static const x86::reg32 kTops[] = { 0, 0x80000000, 0x40000000, 1 };
        setWord32(record + 0x24 + 17 * 16 + 4, kTops[random(4)]);
        setWord32(record + 0x24 + 22 * 16 + 4, kTops[random(4)]);
    }
}
