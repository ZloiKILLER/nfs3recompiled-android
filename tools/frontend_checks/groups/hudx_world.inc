// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.  What their
    // callers count on the real ones keeping (hudx_entry.inc says for each case) is not clobbered.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x422bd0] = { EAX, 0, 0, 0, Kind::Shape };                  // the camera's car: memory to read
        specs[0x4844c0] = { EAX, 0, 0, ESI | EDI, Kind::Plain };
        specs[0x4853c0] = { EAX | EDX, 0, 0, EDX | ESI | EDI, Kind::Plain };
        specs[0x4800a0] = { EAX, 0, 0, ESI | EDI, Kind::Plain };
        specs[0x481f90] = { EAX, 0, 0, ESI | EDI, Kind::Plain };
        specs[0x48ea20] = { EAX | EDX, 0, 0, EBX | EDX | ESI | EDI, Kind::Plain };
        specs[0x49bc80] = { EAX | EDX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain };
        specs[0x4d1480] = { EAX, 0, 0, EBX | EDX | ESI | EDI, Kind::SliderValue };   // a width on the x87's stack
        specs[0x4d1000] = { EAX | EBX | EDX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::SliderValue };
        return true;
    }();
    (void)standInsAdded;

    // The looks: none selected now and then, some not in use, their lights free and full.
    setWord32(0x55b088, random(6) == 0 ? x86::reg32(-1) : random(8));
    for (x86::reg32 l = 0; l < 8; ++l)
    {
        const x86::reg32 look = 0x55b08c + 0x238 * l;
        setWord32(look + 4, random(4) == 0 ? 0 : random(0));
        setWord32(look + 0x230, random(3) == 0 ? 0 : random(9));
        setWord32(look + 0x234, random(9));
        const bool full = random(4) == 0;
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(look + 0x30 + 0x40 * i + 0xc, full || random(2) ? 1 + random(0x7fffffff) : 0);
    }

    // The HUD's keys: split screen or not, the pane up or not, the network state each of its tests has.
    setWord32(0x6fd3a0, random(3));
    setWord32(0x791b1c, random(2));
    for (x86::reg32 at = 0x6fbc50; at < 0x6fbc50 + 3 * 0x348; at += 0x1a4)
    {
        setWord32(at, random(2) ? 0 : random(0));
        setWord32(at + 0xc, random(2) ? 0 : random(0));
        setWord32(at + 0x10, random(2) ? 0x3f800000 : random(0));
    }
    // A car is a cop's or not: sub_422bd0's answers, 16 bytes apart, and their flags 0x200 on.
    for (x86::reg32 i = 0; i < 16; ++i)
        g_memory[0x5f3c00 + 16 * i + 0x200] = x86::reg8(random(2) ? 0x20 : random(256));

    // The rectangles' counts, the surface's and the fonts'.
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(0x55d694 + 4 * i, random(3) == 0 ? 0 : random(40));
    setWord32(0x793108, random(0x400));
    setWord32(0x840184, random(3));
    setFloat(0x84018c, random(8) == 0 ? float(x86::sreg32(random(0))) : someValue(g_random));
    if (random(16) == 0)
        setWord32(0x84018c, random(0));
}
