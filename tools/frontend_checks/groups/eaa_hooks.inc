// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU& cpu, x86::reg32 address);

/* A printf's format and arguments, from the stack slot of the format on: every argument
 * recorded, and what the real one would write (sprintf's mock has no '.' in a width). */
std::string eaaFormat(x86::reg32 formatAt)
{
    const std::string fmt = guestString(word32(formatAt));
    x86::reg32 argument = formatAt + 4;
    std::string out;
    g_log.push_back(word32(formatAt));
    for (std::size_t i = 0; i < fmt.size(); ++i)
    {
        if (fmt[i] != '%' || i + 1 == fmt.size())
        {
            out += fmt[i];
            continue;
        }
        std::size_t j = i + 1;
        while (j < fmt.size() && std::strchr("-+ #0123456789.", fmt[j]))
            ++j;
        if (j == fmt.size())
            break;
        const std::string spec = "%" + fmt.substr(i + 1, j - i - 1) + fmt[j];
        char piece[300] = "";
        const x86::reg32 value = word32(argument);
        switch (fmt[j])
        {
        case 's':
        {
            // Under the viewer's own case what a stack buffer holds past its text is what the frames
            // before left, registers pushed among it, and the top function's registers are not
            // compared: each function's own case sees those strings.
            const std::string text = g_caseAddress == 0x497620 && value >= kStack - 0x10000 ? std::string() : guestString(value, 100);
            g_log.push_back(value);
            std::snprintf(piece, sizeof piece, spec.c_str(), text.c_str());
            for (const char* c = piece; *c; ++c)
                g_log.push_back(x86::reg32(std::uint8_t(*c)));
            argument += 4;
            break;
        }
        case 'd':
        case 'u':
        case 'x':
        case 'X':
        case 'c':
            g_log.push_back(value);
            std::snprintf(piece, sizeof piece, spec.c_str(), int(value));
            argument += 4;
            break;
        case '%':
            piece[0] = '%';
            break;
        default:
            break;
        }
        out += piece;
        i = j;
    }
    return out;
}

/* sub_4df690: as Kind::Sprintf, but all a format's arguments seen and its text right. */
void eaaSprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 buffer = word32(cpu.esp + 4);
    const x86::reg32 formatAt = cpu.esp + 8;
    mock(app, cpu, 0x4df690);
    std::string out = eaaFormat(formatAt);
    if (out.size() > 255)
        out.resize(255);
    std::memcpy(g_memory + buffer, out.c_str(), out.size() + 1);
    cpu.eax = x86::reg32(out.size());
}

/* The console's print: a column, a row, a format and its arguments. */
void eaaConsolePrint(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 formatAt = cpu.esp + 0xc;
    mock(app, cpu, 0x7e040c);
    eaaFormat(formatAt);
}

/* sub_501050, the DirectDraw log: where, a level, a format and its arguments. */
void eaaReport(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 formatAt = cpu.esp + 0xc;
    mock(app, cpu, 0x501050);
    eaaFormat(formatAt);
}

/* sub_4f4b80, fprintf: a file, a format and its arguments. */
void eaaFprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 formatAt = cpu.esp + 8;
    mock(app, cpu, 0x4f4b80);
    eaaFormat(formatAt);
}

/* The console's key: one of those the screens know, Escape often enough to end. */
void eaaKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static const x86::reg32 kKeys[] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x0d, 0x20, 0x43, 0x48, 0x50, 0x53, 0x4800, 0x4900, 0x4b00, 0x4d00, 0x5000, 0x5100, 0x7777 };
    mock(app, cpu, 0x7e0418);
    cpu.eax = kKeys[scripted(sizeof kKeys / sizeof kKeys[0])];
    // The sort's own case walks the header as far as it goes, to its last column.
    if (g_caseAddress == 0x498f00 && scripted(8) != 0)
        cpu.eax = scripted(6) == 0 ? 0x4b00 : 0x4d00;
}

/* The block a viewer is to show: its data. */
void eaaBlockArgument(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4f46b0);
    cpu.eax = 0xa02000 + 0x80 * scripted(24) + 0x10;
}

/* A block's name: of any length. */
void eaaBlockName(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e15e0);
    cpu.eax = 0xa04000 + 0x40 * scripted(4);
}

/* A pack's entries: a few, at least one. */
void eaaPackCount(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4ea654);
    cpu.eax = 1 + scripted(4);
}

/* sub_4ed480: a surface, or none. */
void eaaMakeSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4ed480);
    cpu.eax = scripted(4) == 0 ? 0 : 0xa00510;
}

/* sub_4ed770: the back surface. */
void eaaBackSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4ed770);
    cpu.eax = scripted(4) == 0 ? 0 : 0xa00520;
}

/* DirectDrawCreate: the object through its second argument, or none. */
void eaaDirectDrawCreate(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    mock(app, cpu, 0x533e86);
    setWord32(out, scripted(2) == 0 ? 0 : 0xa00500);
    cpu.eax = scripted(3) == 0 ? 0x88760001 : 0;
}

/* A surface's Lock: well, or lost, or another error. */
void eaaLock(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x7e0764);
    const x86::reg32 pick = scripted(5);
    cpu.eax = pick <= 1 ? 0x887601c2 : pick == 2 ? 0x88760005 : 0;
}

/* QueryInterface: the second interface through its third argument. */
void eaaQueryInterface(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 0xc);
    mock(app, cpu, 0x7e0600);
    setWord32(out, 0xa00500);
    cpu.eax = scripted(4) == 0 ? 0x80004002 : 0;
}

bool eaaCapsClear = false;  // sub_4ebaf0's case: the object gone after GetCaps, for its last test

/* GetCaps of DirectDraw. */
void eaaGetCaps(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x7e062c);
    if (eaaCapsClear && scripted(4) == 0)
    {
        setWord32(0x9f38d0, 0);
        setWord32(0xa05000, 0);
    }
}

/* CreateClipper: the clipper through its third argument. */
void eaaCreateClipper(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 0xc);
    mock(app, cpu, 0x7e0610);
    setWord32(out, 0xa00540);
}
