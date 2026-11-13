// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    // A row: where it comes from and goes, the destination on any byte, a count often small.
    constexpr x86::reg32 kSource = 0x900000, kDestination = 0x910000;
    const auto fill = [&](x86::reg32 at, x86::reg32 bytes) {
        for (x86::reg32 i = 0; i < bytes; ++i)
            g_memory[at + i] = x86::reg8(random(256));
    };
    const auto row = [&](x86::reg32 bytesPerPixelTimes2) {
        const x86::reg32 pick = random(8);
        const x86::reg32 count = pick == 0 ? 0 : pick == 1 ? x86::reg32(-1) - random(3) : pick == 2 ? 1 + random(4) : random(40);
        const x86::reg32 pixels = x86::sreg32(count) > 0 ? count : 0;
        fill(kSource, pixels * bytesPerPixelTimes2 / 2 + 16);
        entry.eax = kSource + random(4);
        // Now and then over the source, as sub_4eafc0 converts a row where it is: what is
        // written before what is read then shows.
        const x86::reg32 overlap = random(4);
        entry.edx = overlap == 0 ? entry.eax + random(2) * (random(5) * 2) : overlap == 1 ? entry.eax + random(16) - 4
                                 : kDestination + (random(2) ? 4 * random(4) : random(8));
        entry.ebx = count;
    };
    switch (c.address)
    {
    case 0x5040b8:
    case 0x504133:
    case 0x5042c4:
        row(4);
        break;
    case 0x5041ae:
        row(2);
        fill(0x920000, 0x400);
        setWord32(0x568390, 0x920000);
        // The one count `jg` after its sub reads through the overflow.
        if (random(16) == 0)
            entry.ebx = 0x80000000;
        break;
    case 0x5041d0:
        row(2);
        fill(0xa0ac00, 0x210);
        break;
    case 0x504335:
        row(2);
        fill(0xa0ac00, 0x40);
        break;
    case 0x50425d:
        row(6);
        break;
    case 0x5089d0:
    {
        const auto number = [&](x86::reg32 usual) {
            const x86::reg32 pick = random(4);
            return pick == 0 ? 0 : pick == 1 ? x86::reg32(-1) : pick == 2 ? usual : random(0);
        };
        entry.eax = number(640);
        entry.edx = number(480);
        entry.ebx = random(5) == 0 ? 15 : number(16);
        break;
    }
    case 0x518f80:
        entry.eax = random(0x2000);
        break;
    case 0x525b80:
        entry.eax = 0x930000 + 4 * random(16);
        break;
    default:
        break;
    }
}
