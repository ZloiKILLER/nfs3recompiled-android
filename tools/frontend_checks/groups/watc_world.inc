// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    static constexpr x86::reg32 kLock = 0x7e0500, kGetter = 0x7e0510, kClear = 0x7e0520, kAsked = 0x7e0530, kRaise = 0x7e0540;
    static constexpr x86::reg32 kEndHook = 0x7e0550, kStartHook = 0x7e0560, kAbortHook = 0x7e0570, kHandler = 0x7e0580;
    static constexpr x86::reg32 kThreadFunction = 0x7e0590, kWinProc = 0x7e05a0, kTapiProcs = 0x7e0600;
    static constexpr x86::reg32 kData = 0xb00000, kTable = 0xb01000, kExtra = 0xb02000, kList = 0xb03000, kFlags = 0xb03800;
    static constexpr x86::reg32 kStrings = 0xb04000, kBlock = 0xb05000, kNodes = 0xb06000, kRows = 0xb07000, kColumns = 0xb07800;
    static constexpr x86::reg32 kPixels = 0xb08000, kFileInfo = 0xb0a000;
    static const bool watcAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto import = [&](x86::reg32 slot, std::uint8_t arguments, Kind kind) {
            specs[word32(slot)] = Spec{ 0, arguments, std::uint8_t(4 * arguments), ECX | EDX, kind };
        };
        import(0x534488, 1, Kind::Plain);   // CloseHandle
        import(0x534494, 4, Kind::Plain);   // CreateEvent
        import(0x5344a4, 6, Kind::Bool);    // CreateThread
        import(0x5344ac, 1, Kind::Plain);   // DeleteCriticalSection
        import(0x5344b8, 1, Kind::Plain);   // EnterCriticalSection
        import(0x5344c0, 1, Kind::Plain);   // ExitProcess
        import(0x5344c4, 1, Kind::Plain);   // ExitThread
        import(0x5344d8, 1, Kind::Plain);   // FreeEnvironmentStrings
        import(0x5344e0, 0, Kind::Plain);   // GetOEMCP
        import(0x5344e4, 2, Kind::Plain);   // GetCPInfo
        import(0x53450c, 0, Kind::Plain);   // GetCurrentThreadId
        import(0x534510, 0, Kind::Plain);   // GetCurrentProcess
        import(0x534534, 0, Kind::Plain);   // GetLastError
        import(0x534550, 0, Kind::Plain);   // GetACP
        import(0x534558, 2, Kind::Plain);   // GetProcAddress
        import(0x534584, 1, Kind::Plain);   // InitializeCriticalSection
        import(0x534588, 1, Kind::Plain);   // LeaveCriticalSection
        import(0x53458c, 1, Kind::Bool);    // LoadLibrary
        import(0x5345cc, 2, Kind::Bool);    // SetConsoleCtrlHandler
        import(0x5345dc, 2, Kind::Bool);    // SetEnvironmentVariable
        import(0x5345e8, 1, Kind::Plain);   // SetEvent
        import(0x5345f4, 1, Kind::Plain);   // SetLastError
        import(0x5345f8, 2, Kind::Plain);   // SetStdHandle
        import(0x53460c, 0, Kind::Plain);   // TlsAlloc
        import(0x534610, 1, Kind::Plain);   // TlsFree
        import(0x534614, 1, Kind::Plain);   // TlsGetValue
        import(0x534618, 2, Kind::Plain);   // TlsSetValue
        import(0x53461c, 1, Kind::Bool);    // UnhandledExceptionFilter
        import(0x534634, 2, Kind::Plain);   // WaitForSingleObject
        import(0x534640, 5, Kind::Plain);   // WriteFile
        g_methods[word32(0x53450c)] = &nfs3hp::watcStandInThreadId;
        g_methods[word32(0x534614)] = &nfs3hp::watcStandInTlsGet;
        g_methods[word32(0x53460c)] = &nfs3hp::watcStandInTlsAlloc;
        g_methods[word32(0x534558)] = &nfs3hp::watcStandInProc;
        g_methods[word32(0x5344e4)] = &nfs3hp::watcStandInCpInfo;

        specs[kLock] = Spec{ 0, 0, 0, 0, Kind::NoEax };
        specs[kGetter] = Spec{ 0, 0, 0, 0, Kind::Plain };
        specs[kClear] = Spec{ EAX, 0, 0, 0, Kind::NoEax };
        specs[kAsked] = Spec{ EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[kRaise] = Spec{ EAX, 0, 0, 0, Kind::NoEax };
        specs[kEndHook] = Spec{ 0, 0, 0, 0, Kind::Plain };
        specs[kStartHook] = Spec{ EAX, 0, 0, 0, Kind::Bool };
        specs[kAbortHook] = Spec{ 0, 0, 0, 0, Kind::Plain };
        specs[kHandler] = Spec{ EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0] = Spec{ EAX | EDX, 0, 0, 0, Kind::Plain };         // a handler of 0, called all the same
        specs[kThreadFunction] = Spec{ EAX, 0, 0, 0, Kind::Plain };
        specs[kWinProc] = Spec{ 0, 0, 0, ECX | EDX, Kind::Bool };
        for (std::uint8_t arguments = 1; arguments <= 9; ++arguments)
            specs[kTapiProcs + 0x10 * arguments] = Spec{ EAX, arguments, std::uint8_t(4 * arguments), ECX | EDX, Kind::Plain };
        g_methods[kGetter] = &nfs3hp::watcStandInGetter;
        g_methods[kAsked] = &nfs3hp::watcStandInAsked;
        g_methods[kRaise] = &nfs3hp::watcStandInRaise;

        specs[0x4f82b6].kind = Kind::SliderValue;   // strtod: a double on the x87
        specs[0x5243b1].kind = Kind::SliderValue;   // the report's answer on the x87
        specs[0x4e0640] = Spec{ EAX | EBX | EDX, 0, 0, 0, Kind::Memset };
        specs[0x4e0e30] = Spec{ EAX | EBX | EDX, 0, 0, 0, Kind::Strncpy };
        g_methods[0x401010] = &nfs3hp::watcStandInLog;
        g_methods[0x4f7900] = &nfs3hp::watcStandInAlloc<0x4f7900>;
        g_methods[0x5185c0] = &nfs3hp::watcStandInAlloc<0x5185c0>;
        g_methods[0x525df0] = &nfs3hp::watcStandInAlloc<0x525df0>;
        g_methods[0x51e7ec] = &nfs3hp::watcStandInMakeData;
        g_methods[0x50ead0] = &nfs3hp::watcStandInGetenv;
        g_methods[0x518050] = &nfs3hp::watcStandInMbslen;
        g_methods[0x4eede0] = &nfs3hp::watcStandInToupper;
        g_methods[0x4f0ae0] = &nfs3hp::watcStandInTolower;
        g_methods[0x52ede0] = &nfs3hp::watcStandInMbvtop;
        g_methods[0x52b5b0] = &nfs3hp::watcStandInMbclen;
        g_methods[0x5250c0] = &nfs3hp::watcStandInMbterm;
        g_methods[0x52e3a0] = &nfs3hp::watcStandInMbccmp;
        g_methods[0x525100] = &nfs3hp::watcStandInMbsinc;

        // Jumps by name in the generated code, to natives of other groups: the generated side of these
        // one-instruction functions as they are, but by address.
        g_methods[0x52b8d0] = &nfs3hp::watcGeneratedViewerOnWindow;
        g_methods[0x52bb20] = &nfs3hp::watcGeneratedViewerOnMono;
        g_methods[0x52b8f0] = &nfs3hp::watcGeneratedWindowEnd;
        g_methods[0x52e420] = &nfs3hp::frontend::winaTextStart;

        // The window's tables: a row and a column an offset each, inside the pixels.
        for (x86::reg32 i = 0; i < 256; ++i)
        {
            setWord32(kRows + 4 * i, 0x100 * (i & 0x7f));
            setWord32(kColumns + 4 * i, 2 * (i & 0x7f));
        }
        return true;
    }();
    (void)watcAdded;

    // The runtime's and the library's strings, which the world's fill of what is watched has taken.
    std::memcpy(g_memory + 0x551300, g_image.data() + (0x551300 - 0x534000), 0xf00);

    const auto someHandler = [&]() -> x86::reg32 {
        const x86::reg32 pick = random(6);
        return pick < 4 ? pick : kHandler;
    };
    // The hooks.
    for (const x86::reg32 hook : { 0x567790u, 0x567794u, 0x567798u, 0x56779cu, 0x5677a4u, 0x5677a8u })
        setWord32(hook, kLock);
    setWord32(0x567764, kGetter);
    setWord32(0x56acd0, kClear);
    setWord32(0x56acd4, kAsked);
    setWord32(0x56acd8, random(4) == 0 ? 0 : kRaise);
    setWord32(0x56b1c4, kEndHook);
    setWord32(0x56b1c8, kStartHook);
    setWord32(0x56b1cc, kAbortHook);
    std::memcpy(g_memory + 0x567853, "\x00\x00\x00\x00", 4);
    setWord32(0x567853, kEndHook);

    // A thread's data.
    fillRandom(kData, kData + 0x200);
    setWord32(kData + 0xf0, random(0x40));
    setWord32(kData + 0xda, random(3));
    setWord32(kData + 0xde, random(2) ? random(0) : 0);
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        setWord32(kData + 0x58 + 8 * i, someHandler());
        setWord32(kData + 0x5c + 8 * i, random(4));
        setWord32(0x56b0bc + 8 * i, someHandler());
        setWord32(0x56b0c0 + 8 * i, random(4));
    }
    setWord32(8, kData);
    setWord32(0xc, 0);
    setWord32(0x9f642c, kData);
    setWord32(0x9f6420, random(2));
    setWord32(0x56af70, 0xf4 + 4 * random(16));
    setWord32(0x567760, random(3) == 0 ? 0xffffffffu : random(8));
    g_memory[0x567841] = 0;
    g_memory[0x567842] = x86::reg8(random(2) ? 0x80 : 0x10);
    g_memory[0x56783f] = x86::reg8(random(8));
    g_memory[0x56b124] = x86::reg8(random(2));
    g_memory[0xa0ba94] = x86::reg8(random(2));

    // The handle table and the locks.
    setWord32(0x56acbc, kTable);
    setWord32(0x56acc0, random(16));
    fillRandom(kTable, kTable + 0x40);
    setWord32(0xa0ba80, random(6) == 0 ? 0 : 0x3e + random(4));
    setWord32(0xa0ba84, random(4));
    setWord32(0xa0ba88, kExtra);
    fillRandom(kExtra, kExtra + 0x10);
    for (const x86::reg32 lock : { 0xa0b320u, 0xa0b330u, 0xa0b440u, 0xa0ba50u, 0xa0ba60u, 0xa0ba70u, 0xa0b340u, 0xa0b350u, 0xa0b360u })
    {
        setWord32(lock, random(0));
        setWord32(lock + 4, random(2));
        setWord32(lock + 8, random(4));
        setWord32(lock + 0xc, random(3));
    }
    setWord32(0xa0ba54, 1);  // the lock of locks is made first

    // Threads' list.
    const x86::reg32 nodes = random(4);
    setWord32(0xa18250, nodes ? kNodes : 0);
    for (x86::reg32 i = 0; i < nodes; ++i)
    {
        const x86::reg32 node = kNodes + 0x10 * i;
        setWord32(node, i + 1 < nodes ? node + 0x10 : 0);
        setWord32(node + 4, random(3));
        setWord32(node + 8, kData);
        setWord32(node + 0xc, random(2));
    }

    // The environment.
    static const char* const kVariables[] = { "PATH=x", "ab=1", "NAME=val", "Z=" };
    x86::reg32 at = kStrings;
    const x86::reg32 variables = random(5);
    for (x86::reg32 i = 0; i < variables; ++i)
    {
        setWord32(kList + 4 * i, at);
        const char* const text = kVariables[random(4)];
        std::strcpy(reinterpret_cast<char*>(g_memory) + at, text);
        at += x86::reg32(std::strlen(text)) + 1;
        g_memory[kFlags + i] = x86::reg8(random(2));
    }
    setWord32(kList + 4 * variables, 0);
    setWord32(0xa0b154, random(4) == 0 ? 0 : kList);
    setWord32(0xa0b150, random(3) == 0 ? 0 : kFlags);
    setWord32(0xa0b158, random(2));
    setWord32(0xa18230, random(2) ? kStrings : 0);
    std::memcpy(g_memory + kBlock, random(4) == 0 ? "\0\0\0\0\0\0\0" : "A=1\0BB=2\0", 10);
    setWord32(0x567839, kBlock);
    // C_FILE_INFO: fields of a few hex digits, each ended.
    x86::reg32 written = 0;
    for (x86::reg32 entries = random(3); entries > 0; --entries)
        written += std::snprintf(reinterpret_cast<char*>(g_memory) + kFileInfo + written, 32, "%x:%x:%x*", unsigned(random(16)), unsigned(random(0x10000)), unsigned(random(16)));
    g_memory[kFileInfo + written] = 0;

    // The window.
    setWord32(0x565000, random(8));
    setWord32(0x565004, random(8));
    setWord32(0x565008, 20 + random(40));
    setWord32(0x56500c, 20 + random(40));
    static constexpr x86::reg8 kDepths[] = { 0x10, 0x10, 0x10, 8, 4, 0xf, 0x18, 0x20, 3, 5, 0x12, 0x19, 0x21 };
    g_memory[0x565010] = kDepths[random(std::size(kDepths))];
    g_memory[0x565012] = x86::reg8(random(4) == 0);
    setWord32(0x565014, kPixels);
    setWord32(0x565020, kRows);
    setWord32(0x565024, kColumns);
    fillRandom(kPixels, kPixels + 0x100);
    setWord32(0x564ff8, random(6) == 0 ? x86::reg32(0) - random(100) : random(800));
    setWord32(0x564ffc, random(6) == 0 ? x86::reg32(0) - random(100) : random(600));
    setWord32(0x564384, random(800));
    setWord32(0x5671f8, random(0));
    setWord32(0x5671fc, random(0));
    setWord32(0x567294, random(3) == 0 ? 0x50b580 : 0);

    // TAPI.
    const x86::reg32 tapi = random(6);
    setWord32(0x56d12c, tapi == 0 ? 0 : tapi == 1 ? 0xffffffffu : 0x7e0700);
}
