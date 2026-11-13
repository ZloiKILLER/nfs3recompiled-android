// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

constexpr x86::reg32 kWatcThreadData = 0xb00000, kWatcFileInfo = 0xb0a000;
constexpr x86::reg32 kWatcWinProc = 0x7e05a0, kWatcTapiProcs = 0x7e0600;

/* The library's report: what its format takes, recorded too. */
void watcStandInLog(win32::WinApplication* app, x86::CPU& cpu)
{
    unsigned arguments = 0;
    for (const char c : guestString(word32(cpu.esp + 4)))
        arguments += c == '%' ? 1 : 0;
    for (unsigned i = 0; i < arguments; ++i)
        g_log.push_back(word32(cpu.esp + 8 + 4 * i));
    mock(app, cpu, 0x401010);
}

/* nmalloc, nrealloc, calloc: memory of the world's, now and then none. */
template <x86::reg32 Address>
void watcStandInAlloc(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    cpu.eax = scripted(8) == 0 ? 0 : 0xe00000 + 0x400 * (g_calls & 0xff);
}

/* Windows' thread id: one of few, so that a lock is sometimes already this thread's. */
void watcStandInThreadId(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = word32(0x53450c);
    mock(app, cpu, slot);
    cpu.eax = scripted(3);
}

/* TlsGetValue: a thread's data, or none yet. */
void watcStandInTlsGet(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = word32(0x534614);
    mock(app, cpu, slot);
    cpu.eax = scripted(3) == 0 ? 0 : kWatcThreadData;
}

/* TlsAlloc: an index, the low ones often, now and then none. */
void watcStandInTlsAlloc(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = word32(0x53460c);
    mock(app, cpu, slot);
    cpu.eax = scripted(5) == 0 ? 0xffffffffu : scripted(6);
}

/* GetProcAddress: nothing, or a function that takes what its stub is given. */
void watcStandInProc(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = word32(0x534558);
    mock(app, cpu, slot);
    x86::reg32 proc = kWatcWinProc;
    switch (g_caseAddress)
    {
    case 0x52f661: case 0x52f969: case 0x5324d2: proc = kWatcTapiProcs + 0x10; break;
    case 0x530b4d: proc = kWatcTapiProcs + 0x20; break;
    case 0x52f535: case 0x52fb2e: case 0x53083d: case 0x532162: proc = kWatcTapiProcs + 0x30; break;
    case 0x531f31: proc = kWatcTapiProcs + 0x40; break;
    case 0x530747: case 0x5310be: case 0x5311c0: proc = kWatcTapiProcs + 0x50; break;
    case 0x530a4e: case 0x53138f: proc = kWatcTapiProcs + 0x60; break;
    case 0x531439: proc = kWatcTapiProcs + 0x90; break;
    default: break;
    }
    cpu.eax = scripted(4) == 0 ? 0 : proc;
}

/* GetCPInfo: a code page's lead bytes, ranges ended by 0, 0. */
void watcStandInCpInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = word32(0x5344e4);
    const x86::reg32 info = word32(cpu.esp + 8);
    mock(app, cpu, slot);
    const x86::reg32 ranges = scripted(4);
    for (x86::reg32 i = 0; i < 12; ++i)
        g_memory[info + 6 + i] = 0;
    for (x86::reg32 i = 0; i < ranges; ++i)
    {
        const x86::reg32 low = 0x80 + scripted(0x40);
        g_memory[info + 6 + 2 * i] = x86::reg8(low);
        g_memory[info + 7 + 2 * i] = x86::reg8(low + scripted(8));
    }
    cpu.eax = scripted(4) == 0 ? 0 : 1;
}

/* The thread data's getter, through its hook. */
void watcStandInGetter(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x7e0510);
    cpu.eax = kWatcThreadData;
}

/* What signal asks of a code: 0 to 5. */
void watcStandInAsked(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x7e0530);
    cpu.eax = scripted(6);
}

/* raise: its handler has or has not returned from the exception. */
void watcStandInRaise(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x7e0540);
    g_memory[0xa0ba94] = x86::reg8(scripted(2));
}

/* A thread's data made: the one the world has, or none. */
void watcStandInMakeData(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x51e7ec);
    cpu.eax = scripted(4) == 0 ? 0 : kWatcThreadData;
}

/* getenv: C_FILE_INFO or nothing. */
void watcStandInGetenv(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50ead0);
    cpu.eax = scripted(4) == 0 ? 0 : kWatcFileInfo;
}

void watcStandInMbslen(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x518050);
    cpu.eax = scripted(8);
}

void watcStandInToupper(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 c = cpu.eax;
    mock(app, cpu, 0x4eede0);
    cpu.eax = c >= 'a' && c <= 'z' ? c - 0x20 : c;
}

void watcStandInTolower(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 c = cpu.eax;
    mock(app, cpu, 0x4f0ae0);
    cpu.eax = c >= 'A' && c <= 'Z' ? c + 0x20 : c;
}

/* The multibyte functions, as far as _mbschr needs them. */
void watcStandInMbvtop(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 c = cpu.eax, to = cpu.edx;
    mock(app, cpu, 0x52ede0);
    g_memory[to] = x86::reg8(c);
    cpu.eax = to;
}

void watcStandInMbclen(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x52b5b0);
    cpu.eax = 1;
}

void watcStandInMbterm(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    mock(app, cpu, 0x5250c0);
    cpu.eax = g_memory[at] == 0 ? 1 : 0;
}

void watcStandInMbccmp(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax, other = cpu.edx;
    mock(app, cpu, 0x52e3a0);
    cpu.eax = g_memory[at] == g_memory[other] ? 0 : 1;
}

void watcStandInMbsinc(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    mock(app, cpu, 0x525100);
    cpu.eax = at + 1;
}

/* The mono driver's, jumped to by name from these functions: the extraction turns only calls into
 * calls by address. */
namespace ea
{
void monoClear(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e0ca0);
}
void monoSetAttribute(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e0d50);
}
}

/* sub_52b8d0, sub_52bb20, sub_52b8f0 as generated, whose only jump goes by name to a native of another
 * group: by address here, as the native's. */
void watcGeneratedViewerOnWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = 0x56b170;
    mock(app, cpu, 0x497620);
}
void watcGeneratedViewerOnMono(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = 0x56b198;
    mock(app, cpu, 0x497620);
}
void watcGeneratedWindowEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4ead90);
}
