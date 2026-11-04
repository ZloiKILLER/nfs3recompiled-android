// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto point = [&] { return random(8) == 0 ? random(0) : x86::reg32(random(20)) - 2; };
    switch (c.address)
    {
    case 0x45ab30:
    {
        // An element's item: of either view, all kinds; now and then none of them.
        const x86::reg32 item = 0x79e000 + 0xc0 * random(16);
        const x86::reg32 pick = random(12);
        setWord32(item + 0x3c, pick == 0 ? x86::reg32(-1) - random(4) : pick == 1 ? 46 + random(8) : random(46));
        setWord32(item + 0x70, random(2) ? 0 : 1 + random(4));
        entry.eax = item;
        break;
    }
    case 0x4853c0:
        entry.eax = random(2);
        entry.edx = random(3) == 0 ? 8 : random(8) == 0 ? random(0) : random(26);
        break;
    case 0x4896f0:
        // Around the teens, and far from them on both sides.
        entry.eax = random(4) == 0 ? random(0) : x86::reg32(random(60)) - 15;
        break;
    case 0x489750:
        entry.eax = 0x5f3f40;
        entry.edx = random(8) == 0 ? 0x80000000u : random(4) == 0 ? random(0) : x86::reg32(random(200000)) - 20000;
        break;
    case 0x4898b0:
        entry.eax = random(4) == 0 ? random(0) : x86::reg32(random(200000)) - 20000;
        break;
    case 0x489900:
        entry.edx = random(2) ? 1 : random(4);
        break;
    case 0x48b2a0:
        for (x86::reg32 i = 0; i < 6; ++i)
            argument(i, point());
        argument(6, random(2));
        break;
    case 0x48b4a0:
        for (x86::reg32 i = 0; i < 4; ++i)
            argument(i, point());
        break;
    default:
        break;
    }
}
