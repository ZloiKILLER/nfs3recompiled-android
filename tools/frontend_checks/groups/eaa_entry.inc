// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kBlocks = 0xa02000, kOut = 0xa03000, kOtherDisplay = 0xa05000, kBuffer = 0xa08000;
    const auto block = [&] { return kBlocks + random(24) * 0x80; };
    const auto displayOrNone = [&]() -> x86::reg32 { return random(3) == 0 ? 0 : random(2) ? 0x9f38d0 : kOtherDisplay; };
    const auto aDisplay = [&]() -> x86::reg32 { return random(2) ? 0x9f38d0 : kOtherDisplay; };
    switch (c.address)
    {
    case 0x497620:
        entry.eax = 0xa00000;
        // The table on the stack, if the case makes it there.
        for (x86::reg32 at = entry.esp - 0x2100; at < entry.esp - 0x2c; at += 4)
            setWord32(at, block());
        break;
    case 0x497cc0:
    case 0x497fe0:
    case 0x498f00:
        entry.eax = random(14);
        break;
    case 0x4989a0:
    case 0x4994a0:
        entry.eax = random(16);
        break;
    case 0x498a40:
        entry.eax = random(16);
        entry.edx = random(8);
        entry.ebx = kBuffer;
        break;
    case 0x498b70:
        entry.eax = block() + 0x10;
        entry.edx = kOut;
        entry.ebx = kOut + 4;
        entry.ecx = kOut + 8;
        setWord32(entry.esp + 4, kOut + 12);
        break;
    case 0x498e40:
        entry.edx = kBuffer;
        break;
    case 0x499150:
        entry.eax = 0xa01000 + 8 * random(16);
        entry.edx = 0xa01000 + 8 * random(16);
        // Ties: the same block now and then, the registers columns 5 and 6 compare alike.
        if (random(4) == 0)
            entry.edx = entry.eax;
        if (random(4) == 0)
            entry.esi = entry.ebx;
        break;
    case 0x499330:
        entry.eax = kOut;
        entry.edx = kOut + 4;
        entry.ebx = kOut + 8;
        entry.ecx = kOut + 12;
        break;
    case 0x499400:
        entry.edx = kOut;
        entry.ebx = kOut + 4;
        entry.ecx = kOut + 8;
        setWord32(entry.esp + 4, kOut + 12);
        break;
    case 0x499460:
        entry.eax = random(4) == 0 ? 0 : block();
        break;
    case 0x499490:
    case 0x499580:
        entry.eax = block();
        break;
    case 0x499e90:
        entry.eax = block() + 0x10;
        break;
    case 0x499f70:
        entry.eax = random(5) == 0 ? 0 : block();
        entry.edx = random(2) ? 8 : 0x10;
        break;
    case 0x49a400:
        entry.eax = 0xa03100;
        break;
    case 0x4dc0e0:
        entry.eax = random(3) == 0 ? 0 : 0xa03100;
        entry.edx = random(3) == 0 ? 0 : 0xa03140;
        entry.ebx = random(2);
        break;
    case 0x4dc1b0:
    case 0x4dc1e0:
        // A udff's record, on a file or memory.
        setWord32(kOut, random(2));
        setWord32(kOut + 4, random(2) ? 0 : random(0));
        setWord32(kOut + 8, kBuffer);
        entry.eax = c.address == 0x4dc1b0 && random(4) == 0 ? 0 : kOut;
        break;
    case 0x4ead90:
    case 0x4eada0:
        entry.eax = random(3) == 0 ? 0 : kBuffer;
        break;
    case 0x4eaeb0:
        entry.eax = random(4);
        entry.edx = random(4);
        entry.ebx = random(5);
        entry.ecx = random(5);
        setWord32(entry.esp + 4, kBuffer);
        break;
    case 0x4eafc0:
        entry.eax = kBuffer;
        break;
    case 0x4eb1c0:
        entry.eax = random(0x40);
        break;
    case 0x4eb490:
    case 0x4ec0d0:
        entry.eax = aDisplay();
        break;
    case 0x4eb5e0:
        entry.eax = displayOrNone();
        if (entry.eax != 0)
            setWord32(entry.eax + 0x444, 0xa00510 + 0x10 * random(2));
        entry.edx = 0xa00510 + 0x10 * random(2);
        entry.ebx = kOut;
        break;
    case 0x4eb8a0:
        entry.eax = aDisplay();
        setWord32(entry.eax + 0x10, 0xa06000);
        entry.edx = random(4) == 0 ? x86::reg32(-1) : random(16);
        entry.ebx = kBuffer;
        entry.ecx = random(2);
        break;
    case 0x4ebaf0:
        nfs3hp::eaaCapsClear = true;
        entry.eax = displayOrNone();
        break;
    case 0x4ebe50:
        entry.eax = displayOrNone();
        if (random(8) == 0)
            setWord32(entry.eax != 0 ? entry.eax : 0x9f38d0, 0);
        break;
    case 0x4ec1a0:
        entry.eax = displayOrNone();
        break;
    default:
        break;
    }
}
