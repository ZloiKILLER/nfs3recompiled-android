// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // Results the generated code reads through, or tests: of the kinds kSpecs has.
        specs[0x422bd0].kind = Kind::Shape;
        specs[0x422aa0].kind = Kind::Bool;
        specs[0x4c55a0].kind = Kind::Bool;
        // Registers these may read through indirect calls (their rows say so), and one the
        // native leaves as the original does though nothing reads it: recorded.
        specs[0x470f40].in |= ECX;
        specs[0x471070].in |= EBX | ECX;
        specs[0x42cab0].in |= EBX;
        // Stand-ins that do more than kSpecs' kinds: loopb_hooks.inc.
        g_methods[0x431c30] = &nfs3hp::loopbFirstLayer;
        g_methods[0x435310] = &nfs3hp::loopbReadMouse;
        g_methods[0x4be000] = &nfs3hp::loopbIsNumber;
        g_methods[0x4be570] = &nfs3hp::loopbModeSize;
        g_methods[0x423d90] = &nfs3hp::loopbNearestSpot;
        g_methods[0x4eb1fc] = &nfs3hp::loopbRand;
        g_methods[0x4f0430] = &nfs3hp::loopbAtoi;
        return true;
    }();
    (void)standInsAdded;

    constexpr x86::reg32 kCars = 0xb00000, kCarSize = 0x1000, kCarCount = 12;
    const auto someCar = [&] { return kCars + kCarSize * random(kCarCount); };
    // A float of real size, now and then 0, -0 or a NaN of its own.
    const auto someFloat = [&](x86::reg32 span, float over) {
        const x86::reg32 pick = random(16);
        if (pick == 0)
            return 0.0f;
        if (pick == 1)
            return -0.0f;
        if (pick == 2)
        {
            const x86::reg32 bits = 0x7fc00000 | random(0x400000);
            float nan;
            std::memcpy(&nan, &bits, 4);
            return nan;
        }
        const float size = float(random(span)) / over;
        return random(4) == 0 ? -size : size;
    };
    // An int from a small set, so that two cars tie now and then.
    const auto tying = [&](x86::reg32 span) { return random(4) == 0 ? 0 : random(span); };

    // The tick: around each number it is held against, or anywhere in a race.
    const x86::reg32 tickPick = random(8);
    const x86::reg32 tick = tickPick == 0 ? 0x1e + random(4)
        : tickPick == 1 ? 0x13e + random(4)
        : tickPick == 2 ? 0x8cb0 + random(0xf0)      // the last of the television's rows
        : tickPick == 3 ? 0x1fe + random(4)
        : 0x400 + random(0x8000);
    setWord32(0x7d3684, tick);
    setWord32(0x79c1a4, tick + random(0x40));
    setWord32(0x6fd3b0, random(6));
    const x86::reg32 slices = 100 + random(2000);
    setWord32(0x5dd940, slices);
    setWord32(0x6fd4b4, random(12) == 0 ? x86::reg32(-1) : random(10));
    setWord32(0x6fd4b8, random(4));
    setWord32(0x6fd4c0, random(2));
    setWord32(0x6fd518, random(5));
    setWord32(0x6fd520, random(5));
    setWord32(0x6fd4f4, random(3) == 0 ? random(8) : random(2));
    setWord32(0x6fd3a0, random(4));
    setWord32(0x6fd3b8, random(6) == 0 ? random(0) : random(5));
    setWord32(0x6fbc68, random(4));
    setWord32(0x6fbc4c, random(2));
    setWord32(0x7a3a70, random(4) == 0);
    setWord32(0x7a3d10, random(2));
    setWord32(0x559264, random(2));
    setWord32(0x5600fc, random(2));
    setWord32(0x55e88c, random(3) == 0);
    setWord32(0x563b30, random(2));
    setWord32(0x7d3680, random(2));
    setWord32(0x7d3694, random(2));
    setWord32(0x7d3698, random(4) == 0);
    setWord32(0x7d369c, random(4) == 0);
    setWord32(0x7d368c, random(3) == 0 ? 0 : random(0));
    setWord32(0x55fe3c, random(3) == 0 ? 0 : word32(0x79c1a4) + random(0x20) - 0x10);
    // A form's cockpit, and its mirror as fractions.
    for (x86::reg32 form = 0; form < 2; ++form)
    {
        setWord32(0x6fbc5c + 0x1a4 * form, random(2));
        for (x86::reg32 at = 0x6fbd04; at <= 0x6fbd10; at += 4)
            setFloat(at + 0x1a4 * form, someFloat(1000, 1000.0f));
    }
    // The strip.
    for (x86::reg32 at = 0x7497d8; at <= 0x7497e4; at += 4)
        setWord32(at, random(8) == 0 ? random(0) : random(700));
    // The pace's random numbers.
    setWord32(0x554f10, random(0x10000));
    setWord32(0x554f18, 1 + random(0x8000));
    // The replay: its mode, how far it is written and read, near the end now and then.
    setWord32(0x7cda58, random(10) == 0 ? x86::reg32(-1) : random(6));
    setWord32(0x7cda50, random(4) == 0 ? 0x1e000 - 0x80 - random(4) + 2 : random(0x1d000));
    setWord32(0x7cda54, random(0x1d000));
    {
        // Four packed rows there, 32 bytes each as the game packs them: more would run on
        // over the row into the counts at 0x7a3e30, which the game never has.
        x86::reg32 at = 0x7a5f48 + word32(0x7cda54);
        for (x86::reg32 row = 0; row < 4; ++row)
            for (x86::reg32 done = 0; done < 32;)
            {
                const x86::reg32 count = 1 + random(32 - done);
                if (random(2))
                {
                    g_memory[at++] = x86::reg8(count);
                    g_memory[at++] = x86::reg8(random(256));
                }
                else
                {
                    g_memory[at++] = x86::reg8(0 - count);
                    for (x86::reg32 i = 0; i < count; ++i)
                        g_memory[at++] = x86::reg8(random(256));
                }
                done += count;
            }
    }
    if (random(3) == 0)
        g_memory[0x7a5f48 + word32(0x7cda54)] = 0;
    for (x86::reg32 player = 0; player < 8; ++player)
    {
        setWord32(0x7a3e30 + 4 * player, random(3) == 0 ? 0 : random(3) == 0 ? 0x1f : random(0x21));
        // Controls in runs, as a car's are.
        for (x86::reg32 i = 0; i < 0x80; ++i)
            g_memory[0x7cd548 + 0x80 * player + i] = x86::reg8(random(3) == 0 ? random(256) : random(3));
    }
    setWord32(0x7cd968, random(2));
    setWord32(0x7cd968 + 116, random(2));
    setWord32(0x7cd954, random(2));
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord32(0x7a3dd0 + 4 * i, random(2) ? 0 : random(40));
    for (x86::reg32 i = 0; i < 300 * 16; ++i)
        if ((i & 7) == 0)
            setWord32(0x7c3f48 + 4 * i, random(20));
    // The cars.
    for (x86::reg32 c = 0; c < kCarCount; ++c)
    {
        const x86::reg32 car = kCars + kCarSize * c;
        // Where the run began and how far on, around the distances the times are of and
        // the end of the lap.
        static const x86::reg32 kDistances[] = { 0x42, 0x43, 0xa7, 0x10c };
        const x86::reg32 distance = kDistances[random(4)];
        const x86::reg32 runStart = random(3) == 0 ? distance + random(3) - 1 : random(3) == 0 ? random(0x120) : random(slices);
        const x86::reg32 slicePick = random(6);
        setWord32(car + 0x1c, slicePick == 0 ? random(slices)
            : slicePick == 1 ? slices - distance + random(3) - 1
            : slicePick == 2 ? slices - 1 + random(2)
            : slicePick == 3 ? runStart + distance + random(3) - 1
            : slicePick == 4 ? runStart + slices - distance + random(3) - 1
            : random(0x120));
        setWord32(car + 0x1f0, random(8));
        setWord32(car + 0x1f4, random(2));
        g_memory[car + 0x200] = x86::reg8(random(3) ? (random(256) & 0x2d) | 1 : random(256));
        setWord32(car + 0x204, random(3) == 0 ? word32(0x6fd4b4) : random(11) - 1);
        setWord32(car + 0x208, random(4));
        setWord32(car + 0x224, random(4) == 0 ? x86::reg32(0) - random(100) : tying(8) * slices);
        setWord32(car + 0x228, random(3) == 0 ? 0x1f0 + random(0x20) : random(2) ? tick - 0x1c0 + random(4) - 2 : tying(8) * 0x100);
        setWord32(car + 0x22c, random(3) == 0 ? word32(car + 0x1c) : random(slices));
        setWord32(car + 0x234, random(3) == 0 ? word32(car + 0x204) : random(11) - 1);
        setWord32(car + 0x238, tying(8) * 0x100);
        for (x86::reg32 lap = 0; lap < 12; ++lap)
        {
            setWord32(car + 0x240 + 4 * lap, random(3) == 0 ? 0 : 0x100 + random(0x4000));
            setFloat(car + 0x298 + 4 * lap, someFloat(1000, 8.0f));
        }
        setWord32(car + 0x260, runStart);
        // How long the run has been, around what the two long times must be at least.
        static const x86::reg32 kRuns[] = { 0x400, 0x580 };
        setWord32(car + 0x264, tick - (random(2) ? kRuns[random(2)] + random(3) - 1 : random(0x1000)));
        for (x86::reg32 at = 0x268; at <= 0x28c; at += 4)
            setWord32(car + at, random(2) ? 0 : random(0x1000));
        // Around what a time must be at least.
        static const x86::reg32 kLeast[] = { 0x2c0, 0x640, 0x480 };
        for (x86::reg32 at = 0x278; at <= 0x284; at += 4)
            if (random(3) == 0)
                setWord32(car + at, kLeast[random(3)] + random(3) - 1);
        setFloat(car + 0x2b8, someFloat(1000, 8.0f));
        setWord32(car + 0x2bc, random(4));
        setWord32(car + 0x2c4, random(3) == 0 ? 3 : random(3) == 0 ? 4 : random(6));
        setWord32(car + 0x2cc, random(4));
        // Around the speeds the performance times are of.
        static const float kSpeeds[] = { 0.0f, 0.25f, 0.5f, 0.75f, 26.0f, 26.79f, 26.8f, 27.7f, 27.8f, 44.3f, 44.5f, 44.64f, 44.7f, 80.0f };
        setFloat(car + 0x4f4, random(5) == 0 ? someFloat(1000, 8.0f) : (random(4) == 0 ? -1.0f : 1.0f) * kSpeeds[random(std::size(kSpeeds))]);
        // Or on the bits of a speed the times are of.
        static const x86::reg32 kSpeedBits[] = { 0x41d651ec, 0x41dde76d, 0x42328f5c, 0x4231851f, 0x3f000000 };
        if (random(4) == 0)
            setWord32(car + 0x4f4, kSpeedBits[random(std::size(kSpeedBits))] + random(3) - 1);
        g_memory[car + 0x50b] = x86::reg8(random(3) ? 1 : random(3));
        setWord32(car + 0x528, someCar());
        setFloat(car + 0x1cc, someFloat(1000, 8.0f));
        g_memory[car + 0x164] = x86::reg8(0x1e + random(5));
        g_memory[car + 0x165] = 0;
        setWord32(car + 0x8d4, random(4) == 0);
    }
    // The lists, and their counts.
    for (const x86::reg32 list : { 0x5efa48u, 0x5efac8u, 0x5efb08u, 0x5efb88u, 0x5efc08u, 0x5efc48u, 0x5efd08u })
        for (x86::reg32 i = 0; i < 16; ++i)
            setWord32(list + 4 * i, someCar());
    for (const x86::reg32 count : { 0x5efda4u, 0x5efd9cu, 0x5efd98u, 0x5efd94u, 0x553078u, 0x5efda0u, 0x553074u })
        setWord32(count, random(12) == 0 ? x86::reg32(-1) : random(3) == 0 ? 0 : 1 + random(random(2) ? 2 : 8));
    setWord32(0x5612f0, random(9));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        // Who has left a network's race, a word by driver.
        g_memory[0x7a22ca + 2 * i] = x86::reg8(random(3) == 0);
        g_memory[0x7a22cb + 2 * i] = 0;
        setWord32(0x5612b0 + 4 * i, someCar());
    }
    setWord32(0x5efdfc, random(0x1000));
}
