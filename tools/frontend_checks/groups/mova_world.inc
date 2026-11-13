// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // DirectDraw's object and the overlay surface as they are in the game: a table of methods.
    constexpr x86::reg32 kObject = 0x8ca400, kTable = 0x8ca420, kSurface = 0x8ca480;
    constexpr x86::reg32 kFakeCaps = 0x7e1000, kFakeRelease = 0x7e1004;
    static const bool movaStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[0x7e1000] = { EAX | EBX | ECX | EDX, 3, 12, 0, Kind::Plain };
        specs[0x7e1004] = { 0, 1, 4, 0, Kind::Plain };
        // GetCaps: the sizes it was handed read, the byte of the flags written, most of
        // the time an answer of no error.
        g_methods[0x7e1000] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 card = word32(cpu.esp + 8), emulation = word32(cpu.esp + 12);
            g_log.push_back(word32(card));
            g_log.push_back(word32(emulation));
            nfs3hp::mock(a, cpu, 0x7e1000);
            static const x86::reg8 kFlags[] = { 0x60, 0x20, 0x40, 0x03, 0x01, 0x02, 0x63, 0x00 };
            g_memory[card + 5] = scripted(3) == 0 ? x86::reg8(scripted(256)) : kFlags[scripted(8)];
            if (scripted(4) != 0)
                cpu.eax = 0;
            // What a real call leaves in edx is the method's: the original's dh goes over it.
            cpu.edx = scripted(0);
        };
        return true;
    }();
    (void)movaStandIns;
    setWord32(0x9f38d0, kObject);
    setWord32(kObject, kTable);
    setWord32(kSurface, kTable);
    setWord32(kTable + 0x2c, kFakeCaps);
    setWord32(kTable + 0x08, kFakeRelease);
    // The screen's device, the same as the surface was made for or not; a surface or none.
    setWord32(0x8ca2c0, random(2) ? word32(0x9f3d3c) : random(0));
    setWord32(0x8ca2c8, random(3) == 0 ? 0 : kSurface);
    setWord32(0x8ca2d4, random(3));
    // The projection: the game's own sine table half the time, junk the rest; the aspect a
    // real 16.16 or junk; real places and sizes or junk.
    if (random(2))
        std::memcpy(g_memory + 0x5672ac, g_image.data() + (0x5672ac - 0x534000), 0x101 * 4);
    if (random(2))
        setWord32(0x564450, 0x10000 + random(0x8000));
    if (random(2))
    {
        setWord32(0x564424, random(0x400));
        setWord32(0x564428, random(0x300));
        setWord32(0x56442c, random(0x400));
        setWord32(0x564430, random(0x300));
    }
}
