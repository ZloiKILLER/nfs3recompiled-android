// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x422bd0].kind = Kind::Shape;     // the car a view looks at: memory to read
        specs[0x44aeb0].kind = Kind::Shape;     // the screen shown: likewise
        specs[0x47bc20].kind = Kind::Bool;
        return true;
    }();
    (void)standInsAdded;

    // A fraction of the screen: mostly one, now and then past it.
    const auto fraction = [] { return float(random(1200)) / 1000.0f - 0.1f; };

    // The records' rectangles: boxes of a sensible size, as often overlapping as not.
    for (x86::reg32 record = 0x6fbc50; record < 0x6fbc50 + 4 * 0x348; record += 0x1a4)
    {
        setWord32(record + 0x18, random(4));
        setWord32(record + 0x20, random(3) == 0 ? 0 : 1 + random(3));
        for (x86::reg32 r = 0; r < 24; ++r)
        {
            const x86::reg32 at = record + 0x24 + r * 16;
            const float top = fraction(), left = fraction();
            setFloat(at, top);
            setFloat(at + 4, left);
            setFloat(at + 8, top + float(random(800)) / 1000.0f);
            setFloat(at + 12, left + float(random(800)) / 1000.0f);
            if (random(24) == 0)
                setWord32(at + 4 * random(4), random(0));
            // A NaN, whichever side of a comparison it comes on showing.
            if (random(16) == 0)
                setWord32(at + 4 * random(4), random(2) ? 0xffc00000 : 0x7fc00000);
        }
    }
    // What a car's and a screen's stand-in hands out, 16 places: its flag, its first item.
    for (x86::reg32 k = 0; k < 16; ++k)
    {
        g_memory[0x5f3c00 + 16 * k + 0x200] = x86::reg8(random(2) ? 0x20 : random(256));
        setWord32(0x5f3c00 + 16 * k + 0x18, (kPoolFirst + random(10)) | (random(0) << 16));
    }
    // The screen's items: elements of either view among others, ended somewhere.
    for (std::uint32_t i = 0; i < kPoolItems + 10; ++i)
    {
        const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
        setWord32(item, random(3) == 0 ? 5 : 0x1f);
        setWord32(item + 0x3c, random(46));
    }
    setWord32(kItems + kItemSize * (kPoolFirst + 10 + random(kPoolItems)), 0);

    setWord32(0x7cdac8, random(8) == 0 ? random(0) : random(16) == 0 ? 0 : 320 + random(1024));
    setWord32(0x7cdacc, random(8) == 0 ? random(0) : random(16) == 0 ? 0 : 240 + random(768));
    setWord32(0x6fd3b0, random(3));
    setWord32(0x6fd4c8, random(2) ? 0 : random(0));
    g_memory[0x7a3a58] = x86::reg8(random(3) == 0 ? random(256) : 0x10);

    // The boxes: mostly the screen's numbers.
    for (x86::reg32 at = 0x749758; at < 0x749758 + 4 * 0x170; at += 4)
        setWord32(at, random(5) == 0 ? random(0) : random(640) - 40);

    // What the views have queued and highlighted.
    for (x86::reg32 v = 0; v < 4; ++v)
    {
        setWord32(0x700240 + 4 * v, random(8));
        setWord32(0x55d338 + 4 * v, random(4) == 0 ? x86::reg32(-1) - random(4) : random(8));
    }

    // The batches: where a view's quads are, in the cases' memory.
    setWord32(0x791a58, 0x79c000 + 0x20 * random(8));
    setWord32(0x791ae0, 0x79e000 + 0x20 * random(8));
    setWord32(0x791ae4, 0x79c000 + 0x20 * random(8));
    setWord32(0x791a84, random(0x1000));
    setWord32(0x791a88, random(0x1000));

    // Corners: small numbers around the clip's, depths around its far plane, signs now and then.
    setWord32(0x7d34f8, random(2) ? 0x7f000000 : random(0x40000000));
    setWord32(0x7d34fc, random(4) == 0 ? x86::reg32(-5) : 4);
    setWord32(0x7d350c, 12);
    setWord32(0x7d3504, random(4) == 0 ? x86::reg32(-5) : 4);
    setWord32(0x7d3500, 12);
    for (x86::reg32 at = 0x79c000; at < 0x79e000; at += 0x20)
    {
        setWord32(at, random(4) == 0 ? random(0) : random(20) - 2);
        setWord32(at + 4, random(4) == 0 ? random(0) : random(20) - 2);
        setWord32(at + 0xc, random(3) == 0 ? random(0) : random(0x3f800000));
    }

    // The grid's tables: the exe's half the time, or sizes of their kind.
    if (random(2))
        std::memcpy(g_memory + 0x55d3b8, g_image.data() + (0x55d3b8 - 0x534000), 0x88);
    else if (random(2))
        for (x86::reg32 at = 0x55d3b8; at < 0x55d440; at += 4)
            setFloat(at, float(random(4000)) / 64.0f);

    // The replay bar's state.
    setWord32(0x7cd954, random(12));
    setWord32(0x7cd948, random(2));
    setWord32(0x7cd94c, random(6));
    setWord32(0x7cd958, random(4));
}
