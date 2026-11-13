// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const auto someCamera = [&] { return kSimdCameras + kSimdCameraBytes * random(kSimdCameraCount); };
    const auto richFloat = [&] { return float(int(random(1u << 24)) - (1 << 23)) / float(1u << (15 + random(10))); };
    const auto size = [&]() -> float {
        switch (random(10))
        {
        case 0:
            return 0.0f;
        case 1:
            return -0.0f;
        case 2:
            return simdBits(kSimdNaN);
        case 3:
            return simdBits(random(0));
        case 4:
        {
            // Past a float's range once multiplied, which the x87's exponent holds and a float's would not.
            static const float kFar[] = { 1e30f, -1e30f, 1e-30f, -1e-30f };
            return kFar[random(4)];
        }
        default:
            return richFloat();
        }
    };
    const auto floats = [&](x86::reg32 at, unsigned count) {
        for (unsigned i = 0; i < count; ++i)
            setFloat(at + 4 * i, size());
    };
    // Ints of 16.16: small, large, past what the squares hold, the lowest.
    const auto someInt = [&]() -> x86::reg32 {
        switch (random(8))
        {
        case 0:
            return 0;
        case 1:
            return 0x80000000u;
        case 2:
            return random(0);
        case 3:
            return x86::reg32(int(random(1u << 26)) - (1 << 25));
        case 4:
            return x86::reg32(int(random(5)) - 2);
        default:
            return x86::reg32(int(random(1u << 18)) - (1 << 17));
        }
    };
    const auto ints = [&](x86::reg32 at, unsigned count) {
        for (unsigned i = 0; i < count; ++i)
            setWord32(at + 4 * i, someInt());
    };
    const x86::reg32 a = kSimdVectors, b = kSimdVectors + 0x40, c2 = kSimdVectors + 0x80, d = kSimdVectors + 0xc0;
    switch (c.address)
    {
    case 0x4243a0:
    case 0x424740:
    case 0x424b50:
    case 0x424ce0:
    case 0x424e30:
    case 0x424f30:
    case 0x425990:
    case 0x425ab0:
    case 0x424690:
    {
        const x86::reg32 camera = someCamera();
        entry.eax = camera;
        if (word32(camera + 4) == 0)
            setWord32(camera + 4, kSimdCars);
        // sub_424b50's and sub_424f30's counts and rate; sub_424ce0's tick; the others' floats stay.
        if (c.address == 0x424b50)
        {
            setWord32(camera + 0x124, random(6));
            setWord32(camera + 0x120, random(6));
            floats(camera + 0x11c, 1);
        }
        if (c.address == 0x424f30)
        {
            setWord32(camera + 0x124, random(2));
            setFloat(camera + 0x128, size());
        }
        if (c.address == 0x424ce0)
            setWord32(camera + 0x124, word32(0x79c1a4) - random(4));
        // Products past a float's range that cancel in the sum: the x87's exponent holds them, a float's would not.
        if (c.address == 0x425ab0 && random(8) == 0)
        {
            const x86::reg32 car = word32(camera + 4);
            setFloat(camera + 0x110, 1e30f);
            setFloat(camera + 0x114, -1e30f);
            setFloat(car + 0xc0, 1e30f);
            setFloat(car + 0xcc, 1e30f);
        }
        // Looking back, these change the sign of the offset's third part by its bits.  No NaN in it then: one
        // of the other sign beside the matrix's in a sum, and which comes out says nothing (simdBits).
        if (word32(0x79c4a0 + 4 * word32(camera + 0xc)) != 0
            && (c.address == 0x4243a0 || c.address == 0x424740 || c.address == 0x425990))
        {
            const x86::reg32 car = word32(camera + 4);
            setFloat(camera + 0x118, richFloat());
            setFloat(car + 0x11c, richFloat());
            setFloat(car + 0x4ec, richFloat());
            setFloat(car + 0x4f4, richFloat());
        }
        break;
    }
    case 0x424fd0:
        entry.eax = someCamera();
        entry.edx = a;
        floats(a, 3);
        setFloat(entry.esp + 4, size());
        break;
    case 0x425b40:
        // A distance below 0 had its sign changed, a NaN's too: no other NaN to sum it with (simdBits).
        entry.eax = someCamera();
        entry.edx = a;
        floats(a, 3);
        setFloat(entry.esp + 4, richFloat());
        setFloat(entry.esp + 8, richFloat());
        break;
    case 0x426370:
        entry.eax = someCamera();
        entry.edx = random(2);
        entry.ebx = random(2) ? 8 : random(20);
        break;
    case 0x4dbda0:
        entry.eax = someCamera();
        setWord32(entry.eax + 4, random(4));
        break;
    case 0x439cc0:
        entry.eax = random(40);
        break;
    case 0x4969c0:
        entry.eax = a;
        ints(a, 9);
        ints(entry.esp + 4, 6);
        break;
    case 0x496a50:
        entry.eax = a;
        entry.edx = b;
        floats(a, 9);
        if (random(4) == 0)
            for (x86::reg32 i = 0; i < 3; ++i)
                setWord32(a + 0x18 + 4 * i, word32(a + 0x0c + 4 * i));
        break;
    case 0x4970c0:
        entry.eax = a;
        ints(a, 3);
        // Now and then squares that come to the limit to the bit, or one past it, or one short.
        if (random(4) == 0)
        {
            const auto square = [](x86::reg32 v) {
                const std::uint64_t p = std::uint64_t(std::int64_t(x86::sreg32(v)) * std::int64_t(x86::sreg32(v)));
                return (x86::reg32(p >> 32) << 16) + (x86::reg32(p) >> 16) + ((x86::reg32(p) >> 15) & 1);
            };
            const x86::reg32 part = 0xb504f0;
            const x86::reg32 wanted = 0x40000000u - 2 * (square(part) >> 2) + (random(3) - 1);
            x86::reg32 z = 0;
            while (z < 0x100000 && (square(z) >> 2) < wanted)
                ++z;
            setWord32(a, part);
            setWord32(a + 4, part);
            setWord32(a + 8, z);
        }
        break;
    case 0x4be1f0:
        entry.edx = random(0x40);
        break;
    case 0x4be250:
        entry.eax = kSimdList;
        entry.ecx = random(3) == 0 ? 10 * random(0x20) : random(200);
        break;
    case 0x4c7bd0:
        entry.eax = random(2) ? random(4096) : random(0);
        break;
    case 0x4c9bc0:
        entry.eax = random(kSimdSliceCount);
        entry.edx = random(14);
        entry.ebx = random(2) ? 0 : random(0);
        entry.esi = b;
        break;
    case 0x4e0090:
        entry.eax = a;
        entry.edx = random(4) == 0 ? a : b;
        floats(a, 3);
        floats(b, 3);
        break;
    case 0x4e0110:
        entry.eax = a;
        entry.edx = random(3) == 0 ? a : b;
        floats(a, 9);
        break;
    case 0x4e0160:
    case 0x4e01a0:
        entry.eax = a;
        entry.edx = b;
        entry.ebx = random(3) == 0 ? a : random(3) == 0 ? b : c2;
        floats(a, 3);
        floats(b, 3);
        break;
    case 0x4ea930:
    case 0x4ea950:
        setFloat(entry.esp + 4, size());
        break;
    case 0x4ea970:
        entry.eax = a;
        entry.edx = random(4) == 0 ? a : b;
        entry.ebx = random(4) == 0 ? a : c2;
        setFloat(a, size());
        break;
    case 0x4ea9c0:
        setFloat(entry.esp + 4, size());
        setFloat(entry.esp + 8, size());
        break;
    case 0x4ea9e0:
    case 0x4eaab0:
    case 0x4eab80:
        entry.eax = a;
        floats(a, 9);
        setFloat(entry.esp + 4, size());
        setWord32(entry.esp + 8, random(3) == 0 ? a : b);
        break;
    case 0x4f44e0:
        entry.eax = a;
        entry.edx = random(3) == 0 ? a : b;
        ints(a, 9);
        break;
    case 0x4f4544:
        entry.eax = a;
        entry.edx = b;
        entry.ebx = random(4) == 0 ? a : random(4) == 0 ? b : d;
        ints(a, 3);
        ints(b, 9);
        break;
    default:
        break;
    }
}
