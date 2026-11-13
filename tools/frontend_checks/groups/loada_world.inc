// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    using namespace nfs3hp;
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // What is tested for zero or against small numbers.
        for (const x86::reg32 at : { 0x40e220u, 0x439c10u, 0x4395c0u, 0x439610u, 0x453200u, 0x453290u, 0x439680u, 0x4396e0u,
                                     0x439570u, 0x4f07d0u, 0x4edfe8u, 0x4bd2e0u })
            specs[at].kind = Kind::Bool;
        specs[0x439730].kind = Kind::Focus;
        // Paths, recorded.
        for (const x86::reg32 at : { 0x438340u, 0x4389d0u, 0x49d450u, 0x4edfe8u })
            specs[at].string = true;
        g_methods[0x4396b0] = &loadaCarIsPolice;
        g_methods[0x40e2b0] = &loadaRoadLane;
        g_methods[0x439990] = &loadaFindCarById;
        g_methods[0x439810] = &loadaCarColourRgb;
        g_methods[0x439a50] = &loadaCarRecord;
        g_methods[0x439520] = &loadaCarId;
        g_methods[0x438af0] = &loadaFirstCar;
        g_methods[0x438b10] = &loadaNextCar;
        g_methods[0x4392b0] = &loadaCarFolder;
        g_methods[0x4047f0] = &loadaDriverName;
        g_methods[0x438bd0] = &loadaCarCount;
        g_methods[0x4dc490] = &loadaFindSpec;
        g_methods[0x4a6690] = &loadaPlayerMachine;
        g_methods[0x4e0050] = &loadaScaleVectors;
        g_methods[0x4205a0] = &loadaSnapToTrack;
        g_methods[0x4207e0] = &loadaPositionSurface;
        g_methods[0x4a0aa0] = &loadaPlaceOnRoad;
        g_methods[0x4e9200] = &loadaRead;
        g_methods[0x4eebd0] = &loadaWrite;
        g_methods[0x4eb1c0] = &loadaStopPoll;
        return true;
    }();
    (void)standInsAdded;

    const auto setWord16 = [&](x86::reg32 at, std::uint16_t value) { std::memcpy(g_memory + at, &value, 2); };
    const auto makeString = [&](x86::reg32 at, x86::reg32 length) {
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[at + i] = x86::reg8('a' + random(26));
        g_memory[at + length] = 0;
    };

    // The cars' records: a list, forwards, that ends; a name in each.
    for (x86::reg32 c = 0; c < 16; ++c)
    {
        const x86::reg32 car = kLoadaCars + 0x400 * c;
        setWord32(car, c < 15 && random(8) != 0 ? car + 0x400 : 0);
        makeString(car + 0x110, random(12));
    }
    setWord32(0x554f70, random(4) == 0 ? 0 : kLoadaCars + 0x400 * random(8));
    setWord32(0x554f50, random(20));
    for (x86::reg32 s = 0; s < 16; ++s)
        makeString(kLoadaStrings + 0x10 * s, random(12));
    makeString(0x7a28b4, random(40));
    makeString(0x7a2934, random(40));

    // The race's cars and the lists they go on, none past its table's end.
    for (const x86::reg32 at : { 0x5efd88u, 0x5efd8cu, 0x5efd94u, 0x5efd98u, 0x5efd9cu, 0x5efda0u, 0x5efda4u, 0x553074u, 0x553078u })
        setWord32(at, random(8));
    for (x86::reg32 o = 0; o < 8; ++o)
    {
        const x86::reg32 object = kLoadaObjects + 0x1000 * o;
        setWord32(object + 0x220, 0x6fd52c + 0x6c * random(8));
        setWord32(object + 0x7dc, random(10));
        setWord32(object + 0x1c, random(40));
        setWord32(object + 0x638, random(6));
        g_memory[object + 0x88] = x86::reg8(random(3) == 0);
        g_memory[object + 0x89] = x86::reg8(random(3) != 0);
    }
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        setWord32(0x5efac8 + 4 * i, kLoadaObjects + 0x1000 * random(8));
        setWord32(0x6fd52c + 0x6c * i + 0x10, random(17));
        setWord32(0x6fd52c + 0x6c * i + 0x14, random(40));
        setWord32(0x6fd52c + 0x6c * i + 0x18, random(16));
    }
    setWord32(0x5dd940, 40);
    setWord32(0x5dd948, kLoadaSlices);
    setWord32(0x6fd4c0, random(2));
    setWord32(0x6fdc30, random(40));

    // The cast: small numbers of everyone; what is divided by, never 0.
    setWord32(0x6fd3b0, random(6));
    setWord32(0x6fd3b8, random(5));
    setWord32(0x6fd470, random(3));
    setWord32(0x6fd4e8, random(2) ? 0 : random(0));
    setWord32(0x6fd4d4, random(9));
    setWord32(0x6fd510, random(9));
    setWord16(0x6fd3bc, std::uint16_t(random(8)));
    setWord16(0x6fd40c, std::uint16_t(random(8)));
    // The ranks of the two kinds of stage, 16 and past it among them, and who is still in.
    for (x86::reg32 i = 0; i <= 16; ++i)
    {
        setWord16(0x6fd3be + 2 * i, std::uint16_t(random(18)));
        setWord16(0x6fd40e + 2 * i, std::uint16_t(random(18)));
    }
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord16(0x6fd3de + 2 * i, std::uint16_t(random(3)));
    setWord16(0x7a2276, std::uint16_t(random(8)));
    g_loadaPolice = random(3);
    g_loadaNoRoom = random(4) == 0;
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x5555f8 + 4 * i, 1 + random(49));
    for (const x86::reg32 at : { 0x55b040u, 0x55b04cu, 0x55b058u, 0x55b064u, 0x55b06fu, 0x55b078u, 0x55b080u })
        g_memory[at] = x86::reg8(1 + random(11));
    setWord32(0x6fbb40, random(2) ? 0x20f0 : random(0));

    // A saved race.
    setWord32(kLoadaSaved, random(5) != 0 ? 0x20f0 : random(0));
    setWord32(kLoadaSaved + 0x19d0, random(10));
    setWord32(kLoadaSaved + 0x1994, random(8));
    for (x86::reg32 i = 0; i < 10; ++i)
    {
        static const x86::reg32 kFrom[] = { 0, 0x32, 0x1f4, 0x258, 0xfffffff0u };
        static const x86::reg32 kSpan[] = { 0x32, 0x1c2, 0x64, 16, 16 };
        // Half of them on the very ends of the ranges the code tells apart.
        static const x86::reg32 kEnds[] = { 0xffffffffu, 0, 0x31, 0x32, 0x1f3, 0x1f4, 0x257, 0x258 };
        const x86::reg32 pick = random(5);
        setWord32(kLoadaSaved + 0x6c * i + 0x1a00, random(2) ? kEnds[random(8)] : kFrom[pick] + random(kSpan[pick]));
    }

    // The horizon's pieces, and what stops it.
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(0x55d1d0 + 4 * i, kLoadaOut + 0x10 * random(32));
    setWord32(0x55d15c, random(2) ? 0 : random(0));
    setWord32(0x55d278, random(2) ? 0 : random(0));
    setWord32(0x55e88c, random(8) == 0);
    setWord32(0x6fbb4c, random(8));
}
