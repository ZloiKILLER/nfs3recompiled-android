// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x420bf0] = { 0, 4, 16, 0, Kind::Bool };                 // in a tunnel
        specs[0x4eb390] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Bool };  // a name compared
        specs[0x7e0200] = { 0, 2, 8, 0, Kind::Bool };                  // THRASH_setstate, by its pointer
        using namespace nfs3hp;
        const auto own = [&](x86::reg32 at, win32::MethodPtr method) {
            specs[rendd::kOwn | at] = specs.at(at);
            specs.erase(at);
            g_methods[at] = method;
        };
        // Stand-ins that do more than kSpecs' kinds: rendd_hooks.inc.
        own(0x4e01f0, &rendd::dot);
        own(0x4972f0, &rendd::normalise);
        own(0x4965b0, &rendd::distance);
        own(0x4968a0, &rendd::onX87<0x4968a0>);
        own(0x42cff0, &rendd::onX87<0x42cff0>);
        own(0x4eb230, &rendd::onX87<0x4eb230>);
        own(0x49de20, &rendd::modelPart);
        own(0x4e0430, &rendd::transformPoints);
        own(0x4e0000, &rendd::twoVectors<0x4e0000>);
        own(0x4dffb0, &rendd::twoVectors<0x4dffb0>);
        own(0x4e0050, &rendd::scaleVector);
        own(0x4e01c0, &rendd::normaliseTo);
        own(0x4e0210, &rendd::dotWithSun);
        own(0x420650, &rendd::trackPoint);
        own(0x477d90, &rendd::replayPose);
        own(0x49c880, &rendd::colours<0x49c880>);
        own(0x49ca50, &rendd::colours<0x49ca50>);
        own(0x4dfd80, &rendd::rotation<0x4dfd80>);
        own(0x4dfde0, &rendd::rotation<0x4dfde0>);
        own(0x4dfe40, &rendd::rotation<0x4dfe40>);
        own(0x4e0280, &rendd::multiplyMatrices);
        own(0x422ba0, &rendd::cameraCar);
        own(0x491120, &rendd::lightColour);
        own(0x496550, &rendd::lightDistance);
        own(0x491bc0, &rendd::flareRecord);
        own(0x4bb5d0, &rendd::carVisible);
        own(0x49c470, &rendd::endModels);
        return true;
    }();
    (void)standInsAdded;

    // Floats of a size, and now and then what the noise makes of them.
    const auto real = [&](x86::reg32 at, x86::reg32 count) {
        for (x86::reg32 i = 0; i < count; ++i)
        {
            const x86::reg32 pick = random(8);
            if (pick == 0)
                continue;
            static const float kSpecial[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.25f, 30.0f, 31.0f, 60.0f };
            setFloat(at + 4 * i, pick == 1 ? kSpecial[random(std::size(kSpecial))]
                                 : pick <= 3 ? float(x86::sreg32(random(4096)) - 2048) / 1024.0f
                                             : float(x86::sreg32(random(0x2000000)) - 0x1000000) / 65536.0f);
        }
    };

    // The cars: 8 of 0xa00 bytes, in the table.  Their turns must end (the loops in sub_4b9140).
    for (x86::reg32 k = 0; k < 8; ++k)
    {
        const x86::reg32 car = 0x900000 + 0xa00 * k;
        setWord32(car + 0x8a4, random(6) == 0 ? 0 : 0x910000 + 0x1000 * random(4));
        setWord32(car + 0x8b4, x86::reg32(random(6)) - 1);
        real(car + 0x8b8, 1);
        setFloat(car + 0x934, float(x86::sreg32(random(64)) - 32) / 8.0f);
        setFloat(car + 0x938, float(x86::sreg32(random(64)) - 32) / 8.0f);
        setFloat(car + 0x4f4, float(x86::sreg32(random(2048)) - 1024) / 16.0f);
        setWord32(car + 0x594, random(4) == 0 ? random(0) : x86::reg32(random(200)) - 100);
        setWord32(car + 0x598, random(4) == 0 ? random(0) : x86::reg32(random(200)) - 100);
        g_memory[car + 0x59c] = x86::reg8(random(4));
        g_memory[car + 0x50d] = x86::reg8(random(2));
        setWord32(car + 0x51c, random(3) == 0 ? 0 : x86::reg32(random(40)) - 20);
        setWord32(car + 0x1c, random(0x100));
        setWord32(car + 0x184, random(5) == 0);
        setWord32(car + 0x20, random(5) != 0);
        g_memory[car + 0x200] = x86::reg8(random(6) == 0 ? random(256) : random(256) & ~0x40u);
        setWord32(car + 0x734, random(3) == 0 ? 0 : random(0));
        setWord32(car + 0x660, random(3));
        g_memory[car + 0x509] = x86::reg8(random(2));
        setWord32(car + 0x8e4, random(2));
        setWord32(car + 0x8e8, random(2));
        g_memory[car + 0x88] = x86::reg8(random(4) == 0);
        g_memory[car + 0x89] = x86::reg8(random(4) != 0);
        setWord32(car + 0x174, random(2));
        setWord32(car + 0x9a4, random(3) == 0);
        setWord32(car + 0x2c4, random(5));
        for (const x86::reg32 at : { 0x314u, 0x318u, 0x38cu, 0x390u, 0x404u, 0x408u, 0x47cu, 0x480u })
            setWord32(car + at, random(4) == 0 ? 0 : 1);
        real(car + 0x98, 3 + 9 + 3 + 3);
        real(car + 0xb8, 1);
        real(car + 0xd0, 1);
        real(car + 0xd8, 3);
        real(car + 0x10c, 1);
        real(car + 0x114, 3);
        real(car + 0x118, 2);
        real(car + 0x148, 3);
        real(car + 0x920, 2);
        real(car + 0x930, 1);
        setWord32(car + 0x8b0, random(4) == 0 ? random(0) : random(0x100));   // what sub_4ba350 takes off the brightness
        setWord32(0x5efac8 + 4 * k, car);
    }
    setWord32(0x5efd9c, random(5) == 0 ? 0 : 1 + random(8));
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x5efa48 + 4 * i, 0x900000 + 0xa00 * random(8));
    setWord32(0x6fd4f4, random(4));

    // The models: parts around each detail's least, their lights named by letters.
    for (x86::reg32 k = 0; k < 4; ++k)
    {
        const x86::reg32 model = 0x910000 + 0x1000 * k;
        fillRandom(model, model + 0x1000);
        static const x86::reg32 kParts[] = { 0, 5, 6, 10, 11, 12, 13 };
        setWord32(model + 0xf8, kParts[random(std::size(kParts))]);
        setWord32(model + 0x34, random(7));
        static const x86::reg8 kFirst[] = { 'T', 'H', 'S', 'X' };
        static const x86::reg8 kSide[] = { 'R', 'F', 'X' };
        static const x86::reg8 kThird[] = { 'L', 'X' };
        static const x86::reg8 kFourth[] = { 0, 'E', 'O' };
        static const x86::reg8 kFifth[] = { 'N', 'Y' };
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            const x86::reg32 name = model + 0xa04 + 0x40 * i;
            g_memory[name] = kFirst[random(std::size(kFirst))];
            g_memory[name + 1] = kSide[random(std::size(kSide))];
            g_memory[name + 2] = kThird[random(std::size(kThird))];
            g_memory[name + 3] = kFourth[random(std::size(kFourth))];
            g_memory[name + 4] = kFifth[random(std::size(kFifth))];
        }
    }
    setWord32(0x55fe24, random(4) == 0 ? 0 : 0x910000 + 0x1000 * random(4));
    setWord32(0x55d000, random(2));

    // The views.
    for (x86::reg32 k = 0; k < 4; ++k)
    {
        const x86::reg32 view = 0x930000 + 0x100 * k;
        fillRandom(view, view + 0x100);
        setWord32(view, random(4) == 0 ? 1 : random(3));
        setWord32(view + 4, 0x930000 + 0x100 * random(4));
        real(view + 8, 3);
        real(view + 0x38, 3 + 9);
    }
    setWord32(0x563a90, 0x930000 + 0x100 * random(4));

    // The arena: a block, its next byte, room or little.
    const x86::reg32 taken = 0x20 * random(0x40);
    setWord32(0x55fe34, 0x920000);
    setWord32(0x55fe38, 0x920000 + taken);
    setWord32(0x7a3d04, random(4) == 0 ? random(0x100) : random(4) == 0 ? taken : 0x8000);
    setWord32(0x7a3d08, random(3) == 0 ? 0 : 0x950800 + 0x20 * random(0x20));
    setWord32(0x7a3d0c, random(2) ? 0 : 0x950800);

    // The settings and what they compare with.
    setWord32(0x6fbc1c, random(3));
    for (x86::reg32 at = 0x55fdf4; at < 0x55fe24; at += 4)
        setFloat(at, float(random(0x1000)) / 4096.0f * float(1 + random(4)));
    for (const x86::reg32 at : { 0x6fbc20u, 0x6fbc24u, 0x6fd4c8u, 0x6fd4ccu, 0x554e4cu })
        setWord32(at, random(2));
    setWord32(0x7a3a60, random(2));
    setWord32(0x7d36a0, random(2));
    setFloat(0x7dcfe0, float(x86::sreg32(random(2048)) - 1024) / 64.0f);
    setFloat(0x56009c, float(random(0x10000)) / 64.0f);
    setFloat(0x5600a0, float(random(0x10000)) / 64.0f);
    setFloat(0x5600a4, float(random(1280)) / 2.0f);
    setFloat(0x5600a8, float(random(960)) / 2.0f);

    // The clip's: over a float's bits, as the game's.
    static const float kNearest[] = { 1.0f, 0.5f, 100.0f };
    setFloat(0x7d34f8, kNearest[random(3)]);
    setFloat(0x7d34fc, float(random(64)));
    setFloat(0x7d350c, float(320 + random(320)));
    setFloat(0x7d3504, float(random(64)));
    setFloat(0x7d3500, float(240 + random(240)));

    // Points that land on the clip's edges exactly: no scale, the centre on them.
    if (random(6) == 0)
    {
        setWord32(0x56009c, 0);
        setWord32(0x5600a0, 0);
        setWord32(0x5600a4, word32(random(2) ? 0x7d34fc : 0x7d350c));
        setWord32(0x5600a8, word32(random(2) ? 0x7d3504 : 0x7d3500));
    }

    // The player's model, or none; the driver's switches, one at a time often enough to reach each.
    setWord32(0x55fe2c, random(3) == 0 ? 0 : random(0));
    g_memory[0x55eb2c] = x86::reg8(random(3) == 0 ? random(256) : random(2) ? 1u << random(8) : 0u);
    g_memory[0x55eb2d] = x86::reg8(random(4) == 0 ? 1u << random(2) : 0u);

    // The lights along the track: a count and where they are, every 8 segments.
    setWord32(0x5dd940, 8 + random(0x100));
    for (x86::reg32 block = 0; block < 0x28; ++block)
    {
        const x86::reg32 at = 0x571370 + 0x5c0 * block;
        setWord32(at + 0x5b8, random(3) == 0 ? 0 : random(4));
        setWord32(at + 0x5bc, 0x950000 + 0x10 * random(0x40));
    }

    // Records and surfaces.
    for (x86::reg32 at = 0x950000; at < 0x951000; at += 4)
        setWord32(at, random(0));

    // Where the functions are handed places to fill, read points from.
    setWord32(0x9ef96c, 0x7e0200);
    real(0x961000, 3 * 8);
    real(0x962000, 9);
}
