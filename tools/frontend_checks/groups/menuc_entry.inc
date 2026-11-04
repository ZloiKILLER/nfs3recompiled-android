// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const x86::reg32 at = c.address;
    auto setWord16 = [](x86::reg32 where, std::uint32_t value) {
        const std::uint16_t word = std::uint16_t(value);
        std::memcpy(g_memory + where, &word, 2);
    };
    // Which widget this table is, by its functions' addresses.
    const bool results = at >= 0x46cdc0 && at < 0x46e660;
    const bool standings = at >= 0x46e660 && at < 0x470360;
    const bool sideBySide = at >= 0x470360 && at < 0x470a60;
    if (at == 0x46cce0 || at == 0x4654c0)
    {
        setWord32(entry.esp + 4, random(0));
        setWord32(entry.esp + 8, random(0));
        setWord32(entry.esp + 12, random(0));
    }
    if (at == 0x465d80)
        entry.eax = random(6) == 0 ? 0 : random(0);
    if (results || standings || sideBySide)
    {
        const x86::reg32 item = entry.eax ? entry.eax : poolItem(0);
        entry.eax = item;
        // The fields a table's loops run over and its widths: on the screen,
        // the widths now and then anything.
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord16(item + 0x3c + 2 * i, random(8) == 0 ? random(0x10000) : 20 + random(200));
        setWord16(item + 0x06, random(640));
        setWord16(item + 0x08, random(480));
        if (results)
        {
            setWord16(item + 0x44, 8 + random(30));
            setWord16(item + 0x46, random(8) == 0 ? 0 : random(12));
            setWord16(item + 0x84, random(3) == 0 ? 0 : random(12));
            setWord16(item + 0x86, random(6));
            setWord16(item + 0x88, random(4) == 0 ? random(5) : 0);
            setWord16(item + 0x8a, random(3) == 0 ? 0x290 + random(0x30) : random(4) == 0 ? 0 : random(640));
        }
        if (standings)
        {
            setWord16(item + 0x42, 8 + random(30));
            setWord16(item + 0x44, random(8) == 0 ? 0 : random(12));
            setWord16(item + 0x7a, random(3) == 0 ? 0 : random(12));
            setWord16(item + 0x7c, random(4));
            setWord16(item + 0x7e, random(4) == 0 ? random(5) : 0);
            setWord16(item + 0x80, random(3) == 0 ? 0x290 + random(0x30) : random(4) == 0 ? 0 : random(640));
        }
        if (sideBySide)
        {
            setWord16(item + 0x40, 20 + random(100));
            setWord16(item + 0x42, 8 + random(30));
            setWord16(item + 0x6c, random(4) == 0 ? random(5) : 0);
            setWord16(item + 0x6e, random(3) == 0 ? 0x290 + random(0x30) : random(4) == 0 ? 0 : random(640));
        }
    }
    // Sliding in, a step from where it stops.
    if ((results || standings || sideBySide) && random(4) == 0)
    {
        const x86::reg32 fields = results ? 0x88 : standings ? 0x7e : 0x6c;
        setWord16(entry.eax + fields, 1);
        setWord16(entry.eax + fields + 2, word32(entry.eax + 6) + 0x14);
    }
    if (at == 0x46e270 || at == 0x46f200 || at == 0x470040)
    {
        static const x86::reg32 kKeys[] = { 0x0d, 0x0d, 0x4d00, 0x4b00, 0x4900, 0x5100, 0x1b };
        entry.ebx = (random(0) & 0xffff0000) | kKeys[random(7)];
    }
}
