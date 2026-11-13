// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* tools/apply_race_state.py's and tools/apply_loading_screen.py's calls in sub_4a3400. */
void raceRunning(bool running)
{
    g_log.push_back(0x7e4a0000 | (running ? 1 : 0));
}

void loadingScreenFit(win32::WinApplication*, x86::CPU&, bool on)
{
    g_log.push_back(0x7e4b0000 | (on ? 1 : 0));
}

// These stand in for game functions that need more than the kinds of kSpecs.

/* An integer of a view's size, now and then junk. */
static x86::reg32 loopaViewSize(x86::reg32 most)
{
    return scripted(12) == 0 ? scripted(0) : scripted(most);
}

/* sub_4beb80: the view, to the four places it is handed. */
void loopaViewport(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 x = cpu.eax, y = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    mock(app, cpu, 0x4beb80);
    setWord32(x, loopaViewSize(320));
    setWord32(y, loopaViewSize(240));
    setWord32(width, loopaViewSize(1920));
    setWord32(height, loopaViewSize(1440));
}

/* sub_4beb20: a view's rectangle, to the four places it is handed. */
void loopaReadRect(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 left = cpu.edx, top = cpu.ebx, width = cpu.ecx, height = word32(cpu.esp + 4);
    mock(app, cpu, 0x4beb20);
    setWord32(left, loopaViewSize(320));
    setWord32(top, loopaViewSize(240));
    setWord32(width, loopaViewSize(1920));
    setWord32(height, loopaViewSize(1440));
}

/* sub_4c12e0: its four vertices, all of them recorded: they live in the frame of the
 * caller, which nothing else looks at. */
void loopaClipQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 vertex : { cpu.eax, cpu.edx, cpu.ebx, cpu.ecx })
        for (x86::reg32 at = 0; at < 0x20; at += 4)
            g_log.push_back(word32(vertex + at));
    mock(app, cpu, 0x4c12e0);
}

/* sub_4eb390: what it is handed to compare, in the frame, and the two bytes past it that
 * nothing should touch; equal now and then. */
void loopaCompare(win32::WinApplication* app, x86::CPU& cpu)
{
    for (x86::reg32 at = 0; at < 8; ++at)
        g_log.push_back(g_memory[cpu.edx + at]);
    mock(app, cpu, 0x4eb390);
    if (scripted(2) == 0)
        cpu.eax = 0;
}

/* A stage of a load that says whether it failed: 0 mostly, or the load never gets past it. */
template <x86::reg32 Address>
void loopaMostlyZero(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    cpu.eax = scripted(8) == 0 ? scripted(0) | 1 : 0;
}

/* sub_4a33d0: the keep-alive, which is where a load is stopped. */
void loopaKeepAlive(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4a33d0);
    if (scripted(12) == 0)
        setWord32(0x55e88c, 1 + scripted(3));
}

/* sub_4e43a0: the size of a file, about what a ghost may be. */
void loopaFileSize(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e43a0);
    const x86::reg32 pick = scripted(8);
    cpu.eax = pick == 0 ? 0x41575 : pick == 1 ? 0x41574 : pick == 2 ? scripted(0) | 0x80000000 : scripted(0x41574);
}

/* sub_4c4e40: the step of the race, which is what ends the loop and says how. */
void loopaRaceStep(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4c4e40);
    setWord32(0x7d3698, scripted(3) == 0 ? 1 : 0);
    setWord32(0x7d36b4, scripted(4));
}
