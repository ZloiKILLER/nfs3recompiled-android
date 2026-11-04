// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
if ((c.address >= 0x44de00 && c.address < 0x44e130) || c.address == 0x45ca90
    || (c.address >= 0x45dae0 && c.address < 0x45fae0) || c.address == 0x485680)
{
    // The tables of names, read as strings: as the exe has them, not at random.
    std::memcpy(g_memory + 0x559cbc, g_image.data() + (0x559cbc - 0x534000), 0x559e5c + 17 * 20 - 0x559cbc);
    // The records and the lists: small numbers, each side of what is compared.
    for (x86::reg32 player = 0; player < 3; ++player)
    {
        for (x86::reg32 cop = 0; cop < 2; ++cop)
        {
            const x86::reg32 fields = 0x6fbc50 + player * 0x348 + cop * 0x1a4;
            const x86::reg32 at = player * 8 + cop * 4;
            setWord32(fields, random(4) != 0);
            setWord32(fields + 4, random(2));
            setWord32(fields + 8, random(2));
            setWord32(fields + 0xc, random(2));
            if (random(10) != 0)
                setFloat(fields + 0x10, float(random(70)) / 10.0f - 1.0f);
            setWord32(fields + 0x14, random(7));
            setWord32(fields + 0x18, random(6));
            setWord32(fields + 0x1c, random(5));
            setWord32(fields + 0x20, random(4));
            setWord32(0x6787f8 + at, random(4));
            setWord32(0x678810 + at, random(5));
            setWord32(0x678828 + at, random(7));
            setWord32(0x678840 + at, random(6));
            setWord32(0x678858 + at, random(4));
        }
        // What is shown, often what the lists say.
        const x86::reg32 at = player * 8 + random(2) * 4;
        const x86::reg32 fields = 0x6fbc50 + player * 0x348 + (at & 4 ? 0x1a4 : 0);
        const x86::reg32 own = 4 * player;
        setWord32(0x678874 + own, random(3) ? word32(0x678810 + at) : random(5));
        setWord32(0x678884 + own, random(3) ? word32(0x678840 + at) : random(6));
        setWord32(0x678890 + own, random(3) ? word32(0x678858 + at) : random(4));
        setWord32(0x67889c + own, random(3) ? word32(fields + 4) : random(2));
        setWord32(0x6788a8 + own, random(3) ? word32(0x6787f8 + at) : random(4));
        setWord32(0x6788bc + own, random(3) ? word32(fields + 8) : random(2));
        setWord32(0x6788cc + own, random(3) ? word32(0x678828 + at) : random(7));
    }
    setWord32(0x678880, random(3));
    setWord32(0x678900, random(8));
    setWord32(0x6788b4, random(2) ? word32(0x678900) : random(8));
    setWord32(0x678904, random(2));
    setWord32(0x6788c8, random(2) ? word32(0x678904) : random(3));
    // The list of whose HUD is laid out.
    setWord32(0x6788b8, 0x678a00);
    static const x86::reg32 kHudeditChoices[] = { 0x19d, 0x19e, 0x1a0, 0x1a1, 0x1a3, 0x1a4 };
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 choice = kHudeditChoices[random(6)];
        std::memcpy(g_memory + 0x678a00 + 2 * i, &choice, 2);
    }
    // The miniature's art names, for the items.
    static const char* const kHudeditArt[] = { "splb", "spl1", "spl2", "splt", "scrb", "scrn" };
    for (x86::reg32 i = 0; i < 6; ++i)
        std::strcpy(reinterpret_cast<char*>(g_memory) + 0x678a40 + 8 * i, kHudeditArt[i]);
    // And for their art's file: a short name, the map's path has 0x5c bytes.
    for (std::uint32_t i = 0; i < kPoolItems; ++i)
    {
        setWord32(poolItem(i) + 0x3c, 0x678a40 + 8 * random(6));
        setWord32(poolItem(i) + 0x40, 0x678a40 + 8 * random(6));
    }
    setWord32(0x55d440, random(2));
}
if (c.address == 0x45dae0 || c.address == 0x45deb0)
{
    entry.eax = random(3);
    entry.edx = random(2);
    entry.ebx = g_menu;
}
if (c.address == 0x45e3d0 || c.address == 0x45f300)
{
    entry.eax = g_menu;
    entry.edx = random(3);
    entry.ebx = random(2);
}
if (c.address == 0x45ca90 || c.address == 0x44de20)
{
    // An element's number in its item, and elements among the screen's items.
    for (x86::reg32 item = kItems; item < 0x608000; item += kItemSize)
    {
        setWord32(item + 0x3c, random(46));
        if (c.address == 0x44de20 && random(5) == 0 && word32(item) != 0)
            setWord32(item, 0x1f);
    }
}
