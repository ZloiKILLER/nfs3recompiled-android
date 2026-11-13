// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kDisplay = 0xa0f000, kSurface = 0xa00510, kFontRecord = 0xa05000, kGlyphs = 0xa06000, kFontData = 0xa04000;
    constexpr x86::reg32 kDevices = 0xa0a000, kOut = 0xa07800, kWindow = 0xa0e000, kText = 0xa01f00;
    const auto outOrNone = [&](x86::reg32 at) -> x86::reg32 { return random(4) == 0 ? 0 : at; };
    const auto aDoubleHigh = [&]() -> x86::reg32 {
        const x86::reg32 sign = random(2) ? 0x80000000 : 0;
        switch (random(6))
        {
        case 0: return sign;
        case 1: return sign | random(0x100000);
        case 2: return sign | 0x7ff00000 | random(0x100000);
        case 3: return sign | (random(0x7ff) << 20) | random(0x100000);
        case 4: return sign | ((0x3f0 + random(0x20)) << 20) | random(0x100000);
        default: return random(0);
        }
    };
    const auto aDoubleLow = [&]() -> x86::reg32 { return random(3) == 0 ? 0 : random(2) ? random(0x10) << 28 : random(0); };
    switch (c.address)
    {
    case 0x4ed330:
        entry.eax = kDisplay;
        break;
    case 0x4ed480:
        entry.eax = kDisplay;
        entry.edx = random(800);
        entry.ebx = random(600);
        setWord32(entry.esp + 4, random(3));
        setWord32(entry.esp + 8, random(3) - 1);
        setWord32(entry.esp + 12, random(2) ? random(0x800) : random(0));
        setWord32(entry.esp + 16, random(2) ? kSurface : 0);
        break;
    case 0x4ed730:
        entry.eax = random(2) ? kDisplay : 0;
        entry.edx = kSurface;
        break;
    case 0x4ed770:
        entry.eax = kSurface;
        break;
    case 0x4ed820:
        entry.eax = kSurface;
        entry.edx = outOrNone(kOut);
        entry.ebx = outOrNone(kOut + 4);
        entry.ecx = outOrNone(kOut + 8);
        setWord32(entry.esp + 4, outOrNone(kOut + 12));
        setWord32(entry.esp + 8, outOrNone(kOut + 16));
        break;
    case 0x4edbc0:
        setWord32(entry.esp + 4, 0xa01000);
        setWord32(entry.esp + 8, outOrNone(0xa01100));
        break;
    case 0x518fd0:
        entry.eax = random(4) == 0 ? 0 : random(40);
        break;
    case 0x4f91d0:
        entry.eax = random(2) ? kText : 0;
        break;
    case 0x4fb860:
        entry.eax = random(32);
        break;
    case 0x4fbd10:
        entry.eax = kDevices + random(32) * 0x68;
        entry.edx = random(0x40);
        entry.ebx = kOut;
        break;
    case 0x4fc340:
    case 0x4fc600:
    case 0x4fc780:
        entry.eax = random(0x40);
        break;
    case 0x50ea20:
    case 0x50ea40:
        entry.eax = 0x9f4f74;
        break;
    case 0x508c70:
        entry.eax = random(8) == 0 ? random(0) : random(3000);
        entry.edx = random(8) == 0 ? random(0) : random(2000);
        setWord32(entry.esp + 4, random(3) == 0 ? random(0x100) : x86::reg32(0) - 1 - random(2));
        break;
    case 0x508da0:
        entry.eax = random(3) == 0 ? 0 : kWindow;
        entry.edx = random(2) ? word32((entry.eax ? entry.eax : 0x564ff4) + 0x28) : random(0x1000);
        setWord32(kWindow + 0x2c, random(2) ? random(0) : 0);
        break;
    case 0x508e70:
        entry.eax = random(3) == 0 ? 0x56502c : random(2) ? 0x565064 : kWindow;
        setWord32(entry.eax + 0x2c, random(2) ? random(0) : 0);
        setWord32(entry.eax + 0x30, random(2) ? random(0) : 0);
        break;
    case 0x5199b0:
        entry.eax = 0x20 + random(0x60);
        entry.edx = kGlyphs;
        entry.ebx = random(0x60);
        entry.ecx = 11;
        break;
    case 0x519a50:
        entry.eax = kFontRecord;
        entry.edx = 0x20 + random(0x60);
        break;
    case 0x519ea0:
        entry.eax = 0xa07000;
        break;
    case 0x519f10:
        entry.eax = kFontData;
        break;
    case 0x5257c0:
        entry.eax = kFontRecord;
        entry.edx = random(0x12);
        entry.ebx = random(3) == 0 ? random(2) : random(2) ? 0x10000 + random(0x10000) : random(0);
        if (entry.edx >= 8 && entry.edx <= 0xb && random(2))
            entry.ebx = word32(kFontRecord + 0x6c + 4 * (entry.edx - 8));
        break;
    case 0x5259d0:
        entry.eax = kFontRecord;
        entry.edx = random(0x12);
        break;
    case 0x51de60:
    case 0x51df60:
        entry.eax = random(5) == 0 ? x86::reg32(0) - random(3) : random(300);
        entry.edx = 0xa08000;
        entry.ebx = 0xa09000;
        break;
    case 0x524930:
    case 0x524950:
        entry.eax = 0x5f3c00;
        break;
    case 0x524970:
        entry.eax = kText;
        setWord32(entry.esp + 4, random(100));
        setWord32(entry.esp + 8, 1 + random(3));
        break;
    case 0x524c30:
    case 0x52e440:
        entry.eax = kText;
        break;
    case 0x52448c:
    {
        entry.eax = 0xa07100;
        const std::uint16_t exponent = std::uint16_t((random(2) ? 0x8000 : 0) | (random(3) == 0 ? 0x7fff : random(2) ? 0 : random(0x7fff)));
        std::memcpy(g_memory + 0xa07100 + 8, &exponent, 2);
        setWord32(0xa07100, random(2) ? 0 : random(0));
        setWord32(0xa07100 + 4, random(3) == 0 ? 0x80000000 : random(2) ? 0 : random(0));
        break;
    }
    case 0x52643c:
    case 0x526642:
    case 0x5267ff:
    case 0x52699c:
        entry.edx = aDoubleHigh();
        entry.eax = aDoubleLow();
        entry.ecx = random(4) == 0 ? entry.edx : aDoubleHigh();
        entry.ebx = random(4) == 0 ? entry.eax : aDoubleLow();
        // Two NaNs: which comes out is the host compiler's choice of operand order.
        if ((entry.edx & 0x7ff00000) == 0x7ff00000 && (entry.ecx & 0x7ff00000) == 0x7ff00000)
            entry.ecx &= 0xbfffffff;
        break;
    case 0x52700c:
        setWord32(entry.esp + 4, aDoubleLow());
        setWord32(entry.esp + 8, aDoubleHigh());
        [[fallthrough]];
    case 0x526f87:
    case 0x526674:
        entry.fpu.push(x86::Float(double(random(1000)) / 7.0));
        entry.fpu.push(x86::Float(double(random(1000)) / 3.0));
        entry.fpu.status.word = random(2) ? 0x3800 : 0;
        break;
    default:
        break;
    }
}
