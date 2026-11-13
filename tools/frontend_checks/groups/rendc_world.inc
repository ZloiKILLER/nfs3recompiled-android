// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What the rows of specs_from_exe.py leave out: a float on the x87, as sub_46c9b0's stand-in
    // does it, for the maths library's calls and sub_49d9d0.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto onX87 = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack, std::uint8_t pops, std::uint8_t clobbers) {
            specs[at] = { in, stack, pops, clobbers, Kind::SliderValue };
        };
        onX87(0x4ea920, 0, 1, 4, 0);
        onX87(0x4ea930, 0, 1, 4, 0);
        onX87(0x4ea950, 0, 1, 4, 0);
        onX87(0x4ea9c0, 0, 2, 8, 0);
        onX87(0x4eb230, 0, 0, 0, 0);
        onX87(0x4f1c30, 0, 2, 8, 0);
        onX87(0x4f2d78, 0, 2, 8, 0);
        onX87(0x49d9d0, EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX);
        // Stand-ins that do more than kSpecs' kinds: rendc_hooks.inc.
        g_methods[0x4dffb0] = &nfs3hp::rendcAddVectors;
        g_methods[0x4e0000] = &nfs3hp::rendcSubtractVectors;
        g_methods[0x4e0280] = &nfs3hp::rendcMultiplyMatrices;
        g_methods[0x4e0430] = &nfs3hp::rendcTransformPoints;
        g_methods[0x420650] = &nfs3hp::rendcPlaceOnTrack;
        g_methods[0x4e1620] = &nfs3hp::rendcReserveMemory;
        g_methods[0x4bbde0] = &nfs3hp::rendcObjectsAllocate;
        return true;
    }();
    (void)standInsAdded;

    const auto number = [&](x86::reg32 span, x86::sreg32 from, float over) {
        // Now and then 0, or a NaN of its own: the order of an fp:: shows only between two of them.
        const x86::reg32 pick = random(16);
        if (pick == 0)
            return 0.0f;
        if (pick == 1)
        {
            const x86::reg32 bits = 0x7fc00000 | random(0x400000);
            float nan;
            std::memcpy(&nan, &bits, 4);
            return nan;
        }
        return float(x86::sreg32(random(span)) + from) / over;
    };
    // The tick, and the ticks stepped to: at most a few behind, around the 0x480 sub_493a30 asks.
    const x86::reg32 tick = random(8) == 0 ? random(0) : 0x470 + random(0x20);
    setWord32(0x7d3684, tick);
    setWord32(0x55e49c, random(4) == 0 ? tick + 1 : tick - random(3));
    setWord32(0x55e4c0, random(4) == 0 ? tick + 1 : tick - random(3));
    setWord32(0x6fd498, random(8) == 0 ? 0 : 1 + random(2));

    // The road's slices.
    const x86::reg32 slices = random(8) == 0 ? 0 : 1 + random(8);
    setWord32(0x5dd940, slices);
    setWord32(0x5dd948, 0xb04000);
    for (x86::reg32 s = 0; s < 40; ++s)
    {
        const x86::reg32 record = 0xb04000 + 36 * s;
        g_memory[record + 0x13] = x86::reg8(random(4) | (random(4) << 4));
        if (random(4) != 0)
        {
            setWord32(record + 0x1c, 0x20000 + random(0x400000));
            setWord32(record + 0x20, 0x20000 + random(0x400000));
        }
    }

    // The two rows of cones, far past both ends for what wraps only once.
    setWord32(0x55e4a0, 0xb01000);
    setWord32(0x55e4a4, 0xb02000);
    for (x86::reg32 row : { 0xb01000u, 0xb02000u })
        for (x86::sreg32 slice = -20; slice < 28; ++slice)
        {
            const x86::reg32 at = row + x86::reg32(slice) * 80;
            g_memory[at] = x86::reg8(random(3) == 0 ? random(3) : 0);
            g_memory[at + 1] = x86::reg8(random(16) == 0 ? 0xff : 0);
            g_memory[at + 2] = x86::reg8(random(3) == 0 ? 1 + random(3) : 0);
            g_memory[at + 3] = 0;
            for (x86::reg32 f = 0x2c; f < 0x50; f += 4)
                setFloat(at + f, number(800, -400, 16.0f));
        }

    // The cones' model.
    setWord32(0x55e4a8, 0xb06000);
    setWord32(0xb06000 + 0xf8, random(6));
    // Its height a number: between two NaNs, as here with a cone's height, the host's compiler
    // commutes a sum as it likes in either version, and the game has no NaN there.
    setFloat(0xb06000 + 0x2c, float(x86::sreg32(random(64)) - 32) / 16.0f);

    // A car: moving around the 5 m/s it asks for, with a box and axes of real sizes.
    for (x86::reg32 car : { 0xb07000u, 0xb08000u })
    {
        setWord32(car + 0x1c, random(8) == 0 ? random(16) : (slices ? random(slices) : 0));
        for (x86::reg32 f = 0x98; f < 0xa4 + 12; f += 4)
            setFloat(car + f, number(800, -400, 16.0f));
        setFloat(car + 0xb8, number(320, 0, 16.0f));
        for (x86::reg32 f = 0xc0; f < 0xe4; f += 4)
            setFloat(car + f, number(256, -128, 128.0f));
        for (x86::reg32 f = 0x114; f < 0x120; f += 4)
            setFloat(car + f, number(256, 0, 16.0f));
        setWord32(car + 0x168, random(2) ? (random(2) ? 0x80000000u : 0) : 0x41200000);
        g_memory[car + 0x200] = x86::reg8(random(2));
        setWord32(car + 0x2c4, random(5));
        setFloat(car + 0x7b8, number(256, -128, 16.0f));
    }

    // The lightning.
    setWord32(0x55e4ac, 1 + random(60));
    setFloat(0x55e4b0, number(256, -128, 16.0f));
    setWord32(0x55e4b4, 1 + random(7));
    setFloat(0x55e4b8, number(256, -128, 16.0f));
    setFloat(0x55e4bc, number(1024, -512, 16.0f));
    setWord32(0x79f3f4, 0x7f);
    for (x86::reg32 i = 0; i < 240; ++i)
    {
        const x86::reg32 node = 0x79c570 + 48 * i;
        setWord32(node, random(4) != 0 ? 0 : random(3) == 0 ? 1 : random(2) ? 1 + random(80) : random(0));
        setFloat(node + 8, number(64, -32, 16.0f));
        for (x86::reg32 f = 0xc; f < 0x1c; f += 4)
            setFloat(node + f, number(800, -400, 16.0f));
    }
    // Now and then the table full, or free only at its first.
    if (random(8) == 0)
    {
        const bool firstFree = random(2) != 0;
        for (x86::reg32 i = 0; i < 240; ++i)
            setWord32(0x79c570 + 48 * i, i == 0 && firstFree ? 0 : 1 + random(80));
    }
    setWord32(0x79f270, random(3) == 0 ? 0 : 0x79c570 + 48 * random(240));

    // The lights' table: no blink, the least blink, any.
    for (x86::reg32 light = 0; light < 16; ++light)
    {
        const x86::reg32 pick = random(4);
        g_memory[0x55e3dc + 12 * light + 4] = x86::reg8(pick == 0 ? 0 : pick == 1 ? 1 : 2 + random(200));
        g_memory[0x55e3dc + 12 * light + 5] = 0;
    }

    // The setting in which the track's lights stay out.
    setWord32(0x6fbc30, random(4));

    // How the objects are drawn: the mirror's way or not.
    setWord32(0x7a3a60, random(2) ? 0 : 1 + random(3));

    // The sky's sprites' turn.
    setWord32(0x55e984, random(4) == 0 ? random(0) : random(6));

    // The picture's path.
    std::memcpy(g_memory + 0x7a3134, "fedata\\art\\", 12);

    // The driver's chain of objects, ending, with their vertices somewhere.
    const x86::reg32 objects = random(4);
    setWord32(0x7a3d08, random(3) == 0 ? 0 : 0xb0b000);
    setWord32(0x7a3d0c, objects ? 0xb0a000 : 0);
    for (x86::reg32 o = 0; o < 3; ++o)
    {
        const x86::reg32 node = 0xb0a000 + 0x100 * o;
        setWord32(node, o + 1 < objects ? node + 0x100 : 0);
        for (x86::reg32 v = 0; v < 3; ++v)
            setWord32(node + 8 + 4 * v, 0xb0a400 + 0x40 * random(16));
    }
}
