// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // GetAsyncKeyState, by what its import's slot holds in the exe.
        specs[word32(0x534738)] = { 0, 1, 4, 0, Kind::Plain };
        // The HUD editor's: each keeps what it pushes.
        specs[0x459db0] = { EAX, 0, 0, 0, Kind::Bool };                       // the place judged
        specs[0x459fe0] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x45a120] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x45ab30] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x47bc20] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x459440] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x49c0e0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x44aeb0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x449ea0] = { EAX, 0, 0, 0, Kind::Plain };
        // The front end's start: edx and ebx come through what these call, as the generated code counts on.
        specs[0x43b390] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x473010] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4960e0] = { EAX, 0, 0, 0, Kind::Plain, true };
        specs[0x4961e0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x473020] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x472fc0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x472d10] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x43c040] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x472820] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4f0240] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x477c00] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49ede0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49e220] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x49e2e0] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x473080] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4b56d0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4bee80] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4483a0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x44b230] = { EAX | EDX, 0, 0, 0, Kind::Plain, true };
        // A screen's effects.
        specs[0x4ee310] = { EAX | EDX, 0, 0, EDX, Kind::Bool, true };         // strcmp: 0 the same
        specs[0x4a46f0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4ef1e0] = { EAX | EDX, 0, 0, EDX, Kind::Tuning };             // 0, or a shape
        specs[0x4ef270] = { EAX | EDX, 0, 0, EDX, Kind::Tuning };
        // The pause's end.
        specs[0x435310] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x44ba30] = { 0, 0, 0, 0, Kind::Plain };
        // The next screens the menu loop is handed: on the stack, which the memory comparison does not hold.
        g_methods[0x44b230] = [](win32::WinApplication* a, x86::CPU& cpu) {
            if (cpu.edx != 0)
                for (x86::reg32 at = 0; at < 12; at += 4)
                    g_log.push_back(word32(cpu.edx + at));
            nfs3hp::mock(a, cpu, 0x44b230);
        };
        // Where the stack stands as they are called, for the frames nothing else hands out by address.
        g_methods[0x45ab30] = nfs3hp::stackSeen<0x45ab30>;
        g_methods[0x459db0] = nfs3hp::stackSeen<0x459db0>;
        g_methods[0x49bfc0] = nfs3hp::stackSeen<0x49bfc0>;
        g_methods[0x435310] = nfs3hp::stackSeen<0x435310>;
        return true;
    }();
    (void)standInsAdded;

    // The mode: the usual ones, a phone's, now and then none at all.
    static const x86::reg32 kWidths[] = { 640, 800, 1024, 1280, 2400, 1920, 512, 0 };
    static const x86::reg32 kHeights[] = { 480, 600, 768, 720, 1080, 1080, 384, 0 };
    const x86::reg32 mode = random(16) == 0 ? 7 : random(7);
    // Any size at all a quarter of the time: which of them rounds differently by a quotient and by an inverse is a
    // matter of the numbers.
    const bool anySize = random(4) == 0 && mode != 7;
    setWord32(0x7cdac8, anySize ? 1 + random(3000) : kWidths[mode]);
    setWord32(0x7cdacc, anySize ? 1 + random(2000) : kHeights[mode]);
    // The front end's start's switches.
    setWord32(0x55b03c, random(2));
    setWord32(0x6fbb54, random(4) == 0);
    setWord32(0x55e88c, random(4) == 0);
    g_memory[0x55eb2d] = x86::reg8(random(2) ? 0 : random(256));
    setWord32(0x67900c, random(2));
    setWord32(0x6fd3b0, random(3));
    setWord32(0x7a22e0, random(2));
    g_memory[0x7a22dc] = x86::reg8(random(2) ? 0 : random(256));
    g_memory[0x7a22dd] = x86::reg8(random(2) ? 0 : random(256));
    const x86::reg32 asked = random(2) ? 0 : 1 + random(20);
    for (x86::reg32 i = 0; i < asked; ++i)
        g_memory[0x7a1fa0 + i] = x86::reg8('a' + random(26));
    g_memory[0x7a1fa0 + asked] = 0;
    setWord32(0x7a1ffc, random(4));
    // The effects': the shown screen's name (sub_44ae00, which runs as itself, its own case's in the base group),
    // one they look for or none, and the band.
    static const x86::reg32 kScreenNames[] = { 0x53cf98, 0x53cfa4, 0x53cfac, 0x53cfb4, 0x53cfbc, 0x53cfd4 };
    setWord32(0x559230, random(4) == 0 ? 0 : g_menu);
    setWord32(g_menu + 0x14, kScreenNames[random(6)]);
    setWord32(0x55eb10, random(2));
    // The pointer, on the 640 by 480 screen, and the room the keys have.
    setWord32(0x55e5e4, random(8) == 0 ? random(0) : random(640));
    setWord32(0x55e5e8, random(8) == 0 ? random(0) : random(480));
    setWord32(0x677e10, random(4) ? 0 : random(word32(0x7cdacc) + 1));
    setWord32(0x677e14, random(4) ? word32(0x7cdacc) : random(word32(0x7cdacc) + 1));
}
