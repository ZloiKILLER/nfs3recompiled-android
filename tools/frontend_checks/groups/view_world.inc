// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.  What the
    // generated code goes on with through a call -- esi and edi in the tiles' loops, ecx the car's
    // folder in sub_464eb0 -- the real ones keep, as sprintf keeps the load's flags in edx: not
    // clobbered.  The rest they lose, to show a register a native should have restored.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x4e0ec0] = { EAX | EDX, 0, 0, EBX | ECX | EDX | EDI, Kind::Plain, true };  // the file's path, what it says
        specs[0x4d4080] = { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d5ed0] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain };
        specs[0x4ef1e0] = { EAX | EDX, 0, 0, EBX | ECX | EDX, Kind::Shape };              // the real one keeps edi
        specs[0x4392b0] = { EAX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain };
        specs[0x439290] = { EAX, 0, 0, EBX | EDX | ESI | EDI, Kind::Plain };
        specs[0x4395c0] = { EAX, 0, 0, EBX | EDX | ESI | EDI, Kind::Bool };
        specs[0x4ee310] = { EAX | EDX, 0, 0, EBX | ESI | EDI, Kind::Bool, true };         // 0 now and then: credits1
        return true;
    }();
    (void)standInsAdded;

    // Strings of odd and even lengths: sub_464eb0 copies its folder two bytes at a time.
    const auto makeString = [](x86::reg32 at) {
        const x86::reg32 length = random(24);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[at + i] = x86::reg8('a' + random(26));
        g_memory[at + length] = 0;
    };
    makeString(0x5f3f40);
    makeString(0x5f3f60);
    makeString(0x7a3234);
    makeString(0x7a28b4);

    // The screen shown, for sub_44ae00, which runs as itself: none now and then.
    setWord32(0x559230, random(4) == 0 ? 0 : g_menu);
    setWord32(g_menu + 0x14, 0x5f3f20);

    // sub_464eb0's: a fade or not, the car already on the tiles or not.
    setWord32(0x559c60, random(4) == 0 ? 1 + random(500) : 0);
    setWord32(0x664794, random(700));
    setWord32(0x678940, random(4) == 0 ? word32(0x664794) : random(700));

    // sub_4652f0's.
    setWord32(0x67894c, random(2) ? 0 : random(0));
    setWord32(0x678950, random(2) ? 0 : random(0));
}
