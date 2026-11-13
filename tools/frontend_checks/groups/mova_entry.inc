// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    // An angle whose sine is not the table's zero: the original would divide by it.
    const auto angle = [] {
        x86::reg32 a = random(4) == 0 ? random(0) : random(0x400);
        return (a & 0xff) == 0 && (a & 0x100) == 0 ? a | 1 : a;
    };
    // Whole degrees, not a multiple of half a turn, for the same reason.
    const auto degrees = [] {
        const x86::reg32 d = 1 + random(179);
        return random(4) == 0 ? x86::reg32(0) - d : random(4) == 0 ? d + 360 : d;
    };
    const auto size = [] { return random(4) == 0 ? random(0) : 1 + random(1280); };
    switch (c.address)
    {
    case 0x4df1f0:
        entry.eax = random(4) == 0 ? random(0) : random(640);
        entry.edx = random(4) == 0 ? random(0) : random(480);
        entry.ebx = size();
        entry.ecx = size();
        setWord32(entry.esp + 4, random(4));
        setWord32(entry.esp + 8, random(2) ? 1 : random(3) == 0 ? random(0) : 0);
        break;
    case 0x4fd280:
        entry.eax = random(2) ? random(0) : random(640);
        entry.edx = random(2) ? random(0) : random(480);
        break;
    case 0x4fd2b0:
        entry.eax = angle();
        entry.edx = random(3) == 0 ? 0 : angle();
        entry.ebx = size();
        entry.ecx = size();
        break;
    case 0x4fd3f0:
        entry.eax = degrees();
        entry.edx = random(3) == 0 ? 0 : degrees();
        entry.ebx = size();
        entry.ecx = size();
        break;
    }
}
