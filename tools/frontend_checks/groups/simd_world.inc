// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // The critical section's two imports, by what the import table holds.
        specs.emplace(word32(0x5344b8), Spec{ 0, 1, 4, 0, Kind::NoEax });
        specs.emplace(word32(0x534588), Spec{ 0, 1, 4, 0, Kind::NoEax });
        g_methods[0x4965b0] = &nfs3hp::simdQuickDistance;
        g_methods[0x4972f0] = &nfs3hp::simdNormalise;
        g_methods[0x4e01c0] = &nfs3hp::simdUnitVector;
        g_methods[0x4e06a0] = &nfs3hp::simdVectorLength;
        g_methods[0x4e06d9] = &nfs3hp::simdArcTangent;
        g_methods[0x423fb0] = &nfs3hp::simdScaledIfSet;
        g_methods[0x424030] = &nfs3hp::simdDampVector;
        g_methods[0x4dffb0] = &nfs3hp::simdAddVectors;
        g_methods[0x4e0000] = &nfs3hp::simdSubtractVectors;
        g_methods[0x4e0050] = &nfs3hp::simdScaleVectors;
        g_methods[0x4b73c0] = &nfs3hp::simdQuatFromMatrix;
        g_methods[0x4b71e0] = &nfs3hp::simdSlerp;
        g_methods[0x4b7590] = &nfs3hp::simdMatrixFromQuat;
        g_methods[0x420650] = &nfs3hp::simdPlaceOnTrack;
        g_methods[0x4c8310] = &nfs3hp::simdStamp;
        g_methods[0x4c6840] = &nfs3hp::simdTakenControls;
        g_methods[0x4a60e0] = &nfs3hp::simdSend;
        g_methods[0x4e1620] = &nfs3hp::simdReserve;
        g_methods[0x420bf0] = &nfs3hp::simdInTunnel;
        g_methods[0x4df690] = &nfs3hp::simdSprintf;
#define SIMD_FRAMED(at) g_methods[at] = &nfs3hp::simdFramed<at>
        SIMD_FRAMED(0x4c9a80);
        SIMD_FRAMED(0x4a33d0);
        SIMD_FRAMED(0x4d5720);
        SIMD_FRAMED(0x4bbd70);
        SIMD_FRAMED(0x4bed20);
        SIMD_FRAMED(0x4bebe0);
        SIMD_FRAMED(0x485680);
        SIMD_FRAMED(0x4c7a60);
        SIMD_FRAMED(0x4c7e50);
        SIMD_FRAMED(0x4dcdf0);
        SIMD_FRAMED(0x418cc0);
        SIMD_FRAMED(0x439c10);
        SIMD_FRAMED(0x475240);
        SIMD_FRAMED(0x4371d0);
        SIMD_FRAMED(0x42c830);
        SIMD_FRAMED(0x42c530);
        SIMD_FRAMED(0x4711e0);
        SIMD_FRAMED(0x471000);
        SIMD_FRAMED(0x4cdd00);
        SIMD_FRAMED(0x44b950);
        SIMD_FRAMED(0x4129c0);
        SIMD_FRAMED(0x493ef0);
        SIMD_FRAMED(0x493e50);
        SIMD_FRAMED(0x4b7c20);
        SIMD_FRAMED(0x4bc8b0);
#undef SIMD_FRAMED
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
        {
            // Past a float's range once multiplied, which the x87's exponent holds and a float's would not.
            static const float kFar[] = { 1e30f, -1e30f, 1e-30f, -1e-30f };
            return kFar[random(4)];
        }
        case 6:
            return simdBits(random(0));
        case 7:
            return simdBits(kSimdNaN);
        default:
            return float(int(random(1u << 24)) - (1 << 23)) / float(1u << 15);
        }
    };
    const auto someFloats = [&](x86::reg32 at, unsigned count) {
        for (unsigned i = 0; i < count; ++i)
            setFloat(at + 4 * i, someFloat());
    };
    const auto carAt = [&](x86::reg32 i) { return kSimdCars + kSimdCarBytes * i; };
    const auto cameraAt = [&](x86::reg32 i) { return kSimdCameras + kSimdCameraBytes * i; };

    // The cars.
    for (x86::reg32 i = 0; i < kSimdCarCount; ++i)
    {
        const x86::reg32 car = carAt(i);
        setWord32(car + 0x1c, random(0));
        someFloats(car + 0x98, 3);
        someFloats(car + 0xc0, 9);
        // Grip: either side of the bits compared, to the bit.
        static const x86::reg32 kGrips[] = { 0x3f34fdf3, 0x3f34fdf4, 0x3f34fdf5, 0x3f800000, 0x3f000000, 0x00000000 };
        setWord32(car + 0x10c, kGrips[random(6)]);
        someFloats(car + 0x118, 2);
        g_memory[car + 0x200] = x86::reg8(random(2) ? 0x20 : random(256));
        setWord32(car + 0x2c4, random(2) ? 3 : random(5));
        setWord32(car + 0x524, x86::reg32(int(random(4096)) - 2048));
        setWord32(car + 0x528, kSimdDriving);
        someFloats(car + 0x4ec, 1);
        someFloats(car + 0x4f4, 1);
        setWord32(car + 0x998, x86::reg32(int(random(3)) - 1));
        setWord32(car + 0x9a4, carAt(random(kSimdCarCount)));
        // Stamps for the network's packet.
        setWord32(car + 8, random(0));
    }
    // The speed's lift: each side of its limit (0x536ec4).
    setWord32(kSimdDriving + 0x2c0, random(2) ? random(64) : x86::reg32(int(random(1u << 20)) - (1 << 19)));

    // The cameras.
    for (x86::reg32 i = 0; i < kSimdCameraCount; ++i)
    {
        const x86::reg32 camera = cameraAt(i);
        setWord32(camera + 4, random(16) == 0 ? 0 : carAt(random(kSimdCarCount)));
        setWord32(camera + 8, carAt(random(kSimdCarCount)));
        setWord32(camera + 0xc, random(2));
        someFloats(camera + 0x14, 3);
        someFloats(camera + 0x20, 3);
        someFloats(camera + 0x2c, 9);
        someFloats(camera + 0x54, 1);
        setWord32(camera + 0x84, random(0));
        setWord32(camera + 0x10c, random(2) ? 0 : random(2) ? 1 : random(6));
        someFloats(camera + 0x110, 3);
        someFloats(camera + 0x11c, 1);
        // Own fields: floats for some modes, counts for others; the modes' entries set them.
        someFloats(camera + 0x120, 1);
        setWord32(camera + 0x124, random(3) == 0 ? 0 : random(8));
        someFloats(camera + 0x128, 1);
    }
    for (x86::reg32 view = 0; view < 2; ++view)
    {
        setWord32(0x552f08 + 4 * view, cameraAt(random(kSimdCameraCount)));
        setWord32(0x79c4a0 + 4 * view, random(3) == 0);
        setWord32(0x5e1010 + 8 * view, random(3) == 0);
        setWord32(0x5e1014 + 8 * view, random(2) ? 0x1f + random(3) : random(0));
    }

    // The race.
    setWord32(0x7d36a8, random(4) == 0);
    setWord32(0x7cd960, random(2));
    setWord32(0x5600ec, random(8) == 0);
    setWord32(0x7d3684, x86::reg32(int(random(1u << 16)) - (1 << 15)));
    setWord32(0x7d3694, random(2));
    setWord32(0x79c1a4, random(1000));
    setWord32(0x55301c, random(2));
    setWord32(0x553020, random(3) == 0 ? 0 : random(3) == 0 ? x86::reg32(-1) : random(5));
    setWord32(0x553028, random(2) ? 4 : random(8));
    setWord32(0x7cda58, 1 + random(2));
    setWord32(0x7cd94c, random(3));
    setWord32(0x55e88c, random(3) == 0);
    setWord32(0x55e894, random(4));
    setWord32(0x6fd3a0, random(4) == 0 ? 2 : random(2));
    setWord32(0x6fd3b0, random(4) == 0 ? random(2) : 2 + random(3));
    setWord32(0x6fd50c, random(8) != 0);
    g_memory[0x7a22da] = x86::reg8(random(2));
    g_memory[0x7a22db] = 0;
    setWord32(0x6fd4cc, random(2));
    setWord32(0x6fd4f4, random(2));
    setWord32(0x6fd520, random(9));
    setWord32(0x6fbb4c, random(2));
    g_memory[0x7a3a58] = x86::reg8(random(2) ? 0x10 : random(256));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        setWord32(0x5efac8 + 4 * i, carAt(random(kSimdCarCount)));
        setWord32(0x5efb88 + 4 * i, carAt(random(kSimdCarCount)));
    }

    // The directory sub_4c45f0 prints into 0x80 bytes of its frame: a short one, as the game's.
    std::memcpy(g_memory + 0x7a2934, "gamedata\\", 10);

    // The track's slices: positions in 16.16, sides as bytes, widths.
    setWord32(0x5dd948, kSimdSlices);
    for (x86::reg32 s = 0; s < kSimdSliceCount; ++s)
        for (x86::reg32 axis = 0; axis < 3; ++axis)
            setWord32(kSimdSlices + kSimdSliceBytes * s + 4 * axis, x86::reg32(int(random(1u << 28)) - (1 << 27)));

    // The task lists: a few entries, in order or not.
    const x86::reg32 tasks = random(10);
    setWord32(kSimdList, 0x20);
    setWord32(kSimdList + 4, random(16) == 0 ? 0 : tasks);
    for (x86::reg32 i = 0; i < 0x20; ++i)
        setWord32(kSimdList + 8 + 16 * i, random(3) == 0 ? random(200) : 10 * i);
    setWord32(0x7d368c, kSimdList);
    setWord32(0x7d3690, kSimdList);
}
