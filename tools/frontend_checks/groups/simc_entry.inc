// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    // The scratch's quaternions and matrices, an output now and then one of the inputs.
    constexpr x86::reg32 kFirst = 0x900000, kSecond = 0x900080, kThird = 0x900100, kFourth = 0x900180, kOut = 0x900200, kOut2 = 0x900280;
    const auto stackWord = [&](unsigned place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto stackFloat = [&](unsigned place) {
        setFloat(entry.esp + 4 + 4 * place, random(4) == 0 ? (random(2) ? 0.0f : 1.0f)
                                            : random(8) == 0 ? nfs3hp::simcSomeFloat(g_random, true)
                                                             : float(random(2049)) / 2048.0f);
    };
    const auto outOr = [&](x86::reg32 out, std::initializer_list<x86::reg32> inputs) {
        if (random(8) != 0)
            return out;
        return *(inputs.begin() + random(x86::reg32(inputs.size())));
    };
    const auto player = [&]() { return random(2); };
    // Values at a float's ends, for the x87's wider range to show: a product past a float's,
    // or under it, that the game stores in a float and reads back, or does not.
    const auto extremes = [&](x86::reg32 at, x86::reg32 count) {
        for (x86::reg32 i = 0; i < count; ++i)
        {
            const float sign = random(2) ? 1.0f : -1.0f;
            const unsigned pick = random(4);
            setFloat(at + 4 * i, sign * (pick < 2 ? float(100 + random(240)) * 1.0e36f : pick == 2 ? float(1 + random(1000)) * 1.0e-28f : 1.5f));
        }
    };
    // The replay's table of events: what the director's switch is on, 3 to 9 and past them.
    const auto simcEvents = [&]() {
        for (x86::reg32 row = 0; row < 600; ++row)
            for (x86::reg32 column = 0; column < 16; ++column)
                setWord32(0x7c3f48 + (row << 6) + 4 * column, random(32) == 0 ? random(0) : random(12));
    };
    switch (c.address)
    {
    case 0x4b70f0:
        entry.eax = kFirst, entry.edx = kSecond, entry.ebx = kThird, entry.ecx = kFourth;
        stackFloat(0);
        stackWord(1, outOr(kOut, { kFirst, kSecond }));
        stackWord(2, outOr(kOut2, { kThird, kFourth }));
        break;
    case 0x4b71e0:
        entry.eax = kFirst, entry.edx = kSecond;
        if (random(4) != 0)
        {
            nfs3hp::simcUnitQuat(g_random, kFirst);
            nfs3hp::simcQuatLike(g_random, kSecond, kFirst);
        }
        stackFloat(0);
        stackWord(1, outOr(kOut, { kFirst, kSecond }));
        break;
    case 0x4b7300:
        entry.eax = random(16) == 0 ? kSecond : kFirst, entry.edx = kSecond;
        if (random(2) != 0)
        {
            nfs3hp::simcUnitQuat(g_random, kFirst);
            nfs3hp::simcQuatLike(g_random, kSecond, kFirst);
        }
        else if (random(2) != 0)
        {
            extremes(kFirst, 4);
            extremes(kSecond, 4);
        }
        break;
    case 0x4b73c0:
    case 0x4b7590:
        entry.eax = kFirst;
        entry.edx = outOr(kOut, { kFirst });
        if (c.address == 0x4b7590 && random(2) != 0)
            nfs3hp::simcUnitQuat(g_random, kFirst);
        else if (random(2) != 0)
            extremes(kFirst, 9);
        break;
    case 0x4b7680:
        entry.eax = kFirst, entry.edx = kSecond;
        stackFloat(0);
        stackWord(1, outOr(kOut, { kFirst, kSecond }));
        break;
    case 0x4b7720:
    case 0x4b7780:
        entry.eax = kFirst, entry.edx = kSecond;
        entry.ebx = outOr(kOut, { kFirst, kSecond });
        break;
    case 0x4b77e0:
        entry.eax = kFirst;
        stackFloat(0);
        stackWord(1, outOr(kOut, { kFirst }));
        break;
    case 0x4b7840:
        entry.eax = kFirst;
        entry.edx = random(3) == 0 ? random(12) : 10 + random(12);
        break;
    case 0x4b79c0:
        entry.eax = kFirst;
        // Signalling NaNs: fld and fstp quiet them, a move of the bits would not.
        for (x86::reg32 at = 0xc; at < 0x24; at += 4)
            if (random(4) == 0)
                setWord32(kFirst + at, random(2) ? 0x7f800001 : 0xff812345);
        break;
    case 0x4ea680:
    case 0x4ea820:
        entry.eax = kFirst;
        stackFloat(0);
        stackWord(1, outOr(random(3) == 0 ? kFirst : kOut, { kFirst }));
        break;
    case 0x4ea920:
        nfs3hp::simcFloats(g_random, entry.esp + 4, 1, random(4) == 0);
        if (random(2) != 0)
            setFloat(entry.esp + 4, float(random(100000)) / 64.0f);
        break;
    case 0x4bc470:
        entry.eax = random(4);
        break;
    case 0x4dbde0:
    {
        setWord32(0x900200, random(3) == 0 ? 1 : random(3) + 1);
        setWord32(0x900204, random(2));
        entry.eax = 0x900200;
        setWord32(0x6fd4c4, random(2));
        break;
    }
    case 0x4bdb60:
    {
        entry.eax = player();
        static const x86::reg32 kCameras[] = { 9, 9, 18, 3, 7, 12 };
        // Now and then the camera the car has already.
        const x86::reg32 replayCar = word32(0x5e10b8 + 4 * entry.eax);
        const x86::reg32 car = replayCar != 0 ? replayCar : word32(0x5e10b0 + 4 * entry.eax);
        entry.edx = random(3) == 0 && car != 0 ? word32(car) : kCameras[random(6)];
        break;
    }
    case 0x4bdbd0:
    case 0x4bdc70:
        entry.eax = player();
        break;
    case 0x4bdd70:
        entry.eax = player();
        simcEvents();
        break;
    case 0x4bdcc0:
        entry.eax = player();
        entry.edx = random(2) ? 18 : random(25);
        break;
    case 0x4bdd10:
        simcEvents();
        entry.eax = random(16);
        entry.edx = random(4) == 0 ? 598 + random(4) : random(600);
        entry.ebx = random(5);
        break;
    case 0x4be000:
    {
        static const x86::reg8 kEdges[] = { '/', '0', '5', '9', ':', '+', '-', ',', '.', 0, 0xb0, 0xb9, 0x80 };
        entry.eax = (entry.eax & 0xffffff00u) | (random(4) == 0 ? random(256) : kEdges[random(13)]);
        break;
    }
    case 0x4be1a0:
    {
        // 32 / eax is what the second divide is by: never nothing.
        static const x86::reg32 kRates[] = { 1, 2, 4, 8, 16, 32, 3, 5, x86::reg32(-1), x86::reg32(-32) };
        entry.eax = kRates[random(10)];
        entry.edx = random(3) == 0 ? random(0) : random(64);
        setWord32(0x7d3684, random(3) == 0 ? random(0) : random(100000));
        break;
    }
    case 0x4c9dd0:
    {
        entry.eax = random(4);
        entry.edx = random(16);
        entry.ebx = random(14);
        break;
    }
    case 0x4c9f50:
        entry.eax = random(4);
        break;
    case 0x4ea570:
    {
        static const x86::reg32 kEdges[] = { 0, 1, x86::reg32(-1), 0x80000000u, 0x7fffffffu };
        entry.eax = random(4) == 0 ? kEdges[random(5)] : x86::reg32(int(random(200000)) - 100000);
        entry.edx = random(4) == 0 ? (random(2) ? entry.eax : 0 - entry.eax)
                                   : random(4) == 0 ? kEdges[random(5)] : x86::reg32(int(random(200000)) - 100000);
        break;
    }
    case 0x4ea654:
    case 0x4ea658:
    {
        entry.eax = 0x900300;
        setWord32(0x900308, random(8));
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(0x900300 + 0x14 + i * 8, random(0x10000));
        entry.edx = random(12);
        break;
    }
    case 0x43bcc0:
        entry.eax = random(64);
        break;
    case 0x465320:
    {
        entry.eax = poolItem(random(kPoolItems));
        break;
    }
    default:
        break;
    }
}
