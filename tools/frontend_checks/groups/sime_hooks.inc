// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// And for the game's functions that need more than kSpecs' kinds: those handed locals of the caller's frame, which
// nothing else looks at, have what they are handed recorded and what they hand back written; those that answer on the
// x87 answer there.  Those handed a frame record esp too, which shows a frame of the wrong size.  The leaves answering
// on the x87 do not: the original calls them with arguments for its next call already pushed.

void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

// The cases' records.
constexpr x86::reg32 kSimeNames = 0x688000;
constexpr x86::reg32 kSimeBlock = 0x689000;
constexpr x86::reg32 kSimeZones = 0x68a000;
constexpr x86::reg32 kSimeCar = 0x68b000;
constexpr x86::reg32 kSimeRecords = 0x68d000;
constexpr unsigned kSimeNameCount = 8, kSimeNameBytes = 16;

/* A float from these bits; of the NaNs only the host's own. */
constexpr x86::reg32 kSimeNaN = 0xffc00000;

float simeBits(x86::reg32 bits)
{
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
        bits = kSimeNaN;
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

/* A float from the script: real sizes with all their bits in use, ends, a NaN. */
float simeFloat()
{
    switch (scripted(24))
    {
    case 0:
        return 0.0f;
    case 1:
        return -0.0f;
    case 2:
        return float(int(scripted(16)) - 8) / 4.0f;
    case 3:
        return simeBits(scripted(0));
    case 4:
        return simeBits(kSimeNaN);
    default:
        return float(int(scripted(1u << 24)) - (1 << 23)) / float(1u << (15 + scripted(10)));
    }
}

void simeRecord(x86::reg32 at, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        g_log.push_back(word32(at + 4 * i));
}

void simeCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, address);
}

// sub_4eb230: on the x87, a fraction; wider than a float, and now and then none at all.
void simeFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4eb230);
    double value = double(1 + scripted((1u << 24) - 2)) * 0x1p-24 * (1.0 + double(scripted(1u << 20)) * 0x1p-40);
    switch (scripted(32))
    {
    case 0:
        value = 0.0;
        break;
    case 1:
        value = 1e30;
        break;
    case 2:
        value = double(simeBits(kSimeNaN));
        break;
    default:
        break;
    }
    cpu.fpu.push(x86::Float(value));
}

// sub_4ea920, sub_4ea930, sub_4ea950: the stack's float taken, what comes of it on the x87.
template <x86::reg32 Address>
void simeOnX87(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    cpu.fpu.push(x86::Float(simeFloat()) * (1.0 + double(scripted(1u << 20)) * 0x1p-40));
}

// sub_4972f0: eax's vector made a unit, its length on the x87.
void simeNormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 vector = cpu.eax;
    simeRecord(vector, 3);
    simeCall(app, cpu, 0x4972f0);
    for (unsigned i = 0; i < 3; ++i)
        setFloat(vector + 4 * i, simeFloat());
    cpu.fpu.push(x86::Float(simeFloat()));
}

// sub_4e0000: edx less ebx to ecx, what comes out often nothing.
void simeSubtract(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 from = cpu.edx, less = cpu.ebx, out = cpu.ecx;
    simeRecord(from, 3);
    simeRecord(less, 3);
    simeCall(app, cpu, 0x4e0000);
    const bool none = scripted(4) == 0;
    for (unsigned i = 0; i < 3; ++i)
        setFloat(out + 4 * i, none ? (scripted(2) ? 0.0f : -0.0f) : simeFloat());
}

// sub_4e0110: eax's matrix to edx: a near-real one, as the strips' corners need one, or anything.
void simeTranspose(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = cpu.edx;
    simeRecord(cpu.eax, 9);
    simeCall(app, cpu, 0x4e0110);
    const bool anything = scripted(8) == 0;
    for (unsigned i = 0; i < 9; ++i)
        setFloat(out + 4 * i, anything ? simeFloat() : (i % 4 == 0 ? 1.0f : 0.0f) + float(int(scripted(1u << 12)) - (1 << 11)) * 0x1p-14f);
}

// sub_4217e0: the node under a view's camera.
void simeCameraNode(win32::WinApplication* app, x86::CPU& cpu)
{
    simeCall(app, cpu, 0x4217e0);
    // At a zone's node or one off it, half the time: where its branches part.
    if (scripted(2) != 0)
        cpu.eax = word32(kSimeZones + 0x24 * scripted(6) + 0x18 + 4 * scripted(3)) + scripted(3) - 1;
    else
        cpu.eax = x86::reg32(int(scripted(90)) - 10);
}

// sub_4386b0, sub_4387e0: a name of the world's, or the end of them.
template <x86::reg32 Address>
void simeFolder(win32::WinApplication* app, x86::CPU& cpu)
{
    simeCall(app, cpu, Address);
    cpu.eax = scripted(5) == 0 ? 0 : kSimeNames + kSimeNameBytes * scripted(kSimeNameCount);
}

// sub_438340 and sub_4389d0: what the path says, recorded.
template <x86::reg32 Address>
void simePath(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 path = cpu.eax;
    for (const char c : guestString(path, 64))
        g_log.push_back(x86::reg32(std::uint8_t(c)));
    simeCall(app, cpu, Address);
    if (Address == 0x438340 && scripted(2) != 0)
    {
        const std::string text = guestString(path, 64);
        g_memory[path + text.size()] = '\\';
        g_memory[path + text.size() + 1] = 0;
    }
}

// sub_4e1620: a block.
void simeReserve(win32::WinApplication* app, x86::CPU& cpu)
{
    simeCall(app, cpu, 0x4e1620);
    cpu.eax = kSimeBlock + 0x10 * scripted(16);
}

// sub_4a33d0: the screen kept alive, and now and then told to stop while it was.
void simeTick(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4a33d0);
    if (scripted(6) == 0)
        setWord32(0x55e88c, 1 + scripted(9));
}
