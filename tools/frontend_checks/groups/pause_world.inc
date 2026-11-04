// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these screens need and the check's table lacks, added to it once.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x422bd0] = { EAX, 0, 0, 0, Kind::Shape };                  // the camera's car: memory to read
        specs[0x4710f0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x48f1f0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4844c0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4be5e0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x472300] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x423a60] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x423ae0] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4238b0] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4e0e60] = { EAX, 0, 0, 0, Kind::Bool, true };             // a file stands
        specs[0x4e1620] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Tuning, true };  // 0 or memory
        specs[0x4eeae0] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain, true };
        return true;
    }();
    (void)standInsAdded;

    // What writes and looks for a save: those with generated code here (the
    // ghost's and the replay's writers are native_load_saves.cpp's).
    static const x86::reg32 kPauseWrites[] = { 0x446550, 0x4468c0 };
    static const x86::reg32 kPauseLooks[] = { 0x446510, 0x446740, 0x446950, 0x446a90 };
    setWord32(0x5591cc, kPauseWrites[random(2)]);
    setWord32(0x5591d0, kPauseLooks[random(4)]);
    setWord32(0x5591e8, random(5));
    setWord32(0x6fbb40, random(5) == 0 ? random(0) : 0x20f0);
    // A save's copy, for sub_478010 tested by itself.
    setWord32(0x5f5100, random(5) == 0 ? random(0) : 0x20f0);
    setWord32(0x5f5100 + 0x1994, random(8));

    setWord32(0x6fd3a0, random(3) == 0 ? random(4) : 0);
    setWord32(0x7d36b4, random(2));
    setWord32(0x7a3a68, random(2));
    setWord32(0x6fbc34, random(2));
    setWord32(0x66440c, random(2));
    setWord32(0x7cdac8, random(2) ? 0x280 : random(0x500));
    if (random(2))
        setWord32(0x662da0, word32(0x6fbc4c));

    // The display's choices and their places, at their ends now and then.
    for (x86::reg32 screen = 0; screen < 3; ++screen)
    {
        for (x86::reg32 kind = 0; kind < 2; ++kind)
        {
            const x86::reg32 choices = 0x6fbc64 + 0x348 * screen + 0x1a4 * kind;
            const x86::reg32 place = 8 * screen + 4 * kind;
            for (x86::reg32 i = 0; i < 4; ++i)
                setWord32(choices + 4 * i, random(3) == 0 ? 0 : random(8));
            setWord32(0x678840 + place, random(3) == 0 ? 4 : random(8));
            setWord32(0x678810 + place, random(3) == 0 ? 3 : random(8));
            setWord32(0x6787f8 + place, random(3) == 0 ? 2 : random(8));
            setWord32(0x678828 + place, random(3) == 0 ? 5 : random(8));
            setFloat(choices - 4, someValue(g_random));
        }
    }
    setWord32(0x5e1054, random(8));

    // The cameras, often the same or none.
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x6fbba8 + 4 * i, random(4));
    if (random(4) == 0)
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord32(0x6fbba8 + 4 * i, 0);
}
