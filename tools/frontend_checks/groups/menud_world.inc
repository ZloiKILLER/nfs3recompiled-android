// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // The calls these functions make that no other screen does: what each reads,
    // what its ret pops, what it changes.  Those native in the game restore every
    // register but eax; the clobbers here are what the generated callers may not lean on.
    static const bool menudSpecs = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs.emplace(0x4a6690, Spec{ EAX, 0, 0, 0, Kind::Plain });                       // playerMachine: keeps edx
        specs.emplace(0x452190, Spec{ EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain, true });  // drawMenuText
        specs.emplace(0x452120, Spec{ EAX | EBX | ECX | EDX, 3, 12, EBX | ECX | EDX, Kind::Plain, true }); // a name in a box
        specs.emplace(0x440b30, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain });
        specs.emplace(0x440b50, Spec{ EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain });
        specs.emplace(0x4d8550, Spec{ EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain });
        specs.emplace(0x4d8930, Spec{ EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain });
        specs.emplace(0x4e0ec0, Spec{ EAX | EDX, 0, 0, 0, Kind::Plain, true });           // loadFile
        specs.emplace(0x4daa10, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4956f0, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain, true });         // loadQfs
        specs.emplace(0x4d6070, Spec{ EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain });     // shapeTexture
        specs.emplace(0x4d9180, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain });               // a colour scaled
        specs.emplace(0x4d88f0, Spec{ EAX | EBX | ECX | EDX, 4, 16, EBX | ECX | EDX, Kind::Plain });  // a rectangle
        specs.emplace(0x4ee310, Spec{ EAX | EDX, 0, 0, EDX, Kind::Bool, true });          // strcmp
        specs.emplace(0x453940, Spec{ 0, 0, 0, 0, Kind::Plain });
        specs.emplace(0x454780, Spec{ 0, 0, 0, 0, Kind::Plain });
        specs.emplace(0x453f60, Spec{ 0, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4eb1c0, Spec{ EAX, 0, 0, 0, Kind::Bool });                        // a key held
        specs.emplace(0x431900, Spec{ EAX | EDX, 0, 0, 0, Kind::Plain });                 // setRenderState: keeps ebx, ecx
        specs.emplace(0x4d6730, Spec{ EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain });  // drawTurnedShape
        specs.emplace(0x4d7160, Spec{ EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain });  // drawArtShape
        specs.emplace(0x44be20, Spec{ 0, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4bc530, Spec{ 0, 0, 0, 0, Kind::Plain });
        // The system's lock, through the import table: what is there in the file.
        specs.emplace(word32(0x5344b8), Spec{ 0, 1, 4, 0, Kind::NoEax });
        specs.emplace(word32(0x534588), Spec{ 0, 1, 4, 0, Kind::NoEax });
        return true;
    }();
    (void)menudSpecs;

    // The shown screen, as sub_44ae00 reads it: a screen by name, or none.
    static const char* const kNames[] = { "showcase", "compare2", "history", "trkinfo", "main" };
    const x86::reg32 nameAt = 0x5f3f40;
    std::strcpy(reinterpret_cast<char*>(g_memory) + nameAt, kNames[random(std::size(kNames))]);
    setWord32(0x559230, random(5) == 0 ? 0 : g_menu);
    setWord32(g_menu + 0x14, nameAt);

    // The pointer: near what the cases ask of it.
    setWord32(0x55e5e4, random(40));
    setWord32(0x55e5e8, random(40));
    setWord32(0x55e5fc, random(2) ? 0 : random(0));

    // The band: below, above and at its ends, the light on its rows or not.
    static const x86::reg32 kBandBottoms[] = { 0x1df, 0x1e0, 0x1cc, 0x60, 0x61, 0x5f, 0x74, 0x100 };
    setWord32(0x55eb0c, random(3) == 0 ? random(0x220) : kBandBottoms[random(std::size(kBandBottoms))]);
    setWord32(0x55eb10, random(2));
    setWord32(0x55eb14, random(3) == 0 ? random(0x26) : random(5) == 0 ? 0 : 0xc8 + random(0x20));

    // The glint's clock: its 300 passes, the ones that show it often.
    setWord32(0x55eb28, random(3) == 0 ? random(0x12d) : 0x12c - random(0x40));

    // The lobby: who is there, and their records' names, ended.
    setWord32(0x7a2274, x86::reg32(random(3) == 0 ? random(9) : random(2)) << 16);
    setWord32(0x7a22e4, random(2));
    setWord32(0x55ecb4, random(0));
    for (x86::reg32 slot = 0; slot < 8; ++slot)
    {
        const x86::reg32 record = 0x79f5f0 + 0x60 * slot;
        g_memory[0x7a227a + 2 * slot] = x86::reg8(random(3) == 0 ? 0xff : slot);
        g_memory[0x7a227b + 2 * slot] = x86::reg8(random(3) == 0 ? 0xff : 0);
        setWord32(record + 4, random(2) ? slot : random(9));
        for (const x86::reg32 at : { 0x09u, 0x29u, 0x39u })
        {
            const x86::reg32 length = random(4) == 0 ? 0 : random(15);
            for (x86::reg32 c = 0; c < length; ++c)
                g_memory[record + at + c] = x86::reg8('A' + random(26));
            g_memory[record + at + length] = 0;
        }
    }
}
