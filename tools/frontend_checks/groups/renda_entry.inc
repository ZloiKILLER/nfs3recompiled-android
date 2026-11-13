// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kOut = 0x7e5000, kPoints = 0x7e6000, kFloats = 0x7e7e00;
    const auto view = [&] { return 0x7e7000 + 0x100 * random(2); };
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    const auto realArgument = [&](x86::reg32 place, float scale) {
        if (nfs3hp::g_rendaNaNs && random(2) == 0)
            setWord32(entry.esp + 4 + 4 * place, nfs3hp::kRendaNaN);
        else if (nfs3hp::g_rendaNaNs || random(16) != 0)
            setFloat(entry.esp + 4 + 4 * place, (float(random(2001)) - 1000.0f) * scale / 1000.0f);
    };
    switch (c.address)
    {
    case 0x470aa0:
    case 0x470ad0:
    case 0x470b20:
    case 0x470b70:
        // A share around a hundred, past it on both sides, none; junk now and then.
        entry.eax = random(4) == 0 ? random(0) : random(4) == 0 ? 0 : x86::reg32(random(130)) - 15;
        entry.edx = random(4) == 0 ? random(0) : x86::reg32(random(130)) - 15;
        break;
    case 0x476fb0:
    {
        const x86::reg32 string = 0x7ea000;
        const x86::reg32 length = random(12);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[string + i] = x86::reg8(1 + random(255));
        g_memory[string + length] = 0;
        entry.eax = string;
        break;
    }
    case 0x478cb0:
        entry.eax = kOut;
        for (x86::reg32 i = 0; i < 37; ++i)
            argument(i, random(0));
        argument(36, random(2) ? 0 : random(0));
        break;
    case 0x4790d0:
        entry.eax = random(4);
        break;
    case 0x479100:
        entry.esi = kOut;
        break;
    case 0x479160:
        entry.eax = view();
        realArgument(0, 100.0f);
        realArgument(1, 100.0f);
        argument(2, kOut);
        argument(3, kFloats);
        break;
    case 0x4792e0:
        entry.eax = random(4) == 0 ? random(4) : 9;
        entry.edx = kPoints;
        entry.ebx = kOut;
        break;
    case 0x479370:
        entry.eax = random(4) == 0 ? random(4) : 9;
        entry.edx = view();
        entry.ebx = kPoints;
        entry.ecx = kOut;
        argument(0, kFloats);
        argument(1, random(0));
        break;
    case 0x479500:
        entry.eax = view();
        realArgument(0, 100.0f);
        realArgument(1, 100.0f);
        argument(2, kFloats);
        argument(3, kOut);
        argument(4, kOut + 0x100);
        break;
    case 0x479680:
    case 0x4799d0:
    case 0x479b90:
    case 0x479ed0:
        entry.eax = view();
        break;
    case 0x479860:
        entry.eax = random(0);
        break;
    case 0x47a4c0:
        setWord32(kFloats, random(0));
        entry.eax = kFloats - 4;
        break;
    case 0x47adc0:
        entry.eax = kPoints + 12 * random(16);
        break;
    case 0x47b4f0:
        entry.eax = view();
        entry.edx = random(4) == 0 ? random(4) : 4 + random(8);
        entry.ebx = kPoints;
        entry.ecx = kPoints + 0x200;
        argument(0, kOut);
        realArgument(1, 1.0f);
        realArgument(2, 1.0f);
        break;
    default:
        break;
    }
}
