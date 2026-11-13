// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* The two the generated code goes on to by name at its end (`return nfs3hp::...`). */
void fillWord(win32::WinApplication* app, x86::CPU& cpu) { app->dynamic_call(0x4e0721, cpu); }
namespace port { void runSystemTasks(win32::WinApplication* app, x86::CPU& cpu) { app->dynamic_call(0x4e7630, cpu); } }

/* What the cases of wina have in guest memory, in what its watched range holds. */
constexpr x86::reg32 kWinaStream = 0xc00000, kWinaChannels = kWinaStream + 0x640, kWinaFiles = kWinaStream + 0x170;
constexpr x86::reg32 kWinaBadHandle = 0xc0a000, kWinaSlots = 0xc0b000, kWinaScanTable = 0xc0be00, kWinaDriver = 0xc0bf00;
constexpr x86::reg32 kWinaFileTable = 0xc10000, kWinaQueue = 0xc10200, kWinaOp = 0xc10400, kWinaOut = 0xc10800;
constexpr x86::reg32 kWinaTextPlace = 0xc10900, kWinaText = 0xc10910, kWinaSurface = 0xc10a00, kWinaTable = 0xc10b00;
constexpr x86::reg32 kWinaName = 0xc10c00, kWinaNameBar = 0xc10c20, kWinaShape = 0xc11000, kWinaPixels = 0xc11400;
constexpr x86::reg32 kWinaFrames = 0xc11800, kWinaCom = 0xc12000, kWinaVtable = 0xc12100, kWinaBlock = 0xc13000;
// Functions that are not there, for what the game calls through a pointer.
constexpr x86::reg32 kWinaCreate = 0x7e0200, kWinaFn = 0x7e0210, kWinaFn2 = 0x7e0214, kWinaFn3 = 0x7e0218, kWinaFn0 = 0x7e021c;
constexpr x86::reg32 kWinaHandler = 0x7e0220, kWinaPriority = 0x7e0230, kWinaQuery = 0x7e0240, kWinaRelease = 0x7e0250;
constexpr x86::reg32 kWinaMakeSurface = 0x7e0260, kWinaCoop = 0x7e0270;

/* A function of the group's in another's case: recorded, not run; in its own, itself. */
template <x86::reg32 Address>
void winaStand(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_caseAddress == Address)
        g_generatedMethods[Address](app, cpu);
    else
        mock(app, cpu, Address);
}

/* A stand-in whose answer is read through: 0 now and then, else one of what the world has. */
template <x86::reg32 Address>
void winaOp(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_caseAddress == Address)
    {
        g_generatedMethods[Address](app, cpu);
        return;
    }
    mock(app, cpu, Address);
    cpu.eax = scripted(5) == 0 ? 0 : kWinaOp;
}

void winaTakeFree(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_caseAddress == 0x4cf280)
    {
        g_generatedMethods[0x4cf280](app, cpu);
        return;
    }
    mock(app, cpu, 0x4cf280);
    cpu.eax = scripted(4) == 0 ? 0 : kWinaFiles + 0x128 * scripted(4);
}

void winaCloseFile(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_caseAddress == 0x4d0060)
    {
        g_generatedMethods[0x4d0060](app, cpu);
        return;
    }
    mock(app, cpu, 0x4d0060);
    if (word32(kWinaStream + 0x54) != 0)
        setWord32(word32(kWinaStream + 0x54) + 4, 4);
}

void winaNextChunk(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_caseAddress == 0x4d0300)
    {
        g_generatedMethods[0x4d0300](app, cpu);
        return;
    }
    mock(app, cpu, 0x4d0300);
    cpu.eax = scripted(4) == 0 ? 0 : kWinaFrames + 4 * scripted(4);
}

void winaFindOp(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 pending = cpu.ebx;
    mock(app, cpu, 0x4fbd10);
    setWord32(pending, scripted(2));
    cpu.eax = scripted(4) == 0 ? 0 : kWinaOp;
}

void winaOpFromFree(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50df60);
    cpu.eax = scripted(5) == 0 ? 0 : kWinaOp;
}

void winaOpByPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50e5a0);
    cpu.eax = scripted(3) == 0 ? 0 : kWinaOp;
}

void winaNameCopy(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x501110);
    cpu.eax = kWinaOut;
}

void winaBsearch(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4fb300);
    cpu.eax = scripted(2) == 0 ? 0 : 0x7f7060 + 8 * scripted(3);
}

/* The loops that wait: what they wait for comes, sooner or later. */
template <x86::reg32 Address>
void winaWaiting(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    if (g_caseAddress == 0x4cfd30 && scripted(3) == 0)
        setWord32(kWinaStream + 0x28, 0);
    if (g_caseAddress == 0x4cdaa0 && scripted(3) == 0)
        setWord32(0x9f3d24, 0);
    if (g_caseAddress == 0x4cd6c0)
    {
        if (word32(0x56126c) == 0)
        {
            if (scripted(2) == 0)
                setWord32(0x56126c, 0x77);
        }
        else if (scripted(2) == 0)
        {
            setWord32(0x56126c, 0);
        }
    }
}

x86::reg32 g_winaPeek = 0, g_winaProc = 0, g_winaVersion = 0;  // what the import table holds

/* The message pump: the window goes in the end. */
void winaPeek(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, g_winaPeek);
    if (scripted(3) == 0)
        setWord32(0x561270, 1);
}

void winaProcAddress(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, g_winaProc);
    cpu.eax = scripted(5) == 0 ? 0 : kWinaCreate;
}

void winaVersionInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = word32(cpu.esp + 4);
    mock(app, cpu, g_winaVersion);
    setWord32(at + 0x10, scripted(3));
    setWord32(at + 4, scripted(7));
}

void winaQueryValue(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 value = word32(cpu.esp + 12), length = word32(cpu.esp + 16);
    mock(app, cpu, 0x533e8c);
    setWord32(value, kWinaText);
    setWord32(length, scripted(6));
}

/* The objects of DirectDraw: a method's answer, the object where one is asked for. */
void winaComCreate(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    mock(app, cpu, kWinaCreate);
    setWord32(out, kWinaCom);
    cpu.eax = scripted(5) == 0 ? 0x80000000 : 0;
}

void winaComQuery(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 12);
    mock(app, cpu, kWinaQuery);
    setWord32(out, kWinaCom);
    cpu.eax = scripted(5) == 0 ? 0x80000000 : 0;
}

void winaComMakeSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 12);
    mock(app, cpu, kWinaMakeSurface);
    setWord32(out, kWinaCom);
    cpu.eax = scripted(5) == 0 ? 0x80000000 : 0;
}

void winaComCoop(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kWinaCoop);
    cpu.eax = scripted(5) == 0 ? 0x80000000 : 0;
}

void winaPriorityOf(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kWinaPriority);
    cpu.eax = scripted(5);
}

/* sub_4ccaa0's pieces: its generated code ends in them.  Each has the frame's ebp to pop. */
template <x86::reg32 Value>
void winaKeyPiece(win32::WinApplication*, x86::CPU& cpu)
{
    cpu.eax = Value;
    cpu.ebp = word32(cpu.esp);
    cpu.esp += 8;
}

void winaKeyPieceLow(win32::WinApplication*, x86::CPU& cpu)
{
    // 0x4ccbb7: below 0x24.
    cpu.eax = cpu.edx == 0x21 ? 0x4900 : cpu.edx == 0x22 ? 0x5100 : cpu.edx == 0x23 ? 0x4f00 : 0;
    cpu.ebp = word32(cpu.esp);
    cpu.esp += 8;
}

/* sub_4fb470's pieces, the rest of it as the game has it from each: the generated code
 * goes to them when the place is outside the clip, and they do not come back. */
void winaPointerRest(win32::WinApplication* app, x86::CPU& cpu, int from)
{
    const auto signedLess = [](x86::reg32 a, x86::reg32 b) { return x86::sreg32(a) < x86::sreg32(b); };
    const auto lockCall = [&](x86::reg32 slot, x86::reg32 lock) {
        cpu.esp -= 4;
        setWord32(cpu.esp, lock);
        cpu.esp -= 4;
        app->dynamic_call(word32(slot), cpu);
    };
    switch (from)
    {
    case 0: goto heldX;
    case 1: goto clampY;
    case 2: goto locked;
    case 3: goto move;
    case 4: goto compareLast;
    case 5: goto keepLast;
    case 6: goto unlock;
    default: goto tell;
    }
heldX:
    cpu.edx = word32(0x9f4f50);
    if (!signedLess(cpu.edx, cpu.edi))
    {
        cpu.ebx = cpu.edx;
        goto clampY;
    }
    cpu.ebx = cpu.edi;
clampY:
    cpu.ecx = word32(0x9f4f40);
    if (!signedLess(cpu.ebx, cpu.ecx))
    {
        cpu.ebx = cpu.ecx;
        goto locked;
    }
    cpu.ebx = word32(0x9f4f50);
    if (signedLess(cpu.ebx, cpu.edi))
        cpu.ebx = cpu.edi;
locked:
    cpu.eax = word32(0x561290);
    lockCall(0x5344b8, cpu.eax);
    cpu.eax = word32(cpu.esp + 4);
    setWord32(cpu.esp + 8, cpu.ebx);
    setWord32(0x9f4f54, cpu.eax);
    if (cpu.ebp == cpu.esi && cpu.ebx == cpu.edi)
        goto compareLast;
move:
    cpu.ebx = word32(cpu.esp + 8);
    cpu.eax = 0x9f38d0;
    cpu.edx = cpu.ebp;
    cpu.esp -= 4;
    app->dynamic_call(0x4cc890, cpu);
compareLast:
    if (cpu.ebp == word32(0x9f4f3c))
    {
        cpu.eax = word32(cpu.esp + 8);
        if (cpu.eax == word32(0x9f4f38))
            goto unlock;
    }
keepLast:
    cpu.eax = word32(cpu.esp + 8);
    setWord32(0x9f4f3c, cpu.ebp);
    setWord32(0x9f4f38, cpu.eax);
unlock:
    cpu.ebx = word32(0x561290);
    cpu.esi = word32(cpu.esp);
    lockCall(0x534588, cpu.ebx);
    if (cpu.esi == 0 || word32(0x56446c) == 0)
        goto end;
tell:
    cpu.eax = word32(cpu.esp + 4);
    cpu.esp -= 4;
    app->dynamic_call(word32(0x56446c), cpu);
end:
    cpu.esp += 0xc;
    cpu.ebp = word32(cpu.esp);
    cpu.edi = word32(cpu.esp + 4);
    cpu.esi = word32(cpu.esp + 8);
    cpu.esp += 16;
}

void winaPointer6a0(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ebp = cpu.eax;
    winaPointerRest(app, cpu, 0);
}

void winaPointer6a7(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ebx = cpu.edx;
    winaPointerRest(app, cpu, 1);
}

void winaPointer6ae(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ebx = cpu.ecx;
    winaPointerRest(app, cpu, 2);
}

void winaPointer6b5(win32::WinApplication* app, x86::CPU& cpu)
{
    winaPointerRest(app, cpu, cpu.ebx != cpu.edi ? 3 : 4);
}

void winaPointer6c2(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = word32(cpu.esp + 8);
    winaPointerRest(app, cpu, cpu.eax != word32(0x9f4f38) ? 5 : 6);
}

void winaPointer6d7(win32::WinApplication* app, x86::CPU& cpu)
{
    winaPointerRest(app, cpu, 7);
}
