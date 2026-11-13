// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    using namespace nfs3hp;
    static const bool netcStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const auto import = [&](x86::reg32 slot, std::uint8_t arguments, Kind kind) {
            specs[word32(slot)] = Spec{ 0, arguments, std::uint8_t(4 * arguments), ECX | EDX, kind };
        };
        import(0x5344b8, 1, Kind::Plain);   // EnterCriticalSection
        import(0x534588, 1, Kind::Plain);   // LeaveCriticalSection
        import(0x53458c, 1, Kind::Bool);    // LoadLibraryA
        import(0x534558, 2, Kind::Bool);    // GetProcAddress
        import(0x534640, 5, Kind::Bool);    // WriteFile
        import(0x5345b0, 5, Kind::Bool);    // ReadFile
        import(0x534534, 0, Kind::Plain);   // GetLastError
        import(0x534554, 4, Kind::Bool);    // GetOverlappedResult
        import(0x5345a8, 2, Kind::Plain);   // PurgeComm
        import(0x534488, 1, Kind::Plain);   // CloseHandle
        import(0x5345f4, 1, Kind::Plain);   // SetLastError
        import(0x534494, 4, Kind::Plain);   // CreateEventA
        import(0x5345c8, 2, Kind::Plain);   // SetCommTimeouts
        import(0x534498, 7, Kind::Plain);   // CreateFileA
        import(0x5344f0, 2, Kind::Bool);    // GetCommState
        import(0x5345c4, 2, Kind::Plain);   // SetCommState
        const std::uint8_t allFour = EAX | EBX | ECX | EDX;
        specs[kNetcFnSend] = Spec{ allFour, 1, 4, EDX, Kind::Plain };
        specs[kNetcFnBool] = Spec{ allFour, 0, 0, EDX, Kind::Bool };
        specs[kNetcFnCallback] = Spec{ allFour, 0, 0, EDX, Kind::Bool };
        specs[kNetcFnPlain] = Spec{ allFour, 0, 0, EDX, Kind::Plain };
        for (const x86::reg32 row : { 0x4df930u, 0x4f5040u, 0x512fd0u, 0x4dfb40u, 0x5126f0u, 0x50e150u })
            specs[row].kind = Kind::Bool;

        // Allocated memory: from the pool, now and then none.
        g_methods[0x4e1620] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x4e1620);
            cpu.eax = scripted(6) == 0 ? 0 : kNetcPool;
        };
        specs[kNetcFnAddress] = Spec{ allFour, 0, 0, EDX, Kind::Bool };
        // A network address handed to the transport, the player's or the game's.
        g_methods[kNetcFnAddress] = [](win32::WinApplication* a, x86::CPU& cpu) {
            netcLogBytes(cpu.edx, 0x14);
            nfs3hp::mock(a, cpu, kNetcFnAddress);
        };
        // What the transport sends: to whom, and the message of the length it is told.
        g_methods[kNetcFnSend] = [](win32::WinApplication* a, x86::CPU& cpu) {
            netcLogBytes(cpu.edx, 0x14);
            netcLogBytes(cpu.ecx, word32(cpu.esp + 4));
            nfs3hp::mock(a, cpu, kNetcFnSend);
        };
        g_methods[0x4f5040] = [](win32::WinApplication* a, x86::CPU& cpu) {  // two players compared
            netcLogBytes(cpu.edx, 0x14);
            netcLogBytes(cpu.ebx, 0x14);
            nfs3hp::mock(a, cpu, 0x4f5040);
        };
        g_methods[0x512f60] = [](win32::WinApplication* a, x86::CPU& cpu) {  // an address made: it is written
            const x86::reg32 out = cpu.edx;
            nfs3hp::mock(a, cpu, 0x512f60);
            for (x86::reg32 i = 0; i < 0x14; ++i)
                g_memory[out + i] = x86::reg8(scripted(256));
        };
        g_methods[0x5133a0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // an answer for the request
            netcLogBytes(cpu.eax, 0x50);
            netcLogBytes(cpu.edx, cpu.ecx);
            nfs3hp::mock(a, cpu, 0x5133a0);
        };
        g_methods[0x5126f0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // data delivered
            netcLogBytes(cpu.edx, cpu.ebx);
            nfs3hp::mock(a, cpu, 0x5126f0);
        };
        g_methods[0x4ea4f0] = [](win32::WinApplication* a, x86::CPU& cpu) {  // memmove
            netcLogBytes(cpu.eax, cpu.ebx);
            nfs3hp::mock(a, cpu, 0x4ea4f0);
        };
        g_methods[0x512180] = [](win32::WinApplication* a, x86::CPU& cpu) {  // the trace
            netcLogBytes(cpu.edx, 4);
            netcLogBytes(cpu.ebx, cpu.ecx);
            nfs3hp::mock(a, cpu, 0x512180);
        };
        g_methods[0x501050] = [](win32::WinApplication* a, x86::CPU& cpu) {  // a line for the modem's log
            const x86::reg32 format = word32(cpu.esp + 12);
            if (format == 0x550adc)
            {
                netcLogString(word32(cpu.esp + 16));
                netcLogString(word32(cpu.esp + 20));
            }
            else if (format == 0x550b08)
            {
                netcLogString(word32(cpu.esp + 16));
            }
            nfs3hp::mock(a, cpu, 0x501050);
        };
        g_methods[word32(0x534640)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // WriteFile
            netcLogBytes(word32(cpu.esp + 8), word32(cpu.esp + 12));
            nfs3hp::mock(a, cpu, word32(0x534640));
        };
        g_methods[word32(0x5345b0)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // ReadFile: it is written
            const x86::reg32 out = word32(cpu.esp + 8), room = word32(cpu.esp + 12), got = word32(cpu.esp + 16);
            nfs3hp::mock(a, cpu, word32(0x5345b0));
            const x86::reg32 count = scripted(room < 16 ? room + 1 : 16);
            for (x86::reg32 i = 0; i < count; ++i)
                g_memory[out + i] = x86::reg8(scripted(256));
            setWord32(got, count);
        };
        g_methods[word32(0x5344f0)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // GetCommState: it is written
            const x86::reg32 out = word32(cpu.esp + 8);
            nfs3hp::mock(a, cpu, word32(0x5344f0));
            for (x86::reg32 i = 0; i < 0x1c; ++i)
                g_memory[out + i] = x86::reg8(scripted(256));
        };
        g_methods[0x5132a0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            netcLogBytes(cpu.edx, 0x14);
            nfs3hp::mock(a, cpu, 0x5132a0);
            cpu.eax = scripted(3) == 0 ? 0 : kNetcGame + 0x88 + 0x64 * scripted(32);
        };
        // A request: what it was handed recorded, its answer in the record it was handed.
        g_methods[0x513190] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 reply = cpu.edx;
            netcLogBytes(reply, 0x50);
            netcLogBytes(cpu.ecx, word32(cpu.esp + 4));
            nfs3hp::mock(a, cpu, 0x513190);
            setWord32(reply + 4, scripted(3) != 0);
            setWord32(reply + 8, scripted(3) == 0 ? 0 : scripted(0));
            setWord32(reply + 0x18, scripted(4) != 0);
        };
        for (const x86::reg32 row : { 0x4df7a0u, 0x4f2a78u })
            specs[row].kind = Kind::Bool;
        specs[kNetcFnRoom] = Spec{ allFour, 0, 0, EDX, Kind::Plain };
        specs[kNetcFnRead] = Spec{ allFour, 0, 0, EDX, Kind::Plain };
        specs[kNetcFnWrite] = Spec{ allFour, 0, 0, EDX, Kind::Plain };
        g_methods[kNetcFnRoom] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, kNetcFnRoom);
            cpu.eax = scripted(2) ? 0x400 : scripted(8);
        };
        g_methods[kNetcFnRead] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 room = cpu.ebx, out = cpu.edx;
            nfs3hp::mock(a, cpu, kNetcFnRead);
            cpu.eax = scripted(room < 16 ? room + 1 : 16);
            for (x86::reg32 i = 0; i < cpu.eax; ++i)
                g_memory[out + i] = x86::reg8(scripted(3) == 0 ? 0xfe : scripted(256));
        };
        // A frame written: what it says recorded.
        g_methods[kNetcFnWrite] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 size = cpu.ebx;
            for (x86::reg32 i = 0; i < size && i < 0x120; ++i)
                g_log.push_back(g_memory[cpu.edx + i]);
            nfs3hp::mock(a, cpu, kNetcFnWrite);
            cpu.eax = scripted(3) ? size : scripted(8);
        };
        g_methods[0x4f4780] = [](win32::WinApplication* a, x86::CPU& cpu) {
            netcLogBytes(cpu.eax, cpu.edx);
            nfs3hp::mock(a, cpu, 0x4f4780);
            cpu.eax = scripted(3) ? 0x1234 : scripted(0);
        };
        // The queues' packets, now and then none.
        g_methods[0x50df60] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50df60);
            cpu.eax = scripted(3) == 0 ? 0 : kNetcPackets + 0x200 * scripted(4);
        };
        g_methods[0x50e470] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50e470);
            cpu.eax = scripted(3) == 0 ? 0 : kNetcPackets + 0x200 * scripted(4);
        };
        g_methods[0x50e500] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50e500);
            cpu.eax = scripted(3) == 0 ? 0 : kNetcPackets + 0x200 * scripted(4);
        };
        g_methods[0x50e800] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50e800);
            cpu.eax = scripted(3) == 0 ? 0 : kNetcPackets + 0x200 * scripted(4);
        };
        g_methods[0x50e2d0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50e2d0);
            cpu.eax = scripted(4) == 0 ? 0xffffffffu : scripted(0x100);
        };
        // The clocks go on where a function waits, sometimes past any time asked for, and the
        // ports' thread closes what it was told to.
        g_methods[0x4df8e0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x4df8e0);
            const x86::reg32 step = scripted(3) == 0 ? 0x20000000 : 1;
            setWord32(0x79c1a0, word32(0x79c1a0) + step);
            setWord32(0x79c1a4, word32(0x79c1a4) + step);
            if (scripted(3) == 0)
                for (x86::reg32 i = 0; i < 16; ++i)
                    setWord32(0xa0b1a0 + 4 * i, 0);
        };
        g_methods[0x4e0820] = [](win32::WinApplication* a, x86::CPU& cpu) {  // strstr
            const x86::reg32 from = cpu.eax;
            const std::string haystack = guestString(from, 200), needle = guestString(cpu.edx, 40);
            netcLogString(from);
            netcLogString(cpu.edx);
            nfs3hp::mock(a, cpu, 0x4e0820);
            const auto found = needle.empty() ? std::string::npos : haystack.find(needle);
            cpu.eax = found == std::string::npos ? 0 : from + x86::reg32(found);
            setWord32(0x79c1a4, word32(0x79c1a4) + (scripted(3) == 0 ? 0x20000000 : 1));
        };
        g_methods[0x4f0430] = [](win32::WinApplication* a, x86::CPU& cpu) {  // atoi
            const std::string text = guestString(cpu.eax, 40);
            netcLogString(cpu.eax);
            nfs3hp::mock(a, cpu, 0x4f0430);
            cpu.eax = x86::reg32(std::atoi(text.c_str()));
        };
        // The thread's wait: what it waits for recorded; the last port goes now and then.
        g_methods[0x4dfbf0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            for (x86::reg32 i = 0; i < cpu.eax && i < 0x21; ++i)
                g_log.push_back(word32(cpu.edx + 4 * i));
            nfs3hp::mock(a, cpu, 0x4dfbf0);
            if (scripted(2) == 0)
                setWord32(0x56abc0, 0);
        };
        g_methods[word32(0x534534)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // GetLastError
            nfs3hp::mock(a, cpu, word32(0x534534));
            const x86::reg32 pick = scripted(3);
            cpu.eax = pick == 0 ? 0 : pick == 1 ? 0x3e5 : 5;
        };
        g_methods[word32(0x534498)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // CreateFileA
            netcLogString(word32(cpu.esp + 4));
            nfs3hp::mock(a, cpu, word32(0x534498));
            cpu.eax = scripted(3) == 0 ? 0xffffffffu : 0x777;
        };
        g_methods[word32(0x534554)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // GetOverlappedResult
            const x86::reg32 got = word32(cpu.esp + 12);
            nfs3hp::mock(a, cpu, word32(0x534554));
            setWord32(got, scripted(3) == 0 ? 0 : scripted(16));
        };
        g_methods[word32(0x5345c8)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // SetCommTimeouts
            for (x86::reg32 i = 0; i < 5; ++i)
                g_log.push_back(word32(word32(cpu.esp + 8) + 4 * i));
            nfs3hp::mock(a, cpu, word32(0x5345c8));
        };
        g_methods[word32(0x5345c4)] = [](win32::WinApplication* a, x86::CPU& cpu) {  // SetCommState
            for (x86::reg32 i = 0; i < 7; ++i)
                g_log.push_back(word32(word32(cpu.esp + 8) + 4 * i));
            nfs3hp::mock(a, cpu, word32(0x5345c4));
        };
        return true;
    }();
    (void)netcStandIns;

    // The gathering.
    setWord32(0x566de0, random(3) == 0 ? random(7) : 0);
    setWord32(0x566dec, random(3) == 0 ? random(7) : 0);
    setWord32(0x5643d8, random(2) ? 60 : random(0));
    setWord32(kNetcConn + 4, kNetcConnInfo);
    setWord32(kNetcConnInfo, random(4) == 0);
    setWord32(kNetcConn + 8, kNetcTable);
    setWord32(kNetcConn + 0xc, random(3) ? random(0) : 0);
    setWord32(kNetcConn + 0x14, random(3) ? kNetcGame : 0);
    setWord32(kNetcConn + 0x18, random(2) ? 0 : kNetcOut);
    setWord32(kNetcConn + 0xa0, random(4));
    setWord32(kNetcConn + 0xa4, random(2) ? random(3) : 0);
    setWord32(kNetcConn + 0xa8, random(3) == 0);
    setWord32(kNetcGame + 8, random(4) == 0);
    setWord32(kNetcGame + 0x50, random(4) == 0 ? 0 : random(0));
    setWord32(kNetcGame + 0x84, 1 + random(33));
    for (x86::reg32 slot = 0; slot < 32; ++slot)
        setWord32(kNetcGame + 0xe4 + 0x64 * slot, random(3) ? 1 + random(9) : 0);
    for (x86::reg32 at = 0; at < 0x60; at += 4)
        setWord32(kNetcTable + at, kNetcFnBool);
    for (const x86::reg32 at : { 0x28u, 0x2cu, 0x30u })
        setWord32(kNetcTable + at, kNetcFnAddress);
    setWord32(kNetcTable + 0x54, kNetcFnSend);
    setWord32(kNetcInfo + 0x30, random(4) == 0 ? 0 : random(4) == 0 ? 1 : random(4) == 0 ? 33 + random(3) : 2 + random(31));
    std::memcpy(g_memory + kNetcName, "a game", 7);
    setWord32(kNetcPool + 0x84, 1 + random(33));

    // A stream's functions.
    setWord32(kNetcOps + 0x00, kNetcFnBool);
    setWord32(kNetcOps + 0x04, kNetcFnPlain);
    setWord32(kNetcOps + 0x08, kNetcFnBool);
    setWord32(kNetcOps + 0x0c, kNetcFnRoom);
    setWord32(kNetcOps + 0x10, kNetcFnBool);
    setWord32(kNetcOps + 0x14, kNetcFnRead);
    setWord32(kNetcOps + 0x18, kNetcFnWrite);
    setWord32(kNetcOps + 0x1c, random(2) ? 0 : random(0x120) | x86::reg32(random(3) == 0 ? random(0x40) << 16 : 0)
                                                | x86::reg32(random(3) == 0) << 24);
    // A connection; which layer it has, its case says.
    setWord32(kNetcLink + 0x14, kNetcFnBool);
    setWord32(kNetcLink + 0x18, kNetcFnPlain);
    setWord32(kNetcLink + 0x1c, kNetcFnPlain);
    setWord32(kNetcLink + 0x24, random(3) == 0 ? 0 : 1 + random(3));
    for (const x86::reg32 at : { 0x2au, 0x2cu, 0x2eu })
    {
        g_memory[kNetcLink + at] = x86::reg8(random(2) ? 0 : random(256));
        g_memory[kNetcLink + at + 1] = x86::reg8(random(2) ? 0 : random(256));
    }
    setWord32(0x566df0, random(4) == 0 ? 0 : 100 + random(100));
    setWord32(0x5643f4, random(4));
    // The layers.
    setWord32(kNetcPlain + 0x00, kNetcLink);
    setWord32(kNetcPlain + 0x04, kNetcOps);
    setWord32(kNetcPlain + 0x08, random(2) ? 0x20 : 0x140);
    g_memory[kNetcPlain + 0x0c] = x86::reg8(random(2));
    setWord32(kNetcPlain + 0x14, kNetcPlainBuffer);
    setWord32(kNetcPlain + 0x18, 0x200);
    setWord32(kNetcPlain + 0x1c, random(3) == 0 ? 0x200 : random(0x40));
    setWord32(kNetcPlain + 0x20, x86::reg32(random(4)) - 1);
    setWord32(kNetcRely + 0x00, kNetcLink);
    setWord32(kNetcRely + 0x04, kNetcOps);
    setWord32(kNetcRely + 0x08, random(2) ? 0x20 : 0x140);
    g_memory[kNetcRely + 0x0c] = x86::reg8(random(2));
    g_memory[kNetcRely + 0x0e] = x86::reg8(random(2));
    setWord32(kNetcRely + 0x18, kNetcPackets + 0x1800);
    setWord32(kNetcRely + 0x1c, kNetcRelyBuffer);
    setWord32(kNetcRely + 0x20, 0x200);
    setWord32(kNetcRely + 0x24, random(3) == 0 ? 0x200 : random(0x40));
    setWord32(kNetcRely + 0x28, random(2));
    setWord32(kNetcRely + 0x2c, x86::reg32(random(4)) - 1);
    setWord32(kNetcRely + 0x30, random(3) == 0 ? 0 : random(0x100));
    setWord32(kNetcRely + 0x34, random(0x100));
    setWord32(kNetcRely + 0x70, random(0x100));
    setWord32(kNetcRely + 0x74, random(0x100));
    // Frames in what the layers have read: some whole, some not.
    for (const x86::reg32 header : { 4u, 6u })
    {
        x86::reg32 at = header == 4 ? kNetcPlainBuffer : kNetcRelyBuffer;
        for (x86::reg32 frame = 0; frame < 6; ++frame)
        {
            at += random(3);
            const x86::reg32 kind = random(4);
            const x86::reg32 length = kind == 0 ? 0 : kind == 1 ? 1 + random(12) : kind == 2 ? 0xff : random(0x100);
            g_memory[at] = 0xfe;
            g_memory[at + 1] = x86::reg8(length);
            if (header == 6)
            {
                g_memory[at + 2] = x86::reg8(random(4) == 0 ? 0xff : random(0x100));
                g_memory[at + 3] = x86::reg8(random(3) == 0 ? random(0x100) : random(0x80));
            }
            g_memory[at + header - 2] = 0x12;
            g_memory[at + header - 1] = 0x34;
            at += header + (length < 0x20 ? length : 0x20);
        }
    }
    // The packets.
    for (x86::reg32 p = 0; p < 16; ++p)
    {
        const x86::reg32 packet = kNetcPackets + 0x200 * p;
        setWord32(packet + 4, random(0x100));
        setWord32(packet + 8, x86::reg32(random(4)) - 1);
        g_memory[packet + 0xd] = x86::reg8(random(3) == 0 ? random(0x40) : random(8));
    }
    // The ports.
    setWord32(0x56abc0, random(3));
    setWord32(0x56abc4, random(0));
    setWord32(0x56abc8, random(3) == 0 ? 0 : random(0));
    setWord32(0x79c1a0, random(100));
    setWord32(0x79c1a4, random(100));
    setWord32(0xa0b19c, 0);
    for (x86::reg32 i = 16; i < 20; ++i)
        setWord32(0xa0b1a0 + 4 * i, 0);
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 record = kNetcPorts + 0x450 * i;
        setWord32(0xa0b1a0 + 4 * i, random(3) == 0 ? 0 : record);
        setWord32(record, random(6));
        setWord32(record + 4, random(0));
        setWord32(record + 8, random(2));
        setWord32(record + 0xc, random(0x200));
        setWord32(record + 0x10, random(0x201));
        setWord32(record + 0x14, random(16));
        setWord32(record + 0x28, random(0));
        setWord32(record + 0x22c, random(2));
        setWord32(record + 0x230, random(0x200));
        setWord32(record + 0x234, random(0x201));
        setWord32(record + 0x238, random(16));
        setWord32(record + 0x24c, random(0));
        for (x86::reg32 at = 0; at < 0x200; ++at)
            g_memory[record + 0x2c + at] = x86::reg8("OK 12\r"[at % 6]);
    }
    // The port's name, its format among the data the world has made up.
    std::memcpy(g_memory + 0x550bf4, "\\\\.\\COM%d", 10);
    // The modem.
    setWord32(0x564454, random(2));
    setWord32(0x564458, random(4) == 0);
    setWord32(0x56aab8, random(2) ? 0 : 5);
    setWord32(0x56aabc, kNetcName);
    setWord32(0x56abb8, random(3));
    setWord32(0x56abbc, random(2));
    std::memcpy(g_memory + kNetcName + 0x10, "OK\0NO\0007\0", 10);
    for (x86::reg32 row = 0; row < 12; ++row)
    {
        setWord32(0x56aac0 + 12 * row, kNetcName + 0x10 + (random(2) ? 0 : 3));
        setWord32(0x56aac4 + 12 * row, kNetcName + 0x10 + (random(2) ? 0 : 3));
        setWord32(0x56aac8 + 12 * row, kNetcName + 0x16);
    }
    setWord32(0x56aab4, kNetcName + 0x13);
    for (x86::reg32 at = 0; at < 17; ++at)
        setWord32(0x56abcc + 4 * at, 9600 + random(100));
    std::memset(g_memory + 0xa0b160, 0, 0x40);
}
