#include "native_watcom.h"
#include "native_watcom_heap.h"
#include "native_windows.h"
#include "watcom_hooks.h"
#include "host_threads.h"
#include <lib/thread.h>
#include "host_files.h"
#include "host_system.h"
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

x86::reg32 tzset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);  // 0x515c2c


namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;

/* The runtime's per-thread data: [__GetThreadPtr]() -- the pointer the
 * runtime swaps for its own when a second thread starts (watcom_hooks.h). */
constexpr x86::reg32 kGetThreadPtr = 0x567764;
constexpr x86::reg32 kThreadData = 0x9f642c;  // the single thread's, what GetThreadPtr hands back
constexpr x86::reg32 kRandomSeed = 0xc;        // rand's, in the thread's data
constexpr x86::reg32 kNextToken = 0x10;        // strtok's
/* _IsTable: a byte of flags for each character, EOF first. */
constexpr x86::reg32 kIsTable = 0x564ef0;
constexpr x86::reg8 kSpace = 0x02;
constexpr x86::reg8 kDigit = 0x20;

/* The registers of a call by Watcom's convention: `registers` into eax, edx, ebx and ecx in turn. */
x86::Args watcomArgs(std::initializer_list<x86::reg32> registers)
{
    x86::Args a;
    x86::reg32* const into[] = {&a.eax, &a.edx, &a.ebx, &a.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : registers)
        *into[i++] = value;
    return a;
}

/* A call through a pointer the runtime holds -- one of its hooks (watcom_hooks.h), or a function its
 * caller handed in or its tables list -- as its code makes it: `registers` into eax, edx, ebx and
 * ecx in turn; its eax, nullopt when the game stopped in it. */
template <x86::reg32 Slot>
std::optional<x86::reg32> hook(win32::WinApplication* app, x86::CPU& cpu,
                               std::initializer_list<x86::reg32> registers = {})
{
    x86::Args a = watcomArgs(registers);
    return guest::pointerCall(app, cpu, at(app, Slot), a);
}

/* The hook at Slot called with the registers of `args` (the thunk's own: what its code has in them at
 * the call), what the hook leaves in them left there, eax too; false when the game stopped. */
template <x86::reg32 Slot>
bool hookWith(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<x86::reg32> eax = guest::pointerCall(app, cpu, at(app, Slot), args);
    if (!eax)
        return false;
    args.eax = *eax;
    return true;
}

std::optional<x86::reg32> through(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                                  std::initializer_list<x86::reg32> registers = {},
                                  std::initializer_list<x86::reg32> words = {})
{
    x86::Args a = watcomArgs(registers);
    return guest::pointerCall(app, cpu, target, a, words);
}

/* One of the runtime's own functions, which its code calls, run as C++ where the call is: the
 * return address's slot under esp, and every register but eax as the caller had it, as the
 * function's thunk keeps them (Kept), esp back after.  What `function` gives back. */
template <typename Function>
auto called(x86::CPU& cpu, Function function)
{
    const EspKept kept(cpu);
    cpu.esp -= 4;
    return function();
}

/* Room on the guest's stack for what a callee is handed the address of: the code's frame (`pushed`
 * bytes of registers under esp on entry, then `bytes`), esp under it while the core runs and back
 * after; an address in it as the frame's base less the offset. */
class GuestFrame
{
public:
    GuestFrame(x86::CPU& cpu, x86::reg32 pushed, x86::reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    /* The registers the code saves, `saved`, where its pushes put them: what a frame at this
     * depth later finds there. */
    GuestFrame(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<x86::reg32> saved, x86::reg32 bytes)
        : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - 4 * x86::reg32(saved.size()))
    {
        x86::reg32 p = esp_;
        for (const x86::reg32 value : saved)
            put(app, p -= 4, value);
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    x86::reg32 operator-(x86::reg32 offset) const { return base_ - offset; }

private:
    x86::CPU& cpu_;
    const x86::reg32 esp_;
    const x86::reg32 base_;
};

/* The return address's slot under esp, as a call makes it: a core that keeps a frame on the
 * guest's stack is called at the code's depth. */
class ReturnSlot
{
public:
    explicit ReturnSlot(x86::CPU& cpu) : cpu_(cpu), esp_(cpu.esp) { cpu.esp -= 4; }
    ~ReturnSlot() { cpu_.esp = esp_; }
    ReturnSlot(const ReturnSlot&) = delete;
    ReturnSlot& operator=(const ReturnSlot&) = delete;

private:
    x86::CPU& cpu_;
    const x86::reg32 esp_;
};

/* The thread's data, as the runtime asks for it; false when the game
 * stopped. */
bool threadData(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& data)
{
    const auto eax = hook<kGetThreadPtr>(app, cpu);
    if (!eax)
        return false;
    data = *eax;
    return true;
}

void returns(x86::CPU& cpu, x86::Args& args, x86::reg32 result)
{
    args.eax = result;
    cpu.esp += 4;
}

/* The registers but eax, as a function had them when it was called (its thunk's args). */
struct Kept
{
    x86::reg32 ebx, ecx, edx, esi, edi, ebp;
    explicit Kept(const x86::Args& args)
        : ebx(args.ebx), ecx(args.ecx), edx(args.edx), esi(args.esi), edi(args.edi), ebp(args.ebp)
    {
    }
    void back(x86::Args& args) const
    {
        args.ebx = ebx;
        args.ecx = ecx;
        args.edx = edx;
        args.esi = esi;
        args.edi = edi;
        args.ebp = ebp;
    }
};

/* A Windows function, its words handed in the order of its parameters: the port's own, called as
 * C++ (native_windows.h).  What it
 * gave back, nullopt when the game stopped in it. */
template <x86::reg32 Slot, auto Function, typename... Words>
std::optional<x86::reg32> windows(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    return guest::windows<Slot, Function>(app, cpu, words...);
}

/* The same with eax `eax`, as the code has it at the call: what Windows gives back in the game. */
template <x86::reg32 Slot, auto Function, typename... Words>
std::optional<x86::reg32> windowsAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, Words... words)
{
    return guest::windowsKeeping<Slot, Function>(app, cpu, eax, words...);
}

/* Plan step 7: the runtime's string and memory functions on the host's C library, over guest
 * memory where it lies in the host's (the guest's flat memory): `hostBytes` the bytes at a guest
 * address, `apart` whether two ranges share no byte and neither wraps past 4 GB -- where the
 * runtime's own loops (a dword at a time, a byte at a time) and the host's copies cannot differ.
 * Where they can, the runtime's loops stay. */
char* hostBytes(win32::WinApplication* app, x86::reg32 address)
{
    return &app->getMemory<char>(address);
}

bool apart(x86::reg32 a, x86::reg32 aLength, x86::reg32 b, x86::reg32 bLength)
{
    const std::uint64_t aEnd = std::uint64_t(a) + aLength, bEnd = std::uint64_t(b) + bLength;
    return aEnd <= 0x100000000ull && bEnd <= 0x100000000ull && (aEnd <= b || bEnd <= a);
}

}  // namespace

/* __CHP (mathlib/a/fchop87.asm, 0x4dfd56): st(0) rounded toward zero -- the
 * game's step from a float to an integer, before every fistp -- by frndint
 * under the caller's control word with truncation in it.  Nothing else
 * changes: the registers, the control word, the status word. */
x86::reg32 CHP(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = clib::chop(app, args.floatIn[0]);
    cpu.esp += 4;
    return args.eax;
}

double clib::chop(win32::WinApplication* app, double value)
{
    NFS2_USE(app);
    return std::trunc(value);
}

/* __GetThreadPtr (0x4ff0ec): the single thread's data, until a second thread
 * starts and the runtime points kGetThreadPtr at its own. */
x86::reg32 GetThreadPtr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, at(app, kThreadData));
    return args.eax;
}

/* __NullAccessRtn (0x4ff0f4): what the runtime's locks are -- the streams',
 * the heap's -- while there is one thread: nothing. */
x86::reg32 NullAccess(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    cpu.esp += 4;
    return args.eax;
}

/* memset (clib/memory/c/memset.c, 0x4e0640): eax the buffer, edx the byte,
 * ebx how many; the buffer back. */
x86::reg32 clib::memset(win32::WinApplication* app, x86::reg32 buffer, x86::reg32 value, x86::reg32 count)
{
    if (count != 0)
        std::memset(&app->getMemory<x86::reg8>(buffer), int(value & 0xff), count);
    return buffer;
}

x86::reg32 memset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::memset(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* __STOSD (clib/cgsupp/a/__stos.asm, 0x4ff4f7): ecx dwords of edx from eax
 * on -- one by one to a 32-byte boundary, then four at a time, then up to
 * three; eax left past them, and ecx 0, but 1 after three at the end (the
 * last of them is not counted down). */
x86::reg32 STOSD(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    for (; args.ecx != 0 && (args.eax & 0x1f) != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    const x86::reg32 tail = args.ecx & 3;
    for (; args.ecx != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    args.ecx = tail == 3 ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

/* __STOSB (clib/cgsupp/a/__stos.asm, 0x4ff4c0): ecx bytes of the pattern in
 * edx from eax on -- byte by byte up to a dword boundary, the pattern turned
 * a byte for each, then dwords (__STOSD), then up to three bytes more: dl,
 * dh, dl.  eax is left past the dwords, edx as turned, ecx 0 -- or 1 after
 * three bytes at the end, as __STOSD. */
x86::reg32 STOSB(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    for (; args.ecx != 0 && (args.eax & 3) != 0; --args.ecx, ++args.eax)
    {
        putByte(app, args.eax, x86::reg8(args.edx));
        args.edx = args.edx >> 8 | args.edx << 24;
    }
    const x86::reg32 tail = args.ecx & 3;
    for (args.ecx >>= 2; args.ecx != 0; --args.ecx, args.eax += 4)
        put(app, args.eax, args.edx);
    const x86::reg8 bytes[] = {x86::reg8(args.edx), x86::reg8(args.edx >> 8), x86::reg8(args.edx)};
    for (x86::reg32 i = 0; i < tail; ++i)
        putByte(app, args.eax + i, bytes[i]);
    args.ecx = tail == 3 ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

/* strcmp (clib/string/a/cmp386.asm, 0x4ee310): eax and edx, the strings;
 * -1, 0 or 1 as the first byte that differs, unsigned, is below or above. */
x86::reg32 clib::strcmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b)
{
    // The host's, which compares as unsigned bytes too; its sign as -1, 0 or 1.
    const int d = std::strcmp(hostBytes(app, a), hostBytes(app, b));
    return d < 0 ? 0xffffffff : d > 0 ? 1 : 0;
}

x86::reg32 strcmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strcmp(app, args.eax, args.edx));
    return args.eax;
}

/* strncmp (clib/string/c/strncmp.c, 0x4eb390): eax and edx, the strings, ebx
 * the most to compare; the first difference of unsigned bytes, or 0. */
x86::reg32 clib::strncmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 count)
{
    for (; count != 0; ++a, ++b, --count)
    {
        const x86::reg8 c = byte(app, a), d = byte(app, b);
        if (c != d)
            return x86::reg32(c) - x86::reg32(d);
        if (c == 0)
            break;
    }
    return 0;
}

x86::reg32 strncmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strncmp(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* _stricmp (clib/string/c/stricmp.c, 0x4e9ec0): strcmp's difference, unsigned,
 * with A to Z taken as a to z and nothing else folded. */
x86::reg32 clib::stricmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b)
{
    const auto lower = [](x86::reg8 c) { return x86::reg8(c >= 'A' && c <= 'Z' ? c + 0x20 : c); };
    for (;; ++a, ++b)
    {
        const x86::reg8 c = lower(byte(app, a)), d = lower(byte(app, b));
        if (c != d || d == 0)
            return x86::reg32(c) - x86::reg32(d);
    }
}

x86::reg32 stricmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::stricmp(app, args.eax, args.edx));
    return args.eax;
}

/* strncpy (clib/string/c/strncpy.c, 0x4e0e30): eax the buffer, edx the
 * string, ebx how many bytes: the string, then zeros to make up the count. */
x86::reg32 clib::strncpy(win32::WinApplication* app, x86::reg32 buffer, x86::reg32 string, x86::reg32 count)
{
    // The host's copy and fill where the string and the buffer share no byte; a byte at a time,
    // each read before it is written, where they do.
    const x86::reg32 length = x86::reg32(strnlen(hostBytes(app, string), count));
    if (apart(buffer, count, string, length + (length < count ? 1 : 0)))
    {
        std::memcpy(hostBytes(app, buffer), hostBytes(app, string), length);
        std::memset(hostBytes(app, buffer) + length, 0, count - length);
        return buffer;
    }
    x86::reg32 to = buffer, from = string;
    for (; count != 0 && byte(app, from) != 0; --count)
        putByte(app, to++, byte(app, from++));
    for (; count != 0; --count)
        putByte(app, to++, 0);
    return buffer;
}

x86::reg32 strncpy(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strncpy(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* strncat (clib/string/c/strncat.c, 0x4eedf0): eax the string, edx what to add,
 * ebx at most how much of it; always ended. */
x86::reg32 clib::strncat(win32::WinApplication* app, x86::reg32 string, x86::reg32 added, x86::reg32 count)
{
    x86::reg32 to = string;
    while (byte(app, to) != 0)
        ++to;
    for (x86::reg32 from = added; count != 0; --count, ++to, ++from)
    {
        const x86::reg8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
    putByte(app, to, 0);
    return string;
}

x86::reg32 strncat(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strncat(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* strrchr (clib/string/c/strrchr.c, 0x4ee2f0): eax the string, dl the byte;
 * where it is last, the end included, or 0. */
x86::reg32 clib::strrchr(win32::WinApplication* app, x86::reg32 string, x86::reg32 c)
{
    const char* const from = hostBytes(app, string);
    const char* const found = std::strrchr(from, int(x86::reg8(c)));
    return found != nullptr ? string + x86::reg32(found - from) : 0;
}

x86::reg32 strrchr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strrchr(app, args.eax, args.edx));
    return args.eax;
}

/* strstr (clib/string/c/strstr.c, 0x4e0820): eax the string, edx what to look
 * for; where it first is, or 0 -- the string itself for an empty one. */
x86::reg32 clib::strstr(win32::WinApplication* app, x86::reg32 string, x86::reg32 wanted)
{
    const char* const from = hostBytes(app, string);
    const char* const found = std::strstr(from, hostBytes(app, wanted));
    return found != nullptr ? string + x86::reg32(found - from) : 0;
}

x86::reg32 strstr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::strstr(app, args.eax, args.edx));
    return args.eax;
}

/* __setbits (clib/string/c/setbits.c, 0x50c540): eax 32 bytes, made the set
 * of the bytes of the string at edx -- a bit each, the low bit first. */
x86::reg32 setbits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 vector = args.eax;
    std::memset(&app->getMemory<x86::reg8>(vector), 0, 0x20);
    for (x86::reg32 at = args.edx; byte(app, at) != 0; ++at)
    {
        const x86::reg8 c = byte(app, at);
        putByte(app, vector + (c >> 3), x86::reg8(byte(app, vector + (c >> 3)) | 1 << (c & 7)));
    }
    cpu.esp += 4;
    return args.eax;
}

/* strtok (clib/string/c/strtok.c, 0x4eff60): eax the string -- 0 to go on
 * where the last call stopped, kept in the thread's data -- edx the bytes
 * that part the tokens; the next token, ended, or 0 when there is none. */
/* strtok's work: the next token into `result`; false when the game stopped. */
bool strtokOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, const x86::reg32 parts,
              x86::reg32& result)
{
    x86::reg32 data = 0;
    if (text == 0)
    {
        if (!threadData(app, cpu, data))
            return false;
        text = at(app, data + kNextToken);
        if (text == 0)
            return result = 0, true;
    }
    std::bitset<256> parting;
    for (x86::reg32 at = parts; byte(app, at) != 0; ++at)
        parting.set(byte(app, at));
    while (byte(app, text) != 0 && parting.test(byte(app, text)))
        ++text;
    if (byte(app, text) == 0)
        return result = 0, true;
    for (x86::reg32 at = text; byte(app, at) != 0; ++at)
        if (parting.test(byte(app, at)))
        {
            putByte(app, at, 0);
            if (!threadData(app, cpu, data))
                return false;
            put(app, data + kNextToken, at + 1);
            return result = text, true;
        }
    if (!threadData(app, cpu, data))
        return false;
    put(app, data + kNextToken, 0);
    result = text;
    return true;
}

x86::reg32 strtok(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 result;
    if (!strtokOf(app, cpu, args.eax, args.edx, result))
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* _strupr (clib/string/c/strupr.c, 0x4f8020): a to z of the string at eax made
 * A to Z, in place; the string back. */
x86::reg32 strupr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    for (x86::reg32 at = args.eax; byte(app, at) != 0; ++at)
        if (byte(app, at) >= 'a' && byte(app, at) <= 'z')
            putByte(app, at, x86::reg8(byte(app, at) - 0x20));
    returns(cpu, args, args.eax);
    return args.eax;
}

/* toupper and tolower (clib/char/c, 0x4eede0 and 0x4f0ae0): eax a character,
 * moved between a-z and A-Z; anything else as it is. */
x86::reg32 clib::toupper(win32::WinApplication* app, x86::reg32 c)
{
    NFS2_USE(app);
    const x86::sreg32 value = x86::sreg32(c);
    return value >= 'a' && value <= 'z' ? c - 0x20 : c;
}

x86::reg32 clib::tolower(win32::WinApplication* app, x86::reg32 c)
{
    NFS2_USE(app);
    const x86::sreg32 value = x86::sreg32(c);
    return value >= 'A' && value <= 'Z' ? c + 0x20 : c;
}

x86::reg32 toupper(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::toupper(app, args.eax));
    return args.eax;
}

x86::reg32 tolower(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::tolower(app, args.eax));
    return args.eax;
}

/* atoi and atol (clib/convert/c, 0x4f0430 and 0x4f0490, the same code): the
 * number at eax, past white space, a sign and then digits as _IsTable has
 * them, in 32 bits as it wraps.  The table is looked up at the character
 * plus one in a byte: 0xff at EOF's place. */
x86::reg32 clib::atoi(win32::WinApplication* app, x86::reg32 string)
{
    const auto is = [&](x86::reg8 c, x86::reg8 what) {
        return (byte(app, kIsTable + x86::reg8(c + 1)) & what) != 0;
    };
    x86::reg32 at = string;
    while (is(byte(app, at), kSpace))
        ++at;
    const x86::reg8 sign = byte(app, at);
    if (sign == '+' || sign == '-')
        ++at;
    x86::reg32 value = 0;
    for (; is(byte(app, at), kDigit); ++at)
        value = value * 10 + byte(app, at) - '0';
    return sign == '-' ? 0 - value : value;
}

x86::reg32 atoi(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, clib::atoi(app, args.eax));
    return args.eax;
}

x86::reg32 atol(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    atoi(app, cpu, args);
    return args.eax;
}

namespace
{

/* Where rand keeps its seed: 0 if the thread has no data.  False when the
 * game stopped. */
bool seedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& seed)
{
    if (!threadData(app, cpu, seed))
        return false;
    seed += kRandomSeed;
    return true;
}

}  // namespace

/* initrandnext (clib/math/c/rand.c, 0x4eb1f0): where rand keeps its seed. */
x86::reg32 initrandnext(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 seed;
    if (seedAt(app, cpu, seed))
        returns(cpu, args, seed);
    return args.eax;
}

/* rand (clib/math/c/rand.c, 0x4eb1fc): the seed taken on, x 1103515245 +
 * 12345, and bits 16 to 30 of it. */
/* rand's work: the next number into `result`; false when the game stopped. */
bool randOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& result)
{
    x86::reg32 seed;
    if (!seedAt(app, cpu, seed))
        return false;
    if (seed == 0)
        return result = 0, true;
    const x86::reg32 next = at(app, seed) * 0x41c64e6d + 0x3039;
    put(app, seed, next);
    result = next >> 16 & 0x7fff;
    return true;
}

x86::reg32 rand(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 result;
    if (!randOf(app, cpu, result))
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* srand (clib/math/c/rand.c, 0x4eb220): eax the seed. */
bool srandOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value, x86::reg32& seed)
{
    if (!seedAt(app, cpu, seed))
        return false;
    if (seed != 0)
        put(app, seed, value);
    return true;
}

x86::reg32 srand(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 seed;
    if (srandOf(app, cpu, args.eax, seed))
        returns(cpu, args, seed);
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
bool compare(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 function, x86::reg32 a,
             x86::reg32 b, x86::sreg32& result)
{
    cpu.esp = esp;
    const auto r = through(app, cpu, function, {a, b});
    if (!r)
        return false;
    result = x86::sreg32(*r);
    return true;
}

/* byteswap's exchange of `size` bytes at p and q: a dword at a time, then a
 * byte -- q read, then p, p written, then q, as its xchg and stos do; what
 * it leaves in eax, the last of p's it moved, given `eax` before. */
x86::reg32 exchange(win32::WinApplication* app, x86::reg32 p, x86::reg32 q, x86::reg32 size, x86::reg32 eax)
{
    for (x86::reg32 i = size >> 2; i != 0; --i, p += 4, q += 4)
    {
        const x86::reg32 fromQ = at(app, q);
        eax = at(app, p);
        put(app, p, fromQ);
        put(app, q, eax);
    }
    for (x86::reg32 i = size & 3; i != 0; --i, ++p, ++q)
    {
        const x86::reg8 fromQ = byte(app, q);
        eax = (eax & 0xffffff00) | byte(app, p);
        putByte(app, p, fromQ);
        putByte(app, q, x86::reg8(eax));
    }
    return eax;
}

/* med3's choice of a, b and c, the median by the caller's function, for a
 * med3 whose return address is at `entry`: its five pushes below that. */
bool median(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 function, x86::reg32 a,
            x86::reg32 b, x86::reg32 c, x86::reg32& chosen)
{
    const x86::reg32 esp = entry - 0x14;  // esi, edi, es, fs, gs
    x86::sreg32 r;
    if (!compare(app, cpu, esp, function, a, b, r))
        return false;
    const bool aAbove = r > 0;
    if (!compare(app, cpu, esp, function, a, c, r))
        return false;
    if (aAbove ? r <= 0 : r >= 0)
    {
        chosen = a;
        return true;
    }
    if (!compare(app, cpu, esp, function, b, c, r))
        return false;
    chosen = (r <= 0) == aAbove ? c : b;
    return true;
}

}  // namespace

/* med3 (clib/search/c/qsort.c, 0x4ee480): the median of the elements at eax,
 * edx and ebx by the function at ecx. */
x86::reg32 med3(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 chosen;
    if (!median(app, cpu, entry, args.ecx, args.eax, args.edx, args.ebx, chosen))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, chosen);
    return args.eax;
}

/* byteswap (clib/search/c/qsortrtn.c, 0x4ee4d0): ecx bytes at esi and edi
 * exchanged -- its own convention, esi and edi left past them, ecx and edx
 * 0. */
x86::reg32 byteswap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = exchange(app, args.esi, args.edi, args.ecx, args.eax);
    args.esi += args.ecx;
    args.edi += args.ecx;
    args.ecx = 0;
    args.edx = 0;
    cpu.esp += 4;
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
bool qsortOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 base, x86::reg32 n,
             const x86::reg32 size, const x86::reg32 function)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 frame = entry - 0x168;  // esi, edi, es, fs, gs, ebp, and 0x150 of its own
    const x86::reg32 pivotCopy = frame + 0x100;
    const int swaptype = ((base | size) & 3) != 0 ? 2 : size > 4 ? 1 : 0;
    auto swap = [&](x86::reg32 p, x86::reg32 q) {
        if (swaptype != 0)
        {
            exchange(app, p, q, size, 0);
        }
        else
        {
            const x86::reg32 t = at(app, p);
            put(app, p, at(app, q));
            put(app, q, t);
        }
    };
    x86::reg32 laterBase[32], laterCount[32];  // the larger parts, for later: the runtime's 32
    unsigned later = 0;
    x86::sreg32 r;
    for (;;)
    {
        if (n > 1 && n < 16)
        {
            const x86::reg32 end = base + n * size;
            for (x86::reg32 shell = 3 * size; x86::sreg32(shell) > 0; shell -= 2 * size)
                for (x86::reg32 p1 = base + shell; p1 < end; p1 += shell)
                    for (x86::reg32 p2 = p1; p2 > base; p2 -= shell)
                    {
                        if (!compare(app, cpu, frame, function, p2 - shell, p2, r))
                            return false;
                        if (r <= 0)
                            break;
                        swap(p2, p2 - shell);
                    }
        }
        else if (n >= 16)
        {
            x86::reg32 mid = base + (n >> 1) * size;
            if (n > 29)
            {
                x86::reg32 p1 = base;
                x86::reg32 p2 = base + (n - 1) * size;
                if (n > 42)
                {
                    const x86::reg32 s = (n >> 3) * size;
                    if (!median(app, cpu, frame - 4, function, p1, p1 + s, p1 + 2 * s, p1)
                        || !median(app, cpu, frame - 4, function, mid - s, mid, mid + s, mid)
                        || !median(app, cpu, frame - 4, function, p2 - 2 * s, p2 - s, p2, p2))
                        return false;
                }
                if (!median(app, cpu, frame - 4, function, p1, mid, p2, mid))
                    return false;
            }
            x86::reg32 pivot = base;
            if (swaptype != 0)
            {
                swap(base, mid);
            }
            else
            {
                pivot = pivotCopy;
                put(app, pivotCopy, at(app, mid));
            }
            x86::reg32 pa = base, pb = base;
            x86::reg32 pc = base + (n - 1) * size, pd = pc;
            x86::reg32 count = n;
            for (;;)
            {
                for (; count != 0; pb += size, --count)
                {
                    if (!compare(app, cpu, frame, function, pb, pivot, r))
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
                    if (!compare(app, cpu, frame, function, pc, pivot, r))
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
            const x86::reg32 pn = base + n * size;
            x86::reg32 moved = x86::sreg32(pa - base) < x86::sreg32(pb - pa) ? pa - base : pb - pa;
            if (moved != 0)
                exchange(app, base, pb - moved, moved, 0);
            moved = std::min(pd - pc, pn - pd - size);
            if (moved != 0)
                exchange(app, pb, pn - moved, moved, 0);
            // The smaller part now, the larger later.
            const x86::reg32 below = pb - pa, above = pd - pc;
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

x86::reg32 qsort(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    if (!qsortOf(app, cpu, args.eax, args.edx, args.ebx, args.ecx))
        return args.eax;
    kept.back(args);
    cpu.esp = entry + 4;
    return args.eax;
}

/* bsearch (clib/search/c/bsearch.c, 0x4fb300): the element of edx's ebx, ecx
 * bytes each, that the function on the stack finds equal to the key at eax,
 * halving from both ends; or 0. */
bool bsearchOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 key, x86::reg32 base,
               x86::reg32 count, x86::reg32 size, x86::reg32 function, x86::reg32& found)
{
    const x86::reg32 frame = entry - 0x1c;  // esi, edi, es, fs, gs, ebp, and the function
    found = 0;
    if (count != 0)
    {
        x86::reg32 low = base, high = base + (count - 1) * size;
        x86::sreg32 r = 1;
        while (low < high)
        {
            const x86::reg32 mid = low + (high - low) / size / 2 * size;
            if (!compare(app, cpu, frame, function, key, mid, r))
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
            if (!compare(app, cpu, frame, function, key, high, r))
                return false;
            found = r == 0 ? high : 0;
        }
    }
    return true;
}

x86::reg32 bsearch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 found;
    if (!bsearchOf(app, cpu, entry, args.eax, args.edx, args.ebx, args.ecx, at(app, entry + 4), found))
        return args.eax;
    kept.back(args);
    args.eax = found;
    cpu.esp = entry + 8;
    return args.eax;
}

/* ---- errno (clib/startup/c/seterrno.c, doserrno.c; clib/_dos/c/dosret.c):
 * the thread's, errno at +4 and _doserrno at +8.  Those that set it leave
 * eax with the thread's data, as theirs do; those that turn a Windows error
 * into it, -1. ---- */

namespace
{

constexpr x86::reg32 kErrno = 4, kDosErrno = 8;  // in the thread's data
/* __set_errno_dos's errno for each of Windows' errors up to 19, a signed
 * byte each; the rest take 19's. */
constexpr x86::reg32 kErrnoOfError = 0x567888;
/* Watcom's errno values. */
constexpr x86::reg32 kENOENT = 1, kENOMEM = 5, kEEXIST = 7, kEINVAL = 9, kERANGE = 14;

/* The Windows functions they call: their slots. */
constexpr x86::reg32 kCreateDirectoryA = 0x534490, kDeleteFileA = 0x5344b0, kFindFirstFileA = 0x5344cc;
constexpr x86::reg32 kGetCurrentDirectoryA = 0x534500, kGetDiskFreeSpaceA = 0x534514;
constexpr x86::reg32 kGetFileAttributesA = 0x534520, kGetLastError = 0x534534, kRemoveDirectoryA = 0x5345b4;
constexpr x86::reg32 kSetCurrentDirectoryA = 0x5345d4, kSetFileAttributesA = 0x5345ec;

/* A word of the thread's data set -- the data in `data`, if asked, which its code leaves in eax;
 * false when the game stopped. */
bool setThreadWord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 offset, x86::reg32 value,
                   x86::reg32* data = nullptr)
{
    x86::reg32 at;
    if (!threadData(app, cpu, at))
        return false;
    put(app, at + offset, value);
    if (data != nullptr)
        *data = at;
    return true;
}

/* __set_errno_dos's: _doserrno the error, errno what it stands for. */
bool setErrnoDos(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 error)
{
    if (!setThreadWord(app, cpu, kDosErrno, error))
        return false;
    x86::reg32 value;
    if (error == 0x7b)  // ERROR_INVALID_NAME
        value = kENOENT;
    else if (error == 0xce)  // ERROR_FILENAME_EXCED_RANGE
        value = kEINVAL;
    else if (error == 0xb7)  // ERROR_ALREADY_EXISTS
        value = kEEXIST;
    else
        value = x86::reg32(x86::sreg32(x86::sreg8(byte(app, kErrnoOfError + std::min<x86::reg32>(error, 0x13)))));
    return setThreadWord(app, cpu, kErrno, value);
}

/* __set_errno_nt's: GetLastError's error, as __set_errno_dos has it. */
bool setErrnoNt(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto error = windows<kGetLastError, &host::lastError>(app, cpu);
    return error && setErrnoDos(app, cpu, *error);
}

/* What the runtime makes of a Windows function's BOOL: 0, or errno from
 * GetLastError and -1. */
bool windowsResult(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 succeeded, x86::reg32& result)
{
    result = succeeded != 0 ? 0 : 0xffffffff;
    return succeeded != 0 || setErrnoNt(app, cpu);
}

}  // namespace

/* __set_errno (0x502880): errno eax. */
x86::reg32 set_errno(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, cpu, kErrno, args.eax))
        return args.eax;
    const x86::reg32 data = args.eax;
    kept.back(args);
    returns(cpu, args, data);
    return args.eax;
}

/* __set_ERANGE (0x5028a4): errno ERANGE. */
x86::reg32 set_ERANGE(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, cpu, kErrno, kERANGE))
        return args.eax;
    const x86::reg32 data = args.eax;
    kept.back(args);
    returns(cpu, args, data);
    return args.eax;
}

/* __set_doserrno (0x5028c8): _doserrno eax. */
x86::reg32 set_doserrno(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, cpu, kDosErrno, args.eax))
        return args.eax;
    const x86::reg32 data = args.eax;
    kept.back(args);
    returns(cpu, args, data);
    return args.eax;
}

/* __set_errno_dos (0x500f8c): errno for the Windows error in eax; -1. */
x86::reg32 set_errno_dos(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setErrnoDos(app, cpu, args.eax))
        return args.eax;
    kept.back(args);
    returns(cpu, args, 0xffffffff);
    return args.eax;
}

/* __set_errno_nt (0x500ff4): errno for GetLastError's; -1. */
x86::reg32 set_errno_nt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setErrnoNt(app, cpu))
        return args.eax;
    kept.back(args);
    returns(cpu, args, 0xffffffff);
    return args.eax;
}

/* ---- Files and directories: each a Windows function on the path at eax,
 * 0 or -1 and errno.  Those that hand Windows a buffer of their own have it
 * where their frame has it, esp below it. ---- */

namespace
{

/* A function of one call to Windows on the path, BOOL its result. */
template <x86::reg32 Slot, auto Function, typename... Words>
void pathCall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, Words... words)
{
    const Kept kept(args);
    x86::reg32 result;
    const auto succeeded = windows<Slot, Function>(app, cpu, words...);
    if (!succeeded || !windowsResult(app, cpu, *succeeded, result))
        return;
    kept.back(args);
    returns(cpu, args, result);
}

/* __fixed_GetFileAttributesA's: the file's attributes by FindFirstFileA --
 * the handle left open, as it leaves it -- or -1, for a path with * or ? in
 * it too; for a call whose return address is at `entry`. */
bool fileAttributes(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 path,
                    x86::reg32& attributes)
{
    attributes = 0xffffffff;
    for (x86::reg32 c = path; byte(app, c) != 0; ++c)
        if (byte(app, c) == '*' || byte(app, c) == '?')
            return true;
    const x86::reg32 found = entry - 0x150;  // ebx, ecx, edx, esi; WIN32_FIND_DATA
    cpu.esp = found;
    const auto handle = windows<kFindFirstFileA, &host::findFirstFile>(app, cpu, path, found);
    if (!handle)
        return false;
    if (*handle != 0xffffffff)
        attributes = at(app, found);
    return true;
}

}  // namespace

namespace
{

/* unlink's: DeleteFileA on `path`; 0, or -1 and errno. */
bool unlinkOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32& result)
{
    const auto deleted = windows<kDeleteFileA, &host::deleteFile>(app, cpu, path);
    return deleted && windowsResult(app, cpu, *deleted, result);
}

}  // namespace

/* unlink (clib/handleio/c/unlnkwnt.c, 0x4e28a0): DeleteFileA. */
x86::reg32 unlink(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    pathCall<kDeleteFileA, &host::deleteFile>(app, cpu, args, args.eax);
    return args.eax;
}

/* remove (clib/file/c/remove.c, 0x4ee2e0): unlink. */
x86::reg32 remove(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    unlink(app, cpu, args);
    return args.eax;
}

/* mkdir (clib/direct/c/mkdirwnt.c, 0x4eb1d0): CreateDirectoryA, no security
 * attributes. */
x86::reg32 mkdir(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    pathCall<kCreateDirectoryA, &host::createDirectory>(app, cpu, args, args.eax, 0);
    return args.eax;
}

/* rmdir (clib/direct/c/rmdirwnt.c, 0x4ee400): RemoveDirectoryA. */
x86::reg32 rmdir(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    pathCall<kRemoveDirectoryA, &host::removeDirectory>(app, cpu, args, args.eax);
    return args.eax;
}

/* chdir (clib/direct/c/chdirwnt.c, 0x4f0260): SetCurrentDirectoryA. */
x86::reg32 chdir(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    pathCall<kSetCurrentDirectoryA, &host::setCurrentDirectory>(app, cpu, args, args.eax);
    return args.eax;
}

/* chmod (clib/file/c/chmodwnt.c, 0x4ee3c0): the file read-only unless edx
 * has S_IWRITE (0x80), its other attributes as __fixed_GetFileAttributesA
 * finds them. */
x86::reg32 chmod(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    const x86::reg32 path = args.eax, mode = args.edx;
    x86::reg32 attributes, result;
    if (!fileAttributes(app, cpu, entry - 0xc, path, attributes))  // ebx, ecx, and the call
        return args.eax;
    cpu.esp = entry;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return args.eax;
    }
    else
    {
        attributes = (attributes & ~x86::reg32(1)) | ((mode & 0x80) != 0 ? 0 : 1);  // FILE_ATTRIBUTE_READONLY
        if (!windows<kSetFileAttributesA, &host::setFileAttributes>(app, cpu, path, attributes)
            || !windowsResult(app, cpu, args.eax, result))
            return args.eax;
    }
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* access (clib/file/c/accsswnt.c, 0x4f05f0): 0 if the file is there, and
 * not read-only if edx asks for writing (2). */
x86::reg32 access(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    const x86::reg32 mode = args.edx;
    if (!windows<kGetFileAttributesA, &host::fileAttributes>(app, cpu, args.eax))
        return args.eax;
    const x86::reg32 attributes = args.eax;
    x86::reg32 result = 0;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return args.eax;
    }
    else if ((mode & 2) != 0 && (attributes & 1) != 0)
    {
        result = 0xffffffff;
        if (!setErrnoDos(app, cpu, 5))  // ERROR_ACCESS_DENIED
            return args.eax;
    }
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

x86::reg32 nmalloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nfree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sopen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

/* getcwd (clib/direct/c/gtcwdwnt.c, 0x4f0730): the current directory into
 * eax's edx bytes -- or, for no buffer, into a block _nmalloc gives of edx
 * bytes or as many as it takes -- or 0: errno ERANGE when it does not fit,
 * ENOMEM when there is no block.  What it copies is as long as
 * GetCurrentDirectoryA says, from its MAX_PATH bytes and on into the
 * registers it pushed -- here too. */
std::optional<x86::reg32> getcwdOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size)
{
    const x86::reg32 entry = cpu.esp;
    // The four registers the code pushes (their values written there are nobody's to read): their room.
    const x86::reg32 local = entry - 0x114;  // MAX_PATH bytes below them
    cpu.esp = local;
    const auto got = windows<kGetCurrentDirectoryA, &host::currentDirectory>(app, cpu, 0x104, local);
    if (!got)
        return std::nullopt;
    const x86::reg32 length = *got;
    x86::reg32 result = 0;
    if (length == 0)
    {
        if (!setErrnoNt(app, cpu))
            return std::nullopt;
    }
    else
    {
        bool fits = true;
        if (buffer == 0)
        {
            buffer = clib::malloc(app, cpu, std::max(size, length + 1));
            if (cpu.terminate)
                return std::nullopt;
            if (buffer == 0)
            {
                fits = false;
                if (!setThreadWord(app, cpu, kErrno, kENOMEM))
                    return std::nullopt;
            }
        }
        else if (length > size)
        {
            fits = false;
            if (!setThreadWord(app, cpu, kErrno, kERANGE))
                return std::nullopt;
        }
        if (fits)
        {
            for (x86::reg32 i = 0; i < length + 1; ++i)
                putByte(app, buffer + i, byte(app, local + i));
            result = buffer;
        }
    }
    cpu.esp = entry;
    return result;
}

x86::reg32 getcwd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (const auto result = getcwdOf(app, cpu, args.eax, args.edx))
        returns(cpu, args, *result);
    return args.eax;
}

/* _getdiskfree (clib/direct/c/diskfree.c, 0x4f07d0): GetDiskFreeSpaceA's of
 * drive eax -- 1 A:, 0 the current one -- into edx's diskfree_t: total and
 * free clusters, sectors a cluster, bytes a sector; 0, or -1. */
std::optional<x86::reg32> getdiskfreeOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 drive, x86::reg32 info)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 local = entry - 0x1c;  // ebx, ecx; the root and four dwords
    x86::reg32 root = 0;
    if (drive != 0)
    {
        putByte(app, local, x86::reg8(drive + 0x40));
        putByte(app, local + 1, ':');
        putByte(app, local + 2, '\\');
        putByte(app, local + 3, 0);
        root = local;
    }
    cpu.esp = local;
    x86::reg32 result;
    const auto freeSpace =
        windows<kGetDiskFreeSpaceA, &host::diskFreeSpace>(app, cpu, root, local + 0x10, local + 0xc, local + 8, local + 4);
    if (!freeSpace || !windowsResult(app, cpu, *freeSpace, result))
        return std::nullopt;
    if (result == 0)
    {
        put(app, info, at(app, local + 4));
        put(app, info + 4, at(app, local + 8));
        put(app, info + 8, at(app, local + 0x10));
        put(app, info + 0xc, at(app, local + 0xc));
    }
    cpu.esp = entry;
    return result;
}

x86::reg32 getdiskfree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (const auto result = getdiskfreeOf(app, cpu, args.eax, args.edx))
        returns(cpu, args, *result);
    return args.eax;
}

/* __fixed_GetFileAttributesA (0x509640): GetFileAttributesA by way of
 * FindFirstFileA, which wildcards would fool: -1 for them. */
x86::reg32 fixed_GetFileAttributesA(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 attributes;
    if (!fileAttributes(app, cpu, entry, args.eax, attributes))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, attributes);
    return args.eax;
}

/* ---- The runtime's handles (clib/handleio/c): a handle is an index into
 * __OSHandles, Windows' HANDLE for it, and __io_mode, its mode; each taken
 * and given back under the file's lock, _AccessFileH and _ReleaseFileH,
 * __NullAccessRtn while there is one thread.  A handle up to __NFiles is
 * good -- __NFiles too, the runtime's own slip, kept. ---- */

namespace
{

constexpr x86::reg32 kOsHandles = 0x56acbc;  // __OSHandles: where the HANDLEs are
constexpr x86::reg32 kFileCount = 0x56ad94;  // __NFiles
constexpr x86::reg32 kIoModes = 0x56ade8;    // __io_mode: where the modes are
constexpr x86::reg32 kAccessFile = 0x567768, kReleaseFile = 0x56776c;  // the file's lock, eax the handle
/* The default windowing's hooks: none in the game. */
constexpr x86::reg32 kIsWindowedHandle = 0x5677b0, kWindowsStdin = 0x5677d8, kWindowsStdout = 0x5677dc;
constexpr x86::reg32 kGetFileType = 0x53452c, kReadFile = 0x5345b0, kSetFilePointer = 0x5345f0;
constexpr x86::reg32 kWriteFile = 0x534640;
constexpr x86::reg32 kEBADF = 4, kENOSPC = 12;
/* A mode's bits. */
constexpr x86::reg32 kAppend = 0x80, kIsTty = 0x2000, kInitialized = 0x4000, kExtended = 0x8000;

bool goodHandle(win32::WinApplication* app, x86::reg32 handle)
{
    return x86::sreg32(handle) >= 0 && handle <= at(app, kFileCount);
}

x86::reg32 osHandle(win32::WinApplication* app, x86::reg32 handle)
{
    return at(app, at(app, kOsHandles) + 4 * handle);
}

/* The file's lock taken (kAccessFile) or given back (kReleaseFile), by its
 * hook (watcom_hooks.h). */
bool fileLock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 handle)
{
    return (slot == kAccessFile ? hook<kAccessFile>(app, cpu, {handle}) : hook<kReleaseFile>(app, cpu, {handle}))
        .has_value();
}

/* The default windowing's window for the handle, if it has one and `hook`
 * is set: _WindowsIsWindowedHandle's. */
bool windowOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 windowing, x86::reg32 handle,
              x86::reg32& window)
{
    window = 0;
    if (at(app, windowing) == 0)
        return true;
    const auto found = through(app, cpu, at(app, kIsWindowedHandle), {handle});
    if (!found)
        return false;
    window = *found;
    return true;
}

/* isatty's: 1 for a window's handle or a character device's, under the lock. */
bool isTty(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& result)
{
    x86::reg32 window;
    if (!fileLock(app, cpu, kAccessFile, handle) || !windowOf(app, cpu, kIsWindowedHandle, handle, window))
        return false;
    result = 1;
    if (window == 0)
    {
        const auto type = windows<kGetFileType, &host::fileType>(app, cpu, osHandle(app, handle));
        if (!type)
            return false;
        result = *type == 2 ? 1 : 0;  // FILE_TYPE_CHAR
    }
    return fileLock(app, cpu, kReleaseFile, handle);
}

/* __GetIOMode's: 0 past __NFiles; for the first three, isatty asked the
 * first time and the answer kept in the mode. */
bool ioMode(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& mode)
{
    mode = 0;
    if (handle >= at(app, kFileCount))
        return true;
    if (x86::sreg32(handle) < 3 && !(byte(app, at(app, kIoModes) + 4 * handle + 1) & (kInitialized >> 8)))
    {
        const x86::reg32 flags = at(app, kIoModes) + 4 * handle + 1;
        putByte(app, flags, byte(app, flags) | (kInitialized >> 8));
        x86::reg32 tty;
        if (!isTty(app, cpu, handle, tty))
            return false;
        if (tty != 0)
        {
            const x86::reg32 again = at(app, kIoModes) + 4 * handle + 1;
            putByte(app, again, byte(app, again) | (kIsTty >> 8));
        }
    }
    mode = at(app, at(app, kIoModes) + 4 * handle);
    return true;
}

/* __SetIOMode's: a mode set, initialized unless 0. */
void setIoMode(win32::WinApplication* app, x86::reg32 handle, x86::reg32 mode)
{
    put(app, at(app, kIoModes) + 4 * handle, mode != 0 ? mode | kInitialized : 0);
}

/* What a handle's function makes of a bad one: errno EBADF, -1. */
bool badHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& result)
{
    result = 0xffffffff;
    return setThreadWord(app, cpu, kErrno, kEBADF);
}

}  // namespace

/* __GetIOMode (clib/handleio/c/iomode.c, 0x522de0): eax's mode. */
x86::reg32 GetIOMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 mode;
    if (!ioMode(app, cpu, args.eax, mode))
        return args.eax;
    kept.back(args);
    returns(cpu, args, mode);
    return args.eax;
}

/* __SetIOMode (clib/handleio/c/iomode.c, 0x522e38): eax's mode edx; eax
 * left 4 times the handle, as its code leaves it. */
x86::reg32 SetIOMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    setIoMode(app, args.eax, args.edx);
    returns(cpu, args, args.eax << 2);
    return args.eax;
}

/* isatty (clib/environ/c/isattwnt.c, 0x523560): whether eax is a
 * character device's, or a window's. */
x86::reg32 isatty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!isTty(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

namespace
{

/* lseek's: SetFilePointer on `handle`, `offset` bytes from where `whence` says; a move forward
 * marks the mode extended unless it is appending.  The new place, or -1. */
bool lseekOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 offset, x86::reg32 whence,
             x86::reg32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    x86::reg32 mode;
    if (!fileLock(app, cpu, kAccessFile, handle) || !ioMode(app, cpu, handle, mode))
        return false;
    if (x86::sreg32(offset) > 0 && !(mode & kAppend))
        setIoMode(app, handle, mode | kExtended);
    const auto place =
        windows<kSetFilePointer, &host::setFilePointer>(app, cpu, osHandle(app, handle), offset, 0, whence);
    if (!place)
        return false;
    result = *place;
    return fileLock(app, cpu, kReleaseFile, handle) && (result != 0xffffffff || setErrnoNt(app, cpu));
}

/* _tell's: `handle`'s place, or -1. */
bool tellOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    const x86::reg32 os = osHandle(app, handle);  // before the lock, as its code reads it
    if (!fileLock(app, cpu, kAccessFile, handle))
        return false;
    const auto place = windows<kSetFilePointer, &host::setFilePointer>(app, cpu, os, 0, 0, 1);
    if (!place)
        return false;
    result = *place;
    return fileLock(app, cpu, kReleaseFile, handle) && (result != 0xffffffff || setErrnoNt(app, cpu));
}

}  // namespace

/* lseek (clib/handleio/c/lseek.c, 0x5028e0): SetFilePointer on eax, edx bytes
 * from where ebx says; a move forward marks the mode extended unless it is
 * appending.  The new place, or -1. */
x86::reg32 lseek(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!lseekOf(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* _tell (clib/handleio/c/tell.c, 0x502960): eax's place, or -1. */
x86::reg32 tell(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!tellOf(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

namespace
{

/* __qread's and __qwrite's way to a window's: the hook on it, eax the window,
 * edx the buffer, ebx the length; how many it took. */
bool windowTransfer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 windowing, x86::reg32 handle,
                    x86::reg32 window, x86::reg32 buffer, x86::reg32 length, x86::reg32& result)
{
    const auto took = through(app, cpu, at(app, windowing), {window, buffer, length});
    if (!took)
        return false;
    result = *took;
    return fileLock(app, cpu, kReleaseFile, handle);
}

bool readHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer, x86::reg32 length,
          x86::reg32 local, x86::reg32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    x86::reg32 window;
    if (!fileLock(app, cpu, kAccessFile, handle) || !windowOf(app, cpu, kWindowsStdin, handle, window))
        return false;
    if (window != 0)
        return windowTransfer(app, cpu, kWindowsStdin, handle, window, buffer, length, result);
    const auto read = windows<kReadFile, &host::readFile>(app, cpu, osHandle(app, handle), buffer, length, local, 0);
    if (!read)
        return false;
    const bool done = *read != 0;
    result = done ? at(app, local) : 0xffffffff;
    return fileLock(app, cpu, kReleaseFile, handle) && (done || setErrnoNt(app, cpu));
}

bool writeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer, x86::reg32 length,
           x86::reg32 local, x86::reg32& result)
{
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    const x86::reg32 os = osHandle(app, handle);  // before the lock, as its code reads it
    x86::reg32 mode, window;
    if (!fileLock(app, cpu, kAccessFile, handle) || !ioMode(app, cpu, handle, mode))
        return false;
    if (mode & kAppend)
    {
        const auto place = windows<kSetFilePointer, &host::setFilePointer>(app, cpu, os, 0, 0, 2);
        if (!place)
            return false;
        if (*place == 0xffffffff)
        {
            result = 0xffffffff;
            return fileLock(app, cpu, kReleaseFile, handle) && setErrnoNt(app, cpu);
        }
    }
    if (!windowOf(app, cpu, kWindowsStdout, handle, window))
        return false;
    if (window != 0)
        return windowTransfer(app, cpu, kWindowsStdout, handle, window, buffer, length, result);
    const auto written = windows<kWriteFile, &host::writeFile>(app, cpu, os, buffer, length, local, 0);
    if (!written)
        return false;
    if (*written == 0)
    {
        result = 0xffffffff;
        return fileLock(app, cpu, kReleaseFile, handle) && setErrnoNt(app, cpu);
    }
    result = at(app, local);
    return (result == length || setThreadWord(app, cpu, kErrno, kENOSPC))
           && fileLock(app, cpu, kReleaseFile, handle);
}

/* __qread's and __qwrite's, at the depth of their calls: their frames under the return address --
 * ecx and esi, or ecx, esi, edi and ebp (their room only), then the dword Windows is handed for how
 * many it moved. */
bool qreadOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer, x86::reg32 length,
             x86::reg32& result)
{
    const x86::reg32 entry = cpu.esp;
    const GuestFrame frame(cpu, 0xc, 0);
    return readHandle(app, cpu, handle, buffer, length, entry - 0xc, result);
}

bool qwriteOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 buffer, x86::reg32 length,
              x86::reg32& result)
{
    const x86::reg32 entry = cpu.esp;
    const GuestFrame frame(cpu, 0x14, 0);
    return writeHandle(app, cpu, handle, buffer, length, entry - 0x14, result);
}

}  // namespace

/* __qread (clib/handleio/c/qread.c, 0x502a50): ebx bytes from eax into edx --
 * a window's by _WindowsStdin, else by ReadFile; how many, or -1. */
x86::reg32 qread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!qreadOf(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __qwrite (clib/handleio/c/qwrit.c, 0x5096c0): ebx bytes from edx to eax --
 * at the end, if it is appending; a window's by _WindowsStdout, else by
 * WriteFile, errno ENOSPC for fewer.  How many, or -1. */
x86::reg32 qwrite(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!qwriteOf(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* ---- Time (clib/time/c): a struct tm in guest memory is nine ints -- sec,
 * min, hour, mday, mon, year since 1900, wday, yday, isdst.  The time zone
 * is the runtime's, __tzset's: still the game's code, called where the
 * runtime calls it.  The structs these keep on their stacks are where their
 * frames have them. ---- */

namespace
{

constexpr x86::reg32 kTzset = 0x515c2c;        // __tzset: the zone from TZ, or from Windows
constexpr x86::reg32 kTimezone = 0x568b94;     // timezone: seconds west of UTC
constexpr x86::reg32 kDstAdjust = 0x568b9c;    // __dst_adjust: seconds daylight saving takes off
constexpr x86::reg32 kDstName = 0x568b90;      // tzname[1]: no daylight saving while it is ""
constexpr x86::reg32 kDstStart = 0x568a40, kDstEnd = 0x568a64;  // its rules, a struct tm each
constexpr x86::reg32 kDaysInYear = 0x552148, kDaysInLeapYear = 0x552162;  // __diyr, __dilyr: shorts
constexpr x86::reg32 kGetLocalTime = 0x534538;
constexpr x86::reg32 kDaysTo1970 = 0x63df;     // from 1 January 1900
constexpr x86::reg32 kDay = 86400;
enum TmField : x86::reg32
{
    kSec = 0, kMin = 4, kHour = 8, kMday = 0xc, kMon = 0x10, kYear = 0x14, kWday = 0x18, kYday = 0x1c,
    kIsdst = 0x20,
};

x86::sreg32 field(win32::WinApplication* app, x86::reg32 tm, TmField f)
{
    return x86::sreg32(at(app, tm + f));
}

/* __diyr's or __dilyr's entry, a short; the index as the runtime's code
 * scales it. */
x86::sreg32 daysBefore(win32::WinApplication* app, x86::reg32 table, x86::reg32 month)
{
    return x86::sreg16(app->getMemory<x86::reg16>(table + 2 * month));
}

/* __leapyear (clib/time/c/leapyear.c). */
bool leapYear(x86::reg32 year)
{
    if (year & 3)
        return false;
    return year % 100 != 0 || year % 400 == 0;
}

x86::reg32 yearTable(x86::reg32 year)
{
    return leapYear(year + 1900) ? kDaysInLeapYear : kDaysInYear;
}

/* __brktime (clib/time/c/gmtime.c): `seconds` seconds `zone` west into day
 * `days` broken down into the struct tm, all but isdst.  Its years are
 * found by 366 a year, then a year's length at a time -- the first year's
 * length each time, as its code does. */
x86::reg32 breakTime(win32::WinApplication* app, x86::reg32 days, x86::reg32 seconds, x86::reg32 zone, x86::reg32 tm)
{
    if (seconds < 0xa8c0 && x86::sreg32(zone) > 0)
    {
        seconds = seconds + kDay - zone;
        days += seconds / kDay - 1;
    }
    else
    {
        seconds -= zone;
        days += seconds / kDay;
    }
    x86::reg32 rest = seconds % kDay;
    put(app, tm + kHour, rest / 3600);
    rest %= 3600;
    put(app, tm + kMin, rest / 60);
    put(app, tm + kSec, rest % 60);
    x86::reg32 year = days / 366;
    x86::reg32 day = days - 365 * year;
    if (year != 0)
        day -= (year - 1) >> 2;
    const x86::reg32 length = (leapYear(year + 1900) ? 1 : 0) + 365;
    while (day >= length)
    {
        day -= length;
        ++year;
    }
    put(app, tm + kYear, year);
    put(app, tm + kYday, day);
    const x86::reg32 table = yearTable(year);
    x86::reg32 month = day / 31;
    if (day >= x86::reg32(daysBefore(app, table, month + 1)))
        ++month;
    put(app, tm + kMon, month);
    put(app, tm + kMday, day - daysBefore(app, table, month) + 1);
    put(app, tm + kWday, (days + 1) % 7);
    return tm;
}

/* time_less (clib/time/c/timeutil.c): whether a's time of day is before b's. */
bool timeLess(win32::WinApplication* app, x86::reg32 a, x86::reg32 b)
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
bool tzsetCalled(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.esp = esp - 4;
    x86::Args registers;
    tzset(app, cpu, registers);
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    return true;
}

bool makeTime(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 tm, x86::reg32& result);

/* calc_yday (clib/time/c/timeutil.c): the day of `year` a daylight saving
 * rule falls on -- Jn, n or Mm.w.d, as its isdst says; mktime asked for the
 * weekday the month begins on, with a struct tm of its frame. */
bool ruleDay(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 rule, x86::reg32 year,
             x86::sreg32& result)
{
    const x86::sreg32 kind = field(app, rule, kIsdst);
    if (kind != 0)
    {
        result = x86::sreg32(at(app, rule + kYday) - (kind == 1 ? 1 : 0));
        return true;
    }
    const x86::reg32 month = at(app, rule + kMon);
    const x86::reg32 table = yearTable(year);
    const x86::sreg32 monthDays = daysBefore(app, table, month + 1) - daysBefore(app, table, month);
    const x86::reg32 first = entry - 0x38;  // ebx, ecx, esi, edi, ebp; the struct tm
    put(app, first + kSec, 0);
    put(app, first + kMin, 0);
    put(app, first + kHour, 0);
    put(app, first + kMday, 1);
    put(app, first + kMon, month);
    put(app, first + kYear, year);
    put(app, first + kIsdst, 0);
    x86::reg32 ignored;
    if (!makeTime(app, cpu, first - 4, first, ignored))
        return false;
    // The rule's fields are anything: its sums wrap as the x86's do.
    const x86::reg32 firstWday = x86::reg32(x86::sreg32(at(app, rule + kWday) - at(app, first + kWday) + 7) % 7);
    const x86::reg32 week = at(app, rule + kMday);
    x86::reg32 nth = week - 1;
    if (week == 5 && x86::sreg32(firstWday + 1 + 28) > monthDays)
        nth = week - 2;
    result = x86::sreg32(7 * nth + at(app, first + kYday) + firstWday);
    return true;
}

/* check_order (clib/time/c/timeutil.c): whether daylight saving starts after
 * it ends in `year` -- the southern hemisphere. */
bool rulesReversed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 start, x86::reg32 end,
                   x86::reg32 year, bool& result)
{
    if (field(app, start, kIsdst) == 0 && field(app, end, kIsdst) == 0
        && field(app, start, kMon) != field(app, end, kMon))
    {
        result = field(app, start, kMon) > field(app, end, kMon);
        return true;
    }
    const x86::reg32 esp = entry - 0xc;  // ecx, esi, edi
    x86::sreg32 startDay, endDay;
    if (!ruleDay(app, cpu, esp - 4, start, year, startDay) || !ruleDay(app, cpu, esp - 4, end, year, endDay))
        return false;
    result = startDay > endDay;
    return true;
}

/* __isindst's test of one rule (clib/time/c/timeutil.c): whether the day is
 * past the rule's day, and whether it is that day, its time to be compared. */
void ruleDayPassed(win32::WinApplication* app, x86::reg32 tm, x86::reg32 rule, x86::sreg32 monthDays, bool start,
                   bool& dst, bool& timeCheck)
{
    timeCheck = false;
    if (field(app, rule, kIsdst) == 0)
    {
        const x86::sreg32 month = field(app, tm, kMon), ruleMonth = field(app, rule, kMon);
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
        const x86::reg32 mday = at(app, tm + kMday), wday = at(app, tm + kWday), ruleWday = at(app, rule + kWday);
        const x86::sreg32 n1 = x86::sreg32(mday - x86::reg32(x86::sreg32(wday + 7 - ruleWday) % 7));
        const x86::sreg32 n2 = x86::sreg32(mday - 1 - x86::reg32(x86::sreg32(wday + 6 - ruleWday) % 7));
        const x86::reg32 week = at(app, rule + kMday);
        const bool lastWeek = week == 5;
        const x86::sreg32 limit = lastWeek ? monthDays - 7 : x86::sreg32(7 * (week - 1) + 1);
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
    x86::reg32 n1 = at(app, rule + kYday);
    if (field(app, rule, kIsdst) == 1)
    {
        if (leapYear(at(app, tm + kYear) + 1900) && x86::sreg32(n1) > daysBefore(app, kDaysInYear, 2))
            ++n1;
        --n1;
    }
    if (field(app, tm, kYday) >= x86::sreg32(n1))
    {
        dst = start;
        timeCheck = at(app, tm + kYday) == n1;
    }
}

/* __isindst (clib/time/c/timeutil.c): whether the struct tm is in daylight
 * saving, by the zone's rules, into its isdst; none while tzname[1] is "". */
bool inDst(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 tm, x86::reg32& result)
{
    bool dst = false;
    if (byte(app, at(app, kDstName)) != 0)
    {
        bool south;
        if (!rulesReversed(app, cpu, entry - 0x34 - 4, kDstStart, kDstEnd, at(app, tm + kYear), south))
            return false;  // ebx, ecx, edx, esi, edi, ebp, and 0x1c of its own
        const x86::reg32 start = south ? kDstEnd : kDstStart, end = south ? kDstStart : kDstEnd;
        const x86::reg32 table = yearTable(at(app, tm + kYear));
        const x86::reg32 month = at(app, tm + kMon);
        const x86::sreg32 monthDays = daysBefore(app, table, month + 1) - daysBefore(app, table, month);
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
bool makeTime(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 tm, x86::reg32& result)
{
    // Its sums wrap as the x86's do: the fields are anything.
    const x86::reg32 esp = entry - 0x1c;  // ebx, ecx, edx, esi, edi, ebp, and a dword
    result = 0xffffffff;
    x86::sreg32 month = field(app, tm, kMon) % 12;
    if (field(app, tm, kYear) < x86::sreg32(0xf4fb7ea1))
        return true;
    x86::reg32 year = at(app, tm + kYear) + x86::reg32(field(app, tm, kMon) / 12);
    while (month < 0)
    {
        month += 12;
        --year;
    }
    if (x86::sreg32(year) < 0)
        return true;
    const x86::reg32 table = yearTable(year);
    x86::reg32 days = x86::reg32(x86::sreg32(year + 3) / 4) + 365 * year
                      - x86::reg32(x86::sreg32(year + 0x63) / 100) + x86::reg32(x86::sreg32(year + 0x12b) / 400);
    days = x86::reg32(daysBefore(app, table, month)) + days + at(app, tm + kMday) - 1;
    x86::reg32 seconds = at(app, tm + kSec) + 60 * (at(app, tm + kMin) + 60 * at(app, tm + kHour));
    while (x86::sreg32(seconds) < 0)
    {
        seconds += kDay;
        --days;
    }
    breakTime(app, days, seconds, 0, tm);
    cpu.esp = esp;
    if (!tzsetCalled(app, cpu))
        return false;
    seconds += at(app, kTimezone);
    if (field(app, tm, kIsdst) < 0)
    {
        x86::reg32 ignored;
        if (!inDst(app, cpu, esp - 4, tm, ignored))
            return false;
    }
    if (field(app, tm, kIsdst) > 0)
        seconds -= at(app, kDstAdjust);
    while (x86::sreg32(seconds) < 0)
    {
        --days;
        seconds += kDay;
    }
    if (x86::sreg32(days) < x86::sreg32(kDaysTo1970 - 1))
        return true;
    if (days == kDaysTo1970 - 1)
    {
        seconds -= kDay;
        if (x86::sreg32(at(app, kTimezone)) > 0 && x86::sreg32(seconds) >= 0)
            result = seconds;
        return true;
    }
    result = seconds + (days - kDaysTo1970) * kDay;
    return true;
}

/* __getctime (clib/time/c/gtctmwnt.c): GetLocalTime's into the struct tm,
 * isdst -1; its milliseconds. */
bool currentTime(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 tm, x86::reg32& result)
{
    const x86::reg32 local = entry - 0x1c;  // ebx, ecx, edx; a SYSTEMTIME
    cpu.esp = local;
    if (!windows<kGetLocalTime, &host::localTime>(app, cpu, local))
        return false;
    auto word16 = [app, local](x86::reg32 offset) { return x86::reg32(app->getMemory<x86::reg16>(local + offset)); };
    put(app, tm + kYear, word16(0) - 1900);
    put(app, tm + kMon, word16(2) - 1);
    put(app, tm + kMday, word16(6));
    put(app, tm + kHour, word16(8));
    put(app, tm + kMin, word16(0xa));
    put(app, tm + kIsdst, 0xffffffff);
    put(app, tm + kSec, word16(0xc));
    result = word16(0xe);
    return true;
}

/* _localtime's (clib/time/c/localtim.c): *timer broken down in the zone,
 * again with daylight saving's hour if it is in it. */
bool localTime(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry, x86::reg32 timer, x86::reg32 tm)
{
    cpu.esp = entry - 0x10;  // ebx, ecx, esi, edi
    if (!tzsetCalled(app, cpu))
        return false;
    const x86::reg32 seconds = at(app, timer);
    breakTime(app, kDaysTo1970, seconds, at(app, kTimezone), tm);
    x86::reg32 dst;
    if (!inDst(app, cpu, entry - 0x14, tm, dst))
        return false;
    if (dst != 0)
        breakTime(app, kDaysTo1970, seconds, at(app, kTimezone) - at(app, kDstAdjust), tm);
    return true;
}

}  // namespace

/* __leapyear (clib/time/c/leapyear.c, 0x5160f0): whether year eax is a leap
 * year. */
x86::reg32 leapyear(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    returns(cpu, args, leapYear(args.eax) ? 1 : 0);
    return args.eax;
}

/* __brktime (clib/time/c/gmtime.c, 0x515fa0): edx seconds ebx west into day
 * eax since 1900 broken down into the struct tm at ecx; that. */
x86::reg32 brktime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, breakTime(app, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* time_less (clib/time/c/timeutil.c, 0x51652c): whether the struct tm at
 * eax's time of day is before edx's. */
x86::reg32 time_less(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, timeLess(app, args.eax, args.edx) ? 1 : 0);
    return args.eax;
}

/* calc_yday (clib/time/c/timeutil.c, 0x51612c): the day of year edx the rule
 * at eax falls on. */
x86::reg32 calc_yday(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::sreg32 day;
    if (!ruleDay(app, cpu, entry, args.eax, args.edx, day))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, day);
    return args.eax;
}

/* check_order (clib/time/c/timeutil.c, 0x516224): whether the rule at eax
 * falls after edx's in year ebx. */
x86::reg32 check_order(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    bool reversed;
    if (!rulesReversed(app, cpu, entry, args.eax, args.edx, args.ebx, reversed))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, reversed ? 1 : 0);
    return args.eax;
}

/* __isindst (clib/time/c/timeutil.c, 0x516270): the struct tm at eax's isdst
 * by the zone's rules; that. */
x86::reg32 isindst(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 dst;
    if (!inDst(app, cpu, entry, args.eax, dst))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, dst);
    return args.eax;
}

/* mktime (clib/time/c/mktime.c, 0x50eba0): the struct tm at eax made a
 * time_t, and put right. */
x86::reg32 mktime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 result;
    if (!makeTime(app, cpu, entry, args.eax, result))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, result);
    return args.eax;
}

/* __getctime (clib/time/c/gtctmwnt.c, 0x50eb30): the local time into the
 * struct tm at eax; its milliseconds. */
x86::reg32 getctime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    x86::reg32 milliseconds;
    if (!currentTime(app, cpu, entry, args.eax, milliseconds))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, milliseconds);
    return args.eax;
}

/* time (clib/time/c/time.c, 0x4f22b0): now, a second on from half of one,
 * as a time_t -- into eax's too, if there is one. */
std::optional<x86::reg32> timeOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 timer)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 tm = entry - 0x2c;  // ebx, edx; the struct tm
    x86::reg32 milliseconds, result;
    if (!currentTime(app, cpu, tm - 4, tm, milliseconds))
        return std::nullopt;
    if (x86::sreg32(milliseconds) >= 500)
        put(app, tm + kSec, at(app, tm + kSec) + 1);
    if (!makeTime(app, cpu, tm - 4, tm, result))
        return std::nullopt;
    if (timer != 0)
        put(app, timer, result);
    cpu.esp = entry;
    return result;
}

x86::reg32 time(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (const auto result = timeOf(app, cpu, args.eax))
        returns(cpu, args, *result);
    return args.eax;
}

/* _localtime (clib/time/c/localtim.c, 0x4f8040): *eax broken down into the
 * struct tm at edx; that. */
x86::reg32 localtime_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    const x86::reg32 tm = args.edx;
    if (!localTime(app, cpu, entry, args.eax, tm))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, tm);
    return args.eax;
}

/* localtime (clib/time/c/localtim.c, 0x4f8094): *eax broken down into the
 * thread's struct tm; that. */
x86::reg32 localtime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(args);
    const x86::reg32 timer = args.eax;
    x86::reg32 data;
    cpu.esp = entry - 8;  // ebx, edx
    if (!threadData(app, cpu, data))
        return args.eax;
    const x86::reg32 tm = data + 0x14;
    if (!localTime(app, cpu, entry - 0xc, timer, tm))
        return args.eax;
    kept.back(args);
    cpu.esp = entry;
    returns(cpu, args, tm);
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

constexpr x86::reg32 kReportError = 0x5243a7;  // __reporterror
constexpr x86::reg32 k8087 = 0x5644c0;         // _8087: whether there is an x87
constexpr x86::reg32 kMinusOne = 0x54c970;     // floor's -1.0
constexpr x86::reg32 kExpMax = 0x56716c, kExpMin = 0x567174;  // exp's limits, doubles
constexpr x86::reg32 kHalfPi = 0x56719c;       // IF@DACOS's pi/2, 80 bits
constexpr x86::reg32 kEDOM = 13;
constexpr double kLog2e = 1.4426950408889634;  // fldl2e, as the port loads it
constexpr double kPi = 3.1415926535897932;     // fldpi
/* sahf's carry and zero: C0 and C3. */
constexpr x86::reg16 kC0 = 0x100, kC3 = 0x4000;

x86::Float loadDouble(win32::WinApplication* app, x86::reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

void storeDouble(win32::WinApplication* app, x86::reg32 address, const x86::Float& value)
{
    app->getMemory<double>(address) = double(value);
}

/* The x87's arithmetic as the control word `control` has it (x86::FloatEnv's, include/fpu.h): at 53 or
 * 64 bits a double's, at 24 rounded to a float's significand in the word's mode -- fp::'s, to
 * nearest.  The maths below run under their caller's word, which their thunks hand down: the
 * races' single precision in the game. */
struct Arithmetic
{
    x86::reg16 control;

    unsigned rc() const { return control >> 10 & 3; }

    double rounded(double value, x86::FloatEnv::Rounded op, double a, double b) const
    {
        if ((control >> 8 & 3) != 0)
            return value;
        if (rc() == 0)
            return fp::single(value, op, a, b);
        std::uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        if ((bits << 1) == 0)
            return value;  // either zero
        return x86::FloatEnv::roundSingleSlow(value, op, a, b, rc());
    }
    double add(double a, double b) const { return rounded(a + b, x86::FloatEnv::Rounded::Add, a, b); }
    double sub(double a, double b) const { return rounded(a - b, x86::FloatEnv::Rounded::Sub, a, b); }
    double mul(double a, double b) const { return rounded(a * b, x86::FloatEnv::Rounded::Mul, a, b); }
    double div(double a, double b) const { return rounded(a / b, x86::FloatEnv::Rounded::Div, a, b); }
    double sqrt(double a) const { return rounded(std::sqrt(a), x86::FloatEnv::Rounded::Sqrt, a, 0.0); }
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
x86::reg16 compared(x86::reg16 status, double a, double b)
{
    return x86::reg16((status & ~0x4500) | fp::compare(a, b));
}

/* modf's: the double at `argument` into its integer part, chopped, at `whole`; its fraction given
 * back. */
x86::Float splitOf(win32::WinApplication* app, const Arithmetic& x87, x86::reg32 argument, x86::reg32 whole)
{
    const x86::Float value = loadDouble(app, argument);
    const x86::Float chopped = clib::chop(app, value);
    const x86::Float fraction = x87.sub(value, chopped);
    storeDouble(app, whole, chopped);
    return fraction;
}

/* floor's: the double at `argument` rounded down, by modf, its integer part at `whole` -- one less
 * for a fraction below 0 -- given back; `status` the status word after its fcom. */
x86::Float floorOf(win32::WinApplication* app, const Arithmetic& x87, x86::reg32 argument, x86::reg32 whole,
                   x86::reg16& status)
{
    const x86::Float fraction = splitOf(app, x87, argument, whole);
    status = compared(status, 0.0, fraction);
    if (!(status & (kC0 | kC3)))
        storeDouble(app, whole, x87.add(loadDouble(app, whole), loadDouble(app, kMinusOne)));
    return loadDouble(app, whole);
}

/* IF@DATAN2's: fpatan's angle of st(0) `x` over st(1) `y` -- atan2 of x by y, after its fxch. */
x86::Float angleOf(const x86::Float& x, const x86::Float& y)
{
    return fp::atan(y, x);
}

/* __math87_err's, for a call whose return address is at `entry`: the double at entry + 4 and the
 * function at entry + 0xc, its byte, made __reporterror's flags -- or, for exp's underflow, 0 -- and
 * what that gives back given back, by way of a double in its frame (under the pushes of ebp, eax,
 * ebx, ecx and edx: their room only).  `status` the status word after its fcoms; nullopt when the
 * game stopped. */
std::optional<x86::Float> mathErrorOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry,
                                      x86::reg16& status)
{
    const x86::reg32 argument = entry + 4;
    const x86::reg32 result = entry - 0x1c;
    const x86::reg8 function = byte(app, entry + 0xc);
    x86::reg32 flags = 0;
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
        const x86::reg32 esp = cpu.esp;
        cpu.esp = result;
        const double retval = guest::direct<kReportError, clib::reporterror>(app, cpu, flags, argument);
        cpu.esp = esp;
        if (cpu.terminate)
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
std::optional<x86::Float> squareRootOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry,
                                       const Arithmetic& x87, const x86::Float& x, const x86::Float& below,
                                       x86::reg32& eax, x86::reg16& status, unsigned& popped)
{
    status = compared(status, x, 0.0);
    const x86::reg32 slot = entry - 0xc;  // fnstsw's word, which xchg gives ax for
    app->getMemory<x86::reg16>(slot) = x86::reg16(eax);
    popped = 0;
    if (!(status & kC0))
    {
        eax = (eax & 0xffff0000) | (status & 0xff00);
        return x87.sqrt(x);
    }
    const x86::reg32 argument = entry - 0x14;
    storeDouble(app, argument, x);
    popped = 1;
    eax = at(app, slot);
    if (x86::reg8(eax) != 3)
    {
        storeDouble(app, argument, below);
        popped = 2;
    }
    const auto result = mathErrorOf(app, cpu, argument - 4, status);
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
std::optional<x86::Float> exponentialOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry,
                                        const Arithmetic& x87, const x86::Float& x, x86::reg32& eax,
                                        x86::reg16& status)
{
    eax = (eax & 0xffffff00) | 4;
    status = compared(status, x, loadDouble(app, kExpMax));
    eax = (eax & 0xffff00ff) | (status & 0xff00);
    if (!(status & (kC0 | kC3)))
    {
        const x86::reg32 argument = entry - 0x14;
        storeDouble(app, argument, x);
        put(app, entry - 0xc, eax);  // al the function, for __math87_err
        const auto result = mathErrorOf(app, cpu, argument - 4, status);
        if (!result)
            return std::nullopt;
        eax = (eax & 0xffffff00) | 1;
        return result;
    }
    status = compared(status, x, loadDouble(app, kExpMin));
    eax = (eax & 0xffff00ff) | (status & 0xff00);
    x86::Float result = 0.0;
    if (!(status & (kC0 | kC3)))
    {
        const x86::Float scaled = x87.mul(x, kLog2e);
        const x86::Float whole = x87.rndint(scaled);
        const x86::Float fraction = x87.sub(scaled, whole);
        const x86::Float power = x87.add(fp::f2xm1(fraction), 1.0);
        result = fp::scale(power, whole);
    }
    eax &= 0xffffff00;
    return result;
}

/* IF@DACOS's, for a call whose return address is at `entry`: acos of `x` (st(0)) under `x87` --
 * atan2 of x by sqrt(1 - x*x), from pi/2; 0 or pi for 1 and -1 -- what it leaves in st(0).  `eax`
 * the caller's, as the code leaves it: ax as it came (its push's word), the rest as __@DSQRT leaves
 * it.  `status` the status word after; nullopt when the game stopped. */
std::optional<x86::Float> arcCosineOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry,
                                      const Arithmetic& x87, const x86::Float& x, x86::reg32& eax,
                                      x86::reg16& status)
{
    const x86::reg32 saved = entry - 4;  // push ax: the port's code takes a dword for it
    app->getMemory<x86::reg16>(saved) = x86::reg16(eax);
    const x86::Float rest = x87.sub(1.0, x87.mul(x, x));
    status = compared(status, rest, 0.0);
    eax = (eax & 0xffff0000) | status;
    x86::Float result;
    if (status & kC3)
    {
        status = compared(status, rest, x);
        eax = (eax & 0xffff0000) | status;
        result = status & (kC0 | kC3) ? x86::Float(0.0) : x86::Float(kPi);
    }
    else
    {
        eax = (eax & 0xffffff00) | 1;
        unsigned popped;
        const auto root = squareRootOf(app, cpu, saved - 4, x87, rest, x, eax, status, popped);
        if (!root)
            return std::nullopt;
        result = *root;
        if (x86::reg8(eax) == 0)
        {
            const x86::Float halfPi = x86::Float(app->getMemory<x86::IEEEf80>(kHalfPi));
            result = x87.sub(halfPi, angleOf(x, *root));
        }
    }
    eax = (eax & 0xffff0000) | app->getMemory<x86::reg16>(saved);
    return result;
}

}  // namespace

/* IF@DATAN2 (mathlib/a/atan87.asm, 0x4e06d9): atan2 of st(1) by st(0), into
 * st(0). */
x86::reg32 IF_DATAN2(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    const x86::Float x = args.floatIn[0], y = args.floatIn[1];
    args.floatsTaken = 2;  // fxch's, the two taken off
    args.floatSet = true;
    args.floatResult = angleOf(x, y);
    cpu.esp += 4;
    return args.eax;
}

/* floor (mathlib/c/floor.c, 0x4f1c30): the double on the stack rounded
 * down, onto the x87's; stdcall. */
x86::reg32 floor(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    x86::reg16 status = cpu.fenv.status.word;
    // ebp, eax, ebx, edx pushed; the integer part.
    const x86::Float result = floorOf(app, {cpu.fenv.control.word}, entry + 4, entry - 0x18, status);
    cpu.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = result;
    cpu.esp = entry + 0xc;
    return args.eax;
}

/* ceil (mathlib/c/ceil.c, 0x4f2d78): -floor(-x). */
x86::reg32 ceil(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 negated = entry - 0x10;  // ebp, eax; floor's argument
    storeDouble(app, negated, -loadDouble(app, entry + 4));
    x86::reg16 status = cpu.fenv.status.word;
    const x86::Float result = floorOf(app, {cpu.fenv.control.word}, negated, negated - 4 - 0x18, status);
    cpu.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = -result;
    cpu.esp = entry + 0xc;
    return args.eax;
}

x86::Float floored(win32::WinApplication* app, x86::CPU& cpu, x86::Float value)
{
    const x86::reg32 entry = cpu.esp - 8;  // the double's two words under the return address's slot
    storeDouble(app, entry + 4, value);
    x86::reg16 status = cpu.fenv.status.word;
    const x86::Float result = floorOf(app, {cpu.fenv.control.word}, entry + 4, entry - 0x18, status);
    cpu.fenv.status.word = status;
    return result;
}

x86::Float ceiled(win32::WinApplication* app, x86::CPU& cpu, x86::Float value)
{
    const x86::reg32 entry = cpu.esp - 8;
    storeDouble(app, entry + 4, value);
    const x86::reg32 negated = entry - 0x10;  // ebp, eax; floor's argument
    storeDouble(app, negated, -value);
    x86::reg16 status = cpu.fenv.status.word;
    const x86::Float result = floorOf(app, {cpu.fenv.control.word}, negated, negated - 4 - 0x18, status);
    cpu.fenv.status.word = status;
    return -result;
}

/* modf (0x50ea4e): the double on the stack into its integer part, at the
 * pointer after it, and its fraction, onto the x87's; stdcall. */
x86::reg32 modf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::Float fraction = splitOf(app, {cpu.fenv.control.word}, entry + 4, at(app, entry + 0xc));
    args.floatSet = true;
    args.floatResult = fraction;
    cpu.esp = entry + 0x10;
    return args.eax;
}

/* _control87 (clib/fpu/c/cntrl87.c, 0x4f7c90): the control word's bits of
 * edx made eax's, if there is an x87 (_8087); the control word, or 0. */
x86::reg32 control87(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 word = 0;
    if (byte(app, k8087) != 0)
    {
        word = cpu.fenv.control.word;
        const x86::reg32 mask = args.edx;
        if (mask != 0)
        {
            word = (~mask & word) | (mask & args.eax);
            cpu.fenv.setControl(x86::reg16(word));
            word = (word & 0xffff0000) | cpu.fenv.control.word;
        }
    }
    returns(cpu, args, word & 0xffff);
    return args.eax;
}

/* IF@DEXP (0x4f80b4): e to st(0), into st(0). */
x86::reg32 IF_DEXP(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    x86::reg32 eax = args.eax;
    x86::reg16 status = cpu.fenv.status.word;
    const auto result = exponentialOf(app, cpu, entry, {cpu.fenv.control.word}, args.floatIn[0], eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    cpu.fenv.status.word = status;
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    cpu.esp = entry + 4;
    return args.eax;
}

/* exp (mathlib/a/exp87.asm, 0x4f8116): e to the double on the stack, onto
 * the x87's; stdcall. */
x86::reg32 exp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    x86::reg32 eax = args.eax;
    x86::reg16 status = cpu.fenv.status.word;
    const auto result =
        exponentialOf(app, cpu, entry - 4, {cpu.fenv.control.word}, loadDouble(app, entry + 4), eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    cpu.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    cpu.esp = entry + 0xc;
    return args.eax;
}

/* exp's core (0x4f8116): e to `power`, as the code leaves it on the x87's stack; 0 with cpu.terminate set when the
 * game stopped.  Called through guest::direct only (its frame at the depth of the game's call). */
double clib::exponent(win32::WinApplication* app, x86::CPU& cpu, double power)
{
    x86::reg32 eax = 0;  // its upper half reaches only __math87_err's record, past exp's range
    x86::reg16 status = cpu.fenv.status.word;
    const auto result = exponentialOf(app, cpu, cpu.esp - 4, {cpu.fenv.control.word}, power, eax, status);
    if (!result)
        return 0.0;
    cpu.fenv.status.word = status;
    return double(*result);
}

/* IF@DACOS (mathlib/a/acos87.asm, 0x4f9482): acos of st(0) -- atan2 of
 * sqrt(1 - x*x) by x, from pi/2; 0 or pi for 1 and -1 -- into st(0). */
x86::reg32 IF_DACOS(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    x86::reg32 eax = args.eax;
    x86::reg16 status = cpu.fenv.status.word;
    const auto result = arcCosineOf(app, cpu, entry, {cpu.fenv.control.word}, args.floatIn[0], eax, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    cpu.fenv.status.word = status;
    args.floatsTaken = 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    cpu.esp = entry + 4;
    return args.eax;
}

/* __@DSQRT (mathlib, 0x51a1ba): st(0)'s root, al the caller's function; al
 * 0, or 1 for a math error. */
x86::reg32 DSQRT(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as __math87_err's pops leave them
    x86::reg32 eax = args.eax;
    x86::reg16 status = cpu.fenv.status.word;
    unsigned popped;
    const auto result = squareRootOf(app, cpu, entry, {cpu.fenv.control.word}, args.floatIn[0], args.floatIn[1], eax,
                                     status, popped);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    cpu.fenv.status.word = status;
    // __math87_err's result pushed after what it took
    args.floatsTaken = popped != 0 ? popped : 1;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    cpu.esp = entry + 4;
    return args.eax;
}

/* __math87_err (0x51655c): a math error -- the argument, a double, and the
 * function, a byte of the dword after it, on the stack -- reported; what
 * _matherr or the runtime gives back onto the x87's.  Takes its arguments
 * off; eax, ebx, ecx and edx as they came. */
x86::reg32 math87_err(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 eax = args.eax, ebx = args.ebx, ecx = args.ecx, edx = args.edx;  // as its pops leave them
    x86::reg16 status = cpu.fenv.status.word;
    const auto result = mathErrorOf(app, cpu, entry, status);
    if (!result)
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    cpu.fenv.status.word = status;
    args.floatSet = true;
    args.floatResult = *result;
    args.eax = eax;
    cpu.esp = entry + 0x10;
    return args.eax;
}

/* __set_EDOM (0x502890): errno EDOM. */
x86::reg32 set_EDOM(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!setThreadWord(app, cpu, kErrno, kEDOM))
        return args.eax;
    const x86::reg32 data = args.eax;
    kept.back(args);
    returns(cpu, args, data);
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

constexpr x86::reg32 kPtr = 0, kCnt = 4, kLink = 8, kFlag = 0xc, kFileHandle = 0x10, kBufsize = 0x14;
constexpr x86::reg32 kUngotten = 0x18;
constexpr x86::reg32 kLinkNext = 0, kLinkStream = 4, kLinkBase = 8, kOrientation = 0xc, kExtFlags = 0x10;
constexpr x86::reg32 kOpenStreams = 0x9f38a0;                      // __OpenStreams: their links
constexpr x86::reg32 kAccessIob = 0x567778, kReleaseIob = 0x56777c;  // the streams' lock
constexpr x86::reg32 kFlushFileBuffers = 0x5344d4;
constexpr x86::reg32 kGetche = 0x522e60;  // the console's getche: still the game's
/* _flag's bits. */
constexpr x86::reg32 kReading = 0x1, kWriting = 0x2, kUnget = 0x4, kBigBuf = 0x8, kAtEof = 0x10, kError = 0x20;
constexpr x86::reg32 kBinary = 0x40;  // and _APPEND a mode's kAppend
constexpr x86::reg32 kFullBuf = 0x100, kLineBuf = 0x200, kNoBuf = 0x400, kBuffering = 0x700;
constexpr x86::reg32 kDirty = 0x1000, kTty = 0x2000;
constexpr x86::reg32 kCommit = 1;  // the link's extended flags

/* A function's frame as its code makes it -- the registers it pushes, its
 * locals -- esp that far down while it calls what it calls. */
class Frame
{
public:
    Frame(x86::CPU& cpu, x86::reg32 size) : cpu_(cpu), entry_(cpu.esp) { cpu.esp = entry_ - size; }
    ~Frame() { cpu_.esp = entry_; }
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

private:
    x86::CPU& cpu_;
    const x86::reg32 entry_;
};

/* One of these called from another as its code calls it: the return
 * address's slot below the caller's frame. */
template <typename Function>
bool nested(x86::CPU& cpu, Function function)
{
    const Frame slot(cpu, 4);
    return function();
}

x86::reg32 linkOf(win32::WinApplication* app, x86::reg32 fp)
{
    return at(app, fp + kLink);
}

x86::reg32 bufferOf(win32::WinApplication* app, x86::reg32 fp)
{
    return at(app, linkOf(app, fp) + kLinkBase);
}

x86::reg32 flags(win32::WinApplication* app, x86::reg32 fp)
{
    return at(app, fp + kFlag);
}

void setFlags(win32::WinApplication* app, x86::reg32 fp, x86::reg32 set, x86::reg32 clear = 0)
{
    put(app, fp + kFlag, (flags(app, fp) & ~clear) | set);
}

bool streamLock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 fp)
{
    return fileLock(app, cpu, slot, at(app, fp + kFileHandle));
}

/* The buffer emptied: _cnt 0, _ptr its start, read first. */
x86::reg32 emptyBuffer(win32::WinApplication* app, x86::reg32 fp)
{
    const x86::reg32 base = bufferOf(app, fp);
    put(app, fp + kCnt, 0);
    put(app, fp + kPtr, base);
    return base;
}

/* fsync's (clib/handleio/c/fsync.c): FlushFileBuffers under the lock; 0, or
 * -1. */
bool syncHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& result)
{
    const Frame frame(cpu, 0x10);  // ebx, ecx, edx, esi
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    result = 0;
    if (!fileLock(app, cpu, kAccessFile, handle))
        return false;
    const auto flushed = windows<kFlushFileBuffers, &host::flushFile>(app, cpu, osHandle(app, handle));
    if (!flushed)
        return false;
    if (*flushed == 0)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return false;
    }
    return fileLock(app, cpu, kReleaseFile, handle);
}

/* __chktty's: a tty's stream marked one, and line-buffered unless it is
 * buffered some way already; `eax` as its code leaves it. */
bool checkTty(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& eax)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    eax = fp;
    if (flags(app, fp) & kTty)
        return true;
    if (!called(cpu, [&] { return isTty(app, cpu, at(app, fp + kFileHandle), eax); }))
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
bool allocateBuffer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& eax)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    if (!nested(cpu, [&] { return checkTty(app, cpu, fp, eax); }))
        return false;
    if (at(app, fp + kBufsize) == 0)
    {
        const x86::reg32 f = flags(app, fp);
        put(app, fp + kBufsize, f & kLineBuf ? 0x86 : f & kNoBuf ? 1 : 0x1000);
    }
    x86::reg32 block;
    if (!called(cpu, [&] { return watcomheap::allocate(app, cpu, kGameHeap, at(app, fp + kBufsize), block); }))
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

bool allocateBuffer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp)
{
    x86::reg32 eax;
    return allocateBuffer(app, cpu, fp, eax);
}

/* __flush's: a written buffer to __qwrite, all of it -- ENOSPC for none
 * taken; a read one's unread part seeked back over, unless it is a tty's;
 * the buffer emptied, and committed if the link says so.  0, or -1. */
bool flushStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x18);  // ebx, ecx, edx, esi, edi, ebp
    result = 0;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    if (flags(app, fp) & kDirty)
    {
        setFlags(app, fp, 0, kDirty);
        x86::reg32 from = bufferOf(app, fp);
        if ((flags(app, fp) & kWriting) && from != 0)
        {
            for (x86::reg32 left = at(app, fp + kCnt); left != 0 && result == 0;)
            {
                x86::reg32 written;
                if (!called(cpu, [&] { return qwriteOf(app, cpu, at(app, fp + kFileHandle), from, left, written); }))
                    return false;
                if (written == 0xffffffff)
                {
                    setFlags(app, fp, kError);
                    result = written;
                }
                else if (written == 0)
                {
                    if (!setThreadWord(app, cpu, kErrno, kENOSPC))
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
            x86::reg32 place = 0;
            if (at(app, fp + kCnt) != 0)
            {
                if (!called(cpu, [&] {
                        return lseekOf(app, cpu, at(app, fp + kFileHandle), 0 - at(app, fp + kCnt), 1, place);  // SEEK_CUR
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
        x86::reg32 synced;
        if (!nested(cpu, [&] { return syncHandle(app, cpu, at(app, fp + kFileHandle), synced); }))
            return false;
        if (synced == 0xffffffff)
            result = synced;
    }
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* fputc's: c into the buffer -- '\n' after a '\r' in text -- flushed when
 * it fills, or for a line or a character as the stream is buffered; a
 * stream of wide characters takes none, one not open for writing is EBADF.
 * The character, or -1. */
bool putChar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 c, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x14);  // ebx, ecx, esi, edi, ebp
    result = 0xffffffff;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    const x86::reg32 link = linkOf(app, fp);
    const x86::reg32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
            return streamLock(app, cpu, kReleaseFile, fp);
        put(app, link + kOrientation, 1);
    }
    if (!(flags(app, fp) & kWriting))
    {
        if (!setThreadWord(app, cpu, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, cpu, kReleaseFile, fp);
    }
    if (bufferOf(app, fp) == 0 && !nested(cpu, [&] { return allocateBuffer(app, cpu, fp); }))
        return false;
    auto put1 = [&](x86::reg8 byte_) {
        setFlags(app, fp, kDirty);
        putByte(app, at(app, fp + kPtr), byte_);
        put(app, fp + kPtr, at(app, fp + kPtr) + 1);
        put(app, fp + kCnt, at(app, fp + kCnt) + 1);
    };
    x86::reg32 flushOn = kNoBuf;
    if (c == '\n')
    {
        flushOn = kNoBuf | kLineBuf;
        if (!(flags(app, fp) & kBinary))
        {
            put1('\r');
            if (at(app, fp + kCnt) == at(app, fp + kBufsize))
            {
                x86::reg32 flushed;
                if (!nested(cpu, [&] { return flushStream(app, cpu, fp, flushed); }))
                    return false;
                if (flushed != 0)
                    return streamLock(app, cpu, kReleaseFile, fp);
            }
        }
    }
    put1(x86::reg8(c));
    if ((flags(app, fp) & flushOn) || at(app, fp + kCnt) == at(app, fp + kBufsize))
    {
        x86::reg32 flushed;
        if (!nested(cpu, [&] { return flushStream(app, cpu, fp, flushed); }))
            return false;
        if (flushed != 0)
            return streamLock(app, cpu, kReleaseFile, fp);
    }
    result = c & 0xff;
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* fputs's: the string by fputc, an unbuffered stream line-buffered while
 * it goes and flushed after; how many, or -1. */
bool putString(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x14);  // ebx, ecx, esi, edi, ebp
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    if (bufferOf(app, fp) == 0 && !nested(cpu, [&] { return allocateBuffer(app, cpu, fp); }))
        return false;
    const bool unbuffered = flags(app, fp) & kNoBuf;
    if (unbuffered)
        setFlags(app, fp, kLineBuf, kLineBuf | kNoBuf);
    result = 0;
    x86::reg32 at_ = string;
    for (; byte(app, at_) != 0; ++at_)
    {
        x86::reg32 put_;
        if (!nested(cpu, [&] { return putChar(app, cpu, byte(app, at_), fp, put_); }))
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
        if (result == 0 && !nested(cpu, [&] { return flushStream(app, cpu, fp, result); }))
            return false;
    }
    if (result == 0)
        result = at_ - string;
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* __flushall's: every open stream with a bit of `mask` counted, and flushed
 * if it is dirty, under the streams' lock -- given back with eax as the walk
 * leaves it: the mask, the last stream, or what flushing it gave. */
bool flushAll(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mask, x86::reg32& count)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    if (!hook<kAccessIob>(app, cpu, {mask}))
        return false;
    count = 0;
    x86::reg32 eax = mask;
    for (x86::reg32 link = at(app, kOpenStreams); link != 0; link = at(app, link + kLinkNext))
    {
        const x86::reg32 fp = eax = at(app, link + kLinkStream);
        if (!(flags(app, fp) & mask))
            continue;
        ++count;
        if ((flags(app, fp) & kDirty) && !nested(cpu, [&] { return flushStream(app, cpu, fp, eax); }))
            return false;
    }
    return hook<kReleaseIob>(app, cpu, {eax}).has_value();
}

/* fwrite's: n of size bytes -- binary, into the buffer, or straight to
 * __qwrite in whole 512s when the buffer is empty and they fill it; text,
 * by fputc, an unbuffered stream fully buffered while it goes -- until an
 * error; the stream's EOF and error as they were, and its new error.  How
 * many whole ones. */
bool writeStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size, x86::reg32 n,
                 x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x20);  // esi, edi, ebp; five dwords
    result = 0;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    if (!(flags(app, fp) & kWriting))
    {
        if (!setThreadWord(app, cpu, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, cpu, kReleaseFile, fp);
    }
    const x86::reg32 total = n * size;
    if (total == 0)
        return streamLock(app, cpu, kReleaseFile, fp);
    if (bufferOf(app, fp) == 0 && !nested(cpu, [&] { return allocateBuffer(app, cpu, fp); }))
        return false;
    const x86::reg32 kept = flags(app, fp) & (kAtEof | kError);
    setFlags(app, fp, 0, kAtEof | kError);
    x86::reg32 written = 0;
    if (flags(app, fp) & kBinary)
    {
        for (x86::reg32 left = total;;)
        {
            x86::reg32 step;
            if (at(app, fp + kCnt) == 0 && left >= at(app, fp + kBufsize))
            {
                x86::reg32 direct = left & 0xfffffe00;
                if (direct == 0)
                    direct = left;
                if (!called(cpu, [&] { return qwriteOf(app, cpu, at(app, fp + kFileHandle), buffer, direct, step); }))
                    return false;
                if (step == 0)
                {
                    x86::reg32 data;
                    if (!threadData(app, cpu, data))
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
                const x86::reg32 to = at(app, fp + kPtr);
                x86::reg32 i = 0;
                for (; i < (step & ~3u); i += 4)
                    put(app, to + i, at(app, buffer + i));
                for (; i < step; ++i)
                    putByte(app, to + i, byte(app, buffer + i));
                put(app, fp + kCnt, at(app, fp + kCnt) + step);
                put(app, fp + kPtr, at(app, fp + kPtr) + step);
                setFlags(app, fp, kDirty);
                if (at(app, fp + kCnt) == at(app, fp + kBufsize) || (flags(app, fp) & kNoBuf))
                {
                    x86::reg32 ignored;
                    if (!nested(cpu, [&] { return flushStream(app, cpu, fp, ignored); }))
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
        const x86::reg32 link = linkOf(app, fp);
        const x86::reg32 orientation = at(app, link + kOrientation);
        put(app, link + kOrientation, 1);
        for (;;)
        {
            x86::reg32 ignored;
            if (!nested(cpu, [&] { return putChar(app, cpu, byte(app, buffer++), fp, ignored); }))
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
            x86::reg32 ignored;
            if (!nested(cpu, [&] { return flushStream(app, cpu, fp, ignored); }))
                return false;
        }
    }
    if (flags(app, fp) & kError)
        written = 0;
    setFlags(app, fp, kept);
    if (!streamLock(app, cpu, kReleaseFile, fp))
        return false;
    result = written / size;
    return true;
}

/* __update_buffer's (clib/streamio/c/fseek.c): a seek `diff` from where the
 * stream is, within what is buffered -- back to the buffer's start, on to
 * what is left in it -- done there, the end of the file cleared, and 0; 1
 * for one outside it. */
x86::reg32 updateBuffer(win32::WinApplication* app, x86::reg32 diff, x86::reg32 fp)
{
    if (x86::sreg32(diff) > x86::sreg32(at(app, fp + kCnt))
        || x86::sreg32(diff) < x86::sreg32(bufferOf(app, fp) - at(app, fp + kPtr)))
        return 1;
    const x86::reg32 ptr = at(app, fp + kPtr) + diff, cnt = at(app, fp + kCnt) - diff;
    setFlags(app, fp, 0, kAtEof);
    put(app, fp + kPtr, ptr);
    put(app, fp + kCnt, cnt);
    return 0;
}

/* __reset_buffer's: the end of the file cleared and the buffer emptied. */
void resetBuffer(win32::WinApplication* app, x86::reg32 fp)
{
    setFlags(app, fp, 0, kAtEof);
    emptyBuffer(app, fp);
}

/* __fill_buffer's (clib/streamio/c/fgetc.c): the buffer filled by __qread
 * -- a byte for an unbuffered stream -- from where it starts, the ttys'
 * streams flushed first if this is a tty's line by line or unbuffered; an
 * unbuffered tty's handle 0 by the console's getche.  How many are in it:
 * none at the end of the file, none and an error for one. */
bool fillBuffer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& count)
{
    const Frame frame(cpu, 0x10);  // ebx, ecx, edx, esi
    if (bufferOf(app, fp) == 0 && !nested(cpu, [&] { return allocateBuffer(app, cpu, fp); }))
        return false;
    if ((flags(app, fp) & kTty) && (flags(app, fp) & (kLineBuf | kNoBuf)))
    {
        x86::reg32 ignored;
        if (!nested(cpu, [&] { return flushAll(app, cpu, kTty, ignored); }))
            return false;
    }
    setFlags(app, fp, 0, kUnget);
    put(app, fp + kPtr, bufferOf(app, fp));
    if ((flags(app, fp) & (kTty | kNoBuf)) == (kTty | kNoBuf) && at(app, fp + kFileHandle) == 0)
    {
        put(app, fp + kCnt, 0);
        const x86::reg32 c = guest::direct<kGetche, clib::getche>(app, cpu);
        if (cpu.terminate)
            return false;
        if (c != 0xffffffff)
        {
            putByte(app, at(app, fp + kPtr), x86::reg8(c));
            put(app, fp + kCnt, 1);
        }
    }
    else
    {
        x86::reg32 read;
        if (!called(cpu, [&] {
                return qreadOf(app, cpu, at(app, fp + kFileHandle), at(app, fp + kPtr),
                               flags(app, fp) & kNoBuf ? 1 : at(app, fp + kBufsize), read);
            }))
            return false;
        put(app, fp + kCnt, read);
    }
    const x86::sreg32 cnt = x86::sreg32(at(app, fp + kCnt));
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
bool fillAndTake(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& c)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    x86::reg32 count;
    if (!nested(cpu, [&] { return fillBuffer(app, cpu, fp, count); }))
        return false;
    c = 0xffffffff;
    if (count != 0)
    {
        const x86::reg32 cnt = at(app, fp + kCnt) - 1, ptr = at(app, fp + kPtr) + 1;
        put(app, fp + kCnt, cnt);
        put(app, fp + kPtr, ptr);
        c = byte(app, ptr - 1);
    }
    return true;
}

/* A byte of the buffer, as fgetc takes one: _cnt down, and the buffer
 * filled when that leaves it below 0. */
bool takeChar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& c)
{
    const x86::reg32 cnt = at(app, fp + kCnt) - 1;
    put(app, fp + kCnt, cnt);
    if (x86::sreg32(cnt) < 0)
        return nested(cpu, [&] { return fillAndTake(app, cpu, fp, c); });
    const x86::reg32 ptr = at(app, fp + kPtr);
    c = byte(app, ptr);
    put(app, fp + kPtr, ptr + 1);
    return true;
}

/* fgetc's: a byte of the stream -- in text a '\r' dropped for the byte
 * after it, ^Z its end -- or -1; a stream of wide characters gives none,
 * one not open for reading is EBADF. */
bool getChar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x14);  // ebx, ecx, edx, esi, ebp
    result = 0xffffffff;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    const x86::reg32 link = linkOf(app, fp);
    const x86::reg32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
            return streamLock(app, cpu, kReleaseFile, fp);
        put(app, link + kOrientation, 1);
    }
    x86::reg32 c = 0xffffffff;
    if (!(flags(app, fp) & kReading))
    {
        if (!setThreadWord(app, cpu, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
    }
    else if (!takeChar(app, cpu, fp, c))
    {
        return false;
    }
    if (!(flags(app, fp) & kBinary))
    {
        if (c == '\r' && !takeChar(app, cpu, fp, c))
            return false;
        if (c == 0x1a)
        {
            setFlags(app, fp, kAtEof);
            c = 0xffffffff;
        }
    }
    result = c;
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* fread's: n of size bytes -- binary, what the buffer has, then straight
 * from __qread in whole 512s (unless the stream is unbuffered) while as many
 * as the buffer holds are left, through the buffer otherwise; text, by the
 * byte, each '\r' dropped for the byte after it, up to a ^Z, its end -- until
 * the end of the file or an error.  How many whole ones. */
bool readStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size, x86::reg32 n,
                x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x1c);  // esi, edi, ebp; four dwords
    result = 0;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    if (!(flags(app, fp) & kReading))
    {
        if (!setThreadWord(app, cpu, kErrno, kEBADF))
            return false;
        setFlags(app, fp, kError);
        return streamLock(app, cpu, kReleaseFile, fp);
    }
    const x86::reg32 total = n * size;
    if (total == 0)
        return streamLock(app, cpu, kReleaseFile, fp);
    if (bufferOf(app, fp) == 0 && !nested(cpu, [&] { return allocateBuffer(app, cpu, fp); }))
        return false;
    x86::reg32 read = 0;
    if (flags(app, fp) & kBinary)
    {
        for (x86::reg32 left = total;;)
        {
            const x86::reg32 cnt = at(app, fp + kCnt);
            if (cnt != 0)
            {
                // rep movsd, then rep movsb: by dwords where they overlap.
                const x86::reg32 step = std::min(cnt, left);
                const x86::reg32 from = at(app, fp + kPtr);
                x86::reg32 i = 0;
                for (; i < (step & ~3u); i += 4)
                    put(app, buffer + i, at(app, from + i));
                for (; i < step; ++i)
                    putByte(app, buffer + i, byte(app, from + i));
                const x86::reg32 ptr = at(app, fp + kPtr) + step, rest = at(app, fp + kCnt) - step;
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
                const x86::reg32 base = bufferOf(app, fp);
                put(app, fp + kCnt, 0);
                x86::reg32 asked = left;
                const bool unbuffered = flags(app, fp) & kNoBuf;
                put(app, fp + kPtr, base);
                if (!unbuffered && asked > 0x200)
                    asked &= 0xfffffe00;
                x86::reg32 got;
                if (!called(cpu, [&] { return qreadOf(app, cpu, at(app, fp + kFileHandle), buffer, asked, got); }))
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
                x86::reg32 count;
                if (!nested(cpu, [&] { return fillBuffer(app, cpu, fp, count); }))
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
        auto take = [&](x86::reg32& c) {
            if (at(app, fp + kCnt) == 0)
            {
                x86::reg32 count;
                if (!nested(cpu, [&] { return fillBuffer(app, cpu, fp, count); }))
                    return false;
                if (count == 0)
                    return c = 0xffffffff, true;
            }
            const x86::reg32 cnt = at(app, fp + kCnt) - 1, ptr = at(app, fp + kPtr);
            put(app, fp + kCnt, cnt);
            put(app, fp + kPtr, ptr + 1);
            c = byte(app, ptr);
            return true;
        };
        for (const x86::reg32 end = buffer + total; buffer != end;)
        {
            x86::reg32 c;
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
            putByte(app, buffer++, x86::reg8(c));
            ++read;
        }
    }
    if (!streamLock(app, cpu, kReleaseFile, fp))
        return false;
    result = read / size;
    return true;
}

/* fgets's: bytes by fgetc into the buffer, up to n - 1 of them and a '\n',
 * and a 0 after; none for the end of the file before any, or an error.  The
 * stream's end and error as they were, and new ones.  fgetc's last result is
 * kept in a dword of its frame, whatever was there for none. */
bool getString(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 n, x86::reg32 fp,
               x86::reg32& result)
{
    const Frame frame(cpu, 0x14);  // ecx, esi, edi, ebp; a dword
    x86::reg32 c = at(app, cpu.esp);
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    const x86::reg32 kept = flags(app, fp) & (kAtEof | kError);
    setFlags(app, fp, 0, kAtEof | kError);
    x86::reg32 to = buffer;
    for (x86::reg32 left = n - 1; x86::sreg32(left) > 0; --left)
    {
        if (!nested(cpu, [&] { return getChar(app, cpu, fp, c); }))
            return false;
        if (c == 0xffffffff)
            break;
        putByte(app, to++, x86::reg8(c));
        if (x86::reg8(c) == '\n')
            break;
    }
    result = buffer;
    if (c == 0xffffffff && (to == buffer || (flags(app, fp) & kError)))
        result = 0;
    else
        putByte(app, to, 0);
    setFlags(app, fp, kept);
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* fseek's: a stream open for writing, or with a byte put back, flushed and
 * its buffer given up, then lseek -- for SEEK_CUR from where the stream is,
 * not the handle; one for reading moved within its buffer where it can be,
 * by lseek and the buffer emptied otherwise.  0, or -1, EINVAL for a
 * whence that is none, and for a flush that failed on a SEEK_SET before 0. */
bool seekStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32 offset, x86::reg32 whence,
                x86::reg32& result)
{
    const Frame frame(cpu, 0xc);  // ecx, esi, edi
    result = 0xffffffff;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    auto invalid = [&] {
        return setThreadWord(app, cpu, kErrno, kEINVAL) && streamLock(app, cpu, kReleaseFile, fp);
    };
    auto seek = [&](x86::reg32 to, bool reset) {
        x86::reg32 place;
        if (!called(cpu, [&] { return lseekOf(app, cpu, at(app, fp + kFileHandle), to, whence, place); }))
            return false;
        if (place == 0xffffffff)
            return streamLock(app, cpu, kReleaseFile, fp);
        if (reset)
            resetBuffer(app, fp);
        result = 0;
        return streamLock(app, cpu, kReleaseFile, fp);
    };
    const x86::reg32 f = flags(app, fp);
    if (f & (kWriting | kUnget))
    {
        if (flags(app, fp) & kDirty)
        {
            x86::reg32 flushed;
            if (!nested(cpu, [&] { return flushStream(app, cpu, fp, flushed); }))
                return false;
            if (flushed != 0)
            {
                if (whence == 0 && x86::sreg32(offset) < 0)
                    return invalid();
                return streamLock(app, cpu, kReleaseFile, fp);
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
        const x86::reg32 cnt = at(app, fp + kCnt);
        if (updateBuffer(app, offset, fp) == 0)
            return result = 0, streamLock(app, cpu, kReleaseFile, fp);
        return seek(offset - cnt, true);
    }
    if (whence == 0)
    {
        x86::reg32 place;
        if (!called(cpu, [&] { return tellOf(app, cpu, at(app, fp + kFileHandle), place); }))
            return false;
        if (updateBuffer(app, offset - (place - at(app, fp + kCnt)), fp) == 0)
            return result = 0, streamLock(app, cpu, kReleaseFile, fp);
        return seek(offset, true);
    }
    if (whence == 2)
    {
        const x86::reg32 link = linkOf(app, fp);
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
bool tellStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0x10);  // ebx, ecx, edx, esi
    if ((flags(app, fp) & kAppend) && (flags(app, fp) & kDirty))
    {
        x86::reg32 ignored;
        if (!nested(cpu, [&] { return flushStream(app, cpu, fp, ignored); }))
            return false;
    }
    if (!called(cpu, [&] { return tellOf(app, cpu, at(app, fp + kFileHandle), result); }))
        return false;
    if (result == 0xffffffff)
        return true;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    const x86::reg32 cnt = at(app, fp + kCnt);
    if (cnt != 0)
        result = flags(app, fp) & kDirty ? result + cnt : result - cnt;
    return streamLock(app, cpu, kReleaseFile, fp);
}

/* ---- Opening and closing: a FILE of __iob's or the heap's, its link on
 * __OpenStreams, and a handle -- __OSHandles' first free, Windows' HANDLE in
 * it -- by CreateFileA; and back. ---- */

constexpr x86::reg32 kClosedStreams = 0x9f38a4;               // the links of streams closed, to be used again
constexpr x86::reg32 kIobEnd = 0x567160;                      // past __iob's 20
constexpr x86::reg32 kIobStart = 0x566f58, kFileSize = 0x1a, kLinkSize = 0x1d;
constexpr x86::reg32 kDynamic = 0x4000, kTmpFile = 0x800;     // _flag's: the heap's FILE; tmpfile's
constexpr x86::reg32 kTmpfChar = 0x14;                        // the link's: a temporary file's number
constexpr x86::reg32 kHandleCount = 0x56acc0;                 // __NHandles: how many __OSHandles has
constexpr x86::reg32 kFakeHandle = 0x56acc4;                  // __NTGetFakeHandle's last
constexpr x86::reg32 kAccessHandles = 0x567798, kReleaseHandles = 0x56779c;  // their lock
constexpr x86::reg32 kAllocHandle = 0x567770, kFreeHandle = 0x567774;        // what calls __allocPOSIXHandle and __freePOSIXHandle
/* The default windowing's other hooks, none in the game. */
constexpr x86::reg32 kRemoveWindowedHandle = 0x5677b4, kNewWindow = 0x5677b8, kDestroyWindow = 0x5677bc;
constexpr x86::reg32 kCommitMode = 0x568260;  // _commode
constexpr x86::reg32 kFileMode = 0x567164;    // _fmode: O_TEXT, or O_BINARY
constexpr x86::reg32 kUmask = 0x56ae60;
constexpr x86::reg32 kConsole = 0x54f6f0;     // "con"
constexpr x86::reg32 kRealloc = 0x5185c0;     // _nrealloc: still the game's
constexpr x86::reg32 kMkTmpFile = 0x4ee168;   // __MkTmpFile: still the game's
constexpr x86::reg32 kCloseHandle = 0x534488, kCreateEventA = 0x534494, kCreateFileA = 0x534498;
constexpr x86::reg32 kEMFILE = 11;
/* open's flags. */
constexpr x86::reg32 kOpenAppend = 0x10, kOpenCreate = 0x20, kOpenTruncate = 0x40, kOpenText = 0x100;
constexpr x86::reg32 kOpenNoInherit = 0x80, kOpenBinary = 0x200, kOpenExclusive = 0x400;

x86::reg32 freeStream(win32::WinApplication* app, x86::reg32 fp);
bool openHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 oflag, x86::reg32 shflag,
                x86::reg32 args, x86::reg32& result);

/* _sopen's, its return address at esp and its words above it -- the name, the flags, the sharing,
 * the rest: __sopen's, with a va_list of its frame for the rest. */
bool sopenAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& result)
{
    const x86::reg32 entry = cpu.esp;
    const Frame frame(cpu, 0x10);  // ebx, ecx, edx; the va_list
    const x86::reg32 args = cpu.esp;
    put(app, args, entry + 0x10);
    return nested(cpu, [&] {
        return openHandle(app, cpu, at(app, entry + 4), at(app, entry + 8), at(app, entry + 0xc), args, result);
    });
}

/* __GetNTAccessAttr's: CreateFileA's access and attributes for a read and
 * write mode. */
void accessAttributes(win32::WinApplication* app, x86::reg32 rwmode, x86::reg32 access, x86::reg32 attributes)
{
    put(app, access, rwmode == 2 ? 0xc0000000 : rwmode == 1 ? 0x40000000 : 0x80000000);
    put(app, attributes, rwmode == 2 || rwmode == 1 ? 0x80 : 1);
}

/* __GetNTShareAttr's: CreateFileA's sharing for SH_COMPAT -- reading, and
 * writing too for a file opened to read -- SH_DENYRW, SH_DENYWR, SH_DENYRD,
 * SH_DENYNO; none written for another.  The sharing bits. */
x86::reg32 shareAttributes(win32::WinApplication* app, x86::reg32 mode, x86::reg32 share)
{
    const x86::reg32 sharing = mode & 0x70;
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
x86::reg32 handlesFull(win32::WinApplication* app)
{
    const x86::reg32 count = at(app, kHandleCount), table = at(app, kOsHandles);
    if (count < at(app, kFileCount))
        return 0;
    if (x86::sreg32(count) <= 0)
        return 1;
    x86::reg32 offset = 0;
    do
    {
        if (at(app, table + offset) == 0)
            return 0;
        offset += 4;
    } while (x86::sreg32(offset) < x86::sreg32(count << 2));
    return 1;
}

/* __NTGetFakeHandle's: an event's HANDLE, or one past 0x80000000 of the
 * runtime's own. */
bool fakeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& handle)
{
    const Frame frame(cpu, 8);  // ecx, edx
    const auto event = windows<kCreateEventA, &host::createEvent>(app, cpu, 0, 0, 0, 0);
    if (!event)
        return false;
    handle = *event;
    if (handle == 0)
        put(app, kFakeHandle, handle = at(app, kFakeHandle) + 1);
    return true;
}

/* __allocPOSIXHandle's: the HANDLE into __OSHandles' first free slot, or
 * one more at its end, by _nrealloc, under the handles' lock -- given back
 * with eax the slot's offset, or the table.  Its handle. */
bool allocHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 os, x86::reg32& handle)
{
    const Frame frame(cpu, 0x10);  // ebx, ecx, edx, esi
    if (!hook<kAccessHandles>(app, cpu, {os}))
        return false;
    const x86::reg32 count = at(app, kHandleCount);
    if (x86::sreg32(count) > 0)
    {
        x86::reg32 offset = 0;
        handle = 0;
        do
        {
            const x86::reg32 slot = at(app, kOsHandles) + offset;
            if (at(app, slot) == 0)
            {
                put(app, slot, os);
                return hook<kReleaseHandles>(app, cpu, {offset}).has_value();
            }
            offset += 4;
            ++handle;
        } while (x86::sreg32(offset) < x86::sreg32(count << 2));
    }
    const x86::reg32 table =
        guest::direct<kRealloc, clib::realloc>(app, cpu, at(app, kOsHandles), (at(app, kHandleCount) + 1) << 2);
    if (cpu.terminate)
        return false;
    const x86::reg32 last = at(app, kHandleCount);
    put(app, kOsHandles, table);
    put(app, table + 4 * last, os);
    put(app, kHandleCount, last + 1);
    if (!hook<kReleaseHandles>(app, cpu, {table}))
        return false;
    handle = at(app, kHandleCount) - 1;
    return true;
}

/* __freePOSIXHandle's: a handle's slot free again, but 0's, under the
 * handles' lock; `eax` as its code leaves it, what the lock's giving back
 * leaves. */
bool freeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& eax)
{
    const Frame frame(cpu, 4);  // edx
    const auto taken = hook<kAccessHandles>(app, cpu, {handle});
    if (!taken)
        return false;
    eax = *taken;
    if (x86::sreg32(handle) > 0 && x86::sreg32(handle) < x86::sreg32(at(app, kHandleCount)))
    {
        eax = handle << 2;
        put(app, at(app, kOsHandles) + eax, 0);
    }
    const auto released = hook<kReleaseHandles>(app, cpu, {eax});
    if (!released)
        return false;
    eax = *released;
    return true;
}

/* __close's: a window's handle's window gone, by the default windowing's
 * hooks, or Windows' HANDLE closed -- EBADF and -1 for one CloseHandle
 * refuses -- and the handle's mode 0.  0, or -1. */
bool closeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32& result)
{
    const Frame frame(cpu, 0x18);  // ebx, ecx, edx, esi, edi, ebp
    if (!goodHandle(app, handle))
        return badHandle(app, cpu, result);
    result = 0;
    const x86::reg32 destroy = at(app, kDestroyWindow), os = osHandle(app, handle);
    x86::reg32 window = 0;
    if (destroy != 0)
    {
        const auto found = through(app, cpu, at(app, kIsWindowedHandle), {handle});
        if (!found)
            return false;
        window = *found;
        if (window != 0)
        {
            if (!through(app, cpu, at(app, kRemoveWindowedHandle), {handle})
                || !through(app, cpu, at(app, kDestroyWindow), {window}))
                return false;
        }
    }
    if (window == 0)
    {
        const auto closed = windows<kCloseHandle, &host::closeHandle>(app, cpu, os);
        if (!closed)
            return false;
        if (*closed == 0)
        {
            result = 0xffffffff;
            return setThreadWord(app, cpu, kErrno, kEBADF);
        }
    }
    setIoMode(app, handle, 0);  // __SetIOMode's
    return true;
}

/* __sopen's (clib/handleio/c/openwnt.c): name opened by CreateFileA as the
 * flags say -- created, truncated, exclusively; once more with no security
 * and the second disposition for one to create it could not open -- its
 * permissions from the va_list less the umask; a handle of __OSHandles for
 * it and its mode: read, write, append, binary as the flags or _fmode say,
 * a tty's.  "con" a window of the default windowing's, if it has any.  The
 * handle, or -1 and errno.  Its frame's eight dwords are CreateFileA's
 * SECURITY_ATTRIBUTES, sharing, access, attributes and dispositions, and
 * the flags, read back from there -- the sharing as the frame had it, for
 * one that is none of the five, which __GetNTShareAttr does not write. */
bool openHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 oflag, x86::reg32 shflag,
                x86::reg32 args, x86::reg32& result)
{
    const Frame frame(cpu, 0x2c);  // esi, edi, ebp; eight dwords
    const x86::reg32 local = cpu.esp;
    const x86::reg32 security = local, share = local + 0xc, access = local + 0x10, attributes = local + 0x14;
    const x86::reg32 retry = local + 0x18, flags = local + 0x1c;
    put(app, flags, oflag);
    result = 0xffffffff;
    if (handlesFull(app) != 0)
        return setThreadWord(app, cpu, kErrno, kEMFILE);
    const x86::reg32 rwmode = oflag & 7;
    accessAttributes(app, rwmode, access, attributes);
    shareAttributes(app, shflag | rwmode, share);
    x86::reg32 fileAttributes = 0x80;
    put(app, security + 4, 0);
    put(app, security, 0xc);
    put(app, security + 8, byte(app, flags) & kOpenNoInherit ? 0 : 1);
    x86::reg32 handle, mode = 0;
    bool window = false;
    if (at(app, kNewWindow) != 0 && clib::stricmp(app, name, kConsole) == 0)
    {
        x86::reg32 fake;
        if (!nested(cpu, [&] { return fakeHandle(app, cpu, fake); }))
            return false;
        const auto allocated = hook<kAllocHandle>(app, cpu, {fake});
        if (!allocated)
            return false;
        handle = *allocated;
        mode = 0x2000;
        // ecx and edx kept across it, which its code reads after.
        if (!through(app, cpu, at(app, kNewWindow), {handle, mode, 0, handle}, {0, handle, 0xffffffff}))
            return false;
        window = true;
    }
    if (!window)
    {
        x86::reg32 disposition;
        if (byte(app, flags) & kOpenCreate)
        {
            const x86::reg32 va = at(app, args) + 4;
            put(app, args, va);
            put(app, attributes, at(app, va - 4));
            put(app, args, 0);
            put(app, attributes, at(app, attributes) & ~at(app, kUmask));
            if ((byte(app, attributes + 1) & 1) && !(byte(app, attributes) & 0x80))
                fileAttributes = 1;  // FILE_ATTRIBUTE_READONLY
            if (byte(app, flags + 1) & (kOpenExclusive >> 8))
            {
                put(app, retry, 1);  // CREATE_NEW
                disposition = 1;
            }
            else if (byte(app, flags) & kOpenTruncate)
            {
                put(app, retry, 2);  // CREATE_ALWAYS
                disposition = 1;
            }
            else
            {
                put(app, retry, 4);  // OPEN_ALWAYS
                disposition = 3;     // OPEN_EXISTING
            }
        }
        else
        {
            disposition = byte(app, flags) & kOpenTruncate ? 5 : 3;  // TRUNCATE_EXISTING, OPEN_EXISTING
        }
        auto opened = windows<kCreateFileA, &host::createFile>(app, cpu, name, at(app, access), at(app, share),
                                                               security, disposition, fileAttributes, 0);
        if (!opened)
            return false;
        if (*opened == 0xffffffff && (byte(app, flags) & kOpenCreate))
        {
            opened = windows<kCreateFileA, &host::createFile>(app, cpu, name, at(app, access), at(app, share), 0,
                                                              at(app, retry), fileAttributes, 0);
            if (!opened)
                return false;
        }
        const x86::reg32 os = *opened;
        if (os == 0xffffffff)
            return setErrnoNt(app, cpu);
        const auto allocated = hook<kAllocHandle>(app, cpu, {os});
        if (!allocated)
            return false;
        handle = *allocated;
        if (handle >= at(app, kFileCount))
        {
            if (!windows<kCloseHandle, &host::closeHandle>(app, cpu, os))
                return false;
            return setThreadWord(app, cpu, kErrno, kENOMEM);
        }
        x86::reg32 tty;
        if (!called(cpu, [&] { return isTty(app, cpu, handle, tty); }))
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
    const x86::reg8 kind = byte(app, flags + 1);
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
bool openFlags(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 extflags, x86::reg32& result)
{
    const Frame frame(cpu, 0x18);  // ebx, ecx, esi, edi, ebp; a dword
    if (extflags != 0)
        put(app, extflags, at(app, kCommitMode) == 1 ? 1 : 0);
    x86::reg32 flags;
    switch (byte(app, mode))
    {
    case 'r': flags = kReading; break;
    case 'w': flags = kWriting; break;
    case 'a': flags = kWriting | kAppend; break;
    default:
        result = 0;
        return setThreadWord(app, cpu, kErrno, kEINVAL);
    }
    bool good = true, plus = false, kind = false, commit = false;
    for (x86::reg32 at_ = mode + 1; byte(app, at_) != 0 && good; ++at_)
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
bool doOpen(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg8 first, x86::reg32 flags,
            x86::reg32 extflags, x86::reg32 shflag, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 0xc);  // esi, edi, ebp
    putByte(app, fp + kFlag, byte(app, fp + kFlag) & ~(kReading | kWriting));
    put(app, fp + kFlag, at(app, fp + kFlag) | flags);
    x86::reg32 oflag = flags & kBinary ? kOpenBinary : kOpenText, permission = 0;
    if (x86::reg8(clib::tolower(app, first)) == 'r')
    {
        oflag |= flags & kWriting ? 2 : 0;
    }
    else
    {
        oflag |= (flags & kReading ? 2 : 1) | kOpenCreate | (flags & kAppend ? kOpenAppend : kOpenTruncate);
        permission = 0x180;
    }
    x86::reg32 handle;
    {
        // _sopen as the code calls it: its four words pushed, the va_list it makes points at the last.
        const EspKept kept(cpu);
        const x86::reg32 esp = cpu.esp;
        put(app, esp - 0x10, name);
        put(app, esp - 0xc, oflag);
        put(app, esp - 8, shflag);
        put(app, esp - 4, permission);
        cpu.esp = esp - 0x14;
        if (!sopenAt(app, cpu, handle))
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
    const x86::reg32 link = linkOf(app, fp);
    put(app, fp + kBufsize, 0);
    put(app, link + kOrientation, 0);
    put(app, linkOf(app, fp) + kExtFlags, extflags);
    put(app, linkOf(app, fp) + 0x15, 0);
    put(app, linkOf(app, fp) + kLinkBase, 0);
    if (flags & kAppend)
    {
        x86::reg32 ignored;
        if (!nested(cpu, [&] { return seekStream(app, cpu, fp, 0, 2, ignored); }))
            return false;
    }
    x86::reg32 ignored;
    if (!nested(cpu, [&] { return checkTty(app, cpu, fp, ignored); }))
        return false;
    result = fp;
    return true;
}

/* __allocfp's (clib/streamio/c/allocfp.c): a stream for fopen, under the
 * streams' lock -- the last closed's again; or one of __iob's 20 not in use,
 * a link from the heap for it; or a FILE and its link from the heap -- its
 * FILE cleared, in use, its link first on __OpenStreams; 0 and ENOMEM for
 * none.  The lock is given back with eax as its code leaves it. */
bool allocateStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32& fp)
{
    const Frame frame(cpu, 0x14);  // ebx, ecx, edx, esi, edi
    if (!hook<kAccessIob>(app, cpu, {eax}))  // eax as its caller has it
        return false;
    x86::reg32 link = at(app, kClosedStreams), flag;
    if (link != 0)
    {
        fp = at(app, link + kLinkStream);
        flag = (at(app, fp + kFlag) & (kDynamic | kReading | kWriting)) | kReading | kWriting;
        put(app, kClosedStreams, at(app, link + kLinkNext));
    }
    else
    {
        fp = 0;
        for (x86::reg32 iob = kIobStart; iob < kIobEnd; iob += kFileSize)
            if (!(byte(app, iob + kFlag) & (kReading | kWriting)))
            {
                fp = iob;
                break;
            }
        if (!called(cpu, [&] {
                return watcomheap::allocate(app, cpu, kGameHeap, fp != 0 ? kLinkSize : kLinkSize + kFileSize, link);
            }))
            return false;
        flag = fp != 0 ? kReading | kWriting : kDynamic | kReading | kWriting;
        if (link == 0)
        {
            fp = 0;
            x86::reg32 data;
            return setThreadWord(app, cpu, kErrno, kENOMEM, &data) && hook<kReleaseIob>(app, cpu, {data});
        }
        if (fp == 0)
            fp = link + kLinkSize;
    }
    clib::memset(app, fp, 0, kFileSize);
    put(app, fp + kFlag, flag);
    put(app, link + kLinkStream, fp);
    const x86::reg32 head = at(app, kOpenStreams);
    put(app, fp + kLink, link);
    put(app, kOpenStreams, link);
    put(app, link + kLinkNext, head);
    return hook<kReleaseIob>(app, cpu, {head}).has_value();
}

/* __freefp's: the stream's link off __OpenStreams and first on the closed
 * ones', its FILE marked in use still; the link, or 0 for a stream not
 * open. */
x86::reg32 freeStream(win32::WinApplication* app, x86::reg32 fp)
{
    for (x86::reg32 before = kOpenStreams, link = at(app, before); link != 0; before = link, link = at(app, link))
    {
        const x86::reg32 stream = at(app, link + kLinkStream);
        if (stream != fp)
            continue;
        putByte(app, fp + kFlag, byte(app, stream + kFlag) | kReading | kWriting);
        put(app, before, at(app, link + kLinkNext));
        const x86::reg32 closed = at(app, kClosedStreams);
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
bool doClose(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32 handleToo, x86::reg32& result)
{
    const Frame frame(cpu, 0x128);  // ebx, ecx, esi, edi, ebp; a path
    const x86::reg32 path = cpu.esp;
    result = 0xffffffff;
    if (at(app, fp + kFlag) == 0)
        return true;
    result = 0;
    if ((flags(app, fp) & kDirty) && !nested(cpu, [&] { return flushStream(app, cpu, fp, result); }))
        return false;
    if (!streamLock(app, cpu, kAccessFile, fp))
        return false;
    x86::reg32 place;
    if (!nested(cpu, [&] { return tellStream(app, cpu, fp, place); }))
        return false;
    if (place != 0xffffffff)
    {
        x86::reg32 ignored;
        if (!called(cpu, [&] { return lseekOf(app, cpu, at(app, fp + kFileHandle), place, 0, ignored); }))
            return false;
    }
    if (handleToo != 0)
    {
        x86::reg32 closed;
        if (!nested(cpu, [&] { return closeHandle(app, cpu, at(app, fp + kFileHandle), closed); }))
            return false;
        result |= closed;
    }
    if (byte(app, fp + kFlag) & kBigBuf)
    {
        if (!called(cpu, [&] { return watcomheap::free(app, cpu, kGameHeap, bufferOf(app, fp)); }))
            return false;
        put(app, linkOf(app, fp) + kLinkBase, 0);
    }
    if (flags(app, fp) & kTmpFile)
    {
        guest::direct<kMkTmpFile, clib::mkTmpFile>(app, cpu, path, x86::reg32(byte(app, linkOf(app, fp) + kTmpfChar)));
        if (cpu.terminate)
            return false;
        x86::reg32 ignored;
        if (!called(cpu, [&] { return unlinkOf(app, cpu, path, ignored); }))
            return false;
    }
    if (!streamLock(app, cpu, kReleaseFile, fp))
        return false;
    return handleToo == 0 || hook<kFreeHandle>(app, cpu, {at(app, fp + kFileHandle)});
}

/* __shutdown_stream's: __doclose's, and __freefp. */
bool shutdownStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32 handleToo,
                    x86::reg32& result)
{
    const Frame frame(cpu, 4);  // ebx
    if (!nested(cpu, [&] { return doClose(app, cpu, fp, handleToo, result); }))
        return false;
    freeStream(app, fp);
    return true;
}

/* fclose's: a stream on __OpenStreams shut down, its handle closed; -1 for
 * one not there.  The streams' lock given back with eax its link, or 0. */
bool closeStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp, x86::reg32& result)
{
    const Frame frame(cpu, 8);  // ebx, edx
    if (!hook<kAccessIob>(app, cpu, {fp}))
        return false;
    x86::reg32 link = at(app, kOpenStreams);
    while (link != 0 && at(app, link + kLinkStream) != fp)
        link = at(app, link + kLinkNext);
    if (!hook<kReleaseIob>(app, cpu, {link}))
        return false;
    result = 0xffffffff;
    return link == 0 || nested(cpu, [&] { return shutdownStream(app, cpu, fp, 1, result); });
}

/* _fsopen's (clib/streamio/c/fopen.c): the mode's flags, a stream, and
 * __doopen, its extended flags a dword of the frame; 0 for a mode that is
 * none or no stream. */
bool openStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 mode, x86::reg32 shflag,
                x86::reg32& result)
{
    const Frame frame(cpu, 0x10);  // ecx, esi, edi; a dword
    const x86::reg32 extflags = cpu.esp;
    x86::reg32 flags, fp;
    if (!nested(cpu, [&] { return openFlags(app, cpu, mode, extflags, flags); }))
        return false;
    result = 0;
    if (flags == 0)
        return true;
    if (!nested(cpu, [&] { return allocateStream(app, cpu, 0, fp); }))
        return false;
    if (fp == 0)
        return true;
    const Frame arguments(cpu, 8);  // shflag and the stream, __doopen's on the stack
    return nested(cpu, [&] {
        return doOpen(app, cpu, name, byte(app, mode), flags, at(app, extflags), shflag, fp, result);
    });
}

}  // namespace

/* fsync (clib/handleio/c/fsync.c, 0x522d80): handle eax's buffers to the
 * disk; 0, or -1. */
x86::reg32 fsync(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!syncHandle(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __chktty (clib/streamio/c/chktty.c, 0x509380): the stream at eax marked a
 * tty's, if it is one. */
x86::reg32 chktty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 eax;
    if (!checkTty(app, cpu, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(cpu, args, eax);
    return args.eax;
}

/* __ioalloc (clib/streamio/c/ioalloc.c, 0x5029c0): a buffer for the stream at
 * eax. */
x86::reg32 ioalloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 eax;
    if (!allocateBuffer(app, cpu, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(cpu, args, eax);
    return args.eax;
}

/* __flush (clib/streamio/c/flush.c, 0x502760): the stream at eax's buffer
 * written, or given back; 0, or -1. */
x86::reg32 flush(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!flushStream(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fputc (clib/streamio/c/fputc.c, 0x5097b0): eax's character to the stream
 * at edx. */
x86::reg32 fputc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!putChar(app, cpu, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fputs (clib/streamio/c/fputs.c, 0x4f7d80): the string at eax to the stream
 * at edx. */
x86::reg32 fputs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!putString(app, cpu, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fwrite (clib/streamio/c/fwrite.c, 0x4eebd0): ebx of edx bytes from eax to
 * the stream at ecx. */
x86::reg32 fwrite(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!writeStream(app, cpu, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __flushall (clib/streamio/c/flushall.c, 0x515858): the open streams with
 * a bit of eax flushed; how many. */
x86::reg32 flushall_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 count;
    if (!flushAll(app, cpu, args.eax, count))
        return args.eax;
    kept.back(args);
    returns(cpu, args, count);
    return args.eax;
}

/* flushall (clib/streamio/c/flushall.c, 0x515850): all of them. */
x86::reg32 flushall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 count;
    if (!flushAll(app, cpu, 0xffffffff, count))
        return args.eax;
    kept.back(args);
    returns(cpu, args, count);
    return args.eax;
}

/* fflush (clib/streamio/c/fflush.c, 0x4f7d70): the stream at eax flushed --
 * or, for none, all of them, and 0. */
x86::reg32 fflush(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (args.eax != 0)
        return flush(app, cpu, args);
    const Kept kept(args);
    x86::reg32 count;
    if (!nested(cpu, [&] { return flushAll(app, cpu, 0xffffffff, count); }))
        return args.eax;
    kept.back(args);
    returns(cpu, args, 0);
    return args.eax;
}

/* __update_buffer (clib/streamio/c/fseek.c, 0x4e9030): a seek of eax from
 * where the stream at edx is, within its buffer: 0, or 1 for one outside. */
x86::reg32 update_buffer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, updateBuffer(app, args.eax, args.edx));
    return args.eax;
}

/* __reset_buffer (clib/streamio/c/fseek.c, 0x4e9070): the stream at eax's
 * buffer emptied; eax left the stream. */
x86::reg32 reset_buffer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    resetBuffer(app, args.eax);
    returns(cpu, args, args.eax);
    return args.eax;
}

/* __fill_buffer (clib/streamio/c/fgetc.c, 0x502be8): the stream at eax's
 * buffer filled; how many are in it. */
x86::reg32 fill_buffer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 count;
    if (!fillBuffer(app, cpu, args.eax, count))
        return args.eax;
    kept.back(args);
    returns(cpu, args, count);
    return args.eax;
}

/* __filbuf (clib/streamio/c/fgetc.c, 0x502bb8): the stream at eax's buffer
 * filled, and its first byte; -1 for none. */
x86::reg32 filbuf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 c;
    if (!fillAndTake(app, cpu, args.eax, c))
        return args.eax;
    kept.back(args);
    returns(cpu, args, c);
    return args.eax;
}

/* fgetc (clib/streamio/c/fgetc.c, 0x502af0): a byte of the stream at eax,
 * or -1. */
x86::reg32 fgetc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!getChar(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fread (clib/streamio/c/fread.c, 0x4e9200): ebx of edx bytes into eax from
 * the stream at ecx. */
x86::reg32 fread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!readStream(app, cpu, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fgets (clib/streamio/c/fgets.c, 0x4f0570): a line of the stream at ebx
 * into eax, edx bytes at most with its 0; eax, or 0. */
x86::reg32 fgets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!getString(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fseek (clib/streamio/c/fseek.c, 0x4e9088): the stream at eax to edx, from
 * ebx's whence; 0, or -1. */
x86::reg32 fseek(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!seekStream(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* ftell (clib/streamio/c/ftell.c, 0x4ee420): where the stream at eax is, or
 * -1. */
x86::reg32 ftell(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!tellStream(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __GetNTAccessAttr (clib/streamio/c/ntfio.c, 0x5234d0): CreateFileA's
 * access to edx's dword and attributes to ebx's for eax's read and write
 * mode. */
x86::reg32 GetNTAccessAttr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    accessAttributes(app, args.eax, args.edx, args.ebx);
    returns(cpu, args, args.eax);
    return args.eax;
}

/* __GetNTShareAttr (clib/streamio/c/ntfio.c, 0x523504): CreateFileA's
 * sharing to edx's dword for eax's sharing and mode; eax left the sharing
 * bits. */
x86::reg32 GetNTShareAttr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, shareAttributes(app, args.eax, args.edx));
    return args.eax;
}

/* 0x51e2b0, _sopen's: whether __OSHandles has no slot free. */
x86::reg32 handlesFull(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, handlesFull(app));
    return args.eax;
}

/* __NTGetFakeHandle (clib/handleio/c/hdlman.c, 0x51e4f0): a HANDLE for a
 * handle with none. */
x86::reg32 NTGetFakeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 handle;
    if (!fakeHandle(app, cpu, handle))
        return args.eax;
    kept.back(args);
    returns(cpu, args, handle);
    return args.eax;
}

/* __allocPOSIXHandle (clib/handleio/c/hdlman.c, 0x51e304): a handle for the
 * HANDLE at eax. */
x86::reg32 allocPOSIXHandle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 handle;
    if (!allocHandle(app, cpu, args.eax, handle))
        return args.eax;
    kept.back(args);
    returns(cpu, args, handle);
    return args.eax;
}

/* __freePOSIXHandle (clib/handleio/c/hdlman.c, 0x51e45c): handle eax's slot
 * free. */
x86::reg32 freePOSIXHandle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 eax;
    if (!freeHandle(app, cpu, args.eax, eax))
        return args.eax;
    kept.back(args);
    returns(cpu, args, eax);
    return args.eax;
}

/* __close (clib/handleio/c/_clsewnt.c, 0x5095b0): handle eax closed; 0, or
 * -1. */
x86::reg32 close(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!closeHandle(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __sopen (clib/handleio/c/openwnt.c, 0x50901c): eax opened with edx's
 * flags and ebx's sharing, the permissions from the va_list at ecx. */
x86::reg32 sopen_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!openHandle(app, cpu, args.eax, args.edx, args.ebx, args.ecx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* _sopen (clib/handleio/c/openwnt.c, 0x508ff4): cdecl (name, flags,
 * sharing, ...); __sopen's, with a va_list of its frame for the rest. */
x86::reg32 sopen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!sopenAt(app, cpu, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __open_flags (clib/streamio/c/fopen.c, 0x4edd80): the mode string at eax's
 * flags, its extended ones to edx's dword. */
x86::reg32 open_flags(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!openFlags(app, cpu, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __doopen (clib/streamio/c/fopen.c, 0x4edebc): eax opened as dl's mode
 * says, with ebx's flags and ecx's extended ones, into the stream on the
 * stack after its sharing; takes them off. */
x86::reg32 doopen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    const x86::reg32 entry = cpu.esp;
    x86::reg32 result;
    if (!doOpen(app, cpu, args.eax, x86::reg8(args.edx), args.ebx, args.ecx, at(app, entry + 4), at(app, entry + 8),
                result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    cpu.esp += 8;
    return args.eax;
}

/* __allocfp (clib/streamio/c/allocfp.c, 0x509250): a stream, or 0. */
x86::reg32 allocfp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 fp;
    if (!allocateStream(app, cpu, args.eax, fp))
        return args.eax;
    kept.back(args);
    returns(cpu, args, fp);
    return args.eax;
}

/* __freefp (clib/streamio/c/allocfp.c, 0x509314): the stream at eax off the
 * open ones; its link, or 0. */
x86::reg32 freefp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, freeStream(app, args.eax));
    return args.eax;
}

/* __doclose (clib/streamio/c/fclose.c, 0x4ee214): the stream at eax given
 * up, its handle closed too if edx says so. */
x86::reg32 doclose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!doClose(app, cpu, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __shutdown_stream (clib/streamio/c/fclose.c, 0x4ee144): the stream at eax
 * given up and freed. */
x86::reg32 shutdown_stream(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!shutdownStream(app, cpu, args.eax, args.edx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fclose (clib/streamio/c/fclose.c, 0x4ee100): the stream at eax closed; 0,
 * or -1. */
x86::reg32 fclose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!closeStream(app, cpu, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* _fsopen (clib/streamio/c/fopen.c, 0x4edfa4): eax opened as the mode at edx
 * says, with ebx's sharing; the stream, or 0. */
x86::reg32 fsopen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!openStream(app, cpu, args.eax, args.edx, args.ebx, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* fopen (clib/streamio/c/fopen.c, 0x4edfe8): _fsopen's, sharing as it may. */
x86::reg32 fopen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    {
        const Frame frame(cpu, 4);  // ebx
        if (!nested(cpu, [&] { return openStream(app, cpu, args.eax, args.edx, 0, result); }))
            return args.eax;
    }
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* ---- The heap (native_watcom_heap.cpp has it; these are the game's calls
 * into it, each keeping the registers its code kept: all but eax, but for
 * what __MemAllocator and __MemFree are handed). ---- */

/* _nmalloc (clib/heap/c/nmalloc.c, 0x4f7900): eax bytes; the block, or 0. */
x86::reg32 nmalloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!watcomheap::allocate(app, cpu, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* _nfree (clib/heap/c/nfree.c, 0x4f79f0): the block at eax given back. */
x86::reg32 nfree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!watcomheap::free(app, cpu, kGameHeap, args.eax))
        return args.eax;
    const x86::reg32 eax = args.eax;
    kept.back(args);
    returns(cpu, args, eax);
    return args.eax;
}

/* __MemAllocator (clib/heap/c/mem.c, 0x515300): eax bytes from the miniheap at
 * ebx (edx the segment, nothing on a flat heap); the block, or 0. */
x86::reg32 MemAllocator(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, watcomheap::allocateIn(app, args.ebx, args.eax));
    return args.eax;
}

/* __MemFree (clib/heap/c/mem.c, 0x5153b0): the block at eax given back to the
 * miniheap at ebx. */
x86::reg32 MemFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    watcomheap::freeIn(app, args.ebx, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* __LinkUpNewNHeap (clib/heap/c/grownear.c, 0x5154e0): the new miniheap at eax
 * into the list; its first block. */
x86::reg32 LinkUpNewNHeap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, watcomheap::link(app, kGameHeap, args.eax));
    return args.eax;
}

/* __AdjustAmount (clib/heap/c/grownear.c, 0x515628): the amount at eax made
 * what a miniheap for it takes, in place; 1, or 0. */
x86::reg32 AdjustAmount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 amount = at(app, args.eax);
    const bool adjusted = watcomheap::adjust(app, kGameHeap, amount);
    put(app, args.eax, amount);
    returns(cpu, args, adjusted ? 1 : 0);
    return args.eax;
}

/* __CreateNewNHeap (clib/heap/c/grownear.c, 0x515558): a miniheap with room
 * for eax bytes; 1, or 0. */
x86::reg32 CreateNewNHeap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!watcomheap::create(app, cpu, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __ExpandDGROUP (clib/heap/c/grownear.c, 0x515614): the empty miniheaps given
 * back, then a new one with room for eax bytes; 1, or 0. */
x86::reg32 ExpandDGROUP(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!watcomheap::expand(app, cpu, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* __nmemneed (clib/heap/c/nmemneed.c, 0x515680): whether the caller could give
 * memory back for eax bytes more -- no. */
x86::reg32 nmemneed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    returns(cpu, args, 0);
    return args.eax;
}

/* _nheapshrink (clib/heap/c/nheapmin.c, 0x5241c0): every empty miniheap given
 * back to the system; 0. */
x86::reg32 nheapshrink(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    if (!watcomheap::shrink(app, cpu, kGameHeap))
        return args.eax;
    kept.back(args);
    returns(cpu, args, 0);
    return args.eax;
}

/* __ReturnMemToSystem (clib/heap/c/nheapmin.c, 0x5241fc): the miniheap at eax
 * to VirtualFree, and the rovers off it; 0, or -1. */
x86::reg32 ReturnMemToSystem(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!watcomheap::giveBack(app, cpu, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
    return args.eax;
}

/* 0x52425c, _nheapshrink's step (clib/heap/c/nheapmin.c, __UnlinkNHeap after
 * __ReturnMemToSystem): the miniheap at eax given back and, if it went, taken
 * off the list; 0, or -1. */
x86::reg32 ReleaseNHeap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    x86::reg32 result;
    if (!watcomheap::release(app, cpu, kGameHeap, args.eax, result))
        return args.eax;
    kept.back(args);
    returns(cpu, args, result);
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

using x86::reg16;
using x86::reg32;
using x86::reg8;

/* SPECS, as __prtf lays it out at its esp. */
constexpr reg32 kDest = 0x00;      // where sprintf writes, the FILE for fprintf
constexpr reg32 kWidth = 0x04;     // _fld_width
constexpr reg32 kPrec = 0x08;      // _prec: -1 for none
constexpr reg32 kOutCount = 0x10;  // how many characters went out
constexpr reg32 kCharacter = 0x15; // the conversion's
constexpr reg32 kPadChar = 0x16;   // ' ' or '0'
constexpr reg32 kSpecs1C = 0x1c;   // a word __prtf sets to 100
constexpr reg32 kFlags = 0x1e;     // '#' 1, ' ' 2, '+' 4, '-' 8, h 0x10, l 0x20, N 0x40, F 0x80
constexpr reg32 kFlags2 = 0x1f;    // I64 or L: 1
constexpr reg32 kN0 = 0x20, kNZ0 = 0x24, kN1 = 0x28, kNZ1 = 0x2c, kN2 = 0x30, kNZ2 = 0x34;

constexpr reg8 kAlternate = 0x01, kBlank = 0x02, kPlus = 0x04, kLeft = 0x08;
constexpr reg8 kShort = 0x10, kLong = 0x20, kNear = 0x40, kFar = 0x80;
constexpr reg8 kLongLong = 0x01;

constexpr reg32 kFloatFormat = 0x56ac64;  // __EFG_printf
constexpr reg32 kDigits = 0x56ac3c;       // "0123456789abcdefghijklmnopqrstuvwxyz": utoa's
constexpr reg32 kDigits64 = 0x56ac6c;     // ulltoa's
constexpr reg32 kDigitsLow = 0x56ac94;    // __prtf's own utoa's
constexpr reg32 kMbcsOn = 0xa0b200;       // whether multibyte characters are on
constexpr reg32 kMbcsLead = 0xa0b211;     // a byte a character: 1 for a lead byte
constexpr reg32 kCodePage = 0x56b0b8;     // the code page wctomb hands Windows
constexpr reg32 kWideCharToMultiByte = 0x534638;

constexpr reg32 kMemPutc = 0x4df660;
constexpr reg32 kFilePutc = 0x4ff000;

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

/* What the segment registers and the direction flag are to __prtf's code, which reads through es and
 * ds and runs string instructions: ees's high word (a mov es leaves it), ds, es as the caller has
 * it, and df.  The game's are all 0 -- flat memory, df clear; the
 * thunks read them from the cpu (segmentsOf) and hand them down, a typed caller has the game's. */
struct Segments
{
    reg32 high = 0;
    reg16 ds = 0;
    reg16 es = 0;
    bool df = false;

    // Where es with the segment `segment` puts an offset: es is ees's low word.
    reg32 base(reg32 segment) const { return high | reg16(segment); }
};

Segments segmentsOf(const x86::CPU& cpu)
{
    (void)cpu;
    return {0, 0, 0, false};  // the flat model's selectors, all 0 in the port; the direction flag: nothing sets it
}

/* What a function's push instructions leave below its return address: each
 * dword in turn, and es's word where it has one (-1 for none), the dword's
 * other half left as it was.  Written as the code writes them, for what reads
 * the stack where they are -- a wide string from __prtf's buffer goes on into
 * its frame.  (Plan step 7: the typed cores keep the room of their pushes and
 * write none of them; what is left of these is the thunks'.) */
void pushed(win32::WinApplication* app, reg32 entry, std::initializer_list<reg32> registers, int esAt = -1,
            reg16 es = 0)
{
    reg32 at_ = entry;
    int i = 0;
    for (const reg32 r : registers)
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
reg32 nextArgument(win32::WinApplication* app, reg32 argp, reg32 size, reg32& moved)
{
    moved = at(app, argp) + size;
    put(app, argp, moved);
    return at(app, moved - size);
}

}  // namespace

/* 0x51e060 (wctomb's core): the wide character `wc` as WideCharToMultiByte makes it in the game's
 * code page, into the two bytes at `to`; how many, or -1.  A null buffer is 0.  At its entry's
 * depth: ecx pushed, then the character, which Windows is handed.  0 with cpu.terminate set when
 * the game stopped. */
reg32 wctombOf(win32::WinApplication* app, x86::CPU& cpu, reg32 to, reg32 wc)
{
    const GuestFrame frame(cpu, 4, 4);
    const reg32 character = frame - 4;
    put(app, character, wc);
    if (to == 0)
        return 0;
    const EspKept kept(cpu);
    const auto n = windowsAt<kWideCharToMultiByte, &host::wideToMultiByte>(app, cpu, to, at(app, kCodePage), 0x200,
                                                                           character, 1, to, 2, 0, 0);
    if (!n)
        return 0;
    return *n == 0 ? 0xffffffff : *n;
}

x86::reg32 wctomb_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = wctombOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* `value`'s digits in `base` from the table `digits`, made backwards in the frame's 0x28 bytes under
 * the four registers utoa's code pushes, then copied out to `out` the right way round: utoa's and
 * __prtf's own, which keep a different word in the frame's 0x24th. */
reg32 digitsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 value, reg32 base, reg32 out, reg32 digits,
               bool keepsRest)
{
    const GuestFrame frame(cpu, 0x10, 0x28);
    const reg32 buffer = frame - 0x28;
    putByte(app, buffer, 0);
    reg32 p = buffer + 1;
    do
    {
        put(app, buffer + 0x24, base);
        const reg32 rest = value % base;
        value /= base;
        put(app, buffer + 0x24, keepsRest ? rest : value);
        putByte(app, p++, byte(app, digits + rest));
    } while (value != 0);
    reg32 to = out;
    reg8 c;
    do
    {
        c = byte(app, --p);
        putByte(app, to++, c);
    } while (c != 0);
    return out;
}

/* 0x51e0a0 (utoa's core, clib/convert/c/itoa.c): `value` in `base`, its digits from kDigits, into
 * `out`; `out` back. */
reg32 utoaOf(win32::WinApplication* app, x86::CPU& cpu, reg32 value, reg32 out, reg32 base)
{
    return digitsOf(app, cpu, value, base, out, kDigits, false);
}

x86::reg32 utoa(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, utoaOf(app, cpu, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x51e0ec (itoa, clib/convert/c/itoa.c): `value` signed in base 10 -- a '-' first -- unsigned in
 * any other, into `out`; `out` back.  At its entry's depth: ecx pushed. */
reg32 itoaOf(win32::WinApplication* app, x86::CPU& cpu, reg32 value, reg32 out, reg32 base)
{
    const GuestFrame frame(cpu, 4, 0);
    reg32 to = out;
    if (base == 10 && x86::sreg32(value) < 0)
    {
        value = 0 - value;
        putByte(app, to++, '-');
    }
    const ReturnSlot slot(cpu);
    utoaOf(app, cpu, value, to, base);
    return out;
}

x86::reg32 itoa(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, itoaOf(app, cpu, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525d59 (__U8D, the compiler's unsigned 64-bit division): `n` by `d`, the remainder into
 * `rest`. */
x86::reg64 u8dOf(x86::reg64 n, x86::reg64 d, x86::reg64& rest)
{
    rest = d == 0 ? n : n % d;
    return d == 0 ? 0 : n / d;
}

x86::reg32 U8D(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    // Its own convention: edx:eax by ecx:ebx, the quotient in edx:eax, the remainder in ecx:ebx;
    // the long division's pushes where it takes them.
    if (args.ecx != 0 && args.ecx < args.edx)
        pushed(app, cpu.esp, {args.ebp, args.esi, args.edi});
    x86::reg64 rest;
    const x86::reg64 q = u8dOf(x86::reg64(args.edx) << 32 | args.eax, x86::reg64(args.ecx) << 32 | args.ebx, rest);
    args.eax = reg32(q);
    args.edx = reg32(q >> 32);
    args.ebx = reg32(rest);
    args.ecx = reg32(rest >> 32);
    cpu.esp += 4;
    return args.eax;
}

/* 0x51e130 (ulltoa, clib/convert/c/lltoa.c): the unsigned 64 bits at `value` in `base`, its digits
 * from kDigits64, into `out`; `out` back -- a digit at a time by __U8D, the number, the quotient
 * and the remainder in its frame: ecx, esi, edi, es and ebp pushed (their room only), then 0x44
 * bytes. */
reg32 ulltoaOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 value, reg32 out, reg32 base)
{
    const GuestFrame frame(cpu, 0x14, 0x44);
    const reg32 f = frame - 0x44;
    put(app, f + 0x3c, out);
    put(app, f + 0x40, out);
    // Two movsd, es its ds, as the direction flag has them.
    const reg32 step = s.df ? reg32(-4) : 4;
    const reg32 es = s.base(s.ds);
    put(app, es + f + 0x34, at(app, value));
    put(app, es + f + 0x34 + step, at(app, value + step));
    put(app, f + 0x24, base);
    put(app, f + 0x28, 0);
    putByte(app, f, 0);
    reg32 p = f + 1;
    x86::reg64 q;
    do
    {
        x86::reg64 rest;
        q = u8dOf(x86::reg64(at(app, f + 0x38)) << 32 | at(app, f + 0x34),
                  x86::reg64(at(app, f + 0x28)) << 32 | at(app, f + 0x24), rest);
        put(app, f + 0x38, reg32(q >> 32));
        put(app, f + 0x34, reg32(q));
        put(app, f + 0x30, reg32(rest >> 32));
        put(app, f + 0x2c, reg32(rest));
        putByte(app, p++, byte(app, at(app, f + 0x2c) + kDigits64));
    } while (q != 0);
    reg8 c;
    do
    {
        const reg32 to = at(app, f + 0x40);
        c = byte(app, --p);
        putByte(app, to, c);
        put(app, f + 0x40, to + 1);
    } while (c != 0);
    return at(app, f + 0x3c);
}

x86::reg32 ulltoa(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, ulltoaOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x51e220 (__prtf's utoa): `value` in `base`, its digits from kDigitsLow, into `out`; `out`
 * back. */
reg32 prtfUtoaOf(win32::WinApplication* app, x86::CPU& cpu, reg32 value, reg32 out, reg32 base)
{
    return digitsOf(app, cpu, value, base, out, kDigitsLow, true);
}

x86::reg32 prtfUtoa(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfUtoaOf(app, cpu, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fefdc (__prtf's strupr): the string at `p` in capitals, by toupper.  What the code leaves in
 * eax: toupper's last, `p` for an empty string. */
reg32 prtfUpperOf(win32::WinApplication* app, x86::CPU& cpu, reg32 p)
{
    NFS2_USE(cpu);  // ebx and edx pushed: their room only
    reg32 last = p;
    if (byte(app, p) != 0)
        do
        {
            last = clib::toupper(app, byte(app, p));
            putByte(app, p, reg8(last));
        } while (byte(app, ++p) != 0);
    return last;
}

x86::reg32 prtfUpper(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfUpperOf(app, cpu, args.eax));
    return args.eax;
}

/* 0x4fe734 (__prtf's strnlen): the length of the string at `p` in the segment `segment`, which the
 * code loads into es, at most `limit` (-1 for any).  Its pushes: their room only. */
reg32 prtfLengthOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 p, reg32 segment,
                   reg32 limit)
{
    NFS2_USE(cpu);
    const reg32 base = s.base(segment);
    reg32 length = 0;
    while (byte(app, base + p++) != 0 && length != limit)
        ++length;
    return length;
}

x86::reg32 prtfLength(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfLengthOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fe758 (__prtf's wcsnlen in bytes): how many bytes the wide string at `p` in the segment
 * `segment` is as wctomb makes it, at most `limit` of them (-1 for any).  At its entry's depth: ecx, esi and es pushed, then the
 * character's bytes.  0 with cpu.terminate set when the game stopped. */
reg32 prtfWideLengthOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 p, reg32 segment,
                       reg32 limit)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 base = s.base(segment);
    const reg32 bytes = frame - 4;
    auto over = [&](reg32 count) { return limit != 0xffffffff && x86::sreg32(count) > x86::sreg32(limit); };
    reg32 count = 0;
    for (;;)
    {
        const reg16 wc = half(app, base + p);
        if (wc == 0 || over(count))
            break;
        p += 2;
        reg32 n;
        {
            const ReturnSlot slot(cpu);
            n = wctombOf(app, cpu, bytes, wc);
        }
        if (cpu.terminate)
            return 0;
        if (n != 0xffffffff)
            count += n;
    }
    return over(count) ? limit : count;
}

x86::reg32 prtfWideLength(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = prtfWideLengthOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* 0x4fe7c4 (__prtf's pointer's digits): `value` in hexadecimal, right-aligned in `width` digits
 * with zeros before, into `out`.  At its entry's depth: four pushes, then the width.  What the code
 * leaves in eax: the count of zeros it ran down, -1. */
reg32 prtfHexOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 value, reg32 width, reg32 out)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 kept = frame - 4;
    put(app, kept, width);
    {
        const ReturnSlot slot(cpu);
        itoaOf(app, cpu, value, out, 0x10);
    }
    // repne scasb from it, es its ds, as the direction flag has it -- es pushed meanwhile (its room).
    reg32 count = 0xffffffff, scan = out;
    reg8 scanned;
    do
    {
        scanned = byte(app, s.base(s.ds) + scan);
        scan += s.df ? reg32(-1) : 1;
        --count;
    } while (scanned != 0 && count != 0);
    const reg32 length = ~count - 1;
    reg32 left = width - 1;
    if (length != 0)
    {
        reg32 from = out + length, to = left + out;
        do
        {
            --to;
            putByte(app, to + 1, byte(app, --from));
            --left;
        } while (from != out);
    }
    for (reg32 p = left + out; x86::sreg32(left) >= 0; --p)
    {
        --left;
        putByte(app, p, '0');
    }
    putByte(app, out + at(app, kept), 0);
    return left;
}

x86::reg32 prtfHex(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfHexOf(app, cpu, segmentsOf(cpu), args.eax, args.ebx, args.edx));
    return args.eax;
}

/* 0x4fe81e: the end of __prtf's %p digits (0x4fe7c4), which its %hf
 * (0x4fe828) jumps to as well: the frame's dword let go and the registers
 * the two keep -- ebp, edi, esi, ecx -- taken back off the stack. */
x86::reg32 prtfHexEnd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    cpu.esp += 4;
    args.ebp = at(app, cpu.esp);
    args.edi = at(app, cpu.esp + 4);
    args.esi = at(app, cpu.esp + 8);
    args.ecx = at(app, cpu.esp + 0xc);
    cpu.esp += 0x10 + 4;
    return args.eax;
}

/* 0x4fe828 (__prtf's %hf): `value` a 16.16 fixed-point number, into `out` -- its whole part by
 * itoa, then the precision's digits (4 for none), rounded half up.  At its entry's depth: four
 * pushes, then the number.  What the code leaves in eax: where its digits went, past a '-'. */
reg32 prtfFixedOf(win32::WinApplication* app, x86::CPU& cpu, reg32 out, reg32 value, reg32 specs)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 local = frame - 4;
    reg32 p = out;
    put(app, local, value);
    if (x86::sreg32(value) < 0)
    {
        putByte(app, p, '-');
        ++p;
        put(app, local, 0 - value);
    }
    if (at(app, specs + kPrec) == 0xffffffff)
        put(app, specs + kPrec, 4);
    {
        const ReturnSlot slot(cpu);
        itoaOf(app, cpu, half(app, local + 2), p, 10);
    }
    const reg32 start = p;
    if (byte(app, p) != 0)
        while (byte(app, ++p) != 0)
        {
        }
    if (at(app, specs + kPrec) != 0)
    {
        putByte(app, p++, '.');
        for (reg32 i = 0; x86::sreg32(i) < x86::sreg32(at(app, specs + kPrec)); ++i)
        {
            putHalf(app, local + 2, 0);
            put(app, local, at(app, local) * 10);
            putByte(app, p++, reg8(byte(app, local + 2) + '0'));
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
                const reg8 point = byte(app, p);
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
            const reg8 digit = byte(app, p);
            if (digit != '9')
            {
                putByte(app, p, reg8(digit + 1));
                break;
            }
            putByte(app, p, '0');
        }
    }
    return start;
}

x86::reg32 prtfFixed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfFixedOf(app, cpu, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4fe924 (__prtf's float): [__EFG_printf] with eax the buffer, edx the arguments' pointer, ebx
 * the SPECS -- the start-up code's formatter (watc::formatFloat, by its hook: watcom_hooks.h),
 * called with the registers it takes; what it leaves in eax, 0 with cpu.terminate set when the
 * game stopped. */
reg32 prtfFloatOf(win32::WinApplication* app, x86::CPU& cpu, reg32 buffer, reg32 argp, reg32 specs)
{
    const EspKept kept(cpu);
    return hook<kFloatFormat>(app, cpu, {buffer, argp, specs}).value_or(0);
}

x86::reg32 prtfFloat(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (!hookWith<kFloatFormat>(app, cpu, args))
        return args.eax;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4fe92c (__prtf's zero padding): with '0' padding and not '-', what the width leaves over the
 * parts of the SPECS at `specs` as zeros after the first. */
void prtfZerosOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs)
{
    NFS2_USE(cpu);  // six pushes: their room only
    if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == '0')
    {
        const reg32 nz0 = at(app, specs + kNZ0);
        const reg32 left = at(app, specs + kWidth) - at(app, specs + kN0) - nz0 - at(app, specs + kN1)
                           - at(app, specs + kNZ1) - at(app, specs + kN2) - at(app, specs + kNZ2);
        if (x86::sreg32(left) > 0)
            put(app, specs + kNZ0, nz0 + left);
    }
}

x86::reg32 prtfZeros(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    prtfZerosOf(app, cpu, args.eax);  // eax left as it was, the SPECS
    cpu.esp += 4;
    return args.eax;
}

/* 0x4fe6cc (__prtf's flags): the flags at `p` into the SPECS at `specs`; past them. */
reg32 prtfFlagsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 p, reg32 specs)
{
    NFS2_USE(cpu);  // ebx and ecx pushed: their room only
    putHalf(app, specs + kFlags, 0);
    for (;; ++p)
    {
        const reg8 c = byte(app, p);
        const reg8 flags = byte(app, specs + kFlags);
        if (c == '-')
            putByte(app, specs + kFlags, flags | kLeft);
        else if (c == '#')
            putByte(app, specs + kFlags, flags | kAlternate);
        else if (c == '+')
            putByte(app, specs + kFlags, reg8((flags | kPlus) & ~kBlank));
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

x86::reg32 prtfFlags(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, prtfFlagsOf(app, cpu, args.eax, args.edx));
    return args.eax;
}

/* 0x4fe578 (__prtf's specification): what follows a '%' at `format` -- flags, width, precision,
 * size -- into the SPECS at `specs`, a '*' taken from the arguments' pointer at `argp`; past it.
 * `edx` what the code leaves there: the size's character in its low byte. */
reg32 prtfSpecOf(win32::WinApplication* app, x86::CPU& cpu, reg32 format, reg32 argp, reg32 specs, reg32& edx)
{
    const GuestFrame frame(cpu, 0xc, 0);
    putByte(app, specs + kPadChar, ' ');
    reg32 p;
    {
        const ReturnSlot slot(cpu);
        p = prtfFlagsOf(app, cpu, format, specs);
    }
    edx = specs;
    reg32 moved;
    auto digits = [&](reg32 field) {
        for (;;)
        {
            const reg8 c = byte(app, p);
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
        if (x86::sreg32(edx) < 0)
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
            if (x86::sreg32(edx) < 0)
                put(app, specs + kPrec, 0xffffffff);
            ++p;
        }
        else
            digits(kPrec);
        if (at(app, specs + kPrec) != 0xffffffff)
            putByte(app, specs + kPadChar, ' ');
    }
    const reg8 c = byte(app, p);
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

x86::reg32 prtfSpec(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 edx;
    const reg32 p = prtfSpecOf(app, cpu, args.eax, args.edx, args.ebx, edx);
    args.edx = edx;  // the size's character, as the code leaves it
    returns(cpu, args, p);
    return args.eax;
}

/* 0x4fe970 (__prtf's %ls): the wide string at `p` in the segment `segment` out by the function at
 * `out`, as wctomb makes
 * each character, as many bytes as the SPECS at `specs` left for it.  At its entry's depth: esi,
 * edi, es and ebp pushed (their room only), then the character's bytes.  What the code leaves in
 * eax; 0 with cpu.terminate set when the game stopped. */
reg32 prtfWideOutOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 p, reg32 segment,
                    reg32 specs, reg32 out)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 base = s.base(segment);
    const reg32 bytes = frame - 4;
    reg32 eax = p;
    if (x86::sreg32(at(app, specs + kN1)) <= 0)
        return eax;
    for (;;)
    {
        const reg16 wc = half(app, base + p);
        p += 2;
        reg32 n;
        {
            const ReturnSlot slot(cpu);
            n = wctombOf(app, cpu, bytes, wc);
        }
        if (cpu.terminate)
            return 0;
        eax = n;
        if (n != 0xffffffff)
        {
            if (x86::sreg32(n) > x86::sreg32(at(app, specs + kN1)))
            {
                put(app, specs + kN1, 0);
                break;
            }
            for (reg32 from = bytes; --n != 0xffffffff; ++from)
            {
                {
                    // The caller's function: the SPECS in eax, the byte in edx.
                    const EspKept kept(cpu);
                    if (!through(app, cpu, out, {specs, byte(app, from)}))
                        return 0;
                }
                eax = at(app, specs + kN1) - 1;
                put(app, specs + kN1, eax);
            }
        }
        if (x86::sreg32(at(app, specs + kN1)) <= 0)
            break;
    }
    return eax;
}

x86::reg32 prtfWideOut(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = prtfWideOutOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx, args.ecx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x4fe9d4 (__prtf's conversion): the argument the SPECS at `specs`'s conversion takes, by the
 * arguments' pointer at `argp`, made into the buffer at `buffer` -- a prefix (sign, 0x, 0) and the
 * parts' lengths in the SPECS; the string to write, its segment into `segment`.  At its entry's
 * depth: esi, edi, es and ebp pushed (their room only), then 0x10 bytes, the integer's argument and
 * wctomb's bytes.  0 with cpu.terminate set when the game stopped. */
reg32 prtfConvertOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 buffer, reg32 argp,
                    reg32 specs, reg16& segment)
{
    const GuestFrame frame(cpu, 0x10, 0x10);
    const reg32 local = frame - 0x10;
    reg32 moved;
    segment = s.ds;
    for (const reg32 field : {kN0, kNZ0, kN1, kNZ1, kN2})
        put(app, specs + field, 0);
    reg32 string = buffer;
    put(app, specs + kNZ2, 0);

    auto prefix = [&](reg8 c) {
        const reg32 n = at(app, specs + kN0);
        put(app, specs + kN0, n + 1);
        putByte(app, buffer + n, c);
    };
    // Each of the others' cores at this one's depth.
    auto length = [&](reg32 from, reg32 limit) {
        const ReturnSlot slot(cpu);
        return prtfLengthOf(app, cpu, s, from, s.ds, limit);
    };

    // The integer's argument: 64 bits at local, 32 at local + 8.
    const reg8 character = byte(app, specs + kCharacter);
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
                put(app, local + 8, reg32(x86::sreg32(x86::sreg16(half(app, local + 8)))));
        }
        const bool negative = byte(app, specs + kFlags2) & kLongLong ? (byte(app, local + 7) & 0x80) != 0
                                                                     : x86::sreg32(at(app, local + 8)) < 0;
        if (negative)
        {
            prefix('-');
            if (byte(app, specs + kFlags2) & kLongLong)
            {
                const reg32 low = ~at(app, local), high = ~at(app, local + 4);
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

    reg32 base = 10;
    reg32 result;
    switch (byte(app, specs + kCharacter))
    {
    case 'f':
    case 'F':
        if (byte(app, specs + kFlags) & kShort)
        {
            const reg32 value = nextArgument(app, argp, 4, moved);
            put(app, local + 8, value);
            {
                const ReturnSlot slot(cpu);
                prtfFixedOf(app, cpu, buffer, value, specs);
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
            const ReturnSlot slot(cpu);
            prtfFloatOf(app, cpu, buffer, argp, specs);
        }
        if (cpu.terminate)
            return 0;
        {
            const ReturnSlot slot(cpu);
            prtfZerosOf(app, cpu, specs);
        }
        segment = s.ds;
        return buffer + 1;
    case 's':
    case 'S':
    {
        putByte(app, buffer, 0);
        const reg8 flags = byte(app, specs + kFlags);
        if (flags & kFar)
        {
            const reg32 offset = nextArgument(app, argp, 8, moved);
            const reg16 far = half(app, moved - 4);
            if (offset != 0 || far != 0)
            {
                segment = far;
                string = offset;
            }
        }
        else
        {
            const reg32 pointer = nextArgument(app, argp, 4, moved);
            if (pointer != 0)
            {
                segment = s.ds;
                string = pointer;
            }
        }
        const bool wide = byte(app, specs + kCharacter) == 'S' ? !(byte(app, specs + kFlags) & kShort)
                                                               : (byte(app, specs + kFlags) & kLong) != 0;
        reg32 n;
        {
            const ReturnSlot slot(cpu);
            n = wide ? prtfWideLengthOf(app, cpu, s, string, segment, at(app, specs + kPrec))
                     : prtfLengthOf(app, cpu, s, string, segment, at(app, specs + kPrec));
        }
        if (cpu.terminate)
            return 0;
        const reg32 precision = at(app, specs + kPrec);
        put(app, specs + kN1, n);
        if (x86::sreg32(precision) >= 0 && x86::sreg32(n) > x86::sreg32(precision))
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
        const reg32 offset = nextArgument(app, argp, 4, moved);
        reg32 to = buffer;
        if (byte(app, specs + kFlags) & kFar)
        {
            const reg32 far = nextArgument(app, argp, 4, moved) & 0xffff;
            {
                const ReturnSlot slot(cpu);
                prtfHexOf(app, cpu, s, far, 4, buffer);
            }
            putByte(app, buffer + 4, ':');
            to = buffer + 5;
        }
        {
            const ReturnSlot slot(cpu);
            prtfHexOf(app, cpu, s, offset, 8, to);
        }
        if (byte(app, specs + kCharacter) == 'P')
        {
            const ReturnSlot slot(cpu);
            prtfUpperOf(app, cpu, buffer);
        }
        put(app, specs + kN0, length(string, 0xffffffff));
        return string;
    }
    case 'c':
    {
        put(app, specs + kN0, 1);
        if (byte(app, specs + kFlags) & kLong)
        {
            const reg32 wc = nextArgument(app, argp, 4, moved) & 0xffff;
            reg32 n;
            {
                const ReturnSlot slot(cpu);
                n = wctombOf(app, cpu, local + 0xc, wc);
            }
            if (cpu.terminate)
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
        putByte(app, buffer, reg8(nextArgument(app, argp, 4, moved)));
        return string;
    }
    case 'C':
    {
        const reg32 wc = nextArgument(app, argp, 4, moved) & 0xffff;
        reg32 n;
        {
            const ReturnSlot slot(cpu);
            n = wctombOf(app, cpu, buffer, wc);
        }
        if (cpu.terminate)
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
                const ReturnSlot slot(cpu);
                ulltoaOf(app, cpu, s, local, at(app, specs + kN0) + buffer, base);
            }
            if (byte(app, specs + kCharacter) == 'X')
            {
                const ReturnSlot slot(cpu);
                prtfUpperOf(app, cpu, buffer);
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
            const ReturnSlot slot(cpu);
            prtfUtoaOf(app, cpu, at(app, local + 8), at(app, specs + kN0) + buffer, base);
        }
        if (byte(app, specs + kCharacter) == 'X')
        {
            const ReturnSlot slot(cpu);
            prtfUpperOf(app, cpu, buffer);
        }
        result = length(string, 0xffffffff);
    }
    put(app, specs + kN1, result);
    const reg32 precision = at(app, specs + kPrec);
    if (x86::sreg32(result) < x86::sreg32(precision))
        put(app, specs + kNZ0, precision - result);
    if (at(app, specs + kPrec) == 0xffffffff)
    {
        const ReturnSlot slot(cpu);
        prtfZerosOf(app, cpu, specs);
    }
    return string;
}

x86::reg32 prtfConvert(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg16 segment;
    const reg32 string = prtfConvertOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx, segment);
    if (cpu.terminate)
        return args.eax;
    args.edx = segment;
    returns(cpu, args, string);
    return args.eax;
}

/* __prtf (0x4fe1d0): the format at `format` with the arguments' pointer at `argp`, each character
 * it makes handed to the function at `out` with eax its SPECS, whose first dword is `dest`; how
 * many went.  At its entry's depth: esi, edi, es and ebp pushed, then the SPECS' 0x70 bytes, which
 * the functions it calls are handed (the pushes: their room only).  0 with cpu.terminate set when
 * the game stopped. */
reg32 prtfOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 dest, reg32 format, reg32 argp,
             reg32 out)
{
    constexpr reg32 kArgCopy = 0x60, kArgCopy2 = 0x64, kFormat = 0x68, kBuffer = 0x38;
    constexpr reg32 kNullChar = 0x6c;
    const GuestFrame frame(cpu, 0x10, 0x70);
    const reg32 specs = frame - 0x70;
    reg16 es = s.es;  // the last conversion's string's segment, es as the code loads it
    putByte(app, specs + kNullChar, 0);
    putHalf(app, specs + kFlags, 0);
    put(app, specs + kDest, dest);
    putHalf(app, specs + kSpecs1C, 100);
    put(app, specs + kOutCount, 0);
    put(app, specs + kFormat, format);

    // A character to the caller's function, the SPECS in eax, the character in edx.
    auto put1 = [&](reg32 c) {
        const EspKept kept(cpu);
        return through(app, cpu, out, {specs, c}).has_value();
    };
    // A field's count of a character, or of the bytes at `from` on.
    auto repeat = [&](reg32 field, reg32 c) {
        while (x86::sreg32(at(app, specs + field)) > 0)
        {
            if (!put1(c))
                return false;
            put(app, specs + field, at(app, specs + field) - 1);
        }
        return true;
    };
    auto bytes = [&](reg32 field, reg32 base, reg32& from) {
        while (x86::sreg32(at(app, specs + field)) > 0)
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
            const reg32 at_ = at(app, specs + kFormat);
            const reg8 c = byte(app, at_);
            if (c != '%')
            {
                put(app, specs + kFormat, at_ + 1);
                if (!put1(c))
                    return 0;
                continue;
            }
            put(app, specs + kArgCopy, at(app, argp));
            reg32 edx, next;
            {
                const ReturnSlot slot(cpu);
                next = prtfSpecOf(app, cpu, at_ + 1, specs + kArgCopy, specs, edx) + 1;
            }
            put(app, argp, at(app, specs + kArgCopy));
            const reg8 conversion = byte(app, next - 1);
            put(app, specs + kFormat, next);
            putByte(app, specs + kCharacter, conversion);
            if (conversion == 0)
                break;
            if (conversion == 'n')
            {
                // How many went so far, into the argument: a short for h,
                // a far pointer as the game's code has it.
                const reg8 flags = byte(app, specs + kFlags);
                const reg32 count = at(app, specs + kOutCount);
                reg32 moved, target;
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
                    putHalf(app, target, reg16(count));
                else
                    put(app, target, count);
                continue;
            }

            put(app, specs + kArgCopy2, at(app, argp));
            reg32 string;
            {
                const ReturnSlot slot(cpu);
                string = prtfConvertOf(app, cpu, s, specs + kBuffer, specs + kArgCopy2, specs, es);
            }
            if (cpu.terminate)
                return 0;
            put(app, argp, at(app, specs + kArgCopy2));
            const reg32 parts = at(app, specs + kN0) + at(app, specs + kNZ0) + at(app, specs + kN1)
                                + at(app, specs + kNZ1) + at(app, specs + kN2) + at(app, specs + kNZ2);
            put(app, specs + kWidth, at(app, specs + kWidth) - parts);
            if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == ' '
                && !repeat(kWidth, ' '))
                return 0;
            reg32 prefix = specs + kBuffer;
            if (!bytes(kN0, 0, prefix) || !repeat(kNZ0, '0'))
                return 0;
            const reg8 kind = byte(app, specs + kCharacter);
            if ((kind == 's' && (byte(app, specs + kFlags) & kLong)) || kind == 'S')
            {
                const ReturnSlot slot(cpu);
                prtfWideOutOf(app, cpu, s, string, es, specs, out);
                if (cpu.terminate)
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

x86::reg32 prtf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 count = prtfOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx, args.ecx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, count);
    return args.eax;
}

/* mem_putc (0x4df660): sprintf's: the character `c` at the SPECS at `specs`'s destination, which
 * moves on, and counted. */
void memPutcOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 c)
{
    NFS2_USE(cpu);  // ebx and ecx pushed: their room only
    const reg32 p = at(app, specs);
    put(app, specs, p + 1);
    putByte(app, p, reg8(c));
    put(app, specs + kOutCount, at(app, specs + kOutCount) + 1);
}

x86::reg32 mem_putc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    memPutcOf(app, cpu, args.eax, args.edx);  // eax left as it was, the SPECS
    cpu.esp += 4;
    return args.eax;
}

/* file_putc (0x4ff000): vfprintf's: the character `c` to the stream in the SPECS at `specs` by
 * fputc, and counted.  At its entry's depth: ebx pushed.  fputc's say; 0 with cpu.terminate set
 * when the game stopped. */
reg32 filePutcOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 c)
{
    const GuestFrame frame(cpu, 4, 0);
    reg32 result;
    {
        const ReturnSlot slot(cpu);
        const EspKept kept(cpu);
        if (!putChar(app, cpu, c, at(app, specs + kDest), result))
            return 0;
    }
    put(app, specs + kOutCount, at(app, specs + kOutCount) + 1);
    return result;
}

x86::reg32 file_putc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = filePutcOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
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
    void add(reg32 from, std::uint64_t length) { ranges_.emplace_back(from, std::uint64_t(from) + length); }
    bool overlap(reg32 from, std::uint64_t length) const
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
    FormatArguments(win32::WinApplication* app, reg32 ap, Reads& reads) : app_(app), ap_(ap), reads_(reads) {}
    FormatArguments(const guest::Words& words, Reads& reads) : words_(&words), reads_(reads) {}

    bool take(reg32& value)
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
    reg32 pointer() const { return ap_; }

private:
    win32::WinApplication* app_ = nullptr;
    reg32 ap_ = 0;
    const guest::Words* words_ = nullptr;
    unsigned next_ = 0;
    Reads& reads_;
};

/* An unsigned number's digits, most significant first, from the runtime's table at `digits` (as
 * utoa and ulltoa take them). */
std::string hostDigits(win32::WinApplication* app, std::uint64_t value, reg32 base, reg32 digits, Reads& reads)
{
    reads.add(digits, 36);
    char text[72];
    int n = 0;
    do
    {
        text[n++] = char(byte(app, digits + reg32(value % base)));
        value /= base;
    } while (value != 0);
    return std::string(std::reverse_iterator<char*>(text + n), std::reverse_iterator<char*>(text));
}

/* The text __prtf makes of the format at `format` with `args`; nullopt where it is left to __prtf
 * (above). */
std::optional<std::string> hostFormatText(win32::WinApplication* app, const Segments& s, reg32 format,
                                          FormatArguments& args, Reads& reads)
{
    if (s.base(s.ds) != 0)
        return std::nullopt;
    std::string text;
    reg32 p = format;
    if (byte(app, p) != 0)
        do
        {
            const reg8 c = byte(app, p);
            if (c != '%')
            {
                text += char(c);
                ++p;
                continue;
            }
            // The specification: flags, width, precision, size (prtfSpecOf).
            reg32 q = p + 1;
            reg8 flags = 0, flags2 = 0;
            reg8 pad = ' ';
            for (;; ++q)
            {
                const reg8 f = byte(app, q);
                if (f == '-')
                    flags |= kLeft;
                else if (f == '#')
                    flags |= kAlternate;
                else if (f == '+')
                    flags = reg8((flags | kPlus) & ~kBlank);
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
            auto digits = [&](reg32& field) {
                for (reg8 d = byte(app, q); d >= '0' && d <= '9'; d = byte(app, ++q))
                    field = field * 10 + (d - '0');
            };
            reg32 width = 0;
            if (byte(app, q) == '*')
            {
                if (!args.take(width))
                    return std::nullopt;
                if (x86::sreg32(width) < 0)
                {
                    width = 0 - width;
                    flags |= kLeft;
                }
                ++q;
            }
            else
                digits(width);
            reg32 precision = 0xffffffff;
            if (byte(app, q) == '.')
            {
                precision = 0;
                ++q;
                if (byte(app, q) == '*')
                {
                    if (!args.take(precision))
                        return std::nullopt;
                    if (x86::sreg32(precision) < 0)
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
            const reg8 conversion = byte(app, q);
            p = q + 1;
            if (conversion == 0)
                break;

            // The conversion (prtfConvertOf): a prefix, the leading zeros, the string.
            std::string prefix;
            reg32 zeros = 0;          // NZ0
            reg32 stringAt = 0;       // the string's bytes, in guest memory, or
            std::string digitsText;   // the integer's digits
            bool fromGuest = false;
            reg32 length = 0;         // N1
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
                reg32 low;
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
                    reg32 high;
                    if (!args.take(high))
                        return std::nullopt;
                    value = std::uint64_t(high) << 32 | low;
                }
                else if (!(flags & kLong) && (flags & kShort))
                    value = isSigned ? reg32(x86::sreg32(x86::sreg16(reg16(low)))) : reg16(low);
                else
                    value = low;
                if (isSigned)
                {
                    const bool negative = longLong ? (value >> 63) != 0 : x86::sreg32(reg32(value)) < 0;
                    if (negative)
                    {
                        prefix += '-';
                        value = longLong ? 0 - value : std::uint64_t(reg32(0 - reg32(value)));
                    }
                    else if (flags & kPlus)
                        prefix += '+';
                    else if (flags & kBlank)
                        prefix += ' ';
                }
                reg32 base = 10;
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
                        ch = char(clib::toupper(app, reg8(ch)));
                    for (char& ch : digitsText)
                        ch = char(clib::toupper(app, reg8(ch)));
                }
                length = reg32(digitsText.size());
                if (x86::sreg32(length) < x86::sreg32(precision))
                    zeros = precision - length;
                if (precision == 0xffffffff && !(flags & kLeft) && pad == '0')
                {
                    const reg32 left = width - reg32(prefix.size()) - zeros - length;
                    if (x86::sreg32(left) > 0)
                        zeros += left;
                }
                break;
            }
            case 's':
            {
                if ((flags & kFar) || (flags & kLong))
                    return std::nullopt;
                reg32 pointer;
                if (!args.take(pointer))
                    return std::nullopt;
                if (pointer != 0)
                {
                    stringAt = pointer;
                    fromGuest = true;
                    reg32 n = 0;
                    for (reg32 s = pointer; byte(app, s) != 0 && n != precision; ++s)
                        ++n;
                    reads.add(pointer, std::uint64_t(n) + 1);
                    length = n;
                    if (x86::sreg32(precision) >= 0 && x86::sreg32(length) > x86::sreg32(precision))
                        length = precision;
                }
                break;
            }
            case 'c':
            {
                if (flags & kLong)
                    return std::nullopt;
                reg32 value;
                if (!args.take(value))
                    return std::nullopt;
                prefix += char(reg8(value));
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
            const std::uint64_t parts = prefix.size() + std::uint64_t(x86::sreg32(zeros) > 0 ? zeros : 0)
                                        + (x86::sreg32(length) > 0 ? length : 0);
            width -= reg32(prefix.size()) + zeros + length;
            const std::uint64_t spaces = x86::sreg32(width) > 0 ? width : 0;
            if (text.size() + parts + spaces > kHostTextMost)
                return std::nullopt;
            if (!(flags & kLeft) && pad == ' ')
                text.append(spaces, ' ');
            text += prefix;
            if (x86::sreg32(zeros) > 0)
                text.append(zeros, '0');
            if (x86::sreg32(length) > 0)
            {
                if (fromGuest)
                    for (reg32 i = 0; i < length; ++i)
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
std::optional<reg32> hostFormatInto(win32::WinApplication* app, reg32 buffer, const std::string& text,
                                    const Reads& reads)
{
    if (reads.overlap(buffer, std::uint64_t(text.size()) + 1))
        return std::nullopt;
    if (!text.empty())
        std::memcpy(&app->getMemory<char>(buffer), text.data(), text.size());
    putByte(app, buffer + reg32(text.size()), 0);
    return reg32(text.size());
}

/* vsprintf's text on the host: the arguments' pointer at `argp`, moved on past what was taken as
 * __prtf moves it; nullopt where __prtf is left to make it -- also where the pointer moved and
 * anything the text was made from lies at `argp`: __prtf writes the pointer back after each
 * conversion, so what it read there later was the moved pointer. */
std::optional<reg32> hostVsprintf(win32::WinApplication* app, const Segments& s, reg32 buffer, reg32 format,
                                  reg32 argp)
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
std::optional<reg32> hostSprintf(win32::WinApplication* app, const Segments& s, reg32 buffer, reg32 format,
                                 reg32 arguments)
{
    Reads reads;
    FormatArguments args(app, arguments, reads);
    const auto text = hostFormatText(app, s, format, args, reads);
    return text ? hostFormatInto(app, buffer, *text, reads) : std::nullopt;
}

/* And a typed caller's, its words in hand. */
std::optional<reg32> hostSprintf(win32::WinApplication* app, const Segments& s, reg32 buffer, reg32 format,
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
 * (hostVsprintf), else __prtf's: at its entry's depth, ecx and esi pushed.  0 with cpu.terminate
 * set when the game stopped. */
reg32 vsprintfOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 buffer, reg32 format,
                 reg32 argp, bool onHost = true)
{
    if (onHost)
        if (const auto n = hostVsprintf(app, s, buffer, format, argp))
            return *n;
    const GuestFrame frame(cpu, 8, 0);
    reg32 n;
    {
        const ReturnSlot slot(cpu);
        n = prtfOf(app, cpu, s, buffer, format, argp, kMemPutc);
    }
    if (cpu.terminate)
        return 0;
    putByte(app, buffer + n, 0);
    return n;
}

x86::reg32 vsprintf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = vsprintfOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* sprintf (clib/string/c/sprintf.c, 0x4df690): cdecl, the buffer and the format on the stack, the
 * arguments after, from `arguments` on.  At its entry's depth: ebx and edx pushed, then the
 * va_list. */
reg32 sprintfOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 buffer, reg32 format,
                reg32 arguments)
{
    if (const auto n = hostSprintf(app, s, buffer, format, arguments))
        return *n;
    const GuestFrame frame(cpu, 8, 4);
    const reg32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(cpu);
    return vsprintfOf(app, cpu, s, buffer, format, list, false);
}

x86::reg32 sprintf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = sprintfOf(app, cpu, segmentsOf(cpu), at(app, cpu.esp + 4), at(app, cpu.esp + 8), cpu.esp + 0xc);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* vfprintf (0x4ff014): the format at `format` with the arguments' pointer at `argp` to the stream
 * at `fp`, under the file's lock: none to a stream of wide characters, an unbuffered one buffered
 * while it goes; how many, or -1 for an error.  At its entry's depth: four pushes.  0 with
 * cpu.terminate set when the game stopped. */
reg32 vfprintfOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 fp, reg32 format, reg32 argp)
{
    const GuestFrame frame(cpu, 0x10, 0);
    // The file's lock and its release, every register kept.
    auto lock = [&](reg32 slot) {
        const EspKept kept(cpu);
        return streamLock(app, cpu, slot, fp);
    };
    if (!lock(kAccessFile))
        return 0;
    const reg32 link = linkOf(app, fp);
    const reg32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
        {
            lock(kReleaseFile);
            return 0;
        }
        put(app, link + kOrientation, 1);
    }
    const reg32 kept = at(app, fp + kFlag) & 0x30;
    putByte(app, fp + kFlag, byte(app, fp + kFlag) & 0xcf);
    if (bufferOf(app, fp) == 0)
    {
        const ReturnSlot slot(cpu);
        const EspKept registers(cpu);
        if (!allocateBuffer(app, cpu, fp))
            return 0;
    }
    const bool unbuffered = byte(app, fp + kFlag + 1) & 4;
    if (unbuffered)
        putByte(app, fp + kFlag + 1, reg8((byte(app, fp + kFlag + 1) & 0xfa) | 1));
    reg32 result;
    {
        const ReturnSlot slot(cpu);
        result = prtfOf(app, cpu, s, fp, format, argp, kFilePutc);
    }
    if (cpu.terminate)
        return 0;
    if (unbuffered)
    {
        const reg8 restored = reg8((byte(app, fp + kFlag + 1) & 0xfa) | 4);
        putByte(app, fp + kFlag + 1, reg8(byte(app, fp + kFlag + 1) & 0xfa));
        putByte(app, fp + kFlag + 1, restored);
        const ReturnSlot slot(cpu);
        const EspKept registers(cpu);
        reg32 flushed;
        if (!flushStream(app, cpu, fp, flushed))
            return 0;
    }
    if (byte(app, fp + kFlag) & 0x20)
        result = 0xffffffff;
    put(app, fp + kFlag, at(app, fp + kFlag) | kept);
    if (!lock(kReleaseFile))
        return 0;
    return result;
}

x86::reg32 vfprintf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = vfprintfOf(app, cpu, segmentsOf(cpu), args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* fprintf (clib/streamio/c/fprintf.c, 0x4f4b80): cdecl, the stream and the format on the stack, the
 * arguments after, from `arguments` on.  At its entry's depth: ebx and edx pushed, then the
 * va_list. */
reg32 fprintfOf(win32::WinApplication* app, x86::CPU& cpu, const Segments& s, reg32 fp, reg32 format,
                reg32 arguments)
{
    const GuestFrame frame(cpu, 8, 4);
    const reg32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(cpu);
    return vfprintfOf(app, cpu, s, fp, format, list);
}

x86::reg32 fprintf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = fprintfOf(app, cpu, segmentsOf(cpu), at(app, cpu.esp + 4), at(app, cpu.esp + 8), cpu.esp + 0xc);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
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

constexpr reg32 kEnviron = 0xa0b154;          // _environ
constexpr reg32 kTzName0 = 0x568a88;          // tzname[0]: 0x80 bytes and a 0
constexpr reg32 kTzName1 = 0x568b09;          // tzname[1]
constexpr reg32 kTimezoneSeconds = 0x568b94;  // timezone
constexpr reg32 kDaylightOn = 0x568b98;       // daylight
constexpr reg32 kDstSeconds = 0x568b9c;       // __dst_adjust
constexpr reg32 kZoneDone = 0x568ba0;         // bit 2: the zone asked of Windows
constexpr reg32 kTzString = 0x5505ac;         // "TZ"
constexpr reg32 kSjisTrail = 0x56b6dd;        // a byte a character: 8 for a trail byte of code page 932
constexpr reg32 kFiniStart = 0x56b81a, kFiniEnd = 0x56b844;  // the runtime's finalizers: 6 bytes each
constexpr reg32 kExitHook = 0x564484, kExitHook2 = 0x564488, kExitUser = 0x5677f0;
constexpr reg32 kExitStarted = 0x9f6420;      // whether the runtime's start-up got as far as its finalizers
constexpr reg32 kExitDone = 0x5677a0, kExitEarly = 0x5677ac;
constexpr reg32 kEnvironCopy = 0x9f6424, kWideEnvironCopy = 0x9f6428;
constexpr reg32 kAtexitCount = 0x9f3890, kAtexitTable = 0x9f380c;
constexpr reg32 kDebuggerOn = 0x56af64;       // a debugger's: the runtime's int3 to it
constexpr reg32 kOsHandleTable = 0x56acbc;    // __OSHandles: stderr's HANDLE at +8
constexpr reg32 kStackOverflow = 0x5662cc;    // "Stack Overflow!"
constexpr reg32 kTmpDir = 0x568278;           // the directory of temporary files, once found
constexpr reg32 kTmpVariables = 0x568264;     // TMP, TEMP, TMPDIR, TEMPDIR, ""
constexpr reg32 kConsoleName = 0x551b08;      // "con"
constexpr reg32 kGetTimeZoneInformation = 0x53456c, kCharUpperBuffA = 0x5346fc, kExitProcess = 0x5344c0;
constexpr reg32 kWriteFileSlot = 0x534640, kSleepEx = 0x534604, kGetCurrentProcessId = 0x534508;
constexpr reg32 kGetFullPathNameA = 0x534530;
constexpr reg32 kThreadPtr = 0x567764;        // __GetThreadPtr's pointer

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

void pushEs(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp -= 4;
    putHalf(app, cpu.esp, 0);  // es: the flat model's 0
}

void popEs(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp += 4;  // es back: the flat model's 0 all along
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it; the callee's thunk run on `args`, the registers the code has at the call, and
 * what it leaves in them (eax too) left there.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, win32::ThunkPtr function, x86::Args& args)
{
    cpu.esp -= 4;
    args.eax = function(app, cpu, args);
    return !cpu.terminate;
}

/* The same through a pointer the code holds (pointerCall: esp back where it was after). */
bool indirect(win32::WinApplication* app, x86::CPU& cpu, reg32 target, x86::Args& args)
{
    const std::optional<reg32> eax = guest::pointerCall(app, cpu, target, args);
    if (!eax)
        return false;
    args.eax = *eax;
    return true;
}

void setLow8(reg32& r, reg32 value)
{
    r = (r & 0xffffff00) | (value & 0xff);
}

bool multibyte(win32::WinApplication* app)
{
    return at(app, kMbcsOn) != 0;
}

bool leadByte(win32::WinApplication* app, reg32 c)
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
reg32 mbtermOf(win32::WinApplication* app, reg32 p)
{
    if (byte(app, p) == 0)
        return 1;
    if (multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) == 0)
        return 2;
    return 0;
}

x86::reg32 mbterm(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbtermOf(app, args.eax));
    return args.eax;
}

/* 0x525100 (_mbsinc): past the character at `p`. */
reg32 mbsincOf(win32::WinApplication* app, reg32 p)
{
    return multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) != 0 ? p + 2 : p + 1;
}

x86::reg32 mbsinc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbsincOf(app, args.eax));
    return args.eax;
}

/* 0x52b5b0 (_mbclen): the character at `p`'s bytes, 1 or 2. */
reg32 mbclenOf(win32::WinApplication* app, reg32 p)
{
    return multibyte(app) && leadByte(app, byte(app, p)) ? 2 : 1;
}

x86::reg32 mbclen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbclenOf(app, args.eax));
    return args.eax;
}

/* 0x52e310 (_mbccpy): the character at `from` to `to`. */
void mbccpyOf(win32::WinApplication* app, reg32 to, reg32 from)
{
    putByte(app, to, byte(app, from));
    if (multibyte(app) && leadByte(app, byte(app, from)))
        putByte(app, to + 1, byte(app, from + 1));
}

x86::reg32 mbccpy(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    mbccpyOf(app, args.eax, args.edx);  // eax left as it was, the destination
    cpu.esp += 4;
    return args.eax;
}

/* 0x52e3a0 (_mbccmp): the characters at `a` and `b` compared, their bytes' difference. */
reg32 mbccmpOf(win32::WinApplication* app, reg32 a, reg32 b)
{
    if (byte(app, a) != byte(app, b))
        return reg32(byte(app, a)) - byte(app, b);
    if (multibyte(app) && leadByte(app, byte(app, a)) && byte(app, a + 1) != byte(app, b + 1))
        return reg32(byte(app, a + 1)) - byte(app, b + 1);
    return 0;
}

x86::reg32 mbccmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbccmpOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x52ed30 (_mbsnextc): the character at `p`, a lead byte's two as one. */
reg32 mbsnextcOf(win32::WinApplication* app, reg32 p)
{
    if (multibyte(app) && leadByte(app, byte(app, p)))
        return reg32(byte(app, p)) << 8 | byte(app, p + 1);
    return byte(app, p);
}

x86::reg32 mbsnextc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbsnextcOf(app, args.eax));
    return args.eax;
}

/* 0x52ede0 (_mbvtop): the character `c` into the bytes at `to`; `to` back. */
reg32 mbvtopOf(win32::WinApplication* app, reg32 c, reg32 to)
{
    if (c & 0xff00)
    {
        putByte(app, to + 1, reg8(c));
        putByte(app, to, reg8(c >> 8));
    }
    else
        putByte(app, to, reg8(c));
    return to;
}

x86::reg32 mbvtop(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbvtopOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x52ef70 (_ismbbtrail, as the runtime has it): whether the byte `c` may follow a lead byte. */
reg32 mbbtrailOf(win32::WinApplication* app, reg32 c)
{
    if (!multibyte(app))
        return 0;
    if (at(app, kCodePage) == 932)
        return byte(app, kSjisTrail + (c & 0xff)) & 8;
    return c != 0 ? 1 : 0;
}

x86::reg32 mbbtrail(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbbtrailOf(app, args.eax));
    return args.eax;
}

/* 0x52ef30 (_ismbclegal's): whether `c` is a character of two bytes. */
reg32 mbclegalOf(win32::WinApplication* app, reg32 c)
{
    if (multibyte(app) && leadByte(app, c >> 8 & 0xff))
        return mbbtrailOf(app, c & 0xff) != 0 ? 1 : 0;
    return 0;
}

x86::reg32 mbclegal(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbclegalOf(app, args.eax));
    return args.eax;
}

/* 0x52eee0 (_ismbclower's, code page 932): whether `c` is a small letter -- a full-width one of
 * two bytes, or one of _IsTable's. */
reg32 mbclowerOf(win32::WinApplication* app, reg32 c)
{
    if (mbclegalOf(app, c) == 0)
        return byte(app, kIsTable + ((c + 1) & 0xff)) & 0x80;
    return c >= 0x8281 && c <= 0x829a ? 1 : 0;
}

x86::reg32 mbclower(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbclowerOf(app, args.eax));
    return args.eax;
}

/* 0x52eec0 (_mbctoupper's, code page 932): a full-width small letter as a capital. */
reg32 mbcupper932Of(win32::WinApplication* app, reg32 c)
{
    return mbclowerOf(app, c) != 0 ? c - 0x21 : c;
}

x86::reg32 mbcupper932(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbcupper932Of(app, args.eax));
    return args.eax;
}

/* 0x52ed70 (_mbctoupper): the character `c` as a capital, by CharUpperBuffA on its bytes in the
 * frame -- or, a full-width one of code page 932, by the runtime's own.  Called at its entry's
 * depth (esp at the return address's slot): three pushes, then the bytes.  0 with cpu.terminate
 * set when the game stopped. */
reg32 mbctoupperOf(win32::WinApplication* app, x86::CPU& cpu, reg32 c)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 bytes = frame - 4;
    mbvtopOf(app, c, bytes);
    putByte(app, bytes + mbclenOf(app, bytes), 0);
    if (at(app, kCodePage) == 932 && multibyte(app) && leadByte(app, byte(app, bytes)))
        return mbcupper932Of(app, c);
    {
        // Every register but eax as _mbctoupper kept them, whatever CharUpperBuffA leaves.
        const EspKept kept(cpu);
        if (!windows<kCharUpperBuffA, &host::charUpperBuffer>(app, cpu, bytes, 1))
            return 0;
    }
    return mbsnextcOf(app, bytes);
}

x86::reg32 mbctoupper(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 upper;
    {
        const EspKept kept(cpu);
        upper = mbctoupperOf(app, cpu, args.eax);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, upper);
    return args.eax;
}

/* 0x52e350 (_mbsupr): the string `s` in capitals, a character at a time; `s` back.  At its
 * entry's depth: three pushes, then a character's bytes.  0 with cpu.terminate set when the game
 * stopped. */
reg32 mbsuprOf(win32::WinApplication* app, x86::CPU& cpu, reg32 s)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 bytes = frame - 4;
    for (reg32 p = s; mbtermOf(app, p) == 0; p = mbsincOf(app, p))
    {
        reg32 upper;
        {
            const ReturnSlot call(cpu);
            upper = mbctoupperOf(app, cpu, mbsnextcOf(app, p));
        }
        if (cpu.terminate)
            return 0;
        mbvtopOf(app, upper, bytes);
        putByte(app, bytes + mbclenOf(app, bytes), 0);
        mbccpyOf(app, p, bytes);
    }
    return s;
}

x86::reg32 mbsupr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 s;
    {
        const EspKept kept(cpu);
        s = mbsuprOf(app, cpu, args.eax);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, s);
    return args.eax;
}

/* 0x52b5e0 (_mbcicmp): the characters at `a` and `b` compared as capitals: each copied into the
 * frame, put in capitals there, and compared.  At its entry's depth: two pushes, then the two
 * characters' 4 bytes each.  0 with cpu.terminate set when the game stopped. */
reg32 mbcicmpOf(win32::WinApplication* app, x86::CPU& cpu, reg32 a, reg32 b)
{
    const GuestFrame frame(cpu, 8, 8);
    const reg32 first = frame - 8, second = frame - 4;
    mbccpyOf(app, first, a);
    putByte(app, first + mbclenOf(app, a), 0);
    mbccpyOf(app, second, b);
    putByte(app, second + mbclenOf(app, b), 0);
    for (const reg32 s : {first, second})
    {
        const ReturnSlot call(cpu);
        mbsuprOf(app, cpu, s);
        if (cpu.terminate)
            return 0;
    }
    return mbccmpOf(app, first, second);
}

x86::reg32 mbcicmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 differs;
    {
        const EspKept kept(cpu);
        differs = mbcicmpOf(app, cpu, args.eax, args.edx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, differs);
    return args.eax;
}

/* 0x523d10 (_mbsnicmp): at most `count` characters of the strings at `a` and `b` compared as
 * capitals: the first difference, or that of the bytes where one ended first, or 0.  At its
 * entry's depth: two pushes.  0 with cpu.terminate set when the game stopped. */
reg32 mbsnicmpOf(win32::WinApplication* app, x86::CPU& cpu, reg32 a, reg32 b, reg32 count)
{
    const GuestFrame frame(cpu, 8, 0);
    for (; count != 0; --count, a = mbsincOf(app, a), b = mbsincOf(app, b))
    {
        if (mbtermOf(app, a) != 0 || mbtermOf(app, b) != 0)
            break;
        reg32 differs;
        {
            const ReturnSlot call(cpu);
            differs = mbcicmpOf(app, cpu, a, b);
        }
        if (cpu.terminate)
            return 0;
        if (differs != 0)
            return differs;
    }
    if (count != 0 && (mbtermOf(app, a) != 0 || mbtermOf(app, b) != 0))
        return reg32(byte(app, a)) - byte(app, b);
    return 0;
}

x86::reg32 mbsnicmp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 differs;
    {
        const EspKept kept(cpu);
        differs = mbsnicmpOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, differs);
    return args.eax;
}

namespace
{

reg32 runs(win32::WinApplication* app, x86::CPU& cpu, win32::ThunkPtr function,
           std::initializer_list<x86::reg32> registers);

}  // namespace

/* getenv's work from the function's entry, esp at its return address's slot: the room of the six
 * registers it keeps pushed (nothing written there), so _mbsnicmp's are made at the depth the
 * runtime's were.  0 with cpu.terminate set when the game stopped. */
reg32 getenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name)
{
    cpu.esp -= 0x18;
    reg32 entry = at(app, kEnviron);
    reg32 value = 0;
    if (entry != 0 && name != 0)
    {
        reg32 length = 0;
        while (byte(app, name + length) != 0)
            ++length;
        for (reg32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
        {
            reg32 differs;
            {
                const ReturnSlot call(cpu);
                differs = mbsnicmpOf(app, cpu, string, name, length);
            }
            if (cpu.terminate)
                return 0;
            if (differs == 0 && byte(app, string + length) == '=')
            {
                value = string + length + 1;
                break;
            }
        }
    }
    cpu.esp += 0x18;
    return value;
}

/* getenv (clib/environ/c/getenv.c, 0x50ead0): the value of the variable named
 * at eax, its name matched as capitals -- or 0. */
x86::reg32 getenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 value = getenvOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, value);
    return args.eax;
}

/* wcstombs (clib/mbyte/c/wcstombs.c, 0x5242f0): the wide string at `from` into `to` as wctomb
 * makes each character, at most `most` bytes -- or, for no buffer, how many it would take; -1 for
 * one wctomb cannot make.  At its entry's depth: four pushes, then 0xc bytes, wctomb's character
 * at +8 of them (WideCharToMultiByte is handed it).  0 with cpu.terminate set when the game
 * stopped. */
reg32 wcstombsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 to, reg32 from, reg32 most)
{
    const GuestFrame frame(cpu, 0x10, 0xc);
    const reg32 character = frame - 4;
    reg32 total = 0;
    if (to == 0)
    {
        for (; half(app, from) != 0; from += 2)
        {
            const reg32 bytes = called(cpu, [&] { return wctombOf(app, cpu, character, half(app, from)); });
            if (cpu.terminate)
                return 0;
            if (bytes == 0xffffffff)
                return bytes;
            total += bytes;
        }
        return total;
    }
    for (reg32 room = most; room != 0; from += 2)
    {
        const reg32 c = half(app, from);
        if (c == 0)
        {
            putByte(app, to, 0);
            break;
        }
        const reg32 bytes = called(cpu, [&] { return wctombOf(app, cpu, character, c); });
        if (cpu.terminate)
            return 0;
        if (bytes == 0xffffffff)
            return bytes;
        if (bytes > room)
            break;
        for (reg32 i = 0; i < bytes; ++i)
            putByte(app, to + i, byte(app, character + i));
        total += bytes;
        room -= bytes;
        to += bytes;
    }
    return total;
}

x86::reg32 wcstombs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 result;
    {
        const EspKept kept(cpu);
        result = wcstombsOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* ---- __tzset (clib/time/c/tzset.c) in its parts: TZ's zone and its daylight saving's name and
 * rules parsed, or the zone asked of Windows.  The zone's and the rules' numbers are C++ locals;
 * what each part leaves in eax is worked out from what it did. ---- */

/* 0x515c90 (__tzset's number): the decimal digits at `p` into `value`; past them. */
reg32 tzNumberOf(win32::WinApplication* app, reg32 p, reg32& value)
{
    value = 0;
    for (; byte(app, p) >= '0' && byte(app, p) <= '9'; ++p)
        value = value * 10 + byte(app, p) - '0';
    return p;
}

x86::reg32 tzNumber(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 value;
    const reg32 past = tzNumberOf(app, args.eax, value);
    put(app, args.edx, value);
    returns(cpu, args, past);
    return args.eax;
}

/* 0x515cbc (__tzset's zone): TZ's name at `p` -- up to a sign, a digit or a comma, at most 0x80
 * of it -- into `name`, and the hours[:minutes[:seconds]] after it, as seconds west, into the
 * dword at `seconds`; past them. */
reg32 tzNameOf(win32::WinApplication* app, reg32 p, reg32 name, reg32 seconds)
{
    if (byte(app, p) == ':')
        ++p;
    const reg32 start = p;
    for (;; ++p)
    {
        const reg8 c = byte(app, p);
        if (c == 0 || c == ',' || c == '-' || c == '+' || (c >= '0' && c <= '9'))
            break;
    }
    reg32 length = p - start;
    if (x86::sreg32(length) > 0x80)
        length = 0x80;
    for (reg32 i = 0; i < length; ++i)
        putByte(app, name + i, byte(app, start + i));
    putByte(app, name + length, 0);
    const reg8 sign = byte(app, p);
    if (sign == '-' || sign == '+')
        ++p;
    if (byte(app, p) >= '0' && byte(app, p) <= '9')
    {
        reg32 hours, minutes = 0, rest = 0;
        p = tzNumberOf(app, p, hours);
        if (byte(app, p) == ':')
        {
            p = tzNumberOf(app, p + 1, minutes);
            if (byte(app, p) == ':')
                p = tzNumberOf(app, p + 1, rest);
        }
        const reg32 west = rest + (minutes + hours * 60) * 60;
        put(app, seconds, sign == '-' ? 0 - west : west);
    }
    return p;
}

x86::reg32 tzName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, tzNameOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x515dd4 (__tzset's rule): daylight saving's start or end at `p` -- Jn, n or Mm.w.d, then
 * /hours[:minutes[:seconds]] -- into the struct tm at `tm`; past it. */
reg32 tzRuleOf(win32::WinApplication* app, reg32 p, reg32 tm)
{
    reg32 kind = 0xffffffff;  // -1 a day of the year from 0, 1 from 1 (J), 0 a week's day (M)
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
    reg32 n;
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
    reg32 hours = 2, minutes = 0, seconds = 0;
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

x86::reg32 tzRule(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, tzRuleOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x515b34 (__tzset's without TZ): the zone, daylight saving and the zones' names as
 * GetTimeZoneInformation has them, once.  At its entry's depth: four pushes, then the
 * TIME_ZONE_INFORMATION Windows fills.  What the code leaves in eax, given `eax` on entry; 0 with
 * cpu.terminate set when the game stopped. */
reg32 tzWindowsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 eax)
{
    const GuestFrame frame(cpu, 0x10, 0xac);
    const reg32 information = frame - 0xac;
    const reg8 done = byte(app, kZoneDone);
    if ((done & 1) && (done & 2))
        return (eax & 0xffff00ff) | reg32(done) << 8;
    putByte(app, kZoneDone, done | 2);
    reg32 answer;
    {
        const EspKept kept(cpu);
        const auto said = windows<kGetTimeZoneInformation, &host::timeZone>(app, cpu, information);
        if (!said)
            return 0;
        answer = *said;
    }
    if (answer == 0 || answer > 2)
        return answer;
    if (answer == 2)
    {
        put(app, kDaylightOn, 1);
        put(app, kDstSeconds, 0 - at(app, information + 0xa8) * 60);
    }
    else
        put(app, kDaylightOn, 0);
    put(app, kTimezoneSeconds, (at(app, information + 0x54) + at(app, information)) * 60);
    reg32 made = 0;
    for (const auto& [name, wide] : {std::pair<reg32, reg32>{kTzName0, 4}, {kTzName1, 0x58}})
    {
        {
            const ReturnSlot call(cpu);
            made = wcstombsOf(app, cpu, name, information + wide, 0x80);
        }
        if (cpu.terminate)
            return 0;
        putByte(app, made == 0xffffffff ? name : name + 0x80, 0);
    }
    return made;
}

x86::reg32 tzWindows(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 eax;
    {
        const EspKept kept(cpu);
        eax = tzWindowsOf(app, cpu, args.eax);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x515ec2: __tzset's after a TZ zone with a daylight name -- its own offset or an hour less, and
 * the rules after commas, the end rule's time made standard time -- in __tzset's frame: `local`
 * its dword (the daylight zone's offset is parsed into it), `nameEnd` where the zone's name ended.
 * What the code leaves in eax. */
reg32 tzDaylightOf(win32::WinApplication* app, reg32 nameEnd, reg32 local)
{
    put(app, kDaylightOn, 1);
    put(app, local, at(app, kTimezoneSeconds) - 3600);
    const reg32 p = tzNameOf(app, nameEnd, kTzName1, local);
    put(app, kDstSeconds, at(app, kTimezoneSeconds) - at(app, local));
    reg32 eax = p, rules = p;
    if (byte(app, p) == ',')
        eax = rules = tzRuleOf(app, p + 1, kDstStart);
    const reg32 dst = at(app, kDstSeconds);
    if (byte(app, rules) == ',')
    {
        tzRuleOf(app, rules + 1, kDstEnd);
        const x86::sreg32 seconds = x86::sreg32(at(app, kDstSeconds));
        const x86::sreg32 minutes = seconds / 60;
        const reg32 hour = at(app, kDstEnd + 8) - reg32(seconds / 3600);
        const reg32 minute = at(app, kDstEnd + 4) - reg32(minutes % 60);
        eax = at(app, kDstEnd) - reg32(seconds % 60);
        put(app, kDstEnd + 8, hour);
        put(app, kDstEnd + 4, minute);
        put(app, kDstEnd, eax);
        put(app, kDstSeconds, reg32(seconds));
        return eax;
    }
    put(app, kDstSeconds, dst);
    return eax;
}

x86::reg32 tzDaylight(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 local = cpu.esp;  // __tzset's dword, its six registers above
    const reg32 eax = tzDaylightOf(app, args.ecx, local);
    cpu.esp = local + 4;
    args.ebp = pop(app, cpu);
    args.edi = pop(app, cpu);
    args.esi = pop(app, cpu);
    args.edx = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    returns(cpu, args, eax);
    return args.eax;
}

/* __tzset (clib/time/c/tzset.c, 0x515c2c): the zone from TZ -- its name, offset, daylight
 * saving's name and rules -- or from Windows.  Its frame -- the six registers it keeps and a
 * dword -- is where tzDaylight finishes, as the code jumps there. */
x86::reg32 tzset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 tz;
    {
        const ReturnSlot call(cpu);
        tz = getenvOf(app, cpu, kTzString);
    }
    if (cpu.terminate)
        return args.eax;
    args.eax = tz;
    if (tz == 0)
        return tzWindows(app, cpu, args);
    for (const reg32 r : {args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp})
        push(app, cpu, r);
    cpu.esp -= 4;
    put(app, kDaylightOn, 0);
    const reg32 p = tzNameOf(app, tz, kTzName0, kTimezoneSeconds);
    if (byte(app, p) != 0)
    {
        args.ecx = p;
        return tzDaylight(app, cpu, args);
    }
    putByte(app, kTzName1, 0);
    cpu.esp += 4;
    args.ebp = pop(app, cpu);
    args.edi = pop(app, cpu);
    args.esi = pop(app, cpu);
    args.edx = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    returns(cpu, args, p & 0xffff00ff);
    return args.eax;
}

/* 0x4ff410 (__FiniRtns' call): the routine at the pointer at `p`, if any, with es its ds -- es
 * pushed meanwhile, and ds pushed and popped into it.  False when the game stopped. */
bool callRoutineOf(win32::WinApplication* app, x86::CPU& cpu, reg32 p)
{
    // es pushed, and ds pushed and popped into it for the routine: the room of es's push (ds's is
    // the call's return address's), nothing written; es and ds are the same everywhere in the game.
    const GuestFrame frame(cpu, 4, 0);
    if (at(app, p) == 0)
        return true;
    const EspKept kept(cpu);
    return through(app, cpu, at(app, p), {p}).has_value();
}

x86::reg32 callRoutine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (!callRoutineOf(app, cpu, args.eax))
        return args.eax;
    cpu.esp += 4;  // eax left as it was, the pointer
    return args.eax;
}

/* __FiniRtns (clib/startup/c/initrtns.c, 0x4ff46c): the runtime's finalizers of priority `low` to
 * `high` not yet run, the highest first, each marked done.  At its entry's depth: three pushes and
 * es.  What the code leaves in eax (its table's end); 0 with cpu.terminate set when the game
 * stopped. */
reg32 finiRtnsOf(win32::WinApplication* app, x86::CPU& cpu, reg8 low, reg8 high)
{
    const GuestFrame frame(cpu, 0x10, 0);
    for (;;)
    {
        reg32 chosen = kFiniEnd;
        reg8 best = low;
        for (reg32 entry = kFiniStart; entry < kFiniEnd; entry += 6)
            if (byte(app, entry) != 2 && best <= byte(app, entry + 1))
            {
                chosen = entry;
                best = byte(app, entry + 1);
            }
        if (chosen == kFiniEnd)
            break;
        if (high >= byte(app, chosen + 1))
        {
            called(cpu, [&] { return callRoutineOf(app, cpu, chosen + 2); });
            if (cpu.terminate)
                return 0;
        }
        putByte(app, chosen, 2);
    }
    return kFiniEnd;
}

x86::reg32 FiniRtns(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg8 low = reg8(args.eax), high = reg8(args.edx);
    reg32 eax;
    {
        const EspKept kept(cpu);
        eax = finiRtnsOf(app, cpu, low, high);
    }
    if (cpu.terminate)
        return args.eax;
    // dh the highest priority, dl the lowest, as the code leaves them (exit_ is stopped with them).
    args.edx = (args.edx & 0xffff0000) | reg32(high) << 8 | low;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x51f11c (__DoneExceptionHandler): the thread's exception record off fs's
 * chain. */
x86::reg32 DoneExceptionHandler(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.edx);
    if (!hookWith<kThreadPtr>(app, cpu, args))
        return args.eax;
    args.eax = at(app, args.eax + 0x54);
    if (args.eax != 0)
    {
        args.eax = at(app, args.eax);
        args.edx = 0;
        put(app, cpu.threadBlock + args.edx, args.eax);
    }
    if (!hookWith<kThreadPtr>(app, cpu, args))
        return args.eax;
    put(app, args.eax + 0x54, 0);
    args.edx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4ff3d0 (the environment's copies given back): what the start-up code made of it, narrow and
 * wide, to _nfree.  What the code leaves in eax, given `eax` on entry: _nfree's last answer, if
 * any.  0 with cpu.terminate set when the game stopped. */
reg32 freeEnvironmentOf(win32::WinApplication* app, x86::CPU& cpu, reg32 eax)
{
    for (const reg32 copy : {kEnvironCopy, kWideEnvironCopy})
        if (at(app, copy) != 0)
        {
            eax = clib::free(app, cpu, at(app, copy));
            if (cpu.terminate)
                return 0;
            put(app, copy, 0);
        }
    return eax;
}

x86::reg32 freeEnvironment(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = freeEnvironmentOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* __exit (clib/startup/c/exit.c, 0x4ff38c): ExitProcess with eax -- the
 * finalizers and the exception record first, unless the start-up got no
 * further than its own -- and, should it come back, the environment's copies
 * given back, as the code after it does. */
x86::reg32 exit_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.ebx = args.eax;
    if (at(app, kExitStarted) != 0)
    {
        if (at(app, kExitEarly) != 0)
        {
            args.edx = 0xff;
            args.eax = 0;
            if (!indirect(app, cpu, at(app, kExitEarly), args))
                return args.eax;
        }
    }
    else
    {
        if (!direct(app, cpu, DoneExceptionHandler, args))
            return args.eax;
        args.edx = 0xf;
        args.eax = 0;
        if (!direct(app, cpu, FiniRtns, args) || !hookWith<kExitDone>(app, cpu, args))
            return args.eax;
    }
    if (!windows<kExitProcess, &host::exitProcess>(app, cpu, args.ebx))
        return args.eax;
    freeEnvironment(app, cpu, args);
    return args.eax;
}

/* _exit (clib/startup/c/exit.c, 0x4dfd30): eax to __exit after the runtime's
 * hooks -- jumped to with edx still pushed. */
x86::reg32 _exit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.edx);
    args.edx = args.eax;
    if (!hookWith<kExitHook>(app, cpu, args) || !hookWith<kExitHook2>(app, cpu, args))
        return args.eax;
    if (at(app, kExitUser) != 0 && !indirect(app, cpu, at(app, kExitUser), args))
        return args.eax;
    args.eax = args.edx;
    exit_(app, cpu, args);
    return args.eax;
}

/* exit (clib/startup/c/exit.c, 0x4dfd04): the finalizers of priority 16 and
 * up, then _exit with eax. */
x86::reg32 exit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebx);
    push(app, cpu, args.edx);
    args.ebx = args.eax;
    if (!hookWith<kExitHook>(app, cpu, args))
        return args.eax;
    if (byte(app, kExitStarted) == 0)
    {
        args.edx = 0xff;
        args.eax = 0x10;
        if (!direct(app, cpu, FiniRtns, args))
            return args.eax;
    }
    args.eax = args.ebx;
    if (!direct(app, cpu, _exit, args))
        return args.eax;
    args.edx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* atexit (clib/startup/c/atexit.c, 0x4f7c20): `function` to the 32 run at exit; 0, or -1 when
 * they are taken. */
reg32 atexitOf(win32::WinApplication* app, reg32 function)
{
    const reg32 count = at(app, kAtexitCount);
    if (x86::sreg32(count) >= 0x20)
        return 0xffffffff;
    put(app, (count + 1) * 4 + kAtexitTable, function);
    put(app, kAtexitCount, count + 1);
    return 0;
}

x86::reg32 atexit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, atexitOf(app, args.eax));
    return args.eax;
}

/* 0x524190 (__EnterWVIDEO): a message to a debugger, by its int3, if there
 * is one; 1, or 0. */
x86::reg32 EnterWVIDEO(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (byte(app, kDebuggerOn) == 0)
    {
        args.eax = 0;
        cpu.esp += 4;
        return args.eax;
    }
    args.edx &= 0xffff;
    push(app, cpu, args.edx);
    push(app, cpu, args.eax);
    NFS2_ASSERT(false);  // int3
    args.eax = 1;
    cpu.esp += 8 + 4;
    return args.eax;
}

/* 0x511cd0 (__fatal_runtime_error's): the message at eax to stderr's handle, then __exit with
 * edx -- jumped to, its frame still there: four pushes and the dword WriteFile counts into. */
x86::reg32 fatalWrite(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 message = args.eax, code = args.edx;
    for (const reg32 r : {args.ebx, args.ecx, args.esi, args.edi})
        push(app, cpu, r);
    cpu.esp -= 4;
    reg32 length = 0;
    while (byte(app, message + length) != 0)
        ++length;
    // The registers as the code leaves them on its way out: they are what the game stops with.
    args.ebx = message + length;
    args.ecx &= 0xffffff00;
    args.edi = message;
    args.esi = code;
    args.edx = at(app, at(app, kOsHandleTable) + 8);
    if (!windows<kWriteFileSlot, &host::writeFile>(app, cpu, args.edx, message, length, cpu.esp, 0))
        return args.eax;
    args.eax = code;
    exit_(app, cpu, args);
    return args.eax;
}

/* __fatal_runtime_error (clib/startup/c/fatalerr.c, 0x511d0c): the message at
 * eax to a debugger, or to stderr and out with edx. */
x86::reg32 fatal_runtime_error(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebx);
    push(app, cpu, args.ecx);
    args.ebx = args.eax;
    args.ecx = args.edx;
    args.edx = 0;  // ds: the flat model's 0
    if (!direct(app, cpu, EnterWVIDEO, args))
        return args.eax;
    if (args.eax == 0)
    {
        args.edx = args.ecx;
        args.eax = args.ebx;
        if (!direct(app, cpu, fatalWrite, args))
            return args.eax;
    }
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* sleep (clib/process/c/sleepwnt.c, 0x4f0710): SleepEx for `seconds`; its answer.  At its entry's
 * depth: two pushes.  0 with cpu.terminate set when the game stopped. */
reg32 sleepOf(win32::WinApplication* app, x86::CPU& cpu, reg32 seconds)
{
    const GuestFrame frame(cpu, 8, 0);
    const EspKept kept(cpu);
    return windowsAt<kSleepEx, &host::sleep>(app, cpu, seconds * 1000, seconds * 1000, 0).value_or(0);
}

x86::reg32 sleep(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 answer = sleepOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, answer);
    return args.eax;
}

/* getpid (clib/environ/c/gtpidwnt.c, 0x5093c0): GetCurrentProcessId.  At its entry's depth: two
 * pushes.  0 with cpu.terminate set when the game stopped. */
reg32 getpidOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 8, 0);
    const EspKept kept(cpu);
    return windows<kGetCurrentProcessId, &host::currentProcessId>(app, cpu).value_or(0);
}

x86::reg32 getpid(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 id = getpidOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, id);
    return args.eax;
}

/* stackavail (clib/startup/c/stackwnt.c, 0x4f45c0): how far esp is above the
 * thread's stack's bottom. */
std::optional<reg32> stackavailOf(win32::WinApplication* app, x86::CPU& cpu, reg32 edx)
{
    push(app, cpu, edx);
    const std::optional<reg32> data = hook<kThreadPtr>(app, cpu);
    if (!data)
        return std::nullopt;
    const reg32 bottom = at(app, *data);
    const reg32 available = cpu.esp - bottom;
    pop(app, cpu);
    return available;
}

x86::reg32 stackavail(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const auto available = stackavailOf(app, cpu, args.edx);
    if (!available)
        return args.eax;
    args.eax = *available;
    cpu.esp += 4;
    return args.eax;
}

/* __GRO (clib/startup/a/stkwnt.asm, 0x4f4610): the stack's pages below esp
 * touched, a dword each, for the size on the stack, which it takes off. */
void stackProbedOf(win32::WinApplication* app, x86::CPU& cpu, reg32 size, reg32 eax, reg32 ebx)
{
    push(app, cpu, eax);
    push(app, cpu, ebx);
    reg32 left = size, page = 0xfffffffc;
    do
    {
        put(app, cpu.esp + page, page);
        page -= 0x1000;
        left -= 0x1000;
    } while (x86::sreg32(left) > 0);
    cpu.esp += 4 + 4;  // the two registers popped; they were not changed
}

x86::reg32 GRO(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    stackProbedOf(app, cpu, at(app, cpu.esp + 4), args.eax, args.ebx);
    cpu.esp += 4 + 4;
    return args.eax;
}

/* 0x4ee15c (__MkTmpFile's digit): `n`, 0 to 15, as a hexadecimal digit. */
reg32 hexDigitOf(reg32 n)
{
    n += '0';
    return x86::sreg32(n) > '9' ? n + 0x27 : n;
}

x86::reg32 hexDigit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    returns(cpu, args, hexDigitOf(args.eax));
    return args.eax;
}

/* _fullpath (clib/misc/c/fullpath.c, 0x523610): the path `path` made whole into `buffer`, `size`
 * bytes of it -- 0x104 from _nmalloc for no buffer -- by GetFullPathNameA; "con" as it is, no path
 * or an empty one the current directory.  0 for an error.  For "con" in fewer than 4 bytes the
 * code frees what it holds in edi -- the block it took, or, given a buffer, `edi` as the caller
 * left it.  At its entry's depth: three pushes, then the dword GetFullPathNameA points at the
 * name in.  0 with cpu.terminate set when the game stopped. */
reg32 fullpathOf(win32::WinApplication* app, x86::CPU& cpu, reg32 buffer, reg32 path, reg32 size, reg32 edi)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 namePart = frame - 4;
    if (buffer == 0)
    {
        size = 0x104;
        buffer = edi = clib::malloc(app, cpu, size);
        if (cpu.terminate)
            return 0;
        if (buffer == 0)
        {
            called(cpu, [&] { return setThreadWord(app, cpu, kErrno, 5); });  // __set_errno's ENOMEM
            return 0;
        }
    }
    if (path == 0 || byte(app, path) == 0)
        return runs(app, cpu, &getcwd, {buffer, size});
    if (clib::stricmp(app, path, kConsoleName) == 0)
    {
        if (size < 4)
        {
            clib::free(app, cpu, edi);
            if (cpu.terminate)
                return 0;
            called(cpu, [&] { return setThreadWord(app, cpu, kErrno, 0xe); });  // __set_errno's ERANGE
            return 0;
        }
        put(app, buffer, at(app, kConsoleName));
        return buffer;
    }
    reg32 length;
    {
        const EspKept kept(cpu);
        const auto made = windows<kGetFullPathNameA, &host::fullPathName>(app, cpu, path, size, buffer, namePart);
        if (!made)
            return 0;
        length = *made;
    }
    if (length == 0)
    {
        called(cpu, [&] { return setErrnoNt(app, cpu); });  // __set_errno_nt's
        return 0;
    }
    return buffer;
}

x86::reg32 fullpath(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 result;
    {
        const EspKept kept(cpu);
        result = fullpathOf(app, cpu, args.eax, args.edx, args.ebx, args.edi);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x5094e0 (__tmpdir): the directory of temporary files, once: the first of TMP, TEMP, TMPDIR,
 * TEMPDIR that fits, made whole by _fullpath, or the current one; a '\\' after it.  At its entry's
 * depth: five pushes.  0 with cpu.terminate set when the game stopped. */
reg32 tmpdirOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0);
    auto length = [&](reg32 s) {
        reg32 n = 0;
        while (byte(app, s + n) != 0)
            ++n;
        return n;
    };
    if (byte(app, kTmpDir) != 0)
        return kTmpDir;
    for (reg32 variable = kTmpVariables; byte(app, at(app, variable)) != 0; variable += 4)
    {
        reg32 value;
        {
            const ReturnSlot call(cpu);
            value = getenvOf(app, cpu, at(app, variable));
        }
        if (cpu.terminate)
            return 0;
        if (value == 0 || length(value) > 0x103)
            continue;
        {
            const ReturnSlot call(cpu);
            // edi past the value, as the inline strlen left it
            fullpathOf(app, cpu, kTmpDir, value, 0x103, value + length(value) + 1);
        }
        if (cpu.terminate)
            return 0;
        break;
    }
    if (byte(app, kTmpDir) == 0)
    {
        // The current directory, in a block of getcwd's, copied in (the block kept, as the code
        // keeps it).
        const reg32 current = runs(app, cpu, &getcwd, {0, 0});
        if (cpu.terminate)
            return 0;
        for (reg32 i = 0;; ++i)
        {
            const reg8 c = byte(app, current + i);
            putByte(app, kTmpDir + i, c);
            if (c == 0)
                break;
        }
    }
    const reg32 last = kTmpDir + length(kTmpDir) - 1;
    if (byte(app, last) != '\\' && byte(app, last) != '/')
    {
        putByte(app, last + 1, '\\');
        putByte(app, last + 2, 0);
    }
    return kTmpDir;
}

x86::reg32 tmpdir(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 directory;
    {
        const EspKept kept(cpu);
        directory = tmpdirOf(app, cpu);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, directory);
    return args.eax;
}

/* __MkTmpFile (clib/streamio/c/tmputil.c, 0x4ee168): a temporary file's name into `name` -- the
 * directory, then t, the process's id in four hexadecimal digits, _, two of `number`'s, .tmp.
 * At its entry's depth: five pushes and a dword.  What the code leaves in eax (the last digit);
 * 0 with cpu.terminate set when the game stopped. */
reg32 mkTmpFileOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 number)
{
    const GuestFrame frame(cpu, 0x14, 4);
    reg32 id;
    {
        const ReturnSlot call(cpu);
        id = getpidOf(app, cpu);
    }
    if (cpu.terminate)
        return 0;
    reg32 directory;
    {
        const ReturnSlot call(cpu);
        directory = tmpdirOf(app, cpu);
    }
    if (cpu.terminate)
        return 0;
    reg32 length = 0;
    for (;; ++length)
    {
        const reg8 c = byte(app, directory + length);
        putByte(app, name + length, c);
        if (c == 0)
            break;
    }
    const reg32 p = name + length;
    putByte(app, p, 't');
    reg32 digits = id >> 16 | id;
    for (reg32 at = p + 4; at != p; --at, digits >>= 4)
        putByte(app, at, reg8(hexDigitOf(digits & 0xf)));
    putByte(app, p + 5, '_');
    putByte(app, p + 6, reg8(hexDigitOf(reg32(x86::sreg32(number) >> 4) & 0xf)));
    const reg32 last = hexDigitOf(number & 0xf);
    putByte(app, p + 7, reg8(last));
    putByte(app, p + 8, '.');
    putByte(app, p + 9, 't');
    putByte(app, p + 0xa, 'm');
    putByte(app, p + 0xb, 'p');
    putByte(app, p + 0xc, 0);
    return last;
}

x86::reg32 MkTmpFile(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 eax;
    {
        const EspKept kept(cpu);
        eax = mkTmpFileOf(app, cpu, args.eax, args.edx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
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

constexpr reg32 kWideEnviron = 0xa0b158;  // _wenviron
constexpr reg32 kEnvMask = 0xa0b150;      // __env_mask: a byte each, 1 for a string the runtime made
constexpr reg32 kHeapFirstBlock = 0x566f4c;  // __nheapbeg
constexpr reg32 kHeapClean = 0xa0b141;    // __nheap_clean
constexpr reg32 kHeapLock = 0x567780, kHeapUnlock = 0x567788;
constexpr reg32 kSetEnvironmentVariableA = 0x5345dc, kMultiByteToWideChar = 0x534598;

/* The miniheap that has the block at `block`, as __HeapManager_expand finds it: from __nheapbeg on,
 * the last one if none; for another segment than ds, `esi` as it was. */
reg32 heapOf(win32::WinApplication* app, const x86::CPU& cpu, reg16 segment, reg32 block, reg32 esi)
{
    (void)cpu;
    if (segment != 0)  // ds: the flat model's 0
        return esi;
    reg32 heap = at(app, kHeapFirstBlock);
    for (; at(app, heap + 8) != 0; heap = at(app, heap + 8))
        if (heap <= block && at(app, heap) + heap > block)
            return heap;
    return heap;
}

}  // namespace

/* _mbslen (clib/mbyte/c/mbslen.c, 0x518050): the characters in the string `s`. */
reg32 mbslenOf(win32::WinApplication* app, reg32 s)
{
    reg32 n = 0;
    for (; mbtermOf(app, s) == 0; s = mbsincOf(app, s))
        ++n;
    return n;
}

x86::reg32 mbslen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, mbslenOf(app, args.eax));
    return args.eax;
}

/* mbtowc (clib/mbyte/c/mbtowc.c, 0x523c00): the character at `from`, at most `most` bytes of it,
 * by MultiByteToWideChar into the word at `to`; its bytes, 0 for none, -1 for one it cannot make.
 * At its entry's depth: four pushes, then the dword MultiByteToWideChar writes the character into.
 * 0 with cpu.terminate set when the game stopped. */
reg32 mbtowcOf(win32::WinApplication* app, x86::CPU& cpu, reg32 to, reg32 from, reg32 most)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 wide = frame - 4;
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
    const reg32 bytes = mbclenOf(app, from);
    reg32 made;
    {
        const EspKept kept(cpu);
        // MB_ERR_INVALID_CHARS
        const auto n = windows<kMultiByteToWideChar, &host::multiByteToWide>(app, cpu, at(app, kCodePage), 8, from,
                                                                             most < bytes ? most : bytes, wide, 1);
        if (!n)
            return 0;
        made = *n;
    }
    if (made == 0)
        return 0xffffffff;
    if (to != 0)
        putHalf(app, to, reg16(at(app, wide)));
    return bytes;
}

x86::reg32 mbtowc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 bytes;
    {
        const EspKept kept(cpu);
        bytes = mbtowcOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, bytes);
    return args.eax;
}

/* mbstowcs (clib/mbyte/c/mbstowcs.c, 0x518080): the string at `from` into the wide string at `to`,
 * at most `most` characters -- or, for no buffer, how many it would take; -1 for one mbtowc cannot
 * make.  At its entry's depth: four pushes.  0 with cpu.terminate set when the game stopped. */
reg32 mbstowcsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 to, reg32 from, reg32 most)
{
    const GuestFrame frame(cpu, 0x10, 0);
    reg32 made = 0;
    auto one = [&](reg32 into) {
        const ReturnSlot call(cpu);
        return mbtowcOf(app, cpu, into, from, 2);
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
            const reg32 bytes = one(to);
            if (cpu.terminate)
                return 0;
            if (bytes == 0xffffffff)
                return bytes;
        }
        return made;
    }
    for (; byte(app, from) != 0; from = mbsincOf(app, from), ++made)
    {
        const reg32 bytes = one(0);
        if (cpu.terminate)
            return 0;
        if (bytes == 0xffffffff)
            return bytes;
    }
    return made;
}

x86::reg32 mbstowcs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 made;
    {
        const EspKept kept(cpu);
        made = mbstowcsOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, made);
    return args.eax;
}

/* 0x525300 (wcslen): the characters in the wide string `s`. */
reg32 wcslenOf(win32::WinApplication* app, reg32 s)
{
    reg32 n = 0;
    while (half(app, s + 2 * n) != 0)
        ++n;
    return n;
}

x86::reg32 wcslen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, wcslenOf(app, args.eax));
    return args.eax;
}

/* The runtime's inline copy of `count` bytes from `from` to `to`: dwords, then the bytes left, each
 * read before it is written, as rep movsd and rep movsb take them. */
void copyForward(win32::WinApplication* app, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count >> 2; ++i, to += 4, from += 4)
        put(app, to, at(app, from));
    for (reg32 i = 0; i < (count & 3); ++i, ++to, ++from)
        putByte(app, to, byte(app, from));
}

/* 0x525320 (memcpy): `count` bytes from `from` to `to`; `to` back. */
reg32 memcpyOf(win32::WinApplication* app, reg32 to, reg32 from, reg32 count)
{
    if (count != 0 && apart(to, count, from, count))
        std::memcpy(hostBytes(app, to), hostBytes(app, from), count);
    else
        copyForward(app, to, from, count);
    return to;
}

x86::reg32 memcpy(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, memcpyOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525350 (wcscpy): the wide string at `from` to `to`; `to` back. */
reg32 wcscpyOf(win32::WinApplication* app, reg32 to, reg32 from)
{
    for (reg32 i = 0;; i += 2)
    {
        const reg16 c = half(app, from + i);
        putHalf(app, to + i, c);
        if (c == 0)
            break;
    }
    return to;
}

x86::reg32 wcscpy(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, wcscpyOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x525370 (towupper, as the environment's functions have it): `c`'s ASCII small letters, in its
 * low word, as capitals -- the dword less 0x20. */
reg32 towupperOf(reg32 c)
{
    const reg16 low = reg16(c);
    return low >= 'a' && low <= 'z' ? c - 0x20 : c;
}

x86::reg32 towupper(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    returns(cpu, args, towupperOf(args.eax));
    return args.eax;
}

/* 0x518640 (memmove): `count` bytes from `from` to `to` -- where they overlap the wrong way
 * backwards, words from the end, then the first byte; else forwards as memcpy does; `to` back. */
reg32 memmoveOf(win32::WinApplication* app, reg32 to, reg32 from, reg32 count)
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
        reg32 s = from + count - 2, d = to + count - 2;
        for (reg32 i = 0; i < count >> 1; ++i, s -= 2, d -= 2)
            putHalf(app, d, half(app, s));
        if (count & 1)
            putByte(app, d + 1, byte(app, s + 1));
        return to;
    }
    copyForward(app, to, from, count);
    return to;
}

x86::reg32 memmove(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, memmoveOf(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x525380 (_msize): the block at `block`'s size, as its header has it. */
reg32 msizeOf(win32::WinApplication* app, reg32 block)
{
    return (at(app, block - 4) & 0xfffffffe) - 4;
}

x86::reg32 msize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, msizeOf(app, args.eax));
    return args.eax;
}

/* 0x5253cc (__HeapManager_expand): the block at `block` in the heap of segment `segment` made
 * `size` bytes, from the free blocks after it or by giving its end back; how much more it still
 * needs into the dword at `more`.  0, 1 for a block after it in use, 2 for the heap's end.  `esi`
 * the caller's, which the miniheap stands for in another segment than ds.  At its entry's depth:
 * three pushes, then 0x10 bytes.  0 with cpu.terminate set when the game stopped. */
reg32 heapExpandOf(win32::WinApplication* app, x86::CPU& cpu, reg32 segment, reg32 block, reg32 size, reg32 more,
                   reg32 esi)
{
    const GuestFrame frame(cpu, 0xc, 0x10);
    reg32 need = (size + 0xb) & 0xfffffff8;
    if (need < size)
        need = 0xffffffff;
    else if (need < 0x10)
        need = 0x10;
    const reg32 header = block - 4;
    const reg32 has = at(app, header) & 0xfffffffe;
    if (need <= has)
    {
        // Smaller: what it no longer needs freed, if it is a block's worth.
        const reg32 rest = has - need;
        if (rest >= 0x10)
        {
            put(app, header, need | 1);
            const reg32 end = header + need;
            put(app, end, rest | 1);
            const reg32 heap = heapOf(app, cpu, reg16(segment), block, esi);
            put(app, heap + 0x18, at(app, heap + 0x18) + 1);
            clib::free(app, cpu, end + 4);
            if (cpu.terminate)
                return 0;
        }
        return 0;
    }
    reg32 wanted = need - has;
    for (reg32 next = header + has;;)
    {
        put(app, more, wanted);
        const reg32 free = at(app, next);
        if (free == 0xffffffff)
            return 2;
        if (free & 1)
            return 1;
        const reg32 after = at(app, next + 8), before = at(app, next + 4);
        const reg32 heap = heapOf(app, cpu, reg16(segment), block, esi);
        if (next == at(app, heap + 0xc))
            put(app, heap + 0xc, at(app, next + 4));
        if (free >= at(app, more) && free - at(app, more) >= 0x10)
        {
            // Split: the rest of the free block stays free after it.
            const reg32 taken = at(app, more);
            const reg32 rest = next + taken;
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

x86::reg32 HeapManager_expand(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = heapExpandOf(app, cpu, args.eax, args.edx, args.ebx, args.ecx, args.esi);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x525578 (_nexpand's way out when it could not): the heap's lock given
 * back, 0, in _nexpand's frame. */
x86::reg32 nexpandFailed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (!hookWith<kHeapUnlock>(app, cpu, args))
        return args.eax;
    args.eax = 0;
    cpu.esp += 4;
    args.esi = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* _nexpand (clib/heap/c/nexpand.c, 0x525390): the block `block` made `size` bytes where it is,
 * under the heap's lock; the block, or 0.  At its entry's depth: three pushes, then the dword
 * __HeapManager_expand counts what it still needs into.  0 with cpu.terminate set when the game
 * stopped. */
reg32 nexpandOf(win32::WinApplication* app, x86::CPU& cpu, reg32 block, reg32 size)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 more = frame - 4;
    if (!hook<kHeapLock>(app, cpu))
        return 0;
    reg32 failed;
    {
        const ReturnSlot slot(cpu);
        failed = heapExpandOf(app, cpu, 0, block, size, more, block);  // ds: the flat model's 0
    }
    if (cpu.terminate || !hook<kHeapUnlock>(app, cpu))
        return 0;
    return failed != 0 ? 0 : block;
}

x86::reg32 nexpand(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 block = nexpandOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, block);
    return args.eax;
}

/* _nrealloc (clib/heap/c/nrealloc.c, 0x5185c0): the block `block` made `size` bytes -- where it is
 * if it can, else a new one with it copied, given back as it was where there is none -- _nmalloc's
 * for none, _nfree's for 0 bytes; the block, or 0.  At its entry's depth: five pushes.  0 with
 * cpu.terminate set when the game stopped. */
reg32 nreallocOf(win32::WinApplication* app, x86::CPU& cpu, reg32 block, reg32 size)
{
    const GuestFrame frame(cpu, 0x14, 0);
    if (block == 0)
        return clib::malloc(app, cpu, size);
    if (size == 0)
    {
        clib::free(app, cpu, block);
        return 0;
    }
    const reg32 had = msizeOf(app, block);
    auto expand = [&](reg32 bytes) {
        const ReturnSlot slot(cpu);
        return nexpandOf(app, cpu, block, bytes);
    };
    const reg32 there = expand(size);
    if (cpu.terminate || there != 0)
        return there;
    const reg32 moved = clib::malloc(app, cpu, size);
    if (cpu.terminate)
        return 0;
    if (moved != 0)
    {
        copyForward(app, moved, block, had);
        clib::free(app, cpu, block);
        return moved;
    }
    expand(had);
    return 0;
}

x86::reg32 nrealloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 block = nreallocOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, block);
    return args.eax;
}

/* 0x4f8f38 (__findenv): the variable named `name` in _environ, its name
 * matched as capitals: its index + 1 -- or, for no `value`, taken out of
 * _environ and __env_mask, the runtime's string freed, and the search going on
 * past the entry that moved into its place -- or, not there, minus how many
 * there are.  0 with cpu.terminate set when the game stopped. */
reg32 findenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 value)
{
    reg32 entry = at(app, kEnviron);
    for (reg32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
    {
        for (reg32 s = string, n = name;; ++s, ++n)
        {
            if (byte(app, s) == '=' && byte(app, n) == 0)
            {
                const reg32 index = reg32(x86::sreg32(entry - at(app, kEnviron)) >> 2);
                if (value != 0)
                    return index + 1;
                // Taken out: the entries after it one down, the terminator too.
                reg32 end = entry, moved;
                do
                {
                    moved = at(app, end + 4);
                    put(app, end, moved);
                    end += 4;
                } while (moved != 0);
                const reg32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, cpu, string);
                        if (cpu.terminate)
                            return 0;
                    }
                    // __env_mask, which follows the entries, after them again, its byte taken out.
                    const reg32 count = reg32(x86::sreg32(end - at(app, kEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    if (cpu.terminate)
                        return 0;
                    put(app, kEnvMask, end);
                    for (reg32 i = index; x86::sreg32(i) < x86::sreg32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                break;  // '=' and the name's end: no capital alike, as the runtime goes on
            }
            if (clib::toupper(app, byte(app, s)) != clib::toupper(app, byte(app, n)) || byte(app, n) == 0)
                break;
        }
    }
    return reg32(x86::sreg32(at(app, kEnviron) - entry) >> 2);
}

x86::reg32 findenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 found = findenvOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, found);
    return args.eax;
}

/* __setenv (clib/environ/c/setenv.c, 0x4f8ce8): the variable named `name`
 * set to the string `value` in _environ -- replaced if `replace` says so,
 * taken out for none -- as a string of the runtime's, _environ and __env_mask
 * grown as it takes; 0, or -1.  0 with cpu.terminate set when the game
 * stopped. */
reg32 setenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 value, reg32 replace)
{
    if (name == 0 || byte(app, name) == 0)
        return 0xffffffff;
    reg32 environment = at(app, kEnviron);
    reg32 index;
    if (environment == 0)
    {
        if (value == 0)
            return 0;
        // _environ made: no entry, its terminator, and __env_mask after it.
        environment = clib::malloc(app, cpu, 9);
        if (cpu.terminate)
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
        const reg32 found = findenvOf(app, cpu, name, value);
        if (cpu.terminate)
            return 0;
        if (value == 0)
            return 0;
        if (x86::sreg32(found) > 0)
        {
            if (replace == 0)
                return 0;
            index = found - 1;
        }
        else
        {
            // Not there: _environ one longer, __env_mask after it.
            index = 0 - found;
            const reg32 entries = index + 1;
            const reg32 masked = index * 4 + 8;  // where __env_mask goes in the block
            const reg32 bytes = entries + masked;
            if (at(app, kEnvMask) == 0)
            {
                const reg32 old = environment;
                environment = clib::malloc(app, cpu, bytes);
                if (cpu.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                for (reg32 i = 0; i < index * 4; ++i)
                    putByte(app, environment + i, byte(app, old + i));
                put(app, kEnvMask, environment + masked);
                clib::memset(app, environment + masked, 0, entries);
            }
            else
            {
                environment = clib::realloc(app, cpu, environment, bytes);
                if (cpu.terminate)
                    return 0;
                if (environment == 0)
                    return 0xffffffff;
                memmoveOf(app, environment + masked, at(app, kEnvMask), index);
                if (cpu.terminate)
                    return 0;
                put(app, kEnvMask, environment + masked);
            }
            put(app, environment + index * 4 + 4, 0);
            put(app, kEnviron, environment);
            putByte(app, at(app, kEnvMask) + index, 0);
        }
    }
    // name=value into a string of the runtime's: the old one grown, or anew.
    auto length = [&](reg32 string) {
        reg32 n = 0;
        while (byte(app, string + n) != 0)
            ++n;
        return n;
    };
    const reg32 nameLength = length(name);
    const reg32 old = byte(app, at(app, kEnvMask) + index) != 0 ? at(app, environment + index * 4) : 0;
    const reg32 string = clib::realloc(app, cpu, old, nameLength + length(value) + 2);
    if (cpu.terminate)
        return 0;
    if (string == 0)
        return 0xffffffff;
    for (reg32 i = 0; i < nameLength; ++i)
        putByte(app, string + i, byte(app, name + i));
    putByte(app, string + nameLength, '=');
    for (reg32 i = 0;; ++i)
    {
        const reg8 c = byte(app, value + i);
        putByte(app, string + nameLength + 1 + i, c);
        if (c == 0)
            break;
    }
    put(app, environment + index * 4, string);
    putByte(app, at(app, kEnvMask) + index, 1);
    return 0;
}

x86::reg32 setenv_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = setenvOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x518498 (__wfindenv): __findenv's for _wenviron, the wide name `name`: its index + 1 -- or,
 * for no `value`, taken out, the search going on past the entry that moved into its place -- or,
 * not there, minus how many there are.  0 with cpu.terminate set when the game stopped. */
reg32 wfindenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 value)
{
    reg32 entry = at(app, kWideEnviron);
    for (reg32 string = at(app, entry); string != 0; entry += 4, string = at(app, entry))
    {
        for (reg32 s = string, n = name;; s += 2, n += 2)
        {
            if (half(app, s) == '=' && half(app, n) == 0)
            {
                const reg32 index = reg32(x86::sreg32(entry - at(app, kWideEnviron)) >> 2);
                if (value != 0)
                    return index + 1;
                reg32 end = entry, moved;
                do
                {
                    moved = at(app, end + 4);
                    put(app, end, moved);
                    end += 4;
                } while (moved != 0);
                const reg32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, cpu, string);
                        if (cpu.terminate)
                            return 0;
                    }
                    const reg32 count = reg32(x86::sreg32(end - at(app, kWideEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    put(app, kEnvMask, end);
                    for (reg32 i = index; x86::sreg32(i) < x86::sreg32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                break;  // '=' and the name's end: no capital alike, as the runtime goes on
            }
            if (reg16(towupperOf(half(app, s))) != reg16(towupperOf(half(app, n))) || half(app, n) == 0)
                break;
        }
    }
    return reg32(x86::sreg32(at(app, kWideEnviron) - entry) >> 2);
}

x86::reg32 wfindenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 found = wfindenvOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, found);
    return args.eax;
}

/* 0x52c300 (__wputenv's find): the wide "name=value" `pair`'s name in _wenviron, matched as
 * capitals: its index + 1 -- or, for `remove`, taken out, and 0 -- or minus how many there are.
 * 0 with cpu.terminate set when the game stopped. */
reg32 wputenvFindOf(win32::WinApplication* app, x86::CPU& cpu, reg32 pair, reg32 remove)
{
    reg32 entry = at(app, kWideEnviron);
    for (; at(app, entry) != 0; entry += 4)
    {
        if (half(app, pair) == 0)
            continue;
        for (reg32 s = at(app, entry), n = pair;; s += 2, n += 2)
        {
            if (reg16(towupperOf(half(app, s))) != reg16(towupperOf(half(app, n))))
                break;
            if (half(app, s) == '=')
            {
                const reg32 index = reg32(x86::sreg32(entry - at(app, kWideEnviron)) >> 2);
                if (remove == 0)
                    return index + 1;
                const reg32 removed = at(app, entry);
                reg32 end = entry;
                if (removed != 0)
                {
                    reg32 moved;
                    do
                    {
                        put(app, end, at(app, end + 4));
                        moved = at(app, end + 4);
                        end += 4;
                    } while (moved != 0);
                }
                const reg32 mask = at(app, kEnvMask);
                if (mask != 0)
                {
                    if (byte(app, index + mask) != 0)
                    {
                        clib::free(app, cpu, removed);
                        if (cpu.terminate)
                            return 0;
                    }
                    const reg32 count = reg32(x86::sreg32(end - at(app, kWideEnviron)) >> 2);
                    memmoveOf(app, end, at(app, kEnvMask), count);
                    put(app, kEnvMask, end);
                    for (reg32 i = index; x86::sreg32(i) < x86::sreg32(count); ++i)
                        putByte(app, end + i, byte(app, end + i + 1));
                }
                return 0;
            }
            if (half(app, n + 2) == 0)
                break;
        }
    }
    return reg32(x86::sreg32(at(app, kWideEnviron) - entry) >> 2);
}

x86::reg32 wputenvFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 found = wputenvFindOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, found);
    return args.eax;
}

/* 0x52c178 (__wputenv): the wide "name=value" `pair` into _wenviron as it is -- "name=" takes it
 * out -- _wenviron and __env_mask grown as it takes; 0, or -1.  For an empty `pair` the code looks
 * for its '=' at `edx` as the caller left it.  At its entry's depth: six pushes, then 8 bytes.  0
 * with cpu.terminate set when the game stopped. */
reg32 wputenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 pair, reg32 edx)
{
    const GuestFrame frame(cpu, 0x18, 8);
    if (pair == 0)
        return 0xffffffff;
    reg32 equals = edx;
    if (half(app, pair) != 0)
        for (equals = pair + 2; half(app, equals) != 0 && half(app, equals) != '='; equals += 2)
            if (half(app, equals + 2) == 0)
            {
                equals += 2;
                break;
            }
    if (half(app, equals) == 0)
        return 0xffffffff;
    const reg32 remove = half(app, equals + 2) == 0 ? 1 : 0;
    reg32 environment = at(app, kWideEnviron);
    reg32 index;
    if (environment == 0)
    {
        if (remove != 0)
            return 0;
        environment = clib::malloc(app, cpu, 9);
        if (cpu.terminate)
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
        reg32 found;
        {
            const ReturnSlot slot(cpu);
            found = wputenvFindOf(app, cpu, pair, remove);
        }
        if (cpu.terminate)
            return 0;
        if (remove != 0)
            return 0;
        if (x86::sreg32(found) > 0)
            index = found - 1;
        else
        {
            index = 0 - found;
            const reg32 entries = index + 1, masked = index * 4 + 8, bytes = masked + entries;
            if (at(app, kEnvMask) == 0)
            {
                const reg32 old = environment;
                environment = clib::malloc(app, cpu, bytes);
                if (cpu.terminate)
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
                    const ReturnSlot slot(cpu);
                    environment = nreallocOf(app, cpu, environment, bytes);
                }
                if (cpu.terminate)
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

x86::reg32 wputenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = wputenvOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x5252a0 (__create_wide_environment): _wenviron made from _environ, a string at a time by
 * mbstowcs into a block of _nmalloc's, which __wputenv takes -- or, one mbstowcs cannot make,
 * _nfree.  At its entry's depth: four pushes.  What the code leaves in eax, given `eax` on entry;
 * 0 with cpu.terminate set when the game stopped. */
reg32 createWideEnvironmentOf(win32::WinApplication* app, x86::CPU& cpu, reg32 eax)
{
    const GuestFrame frame(cpu, 0x10, 0);
    const reg32 environment = at(app, kEnviron);
    if (environment == 0)
        return eax;
    for (reg32 entry = environment; at(app, entry) != 0; entry += 4)
    {
        const reg32 string = at(app, entry);
        const reg32 characters = mbslenOf(app, string) + 1;
        const reg32 wide = clib::malloc(app, cpu, characters * 2);
        if (cpu.terminate)
            return 0;
        eax = wide;
        if (wide == 0)
            continue;
        reg32 made;
        {
            const ReturnSlot slot(cpu);
            made = mbstowcsOf(app, cpu, wide, string, characters);
        }
        if (cpu.terminate)
            return 0;
        if (made != 0xffffffff)
        {
            const ReturnSlot slot(cpu);
            eax = wputenvOf(app, cpu, wide, string);
        }
        else
            eax = clib::free(app, cpu, wide);
        if (cpu.terminate)
            return 0;
    }
    return eax;
}

x86::reg32 create_wide_environment(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = createWideEnvironmentOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* __wsetenv (clib/environ/c/wsetenv.c, 0x518290): __setenv's for _wenviron -- the wide variable
 * `name` set to `value`, replaced if `replace` says so, taken out for none -- made from _environ
 * first if there is none; 0, or -1.  At its entry's depth: four pushes, then 0x14 bytes.  0 with
 * cpu.terminate set when the game stopped. */
reg32 wsetenvOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 value, reg32 replace)
{
    const GuestFrame frame(cpu, 0x10, 0x14);
    if (name == 0 || half(app, name) == 0)
        return 0xffffffff;
    if (at(app, kWideEnviron) == 0)
    {
        {
            const ReturnSlot slot(cpu);
            createWideEnvironmentOf(app, cpu, 0);
        }
        if (cpu.terminate)
            return 0;
    }
    reg32 environment = at(app, kWideEnviron);
    reg32 index;
    if (environment == 0)
    {
        if (value == 0)
            return 0;
        environment = clib::malloc(app, cpu, 9);
        if (cpu.terminate)
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
        const reg32 found = wfindenvOf(app, cpu, name, value);
        if (cpu.terminate)
            return 0;
        if (value == 0)
            return 0;
        if (x86::sreg32(found) > 0)
        {
            if (replace == 0)
                return 0;
            index = found - 1;
        }
        else
        {
            index = 0 - found;
            const reg32 entries = index + 1, masked = index * 4 + 8, bytes = masked + entries;
            if (at(app, kEnvMask) == 0)
            {
                const reg32 old = environment;
                environment = clib::malloc(app, cpu, bytes);
                if (cpu.terminate)
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
                    const ReturnSlot slot(cpu);
                    environment = nreallocOf(app, cpu, environment, bytes);
                }
                if (cpu.terminate)
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
    const reg32 nameLength = wcslenOf(app, name);
    const reg32 old = byte(app, index + at(app, kEnvMask)) != 0 ? at(app, environment + index * 4) : 0;
    reg32 string;
    {
        const ReturnSlot slot(cpu);
        string = nreallocOf(app, cpu, old, (wcslenOf(app, value) + nameLength) * 2 + 4);
    }
    if (cpu.terminate)
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

x86::reg32 wsetenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = wsetenvOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* setenv (clib/environ/c/setenv.c, 0x4f8b50): the variable `name` set to the string `value` --
 * replaced if `replace` says so, an empty one taking it out -- in Windows' environment by
 * SetEnvironmentVariableA, in _environ, and in _wenviron if there is one, its name and value made
 * wide by mbstowcs; 0, or -1.  At its entry's depth: four pushes, then 0x10 bytes.  0 with
 * cpu.terminate set when the game stopped. */
reg32 setenvTopOf(win32::WinApplication* app, x86::CPU& cpu, reg32 name, reg32 value, reg32 replace)
{
    const GuestFrame frame(cpu, 0x10, 0x10);
    auto lookup = [&] {
        const ReturnSlot slot(cpu);
        return getenvOf(app, cpu, name);
    };
    if (value != 0 && byte(app, value) == 0)
    {
        bool none = replace != 0;
        if (!none)
        {
            none = lookup() == 0;
            if (cpu.terminate)
                return 0;
        }
        if (none)
            value = 0;
    }
    bool toWindows = true;
    if (replace == 0)
    {
        toWindows = lookup() == 0;
        if (cpu.terminate)
            return 0;
    }
    if (toWindows)
    {
        reg32 set;
        {
            const EspKept kept(cpu);
            const auto said = windows<kSetEnvironmentVariableA, &host::setEnvironmentVariable>(app, cpu, name, value);
            if (!said)
                return 0;
            set = *said;
        }
        if (set == 0)
            return 0xffffffff;
    }
    reg32 failed;
    {
        const ReturnSlot slot(cpu);
        failed = setenvOf(app, cpu, name, value, replace);
    }
    if (cpu.terminate)
        return 0;
    if (failed != 0)
        return 0xffffffff;
    if (at(app, kWideEnviron) == 0)
        return 0;
    auto noMemory = [&] {
        called(cpu, [&] { return setErrnoDos(app, cpu, 5); });  // __set_errno_dos's ENOMEM
        return cpu.terminate ? 0 : 0xffffffff;
    };
    const reg32 nameCharacters = mbslenOf(app, name) + 1;
    const reg32 wideName = clib::malloc(app, cpu, nameCharacters * 2);
    if (cpu.terminate)
        return 0;
    if (wideName == 0)
        return noMemory();
    reg32 valueCharacters = 0, wideValue = 0;
    if (value != 0)
    {
        valueCharacters = mbslenOf(app, value) + 1;
        wideValue = clib::malloc(app, cpu, valueCharacters * 2);
        if (cpu.terminate)
            return 0;
        if (wideValue == 0)
        {
            clib::free(app, cpu, wideName);
            if (cpu.terminate)
                return 0;
            return noMemory();
        }
    }
    auto freeBoth = [&] {
        clib::free(app, cpu, wideName);
        if (!cpu.terminate)
            clib::free(app, cpu, wideValue);
        return cpu.terminate ? 0 : 0xffffffff;
    };
    auto widen = [&](reg32 into, reg32 from, reg32 characters) {
        const ReturnSlot slot(cpu);
        return mbstowcsOf(app, cpu, into, from, characters);
    };
    if (widen(wideName, name, nameCharacters) == 0xffffffff)
        return freeBoth();
    if (cpu.terminate)
        return 0;
    if (wideValue != 0)
    {
        if (widen(wideValue, value, valueCharacters) == 0xffffffff)
            return freeBoth();
        if (cpu.terminate)
            return 0;
    }
    reg32 result;
    {
        const ReturnSlot slot(cpu);
        result = wsetenvOf(app, cpu, wideName, wideValue, replace);
    }
    if (cpu.terminate)
        return 0;
    clib::free(app, cpu, wideName);
    if (cpu.terminate)
        return 0;
    if (wideValue != 0)
    {
        clib::free(app, cpu, wideValue);
        if (cpu.terminate)
            return 0;
    }
    return result;
}

x86::reg32 setenv(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = setenvTopOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
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
 * depth.  The long double is the port's IEEEf80, its x87 operations the same
 * the game's code has for them, so a number comes out to the bit as the
 * game's code makes it. ---- */
namespace
{

// The specs __scnf hands its conversions: the routines that take a character
// and put one back, the string sscanf reads, the field's width, its flags.
constexpr reg32 kScanGet = 0x0, kScanUnget = 0x4, kScanString = 0x8, kScanWidth = 0xc, kScanFlags = 0x10;
constexpr reg8 kAssign = 0x01, kEndOfInput = 0x02, kFarPointer = 0x04, kNearPointer = 0x08;
constexpr reg8 kShortVar = 0x10, kLongVar = 0x20, kLongLongVar = 0x40, kSegmented = 0x80;
constexpr reg32 kBitMasks = 0x5688a0;      // 1, 2, 4 ... 0x80: a [set]'s bits
constexpr reg32 kScanFloat = 0x56ac68;     // __EFG_scanf: the string to a double
constexpr reg32 kPowersOfTen = 0x568ba4;   // 10^1, 10^2, 10^4 ... as long doubles, 10 bytes each
constexpr reg32 kHugeVal = 0x55217c;       // HUGE_VAL, a double
constexpr reg32 kCgetString = 0x4f04f0, kUncgetString = 0x4f051c;

reg8 scanFlags(win32::WinApplication* app, reg32 specs)
{
    return byte(app, specs + kScanFlags);
}

/* The va_list at `argp` moved on past a conversion's pointer: `step` its new place; the pointer in
 * `target` -- unless it is a far one, whose les is not the game's code's either, `target` then
 * left as it is.  The code keeps the two in registers, and where they are the same register the
 * pointer is the va_list's place: callers alias them as their code does. */
void scanArgument(win32::WinApplication* app, reg32 argp, bool far, reg32& step, reg32& target)
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
reg32 scanPointer(win32::WinApplication* app, reg8 flags, reg32 argp, reg32 esi)
{
    reg32 step, target = esi;
    scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
    return target;
}

void scanEpilogue(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> registers)
{
    for (reg32* r : registers)
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 96 bits, edx:ecx:ebp, shifted left by one: the carry out. */
bool shift96(reg32& high, reg32& middle, reg32& low)
{
    const bool carry = high >> 31;
    high = high << 1 | middle >> 31;
    middle = middle << 1 | low >> 31;
    low <<= 1;
    return carry;
}

/* 96 bits added to 96: the carry out. */
bool add96(reg32& high, reg32& middle, reg32& low, reg32 high2, reg32 middle2, reg32 low2)
{
    const x86::reg64 l = x86::reg64(low) + low2;
    const x86::reg64 m = x86::reg64(middle) + middle2 + (l >> 32);
    const x86::reg64 h = x86::reg64(high) + high2 + (m >> 32);
    low = reg32(l);
    middle = reg32(m);
    high = reg32(h);
    return (h >> 32) != 0;
}

}  // namespace

/* 0x4f04f0 (cget_string): sscanf's next character, from the string in its specs `specs`, or -1 at
 * its end, which the specs' flags note. */
reg32 cgetStringOf(win32::WinApplication* app, reg32 specs)
{
    const reg32 s = at(app, specs + kScanString);
    if (const reg8 c = byte(app, s))
    {
        put(app, specs + kScanString, s + 1);
        return c;
    }
    putByte(app, specs + kScanFlags, byte(app, specs + kScanFlags) | kEndOfInput);
    return 0xffffffff;
}

x86::reg32 cget_string(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, cgetStringOf(app, args.eax));
    return args.eax;
}

/* 0x4f051c (uncget_string): the character back -- the string's pointer in the specs `specs` back by
 * one. */
void uncgetStringOf(win32::WinApplication* app, reg32 specs)
{
    put(app, specs + kScanString, at(app, specs + kScanString) - 1);
}

x86::reg32 uncget_string(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    uncgetStringOf(app, args.edx);  // eax left as it was, the character
    cpu.esp += 4;
    return args.eax;
}

/* 0x50c640 (cgetc): the next character, by the specs' routine -- the game's, by address, eax and
 * edx the specs as the code hands them.  At its entry's depth: a push.  0 with cpu.terminate set
 * when the game stopped. */
reg32 cgetcOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs)
{
    const GuestFrame frame(cpu, 4, 0);
    const EspKept kept(cpu);
    return through(app, cpu, at(app, specs + kScanGet), {specs, specs}).value_or(0);
}

x86::reg32 cgetc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 c = cgetcOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, c);
    return args.eax;
}

/* 0x50c648 (uncget): the character `c` back, by the specs' routine -- the game's, by address, ebx
 * and edx the specs.  At its entry's depth: a push.  What the routine leaves in eax; 0 with
 * cpu.terminate set when the game stopped. */
reg32 uncgetOf(win32::WinApplication* app, x86::CPU& cpu, reg32 c, reg32 specs)
{
    const GuestFrame frame(cpu, 4, 0);
    const EspKept kept(cpu);
    return through(app, cpu, at(app, specs + kScanUnget), {c, specs, specs}).value_or(0);
}

x86::reg32 uncget(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = uncgetOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x50d538 (chktrm): the next character while the field's width lasts -- -1 when it is over, or
 * the input is.  At its entry's depth: two pushes.  0 with cpu.terminate set when the game
 * stopped. */
reg32 chktrmOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs)
{
    const GuestFrame frame(cpu, 8, 0);
    const reg32 width = at(app, specs + kScanWidth);
    put(app, specs + kScanWidth, width - 1);
    if (width == 0)
        return 0xffffffff;
    reg32 c;
    {
        const ReturnSlot slot(cpu);
        c = cgetcOf(app, cpu, specs);
    }
    if (cpu.terminate)
        return 0;
    return scanFlags(app, specs) & kEndOfInput ? 0xffffffff : c;
}

x86::reg32 chktrm(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 c = chktrmOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, c);
    return args.eax;
}

/* 0x50d510 (radix_value): the digit `c` is, in any base up to 16; 16 for none. */
reg32 radixValueOf(win32::WinApplication* app, reg32 c)
{
    if (x86::sreg32(c) >= 0x30 && x86::sreg32(c) <= 0x39)
        return c - 0x30;
    c = clib::tolower(app, c);
    if (x86::sreg32(c) >= 0x61 && x86::sreg32(c) <= 0x66)
        return c - 0x57;
    return 0x10;
}

x86::reg32 radix_value(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, radixValueOf(app, args.eax));
    return args.eax;
}

/* 0x50c904 (get_opt): the format's options after a '%', at `p`, into the specs `specs` -- '*', the
 * width, N or F, then h, l, w, L or I64; past them. */
reg32 getOptOf(win32::WinApplication* app, reg32 p, reg32 specs)
{
    reg8 flags = (byte(app, specs + kScanFlags) | kAssign) & 3;
    put(app, specs + kScanWidth, 0xffffffff);
    putByte(app, specs + kScanFlags, flags);
    if (byte(app, p) == '*')
    {
        flags &= 0xfe;
        ++p;
        putByte(app, specs + kScanFlags, flags);
    }
    auto digit = [&](reg32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kDigit) != 0; };
    if (digit(byte(app, p)))
    {
        reg32 width = 0;
        for (reg32 c = byte(app, p); digit(c); c = byte(app, ++p))
            width = width * 10 + (c - 0x30);
        put(app, specs + kScanWidth, width);
    }
    auto add = [&](reg8 flag, reg32 past) {
        putByte(app, specs + kScanFlags, byte(app, specs + kScanFlags) | flag);
        p += past;
    };
    if (byte(app, p) == 'N')
        add(kNearPointer, 1);
    else if (byte(app, p) == 'F')
        add(kFarPointer, 1);
    const reg8 size = byte(app, p);
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

x86::reg32 get_opt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, getOptOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50ca08 (scan_white): white space skipped: how much; the character after it put back, unless
 * the input is over.  At its entry's depth: four pushes and a dword.  0 with cpu.terminate set when
 * the game stopped. */
reg32 scanWhiteOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs)
{
    const GuestFrame frame(cpu, 0x10, 4);
    reg32 skipped = 0, c;
    for (;; ++skipped)
    {
        {
            const ReturnSlot slot(cpu);
            c = cgetcOf(app, cpu, specs);
        }
        if (cpu.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
    }
    if (!(scanFlags(app, specs) & kEndOfInput))
    {
        {
            const ReturnSlot slot(cpu);
            uncgetOf(app, cpu, c, specs);
        }
        if (cpu.terminate)
            return 0;
    }
    return skipped;
}

x86::reg32 scan_white(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 skipped = scanWhiteOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, skipped);
    return args.eax;
}

/* 0x50ca48 (scan_char): %c and %C: the field's width of characters, 1 for none, into the argument
 * from the va_list at `argp` -- %C's made wide characters by mbtowc.  How many were read; 0 for a
 * character mbtowc could not make.  `esi` as the caller left it, which a far pointer leaves the
 * target.  At its entry's depth: four pushes, es, a push, then 8 bytes: mbtowc's character and
 * its bytes.  0 with cpu.terminate set when the game stopped. */
reg32 scanCharOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 argp, reg32 esi)
{
    const GuestFrame frame(cpu, 0x18, 8);
    const reg32 wide = frame - 8, bytes = frame - 4;
    reg32 target = esi;
    if (scanFlags(app, specs) & kAssign)
        target = scanPointer(app, scanFlags(app, specs), argp, esi);
    reg32 width = at(app, specs + kScanWidth);
    if (width == 0xffffffff)
        width = 1;
    reg32 read = 0;
    auto next = [&] {
        const ReturnSlot slot(cpu);
        return cgetcOf(app, cpu, specs);
    };
    for (; x86::sreg32(width) > 0; --width)
    {
        const reg32 c = next();
        if (cpu.terminate)
            return 0;
        const reg8 flags = scanFlags(app, specs);
        if (flags & kEndOfInput)
            break;
        ++read;
        if (!(flags & kAssign))
            continue;
        if (!(flags & kLongVar))
        {
            putByte(app, target++, reg8(c));
            continue;
        }
        putByte(app, bytes, reg8(c));
        if (multibyte(app) && leadByte(app, c))
        {
            const reg32 trail = next();
            if (cpu.terminate)
                return 0;
            putByte(app, bytes + 1, reg8(trail));
        }
        reg32 made;
        {
            const ReturnSlot slot(cpu);
            made = mbtowcOf(app, cpu, wide, bytes, 2);
        }
        if (cpu.terminate)
            return 0;
        if (made == 0xffffffff)
            return 0;
        putHalf(app, target, reg16(at(app, wide)));
        target += 2;
    }
    return read;
}

x86::reg32 scan_char(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 read = scanCharOf(app, cpu, args.eax, args.edx, args.esi);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, read);
    return args.eax;
}

/* 0x50cb30 (scan_string): %s and %S: white space skipped, then characters to the next, the field's
 * width at most, into the argument from the va_list at `argp` with a 0 after -- %S's wide.  How
 * many were read; 0 for a character mbtowc could not make.  `esi` as for scan_char.  At its entry's
 * depth: four pushes, es, a push, then 0xc bytes: mbtowc's character, its bytes, the size.  0 with
 * cpu.terminate set when the game stopped. */
reg32 scanStringOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 argp, reg32 esi)
{
    const GuestFrame frame(cpu, 0x18, 0xc);
    const reg32 wide = frame - 0xc, bytes = frame - 8;
    const reg32 size = scanFlags(app, specs) & kLongVar ? 2 : 1;
    putByte(app, frame - 4, reg8(size));
    reg32 target = esi;
    if (scanFlags(app, specs) & kAssign)
        target = scanPointer(app, scanFlags(app, specs), argp, esi);
    auto next = [&] {
        const ReturnSlot slot(cpu);
        return cgetcOf(app, cpu, specs);
    };
    auto space = [&](reg32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace) != 0; };
    reg32 read = 0, c;
    for (;; ++read)
    {
        c = next();
        if (cpu.terminate)
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
        const reg32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width != 0)
            for (;;)
            {
                ++read;
                if (scanFlags(app, specs) & kAssign)
                {
                    if (size == 1)
                        putByte(app, target, reg8(c));
                    else
                    {
                        putByte(app, bytes, reg8(c));
                        if (multibyte(app) && leadByte(app, c))
                        {
                            const reg32 trail = next();
                            if (cpu.terminate)
                                return 0;
                            putByte(app, bytes + 1, reg8(trail));
                        }
                        reg32 made;
                        {
                            const ReturnSlot slot(cpu);
                            made = mbtowcOf(app, cpu, wide, bytes, 2);
                        }
                        if (cpu.terminate)
                            return 0;
                        if (made == 0xffffffff)
                            return 0;
                        putHalf(app, target, reg16(at(app, wide)));
                    }
                    target += size;
                }
                {
                    const ReturnSlot slot(cpu);
                    c = chktrmOf(app, cpu, specs);
                }
                if (cpu.terminate)
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
            const ReturnSlot slot(cpu);
            uncgetOf(app, cpu, c, specs);
        }
        if (cpu.terminate)
            return 0;
    }
    if ((scanFlags(app, specs) & kAssign) && x86::sreg32(read) > 0)
    {
        if (size == 1)
            putByte(app, target, 0);
        else
            putHalf(app, target, 0);
    }
    return read;
}

x86::reg32 scan_string(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 read = scanStringOf(app, cpu, args.eax, args.edx, args.esi);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, read);
    return args.eax;
}

/* 0x50cc88 (report_scan): %n: the characters read so far, `read`, into the argument from the va_list
 * at `argp` -- a short for h.  A far pointer's register is the va_list's place, which the code
 * then stores through. */
void reportScanOf(win32::WinApplication* app, reg32 specs, reg32 argp, reg32 read)
{
    const reg8 flags = scanFlags(app, specs);
    if (!(flags & kAssign))
        return;
    reg32 step, target = argp;
    scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
    if (scanFlags(app, specs) & kShortVar)
        putHalf(app, target, reg16(read));
    else
        put(app, target, read);
}

x86::reg32 report_scan(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reportScanOf(app, args.eax, args.edx, args.ebx);  // eax left as it was, the specs
    cpu.esp += 4;
    return args.eax;
}

/* 0x50cce8 (makelist): a [set]'s characters, the format's at `p`, as bits of the 32 bytes at
 * `set`; past its ']' -- one first is the set's. */
reg32 makelistOf(win32::WinApplication* app, reg32 p, reg32 set)
{
    clib::memset(app, set, 0, 0x20);
    for (reg32 c = byte(app, p++); c != 0;)
    {
        const reg32 at = set + reg32(x86::sreg32(c) >> 3);
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

x86::reg32 makelist(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, makelistOf(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50cd2c (scan_arb): %[: characters in the set -- out of it, for a '^' first -- the field's width
 * at most, into the argument from the va_list at `argp` with a 0 after; the format's pointer, in
 * the dword at `format`, moved past the set.  How many were read.  A far pointer's register is the
 * va_list's new place, which the code then stores through.  At its entry's depth: three pushes,
 * es, a push, then 0x30 bytes, the set's 32 first.  0 with cpu.terminate set when the game
 * stopped. */
reg32 scanArbOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 argp, reg32 format)
{
    const GuestFrame frame(cpu, 0x14, 0x30);
    const reg32 set = frame - 0x30;
    const reg32 outside = byte(app, at(app, format)) == '^' ? 1 : 0;
    if (outside)
        put(app, format, at(app, format) + 1);
    put(app, format, makelistOf(app, at(app, format), set));
    reg32 target = 0;
    if (scanFlags(app, specs) & kAssign)
    {
        reg32 step;
        if (scanFlags(app, specs) & kFarPointer)
        {
            scanArgument(app, argp, true, step, target);
            target = step;
        }
        else
            scanArgument(app, argp, false, step, target);
    }
    reg32 read = 0;
    for (reg32 width = at(app, specs + kScanWidth); width != 0; --width)
    {
        reg32 c;
        {
            const ReturnSlot slot(cpu);
            c = cgetcOf(app, cpu, specs);
        }
        if (cpu.terminate)
            return 0;
        if (scanFlags(app, specs) & kEndOfInput)
            break;
        const bool in = (byte(app, set + reg32(x86::sreg32(c) >> 3)) & byte(app, kBitMasks + (c & 7))) != 0;
        if ((in ? 0 : 1) != outside)
        {
            {
                const ReturnSlot slot(cpu);
                uncgetOf(app, cpu, c, specs);
            }
            if (cpu.terminate)
                return 0;
            break;
        }
        ++read;
        if (scanFlags(app, specs) & kAssign)
            putByte(app, target++, reg8(c));
    }
    if ((scanFlags(app, specs) & kAssign) && x86::sreg32(read) > 0)
        putByte(app, target, 0);
    return read;
}

x86::reg32 scan_arb(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 read = scanArbOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, read);
    return args.eax;
}

/* 0x523ca0 (__FDFS): the double `bits` as a float's bits: 0 for one too small, its sign and
 * infinity for one too big. */
reg32 fdfsOf(x86::reg64 bits)
{
    if (!(reg32(bits >> 32) & 0x7ff00000))
        return 0;
    const reg32 sign = reg32(bits >> 63) << 31;
    const x86::reg64 shifted = (bits << 1) + 0x20000000;
    const reg32 high = reg32(shifted >> 32);
    if (high == 0 || high >= 0x8fe00000)
        return 0x7f800000 | sign;
    if (high < 0x70200000)
        return 0;
    return reg32(((x86::reg64(high - 0x70000000) << 32 | reg32(shifted)) << 2) >> 32) | sign;
}

x86::reg32 FDFS(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    returns(cpu, args, fdfsOf(x86::reg64(args.edx) << 32 | args.eax));
    return args.eax;
}

/* 0x523cec (__U8M): `a` times `b`, 64 bits of it; `cross` the cross products' sum, which the code
 * leaves in ecx. */
x86::reg64 u8mOf(x86::reg64 a, x86::reg64 b, reg32& cross)
{
    cross = reg32(a) * reg32(b >> 32) + reg32(a >> 32) * reg32(b);
    return x86::reg64(reg32(a)) * reg32(b) + (x86::reg64(cross) << 32);
}

x86::reg32 U8M(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    // Its own convention: edx:eax and ecx:ebx in, edx:eax out -- ecx the cross products' sum, but
    // for two numbers of 32 bits, which leave it as it was.
    reg32 cross;
    const x86::reg64 product = u8mOf(x86::reg64(args.edx) << 32 | args.eax, x86::reg64(args.ecx) << 32 | args.ebx, cross);
    if (args.edx != 0 || args.ecx != 0)
        args.ecx = cross;
    args.eax = reg32(product);
    args.edx = reg32(product >> 32);
    cpu.esp += 4;
    return args.eax;
}

/* 0x50ce18 (scan_float): %e, %f, %g, %E and %G: a number's characters -- sign, digits, a point,
 * digits, an exponent -- gathered in the frame, then made a double by __EFG_scanf and stored, as a
 * float but for l and L, into the argument from the va_list at `argp`; with h, the code's fixed
 * point, a short whole part and a short fraction.  How many characters were read; 0 for none of a
 * number.  At its entry's depth: four pushes, es, a push, then 0x6c bytes: the characters from the
 * bottom -- as many as the field gives, its end the code's to overrun -- the double at +0x50, and
 * the fixed point's words, which overlap as the code has them, at +0x5c on.  0 with cpu.terminate
 * set when the game stopped. */
reg32 scanFloatOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 argp)
{
    const GuestFrame frame(cpu, 0x18, 0x6c);
    const reg32 local = frame - 0x6c;
    const reg32 number = local + 0x50, ten = local + 0x5c, fraction = local + 0x60, digit = local + 0x62;
    const reg32 fixed = local + 0x64, whole = local + 0x66, skipped = local + 0x68;
    auto isDigit = [&](reg32 c) { return (byte(app, kIsTable + ((c + 1) & 0xff)) & kDigit) != 0; };
    reg32 p = local, read = 0, c;
    put(app, skipped, 0);
    // The character into the buffer, the next one taken; false at -1.
    auto keep = [&] {
        putByte(app, p++, reg8(c));
        const ReturnSlot slot(cpu);
        c = chktrmOf(app, cpu, specs);
        return c != 0xffffffff;
    };
    auto next = [&] {
        const ReturnSlot slot(cpu);
        c = chktrmOf(app, cpu, specs);
        return c != 0xffffffff;
    };
    bool digits = false;
    for (;;)
    {
        {
            const ReturnSlot slot(cpu);
            c = cgetcOf(app, cpu, specs);
        }
        if (cpu.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
        put(app, skipped, at(app, skipped) + 1);
    }
    if (scanFlags(app, specs) & kEndOfInput)
        goto stored;
    {
        const reg32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width == 0)
            goto putBack;
    }
    if (c == '+' || c == '-')
    {
        putByte(app, p++, reg8(c));
        const bool more = next();
        if (cpu.terminate)
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
            putByte(app, p++, reg8(c));
            if (scanFlags(app, specs) & kShortVar)
                putHalf(app, whole, reg16(c + at(app, whole) * 10 - 0x30));
            ++read;
            const bool more = next();
            if (cpu.terminate)
                return 0;
            if (!more)
                goto stored;
        } while (isDigit(c));
    }
    if (c == '.')
    {
        bool more = keep();
        if (cpu.terminate)
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
            if (cpu.terminate)
                return 0;
            if (!more)
                break;
        }
        if (scanFlags(app, specs) & kShortVar)
        {
            // The fraction's digits, back from the last to the point, made a short's fraction:
            // each into the third byte of the dword, over 10, as the code divides them.
            put(app, fraction, 0);
            for (reg32 q = p; byte(app, q - 1) != '.'; --q)
            {
                put(app, ten, 10);
                putByte(app, digit, reg8(byte(app, q - 1) - 0x30));
                put(app, fraction, at(app, fraction) / 10);
            }
            putHalf(app, fixed, reg16(at(app, fraction)));
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
        if (cpu.terminate)
            return 0;
        if (!more)
            goto stored;
        if (c == '+' || c == '-')
        {
            ++read;
            more = keep();
            if (cpu.terminate)
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
        if (cpu.terminate)
            return 0;
        if (!more)
            goto stored;
    } while (isDigit(c));
putBack:
    {
        const ReturnSlot slot(cpu);
        uncgetOf(app, cpu, c, specs);
    }
    if (cpu.terminate)
        return 0;
stored:
    if (x86::sreg32(read) > 0)
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
                const EspKept kept(cpu);
                if (!hook<kScanFloat>(app, cpu, {local, number}))
                    return 0;
            }
            const reg8 flags = scanFlags(app, specs);
            reg32 step, target = 0;
            scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
            if (flags & kFarPointer)
                target = step;
            const reg8 size = scanFlags(app, specs);
            const reg32 to = target;  // es and ds: the flat model's 0
            if (size & kShortVar)
                put(app, to, at(app, fixed));
            else if (size & (kLongVar | kLongLongVar))
            {
                put(app, to, at(app, number));
                put(app, to + 4, at(app, number + 4));
            }
            else
                put(app, to, fdfsOf(x86::reg64(at(app, number + 4)) << 32 | at(app, number)));
        }
    }
    return read;
}

x86::reg32 scan_float(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 read = scanFloatOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, read);
    return args.eax;
}

/* 0x50d0f0 (scan_int): %d, %i, %o, %u, %x and %p: white space skipped, a sign where `sign` says the
 * number has one, `base`'s digits -- 0 for the number's own: 0x for 16, 0 for 8 -- and for a
 * segmented one a ':' and an offset; stored as the flags say into the argument from the va_list at
 * `argp`, a long long for L and I64.  How many characters were read.  At its entry's depth: two
 * pushes, es, a push, then 0x30 bytes.  0 with cpu.terminate set when the game stopped. */
reg32 scanIntOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 argp, reg32 base, reg32 sign)
{
    const GuestFrame frame(cpu, 0x10, 0x30);
    x86::reg64 value = 0;
    reg32 small = 0, skipped = 0, read = 0, minus = '+', c;
    auto next = [&] {
        const ReturnSlot slot(cpu);
        c = chktrmOf(app, cpu, specs);
        return c != 0xffffffff;
    };
    const bool longLong = (scanFlags(app, specs) & kLongLongVar) != 0;
    auto take = [&](reg32 d) {
        if (longLong)
        {
            reg32 cross;
            value = u8mOf(value, base, cross) + d;
        }
        else
            small = small * base + d;
    };
    auto digitOf = [&](reg32 ch, reg32& d) {
        d = radixValueOf(app, ch);
        return x86::sreg32(d) < x86::sreg32(base);
    };
    reg32 d;
    for (;;)
    {
        {
            const ReturnSlot slot(cpu);
            c = cgetcOf(app, cpu, specs);
        }
        if (cpu.terminate)
            return 0;
        if (!(byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace))
            break;
        ++skipped;
    }
    if (scanFlags(app, specs) & kEndOfInput)
        goto stored;
    {
        const reg32 width = at(app, specs + kScanWidth);
        put(app, specs + kScanWidth, width - 1);
        if (width == 0)
            goto putBack;
    }
    if (sign != 0 && (c == '+' || c == '-'))
    {
        minus = c;
        ++skipped;
        const bool more = next();
        if (cpu.terminate)
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
            if (cpu.terminate)
                return 0;
            if (!more)
                goto stored;
            if (c == 'x' || c == 'X')
            {
                skipped += 2;
                read = 0;
                const bool after = next();
                if (cpu.terminate)
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
        if (cpu.terminate)
            return 0;
        if (!more)
            goto stored;
    }
    if (c == ':' && (scanFlags(app, specs) & kSegmented))
        for (;;)
        {
            ++read;
            const bool more = next();
            if (cpu.terminate)
                return 0;
            if (!more)
                goto stored;
            if (!digitOf(c, d))
                break;
            take(d);
        }
putBack:
    {
        const ReturnSlot slot(cpu);
        uncgetOf(app, cpu, c, specs);
    }
    if (cpu.terminate)
        return 0;
stored:
    if (longLong)
    {
        if (minus == '-')
            value = 0 - value;
        if (x86::sreg32(read) > 0)
        {
            read += skipped;
            const reg8 flags = scanFlags(app, specs);
            if (flags & kAssign)
            {
                // The far one through its own segment (es as its pointer has it), else through ds.
                const reg32 step = at(app, argp) + ((flags & kFarPointer) ? 8 : 4);
                put(app, argp, step);
                // A far pointer's selector taken as the base, as the code's es does; ds the flat model's 0.
                const reg32 to = (flags & kFarPointer) ? at(app, step - 8) + half(app, step - 4) : at(app, step - 4);
                put(app, to, reg32(value));
                put(app, to + 4, reg32(value >> 32));
            }
        }
    }
    else
    {
        if (minus == '-')
            small = 0 - small;
        if (x86::sreg32(read) > 0)
        {
            read += skipped;
            const reg8 flags = scanFlags(app, specs);
            if (flags & kAssign)
            {
                reg32 step, target = argp;
                scanArgument(app, argp, (flags & kFarPointer) != 0, step, target);
                const reg32 to = target;  // es and ds: the flat model's 0
                if (scanFlags(app, specs) & kShortVar)
                    putHalf(app, to, reg16(small));
                else
                    put(app, to, small);
            }
        }
    }
    return read;
}

x86::reg32 scan_int(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 read = scanIntOf(app, cpu, args.eax, args.edx, args.ebx, args.ecx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, read);
    return args.eax;
}

/* 0x50c650 (__scnf): the format at `format`, the arguments' va_list at [argp], the input by the
 * specs at `specs`: white space in the format skips the input's, a character of it must be the
 * input's next, and a '%' a conversion.  How many arguments were assigned, or -1 for an input over
 * before any.  At its entry's depth: four pushes, then the format's place and the va_list's copy,
 * which [ and the conversions are handed.  0 with cpu.terminate set when the game stopped. */
reg32 scnfOf(win32::WinApplication* app, x86::CPU& cpu, reg32 specs, reg32 format, reg32 argp)
{
    const GuestFrame frame(cpu, 0x10, 8);
    const reg32 local = frame - 8;
    const reg32 args = local + 4;
    put(app, local, format);
    put(app, args, at(app, argp));
    putByte(app, specs + kScanFlags, scanFlags(app, specs) & reg8(~kEndOfInput));
    reg32 assigned = 0, read = 0;
    auto ended = [&] { return (scanFlags(app, specs) & kEndOfInput) != 0; };
    // The input's next must be `want`: false for the end of the scan.
    auto literal = [&](reg32 want) {
        reg32 c;
        {
            const ReturnSlot slot(cpu);
            c = cgetcOf(app, cpu, specs);
        }
        if (cpu.terminate)
            return false;
        if (c == want)
        {
            ++read;
            return true;
        }
        if (!ended())
        {
            const ReturnSlot slot(cpu);
            uncgetOf(app, cpu, c, specs);
        }
        return false;
    };
    for (;;)
    {
        const reg32 p = at(app, local);
        put(app, local, p + 1);
        const reg32 f = byte(app, p);
        if (f == 0)
            goto end;
        if (byte(app, kIsTable + ((f + 1) & 0xff)) & kSpace)
        {
            const ReturnSlot slot(cpu);
            read += scanWhiteOf(app, cpu, specs);
        }
        else if (f != '%')
        {
            if (!literal(f))
                goto end;
        }
        else
        {
            const reg32 q = getOptOf(app, at(app, local), specs);
            put(app, local, q);
            const reg32 kind = byte(app, q);
            if (kind != 0)
                put(app, local, q + 1);
            reg32 got = 0;
            bool converted = true;
            {
                const ReturnSlot slot(cpu);
                switch (kind)
                {
                case 'd': got = scanIntOf(app, cpu, specs, args, 10, 1); break;
                case 'i': got = scanIntOf(app, cpu, specs, args, 0, 1); break;
                case 'o': got = scanIntOf(app, cpu, specs, args, 8, 1); break;
                case 'u': got = scanIntOf(app, cpu, specs, args, 10, 0); break;
                case 'x': case 'X': case 'p': got = scanIntOf(app, cpu, specs, args, 16, 1); break;
                case 'e': case 'f': case 'g': case 'E': case 'G': got = scanFloatOf(app, cpu, specs, args); break;
                case 'S':
                    putByte(app, specs + kScanFlags, scanFlags(app, specs) | kLongVar);
                    got = scanStringOf(app, cpu, specs, args, specs);
                    break;
                case 's': got = scanStringOf(app, cpu, specs, args, specs); break;
                case '[': got = scanArbOf(app, cpu, specs, args, local); break;
                case 'C':
                    putByte(app, specs + kScanFlags, scanFlags(app, specs) | kLongVar);
                    got = scanCharOf(app, cpu, specs, args, specs);
                    break;
                case 'c': got = scanCharOf(app, cpu, specs, args, specs); break;
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
            if (cpu.terminate)
                return 0;
            if (converted)
            {
                if (x86::sreg32(got) <= 0)
                    goto end;
                read += got;
                if (scanFlags(app, specs) & kAssign)
                    ++assigned;
            }
        }
        if (cpu.terminate)
            return 0;
        if (ended())
            break;
    }
    // The input over: a %n where the format goes on is still reported.
    if (byte(app, at(app, local)) == '%')
    {
        put(app, local, at(app, local) + 1);
        const reg32 q = getOptOf(app, at(app, local), specs);
        put(app, local, q);
        if (byte(app, q) == 'n')
            reportScanOf(app, specs, args, read);
    }
end:
    if (cpu.terminate)
        return 0;
    return assigned == 0 && ended() ? 0xffffffff : assigned;
}

x86::reg32 scnf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 assigned;
    {
        const EspKept kept(cpu);
        assigned = scnfOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, assigned);
    return args.eax;
}

/* 0x4f0520 (vsscanf): the string at `string` scanned by the format at `format`, the arguments'
 * va_list at [argp] -- __scnf with sscanf's routines in specs of its frame: two pushes, then 0x14
 * bytes. */
reg32 vsscanfOf(win32::WinApplication* app, x86::CPU& cpu, reg32 string, reg32 format, reg32 argp)
{
    const GuestFrame frame(cpu, 8, 0x14);
    const reg32 specs = frame - 0x14;
    put(app, specs + kScanString, string);
    put(app, specs + kScanGet, kCgetString);
    put(app, specs + kScanUnget, kUncgetString);
    const ReturnSlot slot(cpu);
    return scnfOf(app, cpu, specs, format, argp);
}

x86::reg32 vsscanf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 assigned;
    {
        const EspKept kept(cpu);
        assigned = vsscanfOf(app, cpu, args.eax, args.edx, args.ebx);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, assigned);
    return args.eax;
}

/* 0x4f0548 (sscanf): vsscanf's, its arguments on the stack from `arguments` on; the va_list in its
 * frame: two pushes, then the dword. */
reg32 sscanfOf(win32::WinApplication* app, x86::CPU& cpu, reg32 string, reg32 format, reg32 arguments)
{
    const GuestFrame frame(cpu, 8, 4);
    const reg32 list = frame - 4;
    put(app, list, arguments);
    const ReturnSlot slot(cpu);
    return vsscanfOf(app, cpu, string, format, list);
}

x86::reg32 sscanf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 assigned;
    {
        const EspKept kept(cpu);
        assigned = sscanfOf(app, cpu, at(app, cpu.esp + 4), at(app, cpu.esp + 8), cpu.esp + 0xc);
    }
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, assigned);
    return args.eax;
}

/* A long double of its parts: the mantissa's low and high dwords, the sign and exponent. */
x86::IEEEf80 longDouble(reg32 low, reg32 high, reg16 exponent)
{
    return x86::IEEEf80(x86::IEEEf80Data{reg16(low), reg16(low >> 16), reg16(high), reg16(high >> 16), exponent});
}

/* The long double at `p`, and one put there. */
x86::IEEEf80 longDoubleAt(win32::WinApplication* app, reg32 p)
{
    const x86::IEEEf80 value = app->getMemory<x86::IEEEf80>(p);
    return value;
}

void putLongDouble(win32::WinApplication* app, reg32 p, const x86::IEEEf80& value)
{
    app->getMemory<x86::IEEEf80>(p) = value;
}

reg32 lowOf(const x86::IEEEf80& value)
{
    return value.data.data1 | reg32(value.data.data2) << 16;
}

/* 0x51663a (the long double's normalizing): the 96 bits `high`:`middle`:`low`, exponent
 * `exponent`, shifted up until the top one is set, then rounded to 64 by the low dword's top bit.
 * The exponent; 0 for no bits at all. */
reg32 normalizeOf(reg32& high, reg32& middle, reg32& low, reg32& exponent)
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
    const reg32 round = low >> 31;
    low <<= 1;
    if (round)
    {
        const x86::reg64 m = x86::reg64(middle) + round;
        const x86::reg64 h = x86::reg64(high) + (m >> 32);
        middle = reg32(m);
        high = reg32(h);
        if (h >> 32)
        {
            high = high >> 1 | 0x80000000;
            ++exponent;
        }
    }
    return exponent;
}

x86::reg32 normalize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    // Its own convention: edx:eax:ebp and edi in, edx:eax and esi out -- the rest as it leaves them.
    args.esi = normalizeOf(args.edx, args.eax, args.ebp, args.edi);
    cpu.esp += 4;
    return args.eax;
}

/* 0x5165d8 (__ZBuf2LD): the decimal `digits` as a long double -- an integer, 96 bits of it,
 * normalized. */
x86::IEEEf80 zbufOf(const std::string& digits)
{
    reg32 high = 0, middle = 0, low = 0;
    for (const char c : digits)
    {
        // Times 10: times 4, plus itself, times 2.
        const reg32 h = high, m = middle, l = low;
        shift96(high, middle, low);
        shift96(high, middle, low);
        add96(high, middle, low, h, m, l);
        shift96(high, middle, low);
        add96(high, middle, low, 0, 0, reg32(c) & 0xf);
    }
    reg32 exponent = 0x405e;
    const reg32 normal = normalizeOf(high, middle, low, exponent);
    return longDouble(middle, high, reg16(normal));
}

/* The string at `p`, up to its 0. */
std::string guestString(win32::WinApplication* app, reg32 p)
{
    std::string s;
    for (reg8 c; (c = byte(app, p)) != 0; ++p)
        s += char(c);
    return s;
}

x86::reg32 ZBuf2LD(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::IEEEf80 value = zbufOf(guestString(app, args.eax));
    app->getMemory<x86::IEEEf80>(args.edx) = value;
    returns(cpu, args, lowOf(value));  // eax the mantissa's low dword, as the code leaves it
    return args.eax;
}

namespace
{

/* 0x5166cf: the scaling's way out, its frame's registers back -- the jump 0x5166d8 makes there. */
void scaleReturn(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    cpu.esp = args.ebp - 0x10;
    scanEpilogue(app, cpu, {&args.edi, &args.esi, &args.ecx, &args.ebx, &args.ebp});
}

}  // namespace

/* 0x51667b (the scaling's powers): `value` times 10 to the `n` -- 0x2000 at most -- by the table's
 * powers of 10^2^n, each product by the x87, under `x87`. */
void scalePowersOf(win32::WinApplication* app, const Arithmetic& x87, x86::IEEEf80& value, reg32 n)
{
    if (x86::sreg32(n) >= 0x2000)
        n = 0x2000;
    for (reg32 power = kPowersOfTen; x86::sreg32(n) > 0; power += 0xa)
    {
        if (n & 1)
        {
            const x86::Float a = x86::Float(value);
            const x86::Float b = x86::Float(longDoubleAt(app, power));
            const x86::Float product = x87.mul(a, b);
            value = x86::Float(product);
        }
        n = reg32(x86::sreg32(n) >> 1);
    }
}

x86::reg32 scalePowers(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::IEEEf80 value = longDoubleAt(app, args.eax);
    scalePowersOf(app, {cpu.fenv.control.word}, value, args.edx);
    putLongDouble(app, args.eax, value);
    returns(cpu, args, args.eax);  // eax the long double's address, as the code leaves it
    return args.eax;
}

/* 0x5166cf: 0x51667b's way out, which 0x5166d8 jumps to. */
x86::reg32 scaleEnd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    scaleReturn(app, cpu, args);
    return args.eax;
}

/* 0x5166d8 (the scaling by a power): `value` times 10 to the `n` -- over it for a negative one --
 * the x87 rounding to its 64 bits meanwhile (its control word `control` with the precision set).
 * The control word as it was, which the code leaves in eax; for an `n` of 0 nothing, which leaves
 * eax as it was. */
reg32 scaleByOf(win32::WinApplication* app, reg16 control, x86::IEEEf80& value, reg32 n)
{
    const Arithmetic x87{reg16(control | 0x300)};
    x86::IEEEf80 power = longDouble(0, 0x80000000, 0x3fff);
    const bool divide = x86::sreg32(n) < 0;
    scalePowersOf(app, x87, power, divide ? 0 - n : n);
    {
        const x86::Float a = x86::Float(value);
        const x86::Float b = x86::Float(power);
        const x86::Float result = divide ? x87.div(a, b) : x87.mul(a, b);
        value = x86::Float(result);
    }
    return control;
}

x86::reg32 scaleBy(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (args.edx != 0)
    {
        const reg32 p = args.eax;
        x86::IEEEf80 value = longDoubleAt(app, p);
        args.eax = scaleByOf(app, cpu.fenv.control.word, value, args.edx);
        putLongDouble(app, p, value);
    }
    cpu.esp += 4;
    return args.eax;
}

/* 0x516759 (__LDScale10x): `value` times 10 to the `n`, in steps of 10^4096 at most.  What the
 * last step leaves in eax: the x87's control word, none for a last step of 0. */
std::optional<reg32> ldScale10xOf(win32::WinApplication* app, reg16 control, x86::IEEEf80& value, reg32 n)
{
    if (x86::sreg32(n) > 0x1000)
    {
        scaleByOf(app, control, value, 0x1000);
        n -= 0x1000;
    }
    else if (x86::sreg32(n) < -0x1000)
    {
        scaleByOf(app, control, value, 0xfffff000);
        n += 0x1000;
    }
    if (n == 0)
        return std::nullopt;
    return scaleByOf(app, control, value, n);
}

x86::reg32 LDScale10x(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::IEEEf80 value = longDoubleAt(app, args.eax);
    const std::optional<reg32> control = ldScale10xOf(app, cpu.fenv.control.word, value, args.edx);
    putLongDouble(app, args.eax, value);
    returns(cpu, args, control.value_or(args.eax));
    return args.eax;
}

/* 0x4f8122 (__Strtold): the number at `s` -- white space, a sign, digits with a point among them,
 * an exponent -- as `value`, from its first 19 significant digits; the end of it at [end], unless
 * `end` is 0.  0 for no digits (the long double 0), 3 for a number too big for a double, 2 for one
 * too small, 1 for any other. */
reg32 strtoldOf(win32::WinApplication* app, x86::CPU& cpu, reg32 s, reg32 end, x86::IEEEf80& value)
{
    reg32 p = s, stop = s;
    auto isDigit = [](reg8 c) { return c >= '0' && c <= '9'; };
    reg8 c = byte(app, p);
    for (; c == ' ' || (c >= 9 && c <= 0xd); c = byte(app, ++p))
    {
    }
    const bool negative = c == '-';
    if (c == '+' || c == '-')
        ++p;
    std::string digits;   // the significant ones, 19 at most
    reg32 count = 0, fraction = 0;
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
            if (x86::sreg32(count) < 0x13)
                digits += char(c);
            ++count;
        }
        any = true;
    }
    reg32 exponent = 0;
    if (any)
    {
        if (c == 'e' || c == 'E')
        {
            const reg32 e = p - 1;
            const reg8 sign = byte(app, p);
            if (sign == '+' || sign == '-')
                ++p;
            bool exponentDigits = false;
            for (; isDigit(c = byte(app, p)); ++p)
            {
                if (x86::sreg32(exponent) < 0x3e8)
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
    if (x86::sreg32(count) > 0x13)
    {
        exponent += count - 0x13;
        count = 0x13;
    }
    while (x86::sreg32(count) > 0 && digits[count - 1] == '0')
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
    x86::IEEEf80 number = zbufOf(digits);
    if (exponent != 0)
        ldScale10xOf(app, 0, number, exponent);  // the control word it leaves in eax is not read
    if (negative)
        number.data.data5 |= 0x8000;
    value = number;
    const reg32 magnitude = exponent + count - 1;
    if (x86::sreg32(magnitude) > 0x134)
        return 3;
    if (x86::sreg32(magnitude) < -0x134)
        return 2;
    return 1;
}

x86::reg32 Strtold(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::IEEEf80 value;
    const reg32 result = strtoldOf(app, cpu, args.eax, args.ebx, value);
    putLongDouble(app, args.edx, value);
    returns(cpu, args, result);
    return args.eax;
}

/* __set_ERANGE's core: errno ERANGE; what it leaves in eax, 0 with cpu.terminate set when the game
 * stopped. */
reg32 setErangeOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const EspKept kept(cpu);
    reg32 data;
    return setThreadWord(app, cpu, kErrno, kERANGE, &data) ? data : 0;
}

/* 0x4f82b6 (strtod): the number at `s` as a double -- HUGE_VAL with its sign, and ERANGE, for one
 * too big; 0 or a denormal, and ERANGE, for one too small; its end at [end], unless `end` is 0.
 * `eax` what the code leaves there: the double's high dword, the long double's exponent, or
 * __set_ERANGE's.  0 with cpu.terminate set when the game stopped. */
double strtodOf(win32::WinApplication* app, x86::CPU& cpu, reg32 s, reg32 end, reg32& eax)
{
    x86::IEEEf80 number;
    const reg32 kind = strtoldOf(app, cpu, s, end, number);
    double d = 0;
    eax = 0;
    if (kind == 0)
        return d;
    auto highOf = [](double v) {
        x86::reg64 bits;
        std::memcpy(&bits, &v, sizeof bits);
        return reg32(bits >> 32);
    };
    const reg32 e = number.data.data5 & 0x7fff;
    bool range = false;
    if (e >= 0x43ff)
    {
        eax = setErangeOf(app, cpu);
        if (cpu.terminate)
            return 0;
        if (number.data.data5 & 0x8000)
        {
            const x86::Float huge = x86::Float(app->getMemory<double>(kHugeVal));
            const x86::Float minus = -huge;
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
        const x86::Float value = x86::Float(number);
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
        eax = setErangeOf(app, cpu);
        if (cpu.terminate)
            return 0;
    }
    return d;
}

x86::reg32 strtod(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    reg32 eax;
    const double d = strtodOf(app, cpu, args.eax, args.edx, eax);
    if (cpu.terminate)
        return args.eax;
    const x86::Float result = x86::Float(d);
    args.floatSet = true;
    args.floatResult = result;
    returns(cpu, args, eax);
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

constexpr reg32 kConsoleIn = 0x568a38, kConsoleOut = 0x568a3c;  // conin$'s and conout$'s HANDLEs, -1 for none yet
constexpr reg32 kConinName = 0x55056c, kConoutName = 0x550574;  // "conin$", "conout$"
constexpr reg32 kUngotKey = 0x567828;    // ungetch's character, 0 for none
constexpr reg32 kKeyState = 0x567168;    // do_getch's: 1 repeating a character, 2 a key's second half
constexpr reg32 kKeyScan = 0x9f38b0;     // the extended key's scan code, its second half
constexpr reg32 kKeyChar = 0x9f38b4, kKeyRepeats = 0x9f38b8;  // the character repeating, how many more times
constexpr reg32 kWindowsKbhit = 0x5677e0, kWindowsGetch = 0x5677e4, kWindowsPutch = 0x5677ec;
constexpr reg32 kPeekConsoleInputA = 0x5345a0, kReadConsoleInputA = 0x5345ac;
constexpr reg32 kGetConsoleMode = 0x5344fc, kSetConsoleMode = 0x5345d0, kWriteConsoleA = 0x53463c;

/* The console's HANDLE opened, if it is not yet, at `handle`: CreateFileA of
 * `name`, for `access`, shared as `share`. */
bool openConsole(win32::WinApplication* app, x86::CPU& cpu, reg32 handle, reg32 name, reg32 access, reg32 share)
{
    if (at(app, handle) != 0xffffffff)
        return true;
    const auto opened = windows<kCreateFileA, &host::createFile>(app, cpu, name, access, share, 0, 3, 0x80, 0);
    if (!opened)
        return false;
    put(app, handle, *opened);
    return true;
}

}  // namespace

/* 0x515690 (__NTRealKey): whether the INPUT_RECORD at `record` is a key pressed, not shift,
 * control or alt alone. */
bool realKeyOf(win32::WinApplication* app, reg32 record)
{
    if (half(app, record) != 1 || at(app, record + 4) == 0)
        return false;
    const reg16 key = half(app, record + 0xa);
    return key < 0x10 || key > 0x12;
}

x86::reg32 NTRealKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    returns(cpu, args, realKeyOf(app, args.eax) ? 1 : 0);
    return args.eax;
}

/* The file's lock taken (kAccessFile) or given back (kReleaseFile), by its hook (watcom_hooks.h),
 * with `eax`, every other register kept: what it leaves in eax, 0 with cpu.terminate set when the
 * game stopped. */
reg32 lockHook(win32::WinApplication* app, x86::CPU& cpu, reg32 slot, reg32 eax)
{
    const EspKept kept(cpu);
    return (slot == kAccessFile ? hook<kAccessFile>(app, cpu, {eax}) : hook<kReleaseFile>(app, cpu, {eax}))
        .value_or(0);
}

/* The default windowing's hook at [slot] for the windowed handle `handle`: __IsWindowedHandle's
 * say, then the hook with it in eax and `edx` -- putch's character; kbhit's and getch's take eax
 * alone, and are handed 0. */
reg32 windowedHook(win32::WinApplication* app, x86::CPU& cpu, reg32 slot, reg32 handle, reg32 edx)
{
    const EspKept kept(cpu);
    const auto window = through(app, cpu, at(app, kIsWindowedHandle), {handle, edx});
    if (!window)
        return 0;
    return through(app, cpu, at(app, slot), {*window, edx}).value_or(0);
}

/* One INPUT_RECORD from the console `handle` into the 0x14 bytes at `record`, how many at its
 * 0x14th: Windows' `Function`, its say. */
template <reg32 Slot, auto Function>
reg32 consoleRecord(win32::WinApplication* app, x86::CPU& cpu, reg32 handle, reg32 record)
{
    const EspKept kept(cpu);
    return windowsAt<Slot, Function>(app, cpu, record, handle, record, 1, record + 0x14).value_or(0);
}

/* 0x5156b8 (the console's handles): conin$ and conout$ opened, the ones not yet, under the file's
 * lock.  What the release leaves in eax. */
reg32 openConsolesOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 8, 0);
    lockHook(app, cpu, kAccessFile, 0);
    if (cpu.terminate)
        return 0;
    {
        const EspKept kept(cpu);
        if (!openConsole(app, cpu, kConsoleIn, kConinName, 0x80000000, 1)
            || !openConsole(app, cpu, kConsoleOut, kConoutName, 0x40000000, 2))
            return 0;
    }
    return lockHook(app, cpu, kReleaseFile, 0);
}

x86::reg32 openConsoles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = openConsolesOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x515728 (__NTConsoleInput): the console's input HANDLE. */
reg32 consoleInputOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const ReturnSlot slot(cpu);
    openConsolesOf(app, cpu);
    return at(app, kConsoleIn);
}

x86::reg32 NTConsoleInput(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 handle = consoleInputOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, handle);
    return args.eax;
}

/* 0x515734 (__NTConsoleOutput): the console's output HANDLE. */
reg32 consoleOutputOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const ReturnSlot slot(cpu);
    openConsolesOf(app, cpu);
    return at(app, kConsoleOut);
}

x86::reg32 NTConsoleOutput(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 handle = consoleOutputOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, handle);
    return args.eax;
}

/* 0x52b290 (putch): the character `c` to the console -- by the default windowing's putch, where
 * there is one, else WriteConsoleA under the file's lock; `c` back.  At its entry's depth: three
 * pushes, then 8 bytes: how many WriteConsoleA wrote, the character.  0 with cpu.terminate set when
 * the game stopped. */
reg32 putchOf(win32::WinApplication* app, x86::CPU& cpu, reg32 c)
{
    const GuestFrame frame(cpu, 0xc, 8);
    const reg32 written = frame - 8, character = frame - 4;
    putByte(app, character, reg8(c));
    if (at(app, kWindowsPutch) != 0)
        windowedHook(app, cpu, kWindowsPutch, 1, c);
    else
    {
        lockHook(app, cpu, kAccessFile, 1);
        if (cpu.terminate)
            return 0;
        const reg32 handle = consoleOutputOf(app, cpu);
        if (cpu.terminate)
            return 0;
        {
            const EspKept kept(cpu);
            if (!windowsAt<kWriteConsoleA, &host::writeConsole>(app, cpu, handle, handle, character, 1, written, 0))
                return 0;
        }
        lockHook(app, cpu, kReleaseFile, 1);
    }
    return cpu.terminate ? 0 : c;
}

x86::reg32 putch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 c = putchOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, c);
    return args.eax;
}

/* 0x4f7ce0 (kbhit): whether a key is waiting -- the console's input looked at, what is not a key
 * taken off it -- or the default windowing's say.  At its entry's depth: three pushes, then 0x18
 * bytes, the INPUT_RECORD and how many.  0 with cpu.terminate set when the game stopped. */
reg32 kbhitOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 0xc, 0x18);
    const reg32 record = frame - 0x18;
    if (at(app, kWindowsKbhit) != 0)
        return windowedHook(app, cpu, kWindowsKbhit, 0, 0);
    lockHook(app, cpu, kAccessFile, 0);
    if (cpu.terminate)
        return 0;
    const reg32 handle = consoleInputOf(app, cpu);
    if (cpu.terminate)
        return 0;
    for (;;)
    {
        // Where the game's code lets the other threads run, as kbhit waits on the console
        // (THREAD_SEGMENTS), and its loop's safepoint.
        app->unlockContext(cpu);
        win32::Thread::sleep(0);
        app->lockContext(cpu);
        if (app->contextWanted())
            app->yieldContext(cpu);
        consoleRecord<kPeekConsoleInputA, &host::peekConsoleInput>(app, cpu, handle, record);
        if (cpu.terminate)
            return 0;
        if (at(app, record + 0x14) == 0 || realKeyOf(app, record))
            break;
        consoleRecord<kReadConsoleInputA, &host::readConsoleInput>(app, cpu, handle, record);
        if (cpu.terminate)
            return 0;
    }
    lockHook(app, cpu, kReleaseFile, 0);
    if (cpu.terminate)
        return 0;
    return at(app, record + 0x14) != 0 ? 1 : 0;
}

x86::reg32 kbhit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 waiting = kbhitOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, waiting);
    return args.eax;
}

/* 0x4f7e20 (do_getch): the next character from the console `handle`, as getch gives it: a
 * character repeated as many times as Windows says, an extended key as 0 and its scan code; -1 for
 * none.  At its entry's depth: six pushes, then 0x18 bytes, the INPUT_RECORD and how many.  0 with
 * cpu.terminate set when the game stopped. */
reg32 doGetchOf(win32::WinApplication* app, x86::CPU& cpu, reg32 handle)
{
    const GuestFrame frame(cpu, 0x18, 0x18);
    const reg32 record = frame - 0x18;
    reg32 repeats = at(app, kKeyRepeats), c = at(app, kKeyChar), key;
    const reg32 state = at(app, kKeyState);
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
        for (;;)
        {
            const reg32 read = consoleRecord<kReadConsoleInputA, &host::readConsoleInput>(app, cpu, handle, record);
            if (cpu.terminate)
                return 0;
            if (read == 0)
            {
                key = 0xffffffff;
                c = at(app, kKeyChar);
                repeats = at(app, kKeyRepeats);
                break;
            }
            if (!realKeyOf(app, record))
                continue;
            repeats = half(app, record + 8) - 1;
            c = byte(app, record + 0xe);
            if ((byte(app, record + 0x11) & 1) || c == 0)
            {
                // An extended key: 0 now, its scan code next.
                c = 0;
                put(app, kKeyState, 2);
                put(app, kKeyScan, half(app, record + 0xc));
            }
            else if (repeats != 0)
                put(app, kKeyState, 1);
            key = c;
            break;
        }
    }
    put(app, kKeyChar, c);
    put(app, kKeyRepeats, repeats);
    return key;
}

x86::reg32 do_getch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 key = doGetchOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, key);
    return args.eax;
}

/* 0x4f7f24 (getch): the next character, not echoed: ungetch's, the default windowing's, or the
 * console's with its mode off for it.  At its entry's depth: four pushes, then the console's mode.
 * 0 with cpu.terminate set when the game stopped. */
reg32 getchOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 mode = frame - 4;
    const reg32 ungot = at(app, kUngotKey);
    if (ungot != 0)
    {
        put(app, kUngotKey, 0);
        return ungot;
    }
    if (at(app, kWindowsGetch) != 0)
        return windowedHook(app, cpu, kWindowsGetch, 0, 0);
    lockHook(app, cpu, kAccessFile, 0);
    if (cpu.terminate)
        return 0;
    const reg32 handle = consoleInputOf(app, cpu);
    if (cpu.terminate)
        return 0;
    reg32 key;
    {
        const EspKept kept(cpu);
        if (!windowsAt<kGetConsoleMode, &host::consoleMode>(app, cpu, mode, handle, mode)
            || !windows<kSetConsoleMode, &host::setConsoleMode>(app, cpu, handle, 0))
            return 0;
    }
    {
        const ReturnSlot slot(cpu);
        key = doGetchOf(app, cpu, handle);
    }
    if (cpu.terminate)
        return 0;
    {
        const EspKept kept(cpu);
        if (!windowsAt<kSetConsoleMode, &host::setConsoleMode>(app, cpu, key, handle, at(app, mode)))
            return 0;
    }
    lockHook(app, cpu, kReleaseFile, 0);
    return cpu.terminate ? 0 : key;
}

x86::reg32 getch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 key = getchOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, key);
    return args.eax;
}

/* 0x522e60 (getche): getch's character, echoed by putch -- unless it was ungetch's.  At its
 * entry's depth: two pushes.  0 with cpu.terminate set when the game stopped. */
reg32 getcheOf(win32::WinApplication* app, x86::CPU& cpu)
{
    const GuestFrame frame(cpu, 8, 0);
    const reg32 ungot = at(app, kUngotKey);
    reg32 key;
    {
        const ReturnSlot slot(cpu);
        key = getchOf(app, cpu);
    }
    if (cpu.terminate)
        return 0;
    if (key != 0xffffffff && ungot == 0)
    {
        const ReturnSlot slot(cpu);
        putchOf(app, cpu, key);
    }
    return cpu.terminate ? 0 : key;
}

x86::reg32 getche(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 key = getcheOf(app, cpu);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, key);
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
constexpr reg32 kTryHandler = 0x4f7b2a;
constexpr reg32 kRtlUnwind = 0x5345c0;
constexpr reg32 kUnwindReturn = 0x4f7bbc;  // where RtlUnwind is told to come back to

}  // namespace

/* 0x4f7af8 (__TryInit): the registration at `registration` put first on fs:[0]'s chain, at no level
 * yet, for the function whose frame is `frame` (its ebp).  The old first goes by the stack, pushed
 * and popped. */
void tryInitOf(win32::WinApplication* app, x86::CPU& cpu, reg32 registration, reg32 frame)
{
    const reg32 previous = at(app, cpu.threadBlock);
    put(app, cpu.esp - 4, previous);
    put(app, registration, previous);
    put(app, registration + 4, kTryHandler);
    put(app, registration + 8, frame);
    putByte(app, registration + 0x10, 0xff);
    putByte(app, registration + 0x11, 0);
    put(app, cpu.threadBlock, registration);
}

x86::reg32 TryInit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    tryInitOf(app, cpu, args.eax, args.ebp);
    cpu.esp += 4;  // eax left as it was, the registration
    return args.eax;
}

/* 0x4f7b8c (__TryHandler's end): ExceptionContinueSearch, 1, the handler's
 * registers back. */
x86::reg32 TryDone(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = 1;
    for (reg32* r : {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp})
        *r = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* A call through a pointer: the callee's thunk run on `args` with esp as it is -- the callee finds its
 * return address's slot where the code's pushes left esp, and leaves esp as its ret does.  Its eax. */
x86::reg32 plainCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target, x86::Args& args)
{
    const win32::ThunkPtr thunk = app->thunkAt(target);
    if (thunk == nullptr)
    {
        cpu.terminate = true;  // no thunk registered at the target: the game cannot go on
        return args.eax;
    }
    return thunk(app, cpu, args);
}

x86::reg32 TryOuter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

/* 0x4f7b61 (__TryHandler's scopes): from the level in ebx out, the first
 * scope with a filter jumped to -- one without, a finally block, passed by. */
x86::reg32 TryScope(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (reg8(args.ebx) == 0xff)
        return TryDone(app, cpu, args);
    putByte(app, args.ebp + 0x12, reg8(args.ebx));
    args.ebx *= 3;
    if (byte(app, args.edi + args.ebx * 2 + 1) != 0)
        return TryOuter(app, cpu, args);
    push(app, cpu, args.ebp);
    push(app, cpu, args.edi);
    push(app, cpu, args.ebx);
    args.ebp = at(app, args.ebp + 8);
    args.eax = plainCall(app, cpu, at(app, args.edi + args.ebx * 2 + 2), args);
    return args.eax;
}

/* 0x4f7b87 (__TryHandler's next scope): the level the scope at ebx is in. */
x86::reg32 TryOuter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    setLow8(args.ebx, byte(app, args.edi + args.ebx * 2));
    TryScope(app, cpu, args);
    return args.eax;
}

/* 0x4f7bc3 (__local_unwind): the finally blocks from the registration's level, at `registration`,
 * out to the level unwound to, each called with ebp its function's frame -- and edi the scopes'
 * table and ebx the scope's index, which it pushes under its return.  The level is a byte of ebx
 * as the code has it: the index made of it times 3, the next level its low byte.  At its entry's
 * depth: six pushes.  False when the game stopped. */
bool localUnwindOf(win32::WinApplication* app, x86::CPU& cpu, reg32 registration, const x86::Args& caller)
{
    // The six pushes written, as structured exception handling is still register by register here:
    // __Except, which calls these, takes its words off first, so their frames lie where the caller's
    // were.
    const GuestFrame frame(app, cpu, {caller.ebp, caller.edi, caller.esi, caller.edx, caller.ecx, caller.ebx}, 0);
    x86::Args regs = caller;  // the registers the finally blocks are called with: eax as each leaves it
    const reg32 table = at(app, registration + 0xc);
    reg32 level = byte(app, registration + 0x10);
    putByte(app, registration + 0x11, 1);
    while (reg8(level) != 0xff && reg8(level) != byte(app, registration + 0x12))
    {
        level *= 3;
        if (byte(app, table + level * 2 + 1) != 0)
        {
            const GuestFrame saved(app, cpu, {registration, table, level}, 0);
            const EspKept kept(cpu);
            x86::Args a = regs;
            a.ebp = at(app, registration + 8);
            a.edi = table;
            a.ebx = level;
            const std::optional<reg32> eax = guest::pointerCall(app, cpu, at(app, table + level * 2 + 2), a);
            if (!eax)
                return false;
            regs.eax = *eax;
        }
        level = (level & 0xffffff00) | byte(app, table + level * 2);
        putByte(app, registration + 0x10, reg8(level));
    }
    putByte(app, registration + 0x11, 0);
    return true;
}

x86::reg32 local_unwind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (!localUnwindOf(app, cpu, args.ebp, args))
        return args.eax;
    cpu.esp += 4;  // eax as the finally blocks leave it
    return args.eax;
}

/* 0x533de4: RtlUnwind's import, jumped to: the port's host code (host_system.h) with the words on
 * the stack as the thunk finds them, stdcall. */
x86::reg32 RtlUnwindThunk(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    return win32::Wrapper<decltype(&host::unwind), &host::unwind>::stdcall(app, cpu, args);
}

/* 0x4f7ba7 (__global_unwind): the registrations above the one at `registration` unwound, by
 * RtlUnwind -- its four words pushed, stdcall, to come back to kUnwindReturn.  At its entry's
 * depth: six pushes.  False when the game stopped. */
bool globalUnwindOf(win32::WinApplication* app, x86::CPU& cpu, reg32 registration, const x86::Args& caller)
{
    // The six pushes written, as localUnwindOf's.
    const GuestFrame frame(app, cpu, {caller.ebp, caller.edi, caller.esi, caller.edx, caller.ecx, caller.ebx}, 0);
    const EspKept kept(cpu);
    for (const reg32 word : {reg32(0), reg32(0), kUnwindReturn, registration})
        push(app, cpu, word);
    x86::Args registers = caller;
    return direct(app, cpu, RtlUnwindThunk, registers);
}

x86::reg32 global_unwind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    if (!globalUnwindOf(app, cpu, args.ebp, args))
        return args.eax;
    cpu.esp += 4;  // eax as RtlUnwind leaves it
    return args.eax;
}

/* 0x4f7b2a (__TryHandler): the exception at [esp+4], for the registration at
 * [esp+8]: when unwinding, its finally blocks; else, its record and context
 * kept, its scopes' filters from its level out. */
x86::reg32 TryHandler(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebp);
    args.ebp = cpu.esp;
    for (const reg32 r : {args.edi, args.esi, args.edx, args.ecx, args.ebx})
        push(app, cpu, r);
    args.edi = at(app, args.ebp + 0xc);
    args.eax = at(app, args.ebp + 8);
    if (at(app, args.eax + 4) & 6)
    {
        args.ebp = args.edi;
        putByte(app, args.ebp + 0x12, 0xff);
        if (!direct(app, cpu, local_unwind, args))
            return args.eax;
        return TryDone(app, cpu, args);
    }
    put(app, args.edi + 0x14, args.eax);
    args.eax = at(app, args.ebp + 0x10);
    put(app, args.edi + 0x18, args.eax);
    args.ebp = args.edi;
    args.edi = at(app, args.ebp + 0xc);
    args.ebx = byte(app, args.ebp + 0x10);
    TryScope(app, cpu, args);
    return args.eax;
}

/* 0x4f7b7d (__Except): a filter's say, in eax, on the scope its pushes
 * name: -1 the exception's code goes on (0), 0 the next scope out, else
 * the scopes unwound and the handler run -- here, back to the filter's
 * caller. */
x86::reg32 Except(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.edx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    args.edi = pop(app, cpu);
    args.ebp = pop(app, cpu);
    ++args.eax;
    if (args.eax == 0)
    {
        for (reg32* r : {&args.ebx, &args.ecx, &args.edx, &args.esi, &args.edi, &args.ebp})
            *r = pop(app, cpu);
        cpu.esp += 4;
        return args.eax;
    }
    --args.eax;
    if (args.eax == 0)
        return TryOuter(app, cpu, args);
    push(app, cpu, args.edx);
    if (!direct(app, cpu, global_unwind, args) || !direct(app, cpu, local_unwind, args))
        return args.eax;
    args.ebp = at(app, args.ebp + 8);
    cpu.esp += 4;
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

constexpr reg32 kInitStart = 0x56b7de, kInitEnd = 0x56b81a;  // the runtime's initializers: 6 bytes each
constexpr reg32 kThreadDataSize = 0x56af70;  // __ThreadDataSize
constexpr reg32 kStackSize = 0x567820;       // the main thread's stack's size
constexpr reg32 kStackLimit = 0x567824;      // _STACKLOW's: where its stack ends
constexpr reg32 kStackTop = 0x56781c;        // _STACKTOP
constexpr reg32 kIsDll = 0x9f6420;           // whether the runtime is a DLL's (the way out's kExitStarted)
constexpr reg32 kMainThreadData = 0x9f642c;
constexpr reg32 kEnvironmentStrings = 0x567839;
constexpr reg32 kOsMajor = 0x56783f, kOsMinor = 0x567840, kOsBuild = 0x567841;  // GetVersion's parts
constexpr reg32 kOsVersion = 0x567843, kWinMajor = 0x567847, kWinMinor = 0x56784b, kWinVersion = 0x56784f;
constexpr reg32 kModuleName = 0x9f6430, kWideModuleName = 0x9f6534;  // the program's, 0x104 bytes and wide
constexpr reg32 kDllName = 0x9f673c, kWideDllName = 0x9f6840;
constexpr reg32 kProgramName = 0x567800, kWideProgramName = 0x56780c;
constexpr reg32 kDllNamePtr = 0x567804, kWideDllNamePtr = 0x567810;
constexpr reg32 kCommandLine = 0x9f6424, kWideCommandLine = 0x9f6428;  // their copies
constexpr reg32 kArguments = 0x5677fc, kWideArguments = 0x567808;      // past the program's name in them
constexpr reg32 kNoWideCommandLine = 0x54e0c0;  // L"" for a Windows without GetCommandLineW
constexpr reg32 kAccessIobHook = 0x5677a4;      // a hook the start calls between the initializers
constexpr reg32 kExceptionHandlerCode = 0x51ef08, kUnhandledFilter = 0x51ed24;
constexpr reg32 kArgc = 0xa18210, kArgv = 0xa18214;
constexpr reg32 kGameMain = 0x4a39f0;  // the game's main, still its code
constexpr reg32 kExitEntry = 0x4dfd04;
constexpr reg32 kGetModuleHandleA = 0x53454c, kGetModuleFileNameA = 0x534544, kGetModuleFileNameW = 0x534548;
constexpr reg32 kGetEnvironmentStrings = 0x53451c, kGetVersion = 0x534574, kGetCommandLineA = 0x5344f4;
constexpr reg32 kGetCommandLineW = 0x5344f8, kGetStdHandle = 0x53455c, kVirtualQuery = 0x53462c;
constexpr reg32 kSetUnhandledExceptionFilter = 0x534600, kGetCurrentThreadId = 0x53450c;

}  // namespace

/* 0x52b640: a start-up hook that does nothing. */
x86::reg32 startHook(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    NFS2_USE(app);
    cpu.esp += 4;
    return args.eax;
}

/* _nmalloc's and _nfree's cores as a call of them runs them, every register kept. */
reg32 nmallocOf(win32::WinApplication* app, x86::CPU& cpu, reg32 size)
{
    const ReturnSlot slot(cpu);
    const EspKept kept(cpu);
    reg32 block;
    return watcomheap::allocate(app, cpu, kGameHeap, size, block) ? block : 0;
}

void nfreeOf(win32::WinApplication* app, x86::CPU& cpu, reg32 block)
{
    const ReturnSlot slot(cpu);
    const EspKept kept(cpu);
    watcomheap::free(app, cpu, kGameHeap, block);
}

/* Whether _IsTable has the character `c` white space. */
bool isSpace(win32::WinApplication* app, reg32 c)
{
    return (byte(app, kIsTable + ((c + 1) & 0xff)) & kSpace) != 0;
}

/* 0x4ff41c (__InitRtns): the runtime's initializers of priority up to `limit` not yet run, the
 * lowest first, each marked run.  At its entry's depth: ebx, ecx, edx, esi and es pushed.  What the
 * code leaves in eax: the table's end. */
reg32 initRtnsOf(win32::WinApplication* app, x86::CPU& cpu, reg8 limit)
{
    const GuestFrame frame(cpu, 0x14, 0);  // the pushes: their room only
    for (;;)
    {
        reg32 first = kInitEnd;
        reg8 lowest = limit;
        for (reg32 entry = kInitStart; entry < kInitEnd; entry += 6)
            if (byte(app, entry) != 2 && lowest >= byte(app, entry + 1))
            {
                first = entry;
                lowest = byte(app, entry + 1);
            }
        if (first == kInitEnd)
            return kInitEnd;
        {
            const ReturnSlot slot(cpu);
            if (!callRoutineOf(app, cpu, first + 2))
                return 0;
        }
        putByte(app, first, 2);
    }
}

x86::reg32 InitRtns(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = initRtnsOf(app, cpu, reg8(args.eax));
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x51ec20 (the stack's limits): where the thread's stack ends, into [low], and where it starts,
 * into [high] -- from VirtualQuery of the frame, the guard pages Windows keeps below as its version
 * says -- either for none.  At its entry's depth: three pushes, then 0x20 bytes, the region asked
 * about at the top and its MEMORY_BASIC_INFORMATION at the bottom.  The end, which the code leaves
 * in eax; 0 with cpu.terminate set when the game stopped. */
reg32 stackLimitsOf(win32::WinApplication* app, x86::CPU& cpu, reg32 low, reg32 high)
{
    const GuestFrame frame(cpu, 0xc, 0x20);
    const reg32 info = frame - 0x20, region = frame - 4;
    {
        const EspKept kept(cpu);
        if (!windowsAt<kVirtualQuery, &host::virtualQuery>(app, cpu, region, region, info, 0x1c))
            return 0;
    }
    const reg32 start = at(app, info) + at(app, info + 0xc);
    reg32 end = at(app, info + 4);
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

x86::reg32 stackLimits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 end = stackLimitsOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, end);
    return args.eax;
}

/* 0x525df0 (calloc): `n` things of `size` bytes, zeroed.  At its entry's depth: ebx pushed. */
reg32 callocOf(win32::WinApplication* app, x86::CPU& cpu, reg32 n, reg32 size)
{
    const GuestFrame frame(cpu, 0x4, 0);
    const reg32 bytes = n * size;
    const reg32 block = nmallocOf(app, cpu, bytes);
    if (cpu.terminate)
        return 0;
    if (block != 0)
        clib::memset(app, block, 0, bytes);
    return block;
}

x86::reg32 calloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 block = callocOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, block);
    return args.eax;
}

/* 0x526030 (the thread's data's start): the data at `data` -- if any -- told its stack's end and
 * its thread.  At its entry's depth: three pushes.  What the code leaves in eax: the thread's id,
 * or 0; 0 with cpu.terminate set when the game stopped. */
reg32 initThreadOf(win32::WinApplication* app, x86::CPU& cpu, reg32 data)
{
    const GuestFrame frame(cpu, 0xc, 0);
    if (data == 0)
        return 0;
    put(app, data + 0xc, 1);
    reg32 end;
    {
        const ReturnSlot slot(cpu);
        end = stackLimitsOf(app, cpu, data, 0);
    }
    if (cpu.terminate)
        return 0;
    const EspKept kept(cpu);
    const auto thread = windowsAt<kGetCurrentThreadId, &host::currentThreadId>(app, cpu, end);
    if (!thread)
        return 0;
    put(app, data + 0xda, *thread);
    return *thread;
}

x86::reg32 initThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = initThreadOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x51e7ec (the main thread's data): the data at `data` -- calloc's, for none -- started; the data,
 * or 0.  At its entry's depth: ebx and edx pushed. */
reg32 initThreadDataOf(win32::WinApplication* app, x86::CPU& cpu, reg32 data)
{
    const GuestFrame frame(cpu, 0x8, 0);
    if (data == 0)
    {
        {
            const ReturnSlot slot(cpu);
            data = callocOf(app, cpu, 1, at(app, kThreadDataSize));
        }
        if (cpu.terminate)
            return 0;
        if (data != 0)
        {
            putByte(app, data + 0x52, 1);
            put(app, data + 0xf0, at(app, kThreadDataSize));
        }
    }
    {
        const ReturnSlot slot(cpu);
        initThreadOf(app, cpu, data);
    }
    return cpu.terminate ? 0 : data;
}

x86::reg32 initThreadData(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 data = initThreadDataOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, data);
    return args.eax;
}

/* 0x51e490 (__initPOSIXHandles): the standard input, output and error Windows hands -- a fake
 * HANDLE for one it has none of -- made handles 0, 1 and 2.  At its entry's depth: ecx and edx
 * pushed.  The last handle, which the code leaves in eax; 0 with cpu.terminate set when the game
 * stopped. */
reg32 initPOSIXHandlesOf(win32::WinApplication* app, x86::CPU& cpu, reg32 eax)
{
    const GuestFrame frame(cpu, 0x8, 0);
    reg32 handle = eax;  // eax as it came: the last handle, as the code has it at each call
    for (const reg32 which : {reg32(-10), reg32(-11), reg32(-12)})
    {
        reg32 os;
        {
            const EspKept kept(cpu);
            const auto standard = windowsAt<kGetStdHandle, &host::stdHandle>(app, cpu, handle, which);
            if (!standard)
                return 0;
            os = *standard;
        }
        const ReturnSlot slot(cpu);
        const EspKept kept(cpu);
        if ((os == 0 || os == 0xffffffff) && !fakeHandle(app, cpu, os))
            return 0;
        if (!allocHandle(app, cpu, os, handle))
            return 0;
    }
    return handle;
}

x86::reg32 initPOSIXHandles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 handle = initPOSIXHandlesOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, handle);
    return args.eax;
}

/* 0x51eb00 (the module's wide name): the name of the module `module` into the `size` wide
 * characters at `out` -- by GetModuleFileNameW where Windows has it, else the narrow name made wide;
 * how long, or 0.  At its entry's depth: four pushes.  0 with cpu.terminate set when the game
 * stopped. */
reg32 moduleWideNameOf(win32::WinApplication* app, x86::CPU& cpu, reg32 module, reg32 out, reg32 size)
{
    const GuestFrame frame(cpu, 0x10, 0);
    // A Windows function run with eax `eax`, as the code calls it, every other register kept: its
    // say.
    auto windowsSays = [&](reg32 eax, auto&& run) {
        const EspKept kept(cpu);
        (void)eax;  // what the code has in eax at the call: the functions run here all give one back
        return run().value_or(0);
    };
    const reg32 version =
        windowsSays(module, [&] { return windows<kGetVersion, &host::windowsVersion>(app, cpu); });
    if (cpu.terminate)
        return 0;
    if (reg16(version >> 16) < 0x8000)
        return windowsSays((version >> 16) & 0xffff, [&] {
            return windows<kGetModuleFileNameW, &host::moduleFileNameWide>(app, cpu, module, out, size);
        });
    const reg32 narrow = nmallocOf(app, cpu, 0x208);
    if (cpu.terminate || narrow == 0)
        return 0;
    const reg32 n = windowsSays(
        narrow, [&] { return windows<kGetModuleFileNameA, &host::moduleFileName>(app, cpu, module, narrow, 0x208); });
    if (cpu.terminate)
        return 0;
    if (n == 0)
    {
        nfreeOf(app, cpu, narrow);
        return 0;
    }
    const reg32 wide = windowsSays(n, [&] {
        return windows<kMultiByteToWideChar, &host::multiByteToWide>(app, cpu, 1, 1, narrow, reg32(-1), out, size);
    });
    if (cpu.terminate)
        return 0;
    nfreeOf(app, cpu, narrow);
    if (cpu.terminate || wide == 0)
        return 0;
    putHalf(app, out + size * 2 - 2, 0);
    return wcslenOf(app, out);
}

x86::reg32 moduleWideName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 n = moduleWideNameOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, n);
    return args.eax;
}

/* 0x51eba0 (the command line's copy): the string at `s` copied into a block of _nmalloc's; 0 for
 * none.  At its entry's depth: four pushes. */
reg32 copyCommandLineOf(win32::WinApplication* app, x86::CPU& cpu, reg32 s)
{
    const GuestFrame frame(cpu, 0x10, 0);
    reg32 n = 0;
    while (byte(app, s + n) != 0)
        ++n;
    ++n;
    const reg32 copy = nmallocOf(app, cpu, n);
    if (cpu.terminate)
        return 0;
    if (copy != 0)
        memcpyOf(app, copy, s, n);
    return copy;
}

x86::reg32 copyCommandLine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 copy = copyCommandLineOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, copy);
    return args.eax;
}

/* 0x51ebf0 (the wide command line's copy): the wide string at `s` copied into a block of
 * _nmalloc's; 0 for none.  At its entry's depth: three pushes. */
reg32 copyWideCommandLineOf(win32::WinApplication* app, x86::CPU& cpu, reg32 s)
{
    const GuestFrame frame(cpu, 0xc, 0);
    const reg32 bytes = (wcslenOf(app, s) + 1) * 2;
    const reg32 copy = nmallocOf(app, cpu, bytes);
    if (cpu.terminate)
        return 0;
    if (copy != 0)
        memcpyOf(app, copy, s, bytes);
    return copy;
}

x86::reg32 copyWideCommandLine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 copy = copyWideCommandLineOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, copy);
    return args.eax;
}

/* The thread's data, by __GetThreadPtr's pointer, every register kept (what the code has in edx at
 * the call, which __GetThreadPtr does not read, is not handed); 0 with cpu.terminate set when the
 * game stopped. */
reg32 threadPtrOf(win32::WinApplication* app, x86::CPU& cpu, reg32)
{
    const EspKept kept(cpu);
    return hook<kThreadPtr>(app, cpu).value_or(0);
}

/* 0x51f0d0 (__InitExceptionHandling): the registration at `registration` the thread's, first on
 * fs:[0]'s chain with the runtime's handler, and the runtime's filter Windows' for an exception
 * nobody handles.  At its entry's depth: ecx and edx pushed.  What SetUnhandledExceptionFilter
 * leaves in eax; 0 with cpu.terminate set when the game stopped. */
reg32 initExceptionHandlingOf(win32::WinApplication* app, x86::CPU& cpu, reg32 registration)
{
    const GuestFrame frame(cpu, 0x8, 0);
    reg32 data = threadPtrOf(app, cpu, registration);
    if (cpu.terminate)
        return 0;
    put(app, data + 0x54, registration);
    const reg32 previous = at(app, cpu.threadBlock);
    data = threadPtrOf(app, cpu, previous);
    if (cpu.terminate)
        return 0;
    put(app, at(app, data + 0x54), previous);
    data = threadPtrOf(app, cpu, previous);
    if (cpu.terminate)
        return 0;
    put(app, at(app, data + 0x54) + 4, kExceptionHandlerCode);
    data = threadPtrOf(app, cpu, 0);
    if (cpu.terminate)
        return 0;
    put(app, cpu.threadBlock, at(app, data + 0x54));
    const EspKept kept(cpu);
    return windowsAt<kSetUnhandledExceptionFilter, &host::setUnhandledExceptionFilter>(app, cpu, at(app, data + 0x54),
                                                                                       kUnhandledFilter)
        .value_or(0);
}

x86::reg32 InitExceptionHandling(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = initExceptionHandlingOf(app, cpu, args.eax);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* Past the program's name in the command line at `p` -- quoted or not -- and the white space after
 * it: `step` bytes a character, `c` its character. */
template <typename Char>
reg32 argumentsOf(win32::WinApplication* app, reg32 p, reg32 step, Char c)
{
    if (c(p) == '"')
    {
        const reg32 second = c(p + step);
        p += step;
        if (second != '"')
            while (c(p) != 0)
            {
                const reg32 next = c(p + step);
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
 * `data`, the module `module`: the data started, the standard handles, the environment, Windows'
 * version, the names, the command lines and where their arguments start.  0 for no thread's data
 * in a DLL, else 1; a program without one leaves by ExitProcess.  At its entry's depth: three
 * pushes.  0 with cpu.terminate set when the game stopped. */
reg32 ntInitOf(win32::WinApplication* app, x86::CPU& cpu, reg32 dll, reg32 data, reg32 module)
{
    const GuestFrame frame(cpu, 0xc, 0);
    // A Windows function run with eax `eax`, as the code calls it, every other register kept: its
    // say.
    auto says = [&](reg32 eax, auto&& run) {
        const EspKept kept(cpu);
        (void)eax;  // what the code has in eax at the call: the functions run here all give one back
        return run().value_or(0);
    };
    // A core of the runtime's at this one's depth.
    auto calls = [&](auto&& run) {
        const ReturnSlot slot(cpu);
        return run();
    };
    put(app, kIsDll, dll);
    data = calls([&] { return initThreadDataOf(app, cpu, data); });
    if (cpu.terminate)
        return 0;
    put(app, kMainThreadData, data);
    if (data == 0)
    {
        if (dll != 0)
            return 0;
        says(0, [&] { return windows<kExitProcess, &host::exitProcess>(app, cpu, 1); });
        if (cpu.terminate)
            return 0;
    }
    const reg32 handle = calls([&] { return initPOSIXHandlesOf(app, cpu, data); });  // eax the data, as stored
    if (cpu.terminate)
        return 0;
    const reg32 environment = says(handle, [&] { return windows<kGetEnvironmentStrings, &host::environmentStrings>(app, cpu); });
    if (cpu.terminate)
        return 0;
    put(app, kEnvironmentStrings, environment);
    put(app, kEnviron, 0);
    const reg32 version = says(environment, [&] { return windows<kGetVersion, &host::windowsVersion>(app, cpu); });
    if (cpu.terminate)
        return 0;
    // Its parts: the major and minor versions, the build -- and Windows' version in Watcom's own
    // words of it.
    const reg32 major = version & 0xff, minor = (version >> 8) & 0xff;
    putByte(app, kOsMajor, reg8(major));
    putHalf(app, kOsBuild, reg16(version >> 16));
    put(app, kOsVersion, half(app, kOsBuild));
    put(app, kWinMajor, major);
    putByte(app, kOsMinor, reg8(minor));
    put(app, kWinMinor, minor);
    put(app, kWinVersion, major << 8 | minor);
    says(minor, [&] { return windows<kGetModuleFileNameA, &host::moduleFileName>(app, cpu, 0, kModuleName, 0x104); });
    if (cpu.terminate)
        return 0;
    put(app, kProgramName, kModuleName);
    const reg32 named = calls([&] { return moduleWideNameOf(app, cpu, 0, kWideModuleName, 0x208); });
    if (cpu.terminate)
        return 0;
    put(app, kWideProgramName, kWideModuleName);

    // The command line's copy, and its arguments.
    const reg32 line = says(named, [&] { return windows<kGetCommandLineA, &host::commandLine>(app, cpu); });
    if (cpu.terminate)
        return 0;
    const reg32 copy = calls([&] { return copyCommandLineOf(app, cpu, line); });
    if (cpu.terminate)
        return 0;
    put(app, kCommandLine, copy);
    const reg32 narrowArguments = argumentsOf(app, copy, 1, [&](reg32 p) { return reg32(byte(app, p)); });
    put(app, kArguments, narrowArguments);

    // The wide one's, the same.
    const reg32 wide = says(narrowArguments, [&] { return windows<kGetCommandLineW, &host::commandLineWide>(app, cpu); });
    if (cpu.terminate)
        return 0;
    reg32 arguments = kNoWideCommandLine;
    if (wide != 0)
    {
        const reg32 wideCopy = calls([&] { return copyWideCommandLineOf(app, cpu, wide); });
        if (cpu.terminate)
            return 0;
        put(app, kWideCommandLine, wideCopy);
        arguments = argumentsOf(app, wideCopy, 2, [&](reg32 p) { return reg32(half(app, p)); });
    }
    put(app, kWideArguments, arguments);

    // A DLL's own name, and wide.
    if (dll != 0)
    {
        says(arguments,
             [&] { return windows<kGetModuleFileNameA, &host::moduleFileName>(app, cpu, module, kDllName, 0x104); });
        if (cpu.terminate)
            return 0;
        put(app, kDllNamePtr, kDllName);
        calls([&] { return moduleWideNameOf(app, cpu, module, kWideDllName, 0x208); });
        if (cpu.terminate)
            return 0;
        put(app, kWideDllNamePtr, kWideDllName);
    }
    return 1;
}

x86::reg32 NTInit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = ntInitOf(app, cpu, args.eax, args.edx, args.ebx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x4ff338 (the program's start): __NTInit for the module, the stack's limits, the exception
 * handling with the registration at `registration`, the thread's data at `data`, and the
 * initializers.  At its entry's depth: four pushes.  What the code leaves in eax: the initializers'
 * table's end; 0 with cpu.terminate set when the game stopped. */
reg32 ntMainOf(win32::WinApplication* app, x86::CPU& cpu, reg32 registration, reg32 data)
{
    const GuestFrame frame(cpu, 0x10, 0);
    reg32 module;
    {
        const EspKept kept(cpu);
        const auto handle = windowsAt<kGetModuleHandleA, &host::moduleHandle>(app, cpu, registration, 0);
        if (!handle)
            return 0;
        module = *handle;
    }
    auto calls = [&](auto&& run) {
        const ReturnSlot slot(cpu);
        return run();
    };
    calls([&] { return ntInitOf(app, cpu, 0, data, module); });
    if (cpu.terminate)
        return 0;
    const reg32 thread = threadPtrOf(app, cpu, kStackTop);
    if (cpu.terminate)
        return 0;
    calls([&] { return stackLimitsOf(app, cpu, thread, kStackTop); });
    if (cpu.terminate)
        return 0;
    calls([&] { return initExceptionHandlingOf(app, cpu, registration); });
    if (cpu.terminate)
        return 0;
    const reg32 end = calls([&] { return initRtnsOf(app, cpu, 0x21); });
    if (cpu.terminate)
        return 0;
    {
        const EspKept kept(cpu);
        if (!hook<kAccessIobHook>(app, cpu, {end}))
            return 0;
    }
    return calls([&] { return initRtnsOf(app, cpu, 0xff); });
}

x86::reg32 NTMain(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 eax = ntMainOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x524290 (the call of main): the stack's room checked -- grown by __GRO
 * where stackavail says it is short -- and where it ends noted; then the
 * game's main with argc and argv, and exit. */
x86::reg32 callMain(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.edx);
    push(app, cpu, args.ebp);
    args.ebp = cpu.esp;
    args.edx = at(app, kStackSize) + 3;
    setLow8(args.edx, args.edx & 0xfc);
    if (!direct(app, cpu, stackavail, args))
        return args.eax;
    if (args.edx < args.eax)
    {
        push(app, cpu, args.edx);
        if (!direct(app, cpu, GRO, args))
            return args.eax;
        args.eax = at(app, kStackSize) + 3;
        setLow8(args.eax, args.eax & 0xfc);
        cpu.esp -= args.eax;
        args.eax = cpu.esp;
    }
    else
        args.eax = 0;
    args.edx = at(app, kStackSize);
    args.eax += args.edx;
    put(app, kStackLimit, args.eax);
    if (!direct(app, cpu, startHook, args))
        return args.eax;
    args.edx = at(app, kArgv);
    args.eax = at(app, kArgc);
    if (!indirect(app, cpu, kGameMain, args) || !indirect(app, cpu, kExitEntry, args))
        return args.eax;
    cpu.esp = args.ebp;
    args.ebp = pop(app, cpu);
    args.edx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x5158a0 (the main thread's start): the initializers of priority 1, the
 * thread's data zeroed on the stack, the program's start with a
 * registration on the stack too, and main. */
x86::reg32 mainStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    for (const reg32 r : {args.ebx, args.ecx, args.edx, args.ebp})
        push(app, cpu, r);
    args.ebp = cpu.esp;
    cpu.esp -= 8;
    args.eax = 1;
    if (!direct(app, cpu, InitRtns, args))
        return args.eax;
    args.eax = at(app, kThreadDataSize) + 3;
    setLow8(args.eax, args.eax & 0xfc);
    args.edx = 0;
    cpu.esp -= args.eax;
    args.ecx = cpu.esp;
    args.ebx = at(app, kThreadDataSize);
    args.eax = args.ecx;
    if (!direct(app, cpu, memset, args))
        return args.eax;
    args.eax = at(app, kThreadDataSize);
    args.edx = args.ecx;
    put(app, args.ecx + 0xf0, args.eax);
    args.eax = args.ebp - 8;
    if (!direct(app, cpu, NTMain, args) || !direct(app, cpu, callMain, args))
        return args.eax;
    cpu.esp = args.ebp;
    for (reg32* r : {&args.ebp, &args.edx, &args.ecx, &args.ebx})
        *r = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4f7fa0 (_cstart_): the program's entry, a jmp to the main thread's
 * start. */
x86::reg32 cstart_(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    mainStart(app, cpu, args);
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

constexpr reg32 kMatherr = 0x56b16c;          // _matherr's pointer
constexpr reg32 kMathErrorNames = 0x56af70;   // the functions' names, by __reporterror's code
constexpr reg32 kMathTypeNames = 0x56b150;    // "DOMAIN", "SING" ...: the error's type's name
constexpr reg32 kMathErrorSeparator = 0x551fed;  // ": " between them
constexpr reg32 kStdStreams = 0x566f58;       // __iob
constexpr reg32 kFdivTable = 0x56ccdc;        // __fdiv_fpr's: 8 variants for each register
constexpr reg32 kFdivBadNibbles = 0x56ccc0;   // which of a divisor's top nibbles the bug can hit
constexpr reg32 kFdivScaleDown = 0x56ccd0, kFdivScaleUp = 0x56ccd4;  // 15/16 and 17/16, floats
constexpr reg32 kFdivDenormalScale = 0x56ccd8;  // 2^63, a float
constexpr reg32 kAtanTable = 0x56cb38;        // atan2's ways out, by the quadrant and the swap
constexpr reg32 kAtanSteps = 0x56cb6c, kAtanStep = 0x56cb70;  // the table's 16 steps, floats
constexpr reg32 kOne = 0x56cb28;              // 1.0, a double
constexpr reg32 kAtanPolynomial = 0x56cb74;   // 6 long doubles
constexpr reg32 kAtanAngles = 0x56cbb0;       // the steps' angles, long doubles 16 bytes apart
constexpr reg32 kHalfPiLong = 0x56cb58, kPiLong = 0x56cb62;  // pi/2 and pi, long doubles

x86::Float loadLong(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<x86::IEEEf80>(address));
}

void storeLong(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<x86::IEEEf80>(address) = x86::Float(value);
}

x86::Float loadSingle(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

/* _matherr's hook as __matherr's call makes it: eax and ebx the struct exception, the others kept;
 * its eax, nullopt when the game stopped (a call through the runtime's pointer, as hook's). */
std::optional<reg32> matherrHook(win32::WinApplication* app, x86::CPU& cpu, reg32 exception)
{
    const EspKept kept(cpu);
    x86::Args a;
    a.eax = exception;
    a.ebx = exception;
    return guest::pointerCall(app, cpu, at(app, kMatherr), a);
}

}  // namespace

/* 0x52e400 (__get_std_stream): __iob's `n`th FILE, the first four; 0 for another. */
reg32 stdStreamOf(reg32 n)
{
    return n > 3 ? 0 : n * 0x1a + kStdStreams;
}

x86::reg32 get_std_stream(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    pushed(app, cpu.esp, {args.edx});
    returns(cpu, args, stdStreamOf(args.eax));
    return args.eax;
}

/* 0x52b647 (the maths' error printed): "<type's name>: <function's name>" and a newline, to stderr,
 * by fputs and fputc -- each at the depth of its call, under three pushes (their room only).
 * fputc's say; 0 with cpu.terminate set when the game stopped. */
reg32 printMathErrorOf(win32::WinApplication* app, x86::CPU& cpu, reg32 type, reg32 name)
{
    const GuestFrame frame(cpu, 0xc, 0);
    const reg32 stream = stdStreamOf(2);
    const ReturnSlot slot(cpu);
    reg32 result;
    for (const reg32 s : {at(app, kMathTypeNames + type * 4), kMathErrorSeparator, name})
    {
        const EspKept kept(cpu);
        if (!putString(app, cpu, s, stream, result))
            return 0;
    }
    const EspKept kept(cpu);
    return putChar(app, cpu, '\n', stream, result) ? result : 0;
}

x86::reg32 printMathError(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 result = printMathErrorOf(app, cpu, args.eax, args.edx);
    if (cpu.terminate)
        return args.eax;
    returns(cpu, args, result);
    return args.eax;
}

/* 0x52b68f (__matherr's call): the struct exception at `exception` to _matherr; for a 0 from it, the
 * error printed, and errno EDOM for a domain error, else ERANGE; its retval given back in `retval`,
 * what the code leaves on the x87's stack.  At its entry's depth: ebx and edx pushed (their room
 * only).  What the code leaves in eax: _matherr's say, or the errno's setting's (the thread's data);
 * 0 with cpu.terminate set when the game stopped. */
reg32 callMatherrOf(win32::WinApplication* app, x86::CPU& cpu, reg32 exception, x86::Float& retval)
{
    const GuestFrame frame(cpu, 8, 0);
    const auto said = matherrHook(app, cpu, exception);
    if (!said)
        return 0;
    reg32 eax = *said;
    if (eax == 0)
    {
        {
            const ReturnSlot slot(cpu);
            printMathErrorOf(app, cpu, at(app, exception), at(app, exception + 4));
        }
        if (cpu.terminate)
            return 0;
        const ReturnSlot slot(cpu);
        const EspKept kept(cpu);
        if (!setThreadWord(app, cpu, kErrno, at(app, exception) == 1 ? kEDOM : kERANGE, &eax))
            return 0;
    }
    retval = loadDouble(app, exception + 0x18);
    return eax;
}

x86::reg32 callMatherr(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::Float retval;
    const reg32 eax = callMatherrOf(app, cpu, args.eax, retval);
    if (cpu.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x5243b1 (__reporterror's struct exception): the error's `flags` -- which function (the low 5
 * bits), and of what type -- its arguments the doubles at `a` and `b`: the struct built in its
 * frame, the result the type says -- -HUGE_VAL, 0, 1, HUGE_VAL or the second argument -- and
 * _matherr's call with it, its retval given back in `retval`.  A type none of the flags give is
 * `esi`'s, what the caller has in esi.  At its entry's depth: ebp, ecx and esi pushed (their room
 * only), then the struct's 0x20 bytes.  What the code leaves in eax; 0 with cpu.terminate set when
 * the game stopped. */
reg32 reportErrorOf(win32::WinApplication* app, x86::CPU& cpu, reg32 flags, reg32 a, reg32 b, reg32 esi,
                    x86::Float& retval)
{
    reg32 type = esi;
    const GuestFrame frame(cpu, 0xc, 0x20);
    const reg32 exception = frame - 0x20;
    put(app, exception + 8, at(app, a));
    put(app, exception + 0xc, at(app, a + 4));
    put(app, exception + 0x10, at(app, b));
    put(app, exception + 0x14, at(app, b + 4));
    const reg8 low = reg8(flags), high = reg8(flags >> 8);
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
    const reg32 result = exception + 0x18;
    if (high & 0x10)
    {
        const x86::Float huge = loadDouble(app, kHugeVal);
        const x86::Float negative = -huge;
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
    const ReturnSlot slot(cpu);
    return callMatherrOf(app, cpu, exception, retval);
}

x86::reg32 reportError(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::Float retval;
    const reg32 eax = reportErrorOf(app, cpu, args.eax, args.edx, args.ebx, args.esi, retval);
    if (cpu.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(cpu, args, eax);
    return args.eax;
}

/* 0x5243a7 (__reporterror): reportError with the argument at `a` for both.  At its entry's depth:
 * ebx pushed (its room only). */
reg32 reporterrorOf(win32::WinApplication* app, x86::CPU& cpu, reg32 flags, reg32 a, reg32 esi,
                    x86::Float& retval)
{
    const GuestFrame frame(cpu, 4, 0);
    const ReturnSlot slot(cpu);
    return reportErrorOf(app, cpu, flags, a, a, esi, retval);
}

x86::reg32 reporterror(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::Float retval;
    const reg32 eax = reporterrorOf(app, cpu, args.eax, args.edx, args.esi, retval);
    if (cpu.terminate)
        return args.eax;
    args.floatSet = true;
    args.floatResult = retval;
    returns(cpu, args, eax);
    return args.eax;
}

/* The two as typed functions (native_watcom.h): what the code leaves on the x87's stack given back,
 * as guest::direct calls them -- the return address's slot under esp.  Every flags their callers
 * hand them names a type, so esi, which only a flags that names none reads, is not theirs. */
double clib::reporterror(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 flags, x86::reg32 argument)
{
    x86::Float retval = 0.0;
    reporterrorOf(app, cpu, flags, argument, 0, retval);
    return cpu.terminate ? 0.0 : double(retval);
}

double clib::reportError(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 flags, x86::reg32 a, x86::reg32 b)
{
    x86::Float retval = 0.0;
    reportErrorOf(app, cpu, flags, a, b, 0, retval);
    return cpu.terminate ? 0.0 : double(retval);
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
x86::reg32 variadic(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<x86::reg32> named,
                    const guest::Words& words, Run run)
{
    const x86::reg32 esp = cpu.esp + 4;
    x86::reg32 slot = esp - 4 * x86::reg32(named.size() + words.count);
    const x86::reg32 first = slot;
    for (const x86::reg32 word : named)
    {
        put(app, slot, word);
        slot += 4;
    }
    for (unsigned i = 0; i < words.count; ++i)
    {
        put(app, slot, words.word[i]);
        slot += 4;
    }
    cpu.esp = first - 4;
    const x86::reg32 n = run(first);
    if (cpu.terminate)
        return 0;
    cpu.esp = esp - 4;
    return n;
}

}  // namespace

x86::reg32 clib::sprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 format,
                         guest::Words words)
{
    // On the host, with the words in hand, where it can be (hostSprintf); eax its count, as
    // sprintf's code leaves it.
    if (const auto n = hostSprintf(app, Segments{}, buffer, format, words))
        return *n;
    return variadic(app, cpu, {buffer, format}, words,
                    [&](x86::reg32 first) { return sprintfOf(app, cpu, Segments{}, buffer, format, first + 8); });
}

x86::reg32 clib::fprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 format,
                         guest::Words words)
{
    return variadic(app, cpu, {stream, format}, words,
                    [&](x86::reg32 first) { return fprintfOf(app, cpu, Segments{}, stream, format, first + 8); });
}

x86::reg32 clib::sscanf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string, x86::reg32 format,
                        guest::Words words)
{
    return variadic(app, cpu, {string, format}, words, [&](x86::reg32 first) {
        const EspKept kept(cpu);
        return sscanfOf(app, cpu, string, format, first + 8);
    });
}

namespace
{

/* A function of the runtime run as the game's code calls it: `registers` into eax, edx, ebx and
 * ecx in turn, every other register kept, eax back -- 0 when the game stopped in it. */
x86::reg32 runs(win32::WinApplication* app, x86::CPU& cpu, win32::ThunkPtr function,
                std::initializer_list<x86::reg32> registers)
{
    const EspKept kept(cpu);
    x86::Args a = watcomArgs(registers);
    cpu.esp -= 4;
    const x86::reg32 eax = function(app, cpu, a);
    return cpu.terminate ? 0 : eax;
}

}  // namespace

x86::reg32 clib::vsprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 format,
                          x86::reg32 arguments)
{
    return called(cpu, [&] { return vsprintfOf(app, cpu, Segments{}, buffer, format, arguments); });
}

x86::reg32 clib::mkTmpFile(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 number)
{
    return mkTmpFileOf(app, cpu, name, number);
}

x86::reg32 clib::getche(win32::WinApplication* app, x86::CPU& cpu)
{
    return getcheOf(app, cpu);
}

x86::reg32 clib::setErrno(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
{
    x86::reg32 data;
    return setThreadWord(app, cpu, kErrno, value, &data) ? data : 0;
}

x86::reg32 clib::setERANGE(win32::WinApplication* app, x86::CPU& cpu)
{
    return setErangeOf(app, cpu);
}

x86::reg32 clib::setErrnoDos(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 error)
{
    return watcom::setErrnoDos(app, cpu, error) ? 0xffffffff : 0;
}

x86::reg32 clib::setIOMode(win32::WinApplication* app, x86::reg32 handle, x86::reg32 mode)
{
    setIoMode(app, handle, mode);
    return handle << 2;
}

x86::reg32 clib::mbslen(win32::WinApplication* app, x86::reg32 s)
{
    return mbslenOf(app, s);
}

x86::reg32 clib::mbstowcs(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 from, x86::reg32 most)
{
    return mbstowcsOf(app, cpu, to, from, most);
}

x86::reg32 clib::wputenv(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pair)
{
    return wputenvOf(app, cpu, pair, 0);  // edx, which a pair with text never has read
}

x86::reg32 clib::memmove(win32::WinApplication* app, x86::reg32 to, x86::reg32 from, x86::reg32 count)
{
    return memmoveOf(app, to, from, count);
}

x86::reg32 clib::itoa(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value, x86::reg32 out, x86::reg32 base)
{
    return itoaOf(app, cpu, value, out, base);
}

x86::reg32 clib::mbvtop(win32::WinApplication* app, x86::reg32 c, x86::reg32 to)
{
    return mbvtopOf(app, c, to);
}

x86::reg32 clib::mbclen(win32::WinApplication* app, x86::reg32 p)
{
    return mbclenOf(app, p);
}

x86::reg32 clib::mbccmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b)
{
    return mbccmpOf(app, a, b);
}

x86::reg32 clib::mbsinc(win32::WinApplication* app, x86::reg32 p)
{
    return mbsincOf(app, p);
}

x86::reg32 clib::mbterm(win32::WinApplication* app, x86::reg32 p)
{
    return mbtermOf(app, p);
}

x86::reg32 clib::fatalRuntimeError(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 message, x86::reg32 code)
{
    // Its code still as it runs, register by register (the way out's): called as the game's call
    // makes it.
    return runs(app, cpu, &fatal_runtime_error, {message, code});
}

x86::reg32 clib::heapShrink(win32::WinApplication* app, x86::CPU& cpu)
{
    watcomheap::shrink(app, cpu, kGameHeap);
    return 0;
}

x86::reg32 clib::consoleOutput(win32::WinApplication* app, x86::CPU& cpu)
{
    return consoleOutputOf(app, cpu);
}

double clib::strtod(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 end, x86::reg32* eax)
{
    x86::reg32 left = 0;
    const double d = strtodOf(app, cpu, s, end, left);
    if (eax != nullptr)
        *eax = left;
    return d;
}

x86::reg32 clib::allocPOSIXHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 os)
{
    x86::reg32 handle = 0;
    return allocHandle(app, cpu, os, handle) ? handle : 0;
}

x86::reg32 clib::freePOSIXHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
{
    x86::reg32 eax = 0;
    return freeHandle(app, cpu, handle, eax) ? eax : 0;
}

x86::reg32 clib::initThreadData(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data)
{
    return initThreadDataOf(app, cpu, data);
}

x86::reg32 clib::initExceptionHandling(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 registration)
{
    return initExceptionHandlingOf(app, cpu, registration);
}

x86::reg32 clib::doneExceptionHandler(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
{
    NFS2_USE(eax);
    // Its code still as it runs, register by register (the way out's): called as the game's call
    // makes it.
    return runs(app, cpu, &DoneExceptionHandler, {});
}

x86::reg32 clib::malloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size)
{
    return called(cpu, [&] {
        x86::reg32 block;
        return watcomheap::allocate(app, cpu, kGameHeap, size, block) ? block : 0;
    });
}

x86::reg32 clib::free(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
{
    // What __nfree's code leaves in eax: the heap lock's giving back's.
    return called(cpu, [&] {
        x86::reg32 eax;
        return watcomheap::free(app, cpu, kGameHeap, block, &eax) ? eax : 0;
    });
}

x86::reg32 clib::realloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block, x86::reg32 size)
{
    // As the runtime's code is called: its return address's slot below the caller's esp, every
    // register but eax kept.
    const EspKept kept(cpu);
    const ReturnSlot slot(cpu);
    return nreallocOf(app, cpu, block, size);
}

x86::reg32 clib::calloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 size)
{
    return called(cpu, [&] { return callocOf(app, cpu, count, size); });
}

x86::reg32 clib::fopen(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 mode)
{
    return called(cpu, [&] {
        x86::reg32 result;
        const Frame frame(cpu, 4);  // ebx
        return nested(cpu, [&] { return openStream(app, cpu, name, mode, 0, result); }) ? result : 0;
    });
}

x86::reg32 clib::fclose(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return closeStream(app, cpu, stream, result) ? result : 0;
    });
}

x86::reg32 clib::fread(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size,
                       x86::reg32 count, x86::reg32 stream)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return readStream(app, cpu, buffer, size, count, stream, result) ? result : 0;
    });
}

x86::reg32 clib::streamShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 close)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return shutdownStream(app, cpu, stream, close, result) ? result : 0;
    });
}

void clib::ldScale10x(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value, x86::reg32 n)
{
    called(cpu, [&] {
        x86::IEEEf80 number = longDoubleAt(app, value);
        ldScale10xOf(app, cpu.fenv.control.word, number, n);
        putLongDouble(app, value, number);
        return 0;
    });
}

x86::reg32 clib::bsearch(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key, x86::reg32 base, x86::reg32 count,
                         x86::reg32 size, x86::reg32 function)
{
    return called(cpu, [&] {
        x86::reg32 found;
        return bsearchOf(app, cpu, cpu.esp, key, base, count, size, function, found) ? found : 0;
    });
}

x86::reg32 clib::fgets(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 n, x86::reg32 stream)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return getString(app, cpu, buffer, n, stream, result) ? result : 0;
    });
}

x86::reg32 clib::strupr(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string)
{
    return called(cpu, [&] {
        for (x86::reg32 at = string; byte(app, at) != 0; ++at)
            if (byte(app, at) >= 'a' && byte(app, at) <= 'z')
                putByte(app, at, x86::reg8(byte(app, at) - 0x20));
        return string;
    });
}

x86::reg32 clib::fseek(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 offset,
                       x86::reg32 whence)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return seekStream(app, cpu, stream, offset, whence, result) ? result : 0;
    });
}

x86::reg32 clib::qsort(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 base, x86::reg32 count,
                       x86::reg32 size, x86::reg32 compare)
{
    called(cpu, [&] { return qsortOf(app, cpu, base, count, size, compare); });
    return 0;  // what its code leaves in eax is nothing of its
}

x86::reg32 clib::strtok(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string, x86::reg32 parts)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return strtokOf(app, cpu, string, parts, result) ? result : 0;
    });
}

x86::reg32 clib::rand(win32::WinApplication* app, x86::CPU& cpu)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return randOf(app, cpu, result) ? result : 0;
    });
}

x86::reg32 clib::getenv(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
{
    // As the runtime's code is called: its return address's slot below the caller's esp.
    const EspKept kept(cpu);
    cpu.esp -= 4;
    return getenvOf(app, cpu, name);
}

x86::reg32 clib::exit(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 status)
{
    return runs(app, cpu, &watcom::exit, {status});
}

x86::reg32 clib::srand(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 seed)
{
    // guest::direct has put the return address's slot under esp and kept the registers.
    x86::reg32 old;
    return srandOf(app, cpu, seed, old) ? old : 0;
}

x86::reg32 clib::fwrite(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size,
                        x86::reg32 count, x86::reg32 stream)
{
    x86::reg32 result;
    return writeStream(app, cpu, buffer, size, count, stream, result) ? result : 0;
}

x86::reg32 clib::time(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 timer)
{
    return timeOf(app, cpu, timer).value_or(0);
}

x86::reg32 clib::getcwd(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size)
{
    return getcwdOf(app, cpu, buffer, size).value_or(0);
}

x86::reg32 clib::getdiskfree(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 drive, x86::reg32 info)
{
    return getdiskfreeOf(app, cpu, drive, info).value_or(0);
}

x86::reg32 clib::stackavail(win32::WinApplication* app, x86::CPU& cpu)
{
    return stackavailOf(app, cpu, 0).value_or(0);
}

x86::reg32 clib::stackProbed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size)
{
    stackProbedOf(app, cpu, size, 0, 0);
    return 0;
}

x86::reg32 clib::setenv(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 value,
                        x86::reg32 replace)
{
    return setenvTopOf(app, cpu, name, value, replace);
}

x86::reg32 clib::atexit(win32::WinApplication* app, x86::reg32 function)
{
    return atexitOf(app, function);
}

x86::reg32 clib::unlink(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
{
    return called(cpu, [&] {
        x86::reg32 result;
        const auto deleted = windows<kDeleteFileA, &host::deleteFile>(app, cpu, path);
        return deleted && windowsResult(app, cpu, *deleted, result) ? result : 0;
    });
}

x86::reg32 clib::remove(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
{
    return clib::unlink(app, cpu, path);
}

x86::reg32 clib::chdir(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
{
    return called(cpu, [&] {
        x86::reg32 result;
        const auto changed = windows<kSetCurrentDirectoryA, &host::setCurrentDirectory>(app, cpu, path);
        return changed && windowsResult(app, cpu, *changed, result) ? result : 0;
    });
}

x86::reg32 clib::rmdir(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
{
    return called(cpu, [&] {
        x86::reg32 result;
        const auto removed = windows<kRemoveDirectoryA, &host::removeDirectory>(app, cpu, path);
        return removed && windowsResult(app, cpu, *removed, result) ? result : 0;
    });
}

x86::reg32 clib::chmod(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 mode)
{
    return called(cpu, [&] {
        const x86::reg32 entry = cpu.esp;
        x86::reg32 attributes, result;
        if (!fileAttributes(app, cpu, entry - 0xc, path, attributes))  // ebx, ecx, and the call
            return x86::reg32(0);
        cpu.esp = entry;
        if (attributes == 0xffffffff)
        {
            result = 0xffffffff;
            if (!setErrnoNt(app, cpu))
                return x86::reg32(0);
        }
        else
        {
            attributes = (attributes & ~x86::reg32(1)) | ((mode & 0x80) != 0 ? 0 : 1);  // FILE_ATTRIBUTE_READONLY
            const auto set = windows<kSetFileAttributesA, &host::setFileAttributes>(app, cpu, path, attributes);
            if (!set || !windowsResult(app, cpu, *set, result))
                return x86::reg32(0);
        }
        return result;
    });
}

x86::reg32 clib::access(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 mode)
{
    return called(cpu, [&] {
        const auto found = windows<kGetFileAttributesA, &host::fileAttributes>(app, cpu, path);
        if (!found)
            return x86::reg32(0);
        const x86::reg32 attributes = *found;
        x86::reg32 result = 0;
        if (attributes == 0xffffffff)
        {
            result = 0xffffffff;
            if (!setErrnoNt(app, cpu))
                return x86::reg32(0);
        }
        else if ((mode & 2) != 0 && (attributes & 1) != 0)
        {
            result = 0xffffffff;
            if (!setErrnoDos(app, cpu, 5))  // ERROR_ACCESS_DENIED
                return x86::reg32(0);
        }
        return result;
    });
}

x86::reg32 clib::ftell(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream)
{
    return called(cpu, [&] {
        x86::reg32 result;
        return tellStream(app, cpu, stream, result) ? result : 0;
    });
}

x86::reg32 clib::vfprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 format,
                          x86::reg32 argp)
{
    return called(cpu, [&] {
        const x86::reg32 n = vfprintfOf(app, cpu, segmentsOf(cpu), stream, format, argp);
        return cpu.terminate ? x86::reg32(0) : n;
    });
}

x86::reg32 clib::fflush(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream)
{
    return called(cpu, [&] {
        x86::reg32 result = 0;
        if (stream != 0)
            return flushStream(app, cpu, stream, result) ? result : x86::reg32(0);
        x86::reg32 count;
        nested(cpu, [&] { return flushAll(app, cpu, 0xffffffff, count); });
        return x86::reg32(0);
    });
}

x86::reg32 clib::mkdir(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
{
    return called(cpu, [&] {
        x86::reg32 result;
        const auto made = windows<kCreateDirectoryA, &host::createDirectory>(app, cpu, path, 0);
        return made && windowsResult(app, cpu, *made, result) ? result : 0;
    });
}

}  // namespace watcom
}  // namespace nfs3hp
