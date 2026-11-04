/* native_render.cpp's natives against the game's code of their functions,
 * made afresh with the port's changes in it (gen_render.py), on the desktop.
 *
 * Two guest memories start alike: nfs3.exe's image with its data and bss made
 * up afresh for every case (pointers into them, floats, small integers, now and
 * then any word), a made-up stack, registers, flags and x87 stack, and the
 * control word of the mode under test.  Every call out goes to a stand-in that
 * logs the call (target, registers, the x87's depth, the words the callee takes
 * off the stack, and for THRASH what its pointers point at), gives a result
 * made from the case and the call's place, pops as the callee pops (ret N, a
 * THRASH stdcall's size), chops st(0) for __CHP and leaves a made-up float for
 * the functions that return one -- and now and then stops the game.  Then the
 * calls, the registers, the flags, the x87 and the memory are compared.
 *
 *     rcheck <function|all> <cases> <first case> <control words, e.g. 7f,37f> [df]
 *
 * df sets the direction flag in a fifth of the cases: the natives copy
 * forwards, as the game always has it, so those differ where a copy runs.
 */
#include "nfs3hp.h"
#include <lib/memmap.h>
#include <lib/thread.h>
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

unsigned char* guestReserve();
void guestCommit(unsigned char* base, unsigned from, unsigned to);
std::vector<unsigned> guestTransient(unsigned char* a, unsigned char* b);
void guestDrop(unsigned char* base);
extern void (*g_runaway)(const char*);
#define WIN32_LEAN_AND_MEAN
#include <atomic>
#include <thread>
#include <chrono>

struct RenderTest
{
    unsigned address;
    const char* name;
    void (*generated)(win32::WinApplication*, x86::CPU&);
    void (*native)(win32::WinApplication*, x86::CPU&);
};
extern const RenderTest kRenderTests[];
extern const unsigned kRenderTestCount;

namespace
{
#include "tables.inc"
}

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
}

/* What native_render.cpp and the port's changes in the game's code want of the
 * rest of the port (nfs3hp_main.cpp's): the same on both sides, each leaving a
 * trace of its own where it writes. */
namespace nfs3hp
{
x86::reg32 mirrorAsMain(x86::reg32 kind) { return kind == 1 ? 0 : kind; }
x86::reg32 viewDistanceReduced(win32::WinApplication*, x86::reg32 reduced) { return reduced; }
x86::reg32 splitFarDistance(win32::WinApplication*, x86::reg32 distance) { return distance; }
x86::reg32 fullFarDistance(win32::WinApplication*, x86::reg32 distance) { return distance; }
void mirrorFollowsMain(win32::WinApplication* app) { app->getMemory<x86::reg32>(0x7a3d10) += 1; }
x86::reg32 hudReferenceWidth(win32::WinApplication* app) { return app->getMemory<x86::reg32>(0x7cdac8) % 2048; }
x86::reg32 hudReferenceHeight(win32::WinApplication* app) { return app->getMemory<x86::reg32>(0x7cdacc) % 2048; }
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot) { app->getMemory<x86::reg32>(slot) += 3; }
void hudTablePanel(win32::WinApplication* app, x86::reg32 player, bool otherLayout)
{
    app->getMemory<x86::reg32>(0x7a3d18) += player + otherLayout;
}
x86::reg32 cabinRect(win32::WinApplication* app, x86::reg32 field) { return app->getMemory<x86::reg32>(0x7cdac8) / (field + 1); }
bool g_fullCarDetail = false;
bool fullCarDetail() { return g_fullCarDetail; }
void loadingScreenFit(win32::WinApplication* app, x86::CPU&, bool on) { app->getMemory<x86::reg32>(0x7a3d1c) += on; }
bool dynamicFullscreenMode(x86::reg32 mode) { return mode == 16; }
bool dynamicFullscreenRefresh(win32::WinApplication* app) { return app->getMemory<x86::reg32>(0x7a3d20) & 1; }
}

namespace
{
constexpr x86::reg32 kImage = 0x400000, kImageEnd = 0xa40000;
constexpr x86::reg32 kData = 0x536000, kDataEnd = 0xa19000;
constexpr x86::reg32 kStackLow = 0xef0000, kStackHigh = 0xf00000;
constexpr x86::reg32 kThrashBase = 0xf0000000;

std::vector<unsigned char> g_image;
std::uint64_t g_seed = 0;
std::atomic<unsigned> g_caseTick{0};
const char* g_curName = "";
unsigned g_curControl = 0, g_curCase = 0;
void runaway(const char* why)
{
    std::printf("RUNAWAY %s %x %u %s\n", g_curName, g_curControl, g_curCase, why);
    std::fflush(stdout);
    std::_Exit(9);
}
unsigned g_nanWords = 0;
long g_stopAt = -1;

std::uint64_t mix(std::uint64_t x)
{
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    return x ^ (x >> 31);
}

struct Rng
{
    std::uint64_t s;
    std::uint64_t next() { return s = mix(s); }
    unsigned below(unsigned n) { return unsigned(next() % n); }
    bool chance(unsigned percent) { return below(100) < percent; }
};

x86::reg32 floatBits(Rng& r)
{
    const unsigned k = r.below(100);
    if (k < 4)
        return 0;
    if (k < 8)
        return 0x3f800000;
    if (k < 10)
        return 0xbf800000;
    if (k < 12)
        return 0x3f000000;
    const x86::reg32 sign = r.chance(30) ? 0x80000000 : 0;
    const x86::reg32 exponent = 0x70 + r.below(0x1d);  // 2^-15 .. 2^13
    return sign | exponent << 23 | x86::reg32(r.next() & 0x7fffff);
}

/* A word of the made-up world. */
bool g_sparse = false;  // a world of mostly small integers: shorter loops, fewer chains
x86::reg32 anyWord(Rng& r)
{
    const unsigned k = r.below(100);
    if (g_sparse)
    {
        if (k < 10)
            return (kData + r.below(kDataEnd - kData)) & ~3u;
        if (k < 40)
            return floatBits(r);
        if (k < 95)
            return r.chance(60) ? 0 : r.below(8);
        return r.below(0x10000);
    }
    if (k < 33)
        return (kData + r.below(kDataEnd - kData)) & ~3u;
    if (k < 63)
        return floatBits(r);
    if (k < 90)
        return r.chance(85) ? r.below(64) : x86::reg32(0) - 1 - r.below(8);
    if (k < 97)
        return r.below(0x10000);
    return x86::reg32(r.next());
}

struct Call
{
    x86::reg32 target;
    std::vector<x86::reg32> words;
    bool operator==(const Call& o) const
    {
        if (target != o.target || words.size() != o.words.size())
            return false;
        for (size_t i = 0; i < words.size(); ++i)
            if (words[i] != o.words[i] && !(nan(words[i]) && nan(o.words[i]) && !std::getenv("RENDER_NAN_EXACT")))
                return false;
        return true;
    }
    static bool nan(x86::reg32 w) { return (w & 0x7f800000) == 0x7f800000 && (w & 0x7fffff); }
};

unsigned thrashBytes(x86::reg32 slot, const char** name)
{
    for (const auto& t : kThrashSlots)
        if (t.slot == slot)
        {
            *name = t.name;
            return t.bytes;
        }
    *name = "?";
    return 0;
}

unsigned popsOf(x86::reg32 address)
{
    for (const auto& p : kPops)
        if (p.address == address)
            return p.pops;
    return 0;
}

bool floatBack(x86::reg32 address)
{
    switch (address)
    {
    case 0x49d5c0: case 0x4e01f0: case 0x49de20: case 0x4d1000: case 0x4d1480: case 0x4f1c30: case 0x4f2d78:
    case 0x4eb230:
        return true;
    }
    return false;
}

}  // namespace

namespace win32
{

class RApp : public WinApplication
{
public:
    RApp() : WinApplication("render_harness", 0, {}) {}
    std::vector<Call> calls;
    x86::reg32& word(x86::reg32 address) { return reinterpret_cast<x86::reg32&>(*&getMemory<x86::reg8>(address)); }
    unsigned char* base() { return &getMemory<unsigned char>(0); }

    void standIn(x86::reg32 address, x86::CPU& cpu)
    {
        Call c{address, {}};
        const x86::reg32 esp = cpu.esp;
        unsigned bytes = 0;
        const char* name = nullptr;
        const bool thrash = (address & 0xff000000) == kThrashBase;
        if (thrash)
            bytes = thrashBytes(address - kThrashBase, &name);
        else
            bytes = popsOf(address);
        for (x86::reg32 v : {cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp, cpu.esp})
            c.words.push_back(v);
        for (unsigned i = 0; i < bytes / 4; ++i)
        {
            const x86::reg32 w = word(esp + 4 + 4 * i);
            c.words.push_back(w);
            if (thrash && w >= kImage && w < kImageEnd - 0x40)
                for (unsigned k = 0; k < 0x20; k += 4)
                    c.words.push_back(word(w + k));
        }
        calls.push_back(c);
        if (long(calls.size()) - 1 == g_stopAt)
        {
            cpu.terminate = true;
            return;
        }
        const std::uint64_t r = mix(g_seed ^ (std::uint64_t(calls.size()) << 32) ^ address);
        if (address == 0x4dfd56)
            cpu.fpu.st(0) = x86::Float(std::trunc(double(cpu.fpu.st(0))));  // __CHP
        else
        {
            const unsigned k = r % 8;
            const x86::reg32 where = cpu.eax;
            cpu.eax = k < 2 ? 0 : k < 3 ? 1 : k < 6 ? (kData + (r >> 8) % (kDataEnd - kData)) & ~3u : x86::reg32(r >> 16);
            // the polygon records' allocator: a record of [edx] bytes to [eax], or none
            if (address == 0x4bbf80)
            {
                cpu.eax = k < 2 ? 0 : 1;
                if (cpu.eax)
                    word(where) = (kData + (r >> 8) % (kDataEnd - kData - 0x400)) & ~3u;
            }
            if (floatBack(address))
            {
                cpu.fpu.count += 1;
                cpu.fpu.st(0) = x86::Float(double(x86::sreg32(r >> 20)) / 7.0);
            }
        }
        // a callee leaves flags of its own
        cpu.flags.cf = (r >> 40) & 1;
        cpu.flags.zf = (r >> 41) & 1;
        cpu.flags.sf = (r >> 42) & 1;
        cpu.flags.of = (r >> 43) & 1;
        cpu.flags.pf = (r >> 44) & 1;
        cpu.flags.af = (r >> 45) & 1;
        cpu.esp = esp + 4 + bytes;
    }
};

RApp* g_apps[2];

WinApplication::WinApplication(const char*, x86::reg32, const std::vector<Section>&)
{
    m_memory = guestReserve();
}
WinApplication::~WinApplication() {}
int WinApplication::s_traceApi = 0;
bool WinApplication::readTraceApi() { return false; }
void WinApplication::traceCall(const Method&, x86::reg32) {}
void WinApplication::yieldContext(const x86::CPU&) {}
void WinApplication::lockContext(const x86::CPU&) {}
void WinApplication::unlockContext(const x86::CPU&) {}
void Thread::sleep(x86::reg32) {}
void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    static_cast<RApp*>(this)->standIn(address, cpu);
}
void MemMap::usedRanges(std::vector<std::pair<x86::reg32, x86::reg32>>& ranges)
{
    ranges.assign(1, {0, 0x1000000});
}

}  // namespace win32

namespace
{
using win32::RApp;

struct Counts
{
    unsigned cases = 0, stopped = 0, failed = 0;
};

std::string hexWords(const std::vector<x86::reg32>& w)
{
    std::string s;
    char b[16];
    for (size_t i = 0; i < w.size() && i < 24; ++i)
    {
        std::snprintf(b, sizeof b, " %08x", w[i]);
        s += b;
    }
    return s;
}

std::string fbits(x86::Float f)
{
    double d = double(f);
    std::uint64_t u;
    std::memcpy(&u, &d, 8);
    char b[64];
    std::snprintf(b, sizeof b, "%016llx(%g)", (unsigned long long)u, d);
    return b;
}

bool compare(RApp& g, RApp& n, const x86::CPU& gc, const x86::CPU& nc, x86::reg32 entry, std::string& what)
{
    char b[600];
    if (gc.terminate != nc.terminate)
    {
        what = "stopped in one run only";
        return false;
    }
    if (!(g.calls == n.calls) && !std::getenv("RENDER_MEMFIRST"))
    {
        size_t i = 0;
        while (i < g.calls.size() && i < n.calls.size() && g.calls[i] == n.calls[i])
            ++i;
        std::snprintf(b, sizeof b, "calls: %zu generated, %zu native; first difference at %zu:\n    gen %08x%s\n    nat %08x%s",
                      g.calls.size(), n.calls.size(), i, i < g.calls.size() ? g.calls[i].target : 0,
                      i < g.calls.size() ? hexWords(g.calls[i].words).c_str() : "",
                      i < n.calls.size() ? n.calls[i].target : 0, i < n.calls.size() ? hexWords(n.calls[i].words).c_str() : "");
        what = b;
        return false;
    }
    if (!gc.terminate)
    {
        const struct { const char* name; x86::reg32 a, b; } regs[] = {
            {"eax", gc.eax, nc.eax}, {"ebx", gc.ebx, nc.ebx}, {"ecx", gc.ecx, nc.ecx}, {"edx", gc.edx, nc.edx},
            {"esi", gc.esi, nc.esi}, {"edi", gc.edi, nc.edi}, {"ebp", gc.ebp, nc.ebp}, {"esp", gc.esp, nc.esp},
            {"cf", gc.flags.cf, nc.flags.cf}, {"zf", gc.flags.zf, nc.flags.zf}, {"sf", gc.flags.sf, nc.flags.sf},
            {"of", gc.flags.of, nc.flags.of}, {"pf", gc.flags.pf, nc.flags.pf}, {"af", gc.flags.af, nc.flags.af},
            {"df", gc.flags.df, nc.flags.df},
            {"fpu count", gc.fpu.count, nc.fpu.count},
            {"fpu control", gc.fpu.control.word, nc.fpu.control.word},
        };
        for (const auto& r : regs)
            if (r.a != r.b)
            {
                std::snprintf(b, sizeof b, "%s %08x generated, %08x native", r.name, r.a, r.b);
                what = b;
                return false;
            }
        // the live part of the x87's stack (popped registers are not seen)
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            const x86::reg32 depth = gc.fpu.count;
            if (!std::getenv("RENDER_ALL_FPU_REGS") && (depth > 8 || ((depth - i) & 7) >= depth))
                continue;
            const double a = double(gc.fpu.regs[i]), c = double(nc.fpu.regs[i]);
            if (std::memcmp(&a, &c, 8) != 0 && !(a != a && c != c))
            {
                std::snprintf(b, sizeof b, "fpu register %u: %s generated, %s native", unsigned(i),
                              fbits(gc.fpu.regs[i]).c_str(), fbits(nc.fpu.regs[i]).c_str());
                what = b;
                return false;
            }
        }
    }
    auto region = [&](x86::reg32 from, x86::reg32 to) {
        if (std::memcmp(g.base() + from, n.base() + from, to - from) == 0)
            return true;
        for (x86::reg32 at = from; at < to; ++at)
            if (g.base()[at] != n.base()[at])
            {
                const x86::reg32 w = at & ~3u;
                float fg, fn;
                std::memcpy(&fg, g.base() + w, 4);
                std::memcpy(&fn, n.base() + w, 4);
                if (fg != fg && fn != fn && !std::getenv("RENDER_NAN_EXACT"))
                {
                    ++g_nanWords;  // two NaNs: which operand's payload the host's compiler kept
                    at = w + 3;
                    continue;
                }
                std::snprintf(b, sizeof b, "memory at %08x: word %08x generated (%g), %08x native (%g)", at,
                              g.word(w), fg, n.word(w), fn);
                what = b;
                return false;
            }
        return true;
    };
    if (!region(kImage, kImageEnd) || !region(entry, kStackHigh))
        return false;
    for (unsigned page : guestTransient(g.base(), n.base()))
        if (!region(page << 12, (page << 12) + 0x1000))
            return false;
    return true;
}

bool runCase(const RenderTest& t, RApp& g, RApp& n, unsigned index, x86::reg16 control, Counts& counts, bool df)
{
    Rng r{mix(0x1998 + index * 0x10001ull + t.address * 0x9e37ull)};
    g_seed = r.next();
    g_sparse = r.chance(50);
    // the world
    std::memcpy(g.base() + kImage, g_image.data(), g_image.size());
    for (x86::reg32 at = kData; at < kDataEnd; at += 4)
        g.word(at) = anyWord(r);
    for (x86::reg32 at = kStackLow; at < kStackHigh; at += 4)
        g.word(at) = anyWord(r);
    for (const auto& s : kThrashSlots)
        g.word(s.slot) = kThrashBase + s.slot;
    // the lightning's 0xf0 bolts: a few alive, few points each, the points in the data
    if (t.address == 0x4946e0)
        for (x86::reg32 bolt = 0x79c570; bolt < 0x79c570 + 0xf0 * 0x30; bolt += 0x30)
        {
            g.word(bolt) = r.chance(3) ? 1 + r.below(200) : 0;
            g.word(bolt + 0x1c) = r.below(4);
            for (x86::reg32 k = 0; k < 4; ++k)
                g.word(bolt + 0x20 + 4 * k) = (kData + r.below(kDataEnd - kData)) & ~3u;
        }
    // the lightning's and the sparks' divisors: mostly made up, -1 now and
    // then, and 0 in every fourth case -- which faults on the x86, and which
    // the natives take as the game's code takes it on arm64 (gen_render.py)
    if (g.word(0x55e4ac) == 0)
        g.word(0x55e4ac) = 3;
    if (g.word(0x8ca27c) == 0)
        g.word(0x8ca27c) = r.chance(50) ? 5 : 0xffffffff;
    if (index % 4 == 3 && (t.address == 0x4946e0 || t.address == 0x4ddb40))
    {
        g.word(0x55e4ac) = 0;
        g.word(0x8ca27c) = 0;
    }
    // NFS_CAR_DETAIL_FULL, off in the odd cases
    nfs3hp::g_fullCarDetail = index % 2 == 0;
    std::memcpy(n.base() + kImage, g.base() + kImage, kImageEnd - kImage);
    std::memcpy(n.base() + kStackLow, g.base() + kStackLow, kStackHigh - kStackLow);

    x86::CPU cpu{};
    cpu.eax = anyWord(r);
    cpu.ebx = anyWord(r);
    cpu.ecx = anyWord(r);
    cpu.edx = anyWord(r);
    cpu.esi = anyWord(r);
    cpu.edi = anyWord(r);
    cpu.ebp = anyWord(r);
    const x86::reg32 entry = kStackHigh - 0x2000 - 4 * r.below(64);
    cpu.esp = entry;
    g.word(entry) = 0xdead0000;
    n.word(entry) = 0xdead0000;
    cpu.flags.eflags = 0x00200202;
    cpu.flags.cf = r.chance(50);
    cpu.flags.zf = r.chance(50);
    cpu.flags.sf = r.chance(50);
    cpu.flags.of = r.chance(50);
    cpu.flags.pf = r.chance(50);
    cpu.flags.af = r.chance(50);
    cpu.flags.df = df && r.chance(20);
    cpu.fpu.init();
    cpu.fpu.setControl(control);
    cpu.fpu.count = r.below(4);
    for (x86::Float& f : cpu.fpu.regs)
    {
        const x86::reg32 bits = floatBits(r);
        float v;
        std::memcpy(&v, &bits, 4);
        f = x86::Float(v);
    }
    g_stopAt = r.chance(10) ? long(r.below(12)) : -1;
    // a screen mode: Dynamic Fullscreen's, the one open, in every third case
    if (t.address == 0x4bef50 && index % 3 == 0)
    {
        cpu.eax = 16;
        g.word(0x55fec0) = 16;
        n.word(0x55fec0) = 16;
    }

    // RENDER_WATCH=6fbc28,5dd0dc: those words as the case starts and as each run leaves them
    std::vector<x86::reg32> watched;
    if (const char* watch = std::getenv("RENDER_WATCH"))
        for (const char* at = watch; *at;)
        {
            char* end = nullptr;
            watched.push_back(x86::reg32(std::strtoul(at, &end, 16)));
            at = *end ? end + 1 : end;
        }
    for (x86::reg32 w : watched)
        std::printf("    watch %08x: %08x\n", w, g.word(w));

    g.calls.clear();
    n.calls.clear();
    x86::CPU gc = cpu, nc = cpu;
    t.native(&n, nc);
    t.generated(&g, gc);
    for (x86::reg32 w : watched)
        std::printf("    watch %08x: %08x generated, %08x native\n", w, g.word(w), n.word(w));

    ++counts.cases;
    if (gc.terminate)
        ++counts.stopped;
    {
        // what the generated code did, for telling the cases the mode changes
        std::uint64_t h = 0x1998;
        auto add = [&](std::uint64_t v) { h = mix(h ^ v); };
        const std::uint64_t* words = reinterpret_cast<const std::uint64_t*>(g.base() + kImage);
        for (size_t i = 0; i < (kImageEnd - kImage) / 8; ++i)
            h ^= words[i] * (0x9e3779b97f4a7c15ull + 2 * i);
        for (x86::reg32 at = entry & ~7u; at < kStackHigh; at += 8)
            add(*reinterpret_cast<const std::uint64_t*>(g.base() + at));
        for (const Call& c : g.calls)
            for (x86::reg32 w : c.words)
                add(w);
        add(gc.eax); add(gc.ebx); add(gc.ecx); add(gc.edx); add(gc.esi); add(gc.edi);
        std::printf("HASH %u %016llx\n", index, (unsigned long long)h);
    }
    std::string what;
    const bool alike = compare(g, n, gc, nc, entry, what);
    guestDrop(g.base());
    guestDrop(n.base());
    if (!alike)
        std::printf("FAILCASE %u\n", index);
    if (!alike && ++counts.failed <= 3)
    {
        std::printf("  %s cw %04x case %u%s: %s\n", t.name, control, index, cpu.flags.df ? " (df)" : "", what.c_str());
        if (std::getenv("RENDER_VERBOSE"))
        {
            std::printf("    entry: eax %08x ebx %08x ecx %08x edx %08x esi %08x edi %08x ebp %08x esp %08x fpu %u\n",
                        cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp, cpu.esp, cpu.fpu.count);
            for (const x86::CPU* c : {&gc, &nc})
                std::printf("    %s exit: eax %08x ebx %08x ecx %08x edx %08x esi %08x edi %08x ebp %08x esp %08x fpu %u\n",
                            c == &gc ? "gen" : "nat", c->eax, c->ebx, c->ecx, c->edx, c->esi, c->edi, c->ebp, c->esp,
                            c->fpu.count);
            for (const RApp* app : {&g, &n})
                for (const Call& c : app->calls)
                    std::printf("    %s %08x%s\n", app == &g ? "gen" : "nat", c.target, hexWords(c.words).c_str());
        }
    }
    std::fflush(stdout);
    return alike;
}

}  // namespace

int main(int argc, char** argv)
{
    const char* only = argc > 1 ? argv[1] : "all";
    const unsigned cases = argc > 2 ? unsigned(std::strtoul(argv[2], nullptr, 10)) : 500;
    const unsigned first = argc > 3 ? unsigned(std::strtoul(argv[3], nullptr, 10)) : 0;
    std::vector<x86::reg16> controls;
    std::string list = argc > 4 ? argv[4] : "7f,37f,27f,c7f,f7f,47f";
    for (size_t at = 0; at < list.size();)
    {
        size_t comma = list.find(',', at);
        if (comma == std::string::npos)
            comma = list.size();
        controls.push_back(x86::reg16(std::strtoul(list.substr(at, comma - at).c_str(), nullptr, 16)));
        at = comma + 1;
    }
    const bool df = argc > 5 && std::strcmp(argv[5], "df") == 0;
    const char* imagePath = std::getenv("RENDER_IMAGE");
    FILE* f = std::fopen(imagePath ? imagePath : "image.bin", "rb");
    if (!f)
    {
        std::printf("no image.bin\n");
        return 2;
    }
    g_image.resize(kImageEnd - kImage);
    std::fread(g_image.data(), 1, g_image.size(), f);
    std::fclose(f);

    g_runaway = runaway;
    std::thread([] {
        unsigned last = 0, same = 0;
        for (;;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            const unsigned now = g_caseTick.load();
            same = now == last ? same + 1 : 0;
            last = now;
            if (same > 40)
                runaway("ran over 4 s");
        }
    }).detach();
    RApp g, n;
    for (RApp* app : {&g, &n})
    {
        guestCommit(app->base(), kImage, kImageEnd);
        guestCommit(app->base(), kStackLow, kStackHigh);
    }
    bool failed = false;
    for (unsigned i = 0; i < kRenderTestCount; ++i)
    {
        const RenderTest& t = kRenderTests[i];
        if (std::strcmp(only, "all") != 0 && std::strcmp(only, t.name) != 0)
            continue;
        for (x86::reg16 control : controls)
        {
            Counts counts;
            for (unsigned c = first; c < first + cases; ++c)
            {
                g_curName = t.name;
                g_curControl = control;
                g_curCase = c;
                ++g_caseTick;
                std::fprintf(stderr, "%s %x %u\r", t.name, control, c);
                runCase(t, g, n, c, control, counts, df);
            }
            std::printf("%-20s cw %04x: %5u cases, %4u stopped, %4u failed\n", t.name, control, counts.cases,
                        counts.stopped, counts.failed);
            std::fflush(stdout);
            failed = failed || counts.failed;
        }
    }
    std::printf("NaN words told apart only by payload: %u\n", g_nanWords);
    std::printf(failed ? "FAILED\n" : "all alike\n");
    return failed ? 1 : 0;
}
