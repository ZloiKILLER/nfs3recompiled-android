// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// And for game functions whose results these functions read through: what they hand back and what they write.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* sub_4dbcc0's patch (tools/apply_view_angle.py): the port's View Angle, recorded. */
x86::reg32 viewAngleFor(win32::WinApplication*, x86::reg32 angle)
{
    g_log.push_back(0x7e4a0000 | angle);
    return word32(0x6fbc40) == 2 ? 105u : angle;
}

/* A block of the frame's arena, as the renderer's allocator hands it out, or none. */
constexpr x86::reg32 kRendeArena = 0x920000, kRendeArenaBytes = 0x8000;

x86::reg32 rendeBlock()
{
    return kRendeArena + 0x20 * scripted(kRendeArenaBytes / 0x20);
}

/* sub_4bbde0: eax where the block goes. */
void rendeAllocate(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 where = cpu.eax;
    mock(app, cpu, 0x4bbde0);
    setWord32(where, rendeBlock());
    cpu.eax = scripted(5) != 0 ? 1 : 0;
}

/* sub_4bbf80: likewise, 0xa0 bytes on the list. */
void rendeAllocateListed(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 where = cpu.eax;
    mock(app, cpu, 0x4bbf80);
    setWord32(where, rendeBlock());
    cpu.eax = scripted(5) != 0 ? 1 : 0;
}

/* sub_4bbe70: an empty list through esi. */
void rendeEmptyList(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 list = cpu.esi;
    mock(app, cpu, 0x4bbe70);
    setWord32(list, 0);
    setWord32(list + 4, 0);
    cpu.eax = list;
}

/* sub_4c2cd0: the polygon clipper: its three corners and their codes recorded, as it reads them. */
void rendeClip(win32::WinApplication* app, x86::CPU& cpu)
{
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g_log.push_back(word32(cpu.eax + 4 * i));
        g_log.push_back(g_memory[cpu.edx + i]);
    }
    const x86::reg32 codes = cpu.edx;
    mock(app, cpu, 0x4c2cd0);
    // It may leave the codes changed: the callers read them after.
    if (scripted(4) == 0)
        g_memory[codes + scripted(4)] = x86::reg8(scripted(3) ? 0 : scripted(256));
}

/* sub_4c6c70: a segment's two sides, in the frame, recorded. */
void rendePutSegment(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 sides : { cpu.ebx, cpu.ecx })
        for (x86::reg32 at = 0; at < 0x18; at += 4)
            g_log.push_back(word32(sides + at));
    mock(app, cpu, 0x4c6c70);
}

/* sub_4d3f50: sets the flag at edx, its own palette or not. */
void rendeSquareTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 flag = cpu.edx;
    mock(app, cpu, 0x4d3f50);
    setWord32(flag, scripted(2));
}

/* sub_423b00: a view's camera, small numbers. */
void rendeCameraMode(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x423b00);
    cpu.eax = scripted(4) == 0 ? scripted(0) : scripted(7) - 1;
}

/* sub_4e0160: the cross product, here floats of real sizes at ebx; its two vectors recorded. */
void rendeCross(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ebx;
    for (x86::reg32 at = 0; at < 12; at += 4)
    {
        g_log.push_back(word32(cpu.eax + at));
        g_log.push_back(word32(cpu.edx + at));
    }
    mock(app, cpu, 0x4e0160);
    for (x86::reg32 at = 0; at < 12; at += 4)
        setFloat(out + at, float(x86::sreg32(scripted(2000)) - 1000) / 256.0f);
}

/* sub_4bf4c0: the corners projected to the stack's place: depth around what is too far,
 * and clip codes, often none. */
void rendeProject(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 count = cpu.eax, out = word32(cpu.esp + 4);
    mock(app, cpu, 0x4bf4c0);
    const x86::reg32 corners = count < 0x100 ? count : 0x100;
    for (x86::reg32 i = 0; i < corners; ++i)
    {
        const x86::reg32 v = out + i * 0x20;
        setFloat(v + 8, float(scripted(300)) + 0.5f);
        g_memory[v + 0x14] = x86::reg8(scripted(3) ? 0 : scripted(16));
    }
}
