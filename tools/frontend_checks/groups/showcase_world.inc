// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // The callees kSpecs does not have, taken from the game's code: which
    // registers each reads and changes.  (kSpecs belongs to frontend_checks.cpp;
    // no group file reaches it otherwise.)
    static const bool specsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs.emplace(0x410460, Spec{ 0, 0, 0, 0, Kind::Plain });                       // the menus' music
        specs.emplace(0x4392b0, Spec{ EAX, 0, 0, 0, Kind::Tuning });                    // a car's folder, or 0
        specs.emplace(0x4395c0, Spec{ EAX, 0, 0, 0, Kind::Bool });                      // a car in its own folder
        specs.emplace(0x439ab0, Spec{ EAX | EDX, 0, 0, EDX, Kind::Tuning });            // a line of a car's text
        specs.emplace(0x4646e0, Spec{ 0, 1, 4, 0, Kind::NoEax });                       // a float on the stack
        specs.emplace(0x4956f0, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain, true });       // a .qfs loaded
        specs.emplace(0x4e0e60, Spec{ EAX, 0, 0, 0, Kind::Bool, true });                // a file there
        specs.emplace(0x4ee310, Spec{ EAX | EDX, 0, 0, EDX, Kind::Bool, true });        // strcmp: 0 now and then
        specs.emplace(0x4eb390, Spec{ EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Bool, true });  // strncmp
        specs.emplace(0x4f0ae0, Spec{ EAX, 0, 0, 0, Kind::Plain });                     // tolower
        specs.emplace(0x4ef190, Spec{ EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Shape });  // a bitmap: memory to read
        specs.emplace(0x4eada0, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4ef1b0, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4ef4c0, Spec{ EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
        specs.emplace(0x4eff00, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4f09c0, Spec{ EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
        specs.emplace(0x4f0a10, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4f0a90, Spec{ EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain });
        // sub_4ef1e0 saves edi round the one path that uses it, and
        // sub_453320 keeps an item in edi across it.
        specs.at(0x4ef1e0).clobbers = EDX;
        return true;
    }();
    (void)specsAdded;

    // The cars' strings, short enough for the buffers on the stack: what
    // sub_4392b0 and sub_439ab0 hand out (Kind::Tuning), and the stem of a
    // record at 1, which sub_439a50's Bool hands out.
    for (x86::reg32 i = 0; i < 0x20; ++i)
        g_memory[0x5f3e00 + i] = random(6) == 0 ? 0 : x86::reg8('a' + random(26));
    g_memory[0x5f3e20] = 0;
    for (x86::reg32 i = 0; i < 12; ++i)
        g_memory[0x316 + i] = i == 11 || random(8) == 0 ? 0 : x86::reg8((random(2) ? 'a' : 'A') + random(26));
    g_memory[0] = 0;
    std::memcpy(g_memory + 0x7a28b4, "cars\\", 6);
    std::memcpy(g_memory + 0x7a3234, "fe\\", 4);

    // The showcase's state.
    setWord32(0x559578, 0x539758);
    setWord32(0x55957c, random(8));
    setWord32(0x559584, random(2) ? word32(0x55957c) : random(8));
    setWord32(0x559580, random(2));
    setWord32(0x664794, random(40));
    setWord32(0x664798, random(2) ? word32(0x664794) : random(40));
    setWord32(0x559588, random(4) == 0 ? random(30) : random(3));
    setWord32(0x55958c, random(8) == 0 ? random(9) : 1 + random(6));
    const x86::reg32 clock = random(8) == 0 ? random(0) : 0x10000 + random(0x10000);
    setWord32(0x79c1a4, clock);
    setWord32(0x559594, clock - random(400));
    setWord32(0x559590, clock - random(1200));
    setWord32(0x66296c, random(2) ? 9 : random(12));
    setWord32(0x559570, random(2));
    setWord32(0x664788, random(40));
    setWord32(0x664784, random(40));
    setWord32(0x66478c, random(2) ? word32(0x664788) : random(40));
    setWord32(0x664790, random(2) ? word32(0x664784) : random(40));
    g_memory[0x565010] = x86::reg8(random(2) ? 8 : 16);
}
