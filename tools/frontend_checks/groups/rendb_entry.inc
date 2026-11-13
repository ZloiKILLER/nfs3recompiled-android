// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kRendbCars = 0x911000, kRendbCarBytes = 0x1000;
    constexpr x86::reg32 kRendbView = 0x910000;
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    switch (c.address)
    {
    case 0x47bc20:
        entry.eax = random(4) == 0 ? random(0) : random(30);
        break;
    case 0x47d1d0:
        // Four corners, in floats on the screen and off it; filled or not; a colour.
        for (x86::reg32 i = 0; i < 8; ++i)
            setFloat(entry.esp + 4 + 4 * i, float(x86::sreg32(random(1200)) - 200));
        argument(8, random(2) ? 0 : random(0));
        argument(9, random(0));
        break;
    case 0x47d480:
        entry.eax = random(2);
        entry.edx = kRendbCars + random(16) * kRendbCarBytes;
        break;
    case 0x47d580:
    {
        static const x86::reg32 kWidths[] = { 320, 512, 640, 800, 1024, 1920 };
        entry.eax = kRendbView;
        entry.edx = random(10);
        entry.ebx = kWidths[random(6)];
        break;
    }
    case 0x47ee80:
        entry.eax = random(2);
        entry.edx = kRendbCars + random(16) * kRendbCarBytes;
        break;
    case 0x47f090:
        entry.eax = kRendbCars + random(16) * kRendbCarBytes;
        break;
    case 0x47f2e0:
        // A rectangle's rows to copy, none now and then: a place, the widths and its size.
        entry.eax = 0x940000 + random(0x100);
        entry.edx = 0x950000 + random(0x100);
        entry.ebx = random(0x40);
        entry.ecx = random(0x40);
        argument(0, random(0x40));
        argument(1, random(0x40));
        argument(2, 1 + random(0x100));
        argument(3, 1 + random(0x100));
        argument(4, random(0x40));
        argument(5, random(6));
        break;
    case 0x47f340:
        entry.eax = random(0);
        break;
    case 0x47fbe0:
    case 0x47fcb0:
        entry.eax = random(0);
        break;
    default:
        break;
    }
}
