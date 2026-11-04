// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // The widget's type counts too.
        specs[0x44a660] = { EAX | EDX, 0, 0, 0, Kind::Bool };
        specs[0x449df0] = { 0, 0, 0, 0, Kind::Bool };
        // A box handed by address: what it says, up to its first zero byte.
        specs[0x49c1f0] = { EAX, 0, 0, 0, Kind::Bool, true };
        specs[0x4181d0] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        // The results.
        specs[0x455c10] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[0x455cd0] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Tuning };
        specs[0x4554e0] = { EAX | EDX, 0, 0, 0, Kind::Tuning };
        specs[0x4cef80] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x455530] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };
        for (const x86::reg32 fills : { 0x455570u, 0x4555c0u, 0x455600u, 0x455660u, 0x4556c0u, 0x455720u, 0x4cee40u })
            specs[fills] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x46cf30] = { EAX, 0, 0, 0, Kind::Plain };
        // Text and drawing, what they are handed read back.
        specs[0x4d1850] = { EAX, 0, 0, 0, Kind::Shape };
        specs[0x452190] = { EAX | EBX | ECX | EDX, 2, 8, 0, Kind::Plain, true };
        specs[0x452120] = { EAX | EBX | ECX | EDX, 3, 12, 0, Kind::Plain, true };
        specs[0x4d8860] = { EAX | EBX | ECX | EDX, 4, 16, 0, Kind::Plain };
        specs[0x4d8930] = { EAX | EBX | ECX | EDX, 1, 4, 0, Kind::Plain };
        specs[0x4d8ac0] = { EAX | EBX | ECX | EDX, 4, 16, 0, Kind::Plain };
        specs[0x4d8ed0] = { EAX | EBX | ECX | EDX, 2, 8, 0, Kind::Plain };
        specs[0x4d7990] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };
        // The menu art.
        specs[0x4d5f40] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x4ef1e0] = { EAX | EDX, 0, 0, EDX, Kind::Tuning };
        specs[0x4d6070] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x4d6390] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };
        // The front end's ends.
        specs[0x4daf80] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4956f0] = { EAX | EDX, 0, 0, 0, Kind::Tuning, true };
        specs[0x4f0650] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x451940] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x451960] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x4c9860] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x431bc0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x431900] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x4d5cc0] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x4f0230] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x49c140] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49c2a0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x410b20] = { EDX, 0, 0, EDI, Kind::Plain };
        specs[0x472360] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x472820] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4d5c40] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4d5d10] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4c9890] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x44b230] = { EAX | EDX, 0, 0, 0, Kind::Plain, true };
        specs[0x4c9a00] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x44e7d0] = { 0, 0, 0, 0, Kind::Plain };
        // Another group's natives among them stand in too: this process would run
        // their generated code, whose own callees are not these functions' concern.
        for (const auto& spec : specs)
            g_methods.erase(spec.first);
        return true;
    }();
    (void)standInsAdded;

    // A line of text: a short string, ending within its 16 bytes.
    for (x86::reg32 k = 0; k < 16; ++k)
        g_memory[0x5f3c00 + 16 * k + 1 + random(15)] = 0;

    // The tables' counts: a racer's row a call or two each.
    setWord32(0x677d30, random(12));
    setWord32(0x677d34, random(12));
    setWord32(0x6fd4b4, random(12));
    setWord32(0x6fd4b8, random(4));
    setWord32(0x6fd4bc, random(9));
    setWord32(0x6fd4f4, random(3));
    setWord32(0x6fbc68, random(6));
    setWord32(0x7a3a50, random(7));
    setWord32(0x6fd470, random(2));
    if (random(4) == 0)
        setWord32(0x6fd3b8, 3);
    // The records the results hand out, 0x5f3e00 and on, four bytes apart: whose
    // they are, their times or none, their status and the rounds won; and their
    // shapes' sizes, for the art.
    for (x86::reg32 at = 0x5f3ff0; at < 0x5f4020; at += 4)
        setWord32(at, random(3));
    for (x86::reg32 at = 0x5f4060; at < 0x5f40c0; at += 4)
        setWord32(at, random(3) == 0 ? 0 : random(0));
    for (x86::reg32 at = 0x5f40c0; at < 0x5f4110; at += 4)
        setWord32(at, random(5));
    static const x86::reg32 kArtSizes[] = { 0x01000100, 0x00200020, 0x01000020, 0x00200100 };
    for (x86::reg32 at = 0x5f3e00; at < 0x5f3e40; at += 4)
        setWord32(at, random(4) == 0 ? random(0) : kArtSizes[random(4)]);
}
