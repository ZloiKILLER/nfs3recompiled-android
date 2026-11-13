// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* Only watches, in the port (native_vertices.cpp); nothing to see. */
bool arenaReset(win32::WinApplication*, x86::CPU&)
{
    return false;
}

/* The frame the wheels last moved: this frame now and then, else a few before, or the wheels
 * of generated code turn for very long (the wrap loops step by 1 and 2). */
x86::reg32 carWheelStamp(win32::WinApplication*, x86::reg32 car, x86::reg32 which)
{
    g_log.push_back(0x7e4a0000 | which);
    g_log.push_back(car);
    return word32(0x7d3684) - scripted(6);
}

/* These stand in for game functions that need more than kSpecs' kinds: what they are handed
 * by address recorded, what they hand back by address or on the x87. */
namespace rendd
{

/* Their rows under an address of their own: a function of another group is otherwise that
 * group's plain stand-in in this process, whatever its world asks (hasStandIn). */
constexpr x86::reg32 kOwn = 0x7e000000;

float realFloat()
{
    static const float kSpecial[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.25f, 0.3f, 2.0f, 255.0f, 1.0e-3f, 40.0f };
    const x86::reg32 pick = scripted(8);
    if (pick == 0)
        return kSpecial[scripted(std::size(kSpecial))];
    if (pick <= 2)
        return float(x86::sreg32(scripted(4000)) - 2000) / 1024.0f;
    // All of a float's bits, or the sums and products would have nothing to round.
    return float(x86::sreg32(scripted(0x2000000)) - 0x1000000) / 65536.0f;
}

/* What a function left on the x87: now and then with more bits than a float has. */
x86::Float x87Result()
{
    const double value = realFloat();
    return scripted(3) == 0 ? value * (1.0 + double(1 + scripted(1000)) * 1.0e-10) : value;
}

void logDwords(x86::reg32 at, x86::reg32 count)
{
    for (x86::reg32 i = 0; i < count; ++i)
        g_log.push_back(word32(at + 4 * i));
}

void writeFloats(x86::reg32 at, x86::reg32 count)
{
    for (x86::reg32 i = 0; i < count; ++i)
        setFloat(at + 4 * i, realFloat());
}

void dot(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.eax, 3);
    logDwords(cpu.edx, 3);
    mock(app, cpu, kOwn | 0x4e01f0);
    cpu.fpu.push(x87Result());
}

void normalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 vector = cpu.eax;
    logDwords(vector, 3);
    mock(app, cpu, kOwn | 0x4972f0);
    writeFloats(vector, 3);
    cpu.fpu.push(x87Result());
}

void distance(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.eax, 3);
    logDwords(cpu.edx, 3);
    mock(app, cpu, kOwn | 0x4965b0);
    // Close and not, against the 0.1 or so these compare with.
    cpu.fpu.push(scripted(2) ? x86::Float(double(scripted(1000)) / 4096.0) : x87Result());
}

template <x86::reg32 Address>
void onX87(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kOwn | Address);
    cpu.fpu.push(x87Result());
}

void modelPart(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.ebx != 0)
        logDwords(cpu.ebx, 9);
    logDwords(word32(cpu.esp + 4), 9);
    logDwords(word32(cpu.esp + 8), 3);
    logDwords(word32(cpu.esp + 12), 3);
    logDwords(word32(cpu.esp + 16), 3);
    mock(app, cpu, kOwn | 0x49de20);
    cpu.fpu.push(x87Result());
}

void transformPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 count = cpu.eax < 8 ? cpu.eax : 8, out = word32(cpu.esp + 4);
    logDwords(cpu.edx, 3 * count);
    logDwords(cpu.ebx, 9);
    logDwords(cpu.ecx, 3);
    mock(app, cpu, kOwn | 0x4e0430);
    writeFloats(out, 3 * count);
}

template <x86::reg32 Address>
void twoVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ecx;
    logDwords(cpu.edx, 3);
    logDwords(cpu.ebx, 3);
    mock(app, cpu, kOwn | Address);
    writeFloats(out, 3);
}

void scaleVector(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    logDwords(cpu.edx, 3);
    mock(app, cpu, kOwn | 0x4e0050);
    writeFloats(out, 3);
}

void normaliseTo(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    logDwords(cpu.eax, 3);
    mock(app, cpu, kOwn | 0x4e01c0);
    writeFloats(out, 3);
}

void dotWithSun(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ecx;
    logDwords(cpu.edx, 3);
    logDwords(cpu.ebx, 3);
    mock(app, cpu, kOwn | 0x4e0210);
    setFloat(out, realFloat());
}

/* The track under a corner: on a polygon or not, of a surface or not, a little higher or not. */
void trackPoint(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 point = cpu.eax, record = cpu.esi;
    logDwords(point, 3);
    mock(app, cpu, kOwn | 0x420650);
    setWord32(record + 0x14, scripted(4) == 0 ? 0 : 0x950000 + 16 * scripted(0x40));
    const x86::reg32 surface = 0x950000 + 16 * scripted(0x40);
    setWord32(record + 0x18, scripted(4) == 0 ? 0 : surface);
    static const x86::reg8 kSurfaces[] = { 0, 0x40, 0x0e, 0x4e, 1, 2, 0x3f };
    g_memory[surface + 1] = kSurfaces[scripted(std::size(kSurfaces))];
    if (scripted(2))
        setWord32(point + 4, word32(point + 4) + x86::reg32(scripted(0x8000)) - 0x2000);
}

/* A pose of the replay's: its numbers in fixed point, of a size. */
void replayPose(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.eax, axes = cpu.edx;
    mock(app, cpu, kOwn | 0x477d90);
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(place + 4 * i, x86::reg32(scripted(0x2000000)) - 0x1000000);
    for (x86::reg32 i = 0; i < 9; ++i)
        setWord32(axes + 4 * i, x86::reg32(scripted(0x20000)) - 0x10000);
    cpu.eax = scripted(6) == 0 ? 0 : 1 + scripted(100);
}

/* Colour to colour: three dwords in, three small ones out. */
template <x86::reg32 Address>
void colours(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    logDwords(cpu.eax, 3);
    mock(app, cpu, kOwn | Address);
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(out + 4 * i, scripted(8) == 0 ? scripted(0) : x86::reg32(scripted(0x180)) - 0x20);
}

template <x86::reg32 Address>
void rotation(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.eax;
    mock(app, cpu, kOwn | Address);
    writeFloats(out, 9);
}

void multiplyMatrices(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ebx;
    logDwords(cpu.eax, 9);
    logDwords(cpu.edx, 9);
    mock(app, cpu, kOwn | 0x4e0280);
    writeFloats(out, 9);
}

/* The car a camera looks at: one of the world's. */
void cameraCar(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kOwn | 0x422ba0);
    cpu.eax = 0x900000 + 0xa00 * scripted(8);
}

/* A light's colour, or none; how far it is, in reach or not. */
void lightColour(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kOwn | 0x491120);
    if (scripted(4) == 0)
        cpu.eax = 0;
}

void lightDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.eax, 3);
    mock(app, cpu, kOwn | 0x496550);
    if (scripted(4) != 0)
        cpu.eax = scripted(8) == 0 ? 0x100000 : scripted(0x120000);
}

/* The road's light of a flare: a record of the arena's, or none. */
void flareRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kOwn | 0x491bc0);
    cpu.eax = scripted(3) == 0 ? 0 : 0x950800 + 0x20 * scripted(0x20);
}

/* A batch of models ended: its list, of what the batch drew. */
void endModels(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 list = cpu.esi;
    mock(app, cpu, kOwn | 0x49c470);
    setWord32(list, scripted(3) == 0 ? 0 : 0x950800 + 0x20 * scripted(0x20));
    setWord32(list + 4, scripted(3) == 0 ? 0 : 0x950800 + 0x20 * scripted(0x20));
}

/* A car's detail by its distance. */
void carVisible(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, kOwn | 0x4bb5d0);
    cpu.eax = x86::reg32(scripted(6)) - 1;
}

}
