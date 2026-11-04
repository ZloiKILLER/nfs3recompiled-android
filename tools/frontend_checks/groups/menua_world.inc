// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // Stand-ins these functions need and the check's table lacks, added to it once.  Those that
    // fill what the real one fills, or whose argument is a record on the stack (which the memory
    // comparison does not hold), go through g_methods: the record logged, then the stand-in.
    static const bool standInsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const std::uint8_t kFour = EAX | EBX | ECX | EDX;
        specs[0x4e7630] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4eede0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4181d0] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4cc8e0] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[0x452190] = { kFour, 2, 8, EBX | ECX | EDX, Kind::Plain, true };
        specs[0x4d7990] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain };
        specs[0x4d7160] = { kFour, 0, 0, EBX | ECX | EDX, Kind::Plain };
        specs[0x4db000] = { kFour, 14, 56, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d6730] = { kFour, 2, 8, EBX | ECX | EDX, Kind::Plain };
        specs[0x49bfc0] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        // The row under the pointer: a small number (its generated caller calls it by name, so that
        // g_methods does not reach it).
        specs[0x49c1c0] = { EAX | EDX, 0, 0, EDX, Kind::KeyHandler };
        specs[0x49c240] = { kFour, 0, 0, EDX, Kind::Bool };
        specs[0x49c1f0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x49c080] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49ca50] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4398c0] = { kFour, 2, 8, EDX, Kind::Plain };
        specs[0x431900] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[0x4d6070] = { kFour, 0, 0, EBX | ECX | EDX, Kind::Plain };
        specs[0x4e1620] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain, true };
        specs[0x4d8910] = { kFour, 1, 4, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d8930] = { kFour, 1, 4, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d88c0] = { kFour, 5, 20, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d81b0] = { kFour, 4, 16, EBX | ECX | EDX, Kind::Plain };
        specs[0x4c12e0] = { kFour, 0, 0, EBX | ECX | EDX, Kind::Plain };
        specs[0x4d1400] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4d1410] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x42d9b0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x437080] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x42d810] = { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4eedf0] = { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain, true };
        specs[0x49a520] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x49abd0] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x49acd0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49ae30] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4a6a10] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x443630] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x43c000] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x44a6b0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x449e50] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        specs[0x451960] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x445e60] = { EAX, 0, 0, 0, Kind::KeyHandler };
        specs[0x44a2d0] = { EAX | EDX, 0, 0, EDX, Kind::KeyHandler };
        specs[0x44aa30] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x44ab40] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x44a240] = { EAX | EDX, 0, 0, EDX, Kind::FindItem };
        // The screen's Escape button: now and then the item selected.
        g_methods[0x44a240] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 menu = cpu.eax;
            nfs3hp::mock(a, cpu, 0x44a240);
            if (scripted(3) == 0)
            {
                std::int16_t first, selected;
                std::memcpy(&first, g_memory + menu + 0x18, 2);
                std::memcpy(&selected, g_memory + menu + 0x1a, 2);
                cpu.eax = kItems + kItemSize * x86::reg32(first + selected);
            }
        };
        // A key, or none.
        g_methods[0x451960] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x451960);
            if (scripted(4) == 0)
                cpu.eax &= 0xffff0000;
        };
        specs[0x4ee310] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        specs[0x44af10] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x44aec0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x449510] = { EAX | EDX, 0, 0, EDX, Kind::Plain };
        specs[0x4d1750] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4bed90] = { EAX, 0, 0, 0, Kind::ListModes };
        specs[0x4358f0].kind = Kind::Bool;

        // A joystick: its buttons, axes, "any held" and hats, on the caller's stack.
        g_methods[0x4358f0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 state = cpu.edx;
            g_log.push_back(state);
            nfs3hp::mock(a, cpu, 0x4358f0);
            if (cpu.eax == 0)
                return;
            std::memset(g_memory + state + 0x30, 0, 32);
            if (scripted(2))
                g_memory[state + 0x30 + scripted(32)] = 1;
            for (x86::reg32 i = 0; i < 8; ++i)
                setWord32(state + 0x50 + 4 * i, scripted(4) ? scripted(0x10000) : scripted(0));
            setWord32(state + 0x70, scripted(2));
            for (x86::reg32 i = 0; i < 4; ++i)
                setWord32(state + 0x74 + 4 * i, scripted(4) == 0 ? scripted(8) : 0);
        };
        // The item under the pointer: -1 none.
        g_methods[0x44a6b0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x44a6b0);
            cpu.eax = scripted(3) == 0 ? 0xffffffff : scripted(12);
        };
        g_methods[0x49c080] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x49c080);
            cpu.eax = scripted(2) ? 0xd : scripted(0x100);
        };
        // The messages: a count and lines of 0x65.
        g_methods[0x437080] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x437080);
            const x86::reg32 list = 0x5f4400;
            const x86::reg32 count = scripted(4);
            setWord32(list, count);
            for (x86::reg32 i = 0; i < count; ++i)
            {
                const x86::reg32 line = list + 4 + i * 0x65;
                const x86::reg32 length = scripted(40);
                for (x86::reg32 k = 0; k < length; ++k)
                    g_memory[line + k] = x86::reg8('a' + scripted(26));
                g_memory[line + length] = 0;
            }
            // The line typed may go while the messages are looked at.
            if (scripted(5) == 0)
                setWord32(0x5f3fdc, 0);
            cpu.eax = list;
        };
        // Memory reserved: somewhere out of the way.
        g_methods[0x4e1620] = [](win32::WinApplication* a, x86::CPU& cpu) {
            nfs3hp::mock(a, cpu, 0x4e1620);
            cpu.eax = 0x900000;
        };
        // A colour on the stack in, one on the stack out.
        g_methods[0x49ca50] = [](win32::WinApplication* a, x86::CPU& cpu) {
            const x86::reg32 hsv = cpu.eax, rgb = cpu.edx;
            for (x86::reg32 i = 0; i < 4; ++i)
                g_log.push_back(word32(hsv + 4 * i));
            nfs3hp::mock(a, cpu, 0x49ca50);
            for (x86::reg32 i = 0; i < 3; ++i)
                setWord32(rgb + 4 * i, scripted(0));
        };
        // The quad on the stack.
        g_methods[0x4c12e0] = [](win32::WinApplication* a, x86::CPU& cpu) {
            for (const x86::reg32 vertex : { cpu.eax, cpu.edx, cpu.ebx, cpu.ecx })
                for (x86::reg32 i = 0; i < 8; ++i)
                    g_log.push_back(i == 5 ? word32(vertex + 4 * i) & 0xff : word32(vertex + 4 * i));
            nfs3hp::mock(a, cpu, 0x4c12e0);
        };
        // The message sent, from the stack.
        g_methods[0x42d810] = [](win32::WinApplication* a, x86::CPU& cpu) {
            for (const char ch : guestString(cpu.edx, 128))
                g_log.push_back(x86::reg32(std::uint8_t(ch)));
            nfs3hp::mock(a, cpu, 0x42d810);
        };
        return true;
    }();
    (void)standInsAdded;

    // The controls' list.
    setWord32(0x555608, 18 + random(10));
    setWord32(0x5f3d9c, random(200));
    setWord32(0x5f3d98, random(100));
    setWord32(0x5f3da0, random(200));
    setWord32(0x555640, random(2));
    setWord32(0x555644, random(40));
    setWord32(0x5f3d94, random(2) ? 0 : random(0));
    {
        static const x86::reg32 kCursorKeys[] = { 0x4800, 0x4b00, 0x5000, 0x4d00, 0, 0xd };
        setWord32(0x66464c, kCursorKeys[random(6)]);
        static const x86::reg32 kScans[] = { 0x4a, 0x4e, 0x37, 0x35, 2, 0xb, 0x3b, 0x44, 0xffffffff, 0x1e, 0x20, 0x45, 0xc };
        setWord32(0x66465c, kScans[random(13)]);
    }
    for (x86::reg32 i = 0; i < 13; ++i)
        setWord32(0x55560c + 4 * i, random(100));
    // Both players' bindings: every kind, of few enough values to meet.
    for (x86::reg32 at = 0x6fd20c; at < 0x6fd2a8; at += 4)
        setWord32(at, random(4) == 0 ? random(0) : (random(2) << 28) | (random(4) << 24) | (random(4) << 16) | (random(4) << 8) | (1 + random(4)));
    for (x86::reg32 device = 0; device < 6; ++device)
    {
        setWord32(0x6fc970 + 0x88 * device + 0x40, random(0x100));
        // The axes at rest, in the range they are sampled in: the table has room for two devices.
        if (device < 2)
            for (x86::reg32 axis = 0; axis < 8; ++axis)
                setWord32(0x5f3d50 + (device << 5) + 4 * axis, random(5) == 0 ? random(0) : random(0x100));
    }
    // The kinds of binding a race has read: digital or not.
    for (const x86::reg32 at : { 0x6fd2dcu, 0x6fd2e0u, 0x6fd2e4u, 0x6fd348u, 0x6fd34cu, 0x6fd350u })
        setWord32(at, random(2));

    // The pool's items, for the list and for the picker: fields that lay them out on the screen.
    for (std::uint32_t i = 0; i < kPoolItems + 10; ++i)
    {
        const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
        const auto setShort = [&](x86::reg32 at, x86::reg32 value) { std::memcpy(g_memory + item + at, &value, 2); };
        setShort(0x06, random(300));
        setShort(0x08, random(300));
        setShort(0x1a, random(300));
        setShort(0x1e, random(300));
        setShort(0x3c, random(3) == 0 ? random(26) : random(8) == 0 ? 0 : 0xffff);
        setShort(0x3e, random(4) == 0 ? 0 : random(100));
        for (x86::reg32 k = 0; k < 4; ++k)
            setWord32(item + 0x40 + 4 * k, random(5) == 0 ? random(0) : random(300));
        setWord32(item + 0x60, random(3));
        // Each area of the picker: its top, left, width and height, never empty; the
        // triangle's height and the bar's divide.
        for (x86::reg32 at = 0x64; at < 0x8c; at += 8)
        {
            setShort(at, random(120));
            setShort(at + 2, random(120));
            setShort(at + 4, 1 + random(120));
            setShort(at + 6, 1 + random(120));
        }
        setShort(0x8c, random(4) == 0 ? random(0x10000) : random(6));
        setShort(0x8e, random(3) == 0 ? random(6) : 0);
    }
    setWord32(0x55e5e4, random(8) == 0 ? random(0) : random(640));
    setWord32(0x55e5e8, random(8) == 0 ? random(0) : random(480));
    setWord32(0x749a38, random(0));
    setWord32(0x79c1a4, random(4) == 0 ? random(0) : random(0x10000));
    setFloat(0x7d34f8, random(2) ? 65536.0f : float(random(1000)));
    setFloat(0x7d34fc, float(random(640)));
    setFloat(0x7d350c, float(random(640)));
    setFloat(0x7d3504, float(random(480)));
    setFloat(0x7d3500, float(random(480)));
    setFloat(0x563aac, random(2) ? 1.0f : float(random(100000)));

    // The network's panel.
    g_memory[0x7a22da] = x86::reg8(random(3) == 0), g_memory[0x7a22db] = 0;
    g_memory[0x7a22dc] = x86::reg8(random(4) == 0), g_memory[0x7a22dd] = 0;
    setWord32(0x7a22e0, random(4) != 0);
    g_memory[0x7a229c] = x86::reg8(random(2) ? 0x21 : random(0x40)), g_memory[0x7a229d] = 0;
    g_memory[0x5556cc] = x86::reg8(random(5) != 0 ? 1 + random(100) : 0), g_memory[0x5556cd] = 0;
    setWord32(0x555748, random(2));
    setWord32(0x5556d0, random(2));
    setWord32(0x5f3fc8, random(200));
    setWord32(0x5f3fcc, 0xf0 + random(0x180));
    // Its shapes, none now and then.
    for (x86::reg32 at = 0x5f3ff4; at < 0x5f4006; at += 2)
    {
        g_memory[at] = x86::reg8(random(3) == 0 ? 0 : 1 + random(200));
        g_memory[at + 1] = 0;
    }
    setWord32(0x5f3fbc, random(400));
    setWord32(0x5f3fb8, random(600));
    setWord32(0x5f3fc4, random(400));
    {
        // The line typed, a text for the tests of a whole text, the player's name.
        const auto makeText = [&](x86::reg32 at, x86::reg32 most) {
            const x86::reg32 length = random(3) == 0 ? most - 1 : random(3) == 0 ? 0 : random(most);
            for (x86::reg32 k = 0; k < length; ++k)
                g_memory[at + k] = x86::reg8(0x20 + random(0x60));
            g_memory[at + length] = 0;
        };
        makeText(0x5f4300, 0x58);
        makeText(0x5f4380, 0x20);
        makeText(0x6fd2fc, 10);
    }
    setWord32(0x5f3fdc, random(6) == 0 ? 0 : 0x5f4300);
    setWord32(0x5f3fd0, random(6) == 0 ? 0 : 0x5f43c0);
    setWord32(0x5f40bc, random(0x40));
    {
        static const x86::reg32 kTypedKeys[] = { 0x4800, 0x4d00, 0x5000, 0x4b00, 0xd, 0x1b, 0x41, 0x8, 0x5300, 0x7f };
        setWord32(0x664654, kTypedKeys[random(10)]);
    }
    setWord32(0x664658, random(2));

    // What may be typed, by the language: what the table has, or nothing.
    for (x86::reg32 language = 0; language < 8; ++language)
    {
        setWord32(0x558c7c + 20 * language, random(4) == 0 ? 0 : 0x443510);
        setWord32(0x558c84 + 20 * language, random(4) == 0 ? 0 : 0x4435c0);
        setWord32(0x558c8c + 20 * language, random(4) == 0 ? 0 : 0x443620);
    }

    // The pause menu.
    setWord32(0x5efac8, 0x5f3c00);
    setWord32(0x5f3c00 + 0x200, random(4));
    setWord32(0x5f3c00 + 0x2c4, 2 + random(4));
    g_memory[0x55eb2e] = x86::reg8(random(2) ? 0x20 : 0);
    setWord32(0x559254, random(2) ? 0 : random(2) ? 5 : random(14));
    setWord32(0x559230, random(4) == 0 ? 0x6040d0 : g_menu);
    setWord32(0x660bb0, random(8));
    for (x86::reg32 m = 0; m < 8; ++m)
    {
        setWord32(0x6040d0 + 0x24 * m + 0x10, random(3) == 0 ? 0 : kShowHandler);
        // Its first item and the one selected, in the pool, as g_menu's.
        const x86::reg32 at = 0x6040d0 + 0x24 * m;
        if (at != g_menu)
        {
            g_memory[at + 0x18] = x86::reg8(kPoolFirst + random(10)), g_memory[at + 0x19] = 0;
            g_memory[at + 0x1a] = x86::reg8(random(10)), g_memory[at + 0x1b] = 0;
        }
    }
    for (std::uint32_t i = 0; i < kPoolItems; ++i)
    {
        const x86::reg32 item = poolItem(i);
        setWord32(item + 0x10, random(4) == 0 ? 0 : random(0));
        if (random(4) == 0)
            setWord32(item + 0x40, 0);
    }
}
