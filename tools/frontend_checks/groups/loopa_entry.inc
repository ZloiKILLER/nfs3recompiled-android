// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto bitsOf = [](float value) {
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        return bits;
    };
    // The bits of a float of real size, 0 or a NaN of its own now and then: the order of an
    // fp:: shows only between two NaNs.
    const auto someBits = [&](x86::reg32 span, float over) {
        const x86::reg32 kind = random(16);
        if (kind == 0)
            return x86::reg32(0);
        if (kind == 1)
            return 0x7fc00000 | random(0x400000);
        if (kind == 2)
            return random(0);
        return bitsOf(float(x86::sreg32(random(span)) - x86::sreg32(span / 2)) / over);
    };
    constexpr x86::reg32 kOut = 0xb00000, kRecord = 0xb01000, kOther = 0xb01100, kPlaceA = 0xb01200, kPlaceB = 0xb01300;
    constexpr x86::reg32 kMatrix = 0xb01400, kCar = 0xb02000;
    // A rotation of four words: near unit, or anything.
    const auto someRotation = [&](x86::reg32 at) {
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            const x86::reg32 word = random(3) == 0 ? random(0x10000) : x86::reg32(x86::sreg32(random(0x8000)) - 0x4000);
            g_memory[at + 2 * i] = x86::reg8(word);
            g_memory[at + 2 * i + 1] = x86::reg8(word >> 8);
        }
    };
    // The nine parts of a matrix, 16.16: the sizes of a rotation with any signs, so that every
    // way through the trace and the diagonal is taken, or anything.
    const auto someMatrix = [&](x86::reg32 at) {
        const bool junk = random(8) == 0;
        for (x86::reg32 i = 0; i < 9; ++i)
            setWord32(at + 4 * i, junk ? random(0) : x86::reg32(x86::sreg32(random(0x20000)) - 0x10000));
        if (!junk && random(2))
            setWord32(at + 4 * random(3) * 4, 0x10000 - random(0x1000));
        // The one way to the root of 0: a diagonal whose largest less the others is exactly
        // -1, its trace wrapped below 0.
        if (random(12) == 0)
        {
            const x86::reg32 largest = random(3);
            for (x86::reg32 i = 0; i < 3; ++i)
                setWord32(at + 16 * i, i == largest ? 0x5fff0000 : 0x30000000);
        }
    };
    const auto somePlace = [&](x86::reg32 at) {
        for (x86::reg32 i = 0; i < 3; ++i)
            setWord32(at + 4 * i, random(4) == 0 ? random(0) : x86::reg32(x86::sreg32(random(0x4000000)) - 0x2000000));
    };
    const auto somePose = [&](x86::reg32 at) {
        g_memory[at + 2] = x86::reg8(random(64));
        g_memory[at + 3] = 0;
        for (x86::reg32 i = 4; i < 10; ++i)
            g_memory[at + i] = x86::reg8(random(256));
        someRotation(at + 10);
    };
    const auto someFraction = [&]() -> x86::reg32 {
        const x86::reg32 kind = random(8);
        return kind == 0 ? 0 : kind == 1 ? 0x10000 : kind == 2 ? random(0) : random(0x10000);
    };
    switch (c.address)
    {
    case 0x459410:
        entry.eax = kRecord;
        setWord32(kRecord + 4, random(0));
        setWord32(kRecord + 6, random(0));
        setWord32(kRecord + 0x60, random(0));
        setWord32(kRecord + 0x64, random(0));
        break;
    case 0x4777d0:
    case 0x477b50:
        entry.eax = random(6) == 0 ? 0 : kOther;
        break;
    case 0x477830:
        entry.eax = kRecord;
        entry.edx = kMatrix;
        someMatrix(kMatrix);
        break;
    case 0x477850:
        entry.eax = kRecord;
        entry.esi = kOut;
        somePose(kRecord);
        break;
    case 0x4778c0:
        entry.eax = kRecord;
        argument(0, someBits(0x400000, 64.0f));
        argument(1, someBits(0x400000, 64.0f));
        argument(2, someBits(0x400000, 64.0f));
        argument(3, random(6) == 0 ? random(0) : random(64));
        argument(4, random(0));
        break;
    case 0x477990:
        entry.eax = kRecord;
        entry.edx = kOther;
        entry.ebx = someFraction();
        entry.ecx = kOut;
        argument(0, kOut + 0x40);
        somePose(kRecord);
        somePose(kOther);
        break;
    case 0x4779e0:
    case 0x477a10:
    {
        const x86::reg32 kind = random(6);
        entry.eax = kind == 0 ? 0 - random(16) : kind == 1 ? 0x3840 - 2 + random(4) : kind == 2 ? random(0) : random(0x100);
        // Round the end of the ghost, where sub_477a10 turns back.
        if (c.address == 0x477a10 && random(2))
            entry.eax = word32(0x6bc6b4) - 3 + random(6);
        break;
    }
    case 0x477c20:
        entry.eax = kCar;
        for (x86::reg32 at = 0x98; at < 0xa4; at += 4)
            setWord32(kCar + at, someBits(0x400000, 64.0f));
        for (x86::reg32 at = 0xc0; at < 0xe4; at += 4)
            setWord32(kCar + at, someBits(0x20000, 65536.0f));
        setWord32(kCar + 6, (random(64) << 16) | random(0x10000));
        setWord32(kCar + 0x1a, (random(64) << 16) | random(0x10000));
        break;
    case 0x477d90:
        entry.eax = kOut;
        entry.edx = kOut + 0x40;
        break;
    case 0x495190:
        entry.eax = kRecord;
        entry.edx = kOther;
        entry.ebx = kPlaceA;
        entry.ecx = kPlaceB;
        for (const x86::reg32 corner : { kRecord, kOther, kPlaceA, kPlaceB })
        {
            setWord32(corner, someBits(1280, 2.0f));
            setWord32(corner + 4, someBits(960, 2.0f));
        }
        break;
    case 0x4a3690:
        entry.eax = random(4) == 0 ? random(0) : random(3);
        entry.edx = random(2) ? 0 : random(0);
        // What the last session left, which this one clears before its first call.
        setWord32(0x55e88c, random(3));
        break;
    case 0x4a7080:
    case 0x4a7310:
    {
        // Now and then exactly on a bound of the clipper: which side of it that counts as.
        const auto onBound = [&](x86::reg32 bound) {
            float value;
            const x86::reg32 bits = word32(bound);
            std::memcpy(&value, &bits, 4);
            return x86::reg32(x86::sreg32(value));
        };
        const auto coordinate = [&] {
            static const x86::reg32 kBounds[] = { 0x7d34fc, 0x7d350c, 0x7d3504, 0x7d3500 };
            const x86::reg32 kind = random(8);
            return kind == 0 ? random(0) : kind < 4 ? onBound(kBounds[random(4)]) : x86::reg32(x86::sreg32(random(1400)) - 200);
        };
        entry.eax = coordinate();
        entry.edx = coordinate();
        entry.ebx = coordinate();
        entry.ecx = coordinate();
        for (x86::reg32 i = 0; i < 4; ++i)
            argument(i, coordinate());
        for (x86::reg32 i = 4; i < 14; ++i)
            argument(i, random(0));
        break;
    }
    case 0x4b6a30:
        entry.eax = kRecord;
        entry.edx = kOther;
        entry.ebx = kPlaceA;
        entry.ecx = kPlaceB;
        argument(0, someFraction());
        argument(1, kOut);
        argument(2, kOut + 0x40);
        someRotation(kRecord);
        someRotation(kOther);
        somePlace(kPlaceA);
        somePlace(kPlaceB);
        break;
    case 0x4b6ca0:
        entry.eax = kRecord;
        entry.edx = kOut;
        someRotation(kRecord);
        break;
    case 0x4b6d90:
        entry.eax = kRecord;
        entry.edx = kOther;
        someRotation(kRecord);
        someRotation(kOther);
        // The same now and then: the differences nothing, so the sums do not win.
        if (random(3) == 0)
            for (x86::reg32 i = 0; i < 8; ++i)
                g_memory[kOther + i] = g_memory[kRecord + i];
        break;
    case 0x4b6e80:
        entry.eax = kMatrix;
        entry.edx = kOut;
        someMatrix(kMatrix);
        break;
    default:
        break;
    }
}
