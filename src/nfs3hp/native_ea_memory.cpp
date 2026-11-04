#include "native_guest.h"

#include <utility>

/* EA's memory manager and its memmove.  disassemble_nfs3hp.py binds each
 * address to the function of that name here (EA_MEMORY_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code.
 *
 * The manager is EA's cmn\memstd.c (its messages name creatememclass,
 * getfreememclass, reservememadr and purgememadr).  Memory is handed out of
 * up to 16 classes, the class table at 0x9ef5c0 a pointer each; a block's
 * flags (and reservememadr's) name its class in their low nibble, and the
 * default class is 0x5643f4's.  A class is made over one stretch of memory:
 * a block "LOW" at its start whose 0x40 bytes are the class's header, one
 * free block for all between, a block "HIGH" of none at its end.
 *
 * A block: its 0x10 bytes of header, then its body, which the caller gets.
 *   +0  its signature: "MB" in use, "FB" free, "SB" the free list's sentinel
 *   +2  its flags: the class in the low nibble; 0x10 taken from the top of
 *       the class, 0x20 from its largest free block; 0x100 named, 0x200
 *       "BEND" after the body, 0x400 the file and line it was asked for at,
 *       0x800 a word cleared; 0x4000 in the free list
 *   +4  its size -- a free one's from its header up to the next block's
 *   +8  the next block in memory, +0xc the one before
 *   +10 and +14, a free one's: the next and the one before in the free list,
 *       which goes by address from the sentinel round to it again
 * After an allocated body: "BEND", the file and line, a zero word and the
 * class's overhead bytes as the flags say, then the name.
 *
 * A class's header (the LOW block's body): +0 its name, +8 and +0xc where its
 * memory starts and ends, +0x10 the free list's sentinel (its size 0x7fffffff,
 * the list's first and last at +0x20 and +0x24), +0x28 how blocks are
 * aligned, +0x2c how its start was, +0x30 the bytes each block takes past its
 * body, +0x34 its flags (0x100: a block's name counted in), +0x38 its lock
 * (0x4eb2e0's, a critical section) or none, +0x3c what its creator said.
 *
 * The game allocates and frees with reservememadr and purgememadr (the
 * sound's, the files', the menus' all), and asks a block's size and name.
 * clearMemClasses, releasememclass, getfreememclass, memBlockFlags and
 * purgememclass have no callers in the game.
 *
 * Its memmove (0x4ea4f0) copies backwards a byte at a time where the copy
 * overlaps its source from above, and otherwise by the forward copy (0x5033e5,
 * mix::copy).
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frames' locals in the guest's memory,
 * the registers as the code leaves them, each x87 instruction on the x87's
 * stack in its order.  reservememadr, purgememadr and memmove are called by
 * address where this file calls them (tools/game_checks stands in for them),
 * as are Error (0x401010), the game's fill (0x4e070c), sprintf (0x4df690),
 * the locks made and freed (0x4eb2e0, 0x4eb360), the forward copy, and the
 * critical sections' Windows functions. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

// The class table, and the default class; whether an error is reported.
constexpr reg32 kClasses = 0x9ef5c0, kDefaultClass = 0x5643f4, kReportErrors = 0x56440c;
// Where an error is from: the file, the function, the line.
constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// Where the call is from, if anywhere: its file, its line.
constexpr reg32 kCalledFile = 0x55219c, kCalledLine = 0x5521a4;
// The signatures.
constexpr reg16 kInUse = 0x424d, kFree = 0x4246, kSentinel = 0x4253;
// memstd.c's strings: its file's name, its functions', its messages.
constexpr reg32 kFileName = 0x549944;
constexpr reg32 kCreateName = 0x549954, kCreateOutOfRange = 0x549964;
constexpr reg32 kLowName = 0x5499ac, kHighName = 0x5499b4;
constexpr reg32 kGetFreeName = 0x5499bc, kNoFreeClasses = 0x5499cc;
constexpr reg32 kReserveName = 0x549a00, kInvalidSize = 0x549a10, kOutOfMemory = 0x549a6c;
constexpr reg32 kPurgeName = 0x549ab8, kNotABlock = 0x549ac4;
// What is called by address.
constexpr reg32 kError = 0x401010, kFillZero = 0x4e070c, kSprintf = 0x4df690;
constexpr reg32 kLockMade = 0x4eb2e0, kLockFreed = 0x4eb360;
constexpr reg32 kReserve = 0x4e1620, kPurge = 0x4e1890, kForwardCopy = 0x5033e5;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

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

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* The two words copied at a time from esi to edi up to the first zero byte
 * -- the zero too: esi and edi left past the last word copied, al the last
 * byte read. */
void copyName(win32::WinApplication* app, x86::CPU& cpu)
{
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            break;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            break;
    }
}

/* sub ecx, ecx; dec ecx; xor eax, eax; repne scasb; not ecx; dec ecx; inc
 * ecx: ecx the length of the string at edi and its zero, edi past it, eax 0. */
void nameLength(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    while (cpu.ecx != 0)
    {
        --cpu.ecx;
        const reg8 b = byte(app, cpu.edi);
        ++cpu.edi;
        if (b == cpu.al)
            break;
    }
    cpu.ecx = ~cpu.ecx;
    --cpu.ecx;
    ++cpu.ecx;
}

/* The error's place set: memstd.c, the function, the line. */
void setWhere(win32::WinApplication* app, reg32 file, reg32 function, reg32 line)
{
    put(app, kWhereFile, file);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
}

}  // namespace

void findFreeBlock(win32::WinApplication* app, x86::CPU& cpu);
void findLargestFreeBlock(win32::WinApplication* app, x86::CPU& cpu);
void linkFreeBlock(win32::WinApplication* app, x86::CPU& cpu);
void unlinkFreeBlock(win32::WinApplication* app, x86::CPU& cpu);
void clearMemClasses(win32::WinApplication* app, x86::CPU& cpu);
void markMemBlock(win32::WinApplication* app, x86::CPU& cpu);
void memClassOverhead(win32::WinApplication* app, x86::CPU& cpu);
void memBlockOverhead(win32::WinApplication* app, x86::CPU& cpu);
void creatememclass(win32::WinApplication* app, x86::CPU& cpu);
void releasememclass(win32::WinApplication* app, x86::CPU& cpu);
void getfreememclass(win32::WinApplication* app, x86::CPU& cpu);
void largestmem(win32::WinApplication* app, x86::CPU& cpu);
void largestmemclass(win32::WinApplication* app, x86::CPU& cpu);
void memBlockSize(win32::WinApplication* app, x86::CPU& cpu);
void memBlockName(win32::WinApplication* app, x86::CPU& cpu);
void memBlockFlags(win32::WinApplication* app, x86::CPU& cpu);
void reservememadr(win32::WinApplication* app, x86::CPU& cpu);
void purgememadr(win32::WinApplication* app, x86::CPU& cpu);
void purgememclass(win32::WinApplication* app, x86::CPU& cpu);
void reservememStdcall(win32::WinApplication* app, x86::CPU& cpu);
void purgememStdcall(win32::WinApplication* app, x86::CPU& cpu);
void memmove(win32::WinApplication* app, x86::CPU& cpu);

/* 0x4e1000: the first free block of class eax of at least edx bytes, from
 * the free list's start (ebx 0) or its end; 0 if none -- the sentinel. */
void findFreeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax += 0x10;
    const reg32 link = cpu.ebx == 0 ? 0x10 : 0x14;
    do
        cpu.eax = at(app, cpu.eax + link);
    while (sreg32(cpu.edx) > sreg32(at(app, cpu.eax + 4)));
    if (half(app, cpu.eax) == kSentinel)
        cpu.eax = 0;
    cpu.esp += 4;
}

/* 0x4e1030: the largest free block of class eax of at least edx bytes (of
 * any, edx 0 or less), the free list gone through from its start (ebx 0) or
 * its end -- of those as large, the first; 0 if none.  edx left its size
 * less one, or the one asked for less one; ebx, from the end, the last
 * size looked at. */
void findLargestFreeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.eax += 0x10;
    --cpu.edx;
    cpu.ecx = 0;
    if (sreg32(cpu.edx) < 0)
        cpu.edx = 0;
    if (cpu.ebx == 0)
    {
        for (;;)
        {
            cpu.eax = at(app, cpu.eax + 0x10);
            cpu.esi = at(app, cpu.eax + 4);
            if (sreg32(cpu.edx) >= sreg32(cpu.esi))
                continue;
            if (half(app, cpu.eax) == kSentinel)
                break;
            cpu.edx = cpu.esi;
            cpu.ecx = cpu.eax;
        }
    }
    else
    {
        for (;;)
        {
            cpu.eax = at(app, cpu.eax + 0x14);
            cpu.ebx = at(app, cpu.eax + 4);
            if (sreg32(cpu.edx) >= sreg32(cpu.ebx))
                continue;
            if (half(app, cpu.eax) == kSentinel)
                break;
            cpu.edx = cpu.ebx;
            cpu.ecx = cpu.eax;
        }
    }
    cpu.eax = cpu.ecx;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1080: block edx put into class eax's free list where its address
 * goes -- looked for from the list's end if it is past the middle of the
 * first and last, else from its start -- as "FB", its size up to the next
 * block, flagged 0x4000.  eax left the block after it in the list, its
 * second byte the block's flags' high byte; edx the one before. */
void linkFreeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.ecx = cpu.edx;
    cpu.ebx = at(app, cpu.ecx + 8);
    cpu.ebx -= cpu.ecx;
    cpu.esi = cpu.ebx - 0x10;
    cpu.edi = at(app, cpu.eax + 0x20);
    cpu.ebx = at(app, cpu.eax + 0x24);
    cpu.ebx -= cpu.edi;
    cpu.ebx >>= 1;
    cpu.eax += 0x10;
    cpu.ebx += cpu.edi;
    cpu.edx = cpu.eax;
    if (cpu.ecx > cpu.ebx)
    {
        do
            cpu.edx = at(app, cpu.edx + 0x14);
        while (cpu.ecx < cpu.edx);
        cpu.eax = at(app, cpu.edx + 0x10);
    }
    else
    {
        do
            cpu.eax = at(app, cpu.eax + 0x10);
        while (cpu.ecx > cpu.eax);
        cpu.edx = at(app, cpu.eax + 0x14);
    }
    put(app, cpu.ecx + 0x14, cpu.edx);
    put(app, cpu.ecx + 4, cpu.esi);
    put(app, cpu.ecx + 0x10, cpu.eax);
    put(app, cpu.edx + 0x10, cpu.ecx);
    put(app, cpu.eax + 0x14, cpu.ecx);
    cpu.ah = byte(app, cpu.ecx + 3);
    putHalf(app, cpu.ecx, kFree);
    cpu.ah |= 0x40;
    putByte(app, cpu.ecx + 3, cpu.ah);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e10e0: free block edx taken out of its free list (eax, its class, not
 * looked at), its signature cleared and its 0x4000 flag.  eax left the
 * block before it in the list, its second byte the block's flags' high
 * byte. */
void unlinkFreeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    cpu.eax = at(app, cpu.edx + 0x14);
    cpu.ecx = at(app, cpu.edx + 0x10);
    put(app, cpu.eax + 0x10, cpu.ecx);
    put(app, cpu.ecx + 0x14, cpu.eax);
    cpu.ah = byte(app, cpu.edx + 3);
    putHalf(app, cpu.edx, 0);
    cpu.ah &= 0xbf;
    putByte(app, cpu.edx + 3, cpu.ah);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1100: the class table cleared; 1.  No callers in the game. */
void clearMemClasses(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = 0x40;
    cpu.eax = kClasses;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.eax = 1;
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1120: block eax marked in use ("MB"): flags the first stack word,
 * size ebx, the block before and after the second and third; after its body
 * "BEND", the file and line it is asked for at and a zero word as the flags
 * say, ecx bytes of the class's, and name edx if there is one and the flags
 * say so.  eax the bytes from the block to past all that; the three words
 * taken off. */
void markMemBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edi = cpu.ecx;
    cpu.ecx = at(app, cpu.esp + 0x10);
    putHalf(app, cpu.eax, kInUse);
    putHalf(app, cpu.eax + 2, cpu.cx);
    put(app, cpu.eax + 4, cpu.ebx);
    cpu.eax = at(app, cpu.esp + 0x18);
    cpu.edx = cpu.ebp + 0x10;
    put(app, cpu.ebp + 8, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x14);
    cpu.edx += cpu.ebx;
    put(app, cpu.ebp + 0xc, cpu.eax);
    if (cpu.ch & 2)
    {
        cpu.eax = 0x42454e44;
        cpu.eax = __builtin_bswap32(cpu.eax);
        put(app, cpu.edx, cpu.eax);
    }
    if (cpu.ch & 4)
    {
        cpu.ebx = at(app, kCalledFile);
        if (cpu.ebx == 0)
        {
            cpu.eax = at(app, kWhereFile);
            put(app, cpu.edx + 4, cpu.eax);
            cpu.eax = at(app, kWhereLine);
        }
        else
        {
            cpu.eax = at(app, kCalledLine);
            put(app, cpu.edx + 4, cpu.ebx);
        }
        put(app, cpu.edx + 8, cpu.eax);
    }
    if (cpu.ch & 8)
        put(app, cpu.edx + 0xc, 0);
    cpu.edx += cpu.edi;
    if (cpu.esi != 0 && (cpu.ch & 1))
    {
        cpu.edi = cpu.edx;
        push(app, cpu, cpu.edi);
        copyName(app, cpu);
        cpu.edi = pop(app, cpu);
        nameLength(app, cpu);
        cpu.edx += cpu.ecx;
    }
    cpu.eax = cpu.edx;
    cpu.eax -= cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x4e11e0: the bytes a block of class eax takes past its body. */
void memClassOverhead(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax &= 0xf;
    cpu.eax = at(app, cpu.eax * 4 + kClasses);
    cpu.eax = at(app, cpu.eax + 0x30);
    cpu.esp += 4;
}

/* 0x4e11f0: the bytes a block of class edx named eax takes past its body:
 * the class's, and the name and its zero if the class counts names in.  edx
 * left the class. */
void memBlockOverhead(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    cpu.edx &= 0xf;
    cpu.ecx = 0;
    cpu.edx = at(app, cpu.edx * 4 + kClasses);
    if (cpu.eax != 0 && (byte(app, cpu.edx + 0x35) & 1))
    {
        push(app, cpu, cpu.edi);
        cpu.edi = cpu.eax;
        nameLength(app, cpu);
        cpu.edi = pop(app, cpu);
    }
    cpu.eax = at(app, cpu.edx + 0x30);
    cpu.eax += cpu.ecx;
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1230: class eax (0 to 15, else an error reported, if errors are, and
 * 0) made, named edx, over the ecx bytes at ebx: its header in a block "<name>
 * LOW" there, a free block from where the stack's second word aligns past
 * it, a block "<name> HIGH" at the end.  The stack's words: how blocks are
 * aligned, how the free block is, the bytes each block takes past its body;
 * whether blocks have "BEND" after them (flag 0x200), the file and line
 * (0x400), their names (0x100); whether the class has a lock; a word kept.
 * The free block's size given back; the eight words taken off. */
void creatememclass(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x110;
    cpu.esi = cpu.eax;
    cpu.edi = cpu.edx;
    put(app, cpu.esp + 0x104, cpu.ebx);
    put(app, cpu.esp + 0x10c, cpu.eax);
    if (sreg32(cpu.eax) < 0 || sreg32(cpu.eax) >= 0x10)
    {
        if (at(app, kReportErrors) != 0)
        {
            push(app, cpu, 0x10);
            cpu.ebp = kFileName;
            cpu.eax = kCreateName;
            push(app, cpu, cpu.esi);
            cpu.edx = 0x11a;
            put(app, kWhereFile, cpu.ebp);
            push(app, cpu, kCreateOutOfRange);
            put(app, kWhereFunction, cpu.eax);
            put(app, kWhereLine, cpu.edx);
            if (!byAddress(app, cpu, kError))
                return;
            cpu.esp += 0xc;
        }
        cpu.eax = 0;
        cpu.esp += 0x110;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 4 + 0x20;
        return;
    }
    if (at(app, cpu.esp + 0x12c) != 0)
    {
        cpu.ah |= 2;
        put(app, cpu.esp + 0x10c, cpu.eax);
    }
    if (at(app, cpu.esp + 0x130) != 0)
        putByte(app, cpu.esp + 0x10d, byte(app, cpu.esp + 0x10d) | 4);
    if (at(app, cpu.esp + 0x134) != 0)
        putByte(app, cpu.esp + 0x10d, byte(app, cpu.esp + 0x10d) | 1);
    // Where the free block starts, aligned; where the class ends.
    cpu.eax = at(app, cpu.esp + 0x104);
    cpu.edx = at(app, cpu.esp + 0x128);
    cpu.eax += 0x50;
    cpu.edx += cpu.eax;
    cpu.eax = at(app, cpu.esp + 0x124);
    cpu.edx += 0x18;
    --cpu.eax;
    cpu.edx += cpu.eax;
    cpu.eax = ~cpu.eax;
    push(app, cpu, cpu.edi);
    cpu.edx &= cpu.eax;
    push(app, cpu, kLowName);
    cpu.edx -= 0x10;
    cpu.eax = at(app, cpu.esp + 0x10c);
    put(app, cpu.esp + 0x110, cpu.edx);
    cpu.ecx += cpu.eax;
    cpu.edx = at(app, cpu.esp + 0x130);
    cpu.ebp = cpu.eax + 0x10;
    cpu.ecx -= 0x18;
    cpu.eax = cpu.esp + 8;
    cpu.ecx -= cpu.edx;
    push(app, cpu, cpu.eax);
    put(app, cpu.esp + 0x10c, cpu.ecx);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    // The block "<name> LOW", the header's.
    cpu.ecx = at(app, cpu.esp + 0x108);
    push(app, cpu, cpu.ecx);
    cpu.ebx = 0x40;
    cpu.eax = at(app, cpu.esp + 0x110);
    push(app, cpu, 0);
    cpu.ah |= 0x80;
    cpu.ecx = cpu.edx;
    push(app, cpu, cpu.eax);
    cpu.edx = cpu.esp + 0xc;
    cpu.eax = at(app, cpu.esp + 0x110);
    if (!direct(app, cpu, markMemBlock))
        return;
    // The free block, as one in use for now.
    cpu.ebx = at(app, cpu.esp + 0x100);
    cpu.eax = at(app, cpu.esp + 0x104);
    push(app, cpu, cpu.ebx);
    cpu.edx = at(app, cpu.esp + 0x110);
    cpu.ecx = at(app, cpu.esp + 0x10c);
    push(app, cpu, cpu.eax);
    cpu.ebx -= cpu.ecx;
    cpu.ecx = at(app, cpu.esp + 0x130);
    cpu.eax = at(app, cpu.esp + 0x110);
    push(app, cpu, cpu.edx);
    cpu.ebx -= 0x10;
    cpu.edx = 0;
    if (!direct(app, cpu, markMemBlock))
        return;
    // The block "<name> HIGH".
    push(app, cpu, cpu.edi);
    push(app, cpu, kHighName);
    cpu.eax = cpu.esp + 8;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.edx = cpu.esp;
    push(app, cpu, 0);
    cpu.ebx = at(app, cpu.esp + 0x10c);
    cpu.eax = at(app, cpu.esp + 0x110);
    push(app, cpu, cpu.ebx);
    cpu.ax |= 0x8010;
    cpu.ecx = at(app, cpu.esp + 0x130);
    push(app, cpu, cpu.eax);
    cpu.ebx = 0;
    cpu.eax = at(app, cpu.esp + 0x10c);
    if (!direct(app, cpu, markMemBlock))
        return;
    // The class's header: in the table, cleared, named.
    cpu.eax = cpu.esi;
    cpu.eax &= 0xf;
    cpu.edx = 0x40;
    cpu.esi = cpu.edi;
    put(app, cpu.eax * 4 + kClasses, cpu.ebp);
    cpu.eax = cpu.ebp;
    cpu.edi = cpu.ebp;
    if (!byAddress(app, cpu, kFillZero))
        return;
    push(app, cpu, cpu.edi);
    copyName(app, cpu);
    cpu.edi = pop(app, cpu);
    putHalf(app, cpu.ebp + 0x10, kSentinel);
    cpu.eax = at(app, cpu.esp + 0x104);
    put(app, cpu.ebp + 0x14, 0x7fffffff);
    put(app, cpu.ebp + 8, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x100);
    put(app, cpu.ebp + 0xc, cpu.eax);
    cpu.eax = cpu.ebp + 0x10;
    put(app, cpu.ebp + 0x20, cpu.eax);
    put(app, cpu.ebp + 0x24, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x120);
    put(app, cpu.ebp + 0x28, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x124);
    put(app, cpu.ebp + 0x2c, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x128);
    put(app, cpu.ebp + 0x30, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x10c);
    cpu.edx = at(app, cpu.esp + 0x108);
    put(app, cpu.ebp + 0x34, cpu.eax);
    cpu.eax = at(app, cpu.esp + 0x13c);
    cpu.esi = at(app, cpu.esp + 0x138);
    put(app, cpu.ebp + 0x3c, cpu.eax);
    cpu.eax = cpu.ebp;
    put(app, cpu.ebp + 0x38, 0);
    // The free block into the free list; the lock.
    if (!direct(app, cpu, linkFreeBlock))
        return;
    if (cpu.esi != 0)
    {
        if (!byAddress(app, cpu, kLockMade))
            return;
        put(app, cpu.ebp + 0x38, cpu.eax);
    }
    cpu.eax = at(app, cpu.esp + 0x108);
    cpu.eax = at(app, cpu.eax + 4);
    cpu.esp += 0x110;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0x20;
}

/* 0x4e14b0: class eax's lock freed, if it has one, and its header cleared;
 * 1.  It stays in the table.  No callers in the game. */
void releasememclass(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.eax &= 0xf;
    cpu.ecx = at(app, cpu.eax * 4 + kClasses);
    cpu.edx = at(app, cpu.ecx + 0x38);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kLockFreed))
            return;
    }
    cpu.edx = 0x40;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.eax = 1;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1500: the first class from 1 on with none in the table (1 straight
 * away if 1 has none; 16 if all have) -- 16 an error reported, if errors
 * are.  No callers in the game. */
void getfreememclass(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.eax = 4;
    cpu.ecx = at(app, kClasses + 4);
    cpu.edx = 1;
    if (cpu.ecx != 0)
    {
        do
        {
            cpu.eax += 4;
            ++cpu.edx;
        } while (sreg32(cpu.eax) < 0x40 && at(app, cpu.eax + kClasses) != 0);
    }
    if (at(app, kReportErrors) != 0 && sreg32(cpu.edx) >= 0x10)
    {
        push(app, cpu, cpu.edi);
        cpu.edi = kFileName;
        push(app, cpu, 0x10);
        cpu.ebp = kGetFreeName;
        cpu.eax = 0x1ae;
        push(app, cpu, kNoFreeClasses);
        setWhere(app, cpu.edi, cpu.ebp, cpu.eax);
        if (!byAddress(app, cpu, kError))
            return;
        cpu.esp += 8;
        cpu.edi = pop(app, cpu);
    }
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1580: the size of the default class's largest free block, or 0. */
void largestmem(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.eax = at(app, kDefaultClass);
    cpu.eax &= 0xf;
    cpu.ebx = 0;
    cpu.edx = 0;
    cpu.eax = at(app, cpu.eax * 4 + kClasses);
    if (!direct(app, cpu, findLargestFreeBlock))
        return;
    if (cpu.eax != 0)
        cpu.eax = at(app, cpu.eax + 4);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e15b0: the size of class eax's largest free block, or 0. */
void largestmemclass(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.eax &= 0xf;
    cpu.ebx = 0;
    cpu.edx = 0;
    cpu.eax = at(app, cpu.eax * 4 + kClasses);
    if (!direct(app, cpu, findLargestFreeBlock))
        return;
    if (cpu.eax != 0)
        cpu.eax = at(app, cpu.eax + 4);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e15d0: the size asked for of the block at eax. */
void memBlockSize(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = at(app, cpu.eax - 0xc);
    cpu.esp += 4;
}

/* 0x4e15e0: the name of the block at eax, past its body and its class's
 * bytes; 0 for one not named (flag 0x100). */
void memBlockName(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.ebx = cpu.edx - 0x10;
    cpu.ecx = 0;
    cpu.cx = half(app, cpu.ebx + 2);
    cpu.eax = 0;
    if (cpu.ch & 1)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, memClassOverhead))
            return;
        cpu.edx += at(app, cpu.ebx + 4);
        cpu.eax += cpu.edx;
    }
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1610: the flags of the block at eax.  No callers in the game. */
void memBlockFlags(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ax = half(app, cpu.eax - 0xe);
    cpu.eax &= 0xffff;
    cpu.esp += 4;
}

/* 0x4e1620: edx bytes (8 at least) of class ebx & 0xf, named eax (or 0),
 * flags ebx with the class's 0x700 -- under the class's lock, if it has one:
 * the first free block large enough from the bottom (from the top, flag
 * 0x10; the largest, 0x20), split if more than 0x40 bytes are left over --
 * the rest left free below (above, from the top) -- and marked in use (its
 * size the one asked for, 0x100 kept only for a name).  The body's address,
 * or 0 for none large enough or a size less than 0, each an error reported
 * if errors are. */
void reservememadr(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x14;
    put(app, cpu.esp + 0xc, cpu.eax);
    cpu.ebp = cpu.edx;
    put(app, cpu.esp + 0x10, cpu.ebx);
    cpu.eax = cpu.ebx;
    cpu.eax &= 0xf;
    cpu.edi = at(app, cpu.eax * 4 + kClasses);
    cpu.edx = 0;
    cpu.ecx = at(app, cpu.edi + 0x38);
    put(app, cpu.esp + 4, cpu.edx);
    if (cpu.ecx != 0)
    {
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kEnterCriticalSection)))
            return;
    }
    cpu.ecx = cpu.ebp;
    if (sreg32(cpu.ebp) < 8)
    {
        if (sreg32(cpu.ebp) < 0)
        {
            // A size less than 0.
            cpu.esi = at(app, cpu.edi + 0x38);
            if (cpu.esi != 0)
            {
                push(app, cpu, cpu.esi);
                if (!byAddress(app, cpu, at(app, kLeaveCriticalSection)))
                    return;
            }
            if (at(app, kReportErrors) != 0)
            {
                cpu.ecx = kFileName;
                cpu.ebx = kReserveName;
                cpu.esi = 0x2ae;
                cpu.edi = at(app, cpu.esp + 0x10);
                put(app, kWhereFile, cpu.ecx);
                put(app, kWhereFunction, cpu.ebx);
                push(app, cpu, cpu.edi);
                cpu.eax = cpu.edi;
                put(app, kWhereLine, cpu.esi);
                if (!direct(app, cpu, largestmemclass))
                    return;
                push(app, cpu, cpu.eax);
                push(app, cpu, cpu.ebp);
                cpu.ebp = at(app, cpu.esp + 0x18);
                push(app, cpu, cpu.ebp);
                push(app, cpu, kInvalidSize);
                if (!byAddress(app, cpu, kError))
                    return;
                cpu.esp += 0x14;
            }
            cpu.eax = 0;
            cpu.esp += 0x14;
            cpu.ebp = pop(app, cpu);
            cpu.edi = pop(app, cpu);
            cpu.esi = pop(app, cpu);
            cpu.ecx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
        cpu.ecx = 8;
    }
    // What the block takes, aligned; a block for it.
    cpu.edx = at(app, cpu.esp + 0x10);
    cpu.eax = at(app, cpu.esp + 0xc);
    if (!direct(app, cpu, memBlockOverhead))
        return;
    cpu.ebx = at(app, cpu.edi + 0x28);
    cpu.eax += cpu.ecx;
    --cpu.ebx;
    cpu.eax += 0x10;
    put(app, cpu.esp + 8, cpu.ebx);
    cpu.ecx = cpu.ebx + cpu.eax;
    cpu.eax = cpu.ebx;
    cpu.eax = ~cpu.eax;
    cpu.eax &= cpu.ecx;
    cpu.ecx = cpu.eax - 0x10;
    cpu.ebx = at(app, cpu.esp + 0x10);
    cpu.ah = byte(app, cpu.esp + 0x10);
    cpu.ebx &= 0x10;
    const bool largest = (cpu.ah & 0x20) != 0;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.edi;
    if (!direct(app, cpu, largest ? findLargestFreeBlock : findFreeBlock))
        return;
    cpu.esi = cpu.eax;
    if (cpu.esi == 0)
    {
        // None large enough.
        if (at(app, kReportErrors) != 0)
        {
            cpu.edx = at(app, cpu.edi + 0x38);
            if (cpu.edx != 0)
            {
                push(app, cpu, cpu.edx);
                if (!byAddress(app, cpu, at(app, kLeaveCriticalSection)))
                    return;
            }
            cpu.ebx = kFileName;
            cpu.esi = kReserveName;
            cpu.edi = 0x2ed;
            cpu.eax = at(app, cpu.esp + 0x10);
            put(app, kWhereFile, cpu.ebx);
            push(app, cpu, cpu.eax);
            put(app, kWhereFunction, cpu.esi);
            put(app, kWhereLine, cpu.edi);
            if (!direct(app, cpu, largestmemclass))
                return;
            push(app, cpu, cpu.eax);
            push(app, cpu, cpu.ebp);
            cpu.edx = at(app, cpu.esp + 0x18);
            push(app, cpu, cpu.edx);
            push(app, cpu, kOutOfMemory);
            if (!byAddress(app, cpu, kError))
                return;
            cpu.esp += 0x14;
            cpu.eax = 0;
            cpu.esp += 0x14;
            cpu.ebp = pop(app, cpu);
            cpu.edi = pop(app, cpu);
            cpu.esi = pop(app, cpu);
            cpu.ecx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
    }
    else
    {
        cpu.edx = cpu.esi;
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, unlinkFreeBlock))
            return;
        cpu.eax = at(app, cpu.esi + 4);
        cpu.eax -= cpu.ecx;
        if (sreg32(cpu.eax) > 0x40)
        {
            if (byte(app, cpu.esp + 0x10) & 0x10)
            {
                // From the top: the rest, aligned down, left free below.
                cpu.ecx = at(app, cpu.esp + 8);
                cpu.ecx = ~cpu.ecx;
                cpu.eax &= cpu.ecx;
                put(app, cpu.esp, cpu.esi);
                cpu.esi += cpu.eax;
                cpu.eax = at(app, cpu.esp);
                cpu.eax = at(app, cpu.eax + 8);
                put(app, cpu.eax + 0xc, cpu.esi);
                cpu.eax = at(app, cpu.esp);
                cpu.eax = at(app, cpu.eax + 8);
                put(app, cpu.esi + 8, cpu.eax);
                cpu.eax = at(app, cpu.esp);
                put(app, cpu.esi + 0xc, cpu.eax);
                push(app, cpu, cpu.esi);
                cpu.ebx = at(app, cpu.eax + 0xc);
                push(app, cpu, cpu.ebx);
                cpu.edx = 0;
                push(app, cpu, 0);
                cpu.ecx = 0;
                cpu.ebx = 0;
                if (!direct(app, cpu, markMemBlock))
                    return;
                cpu.edx = at(app, cpu.esp);
                cpu.eax = cpu.edi;
                if (!direct(app, cpu, linkFreeBlock))
                    return;
            }
            else
            {
                // From the bottom: the rest left free above.
                cpu.eax = cpu.esi + 0x10;
                cpu.eax += cpu.ecx;
                put(app, cpu.esp, cpu.eax);
                cpu.eax = at(app, cpu.esi + 8);
                cpu.ecx = at(app, cpu.esp);
                put(app, cpu.eax + 0xc, cpu.ecx);
                cpu.ecx = at(app, cpu.esi + 8);
                push(app, cpu, cpu.ecx);
                push(app, cpu, cpu.esi);
                cpu.ebx = 0;
                cpu.edx = 0;
                push(app, cpu, 0);
                cpu.eax = at(app, cpu.esp + 0xc);
                cpu.ecx = 0;
                if (!direct(app, cpu, markMemBlock))
                    return;
                cpu.edx = at(app, cpu.esp);
                cpu.eax = cpu.edi;
                if (!direct(app, cpu, linkFreeBlock))
                    return;
                cpu.eax = at(app, cpu.esp);
                put(app, cpu.esi + 8, cpu.eax);
            }
        }
        // The block marked in use.
        cpu.eax = at(app, cpu.edi + 0x34);
        cpu.edx = at(app, cpu.esp + 0x10);
        cpu.eax &= 0x700;
        cpu.edx |= cpu.eax;
        cpu.ecx = at(app, cpu.esp + 0xc);
        put(app, cpu.esp + 0x10, cpu.edx);
        if (cpu.ecx == 0)
            putByte(app, cpu.esp + 0x11, byte(app, cpu.esp + 0x11) & 0xfe);
        cpu.ebx = at(app, cpu.esi + 8);
        push(app, cpu, cpu.ebx);
        cpu.eax = at(app, cpu.esi + 0xc);
        cpu.edx = at(app, cpu.esp + 0x14);
        push(app, cpu, cpu.eax);
        cpu.ecx = at(app, cpu.edi + 0x30);
        cpu.ebx = cpu.ebp;
        push(app, cpu, cpu.edx);
        cpu.eax = cpu.esi;
        cpu.edx = at(app, cpu.esp + 0x18);
        cpu.esi += 0x10;
        if (!direct(app, cpu, markMemBlock))
            return;
        put(app, cpu.esp + 4, cpu.esi);
    }
    cpu.ecx = at(app, cpu.edi + 0x38);
    if (cpu.ecx != 0)
    {
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kLeaveCriticalSection)))
            return;
    }
    cpu.eax = at(app, cpu.esp + 4);
    cpu.esp += 0x14;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1890: the block at eax given back to its class, under its lock if it
 * has one: merged with the block before it and the one after if they are
 * free, into the free list.  1 -- for none too; 0 for one not in use ("MB"),
 * an error reported, if errors are, with its signature. */
void purgememadr(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esi = cpu.eax;
    cpu.ebp = 1;
    if (cpu.eax != 0)
    {
        cpu.ax = half(app, cpu.eax - 0xe);
        cpu.ah = 0;
        cpu.al &= 0xf;
        cpu.edx = cpu.esi - 0x10;
        cpu.eax &= 0xffff;
        cpu.cx = half(app, cpu.edx);
        cpu.ebx = at(app, cpu.eax * 4 + kClasses);
        if (cpu.cx != kInUse)
        {
            if (at(app, kReportErrors) != 0)
            {
                cpu.eax = kFileName;
                cpu.ecx = kPurgeName;
                cpu.ebx = 0x333;
                put(app, kWhereFile, cpu.eax);
                put(app, kWhereFunction, cpu.ecx);
                cpu.ecx = 2;
                cpu.eax = cpu.edx;
                put(app, kWhereLine, cpu.ebx);
                cpu.eax = at(app, cpu.eax);
                cpu.eax = __builtin_bswap32(cpu.eax);
                cpu.ecx = 0 - cpu.ecx;
                cpu.ecx = cpu.ecx * 8 + 0x20;
                cpu.eax >>= cpu.cl & 0x1f;
                push(app, cpu, cpu.eax);
                push(app, cpu, cpu.edx);
                push(app, cpu, cpu.esi);
                push(app, cpu, kNotABlock);
                if (!byAddress(app, cpu, kError))
                    return;
                cpu.esp += 0x10;
            }
            cpu.eax = 0;
            cpu.ebp = pop(app, cpu);
            cpu.edi = pop(app, cpu);
            cpu.esi = pop(app, cpu);
            cpu.edx = pop(app, cpu);
            cpu.ecx = pop(app, cpu);
            cpu.ebx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
        cpu.edx = at(app, cpu.ebx + 0x38);
        if (cpu.edx != 0)
        {
            push(app, cpu, cpu.edx);
            if (!byAddress(app, cpu, at(app, kEnterCriticalSection)))
                return;
        }
        cpu.ecx = cpu.esi - 0x10;
        cpu.edi = at(app, cpu.ecx + 0xc);
        cpu.ah = byte(app, cpu.edi + 3);
        cpu.esi = at(app, cpu.ecx + 8);
        if (cpu.ah & 0x40)
        {
            // Into the free block before it.
            cpu.edx = cpu.edi;
            cpu.eax = cpu.ebx;
            cpu.ecx = cpu.edi;
            if (!direct(app, cpu, unlinkFreeBlock))
                return;
            cpu.edi = at(app, cpu.edi + 0xc);
            put(app, cpu.ecx + 8, cpu.esi);
            put(app, cpu.edi + 8, cpu.ecx);
            put(app, cpu.esi + 0xc, cpu.ecx);
        }
        if (byte(app, cpu.esi + 3) & 0x40)
        {
            // The free block after it into it.
            cpu.edx = cpu.esi;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, unlinkFreeBlock))
                return;
            cpu.esi = at(app, cpu.esi + 8);
            put(app, cpu.ecx + 0xc, cpu.edi);
            put(app, cpu.ecx + 8, cpu.esi);
            put(app, cpu.esi + 0xc, cpu.ecx);
        }
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, linkFreeBlock))
            return;
        cpu.esi = at(app, cpu.ebx + 0x38);
        if (cpu.esi != 0)
        {
            push(app, cpu, cpu.esi);
            if (!byAddress(app, cpu, at(app, kLeaveCriticalSection)))
                return;
        }
    }
    cpu.eax = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e1990: every block of class eax given back, from the start of its
 * memory on, the next of each read after it is -- its LOW and HIGH blocks
 * too, which are not free blocks to give back.  No callers in the game. */
void purgememclass(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.eax &= 0xf;
    cpu.edx = at(app, cpu.eax * 4 + kClasses);
    cpu.edx = at(app, cpu.edx + 8);
    while (cpu.edx != 0)
    {
        cpu.eax = cpu.edx + 0x10;
        if (!byAddress(app, cpu, kPurge))
            return;
        cpu.edx = at(app, cpu.edx + 8);
    }
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4e19d0: the stack word's bytes of the default class, not named -- what
 * the files' code (0x4eb160) takes a buffer with; stdcall. */
void reservememStdcall(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.edx = at(app, cpu.esp + 8);
    cpu.ebx = at(app, kDefaultClass);
    cpu.eax = 0;
    if (!byAddress(app, cpu, kReserve))
        return;
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x4e19f0: the block at the stack word given back, as the files' code
 * (0x4eb160) frees its buffer; stdcall. */
void purgememStdcall(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = at(app, cpu.esp + 4);
    if (!byAddress(app, cpu, kPurge))
        return;
    cpu.esp += 4 + 4;
}

/* 0x4ea4f0: ebx bytes from eax to edx: a byte at a time from the end where
 * edx is within them above eax, else by the MMX copy, the x87's or dwords as
 * 0x5643b4 and 0x5643ac say -- each handed eax the destination, edx the
 * source.  What each leaves in eax, edx and ebx left. */
void memmove(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    cpu.ecx = cpu.edx;
    if (cpu.edx > cpu.eax)
    {
        cpu.edx = cpu.eax + cpu.ebx;
        if (cpu.ecx < cpu.edx)
        {
            cpu.eax = cpu.edx;
            cpu.edx = cpu.ecx + cpu.ebx;
            for (;;)
            {
                --cpu.ebx;
                if (cpu.ebx == 0xffffffff)
                    break;
                --cpu.edx;
                cpu.cl = byte(app, cpu.eax - 1);
                --cpu.eax;
                putByte(app, cpu.edx, cpu.cl);
            }
            cpu.ecx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
    }
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kForwardCopy))
        return;
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace ea
}  // namespace nfs3hp
