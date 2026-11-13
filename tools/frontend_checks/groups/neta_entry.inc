// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kScratch = 0xb40000, kService = 0xb20000;
    switch (c.address)
    {
    case 0x4c5720:
        entry.eax = random(7);
        break;
    case 0x4a5b00:
        entry.eax = random(2) ? 0 : 0x6fd2fc;
        entry.edx = random(4);
        break;
    case 0x4a6a10:
        entry.eax = random(3) == 0 ? 0 : random(4);
        entry.edx = random(4);
        break;
    case 0x4f61f0:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x400;
        entry.ebx = kScratch + 0x480;
        entry.ecx = kScratch + 0x800;
        setWord32(entry.esp + 4, random(0x200));
        break;
    case 0x4c8040:
        entry.eax = kScratch;
        entry.edx = random(16);
        break;
    case 0x4f4d10:
        entry.eax = kScratch;
        setWord32(kScratch + 4, 0xb21000);
        entry.ecx = kScratch + 0x100;
        setWord32(entry.esp + 4, random(0));
        setWord32(entry.esp + 8, random(0));
        setWord32(entry.esp + 12, random(0));
        break;
    case 0x49b9b0:
        entry.eax = kScratch;
        entry.ebx = random(5) == 0 ? 0 - random(4) : random(0x300);
        break;
    case 0x4c7d30:
        entry.edx = kScratch;
        g_memory[kScratch + 1] = x86::reg8(random(9));
        setWord32(kScratch + 4, word32(0x7dd07c + 20 * g_memory[kScratch + 1]) + random(5) - 2);
        break;
    case 0x4f53f0:
        entry.eax = kService;
        entry.edx = kScratch;
        break;
    case 0x4f60b0:
        entry.eax = kScratch;
        entry.edx = random(2) ? 0 : kScratch + 0x100;
        entry.ecx = kScratch + 0x400;
        setWord32(entry.esp + 4, random(0x200));
        break;
    case 0x4f5950:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x400;
        break;
    case 0x4c5b00:
        entry.eax = kScratch + 0x100;
        entry.edx = kScratch;
        setWord32(kScratch + 4, random(6));
        break;
    case 0x4f5130:
        entry.eax = random(3) == 0 ? 0 : kScratch;
        entry.edx = kScratch + 0x400;
        setWord32(kScratch + 0x404, 0x7e1018);
        setWord32(kScratch + 0x40c, random(2));
        for (x86::reg32 at : { 0x14u, 0x18u, 0x1cu, 0x20u, 0x2au, 0x2cu, 0x2eu })
            if (random(2))
                setWord32(kScratch + at, 0);
        setWord32(entry.esp + 4, random(0));
        break;
    case 0x4a68d0:
    case 0x49b310:
        entry.edx = kScratch;
        for (x86::reg32 i = 0; i < 8; ++i)
            g_memory[kScratch + 0x59 + i] = x86::reg8(random(3));
        g_memory[kScratch + 0x62] = x86::reg8(random(8));
        g_memory[kScratch + 0x63] = 0;
        setWord32(kScratch + 4, random(8));
        break;
    case 0x4f54f0:
    case 0x4f55e0:
    case 0x4f5710:
        entry.eax = 0xb22000;
        setWord32(0xb22000 + 0x4c, kService);
        entry.edx = kScratch;
        entry.ebx = random(0x200);
        break;
    case 0x49f880:
        entry.eax = random(3) == 0 ? 0 : kScratch;
        break;
    case 0x4c8250:
        entry.eax = random(0x1000);
        break;
    case 0x4f6030:
    case 0x4f5ea0:
        entry.eax = kScratch;
        setWord32(kScratch + 0x1c, random(2));
        break;
    case 0x4a5e50:
    {
        static const x86::reg32 kMessages[] = { 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x2a0, 0x2a1, 0x1ff, 0x10203 };
        setWord32(entry.esp + 8, kMessages[random(14)] | (random(0) & 0xffff0000u));
        setWord32(entry.esp + 12, kScratch);
        g_memory[kScratch + 0x64] = x86::reg8(random(3));
        if (random(2))
            setWord32(kScratch, word32(0x7a20a0));
        break;
    }
    case 0x49b3e0:
    case 0x49b470:
    case 0x4a6c40:
        entry.eax = kScratch + 0x100;
        entry.edx = kScratch;
        if (random(2))
            g_memory[kScratch + 2] = 0xc, g_memory[kScratch + 3] = 0;
        break;
    case 0x4f4c70:
        entry.eax = 0x7e1018;
        entry.ecx = random(2) ? random(0x40) : random(0);
        break;
    case 0x4f5b10:
        setWord32(0x9f3650, kScratch);
        setWord32(kScratch, random(2) ? 0 : 0x7e1024);
        setWord32(kScratch + 0x20, random(4) != 0);
        break;
    case 0x4f57c0:
        entry.eax = 0xb22000;
        setWord32(0xb22000 + 0x4c, kService);
        setWord32(0xb22000 + 0x48, random(3) == 0 ? random(4) : 0 - random(8));
        entry.edx = kScratch;
        entry.ebx = random(0x200);
        break;
    case 0x4f4bb0:
    {
        entry.eax = kScratch;
        entry.edx = kScratch + 0x20;
        const x86::reg32 same = random(18);
        for (x86::reg32 i = 0; i < 16; ++i)
            g_memory[kScratch + 0x20 + i] = g_memory[kScratch + i];
        if (same < 16)
            g_memory[kScratch + 0x20 + same] ^= x86::reg8(1 + random(255));
        break;
    }
    case 0x4f5390:
    {
        entry.eax = kService;
        entry.edx = kScratch;
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(0x9f2648 + 0x20 * i, random(2) ? kService : random(0));
        break;
    }
    case 0x4c7fd0:
        entry.eax = random(16);
        break;
    case 0x4f5a30:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x200;
        entry.ebx = kScratch + 0x300;
        for (x86::reg32 i = 0; i < 8; ++i)
            g_memory[kScratch + 0x300 + i] = x86::reg8('a' + random(26));
        g_memory[kScratch + 0x300 + random(8)] = 0;
        break;
    case 0x49f730:
        entry.eax = kScratch;
        break;
    case 0x4a5f30:
        entry.eax = random(10);
        break;
    case 0x49b2b0:
    case 0x4c4190:
    case 0x49edb0:
    case 0x4a6df0:
    case 0x4c7f90:
    case 0x4c7e80:
    case 0x4a6440:
        entry.eax = kScratch + 0x100;
        entry.edx = kScratch;
        g_memory[kScratch + 1] = x86::reg8(random(8));
        if (c.address == 0x4a6440)
            g_memory[kScratch] = x86::reg8(random(24));
        break;
    case 0x4f5ac0:
    case 0x4f5a80:
    {
        entry.eax = kScratch;
        entry.edx = kScratch + 0x100;
        if (random(4))
            setWord32(kScratch + 0x120, word32(kScratch + 0x100 + 0));
        else
            setWord32(kScratch + 0x20, random(0));
        setWord32(kScratch + 0x20, word32(kScratch + 0x100));
        setWord32(kScratch + 0x118, random(2) ? 0 : kScratch + 0x200);
        setWord32(kScratch + 4, word32(0x79c1a0) + random(0x20));
        for (x86::reg32 i = 0; i < 8; ++i)
            g_memory[kScratch + 0x104 + 4 + i] = g_memory[kScratch + 0xc + 4 + i];
        if (random(3) == 0)
            g_memory[kScratch + 0x104 + 4 + random(8)] ^= x86::reg8(1 + random(255));
        break;
    }
    case 0x4f4e60:
    case 0x4f4eb0:
    case 0x4f4ed0:
    case 0x4f4f10:
    case 0x4f4f70:
        entry.eax = kScratch;
        setWord32(kScratch + 4, 0xb21000 + 0x100 * random(8));
        break;
    case 0x4a6470:
    case 0x49e320:
        entry.eax = kScratch;
        setWord32(0x7a01e0, random(9));
        setWord32(0x55e710, random(3) == 0);
        break;
    case 0x4f5fe0:
        entry.edx = kScratch;
        entry.ebx = kScratch + 0x20;
        for (x86::reg32 i = 0; i < 16; ++i)
            g_memory[kScratch + 0x20 + i] = g_memory[kScratch + i];
        if (random(2))
            g_memory[kScratch + 0x20 + 4 + random(10)] ^= x86::reg8(1 + random(255));
        break;
    case 0x4f5900:
    case 0x4f58c0:
    case 0x4f5880:
    case 0x4f5f70:
    case 0x4f6010:
    case 0x4f5fa0:
    case 0x4f5f50:
    case 0x4f5fc0:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x200;
        break;
    case 0x4c8310:
    case 0x4a6410:
        entry.eax = kScratch;
        setWord32(0xb01000 + 0x7c0, random(0));
        break;
    case 0x4c8350:
        entry.eax = kScratch;
        entry.edx = kScratch + 0x100;
        break;
    case 0x4c81c0:
        entry.eax = random(0x1000);
        break;
    case 0x4f5850:
        entry.eax = random(3) == 0 ? 0 : kScratch;
        entry.ecx = kScratch + 0x100;
        setWord32(kScratch + 0x104, 0xb21000);
        break;
    case 0x4a5ef0:
        setWord32(entry.esp + 12, random(9));
        setWord32(entry.esp + 20, random(0));
        setWord32(entry.esp + 24, random(0));
        break;
    case 0x4f5050:
        entry.eax = kScratch;
        setWord32(kScratch + 8, 0xb21000);
        setWord32(0xb21000 + 0x38, random(2) ? 0 : 0x7e1018);
        break;
    case 0x4f56e0:
    case 0x4f5700:
        entry.eax = 0xb22000;
        setWord32(0xb22000 + 0x4c, kService);
        break;
    case 0x4f56c0:
        entry.eax = 0xb22000;
        setWord32(0xb22000 + 0x4c, kScratch);
        setWord32(0xb22000 + 0x48, random(2));
        setWord32(kScratch + 0x9c, random(2));
        break;
    case 0x4f50f0:
        entry.eax = kScratch;
        setWord32(kScratch + 4, 0xb21000);
        setWord32(0xb21000 + 0xc, random(2) ? 0 : 0x7e103c);
        break;
    case 0x4f5ba0:
        setWord32(entry.esp - 0x10, random(2) ? 0 : random(0));
        break;
    case 0x4a64c0:
    case 0x4a6ce0:
        g_memory[0x564819] = x86::reg8(random(3) == 0);
        break;
    default:
        break;
    }
}
