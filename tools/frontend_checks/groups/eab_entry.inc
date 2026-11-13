// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto display = [&]() -> x86::reg32 { const x86::reg32 pick = random(3); return pick == 0 ? 0 : pick == 1 ? 0x9f38d0 : 0xa10000; };
    const auto arg = [&](x86::reg32 i) { return entry.esp + 4 + 4 * i; };
    static const x86::reg32 kDepthsAsked[] = { 0, 8, 15, 16, 24, 32 };
    const auto oneOf = [&](std::initializer_list<x86::reg32> values) { return *(values.begin() + random(x86::reg32(values.size()))); };
    constexpr x86::reg32 kText = 0xa13000, kBuffer = 0xa18000, kShape = 0xa1b000, kFrom = 0xa20000;
    const auto makeBitmap = [&] {
        entry.eax = random(8) == 0 ? 0x5000 : random(800);
        entry.edx = random(8) == 0 ? 0x3000 : random(600);
        entry.ebx = kDepthsAsked[random(6)];
    };
    switch (c.address)
    {
    case 0x4ec870:
        entry.eax = display();
        setWord32(0x9f38d0, 0xa10800);
        setWord32(0xa10000, 0xa10800);
        entry.edx = random(3) == 0 ? x86::reg32(-1) : random(8);
        break;
    case 0x4ecd50:
    case 0x4ecf60:
    case 0x4ed2a0:
        entry.eax = display();
        break;
    case 0x4ed0f0:
        entry.eax = display();
        entry.edx = random(4) == 0 ? 0 : random(2) ? 0xa10820 : 0xa10810;
        entry.ebx = random(2) ? kBuffer : 0;
        break;
    case 0x4eee30:
        makeBitmap();
        setWord32(arg(0), random(2));
        break;
    case 0x4ef190:
        makeBitmap();
        break;
    case 0x4ea4a0:
        entry.ebx = kBuffer;
        break;
    case 0x4ef1e0:
    {
        constexpr x86::reg32 kFile = 0xa19000;
        fillRandom(kFile, kFile + 0x60);
        setWord32(kFile + 8, random(6));
        for (x86::reg32 i = 0; i < 6; ++i)
            setWord32(kFile + 0x10 + i * 8, random(4));
        setWord32(kBuffer, random(5));
        entry.eax = kFile;
        entry.edx = kBuffer;
        break;
    }
    case 0x4ef2b0:
        fillRandom(kFrom, kFrom + 0x40);
        entry.eax = kFrom;
        entry.edx = kFrom + 0x100;
        entry.ebx = random(24) - 2;
        break;
    case 0x4ef320:
    case 0x4ef340:
    case 0x4ef370:
    case 0x4ef3c0:
        fillRandom(kFrom, kFrom + 0x100);
        for (x86::reg32 at = kFrom; at < kFrom + 0x100; at += 4)
            if (random(2) == 0)
                setWord32(at, c.address == 0x4ef320 ? 0xffffffff : 0);
        entry.eax = kFrom + 4;
        entry.edx = kFrom + 0x200;
        entry.ebx = 1 + random(16);
        break;
    case 0x4ef3f0:
    case 0x4ef4c0:
    case 0x4f2cb0:
    case 0x4f7660:
        fillRandom(kShape, kShape + 0x20);
        entry.eax = kShape;
        break;
    case 0x4ef520:
    {
        constexpr x86::reg32 kFont = 0xa1c000;
        fillRandom(kFont, kFont + 0x180);
        setWord32(kFont, random(5) == 0 ? random(0) : 0x00544e46 | (random(0x100) << 24));
        setWord32(kFont + 0x1c, 0x100);
        setWord32(kFont + 0x14, 0x40);
        g_memory[kFont + 0xa] = x86::reg8(random(0x100));
        g_memory[kFont + 0xb] = x86::reg8(random(4) == 0);
        g_memory[kFont + 0x21] = x86::reg8(random(4) == 0);
        entry.eax = kFont;
        break;
    }
    case 0x4ef6c0:
        entry.eax = 0xa0a814;
        break;
    case 0x4ef760:
        if (random(6) == 0)
            entry.eax = 0;
        break;
    case 0x4ef830:
        entry.eax = random(3) == 0 ? random(0x200) : random(4);
        entry.edx = random(4);
        break;
    case 0x4efa10:
        entry.eax = 0x20 + random(0x60);
        entry.edx = 0xa14000;
        entry.ebx = random(20);
        entry.ecx = 0xb;
        break;
    case 0x4efa70:
        entry.eax = 0x20 + random(0x60);
        break;
    case 0x4efb10:
        entry.eax = 0x41 + random(2);
        entry.edx = 0x41 + random(2);
        break;
    case 0x4efb50:
    case 0x4efef0:
        entry.eax = random(8) == 0 ? 0 : kText;
        break;
    case 0x4efe30:
        entry.eax = kText;
        break;
    case 0x4efe50:
        entry.eax = random(8) == 0 ? 0 : kText;
        entry.edx = random(14) - 2;
        break;
    case 0x4eff00:
        fillRandom(0xa1d000, 0xa1d040);
        for (const x86::reg32 at : { 0x2c, 0x30, 0x34 })
            setWord32(0xa1d000 + at, random(2) ? random(0) : 0);
        entry.eax = 0xa1d000;
        break;
    case 0x4f22e0:
        setWord32(entry.esp - 0x1c, random(3) == 0 ? 0 : random(0));
        setWord32(entry.esp - 0x1c + 8, random(0));
        entry.eax = kText;
        entry.edx = kBuffer;
        break;
    case 0x4f24e0:
    {
        constexpr x86::reg32 kRecords = 0xa1e000;
        const auto record = [&](x86::reg32 at, x86::reg32 next) { setWord32(kRecords + at, (next << 8) | (random(3) == 0 ? 0x6f : random(0x60))); };
        record(0, 0x20);
        record(0x20, 0x20);
        record(0x40, random(2) ? 0x20 : 0);
        record(0x60, 0);
        entry.eax = random(6) == 0 ? 0 : kRecords;
        break;
    }
    case 0x4f2930:
        entry.eax = random(3) == 0 ? 0x7e0814 : random(0);
        break;
    case 0x4f2ad0:
        fillRandom(kFrom, kFrom + 0x1000);
        entry.eax = kFrom + random(4);
        entry.edx = kFrom + 0x800 + random(8);
        entry.ecx = random(4);
        entry.ebx = entry.ecx + random(0x400);
        break;
    case 0x4f2bf0:
        fillRandom(kFrom, kFrom + 0x400);
        entry.eax = kFrom + 0x100 + random(4);
        entry.edx = random(2) ? entry.eax + random(0x40) - 0x20 : kFrom + 0x300;
        entry.ebx = random(3) == 0 ? random(4) : random(0x80);
        break;
    case 0x4f2c40:
    case 0x4f2c50:
    case 0x4f2c60:
    case 0x4f2c70:
    case 0x4f2c90:
        fillRandom(kFrom, kFrom + 0x400);
        entry.eax = kFrom + 0x100 + random(4);
        entry.edx = random(2) ? entry.eax + random(0x40) - 0x20 : kFrom + 0x600;
        entry.ebx = random(0x40);
        break;
    case 0x4f7450:
        entry.eax = random(2) ? 1234 : 0;
        break;
    case 0x4f75d0:
        entry.eax = 0xa22000;
        break;
    case 0x4f7670:
        entry.eax = kBuffer;
        fillRandom(kBuffer, kBuffer + 4);
        entry.edx = oneOf({ 0, 0xa18320, 0xa18330, 0xa18300, 0xa18300, 0xa18300 });
        entry.ebx = random(2) ? 80 : 0;
        entry.ecx = 0xa18100;
        break;
    case 0x4f7750:
        entry.edx = 0xa18200;
        entry.ebx = 0xa18600;
        entry.ecx = random(2) ? 0xa18700 : 0;
        break;
    case 0x4f77c0:
        fillRandom(0xa18800, 0xa18810);
        std::memcpy(g_memory + 0xa18810, g_memory + 0xa18800, 0x10);
        if (random(2))
            g_memory[0xa18810 + random(0x10)] ^= 1;
        entry.edx = 0xa18800;
        entry.ebx = 0xa18810;
        break;
    case 0x4f77f0:
        entry.eax = 0xa22000;
        break;
    case 0x4f9500:
    {
        setWord32(arg(1), oneOf({ 1, 2, 2, 2, 8, 12, 3, 0, 13 }));
        const x86::reg32 pick = oneOf({ 0, 1, 2, 3, 0x10, 0x20, 0x40, 0x80, 0x90, 0x100, 0x200, 0x4000, 0x4000, 0x4000, 0x4000 });
        setWord32(arg(3), pick);
        setWord32(arg(4), oneOf({ 0, 1, 2, 4, 7, 8, 9, 0x10, 0x20, 0x30, 0x40, 0x50 }));
        break;
    }
    case 0x4fbc30:
        entry.eax = oneOf({ 0xa18300, 0xa18350, 0xa18320 });
        entry.ebx = random(2);
        break;
    case 0x4fbda0:
        setWord32(0x9f4f60, 0xa23000);
        setWord32(entry.esp - 0x20, random(0));
        break;
    case 0x4fc250:
        entry.eax = random(2) * 100;
        entry.edx = random(2) * random(8);
        break;
    case 0x514280:
        entry.eax = 0xa22300;
        entry.edx = 0xa18200;
        setWord32(arg(0), 0xa18300);
        setWord32(arg(1), 0xa18350);
        break;
    case 0x514320:
        entry.eax = 0xa22300;
        break;
    case 0x5150f0:
        entry.eax = kBuffer;
        setWord32(entry.esp - 0x148 + 0x104, random(2));
        std::memcpy(g_memory + entry.esp - 0x148 + 0x108, "1.2.3.4", 8);
        break;
    case 0x517bf0:
        entry.eax = random(0x40);
        break;
    case 0x517e50:
    case 0x517eb0:
        entry.eax = 0xa25000;
        break;
    case 0x517f10:
        entry.eax = random(6) == 0 ? 0 : 0xa25000;
        entry.edx = 0xa18300;
        break;
    case 0x524df0:
        entry.eax = 0xa26010;
        entry.edx = oneOf({ x86::reg32(-1), x86::reg32(-2), 0, 4, 8, 0x20, 0x100, 0x400 });
        break;
    default:
        break;
    }
}
