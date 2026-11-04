// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.
    static const bool menueSpecsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x431900] = { EAX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain };              // setRenderState
        specs[0x4c12e0] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain };  // clipQuad
        specs[0x42d500] = { 0, 2, 0, 0, Kind::Plain };                                    // the log, cdecl
        specs[0x4f0120] = { 0, 0, 0, 0, Kind::Plain };                                    // inkey
        specs[0x4d8570] = { EAX | EBX | ECX | EDX, 5, 20, EBX | ECX | EDX, Kind::Plain }; // drawShadedTriangle
        specs[0x4ef190] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Shape };  // a new bitmap
        specs[0x4eff00] = { EAX, 0, 0, 0, Kind::Plain };                                  // its end
        specs[0x4d6070] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain };  // shapeTexture
        // locateShape, which finds no shape now and then.
        specs[0x4ef270] = { EAX | EDX, 0, 0, EDX, Kind::Tuning, false };
        return true;
    }();
    (void)menueSpecsAdded;

    // What these functions only read, made up here: their memory is no other group's.
    fillRandom(0x7d3000, 0x7d4000);
    fillRandom(0x8b4000, 0x8b6000);

    // The 2D layer's depth, real floats, and the window the corners are clipped
    // to: with corners on both sides of each edge.
    setFloat(0x563aa8, float(random(5)) * 0.25f);
    setFloat(0x563aac, random(4) == 0 ? -1.0f : float(1 + random(4)) * 0.5f);
    setFloat(0x7d34f8, float(1 + random(4)) * 0.5f);
    setFloat(0x7d34fc, float(random(100)));
    setFloat(0x7d3500, float(300 + random(200)));
    setFloat(0x7d3504, float(random(100)));
    setFloat(0x7d350c, float(400 + random(240)));
    g_memory[0x7a3a58] = x86::reg8(random(4) == 0 ? 2 : 0);
    setWord32(0x559234, random(4) == 0);

    // The art records: tiles somewhere in the tiles' table, a run of them or
    // none; not loaded now and then.
    for (x86::reg32 art = 0; art < 8; ++art)
    {
        const x86::reg32 record = 0x8b48bc + 0x14 * art;
        const std::int16_t first = random(8) == 0 ? -1 : std::int16_t(random(40));
        const std::int16_t end = std::int16_t(first + std::int32_t(random(8)) - 1);
        std::memcpy(g_memory + record + 0x10, &first, 2);
        std::memcpy(g_memory + record + 0x12, &end, 2);
        const std::int16_t x = std::int16_t(std::int32_t(random(640)) - 100);
        std::memcpy(g_memory + record + 4, &x, 2);
    }
    for (x86::reg32 tile = 0; tile < 50; ++tile)
    {
        const x86::reg32 at = 0x8b5090 + 12 * tile;
        const std::int16_t side = std::int16_t(random(64));
        const std::int16_t column = std::int16_t(std::int32_t(random(20)) - 4);
        const std::int16_t row = std::int16_t(std::int32_t(random(16)) - 2);
        std::memcpy(g_memory + at + 4, &side, 2);
        std::memcpy(g_memory + at + 6, &column, 2);
        std::memcpy(g_memory + at + 8, &row, 2);
    }

    // The pointer's kind: all of them, past their ends and none.
    setWord32(0x563d3c, random(5) == 0 ? random(0) : x86::reg32(random(18)) - 2);
    // The language's block, or none.
    setWord32(0x563a80, random(3) == 0 ? 0 : random(0));
    // The player's units.
    setWord32(0x6fbc68, random(3) == 0 ? random(0) : random(6));
    setWord32(0x6fbb4c, random(6));
}
