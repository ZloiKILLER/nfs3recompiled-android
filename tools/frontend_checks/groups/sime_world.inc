// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    using namespace nfs3hp;
    static const bool standInsAdded = [] {
        g_methods[0x4eb230] = &nfs3hp::simeFraction;
        g_methods[0x4ea920] = &nfs3hp::simeOnX87<0x4ea920>;
        g_methods[0x4ea930] = &nfs3hp::simeOnX87<0x4ea930>;
        g_methods[0x4ea950] = &nfs3hp::simeOnX87<0x4ea950>;
        g_methods[0x4972f0] = &nfs3hp::simeNormalise;
        g_methods[0x4e0000] = &nfs3hp::simeSubtract;
        g_methods[0x4e0110] = &nfs3hp::simeTranspose;
        g_methods[0x4217e0] = &nfs3hp::simeCameraNode;
        g_methods[0x4386b0] = &nfs3hp::simeFolder<0x4386b0>;
        g_methods[0x4387e0] = &nfs3hp::simeFolder<0x4387e0>;
        g_methods[0x438340] = &nfs3hp::simePath<0x438340>;
        g_methods[0x4389d0] = &nfs3hp::simePath<0x4389d0>;
        g_methods[0x4e1620] = &nfs3hp::simeReserve;
        g_methods[0x4a33d0] = &nfs3hp::simeTick;
        return true;
    }();
    (void)standInsAdded;

    const auto someFloat = [&]() -> float {
        switch (random(24))
        {
        case 0:
            return 0.0f;
        case 1:
            return -0.0f;
        case 2:
            return float(int(random(16)) - 8) / 4.0f;
        case 3:
            return simeBits(random(0));
        case 4:
            return simeBits(kSimeNaN);
        default:
            return float(int(random(1u << 24)) - (1 << 23)) / float(1u << (15 + random(10)));
        }
    };
    // A float of real size, never either zero.
    const auto someSize = [&]() -> float {
        return (float(1 + random((1u << 24) - 1)) / float(1u << (15 + random(10)))) * (random(2) ? 1.0f : -1.0f);
    };

    // The folders: the cars' own and some names, P and three digits among them.
    std::memcpy(g_memory + 0x7a28b4, "cars", 5);
    static const char* const kNames[kSimeNameCount] = { "P123", "P12", "Pabc", "Q123", "P1x3", "P12a", "P999", "Px23" };
    for (unsigned i = 0; i < kSimeNameCount; ++i)
        std::strcpy(reinterpret_cast<char*>(g_memory) + kSimeNames + kSimeNameBytes * i, kNames[i]);
    for (x86::reg32 c = '0'; c <= '9'; ++c)
        g_memory[0x564ef0 + c + 1] = x86::reg8(g_memory[0x564ef0 + c + 1] | 0x20);
    for (x86::reg32 c = 'a'; c <= 'z'; ++c)
        g_memory[0x564ef0 + c + 1] = x86::reg8(g_memory[0x564ef0 + c + 1] & ~0x20);

    // The fog's zones: their nodes in the camera's range, often a side with no width.
    setWord32(0x55d274, random(8) == 0 ? 0 : 1 + random(5));
    setWord32(0x55d278, kSimeZones);
    for (x86::reg32 z = 0; z < 6; ++z)
    {
        const x86::reg32 zone = kSimeZones + 0x24 * z;
        for (x86::reg32 b = 0; b < 12; ++b)
            g_memory[zone + b] = x86::reg8(random(256));
        for (x86::reg32 i = 0; i < 3; ++i)
            setWord32(zone + 0xc + 4 * i, random(8) == 0 ? random(0) : random(1u << 20));
        const int first = int(random(70)) - 5;
        const int middle = random(4) == 0 ? first : first + int(random(20));
        const int last = random(4) == 0 ? middle : middle + int(random(20));
        setWord32(zone + 0x18, x86::reg32(first));
        setWord32(zone + 0x1c, x86::reg32(middle));
        setWord32(zone + 0x20, x86::reg32(last));
    }

    // The view rectangles: about the sizes they have, and the screen's.
    setWord32(0x7cda68, random(8) == 0 ? random(0) : 200 + random(1000));
    for (const x86::reg32 view : { 0x7cda80u, 0x7cdaa0u, 0x7cdb88u })
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(view + 4 * i, random(4) == 0 ? x86::reg32(-int(random(50))) : random(1000));
    setWord32(0x6fbc4c, random(2));
    setWord32(0x6fd3b0, random(3));
    g_memory[0x7a3a58] = x86::reg8(random(2) ? 0x10 : random(256));

    // The clock; each level of effects, the full one most.
    setWord32(0x7d3684, random(4) == 0 ? random(0) : random(4000));
    setWord32(0x6fbc30, random(2) ? 0 : random(5) == 0 ? random(0) : random(3));

    // The kinds.  Divisors that cannot trap: what the game's table has.
    for (x86::reg32 k = 0; k < 14; ++k)
    {
        const x86::reg32 kind = 0x560b64 + (k << 7);
        setWord32(kind + 0x00, random(5) == 0 ? 0 : random(4));
        setWord32(kind + 0x04, 2 + random(random(2) ? 8 : 400));
        setWord32(kind + 0x08, random(2) ? 0 : random(4));
        setFloat(kind + 0x0c, random(4) == 0 ? 0.0f : someFloat());
        for (x86::reg32 at = 0x10; at <= 0x1c; at += 4)
            setFloat(kind + at, someFloat());
        setFloat(kind + 0x24, someFloat());
        setFloat(kind + 0x28, someFloat());
        setFloat(kind + 0x30, someFloat());
        // So that none of the third vector's speed is lost: the sparks' children have a life.
        setFloat(kind + 0x34, someSize());
        for (x86::reg32 at = 0x40; at <= 0x48; at += 4)
            setFloat(kind + at, someFloat());
        if (random(2))
            setWord32(kind + 0x54, 0);
        setWord32(kind + 0x64, random(4) == 0 ? 0 : 1 + random(3));
        setWord32(kind + 0x68, random(6) == 0 ? 0 : 2 + random(random(2) ? 8 : 400));
        setFloat(kind + 0x70, someFloat());
        setFloat(kind + 0x74, random(3) == 0 ? 0.0f : someFloat());
        setFloat(kind + 0x7c, random(2) ? 0.0f : someFloat());
    }
    for (x86::reg32 player = 0; player < 8; ++player)
        setWord32(0x79c4a0 + 4 * player, random(6) == 0 ? 1 + random(9) : 0);

    // The particles: all with life, in small numbers with ties among them or in any at all,
    // or some place free; the first the one to take as often as any.
    const bool particlesTie = random(2) != 0;
    for (x86::reg32 i = 0; i < 600; ++i)
    {
        const x86::reg32 at = 0x7ddfd0 + (i << 7);
        setWord32(at, particlesTie ? 1 + random(20) : (word32(at) | 1));
    }
    switch (random(4))
    {
    case 0:
        setWord32(0x7ddfd0 + (random(600) << 7), 0);
        break;
    case 1:
        setWord32(0x7ddfd0, 0);
        break;
    case 2:
        setWord32(0x7ddfd0, 0x80000000u);
        break;
    default:
        break;
    }

    // The drops: none, a few or all, near each other as often as not.
    const unsigned dropsKeep = random(5);
    for (x86::reg32 i = 0; i < 400; ++i)
    {
        const x86::reg32 drop = 0x7f0bd0 + (i << 6);
        const bool live = dropsKeep == 0 ? false : dropsKeep == 1 ? true : dropsKeep == 4 ? i < 200 : random(dropsKeep == 2 ? 40 : 3) == 0;
        setWord32(drop, live ? 1 + random(100) : 0);
        setWord32(drop + 4, random(3) == 0 ? 0xb : random(14));
        setFloat(drop + 0x34, random(16) == 0 ? someFloat() : float(int(random(16)) - 8) / 8.0f);
        setFloat(drop + 0x38, random(16) == 0 ? someFloat() : float(int(random(16)) - 8) / 8.0f);
        setFloat(drop + 0x3c, random(16) == 0 ? someFloat() : float(random(64)) / 8.0f);
    }

    // The track and the strips over it, corners and wheels within reach of each other.
    setWord32(0x5dd940, 3 + random(60));
    const x86::reg32 car = kSimeCar;
    setWord32(car + 0x1c, random(word32(0x5dd940)));
    for (x86::reg32 at = 0x98; at <= 0xac; at += 4)
        setFloat(car + at, someFloat());
    setFloat(car + 0xb8, someFloat());
    for (x86::reg32 at = 0xc0; at < 0xe4; at += 4)
        setFloat(car + at, someFloat());
    setWord32(car + 0x594, random(2));
    for (x86::reg32 w = 0; w < 4; ++w)
        for (x86::reg32 axis = 0; axis < 3; ++axis)
            setWord32(car + 0x304 + 0x78 * w + 4 * axis, x86::reg32(int(random(1u << 18)) - (1 << 17)));
    for (x86::reg32 s = 0; s < 8; ++s)
    {
        const x86::reg32 strip = 0x7dd850 + 0x50 * s;
        setWord32(strip, random(3) == 0 ? 1 : 0);
        setWord32(strip + 0x34, random(4) == 0 ? random(word32(0x5dd940)) : word32(car + 0x1c) + random(3) - 1);
        setWord32(strip + 0x4c, random(3) == 0 ? word32(0x7d3684) + 1 + random(10) : word32(0x7d3684) - random(10));
        for (x86::reg32 at = 4; at < 0x34; at += 4)
            setFloat(strip + at, random(16) == 0 ? someFloat() : float(int(random(1u << 20)) - (1 << 19)) * 0x1p-18f);
    }

    // Low memory holds something: a write through a null particle would show.
    setWord32(0x40, 0xa5a5a5a5u);
    setWord32(0x55e88c, random(16) == 0 ? 1 + random(9) : 0);
    setWord32(0x560110, random(4) == 0 ? 0 : kSimeBlock);

    // What the skid strips' segment copies, and where.
    for (x86::reg32 at = 0; at < 0x40; at += 4)
        setWord32(kSimeRecords + at, random(0));
}
