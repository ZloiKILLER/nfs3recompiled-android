// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What these functions call and the check's table lacks: the registers each reads,
    // its stack arguments, what its ret pops, what it changes besides eax (read off nfs3.exe).
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto plain = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0, std::uint8_t clobbers = 0) {
            specs[at] = { in, stack, pops, clobbers, Kind::Plain };
        };
        plain(0x41e940, EAX | EDX);
        plain(0x41f6e0, EAX | EBX | EDX, 0, 0, EBX | EDX);
        plain(0x420230, EAX | EDX | ESI, 0, 0, EDX | ESI);
        plain(0x4207e0, EAX | EDX);
        specs[0x420bf0] = { 0, 4, 16, 0, Kind::Bool };
        plain(0x4296c0, EAX, 0, 0, ESI | EDI);
        specs[0x42ee10] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Bool };
        plain(0x42efe0, EAX | EBX | EDX, 1, 4, EBX | EDX);
        plain(0x4972f0, EAX);
        specs[0x49fde0] = { EAX, 0, 0, 0, Kind::Bool };
        plain(0x49ffa0, EAX | EDX);
        plain(0x4a0440, EAX);
        plain(0x4a08c0, EAX);
        plain(0x4a0980, EAX | EDX, 0, 0, EDX);
        plain(0x4a0a10, EAX | EDX, 0, 0, EDX);
        plain(0x4a0f30, EAX | EDX, 1, 4, EDX);
        plain(0x4a1090, EAX);
        plain(0x4a20c0, EAX);
        plain(0x4a2e20, EAX | EDX, 0, 0, EDX);
        plain(0x4b7680, EAX | EDX, 2, 8, EDX | EDI);
        plain(0x4dfd80, EAX, 1, 4);
        plain(0x4dfde0, EAX, 1, 4);
        plain(0x4dfe40, EAX, 1, 4);
        plain(0x4dffb0, EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX);
        plain(0x4e0000, EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX);
        plain(0x4e0050, EAX | EDX, 2, 8, EDX);
        plain(0x4e0110, EAX | EDX);
        plain(0x4e01f0, EAX | EDX);
        plain(0x4e0280, EAX | EBX | EDX, 0, 0, EDX);
        plain(0x4e0430, EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX);
        // Stand-ins that do more than kSpecs' kinds: simb_hooks.inc.
        g_methods[0x4e01f0] = &nfs3hp::simb::dot;
        g_methods[0x4972f0] = &nfs3hp::simb::normalise;
        g_methods[0x4e0110] = &nfs3hp::simb::transpose;
        g_methods[0x4e0000] = &nfs3hp::simb::addOrSubtract<true>;
        g_methods[0x4dffb0] = &nfs3hp::simb::addOrSubtract<false>;
        g_methods[0x4e0050] = &nfs3hp::simb::scale;
        g_methods[0x4e0430] = &nfs3hp::simb::transformPoints;
        g_methods[0x41f6e0] = &nfs3hp::simb::updatePosition;
        g_methods[0x4207e0] = &nfs3hp::simb::positionSurface;
        g_methods[0x420230] = &nfs3hp::simb::positionFrame;
        g_methods[0x42ee10] = &nfs3hp::simb::hitWall;
        g_methods[0x42efe0] = &nfs3hp::simb::boxHitsWall;
        g_methods[0x4b7680] = &nfs3hp::simb::blendFrames;
        g_methods[0x4dfd80] = &nfs3hp::simb::rotation<0x4dfd80>;
        g_methods[0x4dfde0] = &nfs3hp::simb::rotation<0x4dfde0>;
        g_methods[0x4dfe40] = &nfs3hp::simb::rotation<0x4dfe40>;
        g_methods[0x4e0280] = &nfs3hp::simb::multiplyMatrices;
        g_methods[0x4a0440] = &nfs3hp::simb::roadRise;
        g_methods[0x4a0f30] = &nfs3hp::simb::followFrame;
        return true;
    }();
    (void)standInsAdded;

    // Real magnitudes, with the noise of the watched range left through one in ten, and the
    // values the code compares with (0, -0, its 0.9, 0.75, 0.5, 0.02) met often enough.
    const auto realFloat = [&]() -> float {
        static const float kSpecial[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.02f, -0.02f, 0.9f, 0.6f };
        const x86::reg32 pick = random(8);
        if (pick == 0)
            return kSpecial[random(10)];
        if (pick <= 2)
            return float(x86::sreg32(random(2000)) - 1000) / 8192.0f;
        // All of a float's bits, or the sums and products would have nothing to round.
        return float(x86::sreg32(random(0x2000000)) - 0x1000000) / 65536.0f;
    };
    const auto modest = [&]() -> float { return float(x86::sreg32(random(0x1000000)) - 0x800000) / float(0x800000); };

    constexpr x86::reg32 kCar = 0x900000;
    for (x86::reg32 at = kCar; at < kCar + 0x600; at += 4)
        if (random(10) != 0)
            setFloat(at, realFloat());
    // A NaN somewhere, for whichever operand the sums take its bits from.
    for (x86::reg32 i = random(4); i > 0; --i)
        setWord32(kCar + 4 * random(0x180), (random(2) ? 0x7fc00000u : 0xffc00000u) | random(0x3fffff));
    setWord32(kCar + 0x1c, random(64));
    g_memory[kCar + 0x88] = x86::reg8(random(4) == 0);
    g_memory[kCar + 0x89] = x86::reg8(random(4) != 0);
    // Its axes, with parts that are small: the limits on its tilt and spin come into it.
    for (const x86::reg32 at : { 0xc0u, 0xc4u, 0xc8u, 0xccu, 0xd0u, 0xd4u, 0xd8u, 0xdcu, 0xe0u, 0xa4u, 0xa8u, 0xacu })
        if (random(4) != 0)
            setFloat(kCar + at, modest());
    for (const x86::reg32 at : { 0xe4u, 0xe8u, 0xecu })
        if (random(3) != 0)
            setFloat(kCar + at, modest() / 2.0f);
    static const float kUpright[] = { 0.95f, 0.91f, 0.9f, 0.85f, 0.76f, 0.75f, 0.7f, -0.95f, -0.5f, 1.0f, 0.0f, -0.0f };
    if (random(8) != 0)
        setFloat(kCar + 0x10c, kUpright[random(12)]);
    setFloat(kCar + 0xb8, random(4) == 0 ? realFloat() : random(3) == 0 ? 10.0f : float(random(40)));
    // Its bits are compared as an integer: a NaN's say more than 10 where the x87 would not.
    if (random(16) == 0)
        setWord32(kCar + 0xb8, 0x7fc00000);
    if (random(4) != 0)
        setFloat(kCar + 0x15c, random(2) ? (random(2) ? 0.0f : -0.0f) : modest());
    g_memory[kCar + 0x164] = x86::reg8(random(3) != 0);
    g_memory[kCar + 0x165] = 0;
    setWord32(kCar + 0x18c, random(0x100));
    g_memory[kCar + 0x200] = x86::reg8(random(256));
    setWord32(kCar + 0x220, 0x903000);
    setWord32(kCar + 0x528, random(8) == 0 ? 0 : 0x902000);

    // Its handling and tuning.
    for (x86::reg32 at = 0x902000; at < 0x902400; at += 4)
        if (random(10) != 0)
            setFloat(at, modest());
    for (x86::reg32 at = 0x903000; at < 0x903100; at += 4)
        if (random(10) != 0)
            setFloat(at, modest());
    // The two that make the bounce, past its limit of 0.2 as often as not.
    setFloat(0x902000 + 0x2e0, modest() * (random(2) ? 8.0f : 1.0f));
    setFloat(0x903000 + 0x50, modest() * (random(2) ? 8.0f : 1.0f));

    // The road: its nodes' points in 16.16, all else their noise; its roofs by node.
    setWord32(0x5dd948, 0x904000);
    for (x86::reg32 node = 0; node < 64; ++node)
        for (x86::reg32 i = 0; i < 3; ++i)
            if (random(8) != 0)
                setWord32(0x904000 + 36 * node + 4 * i, x86::reg32(random(0x2000000)) - 0x1000000);
    setWord32(0x552e54, 0x905000);
    for (x86::reg32 node = 0; node < 64; ++node)
        if (random(10) != 0)
            setFloat(0x905000 + 4 * node, realFloat());

    setFloat(0x5efdd8, random(3) ? 0.9f : realFloat());
    setFloat(0x5efddc, random(3) ? -0.5f : realFloat());

    // No signalling NaN: nothing the x87 computes is one, and the generated code's own results
    // for one hang on what the compiler folds (fld, fchs and fstp, into a change of sign).
    for (x86::reg32 at = 0x900000; at < 0x90a000; at += 4)
    {
        const x86::reg32 value = word32(at);
        if ((value & 0x7fc00000) == 0x7f800000 && (value & 0x3fffff) != 0)
            setWord32(at, value | 0x400000);
    }
}
