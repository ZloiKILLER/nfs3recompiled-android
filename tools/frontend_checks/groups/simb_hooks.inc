// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  These stand in for game functions that need more than kSpecs'
// kinds: what they are handed by address recorded, what they hand back by address or on the x87.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

namespace simb
{

float realFloat()
{
    static const float kSpecial[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.02f, 0.9f };
    const x86::reg32 pick = scripted(8);
    if (pick == 0)
        return kSpecial[scripted(8)];
    if (pick == 1)
        return float(x86::sreg32(scripted(2000)) - 1000) / 8192.0f;
    // All of a float's bits, or the sums and products would have nothing to round.
    return float(x86::sreg32(scripted(0x2000000)) - 0x1000000) / 65536.0f;
}

/* What a function left on the x87: now and then with more bits than a float has, as sin's
 * or a value of the x87's wider range would be. */
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

double floatAt(x86::reg32 at)
{
    float value;
    std::memcpy(&value, g_memory + at, 4);
    return double(value);
}

void dot(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.eax, 3);
    logDwords(cpu.edx, 3);
    mock(app, cpu, 0x4e01f0);
    cpu.fpu.push(x87Result());
}

void normalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 vector = cpu.eax;
    logDwords(vector, 3);
    mock(app, cpu, 0x4972f0);
    writeFloats(vector, 3);
    cpu.fpu.push(x87Result());
}

void transpose(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 from = cpu.eax, to = cpu.edx;
    logDwords(from, 9);
    x86::reg32 values[9];
    for (x86::reg32 i = 0; i < 9; ++i)
        values[i] = word32(from + 4 * i);
    mock(app, cpu, 0x4e0110);
    for (x86::reg32 row = 0; row < 3; ++row)
        for (x86::reg32 column = 0; column < 3; ++column)
            setWord32(to + 4 * (3 * row + column), values[3 * column + row]);
}

template <bool Subtract>
void addOrSubtract(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 a = cpu.edx, b = cpu.ebx, out = cpu.ecx;
    logDwords(a, 3);
    logDwords(b, 3);
    float results[3];
    for (x86::reg32 i = 0; i < 3; ++i)
        results[i] = float(Subtract ? fp::sub(floatAt(a + 4 * i), floatAt(b + 4 * i)) : fp::add(floatAt(a + 4 * i), floatAt(b + 4 * i)));
    mock(app, cpu, Subtract ? 0x4e0000 : 0x4dffb0);
    for (x86::reg32 i = 0; i < 3; ++i)
        setFloat(out + 4 * i, results[i]);
}

void scale(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 from = cpu.edx, out = word32(cpu.esp + 8);
    logDwords(from, 3);
    const double factor = floatAt(cpu.esp + 4);
    float results[3];
    for (x86::reg32 i = 0; i < 3; ++i)
        results[i] = float(fp::mul(floatAt(from + 4 * i), factor));
    mock(app, cpu, 0x4e0050);
    for (x86::reg32 i = 0; i < 3; ++i)
        setFloat(out + 4 * i, results[i]);
}

void transformPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 4);
    logDwords(cpu.edx, 3);
    logDwords(cpu.ebx, 9);
    logDwords(cpu.ecx, 3);
    mock(app, cpu, 0x4e0430);
    writeFloats(out, 3);
}

/* A track position after the lookup: on a polygon or not, its normal, its point. */
void updatePosition(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 position = cpu.edx;
    logDwords(cpu.eax, 3);
    logDwords(position, 0x1e);
    mock(app, cpu, 0x41f6e0);
    setWord32(position + 0x14, scripted(5) == 0 ? 0 : 1 + scripted(100));
    setWord32(position + 0x18, scripted(5) == 0 ? 0 : 0x906800 + scripted(0x100));
    setWord32(position + 0x1c, 0x906000 + 2 * scripted(0x400));
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(position + 0x24 + 4 * i, x86::reg32(scripted(0x2000000)) - 0x1000000);
    g_memory[position + 0x6a] = x86::reg8(scripted(5) == 0 ? 0 : 1 + scripted(4));
    g_memory[position + 0x6b] = 0;
    setWord32(position + 0x6c, scripted(5) == 0 ? 1 + scripted(100) : 0);
}

void positionSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(word32(cpu.eax + 0x14));
    g_log.push_back(word32(cpu.eax + 0x18));
    mock(app, cpu, 0x4207e0);
    static const x86::reg32 kSurfaces[] = { 0, 0x0e, 0x40, 0x4e, 1, 2, 0x3f, 0x85 };
    cpu.eax = scripted(4) == 0 ? scripted(0) : kSurfaces[scripted(8)];
}

void positionFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 fixedFrame = cpu.edx, other = cpu.esi;
    logDwords(cpu.eax, 0x1e);
    mock(app, cpu, 0x420230);
    for (x86::reg32 i = 0; i < 9; ++i)
        setWord32(fixedFrame + 4 * i, scripted(8) == 0 ? scripted(0) : x86::reg32(scripted(0x20000)) - 0x10000);
    for (x86::reg32 i = 0; i < 3; ++i)
        setWord32(other + 4 * i, scripted(0));
}

void hitWall(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.edx, 3);
    logDwords(cpu.ebx, 3);
    logDwords(cpu.ecx, 3);
    mock(app, cpu, 0x42ee10);
}

void boxHitsWall(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.edx, 3);
    logDwords(cpu.ebx, 3);
    mock(app, cpu, 0x42efe0);
}

void blendFrames(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    logDwords(cpu.eax, 9);
    logDwords(cpu.edx, 9);
    mock(app, cpu, 0x4b7680);
    writeFloats(out, 9);
}

template <x86::reg32 Address>
void rotation(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.eax;
    mock(app, cpu, Address);
    writeFloats(out, 9);
}

void multiplyMatrices(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ebx;
    logDwords(cpu.eax, 9);
    logDwords(cpu.edx, 9);
    mock(app, cpu, 0x4e0280);
    writeFloats(out, 9);
}

void roadRise(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4a0440);
    cpu.fpu.push(x87Result());
}

void followFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    logDwords(cpu.edx, 3);
    mock(app, cpu, 0x4a0f30);
}

}
