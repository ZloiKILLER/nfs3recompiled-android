// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    constexpr x86::reg32 kEnd = 0xb00000;  // what the machines' endpoints point at
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs[word32(0x5344b8)] = { 0, 1, 4, ECX | EDX, Kind::Plain };
        specs[word32(0x534588)] = { 0, 1, 4, ECX | EDX, Kind::Plain };
        specs[0x7e1000] = { 0, 5, 0, ECX | EDX, Kind::Plain };   // NPS's open and attach
        // Memory the callers read through what these give.
        specs[0x4e1620].kind = Kind::Shape;   // reservememadr
        specs[0x439a50].kind = Kind::Shape;   // a car's record
        specs[0x439520].kind = Kind::Shape;   // a car's id
        specs[0x439510].kind = Kind::Shape;   // a car's extras
        specs[0x439ab0].kind = Kind::Shape;   // a car's text
        specs[0x50e5a0].kind = Kind::Tuning;  // a packet from a queue, or none
        specs[0x4ea4f0].kind = Kind::Copy;
        specs[0x4e0640].kind = Kind::Memset;
        g_methods[0x4e7630] = &nfs3hp::netaTasks;
        g_methods[0x4df8e0] = &nfs3hp::netaSleep;
        g_methods[0x4a6690] = &nfs3hp::netaPlayerMachine;
        // What takes a buffer by pointer: its bytes go into the record.
        g_methods[0x4a60e0] = &nfs3hp::netaSendPacket;
        g_methods[0x4e9200] = &nfs3hp::netaFread;
        g_methods[0x533e44] = &nfs3hp::netaBind;
        g_methods[0x533e3e] = &nfs3hp::netaGetSockName;
        g_methods[0x533e56] = &nfs3hp::netaSetSockOpt;
        g_methods[0x533e5c] = &nfs3hp::netaRecvFrom;
        g_methods[0x512890] = &nfs3hp::netaDecrypt;
        g_methods[0x50e5a0] = &nfs3hp::netaQueueFetch;
        g_methods[0x49efb0] = &nfs3hp::netaSetUpEndpoint;
        g_methods[0x49f6a0] = &nfs3hp::netaEnqueuePeer;
        g_methods[0x4ee310] = &nfs3hp::netaStrcmp;
        g_methods[0x4eb390] = &nfs3hp::netaStrncmp;
        g_methods[0x43b9b0] = &nfs3hp::netaCloseViv;
        g_methods[0x512180] = &nfs3hp::netaNetLog;
        g_methods[0x4e0716] = &nfs3hp::netaFillByte;
        g_methods[0x7e1040] = &nfs3hp::netaNpsOpen;
        g_methods[0x7e1044] = &nfs3hp::netaNpsAttach;
        g_methods[0x7e1010] = &nfs3hp::netaServiceReceive;
        g_methods[0x7e1014] = &nfs3hp::netaServiceHandler;
        g_methods[0x7e1034] = &nfs3hp::netaServiceSend;
        g_methods[0x401010] = &nfs3hp::netaError;
        g_methods[0x533e32] = &nfs3hp::netaSendTo;
        g_methods[0x50e220] = &nfs3hp::netaFindInQueue;
        g_methods[0x7e1020] = &nfs3hp::netaRasEnum;
        specs[word32(0x53458c)] = { 0, 1, 4, ECX | EDX, Kind::Bool };      // LoadLibraryA
        g_methods[word32(0x534558)] = &nfs3hp::netaGetProc;
        g_methods[0x49b800] = &nfs3hp::netaBeginCarSend;
        specs[0x7e1024] = { EAX, 0, 0, ECX | EDX, Kind::Plain };           // a game's handler
        const std::uint8_t kFour = EAX | EBX | ECX | EDX;
        specs[0x7e1010] = { kFour, 1, 4, EBX | ECX | EDX, Kind::Bool };    // a service's receive
        specs[0x7e1014] = { kFour, 2, 8, EBX | ECX | EDX, Kind::Bool };    // the service's packet handler
        specs[0x7e1018] = { kFour, 0, 0, EBX | ECX | EDX, Kind::Plain };   // a service's call, its open
        specs[0x7e101c] = { kFour, 2, 8, EBX | ECX | EDX, Kind::Plain };   // the services' open game
        g_methods[0x4dfb70] = &nfs3hp::netaWaitEvent;
        g_methods[0x533eaa] = &nfs3hp::netaSocket;
        g_methods[0x533ea4] = &nfs3hp::netaCloseSocket;
        g_methods[0x533e50] = &nfs3hp::netaIoctl;
        g_methods[0x7e1028] = &nfs3hp::netaRasConnections;
        specs[word32(0x534750)] = { 0, 1, 4, ECX | EDX, Kind::Bool };      // GetSystemMetrics
        specs[0x7e102c] = { 0, 2, 0, ECX | EDX, Kind::Plain };             // NPS's close
        specs[0x7e1030] = { 0, 1, 0, ECX | EDX, Kind::Plain };             // NPS's end
        specs[0x7e1034] = { kFour, 1, 4, EBX | ECX | EDX, Kind::Plain };   // a service's send
        specs[0x50df60].kind = Kind::Tuning; // a packet off a queue, or none
        specs[0x533eb0].kind = Kind::Bool;   // WSAStartup
        specs[0x50e1d0].kind = Kind::Bool;   // a queue's length
        specs[0x4f5040].kind = Kind::Bool;   // a game's player
        specs[0x4eb390].kind = Kind::Bool;   // strncmp
        specs[0x7e1038] = { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain };     // a packet's handler, an endpoint's receive
        specs[0x7e103c] = { EAX, 0, 0, EBX | ECX | EDX, Kind::Plain };     // a service's poll
        specs[0x49f1c0].kind = Kind::Focus;  // a session's index
        g_methods[0x50e470] = &nfs3hp::netaQueueFirst;
        specs[word32(0x5344dc)] = { 0, 1, 4, ECX | EDX, Kind::Plain };     // FreeLibrary
        specs[0x49efb0].kind = Kind::Shape;  // an endpoint made
        specs[0x533e44].kind = Kind::Bool;   // bind
        specs[0x533e3e].kind = Kind::Bool;   // getsockname
        specs[0x4ee310].kind = Kind::Bool;   // strcmp
        specs[0x4ee2f0].kind = Kind::Bool;   // strrchr
        specs[0x49f680].kind = Kind::Bool;   // nobody pending
        specs[0x49fb10].kind = Kind::Bool;   // dial-up's functions there
        specs[0x4f5390].kind = Kind::Focus;  // a game's place
        specs[0x512660].kind = Kind::Bool;
        specs[0x512890].kind = Kind::Bool;
        specs[0x4a6650].kind = Kind::Focus;  // a machine's number
        return true;
    }();
    (void)standInsAdded;
    const auto setWord16 = [&](x86::reg32 at, x86::reg32 value) {
        g_memory[at] = x86::reg8(value);
        g_memory[at + 1] = x86::reg8(value >> 8);
    };
    const auto shortString = [&](x86::reg32 at) {
        const x86::reg32 length = random(9);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[at + i] = x86::reg8('a' + random(26));
        g_memory[at + length] = 0;
    };
    setWord32(0x79c1a0, random(0x10000));
    setWord32(0x79c1a4, random(0x10000));
    setWord16(0x7a2276, random(5));
    setWord16(0x7a2278, random(5));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        setWord32(0x7a21f0 + 4 * i, kEnd + 0x100 * i);
        setWord32(kEnd + 0x100 * i + 0x24, random(4) != 0);
        setWord16(0x7a22ba + 2 * i, random(4));
        setWord16(0x7a22ca + 2 * i, random(5) == 0);
        setWord32(0x7a230c + 0x6c * i, random(2));
        setWord32(0x7a22f8 + 0x6c * i, random(2));
        shortString(0x7a25dc + 11 * i);
    }
    setWord16(0x7a229a, random(2));
    setWord16(0x7a2298, random(2));
    setWord16(0x7a229c, random(4));
    setWord16(0x7a22a8, random(2));
    setWord16(0x7a2296, random(2));
    setWord16(0x7a22da, random(2));
    setWord16(0x7a22dc, random(6) == 0);
    setWord32(0x7a22e0, random(4) == 0 ? 0 : 3);
    setWord32(0x7a22e4, random(3));
    setWord32(0x6fd3b0, random(3));
    setWord32(0x6fd3a0, random(3));
    setWord32(0x6fd514, random(6));
    setWord32(0x6fd520, random(6));
    setWord32(0x5efda4, random(5));
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        setWord32(0x5efa48 + 4 * i, kEnd + 0x1000 + 0x600 * i);
        setWord32(0x5efac8 + 4 * i, kEnd + 0x1000 + 0x600 * i);
    }
    setWord32(0x566de0, random(7));
    setWord32(0x566de8, random(2));
    setWord32(0x7d36c4, random(3) == 0);
    setWord32(0x7d36dc, random(0x10000));
    setWord32(0x7a21e4, 0x7e1040);
    setWord32(0x7a21dc, 0x7e1044);
    for (x86::reg32 at : { 0x6fd2fcu, 0x55e898u, 0x55ed08u, 0x7a1fa0u, 0x7a1fe0u, 0x79f921u })
        shortString(at);
    // Settings as the setup packet last had them, now and then not.
    static const x86::reg32 kFields[][2] = {
        { 0x79f8f1, 0x6fd3b8 }, { 0x79f8f2, 0x6fd4ac }, { 0x79f8f3, 0x6fd504 }, { 0x79f8f4, 0x6fd4b4 },
        { 0x79f8f5, 0x6fd4b0 }, { 0x79f8f6, 0x6fd4d4 }, { 0x79f908, 0x6fd4d0 }, { 0x79f909, 0x6fd4e8 },
        { 0x79f90a, 0x6fd4c4 }, { 0x79f90b, 0x6fd4cc }, { 0x79f90c, 0x6fd4c8 }, { 0x79f90d, 0x6fd4c0 },
        { 0x79f914, 0x6fd470 }, { 0x79f92c, 0x6fd2bc }, { 0x79f92d, 0x6fd2d4 }, { 0x79f920, 0x6fd508 },
    };
    const bool settled = random(2) != 0;
    for (const auto& field : kFields)
        setWord32(field[1], settled && random(40) != 0 ? g_memory[field[0]] : random(256));
    setWord32(0x79f910, settled ? word32(0x6fd3b4) : random(0));
    setWord32(0x79f978, random(2));
    setWord32(0x6fd4f4, random(8));
    setWord32(0x6fd510, random(9));
    setWord32(0x6fd50c, random(2));
    for (x86::reg32 i = 0; i < 16; ++i)
        if (random(4) == 0)
            setWord32(0x5efac8 + 4 * i, 0);
    // The services: a table of each, a record of each.
    constexpr x86::reg32 kService = 0xb20000, kTable = 0xb21000, kGame = 0xb22000;
    setWord32(0x566de4, random(4));
    setWord32(0x566df4, random(2));
    setWord32(0x566df8, random(3));
    setWord32(0x566dfc, random(2) ? random(0) : 0);
    setWord32(0x566e00, random(6));
    setWord32(0x5643d8, random(1000));
    setWord32(0x566ddc, random(2));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 table = kTable + 0x100 * i;
        setWord32(0x9f25b0 + 4 * i, table);
        setWord32(0x9f25d0 + 4 * i, kTable + 0x800 + 0x40 * i);
        shortString(kTable + 0x800 + 0x40 * i + 4);
        setWord32(table + 4, random(2) ? 0 : 0x7e1018);
        setWord32(table + 0x54, 0x7e1034);
        setWord32(table + 8, 0x7e101c);
        setWord32(table + 0x40, 0x7e1014);
        setWord32(table + 0x48, random(2) ? 0 : 0x7e1018);
        setWord32(table + 0x58, 0x7e1010);
        setWord32(table + 0x5c, 0x7e1018);
        setWord32(kGame + 0x100 * i + 8, random(2) ? 0 : 0x7e1024);
        setWord32(0x9f2644 + 0x20 * i, kGame + 0x100 * i);
    }
    setWord32(kService + 4, kTable);
    setWord32(kService + 8, kTable + 0x100);
    setWord32(kService + 0xc, random(4) ? 0xb23000 : 0);
    setWord32(kService + 0xa8, random(3));
    setWord32(0x7a05d0, 0x7e1020);
    // The game stopping is waited out in sub_4a65b0: only where nothing waits on it.
    g_memory[0x564819] = 0;
    for (x86::reg32 i = 0; i < 0x13; ++i)
        setWord32(0x55ecbc + 4 * i, 0x7e1038);
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        setWord32(0x7a2308 + 0x6c * i, 0x7e1038);
        g_memory[0x7a231c + 0x6c * i] = x86::reg8(random(8));
        setWord32(kTable + 0x100 * i + 0xc, 0x7e103c);
        setWord32(kTable + 0x100 * i + 0x10, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x14, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x1c, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x28, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x38, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x4c, 0x7e1018);
        setWord32(kTable + 0x100 * i + 0x50, 0x7e1018);
    }
    setWord32(0x7a01ec, random(2));
    setWord32(0x55e860, random(2));
    if (settled)
    {
        // This machine's record as it was last sent, so that nothing has changed.
        for (x86::reg32 m = 0; m < 4; ++m)
        {
            g_memory[0x60 * m + 0x79f604] = g_memory[0x79f92c];
            g_memory[0x60 * m + 0x79f605] = g_memory[0x79f92d];
            g_memory[0x60 * m + 0x79f5f8] = g_memory[0x79f920];
        }
        setWord32(0x79f961, word32(0x6fd307));
        for (x86::reg32 slot = 0; slot < 16; ++slot)
            setWord32(0x5f3c00 + 16 * slot + 0x380, word32(0x79f968));
    }
    setWord32(0x7a0394, kTable);
    setWord32(0xb24004, kTable);
    setWord32(0x55e714, random(0x10000));
    setWord32(0x7a05cc, 0x7e1028);
    setWord32(0x7a21e0, 0x7e102c);
    setWord32(0x7a21cc, 0x7e1030);
    setWord32(0x7a01e4, random(6));
    setWord32(0x7d36ac, random(0x1000));
    setWord32(0x55ecb0, random(2) ? random(100) : 0xfffffff0u);
    setWord32(0x7a215c, random(2));
    setWord32(0x7a2160, random(2));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        setWord32(0x7dd080 + 20 * i, random(2));
        setWord32(0x7dd07c + 20 * i, random(0x1000));
    }
}
