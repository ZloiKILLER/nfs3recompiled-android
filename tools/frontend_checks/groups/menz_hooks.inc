// Recording stand-ins for what these functions read through: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU& cpu, x86::reg32 address);

constexpr x86::reg32 kMenzDisplay = 0xa0f000, kMenzSurface = 0xa00510;
constexpr x86::reg32 kMenzComBase = 0x7e0500, kMenzAllocate = 0x7e0614, kMenzOps = 0xa0c000;

/* IDirectDraw::CreateSurface: the surface through its third argument, or none. */
void menzCreateSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 0xc);
    mock(app, cpu, kMenzComBase + 0x18);
    setWord32(out, scripted(4) == 0 ? 0 : kMenzSurface);
    cpu.eax = scripted(4) == 0 ? 0x10005 : 0x10000;
}

/* GetSurfaceDesc: the bits a pixel that choose a way, in what it fills. */
void menzSurfaceDesc(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    mock(app, cpu, kMenzComBase + 0x58);
    static const x86::reg32 kBits[] = { 8, 4, 0x10, 0x18 };
    setWord32(out + 0x54, kBits[scripted(4)]);
    cpu.eax = scripted(4) == 0 ? 1 : 0x10000;
}

/* A surface locked: 0 now and then, its bits where they were half of the time. */
void menzLock(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 desc = cpu.ebx;
    mock(app, cpu, 0x4eb5e0);
    cpu.eax = scripted(3) == 0 ? 1 : 0;
    setWord32(desc + 0x24, scripted(2) ? word32(kMenzDisplay + 0x440) : 0x1234);
}

void menzGetLastError(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, word32(0x534534));
    cpu.eax = scripted(2) ? 0xb7 : 0;
}

void menzGetKeyboardType(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 which = word32(cpu.esp + 4);
    mock(app, cpu, word32(0x534748));
    if (which == 0)
        cpu.eax = scripted(3) == 0 ? 4 : 7;
    else
        cpu.eax = 0xd00 + scripted(6);
}

/* The driver's allocation: 0 now and then, or memory of the cases'. */
void menzAllocate(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 bytes = cpu.eax;
    mock(app, cpu, kMenzAllocate);
    // Only a font says what it does without; the video buffers' list is written at once.
    cpu.eax = bytes == 0xe0 && scripted(6) == 0 ? 0 : 0xa03000 + scripted(4) * 0x400;
}

/* An operation of the list, or none. */
void menzListFind(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50e3f0);
    cpu.eax = scripted(3) == 0 ? 0 : kMenzOps + scripted(8) * 0x40;
}

void menzListFetch(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50e5a0);
    cpu.eax = scripted(16) == 0 ? 0 : kMenzOps + scripted(8) * 0x40;
}

void menzCountBits(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4eb880);
    cpu.eax = scripted(9);
}

/* While an operation is waited for, the device's thread finishes the one it has. */
void menzDevicesGoOn()
{
    for (x86::reg32 i = 0; i < 32; ++i)
        setWord32(0xa0a000 + i * 0x68 + 0x18, 0);
}

void menzSleep(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4df8e0);
    menzDevicesGoOn();
}

void menzWaitEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4dfc60);
    menzDevicesGoOn();
}
