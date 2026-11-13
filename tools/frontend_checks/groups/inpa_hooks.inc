// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* sub_4e7630, the system's tasks, is where the race's time passes: the clocks move,
 * more frames come in, and sooner or later the race is over, for the loop to end. */
void inpaSystemTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4e7630);
    setWord32(0x5efdf8, word32(0x5efdf8) + scripted(8));
    setWord32(0x5efdfc, word32(0x5efdfc) + scripted(8));
    // The input frames, now and then about the 16 the loop lets its keys fall behind.
    setWord32(0x79c4ac, word32(0x79c4ac) + (scripted(4) == 0 ? 14 + scripted(6) : scripted(4)));
    if (scripted(4) == 0)
        setWord32(0x560104, word32(0x560104) + scripted(3));
    if (scripted(4) == 0)
        setWord32(0x7d369c, 1);
    // The clock, now and then, to where the first pass's comparison is close: the ticks since
    // the loop began less its 10000 against the half frames due (the world keeps those ticks).
    if (scripted(3) == 0)
        setWord32(0x5efdf8, word32(0x880f00) + 10000 + 2 * word32(0x560104) + scripted(3) - 1);
}

/* A result that is a small number, as what reads it compares it. */
template <x86::reg32 Address, x86::reg32 Below>
void inpaSmall(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    cpu.eax = scripted(Below);
}

/* A stand-in recording esp, for a native's frame to show, which the table's do not. */
template <x86::reg32 Address>
void inpaAtEsp(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, Address);
}

/* sub_4c4de0, 0 most of the time, for the frames to run. */
void inpaFrameHeld(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4c4de0);
    cpu.eax = scripted(4) == 0 ? 1 + scripted(4) : 0;
}

/* sub_4a60e0: what a network race's hold sends, from its frame: its kind and its two
 * words, the three bytes between them left as they were by both. */
void inpaSendPacket(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    g_log.push_back(g_memory[cpu.edx]);
    g_log.push_back(word32(cpu.edx + 4));
    g_log.push_back(word32(cpu.edx + 8));
    mock(app, cpu, 0x4a60e0);
}

/* sub_4c69a0: the frames taken, often at the frame a hold waits for or just about. */
void inpaFramesTaken(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4c69a0);
    cpu.eax = scripted(2) != 0 ? word32(0x7d36c8) + scripted(3) - 1 : scripted(64);
}

/* sub_4bd640: the panel's keys, and the race over meanwhile: the one way the loop's
 * frame keys see it over, past its own look at that. */
void inpaPanelKeys(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4bd640);
    if (scripted(3) == 0)
    {
        setWord32(0x7d369c, 1);
        setWord32(0x7d36a8, scripted(2));
    }
}

/* sub_435310: the mouse into [edx] and [ebx], the ways the loop's comparisons go. */
void inpaReadMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 x = cpu.edx, y = cpu.ebx;
    mock(app, cpu, 0x435310);
    // Mostly still: a move drops the click (0x55e5ec) that what comes after it looks at.
    setWord32(x, scripted(4) == 0 ? scripted(3) : word32(0x7d36b8));
    setWord32(y, scripted(4) == 0 ? scripted(3) : word32(0x7d36bc));
}

/* What these functions go on to read may change in a call: now and then it does, so
 * that a native reading it at the wrong moment, or keeping it, shows. */
template <x86::reg32 Address>
void inpaMovesFlags(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, Address);
    switch (scripted(16))
    {
    case 0: setWord32(0x7d36a0, scripted(2)); break;
    case 1: setWord32(0x7d369c, scripted(2)); break;
    case 2: setWord32(0x7d36a8, scripted(2)); break;
    case 3: setWord32(0x7d36b0, word32(0x7d36b0) + scripted(2)); break;
    case 4: setWord32(0x7d3688, word32(0x7d3688) + scripted(2)); break;
    case 5: setWord32(0x6fd3b0, scripted(5)); break;
    case 6: setWord32(0x6fd3a0, scripted(3)); break;
    case 7: setWord32(0x7cda58, scripted(5)); break;
    case 8: setWord32(0x5600fc, scripted(2)); break;
    case 9: setWord32(0x7d36c8, scripted(8)); break;
    case 10: setWord32(0x7d36c0, scripted(2)); break;
    case 11: setWord32(0x7d3674, scripted(2)); break;
    case 12: setWord32(0x55e88c, scripted(2)); break;
    case 13: setWord32(0x7d36b4, scripted(3)); break;
    case 14: setWord32(0x7a22dc, scripted(3) == 0 ? scripted(2) : 0); break;
    default: break;
    }
}
