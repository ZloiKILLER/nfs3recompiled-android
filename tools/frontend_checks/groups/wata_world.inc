// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // The library's functions as they are, for the functions that go on from what they find, and
    // what Windows answers.
    static const bool wataStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto import = [&](x86::reg32 slot, std::uint8_t arguments, Kind kind) {
            specs[word32(slot)] = Spec{ 0, arguments, std::uint8_t(4 * arguments), ECX | EDX, kind };
        };
        import(0x53458c, 1, Kind::Bool);    // LoadLibraryA
        import(0x534558, 2, Kind::Plain);   // GetProcAddress
        import(0x534764, 4, Kind::Plain);   // MessageBoxA
        import(0x534534, 0, Kind::Plain);   // GetLastError
        import(0x53459c, 1, Kind::Plain);   // OutputDebugStringA
        import(0x53446c, 5, Kind::Bool);    // RegOpenKeyExA
        import(0x534470, 6, Kind::Bool);    // RegQueryValueExA
        import(0x534464, 1, Kind::Plain);   // RegCloseKey
        import(0x5344c8, 1, Kind::Plain);   // FindClose
        import(0x5344cc, 2, Kind::Plain);   // FindFirstFileA
        import(0x5344d0, 2, Kind::Bool);    // FindNextFileA
        import(0x534520, 1, Kind::Plain);   // GetFileAttributesA
        import(0x534518, 1, Kind::Plain);   // GetDriveTypeA
        import(0x534744, 1, Kind::Plain);   // GetKeyState
        import(0x5347ac, 4, Kind::Plain);   // keybd_event
        import(0x534758, 2, Kind::Plain);   // GetWindowLongA

        static x86::reg32 slotFirst, slotAttributes, slotDrive;
        slotFirst = word32(0x5344cc), slotAttributes = word32(0x534520), slotDrive = word32(0x534518);
        g_methods[slotFirst] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, slotFirst);
            if (scripted(3) == 0)
                cpu.eax = 0xffffffffu;
        };
        g_methods[slotAttributes] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, slotAttributes);
            const x86::reg32 pick = scripted(3);
            cpu.eax = pick == 0 ? 0xffffffffu : pick == 1 ? 0x10 : 0x20;
        };
        g_methods[slotDrive] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, slotDrive);
            // A disc is looked for in every drive: few of them, for the case that finds none.
            cpu.eax = scripted(g_caseAddress == 0x4b6320 ? 12 : 2) == 1 ? 5 : 3;
        };

        specs[0x4f82b6].kind = Kind::SliderValue;     // strtod: a float on the x87
        specs[0x4ea950] = Spec{ 0, 1, 4, 0, Kind::SliderValue };     // cos and sin of a float on the stack
        specs[0x4ea930] = Spec{ 0, 1, 4, 0, Kind::SliderValue };
        for (const x86::reg32 row : { 0x4e9ec0u, 0x4eb390u, 0x4ee310u, 0x50e1d0u, 0x4f9070u, 0x4df7a0u, 0x4e2de0u, 0x49c080u, 0x459db0u,
                                      0x5310beu, 0x4f9d20u, 0x4b62a0u, 0x4ea040u, 0x4ef270u })
            specs[row].kind = Kind::Bool;
        specs[0x422bd0].kind = Kind::Shape;
        specs[0x4ef190].kind = Kind::Shape;
        specs[0x7e0200] = Spec{ EAX, 0, 0, 0, Kind::Plain };         // a callback

        g_methods[0x4e0820] = [](win32::WinApplication* a, x86::CPU& cpu) {  // strstr
            const x86::reg32 from = cpu.eax;
            const std::string haystack = from < 0x1000000 ? guestString(from, 300) : "", needle = cpu.edx < 0x1000000 ? guestString(cpu.edx, 300) : "x";
            nfs3hp::mock(a, cpu, 0x4e0820);
            const auto found = haystack.find(needle);
            cpu.eax = found == std::string::npos ? 0 : from + x86::reg32(found);
        };
        g_methods[0x4f0430] = [](win32::WinApplication* a, x86::CPU& cpu) {  // atoi
            const std::string text = guestString(cpu.eax, 90);
            nfs3hp::mock(a, cpu, 0x4f0430);
            cpu.eax = x86::reg32(std::atoi(text.c_str()));
        };
        g_methods[0x4f82b6] = [](win32::WinApplication* a, x86::CPU& cpu) {  // strtod
            const std::string text = guestString(cpu.eax, 90);
            nfs3hp::mock(a, cpu, 0x4f82b6);
            cpu.fpu.st(0) = x86::Float(std::strtod(text.c_str(), nullptr));
        };
        g_methods[0x4f0ae0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // tolower
            const int c = int(cpu.eax & 0xff);
            nfs3hp::mock(a, cpu, 0x4f0ae0);
            cpu.eax = x86::reg32(c >= 'A' && c <= 'Z' ? c + 32 : c);
        };
        g_methods[0x4f0548] = [](win32::WinApplication* a, x86::CPU& cpu) {  // sscanf: a number, or none
            const x86::reg32 out = word32(cpu.esp + 12);
            nfs3hp::mock(a, cpu, 0x4f0548);
            setWord32(out, scripted(40));
            cpu.eax = scripted(3) ? 1 : 0;
        };
        g_methods[0x4edfe8] = [](win32::WinApplication* a, x86::CPU& cpu) {  // fopen
            nfs3hp::mock(a, cpu, 0x4edfe8);
            cpu.eax = scripted(5) ? 0x940000 : 0;
        };
        g_methods[0x4f0570] = [](win32::WinApplication* a, x86::CPU& cpu) {  // fgets
            const x86::reg32 buffer = cpu.eax;
            nfs3hp::mock(a, cpu, 0x4f0570);
            static const char* const kLines[] = { "english\r\n", "\n", "", "francais\n", "abc\rdef\n", "tail" };
            const char* const line = kLines[scripted(6)];
            std::memcpy(g_memory + buffer, line, std::strlen(line) + 1);
            if (scripted(3) == 0)
                g_memory[0x940000 + 0xc] |= 0x10;
        };
        g_methods[0x4e1620] = [](win32::WinApplication* a, x86::CPU& cpu) {  // reservememadr
            nfs3hp::mock(a, cpu, 0x4e1620);
            cpu.eax = g_caseAddress == 0x4f0da0 ? 0x950000 : 0x958000;
        };
        // A request off a queue: one of the sixteen, or none.
        static const auto takeRequest = [](win32::WinApplication* a, x86::CPU& cpu, x86::reg32 address) {
            nfs3hp::mock(a, cpu, address);
            cpu.eax = scripted(3) == 0 ? 0 : 0x948000 + 0x40 * scripted(16);
        };
        g_methods[0x50df60] = [](win32::WinApplication* a, x86::CPU& cpu) { takeRequest(a, cpu, 0x50df60); };
        g_methods[0x50dff0] = [](win32::WinApplication* a, x86::CPU& cpu) { takeRequest(a, cpu, 0x50dff0); };
        g_methods[0x50e5a0] = [](win32::WinApplication* a, x86::CPU& cpu) { takeRequest(a, cpu, 0x50e5a0); };
        // What the reader's waits wait for comes with the wait.
        g_methods[0x4df8e0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // sleepMs
            nfs3hp::mock(a, cpu, 0x4df8e0);
            const x86::reg32 record = word32(0x9f0be0);
            if (record == 0)
                return;
            if (g_caseAddress == 0x4f0da0)
            {
                setWord32(record, word32(record) == 0);
                setWord32(record + 0x18, 0);
            }
            else if (g_caseAddress == 0x4f1820)
                setWord32(record + 4, scripted(3) == 0);
            else
            {
                setWord32(record, 0);
                setWord32(record + 0x18, 0);
            }
        };
        g_methods[0x4dfb70] = [](win32::WinApplication* a, x86::CPU& cpu) {  // waitEvent
            nfs3hp::mock(a, cpu, 0x4dfb70);
            setWord32(word32(0x9f0be0) + 4, 1);
        };
        g_methods[0x4e7630] = [](win32::WinApplication* a, x86::CPU& cpu) {  // runSystemTasks: time goes by
            nfs3hp::mock(a, cpu, 0x4e7630);
            setWord32(0x79c1a4, word32(0x79c1a4) + 1);
        };
        g_methods[0x4f24e0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // a shape's name
            nfs3hp::mock(a, cpu, 0x4f24e0);
            std::string name;
            for (const x86::reg32 word : { 0x53b300u, 0x53b304u, 0x53b308u, 0x53b30cu, 0x53b314u, 0x53b31cu })
                if (scripted(8) != 0)
                    name += guestString(word) + std::to_string(scripted(360)) + " ";
            std::memcpy(g_memory + 0x941000, name.c_str(), name.size() + 1);
            cpu.eax = scripted(6) ? 0x941000 : 0;
        };
        g_methods[0x4d1850] = [](win32::WinApplication* a, x86::CPU& cpu) {  // a car's name
            nfs3hp::mock(a, cpu, 0x4d1850);
            static const x86::reg32 kWords[] = { 0x53cf40, 0x53cf48, 0x53cf50, 0x53cf5c, 0x53cf68, 0x53cf70, 0x53cf40 };
            const std::string name = scripted(7) == 0 ? "other" : "the " + guestString(kWords[scripted(7)]) + " car";
            std::memcpy(g_memory + 0x941000, name.c_str(), name.size() + 1);
            cpu.eax = 0x941000;
        };
        return true;
    }();
    (void)wataStandIns;
}


// The watched memory comes filled with noise: the tables of names and the character classes these
// functions go through are the image's.
{
    const auto fromImage = [](x86::reg32 at, x86::reg32 bytes) { std::memcpy(g_memory + at, g_image.data() + (at - 0x534000), bytes); };
    fromImage(0x564ef0, 0x101);
    fromImage(0x55eb34, 4 * 0x21);
    fromImage(0x55ebb8, 4 * 0x21);
    fromImage(0x55ec3c, 4 * 5);
    fromImage(0x55ec50, 4 * 0xc);
    fromImage(0x55eca0, 4 * 4);
    fromImage(0x55fd24, 4 * 6);
    fromImage(0x554fa8, 2 * 12);
    fromImage(0x566b54, 2 * 256);
}

// carp.txt's reader: a record's room, the defaults, what it has read.
{
    constexpr x86::reg32 kRecord = 0x920000;
    for (x86::reg32 at = 0; at < 0x4ac; at += 4)
        setWord32(kRecord + at, random(0));
    for (x86::reg32 at = 0; at < 0xe00; at += 4)
        setWord32(0x55ef20 + at, random(3) == 0 ? random(0) : 0x3f800000 + random(0x100000));
    for (x86::reg32 i = 0; i < 0x50; ++i)
        g_memory[0x55ed8c + i] = x86::reg8(random(3) == 0);
    setWord32(0x55ed88, 0);
}

// The reader's record and its requests.
{
    constexpr x86::reg32 kRequests = 0x948000, kName = 0x941800;
    setWord32(0x9f0be0, random(8) == 0 ? 0 : 0x950000);
    std::memset(g_memory + 0x950000, 0, 0x100);
    setWord32(0x950000, random(2));
    setWord32(0x950008, random(2) * random(0));
    setWord32(0x950014, 0x950000);
    setWord32(0x950018, random(2) ? 0 : kRequests + 0x40 * random(16));
    std::memcpy(g_memory + kName, "file.dat", 9);
    for (x86::reg32 r = 0; r < 16; ++r)
    {
        const x86::reg32 request = kRequests + 0x40 * r;
        for (x86::reg32 at = 0; at < 0x28; at += 4)
            setWord32(request + at, random(0));
        setWord32(request + 8, random(2) * 0x958000);
        setWord32(request + 0x10, random(0x10000));
        setWord32(request + 0x18, random(3) == 0 ? 0 : random(2) ? 0x950000 : 0x958000);
        setWord32(request + 0x1c, random(2) * kName);
        setWord32(request + 0x20, random(7));
        setWord32(request + 0x24, random(2) * 0x7e0200);
    }
    std::memset(g_memory + 0x940000, 0, 0x20);
}

// Files found, drives, the language, the DLLs' state.
{
    setWord32(0x5651d4, random(3) == 0 ? 0xffffffffu : random(0));
    setWord32(0x9f09c0, random(8));
    std::memcpy(g_memory + 0x9f09c4, "d:\\dir\\*.fsh", 13);
    setWord32(0x7a3a50, random(6));
    setWord32(0x6fd2bc, random(12));
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x55ec80 + 4 * i, random(4) == 0);
    setWord32(0x7a1fdc, random(2));
    setWord32(0x7a1ff8, random(2));
    g_memory[0x7a1fa0] = x86::reg8(random(2) * 'a'), g_memory[0x7a1fa1] = 0;
    g_memory[0x7a1fe0] = x86::reg8(random(2) * 'a'), g_memory[0x7a1fe1] = 0;
    setWord32(0x79c1a4, random(0x10000));
    setWord32(0x5651dc, random(0x10000));
    setWord32(0x565270, random(2) ? 0xffffffffu : random(4));
    setWord32(0x5643b4, random(2));
    setWord32(0x567dcc, random(2));
    setWord32(0x565028, random(4) != 0);
    setWord32(0x5671f8, random(640));
    setWord32(0x5671fc, random(480));
    setWord32(0x9f0bdc, random(100));
    setWord32(0x9f0bd8, random(100));
    setWord32(0x9f0bd0, random(600));
    setWord32(0x9f0bd4, random(400));
    setWord32(0x5662e0, random(8));
    setWord32(0x55e558, random(4));
    setWord32(0x79f5c4, 0x942000);
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0x942000 + 8 * i, random(0));
    setWord32(0x56440c, random(2));
    setWord32(0x564a68, random(2));
    setWord32(0x564a6c, random(2));
    setWord32(0x567194, random(2));
    setWord32(0x5644fc, random(2));
    setWord32(0x564364, random(2));
    setWord32(0x564368, random(2));
    setWord32(0x5671a8, random(5) == 0 ? 0xffffffffu : 0);
    setWord32(0x9f4d90, random(2));
    g_memory[0x9f3d2f] = x86::reg8(random(3) == 0);
    setWord32(0x9f3890, random(4) == 0 ? 0x21 : random(5));
    for (x86::reg32 i = 0; i < 6; ++i)
        setWord32(0x9f380c + 4 * i, 0x7e0200);
}

// A HUD element under the mouse.
{
    constexpr x86::reg32 kElement = 0x944000;
    static const x86::reg32 kHandles[] = { 0, 0, 0x108, 0x208, 0x409, 0x50a, 0x60b, 0x809, 0x90b, 0xa0a, 0x30c };
    const auto word = [](x86::reg32 at, x86::reg32 value) { g_memory[at] = x86::reg8(value), g_memory[at + 1] = x86::reg8(value >> 8); };
    word(kElement + 6, random(640) - 40);
    word(kElement + 8, random(480) - 40);
    word(kElement + 0x1a, random(100));
    word(kElement + 0x1c, random(100));
    word(kElement + 0x1e, random(100));
    word(kElement + 0x20, random(100));
    setWord32(kElement + 0x3c, random(40));
    for (x86::reg32 at = 0x50; at < 0x60; at += 4)
        setWord32(kElement + at, random(0));
    setWord32(kElement + 0x60, random(700));
    setWord32(kElement + 0x64, random(500));
    setWord32(kElement + 0x68, x86::reg32(random(80)) - 40);
    setWord32(kElement + 0x6c, x86::reg32(random(80)) - 40);
    setWord32(kElement + 0x70, random(2));
    setWord32(kElement + 0x74, random(2) * (kElement + 0x200));
    setWord32(kElement + 0x78, kHandles[random(11)]);
    word(kElement + 0x7c, random(2));
    word(kElement + 0x100, random(100));
    word(kElement + 0x102, random(100));
    setWord32(0x7cdac8, random(2) ? 640 : 1280);
    setWord32(0x7cdacc, random(2) ? 480 : 720);
    setWord32(0x55e5e4, random(640));
    setWord32(0x55e5e8, random(480));
    setWord32(0x677df8, random(2));
    setWord32(0x677dfc, random(4));
    setWord32(0x6fd3b0, random(2));
    // Now and then a side that resizing leaves right at the smallest it may be.
    if (random(2) == 0)
    {
        const auto doubleAt = [](x86::reg32 at) { double value; std::memcpy(&value, g_memory + at, 8); return value; };
        const x86::reg32 handle = word32(kElement + 0x78);
        const auto place = [&](x86::reg32 screen, x86::reg32 mouse, x86::reg32 toPixels, x86::reg32 toPercent, x86::reg32 nearEdge, x86::reg32 farEdge,
                               x86::reg32 grab, x86::reg32 movesNear, x86::reg32 movesFar) {
            const x86::reg32 size = word32(screen);
            const x86::sreg32 at = x86::sreg32(word32(mouse) * size * doubleAt(toPixels));
            const x86::sreg32 pixels = x86::sreg32(26.0 * size / doubleAt(toPercent)) + x86::sreg32(random(3));
            setWord32(kElement + grab, 0);
            if (handle & movesNear)
                setWord32(kElement + farEdge, x86::reg32(at + pixels - 1));
            if (handle & movesFar)
                word(kElement + nearEdge, x86::reg32(at - pixels + 1));
        };
        place(0x7cdac8, 0x55e5e4, 0x539bd4, 0x539bec, 6, 0x60, 0x68, 0x400, 0x800);
        place(0x7cdacc, 0x55e5e8, 0x539bdc, 0x539bf4, 8, 0x64, 0x6c, 0x100, 0x200);
    }
}
