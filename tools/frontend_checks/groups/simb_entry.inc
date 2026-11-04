// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto someFloat = [&] {
        static const float kSpecial[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f };
        const x86::reg32 pick = random(8);
        return pick == 0 ? kSpecial[random(6)]
             : pick == 1 ? float(x86::sreg32(random(2000)) - 1000) / 8192.0f
                         : float(x86::sreg32(random(0x2000000)) - 0x1000000) / 65536.0f;
    };
    entry.eax = 0x900000;
    switch (c.address)
    {
    case 0x4a0aa0:
        entry.edx = random(64);
        entry.ebx = 0x907000;
        for (x86::reg32 i = 0; i < 3; ++i)
            setWord32(0x907000 + 4 * i, random(8) == 0 ? random(0) : x86::reg32(random(0x400000)) - 0x200000);
        entry.ecx = random(4) == 0 ? random(0) : random(2);
        break;
    case 0x4a1610:
        // The up it is to come to: often near the car's own, that it may be nearly there.
        for (x86::reg32 i = 0; i < 3; ++i)
            if (random(8) != 0)
                setFloat(entry.esp + 4 + 4 * i, someFloat());
        if (random(2) == 0)
            setWord32(entry.esp + 4, word32(0x900000 + 0xcc));
        if (random(2) == 0)
            setWord32(entry.esp + 12, word32(0x900000 + 0xd4));
        break;
    case 0x4a1ce0:
        entry.edx = 0x907100;
        for (x86::reg32 i = 0; i < 3; ++i)
            if (random(8) != 0)
                setFloat(0x907100 + 4 * i, someFloat());
        break;
    case 0x4a2cc0:
        entry.eax = 0x908000;
        for (x86::reg32 i = 0; i < 3; ++i)
            if (random(16) != 0)
                setFloat(entry.esp + 4 + 4 * i, random(2) ? someFloat() : (random(2) ? 0.0f : -0.0f));
        // Now and then no turn at all, of one zero or the other: only the total's sign shows those.
        if (random(4) == 0)
        {
            const float zero = random(2) ? 0.0f : -0.0f;
            for (x86::reg32 i = 0; i < 3; ++i)
                setFloat(entry.esp + 4 + 4 * i, zero);
        }
        setWord32(entry.esp + 16, 0x908100);
        setFloat(0x908100, random(4) == 0 ? (random(2) ? 0.0f : -0.0f) : someFloat());
        break;
    default:
        break;
    }
    // Of the stack's arguments' noise, no signalling NaN (simb_world.inc).
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        const x86::reg32 value = word32(entry.esp + 4 + 4 * i);
        if ((value & 0x7fc00000) == 0x7f800000 && (value & 0x3fffff) != 0)
            setWord32(entry.esp + 4 + 4 * i, value | 0x400000);
    }
}
