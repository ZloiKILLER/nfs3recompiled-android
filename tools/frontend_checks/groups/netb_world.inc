// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kSession = 0xb00000, kNet = 0xb01000, kFunctions = 0xb02000, kGame = 0xb03000;
    constexpr x86::reg32 kMessage = 0xb04000;
    constexpr x86::reg32 kSendFn = 0x7e0800, kAnswerFn = 0x7e0804, kCallbackFn = 0x7e0808;
    static const bool netbStandIns = [&] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // Imports: what is in the table in the file; stdcall, ecx and edx not kept.
        struct ImportRow { x86::reg32 slot; std::uint8_t words; Kind kind; };
        const ImportRow imports[] = {
            { 0x5344b8, 1, Kind::Plain }, { 0x534588, 1, Kind::Plain }, { 0x534568, 0, Kind::Plain },
            { 0x534758, 2, Kind::Plain }, { 0x534750, 1, Kind::Plain }, { 0x53474c, 4, Kind::Bool },
            { 0x5347a4, 1, Kind::Plain }, { 0x53472c, 1, Kind::Plain }, { 0x534770, 4, Kind::Plain },
            { 0x5344ec, 2, Kind::Bool }, { 0x534488, 1, Kind::Plain },
        };
        for (const ImportRow& row : imports)
            specs[word32(row.slot)] = { 0, row.words, std::uint8_t(row.words * 4), ECX | EDX, row.kind };
        // What the indirect calls of these functions reach.
        specs[kSendFn] = { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Bool };
        specs[kAnswerFn] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        specs[kCallbackFn] = { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Bool };
        specs[0x7e0814] = { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain };  // a layer's unpacker
        // Their answers read: both ways.
        specs[0x4f5040] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Bool };
        specs[0x4eb390] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Bool };
        specs[0x7e080c] = { EAX | EDX, 0, 0, EDX, Kind::Focus };   // a queue's priority function
        specs[0x4eb2e0] = { 0, 0, 0, 0, Kind::Bool };              // a lock, 0 now and then
        specs[0x50e1d0] = { EAX, 0, 0, 0, Kind::Focus };           // a queue's length
        // Allocated memory, a layer found: somewhere that is there.
        g_methods[0x4e1620] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x4e1620);
            // Each one apart from the others of the case: a counter in guest memory, put back for the other side.
            const x86::reg32 taken = word32(0xb07ff0);
            setWord32(0xb07ff0, taken + 1);
            cpu.eax = 0xb0c000 + 0x400 * (taken % 16);
        };
        g_methods[0x50e470] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x50e470);
            cpu.eax = 0xb07c00;
        };
        specs[0x7e0810] = { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain };  // keeps every register, as those read on with them
        specs[0x4f5ac0] = { EAX | EBX | EDX, 0, 0, EDX, Kind::Bool };
        for (const x86::reg32 tested : { 0x4df930u, 0x4f5c10u, 0x50e150u, 0x533eb0u, 0x533e1au, 0x533e44u, 0x533e9eu })
            specs[tested].kind = Kind::Bool;
        g_methods[0x533e38] = &nfs3hp::netbLastError;
        g_methods[0x533e2c] = &nfs3hp::netbRecv;
        g_methods[0x533eaa] = &nfs3hp::netbThreeWays<0x533eaa>;
        g_methods[0x533e20] = &nfs3hp::netbSelect;
        g_methods[0x533e44] = &nfs3hp::netbBufferCall<0x533e44, 2, 3>;  // bind
        g_methods[0x533e0e] = &nfs3hp::netbBufferCall<0x533e0e, 2, 3>;  // connect
        g_methods[0x533e56] = &nfs3hp::netbBufferCall<0x533e56, 4, 5>;  // setsockopt
        g_methods[0x533e50] = [](win32::WinApplication* a, x86::CPU& cpu) {  // ioctlsocket
            nfs3hp::netbRecordLength(a, cpu, 0x533e50);
        };
        g_methods[0x533e14] = [](win32::WinApplication* a, x86::CPU& cpu) {  // accept: the length going in
            nfs3hp::netbRecord(word32(cpu.esp + 12), 4);
            nfs3hp::netbThreeWays<0x533e14>(a, cpu);
        };
        g_methods[0x533e3e] = [](win32::WinApplication* a, x86::CPU& cpu) {  // getsockname: the length going in
            nfs3hp::netbRecordLength(a, cpu, 0x533e3e);
        };
        g_methods[0x533e26] = &nfs3hp::netbSend;
        g_methods[0x4e0716] = &nfs3hp::netbFillByte;
        g_methods[0x401010] = &nfs3hp::netbError;
        g_methods[0x51ba50] = &nfs3hp::netbPortSend;
        g_methods[0x51b530] = &nfs3hp::netbPortReceive;
        g_methods[0x4f5900] = [](win32::WinApplication* a, x86::CPU& cpu) {  // an address made from a port
            nfs3hp::netbRecord(cpu.edx, 14);
            nfs3hp::mock(a, cpu, 0x4f5900);
        };
        g_methods[0x4f5a30] = [](win32::WinApplication* a, x86::CPU& cpu) {  // a game found: its address and name
            nfs3hp::netbRecord(cpu.edx, 14);
            nfs3hp::netbRecord(cpu.ebx, 0x30);
            nfs3hp::mock(a, cpu, 0x4f5a30);
        };
        // A layer's unpacker: the pieces, {length, address} each.
        g_methods[0x7e0814] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::netbRecord(cpu.ecx, 8 * (cpu.ebx < 8 ? cpu.ebx : 8));
            nfs3hp::mock(a, cpu, 0x7e0814);
        };
        for (const x86::reg32 tested : { 0x5310beu, 0x53138fu, 0x5324d2u, 0x530a4eu, 0x53083du, 0x530747u, 0x531f31u, 0x531439u, 0x51b030u,
                                         0x51a930u, 0x4f9920u, 0x533e3eu, 0x4df7a0u })
            specs[tested].kind = Kind::Bool;
        // TAPI's thunks: stdcall, ebx kept, as the code that calls them counts on.
        for (const x86::reg32 thunk : { 0x52fb2eu, 0x530747u, 0x53083du, 0x530a4eu, 0x5310beu, 0x5311c0u, 0x53138fu, 0x531439u,
                                        0x531f31u, 0x532162u, 0x5324d2u })
            specs[thunk].clobbers = ECX | EDX;
        g_methods[0x4df8e0] = &nfs3hp::netbSleep;
        g_methods[word32(0x53472c)] = &nfs3hp::netbDispatch;
        g_methods[0x4e7720] = &nfs3hp::netbWaitTasks;
        g_methods[0x533e4a] = &nfs3hp::netbHtons;
        g_methods[0x530a4e] = &nfs3hp::netbLineGetId;
        g_methods[0x53083d] = &nfs3hp::netbLineGetConfig;
        g_methods[0x530747] = &nfs3hp::netbLineGetCaps;
        g_methods[0x533e5c] = &nfs3hp::netbRecvFrom;
        g_methods[0x5311c0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::netbRecord(word32(cpu.esp + 20), 0x70);
            nfs3hp::mock(a, cpu, 0x5311c0);
        };
        g_methods[0x531f31] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::netbRecordStack(cpu, 2, 3);
            nfs3hp::mock(a, cpu, 0x531f31);
        };
        g_methods[0x533e32] = [](win32::WinApplication* a, x86::CPU& cpu) {  // sendto: the packet and the address
            nfs3hp::netbRecordStack(cpu, 2, 3);
            nfs3hp::netbRecordStack(cpu, 5, 6);
            nfs3hp::mock(a, cpu, 0x533e32);
        };
        // What is handed over by address, recorded: a reply, a printed line's arguments.
        g_methods[kSendFn] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 size = word32(cpu.esp + 4);
            nfs3hp::netbRecord(cpu.ecx, size < 0x400 ? size : 0x400);
            nfs3hp::mock(a, cpu, 0x7e0800);
        };
        g_methods[0x4e08f0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            if (word32(cpu.esp + 4) == 0x55016c)
                nfs3hp::netbRecord(cpu.esp + 8, 7 * 4);
            nfs3hp::mock(a, cpu, 0x4e08f0);
        };
        return true;
    }();
    (void)netbStandIns;

    // The NET object and its game, as sub_5133a0 and those under it read them.
    setWord32(kSession + 0x10, kNet);
    setWord32(kNet + 8, kFunctions);
    setWord32(kFunctions + 0x54, kSendFn);
    setWord32(kNet + 0x10, random(4) == 0 ? 0 : kAnswerFn);
    setWord32(kNet + 0x14, random(3) == 0 ? 0 : kGame);
    setWord32(kNet + 0xa0, random(3) == 0 ? 0 : random(0));
    setWord32(kNet + 0xa4, random(3) == 0 ? 0 : random(0));
    setWord32(kGame + 4, kNet);
    setWord32(kGame + 8, random(3) == 0 ? 1 : 0);
    setWord32(kGame + 0x4c, random(3) == 0 ? 0 : kCallbackFn);
    setWord32(kGame + 0x50, random(0x100));
    setWord32(kGame + 0x84, 1 + random(9));
    g_memory[kGame + 0x3c] = x86::reg8(random(2) ? 0 : 0x41);
    setWord32(kSession, random(3) == 0 ? 0 : random(4));
    setWord32(kSession + 0x14, random(3) == 0 ? 0 : kCallbackFn);
    setWord32(kSession + 0x18, random(3) == 0 ? 0 : kCallbackFn);
    setWord32(0x566de0, random(6));
    (void)kMessage;
    // A queue of a few elements, 0x100 bytes apart; a connection on it; a layer.
    constexpr x86::reg32 kQueue = 0xb07800, kElements = 0xb08000, kConnection = 0xb09f00;
    const x86::reg32 elements = random(5);
    setWord32(kQueue, elements);
    setWord32(kQueue + 8, elements ? kElements : 0);
    setWord32(kQueue + 0xc, elements ? kElements + 0x100 * (elements - 1) : 0);
    setWord32(kQueue + 0x10, 0x7e080c);
    setWord32(kQueue + 0x18, random(6) == 0 ? 0 : random(0));
    for (x86::reg32 i = 0; i < elements; ++i)
        setWord32(kElements + 0x100 * i, i + 1 < elements ? kElements + 0x100 * (i + 1) : 0);
    setWord32(kConnection + 0x20, kCallbackFn);
    setWord32(kConnection + 0x30, kQueue);
    setWord32(0xb07c08, 0x7e0814);
    // Connections: the elements and the one above, each with its functions, buffer and socket.
    constexpr x86::reg32 kListener = 0xb09000, kBuffer = 0xb09800, kConnFunctions = 0xb07b00, kNoClobberFn = 0x7e0810;
    setWord32(kConnFunctions, random(4) == 0 ? 2 : 1);
    setWord32(kBuffer, random(2));
    setWord32(kBuffer + 4, random(2) ? 0x7fff : random(0x40));
    setWord32(kBuffer + 8, kMessage);
    g_memory[kBuffer + 6] = x86::reg8(random(2) ? 0 : random(0x40));
    g_memory[kBuffer + 7] = 0;
    for (const x86::reg32 at : { kConnection, kElements, kElements + 0x100, kElements + 0x200, kElements + 0x300 })
    {
        setWord32(at + 4, kConnFunctions);
        setWord32(at + 0x10, random(5) == 0 ? (at == kConnection ? 0 : kBuffer) : kBuffer);
        setWord32(at + 0x14, kAnswerFn);
        setWord32(at + 0x18, kNoClobberFn);
        setWord32(at + 0x24, random(2));
        setWord32(at + 0x30, kQueue);
        setWord32(at + 0x48, random(5) == 0 ? 0xffffffff : random(0));
        setWord32(at + 0x4c, kNet);
        setWord32(at + 0x20, kCallbackFn);
        g_memory[at + 0x34] = x86::reg8(random(2));
        g_memory[at + 0x35] = 0;
    }
    setWord32(kNet + 0xc, kListener);
    setWord32(0x9f37f0, kListener);
    setWord32(kListener + 0x10, 0);
    setWord32(kListener + 0x1c, random(2) ? 0xffffffff : random(0));
    setWord32(kListener + 0x38, random(3) == 0 ? kElements + 0x400 : 0);
    setWord32(kElements + 0x400 + 0x48, random(0));
    setWord32(kElements + 0x400 + 0x24, 0);
    setWord32(kListener + 0x3c, random(3) == 0 ? 0 : kNet);
    setWord32(kListener + 0x40, random(2) ? 0 : kCallbackFn);
    g_memory[kListener + 0x46] = x86::reg8(random(8) == 0);
    setWord32(kNet + 0x10, random(4) == 0 ? 0 : kAnswerFn);
    // The open queue holds the elements, the done-with queue none: an element on both would
    // make a ring of them, which the game never has.
    setWord32(0x9f2628, elements);
    setWord32(0x9f2628 + 4, 0);
    setWord32(0x9f2628 + 8, elements ? kElements : 0);
    setWord32(0x9f2628 + 0xc, elements ? kElements + 0x100 * (elements - 1) : 0);
    setWord32(0x9f2628 + 0x18, 1 + random(100));
    setWord32(0x9f260c, 0);
    setWord32(0x9f260c + 4, 0);
    setWord32(0x9f260c + 8, 0);
    setWord32(0x9f260c + 0xc, 0);
    setWord32(0x9f260c + 0x18, 1 + random(100));
    setWord32(0x5643d8, 1 + random(120));
    g_memory[0x9f37e8] = x86::reg8(random(3) == 0 ? 0 : random(0));
    g_memory[0x9f37e9] = x86::reg8(random(2) ? 0 : random(0));
    setWord32(0x566de4, random(4));
    setWord32(0x564480, random(4) == 0 ? 0 : random(0));
    setWord32(0x566e84, random(2));
    setWord32(0x566f48, random(2));
    setWord32(0x566e88, random(3));
    // TAPI's state and the modem's settings.
    constexpr x86::reg32 kModemSettings = 0xb0b800, kModemList = 0xb0bc00;
    setWord32(0x9f4d84, kModemSettings);
    setWord32(kModemSettings + 0x14, kAnswerFn);
    setWord32(kModemSettings + 0x3c, random(4));
    setWord32(kModemSettings + 0x68, random(2) ? 0 : kMessage);
    setWord32(kModemSettings + 0x34, random(2));
    g_memory[kModemSettings + 0x2a] = x86::reg8(random(2)), g_memory[kModemSettings + 0x2b] = 0;
    g_memory[kModemSettings + 0x2c] = x86::reg8(random(2)), g_memory[kModemSettings + 0x2d] = 0;
    g_memory[kModemSettings + 0x2e] = x86::reg8(random(2)), g_memory[kModemSettings + 0x2f] = 0;
    setWord32(0x9f4d94, random(3) == 0 ? 0 : kModemList);
    for (x86::reg32 i = 0; i < 20; ++i)
        setWord32(kModemList + 8 * i + 4, random(2) ? 0 : random(0));
    setWord32(0x9f4d80, random(5));
    setWord32(0x9f4d90, random(4));
    setWord32(0x9f4d88, random(3) == 0 ? 0 : 0xb0a400);
    setWord32(0x5671a8, random(4) == 0 ? 0xffffffff : random(4));
    setWord32(0x9f4da0, random(3) == 0 ? 0 : random(0));
    setWord32(0x9f4da4, random(3) == 0 ? 0 : random(0));
    setWord32(0x9f4da8, random(3) == 0 ? 0 : random(0));
    setWord32(0x9f4d78, random(2));
    setWord32(0x9f4d7c, random(2));
    setWord32(0x9f4d9c, random(2) ? 9 : random(12));
    setWord32(0x9f4d98, random(2));
    setWord32(0x5671ac, random(2) ? 0 : kNoClobberFn);
    setWord32(0x9f4eac, random(2) ? 0 : kNoClobberFn);
    setWord32(0x566dec, random(3));
    setWord32(0xa0b0b8, random(2) ? 0 : random(0));
    setWord32(0xa0b0b4, random(2));
    // The pieces of a packet.
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        setWord32(0xb07a00 + 8 * i, random(0x40));
        setWord32(0xb07a04 + 8 * i, kMessage + 0x10 * i);
    }
    setWord32(0xb07ff0, 0);
    // The log's ring.
    setWord32(0x5689d0, random(6) == 0 ? 0 : 0xb06000);
    setWord32(0x5689d4, random(2));
    setWord32(0xa0b090, 1 + random(16));
    setWord32(0xa0b094, random(16));
    setWord32(0xa0b098, random(2) ? word32(0xa0b090) : random(word32(0xa0b090)));
}
