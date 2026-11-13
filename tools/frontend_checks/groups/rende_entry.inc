// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const auto argument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    constexpr x86::reg32 kPlaces = 0x90e000;                 // the cases' points and vertices, 0x40 apart
    constexpr x86::reg32 kPolygons = 0x910000;               // the cases' polygons, 0x40 apart
    x86::reg32 nextPolygon = kPolygons;
    const auto someFloat = [&]() -> float {
        const x86::reg32 pick = random(12);
        if (pick == 0)
            return 0.0f;
        if (pick == 1)
        {
            float junk;
            const x86::reg32 bits = random(0);
            std::memcpy(&junk, &bits, 4);
            return junk;
        }
        if (pick == 2)
            return float(x86::sreg32(random(0x7fffffff))) * 16.0f;     // past what sub_4c6cb0 calls far
        return float(x86::sreg32(random(4000)) - 2000) / 16.0f;
    };
    const auto place = [&](x86::reg32 at, x86::reg32 count) {
        for (x86::reg32 i = 0; i < count; ++i)
            setFloat(at + 4 * i, someFloat());
    };
    const auto point = [&](x86::reg32 index) {
        const x86::reg32 at = kPlaces + index * 0x40;
        place(at, 0x10);
        return at;
    };
    // A vertex with a depth and a clip code, often none.
    const auto vertex = [&](x86::reg32 index) {
        const x86::reg32 at = kPlaces + index * 0x40;
        setFloat(at + 8, random(10) == 0 ? someFloat() : float(random(2000)) / 4.0f);
        g_memory[at + 0x14] = x86::reg8(random(3) ? 0 : random(4) ? 1 << random(4) : random(256));
        return at;
    };
    // A list of polygons of all kinds, with depths such as sub_4c7180 gives, from here on the arena.
    const auto polygons = [&](x86::reg32 count, bool sensibleKinds) {
        x86::reg32 first = 0, previous = 0;
        for (x86::reg32 i = 0; i < count; ++i)
        {
            const x86::reg32 polygon = nextPolygon;
            nextPolygon += 0x40;
            setWord32(polygon, 0);
            g_memory[polygon + 4] = x86::reg8(sensibleKinds || random(4) ? random(4) : random(256));
            g_memory[polygon + 5] = x86::reg8(sensibleKinds || random(8) ? 0 : 1);
            for (x86::reg32 k = 0; k < 4; ++k)
                setWord32(polygon + 8 + 4 * k, vertex(random(0x20)));
            setFloat(polygon + 0x1c, float(random(2000)) / 4.0f);
            if (previous)
                setWord32(previous, polygon);
            else
                first = polygon;
            previous = polygon;
        }
        return first;
    };
    const auto someBuckets = [&](x86::reg32 table, x86::reg32 count) {
        // A few lists of their own in the table, the list's last with it.
        for (x86::reg32 n = random(6); n > 0; --n)
        {
            const x86::reg32 bucket = table + 8 * (random(4) == 0 ? count - 1 : random(count));
            const x86::reg32 first = polygons(1 + random(2), true);
            x86::reg32 last = first;
            while (word32(last) != 0)
                last = word32(last);
            if (word32(bucket) != 0)
                continue;
            setWord32(bucket, first);
            setWord32(bucket + 4, last);
        }
    };
    constexpr x86::reg32 kView = 0x90d800;
    constexpr x86::reg32 kResult = 0x90d700;
    const auto someView = [&] {
        place(kView + 0x38, 3);
        place(kView + 0x44, 9);
        setWord32(kView, random(3) == 0);
        setWord32(kView + 4, random(2));
        return kView;
    };
    switch (c.address)
    {
    case 0x4c1900:
    case 0x4c19e0:
        // eax, edx, ebx, ecx the corners, each with its code.
        entry.eax = vertex(0);
        entry.edx = vertex(1);
        entry.ebx = vertex(2);
        entry.ecx = vertex(3);
        break;
    case 0x4c6cb0:
        entry.eax = point(0);
        entry.edx = point(1);
        argument(0, random(3) == 0 ? 0 : random(3));
        break;
    case 0x4c6f00:
    case 0x4c9f70:
    case 0x4cc5c0:
    case 0x4dd930:
        entry.eax = someView();
        for (x86::reg32 at = 0; at < 3; ++at)
            setFloat(entry.esp + 4 + 4 * at, random(8) == 0 ? 0.0f : someFloat());
        if (c.address == 0x4dd930 && random(4) == 0)
        {
            // z of exactly nothing: the 65535 branch.
            for (x86::reg32 at = 0; at < 3; ++at)
                setFloat(entry.esp + 4 + 4 * at, 0.0f);
            setFloat(kView + 0x40, 0.0f);
        }
        if (c.address == 0x4dd930)
        {
            setWord32(0x8ca278, random(12));
            setWord32(0x8ca27c, random(4) == 0 ? random(4) : random(400));
            for (x86::reg32 at = 0x8c08c8; at < 0x8c08c8 + 2 * 0x190; at += 0x14)
                setWord32(at, random(3) == 0 ? 0 : random(50));
        }
        if (c.address == 0x4c9f70)
        {
            for (x86::reg32 i = 0; i < 8; ++i)
            {
                const x86::reg32 light = 0x7dd850 + i * 0x50;
                setWord32(light, random(3) != 0);
                setWord32(light + 0x4c, random(2000));
            }
        }
        break;
    case 0x4c7180:
        entry.eax = polygons(random(5), false);
        entry.edx = kResult;
        entry.ebx = kResult + 4;
        break;
    case 0x4c7260:
        // A least and a scale such as sub_4c7180 gives: 800 places over the depths.
        entry.eax = polygons(random(6), false);
        someBuckets(0x7d9ddc, 8);
        // Few places, so that a polygon often finds its bucket taken, before and after what is there.
        for (x86::reg32 at = kPolygons; at < nextPolygon; at += 0x40)
            setFloat(at + 0x1c, float(random(20)) / 4.0f);
        setFloat(entry.esp + 4, 0.0f);
        setFloat(entry.esp + 8, 1.5f);
        break;
    case 0x4c7380:
        entry.esi = kResult;
        argument(0, polygons(random(6), true));
        argument(1, random(0));
        // sub_4c7180 and sub_4c7260 as they are: the depths real, so the places are.
        for (x86::reg32 at = kPolygons; at < nextPolygon; at += 0x40)
            for (x86::reg32 k = 0; k < 4; ++k)
                setFloat(word32(at + 8 + 4 * k) + 8, float(random(2000)) / 4.0f);
        break;
    case 0x4c7400:
        argument(0, random(0));
        argument(1, random(0));
        setFloat(entry.esp + 12, random(6) == 0 ? someFloat() : float(random(600)) - 100.0f);
        break;
    case 0x4c7480:
    case 0x4c7940:
    {
        const x86::reg32 list = random(6) == 0 ? 0 : polygons(1 + random(5), false);
        someBuckets(0x7d5f5c, 2000);
        if (c.address == 0x4c7480)
        {
            entry.eax = list;
            setFloat(entry.esp + 4, float(random(100)));
        }
        else
        {
            argument(0, list);
            argument(1, random(0));
            setFloat(entry.esp + 12, float(random(100)));
        }
        break;
    }
    case 0x4c7580:
        entry.esi = kResult;
        someBuckets(0x7d5f5c, 2000);
        break;
    case 0x4c76e0:
        entry.esi = kResult;
        someBuckets(0x7db6dc, 800);
        break;
    case 0x4cb220:
    {
        entry.eax = random(2);
        for (x86::reg32 i = 0; i < 200; ++i)
        {
            const x86::reg32 fade = 0x7f0bd0 + entry.eax * 0x3200 + i * 0x40;
            setWord32(fade, random(3) == 0 ? 0 : random(40));
            setWord32(fade + 4, random(16));
        }
        break;
    }
    case 0x4cb590:
    case 0x4d4080:
        entry.eax = random(4) == 0 ? 0 : random(0);
        if (c.address == 0x4d4080)
        {
            entry.edx = 0x5f3c00 + 16 * random(16);
            entry.ebx = random(0x100);
            entry.ecx = 0x90d000;
            argument(0, random(8));
        }
        break;
    case 0x4cc540:
        entry.eax = random(2);
        setWord32(0x79c4a0 + 4 * entry.eax, random(2));
        setWord32(0x552ff4 + 4 * entry.eax, random(3) == 0);
        setWord32(0x7f6fd8 + 4 * entry.eax, random(2) ? random(5) : 0);
        break;
    case 0x4cc600:
    case 0x4dedf0:
        entry.eax = someView();
        entry.esi = kResult;
        setWord32(0x8ca22c, random(4));
        break;
    case 0x4d18b0:
        entry.eax = random(8);
        break;
    case 0x4d3380:
    {
        // A small shape, its palette and a buffer.
        const x86::reg32 shape = 0x905000;
        const x86::reg32 width = random(16), height = random(16);
        g_memory[shape + 4] = x86::reg8(width);
        g_memory[shape + 5] = 0;
        g_memory[shape + 6] = x86::reg8(height);
        g_memory[shape + 7] = 0;
        for (x86::reg32 i = 0; i < width * height; ++i)
            g_memory[shape + 0x10 + i] = x86::reg8(random(4) == 0 ? 0xff : random(256));
        entry.eax = shape;
        entry.edx = 0x906000;
        entry.ebx = 0x907000;
        break;
    }
    case 0x4d4120:
        entry.eax = random(4) == 0 ? random(0) : random(0x120);
        entry.edx = random(4) == 0 ? random(0) : random(0x120);
        break;
    case 0x4d4440:
    {
        const x86::reg32 bar = random(4);
        const x86::reg32 fields = 0x792094 + bar * 36;
        const x86::reg32 shape = 0x905000;
        g_memory[shape + 4] = x86::reg8(4 + random(8));
        g_memory[shape + 5] = 0;
        setWord32(fields + 0, 4 + random(16));
        setWord32(fields + 4, random(8));
        setWord32(fields + 8, random(8));
        setWord32(fields + 0xc, random(8));
        setWord32(fields + 0x10, random(8));
        setWord32(fields + 0x14, random(8));
        setWord32(fields + 0x18, random(8));
        setWord32(fields + 0x1c, random(2));
        entry.eax = bar;
        entry.edx = shape;
        entry.ebx = 0x906000;
        break;
    }
    case 0x4db750:
        entry.eax = random(5);
        entry.edx = point(0);
        entry.ebx = point(1);
        argument(0, random(0));
        argument(1, random(4));
        break;
    case 0x4db8b0:
        entry.eax = point(0);
        entry.edx = point(1);
        entry.ebx = point(2);
        entry.ecx = point(3);
        argument(0, kPlaces + 4 * 0x40);
        break;
    case 0x4db940:
    {
        entry.edx = random(4);
        const x86::reg32 mark = 0x8b7ba0 + (entry.eax & 0xf) * 0xd0 + entry.edx * 0x34;
        setWord32(mark, random(1200));
        setWord32(mark + 0x2c, random(2));
        setWord32(mark + 0x30, random(2));
        const auto integers = [&](x86::reg32 at) {
            for (x86::reg32 i = 0; i < 3; ++i)
                setWord32(at + 4 * i, x86::reg32(x86::sreg32(random(0x10000000)) - 0x8000000));
            return at;
        };
        entry.ebx = integers(kPlaces);
        entry.ecx = integers(kPlaces + 0x40);
        argument(0, random(2));
        const x86::reg32 pick = random(6);
        setFloat(entry.esp + 8, pick == 0 ? -0.5f : pick == 1 ? 1.5f : pick == 2 ? someFloat() : float(random(100)) / 100.0f);
        break;
    }
    default:
        break;
    }
}
