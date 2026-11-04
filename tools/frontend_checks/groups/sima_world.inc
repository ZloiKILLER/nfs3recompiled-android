// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What these functions call and the check's table lacks: the registers each reads,
    // its stack arguments, what its ret pops, what it changes besides eax (read off nfs3.exe).
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto plain = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0, std::uint8_t clobbers = 0) {
            specs[at] = { in, stack, pops, clobbers, Kind::Plain };
        };
        const auto noEax = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0) {
            specs[at] = { in, stack, pops, 0, Kind::NoEax };
        };
        const auto yesNo = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0) {
            specs[at] = { in, stack, pops, 0, Kind::Bool };
        };
        plain(0x40da30, EAX);
        plain(0x40daf0, EAX);
        plain(0x406e50, EAX | ESI, 0, 0, ESI);
        plain(0x4e06d9, 0);
        yesNo(0x4bdc70, EAX);
        yesNo(0x423fe0, EAX);
        plain(0x41ffd0, EAX | EBX | EDX, 0, 0, EBX | EDX);
        plain(0x4bdbd0, EAX);
        plain(0x424fd0, EAX | EDX, 1, 4, EDX);
        plain(0x425b40, EAX | EDX, 2, 8, EDX);
        plain(0x422290, EAX | EDX, 0, 0, EDX);
        plain(0x424f30, EAX);
        plain(0x423d90, EAX);
        plain(0x4238b0, EAX | EDX, 0, 0, EDX);
        plain(0x423960, EAX | EDX, 0, 0, EDX);
        plain(0x422610, EAX | EDX, 0, 0, EDX);
        yesNo(0x423ab0, EAX);
        noEax(0x4b7590, EAX | EDX);
        noEax(0x4b79c0, EAX);
        plain(0x4eac60, EAX | EBX | EDX);
        plain(0x40f3b0, EAX);
        plain(0x40f940, EAX | EBX | EDX, 0, 0, EBX | EDX);
        yesNo(0x40fac0, EAX);
        plain(0x40f460, EAX);
        plain(0x40f8d0, EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX);
        plain(0x4e0280, EAX | EBX | EDX, 0, 0, EDX);
        noEax(0x4972f0, EAX);
        plain(0x4e0050, EAX | EDX, 2, 8, EDX);
        plain(0x4dffb0, EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX);
        plain(0x4215c0, EAX | EDX, 0, 0, EDX);
        plain(0x485640, EAX);
        plain(0x4853c0, EAX | EDX, 0, 0, EDX);
        plain(0x4844c0, EAX);
        plain(0x4bdcc0, EAX | EDX, 0, 0, EDX);
        plain(0x426370, EAX | EBX | EDX);
        for (const x86::reg32 step : { 0x4240c0u, 0x424260u, 0x424740u, 0x4243a0u, 0x424b50u, 0x425990u, 0x425ab0u, 0x424690u, 0x424ce0u, 0x424e30u })
            plain(step, EAX);
        noEax(0x4ea9c0, 0, 2, 8);
        plain(0x420230, EAX | EDX | ESI, 0, 0, EDX | ESI);
        plain(0x40e120, EAX | EDX, 0, 0, EDX);
        plain(0x4e0430, EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX);
        noEax(0x4e01f0, EAX | EDX);
        plain(0x49fe70, EAX);
        plain(0x4a00d0, EAX);
        plain(0x4a0440, EAX);
        noEax(0x4294c0, EAX);
        plain(0x4dcdd0, 0);
        yesNo(0x420bf0, 0, 4, 16);
        plain(0x4207e0, EAX | EDX);
        // Stand-ins that do more than kSpecs' kinds: sima_hooks.inc.
        g_methods[0x41ffd0] = &nfs3hp::simaLanePosition;
        g_methods[0x40f8d0] = &nfs3hp::simaPathEndPose;
        g_methods[0x40f940] = &nfs3hp::simaAnimationStep;
        g_methods[0x4e0280] = &nfs3hp::simaMultiplyMatrices;
        g_methods[0x4972f0] = &nfs3hp::simaNormalise;
        g_methods[0x4e0050] = &nfs3hp::simaScaleVectors;
        g_methods[0x4dffb0] = &nfs3hp::simaAddVectors;
        g_methods[0x4eac60] = &nfs3hp::simaAngleParts;
        g_methods[0x424fd0] = &nfs3hp::simaMoveTowards;
        g_methods[0x425b40] = &nfs3hp::simaGlideTowards;
        g_methods[0x4b7590] = &nfs3hp::simaMatrixFromQuaternion;
        g_methods[0x420230] = &nfs3hp::simaPositionFrame;
        g_methods[0x4e0430] = &nfs3hp::simaTransformPoints;
        g_methods[0x4e01f0] = &nfs3hp::simaDot;
        g_methods[0x406e50] = &nfs3hp::simaWayToGo;
        g_methods[0x4e06d9] = &nfs3hp::simaArcTangent;
        g_methods[0x4ea9c0] = &nfs3hp::simaArcTangentOfFloats;
        g_methods[0x4a00d0] = &nfs3hp::simaHeightOverRoad;
        g_methods[0x4a0440] = &nfs3hp::simaLowestPoint;
        g_methods[0x423d90] = &nfs3hp::simaNearestSpot;
        g_methods[0x40f3b0] = &nfs3hp::simaStartAnimation;
        g_methods[0x40e120] = &nfs3hp::simaNodeGap;
#define SIMA_FRAMED(at) g_methods[at] = &nfs3hp::simaFramed<at>
        SIMA_FRAMED(0x40da30);
        SIMA_FRAMED(0x40daf0);
        SIMA_FRAMED(0x4bdc70);
        SIMA_FRAMED(0x423fe0);
        SIMA_FRAMED(0x4bdbd0);
        SIMA_FRAMED(0x422290);
        SIMA_FRAMED(0x424f30);
        SIMA_FRAMED(0x4238b0);
        SIMA_FRAMED(0x423960);
        SIMA_FRAMED(0x422610);
        SIMA_FRAMED(0x423ab0);
        SIMA_FRAMED(0x4b79c0);
        SIMA_FRAMED(0x40fac0);
        SIMA_FRAMED(0x40f460);
        SIMA_FRAMED(0x4215c0);
        SIMA_FRAMED(0x485640);
        SIMA_FRAMED(0x4844c0);
        SIMA_FRAMED(0x4bdcc0);
        SIMA_FRAMED(0x426370);
        SIMA_FRAMED(0x4240c0);
        SIMA_FRAMED(0x424260);
        SIMA_FRAMED(0x424740);
        SIMA_FRAMED(0x4243a0);
        SIMA_FRAMED(0x424b50);
        SIMA_FRAMED(0x425990);
        SIMA_FRAMED(0x425ab0);
        SIMA_FRAMED(0x424690);
        SIMA_FRAMED(0x424ce0);
        SIMA_FRAMED(0x424e30);
        SIMA_FRAMED(0x49fe70);
        SIMA_FRAMED(0x4294c0);
        SIMA_FRAMED(0x4dcdd0);
        SIMA_FRAMED(0x420bf0);
        SIMA_FRAMED(0x4207e0);
#undef SIMA_FRAMED
        return true;
    }();
    (void)standInsAdded;

    using namespace nfs3hp;
    // A float of real size mostly; the ends; now and then any bits at all.
    const auto someFloat = [&]() -> float {
        switch (random(16))
        {
        case 0:
            return 0.0f;
        case 1:
            return -0.0f;
        case 2:
        {
            static const float kRound[] = { 0.25f, 0.5f, 1.0f, -1.0f, 2.0f, 3.0f, 5.0f, 12.0f, 15.0f, 20.0f };
            return kRound[random(10)];
        }
        case 3:
        case 4:
            return float(int(random(1u << 24)) - (1 << 23)) / float(1u << (15 + random(10)));
        case 5:
            return 1e30f;
        case 6:
            return simaBits(random(0));
        case 7:
            return simaBits(kSimaNaN);
        default:
            return float(int(random(1u << 24)) - (1 << 23)) / float(1u << 15);
        }
    };
    const auto someFloats = [&](x86::reg32 at, unsigned count) {
        for (unsigned i = 0; i < count; ++i)
            setFloat(at + 4 * i, someFloat());
    };
    const auto carAt = [&](x86::reg32 i) { return kSimaCars + kSimaCarBytes * i; };
    const auto cameraAt = [&](x86::reg32 i) { return kSimaCameras + kSimaCameraBytes * i; };

    // The track: at least a slice and its next.
    const x86::reg32 slices = 2 + random(60);
    setWord32(0x5dd940, slices);
    setWord32(0x5dd948, kSimaSlices);
    for (x86::reg32 s = 0; s < slices; ++s)
        for (x86::reg32 axis = 0; axis < 3; ++axis)
            if (random(2))
                setWord32(kSimaSlices + kSimaSliceBytes * s + 4 * axis, x86::reg32(int(random(1u << 28)) - (1 << 27)));
    g_simaAbove = x86::sreg32(int(random(1u << 26)) - (1 << 25));
    if (random(4) != 0)
        for (x86::reg32 s = 0; s < slices; ++s)
            setWord32(kSimaSlices + kSimaSliceBytes * s + 0x20, x86::reg32(g_simaAbove));

    // The lane's place a case's cameras often stand at.
    for (x86::reg32 i = 0; i < 3; ++i)
        g_simaLane[i] = x86::reg32(int(random(1u << 26)) - (1 << 25));

    // The cars.
    const x86::reg32 driving = kSimaDriving;
    setFloat(driving + 0x1cc, float(random(100)) / 50.0f);
    for (x86::reg32 row = 0; row < 112; ++row)
        setFloat(driving + 0x2ec + 4 * row, random(16) == 0 ? someFloat() : float(random(3000)) / 16.0f);
    for (x86::reg32 i = 0; i < kSimaCarCount; ++i)
    {
        const x86::reg32 car = carAt(i);
        setWord32(car + 8, random(2) ? 50 + random(100) : random(0));
        setWord32(car + 0x1c, random(slices));
        setWord32(car + 0x20, random(2));
        g_memory[car + 0x76] = x86::reg8(random(2)), g_memory[car + 0x77] = 0;
        g_memory[car + 0x88] = x86::reg8(random(3));
        g_memory[car + 0x89] = x86::reg8(random(4) == 0 ? 0 : 1 + random(2));
        setFloat(car + 0x84, someFloat());
        someFloats(car + 0x98, 3);
        someFloats(car + 0x10c, 1);
        if (random(2))
            setFloat(car + 0x10c, float(int(random(2000)) - 1000) / 1000.0f);
        someFloats(car + 0x114, 3);
        someFloats(car + 0x120, 1);
        someFloats(car + 0x124, 9);
        setWord32(car + 0x184, random(4) == 0 ? 1 + random(3) : random(2) ? 0 : x86::reg32(-1));
        setWord32(car + 0x1f0, random(8));
        setWord32(car + 0x210, random(2) ? x86::reg32(int(random(2000)) - 1000) : random(2) ? 0 : random(0));
        setWord32(car + 0x2c4, random(5));
        setWord32(car + 0x524, random(4) == 0);
        setWord32(car + 0x528, driving);
        setWord32(car + 0x638, random(14));
        setWord32(car + 0x6e4, x86::reg32(int(random(3)) - 1));
        // The pedal and the speed, both sides of the limit and the gears.  The pedal stays
        // within the flyover's table of gears, or a NaN: past it the original divides by
        // whatever is in its frame, and a world with it would only stop on a zero.
        setFloat(car + 0x7bc, random(4) == 0 ? someFloat() : float(random(64)) / 32.0f);
        setFloat(car + 0x7c0, random(16) == 0 ? std::numeric_limits<float>::quiet_NaN() : float(random(1400)) / 10.0f);
        setFloat(car + 0x7c4, random(8) == 0 ? someFloat() : float(int(random(1500)) - 100) / 10.0f);
        setWord32(car + 0x99c, random(6) == 0);
        setWord32(car + 0x9a4, random(6) == 0);
    }
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        setWord32(0x5efa48 + 4 * i, carAt(random(kSimaCarCount)));
        setWord32(0x5efac8 + 4 * i, carAt(random(kSimaCarCount)));
    }
    setWord32(0x5efda4, random(4));
    setWord32(0x6fd4f4, random(2));
    setWord32(0x6fd4f8, random(2));
    setWord32(0x6fd518, random(9));
    setWord32(0x6fd4f0, random(2));
    setWord32(0x6fd4cc, random(2));
    setWord32(0x6fd50c, random(2));
    setWord32(0x6fd4a0, random(2));
    setWord32(0x6fd4c0, random(2));

    // The cameras.  Mode 19 runs the mode it was told and is never told itself; the table
    // of modes holds a camera for each, and two of its rows are read by name (19 and 21).
    for (x86::reg32 i = 0; i < kSimaCameraCount; ++i)
    {
        const x86::reg32 camera = cameraAt(i);
        setWord32(camera, random(3) == 0 ? 0x13 : random(4) == 0 ? 0x10 + random(2) : random(26));
        setWord32(camera + 4, carAt(random(kSimaCarCount)));
        setWord32(camera + 0xc, random(2));
        const float lane[3] = { float(fp::mul(x86::Float(x86::sreg32(g_simaLane[0])), 1.0 / 65536.0)),
                                float(fp::mul(x86::Float(x86::sreg32(g_simaLane[1])), 1.0 / 65536.0)),
                                float(fp::mul(x86::Float(x86::sreg32(g_simaLane[2])), 1.0 / 65536.0)) };
        // At the lane's place, or off it in one way only, or anywhere.
        const x86::reg32 offAxis = random(5);
        if (offAxis == 4)
            someFloats(camera + 0x14, 3);
        else
            for (x86::reg32 axis = 0; axis < 3; ++axis)
                setFloat(camera + 0x14 + 4 * axis, axis == offAxis ? lane[axis] + 1.0f : lane[axis]);
        setFloat(camera + 0x54, someFloat());
        setWord32(camera + 0x84, random(slices));
        const x86::reg32 own = random(4);
        // A wait's time: as an integer against 1.0's bits, and any at all.
        static const float kTimes[] = { 0.25f, 1.0f, 1.0000001f, 1.5f, -2.0f };
        setFloat(camera + 0x110, random(4) == 0 ? someFloat() : kTimes[random(5)]);
        if (own == 0)
            setWord32(camera + 0x110, 0);
        setWord32(camera + 0x114, random(2) ? 4 + random(2) : random(8));
        if (random(4) == 0)
            someFloats(camera + 0x114, 1);
        setWord32(camera + 0x118, random(3) == 0 ? x86::reg32(-1) : random(3) == 0 ? 0 : random(8));
        const x86::reg32 next = random(25);
        setWord32(camera + 0x11c, next == 19 ? 0 : next);
    }
    for (x86::reg32 row = 0; row < 2 * 27; ++row)
        setWord32(0x552ec8 + 4 * row, cameraAt(random(kSimaCameraCount)));
    for (x86::reg32 view = 0; view < 2; ++view)
    {
        setWord32(0x5e10b0 + 4 * view, random(10) == 0 ? 0 : cameraAt(random(kSimaCameraCount)));
        setWord32(0x5e10b8 + 4 * view, random(2) ? 0 : cameraAt(random(kSimaCameraCount)));
        setWord32(0x6fbbb8 + 4 * view, random(26));
        // The flyover's step, node and gear.
        const x86::reg32 gear = random(8);
        setWord32(0x553014 + 4 * view, gear);
        setWord32(0x55300c + 4 * view, random(slices));
        const x86::reg32 steps = word32(0x423d70 + 4 * gear);
        setWord32(0x553004 + 4 * view, random(2) ? steps : random(70));
    }
    setWord32(0x5e1054, random(26));
    setWord32(0x552e50, kSimaSpots);
    setWord32(0x553024, random(5));
    setWord32(0x7d3680, random(2));
    setWord32(0x7d3694, random(2));
    setWord32(0x7d36a0, random(4) == 0);
    setWord32(0x7d36a8, random(4) == 0);
    setWord32(0x7cd960, random(2));
    setWord32(0x5600ec, random(8) == 0);
    setWord32(0x559264, random(2));
    g_memory[0x55eb2e] = x86::reg8(random(2) ? 0x20 : random(256));
}
