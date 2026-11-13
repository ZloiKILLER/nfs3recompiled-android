// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
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
    const auto argument = [&](x86::reg32 place) { return entry.esp + 4 + 4 * place; };
    const auto someKind = [&]() -> x86::reg32 {
        static const x86::reg32 kOften[] = { 0, 3, 4, 7, 8, 9, 0xa, 0xc };
        return random(2) ? kOften[random(8)] : random(14);
    };
    const auto floats = [&](x86::reg32 from, unsigned count) {
        for (unsigned i = 0; i < count; ++i)
            setFloat(argument(from + i), someFloat());
    };
    switch (c.address)
    {
    case 0x478e10:
        entry.eax = kSimeRecords + 0x40;
        setWord32(kSimeRecords + 0x44, random(2));
        entry.edx = kSimeRecords + 0x50;
        entry.ebx = kSimeRecords + 0x54;
        break;
    case 0x497500:
        entry.eax = kSimeRecords;
        entry.edx = random(4) == 0 ? kSimeRecords : kSimeRecords + 0x40;
        for (x86::reg32 i = 0; i < 3; ++i)
            setFloat(kSimeRecords + 4 * i, someFloat());
        break;
    case 0x4be970:
        entry.eax = random(2) ? x86::reg32(-int(random(5))) : random(5);
        break;
    case 0x4bea40:
    {
        static const x86::reg32 kViews[] = { 0x7cda80, 0x7cdaa0, 0x7cdb88 };
        entry.eax = kViews[random(3)];
        entry.edx = random(4) == 0 ? 0 : x86::reg32(int(random(200)) - 100);
        // Each limit, and a side of it.
        {
            const x86::reg32 change = entry.edx;
            const x86::reg32 extent = word32(0x7cda68);
            const x86::reg32 across = x86::reg32(x86::sreg32(3u * change) / 4);
            const x86::reg32 off = x86::reg32(int(random(3)) - 1);
            switch (random(4))
            {
            case 0:
                setWord32(entry.eax + 8, extent - change + off);
                break;
            case 1:
                setWord32(entry.eax + 8, x86::reg32(x86::sreg32(extent) / 2) - change + off);
                break;
            case 2:
                setWord32(entry.eax + 0xc, 0u - across + off);
                setWord32(entry.eax + 8, extent / 2 + 20 - change);
                break;
            default:
                break;
            }
        }
        break;
    }
    case 0x4c6c70:
        entry.eax = kSimeRecords + 0x100;
        entry.edx = kSimeRecords + 0x180;
        entry.ebx = kSimeRecords;
        entry.ecx = random(4) == 0 ? kSimeRecords + 0x100 : kSimeRecords + 0x20;
        setWord32(argument(0), random(0));
        setWord32(argument(1), random(0));
        break;
    case 0x4ca160:
        entry.eax = kSimeCar;
        entry.edx = kSimeRecords + 0x40;
        break;
    case 0x4ca500:
        entry.eax = random(2);
        floats(0, 3);
        setWord32(argument(3), someKind());
        // Near the drops there are, often.
        if (random(2))
        {
            setFloat(argument(0), float(int(random(16)) - 8) / 8.0f);
            setFloat(argument(1), float(int(random(16)) - 8) / 8.0f);
            setFloat(argument(2), random(4) == 0 ? 0.0f : 1.0f);
        }
        break;
    case 0x4ca850:
        entry.eax = someKind();
        floats(0, 13);
        setWord32(argument(0), random(0));
        // Nothing to go by: it does not move, and has no life.
        if (random(6) == 0)
            for (x86::reg32 i = 3; i < 13; ++i)
                setFloat(argument(i), random(2) ? 0.0f : -0.0f);
        break;
    case 0x4cab60:
        for (x86::reg32 i = 0; i < 32; ++i)
            setWord32(argument(i), random(0));
        setWord32(argument(1), someKind());
        floats(3, 6);
        if (random(4) == 0)
            for (x86::reg32 i = 6; i < 9; ++i)
                setFloat(argument(i), random(2) ? 0.0f : -0.0f);
        g_memory[argument(0) + 0x7e] = x86::reg8(random(2) ? random(4) : random(256));
        break;
    case 0x4cac20:
        entry.eax = kSimeCar;
        floats(0, 3);
        setFloat(argument(3), random(2) ? float(random(40)) : someFloat());
        break;
    case 0x4cae00:
        entry.eax = someKind();
        floats(0, 3);
        break;
    case 0x4caea0:
        entry.eax = kSimeCar;
        floats(0, 3);
        break;
    case 0x4cb010:
        entry.eax = kSimeCar;
        entry.edx = random(4);
        for (x86::reg32 i = 0; i < 3; ++i)
            setWord32(argument(i), random(4) == 0 ? random(0) : x86::reg32(int(random(1u << 24)) - (1 << 23)));
        setWord32(argument(3), someKind());
        break;
    case 0x4db860:
        entry.edx = random(4);
        break;
    default:
        break;
    }
}
