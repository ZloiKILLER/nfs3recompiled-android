// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  These stand in for game functions that need more than
// kSpecs' kinds: what they are handed lives in the caller's frame, which nothing else looks
// at, so it is recorded; what they give back is read through the same frame, so it is
// written, real arithmetic where that keeps the values real.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

constexpr x86::reg32 kRendaPool = 0x7e4000;     // the polygons of 0xa0 bytes the list hands out
constexpr x86::reg32 kRendaPoolRecords = 24;
constexpr x86::reg32 kRendaNaN = 0x7fc12345;    // the one NaN of a world of NaNs (renda_world.inc)
bool g_rendaNaNs = false;

/* A float a stand-in works out, written: in a world of NaNs any NaN as that one. */
void rendaPut(x86::reg32 at, float value)
{
    if (g_rendaNaNs && value != value)
        setWord32(at, kRendaNaN);
    else
        setFloat(at, value);
}

void rendaLogFloats(x86::reg32 at, x86::reg32 words)
{
    for (x86::reg32 i = 0; i < words; ++i)
        g_log.push_back(word32(at + 4 * i));
}

float rendaFloat(x86::reg32 at)
{
    float value;
    std::memcpy(&value, g_memory + at, 4);
    return value;
}

/* A float of the script's: real magnitudes, of either sign. */
float rendaScriptFloat()
{
    return (float(scripted(4001)) - 2000.0f) / 8.0f;
}

/* sub_4e0000 and sub_4dffb0: eax vectors at edx and ebx, to ecx. */
template <x86::reg32 Address, int Sign>
void rendaVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 count = std::min<x86::reg32>(cpu.eax, 16), a = cpu.edx, b = cpu.ebx, out = cpu.ecx;
    g_log.push_back(out);
    rendaLogFloats(a, count * 3);
    rendaLogFloats(b, count * 3);
    mock(app, cpu, Address);
    for (x86::reg32 i = 0; i < count * 3; ++i)
        rendaPut(out + 4 * i, rendaFloat(a + 4 * i) + float(Sign) * rendaFloat(b + 4 * i));
}

/* sub_4e0050: eax vectors at edx times the float on the stack, to the next word. */
void rendaScale(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 count = std::min<x86::reg32>(cpu.eax, 16), in = cpu.edx;
    const float factor = rendaFloat(cpu.esp + 4);
    const x86::reg32 out = word32(cpu.esp + 8);
    rendaLogFloats(in, count * 3);
    mock(app, cpu, 0x4e0050);
    for (x86::reg32 i = 0; i < count * 3; ++i)
        rendaPut(out + 4 * i, rendaFloat(in + 4 * i) * factor);
}

/* sub_4e01c0: the vector at eax made a unit, to edx. */
void rendaUnit(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 in = cpu.eax, out = cpu.edx;
    rendaLogFloats(in, 3);
    mock(app, cpu, 0x4e01c0);
    const float x = rendaFloat(in), y = rendaFloat(in + 4), z = rendaFloat(in + 8);
    const float length = std::sqrt(x * x + y * y + z * z);
    rendaPut(out, x / length);
    rendaPut(out + 4, y / length);
    rendaPut(out + 8, z / length);
}

/* sub_4e01a0: the unit cross of the vectors at eax and edx, to ebx. */
void rendaCross(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 a = cpu.eax, b = cpu.edx, out = cpu.ebx;
    rendaLogFloats(a, 3);
    rendaLogFloats(b, 3);
    mock(app, cpu, 0x4e01a0);
    const float x = rendaFloat(a + 4) * rendaFloat(b + 8) - rendaFloat(a + 8) * rendaFloat(b + 4);
    const float y = rendaFloat(a + 8) * rendaFloat(b) - rendaFloat(a) * rendaFloat(b + 8);
    const float z = rendaFloat(a) * rendaFloat(b + 4) - rendaFloat(a + 4) * rendaFloat(b);
    const float length = std::sqrt(x * x + y * y + z * z);
    rendaPut(out, x / length);
    rendaPut(out + 4, y / length);
    rendaPut(out + 8, z / length);
}

/* What sin, cos, atan and a root leave on the x87: the full width the x87 does not round,
 * wider than a float, for what is done with it to show. */
void rendaPushWide(x86::CPU& cpu)
{
    if (scripted(2) == 0 && g_rendaNaNs)
    {
        float nan;
        std::memcpy(&nan, &kRendaNaN, 4);
        cpu.fpu.push(x86::Float(nan));
        return;
    }
    cpu.fpu.push(x86::Float((double(scripted(200001)) - 100000.0) / 3000.0));
}

template <x86::reg32 Address>
void rendaOnX87(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    rendaPushWide(cpu);
}

/* sub_4e06a0: the length of the vector at eax, on the x87: what it was handed recorded. */
void rendaLength(win32::WinApplication* app, x86::CPU& cpu)
{
    rendaLogFloats(cpu.eax, 3);
    mock(app, cpu, 0x4e06a0);
    rendaPushWide(cpu);
}

/* sub_4964d0: the float at eax between 0 and 1, as the real one leaves it: the rings
 * index their tables with a sixteenth of it. */
void rendaFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    g_log.push_back(word32(at));
    mock(app, cpu, 0x4964d0);
    const float value = rendaFloat(at);
    setFloat(at, value == value && value - value == 0.0f ? value - std::floor(value) : 0.0f);
}

/* sub_4bf370: eax points at edx as vertices at ebx: their places and codes the script's. */
void rendaScreenVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 count = std::min<x86::reg32>(cpu.eax, 16), in = cpu.edx, out = cpu.ebx;
    rendaLogFloats(in, count * 3);
    mock(app, cpu, 0x4bf370);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        const x86::reg32 vertex = out + 0x20 * i;
        setFloat(vertex, rendaScriptFloat());
        setFloat(vertex + 4, rendaScriptFloat());
        setFloat(vertex + 8, rendaScriptFloat());
        setFloat(vertex + 12, rendaScriptFloat());
        g_memory[vertex + 0x14] = x86::reg8(scripted(3) == 0 ? scripted(32) : 1 << scripted(4));
    }
}

/* sub_4bf4c0: eax points at edx, by the matrix at ebx and the shift at ecx: recorded. */
void rendaObjectVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    rendaLogFloats(cpu.edx, std::min<x86::reg32>(cpu.eax, 16) * 3);
    rendaLogFloats(cpu.ecx, 3);
    mock(app, cpu, 0x4bf4c0);
}

/* A side of the view's rectangle: often just on the screen's edges, the clip codes' limits. */
x86::reg32 rendaViewSide(x86::reg32 most)
{
    const x86::reg32 pick = scripted(3);
    if (pick == 0)
        return scripted(2) * most;
    return scripted(most + 1);
}

/* sub_4beb80: the view's rectangle, to the four addresses. */
void rendaViewport(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 x = cpu.eax, y = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    mock(app, cpu, 0x4beb80);
    setWord32(x, rendaViewSide(640));
    setWord32(y, rendaViewSide(480));
    setWord32(width, rendaViewSide(640));
    setWord32(height, rendaViewSide(480));
}

/* sub_4bbf80: a record of the pool's to [eax], 1; now and then none, 0. */
void rendaListAdd(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 slot = cpu.eax;
    mock(app, cpu, 0x4bbf80);
    if (scripted(8) == 0)
    {
        cpu.eax = 0;
        return;
    }
    setWord32(slot, kRendaPool + 0xa0 * scripted(kRendaPoolRecords));
    cpu.eax = 1;
}

/* sub_4bbfc0: the list's two ends, to esi. */
void rendaListEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.esi;
    mock(app, cpu, 0x4bbfc0);
    setWord32(out, scripted(0));
    setWord32(out + 4, scripted(0));
}

/* sub_4bc030: what it keeps, from [edx]. */
void rendaListKeep(win32::WinApplication* app, x86::CPU& cpu)
{
    rendaLogFloats(cpu.edx, 2);
    mock(app, cpu, 0x4bc030);
}

/* sub_4217e0: the track's node, around the tunnel's half the time. */
void rendaCameraNode(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4217e0);
    if (scripted(2) == 0)
        cpu.eax = 0x386 + scripted(0xa8);
}

/* sub_494390: the newest flash, its place to eax and brightness to edx; 1 if there is one. */
void rendaNewestFlash(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.eax, brightness = cpu.edx;
    mock(app, cpu, 0x494390);
    if (scripted(3) == 0)
    {
        cpu.eax = 0;
        return;
    }
    // Round the grid's thirteen cells and past them on both sides.
    setFloat(place, float(scripted(9000)) - 4500.0f);
    setFloat(place + 4, rendaScriptFloat());
    setFloat(place + 8, float(scripted(9000)) - 4500.0f);
    setWord32(brightness, scripted(0));
    cpu.eax = 1;
}
