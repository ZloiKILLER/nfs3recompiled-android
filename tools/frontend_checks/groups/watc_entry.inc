// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kZone = 0xb10000, kTable64 = 0xb20000, kSource = 0xb30000, kTarget = 0xb31000;
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto put = [&](x86::reg32 at, const char* text) { std::strcpy(reinterpret_cast<char*>(g_memory) + at, text); };
    const auto little = [&](x86::reg32 most) { return random(6) == 0 ? x86::reg32(0) - random(4) : random(most); };
    const auto tapiStub = [&](x86::reg32 slot, x86::reg32 count) {
        const x86::reg32 pick = random(4);
        setWord32(slot, pick == 0 ? 0 : pick == 1 ? 0xffffffffu : 0x7e0600 + 0x10 * count);
        for (x86::reg32 i = 0; i < count; ++i)
            argument(i, random(0));
    };
    const auto someLock = [&]() -> x86::reg32 {
        static constexpr x86::reg32 kLocks[] = { 0xa0b320, 0xa0b330, 0xa0b440, 0xa0ba60, 0xa0ba70, 0xa0b340 };
        return kLocks[random(6)];
    };
    const auto number = [&](x86::reg32 at) {
        static const char* const kNumbers[] = { "123", "  -45", "+7", "0x1f", "0X7", "017", "zz", "", "-", "ffffffffffff", "4294967296", "2147483648", "-2147483648", "9", " 0x" };
        put(at, kNumbers[random(15)]);
    };
    const auto someDouble = [&](x86::reg32 at) {
        static const double kDoubles[] = { 0.0, -0.0, 1.5, -2.5, 1e300, -1e-300, std::numeric_limits<double>::quiet_NaN() };
        std::memcpy(g_memory + at, &kDoubles[random(7)], 8);
    };
    switch (c.address)
    {
    case 0x51e388:
        entry.edx = little(24);
        break;
    case 0x51e670:
        entry.eax = kZone;
        break;
    case 0x51e698:
    case 0x51e6a8:
    case 0x51e6c0:
        entry.eax = random(3);
        break;
    case 0x51e71c:
    case 0x51e780:
        entry.eax = someLock();
        break;
    case 0x51e8dc:
        entry.eax = random(2);
        break;
    case 0x51eccc:
    {
        static constexpr x86::reg32 kLines[] = { 0x5513f0, 0x551688, 0x5516bc, 0x5516e4, 0x5517f4, 0x551828 };
        put(kZone, random(3) == 0 ? "" : "some text ");
        entry.eax = kZone;
        entry.edx = kLines[random(6)];
        entry.ebx = random(3) == 0 ? 0 : random(0);
        break;
    }
    case 0x51ed24:
    {
        static constexpr x86::reg32 kCodes[] = { 0xc0000005, 0xc000001d, 0xc000008d, 0xc000008e, 0xc000008f, 0xc0000090, 0xc0000091, 0xc0000092,
                                                 0xc0000093, 0xc0000094, 0xc0000095, 0xc0000096, 0xc00000fd, 0x80000003, 0xc000001c, 0xc0000006 };
        fillRandom(kZone, kZone + 0x100);
        setWord32(kZone, kZone + 0x10);
        setWord32(kZone + 4, kZone + 0x40);
        setWord32(kZone + 0x10, kCodes[random(16)]);
        setWord32(kZone + 0x10 + 0x14, random(2));
        argument(0, kZone);
        setWord32(0x56acbc, kZone + 0x80);
        break;
    }
    case 0x51ef08:
    {
        fillRandom(kZone, kZone + 0x100);
        setWord32(kZone, random(5) == 0 ? random(0) : 0xc000008d + random(7));
        setWord32(kZone + 4, random(6) == 0 ? 2 : 0);
        setWord32(kZone + 0x40 + 0x28, kZone + 0x80);
        static constexpr std::uint16_t kInstructions[] = { 0xfad9, 0xf1d9, 0x10db, 0x38df, 0x30d8, 0x31d8, 0x00db, 0x20d9, 0x35de };
        std::memcpy(g_memory + kZone + 0x80, &kInstructions[random(9)], 2);
        argument(0, kZone);
        argument(2, kZone + 0x40);
        break;
    }
    case 0x51f140:
    {
        fillRandom(kZone, kZone + 0x100);
        static constexpr char kCharacters[] = { 'g', 'G', 'e', 'E', 'f', 'F' };
        g_memory[kZone + 0x15] = x86::reg8(kCharacters[random(6)]);
        setWord32(kZone + 8, random(3) == 0 ? 0xffffffffu : random(8));
        setWord32(kZone + 0x20, random(16));
        setWord32(kZone + 0x80, kZone + 0xa0);
        someDouble(kZone + 0xa0);
        someDouble(kZone + 0xa8);
        entry.eax = kTarget;
        entry.edx = kZone + 0x80;
        entry.ebx = kZone;
        break;
    }
    case 0x51f254:
        entry.edx = kZone;
        break;
    case 0x5236c0:
    {
        static constexpr x86::reg8 kTypes[] = { 0x7b, 0x40, 0x41, 0x42, 0x78, 0x7d, 0x7e, 0x80, 0xfb, 0xc1 };
        fillRandom(kZone, kZone + 0x10);
        g_memory[kZone] = kTypes[random(10)];
        entry.eax = kZone;
        break;
    }
    case 0x523760:
    case 0x523b00:
    {
        static constexpr x86::reg32 kAlphas[] = { 0x00, 0xff, 0x0f, 0x10, 0xfb, 0xfc, 0x80, 0x01 };
        entry.eax = little(70);
        entry.edx = little(70);
        entry.ebx = (kAlphas[random(8)] << 24) | (random(0) & 0xffffff);
        break;
    }
    case 0x5239ec:
    {
        fillRandom(kSource, kSource + 0x400);
        fillRandom(kTarget, kTarget + 0x400);
        static bool tableFilled = [&] { fillRandom(kTable64, kTable64 + 0x10000); return true; }();
        (void)tableFilled;
        argument(0, kTable64);
        argument(1, kSource);
        argument(2, kTarget);
        argument(3, little(14));
        argument(4, little(6));
        argument(5, random(8));
        argument(6, random(8));
        break;
    }
    case 0x525ba0:
    {
        static constexpr x86::reg32 kPages[] = { 0xffffffff, 0xfffffffe, 0xfffffffd, 0xfffffffc, 0, 1, 932, 1252 };
        entry.eax = kPages[random(8)];
        break;
    }
    case 0x525f20:
        entry.eax = random(3);
        entry.edx = 0xb00000;
        break;
    case 0x525f84:
        entry.eax = random(3);
        break;
    case 0x526080:
    case 0x5260c8:
    case 0x5260ec:
    case 0x526110:
    case 0x526338:
        entry.eax = random(15);
        entry.edx = random(4);
        break;
    case 0x52612c:
        argument(0, random(3));
        break;
    case 0x52b523:
        someDouble(entry.esp + 4);
        someDouble(entry.esp + 0xc);
        setWord32(entry.esp + 0x14, random(3));
        break;
    case 0x52b6c0:
        fillRandom(kZone, kZone + 0x20);
        setWord32(kZone + 4, random(100));
        setWord32(kZone + 8, random(100));
        entry.eax = kZone;
        entry.edx = little(120);
        entry.ebx = little(120);
        entry.ecx = little(120);
        argument(0, little(120));
        break;
    case 0x52b720:
    case 0x52bd50:
        entry.eax = little(30);
        entry.edx = little(30);
        entry.ebx = little(c.address == 0x52bd50 ? 6 : 30);
        entry.ecx = little(c.address == 0x52bd50 ? 6 : 30);
        argument(0, random(24));
        break;
    case 0x52b8b0:
        fillRandom(kZone, kZone + 0x20);
        entry.eax = kZone;
        break;
    case 0x52b910:
    case 0x52bb50:
        entry.eax = random(7);
        break;
    case 0x52b970:
    case 0x52bc00:
        argument(0, random(100));
        argument(1, random(100));
        argument(2, kZone);
        argument(3, random(0));
        break;
    case 0x52bb90:
        fillRandom(kZone, kZone + 0x10);
        entry.eax = kZone;
        break;
    case 0x52ba00:
        entry.eax = random(24);
        entry.edx = random(24);
        break;
    case 0x52ba70:
    case 0x52bbf0:
    case 0x52bd00:
        entry.eax = random(24);
        break;
    case 0x52baf0:
        argument(0, random(24));
        break;
    case 0x52e500:
    case 0x52e660:
    {
        static constexpr x86::reg32 kBases[] = { 0, 0, 0, 2, 8, 10, 16, 16, 36, 1, 37, 0xffffffff };
        number(kZone);
        entry.eax = kZone;
        entry.edx = random(3) == 0 ? 0 : kTarget;
        entry.ebx = kBases[random(12)];
        if (c.address == 0x52e500)
            entry.ecx = random(3);
        break;
    }
    case 0x52e6d0:
    case 0x52e88c:
    case 0x52ee60:
    {
        static const char* const kTexts[] = { "NAME=val", "ab=1", "PATH=", "Z=", "=x", "none", "" };
        put(kZone, kTexts[random(7)]);
        entry.eax = c.address == 0x52e88c && random(8) == 0 ? 0 : kZone;
        entry.edx = c.address == 0x52ee60 ? (random(4) == 0 ? 0 : 0x3d) : kZone;
        break;
    }
    case 0x52ea3c:
    {
        static const char* const kNames[] = { "NAME=", "name=val", "AB=", "path=q", "Z=", "none=", "" };
        put(kZone, kNames[random(7)]);
        setWord32(0xa0b154, 0xb03000);  // its caller has seen there is an environ
        entry.eax = kZone;
        entry.edx = random(2);
        break;
    }
    case 0x52eb70:
        fillRandom(kZone, kZone + 0x20);
        setWord32(kZone, 0x7e0590);
        argument(0, kZone);
        break;
    case 0x52ec04:
        entry.eax = 0x7e0590;
        break;
    case 0x52ee00:
        entry.eax = little(900);
        entry.ebx = little(900);
        entry.edx = little(900);
        entry.ecx = little(900);
        break;
    case 0x52f318:
        setWord32(kZone, random(0));
        argument(0, 0x56ba50);
        argument(1, kZone);
        break;
    case 0x52f535: tapiStub(0x56d14c, 3); break;
    case 0x52f661: tapiStub(0x56d15c, 1); break;
    case 0x52f969: tapiStub(0x56d184, 1); break;
    case 0x52fb2e: tapiStub(0x56d19c, 3); break;
    case 0x530747: tapiStub(0x56d238, 5); break;
    case 0x53083d: tapiStub(0x56d244, 3); break;
    case 0x530a4e: tapiStub(0x56d260, 6); break;
    case 0x530b4d: tapiStub(0x56d26c, 2); break;
    case 0x5310be: tapiStub(0x56d2b8, 5); break;
    case 0x5311c0: tapiStub(0x56d2c4, 5); break;
    case 0x53138f: tapiStub(0x56d2dc, 6); break;
    case 0x531439: tapiStub(0x56d2e4, 9); break;
    case 0x531f31: tapiStub(0x56d374, 4); break;
    case 0x532162: tapiStub(0x56d390, 3); break;
    case 0x5324d2: tapiStub(0x56d3bc, 1); break;
    default:
        break;
    }
}
