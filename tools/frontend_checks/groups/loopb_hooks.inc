// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* tools/apply_widescreen.py's patches in sub_4be5e0 and sub_4dbce0: what they are handed,
 * recorded, and an answer of the script's, often what they were handed. */
x86::reg32 letterboxHeight(x86::reg32 width, x86::reg32 height, x86::reg32 half)
{
    g_log.push_back(0x7e4b0000);
    g_log.push_back(width);
    g_log.push_back(height);
    g_log.push_back(half);
    return scripted(2) ? half : scripted(600);
}

x86::reg32 widescreenHalfAngle(win32::WinApplication*, x86::reg32 half, x86::reg32 vertical)
{
    g_log.push_back(0x7e4c0000);
    g_log.push_back(half);
    g_log.push_back(vertical);
    return scripted(2) ? half : scripted(90);
}

/* sub_431c30: the x87's control word as the frame opens, which nothing else shows. */
void loopbFirstLayer(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.fpu.control.word);
    mock(app, cpu, 0x431c30);
}

/* sub_435310: the mouse, written where it was asked: often where it was, or next to it. */
void loopbReadMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 buttons = cpu.eax, x = cpu.edx, y = cpu.ebx;
    mock(app, cpu, 0x435310);
    const auto nextTo = [](x86::reg32 was) { return was + scripted(5) - 2; };
    setWord32(x, scripted(4) ? nextTo(word32(0x7a3cf8)) : scripted(0));
    setWord32(y, scripted(4) ? nextTo(word32(0x7a3cfc)) : scripted(0));
    setWord32(buttons, scripted(4) ? word32(0x7a3d00) : scripted(4));
}

/* sub_4be000: al 1 for a digit or a sign, as the real one. */
void loopbIsNumber(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 eax = cpu.eax;
    mock(app, cpu, 0x4be000);
    const x86::reg8 c = x86::reg8(eax);
    cpu.eax = (eax & 0xffffff00u) | (((c >= '0' && c <= '9') || c == '-' || c == '+') ? 1u : 0u);
}

/* sub_4be570: the mode's size, written where it was asked, of real sizes mostly. */
void loopbModeSize(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 x = cpu.edx, y = cpu.ebx, width = cpu.ecx, height = word32(cpu.esp + 4);
    mock(app, cpu, 0x4be570);
    static const x86::reg32 kWidths[] = { 640, 800, 1024, 1280, 1920, 2400 };
    static const x86::reg32 kHeights[] = { 480, 600, 768, 720, 1080, 1079 };
    setWord32(x, scripted(3) ? 0 : scripted(100));
    setWord32(y, scripted(3) ? 0 : scripted(100) - 50);
    setWord32(width, scripted(8) ? kWidths[scripted(6)] : scripted(0));
    setWord32(height, scripted(8) ? kHeights[scripted(6)] : scripted(0));
}

/* sub_423d90: -1, none, half the time. */
void loopbNearestSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x423d90);
    cpu.eax = scripted(2) ? x86::reg32(-1) : scripted(8);
}

/* sub_4f0430: atoi, what it is handed in the frame recorded. */
void loopbAtoi(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const char c : guestString(cpu.eax, 0x180))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    mock(app, cpu, 0x4f0430);
}

/* sub_4eb1fc: rand's range. */
void loopbRand(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4eb1fc);
    cpu.eax = scripted(0x8000);
}
