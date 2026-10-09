/* src/nfs3hp/native_loading.cpp against the generated code it stands in for,
 * on the desktop, without the game or its data.
 *
 * The generated functions are copied out of a tree without
 * tools/apply_native_loading.py's hooks (extract.py) and built with the port's
 * own headers; the few game functions they call -- the file's name and the
 * file, the game's heap -- are stand-ins here (mock*).  Each case starts both
 * from the same memory and registers: the generated code, then the native, and
 * then all of memory but the stack below the returned esp, the registers, the
 * flags the generated code keeps, the FPU and MMX state are compared.
 *
 * The data is made up: RefPack streams of every command (overlapping copies,
 * the long forms, both headers), textures of every pixel format with and
 * without alpha, FCE models and FRD tracks with blocks, polygons in pieces,
 * objects and lights -- the layouts as the game reads them (OpenNFS's).
 *
 * See run.sh. */
#include <nfs3hp.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <sys/mman.h>

namespace x86
{
void assertLog(const char* file, int line, const char* expr)
{
    std::printf("assert %s:%d %s\n", file, line, expr);
}
}

namespace nfs3hp
{
bool nativesEnabled() { return true; }
bool nativeOriginalRunning() { return false; }
bool nativeChecking() { return false; }

bool refpack(win32::WinApplication* app, x86::CPU& cpu);
bool alpha4444(win32::WinApplication* app, x86::CPU& cpu);
bool alpha8888(win32::WinApplication* app, x86::CPU& cpu);
bool alphaNibbles(win32::WinApplication* app, x86::CPU& cpu);
bool alphaPaletted(win32::WinApplication* app, x86::CPU& cpu);
bool alpha1555(win32::WinApplication* app, x86::CPU& cpu);
bool alpha565(win32::WinApplication* app, x86::CPU& cpu);
bool fceRecords(win32::WinApplication* app, x86::CPU& cpu);
bool frd(win32::WinApplication* app, x86::CPU& cpu);
}

namespace
{

constexpr x86::reg32 kMemory = 0x4000000;
constexpr x86::reg32 kStack = 0x3ff0000;
// Where the stand-ins keep their state, in guest memory so that it is put
// back with the rest between the two runs.
constexpr x86::reg32 kHeapNext = 0x27ffff0;
constexpr x86::reg32 kFileAt = 0x27fffe0;
constexpr x86::reg32 kHeap = 0x2800000;

x86::reg8* g_memory = nullptr;
std::map<x86::reg32, win32::MethodPtr> g_methods;

}

namespace win32
{

WinApplication::WinApplication(const char*, x86::reg32, const std::vector<Section>&)
{
    void* memory = mmap(nullptr, kMemory, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
        std::abort();
    m_memory = static_cast<x86::reg8*>(memory);
    g_memory = m_memory;
}

WinApplication::~WinApplication() {}

int WinApplication::s_traceApi = 0;
bool WinApplication::readTraceApi() { return false; }
void WinApplication::traceCall(const Method&, x86::reg32) {}
void WinApplication::yieldContext(const x86::CPU&) {}

void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    auto method = g_methods.find(address);
    if (method == g_methods.end())
    {
        std::printf("no method at %08x\n", unsigned(address));
        std::abort();
    }
    method->second(this, cpu);
}

void MemMap::usedRanges(std::vector<std::pair<x86::reg32, x86::reg32>>& ranges)
{
    ranges.assign(1, {0, kMemory});
}

}

namespace nfs3hp
{

inline x86::reg32 word(x86::reg32 at)
{
    x86::reg32 value;
    std::memcpy(&value, g_memory + at, 4);
    return value;
}

inline void setWord(x86::reg32 at, x86::reg32 value)
{
    std::memcpy(g_memory + at, &value, 4);
}

// sub_41d8a0: the track's file name, in the buffer at 0x5dd86c; edx and ebp kept.
void Application::sub_41d8a0(WinApplication*, x86::CPU& cpu)
{
    std::strcpy(reinterpret_cast<char*>(g_memory + 0x5dd86c), "gamedata\\tracks\\trk000\\tr00.frd");
    cpu.eax = 0x5dd86c;
    cpu.ecx = 0x11111111;
    cpu.flags.zf = 1;
    cpu.esp += 4;
}

// sub_4e0ec0: the file, already in memory at [kFileAt].
void Application::sub_4e0ec0(WinApplication*, x86::CPU& cpu)
{
    cpu.eax = word(kFileAt);
    cpu.ecx = 0x22222222;
    cpu.edx = 0x33333333;
    cpu.flags.cf = 1;
    cpu.esp += 4;
}

// sub_4e1620: the game's heap, as a bump allocator.
void Application::sub_4e1620(WinApplication*, x86::CPU& cpu)
{
    const x86::reg32 at = word(kHeapNext);
    setWord(kHeapNext, at + ((cpu.edx + 15) & ~15u));
    cpu.eax = at;
    cpu.flags.zf = 0;
    cpu.flags.sf = 1;
    cpu.esp += 4;
}

// sub_4e1890: the heap's free; its flags and eax are what the caller returns.
void Application::sub_4e1890(WinApplication*, x86::CPU& cpu)
{
    cpu.eax = 0x5a5a5a5a;
    cpu.edx = 0x44444444;
    cpu.flags.cf = 1;
    cpu.flags.zf = 0;
    cpu.flags.sf = 1;
    cpu.flags.of = 1;
    cpu.esp += 4;
}

Application::Application(const char* appName)
    : WinApplication(appName, 0x400000, {})
{
    g_methods = {
        {0x5102a4, &Application::sub_5102a4}, {0x4d18e0, &Application::sub_4d18e0},
        {0x4d1950, &Application::sub_4d1950}, {0x4d19b0, &Application::sub_4d19b0},
        {0x4d1a00, &Application::sub_4d1a00}, {0x4d1a90, &Application::sub_4d1a90},
        {0x4d1ad0, &Application::sub_4d1ad0}, {0x49cfa0, &Application::sub_49cfa0},
        {0x419c20, &Application::sub_419c20}, {0x41d8a0, &Application::sub_41d8a0},
        {0x4e0ec0, &Application::sub_4e0ec0}, {0x4e1620, &Application::sub_4e1620},
        {0x4e1890, &Application::sub_4e1890},
    };
}

}

namespace
{

typedef bool (*Native)(win32::WinApplication*, x86::CPU&);

std::mt19937 g_random(1998);

x86::reg32 random(x86::reg32 below)
{
    return below ? x86::reg32(g_random() % below) : 0;
}

void fillRandom(x86::reg32 at, x86::reg32 size)
{
    for (x86::reg32 i = 0; i < size; ++i)
        g_memory[at + i] = x86::reg8(g_random());
}

x86::CPU entryState()
{
    x86::CPU cpu;
    std::memset(&cpu, 0, sizeof cpu);
    cpu.init(0, 0);
    cpu.ebx = g_random();
    cpu.ecx = g_random();
    cpu.edx = g_random();
    cpu.esi = g_random();
    cpu.edi = g_random();
    cpu.ebp = g_random();
    cpu.esp = kStack;
    // PF and AF as they come: neither side touches them
    cpu.flags.lo = x86::reg8((g_random() & 0xd5) | 0x02);
    cpu.flags.of = g_random() & 1;
    cpu.flags.df = 0;
    return cpu;
}

struct Totals
{
    unsigned cases = 0;
    unsigned failures = 0;
    double generated = 0;
    double native = 0;
};

std::map<std::string, Totals> g_totals;
std::vector<x86::reg8> g_before;
std::vector<x86::reg8> g_original;

/* One case: the generated function at `address` and then `native`, from the
 * same memory and registers. */
bool compare(nfs3hp::Application& app, const char* name, x86::reg32 address, Native native, x86::CPU entry,
             bool expectNative = true)
{
    Totals& totals = g_totals[name];
    ++totals.cases;
    g_before.assign(g_memory, g_memory + kMemory);

    x86::CPU original = entry;
    auto start = std::chrono::steady_clock::now();
    app.dynamic_call(address, original);
    auto end = std::chrono::steady_clock::now();
    totals.generated += std::chrono::duration<double>(end - start).count();
    g_original.assign(g_memory, g_memory + kMemory);

    std::memcpy(g_memory, g_before.data(), kMemory);
    x86::CPU mine = entry;
    start = std::chrono::steady_clock::now();
    const bool took = native(&app, mine);
    end = std::chrono::steady_clock::now();
    totals.native += std::chrono::duration<double>(end - start).count();

    char what[300] = "";
    if (took != expectNative)
    {
        std::snprintf(what, sizeof what, "the native %s", took ? "ran" : "stepped aside");
    }
    else if (!took)
    {
        return true;
    }
    else
    {
        const x86::reg32 stackEnd = original.esp;
        const x86::reg32 stackStart = original.esp - 0x10000;
        unsigned differing = 0;
        for (x86::reg32 chunk = 0; chunk < kMemory; chunk += 0x10000)
        {
            if (std::memcmp(g_memory + chunk, &g_original[chunk], 0x10000) == 0)
                continue;
            for (x86::reg32 i = chunk; i < chunk + 0x10000; ++i)
            {
                if (g_memory[i] == g_original[i] || (i >= stackStart && i < stackEnd))
                    continue;
                if (differing++ == 0)
                    std::snprintf(what, sizeof what, "memory at %08x: %02x native, %02x generated", unsigned(i),
                                  unsigned(g_memory[i]), unsigned(g_original[i]));
            }
        }
        if (differing > 1)
        {
            const std::size_t length = std::strlen(what);
            std::snprintf(what + length, sizeof what - length, " (%u bytes)", differing);
        }
        if (!what[0]
            && (mine.eax != original.eax || mine.ebx != original.ebx || mine.ecx != original.ecx
                || mine.edx != original.edx || mine.esi != original.esi || mine.edi != original.edi
                || mine.ebp != original.ebp || mine.esp != original.esp))
            std::snprintf(what, sizeof what,
                          "registers native/generated: eax %08x/%08x ebx %08x/%08x ecx %08x/%08x edx %08x/%08x "
                          "esi %08x/%08x edi %08x/%08x ebp %08x/%08x esp %08x/%08x",
                          unsigned(mine.eax), unsigned(original.eax), unsigned(mine.ebx), unsigned(original.ebx),
                          unsigned(mine.ecx), unsigned(original.ecx), unsigned(mine.edx), unsigned(original.edx),
                          unsigned(mine.esi), unsigned(original.esi), unsigned(mine.edi), unsigned(original.edi),
                          unsigned(mine.ebp), unsigned(original.ebp), unsigned(mine.esp), unsigned(original.esp));
        if (!what[0] && ((mine.flags.lo & 0xd5) != (original.flags.lo & 0xd5) || mine.flags.of != original.flags.of))
            std::snprintf(what, sizeof what, "flags native/generated: %02x/%02x of %u/%u", unsigned(mine.flags.lo),
                          unsigned(original.flags.lo), unsigned(mine.flags.of), unsigned(original.flags.of));
        if (!what[0] && (mine.fpu.status.word != original.fpu.status.word || mine.fpu.count != original.fpu.count))
            std::snprintf(what, sizeof what, "fpu");
        if (!what[0] && std::memcmp(&mine.mmx, &original.mmx, sizeof mine.mmx) != 0)
            std::snprintf(what, sizeof what, "mmx");
    }
    if (what[0])
    {
        ++totals.failures;
        if (totals.failures <= 10)
            std::printf("FAIL %s case %u: %s\n", name, totals.cases, what);
        return false;
    }
    return true;
}

/* ---- RefPack ----------------------------------------------------------- */

struct RefPackWriter
{
    std::vector<x86::reg8> code;
    x86::reg32 out = 0;

    void literals(const std::vector<x86::reg8>& bytes, std::size_t& at, x86::reg32 count)
    {
        for (x86::reg32 i = 0; i < count; ++i)
            code.push_back(bytes[at++]);
        out += count;
    }
};

/* A stream that unpacks to `size` bytes or a little more, every kind of
 * command in it; `bytes` gives the literals. */
std::vector<x86::reg8> makeRefPack(x86::reg32 size, bool packedSize, x86::reg32& unpacked)
{
    std::vector<x86::reg8> bytes(size + 4096);
    for (auto& b : bytes)
        b = x86::reg8(random(4) == 0 ? g_random() : 'a' + random(4));
    RefPackWriter w;
    std::size_t at = 0;
    while (w.out < size)
    {
        const x86::reg32 kind = w.out < 4 ? 3 : random(8);
        if (kind == 0 || kind == 4)
        {
            const x86::reg32 lits = random(4);
            const x86::reg32 back = 1 + random(std::min<x86::reg32>(1024, w.out + lits));
            const x86::reg32 length = 3 + random(8);
            w.code.push_back(x86::reg8((((back - 1) >> 3) & 0x60) | ((length - 3) << 2) | lits));
            w.code.push_back(x86::reg8(back - 1));
            w.literals(bytes, at, lits);
            w.out += length;
        }
        else if (kind == 1 || kind == 5)
        {
            const x86::reg32 lits = random(4);
            const x86::reg32 back = 1 + random(std::min<x86::reg32>(16384, w.out + lits));
            const x86::reg32 length = 4 + random(64);
            w.code.push_back(x86::reg8(0x80 | (length - 4)));
            w.code.push_back(x86::reg8((lits << 6) | ((back - 1) >> 8)));
            w.code.push_back(x86::reg8(back - 1));
            w.literals(bytes, at, lits);
            w.out += length;
        }
        else if (kind == 2 || kind == 6)
        {
            const x86::reg32 lits = random(4);
            const x86::reg32 limit = std::min<x86::reg32>(131072, w.out + lits);
            // near and far: from 1 back (a run) to as far as there is
            const x86::reg32 back = random(3) == 0 ? 1 + random(std::min<x86::reg32>(8, limit)) : 1 + random(limit);
            const x86::reg32 length = 5 + random(random(4) == 0 ? 1024 : 64);
            w.code.push_back(x86::reg8(0xc0 | (((back - 1) >> 16) & 1) << 4 | (((length - 5) >> 8) & 3) << 2 | lits));
            w.code.push_back(x86::reg8((back - 1) >> 8));
            w.code.push_back(x86::reg8(back - 1));
            w.code.push_back(x86::reg8(length - 5));
            w.literals(bytes, at, lits);
            w.out += length;
        }
        else
        {
            const x86::reg32 dwords = 1 + random(28);
            w.code.push_back(x86::reg8(0xe0 | (dwords - 1)));
            w.literals(bytes, at, dwords * 4);
        }
    }
    const x86::reg32 lits = random(4);
    w.code.push_back(x86::reg8(0xfc | lits));
    w.literals(bytes, at, lits);
    unpacked = w.out;

    std::vector<x86::reg8> stream;
    stream.push_back(packedSize ? 0x11 : 0x10);
    stream.push_back(0xfb);
    if (packedSize)
    {
        const x86::reg32 packed = x86::reg32(w.code.size() + 8);
        stream.push_back(x86::reg8(packed >> 16));
        stream.push_back(x86::reg8(packed >> 8));
        stream.push_back(x86::reg8(packed));
    }
    stream.push_back(x86::reg8(unpacked >> 16));
    stream.push_back(x86::reg8(unpacked >> 8));
    stream.push_back(x86::reg8(unpacked));
    stream.insert(stream.end(), w.code.begin(), w.code.end());
    return stream;
}

void refpackCases(nfs3hp::Application& app)
{
    for (unsigned c = 0; c < 120; ++c)
    {
        const x86::reg32 size = c < 100 ? 1 + random(c < 50 ? 2000 : 200000) : 1500000;
        x86::reg32 unpacked = 0;
        const std::vector<x86::reg8> stream = makeRefPack(size, random(2), unpacked);
        const x86::reg32 source = 0x100000 + random(16);
        const x86::reg32 to = 0x3000000 + random(16);
        std::memset(g_memory + 0x100000, 0, 0x1000000);
        std::memcpy(g_memory + source, stream.data(), stream.size());
        x86::CPU cpu = entryState();
        cpu.eax = source;
        cpu.edx = to;
        cpu.ebx = 1;
        compare(app, "refpack sub_5102a4", 0x5102a4, nfs3hp::refpack, cpu);
        // the size alone
        cpu.ebx = 0;
        compare(app, "refpack sub_5102a4", 0x5102a4, nfs3hp::refpack, cpu);
    }
    // no data at all
    x86::CPU cpu = entryState();
    cpu.eax = 0;
    cpu.ebx = 1;
    compare(app, "refpack sub_5102a4", 0x5102a4, nfs3hp::refpack, cpu);
    // the direction flag set: the generated code's business (which, on this
    // data, copies backwards out of the buffer -- so it is not run here)
    cpu = entryState();
    cpu.eax = 0x100000;
    cpu.edx = 0x3000000;
    cpu.ebx = 1;
    cpu.flags.df = 1;
    Totals& totals = g_totals["refpack sub_5102a4"];
    ++totals.cases;
    if (nfs3hp::refpack(&app, cpu))
    {
        ++totals.failures;
        std::printf("FAIL refpack: the native ran with the direction flag set\n");
    }
}

/* ---- FSH alpha scans ---------------------------------------------------- */

void scanCases(nfs3hp::Application& app)
{
    struct Scan
    {
        const char* name;
        x86::reg32 address;
        Native native;
        x86::reg32 bytes;
    };
    const Scan scans[] = {
        {"fsh 4444 sub_4d18e0", 0x4d18e0, nfs3hp::alpha4444, 2},
        {"fsh 8888 sub_4d1950", 0x4d1950, nfs3hp::alpha8888, 4},
        {"fsh 1555 sub_4d1a90", 0x4d1a90, nfs3hp::alpha1555, 2},
        {"fsh 565 sub_4d1ad0", 0x4d1ad0, nfs3hp::alpha565, 2},
        {"fsh 8-bit sub_4d1a00", 0x4d1a00, nfs3hp::alphaPaletted, 1},
    };
    const x86::reg32 pixels = 0x1000000;
    for (const Scan& scan : scans)
    {
        for (unsigned c = 0; c < 200; ++c)
        {
            x86::reg32 rows = 1 + random(c < 150 ? 64 : 256);
            x86::reg32 columns = 1 + random(c < 150 ? 64 : 256);
            if (c % 25 == 0)
                rows = random(2) ? 0 : x86::reg32(-1 - random(3));
            if (c % 25 == 1)
                columns = random(2) ? 0 : x86::reg32(-1);
            const x86::reg32 count = (x86::sreg32(rows) > 0 && x86::sreg32(columns) > 0) ? rows * columns : 0;
            // opaque, keyed or translucent, and where the first odd pixel lies
            const x86::reg32 kind = random(3);
            const x86::reg32 odd = random(count + 1);
            for (x86::reg32 i = 0; i < count; ++i)
            {
                const x86::reg32 at = pixels + i * scan.bytes;
                const bool here = kind != 0 && (i == odd || (i > odd && random(16) == 0));
                const x86::reg32 v = g_random();
                switch (scan.address)
                {
                case 0x4d18e0:
                {
                    const x86::reg16 p = x86::reg16((v & 0x0fff) | (here ? (kind == 1 ? 0 : (1 + random(14)) << 12) : 0xf000));
                    std::memcpy(g_memory + at, &p, 2);
                    break;
                }
                case 0x4d1950:
                {
                    const x86::reg32 p = (v & 0xffffff) | (here ? (kind == 1 ? 0 : (1 + random(254)) << 24) : 0xff000000);
                    std::memcpy(g_memory + at, &p, 4);
                    break;
                }
                case 0x4d1a90:
                {
                    const x86::reg16 p = x86::reg16((v & 0x7fff) | (here ? 0 : 0x8000));
                    std::memcpy(g_memory + at, &p, 2);
                    break;
                }
                case 0x4d1ad0:
                {
                    x86::reg16 p = x86::reg16(v);
                    if (p == 0x7c0 || p == 0x7e0)
                        p = 0;
                    if (here)
                        p = random(2) ? 0x7c0 : 0x7e0;
                    std::memcpy(g_memory + at, &p, 2);
                    break;
                }
                default:
                    g_memory[at] = x86::reg8(here ? (random(2) ? 0xff : random(4)) : 4 + random(251));
                    break;
                }
            }
            // the palette: 0-3 keyed (alpha 0) or translucent, the rest opaque
            const x86::reg32 palette = 0x1800000;
            for (x86::reg32 i = 0; i < 256; ++i)
            {
                x86::reg32 colour = g_random() & 0xffffff;
                colour |= (i < 4 ? (kind == 1 ? 0 : 1 + random(254)) : 0xff) << 24;
                std::memcpy(g_memory + palette + i * 4, &colour, 4);
            }
            nfs3hp::setWord(0x563a88, c % 40 == 7 ? 0 : palette);
            x86::CPU cpu = entryState();
            cpu.eax = pixels;
            cpu.edx = rows;
            cpu.ebx = columns;
            compare(app, scan.name, scan.address, scan.native, cpu);
        }
    }
    // 0x7a: the width in edx, rows in ebx, until a byte with a nibble of 0
    for (unsigned c = 0; c < 200; ++c)
    {
        const x86::reg32 bytes = 1 + random(4000);
        for (x86::reg32 i = 0; i < bytes; ++i)
            g_memory[pixels + i] = x86::reg8(0x11 + random(0xee) | 0x11);
        g_memory[pixels + bytes] = random(2) ? 0x0f : 0xa0;
        x86::CPU cpu = entryState();
        cpu.eax = pixels;
        const x86::reg32 widths[] = {0, x86::reg32(-1), 1, 2, 7, 8, 64, 100};
        cpu.edx = widths[random(8)];
        cpu.ebx = c % 20 == 0 ? x86::reg32(-1) : random(c % 3 ? 4 : 2);
        compare(app, "fsh 0x7a sub_4d19b0", 0x4d19b0, nfs3hp::alphaNibbles, cpu);
    }
}

/* ---- FCE -------------------------------------------------------------- */

void fceCases(nfs3hp::Application& app)
{
    for (unsigned c = 0; c < 200; ++c)
    {
        const x86::reg32 fce = 0x1000000 + random(4) * 4;
        std::memset(g_memory + 0x1000000, 0, 0x400000);
        fillRandom(fce, 0x1f04);
        const x86::reg32 parts = c % 30 == 0 ? (random(2) ? 0 : x86::reg32(-2)) : 1 + random(8);
        const x86::sreg32 partCount = x86::sreg32(parts) > 0 ? x86::sreg32(parts) : 0;
        x86::reg32 vertices = 0;
        x86::reg32 triangles = 0;
        for (x86::sreg32 p = 0; p < partCount; ++p)
        {
            const x86::reg32 v = random(40);
            const x86::reg32 t = random(60);
            nfs3hp::setWord(fce + 0x3fc + p * 4, vertices);
            nfs3hp::setWord(fce + 0x4fc + p * 4, v);
            nfs3hp::setWord(fce + 0x5fc + p * 4, triangles);
            nfs3hp::setWord(fce + 0x6fc + p * 4, t);
            vertices += v;
            triangles += t;
        }
        nfs3hp::setWord(fce + 0x04, triangles);
        nfs3hp::setWord(fce + 0x08, vertices);
        nfs3hp::setWord(fce + 0xf8, parts);
        const x86::reg32 vertexTable = vertices * 12;
        const x86::reg32 triangleTable = vertexTable * 2 + random(64) * 4;
        const x86::reg32 reserve = triangleTable + triangles * 0x38;
        nfs3hp::setWord(fce + 0x10, 0);
        nfs3hp::setWord(fce + 0x18, triangleTable);
        nfs3hp::setWord(fce + 0x1c, reserve);
        fillRandom(fce + 0x1f04, reserve + vertices * 0x20);
        // vertex indices within the part
        for (x86::reg32 t = 0; t < triangles; ++t)
            for (x86::reg32 k = 1; k <= 3; ++k)
                nfs3hp::setWord(fce + 0x1f04 + triangleTable + t * 0x38 + k * 4, random(40));
        nfs3hp::setWord(kHeapNext, kHeap);
        x86::CPU cpu = entryState();
        cpu.eax = fce;
        compare(app, "fce sub_49cfa0", 0x49cfa0, nfs3hp::fceRecords, cpu);
    }
}

/* ---- FRD -------------------------------------------------------------- */

struct FrdWriter
{
    std::vector<x86::reg8> data;
    // every count 0: nothing read into what the arena hands out
    bool empty = false;
    void bytes(x86::reg32 count)
    {
        for (x86::reg32 i = 0; i < count; ++i)
            data.push_back(x86::reg8(g_random()));
    }
    void word(x86::reg32 value)
    {
        for (int i = 0; i < 4; ++i)
            data.push_back(x86::reg8(value >> (i * 8)));
    }
    x86::reg32 count(x86::reg32 below)
    {
        const x86::reg32 n = empty ? 0 : random(below);
        word(n);
        return n;
    }
    void objects()
    {
        const x86::reg32 n = count(4);
        for (x86::reg32 i = 0; i < n; ++i)
        {
            bytes(4 + 4 + 4 + 12);
            const x86::reg32 extra = empty || random(3) ? 0 : 1 + random(20);
            word(extra);
            bytes(extra);
            const x86::reg32 vertices = count(8);
            bytes(vertices * 12 + vertices * 4);
            bytes(count(6) * 14);
        }
    }
};

std::vector<x86::reg8> makeFrd(x86::reg32 lastBlock, bool empty)
{
    FrdWriter w;
    w.empty = empty;
    w.bytes(28);
    w.word(lastBlock);
    for (x86::reg32 b = 0; b <= lastBlock; ++b)
    {
        w.bytes(0xc + 0x30);
        const x86::reg32 vertices = w.count(30);
        w.bytes(0x14);
        w.bytes(vertices * 12 + vertices * 4);
        w.bytes(0x4b0);
        w.bytes(4);
        const x86::reg32 sizes[7] = {8, 8, 12, 20, 20, 16, 16};
        x86::reg32 counts[7];
        for (x86::reg32 i = 0; i < 7; ++i)
            counts[i] = w.count(6);
        for (x86::reg32 i = 0; i < 7; ++i)
            w.bytes(counts[i] * sizes[i]);
    }
    for (x86::reg32 b = 0; b <= lastBlock; ++b)
    {
        for (x86::reg32 level = 0; level < 11; ++level)
        {
            const x86::reg32 polygons = empty || random(3) == 0 ? 0 : 1 + random(12);
            w.word(polygons);
            if (polygons == 0)
                continue;
            if (level <= 6)
            {
                const x86::reg32 n = random(polygons + 1);
                w.word(n);
                w.bytes(n * 14);
                continue;
            }
            // in pieces, only those flagged 1 read
            const x86::reg32 pieces = random(4);
            w.word(pieces);
            x86::reg32 left = polygons;
            for (x86::reg32 p = 0; p < pieces; ++p)
            {
                const bool read = random(3) != 0;
                w.word(read ? 1 : random(2) ? 0 : 2);
                if (!read)
                    continue;
                const x86::reg32 n = random(left + 1);
                left -= n;
                w.word(n);
                w.bytes(n * 14);
            }
        }
    }
    for (x86::reg32 b = 0; b <= lastBlock; ++b)
        for (int i = 0; i < 4; ++i)
            w.objects();
    w.objects();
    const x86::reg32 lights = w.count(6);
    for (x86::reg32 i = 0; i < lights; ++i)
    {
        const std::size_t at = w.data.size();
        w.bytes(0x2f);
        w.data[at + 0x2c] = random(2) ? 0 : 1 + random(3);
        w.data[at + 0x2d] = x86::reg8(random(8));
        w.data[at + 0x2e] = 0;
    }
    w.bytes(64);
    return w.data;
}

void frdCases(nfs3hp::Application& app)
{
    // the tables the lights' kinds look up
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        nfs3hp::setWord(0x8b41a0 + i * 4, 0x2a00000 + i * 0x40);
        nfs3hp::setWord(0x2a00000 + i * 0x40 + 4, 0xabc00000 + i);
        nfs3hp::setWord(0x2b00000 + i * 0x2c + 4, 0xdef00000 + i);
    }
    nfs3hp::setWord(0x5dd858, 0x2b00000);
    for (unsigned c = 0; c < 60; ++c)
    {
        const x86::reg32 lastBlock = c % 20 == 0 ? 0 : random(c < 40 ? 6 : 40);
        // the arena full now and then: its allocations then fail, 0 -- with
        // nothing to read into them, as anything read to 0 would be the end
        // of the generated code as well
        const bool full = c % 15 == 5;
        const std::vector<x86::reg8> file = makeFrd(lastBlock, full);
        const x86::reg32 at = 0x1000000 + random(4);
        std::memset(g_memory + 0x1000000, 0, 0x1000000);
        std::memcpy(g_memory + at, file.data(), file.size());
        nfs3hp::setWord(kFileAt, c % 25 == 3 ? 0 : at);
        nfs3hp::setWord(0x552dfc, 0x2000000);
        nfs3hp::setWord(0x552e00, full ? 0x310000 + random(2) : 0);
        // MMX (the port's processor) or neither MMX nor the FPU's copy
        nfs3hp::setWord(0x5643b4, c % 4 == 1 ? 0 : 1);
        nfs3hp::setWord(0x5643ac, 0);
        x86::CPU cpu = entryState();
        // something in the MMX registers for the emms to clear
        cpu.mmx.mm3 = x86::from_reg64(g_random());
        if (nfs3hp::word(kFileAt) == 0)
        {
            // no file: the generated code reads from address 0, so does the native
            std::memcpy(g_memory, file.data(), std::min<std::size_t>(file.size(), 0x100000));
        }
        compare(app, "frd sub_419c20", 0x419c20, nfs3hp::frd, cpu);
    }
    // the FPU's copy: the generated code's business
    nfs3hp::setWord(0x5643b4, 0);
    nfs3hp::setWord(0x5643ac, 1);
    x86::CPU cpu = entryState();
    Totals& totals = g_totals["frd sub_419c20"];
    ++totals.cases;
    if (nfs3hp::frd(&app, cpu))
    {
        ++totals.failures;
        std::printf("FAIL frd: the native ran where sub_4ea4f0 copies by the FPU\n");
    }
}

}

int main()
{
    nfs3hp::Application app("loading_checks");
    refpackCases(app);
    scanCases(app);
    fceCases(app);
    frdCases(app);
    unsigned failures = 0;
    for (const auto& entry : g_totals)
    {
        const Totals& t = entry.second;
        std::printf("%-24s %4u cases, %u differ; generated %.1f ms, native %.1f ms (x%.1f)\n", entry.first.c_str(),
                    t.cases, t.failures, t.generated * 1000, t.native * 1000,
                    t.native > 0 ? t.generated / t.native : 0.0);
        failures += t.failures;
    }
    std::printf(failures ? "FAILED\n" : "all match\n");
    return failures ? 1 : 0;
}
