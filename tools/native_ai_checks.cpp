/* src/nfs3hp/native_ai.h against the generated code, on random states; built
 * and run by tools/native_ai_checks.py, which hands it the generated functions
 * (native_ai_generated.inc), the executable for its data, and a case count.
 *
 * Both run on a plain block of memory holding nfs3.exe's sections: the
 * generated sub_4064f0 and sub_4070c0 as the recompiler wrote them, and
 * nfs3hp::ai::aiPull and aiHeading.  The way an opponent wants to go
 * (sub_406e50) is a stand-in here, the same for both: random directions and a
 * random eax and flags back.  The arc tangent (sub_4e06d9) is the generated
 * one, its software path (sub_4ff624, taken when [0x567858] says the FPU has
 * the Pentium's FDIV bug) a stand-in again.  After each pair: every register,
 * every flag, the x87 stack, status and control words, the car and the
 * globals; every 499 cases all 16 MB of memory but the stack under it. */
#include <cpu.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <random>
#include <vector>

namespace x86
{
void assertLog(const char*, int, const char*) {}
void CPU::cpuid() {}
void CPU::rdtsc() {}
}

struct MockApp
{
    std::vector<uint8_t> memory = std::vector<uint8_t>(0x1000000);

    template <typename T>
    T& getMemory(x86::reg32 address)
    {
        return *reinterpret_cast<T*>(&memory[address & 0xffffff]);
    }

    void dynamic_call(x86::reg32 address, x86::CPU& cpu);
};

/* What the stand-in for sub_406e50 gives back, drawn per case. */
struct WayOut
{
    float direction[3];
    x86::reg32 eax;
    x86::reg32 flags;
} g_way;

static void sub_406e50(MockApp* app, x86::CPU& cpu)
{
    for (int i = 0; i < 3; ++i)
        app->getMemory<float>(cpu.esi + 4 * i) = g_way.direction[i];
    cpu.eax = g_way.eax;
    cpu.flags.eflags = (cpu.flags.eflags & ~0x08d5u) | (g_way.flags & 0x08d5u);
    cpu.esp += 4;
}

/* Watcom's software fpatan: y in st(1), x in st(0), the result left in st(0). */
static void sub_4ff624(MockApp*, x86::CPU& cpu)
{
    const x86::Float x = cpu.fpu.st(0);
    const x86::Float y = cpu.fpu.st(1);
    cpu.fpu.pop();
    cpu.fpu.st(0) = cpu.fpu.sub(cpu.fpu.atan(x, y), x86::Float(1e-9));
    cpu.esp += 4;
}

static void sub_4e06d9(MockApp* app, x86::CPU& cpu);

#include "native_ai_generated.inc"
#include "native_ai.h"

void MockApp::dynamic_call(x86::reg32 address, x86::CPU& cpu)
{
    switch (address)
    {
    case 0x406e50: sub_406e50(this, cpu); break;
    case 0x4e06d9: sub_4e06d9(this, cpu); break;
    case 0x4ff624: sub_4ff624(this, cpu); break;
    default: std::printf("unexpected call %08x\n", unsigned(address)); std::exit(2);
    }
}

static std::mt19937_64 g(424242);

static float someFloat(float scale)
{
    std::uniform_real_distribution<float> u(-1.0f, 1.0f);
    switch (g() % 16)
    {
    case 0: return 0.0f;
    case 1: return -0.0f;
    case 2: return float(int(g() % 200) - 100) * 0.5f;           // halves: ties
    case 3: return std::numeric_limits<float>::quiet_NaN();
    case 4: return std::numeric_limits<float>::infinity();
    case 5: return float(g() % 130);                              // whole speeds
    case 6: return u(g) * 1e30f;
    case 7: return u(g) * 1e-30f;
    default: return u(g) * scale;
    }
}

static void loadExe(MockApp& app, const char* path)
{
    FILE* f = std::fopen(path, "rb");
    if (!f)
    {
        std::printf("cannot read %s\n", path);
        std::exit(2);
    }
    std::vector<uint8_t> d;
    uint8_t buf[65536];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0)
        d.insert(d.end(), buf, buf + n);
    std::fclose(f);
    uint32_t pe, base;
    uint16_t count, optional;
    std::memcpy(&pe, &d[0x3c], 4);
    std::memcpy(&count, &d[pe + 6], 2);
    std::memcpy(&optional, &d[pe + 20], 2);
    std::memcpy(&base, &d[pe + 24 + 28], 4);
    size_t off = pe + 24 + optional;
    for (int i = 0; i < count; ++i, off += 40)
    {
        uint32_t vsize, rva, rsize, rptr;
        std::memcpy(&vsize, &d[off + 8], 4);
        std::memcpy(&rva, &d[off + 12], 4);
        std::memcpy(&rsize, &d[off + 16], 4);
        std::memcpy(&rptr, &d[off + 20], 4);
        // uninitialised data (.bss) has no bytes in the file
        if (rptr == 0 || rptr >= d.size())
            continue;
        if (rptr + rsize > d.size())
            rsize = uint32_t(d.size() - rptr);
        if (rsize && base + rva + rsize < app.memory.size())
            std::memcpy(&app.memory[base + rva], &d[rptr], rsize);
    }
}

static const x86::reg32 kStack = 0x900000;
static const x86::reg32 kCars = 0x5e1120;
static const x86::reg32 kCarSize = 0x9ac;
static const x86::reg32 kAi = 0x600000;

template <typename T>
static void put(MockApp& a, MockApp& b, x86::reg32 address, T value)
{
    a.getMemory<T>(address) = value;
    b.getMemory<T>(address) = value;
}

static bool sameFloat(const x86::Float& a, const x86::Float& b)
{
    return std::memcmp(&a, &b, sizeof(x86::Float)) == 0;
}

int main(int argc, char** argv)
{
    const long cases = argc > 2 ? std::atol(argv[2]) : 200000;
    MockApp generated, native;
    loadExe(generated, argv[1]);
    native.memory = generated.memory;
    const uint16_t words[] = { 0x007f, 0x007f, 0x007f, 0x037f, 0x037f, 0x027f, 0x0c7f, 0x047f, 0x087f, 0x0f7f };
    long bad[2] = { 0, 0 }, runs[2] = { 0, 0 };
    int shown = 0;
    for (long n = 0; n < cases; ++n)
    {
        const int which = int(g() % 2);
        // the cars, the table of them and the driver's pull tables
        for (int k = 0; k < 8; ++k)
        {
            const x86::reg32 car = kCars + k * kCarSize;
            put(generated, native, 0x5efa48 + 4 * k, car);
            put(generated, native, car + 0x200, x86::reg8(g()));
            put(generated, native, car + 0x509, x86::reg8(g()));
            put(generated, native, car + 0x528, kAi + k * 0x400);
            put(generated, native, car + 8, x86::reg32(50 + g() % 100));
            put(generated, native, car + 0x6e4, x86::reg32(int(g() % 4) - 1));
            put(generated, native, car + 0x1f0, x86::reg32(g() % 8));
            put(generated, native, car + 0x1c, x86::reg32(int(g() % 120) - 60));
            put(generated, native, car + 0x210, x86::reg32(g() % 4 ? int(g() % 2048) - 1024 : int(g())));
            put(generated, native, car + 0x7bc, someFloat(4.0f));
            put(generated, native, car + 0x7c0, someFloat(100.0f));
            put(generated, native, car + 0x7c4, std::fabs(someFloat(130.0f)) * (g() % 8 ? 1.0f : -1.0f));
            put(generated, native, kAi + k * 0x400 + 0x1cc, someFloat(100.0f));
            for (int i = 0; i < 112; ++i)
                put(generated, native, kAi + k * 0x400 + 0x2ec + 4 * i, someFloat(30.0f));
        }
        put(generated, native, 0x6fd4f0, x86::reg32(g() % 2));
        put(generated, native, 0x6fd4cc, x86::reg32(g() % 2));
        put(generated, native, 0x6fd50c, x86::reg32(g() % 2));
        put(generated, native, 0x6fd4f4, x86::reg32(g() % 8));
        put(generated, native, 0x6fd518, x86::reg32(g() % 9));
        put(generated, native, 0x7a22da, x86::reg16(g() % 2));
        put(generated, native, 0x55eb2e, x86::reg8(g()));
        put(generated, native, 0x567858, x86::reg8(g() % 4 == 0 ? 1 : 0));
        for (int i = 0; i < 3; ++i)
            g_way.direction[i] = someFloat(50.0f);
        g_way.eax = x86::reg32(g());
        g_way.flags = x86::reg32(g());

        x86::CPU a;
        std::memset(&a, 0, sizeof a);
        a.fpu.init();
        a.fpu.setControl(words[g() % (sizeof words / sizeof words[0])]);
        const int depth = int(g() % 4);
        for (int i = 0; i < depth; ++i)
            a.fpu.push(x86::Float(double(someFloat(1000.0f))));
        a.fpu.status.word = x86::reg16(g() & 0x4700);
        a.ebx = x86::reg32(g());
        a.ecx = x86::reg32(g());
        a.edx = x86::reg32(g());
        a.esi = x86::reg32(g());
        a.edi = x86::reg32(g());
        a.ebp = x86::reg32(g());
        a.flags.eflags = x86::reg32(g() & 0x08d5) | 0x0202;
        a.eax = kCars + x86::reg32(g() % 8) * kCarSize;
        a.esp = kStack - 4;  // the call's return address
        x86::CPU b = a;

        if (which == 0)
        {
            sub_4064f0(&generated, a);
            nfs3hp::ai::aiPull(&native, b);
        }
        else
        {
            sub_4070c0(&generated, a);
            nfs3hp::ai::aiHeading(&native, b);
        }
        // fpu.setControl is per thread in the 80-bit build: both ran with the same word
        ++runs[which];
        bool same = a.eax == b.eax && a.ebx == b.ebx && a.ecx == b.ecx && a.edx == b.edx && a.esi == b.esi
                 && a.edi == b.edi && a.ebp == b.ebp && a.esp == b.esp && a.flags.eflags == b.flags.eflags
                 && a.fpu.count == b.fpu.count && a.fpu.status.word == b.fpu.status.word
                 && a.fpu.control.word == b.fpu.control.word;
        for (x86::reg32 i = 0; same && i < a.fpu.count && i < 8; ++i)
            same = sameFloat(a.fpu.st(int(i)), b.fpu.st(int(i)));
        same = same && std::memcmp(&generated.memory[kCars], &native.memory[kCars], 8 * kCarSize) == 0
                    && std::memcmp(&generated.memory[0x56ec50], &native.memory[0x56ec50], 0x30) == 0;
        if (n % 499 == 0)
        {
            // all of memory, the stack below the caller's esp aside (a native leaves it unwritten)
            std::memset(&generated.memory[kStack - 0x10000], 0, 0x10000);
            std::memset(&native.memory[kStack - 0x10000], 0, 0x10000);
            same = same && generated.memory == native.memory;
        }
        if (!same)
        {
            ++bad[which];
            if (shown++ < 8)
                std::printf("  %s: eax %08x/%08x flags %08x/%08x fpu %u/%u sw %04x/%04x st0 %.17g/%.17g esp %08x/%08x\n",
                            which ? "sub_4070c0" : "sub_4064f0", unsigned(a.eax), unsigned(b.eax),
                            unsigned(a.flags.eflags), unsigned(b.flags.eflags), unsigned(a.fpu.count),
                            unsigned(b.fpu.count), a.fpu.status.word, b.fpu.status.word,
                            double(a.fpu.st(0)), double(b.fpu.st(0)), unsigned(a.esp), unsigned(b.esp));
            native.memory = generated.memory;
        }
    }
    std::printf("sub_4064f0 (aiPull):    %ld cases, %ld differ\n", runs[0], bad[0]);
    std::printf("sub_4070c0 (aiHeading): %ld cases, %ld differ\n", runs[1], bad[1]);
    return bad[0] || bad[1] ? 1 : 0;
}
