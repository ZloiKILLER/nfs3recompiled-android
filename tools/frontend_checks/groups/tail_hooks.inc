// Recording stand-ins for what these functions call: definitions, in namespace nfs3hp.  Those handed locals of the
// caller's frame have what they are handed recorded, esp with it, which shows a frame of the wrong size, and what they
// hand back written; those that answer on the x87 answer there.

void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

// The cases' records.
constexpr x86::reg32 kTailTasks = 0x688000, kTailTasksLate = 0x688400;
constexpr x86::reg32 kTailPlaces = 0x689000;
constexpr x86::reg32 kTailOut = 0x68a000;
constexpr x86::reg32 kTailNodes = 0x68b000;
constexpr x86::reg32 kTailViews = 0x68c000;
constexpr x86::reg32 kTailCars = 0x68c800;
constexpr x86::reg32 kTailCameras = 0x68d000;
constexpr x86::reg32 kTailOutList = 0x68d800;
constexpr x86::reg32 kTailTask = 0x7e0200;     // a task's function: a row of its own, in the world
constexpr x86::reg32 kTailNaN = 0xffc00000;

float tailFloatFromBits(x86::reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

// sub_4eb230: a fraction on the x87, wider than a float, now and then none at all.  The set-up asks 4800 of them, and
// the hash of memory at each is most of a case's time: all of it at the first few and every so often, for the order of
// a drop's writes is the same each time; the rest what it can be told by cheaply.
void tailFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    if (g_calls < 24 || g_log.size() % 61 == 0)
    {
        mock(app, cpu, 0x4eb230);
    }
    else
    {
        g_log.push_back(0x4eb230);
        g_log.push_back(cpu.fpu.count);
        cpu.esp += 4;
    }
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
        value = double(tailFloatFromBits(kTailNaN));
        break;
    default:
        break;
    }
    cpu.fpu.push(x86::Float(value));
}

// sub_420d60: 0 half the time, or anything.
void tailFacing(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x420d60);
    cpu.eax = scripted(2) ? 0 : (scripted(2) ? scripted(0) : 1);
}

// sub_422ba0: one of the world's cars.
void tailCameraCar(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x422ba0);
    cpu.eax = kTailCars + 0x40 * scripted(4);
}

// sub_4bbe70: the list esi points at, made.
void tailEmptyList(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 list = cpu.esi;
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4bbe70);
    setWord32(list, scripted(0));
    setWord32(list + 4, scripted(0));
}

// sub_4de700: the list added to; how many were seen to ebx's address.
void tailStreaks(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 list = cpu.esi, seen = cpu.ebx;
    g_log.push_back(cpu.esp);
    g_log.push_back(word32(seen));
    g_log.push_back(word32(list));
    mock(app, cpu, 0x4de700);
    setWord32(seen, scripted(3) == 0 ? 0 : 1 + scripted(1000));
    setWord32(list, scripted(0));
    setWord32(list + 4, scripted(0));
}

// sub_4de070: the list added to.
void tailFlakes(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 list = cpu.esi;
    g_log.push_back(cpu.esp);
    g_log.push_back(word32(list));
    mock(app, cpu, 0x4de070);
    setWord32(list, scripted(0));
    setWord32(list + 4, scripted(0));
}

// sub_4ea970: the float eax points at, recorded; two floats back.
void tailSineCosine(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 sine = cpu.edx, cosine = cpu.ebx;
    g_log.push_back(cpu.esp);
    g_log.push_back(word32(cpu.eax));
    mock(app, cpu, 0x4ea970);
    setFloat(sine, float(int(scripted(1u << 16)) - (1 << 15)) / 32768.0f);
    setFloat(cosine, float(int(scripted(1u << 16)) - (1 << 15)) / 32768.0f);
}

// sub_4e0050: the esp it is called at; the rest its row's.
void tailScale(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, 0x4e0050);
}
