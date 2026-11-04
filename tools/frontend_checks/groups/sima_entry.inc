// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const auto someCar = [&] { return kSimaCars + kSimaCarBytes * random(kSimaCarCount); };
    const auto someCamera = [&] { return kSimaCameras + kSimaCameraBytes * random(kSimaCameraCount); };
    const auto richFloat = [&] { return float(1 + random((1u << 24) - 1)) / float(1u << (15 + random(10))); };
    const auto size = [&]() -> float {
        switch (random(10))
        {
        case 0:
            return 0.0f;
        case 1:
            return simaBits(random(0));
        case 2:
            return simaBits(kSimaNaN);
        case 3:
            return -richFloat();
        default:
            return richFloat();
        }
    };
    switch (c.address)
    {
    case 0x4064f0:
    {
        const x86::reg32 car = kSimaCars + kSimaCarBytes * (1 + random(kSimaCarCount - 1));
        entry.eax = car;
        // Not held back most of the time; when it is, a limit that comes out to the bit as the
        // pedal, or a hair off it only before the product is rounded.
        if (random(4) != 0)
            g_memory[car + 0x200] &= ~x86::reg8(1);
        const float limit = richFloat(), throttle = richFloat();
        setFloat(kSimaDriving + 0x1cc, limit);
        setFloat(car + 0x7bc, throttle);
        if (random(2))
            setFloat(car + 0x7c0, float(fp::mul(x86::Float(limit), x86::Float(throttle))));
        else if (random(2))
            setFloat(car + 0x7c0, limit);
        // Each row where something changes, and either side of it.
        static const int kRows[] = { -1, 0, 1, 2, 3, 44, 45, 46, 110, 111, 112, 113, 200 };
        const int row = kRows[random(13)];
        setFloat(car + 0x7c4, float(row) + 0.5f + (random(2) ? 0.0f : 0.25f));
        // The row a signalling NaN now and then, alone in its product: quieted on the way, as the x87 does.
        if (random(8) == 0)
            setWord32(kSimaDriving + 0x2ec + 4 * x86::reg32(row < 0 ? 0 : row > 0x6f ? 0x6f : row), 0x7f800001);
        // The catch-up's gaps, each side of each limit.
        static const int kGaps[] = { -9, -8, -7, -4, -3, -2, 14, 15, 16, 29, 30, 31, 0 };
        setWord32(0x6fd50c, random(4) != 0);
        setWord32(0x5efa48 + 4 * word32(0x6fd4f4), kSimaCars);
        setWord32(car + 0x1c, word32(kSimaCars + 0x1c) + x86::reg32(kGaps[random(13)]));
        g_memory[0x7a22da] = x86::reg8(random(2));
        g_memory[0x7a22db] = 0;
        setWord32(car + 0x1f0, random(3) == 0 ? word32(0x6fd518) - random(2) : 2 + random(6));
        break;
    }
    case 0x425d50:
    {
        const x86::reg32 camera = someCamera();
        entry.eax = camera;
        const x86::reg32 view = word32(camera + 0xc), car = word32(camera + 4);
        setWord32(0x7d36a8, 0);
        setWord32(0x5600ec, random(16) == 0);
        const x86::reg32 slices = word32(0x5dd940);
        // A step short of the gear's end, the car a few nodes from it either way, at the slices' ends too.
        const x86::reg32 gear = word32(0x553014 + 4 * view);
        const x86::reg32 steps = word32(0x423d70 + 4 * gear);
        if (random(4) != 0)
            setWord32(0x553004 + 4 * view, word32(0x7d3680) != 0 ? steps - 1 : steps);
        const int place = random(4) == 0 ? int(slices) - 1 - int(random(3)) : random(4) == 0 ? int(random(3)) : int(random(slices));
        setWord32(car + 0x1c, x86::reg32(place < 0 ? 0 : place));
        const int gap = int(random(20)) - 10;
        int node = place - gap - 1;
        while (node < 0)
            node += int(slices);
        while (node >= int(slices))
            node -= int(slices);
        setWord32(0x55300c + 4 * view, x86::reg32(node));
        // A pace near its gear's, or no number; not past the table's last gear (sima_world.inc).
        if (random(8) == 0)
        {
            setFloat(car + 0x7c0, std::numeric_limits<float>::quiet_NaN());
        }
        else
        {
            // Off it by its limits to the bit, and by anything.
            static const float kOff[] = { 5.0f, -5.0f, 0.0f, 3.0f, 15.0f, -8.0f };
            const float off = random(4) == 0 ? kOff[random(6)] : float(int(random(1u << 20)) - (1 << 19)) / float(1 << 16);
            const float pace = simaBits(word32(0x423d50 + 4 * gear)) + off;
            setFloat(car + 0x7c0, pace > 140.0f ? 140.0f : pace);
        }
        // The car close by and the track small, for the last bits of the drop and the height to show.
        if (random(2))
        {
            for (x86::reg32 s = 0; s < slices; ++s)
                for (x86::reg32 axis = 0; axis < 3; ++axis)
                    setWord32(kSimaSlices + kSimaSliceBytes * s + 4 * axis, x86::reg32(int(random(1u << 17)) - (1 << 16)));
            setFloat(car + 0x98, richFloat() / 256.0f);
            setFloat(car + 0xa0, richFloat() / 256.0f);
        }
        break;
    }
    case 0x425640:
    {
        const x86::reg32 camera = someCamera();
        entry.eax = camera;
        if (word32(camera + 0x118) == x86::reg32(-1) && random(4) != 0)
            setWord32(camera + 0x118, random(8));
        setWord32(0x6fd3b0, random(4) != 0 ? 2 : random(2));
        setWord32(0x6fd518, random(5));
        for (x86::reg32 i = 0; i < kSimaCarCount; ++i)
        {
            const x86::reg32 car = kSimaCars + kSimaCarBytes * i;
            setWord32(car + 0x2c4, random(4) == 0);
            g_memory[car + 0x200] = x86::reg8(random(2) ? 0x80 * random(2) + random(2) : random(256));
        }
        break;
    }
    case 0x4251f0:
    case 0x4253d0:
    case 0x425060:
    case 0x425bf0:
    case 0x426290:
        entry.eax = someCamera();
        if (random(2))
            setWord32(0x7d36a8, 0), setWord32(0x5600ec, 0);
        break;
    case 0x4262a0:
    {
        const x86::reg32 camera = someCamera();
        entry.eax = camera;
        setWord32(camera + 0x118, random(8) == 0 ? 0 : 1);
        // A time that is no number but, as an integer, past 1.0's bits: the original compares integers.
        if (random(8) == 0)
            setWord32(camera + 0x110, 0x7fc00000);
        break;
    }
    case 0x426310:
    {
        const x86::reg32 view = random(2);
        entry.eax = view;
        const x86::reg32 camera = word32(0x5e10b0 + 4 * view);
        if (camera != 0 && random(3) != 0)
            setWord32(camera, 0x13);
        break;
    }
    case 0x426560:
    {
        const x86::reg32 view = random(2);
        entry.eax = view;
        const x86::reg32 forced = word32(0x5e10b8 + 4 * view);
        if (forced != 0 && random(2))
            setWord32(forced, 0x18);
        const x86::reg32 shown = forced != 0 ? forced : word32(0x5e10b0 + 4 * view);
        const x86::reg32 car = word32(shown + 4);
        if (car != 0 && random(2))
        {
            // None of what forces a mode, or only the last.
            setWord32(0x7d36a0, 0);
            setWord32(car + 0x9a4, 0);
            setWord32(car + 0x99c, 0);
            setWord32(car + 0x184, random(2) ? 0 : x86::reg32(-1));
            setWord32(0x6fd4f8, word32(0x6fd4f4));
        }
        break;
    }
    case 0x426680:
    {
        setWord32(0x6fd3b0, random(2) ? 1 : random(3));
        for (x86::reg32 view = 0; view < 2; ++view)
        {
            const x86::reg32 camera = word32(0x5e10b0 + 4 * view);
            if (camera == 0)
                continue;
            if (random(2))
                setWord32(camera, 0x10 + random(2));
            if (random(2))
                setWord32(camera + 0x11c, 0);
            const x86::reg32 car = word32(camera + 4);
            g_memory[car + 0x200] |= x86::reg8(random(4) != 0);
            if (random(4) != 0)
                setWord32(car + 0x2c4, 3);
            if (random(2))
                setWord32(camera + 0x118, x86::reg32(-1));
        }
        break;
    }
    case 0x426450:
        entry.eax = random(27);
        entry.edx = random(2);
        break;
    case 0x4965b0:
        for (x86::reg32 i = 0; i < 6; ++i)
            setFloat(kSimaPoints + 4 * i, size());
        // A way of -0, which the original keeps.
        for (x86::reg32 axis = 0; axis < 3; ++axis)
            if (random(4) == 0)
            {
                setFloat(kSimaPoints + 4 * axis, -0.0f);
                setFloat(kSimaPoints + 12 + 4 * axis, 0.0f);
            }
        entry.eax = kSimaPoints;
        entry.edx = kSimaPoints + 12;
        break;
    case 0x4967e0:
    {
        // Sizes of every proportion, for each of its shares: the second often a share of the first.
        const float first = size();
        setFloat(entry.esp + 4, first);
        setFloat(entry.esp + 8, random(2) ? size() : first * float(random(32)) / 16.0f);
        break;
    }
    case 0x49fde0:
    {
        const x86::reg32 car = someCar();
        entry.eax = car;
        // Each side of the limits, and no numbers.
        static const float kLeans[] = { -0.95f, -0.9f, -0.85f, 0.4f, 0.5f, 0.6f, 0.85f, 0.9f, 0.95f, -0.0f, 0.0f, 1.5f };
        setFloat(car + 0x10c, random(4) == 0 ? size() : kLeans[random(12)]);
        break;
    }
    case 0x49fee0:
        entry.eax = random(word32(0x5dd940));
        break;
    case 0x4a00c0:
        entry.eax = random(64);
        entry.edx = random(64);
        break;
    case 0x4a0580:
    {
        const x86::reg32 car = someCar();
        entry.eax = car;
        setWord32(0x6fd3b0, random(4) != 0 ? 0 : random(3));
        setWord32(0x5efda4, random(4) != 0 ? 1 + random(2) : random(4));
        break;
    }
    case 0x4a0140:
    {
        const x86::reg32 car = someCar();
        entry.eax = car;
        // A signalling NaN among the sizes, for corners only stand-ins see: quieted when negated.
        if (random(8) == 0)
            setWord32(car + 0x114 + 4 * random(3), 0x7f800001);
        break;
    }
    case 0x4070c0:
    case 0x40de30:
    case 0x49ffa0:
    case 0x4a08c0:
        entry.eax = someCar();
        break;
    default:
        break;
    }
}
