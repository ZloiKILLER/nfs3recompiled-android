// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What leaves a float on the x87 and writes through a frame: kSpecs' kinds and this
    // group's hooks (renda_hooks.inc).
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x4ea9c0] = { 0, 2, 8, 0, Kind::NoEax };
        specs[0x4ea930] = { 0, 1, 4, 0, Kind::NoEax };
        specs[0x4ea950] = { 0, 1, 4, 0, Kind::NoEax };
        specs[0x4ea920] = { 0, 1, 4, 0, Kind::SliderValue };  // another group's: its stand-in goes by kSpecs alone
        specs[0x4e06a0] = { EAX, 0, 0, 0, Kind::NoEax };
        g_methods[0x4ea9c0] = &nfs3hp::rendaOnX87<0x4ea9c0>;
        g_methods[0x4ea930] = &nfs3hp::rendaOnX87<0x4ea930>;
        g_methods[0x4ea950] = &nfs3hp::rendaOnX87<0x4ea950>;
        g_methods[0x4e0000] = &nfs3hp::rendaVectors<0x4e0000, -1>;
        g_methods[0x4dffb0] = &nfs3hp::rendaVectors<0x4dffb0, 1>;
        g_methods[0x4e0050] = &nfs3hp::rendaScale;
        g_methods[0x4e01c0] = &nfs3hp::rendaUnit;
        g_methods[0x4e01a0] = &nfs3hp::rendaCross;
        g_methods[0x4e06a0] = &nfs3hp::rendaLength;
        g_methods[0x4964d0] = &nfs3hp::rendaFraction;
        g_methods[0x4bf370] = &nfs3hp::rendaScreenVertices;
        g_methods[0x4bf4c0] = &nfs3hp::rendaObjectVertices;
        g_methods[0x4beb80] = &nfs3hp::rendaViewport;
        g_methods[0x4bbf80] = &nfs3hp::rendaListAdd;
        g_methods[0x4bbfc0] = &nfs3hp::rendaListEnd;
        g_methods[0x4bc030] = &nfs3hp::rendaListKeep;
        g_methods[0x4217e0] = &nfs3hp::rendaCameraNode;
        g_methods[0x494390] = &nfs3hp::rendaNewestFlash;
        return true;
    }();
    (void)standInsAdded;

    // A float of the world's: a real magnitude, now and then the noise it was left with.  Some
    // worlds are far past the game's, where a product leaves a float's range, as it may on the x87.
    const std::uint32_t special = random(8);
    const float extreme = special == 0 ? 1.0e25f : 1.0f;
    // And some are half NaNs, all one NaN and no noise beside them: of two different NaNs the
    // compiled + or * keeps either, the same source giving both (sub_479160's last sum, read
    // in both as fp::add(rise, c), differed in one case in forty), so that cannot be checked.
    const bool nans = special == 1;
    nfs3hp::g_rendaNaNs = nans;
    const auto real = [&](x86::reg32 at, float scale) {
        if (nans && random(2) == 0)
            setWord32(at, nfs3hp::kRendaNaN);
        else if (nans || random(16) != 0)
            setFloat(at, (float(random(2001)) - 1000.0f) * scale / 1000.0f * extreme);
    };
    const auto realVector = [&](x86::reg32 at, float scale) {
        for (x86::reg32 i = 0; i < 3; ++i)
            real(at + 4 * i, scale);
    };
    const auto realMatrix = [&](x86::reg32 at) {
        if (random(4) == 0)
        {
            // The identity: a spark's z comes out as it went in, to reach its limits.
            for (x86::reg32 i = 0; i < 9; ++i)
                setFloat(at + 4 * i, i % 4 == 0 ? 1.0f : 0.0f);
            return;
        }
        for (x86::reg32 i = 0; i < 9; ++i)
            real(at + 4 * i, 1.0f);
    };

    // The drivers: voodoo2a's most of the time.
    setWord32(0x7a3a64, random(4) == 0 ? 0 : 4);
    static const std::uint16_t kDriverFlags[] = { 0x10, 0x10, 0x10, 0x10, 0x06, 0x40, 0x180, 0x20 };
    const std::uint16_t driverFlags = kDriverFlags[random(std::size(kDriverFlags))];
    g_memory[0x7a3a58] = x86::reg8(driverFlags);
    g_memory[0x7a3a59] = x86::reg8(driverFlags >> 8);
    setWord32(0x7a3a60, random(2));

    // The screen.
    setFloat(0x56009c, 320.0f);
    setFloat(0x5600a0, 240.0f);
    setFloat(0x5600a4, 320.0f);
    setFloat(0x5600a8, 240.0f);
    real(0x56009c, 640.0f);
    real(0x5600a0, 480.0f);
    // The far plane: near, far, or just where the flat sky's quad stands.
    const x86::reg32 farPick = random(4);
    if (farPick == 0)
        setFloat(0x7d34f8, 0.25f);
    else if (farPick == 1)
        setWord32(0x7d34f8, 0x3f7fff00);
    else
        setFloat(0x7d34f8, 1.0e6f);
    setFloat(0x7d34fc, 0.0f);
    setFloat(0x7d3500, 480.0f);
    setFloat(0x7d3504, 0.0f);
    setFloat(0x7d350c, 640.0f);
    realMatrix(0x7a3d58);
    realVector(0x7a3d4c, 30.0f);

    // The race.
    setWord32(0x7d3684, 1000 + random(1000));
    setWord32(0x6fd3b0, random(3));
    setWord32(0x6fd4ac, random(3) == 0 ? random(10) : 5);

    // The race's font and its table of widths.
    const x86::reg32 font = 0x7e8000;
    setWord32(0x679010, font);
    real(0x679014, 4.0f);
    g_memory[font] = x86::reg8(0x20 + random(0x20));
    g_memory[font + 1] = x86::reg8(random(4) == 0 ? random(256) : 0x40 + random(0x20));
    for (x86::reg32 c = 0; c < 256; c += 3 + random(3))
        setFloat(font + (c << 5) + 0x14, float(random(40)));

    // The sparks: three sets.
    setWord32(0x55b088, random(8) == 0 ? x86::reg32(-1) : random(3));
    for (x86::reg32 s = 0; s < 3; ++s)
    {
        const x86::reg32 set = 0x55b08c + s * 0x238;
        setWord32(set, random(4) == 0 ? random(6) : 4);
        setWord32(set + 4, random(5) == 0 ? 0 : 1);
        setWord32(set + 0x230, random(8));
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            const x86::reg32 spark = set + (i << 6);
            setWord32(spark + 0x30, random(4) == 0 ? random(4) : 2);
            setWord32(spark + 0x3c, random(5) == 0 ? 0 : 1);
            setWord32(spark + 0x40, 1000 + random(1000));
            setWord32(spark + 0x48, random(2) == 0 ? 0 : 0x7e7f00 + 4 * random(8));
            realVector(spark + 0x64, random(2) == 0 ? 130.0f : 90.0f);
            if (random(6) == 0)
                for (x86::reg32 k = 0; k < 3; ++k)
                    setFloat(spark + 0x64 + 4 * k, 0.0f);
        }
    }
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(0x7e7f00 + 4 * i, random(2));
    if (random(6) == 0)
        for (x86::reg32 k = 0; k < 3; ++k)
            setFloat(0x7a3d4c + 4 * k, 0.0f);

    // A view: its height and its two matrices.
    for (x86::reg32 v = 0; v < 2; ++v)
    {
        const x86::reg32 view = 0x7e7000 + v * 0x100;
        // Never a NaN: sub_47b4f0 turns its sign, and a NaN of the other sign counts as another.
        setFloat(view + 0xc, (float(random(2001)) - 1000.0f) / 20.0f);
        realMatrix(view + 0x14);
        realMatrix(view + 0x44);
    }

    // The sky's points and vertices handed in, and its numbers.
    for (x86::reg32 at = 0x7e6000; at < 0x7e6400; at += 4)
        real(at, 200.0f);
    realVector(0x55d2cc, 100.0f);
    realVector(0x55d2b4, 100.0f);
    realVector(0x55d2c0, 100.0f);
    real(0x55d1c8, 300.0f);
    real(0x55d1cc, 1.0f);
    for (x86::reg32 at = 0x55d164; at <= 0x55d174; at += 4)
        real(at, 200.0f);
    setWord32(0x55d15c, random(2));
    setWord32(0x55d158, random(3) == 0);
    for (x86::reg32 i = 0; i < 16; ++i)
        real(0x7001e8 + 4 * i, 1.0f);

    // The fog's colour.
    real(0x55d268, 3.0f);

    // The sun.
    setWord32(0x55d27c, random(5) != 0);
    setWord32(0x55d29c, random(2));
    realVector(0x55d280, 200.0f);
    realVector(0x55d28c, 200.0f);
    real(0x55d298, 50.0f);

    // The dome's grid: the brightest a byte or so, the noise less, and the sky's curve.
    setWord32(0x55d1f8, random(8) == 0 ? random(0x10000) : 128 + random(128));
    setWord32(0x55d1fc, random(16) == 0 ? random(0) : random(300));
    setWord32(0x55d200, random(3) == 0 ? 0 : 1 + random(64));
    real(0x55d24c, 200.0f);
    real(0x55d248, 200.0f);
    for (x86::reg32 at = 0x6ff9fc; at < 0x6ff9fc + 13 * 0x9c; at += 4)
        real(at, 500.0f);

    // The weather: its clock a little behind the race's, a flash's cell around the grid.
    setWord32(0x55d154, word32(0x7d3684) - random(4));
    setWord32(0x55d260, random(4) == 0 ? 0 : random(101));
    setWord32(0x55d264, random(4) == 0 ? 0 : 1 + random(3));
    setWord32(0x55d254, x86::reg32(random(17)) - 2);
    setWord32(0x55d258, x86::reg32(random(17)) - 2);
    setWord32(0x55d1f4, 1 + random(3));
    real(0x55d20c, 1.0f);
    real(0x55d210, 1.0f);
    real(0x55d214, 2.0f);
    real(0x55d218, 2.0f);
}
