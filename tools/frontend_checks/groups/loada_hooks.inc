// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  These stand in for game functions whose results the group reads through,
// or that are handed things in the frame, which only they would see.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

// The yield THREAD_SEGMENTS puts at sub_43a6e0's two loops, recorded, so that it is compared; none of the
// checks links the real ones.  A group with another of those addresses shares these definitions.
}
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
void win32::WinApplication::unlockContext(const x86::CPU&) { g_log.push_back(0x7e4a0000); }
void win32::WinApplication::lockContext(const x86::CPU&) { g_log.push_back(0x7e4a0001); }
void win32::Thread::sleep(x86::reg32 milliseconds) { g_log.push_back(0x7e4b0000 | milliseconds); }
namespace nfs3hp
{

constexpr x86::reg32 kLoadaCars = 0xb00000;      // 16 records of 0x400
constexpr x86::reg32 kLoadaStrings = 0xb04000;   // 16 of 0x10
constexpr x86::reg32 kLoadaList = 0xb05000;
constexpr x86::reg32 kLoadaOut = 0xb06000;
constexpr x86::reg32 kLoadaSlices = 0xb08000;    // 40 of 36 bytes
constexpr x86::reg32 kLoadaObjects = 0xb10000;   // 8 race cars of 0x1000
constexpr x86::reg32 kLoadaSaved = 0xb18000;

static x86::reg32 loadaSomeCar()
{
    return kLoadaCars + 0x400 * scripted(16);
}

static x86::reg32 loadaSomeString()
{
    return kLoadaStrings + 0x10 * scripted(16);
}

static void loadaRecord(x86::reg32 at, x86::reg32 words)
{
    for (x86::reg32 i = 0; i < words; ++i)
        g_log.push_back(word32(at + 4 * i));
}

static float loadaNumber()
{
    if (scripted(16) == 0)
    {
        // Junk, half of it a NaN of its own.
        const x86::reg32 bits = scripted(2) ? scripted(0) : 0x7fc00000 | scripted(0x400000);
        float junk;
        std::memcpy(&junk, &bits, 4);
        return junk;
    }
    return float(x86::sreg32(scripted(4096)) - 2048) / 16.0f;
}

// Whether a car is police: for a whole case all are, none are, or by the script, so that a network
// Hot Pursuit of nothing but people or nothing but cops comes up.
static x86::reg32 g_loadaPolice = 2;

void loadaCarIsPolice(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4396b0);
    cpu.eax = g_loadaPolice == 2 ? scripted(2) : g_loadaPolice;
}

// A lane, of few, so that a car in the way is met.
void loadaRoadLane(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x40e2b0);
    cpu.eax = scripted(6);
}

// A car's number, 0 among them, or none: -1.
void loadaFindCarById(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x439990);
    cpu.eax = scripted(4) == 0 ? 0xffffffffu : scripted(4) == 0 ? 0 : scripted(40);
}

void loadaCarRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x439a50);
    cpu.eax = scripted(5) == 0 ? 0 : loadaSomeCar();
}

void loadaCarId(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x439520);
    cpu.eax = scripted(5) == 0 ? 0 : loadaSomeString();
}

void loadaFirstCar(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x438af0);
    cpu.eax = scripted(4) == 0 ? 0 : loadaSomeCar();
}

void loadaNextCar(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x438b10);
    cpu.eax = scripted(4) == 0 ? 0 : loadaSomeCar();
}

void loadaCarFolder(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4392b0);
    cpu.eax = loadaSomeString();
}

void loadaDriverName(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4047f0);
    cpu.eax = loadaSomeString();
}

// Past the first number, at least eight cars, as the choices the world sets are below eight.
void loadaCarCount(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x438bd0);
    cpu.eax = word32(0x554f50) + 8 + scripted(12);
}

void loadaFindSpec(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4dc490);
    cpu.eax = scripted(3) == 0 ? 0xffffffffu : scripted(50);
}

void loadaPlayerMachine(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4a6690);
    cpu.eax = (scripted(0) & 0xffff0000u) | (scripted(4) == 0 ? 0xffffu : scripted(8));
}

// The vector handed in recorded, and an answer where it was told.
void loadaScaleVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 out = word32(cpu.esp + 8);
    loadaRecord(cpu.edx, 3);
    mock(app, cpu, 0x4e0050);
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        // The caller adds this vector to the one 0x18 before it.  Of two NaNs added, which comes out is the
        // host compiler's choice of operands (an add commutes for it), the same source or not: never two.
        float value = loadaNumber();
        float before;
        std::memcpy(&before, g_memory + out - 0x18 + 4 * i, 4);
        if (value != value && before != before)
            value = 1.0f;
        setFloat(out + 4 * i, value);
    }
}

// The four bytes of the colour written where it was told, as the real one does: else the caller reads back
// stack that earlier calls left, their saved registers among it, which differ with the caller's own.
void loadaCarColourRgb(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 places[4] = { cpu.ecx, word32(cpu.esp + 4), word32(cpu.esp + 8), word32(cpu.esp + 12) };
    mock(app, cpu, 0x439810);
    for (const x86::reg32 place : places)
        g_memory[place] = x86::reg8(scripted(256));
}

// For a whole case no place is free: a free one is otherwise found within a slice or two, and the walk along
// the road never gets to where it wraps.
static bool g_loadaNoRoom = false;

// The point recorded; the place's record filled in, as the caller reads it.
void loadaSnapToTrack(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.esi;
    loadaRecord(cpu.eax, 3);
    mock(app, cpu, 0x4205a0);
    setWord32(place + 0x10, scripted(40));
    setWord32(place + 0x14, g_loadaNoRoom || scripted(6) == 0 ? 0 : scripted(0) | 1);
    setWord32(place + 0x18, scripted(6) == 0 ? 0 : scripted(0) | 1);
    setWord32(place + 0x1c, kLoadaOut + 4 * scripted(16));
}

// What is under the place; now and then the record's hit taken back, which the caller looks at again.
void loadaPositionSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 place = cpu.eax;
    mock(app, cpu, 0x4207e0);
    if (scripted(4) == 0)
        setWord32(place + 0x14, 0);
    cpu.eax = scripted(4) == 0 ? scripted(0) : (scripted(0) & 0xffffff00u) | (scripted(8) == 0 ? 0xe : scripted(0x40));
}

// Where the car is put, in the frame: recorded.
void loadaPlaceOnRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    loadaRecord(cpu.ebx, 3);
    mock(app, cpu, 0x4a0aa0);
}

void loadaRead(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e9200);
    cpu.eax = scripted(4) == 0 ? 0 : 1 + scripted(0x200);
}

void loadaWrite(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 asked = cpu.ebx;
    mock(app, cpu, 0x4eebd0);
    cpu.eax = scripted(5) == 0 ? asked + 1 : asked;
}

// Mostly no stop, or nothing past the first step is reached.
void loadaStopPoll(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4eb1c0);
    cpu.eax = scripted(12) == 0 ? 1 : 0;
}
