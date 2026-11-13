// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto bitsOf = [](float value) {
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        return bits;
    };
    // A float of real size, now and then 0, or a NaN of its own.
    const auto someBits = [&](x86::reg32 span, x86::sreg32 from, float over) {
        const x86::reg32 pick = random(16);
        if (pick == 0)
            return bitsOf(0.0f);
        if (pick == 1)
            return x86::reg32(0x7fc00000 | random(0x400000));
        return bitsOf(float(x86::sreg32(random(span)) + from) / over);
    };
    const auto someObject = [&] { return kLoadaObjects + 0x1000 * random(8); };
    const auto someCar = [&] { return kLoadaCars + 0x400 * random(16); };
    const auto someEntry = [&] { return 0x6fd52c + 0x6c * random(8); };
    const auto someString = [&] { return kLoadaStrings + 0x10 * random(16); };
    switch (c.address)
    {
    case 0x4284e0:
        entry.eax = someObject();
        argument(0, random(0));
        argument(1, random(8) == 0 ? bitsOf(10.0f) : someBits(400, -100, 16.0f));
        argument(2, random(0));
        break;
    case 0x4285f0:
    case 0x428970:
        entry.eax = someObject();
        break;
    case 0x428700:
        entry.eax = someObject();
        if (random(2))
            setWord32(0x6fdc30, word32(word32(entry.eax + 0x220) + 0x14));
        if (random(2))
            setWord32(0x6fd3b8, 2);
        break;
    case 0x428ad0:
        if (random(2))
            setWord32(0x5efd9c, 0);
        entry.eax = someObject();
        entry.edx = random(40);
        argument(0, someBits(256, -128, 16.0f));
        argument(1, random(0));
        argument(2, someBits(256, -128, 16.0f));
        break;
    case 0x428d60:
        if (random(2))
            setWord32(0x5efd9c, 0);
        entry.eax = someObject();
        // Near either end of the track as often as not: the walk along it wraps there.
        entry.edx = random(3) == 0 ? random(3) : random(2) ? 37 + random(3) : random(40);
        break;
    case 0x438b70:
        entry.eax = random(4) == 0 ? random(0) : word32(someCar());
        break;
    case 0x438f60:
        // Low memory not zero, so that no record reads differently from the way out it takes.
        for (x86::reg32 i = 0; i < 4; ++i)
            g_memory[0x38f + i] = x86::reg8(1 + random(255));
        entry.eax = random(40);
        entry.edx = random(6);
        break;
    case 0x439510:
    case 0x4395a0:
    case 0x4395e0:
    case 0x439710:
        entry.eax = random(40);
        break;
    case 0x439a10:
        entry.eax = random(16);
        break;
    case 0x439bb0:
        entry.eax = random(4) == 0 ? 0 : someCar();
        break;
    case 0x439bf0:
    {
        const x86::reg32 car = someCar();
        setWord32(car + 4, random(2) ? 0 : random(0));
        entry.eax = car;
        break;
    }
    case 0x43a4d0:
        setWord32(kLoadaOut + 0x100, someString());
        setWord32(kLoadaOut + 0x104, someString());
        entry.eax = kLoadaOut + 0x100;
        entry.edx = kLoadaOut + 0x104;
        break;
    case 0x43a6b0:
        entry.eax = random(50);
        entry.edx = random(4);
        if (random(4) == 0)
            setWord32(entry.edx * 200 + 0x5552d8 + entry.eax * 4, 0);
        break;
    case 0x43a6e0:
    {
        // A few of the flags at a time, or the pass over the cars is all a case does.
        x86::reg32 flags = random(4) << 2;
        for (x86::reg32 i = random(4); i > 0; --i)
            flags |= x86::reg32(1) << (4 + random(18));
        if (random(8) == 0)
            flags |= 1;
        if (random(8) == 0)
            flags |= 2;
        entry.eax = random(16) == 0 ? random(0) : flags;
        entry.edx = random(2);
        setWord32(0x6fd4d4, random(8));
        setWord32(0x6fd2bc, random(8));
        setWord32(0x6fd328, random(8));
        break;
    }
    case 0x43b2a0:
        entry.eax = someCar();
        break;
    case 0x474640:
        // A network race at the numbers its limits turn on.
        if (random(2))
        {
            std::uint16_t players = std::uint16_t(random(2) ? 4 : 6);
            std::memcpy(g_memory + 0x7a2276, &players, 2);
            setWord32(0x6fd3b0, 2 + random(3));
            setWord32(0x6fd4e8, 0);
        }
        break;
    case 0x4516a0:
    {
        // Sometimes exactly the room there is, as the frame's three words multiply.
        const x86::reg32 words = entry.esp - 0x118 + 0xfc;
        const x86::reg32 room = word32(words + 4) * word32(words + 0xc) * word32(words + 8);
        entry.eax = random(4) == 0 ? room : random(2) ? room + 1 : room - 1;
        break;
    }
    case 0x452a00:
        entry.eax = someString();
        entry.edx = someString();
        break;
    case 0x473360:
        entry.eax = random(4);
        entry.edx = random(300);
        entry.ebx = 1 + random(49);
        break;
    case 0x4733b0:
        entry.eax = random(4);
        entry.edx = random(256);
        break;
    case 0x4734d0:
    case 0x473550:
    case 0x473570:
    case 0x4735c0:
        entry.eax = random(16);
        break;
    case 0x4736c0:
        entry.eax = someEntry();
        break;
    case 0x473710:
        entry.eax = someEntry();
        entry.edx = random(8);
        entry.ebx = random(2);
        break;
    case 0x473890:
        entry.eax = someEntry();
        entry.edx = random(16);
        entry.ebx = random(2) ? 0 : 1 + random(3);
        break;
    case 0x473960:
    case 0x473a70:
        entry.eax = someEntry();
        entry.edx = random(16);
        break;
    case 0x473b70:
        entry.eax = random(8) == 0 ? x86::reg32(0) - random(3) : random(9);
        entry.edx = random(3) == 0 ? 0 : 0x6fd3be;
        break;
    case 0x474c60:
        entry.eax = random(0x400);
        break;
    case 0x478090:
    case 0x478100:
    case 0x4781b0:
        entry.eax = random(6) == 0 ? 0x6fbb40 : kLoadaSaved;
        break;
    case 0x4782b0:
        entry.eax = random(8);
        break;
    case 0x4782e0:
        entry.esi = kLoadaOut + 0x200;
        break;
    case 0x478320:
        entry.eax = kLoadaOut + 0x300;
        entry.edx = kLoadaOut + 0x400;
        entry.ebx = kLoadaOut + 0x404;
        break;
    case 0x4be2f0:
    {
        const x86::reg32 count = random(9);
        setWord32(kLoadaList + 4, count);
        for (x86::reg32 i = 0; i < 9; ++i)
        {
            setWord32(kLoadaList + 8 + 0x10 * i + 4, random(4));
            setWord32(kLoadaList + 8 + 0x10 * i + 8, random(4));
        }
        entry.eax = random(8) == 0 ? 0 : kLoadaList;
        entry.edx = random(4);
        entry.ebx = random(4);
        break;
    }
    case 0x4dc550:
        entry.eax = random(3);
        entry.edx = random(4) == 0 ? 0 : random(0);
        break;
    default:
        break;
    }
}
