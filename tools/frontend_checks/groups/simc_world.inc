// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // What these functions call and the check's table lacks: the registers each reads, its stack
    // arguments, what its ret pops, what it changes besides eax (read off nfs3.exe, all that its
    // callees change too); and those that need more, through g_methods, simc_hooks.inc.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto plain = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0, std::uint8_t pops = 0, std::uint8_t clobbers = 0) {
            specs[at] = { in, stack, pops, clobbers, Kind::Plain };
        };
        const auto keepsEax = [&](x86::reg32 at, std::uint8_t in, std::uint8_t stack = 0) {
            specs[at] = { in, stack, 0, 0, Kind::NoEax };
        };
        keepsEax(0x4f9482, 0);
        keepsEax(0x4e0110, EAX | EDX);
        plain(0x4e0280, EAX | EDX | EBX, 0, 0, EDX);
        keepsEax(0x4ea970, EAX | EDX | EBX);
        keepsEax(0x4972f0, EAX);
        keepsEax(0x4e0160, EAX | EDX | EBX);
        plain(0x4c9bc0, EAX | EDX | EBX | ESI, 0, 0, EBX | EDX | ESI);
        plain(0x435310, EAX | EDX | EBX | ECX, 0, 0, EBX | ECX | EDX);
        plain(nfs3hp::kSimcAlias + 0x4dab10, EAX | EDX);
        plain(0x503b00, EAX | EDX | ESI, 0, 0, EDX | ESI);
        plain(0x503760, 0, 3, 12);
        plain(0x4bbd70, 0);
        plain(0x421450, EAX | EDX);
        plain(0x4349a0, EAX);
        plain(0x41bb50, EAX);
        plain(0x4bc160, EAX);
        plain(0x4349d0, EAX, 2, 8);
        plain(0x422610, EAX | EDX, 0, 0, EDX);
        plain(0x421490, EAX);
        plain(0x48f8d0, EAX | EDX);
        plain(0x4bd3f0, EAX);
        plain(0x451960, EAX);
        plain(0x4c4d80, 0, 0, 0, EBX | ESI);
        plain(0x426680, EAX);
        plain(0x423d90, EAX);
        plain(0x434a90, 0);
        plain(0x4dbcc0, 0);
        plain(0x4bed20, EAX | EDX | EBX, 0, 0, EBX | EDX);
        plain(0x439a50, EAX);
        plain(0x4392b0, EAX);
        plain(nfs3hp::kSimcAlias + 0x439290, EAX);
        specs[0x4395c0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x44a660] = { EAX | EDX, 0, 0, 0, Kind::Bool };
        plain(nfs3hp::kSimcAlias + 0x464700, EAX | EDX | ECX, 0, 0, EDX);
        keepsEax(0x42d510, 0, 6);
        specs[0x4ee310] = { EAX | EDX, 0, 0, EDX, Kind::Bool, true };  // 0 now and then: credits1

        g_methods[0x4f9482] = &nfs3hp::simcArcCosine;
        g_methods[0x4e0110] = &nfs3hp::simcTranspose;
        g_methods[0x4e0280] = &nfs3hp::simcMultiply;
        g_methods[0x4ea970] = &nfs3hp::simcSineCosine;
        g_methods[0x4972f0] = &nfs3hp::simcNormalise;
        g_methods[0x4e0160] = &nfs3hp::simcCross;
        g_methods[0x4c9bc0] = &nfs3hp::simcNodeEnd;
        g_methods[0x435310] = &nfs3hp::simcReadMouse;
        g_methods[0x4dab10] = &nfs3hp::simcHotSpot;
        g_methods[0x503b00] = &nfs3hp::simcShiftLeft;
        g_methods[0x48f8d0] = &nfs3hp::simcKeyPressed;
        g_methods[0x423d90] = &nfs3hp::simcNearestSpot;
        g_methods[0x439a50] = &nfs3hp::simcCarRecord;
        g_methods[0x464700] = &nfs3hp::simcMakeTiles;
        // The table's others, for their esp to be recorded.
        g_methods[0x503760] = &nfs3hp::simcStandIn<0x503760>;
        g_methods[0x4bbd70] = &nfs3hp::simcStandIn<0x4bbd70>;
        g_methods[0x421450] = &nfs3hp::simcStandIn<0x421450>;
        g_methods[0x4349a0] = &nfs3hp::simcStandIn<0x4349a0>;
        g_methods[0x41bb50] = &nfs3hp::simcStandIn<0x41bb50>;
        g_methods[0x4bc160] = &nfs3hp::simcStandIn<0x4bc160>;
        g_methods[0x4349d0] = &nfs3hp::simcStandIn<0x4349d0>;
        g_methods[0x422610] = &nfs3hp::simcStandIn<0x422610>;
        g_methods[0x421490] = &nfs3hp::simcStandIn<0x421490>;
        g_methods[0x4bd3f0] = &nfs3hp::simcStandIn<0x4bd3f0>;
        g_methods[0x451960] = &nfs3hp::simcStandIn<0x451960>;
        g_methods[0x4c4d80] = &nfs3hp::simcStandIn<0x4c4d80>;
        g_methods[0x426680] = &nfs3hp::simcStandIn<0x426680>;
        g_methods[0x434a90] = &nfs3hp::simcStandIn<0x434a90>;
        g_methods[0x4dbcc0] = &nfs3hp::simcStandIn<0x4dbcc0>;
        g_methods[0x4bed20] = &nfs3hp::simcStandIn<0x4bed20>;
        g_methods[0x4392b0] = &nfs3hp::simcStandIn<0x4392b0>;
        g_methods[0x439290] = &nfs3hp::simcStandIn<nfs3hp::kSimcAlias + 0x439290>;
        g_methods[0x4395c0] = &nfs3hp::simcStandIn<0x4395c0>;
        g_methods[0x42d510] = &nfs3hp::simcStandIn<0x42d510>;
        g_methods[0x4ee310] = &nfs3hp::simcStandIn<0x4ee310>;
        g_methods[0x4df690] = &nfs3hp::simcStandIn<0x4df690>;
        return true;
    }();
    (void)standInsAdded;

    // The game's own tables and constants, which the watched range's junk fills over.
    const auto fromImage = [](x86::reg32 at, x86::reg32 bytes) {
        std::memcpy(g_memory + at, g_image.data() + (at - 0x534000), bytes);
    };
    fromImage(0x55fe44, 9 * 4);                     // the panel's cameras
    fromImage(0x55fe68, 21 * 4);                    // the director's
    fromImage(0x55fdcc, 10 * 4);                    // the series' coefficients
    fromImage(0x564498, 9 * 4);                     // the identity

    // The scratch: all floats, some worlds with junk among them.
    const bool junk = random(4) == 0;
    nfs3hp::simcFloats(g_random, 0x900000, 0x400, junk);
    nfs3hp::simcFloats(g_random, 0x5600a0, 8, junk);

    // Cars: their camera, their record (with what sub_4bdd70 asks of it), their zoom.
    constexpr x86::reg32 kCars = 0x902800;
    for (x86::reg32 c = 0; c < 4; ++c)
    {
        const x86::reg32 car = kCars + 0x80 * c;
        setWord32(car, random(2) ? 8 + random(6) : random(25));
        setWord32(car + 4, 0x901000 + 0x10 * random(16));
        setFloat(car + 0x54, random(3) == 0 ? (random(2) ? 0.0f : -0.0f) : nfs3hp::simcSomeFloat(g_random, junk));
    }
    for (x86::reg32 c = 0; c < 16; ++c)
        setWord32(0x901000 + 0x10 * c + 0x998, random(2) ? 0 : random(0));
    for (x86::reg32 p = 0; p < 2; ++p)
    {
        setWord32(0x5e10b8 + 4 * p, random(3) == 0 ? kCars + 0x80 * random(2) : 0);
        setWord32(0x5e10b0 + 4 * p, random(5) == 0 ? 0 : kCars + 0x80 * (2 + random(2)));
    }

    // The panel and the director: state as the replay has it, on both sides of its tests.
    setWord32(0x6fd3b0, random(3));
    setWord32(0x7cd954, random(2) ? random(9) : random(2) ? 0 : 8);  // the list's ends, where it wraps
    setWord32(0x7cd964, random(4) == 0 ? 0 : random(9));
    setWord32(0x7cd960, random(2));
    setWord32(0x7cd95c, random(4) == 0 ? 1 + random(8) : 0);
    setWord32(0x7cd958, random(8) == 0 ? 4 + random(4) : random(4));
    setWord32(0x7cd94c, random(3) == 0 ? 0 : random(5));
    setWord32(0x7d36a8, random(3) == 0);
    setWord32(0x7d3684, random(3) == 0 ? random(100000) : random(3000));
    setWord32(0x79c1a4, random(1000));
    setWord32(0x55febc, random(1000));
    setWord32(0x6fd4f8, random(16));
    for (x86::reg32 at = 0x7497d8; at <= 0x7497e4; at += 4)
        setWord32(at, 0);
    setWord32(0x7497d8, random(200));
    setWord32(0x7497dc, random(200));
    setWord32(0x7497e0, 200 + random(400));
    setWord32(0x7497e4, 200 + random(250));
    for (x86::reg32 p = 0; p < 2; ++p)
    {
        const x86::reg32 record = 0x7cd968 + 0x74 * p;
        setWord32(record, random(2));
        setWord32(record + 4, random(4000));
        setWord32(record + 8, random(21));
        setWord32(record + 0xc, random(3) == 0 ? random(2000) : random(2) ? 0xa0 : 0x140);
        for (x86::reg32 k = 0; k < 25; ++k)
            setWord32(record + 0x10 + 4 * k, random(3) == 0 ? 0 : random(0));
        // One of the list's at least, or sub_4bdbd0 would never end.
        const x86::reg32 kept = word32(0x55fe68 + 4 * random(21));
        setWord32(record + 0x10 + 4 * kept, 1);
    }
    // The siren's node: a table of the track's.
    setWord32(0x5dd948, 0x901400);

    // The view widget's strings and car (see view_world.inc).
    const auto makeString = [](x86::reg32 at) {
        const x86::reg32 length = random(24);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[at + i] = x86::reg8('a' + random(26));
        g_memory[at + length] = 0;
    };
    makeString(0x7a3234);
    makeString(0x7a28b4);
    setWord32(0x559230, random(4) == 0 ? 0 : g_menu);
    setWord32(g_menu + 0x14, 0x5f3f20);
    std::memcpy(g_memory + 0x5f3f20, random(2) ? "credits1" : "trk00\0\0", 9);
    setWord32(0x664798, random(700));
}
