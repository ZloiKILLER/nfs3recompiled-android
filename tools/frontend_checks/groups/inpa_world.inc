// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static const bool inpaStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // sub_4f2a7d leaves the game: nothing after its call is code, whatever the rows say of it.
        specs[0x4f2a7d] = { 0, 0, 0, EBX | ECX | EDX | EDI, Kind::Plain };
        // What is read as an answer.
        specs[0x48f7b0] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        specs[0x48f8d0] = { EAX | EDX, 0, 0, 0, Kind::Bool };
        specs[0x4eb1c0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x4c4de0] = { EAX, 0, 0, ESI | EDI, Kind::Bool };
        specs[0x4f0100] = { EAX, 0, 0, EBX | ECX, Kind::Bool };
        // Handed eax or edx by the generated code though the file's rows do not read them: compared all the same.
        specs[0x426680] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4c48d0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4ead90] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4eb360] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        // The system's lock, through the import table: what is there in the file.
        specs[word32(0x5344b8)] = { 0, 1, 4, 0, Kind::NoEax };
        specs[word32(0x534588)] = { 0, 1, 4, 0, Kind::NoEax };

        g_methods[0x4e7630] = &nfs3hp::inpaSystemTasks;
        g_methods[0x435310] = &nfs3hp::inpaReadMouse;
        g_methods[0x4c5720] = &nfs3hp::inpaSmall<0x4c5720, 7>;
        g_methods[0x4c69a0] = &nfs3hp::inpaFramesTaken;
        g_methods[0x4c6970] = &nfs3hp::inpaSmall<0x4c6970, 8>;
        g_methods[0x4c4de0] = &nfs3hp::inpaFrameHeld;
        g_methods[0x4a60e0] = &nfs3hp::inpaSendPacket;
        g_methods[0x48f8d0] = &nfs3hp::inpaAtEsp<0x48f8d0>;
        g_methods[0x4eb2e0] = &nfs3hp::inpaAtEsp<0x4eb2e0>;
        g_methods[0x4e070c] = &nfs3hp::inpaAtEsp<0x4e070c>;
        g_methods[0x4a6a10] = &nfs3hp::inpaAtEsp<0x4a6a10>;
        g_methods[0x4c6a40] = &nfs3hp::inpaAtEsp<0x4c6a40>;
        g_methods[0x4eb360] = &nfs3hp::inpaAtEsp<0x4eb360>;
        // What each of these may leave changed for what follows them to read.
        g_methods[0x4bc820] = &nfs3hp::inpaMovesFlags<0x4bc820>;
        g_methods[0x4bd640] = &nfs3hp::inpaPanelKeys;
        g_methods[0x4c48d0] = &nfs3hp::inpaMovesFlags<0x4c48d0>;
        g_methods[0x426680] = &nfs3hp::inpaMovesFlags<0x426680>;
        g_methods[0x4bc680] = &nfs3hp::inpaMovesFlags<0x4bc680>;
        g_methods[0x4ce750] = &nfs3hp::inpaMovesFlags<0x4ce750>;
        g_methods[0x44bac0] = &nfs3hp::inpaMovesFlags<0x44bac0>;
        g_methods[0x4d5d10] = &nfs3hp::inpaMovesFlags<0x4d5d10>;
        g_methods[0x4a33d0] = &nfs3hp::inpaMovesFlags<0x4a33d0>;
        g_methods[0x470fc0] = &nfs3hp::inpaMovesFlags<0x470fc0>;
        g_methods[0x42d800] = &nfs3hp::inpaMovesFlags<0x42d800>;
        return true;
    }();
    (void)inpaStandIns;

    // The race's flags: a few values each, mostly the ordinary one.
    const auto oneIn = [](x86::reg32 n) { return random(n) == 0; };
    setWord32(0x6fd3b0, oneIn(3) ? random(5) : random(2));              // the game's mode
    setWord32(0x6fd3a0, oneIn(3) ? random(4) : random(2));              // a replay
    setWord32(0x7cda58, oneIn(3) ? 2 : oneIn(3) ? random(6) : 0);        // the panel's state
    setWord32(0x7d369c, oneIn(5));                                      // leaving
    setWord32(0x7d3698, random(2));
    setWord32(0x7d36a0, oneIn(3));                                      // paused
    setWord32(0x7d36a8, oneIn(4));
    setWord32(0x7d36b4, oneIn(2) ? 0 : random(3));
    setWord32(0x7cd948, oneIn(3));
    setWord32(0x7cd94c, random(5));
    setWord32(0x7cd95c, oneIn(2) ? 4 : random(6));
    setWord32(0x7d3674, oneIn(4));
    g_memory[0x55e5ed] = x86::reg8(oneIn(4) ? random(256) : random(4));
    g_memory[0x55eb2e] = x86::reg8(oneIn(2) ? 0x20 : random(256));
    g_memory[0x7a22dc] = x86::reg8(oneIn(5));
    g_memory[0x7a22dd] = 0;
    g_memory[0x7a22da] = x86::reg8(random(2));
    g_memory[0x7a22db] = 0;

    // The clocks and the frames, near each other so that their comparisons go both ways.
    const x86::reg32 ticks = oneIn(8) ? random(2) * 0xfffffff0u : 1 + random(100000);
    setWord32(0x5efdf8, ticks);
    setWord32(0x880f00, ticks);                                         // for inpaSystemTasks
    const x86::reg32 menuTicks = random(1000);
    setWord32(0x5efdfc, menuTicks);
    setWord32(0x7d36a4, menuTicks + random(8) - 4);
    const x86::reg32 frame = random(1000);
    setWord32(0x79c4ac, frame);
    setWord32(0x7d3688, oneIn(8) ? frame - 20 : frame + random(8) - 4);
    const x86::reg32 ready = random(40);
    setWord32(0x560104, ready);
    setWord32(0x7d36b0, 2 * ready + random(6) - 3);
    setWord32(0x7d36ac, 2 * ready);
    setWord32(0x5600ec, random(2));

    // The developer's keys.
    setWord32(0x5600fc, oneIn(2));
    setWord32(0x55301c, oneIn(2));
    setWord32(0x553020, random(0));
    setWord32(0x7d3670, random(0));

    // The pointer and the cars it may turn the views to.
    setWord32(0x7d367c, random(3));
    setWord32(0x7d3678, random(3));
    setWord32(0x7d36b8, random(3));
    setWord32(0x7d36bc, random(3));
    setWord32(0x7cdb2c, random(3));
    setWord32(0x6fd4f4, oneIn(2) ? 0 : random(4));
    setWord32(0x6fd4f8, random(4));
    for (x86::reg32 p = 0; p < 4; ++p)
    {
        const x86::reg32 car = 0x880000 + 0x400 * p;
        setWord32(0x5efac8 + 4 * p, car);
        g_memory[car + 0x200] = x86::reg8(random(2));
        setWord32(car + 0x2c4, oneIn(2) ? random(6) : 3 + random(2));
    }

    // A network race's hold.
    setWord32(0x7d36c0, oneIn(2));
    setWord32(0x7d36c4, random(0));
    setWord32(0x7d36c8, oneIn(2) ? 0 : random(64));
    setWord32(0x7d36cc, random(64) - 0x20);
    setWord32(0x7d36d8, random(0));
    setWord32(0x560114, oneIn(2) ? 0 : random(0));
    setWord32(0x55e88c, oneIn(3));
}
