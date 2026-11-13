// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  These stand in for game functions that need more than kSpecs'
// kinds: what the vector and matrix calls are handed in the frame, recorded, and what they
// and the track's placing leave there for the caller to read back, floats of real sizes
// with now and then a junk one.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

static float rendcNumber(x86::reg32 span, x86::sreg32 from, float over)
{
    if (scripted(16) == 0)
    {
        // Junk, half of it a NaN of its own: the order of an fp:: shows only between two of them.
        const x86::reg32 bits = scripted(2) ? scripted(0) : 0x7fc00000 | scripted(0x400000);
        float junk;
        std::memcpy(&junk, &bits, 4);
        return junk;
    }
    return float(x86::sreg32(scripted(span)) + from) / over;
}

static void rendcRecord(x86::reg32 at, x86::reg32 words)
{
    for (x86::reg32 i = 0; i < words; ++i)
        g_log.push_back(word32(at + 4 * i));
}

/* sub_4dffb0 and sub_4e0000: eax vectors at edx and ebx into ecx. */
static void rendcVectors(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    const x86::reg32 out = cpu.ecx;
    rendcRecord(cpu.edx, 3);
    rendcRecord(cpu.ebx, 3);
    mock(app, cpu, address);
    for (x86::reg32 i = 0; i < 3; ++i)
        setFloat(out + 4 * i, rendcNumber(256, -128, 16.0f));
}

void rendcAddVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    rendcVectors(app, cpu, 0x4dffb0);
}

void rendcSubtractVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    rendcVectors(app, cpu, 0x4e0000);
}

/* sub_4e0280: the matrices at eax and edx into ebx. */
void rendcMultiplyMatrices(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ebx;
    rendcRecord(cpu.eax, 9);
    rendcRecord(cpu.edx, 9);
    mock(app, cpu, 0x4e0280);
    for (x86::reg32 i = 0; i < 9; ++i)
        setFloat(out + 4 * i, rendcNumber(256, -128, 128.0f));
}

/* sub_4e0430: the point at edx through the matrix at ebx and the offset at ecx, to the stack's
 * address: depths around the cones' tests. */
void rendcTransformPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 4);
    rendcRecord(cpu.edx, 3);
    rendcRecord(cpu.ebx, 9);
    rendcRecord(cpu.ecx, 3);
    mock(app, cpu, 0x4e0430);
    for (x86::reg32 i = 0; i < 3; ++i)
        setFloat(out + 4 * i, rendcNumber(1024, -128, 8.0f));
}

/* sub_420650: the point at eax in 16.16 put on the track, in the record at esi. */
void rendcPlaceOnTrack(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 point = cpu.eax, record = cpu.esi;
    rendcRecord(point, 3);
    mock(app, cpu, 0x420650);
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(point + 4 * i, scripted(0x1000000) - 0x800000);
    setWord32(record + 0x6c, scripted(3) == 0 ? scripted(0) : 0);
}

/* sub_4e1620: memory for a row of cones. */
void rendcReserveMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e1620);
    cpu.eax = 0xb01000 + 0x1000 * scripted(2);
}

/* sub_4bbde0: memory for an object, its address at eax. */
void rendcObjectsAllocate(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 where = cpu.eax;
    mock(app, cpu, 0x4bbde0);
    setWord32(where, 0xb0c000 + 0x200 * scripted(32));
}
