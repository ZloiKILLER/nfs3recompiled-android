// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kCars = 0x911000, kCarBytes = 0x1000, kCarCount = 16;
    constexpr x86::reg32 kCarData = 0x921000, kCarDataBytes = 0x200;
    constexpr x86::reg32 kSliceTable = 0x923000;
    constexpr x86::reg32 kView = 0x910000;
    constexpr x86::reg32 kFrameCorners = 0x930000;

    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // Another group's, stood in for here by hand: a row would go round rendbDistance.
        specs.erase(0x4965b0);
        // The file there or not, and the markers set or not.
        specs[0x4e0ec0].kind = Kind::Bool;
        specs[0x48b5f0].kind = Kind::Bool;
        g_methods[0x4965b0] = &nfs3hp::rendbDistance;
        g_methods[0x4c12e0] = &nfs3hp::rendbQuad;
        g_methods[0x4c0b20] = &nfs3hp::rendbSegment;
        g_methods[0x49bd10] = &nfs3hp::rendbNewBatch;
        g_methods[0x4e01f0] = &nfs3hp::rendbDot;
        g_methods[0x4ea9c0] = &nfs3hp::rendbAngle;
        g_methods[0x4eaab0] = &nfs3hp::rendbRotate;
        g_methods[0x4ea9e0] = &nfs3hp::rendbScale;
        g_methods[0x4ef1e0] = &nfs3hp::rendbFindPicture;
        g_methods[0x4e0ec0] = &nfs3hp::rendbLoadFile;
        return true;
    }();
    (void)standInsAdded;

    // A float of real size, now and then an end.
    const auto size = [&](float scale) {
        switch (random(16))
        {
        case 0:
            return 0.0f;
        case 1:
            return -0.0f;
        case 2:
            return nfs3hp::rendbBits(0x7fc00000);
        // Past a float in a product, where the x87's exponent goes on.
        case 3:
            return random(2) ? 1e30f : -1e30f;
        case 4:
            return 1e-30f;
        default:
            return float(x86::sreg32(random(4001)) - 2000) * scale / 2000.0f;
        }
    };

    // The cars.
    setWord32(0x5efda0, random(10));
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 car = kCars + i * kCarBytes;
        setWord32(0x5efc48 + 4 * i, car);
        setWord32(car + 0x2bc, random(11));
        setWord32(car + 0x1f0, random(16));
        setWord32(car + 0x220, random(0));
        setWord32(car + 0x200, random(4) == 0 ? random(256) : random(2) ? 0x20 : 1);
        setWord32(car + 0x2c4, random(6));
        setWord32(car + 0x624, random(3) == 0 ? 0 : random(0));
        setWord32(car + 0x1c, random(8));
        setWord32(car + 0x528, kCarData + i * kCarDataBytes);
        for (x86::reg32 at = 0x98; at < 0xa4; at += 4)
            setFloat(car + at, size(2000.0f));
        // The rev limit: a little to either side of where the scales change, in five sixths.
        static const x86::reg32 kScales[] = { 0x222e, 0x203a, 0x1e46, 0x1c52, 0x1a5e, 0x186a, 0x1676, 0x1482, 0x128e, 0x109a };
        setWord32(kCarData + i * kCarDataBytes + 0x1c4, (kScales[random(10)] * 6) / 5 + random(5) - 2);
    }
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord32(0x793098 + 4 * i, random(0));
    for (x86::reg32 i = 0; i < 2; ++i)
        setWord32(0x5efac8 + 4 * i, kCars + random(kCarCount) * kCarBytes);
    setWord32(0x6fd4f4, random(2));
    setWord32(0x6fd3b0, random(3) == 0 ? random(3) : 1);
    setWord32(0x6fd4c8, random(2));

    // The rows in pursuit, by view.
    for (x86::reg32 v = 0; v < 2; ++v)
        for (x86::reg32 i = 0; i < 16; ++i)
        {
            const x86::reg32 row = 0x725278 + v * 0x140 + i * 0x28;
            setWord32(row, random(8) == 0 ? 7 : random(8));
            setWord32(row + 0x24, kCars + random(kCarCount) * kCarBytes);
        }
    for (x86::reg32 at = 0x700230; at < 0x700240; at += 4)
        setWord32(at, random(2));

    // The view: its axes and position, now and then a depth of nothing either way.
    setWord32(kView + 4, random(2));
    for (x86::reg32 at = 8; at < 0x68; at += 4)
        setFloat(kView + at, size(1.5f));
    if (random(4) == 0)
    {
        for (const x86::reg32 at : { 0x4cu, 0x58u, 0x64u })
            setFloat(kView + at, 0.0f);
        setFloat(kView + 0x40, random(2) ? 0.0f : -0.0f);
    }
    setFloat(0x56009c, size(600.0f));
    setFloat(0x5600a0, size(600.0f));
    setFloat(0x5600a4, float(random(640)));
    setFloat(0x5600a8, float(random(480)));
    g_memory[0x7d3684] = x86::reg8(random(256));
    // The screen's limits, as a corner's numbers are floats; compared as integers.
    {
        const auto bitsOf = [](float value) {
            x86::reg32 bits;
            std::memcpy(&bits, &value, 4);
            return bits;
        };
        setWord32(0x7d34f8, random(4) == 0 ? random(0) : bitsOf(float(1 + random(8))));
        setWord32(0x7d34fc, random(4) == 0 ? x86::reg32(-5) : bitsOf(float(random(40))));
        setWord32(0x7d3504, random(4) == 0 ? x86::reg32(-5) : bitsOf(float(random(40))));
        setWord32(0x7d350c, bitsOf(float(100 + random(800))));
        setWord32(0x7d3500, bitsOf(float(100 + random(800))));
    }
    setWord32(0x7cdac8, random(2) ? 320 + random(2) : 640 + random(1000));
    setWord32(0x7cdacc, 200 + random(1000));

    // The blank quads and the identity, in the image: noise, to see them copied, and w's to the far limit's sides.
    for (x86::reg32 at = 0x47ba70; at < 0x47bb70 + 36; at += 4)
        setWord32(at, random(0));
    for (x86::reg32 corner = 0; corner < 4; ++corner)
    {
        static const x86::reg32 kWs[] = { 0x3f7fff00, 0x3f7fff00, 0x80000000, 0x7f000000, 0x10 };
        setWord32(0x47ba70 + corner * 0x20 + 0xc, kWs[random(5)]);
    }

    // The tables of the figures' rectangles.
    for (x86::reg32 at = 0x55d370; at < 0x55d400; at += 4)
        setFloat(at, random(16) == 0 ? nfs3hp::rendbBits(0x7fc00000) : random(8) == 0 ? 3e38f : float(random(256)));

    // Whether a mode has a look and a turn, and the mode.
    for (x86::reg32 at = 0x55d2dc; at < 0x55d334; at += 4)
        setWord32(at, random(2));
    setWord32(0x55d2d8, random(11));
    setWord32(0x6fd4c0, random(2));
    // The slices: noise, the bytes the vectors are read from among it.
    setWord32(0x5dd948, kSliceTable);
    fillRandom(kSliceTable, kSliceTable + 8 * 36 + 0x20);

    // Element 9's frame: its sides at the screen's edges and either side of them.
    setWord32(0x791a90, kFrameCorners);
    setWord32(0x791a80, random(0x100));
    setFloat(kFrameCorners, random(2) ? float(x86::sreg32(random(5)) - 2) : size(100.0f));
    setFloat(kFrameCorners + 4, random(2) ? float(x86::sreg32(random(5)) - 2) : size(100.0f));
    setFloat(kFrameCorners + 0x120, float(word32(0x7cdac8)) + float(x86::sreg32(random(5)) - 2));
    setFloat(kFrameCorners + 0x244, float(word32(0x7cdacc)) + float(x86::sreg32(random(5)) - 2));
}
