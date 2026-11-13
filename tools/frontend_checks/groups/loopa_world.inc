// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // A root on the x87, as the stand-in of sub_46c9b0 does it.
        specs[0x4ea920] = { 0, 1, 4, 0, Kind::SliderValue };
        // Both answers of what the code branches on.
        specs[0x4e0e60].kind = Kind::Bool;
        specs[0x478100].kind = Kind::Bool;
        specs[0x439d90].kind = Kind::Bool;
        specs[0x438bd0].kind = Kind::Bool;
        // The number of the message with its format: the row reads only the format.
        specs[0x401010] = { 0, 2, 0, 0, Kind::Plain };
        g_methods[0x4beb80] = &nfs3hp::loopaViewport;
        g_methods[0x4beb20] = &nfs3hp::loopaReadRect;
        g_methods[0x4c12e0] = &nfs3hp::loopaClipQuad;
        g_methods[0x4eb390] = &nfs3hp::loopaCompare;
        g_methods[0x4dbbf0] = &nfs3hp::loopaMostlyZero<0x4dbbf0>;
        g_methods[0x4283d0] = &nfs3hp::loopaMostlyZero<0x4283d0>;
        g_methods[0x4c62a0] = &nfs3hp::loopaMostlyZero<0x4c62a0>;
        g_methods[0x4a33d0] = &nfs3hp::loopaKeepAlive;
        g_methods[0x4e43a0] = &nfs3hp::loopaFileSize;
        g_methods[0x4c4e40] = &nfs3hp::loopaRaceStep;
        return true;
    }();
    (void)standInsAdded;

    // The clock: the pose of a tick in the first of the tables, now and then past either end.
    const x86::reg32 pick = random(16);
    setWord32(0x7d3684, pick == 0 ? 0 - random(64) : pick == 1 ? 0x3840 * 8 - 16 + random(32) : random(100 * 8));
    setWord32(0x6fd3a0, random(4) == 0 ? random(3) : 0);
    setWord32(0x55d000, random(4) == 0 ? random(3) : 1);
    setWord32(0x67b140, random(0x3840));
    setWord32(0x6bc6b4, random(8) == 0 ? random(4) : 2 + random(120));
    setWord32(0x679010, random(3) == 0 ? 0 : random(0));

    // The slices of the road: points in 16.16, some of them far out.
    setWord32(0x5dd948, 0xb04000);
    for (x86::reg32 s = 0; s < 64; ++s)
        for (x86::reg32 at = 0; at < 12; at += 4)
            setWord32(0xb04000 + 36 * s + at, random(4) == 0 ? random(0) : x86::reg32(x86::sreg32(random(0x4000000)) - 0x2000000));
    // Poses that stand on them.
    for (x86::reg32 p = 0; p < 130; ++p)
    {
        for (const x86::reg32 table : { 0x67b144u, 0x6bc6b8u })
        {
            g_memory[table + 18 * p + 2] = x86::reg8(random(64));
            g_memory[table + 18 * p + 3] = 0;
        }
    }

    // The bounds of the clipper, the bits of floats of a view's size.
    const auto bitsOf = [](float value) {
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        return bits;
    };
    // The far bound: now and then exactly the depth of the two quads' templates.
    const x86::reg32 farPick = random(6);
    setWord32(0x7d34f8, farPick == 0 ? 0 : farPick == 1 ? word32(0x4a6f8c) : farPick == 2 ? word32(0x4a700c) : bitsOf(float(1 + random(4))));
    setWord32(0x7d34fc, bitsOf(float(random(200))));
    setWord32(0x7d350c, bitsOf(float(200 + random(800))));
    setWord32(0x7d3504, bitsOf(float(random(200))));
    setWord32(0x7d3500, bitsOf(float(200 + random(600))));

    // A session: how it starts and whether the loop goes round.
    setWord32(0x7d3698, random(2));
    setWord32(0x7d36b4, random(4));
    setWord32(0x55e88c, 0);
    setWord32(0x552854, random(2));
    setWord32(0x6fbbf0, random(3) == 0 ? 0 : random(5));
    setWord32(0x6fbbe4, random(5));
    setWord32(0x6fd3b0, random(3));
    setWord32(0x6fd3b8, random(4));
    setWord32(0x7a22e0, random(2));
    g_memory[0x7a22dc] = x86::reg8(random(2));
    g_memory[0x7a22dd] = 0;

    // The start: a type of driver of the few, now and then past the width of the shift.
    setWord32(0x7a3a54, random(8) == 0 ? random(0) : 1 + random(6));
    setWord32(0x7a3a50, random(6));
}
