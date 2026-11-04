/* native_vertices.cpp's natives against the game's generated code, made
 * afresh (gen_vertices.py) -- the port's build has none of it since they are
 * native whole -- on the desktop.
 *
 * Two guest memories of 16 MB hold the same state: the game's data as it
 * loads, and a world of records made up for each case.  The native runs on
 * one, the generated code on the other, both from the same registers and the
 * same x87 control word; then every byte of memory (all but the stack below
 * the esp the call returns with), the registers, the flags (CF PF AF ZF SF
 * OF), the x87's control and status words, its depth and live registers, and
 * every call out (THRASH's, the game's functions left as stand-ins) are
 * compared.  The functions the natives call that both sides run as the game's
 * code are gen_vertices.py's PIECES.
 *
 *     vcheck [cases] [name ...] */
#include "nfs3hp.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace x86
{
void assertLog(const char* file, int line, const char* expr)
{
    std::printf("assert %s:%d %s\n", file, line, expr);
}
}

extern "C"
{
Uint64 SDL_GetPerformanceCounter(void) { return 0; }
Uint64 SDL_GetPerformanceFrequency(void) { return 1; }
const char* SDL_getenv(const char* name) { return std::getenv(name); }
int SDL_strcmp(const char* a, const char* b) { return std::strcmp(a, b); }
void SDL_Log(SDL_PRINTF_FORMAT_STRING const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::vprintf(fmt, ap);
    va_end(ap);
    std::printf("\n");
}
int SDL_snprintf(SDL_OUT_Z_CAP(maxlen) char* text, size_t maxlen, SDL_PRINTF_FORMAT_STRING const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    const int n = std::vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);
    return n;
}
Uint64 SDL_GetTicks(void) { return 0; }
Uint64 SDL_GetTicksNS(void) { return 0; }
unsigned long SDL_strtoul(const char* s, char** end, int base) { return std::strtoul(s, end, base); }
SDL_IOStream* SDL_IOFromFile(const char*, const char*) { return nullptr; }
size_t SDL_WriteIO(SDL_IOStream*, const void*, size_t) { return 0; }
bool SDL_FlushIO(SDL_IOStream*) { return true; }
}

struct GameSegment
{
    x86::reg32 address, length;
    const x86::reg8* bytes;
};
extern const GameSegment kGameSegments[];
extern const unsigned kGameSegmentCount;

namespace nfs3hp
{
x86::reg32 mirrorAsMain(x86::reg32 kind) { return kind == 1 ? 0 : kind; }
double mirrorCarScale(double scale) { return scale; }
}

namespace
{
constexpr x86::reg32 kMemory = 0x1000000;
constexpr x86::reg32 kStackLow = 0xef0000, kStackHigh = 0xf00000;
constexpr x86::reg32 kThrash = 0xfe0000;  // THRASH's pointers lead here: kThrash + slot - 0x9ef900

typedef void (*Function)(win32::WinApplication*, x86::CPU&);

struct Tested
{
    x86::reg32 address;
    const char* name;
    Function generated;
    Function native;
};
const Tested kTested[] = {
#include "tested.inc"
};
struct Piece
{
    x86::reg32 address;
    Function generated;
};
const Piece kPieces[] = {
#include "pieces.inc"
};

struct Call
{
    std::vector<x86::reg32> words;
    bool operator==(const Call& o) const { return words == o.words; }
};

unsigned g_nanOnly = 0;    // NaNs alike but for their sign or payload
unsigned g_stopped = 0;     // cases where the game stopped in a callee
}

namespace win32
{
struct VApp : WinApplication
{
    VApp() : WinApplication("v", 0x400000, {}) {}
    bool generated = false;
    std::vector<Call> calls;
};

WinApplication::WinApplication(const char*, x86::reg32, const std::vector<Section>&)
{
    m_memory = static_cast<x86::reg8*>(std::calloc(kMemory, 1));
    if (!m_memory)
        std::abort();
}
WinApplication::~WinApplication() {}
int WinApplication::s_traceApi = 0;
bool WinApplication::readTraceApi() { return false; }
void WinApplication::traceCall(const Method&, x86::reg32) {}
void WinApplication::traceNewCalls() {}
void WinApplication::yieldContext(const x86::CPU&) {}
void WinApplication::lockContext(const x86::CPU&) {}
void WinApplication::unlockContext(const x86::CPU&) {}

std::uint64_t mixHash(std::uint64_t x)
{
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    return x ^ (x >> 31);
}

void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    VApp* app = static_cast<VApp*>(this);
    for (const Tested& t : kTested)
        if (t.address == address)
        {
            if (app->generated)
                t.generated(this, cpu);
            else
                t.native(this, cpu);
            return;
        }
    for (const Piece& p : kPieces)
        if (p.address == address)
        {
            p.generated(this, cpu);
            return;
        }
    // A stand-in: logged with its registers and its first stack words; eax
    // (and for THRASH's, stdcall, ecx and edx) made up from them.
    // THRASH's (stdcall) read only their arguments: those are logged; the
    // game's own stand-ins (only called from the shared pieces) their
    // registers and two words besides.
    if (app->calls.size() > 20000)
    {
        // a loop that never ends on made-up data: the game stopped, on both sides
        cpu.terminate = true;
        return;
    }
    Call call;
    x86::reg32 words = 0;
    const bool thrash = address >= kThrash && address < kThrash + 0x100;
    if (thrash)
    {
        switch (address - kThrash + 0x9ef900)
        {
        case 0x9ef96c: words = 2; break;  // setstate
        case 0x9ef974: words = 3; break;  // drawtri
        case 0x9ef94c: words = 4; break;  // drawquad
        case 0x9ef960: words = 2; break;  // drawtrifan
        case 0x9ef930: words = 3; break;  // drawquadmesh
        default: std::printf("THRASH slot %x\n", unsigned(address - kThrash + 0x9ef900)); std::exit(2);
        }
        call.words = {address};
        for (x86::reg32 i = 1; i <= words; ++i)
            call.words.push_back(getMemory<x86::reg32>(cpu.esp + 4 * i));
    }
    else
    {
        // The game's own, called only from the pieces both sides run alike:
        // as each is called, what it reads logged.
        std::uint64_t r = app->calls.size() * 0x9e3779b97f4a7c15ull + address;
        r = mixHash(r);
        switch (address)
        {
        case 0x4ff624:  // fpatan without an x87
        {
            call.words = {address};
            const x86::Float y = cpu.fpu.st(1), x = cpu.fpu.st(0);
            cpu.fpu.count -= 1;
            cpu.fpu.st(0) = cpu.fpu.atan(x, y);
            app->calls.push_back(call);
            cpu.esp += 4;
            return;
        }
        case 0x4eb230:  // a random float in [0, 1) on the x87 stack
            call.words = {address};
            app->calls.push_back(call);
            cpu.fpu.push(x86::Float(double(r >> 11) / 9007199254740992.0));
            cpu.esp += 4;
            return;
        case 0x4e0d88:  // a random integer
            call.words = {address};
            app->calls.push_back(call);
            cpu.eax = x86::reg32(r & 0x7fff);
            cpu.edx = 0;
            cpu.esp += 4;
            return;
        case 0x4ca850:  // a particle spawned (stdcall, 13 words): none
            call.words = {address};
            for (x86::reg32 i = 1; i <= 13; ++i)
                call.words.push_back(getMemory<x86::reg32>(cpu.esp + 4 * i));
            app->calls.push_back(call);
            cpu.eax = 0;
            cpu.esp += 4 + 13 * 4;
            return;
        case 0x497500:
        case 0x415010:
            call.words = {address, cpu.eax, cpu.edx};
            app->calls.push_back(call);
            cpu.esp += 4;
            return;
        default:
            call.words = {address, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, getMemory<x86::reg32>(cpu.esp + 4),
                          getMemory<x86::reg32>(cpu.esp + 8)};
        }
    }
    std::uint64_t h = app->calls.size();
    for (x86::reg32 w : call.words)
        h = mixHash(h ^ w);
    app->calls.push_back(call);
    cpu.eax = x86::reg32(h);
    if (thrash)
    {
        cpu.ecx = x86::reg32(h >> 32);
        cpu.edx = x86::reg32(h >> 16);
    }
    cpu.esp += 4 + 4 * words;
}
}  // namespace win32

/* The game's functions the generated code calls by address: stand-ins. */
#define STAND_IN(a)                                                                      \
    void nfs3hp::Application::sub_##a(WinApplication* app, x86::CPU& cpu)               \
    {                                                                                    \
        app->dynamic_call(0x##a, cpu);                                                   \
    }
#include "called.inc"

namespace
{
using win32::VApp;

std::mt19937 g_random(1998);
x86::reg32 random32() { return x86::reg32(g_random()); }
x86::reg32 below(x86::reg32 n) { return n ? x86::reg32(g_random() % n) : 0; }
bool chance(int percent) { return int(g_random() % 100) < percent; }

/* A float of the sizes the game's have, now and then an odd one. */
float aFloat(float scale = 100.0f)
{
    const x86::reg32 r = random32();
    if (r % 100 < 3)
    {
        static const x86::reg32 odd[] = {0, 0x80000000, 0x3f800000, 0xbf800000, 0x00000001, 0x7f7fffff,
                                         0x00800000, 0x7f800000, 0x34000000, 0x37800080, 0x7fc00000, 0xffc00000};
        const x86::reg32 bits = odd[below(sizeof odd / sizeof odd[0])];
        float f;
        std::memcpy(&f, &bits, 4);
        return f;
    }
    if (r % 100 < 10)
        return float(int(below(2001)) - 1000) / 8.0f;  // exact, ties likely
    const double u = double(random32()) / 4294967296.0;
    const double size = std::pow(10.0, u * 6.0 - 3.0);  // 0.001 .. 1000
    return float((random32() & 1 ? -size : size) * scale / 100.0);
}
float positive(float scale = 100.0f) { return std::fabs(aFloat(scale)); }
float unit() { return float(double(random32()) / 4294967296.0 * 2.0 - 1.0); }

struct World
{
    VApp& a;
    x86::reg32 heap = 0xa40000;  // up to 0xee0000
    x86::reg32 alloc(x86::reg32 n)
    {
        const x86::reg32 at = heap;
        heap += (n + 15) & ~15u;
        if (heap > 0xee0000)
            std::abort();
        return at;
    }
    x86::reg32& w(x86::reg32 at) { return *reinterpret_cast<x86::reg32*>(&a.getMemory<x86::reg8>(at)); }
    x86::reg16& h(x86::reg32 at) { return *reinterpret_cast<x86::reg16*>(&a.getMemory<x86::reg8>(at)); }
    x86::reg8& b(x86::reg32 at) { return *reinterpret_cast<x86::reg8*>(&a.getMemory<x86::reg8>(at)); }
    float& f(x86::reg32 at) { return *reinterpret_cast<float*>(&a.getMemory<x86::reg8>(at)); }
    double& d(x86::reg32 at) { return *reinterpret_cast<double*>(&a.getMemory<x86::reg8>(at)); }
    void floats(x86::reg32 at, x86::reg32 n, float scale = 100.0f)
    {
        for (x86::reg32 i = 0; i < n; ++i)
            f(at + 4 * i) = aFloat(scale);
    }
    void randomBytes(x86::reg32 at, x86::reg32 n)
    {
        for (x86::reg32 i = 0; i < n; i += 4)
            w(at + i) = random32();
    }
    x86::reg32 matrix()
    {
        const x86::reg32 m = alloc(36);
        for (x86::reg32 i = 0; i < 9; ++i)
            f(m + 4 * i) = chance(90) ? unit() : aFloat(2.0f);
        return m;
    }
    x86::reg32 vectors(x86::reg32 n, float scale = 100.0f)
    {
        const x86::reg32 v = alloc(12 * n + 12);
        floats(v, 3 * n + 3, scale);
        return v;
    }
    /* A screen vertex record of 0x20 bytes: x, y, z, 1/z, colour, code, s, t. */
    x86::reg32 screenVertex()
    {
        const x86::reg32 v = alloc(0x20);
        f(v) = chance(80) ? float(below(800)) - 80.0f + unit() : aFloat(1000.0f);
        f(v + 4) = chance(80) ? float(below(600)) - 60.0f + unit() : aFloat(1000.0f);
        f(v + 8) = positive(50.0f);
        f(v + 0xc) = chance(85) ? std::fabs(unit()) * 1.2f : aFloat(1.0f);
        w(v + 0x10) = random32();
        w(v + 0x14) = chance(60) ? 0 : (chance(50) ? 1u << below(6) : random32() & 0x3f);
        f(v + 0x18) = unit() * 256.0f;
        f(v + 0x1c) = unit() * 256.0f;
        return v;
    }
};

/* The globals most of them read. */
void screenGlobals(World& g)
{
    g.f(0x56009c) = 320.0f + unit();
    g.f(0x5600a0) = -240.0f + unit();
    g.f(0x5600a4) = 320.0f;
    g.f(0x5600a8) = 240.0f;
    // the clip bounds, compared as integers: 1/z's limit, left, top, bottom, right
    g.f(0x7d34f8) = chance(80) ? 2.0f : positive(10.0f);
    g.f(0x7d34fc) = 0.0f;
    g.f(0x7d3504) = 0.0f;
    g.f(0x7d3500) = 479.0f;
    g.f(0x7d350c) = 639.0f;
    // the clipper's planes: near w, top, bottom, left, right
    g.f(0x7d34f0) = chance(80) ? 1.0f : positive(2.0f);
    g.f(0x7d34dc) = 0.0f;
    g.f(0x7d34e0) = 479.0f;
    g.f(0x7d34f4) = 0.0f;
    g.f(0x7d34ec) = 639.0f;
    g.f(0x560094) = chance(80) ? 65535.0f : aFloat(70000.0f);
    g.f(0x560098) = chance(80) ? 0.0f : aFloat(10.0f);
    // the frame's arena (Render_GetTm's buffer)
    g.w(0x55fe34) = 0xc00000;
    g.w(0x55fe38) = 0xc00000;
    g.w(0x7a3d04) = chance(95) ? 0x200000 : below(0x400);
    // THRASH's pointers
    for (x86::reg32 slot = 0x9ef900; slot < 0x9ef988; slot += 4)
        g.w(slot) = kThrash + slot - 0x9ef900;
    g.f(0x6fbc38) = chance(50) ? 1.0f : std::fabs(unit());
    g.b(0x7a3a58) = x86::reg8(random32());
    g.w(0x554e4c) = below(2);
    g.w(0x7d3684) = random32() & 0xffff;
}

struct Args
{
    x86::reg32 stack[10] = {};
};

/* Each function's world: its registers and stack words, and what they point at. */
void makeCase(World& g, const std::string& name, x86::CPU& cpu, Args& a)
{
    screenGlobals(g);
    auto count = [](x86::reg32 most) -> x86::reg32 { return chance(5) ? x86::reg32(0) - below(2) : below(most + 1); };
    if (name == "rotateVectors")
    {
        const x86::reg32 n = count(12);
        cpu.eax = n;
        cpu.edx = g.vectors(16);
        cpu.ebx = g.matrix();
        cpu.ecx = chance(30) ? cpu.edx : g.alloc(12 * 16);
    }
    else if (name == "dotProducts")
    {
        cpu.eax = count(12);
        cpu.edx = g.vectors(16);
        cpu.ebx = g.vectors(1, 2.0f);
        cpu.ecx = g.alloc(4 * 16);
    }
    else if (name == "multiplyMatrices")
    {
        const x86::reg32 m1 = g.matrix(), m2 = g.matrix(), m3 = g.alloc(36);
        const x86::reg32 kind = below(4);
        cpu.eax = m1;
        cpu.edx = kind == 3 ? m1 : m2;
        cpu.ebx = kind == 0 ? m3 : kind == 1 ? m1 : kind == 2 ? m2 : m1;
    }
    else if (name == "transformPoints")
    {
        cpu.eax = count(12);
        cpu.edx = g.vectors(16);
        cpu.ebx = g.matrix();
        cpu.ecx = g.vectors(1);
        a.stack[0] = chance(30) ? cpu.edx : g.alloc(12 * 16);
    }
    else if (name == "scaleVectors")
    {
        cpu.eax = count(12);
        cpu.edx = g.vectors(16);
        float s = aFloat(5.0f);
        std::memcpy(&a.stack[0], &s, 4);
        a.stack[1] = chance(30) ? cpu.edx : g.alloc(12 * 16);
    }
    else if (name == "addVectors" || name == "subtractVectors")
    {
        cpu.eax = count(12);
        cpu.edx = g.vectors(16);
        cpu.ebx = g.vectors(1);
        cpu.ecx = chance(30) ? cpu.edx : g.alloc(12 * 16);
    }
    else if (name == "vectorLengthOf" || name == "normalise")
    {
        cpu.eax = g.vectors(1);
        if (chance(10))
            for (int i = 0; i < 3; ++i)
                g.w(cpu.eax + 4 * i) = chance(50) ? 0 : 0x80000000;
    }
    else if (name == "unitVector")
    {
        cpu.eax = g.vectors(1);
        cpu.edx = chance(30) ? cpu.eax : g.alloc(12);
    }
    else if (name == "dotPush")
    {
        cpu.eax = g.vectors(1);
        cpu.edx = g.vectors(1);
    }
    else if (name == "groundHeight")
    {
        cpu.eax = g.matrix();
        for (int i = 0; i < 6; ++i)
        {
            float v = aFloat(1000.0f);
            std::memcpy(&a.stack[i], &v, 4);
        }
    }
    else if (name == "groundDistanceOf")
    {
        cpu.eax = g.alloc(12);
        cpu.edx = g.alloc(12);
        g.randomBytes(cpu.eax, 12);
        g.randomBytes(cpu.edx, 12);
    }
    else if (name == "nearestBlock")
    {
        cpu.eax = g.vectors(1, 3000.0f);
        const x86::reg32 n = below(30);
        g.w(0x5dd940) = n;
        const x86::reg32 blocks = g.alloc(36 * n + 36);
        g.w(0x5dd948) = blocks;
        for (x86::reg32 i = 0; i < n; ++i)
            for (x86::reg32 k = 0; k < 3; ++k)
                g.w(blocks + 36 * i + 4 * k) = random32() >> (chance(50) ? 4 : 0);
    }
    else if (name == "trackVertices")
    {
        const x86::reg32 view = g.alloc(0x80);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(view + 0x28 + 4 * i) = unit();
        g.floats(view + 0x58, 3, 50.0f);
        g.w(view + 0x70) = random32();
        g.w(view + 0x74) = chance(50) ? 0 : random32();
        g.f(0x552e04) = positive(50.0f);
        g.f(0x552e10) = positive(0.01f);
        g.f(0x552e0c) = positive(1.0f);
        g.f(0x552e08) = positive(500.0f);
        const x86::reg32 n = below(16);
        cpu.eax = view;
        cpu.edx = n;
        cpu.ebx = g.vectors(n, 100.0f);
        cpu.ecx = g.alloc(0x1c * n + 0x1c);
        a.stack[0] = g.alloc(0x20 * n + 0x20);
        g.floats(a.stack[0], 8 * n + 8, 1.0f);
    }
    else if (name == "objectVertices")
    {
        const x86::reg32 n = count(16);
        cpu.eax = n;
        cpu.edx = g.vectors(17, 20.0f);
        cpu.ebx = g.matrix();
        cpu.ecx = g.vectors(1, 50.0f);
        a.stack[0] = g.alloc(0x20 * 17);
    }
    else if (name == "objectVerticesNearest")
    {
        const x86::reg32 n = count(16);
        cpu.eax = n;
        cpu.ebx = g.alloc(0x20 * 17);
        cpu.ecx = g.matrix();
        cpu.edx = g.vectors(17, 20.0f);
        a.stack[0] = g.vectors(1, 50.0f);
        a.stack[1] = random32();
    }
    else if (name == "objectVerticesColoured")
    {
        const x86::reg32 view = g.alloc(0x80);
        g.floats(view + 0x38, 3, 50.0f);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(view + 0x44 + 4 * i) = unit();
        const x86::reg32 n = below(16);
        cpu.eax = view;
        cpu.edx = n;
        cpu.ebx = g.vectors(17, 20.0f);
        cpu.ecx = g.alloc(4 * 17);
        g.randomBytes(cpu.ecx, 4 * 17);
        a.stack[0] = g.alloc(0x20 * 17);
    }
    else if (name == "objectVerticesList")
    {
        const x86::reg32 n = chance(30) ? 21 + below(10) : below(21);  // a count below 0 loops for good
        cpu.eax = g.matrix();
        cpu.edx = g.vectors(1, 50.0f);
        cpu.ebx = n;
        cpu.ecx = g.vectors(32, 20.0f);
        a.stack[0] = g.alloc(4 * 32);
        g.randomBytes(a.stack[0], 4 * 32);
        a.stack[1] = g.alloc(0x20 * 32);
    }
    else if (name == "envMapCoords")
    {
        const x86::reg32 n = below(16);
        cpu.eax = n;
        cpu.edx = g.vectors(16, 1.0f);
        cpu.ebx = g.alloc(12 * 16);
        cpu.ecx = g.alloc(0x20 * 16);
        a.stack[0] = g.matrix();
    }
    else if (name == "shadeNormals")
    {
        const x86::reg32 n = below(16);
        const x86::reg32 car = g.alloc(8);
        const x86::reg32 records = g.alloc(0x84 * 4);
        g.w(car) = records;
        g.w(car + 4) = below(3);
        for (x86::reg32 i = 0; i < 3; ++i)
            g.f(0x55e6a0 + 4 * i) = unit();
        for (x86::reg32 at = 0x55e6b8; at <= 0x55e6e8; at += 4)
            g.w(at) = below(300);
        g.w(0x55e6e4) = random32() & 0xff000000;
        cpu.eax = car;
        cpu.edx = n;
        cpu.ebx = g.vectors(16, 1.0f);
        cpu.ecx = g.alloc(12 * 16);
        a.stack[0] = g.alloc(0x20 * 16);
        a.stack[1] = g.matrix();
    }
    else if (name == "cullPolygons")
    {
        const x86::reg32 object = g.alloc(0x1f04);
        const x86::reg32 part = below(4);
        const x86::reg32 n = below(10);
        const x86::reg32 polygons = g.alloc(0x84 * 12);
        g.w(object) = polygons;
        g.w(object + part * 4 + 0x5fc) = below(2);
        g.w(object + part * 4 + 0x6fc) = n;
        g.randomBytes(object + 0x1e04, 0x100);
        x86::reg32 vertices[8];
        for (x86::reg32& v : vertices)
            v = g.screenVertex();
        for (x86::reg32 i = 0; i < 12; ++i)
        {
            const x86::reg32 p = polygons + 0x84 * i;
            g.randomBytes(p, 0x84);
            for (x86::reg32 k = 0; k < 3; ++k)
                g.w(p + 8 + 4 * k) = vertices[below(8)];
        }
        g.w(0x7a3d0c) = chance(50) ? 0 : g.alloc(4);
        g.w(0x7a3d08) = g.alloc(4);
        cpu.eax = object;
        cpu.edx = part;
        cpu.ebx = chance(50) ? 0 : random32();
    }
    else if (name == "boxOutside")
    {
        const x86::reg32 view = g.alloc(8);
        g.w(view) = chance(70) ? 0 : 1;
        g.w(view + 4) = below(2);
        for (x86::reg32 i = 0; i < 2; ++i)
        {
            const x86::reg32 camera = g.alloc(0x60);
            g.f(camera + 0x54) = chance(30) ? 0.0f : positive(2.0f);
            g.w(0x5e10b8 + 4 * i) = chance(50) ? 0 : camera;
            g.w(0x5e10b0 + 4 * i) = camera;
        }
        cpu.eax = view;
        cpu.edx = g.vectors(4, 50.0f);
        cpu.ebx = g.matrix();
        cpu.ecx = g.vectors(1, 50.0f);
    }
    else if (name == "clipPolygon")
    {
        const x86::reg32 list = g.alloc(12);
        const x86::reg32 codes = g.alloc(4);
        x86::reg8 any = 0;
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            g.w(list + 4 * i) = g.screenVertex();
            const x86::reg8 code = x86::reg8(chance(50) ? 0 : random32() & 0x1f);
            g.b(codes + i) = code;
            any |= code;
        }
        cpu.eax = list;
        cpu.ebx = (random32() & 0xffffff00) | (chance(90) ? any : x86::reg8(random32() & 0x1f));
        cpu.edx = codes;
    }
    else if (name == "clipTriangle" || name == "clipQuad" || name == "clipTriangleByZ")
    {
        x86::reg32 v[4];
        for (x86::reg32& x : v)
        {
            x = g.screenVertex();
            if (chance(60))
                g.w(x + 0x14) = 0;
        }
        cpu.eax = v[0];
        cpu.edx = v[1];
        cpu.ebx = v[2];
        if (name == "clipQuad")
            cpu.ecx = v[3];
    }
    else if (name == "quadMesh")
    {
        x86::reg32 base = g.alloc(0x20 * 16);
        for (x86::reg32 i = 0; i < 16; ++i)
        {
            const x86::reg32 v = g.screenVertex();
            std::memcpy(&g.a.getMemory<x86::reg8>(base + 0x20 * i), &g.a.getMemory<x86::reg8>(v), 0x20);
            if (chance(70))
                g.w(base + 0x20 * i + 0x14) = 0;
        }
        const x86::reg32 n = 1 + below(6);
        const x86::reg32 quads = g.alloc(0x10 * n);
        for (x86::reg32 i = 0; i < 4 * n; ++i)
            g.w(quads + 4 * i) = below(16);
        cpu.eax = n;
        cpu.edx = base;
        cpu.ebx = quads;
    }
    else if (name == "triangleList" || name == "quadList")
    {
        const bool quads = name == "quadList";
        const x86::reg32 n = 1 + below(5);
        x86::reg32 first = 0, previous = 0;
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 r = g.alloc(0x30);
            g.randomBytes(r, 0x30);
            g.h(r + 4) = i == 0 || chance(90) ? (quads ? 0 : 1) : 5;
            g.b(r + 6) = x86::reg8(random32() & (quads ? 0x17 : 0x17));
            for (x86::reg32 k = 0; k < (quads ? 4u : 3u); ++k)
            {
                const x86::reg32 v = g.screenVertex();
                if (chance(60))
                    g.w(v + 0x14) = 0;
                g.w(r + 8 + 4 * k) = v;
            }
            if (chance(30))
                g.w(r + 0x18) = 0;
            else
            {
                const x86::reg32 t = g.alloc(0x28);
                g.randomBytes(t, 0x28);
                g.w(t + 4) = below(3);
                g.w(r + 0x18) = t;
            }
            g.w(r) = 0;
            if (previous)
                g.w(previous) = r;
            else
                first = r;
            previous = r;
        }
        g.w(0x554e58) = below(2);
        g.w(0x554e54) = below(3);
        g.w(0x554e5c) = below(2);
        g.w(0x7a3a64) = below(2);
        cpu.eax = first;
    }
    else if (name == "projectedTriangle")
    {
        const x86::reg32 slot = g.alloc(4);
        g.w(slot) = g.alloc(0xa0);
        x86::reg32 records[3];
        for (x86::reg32& r : records)
        {
            r = g.alloc(0x1c);
            g.randomBytes(r, 0x1c);
            g.w(r + 4) = chance(70) ? 0 : random32() & 0x3f;
            g.w(r + 8) = g.screenVertex();
            if (chance(70))
                g.w(g.w(r + 8) + 0x14) = 0;
        }
        cpu.eax = slot;
        cpu.ecx = records[0];
        a.stack[0] = records[1];
        a.stack[1] = records[2];
    }
    else if (name == "submitPolygons")
    {
        const x86::reg32 n = 1 + below(5);
        x86::reg32 first = 0, previous = 0;
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 p = g.alloc(0x80);
            g.randomBytes(p, 0x80);
            g.w(p) = 0;
            g.h(p + 4) = i == 0 || chance(90) ? 3 : 1;
            g.b(p + 7) = x86::reg8(random32() & 0xc0);
            for (x86::reg32 k = 0; k < 3; ++k)
            {
                const x86::reg32 v = g.screenVertex();
                if (chance(70))
                    g.w(v + 0x14) = 0;
                g.w(p + 8 + 4 * k) = v;
            }
            if (chance(40))
                g.w(p + 0x18) = 0;
            if (chance(40))
                g.w(p + 0x50) = 0;
            if (previous)
                g.w(previous) = p;
            else
                first = p;
            previous = p;
        }
        g.w(0x554e58) = below(2);
        g.w(0x554e54) = below(3);
        cpu.eax = first;
    }
    else if (name == "trackPolygons" || name == "trackRecords")
    {
        const bool records = name == "trackRecords";
        const x86::reg32 size = records ? 0x20 : 0x1c;
        const x86::reg32 recs = g.alloc(size * 8);
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            const x86::reg32 r = recs + size * i;
            if (records)
            {
                const x86::reg32 v = g.screenVertex();
                std::memcpy(&g.a.getMemory<x86::reg8>(r), &g.a.getMemory<x86::reg8>(v), 0x20);
                if (chance(50))
                    g.w(r + 0x14) = 0;
            }
            else
            {
                g.randomBytes(r, size);
                g.w(r + 4) = chance(70) ? 0 : random32() & 0x3f;
                g.w(r + 8) = g.screenVertex();
                if (chance(50))
                    g.w(g.w(r + 8) + 0x14) = 0;
            }
        }
        const x86::reg32 pieceRow = below(4), pieceCol = below(11);
        const x86::reg32 piece = 0x571370 + pieceRow * 0x5c0;
        const x86::reg32 n = below(8);
        const x86::reg32 polygons = g.alloc(14 * n + 16);
        g.randomBytes(polygons, 14 * n + 16);
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 p = polygons + 14 * i;
            for (x86::reg32 k = 0; k < 4; ++k)
                g.h(p + 2 * k) = x86::reg16(below(8));
            g.h(p + 8) = x86::reg16(below(4));
            g.h(p + 0xa) = x86::reg16(below(4));
            g.b(p + 0xc) = x86::reg8(random32() & 0x17);
            g.b(p + 0xd) = x86::reg8(((1 + below(7)) << 3) | (1 + below(7)));
        }
        g.w(piece + pieceCol * 4) = n;
        g.w(piece + pieceCol * 4 + 0x2c) = polygons;
        const x86::reg32 textures = g.alloc(47 * 16);
        g.randomBytes(textures, 47 * 16);
        g.w(0x552df4) = textures;
        const x86::reg32 far = g.alloc(0x40);
        g.f(far + 0x30) = positive(100.0f);
        g.w(0x5dd830) = far;
        g.w(0x6fbc3c) = chance(80) ? 1 : 0;
        g.w(0x7a3a60) = chance(80) ? 1 : 0;
        g.w(0x6fbc48) = below(2);
        g.w(0x552e1c) = random32();
        const x86::reg32 tail = g.alloc(4), head = g.alloc(4);
        g.w(tail) = chance(50) ? 0 : g.alloc(4);
        g.w(head) = 0;
        if (records)
        {
            cpu.ecx = recs;
            cpu.edx = pieceRow;
            cpu.ebx = pieceCol;
            a.stack[0] = tail;
            a.stack[1] = head;
        }
        else
        {
            const x86::reg32 view = g.alloc(0x80);
            g.w(view) = below(3);
            cpu.eax = view;
            cpu.edx = random32();
            cpu.ebx = pieceRow;
            cpu.ecx = pieceCol;
            a.stack[0] = recs;
            a.stack[1] = tail;
            a.stack[2] = head;
        }
    }
    else if (name == "objectRecords")
    {
        const x86::reg32 base = g.alloc(0x20 * 8);
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            const x86::reg32 v = g.screenVertex();
            std::memcpy(&g.a.getMemory<x86::reg8>(base + 0x20 * i), &g.a.getMemory<x86::reg8>(v), 0x20);
            if (chance(50))
                g.w(base + 0x20 * i + 0x14) = 0;
        }
        const x86::reg32 n = below(8);
        const x86::reg32 polygons = g.alloc(14 * n + 16);
        g.randomBytes(polygons, 14 * n + 16);
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 p = polygons + 14 * i;
            for (x86::reg32 k = 0; k < 4; ++k)
                g.h(p + 2 * k) = x86::reg16(below(8));
            g.h(p + 8) = x86::reg16(below(4));
            g.h(p + 0xa) = x86::reg16(below(4));
            g.b(p + 0xc) = x86::reg8(random32() & 0x17);
            g.b(p + 0xd) = x86::reg8(((1 + below(7)) << 3) | (1 + below(7)));
        }
        const x86::reg32 textures = g.alloc(47 * 16);
        g.randomBytes(textures, 47 * 16);
        g.w(0x552df4) = textures;
        const x86::reg32 far = g.alloc(0x40);
        g.f(far + 0x30) = positive(100.0f);
        g.w(0x5dd830) = far;
        g.w(0x6fbc48) = below(2);
        g.w(0x552e1c) = random32();
        cpu.ebx = polygons;
        cpu.edx = n;
        cpu.ecx = g.alloc(0x20 * 8);
        a.stack[0] = base;
    }
    else if (name == "gatherBuckets" || name == "gatherSorted")
    {
        // every bucket a head and a tail: a short list of nodes, or none
        auto fill = [&](x86::reg32 bucket) {
            if (chance(85))
            {
                g.w(bucket) = 0;
                g.w(bucket + 4) = random32();
                return;
            }
            const x86::reg32 node = g.alloc(8), last = chance(50) ? node : g.alloc(8);
            g.w(node) = last == node ? 0 : last;
            g.w(last) = 0;
            g.w(bucket) = node;
            g.w(bucket + 4) = last;
        };
        if (name == "gatherBuckets")
        {
            for (x86::reg32 i = 0; i < 0x100 + 0x7d0; ++i)
                fill(0x7d575c + 8 * i);
            g.w(0x7a3a60) = chance(85) ? 1 : 0;
            cpu.eax = below(2) ? random32() : 0;
        }
        else
            for (x86::reg32 i = 0; i < 0x320; ++i)
                fill(0x7d9ddc + 8 * i);
        cpu.esi = g.alloc(8);
    }
    else if (name == "sortedBuckets" || name == "depthBuckets")
    {
        g.f(0x54024c) = chance(80) ? 10.0f : aFloat(100.0f);
        g.f(0x54025c) = chance(80) ? 2.0f : aFloat(100.0f);
        g.f(0x7dcfdc) = aFloat(10.0f);
        g.w(0x7a3a60) = chance(85) ? 1 : 0;
        // some sorted buckets already hold a list, deepest first
        for (x86::reg32 i = 0; i < 0x320; ++i)
        {
            const x86::reg32 bucket = 0x7db6dc + 8 * i;
            g.w(bucket) = 0;
            if (chance(3))
            {
                x86::reg32 previous = 0;
                float depth = 1000.0f;
                for (x86::reg32 k = 0, n = 1 + below(3); k < n; ++k)
                {
                    const x86::reg32 node = g.alloc(0x20);
                    depth -= positive(10.0f);
                    g.f(node + 0x1c) = depth;
                    g.w(node) = 0;
                    if (previous)
                        g.w(previous) = node;
                    else
                        g.w(bucket) = node;
                    previous = node;
                }
                g.w(bucket + 4) = previous;
            }
        }
        for (x86::reg32 i = 0; i < 0x100 + 0x7d0; ++i)
        {
            const x86::reg32 bucket = 0x7d575c + 8 * i;
            g.w(bucket) = chance(5) ? g.alloc(4) : 0;
            g.w(bucket + 4) = g.w(bucket) ? g.w(bucket) : random32();
        }
        const x86::reg32 n = 1 + below(6);
        x86::reg32 first = 0, previous = 0;
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 p = g.alloc(0x20);
            g.randomBytes(p, 0x20);
            g.w(p) = 0;
            g.f(p + 0x1c) = chance(10) ? aFloat(100.0f) : float(below(1000)) / 8.0f;
            if (name == "depthBuckets")
            {
                g.h(p + 4) = x86::reg16(chance(80) ? below(2) : below(4));
                for (x86::reg32 k = 0; k < 4; ++k)
                {
                    const x86::reg32 v = g.screenVertex();
                    if (chance(50))
                        g.w(v + 0x10) |= 0xff000000;
                    g.w(p + 8 + 4 * k) = v;
                }
                if (chance(30))
                    g.w(p + 0x18) = 0;
                else
                {
                    const x86::reg32 t = g.alloc(0x30);
                    g.randomBytes(t, 0x30);
                    g.b(t + 0x28) = x86::reg8(random32() & (chance(70) ? 4 : 6));
                    g.w(p + 0x18) = t;
                }
            }
            if (previous)
                g.w(previous) = p;
            else
                first = p;
            previous = p;
        }
        cpu.eax = first;
        float start = aFloat(10.0f);
        std::memcpy(&a.stack[0], &start, 4);
        if (name == "sortedBuckets")
            a.stack[0] = g.alloc(8);
    }
    else if (name == "particles" || name == "moveParticles" || name == "drops")
    {
        for (x86::reg32 i = 0; i < 0x258; ++i)
        {
            const x86::reg32 p = 0x7ddfd0 + 0x80 * i;
            g.randomBytes(p, 0x80);
            g.w(p) = chance(30) ? 1 + below(40) : 0;
            g.w(p + 4) = below(8);
            g.w(p + 8) = 1 + below(60);
            g.floats(p + 0xc, 3, 100.0f);
            g.floats(p + 0x18, 3, 5.0f);
            g.floats(p + 0x24, 1, 2.0f);
            g.floats(p + 0x28, 2, 2.0f);
            g.f(p + 0x30) = unit() * 4.0f;
            g.f(p + 0x34) = chance(50) ? 0.0f : unit();
            g.f(p + 0x38) = unit();
            g.f(p + 0x3c) = unit();
            g.f(p + 0x40) = unit() * 4.0f;
            g.f(p + 0x44) = chance(50) ? 0.0f : unit();
            g.f(p + 0x48) = unit();
            g.f(p + 0x4c) = unit();
            g.floats(p + 0x58, 8, 300.0f);
            g.b(p + 0x7c) = x86::reg8(chance(30));
            g.b(p + 0x7d) = x86::reg8(random32() & 1);
            g.b(p + 0x7e) = x86::reg8(chance(10));
        }
        for (x86::reg32 k = 0; k < 8; ++k)
        {
            const x86::reg32 kind = 0x560b64 + 0x80 * k;
            g.randomBytes(kind, 0x80);
            g.f(kind + 0x2c) = unit();
            g.f(kind + 0x38) = std::fabs(unit());
            g.w(kind + 0x3c) = below(2);
            g.w(kind + 0x50) = chance(50) ? 0 : random32();
            g.w(kind + 0x68) = 2 + below(100);
            g.w(0x560bbc + 0x80 * k) = random32() & 0xffff;
            g.f(0x560bc4 + 0x80 * k) = positive(200.0f);
        }
        for (x86::reg32 l = 0; l < 4; ++l)
            g.f(0x4ca480 + 4 * l) = 0.5f + 0.25f * float(l);
        g.w(0x6fbc28) = below(4);
        g.w(0x6fbc30) = below(3);
        g.w(0x6fd3b0) = below(2);
        g.w(0x7d3684) = random32() & 0xff;
        for (x86::reg32 i = 0; i < 0xc8 * 2; ++i)
        {
            const x86::reg32 d = 0x7f0bd0 + 0x40 * i;
            g.randomBytes(d, 0x40);
            g.w(d) = chance(20) ? 1 + below(50) : 0;
            g.w(d + 4) = below(8);
            g.floats(d + 8, 8, 1.0f);
        }
        for (x86::reg32 pl = 0; pl < 2; ++pl)
        {
            const x86::reg32 camera = g.alloc(0x60);
            g.w(camera) = chance(80) ? below(4) : (chance(50) ? 8 : 0xe);
            g.w(0x5e10b8 + 4 * pl) = chance(50) ? 0 : camera;
            g.w(0x5e10b0 + 4 * pl) = camera;
        }
        g.w(0x7a3d0c) = chance(50) ? 0 : g.alloc(4);
        g.w(0x7a3d08) = g.alloc(4);
        const x86::reg32 view = g.alloc(0x80);
        g.randomBytes(view, 0x80);
        g.w(view) = below(2);
        g.w(view + 4) = below(2);
        g.floats(view + 0x38, 6, 50.0f);
        cpu.eax = view;
        cpu.edx = g.matrix();
        cpu.esi = g.alloc(8);
    }
    else if (name == "lightGlow")
    {
        const x86::reg32 view = g.alloc(0x80);
        g.floats(view + 0x38, 3, 20.0f);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(view + 0x44 + 4 * i) = unit();
        cpu.eax = view;
        for (int i = 0; i < 3; ++i)
        {
            const float v = aFloat(100.0f);
            std::memcpy(&a.stack[i], &v, 4);
        }
        a.stack[3] = random32();
        const float values[3] = {chance(10) ? -1.0f : positive(5.0f), chance(40) ? 0.0f : positive(100.0f),
                                 aFloat(5.0f)};
        std::memcpy(&a.stack[4], values, 12);
        g.w(0x7a3a60) = 1;
        g.w(0x7dcffc) = g.alloc(8);
        for (x86::reg32 i = 0; i < 0x320; ++i)
            g.w(0x7db6dc + 8 * i) = 0;
    }
    else if (name == "headlightBeam")
    {
        const x86::reg32 view = g.alloc(0x80);
        g.floats(view + 8, 3, 30.0f);
        g.floats(view + 0x38, 3, 20.0f);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(view + 0x44 + 4 * i) = unit();
        cpu.eax = view;
        float args[10];
        for (int i = 0; i < 3; ++i)
            args[i] = aFloat(30.0f);
        x86::reg32 matrix = g.matrix(), colour = random32();
        std::memcpy(&args[3], &matrix, 4);
        std::memcpy(&args[4], &colour, 4);
        args[5] = chance(10) ? 0.0f : positive(20.0f);
        args[6] = positive(5.0f);
        args[7] = positive(2.0f);
        args[8] = unit();
        args[9] = aFloat(5.0f);
        std::memcpy(a.stack, args, sizeof args);
        g.w(0x6fbc30) = below(3);
        g.b(0x567858) = 0;
        g.w(0x7a3a60) = 1;
        g.w(0x7dcffc) = g.alloc(8);
        for (x86::reg32 i = 0; i < 0x320; ++i)
            g.w(0x7db6dc + 8 * i) = 0;
    }
    else if (name == "carDetail")
    {
        const x86::reg32 view = g.alloc(0x80);
        g.w(view) = below(3);
        g.w(view + 4) = below(2);
        g.floats(view + 8, 3, 30.0f);
        g.floats(view + 0x38, 3, 20.0f);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(view + 0x44 + 4 * i) = unit();
        const x86::reg32 car = g.alloc(0x200);
        g.randomBytes(car, 0x200);
        g.b(car + 0x89) = x86::reg8(chance(90));
        g.b(car + 0x88) = x86::reg8(chance(10));
        g.w(car + 0x1c) = below(64) << 3;
        g.floats(car + 0x98, 3, 30.0f);
        for (x86::reg32 i = 0; i < 9; ++i)
            g.f(car + 0xc0 + 4 * i) = unit();
        for (x86::reg32 i = 0; i < 3; ++i)
            g.f(car + 0x114 + 4 * i) = positive(3.0f);
        for (x86::reg32 pl = 0; pl < 2; ++pl)
        {
            const x86::reg32 camera = g.alloc(0x60);
            g.w(camera) = below(4);
            g.w(camera + 4) = chance(30) ? car : random32();
            g.f(camera + 0x54) = chance(50) ? 0.0f : positive(2.0f);
            g.w(0x5e10b8 + 4 * pl) = chance(50) ? 0 : camera;
            g.w(0x5e10b0 + 4 * pl) = camera;
        }
        for (x86::reg32 i = 0; i < 64; ++i)
        {
            const x86::reg32 block = 0x571370 + i * 0x5c0;
            g.floats(block + 0x78, 3, 50.0f);
            g.floats(block + 0x84, 12, 50.0f);
        }
        const x86::reg32 far = g.alloc(0x40);
        g.f(far + 0x30) = positive(100.0f);
        g.f(far + 0x34) = positive(300.0f);
        g.w(0x5dd830) = far;
        g.w(0x6fbc1c) = below(4);
        g.w(0x6fd3b0) = below(2);
        g.w(0x6fd4c8) = below(2);
        for (x86::reg32 i = 0; i < 16; ++i)
            g.f(0x55fdf4 + 4 * i) = chance(10) ? 0.0f : positive(100.0f);
        cpu.eax = view;
        cpu.edx = car;
        cpu.ebx = g.alloc(4);
    }
    else
    {
        std::printf("no world for %s\n", name.c_str());
        std::exit(3);
    }
}

std::string floatBits(const x86::Float& f)
{
    double d = double(f);
    std::uint64_t u;
    std::memcpy(&u, &d, 8);
    char text[64];
    std::snprintf(text, sizeof text, "%016llx (%.17g)", (unsigned long long)u, d);
    return text;
}

const x86::reg16 kModes[] = {
    0x007f,  // single, nearest: the one mode the game's code is in now (fp::, include/fpu.h)
};

struct Counts
{
    unsigned cases = 0, failed = 0;
};

bool compare(VApp& n, VApp& g, const x86::CPU& nc, const x86::CPU& gc, char* what, size_t size)
{
    if (nc.terminate != gc.terminate)
        return std::snprintf(what, size, "terminate %d native, %d generated", nc.terminate, gc.terminate), false;
    if (!(n.calls == g.calls))
    {
        int at = std::snprintf(what, size, "calls: %zu native, %zu generated", n.calls.size(), g.calls.size());
        for (size_t i = 0; i < n.calls.size() && i < g.calls.size(); ++i)
            if (!(n.calls[i] == g.calls[i]))
            {
                at += std::snprintf(what + at, size - at, "; call %zu:", i);
                for (size_t k = 0; k < n.calls[i].words.size(); ++k)
                    at += std::snprintf(what + at, size - at, " %08x/%08x", n.calls[i].words[k], g.calls[i].words[k]);
                break;
            }
        return false;
    }
    if (nc.terminate)
    {
        // the game stopped in a callee: nothing after it counts, the stack
        // where each run was when it stopped neither
        const x86::reg8* m = &n.getMemory<x86::reg8>(0);
        const x86::reg8* o = &g.getMemory<x86::reg8>(0);
        if (std::memcmp(m, o, kStackLow) != 0)
            return std::snprintf(what, size, "memory differs below the stack, stopped"), false;
        ++g_stopped;
        return true;
    }
    const struct { const char* name; x86::reg32 a, b; } regs[] = {
        {"eax", nc.eax, gc.eax}, {"ebx", nc.ebx, gc.ebx}, {"ecx", nc.ecx, gc.ecx}, {"edx", nc.edx, gc.edx},
        {"esi", nc.esi, gc.esi}, {"edi", nc.edi, gc.edi}, {"ebp", nc.ebp, gc.ebp}, {"esp", nc.esp, gc.esp}};
    for (const auto& r : regs)
        if (r.a != r.b)
            return std::snprintf(what, size, "%s %08x native, %08x generated", r.name, r.a, r.b), false;
    if ((nc.flags.lo & 0xd5) != (gc.flags.lo & 0xd5) || nc.flags.of != gc.flags.of)
        return std::snprintf(what, size, "flags %02x/%u native, %02x/%u generated", nc.flags.lo & 0xd5,
                             unsigned(nc.flags.of), gc.flags.lo & 0xd5, unsigned(gc.flags.of)), false;
    if (nc.fpu.control.word != gc.fpu.control.word || nc.fpu.count != gc.fpu.count)
        return std::snprintf(what, size, "fpu control %04x/%04x depth %u/%u",
                             nc.fpu.control.word, gc.fpu.control.word,
                             unsigned(nc.fpu.count), unsigned(gc.fpu.count)), false;
    for (x86::reg32 i = 0; i < nc.fpu.count && i < 8; ++i)
    {
        x86::CPU& ncm = const_cast<x86::CPU&>(nc);
        x86::CPU& gcm = const_cast<x86::CPU&>(gc);
        const double x = double(ncm.fpu.st(int(i))), y = double(gcm.fpu.st(int(i)));
        if (std::memcmp(&x, &y, 8) == 0)
            continue;
        if (x != x && y != y)
        {
            if (std::getenv("VCHECK_DUMP"))
                std::printf("    st(%u) NaN %s native, %s generated\n", unsigned(i),
                            floatBits(ncm.fpu.st(int(i))).c_str(), floatBits(gcm.fpu.st(int(i))).c_str());
            ++g_nanOnly;
        }
        else
            return std::snprintf(what, size, "st(%u) %s native, %s generated", unsigned(i),
                                 floatBits(ncm.fpu.st(int(i))).c_str(), floatBits(gcm.fpu.st(int(i))).c_str()), false;
    }
    // memory: all of it but the stack below the returned esp
    const x86::reg8* m = &n.getMemory<x86::reg8>(0);
    const x86::reg8* o = &g.getMemory<x86::reg8>(0);
    auto range = [&](x86::reg32 from, x86::reg32 to) -> bool {
        if (std::memcmp(m + from, o + from, to - from) == 0)
            return true;
        for (x86::reg32 at = from; at < to; ++at)
            if (m[at] != o[at])
            {
                const x86::reg32 aligned = at & ~3u;
                x86::reg32 a, b;
                std::memcpy(&a, m + aligned, 4);
                std::memcpy(&b, o + aligned, 4);
                /* Two NaNs of another sign or payload: which operand's NaN a
                 * host operation passes on is the host compiler's operand
                 * order, not the source's -- counted, not failed. */
                if ((a & 0x7f800000) == 0x7f800000 && (a & 0x7fffff) && (b & 0x7f800000) == 0x7f800000
                    && (b & 0x7fffff))
                {
                    if (std::getenv("VCHECK_DUMP"))
                        std::printf("    %08x NaN %08x native, %08x generated\n", aligned, a, b);
                    ++g_nanOnly;
                    at = aligned + 3;
                    continue;
                }
                float fa, fb;
                std::memcpy(&fa, &a, 4);
                std::memcpy(&fb, &b, 4);
                std::snprintf(what, size, "memory at %08x: %08x (%.9g) native, %08x (%.9g) generated", at, a,
                              double(fa), b, double(fb));
                if (std::getenv("VCHECK_DUMP"))
                    for (x86::reg32 x = aligned, k = 0; x < to && k < 40; x += 4)
                    {
                        std::memcpy(&a, m + x, 4);
                        std::memcpy(&b, o + x, 4);
                        if (a != b && ++k)
                            std::printf("    %08x: %08x native, %08x generated\n", x, a, b);
                    }
                return false;
            }
        return true;
    };
    const x86::reg32 esp = nc.esp < kStackLow ? kStackLow : nc.esp > kStackHigh ? kStackHigh : nc.esp;
    return range(0, kStackLow) && range(esp, kMemory);
}
}  // namespace

extern "C" __declspec(dllimport) void* __stdcall SetUnhandledExceptionFilter(void* filter);
extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* name);

static long __stdcall crashed(void* pointers)
{
    struct Record
    {
        unsigned long code, flags;
        void* next;
        void* address;
        unsigned long count;
        unsigned long long info[15];
    };
    const Record* record = *static_cast<Record* const*>(pointers);
    std::printf("crash %08lx at rva %llx (reading %llx)\n", record->code,
                (unsigned long long)(static_cast<char*>(record->address) - static_cast<char*>(GetModuleHandleA(nullptr))),
                record->info[1]);
    std::fflush(stdout);
    std::_Exit(5);
}

int main(int argc, char** argv)
{
    SetUnhandledExceptionFilter(reinterpret_cast<void*>(&crashed));
    const unsigned cases = argc > 1 ? unsigned(std::atoi(argv[1])) : 200;
    std::vector<std::string> names(argv + (argc > 2 ? 2 : argc), argv + argc);
    static VApp nativeApp, generatedApp;
    // The game's data as it loads, into both.
    for (unsigned i = 0; i < kGameSegmentCount; ++i)
        std::memcpy(&nativeApp.getMemory<x86::reg8>(kGameSegments[i].address), kGameSegments[i].bytes,
                    kGameSegments[i].length);
    std::vector<x86::reg8> pristine(&nativeApp.getMemory<x86::reg8>(0), &nativeApp.getMemory<x86::reg8>(0) + kMemory);
    unsigned failedFunctions = 0;
    for (const Tested& t : kTested)
    {
        if (!names.empty() && std::find(names.begin(), names.end(), std::string(t.name)) == names.end())
            continue;
        Counts total;
        const unsigned nanBefore = g_nanOnly, stoppedBefore = g_stopped;
        std::string firstFailures;
        for (x86::reg16 mode : kModes)
        {
            Counts counts;
            for (unsigned c = 0; c < cases; ++c)
            {
                g_random.seed(unsigned(c * 7919 + mode * 31 + t.address));
                std::memcpy(&nativeApp.getMemory<x86::reg8>(0), pristine.data(), kMemory);
                World world{nativeApp};
                x86::CPU cpu{};
                cpu.fpu.init();
                cpu.fpu.setControl(mode);
                cpu.eax = random32();
                cpu.ebx = random32();
                cpu.ecx = random32();
                cpu.edx = random32();
                cpu.esi = random32();
                cpu.edi = random32();
                cpu.ebp = random32();
                cpu.flags.lo = x86::reg8(random32() & 0xd5) | 2;
                cpu.flags.of = random32() & 1;
                Args args;
                makeCase(world, t.name, cpu, args);
                if (const char* show = std::getenv("VCHECK_SHOW"))
                {
                    unsigned sm = 0, sc = 0;
                    if (std::sscanf(show, "%x:%u", &sm, &sc) == 2 && sm == mode && sc == c)
                        for (x86::reg32 r : {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx})
                        {
                            std::printf("  %08x:", r);
                            for (x86::reg32 k = 0; k < 0x20 && r < 0xf00000; k += 4)
                            {
                                const float v = world.f(r + k);
                                std::printf(" %08x(%g)", world.w(r + k), double(v));
                            }
                            std::printf("\n");
                        }
                }
                const x86::reg32 esp = kStackHigh - 0x2000;
                for (x86::reg32 at = kStackLow; at < kStackHigh; at += 4)
                    world.w(at) = random32();
                world.w(esp) = 0xdead0000;
                for (int i = 0; i < 10; ++i)
                    world.w(esp + 4 + 4 * i) = args.stack[i];
                cpu.esp = esp;
                std::memcpy(&generatedApp.getMemory<x86::reg8>(0), &nativeApp.getMemory<x86::reg8>(0), kMemory);
                nativeApp.calls.clear();
                generatedApp.calls.clear();
                x86::CPU nc = cpu, gc = cpu;
                nativeApp.generated = false;
                generatedApp.generated = true;
                const bool trace = std::getenv("VCHECK_TRACE") != nullptr;
                if (trace)
                    std::printf("%s %04x %u native\n", t.name, unsigned(mode), c), std::fflush(stdout);
                try
                {
                    t.native(&nativeApp, nc);
                }
                catch (const std::exception& e)
                {
                    std::printf("%s %04x %u: the native threw %s\n", t.name, unsigned(mode), c, e.what());
                    std::exit(6);
                }
                if (trace)
                    std::printf("%s %04x %u generated\n", t.name, unsigned(mode), c), std::fflush(stdout);
                t.generated(&generatedApp, gc);
                ++counts.cases;
                char what[600] = "";
                if (compare(nativeApp, generatedApp, nc, gc, what, sizeof what))
                    continue;
                if (++counts.failed <= 3)
                {
                    char line[800];
                    std::snprintf(line, sizeof line, "  %s mode %04x case %u: %s\n", t.name, unsigned(mode), c, what);
                    firstFailures += line;
                }
            }
            total.cases += counts.cases;
            total.failed += counts.failed;
        }
        std::printf("%-24s %s (%u cases, %u stopped in a callee, %u NaN-only words)\n", t.name,
                    total.failed ? "DIFFERS" : "alike", total.cases, g_stopped - stoppedBefore,
                    g_nanOnly - nanBefore);
        std::printf("%s", firstFailures.c_str());
        std::fflush(stdout);
        failedFunctions += total.failed != 0;
    }
    return failedFunctions ? 1 : 0;
}
