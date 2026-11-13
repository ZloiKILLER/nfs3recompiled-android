// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kOtherDisplay = 0xa10000, kComTable = 0xa10900, kNetTable = 0xa10a00;
    constexpr x86::reg32 kDirectDrawObject = 0xa10800, kPaletteObject = 0xa10810, kSurfaceObject = 0xa10820;
    constexpr x86::reg32 kModes = 0xa11000, kText = 0xa13000, kGlyphs = 0xa14000, kKerningPairs = 0xa16000, kTabs = 0xa16100;
    constexpr x86::reg32 kColourTable = 0xa30000, kThreads = 0xa23000, kRequests = 0xa2f000, kPool = 0xa22400;
    constexpr x86::reg32 kDumpBlocks = 0xa22500, kNameText = 0xa15100, kLabelText = 0xa15200;
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto windows = [&](x86::reg32 slot, std::uint8_t words, Kind kind) {
            specs[word32(slot)] = { 0, words, std::uint8_t(words * 4), ECX | EDX, kind };
        };
        windows(0x5344b8, 1, Kind::NoEax);   // EnterCriticalSection
        windows(0x534588, 1, Kind::NoEax);   // LeaveCriticalSection
        windows(0x534568, 0, Kind::Plain);   // GetTickCount
        windows(0x53473c, 2, Kind::Plain);   // GetClientRect
        windows(0x534700, 2, Kind::Plain);   // ClientToScreen
        windows(0x534750, 1, Kind::Focus);   // GetSystemMetrics
        windows(0x534758, 2, Kind::Plain);   // GetWindowLong
        windows(0x5346ec, 3, Kind::Plain);   // AdjustWindowRect
        windows(0x53478c, 7, Kind::Plain);   // SetWindowPos
        windows(0x534468, 1, Kind::Plain);
        windows(0x53475c, 1, Kind::Bool);    // IsIconic
        windows(0x534488, 1, Kind::Bool);    // CloseHandle
        specs[word32(0x5347b0)] = { 0, 2, 0, ECX | EDX, Kind::Plain };  // wsprintf
        // sub_4ea290: a BIG archive's entry, by the answer 0 or a short name.
        g_methods[0x4ea290] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x4ea290);
            const x86::reg32 pick = cpu.eax % 3;
            cpu.eax = pick == 0 ? 0 : 0xa15300 + pick;
            for (x86::reg32 i = 0; i < 7; ++i)
                app->getMemory<x86::reg8>(0xa15300 + i) = i == 6 ? 0 : x86::reg8('a' + i);
        };
        g_methods[word32(0x5347b0)] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 buffer = word32(cpu.esp + 4), formatAt = cpu.esp + 8;
            nfs3hp::mock(app, cpu, word32(0x5347b0));
            std::string out = nfs3hp::eaaFormat(formatAt);
            if (out.size() > 255)
                out.resize(255);
            std::memcpy(g_memory + buffer, out.c_str(), out.size() + 1);
        };
        // COM: stdcall, the object first.
        const auto com = [&](x86::reg32 slot, std::uint8_t words, Kind kind) {
            specs[0x7e0600 + slot] = { 0, words, std::uint8_t(words * 4), ECX | EDX, kind };
        };
        com(0x08, 1, Kind::Plain);
        com(0x10, 5, Kind::Plain);
        com(0x18, 5, Kind::Plain);
        com(0x30, 2, Kind::Plain);
        com(0x54, 6, Kind::Focus);
        com(0x80, 2, Kind::Plain);
        // GetDisplayMode: the mode, into the description.
        g_methods[0x7e0630] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 out = word32(cpu.esp + 8);
            nfs3hp::mock(app, cpu, 0x7e0630);
            setWord32(out + 8, 400 + scripted(300));
            setWord32(out + 0xc, 500 + scripted(400));
            setWord32(out + 0x54, scripted(2) ? 16 : 32);
        };
        // The font's functions.
        specs[0x7e0800] = { EAX | EBX | ECX | EDX, 4, 16, ECX | EDX, Kind::Plain };
        specs[0x7e0804] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x7e0808] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x7e080c] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x7e0810] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x7e0814] = { EAX, 0, 0, 0, Kind::Plain };   // a low timer's function
        specs[0x7e0818] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };   // a request's callback
        g_methods[0x7e080c] = [](win32::WinApplication* app, x86::CPU& cpu) {
            // The reader: the character at the cursor, the cursor past it.
            const x86::reg32 cursor = cpu.eax;
            nfs3hp::mock(app, cpu, 0x7e080c);
            const x86::reg32 at = word32(cursor);
            const x86::reg8 c = g_memory[at];
            if (c != 0)
                setWord32(cursor, at + 1);
            cpu.eax = c;
        };
        g_methods[0x7e0810] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 x = cpu.eax, y = cpu.edx;
            nfs3hp::mock(app, cpu, 0x7e0810);
            setWord32(x, scripted(5));
            setWord32(y, scripted(5));
        };
        // A transport's table.
        specs[0x7e0914] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x7e0918] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x7e091c] = { EAX | EBX | EDX, 0, 0, 0, Kind::Bool };
        specs[0x7e0920] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x7e0944] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };
        // The network's two found functions.
        specs[0x7e0950] = { 0, 3, 12, ECX | EDX, Kind::Bool };
        specs[0x7e0954] = { 0, 4, 16, ECX | EDX, Kind::Bool };
        g_methods[0x7e0950] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 count = word32(cpu.esp + 0xc);
            nfs3hp::mock(app, cpu, 0x7e0950);
            setWord32(count, scripted(3));
        };
        // What hands back a pointer, and what the waiting loops wait for.
        g_methods[0x401010] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 formatAt = cpu.esp + 4;
            nfs3hp::mock(app, cpu, 0x401010);
            nfs3hp::eaaFormat(formatAt);
        };
        g_methods[0x4df6c0] = [](win32::WinApplication* app, x86::CPU& cpu) {
            const x86::reg32 formatAt = cpu.esp + 4;
            nfs3hp::mock(app, cpu, 0x4df6c0);
            nfs3hp::eaaFormat(formatAt);
        };
        g_methods[0x4df690] = &nfs3hp::eaaSprintf;
        g_methods[0x501050] = &nfs3hp::eaaReport;
        g_methods[0x4e1620] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x4e1620);
            cpu.eax = 0xa22000 + 0x40 * (1 + scripted(3));
            if ((g_caseAddress == 0x4eee30 || g_caseAddress == 0x4ef190) && scripted(5) == 0)
                cpu.eax = 0;
        };
        g_methods[0x4e15e0] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x4e15e0);
            cpu.eax = scripted(3) == 0 ? 0 : 0xa15100;
        };
        g_methods[0x4e15d0] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x4e15d0);
            cpu.eax = word32(0xa25ff0);
        };
        g_methods[0x501110] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x501110);
            cpu.eax = 0xa24000;
        };
        g_methods[0x533df6] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x533df6);
            cpu.eax = scripted(3) == 0 ? 0 : 0xa18400;
        };
        g_methods[0x533df0] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x533df0);
            cpu.eax = 0xa18300;
        };
        g_methods[0x533dfc] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x533dfc);
            cpu.eax = scripted(2) == 0 ? x86::reg32(-1) : scripted(0);
        };
        g_methods[0x50df60] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x50df60);
            cpu.eax = scripted(3) == 0 ? 0 : 0xa2f000 + 0x40 * scripted(4);
        };
        // The waits: what they wait for comes after a few.
        static const auto waited = [](win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address) {
            nfs3hp::mock(app, cpu, address);
            switch (g_caseAddress)
            {
            case 0x4f7450:
            {
                const x86::reg32 record = word32(0x9f37f0);
                const x86::reg32 pick = scripted(4);
                if (pick == 0)
                    g_memory[record + 0x47] = 1;
                else if (pick == 1)
                    g_memory[record + 0x46] = 1;
                break;
            }
            case 0x4f75d0:
                if (scripted(2) == 0)
                    g_memory[0xa22000 + 0x47] = 0;
                break;
            case 0x514320:
                if (scripted(2) == 0)
                    setWord32(0xa22300 + 0xa8, 0);
                break;
            case 0x4fc170:
                if (scripted(2) == 0)
                    for (x86::reg32 at = 0; at < 0xd00; at += 0x68)
                        setWord32(0xa23000 + at, 0);
                break;
            case 0x4fbda0:
                if (scripted(3) == 0)
                    setWord32(0x5671f0, 1);
                break;
            case 0x4f27a0:
                if (scripted(3) == 0)
                    setWord32(0x9f0c34, 0);
                break;
            default:
                break;
            }
        };
        g_methods[0x4df8e0] = [](win32::WinApplication* app, x86::CPU& cpu) { waited(app, cpu, 0x4df8e0); };
        g_methods[0x4e7630] = [](win32::WinApplication* app, x86::CPU& cpu) { waited(app, cpu, 0x4e7630); };
        g_methods[0x4dfb70] = [](win32::WinApplication* app, x86::CPU& cpu) { waited(app, cpu, 0x4dfb70); };
        g_methods[0x4dfc50] = [](win32::WinApplication* app, x86::CPU& cpu) { waited(app, cpu, 0x4dfc50); };
        // exit does not come back.
        g_methods[0x4dfd04] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x4dfd04);
            cpu.terminate = true;
            // The registers sub_524bb0 had pushed stay on the stack above where it stopped.
            if (g_caseAddress == 0x524bb0)
                for (x86::reg32 at = kStack - 0xc; at < kStack; at += 4)
                    setWord32(at, 0);
        };
        specs[0x4f47c0].kind = Kind::Focus;
        specs[0x4f4780].kind = Kind::Focus;
        specs[0x4e11f0].kind = Kind::Focus;
        specs[0x515050].kind = Kind::Bool;
        specs[0x4f9120].kind = Kind::Bool;
        // What is tested against 0: the scripts' answers would all but never be.
        for (const x86::reg32 tested : { 0x533dde, 0x533e02, 0x52f661, 0x5324d2, 0x4df7a0, 0x4df930, 0x4e3e60, 0x4e3980, 0x4e4230,
                                         0x4e3600, 0x4dfcf0, 0x524970, 0x5098d0, 0x509a30, 0x4eb5e0, 0x4e3620, 0x50e150 })
            specs[tested].kind = Kind::Focus;
        g_methods[0x533e3e] = [](win32::WinApplication* app, x86::CPU& cpu) {
            nfs3hp::mock(app, cpu, 0x533e3e);
            const x86::reg32 pick = scripted(3);
            cpu.eax = pick == 0 ? x86::reg32(-1) : pick == 1 ? 0 : 1;
        };
        // The jmp of sub_4f00e0 is to it by name: the same on both sides.
        specs.erase(0x50c584);
        g_methods[0x50c584] = &nfs3hp::frontend::watbDispatchKey;
        // Tables that cost a page each: once.
        fillRandom(kColourTable, kColourTable + 0x8000);
        return true;
    }();
    (void)standInsAdded;
    const auto putString = [&](x86::reg32 at, const char* text) { std::memcpy(g_memory + at, text, std::strlen(text) + 1); };
    // The displays, their objects.
    setWord32(kDirectDrawObject, kComTable);
    setWord32(kPaletteObject, kComTable);
    setWord32(kSurfaceObject, kComTable);
    for (x86::reg32 slot = 0; slot < 0x90; slot += 4)
    {
        setWord32(kComTable + slot, 0x7e0600 + slot);
        setWord32(kNetTable + slot, 0x7e0900 + slot);
    }
    for (const x86::reg32 display : { x86::reg32(0x9f38d0), kOtherDisplay })
    {
        fillRandom(display + 0x30, display + 0x60);
        setWord32(display, random(4) == 0 ? 0 : kDirectDrawObject);
        setWord32(display + 8, 1 + random(6));
        setWord32(display + 0x10, random(5) == 0 ? 0 : kModes);
        setWord32(display + 0x14, 300 + random(400));
        setWord32(display + 0x18, 200 + random(400));
        setWord32(display + 0x1c, random(3) == 0 ? 16 : 8);
        setWord32(display + 0x24, random(3) == 0 ? 0 : kPaletteObject);
        g_memory[display + 0x430] = x86::reg8(random(3) == 0 ? 0 : random(4));
        g_memory[display + 0x431] = x86::reg8(random(4) == 0 ? 1 : 0);
        g_memory[display + 0x432] = x86::reg8(random(200));
        g_memory[display + 0x433] = 0;
        setWord32(display + 0x43c, random(2) == 0 ? 0 : kSurfaceObject);
        setWord32(display + 0x440, random(0));
        setWord32(display + 0x444, random(2) == 0 ? kSurfaceObject : kPaletteObject);
        setWord32(display + 0x454, random(0));
        g_memory[display + 0x45d] = x86::reg8(random(2));
        setWord32(display + 0x464, random(3) == 0 ? x86::reg32(-6000) : random(100));
        setWord32(display + 0x468, random(3) == 0 ? x86::reg32(-6000) : random(100));
    }
    fillRandom(kModes, kModes + 0x100);
    setWord32(0x564480, 0x77000000 + random(16));
    setWord32(0x56440c, random(2));
    static const x86::reg32 kDepths[] = { 8, 15, 16, 24, 32 };
    setWord32(0x56438c, kDepths[random(5)]);
    setWord32(0x568380, random(4) != 0);
    setWord32(0x564ee8, random(2));
    setWord32(0x564eec, random(1000));
    setWord32(0x564ee4, random(1000));
    // The font.
    for (x86::reg32 i = 0; i < 0x60; ++i)
    {
        fillRandom(kGlyphs + i * 11, kGlyphs + i * 11 + 11);
        const x86::reg32 c = random(3) != 0 ? 0x20 + i : random(0x100);
        g_memory[kGlyphs + i * 11] = x86::reg8(c);
        g_memory[kGlyphs + i * 11 + 1] = 0;
    }
    setWord32(0x567278, kGlyphs);
    setWord32(0x567268, random(20));
    setWord32(0x567270, random(4) == 0 ? 0x63 : 0x64);
    setWord32(0x5672a0, random(2) ? kKerningPairs : 0);
    setWord32(0x56729c, random(5));
    for (x86::reg32 i = 0; i < 40; ++i)
        g_memory[kKerningPairs + i] = x86::reg8(i % 8 == 1 || i % 8 == 3 ? 0 : i % 8 < 4 ? 0x41 + random(2) : random(0x100));
    static const char kCharacters[] = { 9, 10, 0x20, 0x41, 0x42, 0x41, 0x42, 0x7f, char(0x80), 0x43 };
    const x86::reg32 length = random(12);
    for (x86::reg32 i = 0; i < length; ++i)
        g_memory[kText + i] = x86::reg8(kCharacters[random(10)]);
    g_memory[kText + length] = 0;
    setWord32(0x5671f8, random(100));
    setWord32(0x5671fc, random(100));
    setWord32(0x567200, random(10));
    setWord32(0x567204, random(10));
    setWord32(0x567210, random(10));
    setWord32(0x567214, random(10));
    setWord32(0x567218, random(20));
    setWord32(0x56726c, random(100));
    setWord32(0x56727c, random(0));
    setWord32(0x567288, 0x7e0800);
    setWord32(0x56728c, random(2) ? 0x7e0804 : 0);
    setWord32(0x567290, random(2) ? 0x7e0808 : 0);
    setWord32(0x567294, 0x7e080c);
    setWord32(0x567298, random(2) ? 0x7e0810 : 0);
    setWord32(0x5651c8, random(2) ? kTabs : 0);
    setWord32(0x5651c4, random(5));
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(kTabs + i * 4, random(300));
    setWord32(0x5651cc, random(2));
    static const std::uint8_t kScreenBits[] = { 4, 8, 12, 15, 16, 24, 32, 8 };
    g_memory[0x565010] = kScreenBits[random(8)];
    setWord32(0x5650c0, random(2) ? kColourTable : 0);
    for (x86::reg32 i = 0; i < 0x100; ++i)
        g_memory[0x5650c4 + i] = x86::reg8(random(3));
    fillRandom(0xa0a814, 0xa0ac14);
    static const std::uint8_t kBits[] = { 0, 4, 8, 16, 24, 32 };
    for (x86::reg32 type = 0; type < 0x80; ++type)
        g_memory[0x566d54 + type] = kBits[random(6)];
    setWord32(0x567220, random(4));
    setWord32(0x567224, random(4));
    setWord32(0x5672a8, random(2));
    // Strings, an address, a host.
    putString(0xa18300, "12.3.4.5");
    putString(0xa18320, "");
    putString(0xa18330, "unfinished.");
    putString(0xa18350, "pack|member");
    fillRandom(0xa18200, 0xa18220);
    fillRandom(0xa18400, 0xa18500);
    g_memory[0xa18408] = x86::reg8(random(2) ? 2 : 3);
    g_memory[0xa18409] = 0;
    g_memory[0xa1840a] = x86::reg8(random(2) ? 4 : random(8));
    g_memory[0xa1840b] = 0;
    setWord32(0xa1840c, 0xa18480);
    setWord32(0xa18480, 0xa18490);
    setWord32(0xa18490, 0xa18200);
    // The low timer.
    setWord32(0x9f0c34, random(4) == 0 ? 0 : 0x77000100);
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(0x9f0c00 + i * 4, random(3) == 0 ? 0 : 0x7e0814);
    setWord32(0x5652ac, random(2) ? 0x77000200 : 0);
    setWord32(0x561290, random(2));
    // Sockets, the modem.
    setWord32(0x566de0, random(6));
    setWord32(0x9f37f0, 0xa22040);
    fillRandom(0xa22000, 0xa22048);
    g_memory[0xa22047] = x86::reg8(random(2));
    setWord32(0xa22018, random(3) == 0 ? x86::reg32(-1) : random(100));
    setWord32(0x9f4da4, random(2) ? random(0) : 0);
    setWord32(0x9f4da0, random(2) ? random(0) : 0);
    setWord32(0x9f4d84, random(2) ? 0xa22100 : 0);
    setWord32(0xa22100 + 0x38, random(2) ? x86::reg32(-1) : random(100));
    setWord32(0x9f4da8, random(2) ? random(0) : 0);
    setWord32(0x9f4d88, random(2) ? random(0) : 0);
    setWord32(0x9f4d94, random(2) ? 0xa22200 : 0);
    setWord32(0x9f4d80, random(5));
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(0xa22204 + i * 8, random(2) ? random(0) : 0);
    // The file requests.
    fillRandom(kThreads, kThreads + 0xd00);
    for (x86::reg32 i = 0; i < 32; ++i)
        setWord32(kThreads + i * 0x68, random(2));
    setWord32(0x9f4f60, random(5) == 0 ? 0 : kThreads);
    setWord32(0x9f4f64, random(3));
    setWord32(0x5671f0, random(8) == 0);
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(kThreads + i * 0x68 + 0x64, random(0x20));
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        const x86::reg32 request = kRequests + i * 0x40;
        fillRandom(request, request + 0x40);
        setWord32(request + 8, random(9) == 0 ? 11 + random(4) : random(11));
        setWord32(request + 0xc, random(4));
        g_memory[request + 0x11] = x86::reg8(random(0x40));
        setWord32(request + 0x20, random(2) ? 0x7e0818 : 0);
        setWord32(request + 0x2c, random(5) == 0 ? 0 : 0xa18900);
        g_memory[request + 0x24] = x86::reg8(random(0x40));
    }
    putString(0xa18900, "file");
    putString(0xa18914, "pack|file");
    // A net game.
    setWord32(0xa22300 + 8, kNetTable);
    setWord32(0xa22300 + 0xc, random(5) == 0 ? 0 : random(0));
    setWord32(0xa22300 + 0x14, random(2));
    setWord32(0xa22300 + 0xa0, random(2));
    setWord32(0xa22300 + 0xa8, random(2));
    setWord32(0x5689e4, random(3));
    setWord32(0x568a2c, random(3) == 0 ? 0 : 0x7e0950);
    setWord32(0x568a30, random(3) == 0 ? 0 : 0x7e0954);
    // The memory pools and blocks.
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord32(0x9ef5c0 + i * 4, kPool);
    setWord32(kPool + 8, random(4) == 0 ? 0 : kDumpBlocks);
    setWord32(kPool + 0x38, random(2) ? 0x77000300 : 0);
    static const x86::reg32 kGranules[] = { 8, 16, 0x40 };
    setWord32(kPool + 0x28, kGranules[random(3)]);
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        const x86::reg32 block = kDumpBlocks + i * 0x80;
        fillRandom(block, block + 0x80);
        setWord32(block + 4, random(4) == 0 ? 0 : random(0x20));
        setWord32(block + 8, i == 3 ? 0 : block + 0x80);
        // What a block of a loaded file ends in: its file's name and line.
        const x86::reg32 end = block + 0x10 + word32(block + 4);
        setWord32(end + 4, random(2) ? kNameText : 0);
    }
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(0x569020 + i * 4, kLabelText);
    putString(kLabelText, "label");
    putString(kNameText, "name");
    // The dump's formats: the world's memory may have made them anything.
    putString(0x550610, "-");
    putString(0x550614, "----\n");
    putString(0x550658, "");
    putString(0x55065c, "   SPACE ");
    putString(0x550668, " %s.%d");
    putString(0x550670, "\n");
    putString(0x550674, "\nTOTAL FREE MEMORY: %d ($%08x)\n");
    putString(0x568fb4, "\n====\n");
    putString(0x568ff7, "%-12s %08x %11d %11d %04x %04x %3s");
    // A block that may end in its tag and checksum, under its header.
    const x86::reg32 size = random(4) == 0 ? random(0xc) : 0xc + 4 * random(32);
    setWord32(0xa25ff0, size);
    fillRandom(0xa25000, 0xa25100);
    if (random(4) != 0 && size >= 0xc)
    {
        setWord32(0xa25000 + size - 0xc, word32(0x5506b4));
        setWord32(0xa25000 + size - 4, random(3));
    }
    setWord32(0x569030, random(2));
    setWord32(0x56902c, random(0x20));
    fillRandom(0xa24ff0, 0xa25000);
    setWord32(0xa24ff0 + 4, random(0x40));
    setWord32(0xa24ff0 + 8, 0xa25400);
    fillRandom(0xa25400, 0xa25410);
    setWord32(0xa25400 + 8, 0xa25800);
    // A block to resize.
    setWord32(0xa26000 + 4, random(0x40));
    g_memory[0xa26002] = x86::reg8(random(16));
    g_memory[0xa26003] = 0;
    const x86::reg32 next = 0xa26000 + 0x40 + 8 * random(100);
    setWord32(0xa26000 + 8, next);
    fillRandom(next, next + 0x10);
    setWord32(next + 8, next + 0x400);
    setWord32(0x564364, random(2));
    setWord32(0x564384, 640);
    setWord32(0x564388, 480);
    setWord32(0x565000, random(100));
    setWord32(0x565004, random(100));
    setWord32(0x565008, 100 + random(100));
    setWord32(0x56500c, 100 + random(100));
}
