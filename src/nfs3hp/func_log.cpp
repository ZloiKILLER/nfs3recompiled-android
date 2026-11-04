/* The live functions log (debug mode, Advanced screen; NFS_FUNC_LOG=1).
 *
 * Nothing is added to the game's code: a timer (ITIMER_PROF, every 4 ms of the
 * process's CPU time) interrupts whichever thread is running, and the handler
 * walks its frame-pointer chain -- clang keeps frame pointers on arm64 -- and
 * counts the return addresses that lie in this library.  Two counts are kept:
 * the function the time is spent in (its first frame inside the library) and
 * every function on the stack, so the callers of a hot function show up too.
 * With the switch off none of this exists, and the game is as it was.
 *
 * A thread appends what changed to logs/functions.bin in the game's folder every
 * two seconds, and says in logcat ([FUNCS]) how it goes.  The file holds
 * addresses, not names: tools/profile_report.py reads them against the
 * unstripped libmain.so of the same build.
 *
 * It also notes the x87's precision: every control word the game sets
 * (FPU::s_controlHook) is kept for its thread, and each sample says the
 * precision control its thread was under -- 24, 53 or 64 bits -- so the
 * report can say how the game's time divides between them.
 *
 * File: "NFF2", u64 load base, u8 length and the bytes of the library's GNU
 * build id, then records of two u32:
 *   {0xFFFFFFFF, ms since start}                            starts a snapshot
 *   {address - base, samples since the last snapshot}       on the stack
 *   {(address - base) | 0x80000000 | P, samples ...}        the function running
 *   {0xFFFFFFE0 | P >> 28, samples ...}                     running outside the library
 *   {0xFFFFFFFD, control word}, {0xFFFFFFFC, times set}     fldcw since the last snapshot
 * P is 0 when the thread has set no control word, else 0x40000000 with the
 * word's precision control (0 single, 2 double, 3 extended) in bits 28-29.
 */
#include "func_log.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cxxabi.h>
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ucontext.h>
#include <unistd.h>
#include <SDL3/SDL_log.h>
#include <fpu.h>

namespace nfs3hp
{
namespace
{
constexpr uint32_t kSlotBits = 19;
constexpr uint32_t kSlots = 1u << kSlotBits;
constexpr uint32_t kSelf = 0x80000000u;
constexpr uint32_t kKnown = 0x40000000u;
constexpr uint32_t kOutside = 0xFFFFFFE0u;
constexpr uint32_t kWordMark = 0xFFFFFFFDu;
constexpr uint32_t kWordCount = 0xFFFFFFFCu;
constexpr int kWords = 16;
constexpr int kMaxFrames = 64;
constexpr int kIntervalUs = 4000;

std::atomic<uint32_t> gKeys[kSlots];     // address - base, kSelf for "running in"
std::atomic<uint32_t> gCounts[kSlots];
uint32_t gSaved[kSlots];
uintptr_t gBase, gEnd;
int gPipe[2] = {-1, -1};
std::atomic_flag gBusy = ATOMIC_FLAG_INIT;
std::atomic<uint64_t> gSamples{0}, gDropped{0};

/* Each thread's last control word, (tid << 16) | word, by tid & 63; and how
 * often each word was set. */
std::atomic<uint64_t> gThreadWord[64];
std::atomic<uint32_t> gWords[kWords];       // word + 1, 0 for none
std::atomic<uint32_t> gWordCounts[kWords];
uint32_t gWordSaved[kWords];

void onControl(x86::reg16 word)
{
    const uint32_t tid = uint32_t(gettid());
    gThreadWord[tid & 63].store((uint64_t(tid) << 16) | word, std::memory_order_relaxed);
    for (int i = 0; i < kWords; i++)
    {
        uint32_t w = gWords[i].load(std::memory_order_relaxed);
        if (w == 0)
        {
            uint32_t zero = 0;
            w = gWords[i].compare_exchange_strong(zero, uint32_t(word) + 1) ? uint32_t(word) + 1 : zero;
        }
        if (w == uint32_t(word) + 1)
        {
            gWordCounts[i].fetch_add(1, std::memory_order_relaxed);
            return;
        }
    }
}

/* The interrupted thread's precision, as a sample's key carries it. */
uint32_t precisionTag()
{
    const uint32_t tid = uint32_t(gettid());
    const uint64_t v = gThreadWord[tid & 63].load(std::memory_order_relaxed);
    if (uint32_t(v >> 16) != tid)
        return 0;
    return kKnown | (((uint32_t(v) >> 8) & 3u) << 28);
}

void count(uint32_t key)
{
    uint32_t i = (key * 0x9E3779B1u) >> (32 - kSlotBits);
    for (int probe = 0; probe < 64; probe++, i = (i + 1) & (kSlots - 1))
    {
        uint32_t k = gKeys[i].load(std::memory_order_relaxed);
        if (k == 0)
        {
            uint32_t zero = 0;
            if (!gKeys[i].compare_exchange_strong(zero, key))
                k = zero;
            else
                k = key;
        }
        if (k == key)
        {
            gCounts[i].fetch_add(1, std::memory_order_relaxed);
            return;
        }
    }
    gDropped.fetch_add(1, std::memory_order_relaxed);
}

/* Two words at addr, or false if that memory is not there: handed to a pipe,
 * the kernel says EFAULT instead of the thread faulting. */
bool readFrame(uintptr_t addr, uintptr_t out[2])
{
    if (write(gPipe[1], (const void*)addr, 16) == 16 && read(gPipe[0], out, 16) == 16)
        return true;
    char drain[64];
    while (read(gPipe[0], drain, sizeof drain) > 0)
    {
    }
    return false;
}

void onSample(int, siginfo_t*, void* context)
{
    if (gBusy.test_and_set(std::memory_order_acquire))
        return;
    int savedErrno = errno;
    const auto& r = ((ucontext_t*)context)->uc_mcontext;
    uintptr_t frames[kMaxFrames];
    int n = 0;
#if defined(__aarch64__)
    frames[n++] = (uintptr_t)r.pc;
    uintptr_t fp = (uintptr_t)r.regs[29];
#else
    // armeabi-v7a: only the function it is in; Thumb code keeps no frame chain to walk.
    frames[n++] = (uintptr_t)r.arm_pc;
    uintptr_t fp = 0;
#endif
    while (n < kMaxFrames && fp && (fp & 15) == 0)
    {
        uintptr_t rec[2];
        if (!readFrame(fp, rec) || rec[0] <= fp || rec[0] - fp > (1u << 22))
            break;
        frames[n++] = rec[1] - 1;    // inside the call, not after it
        fp = rec[0];
    }
    const uint32_t tag = precisionTag();
    bool first = true;
    for (int i = 0; i < n; i++)
    {
        uintptr_t a = frames[i];
        if (a < gBase || a >= gEnd)
            continue;
        uint32_t rel = uint32_t(a - gBase);
        bool dup = false;
        for (int j = 0; j < i && !dup; j++)
            dup = frames[j] == a;
        if (first)
            count(rel | kSelf | tag);
        if (!dup)
            count(rel);
        first = false;
    }
    if (first)
        count(kOutside | (tag >> 28));
    gSamples.fetch_add(1, std::memory_order_relaxed);
    errno = savedErrno;
    gBusy.clear(std::memory_order_release);
}

uint64_t nowMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return uint64_t(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

struct Library
{
    uintptr_t base = 0, end = 0;
    uint8_t idLength = 0;
    uint8_t id[32];
};

int findLibrary(dl_phdr_info* info, size_t, void* out)
{
    Library* lib = (Library*)out;
    uintptr_t self = (uintptr_t)&findLibrary;
    uintptr_t lo = ~(uintptr_t)0, hi = 0;
    for (int i = 0; i < info->dlpi_phnum; i++)
        if (info->dlpi_phdr[i].p_type == PT_LOAD)
        {
            uintptr_t a = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
            lo = a < lo ? a : lo;
            hi = a + info->dlpi_phdr[i].p_memsz > hi ? a + info->dlpi_phdr[i].p_memsz : hi;
        }
    if (self < lo || self >= hi)
        return 0;
    lib->base = info->dlpi_addr;
    lib->end = hi;
    for (int i = 0; i < info->dlpi_phnum; i++)
    {
        if (info->dlpi_phdr[i].p_type != PT_NOTE)
            continue;
        const uint8_t* p = (const uint8_t*)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
        const uint8_t* e = p + info->dlpi_phdr[i].p_memsz;
        while (p + sizeof(ElfW(Nhdr)) <= e)
        {
            const ElfW(Nhdr)* h = (const ElfW(Nhdr)*)p;
            const uint8_t* desc = p + sizeof(*h) + ((h->n_namesz + 3) & ~3u);
            if (h->n_type == NT_GNU_BUILD_ID && h->n_descsz <= sizeof(lib->id))
            {
                lib->idLength = (uint8_t)h->n_descsz;
                memcpy(lib->id, desc, h->n_descsz);
            }
            p = desc + ((h->n_descsz + 3) & ~3u);
        }
    }
    return 1;
}

/* The name of the function an address is in, for the lines in logcat; the
 * library exports every function, but not the ones in anonymous namespaces. */
void name(uint32_t rel, char* out, size_t size)
{
    Dl_info info;
    out[0] = 0;
    if (dladdr((void*)(gBase + rel), &info) && info.dli_sname)
    {
        int status = 0;
        char* d = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        snprintf(out, size, "%s", status == 0 && d ? d : info.dli_sname);
        free(d);
        char* paren = strchr(out, '(');
        if (paren)
            *paren = 0;
    }
    else
        snprintf(out, size, "+0x%x", rel);
}

void* writer(void*)
{
    mkdir("/sdcard/Android/data/dev.nfs3hp.port/files/logs", 0770);
    const char* paths[] = {"/sdcard/Android/data/dev.nfs3hp.port/files/logs/functions.bin", "/data/local/tmp/functions.bin"};
    FILE* f = nullptr;
    for (const char* p : paths)
        if ((f = fopen(p, "wb")))
        {
            SDL_Log("[FUNCS] writing %s", p);
            break;
        }
    if (!f)
    {
        SDL_Log("[FUNCS] cannot open functions.bin");
        return nullptr;
    }
    Library lib;
    dl_iterate_phdr(findLibrary, &lib);
    fwrite("NFF2", 1, 4, f);
    uint64_t base = lib.base;
    fwrite(&base, 8, 1, f);
    fwrite(&lib.idLength, 1, 1, f);
    fwrite(lib.id, 1, lib.idLength, f);

    uint64_t t0 = nowMs();
    for (int tick = 1;; tick++)
    {
        sleep(2);
        uint32_t head[2] = {0xFFFFFFFFu, uint32_t(nowMs() - t0)};
        fwrite(head, 4, 2, f);
        uint32_t topKey = 0, topCount = 0;
        for (uint32_t i = 0; i < kSlots; i++)
        {
            uint32_t k = gKeys[i].load(std::memory_order_relaxed);
            if (!k)
                continue;
            uint32_t c = gCounts[i].load(std::memory_order_relaxed);
            if (c == gSaved[i])
                continue;
            uint32_t rec[2] = {k, c - gSaved[i]};
            if (k < kOutside && (k & kSelf) && rec[1] > topCount)
            {
                topKey = k & 0x0FFFFFFFu;
                topCount = rec[1];
            }
            fwrite(rec, 4, 2, f);
            gSaved[i] = c;
        }
        for (int i = 0; i < kWords; i++)
        {
            uint32_t w = gWords[i].load(std::memory_order_relaxed);
            uint32_t c = gWordCounts[i].load(std::memory_order_relaxed);
            if (w && c != gWordSaved[i])
            {
                uint32_t rec[4] = {kWordMark, w - 1, kWordCount, c - gWordSaved[i]};
                fwrite(rec, 4, 4, f);
                gWordSaved[i] = c;
            }
        }
        fflush(f);
        if (tick % 5 == 0)
        {
            char hot[160];
            name(topKey, hot, sizeof hot);
            SDL_Log("[FUNCS] t=%us samples=%llu dropped=%llu busiest now: %s", head[1] / 1000,
                    (unsigned long long)gSamples.load(), (unsigned long long)gDropped.load(),
                    topCount ? hot : "-");
        }
    }
}
}

void startFunctionLog()
{
    Library lib;
    dl_iterate_phdr(findLibrary, &lib);
    gBase = lib.base;
    gEnd = lib.end;
    if (pipe2(gPipe, O_NONBLOCK | O_CLOEXEC) != 0)
        return;
    x86::FPU::s_controlHook = onControl;

    struct sigaction sa = {};
    sa.sa_sigaction = onSample;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPROF, &sa, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, writer, nullptr);
    pthread_detach(t);

    itimerval timer = {{0, kIntervalUs}, {0, kIntervalUs}};
    setitimer(ITIMER_PROF, &timer, nullptr);
    SDL_Log("[FUNCS] sampling every %d ms", kIntervalUs / 1000);
}
}
