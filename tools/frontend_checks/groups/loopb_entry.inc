// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kCars = 0xb00000, kCarSize = 0x1000, kCarCount = 12;
    constexpr x86::reg32 kScratch = 0xb0d000;
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto someCar = [&] { return kCars + kCarSize * random(kCarCount); };
    // Bytes in runs, for the packing.
    const auto someRow = [&](x86::reg32 at, x86::reg32 bytes) {
        for (x86::reg32 i = 0; i < bytes; ++i)
            g_memory[at + i] = x86::reg8(random(3) == 0 ? random(256) : random(3));
    };
    switch (c.address)
    {
    case 0x4bc100:
    case 0x4bc130:
        entry.eax = kScratch;
        setWord32(kScratch + 4, random(4));
        break;
    case 0x4bc620:
    case 0x4bc680:
    case 0x4c4de0:
        entry.eax = random(2) ? 0 : 1 + random(4);
        break;
    case 0x4bc9b0:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x100;
        someRow(kScratch, 0x28);  // what follows the row looks like it: a run must stop at 32
        break;
    case 0x4bca80:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x400;
        for (x86::reg32 i = 0; i < 0x200; ++i)
            g_memory[kScratch + i] = x86::reg8(random(8) == 0 ? 0 : random(2) ? 1 + random(8) : random(256));
        break;
    case 0x4bcda0:
        someRow(entry.esp + 4, 0x80);
        break;
    case 0x4bcee0:
        entry.esi = kScratch;
        break;
    case 0x4bd000:
    case 0x4bd090:
        entry.eax = random(2);
        break;
    case 0x4bd380:
    case 0x4cded0:
    case 0x4cdf10:
    case 0x4ce280:
    case 0x4ce5a0:
        entry.eax = someCar();
        break;
    case 0x4bd3f0:
        entry.eax = random(6) == 0 ? random(0) : random(5);
        break;
    case 0x4bd9d0:
        entry.eax = random(8) == 0 ? random(0) : random(40);
        entry.edx = (random(0) & 0xffffff00u) | random(16);
        break;
    case 0x4be020:
    {
        // A text: what is not a number, /comments/, and a number after them.
        const x86::reg32 text = kScratch + 0x100;
        x86::reg32 at = text;
        for (x86::reg32 pieces = random(4); pieces > 0; --pieces)
        {
            if (random(2))
            {
                g_memory[at++] = '/';
                for (x86::reg32 i = random(5); i > 0; --i)
                    g_memory[at++] = x86::reg8(random(2) ? '0' + random(10) : 'a' + random(26));
                g_memory[at++] = '/';
            }
            else
            {
                g_memory[at++] = x86::reg8(random(2) ? ' ' : 'a' + random(26));
            }
        }
        if (random(3) == 0)
            g_memory[at++] = random(2) ? '-' : '+';
        for (x86::reg32 i = 1 + random(random(8) == 0 ? 200 : 6); i > 0; --i)
            g_memory[at++] = x86::reg8('0' + random(10));
        g_memory[at++] = ' ';
        g_memory[at] = 0;
        setWord32(kScratch, text);
        entry.eax = kScratch;
        break;
    }
    case 0x4be5e0:
        entry.eax = random(2);
        break;
    case 0x4bed20:
    case 0x4bed50:
    case 0x4bed70:
        entry.eax = kScratch;
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(kScratch + 4 * i, random(8) == 0 ? random(0) : random(2000));
        entry.edx = random(8) == 0 ? random(0) : random(180);
        break;
    case 0x4cdd30:
    {
        setWord32(kScratch, someCar());
        setWord32(kScratch + 4, someCar());
        entry.eax = kScratch;
        entry.edx = kScratch + 4;
        break;
    }
    case 0x4cdef0:
        entry.eax = x86::reg32(random(12)) - 2;
        break;
    case 0x4ce750:
        entry.eax = random(6);
        break;
    case 0x4ce800:
        entry.eax = 0x5efc08;
        entry.edx = random(8) == 0 ? x86::reg32(-1) : random(9);
        entry.ebx = random(6);
        break;
    case 0x4dbce0:
        entry.eax = random(2000);
        entry.edx = random(2000);
        entry.ebx = random(2000);
        entry.ecx = random(2000);
        argument(0, random(8) == 0 ? random(0) : random(180));
        argument(1, random(0));
        break;
    default:
        break;
    }
}
