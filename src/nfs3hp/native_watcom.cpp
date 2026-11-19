#include "native_watcom.h"
#include "native_watcom_heap.h"
#include "native_guest.h"
#include "watcom_hooks.h"
#include "os_threads.h"
#include "os_time.h"
#include <lib/thread.h>
#include "os_files.h"
#include "os_system.h"
#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using nfs3hp::guest::EspKept;

/* The Watcom C runtime the game was linked with, in C++: the functions of it
 * the port has natively, where their code was -- each address is bound to the
 * function of that name here (GAME_NATIVES), and the game's code calls it by
 * name.  The names are Open Watcom's (bld/clib,
 * bld/mathlib), the runtime's own descendant, which says what each does;
 * src/nfs3hp/watcom.md maps all of the runtime in nfs3.exe, what is native
 * and what is not yet.
 *
 * They take their arguments in registers, as Watcom's runtime does -- eax,
 * edx, ebx, ecx -- and keep every other register, as it does; the registers
 * an argument came in are the caller's no longer, and are left as they are
 * here.  What they read and write of the runtime's own state -- the
 * per-thread data, the character table, the heap -- is where the runtime
 * keeps it, in guest memory, as the code that is still the game's expects. */

namespace nfs3hp
{
namespace watcom
{

nfs::u32 tzset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);  // 0x515c2c


namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;

/* The runtime's per-thread data: [__GetThreadPtr]() -- the pointer the
 * runtime swaps for its own when a second thread starts (watcom_hooks.h). */
constexpr nfs::u32 kGetThreadPtr = 0x567764;
constexpr nfs::u32 kThreadData = 0x9f642c;  // the single thread's, what GetThreadPtr hands back
constexpr nfs::u32 kRandomSeed = 0xc;        // rand's, in the thread's data
constexpr nfs::u32 kNextToken = 0x10;        // strtok's
/* _IsTable: a byte of flags for each character, EOF first. */
constexpr nfs::u32 kIsTable = 0x564ef0;
constexpr nfs::u8 kSpace = 0x02;
constexpr nfs::u8 kDigit = 0x20;

/* The registers of a call by Watcom's convention: `registers` into eax, edx, ebx and ecx in turn. */
nfs::Args watcomArgs(std::initializer_list<nfs::u32> registers)
{
    nfs::Args a;
    nfs::u32* const into[] = {&a.eax, &a.edx, &a.ebx, &a.ecx};
    unsigned i = 0;
    for (const nfs::u32 value : registers)
        *into[i++] = value;
    return a;
}

/* A call through a pointer the runtime holds -- one of its hooks (watcom_hooks.h), or a function its
 * caller handed in or its tables list -- as its code makes it: `registers` into eax, edx, ebx and
 * ecx in turn; its eax, nullopt when the game stopped in it. */
template <nfs::u32 Slot>
std::optional<nfs::u32> hook(nfs::App* app, nfs::ThreadState& ts,
                               std::initializer_list<nfs::u32> registers = {})
{
    nfs::Args a = watcomArgs(registers);
    return guest::pointerCall(app, ts, at(app, Slot), a);
}

/* The hook at Slot called with the registers of `args` (the thunk's own: what its code has in them at
 * the call), what the hook leaves in them left there, eax too; false when the game stopped. */
template <nfs::u32 Slot>
bool hookWith(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<nfs::u32> eax = guest::pointerCall(app, ts, at(app, Slot), args);
    if (!eax)
        return false;
    args.eax = *eax;
    return true;
}

std::optional<nfs::u32> through(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target,
                                  std::initializer_list<nfs::u32> registers = {},
                                  std::initializer_list<nfs::u32> words = {})
{
    nfs::Args a = watcomArgs(registers);
    return guest::pointerCall(app, ts, target, a, words);
}

/* One of the runtime's own functions, which its code calls, run as C++ where the call is: the
 * return address's slot under esp, and every register but eax as the caller had it, as the
 * function's thunk keeps them (Kept), esp back after.  What `function` gives back. */
template <typename Function>
auto called(nfs::ThreadState& ts, Function function)
{
    const EspKept kept(ts);
    ts.sp -= 4;
    return function();
}

/* Room on the guest's stack for what a callee is handed the address of: the code's frame (`pushed`
 * bytes of registers under esp on entry, then `bytes`), esp under it while the core runs and back
 * after; an address in it as the frame's base less the offset. */
class GuestFrame
{
public:
    GuestFrame(nfs::ThreadState& ts, nfs::u32 pushed, nfs::u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    /* The registers the code saves, `saved`, where its pushes put them: what a frame at this
     * depth later finds there. */
    GuestFrame(nfs::App* app, nfs::ThreadState& ts, std::initializer_list<nfs::u32> saved, nfs::u32 bytes)
        : ts_(ts), esp_(ts.sp), base_(ts.sp - 4 * nfs::u32(saved.size()))
    {
        nfs::u32 p = esp_;
        for (const nfs::u32 value : saved)
            put(app, p -= 4, value);
        ts.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    nfs::u32 operator-(nfs::u32 offset) const { return base_ - offset; }

private:
    nfs::ThreadState& ts_;
    const nfs::u32 esp_;
    const nfs::u32 base_;
};

/* The return address's slot under esp, as a call makes it: a core that keeps a frame on the
 * guest's stack is called at the code's depth. */
class ReturnSlot
{
public:
    explicit ReturnSlot(nfs::ThreadState& ts) : ts_(ts), esp_(ts.sp) { ts.sp -= 4; }
    ~ReturnSlot() { ts_.sp = esp_; }
    ReturnSlot(const ReturnSlot&) = delete;
    ReturnSlot& operator=(const ReturnSlot&) = delete;

private:
    nfs::ThreadState& ts_;
    const nfs::u32 esp_;
};

/* The thread's data, as the runtime asks for it; false when the game
 * stopped. */
bool threadData(nfs::App* app, nfs::ThreadState& ts, nfs::u32& data)
{
    const auto eax = hook<kGetThreadPtr>(app, ts);
    if (!eax)
        return false;
    data = *eax;
    return true;
}

void returns(nfs::ThreadState& ts, nfs::Args& args, nfs::u32 result)
{
    args.eax = result;
    ts.sp += 4;
}

/* The registers but eax, as a function had them when it was called (its thunk's args). */
struct Kept
{
    nfs::u32 ebx, ecx, edx, esi, edi, ebp;
    explicit Kept(const nfs::Args& args)
        : ebx(args.ebx), ecx(args.ecx), edx(args.edx), esi(args.esi), edi(args.edi), ebp(args.ebp)
    {
    }
    void back(nfs::Args& args) const
    {
        args.ebx = ebx;
        args.ecx = ecx;
        args.edx = edx;
        args.esi = esi;
        args.edi = edi;
        args.ebp = ebp;
    }
};

/* Plan step 7: the runtime's string and memory functions on the host's C library, over guest
 * memory where it lies in the host's (the guest's flat memory): `hostBytes` the bytes at a guest
 * address, `apart` whether two ranges share no byte and neither wraps past 4 GB -- where the
 * runtime's own loops (a dword at a time, a byte at a time) and the host's copies cannot differ.
 * Where they can, the runtime's loops stay. */
char* hostBytes(nfs::App* app, nfs::u32 address)
{
    return &app->getMemory<char>(address);
}

bool apart(nfs::u32 a, nfs::u32 aLength, nfs::u32 b, nfs::u32 bLength)
{
    const std::uint64_t aEnd = std::uint64_t(a) + aLength, bEnd = std::uint64_t(b) + bLength;
    return aEnd <= 0x100000000ull && bEnd <= 0x100000000ull && (aEnd <= b || bEnd <= a);
}

}  // namespace

/* __CHP (mathlib/a/fchop87.asm, 0x4dfd56): st(0) rounded toward zero -- the
 * game's step from a float to an integer, before every fistp -- by frndint
 * under the caller's control word with truncation in it.  Nothing else
 * changes: the registers, the control word, the status word. */
nfs::u32 CHP(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = clib::chop(app, args.floatIn[0]);
    ts.sp += 4;
    return args.eax;
}

double clib::chop(nfs::App* app, double value)
{
    NFS2_USE(app);
    return std::trunc(value);
}

/* __GetThreadPtr (0x4ff0ec): the single thread's data, until a second thread
 * starts and the runtime points kGetThreadPtr at its own. */
nfs::u32 GetThreadPtr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, at(app, kThreadData));
    return args.eax;
}

/* __NullAccessRtn (0x4ff0f4): what the runtime's locks are -- the streams',
 * the heap's -- while there is one thread: nothing. */
nfs::u32 NullAccess(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    ts.sp += 4;
    return args.eax;
}

/* memset (clib/memory/c/memset.c, 0x4e0640): eax the buffer, edx the byte,
 * ebx how many; the buffer back. */
nfs::u32 clib::memset(nfs::App* app, nfs::u32 buffer, nfs::u32 value, nfs::u32 count)
{
    if (count != 0)
        std::memset(&app->getMemory<nfs::u8>(buffer), int(value & 0xff), count);
    return buffer;
}

nfs::u32 memset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::memset(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* __STOSD (clib/cgsupp/a/__stos.asm, 0x4ff4f7): ecx dwords of edx from eax
 * on -- one by one to a 32-byte boundary, then four at a time, then up to
 * three; eax left past them, and ecx 0, but 1 after three at the end (the
 * last of them is not counted down). */
nfs::u32 STOSD(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    for (; args.ecx != 0 && (args.eax & 0x1f) != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    const nfs::u32 tail = args.ecx & 3;
    for (; args.ecx != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    args.ecx = tail == 3 ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* __STOSB (clib/cgsupp/a/__stos.asm, 0x4ff4c0): ecx bytes of the pattern in
 * edx from eax on -- byte by byte up to a dword boundary, the pattern turned
 * a byte for each, then dwords (__STOSD), then up to three bytes more: dl,
 * dh, dl.  eax is left past the dwords, edx as turned, ecx 0 -- or 1 after
 * three bytes at the end, as __STOSD. */
nfs::u32 STOSB(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    for (; args.ecx != 0 && (args.eax & 3) != 0; --args.ecx, ++args.eax)
    {
        putByte(app, args.eax, nfs::u8(args.edx));
        args.edx = args.edx >> 8 | args.edx << 24;
    }
    const nfs::u32 tail = args.ecx & 3;
    for (args.ecx >>= 2; args.ecx != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    const nfs::u8 bytes[] = {nfs::u8(args.edx), nfs::u8(args.edx >> 8), nfs::u8(args.edx)};
    for (nfs::u32 i = 0; i < tail; ++i)
        putByte(app, args.eax + i, bytes[i]);
    args.ecx = tail == 3 ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* strcmp (clib/string/a/cmp386.asm, 0x4ee310): eax and edx, the strings;
 * -1, 0 or 1 as the first byte that differs, unsigned, is below or above. */
nfs::u32 clib::strcmp(nfs::App* app, nfs::u32 a, nfs::u32 b)
{
    // The host's, which compares as unsigned bytes too; its sign as -1, 0 or 1.
    const int d = std::strcmp(hostBytes(app, a), hostBytes(app, b));
    return d < 0 ? 0xffffffff : d > 0 ? 1 : 0;
}

nfs::u32 strcmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strcmp(app, args.eax, args.edx));
    return args.eax;
}

/* strncmp (clib/string/c/strncmp.c, 0x4eb390): eax and edx, the strings, ebx
 * the most to compare; the first difference of unsigned bytes, or 0. */
nfs::u32 clib::strncmp(nfs::App* app, nfs::u32 a, nfs::u32 b, nfs::u32 count)
{
    for (; count != 0; ++a, ++b, --count)
    {
        const nfs::u8 c = byte(app, a), d = byte(app, b);
        if (c != d)
            return nfs::u32(c) - nfs::u32(d);
        if (c == 0)
            break;
    }
    return 0;
}

nfs::u32 strncmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strncmp(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* _stricmp (clib/string/c/stricmp.c, 0x4e9ec0): strcmp's difference, unsigned,
 * with A to Z taken as a to z and nothing else folded. */
nfs::u32 clib::stricmp(nfs::App* app, nfs::u32 a, nfs::u32 b)
{
    const auto lower = [](nfs::u8 c) { return nfs::u8(c >= 'A' && c <= 'Z' ? c + 0x20 : c); };
    for (;; ++a, ++b)
    {
        const nfs::u8 c = lower(byte(app, a)), d = lower(byte(app, b));
        if (c != d || d == 0)
            return nfs::u32(c) - nfs::u32(d);
    }
}

nfs::u32 stricmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::stricmp(app, args.eax, args.edx));
    return args.eax;
}

/* strncpy (clib/string/c/strncpy.c, 0x4e0e30): eax the buffer, edx the
 * string, ebx how many bytes: the string, then zeros to make up the count. */
nfs::u32 clib::strncpy(nfs::App* app, nfs::u32 buffer, nfs::u32 string, nfs::u32 count)
{
    // The host's copy and fill where the string and the buffer share no byte; a byte at a time,
    // each read before it is written, where they do.
    const nfs::u32 length = nfs::u32(strnlen(hostBytes(app, string), count));
    if (apart(buffer, count, string, length + (length < count ? 1 : 0)))
    {
        std::memcpy(hostBytes(app, buffer), hostBytes(app, string), length);
        std::memset(hostBytes(app, buffer) + length, 0, count - length);
        return buffer;
    }
    nfs::u32 to = buffer, from = string;
    for (; count != 0 && byte(app, from) != 0; --count)
        putByte(app, to++, byte(app, from++));
    for (; count != 0; --count)
        putByte(app, to++, 0);
    return buffer;
}

nfs::u32 strncpy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strncpy(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* strncat (clib/string/c/strncat.c, 0x4eedf0): eax the string, edx what to add,
 * ebx at most how much of it; always ended. */
nfs::u32 clib::strncat(nfs::App* app, nfs::u32 string, nfs::u32 added, nfs::u32 count)
{
    nfs::u32 to = string;
    while (byte(app, to) != 0)
        ++to;
    for (nfs::u32 from = added; count != 0; --count, ++to, ++from)
    {
        const nfs::u8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
    putByte(app, to, 0);
    return string;
}

nfs::u32 strncat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strncat(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* strrchr (clib/string/c/strrchr.c, 0x4ee2f0): eax the string, dl the byte;
 * where it is last, the end included, or 0. */
nfs::u32 clib::strrchr(nfs::App* app, nfs::u32 string, nfs::u32 c)
{
    const char* const from = hostBytes(app, string);
    const char* const found = std::strrchr(from, int(nfs::u8(c)));
    return found != nullptr ? string + nfs::u32(found - from) : 0;
}

nfs::u32 strrchr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strrchr(app, args.eax, args.edx));
    return args.eax;
}

/* strstr (clib/string/c/strstr.c, 0x4e0820): eax the string, edx what to look
 * for; where it first is, or 0 -- the string itself for an empty one. */
nfs::u32 clib::strstr(nfs::App* app, nfs::u32 string, nfs::u32 wanted)
{
    const char* const from = hostBytes(app, string);
    const char* const found = std::strstr(from, hostBytes(app, wanted));
    return found != nullptr ? string + nfs::u32(found - from) : 0;
}

nfs::u32 strstr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::strstr(app, args.eax, args.edx));
    return args.eax;
}

/* __setbits (clib/string/c/setbits.c, 0x50c540): eax 32 bytes, made the set
 * of the bytes of the string at edx -- a bit each, the low bit first. */
nfs::u32 setbits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 vector = args.eax;
    std::memset(&app->getMemory<nfs::u8>(vector), 0, 0x20);
    for (nfs::u32 at = args.edx; byte(app, at) != 0; ++at)
    {
        const nfs::u8 c = byte(app, at);
        putByte(app, vector + (c >> 3), nfs::u8(byte(app, vector + (c >> 3)) | 1 << (c & 7)));
    }
    ts.sp += 4;
    return args.eax;
}

/* strtok (clib/string/c/strtok.c, 0x4eff60): eax the string -- 0 to go on
 * where the last call stopped, kept in the thread's data -- edx the bytes
 * that part the tokens; the next token, ended, or 0 when there is none. */
/* strtok's work: the next token into `result`; false when the game stopped. */
bool strtokOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 text, const nfs::u32 parts,
              nfs::u32& result)
{
    nfs::u32 data = 0;
    if (text == 0)
    {
        if (!threadData(app, ts, data))
            return false;
        text = at(app, data + kNextToken);
        if (text == 0)
            return result = 0, true;
    }
    std::bitset<256> parting;
    for (nfs::u32 at = parts; byte(app, at) != 0; ++at)
        parting.set(byte(app, at));
    while (byte(app, text) != 0 && parting.test(byte(app, text)))
        ++text;
    if (byte(app, text) == 0)
        return result = 0, true;
    for (nfs::u32 at = text; byte(app, at) != 0; ++at)
        if (parting.test(byte(app, at)))
        {
            putByte(app, at, 0);
            if (!threadData(app, ts, data))
                return false;
            put(app, data + kNextToken, at + 1);
            return result = text, true;
        }
    if (!threadData(app, ts, data))
        return false;
    put(app, data + kNextToken, 0);
    result = text;
    return true;
}

nfs::u32 strtok(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 result;
    if (!strtokOf(app, ts, args.eax, args.edx, result))
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* _strupr (clib/string/c/strupr.c, 0x4f8020): a to z of the string at eax made
 * A to Z, in place; the string back. */
nfs::u32 strupr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    for (nfs::u32 at = args.eax; byte(app, at) != 0; ++at)
        if (byte(app, at) >= 'a' && byte(app, at) <= 'z')
            putByte(app, at, nfs::u8(byte(app, at) - 0x20));
    returns(ts, args, args.eax);
    return args.eax;
}

/* toupper and tolower (clib/char/c, 0x4eede0 and 0x4f0ae0): eax a character,
 * moved between a-z and A-Z; anything else as it is. */
nfs::u32 clib::toupper(nfs::App* app, nfs::u32 c)
{
    NFS2_USE(app);
    const nfs::s32 value = nfs::s32(c);
    return value >= 'a' && value <= 'z' ? c - 0x20 : c;
}

nfs::u32 clib::tolower(nfs::App* app, nfs::u32 c)
{
    NFS2_USE(app);
    const nfs::s32 value = nfs::s32(c);
    return value >= 'A' && value <= 'Z' ? c + 0x20 : c;
}

nfs::u32 toupper(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::toupper(app, args.eax));
    return args.eax;
}

nfs::u32 tolower(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::tolower(app, args.eax));
    return args.eax;
}

/* atoi and atol (clib/convert/c, 0x4f0430 and 0x4f0490, the same code): the
 * number at eax, past white space, a sign and then digits as _IsTable has
 * them, in 32 bits as it wraps.  The table is looked up at the character
 * plus one in a byte: 0xff at EOF's place. */
nfs::u32 clib::atoi(nfs::App* app, nfs::u32 string)
{
    const auto is = [&](nfs::u8 c, nfs::u8 what) {
        return (byte(app, kIsTable + nfs::u8(c + 1)) & what) != 0;
    };
    nfs::u32 at = string;
    while (is(byte(app, at), kSpace))
        ++at;
    const nfs::u8 sign = byte(app, at);
    if (sign == '+' || sign == '-')
        ++at;
    nfs::u32 value = 0;
    for (; is(byte(app, at), kDigit); ++at)
        value = value * 10 + byte(app, at) - '0';
    return sign == '-' ? 0 - value : value;
}

nfs::u32 atoi(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, clib::atoi(app, args.eax));
    return args.eax;
}

nfs::u32 atol(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    atoi(app, ts, args);
    return args.eax;
}

namespace
{

/* Where rand keeps its seed: 0 if the thread has no data.  False when the
 * game stopped. */
bool seedAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32& seed)
{
    if (!threadData(app, ts, seed))
        return false;
    seed += kRandomSeed;
    return true;
}

}  // namespace

/* initrandnext (clib/math/c/rand.c, 0x4eb1f0): where rand keeps its seed. */
nfs::u32 initrandnext(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 seed;
    if (seedAt(app, ts, seed))
        returns(ts, args, seed);
    return args.eax;
}

/* rand (clib/math/c/rand.c, 0x4eb1fc): the seed taken on, x 1103515245 +
 * 12345, and bits 16 to 30 of it. */
/* rand's work: the next number into `result`; false when the game stopped. */
bool randOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32& result)
{
    nfs::u32 seed;
    if (!seedAt(app, ts, seed))
        return false;
    if (seed == 0)
        return result = 0, true;
    const nfs::u32 next = at(app, seed) * 0x41c64e6d + 0x3039;
    put(app, seed, next);
    result = next >> 16 & 0x7fff;
    return true;
}

nfs::u32 rand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 result;
    if (!randOf(app, ts, result))
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* srand (clib/math/c/rand.c, 0x4eb220): eax the seed. */
bool srandOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value, nfs::u32& seed)
{
    if (!seedAt(app, ts, seed))
        return false;
    if (seed != 0)
        put(app, seed, value);
    return true;
}

nfs::u32 srand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 seed;
    if (srandOf(app, ts, args.eax, seed))
        returns(ts, args, seed);
    return args.eax;
}

/* ---- Search and sort (clib/search/c): each comparison is the caller's
 * function called as the runtime calls it -- the same elements in the same
 * order, and esp where the runtime's frame has it, for qsort's pivot lies in
 * that frame. ---- */

namespace
{

/* The caller's function on a and b -- in eax and edx, Watcom's registers --
 * called with esp at `esp`; its result.  False when the game stopped in it. */
bool compare(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::u32 function, nfs::u32 a,
             nfs::u32 b, nfs::s32& result)
{
    ts.sp = esp;
    const auto r = through(app, ts, function, {a, b});
    if (!r)
        return false;
    result = nfs::s32(*r);
    return true;
}

/* byteswap's exchange of `size` bytes at p and q: a dword at a time, then a
 * byte -- q read, then p, p written, then q, as its xchg and stos do; what
 * it leaves in eax, the last of p's it moved, given `eax` before. */
nfs::u32 exchange(nfs::App* app, nfs::u32 p, nfs::u32 q, nfs::u32 size, nfs::u32 eax)
{
    for (nfs::u32 i = size >> 2; i != 0; --i, p += 4, q += 4)
    {
        const nfs::u32 fromQ = at(app, q);
        eax = at(app, p);
        put(app, p, fromQ);
        put(app, q, eax);
    }
    for (nfs::u32 i = size & 3; i != 0; --i, ++p, ++q)
    {
        const nfs::u8 fromQ = byte(app, q);
        eax = (eax & 0xffffff00) | byte(app, p);
        putByte(app, p, fromQ);
        putByte(app, q, nfs::u8(eax));
    }
    return eax;
}

/* med3's choice of a, b and c, the median by the caller's function, for a
 * med3 whose return address is at `entry`: its five pushes below that. */
bool median(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 function, nfs::u32 a,
            nfs::u32 b, nfs::u32 c, nfs::u32& chosen)
{
    const nfs::u32 esp = entry - 0x14;  // esi, edi, es, fs, gs
    nfs::s32 r;
    if (!compare(app, ts, esp, function, a, b, r))
        return false;
    const bool aAbove = r > 0;
    if (!compare(app, ts, esp, function, a, c, r))
        return false;
    if (aAbove ? r <= 0 : r >= 0)
    {
        chosen = a;
        return true;
    }
    if (!compare(app, ts, esp, function, b, c, r))
        return false;
    chosen = (r <= 0) == aAbove ? c : b;
    return true;
}

}  // namespace

/* med3 (clib/search/c/qsort.c, 0x4ee480): the median of the elements at eax,
 * edx and ebx by the function at ecx. */
nfs::u32 med3(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::u32 chosen;
    if (!median(app, ts, entry, args.ecx, args.eax, args.edx, args.ebx, chosen))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, chosen);
    return args.eax;
}

/* byteswap (clib/search/c/qsortrtn.c, 0x4ee4d0): ecx bytes at esi and edi
 * exchanged -- its own convention, esi and edi left past them, ecx and edx
 * 0. */
nfs::u32 byteswap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = exchange(app, args.esi, args.edi, args.ecx, args.eax);
    args.esi += args.ecx;
    args.edi += args.ecx;
    args.ecx = 0;
    args.edx = 0;
    ts.sp += 4;
    return args.eax;
}

/* qsort (clib/search/c/qsortrtn.c, 0x4ee4f8): edx elements of ebx bytes at
 * eax sorted by the function at ecx.  Fewer than 16 by a shell sort, steps of
 * 3 and 1; more by quicksort, the pivot the median of 3 from 30 on, of 9 from
 * 43 on, the elements equal to it gathered at both ends and swapped to the
 * middle, the larger part kept for later.  An array of dwords, aligned, has
 * its pivot copied into the frame and the function handed that. */
/* The work of qsort below, from its entry (esp at its return address's slot); false when the game
 * stopped. */
bool qsortOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 base, nfs::u32 n,
             const nfs::u32 size, const nfs::u32 function)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 frame = entry - 0x168;  // esi, edi, es, fs, gs, ebp, and 0x150 of its own
    const nfs::u32 pivotCopy = frame + 0x100;
    const int swaptype = ((base | size) & 3) != 0 ? 2 : size > 4 ? 1 : 0;
    auto swap = [&](nfs::u32 p, nfs::u32 q) {
        if (swaptype != 0)
        {
            exchange(app, p, q, size, 0);
        }
        else
        {
            const nfs::u32 t = at(app, p);
            put(app, p, at(app, q));
            put(app, q, t);
        }
    };
    nfs::u32 laterBase[32], laterCount[32];  // the larger parts, for later: the runtime's 32
    unsigned later = 0;
    nfs::s32 r;
    for (;;)
    {
        if (n > 1 && n < 16)
        {
            const nfs::u32 end = base + n * size;
            for (nfs::u32 shell = 3 * size; nfs::s32(shell) > 0; shell -= 2 * size)
                for (nfs::u32 p1 = base + shell; p1 < end; p1 += shell)
                    for (nfs::u32 p2 = p1; p2 > base; p2 -= shell)
                    {
                        if (!compare(app, ts, frame, function, p2 - shell, p2, r))
                            return false;
                        if (r <= 0)
                            break;
                        swap(p2, p2 - shell);
                    }
        }
        else if (n >= 16)
        {
            nfs::u32 mid = base + (n >> 1) * size;
            if (n > 29)
            {
                nfs::u32 p1 = base;
                nfs::u32 p2 = base + (n - 1) * size;
                if (n > 42)
                {
                    const nfs::u32 s = (n >> 3) * size;
                    if (!median(app, ts, frame - 4, function, p1, p1 + s, p1 + 2 * s, p1)
                        || !median(app, ts, frame - 4, function, mid - s, mid, mid + s, mid)
                        || !median(app, ts, frame - 4, function, p2 - 2 * s, p2 - s, p2, p2))
                        return false;
                }
                if (!median(app, ts, frame - 4, function, p1, mid, p2, mid))
                    return false;
            }
            nfs::u32 pivot = base;
            if (swaptype != 0)
            {
                swap(base, mid);
            }
            else
            {
                pivot = pivotCopy;
                put(app, pivotCopy, at(app, mid));
            }
            nfs::u32 pa = base, pb = base;
            nfs::u32 pc = base + (n - 1) * size, pd = pc;
            nfs::u32 count = n;
            for (;;)
            {
                for (; count != 0; pb += size, --count)
                {
                    if (!compare(app, ts, frame, function, pb, pivot, r))
                        return false;
                    if (r > 0)
                        break;
                    if (r == 0)
                    {
                        swap(pa, pb);
                        pa += size;
                    }
                }
                for (; count != 0; pc -= size, --count)
                {
                    if (!compare(app, ts, frame, function, pc, pivot, r))
                        return false;
                    if (r < 0)
                        break;
                    if (r == 0)
                    {
                        swap(pc, pd);
                        pd -= size;
                    }
                }
                if (count == 0)
                    break;
                swap(pb, pc);
                pb += size;
                if (--count == 0)
                    break;
                pc -= size;
                --count;
            }
            // The equal ones from both ends into the middle.
            const nfs::u32 pn = base + n * size;
            nfs::u32 moved = nfs::s32(pa - base) < nfs::s32(pb - pa) ? pa - base : pb - pa;
            if (moved != 0)
                exchange(app, base, pb - moved, moved, 0);
            moved = std::min(pd - pc, pn - pd - size);
            if (moved != 0)
                exchange(app, pb, pn - moved, moved, 0);
            // The smaller part now, the larger later.
            const nfs::u32 below = pb - pa, above = pd - pc;
            if (above >= below)
            {
                laterBase[later] = pn - above;
                laterCount[later] = above / size;
                n = below / size;
                ++later;
                continue;
            }
            if (below > size)
            {
                laterBase[later] = base;
                laterCount[later] = below / size;
                n = above / size;
                base = pn - above;
                ++later;
                continue;
            }
        }
        if (later == 0)
            break;
        --later;
        base = laterBase[later];
        n = laterCount[later];
    }
    return true;
}

nfs::u32 qsort(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    if (!qsortOf(app, ts, args.eax, args.edx, args.ebx, args.ecx))
        return args.eax;
    kept.back(args);
    ts.sp = entry + 4;
    return args.eax;
}

/* bsearch (clib/search/c/bsearch.c, 0x4fb300): the element of edx's ebx, ecx
 * bytes each, that the function on the stack finds equal to the key at eax,
 * halving from both ends; or 0. */
bool bsearchOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 key, nfs::u32 base,
               nfs::u32 count, nfs::u32 size, nfs::u32 function, nfs::u32& found)
{
    const nfs::u32 frame = entry - 0x1c;  // esi, edi, es, fs, gs, ebp, and the function
    found = 0;
    if (count != 0)
    {
        nfs::u32 low = base, high = base + (count - 1) * size;
        nfs::s32 r = 1;
        while (low < high)
        {
            const nfs::u32 mid = low + (high - low) / size / 2 * size;
            if (!compare(app, ts, frame, function, key, mid, r))
                return false;
            if (r == 0)
            {
                found = mid;
                break;
            }
            if (r < 0)
                high = mid;
            else
                low = mid + size;
        }
        if (r != 0 && low == high)
        {
            if (!compare(app, ts, frame, function, key, high, r))
                return false;
            found = r == 0 ? high : 0;
        }
    }
    return true;
}

nfs::u32 bsearch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::u32 found;
    if (!bsearchOf(app, ts, entry, args.eax, args.edx, args.ebx, args.ecx, at(app, entry + 4), found))
        return args.eax;
    kept.back(args);
    args.eax = found;
    ts.sp = entry + 8;
    return args.eax;
}

/* ---- errno (clib/startup/c/seterrno.c, doserrno.c; clib/_dos/c/dosret.c):
 * the thread's, errno at +4 and _doserrno at +8.  Those that set it leave
 * eax with the thread's data, as theirs do; those that turn a Windows error
 * into it, -1. ---- */

namespace
{

constexpr nfs::u32 kErrno = 4, kDosErrno = 8;  // in the thread's data
/* __set_errno_dos's errno for each of Windows' errors up to 19, a signed
 * byte each; the rest take 19's. */
constexpr nfs::u32 kErrnoOfError = 0x567888;
/* Watcom's errno values. */
constexpr nfs::u32 kENOENT = 1, kENOMEM = 5, kEEXIST = 7, kEINVAL = 9, kERANGE = 14;

/* A word of the thread's data set -- the data in `data`, if asked, which its code leaves in eax;
 * false when the game stopped. */
bool setThreadWord(nfs::App* app, nfs::ThreadState& ts, nfs::u32 offset, nfs::u32 value,
                   nfs::u32* data = nullptr)
{
    nfs::u32 at;
    if (!threadData(app, ts, at))
        return false;
    put(app, at + offset, value);
    if (data != nullptr)
        *data = at;
    return true;
}

/* __set_errno_dos's: _doserrno the error, errno what it stands for. */
bool setErrnoDos(nfs::App* app, nfs::ThreadState& ts, nfs::u32 error)
{
    if (!setThreadWord(app, ts, kDosErrno, error))
        return false;
    nfs::u32 value;
    if (error == 0x7b)  // ERROR_INVALID_NAME
        value = kENOENT;
    else if (error == 0xce)  // ERROR_FILENAME_EXCED_RANGE
        value = kEINVAL;
    else if (error == 0xb7)  // ERROR_ALREADY_EXISTS
        value = kEEXIST;
    else
        value = nfs::u32(nfs::s32(nfs::s8(byte(app, kErrnoOfError + std::min<nfs::u32>(error, 0x13)))));
    return setThreadWord(app, ts, kErrno, value);
}

/* __set_errno_nt's: the system's error, as __set_errno_dos has it.  The port's calls give no
 * error's code: it is 0. */
bool setErrnoNt(nfs::App* app, nfs::ThreadState& ts)
{
    return setErrnoDos(app, ts, 0);
}

/* What the runtime makes of a call's success: 0, or errno (__set_errno_nt's) and -1. */
bool callResult(nfs::App* app, nfs::ThreadState& ts, bool succeeded, nfs::u32& result)
{
    result = succeeded ? 0 : 0xffffffff;
    return succeeded || setErrnoNt(app, ts);
}

}  // namespace

/* __set_errno (0x502880): errno eax. */
nfs::u32 set_errno(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, ts, kErrno, args.eax))
        return args.eax;
    const nfs::u32 data = args.eax;
    kept.back(args);
    returns(ts, args, data);
    return args.eax;
}

/* __set_ERANGE (0x5028a4): errno ERANGE. */
nfs::u32 set_ERANGE(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, ts, kErrno, kERANGE))
        return args.eax;
    const nfs::u32 data = args.eax;
    kept.back(args);
    returns(ts, args, data);
    return args.eax;
}

/* __set_doserrno (0x5028c8): _doserrno eax. */
nfs::u32 set_doserrno(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, ts, kDosErrno, args.eax))
        return args.eax;
    const nfs::u32 data = args.eax;
    kept.back(args);
    returns(ts, args, data);
    return args.eax;
}

/* __set_errno_dos (0x500f8c): errno for the Windows error in eax; -1. */
nfs::u32 set_errno_dos(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setErrnoDos(app, ts, args.eax))
        return args.eax;
    kept.back(args);
    returns(ts, args, 0xffffffff);
    return args.eax;
}

/* __set_errno_nt (0x500ff4): errno for GetLastError's; -1. */
nfs::u32 set_errno_nt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setErrnoNt(app, ts))
        return args.eax;
    kept.back(args);
    returns(ts, args, 0xffffffff);
    return args.eax;
}

/* ---- Files and directories: each one of the port's calls on the path at
 * eax, 0 or -1 and errno. ---- */

namespace
{

/* A function of one call on the path, `succeeded` its answer. */
void pathResult(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, bool succeeded)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!callResult(app, ts, succeeded, result))
        return;
    kept.back(args);
    returns(ts, args, result);
}

/* What the runtime's attributes say of a directory (FILE_ATTRIBUTE_DIRECTORY). */
constexpr nfs::u32 kDirectoryAttribute = 0x10;

/* __fixed_GetFileAttributesA's: the attributes of the file the path finds --
 * a directory's, or none -- or -1, for a path with * or ? in it too.  The
 * search is left open, as it leaves it. */
nfs::u32 fileAttributes(nfs::App* app, nfs::u32 path)
{
    for (nfs::u32 c = path; byte(app, c) != 0; ++c)
        if (byte(app, c) == '*' || byte(app, c) == '?')
            return 0xffffffff;
    os::files::Found found;
    if (!os::files::findFirst(app, guest::text(app, path), found))
        return 0xffffffff;
    return found.directory ? kDirectoryAttribute : 0;
}

/* unlink's: the file at `path` deleted; 0, or -1 and errno. */
bool unlinkOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32& result)
{
    return callResult(app, ts, os::files::remove(guest::text(app, path)), result);
}

}  // namespace

/* unlink (clib/handleio/c/unlnkwnt.c, 0x4e28a0): the file deleted. */
nfs::u32 unlink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    pathResult(app, ts, args, os::files::remove(guest::text(app, args.eax)));
    return args.eax;
}

/* remove (clib/file/c/remove.c, 0x4ee2e0): unlink. */
nfs::u32 remove(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    unlink(app, ts, args);
    return args.eax;
}

/* mkdir (clib/direct/c/mkdirwnt.c, 0x4eb1d0): the directory made. */
nfs::u32 mkdir(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    pathResult(app, ts, args, os::files::makeDirectory(guest::text(app, args.eax)));
    return args.eax;
}

/* rmdir (clib/direct/c/rmdirwnt.c, 0x4ee400): the directory removed. */
nfs::u32 rmdir(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    pathResult(app, ts, args, os::files::removeDirectory(guest::text(app, args.eax)));
    return args.eax;
}

/* chdir (clib/direct/c/chdirwnt.c, 0x4f0260): the current directory set. */
nfs::u32 chdir(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    os::files::setCurrentDirectory(guest::text(app, args.eax));
    pathResult(app, ts, args, true);
    return args.eax;
}

/* chmod (clib/file/c/chmodwnt.c, 0x4ee3c0): the file read-only unless edx
 * has S_IWRITE (0x80), its other attributes as __fixed_GetFileAttributesA
 * finds them.  The port keeps no attributes: none are set, and the code,
 * which takes its result from eax -- the path, not the setting's answer --
 * reports the file changed. */
nfs::u32 chmod(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    const nfs::u32 attributes = fileAttributes(app, args.eax);
    nfs::u32 result;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, ts))
            return args.eax;
    }
    else if (!callResult(app, ts, args.eax != 0, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* access (clib/file/c/accsswnt.c, 0x4f05f0): 0 if the file is there, and
 * not read-only if edx asks for writing (2).  The code takes the attributes
 * from eax -- the path's address, not what a lookup of it finds -- so a file
 * is always there, read-only when the address is odd. */
nfs::u32 access(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    const nfs::u32 mode = args.edx;
    const nfs::u32 attributes = args.eax;
    nfs::u32 result = 0;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, ts))
            return args.eax;
    }
    else if ((mode & 2) != 0 && (attributes & 1) != 0)
    {
        result = 0xffffffff;
        if (!setErrnoDos(app, ts, 5))  // ERROR_ACCESS_DENIED
            return args.eax;
    }
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

nfs::u32 nmalloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nfree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sopen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

/* getcwd (clib/direct/c/gtcwdwnt.c, 0x4f0730): the current directory into
 * eax's edx bytes -- or, for no buffer, into a block _nmalloc gives of edx
 * bytes or as many as it takes -- or 0: errno ERANGE when it does not fit,
 * ENOMEM when there is no block.  It copies all of the directory, however
 * long. */
std::optional<nfs::u32> getcwdOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size)
{
    const nfs::u32 entry = ts.sp;
    // The four registers the code pushes and its MAX_PATH bytes below them: the depth it calls at.
    ts.sp = entry - 0x114;
    const std::string& directory = os::files::currentDirectory();
    const nfs::u32 length = nfs::u32(directory.size());
    nfs::u32 result = 0;
    if (length == 0)
    {
        if (!setErrnoNt(app, ts))
            return std::nullopt;
    }
    else
    {
        bool fits = true;
        if (buffer == 0)
        {
            buffer = clib::malloc(app, ts, std::max(size, length + 1));
            if (ts.terminate)
                return std::nullopt;
            if (buffer == 0)
            {
                fits = false;
                if (!setThreadWord(app, ts, kErrno, kENOMEM))
                    return std::nullopt;
            }
        }
        else if (length > size)
        {
            fits = false;
            if (!setThreadWord(app, ts, kErrno, kERANGE))
                return std::nullopt;
        }
        if (fits)
        {
            guest::putText(app, buffer, directory.c_str());
            result = buffer;
        }
    }
    ts.sp = entry;
    return result;
}

nfs::u32 getcwd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (const auto result = getcwdOf(app, ts, args.eax, args.edx))
        returns(ts, args, *result);
    return args.eax;
}

/* _getdiskfree (clib/direct/c/diskfree.c, 0x4f07d0): the volume of drive eax
 * -- 1 A:, 0 the current one -- into edx's diskfree_t: total and free
 * clusters, sectors a cluster, bytes a sector; 0.  Every drive has the
 * port's one volume. */
std::optional<nfs::u32> getdiskfreeOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 drive, nfs::u32 info)
{
    NFS2_USE(ts);
    NFS2_USE(drive);
    const os::files::Volume& volume = os::files::kVolume;
    put(app, info, volume.totalClusters);
    put(app, info + 4, volume.freeClusters);
    put(app, info + 8, volume.sectorsPerCluster);
    put(app, info + 0xc, volume.bytesPerSector);
    return 0;
}

nfs::u32 getdiskfree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (const auto result = getdiskfreeOf(app, ts, args.eax, args.edx))
        returns(ts, args, *result);
    return args.eax;
}

/* __fixed_GetFileAttributesA (0x509640): GetFileAttributesA by way of
 * FindFirstFileA, which wildcards would fool: -1 for them. */
nfs::u32 fixed_GetFileAttributesA(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    const nfs::u32 attributes = fileAttributes(app, args.eax);
    kept.back(args);
    returns(ts, args, attributes);
    return args.eax;
}

/* ---- The runtime's handles (clib/handleio/c): a handle is an index into
 * __OSHandles, the port's handle for it, and __io_mode, its mode; each taken
 * and given back under the file's lock, _AccessFileH and _ReleaseFileH,
 * __NullAccessRtn while there is one thread.  A handle up to __NFiles is
 * good -- __NFiles too, the runtime's own slip, kept. ---- */

namespace
{

constexpr nfs::u32 kOsHandles = 0x56acbc;  // __OSHandles: where the port's handles are
constexpr nfs::u32 kFileCount = 0x56ad94;  // __NFiles
constexpr nfs::u32 kIoModes = 0x56ade8;    // __io_mode: where the modes are
constexpr nfs::u32 kAccessFile = 0x567768, kReleaseFile = 0x56776c;  // the file's lock, eax the handle
/* The default windowing's hooks: none in the game. */
constexpr nfs::u32 kIsWindowedHandle = 0x5677b0, kWindowsStdin = 0x5677d8, kWindowsStdout = 0x5677dc;
constexpr nfs::u32 kEBADF = 4, kENOSPC = 12;
/* A mode's bits. */
constexpr nfs::u32 kAppend = 0x80, kIsTty = 0x2000, kInitialized = 0x4000, kExtended = 0x8000;

bool goodHandle(nfs::App* app, nfs::u32 handle)
{
    return nfs::s32(handle) >= 0 && handle <= at(app, kFileCount);
}

nfs::u32 osHandle(nfs::App* app, nfs::u32 handle)
{
    return at(app, at(app, kOsHandles) + 4 * handle);
}

/* The file's lock taken (kAccessFile) or given back (kReleaseFile), by its
 * hook (watcom_hooks.h). */
bool fileLock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 slot, nfs::u32 handle)
{
    return (slot == kAccessFile ? hook<kAccessFile>(app, ts, {handle}) : hook<kReleaseFile>(app, ts, {handle}))
        .has_value();
}

/* The default windowing's window for the handle, if it has one and `hook`
 * is set: _WindowsIsWindowedHandle's. */
bool windowOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 windowing, nfs::u32 handle,
              nfs::u32& window)
{
    window = 0;
    if (at(app, windowing) == 0)
        return true;
    const auto found = through(app, ts, at(app, kIsWindowedHandle), {handle});
    if (!found)
        return false;
    window = *found;
    return true;
}

/* isatty's: 1 for a window's handle or a character device's, under the lock. */
bool isTty(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& result)
{
    nfs::u32 window;
    if (!fileLock(app, ts, kAccessFile, handle) || !windowOf(app, ts, kIsWindowedHandle, handle, window))
        return false;
    // A window's is one; the port's handles are files, never character devices.
    result = window != 0 ? 1 : 0;
    return fileLock(app, ts, kReleaseFile, handle);
}

/* __GetIOMode's: 0 past __NFiles; for the first three, isatty asked the
 * first time and the answer kept in the mode. */
bool ioMode(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& mode)
{
    mode = 0;
    if (handle >= at(app, kFileCount))
        return true;
    if (nfs::s32(handle) < 3 && !(byte(app, at(app, kIoModes) + 4 * handle + 1) & (kInitialized >> 8)))
    {
        const nfs::u32 flags = at(app, kIoModes) + 4 * handle + 1;
        putByte(app, flags, byte(app, flags) | (kInitialized >> 8));
        nfs::u32 tty;
        if (!isTty(app, ts, handle, tty))
            return false;
        if (tty != 0)
        {
            const nfs::u32 again = at(app, kIoModes) + 4 * handle + 1;
            putByte(app, again, byte(app, again) | (kIsTty >> 8));
        }
    }
    mode = at(app, at(app, kIoModes) + 4 * handle);
    return true;
}

/* __SetIOMode's: a mode set, initialized unless 0. */
void setIoMode(nfs::App* app, nfs::u32 handle, nfs::u32 mode)
{
    put(app, at(app, kIoModes) + 4 * handle, mode != 0 ? mode | kInitialized : 0);
}

/* What a handle's function makes of a bad one: errno EBADF, -1. */
bool badHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32& result)
{
    result = 0xffffffff;
    return setThreadWord(app, ts, kErrno, kEBADF);
}

}  // namespace

/* __GetIOMode (clib/handleio/c/iomode.c, 0x522de0): eax's mode. */
nfs::u32 GetIOMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 mode;
    if (!ioMode(app, ts, args.eax, mode))
        return args.eax;
    kept.back(args);
    returns(ts, args, mode);
    return args.eax;
}

/* __SetIOMode (clib/handleio/c/iomode.c, 0x522e38): eax's mode edx; eax
 * left 4 times the handle, as its code leaves it. */
nfs::u32 SetIOMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    setIoMode(app, args.eax, args.edx);
    returns(ts, args, args.eax << 2);
    return args.eax;
}

/* isatty (clib/environ/c/isattwnt.c, 0x523560): whether eax is a
 * character device's, or a window's. */
nfs::u32 isatty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!isTty(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

namespace
{

/* Where a seek counts from, by the runtime's SEEK_SET, SEEK_CUR and SEEK_END; another is taken
 * from the start. */
os::files::From seekFrom(nfs::u32 whence)
{
    return whence == 1 ? os::files::From::current : whence == 2 ? os::files::From::end : os::files::From::start;
}

/* lseek's: `handle` moved `offset` bytes from where `whence` says; a move forward marks the mode
 * extended unless it is appending.  The new place, or -1. */
bool lseekOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32 offset, nfs::u32 whence,
             nfs::u32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    nfs::u32 mode;
    if (!fileLock(app, ts, kAccessFile, handle) || !ioMode(app, ts, handle, mode))
        return false;
    if (nfs::s32(offset) > 0 && !(mode & kAppend))
        setIoMode(app, handle, mode | kExtended);
    result = os::files::seek(app, osHandle(app, handle), nfs::s32(offset), seekFrom(whence)).value_or(0xffffffff);
    return fileLock(app, ts, kReleaseFile, handle) && (result != 0xffffffff || setErrnoNt(app, ts));
}

/* _tell's: `handle`'s place, or -1. */
bool tellOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    const nfs::u32 os = osHandle(app, handle);  // before the lock, as its code reads it
    if (!fileLock(app, ts, kAccessFile, handle))
        return false;
    result = os::files::seek(app, os, 0, os::files::From::current).value_or(0xffffffff);
    return fileLock(app, ts, kReleaseFile, handle) && (result != 0xffffffff || setErrnoNt(app, ts));
}

}  // namespace

/* lseek (clib/handleio/c/lseek.c, 0x5028e0): eax moved edx bytes from where
 * ebx says; a move forward marks the mode extended unless it is
 * appending.  The new place, or -1. */
nfs::u32 lseek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!lseekOf(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* _tell (clib/handleio/c/tell.c, 0x502960): eax's place, or -1. */
nfs::u32 tell(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!tellOf(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

namespace
{

/* __qread's and __qwrite's way to a window's: the hook on it, eax the window,
 * edx the buffer, ebx the length; how many it took. */
bool windowTransfer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 windowing, nfs::u32 handle,
                    nfs::u32 window, nfs::u32 buffer, nfs::u32 length, nfs::u32& result)
{
    const auto took = through(app, ts, at(app, windowing), {window, buffer, length});
    if (!took)
        return false;
    result = *took;
    return fileLock(app, ts, kReleaseFile, handle);
}

bool readHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32 buffer, nfs::u32 length,
                nfs::u32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    nfs::u32 window;
    if (!fileLock(app, ts, kAccessFile, handle) || !windowOf(app, ts, kWindowsStdin, handle, window))
        return false;
    if (window != 0)
        return windowTransfer(app, ts, kWindowsStdin, handle, window, buffer, length, result);
    nfs::u32 got = 0;
    const bool done = os::files::read(app, ts, osHandle(app, handle), guest::memory(app, buffer), length, got);
    result = done ? got : 0xffffffff;
    return fileLock(app, ts, kReleaseFile, handle) && (done || setErrnoNt(app, ts));
}

bool writeHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32 buffer, nfs::u32 length,
                 nfs::u32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    const nfs::u32 os = osHandle(app, handle);  // before the lock, as its code reads it
    nfs::u32 mode, window;
    if (!fileLock(app, ts, kAccessFile, handle) || !ioMode(app, ts, handle, mode))
        return false;
    if (mode & kAppend)
    {
        if (!os::files::seek(app, os, 0, os::files::From::end))
        {
            result = 0xffffffff;
            return fileLock(app, ts, kReleaseFile, handle) && setErrnoNt(app, ts);
        }
    }
    if (!windowOf(app, ts, kWindowsStdout, handle, window))
        return false;
    if (window != 0)
        return windowTransfer(app, ts, kWindowsStdout, handle, window, buffer, length, result);
    nfs::u32 wrote = 0;
    if (!os::files::write(app, ts, os, guest::memory(app, buffer), length, wrote))
    {
        result = 0xffffffff;
        return fileLock(app, ts, kReleaseFile, handle) && setErrnoNt(app, ts);
    }
    result = wrote;
    return (result == length || setThreadWord(app, ts, kErrno, kENOSPC))
           && fileLock(app, ts, kReleaseFile, handle);
}

/* __qread's and __qwrite's, at the depth of their calls: their frames under the return address --
 * ecx and esi, or ecx, esi, edi and ebp (their room only). */
bool qreadOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32 buffer, nfs::u32 length,
             nfs::u32& result)
{
    const GuestFrame frame(ts, 0xc, 0);
    return readHandle(app, ts, handle, buffer, length, result);
}

bool qwriteOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32 buffer, nfs::u32 length,
              nfs::u32& result)
{
    const GuestFrame frame(ts, 0x14, 0);
    return writeHandle(app, ts, handle, buffer, length, result);
}

}  // namespace

/* __qread (clib/handleio/c/qread.c, 0x502a50): ebx bytes from eax into edx --
 * a window's by _WindowsStdin, else by ReadFile; how many, or -1. */
nfs::u32 qread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!qreadOf(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __qwrite (clib/handleio/c/qwrit.c, 0x5096c0): ebx bytes from edx to eax --
 * at the end, if it is appending; a window's by _WindowsStdout, else by
 * WriteFile, errno ENOSPC for fewer.  How many, or -1. */
nfs::u32 qwrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!qwriteOf(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* ---- Time (clib/time/c): a struct tm in guest memory is nine ints -- sec,
 * min, hour, mday, mon, year since 1900, wday, yday, isdst.  The time zone
 * is the runtime's, __tzset's: still the game's code, called where the
 * runtime calls it.  The structs these keep on their stacks are where their
 * frames have them. ---- */

namespace
{

constexpr nfs::u32 kTzset = 0x515c2c;        // __tzset: the zone from TZ, or from Windows
constexpr nfs::u32 kTimezone = 0x568b94;     // timezone: seconds west of UTC
constexpr nfs::u32 kDstAdjust = 0x568b9c;    // __dst_adjust: seconds daylight saving takes off
constexpr nfs::u32 kDstName = 0x568b90;      // tzname[1]: no daylight saving while it is ""
constexpr nfs::u32 kDstStart = 0x568a40, kDstEnd = 0x568a64;  // its rules, a struct tm each
constexpr nfs::u32 kDaysInYear = 0x552148, kDaysInLeapYear = 0x552162;  // __diyr, __dilyr: shorts
constexpr nfs::u32 kDaysTo1970 = 0x63df;     // from 1 January 1900
constexpr nfs::u32 kDay = 86400;
enum TmField : nfs::u32
{
    kSec = 0, kMin = 4, kHour = 8, kMday = 0xc, kMon = 0x10, kYear = 0x14, kWday = 0x18, kYday = 0x1c,
    kIsdst = 0x20,
};

nfs::s32 field(nfs::App* app, nfs::u32 tm, TmField f)
{
    return nfs::s32(at(app, tm + f));
}

/* __diyr's or __dilyr's entry, a short; the index as the runtime's code
 * scales it. */
nfs::s32 daysBefore(nfs::App* app, nfs::u32 table, nfs::u32 month)
{
    return nfs::s16(app->getMemory<nfs::u16>(table + 2 * month));
}

/* __leapyear (clib/time/c/leapyear.c). */
bool leapYear(nfs::u32 year)
{
    if (year & 3)
        return false;
    return year % 100 != 0 || year % 400 == 0;
}

nfs::u32 yearTable(nfs::u32 year)
{
    return leapYear(year + 1900) ? kDaysInLeapYear : kDaysInYear;
}

/* __brktime (clib/time/c/gmtime.c): `seconds` seconds `zone` west into day
 * `days` broken down into the struct tm, all but isdst.  Its years are
 * found by 366 a year, then a year's length at a time -- the first year's
 * length each time, as its code does. */
nfs::u32 breakTime(nfs::App* app, nfs::u32 days, nfs::u32 seconds, nfs::u32 zone, nfs::u32 tm)
{
    if (seconds < 0xa8c0 && nfs::s32(zone) > 0)
    {
        seconds = seconds + kDay - zone;
        days += seconds / kDay - 1;
    }
    else
    {
        seconds -= zone;
        days += seconds / kDay;
    }
    nfs::u32 rest = seconds % kDay;
    put(app, tm + kHour, rest / 3600);
    rest %= 3600;
    put(app, tm + kMin, rest / 60);
    put(app, tm + kSec, rest % 60);
    nfs::u32 year = days / 366;
    nfs::u32 day = days - 365 * year;
    if (year != 0)
        day -= (year - 1) >> 2;
    const nfs::u32 length = (leapYear(year + 1900) ? 1 : 0) + 365;
    while (day >= length)
    {
        day -= length;
        ++year;
    }
    put(app, tm + kYear, year);
    put(app, tm + kYday, day);
    const nfs::u32 table = yearTable(year);
    nfs::u32 month = day / 31;
    if (day >= nfs::u32(daysBefore(app, table, month + 1)))
        ++month;
    put(app, tm + kMon, month);
    put(app, tm + kMday, day - daysBefore(app, table, month) + 1);
    put(app, tm + kWday, (days + 1) % 7);
    return tm;
}

/* time_less (clib/time/c/timeutil.c): whether a's time of day is before b's. */
bool timeLess(nfs::App* app, nfs::u32 a, nfs::u32 b)
{
    if (field(app, a, kHour) != field(app, b, kHour))
        return field(app, a, kHour) < field(app, b, kHour);
    if (field(app, a, kMin) != field(app, b, kMin))
        return field(app, a, kMin) < field(app, b, kMin);
    return field(app, a, kSec) < field(app, b, kSec);
}

/* __tzset (0x515c2c) called as the code calls it: the return address's slot under esp, esp back after.  The
 * registers it keeps (it pushes them and pops them back: their room and a dead frame's words) are not
 * given: zeros. */
bool tzsetCalled(nfs::App* app, nfs::ThreadState& ts)
{
    const nfs::u32 esp = ts.sp;
    ts.sp = esp - 4;
    nfs::Args registers;
    tzset(app, ts, registers);
    if (ts.terminate)
        return false;
    ts.sp = esp;
    return true;
}

bool makeTime(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 tm, nfs::u32& result);

/* calc_yday (clib/time/c/timeutil.c): the day of `year` a daylight saving
 * rule falls on -- Jn, n or Mm.w.d, as its isdst says; mktime asked for the
 * weekday the month begins on, with a struct tm of its frame. */
bool ruleDay(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 rule, nfs::u32 year,
             nfs::s32& result)
{
    const nfs::s32 kind = field(app, rule, kIsdst);
    if (kind != 0)
    {
        result = nfs::s32(at(app, rule + kYday) - (kind == 1 ? 1 : 0));
        return true;
    }
    const nfs::u32 month = at(app, rule + kMon);
    const nfs::u32 table = yearTable(year);
    const nfs::s32 monthDays = daysBefore(app, table, month + 1) - daysBefore(app, table, month);
    const nfs::u32 first = entry - 0x38;  // ebx, ecx, esi, edi, ebp; the struct tm
    put(app, first + kSec, 0);
    put(app, first + kMin, 0);
    put(app, first + kHour, 0);
    put(app, first + kMday, 1);
    put(app, first + kMon, month);
    put(app, first + kYear, year);
    put(app, first + kIsdst, 0);
    nfs::u32 ignored;
    if (!makeTime(app, ts, first - 4, first, ignored))
        return false;
    // The rule's fields are anything: its sums wrap as the x86's do.
    const nfs::u32 firstWday = nfs::u32(nfs::s32(at(app, rule + kWday) - at(app, first + kWday) + 7) % 7);
    const nfs::u32 week = at(app, rule + kMday);
    nfs::u32 nth = week - 1;
    if (week == 5 && nfs::s32(firstWday + 1 + 28) > monthDays)
        nth = week - 2;
    result = nfs::s32(7 * nth + at(app, first + kYday) + firstWday);
    return true;
}

/* check_order (clib/time/c/timeutil.c): whether daylight saving starts after
 * it ends in `year` -- the southern hemisphere. */
bool rulesReversed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 start, nfs::u32 end,
                   nfs::u32 year, bool& result)
{
    if (field(app, start, kIsdst) == 0 && field(app, end, kIsdst) == 0
        && field(app, start, kMon) != field(app, end, kMon))
    {
        result = field(app, start, kMon) > field(app, end, kMon);
        return true;
    }
    const nfs::u32 esp = entry - 0xc;  // ecx, esi, edi
    nfs::s32 startDay, endDay;
    if (!ruleDay(app, ts, esp - 4, start, year, startDay) || !ruleDay(app, ts, esp - 4, end, year, endDay))
        return false;
    result = startDay > endDay;
    return true;
}

/* __isindst's test of one rule (clib/time/c/timeutil.c): whether the day is
 * past the rule's day, and whether it is that day, its time to be compared. */
void ruleDayPassed(nfs::App* app, nfs::u32 tm, nfs::u32 rule, nfs::s32 monthDays, bool start,
                   bool& dst, bool& timeCheck)
{
    timeCheck = false;
    if (field(app, rule, kIsdst) == 0)
    {
        const nfs::s32 month = field(app, tm, kMon), ruleMonth = field(app, rule, kMon);
        if (month > ruleMonth)
        {
            dst = start;
            return;
        }
        if (month != ruleMonth)
            return;
        if (!start)
            dst = false;
        // The rule's fields are anything: its sums wrap as the x86's do.
        const nfs::u32 mday = at(app, tm + kMday), wday = at(app, tm + kWday), ruleWday = at(app, rule + kWday);
        const nfs::s32 n1 = nfs::s32(mday - nfs::u32(nfs::s32(wday + 7 - ruleWday) % 7));
        const nfs::s32 n2 = nfs::s32(mday - 1 - nfs::u32(nfs::s32(wday + 6 - ruleWday) % 7));
        const nfs::u32 week = at(app, rule + kMday);
        const bool lastWeek = week == 5;
        const nfs::s32 limit = lastWeek ? monthDays - 7 : nfs::s32(7 * (week - 1) + 1);
        const bool n1Past = lastWeek ? n1 > limit : n1 >= limit;
        const bool n2Past = lastWeek ? n2 > limit : n2 >= limit;
        if (start)
        {
            if (n1Past)
            {
                dst = true;
                timeCheck = !n2Past;
            }
        }
        else if (!n1Past)
        {
            dst = true;
        }
        else
        {
            timeCheck = !n2Past;
        }
        return;
    }
    nfs::u32 n1 = at(app, rule + kYday);
    if (field(app, rule, kIsdst) == 1)
    {
        if (leapYear(at(app, tm + kYear) + 1900) && nfs::s32(n1) > daysBefore(app, kDaysInYear, 2))
            ++n1;
        --n1;
    }
    if (field(app, tm, kYday) >= nfs::s32(n1))
    {
        dst = start;
        timeCheck = at(app, tm + kYday) == n1;
    }
}

/* __isindst (clib/time/c/timeutil.c): whether the struct tm is in daylight
 * saving, by the zone's rules, into its isdst; none while tzname[1] is "". */
bool inDst(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 tm, nfs::u32& result)
{
    bool dst = false;
    if (byte(app, at(app, kDstName)) != 0)
    {
        bool south;
        if (!rulesReversed(app, ts, entry - 0x34 - 4, kDstStart, kDstEnd, at(app, tm + kYear), south))
            return false;  // ebx, ecx, edx, esi, edi, ebp, and 0x1c of its own
        const nfs::u32 start = south ? kDstEnd : kDstStart, end = south ? kDstStart : kDstEnd;
        const nfs::u32 table = yearTable(at(app, tm + kYear));
        const nfs::u32 month = at(app, tm + kMon);
        const nfs::s32 monthDays = daysBefore(app, table, month + 1) - daysBefore(app, table, month);
        bool timeCheck;
        ruleDayPassed(app, tm, start, monthDays, true, dst, timeCheck);
        if (timeCheck)
            dst = !timeLess(app, tm, start);
        if (dst)
        {
            ruleDayPassed(app, tm, end, monthDays, false, dst, timeCheck);
            if (timeCheck)
                dst = timeLess(app, tm, end);
        }
        if (south)
            dst = !dst;
    }
    result = dst ? 1 : 0;
    put(app, tm + kIsdst, result);
    return true;
}

/* mktime (clib/time/c/mktime.c): the struct tm made a time_t, and put right
 * -- months past 11, seconds past 59 and the like -- by __brktime; isdst
 * found by __isindst if it is negative; -1 before 1970, or for a year too
 * far back to count. */
bool makeTime(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 tm, nfs::u32& result)
{
    // Its sums wrap as the x86's do: the fields are anything.
    const nfs::u32 esp = entry - 0x1c;  // ebx, ecx, edx, esi, edi, ebp, and a dword
    result = 0xffffffff;
    nfs::s32 month = field(app, tm, kMon) % 12;
    if (field(app, tm, kYear) < nfs::s32(0xf4fb7ea1))
        return true;
    nfs::u32 year = at(app, tm + kYear) + nfs::u32(field(app, tm, kMon) / 12);
    while (month < 0)
    {
        month += 12;
        --year;
    }
    if (nfs::s32(year) < 0)
        return true;
    const nfs::u32 table = yearTable(year);
    nfs::u32 days = nfs::u32(nfs::s32(year + 3) / 4) + 365 * year
                      - nfs::u32(nfs::s32(year + 0x63) / 100) + nfs::u32(nfs::s32(year + 0x12b) / 400);
    days = nfs::u32(daysBefore(app, table, month)) + days + at(app, tm + kMday) - 1;
    nfs::u32 seconds = at(app, tm + kSec) + 60 * (at(app, tm + kMin) + 60 * at(app, tm + kHour));
    while (nfs::s32(seconds) < 0)
    {
        seconds += kDay;
        --days;
    }
    breakTime(app, days, seconds, 0, tm);
    ts.sp = esp;
    if (!tzsetCalled(app, ts))
        return false;
    seconds += at(app, kTimezone);
    if (field(app, tm, kIsdst) < 0)
    {
        nfs::u32 ignored;
        if (!inDst(app, ts, esp - 4, tm, ignored))
            return false;
    }
    if (field(app, tm, kIsdst) > 0)
        seconds -= at(app, kDstAdjust);
    while (nfs::s32(seconds) < 0)
    {
        --days;
        seconds += kDay;
    }
    if (nfs::s32(days) < nfs::s32(kDaysTo1970 - 1))
        return true;
    if (days == kDaysTo1970 - 1)
    {
        seconds -= kDay;
        if (nfs::s32(at(app, kTimezone)) > 0 && nfs::s32(seconds) >= 0)
            result = seconds;
        return true;
    }
    result = seconds + (days - kDaysTo1970) * kDay;
    return true;
}

/* __getctime (clib/time/c/gtctmwnt.c): the local time (os::time) into the struct tm, isdst -1;
 * its milliseconds.  esp below the code's pushes (ebx, ecx, edx) and its locals, as it is after. */
bool currentTime(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 tm, nfs::u32& result)
{
    ts.sp = entry - 0x1c;
    const os::time::Date date = os::time::now();
    put(app, tm + kYear, date.year - 1900);
    put(app, tm + kMon, date.month - 1);
    put(app, tm + kMday, date.day);
    put(app, tm + kHour, date.hour);
    put(app, tm + kMin, date.minute);
    put(app, tm + kIsdst, 0xffffffff);
    put(app, tm + kSec, date.second);
    result = date.millisecond;
    return true;
}

/* _localtime's (clib/time/c/localtim.c): *timer broken down in the zone,
 * again with daylight saving's hour if it is in it. */
bool localTime(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry, nfs::u32 timer, nfs::u32 tm)
{
    ts.sp = entry - 0x10;  // ebx, ecx, esi, edi
    if (!tzsetCalled(app, ts))
        return false;
    const nfs::u32 seconds = at(app, timer);
    breakTime(app, kDaysTo1970, seconds, at(app, kTimezone), tm);
    nfs::u32 dst;
    if (!inDst(app, ts, entry - 0x14, tm, dst))
        return false;
    if (dst != 0)
        breakTime(app, kDaysTo1970, seconds, at(app, kTimezone) - at(app, kDstAdjust), tm);
    return true;
}

}  // namespace

/* __leapyear (clib/time/c/leapyear.c, 0x5160f0): whether year eax is a leap
 * year. */
nfs::u32 leapyear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    returns(ts, args, leapYear(args.eax) ? 1 : 0);
    return args.eax;
}

/* __brktime (clib/time/c/gmtime.c, 0x515fa0): edx seconds ebx west into day
 * eax since 1900 broken down into the struct tm at ecx; that. */
nfs::u32 brktime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, breakTime(app, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* time_less (clib/time/c/timeutil.c, 0x51652c): whether the struct tm at
 * eax's time of day is before edx's. */
nfs::u32 time_less(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, timeLess(app, args.eax, args.edx) ? 1 : 0);
    return args.eax;
}

/* calc_yday (clib/time/c/timeutil.c, 0x51612c): the day of year edx the rule
 * at eax falls on. */
nfs::u32 calc_yday(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::s32 day;
    if (!ruleDay(app, ts, entry, args.eax, args.edx, day))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, day);
    return args.eax;
}

/* check_order (clib/time/c/timeutil.c, 0x516224): whether the rule at eax
 * falls after edx's in year ebx. */
nfs::u32 check_order(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    bool reversed;
    if (!rulesReversed(app, ts, entry, args.eax, args.edx, args.ebx, reversed))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, reversed ? 1 : 0);
    return args.eax;
}

/* __isindst (clib/time/c/timeutil.c, 0x516270): the struct tm at eax's isdst
 * by the zone's rules; that. */
nfs::u32 isindst(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::u32 dst;
    if (!inDst(app, ts, entry, args.eax, dst))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, dst);
    return args.eax;
}

/* mktime (clib/time/c/mktime.c, 0x50eba0): the struct tm at eax made a
 * time_t, and put right. */
nfs::u32 mktime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::u32 result;
    if (!makeTime(app, ts, entry, args.eax, result))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, result);
    return args.eax;
}

/* __getctime (clib/time/c/gtctmwnt.c, 0x50eb30): the local time into the
 * struct tm at eax; its milliseconds. */
nfs::u32 getctime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    nfs::u32 milliseconds;
    if (!currentTime(app, ts, entry, args.eax, milliseconds))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, milliseconds);
    return args.eax;
}

/* time (clib/time/c/time.c, 0x4f22b0): now, a second on from half of one,
 * as a time_t -- into eax's too, if there is one. */
std::optional<nfs::u32> timeOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 timer)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 tm = entry - 0x2c;  // ebx, edx; the struct tm
    nfs::u32 milliseconds, result;
    if (!currentTime(app, ts, tm - 4, tm, milliseconds))
        return std::nullopt;
    if (nfs::s32(milliseconds) >= 500)
        put(app, tm + kSec, at(app, tm + kSec) + 1);
    if (!makeTime(app, ts, tm - 4, tm, result))
        return std::nullopt;
    if (timer != 0)
        put(app, timer, result);
    ts.sp = entry;
    return result;
}

nfs::u32 time(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (const auto result = timeOf(app, ts, args.eax))
        returns(ts, args, *result);
    return args.eax;
}

/* _localtime (clib/time/c/localtim.c, 0x4f8040): *eax broken down into the
 * struct tm at edx; that. */
nfs::u32 localtime_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    const nfs::u32 tm = args.edx;
    if (!localTime(app, ts, entry, args.eax, tm))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, tm);
    return args.eax;
}

/* localtime (clib/time/c/localtim.c, 0x4f8094): *eax broken down into the
 * thread's struct tm; that. */
nfs::u32 localtime(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const Kept kept(args);
    const nfs::u32 timer = args.eax;
    nfs::u32 data;
    ts.sp = entry - 8;  // ebx, edx
    if (!threadData(app, ts, data))
        return args.eax;
    const nfs::u32 tm = data + 0x14;
    if (!localTime(app, ts, entry - 0xc, timer, tm))
        return args.eax;
    kept.back(args);
    ts.sp = entry;
    returns(ts, args, tm);
    return args.eax;
}

/* ---- The x87's maths (mathlib): each of its instructions the same
 * operation of the port's x87 as the game's code has for it, in the same
 * order -- its constants too, fldpi's and fldl2e's doubles -- so that the
 * results are its to the bit in either FPU, and the register stack, the
 * control word and the status word are left as its code leaves them; eax
 * too, where fnstsw and sahf leave a byte of the status word in it.  A math
 * error goes to __math87_err, and on to __reporterror, still the game's
 * code: its struct exception, _matherr, errno.  On a Pentium with the FDIV
 * bug IF@DATAN2 went to __fpatan_wrap, the game's code
 * too. ---- */

namespace
{

constexpr nfs::u32 kReportError = 0x5243a7;  // __reporterror
constexpr nfs::u32 k8087 = 0x5644c0;         // _8087: whether there is an x87
constexpr nfs::u32 kMinusOne = 0x54c970;     // floor's -1.0
constexpr nfs::u32 kExpMax = 0x56716c, kExpMin = 0x567174;  // exp's limits, doubles
constexpr nfs::u32 kHalfPi = 0x56719c;       // IF@DACOS's pi/2, 80 bits
constexpr nfs::u32 kEDOM = 13;
constexpr double kLog2e = 1.4426950408889634;  // fldl2e, as the port loads it
constexpr double kPi = 3.1415926535897932;     // fldpi
/* sahf's carry and zero: C0 and C3. */
constexpr nfs::u16 kC0 = 0x100, kC3 = 0x4000;

nfs::Float loadDouble(nfs::App* app, nfs::u32 address)
{
    return nfs::Float(app->getMemory<double>(address));
}

void storeDouble(nfs::App* app, nfs::u32 address, const nfs::Float& value)
{
    app->getMemory<double>(address) = double(value);
}

/* The x87's arithmetic as the control word `control` has it (nfs::FloatEnv's, include/fpu.h): at 53 or
 * 64 bits a double's, at 24 rounded to a float's significand in the word's mode -- fp::'s, to
 * nearest.  The maths below run under their caller's word, which their thunks hand down: the
 * races' single precision in the game. */
struct Arithmetic
{
    nfs::u16 control;

    unsigned rc() const { return control >> 10 & 3; }

    double rounded(double value, nfs::FloatEnv::Rounded op, double a, double b) const
    {
        if ((control >> 8 & 3) != 0)
            return value;
        if (rc() == 0)
            return fp::single(value, op, a, b);
        std::uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        if ((bits << 1) == 0)
            return value;  // either zero
        return nfs::FloatEnv::roundSingleSlow(value, op, a, b, rc());
    }
    double add(double a, double b) const { return rounded(a + b, nfs::FloatEnv::Rounded::Add, a, b); }
    double sub(double a, double b) const { return rounded(a - b, nfs::FloatEnv::Rounded::Sub, a, b); }
    double mul(double a, double b) const { return rounded(a * b, nfs::FloatEnv::Rounded::Mul, a, b); }
    double div(double a, double b) const { return rounded(a / b, nfs::FloatEnv::Rounded::Div, a, b); }
    double sqrt(double a) const { return rounded(std::sqrt(a), nfs::FloatEnv::Rounded::Sqrt, a, 0.0); }
    // frndint, in the word's mode.
    double rndint(double a) const
    {
        switch (rc())
        {
        case 0:
            return std::nearbyint(a);
        case 1:
            return std::floor(a);
        case 2:
            return std::ceil(a);
        default:
            return std::trunc(a);
        }
    }
};

/* The status word `status` after fcom of a and b: its C3, C2 and C0 that comparison's, the rest as
 * it was. */
nfs::u16 compared(nfs::u16 status, double a, double b)
{
    return nfs::u16((status & ~0x4500) | fp::compare(a, b));
}

/* modf's: the double at `argument` into its integer part, chopped, at `whole`; its fraction given
 * back. */
nfs::Float splitOf(nfs::App* app, const Arithmetic& x87, nfs::u32 argument, nfs::u32 whole)
{
    const nfs::Float value = loadDouble(app, argument);
    const nfs::Float chopped = clib::chop(app, value);
    const nfs::Float fraction = x87.sub(value, chopped);
    storeDouble(app, whole, chopped);
    return fraction;
}

/* floor's: the double at `argument` rounded down, by modf, its integer part at `whole` -- one less
 * for a fraction below 0 -- given back; `status` the status word after its fcom. */
nfs::Float floorOf(nfs::App* app, const Arithmetic& x87, nfs::u32 argument, nfs::u32 whole,
                   nfs::u16& status)
{
    const nfs::Float fraction = splitOf(app, x87, argument, whole);
    status = compared(status, 0.0, fraction);
    if (!(status & (kC0 | kC3)))
        storeDouble(app, whole, x87.add(loadDouble(app, whole), loadDouble(app, kMinusOne)));
    return loadDouble(app, whole);
}

/* IF@DATAN2's: fpatan's angle of st(0) `x` over st(1) `y` -- atan2 of x by y, after its fxch. */
nfs::Float angleOf(const nfs::Float& x, const nfs::Float& y)
{
    return fp::atan(y, x);
}

/* __math87_err's, for a call whose return address is at `entry`: the double at entry + 4 and the
 * function at entry + 0xc, its byte, made __reporterror's flags -- or, for exp's underflow, 0 -- and
 * what that gives back given back, by way of a double in its frame (under the pushes of ebp, eax,
 * ebx, ecx and edx: their room only).  `status` the status word after its fcoms; nullopt when the
 * game stopped. */
std::optional<nfs::Float> mathErrorOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry,
                                      nfs::u16& status)
{
    const nfs::u32 argument = entry + 4;
    const nfs::u32 result = entry - 0x1c;
    const nfs::u8 function = byte(app, entry + 0xc);
    nfs::u32 flags = 0;
    bool underflow = false;
    if (function >= 1 && function <= 3)
    {
        flags = 0x2000 | (function | 0x40);
    }
    else
    {
        if (function == 4)
        {
            status = compared(status, 0.0, loadDouble(app, argument));
            underflow = !(status & (kC0 | kC3));
        }
        if (!underflow)
        {
            flags = 0x8100 | function;
            if (function == 6)
            {
                status = compared(status, 0.0, loadDouble(app, argument));
                if (!(status & (kC0 | kC3)))
                    flags = 0x1100 | function;
            }
        }
    }
    if (underflow)
    {
        put(app, result, 0);
        put(app, result + 4, 0);
    }
    else
    {
        const nfs::u32 esp = ts.sp;
        ts.sp = result;
        const double retval = guest::direct<kReportError, clib::reporterror>(app, ts, flags, argument);
        ts.sp = esp;
        if (ts.terminate)
            return std::nullopt;
        storeDouble(app, result, retval);
    }
    return loadDouble(app, result);
}

/* __@DSQRT's, for a call whose return address is at `entry`: the root of `x` (st(0)) under `x87`.
 * Its code's ax comes in `eax` -- al the caller's function -- and is given back as it leaves it: for
 * a root al 0 and ah the status word's high byte after its fcom; for a negative `x`, or none,
 * __math87_err's result instead -- `x` taken off, and `below` (st(1)) too unless al is sqrt's 3,
 * `popped` how many -- al 1 and the rest of eax the frame's word that held ax.  Its frame: ebp and
 * edx pushed (their room only), ax's word, the argument.  `status` the status word after; nullopt
 * when the game stopped. */
std::optional<nfs::Float> squareRootOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry,
                                       const Arithmetic& x87, const nfs::Float& x, const nfs::Float& below,
                                       nfs::u32& eax, nfs::u16& status, unsigned& popped)
{
    status = compared(status, x, 0.0);
    const nfs::u32 slot = entry - 0xc;  // fnstsw's word, which xchg gives ax for
    app->getMemory<nfs::u16>(slot) = nfs::u16(eax);
    popped = 0;
    if (!(status & kC0))
    {
        eax = (eax & 0xffff0000) | (status & 0xff00);
        return x87.sqrt(x);
    }
    const nfs::u32 argument = entry - 0x14;
    storeDouble(app, argument, x);
    popped = 1;
    eax = at(app, slot);
    if (nfs::u8(eax) != 3)
    {
        storeDouble(app, argument, below);
        popped = 2;
    }
    const auto result = mathErrorOf(app, ts, argument - 4, status);
    if (!result)
        return std::nullopt;
    eax = (eax & 0xffffff00) | 1;
    return result;
}

/* IF@DEXP's, for a call whose return address is at `entry`: e to `x` (st(0)) under `x87`, by fldl2e,
 * frndint, f2xm1 and fscale; past the largest double __math87_err's (its frame: ebp and edx pushed,
 * their room only, eax's word, the argument), below the smallest 0.  What the code leaves in st(0)
 * given back; `eax` the caller's, as the code leaves it: al 4 on the way, then 0, or 1 for an
 * error, ah fnstsw's last.  `status` the status word after; nullopt when the game stopped. */
std::optional<nfs::Float> exponentialOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry,
                                        const Arithmetic& x87, const nfs::Float& x, nfs::u32& eax,
                                        nfs::u16& status)
{
    eax = (eax & 0xffffff00) | 4;
    status = compared(status, x, loadDouble(app, kExpMax));
    eax = (eax & 0xffff00ff) | (status & 0xff00);
    if (!(status & (kC0 | kC3)))
    {
        const nfs::u32 argument = entry - 0x14;
        storeDouble(app, argument, x);
        put(app, entry - 0xc, eax);  // al the function, for __math87_err
        const auto result = mathErrorOf(app, ts, argument - 4, status);
        if (!result)
            return std::nullopt;
        eax = (eax & 0xffffff00) | 1;
        return result;
    }
    status = compared(status, x, loadDouble(app, kExpMin));
    eax = (eax & 0xffff00ff) | (status & 0xff00);
    nfs::Float result = 0.0;
    if (!(status & (kC0 | kC3)))
    {
        const nfs::Float scaled = x87.mul(x, kLog2e);
        const nfs::Float whole = x87.rndint(scaled);
        const nfs::Float fraction = x87.sub(scaled, whole);
        const nfs::Float power = x87.add(fp::f2xm1(fraction), 1.0);
        result = fp::scale(power, whole);
    }
    eax &= 0xffffff00;
    return result;
}

/* IF@DACOS's, for a call whose return address is at `entry`: acos of `x` (st(0)) under `x87` --
 * atan2 of x by sqrt(1 - x*x), from pi/2; 0 or pi for 1 and -1 -- what it leaves in st(0).  `eax`
 * the caller's, as the code leaves it: ax as it came (its push's word), the rest as __@DSQRT leaves
 * it.  `status` the status word after; nullopt when the game stopped. */
std::optional<nfs::Float> arcCosineOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 entry,
                                      const Arithmetic& x87, const nfs::Float& x, nfs::u32& eax,
                                      nfs::u16& status)
{
    const nfs::u32 saved = entry - 4;  // push ax: the port's code takes a dword for it
    app->getMemory<nfs::u16>(saved) = nfs::u16(eax);
    const nfs::Float rest = x87.sub(1.0, x87.mul(x, x));
    status = compared(status, rest, 0.0);
    eax = (eax & 0xffff0000) | status;
    nfs::Float result;
    if (status & kC3)
    {
        status = compared(status, rest, x);
        eax = (eax & 0xffff0000) | status;
        result = status & (kC0 | kC3) ? nfs::Float(0.0) : nfs::Float(kPi);
    }
    else
    {
        eax = (eax & 0xffffff00) | 1;
        unsigned popped;
        const auto root = squareRootOf(app, ts, saved - 4, x87, rest, x, eax, status, popped);
        if (!root)
            return std::nullopt;
        result = *root;
        if (nfs::u8(eax) == 0)
        {
            const nfs::Float halfPi = nfs::Float(app->getMemory<nfs::Float80>(kHalfPi));
            result = x87.sub(halfPi, angleOf(x, *root));
        }
    }
    eax = (eax & 0xffff0000) | app->getMemory<nfs::u16>(saved);
    return result;
}

}  // namespace

/* IF@DATAN2 (mathlib/a/atan87.asm, 0x4e06d9): atan2 of st(1) by st(0), into
 * st(0). */
nfs::u32 IF_DATAN2(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    const nfs::Float x = args.floatIn[0], y = args.floatIn[1];
    args.floatsTaken = 2;  // fxch's, the two taken off
    args.floatSet = true;
    args.floatResult = angleOf(x, y);
    ts.sp += 4;
    return args.eax;
}

/* floor (mathlib/c/floor.c, 0x4f1c30): the double on the stack rounded
 * down, onto the x87's; stdcall. */
nfs::u32 floor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    nfs::u16 status = ts.fenv.status.word;
    // ebp, eax, ebx, edx pushed; the integer part.
    const nfs::Float result = floorOf(app, {ts.fenv.control.word}, entry + 4, entry - 0x18, status);
    ts.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = result;
    ts.sp = entry + 0xc;
    return args.eax;
}

/* ceil (mathlib/c/ceil.c, 0x4f2d78): -floor(-x). */
nfs::u32 ceil(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 negated = entry - 0x10;  // ebp, eax; floor's argument
    storeDouble(app, negated, -loadDouble(app, entry + 4));
    nfs::u16 status = ts.fenv.status.word;
    const nfs::Float result = floorOf(app, {ts.fenv.control.word}, negated, negated - 4 - 0x18, status);
    ts.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = -result;
    ts.sp = entry + 0xc;
    return args.eax;
}

nfs::Float floored(nfs::App* app, nfs::ThreadState& ts, nfs::Float value)
{
    const nfs::u32 entry = ts.sp - 8;  // the double's two words under the return address's slot
    storeDouble(app, entry + 4, value);
    nfs::u16 status = ts.fenv.status.word;
    const nfs::Float result = floorOf(app, {ts.fenv.control.word}, entry + 4, entry - 0x18, status);
    ts.fenv.status.word = status;
    return result;
}

nfs::Float ceiled(nfs::App* app, nfs::ThreadState& ts, nfs::Float value)
{
    const nfs::u32 entry = ts.sp - 8;
    storeDouble(app, entry + 4, value);
    const nfs::u32 negated = entry - 0x10;  // ebp, eax; floor's argument
    storeDouble(app, negated, -value);
    nfs::u16 status = ts.fenv.status.word;
    const nfs::Float result = floorOf(app, {ts.fenv.control.word}, negated, negated - 4 - 0x18, status);
    ts.fenv.status.word = status;
    return -result;
}

/* modf (0x50ea4e): the double on the stack into its integer part, at the
 * pointer after it, and its fraction, onto the x87's; stdcall. */
nfs::u32 modf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::Float fraction = splitOf(app, {ts.fenv.control.word}, entry + 4, at(app, entry + 0xc));
    args.floatSet = true;
    args.floatResult = fraction;
    ts.sp = entry + 0x10;
    return args.eax;
}

/* _control87 (clib/fpu/c/cntrl87.c, 0x4f7c90): the control word's bits of
 * edx made eax's, if there is an x87 (_8087); the control word, or 0. */
nfs::u32 control87(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 word = 0;
    if (byte(app, k8087) != 0)
    {
        word = ts.fenv.control.word;
        const nfs::u32 mask = args.edx;
        if (mask != 0)
        {
            word = (~mask & word) | (mask & args.eax);
            ts.fenv.setControl(nfs::u16(word));
            word = (word & 0xffff0000) | ts.fenv.control.word;
        }
    }
    returns(ts, args, word & 0xffff);
    return args.eax;
}

/* IF@DEXP (0x4f80b4): e to st(0), into st(0). */
nfs::u32 IF_DEXP(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    nfs::u32 eax = args.eax;
    nfs::u16 status = ts.fenv.status.word;
    const auto result = exponentialOf(app, ts, entry, {ts.fenv.control.word}, args.floatIn[0], eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    ts.fenv.status.word = status;
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    ts.sp = entry + 4;
    return args.eax;
}

/* exp (mathlib/a/exp87.asm, 0x4f8116): e to the double on the stack, onto
 * the x87's; stdcall. */
nfs::u32 exp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    nfs::u32 eax = args.eax;
    nfs::u16 status = ts.fenv.status.word;
    const auto result =
        exponentialOf(app, ts, entry - 4, {ts.fenv.control.word}, loadDouble(app, entry + 4), eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    ts.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    ts.sp = entry + 0xc;
    return args.eax;
}

/* exp's core (0x4f8116): e to `power`, as the code leaves it on the x87's stack; 0 with ts.terminate set when the
 * game stopped.  Called through guest::direct only (its frame at the depth of the game's call). */
double clib::exponent(nfs::App* app, nfs::ThreadState& ts, double power)
{
    nfs::u32 eax = 0;  // its upper half reaches only __math87_err's record, past exp's range
    nfs::u16 status = ts.fenv.status.word;
    const auto result = exponentialOf(app, ts, ts.sp - 4, {ts.fenv.control.word}, power, eax, status);
    if (!result)
        return 0.0;
    ts.fenv.status.word = status;
    return double(*result);
}

/* IF@DACOS (mathlib/a/acos87.asm, 0x4f9482): acos of st(0) -- atan2 of
 * sqrt(1 - x*x) by x, from pi/2; 0 or pi for 1 and -1 -- into st(0). */
nfs::u32 IF_DACOS(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    nfs::u32 eax = args.eax;
    nfs::u16 status = ts.fenv.status.word;
    const auto result = arcCosineOf(app, ts, entry, {ts.fenv.control.word}, args.floatIn[0], eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    ts.fenv.status.word = status;
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    ts.sp = entry + 4;
    return args.eax;
}

/* __@DSQRT (mathlib, 0x51a1ba): st(0)'s root, al the caller's function; al
 * 0, or 1 for a math error. */
nfs::u32 DSQRT(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    nfs::u32 eax = args.eax;
    nfs::u16 status = ts.fenv.status.word;
    unsigned popped;
    const auto result = squareRootOf(app, ts, entry, {ts.fenv.control.word}, args.floatIn[0], args.floatIn[1], eax,
                                     status, popped);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    ts.fenv.status.word = status;
    // __math87_err's result pushed after what it took
    args.floatsTaken = popped != 0 ? popped : 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    ts.sp = entry + 4;
    return args.eax;
}

/* __math87_err (0x51655c): a math error -- the argument, a double, and the
 * function, a byte of the dword after it, on the stack -- reported; what
 * _matherr or the runtime gives back onto the x87's.  Takes its arguments
 * off; eax, ebx, ecx and edx as they came. */
nfs::u32 math87_err(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::u32 entry = ts.sp;
    const nfs::u32 eax = args.eax, ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as its pops leave them
    nfs::u16 status = ts.fenv.status.word;
    const auto result = mathErrorOf(app, ts, entry, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    ts.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    ts.sp = entry + 0x10;
    return args.eax;
}

/* __set_EDOM (0x502890): errno EDOM. */
nfs::u32 set_EDOM(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, ts, kErrno, kEDOM))
        return args.eax;
    const nfs::u32 data = args.eax;
    kept.back(args);
    returns(ts, args, data);
    return args.eax;
}

/* ---- Streams (clib/streamio/c): a FILE is the runtime's, in guest memory
 * -- _ptr, _cnt, _link, _flag, _handle, _bufsize, _ungotten, _tmpfchar --
 * and its __stream_link has the buffer, the orientation and the extended
 * flags; __OpenStreams lists the links.  Each function takes the file's
 * lock where its code does, the handle in eax, and goes down to the
 * handles' functions (__qread, __qwrite, lseek, _tell, isatty) and the
 * heap's where its code does, with esp where its frame has it: __qread and
 * __qwrite hand Windows a dword of their own frames.  The flags' bits each
 * changes are the ones its code changes. ---- */

namespace
{

constexpr nfs::u32 kPtr = 0, kCnt = 4, kLink = 8, kFlag = 0xc, kFileHandle = 0x10, kBufsize = 0x14;
constexpr nfs::u32 kUngotten = 0x18;
constexpr nfs::u32 kLinkNext = 0, kLinkStream = 4, kLinkBase = 8, kOrientation = 0xc, kExtFlags = 0x10;
constexpr nfs::u32 kOpenStreams = 0x9f38a0;                      // __OpenStreams: their links
constexpr nfs::u32 kAccessIob = 0x567778, kReleaseIob = 0x56777c;  // the streams' lock
constexpr nfs::u32 kGetche = 0x522e60;  // the console's getche: still the game's
/* _flag's bits. */
constexpr nfs::u32 kReading = 0x1, kWriting = 0x2, kUnget = 0x4, kBigBuf = 0x8, kAtEof = 0x10, kError = 0x20;
constexpr nfs::u32 kBinary = 0x40;  // and _APPEND a mode's kAppend
constexpr nfs::u32 kFullBuf = 0x100, kLineBuf = 0x200, kNoBuf = 0x400, kBuffering = 0x700;
constexpr nfs::u32 kDirty = 0x1000, kTty = 0x2000;
constexpr nfs::u32 kCommit = 1;  // the link's extended flags

/* A function's frame as its code makes it -- the registers it pushes, its
 * locals -- esp that far down while it calls what it calls. */
class Frame
{
public:
    Frame(nfs::ThreadState& ts, nfs::u32 size) : ts_(ts), entry_(ts.sp) { ts.sp = entry_ - size; }
    ~Frame() { ts_.sp = entry_; }
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

private:
    nfs::ThreadState& ts_;
    const nfs::u32 entry_;
};

/* One of these called from another as its code calls it: the return
 * address's slot below the caller's frame. */
template <typename Function>
bool nested(nfs::ThreadState& ts, Function function)
{
    const Frame slot(ts, 4);
    return function();
}

nfs::u32 linkOf(nfs::App* app, nfs::u32 fp)
{
    return at(app, fp + kLink);
}

nfs::u32 bufferOf(nfs::App* app, nfs::u32 fp)
{
    return at(app, linkOf(app, fp) + kLinkBase);
}

nfs::u32 flags(nfs::App* app, nfs::u32 fp)
{
    return at(app, fp + kFlag);
}

void setFlags(nfs::App* app, nfs::u32 fp, nfs::u32 set, nfs::u32 clear = 0)
{
    put(app, fp + kFlag, (flags(app, fp) & ~clear) | set);
}

bool streamLock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 slot, nfs::u32 fp)
{
    return fileLock(app, ts, slot, at(app, fp + kFileHandle));
}

/* The buffer emptied: _cnt 0, _ptr its start, read first. */
nfs::u32 emptyBuffer(nfs::App* app, nfs::u32 fp)
{
    const nfs::u32 base = bufferOf(app, fp);
    put(app, fp + kCnt, 0);
    put(app, fp + kPtr, base);
    return base;
}

/* fsync's (clib/handleio/c/fsync.c): the file flushed under the lock; 0, or
 * -1.  The port flushes no file: fsync fails. */
bool syncHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& result)
{
    const Frame frame(ts, 0x10);  // ebx, ecx, edx, esi
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    result = 0;
    if (!fileLock(app, ts, kAccessFile, handle))
        return false;
    result = 0xffffffff;
    if (!setErrnoNt(app, ts))
        return false;
    return fileLock(app, ts, kReleaseFile, handle);
}

/* __chktty's: a tty's stream marked one, and line-buffered unless it is
 * buffered some way already; `eax` as its code leaves it. */
bool checkTty(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& eax)
{
    const Frame frame(ts, 0xc);  // ebx, ecx, edx
    eax = fp;
    if (flags(app, fp) & kTty)
        return true;
    if (!called(ts, [&] { return isTty(app, ts, at(app, fp + kFileHandle), eax); }))
        return false;
    if (eax != 0)
    {
        setFlags(app, fp, kTty);
        if (!(flags(app, fp) & kBuffering))
            setFlags(app, fp, kLineBuf);
    }
    return true;
}

/* __ioalloc's: a buffer from _nmalloc -- _bufsize's, or 0x86 bytes
 * line-buffered, 1 unbuffered, 4K -- or, for none, _ungotten, unbuffered.
 * `eax` as its code leaves it: the buffer's start. */
bool allocateBuffer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& eax)
{
    const Frame frame(ts, 0xc);  // ebx, ecx, edx
    if (!nested(ts, [&] { return checkTty(app, ts, fp, eax); }))
        return false;
    if (at(app, fp + kBufsize) == 0)
    {
        const nfs::u32 f = flags(app, fp);
        put(app, fp + kBufsize, f & kLineBuf ? 0x86 : f & kNoBuf ? 1 : 0x1000);
    }
    nfs::u32 block;
    if (!called(ts, [&] { return watcomheap::allocate(app, ts, kGameHeap, at(app, fp + kBufsize), block); }))
        return false;
    put(app, linkOf(app, fp) + kLinkBase, block);
    if (bufferOf(app, fp) == 0)
    {
        setFlags(app, fp, kNoBuf, kBuffering);
        put(app, linkOf(app, fp) + kLinkBase, fp + kUngotten);
        put(app, fp + kBufsize, 1);
    }
    else
    {
        setFlags(app, fp, kBigBuf);
    }
    eax = emptyBuffer(app, fp);
    return true;
}

bool allocateBuffer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp)
{
    nfs::u32 eax;
    return allocateBuffer(app, ts, fp, eax);
}

/* __flush's: a written buffer to __qwrite, all of it -- ENOSPC for none
 * taken; a read one's unread part seeked back over, unless it is a tty's;
 * the buffer emptied, and committed if the link says so.  0, or -1. */
bool flushStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x18);  // ebx, ecx, edx, esi, edi, ebp
    result = 0;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    if (flags(app, fp) & kDirty)
    {
        setFlags(app, fp, 0, kDirty);
        nfs::u32 from = bufferOf(app, fp);
        if ((flags(app, fp) & kWriting) && from != 0)
        {
            for (nfs::u32 left = at(app, fp + kCnt); left != 0 && result == 0;)
            {
                nfs::u32 written;
                if (!called(ts, [&] { return qwriteOf(app, ts, at(app, fp + kFileHandle), from, left, written); }))
                    return false;
                if (written == 0xffffffff)
                {
                    setFlags(app, fp, kError);
                    result = written;
                }
                else if (written == 0)
                {
                    if (!setThreadWord(app, ts, kErrno, kENOSPC))
                        return false;
                    setFlags(app, fp, kError);
                    result = 0xffffffff;
                }
                from += written;
                left -= written;
            }
        }
    }
    else if (bufferOf(app, fp) != 0)
    {
        setFlags(app, fp, 0, kAtEof);
        if (!(flags(app, fp) & kTty))
        {
            nfs::u32 place = 0;
            if (at(app, fp + kCnt) != 0)
            {
                if (!called(ts, [&] {
                        return lseekOf(app, ts, at(app, fp + kFileHandle), 0 - at(app, fp + kCnt), 1, place);  // SEEK_CUR
                    }))
                    return false;
            }
            if (place == 0xffffffff)
            {
                setFlags(app, fp, kError);
                result = place;
            }
        }
    }
    emptyBuffer(app, fp);
    if (result == 0 && (at(app, linkOf(app, fp) + kExtFlags) & kCommit))
    {
        nfs::u32 synced;
        if (!nested(ts, [&] { return syncHandle(app, ts, at(app, fp + kFileHandle), synced); }))
            return false;
        if (synced == 0xffffffff)
            result = synced;
    }
    return streamLock(app, ts, kReleaseFile, fp);
}

/* fputc's: c into the buffer -- '\n' after a '\r' in text -- flushed when
 * it fills, or for a line or a character as the stream is buffered; a
 * stream of wide characters takes none, one not open for writing is EBADF.
 * The character, or -1. */
bool putChar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 c, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x14);  // ebx, ecx, esi, edi, ebp
    result = 0xffffffff;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    const nfs::u32 link = linkOf(app, fp);
    const nfs::u32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
            return streamLock(app, ts, kReleaseFile, fp);
        put(app, link + kOrientation, 1);
    }
    if (!(flags(app, fp) & kWriting))
    {
        if (!setThreadWord(app, ts, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, ts, kReleaseFile, fp);
    }
    if (bufferOf(app, fp) == 0 && !nested(ts, [&] { return allocateBuffer(app, ts, fp); }))
        return false;
    auto put1 = [&](nfs::u8 byte_) {
        setFlags(app, fp, kDirty);
        putByte(app, at(app, fp + kPtr), byte_);
        put(app, fp + kPtr, at(app, fp + kPtr) + 1);
        put(app, fp + kCnt, at(app, fp + kCnt) + 1);
    };
    nfs::u32 flushOn = kNoBuf;
    if (c == '\n')
    {
        flushOn = kNoBuf | kLineBuf;
        if (!(flags(app, fp) & kBinary))
        {
            put1('\r');
            if (at(app, fp + kCnt) == at(app, fp + kBufsize))
            {
                nfs::u32 flushed;
                if (!nested(ts, [&] { return flushStream(app, ts, fp, flushed); }))
                    return false;
                if (flushed != 0)
                    return streamLock(app, ts, kReleaseFile, fp);
            }
        }
    }
    put1(nfs::u8(c));
    if ((flags(app, fp) & flushOn) || at(app, fp + kCnt) == at(app, fp + kBufsize))
    {
        nfs::u32 flushed;
        if (!nested(ts, [&] { return flushStream(app, ts, fp, flushed); }))
            return false;
        if (flushed != 0)
            return streamLock(app, ts, kReleaseFile, fp);
    }
    result = c & 0xff;
    return streamLock(app, ts, kReleaseFile, fp);
}

/* fputs's: the string by fputc, an unbuffered stream line-buffered while
 * it goes and flushed after; how many, or -1. */
bool putString(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x14);  // ebx, ecx, esi, edi, ebp
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    if (bufferOf(app, fp) == 0 && !nested(ts, [&] { return allocateBuffer(app, ts, fp); }))
        return false;
    const bool unbuffered = flags(app, fp) & kNoBuf;
    if (unbuffered)
        setFlags(app, fp, kLineBuf, kLineBuf | kNoBuf);
    result = 0;
    nfs::u32 at_ = string;
    for (; byte(app, at_) != 0; ++at_)
    {
        nfs::u32 put_;
        if (!nested(ts, [&] { return putChar(app, ts, byte(app, at_), fp, put_); }))
            return false;
        if (put_ == 0xffffffff)
        {
            result = put_;
            break;
        }
    }
    if (unbuffered)
    {
        setFlags(app, fp, kNoBuf, kLineBuf | kNoBuf);
        if (result == 0 && !nested(ts, [&] { return flushStream(app, ts, fp, result); }))
            return false;
    }
    if (result == 0)
        result = at_ - string;
    return streamLock(app, ts, kReleaseFile, fp);
}

/* __flushall's: every open stream with a bit of `mask` counted, and flushed
 * if it is dirty, under the streams' lock -- given back with eax as the walk
 * leaves it: the mask, the last stream, or what flushing it gave. */
bool flushAll(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mask, nfs::u32& count)
{
    const Frame frame(ts, 0xc);  // ebx, ecx, edx
    if (!hook<kAccessIob>(app, ts, {mask}))
        return false;
    count = 0;
    nfs::u32 eax = mask;
    for (nfs::u32 link = at(app, kOpenStreams); link != 0; link = at(app, link + kLinkNext))
    {
        const nfs::u32 fp = eax = at(app, link + kLinkStream);
        if (!(flags(app, fp) & mask))
            continue;
        ++count;
        if ((flags(app, fp) & kDirty) && !nested(ts, [&] { return flushStream(app, ts, fp, eax); }))
            return false;
    }
    return hook<kReleaseIob>(app, ts, {eax}).has_value();
}

/* fwrite's: n of size bytes -- binary, into the buffer, or straight to
 * __qwrite in whole 512s when the buffer is empty and they fill it; text,
 * by fputc, an unbuffered stream fully buffered while it goes -- until an
 * error; the stream's EOF and error as they were, and its new error.  How
 * many whole ones. */
bool writeStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size, nfs::u32 n,
                 nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x20);  // esi, edi, ebp; five dwords
    result = 0;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    if (!(flags(app, fp) & kWriting))
    {
        if (!setThreadWord(app, ts, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, ts, kReleaseFile, fp);
    }
    const nfs::u32 total = n * size;
    if (total == 0)
        return streamLock(app, ts, kReleaseFile, fp);
    if (bufferOf(app, fp) == 0 && !nested(ts, [&] { return allocateBuffer(app, ts, fp); }))
        return false;
    const nfs::u32 kept = flags(app, fp) & (kAtEof | kError);
    setFlags(app, fp, 0, kAtEof | kError);
    nfs::u32 written = 0;
    if (flags(app, fp) & kBinary)
    {
        for (nfs::u32 left = total;;)
        {
            nfs::u32 step;
            if (at(app, fp + kCnt) == 0 && left >= at(app, fp + kBufsize))
            {
                nfs::u32 direct = left & 0xfffffe00;
                if (direct == 0)
                    direct = left;
                if (!called(ts, [&] { return qwriteOf(app, ts, at(app, fp + kFileHandle), buffer, direct, step); }))
                    return false;
                if (step == 0)
                {
                    nfs::u32 data;
                    if (!threadData(app, ts, data))
                        return false;
                    put(app, data + kErrno, kENOSPC);
                }
                if (step == 0 || step == 0xffffffff)
                    setFlags(app, fp, kError);
            }
            else
            {
                // rep movsd, then rep movsb: by dwords where they overlap.
                step = std::min(at(app, fp + kBufsize) - at(app, fp + kCnt), left);
                const nfs::u32 to = at(app, fp + kPtr);
                nfs::u32 i = 0;
                for (; i < (step & ~3u); i += 4)
                    put(app, to + i, at(app, buffer + i));
                for (; i < step; ++i)
                    putByte(app, to + i, byte(app, buffer + i));
                put(app, fp + kCnt, at(app, fp + kCnt) + step);
                put(app, fp + kPtr, at(app, fp + kPtr) + step);
                setFlags(app, fp, kDirty);
                if (at(app, fp + kCnt) == at(app, fp + kBufsize) || (flags(app, fp) & kNoBuf))
                {
                    nfs::u32 ignored;
                    if (!nested(ts, [&] { return flushStream(app, ts, fp, ignored); }))
                        return false;
                }
            }
            buffer += step;
            written += step;
            left -= step;
            if (left == 0 || (flags(app, fp) & kError))
                break;
        }
    }
    else
    {
        const bool unbuffered = flags(app, fp) & kNoBuf;
        if (unbuffered)
            setFlags(app, fp, kFullBuf, kFullBuf | kNoBuf);
        const nfs::u32 link = linkOf(app, fp);
        const nfs::u32 orientation = at(app, link + kOrientation);
        put(app, link + kOrientation, 1);
        for (;;)
        {
            nfs::u32 ignored;
            if (!nested(ts, [&] { return putChar(app, ts, byte(app, buffer++), fp, ignored); }))
                return false;
            if (flags(app, fp) & (kAtEof | kError))
                break;
            if (++written == total)
                break;
        }
        put(app, linkOf(app, fp) + kOrientation, orientation);
        if (unbuffered)
        {
            setFlags(app, fp, kNoBuf, kFullBuf | kNoBuf);
            nfs::u32 ignored;
            if (!nested(ts, [&] { return flushStream(app, ts, fp, ignored); }))
                return false;
        }
    }
    if (flags(app, fp) & kError)
        written = 0;
    setFlags(app, fp, kept);
    if (!streamLock(app, ts, kReleaseFile, fp))
        return false;
    result = written / size;
    return true;
}

/* __update_buffer's (clib/streamio/c/fseek.c): a seek `diff` from where the
 * stream is, within what is buffered -- back to the buffer's start, on to
 * what is left in it -- done there, the end of the file cleared, and 0; 1
 * for one outside it. */
nfs::u32 updateBuffer(nfs::App* app, nfs::u32 diff, nfs::u32 fp)
{
    if (nfs::s32(diff) > nfs::s32(at(app, fp + kCnt))
        || nfs::s32(diff) < nfs::s32(bufferOf(app, fp) - at(app, fp + kPtr)))
        return 1;
    const nfs::u32 ptr = at(app, fp + kPtr) + diff, cnt = at(app, fp + kCnt) - diff;
    setFlags(app, fp, 0, kAtEof);
    put(app, fp + kPtr, ptr);
    put(app, fp + kCnt, cnt);
    return 0;
}

/* __reset_buffer's: the end of the file cleared and the buffer emptied. */
void resetBuffer(nfs::App* app, nfs::u32 fp)
{
    setFlags(app, fp, 0, kAtEof);
    emptyBuffer(app, fp);
}

/* __fill_buffer's (clib/streamio/c/fgetc.c): the buffer filled by __qread
 * -- a byte for an unbuffered stream -- from where it starts, the ttys'
 * streams flushed first if this is a tty's line by line or unbuffered; an
 * unbuffered tty's handle 0 by the console's getche.  How many are in it:
 * none at the end of the file, none and an error for one. */
bool fillBuffer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& count)
{
    const Frame frame(ts, 0x10);  // ebx, ecx, edx, esi
    if (bufferOf(app, fp) == 0 && !nested(ts, [&] { return allocateBuffer(app, ts, fp); }))
        return false;
    if ((flags(app, fp) & kTty) && (flags(app, fp) & (kLineBuf | kNoBuf)))
    {
        nfs::u32 ignored;
        if (!nested(ts, [&] { return flushAll(app, ts, kTty, ignored); }))
            return false;
    }
    setFlags(app, fp, 0, kUnget);
    put(app, fp + kPtr, bufferOf(app, fp));
    if ((flags(app, fp) & (kTty | kNoBuf)) == (kTty | kNoBuf) && at(app, fp + kFileHandle) == 0)
    {
        put(app, fp + kCnt, 0);
        const nfs::u32 c = guest::direct<kGetche, clib::getche>(app, ts);
        if (ts.terminate)
            return false;
        if (c != 0xffffffff)
        {
            putByte(app, at(app, fp + kPtr), nfs::u8(c));
            put(app, fp + kCnt, 1);
        }
    }
    else
    {
        nfs::u32 read;
        if (!called(ts, [&] {
                return qreadOf(app, ts, at(app, fp + kFileHandle), at(app, fp + kPtr),
                               flags(app, fp) & kNoBuf ? 1 : at(app, fp + kBufsize), read);
            }))
            return false;
        put(app, fp + kCnt, read);
    }
    const nfs::s32 cnt = nfs::s32(at(app, fp + kCnt));
    if (cnt == 0)
    {
        setFlags(app, fp, kAtEof);
    }
    else if (cnt < 0)
    {
        put(app, fp + kCnt, 0);
        setFlags(app, fp, kError);
    }
    count = at(app, fp + kCnt);
    return true;
}

/* __filbuf's: the buffer filled, and its first byte taken; -1 for none. */
bool fillAndTake(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& c)
{
    const Frame frame(ts, 0xc);  // ebx, ecx, edx
    nfs::u32 count;
    if (!nested(ts, [&] { return fillBuffer(app, ts, fp, count); }))
        return false;
    c = 0xffffffff;
    if (count != 0)
    {
        const nfs::u32 cnt = at(app, fp + kCnt) - 1, ptr = at(app, fp + kPtr) + 1;
        put(app, fp + kCnt, cnt);
        put(app, fp + kPtr, ptr);
        c = byte(app, ptr - 1);
    }
    return true;
}

/* A byte of the buffer, as fgetc takes one: _cnt down, and the buffer
 * filled when that leaves it below 0. */
bool takeChar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& c)
{
    const nfs::u32 cnt = at(app, fp + kCnt) - 1;
    put(app, fp + kCnt, cnt);
    if (nfs::s32(cnt) < 0)
        return nested(ts, [&] { return fillAndTake(app, ts, fp, c); });
    const nfs::u32 ptr = at(app, fp + kPtr);
    c = byte(app, ptr);
    put(app, fp + kPtr, ptr + 1);
    return true;
}

/* fgetc's: a byte of the stream -- in text a '\r' dropped for the byte
 * after it, ^Z its end -- or -1; a stream of wide characters gives none,
 * one not open for reading is EBADF. */
bool getChar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x14);  // ebx, ecx, edx, esi, ebp
    result = 0xffffffff;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    const nfs::u32 link = linkOf(app, fp);
    const nfs::u32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
            return streamLock(app, ts, kReleaseFile, fp);
        put(app, link + kOrientation, 1);
    }
    nfs::u32 c = 0xffffffff;
    if (!(flags(app, fp) & kReading))
    {
        if (!setThreadWord(app, ts, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
    }
    else if (!takeChar(app, ts, fp, c))
    {
        return false;
    }
    if (!(flags(app, fp) & kBinary))
    {
        if (c == '\r' && !takeChar(app, ts, fp, c))
            return false;
        if (c == 0x1a)
        {
            setFlags(app, fp, kAtEof);
            c = 0xffffffff;
        }
    }
    result = c;
    return streamLock(app, ts, kReleaseFile, fp);
}

/* fread's: n of size bytes -- binary, what the buffer has, then straight
 * from __qread in whole 512s (unless the stream is unbuffered) while as many
 * as the buffer holds are left, through the buffer otherwise; text, by the
 * byte, each '\r' dropped for the byte after it, up to a ^Z, its end -- until
 * the end of the file or an error.  How many whole ones. */
bool readStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size, nfs::u32 n,
                nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x1c);  // esi, edi, ebp; four dwords
    result = 0;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    if (!(flags(app, fp) & kReading))
    {
        if (!setThreadWord(app, ts, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, ts, kReleaseFile, fp);
    }
    const nfs::u32 total = n * size;
    if (total == 0)
        return streamLock(app, ts, kReleaseFile, fp);
    if (bufferOf(app, fp) == 0 && !nested(ts, [&] { return allocateBuffer(app, ts, fp); }))
        return false;
    nfs::u32 read = 0;
    if (flags(app, fp) & kBinary)
    {
        for (nfs::u32 left = total;;)
        {
            const nfs::u32 cnt = at(app, fp + kCnt);
            if (cnt != 0)
            {
                // rep movsd, then rep movsb: by dwords where they overlap.
                const nfs::u32 step = std::min(cnt, left);
                const nfs::u32 from = at(app, fp + kPtr);
                nfs::u32 i = 0;
                for (; i < (step & ~3u); i += 4)
                    put(app, buffer + i, at(app, from + i));
                for (; i < step; ++i)
                    putByte(app, buffer + i, byte(app, from + i));
                const nfs::u32 ptr = at(app, fp + kPtr) + step, rest = at(app, fp + kCnt) - step;
                buffer += step;
                read += step;
                left -= step;
                put(app, fp + kPtr, ptr);
                put(app, fp + kCnt, rest);
            }
            if (left == 0)
                break;
            if (left >= at(app, fp + kBufsize) || (flags(app, fp) & kNoBuf))
            {
                const nfs::u32 base = bufferOf(app, fp);
                put(app, fp + kCnt, 0);
                nfs::u32 asked = left;
                const bool unbuffered = flags(app, fp) & kNoBuf;
                put(app, fp + kPtr, base);
                if (!unbuffered && asked > 0x200)
                    asked &= 0xfffffe00;
                nfs::u32 got;
                if (!called(ts, [&] { return qreadOf(app, ts, at(app, fp + kFileHandle), buffer, asked, got); }))
                    return false;
                if (got == 0xffffffff)
                {
                    setFlags(app, fp, kError);
                    break;
                }
                if (got == 0)
                {
                    setFlags(app, fp, kAtEof);
                    break;
                }
                buffer += got;
                left -= got;
                read += got;
            }
            else
            {
                nfs::u32 count;
                if (!nested(ts, [&] { return fillBuffer(app, ts, fp, count); }))
                    return false;
                if (count == 0)
                    break;
            }
        }
    }
    else
    {
        // Each byte as fgetc's code takes one, but for filling the buffer
        // only once it is empty.
        auto take = [&](nfs::u32& c) {
            if (at(app, fp + kCnt) == 0)
            {
                nfs::u32 count;
                if (!nested(ts, [&] { return fillBuffer(app, ts, fp, count); }))
                    return false;
                if (count == 0)
                    return c = 0xffffffff, true;
            }
            const nfs::u32 cnt = at(app, fp + kCnt) - 1, ptr = at(app, fp + kPtr);
            put(app, fp + kCnt, cnt);
            put(app, fp + kPtr, ptr + 1);
            c = byte(app, ptr);
            return true;
        };
        for (const nfs::u32 end = buffer + total; buffer != end;)
        {
            nfs::u32 c;
            if (!take(c))
                return false;
            if (c == '\r' && !take(c))
                return false;
            if (c == 0xffffffff)
                break;
            if (c == 0x1a)
            {
                setFlags(app, fp, kAtEof);
                break;
            }
            putByte(app, buffer++, nfs::u8(c));
            ++read;
        }
    }
    if (!streamLock(app, ts, kReleaseFile, fp))
        return false;
    result = read / size;
    return true;
}

/* fgets's: bytes by fgetc into the buffer, up to n - 1 of them and a '\n',
 * and a 0 after; none for the end of the file before any, or an error.  The
 * stream's end and error as they were, and new ones.  fgetc's last result is
 * kept in a dword of its frame, whatever was there for none. */
bool getString(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 n, nfs::u32 fp,
               nfs::u32& result)
{
    const Frame frame(ts, 0x14);  // ecx, esi, edi, ebp; a dword
    nfs::u32 c = at(app, ts.sp);
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    const nfs::u32 kept = flags(app, fp) & (kAtEof | kError);
    setFlags(app, fp, 0, kAtEof | kError);
    nfs::u32 to = buffer;
    for (nfs::u32 left = n - 1; nfs::s32(left) > 0; --left)
    {
        if (!nested(ts, [&] { return getChar(app, ts, fp, c); }))
            return false;
        if (c == 0xffffffff)
            break;
        putByte(app, to++, nfs::u8(c));
        if (nfs::u8(c) == '\n')
            break;
    }
    result = buffer;
    if (c == 0xffffffff && (to == buffer || (flags(app, fp) & kError)))
        result = 0;
    else
        putByte(app, to, 0);
    setFlags(app, fp, kept);
    return streamLock(app, ts, kReleaseFile, fp);
}

/* fseek's: a stream open for writing, or with a byte put back, flushed and
 * its buffer given up, then lseek -- for SEEK_CUR from where the stream is,
 * not the handle; one for reading moved within its buffer where it can be,
 * by lseek and the buffer emptied otherwise.  0, or -1, EINVAL for a
 * whence that is none, and for a flush that failed on a SEEK_SET before 0. */
bool seekStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32 offset, nfs::u32 whence,
                nfs::u32& result)
{
    const Frame frame(ts, 0xc);  // ecx, esi, edi
    result = 0xffffffff;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    auto invalid = [&] {
        return setThreadWord(app, ts, kErrno, kEINVAL) && streamLock(app, ts, kReleaseFile, fp);
    };
    auto seek = [&](nfs::u32 to, bool reset) {
        nfs::u32 place;
        if (!called(ts, [&] { return lseekOf(app, ts, at(app, fp + kFileHandle), to, whence, place); }))
            return false;
        if (place == 0xffffffff)
            return streamLock(app, ts, kReleaseFile, fp);
        if (reset)
            resetBuffer(app, fp);
        result = 0;
        return streamLock(app, ts, kReleaseFile, fp);
    };
    const nfs::u32 f = flags(app, fp);
    if (f & (kWriting | kUnget))
    {
        if (flags(app, fp) & kDirty)
        {
            nfs::u32 flushed;
            if (!nested(ts, [&] { return flushStream(app, ts, fp, flushed); }))
                return false;
            if (flushed != 0)
            {
                if (whence == 0 && nfs::s32(offset) < 0)
                    return invalid();
                return streamLock(app, ts, kReleaseFile, fp);
            }
        }
        else
        {
            if (whence == 1)
                offset -= at(app, fp + kCnt);
            emptyBuffer(app, fp);
        }
        setFlags(app, fp, 0, kUnget | kAtEof);
        return seek(offset, false);
    }
    if (whence == 1)
    {
        const nfs::u32 cnt = at(app, fp + kCnt);
        if (updateBuffer(app, offset, fp) == 0)
            return result = 0, streamLock(app, ts, kReleaseFile, fp);
        return seek(offset - cnt, true);
    }
    if (whence == 0)
    {
        nfs::u32 place;
        if (!called(ts, [&] { return tellOf(app, ts, at(app, fp + kFileHandle), place); }))
            return false;
        if (updateBuffer(app, offset - (place - at(app, fp + kCnt)), fp) == 0)
            return result = 0, streamLock(app, ts, kReleaseFile, fp);
        return seek(offset, true);
    }
    if (whence == 2)
    {
        const nfs::u32 link = linkOf(app, fp);
        setFlags(app, fp, 0, kAtEof);
        put(app, fp + kPtr, at(app, link + kLinkBase));
        put(app, fp + kCnt, 0);
        return seek(offset, false);
    }
    return invalid();
}

/* ftell's: where the stream is -- _tell's place, less what is left to read
 * of the buffer, or more what is written to it; a stream appended to and
 * dirty flushed first. */
bool tellStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0x10);  // ebx, ecx, edx, esi
    if ((flags(app, fp) & kAppend) && (flags(app, fp) & kDirty))
    {
        nfs::u32 ignored;
        if (!nested(ts, [&] { return flushStream(app, ts, fp, ignored); }))
            return false;
    }
    if (!called(ts, [&] { return tellOf(app, ts, at(app, fp + kFileHandle), result); }))
        return false;
    if (result == 0xffffffff)
        return true;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    const nfs::u32 cnt = at(app, fp + kCnt);
    if (cnt != 0)
        result = flags(app, fp) & kDirty ? result + cnt : result - cnt;
    return streamLock(app, ts, kReleaseFile, fp);
}

/* ---- Opening and closing: a FILE of __iob's or the heap's, its link on
 * __OpenStreams, and a handle -- __OSHandles' first free, Windows' HANDLE in
 * it -- by CreateFileA; and back. ---- */

constexpr nfs::u32 kClosedStreams = 0x9f38a4;               // the links of streams closed, to be used again
constexpr nfs::u32 kIobEnd = 0x567160;                      // past __iob's 20
constexpr nfs::u32 kIobStart = 0x566f58, kFileSize = 0x1a, kLinkSize = 0x1d;
constexpr nfs::u32 kDynamic = 0x4000, kTmpFile = 0x800;     // _flag's: the heap's FILE; tmpfile's
constexpr nfs::u32 kTmpfChar = 0x14;                        // the link's: a temporary file's number
constexpr nfs::u32 kHandleCount = 0x56acc0;                 // __NHandles: how many __OSHandles has
constexpr nfs::u32 kFakeHandle = 0x56acc4;                  // __NTGetFakeHandle's last
constexpr nfs::u32 kAccessHandles = 0x567798, kReleaseHandles = 0x56779c;  // their lock
constexpr nfs::u32 kAllocHandle = 0x567770, kFreeHandle = 0x567774;        // what calls __allocPOSIXHandle and __freePOSIXHandle
/* The default windowing's other hooks, none in the game. */
constexpr nfs::u32 kRemoveWindowedHandle = 0x5677b4, kNewWindow = 0x5677b8, kDestroyWindow = 0x5677bc;
constexpr nfs::u32 kCommitMode = 0x568260;  // _commode
constexpr nfs::u32 kFileMode = 0x567164;    // _fmode: O_TEXT, or O_BINARY
constexpr nfs::u32 kConsole = 0x54f6f0;     // "con"
constexpr nfs::u32 kRealloc = 0x5185c0;     // _nrealloc: still the game's
constexpr nfs::u32 kMkTmpFile = 0x4ee168;   // __MkTmpFile: still the game's
constexpr nfs::u32 kEMFILE = 11;
/* open's flags. */
constexpr nfs::u32 kOpenAppend = 0x10, kOpenCreate = 0x20, kOpenTruncate = 0x40, kOpenText = 0x100;
constexpr nfs::u32 kOpenNoInherit = 0x80, kOpenBinary = 0x200, kOpenExclusive = 0x400;

nfs::u32 freeStream(nfs::App* app, nfs::u32 fp);
bool openHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 oflag, nfs::u32 shflag,
                nfs::u32 args, nfs::u32& result);

/* _sopen's, its return address at esp and its words above it -- the name, the flags, the sharing,
 * the rest: __sopen's, with a va_list of its frame for the rest. */
bool sopenAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32& result)
{
    const nfs::u32 entry = ts.sp;
    const Frame frame(ts, 0x10);  // ebx, ecx, edx; the va_list
    const nfs::u32 args = ts.sp;
    put(app, args, entry + 0x10);
    return nested(ts, [&] {
        return openHandle(app, ts, at(app, entry + 4), at(app, entry + 8), at(app, entry + 0xc), args, result);
    });
}

/* __GetNTAccessAttr's: CreateFileA's access and attributes for a read and
 * write mode. */
void accessAttributes(nfs::App* app, nfs::u32 rwmode, nfs::u32 access, nfs::u32 attributes)
{
    put(app, access, rwmode == 2 ? 0xc0000000 : rwmode == 1 ? 0x40000000 : 0x80000000);
    put(app, attributes, rwmode == 2 || rwmode == 1 ? 0x80 : 1);
}

/* __GetNTShareAttr's: CreateFileA's sharing for SH_COMPAT -- reading, and
 * writing too for a file opened to read -- SH_DENYRW, SH_DENYWR, SH_DENYRD,
 * SH_DENYNO; none written for another.  The sharing bits. */
nfs::u32 shareAttributes(nfs::App* app, nfs::u32 mode, nfs::u32 share)
{
    const nfs::u32 sharing = mode & 0x70;
    switch (sharing)
    {
    case 0x00: put(app, share, (mode & 7) == 0 ? 3 : 1); break;
    case 0x10: put(app, share, 0); break;
    case 0x20: put(app, share, 1); break;
    case 0x30: put(app, share, 2); break;
    case 0x40: put(app, share, 3); break;
    }
    return sharing;
}

/* The one of 0x51e2b0: 1 when __OSHandles is as long as __NFiles and has no
 * slot free, 0 otherwise. */
nfs::u32 handlesFull(nfs::App* app)
{
    const nfs::u32 count = at(app, kHandleCount), table = at(app, kOsHandles);
    if (count < at(app, kFileCount))
        return 0;
    if (nfs::s32(count) <= 0)
        return 1;
    nfs::u32 offset = 0;
    do
    {
        if (at(app, table + offset) == 0)
            return 0;
        offset += 4;
    } while (nfs::s32(offset) < nfs::s32(count << 2));
    return 1;
}

/* __NTGetFakeHandle's: an event's HANDLE, or one past 0x80000000 of the
 * runtime's own. */
bool fakeHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32& handle)
{
    const Frame frame(ts, 8);  // ecx, edx
    handle = os::events::make(app, false, false);
    if (handle == 0)
        put(app, kFakeHandle, handle = at(app, kFakeHandle) + 1);
    return true;
}

/* __allocPOSIXHandle's: the HANDLE into __OSHandles' first free slot, or
 * one more at its end, by _nrealloc, under the handles' lock -- given back
 * with eax the slot's offset, or the table.  Its handle. */
bool allocHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 os, nfs::u32& handle)
{
    const Frame frame(ts, 0x10);  // ebx, ecx, edx, esi
    if (!hook<kAccessHandles>(app, ts, {os}))
        return false;
    const nfs::u32 count = at(app, kHandleCount);
    if (nfs::s32(count) > 0)
    {
        nfs::u32 offset = 0;
        handle = 0;
        do
        {
            const nfs::u32 slot = at(app, kOsHandles) + offset;
            if (at(app, slot) == 0)
            {
                put(app, slot, os);
                return hook<kReleaseHandles>(app, ts, {offset}).has_value();
            }
            offset += 4;
            ++handle;
        } while (nfs::s32(offset) < nfs::s32(count << 2));
    }
    const nfs::u32 table =
        guest::direct<kRealloc, clib::realloc>(app, ts, at(app, kOsHandles), (at(app, kHandleCount) + 1) << 2);
    if (ts.terminate)
        return false;
    const nfs::u32 last = at(app, kHandleCount);
    put(app, kOsHandles, table);
    put(app, table + 4 * last, os);
    put(app, kHandleCount, last + 1);
    if (!hook<kReleaseHandles>(app, ts, {table}))
        return false;
    handle = at(app, kHandleCount) - 1;
    return true;
}

/* __freePOSIXHandle's: a handle's slot free again, but 0's, under the
 * handles' lock; `eax` as its code leaves it, what the lock's giving back
 * leaves. */
bool freeHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& eax)
{
    const Frame frame(ts, 4);  // edx
    const auto taken = hook<kAccessHandles>(app, ts, {handle});
    if (!taken)
        return false;
    eax = *taken;
    if (nfs::s32(handle) > 0 && nfs::s32(handle) < nfs::s32(at(app, kHandleCount)))
    {
        eax = handle << 2;
        put(app, at(app, kOsHandles) + eax, 0);
    }
    const auto released = hook<kReleaseHandles>(app, ts, {eax});
    if (!released)
        return false;
    eax = *released;
    return true;
}

/* __close's: a window's handle's window gone, by the default windowing's
 * hooks, or the port's handle let go, and the handle's mode 0.  0, or -1. */
bool closeHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle, nfs::u32& result)
{
    const Frame frame(ts, 0x18);  // ebx, ecx, edx, esi, edi, ebp
    if (!goodHandle(app, handle))
        return badHandle(app, ts, result);
    result = 0;
    const nfs::u32 destroy = at(app, kDestroyWindow), os = osHandle(app, handle);
    nfs::u32 window = 0;
    if (destroy != 0)
    {
        const auto found = through(app, ts, at(app, kIsWindowedHandle), {handle});
        if (!found)
            return false;
        window = *found;
        if (window != 0)
        {
            if (!through(app, ts, at(app, kRemoveWindowedHandle), {handle})
                || !through(app, ts, at(app, kDestroyWindow), {window}))
                return false;
        }
    }
    if (window == 0)
        os::release(app, os);
    setIoMode(app, handle, 0);  // __SetIOMode's
    return true;
}

/* __sopen's (clib/handleio/c/openwnt.c): name opened as the flags say --
 * read, written or both, created, truncated, exclusively; once more, as
 * the second disposition says, for one to create it could not open --
 * its permissions taken from the va_list (the port keeps none); a handle
 * of __OSHandles for it and its mode: read, write, append, binary as the
 * flags or _fmode say, a tty's.  "con" a window of the default
 * windowing's, if it has any.  The handle, or -1 and errno.  The sharing
 * asked (shflag) means nothing to the port.  Its frame's last dword is the
 * flags, read back from there. */
bool openHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 oflag, nfs::u32 shflag,
                nfs::u32 args, nfs::u32& result)
{
    NFS2_USE(shflag);
    const Frame frame(ts, 0x2c);  // esi, edi, ebp; eight dwords
    const nfs::u32 flags = ts.sp + 0x1c;
    put(app, flags, oflag);
    result = 0xffffffff;
    if (handlesFull(app) != 0)
        return setThreadWord(app, ts, kErrno, kEMFILE);
    const nfs::u32 rwmode = oflag & 7;
    const os::files::Access access = rwmode == 2   ? os::files::Access::readWrite
                                     : rwmode == 1 ? os::files::Access::write
                                                   : os::files::Access::read;
    nfs::u32 handle, mode = 0;
    bool window = false;
    if (at(app, kNewWindow) != 0 && clib::stricmp(app, name, kConsole) == 0)
    {
        nfs::u32 fake;
        if (!nested(ts, [&] { return fakeHandle(app, ts, fake); }))
            return false;
        const auto allocated = hook<kAllocHandle>(app, ts, {fake});
        if (!allocated)
            return false;
        handle = *allocated;
        mode = 0x2000;
        // ecx and edx kept across it, which its code reads after.
        if (!through(app, ts, at(app, kNewWindow), {handle, mode, 0, handle}, {0, handle, 0xffffffff}))
            return false;
        window = true;
    }
    if (!window)
    {
        using os::files::Create;
        Create disposition, retry = Create::openExisting;
        if (byte(app, flags) & kOpenCreate)
        {
            // The permissions' word taken off the va_list.
            const nfs::u32 va = at(app, args) + 4;
            put(app, args, va);
            put(app, args, 0);
            if (byte(app, flags + 1) & (kOpenExclusive >> 8))
                disposition = retry = Create::createNew;
            else if (byte(app, flags) & kOpenTruncate)
            {
                retry = Create::createAlways;
                disposition = Create::createNew;
            }
            else
            {
                retry = Create::openAlways;
                disposition = Create::openExisting;
            }
        }
        else
        {
            disposition = byte(app, flags) & kOpenTruncate ? Create::truncateExisting : Create::openExisting;
        }
        auto opened = os::files::open(app, guest::text(app, name), access, disposition);
        if (!opened && (byte(app, flags) & kOpenCreate))
            opened = os::files::open(app, guest::text(app, name), access, retry);
        if (!opened)
            return setErrnoNt(app, ts);
        const nfs::u32 os = *opened;
        const auto allocated = hook<kAllocHandle>(app, ts, {os});
        if (!allocated)
            return false;
        handle = *allocated;
        if (handle >= at(app, kFileCount))
        {
            os::release(app, os);
            return setThreadWord(app, ts, kErrno, kENOMEM);
        }
        nfs::u32 tty;
        if (!called(ts, [&] { return isTty(app, ts, handle, tty); }))
            return false;
        if (tty != 0)
            mode = kIsTty;
    }
    if (rwmode == 2)
        mode |= 3;
    else if (rwmode == 0)
        mode |= 1;
    else if (rwmode == 1)
        mode |= 2;
    if (byte(app, flags) & kOpenAppend)
        mode |= kAppend;
    const nfs::u8 kind = byte(app, flags + 1);
    if (kind & ((kOpenText | kOpenBinary) >> 8) ? (kind & (kOpenBinary >> 8)) != 0 : at(app, kFileMode) == kOpenBinary)
        mode |= kBinary;
    setIoMode(app, handle, mode);  // __SetIOMode's
    result = handle;
    return true;
}

/* __open_flags' (clib/streamio/c/fopen.c): a mode string's flags -- 'r',
 * 'w' or 'a' first, then '+', 't' or 'b', 'c' or 'n' each once, anything
 * else let be; one twice ends it there -- binary by _fmode for neither 't'
 * nor 'b'; the extended flags committed as _commode says, or as 'c' and 'n'
 * say.  0 and EINVAL for another first. */
bool openFlags(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mode, nfs::u32 extflags, nfs::u32& result)
{
    const Frame frame(ts, 0x18);  // ebx, ecx, esi, edi, ebp; a dword
    if (extflags != 0)
        put(app, extflags, at(app, kCommitMode) == 1 ? 1 : 0);
    nfs::u32 flags;
    switch (byte(app, mode))
    {
    case 'r': flags = kReading; break;
    case 'w': flags = kWriting; break;
    case 'a': flags = kWriting | kAppend; break;
    default:
        result = 0;
        return setThreadWord(app, ts, kErrno, kEINVAL);
    }
    bool good = true, plus = false, kind = false, commit = false;
    for (nfs::u32 at_ = mode + 1; byte(app, at_) != 0 && good; ++at_)
    {
        switch (byte(app, at_))
        {
        case '+':
            good = !plus;
            if (good)
                flags |= kReading | kWriting, plus = true;
            break;
        case 't':
        case 'b':
            good = !kind;
            if (good && byte(app, at_) == 'b')
                flags |= kBinary;
            kind = true;
            break;
        case 'c':
        case 'n':
            good = !commit;
            if (good)
                putByte(app, extflags, byte(app, at_) == 'c' ? byte(app, extflags) | kCommit : byte(app, extflags) & ~kCommit);
            commit = true;
            break;
        }
    }
    if (!kind && at(app, kFileMode) == kOpenBinary)
        flags |= kBinary;
    result = flags;
    return true;
}

/* __doopen's (clib/streamio/c/fopen.c): the stream's flags, its handle by
 * _sopen -- read, or written, created and truncated or appended to, as the
 * mode's first says, text or binary, 0600 -- its buffer and orientation
 * none, its extended flags; an appended one seeked to its end, a tty's
 * marked.  The stream, or 0, given back, for a handle refused. */
bool doOpen(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u8 first, nfs::u32 flags,
            nfs::u32 extflags, nfs::u32 shflag, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 0xc);  // esi, edi, ebp
    putByte(app, fp + kFlag, byte(app, fp + kFlag) & ~(kReading | kWriting));
    put(app, fp + kFlag, at(app, fp + kFlag) | flags);
    nfs::u32 oflag = flags & kBinary ? kOpenBinary : kOpenText, permission = 0;
    if (nfs::u8(clib::tolower(app, first)) == 'r')
    {
        oflag |= flags & kWriting ? 2 : 0;
    }
    else
    {
        oflag |= (flags & kReading ? 2 : 1) | kOpenCreate | (flags & kAppend ? kOpenAppend : kOpenTruncate);
        permission = 0x180;
    }
    nfs::u32 handle;
    {
        // _sopen as the code calls it: its four words pushed, the va_list it makes points at the last.
        const EspKept kept(ts);
        const nfs::u32 esp = ts.sp;
        put(app, esp - 0x10, name);
        put(app, esp - 0xc, oflag);
        put(app, esp - 8, shflag);
        put(app, esp - 4, permission);
        ts.sp = esp - 0x14;
        if (!sopenAt(app, ts, handle))
            return false;
    }
    put(app, fp + kFileHandle, handle);
    result = 0;
    if (at(app, fp + kFileHandle) == 0xffffffff)
    {
        freeStream(app, fp);
        return true;
    }
    put(app, fp + kCnt, 0);
    const nfs::u32 link = linkOf(app, fp);
    put(app, fp + kBufsize, 0);
    put(app, link + kOrientation, 0);
    put(app, linkOf(app, fp) + kExtFlags, extflags);
    put(app, linkOf(app, fp) + 0x15, 0);
    put(app, linkOf(app, fp) + kLinkBase, 0);
    if (flags & kAppend)
    {
        nfs::u32 ignored;
        if (!nested(ts, [&] { return seekStream(app, ts, fp, 0, 2, ignored); }))
            return false;
    }
    nfs::u32 ignored;
    if (!nested(ts, [&] { return checkTty(app, ts, fp, ignored); }))
        return false;
    result = fp;
    return true;
}

/* __allocfp's (clib/streamio/c/allocfp.c): a stream for fopen, under the
 * streams' lock -- the last closed's again; or one of __iob's 20 not in use,
 * a link from the heap for it; or a FILE and its link from the heap -- its
 * FILE cleared, in use, its link first on __OpenStreams; 0 and ENOMEM for
 * none.  The lock is given back with eax as its code leaves it. */
bool allocateStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax, nfs::u32& fp)
{
    const Frame frame(ts, 0x14);  // ebx, ecx, edx, esi, edi
    if (!hook<kAccessIob>(app, ts, {eax}))  // eax as its caller has it
        return false;
    nfs::u32 link = at(app, kClosedStreams), flag;
    if (link != 0)
    {
        fp = at(app, link + kLinkStream);
        flag = (at(app, fp + kFlag) & (kDynamic | kReading | kWriting)) | kReading | kWriting;
        put(app, kClosedStreams, at(app, link + kLinkNext));
    }
    else
    {
        fp = 0;
        for (nfs::u32 iob = kIobStart; iob < kIobEnd; iob += kFileSize)
            if (!(byte(app, iob + kFlag) & (kReading | kWriting)))
            {
                fp = iob;
                break;
            }
        if (!called(ts, [&] {
                return watcomheap::allocate(app, ts, kGameHeap, fp != 0 ? kLinkSize : kLinkSize + kFileSize, link);
            }))
            return false;
        flag = fp != 0 ? kReading | kWriting : kDynamic | kReading | kWriting;
        if (link == 0)
        {
            fp = 0;
            nfs::u32 data;
            return setThreadWord(app, ts, kErrno, kENOMEM, &data) && hook<kReleaseIob>(app, ts, {data});
        }
        if (fp == 0)
            fp = link + kLinkSize;
    }
    clib::memset(app, fp, 0, kFileSize);
    put(app, fp + kFlag, flag);
    put(app, link + kLinkStream, fp);
    const nfs::u32 head = at(app, kOpenStreams);
    put(app, fp + kLink, link);
    put(app, kOpenStreams, link);
    put(app, link + kLinkNext, head);
    return hook<kReleaseIob>(app, ts, {head}).has_value();
}

/* __freefp's: the stream's link off __OpenStreams and first on the closed
 * ones', its FILE marked in use still; the link, or 0 for a stream not
 * open. */
nfs::u32 freeStream(nfs::App* app, nfs::u32 fp)
{
    for (nfs::u32 before = kOpenStreams, link = at(app, before); link != 0; before = link, link = at(app, link))
    {
        const nfs::u32 stream = at(app, link + kLinkStream);
        if (stream != fp)
            continue;
        putByte(app, fp + kFlag, byte(app, stream + kFlag) | kReading | kWriting);
        put(app, before, at(app, link + kLinkNext));
        const nfs::u32 closed = at(app, kClosedStreams);
        put(app, kClosedStreams, link);
        put(app, link + kLinkNext, closed);
        return link;
    }
    return 0;
}

/* __doclose's (clib/streamio/c/fclose.c): a stream given up -- flushed, the
 * handle where the stream is, closed if asked, the heap's buffer freed, a
 * temporary file removed by the name __MkTmpFile makes it in its frame --
 * and the handle's slot freed.  0, or what flushing and closing gave; -1
 * for a FILE not in use. */
bool doClose(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32 handleToo, nfs::u32& result)
{
    const Frame frame(ts, 0x128);  // ebx, ecx, esi, edi, ebp; a path
    const nfs::u32 path = ts.sp;
    result = 0xffffffff;
    if (at(app, fp + kFlag) == 0)
        return true;
    result = 0;
    if ((flags(app, fp) & kDirty) && !nested(ts, [&] { return flushStream(app, ts, fp, result); }))
        return false;
    if (!streamLock(app, ts, kAccessFile, fp))
        return false;
    nfs::u32 place;
    if (!nested(ts, [&] { return tellStream(app, ts, fp, place); }))
        return false;
    if (place != 0xffffffff)
    {
        nfs::u32 ignored;
        if (!called(ts, [&] { return lseekOf(app, ts, at(app, fp + kFileHandle), place, 0, ignored); }))
            return false;
    }
    if (handleToo != 0)
    {
        nfs::u32 closed;
        if (!nested(ts, [&] { return closeHandle(app, ts, at(app, fp + kFileHandle), closed); }))
            return false;
        result |= closed;
    }
    if (byte(app, fp + kFlag) & kBigBuf)
    {
        if (!called(ts, [&] { return watcomheap::free(app, ts, kGameHeap, bufferOf(app, fp)); }))
            return false;
        put(app, linkOf(app, fp) + kLinkBase, 0);
    }
    if (flags(app, fp) & kTmpFile)
    {
        guest::direct<kMkTmpFile, clib::mkTmpFile>(app, ts, path, nfs::u32(byte(app, linkOf(app, fp) + kTmpfChar)));
        if (ts.terminate)
            return false;
        nfs::u32 ignored;
        if (!called(ts, [&] { return unlinkOf(app, ts, path, ignored); }))
            return false;
    }
    if (!streamLock(app, ts, kReleaseFile, fp))
        return false;
    return handleToo == 0 || hook<kFreeHandle>(app, ts, {at(app, fp + kFileHandle)});
}

/* __shutdown_stream's: __doclose's, and __freefp. */
bool shutdownStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32 handleToo,
                    nfs::u32& result)
{
    const Frame frame(ts, 4);  // ebx
    if (!nested(ts, [&] { return doClose(app, ts, fp, handleToo, result); }))
        return false;
    freeStream(app, fp);
    return true;
}

/* fclose's: a stream on __OpenStreams shut down, its handle closed; -1 for
 * one not there.  The streams' lock given back with eax its link, or 0. */
bool closeStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fp, nfs::u32& result)
{
    const Frame frame(ts, 8);  // ebx, edx
    if (!hook<kAccessIob>(app, ts, {fp}))
        return false;
    nfs::u32 link = at(app, kOpenStreams);
    while (link != 0 && at(app, link + kLinkStream) != fp)
        link = at(app, link + kLinkNext);
    if (!hook<kReleaseIob>(app, ts, {link}))
        return false;
    result = 0xffffffff;
    return link == 0 || nested(ts, [&] { return shutdownStream(app, ts, fp, 1, result); });
}

/* _fsopen's (clib/streamio/c/fopen.c): the mode's flags, a stream, and
 * __doopen, its extended flags a dword of the frame; 0 for a mode that is
 * none or no stream. */
bool openStream(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 mode, nfs::u32 shflag,
                nfs::u32& result)
{
    const Frame frame(ts, 0x10);  // ecx, esi, edi; a dword
    const nfs::u32 extflags = ts.sp;
    nfs::u32 flags, fp;
    if (!nested(ts, [&] { return openFlags(app, ts, mode, extflags, flags); }))
        return false;
    result = 0;
    if (flags == 0)
        return true;
    if (!nested(ts, [&] { return allocateStream(app, ts, 0, fp); }))
        return false;
    if (fp == 0)
        return true;
    const Frame arguments(ts, 8);  // shflag and the stream, __doopen's on the stack
    return nested(ts, [&] {
        return doOpen(app, ts, name, byte(app, mode), flags, at(app, extflags), shflag, fp, result);
    });
}

}  // namespace

/* fsync (clib/handleio/c/fsync.c, 0x522d80): handle eax's buffers to the
 * disk; 0, or -1. */
nfs::u32 fsync(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!syncHandle(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __chktty (clib/streamio/c/chktty.c, 0x509380): the stream at eax marked a
 * tty's, if it is one. */
nfs::u32 chktty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 eax;
    if (!checkTty(app, ts, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(ts, args, eax);
    return args.eax;
}

/* __ioalloc (clib/streamio/c/ioalloc.c, 0x5029c0): a buffer for the stream at
 * eax. */
nfs::u32 ioalloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 eax;
    if (!allocateBuffer(app, ts, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(ts, args, eax);
    return args.eax;
}

/* __flush (clib/streamio/c/flush.c, 0x502760): the stream at eax's buffer
 * written, or given back; 0, or -1. */
nfs::u32 flush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!flushStream(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fputc (clib/streamio/c/fputc.c, 0x5097b0): eax's character to the stream
 * at edx. */
nfs::u32 fputc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!putChar(app, ts, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fputs (clib/streamio/c/fputs.c, 0x4f7d80): the string at eax to the stream
 * at edx. */
nfs::u32 fputs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!putString(app, ts, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fwrite (clib/streamio/c/fwrite.c, 0x4eebd0): ebx of edx bytes from eax to
 * the stream at ecx. */
nfs::u32 fwrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!writeStream(app, ts, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __flushall (clib/streamio/c/flushall.c, 0x515858): the open streams with
 * a bit of eax flushed; how many. */
nfs::u32 flushall_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 count;
    if (!flushAll(app, ts, args.eax, count))
        return args.eax;
    kept.back(args);
    returns(ts, args, count);
    return args.eax;
}

/* flushall (clib/streamio/c/flushall.c, 0x515850): all of them. */
nfs::u32 flushall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 count;
    if (!flushAll(app, ts, 0xffffffff, count))
        return args.eax;
    kept.back(args);
    returns(ts, args, count);
    return args.eax;
}

/* fflush (clib/streamio/c/fflush.c, 0x4f7d70): the stream at eax flushed --
 * or, for none, all of them, and 0. */
nfs::u32 fflush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (args.eax != 0)
        return flush(app, ts, args);
    const Kept kept(args);
    nfs::u32 count;
    if (!nested(ts, [&] { return flushAll(app, ts, 0xffffffff, count); }))
        return args.eax;
    kept.back(args);
    returns(ts, args, 0);
    return args.eax;
}

/* __update_buffer (clib/streamio/c/fseek.c, 0x4e9030): a seek of eax from
 * where the stream at edx is, within its buffer: 0, or 1 for one outside. */
nfs::u32 update_buffer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, updateBuffer(app, args.eax, args.edx));
    return args.eax;
}

/* __reset_buffer (clib/streamio/c/fseek.c, 0x4e9070): the stream at eax's
 * buffer emptied; eax left the stream. */
nfs::u32 reset_buffer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    resetBuffer(app, args.eax);
    returns(ts, args, args.eax);
    return args.eax;
}

/* __fill_buffer (clib/streamio/c/fgetc.c, 0x502be8): the stream at eax's
 * buffer filled; how many are in it. */
nfs::u32 fill_buffer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 count;
    if (!fillBuffer(app, ts, args.eax, count))
        return args.eax;
    kept.back(args);
    returns(ts, args, count);
    return args.eax;
}

/* __filbuf (clib/streamio/c/fgetc.c, 0x502bb8): the stream at eax's buffer
 * filled, and its first byte; -1 for none. */
nfs::u32 filbuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 c;
    if (!fillAndTake(app, ts, args.eax, c))
        return args.eax;
    kept.back(args);
    returns(ts, args, c);
    return args.eax;
}

/* fgetc (clib/streamio/c/fgetc.c, 0x502af0): a byte of the stream at eax,
 * or -1. */
nfs::u32 fgetc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!getChar(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fread (clib/streamio/c/fread.c, 0x4e9200): ebx of edx bytes into eax from
 * the stream at ecx. */
nfs::u32 fread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!readStream(app, ts, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fgets (clib/streamio/c/fgets.c, 0x4f0570): a line of the stream at ebx
 * into eax, edx bytes at most with its 0; eax, or 0. */
nfs::u32 fgets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!getString(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fseek (clib/streamio/c/fseek.c, 0x4e9088): the stream at eax to edx, from
 * ebx's whence; 0, or -1. */
nfs::u32 fseek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!seekStream(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* ftell (clib/streamio/c/ftell.c, 0x4ee420): where the stream at eax is, or
 * -1. */
nfs::u32 ftell(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!tellStream(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __GetNTAccessAttr (clib/streamio/c/ntfio.c, 0x5234d0): CreateFileA's
 * access to edx's dword and attributes to ebx's for eax's read and write
 * mode. */
nfs::u32 GetNTAccessAttr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    accessAttributes(app, args.eax, args.edx, args.ebx);
    returns(ts, args, args.eax);
    return args.eax;
}

/* __GetNTShareAttr (clib/streamio/c/ntfio.c, 0x523504): CreateFileA's
 * sharing to edx's dword for eax's sharing and mode; eax left the sharing
 * bits. */
nfs::u32 GetNTShareAttr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, shareAttributes(app, args.eax, args.edx));
    return args.eax;
}

/* 0x51e2b0, _sopen's: whether __OSHandles has no slot free. */
nfs::u32 handlesFull(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, handlesFull(app));
    return args.eax;
}

/* __NTGetFakeHandle (clib/handleio/c/hdlman.c, 0x51e4f0): a HANDLE for a
 * handle with none. */
nfs::u32 NTGetFakeHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 handle;
    if (!fakeHandle(app, ts, handle))
        return args.eax;
    kept.back(args);
    returns(ts, args, handle);
    return args.eax;
}

/* __allocPOSIXHandle (clib/handleio/c/hdlman.c, 0x51e304): a handle for the
 * HANDLE at eax. */
nfs::u32 allocPOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 handle;
    if (!allocHandle(app, ts, args.eax, handle))
        return args.eax;
    kept.back(args);
    returns(ts, args, handle);
    return args.eax;
}

/* __freePOSIXHandle (clib/handleio/c/hdlman.c, 0x51e45c): handle eax's slot
 * free. */
nfs::u32 freePOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 eax;
    if (!freeHandle(app, ts, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(ts, args, eax);
    return args.eax;
}

/* __close (clib/handleio/c/_clsewnt.c, 0x5095b0): handle eax closed; 0, or
 * -1. */
nfs::u32 close(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!closeHandle(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __sopen (clib/handleio/c/openwnt.c, 0x50901c): eax opened with edx's
 * flags and ebx's sharing, the permissions from the va_list at ecx. */
nfs::u32 sopen_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!openHandle(app, ts, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* _sopen (clib/handleio/c/openwnt.c, 0x508ff4): cdecl (name, flags,
 * sharing, ...); __sopen's, with a va_list of its frame for the rest. */
nfs::u32 sopen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!sopenAt(app, ts, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __open_flags (clib/streamio/c/fopen.c, 0x4edd80): the mode string at eax's
 * flags, its extended ones to edx's dword. */
nfs::u32 open_flags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!openFlags(app, ts, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __doopen (clib/streamio/c/fopen.c, 0x4edebc): eax opened as dl's mode
 * says, with ebx's flags and ecx's extended ones, into the stream on the
 * stack after its sharing; takes them off. */
nfs::u32 doopen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    const nfs::u32 entry = ts.sp;
    nfs::u32 result;
    if (!doOpen(app, ts, args.eax, nfs::u8(args.edx), args.ebx, args.ecx, at(app, entry + 4), at(app, entry + 8),
                result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    ts.sp += 8;
    return args.eax;
}

/* __allocfp (clib/streamio/c/allocfp.c, 0x509250): a stream, or 0. */
nfs::u32 allocfp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 fp;
    if (!allocateStream(app, ts, args.eax, fp))
        return args.eax;
    kept.back(args);
    returns(ts, args, fp);
    return args.eax;
}

/* __freefp (clib/streamio/c/allocfp.c, 0x509314): the stream at eax off the
 * open ones; its link, or 0. */
nfs::u32 freefp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, freeStream(app, args.eax));
    return args.eax;
}

/* __doclose (clib/streamio/c/fclose.c, 0x4ee214): the stream at eax given
 * up, its handle closed too if edx says so. */
nfs::u32 doclose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!doClose(app, ts, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __shutdown_stream (clib/streamio/c/fclose.c, 0x4ee144): the stream at eax
 * given up and freed. */
nfs::u32 shutdown_stream(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!shutdownStream(app, ts, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fclose (clib/streamio/c/fclose.c, 0x4ee100): the stream at eax closed; 0,
 * or -1. */
nfs::u32 fclose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!closeStream(app, ts, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* _fsopen (clib/streamio/c/fopen.c, 0x4edfa4): eax opened as the mode at edx
 * says, with ebx's sharing; the stream, or 0. */
nfs::u32 fsopen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!openStream(app, ts, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* fopen (clib/streamio/c/fopen.c, 0x4edfe8): _fsopen's, sharing as it may. */
nfs::u32 fopen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    {
        const Frame frame(ts, 4);  // ebx
        if (!nested(ts, [&] { return openStream(app, ts, args.eax, args.edx, 0, result); }))
            return args.eax;
    }
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* ---- The heap (native_watcom_heap.cpp has it; these are the game's calls
 * into it, each keeping the registers its code kept: all but eax, but for
 * what __MemAllocator and __MemFree are handed). ---- */

/* _nmalloc (clib/heap/c/nmalloc.c, 0x4f7900): eax bytes; the block, or 0. */
nfs::u32 nmalloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!watcomheap::allocate(app, ts, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* _nfree (clib/heap/c/nfree.c, 0x4f79f0): the block at eax given back. */
nfs::u32 nfree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!watcomheap::free(app, ts, kGameHeap, args.eax))
        return args.eax;
    const nfs::u32 eax = args.eax;
    kept.back(args);
    returns(ts, args, eax);
    return args.eax;
}

/* __MemAllocator (clib/heap/c/mem.c, 0x515300): eax bytes from the miniheap at
 * ebx (edx the segment, nothing on a flat heap); the block, or 0. */
nfs::u32 MemAllocator(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, watcomheap::allocateIn(app, args.ebx, args.eax));
    return args.eax;
}

/* __MemFree (clib/heap/c/mem.c, 0x5153b0): the block at eax given back to the
 * miniheap at ebx. */
nfs::u32 MemFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watcomheap::freeIn(app, args.ebx, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* __LinkUpNewNHeap (clib/heap/c/grownear.c, 0x5154e0): the new miniheap at eax
 * into the list; its first block. */
nfs::u32 LinkUpNewNHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, watcomheap::link(app, kGameHeap, args.eax));
    return args.eax;
}

/* __AdjustAmount (clib/heap/c/grownear.c, 0x515628): the amount at eax made
 * what a miniheap for it takes, in place; 1, or 0. */
nfs::u32 AdjustAmount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u32 amount = at(app, args.eax);
    const bool adjusted = watcomheap::adjust(app, kGameHeap, amount);
    put(app, args.eax, amount);
    returns(ts, args, adjusted ? 1 : 0);
    return args.eax;
}

/* __CreateNewNHeap (clib/heap/c/grownear.c, 0x515558): a miniheap with room
 * for eax bytes; 1, or 0. */
nfs::u32 CreateNewNHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!watcomheap::create(app, ts, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __ExpandDGROUP (clib/heap/c/grownear.c, 0x515614): the empty miniheaps given
 * back, then a new one with room for eax bytes; 1, or 0. */
nfs::u32 ExpandDGROUP(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!watcomheap::expand(app, ts, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* __nmemneed (clib/heap/c/nmemneed.c, 0x515680): whether the caller could give
 * memory back for eax bytes more -- no. */
nfs::u32 nmemneed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    returns(ts, args, 0);
    return args.eax;
}

/* _nheapshrink (clib/heap/c/nheapmin.c, 0x5241c0): every empty miniheap given
 * back to the system; 0. */
nfs::u32 nheapshrink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    if (!watcomheap::shrink(app, ts, kGameHeap))
        return args.eax;
    kept.back(args);
    returns(ts, args, 0);
    return args.eax;
}

/* __ReturnMemToSystem (clib/heap/c/nheapmin.c, 0x5241fc): the miniheap at eax
 * to VirtualFree, and the rovers off it; 0, or -1. */
nfs::u32 ReturnMemToSystem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!watcomheap::giveBack(app, ts, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}

/* 0x52425c, _nheapshrink's step (clib/heap/c/nheapmin.c, __UnlinkNHeap after
 * __ReturnMemToSystem): the miniheap at eax given back and, if it went, taken
 * off the list; 0, or -1. */
nfs::u32 ReleaseNHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept(args);
    nfs::u32 result;
    if (!watcomheap::release(app, ts, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(ts, args, result);
    return args.eax;
}


/* ---- Formatted output: __prtf, what it calls and what calls it -- sprintf,
 * vsprintf, fprintf, vfprintf.
 *
 * __prtf walks the format and hands each character it makes to a function of
 * its caller's (sprintf's mem_putc, vfprintf's file_putc) with eax its SPECS:
 * a record in __prtf's own frame, which the functions it calls read and
 * write, so the frame is where its code has it, on the guest's stack, and so
 * is every other frame a pointer into is handed out of.  The conversions it
 * has itself are integer ones; a float's (e, f, g) goes to [__EFG_printf],
 * the x87's formatter the start-up code puts there, which stays the game's code.
 * Each is a typed core (fooOf) and its thunk for the calls by address; a
 * core called from another runs at its code's depth, its pushes written where
 * the code writes them -- what a string run past its end in __prtf's frame
 * reads, the registers the code keeps its pointers in among them.
 *
 * es is as the code has it: es is ees's low word, so what is read through it
 * is at ees with the segment loaded in it -- a far pointer's ('F'), or ds for
 * the game's own.  les is never run, NFS2_ASSERT(false) where it would be,
 * and what the register held stays. */
namespace
{

using nfs::u16;
using nfs::u32;
using nfs::u8;

/* SPECS, as __prtf lays it out at its esp. */
constexpr u32 kDest = 0x00;      // where sprintf writes, the FILE for fprintf
constexpr u32 kWidth = 0x04;     // _fld_width
constexpr u32 kPrec = 0x08;      // _prec: -1 for none
constexpr u32 kOutCount = 0x10;  // how many characters went out
constexpr u32 kCharacter = 0x15; // the conversion's
constexpr u32 kPadChar = 0x16;   // ' ' or '0'
constexpr u32 kSpecs1C = 0x1c;   // a word __prtf sets to 100
constexpr u32 kFlags = 0x1e;     // '#' 1, ' ' 2, '+' 4, '-' 8, h 0x10, l 0x20, N 0x40, F 0x80
constexpr u32 kFlags2 = 0x1f;    // I64 or L: 1
constexpr u32 kN0 = 0x20, kNZ0 = 0x24, kN1 = 0x28, kNZ1 = 0x2c, kN2 = 0x30, kNZ2 = 0x34;

constexpr u8 kAlternate = 0x01, kBlank = 0x02, kPlus = 0x04, kLeft = 0x08;
constexpr u8 kShort = 0x10, kLong = 0x20, kNear = 0x40, kFar = 0x80;
constexpr u8 kLongLong = 0x01;

constexpr u32 kFloatFormat = 0x56ac64;  // __EFG_printf
constexpr u32 kDigits = 0x56ac3c;       // "0123456789abcdefghijklmnopqrstuvwxyz": utoa's
constexpr u32 kDigits64 = 0x56ac6c;     // ulltoa's
constexpr u32 kDigitsLow = 0x56ac94;    // __prtf's own utoa's
constexpr u32 kMbcsOn = 0xa0b200;       // whether multibyte characters are on
constexpr u32 kMbcsLead = 0xa0b211;     // a byte a character: 1 for a lead byte
constexpr u32 kCodePage = 0x56b0b8;     // the code page the runtime has set

constexpr u32 kMemPutc = 0x4df660;
constexpr u32 kFilePutc = 0x4ff000;

u16 half(nfs::App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putHalf(nfs::App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

/* What the segment registers and the direction flag are to __prtf's code, which reads through es and
 * ds and runs string instructions: ees's high word (a mov es leaves it), ds, es as the caller has
 * it, and df.  The game's are all 0 -- flat memory, df clear; the
 * thunks read them from the ts (segmentsOf) and hand them down, a typed caller has the game's. */
struct Segments
{
    u32 high = 0;
    u16 ds = 0;
    u16 es = 0;
    bool df = false;

    // Where es with the segment `segment` puts an offset: es is ees's low word.
    u32 base(u32 segment) const { return high | u16(segment); }
};

Segments segmentsOf(const nfs::ThreadState& ts)
{
    (void)ts;
    return {0, 0, 0, false};  // the flat model's selectors, all 0 in the port; the direction flag: nothing sets it
}

/* What a function's push instructions leave below its return address: each
 * dword in turn, and es's word where it has one (-1 for none), the dword's
 * other half left as it was.  Written as the code writes them, for what reads
 * the stack where they are -- a wide string from __prtf's buffer goes on into
 * its frame.  (Plan step 7: the typed cores keep the room of their pushes and
 * write none of them; what is left of these is the thunks'.) */
void pushed(nfs::App* app, u32 entry, std::initializer_list<u32> registers, int esAt = -1,
            u16 es = 0)
{
    u32 at_ = entry;
    int i = 0;
    for (const u32 r : registers)
    {
        at_ -= 4;
        if (i++ == esAt)
            putHalf(app, at_, es);
        else
            put(app, at_, r);
    }
}


/* The next argument's dword, va_arg's way: the pointer at `argp` moved on by
 * `size` and the dword `size` back read. */
u32 nextArgument(nfs::App* app, u32 argp, u32 size, u32& moved)
{
    moved = at(app, argp) + size;
    put(app, argp, moved);
    return at(app, moved - size);
}

}  // namespace

/* 0x51e060 (wctomb's core): the wide character `wc` in the game's code page, into the two bytes
 * at `to`; how many, or -1.  A null buffer is 0.  The port makes no multibyte characters of wide
 * ones: -1 for every character. */
u32 wctombOf(nfs::App* app, nfs::ThreadState& ts, u32 to, u32 wc)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    NFS2_USE(wc);
    return to == 0 ? 0 : 0xffffffff;
}

nfs::u32 wctomb_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = wctombOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* `value`'s digits in `base` from the table `digits`, made backwards in the frame's 0x28 bytes under
 * the four registers utoa's code pushes, then copied out to `out` the right way round: utoa's and
 * __prtf's own, which keep a different word in the frame's 0x24th. */
u32 digitsOf(nfs::App* app, nfs::ThreadState& ts, u32 value, u32 base, u32 out, u32 digits,
               bool keepsRest)
{
    const GuestFrame frame(ts, 0x10, 0x28);
    const u32 buffer = frame - 0x28;
    putByte(app, buffer, 0);
    u32 p = buffer + 1;
    do
    {
        put(app, buffer + 0x24, base);
        const u32 rest = value % base;
        value /= base;
        put(app, buffer + 0x24, keepsRest ? rest : value);
        putByte(app, p++, byte(app, digits + rest));
    } while (value != 0);
    u32 to = out;
    u8 c;
    do
    {
        c = byte(app, --p);
        putByte(app, to++, c);
    } while (c != 0);
    return out;
}

/* 0x51e0a0 (utoa's core, clib/convert/c/itoa.c): `value` in `base`, its digits from kDigits, into
 * `out`; `out` back. */
u32 utoaOf(nfs::App* app, nfs::ThreadState& ts, u32 value, u32 out, u32 base)
{
    return digitsOf(app, ts, value, base, out, kDigits, false);
}

nfs::u32 utoa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, utoaOf(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x51e0ec (itoa, clib/convert/c/itoa.c): `value` signed in base 10 -- a '-' first -- unsigned in
 * any other, into `out`; `out` back.  At its entry's depth: ecx pushed. */
u32 itoaOf(nfs::App* app, nfs::ThreadState& ts, u32 value, u32 out, u32 base)
{
    const GuestFrame frame(ts, 4, 0);
    u32 to = out;
    if (base == 10 && nfs::s32(value) < 0)
    {
        value = 0 - value;
        putByte(app, to++, '-');
    }
    const ReturnSlot slot(ts);
    utoaOf(app, ts, value, to, base);
    return out;
}

nfs::u32 itoa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, itoaOf(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525d59 (__U8D, the compiler's unsigned 64-bit division): `n` by `d`, the remainder into
 * `rest`. */
nfs::u64 u8dOf(nfs::u64 n, nfs::u64 d, nfs::u64& rest)
{
    rest = d == 0 ? n : n % d;
    return d == 0 ? 0 : n / d;
}

nfs::u32 U8D(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // Its own convention: edx:eax by ecx:ebx, the quotient in edx:eax, the remainder in ecx:ebx;
    // the long division's pushes where it takes them.
    if (args.ecx != 0 && args.ecx < args.edx)
        pushed(app, ts.sp, {args.ebp, args.esi, args.edi});
    nfs::u64 rest;
    const nfs::u64 q = u8dOf(nfs::u64(args.edx) << 32 | args.eax, nfs::u64(args.ecx) << 32 | args.ebx, rest);
    args.eax = u32(q);
    args.edx = u32(q >> 32);
    args.ebx = u32(rest);
    args.ecx = u32(rest >> 32);
    ts.sp += 4;
    return args.eax;
}

/* 0x51e130 (ulltoa, clib/convert/c/lltoa.c): the unsigned 64 bits at `value` in `base`, its digits
 * from kDigits64, into `out`; `out` back -- a digit at a time by __U8D, the number, the quotient
 * and the remainder in its frame: ecx, esi, edi, es and ebp pushed (their room only), then 0x44
 * bytes. */
u32 ulltoaOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 value, u32 out, u32 base)
{
    const GuestFrame frame(ts, 0x14, 0x44);
    const u32 f = frame - 0x44;
    put(app, f + 0x3c, out);
    put(app, f + 0x40, out);
    // Two movsd, es its ds, as the direction flag has them.
    const u32 step = s.df ? u32(-4) : 4;
    const u32 es = s.base(s.ds);
    put(app, es + f + 0x34, at(app, value));
    put(app, es + f + 0x34 + step, at(app, value + step));
    put(app, f + 0x24, base);
    put(app, f + 0x28, 0);
    putByte(app, f, 0);
    u32 p = f + 1;
    nfs::u64 q;
    do
    {
        nfs::u64 rest;
        q = u8dOf(nfs::u64(at(app, f + 0x38)) << 32 | at(app, f + 0x34),
                  nfs::u64(at(app, f + 0x28)) << 32 | at(app, f + 0x24), rest);
        put(app, f + 0x38, u32(q >> 32));
        put(app, f + 0x34, u32(q));
        put(app, f + 0x30, u32(rest >> 32));
        put(app, f + 0x2c, u32(rest));
        putByte(app, p++, byte(app, at(app, f + 0x2c) + kDigits64));
    } while (q != 0);
    u8 c;
    do
    {
        const u32 to = at(app, f + 0x40);
        c = byte(app, --p);
        putByte(app, to, c);
        put(app, f + 0x40, to + 1);
    } while (c != 0);
    return at(app, f + 0x3c);
}

nfs::u32 ulltoa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, ulltoaOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x51e220 (__prtf's utoa): `value` in `base`, its digits from kDigitsLow, into `out`; `out`
 * back. */
u32 prtfUtoaOf(nfs::App* app, nfs::ThreadState& ts, u32 value, u32 out, u32 base)
{
    return digitsOf(app, ts, value, base, out, kDigitsLow, true);
}

nfs::u32 prtfUtoa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfUtoaOf(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fefdc (__prtf's strupr): the string at `p` in capitals, by toupper.  What the code leaves in
 * eax: toupper's last, `p` for an empty string. */
u32 prtfUpperOf(nfs::App* app, nfs::ThreadState& ts, u32 p)
{
    NFS2_USE(ts);  // ebx and edx pushed: their room only
    u32 last = p;
    if (byte(app, p) != 0)
        do
        {
            last = clib::toupper(app, byte(app, p));
            putByte(app, p, u8(last));
        } while (byte(app, ++p) != 0);
    return last;
}

nfs::u32 prtfUpper(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfUpperOf(app, ts, args.eax));
    return args.eax;
}

/* 0x4fe734 (__prtf's strnlen): the length of the string at `p` in the segment `segment`, which the
 * code loads into es, at most `limit` (-1 for any).  Its pushes: their room only. */
u32 prtfLengthOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 p, u32 segment,
                   u32 limit)
{
    NFS2_USE(ts);
    const u32 base = s.base(segment);
    u32 length = 0;
    while (byte(app, base + p++) != 0 && length != limit)
        ++length;
    return length;
}

nfs::u32 prtfLength(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfLengthOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fe758 (__prtf's wcsnlen in bytes): how many bytes the wide string at `p` in the segment
 * `segment` is as wctomb makes it, at most `limit` of them (-1 for any).  At its entry's depth: ecx, esi and es pushed, then the
 * character's bytes.  0 with ts.terminate set when the game stopped. */
u32 prtfWideLengthOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 p, u32 segment,
                       u32 limit)
{
    const GuestFrame frame(ts, 0xc, 4);
    const u32 base = s.base(segment);
    const u32 bytes = frame - 4;
    auto over = [&](u32 count) { return limit != 0xffffffff && nfs::s32(count) > nfs::s32(limit); };
    u32 count = 0;
    for (;;)
    {
        const u16 wc = half(app, base + p);
        if (wc == 0 || over(count))
            break;
        p += 2;
        u32 n;
        {
            const ReturnSlot slot(ts);
            n = wctombOf(app, ts, bytes, wc);
        }
        if (ts.terminate)
            return 0;
        if (n != 0xffffffff)
            count += n;
    }
    return over(count) ? limit : count;
}

nfs::u32 prtfWideLength(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = prtfWideLengthOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* 0x4fe7c4 (__prtf's pointer's digits): `value` in hexadecimal, right-aligned in `width` digits
 * with zeros before, into `out`.  At its entry's depth: four pushes, then the width.  What the code
 * leaves in eax: the count of zeros it ran down, -1. */
u32 prtfHexOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 value, u32 width, u32 out)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 kept = frame - 4;
    put(app, kept, width);
    {
        const ReturnSlot slot(ts);
        itoaOf(app, ts, value, out, 0x10);
    }
    // repne scasb from it, es its ds, as the direction flag has it -- es pushed meanwhile (its room).
    u32 count = 0xffffffff, scan = out;
    u8 scanned;
    do
    {
        scanned = byte(app, s.base(s.ds) + scan);
        scan += s.df ? u32(-1) : 1;
        --count;
    } while (scanned != 0 && count != 0);
    const u32 length = ~count - 1;
    u32 left = width - 1;
    if (length != 0)
    {
        u32 from = out + length, to = left + out;
        do
        {
            --to;
            putByte(app, to + 1, byte(app, --from));
            --left;
        } while (from != out);
    }
    for (u32 p = left + out; nfs::s32(left) >= 0; --p)
    {
        --left;
        putByte(app, p, '0');
    }
    putByte(app, out + at(app, kept), 0);
    return left;
}

nfs::u32 prtfHex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfHexOf(app, ts, segmentsOf(ts), args.eax, args.ebx, args.edx));
    return args.eax;
}

/* 0x4fe81e: the end of __prtf's %p digits (0x4fe7c4), which its %hf
 * (0x4fe828) jumps to as well: the frame's dword let go and the registers
 * the two keep -- ebp, edi, esi, ecx -- taken back off the stack. */
nfs::u32 prtfHexEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    args.ebp = at(app, ts.sp);
    args.edi = at(app, ts.sp + 4);
    args.esi = at(app, ts.sp + 8);
    args.ecx = at(app, ts.sp + 0xc);
    ts.sp += 0x10 + 4;
    return args.eax;
}

/* 0x4fe828 (__prtf's %hf): `value` a 16.16 fixed-point number, into `out` -- its whole part by
 * itoa, then the precision's digits (4 for none), rounded half up.  At its entry's depth: four
 * pushes, then the number.  What the code leaves in eax: where its digits went, past a '-'. */
u32 prtfFixedOf(nfs::App* app, nfs::ThreadState& ts, u32 out, u32 value, u32 specs)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 local = frame - 4;
    u32 p = out;
    put(app, local, value);
    if (nfs::s32(value) < 0)
    {
        putByte(app, p, '-');
        ++p;
        put(app, local, 0 - value);
    }
    if (at(app, specs + kPrec) == 0xffffffff)
        put(app, specs + kPrec, 4);
    {
        const ReturnSlot slot(ts);
        itoaOf(app, ts, half(app, local + 2), p, 10);
    }
    const u32 start = p;
    if (byte(app, p) != 0)
        while (byte(app, ++p) != 0)
        {
        }
    if (at(app, specs + kPrec) != 0)
    {
        putByte(app, p++, '.');
        for (u32 i = 0; nfs::s32(i) < nfs::s32(at(app, specs + kPrec)); ++i)
        {
            putHalf(app, local + 2, 0);
            put(app, local, at(app, local) * 10);
            putByte(app, p++, u8(byte(app, local + 2) + '0'));
        }
        putByte(app, p, 0);
    }
    if (byte(app, local + 1) & 0x80)
    {
        // Rounded up: a 9 to a 0 and the digit before up, to a 1 before them all, which moves the
        // point and the rest one on.
        for (;;)
        {
            if (p == start)
            {
                p = start + 1;
                putByte(app, start, '1');
                while (byte(app, p) == '0')
                    ++p;
                const u8 point = byte(app, p);
                if (point == '.')
                {
                    putByte(app, p, '0');
                    ++p;
                    putByte(app, p, point);
                    if (byte(app, ++p) == '0')
                        while (byte(app, ++p) == '0')
                        {
                        }
                }
                putByte(app, p, '0');
                putByte(app, p + 1, 0);
                break;
            }
            if (byte(app, --p) == '.')
                --p;
            const u8 digit = byte(app, p);
            if (digit != '9')
            {
                putByte(app, p, u8(digit + 1));
                break;
            }
            putByte(app, p, '0');
        }
    }
    return start;
}

nfs::u32 prtfFixed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfFixedOf(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fe924 (__prtf's float): [__EFG_printf] with eax the buffer, edx the arguments' pointer, ebx
 * the SPECS -- the start-up code's formatter (watc::formatFloat, by its hook: watcom_hooks.h),
 * called with the registers it takes; what it leaves in eax, 0 with ts.terminate set when the
 * game stopped. */
u32 prtfFloatOf(nfs::App* app, nfs::ThreadState& ts, u32 buffer, u32 argp, u32 specs)
{
    const EspKept kept(ts);
    return hook<kFloatFormat>(app, ts, {buffer, argp, specs}).value_or(0);
}

nfs::u32 prtfFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!hookWith<kFloatFormat>(app, ts, args))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x4fe92c (__prtf's zero padding): with '0' padding and not '-', what the width leaves over the
 * parts of the SPECS at `specs` as zeros after the first. */
void prtfZerosOf(nfs::App* app, nfs::ThreadState& ts, u32 specs)
{
    NFS2_USE(ts);  // six pushes: their room only
    if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == '0')
    {
        const u32 nz0 = at(app, specs + kNZ0);
        const u32 left = at(app, specs + kWidth) - at(app, specs + kN0) - nz0 - at(app, specs + kN1)
                           - at(app, specs + kNZ1) - at(app, specs + kN2) - at(app, specs + kNZ2);
        if (nfs::s32(left) > 0)
            put(app, specs + kNZ0, nz0 + left);
    }
}

nfs::u32 prtfZeros(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    prtfZerosOf(app, ts, args.eax);  // eax left as it was, the SPECS
    ts.sp += 4;
    return args.eax;
}

/* 0x4fe6cc (__prtf's flags): the flags at `p` into the SPECS at `specs`; past them. */
u32 prtfFlagsOf(nfs::App* app, nfs::ThreadState& ts, u32 p, u32 specs)
{
    NFS2_USE(ts);  // ebx and ecx pushed: their room only
    putHalf(app, specs + kFlags, 0);
    for (;; ++p)
    {
        const u8 c = byte(app, p);
        const u8 flags = byte(app, specs + kFlags);
        if (c == '-')
            putByte(app, specs + kFlags, flags | kLeft);
        else if (c == '#')
            putByte(app, specs + kFlags, flags | kAlternate);
        else if (c == '+')
            putByte(app, specs + kFlags, u8((flags | kPlus) & ~kBlank));
        else if (c == ' ')
        {
            if (!(flags & kPlus))
                putByte(app, specs + kFlags, flags | kBlank);
        }
        else if (c == '0')
            putByte(app, specs + kPadChar, c);
        else
            break;
    }
    return p;
}

nfs::u32 prtfFlags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, prtfFlagsOf(app, ts, args.eax, args.edx));
    return args.eax;
}

/* 0x4fe578 (__prtf's specification): what follows a '%' at `format` -- flags, width, precision,
 * size -- into the SPECS at `specs`, a '*' taken from the arguments' pointer at `argp`; past it.
 * `edx` what the code leaves there: the size's character in its low byte. */
u32 prtfSpecOf(nfs::App* app, nfs::ThreadState& ts, u32 format, u32 argp, u32 specs, u32& edx)
{
    const GuestFrame frame(ts, 0xc, 0);
    putByte(app, specs + kPadChar, ' ');
    u32 p;
    {
        const ReturnSlot slot(ts);
        p = prtfFlagsOf(app, ts, format, specs);
    }
    edx = specs;
    u32 moved;
    auto digits = [&](u32 field) {
        for (;;)
        {
            const u8 c = byte(app, p);
            edx = (edx & 0xffffff00) | c;
            if (c < '0' || c > '9')
                break;
            edx = c - '0';
            put(app, specs + field, at(app, specs + field) * 10 + edx);
            ++p;
        }
    };
    put(app, specs + kWidth, 0);
    if (byte(app, p) == '*')
    {
        edx = nextArgument(app, argp, 4, moved);
        put(app, specs + kWidth, edx);
        if (nfs::s32(edx) < 0)
        {
            put(app, specs + kWidth, 0 - edx);
            putByte(app, specs + kFlags, byte(app, specs + kFlags) | kLeft);
        }
        ++p;
    }
    else
        digits(kWidth);
    put(app, specs + kPrec, 0xffffffff);
    if (byte(app, p) == '.')
    {
        put(app, specs + kPrec, 0);
        ++p;
        if (byte(app, p) == '*')
        {
            edx = nextArgument(app, argp, 4, moved);
            put(app, specs + kPrec, edx);
            if (nfs::s32(edx) < 0)
                put(app, specs + kPrec, 0xffffffff);
            ++p;
        }
        else
            digits(kPrec);
        if (at(app, specs + kPrec) != 0xffffffff)
            putByte(app, specs + kPadChar, ' ');
    }
    const u8 c = byte(app, p);
    edx = (edx & 0xffffff00) | c;
    switch (c)
    {
    case 'N':
        putByte(app, specs + kFlags, byte(app, specs + kFlags) | kNear);
        ++p;
        break;
    case 'l':
    case 'w':
        putByte(app, specs + kFlags, byte(app, specs + kFlags) | kLong);
        ++p;
        break;
    case 'h':
        putByte(app, specs + kFlags, byte(app, specs + kFlags) | kShort);
        ++p;
        break;
    case 'I':
        if (byte(app, p + 1) == '6' && byte(app, p + 2) == '4')
        {
            putByte(app, specs + kFlags2, byte(app, specs + kFlags2) | kLongLong);
            p += 3;
        }
        break;
    case 'L':
        putByte(app, specs + kFlags2, byte(app, specs + kFlags2) | kLongLong);
        ++p;
        break;
    case 'F':
        putByte(app, specs + kFlags, byte(app, specs + kFlags) | kFar);
        ++p;
        break;
    default:
        break;
    }
    return p;
}

nfs::u32 prtfSpec(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 edx;
    const u32 p = prtfSpecOf(app, ts, args.eax, args.edx, args.ebx, edx);
    args.edx = edx;  // the size's character, as the code leaves it
    returns(ts, args, p);
    return args.eax;
}

/* 0x4fe970 (__prtf's %ls): the wide string at `p` in the segment `segment` out by the function at
 * `out`, as wctomb makes
 * each character, as many bytes as the SPECS at `specs` left for it.  At its entry's depth: esi,
 * edi, es and ebp pushed (their room only), then the character's bytes.  What the code leaves in
 * eax; 0 with ts.terminate set when the game stopped. */
u32 prtfWideOutOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 p, u32 segment,
                    u32 specs, u32 out)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 base = s.base(segment);
    const u32 bytes = frame - 4;
    u32 eax = p;
    if (nfs::s32(at(app, specs + kN1)) <= 0)
        return eax;
    for (;;)
    {
        const u16 wc = half(app, base + p);
        p += 2;
        u32 n;
        {
            const ReturnSlot slot(ts);
            n = wctombOf(app, ts, bytes, wc);
        }
        if (ts.terminate)
            return 0;
        eax = n;
        if (n != 0xffffffff)
        {
            if (nfs::s32(n) > nfs::s32(at(app, specs + kN1)))
            {
                put(app, specs + kN1, 0);
                break;
            }
            for (u32 from = bytes; --n != 0xffffffff; ++from)
            {
                {
                    // The caller's function: the SPECS in eax, the byte in edx.
                    const EspKept kept(ts);
                    if (!through(app, ts, out, {specs, byte(app, from)}))
                        return 0;
                }
                eax = at(app, specs + kN1) - 1;
                put(app, specs + kN1, eax);
            }
        }
        if (nfs::s32(at(app, specs + kN1)) <= 0)
            break;
    }
    return eax;
}

nfs::u32 prtfWideOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = prtfWideOutOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx, args.ecx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x4fe9d4 (__prtf's conversion): the argument the SPECS at `specs`'s conversion takes, by the
 * arguments' pointer at `argp`, made into the buffer at `buffer` -- a prefix (sign, 0x, 0) and the
 * parts' lengths in the SPECS; the string to write, its segment into `segment`.  At its entry's
 * depth: esi, edi, es and ebp pushed (their room only), then 0x10 bytes, the integer's argument and
 * wctomb's bytes.  0 with ts.terminate set when the game stopped. */
u32 prtfConvertOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 buffer, u32 argp,
                    u32 specs, u16& segment)
{
    const GuestFrame frame(ts, 0x10, 0x10);
    const u32 local = frame - 0x10;
    u32 moved;
    segment = s.ds;
    for (const u32 field : {kN0, kNZ0, kN1, kNZ1, kN2})
        put(app, specs + field, 0);
    u32 string = buffer;
    put(app, specs + kNZ2, 0);

    auto prefix = [&](u8 c) {
        const u32 n = at(app, specs + kN0);
        put(app, specs + kN0, n + 1);
        putByte(app, buffer + n, c);
    };
    // Each of the others' cores at this one's depth.
    auto length = [&](u32 from, u32 limit) {
        const ReturnSlot slot(ts);
        return prtfLengthOf(app, ts, s, from, s.ds, limit);
    };

    // The integer's argument: 64 bits at local, 32 at local + 8.
    const u8 character = byte(app, specs + kCharacter);
    if (character == 'u' || character == 'x' || character == 'o' || character == 'X')
    {
        if (byte(app, specs + kFlags2) & kLongLong)
        {
            put(app, local, nextArgument(app, argp, 4, moved));
            put(app, local + 4, nextArgument(app, argp, 4, moved));
        }
        else if (byte(app, specs + kFlags) & kLong)
            put(app, local + 8, nextArgument(app, argp, 4, moved));
        else
        {
            put(app, local + 8, nextArgument(app, argp, 4, moved));
            if (byte(app, specs + kFlags) & kShort)
                put(app, local + 8, half(app, local + 8));
        }
    }
    else if (character == 'i' || character == 'd')
    {
        if (byte(app, specs + kFlags2) & kLongLong)
        {
            put(app, local, nextArgument(app, argp, 4, moved));
            put(app, local + 4, nextArgument(app, argp, 4, moved));
        }
        else if (byte(app, specs + kFlags) & kLong)
            put(app, local + 8, nextArgument(app, argp, 4, moved));
        else
        {
            put(app, local + 8, nextArgument(app, argp, 4, moved));
            if (byte(app, specs + kFlags) & kShort)
                put(app, local + 8, u32(nfs::s32(nfs::s16(half(app, local + 8)))));
        }
        const bool negative = byte(app, specs + kFlags2) & kLongLong ? (byte(app, local + 7) & 0x80) != 0
                                                                     : nfs::s32(at(app, local + 8)) < 0;
        if (negative)
        {
            prefix('-');
            if (byte(app, specs + kFlags2) & kLongLong)
            {
                const u32 low = ~at(app, local), high = ~at(app, local + 4);
                put(app, local, low + 1);
                put(app, local + 4, low + 1 == 0 ? high + 1 : high);
            }
            else
                put(app, local + 8, 0 - at(app, local + 8));
        }
        else if (byte(app, specs + kFlags) & kPlus)
            prefix('+');
        else if (byte(app, specs + kFlags) & kBlank)
            prefix(' ');
    }

    u32 base = 10;
    u32 result;
    switch (byte(app, specs + kCharacter))
    {
    case 'f':
    case 'F':
        if (byte(app, specs + kFlags) & kShort)
        {
            const u32 value = nextArgument(app, argp, 4, moved);
            put(app, local + 8, value);
            {
                const ReturnSlot slot(ts);
                prtfFixedOf(app, ts, buffer, value, specs);
            }
            put(app, specs + kN1, length(buffer, 0xffffffff));
            return string;
        }
        [[fallthrough]];
    case 'e':
    case 'E':
    case 'g':
    case 'G':
        {
            const ReturnSlot slot(ts);
            prtfFloatOf(app, ts, buffer, argp, specs);
        }
        if (ts.terminate)
            return 0;
        {
            const ReturnSlot slot(ts);
            prtfZerosOf(app, ts, specs);
        }
        segment = s.ds;
        return buffer + 1;
    case 's':
    case 'S':
    {
        putByte(app, buffer, 0);
        const u8 flags = byte(app, specs + kFlags);
        if (flags & kFar)
        {
            const u32 offset = nextArgument(app, argp, 8, moved);
            const u16 far = half(app, moved - 4);
            if (offset != 0 || far != 0)
            {
                segment = far;
                string = offset;
            }
        }
        else
        {
            const u32 pointer = nextArgument(app, argp, 4, moved);
            if (pointer != 0)
            {
                segment = s.ds;
                string = pointer;
            }
        }
        const bool wide = byte(app, specs + kCharacter) == 'S' ? !(byte(app, specs + kFlags) & kShort)
                                                               : (byte(app, specs + kFlags) & kLong) != 0;
        u32 n;
        {
            const ReturnSlot slot(ts);
            n = wide ? prtfWideLengthOf(app, ts, s, string, segment, at(app, specs + kPrec))
                     : prtfLengthOf(app, ts, s, string, segment, at(app, specs + kPrec));
        }
        if (ts.terminate)
            return 0;
        const u32 precision = at(app, specs + kPrec);
        put(app, specs + kN1, n);
        if (nfs::s32(precision) >= 0 && nfs::s32(n) > nfs::s32(precision))
            put(app, specs + kN1, precision);
        return string;
    }
    case 'x':
    case 'X':
        if (byte(app, specs + kFlags) & kAlternate)
        {
            const bool nonZero = byte(app, specs + kFlags2) & kLongLong
                                     ? at(app, local) != 0 || at(app, local + 4) != 0
                                     : at(app, local + 8) != 0;
            if (nonZero)
            {
                prefix('0');
                prefix(byte(app, specs + kCharacter));
            }
        }
        base = 16;
        break;
    case 'o':
        base = 8;
        if (byte(app, specs + kFlags) & kAlternate)
            prefix('0');
        break;
    case 'd':
    case 'i':
    case 'u':
        break;
    case 'p':
    case 'P':
    {
        if (at(app, specs + kWidth) == 0)
            put(app, specs + kWidth, byte(app, specs + kFlags) & kFar ? 13 : 8);
        putByte(app, specs + kFlags, byte(app, specs + kFlags) & 0xf9);
        const u32 offset = nextArgument(app, argp, 4, moved);
        u32 to = buffer;
        if (byte(app, specs + kFlags) & kFar)
        {
            const u32 far = nextArgument(app, argp, 4, moved) & 0xffff;
            {
                const ReturnSlot slot(ts);
                prtfHexOf(app, ts, s, far, 4, buffer);
            }
            putByte(app, buffer + 4, ':');
            to = buffer + 5;
        }
        {
            const ReturnSlot slot(ts);
            prtfHexOf(app, ts, s, offset, 8, to);
        }
        if (byte(app, specs + kCharacter) == 'P')
        {
            const ReturnSlot slot(ts);
            prtfUpperOf(app, ts, buffer);
        }
        put(app, specs + kN0, length(string, 0xffffffff));
        return string;
    }
    case 'c':
    {
        put(app, specs + kN0, 1);
        if (byte(app, specs + kFlags) & kLong)
        {
            const u32 wc = nextArgument(app, argp, 4, moved) & 0xffff;
            u32 n;
            {
                const ReturnSlot slot(ts);
                n = wctombOf(app, ts, local + 0xc, wc);
            }
            if (ts.terminate)
                return 0;
            if (n == 0xffffffff)
                return string;
            putByte(app, buffer, byte(app, local + 0xc));
            if (at(app, kMbcsOn) == 0 || !(byte(app, kMbcsLead + byte(app, local + 0xc)) & 1))
                return string;
            putByte(app, buffer + 1, byte(app, local + 0xd));
            put(app, specs + kN0, at(app, specs + kN0) + 1);
            return string;
        }
        putByte(app, buffer, u8(nextArgument(app, argp, 4, moved)));
        return string;
    }
    case 'C':
    {
        const u32 wc = nextArgument(app, argp, 4, moved) & 0xffff;
        u32 n;
        {
            const ReturnSlot slot(ts);
            n = wctombOf(app, ts, buffer, wc);
        }
        if (ts.terminate)
            return 0;
        put(app, specs + kN0, n == 0xffffffff ? 0 : n);
        return string;
    }
    default:
        put(app, specs + kWidth, 0);
        putByte(app, buffer, byte(app, specs + kCharacter));
        put(app, specs + kN0, 1);
        return string;
    }

    // The integer's digits after the prefix, by utoa or ulltoa: none for 0
    // with a precision of 0.
    segment = s.ds;
    string = buffer + at(app, specs + kN0);
    if (byte(app, specs + kFlags2) & kLongLong)
    {
        if (at(app, specs + kPrec) == 0 && at(app, local) == 0 && at(app, local + 4) == 0)
        {
            putByte(app, s.base(s.ds) + string, 0);
            result = 0;
        }
        else
        {
            {
                const ReturnSlot slot(ts);
                ulltoaOf(app, ts, s, local, at(app, specs + kN0) + buffer, base);
            }
            if (byte(app, specs + kCharacter) == 'X')
            {
                const ReturnSlot slot(ts);
                prtfUpperOf(app, ts, buffer);
            }
            result = length(string, 0xffffffff);
        }
    }
    else if (at(app, specs + kPrec) == 0 && at(app, local + 8) == 0)
    {
        putByte(app, s.base(s.ds) + string, 0);
        result = 0;
    }
    else
    {
        {
            const ReturnSlot slot(ts);
            prtfUtoaOf(app, ts, at(app, local + 8), at(app, specs + kN0) + buffer, base);
        }
        if (byte(app, specs + kCharacter) == 'X')
        {
            const ReturnSlot slot(ts);
            prtfUpperOf(app, ts, buffer);
        }
        result = length(string, 0xffffffff);
    }
    put(app, specs + kN1, result);
    const u32 precision = at(app, specs + kPrec);
    if (nfs::s32(result) < nfs::s32(precision))
        put(app, specs + kNZ0, precision - result);
    if (at(app, specs + kPrec) == 0xffffffff)
    {
        const ReturnSlot slot(ts);
        prtfZerosOf(app, ts, specs);
    }
    return string;
}

nfs::u32 prtfConvert(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u16 segment;
    const u32 string = prtfConvertOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx, segment);
    if (ts.terminate)
        return args.eax;
    args.edx = segment;
    returns(ts, args, string);
    return args.eax;
}

/* __prtf (0x4fe1d0): the format at `format` with the arguments' pointer at `argp`, each character
 * it makes handed to the function at `out` with eax its SPECS, whose first dword is `dest`; how
 * many went.  At its entry's depth: esi, edi, es and ebp pushed, then the SPECS' 0x70 bytes, which
 * the functions it calls are handed (the pushes: their room only).  0 with ts.terminate set when
 * the game stopped. */
u32 prtfOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 dest, u32 format, u32 argp,
             u32 out)
{
    constexpr u32 kArgCopy = 0x60, kArgCopy2 = 0x64, kFormat = 0x68, kBuffer = 0x38;
    constexpr u32 kNullChar = 0x6c;
    const GuestFrame frame(ts, 0x10, 0x70);
    const u32 specs = frame - 0x70;
    u16 es = s.es;  // the last conversion's string's segment, es as the code loads it
    putByte(app, specs + kNullChar, 0);
    putHalf(app, specs + kFlags, 0);
    put(app, specs + kDest, dest);
    putHalf(app, specs + kSpecs1C, 100);
    put(app, specs + kOutCount, 0);
    put(app, specs + kFormat, format);

    // A character to the caller's function, the SPECS in eax, the character in edx.
    auto put1 = [&](u32 c) {
        const EspKept kept(ts);
        return through(app, ts, out, {specs, c}).has_value();
    };
    // A field's count of a character, or of the bytes at `from` on.
    auto repeat = [&](u32 field, u32 c) {
        while (nfs::s32(at(app, specs + field)) > 0)
        {
            if (!put1(c))
                return false;
            put(app, specs + field, at(app, specs + field) - 1);
        }
        return true;
    };
    auto bytes = [&](u32 field, u32 base, u32& from) {
        while (nfs::s32(at(app, specs + field)) > 0)
        {
            if (!put1(byte(app, base + from)))
                return false;
            put(app, specs + field, at(app, specs + field) - 1);
            ++from;
        }
        return true;
    };

    if (byte(app, format) != 0)
        do
        {
            const u32 at_ = at(app, specs + kFormat);
            const u8 c = byte(app, at_);
            if (c != '%')
            {
                put(app, specs + kFormat, at_ + 1);
                if (!put1(c))
                    return 0;
                continue;
            }
            put(app, specs + kArgCopy, at(app, argp));
            u32 edx, next;
            {
                const ReturnSlot slot(ts);
                next = prtfSpecOf(app, ts, at_ + 1, specs + kArgCopy, specs, edx) + 1;
            }
            put(app, argp, at(app, specs + kArgCopy));
            const u8 conversion = byte(app, next - 1);
            put(app, specs + kFormat, next);
            putByte(app, specs + kCharacter, conversion);
            if (conversion == 0)
                break;
            if (conversion == 'n')
            {
                // How many went so far, into the argument: a short for h,
                // a far pointer as the game's code has it.
                const u8 flags = byte(app, specs + kFlags);
                const u32 count = at(app, specs + kOutCount);
                u32 moved, target;
                const bool word = !(flags & kLong) && (flags & kShort);
                if (flags & kFar)
                {
                    nextArgument(app, argp, 8, moved);
                    NFS2_ASSERT(false);  // les
                    target = flags & kLong ? moved : edx;
                }
                else
                    target = nextArgument(app, argp, 4, moved);
                if (flags & kFar)
                    target += s.base(es);
                if (word)
                    putHalf(app, target, u16(count));
                else
                    put(app, target, count);
                continue;
            }

            put(app, specs + kArgCopy2, at(app, argp));
            u32 string;
            {
                const ReturnSlot slot(ts);
                string = prtfConvertOf(app, ts, s, specs + kBuffer, specs + kArgCopy2, specs, es);
            }
            if (ts.terminate)
                return 0;
            put(app, argp, at(app, specs + kArgCopy2));
            const u32 parts = at(app, specs + kN0) + at(app, specs + kNZ0) + at(app, specs + kN1)
                                + at(app, specs + kNZ1) + at(app, specs + kN2) + at(app, specs + kNZ2);
            put(app, specs + kWidth, at(app, specs + kWidth) - parts);
            if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == ' '
                && !repeat(kWidth, ' '))
                return 0;
            u32 prefix = specs + kBuffer;
            if (!bytes(kN0, 0, prefix) || !repeat(kNZ0, '0'))
                return 0;
            const u8 kind = byte(app, specs + kCharacter);
            if ((kind == 's' && (byte(app, specs + kFlags) & kLong)) || kind == 'S')
            {
                const ReturnSlot slot(ts);
                prtfWideOutOf(app, ts, s, string, es, specs, out);
                if (ts.terminate)
                    return 0;
            }
            else if (!bytes(kN1, s.base(es), string))
                return 0;
            if (!repeat(kNZ1, '0') || !bytes(kN2, s.base(es), string) || !repeat(kNZ2, '0'))
                return 0;
            if ((byte(app, specs + kFlags) & kLeft) && !repeat(kWidth, ' '))
                return 0;
        } while (byte(app, at(app, specs + kFormat)) != 0);
    return at(app, specs + kOutCount);
}

nfs::u32 prtf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 count = prtfOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx, args.ecx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, count);
    return args.eax;
}

/* mem_putc (0x4df660): sprintf's: the character `c` at the SPECS at `specs`'s destination, which
 * moves on, and counted. */
void memPutcOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 c)
{
    NFS2_USE(ts);  // ebx and ecx pushed: their room only
    const u32 p = at(app, specs);
    put(app, specs, p + 1);
    putByte(app, p, u8(c));
    put(app, specs + kOutCount, at(app, specs + kOutCount) + 1);
}

nfs::u32 mem_putc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    memPutcOf(app, ts, args.eax, args.edx);  // eax left as it was, the SPECS
    ts.sp += 4;
    return args.eax;
}

/* file_putc (0x4ff000): vfprintf's: the character `c` to the stream in the SPECS at `specs` by
 * fputc, and counted.  At its entry's depth: ebx pushed.  fputc's say; 0 with ts.terminate set
 * when the game stopped. */
u32 filePutcOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 c)
{
    const GuestFrame frame(ts, 4, 0);
    u32 result;
    {
        const ReturnSlot slot(ts);
        const EspKept kept(ts);
        if (!putChar(app, ts, c, at(app, specs + kDest), result))
            return 0;
    }
    put(app, specs + kOutCount, at(app, specs + kOutCount) + 1);
    return result;
}

nfs::u32 file_putc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = filePutcOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* sprintf's and vsprintf's text made on the host (plan step 7): what __prtf above makes of a
 * format and its arguments, as C++ on the host's strings -- no SPECS, no frames on the guest's
 * stack, no call of mem_putc a character -- and then written into the buffer at once.  __prtf's
 * own rules, which are not C's: '0' pads only an integer with no precision ("%05s" and "%05c" are
 * not padded at all), "%#o" of 0 is "00", a precision drops the '0' flag, an unknown conversion
 * is its character with the width dropped, "%" at the end stops the text.
 *
 * The conversions done here: d, i, u, x, X, o with any flags, width, precision and size (h, l, w,
 * N, L, I64), s of bytes, c of a byte, and the unknown ones.  Anything else -- e, f, g (the x87's
 * formatter and %hf), n, p, the wide ones (S, C, %ls, %lc) and far pointers -- leaves the text to
 * __prtf, as does a text whose buffer overlaps anything it reads (the format, a string, an
 * argument, the va_list), which __prtf's character-at-a-time writes would see; a text past
 * kHostTextMost; es over ds not at 0, where __prtf reads its digits through es; and a 64-bit
 * integer under the direction flag, which ulltoa's movsd read backwards.  So the
 * result is __prtf's to the byte, each case taking whichever way it falls to. */
namespace
{

constexpr std::size_t kHostTextMost = 0x10000;

/* The ranges of guest memory a text read. */
class Reads
{
public:
    void add(u32 from, std::uint64_t length) { ranges_.emplace_back(from, std::uint64_t(from) + length); }
    bool overlap(u32 from, std::uint64_t length) const
    {
        const std::uint64_t to = std::uint64_t(from) + length;
        for (const auto& r : ranges_)
            if (from < r.second && r.first < to)
                return true;
        return false;
    }

private:
    std::vector<std::pair<std::uint64_t, std::uint64_t>> ranges_;
};

/* A format's arguments: the dwords at the va_list's pointer in guest memory on, or the words a
 * typed caller handed (guest::Words) -- false past the last of those, where __prtf would read the
 * guest's stack on. */
class FormatArguments
{
public:
    FormatArguments(nfs::App* app, u32 ap, Reads& reads) : app_(app), ap_(ap), reads_(reads) {}
    FormatArguments(const guest::Words& words, Reads& reads) : words_(&words), reads_(reads) {}

    bool take(u32& value)
    {
        if (words_ != nullptr)
        {
            if (next_ >= words_->count)
                return false;
            value = words_->word[next_++];
            return true;
        }
        reads_.add(ap_, 4);
        value = at(app_, ap_);
        ap_ += 4;
        return true;
    }
    u32 pointer() const { return ap_; }

private:
    nfs::App* app_ = nullptr;
    u32 ap_ = 0;
    const guest::Words* words_ = nullptr;
    unsigned next_ = 0;
    Reads& reads_;
};

/* An unsigned number's digits, most significant first, from the runtime's table at `digits` (as
 * utoa and ulltoa take them). */
std::string hostDigits(nfs::App* app, std::uint64_t value, u32 base, u32 digits, Reads& reads)
{
    reads.add(digits, 36);
    char text[72];
    int n = 0;
    do
    {
        text[n++] = char(byte(app, digits + u32(value % base)));
        value /= base;
    } while (value != 0);
    return std::string(std::reverse_iterator<char*>(text + n), std::reverse_iterator<char*>(text));
}

/* The text __prtf makes of the format at `format` with `args`; nullopt where it is left to __prtf
 * (above). */
std::optional<std::string> hostFormatText(nfs::App* app, const Segments& s, u32 format,
                                          FormatArguments& args, Reads& reads)
{
    if (s.base(s.ds) != 0)
        return std::nullopt;
    std::string text;
    u32 p = format;
    if (byte(app, p) != 0)
        do
        {
            const u8 c = byte(app, p);
            if (c != '%')
            {
                text += char(c);
                ++p;
                continue;
            }
            // The specification: flags, width, precision, size (prtfSpecOf).
            u32 q = p + 1;
            u8 flags = 0, flags2 = 0;
            u8 pad = ' ';
            for (;; ++q)
            {
                const u8 f = byte(app, q);
                if (f == '-')
                    flags |= kLeft;
                else if (f == '#')
                    flags |= kAlternate;
                else if (f == '+')
                    flags = u8((flags | kPlus) & ~kBlank);
                else if (f == ' ')
                {
                    if (!(flags & kPlus))
                        flags |= kBlank;
                }
                else if (f == '0')
                    pad = '0';
                else
                    break;
            }
            auto digits = [&](u32& field) {
                for (u8 d = byte(app, q); d >= '0' && d <= '9'; d = byte(app, ++q))
                    field = field * 10 + (d - '0');
            };
            u32 width = 0;
            if (byte(app, q) == '*')
            {
                if (!args.take(width))
                    return std::nullopt;
                if (nfs::s32(width) < 0)
                {
                    width = 0 - width;
                    flags |= kLeft;
                }
                ++q;
            }
            else
                digits(width);
            u32 precision = 0xffffffff;
            if (byte(app, q) == '.')
            {
                precision = 0;
                ++q;
                if (byte(app, q) == '*')
                {
                    if (!args.take(precision))
                        return std::nullopt;
                    if (nfs::s32(precision) < 0)
                        precision = 0xffffffff;
                    ++q;
                }
                else
                    digits(precision);
                if (precision != 0xffffffff)
                    pad = ' ';
            }
            switch (byte(app, q))
            {
            case 'N':
                flags |= kNear;
                ++q;
                break;
            case 'l':
            case 'w':
                flags |= kLong;
                ++q;
                break;
            case 'h':
                flags |= kShort;
                ++q;
                break;
            case 'I':
                if (byte(app, q + 1) == '6' && byte(app, q + 2) == '4')
                {
                    flags2 |= kLongLong;
                    q += 3;
                }
                break;
            case 'L':
                flags2 |= kLongLong;
                ++q;
                break;
            case 'F':
                flags |= kFar;
                ++q;
                break;
            default:
                break;
            }
            const u8 conversion = byte(app, q);
            p = q + 1;
            if (conversion == 0)
                break;

            // The conversion (prtfConvertOf): a prefix, the leading zeros, the string.
            std::string prefix;
            u32 zeros = 0;          // NZ0
            u32 stringAt = 0;       // the string's bytes, in guest memory, or
            std::string digitsText;   // the integer's digits
            bool fromGuest = false;
            u32 length = 0;         // N1
            switch (conversion)
            {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            {
                std::uint64_t value;
                u32 low;
                if (!args.take(low))
                    return std::nullopt;
                const bool longLong = flags2 & kLongLong;
                // ulltoa takes the 64 bits by two movsd: with the direction flag set its high
                // dword is what its frame held.
                if (longLong && s.df)
                    return std::nullopt;
                const bool isSigned = conversion == 'd' || conversion == 'i';
                if (longLong)
                {
                    u32 high;
                    if (!args.take(high))
                        return std::nullopt;
                    value = std::uint64_t(high) << 32 | low;
                }
                else if (!(flags & kLong) && (flags & kShort))
                    value = isSigned ? u32(nfs::s32(nfs::s16(u16(low)))) : u16(low);
                else
                    value = low;
                if (isSigned)
                {
                    const bool negative = longLong ? (value >> 63) != 0 : nfs::s32(u32(value)) < 0;
                    if (negative)
                    {
                        prefix += '-';
                        value = longLong ? 0 - value : std::uint64_t(u32(0 - u32(value)));
                    }
                    else if (flags & kPlus)
                        prefix += '+';
                    else if (flags & kBlank)
                        prefix += ' ';
                }
                u32 base = 10;
                if (conversion == 'x' || conversion == 'X')
                {
                    if ((flags & kAlternate) && value != 0)
                    {
                        prefix += '0';
                        prefix += char(conversion);
                    }
                    base = 16;
                }
                else if (conversion == 'o')
                {
                    base = 8;
                    if (flags & kAlternate)
                        prefix += '0';
                }
                if (!(precision == 0 && value == 0))
                    digitsText = hostDigits(app, value, base, longLong ? kDigits64 : kDigitsLow, reads);
                if (conversion == 'X')
                {
                    for (char& ch : prefix)
                        ch = char(clib::toupper(app, u8(ch)));
                    for (char& ch : digitsText)
                        ch = char(clib::toupper(app, u8(ch)));
                }
                length = u32(digitsText.size());
                if (nfs::s32(length) < nfs::s32(precision))
                    zeros = precision - length;
                if (precision == 0xffffffff && !(flags & kLeft) && pad == '0')
                {
                    const u32 left = width - u32(prefix.size()) - zeros - length;
                    if (nfs::s32(left) > 0)
                        zeros += left;
                }
                break;
            }
            case 's':
            {
                if ((flags & kFar) || (flags & kLong))
                    return std::nullopt;
                u32 pointer;
                if (!args.take(pointer))
                    return std::nullopt;
                if (pointer != 0)
                {
                    stringAt = pointer;
                    fromGuest = true;
                    u32 n = 0;
                    for (u32 s = pointer; byte(app, s) != 0 && n != precision; ++s)
                        ++n;
                    reads.add(pointer, std::uint64_t(n) + 1);
                    length = n;
                    if (nfs::s32(precision) >= 0 && nfs::s32(length) > nfs::s32(precision))
                        length = precision;
                }
                break;
            }
            case 'c':
            {
                if (flags & kLong)
                    return std::nullopt;
                u32 value;
                if (!args.take(value))
                    return std::nullopt;
                prefix += char(u8(value));
                break;
            }
            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'S':
            case 'C':
            case 'p':
            case 'P':
            case 'n':
                return std::nullopt;
            default:
                width = 0;
                prefix += char(conversion);
                break;
            }

            // The field (prtfOf): spaces before or after, the prefix, the zeros, the string.
            const std::uint64_t parts = prefix.size() + std::uint64_t(nfs::s32(zeros) > 0 ? zeros : 0)
                                        + (nfs::s32(length) > 0 ? length : 0);
            width -= u32(prefix.size()) + zeros + length;
            const std::uint64_t spaces = nfs::s32(width) > 0 ? width : 0;
            if (text.size() + parts + spaces > kHostTextMost)
                return std::nullopt;
            if (!(flags & kLeft) && pad == ' ')
                text.append(spaces, ' ');
            text += prefix;
            if (nfs::s32(zeros) > 0)
                text.append(zeros, '0');
            if (nfs::s32(length) > 0)
            {
                if (fromGuest)
                    for (u32 i = 0; i < length; ++i)
                        text += char(byte(app, stringAt + i));
                else
                    text.append(digitsText, 0, length);
            }
            if (flags & kLeft)
                text.append(spaces, ' ');
        } while (byte(app, p) != 0);
    reads.add(format, std::uint64_t(p - format) + 3);
    if (text.size() > kHostTextMost)
        return std::nullopt;
    return text;
}

/* The text into `buffer`, a 0 after, if it overlaps nothing it was made from; how many
 * characters, or nullopt. */
std::optional<u32> hostFormatInto(nfs::App* app, u32 buffer, const std::string& text,
                                    const Reads& reads)
{
    if (reads.overlap(buffer, std::uint64_t(text.size()) + 1))
        return std::nullopt;
    if (!text.empty())
        std::memcpy(&app->getMemory<char>(buffer), text.data(), text.size());
    putByte(app, buffer + u32(text.size()), 0);
    return u32(text.size());
}

/* vsprintf's text on the host: the arguments' pointer at `argp`, moved on past what was taken as
 * __prtf moves it; nullopt where __prtf is left to make it -- also where the pointer moved and
 * anything the text was made from lies at `argp`: __prtf writes the pointer back after each
 * conversion, so what it read there later was the moved pointer. */
std::optional<u32> hostVsprintf(nfs::App* app, const Segments& s, u32 buffer, u32 format,
                                  u32 argp)
{
    Reads reads;
    FormatArguments args(app, at(app, argp), reads);
    const auto text = hostFormatText(app, s, format, args, reads);
    if (!text)
        return std::nullopt;
    if (args.pointer() != at(app, argp) && reads.overlap(argp, 4))
        return std::nullopt;
    reads.add(argp, 4);
    const auto n = hostFormatInto(app, buffer, *text, reads);
    if (n && args.pointer() != at(app, argp))
        put(app, argp, args.pointer());
    return n;
}

/* sprintf's: the arguments at `arguments` on, on the guest's stack. */
std::optional<u32> hostSprintf(nfs::App* app, const Segments& s, u32 buffer, u32 format,
                                 u32 arguments)
{
    Reads reads;
    FormatArguments args(app, arguments, reads);
    const auto text = hostFormatText(app, s, format, args, reads);
    return text ? hostFormatInto(app, buffer, *text, reads) : std::nullopt;
}

/* And a typed caller's, its words in hand. */
std::optional<u32> hostSprintf(nfs::App* app, const Segments& s, u32 buffer, u32 format,
                                 const guest::Words& words)
{
    Reads reads;
    FormatArguments args(words, reads);
    const auto text = hostFormatText(app, s, format, args, reads);
    return text ? hostFormatInto(app, buffer, *text, reads) : std::nullopt;
}

}  // namespace

/* vsprintf (clib/string/c/vsprintf.c, 0x4df674): the format at `format` with the arguments'
 * pointer at `argp` into `buffer`, a 0 after; how many.  On the host where it can be
 * (hostVsprintf), else __prtf's: at its entry's depth, ecx and esi pushed.  0 with ts.terminate
 * set when the game stopped. */
u32 vsprintfOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 buffer, u32 format,
                 u32 argp, bool onHost = true)
{
    if (onHost)
        if (const auto n = hostVsprintf(app, s, buffer, format, argp))
            return *n;
    const GuestFrame frame(ts, 8, 0);
    u32 n;
    {
        const ReturnSlot slot(ts);
        n = prtfOf(app, ts, s, buffer, format, argp, kMemPutc);
    }
    if (ts.terminate)
        return 0;
    putByte(app, buffer + n, 0);
    return n;
}

nfs::u32 vsprintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = vsprintfOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* sprintf (clib/string/c/sprintf.c, 0x4df690): cdecl, the buffer and the format on the stack, the
 * arguments after, from `arguments` on.  At its entry's depth: ebx and edx pushed, then the
 * va_list. */
u32 sprintfOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 buffer, u32 format,
                u32 arguments)
{
    if (const auto n = hostSprintf(app, s, buffer, format, arguments))
        return *n;
    const GuestFrame frame(ts, 8, 4);
    const u32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(ts);
    return vsprintfOf(app, ts, s, buffer, format, list, false);
}

nfs::u32 sprintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = sprintfOf(app, ts, segmentsOf(ts), at(app, ts.sp + 4), at(app, ts.sp + 8), ts.sp + 0xc);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* vfprintf (0x4ff014): the format at `format` with the arguments' pointer at `argp` to the stream
 * at `fp`, under the file's lock: none to a stream of wide characters, an unbuffered one buffered
 * while it goes; how many, or -1 for an error.  At its entry's depth: four pushes.  0 with
 * ts.terminate set when the game stopped. */
u32 vfprintfOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 fp, u32 format, u32 argp)
{
    const GuestFrame frame(ts, 0x10, 0);
    // The file's lock and its release, every register kept.
    auto lock = [&](u32 slot) {
        const EspKept kept(ts);
        return streamLock(app, ts, slot, fp);
    };
    if (!lock(kAccessFile))
        return 0;
    const u32 link = linkOf(app, fp);
    const u32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
        {
            lock(kReleaseFile);
            return 0;
        }
        put(app, link + kOrientation, 1);
    }
    const u32 kept = at(app, fp + kFlag) & 0x30;
    putByte(app, fp + kFlag, byte(app, fp + kFlag) & 0xcf);
    if (bufferOf(app, fp) == 0)
    {
        const ReturnSlot slot(ts);
        const EspKept registers(ts);
        if (!allocateBuffer(app, ts, fp))
            return 0;
    }
    const bool unbuffered = byte(app, fp + kFlag + 1) & 4;
    if (unbuffered)
        putByte(app, fp + kFlag + 1, u8((byte(app, fp + kFlag + 1) & 0xfa) | 1));
    u32 result;
    {
        const ReturnSlot slot(ts);
        result = prtfOf(app, ts, s, fp, format, argp, kFilePutc);
    }
    if (ts.terminate)
        return 0;
    if (unbuffered)
    {
        const u8 restored = u8((byte(app, fp + kFlag + 1) & 0xfa) | 4);
        putByte(app, fp + kFlag + 1, u8(byte(app, fp + kFlag + 1) & 0xfa));
        putByte(app, fp + kFlag + 1, restored);
        const ReturnSlot slot(ts);
        const EspKept registers(ts);
        u32 flushed;
        if (!flushStream(app, ts, fp, flushed))
            return 0;
    }
    if (byte(app, fp + kFlag) & 0x20)
        result = 0xffffffff;
    put(app, fp + kFlag, at(app, fp + kFlag) | kept);
    if (!lock(kReleaseFile))
        return 0;
    return result;
}

nfs::u32 vfprintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = vfprintfOf(app, ts, segmentsOf(ts), args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* fprintf (clib/streamio/c/fprintf.c, 0x4f4b80): cdecl, the stream and the format on the stack, the
 * arguments after, from `arguments` on.  At its entry's depth: ebx and edx pushed, then the
 * va_list. */
u32 fprintfOf(nfs::App* app, nfs::ThreadState& ts, const Segments& s, u32 fp, u32 format,
                u32 arguments)
{
    const GuestFrame frame(ts, 8, 4);
    const u32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(ts);
    return vfprintfOf(app, ts, s, fp, format, list);
}

nfs::u32 fprintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = fprintfOf(app, ts, segmentsOf(ts), at(app, ts.sp + 4), at(app, ts.sp + 8), ts.sp + 0xc);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}


/* ---- The environment, the time zone, temporary files' names, the stack's
 * probe and the way out: what of the runtime the game calls besides its
 * strings, streams, heap and formatted output, and the multibyte characters'
 * functions getenv compares names with.
 *
 * Most of it runs once, at the start or at the end, and some of it uses the
 * stack as no compiler would -- __GRO touches each page below it, _exit jumps
 * into __exit with a register still pushed, the time zone's parser continues
 * in another function's frame.  So these are written as their code is: each
 * push and pop to the guest's stack, each call with the return address's
 * slot below esp and esp as the callee leaves it, the registers as the code
 * leaves them.  Calls to Windows go through the game's imports. */
namespace
{

constexpr u32 kEnviron = 0xa0b154;          // _environ
constexpr u32 kTzName0 = 0x568a88;          // tzname[0]: 0x80 bytes and a 0
constexpr u32 kTzName1 = 0x568b09;          // tzname[1]
constexpr u32 kTimezoneSeconds = 0x568b94;  // timezone
constexpr u32 kDaylightOn = 0x568b98;       // daylight
constexpr u32 kDstSeconds = 0x568b9c;       // __dst_adjust
constexpr u32 kZoneDone = 0x568ba0;         // bit 2: the zone asked for
constexpr u32 kTzString = 0x5505ac;         // "TZ"
constexpr u32 kSjisTrail = 0x56b6dd;        // a byte a character: 8 for a trail byte of code page 932
constexpr u32 kFiniStart = 0x56b81a, kFiniEnd = 0x56b844;  // the runtime's finalizers: 6 bytes each
constexpr u32 kExitHook = 0x564484, kExitHook2 = 0x564488, kExitUser = 0x5677f0;
constexpr u32 kExitStarted = 0x9f6420;      // whether the runtime's start-up got as far as its finalizers
constexpr u32 kExitDone = 0x5677a0, kExitEarly = 0x5677ac;
constexpr u32 kEnvironCopy = 0x9f6424, kWideEnvironCopy = 0x9f6428;
constexpr u32 kAtexitCount = 0x9f3890, kAtexitTable = 0x9f380c;
constexpr u32 kDebuggerOn = 0x56af64;       // a debugger's: the runtime's int3 to it
constexpr u32 kOsHandleTable = 0x56acbc;    // __OSHandles: stderr's HANDLE at +8
constexpr u32 kStackOverflow = 0x5662cc;    // "Stack Overflow!"
constexpr u32 kTmpDir = 0x568278;           // the directory of temporary files, once found
constexpr u32 kTmpVariables = 0x568264;     // TMP, TEMP, TMPDIR, TEMPDIR, ""
constexpr u32 kConsoleName = 0x551b08;      // "con"
constexpr u32 kThreadPtr = 0x567764;        // __GetThreadPtr's pointer

void push(nfs::App* app, nfs::ThreadState& ts, u32 value)
{
    ts.sp -= 4;
    put(app, ts.sp, value);
}

u32 pop(nfs::App* app, nfs::ThreadState& ts)
{
    const u32 value = at(app, ts.sp);
    ts.sp += 4;
    return value;
}

void pushEs(nfs::App* app, nfs::ThreadState& ts)
{
    ts.sp -= 4;
    putHalf(app, ts.sp, 0);  // es: the flat model's 0
}

void popEs(nfs::App* app, nfs::ThreadState& ts)
{
    ts.sp += 4;  // es back: the flat model's 0 all along
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it; the callee's thunk run on `args`, the registers the code has at the call, and
 * what it leaves in them (eax too) left there.  False when the game stopped in it. */
bool direct(nfs::App* app, nfs::ThreadState& ts, nfs::ThunkPtr function, nfs::Args& args)
{
    ts.sp -= 4;
    args.eax = function(app, ts, args);
    return !ts.terminate;
}

/* The same through a pointer the code holds (pointerCall: esp back where it was after). */
bool indirect(nfs::App* app, nfs::ThreadState& ts, u32 target, nfs::Args& args)
{
    const std::optional<u32> eax = guest::pointerCall(app, ts, target, args);
    if (!eax)
        return false;
    args.eax = *eax;
    return true;
}

void setLow8(u32& r, u32 value)
{
    r = (r & 0xffffff00) | (value & 0xff);
}

bool multibyte(nfs::App* app)
{
    return at(app, kMbcsOn) != 0;
}

bool leadByte(nfs::App* app, u32 c)
{
    return (byte(app, kMbcsLead + (c & 0xff)) & 1) != 0;
}

}  // namespace

/* ---- The multibyte characters' functions: each a typed core (fooOf) and, under the runtime's
 * name, its thunk for the calls by address, which takes the registers and keeps every one the
 * runtime's code kept.  A core that hands an address in its frame to another (CharUpperBuffA's
 * buffer) keeps that frame where the code had it, and is called at the code's depth. ---- */

/* 0x5250c0 (_mbterm): whether the character at `p` ends a string -- 1 for its 0, 2 for a lead
 * byte with a 0 after it -- or 0. */
u32 mbtermOf(nfs::App* app, u32 p)
{
    if (byte(app, p) == 0)
        return 1;
    if (multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) == 0)
        return 2;
    return 0;
}

nfs::u32 mbterm(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbtermOf(app, args.eax));
    return args.eax;
}

/* 0x525100 (_mbsinc): past the character at `p`. */
u32 mbsincOf(nfs::App* app, u32 p)
{
    return multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) != 0 ? p + 2 : p + 1;
}

nfs::u32 mbsinc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbsincOf(app, args.eax));
    return args.eax;
}

/* 0x52b5b0 (_mbclen): the character at `p`'s bytes, 1 or 2. */
u32 mbclenOf(nfs::App* app, u32 p)
{
    return multibyte(app) && leadByte(app, byte(app, p)) ? 2 : 1;
}

nfs::u32 mbclen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbclenOf(app, args.eax));
    return args.eax;
}

/* 0x52e310 (_mbccpy): the character at `from` to `to`. */
void mbccpyOf(nfs::App* app, u32 to, u32 from)
{
    putByte(app, to, byte(app, from));
    if (multibyte(app) && leadByte(app, byte(app, from)))
        putByte(app, to + 1, byte(app, from + 1));
}

nfs::u32 mbccpy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    mbccpyOf(app, args.eax, args.edx);  // eax left as it was, the destination
    ts.sp += 4;
    return args.eax;
}

/* 0x52e3a0 (_mbccmp): the characters at `a` and `b` compared, their bytes' difference. */
u32 mbccmpOf(nfs::App* app, u32 a, u32 b)
{
    if (byte(app, a) != byte(app, b))
        return u32(byte(app, a)) - byte(app, b);
    if (multibyte(app) && leadByte(app, byte(app, a)) && byte(app, a + 1) != byte(app, b + 1))
        return u32(byte(app, a + 1)) - byte(app, b + 1);
    return 0;
}

nfs::u32 mbccmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbccmpOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x52ed30 (_mbsnextc): the character at `p`, a lead byte's two as one. */
u32 mbsnextcOf(nfs::App* app, u32 p)
{
    if (multibyte(app) && leadByte(app, byte(app, p)))
        return u32(byte(app, p)) << 8 | byte(app, p + 1);
    return byte(app, p);
}

nfs::u32 mbsnextc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbsnextcOf(app, args.eax));
    return args.eax;
}

/* 0x52ede0 (_mbvtop): the character `c` into the bytes at `to`; `to` back. */
u32 mbvtopOf(nfs::App* app, u32 c, u32 to)
{
    if (c & 0xff00)
    {
        putByte(app, to + 1, u8(c));
        putByte(app, to, u8(c >> 8));
    }
    else
        putByte(app, to, u8(c));
    return to;
}

nfs::u32 mbvtop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbvtopOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x52ef70 (_ismbbtrail, as the runtime has it): whether the byte `c` may follow a lead byte. */
u32 mbbtrailOf(nfs::App* app, u32 c)
{
    if (!multibyte(app))
        return 0;
    if (at(app, kCodePage) == 932)
        return byte(app, kSjisTrail + (c & 0xff)) & 8;
    return c != 0 ? 1 : 0;
}

nfs::u32 mbbtrail(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbbtrailOf(app, args.eax));
    return args.eax;
}

/* 0x52ef30 (_ismbclegal's): whether `c` is a character of two bytes. */
u32 mbclegalOf(nfs::App* app, u32 c)
{
    if (multibyte(app) && leadByte(app, c >> 8 & 0xff))
        return mbbtrailOf(app, c & 0xff) != 0 ? 1 : 0;
    return 0;
}

nfs::u32 mbclegal(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbclegalOf(app, args.eax));
    return args.eax;
}

/* 0x52eee0 (_ismbclower's, code page 932): whether `c` is a small letter -- a full-width one of
 * two bytes, or one of _IsTable's. */
u32 mbclowerOf(nfs::App* app, u32 c)
{
    if (mbclegalOf(app, c) == 0)
        return byte(app, kIsTable + ((c + 1) & 0xff)) & 0x80;
    return c >= 0x8281 && c <= 0x829a ? 1 : 0;
}

nfs::u32 mbclower(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbclowerOf(app, args.eax));
    return args.eax;
}

/* 0x52eec0 (_mbctoupper's, code page 932): a full-width small letter as a capital. */
u32 mbcupper932Of(nfs::App* app, u32 c)
{
    return mbclowerOf(app, c) != 0 ? c - 0x21 : c;
}

nfs::u32 mbcupper932(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbcupper932Of(app, args.eax));
    return args.eax;
}

/* 0x52ed70 (_mbctoupper): the character `c` as a capital, by CharUpperBuffA on its bytes in the
 * frame -- or, a full-width one of code page 932, by the runtime's own.  Called at its entry's
 * depth (esp at the return address's slot): three pushes, then the bytes.  0 with ts.terminate
 * set when the game stopped. */
u32 mbctoupperOf(nfs::App* app, nfs::ThreadState& ts, u32 c)
{
    const GuestFrame frame(ts, 0xc, 4);
    const u32 bytes = frame - 4;
    mbvtopOf(app, c, bytes);
    putByte(app, bytes + mbclenOf(app, bytes), 0);
    if (at(app, kCodePage) == 932 && multibyte(app) && leadByte(app, byte(app, bytes)))
        return mbcupper932Of(app, c);
    os::text::upper(guest::memory(app, bytes), 1);
    return mbsnextcOf(app, bytes);
}

nfs::u32 mbctoupper(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 upper;
    {
        const EspKept kept(ts);
        upper = mbctoupperOf(app, ts, args.eax);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, upper);
    return args.eax;
}

/* 0x52e350 (_mbsupr): the string `s` in capitals, a character at a time; `s` back.  At its
 * entry's depth: three pushes, then a character's bytes.  0 with ts.terminate set when the game
 * stopped. */
u32 mbsuprOf(nfs::App* app, nfs::ThreadState& ts, u32 s)
{
    const GuestFrame frame(ts, 0xc, 4);
    const u32 bytes = frame - 4;
    for (u32 p = s; mbtermOf(app, p) == 0; p = mbsincOf(app, p))
    {
        u32 upper;
        {
            const ReturnSlot call(ts);
            upper = mbctoupperOf(app, ts, mbsnextcOf(app, p));
        }
        if (ts.terminate)
            return 0;
        mbvtopOf(app, upper, bytes);
        putByte(app, bytes + mbclenOf(app, bytes), 0);
        mbccpyOf(app, p, bytes);
    }
    return s;
}

nfs::u32 mbsupr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 s;
    {
        const EspKept kept(ts);
        s = mbsuprOf(app, ts, args.eax);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, s);
    return args.eax;
}

/* 0x52b5e0 (_mbcicmp): the characters at `a` and `b` compared as capitals: each copied into the
 * frame, put in capitals there, and compared.  At its entry's depth: two pushes, then the two
 * characters' 4 bytes each.  0 with ts.terminate set when the game stopped. */
u32 mbcicmpOf(nfs::App* app, nfs::ThreadState& ts, u32 a, u32 b)
{
    const GuestFrame frame(ts, 8, 8);
    const u32 first = frame - 8, second = frame - 4;
    mbccpyOf(app, first, a);
    putByte(app, first + mbclenOf(app, a), 0);
    mbccpyOf(app, second, b);
    putByte(app, second + mbclenOf(app, b), 0);
    for (const u32 s : {first, second})
    {
        const ReturnSlot call(ts);
        mbsuprOf(app, ts, s);
        if (ts.terminate)
            return 0;
    }
    return mbccmpOf(app, first, second);
}

nfs::u32 mbcicmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 differs;
    {
        const EspKept kept(ts);
        differs = mbcicmpOf(app, ts, args.eax, args.edx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, differs);
    return args.eax;
}

/* 0x523d10 (_mbsnicmp): at most `count` characters of the strings at `a` and `b` compared as
 * capitals: the first difference, or that of the bytes where one ended first, or 0.  At its
 * entry's depth: two pushes.  0 with ts.terminate set when the game stopped. */
u32 mbsnicmpOf(nfs::App* app, nfs::ThreadState& ts, u32 a, u32 b, u32 count)
{
    const GuestFrame frame(ts, 8, 0);
    for (; count != 0; --count, a = mbsincOf(app, a), b = mbsincOf(app, b))
    {
        if (mbtermOf(app, a) != 0 || mbtermOf(app, b) != 0)
            break;
        u32 differs;
        {
            const ReturnSlot call(ts);
            differs = mbcicmpOf(app, ts, a, b);
        }
        if (ts.terminate)
            return 0;
        if (differs != 0)
            return differs;
    }
    if (count != 0 && (mbtermOf(app, a) != 0 || mbtermOf(app, b) != 0))
        return u32(byte(app, a)) - byte(app, b);
    return 0;
}

nfs::u32 mbsnicmp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 differs;
    {
        const EspKept kept(ts);
        differs = mbsnicmpOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, differs);
    return args.eax;
}

namespace
{

u32 runs(nfs::App* app, nfs::ThreadState& ts, nfs::ThunkPtr function,
           std::initializer_list<nfs::u32> registers);

}  // namespace

/* getenv's work from the function's entry, esp at its return address's slot: the room of the six
 * registers it keeps pushed (nothing written there), so _mbsnicmp's are made at the depth the
 * runtime's were.  0 with ts.terminate set when the game stopped. */
u32 getenvOf(nfs::App* app, nfs::ThreadState& ts, u32 name)
{
    ts.sp -= 0x18;
    u32 entry = at(app, kEnviron);
    u32 value = 0;
    if (entry != 0 && name != 0)
    {
        u32 length = 0;
        while (byte(app, name + length) != 0)
            ++length;
        for (u32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
        {
            u32 differs;
            {
                const ReturnSlot call(ts);
                differs = mbsnicmpOf(app, ts, string, name, length);
            }
            if (ts.terminate)
                return 0;
            if (differs == 0 && byte(app, string + length) == '=')
            {
                value = string + length + 1;
                break;
            }
        }
    }
    ts.sp += 0x18;
    return value;
}

/* getenv (clib/environ/c/getenv.c, 0x50ead0): the value of the variable named
 * at eax, its name matched as capitals -- or 0. */
nfs::u32 getenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 value = getenvOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, value);
    return args.eax;
}

/* wcstombs (clib/mbyte/c/wcstombs.c, 0x5242f0): the wide string at `from` into `to` as wctomb
 * makes each character, at most `most` bytes -- or, for no buffer, how many it would take; -1 for
 * one wctomb cannot make.  At its entry's depth: four pushes, then 0xc bytes, wctomb's character
 * at +8 of them (WideCharToMultiByte is handed it).  0 with ts.terminate set when the game
 * stopped. */
u32 wcstombsOf(nfs::App* app, nfs::ThreadState& ts, u32 to, u32 from, u32 most)
{
    const GuestFrame frame(ts, 0x10, 0xc);
    const u32 character = frame - 4;
    u32 total = 0;
    if (to == 0)
    {
        for (; half(app, from) != 0; from += 2)
        {
            const u32 bytes = called(ts, [&] { return wctombOf(app, ts, character, half(app, from)); });
            if (ts.terminate)
                return 0;
            if (bytes == 0xffffffff)
                return bytes;
            total += bytes;
        }
        return total;
    }
    for (u32 room = most; room != 0; from += 2)
    {
        const u32 c = half(app, from);
        if (c == 0)
        {
            putByte(app, to, 0);
            break;
        }
        const u32 bytes = called(ts, [&] { return wctombOf(app, ts, character, c); });
        if (ts.terminate)
            return 0;
        if (bytes == 0xffffffff)
            return bytes;
        if (bytes > room)
            break;
        for (u32 i = 0; i < bytes; ++i)
            putByte(app, to + i, byte(app, character + i));
        total += bytes;
        room -= bytes;
        to += bytes;
    }
    return total;
}

nfs::u32 wcstombs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 result;
    {
        const EspKept kept(ts);
        result = wcstombsOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* ---- __tzset (clib/time/c/tzset.c) in its parts: TZ's zone and its daylight saving's name and
 * rules parsed, or the zone asked of Windows.  The zone's and the rules' numbers are C++ locals;
 * what each part leaves in eax is worked out from what it did. ---- */

/* 0x515c90 (__tzset's number): the decimal digits at `p` into `value`; past them. */
u32 tzNumberOf(nfs::App* app, u32 p, u32& value)
{
    value = 0;
    for (; byte(app, p) >= '0' && byte(app, p) <= '9'; ++p)
        value = value * 10 + byte(app, p) - '0';
    return p;
}

nfs::u32 tzNumber(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 value;
    const u32 past = tzNumberOf(app, args.eax, value);
    put(app, args.edx, value);
    returns(ts, args, past);
    return args.eax;
}

/* 0x515cbc (__tzset's zone): TZ's name at `p` -- up to a sign, a digit or a comma, at most 0x80
 * of it -- into `name`, and the hours[:minutes[:seconds]] after it, as seconds west, into the
 * dword at `seconds`; past them. */
u32 tzNameOf(nfs::App* app, u32 p, u32 name, u32 seconds)
{
    if (byte(app, p) == ':')
        ++p;
    const u32 start = p;
    for (;; ++p)
    {
        const u8 c = byte(app, p);
        if (c == 0 || c == ',' || c == '-' || c == '+' || (c >= '0' && c <= '9'))
            break;
    }
    u32 length = p - start;
    if (nfs::s32(length) > 0x80)
        length = 0x80;
    for (u32 i = 0; i < length; ++i)
        putByte(app, name + i, byte(app, start + i));
    putByte(app, name + length, 0);
    const u8 sign = byte(app, p);
    if (sign == '-' || sign == '+')
        ++p;
    if (byte(app, p) >= '0' && byte(app, p) <= '9')
    {
        u32 hours, minutes = 0, rest = 0;
        p = tzNumberOf(app, p, hours);
        if (byte(app, p) == ':')
        {
            p = tzNumberOf(app, p + 1, minutes);
            if (byte(app, p) == ':')
                p = tzNumberOf(app, p + 1, rest);
        }
        const u32 west = rest + (minutes + hours * 60) * 60;
        put(app, seconds, sign == '-' ? 0 - west : west);
    }
    return p;
}

nfs::u32 tzName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, tzNameOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x515dd4 (__tzset's rule): daylight saving's start or end at `p` -- Jn, n or Mm.w.d, then
 * /hours[:minutes[:seconds]] -- into the struct tm at `tm`; past it. */
u32 tzRuleOf(nfs::App* app, u32 p, u32 tm)
{
    u32 kind = 0xffffffff;  // -1 a day of the year from 0, 1 from 1 (J), 0 a week's day (M)
    if (byte(app, p) == 'J')
    {
        kind = 1;
        ++p;
    }
    if (byte(app, p) == 'M')
    {
        ++p;
        kind = 0;
    }
    put(app, tm + 0x20, kind);
    u32 n;
    p = tzNumberOf(app, p, n);
    if (kind != 0)
        put(app, tm + 0x1c, n);
    else
    {
        put(app, tm + 0x10, n - 1);
        if (byte(app, p) == '.')
        {
            p = tzNumberOf(app, p + 1, n);
            put(app, tm + 0xc, n);
            if (byte(app, p) == '.')
            {
                p = tzNumberOf(app, p + 1, n);
                put(app, tm + 0x18, n);
            }
        }
        put(app, tm + 0x1c, 0);
    }
    u32 hours = 2, minutes = 0, seconds = 0;
    if (byte(app, p) == '/')
    {
        p = tzNumberOf(app, p + 1, hours);
        if (byte(app, p) == ':')
        {
            p = tzNumberOf(app, p + 1, minutes);
            if (byte(app, p) == ':')
                p = tzNumberOf(app, p + 1, seconds);
        }
    }
    put(app, tm, seconds);
    put(app, tm + 4, minutes);
    put(app, tm + 8, hours);
    return p;
}

nfs::u32 tzRule(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, tzRuleOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x515b34 (__tzset's without TZ): the zone, daylight saving and the zones' names, once.  The
 * port tells no time zone (os::time): they stay as the runtime has them, and 0 is what the code
 * leaves in eax -- given `eax` on entry, as it is when asked once already. */
u32 tzWindowsOf(nfs::App* app, nfs::ThreadState& ts, u32 eax)
{
    NFS2_USE(ts);
    const u8 done = byte(app, kZoneDone);
    if ((done & 1) && (done & 2))
        return (eax & 0xffff00ff) | u32(done) << 8;
    putByte(app, kZoneDone, done | 2);
    return 0;
}

nfs::u32 tzWindows(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 eax;
    {
        const EspKept kept(ts);
        eax = tzWindowsOf(app, ts, args.eax);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x515ec2: __tzset's after a TZ zone with a daylight name -- its own offset or an hour less, and
 * the rules after commas, the end rule's time made standard time -- in __tzset's frame: `local`
 * its dword (the daylight zone's offset is parsed into it), `nameEnd` where the zone's name ended.
 * What the code leaves in eax. */
u32 tzDaylightOf(nfs::App* app, u32 nameEnd, u32 local)
{
    put(app, kDaylightOn, 1);
    put(app, local, at(app, kTimezoneSeconds) - 3600);
    const u32 p = tzNameOf(app, nameEnd, kTzName1, local);
    put(app, kDstSeconds, at(app, kTimezoneSeconds) - at(app, local));
    u32 eax = p, rules = p;
    if (byte(app, p) == ',')
        eax = rules = tzRuleOf(app, p + 1, kDstStart);
    const u32 dst = at(app, kDstSeconds);
    if (byte(app, rules) == ',')
    {
        tzRuleOf(app, rules + 1, kDstEnd);
        const nfs::s32 seconds = nfs::s32(at(app, kDstSeconds));
        const nfs::s32 minutes = seconds / 60;
        const u32 hour = at(app, kDstEnd + 8) - u32(seconds / 3600);
        const u32 minute = at(app, kDstEnd + 4) - u32(minutes % 60);
        eax = at(app, kDstEnd) - u32(seconds % 60);
        put(app, kDstEnd + 8, hour);
        put(app, kDstEnd + 4, minute);
        put(app, kDstEnd, eax);
        put(app, kDstSeconds, u32(seconds));
        return eax;
    }
    put(app, kDstSeconds, dst);
    return eax;
}

nfs::u32 tzDaylight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 local = ts.sp;  // __tzset's dword, its six registers above
    const u32 eax = tzDaylightOf(app, args.ecx, local);
    ts.sp = local + 4;
    args.ebp = pop(app, ts);
    args.edi = pop(app, ts);
    args.esi = pop(app, ts);
    args.edx = pop(app, ts);
    args.ecx = pop(app, ts);
    args.ebx = pop(app, ts);
    returns(ts, args, eax);
    return args.eax;
}

/* __tzset (clib/time/c/tzset.c, 0x515c2c): the zone from TZ -- its name, offset, daylight
 * saving's name and rules -- or from Windows.  Its frame -- the six registers it keeps and a
 * dword -- is where tzDaylight finishes, as the code jumps there. */
nfs::u32 tzset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 tz;
    {
        const ReturnSlot call(ts);
        tz = getenvOf(app, ts, kTzString);
    }
    if (ts.terminate)
        return args.eax;
    args.eax = tz;
    if (tz == 0)
        return tzWindows(app, ts, args);
    for (const u32 r : {args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp})
        push(app, ts, r);
    ts.sp -= 4;
    put(app, kDaylightOn, 0);
    const u32 p = tzNameOf(app, tz, kTzName0, kTimezoneSeconds);
    if (byte(app, p) != 0)
    {
        args.ecx = p;
        return tzDaylight(app, ts, args);
    }
    putByte(app, kTzName1, 0);
    ts.sp += 4;
    args.ebp = pop(app, ts);
    args.edi = pop(app, ts);
    args.esi = pop(app, ts);
    args.edx = pop(app, ts);
    args.ecx = pop(app, ts);
    args.ebx = pop(app, ts);
    returns(ts, args, p & 0xffff00ff);
    return args.eax;
}

/* 0x4ff410 (__FiniRtns' call): the routine at the pointer at `p`, if any, with es its ds -- es
 * pushed meanwhile, and ds pushed and popped into it.  False when the game stopped. */
bool callRoutineOf(nfs::App* app, nfs::ThreadState& ts, u32 p)
{
    // es pushed, and ds pushed and popped into it for the routine: the room of es's push (ds's is
    // the call's return address's), nothing written; es and ds are the same everywhere in the game.
    const GuestFrame frame(ts, 4, 0);
    if (at(app, p) == 0)
        return true;
    const EspKept kept(ts);
    return through(app, ts, at(app, p), {p}).has_value();
}

nfs::u32 callRoutine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!callRoutineOf(app, ts, args.eax))
        return args.eax;
    ts.sp += 4;  // eax left as it was, the pointer
    return args.eax;
}

/* __FiniRtns (clib/startup/c/initrtns.c, 0x4ff46c): the runtime's finalizers of priority `low` to
 * `high` not yet run, the highest first, each marked done.  At its entry's depth: three pushes and
 * es.  What the code leaves in eax (its table's end); 0 with ts.terminate set when the game
 * stopped. */
u32 finiRtnsOf(nfs::App* app, nfs::ThreadState& ts, u8 low, u8 high)
{
    const GuestFrame frame(ts, 0x10, 0);
    for (;;)
    {
        u32 chosen = kFiniEnd;
        u8 best = low;
        for (u32 entry = kFiniStart; entry < kFiniEnd; entry += 6)
            if (byte(app, entry) != 2 && best <= byte(app, entry + 1))
            {
                chosen = entry;
                best = byte(app, entry + 1);
            }
        if (chosen == kFiniEnd)
            break;
        if (high >= byte(app, chosen + 1))
        {
            called(ts, [&] { return callRoutineOf(app, ts, chosen + 2); });
            if (ts.terminate)
                return 0;
        }
        putByte(app, chosen, 2);
    }
    return kFiniEnd;
}

nfs::u32 FiniRtns(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u8 low = u8(args.eax), high = u8(args.edx);
    u32 eax;
    {
        const EspKept kept(ts);
        eax = finiRtnsOf(app, ts, low, high);
    }
    if (ts.terminate)
        return args.eax;
    // dh the highest priority, dl the lowest, as the code leaves them (exit_ is stopped with them).
    args.edx = (args.edx & 0xffff0000) | u32(high) << 8 | low;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x51f11c (__DoneExceptionHandler): the thread's exception record off fs's
 * chain. */
nfs::u32 DoneExceptionHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.edx);
    if (!hookWith<kThreadPtr>(app, ts, args))
        return args.eax;
    args.eax = at(app, args.eax + 0x54);
    if (args.eax != 0)
    {
        args.eax = at(app, args.eax);
        args.edx = 0;
        put(app, ts.threadBlock + args.edx, args.eax);
    }
    if (!hookWith<kThreadPtr>(app, ts, args))
        return args.eax;
    put(app, args.eax + 0x54, 0);
    args.edx = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* 0x4ff3d0 (the environment's copies given back): what the start-up code made of it, narrow and
 * wide, to _nfree.  What the code leaves in eax, given `eax` on entry: _nfree's last answer, if
 * any.  0 with ts.terminate set when the game stopped. */
u32 freeEnvironmentOf(nfs::App* app, nfs::ThreadState& ts, u32 eax)
{
    for (const u32 copy : {kEnvironCopy, kWideEnvironCopy})
        if (at(app, copy) != 0)
        {
            eax = clib::free(app, ts, at(app, copy));
            if (ts.terminate)
                return 0;
            put(app, copy, 0);
        }
    return eax;
}

nfs::u32 freeEnvironment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = freeEnvironmentOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* __exit (clib/startup/c/exit.c, 0x4ff38c): ExitProcess with eax -- the
 * finalizers and the exception record first, unless the start-up got no
 * further than its own -- and, should it come back, the environment's copies
 * given back, as the code after it does. */
nfs::u32 exit_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.ebx = args.eax;
    if (at(app, kExitStarted) != 0)
    {
        if (at(app, kExitEarly) != 0)
        {
            args.edx = 0xff;
            args.eax = 0;
            if (!indirect(app, ts, at(app, kExitEarly), args))
                return args.eax;
        }
    }
    else
    {
        if (!direct(app, ts, DoneExceptionHandler, args))
            return args.eax;
        args.edx = 0xf;
        args.eax = 0;
        if (!direct(app, ts, FiniRtns, args) || !hookWith<kExitDone>(app, ts, args))
            return args.eax;
    }
    os::exitProcess(app, ts, args.ebx);
    if (ts.terminate)
        return args.eax;
    freeEnvironment(app, ts, args);
    return args.eax;
}

/* _exit (clib/startup/c/exit.c, 0x4dfd30): eax to __exit after the runtime's
 * hooks -- jumped to with edx still pushed. */
nfs::u32 _exit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.edx);
    args.edx = args.eax;
    if (!hookWith<kExitHook>(app, ts, args) || !hookWith<kExitHook2>(app, ts, args))
        return args.eax;
    if (at(app, kExitUser) != 0 && !indirect(app, ts, at(app, kExitUser), args))
        return args.eax;
    args.eax = args.edx;
    exit_(app, ts, args);
    return args.eax;
}

/* exit (clib/startup/c/exit.c, 0x4dfd04): the finalizers of priority 16 and
 * up, then _exit with eax. */
nfs::u32 exit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.ebx);
    push(app, ts, args.edx);
    args.ebx = args.eax;
    if (!hookWith<kExitHook>(app, ts, args))
        return args.eax;
    if (byte(app, kExitStarted) == 0)
    {
        args.edx = 0xff;
        args.eax = 0x10;
        if (!direct(app, ts, FiniRtns, args))
            return args.eax;
    }
    args.eax = args.ebx;
    if (!direct(app, ts, _exit, args))
        return args.eax;
    args.edx = pop(app, ts);
    args.ebx = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* atexit (clib/startup/c/atexit.c, 0x4f7c20): `function` to the 32 run at exit; 0, or -1 when
 * they are taken. */
u32 atexitOf(nfs::App* app, u32 function)
{
    const u32 count = at(app, kAtexitCount);
    if (nfs::s32(count) >= 0x20)
        return 0xffffffff;
    put(app, (count + 1) * 4 + kAtexitTable, function);
    put(app, kAtexitCount, count + 1);
    return 0;
}

nfs::u32 atexit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, atexitOf(app, args.eax));
    return args.eax;
}

/* 0x524190 (__EnterWVIDEO): a message to a debugger, by its int3, if there
 * is one; 1, or 0. */
nfs::u32 EnterWVIDEO(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (byte(app, kDebuggerOn) == 0)
    {
        args.eax = 0;
        ts.sp += 4;
        return args.eax;
    }
    args.edx &= 0xffff;
    push(app, ts, args.edx);
    push(app, ts, args.eax);
    NFS2_ASSERT(false);  // int3
    args.eax = 1;
    ts.sp += 8 + 4;
    return args.eax;
}

/* 0x511cd0 (__fatal_runtime_error's): the message at eax to stderr's handle, then __exit with
 * edx -- jumped to, its frame still there: four pushes and the dword WriteFile counts into. */
nfs::u32 fatalWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 message = args.eax, code = args.edx;
    for (const u32 r : {args.ebx, args.ecx, args.esi, args.edi})
        push(app, ts, r);
    ts.sp -= 4;
    u32 length = 0;
    while (byte(app, message + length) != 0)
        ++length;
    // The registers as the code leaves them on its way out: they are what the game stops with.
    args.ebx = message + length;
    args.ecx &= 0xffffff00;
    args.edi = message;
    args.esi = code;
    args.edx = at(app, at(app, kOsHandleTable) + 8);
    // How many went, into the word under esp as the code has it written there.
    nfs::u32 wrote = at(app, ts.sp);
    os::files::write(app, ts, args.edx, guest::memory(app, message), length, wrote);
    put(app, ts.sp, wrote);
    args.eax = code;
    exit_(app, ts, args);
    return args.eax;
}

/* __fatal_runtime_error (clib/startup/c/fatalerr.c, 0x511d0c): the message at
 * eax to a debugger, or to stderr and out with edx. */
nfs::u32 fatal_runtime_error(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.ebx);
    push(app, ts, args.ecx);
    args.ebx = args.eax;
    args.ecx = args.edx;
    args.edx = 0;  // ds: the flat model's 0
    if (!direct(app, ts, EnterWVIDEO, args))
        return args.eax;
    if (args.eax == 0)
    {
        args.edx = args.ecx;
        args.eax = args.ebx;
        if (!direct(app, ts, fatalWrite, args))
            return args.eax;
    }
    args.ecx = pop(app, ts);
    args.ebx = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* sleep (clib/process/c/sleepwnt.c, 0x4f0710): SleepEx for `seconds`; its answer.  At its entry's
 * depth: two pushes.  0 with ts.terminate set when the game stopped. */
u32 sleepOf(nfs::App* app, nfs::ThreadState& ts, u32 seconds)
{
    const GuestFrame frame(ts, 8, 0);
    const EspKept kept(ts);
    os::threads::sleep(app, ts, seconds * 1000);
    return 0;  // what the code is told of a sleep
}

nfs::u32 sleep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 answer = sleepOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, answer);
    return args.eax;
}

/* getpid (clib/environ/c/gtpidwnt.c, 0x5093c0): the process's number.  The port gives the process
 * none: 0. */
u32 getpidOf(nfs::App* app, nfs::ThreadState& ts)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    return 0;
}

nfs::u32 getpid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 id = getpidOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, id);
    return args.eax;
}

/* stackavail (clib/startup/c/stackwnt.c, 0x4f45c0): how far esp is above the
 * thread's stack's bottom. */
std::optional<u32> stackavailOf(nfs::App* app, nfs::ThreadState& ts, u32 edx)
{
    push(app, ts, edx);
    const std::optional<u32> data = hook<kThreadPtr>(app, ts);
    if (!data)
        return std::nullopt;
    const u32 bottom = at(app, *data);
    const u32 available = ts.sp - bottom;
    pop(app, ts);
    return available;
}

nfs::u32 stackavail(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto available = stackavailOf(app, ts, args.edx);
    if (!available)
        return args.eax;
    args.eax = *available;
    ts.sp += 4;
    return args.eax;
}

/* __GRO (clib/startup/a/stkwnt.asm, 0x4f4610): the stack's pages below esp
 * touched, a dword each, for the size on the stack, which it takes off. */
void stackProbedOf(nfs::App* app, nfs::ThreadState& ts, u32 size, u32 eax, u32 ebx)
{
    push(app, ts, eax);
    push(app, ts, ebx);
    u32 left = size, page = 0xfffffffc;
    do
    {
        put(app, ts.sp + page, page);
        page -= 0x1000;
        left -= 0x1000;
    } while (nfs::s32(left) > 0);
    ts.sp += 4 + 4;  // the two registers popped; they were not changed
}

nfs::u32 GRO(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    stackProbedOf(app, ts, at(app, ts.sp + 4), args.eax, args.ebx);
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4ee15c (__MkTmpFile's digit): `n`, 0 to 15, as a hexadecimal digit. */
u32 hexDigitOf(u32 n)
{
    n += '0';
    return nfs::s32(n) > '9' ? n + 0x27 : n;
}

nfs::u32 hexDigit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    returns(ts, args, hexDigitOf(args.eax));
    return args.eax;
}

/* _fullpath (clib/misc/c/fullpath.c, 0x523610): the path `path` made whole into `buffer`, `size`
 * bytes of it -- 0x104 from _nmalloc for no buffer -- by GetFullPathNameA; "con" as it is, no path
 * or an empty one the current directory.  0 for an error.  For "con" in fewer than 4 bytes the
 * code frees what it holds in edi -- the block it took, or, given a buffer, `edi` as the caller
 * left it.  At its entry's depth: three pushes, then the dword GetFullPathNameA points at the
 * name in.  0 with ts.terminate set when the game stopped. */
u32 fullpathOf(nfs::App* app, nfs::ThreadState& ts, u32 buffer, u32 path, u32 size, u32 edi)
{
    const GuestFrame frame(ts, 0xc, 4);
    if (buffer == 0)
    {
        size = 0x104;
        buffer = edi = clib::malloc(app, ts, size);
        if (ts.terminate)
            return 0;
        if (buffer == 0)
        {
            called(ts, [&] { return setThreadWord(app, ts, kErrno, 5); });  // __set_errno's ENOMEM
            return 0;
        }
    }
    if (path == 0 || byte(app, path) == 0)
        return runs(app, ts, &getcwd, {buffer, size});
    if (clib::stricmp(app, path, kConsoleName) == 0)
    {
        if (size < 4)
        {
            clib::free(app, ts, edi);
            if (ts.terminate)
                return 0;
            called(ts, [&] { return setThreadWord(app, ts, kErrno, 0xe); });  // __set_errno's ERANGE
            return 0;
        }
        put(app, buffer, at(app, kConsoleName));
        return buffer;
    }
    // The port's full path, all of it, however much room the buffer has.
    const std::string full = os::files::fullPath(guest::text(app, path)).path;
    guest::putText(app, buffer, full.c_str());
    const u32 length = u32(full.size());
    if (length == 0)
    {
        called(ts, [&] { return setErrnoNt(app, ts); });  // __set_errno_nt's
        return 0;
    }
    return buffer;
}

nfs::u32 fullpath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 result;
    {
        const EspKept kept(ts);
        result = fullpathOf(app, ts, args.eax, args.edx, args.ebx, args.edi);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x5094e0 (__tmpdir): the directory of temporary files, once: the first of TMP, TEMP, TMPDIR,
 * TEMPDIR that fits, made whole by _fullpath, or the current one; a '\\' after it.  At its entry's
 * depth: five pushes.  0 with ts.terminate set when the game stopped. */
u32 tmpdirOf(nfs::App* app, nfs::ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0);
    auto length = [&](u32 s) {
        u32 n = 0;
        while (byte(app, s + n) != 0)
            ++n;
        return n;
    };
    if (byte(app, kTmpDir) != 0)
        return kTmpDir;
    for (u32 variable = kTmpVariables; byte(app, at(app, variable)) != 0; variable += 4)
    {
        u32 value;
        {
            const ReturnSlot call(ts);
            value = getenvOf(app, ts, at(app, variable));
        }
        if (ts.terminate)
            return 0;
        if (value == 0 || length(value) > 0x103)
            continue;
        {
            const ReturnSlot call(ts);
            // edi past the value, as the inline strlen left it
            fullpathOf(app, ts, kTmpDir, value, 0x103, value + length(value) + 1);
        }
        if (ts.terminate)
            return 0;
        break;
    }
    if (byte(app, kTmpDir) == 0)
    {
        // The current directory, in a block of getcwd's, copied in (the block kept, as the code
        // keeps it).
        const u32 current = runs(app, ts, &getcwd, {0, 0});
        if (ts.terminate)
            return 0;
        for (u32 i = 0;; ++i)
        {
            const u8 c = byte(app, current + i);
            putByte(app, kTmpDir + i, c);
            if (c == 0)
                break;
        }
    }
    const u32 last = kTmpDir + length(kTmpDir) - 1;
    if (byte(app, last) != '\\' && byte(app, last) != '/')
    {
        putByte(app, last + 1, '\\');
        putByte(app, last + 2, 0);
    }
    return kTmpDir;
}

nfs::u32 tmpdir(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 directory;
    {
        const EspKept kept(ts);
        directory = tmpdirOf(app, ts);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, directory);
    return args.eax;
}

/* __MkTmpFile (clib/streamio/c/tmputil.c, 0x4ee168): a temporary file's name into `name` -- the
 * directory, then t, the process's id in four hexadecimal digits, _, two of `number`'s, .tmp.
 * At its entry's depth: five pushes and a dword.  What the code leaves in eax (the last digit);
 * 0 with ts.terminate set when the game stopped. */
u32 mkTmpFileOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 number)
{
    const GuestFrame frame(ts, 0x14, 4);
    u32 id;
    {
        const ReturnSlot call(ts);
        id = getpidOf(app, ts);
    }
    if (ts.terminate)
        return 0;
    u32 directory;
    {
        const ReturnSlot call(ts);
        directory = tmpdirOf(app, ts);
    }
    if (ts.terminate)
        return 0;
    u32 length = 0;
    for (;; ++length)
    {
        const u8 c = byte(app, directory + length);
        putByte(app, name + length, c);
        if (c == 0)
            break;
    }
    const u32 p = name + length;
    putByte(app, p, 't');
    u32 digits = id >> 16 | id;
    for (u32 at = p + 4; at != p; --at, digits >>= 4)
        putByte(app, at, u8(hexDigitOf(digits & 0xf)));
    putByte(app, p + 5, '_');
    putByte(app, p + 6, u8(hexDigitOf(u32(nfs::s32(number) >> 4) & 0xf)));
    const u32 last = hexDigitOf(number & 0xf);
    putByte(app, p + 7, u8(last));
    putByte(app, p + 8, '.');
    putByte(app, p + 9, 't');
    putByte(app, p + 0xa, 'm');
    putByte(app, p + 0xb, 'p');
    putByte(app, p + 0xc, 0);
    return last;
}

nfs::u32 MkTmpFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 eax;
    {
        const EspKept kept(ts);
        eax = mkTmpFileOf(app, ts, args.eax, args.edx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}


/* ---- setenv and the wide environment: the variables the game sets, kept
 * in the runtime's _environ -- and, once something asked for it, _wenviron
 * -- with a byte each of whether the runtime made the string, and so frees
 * it (__env_mask); and _nrealloc and _nexpand, which grow them, from the
 * heap's own blocks.  Written as their code is, like the environment's
 * above. */
namespace
{

constexpr u32 kWideEnviron = 0xa0b158;  // _wenviron
constexpr u32 kEnvMask = 0xa0b150;      // __env_mask: a byte each, 1 for a string the runtime made
constexpr u32 kHeapFirstBlock = 0x566f4c;  // __nheapbeg
constexpr u32 kHeapClean = 0xa0b141;    // __nheap_clean
constexpr u32 kHeapLock = 0x567780, kHeapUnlock = 0x567788;

/* The miniheap that has the block at `block`, as __HeapManager_expand finds it: from __nheapbeg on,
 * the last one if none; for another segment than ds, `esi` as it was. */
u32 heapOf(nfs::App* app, const nfs::ThreadState& ts, u16 segment, u32 block, u32 esi)
{
    (void)ts;
    if (segment != 0)  // ds: the flat model's 0
        return esi;
    u32 heap = at(app, kHeapFirstBlock);
    for (; at(app, heap + 8) != 0; heap = at(app, heap + 8))
        if (heap <= block && at(app, heap) + heap > block)
            return heap;
    return heap;
}

}  // namespace

/* _mbslen (clib/mbyte/c/mbslen.c, 0x518050): the characters in the string `s`. */
u32 mbslenOf(nfs::App* app, u32 s)
{
    u32 n = 0;
    for (; mbtermOf(app, s) == 0; s = mbsincOf(app, s))
        ++n;
    return n;
}

nfs::u32 mbslen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, mbslenOf(app, args.eax));
    return args.eax;
}

/* mbtowc (clib/mbyte/c/mbtowc.c, 0x523c00): the character at `from`, at most `most` bytes of it,
 * by MultiByteToWideChar into the word at `to`; its bytes, 0 for none, -1 for one it cannot make.
 * At its entry's depth: four pushes, then the dword MultiByteToWideChar writes the character into.
 * 0 with ts.terminate set when the game stopped. */
u32 mbtowcOf(nfs::App* app, nfs::ThreadState& ts, u32 to, u32 from, u32 most)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 wide = frame - 4;
    if (from == 0)
        return 0;
    if (most == 0)
        return 0xffffffff;
    if (byte(app, from) == 0)
    {
        if (to != 0)
            putHalf(app, to, 0);
        return 0;
    }
    if (multibyte(app) && leadByte(app, byte(app, from)) && byte(app, from + 1) == 0)
        return 0xffffffff;
    const u32 bytes = mbclenOf(app, from);
    u32 made;
    {
        const EspKept kept(ts);
        made = u32(os::text::widen(guest::text(app, from), int(most < bytes ? most : bytes),
                                   reinterpret_cast<nfs::u16*>(guest::memory(app, wide))));
    }
    if (made == 0)
        return 0xffffffff;
    if (to != 0)
        putHalf(app, to, u16(at(app, wide)));
    return bytes;
}

nfs::u32 mbtowc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 bytes;
    {
        const EspKept kept(ts);
        bytes = mbtowcOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, bytes);
    return args.eax;
}

/* mbstowcs (clib/mbyte/c/mbstowcs.c, 0x518080): the string at `from` into the wide string at `to`,
 * at most `most` characters -- or, for no buffer, how many it would take; -1 for one mbtowc cannot
 * make.  At its entry's depth: four pushes.  0 with ts.terminate set when the game stopped. */
u32 mbstowcsOf(nfs::App* app, nfs::ThreadState& ts, u32 to, u32 from, u32 most)
{
    const GuestFrame frame(ts, 0x10, 0);
    u32 made = 0;
    auto one = [&](u32 into) {
        const ReturnSlot call(ts);
        return mbtowcOf(app, ts, into, from, 2);
    };
    if (to != 0)
    {
        for (; most != 0; --most, from = mbsincOf(app, from), to += 2, ++made)
        {
            if (byte(app, from) == 0)
            {
                putHalf(app, to, 0);
                break;
            }
            const u32 bytes = one(to);
            if (ts.terminate)
                return 0;
            if (bytes == 0xffffffff)
                return bytes;
        }
        return made;
    }
    for (; byte(app, from) != 0; from = mbsincOf(app, from), ++made)
    {
        const u32 bytes = one(0);
        if (ts.terminate)
            return 0;
        if (bytes == 0xffffffff)
            return bytes;
    }
    return made;
}

nfs::u32 mbstowcs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 made;
    {
        const EspKept kept(ts);
        made = mbstowcsOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, made);
    return args.eax;
}

/* 0x525300 (wcslen): the characters in the wide string `s`. */
u32 wcslenOf(nfs::App* app, u32 s)
{
    u32 n = 0;
    while (half(app, s + 2 * n) != 0)
        ++n;
    return n;
}

nfs::u32 wcslen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, wcslenOf(app, args.eax));
    return args.eax;
}

/* The runtime's inline copy of `count` bytes from `from` to `to`: dwords, then the bytes left, each
 * read before it is written, as rep movsd and rep movsb take them. */
void copyForward(nfs::App* app, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count >> 2; ++i, to += 4, from += 4)
        put(app, to, at(app, from));
    for (u32 i = 0; i < (count & 3); ++i, ++to, ++from)
        putByte(app, to, byte(app, from));
}

/* 0x525320 (memcpy): `count` bytes from `from` to `to`; `to` back. */
u32 memcpyOf(nfs::App* app, u32 to, u32 from, u32 count)
{
    if (count != 0 && apart(to, count, from, count))
        std::memcpy(hostBytes(app, to), hostBytes(app, from), count);
    else
        copyForward(app, to, from, count);
    return to;
}

nfs::u32 memcpy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, memcpyOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525350 (wcscpy): the wide string at `from` to `to`; `to` back. */
u32 wcscpyOf(nfs::App* app, u32 to, u32 from)
{
    for (u32 i = 0;; i += 2)
    {
        const u16 c = half(app, from + i);
        putHalf(app, to + i, c);
        if (c == 0)
            break;
    }
    return to;
}

nfs::u32 wcscpy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, wcscpyOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x525370 (towupper, as the environment's functions have it): `c`'s ASCII small letters, in its
 * low word, as capitals -- the dword less 0x20. */
u32 towupperOf(u32 c)
{
    const u16 low = u16(c);
    return low >= 'a' && low <= 'z' ? c - 0x20 : c;
}

nfs::u32 towupper(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    returns(ts, args, towupperOf(args.eax));
    return args.eax;
}

/* 0x518640 (memmove): `count` bytes from `from` to `to` -- where they overlap the wrong way
 * backwards, words from the end, then the first byte; else forwards as memcpy does; `to` back. */
u32 memmoveOf(nfs::App* app, u32 to, u32 from, u32 count)
{
    if (from == to)
        return to;
    if (count != 0 && apart(to, count, from, count))
    {
        std::memcpy(hostBytes(app, to), hostBytes(app, from), count);
        return to;
    }
    if (from < to && from + count > to)
    {
        u32 s = from + count - 2, d = to + count - 2;
        for (u32 i = 0; i < count >> 1; ++i, s -= 2, d -= 2)
            putHalf(app, d, half(app, s));
        if (count & 1)
            putByte(app, d + 1, byte(app, s + 1));
        return to;
    }
    copyForward(app, to, from, count);
    return to;
}

nfs::u32 memmove(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, memmoveOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525380 (_msize): the block at `block`'s size, as its header has it. */
u32 msizeOf(nfs::App* app, u32 block)
{
    return (at(app, block - 4) & 0xfffffffe) - 4;
}

nfs::u32 msize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, msizeOf(app, args.eax));
    return args.eax;
}

/* 0x5253cc (__HeapManager_expand): the block at `block` in the heap of segment `segment` made
 * `size` bytes, from the free blocks after it or by giving its end back; how much more it still
 * needs into the dword at `more`.  0, 1 for a block after it in use, 2 for the heap's end.  `esi`
 * the caller's, which the miniheap stands for in another segment than ds.  At its entry's depth:
 * three pushes, then 0x10 bytes.  0 with ts.terminate set when the game stopped. */
u32 heapExpandOf(nfs::App* app, nfs::ThreadState& ts, u32 segment, u32 block, u32 size, u32 more,
                   u32 esi)
{
    const GuestFrame frame(ts, 0xc, 0x10);
    u32 need = (size + 0xb) & 0xfffffff8;
    if (need < size)
        need = 0xffffffff;
    else if (need < 0x10)
        need = 0x10;
    const u32 header = block - 4;
    const u32 has = at(app, header) & 0xfffffffe;
    if (need <= has)
    {
        // Smaller: what it no longer needs freed, if it is a block's worth.
        const u32 rest = has - need;
        if (rest >= 0x10)
        {
            put(app, header, need | 1);
            const u32 end = header + need;
            put(app, end, rest | 1);
            const u32 heap = heapOf(app, ts, u16(segment), block, esi);
            put(app, heap + 0x18, at(app, heap + 0x18) + 1);
            clib::free(app, ts, end + 4);
            if (ts.terminate)
                return 0;
        }
        return 0;
    }
    u32 wanted = need - has;
    for (u32 next = header + has;;)
    {
        put(app, more, wanted);
        const u32 free = at(app, next);
        if (free == 0xffffffff)
            return 2;
        if (free & 1)
            return 1;
        const u32 after = at(app, next + 8), before = at(app, next + 4);
        const u32 heap = heapOf(app, ts, u16(segment), block, esi);
        if (next == at(app, heap + 0xc))
            put(app, heap + 0xc, at(app, next + 4));
        if (free >= at(app, more) && free - at(app, more) >= 0x10)
        {
            // Split: the rest of the free block stays free after it.
            const u32 taken = at(app, more);
            const u32 rest = next + taken;
            put(app, rest, free - taken);
            put(app, rest + 4, before);
            put(app, rest + 8, after);
            put(app, before + 8, rest);
            put(app, after + 4, rest);
            put(app, header, at(app, header) + taken);
            putByte(app, kHeapClean, 0);
            return 0;
        }
        // All of the free block: off the free list, onto the block.
        put(app, before + 8, after);
        put(app, after + 4, before);
        put(app, header, at(app, header) + free);
        put(app, heap + 0x1c, at(app, heap + 0x1c) - 1);
        putByte(app, kHeapClean, 0);
        if (free >= at(app, more))
            return 0;
        wanted = at(app, more) - free;
        next += free;
    }
}

nfs::u32 HeapManager_expand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = heapExpandOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, args.esi);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x525578 (_nexpand's way out when it could not): the heap's lock given
 * back, 0, in _nexpand's frame. */
nfs::u32 nexpandFailed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!hookWith<kHeapUnlock>(app, ts, args))
        return args.eax;
    args.eax = 0;
    ts.sp += 4;
    args.esi = pop(app, ts);
    args.ecx = pop(app, ts);
    args.ebx = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* _nexpand (clib/heap/c/nexpand.c, 0x525390): the block `block` made `size` bytes where it is,
 * under the heap's lock; the block, or 0.  At its entry's depth: three pushes, then the dword
 * __HeapManager_expand counts what it still needs into.  0 with ts.terminate set when the game
 * stopped. */
u32 nexpandOf(nfs::App* app, nfs::ThreadState& ts, u32 block, u32 size)
{
    const GuestFrame frame(ts, 0xc, 4);
    const u32 more = frame - 4;
    if (!hook<kHeapLock>(app, ts))
        return 0;
    u32 failed;
    {
        const ReturnSlot slot(ts);
        failed = heapExpandOf(app, ts, 0, block, size, more, block);  // ds: the flat model's 0
    }
    if (ts.terminate || !hook<kHeapUnlock>(app, ts))
        return 0;
    return failed != 0 ? 0 : block;
}

nfs::u32 nexpand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 block = nexpandOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, block);
    return args.eax;
}

/* _nrealloc (clib/heap/c/nrealloc.c, 0x5185c0): the block `block` made `size` bytes -- where it is
 * if it can, else a new one with it copied, given back as it was where there is none -- _nmalloc's
 * for none, _nfree's for 0 bytes; the block, or 0.  At its entry's depth: five pushes.  0 with
 * ts.terminate set when the game stopped. */
u32 nreallocOf(nfs::App* app, nfs::ThreadState& ts, u32 block, u32 size)
{
    const GuestFrame frame(ts, 0x14, 0);
    if (block == 0)
        return clib::malloc(app, ts, size);
    if (size == 0)
    {
        clib::free(app, ts, block);
        return 0;
    }
    const u32 had = msizeOf(app, block);
    auto expand = [&](u32 bytes) {
        const ReturnSlot slot(ts);
        return nexpandOf(app, ts, block, bytes);
    };
    const u32 there = expand(size);
    if (ts.terminate || there != 0)
        return there;
    const u32 moved = clib::malloc(app, ts, size);
    if (ts.terminate)
        return 0;
    if (moved != 0)
    {
        copyForward(app, moved, block, had);
        clib::free(app, ts, block);
        return moved;
    }
    expand(had);
    return 0;
}

nfs::u32 nrealloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 block = nreallocOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, block);
    return args.eax;
}

/* 0x4f8f38 (__findenv): the variable named `name` in _environ, its name
 * matched as capitals: its index + 1 -- or, for no `value`, taken out of
 * _environ and __env_mask, the runtime's string freed, and the search going on
 * past the entry that moved into its place -- or, not there, minus how many
 * there are.  0 with ts.terminate set when the game stopped. */
u32 findenvOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 value)
{
    u32 entry = at(app, kEnviron);
    for (u32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
    {
        for (u32 s = string, n = name;; ++s, ++n)
        {
            if (byte(app, s) == '=' && byte(app, n) == 0)
            {
                const u32 index = u32(nfs::s32(entry - at(app, kEnviron)) >> 2);
                if (value != 0)
                    return index + 1;
                // Taken out: the entries after it one down, the terminator too.
                u32 end = entry, moved;
                do
                {
                    moved = at(app, end + 4);
                    put(app, end, moved);
                    end += 4;
                } while (moved != 0);
                const u32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, ts, string);
                        if (ts.terminate)
                            return 0;
                    }
                    // __env_mask, which follows the entries, after them again, its byte taken out.
                    const u32 count = u32(nfs::s32(end - at(app, kEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    if (ts.terminate)
                        return 0;
                    put(app, kEnvMask, end);
                    for (u32 i = index; nfs::s32(i) < nfs::s32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                break;  // '=' and the name's end: no capital alike, as the runtime goes on
            }
            if (clib::toupper(app, byte(app, s)) != clib::toupper(app, byte(app, n)) || byte(app, n) == 0)
                break;
        }
    }
    return u32(nfs::s32(at(app, kEnviron) - entry) >> 2);
}

nfs::u32 findenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 found = findenvOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, found);
    return args.eax;
}

/* __setenv (clib/environ/c/setenv.c, 0x4f8ce8): the variable named `name`
 * set to the string `value` in _environ -- replaced if `replace` says so,
 * taken out for none -- as a string of the runtime's, _environ and __env_mask
 * grown as it takes; 0, or -1.  0 with ts.terminate set when the game
 * stopped. */
u32 setenvOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 value, u32 replace)
{
    if (name == 0 || byte(app, name) == 0)
        return 0xffffffff;
    u32 environment = at(app, kEnviron);
    u32 index;
    if (environment == 0)
    {
        if (value == 0)
            return 0;
        // _environ made: no entry, its terminator, and __env_mask after it.
        environment = clib::malloc(app, ts, 9);
        if (ts.terminate)
            return 0;
        if (environment == 0)
            return 0xffffffff;
        put(app, environment, 0);
        put(app, kEnviron, environment);
        put(app, environment + 4, 0);
        put(app, kEnvMask, environment + 8);
        index = 0;
    }
    else
    {
        const u32 found = findenvOf(app, ts, name, value);
        if (ts.terminate)
            return 0;
        if (value == 0)
            return 0;
        if (nfs::s32(found) > 0)
        {
            if (replace == 0)
                return 0;
            index = found - 1;
        }
        else
        {
            // Not there: _environ one longer, __env_mask after it.
            index = 0 - found;
            const u32 entries = index + 1;
            const u32 masked = index * 4 + 8;  // where __env_mask goes in the block
            const u32 bytes = entries + masked;
            if (at(app, kEnvMask) == 0)
            {
                const u32 old = environment;
                environment = clib::malloc(app, ts, bytes);
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                for (u32 i = 0; i < index * 4; ++i)
                    putByte(app, environment + i, byte(app, old + i));
                put(app, kEnvMask, environment + masked);
                clib::memset(app, environment + masked, 0, entries);
            }
            else
            {
                environment = clib::realloc(app, ts, environment, bytes);
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memmoveOf(app, environment + masked, at(app, kEnvMask), index);
                if (ts.terminate)
                    return 0;
                put(app, kEnvMask, environment + masked);
            }
            put(app, environment + index * 4 + 4, 0);
            put(app, kEnviron, environment);
            putByte(app, at(app, kEnvMask) + index, 0);
        }
    }
    // name=value into a string of the runtime's: the old one grown, or anew.
    auto length = [&](u32 string) {
        u32 n = 0;
        while (byte(app, string + n) != 0)
            ++n;
        return n;
    };
    const u32 nameLength = length(name);
    const u32 old = byte(app, at(app, kEnvMask) + index) != 0 ? at(app, environment + index * 4) : 0;
    const u32 string = clib::realloc(app, ts, old, nameLength + length(value) + 2);
    if (ts.terminate)
        return 0;
    if (string == 0)
        return 0xffffffff;
    for (u32 i = 0; i < nameLength; ++i)
        putByte(app, string + i, byte(app, name + i));
    putByte(app, string + nameLength, '=');
    for (u32 i = 0;; ++i)
    {
        const u8 c = byte(app, value + i);
        putByte(app, string + nameLength + 1 + i, c);
        if (c == 0)
            break;
    }
    put(app, environment + index * 4, string);
    putByte(app, at(app, kEnvMask) + index, 1);
    return 0;
}

nfs::u32 setenv_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = setenvOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x518498 (__wfindenv): __findenv's for _wenviron, the wide name `name`: its index + 1 -- or,
 * for no `value`, taken out, the search going on past the entry that moved into its place -- or,
 * not there, minus how many there are.  0 with ts.terminate set when the game stopped. */
u32 wfindenvOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 value)
{
    u32 entry = at(app, kWideEnviron);
    for (u32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
    {
        for (u32 s = string, n = name;; s += 2, n += 2)
        {
            if (half(app, s) == '=' && half(app, n) == 0)
            {
                const u32 index = u32(nfs::s32(entry - at(app, kWideEnviron)) >> 2);
                if (value != 0)
                    return index + 1;
                u32 end = entry, moved;
                do
                {
                    moved = at(app, end + 4);
                    put(app, end, moved);
                    end += 4;
                } while (moved != 0);
                const u32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, ts, string);
                        if (ts.terminate)
                            return 0;
                    }
                    const u32 count = u32(nfs::s32(end - at(app, kWideEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    put(app, kEnvMask, end);
                    for (u32 i = index; nfs::s32(i) < nfs::s32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                break;  // '=' and the name's end: no capital alike, as the runtime goes on
            }
            if (u16(towupperOf(half(app, s))) != u16(towupperOf(half(app, n))) || half(app, n) == 0)
                break;
        }
    }
    return u32(nfs::s32(at(app, kWideEnviron) - entry) >> 2);
}

nfs::u32 wfindenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 found = wfindenvOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, found);
    return args.eax;
}

/* 0x52c300 (__wputenv's find): the wide "name=value" `pair`'s name in _wenviron, matched as
 * capitals: its index + 1 -- or, for `remove`, taken out, and 0 -- or minus how many there are.
 * 0 with ts.terminate set when the game stopped. */
u32 wputenvFindOf(nfs::App* app, nfs::ThreadState& ts, u32 pair, u32 remove)
{
    u32 entry = at(app, kWideEnviron);
    for (; at(app, entry) != 0; entry += 4)
    {
        if (half(app, pair) == 0)
            continue;
        for (u32 s = at(app, entry), n = pair;; s += 2, n += 2)
        {
            if (u16(towupperOf(half(app, s))) != u16(towupperOf(half(app, n))))
                break;
            if (half(app, s) == '=')
            {
                const u32 index = u32(nfs::s32(entry - at(app, kWideEnviron)) >> 2);
                if (remove == 0)
                    return index + 1;
                const u32 removed = at(app, entry);
                u32 end = entry;
                if (removed != 0)
                {
                    u32 moved;
                    do
                    {
                        put(app, end, at(app, end + 4));
                        moved = at(app, end + 4);
                        end += 4;
                    } while (moved != 0);
                }
                const u32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, ts, removed);
                        if (ts.terminate)
                            return 0;
                    }
                    const u32 count = u32(nfs::s32(end - at(app, kWideEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    put(app, kEnvMask, end);
                    for (u32 i = index; nfs::s32(i) < nfs::s32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                return 0;
            }
            if (half(app, n + 2) == 0)
                break;
        }
    }
    return u32(nfs::s32(at(app, kWideEnviron) - entry) >> 2);
}

nfs::u32 wputenvFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 found = wputenvFindOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, found);
    return args.eax;
}

/* 0x52c178 (__wputenv): the wide "name=value" `pair` into _wenviron as it is -- "name=" takes it
 * out -- _wenviron and __env_mask grown as it takes; 0, or -1.  For an empty `pair` the code looks
 * for its '=' at `edx` as the caller left it.  At its entry's depth: six pushes, then 8 bytes.  0
 * with ts.terminate set when the game stopped. */
u32 wputenvOf(nfs::App* app, nfs::ThreadState& ts, u32 pair, u32 edx)
{
    const GuestFrame frame(ts, 0x18, 8);
    if (pair == 0)
        return 0xffffffff;
    u32 equals = edx;
    if (half(app, pair) != 0)
        for (equals = pair + 2; half(app, equals) != 0 && half(app, equals) != '='; equals += 2)
            if (half(app, equals + 2) == 0)
            {
                equals += 2;
                break;
            }
    if (half(app, equals) == 0)
        return 0xffffffff;
    const u32 remove = half(app, equals + 2) == 0 ? 1 : 0;
    u32 environment = at(app, kWideEnviron);
    u32 index;
    if (environment == 0)
    {
        if (remove != 0)
            return 0;
        environment = clib::malloc(app, ts, 9);
        if (ts.terminate)
            return 0;
        if (environment == 0)
            return 0xffffffff;
        put(app, kWideEnviron, environment);
        put(app, environment, 0);
        put(app, environment + 4, 0);
        put(app, kEnvMask, environment + 8);
        index = 0;
    }
    else
    {
        u32 found;
        {
            const ReturnSlot slot(ts);
            found = wputenvFindOf(app, ts, pair, remove);
        }
        if (ts.terminate)
            return 0;
        if (remove != 0)
            return 0;
        if (nfs::s32(found) > 0)
            index = found - 1;
        else
        {
            index = 0 - found;
            const u32 entries = index + 1, masked = index * 4 + 8, bytes = masked + entries;
            if (at(app, kEnvMask) == 0)
            {
                const u32 old = environment;
                environment = clib::malloc(app, ts, bytes);
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memcpyOf(app, environment, old, index * 4);
                put(app, kEnvMask, environment + masked);
                clib::memset(app, environment + masked, 0, entries);
            }
            else
            {
                {
                    const ReturnSlot slot(ts);
                    environment = nreallocOf(app, ts, environment, bytes);
                }
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memmoveOf(app, environment + masked, at(app, kEnvMask), index);
                put(app, kEnvMask, environment + masked);
            }
            put(app, kWideEnviron, environment);
            put(app, environment + index * 4 + 4, 0);
        }
    }
    put(app, environment + index * 4, pair);
    putByte(app, index + at(app, kEnvMask), 0);
    return 0;
}

nfs::u32 wputenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = wputenvOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x5252a0 (__create_wide_environment): _wenviron made from _environ, a string at a time by
 * mbstowcs into a block of _nmalloc's, which __wputenv takes -- or, one mbstowcs cannot make,
 * _nfree.  At its entry's depth: four pushes.  What the code leaves in eax, given `eax` on entry;
 * 0 with ts.terminate set when the game stopped. */
u32 createWideEnvironmentOf(nfs::App* app, nfs::ThreadState& ts, u32 eax)
{
    const GuestFrame frame(ts, 0x10, 0);
    const u32 environment = at(app, kEnviron);
    if (environment == 0)
        return eax;
    for (u32 entry = environment; at(app, entry) != 0; entry += 4)
    {
        const u32 string = at(app, entry);
        const u32 characters = mbslenOf(app, string) + 1;
        const u32 wide = clib::malloc(app, ts, characters * 2);
        if (ts.terminate)
            return 0;
        eax = wide;
        if (wide == 0)
            continue;
        u32 made;
        {
            const ReturnSlot slot(ts);
            made = mbstowcsOf(app, ts, wide, string, characters);
        }
        if (ts.terminate)
            return 0;
        if (made != 0xffffffff)
        {
            const ReturnSlot slot(ts);
            eax = wputenvOf(app, ts, wide, string);
        }
        else
            eax = clib::free(app, ts, wide);
        if (ts.terminate)
            return 0;
    }
    return eax;
}

nfs::u32 create_wide_environment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = createWideEnvironmentOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* __wsetenv (clib/environ/c/wsetenv.c, 0x518290): __setenv's for _wenviron -- the wide variable
 * `name` set to `value`, replaced if `replace` says so, taken out for none -- made from _environ
 * first if there is none; 0, or -1.  At its entry's depth: four pushes, then 0x14 bytes.  0 with
 * ts.terminate set when the game stopped. */
u32 wsetenvOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 value, u32 replace)
{
    const GuestFrame frame(ts, 0x10, 0x14);
    if (name == 0 || half(app, name) == 0)
        return 0xffffffff;
    if (at(app, kWideEnviron) == 0)
    {
        {
            const ReturnSlot slot(ts);
            createWideEnvironmentOf(app, ts, 0);
        }
        if (ts.terminate)
            return 0;
    }
    u32 environment = at(app, kWideEnviron);
    u32 index;
    if (environment == 0)
    {
        if (value == 0)
            return 0;
        environment = clib::malloc(app, ts, 9);
        if (ts.terminate)
            return 0;
        if (environment == 0)
            return 0xffffffff;
        put(app, environment, 0);
        put(app, kWideEnviron, environment);
        put(app, environment + 4, 0);
        put(app, kEnvMask, environment + 8);
        index = 0;
    }
    else
    {
        const u32 found = wfindenvOf(app, ts, name, value);
        if (ts.terminate)
            return 0;
        if (value == 0)
            return 0;
        if (nfs::s32(found) > 0)
        {
            if (replace == 0)
                return 0;
            index = found - 1;
        }
        else
        {
            index = 0 - found;
            const u32 entries = index + 1, masked = index * 4 + 8, bytes = masked + entries;
            if (at(app, kEnvMask) == 0)
            {
                const u32 old = environment;
                environment = clib::malloc(app, ts, bytes);
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memcpyOf(app, environment, old, index * 4);
                put(app, kEnvMask, environment + masked);
                clib::memset(app, environment + masked, 0, entries);
            }
            else
            {
                {
                    const ReturnSlot slot(ts);
                    environment = nreallocOf(app, ts, environment, bytes);
                }
                if (ts.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memmoveOf(app, environment + masked, at(app, kEnvMask), index);
                put(app, kEnvMask, environment + masked);
            }
            put(app, environment + index * 4 + 4, 0);
            put(app, kWideEnviron, environment);
            putByte(app, index + at(app, kEnvMask), 0);
        }
    }
    // name=value into a wide string of the runtime's: the old one grown, or anew.
    const u32 nameLength = wcslenOf(app, name);
    const u32 old = byte(app, index + at(app, kEnvMask)) != 0 ? at(app, environment + index * 4) : 0;
    u32 string;
    {
        const ReturnSlot slot(ts);
        string = nreallocOf(app, ts, old, (wcslenOf(app, value) + nameLength) * 2 + 4);
    }
    if (ts.terminate)
        return 0;
    if (string == 0)
        return 0xffffffff;
    memcpyOf(app, string, name, nameLength * 2);
    putHalf(app, string + nameLength * 2, '=');
    wcscpyOf(app, string + nameLength * 2 + 2, value);
    put(app, environment + index * 4, string);
    putByte(app, index + at(app, kEnvMask), 1);
    return 0;
}

nfs::u32 wsetenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = wsetenvOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* setenv (clib/environ/c/setenv.c, 0x4f8b50): the variable `name` set to the string `value` --
 * replaced if `replace` says so, an empty one taking it out -- in Windows' environment by
 * SetEnvironmentVariableA, in _environ, and in _wenviron if there is one, its name and value made
 * wide by mbstowcs; 0, or -1.  At its entry's depth: four pushes, then 0x10 bytes.  0 with
 * ts.terminate set when the game stopped. */
u32 setenvTopOf(nfs::App* app, nfs::ThreadState& ts, u32 name, u32 value, u32 replace)
{
    const GuestFrame frame(ts, 0x10, 0x10);
    auto lookup = [&] {
        const ReturnSlot slot(ts);
        return getenvOf(app, ts, name);
    };
    if (value != 0 && byte(app, value) == 0)
    {
        bool none = replace != 0;
        if (!none)
        {
            none = lookup() == 0;
            if (ts.terminate)
                return 0;
        }
        if (none)
            value = 0;
    }
    bool toWindows = true;
    if (replace == 0)
    {
        toWindows = lookup() == 0;
        if (ts.terminate)
            return 0;
    }
    // The port keeps no environment of its own: setting it there fails.
    if (toWindows)
        return 0xffffffff;
    u32 failed;
    {
        const ReturnSlot slot(ts);
        failed = setenvOf(app, ts, name, value, replace);
    }
    if (ts.terminate)
        return 0;
    if (failed != 0)
        return 0xffffffff;
    if (at(app, kWideEnviron) == 0)
        return 0;
    auto noMemory = [&] {
        called(ts, [&] { return setErrnoDos(app, ts, 5); });  // __set_errno_dos's ENOMEM
        return ts.terminate ? 0 : 0xffffffff;
    };
    const u32 nameCharacters = mbslenOf(app, name) + 1;
    const u32 wideName = clib::malloc(app, ts, nameCharacters * 2);
    if (ts.terminate)
        return 0;
    if (wideName == 0)
        return noMemory();
    u32 valueCharacters = 0, wideValue = 0;
    if (value != 0)
    {
        valueCharacters = mbslenOf(app, value) + 1;
        wideValue = clib::malloc(app, ts, valueCharacters * 2);
        if (ts.terminate)
            return 0;
        if (wideValue == 0)
        {
            clib::free(app, ts, wideName);
            if (ts.terminate)
                return 0;
            return noMemory();
        }
    }
    auto freeBoth = [&] {
        clib::free(app, ts, wideName);
        if (!ts.terminate)
            clib::free(app, ts, wideValue);
        return ts.terminate ? 0 : 0xffffffff;
    };
    auto widen = [&](u32 into, u32 from, u32 characters) {
        const ReturnSlot slot(ts);
        return mbstowcsOf(app, ts, into, from, characters);
    };
    if (widen(wideName, name, nameCharacters) == 0xffffffff)
        return freeBoth();
    if (ts.terminate)
        return 0;
    if (wideValue != 0)
    {
        if (widen(wideValue, value, valueCharacters) == 0xffffffff)
            return freeBoth();
        if (ts.terminate)
            return 0;
    }
    u32 result;
    {
        const ReturnSlot slot(ts);
        result = wsetenvOf(app, ts, wideName, wideValue, replace);
    }
    if (ts.terminate)
        return 0;
    clib::free(app, ts, wideName);
    if (ts.terminate)
        return 0;
    if (wideValue != 0)
    {
        clib::free(app, ts, wideValue);
        if (ts.terminate)
            return 0;
    }
    return result;
}

nfs::u32 setenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = setenvTopOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}


/* ---- Formatted input: sscanf, __scnf and its conversions, strtod and the
 * long double it builds the number in -- its decimal digits made binary, then
 * scaled by powers of ten on the x87.
 *
 * __scnf reads through the two routines its specs hold -- sscanf's take the
 * string's characters, and put one back -- and each conversion takes its
 * argument's pointer as the code does: the va_list moved on, the store
 * through ds (an 'F' one's les the game's code has not either).  Each is a
 * typed core (fooOf) and its thunk for the calls by address; a core keeps
 * the frame the code had where it hands the frame's addresses on -- a width
 * the format gives no limit to lets the float's conversion run over its buffer
 * on the stack, and that is the code's too -- and is called at its code's
 * depth.  The long double is the port's Float80, its x87 operations the same
 * the game's code has for them, so a number comes out to the bit as the
 * game's code makes it. ---- */
namespace
{

// The specs __scnf hands its conversions: the routines that take a character
// and put one back, the string sscanf reads, the field's width, its flags.
constexpr u32 kScanGet = 0x0, kScanUnget = 0x4, kScanString = 0x8, kScanWidth = 0xc, kScanFlags = 0x10;
constexpr u8 kAssign = 0x01, kEndOfInput = 0x02, kFarPointer = 0x04, kNearPointer = 0x08;
constexpr u8 kShortVar = 0x10, kLongVar = 0x20, kLongLongVar = 0x40, kSegmented = 0x80;
constexpr u32 kBitMasks = 0x5688a0;      // 1, 2, 4 ... 0x80: a [set]'s bits
constexpr u32 kScanFloat = 0x56ac68;     // __EFG_scanf: the string to a double
constexpr u32 kPowersOfTen = 0x568ba4;   // 10^1, 10^2, 10^4 ... as long doubles, 10 bytes each
constexpr u32 kHugeVal = 0x55217c;       // HUGE_VAL, a double
constexpr u32 kCgetString = 0x4f04f0, kUncgetString = 0x4f051c;

u8 scanFlags(nfs::App* app, u32 specs)
{
    return byte(app, specs + kScanFlags);
}

/* The va_list at `argp` moved on past a conversion's pointer: `step` its new place; the pointer in
 * `target` -- unless it is a far one, whose les is not the game's code's either, `target` then
 * left as it is.  The code keeps the two in registers, and where they are the same register the
 * pointer is the va_list's place: callers alias them as their code does. */
void scanArgument(nfs::App* app, u32 argp, bool far, u32& step, u32& target)
{
    step = at(app, argp) + (far ? 8 : 4);
    put(app, argp, step);
    if (far)
    {
        NFS2_ASSERT(false);  // les
        return;
    }
    target = at(app, step - 4);
}

/* A conversion's pointer as scan_char's and scan_string's code takes it: the far one's register
 * left as it was (`esi`). */
u32 scanPointer(nfs::App* app, u8 flags, u32 argp, u32 esi)
{
    u32 step, target = esi;
    scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
    return target;
}

void scanEpilogue(nfs::App* app, nfs::ThreadState& ts, std::initializer_list<u32*> registers)
{
    for (u32* r : registers)
        *r = pop(app, ts);
    ts.sp += 4;
}

/* 96 bits, edx:ecx:ebp, shifted left by one: the carry out. */
bool shift96(u32& high, u32& middle, u32& low)
{
    const bool carry = high >> 31;
    high = high << 1 | middle >> 31;
    middle = middle << 1 | low >> 31;
    low <<= 1;
    return carry;
}

/* 96 bits added to 96: the carry out. */
bool add96(u32& high, u32& middle, u32& low, u32 high2, u32 middle2, u32 low2)
{
    const nfs::u64 l = nfs::u64(low) + low2;
    const nfs::u64 m = nfs::u64(middle) + middle2 + (l >> 32);
    const nfs::u64 h = nfs::u64(high) + high2 + (m >> 32);
    low = u32(l);
    middle = u32(m);
    high = u32(h);
    return (h >> 32) != 0;
}

}  // namespace

/* 0x4f04f0 (cget_string): sscanf's next character, from the string in its specs `specs`, or -1 at
 * its end, which the specs' flags note. */
u32 cgetStringOf(nfs::App* app, u32 specs)
{
    const u32 s = at(app, specs + kScanString);
    if (const u8 c = byte(app, s))
    {
        put(app, specs + kScanString, s + 1);
        return c;
    }
    putByte(app, specs + kScanFlags, byte(app, specs + kScanFlags) | kEndOfInput);
    return 0xffffffff;
}

nfs::u32 cget_string(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, cgetStringOf(app, args.eax));
    return args.eax;
}

/* 0x4f051c (uncget_string): the character back -- the string's pointer in the specs `specs` back by
 * one. */
void uncgetStringOf(nfs::App* app, u32 specs)
{
    put(app, specs + kScanString, at(app, specs + kScanString) - 1);
}

nfs::u32 uncget_string(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    uncgetStringOf(app, args.edx);  // eax left as it was, the character
    ts.sp += 4;
    return args.eax;
}

/* 0x50c640 (cgetc): the next character, by the specs' routine -- the game's, by address, eax and
 * edx the specs as the code hands them.  At its entry's depth: a push.  0 with ts.terminate set
 * when the game stopped. */
u32 cgetcOf(nfs::App* app, nfs::ThreadState& ts, u32 specs)
{
    const GuestFrame frame(ts, 4, 0);
    const EspKept kept(ts);
    return through(app, ts, at(app, specs + kScanGet), {specs, specs}).value_or(0);
}

nfs::u32 cgetc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 c = cgetcOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, c);
    return args.eax;
}

/* 0x50c648 (uncget): the character `c` back, by the specs' routine -- the game's, by address, ebx
 * and edx the specs.  At its entry's depth: a push.  What the routine leaves in eax; 0 with
 * ts.terminate set when the game stopped. */
u32 uncgetOf(nfs::App* app, nfs::ThreadState& ts, u32 c, u32 specs)
{
    const GuestFrame frame(ts, 4, 0);
    const EspKept kept(ts);
    return through(app, ts, at(app, specs + kScanUnget), {c, specs, specs}).value_or(0);
}

nfs::u32 uncget(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = uncgetOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x50d538 (chktrm): the next character while the field's width lasts -- -1 when it is over, or
 * the input is.  At its entry's depth: two pushes.  0 with ts.terminate set when the game
 * stopped. */
u32 chktrmOf(nfs::App* app, nfs::ThreadState& ts, u32 specs)
{
    const GuestFrame frame(ts, 8, 0);
    const u32 width = at(app, specs + kScanWidth);
    put(app, specs + kScanWidth, width - 1);
    if (width == 0)
        return 0xffffffff;
    u32 c;
    {
        const ReturnSlot slot(ts);
        c = cgetcOf(app, ts, specs);
    }
    if (ts.terminate)
        return 0;
    return scanFlags(app, specs) & kEndOfInput ? 0xffffffff : c;
}

nfs::u32 chktrm(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 c = chktrmOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, c);
    return args.eax;
}

/* 0x50d510 (radix_value): the digit `c` is, in any base up to 16; 16 for none. */
u32 radixValueOf(nfs::App* app, u32 c)
{
    if (nfs::s32(c) >= 0x30 && nfs::s32(c) <= 0x39)
        return c - 0x30;
    c = clib::tolower(app, c);
    if (nfs::s32(c) >= 0x61 && nfs::s32(c) <= 0x66)
        return c - 0x57;
    return 0x10;
}

nfs::u32 radix_value(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, radixValueOf(app, args.eax));
    return args.eax;
}

/* 0x50c904 (get_opt): the format's options after a '%', at `p`, into the specs `specs` -- '*', the
 * width, N or F, then h, l, w, L or I64; past them. */
u32 getOptOf(nfs::App* app, u32 p, u32 specs)
{
    u8 flags = (byte(app, specs + kScanFlags) | kAssign) & 3;
    put(app, specs + kScanWidth, 0xffffffff);
    putByte(app, specs + kScanFlags, flags);
    if (byte(app, p) == '*')
    {
        flags &= 0xfe;
        ++p;
        putByte(app, specs + kScanFlags, flags);
    }
    auto digit = [&](u32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kDigit) != 0; };
    if (digit(byte(app, p)))
    {
        u32 width = 0;
        for (u32 c = byte(app, p); digit(c); c = byte(app, ++p))
            width = width * 10 + (c - 0x30);
        put(app, specs + kScanWidth, width);
    }
    auto add = [&](u8 flag, u32 past) {
        putByte(app, specs + kScanFlags, byte(app, specs + kScanFlags) | flag);
        p += past;
    };
    if (byte(app, p) == 'N')
        add(kNearPointer, 1);
    else if (byte(app, p) == 'F')
        add(kFarPointer, 1);
    const u8 size = byte(app, p);
    if (size == 'h')
        add(kShortVar, 1);
    else if (size == 'l' || size == 'w')
        add(kLongVar, 1);
    else if (size == 'L')
        add(kLongLongVar, 1);
    else if (size == 'I' && byte(app, p + 1) == '6' && byte(app, p + 2) == '4')
        add(kLongLongVar, 3);
    return p;
}

nfs::u32 get_opt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, getOptOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50ca08 (scan_white): white space skipped: how much; the character after it put back, unless
 * the input is over.  At its entry's depth: four pushes and a dword.  0 with ts.terminate set when
 * the game stopped. */
u32 scanWhiteOf(nfs::App* app, nfs::ThreadState& ts, u32 specs)
{
    const GuestFrame frame(ts, 0x10, 4);
    u32 skipped = 0, c;
    for (;; ++skipped)
    {
        {
            const ReturnSlot slot(ts);
            c = cgetcOf(app, ts, specs);
        }
        if (ts.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
    }
    if (!(scanFlags(app, specs) & kEndOfInput))
    {
        {
            const ReturnSlot slot(ts);
            uncgetOf(app, ts, c, specs);
        }
        if (ts.terminate)
            return 0;
    }
    return skipped;
}

nfs::u32 scan_white(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 skipped = scanWhiteOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, skipped);
    return args.eax;
}

/* 0x50ca48 (scan_char): %c and %C: the field's width of characters, 1 for none, into the argument
 * from the va_list at `argp` -- %C's made wide characters by mbtowc.  How many were read; 0 for a
 * character mbtowc could not make.  `esi` as the caller left it, which a far pointer leaves the
 * target.  At its entry's depth: four pushes, es, a push, then 8 bytes: mbtowc's character and
 * its bytes.  0 with ts.terminate set when the game stopped. */
u32 scanCharOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 argp, u32 esi)
{
    const GuestFrame frame(ts, 0x18, 8);
    const u32 wide = frame - 8, bytes = frame - 4;
    u32 target = esi;
    if (scanFlags(app, specs) & kAssign)
        target = scanPointer(app, scanFlags(app, specs), argp, esi);
    u32 width = at(app, specs + kScanWidth);
    if (width == 0xffffffff)
        width = 1;
    u32 read = 0;
    auto next = [&] {
        const ReturnSlot slot(ts);
        return cgetcOf(app, ts, specs);
    };
    for (; nfs::s32(width) > 0; --width)
    {
        const u32 c = next();
        if (ts.terminate)
            return 0;
        const u8 flags = scanFlags(app, specs);
        if (flags & kEndOfInput)
            break;
        ++read;
        if (!(flags & kAssign))
            continue;
        if (!(flags & kLongVar))
        {
            putByte(app, target++, u8(c));
            continue;
        }
        putByte(app, bytes, u8(c));
        if (multibyte(app) && leadByte(app, c))
        {
            const u32 trail = next();
            if (ts.terminate)
                return 0;
            putByte(app, bytes + 1, u8(trail));
        }
        u32 made;
        {
            const ReturnSlot slot(ts);
            made = mbtowcOf(app, ts, wide, bytes, 2);
        }
        if (ts.terminate)
            return 0;
        if (made == 0xffffffff)
            return 0;
        putHalf(app, target, u16(at(app, wide)));
        target += 2;
    }
    return read;
}

nfs::u32 scan_char(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 read = scanCharOf(app, ts, args.eax, args.edx, args.esi);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, read);
    return args.eax;
}

/* 0x50cb30 (scan_string): %s and %S: white space skipped, then characters to the next, the field's
 * width at most, into the argument from the va_list at `argp` with a 0 after -- %S's wide.  How
 * many were read; 0 for a character mbtowc could not make.  `esi` as for scan_char.  At its entry's
 * depth: four pushes, es, a push, then 0xc bytes: mbtowc's character, its bytes, the size.  0 with
 * ts.terminate set when the game stopped. */
u32 scanStringOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 argp, u32 esi)
{
    const GuestFrame frame(ts, 0x18, 0xc);
    const u32 wide = frame - 0xc, bytes = frame - 8;
    const u32 size = scanFlags(app, specs) & kLongVar ? 2 : 1;
    putByte(app, frame - 4, u8(size));
    u32 target = esi;
    if (scanFlags(app, specs) & kAssign)
        target = scanPointer(app, scanFlags(app, specs), argp, esi);
    auto next = [&] {
        const ReturnSlot slot(ts);
        return cgetcOf(app, ts, specs);
    };
    auto space = [&](u32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace) != 0; };
    u32 read = 0, c;
    for (;; ++read)
    {
        c = next();
        if (ts.terminate)
            return 0;
        if (!space(c))
            break;
    }
    bool putBack = false;
    if (scanFlags(app, specs) & kEndOfInput)
        read = 0;
    else
    {
        putBack = true;
        const u32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width != 0)
            for (;;)
            {
                ++read;
                if (scanFlags(app, specs) & kAssign)
                {
                    if (size == 1)
                        putByte(app, target, u8(c));
                    else
                    {
                        putByte(app, bytes, u8(c));
                        if (multibyte(app) && leadByte(app, c))
                        {
                            const u32 trail = next();
                            if (ts.terminate)
                                return 0;
                            putByte(app, bytes + 1, u8(trail));
                        }
                        u32 made;
                        {
                            const ReturnSlot slot(ts);
                            made = mbtowcOf(app, ts, wide, bytes, 2);
                        }
                        if (ts.terminate)
                            return 0;
                        if (made == 0xffffffff)
                            return 0;
                        putHalf(app, target, u16(at(app, wide)));
                    }
                    target += size;
                }
                {
                    const ReturnSlot slot(ts);
                    c = chktrmOf(app, ts, specs);
                }
                if (ts.terminate)
                    return 0;
                if (c == 0xffffffff)
                {
                    putBack = false;
                    break;
                }
                if (space(c))
                    break;
            }
    }
    if (putBack)
    {
        {
            const ReturnSlot slot(ts);
            uncgetOf(app, ts, c, specs);
        }
        if (ts.terminate)
            return 0;
    }
    if ((scanFlags(app, specs) & kAssign) && nfs::s32(read) > 0)
    {
        if (size == 1)
            putByte(app, target, 0);
        else
            putHalf(app, target, 0);
    }
    return read;
}

nfs::u32 scan_string(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 read = scanStringOf(app, ts, args.eax, args.edx, args.esi);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, read);
    return args.eax;
}

/* 0x50cc88 (report_scan): %n: the characters read so far, `read`, into the argument from the va_list
 * at `argp` -- a short for h.  A far pointer's register is the va_list's place, which the code
 * then stores through. */
void reportScanOf(nfs::App* app, u32 specs, u32 argp, u32 read)
{
    const u8 flags = scanFlags(app, specs);
    if (!(flags & kAssign))
        return;
    u32 step, target = argp;
    scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
    if (scanFlags(app, specs) & kShortVar)
        putHalf(app, target, u16(read));
    else
        put(app, target, read);
}

nfs::u32 report_scan(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    reportScanOf(app, args.eax, args.edx, args.ebx);  // eax left as it was, the specs
    ts.sp += 4;
    return args.eax;
}

/* 0x50cce8 (makelist): a [set]'s characters, the format's at `p`, as bits of the 32 bytes at
 * `set`; past its ']' -- one first is the set's. */
u32 makelistOf(nfs::App* app, u32 p, u32 set)
{
    clib::memset(app, set, 0, 0x20);
    for (u32 c = byte(app, p++); c != 0;)
    {
        const u32 at = set + u32(nfs::s32(c) >> 3);
        putByte(app, at, byte(app, at) | byte(app, kBitMasks + (c & 7)));
        c = byte(app, p);
        if (c == 0)
            break;
        ++p;
        if (c == ']')
            break;
    }
    return p;
}

nfs::u32 makelist(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, makelistOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50cd2c (scan_arb): %[: characters in the set -- out of it, for a '^' first -- the field's width
 * at most, into the argument from the va_list at `argp` with a 0 after; the format's pointer, in
 * the dword at `format`, moved past the set.  How many were read.  A far pointer's register is the
 * va_list's new place, which the code then stores through.  At its entry's depth: three pushes,
 * es, a push, then 0x30 bytes, the set's 32 first.  0 with ts.terminate set when the game
 * stopped. */
u32 scanArbOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 argp, u32 format)
{
    const GuestFrame frame(ts, 0x14, 0x30);
    const u32 set = frame - 0x30;
    const u32 outside = byte(app, at(app, format)) == '^' ? 1 : 0;
    if (outside)
        put(app, format, at(app, format) + 1);
    put(app, format, makelistOf(app, at(app, format), set));
    u32 target = 0;
    if (scanFlags(app, specs) & kAssign)
    {
        u32 step;
        if (scanFlags(app, specs) & kFarPointer)
        {
            scanArgument(app, argp, true, step, target);
            target = step;
        }
        else
            scanArgument(app, argp, false, step, target);
    }
    u32 read = 0;
    for (u32 width = at(app, specs + kScanWidth); width != 0; --width)
    {
        u32 c;
        {
            const ReturnSlot slot(ts);
            c = cgetcOf(app, ts, specs);
        }
        if (ts.terminate)
            return 0;
        if (scanFlags(app, specs) & kEndOfInput)
            break;
        const bool in = (byte(app, set + u32(nfs::s32(c) >> 3)) & byte(app, kBitMasks + (c & 7))) != 0;
        if ((in ? 0 : 1) != outside)
        {
            {
                const ReturnSlot slot(ts);
                uncgetOf(app, ts, c, specs);
            }
            if (ts.terminate)
                return 0;
            break;
        }
        ++read;
        if (scanFlags(app, specs) & kAssign)
            putByte(app, target++, u8(c));
    }
    if ((scanFlags(app, specs) & kAssign) && nfs::s32(read) > 0)
        putByte(app, target, 0);
    return read;
}

nfs::u32 scan_arb(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 read = scanArbOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, read);
    return args.eax;
}

/* 0x523ca0 (__FDFS): the double `bits` as a float's bits: 0 for one too small, its sign and
 * infinity for one too big. */
u32 fdfsOf(nfs::u64 bits)
{
    if (!(u32(bits >> 32) & 0x7ff00000))
        return 0;
    const u32 sign = u32(bits >> 63) << 31;
    const nfs::u64 shifted = (bits << 1) + 0x20000000;
    const u32 high = u32(shifted >> 32);
    if (high == 0 || high >= 0x8fe00000)
        return 0x7f800000 | sign;
    if (high < 0x70200000)
        return 0;
    return u32(((nfs::u64(high - 0x70000000) << 32 | u32(shifted)) << 2) >> 32) | sign;
}

nfs::u32 FDFS(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    returns(ts, args, fdfsOf(nfs::u64(args.edx) << 32 | args.eax));
    return args.eax;
}

/* 0x523cec (__U8M): `a` times `b`, 64 bits of it; `cross` the cross products' sum, which the code
 * leaves in ecx. */
nfs::u64 u8mOf(nfs::u64 a, nfs::u64 b, u32& cross)
{
    cross = u32(a) * u32(b >> 32) + u32(a >> 32) * u32(b);
    return nfs::u64(u32(a)) * u32(b) + (nfs::u64(cross) << 32);
}

nfs::u32 U8M(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    // Its own convention: edx:eax and ecx:ebx in, edx:eax out -- ecx the cross products' sum, but
    // for two numbers of 32 bits, which leave it as it was.
    u32 cross;
    const nfs::u64 product = u8mOf(nfs::u64(args.edx) << 32 | args.eax, nfs::u64(args.ecx) << 32 | args.ebx, cross);
    if (args.edx != 0 || args.ecx != 0)
        args.ecx = cross;
    args.eax = u32(product);
    args.edx = u32(product >> 32);
    ts.sp += 4;
    return args.eax;
}

/* 0x50ce18 (scan_float): %e, %f, %g, %E and %G: a number's characters -- sign, digits, a point,
 * digits, an exponent -- gathered in the frame, then made a double by __EFG_scanf and stored, as a
 * float but for l and L, into the argument from the va_list at `argp`; with h, the code's fixed
 * point, a short whole part and a short fraction.  How many characters were read; 0 for none of a
 * number.  At its entry's depth: four pushes, es, a push, then 0x6c bytes: the characters from the
 * bottom -- as many as the field gives, its end the code's to overrun -- the double at +0x50, and
 * the fixed point's words, which overlap as the code has them, at +0x5c on.  0 with ts.terminate
 * set when the game stopped. */
u32 scanFloatOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 argp)
{
    const GuestFrame frame(ts, 0x18, 0x6c);
    const u32 local = frame - 0x6c;
    const u32 number = local + 0x50, ten = local + 0x5c, fraction = local + 0x60, digit = local + 0x62;
    const u32 fixed = local + 0x64, whole = local + 0x66, skipped = local + 0x68;
    auto isDigit = [&](u32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kDigit) != 0; };
    u32 p = local, read = 0, c;
    put(app, skipped, 0);
    // The character into the buffer, the next one taken; false at -1.
    auto keep = [&] {
        putByte(app, p++, u8(c));
        const ReturnSlot slot(ts);
        c = chktrmOf(app, ts, specs);
        return c != 0xffffffff;
    };
    auto next = [&] {
        const ReturnSlot slot(ts);
        c = chktrmOf(app, ts, specs);
        return c != 0xffffffff;
    };
    bool digits = false;
    for (;;)
    {
        {
            const ReturnSlot slot(ts);
            c = cgetcOf(app, ts, specs);
        }
        if (ts.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
        put(app, skipped, at(app, skipped) + 1);
    }
    if (scanFlags(app, specs) & kEndOfInput)
        goto stored;
    {
        const u32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width == 0)
            goto putBack;
    }
    if (c == '+' || c == '-')
    {
        putByte(app, p++, u8(c));
        const bool more = next();
        if (ts.terminate)
            return 0;
        put(app, skipped, at(app, skipped) + 1);
        if (!more)
            goto stored;
    }
    if (!isDigit(c) && c != '.')
        goto putBack;
    put(app, fixed, 0);
    if (isDigit(c))
    {
        digits = true;
        do
        {
            putByte(app, p++, u8(c));
            if (scanFlags(app, specs) & kShortVar)
                putHalf(app, whole, u16(c + at(app, whole) * 10 - 0x30));
            ++read;
            const bool more = next();
            if (ts.terminate)
                return 0;
            if (!more)
                goto stored;
        } while (isDigit(c));
    }
    if (c == '.')
    {
        bool more = keep();
        if (ts.terminate)
            return 0;
        if (!more)
            goto stored;
        if (!digits && !isDigit(c))
            goto putBack;
        ++read;
        while (isDigit(c))
        {
            ++read;
            more = keep();
            if (ts.terminate)
                return 0;
            if (!more)
                break;
        }
        if (scanFlags(app, specs) & kShortVar)
        {
            // The fraction's digits, back from the last to the point, made a short's fraction:
            // each into the third byte of the dword, over 10, as the code divides them.
            put(app, fraction, 0);
            for (u32 q = p; byte(app, q - 1) != '.'; --q)
            {
                put(app, ten, 10);
                putByte(app, digit, u8(byte(app, q - 1) - 0x30));
                put(app, fraction, at(app, fraction) / 10);
            }
            putHalf(app, fixed, u16(at(app, fraction)));
        }
        if (c == 0xffffffff)
            goto stored;
    }
    if (scanFlags(app, specs) & kShortVar)
        goto putBack;
    if (c != 'e' && c != 'E')
        goto putBack;
    ++read;
    {
        bool more = keep();
        if (ts.terminate)
            return 0;
        if (!more)
            goto stored;
        if (c == '+' || c == '-')
        {
            ++read;
            more = keep();
            if (ts.terminate)
                return 0;
            if (!more)
                goto stored;
        }
    }
    if (!isDigit(c))
    {
        read = 0;
        goto putBack;
    }
    do
    {
        ++read;
        const bool more = keep();
        if (ts.terminate)
            return 0;
        if (!more)
            goto stored;
    } while (isDigit(c));
putBack:
    {
        const ReturnSlot slot(ts);
        uncgetOf(app, ts, c, specs);
    }
    if (ts.terminate)
        return 0;
stored:
    if (nfs::s32(read) > 0)
    {
        read += at(app, skipped);
        if (scanFlags(app, specs) & kAssign)
        {
            putByte(app, p, 0);
            if (scanFlags(app, specs) & kShortVar)
            {
                if (byte(app, local) == '-')
                    put(app, fixed, 0 - at(app, fixed));
            }
            else
            {
                const EspKept kept(ts);
                if (!hook<kScanFloat>(app, ts, {local, number}))
                    return 0;
            }
            const u8 flags = scanFlags(app, specs);
            u32 step, target = 0;
            scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
            if (flags & kFarPointer)
                target = step;
            const u8 size = scanFlags(app, specs);
            const u32 to = target;  // es and ds: the flat model's 0
            if (size & kShortVar)
                put(app, to, at(app, fixed));
            else if (size & (kLongVar | kLongLongVar))
            {
                put(app, to, at(app, number));
                put(app, to + 4, at(app, number + 4));
            }
            else
                put(app, to, fdfsOf(nfs::u64(at(app, number + 4)) << 32 | at(app, number)));
        }
    }
    return read;
}

nfs::u32 scan_float(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 read = scanFloatOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, read);
    return args.eax;
}

/* 0x50d0f0 (scan_int): %d, %i, %o, %u, %x and %p: white space skipped, a sign where `sign` says the
 * number has one, `base`'s digits -- 0 for the number's own: 0x for 16, 0 for 8 -- and for a
 * segmented one a ':' and an offset; stored as the flags say into the argument from the va_list at
 * `argp`, a long long for L and I64.  How many characters were read.  At its entry's depth: two
 * pushes, es, a push, then 0x30 bytes.  0 with ts.terminate set when the game stopped. */
u32 scanIntOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 argp, u32 base, u32 sign)
{
    const GuestFrame frame(ts, 0x10, 0x30);
    nfs::u64 value = 0;
    u32 small = 0, skipped = 0, read = 0, minus = '+', c;
    auto next = [&] {
        const ReturnSlot slot(ts);
        c = chktrmOf(app, ts, specs);
        return c != 0xffffffff;
    };
    const bool longLong = (scanFlags(app, specs) & kLongLongVar) != 0;
    auto take = [&](u32 d) {
        if (longLong)
        {
            u32 cross;
            value = u8mOf(value, base, cross) + d;
        }
        else
            small = small * base + d;
    };
    auto digitOf = [&](u32 ch, u32& d) {
        d = radixValueOf(app, ch);
        return nfs::s32(d) < nfs::s32(base);
    };
    u32 d;
    for (;;)
    {
        {
            const ReturnSlot slot(ts);
            c = cgetcOf(app, ts, specs);
        }
        if (ts.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
        ++skipped;
    }
    if (scanFlags(app, specs) & kEndOfInput)
        goto stored;
    {
        const u32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width == 0)
            goto putBack;
    }
    if (sign != 0 && (c == '+' || c == '-'))
    {
        minus = c;
        ++skipped;
        const bool more = next();
        if (ts.terminate)
            return 0;
        if (!more)
            goto stored;
    }
    if (base == 0 || (base == 16 && c == '0'))
    {
        const bool any = base == 0;
        if (c == '0')
        {
            read = 1;
            const bool more = next();
            if (ts.terminate)
                return 0;
            if (!more)
                goto stored;
            if (c == 'x' || c == 'X')
            {
                skipped += 2;
                read = 0;
                const bool after = next();
                if (ts.terminate)
                    return 0;
                if (!after)
                    goto stored;
                if (any)
                    base = 16;
            }
            else if (any)
                base = 8;
        }
        else
            base = 10;
    }
    while (digitOf(c, d))
    {
        take(d);
        ++read;
        const bool more = next();
        if (ts.terminate)
            return 0;
        if (!more)
            goto stored;
    }
    if (c == ':' && (scanFlags(app, specs) & kSegmented))
        for (;;)
        {
            ++read;
            const bool more = next();
            if (ts.terminate)
                return 0;
            if (!more)
                goto stored;
            if (!digitOf(c, d))
                break;
            take(d);
        }
putBack:
    {
        const ReturnSlot slot(ts);
        uncgetOf(app, ts, c, specs);
    }
    if (ts.terminate)
        return 0;
stored:
    if (longLong)
    {
        if (minus == '-')
            value = 0 - value;
        if (nfs::s32(read) > 0)
        {
            read += skipped;
            const u8 flags = scanFlags(app, specs);
            if (flags & kAssign)
            {
                // The far one through its own segment (es as its pointer has it), else through ds.
                const u32 step = at(app, argp) + ((flags & kFarPointer) ? 8 : 4);
                put(app, argp, step);
                // A far pointer's selector taken as the base, as the code's es does; ds the flat model's 0.
                const u32 to = (flags & kFarPointer) ? at(app, step - 8) + half(app, step - 4) : at(app, step - 4);
                put(app, to, u32(value));
                put(app, to + 4, u32(value >> 32));
            }
        }
    }
    else
    {
        if (minus == '-')
            small = 0 - small;
        if (nfs::s32(read) > 0)
        {
            read += skipped;
            const u8 flags = scanFlags(app, specs);
            if (flags & kAssign)
            {
                u32 step, target = argp;
                scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
                const u32 to = target;  // es and ds: the flat model's 0
                if (scanFlags(app, specs) & kShortVar)
                    putHalf(app, to, u16(small));
                else
                    put(app, to, small);
            }
        }
    }
    return read;
}

nfs::u32 scan_int(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 read = scanIntOf(app, ts, args.eax, args.edx, args.ebx, args.ecx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, read);
    return args.eax;
}

/* 0x50c650 (__scnf): the format at `format`, the arguments' va_list at [argp], the input by the
 * specs at `specs`: white space in the format skips the input's, a character of it must be the
 * input's next, and a '%' a conversion.  How many arguments were assigned, or -1 for an input over
 * before any.  At its entry's depth: four pushes, then the format's place and the va_list's copy,
 * which [ and the conversions are handed.  0 with ts.terminate set when the game stopped. */
u32 scnfOf(nfs::App* app, nfs::ThreadState& ts, u32 specs, u32 format, u32 argp)
{
    const GuestFrame frame(ts, 0x10, 8);
    const u32 local = frame - 8;
    const u32 args = local + 4;
    put(app, local, format);
    put(app, args, at(app, argp));
    putByte(app, specs + kScanFlags, scanFlags(app, specs) & u8(~kEndOfInput));
    u32 assigned = 0, read = 0;
    auto ended = [&] { return (scanFlags(app, specs) & kEndOfInput) != 0; };
    // The input's next must be `want`: false for the end of the scan.
    auto literal = [&](u32 want) {
        u32 c;
        {
            const ReturnSlot slot(ts);
            c = cgetcOf(app, ts, specs);
        }
        if (ts.terminate)
            return false;
        if (c == want)
        {
            ++read;
            return true;
        }
        if (!ended())
        {
            const ReturnSlot slot(ts);
            uncgetOf(app, ts, c, specs);
        }
        return false;
    };
    for (;;)
    {
        const u32 p = at(app, local);
        put(app, local, p + 1);
        const u32 f = byte(app, p);
        if (f == 0)
            goto end;
        if (byte(app, kIsTable + ((f + 1) & 0xff)) & kSpace)
        {
            const ReturnSlot slot(ts);
            read += scanWhiteOf(app, ts, specs);
        }
        else if (f != '%')
        {
            if (!literal(f))
                goto end;
        }
        else
        {
            const u32 q = getOptOf(app, at(app, local), specs);
            put(app, local, q);
            const u32 kind = byte(app, q);
            if (kind != 0)
                put(app, local, q + 1);
            u32 got = 0;
            bool converted = true;
            {
                const ReturnSlot slot(ts);
                switch (kind)
                {
                case 'd': got = scanIntOf(app, ts, specs, args, 10, 1); break;
                case 'i': got = scanIntOf(app, ts, specs, args, 0, 1); break;
                case 'o': got = scanIntOf(app, ts, specs, args, 8, 1); break;
                case 'u': got = scanIntOf(app, ts, specs, args, 10, 0); break;
                case 'x': case 'X': case 'p': got = scanIntOf(app, ts, specs, args, 16, 1); break;
                case 'e': case 'f': case 'g': case 'E': case 'G': got = scanFloatOf(app, ts, specs, args); break;
                case 'S':
                    putByte(app, specs + kScanFlags, scanFlags(app, specs) | kLongVar);
                    got = scanStringOf(app, ts, specs, args, specs);
                    break;
                case 's': got = scanStringOf(app, ts, specs, args, specs); break;
                case '[': got = scanArbOf(app, ts, specs, args, local); break;
                case 'C':
                    putByte(app, specs + kScanFlags, scanFlags(app, specs) | kLongVar);
                    got = scanCharOf(app, ts, specs, args, specs);
                    break;
                case 'c': got = scanCharOf(app, ts, specs, args, specs); break;
                case 'n':
                    reportScanOf(app, specs, args, read);
                    converted = false;
                    break;
                case '%':
                    converted = false;
                    if (!literal('%'))
                        goto end;
                    break;
                default:
                    converted = false;
                    break;
                }
            }
            if (ts.terminate)
                return 0;
            if (converted)
            {
                if (nfs::s32(got) <= 0)
                    goto end;
                read += got;
                if (scanFlags(app, specs) & kAssign)
                    ++assigned;
            }
        }
        if (ts.terminate)
            return 0;
        if (ended())
            break;
    }
    // The input over: a %n where the format goes on is still reported.
    if (byte(app, at(app, local)) == '%')
    {
        put(app, local, at(app, local) + 1);
        const u32 q = getOptOf(app, at(app, local), specs);
        put(app, local, q);
        if (byte(app, q) == 'n')
            reportScanOf(app, specs, args, read);
    }
end:
    if (ts.terminate)
        return 0;
    return assigned == 0 && ended() ? 0xffffffff : assigned;
}

nfs::u32 scnf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 assigned;
    {
        const EspKept kept(ts);
        assigned = scnfOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, assigned);
    return args.eax;
}

/* 0x4f0520 (vsscanf): the string at `string` scanned by the format at `format`, the arguments'
 * va_list at [argp] -- __scnf with sscanf's routines in specs of its frame: two pushes, then 0x14
 * bytes. */
u32 vsscanfOf(nfs::App* app, nfs::ThreadState& ts, u32 string, u32 format, u32 argp)
{
    const GuestFrame frame(ts, 8, 0x14);
    const u32 specs = frame - 0x14;
    put(app, specs + kScanString, string);
    put(app, specs + kScanGet, kCgetString);
    put(app, specs + kScanUnget, kUncgetString);
    const ReturnSlot slot(ts);
    return scnfOf(app, ts, specs, format, argp);
}

nfs::u32 vsscanf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 assigned;
    {
        const EspKept kept(ts);
        assigned = vsscanfOf(app, ts, args.eax, args.edx, args.ebx);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, assigned);
    return args.eax;
}

/* 0x4f0548 (sscanf): vsscanf's, its arguments on the stack from `arguments` on; the va_list in its
 * frame: two pushes, then the dword. */
u32 sscanfOf(nfs::App* app, nfs::ThreadState& ts, u32 string, u32 format, u32 arguments)
{
    const GuestFrame frame(ts, 8, 4);
    const u32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(ts);
    return vsscanfOf(app, ts, string, format, list);
}

nfs::u32 sscanf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 assigned;
    {
        const EspKept kept(ts);
        assigned = sscanfOf(app, ts, at(app, ts.sp + 4), at(app, ts.sp + 8), ts.sp + 0xc);
    }
    if (ts.terminate)
        return args.eax;
    returns(ts, args, assigned);
    return args.eax;
}

/* A long double of its parts: the mantissa's low and high dwords, the sign and exponent. */
nfs::Float80 longDouble(u32 low, u32 high, u16 exponent)
{
    return nfs::Float80(nfs::Float80Data{u16(low), u16(low >> 16), u16(high), u16(high >> 16), exponent});
}

/* The long double at `p`, and one put there. */
nfs::Float80 longDoubleAt(nfs::App* app, u32 p)
{
    const nfs::Float80 value = app->getMemory<nfs::Float80>(p);
    return value;
}

void putLongDouble(nfs::App* app, u32 p, const nfs::Float80& value)
{
    app->getMemory<nfs::Float80>(p) = value;
}

u32 lowOf(const nfs::Float80& value)
{
    return value.data.data1 | u32(value.data.data2) << 16;
}

/* 0x51663a (the long double's normalizing): the 96 bits `high`:`middle`:`low`, exponent
 * `exponent`, shifted up until the top one is set, then rounded to 64 by the low dword's top bit.
 * The exponent; 0 for no bits at all. */
u32 normalizeOf(u32& high, u32& middle, u32& low, u32& exponent)
{
    if ((middle | high | low) == 0)
        return 0;
    for (int i = 0; i < 2; ++i)
        if (high == 0)
        {
            high = middle;
            middle = low;
            low = 0;
            exponent -= 0x20;
        }
    while (!(high & 0x80000000))
    {
        --exponent;
        shift96(high, middle, low);
    }
    const u32 round = low >> 31;
    low <<= 1;
    if (round)
    {
        const nfs::u64 m = nfs::u64(middle) + round;
        const nfs::u64 h = nfs::u64(high) + (m >> 32);
        middle = u32(m);
        high = u32(h);
        if (h >> 32)
        {
            high = high >> 1 | 0x80000000;
            ++exponent;
        }
    }
    return exponent;
}

nfs::u32 normalize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    // Its own convention: edx:eax:ebp and edi in, edx:eax and esi out -- the rest as it leaves them.
    args.esi = normalizeOf(args.edx, args.eax, args.ebp, args.edi);
    ts.sp += 4;
    return args.eax;
}

/* 0x5165d8 (__ZBuf2LD): the decimal `digits` as a long double -- an integer, 96 bits of it,
 * normalized. */
nfs::Float80 zbufOf(const std::string& digits)
{
    u32 high = 0, middle = 0, low = 0;
    for (const char c : digits)
    {
        // Times 10: times 4, plus itself, times 2.
        const u32 h = high, m = middle, l = low;
        shift96(high, middle, low);
        shift96(high, middle, low);
        add96(high, middle, low, h, m, l);
        shift96(high, middle, low);
        add96(high, middle, low, 0, 0, u32(c) & 0xf);
    }
    u32 exponent = 0x405e;
    const u32 normal = normalizeOf(high, middle, low, exponent);
    return longDouble(middle, high, u16(normal));
}

/* The string at `p`, up to its 0. */
std::string guestString(nfs::App* app, u32 p)
{
    std::string s;
    for (u8 c; (c = byte(app, p)) != 0; ++p)
        s += char(c);
    return s;
}

nfs::u32 ZBuf2LD(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const nfs::Float80 value = zbufOf(guestString(app, args.eax));
    app->getMemory<nfs::Float80>(args.edx) = value;
    returns(ts, args, lowOf(value));  // eax the mantissa's low dword, as the code leaves it
    return args.eax;
}

namespace
{

/* 0x5166cf: the scaling's way out, its frame's registers back -- the jump 0x5166d8 makes there. */
void scaleReturn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp = args.ebp - 0x10;
    scanEpilogue(app, ts, {&args.edi, &args.esi, &args.ecx, &args.ebx, &args.ebp});
}

}  // namespace

/* 0x51667b (the scaling's powers): `value` times 10 to the `n` -- 0x2000 at most -- by the table's
 * powers of 10^2^n, each product by the x87, under `x87`. */
void scalePowersOf(nfs::App* app, const Arithmetic& x87, nfs::Float80& value, u32 n)
{
    if (nfs::s32(n) >= 0x2000)
        n = 0x2000;
    for (u32 power = kPowersOfTen; nfs::s32(n) > 0; power += 0xa)
    {
        if (n & 1)
        {
            const nfs::Float a = nfs::Float(value);
            const nfs::Float b = nfs::Float(longDoubleAt(app, power));
            const nfs::Float product = x87.mul(a, b);
            value = nfs::Float(product);
        }
        n = u32(nfs::s32(n) >> 1);
    }
}

nfs::u32 scalePowers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float80 value = longDoubleAt(app, args.eax);
    scalePowersOf(app, {ts.fenv.control.word}, value, args.edx);
    putLongDouble(app, args.eax, value);
    returns(ts, args, args.eax);  // eax the long double's address, as the code leaves it
    return args.eax;
}

/* 0x5166cf: 0x51667b's way out, which 0x5166d8 jumps to. */
nfs::u32 scaleEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    scaleReturn(app, ts, args);
    return args.eax;
}

/* 0x5166d8 (the scaling by a power): `value` times 10 to the `n` -- over it for a negative one --
 * the x87 rounding to its 64 bits meanwhile (its control word `control` with the precision set).
 * The control word as it was, which the code leaves in eax; for an `n` of 0 nothing, which leaves
 * eax as it was. */
u32 scaleByOf(nfs::App* app, u16 control, nfs::Float80& value, u32 n)
{
    const Arithmetic x87{u16(control | 0x300)};
    nfs::Float80 power = longDouble(0, 0x80000000, 0x3fff);
    const bool divide = nfs::s32(n) < 0;
    scalePowersOf(app, x87, power, divide ? 0 - n : n);
    {
        const nfs::Float a = nfs::Float(value);
        const nfs::Float b = nfs::Float(power);
        const nfs::Float result = divide ? x87.div(a, b) : x87.mul(a, b);
        value = nfs::Float(result);
    }
    return control;
}

nfs::u32 scaleBy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (args.edx != 0)
    {
        const u32 p = args.eax;
        nfs::Float80 value = longDoubleAt(app, p);
        args.eax = scaleByOf(app, ts.fenv.control.word, value, args.edx);
        putLongDouble(app, p, value);
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x516759 (__LDScale10x): `value` times 10 to the `n`, in steps of 10^4096 at most.  What the
 * last step leaves in eax: the x87's control word, none for a last step of 0. */
std::optional<u32> ldScale10xOf(nfs::App* app, u16 control, nfs::Float80& value, u32 n)
{
    if (nfs::s32(n) > 0x1000)
    {
        scaleByOf(app, control, value, 0x1000);
        n -= 0x1000;
    }
    else if (nfs::s32(n) < -0x1000)
    {
        scaleByOf(app, control, value, 0xfffff000);
        n += 0x1000;
    }
    if (n == 0)
        return std::nullopt;
    return scaleByOf(app, control, value, n);
}

nfs::u32 LDScale10x(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float80 value = longDoubleAt(app, args.eax);
    const std::optional<u32> control = ldScale10xOf(app, ts.fenv.control.word, value, args.edx);
    putLongDouble(app, args.eax, value);
    returns(ts, args, control.value_or(args.eax));
    return args.eax;
}

/* 0x4f8122 (__Strtold): the number at `s` -- white space, a sign, digits with a point among them,
 * an exponent -- as `value`, from its first 19 significant digits; the end of it at [end], unless
 * `end` is 0.  0 for no digits (the long double 0), 3 for a number too big for a double, 2 for one
 * too small, 1 for any other. */
u32 strtoldOf(nfs::App* app, nfs::ThreadState& ts, u32 s, u32 end, nfs::Float80& value)
{
    u32 p = s, stop = s;
    auto isDigit = [](u8 c) { return c >= '0' && c <= '9'; };
    u8 c = byte(app, p);
    for (; c == ' ' || (c >= 9 && c <= 0xd); c = byte(app, ++p))
    {
    }
    const bool negative = c == '-';
    if (c == '+' || c == '-')
        ++p;
    std::string digits;   // the significant ones, 19 at most
    u32 count = 0, fraction = 0;
    bool point = false, any = false, significant = false;
    for (;;)
    {
        c = byte(app, p++);
        if (c == '.')
        {
            if (point)
                break;
            point = true;
            continue;
        }
        if (!isDigit(c))
            break;
        if (point)
            ++fraction;
        significant = significant || c != '0';
        if (significant)
        {
            if (nfs::s32(count) < 0x13)
                digits += char(c);
            ++count;
        }
        any = true;
    }
    u32 exponent = 0;
    if (any)
    {
        if (c == 'e' || c == 'E')
        {
            const u32 e = p - 1;
            const u8 sign = byte(app, p);
            if (sign == '+' || sign == '-')
                ++p;
            bool exponentDigits = false;
            for (; isDigit(c = byte(app, p)); ++p)
            {
                if (nfs::s32(exponent) < 0x3e8)
                    exponent = exponent * 10 + c - 0x30;
                exponentDigits = true;
            }
            if (sign == '-')
                exponent = 0 - exponent;
            if (!exponentDigits)
                p = e;
        }
        else
            --p;
        stop = p;
    }
    if (end != 0)
        put(app, end, stop);
    exponent -= fraction;
    if (nfs::s32(count) > 0x13)
    {
        exponent += count - 0x13;
        count = 0x13;
    }
    while (nfs::s32(count) > 0 && digits[count - 1] == '0')
    {
        ++exponent;
        --count;
    }
    if (count == 0)
    {
        value = longDouble(0, 0, 0);
        return 0;
    }
    digits.resize(count);
    nfs::Float80 number = zbufOf(digits);
    if (exponent != 0)
        ldScale10xOf(app, 0, number, exponent);  // the control word it leaves in eax is not read
    if (negative)
        number.data.data5 |= 0x8000;
    value = number;
    const u32 magnitude = exponent + count - 1;
    if (nfs::s32(magnitude) > 0x134)
        return 3;
    if (nfs::s32(magnitude) < -0x134)
        return 2;
    return 1;
}

nfs::u32 Strtold(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float80 value;
    const u32 result = strtoldOf(app, ts, args.eax, args.ebx, value);
    putLongDouble(app, args.edx, value);
    returns(ts, args, result);
    return args.eax;
}

/* __set_ERANGE's core: errno ERANGE; what it leaves in eax, 0 with ts.terminate set when the game
 * stopped. */
u32 setErangeOf(nfs::App* app, nfs::ThreadState& ts)
{
    const EspKept kept(ts);
    u32 data;
    return setThreadWord(app, ts, kErrno, kERANGE, &data) ? data : 0;
}

/* 0x4f82b6 (strtod): the number at `s` as a double -- HUGE_VAL with its sign, and ERANGE, for one
 * too big; 0 or a denormal, and ERANGE, for one too small; its end at [end], unless `end` is 0.
 * `eax` what the code leaves there: the double's high dword, the long double's exponent, or
 * __set_ERANGE's.  0 with ts.terminate set when the game stopped. */
double strtodOf(nfs::App* app, nfs::ThreadState& ts, u32 s, u32 end, u32& eax)
{
    nfs::Float80 number;
    const u32 kind = strtoldOf(app, ts, s, end, number);
    double d = 0;
    eax = 0;
    if (kind == 0)
        return d;
    auto highOf = [](double v) {
        nfs::u64 bits;
        std::memcpy(&bits, &v, sizeof bits);
        return u32(bits >> 32);
    };
    const u32 e = number.data.data5 & 0x7fff;
    bool range = false;
    if (e >= 0x43ff)
    {
        eax = setErangeOf(app, ts);
        if (ts.terminate)
            return 0;
        if (number.data.data5 & 0x8000)
        {
            const nfs::Float huge = nfs::Float(app->getMemory<double>(kHugeVal));
            const nfs::Float minus = -huge;
            d = double(minus);
        }
        else
        {
            d = app->getMemory<double>(kHugeVal);
            eax = at(app, kHugeVal + 4);
        }
    }
    else
    {
        const nfs::Float value = nfs::Float(number);
        d = double(value);
        eax = highOf(d);
        if (e >= 0x3bcd && kind == 3)
        {
            eax = e;
            range = true;
        }
        else
            range = !((highOf(d) >> 16) & 0x7ff0);
    }
    if (range)
    {
        eax = setErangeOf(app, ts);
        if (ts.terminate)
            return 0;
    }
    return d;
}

nfs::u32 strtod(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 eax;
    const double d = strtodOf(app, ts, args.eax, args.edx, eax);
    if (ts.terminate)
        return args.eax;
    const nfs::Float result = nfs::Float(d);
    args.floatSet = true;
    args.floatResult = result;
    returns(ts, args, eax);
    return args.eax;
}


/* ---- The console: getch, getche, kbhit and putch, on the console's input
 * and output Windows hands the runtime by CreateFileA -- or the default
 * windowing's hooks, where there are any -- under the file's lock.  Typed
 * cores and their thunks, the INPUT_RECORDs and counts in the frames where
 * the code had them, which it hands Windows; the calls to Windows through the
 * game's imports. ---- */
namespace
{

constexpr u32 kConsoleIn = 0x568a38, kConsoleOut = 0x568a3c;  // conin$'s and conout$'s HANDLEs, -1 for none yet
constexpr u32 kConinName = 0x55056c, kConoutName = 0x550574;  // "conin$", "conout$"
constexpr u32 kUngotKey = 0x567828;    // ungetch's character, 0 for none
constexpr u32 kKeyState = 0x567168;    // do_getch's: 1 repeating a character, 2 a key's second half
constexpr u32 kKeyScan = 0x9f38b0;     // the extended key's scan code, its second half
constexpr u32 kKeyChar = 0x9f38b4, kKeyRepeats = 0x9f38b8;  // the character repeating, how many more times
constexpr u32 kWindowsKbhit = 0x5677e0, kWindowsGetch = 0x5677e4, kWindowsPutch = 0x5677ec;
/* The console's handle opened, if it is not yet, at `handle`: the file `name` opened for `access`
 * -- the port has no console, so a file of that name, if any, in the data's root -- or -1. */
bool openConsole(nfs::App* app, nfs::ThreadState& ts, u32 handle, u32 name, os::files::Access access)
{
    NFS2_USE(ts);
    if (at(app, handle) != 0xffffffff)
        return true;
    const auto opened = os::files::open(app, guest::text(app, name), access, os::files::Create::openExisting);
    put(app, handle, opened.value_or(0xffffffff));
    return true;
}

}  // namespace

/* 0x515690 (__NTRealKey): whether the INPUT_RECORD at `record` is a key pressed, not shift,
 * control or alt alone. */
bool realKeyOf(nfs::App* app, u32 record)
{
    if (half(app, record) != 1 || at(app, record + 4) == 0)
        return false;
    const u16 key = half(app, record + 0xa);
    return key < 0x10 || key > 0x12;
}

nfs::u32 NTRealKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    returns(ts, args, realKeyOf(app, args.eax) ? 1 : 0);
    return args.eax;
}

/* The file's lock taken (kAccessFile) or given back (kReleaseFile), by its hook (watcom_hooks.h),
 * with `eax`, every other register kept: what it leaves in eax, 0 with ts.terminate set when the
 * game stopped. */
u32 lockHook(nfs::App* app, nfs::ThreadState& ts, u32 slot, u32 eax)
{
    const EspKept kept(ts);
    return (slot == kAccessFile ? hook<kAccessFile>(app, ts, {eax}) : hook<kReleaseFile>(app, ts, {eax}))
        .value_or(0);
}

/* The default windowing's hook at [slot] for the windowed handle `handle`: __IsWindowedHandle's
 * say, then the hook with it in eax and `edx` -- putch's character; kbhit's and getch's take eax
 * alone, and are handed 0. */
u32 windowedHook(nfs::App* app, nfs::ThreadState& ts, u32 slot, u32 handle, u32 edx)
{
    const EspKept kept(ts);
    const auto window = through(app, ts, at(app, kIsWindowedHandle), {handle, edx});
    if (!window)
        return 0;
    return through(app, ts, at(app, slot), {*window, edx}).value_or(0);
}

/* 0x5156b8 (the console's handles): conin$ and conout$ opened, the ones not yet, under the file's
 * lock.  What the release leaves in eax. */
u32 openConsolesOf(nfs::App* app, nfs::ThreadState& ts)
{
    const GuestFrame frame(ts, 8, 0);
    lockHook(app, ts, kAccessFile, 0);
    if (ts.terminate)
        return 0;
    {
        const EspKept kept(ts);
        if (!openConsole(app, ts, kConsoleIn, kConinName, os::files::Access::read)
            || !openConsole(app, ts, kConsoleOut, kConoutName, os::files::Access::write))
            return 0;
    }
    return lockHook(app, ts, kReleaseFile, 0);
}

nfs::u32 openConsoles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = openConsolesOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x515728 (__NTConsoleInput): the console's input HANDLE. */
u32 consoleInputOf(nfs::App* app, nfs::ThreadState& ts)
{
    const ReturnSlot slot(ts);
    openConsolesOf(app, ts);
    return at(app, kConsoleIn);
}

nfs::u32 NTConsoleInput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 handle = consoleInputOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, handle);
    return args.eax;
}

/* 0x515734 (__NTConsoleOutput): the console's output HANDLE. */
u32 consoleOutputOf(nfs::App* app, nfs::ThreadState& ts)
{
    const ReturnSlot slot(ts);
    openConsolesOf(app, ts);
    return at(app, kConsoleOut);
}

nfs::u32 NTConsoleOutput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 handle = consoleOutputOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, handle);
    return args.eax;
}

/* 0x52b290 (putch): the character `c` to the console -- by the default windowing's putch, where
 * there is one, else under the file's lock to the console, which the port does not have: nowhere;
 * `c` back.  At its entry's depth: three pushes, then 8 bytes.  0 with ts.terminate set when the
 * game stopped. */
u32 putchOf(nfs::App* app, nfs::ThreadState& ts, u32 c)
{
    const GuestFrame frame(ts, 0xc, 8);
    if (at(app, kWindowsPutch) != 0)
        windowedHook(app, ts, kWindowsPutch, 1, c);
    else
    {
        lockHook(app, ts, kAccessFile, 1);
        if (ts.terminate)
            return 0;
        consoleOutputOf(app, ts);
        if (ts.terminate)
            return 0;
        lockHook(app, ts, kReleaseFile, 1);
    }
    return ts.terminate ? 0 : c;
}

nfs::u32 putch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 c = putchOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, c);
    return args.eax;
}

/* 0x4f7ce0 (kbhit): whether a key is waiting -- the console's input looked at, what is not a key
 * taken off it -- or the default windowing's say.  The port has no console: no key ever waits on
 * it.  At its entry's depth: three pushes, then 0x18 bytes.  0 with ts.terminate set when the game
 * stopped. */
u32 kbhitOf(nfs::App* app, nfs::ThreadState& ts)
{
    const GuestFrame frame(ts, 0xc, 0x18);
    if (at(app, kWindowsKbhit) != 0)
        return windowedHook(app, ts, kWindowsKbhit, 0, 0);
    lockHook(app, ts, kAccessFile, 0);
    if (ts.terminate)
        return 0;
    consoleInputOf(app, ts);
    if (ts.terminate)
        return 0;
    // Where the game's code lets the other threads run, as kbhit waits on the console
    // (THREAD_SEGMENTS), and its loop's safepoint: its loop goes round once.
    app->unlockContext(ts);
    nfs::Thread::sleep(0);
    app->lockContext(ts);
    if (app->contextWanted())
        app->yieldContext(ts);
    lockHook(app, ts, kReleaseFile, 0);
    return 0;
}

nfs::u32 kbhit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 waiting = kbhitOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, waiting);
    return args.eax;
}

/* 0x4f7e20 (do_getch): the next character from the console `handle`, as getch gives it: a
 * character repeated as many times as its key says, an extended key as 0 and its scan code; -1 for
 * none -- the port has no console, so no key comes from it.  At its entry's depth: six pushes,
 * then 0x18 bytes.  0 with ts.terminate set when the game stopped. */
u32 doGetchOf(nfs::App* app, nfs::ThreadState& ts, u32 handle)
{
    NFS2_USE(ts);
    NFS2_USE(handle);
    u32 repeats = at(app, kKeyRepeats), c = at(app, kKeyChar), key;
    const u32 state = at(app, kKeyState);
    if (state == 1)
    {
        --repeats;
        if (c == 0)
            put(app, kKeyState, 2);
        else if (repeats == 0)
            put(app, kKeyState, 0);
        key = c;
    }
    else if (state == 2)
    {
        put(app, kKeyState, repeats != 0 ? 1 : 0);
        key = at(app, kKeyScan);
    }
    else
    {
        put(app, kKeyRepeats, repeats);
        put(app, kKeyChar, c);
        key = 0xffffffff;
    }
    put(app, kKeyChar, c);
    put(app, kKeyRepeats, repeats);
    return key;
}

nfs::u32 do_getch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 key = doGetchOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, key);
    return args.eax;
}

/* 0x4f7f24 (getch): the next character, not echoed: ungetch's, the default windowing's, or the
 * console's -- whose mode the port has none of to set.  At its entry's depth: four pushes, then
 * the console's mode.  0 with ts.terminate set when the game stopped. */
u32 getchOf(nfs::App* app, nfs::ThreadState& ts)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 ungot = at(app, kUngotKey);
    if (ungot != 0)
    {
        put(app, kUngotKey, 0);
        return ungot;
    }
    if (at(app, kWindowsGetch) != 0)
        return windowedHook(app, ts, kWindowsGetch, 0, 0);
    lockHook(app, ts, kAccessFile, 0);
    if (ts.terminate)
        return 0;
    const u32 handle = consoleInputOf(app, ts);
    if (ts.terminate)
        return 0;
    u32 key;
    {
        const ReturnSlot slot(ts);
        key = doGetchOf(app, ts, handle);
    }
    if (ts.terminate)
        return 0;
    lockHook(app, ts, kReleaseFile, 0);
    return ts.terminate ? 0 : key;
}

nfs::u32 getch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 key = getchOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, key);
    return args.eax;
}

/* 0x522e60 (getche): getch's character, echoed by putch -- unless it was ungetch's.  At its
 * entry's depth: two pushes.  0 with ts.terminate set when the game stopped. */
u32 getcheOf(nfs::App* app, nfs::ThreadState& ts)
{
    const GuestFrame frame(ts, 8, 0);
    const u32 ungot = at(app, kUngotKey);
    u32 key;
    {
        const ReturnSlot slot(ts);
        key = getchOf(app, ts);
    }
    if (ts.terminate)
        return 0;
    if (key != 0xffffffff && ungot == 0)
    {
        const ReturnSlot slot(ts);
        putchOf(app, ts, key);
    }
    return ts.terminate ? 0 : key;
}

nfs::u32 getche(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 key = getcheOf(app, ts);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, key);
    return args.eax;
}


/* ---- Structured exception handling (clib/startup/a/seh386.asm): __try's
 * registration on fs:[0]'s chain, and __TryHandler, which Windows hands an
 * exception and which runs the scopes' filters, unwinding to the one that
 * takes it.  A filter is jumped to with ebp the function's frame and the
 * handler's registers pushed, and calls __Except with its say in eax; the
 * scopes' finally blocks are called.  As the game's code has these: a jmp
 * to another function's code is a call that returns for this one too, and
 * __Except's return into the handler is the return to its caller. ---- */
namespace
{

// A registration on fs:[0]: the next one, the handler, the frame, the
// scopes' table, the level __try is at, whether unwinding, the level
// unwound to, the exception's record and context.
constexpr u32 kTryHandler = 0x4f7b2a;
constexpr u32 kRtlUnwind = 0x5345c0;
constexpr u32 kUnwindReturn = 0x4f7bbc;  // where RtlUnwind is told to come back to

}  // namespace

/* 0x4f7af8 (__TryInit): the registration at `registration` put first on fs:[0]'s chain, at no level
 * yet, for the function whose frame is `frame` (its ebp).  The old first goes by the stack, pushed
 * and popped. */
void tryInitOf(nfs::App* app, nfs::ThreadState& ts, u32 registration, u32 frame)
{
    const u32 previous = at(app, ts.threadBlock);
    put(app, ts.sp - 4, previous);
    put(app, registration, previous);
    put(app, registration + 4, kTryHandler);
    put(app, registration + 8, frame);
    putByte(app, registration + 0x10, 0xff);
    putByte(app, registration + 0x11, 0);
    put(app, ts.threadBlock, registration);
}

nfs::u32 TryInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    tryInitOf(app, ts, args.eax, args.ebp);
    ts.sp += 4;  // eax left as it was, the registration
    return args.eax;
}

/* 0x4f7b8c (__TryHandler's end): ExceptionContinueSearch, 1, the handler's
 * registers back. */
nfs::u32 TryDone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = 1;
    for (u32* r : {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp})
        *r = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* A call through a pointer: the callee's thunk run on `args` with esp as it is -- the callee finds its
 * return address's slot where the code's pushes left esp, and leaves esp as its ret does.  Its eax. */
nfs::u32 plainCall(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target, nfs::Args& args)
{
    const nfs::ThunkPtr thunk = app->thunkAt(target);
    if (thunk == nullptr)
    {
        ts.terminate = true;  // no thunk registered at the target: the game cannot go on
        return args.eax;
    }
    return thunk(app, ts, args);
}

nfs::u32 TryOuter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

/* 0x4f7b61 (__TryHandler's scopes): from the level in ebx out, the first
 * scope with a filter jumped to -- one without, a finally block, passed by. */
nfs::u32 TryScope(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (u8(args.ebx) == 0xff)
        return TryDone(app, ts, args);
    putByte(app, args.ebp + 0x12, u8(args.ebx));
    args.ebx *= 3;
    if (byte(app, args.edi + args.ebx * 2 + 1) != 0)
        return TryOuter(app, ts, args);
    push(app, ts, args.ebp);
    push(app, ts, args.edi);
    push(app, ts, args.ebx);
    args.ebp = at(app, args.ebp + 8);
    args.eax = plainCall(app, ts, at(app, args.edi + args.ebx * 2 + 2), args);
    return args.eax;
}

/* 0x4f7b87 (__TryHandler's next scope): the level the scope at ebx is in. */
nfs::u32 TryOuter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    setLow8(args.ebx, byte(app, args.edi + args.ebx * 2));
    TryScope(app, ts, args);
    return args.eax;
}

/* 0x4f7bc3 (__local_unwind): the finally blocks from the registration's level, at `registration`,
 * out to the level unwound to, each called with ebp its function's frame -- and edi the scopes'
 * table and ebx the scope's index, which it pushes under its return.  The level is a byte of ebx
 * as the code has it: the index made of it times 3, the next level its low byte.  At its entry's
 * depth: six pushes.  False when the game stopped. */
bool localUnwindOf(nfs::App* app, nfs::ThreadState& ts, u32 registration, const nfs::Args& caller)
{
    // The six pushes written, as structured exception handling is still register by register here:
    // __Except, which calls these, takes its words off first, so their frames lie where the caller's
    // were.
    const GuestFrame frame(app, ts, {caller.ebp, caller.edi, caller.esi, caller.edx, caller.ecx, caller.ebx}, 0);
    nfs::Args regs = caller;  // the registers the finally blocks are called with: eax as each leaves it
    const u32 table = at(app, registration + 0xc);
    u32 level = byte(app, registration + 0x10);
    putByte(app, registration + 0x11, 1);
    while (u8(level) != 0xff && u8(level) != byte(app, registration + 0x12))
    {
        level *= 3;
        if (byte(app, table + level * 2 + 1) != 0)
        {
            const GuestFrame saved(app, ts, {registration, table, level}, 0);
            const EspKept kept(ts);
            nfs::Args a = regs;
            a.ebp = at(app, registration + 8);
            a.edi = table;
            a.ebx = level;
            const std::optional<u32> eax = guest::pointerCall(app, ts, at(app, table + level * 2 + 2), a);
            if (!eax)
                return false;
            regs.eax = *eax;
        }
        level = (level & 0xffffff00) | byte(app, table + level * 2);
        putByte(app, registration + 0x10, u8(level));
    }
    putByte(app, registration + 0x11, 0);
    return true;
}

nfs::u32 local_unwind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!localUnwindOf(app, ts, args.ebp, args))
        return args.eax;
    ts.sp += 4;  // eax as the finally blocks leave it
    return args.eax;
}

/* 0x533de4: RtlUnwind's import, jumped to.  The port unwinds nothing (it says so in the log): its
 * four words and the return address popped, stdcall, eax as it was. */
nfs::u32 RtlUnwindThunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    os::debugBreak();
    ts.sp += 4 + 4 * 4;
    return args.eax;
}

/* 0x4f7ba7 (__global_unwind): the registrations above the one at `registration` unwound, by
 * RtlUnwind -- its four words pushed, stdcall, to come back to kUnwindReturn.  At its entry's
 * depth: six pushes.  False when the game stopped. */
bool globalUnwindOf(nfs::App* app, nfs::ThreadState& ts, u32 registration, const nfs::Args& caller)
{
    // The six pushes written, as localUnwindOf's.
    const GuestFrame frame(app, ts, {caller.ebp, caller.edi, caller.esi, caller.edx, caller.ecx, caller.ebx}, 0);
    const EspKept kept(ts);
    for (const u32 word : {u32(0), u32(0), kUnwindReturn, registration})
        push(app, ts, word);
    nfs::Args registers = caller;
    return direct(app, ts, RtlUnwindThunk, registers);
}

nfs::u32 global_unwind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!globalUnwindOf(app, ts, args.ebp, args))
        return args.eax;
    ts.sp += 4;  // eax as RtlUnwind leaves it
    return args.eax;
}

/* 0x4f7b2a (__TryHandler): the exception at [esp+4], for the registration at
 * [esp+8]: when unwinding, its finally blocks; else, its record and context
 * kept, its scopes' filters from its level out. */
nfs::u32 TryHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.ebp);
    args.ebp = ts.sp;
    for (const u32 r : {args.edi, args.esi, args.edx, args.ecx, args.ebx})
        push(app, ts, r);
    args.edi = at(app, args.ebp + 0xc);
    args.eax = at(app, args.ebp + 8);
    if (at(app, args.eax + 4) & 6)
    {
        args.ebp = args.edi;
        putByte(app, args.ebp + 0x12, 0xff);
        if (!direct(app, ts, local_unwind, args))
            return args.eax;
        return TryDone(app, ts, args);
    }
    put(app, args.edi + 0x14, args.eax);
    args.eax = at(app, args.ebp + 0x10);
    put(app, args.edi + 0x18, args.eax);
    args.ebp = args.edi;
    args.edi = at(app, args.ebp + 0xc);
    args.ebx = byte(app, args.ebp + 0x10);
    TryScope(app, ts, args);
    return args.eax;
}

/* 0x4f7b7d (__Except): a filter's say, in eax, on the scope its pushes
 * name: -1 the exception's code goes on (0), 0 the next scope out, else
 * the scopes unwound and the handler run -- here, back to the filter's
 * caller. */
nfs::u32 Except(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.edx = pop(app, ts);
    args.ebx = pop(app, ts);
    args.edi = pop(app, ts);
    args.ebp = pop(app, ts);
    ++args.eax;
    if (args.eax == 0)
    {
        for (u32* r : {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp})
            *r = pop(app, ts);
        ts.sp += 4;
        return args.eax;
    }
    --args.eax;
    if (args.eax == 0)
        return TryOuter(app, ts, args);
    push(app, ts, args.edx);
    if (!direct(app, ts, global_unwind, args) || !direct(app, ts, local_unwind, args))
        return args.eax;
    args.ebp = at(app, args.ebp + 8);
    ts.sp += 4;
    return args.eax;
}


/* ---- The start (clib/startup): _cstart_ and what it runs before the game's
 * main -- the initializers, the thread's data and its stack's limits, the
 * standard handles, the environment's and the command line's copies, the
 * exception handler -- then the game's main, by its address, and exit.  Each
 * is a typed core and its thunk, the frames where Windows is handed them;
 * the calls to Windows through the game's imports, with eax as the code has
 * it when it calls.  The main thread's start and the call of main stay as
 * the code is, each push to the guest's stack: they make room on it for the
 * thread's data and the stack's growth. ---- */
namespace
{

constexpr u32 kInitStart = 0x56b7de, kInitEnd = 0x56b81a;  // the runtime's initializers: 6 bytes each
constexpr u32 kThreadDataSize = 0x56af70;  // __ThreadDataSize
constexpr u32 kStackSize = 0x567820;       // the main thread's stack's size
constexpr u32 kStackLimit = 0x567824;      // _STACKLOW's: where its stack ends
constexpr u32 kStackTop = 0x56781c;        // _STACKTOP
constexpr u32 kIsDll = 0x9f6420;           // whether the runtime is a DLL's (the way out's kExitStarted)
constexpr u32 kMainThreadData = 0x9f642c;
constexpr u32 kEnvironmentStrings = 0x567839;
constexpr u32 kOsMajor = 0x56783f, kOsMinor = 0x567840, kOsBuild = 0x567841;  // GetVersion's parts
constexpr u32 kOsVersion = 0x567843, kWinMajor = 0x567847, kWinMinor = 0x56784b, kWinVersion = 0x56784f;
constexpr u32 kModuleName = 0x9f6430, kWideModuleName = 0x9f6534;  // the program's, 0x104 bytes and wide
constexpr u32 kDllName = 0x9f673c, kWideDllName = 0x9f6840;
constexpr u32 kProgramName = 0x567800, kWideProgramName = 0x56780c;
constexpr u32 kDllNamePtr = 0x567804, kWideDllNamePtr = 0x567810;
constexpr u32 kCommandLine = 0x9f6424, kWideCommandLine = 0x9f6428;  // their copies
constexpr u32 kArguments = 0x5677fc, kWideArguments = 0x567808;      // past the program's name in them
constexpr u32 kNoWideCommandLine = 0x54e0c0;  // L"" for a Windows without GetCommandLineW
constexpr u32 kAccessIobHook = 0x5677a4;      // a hook the start calls between the initializers
constexpr u32 kExceptionHandlerCode = 0x51ef08;
constexpr u32 kArgc = 0xa18210, kArgv = 0xa18214;
constexpr u32 kGameMain = 0x4a39f0;  // the game's main, still its code
constexpr u32 kExitEntry = 0x4dfd04;
/* The version the runtime records of the system: 4.3, a build past 0x8000 -- Windows 95's way of
 * giving it, as the port always has. */
constexpr u32 kSystemVersion = 0x80000304;

}  // namespace

/* 0x52b640: a start-up hook that does nothing. */
nfs::u32 startHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    ts.sp += 4;
    return args.eax;
}

/* _nmalloc's and _nfree's cores as a call of them runs them, every register kept. */
u32 nmallocOf(nfs::App* app, nfs::ThreadState& ts, u32 size)
{
    const ReturnSlot slot(ts);
    const EspKept kept(ts);
    u32 block;
    return watcomheap::allocate(app, ts, kGameHeap, size, block) ? block : 0;
}

void nfreeOf(nfs::App* app, nfs::ThreadState& ts, u32 block)
{
    const ReturnSlot slot(ts);
    const EspKept kept(ts);
    watcomheap::free(app, ts, kGameHeap, block);
}

/* Whether _IsTable has the character `c` white space. */
bool isSpace(nfs::App* app, u32 c)
{
    return (byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace) != 0;
}

/* 0x4ff41c (__InitRtns): the runtime's initializers of priority up to `limit` not yet run, the
 * lowest first, each marked run.  At its entry's depth: ebx, ecx, edx, esi and es pushed.  What the
 * code leaves in eax: the table's end. */
u32 initRtnsOf(nfs::App* app, nfs::ThreadState& ts, u8 limit)
{
    const GuestFrame frame(ts, 0x14, 0);  // the pushes: their room only
    for (;;)
    {
        u32 first = kInitEnd;
        u8 lowest = limit;
        for (u32 entry = kInitStart; entry < kInitEnd; entry += 6)
            if (byte(app, entry) != 2 && lowest >= byte(app, entry + 1))
            {
                first = entry;
                lowest = byte(app, entry + 1);
            }
        if (first == kInitEnd)
            return kInitEnd;
        {
            const ReturnSlot slot(ts);
            if (!callRoutineOf(app, ts, first + 2))
                return 0;
        }
        putByte(app, first, 2);
    }
}

nfs::u32 InitRtns(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = initRtnsOf(app, ts, u8(args.eax));
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x51ec20 (the stack's limits): where the thread's stack ends, into [low], and where it starts,
 * into [high] -- from the block of memory the frame is in, the guard pages Windows kept below as
 * its version says -- either for none.  At its entry's depth: three pushes, then 0x20 bytes, the
 * place asked about at the top.  The end, which the code leaves in eax; 0 with ts.terminate set
 * when the game stopped. */
u32 stackLimitsOf(nfs::App* app, nfs::ThreadState& ts, u32 low, u32 high)
{
    const GuestFrame frame(ts, 0xc, 0x20);
    const os::memory::Region region = os::memory::regionOf(frame - 4);
    const u32 start = region.base + region.size;
    u32 end = region.base;
    if (half(app, kOsBuild) < 0x8000)
        end += 0x3000;
    else if (byte(app, kOsMajor) < 4)
        end += 0x12000;
    else
        end += 0x13000;
    if (low != 0)
        put(app, low, end);
    if (high != 0)
        put(app, high, start);
    return end;
}

nfs::u32 stackLimits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 end = stackLimitsOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, end);
    return args.eax;
}

/* 0x525df0 (calloc): `n` things of `size` bytes, zeroed.  At its entry's depth: ebx pushed. */
u32 callocOf(nfs::App* app, nfs::ThreadState& ts, u32 n, u32 size)
{
    const GuestFrame frame(ts, 0x4, 0);
    const u32 bytes = n * size;
    const u32 block = nmallocOf(app, ts, bytes);
    if (ts.terminate)
        return 0;
    if (block != 0)
        clib::memset(app, block, 0, bytes);
    return block;
}

nfs::u32 calloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 block = callocOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, block);
    return args.eax;
}

/* 0x526030 (the thread's data's start): the data at `data` -- if any -- told its stack's end and
 * its thread.  At its entry's depth: three pushes.  What the code leaves in eax: the thread's id,
 * or 0; 0 with ts.terminate set when the game stopped. */
u32 initThreadOf(nfs::App* app, nfs::ThreadState& ts, u32 data)
{
    const GuestFrame frame(ts, 0xc, 0);
    if (data == 0)
        return 0;
    put(app, data + 0xc, 1);
    u32 end;
    {
        const ReturnSlot slot(ts);
        end = stackLimitsOf(app, ts, data, 0);
    }
    if (ts.terminate)
        return 0;
    const EspKept kept(ts);
    NFS2_USE(end);
    const u32 thread = os::threads::currentId(app, ts);
    put(app, data + 0xda, thread);
    return thread;
}

nfs::u32 initThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = initThreadOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x51e7ec (the main thread's data): the data at `data` -- calloc's, for none -- started; the data,
 * or 0.  At its entry's depth: ebx and edx pushed. */
u32 initThreadDataOf(nfs::App* app, nfs::ThreadState& ts, u32 data)
{
    const GuestFrame frame(ts, 0x8, 0);
    if (data == 0)
    {
        {
            const ReturnSlot slot(ts);
            data = callocOf(app, ts, 1, at(app, kThreadDataSize));
        }
        if (ts.terminate)
            return 0;
        if (data != 0)
        {
            putByte(app, data + 0x52, 1);
            put(app, data + 0xf0, at(app, kThreadDataSize));
        }
    }
    {
        const ReturnSlot slot(ts);
        initThreadOf(app, ts, data);
    }
    return ts.terminate ? 0 : data;
}

nfs::u32 initThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 data = initThreadDataOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, data);
    return args.eax;
}

/* 0x51e490 (__initPOSIXHandles): the port's standard input, output and error -- a fake handle for
 * one it has none of -- made handles 0, 1 and 2.  At its entry's depth: ecx and edx pushed.  The
 * last handle, which the code leaves in eax; 0 with ts.terminate set when the game stopped. */
u32 initPOSIXHandlesOf(nfs::App* app, nfs::ThreadState& ts, u32 eax)
{
    const GuestFrame frame(ts, 0x8, 0);
    u32 handle = eax;  // eax as it came: the last handle, as the code has it at each call
    for (const int stream : {0, 1, 2})
    {
        u32 os = os::standardStream(app, stream);
        const ReturnSlot slot(ts);
        const EspKept kept(ts);
        if ((os == 0 || os == 0xffffffff) && !fakeHandle(app, ts, os))
            return 0;
        if (!allocHandle(app, ts, os, handle))
            return 0;
    }
    return handle;
}

nfs::u32 initPOSIXHandles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 handle = initPOSIXHandlesOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, handle);
    return args.eax;
}

/* The name of the module `module` into `out`, as the port gives it: for the program (0) its path,
 * C:\ and its name, and how long it is; for another module C:\ alone, no ending, and 0. */
u32 moduleNameOf(nfs::App* app, u32 module, u32 out)
{
    if (module != 0)
    {
        for (u32 i = 0; i < 3; ++i)
            putByte(app, out + i, u8("C:\\"[i]));
        return 0;
    }
    const std::string path = os::programPath(app);
    guest::putText(app, out, path.c_str());
    return u32(path.size() - 3);  // the name's length, C:\ not counted
}

/* 0x51eb00 (the module's wide name): the name of the module `module` into the `size` wide
 * characters at `out` -- the narrow name made wide, the version the runtime records (ntInitOf's)
 * having no wide module names; how long, or 0.  At its entry's depth: four pushes.  0 with
 * ts.terminate set when the game stopped. */
u32 moduleWideNameOf(nfs::App* app, nfs::ThreadState& ts, u32 module, u32 out, u32 size)
{
    const GuestFrame frame(ts, 0x10, 0);
    const u32 narrow = nmallocOf(app, ts, 0x208);
    if (ts.terminate || narrow == 0)
        return 0;
    if (moduleNameOf(app, module, narrow) == 0)
    {
        nfreeOf(app, ts, narrow);
        return 0;
    }
    // The whole name asked for (-1 bytes), as the code asks: nothing taken, the mark alone written.
    const u32 wide = u32(os::text::widen(guest::text(app, narrow), -1, reinterpret_cast<nfs::u16*>(guest::memory(app, out))));
    nfreeOf(app, ts, narrow);
    if (ts.terminate || wide == 0)
        return 0;
    putHalf(app, out + size * 2 - 2, 0);
    return wcslenOf(app, out);
}

nfs::u32 moduleWideName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 n = moduleWideNameOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, n);
    return args.eax;
}

/* 0x51eba0 (the command line's copy): the string at `s` copied into a block of _nmalloc's; 0 for
 * none.  At its entry's depth: four pushes. */
u32 copyCommandLineOf(nfs::App* app, nfs::ThreadState& ts, u32 s)
{
    const GuestFrame frame(ts, 0x10, 0);
    u32 n = 0;
    while (byte(app, s + n) != 0)
        ++n;
    ++n;
    const u32 copy = nmallocOf(app, ts, n);
    if (ts.terminate)
        return 0;
    if (copy != 0)
        memcpyOf(app, copy, s, n);
    return copy;
}

nfs::u32 copyCommandLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 copy = copyCommandLineOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, copy);
    return args.eax;
}

/* 0x51ebf0 (the wide command line's copy): the wide string at `s` copied into a block of
 * _nmalloc's; 0 for none.  At its entry's depth: three pushes. */
u32 copyWideCommandLineOf(nfs::App* app, nfs::ThreadState& ts, u32 s)
{
    const GuestFrame frame(ts, 0xc, 0);
    const u32 bytes = (wcslenOf(app, s) + 1) * 2;
    const u32 copy = nmallocOf(app, ts, bytes);
    if (ts.terminate)
        return 0;
    if (copy != 0)
        memcpyOf(app, copy, s, bytes);
    return copy;
}

nfs::u32 copyWideCommandLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 copy = copyWideCommandLineOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, copy);
    return args.eax;
}

/* The thread's data, by __GetThreadPtr's pointer, every register kept (what the code has in edx at
 * the call, which __GetThreadPtr does not read, is not handed); 0 with ts.terminate set when the
 * game stopped. */
u32 threadPtrOf(nfs::App* app, nfs::ThreadState& ts, u32)
{
    const EspKept kept(ts);
    return hook<kThreadPtr>(app, ts).value_or(0);
}

/* 0x51f0d0 (__InitExceptionHandling): the registration at `registration` the thread's, first on
 * fs:[0]'s chain with the runtime's handler, and the runtime's filter Windows' for an exception
 * nobody handles.  At its entry's depth: ecx and edx pushed.  What SetUnhandledExceptionFilter
 * leaves in eax; 0 with ts.terminate set when the game stopped. */
u32 initExceptionHandlingOf(nfs::App* app, nfs::ThreadState& ts, u32 registration)
{
    const GuestFrame frame(ts, 0x8, 0);
    u32 data = threadPtrOf(app, ts, registration);
    if (ts.terminate)
        return 0;
    put(app, data + 0x54, registration);
    const u32 previous = at(app, ts.threadBlock);
    data = threadPtrOf(app, ts, previous);
    if (ts.terminate)
        return 0;
    put(app, at(app, data + 0x54), previous);
    data = threadPtrOf(app, ts, previous);
    if (ts.terminate)
        return 0;
    put(app, at(app, data + 0x54) + 4, kExceptionHandlerCode);
    data = threadPtrOf(app, ts, 0);
    if (ts.terminate)
        return 0;
    put(app, ts.threadBlock, at(app, data + 0x54));
    // The runtime's filter for what is not handled: the port keeps no filter, so the one before it
    // is none, 0.
    return 0;
}

nfs::u32 InitExceptionHandling(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = initExceptionHandlingOf(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* Past the program's name in the command line at `p` -- quoted or not -- and the white space after
 * it: `step` bytes a character, `c` its character. */
template <typename Char>
u32 argumentsOf(nfs::App* app, u32 p, u32 step, Char c)
{
    if (c(p) == '"')
    {
        const u32 second = c(p + step);
        p += step;
        if (second != '"')
            while (c(p) != 0)
            {
                const u32 next = c(p + step);
                p += step;
                if (next == '"')
                    break;
            }
        if (c(p) != 0)
            p += step;
    }
    else
        while (!isSpace(app, byte(app, p)) && c(p) != 0)
            p += step;
    while (isSpace(app, byte(app, p)))
        p += step;
    return p;
}

/* 0x4ff108 (__NTInit): the runtime's state for a program (`dll` 0) or a DLL, the thread's data at
 * `data`, the module `module`: the data started, the standard handles, the environment, the
 * system's version, the names, the command lines and where their arguments start.  0 for no thread's data
 * in a DLL, else 1; a program without one leaves by ExitProcess.  At its entry's depth: three
 * pushes.  0 with ts.terminate set when the game stopped. */
u32 ntInitOf(nfs::App* app, nfs::ThreadState& ts, u32 dll, u32 data, u32 module)
{
    const GuestFrame frame(ts, 0xc, 0);
    // A core of the runtime's at this one's depth.
    auto calls = [&](auto&& run) {
        const ReturnSlot slot(ts);
        return run();
    };
    put(app, kIsDll, dll);
    data = calls([&] { return initThreadDataOf(app, ts, data); });
    if (ts.terminate)
        return 0;
    put(app, kMainThreadData, data);
    if (data == 0)
    {
        if (dll != 0)
            return 0;
        os::exitProcess(app, ts, 1);
        if (ts.terminate)
            return 0;
    }
    calls([&] { return initPOSIXHandlesOf(app, ts, data); });  // eax the data, as stored
    if (ts.terminate)
        return 0;
    put(app, kEnvironmentStrings, os::environment(app));
    put(app, kEnviron, 0);
    const u32 version = kSystemVersion;
    // Its parts: the major and minor versions, the build -- and the version in Watcom's own words
    // of it.
    const u32 major = version & 0xff, minor = (version >> 8) & 0xff;
    putByte(app, kOsMajor, u8(major));
    putHalf(app, kOsBuild, u16(version >> 16));
    put(app, kOsVersion, half(app, kOsBuild));
    put(app, kWinMajor, major);
    putByte(app, kOsMinor, u8(minor));
    put(app, kWinMinor, minor);
    put(app, kWinVersion, major << 8 | minor);
    moduleNameOf(app, 0, kModuleName);
    put(app, kProgramName, kModuleName);
    calls([&] { return moduleWideNameOf(app, ts, 0, kWideModuleName, 0x208); });
    if (ts.terminate)
        return 0;
    put(app, kWideProgramName, kWideModuleName);

    // The command line's copy, and its arguments.
    const u32 copy = calls([&] { return copyCommandLineOf(app, ts, os::commandLine(app)); });
    if (ts.terminate)
        return 0;
    put(app, kCommandLine, copy);
    const u32 narrowArguments = argumentsOf(app, copy, 1, [&](u32 p) { return u32(byte(app, p)); });
    put(app, kArguments, narrowArguments);

    // The wide one's, the same: the port has none.
    const u32 wide = 0;
    u32 arguments = kNoWideCommandLine;
    if (wide != 0)
    {
        const u32 wideCopy = calls([&] { return copyWideCommandLineOf(app, ts, wide); });
        if (ts.terminate)
            return 0;
        put(app, kWideCommandLine, wideCopy);
        arguments = argumentsOf(app, wideCopy, 2, [&](u32 p) { return u32(half(app, p)); });
    }
    put(app, kWideArguments, arguments);

    // A DLL's own name, and wide.
    if (dll != 0)
    {
        moduleNameOf(app, module, kDllName);
        put(app, kDllNamePtr, kDllName);
        calls([&] { return moduleWideNameOf(app, ts, module, kWideDllName, 0x208); });
        if (ts.terminate)
            return 0;
        put(app, kWideDllNamePtr, kWideDllName);
    }
    return 1;
}

nfs::u32 NTInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = ntInitOf(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x4ff338 (the program's start): __NTInit for the module, the stack's limits, the exception
 * handling with the registration at `registration`, the thread's data at `data`, and the
 * initializers.  At its entry's depth: four pushes.  What the code leaves in eax: the initializers'
 * table's end; 0 with ts.terminate set when the game stopped. */
u32 ntMainOf(nfs::App* app, nfs::ThreadState& ts, u32 registration, u32 data)
{
    const GuestFrame frame(ts, 0x10, 0);
    const u32 module = os::kProgramModule;
    auto calls = [&](auto&& run) {
        const ReturnSlot slot(ts);
        return run();
    };
    calls([&] { return ntInitOf(app, ts, 0, data, module); });
    if (ts.terminate)
        return 0;
    const u32 thread = threadPtrOf(app, ts, kStackTop);
    if (ts.terminate)
        return 0;
    calls([&] { return stackLimitsOf(app, ts, thread, kStackTop); });
    if (ts.terminate)
        return 0;
    calls([&] { return initExceptionHandlingOf(app, ts, registration); });
    if (ts.terminate)
        return 0;
    const u32 end = calls([&] { return initRtnsOf(app, ts, 0x21); });
    if (ts.terminate)
        return 0;
    {
        const EspKept kept(ts);
        if (!hook<kAccessIobHook>(app, ts, {end}))
            return 0;
    }
    return calls([&] { return initRtnsOf(app, ts, 0xff); });
}

nfs::u32 NTMain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = ntMainOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x524290 (the call of main): the stack's room checked -- grown by __GRO
 * where stackavail says it is short -- and where it ends noted; then the
 * game's main with argc and argv, and exit. */
nfs::u32 callMain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    push(app, ts, args.edx);
    push(app, ts, args.ebp);
    args.ebp = ts.sp;
    args.edx = at(app, kStackSize) + 3;
    setLow8(args.edx, args.edx & 0xfc);
    if (!direct(app, ts, stackavail, args))
        return args.eax;
    if (args.edx < args.eax)
    {
        push(app, ts, args.edx);
        if (!direct(app, ts, GRO, args))
            return args.eax;
        args.eax = at(app, kStackSize) + 3;
        setLow8(args.eax, args.eax & 0xfc);
        ts.sp -= args.eax;
        args.eax = ts.sp;
    }
    else
        args.eax = 0;
    args.edx = at(app, kStackSize);
    args.eax += args.edx;
    put(app, kStackLimit, args.eax);
    if (!direct(app, ts, startHook, args))
        return args.eax;
    args.edx = at(app, kArgv);
    args.eax = at(app, kArgc);
    if (!indirect(app, ts, kGameMain, args) || !indirect(app, ts, kExitEntry, args))
        return args.eax;
    ts.sp = args.ebp;
    args.ebp = pop(app, ts);
    args.edx = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* 0x5158a0 (the main thread's start): the initializers of priority 1, the
 * thread's data zeroed on the stack, the program's start with a
 * registration on the stack too, and main. */
nfs::u32 mainStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    for (const u32 r : {args.ebx, args.ecx, args.edx, args.ebp})
        push(app, ts, r);
    args.ebp = ts.sp;
    ts.sp -= 8;
    args.eax = 1;
    if (!direct(app, ts, InitRtns, args))
        return args.eax;
    args.eax = at(app, kThreadDataSize) + 3;
    setLow8(args.eax, args.eax & 0xfc);
    args.edx = 0;
    ts.sp -= args.eax;
    args.ecx = ts.sp;
    args.ebx = at(app, kThreadDataSize);
    args.eax = args.ecx;
    if (!direct(app, ts, memset, args))
        return args.eax;
    args.eax = at(app, kThreadDataSize);
    args.edx = args.ecx;
    put(app, args.ecx + 0xf0, args.eax);
    args.eax = args.ebp - 8;
    if (!direct(app, ts, NTMain, args) || !direct(app, ts, callMain, args))
        return args.eax;
    ts.sp = args.ebp;
    for (u32* r : {&args.ebp, &args.edx, &args.ecx, &args.ebx})
        *r = pop(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* 0x4f7fa0 (_cstart_): the program's entry, a jmp to the main thread's
 * start. */
nfs::u32 cstart_(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    mainStart(app, ts, args);
    return args.eax;
}


/* ---- The maths' error report: __reporterror, which builds _matherr's struct
 * exception and prints the error when _matherr does not take it.  (The game's
 * way round the Pentium's FDIV bug, __fpatan_wrap and __fdiv_fpr, is gone: no
 * phone has it.)  The x87's instructions are the same operations of the port's
 * x87 as the game's code has for them, in the same order, so that the
 * results, the control word and the status word are its to the bit. ---- */
namespace
{

constexpr u32 kMatherr = 0x56b16c;          // _matherr's pointer
constexpr u32 kMathErrorNames = 0x56af70;   // the functions' names, by __reporterror's code
constexpr u32 kMathTypeNames = 0x56b150;    // "DOMAIN", "SING" ...: the error's type's name
constexpr u32 kMathErrorSeparator = 0x551fed;  // ": " between them
constexpr u32 kStdStreams = 0x566f58;       // __iob
constexpr u32 kFdivTable = 0x56ccdc;        // __fdiv_fpr's: 8 variants for each register
constexpr u32 kFdivBadNibbles = 0x56ccc0;   // which of a divisor's top nibbles the bug can hit
constexpr u32 kFdivScaleDown = 0x56ccd0, kFdivScaleUp = 0x56ccd4;  // 15/16 and 17/16, floats
constexpr u32 kFdivDenormalScale = 0x56ccd8;  // 2^63, a float
constexpr u32 kAtanTable = 0x56cb38;        // atan2's ways out, by the quadrant and the swap
constexpr u32 kAtanSteps = 0x56cb6c, kAtanStep = 0x56cb70;  // the table's 16 steps, floats
constexpr u32 kOne = 0x56cb28;              // 1.0, a double
constexpr u32 kAtanPolynomial = 0x56cb74;   // 6 long doubles
constexpr u32 kAtanAngles = 0x56cbb0;       // the steps' angles, long doubles 16 bytes apart
constexpr u32 kHalfPiLong = 0x56cb58, kPiLong = 0x56cb62;  // pi/2 and pi, long doubles

nfs::Float loadLong(nfs::App* app, u32 address)
{
    return nfs::Float(app->getMemory<nfs::Float80>(address));
}

void storeLong(nfs::App* app, u32 address, const nfs::Float& value)
{
    app->getMemory<nfs::Float80>(address) = nfs::Float(value);
}

nfs::Float loadSingle(nfs::App* app, u32 address)
{
    return nfs::Float(app->getMemory<float>(address));
}

/* _matherr's hook as __matherr's call makes it: eax and ebx the struct exception, the others kept;
 * its eax, nullopt when the game stopped (a call through the runtime's pointer, as hook's). */
std::optional<u32> matherrHook(nfs::App* app, nfs::ThreadState& ts, u32 exception)
{
    const EspKept kept(ts);
    nfs::Args a;
    a.eax = exception;
    a.ebx = exception;
    return guest::pointerCall(app, ts, at(app, kMatherr), a);
}

}  // namespace

/* 0x52e400 (__get_std_stream): __iob's `n`th FILE, the first four; 0 for another. */
u32 stdStreamOf(u32 n)
{
    return n > 3 ? 0 : n * 0x1a + kStdStreams;
}

nfs::u32 get_std_stream(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    pushed(app, ts.sp, {args.edx});
    returns(ts, args, stdStreamOf(args.eax));
    return args.eax;
}

/* 0x52b647 (the maths' error printed): "<type's name>: <function's name>" and a newline, to stderr,
 * by fputs and fputc -- each at the depth of its call, under three pushes (their room only).
 * fputc's say; 0 with ts.terminate set when the game stopped. */
u32 printMathErrorOf(nfs::App* app, nfs::ThreadState& ts, u32 type, u32 name)
{
    const GuestFrame frame(ts, 0xc, 0);
    const u32 stream = stdStreamOf(2);
    const ReturnSlot slot(ts);
    u32 result;
    for (const u32 s : {at(app, kMathTypeNames + type * 4), kMathErrorSeparator, name})
    {
        const EspKept kept(ts);
        if (!putString(app, ts, s, stream, result))
            return 0;
    }
    const EspKept kept(ts);
    return putChar(app, ts, '\n', stream, result) ? result : 0;
}

nfs::u32 printMathError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = printMathErrorOf(app, ts, args.eax, args.edx);
    if (ts.terminate)
        return args.eax;
    returns(ts, args, result);
    return args.eax;
}

/* 0x52b68f (__matherr's call): the struct exception at `exception` to _matherr; for a 0 from it, the
 * error printed, and errno EDOM for a domain error, else ERANGE; its retval given back in `retval`,
 * what the code leaves on the x87's stack.  At its entry's depth: ebx and edx pushed (their room
 * only).  What the code leaves in eax: _matherr's say, or the errno's setting's (the thread's data);
 * 0 with ts.terminate set when the game stopped. */
u32 callMatherrOf(nfs::App* app, nfs::ThreadState& ts, u32 exception, nfs::Float& retval)
{
    const GuestFrame frame(ts, 8, 0);
    const auto said = matherrHook(app, ts, exception);
    if (!said)
        return 0;
    u32 eax = *said;
    if (eax == 0)
    {
        {
            const ReturnSlot slot(ts);
            printMathErrorOf(app, ts, at(app, exception), at(app, exception + 4));
        }
        if (ts.terminate)
            return 0;
        const ReturnSlot slot(ts);
        const EspKept kept(ts);
        if (!setThreadWord(app, ts, kErrno, at(app, exception) == 1 ? kEDOM : kERANGE, &eax))
            return 0;
    }
    retval = loadDouble(app, exception + 0x18);
    return eax;
}

nfs::u32 callMatherr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float retval;
    const u32 eax = callMatherrOf(app, ts, args.eax, retval);
    if (ts.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x5243b1 (__reporterror's struct exception): the error's `flags` -- which function (the low 5
 * bits), and of what type -- its arguments the doubles at `a` and `b`: the struct built in its
 * frame, the result the type says -- -HUGE_VAL, 0, 1, HUGE_VAL or the second argument -- and
 * _matherr's call with it, its retval given back in `retval`.  A type none of the flags give is
 * `esi`'s, what the caller has in esi.  At its entry's depth: ebp, ecx and esi pushed (their room
 * only), then the struct's 0x20 bytes.  What the code leaves in eax; 0 with ts.terminate set when
 * the game stopped. */
u32 reportErrorOf(nfs::App* app, nfs::ThreadState& ts, u32 flags, u32 a, u32 b, u32 esi,
                    nfs::Float& retval)
{
    u32 type = esi;
    const GuestFrame frame(ts, 0xc, 0x20);
    const u32 exception = frame - 0x20;
    put(app, exception + 8, at(app, a));
    put(app, exception + 0xc, at(app, a + 4));
    put(app, exception + 0x10, at(app, b));
    put(app, exception + 0x14, at(app, b + 4));
    const u8 low = u8(flags), high = u8(flags >> 8);
    if (low & 0x40)
        type = 1;  // DOMAIN
    else if (low & 0x80)
        type = 2;  // SING
    else if (high & 1)
        type = 3;  // OVERFLOW
    else if (high & 8)
        type = 4;  // UNDERFLOW
    else if (high & 2)
        type = 6;  // PLOSS
    else if (high & 4)
        type = 5;  // TLOSS
    put(app, exception, type);
    put(app, exception + 4, at(app, kMathErrorNames + (flags & 0x1f) * 4));
    const u32 result = exception + 0x18;
    if (high & 0x10)
    {
        const nfs::Float huge = loadDouble(app, kHugeVal);
        const nfs::Float negative = -huge;
        storeDouble(app, result, negative);
    }
    else if (high & 0x20)
    {
        put(app, result, 0);
        put(app, result + 4, 0);
    }
    else if (high & 0x40)
    {
        put(app, result, 0);
        put(app, result + 4, 0x3ff00000);
    }
    else if (high & 0x80)
    {
        put(app, result, at(app, kHugeVal));
        put(app, result + 4, at(app, kHugeVal + 4));
    }
    else
    {
        put(app, result, at(app, b));
        put(app, result + 4, at(app, b + 4));
    }
    const ReturnSlot slot(ts);
    return callMatherrOf(app, ts, exception, retval);
}

nfs::u32 reportError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float retval;
    const u32 eax = reportErrorOf(app, ts, args.eax, args.edx, args.ebx, args.esi, retval);
    if (ts.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(ts, args, eax);
    return args.eax;
}

/* 0x5243a7 (__reporterror): reportError with the argument at `a` for both.  At its entry's depth:
 * ebx pushed (its room only). */
u32 reporterrorOf(nfs::App* app, nfs::ThreadState& ts, u32 flags, u32 a, u32 esi,
                    nfs::Float& retval)
{
    const GuestFrame frame(ts, 4, 0);
    const ReturnSlot slot(ts);
    return reportErrorOf(app, ts, flags, a, a, esi, retval);
}

nfs::u32 reporterror(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::Float retval;
    const u32 eax = reporterrorOf(app, ts, args.eax, args.edx, args.esi, retval);
    if (ts.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(ts, args, eax);
    return args.eax;
}

/* The two as typed functions (native_watcom.h): what the code leaves on the x87's stack given back,
 * as guest::direct calls them -- the return address's slot under esp.  Every flags their callers
 * hand them names a type, so esi, which only a flags that names none reads, is not theirs. */
double clib::reporterror(nfs::App* app, nfs::ThreadState& ts, nfs::u32 flags, nfs::u32 argument)
{
    nfs::Float retval = 0.0;
    reporterrorOf(app, ts, flags, argument, 0, retval);
    return ts.terminate ? 0.0 : double(retval);
}

double clib::reportError(nfs::App* app, nfs::ThreadState& ts, nfs::u32 flags, nfs::u32 a, nfs::u32 b)
{
    nfs::Float retval = 0.0;
    reportErrorOf(app, ts, flags, a, b, 0, retval);
    return ts.terminate ? 0.0 : double(retval);
}

/* ---- The formatted ones as typed functions (native_watcom.h): the functions
 * above run as the game's code ran them, their words pushed below esp as its
 * call pushed them, without the dispatch by address.  Every register kept but
 * eax, as their code keeps them. ---- */

namespace
{

/* A cdecl function of the runtime called as C++ (guest::direct), the return address's slot already
 * under esp: the words pushed where the game's call pushes them, over that slot, so the function's
 * frame -- and the va_list it points into it -- lies where its code has it (as ea::error). */
template <typename Run>
nfs::u32 variadic(nfs::App* app, nfs::ThreadState& ts, std::initializer_list<nfs::u32> named,
                    const guest::Words& words, Run run)
{
    const nfs::u32 esp = ts.sp + 4;
    nfs::u32 slot = esp - 4 * nfs::u32(named.size() + words.count);
    const nfs::u32 first = slot;
    for (const nfs::u32 word : named)
    {
        put(app, slot, word);
        slot += 4;
    }
    for (unsigned i = 0; i < words.count; ++i)
    {
        put(app, slot, words.word[i]);
        slot += 4;
    }
    ts.sp = first - 4;
    const nfs::u32 n = run(first);
    if (ts.terminate)
        return 0;
    ts.sp = esp - 4;
    return n;
}

}  // namespace

nfs::u32 clib::sprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 format,
                         guest::Words words)
{
    // On the host, with the words in hand, where it can be (hostSprintf); eax its count, as
    // sprintf's code leaves it.
    if (const auto n = hostSprintf(app, Segments{}, buffer, format, words))
        return *n;
    return variadic(app, ts, {buffer, format}, words,
                    [&](nfs::u32 first) { return sprintfOf(app, ts, Segments{}, buffer, format, first + 8); });
}

nfs::u32 clib::fprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 format,
                         guest::Words words)
{
    return variadic(app, ts, {stream, format}, words,
                    [&](nfs::u32 first) { return fprintfOf(app, ts, Segments{}, stream, format, first + 8); });
}

nfs::u32 clib::sscanf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string, nfs::u32 format,
                        guest::Words words)
{
    return variadic(app, ts, {string, format}, words, [&](nfs::u32 first) {
        const EspKept kept(ts);
        return sscanfOf(app, ts, string, format, first + 8);
    });
}

namespace
{

/* A function of the runtime run as the game's code calls it: `registers` into eax, edx, ebx and
 * ecx in turn, every other register kept, eax back -- 0 when the game stopped in it. */
nfs::u32 runs(nfs::App* app, nfs::ThreadState& ts, nfs::ThunkPtr function,
                std::initializer_list<nfs::u32> registers)
{
    const EspKept kept(ts);
    nfs::Args a = watcomArgs(registers);
    ts.sp -= 4;
    const nfs::u32 eax = function(app, ts, a);
    return ts.terminate ? 0 : eax;
}

}  // namespace

nfs::u32 clib::vsprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 format,
                          nfs::u32 arguments)
{
    return called(ts, [&] { return vsprintfOf(app, ts, Segments{}, buffer, format, arguments); });
}

nfs::u32 clib::mkTmpFile(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 number)
{
    return mkTmpFileOf(app, ts, name, number);
}

nfs::u32 clib::getche(nfs::App* app, nfs::ThreadState& ts)
{
    return getcheOf(app, ts);
}

nfs::u32 clib::setErrno(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value)
{
    nfs::u32 data;
    return setThreadWord(app, ts, kErrno, value, &data) ? data : 0;
}

nfs::u32 clib::setERANGE(nfs::App* app, nfs::ThreadState& ts)
{
    return setErangeOf(app, ts);
}

nfs::u32 clib::setErrnoDos(nfs::App* app, nfs::ThreadState& ts, nfs::u32 error)
{
    return watcom::setErrnoDos(app, ts, error) ? 0xffffffff : 0;
}

nfs::u32 clib::setIOMode(nfs::App* app, nfs::u32 handle, nfs::u32 mode)
{
    setIoMode(app, handle, mode);
    return handle << 2;
}

nfs::u32 clib::mbslen(nfs::App* app, nfs::u32 s)
{
    return mbslenOf(app, s);
}

nfs::u32 clib::mbstowcs(nfs::App* app, nfs::ThreadState& ts, nfs::u32 to, nfs::u32 from, nfs::u32 most)
{
    return mbstowcsOf(app, ts, to, from, most);
}

nfs::u32 clib::wputenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pair)
{
    return wputenvOf(app, ts, pair, 0);  // edx, which a pair with text never has read
}

nfs::u32 clib::memmove(nfs::App* app, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    return memmoveOf(app, to, from, count);
}

nfs::u32 clib::itoa(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value, nfs::u32 out, nfs::u32 base)
{
    return itoaOf(app, ts, value, out, base);
}

nfs::u32 clib::mbvtop(nfs::App* app, nfs::u32 c, nfs::u32 to)
{
    return mbvtopOf(app, c, to);
}

nfs::u32 clib::mbclen(nfs::App* app, nfs::u32 p)
{
    return mbclenOf(app, p);
}

nfs::u32 clib::mbccmp(nfs::App* app, nfs::u32 a, nfs::u32 b)
{
    return mbccmpOf(app, a, b);
}

nfs::u32 clib::mbsinc(nfs::App* app, nfs::u32 p)
{
    return mbsincOf(app, p);
}

nfs::u32 clib::mbterm(nfs::App* app, nfs::u32 p)
{
    return mbtermOf(app, p);
}

nfs::u32 clib::fatalRuntimeError(nfs::App* app, nfs::ThreadState& ts, nfs::u32 message, nfs::u32 code)
{
    // Its code still as it runs, register by register (the way out's): called as the game's call
    // makes it.
    return runs(app, ts, &fatal_runtime_error, {message, code});
}

nfs::u32 clib::heapShrink(nfs::App* app, nfs::ThreadState& ts)
{
    watcomheap::shrink(app, ts, kGameHeap);
    return 0;
}

nfs::u32 clib::consoleOutput(nfs::App* app, nfs::ThreadState& ts)
{
    return consoleOutputOf(app, ts);
}

double clib::strtod(nfs::App* app, nfs::ThreadState& ts, nfs::u32 s, nfs::u32 end, nfs::u32* eax)
{
    nfs::u32 left = 0;
    const double d = strtodOf(app, ts, s, end, left);
    if (eax != nullptr)
        *eax = left;
    return d;
}

nfs::u32 clib::allocPOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 os)
{
    nfs::u32 handle = 0;
    return allocHandle(app, ts, os, handle) ? handle : 0;
}

nfs::u32 clib::freePOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle)
{
    nfs::u32 eax = 0;
    return freeHandle(app, ts, handle, eax) ? eax : 0;
}

nfs::u32 clib::initThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::u32 data)
{
    return initThreadDataOf(app, ts, data);
}

nfs::u32 clib::initExceptionHandling(nfs::App* app, nfs::ThreadState& ts, nfs::u32 registration)
{
    return initExceptionHandlingOf(app, ts, registration);
}

nfs::u32 clib::doneExceptionHandler(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax)
{
    NFS2_USE(eax);
    // Its code still as it runs, register by register (the way out's): called as the game's call
    // makes it.
    return runs(app, ts, &DoneExceptionHandler, {});
}

nfs::u32 clib::malloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size)
{
    return called(ts, [&] {
        nfs::u32 block;
        return watcomheap::allocate(app, ts, kGameHeap, size, block) ? block : 0;
    });
}

nfs::u32 clib::free(nfs::App* app, nfs::ThreadState& ts, nfs::u32 block)
{
    // What __nfree's code leaves in eax: the heap lock's giving back's.
    return called(ts, [&] {
        nfs::u32 eax;
        return watcomheap::free(app, ts, kGameHeap, block, &eax) ? eax : 0;
    });
}

nfs::u32 clib::realloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 block, nfs::u32 size)
{
    // As the runtime's code is called: its return address's slot below the caller's esp, every
    // register but eax kept.
    const EspKept kept(ts);
    const ReturnSlot slot(ts);
    return nreallocOf(app, ts, block, size);
}

nfs::u32 clib::calloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 count, nfs::u32 size)
{
    return called(ts, [&] { return callocOf(app, ts, count, size); });
}

nfs::u32 clib::fopen(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 mode)
{
    return called(ts, [&] {
        nfs::u32 result;
        const Frame frame(ts, 4);  // ebx
        return nested(ts, [&] { return openStream(app, ts, name, mode, 0, result); }) ? result : 0;
    });
}

nfs::u32 clib::fclose(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream)
{
    return called(ts, [&] {
        nfs::u32 result;
        return closeStream(app, ts, stream, result) ? result : 0;
    });
}

nfs::u32 clib::fread(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size,
                       nfs::u32 count, nfs::u32 stream)
{
    return called(ts, [&] {
        nfs::u32 result;
        return readStream(app, ts, buffer, size, count, stream, result) ? result : 0;
    });
}

nfs::u32 clib::streamShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 close)
{
    return called(ts, [&] {
        nfs::u32 result;
        return shutdownStream(app, ts, stream, close, result) ? result : 0;
    });
}

void clib::ldScale10x(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value, nfs::u32 n)
{
    called(ts, [&] {
        nfs::Float80 number = longDoubleAt(app, value);
        ldScale10xOf(app, ts.fenv.control.word, number, n);
        putLongDouble(app, value, number);
        return 0;
    });
}

nfs::u32 clib::bsearch(nfs::App* app, nfs::ThreadState& ts, nfs::u32 key, nfs::u32 base, nfs::u32 count,
                         nfs::u32 size, nfs::u32 function)
{
    return called(ts, [&] {
        nfs::u32 found;
        return bsearchOf(app, ts, ts.sp, key, base, count, size, function, found) ? found : 0;
    });
}

nfs::u32 clib::fgets(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 n, nfs::u32 stream)
{
    return called(ts, [&] {
        nfs::u32 result;
        return getString(app, ts, buffer, n, stream, result) ? result : 0;
    });
}

nfs::u32 clib::strupr(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string)
{
    return called(ts, [&] {
        for (nfs::u32 at = string; byte(app, at) != 0; ++at)
            if (byte(app, at) >= 'a' && byte(app, at) <= 'z')
                putByte(app, at, nfs::u8(byte(app, at) - 0x20));
        return string;
    });
}

nfs::u32 clib::fseek(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 offset,
                       nfs::u32 whence)
{
    return called(ts, [&] {
        nfs::u32 result;
        return seekStream(app, ts, stream, offset, whence, result) ? result : 0;
    });
}

nfs::u32 clib::qsort(nfs::App* app, nfs::ThreadState& ts, nfs::u32 base, nfs::u32 count,
                       nfs::u32 size, nfs::u32 compare)
{
    called(ts, [&] { return qsortOf(app, ts, base, count, size, compare); });
    return 0;  // what its code leaves in eax is nothing of its
}

nfs::u32 clib::strtok(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string, nfs::u32 parts)
{
    return called(ts, [&] {
        nfs::u32 result;
        return strtokOf(app, ts, string, parts, result) ? result : 0;
    });
}

nfs::u32 clib::rand(nfs::App* app, nfs::ThreadState& ts)
{
    return called(ts, [&] {
        nfs::u32 result;
        return randOf(app, ts, result) ? result : 0;
    });
}

nfs::u32 clib::getenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name)
{
    // As the runtime's code is called: its return address's slot below the caller's esp.
    const EspKept kept(ts);
    ts.sp -= 4;
    return getenvOf(app, ts, name);
}

nfs::u32 clib::exit(nfs::App* app, nfs::ThreadState& ts, nfs::u32 status)
{
    return runs(app, ts, &watcom::exit, {status});
}

nfs::u32 clib::srand(nfs::App* app, nfs::ThreadState& ts, nfs::u32 seed)
{
    // guest::direct has put the return address's slot under esp and kept the registers.
    nfs::u32 old;
    return srandOf(app, ts, seed, old) ? old : 0;
}

nfs::u32 clib::fwrite(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size,
                        nfs::u32 count, nfs::u32 stream)
{
    nfs::u32 result;
    return writeStream(app, ts, buffer, size, count, stream, result) ? result : 0;
}

nfs::u32 clib::time(nfs::App* app, nfs::ThreadState& ts, nfs::u32 timer)
{
    return timeOf(app, ts, timer).value_or(0);
}

nfs::u32 clib::getcwd(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size)
{
    return getcwdOf(app, ts, buffer, size).value_or(0);
}

nfs::u32 clib::getdiskfree(nfs::App* app, nfs::ThreadState& ts, nfs::u32 drive, nfs::u32 info)
{
    return getdiskfreeOf(app, ts, drive, info).value_or(0);
}

nfs::u32 clib::stackavail(nfs::App* app, nfs::ThreadState& ts)
{
    return stackavailOf(app, ts, 0).value_or(0);
}

nfs::u32 clib::stackProbed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size)
{
    stackProbedOf(app, ts, size, 0, 0);
    return 0;
}

nfs::u32 clib::setenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 value,
                        nfs::u32 replace)
{
    return setenvTopOf(app, ts, name, value, replace);
}

nfs::u32 clib::atexit(nfs::App* app, nfs::u32 function)
{
    return atexitOf(app, function);
}

nfs::u32 clib::unlink(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path)
{
    return called(ts, [&] {
        nfs::u32 result;
        return callResult(app, ts, os::files::remove(guest::text(app, path)), result) ? result : 0;
    });
}

nfs::u32 clib::remove(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path)
{
    return clib::unlink(app, ts, path);
}

nfs::u32 clib::chdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path)
{
    return called(ts, [&] {
        nfs::u32 result;
        os::files::setCurrentDirectory(guest::text(app, path));
        return callResult(app, ts, true, result) ? result : 0;
    });
}

nfs::u32 clib::rmdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path)
{
    return called(ts, [&] {
        nfs::u32 result;
        return callResult(app, ts, os::files::removeDirectory(guest::text(app, path)), result) ? result : 0;
    });
}

/* chmod: the port keeps no attributes, so a file found cannot have its set. */
nfs::u32 clib::chmod(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode)
{
    NFS2_USE(mode);
    return called(ts, [&] {
        nfs::u32 result;
        if (fileAttributes(app, path) == 0xffffffff)
        {
            result = 0xffffffff;
            if (!setErrnoNt(app, ts))
                return nfs::u32(0);
        }
        else if (!callResult(app, ts, false, result))
            return nfs::u32(0);
        return result;
    });
}

/* access: the port reports the files it finds read-only, so asking to write one is refused. */
nfs::u32 clib::access(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode)
{
    return called(ts, [&] {
        nfs::u32 result = 0;
        if (!os::files::isFile(guest::text(app, path)))
        {
            result = 0xffffffff;
            if (!setErrnoNt(app, ts))
                return nfs::u32(0);
        }
        else if ((mode & 2) != 0)
        {
            result = 0xffffffff;
            if (!setErrnoDos(app, ts, 5))  // ERROR_ACCESS_DENIED
                return nfs::u32(0);
        }
        return result;
    });
}

nfs::u32 clib::ftell(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream)
{
    return called(ts, [&] {
        nfs::u32 result;
        return tellStream(app, ts, stream, result) ? result : 0;
    });
}

nfs::u32 clib::vfprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 format,
                          nfs::u32 argp)
{
    return called(ts, [&] {
        const nfs::u32 n = vfprintfOf(app, ts, segmentsOf(ts), stream, format, argp);
        return ts.terminate ? nfs::u32(0) : n;
    });
}

nfs::u32 clib::fflush(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream)
{
    return called(ts, [&] {
        nfs::u32 result = 0;
        if (stream != 0)
            return flushStream(app, ts, stream, result) ? result : nfs::u32(0);
        nfs::u32 count;
        nested(ts, [&] { return flushAll(app, ts, 0xffffffff, count); });
        return nfs::u32(0);
    });
}

nfs::u32 clib::mkdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path)
{
    return called(ts, [&] {
        nfs::u32 result;
        return callResult(app, ts, os::files::makeDirectory(guest::text(app, path)), result) ? result : 0;
    });
}

}  // namespace watcom
}  // namespace nfs3hp
