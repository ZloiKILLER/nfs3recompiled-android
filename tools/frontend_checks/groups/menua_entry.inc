// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const x86::reg32 at = c.address;
    // A key that is one of those the functions look for, usually.
    static const x86::reg32 kKeys[] = { 0xd, 0x1b, 9, 8, 0x5300, 0x4800, 0x4900, 0x4b00, 0x4d00, 0x5000, 0x5100, 0x41, 0, 0x2000 };
    const x86::reg32 key = random(8) == 0 ? random(0x10000) : kKeys[random(14)];
    // A binding of every kind, with what an axis's picture depends on.
    const auto someBinding = [&]() -> x86::reg32 {
        static const x86::reg32 kFrom[] = { 0x80, 0x7f, 0x00, 0xff, 0x10 };
        static const x86::reg32 kLow[] = { 0xff, 0x00, 0x10 };
        const x86::reg32 kind = random(6);
        return (random(16) << 28) | (random(8) << 24) | (kFrom[random(5)] << 16) | (kLow[random(3)] << 8) | kind;
    };
    // The language's line copied onto the stack, where this is: a short one.
    const bool copiesText = at == 0x43d7f0 || at == 0x43c110 || at == 0x43c900;
    const_cast<Spec&>(kSpecs.at(0x4d1850)).kind = copiesText ? Kind::EditText : Kind::Plain;
    // The row under the pointer, whose generated callers call it by name, past g_methods: a small
    // number, and, where it is only compared, any at all; a large one where Enter would read its slot.
    // At the list's first rows, often.
    const x86::reg32 rowKind = random(4);
    const_cast<Spec&>(kSpecs.at(0x49c1c0)).kind = rowKind == 0 ? Kind::Bool : rowKind == 1 ? Kind::KeyHandler
        : (at == 0x43cf80 && key == 0xd) ? Kind::Shape : Kind::Plain;

    if (at == 0x43c110)
    {
        entry.edx = random(4) == 0 ? random(0) : someBinding();
        // A key's name is in its 16 bits: now and then one that has one.
        if ((entry.edx & 0xff) == 4 && random(2))
            entry.edx = (entry.edx & 0xff0000ff) | ((random(0x60)) << 8);
        entry.ebx = random(640);
        entry.ecx = random(480);
        setWord32(entry.esp + 4, random(2));
    }
    else if (at == 0x43c900)
        entry.edx = random(2);
    else if (at == 0x43cda0)
        entry.eax = random(4) == 0 ? random(0) : random(4) == 0 ? x86::reg32(0) - random(2) : random(0x100);
    else if (at == 0x43ce80)
    {
        entry.eax = x86::reg32(random(32)) - 2;
        entry.edx = someBinding();
    }
    else if (at == 0x43cf80)
    {
        // The arrows, often.
        static const x86::reg32 kArrows[] = { 0x4800, 0x4b00, 0x4d00, 0x5000 };
        entry.ebx = (random(0) & 0xffff0000) | (random(2) ? kArrows[random(4)] : key);
    }
    else if (at == 0x43ec40)
        entry.ebx = (random(0) & 0xffff0000) | key;
    else if (at == 0x43d6a0)
    {
        entry.eax = random(8) == 0 ? 0 : random(2) ? someBinding() : word32(0x6fd20c + 4 * random(26));
        if (random(3) == 0)
            entry.eax = (entry.eax & 0xffffff00) | 1;
    }
    else if (at == 0x43d7f0)
    {
        entry.eax = random(2) ? random(0) : (random(0x60) << 8) | (random(0) & 0xff0000ff);
        entry.edx = 0x5f4500;
    }
    else if (at == 0x43de20)
    {
        entry.eax = 0x5f4600;
        entry.edx = 0x5f4620;
    }
    else if (at == 0x43de60)
    {
        entry.eax = random(4) == 0 ? random(0) : random(200);
        entry.edx = random(4) == 0 ? random(0) : random(200);
        entry.ebx = random(4) == 0 ? random(0) : random(300);
        entry.ecx = random(4) == 0 ? random(0) : random(300);
        setWord32(entry.esp + 4, 0x5f4600);
        setWord32(entry.esp + 8, 0x5f4604);
    }
    else if (at == 0x43def0)
    {
        entry.eax = random(4) == 0 ? random(0) : random(200);
        entry.edx = 1 + random(200);
        entry.ebx = 0x5f4600;
        entry.ecx = 0x5f4604;
        setWord32(entry.esp + 4, random(4) == 0 ? random(0) : random(240));
        setWord32(entry.esp + 8, random(4) == 0 ? random(0) : random(240));
    }
    else if (at == 0x440b50)
    {
        entry.eax = random(2);
        entry.edx = random(8);
        entry.ebx = random(640);
        entry.ecx = random(480);
    }
    else if (at == 0x441700 || at == 0x441b20)
        entry.eax = (random(0) & 0xffff0000) | key;
    else if (at == 0x441900)
        entry.eax = random(8) == 0 ? 0 : random(4) == 0 ? 0x5f4300 + 0x57 : 0x5f4300;
    else if (at == 0x441950)
    {
        entry.eax = random(640);
        entry.edx = random(480);
    }
    else if (at == 0x4434f0 || at == 0x443510 || at == 0x443620 || at == 0x4435c0 || at == 0x4436c0 || at == 0x443740 || at == 0x4437c0)
        entry.eax = random(4) == 0 ? random(0) : (random(0) & 0xffffff00) | random(256);
    else if (at == 0x443520 || at == 0x4435e0)
        entry.eax = random(8) == 0 ? 0 : 0x5f4380;
    else if (at == 0x43e8e0 || at == 0x43ead0 || at == 0x43ebc0 || at == 0x43e7a0)
    {
        // The pointer on one of the picker's areas, usually the one pressed.
        static const x86::reg32 kAreas[] = { 0x7c, 0x84, 0x64, 0x6c };
        std::int16_t x, y, top, left, width, height;
        const x86::reg32 which = random(4);
        const x86::reg32 area = kAreas[which];
        std::memcpy(&x, g_memory + entry.eax + 6, 2);
        std::memcpy(&y, g_memory + entry.eax + 8, 2);
        std::memcpy(&top, g_memory + entry.eax + area, 2);
        std::memcpy(&left, g_memory + entry.eax + area + 2, 2);
        std::memcpy(&width, g_memory + entry.eax + area + 4, 2);
        std::memcpy(&height, g_memory + entry.eax + area + 6, 2);
        if (random(5) != 0)
        {
            setWord32(0x55e5e4, x + left + width / 2);
            setWord32(0x55e5e8, y + top + height / 2);
        }
        static const std::uint16_t kHits[] = { 1, 2, 3, 4 };
        if (random(3) == 0)
            std::memcpy(g_memory + entry.eax + 0x8e, &kHits[which], 2);
    }
    else if (at == 0x443810)
    {
        entry.eax = random(0);
        entry.edx = random(4) == 0 ? random(0) : random(5);
    }
    else if (at == 0x448330)
        entry.esi = 0x5f4700;
    else if (at == 0x4483a0)
        entry.eax = random(2) ? random(0) : word32(0x661220 + 0x114 * random(12));
}
if (c.address == 0x448210)
{
    // A mode from the driver's list: often the list full, and now and then past it.
    entry.eax = 320 + random(1600);
    entry.edx = 200 + random(1000);
    entry.ebx = random(64);
    entry.ecx = random(3) == 0 ? 16 : random(33);
    setWord32(entry.esp + 4, random(2));
    setWord32(0x559218, random(4) == 0 ? 20 + random(3) : random(20));
}
