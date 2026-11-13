// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto number = [&](x86::reg32 span, x86::sreg32 from, float over) { return float(x86::sreg32(random(span)) + from) / over; };
    const auto bitsOf = [](float value) {
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        return bits;
    };
    // A float's bits of real size, or now and then a NaN of its own.
    const auto someBits = [&](x86::reg32 span, x86::sreg32 from, float over) {
        return random(8) == 0 ? 0x7fc00000 | random(0x400000) : bitsOf(number(span, from, over));
    };
    // Now and then none: the three steps of a model's transforms.
    const auto maybe = [&](x86::reg32 address) { return random(3) == 0 ? 0 : address; };
    const x86::reg32 slices = word32(0x5dd940);
    const auto someSlice = [&] { return slices ? random(slices) : 0; };
    constexpr x86::reg32 kOut = 0xb09000;
    constexpr x86::reg32 kModel = 0xb06000;
    switch (c.address)
    {
    case 0x491120:
        entry.eax = random(16);
        break;
    case 0x493380:
        entry.eax = someSlice();
        entry.edx = random(16);
        break;
    case 0x4934c0:
        entry.eax = someSlice();
        entry.edx = random(2);
        entry.esi = kOut;
        break;
    case 0x493a30:
    case 0x493ad0:
        entry.eax = 0xb07000 + 0x1000 * random(2);
        break;
    case 0x493ef0:
        if (random(2))
        {
            setWord32(0x55e4a0, 0);
            setWord32(0x55e4a4, 0);
        }
        if (random(2))
            setWord32(0x55e4a8, 0);
        break;
    case 0x493fb0:
        setWord32(0x55e4a4, random(0));
        if (random(3) == 0)
            setWord32(0x55e4a0, 0);
        if (random(3) == 0)
            setWord32(0x55e4a8, 0);
        break;
    case 0x494170:
        entry.eax = kOut + 0x100;
        entry.edx = someSlice();
        break;
    case 0x494390:
        entry.eax = kOut;
        entry.edx = kOut + 0x10;
        break;
    case 0x494420:
    {
        entry.eax = 0x79c570 + 48 * random(240);
        entry.edx = random(2) ? kOut : entry.eax + 0x20;
        entry.ebx = random(6);
        entry.ecx = random(2) ? entry.ebx : random(6);
        argument(0, someBits(1024, -512, 16.0f));
        break;
    }
    case 0x4945a0:
        for (x86::reg32 i = 0; i < 4; ++i)
            argument(i, random(16) == 0 ? random(0) : bitsOf(number(1024, -512, 16.0f)));
        break;
    case 0x4964d0:
    {
        static const x86::reg32 kOdd[] = { 0xffc00000, 0x7fc00000, 0x7f800000, 0x80000000, 0, 0x3f800000, 0x7fc00001, 0xbf800000 };
        entry.eax = kOut;
        setWord32(kOut, random(4) == 0 ? kOdd[random(8)] : random(8) == 0 ? random(0) : bitsOf(number(1024, -512, 64.0f)));
        break;
    }
    case 0x496550:
        entry.eax = kOut;
        entry.edx = kOut + 0x10;
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(kOut + (i < 4 ? 4 * i : 0x10 + 4 * (i - 4)), random(4) == 0 ? random(0) : random(0x1000000) - 0x800000);
        // Now and then right beside each other, for the sizes' edge.
        if (random(3) == 0)
            for (x86::reg32 i = 0; i < 3; ++i)
                setWord32(kOut + 0x10 + 4 * i, word32(kOut + 4 * i) + random(5) - 2);
        break;
    case 0x496bf0:
        // At most one NaN among them: between two the host's compiler commutes a sum as it likes.
        argument(0, someBits(1024, -512, 16.0f));
        argument(1, bitsOf(number(1024, -512, 16.0f)));
        argument(2, bitsOf(number(256, -128, 64.0f)));
        argument(3, kOut);
        argument(4, kOut + 4);
        break;
    case 0x496f20:
        entry.eax = kOut;
        entry.edx = kOut + 0x10;
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(kOut + (i < 4 ? 4 * i : 0x10 + 4 * (i - 4)), random(16) == 0 ? random(0) : someBits(256, -128, 8.0f));
        argument(0, random(16) == 0 ? random(0) : someBits(256, 0, 8.0f));
        break;
    case 0x49c420:
        entry.eax = kOut + 0x100;
        entry.edx = kOut;
        argument(0, bitsOf(number(256, -128, 8.0f)));
        break;
    case 0x49c470:
    case 0x49c4a0:
        entry.esi = kOut;
        break;
    case 0x49d5a0:
        entry.eax = kOut;
        setWord32(kOut, random(3) == 0 ? 0 : random(0));
        break;
    case 0x49d5c0:
        entry.eax = kModel;
        entry.edx = maybe(kOut);
        entry.ebx = maybe(kOut + 0x40);
        entry.ecx = maybe(kOut + 0x80);
        argument(0, maybe(kOut + 0xc0));
        argument(1, random(2));
        break;
    case 0x49d760:
    case 0x49d780:
        entry.eax = kOut;
        for (x86::reg32 i = 0; i < 3; ++i)
            setWord32(kOut + 4 * i, random(16) == 0 ? random(0) : bitsOf(number(256, -128, 8.0f)));
        break;
    case 0x49de20:
        entry.eax = random(16) == 0 ? 0 : kModel;
        entry.edx = random(8);
        entry.ebx = maybe(kOut);
        entry.ecx = maybe(kOut + 0x40);
        argument(0, maybe(kOut + 0x80));
        argument(1, maybe(kOut + 0xc0));
        argument(2, maybe(kOut + 0x100));
        argument(3, maybe(kOut + 0x140));
        argument(4, random(2));
        setWord32(0x55e69c, random(2) ? 1 : random(3));
        break;
    case 0x4a4060:
        entry.eax = random(4) == 0 ? random(0) & 0x3fffffff : random(0x300);
        entry.edx = random(4) == 0 ? random(0) & 0x3fffffff : random(0x300);
        break;
    case 0x4a40c0:
        entry.eax = random(3) == 0 ? 9 : random(10);
        break;
    default:
        break;
    }
}
