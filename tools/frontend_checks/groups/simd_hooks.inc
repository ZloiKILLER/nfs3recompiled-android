// Recording stand-ins for what these functions call and kSpecs' kinds cannot answer: what is handed in the
// caller's frame is recorded, what comes back there is written, what answers on the x87 answers there.  Every
// one records esp too, which shows a frame of the wrong size.

void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

// The cases' records: cameras and cars in their real shapes, then the rest.
constexpr x86::reg32 kSimdCameras = 0x690000, kSimdCameraBytes = 0x200, kSimdCameraCount = 2;
constexpr x86::reg32 kSimdCars = 0x691000, kSimdCarBytes = 0xa00, kSimdCarCount = 2;
constexpr x86::reg32 kSimdDriving = 0x692800;
constexpr x86::reg32 kSimdVectors = 0x693000;   // floats or ints, 0x40 apart
constexpr x86::reg32 kSimdList = 0x694000;      // a task list
constexpr x86::reg32 kSimdReserved = 0x695000;  // what sub_4e1620 hands out, 0x1000 apart: two
constexpr x86::reg32 kSimdSlices = 0x697000, kSimdSliceBytes = 36, kSimdSliceCount = 16;

/* A float from these bits; of the NaNs only the host's own.  With two different ones in a sum
 * or a product the host keeps the first, and the compiler is free to swap the two, so which
 * comes out says nothing about the code. */
constexpr x86::reg32 kSimdNaN = 0xffc00000;

float simdBits(x86::reg32 bits)
{
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
        bits = kSimdNaN;
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

void simdCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, address);
}

template <x86::reg32 Address>
void simdFramed(win32::WinApplication* app, x86::CPU& cpu)
{
    simdCall(app, cpu, Address);
}

/* A float from the script: real sizes with all their bits in use, ends, NaNs. */
float simdFloat()
{
    switch (scripted(16))
    {
    case 0:
    case 1:
        return 0.0f;
    case 2:
        return -0.0f;
    case 3:
        return float(int(scripted(16)) - 8) / 4.0f;
    case 4:
        return simdBits(scripted(0));
    case 5:
        return simdBits(kSimdNaN);
    case 6:
    case 7:
        return float(int(scripted(1u << 24)) - (1 << 23)) / float(1u << (15 + scripted(10)));
    default:
        return float(int(scripted(1u << 24)) - (1 << 23)) / float(1u << 15);
    }
}

/* What the x87 hands back, wider than a float. */
double simdWide()
{
    return double(simdFloat()) * (1.0 + double(scripted(1u << 20)) * 0x1p-40);
}

void simdWriteFloats(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        setFloat(at + 4 * i, simdFloat());
}

void simdRecord(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        g_log.push_back(word32(at + 4 * i));
}

// sub_4965b0: a quick distance of two places, on the x87.
void simdQuickDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    simdRecord(cpu.eax, 3);
    simdRecord(cpu.edx, 3);
    simdCall(app, cpu, 0x4965b0);
    cpu.fpu.push(x86::Float(simdWide()));
}

// sub_4972f0: a vector made one long in place, its length on the x87.
void simdNormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    simdRecord(at, 3);
    simdCall(app, cpu, 0x4972f0);
    simdWriteFloats(at, 3);
    cpu.fpu.push(x86::Float(simdWide()));
}

// sub_4e01c0: a vector made one long, eax to edx.
void simdUnitVector(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    simdRecord(cpu.eax, 3);
    simdCall(app, cpu, 0x4e01c0);
    simdWriteFloats(out, 3);
}

// sub_4e06a0: a vector's length, on the x87.
void simdVectorLength(win32::WinApplication* app, x86::CPU& cpu)
{
    simdRecord(cpu.eax, 3);
    simdCall(app, cpu, 0x4e06a0);
    cpu.fpu.push(x86::Float(simdWide()));
}

// sub_4e06d9: st(1) by st(0), into one: as the game's, the width of a double.
void simdArcTangent(win32::WinApplication* app, x86::CPU& cpu)
{
    const double y = cpu.fpu.st(1), x = cpu.fpu.st(0);
    x86::reg32 words[4];
    std::memcpy(words, &x, 8);
    std::memcpy(words + 2, &y, 8);
    for (const x86::reg32 word : words)
        g_log.push_back(word);
    simdCall(app, cpu, 0x4e06d9);
    cpu.fpu.pop();
    cpu.fpu.pop();
    cpu.fpu.push(x86::Float(std::atan2(y, x)));
}

// sub_423fb0: a float scaled, on the x87.
void simdScaledIfSet(win32::WinApplication* app, x86::CPU& cpu)
{
    simdCall(app, cpu, 0x423fb0);
    cpu.fpu.push(x86::Float(simdWide()));
}

// sub_424030: the vector at the stack's second damped in place.
void simdDampVector(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = word32(cpu.esp + 8);
    simdRecord(at, 3);
    simdCall(app, cpu, 0x424030);
    simdWriteFloats(at, 3);
}

// sub_4dffb0 and sub_4e0000: two vectors, three floats to ecx.
void simdAddVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ecx;
    simdRecord(cpu.edx, 3);
    simdRecord(cpu.ebx, 3);
    simdCall(app, cpu, 0x4dffb0);
    simdWriteFloats(out, 3);
}

void simdSubtractVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ecx;
    simdRecord(cpu.edx, 3);
    simdRecord(cpu.ebx, 3);
    simdCall(app, cpu, 0x4e0000);
    simdWriteFloats(out, 3);
}

// sub_4e0050: a vector by the stack's scale, three floats to its second.
void simdScaleVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    simdRecord(cpu.edx, 3);
    simdCall(app, cpu, 0x4e0050);
    simdWriteFloats(out, 3);
}

// sub_4b73c0: a matrix's quaternion, four floats to edx.
void simdQuatFromMatrix(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    simdRecord(cpu.eax, 9);
    simdCall(app, cpu, 0x4b73c0);
    simdWriteFloats(out, 4);
}

// sub_4b71e0: two quaternions slerped, four floats to the stack's second.
void simdSlerp(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    simdRecord(cpu.eax, 4);
    simdRecord(cpu.edx, 4);
    simdCall(app, cpu, 0x4b71e0);
    simdWriteFloats(out, 4);
}

// sub_4b7590: a quaternion of four floats.
void simdMatrixFromQuat(win32::WinApplication* app, x86::CPU& cpu)
{
    simdRecord(cpu.eax, 4);
    simdCall(app, cpu, 0x4b7590);
}

// sub_420650: a point in 16.16 put on the track, in place.
void simdPlaceOnTrack(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    simdRecord(at, 3);
    g_log.push_back(cpu.esi);
    simdCall(app, cpu, 0x420650);
    for (unsigned i = 0; i < 3; ++i)
        if (scripted(2))
            setWord32(at + 4 * i, x86::reg32(int(scripted(1u << 28)) - (1 << 27)));
}

// sub_4c8310: the stamp, four bytes to eax.
void simdStamp(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    simdCall(app, cpu, 0x4c8310);
    setWord32(at, scripted(0));
}

// sub_4c6840: the controls, four bytes to edx.
void simdTakenControls(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.edx;
    simdCall(app, cpu, 0x4c6840);
    setWord32(at, scripted(0));
}

// sub_4a60e0: a packet: what it says.
void simdSend(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 bytes = cpu.ebx < 0x40 ? cpu.ebx : 0x40;
    for (x86::reg32 i = 0; i < bytes; ++i)
        g_log.push_back(g_memory[cpu.edx + i]);
    simdCall(app, cpu, 0x4a60e0);
}

// sub_4e1620: memory, one of two blocks.
void simdReserve(win32::WinApplication* app, x86::CPU& cpu)
{
    simdCall(app, cpu, 0x4e1620);
    cpu.eax = kSimdReserved + 0x1000 * scripted(2);
}

// sub_420bf0: whether a place is in a tunnel: yes or no.
void simdInTunnel(win32::WinApplication* app, x86::CPU& cpu)
{
    simdCall(app, cpu, 0x420bf0);
    cpu.eax = scripted(3) != 0 ? 1 : 0;
}

// sub_4df690: sprintf: what it is handed, the developers' trace's two doubles too, which the buffer on the
// stack would not show.
void simdSprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    simdRecord(cpu.esp + 4, word32(cpu.esp + 8) == 0x536f24 ? 6 : 3);
    simdCall(app, cpu, 0x4df690);
}
