#include "native_guest.h"
#include "native_watcom_heap.h"
#include <lib/thread.h>
#include <algorithm>
#include <bitset>
#include <cstring>
#include <utility>

/* The Watcom C runtime the game was linked with, in C++: the functions of it
 * the port has natively, where their code was -- disassemble_nfs3hp.py binds
 * each address to the function of that name here (GAME_NATIVES), and the
 * generated code calls it by name.  The names are Open Watcom's (bld/clib,
 * bld/mathlib), the runtime's own descendant, which says what each does;
 * src/nfs3hp/watcom.md maps all of the runtime in nfs3.exe, what is native
 * and what is not yet.  tools/game_checks generates the game's code for them
 * afresh and holds each one against it.
 *
 * They take their arguments in registers, as Watcom's runtime does -- eax,
 * edx, ebx, ecx -- and keep every other register, as it does; the registers
 * an argument came in are the caller's no longer, and are left as they are
 * here.  What they read and write of the runtime's own state -- the
 * per-thread data, the character table, the heap -- is where the runtime
 * keeps it, in guest memory, as the code that is still generated expects. */

namespace nfs3hp
{
namespace watcom
{

namespace
{

using guest::at;
using guest::byte;
using guest::call;
using guest::put;
using guest::putByte;

/* The runtime's per-thread data: [__GetThreadPtr]() -- the pointer the
 * runtime swaps for its own when a second thread starts. */
constexpr x86::reg32 kGetThreadPtr = 0x567764;
constexpr x86::reg32 kThreadData = 0x9f642c;  // the single thread's, what GetThreadPtr hands back
constexpr x86::reg32 kRandomSeed = 0xc;        // rand's, in the thread's data
constexpr x86::reg32 kNextToken = 0x10;        // strtok's
/* _IsTable: a byte of flags for each character, EOF first. */
constexpr x86::reg32 kIsTable = 0x564ef0;
constexpr x86::reg8 kSpace = 0x02;
constexpr x86::reg8 kDigit = 0x20;

/* The thread's data, as the runtime asks for it; false when the game
 * stopped. */
bool threadData(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& data)
{
    if (!call(app, cpu, at(app, kGetThreadPtr)))
        return false;
    data = cpu.eax;
    return true;
}

void returns(x86::CPU& cpu, x86::reg32 result)
{
    cpu.eax = result;
    cpu.esp += 4;
}

/* The registers but eax, as a function had them when it was called. */
struct Kept
{
    x86::reg32 ebx, ecx, edx, esi, edi, ebp;
    explicit Kept(const x86::CPU& cpu)
        : ebx(cpu.ebx), ecx(cpu.ecx), edx(cpu.edx), esi(cpu.esi), edi(cpu.edi), ebp(cpu.ebp)
    {
    }
    void back(x86::CPU& cpu) const
    {
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
    }
};

}  // namespace

/* __CHP (mathlib/a/fchop87.asm, 0x4dfd56): st(0) rounded toward zero -- the
 * game's step from a float to an integer, before every fistp -- by frndint
 * under the caller's control word with truncation in it.  Nothing else
 * changes: the registers, the control word, the status word. */
namespace
{

void chop(x86::CPU& cpu)
{
    const auto rc = cpu.fpu.control.rc;
    cpu.fpu.control.rc = 3;
    cpu.fpu.st(0) = cpu.fpu.rndint(cpu.fpu.st(0));
    cpu.fpu.control.rc = rc;
}

}  // namespace

void CHP(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    chop(cpu);
    cpu.esp += 4;
}

/* __GetThreadPtr (0x4ff0ec): the single thread's data, until a second thread
 * starts and the runtime points kGetThreadPtr at its own. */
void GetThreadPtr(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, at(app, kThreadData));
}

/* __NullAccessRtn (0x4ff0f4): what the runtime's locks are -- the streams',
 * the heap's -- while there is one thread: nothing. */
void NullAccess(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.esp += 4;
}

/* memset (clib/memory/c/memset.c, 0x4e0640): eax the buffer, edx the byte,
 * ebx how many; the buffer back. */
void memset(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.ebx != 0)
        std::memset(&app->getMemory<x86::reg8>(cpu.eax), int(cpu.edx & 0xff), cpu.ebx);
    returns(cpu, cpu.eax);
}

/* __STOSD (clib/cgsupp/a/__stos.asm, 0x4ff4f7): ecx dwords of edx from eax
 * on -- one by one to a 32-byte boundary, then four at a time, then up to
 * three; eax left past them, and ecx 0, but 1 after three at the end (the
 * last of them is not counted down). */
void STOSD(win32::WinApplication* app, x86::CPU& cpu)
{
    for (; cpu.ecx != 0 && (cpu.eax & 0x1f) != 0; --cpu.ecx, cpu.eax += 4)
        put(app, cpu.eax, cpu.edx);
    const x86::reg32 tail = cpu.ecx & 3;
    for (; cpu.ecx != 0; --cpu.ecx, cpu.eax += 4)
        put(app, cpu.eax, cpu.edx);
    cpu.ecx = tail == 3 ? 1 : 0;
    cpu.esp += 4;
}

/* __STOSB (clib/cgsupp/a/__stos.asm, 0x4ff4c0): ecx bytes of the pattern in
 * edx from eax on -- byte by byte up to a dword boundary, the pattern turned
 * a byte for each, then dwords (__STOSD), then up to three bytes more: dl,
 * dh, dl.  eax is left past the dwords, edx as turned, ecx 0 -- or 1 after
 * three bytes at the end, as __STOSD. */
void STOSB(win32::WinApplication* app, x86::CPU& cpu)
{
    for (; cpu.ecx != 0 && (cpu.eax & 3) != 0; --cpu.ecx, ++cpu.eax)
    {
        putByte(app, cpu.eax, x86::reg8(cpu.edx));
        cpu.edx = cpu.edx >> 8 | cpu.edx << 24;
    }
    const x86::reg32 tail = cpu.ecx & 3;
    for (cpu.ecx >>= 2; cpu.ecx != 0; --cpu.ecx, cpu.eax += 4)
        put(app, cpu.eax, cpu.edx);
    const x86::reg8 bytes[] = {x86::reg8(cpu.edx), x86::reg8(cpu.edx >> 8), x86::reg8(cpu.edx)};
    for (x86::reg32 i = 0; i < tail; ++i)
        putByte(app, cpu.eax + i, bytes[i]);
    cpu.ecx = tail == 3 ? 1 : 0;
    cpu.esp += 4;
}

/* strcmp (clib/string/a/cmp386.asm, 0x4ee310): eax and edx, the strings;
 * -1, 0 or 1 as the first byte that differs, unsigned, is below or above. */
void strcmp(win32::WinApplication* app, x86::CPU& cpu)
{
    for (x86::reg32 a = cpu.eax, b = cpu.edx;; ++a, ++b)
    {
        const x86::reg8 c = byte(app, a), d = byte(app, b);
        if (c != d)
            return returns(cpu, c < d ? 0xffffffff : 1);
        if (c == 0)
            return returns(cpu, 0);
    }
}

/* strncmp (clib/string/c/strncmp.c, 0x4eb390): eax and edx, the strings, ebx
 * the most to compare; the first difference of unsigned bytes, or 0. */
void strncmp(win32::WinApplication* app, x86::CPU& cpu)
{
    for (x86::reg32 a = cpu.eax, b = cpu.edx, n = cpu.ebx; n != 0; ++a, ++b, --n)
    {
        const x86::reg8 c = byte(app, a), d = byte(app, b);
        if (c != d)
            return returns(cpu, x86::reg32(c) - x86::reg32(d));
        if (c == 0)
            break;
    }
    returns(cpu, 0);
}

/* _stricmp (clib/string/c/stricmp.c, 0x4e9ec0): strcmp's difference, unsigned,
 * with A to Z taken as a to z and nothing else folded. */
void stricmp(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto lower = [](x86::reg8 c) { return x86::reg8(c >= 'A' && c <= 'Z' ? c + 0x20 : c); };
    for (x86::reg32 a = cpu.eax, b = cpu.edx;; ++a, ++b)
    {
        const x86::reg8 c = lower(byte(app, a)), d = lower(byte(app, b));
        if (c != d || d == 0)
            return returns(cpu, x86::reg32(c) - x86::reg32(d));
    }
}

/* strncpy (clib/string/c/strncpy.c, 0x4e0e30): eax the buffer, edx the
 * string, ebx how many bytes: the string, then zeros to make up the count. */
void strncpy(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 to = cpu.eax, from = cpu.edx, n = cpu.ebx;
    for (; n != 0 && byte(app, from) != 0; --n)
        putByte(app, to++, byte(app, from++));
    for (; n != 0; --n)
        putByte(app, to++, 0);
    returns(cpu, cpu.eax);
}

/* strncat (clib/string/c/strncat.c, 0x4eedf0): eax the string, edx what to add,
 * ebx at most how much of it; always ended. */
void strncat(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 to = cpu.eax;
    while (byte(app, to) != 0)
        ++to;
    for (x86::reg32 from = cpu.edx, n = cpu.ebx; n != 0; --n, ++to, ++from)
    {
        const x86::reg8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
    putByte(app, to, 0);
    returns(cpu, cpu.eax);
}

/* strrchr (clib/string/c/strrchr.c, 0x4ee2f0): eax the string, dl the byte;
 * where it is last, the end included, or 0. */
void strrchr(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8 wanted = x86::reg8(cpu.edx);
    x86::reg32 found = 0;
    for (x86::reg32 at = cpu.eax;; ++at)
    {
        const x86::reg8 c = byte(app, at);
        if (c == wanted)
            found = at;
        if (c == 0)
            break;
    }
    returns(cpu, found);
}

/* strstr (clib/string/c/strstr.c, 0x4e0820): eax the string, edx what to look
 * for; where it first is, or 0 -- the string itself for an empty one. */
void strstr(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 text = cpu.eax, wanted = cpu.edx;
    x86::reg32 length = 0;
    while (byte(app, wanted + length) != 0)
        ++length;
    for (x86::reg32 at = text;; ++at)
    {
        x86::reg32 same = 0;
        while (same < length && byte(app, at + same) == byte(app, wanted + same))
            ++same;
        if (same == length)
            return returns(cpu, at);
        if (byte(app, at) == 0)
            return returns(cpu, 0);
    }
}

/* __setbits (clib/string/c/setbits.c, 0x50c540): eax 32 bytes, made the set
 * of the bytes of the string at edx -- a bit each, the low bit first. */
void setbits(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 vector = cpu.eax;
    std::memset(&app->getMemory<x86::reg8>(vector), 0, 0x20);
    for (x86::reg32 at = cpu.edx; byte(app, at) != 0; ++at)
    {
        const x86::reg8 c = byte(app, at);
        putByte(app, vector + (c >> 3), x86::reg8(byte(app, vector + (c >> 3)) | 1 << (c & 7)));
    }
    cpu.esp += 4;
}

/* strtok (clib/string/c/strtok.c, 0x4eff60): eax the string -- 0 to go on
 * where the last call stopped, kept in the thread's data -- edx the bytes
 * that part the tokens; the next token, ended, or 0 when there is none. */
void strtok(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 text = cpu.eax;
    const x86::reg32 parts = cpu.edx;
    x86::reg32 data = 0;
    if (text == 0)
    {
        if (!threadData(app, cpu, data))
            return;
        text = at(app, data + kNextToken);
        if (text == 0)
            return returns(cpu, 0);
    }
    std::bitset<256> parting;
    for (x86::reg32 at = parts; byte(app, at) != 0; ++at)
        parting.set(byte(app, at));
    while (byte(app, text) != 0 && parting.test(byte(app, text)))
        ++text;
    if (byte(app, text) == 0)
        return returns(cpu, 0);
    for (x86::reg32 at = text; byte(app, at) != 0; ++at)
        if (parting.test(byte(app, at)))
        {
            putByte(app, at, 0);
            if (!threadData(app, cpu, data))
                return;
            put(app, data + kNextToken, at + 1);
            return returns(cpu, text);
        }
    if (!threadData(app, cpu, data))
        return;
    put(app, data + kNextToken, 0);
    returns(cpu, text);
}

/* _strupr (clib/string/c/strupr.c, 0x4f8020): a to z of the string at eax made
 * A to Z, in place; the string back. */
void strupr(win32::WinApplication* app, x86::CPU& cpu)
{
    for (x86::reg32 at = cpu.eax; byte(app, at) != 0; ++at)
        if (byte(app, at) >= 'a' && byte(app, at) <= 'z')
            putByte(app, at, x86::reg8(byte(app, at) - 0x20));
    returns(cpu, cpu.eax);
}

/* toupper and tolower (clib/char/c, 0x4eede0 and 0x4f0ae0): eax a character,
 * moved between a-z and A-Z; anything else as it is. */
void toupper(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    const x86::sreg32 c = x86::sreg32(cpu.eax);
    returns(cpu, c >= 'a' && c <= 'z' ? cpu.eax - 0x20 : cpu.eax);
}

void tolower(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    const x86::sreg32 c = x86::sreg32(cpu.eax);
    returns(cpu, c >= 'A' && c <= 'Z' ? cpu.eax + 0x20 : cpu.eax);
}

/* atoi and atol (clib/convert/c, 0x4f0430 and 0x4f0490, the same code): the
 * number at eax, past white space, a sign and then digits as _IsTable has
 * them, in 32 bits as it wraps.  The table is looked up at the character
 * plus one in a byte: 0xff at EOF's place. */
void atoi(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto is = [&](x86::reg8 c, x86::reg8 what) {
        return (byte(app, kIsTable + x86::reg8(c + 1)) & what) != 0;
    };
    x86::reg32 at = cpu.eax;
    while (is(byte(app, at), kSpace))
        ++at;
    const x86::reg8 sign = byte(app, at);
    if (sign == '+' || sign == '-')
        ++at;
    x86::reg32 value = 0;
    for (; is(byte(app, at), kDigit); ++at)
        value = value * 10 + byte(app, at) - '0';
    returns(cpu, sign == '-' ? 0 - value : value);
}

void atol(win32::WinApplication* app, x86::CPU& cpu)
{
    atoi(app, cpu);
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
void initrandnext(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 seed;
    if (seedAt(app, cpu, seed))
        returns(cpu, seed);
}

/* rand (clib/math/c/rand.c, 0x4eb1fc): the seed taken on, x 1103515245 +
 * 12345, and bits 16 to 30 of it. */
void rand(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 seed;
    if (!seedAt(app, cpu, seed))
        return;
    if (seed == 0)
        return returns(cpu, 0);
    const x86::reg32 next = at(app, seed) * 0x41c64e6d + 0x3039;
    put(app, seed, next);
    returns(cpu, next >> 16 & 0x7fff);
}

/* srand (clib/math/c/rand.c, 0x4eb220): eax the seed. */
void srand(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 value = cpu.eax;
    x86::reg32 seed;
    if (!seedAt(app, cpu, seed))
        return;
    if (seed != 0)
        put(app, seed, value);
    returns(cpu, seed);
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
    cpu.eax = a;
    cpu.edx = b;
    if (!call(app, cpu, function))
        return false;
    result = x86::sreg32(cpu.eax);
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
void med3(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 chosen;
    if (!median(app, cpu, entry, cpu.ecx, cpu.eax, cpu.edx, cpu.ebx, chosen))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, chosen);
}

/* byteswap (clib/search/c/qsortrtn.c, 0x4ee4d0): ecx bytes at esi and edi
 * exchanged -- its own convention, esi and edi left past them, ecx and edx
 * 0. */
void byteswap(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = exchange(app, cpu.esi, cpu.edi, cpu.ecx, cpu.eax);
    cpu.esi += cpu.ecx;
    cpu.edi += cpu.ecx;
    cpu.ecx = 0;
    cpu.edx = 0;
    cpu.esp += 4;
}

/* qsort (clib/search/c/qsortrtn.c, 0x4ee4f8): edx elements of ebx bytes at
 * eax sorted by the function at ecx.  Fewer than 16 by a shell sort, steps of
 * 3 and 1; more by quicksort, the pivot the median of 3 from 30 on, of 9 from
 * 43 on, the elements equal to it gathered at both ends and swapped to the
 * middle, the larger part kept for later.  An array of dwords, aligned, has
 * its pivot copied into the frame and the function handed that. */
void qsort(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 base = cpu.eax;
    x86::reg32 n = cpu.edx;
    const x86::reg32 size = cpu.ebx;
    const x86::reg32 function = cpu.ecx;
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
                            return;
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
                        return;
                }
                if (!median(app, cpu, frame - 4, function, p1, mid, p2, mid))
                    return;
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
                        return;
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
                        return;
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
    kept.back(cpu);
    cpu.esp = entry + 4;
}

/* bsearch (clib/search/c/bsearch.c, 0x4fb300): the element of edx's ebx, ecx
 * bytes each, that the function on the stack finds equal to the key at eax,
 * halving from both ends; or 0. */
void bsearch(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 key = cpu.eax, base = cpu.edx, count = cpu.ebx, size = cpu.ecx;
    const x86::reg32 function = at(app, entry + 4);
    const x86::reg32 frame = entry - 0x1c;  // esi, edi, es, fs, gs, ebp, and the function
    x86::reg32 found = 0;
    if (count != 0)
    {
        x86::reg32 low = base, high = base + (count - 1) * size;
        x86::sreg32 r = 1;
        while (low < high)
        {
            const x86::reg32 mid = low + (high - low) / size / 2 * size;
            if (!compare(app, cpu, frame, function, key, mid, r))
                return;
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
                return;
            found = r == 0 ? high : 0;
        }
    }
    kept.back(cpu);
    cpu.eax = found;
    cpu.esp = entry + 8;
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

/* A word of the thread's data set; false when the game stopped. */
bool setThreadWord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 offset, x86::reg32 value)
{
    x86::reg32 data;
    if (!threadData(app, cpu, data))
        return false;
    put(app, data + offset, value);
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
    return call(app, cpu, at(app, kGetLastError)) && setErrnoDos(app, cpu, cpu.eax);
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
void set_errno(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setThreadWord(app, cpu, kErrno, cpu.eax))
        return;
    const x86::reg32 data = cpu.eax;
    kept.back(cpu);
    returns(cpu, data);
}

/* __set_ERANGE (0x5028a4): errno ERANGE. */
void set_ERANGE(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setThreadWord(app, cpu, kErrno, kERANGE))
        return;
    const x86::reg32 data = cpu.eax;
    kept.back(cpu);
    returns(cpu, data);
}

/* __set_doserrno (0x5028c8): _doserrno eax. */
void set_doserrno(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setThreadWord(app, cpu, kDosErrno, cpu.eax))
        return;
    const x86::reg32 data = cpu.eax;
    kept.back(cpu);
    returns(cpu, data);
}

/* __set_errno_dos (0x500f8c): errno for the Windows error in eax; -1. */
void set_errno_dos(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setErrnoDos(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    returns(cpu, 0xffffffff);
}

/* __set_errno_nt (0x500ff4): errno for GetLastError's; -1. */
void set_errno_nt(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setErrnoNt(app, cpu))
        return;
    kept.back(cpu);
    returns(cpu, 0xffffffff);
}

/* ---- Files and directories: each a Windows function on the path at eax,
 * 0 or -1 and errno.  Those that hand Windows a buffer of their own have it
 * where their frame has it, esp below it. ---- */

namespace
{

/* A function of one call to Windows on the path, BOOL its result. */
void pathCall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, std::initializer_list<x86::reg32> arguments)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!call(app, cpu, at(app, slot), arguments) || !windowsResult(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
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
    if (!call(app, cpu, at(app, kFindFirstFileA), {path, found}))
        return false;
    if (cpu.eax != 0xffffffff)
        attributes = at(app, found);
    return true;
}

}  // namespace

/* unlink (clib/handleio/c/unlnkwnt.c, 0x4e28a0): DeleteFileA. */
void unlink(win32::WinApplication* app, x86::CPU& cpu)
{
    pathCall(app, cpu, kDeleteFileA, {cpu.eax});
}

/* remove (clib/file/c/remove.c, 0x4ee2e0): unlink. */
void remove(win32::WinApplication* app, x86::CPU& cpu)
{
    unlink(app, cpu);
}

/* mkdir (clib/direct/c/mkdirwnt.c, 0x4eb1d0): CreateDirectoryA, no security
 * attributes. */
void mkdir(win32::WinApplication* app, x86::CPU& cpu)
{
    pathCall(app, cpu, kCreateDirectoryA, {cpu.eax, 0});
}

/* rmdir (clib/direct/c/rmdirwnt.c, 0x4ee400): RemoveDirectoryA. */
void rmdir(win32::WinApplication* app, x86::CPU& cpu)
{
    pathCall(app, cpu, kRemoveDirectoryA, {cpu.eax});
}

/* chdir (clib/direct/c/chdirwnt.c, 0x4f0260): SetCurrentDirectoryA. */
void chdir(win32::WinApplication* app, x86::CPU& cpu)
{
    pathCall(app, cpu, kSetCurrentDirectoryA, {cpu.eax});
}

/* chmod (clib/file/c/chmodwnt.c, 0x4ee3c0): the file read-only unless edx
 * has S_IWRITE (0x80), its other attributes as __fixed_GetFileAttributesA
 * finds them. */
void chmod(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 path = cpu.eax, mode = cpu.edx;
    x86::reg32 attributes, result;
    if (!fileAttributes(app, cpu, entry - 0xc, path, attributes))  // ebx, ecx, and the call
        return;
    cpu.esp = entry;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return;
    }
    else
    {
        attributes = (attributes & ~x86::reg32(1)) | ((mode & 0x80) != 0 ? 0 : 1);  // FILE_ATTRIBUTE_READONLY
        if (!call(app, cpu, at(app, kSetFileAttributesA), {path, attributes})
            || !windowsResult(app, cpu, cpu.eax, result))
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

/* access (clib/file/c/accsswnt.c, 0x4f05f0): 0 if the file is there, and
 * not read-only if edx asks for writing (2). */
void access(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    const x86::reg32 mode = cpu.edx;
    if (!call(app, cpu, at(app, kGetFileAttributesA), {cpu.eax}))
        return;
    const x86::reg32 attributes = cpu.eax;
    x86::reg32 result = 0;
    if (attributes == 0xffffffff)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return;
    }
    else if ((mode & 2) != 0 && (attributes & 1) != 0)
    {
        result = 0xffffffff;
        if (!setErrnoDos(app, cpu, 5))  // ERROR_ACCESS_DENIED
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

void nmalloc(win32::WinApplication* app, x86::CPU& cpu);
void nfree(win32::WinApplication* app, x86::CPU& cpu);
void sopen(win32::WinApplication* app, x86::CPU& cpu);

/* getcwd (clib/direct/c/gtcwdwnt.c, 0x4f0730): the current directory into
 * eax's edx bytes -- or, for no buffer, into a block _nmalloc gives of edx
 * bytes or as many as it takes -- or 0: errno ERANGE when it does not fit,
 * ENOMEM when there is no block.  What it copies is as long as
 * GetCurrentDirectoryA says, from its MAX_PATH bytes and on into the
 * registers it pushed -- here too. */
void getcwd(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 buffer = cpu.eax;
    const x86::reg32 size = cpu.edx;
    put(app, entry - 4, cpu.ebx);
    put(app, entry - 8, cpu.ecx);
    put(app, entry - 0xc, cpu.esi);
    put(app, entry - 0x10, cpu.edi);
    const x86::reg32 local = entry - 0x114;  // MAX_PATH bytes below them
    cpu.esp = local;
    if (!call(app, cpu, at(app, kGetCurrentDirectoryA), {0x104, local}))
        return;
    const x86::reg32 length = cpu.eax;
    x86::reg32 result = 0;
    if (length == 0)
    {
        if (!setErrnoNt(app, cpu))
            return;
    }
    else
    {
        bool fits = true;
        if (buffer == 0)
        {
            cpu.eax = std::max(size, length + 1);
            if (!call(app, cpu, nmalloc))
                return;
            buffer = cpu.eax;
            if (buffer == 0)
            {
                fits = false;
                if (!setThreadWord(app, cpu, kErrno, kENOMEM))
                    return;
            }
        }
        else if (length > size)
        {
            fits = false;
            if (!setThreadWord(app, cpu, kErrno, kERANGE))
                return;
        }
        if (fits)
        {
            for (x86::reg32 i = 0; i < length + 1; ++i)
                putByte(app, buffer + i, byte(app, local + i));
            result = buffer;
        }
    }
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
}

/* _getdiskfree (clib/direct/c/diskfree.c, 0x4f07d0): GetDiskFreeSpaceA's of
 * drive eax -- 1 A:, 0 the current one -- into edx's diskfree_t: total and
 * free clusters, sectors a cluster, bytes a sector; 0, or -1. */
void getdiskfree(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 drive = cpu.eax, info = cpu.edx;
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
    if (!call(app, cpu, at(app, kGetDiskFreeSpaceA), {root, local + 0x10, local + 0xc, local + 8, local + 4})
        || !windowsResult(app, cpu, cpu.eax, result))
        return;
    if (result == 0)
    {
        put(app, info, at(app, local + 4));
        put(app, info + 4, at(app, local + 8));
        put(app, info + 8, at(app, local + 0x10));
        put(app, info + 0xc, at(app, local + 0xc));
    }
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
}

/* __fixed_GetFileAttributesA (0x509640): GetFileAttributesA by way of
 * FindFirstFileA, which wildcards would fool: -1 for them. */
void fixed_GetFileAttributesA(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 attributes;
    if (!fileAttributes(app, cpu, entry, cpu.eax, attributes))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, attributes);
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

/* The file's lock taken or given back. */
bool fileLock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 handle)
{
    cpu.eax = handle;
    return call(app, cpu, at(app, slot));
}

/* The default windowing's window for the handle, if it has one and `hook`
 * is set: _WindowsIsWindowedHandle's. */
bool windowOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 hook, x86::reg32 handle, x86::reg32& window)
{
    window = 0;
    if (at(app, hook) == 0)
        return true;
    cpu.eax = handle;
    if (!call(app, cpu, at(app, kIsWindowedHandle)))
        return false;
    window = cpu.eax;
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
        if (!call(app, cpu, at(app, kGetFileType), {osHandle(app, handle)}))
            return false;
        result = cpu.eax == 2 ? 1 : 0;  // FILE_TYPE_CHAR
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
void GetIOMode(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 mode;
    if (!ioMode(app, cpu, cpu.eax, mode))
        return;
    kept.back(cpu);
    returns(cpu, mode);
}

/* __SetIOMode (clib/handleio/c/iomode.c, 0x522e38): eax's mode edx; eax
 * left 4 times the handle, as its code leaves it. */
void SetIOMode(win32::WinApplication* app, x86::CPU& cpu)
{
    setIoMode(app, cpu.eax, cpu.edx);
    returns(cpu, cpu.eax << 2);
}

/* isatty (clib/environ/c/isattwnt.c, 0x523560): whether eax is a
 * character device's, or a window's. */
void isatty(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!isTty(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* lseek (clib/handleio/c/lseek.c, 0x5028e0): SetFilePointer on eax, edx bytes
 * from where ebx says; a move forward marks the mode extended unless it is
 * appending.  The new place, or -1. */
void lseek(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    const x86::reg32 handle = cpu.eax, offset = cpu.edx, whence = cpu.ebx;
    x86::reg32 result;
    if (!goodHandle(app, handle))
    {
        if (!badHandle(app, cpu, result))
            return;
    }
    else
    {
        x86::reg32 mode;
        if (!fileLock(app, cpu, kAccessFile, handle) || !ioMode(app, cpu, handle, mode))
            return;
        if (x86::sreg32(offset) > 0 && !(mode & kAppend))
            setIoMode(app, handle, mode | kExtended);
        if (!call(app, cpu, at(app, kSetFilePointer), {osHandle(app, handle), offset, 0, whence}))
            return;
        result = cpu.eax;
        if (!fileLock(app, cpu, kReleaseFile, handle) || (result == 0xffffffff && !setErrnoNt(app, cpu)))
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

/* _tell (clib/handleio/c/tell.c, 0x502960): eax's place, or -1. */
void tell(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    const x86::reg32 handle = cpu.eax;
    x86::reg32 result;
    if (!goodHandle(app, handle))
    {
        if (!badHandle(app, cpu, result))
            return;
    }
    else
    {
        const x86::reg32 os = osHandle(app, handle);  // before the lock, as its code reads it
        if (!fileLock(app, cpu, kAccessFile, handle) || !call(app, cpu, at(app, kSetFilePointer), {os, 0, 0, 1}))
            return;
        result = cpu.eax;
        if (!fileLock(app, cpu, kReleaseFile, handle) || (result == 0xffffffff && !setErrnoNt(app, cpu)))
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

namespace
{

/* __qread's and __qwrite's way to a window's: the hook on it, eax the window,
 * edx the buffer, ebx the length; how many it took. */
bool windowTransfer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 hook, x86::reg32 handle,
                    x86::reg32 window, x86::reg32 buffer, x86::reg32 length, x86::reg32& result)
{
    cpu.eax = window;
    cpu.edx = buffer;
    cpu.ebx = length;
    if (!call(app, cpu, at(app, hook)))
        return false;
    result = cpu.eax;
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
    if (!call(app, cpu, at(app, kReadFile), {osHandle(app, handle), buffer, length, local, 0}))
        return false;
    const bool done = cpu.eax != 0;
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
        if (!call(app, cpu, at(app, kSetFilePointer), {os, 0, 0, 2}))
            return false;
        if (cpu.eax == 0xffffffff)
        {
            result = 0xffffffff;
            return fileLock(app, cpu, kReleaseFile, handle) && setErrnoNt(app, cpu);
        }
    }
    if (!windowOf(app, cpu, kWindowsStdout, handle, window))
        return false;
    if (window != 0)
        return windowTransfer(app, cpu, kWindowsStdout, handle, window, buffer, length, result);
    if (!call(app, cpu, at(app, kWriteFile), {os, buffer, length, local, 0}))
        return false;
    if (cpu.eax == 0)
    {
        result = 0xffffffff;
        return fileLock(app, cpu, kReleaseFile, handle) && setErrnoNt(app, cpu);
    }
    result = at(app, local);
    return (result == length || setThreadWord(app, cpu, kErrno, kENOSPC))
           && fileLock(app, cpu, kReleaseFile, handle);
}

}  // namespace

/* __qread (clib/handleio/c/qread.c, 0x502a50): ebx bytes from eax into edx --
 * a window's by _WindowsStdin, else by ReadFile; how many, or -1. */
void qread(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    cpu.esp = entry - 0xc;  // ecx, esi; how many were read
    x86::reg32 result;
    if (!readHandle(app, cpu, cpu.eax, cpu.edx, cpu.ebx, entry - 0xc, result))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
}

/* __qwrite (clib/handleio/c/qwrit.c, 0x5096c0): ebx bytes from edx to eax --
 * at the end, if it is appending; a window's by _WindowsStdout, else by
 * WriteFile, errno ENOSPC for fewer.  How many, or -1. */
void qwrite(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    cpu.esp = entry - 0x14;  // ecx, esi, edi, ebp; how many were written
    x86::reg32 result;
    if (!writeHandle(app, cpu, cpu.eax, cpu.edx, cpu.ebx, entry - 0x14, result))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
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
    if (!call(app, cpu, kTzset))
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
    if (!call(app, cpu, at(app, kGetLocalTime), {local}))
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
    if (!call(app, cpu, kTzset))
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
void leapyear(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    returns(cpu, leapYear(cpu.eax) ? 1 : 0);
}

/* __brktime (clib/time/c/gmtime.c, 0x515fa0): edx seconds ebx west into day
 * eax since 1900 broken down into the struct tm at ecx; that. */
void brktime(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, breakTime(app, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx));
}

/* time_less (clib/time/c/timeutil.c, 0x51652c): whether the struct tm at
 * eax's time of day is before edx's. */
void time_less(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, timeLess(app, cpu.eax, cpu.edx) ? 1 : 0);
}

/* calc_yday (clib/time/c/timeutil.c, 0x51612c): the day of year edx the rule
 * at eax falls on. */
void calc_yday(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::sreg32 day;
    if (!ruleDay(app, cpu, entry, cpu.eax, cpu.edx, day))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, day);
}

/* check_order (clib/time/c/timeutil.c, 0x516224): whether the rule at eax
 * falls after edx's in year ebx. */
void check_order(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    bool reversed;
    if (!rulesReversed(app, cpu, entry, cpu.eax, cpu.edx, cpu.ebx, reversed))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, reversed ? 1 : 0);
}

/* __isindst (clib/time/c/timeutil.c, 0x516270): the struct tm at eax's isdst
 * by the zone's rules; that. */
void isindst(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 dst;
    if (!inDst(app, cpu, entry, cpu.eax, dst))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, dst);
}

/* mktime (clib/time/c/mktime.c, 0x50eba0): the struct tm at eax made a
 * time_t, and put right. */
void mktime(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 result;
    if (!makeTime(app, cpu, entry, cpu.eax, result))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
}

/* __getctime (clib/time/c/gtctmwnt.c, 0x50eb30): the local time into the
 * struct tm at eax; its milliseconds. */
void getctime(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    x86::reg32 milliseconds;
    if (!currentTime(app, cpu, entry, cpu.eax, milliseconds))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, milliseconds);
}

/* time (clib/time/c/time.c, 0x4f22b0): now, a second on from half of one,
 * as a time_t -- into eax's too, if there is one. */
void time(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 timer = cpu.eax;
    const x86::reg32 tm = entry - 0x2c;  // ebx, edx; the struct tm
    x86::reg32 milliseconds, result;
    if (!currentTime(app, cpu, tm - 4, tm, milliseconds))
        return;
    if (x86::sreg32(milliseconds) >= 500)
        put(app, tm + kSec, at(app, tm + kSec) + 1);
    if (!makeTime(app, cpu, tm - 4, tm, result))
        return;
    if (timer != 0)
        put(app, timer, result);
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, result);
}

/* _localtime (clib/time/c/localtim.c, 0x4f8040): *eax broken down into the
 * struct tm at edx; that. */
void localtime_(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 tm = cpu.edx;
    if (!localTime(app, cpu, entry, cpu.eax, tm))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, tm);
}

/* localtime (clib/time/c/localtim.c, 0x4f8094): *eax broken down into the
 * thread's struct tm; that. */
void localtime(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const Kept kept(cpu);
    const x86::reg32 timer = cpu.eax;
    x86::reg32 data;
    cpu.esp = entry - 8;  // ebx, edx
    if (!threadData(app, cpu, data))
        return;
    const x86::reg32 tm = data + 0x14;
    if (!localTime(app, cpu, entry - 0xc, timer, tm))
        return;
    kept.back(cpu);
    cpu.esp = entry;
    returns(cpu, tm);
}

/* ---- The x87's maths (mathlib): each of its instructions the same
 * operation of the port's x87 as the generated code has for it, in the same
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

void pushFloat(x86::CPU& cpu, const x86::Float& value)
{
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = value;
}

void setAh(x86::CPU& cpu, x86::reg16 status)
{
    cpu.eax = (cpu.eax & 0xffff00ff) | (status & 0xff00);
}

/* modf's: the double at `argument` into its integer part, at `whole`, and
 * its fraction on the stack. */
void splitDouble(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 argument, x86::reg32 whole)
{
    const x86::Float value = loadDouble(app, argument);
    cpu.fpu.count += 2;
    cpu.fpu.st(0) = value;
    cpu.fpu.st(1) = value;
    chop(cpu);
    const x86::Float fraction = cpu.fpu.sub(cpu.fpu.st(1), cpu.fpu.st(0));
    storeDouble(app, whole, cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    cpu.fpu.st(0) = fraction;
}

/* floor's: the double at `argument` rounded down, by modf, its integer part
 * at `whole`. */
void roundDown(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 argument, x86::reg32 whole)
{
    splitDouble(app, cpu, argument, whole);
    cpu.fpu.compare(0.0, cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    if (!(cpu.fpu.status.word & (kC0 | kC3)))
        storeDouble(app, whole, cpu.fpu.add(loadDouble(app, whole), loadDouble(app, kMinusOne)));
    pushFloat(cpu, loadDouble(app, whole));
}

bool mathError(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry);

/* __@DSQRT's: st(0)'s root; a negative one, or none, __math87_err's, al the
 * caller's function -- st(1) dropped too unless it is sqrt's (3).  al 0, or
 * 1. */
bool squareRoot(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry)
{
    put(app, entry - 4, cpu.ebp);
    cpu.fpu.compare(cpu.fpu.st(0), 0.0);
    const x86::reg16 status = cpu.fpu.status.word;
    const x86::reg32 slot = entry - 0xc;  // fnstsw's word, which xchg gives ax for
    app->getMemory<x86::reg16>(slot) = x86::reg16(cpu.eax);
    if (!(status & kC0))
    {
        cpu.fpu.st(0) = cpu.fpu.sqrt(cpu.fpu.st(0));
        cpu.eax = (cpu.eax & 0xffff0000) | (status & 0xff00);
        return true;
    }
    const x86::reg32 argument = entry - 0x14;
    storeDouble(app, argument, cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    cpu.eax = at(app, slot);
    if (x86::reg8(cpu.eax) != 3)
    {
        storeDouble(app, argument, cpu.fpu.st(0));
        cpu.fpu.count -= 1;
    }
    put(app, entry - 8, cpu.edx);
    cpu.esp = argument;
    if (!mathError(app, cpu, argument - 4))
        return false;
    cpu.eax = (cpu.eax & 0xffffff00) | 1;
    return true;
}

/* IF@DATAN2's: atan2 of st(1) by st(0), fpatan's. */
bool arcTangent(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry)
{
    NFS2_USE(app);
    NFS2_USE(entry);
    std::swap(cpu.fpu.st(0), cpu.fpu.st(1));
    const x86::Float angle = cpu.fpu.atan(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.fpu.count -= 1;
    cpu.fpu.st(0) = angle;
    return true;
}

/* __math87_err's: the double at entry + 4 and the function at entry + 0xc,
 * its byte, made __reporterror's flags -- or, for exp's underflow, 0 -- and
 * what it gives back pushed. */
bool mathError(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry)
{
    const x86::reg32 argument = entry + 4;
    const x86::reg32 result = entry - 0x1c;  // ebp, eax, ebx, ecx, edx; a double
    const x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
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
            cpu.fpu.compare(0.0, loadDouble(app, argument));
            underflow = !(cpu.fpu.status.word & (kC0 | kC3));
        }
        if (!underflow)
        {
            flags = 0x8100 | function;
            if (function == 6)
            {
                cpu.fpu.compare(0.0, loadDouble(app, argument));
                if (!(cpu.fpu.status.word & (kC0 | kC3)))
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
        cpu.eax = flags;
        cpu.edx = argument;
        cpu.esp = result;
        if (!call(app, cpu, kReportError))
            return false;
        storeDouble(app, result, cpu.fpu.st(0));
        cpu.fpu.count -= 1;
    }
    pushFloat(cpu, loadDouble(app, result));
    cpu.eax = eax;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp = entry + 0x10;
    return true;
}

/* IF@DEXP's: e to st(0), al 4, exp's: by fldl2e, frndint, f2xm1 and fscale;
 * past the largest double __math87_err's, below the smallest 0.  al 0, or 1;
 * ah fnstsw's last. */
bool exponential(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entry)
{
    cpu.eax = (cpu.eax & 0xffffff00) | 4;
    put(app, entry - 4, cpu.ebp);
    cpu.fpu.compare(cpu.fpu.st(0), loadDouble(app, kExpMax));
    setAh(cpu, cpu.fpu.status.word);
    if (!(cpu.fpu.status.word & (kC0 | kC3)))
    {
        const x86::reg32 argument = entry - 0x14;
        storeDouble(app, argument, cpu.fpu.st(0));
        put(app, entry - 0xc, cpu.eax);
        put(app, entry - 8, cpu.edx);
        cpu.fpu.count -= 1;
        cpu.esp = argument;
        if (!mathError(app, cpu, argument - 4))
            return false;
        cpu.eax = (cpu.eax & 0xffffff00) | 1;
        return true;
    }
    cpu.fpu.compare(cpu.fpu.st(0), loadDouble(app, kExpMin));
    setAh(cpu, cpu.fpu.status.word);
    if (cpu.fpu.status.word & (kC0 | kC3))
    {
        pushFloat(cpu, 0.0);
    }
    else
    {
        const x86::Float scaled = cpu.fpu.mul(cpu.fpu.st(0), kLog2e);
        const x86::Float whole = cpu.fpu.rndint(scaled);
        const x86::Float fraction = cpu.fpu.sub(scaled, whole);
        const x86::Float power = cpu.fpu.add(cpu.fpu.f2xm1(fraction), 1.0);
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = cpu.fpu.scale(power, whole);
        cpu.fpu.st(1) = whole;
    }
    const x86::Float result = cpu.fpu.st(0);
    cpu.fpu.count -= 1;
    cpu.fpu.st(0) = result;
    cpu.eax &= 0xffffff00;
    return true;
}

}  // namespace

/* IF@DATAN2 (mathlib/a/atan87.asm, 0x4e06d9): atan2 of st(1) by st(0), into
 * st(0). */
void IF_DATAN2(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    if (!arcTangent(app, cpu, entry))
        return;
    cpu.esp = entry + 4;
}

/* floor (mathlib/c/floor.c, 0x4f1c30): the double on the stack rounded
 * down, onto the x87's; stdcall. */
void floor(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    roundDown(app, cpu, entry + 4, entry - 0x18);  // ebp, eax, ebx, edx; the integer part
    cpu.esp = entry + 0xc;
}

/* ceil (mathlib/c/ceil.c, 0x4f2d78): -floor(-x). */
void ceil(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 negated = entry - 0x10;  // ebp, eax; floor's argument
    storeDouble(app, negated, -loadDouble(app, entry + 4));
    roundDown(app, cpu, negated, negated - 4 - 0x18);
    cpu.fpu.st(0) = -cpu.fpu.st(0);
    cpu.esp = entry + 0xc;
}

/* modf (0x50ea4e): the double on the stack into its integer part, at the
 * pointer after it, and its fraction, onto the x87's; stdcall. */
void modf(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    splitDouble(app, cpu, entry + 4, at(app, entry + 0xc));
    cpu.esp = entry + 0x10;
}

/* _control87 (clib/fpu/c/cntrl87.c, 0x4f7c90): the control word's bits of
 * edx made eax's, if there is an x87 (_8087); the control word, or 0. */
void control87(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 word = 0;
    if (byte(app, k8087) != 0)
    {
        word = cpu.fpu.control.word;
        const x86::reg32 mask = cpu.edx;
        if (mask != 0)
        {
            word = (~mask & word) | (mask & cpu.eax);
            cpu.fpu.setControl(x86::reg16(word));
            word = (word & 0xffff0000) | cpu.fpu.control.word;
        }
    }
    returns(cpu, word & 0xffff);
}

/* IF@DEXP (0x4f80b4): e to st(0), into st(0). */
void IF_DEXP(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    if (!exponential(app, cpu, entry))
        return;
    cpu.esp = entry + 4;
}

/* exp (mathlib/a/exp87.asm, 0x4f8116): e to the double on the stack, onto
 * the x87's; stdcall. */
void exp(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    pushFloat(cpu, loadDouble(app, entry + 4));
    if (!exponential(app, cpu, entry - 4))
        return;
    cpu.esp = entry + 0xc;
}

/* IF@DACOS (mathlib/a/acos87.asm, 0x4f9482): acos of st(0) -- atan2 of
 * sqrt(1 - x*x) by x, from pi/2; 0 or pi for 1 and -1 -- into st(0). */
void IF_DACOS(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved = entry - 4;  // push ax: the port's code takes a dword for it
    app->getMemory<x86::reg16>(saved) = x86::reg16(cpu.eax);
    cpu.esp = saved;
    const x86::Float x = cpu.fpu.st(0);
    const x86::Float rest = cpu.fpu.sub(1.0, cpu.fpu.mul(x, x));
    cpu.fpu.compare(rest, 0.0);
    cpu.eax = (cpu.eax & 0xffff0000) | cpu.fpu.status.word;
    pushFloat(cpu, rest);
    if (cpu.fpu.status.word & kC3)
    {
        cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
        cpu.eax = (cpu.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.fpu.count -= 2;
        pushFloat(cpu, cpu.fpu.status.word & (kC0 | kC3) ? x86::Float(0.0) : x86::Float(kPi));
    }
    else
    {
        cpu.eax = (cpu.eax & 0xffffff00) | 1;
        if (!squareRoot(app, cpu, saved - 4))
            return;
        cpu.esp = saved;
        if (x86::reg8(cpu.eax) == 0)
        {
            std::swap(cpu.fpu.st(0), cpu.fpu.st(1));
            if (!arcTangent(app, cpu, saved - 4))
                return;
            cpu.esp = saved;
            const x86::Float halfPi = x86::Float(app->getMemory<x86::IEEEf80>(kHalfPi));
            cpu.fpu.st(0) = cpu.fpu.sub(halfPi, cpu.fpu.st(0));
        }
    }
    cpu.eax = (cpu.eax & 0xffff0000) | app->getMemory<x86::reg16>(saved);
    cpu.esp = entry + 4;
}

/* __@DSQRT (mathlib, 0x51a1ba): st(0)'s root, al the caller's function; al
 * 0, or 1 for a math error. */
void DSQRT(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 entry = cpu.esp;
    if (!squareRoot(app, cpu, entry))
        return;
    cpu.esp = entry + 4;
}

/* __math87_err (0x51655c): a math error -- the argument, a double, and the
 * function, a byte of the dword after it, on the stack -- reported; what
 * _matherr or the runtime gives back onto the x87's.  Takes its arguments
 * off. */
void math87_err(win32::WinApplication* app, x86::CPU& cpu)
{
    mathError(app, cpu, cpu.esp);
}

/* __set_EDOM (0x502890): errno EDOM. */
void set_EDOM(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!setThreadWord(app, cpu, kErrno, kEDOM))
        return;
    const x86::reg32 data = cpu.eax;
    kept.back(cpu);
    returns(cpu, data);
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
    if (!fileLock(app, cpu, kAccessFile, handle)
        || !call(app, cpu, at(app, kFlushFileBuffers), {osHandle(app, handle)}))
        return false;
    if (cpu.eax == 0)
    {
        result = 0xffffffff;
        if (!setErrnoNt(app, cpu))
            return false;
    }
    return fileLock(app, cpu, kReleaseFile, handle);
}

/* __chktty's: a tty's stream marked one, and line-buffered unless it is
 * buffered some way already; eax as its code leaves it. */
bool checkTty(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    cpu.eax = fp;
    if (flags(app, fp) & kTty)
        return true;
    cpu.eax = at(app, fp + kFileHandle);
    if (!call(app, cpu, isatty))
        return false;
    if (cpu.eax != 0)
    {
        setFlags(app, fp, kTty);
        if (!(flags(app, fp) & kBuffering))
            setFlags(app, fp, kLineBuf);
    }
    return true;
}

/* __ioalloc's: a buffer from _nmalloc -- _bufsize's, or 0x86 bytes
 * line-buffered, 1 unbuffered, 4K -- or, for none, _ungotten, unbuffered. */
bool allocateBuffer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fp)
{
    const Frame frame(cpu, 0xc);  // ebx, ecx, edx
    if (!nested(cpu, [&] { return checkTty(app, cpu, fp); }))
        return false;
    if (at(app, fp + kBufsize) == 0)
    {
        const x86::reg32 f = flags(app, fp);
        put(app, fp + kBufsize, f & kLineBuf ? 0x86 : f & kNoBuf ? 1 : 0x1000);
    }
    cpu.eax = at(app, fp + kBufsize);
    if (!call(app, cpu, nmalloc))
        return false;
    put(app, linkOf(app, fp) + kLinkBase, cpu.eax);
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
    cpu.eax = emptyBuffer(app, fp);
    return true;
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
                cpu.eax = at(app, fp + kFileHandle);
                cpu.edx = from;
                cpu.ebx = left;
                if (!call(app, cpu, qwrite))
                    return false;
                const x86::reg32 written = cpu.eax;
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
                cpu.eax = at(app, fp + kFileHandle);
                cpu.edx = 0 - at(app, fp + kCnt);
                cpu.ebx = 1;  // SEEK_CUR
                if (!call(app, cpu, lseek))
                    return false;
                place = cpu.eax;
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
    cpu.eax = mask;
    if (!call(app, cpu, at(app, kAccessIob)))
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
    cpu.eax = eax;
    return call(app, cpu, at(app, kReleaseIob));
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
                cpu.eax = at(app, fp + kFileHandle);
                cpu.edx = buffer;
                cpu.ebx = direct;
                if (!call(app, cpu, qwrite))
                    return false;
                step = cpu.eax;
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
        if (!call(app, cpu, kGetche))
            return false;
        if (cpu.eax != 0xffffffff)
        {
            putByte(app, at(app, fp + kPtr), x86::reg8(cpu.eax));
            put(app, fp + kCnt, 1);
        }
    }
    else
    {
        cpu.ebx = flags(app, fp) & kNoBuf ? 1 : at(app, fp + kBufsize);
        cpu.edx = at(app, fp + kPtr);
        cpu.eax = at(app, fp + kFileHandle);
        if (!call(app, cpu, qread))
            return false;
        put(app, fp + kCnt, cpu.eax);
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
                cpu.ebx = asked;
                cpu.edx = buffer;
                cpu.eax = at(app, fp + kFileHandle);
                if (!call(app, cpu, qread))
                    return false;
                if (cpu.eax == 0xffffffff)
                {
                    setFlags(app, fp, kError);
                    break;
                }
                if (cpu.eax == 0)
                {
                    setFlags(app, fp, kAtEof);
                    break;
                }
                buffer += cpu.eax;
                left -= cpu.eax;
                read += cpu.eax;
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
        cpu.ebx = whence;
        cpu.edx = to;
        cpu.eax = at(app, fp + kFileHandle);
        if (!call(app, cpu, lseek))
            return false;
        if (cpu.eax == 0xffffffff)
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
        cpu.eax = at(app, fp + kFileHandle);
        if (!call(app, cpu, tell))
            return false;
        if (updateBuffer(app, offset - (cpu.eax - at(app, fp + kCnt)), fp) == 0)
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
    cpu.eax = at(app, fp + kFileHandle);
    if (!call(app, cpu, tell))
        return false;
    result = cpu.eax;
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
    if (!call(app, cpu, at(app, kCreateEventA), {0, 0, 0, 0}))
        return false;
    handle = cpu.eax;
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
    cpu.eax = os;
    if (!call(app, cpu, at(app, kAccessHandles)))
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
                cpu.eax = offset;
                return call(app, cpu, at(app, kReleaseHandles));
            }
            offset += 4;
            ++handle;
        } while (x86::sreg32(offset) < x86::sreg32(count << 2));
    }
    cpu.edx = (at(app, kHandleCount) + 1) << 2;
    cpu.eax = at(app, kOsHandles);
    if (!call(app, cpu, kRealloc))
        return false;
    const x86::reg32 last = at(app, kHandleCount);
    put(app, kOsHandles, cpu.eax);
    put(app, cpu.eax + 4 * last, os);
    put(app, kHandleCount, last + 1);
    if (!call(app, cpu, at(app, kReleaseHandles)))
        return false;
    handle = at(app, kHandleCount) - 1;
    return true;
}

/* __freePOSIXHandle's: a handle's slot free again, but 0's, under the
 * handles' lock; eax as its code leaves it. */
bool freeHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
{
    const Frame frame(cpu, 4);  // edx
    cpu.eax = handle;
    if (!call(app, cpu, at(app, kAccessHandles)))
        return false;
    if (x86::sreg32(handle) > 0 && x86::sreg32(handle) < x86::sreg32(at(app, kHandleCount)))
    {
        cpu.eax = handle << 2;
        put(app, at(app, kOsHandles) + cpu.eax, 0);
    }
    return call(app, cpu, at(app, kReleaseHandles));
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
        cpu.eax = handle;
        if (!call(app, cpu, at(app, kIsWindowedHandle)))
            return false;
        window = cpu.eax;
        if (window != 0)
        {
            cpu.eax = handle;
            if (!call(app, cpu, at(app, kRemoveWindowedHandle)))
                return false;
            cpu.eax = window;
            if (!call(app, cpu, at(app, kDestroyWindow)))
                return false;
        }
    }
    if (window == 0)
    {
        if (!call(app, cpu, at(app, kCloseHandle), {os}))
            return false;
        if (cpu.eax == 0)
        {
            result = 0xffffffff;
            return setThreadWord(app, cpu, kErrno, kEBADF);
        }
    }
    cpu.eax = handle;
    cpu.edx = 0;
    return call(app, cpu, SetIOMode);
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
    if (at(app, kNewWindow) != 0)
    {
        cpu.edx = kConsole;
        cpu.eax = name;
        if (!call(app, cpu, stricmp))
            return false;
        if (cpu.eax == 0)
        {
            x86::reg32 fake;
            if (!nested(cpu, [&] { return fakeHandle(app, cpu, fake); }))
                return false;
            cpu.eax = fake;
            if (!call(app, cpu, at(app, kAllocHandle)))
                return false;
            cpu.ebx = 0;
            cpu.edx = 0x2000;
            cpu.ecx = cpu.eax;
            if (!call(app, cpu, at(app, kNewWindow), {0, cpu.eax, 0xffffffff}))
                return false;
            handle = cpu.ecx;
            mode = cpu.edx;
            window = true;
        }
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
        if (!call(app, cpu, at(app, kCreateFileA),
                  {name, at(app, access), at(app, share), security, disposition, fileAttributes, 0}))
            return false;
        x86::reg32 os = cpu.eax;
        if (os == 0xffffffff && (byte(app, flags) & kOpenCreate))
        {
            if (!call(app, cpu, at(app, kCreateFileA),
                      {name, at(app, access), at(app, share), 0, at(app, retry), fileAttributes, 0}))
                return false;
            os = cpu.eax;
        }
        if (os == 0xffffffff)
            return setErrnoNt(app, cpu);
        cpu.eax = os;
        if (!call(app, cpu, at(app, kAllocHandle)))
            return false;
        handle = cpu.eax;
        if (handle >= at(app, kFileCount))
        {
            if (!call(app, cpu, at(app, kCloseHandle), {os}))
                return false;
            return setThreadWord(app, cpu, kErrno, kENOMEM);
        }
        cpu.eax = handle;
        if (!call(app, cpu, isatty))
            return false;
        if (cpu.eax != 0)
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
    cpu.eax = handle;
    cpu.edx = mode;
    if (!call(app, cpu, SetIOMode))
        return false;
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
    cpu.eax = first;
    if (!call(app, cpu, tolower))
        return false;
    x86::reg32 oflag = flags & kBinary ? kOpenBinary : kOpenText, permission = 0;
    if (x86::reg8(cpu.eax) == 'r')
    {
        oflag |= flags & kWriting ? 2 : 0;
    }
    else
    {
        oflag |= (flags & kReading ? 2 : 1) | kOpenCreate | (flags & kAppend ? kOpenAppend : kOpenTruncate);
        permission = 0x180;
    }
    if (!call(app, cpu, sopen, {name, oflag, shflag, permission}))
        return false;
    put(app, fp + kFileHandle, cpu.eax);
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
        cpu.ebx = 2;
        cpu.edx = 0;
        if (!nested(cpu, [&] { return seekStream(app, cpu, fp, 0, 2, ignored); }))
            return false;
    }
    if (!nested(cpu, [&] { return checkTty(app, cpu, fp); }))
        return false;
    result = fp;
    return true;
}

/* __allocfp's (clib/streamio/c/allocfp.c): a stream for fopen, under the
 * streams' lock -- the last closed's again; or one of __iob's 20 not in use,
 * a link from the heap for it; or a FILE and its link from the heap -- its
 * FILE cleared, in use, its link first on __OpenStreams; 0 and ENOMEM for
 * none.  The lock is given back with eax as its code leaves it. */
bool allocateStream(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& fp)
{
    const Frame frame(cpu, 0x14);  // ebx, ecx, edx, esi, edi
    if (!call(app, cpu, at(app, kAccessIob)))
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
        cpu.eax = fp != 0 ? kLinkSize : kLinkSize + kFileSize;
        if (!call(app, cpu, nmalloc))
            return false;
        link = cpu.eax;
        flag = fp != 0 ? kReading | kWriting : kDynamic | kReading | kWriting;
        if (link == 0)
        {
            fp = 0;
            return setThreadWord(app, cpu, kErrno, kENOMEM) && call(app, cpu, at(app, kReleaseIob));
        }
        if (fp == 0)
            fp = link + kLinkSize;
    }
    cpu.ebx = kFileSize;
    cpu.eax = fp;
    cpu.edx = 0;
    if (!call(app, cpu, memset))
        return false;
    put(app, fp + kFlag, flag);
    put(app, link + kLinkStream, fp);
    const x86::reg32 head = at(app, kOpenStreams);
    put(app, fp + kLink, link);
    put(app, kOpenStreams, link);
    put(app, link + kLinkNext, head);
    cpu.eax = head;
    return call(app, cpu, at(app, kReleaseIob));
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
        cpu.edx = place;
        cpu.ebx = 0;
        cpu.eax = at(app, fp + kFileHandle);
        if (!call(app, cpu, lseek))
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
        cpu.eax = bufferOf(app, fp);
        if (!call(app, cpu, nfree))
            return false;
        put(app, linkOf(app, fp) + kLinkBase, 0);
    }
    if (flags(app, fp) & kTmpFile)
    {
        cpu.edx = byte(app, linkOf(app, fp) + kTmpfChar);
        cpu.eax = path;
        if (!call(app, cpu, kMkTmpFile))
            return false;
        cpu.eax = path;
        if (!call(app, cpu, remove))
            return false;
    }
    if (!streamLock(app, cpu, kReleaseFile, fp))
        return false;
    cpu.eax = at(app, fp + kFileHandle);
    return handleToo == 0 || call(app, cpu, at(app, kFreeHandle));
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
    cpu.eax = fp;
    if (!call(app, cpu, at(app, kAccessIob)))
        return false;
    x86::reg32 link = at(app, kOpenStreams);
    while (link != 0 && at(app, link + kLinkStream) != fp)
        link = at(app, link + kLinkNext);
    cpu.eax = link;
    if (!call(app, cpu, at(app, kReleaseIob)))
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
    cpu.eax = 0;
    if (!nested(cpu, [&] { return allocateStream(app, cpu, fp); }))
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
void fsync(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!syncHandle(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __chktty (clib/streamio/c/chktty.c, 0x509380): the stream at eax marked a
 * tty's, if it is one. */
void chktty(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!checkTty(app, cpu, cpu.eax))
        return;
    const x86::reg32 eax = cpu.eax;
    kept.back(cpu);
    returns(cpu, eax);
}

/* __ioalloc (clib/streamio/c/ioalloc.c, 0x5029c0): a buffer for the stream at
 * eax. */
void ioalloc(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!allocateBuffer(app, cpu, cpu.eax))
        return;
    const x86::reg32 eax = cpu.eax;
    kept.back(cpu);
    returns(cpu, eax);
}

/* __flush (clib/streamio/c/flush.c, 0x502760): the stream at eax's buffer
 * written, or given back; 0, or -1. */
void flush(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!flushStream(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fputc (clib/streamio/c/fputc.c, 0x5097b0): eax's character to the stream
 * at edx. */
void fputc(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!putChar(app, cpu, cpu.eax, cpu.edx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fputs (clib/streamio/c/fputs.c, 0x4f7d80): the string at eax to the stream
 * at edx. */
void fputs(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!putString(app, cpu, cpu.eax, cpu.edx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fwrite (clib/streamio/c/fwrite.c, 0x4eebd0): ebx of edx bytes from eax to
 * the stream at ecx. */
void fwrite(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!writeStream(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __flushall (clib/streamio/c/flushall.c, 0x515858): the open streams with
 * a bit of eax flushed; how many. */
void flushall_(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 count;
    if (!flushAll(app, cpu, cpu.eax, count))
        return;
    kept.back(cpu);
    returns(cpu, count);
}

/* flushall (clib/streamio/c/flushall.c, 0x515850): all of them. */
void flushall(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 count;
    if (!flushAll(app, cpu, 0xffffffff, count))
        return;
    kept.back(cpu);
    returns(cpu, count);
}

/* fflush (clib/streamio/c/fflush.c, 0x4f7d70): the stream at eax flushed --
 * or, for none, all of them, and 0. */
void fflush(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.eax != 0)
        return flush(app, cpu);
    const Kept kept(cpu);
    x86::reg32 count;
    if (!nested(cpu, [&] { return flushAll(app, cpu, 0xffffffff, count); }))
        return;
    kept.back(cpu);
    returns(cpu, 0);
}

/* __update_buffer (clib/streamio/c/fseek.c, 0x4e9030): a seek of eax from
 * where the stream at edx is, within its buffer: 0, or 1 for one outside. */
void update_buffer(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, updateBuffer(app, cpu.eax, cpu.edx));
}

/* __reset_buffer (clib/streamio/c/fseek.c, 0x4e9070): the stream at eax's
 * buffer emptied; eax left the stream. */
void reset_buffer(win32::WinApplication* app, x86::CPU& cpu)
{
    resetBuffer(app, cpu.eax);
    returns(cpu, cpu.eax);
}

/* __fill_buffer (clib/streamio/c/fgetc.c, 0x502be8): the stream at eax's
 * buffer filled; how many are in it. */
void fill_buffer(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 count;
    if (!fillBuffer(app, cpu, cpu.eax, count))
        return;
    kept.back(cpu);
    returns(cpu, count);
}

/* __filbuf (clib/streamio/c/fgetc.c, 0x502bb8): the stream at eax's buffer
 * filled, and its first byte; -1 for none. */
void filbuf(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 c;
    if (!fillAndTake(app, cpu, cpu.eax, c))
        return;
    kept.back(cpu);
    returns(cpu, c);
}

/* fgetc (clib/streamio/c/fgetc.c, 0x502af0): a byte of the stream at eax,
 * or -1. */
void fgetc(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!getChar(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fread (clib/streamio/c/fread.c, 0x4e9200): ebx of edx bytes into eax from
 * the stream at ecx. */
void fread(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!readStream(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fgets (clib/streamio/c/fgets.c, 0x4f0570): a line of the stream at ebx
 * into eax, edx bytes at most with its 0; eax, or 0. */
void fgets(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!getString(app, cpu, cpu.eax, cpu.edx, cpu.ebx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fseek (clib/streamio/c/fseek.c, 0x4e9088): the stream at eax to edx, from
 * ebx's whence; 0, or -1. */
void fseek(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!seekStream(app, cpu, cpu.eax, cpu.edx, cpu.ebx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* ftell (clib/streamio/c/ftell.c, 0x4ee420): where the stream at eax is, or
 * -1. */
void ftell(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!tellStream(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __GetNTAccessAttr (clib/streamio/c/ntfio.c, 0x5234d0): CreateFileA's
 * access to edx's dword and attributes to ebx's for eax's read and write
 * mode. */
void GetNTAccessAttr(win32::WinApplication* app, x86::CPU& cpu)
{
    accessAttributes(app, cpu.eax, cpu.edx, cpu.ebx);
    returns(cpu, cpu.eax);
}

/* __GetNTShareAttr (clib/streamio/c/ntfio.c, 0x523504): CreateFileA's
 * sharing to edx's dword for eax's sharing and mode; eax left the sharing
 * bits. */
void GetNTShareAttr(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, shareAttributes(app, cpu.eax, cpu.edx));
}

/* 0x51e2b0, _sopen's: whether __OSHandles has no slot free. */
void handlesFull(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, handlesFull(app));
}

/* __NTGetFakeHandle (clib/handleio/c/hdlman.c, 0x51e4f0): a HANDLE for a
 * handle with none. */
void NTGetFakeHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 handle;
    if (!fakeHandle(app, cpu, handle))
        return;
    kept.back(cpu);
    returns(cpu, handle);
}

/* __allocPOSIXHandle (clib/handleio/c/hdlman.c, 0x51e304): a handle for the
 * HANDLE at eax. */
void allocPOSIXHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 handle;
    if (!allocHandle(app, cpu, cpu.eax, handle))
        return;
    kept.back(cpu);
    returns(cpu, handle);
}

/* __freePOSIXHandle (clib/handleio/c/hdlman.c, 0x51e45c): handle eax's slot
 * free. */
void freePOSIXHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!freeHandle(app, cpu, cpu.eax))
        return;
    const x86::reg32 eax = cpu.eax;
    kept.back(cpu);
    returns(cpu, eax);
}

/* __close (clib/handleio/c/_clsewnt.c, 0x5095b0): handle eax closed; 0, or
 * -1. */
void close(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!closeHandle(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __sopen (clib/handleio/c/openwnt.c, 0x50901c): eax opened with edx's
 * flags and ebx's sharing, the permissions from the va_list at ecx. */
void sopen_(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!openHandle(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* _sopen (clib/handleio/c/openwnt.c, 0x508ff4): cdecl (name, flags,
 * sharing, ...); __sopen's, with a va_list of its frame for the rest. */
void sopen(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    const x86::reg32 entry = cpu.esp;
    x86::reg32 result;
    {
        const Frame frame(cpu, 0x10);  // ebx, ecx, edx; the va_list
        const x86::reg32 args = cpu.esp;
        put(app, args, entry + 0x10);
        if (!nested(cpu, [&] {
                return openHandle(app, cpu, at(app, entry + 4), at(app, entry + 8), at(app, entry + 0xc), args, result);
            }))
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

/* __open_flags (clib/streamio/c/fopen.c, 0x4edd80): the mode string at eax's
 * flags, its extended ones to edx's dword. */
void open_flags(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!openFlags(app, cpu, cpu.eax, cpu.edx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __doopen (clib/streamio/c/fopen.c, 0x4edebc): eax opened as dl's mode
 * says, with ebx's flags and ecx's extended ones, into the stream on the
 * stack after its sharing; takes them off. */
void doopen(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    const x86::reg32 entry = cpu.esp;
    x86::reg32 result;
    if (!doOpen(app, cpu, cpu.eax, x86::reg8(cpu.edx), cpu.ebx, cpu.ecx, at(app, entry + 4), at(app, entry + 8),
                result))
        return;
    kept.back(cpu);
    returns(cpu, result);
    cpu.esp += 8;
}

/* __allocfp (clib/streamio/c/allocfp.c, 0x509250): a stream, or 0. */
void allocfp(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 fp;
    if (!allocateStream(app, cpu, fp))
        return;
    kept.back(cpu);
    returns(cpu, fp);
}

/* __freefp (clib/streamio/c/allocfp.c, 0x509314): the stream at eax off the
 * open ones; its link, or 0. */
void freefp(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, freeStream(app, cpu.eax));
}

/* __doclose (clib/streamio/c/fclose.c, 0x4ee214): the stream at eax given
 * up, its handle closed too if edx says so. */
void doclose(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!doClose(app, cpu, cpu.eax, cpu.edx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __shutdown_stream (clib/streamio/c/fclose.c, 0x4ee144): the stream at eax
 * given up and freed. */
void shutdown_stream(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!shutdownStream(app, cpu, cpu.eax, cpu.edx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fclose (clib/streamio/c/fclose.c, 0x4ee100): the stream at eax closed; 0,
 * or -1. */
void fclose(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!closeStream(app, cpu, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* _fsopen (clib/streamio/c/fopen.c, 0x4edfa4): eax opened as the mode at edx
 * says, with ebx's sharing; the stream, or 0. */
void fsopen(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!openStream(app, cpu, cpu.eax, cpu.edx, cpu.ebx, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* fopen (clib/streamio/c/fopen.c, 0x4edfe8): _fsopen's, sharing as it may. */
void fopen(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    {
        const Frame frame(cpu, 4);  // ebx
        if (!nested(cpu, [&] { return openStream(app, cpu, cpu.eax, cpu.edx, 0, result); }))
            return;
    }
    kept.back(cpu);
    returns(cpu, result);
}

/* ---- The heap (native_watcom_heap.cpp has it; these are the game's calls
 * into it, each keeping the registers its code kept: all but eax, but for
 * what __MemAllocator and __MemFree are handed). ---- */

/* _nmalloc (clib/heap/c/nmalloc.c, 0x4f7900): eax bytes; the block, or 0. */
void nmalloc(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!watcomheap::allocate(app, cpu, kGameHeap, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* _nfree (clib/heap/c/nfree.c, 0x4f79f0): the block at eax given back. */
void nfree(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!watcomheap::free(app, cpu, kGameHeap, cpu.eax))
        return;
    const x86::reg32 eax = cpu.eax;
    kept.back(cpu);
    returns(cpu, eax);
}

/* __MemAllocator (clib/heap/c/mem.c, 0x515300): eax bytes from the miniheap at
 * ebx (edx the segment, nothing on a flat heap); the block, or 0. */
void MemAllocator(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, watcomheap::allocateIn(app, cpu.ebx, cpu.eax));
}

/* __MemFree (clib/heap/c/mem.c, 0x5153b0): the block at eax given back to the
 * miniheap at ebx. */
void MemFree(win32::WinApplication* app, x86::CPU& cpu)
{
    watcomheap::freeIn(app, cpu.ebx, cpu.eax);
    cpu.esp += 4;
}

/* __LinkUpNewNHeap (clib/heap/c/grownear.c, 0x5154e0): the new miniheap at eax
 * into the list; its first block. */
void LinkUpNewNHeap(win32::WinApplication* app, x86::CPU& cpu)
{
    returns(cpu, watcomheap::link(app, kGameHeap, cpu.eax));
}

/* __AdjustAmount (clib/heap/c/grownear.c, 0x515628): the amount at eax made
 * what a miniheap for it takes, in place; 1, or 0. */
void AdjustAmount(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 amount = at(app, cpu.eax);
    const bool adjusted = watcomheap::adjust(app, kGameHeap, amount);
    put(app, cpu.eax, amount);
    returns(cpu, adjusted ? 1 : 0);
}

/* __CreateNewNHeap (clib/heap/c/grownear.c, 0x515558): a miniheap with room
 * for eax bytes; 1, or 0. */
void CreateNewNHeap(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!watcomheap::create(app, cpu, kGameHeap, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __ExpandDGROUP (clib/heap/c/grownear.c, 0x515614): the empty miniheaps given
 * back, then a new one with room for eax bytes; 1, or 0. */
void ExpandDGROUP(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!watcomheap::expand(app, cpu, kGameHeap, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* __nmemneed (clib/heap/c/nmemneed.c, 0x515680): whether the caller could give
 * memory back for eax bytes more -- no. */
void nmemneed(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    returns(cpu, 0);
}

/* _nheapshrink (clib/heap/c/nheapmin.c, 0x5241c0): every empty miniheap given
 * back to the system; 0. */
void nheapshrink(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    if (!watcomheap::shrink(app, cpu, kGameHeap))
        return;
    kept.back(cpu);
    returns(cpu, 0);
}

/* __ReturnMemToSystem (clib/heap/c/nheapmin.c, 0x5241fc): the miniheap at eax
 * to VirtualFree, and the rovers off it; 0, or -1. */
void ReturnMemToSystem(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!watcomheap::giveBack(app, cpu, kGameHeap, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
}

/* 0x52425c, _nheapshrink's step (clib/heap/c/nheapmin.c, __UnlinkNHeap after
 * __ReturnMemToSystem): the miniheap at eax given back and, if it went, taken
 * off the list; 0, or -1. */
void ReleaseNHeap(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu);
    x86::reg32 result;
    if (!watcomheap::release(app, cpu, kGameHeap, cpu.eax, result))
        return;
    kept.back(cpu);
    returns(cpu, result);
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
 * the x87's formatter the start-up code puts there, which stays generated.
 *
 * es is as the code has it: what it reads and writes through es is at ees,
 * the register whole, and the offset -- a far pointer's segment ('F') added
 * to its offset, and nothing for the game's own, whose es is its ds.  les is
 * never run, NFS2_ASSERT(false) where it would be, and what the register held
 * stays. */
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

/* What a function's push instructions leave below its return address: each
 * dword in turn, and es's word where it has one (-1 for none), the dword's
 * other half left as it was.  Written as the code writes them, for what reads
 * the stack where they are -- a wide string from __prtf's buffer goes on into
 * its frame. */
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

/* A native of these called from another, as the generated code calls it: the
 * return address's slot, and esp back after it. */
bool calls(win32::WinApplication* app, x86::CPU& cpu, guest::Native function)
{
    return call(app, cpu, function);
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

/* 0x51e060 (wctomb's core): the wide character in edx as WideCharToMultiByte
 * makes it in the game's code page, into eax's two bytes; how many, or -1.  A
 * null buffer is 0. */
void wctomb_(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx;
    const reg32 local = cpu.esp - 8;  // ecx, then the character
    pushed(app, cpu.esp, {ecx});
    put(app, local, cpu.edx);
    if (cpu.eax != 0)
    {
        cpu.esp = local;
        cpu.edx = at(app, kCodePage);
        if (!call(app, cpu, at(app, kWideCharToMultiByte), {cpu.edx, 0x200, local, 1, cpu.eax, 2, 0, 0}))
            return;
        if (cpu.eax == 0)
            cpu.eax = 0xffffffff;
        cpu.esp = local + 8;
    }
    cpu.ecx = ecx;
    cpu.esp += 4;
}

/* 0x51e0a0 (utoa's core, clib/convert/c/itoa.c): eax in the base in ebx, its
 * digits from kDigits, into edx; edx back.  Its digits are made in its frame
 * backwards and copied out the right way round. */
void utoa(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg32 frame = cpu.esp - 0x38;  // ecx, esi, edi, ebp, and 0x28 of its own
    const reg32 base = cpu.ebx, out = cpu.edx;
    pushed(app, cpu.esp, {ecx, esi, edi, ebp});
    putByte(app, frame, 0);
    reg32 p = frame + 1, value = cpu.eax, rest = 0;
    do
    {
        put(app, frame + 0x24, base);
        rest = value % base;
        value /= base;
        put(app, frame + 0x24, value);
        putByte(app, p++, byte(app, kDigits + rest));
    } while (value != 0);
    reg32 to = out;
    reg8 c;
    do
    {
        c = byte(app, --p);
        putByte(app, to++, c);
    } while (c != 0);
    cpu.eax = out;
    cpu.ebx = frame + 0x24;
    cpu.edx = rest;
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.esp += 4;
}

/* 0x51e0ec (itoa, clib/convert/c/itoa.c): eax signed in base 10 -- a '-'
 * first -- unsigned in any other, into edx; edx back. */
void itoa(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, out = cpu.edx;
    pushed(app, cpu.esp, {ecx});
    if (cpu.ebx == 10 && x86::sreg32(cpu.eax) < 0)
    {
        cpu.eax = 0 - cpu.eax;
        putByte(app, cpu.edx, '-');
        ++cpu.edx;
    }
    cpu.esp -= 4;  // ecx
    if (!calls(app, cpu, utoa))
        return;
    cpu.esp += 4;
    cpu.eax = out;
    cpu.ecx = ecx;
    cpu.esp += 4;
}

/* 0x525d59 (__U8D, the compiler's unsigned 64-bit division): edx:eax by
 * ecx:ebx; the quotient in edx:eax, the remainder in ecx:ebx. */
void U8D(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.ecx == 0 && cpu.ebx == 1)
    {
        cpu.ebx = 0;
        cpu.esp += 4;
        return;
    }
    if (cpu.ecx != 0 && cpu.ecx < cpu.edx)
        pushed(app, cpu.esp, {cpu.ebp, cpu.esi, cpu.edi});  // the long division's
    const x86::reg64 n = x86::reg64(cpu.edx) << 32 | cpu.eax;
    const x86::reg64 d = x86::reg64(cpu.ecx) << 32 | cpu.ebx;
    const x86::reg64 q = d == 0 ? 0 : n / d, r = d == 0 ? n : n % d;
    cpu.eax = reg32(q);
    cpu.edx = reg32(q >> 32);
    cpu.ebx = reg32(r);
    cpu.ecx = reg32(r >> 32);
    cpu.esp += 4;
}

/* 0x51e130 (ulltoa, clib/convert/c/lltoa.c): the unsigned 64 bits at eax in
 * the base in ebx, its digits from kDigits64, into edx; edx back -- a digit
 * at a time by __U8D, the quotient and remainder in its frame. */
void ulltoa(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg16 es = cpu.es;
    const reg32 frame = cpu.esp - 0x58;  // ecx, esi, edi, es, ebp, and 0x44 of its own
    const reg32 value = cpu.eax, out = cpu.edx;
    pushed(app, cpu.esp, {ecx, esi, edi, 0, ebp}, 3, es);
    put(app, frame + 0x3c, out);
    put(app, frame + 0x40, out);
    cpu.es = cpu.ds;
    // Two movsd, as the direction flag has them.
    const reg32 step = cpu.flags.df ? reg32(-4) : 4;
    put(app, cpu.ees + frame + 0x34, at(app, value));
    put(app, cpu.ees + frame + 0x34 + step, at(app, value + step));
    put(app, frame + 0x24, cpu.ebx);
    put(app, frame + 0x28, 0);
    putByte(app, frame, 0);
    reg32 p = frame + 1;
    cpu.esp = frame;
    do
    {
        cpu.edi = frame + 0x2c;
        cpu.esi = frame + 0x34;
        cpu.edx = at(app, frame + 0x38);
        cpu.eax = at(app, frame + 0x34);
        cpu.ecx = at(app, frame + 0x28);
        cpu.ebx = at(app, frame + 0x24);
        if (!calls(app, cpu, U8D))
            return;
        put(app, frame + 0x38, cpu.edx);
        put(app, frame + 0x34, cpu.eax);
        put(app, frame + 0x30, cpu.ecx);
        put(app, frame + 0x2c, cpu.ebx);
        putByte(app, p++, byte(app, at(app, frame + 0x2c) + kDigits64));
        cpu.ebx = at(app, frame + 0x34);
    } while (cpu.ebx != 0 || at(app, frame + 0x38) != 0);
    reg8 c;
    do
    {
        cpu.ebx = at(app, frame + 0x40);
        c = byte(app, --p);
        cpu.esi = cpu.ebx + 1;
        putByte(app, cpu.ebx, c);
        put(app, frame + 0x40, cpu.esi);
    } while (c != 0);
    cpu.eax = at(app, frame + 0x3c);
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.es = es;
    cpu.esp = frame + 0x58 + 4;
}

/* 0x51e220 (__prtf's utoa): eax in the base in ebx, its digits from
 * kDigitsLow, into edx; edx back. */
void prtfUtoa(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg32 frame = cpu.esp - 0x38;  // ecx, esi, edi, ebp, and 0x28 of its own
    const reg32 base = cpu.ebx, out = cpu.edx;
    pushed(app, cpu.esp, {ecx, esi, edi, ebp});
    putByte(app, frame, 0);
    reg32 p = frame + 1, value = cpu.eax, rest = 0;
    reg8 digit = 0;
    do
    {
        put(app, frame + 0x24, base);
        rest = value % base;
        value /= base;
        put(app, frame + 0x24, rest);
        digit = byte(app, rest + kDigitsLow);
        putByte(app, p++, digit);
    } while (value != 0);
    reg32 to = out;
    reg8 c;
    do
    {
        c = byte(app, --p);
        putByte(app, to++, c);
    } while (c != 0);
    cpu.eax = out;
    cpu.ebx = (rest & 0xffffff00) | digit;
    cpu.edx = rest;
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.esp += 4;
}

/* 0x4fefdc (__prtf's strupr): the string at eax in capitals, by toupper. */
void prtfUpper(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ebx = cpu.ebx, edx = cpu.edx;
    pushed(app, cpu.esp, {ebx, edx});
    cpu.esp -= 8;  // ebx, edx
    reg32 p = cpu.eax;
    if (byte(app, p) != 0)
    {
        reg8 next;
        do
        {
            cpu.eax = byte(app, p);
            cpu.edx = p;
            if (!calls(app, cpu, toupper))
                return;
            putByte(app, p, reg8(cpu.eax));
            next = byte(app, p + 1);
            cpu.ebx = (cpu.ebx & 0xffffff00) | next;
            ++p;
        } while (next != 0);
    }
    cpu.ebx = ebx;
    cpu.edx = edx;
    cpu.esp += 8 + 4;
}

/* 0x4fe734 (__prtf's strnlen): the length of the string at eax, at most ebx
 * (-1 for any); edx its segment. */
void prtfLength(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg16 es = cpu.es;
    const reg32 limit = cpu.ebx;
    reg32 p = cpu.eax, length = 0;
    reg8 c;
    pushed(app, cpu.esp, {cpu.ecx, cpu.esi, cpu.edi, 0}, 3, es);
    cpu.es = reg16(cpu.edx);
    const reg32 base = cpu.ees;
    cpu.es = es;
    for (;;)
    {
        c = byte(app, base + p++);
        if (c == 0 || length == limit)
            break;
        ++length;
    }
    cpu.eax = length;
    cpu.edx = p;
    cpu.ebx = (limit & 0xffffff00) | c;
    cpu.esp += 4;
}

/* 0x4fe758 (__prtf's wcsnlen in bytes): how many bytes the wide string at
 * eax is as wctomb makes it, at most ebx of them (-1 for any); edx its
 * segment. */
void prtfWideLength(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi;
    const reg32 limit = cpu.ebx;
    const reg32 local = cpu.esp - 0x10;  // ecx, esi, es, and the character's bytes
    const reg16 es = cpu.es;
    reg32 p = cpu.eax, count = 0;
    pushed(app, cpu.esp, {ecx, esi, 0}, 2, es);
    cpu.es = reg16(cpu.edx);
    cpu.esp = local;
    for (;;)
    {
        const reg16 wc = half(app, cpu.ees + p);
        if (wc == 0 || (limit != 0xffffffff && x86::sreg32(count) > x86::sreg32(limit)))
            break;
        cpu.edx = wc;
        cpu.eax = local;
        p += 2;
        cpu.ebx = p;
        if (!calls(app, cpu, wctomb_))
            return;
        if (cpu.eax != 0xffffffff)
            count += cpu.eax;
    }
    cpu.eax = limit != 0xffffffff && x86::sreg32(count) > x86::sreg32(limit) ? limit : count;
    cpu.ebx = p;
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.es = es;
    cpu.esp = local + 0x10 + 4;
}

/* 0x4fe7c4 (__prtf's pointer's digits): eax in hexadecimal, right-aligned in
 * ebx digits with zeros before, into edx. */
void prtfHex(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg32 frame = cpu.esp - 0x14;  // ecx, esi, edi, ebp, and the width
    const reg32 out = cpu.edx, width = cpu.ebx;
    pushed(app, cpu.esp, {ecx, esi, edi, ebp});
    put(app, frame, width);
    cpu.ebx = 0x10;
    cpu.esp = frame;
    if (!calls(app, cpu, itoa))
        return;
    // repne scasb from it, es its ds, as the direction flag has it.
    const reg16 es = cpu.es;
    putHalf(app, frame - 4, es);  // push es
    cpu.es = cpu.ds;
    reg32 count = 0xffffffff, scan = out;
    reg8 scanned;
    do
    {
        scanned = byte(app, cpu.ees + scan);
        scan += cpu.flags.df ? reg32(-1) : 1;
        --count;
    } while (scanned != 0 && count != 0);
    cpu.es = es;
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
    putByte(app, out + at(app, frame), 0);
    cpu.eax = left;
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.esp = frame + 0x14 + 4;
}

/* 0x4fe81e: the end of __prtf's %p digits (0x4fe7c4), which its %hf
 * (0x4fe828) jumps to as well: the frame's dword let go and the registers
 * the two keep -- ebp, edi, esi, ecx -- taken back off the stack. */
void prtfHexEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp += 4;
    cpu.ebp = at(app, cpu.esp);
    cpu.edi = at(app, cpu.esp + 4);
    cpu.esi = at(app, cpu.esp + 8);
    cpu.ecx = at(app, cpu.esp + 0xc);
    cpu.esp += 0x10 + 4;
}

/* 0x4fe828 (__prtf's %hf): edx a 16.16 fixed-point number, into eax -- its
 * whole part by itoa, then the precision's digits (4 for none), rounded
 * half up. */
void prtfFixed(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg32 local = cpu.esp - 0x14;  // ecx, esi, edi, ebp, and the number
    const reg32 specs = cpu.ebx;
    reg32 p = cpu.eax;
    pushed(app, cpu.esp, {ecx, esi, edi, ebp});
    put(app, local, cpu.edx);
    if (x86::sreg32(cpu.edx) < 0)
    {
        putByte(app, p, '-');
        ++p;
        put(app, local, 0 - cpu.edx);
    }
    if (at(app, specs + kPrec) == 0xffffffff)
        put(app, specs + kPrec, 4);
    cpu.ebx = 10;
    cpu.eax = half(app, local + 2);
    cpu.edx = p;
    cpu.esp = local;
    if (!calls(app, cpu, itoa))
        return;
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
        // Rounded up: a 9 to a 0 and the digit before up, to a 1 before
        // them all, which moves the point and the rest one on.
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
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.esp = local + 0x14 + 4;
}

/* 0x4fe924 (__prtf's float): [__EFG_printf] with eax the buffer, edx the
 * arguments' pointer, ebx the SPECS. */
void prtfFloat(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!call(app, cpu, at(app, kFloatFormat)))
        return;
    cpu.esp += 4;
}

/* 0x4fe92c (__prtf's zero padding): with '0' padding and not '-', what the
 * width leaves over the parts as zeros after the first. */
void prtfZeros(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 specs = cpu.eax;
    pushed(app, cpu.esp, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == '0')
    {
        const reg32 nz0 = at(app, specs + kNZ0);
        const reg32 left = at(app, specs + kWidth) - at(app, specs + kN0) - nz0 - at(app, specs + kN1)
                           - at(app, specs + kNZ1) - at(app, specs + kN2) - at(app, specs + kNZ2);
        if (x86::sreg32(left) > 0)
            put(app, specs + kNZ0, nz0 + left);
    }
    cpu.esp += 4;
}

/* 0x4fe6cc (__prtf's flags): the flags at eax into the SPECS at edx; past
 * them. */
void prtfFlags(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 specs = cpu.edx;
    reg32 p = cpu.eax;
    pushed(app, cpu.esp, {cpu.ebx, cpu.ecx});
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
    cpu.eax = p;
    cpu.esp += 4;
}

/* 0x4fe578 (__prtf's specification): what follows a '%' at eax -- flags,
 * width, precision, size -- into the SPECS at ebx, a '*' taken from the
 * arguments' pointer at edx; past it. */
void prtfSpec(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi;
    const reg32 argp = cpu.edx, specs = cpu.ebx;
    pushed(app, cpu.esp, {ecx, esi, edi});
    putByte(app, specs + kPadChar, ' ');
    cpu.edx = specs;
    cpu.esp -= 0xc;  // ecx, esi, edi
    if (!calls(app, cpu, prtfFlags))
        return;
    cpu.esp += 0xc;
    reg32 p = cpu.eax, edx = cpu.edx, moved;
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
    cpu.eax = p;
    cpu.edx = edx;
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.esp += 4;
}

/* 0x4fe970 (__prtf's %ls): the wide string at eax (edx its segment) out by
 * the function in ecx, as wctomb makes each character, as many bytes as the
 * SPECS at ebx left for it. */
void prtfWideOut(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg16 es = cpu.es;
    const reg32 local = cpu.esp - 0x14;  // esi, edi, es, ebp, and the character's bytes
    const reg32 specs = cpu.ebx, out = cpu.ecx;
    reg32 p = cpu.eax;
    pushed(app, cpu.esp, {esi, edi, 0, ebp}, 2, es);
    cpu.es = reg16(cpu.edx);
    cpu.esp = local;
    if (x86::sreg32(at(app, specs + kN1)) > 0)
    {
        for (;;)
        {
            cpu.edx = half(app, cpu.ees + p);
            cpu.eax = local;
            p += 2;
            cpu.edi = p;
            if (!calls(app, cpu, wctomb_))
                return;
            reg32 n = cpu.eax;
            cpu.ecx = n;
            if (n != 0xffffffff)
            {
                if (x86::sreg32(n) > x86::sreg32(at(app, specs + kN1)))
                {
                    put(app, specs + kN1, 0);
                    break;
                }
                for (reg32 from = local; --n != 0xffffffff; ++from)
                {
                    cpu.edx = byte(app, from);
                    cpu.eax = specs;
                    cpu.ecx = n;
                    cpu.esi = from;
                    cpu.edi = p;
                    cpu.ebp = out;
                    if (!call(app, cpu, out))
                        return;
                    cpu.eax = at(app, specs + kN1) - 1;
                    put(app, specs + kN1, cpu.eax);
                }
                cpu.ecx = n;
            }
            if (x86::sreg32(at(app, specs + kN1)) <= 0)
                break;
        }
    }
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.es = es;
    cpu.esp = local + 0x14 + 4;
}

/* 0x4fe9d4 (__prtf's conversion): the argument the SPECS at ebx's conversion
 * takes, by the arguments' pointer at edx, made into the buffer at eax -- a
 * prefix (sign, 0x, 0) and the parts' lengths in the SPECS; the string to
 * write back, its segment in edx. */
void prtfConvert(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg16 savedEs = cpu.es;
    const reg32 local = cpu.esp - 0x20;  // esi, edi, es, ebp, and 0x10 of its own
    const reg32 buffer = cpu.eax, argp = cpu.edx, specs = cpu.ebx;
    reg32 moved;
    pushed(app, cpu.esp, {esi, edi, 0, ebp}, 2, savedEs);
    cpu.es = cpu.ds;
    for (const reg32 field : {kN0, kNZ0, kN1, kNZ1, kN2})
        put(app, specs + field, 0);
    reg32 string = buffer;
    put(app, specs + kNZ2, 0);
    cpu.esp = local;
    cpu.ecx = specs;
    cpu.esi = buffer;
    cpu.edi = buffer;

    auto prefix = [&](reg8 c) {
        const reg32 n = at(app, specs + kN0);
        put(app, specs + kN0, n + 1);
        putByte(app, buffer + n, c);
    };
    auto finish = [&] {
        cpu.edx = cpu.es;
        cpu.eax = string;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.es = savedEs;
        cpu.esp = local + 0x20 + 4;
    };
    // The length of what is at `from` in `segment`, by __prtf's strnlen.
    auto length = [&](reg32 from, reg32 segment, reg32 limit, reg32& result) {
        cpu.ebx = limit;
        cpu.edx = segment;
        cpu.eax = from;
        if (!calls(app, cpu, prtfLength))
            return false;
        result = cpu.eax;
        return true;
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
            cpu.ebx = specs;
            cpu.edx = value;
            cpu.eax = buffer;
            if (!calls(app, cpu, prtfFixed) || !length(buffer, cpu.ds, 0xffffffff, result))
                return;
            put(app, specs + kN1, result);
            return finish();
        }
        [[fallthrough]];
    case 'e':
    case 'E':
    case 'g':
    case 'G':
        cpu.ebx = specs;
        cpu.eax = buffer;
        cpu.edx = argp;
        if (!calls(app, cpu, prtfFloat))
            return;
        cpu.eax = specs;
        if (!calls(app, cpu, prtfZeros))
            return;
        cpu.es = cpu.ds;
        string = buffer + 1;
        return finish();
    case 's':
    case 'S':
    {
        putByte(app, buffer, 0);
        const reg8 flags = byte(app, specs + kFlags);
        if (flags & kFar)
        {
            const reg32 offset = nextArgument(app, argp, 8, moved);
            const reg16 segment = half(app, moved - 4);
            if (offset != 0 || segment != 0)
            {
                cpu.es = segment;
                string = offset;
            }
        }
        else
        {
            const reg32 pointer = nextArgument(app, argp, 4, moved);
            if (pointer != 0)
            {
                cpu.es = cpu.ds;
                string = pointer;
            }
        }
        const bool wide = byte(app, specs + kCharacter) == 'S' ? !(byte(app, specs + kFlags) & kShort)
                                                               : (byte(app, specs + kFlags) & kLong) != 0;
        cpu.edx = cpu.es;
        cpu.eax = string;
        cpu.ebx = at(app, specs + kPrec);
        if (!calls(app, cpu, wide ? prtfWideLength : prtfLength))
            return;
        const reg32 precision = at(app, specs + kPrec);
        put(app, specs + kN1, cpu.eax);
        if (x86::sreg32(precision) >= 0 && x86::sreg32(cpu.eax) > x86::sreg32(precision))
            put(app, specs + kN1, precision);
        return finish();
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
            const reg32 segment = nextArgument(app, argp, 4, moved) & 0xffff;
            cpu.ebx = 4;
            cpu.edx = buffer;
            cpu.eax = segment;
            if (!calls(app, cpu, prtfHex))
                return;
            putByte(app, buffer + 4, ':');
            to = buffer + 5;
        }
        cpu.ebx = 8;
        cpu.edx = to;
        cpu.eax = offset;
        if (!calls(app, cpu, prtfHex))
            return;
        if (byte(app, specs + kCharacter) == 'P')
        {
            cpu.eax = buffer;
            if (!calls(app, cpu, prtfUpper))
                return;
        }
        if (!length(string, cpu.es, 0xffffffff, result))
            return;
        put(app, specs + kN0, result);
        return finish();
    }
    case 'c':
    {
        put(app, specs + kN0, 1);
        if (byte(app, specs + kFlags) & kLong)
        {
            const reg32 wc = nextArgument(app, argp, 4, moved) & 0xffff;
            cpu.edx = wc;
            cpu.eax = local + 0xc;
            if (!calls(app, cpu, wctomb_))
                return;
            if (cpu.eax == 0xffffffff)
                return finish();
            putByte(app, buffer, byte(app, local + 0xc));
            if (at(app, kMbcsOn) == 0 || !(byte(app, kMbcsLead + byte(app, local + 0xc)) & 1))
                return finish();
            putByte(app, buffer + 1, byte(app, local + 0xd));
            put(app, specs + kN0, at(app, specs + kN0) + 1);
            return finish();
        }
        putByte(app, buffer, reg8(nextArgument(app, argp, 4, moved)));
        return finish();
    }
    case 'C':
    {
        cpu.edx = nextArgument(app, argp, 4, moved) & 0xffff;
        cpu.eax = buffer;
        if (!calls(app, cpu, wctomb_))
            return;
        put(app, specs + kN0, cpu.eax == 0xffffffff ? 0 : cpu.eax);
        return finish();
    }
    default:
        put(app, specs + kWidth, 0);
        putByte(app, buffer, byte(app, specs + kCharacter));
        put(app, specs + kN0, 1);
        return finish();
    }

    // The integer's digits after the prefix, by utoa or ulltoa: none for 0
    // with a precision of 0.
    cpu.es = cpu.ds;
    string = buffer + at(app, specs + kN0);
    if (byte(app, specs + kFlags2) & kLongLong)
    {
        if (at(app, specs + kPrec) == 0 && at(app, local) == 0 && at(app, local + 4) == 0)
        {
            putByte(app, cpu.ees + string, 0);
            result = 0;
        }
        else
        {
            cpu.ebx = base;
            cpu.edx = at(app, specs + kN0) + buffer;
            cpu.eax = local;
            if (!calls(app, cpu, ulltoa))
                return;
            if (byte(app, specs + kCharacter) == 'X')
            {
                cpu.eax = buffer;
                if (!calls(app, cpu, prtfUpper))
                    return;
            }
            if (!length(string, cpu.es, 0xffffffff, result))
                return;
        }
    }
    else if (at(app, specs + kPrec) == 0 && at(app, local + 8) == 0)
    {
        putByte(app, cpu.ees + string, 0);
        result = 0;
    }
    else
    {
        cpu.ebx = base;
        cpu.edx = at(app, specs + kN0) + buffer;
        cpu.eax = at(app, local + 8);
        if (!calls(app, cpu, prtfUtoa))
            return;
        if (byte(app, specs + kCharacter) == 'X')
        {
            cpu.eax = buffer;
            if (!calls(app, cpu, prtfUpper))
                return;
        }
        if (!length(string, cpu.es, 0xffffffff, result))
            return;
    }
    put(app, specs + kN1, result);
    const reg32 precision = at(app, specs + kPrec);
    if (x86::sreg32(result) < x86::sreg32(precision))
        put(app, specs + kNZ0, precision - result);
    if (at(app, specs + kPrec) == 0xffffffff)
    {
        cpu.eax = specs;
        if (!calls(app, cpu, prtfZeros))
            return;
    }
    finish();
}

/* __prtf (0x4fe1d0): the format at edx with the
 * arguments' pointer at ebx, each character it makes handed to the function
 * in ecx with eax its SPECS, whose first dword is eax; how many went. */
void prtf(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg16 savedEs = cpu.es;
    const reg32 specs = cpu.esp - 0x80;  // esi, edi, es, ebp, and 0x70 of its own
    const reg32 argp = cpu.ebx, out = cpu.ecx;
    constexpr reg32 kArgCopy = 0x60, kArgCopy2 = 0x64, kFormat = 0x68, kNullChar = 0x6c, kBuffer = 0x38;
    pushed(app, cpu.esp, {esi, edi, 0, ebp}, 2, savedEs);
    putByte(app, specs + kNullChar, 0);
    putHalf(app, specs + kFlags, 0);
    put(app, specs + kDest, cpu.eax);
    putHalf(app, specs + kSpecs1C, 100);
    put(app, specs + kOutCount, 0);
    put(app, specs + kFormat, cpu.edx);
    cpu.esp = specs;
    cpu.esi = argp;
    cpu.edi = out;

    auto put1 = [&](reg32 c) {
        cpu.eax = specs;
        cpu.edx = c;
        return call(app, cpu, out);
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

    if (byte(app, cpu.edx) != 0)
        do
        {
            const reg32 at_ = at(app, specs + kFormat);
            const reg8 c = byte(app, at_);
            if (c != '%')
            {
                put(app, specs + kFormat, at_ + 1);
                if (!put1(c))
                    return;
                continue;
            }
            cpu.ebx = specs;
            put(app, specs + kArgCopy, at(app, argp));
            cpu.edx = specs + kArgCopy;
            cpu.eax = at_ + 1;
            if (!calls(app, cpu, prtfSpec))
                return;
            const reg32 edx = cpu.edx;
            const reg32 next = cpu.eax + 1;
            put(app, argp, at(app, specs + kArgCopy));
            const reg8 conversion = byte(app, next - 1);
            put(app, specs + kFormat, next);
            putByte(app, specs + kCharacter, conversion);
            if (conversion == 0)
                break;
            if (conversion == 'n')
            {
                // How many went so far, into the argument: a short for h,
                // a far pointer as the generated code has it.
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
                    target += cpu.ees;
                if (word)
                    putHalf(app, target, reg16(count));
                else
                    put(app, target, count);
                continue;
            }

            put(app, specs + kArgCopy2, at(app, argp));
            cpu.ecx = specs + kNullChar;
            cpu.ebx = specs;
            cpu.edx = specs + kArgCopy2;
            cpu.eax = specs + kBuffer;
            if (!calls(app, cpu, prtfConvert))
                return;
            reg32 string = cpu.eax;
            cpu.es = reg16(cpu.edx);
            put(app, argp, at(app, specs + kArgCopy2));
            const reg32 parts = at(app, specs + kN0) + at(app, specs + kNZ0) + at(app, specs + kN1)
                                + at(app, specs + kNZ1) + at(app, specs + kN2) + at(app, specs + kNZ2);
            put(app, specs + kWidth, at(app, specs + kWidth) - parts);
            if (!(byte(app, specs + kFlags) & kLeft) && byte(app, specs + kPadChar) == ' '
                && !repeat(kWidth, ' '))
                return;
            reg32 prefix = specs + kBuffer;
            if (!bytes(kN0, 0, prefix) || !repeat(kNZ0, '0'))
                return;
            const reg8 kind = byte(app, specs + kCharacter);
            if ((kind == 's' && (byte(app, specs + kFlags) & kLong)) || kind == 'S')
            {
                cpu.ebx = specs;
                cpu.ecx = out;
                cpu.edx = cpu.es;
                cpu.eax = string;
                if (!calls(app, cpu, prtfWideOut))
                    return;
            }
            else if (!bytes(kN1, cpu.ees, string))
                return;
            if (!repeat(kNZ1, '0') || !bytes(kN2, cpu.ees, string) || !repeat(kNZ2, '0'))
                return;
            if ((byte(app, specs + kFlags) & kLeft) && !repeat(kWidth, ' '))
                return;
        } while (byte(app, at(app, specs + kFormat)) != 0);
    cpu.eax = at(app, specs + kOutCount);
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.es = savedEs;
    cpu.esp = specs + 0x80 + 4;
}

/* mem_putc (0x4df660): sprintf's: edx's character
 * at the SPECS at eax's destination, which moves on, and counted. */
void mem_putc(win32::WinApplication* app, x86::CPU& cpu)
{
    pushed(app, cpu.esp, {cpu.ebx, cpu.ecx});
    const reg32 p = at(app, cpu.eax);
    put(app, cpu.eax, p + 1);
    putByte(app, p, reg8(cpu.edx));
    put(app, cpu.eax + kOutCount, at(app, cpu.eax + kOutCount) + 1);
    cpu.esp += 4;
}

/* file_putc (0x4ff000): vfprintf's: edx's
 * character to the stream in the SPECS at eax by fputc, and counted. */
void file_putc(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ebx = cpu.ebx, specs = cpu.eax;
    pushed(app, cpu.esp, {ebx});
    cpu.esp -= 4;  // ebx
    cpu.ebx = specs;
    cpu.eax = cpu.edx;
    cpu.edx = at(app, specs + kDest);
    if (!calls(app, cpu, fputc))
        return;
    put(app, specs + kOutCount, at(app, specs + kOutCount) + 1);
    cpu.ebx = ebx;
    cpu.esp += 4 + 4;
}

/* vsprintf (clib/string/c/vsprintf.c, 0x4df674): the format at edx with the
 * arguments' pointer at ebx into eax, a 0 after; how many. */
void vsprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, buffer = cpu.eax;
    pushed(app, cpu.esp, {ecx, esi});
    cpu.esp -= 8;  // ecx, esi
    cpu.esi = buffer;
    cpu.ecx = kMemPutc;
    if (!calls(app, cpu, prtf))
        return;
    putByte(app, buffer + cpu.eax, 0);
    cpu.ecx = ecx;
    cpu.esi = esi;
    cpu.esp += 8 + 4;
}

/* sprintf (clib/string/c/sprintf.c, 0x4df690): cdecl, the buffer and the
 * format on the stack, the arguments after. */
void sprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ebx = cpu.ebx, edx = cpu.edx, entry = cpu.esp;
    const reg32 list = entry - 0xc;  // ebx, edx, and the va_list
    pushed(app, entry, {ebx, edx});
    put(app, list, entry + 0xc);
    cpu.ebx = list;
    cpu.edx = at(app, entry + 8);
    cpu.eax = at(app, entry + 4);
    cpu.esp = list;
    if (!calls(app, cpu, vsprintf))
        return;
    cpu.edx = edx;
    cpu.ebx = ebx;
    cpu.esp = entry + 4;
}

/* vfprintf (0x4ff014): the format at edx with the
 * arguments' pointer at ebx to the stream at eax, under the file's lock: none
 * to a stream of wide characters, an unbuffered one buffered while it goes;
 * how many, or -1 for an error. */
void vfprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ecx = cpu.ecx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const reg32 fp = cpu.eax, format = cpu.edx, argp = cpu.ebx;
    pushed(app, cpu.esp, {ecx, esi, edi, ebp});
    cpu.esp -= 0x10;  // ecx, esi, edi, ebp
    cpu.esi = fp;
    auto back = [&](reg32 result) {
        cpu.eax = result;
        cpu.ecx = ecx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp += 0x10 + 4;
    };
    if (!streamLock(app, cpu, kAccessFile, fp))
        return;
    const reg32 link = linkOf(app, fp);
    const reg32 orientation = at(app, link + kOrientation);
    if (orientation != 1)
    {
        if (orientation != 0)
        {
            if (!streamLock(app, cpu, kReleaseFile, fp))
                return;
            return back(0);
        }
        put(app, link + kOrientation, 1);
    }
    const reg32 kept = at(app, fp + kFlag) & 0x30;
    putByte(app, fp + kFlag, byte(app, fp + kFlag) & 0xcf);
    if (bufferOf(app, fp) == 0)
    {
        cpu.eax = fp;
        if (!calls(app, cpu, ioalloc))
            return;
    }
    const bool unbuffered = byte(app, fp + kFlag + 1) & 4;
    if (unbuffered)
        putByte(app, fp + kFlag + 1, reg8((byte(app, fp + kFlag + 1) & 0xfa) | 1));
    cpu.ecx = kFilePutc;
    cpu.eax = fp;
    cpu.edx = format;
    cpu.ebx = argp;
    if (!calls(app, cpu, prtf))
        return;
    reg32 result = cpu.eax;
    if (unbuffered)
    {
        const reg8 restored = reg8((byte(app, fp + kFlag + 1) & 0xfa) | 4);
        putByte(app, fp + kFlag + 1, reg8(byte(app, fp + kFlag + 1) & 0xfa));
        cpu.ebx = (cpu.ebx & 0xffffff00) | restored;
        putByte(app, fp + kFlag + 1, restored);
        cpu.eax = fp;
        if (!calls(app, cpu, flush))
            return;
    }
    if (byte(app, fp + kFlag) & 0x20)
        result = 0xffffffff;
    put(app, fp + kFlag, at(app, fp + kFlag) | kept);
    cpu.edx = result;
    if (!streamLock(app, cpu, kReleaseFile, fp))
        return;
    cpu.edx = result;
    back(result);
}

/* fprintf (clib/streamio/c/fprintf.c, 0x4f4b80): cdecl, the stream and the
 * format on the stack, the arguments after. */
void fprintf(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 ebx = cpu.ebx, edx = cpu.edx, entry = cpu.esp;
    const reg32 list = entry - 0xc;  // ebx, edx, and the va_list
    pushed(app, entry, {ebx, edx});
    put(app, list, entry + 0xc);
    cpu.ebx = list;
    cpu.edx = at(app, entry + 8);
    cpu.eax = at(app, entry + 4);
    cpu.esp = list;
    if (!calls(app, cpu, vfprintf))
        return;
    cpu.edx = edx;
    cpu.ebx = ebx;
    cpu.esp = entry + 4;
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
    putHalf(app, cpu.esp, cpu.es);
}

void popEs(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.es = half(app, cpu.esp);
    cpu.esp += 4;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, guest::Native function)
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool indirect(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

void setLow8(reg32& r, reg32 value)
{
    r = (r & 0xffffff00) | (value & 0xff);
}

void setHigh8(reg32& r, reg32 value)
{
    r = (r & 0xffff00ff) | (value & 0xff) << 8;
}

bool multibyte(win32::WinApplication* app)
{
    return at(app, kMbcsOn) != 0;
}

bool leadByte(win32::WinApplication* app, reg32 c)
{
    return (byte(app, kMbcsLead + (c & 0xff)) & 1) != 0;
}

/* repne scasb: al looked for from es:edi, ecx at most. */
void scanBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    while (cpu.ecx != 0)
    {
        const reg8 b = byte(app, cpu.ees + cpu.edi);
        cpu.edi += cpu.flags.df ? reg32(-1) : 1;
        --cpu.ecx;
        if (b == reg8(cpu.eax))
            break;
    }
}

/* The length of the string at edi as the runtime's inline strlen takes it:
 * es pushed and made ds, repne scasb for 0 from -1, not and dec. */
void inlineStrlen(win32::WinApplication* app, x86::CPU& cpu)
{
    pushEs(app, cpu);
    cpu.eax = cpu.ds;
    cpu.es = reg16(cpu.eax);
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    scanBytes(app, cpu);
    cpu.ecx = ~cpu.ecx - 1;
    popEs(app, cpu);
}

/* rep movs of `size` bytes each, from esi to es:edi. */
void repMove(win32::WinApplication* app, x86::CPU& cpu, reg32 size)
{
    const reg32 step = cpu.flags.df ? reg32(0) - size : size;
    while (cpu.ecx != 0)
    {
        if (size == 4)
            put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
        else
            putByte(app, cpu.ees + cpu.edi, byte(app, cpu.esi));
        cpu.edi += step;
        cpu.esi += step;
        --cpu.ecx;
    }
}

/* The runtime's inline memcpy: ecx bytes from esi to es:edi, es pushed and
 * made ds, edi kept: rep movsd, then the rest by rep movsb. */
void inlineMemcpy(win32::WinApplication* app, x86::CPU& cpu)
{
    pushEs(app, cpu);
    cpu.eax = cpu.ds;
    cpu.es = reg16(cpu.eax);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.ecx >>= 2;
    repMove(app, cpu, 4);
    setLow8(cpu.ecx, cpu.eax & 3);
    repMove(app, cpu, 1);
    cpu.edi = pop(app, cpu);
    popEs(app, cpu);
}

}  // namespace

/* 0x5250c0 (_mbterm): whether the character at eax ends a string -- 1 for
 * its 0, 2 for a lead byte with a 0 after it -- or 0. */
void mbterm(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    const reg32 p = cpu.eax;
    if (byte(app, p) == 0)
        cpu.eax = 1;
    else if (multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) == 0)
        cpu.eax = 2;
    else
        cpu.eax = 0;
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525100 (_mbsinc): past the character at eax. */
void mbsinc(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    const reg32 p = cpu.eax;
    cpu.eax = multibyte(app) && leadByte(app, byte(app, p)) && byte(app, p + 1) != 0 ? p + 2 : p + 1;
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52b5b0 (_mbclen): the character at eax's bytes, 1 or 2. */
void mbclen(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = multibyte(app) && leadByte(app, byte(app, cpu.eax)) ? 2 : 1;
    cpu.esp += 4;
}

/* 0x52e310 (_mbccpy): the character at edx to eax. */
void mbccpy(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    const reg32 to = cpu.eax, from = cpu.edx;
    if (multibyte(app) && leadByte(app, byte(app, from)))
    {
        putByte(app, to, byte(app, from));
        setLow8(cpu.edx, byte(app, from + 1));
        putByte(app, to + 1, reg8(cpu.edx));
    }
    else
    {
        setLow8(cpu.edx, byte(app, from));
        putByte(app, to, reg8(cpu.edx));
    }
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52e3a0 (_mbccmp): the characters at eax and edx compared, their bytes'
 * difference. */
void mbccmp(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    const reg32 a = cpu.eax, b = cpu.edx;
    if (byte(app, a) != byte(app, b))
        cpu.eax = reg32(byte(app, a)) - byte(app, b);
    else if (multibyte(app) && leadByte(app, byte(app, a)) && byte(app, a + 1) != byte(app, b + 1))
    {
        cpu.edx = byte(app, b + 1);
        cpu.eax = reg32(byte(app, a + 1)) - cpu.edx;
    }
    else
        cpu.eax = 0;
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52ed30 (_mbsnextc): the character at eax, a lead byte's two as one. */
void mbsnextc(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    const reg32 p = cpu.eax;
    if (multibyte(app) && leadByte(app, byte(app, p)))
        cpu.eax = reg32(byte(app, p)) << 8 | byte(app, p + 1);
    else
        cpu.eax = byte(app, p);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52ede0 (_mbvtop): the character eax into the bytes at edx; edx back. */
void mbvtop(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    const reg32 c = cpu.eax, to = cpu.edx;
    cpu.eax = to;
    if (c & 0xff00)
    {
        putByte(app, to + 1, reg8(c));
        cpu.edx = c >> 8;
        putByte(app, to, reg8(cpu.edx));
    }
    else
        putByte(app, to, reg8(c));
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52ef70 (_ismbbtrail, as the runtime has it): whether the byte eax may
 * follow a lead byte. */
void mbbtrail(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!multibyte(app))
        cpu.eax = 0;
    else if (at(app, kCodePage) == 932)
        cpu.eax = byte(app, kSjisTrail + (cpu.eax & 0xff)) & 8;
    else if (cpu.eax != 0)
        cpu.eax = 1;
    cpu.esp += 4;
}

/* 0x52ef30 (_ismbclegal's): whether eax is a character of two bytes. */
void mbclegal(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax >> 8 & 0xff;
    if (multibyte(app) && leadByte(app, cpu.edx))
    {
        cpu.edx = 1;
        cpu.eax &= 0xff;
        if (!direct(app, cpu, mbbtrail))
            return;
        cpu.eax = cpu.eax != 0 ? 1 : 0;
    }
    else
    {
        if (multibyte(app))
            cpu.edx = 0;
        cpu.eax = 0;
    }
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52eee0 (_ismbclower's, code page 932): whether eax is a small letter --
 * a full-width one of two bytes, or one of _IsTable's. */
void mbclower(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, mbclegal))
        return;
    if (cpu.eax == 0)
    {
        cpu.eax = (cpu.edx + 1) & 0xff;
        cpu.eax = byte(app, kIsTable + cpu.eax) & 0x80;
        cpu.edx = cpu.eax;
    }
    else
    {
        cpu.edx = cpu.edx >= 0x8281 && cpu.edx <= 0x829a ? 1 : 0;
        cpu.eax = cpu.edx;
    }
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52eec0 (_mbctoupper's, code page 932): a full-width small letter as a
 * capital. */
void mbcupper932(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, mbclower))
        return;
    if (cpu.eax != 0)
        cpu.edx -= 0x21;
    cpu.eax = cpu.edx;
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52ed70 (_mbctoupper): the character eax as a capital, by CharUpperBuffA
 * -- or, a full-width one of code page 932, by the runtime's own. */
void mbctoupper(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 4;
    const reg32 local = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.edx = local;
    if (!direct(app, cpu, mbvtop))
        return;
    cpu.eax = local;
    setLow8(cpu.edx, 0);
    if (!direct(app, cpu, mbclen))
        return;
    putByte(app, local + cpu.eax, reg8(cpu.edx));
    if (at(app, kCodePage) == 932 && multibyte(app) && leadByte(app, byte(app, local)))
    {
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, mbcupper932))
            return;
    }
    else
    {
        push(app, cpu, 1);
        cpu.eax = local;
        push(app, cpu, cpu.eax);
        if (!indirect(app, cpu, at(app, kCharUpperBuffA)))
            return;
        cpu.eax = local;
        if (!direct(app, cpu, mbsnextc))
            return;
    }
    cpu.esp = local + 4;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52e350 (_mbsupr): the string at eax in capitals, a character at a time;
 * eax back. */
void mbsupr(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 4;
    const reg32 local = cpu.esp;
    cpu.ecx = cpu.eax;
    for (;;)
    {
        cpu.ebx = cpu.eax;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, mbterm))
            return;
        if (cpu.eax != 0)
            break;
        cpu.eax = cpu.ebx;
        cpu.edx = local;
        if (!direct(app, cpu, mbsnextc) || !direct(app, cpu, mbctoupper) || !direct(app, cpu, mbvtop))
            return;
        cpu.eax = local;
        setLow8(cpu.edx, 0);
        if (!direct(app, cpu, mbclen))
            return;
        putByte(app, local + cpu.eax, reg8(cpu.edx));
        cpu.edx = local;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, mbccpy))
            return;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, mbsinc))
            return;
    }
    cpu.eax = cpu.ecx;
    cpu.esp = local + 4;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52b5e0 (_mbcicmp): the characters at eax and edx compared as capitals. */
void mbcicmp(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.esp -= 8;
    const reg32 local = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.edx = cpu.eax;
    cpu.eax = local;
    if (!direct(app, cpu, mbccpy))
        return;
    cpu.eax = cpu.ebx;
    setLow8(cpu.edx, 0);
    if (!direct(app, cpu, mbclen))
        return;
    putByte(app, local + cpu.eax, reg8(cpu.edx));
    cpu.eax = local + 4;
    cpu.edx = cpu.ecx;
    if (!direct(app, cpu, mbccpy))
        return;
    cpu.eax = cpu.ecx;
    setHigh8(cpu.edx, 0);
    if (!direct(app, cpu, mbclen))
        return;
    putByte(app, local + cpu.eax + 4, reg8(cpu.edx >> 8));
    cpu.eax = local;
    if (!direct(app, cpu, mbsupr))
        return;
    cpu.eax = local + 4;
    cpu.edx = local + 4;
    if (!direct(app, cpu, mbsupr))
        return;
    cpu.eax = local;
    if (!direct(app, cpu, mbccmp))
        return;
    cpu.esp = local + 8;
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x523d10 (_mbsnicmp): at most ebx characters of the strings at eax and
 * edx compared as capitals. */
void mbsnicmp(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edx = 0;
    auto back = [&] {
        cpu.eax = cpu.edx;
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    while (cpu.ebx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, mbterm))
            return;
        if (cpu.eax != 0)
            break;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, mbterm))
            return;
        if (cpu.eax != 0)
            break;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, mbcicmp))
            return;
        cpu.edx = cpu.eax;
        if (cpu.eax != 0)
            return back();
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, mbsinc))
            return;
        cpu.ecx = cpu.eax;
        cpu.eax = cpu.esi;
        --cpu.ebx;
        if (!direct(app, cpu, mbsinc))
            return;
        cpu.esi = cpu.eax;
    }
    if (cpu.ebx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, mbterm))
            return;
        bool ended = cpu.eax != 0;
        if (!ended)
        {
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, mbterm))
                return;
            ended = cpu.eax != 0;
        }
        if (ended)
        {
            cpu.edx = reg32(byte(app, cpu.ecx)) - byte(app, cpu.esi);
            cpu.eax = byte(app, cpu.esi);
        }
    }
    back();
}

/* getenv (clib/environ/c/getenv.c, 0x50ead0): the value of the variable named
 * at eax, its name matched as capitals -- or 0. */
void getenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.ebp = cpu.eax;
    cpu.esi = at(app, kEnviron);
    reg32 result = 0;
    if (cpu.esi != 0 && cpu.eax != 0)
    {
        cpu.edi = cpu.eax;
        inlineStrlen(app, cpu);
        cpu.edi = cpu.ecx;
        for (cpu.ecx = at(app, cpu.esi); cpu.ecx != 0; cpu.ecx = at(app, cpu.esi))
        {
            cpu.ebx = cpu.edi;
            cpu.edx = cpu.ebp;
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, mbsnicmp))
                return;
            if (cpu.eax == 0 && byte(app, cpu.ecx + cpu.edi) == '=')
            {
                result = cpu.edi + 1 + cpu.ecx;
                break;
            }
            cpu.esi += 4;
        }
    }
    cpu.eax = result;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* wcstombs (clib/mbyte/c/wcstombs.c, 0x5242f0): the wide string at edx into
 * eax as wctomb makes each character, at most ebx bytes -- or, for no buffer,
 * how many it would take; -1 for one wctomb cannot make. */
void wcstombs(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0xc;
    const reg32 local = cpu.esp;
    put(app, local + 4, cpu.eax);
    cpu.ebp = cpu.edx;
    put(app, local, cpu.ebx);
    cpu.ebx = 0;
    auto back = [&] {
        cpu.esp = local + 0xc;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0)
    {
        while (half(app, cpu.ebp) != 0)
        {
            cpu.edx = half(app, cpu.ebp);
            cpu.eax = local + 8;
            if (!direct(app, cpu, wctomb_))
                return;
            if (cpu.eax == 0xffffffff)
                return back();
            cpu.ebp += 2;
            cpu.ebx += cpu.eax;
        }
        cpu.eax = cpu.ebx;
        return back();
    }
    while (at(app, local) != 0)
    {
        cpu.ecx = (cpu.ecx & 0xffff0000) | half(app, cpu.ebp);
        if ((cpu.ecx & 0xffff) == 0)
        {
            putByte(app, at(app, local + 4), 0);
            break;
        }
        cpu.edx = cpu.ecx & 0xffff;
        cpu.eax = local + 8;
        if (!direct(app, cpu, wctomb_))
            return;
        cpu.edx = cpu.eax;
        if (cpu.eax == 0xffffffff)
            return back();
        if (cpu.eax > at(app, local))
            break;
        cpu.esi = local + 8;
        cpu.edi = at(app, local + 4);
        cpu.ecx = cpu.eax;
        inlineMemcpy(app, cpu);
        cpu.ebp += 2;
        cpu.ebx += cpu.edx;
        cpu.eax = at(app, local) - cpu.edx;
        cpu.edi += cpu.edx;
        put(app, local + 4, cpu.edi);
        put(app, local, cpu.eax);
    }
    cpu.eax = cpu.ebx;
    back();
}

/* 0x515c90 (__tzset's number): the decimal digits at eax into the dword at
 * edx; past them. */
void tzNumber(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ecx = cpu.edx;
    setLow8(cpu.ebx, byte(app, cpu.eax));
    cpu.edx = 0;
    if ((cpu.ebx & 0xff) >= '0')
        while (byte(app, cpu.eax) <= '9')
        {
            cpu.edx *= 10;
            cpu.ebx = byte(app, cpu.eax);
            ++cpu.eax;
            cpu.edx += cpu.ebx;
            setHigh8(cpu.ebx, byte(app, cpu.eax));
            cpu.edx -= '0';
            if (((cpu.ebx >> 8) & 0xff) < '0')
                break;
        }
    put(app, cpu.ecx, cpu.edx);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x515cbc (__tzset's zone): TZ's name at eax -- up to a sign, a digit or a
 * comma, at most 0x80 of it -- into edx, and the hours[:minutes[:seconds]]
 * after it, as seconds west, into the dword at ebx; past them. */
void tzName(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x14;
    const reg32 local = cpu.esp;
    cpu.ebp = cpu.eax;
    put(app, local, cpu.edx);
    put(app, local + 0x10, cpu.ebx);
    if (byte(app, cpu.eax) == ':')
        ++cpu.ebp;
    cpu.esi = cpu.ebp;
    for (;; ++cpu.ebp)
    {
        setLow8(cpu.edx, byte(app, cpu.ebp));
        const reg8 c = reg8(cpu.edx);
        if (c == 0 || c == ',' || c == '-' || c == '+' || (c >= '0' && c <= '9'))
            break;
    }
    cpu.ebx = cpu.ebp - cpu.esi;
    if (x86::sreg32(cpu.ebx) > 0x80)
        cpu.ebx = 0x80;
    cpu.edi = at(app, local);
    cpu.ecx = cpu.ebx;
    inlineMemcpy(app, cpu);
    cpu.ebx += cpu.edi;
    cpu.eax = cpu.ebp + 1;
    cpu.esi = 0;
    putByte(app, cpu.ebx, 0);
    if (reg8(cpu.edx) == '-')
    {
        cpu.esi = 1;
        cpu.ebp = cpu.eax;
    }
    else if (reg8(cpu.edx) == '+')
        cpu.ebp = cpu.eax;
    setLow8(cpu.eax, byte(app, cpu.ebp));
    if (reg8(cpu.eax) >= '0' && reg8(cpu.eax) <= '9')
    {
        cpu.edx = local + 0xc;
        cpu.edi = 0;
        cpu.eax = cpu.ebp;
        put(app, local + 4, 0);
        put(app, local + 8, 0);
        put(app, local + 0xc, 0);
        if (!direct(app, cpu, tzNumber))
            return;
        setLow8(cpu.edx, byte(app, cpu.eax));
        cpu.ebp = cpu.eax;
        if (reg8(cpu.edx) == ':')
        {
            cpu.edx = local + 8;
            ++cpu.eax;
            if (!direct(app, cpu, tzNumber))
                return;
            setHigh8(cpu.edx, byte(app, cpu.eax));
            cpu.ebp = cpu.eax;
            if (reg8(cpu.edx >> 8) == ':')
            {
                cpu.edx = local + 4;
                ++cpu.eax;
                if (!direct(app, cpu, tzNumber))
                    return;
                cpu.ebp = cpu.eax;
            }
        }
        cpu.ebx = at(app, local + 0xc);
        cpu.edx = at(app, local + 8) + cpu.ebx * 60;
        cpu.edx = at(app, local + 4) + cpu.edx * 60;
        cpu.eax = at(app, local + 0x10);
        put(app, cpu.eax, cpu.edx);
        if (cpu.esi != 0)
        {
            cpu.ebx = 0 - cpu.edx;
            put(app, cpu.eax, cpu.ebx);
        }
    }
    cpu.eax = cpu.ebp;
    cpu.esp = local + 0x14;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x515dd4 (__tzset's rule): daylight saving's start or end at eax -- Jn,
 * n or Mm.w.d, then /hours[:minutes[:seconds]] -- into the struct tm at edx;
 * past it. */
void tzRule(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    setHigh8(cpu.eax, byte(app, cpu.eax));
    cpu.esi = 0xffffffff;
    if (byte(app, cpu.ebx) == 'J')
    {
        cpu.esi = 1;
        cpu.ebx += 1;
    }
    if (byte(app, cpu.ebx) == 'M')
    {
        ++cpu.ebx;
        cpu.esi = 0;
    }
    cpu.edx = local + 0xc;
    cpu.eax = cpu.ebx;
    put(app, cpu.ecx + 0x20, cpu.esi);
    if (!direct(app, cpu, tzNumber))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = cpu.eax;
    if (cpu.esi != 0)
    {
        cpu.eax = at(app, local + 0xc);
        put(app, cpu.ecx + 0x1c, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, local + 0xc) - 1;
        put(app, cpu.ecx + 0x10, cpu.eax);
        if (byte(app, cpu.edx) == '.')
        {
            cpu.eax = cpu.edx + 1;
            cpu.edx = local + 0xc;
            if (!direct(app, cpu, tzNumber))
                return;
            cpu.esi = cpu.eax;
            cpu.ebx = cpu.eax;
            cpu.eax = at(app, local + 0xc);
            put(app, cpu.ecx + 0xc, cpu.eax);
            if (byte(app, cpu.esi) == '.')
            {
                cpu.edx = local + 0xc;
                cpu.eax = cpu.esi + 1;
                if (!direct(app, cpu, tzNumber))
                    return;
                cpu.ebx = cpu.eax;
                cpu.eax = at(app, local + 0xc);
                put(app, cpu.ecx + 0x18, cpu.eax);
            }
        }
        put(app, cpu.ecx + 0x1c, 0);
    }
    cpu.edx = 2;
    cpu.esi = 0;
    put(app, local + 8, cpu.edx);
    put(app, local, cpu.esi);
    put(app, local + 4, cpu.esi);
    if (byte(app, cpu.ebx) == '/')
    {
        cpu.edx = local + 8;
        cpu.eax = cpu.ebx + 1;
        if (!direct(app, cpu, tzNumber))
            return;
        setHigh8(cpu.edx, byte(app, cpu.eax));
        cpu.ebx = cpu.eax;
        if (reg8(cpu.edx >> 8) == ':')
        {
            cpu.edx = local + 4;
            ++cpu.eax;
            if (!direct(app, cpu, tzNumber))
                return;
            setLow8(cpu.edx, byte(app, cpu.eax));
            cpu.ebx = cpu.eax;
            if (reg8(cpu.edx) == ':')
            {
                cpu.edx = local;
                ++cpu.eax;
                if (!direct(app, cpu, tzNumber))
                    return;
                cpu.ebx = cpu.eax;
            }
        }
    }
    put(app, cpu.ecx, at(app, local));
    put(app, cpu.ecx + 4, at(app, local + 4));
    cpu.eax = at(app, local + 8);
    put(app, cpu.ecx + 8, cpu.eax);
    cpu.eax = cpu.ebx;
    cpu.esp = local + 0x10;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x515b34 (__tzset's without TZ): the zone, daylight saving and the zones'
 * names as GetTimeZoneInformation has them, once. */
void tzWindows(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi})
        push(app, cpu, r);
    cpu.esp -= 0xac;
    const reg32 local = cpu.esp;  // a TIME_ZONE_INFORMATION
    auto back = [&] {
        cpu.esp = local + 0xac;
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    setHigh8(cpu.eax, byte(app, kZoneDone));
    if ((byte(app, kZoneDone) & 1) && (byte(app, kZoneDone) & 2))
        return back();
    setHigh8(cpu.edx, byte(app, kZoneDone) | 2);
    cpu.eax = local;
    push(app, cpu, cpu.eax);
    putByte(app, kZoneDone, reg8(cpu.edx >> 8));
    if (!indirect(app, cpu, at(app, kGetTimeZoneInformation)))
        return;
    if (cpu.eax == 0 || cpu.eax > 2)
        return back();
    if (cpu.eax == 2)
    {
        put(app, kDaylightOn, 1);
        cpu.edx = at(app, local + 0xa8);
        cpu.eax = cpu.edx * 60;
        cpu.ebx = cpu.eax;
        put(app, kDstSeconds, cpu.eax);
        cpu.ebx = 0 - cpu.ebx;
        put(app, kDstSeconds, cpu.ebx);
    }
    else
    {
        cpu.ecx = 0;
        put(app, kDaylightOn, cpu.ecx);
    }
    cpu.esi = at(app, local);
    cpu.edx = at(app, local + 0x54) + cpu.esi;
    cpu.eax = cpu.edx * 60;
    cpu.ebx = 0x80;
    cpu.edx = local + 4;
    put(app, kTimezoneSeconds, cpu.eax);
    cpu.eax = kTzName0;
    if (!direct(app, cpu, wcstombs))
        return;
    if (cpu.eax == 0xffffffff)
    {
        setHigh8(cpu.ebx, 0);
        putByte(app, kTzName0, 0);
    }
    else
    {
        setLow8(cpu.ebx, 0);
        putByte(app, kTzName0 + 0x80, 0);
    }
    cpu.ebx = 0x80;
    cpu.edx = local + 0x58;
    cpu.eax = kTzName1;
    if (!direct(app, cpu, wcstombs))
        return;
    if (cpu.eax == 0xffffffff)
    {
        setHigh8(cpu.ecx, 0);
        putByte(app, kTzName1, 0);
    }
    else
    {
        setLow8(cpu.ecx, 0);
        putByte(app, kTzName1 + 0x80, 0);
    }
    back();
}

/* 0x515ec2: __tzset's after a TZ zone with a daylight name -- its own offset
 * or an hour less, and the rules after commas, the end rule's time made
 * standard time -- in __tzset's frame, ecx where the zone's name ended. */
void tzDaylight(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ebx = 1;
    cpu.edx = kTzName1;
    cpu.eax = at(app, kTimezoneSeconds);
    put(app, kDaylightOn, cpu.ebx);
    cpu.eax -= 3600;
    cpu.ebx = cpu.esp;
    put(app, cpu.esp, cpu.eax);
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, tzName))
        return;
    cpu.ecx = at(app, cpu.esp);
    cpu.esi = at(app, kTimezoneSeconds);
    cpu.edx = cpu.eax;
    cpu.esi -= cpu.ecx;
    setLow8(cpu.ebx, byte(app, cpu.eax));
    put(app, kDstSeconds, cpu.esi);
    if (reg8(cpu.ebx) == ',')
    {
        cpu.edx = kDstStart;
        ++cpu.eax;
        if (!direct(app, cpu, tzRule))
            return;
        cpu.edx = cpu.eax;
    }
    setHigh8(cpu.ebx, byte(app, cpu.edx));
    cpu.esi = at(app, kDstSeconds);
    if (reg8(cpu.ebx >> 8) == ',')
    {
        cpu.eax = cpu.edx + 1;
        cpu.edx = kDstEnd;
        if (!direct(app, cpu, tzRule))
            return;
        cpu.esi = at(app, kDstSeconds);
        const x86::sreg32 seconds = x86::sreg32(cpu.esi);
        cpu.ebx = 60;
        cpu.edi = at(app, kDstEnd + 8) - reg32(seconds / 3600);
        const x86::sreg32 minutes = seconds / 60;
        cpu.eax = reg32(minutes / 60);
        cpu.edx = reg32(minutes % 60);
        cpu.ebp = at(app, kDstEnd + 4) - cpu.edx;
        cpu.edx = reg32(seconds % 60);
        cpu.eax = at(app, kDstEnd) - cpu.edx;
        put(app, kDstEnd + 8, cpu.edi);
        put(app, kDstEnd + 4, cpu.ebp);
        put(app, kDstEnd, cpu.eax);
    }
    put(app, kDstSeconds, cpu.esi);
    cpu.esp += 4;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* __tzset (clib/time/c/tzset.c, 0x515c2c): the zone from TZ -- its name,
 * offset, daylight saving's name and rules -- or from Windows. */
void tzset(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = kTzString;
    if (!direct(app, cpu, getenv))
        return;
    if (cpu.eax == 0)
        return tzWindows(app, cpu);
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 4;
    cpu.esi = at(app, kDstSeconds);
    cpu.edx = 0;
    cpu.ebx = kTimezoneSeconds;
    put(app, kDaylightOn, cpu.edx);
    cpu.edx = kTzName0;
    if (!direct(app, cpu, tzName))
        return;
    cpu.ecx = cpu.eax;
    setHigh8(cpu.eax, byte(app, cpu.eax));
    if (reg8(cpu.eax >> 8) != 0)
        return tzDaylight(app, cpu);
    cpu.esi = at(app, kDstSeconds);
    putByte(app, kTzName1, 0);
    put(app, kDstSeconds, cpu.esi);
    cpu.esp += 4;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4ff410 (__FiniRtns' call): the routine at the pointer at eax, if any,
 * with es its ds. */
void callRoutine(win32::WinApplication* app, x86::CPU& cpu)
{
    pushEs(app, cpu);
    if (at(app, cpu.eax) != 0)
    {
        cpu.esp -= 4;
        putHalf(app, cpu.esp, cpu.ds);  // push ds
        cpu.es = half(app, cpu.esp);    // pop es
        cpu.esp += 4;
        if (!indirect(app, cpu, at(app, cpu.eax)))
            return;
    }
    popEs(app, cpu);
    cpu.esp += 4;
}

/* __FiniRtns (clib/startup/c/initrtns.c, 0x4ff46c): the runtime's
 * finalizers of priority al to dl not yet run, the highest first. */
void FiniRtns(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi})
        push(app, cpu, r);
    pushEs(app, cpu);
    cpu.esi = kFiniEnd;
    setLow8(cpu.ecx, cpu.eax);
    setHigh8(cpu.edx, cpu.edx);
    for (;;)
    {
        cpu.eax = kFiniStart;
        cpu.ebx = cpu.esi;
        setLow8(cpu.edx, cpu.ecx);
        if (cpu.esi > cpu.eax)
            do
            {
                if (byte(app, cpu.eax) != 2)
                {
                    setHigh8(cpu.ecx, byte(app, cpu.eax + 1));
                    if (reg8(cpu.edx) <= reg8(cpu.ecx >> 8))
                    {
                        cpu.ebx = cpu.eax;
                        setLow8(cpu.edx, cpu.ecx >> 8);
                    }
                }
                cpu.eax += 6;
            } while (cpu.eax < kFiniEnd);
        if (cpu.ebx == kFiniEnd)
            break;
        if (reg8(cpu.edx >> 8) >= byte(app, cpu.ebx + 1))
        {
            cpu.eax = cpu.ebx + 2;
            if (!direct(app, cpu, callRoutine))
                return;
        }
        putByte(app, cpu.ebx, 2);
    }
    popEs(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51f11c (__DoneExceptionHandler): the thread's exception record off fs's
 * chain. */
void DoneExceptionHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    cpu.eax = at(app, cpu.eax + 0x54);
    if (cpu.eax != 0)
    {
        cpu.eax = at(app, cpu.eax);
        cpu.edx = 0;
        put(app, cpu.efs + cpu.edx, cpu.eax);
    }
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    put(app, cpu.eax + 0x54, 0);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4ff3d0 (the environment's copies given back): what the start-up code made
 * of it, narrow and wide, to _nfree. */
void freeEnvironment(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi})
        push(app, cpu, r);
    cpu.edx = at(app, kEnvironCopy);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ebx = 0;
        if (!direct(app, cpu, nfree))
            return;
        put(app, kEnvironCopy, cpu.ebx);
    }
    cpu.ecx = at(app, kWideEnvironCopy);
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        cpu.esi = 0;
        if (!direct(app, cpu, nfree))
            return;
        put(app, kWideEnvironCopy, cpu.esi);
    }
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* __exit (clib/startup/c/exit.c, 0x4ff38c): ExitProcess with eax -- the
 * finalizers and the exception record first, unless the start-up got no
 * further than its own -- and, should it come back, the environment's copies
 * given back, as the code after it does. */
void exit_(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ebx = cpu.eax;
    if (at(app, kExitStarted) != 0)
    {
        if (at(app, kExitEarly) != 0)
        {
            cpu.edx = 0xff;
            cpu.eax = 0;
            if (!indirect(app, cpu, at(app, kExitEarly)))
                return;
        }
    }
    else
    {
        if (!direct(app, cpu, DoneExceptionHandler))
            return;
        cpu.edx = 0xf;
        cpu.eax = 0;
        if (!direct(app, cpu, FiniRtns) || !indirect(app, cpu, at(app, kExitDone)))
            return;
    }
    push(app, cpu, cpu.ebx);
    if (!indirect(app, cpu, at(app, kExitProcess)))
        return;
    freeEnvironment(app, cpu);
}

/* _exit (clib/startup/c/exit.c, 0x4dfd30): eax to __exit after the runtime's
 * hooks -- jumped to with edx still pushed. */
void _exit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!indirect(app, cpu, at(app, kExitHook)) || !indirect(app, cpu, at(app, kExitHook2)))
        return;
    if (at(app, kExitUser) != 0 && !indirect(app, cpu, at(app, kExitUser)))
        return;
    cpu.eax = cpu.edx;
    exit_(app, cpu);
}

/* exit (clib/startup/c/exit.c, 0x4dfd04): the finalizers of priority 16 and
 * up, then _exit with eax. */
void exit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.ebx = cpu.eax;
    if (!indirect(app, cpu, at(app, kExitHook)))
        return;
    if (byte(app, kExitStarted) == 0)
    {
        cpu.edx = 0xff;
        cpu.eax = 0x10;
        if (!direct(app, cpu, FiniRtns))
            return;
    }
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, _exit))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* atexit (clib/startup/c/atexit.c, 0x4f7c20): eax to the 32 run at exit; 0,
 * or -1 when they are taken. */
void atexit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = at(app, kAtexitCount);
    if (x86::sreg32(cpu.edx) >= 0x20)
        cpu.eax = 0xffffffff;
    else
    {
        ++cpu.edx;
        put(app, cpu.edx * 4 + kAtexitTable, cpu.eax);
        put(app, kAtexitCount, cpu.edx);
        cpu.eax = 0;
    }
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x524190 (__EnterWVIDEO): a message to a debugger, by its int3, if there
 * is one; 1, or 0. */
void EnterWVIDEO(win32::WinApplication* app, x86::CPU& cpu)
{
    if (byte(app, kDebuggerOn) == 0)
    {
        cpu.eax = 0;
        cpu.esp += 4;
        return;
    }
    cpu.edx &= 0xffff;
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.eax);
    NFS2_ASSERT(false);  // int3
    cpu.eax = 1;
    cpu.esp += 8 + 4;
}

/* 0x511cd0 (__fatal_runtime_error's): the message at eax to stderr's handle,
 * then __exit with edx -- jumped to, its frame still there. */
void fatalWrite(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.esp -= 4;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edx = 0;
    for (;;)
    {
        cpu.ebx = cpu.eax;
        setLow8(cpu.ecx, byte(app, cpu.ebx));
        ++cpu.eax;
        if (reg8(cpu.ecx) == 0)
            break;
        ++cpu.edx;
    }
    push(app, cpu, 0);
    cpu.eax = cpu.esp + 4;
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.edx);
    cpu.eax = at(app, kOsHandleTable);
    push(app, cpu, cpu.edi);
    cpu.edx = at(app, cpu.eax + 8);
    push(app, cpu, cpu.edx);
    if (!indirect(app, cpu, at(app, kWriteFileSlot)))
        return;
    cpu.eax = cpu.esi;
    exit_(app, cpu);
}

/* __fatal_runtime_error (clib/startup/c/fatalerr.c, 0x511d0c): the message at
 * eax to a debugger, or to stderr and out with edx. */
void fatal_runtime_error(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.edx = cpu.ds;
    if (!direct(app, cpu, EnterWVIDEO))
        return;
    if (cpu.eax == 0)
    {
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, fatalWrite))
            return;
    }
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* sleep (clib/process/c/sleepwnt.c, 0x4f0710): SleepEx for eax seconds. */
void sleep(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.eax *= 1000;
    push(app, cpu, 0);
    push(app, cpu, cpu.eax);
    if (!indirect(app, cpu, at(app, kSleepEx)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* getpid (clib/environ/c/gtpidwnt.c, 0x5093c0): GetCurrentProcessId. */
void getpid(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    if (!indirect(app, cpu, at(app, kGetCurrentProcessId)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* stackavail (clib/startup/c/stackwnt.c, 0x4f45c0): how far esp is above the
 * thread's stack's bottom. */
void stackavail(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.esp - at(app, cpu.edx);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* __GRO (clib/startup/a/stkwnt.asm, 0x4f4610): the stack's pages below esp
 * touched, a dword each, for the size on the stack, which it takes off. */
void GRO(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ebx);
    cpu.eax = at(app, cpu.esp + 0xc);
    cpu.ebx = 0xfffffffc;
    do
    {
        put(app, cpu.esp + cpu.ebx, cpu.ebx);
        cpu.ebx -= 0x1000;
        cpu.eax -= 0x1000;
    } while (x86::sreg32(cpu.eax) > 0);
    cpu.ebx = pop(app, cpu);
    cpu.eax = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x4ee15c (__MkTmpFile's digit): eax, 0 to 15, as a hexadecimal digit. */
void hexDigit(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.eax += '0';
    if (x86::sreg32(cpu.eax) > '9')
        cpu.eax += 0x27;
    cpu.esp += 4;
}

/* _fullpath (clib/misc/c/fullpath.c, 0x523610): the path at edx made whole
 * into eax, ebx bytes of it -- 0x104 from _nmalloc for no buffer -- by
 * GetFullPathNameA; "con" as it is, no path or an empty one the current
 * directory.  0 for an error. */
void fullpath(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.esp -= 4;
    const reg32 local = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    auto back = [&] {
        cpu.esp = local + 4;
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0)
    {
        cpu.ebx = 0x104;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, nmalloc))
            return;
        cpu.edi = cpu.eax;
        if (cpu.eax == 0)
        {
            cpu.eax = 5;  // ENOMEM
            if (!direct(app, cpu, set_errno))
                return;
            cpu.eax = 0;
            return back();
        }
        cpu.esi = cpu.eax;
    }
    if (cpu.ecx == 0 || byte(app, cpu.ecx) == 0)
    {
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, getcwd))
            return;
        return back();
    }
    cpu.edx = kConsoleName;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, stricmp))
        return;
    if (cpu.eax == 0)
    {
        if (cpu.ebx < 4)
        {
            cpu.eax = cpu.edi;
            if (!direct(app, cpu, nfree))
                return;
            cpu.eax = 0xe;  // ERANGE
            if (!direct(app, cpu, set_errno))
                return;
            cpu.eax = 0;
            return back();
        }
        cpu.eax = at(app, kConsoleName);
        put(app, cpu.esi, cpu.eax);
        cpu.eax = cpu.esi;
        return back();
    }
    cpu.eax = local;
    for (const reg32 r : {cpu.eax, cpu.esi, cpu.ebx, cpu.ecx})
        push(app, cpu, r);
    if (!indirect(app, cpu, at(app, kGetFullPathNameA)))
        return;
    if (cpu.eax == 0)
    {
        if (!direct(app, cpu, set_errno_nt))
            return;
        cpu.eax = 0;
        return back();
    }
    cpu.eax = cpu.esi;
    back();
}

/* 0x5094e0 (__tmpdir): the directory of temporary files, once: the first of
 * TMP, TEMP, TMPDIR, TEMPDIR that fits, made whole by _fullpath, or the
 * current one; a '\\' after it. */
void tmpdir(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    if (byte(app, kTmpDir) == 0)
    {
        for (cpu.ebx = kTmpVariables; ; cpu.ebx += 4)
        {
            cpu.eax = at(app, cpu.ebx);
            if (byte(app, cpu.eax) == 0)
                break;
            if (!direct(app, cpu, getenv))
                return;
            cpu.edx = cpu.eax;
            if (cpu.eax == 0)
                continue;
            cpu.edi = cpu.eax;
            inlineStrlen(app, cpu);
            if (cpu.ecx > 0x103)
                continue;
            cpu.ebx = 0x103;
            cpu.eax = kTmpDir;
            if (!direct(app, cpu, fullpath))
                return;
            break;
        }
        if (byte(app, kTmpDir) == 0)
        {
            cpu.edx = 0;
            cpu.eax = 0;
            cpu.edi = kTmpDir;
            if (!direct(app, cpu, getcwd))
                return;
            cpu.esi = cpu.eax;
            push(app, cpu, cpu.edi);
            for (;;)
            {
                setLow8(cpu.eax, byte(app, cpu.esi));
                putByte(app, cpu.edi, reg8(cpu.eax));
                if (reg8(cpu.eax) == 0)
                    break;
                setLow8(cpu.eax, byte(app, cpu.esi + 1));
                cpu.esi += 2;
                putByte(app, cpu.edi + 1, reg8(cpu.eax));
                cpu.edi += 2;
                if (reg8(cpu.eax) == 0)
                    break;
            }
            cpu.edi = pop(app, cpu);
        }
        cpu.edi = kTmpDir;
        inlineStrlen(app, cpu);
        cpu.eax = cpu.ecx - 1 + kTmpDir;
        setLow8(cpu.ebx, byte(app, cpu.eax));
        if (reg8(cpu.ebx) != '\\' && reg8(cpu.ebx) != '/')
        {
            ++cpu.eax;
            putByte(app, cpu.eax, '\\');
            ++cpu.eax;
            putByte(app, cpu.eax, 0);
        }
    }
    cpu.eax = kTmpDir;
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* __MkTmpFile (clib/streamio/c/tmputil.c, 0x4ee168): a temporary file's name
 * into eax -- the directory, then t, the process's id in four hexadecimal
 * digits, _, two of edx's, .tmp. */
void MkTmpFile(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 4;
    const reg32 local = cpu.esp;
    cpu.ebp = cpu.eax;
    put(app, local, cpu.edx);
    if (!direct(app, cpu, getpid))
        return;
    cpu.ebx = cpu.eax >> 16 | cpu.eax;
    cpu.edi = cpu.ebp;
    if (!direct(app, cpu, tmpdir))
        return;
    cpu.esi = cpu.eax;
    push(app, cpu, cpu.edi);
    for (;;)
    {
        setLow8(cpu.eax, byte(app, cpu.esi));
        putByte(app, cpu.edi, reg8(cpu.eax));
        if (reg8(cpu.eax) == 0)
            break;
        setLow8(cpu.eax, byte(app, cpu.esi + 1));
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, reg8(cpu.eax));
        cpu.edi += 2;
        if (reg8(cpu.eax) == 0)
            break;
    }
    cpu.edi = pop(app, cpu);
    inlineStrlen(app, cpu);
    cpu.esi = cpu.ecx + cpu.ebp;
    cpu.edx = cpu.esi + 4;
    putByte(app, cpu.esi, 't');
    cpu.ecx = cpu.esi;
    do
    {
        cpu.eax = cpu.ebx & 0xf;
        --cpu.edx;
        if (!direct(app, cpu, hexDigit))
            return;
        cpu.ebx >>= 4;
        putByte(app, cpu.edx + 1, reg8(cpu.eax));
    } while (cpu.edx != cpu.ecx);
    cpu.eax = reg32(x86::sreg32(at(app, local)) >> 4) & 0xf;
    putByte(app, cpu.esi + 5, '_');
    if (!direct(app, cpu, hexDigit))
        return;
    putByte(app, cpu.esi + 6, reg8(cpu.eax));
    cpu.eax = at(app, local) & 0xf;
    if (!direct(app, cpu, hexDigit))
        return;
    putByte(app, cpu.esi + 8, '.');
    putByte(app, cpu.esi + 9, 't');
    putByte(app, cpu.esi + 0xa, 'm');
    putByte(app, cpu.esi + 0xb, 'p');
    putByte(app, cpu.esi + 0xc, 0);
    putByte(app, cpu.esi + 7, reg8(cpu.eax));
    cpu.esp = local + 4;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
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

/* rep movs of `size` bytes each -- 1, 2 or 4 -- from esi to es:edi. */
void repMoveSized(win32::WinApplication* app, x86::CPU& cpu, reg32 size)
{
    const reg32 step = cpu.flags.df ? reg32(0) - size : size;
    while (cpu.ecx != 0)
    {
        if (size == 4)
            put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
        else if (size == 2)
            putHalf(app, cpu.ees + cpu.edi, half(app, cpu.esi));
        else
            putByte(app, cpu.ees + cpu.edi, byte(app, cpu.esi));
        cpu.edi += step;
        cpu.esi += step;
        --cpu.ecx;
    }
}

/* The runtime's inline strcpy: esi to edi, two bytes at a time, edi kept. */
void inlineStrcpy(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    for (;;)
    {
        setLow8(cpu.eax, byte(app, cpu.esi));
        putByte(app, cpu.edi, reg8(cpu.eax));
        if (reg8(cpu.eax) == 0)
            break;
        setLow8(cpu.eax, byte(app, cpu.esi + 1));
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, reg8(cpu.eax));
        cpu.edi += 2;
        if (reg8(cpu.eax) == 0)
            break;
    }
    cpu.edi = pop(app, cpu);
}

/* The miniheap that has the block at edi, as __HeapManager_expand finds it:
 * from __nheapbeg on, the last one if none; for another segment than ds,
 * esi as it was. */
void heapOf(win32::WinApplication* app, x86::CPU& cpu, reg16 segment)
{
    cpu.edx = cpu.ds;
    cpu.eax = kHeapFirstBlock;
    if (reg16(cpu.edx) != segment)
        return;
    cpu.esi = at(app, kHeapFirstBlock);
    while (at(app, cpu.esi + 8) != 0)
    {
        if (cpu.esi <= cpu.edi)
        {
            cpu.eax = at(app, cpu.esi) + cpu.esi;
            if (cpu.eax > cpu.edi)
                return;
        }
        cpu.esi = at(app, cpu.esi + 8);
    }
}

}  // namespace

/* _mbslen (clib/mbyte/c/mbslen.c, 0x518050): the characters in the string at
 * eax. */
void mbslen(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.ebx = 0;
    for (;;)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, mbterm))
            return;
        if (cpu.eax != 0)
            break;
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, mbsinc))
            return;
        ++cpu.ebx;
        cpu.edx = cpu.eax;
    }
    cpu.eax = cpu.ebx;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* mbtowc (clib/mbyte/c/mbtowc.c, 0x523c00): the character at edx, at most
 * ebx bytes of it, by MultiByteToWideChar into the word at eax; its bytes,
 * 0 for none, -1 for one it cannot make. */
void mbtowc(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.esi = cpu.eax;
    auto back = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.edx == 0)
    {
        cpu.eax = 0;
        return back();
    }
    if (cpu.ebx == 0)
    {
        cpu.eax = 0xffffffff;
        return back();
    }
    if (byte(app, cpu.edx) == 0)
    {
        if (cpu.esi != 0)
            putHalf(app, cpu.esi, 0);
        cpu.eax = 0;
        return back();
    }
    if (multibyte(app))
    {
        cpu.eax = byte(app, kMbcsLead + byte(app, cpu.edx)) & 1;
        if (cpu.eax != 0 && byte(app, cpu.edx + 1) == 0)
        {
            cpu.eax = 0xffffffff;
            return back();
        }
    }
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, mbclen))
        return;
    push(app, cpu, 1);
    cpu.ecx = cpu.ebp - 4;
    cpu.edi = cpu.eax;
    push(app, cpu, cpu.ecx);
    if (cpu.ebx < cpu.eax)
        cpu.eax = cpu.ebx;
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.edx);
    push(app, cpu, 8);  // MB_ERR_INVALID_CHARS
    cpu.eax = at(app, kCodePage);
    push(app, cpu, cpu.eax);
    if (!indirect(app, cpu, at(app, kMultiByteToWideChar)))
        return;
    if (cpu.eax == 0)
    {
        cpu.eax = 0xffffffff;
        return back();
    }
    if (cpu.esi != 0)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        putHalf(app, cpu.esi, reg16(cpu.eax));
    }
    cpu.eax = cpu.edi;
    back();
}

/* mbstowcs (clib/mbyte/c/mbstowcs.c, 0x518080): the string at edx into the
 * wide string at eax, at most ebx characters -- or, for no buffer, how many
 * it would take; -1 for one mbtowc cannot make. */
void mbstowcs(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.ebp = cpu.ebx;
    cpu.edi = 0;
    auto back = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax != 0)
    {
        while (cpu.ebp != 0)
        {
            if (byte(app, cpu.ecx) == 0)
            {
                putHalf(app, cpu.esi, 0);
                break;
            }
            cpu.ebx = 2;
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, mbtowc))
                return;
            if (cpu.eax == 0xffffffff)
                return back();
            cpu.eax = cpu.ecx;
            --cpu.ebp;
            if (!direct(app, cpu, mbsinc))
                return;
            cpu.esi += 2;
            ++cpu.edi;
            cpu.ecx = cpu.eax;
        }
        cpu.eax = cpu.edi;
        return back();
    }
    cpu.esi = 2;
    while (byte(app, cpu.ecx) != 0)
    {
        cpu.ebx = cpu.esi;
        cpu.edx = cpu.ecx;
        cpu.eax = 0;
        if (!direct(app, cpu, mbtowc))
            return;
        if (cpu.eax == 0xffffffff)
            return back();
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, mbsinc))
            return;
        ++cpu.edi;
        cpu.ecx = cpu.eax;
    }
    cpu.eax = cpu.edi;
    back();
}

/* 0x525300 (wcslen): the characters in the wide string at eax. */
void wcslen(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (half(app, cpu.eax) != 0)
        do
        {
            cpu.ecx = (cpu.ecx & 0xffff0000) | half(app, cpu.eax + 2);
            cpu.eax += 2;
        } while ((cpu.ecx & 0xffff) != 0);
    cpu.eax = reg32(x86::sreg32(cpu.eax - cpu.edx) >> 1);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525320 (memcpy): ebx bytes from edx to eax; eax back. */
void memcpy(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.ecx = cpu.ebx;
    cpu.esi = cpu.edx;
    cpu.edi = cpu.eax;
    inlineMemcpy(app, cpu);
    cpu.eax = cpu.edi;
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525350 (wcscpy): the wide string at edx to eax; eax back. */
void wcscpy(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ecx = cpu.eax;
    do
    {
        cpu.eax += 2;
        cpu.ebx = (cpu.ebx & 0xffff0000) | half(app, cpu.edx);
        cpu.edx += 2;
        putHalf(app, cpu.eax - 2, reg16(cpu.ebx));
    } while ((cpu.ebx & 0xffff) != 0);
    cpu.eax = cpu.ecx;
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525370 (towupper, as the environment's functions have it): ax's ASCII
 * small letters as capitals. */
void towupper(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    const reg16 c = reg16(cpu.eax);
    if (c >= 'a' && c <= 'z')
        cpu.eax -= 0x20;
    cpu.esp += 4;
}

/* 0x518640 (memmove): ebx bytes from edx to eax, backwards -- words, then a
 * byte -- where they overlap the wrong way; eax kept. */
void memmove(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.ecx = cpu.ebx;
    if (cpu.edx != cpu.eax)
    {
        if (cpu.edx < cpu.eax && cpu.edx + cpu.ebx > cpu.eax)
        {
            cpu.esi = cpu.edx + cpu.ebx - 1;
            cpu.edi = cpu.eax + cpu.ebx - 1;
            cpu.edx = cpu.ds;
            pushEs(app, cpu);
            cpu.es = reg16(cpu.edx);
            cpu.flags.df = 1;
            --cpu.esi;
            --cpu.edi;
            const reg32 odd = cpu.ecx & 1;
            cpu.ecx >>= 1;
            repMoveSized(app, cpu, 2);
            cpu.ecx = odd;
            ++cpu.esi;
            ++cpu.edi;
            repMoveSized(app, cpu, 1);
            popEs(app, cpu);
            cpu.flags.df = 0;
        }
        else
        {
            cpu.esi = cpu.edx;
            cpu.edi = cpu.eax;
            cpu.edx = cpu.ds;
            pushEs(app, cpu);
            cpu.es = reg16(cpu.edx);
            push(app, cpu, cpu.ecx);
            cpu.ecx >>= 2;
            repMoveSized(app, cpu, 4);
            cpu.ecx = pop(app, cpu) & 3;
            repMoveSized(app, cpu, 1);
            popEs(app, cpu);
        }
    }
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525380 (_msize): the block at eax's size, as its header has it. */
void msize(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = (at(app, cpu.eax - 4) & 0xfffffffe) - 4;
    cpu.esp += 4;
}

/* 0x5253cc (__HeapManager_expand): the block at edx in the heap of segment ax
 * made ebx bytes, from the free block after it or by giving its end back;
 * how much more it still needs into the dword at ecx.  0, 1 for a block
 * after it in use, 2 for the heap's end. */
void HeapManager_expand(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    put(app, local + 0xc, cpu.eax);
    cpu.edi = cpu.edx;
    cpu.ebp = cpu.ecx;
    auto back = [&] {
        cpu.eax = cpu.ebx;
        cpu.esp = local + 0x10;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.eax = cpu.ebx + 0xb;
    setLow8(cpu.eax, cpu.eax & 0xf8);
    if (cpu.eax < cpu.ebx)
        cpu.eax = 0xffffffff;
    else if (cpu.eax < 0x10)
        cpu.eax = 0x10;
    cpu.edx = cpu.edi - 4;
    put(app, local, cpu.edx);
    cpu.edx = at(app, cpu.edx);
    setLow8(cpu.edx, cpu.edx & 0xfe);
    if (cpu.eax <= cpu.edx)
    {
        // Smaller: what it no longer needs freed, if it is a block's worth.
        cpu.edx -= cpu.eax;
        if (cpu.edx >= 0x10)
        {
            cpu.ebx = cpu.eax;
            cpu.ecx = at(app, local);
            setLow8(cpu.ebx, cpu.ebx | 1);
            setLow8(cpu.edx, cpu.edx | 1);
            put(app, cpu.ecx, cpu.ebx);
            cpu.ebx = cpu.ecx + cpu.eax;
            put(app, local, cpu.ebx);
            put(app, cpu.ebx, cpu.edx);
            cpu.ebx = at(app, local + 0xc);
            heapOf(app, cpu, reg16(cpu.ebx));
            put(app, cpu.esi + 0x18, at(app, cpu.esi + 0x18) + 1);
            cpu.eax = at(app, local) + 4;
            if (!direct(app, cpu, nfree))
                return;
        }
        cpu.ebx = 0;
        return back();
    }
    cpu.ecx = at(app, local);
    cpu.eax -= cpu.edx;
    cpu.ecx += cpu.edx;
    for (;;)
    {
        put(app, cpu.ebp, cpu.eax);
        cpu.ebx = at(app, cpu.ecx);
        if (cpu.ebx == 0xffffffff)
        {
            cpu.ebx = 2;
            return back();
        }
        if (cpu.ebx & 1)
        {
            cpu.ebx = 1;
            return back();
        }
        cpu.eax = at(app, cpu.ecx + 8);
        put(app, local + 8, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 4);
        put(app, local + 4, cpu.eax);
        heapOf(app, cpu, half(app, local + 0xc));
        cpu.edx = at(app, cpu.esi + 0xc);
        if (cpu.ecx == cpu.edx)
        {
            cpu.eax = at(app, cpu.edx + 4);
            put(app, cpu.esi + 0xc, cpu.eax);
        }
        if (cpu.ebx >= at(app, cpu.ebp))
        {
            cpu.eax = cpu.ebx - at(app, cpu.ebp);
            if (cpu.eax >= 0x10)
            {
                // Split: the rest of the free block stays free after it.
                cpu.edx = at(app, cpu.ebp);
                cpu.ecx += cpu.edx;
                put(app, cpu.ecx, cpu.eax);
                put(app, cpu.ecx + 4, at(app, local + 4));
                put(app, cpu.ecx + 8, at(app, local + 8));
                put(app, at(app, local + 4) + 8, cpu.ecx);
                put(app, at(app, local + 8) + 4, cpu.ecx);
                cpu.edx = at(app, local);
                cpu.eax = at(app, cpu.ebp);
                cpu.ecx = at(app, cpu.edx) + cpu.eax;
                cpu.ebx = 0;
                setHigh8(cpu.eax, 0);
                put(app, cpu.edx, cpu.ecx);
                putByte(app, kHeapClean, 0);
                return back();
            }
        }
        // All of the free block: off the free list, onto the block.
        cpu.eax = at(app, local + 4);
        cpu.edx = at(app, local + 8);
        put(app, cpu.eax + 8, cpu.edx);
        cpu.eax = cpu.edx;
        cpu.edx = at(app, local + 4);
        put(app, cpu.eax + 4, cpu.edx);
        cpu.eax = at(app, local);
        put(app, cpu.eax, at(app, cpu.eax) + cpu.ebx);
        cpu.eax = at(app, cpu.esi + 0x1c) - 1;
        setLow8(cpu.edx, 0);
        put(app, cpu.esi + 0x1c, cpu.eax);
        putByte(app, kHeapClean, 0);
        cpu.edx = at(app, cpu.ebp);
        if (cpu.ebx >= cpu.edx)
        {
            cpu.ebx = 0;
            return back();
        }
        cpu.eax = cpu.edx - cpu.ebx;
        cpu.ecx += cpu.ebx;
    }
}

/* 0x525578 (_nexpand's way out when it could not): the heap's lock given
 * back, 0, in _nexpand's frame. */
void nexpandFailed(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!indirect(app, cpu, at(app, kHeapUnlock)))
        return;
    cpu.eax = 0;
    cpu.esp += 4;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* _nexpand (clib/heap/c/nexpand.c, 0x525390): the block at eax made edx bytes
 * where it is, under the heap's lock; the block, or 0. */
void nexpand(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi})
        push(app, cpu, r);
    cpu.esp -= 4;
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    if (!indirect(app, cpu, at(app, kHeapLock)))
        return;
    cpu.edx = cpu.ds;
    cpu.eax = cpu.edx & 0xffff;
    cpu.ecx = cpu.esp;
    cpu.edx = cpu.esi;
    if (!direct(app, cpu, HeapManager_expand))
        return;
    if (cpu.eax != 0)
        return nexpandFailed(app, cpu);
    if (!indirect(app, cpu, at(app, kHeapUnlock)))
        return;
    cpu.eax = cpu.esi;
    cpu.esp += 4;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* _nrealloc (clib/heap/c/nrealloc.c, 0x5185c0): the block at eax made edx
 * bytes -- where it is if it can, else a new one with it copied -- _nmalloc's
 * for none, _nfree's for 0 bytes; the block, or 0. */
void nrealloc(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    auto back = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, nmalloc))
            return;
        return back();
    }
    if (cpu.edx == 0)
    {
        if (!direct(app, cpu, nfree))
            return;
        cpu.eax = 0;
        return back();
    }
    if (!direct(app, cpu, msize))
        return;
    cpu.esi = cpu.eax;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, nexpand))
        return;
    cpu.ebp = cpu.eax;
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, nmalloc))
            return;
        cpu.ebp = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.ecx = cpu.esi;
            cpu.edi = cpu.eax;
            cpu.esi = cpu.ebx;
            inlineMemcpy(app, cpu);
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, nfree))
                return;
        }
        else
        {
            cpu.edx = cpu.esi;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, nexpand))
                return;
        }
    }
    cpu.eax = cpu.ebp;
    back();
}

/* 0x4f8f38 (__findenv): the variable named at eax in _environ, its name
 * matched as capitals: its index + 1 -- or, with no value (edx 0), taken out
 * of _environ and __env_mask, the runtime's string freed -- or, not there,
 * minus how many there are. */
void findenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    put(app, local, cpu.eax);
    put(app, local + 8, cpu.edx);
    cpu.eax = at(app, kEnviron);
    put(app, local + 4, cpu.eax);
    auto back = [&] {
        cpu.esp = local + 0x10;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    for (;;)
    {
        cpu.eax = at(app, at(app, local + 4));
        put(app, local + 0xc, cpu.eax);
        if (cpu.eax == 0)
            break;
        cpu.ebp = at(app, local);
        for (;;)
        {
            cpu.eax = at(app, local + 0xc);
            if (byte(app, cpu.eax) == '=' && byte(app, cpu.ebp) == 0)
            {
                cpu.esi = reg32(x86::sreg32(at(app, local + 4) - at(app, kEnviron)) >> 2);
                cpu.edi = at(app, local + 8);
                if (cpu.edi != 0)
                {
                    cpu.eax = cpu.esi + 1;
                    return back();
                }
                cpu.edi = at(app, at(app, local + 4));
                cpu.ecx = at(app, local + 4);
                if (cpu.edi != 0)
                    do
                    {
                        cpu.eax = at(app, cpu.ecx + 4);
                        put(app, cpu.ecx, cpu.eax);
                        cpu.eax = at(app, cpu.ecx + 4);
                        cpu.ecx += 4;
                    } while (cpu.eax != 0);
                cpu.edx = at(app, kEnvMask);
                if (cpu.edx != 0)
                {
                    cpu.eax = cpu.edx;
                    if (byte(app, cpu.esi + cpu.eax) != 0)
                    {
                        cpu.eax = cpu.edi;
                        if (!direct(app, cpu, nfree))
                            return;
                    }
                    cpu.ebx = at(app, kEnviron);
                    cpu.edi = cpu.ecx - cpu.ebx;
                    cpu.edx = at(app, kEnvMask);
                    cpu.edi = reg32(x86::sreg32(cpu.edi) >> 2);
                    cpu.eax = cpu.ecx;
                    cpu.ebx = cpu.edi;
                    if (!direct(app, cpu, memmove))
                        return;
                    put(app, kEnvMask, cpu.ecx);
                    if (x86::sreg32(cpu.esi) < x86::sreg32(cpu.edi))
                    {
                        cpu.ecx += cpu.esi;
                        do
                        {
                            ++cpu.ecx;
                            setLow8(cpu.eax, byte(app, cpu.ecx));
                            ++cpu.esi;
                            putByte(app, cpu.ecx - 1, reg8(cpu.eax));
                        } while (x86::sreg32(cpu.esi) < x86::sreg32(cpu.edi));
                    }
                }
            }
            cpu.edx = at(app, local + 0xc);
            cpu.eax = byte(app, cpu.edx);
            if (!direct(app, cpu, toupper))
                return;
            cpu.edx = cpu.eax;
            cpu.eax = byte(app, cpu.ebp);
            if (!direct(app, cpu, toupper))
                return;
            if (cpu.edx != cpu.eax || byte(app, cpu.ebp) == 0)
                break;
            cpu.ecx = at(app, local + 0xc) + 1;
            ++cpu.ebp;
            put(app, local + 0xc, cpu.ecx);
        }
        put(app, local + 4, at(app, local + 4) + 4);
    }
    cpu.ebx = at(app, local + 4);
    cpu.eax = reg32(x86::sreg32(at(app, kEnviron) - cpu.ebx) >> 2);
    back();
}

/* __setenv (clib/environ/c/setenv.c, 0x4f8ce8): the variable named at eax
 * set to the string at edx in _environ -- replaced if ebx says so, taken out
 * for none -- as a string of the runtime's, _environ and __env_mask grown as
 * it takes; 0, or -1. */
void setenv_(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    put(app, local + 4, cpu.eax);
    put(app, local + 8, cpu.edx);
    auto back = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = local + 0x10;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0 || byte(app, cpu.eax) == 0)
        return back(0xffffffff);
    cpu.eax = at(app, kEnviron);
    put(app, local + 0xc, cpu.eax);
    if (cpu.eax == 0)
    {
        if (at(app, local + 8) == 0)
            return back(cpu.eax);
        cpu.eax = 9;
        if (!direct(app, cpu, nmalloc))
            return;
        put(app, local + 0xc, cpu.eax);
        if (cpu.eax == 0)
            return back(0xffffffff);
        put(app, cpu.eax, 0);
        put(app, kEnviron, cpu.eax);
        put(app, cpu.eax + 4, 0);
        cpu.eax += 8;
        cpu.ebp = 0;
        put(app, kEnvMask, cpu.eax);
    }
    else
    {
        cpu.edx = at(app, local + 8);
        cpu.eax = at(app, local + 4);
        cpu.esi = at(app, local + 8);
        if (!direct(app, cpu, findenv))
            return;
        if (cpu.esi == 0)
            return back(0);
        if (x86::sreg32(cpu.eax) > 0)
        {
            if (cpu.ebx == 0)
                return back(0);
            cpu.ebp = cpu.eax - 1;
        }
        else
        {
            // Not there: _environ one longer, __env_mask after it.
            cpu.eax = 0 - cpu.eax;
            cpu.ebp = cpu.eax;
            cpu.edi = at(app, kEnvMask);
            cpu.ecx = cpu.ebp * 4;
            ++cpu.eax;
            cpu.ebx = cpu.ecx + 8;
            put(app, local, cpu.eax);
            cpu.eax += cpu.ebx;
            if (cpu.edi == 0)
            {
                if (!direct(app, cpu, nmalloc))
                    return;
                cpu.edx = cpu.eax;
                put(app, local + 0xc, cpu.eax);
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.esi = at(app, kEnviron);
                cpu.edi = cpu.eax;
                inlineMemcpy(app, cpu);
                cpu.edx += cpu.ebx;
                cpu.ebx = at(app, local);
                put(app, kEnvMask, cpu.edx);
                cpu.eax = cpu.edx;
                cpu.edx = 0;
                if (!direct(app, cpu, memset))
                    return;
            }
            else
            {
                cpu.edx = cpu.eax;
                cpu.eax = at(app, local + 0xc);
                if (!direct(app, cpu, nrealloc))
                    return;
                put(app, local + 0xc, cpu.eax);
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.edx = at(app, kEnvMask);
                cpu.ecx = cpu.eax + cpu.ebx;
                cpu.ebx = cpu.ebp;
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, memmove))
                    return;
                put(app, kEnvMask, cpu.ecx);
            }
            cpu.edx = at(app, local + 0xc);
            cpu.eax = cpu.ebp * 4 + cpu.edx;
            put(app, cpu.eax + 4, 0);
            cpu.eax = at(app, kEnvMask);
            put(app, kEnviron, cpu.edx);
            putByte(app, cpu.eax + cpu.ebp, 0);
        }
    }
    // name=value into a string of the runtime's: the old one grown, or anew.
    cpu.edi = at(app, local + 4);
    inlineStrlen(app, cpu);
    cpu.eax = at(app, kEnvMask);
    setHigh8(cpu.edx, byte(app, cpu.eax + cpu.ebp));
    cpu.ebx = cpu.ecx;
    if (reg8(cpu.edx >> 8) != 0)
    {
        cpu.ecx = at(app, local + 0xc);
        cpu.esi = at(app, cpu.ebp * 4 + cpu.ecx);
    }
    else
        cpu.esi = 0;
    cpu.edi = at(app, local + 8);
    inlineStrlen(app, cpu);
    cpu.edx = cpu.ebx + cpu.ecx + 2;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, nrealloc))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
        return back(0xffffffff);
    cpu.esi = at(app, local + 4);
    cpu.ecx = cpu.ebx;
    cpu.edi = cpu.eax;
    inlineMemcpy(app, cpu);
    putByte(app, cpu.edx + cpu.ebx, '=');
    ++cpu.ebx;
    cpu.esi = at(app, local + 8);
    cpu.edi = cpu.edx + cpu.ebx;
    inlineStrcpy(app, cpu);
    cpu.esi = at(app, local + 0xc);
    cpu.eax = cpu.ebp * 4 + cpu.esi;
    put(app, cpu.eax, cpu.edx);
    cpu.eax = at(app, kEnvMask);
    putByte(app, cpu.eax + cpu.ebp, 1);
    back(0);
}

/* 0x518498 (__wfindenv): __findenv's for _wenviron. */
void wfindenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    put(app, local + 4, cpu.eax);
    put(app, local + 8, cpu.edx);
    cpu.eax = at(app, kWideEnviron);
    put(app, local, cpu.eax);
    auto back = [&] {
        cpu.esp = local + 0x10;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    for (;;)
    {
        cpu.eax = at(app, at(app, local));
        put(app, local + 0xc, cpu.eax);
        if (cpu.eax == 0)
            break;
        cpu.ebp = at(app, local + 4);
        for (;;)
        {
            cpu.eax = at(app, local + 0xc);
            if (half(app, cpu.eax) == '=' && half(app, cpu.ebp) == 0)
            {
                cpu.esi = reg32(x86::sreg32(at(app, local) - at(app, kWideEnviron)) >> 2);
                cpu.edi = at(app, local + 8);
                if (cpu.edi != 0)
                {
                    cpu.eax = cpu.esi + 1;
                    return back();
                }
                cpu.edi = at(app, at(app, local));
                cpu.ecx = at(app, local);
                if (cpu.edi != 0)
                    do
                    {
                        cpu.eax = at(app, cpu.ecx + 4);
                        put(app, cpu.ecx, cpu.eax);
                        cpu.eax = at(app, cpu.ecx + 4);
                        cpu.ecx += 4;
                    } while (cpu.eax != 0);
                cpu.edx = at(app, kEnvMask);
                if (cpu.edx != 0)
                {
                    cpu.eax = cpu.edx;
                    if (byte(app, cpu.esi + cpu.eax) != 0)
                    {
                        cpu.eax = cpu.edi;
                        if (!direct(app, cpu, nfree))
                            return;
                    }
                    cpu.ebx = at(app, kWideEnviron);
                    cpu.edi = cpu.ecx - cpu.ebx;
                    cpu.edx = at(app, kEnvMask);
                    cpu.edi = reg32(x86::sreg32(cpu.edi) >> 2);
                    cpu.eax = cpu.ecx;
                    cpu.ebx = cpu.edi;
                    if (!direct(app, cpu, memmove))
                        return;
                    put(app, kEnvMask, cpu.ecx);
                    if (x86::sreg32(cpu.esi) < x86::sreg32(cpu.edi))
                    {
                        cpu.ecx += cpu.esi;
                        do
                        {
                            ++cpu.ecx;
                            setLow8(cpu.eax, byte(app, cpu.ecx));
                            ++cpu.esi;
                            putByte(app, cpu.ecx - 1, reg8(cpu.eax));
                        } while (x86::sreg32(cpu.esi) < x86::sreg32(cpu.edi));
                    }
                }
            }
            cpu.edx = at(app, local + 0xc);
            cpu.eax = half(app, cpu.edx);
            if (!direct(app, cpu, towupper))
                return;
            cpu.edx = cpu.eax;
            cpu.eax = half(app, cpu.ebp);
            if (!direct(app, cpu, towupper))
                return;
            if (reg16(cpu.edx) != reg16(cpu.eax) || half(app, cpu.ebp) == 0)
                break;
            cpu.ecx = at(app, local + 0xc) + 2;
            cpu.ebp += 2;
            put(app, local + 0xc, cpu.ecx);
        }
        put(app, local, at(app, local) + 4);
    }
    cpu.ebx = at(app, local);
    cpu.eax = reg32(x86::sreg32(at(app, kWideEnviron) - cpu.ebx) >> 2);
    back();
}

/* 0x52c300 (__wputenv's find): the wide "name=value" at eax's name in
 * _wenviron, matched as capitals: its index + 1, or -- edx 0 -- taken out, or
 * minus how many there are. */
void wputenvFind(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.edi = cpu.eax;
    cpu.ebp = cpu.edx;
    auto back = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    for (cpu.esi = at(app, kWideEnviron); (cpu.edx = at(app, cpu.esi)) != 0; cpu.esi += 4)
    {
        cpu.ecx = (cpu.ecx & 0xffff0000) | half(app, cpu.edi);
        cpu.ebx = cpu.edi;
        if ((cpu.ecx & 0xffff) == 0)
            continue;
        for (;;)
        {
            cpu.eax = half(app, cpu.edx);
            if (!direct(app, cpu, towupper))
                return;
            cpu.ecx = cpu.eax;
            cpu.eax = half(app, cpu.ebx);
            if (!direct(app, cpu, towupper))
                return;
            if (reg16(cpu.ecx) != reg16(cpu.eax))
                break;
            if (half(app, cpu.edx) == '=')
            {
                cpu.edx = at(app, kWideEnviron);
                cpu.edi = reg32(x86::sreg32(cpu.esi - cpu.edx) >> 2);
                if (cpu.ebp == 0)
                {
                    cpu.eax = cpu.edi + 1;
                    return back();
                }
                cpu.edx = at(app, cpu.esi);
                cpu.ecx = cpu.esi;
                if (cpu.edx != 0)
                    do
                    {
                        cpu.eax = at(app, cpu.ecx + 4);
                        put(app, cpu.ecx, cpu.eax);
                        cpu.ebx = at(app, cpu.ecx + 4);
                        cpu.ecx += 4;
                    } while (cpu.ebx != 0);
                cpu.esi = at(app, kEnvMask);
                if (cpu.esi != 0)
                {
                    cpu.eax = cpu.esi;
                    if (byte(app, cpu.edi + cpu.eax) != 0)
                    {
                        cpu.eax = cpu.edx;
                        if (!direct(app, cpu, nfree))
                            return;
                    }
                    cpu.ebp = at(app, kWideEnviron);
                    cpu.esi = cpu.ecx - cpu.ebp;
                    cpu.edx = at(app, kEnvMask);
                    cpu.esi = reg32(x86::sreg32(cpu.esi) >> 2);
                    cpu.eax = cpu.ecx;
                    cpu.ebx = cpu.esi;
                    if (!direct(app, cpu, memmove))
                        return;
                    put(app, kEnvMask, cpu.ecx);
                    if (x86::sreg32(cpu.edi) < x86::sreg32(cpu.esi))
                        do
                        {
                            cpu.eax = cpu.edi + cpu.ecx + 1;
                            setLow8(cpu.edx, byte(app, cpu.eax));
                            ++cpu.edi;
                            putByte(app, cpu.eax - 1, reg8(cpu.edx));
                        } while (x86::sreg32(cpu.edi) < x86::sreg32(cpu.esi));
                }
                cpu.eax = 0;
                return back();
            }
            cpu.edx += 2;
            cpu.eax = (cpu.eax & 0xffff0000) | half(app, cpu.ebx + 2);
            cpu.ebx += 2;
            if (reg16(cpu.eax) == 0)
                break;
        }
    }
    cpu.eax = reg32(x86::sreg32(at(app, kWideEnviron) - cpu.esi) >> 2);
    back();
}

/* 0x52c178 (__wputenv): the wide "name=value" at eax into _wenviron as it is
 * -- "name=" takes it out -- _wenviron and __env_mask grown as it takes; 0,
 * or -1. */
void wputenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 8;
    const reg32 local = cpu.esp;
    cpu.edi = cpu.eax;
    auto back = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = local + 8;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0)
        return back(0xffffffff);
    if (half(app, cpu.eax) != 0)
    {
        cpu.edx = cpu.eax + 2;
        while (half(app, cpu.edx) != 0 && half(app, cpu.edx) != '=')
        {
            cpu.esi = (cpu.esi & 0xffff0000) | half(app, cpu.edx + 2);
            cpu.edx += 2;
            if ((cpu.esi & 0xffff) == 0)
                break;
        }
    }
    if (half(app, cpu.edx) == 0)
        return back(0xffffffff);
    setLow8(cpu.eax, half(app, cpu.edx + 2) == 0 ? 1 : 0);
    cpu.ebx = 0;
    cpu.ecx = at(app, kWideEnviron);
    setLow8(cpu.ebx, cpu.eax);
    if (cpu.ecx == 0)
    {
        if (cpu.ebx != 0)
            return back(0);
        cpu.eax = 9;
        if (!direct(app, cpu, nmalloc))
            return;
        cpu.ecx = cpu.eax;
        if (cpu.eax == 0)
            return back(0xffffffff);
        put(app, kWideEnviron, cpu.eax);
        cpu.eax += 8;
        put(app, cpu.eax - 8, cpu.ebx);
        cpu.esi = 0;
        put(app, cpu.eax - 4, cpu.ebx);
        put(app, kEnvMask, cpu.eax);
    }
    else
    {
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, wputenvFind))
            return;
        if (cpu.ebx != 0)
            return back(0);
        if (x86::sreg32(cpu.eax) > 0)
            cpu.esi = cpu.eax - 1;
        else
        {
            cpu.eax = 0 - cpu.eax;
            cpu.ebp = at(app, kEnvMask);
            cpu.esi = cpu.eax;
            ++cpu.eax;
            cpu.ebx = cpu.esi * 4;
            put(app, local, cpu.eax);
            cpu.eax = cpu.ebx + 8;
            cpu.edx = at(app, local);
            put(app, local + 4, cpu.eax);
            cpu.eax += cpu.edx;
            if (cpu.ebp == 0)
            {
                if (!direct(app, cpu, nmalloc))
                    return;
                cpu.ebp = cpu.eax;
                cpu.ecx = cpu.eax;
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.edx = at(app, kWideEnviron);
                if (!direct(app, cpu, memcpy))
                    return;
                cpu.eax = at(app, local + 4);
                cpu.ebx = at(app, local);
                cpu.eax += cpu.ebp;
                cpu.edx = 0;
                put(app, kEnvMask, cpu.eax);
                if (!direct(app, cpu, memset))
                    return;
            }
            else
            {
                cpu.edx = cpu.eax;
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, nrealloc))
                    return;
                cpu.ecx = cpu.eax;
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.ebp = at(app, local + 4);
                cpu.edx = at(app, kEnvMask);
                cpu.ebp += cpu.eax;
                cpu.ebx = cpu.esi;
                cpu.eax = cpu.ebp;
                if (!direct(app, cpu, memmove))
                    return;
                put(app, kEnvMask, cpu.ebp);
            }
            put(app, kWideEnviron, cpu.ecx);
            put(app, cpu.ecx + cpu.esi * 4 + 4, 0);
        }
    }
    cpu.eax = at(app, kEnvMask);
    put(app, cpu.ecx + cpu.esi * 4, cpu.edi);
    putByte(app, cpu.esi + cpu.eax, 0);
    back(0);
}

/* 0x5252a0 (__create_wide_environment): _wenviron made from _environ, a
 * string at a time by mbstowcs into a block of _nmalloc's. */
void create_wide_environment(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi})
        push(app, cpu, r);
    cpu.esi = at(app, kEnviron);
    if (cpu.esi != 0)
        for (;;)
        {
            cpu.edx = at(app, cpu.esi);
            cpu.esi += 4;
            if (cpu.edx == 0)
                break;
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, mbslen))
                return;
            cpu.ebx = cpu.eax + 1;
            cpu.eax = cpu.ebx * 2;
            if (!direct(app, cpu, nmalloc))
                return;
            cpu.ecx = cpu.eax;
            if (cpu.eax == 0)
                continue;
            if (!direct(app, cpu, mbstowcs))
                return;
            const bool made = cpu.eax != 0xffffffff;
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, made ? wputenv : nfree))
                return;
        }
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* __wsetenv (clib/environ/c/wsetenv.c, 0x518290): __setenv's for _wenviron,
 * made from _environ first if there is none. */
void wsetenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x14;
    const reg32 local = cpu.esp;
    cpu.ebp = cpu.eax;
    cpu.edi = cpu.edx;
    auto back = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = local + 0x14;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.eax == 0 || half(app, cpu.eax) == 0)
        return back(0xffffffff);
    if (at(app, kWideEnviron) == 0 && !direct(app, cpu, create_wide_environment))
        return;
    cpu.esi = at(app, kWideEnviron);
    if (cpu.esi == 0)
    {
        if (cpu.edi == 0)
            return back(0);
        cpu.eax = 9;
        if (!direct(app, cpu, nmalloc))
            return;
        cpu.esi = cpu.eax;
        if (cpu.eax == 0)
            return back(0xffffffff);
        put(app, cpu.eax, 0);
        put(app, kWideEnviron, cpu.eax);
        put(app, cpu.eax + 4, 0);
        cpu.eax += 8;
        cpu.ecx = 0;
        put(app, kEnvMask, cpu.eax);
    }
    else
    {
        cpu.edx = cpu.edi;
        cpu.eax = cpu.ebp;
        if (!direct(app, cpu, wfindenv))
            return;
        if (cpu.edi == 0)
            return back(0);
        if (x86::sreg32(cpu.eax) > 0)
        {
            if (cpu.ebx == 0)
                return back(0);
            cpu.ecx = cpu.eax - 1;
        }
        else
        {
            cpu.eax = 0 - cpu.eax;
            cpu.ecx = cpu.eax;
            ++cpu.eax;
            cpu.ebx = cpu.ecx * 4;
            put(app, local + 8, cpu.eax);
            cpu.eax = cpu.ebx + 8;
            cpu.edx = at(app, local + 8);
            put(app, local + 0xc, cpu.eax);
            cpu.eax += cpu.edx;
            if (at(app, kEnvMask) == 0)
            {
                if (!direct(app, cpu, nmalloc))
                    return;
                cpu.esi = cpu.eax;
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.edx = at(app, kWideEnviron);
                if (!direct(app, cpu, memcpy))
                    return;
                cpu.edx = at(app, local + 0xc);
                cpu.eax = cpu.esi;
                cpu.ebx = at(app, local + 8);
                cpu.eax += cpu.edx;
                cpu.edx = 0;
                put(app, kEnvMask, cpu.eax);
                if (!direct(app, cpu, memset))
                    return;
            }
            else
            {
                cpu.edx = cpu.eax;
                cpu.eax = cpu.esi;
                if (!direct(app, cpu, nrealloc))
                    return;
                cpu.esi = cpu.eax;
                if (cpu.eax == 0)
                    return back(0xffffffff);
                cpu.ebx = at(app, local + 0xc) + cpu.eax;
                put(app, local, cpu.ebx);
                cpu.edx = at(app, kEnvMask);
                cpu.eax = at(app, local);
                cpu.ebx = cpu.ecx;
                if (!direct(app, cpu, memmove))
                    return;
                cpu.eax = at(app, local);
                put(app, kEnvMask, cpu.eax);
            }
            cpu.eax = at(app, kEnvMask);
            put(app, cpu.esi + cpu.ecx * 4 + 4, 0);
            put(app, kWideEnviron, cpu.esi);
            putByte(app, cpu.ecx + cpu.eax, 0);
        }
    }
    // name=value into a wide string of the runtime's.
    cpu.eax = cpu.ebp;
    if (!direct(app, cpu, wcslen))
        return;
    put(app, local + 4, cpu.eax);
    cpu.eax = at(app, kEnvMask);
    cpu.ebx = byte(app, cpu.ecx + cpu.eax) != 0 ? at(app, cpu.esi + cpu.ecx * 4) : 0;
    cpu.eax = cpu.edi;
    cpu.edx = at(app, local + 4);
    if (!direct(app, cpu, wcslen))
        return;
    cpu.eax += cpu.edx;
    cpu.edx = cpu.eax * 2 + 4;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, nrealloc))
        return;
    put(app, local + 0x10, cpu.eax);
    if (cpu.eax == 0)
        return back(0xffffffff);
    cpu.eax = at(app, local + 4) * 2;
    cpu.edx = cpu.ebp;
    put(app, local, cpu.eax);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, local + 0x10);
    cpu.ebp = at(app, local);
    if (!direct(app, cpu, memcpy))
        return;
    cpu.eax = at(app, local + 0x10) + cpu.ebp;
    cpu.edx = at(app, local + 0x10);
    putHalf(app, cpu.eax, '=');
    cpu.eax = cpu.ebp + 2 + cpu.edx;
    cpu.edx = cpu.edi;
    if (!direct(app, cpu, wcscpy))
        return;
    cpu.edx = at(app, local + 0x10);
    cpu.eax = at(app, kEnvMask);
    put(app, cpu.esi + cpu.ecx * 4, cpu.edx);
    putByte(app, cpu.ecx + cpu.eax, 1);
    back(0);
}

/* setenv (clib/environ/c/setenv.c, 0x4f8b50): the variable named at eax set
 * to the string at edx -- replaced if ebx says so, an empty one taking it out
 * -- in Windows' environment by SetEnvironmentVariableA, in _environ, and in
 * _wenviron if there is one; 0, or -1. */
void setenv(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x10;
    const reg32 local = cpu.esp;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    put(app, local + 0xc, cpu.ebx);
    cpu.edx = 2;
    cpu.ebx = 1;
    put(app, local + 4, cpu.edx);
    put(app, local, cpu.ebx);
    auto back = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = local + 0x10;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.esi != 0 && byte(app, cpu.esi) == 0)
    {
        bool none = at(app, local + 0xc) != 0;
        if (!none)
        {
            cpu.eax = cpu.edi;
            if (!direct(app, cpu, getenv))
                return;
            none = cpu.eax == 0;
        }
        if (none)
            cpu.esi = 0;
    }
    bool toWindows = true;
    if (at(app, local + 0xc) == 0)
    {
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, getenv))
            return;
        toWindows = cpu.eax == 0;
    }
    if (toWindows)
    {
        push(app, cpu, cpu.esi);
        push(app, cpu, cpu.edi);
        if (!indirect(app, cpu, at(app, kSetEnvironmentVariableA)))
            return;
        if (cpu.eax == 0)
            return back(0xffffffff);
    }
    cpu.ebx = at(app, local + 0xc);
    cpu.edx = cpu.esi;
    cpu.eax = cpu.edi;
    if (!direct(app, cpu, setenv_))
        return;
    if (cpu.eax != 0)
        return back(0xffffffff);
    cpu.eax = at(app, kWideEnviron);
    if (cpu.eax == 0)
        return back(cpu.eax);
    cpu.eax = cpu.edi;
    if (!direct(app, cpu, mbslen))
        return;
    cpu.ebx = cpu.eax + 1;
    cpu.eax = at(app, local + 4) * cpu.ebx;
    if (!direct(app, cpu, nmalloc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebp = cpu.eax;
    auto noMemory = [&] {
        cpu.eax = 5;  // ENOMEM
        if (!direct(app, cpu, set_errno_dos))
            return;
        back(0xffffffff);
    };
    if (cpu.eax == 0)
        return noMemory();
    if (cpu.esi != 0)
    {
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, mbslen))
            return;
        ++cpu.eax;
        cpu.ecx = at(app, local + 4);
        put(app, local + 8, cpu.eax);
        cpu.eax *= cpu.ecx;
        if (!direct(app, cpu, nmalloc))
            return;
        cpu.ecx = cpu.eax;
        if (cpu.eax == 0)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, nfree))
                return;
            return noMemory();
        }
    }
    else
        cpu.ecx = 0;
    auto freeBoth = [&] {
        cpu.eax = cpu.ebp;
        if (!direct(app, cpu, nfree))
            return;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, nfree))
            return;
        back(0xffffffff);
    };
    cpu.ebx *= at(app, local);
    cpu.edx = cpu.edi;
    cpu.eax = cpu.ebp;
    if (!direct(app, cpu, mbstowcs))
        return;
    if (cpu.eax == 0xffffffff)
        return freeBoth();
    if (cpu.ecx != 0)
    {
        cpu.ebx = at(app, local + 8) * at(app, local);
        cpu.eax = cpu.ecx;
        cpu.edx = cpu.esi;
        if (!direct(app, cpu, mbstowcs))
            return;
        if (cpu.eax == 0xffffffff)
            return freeBoth();
    }
    cpu.ebx = at(app, local + 0xc);
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebp;
    if (!direct(app, cpu, wsetenv))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebp;
    if (!direct(app, cpu, nfree))
        return;
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, nfree))
            return;
    }
    back(cpu.edx);
}


/* ---- Formatted input: sscanf, __scnf and its conversions, strtod and the
 * long double it builds the number in -- its decimal digits made binary, then
 * scaled by powers of ten on the x87.
 *
 * __scnf reads through the two routines its specs hold -- sscanf's take the
 * string's characters, and put one back -- and each conversion takes its
 * argument's pointer as the code does: the va_list moved on, es made ds (or,
 * for an 'F' one, les, which the generated code has not either), the store
 * through es.  These are written as the code is, as the environment's are:
 * each push and pop to the guest's stack, each call with the return address's
 * slot, the registers as the code leaves them -- a width the format gives no
 * limit to lets the float's conversion run over its buffer on the stack, and
 * that is the code's too.  Their x87 instructions are the same operations of
 * the port's x87 as the generated code has for them, so a number comes out to
 * the bit as the game's code makes it. ---- */
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

/* Whether _IsTable says the character, eax as it is, is `kind`: the code's
 * inc al and and eax, 0xff, which leave eax the table's index. */
bool isKind(win32::WinApplication* app, x86::CPU& cpu, reg8 kind)
{
    cpu.eax = (cpu.eax + 1) & 0xff;
    return (byte(app, kIsTable + cpu.eax) & kind) != 0;
}

/* The pointer a conversion stores through, from the va_list at `argp`: the
 * va_list moved on into `step`, then a far pointer's les -- not the generated
 * code's either -- or es made ds through `ds` and the pointer into
 * `target`. */
void scanTarget(win32::WinApplication* app, x86::CPU& cpu, reg32 argp, bool far, reg32& step, reg32& target,
                reg32& ds)
{
    step = at(app, argp) + (far ? 8 : 4);
    put(app, argp, step);
    if (far)
    {
        NFS2_ASSERT(false);  // les
        return;
    }
    ds = cpu.ds;
    cpu.es = reg16(ds);
    target = at(app, step - 4);
}

/* movsd: a dword from esi to es:edi. */
void moveDword(win32::WinApplication* app, x86::CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(-4) : 4;
    cpu.edi += step;
    cpu.esi += step;
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

/* 0x4f04f0 (cget_string): sscanf's next character, from the string in its
 * specs, or -1 at its end, which the specs' flags note. */
void cget_string(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.ebx = at(app, cpu.eax + kScanString);
    cpu.edx = byte(app, cpu.ebx);
    if (cpu.edx != 0)
    {
        ++cpu.ebx;
        put(app, cpu.eax + kScanString, cpu.ebx);
        cpu.eax = cpu.edx;
    }
    else
    {
        setLow8(cpu.ebx, byte(app, cpu.eax + kScanFlags) | kEndOfInput);
        cpu.edx = 0xffffffff;
        putByte(app, cpu.eax + kScanFlags, reg8(cpu.ebx));
        cpu.eax = cpu.edx;
    }
    scanEpilogue(app, cpu, {&cpu.edx, &cpu.ebx});
}

/* 0x4f051c (uncget_string): the character back -- the string's pointer back
 * by one. */
void uncget_string(win32::WinApplication* app, x86::CPU& cpu)
{
    put(app, cpu.edx + kScanString, at(app, cpu.edx + kScanString) - 1);
    cpu.esp += 4;
}

/* 0x50c640 (cgetc): the next character, by the specs' routine. */
void cgetc(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!indirect(app, cpu, at(app, cpu.edx + kScanGet)))
        return;
    scanEpilogue(app, cpu, {&cpu.edx});
}

/* 0x50c648 (uncget): the character in eax back, by the specs' routine. */
void uncget(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.ebx = cpu.edx;
    if (!indirect(app, cpu, at(app, cpu.ebx + kScanUnget)))
        return;
    scanEpilogue(app, cpu, {&cpu.ebx});
}

/* 0x50d538 (chktrm): the next character while the field's width lasts -- -1
 * when it is over, or the input is. */
void chktrm(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + kScanWidth);
    cpu.ebx = cpu.eax - 1;
    put(app, cpu.edx + kScanWidth, cpu.ebx);
    bool over = cpu.eax == 0;
    if (!over)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, cgetc))
            return;
        over = (scanFlags(app, cpu.edx) & kEndOfInput) != 0;
    }
    if (over)
        cpu.eax = 0xffffffff;
    scanEpilogue(app, cpu, {&cpu.edx, &cpu.ebx});
}

/* 0x50d510 (radix_value): the digit eax is, in any base up to 16; 16 for
 * none. */
void radix_value(win32::WinApplication* app, x86::CPU& cpu)
{
    if (x86::sreg32(cpu.eax) >= 0x30 && x86::sreg32(cpu.eax) <= 0x39)
        return returns(cpu, cpu.eax - 0x30);
    if (!direct(app, cpu, tolower))
        return;
    if (x86::sreg32(cpu.eax) >= 0x61 && x86::sreg32(cpu.eax) <= 0x66)
        return returns(cpu, cpu.eax - 0x57);
    returns(cpu, 0x10);
}

/* 0x50c904 (get_opt): the format's options after a '%', at eax, into the
 * specs at edx -- '*', the width, N or F, then h, l, w, L or I64; past them. */
void get_opt(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    auto done = [&] { scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx}); };
    cpu.edi = cpu.edx;
    setLow8(cpu.edx, byte(app, cpu.edx + kScanFlags) | kAssign);
    putByte(app, cpu.edi + kScanFlags, reg8(cpu.edx));
    setHigh8(cpu.edx, reg8(cpu.edx));
    put(app, cpu.edi + kScanWidth, 0xffffffff);
    setHigh8(cpu.edx, (cpu.edx >> 8) & 3);
    putByte(app, cpu.edi + kScanFlags, reg8(cpu.edx >> 8));
    if (byte(app, cpu.eax) == '*')
    {
        setHigh8(cpu.ebx, byte(app, cpu.edi + kScanFlags) & 0xfe);
        ++cpu.eax;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.ebx >> 8));
    }
    cpu.ebx = byte(app, cpu.eax);
    cpu.edx = (cpu.ebx + 1) & 0xff;
    if (byte(app, kIsTable + cpu.edx) & kDigit)
    {
        cpu.edx = 0;
        for (;;)
        {
            cpu.edx = cpu.edx * 10 + (cpu.ebx - 0x30);
            cpu.ebx = byte(app, cpu.eax + 1);
            setLow8(cpu.ecx, cpu.ebx + 1);
            cpu.esi = reg8(cpu.ecx);
            setHigh8(cpu.ecx, byte(app, kIsTable + cpu.esi));
            ++cpu.eax;
            if (!(reg8(cpu.ecx >> 8) & kDigit))
                break;
        }
        put(app, cpu.edi + kScanWidth, cpu.edx);
    }
    setLow8(cpu.ebx, byte(app, cpu.eax));
    cpu.edx = cpu.eax + 1;
    if (reg8(cpu.ebx) == 'N')
    {
        setHigh8(cpu.ecx, byte(app, cpu.edi + kScanFlags) | kNearPointer);
        cpu.eax = cpu.edx;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.ecx >> 8));
    }
    else if (reg8(cpu.ebx) == 'F')
    {
        setLow8(cpu.ecx, byte(app, cpu.edi + kScanFlags) | kFarPointer);
        cpu.eax = cpu.edx;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.ecx));
    }
    setLow8(cpu.edx, byte(app, cpu.eax));
    cpu.ebx = cpu.eax + 1;
    const reg8 dl = reg8(cpu.edx);
    if (dl == 'h')
    {
        setHigh8(cpu.edx, byte(app, cpu.edi + kScanFlags) | kShortVar);
        cpu.eax = cpu.ebx;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.edx >> 8));
    }
    else if (dl == 'l' || dl == 'w')
    {
        setLow8(cpu.edx, byte(app, cpu.edi + kScanFlags) | kLongVar);
        ++cpu.eax;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.edx));
    }
    else if (dl == 'L')
    {
        setLow8(cpu.ecx, byte(app, cpu.edi + kScanFlags) | kLongLongVar);
        cpu.eax = cpu.ebx;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.ecx));
    }
    else if (dl == 'I' && byte(app, cpu.eax + 1) == '6' && byte(app, cpu.eax + 2) == '4')
    {
        setHigh8(cpu.edx, byte(app, cpu.edi + kScanFlags) | kLongLongVar);
        cpu.eax += 3;
        putByte(app, cpu.edi + kScanFlags, reg8(cpu.edx >> 8));
    }
    done();
}

/* 0x50ca08 (scan_white): white space skipped: how much; the character after
 * it put back, unless the input is over. */
void scan_white(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 4;
    cpu.edx = cpu.eax;
    cpu.ebx = 0;
    for (;;)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, cgetc))
            return;
        put(app, cpu.esp, cpu.eax);
        setLow8(cpu.ecx, byte(app, cpu.esp) + 1);
        cpu.esi = reg8(cpu.ecx);
        if (!(byte(app, kIsTable + cpu.esi) & kSpace))
            break;
        ++cpu.ebx;
    }
    if (!(scanFlags(app, cpu.edx) & kEndOfInput) && !direct(app, cpu, uncget))
        return;
    cpu.eax = cpu.ebx;
    cpu.esp += 4;
    scanEpilogue(app, cpu, {&cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x50ca48 (scan_char): %c and %C: the field's width of characters, 1 for
 * none, into the argument -- %C's made wide characters by mbtowc.  How many
 * were read; 0 for a character mbtowc could not make. */
void scan_char(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 8;
    const reg32 local = cpu.esp;
    cpu.ecx = cpu.eax;
    if (scanFlags(app, cpu.eax) & kAssign)
    {
        setLow8(cpu.ebx, scanFlags(app, cpu.ecx));
        if (reg8(cpu.ebx) & kFarPointer)
            scanTarget(app, cpu, cpu.edx, true, cpu.edi, cpu.esi, cpu.eax);
        else if (reg8(cpu.ebx) & kNearPointer)
            scanTarget(app, cpu, cpu.edx, false, cpu.esi, cpu.esi, cpu.eax);
        else
            scanTarget(app, cpu, cpu.edx, false, cpu.ebx, cpu.esi, cpu.eax);
    }
    cpu.edi = at(app, cpu.ecx + kScanWidth);
    cpu.ebp = 0;
    if (cpu.edi == 0xffffffff)
        cpu.edi = 1;
    while (x86::sreg32(cpu.edi) > 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cgetc))
            return;
        setLow8(cpu.edx, scanFlags(app, cpu.ecx));
        if (reg8(cpu.edx) & kEndOfInput)
            break;
        ++cpu.ebp;
        --cpu.edi;
        if (!(reg8(cpu.edx) & kAssign))
            continue;
        if (!(reg8(cpu.edx) & kLongVar))
        {
            ++cpu.esi;
            putByte(app, cpu.ees + cpu.esi - 1, reg8(cpu.eax));
            continue;
        }
        cpu.edx = at(app, kMbcsOn);
        putByte(app, local + 4, reg8(cpu.eax));
        if (cpu.edx != 0)
        {
            cpu.eax = byte(app, kMbcsLead + (cpu.eax & 0xff)) & 1;
            if (cpu.eax != 0)
            {
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, cgetc))
                    return;
                putByte(app, local + 5, reg8(cpu.eax));
            }
        }
        cpu.ebx = 2;
        cpu.edx = local + 4;
        cpu.eax = local;
        if (!direct(app, cpu, mbtowc))
            return;
        if (cpu.eax == 0xffffffff)
        {
            cpu.eax = 0;
            goto out;
        }
        cpu.esi += 2;
        cpu.eax = at(app, local);
        putHalf(app, cpu.ees + cpu.esi - 2, reg16(cpu.eax));
    }
    cpu.eax = cpu.ebp;
out:
    cpu.esp += 8;
    cpu.ebp = pop(app, cpu);
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx});
}

/* 0x50cb30 (scan_string): %s and %S: white space skipped, then characters to
 * the next, the field's width at most, into the argument with a 0 after --
 * %S's wide.  How many were read; 0 for a character mbtowc could not make. */
void scan_string(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0xc;
    const reg32 local = cpu.esp;
    cpu.ecx = cpu.eax;
    putByte(app, local + 8, scanFlags(app, cpu.eax) & kLongVar ? 2 : 1);
    auto size = [&] { return byte(app, local + 8); };
    setLow8(cpu.ebx, scanFlags(app, cpu.ecx));
    if (reg8(cpu.ebx) & kAssign)
    {
        if (reg8(cpu.ebx) & kFarPointer)
            scanTarget(app, cpu, cpu.edx, true, cpu.edi, cpu.esi, cpu.eax);
        else if (reg8(cpu.ebx) & kNearPointer)
            scanTarget(app, cpu, cpu.edx, false, cpu.esi, cpu.esi, cpu.eax);
        else
            scanTarget(app, cpu, cpu.edx, false, cpu.ebx, cpu.esi, cpu.eax);
    }
    cpu.edi = 0;
    for (;;)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cgetc))
            return;
        cpu.ebx = cpu.eax;
        if (!isKind(app, cpu, kSpace))
            break;
        ++cpu.edi;
    }
    if (scanFlags(app, cpu.ecx) & kEndOfInput)
    {
        cpu.edi = 0;
        goto end;
    }
    cpu.eax = at(app, cpu.ecx + kScanWidth);
    cpu.edx = cpu.eax - 1;
    put(app, cpu.ecx + kScanWidth, cpu.edx);
    if (cpu.eax != 0)
    {
        for (;;)
        {
            setLow8(cpu.eax, scanFlags(app, cpu.ecx));
            ++cpu.edi;
            if (reg8(cpu.eax) & kAssign)
            {
                if (size() == 1)
                    putByte(app, cpu.ees + cpu.esi, reg8(cpu.ebx));
                else
                {
                    cpu.ebp = at(app, kMbcsOn);
                    putByte(app, local + 4, reg8(cpu.ebx));
                    if (cpu.ebp != 0)
                    {
                        cpu.eax = byte(app, kMbcsLead + reg8(cpu.ebx)) & 1;
                        if (cpu.eax != 0)
                        {
                            cpu.eax = cpu.ecx;
                            if (!direct(app, cpu, cgetc))
                                return;
                            putByte(app, local + 5, reg8(cpu.eax));
                        }
                    }
                    cpu.ebx = 2;
                    cpu.edx = local + 4;
                    cpu.eax = local;
                    if (!direct(app, cpu, mbtowc))
                        return;
                    if (cpu.eax == 0xffffffff)
                    {
                        cpu.eax = 0;
                        goto out;
                    }
                    cpu.eax = at(app, local);
                    putHalf(app, cpu.ees + cpu.esi, reg16(cpu.eax));
                }
                cpu.eax = size();
                cpu.esi += cpu.eax;
            }
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, chktrm))
                return;
            cpu.ebx = cpu.eax;
            if (cpu.eax == 0xffffffff)
                goto end;
            if (isKind(app, cpu, kSpace))
                break;
        }
    }
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, uncget))
        return;
end:
    if ((scanFlags(app, cpu.ecx) & kAssign) && x86::sreg32(cpu.edi) > 0)
    {
        if (size() == 1)
            putByte(app, cpu.ees + cpu.esi, 0);
        else
            putHalf(app, cpu.ees + cpu.esi, 0);
    }
    cpu.eax = cpu.edi;
out:
    cpu.esp += 0xc;
    cpu.ebp = pop(app, cpu);
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx});
}

/* 0x50cc88 (report_scan): %n: the characters read so far, ebx, into the
 * argument -- a short for h. */
void report_scan(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    setLow8(cpu.ecx, scanFlags(app, cpu.eax));
    if (reg8(cpu.ecx) & kAssign)
    {
        if (reg8(cpu.ecx) & kFarPointer)
            scanTarget(app, cpu, cpu.edx, true, cpu.edi, cpu.edx, cpu.ecx);
        else if (reg8(cpu.ecx) & kNearPointer)
            scanTarget(app, cpu, cpu.edx, false, cpu.esi, cpu.edx, cpu.ecx);
        else
        {
            cpu.ecx = at(app, cpu.edx) + 4;
            put(app, cpu.edx, cpu.ecx);
            cpu.edx = cpu.ecx;
            cpu.ecx = cpu.ds;
            cpu.es = reg16(cpu.ecx);
            cpu.edx = at(app, cpu.edx - 4);
        }
        setHigh8(cpu.ecx, scanFlags(app, cpu.eax));
        if (reg8(cpu.ecx >> 8) & kShortVar)
            putHalf(app, cpu.ees + cpu.edx, reg16(cpu.ebx));
        else
            put(app, cpu.ees + cpu.edx, cpu.ebx);
    }
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx});
}

/* 0x50cce8 (makelist): a [set]'s characters, the format's at eax, as bits of
 * the 32 bytes at edx; past its ']' -- one first is the set's. */
void makelist(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.ebx = 0x20;
    cpu.eax = cpu.esi;
    cpu.edx = 0;
    if (!direct(app, cpu, memset))
        return;
    cpu.eax = byte(app, cpu.ecx);
    ++cpu.ecx;
    while (cpu.eax != 0)
    {
        cpu.edx = cpu.eax;
        cpu.eax &= 7;
        cpu.edx = reg32(x86::sreg32(cpu.edx) >> 3);
        cpu.eax = byte(app, kBitMasks + cpu.eax);
        putByte(app, cpu.edx + cpu.esi, byte(app, cpu.edx + cpu.esi) | reg8(cpu.eax));
        cpu.eax = byte(app, cpu.ecx);
        if (cpu.eax == 0)
            break;
        ++cpu.ecx;
        if (cpu.eax == ']')
            break;
    }
    cpu.eax = cpu.ecx;
    scanEpilogue(app, cpu, {&cpu.esi, &cpu.ecx, &cpu.ebx});
}

/* 0x50cd2c (scan_arb): %[: characters in the set -- out of it, for a '^'
 * first -- the field's width at most, into the argument with a 0 after; ebx
 * the format's pointer, moved past the set.  How many were read. */
void scan_arb(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x30;
    const reg32 local = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.eax = at(app, cpu.ebx);
    cpu.edx = byte(app, cpu.eax) == '^' ? 1 : 0;
    put(app, local + 0x24, cpu.edx);
    if (cpu.edx != 0)
    {
        ++cpu.eax;
        put(app, cpu.ebx, cpu.eax);
    }
    cpu.edx = local;
    cpu.eax = at(app, cpu.ebx);
    if (!direct(app, cpu, makelist))
        return;
    put(app, cpu.ebx, cpu.eax);
    setLow8(cpu.ebx, scanFlags(app, cpu.ecx));
    if (reg8(cpu.ebx) & kAssign)
    {
        if (reg8(cpu.ebx) & kFarPointer)
            scanTarget(app, cpu, cpu.edi, true, cpu.esi, cpu.esi, cpu.eax);
        else if (reg8(cpu.ebx) & kNearPointer)
            scanTarget(app, cpu, cpu.edi, false, cpu.ebx, cpu.esi, cpu.eax);
        else
            scanTarget(app, cpu, cpu.edi, false, cpu.edx, cpu.esi, cpu.eax);
    }
    cpu.edx = at(app, cpu.ecx + kScanWidth);
    cpu.ebx = 0;
    while (cpu.edx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cgetc))
            return;
        put(app, local + 0x28, cpu.eax);
        if (scanFlags(app, cpu.ecx) & kEndOfInput)
            break;
        cpu.edi = byte(app, local + reg32(x86::sreg32(cpu.eax) >> 3));
        put(app, local + 0x20, cpu.edi);
        cpu.edi = cpu.eax & 7;
        cpu.ebp = at(app, local + 0x20);
        cpu.edi = byte(app, kBitMasks + cpu.edi);
        putByte(app, local + 0x2c, (cpu.edi & cpu.ebp) == 0 ? 1 : 0);
        cpu.edi = byte(app, local + 0x2c);
        if (cpu.edi != at(app, local + 0x24))
        {
            cpu.edx = cpu.ecx;
            if (!direct(app, cpu, uncget))
                return;
            break;
        }
        ++cpu.ebx;
        setHigh8(cpu.eax, scanFlags(app, cpu.ecx));
        --cpu.edx;
        if (reg8(cpu.eax >> 8) & kAssign)
        {
            ++cpu.esi;
            setLow8(cpu.eax, byte(app, local + 0x28));
            putByte(app, cpu.ees + cpu.esi - 1, reg8(cpu.eax));
        }
    }
    if ((scanFlags(app, cpu.ecx) & kAssign) && x86::sreg32(cpu.ebx) > 0)
        putByte(app, cpu.ees + cpu.esi, 0);
    cpu.eax = cpu.ebx;
    cpu.esp += 0x30;
    cpu.ebp = pop(app, cpu);
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx});
}

/* 0x523ca0 (__FDFS): the double in edx:eax as a float, in eax: 0 for one too
 * small, its sign and infinity for one too big. */
void FDFS(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    auto done = [&] { scanEpilogue(app, cpu, {&cpu.ebx}); };
    if (!(cpu.edx & 0x7ff00000))
    {
        cpu.eax = 0;
        return done();
    }
    cpu.ebx = 0;
    x86::reg64 value = x86::reg64(cpu.edx) << 32 | cpu.eax;
    cpu.ebx = reg32(value >> 63) << 31;
    value = (value << 1) + 0x20000000;
    cpu.eax = reg32(value);
    cpu.edx = reg32(value >> 32);
    if (cpu.edx == 0 || cpu.edx >= 0x8fe00000)
    {
        cpu.eax = 0x7f800000 | cpu.ebx;
        return done();
    }
    if (cpu.edx < 0x70200000)
    {
        cpu.eax = 0;
        return done();
    }
    cpu.edx -= 0x70000000;
    value = (x86::reg64(cpu.edx) << 32 | cpu.eax) << 2;
    cpu.eax = reg32(value);
    cpu.edx = reg32(value >> 32) | cpu.ebx;
    cpu.eax = cpu.edx;
    done();
}

/* 0x523cec (__U8M): edx:eax times ecx:ebx, in edx:eax; ecx left the cross
 * products' sum. */
void U8M(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.edx == 0 && cpu.ecx == 0)
    {
        const x86::reg64 product = x86::reg64(cpu.eax) * cpu.ebx;
        cpu.eax = reg32(product);
        cpu.edx = reg32(product >> 32);
        cpu.esp += 4;
        return;
    }
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.edx);
    x86::reg64 product = x86::reg64(cpu.eax) * cpu.ecx;
    cpu.ecx = reg32(product);
    cpu.eax = pop(app, cpu);
    product = x86::reg64(cpu.eax) * cpu.ebx;
    cpu.ecx += reg32(product);
    cpu.eax = pop(app, cpu);
    product = x86::reg64(cpu.eax) * cpu.ebx;
    cpu.eax = reg32(product);
    cpu.edx = reg32(product >> 32) + cpu.ecx;
    cpu.esp += 4;
}

/* 0x50ce18 (scan_float): %e, %f, %g, %E and %G: a number's characters --
 * sign, digits, a point, digits, an exponent -- gathered on the stack, then
 * made a double by __EFG_scanf and stored, as a float but for l and L; with
 * h, the code's fixed point, a short whole part and a short fraction.  How
 * many characters were read; 0 for none of a number. */
void scan_float(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x6c;
    const reg32 local = cpu.esp;
    cpu.ecx = cpu.eax;
    put(app, local + 0x58, cpu.edx);
    cpu.edi = 0;
    cpu.esi = local;
    put(app, local + 0x68, cpu.edi);
    // The character into the buffer, the next one taken; false at -1.
    auto keep = [&](bool& ok) {
        putByte(app, cpu.esi, reg8(cpu.ebx));
        ++cpu.esi;
        cpu.eax = cpu.ecx;
        ok = direct(app, cpu, chktrm);
        cpu.ebx = cpu.eax;
        return ok && cpu.eax != 0xffffffff;
    };
    bool ok = true;
    for (;;)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cgetc))
            return;
        setLow8(cpu.edx, cpu.eax + 1);
        cpu.ebp = reg8(cpu.edx);
        setLow8(cpu.edx, byte(app, kIsTable + cpu.ebp));
        cpu.ebx = cpu.eax;
        if (!(reg8(cpu.edx) & kSpace))
            break;
        put(app, local + 0x68, at(app, local + 0x68) + 1);
    }
    if (scanFlags(app, cpu.ecx) & kEndOfInput)
        goto L_d031;
    cpu.edx = at(app, cpu.ecx + kScanWidth);
    cpu.ebp = cpu.edx - 1;
    put(app, cpu.ecx + kScanWidth, cpu.ebp);
    if (cpu.edx == 0)
        goto L_d028;
    if (cpu.eax == '+' || cpu.eax == '-')
    {
        cpu.ebp = at(app, local + 0x68);
        putByte(app, cpu.esi, reg8(cpu.ebx));
        ++cpu.esi;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, chktrm))
            return;
        ++cpu.ebp;
        cpu.ebx = cpu.eax;
        put(app, local + 0x68, cpu.ebp);
        if (cpu.eax == 0xffffffff)
            goto L_d031;
    }
    cpu.eax = cpu.ebx;
    if (!isKind(app, cpu, kDigit) && cpu.ebx != '.')
        goto L_d028;
    cpu.eax = 0;
    put(app, local + 0x64, cpu.eax);
    cpu.eax = cpu.ebx;
    cpu.ebp = 0;
    if (isKind(app, cpu, kDigit))
    {
        cpu.ebp = 1;
        do
        {
            putByte(app, cpu.esi, reg8(cpu.ebx));
            setLow8(cpu.eax, scanFlags(app, cpu.ecx));
            ++cpu.esi;
            if (reg8(cpu.eax) & kShortVar)
            {
                cpu.eax = at(app, local + 0x66) * 10;
                cpu.ebx = cpu.ebx + cpu.eax - 0x30;
                putHalf(app, local + 0x66, reg16(cpu.ebx));
            }
            cpu.eax = cpu.ecx;
            ++cpu.edi;
            if (!direct(app, cpu, chktrm))
                return;
            cpu.ebx = cpu.eax;
            if (cpu.eax == 0xffffffff)
                goto L_d031;
        } while (isKind(app, cpu, kDigit));
    }
    if (cpu.ebx == '.')
    {
        if (!keep(ok))
        {
            if (!ok)
                return;
            goto L_d031;
        }
        if (cpu.ebp == 0 && !isKind(app, cpu, kDigit))
            goto L_d028;
        ++cpu.edi;
        for (;;)
        {
            cpu.eax = cpu.ebx;
            if (!isKind(app, cpu, kDigit))
                break;
            ++cpu.edi;
            if (!keep(ok))
            {
                if (!ok)
                    return;
                break;
            }
        }
        if (scanFlags(app, cpu.ecx) & kShortVar)
        {
            // The fraction's digits, back from the last to the point, made a
            // short's fraction: each over 10, as the code divides them.
            cpu.edx = 0;
            cpu.ebp = cpu.esi;
            put(app, local + 0x60, cpu.edx);
            for (;;)
            {
                setLow8(cpu.eax, byte(app, cpu.ebp - 1));
                --cpu.ebp;
                if (reg8(cpu.eax) == '.')
                    break;
                setLow8(cpu.eax, reg8(cpu.eax) - 0x30);
                put(app, local + 0x5c, 10);
                putByte(app, local + 0x62, reg8(cpu.eax));
                cpu.edx = 0;
                cpu.eax = at(app, local + 0x60);
                cpu.edx = cpu.eax % 10;
                cpu.eax /= 10;
                put(app, local + 0x60, cpu.eax);
            }
            cpu.eax = at(app, local + 0x60);
            putHalf(app, local + 0x64, reg16(cpu.eax));
        }
        if (cpu.ebx == 0xffffffff)
            goto L_d031;
    }
    if (scanFlags(app, cpu.ecx) & kShortVar)
        goto L_d028;
    if (cpu.ebx != 'e' && cpu.ebx != 'E')
        goto L_d028;
    ++cpu.edi;
    if (!keep(ok))
    {
        if (!ok)
            return;
        goto L_d031;
    }
    if (cpu.eax == '+' || cpu.eax == '-')
    {
        ++cpu.edi;
        if (!keep(ok))
        {
            if (!ok)
                return;
            goto L_d031;
        }
    }
    cpu.eax = cpu.ebx;
    if (!isKind(app, cpu, kDigit))
    {
        cpu.edi = 0;
        goto L_d028;
    }
    do
    {
        ++cpu.edi;
        if (!keep(ok))
        {
            if (!ok)
                return;
            goto L_d031;
        }
    } while (isKind(app, cpu, kDigit));
L_d028:
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, uncget))
        return;
L_d031:
    if (x86::sreg32(cpu.edi) > 0)
    {
        cpu.edi += at(app, local + 0x68);
        if (scanFlags(app, cpu.ecx) & kAssign)
        {
            putByte(app, cpu.esi, 0);
            if (scanFlags(app, cpu.ecx) & kShortVar)
            {
                if (byte(app, local) == '-')
                    put(app, local + 0x64, 0 - at(app, local + 0x64));
            }
            else
            {
                cpu.edx = local + 0x50;
                cpu.eax = local;
                if (!indirect(app, cpu, at(app, kScanFloat)))
                    return;
            }
            setHigh8(cpu.eax, scanFlags(app, cpu.ecx));
            const reg8 flags = reg8(cpu.eax >> 8);
            cpu.eax = at(app, local + 0x58);
            if (flags & kFarPointer)
                scanTarget(app, cpu, cpu.eax, true, cpu.ebx, cpu.ebx, cpu.eax);
            else if (flags & kNearPointer)
                scanTarget(app, cpu, cpu.eax, false, cpu.edx, cpu.ebx, cpu.eax);
            else
                scanTarget(app, cpu, cpu.eax, false, cpu.ebp, cpu.ebx, cpu.eax);
            setHigh8(cpu.edx, scanFlags(app, cpu.ecx));
            const reg8 dh = reg8(cpu.edx >> 8);
            if (dh & kShortVar)
            {
                cpu.eax = at(app, local + 0x64);
                put(app, cpu.ees + cpu.ebx, cpu.eax);
            }
            else if (dh & (kLongVar | kLongLongVar))
            {
                cpu.eax = at(app, local + 0x50);
                put(app, cpu.ees + cpu.ebx, cpu.eax);
                cpu.eax = at(app, local + 0x54);
                put(app, cpu.ees + cpu.ebx + 4, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, local + 0x50);
                cpu.edx = at(app, local + 0x54);
                if (!direct(app, cpu, FDFS))
                    return;
                put(app, cpu.ees + cpu.ebx, cpu.eax);
            }
        }
    }
    cpu.eax = cpu.edi;
    cpu.esp += 0x6c;
    cpu.ebp = pop(app, cpu);
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx});
}

/* 0x50d0f0 (scan_int): %d, %i, %o, %u, %x and %p: white space skipped, a
 * sign where ecx says the number has one, a base's digits -- ebx's base, 0
 * for the number's own: 0x for 16, 0 for 8 -- and for a segmented one a ':'
 * and an offset; stored as the flags say, a long long for L and I64.  How
 * many characters were read. */
void scan_int(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    pushEs(app, cpu);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x30;
    const reg32 local = cpu.esp;
    cpu.edi = cpu.eax;
    put(app, local + 0x1c, cpu.edx);
    put(app, local + 0x28, cpu.ebx);
    cpu.edx = 0;
    cpu.ebp = 0;
    put(app, local + 8, cpu.edx);
    put(app, local + 0xc, cpu.edx);
    put(app, local + 0x24, cpu.edx);
    put(app, local + 0x20, cpu.edx);
    auto character = [&] { return at(app, local + 0x2c); };
    // The next character, kept; false at -1, `ok` false if the game stopped.
    bool ok = true;
    auto next = [&] {
        cpu.eax = cpu.edi;
        ok = direct(app, cpu, chktrm);
        put(app, local + 0x2c, cpu.eax);
        return ok && cpu.eax != 0xffffffff;
    };
    // The 64-bit value times the base, plus the digit in eax.
    auto accumulate = [&] {
        cpu.esi = local + 8;
        put(app, local, cpu.eax);
        put(app, local + 4, 0);
        cpu.ebx = local + 0x10;
        cpu.eax = local + 8;
        cpu.edx = at(app, cpu.eax + 4);
        cpu.eax = at(app, cpu.eax);
        cpu.ecx = at(app, cpu.ebx + 4);
        cpu.ebx = at(app, cpu.ebx);
        if (!direct(app, cpu, U8M))
            return false;
        put(app, cpu.esi + 4, cpu.edx);
        put(app, cpu.esi, cpu.eax);
        cpu.esi = local + 8;
        cpu.ebx = local;
        cpu.eax = local + 8;
        cpu.edx = at(app, cpu.eax + 4);
        cpu.eax = at(app, cpu.eax);
        cpu.ecx = at(app, cpu.ebx + 4);
        cpu.ebx = at(app, cpu.ebx);
        const x86::reg64 sum = (x86::reg64(cpu.edx) << 32 | cpu.eax) + (x86::reg64(cpu.ecx) << 32 | cpu.ebx);
        cpu.eax = reg32(sum);
        cpu.edx = reg32(sum >> 32);
        put(app, cpu.esi + 4, cpu.edx);
        put(app, cpu.esi, cpu.eax);
        return true;
    };
    for (;;)
    {
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, cgetc))
            return;
        put(app, local + 0x2c, cpu.eax);
        setLow8(cpu.ebx, byte(app, local + 0x2c) + 1);
        cpu.ebx &= 0xff;
        if (!(byte(app, kIsTable + cpu.ebx) & kSpace))
            break;
        put(app, local + 0x20, at(app, local + 0x20) + 1);
    }
    if (scanFlags(app, cpu.edi) & kEndOfInput)
        goto L_d3d5;
    cpu.ebx = at(app, cpu.edi + kScanWidth);
    cpu.esi = cpu.ebx - 1;
    put(app, cpu.edi + kScanWidth, cpu.esi);
    if (cpu.ebx == 0)
        goto L_d3ca;
    cpu.ebx = '+';
    put(app, local + 0x18, cpu.ebx);
    if (cpu.ecx != 0 && (cpu.eax == cpu.ebx || cpu.eax == '-'))
    {
        cpu.eax = character();
        cpu.ecx = at(app, local + 0x20);
        put(app, local + 0x18, cpu.eax);
        ++cpu.ecx;
        put(app, local + 0x20, cpu.ecx);
        if (!next())
        {
            if (!ok)
                return;
            goto L_d3d5;
        }
    }
    cpu.esi = at(app, local + 0x28);
    if (cpu.esi == 0 || (cpu.esi == 16 && character() == '0'))
    {
        const bool any = cpu.esi == 0;
        if (character() == '0')
        {
            cpu.ebp = 1;
            if (!next())
            {
                if (!ok)
                    return;
                goto L_d3d5;
            }
            if (cpu.eax == 'x' || cpu.eax == 'X')
            {
                reg32& count = any ? cpu.edx : cpu.ebx;
                count = at(app, local + 0x20);
                cpu.eax = cpu.edi;
                if (!direct(app, cpu, chktrm))
                    return;
                cpu.ebp = 0;
                count += 2;
                put(app, local + 0x2c, cpu.eax);
                put(app, local + 0x20, count);
                if (cpu.eax == 0xffffffff)
                    goto L_d3d5;
                if (any)
                    put(app, local + 0x28, 16);
            }
            else if (any)
                put(app, local + 0x28, 8);
        }
        else
            put(app, local + 0x28, 10);
    }
    if (scanFlags(app, cpu.edi) & kLongLongVar)
    {
        cpu.eax = at(app, local + 0x28);
        cpu.ecx = 0;
        put(app, local + 0x10, cpu.eax);
        put(app, local + 0x14, cpu.ecx);
        for (;;)
        {
            cpu.eax = character();
            cpu.edx = at(app, local + 0x28);
            if (!direct(app, cpu, radix_value))
                return;
            if (x86::sreg32(cpu.eax) >= x86::sreg32(cpu.edx))
                break;
            if (!accumulate())
                return;
            ++cpu.ebp;
            if (!next())
            {
                if (!ok)
                    return;
                goto L_d3d5;
            }
        }
        if (character() != ':' || !(scanFlags(app, cpu.edi) & kSegmented))
            goto L_d3ca;
        for (;;)
        {
            ++cpu.ebp;
            if (!next())
            {
                if (!ok)
                    return;
                goto L_d3d5;
            }
            cpu.esi = at(app, local + 0x28);
            if (!direct(app, cpu, radix_value))
                return;
            if (x86::sreg32(cpu.eax) >= x86::sreg32(cpu.esi))
                break;
            if (!accumulate())
                return;
        }
    }
    else
    {
        for (;;)
        {
            cpu.esi = at(app, local + 0x28);
            cpu.eax = character();
            if (!direct(app, cpu, radix_value))
                return;
            if (x86::sreg32(cpu.eax) >= x86::sreg32(cpu.esi))
                break;
            cpu.ebx = at(app, local + 0x24) * cpu.esi;
            ++cpu.ebp;
            cpu.ebx += cpu.eax;
            put(app, local + 0x24, cpu.ebx);
            if (!next())
            {
                if (!ok)
                    return;
                goto L_d3d5;
            }
        }
        if (character() != ':' || !(scanFlags(app, cpu.edi) & kSegmented))
            goto L_d3ca;
        cpu.ecx = at(app, local + 0x28);
        for (;;)
        {
            ++cpu.ebp;
            if (!next())
            {
                if (!ok)
                    return;
                goto L_d3d5;
            }
            if (!direct(app, cpu, radix_value))
                return;
            if (x86::sreg32(cpu.eax) >= x86::sreg32(cpu.ecx))
                break;
            cpu.ebx = at(app, local + 0x24) * cpu.ecx + cpu.eax;
            put(app, local + 0x24, cpu.ebx);
        }
    }
L_d3ca:
    cpu.eax = character();
    cpu.edx = cpu.edi;
    if (!direct(app, cpu, uncget))
        return;
L_d3d5:
    if (scanFlags(app, cpu.edi) & kLongLongVar)
    {
        if (at(app, local + 0x18) == '-')
        {
            cpu.esi = ~at(app, local + 8);
            cpu.eax = ~at(app, local + 0xc);
            put(app, local + 8, cpu.esi);
            cpu.edx = cpu.esi + 1;
            put(app, local + 0xc, cpu.eax);
            put(app, local + 8, cpu.edx);
            if (cpu.edx == 0)
                ++cpu.eax;
            put(app, local + 0xc, cpu.eax);
        }
        if (x86::sreg32(cpu.ebp) > 0)
        {
            cpu.ecx = at(app, local + 0x20);
            setHigh8(cpu.ebx, scanFlags(app, cpu.edi));
            cpu.ebp += cpu.ecx;
            const reg8 bh = reg8(cpu.ebx >> 8);
            if (bh & kAssign)
            {
                bool move = true;
                if (bh & kFarPointer)
                {
                    cpu.eax = at(app, local + 0x1c);
                    cpu.edx = at(app, cpu.eax) + 8;
                    put(app, cpu.eax, cpu.edx);
                    cpu.eax = (cpu.eax & 0xffff0000) | half(app, cpu.edx - 4);
                    cpu.edi = at(app, cpu.edx - 8);
                }
                else if (bh & kNearPointer)
                {
                    // The far one's segment is not taken: es made ds, the
                    // two moved, and on to the end without esi's lea after.
                    cpu.eax = at(app, local + 0x1c);
                    cpu.edi = at(app, cpu.eax) + 4;
                    put(app, cpu.eax, cpu.edi);
                    cpu.eax = cpu.ds;
                    cpu.edi = at(app, cpu.edi - 4);
                    cpu.esi = local + 8;
                    cpu.es = reg16(cpu.eax);
                    move = false;
                    moveDword(app, cpu);
                    moveDword(app, cpu);
                }
                else
                {
                    cpu.eax = at(app, local + 0x1c);
                    cpu.esi = at(app, cpu.eax) + 4;
                    put(app, cpu.eax, cpu.esi);
                    cpu.eax = cpu.ds;
                    cpu.edi = at(app, cpu.esi - 4);
                }
                if (move)
                {
                    cpu.esi = local + 8;
                    cpu.es = reg16(cpu.eax);
                    moveDword(app, cpu);
                    moveDword(app, cpu);
                }
            }
        }
    }
    else
    {
        if (at(app, local + 0x18) == '-')
            put(app, local + 0x24, 0 - at(app, local + 0x24));
        if (x86::sreg32(cpu.ebp) > 0)
        {
            cpu.ecx = at(app, local + 0x20);
            setLow8(cpu.eax, scanFlags(app, cpu.edi));
            cpu.ebp += cpu.ecx;
            const reg8 al = reg8(cpu.eax);
            if (al & kAssign)
            {
                cpu.eax = at(app, local + 0x1c);
                if (al & kFarPointer)
                    scanTarget(app, cpu, cpu.eax, true, cpu.ebx, cpu.eax, cpu.ebx);
                else if (al & kNearPointer)
                    scanTarget(app, cpu, cpu.eax, false, cpu.edx, cpu.eax, cpu.ebx);
                else
                    scanTarget(app, cpu, cpu.eax, false, cpu.esi, cpu.eax, cpu.ebx);
                setHigh8(cpu.edx, scanFlags(app, cpu.edi));
                cpu.ebx = at(app, local + 0x24);
                if (reg8(cpu.edx >> 8) & kShortVar)
                    putHalf(app, cpu.ees + cpu.eax, reg16(cpu.ebx));
                else
                    put(app, cpu.ees + cpu.eax, cpu.ebx);
            }
        }
    }
    cpu.eax = cpu.ebp;
    cpu.esp += 0x30;
    cpu.ebp = pop(app, cpu);
    popEs(app, cpu);
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi});
}

/* 0x50c650 (__scnf): the format at edx, the arguments' va_list at [ebx],
 * the input by the specs at eax: white space in the format skips the
 * input's, a character of it must be the input's next, and a '%' a
 * conversion.  How many arguments were assigned, or -1 for an input over
 * before any. */
void scnf(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 8;
    const reg32 local = cpu.esp;
    const reg32 args = local + 4;
    cpu.esi = cpu.eax;
    put(app, local, cpu.edx);
    cpu.eax = at(app, cpu.ebx);
    put(app, args, cpu.eax);
    setHigh8(cpu.eax, byte(app, cpu.esi + kScanFlags));
    cpu.ebp = 0;
    setHigh8(cpu.eax, reg8(cpu.eax >> 8) & 0xfd);
    cpu.edi = 0;
    putByte(app, cpu.esi + kScanFlags, reg8(cpu.eax >> 8));
    auto ended = [&] { return (scanFlags(app, cpu.esi) & kEndOfInput) != 0; };
    // The input's next must be ebx: false for the end of the scan.
    auto literal = [&](bool& ok) {
        cpu.eax = cpu.esi;
        ok = direct(app, cpu, cgetc);
        if (!ok)
            return false;
        if (cpu.eax == cpu.ebx)
        {
            ++cpu.edi;
            return true;
        }
        if (!ended())
        {
            cpu.edx = cpu.esi;
            ok = direct(app, cpu, uncget);
        }
        return false;
    };
    bool ok = true;
    for (;;)
    {
        cpu.ebx = at(app, local);
        cpu.edx = cpu.ebx + 1;
        put(app, local, cpu.edx);
        cpu.ebx = byte(app, cpu.ebx);
        if (cpu.ebx == 0)
            goto end;
        setLow8(cpu.eax, cpu.ebx);
        if (isKind(app, cpu, kSpace))
        {
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, scan_white))
                return;
            cpu.edi += cpu.eax;
        }
        else if (cpu.ebx != '%')
        {
            if (!literal(ok))
            {
                if (!ok)
                    return;
                goto end;
            }
        }
        else
        {
            cpu.eax = at(app, local);
            cpu.edx = cpu.esi;
            if (!direct(app, cpu, get_opt))
                return;
            cpu.ebx = 0;
            put(app, local, cpu.eax);
            cpu.ebx = byte(app, cpu.eax);
            if (cpu.ebx != 0)
            {
                ++cpu.eax;
                put(app, local, cpu.eax);
            }
            guest::Native conversion = nullptr;
            switch (cpu.ebx)
            {
            case 'd': cpu.ecx = 1; cpu.ebx = 10; conversion = scan_int; break;
            case 'i': cpu.ecx = 1; cpu.ebx = 0; conversion = scan_int; break;
            case 'o': cpu.ecx = 1; cpu.ebx = 8; conversion = scan_int; break;
            case 'u': cpu.ebx = 10; cpu.ecx = 0; conversion = scan_int; break;
            case 'x': case 'X': case 'p': cpu.ecx = 1; cpu.ebx = 16; conversion = scan_int; break;
            case 'e': case 'f': case 'g': case 'E': case 'G': conversion = scan_float; break;
            case 'S':
                putByte(app, cpu.esi + kScanFlags, scanFlags(app, cpu.esi) | kLongVar);
                conversion = scan_string;
                break;
            case 's': conversion = scan_string; break;
            case '[': cpu.ebx = local; conversion = scan_arb; break;
            case 'C':
                putByte(app, cpu.esi + kScanFlags, scanFlags(app, cpu.esi) | kLongVar);
                conversion = scan_char;
                break;
            case 'c': conversion = scan_char; break;
            case 'n':
                cpu.edx = args;
                cpu.ebx = cpu.edi;
                cpu.eax = cpu.esi;
                if (!direct(app, cpu, report_scan))
                    return;
                break;
            case '%':
                if (!literal(ok))
                {
                    if (!ok)
                        return;
                    goto end;
                }
                break;
            }
            if (conversion != nullptr)
            {
                cpu.edx = args;
                cpu.eax = cpu.esi;
                if (!direct(app, cpu, conversion))
                    return;
                cpu.ebx = cpu.eax;
                if (x86::sreg32(cpu.ebx) <= 0)
                    goto end;
                cpu.edi += cpu.ebx;
                if (scanFlags(app, cpu.esi) & kAssign)
                    ++cpu.ebp;
            }
        }
        if (ended())
            break;
    }
    // The input over: a %n where the format goes on is still reported.
    cpu.eax = at(app, local);
    if (byte(app, cpu.eax) == '%')
    {
        cpu.ebx = cpu.eax + 1;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ebx;
        put(app, local, cpu.ebx);
        if (!direct(app, cpu, get_opt))
            return;
        put(app, local, cpu.eax);
        if (byte(app, cpu.eax) == 'n')
        {
            cpu.edx = args;
            cpu.ebx = cpu.edi;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, report_scan))
                return;
        }
    }
end:
    cpu.eax = cpu.ebp == 0 && ended() ? 0xffffffff : cpu.ebp;
    cpu.esp += 8;
    scanEpilogue(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx});
}

/* 0x4f0520 (vsscanf): the string at eax scanned by the format at edx, the
 * arguments' va_list at ebx -- __scnf with sscanf's routines in its specs. */
void vsscanf(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 0x14;
    cpu.ecx = kCgetString;
    cpu.esi = kUncgetString;
    put(app, cpu.esp + kScanString, cpu.eax);
    cpu.eax = cpu.esp;
    put(app, cpu.esp + kScanGet, cpu.ecx);
    put(app, cpu.esp + kScanUnget, cpu.esi);
    if (!direct(app, cpu, scnf))
        return;
    cpu.esp += 0x14;
    scanEpilogue(app, cpu, {&cpu.esi, &cpu.ecx});
}

/* 0x4f0548 (sscanf): vsscanf's, its arguments on the stack. */
void sscanf(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 4;
    cpu.eax = cpu.esp + 0x18;
    cpu.ebx = cpu.esp;
    cpu.edx = at(app, cpu.esp + 0x14);
    put(app, cpu.esp, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x10);
    if (!direct(app, cpu, vsscanf))
        return;
    cpu.esp += 4;
    scanEpilogue(app, cpu, {&cpu.edx, &cpu.ebx});
}

/* 0x51663a (the long double's normalizing): the 96 bits in edx:eax:ebp,
 * exponent edi, shifted up until the top one is set, then rounded to 64 by
 * ebp's top bit; the exponent in esi -- 0 for no bits at all. */
void normalize(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esi = cpu.eax | cpu.edx | cpu.ebp;
    if (cpu.esi != 0)
    {
        for (int i = 0; i < 2; ++i)
            if (cpu.edx == 0)
            {
                cpu.edx = cpu.eax;
                cpu.eax = cpu.ebp;
                cpu.ebp = 0;
                cpu.edi -= 0x20;
            }
        while (!(cpu.edx & 0x80000000))
        {
            --cpu.edi;
            shift96(cpu.edx, cpu.eax, cpu.ebp);
        }
        const reg32 round = cpu.ebp >> 31;
        cpu.ebp <<= 1;
        if (round)
        {
            const x86::reg64 middle = x86::reg64(cpu.eax) + round;
            const x86::reg64 high = x86::reg64(cpu.edx) + (middle >> 32);
            cpu.eax = reg32(middle);
            cpu.edx = reg32(high);
            if (high >> 32)
            {
                cpu.edx = cpu.edx >> 1 | 0x80000000;
                ++cpu.edi;
            }
        }
        cpu.esi = cpu.edi;
    }
    cpu.esp += 4;
    NFS2_USE(app);
}

/* 0x5165d8 (__ZBuf2LD): the decimal digits at eax as the long double at
 * edx -- an integer, 96 bits of it, normalized. */
void ZBuf2LD(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.esi = cpu.eax;
    cpu.edx = 0;
    cpu.ecx = 0;
    cpu.ebp = 0;
    for (cpu.eax = byte(app, cpu.esi); cpu.eax != 0; cpu.eax = byte(app, cpu.esi))
    {
        // Times 10: times 4, plus itself, times 2.
        cpu.edi = cpu.edx;
        cpu.ebx = cpu.ecx;
        cpu.eax = cpu.ebp;
        shift96(cpu.edx, cpu.ecx, cpu.ebp);
        shift96(cpu.edx, cpu.ecx, cpu.ebp);
        add96(cpu.edx, cpu.ecx, cpu.ebp, cpu.edi, cpu.ebx, cpu.eax);
        shift96(cpu.edx, cpu.ecx, cpu.ebp);
        cpu.eax = byte(app, cpu.esi) & 0xf;
        add96(cpu.edx, cpu.ecx, cpu.ebp, 0, 0, cpu.eax);
        ++cpu.esi;
    }
    cpu.eax = cpu.ecx;
    cpu.edi = 0x405e;
    if (!direct(app, cpu, normalize))
        return;
    cpu.ebp = pop(app, cpu);
    put(app, cpu.ebp + 4, cpu.edx);
    put(app, cpu.ebp, cpu.eax);
    putHalf(app, cpu.ebp + 8, reg16(cpu.esi));
    scanEpilogue(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edi, &cpu.esi, &cpu.ebp});
}

namespace
{

/* 0x5166cf: the scaling's way out, its frame's registers back. */
void scaleReturn(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp = cpu.ebp - 0x10;
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx, &cpu.ebp});
}

}  // namespace

/* 0x51667b (the scaling's powers): the long double at edx made 10 to the edx
 * -- 0x2000 at most -- by the table's powers of 10^2^n it multiplies the
 * long double at eax by. */
void scalePowers(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.esp -= 0xc;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    if (x86::sreg32(cpu.edx) >= 0x2000)
        cpu.esi = 0x2000;
    for (cpu.ecx = kPowersOfTen; x86::sreg32(cpu.esi) > 0; cpu.ecx += 0xa)
    {
        if (cpu.esi & 1)
        {
            cpu.eax = (cpu.eax & 0xffff0000) | half(app, cpu.ecx + 8);
            putHalf(app, cpu.ebp - 0x14, reg16(cpu.eax));
            cpu.eax = at(app, cpu.ecx + 4);
            put(app, cpu.ebp - 0x18, cpu.eax);
            cpu.edx = cpu.ebp - 0x1c;
            cpu.eax = at(app, cpu.ecx);
            cpu.ebx = cpu.edi;
            put(app, cpu.ebp - 0x1c, cpu.eax);
            cpu.eax = cpu.edi;
            const x86::Float a = x86::Float(app->getMemory<x86::IEEEf80>(cpu.eax));
            const x86::Float b = x86::Float(app->getMemory<x86::IEEEf80>(cpu.edx));
            const x86::Float product = cpu.fpu.mul(a, b);
            app->getMemory<x86::IEEEf80>(cpu.ebx) = x86::Float(product);
        }
        cpu.esi = reg32(x86::sreg32(cpu.esi) >> 1);
    }
    scaleReturn(app, cpu);
}

/* 0x5166cf: 0x51667b's way out, which 0x5166d8 jumps to. */
void scaleEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    scaleReturn(app, cpu);
}

/* 0x5166d8 (the scaling by a power): the long double at eax times 10 to the
 * edx -- over it for a negative one -- the x87 rounding to its 64 bits. */
void scaleBy(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.esp -= 0xc;
    cpu.ecx = cpu.eax;
    if (cpu.edx == 0)
        return scaleReturn(app, cpu);
    push(app, cpu, 0);
    app->getMemory<x86::reg16>(cpu.esp) = cpu.fpu.control.word;
    cpu.eax = pop(app, cpu);
    cpu.esi = cpu.eax;
    setHigh8(cpu.eax, (cpu.eax >> 8) | 3);
    cpu.eax &= 0xffff;
    cpu.ebx = 0x3fff;
    push(app, cpu, cpu.eax);
    cpu.fpu.setControl(app->getMemory<x86::reg16>(cpu.esp));
    cpu.eax = pop(app, cpu);
    cpu.edi = 0;
    putHalf(app, cpu.ebp - 0x14, reg16(cpu.ebx));
    cpu.ebx = 0x80000000;
    put(app, cpu.ebp - 0x1c, cpu.edi);
    put(app, cpu.ebp - 0x18, cpu.ebx);
    const bool divide = x86::sreg32(cpu.edx) < 0;
    cpu.eax = cpu.ebp - 0x1c;
    if (divide)
        cpu.edx = 0 - cpu.edx;
    if (!direct(app, cpu, scalePowers))
        return;
    cpu.ebx = cpu.ecx;
    cpu.edx = cpu.ebp - 0x1c;
    cpu.eax = cpu.ecx;
    {
        const x86::Float a = x86::Float(app->getMemory<x86::IEEEf80>(cpu.eax));
        const x86::Float b = x86::Float(app->getMemory<x86::IEEEf80>(cpu.edx));
        const x86::Float result = divide ? cpu.fpu.div(a, b) : cpu.fpu.mul(a, b);
        app->getMemory<x86::IEEEf80>(cpu.ebx) = x86::Float(result);
    }
    cpu.eax = cpu.esi & 0xffff;
    push(app, cpu, cpu.eax);
    cpu.fpu.setControl(app->getMemory<x86::reg16>(cpu.esp));
    cpu.eax = pop(app, cpu);
    scaleReturn(app, cpu);
}

/* 0x516759 (__LDScale10x): the long double at eax times 10 to the edx, in
 * steps of 10^4096 at most. */
void LDScale10x(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    if (x86::sreg32(cpu.edx) > 0x1000)
    {
        cpu.edx = 0x1000;
        if (!direct(app, cpu, scaleBy))
            return;
        cpu.ebx -= 0x1000;
    }
    else if (x86::sreg32(cpu.edx) < -0x1000)
    {
        cpu.edx = 0xfffff000;
        if (!direct(app, cpu, scaleBy))
            return;
        cpu.ebx += 0x1000;
    }
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, scaleBy))
        return;
    scanEpilogue(app, cpu, {&cpu.ecx, &cpu.ebx});
}

/* 0x4f8122 (__Strtold): the number at eax -- white space, a sign, digits
 * with a point among them, an exponent -- as the long double at edx, from
 * its first 19 significant digits; the end of it at [ebx], unless ebx is 0.
 * 0 for no digits (the long double 0), 3 for a number too big for a double,
 * 2 for one too small, 1 for any other. */
void Strtold(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.esp -= 0x34;
    const reg32 frame = cpu.ebp;
    cpu.edi = cpu.edx;
    put(app, frame - 0x18, cpu.ebx);
    put(app, frame - 0x20, cpu.eax);
    auto dl = [&] { return reg8(cpu.edx); };
    for (;; ++cpu.eax)
    {
        setLow8(cpu.edx, byte(app, cpu.eax));
        if (dl() != ' ' && (dl() < 9 || dl() > 0xd))
            break;
    }
    cpu.ebx = cpu.eax + 1;
    setLow8(cpu.ecx, 0);
    if (dl() == '+')
        cpu.eax = cpu.ebx;
    else if (dl() == '-')
    {
        setLow8(cpu.ecx, 1);
        cpu.eax = cpu.ebx;
    }
    cpu.ebx = 0;
    setHigh8(cpu.edx, '0');
    put(app, frame - 0x1c, cpu.ebx);
    for (;;)
    {
        setLow8(cpu.edx, byte(app, cpu.eax));
        ++cpu.eax;
        if (dl() == '.')
        {
            if (cpu.ecx & 8)
                break;
            cpu.ecx |= 8;
            continue;
        }
        if (dl() < '0' || dl() > '9')
            break;
        if (cpu.ecx & 8)
            put(app, frame - 0x1c, at(app, frame - 0x1c) + 1);
        setHigh8(cpu.edx, reg8(cpu.edx >> 8) | dl());
        if (reg8(cpu.edx >> 8) != '0')
        {
            if (x86::sreg32(cpu.ebx) < 0x13)
                putByte(app, cpu.ebx + frame - 0x40, dl());
            ++cpu.ebx;
        }
        cpu.ecx |= 4;
    }
    cpu.esi = 0;
    if (cpu.ecx & 4)
    {
        if (dl() == 'e' || dl() == 'E')
        {
            cpu.edx = cpu.eax - 1;
            setHigh8(cpu.ecx, byte(app, cpu.eax));
            put(app, frame - 0x10, cpu.edx);
            cpu.edx = cpu.eax + 1;
            const reg8 ch = reg8(cpu.ecx >> 8);
            if (ch == '+')
                cpu.eax = cpu.edx;
            else if (ch == '-')
            {
                cpu.ecx |= 2;
                cpu.eax = cpu.edx;
            }
            cpu.ecx &= 0xfffffffb;
            for (;; ++cpu.eax)
            {
                setLow8(cpu.edx, byte(app, cpu.eax));
                if (dl() < '0' || dl() > '9')
                    break;
                if (x86::sreg32(cpu.esi) < 0x3e8)
                {
                    cpu.esi *= 10;
                    put(app, frame - 0x14, cpu.esi);
                    cpu.esi = dl() + at(app, frame - 0x14) - 0x30;
                }
                cpu.ecx |= 4;
            }
            if (cpu.ecx & 2)
                cpu.esi = 0 - cpu.esi;
            if (!(cpu.ecx & 4))
                cpu.eax = at(app, frame - 0x10);
        }
        else
            --cpu.eax;
        put(app, frame - 0x20, cpu.eax);
    }
    if (at(app, frame - 0x18) != 0)
    {
        cpu.edx = at(app, frame - 0x18);
        cpu.eax = at(app, frame - 0x20);
        put(app, cpu.edx, cpu.eax);
    }
    cpu.esi -= at(app, frame - 0x1c);
    if (x86::sreg32(cpu.ebx) > 0x13)
    {
        cpu.ebx -= 0x13;
        cpu.esi += cpu.ebx;
        cpu.ebx = 0x13;
    }
    while (x86::sreg32(cpu.ebx) > 0 && byte(app, cpu.ebx + frame - 0x41) == '0')
    {
        ++cpu.esi;
        --cpu.ebx;
    }
    if (cpu.ebx == 0)
    {
        putHalf(app, cpu.edi + 8, 0);
        put(app, cpu.edi + 4, cpu.ebx);
        put(app, cpu.edi, cpu.ebx);
        cpu.eax = 0;
    }
    else
    {
        setLow8(cpu.edx, 0);
        cpu.eax = frame - 0x40;
        putByte(app, cpu.ebx + frame - 0x40, 0);
        cpu.edx = frame - 0x2c;
        if (!direct(app, cpu, ZBuf2LD))
            return;
        if (cpu.esi != 0)
        {
            cpu.eax = frame - 0x2c;
            cpu.edx = cpu.esi;
            if (!direct(app, cpu, LDScale10x))
                return;
        }
        if (cpu.ecx & 1)
            putByte(app, frame - 0x23, byte(app, frame - 0x23) | 0x80);
        cpu.eax = at(app, frame - 0x24);
        putHalf(app, cpu.edi + 8, reg16(cpu.eax));
        cpu.eax = at(app, frame - 0x28);
        put(app, cpu.edi + 4, cpu.eax);
        cpu.eax = at(app, frame - 0x2c);
        put(app, cpu.edi, cpu.eax);
        cpu.eax = cpu.esi + cpu.ebx - 1;
        if (x86::sreg32(cpu.eax) > 0x134)
            cpu.eax = 3;
        else if (x86::sreg32(cpu.eax) < -0x134)
            cpu.eax = 2;
        else
            cpu.eax = 1;
    }
    cpu.esp = frame - 0xc;
    scanEpilogue(app, cpu, {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebp});
}

/* 0x4f82b6 (strtod): the number at eax as a double, in st(0) -- HUGE_VAL
 * with its sign, and ERANGE, for one too big; 0 or a denormal, and ERANGE,
 * for one too small; its end at [edx], unless edx is 0. */
void strtod(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.esp -= 0x24;
    const reg32 frame = cpu.ebp;
    cpu.ebx = cpu.edx;
    cpu.edx = frame - 0x2c;
    if (!direct(app, cpu, Strtold))
        return;
    cpu.ebx = cpu.eax;
    // The long double made the double at ebp-0x18, by the x87.
    auto toDouble = [&] {
        cpu.edx = frame - 0x18;
        cpu.eax = frame - 0x2c;
        const x86::Float value = x86::Float(app->getMemory<x86::IEEEf80>(cpu.eax));
        app->getMemory<double>(cpu.edx) = double(value);
    };
    bool range = false;
    if (cpu.eax == 0)
    {
        put(app, frame - 0x18, cpu.eax);
        put(app, frame - 0x14, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, frame - 0x24);
        setHigh8(cpu.eax, (cpu.eax >> 8) & 0x7f);
        cpu.eax &= 0xffff;
        if (x86::sreg32(cpu.eax) >= 0x43ff)
        {
            if (!direct(app, cpu, set_ERANGE))
                return;
            if (byte(app, frame - 0x23) & 0x80)
            {
                const x86::Float huge = x86::Float(app->getMemory<double>(kHugeVal));
                const x86::Float negative = -huge;
                app->getMemory<double>(frame - 0x18) = double(negative);
            }
            else
            {
                cpu.eax = at(app, kHugeVal);
                put(app, frame - 0x18, cpu.eax);
                cpu.eax = at(app, kHugeVal + 4);
                put(app, frame - 0x14, cpu.eax);
            }
        }
        else if (x86::sreg32(cpu.eax) < 0x3bcd)
        {
            if (x86::sreg32(cpu.eax) < -0x34)
            {
                cpu.ecx = 0;
                if (!direct(app, cpu, set_ERANGE))
                    return;
                put(app, frame - 0x18, cpu.ecx);
                put(app, frame - 0x14, cpu.ecx);
            }
            else
            {
                toDouble();
                cpu.eax = at(app, frame - 0x18);
                put(app, frame - 0x10, cpu.eax);
                cpu.eax = at(app, frame - 0x14);
                cpu.ebx = at(app, frame - 0x18);
                put(app, frame - 0xc, cpu.eax);
                range = ((cpu.eax & 0x7fffffff) == 0 && cpu.ebx == 0) || !(half(app, frame - 0xa) & 0x7ff0);
            }
        }
        else
        {
            toDouble();
            if (cpu.ebx == 3)
                range = true;
            else
            {
                cpu.eax = at(app, frame - 0x18);
                put(app, frame - 0x20, cpu.eax);
                cpu.eax = at(app, frame - 0x14);
                put(app, frame - 0x1c, cpu.eax);
                range = !(half(app, frame - 0x1a) & 0x7ff0);
            }
        }
    }
    if (range && !direct(app, cpu, set_ERANGE))
        return;
    const x86::Float result = x86::Float(app->getMemory<double>(frame - 0x18));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = result;
    cpu.esp = frame - 8;
    scanEpilogue(app, cpu, {&cpu.ecx, &cpu.ebx, &cpu.ebp});
}


/* ---- The console: getch, getche, kbhit and putch, on the console's input
 * and output Windows hands the runtime by CreateFileA -- or the default
 * windowing's hooks, where there are any -- under the file's lock.  Written
 * as the code is, as the environment's: each push and pop to the guest's
 * stack, each call with the return address's slot, the registers as the code
 * leaves them; the calls to Windows through the game's imports. ---- */
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

/* One INPUT_RECORD from the console at ebx into the frame's first 0x14
 * bytes, how many at its 0x14th: Windows' `function`. */
bool consoleInput(win32::WinApplication* app, x86::CPU& cpu, reg32 function, reg32 local)
{
    cpu.eax = local + 0x14;
    push(app, cpu, cpu.eax);
    push(app, cpu, 1);
    cpu.eax = local;
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ebx);
    return indirect(app, cpu, at(app, function));
}

/* The console's HANDLE opened, if it is not yet, at `handle`: CreateFileA of
 * `name`, for `access`, shared as `share`. */
bool openConsole(win32::WinApplication* app, x86::CPU& cpu, reg32 handle, reg32 name, reg32 access, reg32 share)
{
    if (at(app, handle) != 0xffffffff)
        return true;
    for (const reg32 word : {reg32(0), reg32(0x80), reg32(3), reg32(0), share, access, name})
        push(app, cpu, word);
    if (!indirect(app, cpu, at(app, kCreateFileA)))
        return false;
    put(app, handle, cpu.eax);
    return true;
}

}  // namespace

/* 0x515690 (__NTRealKey): whether the INPUT_RECORD at eax is a key pressed,
 * not shift, control or alt alone. */
void NTRealKey(win32::WinApplication* app, x86::CPU& cpu)
{
    if (half(app, cpu.eax) == 1 && at(app, cpu.eax + 4) != 0)
    {
        cpu.eax = (cpu.eax & 0xffff0000) | half(app, cpu.eax + 0xa);
        const reg16 key = reg16(cpu.eax);
        cpu.eax = key < 0x10 || key > 0x12 ? 1 : 0;
    }
    else
        cpu.eax = 0;
    cpu.esp += 4;
}

/* 0x5156b8 (the console's handles): conin$ and conout$ opened, the ones not
 * yet, under the file's lock. */
void openConsoles(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.eax = 0;
    if (!indirect(app, cpu, at(app, kAccessFile)))
        return;
    if (!openConsole(app, cpu, kConsoleIn, kConinName, 0x80000000, 1)
        || !openConsole(app, cpu, kConsoleOut, kConoutName, 0x40000000, 2))
        return;
    cpu.eax = 0;
    if (!indirect(app, cpu, at(app, kReleaseFile)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x515728 (__NTConsoleInput): the console's input HANDLE. */
void NTConsoleInput(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!direct(app, cpu, openConsoles))
        return;
    returns(cpu, at(app, kConsoleIn));
}

/* 0x515734 (__NTConsoleOutput): the console's output HANDLE. */
void NTConsoleOutput(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!direct(app, cpu, openConsoles))
        return;
    returns(cpu, at(app, kConsoleOut));
}

/* 0x52b290 (putch): the character in eax to the console -- by the default
 * windowing's putch, where there is one, else WriteConsoleA; eax back. */
void putch(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 8;
    const reg32 local = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.edx = at(app, kWindowsPutch);
    putByte(app, local + 4, reg8(cpu.ebx));
    if (cpu.edx != 0)
    {
        cpu.eax = 1;
        cpu.edx = cpu.ebx;
        if (!indirect(app, cpu, at(app, kIsWindowedHandle)) || !indirect(app, cpu, at(app, kWindowsPutch)))
            return;
    }
    else
    {
        cpu.eax = 1;
        if (!indirect(app, cpu, at(app, kAccessFile)) || !direct(app, cpu, NTConsoleOutput))
            return;
        push(app, cpu, cpu.edx);
        cpu.edx = cpu.esp + 4;
        push(app, cpu, cpu.edx);
        push(app, cpu, 1);
        cpu.edx = cpu.esp + 0x10;
        push(app, cpu, cpu.edx);
        push(app, cpu, cpu.eax);
        if (!indirect(app, cpu, at(app, kWriteConsoleA)))
            return;
        cpu.eax = 1;
        if (!indirect(app, cpu, at(app, kReleaseFile)))
            return;
    }
    cpu.eax = cpu.ebx;
    cpu.esp += 8;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f7ce0 (kbhit): whether a key is waiting -- the console's input looked
 * at, what is not a key taken off it -- or the default windowing's say. */
void kbhit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 0x18;
    const reg32 local = cpu.esp;
    if (at(app, kWindowsKbhit) != 0)
    {
        cpu.eax = 0;
        if (!indirect(app, cpu, at(app, kIsWindowedHandle)) || !indirect(app, cpu, at(app, kWindowsKbhit)))
            return;
    }
    else
    {
        cpu.eax = 0;
        if (!indirect(app, cpu, at(app, kAccessFile)) || !direct(app, cpu, NTConsoleInput))
            return;
        cpu.ebx = cpu.eax;
        for (;;)
        {
            // Where the game's code lets the other threads run, as kbhit
            // waits on the console (THREAD_SEGMENTS), and its loop's
            // safepoint.
            app->unlockContext(cpu);
            win32::Thread::sleep(0);
            app->lockContext(cpu);
            if (app->contextWanted())
                app->yieldContext(cpu);
            if (!consoleInput(app, cpu, kPeekConsoleInputA, local))
                return;
            if (at(app, local + 0x14) == 0)
                break;
            cpu.eax = local;
            if (!direct(app, cpu, NTRealKey))
                return;
            if (cpu.eax != 0)
                break;
            if (!consoleInput(app, cpu, kReadConsoleInputA, local))
                return;
        }
        cpu.eax = 0;
        if (!indirect(app, cpu, at(app, kReleaseFile)))
            return;
        cpu.eax = at(app, local + 0x14) != 0 ? 1 : 0;
    }
    cpu.esp += 0x18;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f7e20 (do_getch): the next character from the console at eax, as getch
 * gives it: a character repeated as many times as Windows says, an extended
 * key as 0 and its scan code; -1 for none. */
void do_getch(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    cpu.esp -= 0x18;
    const reg32 local = cpu.esp;
    cpu.esi = at(app, kKeyRepeats);
    cpu.edi = at(app, kKeyChar);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, kKeyState);
    if (cpu.eax == 1)
    {
        --cpu.esi;
        if (cpu.edi == 0)
            put(app, kKeyState, 2);
        else if (cpu.esi == 0)
            put(app, kKeyState, cpu.esi);
        cpu.eax = cpu.edi;
    }
    else if (cpu.eax == 2)
    {
        cpu.eax = cpu.esi != 0 ? 1 : 0;
        put(app, kKeyState, cpu.eax);
        cpu.eax = at(app, kKeyScan);
    }
    else
    {
        put(app, kKeyRepeats, cpu.esi);
        put(app, kKeyChar, cpu.edi);
        for (;;)
        {
            if (!consoleInput(app, cpu, kReadConsoleInputA, local))
                return;
            if (cpu.eax == 0)
            {
                cpu.eax = 0xffffffff;
                cpu.edi = at(app, kKeyChar);
                cpu.esi = at(app, kKeyRepeats);
                break;
            }
            cpu.eax = local;
            if (!direct(app, cpu, NTRealKey))
                return;
            if (cpu.eax == 0)
                continue;
            cpu.eax = half(app, local + 8);
            cpu.esi = cpu.eax - 1;
            setHigh8(cpu.eax, byte(app, local + 0x11));
            cpu.edi = byte(app, local + 0xe);
            if ((reg8(cpu.eax >> 8) & 1) || cpu.edi == 0)
            {
                // An extended key: 0 now, its scan code next.
                cpu.ebp = 2;
                cpu.eax = 0;
                cpu.edi = 0;
                cpu.eax = half(app, local + 0xc);
                put(app, kKeyState, cpu.ebp);
                put(app, kKeyScan, cpu.eax);
            }
            else if (cpu.esi != 0)
                put(app, kKeyState, 1);
            cpu.eax = cpu.edi;
            break;
        }
    }
    put(app, kKeyChar, cpu.edi);
    put(app, kKeyRepeats, cpu.esi);
    cpu.esp += 0x18;
    for (reg32* r : {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f7f24 (getch): the next character, not echoed: ungetch's, the default
 * windowing's, or the console's with its mode off for it. */
void getch(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 4;
    const reg32 local = cpu.esp;
    cpu.eax = at(app, kUngotKey);
    if (cpu.eax != 0)
    {
        cpu.esi = 0;
        put(app, kUngotKey, cpu.esi);
    }
    else
    {
        if (at(app, kWindowsGetch) != 0)
        {
            if (!indirect(app, cpu, at(app, kIsWindowedHandle)) || !indirect(app, cpu, at(app, kWindowsGetch)))
                return;
            cpu.esi = cpu.eax;
        }
        else
        {
            if (!indirect(app, cpu, at(app, kAccessFile)) || !direct(app, cpu, NTConsoleInput))
                return;
            cpu.ebx = cpu.eax;
            cpu.eax = local;
            push(app, cpu, cpu.eax);
            push(app, cpu, cpu.ebx);
            if (!indirect(app, cpu, at(app, kGetConsoleMode)))
                return;
            push(app, cpu, 0);
            push(app, cpu, cpu.ebx);
            if (!indirect(app, cpu, at(app, kSetConsoleMode)))
                return;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, do_getch))
                return;
            cpu.ecx = at(app, local);
            push(app, cpu, cpu.ecx);
            push(app, cpu, cpu.ebx);
            cpu.esi = cpu.eax;
            if (!indirect(app, cpu, at(app, kSetConsoleMode)))
                return;
            cpu.eax = 0;
            if (!indirect(app, cpu, at(app, kReleaseFile)))
                return;
        }
        cpu.eax = cpu.esi;
    }
    cpu.esp += 4;
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x522e60 (getche): getch's character, echoed by putch -- unless it was
 * ungetch's. */
void getche(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.ebx = at(app, kUngotKey);
    if (!direct(app, cpu, getch))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0xffffffff && cpu.ebx == 0 && !direct(app, cpu, putch))
        return;
    cpu.eax = cpu.edx;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}


/* ---- Structured exception handling (clib/startup/a/seh386.asm): __try's
 * registration on fs:[0]'s chain, and __TryHandler, which Windows hands an
 * exception and which runs the scopes' filters, unwinding to the one that
 * takes it.  A filter is jumped to with ebp the function's frame and the
 * handler's registers pushed, and calls __Except with its say in eax; the
 * scopes' finally blocks are called.  As the generated code has these: a jmp
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

/* 0x4f7af8 (__TryInit): the registration at eax put first on fs:[0]'s chain,
 * at no level yet. */
void TryInit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, at(app, cpu.efs));
    put(app, cpu.eax, pop(app, cpu));
    put(app, cpu.eax + 4, kTryHandler);
    put(app, cpu.eax + 8, cpu.ebp);
    putByte(app, cpu.eax + 0x10, 0xff);
    putByte(app, cpu.eax + 0x11, 0);
    put(app, cpu.efs, cpu.eax);
    cpu.esp += 4;
}

/* 0x4f7b8c (__TryHandler's end): ExceptionContinueSearch, 1, the handler's
 * registers back. */
void TryDone(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = 1;
    for (reg32* r : {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

void TryOuter(win32::WinApplication* app, x86::CPU& cpu);

/* 0x4f7b61 (__TryHandler's scopes): from the level in ebx out, the first
 * scope with a filter jumped to -- one without, a finally block, passed by. */
void TryScope(win32::WinApplication* app, x86::CPU& cpu)
{
    if (reg8(cpu.ebx) == 0xff)
        return TryDone(app, cpu);
    putByte(app, cpu.ebp + 0x12, reg8(cpu.ebx));
    cpu.ebx *= 3;
    if (byte(app, cpu.edi + cpu.ebx * 2 + 1) != 0)
        return TryOuter(app, cpu);
    push(app, cpu, cpu.ebp);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebx);
    cpu.ebp = at(app, cpu.ebp + 8);
    app->dynamic_call(at(app, cpu.edi + cpu.ebx * 2 + 2), cpu);
}

/* 0x4f7b87 (__TryHandler's next scope): the level the scope at ebx is in. */
void TryOuter(win32::WinApplication* app, x86::CPU& cpu)
{
    setLow8(cpu.ebx, byte(app, cpu.edi + cpu.ebx * 2));
    TryScope(app, cpu);
}

/* 0x4f7bc3 (__local_unwind): the finally blocks from the registration's
 * level, at ebp, out to the level unwound to, each called with ebp its
 * frame. */
void local_unwind(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebp, cpu.edi, cpu.esi, cpu.edx, cpu.ecx, cpu.ebx})
        push(app, cpu, r);
    cpu.edi = at(app, cpu.ebp + 0xc);
    cpu.ebx = byte(app, cpu.ebp + 0x10);
    putByte(app, cpu.ebp + 0x11, 1);
    while (reg8(cpu.ebx) != 0xff && reg8(cpu.ebx) != byte(app, cpu.ebp + 0x12))
    {
        cpu.ebx *= 3;
        if (byte(app, cpu.edi + cpu.ebx * 2 + 1) != 0)
        {
            push(app, cpu, cpu.ebp);
            push(app, cpu, cpu.edi);
            push(app, cpu, cpu.ebx);
            cpu.ebp = at(app, cpu.ebp + 8);
            if (!indirect(app, cpu, at(app, cpu.edi + cpu.ebx * 2 + 2)))
                return;
            cpu.ebx = pop(app, cpu);
            cpu.edi = pop(app, cpu);
            cpu.ebp = pop(app, cpu);
        }
        setLow8(cpu.ebx, byte(app, cpu.edi + cpu.ebx * 2));
        putByte(app, cpu.ebp + 0x10, reg8(cpu.ebx));
    }
    putByte(app, cpu.ebp + 0x11, 0);
    for (reg32* r : {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x533de4: RtlUnwind's import, jumped to. */
void RtlUnwindThunk(win32::WinApplication* app, x86::CPU& cpu)
{
    app->dynamic_call(at(app, kRtlUnwind), cpu);
}

/* 0x4f7ba7 (__global_unwind): the registrations above the one at ebp
 * unwound, by RtlUnwind. */
void global_unwind(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebp, cpu.edi, cpu.esi, cpu.edx, cpu.ecx, cpu.ebx, reg32(0), reg32(0), kUnwindReturn,
                          cpu.ebp})
        push(app, cpu, r);
    if (!direct(app, cpu, RtlUnwindThunk))
        return;
    for (reg32* r : {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f7b2a (__TryHandler): the exception at [esp+4], for the registration at
 * [esp+8]: when unwinding, its finally blocks; else, its record and context
 * kept, its scopes' filters from its level out. */
void TryHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (const reg32 r : {cpu.edi, cpu.esi, cpu.edx, cpu.ecx, cpu.ebx})
        push(app, cpu, r);
    cpu.edi = at(app, cpu.ebp + 0xc);
    cpu.eax = at(app, cpu.ebp + 8);
    if (at(app, cpu.eax + 4) & 6)
    {
        cpu.ebp = cpu.edi;
        putByte(app, cpu.ebp + 0x12, 0xff);
        if (!direct(app, cpu, local_unwind))
            return;
        return TryDone(app, cpu);
    }
    put(app, cpu.edi + 0x14, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x10);
    put(app, cpu.edi + 0x18, cpu.eax);
    cpu.ebp = cpu.edi;
    cpu.edi = at(app, cpu.ebp + 0xc);
    cpu.ebx = byte(app, cpu.ebp + 0x10);
    TryScope(app, cpu);
}

/* 0x4f7b7d (__Except): a filter's say, in eax, on the scope its pushes
 * name: -1 the exception's code goes on (0), 0 the next scope out, else
 * the scopes unwound and the handler run -- here, back to the filter's
 * caller. */
void Except(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.ebp = pop(app, cpu);
    ++cpu.eax;
    if (cpu.eax == 0)
    {
        for (reg32* r : {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp})
            *r = pop(app, cpu);
        cpu.esp += 4;
        return;
    }
    --cpu.eax;
    if (cpu.eax == 0)
        return TryOuter(app, cpu);
    push(app, cpu, cpu.edx);
    if (!direct(app, cpu, global_unwind) || !direct(app, cpu, local_unwind))
        return;
    cpu.ebp = at(app, cpu.ebp + 8);
    cpu.esp += 4;
}


/* ---- The start (clib/startup): _cstart_ and what it runs before the game's
 * main -- the initializers, the thread's data and its stack's limits, the
 * standard handles, the environment's and the command line's copies, the
 * exception handler -- then the game's main, by its address, and exit.
 * Written as the code is, as the environment's: each push and pop to the
 * guest's stack, each call with the return address's slot, the registers as
 * the code leaves them; the calls to Windows through the game's imports. ---- */
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

bool space(win32::WinApplication* app, x86::CPU& cpu, reg32 at)
{
    setLow8(cpu.edx, byte(app, at) + 1);
    cpu.edx &= 0xff;
    return (byte(app, kIsTable + cpu.edx) & kSpace) != 0;
}

}  // namespace

/* 0x52b640: a start-up hook that does nothing. */
void startHook(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.esp += 4;
}

/* 0x4ff41c (__InitRtns): the runtime's initializers of priority up to al
 * not yet run, the lowest first. */
void InitRtns(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi})
        push(app, cpu, r);
    pushEs(app, cpu);
    cpu.esi = kInitEnd;
    setHigh8(cpu.edx, cpu.eax);
    for (;;)
    {
        cpu.eax = kInitStart;
        cpu.ebx = cpu.esi;
        setLow8(cpu.edx, cpu.edx >> 8);
        if (cpu.esi > cpu.eax)
            do
            {
                if (byte(app, cpu.eax) != 2)
                {
                    setHigh8(cpu.ecx, byte(app, cpu.eax + 1));
                    if (reg8(cpu.edx) >= reg8(cpu.ecx >> 8))
                    {
                        cpu.ebx = cpu.eax;
                        setLow8(cpu.edx, cpu.ecx >> 8);
                    }
                }
                cpu.eax += 6;
            } while (cpu.eax < kInitEnd);
        if (cpu.ebx == kInitEnd)
            break;
        cpu.eax = cpu.ebx + 2;
        if (!direct(app, cpu, callRoutine))
            return;
        putByte(app, cpu.ebx, 2);
    }
    popEs(app, cpu);
    for (reg32* r : {&cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51ec20 (the stack's limits): where the thread's stack ends, into [eax],
 * and where it starts, into [edx] -- from VirtualQuery of the frame, the
 * guard pages Windows keeps below as its version says -- either for none. */
void stackLimits(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 0x20;
    const reg32 local = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.esi = cpu.edx;
    push(app, cpu, 0x1c);
    cpu.eax = cpu.esp + 4;
    push(app, cpu, cpu.eax);
    cpu.eax = cpu.esp + 0x24;
    push(app, cpu, cpu.eax);
    if (!indirect(app, cpu, at(app, kVirtualQuery)))
        return;
    cpu.edx = at(app, local) + at(app, local + 0xc);
    cpu.ecx = (cpu.ecx & 0xffff0000) | half(app, kOsBuild);
    cpu.eax = at(app, local + 4);
    if (reg16(cpu.ecx) < 0x8000)
        cpu.eax += 0x3000;
    else if (byte(app, kOsMajor) < 4)
        cpu.eax += 0x12000;
    else
        cpu.eax += 0x13000;
    if (cpu.ebx != 0)
        put(app, cpu.ebx, cpu.eax);
    if (cpu.esi != 0)
        put(app, cpu.esi, cpu.edx);
    cpu.esp += 0x20;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x525df0 (calloc): eax things of edx bytes, zeroed. */
void calloc(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.eax *= cpu.edx;
    cpu.ebx = cpu.eax;
    if (!direct(app, cpu, nmalloc))
        return;
    if (cpu.eax != 0)
    {
        cpu.edx = 0;
        if (!direct(app, cpu, memset))
            return;
    }
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x526030 (the thread's data's start): the data at eax -- if any -- told
 * its stack's end and its thread. */
void initThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.ebx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.edx = 0;
        put(app, cpu.eax + 0xc, 1);
        if (!direct(app, cpu, stackLimits) || !indirect(app, cpu, at(app, kGetCurrentThreadId)))
            return;
        put(app, cpu.ebx + 0xda, cpu.eax);
    }
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51e7ec (the main thread's data): the data at eax -- calloc's, for none --
 * started; the data, or 0. */
void initThreadData(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
    {
        cpu.eax = 1;
        cpu.edx = at(app, kThreadDataSize);
        if (!direct(app, cpu, calloc))
            return;
        cpu.edx = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.ebx = at(app, kThreadDataSize);
            putByte(app, cpu.eax + 0x52, 1);
            put(app, cpu.eax + 0xf0, cpu.ebx);
        }
    }
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, initThread))
        return;
    cpu.eax = cpu.edx;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51e490 (__initPOSIXHandles): the standard input, output and error
 * Windows hands -- a fake HANDLE for one it has none of -- made handles 0, 1
 * and 2. */
void initPOSIXHandles(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    for (const reg32 which : {reg32(-10), reg32(-11), reg32(-12)})
    {
        push(app, cpu, which);
        if (!indirect(app, cpu, at(app, kGetStdHandle)))
            return;
        cpu.edx = cpu.eax;
        if ((cpu.eax == 0 || cpu.eax == 0xffffffff) && !direct(app, cpu, NTGetFakeHandle))
            return;
        if (!direct(app, cpu, allocPOSIXHandle))
            return;
    }
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51eb00 (the module's wide name): the name of the module at eax into the
 * ebx bytes at edx -- by GetModuleFileNameW where Windows has it, else the
 * narrow name made wide; how long, or 0. */
void moduleWideName(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp})
        push(app, cpu, r);
    auto done = [&] {
        for (reg32* r : {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx})
            *r = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ebp = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edi = cpu.ebx;
    if (!indirect(app, cpu, at(app, kGetVersion)))
        return;
    cpu.eax = (cpu.eax >> 16) & 0xffff;
    if (reg16(cpu.eax) < 0x8000)
    {
        push(app, cpu, cpu.ebx);
        push(app, cpu, cpu.esi);
        push(app, cpu, cpu.ebp);
        if (!indirect(app, cpu, at(app, kGetModuleFileNameW)))
            return;
        return done();
    }
    cpu.eax = 0x208;
    if (!direct(app, cpu, nmalloc))
        return;
    cpu.ebx = cpu.eax;
    if (cpu.eax == 0)
        return done();
    push(app, cpu, 0x208);
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ebp);
    if (!indirect(app, cpu, at(app, kGetModuleFileNameA)))
        return;
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, nfree))
            return;
        cpu.eax = 0;
        return done();
    }
    for (const reg32 word : {cpu.edi, cpu.esi, reg32(-1), cpu.ebx, reg32(1), reg32(1)})
        push(app, cpu, word);
    if (!indirect(app, cpu, at(app, kMultiByteToWideChar)))
        return;
    cpu.ebp = cpu.eax;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, nfree))
        return;
    if (cpu.ebp == 0)
    {
        cpu.eax = 0;
        return done();
    }
    cpu.eax = cpu.esi;
    putHalf(app, cpu.esi + cpu.edi * 2 - 2, 0);
    if (!direct(app, cpu, wcslen))
        return;
    done();
}

/* 0x51eba0 (the command line's copy): the string at eax copied into a block
 * of _nmalloc's; 0 for none. */
void copyCommandLine(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ecx, cpu.edx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.esi = cpu.eax;
    cpu.edi = cpu.eax;
    inlineStrlen(app, cpu);
    ++cpu.ecx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, nmalloc))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.edi = cpu.eax;
        inlineMemcpy(app, cpu);
    }
    cpu.eax = cpu.edx;
    for (reg32* r : {&cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51ebf0 (the wide command line's copy): the wide string at eax copied
 * into a block of _nmalloc's; 0 for none. */
void copyWideCommandLine(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, wcslen))
        return;
    ++cpu.eax;
    cpu.ebx = cpu.eax * 2;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, nmalloc))
        return;
    cpu.ecx = cpu.eax;
    if (cpu.eax != 0 && !direct(app, cpu, memcpy))
        return;
    cpu.eax = cpu.ecx;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x51f0d0 (__InitExceptionHandling): the registration at eax the thread's,
 * first on fs:[0]'s chain with the runtime's handler, and the runtime's
 * filter Windows' for an exception nobody handles. */
void InitExceptionHandling(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    put(app, cpu.eax + 0x54, cpu.edx);
    cpu.eax = 0;
    cpu.eax = at(app, cpu.efs + cpu.eax);
    cpu.edx = cpu.eax;
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    cpu.eax = at(app, cpu.eax + 0x54);
    put(app, cpu.eax, cpu.edx);
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    cpu.eax = at(app, cpu.eax + 0x54);
    cpu.edx = 0;
    put(app, cpu.eax + 4, kExceptionHandlerCode);
    if (!indirect(app, cpu, at(app, kThreadPtr)))
        return;
    cpu.eax = at(app, cpu.eax + 0x54);
    put(app, cpu.efs + cpu.edx, cpu.eax);
    push(app, cpu, kUnhandledFilter);
    if (!indirect(app, cpu, at(app, kSetUnhandledExceptionFilter)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4ff108 (__NTInit): the runtime's state for a program (eax 0) or a DLL,
 * the thread's data at edx, the module ebx: the data started, the standard
 * handles, the environment, Windows' version, the names, the command lines
 * and where their arguments start.  0 for no thread's data in a DLL; a
 * program without one leaves by ExitProcess. */
void NTInit(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    auto done = [&] {
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.edi = cpu.ebx;
    cpu.eax = cpu.edx;
    put(app, kIsDll, cpu.esi);
    if (!direct(app, cpu, initThreadData))
        return;
    put(app, kMainThreadData, cpu.eax);
    if (cpu.eax == 0)
    {
        if (cpu.esi != 0)
            return done();
        push(app, cpu, 1);
        if (!indirect(app, cpu, at(app, kExitProcess)))
            return;
    }
    if (!direct(app, cpu, initPOSIXHandles) || !indirect(app, cpu, at(app, kGetEnvironmentStrings)))
        return;
    cpu.edx = 0;
    put(app, kEnvironmentStrings, cpu.eax);
    put(app, kEnviron, cpu.edx);
    if (!indirect(app, cpu, at(app, kGetVersion)))
        return;
    // Its parts: the major and minor versions, the build -- and Windows'
    // version in Watcom's own words of it.
    cpu.edx = cpu.eax;
    cpu.ebx = cpu.eax;
    putByte(app, kOsMajor, reg8(cpu.eax));
    cpu.eax = (cpu.eax >> 16) & 0xffff;
    push(app, cpu, 0x104);
    putHalf(app, kOsBuild, reg16(cpu.eax));
    cpu.eax = 0;
    cpu.edx &= 0xffff;
    cpu.eax = half(app, kOsBuild);
    push(app, cpu, kModuleName);
    put(app, kOsVersion, cpu.eax);
    cpu.eax = 0;
    cpu.edx = reg32(x86::sreg32(cpu.edx) >> 8);
    setLow8(cpu.eax, cpu.ebx);
    cpu.edx &= 0xff;
    put(app, kWinMajor, cpu.eax);
    cpu.eax = 0;
    putByte(app, kOsMinor, reg8(cpu.edx));
    setLow8(cpu.eax, cpu.edx);
    cpu.edx = at(app, kWinMajor);
    push(app, cpu, 0);
    cpu.edx <<= 8;
    cpu.ebx = kModuleName;
    cpu.edx |= cpu.eax;
    put(app, kWinMinor, cpu.eax);
    put(app, kWinVersion, cpu.edx);
    if (!indirect(app, cpu, at(app, kGetModuleFileNameA)))
        return;
    cpu.edx = kWideModuleName;
    put(app, kProgramName, cpu.ebx);
    cpu.eax = 0;
    cpu.ebx = 0x208;
    cpu.ecx = kWideModuleName;
    if (!direct(app, cpu, moduleWideName))
        return;
    put(app, kWideProgramName, cpu.ecx);

    // The command line's copy, and its arguments: past the program's name,
    // quoted or not, and the white space after it.
    if (!indirect(app, cpu, at(app, kGetCommandLineA)) || !direct(app, cpu, copyCommandLine))
        return;
    cpu.edx = cpu.eax;
    setLow8(cpu.ebx, byte(app, cpu.eax));
    put(app, kCommandLine, cpu.eax);
    if (reg8(cpu.ebx) == '"')
    {
        setHigh8(cpu.ebx, byte(app, cpu.eax + 1));
        ++cpu.eax;
        if (reg8(cpu.ebx >> 8) != reg8(cpu.ebx))
            while (byte(app, cpu.eax) != 0)
            {
                setLow8(cpu.edx, byte(app, cpu.eax + 1));
                ++cpu.eax;
                if (reg8(cpu.edx) == '"')
                    break;
            }
        if (byte(app, cpu.eax) != 0)
            ++cpu.eax;
    }
    else
        while (!space(app, cpu, cpu.eax) && byte(app, cpu.eax) != 0)
            ++cpu.eax;
    while (space(app, cpu, cpu.eax))
        ++cpu.eax;
    put(app, kArguments, cpu.eax);

    // The wide one's, the same.
    if (!indirect(app, cpu, at(app, kGetCommandLineW)))
        return;
    if (cpu.eax == 0)
        cpu.eax = kNoWideCommandLine;
    else
    {
        if (!direct(app, cpu, copyWideCommandLine))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = (cpu.ebx & 0xffff0000) | half(app, cpu.eax);
        put(app, kWideCommandLine, cpu.eax);
        if (reg16(cpu.ebx) == '"')
        {
            cpu.ecx = (cpu.ecx & 0xffff0000) | half(app, cpu.eax + 2);
            cpu.eax += 2;
            if (reg16(cpu.ecx) != reg16(cpu.ebx))
                while (half(app, cpu.eax) != 0)
                {
                    cpu.ebx = (cpu.ebx & 0xffff0000) | half(app, cpu.eax + 2);
                    cpu.eax += 2;
                    if (reg16(cpu.ebx) == '"')
                        break;
                }
            if (half(app, cpu.eax) != 0)
                cpu.eax += 2;
        }
        else
        {
            cpu.ebx = 2;
            while (!space(app, cpu, cpu.eax) && half(app, cpu.eax) != 0)
                cpu.eax += 2;
        }
        cpu.ebx = 2;
        while (space(app, cpu, cpu.eax))
            cpu.eax += 2;
    }
    put(app, kWideArguments, cpu.eax);

    // A DLL's own name, and wide.
    if (cpu.esi != 0)
    {
        push(app, cpu, 0x104);
        push(app, cpu, kDllName);
        push(app, cpu, cpu.edi);
        cpu.esi = kDllName;
        cpu.ebx = 0x208;
        if (!indirect(app, cpu, at(app, kGetModuleFileNameA)))
            return;
        cpu.edx = kWideDllName;
        put(app, kDllNamePtr, cpu.esi);
        cpu.eax = cpu.edi;
        cpu.edi = kWideDllName;
        if (!direct(app, cpu, moduleWideName))
            return;
        put(app, kWideDllNamePtr, cpu.edi);
    }
    cpu.eax = 1;
    done();
}

/* 0x4ff338 (the program's start): __NTInit for the module, the stack's
 * limits, the exception handling with the registration at eax, and the
 * initializers. */
void NTMain(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi})
        push(app, cpu, r);
    cpu.esi = cpu.eax;
    cpu.edi = cpu.edx;
    push(app, cpu, 0);
    if (!indirect(app, cpu, at(app, kGetModuleHandleA)))
        return;
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.edi;
    cpu.eax = 0;
    if (!direct(app, cpu, NTInit))
        return;
    cpu.edx = kStackTop;
    if (!indirect(app, cpu, at(app, kThreadPtr)) || !direct(app, cpu, stackLimits))
        return;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, InitExceptionHandling))
        return;
    cpu.eax = 0x21;
    if (!direct(app, cpu, InitRtns) || !indirect(app, cpu, at(app, kAccessIobHook)))
        return;
    cpu.eax = 0xff;
    if (!direct(app, cpu, InitRtns))
        return;
    for (reg32* r : {&cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x524290 (the call of main): the stack's room checked -- grown by __GRO
 * where stackavail says it is short -- and where it ends noted; then the
 * game's main with argc and argv, and exit. */
void callMain(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kStackSize) + 3;
    setLow8(cpu.edx, cpu.edx & 0xfc);
    if (!direct(app, cpu, stackavail))
        return;
    if (cpu.edx < cpu.eax)
    {
        push(app, cpu, cpu.edx);
        if (!direct(app, cpu, GRO))
            return;
        cpu.eax = at(app, kStackSize) + 3;
        setLow8(cpu.eax, cpu.eax & 0xfc);
        cpu.esp -= cpu.eax;
        cpu.eax = cpu.esp;
    }
    else
        cpu.eax = 0;
    cpu.edx = at(app, kStackSize);
    cpu.eax += cpu.edx;
    put(app, kStackLimit, cpu.eax);
    if (!direct(app, cpu, startHook))
        return;
    cpu.edx = at(app, kArgv);
    cpu.eax = at(app, kArgc);
    if (!indirect(app, cpu, kGameMain) || !indirect(app, cpu, kExitEntry))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x5158a0 (the main thread's start): the initializers of priority 1, the
 * thread's data zeroed on the stack, the program's start with a
 * registration on the stack too, and main. */
void mainStart(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const reg32 r : {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp})
        push(app, cpu, r);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.eax = 1;
    if (!direct(app, cpu, InitRtns))
        return;
    cpu.eax = at(app, kThreadDataSize) + 3;
    setLow8(cpu.eax, cpu.eax & 0xfc);
    cpu.edx = 0;
    cpu.esp -= cpu.eax;
    cpu.ecx = cpu.esp;
    cpu.ebx = at(app, kThreadDataSize);
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, memset))
        return;
    cpu.eax = at(app, kThreadDataSize);
    cpu.edx = cpu.ecx;
    put(app, cpu.ecx + 0xf0, cpu.eax);
    cpu.eax = cpu.ebp - 8;
    if (!direct(app, cpu, NTMain) || !direct(app, cpu, callMain))
        return;
    cpu.esp = cpu.ebp;
    for (reg32* r : {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx})
        *r = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f7fa0 (_cstart_): the program's entry, a jmp to the main thread's
 * start. */
void cstart_(win32::WinApplication* app, x86::CPU& cpu)
{
    mainStart(app, cpu);
}


/* ---- The maths' error report: __reporterror, which builds _matherr's struct
 * exception and prints the error when _matherr does not take it.  (The game's
 * way round the Pentium's FDIV bug, __fpatan_wrap and __fdiv_fpr, is gone: no
 * phone has it.)  The x87's instructions are the same operations of the port's
 * x87 as the generated code has for them, in the same order, so that the
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

/* fstp of a long double. */
void popLong(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    storeLong(app, address, cpu.fpu.st(0));
    cpu.fpu.count -= 1;
}

/* fld of a long double. */
void pushLong(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    pushFloat(cpu, loadLong(app, address));
}

void exchange(x86::CPU& cpu, int i)
{
    if (i != 0)
        std::swap(cpu.fpu.st(0), cpu.fpu.st(i));
}

/* fnstcw to `saved`, and fldcw of it with every exception masked, 64 bits'
 * precision and rounding to nearest -- as the dword the code makes of it, at
 * `masked`. */
void maskedControl(win32::WinApplication* app, x86::CPU& cpu, reg32 saved, reg32 masked, reg32& r)
{
    app->getMemory<x86::reg16>(saved) = cpu.fpu.control.word;
    r = at(app, saved);
    r |= 0x33f;
    r &= 0xf3ff;
    put(app, masked, r);
    cpu.fpu.setControl(app->getMemory<x86::reg16>(masked));
}

}  // namespace

/* 0x52e400 (__get_std_stream): __iob's eax'th FILE, the first four; 0 for
 * another. */
void get_std_stream(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.eax = cpu.eax > 3 ? 0 : cpu.eax * 0x1a + kStdStreams;
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52b647 (the maths' error printed): "<type's name>: <function's name>"
 * and a newline, to stderr. */
void printMathError(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax = 2;
    if (!direct(app, cpu, get_std_stream))
        return;
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, kMathTypeNames + cpu.esi * 4);
    cpu.edx = cpu.ebx;
    if (!direct(app, cpu, fputs))
        return;
    cpu.eax = kMathErrorSeparator;
    cpu.edx = cpu.ebx;
    if (!direct(app, cpu, fputs))
        return;
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, fputs))
        return;
    cpu.eax = '\n';
    cpu.edx = cpu.ebx;
    if (!direct(app, cpu, fputc))
        return;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x52b68f (__matherr's call): the struct exception at eax to _matherr; for
 * a 0 from it, the error printed, and errno EDOM for a domain error, else
 * ERANGE; its retval onto the x87's stack. */
void callMatherr(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.ebx = cpu.eax;
    if (!indirect(app, cpu, at(app, kMatherr)))
        return;
    if (cpu.eax == 0)
    {
        cpu.edx = at(app, cpu.ebx + 4);
        cpu.eax = at(app, cpu.ebx);
        if (!direct(app, cpu, printMathError)
            || !direct(app, cpu, at(app, cpu.ebx) == 1 ? set_EDOM : set_ERANGE))
            return;
    }
    const x86::Float value = loadDouble(app, cpu.ebx + 0x18);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    pushFloat(cpu, value);
    cpu.esp += 4;
}

/* 0x5243b1 (__reporterror's struct exception): the error's flags in eax --
 * which function (the low 5 bits), and of what type -- its arguments the
 * doubles at edx and ebx: the struct built, the result the type says --
 * -HUGE_VAL, 0, 1, HUGE_VAL or the second argument -- and _matherr's call. */
void reportError(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 0x20;
    const reg32 frame = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.edx);
    put(app, frame - 0x20, cpu.eax);
    cpu.eax = at(app, cpu.edx + 4);
    put(app, frame - 0x1c, cpu.eax);
    cpu.eax = at(app, cpu.ebx);
    put(app, frame - 0x18, cpu.eax);
    cpu.eax = at(app, cpu.ebx + 4);
    put(app, frame - 0x14, cpu.eax);
    const reg8 cl = reg8(cpu.ecx), ch = reg8(cpu.ecx >> 8);
    if (cl & 0x40)
        cpu.esi = 1;  // DOMAIN
    else if (cl & 0x80)
        cpu.esi = 2;  // SING
    else if (ch & 1)
        cpu.esi = 3;  // OVERFLOW
    else if (ch & 8)
        cpu.esi = 4;  // UNDERFLOW
    else if (ch & 2)
        cpu.esi = 6;  // PLOSS
    else if (ch & 4)
        cpu.esi = 5;  // TLOSS
    cpu.eax = at(app, kMathErrorNames + (cpu.ecx & 0x1f) * 4);
    put(app, frame - 0x28, cpu.esi);
    put(app, frame - 0x24, cpu.eax);
    if (ch & 0x10)
    {
        const x86::Float huge = loadDouble(app, kHugeVal);
        const x86::Float negative = -huge;
        storeDouble(app, frame - 0x10, negative);
    }
    else if (ch & 0x20)
    {
        cpu.ecx = 0;
        put(app, frame - 0x10, cpu.ecx);
        put(app, frame - 0xc, cpu.ecx);
    }
    else if (ch & 0x40)
    {
        cpu.edx = 0;
        cpu.ebx = 0x3ff00000;
        put(app, frame - 0x10, cpu.edx);
        put(app, frame - 0xc, cpu.ebx);
    }
    else if (ch & 0x80)
    {
        cpu.eax = at(app, kHugeVal);
        put(app, frame - 0x10, cpu.eax);
        cpu.eax = at(app, kHugeVal + 4);
        put(app, frame - 0xc, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebx);
        put(app, frame - 0x10, cpu.eax);
        cpu.eax = at(app, cpu.ebx + 4);
        put(app, frame - 0xc, cpu.eax);
    }
    cpu.eax = frame - 0x28;
    if (!direct(app, cpu, callMatherr))
        return;
    cpu.esp = frame - 8;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x5243a7 (__reporterror): reportError with the argument at edx for both. */
void reporterror(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.ebx = cpu.edx;
    if (!direct(app, cpu, reportError))
        return;
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace watcom
}  // namespace nfs3hp
