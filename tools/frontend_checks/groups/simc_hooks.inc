// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* tools/apply_widescreen.py's patch in sub_4dbde0: recorded, its answer from the script. */
float zoomHalfWidth(win32::WinApplication*)
{
    g_log.push_back(0x7e4a0000);
    return float(scripted(2000)) / 4.0f;
}

/* And for game functions that need more than kSpecs' kinds: what they are handed in a
 * frame, which nothing else looks at, and what they write there.  All of them record
 * esp, for a native's frame to show, which the table's stand-ins do not.  A function that
 * is a row of another group has its table entry at an address of its own, kSimcAlias on:
 * with one at its real address hasStandIn would pass this over for the table's. */
constexpr x86::reg32 kSimcAlias = 0x7e000000;

template <x86::reg32 Address>
void simcStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, Address);
}

void simcLogWords(x86::reg32 at, x86::reg32 count)
{
    for (x86::reg32 i = 0; i < count; ++i)
        g_log.push_back(word32(at + 4 * i));
}

/* Floats of a real size from the script. */
void simcAnswerFloats(x86::reg32 at, x86::reg32 count)
{
    for (x86::reg32 i = 0; i < count; ++i)
        setFloat(at + 4 * i, float(int(scripted(4000)) - 2000) / 1024.0f + float(scripted(1024)) * 1.0e-6f);
}

/* sub_4f9482: its arc cosine, the value on the x87 recorded; it keeps the registers. */
void simcArcCosine(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 bits[2];
    std::memcpy(bits, &cpu.fpu.st(0), sizeof bits);
    g_log.push_back(bits[0]);
    g_log.push_back(bits[1]);
    const double value = cpu.fpu.st(0);
    simcStandIn<0x4f9482>(app, cpu);
    cpu.fpu.st(0) = std::acos(value);
}

/* sub_4e0110: a matrix's transpose: the matrix recorded, done as the game does it. */
void simcTranspose(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 from = cpu.eax, to = cpu.edx;
    simcLogWords(from, 9);
    g_log.push_back(to);
    simcStandIn<0x4e0110>(app, cpu);
    x86::reg32 words[9];
    std::memcpy(words, g_memory + from, sizeof words);
    static const int kTransposed[9] = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
    for (int i = 0; i < 9; ++i)
        setWord32(to + 4 * i, words[kTransposed[i]]);
}

/* sub_4e0280: a matrix product: the two recorded, the answer from the script. */
void simcMultiply(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 to = cpu.ebx;
    simcLogWords(cpu.eax, 9);
    simcLogWords(cpu.edx, 9);
    simcStandIn<0x4e0280>(app, cpu);
    simcAnswerFloats(to, 9);
}

/* sub_4ea970: a sine and a cosine, the angle recorded. */
void simcSineCosine(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 sine = cpu.edx, cosine = cpu.ebx;
    simcLogWords(cpu.eax, 1);
    simcStandIn<0x4ea970>(app, cpu);
    simcAnswerFloats(sine, 1);
    simcAnswerFloats(cosine, 1);
}

/* sub_4972f0: a vector normalised, its length on the x87. */
void simcNormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 vector = cpu.eax;
    simcLogWords(vector, 3);
    simcStandIn<0x4972f0>(app, cpu);
    simcAnswerFloats(vector, 3);
    cpu.fpu.push(x86::Float(scripted(1000)));
}

/* sub_4e0160: a cross product; eax past the first vector, as the game leaves it. */
void simcCross(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 first = cpu.eax, to = cpu.ebx;
    simcLogWords(first, 3);
    simcLogWords(cpu.edx, 3);
    simcStandIn<0x4e0160>(app, cpu);
    simcAnswerFloats(to, 3);
    cpu.eax = first + 12;
}

/* sub_4c9bc0: an end of a node's side, into esi. */
void simcNodeEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 to = cpu.esi;
    simcStandIn<0x4c9bc0>(app, cpu);
    simcAnswerFloats(to, 3);
}

/* sub_435310: the mouse: its buttons, x and y, around the panel. */
void simcReadMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 buttons = cpu.eax, x = cpu.edx, y = cpu.ebx;
    simcStandIn<0x435310>(app, cpu);
    setWord32(buttons, scripted(3));
    setWord32(x, scripted(700) - 30);
    setWord32(y, scripted(520) - 20);
}

/* sub_4dab10: the pointer's hot spot: most of the time what brings the pointer (the mouse is
 * eight bytes under its hot spot in the frame; sub_4bd4d0 adds 7 and 1) to an edge of the
 * panel or of a button, or a pixel off. */
void simcHotSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 x = cpu.eax, y = cpu.edx;
    simcStandIn<kSimcAlias + 0x4dab10>(app, cpu);
    if (scripted(4) == 0)
    {
        setWord32(x, scripted(16));
        setWord32(y, scripted(16));
        return;
    }
    const x86::reg32 left = word32(0x7497d8), top = word32(0x7497dc), right = word32(0x7497e0), bottom = word32(0x7497e4);
    const x86::reg32 width = right - left;
    const x86::reg32 acrossX[] = { left, right, left + width * 32 / 100, left + width * 63 / 100, left + width * 72 / 100,
                                   left + width / 2 };
    const x86::reg32 acrossY[] = { top, bottom, (top + bottom) / 2, (top + bottom) / 2 };
    setWord32(x, word32(x - 8) + 7 - (acrossX[scripted(6)] + scripted(3) - 1));
    setWord32(y, word32(y - 8) + 1 - (acrossY[scripted(4)] + scripted(3) - 1));
}

/* sub_503b00: eax shifted left by edx, 32 here, into the 64 bits at esi. */
void simcShiftLeft(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 value = cpu.eax, to = cpu.esi;
    simcStandIn<0x503b00>(app, cpu);
    setWord32(to, 0);
    setWord32(to + 4, value);
    cpu.eax = to;
}

/* sub_48f8d0: a key, seldom down, so that each of the panel's keys comes up. */
void simcKeyPressed(win32::WinApplication* app, x86::CPU& cpu)
{
    simcStandIn<0x48f8d0>(app, cpu);
    cpu.eax = scripted(3) == 0 ? scripted(0) | 1 : 0;
}

/* sub_423d90: the nearest spot, or none. */
void simcNearestSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    simcStandIn<0x423d90>(app, cpu);
    if (scripted(2) == 0)
        cpu.eax = 0xffffffff;
}

/* sub_439a50: a car's record, memory to read, or none. */
void simcCarRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    simcStandIn<0x439a50>(app, cpu);
    cpu.eax = scripted(4) == 0 ? 0 : 0x902000 + 16 * scripted(16);
}

/* sub_464700: the tiles made: the file and the folder it is handed, from the frame. */
void simcMakeTiles(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const char c : guestString(cpu.eax))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    for (const char c : guestString(cpu.edx))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    simcStandIn<kSimcAlias + 0x464700>(app, cpu);
}

/* The one NaN a case has, the one the host's arithmetic makes of nothing: which of two
 * different NaNs a sum gives is the compiler's to choose (it swaps the operands of an add),
 * and differs between the native and the generated code as no game's data would have it. */
constexpr x86::reg32 kSimcNaN = 0xffc00000;

/* Floats of a real size, some of them exactly nothing or one.  And when `junk` says so raw
 * bits, those near a float's ends, where a sum or a product leaves its range, as the x87's
 * does not, for storing it in a float to tell, and NaN and the infinities. */
float simcSomeFloat(std::mt19937& random, bool junk)
{
    const unsigned pick = random() % (junk ? 16 : 10);
    if (pick >= 12)
    {
        static const x86::reg32 kSpecial[] = { kSimcNaN, kSimcNaN, kSimcNaN, 0x7f800000, 0xff800000, 0x80000000 };
        const float sign = random() % 2 ? 1.0f : -1.0f;
        if (pick == 12 || pick == 13)
            return sign * float(100 + random() % 240) * 1.0e36f;
        if (pick == 14)
            return sign * float(1 + random() % 1000) * 1.0e-28f;
        const x86::reg32 bits = kSpecial[random() % 6];
        float value;
        std::memcpy(&value, &bits, sizeof value);
        return value;
    }
    if (pick < 7)
        return float(int(random() % 8000) - 4000) / 1000.0f + float(random() % 1000) * 1.0e-6f;
    if (pick == 7)
        return float(int(random() % 3) - 1);
    if (pick == 8)
        return (float(int(random() % 2000) - 1000) + 0.1f) * 1.0e6f;
    if (pick == 9)
        return (float(int(random() % 2000) - 1000) + 0.1f) * 1.0e-6f;
    x86::reg32 bits = random();
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
        bits = kSimcNaN;
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

void simcFloats(std::mt19937& random, x86::reg32 at, x86::reg32 count, bool junk)
{
    for (x86::reg32 i = 0; i < count; ++i)
        setFloat(at + 4 * i, simcSomeFloat(random, junk));
}

/* A quaternion of length one. */
void simcUnitQuat(std::mt19937& random, x86::reg32 at)
{
    double q[4], length = 0;
    for (double& value : q)
    {
        value = double(int(random() % 2001) - 1000) + 0.5;
        length += value * value;
    }
    length = std::sqrt(length);
    for (x86::reg32 i = 0; i < 4; ++i)
        setFloat(at + 4 * i, float(q[i] / length));
}

/* A quaternion near the one at `from`, or it, or its negative, or none like it. */
void simcQuatLike(std::mt19937& random, x86::reg32 at, x86::reg32 from)
{
    const unsigned pick = random() % 5;
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        float value;
        std::memcpy(&value, g_memory + from + 4 * i, 4);
        const float nudge = float(int(random() % 2001) - 1000) * 1.0e-5f;
        setFloat(at + 4 * i, pick == 0 ? value : pick == 1 ? -value : pick == 2 ? value + nudge : -value + nudge);
    }
    if (pick == 4)
        simcUnitQuat(random, at);
}
