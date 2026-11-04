// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
if (c.address == 0x45b940 || c.address == 0x45bcb0 || c.address == 0x45bd20 || c.address == 0x45c9d0)
{
    // An element: where it stands, on the mode's screen most of the time, and its record about the same.
    const x86::reg32 element = poolItem(random(kPoolItems));
    entry.eax = element;
    const auto setShort = [&](x86::reg32 at, std::int32_t value) {
        const std::uint16_t half = std::uint16_t(value);
        std::memcpy(g_memory + at, &half, 2);
    };
    const std::int32_t width = std::int32_t(word32(0x7cdac8)), height = std::int32_t(word32(0x7cdacc));
    const std::int32_t screenWidth = width > 0 ? width : 640, screenHeight = height > 0 ? height : 480;
    const std::int32_t left = random(16) == 0 ? std::int32_t(random(65536)) - 32768
        : random(4) == 0 ? 0 : std::int32_t(random(screenWidth));
    const std::int32_t top = random(16) == 0 ? std::int32_t(random(65536)) - 32768
        : random(4) == 0 ? std::int32_t(word32(0x677e10)) : std::int32_t(random(screenHeight));
    const std::int32_t wide = 1 + std::int32_t(random(screenWidth)), high = 1 + std::int32_t(random(screenHeight));
    setShort(element + 6, left);
    setShort(element + 8, top);
    // The right and the bottom: inside the screen, past 16 bits now and then, and at a stroke or less from the room's end.
    // At the room's end by a stroke exactly, as well.
    const auto farEdge = [&](std::int32_t start, std::int32_t size, std::int32_t limit, std::int32_t stroke) {
        const x86::reg32 pick = random(8);
        return pick == 0 ? x86::reg32(limit) : pick == 1 ? random(0)
            : pick == 2 ? x86::reg32(limit - stroke) : pick == 3 ? x86::reg32(limit - std::int32_t(random(stroke + 3)))
            : x86::reg32(start + size);
    };
    setWord32(element + 0x60, farEdge(left, wide, width, width / 80));
    setWord32(element + 0x64, farEdge(top, high, std::int32_t(word32(0x677e14)), width / 80));
    setShort(element + 0x1c, std::int32_t(random(640)));
    setShort(element + 0x1a, std::int32_t(random(480)));
    // The sizes: often just over the least they may be, where a stroke is cut short.
    setShort(element + 0x1e, random(2) ? 16640 / screenWidth + std::int32_t(random(16)) : std::int32_t(random(640)));
    setShort(element + 0x20, random(2) ? 12480 / screenHeight + std::int32_t(random(16)) : std::int32_t(random(480)));
    static const x86::reg32 kNumbers[] = { 17, 22, 40, 45, 23, 24, 6, 13, 15, 20, 19, 0 };
    setWord32(element + 0x3c, random(3) == 0 ? random(46) : random(12) == 0 ? random(0) : kNumbers[random(12)]);
    static const x86::reg32 kGrips[] = { 0, 0, 0x409, 0x809, 0x108, 0x208, 0x50a, 0xa0a };
    setWord32(element + 0x78, random(8) == 0 ? random(0) : kGrips[random(8)]);
    setWord32(element + 0x70, random(2));
    if (c.address == 0x45b940)
    {
        // The pointer, often at an edge: within the grip's reach of it, by the scale.
        const auto nearEdge = [&](std::int32_t edge, std::int32_t screen, std::int32_t units) {
            return random(4) == 0 ? random(units) : x86::reg32((edge + std::int32_t(random(17)) - 8) * units / screen);
        };
        setWord32(0x55e5e4, nearEdge(random(2) ? left : left + wide, screenWidth, 640));
        setWord32(0x55e5e8, nearEdge(random(2) ? top : top + high, screenHeight, 480));
        setShort(element + 0x7c, random(4) == 0);
        setShort(element + 0x7e, random(4) == 0);
        // The far edges where the pointer is, in 32 bits or past them by a multiple of 16 bits.
        setWord32(element + 0x60, x86::reg32(left + wide) + (random(4) == 0 ? 0x10000 : 0));
        setWord32(element + 0x64, x86::reg32(top + high) + (random(4) == 0 ? 0x10000 : 0));
    }
    else if (c.address == 0x45bd20)
    {
        // Held or not, and the keys it answers to.
        setShort(element + 0x7c, std::int32_t(random(2)));
        setShort(element + 0x7e, random(2) ? 0 : random(8) == 0 ? std::int32_t(random(65536)) : 1);
        static const x86::reg32 kKeys[] = { 0x4b00, 0x4d00, 0x4800, 0x5000 };
        x86::reg32 key = random(16) == 0 ? 0x0d : random(16) == 0 ? random(65536) : kKeys[random(4)];
        // Half the time a key its grip answers to, held: a stroke that does something.
        if (random(2))
        {
            const x86::reg32 grip = word32(element + 0x78);
            const bool across = grip == 0x409 || grip == 0x809;
            const bool down = grip == 0x108 || grip == 0x208;
            if (across || down)
            {
                key = across ? kKeys[random(2)] : kKeys[2 + random(2)];
                setShort(element + 0x7e, 1);
            }
        }
        entry.ebx = (random(0) & 0xffff0000) | key;
        // The few sizes where a quotient and a product by the inverse truncate differently, on the numbers that do.
        if (random(8) == 0)
        {
            static const x86::reg32 kAwkward[] = { 115, 230, 1840 };
            const x86::reg32 awkward = kAwkward[random(3)];
            const std::int32_t stroke = std::int32_t(awkward / 80);
            setWord32(0x7cdac8, awkward);
            setWord32(0x7cdacc, awkward);
            setWord32(0x677e10, 0);
            setShort(element + 6, std::int32_t(awkward * (1 + random(8))) + stroke * (std::int32_t(random(3)) - 1));
            setShort(element + 8, std::int32_t(awkward * (1 + random(8))) + stroke * (std::int32_t(random(3)) - 1));
            if (random(2))
                setShort(element + 0x20, 1);
        }
    }
    else if (c.address == 0x45bcb0)
        entry.edx = random(3) == 0 ? random(0) : random(2);
}
else if (c.address == 0x4a4a10)
{
    const x86::reg32 item = poolItem(random(kPoolItems));
    entry.eax = item;
    entry.edx = random(2) ? 0 : random(2) ? 1 : random(0);
    setWord32(item + 0x40, random(2) ? 0 : 0x5f3f20);
    setWord32(item + 0x3c, random(2) ? 0 : 0x5f3f06);
}
