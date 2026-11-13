// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kQueueTest = 0x7e0320, kKeyHandler = 0x7e0330;
    constexpr x86::reg32 kShapes = 0xb80000, kPalette = 0xb88000, kRowSpace = 0xba0000;
    constexpr x86::reg32 kFrom = 0xb00000, kTo = 0xb02000, kScratch = 0xb05000, kFont = 0xb90000;
    constexpr x86::reg32 kOut = 0xb91000, kLinks = 0xbd0000;
    constexpr x86::reg32 kLong = 0xbe0000, kCvt = 0xbe0100, kBuffer = 0xbe0200, kDigits = 0xbe0400;
    constexpr x86::reg32 kQueue = 0xba8000, kHeaps = 0xba9000, kWindow = 0xbaa000;
    static const x86::reg32 kTypes[] = { 0x40, 0x41, 0x42, 0x43, 0x6d, 0x78, 0x7a, 0x7b, 0x7d, 0x7e, 0x7f };
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto someShape = [&] { return kShapes + random(6) * 0x400; };
    const auto someType = [&] { return random(8) == 0 ? random(0x90) : kTypes[random(std::size(kTypes))]; };
    const auto someCount = [&] { return random(8) == 0 ? x86::reg32(0) - random(3) : 1 + random(40); };
    const auto someCoordinate = [&] { return random(8) == 0 ? x86::reg32(0) - random(12) : random(80); };
    // The other depths, for what only reports them: not for what goes on to draw.
    const auto oddDepth = [&] {
        if (random(6) == 0)
            g_memory[0x565010] = x86::reg8(random(40));
    };
    // A CVT of what printf asks.
    const auto someCvt = [&] {
        setWord32(kCvt + 0, random(20));
        setWord32(kCvt + 4, random(7) - 3);
        setWord32(kCvt + 8, random(128));
        setWord32(kCvt + 0xc, random(3) == 0 ? 'E' : 0);
        setWord32(kCvt + 0x10, random(6) == 5 ? x86::reg32(0) - 1 : random(6));
        setWord32(kCvt + 0x1c, random(4));
    };
    const auto someDigits = [&](x86::reg32 at, x86::reg32 count) {
        for (x86::reg32 i = 0; i < count; ++i)
            g_memory[at + i] = x86::reg8(random(3) == 0 ? '0' : random(2) ? '9' : '0' + random(10));
        g_memory[at + count] = 0;
    };
    const auto someLongDouble = [&](x86::reg32 at) {
        setWord32(at, random(0));
        setWord32(at + 4, random(0) | 0x80000000);
        const x86::reg32 pick = random(6);
        const x86::reg32 exponent = pick == 0 ? 0 : pick == 1 ? 0x4019 + random(2) : pick == 2 ? 0x4034 + random(2) : 0x3ff0 + random(0x60);
        g_memory[at + 8] = x86::reg8(exponent);
        g_memory[at + 9] = x86::reg8((exponent >> 8) | (random(2) << 7));
        if (pick == 1 && random(2))
            setWord32(at + 4, 0xbebc2000 - 1 + random(3));
        if (pick == 2 && random(2))
            setWord32(at + 4, 0x8e1bc9bf - 1 + random(3));
    };
    // __LDcvt's frame, for its parts.
    const auto someFrame = [&] {
        entry.ebp = kStack - 0x100;
        const x86::reg32 frame = entry.ebp;
        entry.esp = frame - 0x180;
        entry.esi = kCvt;
        entry.ecx = random(24) - 8;
        someCvt();
        setWord32(frame - 0x14, random(2) ? 0x7f : 0x37f);
        setWord32(frame - 0x20, kBuffer);
        const x86::reg32 count = 1 + random(16);
        setWord32(frame - 0x24, count);
        setWord32(frame - 0x18, random(32));
        setWord32(frame - 0x28, frame - 0x97 + count);
        g_memory[frame - 0x98] = '0';
        someDigits(frame - 0x97, count);
        someLongDouble(frame - 0x34);
        entry.fpu.setControl(0x37f);
    };
    // A type that is none looks a converter up below the tables: there are globals there,
    // which the game's report of it would not have got to.
    if (c.address == 0x50aad0 || c.address == 0x50b0b0 || c.address == 0x50db30 || c.address == 0x4fd540)
        for (x86::reg32 at = 0x568370; at < 0x568394; at += 4)
            setWord32(at, 0x7e0310);
    // At 8 bits or less the font's table is 4096 calls: now and then.
    if ((c.address == 0x50bfd0 || c.address == 0x50c180 || c.address == 0x50c0d0) && g_memory[0x565010] <= 8 && random(16) != 0)
        g_memory[0x565010] = 16;
    // No tables yet.
    if ((c.address == 0x509940 || c.address == 0x509a00) && random(2))
        setWord32(0x568380, 0);
    if ((c.address == 0x50c0d0 || c.address == 0x50c1e0) && random(2))
        setWord32(0x567280, 0);
    // A shape of no depth, a window of no type: reported.
    if ((c.address == 0x4fd1c0 || c.address == 0x50b0b0 || c.address == 0x50aad0) && random(8) == 0)
        for (x86::reg32 i = 0; i < 6; ++i)
            g_memory[kShapes + i * 0x400] = x86::reg8(random(0x80));
    if (c.address == 0x50b0b0 && random(8) == 0)
        g_memory[0x565011] = 0;
    switch (c.address)
    {
    case 0x4fb820:
        entry.eax = kScratch;
        if (random(2))
            entry.edx = word32(kScratch + 4);
        break;
    case 0x4fb830:
        entry.eax = kScratch;
        break;
    case 0x4fd1c0:
        entry.eax = random(8) == 0 ? 0 : someShape();
        break;
    case 0x4fd540:
        entry.eax = someCoordinate();
        entry.edx = someCoordinate();
        entry.ebx = random(30);
        entry.ecx = random(30);
        argument(0, random(0));
        break;
    case 0x503bc0:
        oddDepth();
        entry.eax = someCoordinate();
        entry.edx = someCoordinate();
        break;
    case 0x508b80:
        entry.eax = kWindow;
        entry.edx = 1 + random(30);
        entry.ebx = 1 + random(30);
        entry.ecx = random(0x800);
        argument(0, random(4) == 0 ? 0 : random(2) ? 16 : random(33));
        argument(1, random(0));
        argument(2, random(0));
        break;
    case 0x5098d0:
        entry.eax = random(40);
        break;
    case 0x509a30:
    {
        entry.eax = random(40);
        entry.edx = random(8);
        entry.ebx = random(4);
        entry.ecx = random(26);
        // A run there is, its first two offsets asked for again.
        if (random(2))
        {
            x86::reg32 at = kRowSpace;
            for (x86::reg32 i = random(8); i > 0; --i)
                at += (word32(at) + 2) * 4;
            entry.eax = word32(at + 8);
            entry.edx = word32(at + 0xc) - entry.eax;
            entry.ebx = 0;
            entry.ecx = word32(at) - random(2);
        }
        break;
    }
    case 0x509b10:
    {
        x86::reg32 at = kRowSpace;
        for (x86::reg32 i = random(8); i > 0; --i)
            at += (word32(at) + 2) * 4;
        entry.eax = at + 8 + (random(10) == 0 ? 4 : 0);
        if (random(3))
            setWord32(entry.eax - 4, 1);
        break;
    }
    case 0x509c00:
    {
        // Heaps: each a head, a first block and two more.
        for (x86::reg32 h = 0; h < 16; ++h)
            setWord32(0x9ef5c0 + 4 * h, random(3) == 0 ? kHeaps + 0x100 * (h & 3) : 0);
        for (x86::reg32 h = 0; h < 4; ++h)
        {
            const x86::reg32 head = kHeaps + 0x100 * h;
            setWord32(head + 8, head + 0x20);
            setWord32(head + 0x20 + 8, head + 0x40);
            setWord32(head + 0x40, random(2) ? 0 : 0xc0000000);
            setWord32(head + 0x40 + 4, 0x10);
            setWord32(head + 0x40 + 8, head + 0x80);
            setWord32(head + 0x80, random(2) ? 0 : 0x40000000);
            setWord32(head + 0x80 + 4, 0x10);
            setWord32(head + 0x80 + 8, 0);
        }
        entry.eax = kHeaps + 0x100 * random(4) + (random(2) ? 0x50 : 0x90) + random(0x20);
        break;
    }
    case 0x509c50: case 0x509c60: case 0x509c70: case 0x509cd0: case 0x509d20: case 0x509df0: case 0x509e50:
    case 0x509eb0: case 0x509ef0: case 0x509f50: case 0x509fb0: case 0x50a020: case 0x50a0a0: case 0x50a100:
    case 0x50a150: case 0x50a1c0: case 0x50a1e0: case 0x50a240: case 0x50a2b0: case 0x50a330: case 0x50a3c0:
    case 0x50a440: case 0x50a4c0: case 0x50a510:
        entry.eax = kFrom;
        entry.edx = kTo;
        entry.ebx = someCount();
        // Colours the key is, and that come out 0.
        setWord32(0x568388, random(2) ? (word32(kFrom + 2 * random(40)) & 0xffff) : random(0x10000));
        for (x86::reg32 i = 0; i < 40; ++i)
            if (random(6) == 0)
                setWord32(kFrom + 4 * i, random(3) == 0 ? 0 : random(2) ? 0x3f000000 : word32(kFrom + 4 * i) & 0xff000007);
        if (c.address == 0x509d20 && random(2))
        {
            setWord32(0x5650c0, kPalette);
            fillRandom(kPalette, kPalette + 0x8000);
        }
        break;
    case 0x50a570:
        entry.eax = random(2) ? kTypes[random(std::size(kTypes))] : 0x38 + random(0x50);
        break;
    case 0x50a5d0:
    case 0x50a830:
        entry.eax = someShape();
        entry.edx = kOut;
        break;
    case 0x50a960:
        entry.eax = someShape();
        entry.edx = someType();
        entry.ebx = random(2);
        break;
    case 0x50aad0:
        entry.eax = someShape();
        entry.edx = someType();
        entry.ebx = random(2);
        entry.ecx = random(2);
        break;
    case 0x50afc0:
        entry.eax = kFrom;
        entry.edx = kTo + random(5);
        entry.ebx = random(8) == 0 ? 0 : 1 + random(24);
        for (x86::reg32 i = 0; i < 40; ++i)
            if (random(5) == 0)
                g_memory[kFrom + i] = 0xff;
        break;
    case 0x50b080:
        entry.eax = kFrom;
        entry.edx = kTo;
        entry.ebx = 1 + random(40);
        break;
    case 0x50b0b0:
    case 0x50db30:
        entry.eax = someShape();
        entry.edx = someCoordinate();
        entry.ebx = someCoordinate();
        entry.ecx = random(2) ? 0x5676b4 : 0x5688a8;
        break;
    case 0x50b540:
    case 0x50b580:
        setWord32(kScratch, kFrom);
        entry.eax = kScratch;
        break;
    case 0x50b560:
        entry.eax = random(4) == 0 ? random(0) | 0x80000000 : random(0x90);
        break;
    case 0x50b5c0:
    case 0x50ba90:
    case 0x50bb80:
    {
        if (c.address == 0x50bb80)
            oddDepth();
        const x86::reg32 clipLeft = word32(0x565000), clipTop = word32(0x565004);
        const bool inside = random(4) != 0;
        entry.eax = inside ? clipLeft + random(4) : someCoordinate();
        entry.edx = inside ? clipTop + random(4) : someCoordinate();
        entry.ebx = kFont;
        entry.ecx = random(8) == 0 ? x86::reg32(0) - random(8) : random(16);
        x86::reg32 width = inside ? 1 + random(16) : 1 + random(90);
        x86::reg32 height = inside ? 1 + random(16) : 1 + random(90);
        if (random(10) == 0)
            width = x86::reg32(0) - random(3);
        if (random(10) == 0)
            height = x86::reg32(0) - random(3);
        if (x86::sreg32(width) <= 0 || x86::sreg32(height) <= 0)
            g_memory[0x565012] = 1;
        argument(0, random(4));
        argument(1, width);
        argument(2, height);
        argument(3, 8 + random(16));
        fillRandom(kFont, kFont + 0x1000);
        break;
    }
    case 0x50bdf0:
        // Most of the 16 with no alpha: each that has one is 256 calls.
        entry.eax = kScratch;
        for (x86::reg32 k = 0; k < 16; ++k)
            g_memory[kScratch + 4 * k + 3] = x86::reg8(random(8) == 0 ? random(256) : 0);
        entry.edx = kOut;
        break;
    case 0x50bfd0:
        entry.eax = random(256);
        break;
    case 0x50c220:
    case 0x50c270:
    case 0x50c2c0:
    case 0x50c350:
        entry.ebx = kTo + random(4);
        break;
    case 0x50c3a0:
    {
        oddDepth();
        const x86::reg8 depth = g_memory[0x565010];
        if (depth == 15 || depth == 16 || depth == 24 || depth == 32)
            entry.eax = random(3) == 0 ? random(0) : random(4) == 0 ? 0 : random(0x10000);
        else
            entry.eax = random(4) == 0 ? x86::reg32(0) - 1 - random(4) : random(0x120);
        break;
    }
    case 0x50c584:
        entry.eax = random(2) ? (random(0) & 0xffffff00) | (1 + random(255)) : (random(0) & 0xffff0000) | (random(256) << 8);
        break;
    case 0x50c5d4:
    {
        entry.eax = random(2) ? 1 + random(0x100) : random(0x90) << 8;
        entry.edx = random(2) ? kKeyHandler : random(0) | 0x10000;
        const x86::reg32 taken = random(4) == 0 ? 0x40 : random(6);
        for (x86::reg32 i = 0; i < 0x40; ++i)
            setWord32(0x56879e + 4 * i, i < taken ? (random(8) == 0 ? entry.edx : (random(0) | 1)) : 0);
        break;
    }
    case 0x50d560:
    {
        entry.eax = kTo + random(9);
        entry.edx = random(3) == 0 ? entry.eax + random(0x40) - 0x20 : kFrom + random(9);
        const x86::reg32 pick = random(5);
        entry.ebx = pick == 0 ? x86::reg32(0) - 1 - random(8) : pick == 1 ? 0x80 + random(0x200) : random(0x60);
        break;
    }
    case 0x50d6e0:
    case 0x50d7e0:
    case 0x50d880:
    case 0x50d910:
    case 0x50d9c0:
    case 0x50daa0:
        entry.eax = kFrom;
        entry.edx = kTo;
        entry.ebx = 1 + random(24);
        for (x86::reg32 i = 0; i < 24; ++i)
        {
            static const x86::reg8 kAlphas[] = { 0, 0x08, 0x20, 0x80, 0xfb, 0xfc, 0xff };
            g_memory[kFrom + 4 * i + 3] = kAlphas[random(std::size(kAlphas))];
            g_memory[kPalette + 4 * i + 3] = kAlphas[random(std::size(kAlphas))];
            if (random(3) == 0)
                g_memory[kTo + 4 * i + 3] = 0;
        }
        break;
    case 0x50e5a0:
    {
        entry.eax = kQueue;
        entry.edx = random(8) == 0 ? 0 : kQueueTest;
        const x86::reg32 count = random(4);
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord32(kQueue + 0x40 + 0x10 * i, i + 1 < count ? kQueue + 0x50 + 0x10 * i : 0);
        setWord32(kQueue + 8, count != 0 ? kQueue + 0x40 : 0);
        setWord32(kQueue + 0x18, random(8) == 0 ? 0 : 0x1234 + random(4));
        break;
    }
    case 0x5157f4:
        entry.eax = random(4);
        break;
    case 0x5159e4:
        entry.eax = kLinks + 0x1000;
        entry.edx = random(2) ? 0 : kLinks + 0x1400;
        entry.ebx = kLinks + 0x1800;
        break;
    case 0x5167b3:
        someLongDouble(kLong);
        someCvt();
        entry.eax = kLong;
        entry.edx = kCvt;
        entry.ebx = kBuffer;
        break;
    case 0x516a87:
        entry.eax = random(4) == 0 ? random(0) : random(100000000);
        entry.ebx = kBuffer;
        break;
    case 0x516a9d:
        entry.eax = random(4) == 0 ? random(0x1000000) : random(10000);
        entry.ebx = kBuffer;
        break;
    case 0x516ab4:
        entry.eax = random(4) == 0 ? random(0x10000) : random(100);
        entry.ebx = kBuffer;
        break;
    case 0x5169ef:
        someFrame();
        entry.edi = random(2) ? 0 : random(100000000);
        setWord32(entry.ebp - 0x24, 0);
        setWord32(entry.ebp - 0x28, entry.ebp - 0x97);
        g_memory[entry.ebp - 0x97] = 0;
        break;
    case 0x516acc:
        someFrame();
        setWord32(entry.esp, word32(entry.ebp - 0x24));
        setWord32(entry.esp + 4, entry.ecx);
        entry.ebx = word32(entry.ebp - 0x28) + 8 * random(2);
        setWord32(entry.ebp - 0x18, random(2) ? 0 : 8);
        break;
    case 0x516ae2:
    case 0x516be2:
        someFrame();
        break;
    case 0x516dfa:
        entry.ebp = entry.esp + 0x40;
        break;
    case 0x516bf5:
    case 0x516e13:
        someCvt();
        someDigits(kDigits, 24);
        entry.eax = kCvt;
        entry.edx = kDigits;
        entry.ebx = random(24);
        entry.ecx = c.address == 0x516bf5 ? random(24) - 12 : random(2400) - 1200;
        argument(0, kBuffer);
        break;
    default:
        break;
    }
}
