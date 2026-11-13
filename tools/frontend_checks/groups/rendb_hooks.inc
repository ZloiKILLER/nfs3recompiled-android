// Recording stand-ins for what these functions call: definitions, in namespace nfs3hp.  What needs more than kSpecs' kinds:
// those handed locals of the caller's frame, which nothing else looks at, have them recorded; those that answer on the x87
// answer there; a batch hands out somewhere for its corners.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

constexpr x86::reg32 kRendbBatches = 0x900000;

/* A float from these bits; of the NaNs only the host's own: with two different ones in a sum or a
 * product the host keeps the first, and the compiler is free to swap the two. */
float rendbBits(x86::reg32 bits)
{
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
        bits = 0xffc00000;
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

void rendbRecordWords(x86::reg32 at, x86::reg32 words)
{
    for (x86::reg32 i = 0; i < words; ++i)
        g_log.push_back(word32(at + 4 * i));
}

/* sub_4c12e0: its four corners, whole. */
void rendbQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 corner : { cpu.eax, cpu.edx, cpu.ebx, cpu.ecx })
        rendbRecordWords(corner, 8);
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4c12e0);
}

/* sub_4c0b20: its two. */
void rendbSegment(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 corner : { cpu.eax, cpu.edx })
        rendbRecordWords(corner, 8);
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4c0b20);
}

/* sub_49bd10: a batch of corners, each somewhere of its own, and a first number. */
void rendbNewBatch(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 corners = cpu.ebx, firstNumber = cpu.ecx;
    mock(app, cpu, 0x49bd10);
    setWord32(corners, kRendbBatches + (g_calls << 12));
    setWord32(firstNumber, scripted(0x100));
}

/* sub_4965b0: what it was handed, by hand: it is another group's, and a stand-in in kSpecs would
 * go round this.  Onto the x87 a distance around the 300 sub_47d580 stops at. */
void rendbDistance(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0xca110000);
    g_log.push_back(0x4965b0);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    rendbRecordWords(cpu.eax, 3);
    g_log.push_back(cpu.esp);
    g_log.push_back(watchedHash());
    g_log.push_back(cpu.fpu.count);
    float distance;
    switch (scripted(12))
    {
    case 0:
        distance = 300.0f;
        break;
    case 1:
        distance = rendbBits(0x7fc00000);
        break;
    case 2:
        distance = 320.0f + float(scripted(1000));
        break;
    case 3:
        distance = 0.0f;
        break;
    default:
        distance = float(scripted(300000)) / 1000.0f;
        break;
    }
    cpu.fpu.push(x86::Float(distance));
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* sub_4e01f0: the vector it was handed, and a dot product onto the x87. */
void rendbDot(win32::WinApplication* app, x86::CPU& cpu)
{
    rendbRecordWords(cpu.edx, 3);
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4e01f0);
    cpu.fpu.push(x86::Float(scripted(16) == 0 ? rendbBits(0x7fc00000) : float(x86::sreg32(scripted(2001)) - 1000) / 1000.0f));
}

/* sub_4ea9c0: an angle onto the x87, as a share of a turn: often just at what sub_47f090 compares it
 * with, a float's or a double's. */
void rendbAngle(win32::WinApplication* app, x86::CPU& cpu)
{
    static const double kAt[] = { 0.0, -0.0, 0.25, -0.25, 0.5, -0.5, 0.02, -0.02, double(0.02f), double(-0.02f), 0.12, -0.12, double(0.12f), double(-0.12f) };
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4ea9c0);
    const x86::reg32 pick = scripted(24);
    double angle;
    if (pick < std::size(kAt))
        angle = kAt[pick];
    else if (pick == 23)
        angle = double(rendbBits(0x7fc00000));
    else
        angle = double(x86::sreg32(scripted(1400)) - 700) / 1000.0;
    cpu.fpu.push(x86::Float(angle));
}

/* sub_4eaab0 and sub_4ea9e0: the matrix they start from. */
void rendbRotate(win32::WinApplication* app, x86::CPU& cpu)
{
    rendbRecordWords(cpu.eax, 9);
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4eaab0);
}

void rendbScale(win32::WinApplication* app, x86::CPU& cpu)
{
    rendbRecordWords(cpu.eax, 9);
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4ea9e0);
}

/* sub_4ef1e0: the name it is handed, what is in the frame; now and then no such picture. */
void rendbFindPicture(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const char c : guestString(cpu.edx, 16))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4ef1e0);
    if (scripted(5) == 0)
        cpu.eax = 0;
}

/* sub_4e0ec0: the path it is handed, what sub_4df690 wrote in the frame. */
void rendbLoadFile(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const char c : guestString(cpu.eax, 64))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4e0ec0);
}
