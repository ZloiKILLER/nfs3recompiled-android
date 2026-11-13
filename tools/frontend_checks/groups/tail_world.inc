// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    using namespace nfs3hp;
    static const bool tailStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // The system's lock, through the import table: what is there in the file.
        specs[word32(0x5344b8)] = { 0, 1, 4, 0, Kind::NoEax };
        specs[word32(0x534588)] = { 0, 1, 4, 0, Kind::NoEax };
        // Handed ebx by the generated code though the file's row does not read it: compared all the same.
        specs[0x477170] = { EAX | EBX | EDX, 0, 0, EBX, Kind::Plain };
        // A task: what the loop leaves in the registers it hands over.
        specs[kTailTask] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };

        g_methods[0x4eb230] = &nfs3hp::tailFraction;
        g_methods[0x420d60] = &nfs3hp::tailFacing;
        g_methods[0x422ba0] = &nfs3hp::tailCameraCar;
        g_methods[0x4bbe70] = &nfs3hp::tailEmptyList;
        g_methods[0x4de700] = &nfs3hp::tailStreaks;
        g_methods[0x4de070] = &nfs3hp::tailFlakes;
        g_methods[0x4ea970] = &nfs3hp::tailSineCosine;
        g_methods[0x4e0050] = &nfs3hp::tailScale;
        return true;
    }();
    (void)tailStandIns;

    const auto someFloat = [&]() -> float {
        switch (random(24))
        {
        case 0:
            return 0.0f;
        case 1:
            return -0.0f;
        case 2:
            return float(int(random(16)) - 8) / 4.0f;
        case 3:
            return tailFloatFromBits(kTailNaN);
        default:
            return float(int(random(1u << 24)) - (1 << 23)) / float(1u << (15 + random(10)));
        }
    };

    // The clock: around the countdown's steps, or anywhere small.
    static const x86::reg32 kSteps[] = { 0x140, 0x180, 0x1c0, 0x200 };
    const x86::reg32 clock = random(2) ? kSteps[random(4)] + random(3) - 1 : random(0x400);
    setWord32(0x7d3684, clock);
    setWord32(0x7d3680, random(3) == 0 ? 1 + random(5) : 0);
    setWord32(0x6fd3a0, random(2) ? 0 : 1 + random(3));
    g_memory[0x552858] = x86::reg8(random(5));
    setWord32(0x7d36a0, random(2));

    // The tasks: lists of a few, with functions or without; an empty one now and then.
    setWord32(0x7d3690, kTailTasks);
    setWord32(0x7d368c, kTailTasksLate);
    for (const x86::reg32 list : { kTailTasks, kTailTasksLate })
    {
        const x86::reg32 count = random(5) == 0 ? x86::reg32(0) - random(2) : random(5);
        setWord32(list + 4, count);
        for (x86::reg32 i = 0; i < 6; ++i)
            setWord32(list + (i << 4) + 0xc, random(3) == 0 ? 0 : kTailTask);
    }

    // The places: a list a track or none, the kinds of list each way through sub_4c8470.
    const x86::reg32 track = random(13);
    setWord32(0x6fd4ac, track);
    setWord32(0x560aa0 + track * 4, random(8) == 0 ? 0 : kTailPlaces);
    {
        const x86::reg32 places = random(6);
        x86::reg32 node = 10 + random(20);
        for (x86::reg32 p = 0; p < places; ++p)
        {
            const x86::reg32 at = kTailPlaces + p * 0x10;
            setWord32(at, node);
            node += random(6);
            setWord32(at + 4, node);
            node += random(8);
        }
        setWord32(kTailPlaces + places * 0x10, x86::reg32(random(4) == 0 ? -2 : -1));
        setWord32(kTailPlaces + (places + 1) * 0x10, x86::reg32(-1));
        setWord32(kTailPlaces + (places + 2) * 0x10, x86::reg32(-1));
    }

    // The track's nodes, their edges around the widths a lane can come to.
    setWord32(0x5dd948, kTailNodes);
    setWord32(0x5dd940, 100 + random(100));
    for (x86::reg32 n = 0; n < 32; ++n)
    {
        const x86::reg32 node = kTailNodes + n * 36;
        g_memory[node + 0x13] = x86::reg8(random(256));
        g_memory[node + 0x17] = x86::reg8(random(256));
        g_memory[node + 0x1b] = x86::reg8(random(256));
        setWord32(node + 0x1c, random(4) == 0 ? random(0) : random(0x600000));
        setWord32(node + 0x20, random(4) == 0 ? random(0) : random(0x600000));
    }

    // The weather's kinds: the file's, moved a little, or anything.
    for (x86::reg32 k = 1; k < 3; ++k)
    {
        const x86::reg32 row = 0x564290 + k * 52;
        if (random(4) == 0)
            continue;
        setWord32(row, random(0x100000));
        setFloat(row + 4, someFloat());
        setWord32(row + 8, random(120));
        setWord32(row + 0xc, random(3) == 0 ? x86::reg32(int(random(4)) - 2) : random(40));
        setWord32(row + 0x10, random(30));
        setWord32(row + 0x14, random(10));
        setWord32(row + 0x18, random(360));
        setWord32(row + 0x1c, random(10));
        setFloat(row + 0x28, someFloat());
        setWord32(row + 0x30, random(120));
    }

    // The weather as set up.
    setWord32(0x8ca22c, random(4) ? 1 + random(2) : random(4));
    setWord32(0x8ca234, random(6) == 0 ? 0 : random(30));
    setWord32(0x8ca278, random(4));
    g_memory[0x8ca1e8] = x86::reg8(random(256));
    const float width = random(8) == 0 ? someFloat() : float(1 + random(30));
    setFloat(0x8ca25c, width);
    const float low = random(8) == 0 ? someFloat() : -float(random(10));
    const float high = random(8) == 0 ? someFloat() : float(random(20));
    setFloat(0x8ca260, low);
    setFloat(0x8ca264, high);
    setFloat(0x8ca268, high - low);
    setWord32(0x6fbc30, random(4));

    // How much falls where: a stretch of nodes that may wrap, and the times.
    {
        const int first = int(random(200)) - 20, last = int(random(200)) - 20;
        g_memory[0x8ca23c] = x86::reg8(first & 0xff), g_memory[0x8ca23d] = x86::reg8((first >> 8) & 0xff);
        const int end = random(6) == 0 ? first : last;
        g_memory[0x8ca23e] = x86::reg8(end & 0xff), g_memory[0x8ca23f] = x86::reg8((end >> 8) & 0xff);
        const int fade = random(4) == 0 ? 0 : int(random(30)) - 5;
        g_memory[0x8ca240] = x86::reg8(fade & 0xff), g_memory[0x8ca241] = x86::reg8((fade >> 8) & 0xff);
        setWord32(0x8ca244, random(3) == 0 ? random(20) : 0);
        x86::reg32 at = 1 + random(50);
        for (const x86::reg32 time : { 0x8ca1f0u, 0x8ca1f4u, 0x8ca1f8u, 0x8ca1fcu })
        {
            setWord32(time, random(10) == 0 ? 0 : at);
            at += 1 + random(50);
        }
        setWord32(0x8ca248, clock - random(250));
    }

    // Each player's camera, view and car, their drops around the view.
    for (x86::reg32 p = 0; p < 2; ++p)
    {
        setWord32(kTailCameras + 4 * p, random(7));
        setWord32(kTailCameras + 8 + 4 * p, random(7));
        setWord32(0x5e10b8 + 4 * p, random(2) ? 0 : kTailCameras + 4 * p);
        setWord32(0x5e10b0 + 4 * p, random(3) == 0 ? 0 : kTailCameras + 8 + 4 * p);
        setWord32(0x79c4a0 + 4 * p, random(2));
        setWord32(0x8ca2b4 + 4 * p, random(2));
        setWord32(0x552ff4 + 4 * p, random(3) == 0 ? 1 + random(3) : 0);
        setWord32(0x8c0be0 + 4 * p, clock - random(4));
        const x86::reg32 view = kTailViews + 0x40 * p;
        setWord32(view, random(5) == 0 ? 1 + random(3) : 0);
        setWord32(view + 4, p);
        const float viewX = float(int(random(4000)) - 2000) / 8.0f, viewY = float(int(random(4000)) - 2000) / 8.0f, viewZ = float(int(random(4000)) - 2000) / 8.0f;
        setFloat(view + 8, viewX);
        setFloat(view + 0xc, viewY);
        setFloat(view + 0x10, viewZ);
        for (x86::reg32 i = 0; i < 40; ++i)
        {
            const x86::reg32 at = p * 0x2580 + i * 12;
            // Inside the box, or a little out of it by one side, or both.
            const auto off = [&](float from, float to) -> float {
                switch (random(6))
                {
                case 0:
                    return from - float(1 + random(8));
                case 1:
                    return to + float(1 + random(8));
                case 2:
                    return random(4) == 0 ? tailFloatFromBits(kTailNaN) : from;
                default:
                    return from + (to - from) * float(random(256)) / 256.0f;
                }
            };
            setFloat(0x8c56e8 + at, viewX + off(-width, width));
            setFloat(0x8c56ec + at, viewY + off(low, high));
            setFloat(0x8c56f0 + at, viewZ + off(-width, width));
            g_memory[0x8c0bf0 + at] = x86::reg8(random(2) ? 0 : 1 + random(4));
        }
        for (x86::reg32 j = 0; j < 20; ++j)
            setWord32(0x8c08c8 + p * 400 + j * 20, random(2) ? 0 : 1 + random(4));
    }
    for (x86::reg32 c = 0; c < 4; ++c)
        setWord32(kTailCars + 0x40 * c + 0x1c, x86::reg32(int(random(240)) - 30));
}
