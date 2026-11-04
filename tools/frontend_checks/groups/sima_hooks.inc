// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// And for the game's functions that need more than kSpecs' kinds: those handed locals of the caller's frame, which
// nothing else looks at, have what they are handed recorded and what they hand back written; those that answer on the
// x87 answer there.  Every one records esp too, which shows a frame of the wrong size.

void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* sub_40de30's hook, the trace NFS_CAR_TRACE turns on: off, and nothing to see. */
bool aiVotes(win32::WinApplication*, x86::CPU&) { return false; }

// The cases' records: cameras and cars in their real shapes, slices and spots after them.
constexpr x86::reg32 kSimaCameras = 0x680000, kSimaCameraBytes = 0x200, kSimaCameraCount = 4;
constexpr x86::reg32 kSimaCars = 0x681000, kSimaCarBytes = 0xa00, kSimaCarCount = 4;
constexpr x86::reg32 kSimaDriving = 0x684000;
constexpr x86::reg32 kSimaSlices = 0x685000, kSimaSliceBytes = 36;
constexpr x86::reg32 kSimaSpots = 0x686000, kSimaSpotBytes = 28;
constexpr x86::reg32 kSimaPoints = 0x686200;

// The place in the lane a case's cameras often stand at already, in 16.16.
x86::reg32 g_simaLane[3];
// The slices' upper limit in 16.16, for a height a hair over it.
x86::sreg32 g_simaAbove;

template <x86::reg32 Address>
void simaFramed(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, Address);
}

void simaCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, address);
}

/* A float from these bits; of the NaNs only the host's own.  With two different ones in a sum
 * or a product the host keeps the first, and the compiler is free to swap the two, so which
 * comes out says nothing about the code. */
constexpr x86::reg32 kSimaNaN = 0xffc00000;

float simaBits(x86::reg32 bits)
{
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
        bits = kSimaNaN;
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

/* A float from the script: real sizes with all their bits in use, ends, NaNs of either kind. */
float simaFloat()
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
        return simaBits(scripted(0));
    case 5:
        return simaBits(kSimaNaN);
    case 6:
    case 7:
        return float(int(scripted(1u << 24)) - (1 << 23)) / float(1u << (15 + scripted(10)));
    default:
        return float(int(scripted(1u << 24)) - (1 << 23)) / float(1u << 15);
    }
}

/* What the x87 hands back, wider than a float. */
double simaWide()
{
    return double(simaFloat()) * (1.0 + double(scripted(1u << 20)) * 0x1p-40);
}

void simaWriteFloats(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        setFloat(at + 4 * i, simaFloat());
}

void simaWriteInts(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        setWord32(at + 4 * i, x86::reg32(int(scripted(1u << 26)) - (1 << 25)));
}

void simaRecord(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        g_log.push_back(word32(at + 4 * i));
}

// sub_41ffd0: the place in the lane, three ints to edx.
void simaLanePosition(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    simaCall(app, cpu, 0x41ffd0);
    if (scripted(4) != 0)
        for (unsigned i = 0; i < 3; ++i)
            setWord32(out + 4 * i, g_simaLane[i]);
    else
        simaWriteInts(out, 3);
}

// sub_40f8d0: a path's end pose in 16.16: three ints to ecx, nine to the stack's pointer.
void simaPathEndPose(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.ecx, matrix = word32(cpu.esp + 4);
    simaCall(app, cpu, 0x40f8d0);
    simaWriteInts(place, 3);
    simaWriteInts(matrix, 9);
}

// sub_40f940: a path's step as floats: three to edx, nine to ebx.
void simaAnimationStep(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.edx, matrix = cpu.ebx;
    simaCall(app, cpu, 0x40f940);
    simaWriteFloats(place, 3);
    simaWriteFloats(matrix, 9);
}

// sub_4e0280: nine and nine floats, nine to ebx.
void simaMultiplyMatrices(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ebx;
    simaRecord(cpu.eax, 9);
    simaRecord(cpu.edx, 9);
    simaCall(app, cpu, 0x4e0280);
    simaWriteFloats(out, 9);
}

// sub_4972f0: a vector made one long in place, its length on the x87.
void simaNormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 at = cpu.eax;
    simaRecord(at, 3);
    simaCall(app, cpu, 0x4972f0);
    simaWriteFloats(at, 3);
    cpu.fpu.push(x86::Float(simaWide()));
}

// sub_4e0050: a vector by the stack's scale, three floats to its second.
void simaScaleVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    simaRecord(cpu.edx, 3);
    simaCall(app, cpu, 0x4e0050);
    simaWriteFloats(out, 3);
}

// sub_4dffb0: two vectors, three floats to ecx.
void simaAddVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.ecx;
    simaRecord(cpu.edx, 3);
    simaRecord(cpu.ebx, 3);
    simaCall(app, cpu, 0x4dffb0);
    simaWriteFloats(out, 3);
}

// sub_4eac60: an angle's two parts, ints to edx and ebx: any, and zeros for a ratio that is no number.
void simaAngleParts(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 a = cpu.edx, b = cpu.ebx;
    simaCall(app, cpu, 0x4eac60);
    const bool none = scripted(4) == 0;
    setWord32(a, none || scripted(8) == 0 ? 0 : x86::reg32(int(scripted(1u << 17)) - (1 << 16)));
    setWord32(b, none || scripted(8) == 0 ? 0 : x86::reg32(int(scripted(1u << 17)) - (1 << 16)));
}

// sub_424fd0 and sub_425b40: a spot of three floats to go towards.
void simaMoveTowards(win32::WinApplication* app, x86::CPU& cpu)
{
    simaRecord(cpu.edx, 3);
    simaCall(app, cpu, 0x424fd0);
}

void simaGlideTowards(win32::WinApplication* app, x86::CPU& cpu)
{
    simaRecord(cpu.edx, 3);
    simaCall(app, cpu, 0x425b40);
}

// sub_4b7590: a quaternion of four floats.
void simaMatrixFromQuaternion(win32::WinApplication* app, x86::CPU& cpu)
{
    simaRecord(cpu.eax, 4);
    simaCall(app, cpu, 0x4b7590);
}

// sub_420230: the track's frame in 16.16: nine ints to edx, three to esi.
void simaPositionFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 axes = cpu.edx, centre = cpu.esi;
    simaCall(app, cpu, 0x420230);
    simaWriteInts(axes, 9);
    simaWriteInts(centre, 3);
}

// sub_4e0430: eight corners given, eight back at the stack's pointer.
void simaTransformPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 4);
    simaRecord(cpu.edx, 24);
    simaCall(app, cpu, 0x4e0430);
    simaWriteFloats(out, 24);
}

// sub_4e01f0: a dot product of two vectors, on the x87: often a hair over the slices' upper limit, the
// same as a float, which tells the height left on the x87 from the one stored.
void simaDot(win32::WinApplication* app, x86::CPU& cpu)
{
    simaRecord(cpu.eax, 3);
    simaRecord(cpu.edx, 3);
    simaCall(app, cpu, 0x4e01f0);
    if (scripted(3) == 0)
    {
        const double above = double(float(fp::mul(x86::Float(g_simaAbove), x86::Float(1.0 / 65536.0))));
        cpu.fpu.push(x86::Float(above + std::fabs(above) * 0x1p-26));
    }
    else
    {
        cpu.fpu.push(x86::Float(simaWide()));
    }
}

// sub_406e50: the way to go, floats to esi.
void simaWayToGo(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.esi;
    simaCall(app, cpu, 0x406e50);
    simaWriteFloats(out, 6);
}

// sub_4e06d9: st(1) by st(0), into one: as the game's, the width of a double.
void simaArcTangent(win32::WinApplication* app, x86::CPU& cpu)
{
    const double y = cpu.fpu.st(1), x = cpu.fpu.st(0);
    x86::reg32 words[4];
    std::memcpy(words, &x, 8);
    std::memcpy(words + 2, &y, 8);
    for (const x86::reg32 word : words)
        g_log.push_back(word);
    simaCall(app, cpu, 0x4e06d9);
    cpu.fpu.pop();
    cpu.fpu.pop();
    cpu.fpu.push(x86::Float(std::atan2(y, x)));
}

// sub_4ea9c0: the stack's two floats (kSpecs records them), onto the x87.
void simaArcTangentOfFloats(win32::WinApplication* app, x86::CPU& cpu)
{
    const float y = simaBits(word32(cpu.esp + 4)), x = simaBits(word32(cpu.esp + 8));
    simaCall(app, cpu, 0x4ea9c0);
    // Now and then a hair under an integer once multiplied by 1024: rounded to a float's bits, it is the integer.
    if (scripted(4) == 0)
        cpu.fpu.push(x86::Float((double(scripted(3000)) + 1.0 - 0x1p-14) / 1024.0));
    else
        cpu.fpu.push(x86::Float(std::atan2(double(y), double(x))));
}

// sub_4a00d0 and sub_4a0440: a height on the x87.
void simaHeightOverRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    simaCall(app, cpu, 0x4a00d0);
    cpu.fpu.push(x86::Float(simaWide()));
}

void simaLowestPoint(win32::WinApplication* app, x86::CPU& cpu)
{
    simaCall(app, cpu, 0x4a0440);
    cpu.fpu.push(x86::Float(simaWide()));
}

// sub_423d90: a spot's number, or -1 for none near.
void simaNearestSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    simaCall(app, cpu, 0x423d90);
    cpu.eax = scripted(4) == 0 ? x86::reg32(-1) : scripted(8);
}

// sub_40f3b0: an animation, or -1 for none to be had.
void simaStartAnimation(win32::WinApplication* app, x86::CPU& cpu)
{
    simaCall(app, cpu, 0x40f3b0);
    cpu.eax = scripted(4) == 0 ? x86::reg32(-1) : scripted(8);
}

// sub_40e120: how many nodes, sometimes past where 366 is.
void simaNodeGap(win32::WinApplication* app, x86::CPU& cpu)
{
    simaCall(app, cpu, 0x40e120);
    cpu.eax = scripted(120);
}
