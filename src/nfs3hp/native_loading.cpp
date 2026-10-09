#include <nfs3hp.h>
#include "native_thrash.h"
#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <utility>
#include <vector>

/* Native stand-ins for the loading of the game's data, entered from the top of
 * the generated functions (tools/apply_native_loading.py) as the vertex loops
 * are (native_vertices.cpp):
 *
 *   - EA's RefPack decoder (sub_5102a4), behind every .qfs and every packed
 *     file the game reads: the track's textures, the sky, the HUD, the menus.
 *     The generated code moves each byte of its output as a loop of its own
 *     (rep movsb, one emulated step per byte);
 *   - the FSH textures' alpha scans (sub_4d18e0 ... sub_4d1ad0): every pixel of
 *     every texture looked at once, to tell an opaque texture from a keyed or
 *     a translucent one, before THRASH takes it;
 *   - an FCE model turned into the game's polygon records (sub_49cfa0);
 *   - a track's FRD file read into its blocks (sub_419c20 with its readers
 *     sub_419bf0, sub_419bc0, sub_4198e0 and sub_419ab0).
 *
 * The formats are the ones OpenNFS reads (FRD, FCE, FSH/QFS); the layouts in
 * memory are the game's own, which nothing but the game reads, so each of
 * these does what its generated function does, in the same order, to the same
 * bytes.  Memory, registers and flags come out as the generated code leaves
 * them -- the flags it computes, which are not PF and AF, and only where an
 * instruction's flags may be read (an xor before a ret leaves the cmp's
 * before it) -- and only the stack
 * below the returned esp is not written.  The calls the generated code makes
 * to the rest of the game (the file opened, the game's heap) are made the same
 * way, through dynamic_call.
 *
 * NFS_NATIVE_LOAD=0 (or NFS_NATIVES=0) leaves all of them to the generated
 * code; NFS_NATIVE_CHECK=load (or =1) runs each call both ways and logs any
 * difference ([NATIVE]). */
namespace nfs3hp
{

namespace
{

bool loadNativesOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVE_LOAD");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on && nativesEnabled();
}

bool loadChecking()
{
    static const bool check = []() {
        const char* value = SDL_getenv("NFS_NATIVE_CHECK");
        return value && (SDL_strcmp(value, "load") == 0 || SDL_strcmp(value, "1") == 0);
    }();
    return check;
}

/* Set while a check runs the generated code, which must not come back here. */
thread_local bool t_loadOriginal = false;

/* Whether the generated code is to run: the stand-ins switched off, or a check
 * (here or in native_vertices.cpp) running the generated code. */
bool stepAside()
{
    if (!loadNativesOn())
        return true;
    if (loadChecking() && t_loadOriginal)
        return true;
    return nativeOriginalRunning();
}

inline x86::reg8* guest(win32::WinApplication* app)
{
    return &app->getMemory<x86::reg8>(0);
}

inline x86::reg16 read16(const x86::reg8* m, x86::reg32 at)
{
    x86::reg16 value;
    std::memcpy(&value, m + at, sizeof value);
    return value;
}

inline x86::reg32 read32(const x86::reg8* m, x86::reg32 at)
{
    x86::reg32 value;
    std::memcpy(&value, m + at, sizeof value);
    return value;
}

inline void write16(x86::reg8* m, x86::reg32 at, x86::reg16 value)
{
    std::memcpy(m + at, &value, sizeof value);
}

inline void write32(x86::reg8* m, x86::reg32 at, x86::reg32 value)
{
    std::memcpy(m + at, &value, sizeof value);
}

/* ret: the return address popped. */
inline void ret(x86::CPU& cpu, x86::reg32 pop = 0)
{
    cpu.esp += 4 + pop;
}

/* The flags as the generated code sets them (cpu.h's set_szp and clear_co):
 * test, or, and and xor clear CF and OF; cmp works out all four. */
inline void logicFlags(x86::CPU& cpu, x86::reg32 value)
{
    cpu.flags.cf = 0;
    cpu.flags.of = 0;
    cpu.flags.zf = !value;
    cpu.flags.sf = value >> 31;
}

inline void compareFlags(x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
{
    const x86::reg32 result = a - b;
    cpu.flags.cf = a < b;
    cpu.flags.of = ((a >> 31) != (result >> 31)) && ((a >> 31) != (b >> 31));
    cpu.flags.zf = !result;
    cpu.flags.sf = result >> 31;
}

inline void compareFlags8(x86::CPU& cpu, x86::reg8 a, x86::reg8 b)
{
    const x86::reg8 result = x86::reg8(a - b);
    cpu.flags.cf = a < b;
    cpu.flags.of = ((a >> 7) != (result >> 7)) && ((a >> 7) != (b >> 7));
    cpu.flags.zf = !result;
    cpu.flags.sf = result >> 7;
}

/* The registers a function pushes as it starts and pops as it returns, esp
 * with them. */
struct Saved
{
    x86::reg32 ebx, ecx, edx, esi, edi, ebp, esp;
    explicit Saved(const x86::CPU& cpu)
        : ebx(cpu.ebx), ecx(cpu.ecx), edx(cpu.edx), esi(cpu.esi), edi(cpu.edi), ebp(cpu.ebp), esp(cpu.esp)
    {
    }
    void restore(x86::CPU& cpu) const
    {
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
    }
};

/* A call as the generated code makes one: a slot for the return address, then
 * the callee.  False when the game is shutting down, as the generated code
 * then returns at once. */
bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    return !cpu.terminate;
}

/* rep movsb forward: the same as memmove unless the destination starts inside
 * the source, where each byte read may be one just written (a run repeated). */
inline void moveBytes(x86::reg8* m, x86::reg32 to, x86::reg32 from, x86::reg32 count)
{
    // most of RefPack's copies are a few bytes, for which a call costs more
    if (count <= 16)
    {
        for (x86::reg32 i = 0; i < count; ++i)
            m[x86::reg32(to + i)] = m[x86::reg32(from + i)];
        return;
    }
    if (x86::reg32(to - from) >= count)
    {
        std::memmove(m + to, m + from, count);
        return;
    }
    for (x86::reg32 i = 0; i < count; ++i)
        m[x86::reg32(to + i)] = m[x86::reg32(from + i)];
}

/* rep movsd forward, `count` doublewords: the same, a doubleword at a time. */
inline void moveDwords(x86::reg8* m, x86::reg32 to, x86::reg32 from, x86::reg32 count)
{
    const x86::reg32 bytes = count * 4;
    if (x86::reg32(to - from) >= bytes)
    {
        std::memmove(m + to, m + from, bytes);
        return;
    }
    for (x86::reg32 i = 0; i < count; ++i)
        write32(m, to + i * 4, read32(m, from + i * 4));
}

/* ------------------------------------------------------------------------
 * RefPack
 * ------------------------------------------------------------------------ */

/* sub_5102a4: EA's RefPack (QFS) decoder, as OpenNFS has it.
 *   eax  the packed data, 0x10fb (0x11fb with the packed size after it);
 *   edx  where to;  ebx  0 for the size alone.
 * Returns the unpacked size from the header in eax.  ebx is left on the stop
 * code (or after the header), edx holds the doubleword read there; ecx, esi,
 * edi and ebp are the caller's again. */
void refpackNative(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg8* m = guest(app);
    const x86::reg32 source = cpu.eax;
    if (source == 0)
    {
        // or ebx, ebx; je
        cpu.eax = 0;
        cpu.ebx = 0;
        logicFlags(cpu, 0);
        ret(cpu);
        return;
    }
    x86::reg32 at = source + 2;
    if (m[source] & 1)
        at += 3;
    const x86::reg32 size = x86::reg32(m[at]) << 16 | x86::reg32(m[at + 1]) << 8 | m[at + 2];
    at += 3;
    if (cpu.ebx == 0)
    {
        // cmp ecx, 0; je
        cpu.eax = size;
        cpu.ebx = at;
        compareFlags(cpu, 0, 0);
        ret(cpu);
        return;
    }

    x86::reg32 out = cpu.edx;
    for (;;)
    {
        const x86::reg32 b0 = m[at];
        if (!(b0 & 0x80))
        {
            // two bytes: 0-3 literals, then 3-10 bytes from up to 1024 back
            const x86::reg32 b1 = m[at + 1];
            const x86::reg32 literals = b0 & 3;
            moveBytes(m, out, at + 2, literals);
            at += 2 + literals;
            out += literals;
            const x86::reg32 back = (((b0 & 0x60) << 3) | b1) + 1;
            const x86::reg32 length = ((b0 & 0x1c) >> 2) + 3;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (!(b0 & 0x40))
        {
            // three bytes: 0-3 literals, then 4-67 bytes from up to 16384 back
            const x86::reg32 b1 = m[at + 1];
            const x86::reg32 b2 = m[at + 2];
            const x86::reg32 literals = b1 >> 6;
            moveBytes(m, out, at + 3, literals);
            at += 3 + literals;
            out += literals;
            const x86::reg32 back = (((b1 & 0x3f) << 8) | b2) + 1;
            const x86::reg32 length = (b0 & 0x3f) + 4;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (!(b0 & 0x20))
        {
            // four bytes: 0-3 literals, then 5-1028 bytes from up to 131072
            // back -- by doublewords when that far back, the same bytes
            const x86::reg32 b1 = m[at + 1];
            const x86::reg32 b2 = m[at + 2];
            const x86::reg32 b3 = m[at + 3];
            const x86::reg32 literals = b0 & 3;
            moveBytes(m, out, at + 4, literals);
            at += 4 + literals;
            out += literals;
            const x86::reg32 back = (((b0 & 0x10) << 12) | (b1 << 8) | b2) + 1;
            const x86::reg32 length = (((b0 & 0x0c) << 6) | b3) + 5;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (b0 < 0xfc)
        {
            // 4-128 literals, by doublewords
            const x86::reg32 dwords = (b0 & 0x1f) + 1;
            moveDwords(m, out, at + 1, dwords);
            at += 1 + dwords * 4;
            out += dwords * 4;
        }
        else
        {
            // the stop code, with its 0-3 literals: cmp dl, 0xfc; jae
            moveBytes(m, out, at + 1, b0 & 3);
            compareFlags8(cpu, x86::reg8(b0), 0xfc);
            break;
        }
    }
    cpu.eax = size;
    cpu.ebx = at;
    cpu.edx = read32(m, at);
    ret(cpu);
}

/* ------------------------------------------------------------------------
 * FSH alpha scans
 * ------------------------------------------------------------------------ */

/* The scans of sub_4d37a0, one for each of the FSH's pixel formats, over a
 * texture's pixels at eax: 0 when every pixel is opaque, 1 when some are fully
 * transparent (a keyed texture), 2 when one lies between.  Each leaves its two
 * counters in ebx and edx as the generated code does. */

/* sub_4d18e0, 0x6d (4444): rows in edx, columns in ebx; the top nibble. */
void alpha4444Native(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.edx);
    const x86::sreg32 columns = x86::sreg32(cpu.ebx);
    x86::reg32 row = 0;
    x86::reg32 column = cpu.ebx;
    bool keyed = false;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (column = 0; x86::sreg32(column) < columns; ++column, at += 2)
        {
            const x86::reg32 alpha = read16(m, at) & 0xf000;
            if (alpha == 0xf000)
                continue;
            if (alpha != 0)
            {
                // test edi, edi; je
                cpu.eax = 2;
                cpu.ebx = column;
                cpu.edx = row;
                logicFlags(cpu, alpha);
                ret(cpu);
                return;
            }
            keyed = true;
        }
    }
    cpu.eax = keyed ? 1 : 0;
    cpu.ebx = column;
    cpu.edx = row;
    logicFlags(cpu, keyed);  // test ecx, ecx
    ret(cpu);
}

/* sub_4d1950, 0x7d (8888): rows in edx, columns in ebx; the top byte. */
void alpha8888Native(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.edx);
    const x86::sreg32 columns = x86::sreg32(cpu.ebx);
    x86::reg32 row = 0;
    x86::reg32 column = cpu.ebx;
    x86::reg32 keyed = 0;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (column = 0; x86::sreg32(column) < columns; ++column, at += 4)
        {
            const x86::reg32 alpha = read32(m, at) & 0xff000000;
            if (alpha == 0xff000000)
                continue;
            if (alpha != 0)
            {
                cpu.eax = 2;
                cpu.ebx = column;
                cpu.edx = row;
                logicFlags(cpu, alpha);  // test edi, edi
                ret(cpu);
                return;
            }
            keyed = 1;
        }
    }
    cpu.eax = keyed;
    cpu.ebx = column;
    cpu.edx = row;
    logicFlags(cpu, keyed);  // test eax, eax
    ret(cpu);
}

/* sub_4d19b0, 0x7a: rows in ebx, the width in edx; a byte at a time, a
 * nibble of either half 0 is a keyed texture.  The original's row loop never
 * counts its columns: it stops at the first such byte however far that is,
 * and only a width of 0 lets a row end. */
void alphaNibblesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.ebx);
    const x86::reg32 width = cpu.edx;
    x86::reg32 row = 0;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (x86::reg32 column = 0;; column += 2, ++at)
        {
            const x86::reg32 end = x86::sreg32(column) < x86::sreg32(width & 1) ? width + 1 : width;
            if (end == 0)
                break;
            if (!(m[at] & 0x0f) || !(m[at] & 0xf0))
            {
                cpu.eax = 1;
                cpu.ebx = row;
                logicFlags(cpu, 0);  // test byte ptr [eax], 0xf (or 0xf0): zero
                ret(cpu);
                return;
            }
        }
    }
    cpu.eax = 0;
    cpu.ebx = row;
    compareFlags(cpu, row, x86::reg32(rows));  // cmp ebx, esi; jge (the xor after it keeps them)
    ret(cpu);
}

/* sub_4d1a00, 0x7b (8 bits through the palette at [0x563a88]): rows in edx,
 * columns in ebx; index 0xff, or a colour of alpha 0, is a keyed pixel. */
void alphaPalettedNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.edx);
    const x86::sreg32 columns = x86::sreg32(cpu.ebx);
    cpu.edx = 0;
    if (read32(m, 0x563a88) == 0)
    {
        cpu.eax = 2;
        logicFlags(cpu, 0);  // test ecx, ecx
        ret(cpu);
        return;
    }
    x86::reg32 row = 0;
    x86::reg32 keyed = 0;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (x86::reg32 column = 0; x86::sreg32(column) < columns; ++column, ++at)
        {
            const x86::reg8 index = m[at];
            if (index == 0xff)
            {
                keyed = 1;
                continue;
            }
            const x86::reg8 alpha = m[read32(m, 0x563a88) + x86::reg32(index) * 4 + 3];
            if (alpha == 0)
            {
                keyed = 1;
            }
            else if (alpha < 0xff)
            {
                cpu.eax = 2;
                cpu.ebx = row;
                cpu.edx = keyed;
                compareFlags8(cpu, alpha, 0xff);
                ret(cpu);
                return;
            }
        }
    }
    cpu.eax = keyed;
    cpu.ebx = row;
    cpu.edx = keyed;
    logicFlags(cpu, keyed);  // test edx, edx (xor eax, eax the same)
    ret(cpu);
}

/* sub_4d1a90, 0x7e (1555): rows in edx, columns in ebx; a pixel without its
 * top bit is a keyed texture. */
void alpha1555Native(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.edx);
    const x86::sreg32 columns = x86::sreg32(cpu.ebx);
    x86::reg32 row = 0;
    x86::reg32 column = cpu.ebx;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (column = 0; x86::sreg32(column) < columns; ++column, at += 2)
        {
            if (!(m[at + 1] & 0x80))
            {
                cpu.eax = 1;
                cpu.ebx = column;
                cpu.edx = row;
                logicFlags(cpu, 0);  // test byte ptr [eax + 1], 0x80
                ret(cpu);
                return;
            }
        }
    }
    cpu.eax = 0;
    cpu.ebx = column;
    cpu.edx = row;
    compareFlags(cpu, row, x86::reg32(rows));  // cmp edx, ecx; jge (the xor after it keeps them)
    ret(cpu);
}

/* sub_4d1ad0, 0x78 (565): rows in edx, columns in ebx; the key colours 0x7c0
 * and 0x7e0 make a keyed texture. */
void alpha565Native(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    x86::reg32 at = cpu.eax;
    const x86::sreg32 rows = x86::sreg32(cpu.edx);
    const x86::sreg32 columns = x86::sreg32(cpu.ebx);
    x86::reg32 row = 0;
    x86::reg32 column = cpu.ebx;
    for (; x86::sreg32(row) < rows; ++row)
    {
        for (column = 0; x86::sreg32(column) < columns; ++column, at += 2)
        {
            const x86::reg16 pixel = read16(m, at);
            if (pixel == 0x7c0 || pixel == 0x7e0)
            {
                cpu.eax = 1;
                cpu.ebx = column;
                cpu.edx = row;
                logicFlags(cpu, 0);  // cmp di, 0x7c0 / 0x7e0: equal
                ret(cpu);
                return;
            }
        }
    }
    cpu.eax = 0;
    cpu.ebx = column;
    cpu.edx = row;
    compareFlags(cpu, row, x86::reg32(rows));  // cmp edx, ecx; jge (the xor after it keeps them)
    ret(cpu);
}

/* ------------------------------------------------------------------------
 * FCE
 * ------------------------------------------------------------------------ */

/* sub_49cfa0: an NFS3 FCE in memory at eax turned into the game's polygon
 * records.  The header is OpenNFS's FCE::NFS3::HEADER, 0x1f04 bytes:
 *   +0x04 triangles, +0x08 vertices, +0x18 the triangle table, +0x1c the
 *   first reserve table (0x20 bytes a vertex, the game's vertex records),
 *   +0xf8 parts, +0x3fc/+0x4fc each part's first vertex and vertex count,
 *   +0x5fc/+0x6fc its first triangle and triangle count.
 * A triangle is 0x38 bytes: texture page, three vertex indices, the flags at
 * +0x1c, U at +0x20 and V at +0x2c.  A record is 0x84 bytes; the records
 * and a word a vertex come from the game's heap (sub_4e1620), and their
 * address goes over the header's first doubleword.  Returns the FCE in eax. */
void fceRecordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg8* m = guest(app);
    const Saved saved(cpu);
    const x86::reg32 entryEsp = cpu.esp;
    const x86::reg32 fce = cpu.eax;
    const x86::reg32 data = fce + 0x1f04;
    const x86::reg32 frame = entryEsp - 0x18;  // ebp after six pushes
    const x86::reg32 size = read32(m, fce + 4) * 0x84 + read32(m, fce + 8) * 4;

    write32(m, frame - 0x10, data);
    cpu.ebp = frame;
    cpu.esp = frame - 0x14;
    cpu.eax = 0x53c60c;
    cpu.ebx = 0;
    cpu.ecx = fce;
    cpu.edx = size;
    cpu.esi = data;
    if (!call(app, cpu, 0x4e1620))
        return;
    const x86::reg32 records = cpu.eax;
    m = guest(app);
    write32(m, fce, records);

    x86::reg32 part = 0;
    for (; x86::sreg32(part) < x86::sreg32(read32(m, fce + 0xf8)); ++part)
    {
        const x86::reg32 vertices = data + read32(m, fce + 0x1c) + read32(m, fce + 0x3fc + part * 4) * 0x20;
        const x86::reg32 first = read32(m, fce + 0x5fc + part * 4);
        x86::reg32 triangle = data + read32(m, fce + 0x18) + first * 0x38;
        x86::reg32 record = records + first * 0x84;
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(read32(m, fce + 0x4fc + part * 4)); ++i)
            write32(m, vertices + i * 0x20 + 0x14, 0);
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(read32(m, fce + 0x6fc + part * 4)); ++i)
        {
            write32(m, record + 0x08, vertices + (read32(m, triangle + 0x04) << 5));
            write32(m, record + 0x0c, vertices + (read32(m, triangle + 0x08) << 5));
            write16(m, record + 0x04, 3);
            write32(m, record + 0x10, vertices + (read32(m, triangle + 0x0c) << 5));
            // U and V of each corner side by side
            write32(m, record + 0x20, read32(m, triangle + 0x20));
            write32(m, record + 0x24, read32(m, triangle + 0x2c));
            write32(m, record + 0x28, read32(m, triangle + 0x24));
            write32(m, record + 0x2c, read32(m, triangle + 0x30));
            write32(m, record + 0x30, read32(m, triangle + 0x28));
            write32(m, record + 0x34, read32(m, triangle + 0x34));
            write16(m, record + 0x06, read16(m, triangle));
            const x86::reg8 flags = m[triangle + 0x1c];
            if (flags & 4)
                m[record + 7] |= 1;
            if (flags & 8)
                m[record + 7] |= 2;
            if (flags & 1)
                m[record + 7] |= 4;
            if (flags & 2)
                m[record + 7] |= 8;
            triangle += 0x38;
            record += 0x84;
        }
    }
    compareFlags(cpu, part, read32(m, fce + 0xf8));  // cmp ebx, [ecx + 0xf8]; jge
    saved.restore(cpu);
    cpu.eax = fce;
    ret(cpu);
}

/* ------------------------------------------------------------------------
 * FRD
 * ------------------------------------------------------------------------ */

/* The reading of a track's FRD (sub_419c20): the file whole in memory at
 * [0x5dd078] read from front to back into the 0x5c0-byte blocks at 0x571370
 * (OpenNFS's TRKBLOCK with its polygon blocks and objects), and what each
 * points at taken from an arena of 0x310000 bytes at [0x552dfc], [0x552e00]
 * of it used. */
struct FrdReader
{
    win32::WinApplication* app;
    x86::reg8* m;
    // Whether a read went sub_4ea4f0's way through sub_5033e5, whose emms
    // clears the MMX registers.
    bool mmx = false;

    x86::reg32 word(x86::reg32 at) const { return read32(m, at); }

    /* sub_419bf0: `count` bytes from the file to `to`, through sub_4ea4f0 (a
     * memmove of src eax, dst edx, ebx bytes). */
    void read(x86::reg32 to, x86::reg32 count)
    {
        const x86::reg32 from = word(0x5dd078);
        std::memmove(m + to, m + from, count);
        if (!(to > from && to < from + count))
            mmx = true;
        write32(m, 0x5dd078, from + count);
    }

    /* sub_419bc0: `size` bytes of the arena, 0 when it is full. */
    x86::reg32 allocate(x86::reg32 size)
    {
        const x86::reg32 used = word(0x552e00);
        const x86::reg32 end = size + used;
        if (x86::sreg32(end) > 0x310000)
            return 0;
        write32(m, 0x552e00, end);
        return word(0x552dfc) + used;
    }

    /* An array of `count` items of `size` bytes: allocated, then read. */
    x86::reg32 array(x86::reg32 count, x86::reg32 size)
    {
        const x86::reg32 at = allocate(count * size);
        read(at, count * size);
        return at;
    }

    /* sub_4198e0: a list of objects at `list` -- the count, then 0x34 bytes
     * each with the arrays they point at: their extra data, vertices (12
     * bytes), colours (4) and polygons (14). */
    void objects(x86::reg32 list)
    {
        read(list, 4);
        write32(m, list + 4, allocate(word(list) * 0x34));
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(word(list)); ++i)
        {
            const x86::reg32 object = word(list + 4) + i * 0x34;
            read(object, 4);
            read(object + 4, 4);
            read(object + 8, 4);
            read(object + 0xc, 0xc);
            read(object + 0x18, 4);
            const x86::reg32 extra = word(object + 0x18);
            if (extra != 0)
            {
                write32(m, object + 0x1c, allocate(extra));
                read(word(object + 0x1c), word(object + 0x18));
            }
            else
            {
                write32(m, object + 0x1c, 0);
            }
            read(object + 0x20, 4);
            write32(m, object + 0x24, allocate(word(object + 0x20) * 12));
            read(word(object + 0x24), word(object + 0x20) * 12);
            write32(m, object + 0x28, allocate(word(object + 0x20) * 4));
            read(word(object + 0x28), word(object + 0x20) * 4);
            read(object + 0x2c, 4);
            write32(m, object + 0x30, allocate(word(object + 0x2c) * 14));
            read(word(object + 0x30), word(object + 0x2c) * 14);
        }
    }

    /* sub_419ab0: the lights and sound sources, 0x2f bytes each, each given
     * the record of its kind -- from the table at 0x8b41a0 or the one at
     * [0x5dd858].  Returns its eax. */
    x86::reg32 lights()
    {
        read(0x552df8, 4);
        x86::reg32 eax = allocate(word(0x552df8) * 0x2f);
        write32(m, 0x552df4, eax);
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(word(0x552df8)); ++i)
        {
            const x86::reg32 light = word(0x552df4) + i * 0x2f;
            read(light, 0x2f);
            const x86::reg32 kind = x86::reg32(x86::sreg32(word(light + 0x2b)) >> 16);
            if (m[light + 0x2c] != 0)
                eax = word(word(kind * 4 + 0x8b41a0) + 4);
            else
                eax = word(kind * 0x2c + word(0x5dd858) + 4);
            write32(m, light + 4, eax);
        }
        return eax;
    }
};

void frdNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Saved saved(cpu);
    const x86::reg32 frame = cpu.esp - 0x18;  // ebp after six pushes
    cpu.ebp = frame;
    cpu.esp = frame - 0x24;

    // The file's name (sub_41d8a0, "%s...%s.frd"), and the file (sub_4e0ec0).
    cpu.eax = 0x536c64;
    cpu.edx = 0x10;
    if (!call(app, cpu, 0x41d8a0))
        return;
    if (!call(app, cpu, 0x4e0ec0))
        return;
    FrdReader r{app, guest(app)};
    write32(r.m, 0x5dd078, cpu.eax);
    write32(r.m, 0x5dd07c, cpu.eax);

    // The header: 28 bytes the game has no use for, then the last block's number.
    r.read(frame - 0x18, 4);
    r.read(frame - 0x24, 8);
    r.read(frame - 0x24, 8);
    r.read(frame - 0x24, 8);
    r.read(0x5dd83c, 4);

    // Each block: its points, extents and vertices, its neighbours, and the
    // lists that go with it (positions, polygon groups, extra objects, ...).
    x86::reg32 block = 0;
    for (; x86::sreg32(block) <= x86::sreg32(r.word(0x5dd83c)); ++block)
    {
        const x86::reg32 b = 0x571370 + block * 0x5c0;
        r.read(b + 0x78, 0xc);
        r.read(b + 0x84, 0x30);
        r.read(b + 0x58, 4);
        r.read(b + 0x5c, 0x14);
        write32(r.m, b + 0x70, r.array(r.word(b + 0x58), 12));
        write32(r.m, b + 0x74, r.array(r.word(b + 0x58), 4));
        r.read(b + 0xb4, 0x4b0);
        for (x86::reg32 at : {0x584u, 0x588u, 0x590u, 0x598u, 0x5a0u, 0x5a8u, 0x5b0u, 0x5b8u})
            r.read(b + at, 4);
        // all seven allocated first, then all seven read
        static const x86::reg32 lists[7][2] = {
            {0x588, 8}, {0x590, 8}, {0x598, 12}, {0x5a0, 20}, {0x5a8, 20}, {0x5b0, 16}, {0x5b8, 16},
        };
        for (const auto& list : lists)
            write32(r.m, b + list[0] + 4, r.allocate(r.word(b + list[0]) * list[1]));
        for (const auto& list : lists)
            r.read(r.word(b + list[0] + 4), r.word(b + list[0]) * list[1]);
    }
    x86::reg32 edi = block;

    // Each block's polygons at its eleven levels of detail: a count, then
    // that many 14-byte polygons -- in pieces for the last four.
    x86::reg32 piece = 0;
    for (piece = 0; x86::sreg32(piece) <= x86::sreg32(r.word(0x5dd83c)); ++piece)
    {
        const x86::reg32 b = 0x571370 + piece * 0x5c0;
        write32(r.m, frame - 4, b);
        write32(r.m, 0x5dd080, 0);
        for (x86::reg32 level = 0; level < 0xb; ++level)
        {
            edi = level * 4;
            r.read(b + edi, 4);
            const x86::reg32 count = r.word(b + edi);
            if (count == 0)
            {
                write32(r.m, b + 0x2c + edi, 0);
                continue;
            }
            x86::reg32 to = r.allocate(count * 14);
            write32(r.m, b + 0x2c + edi, to);
            to = r.word(b + 0x2c + edi);
            if (level < 6)
            {
                r.read(frame - 0x14, 4);
                r.read(to, r.word(frame - 0x14) * 14);
            }
            else if (level == 6)
            {
                r.read(frame - 0x10, 4);
                r.read(to, r.word(frame - 0x10) * 14);
            }
            else
            {
                r.read(frame - 0x1c, 4);
                for (;;)
                {
                    edi = r.word(frame - 0x1c) - 1;
                    write32(r.m, frame - 0x1c, edi);
                    if (edi == 0xffffffff)
                        break;
                    r.read(frame - 0xc, 4);
                    if (r.word(frame - 0xc) != 1)
                        continue;
                    r.read(frame - 8, 4);
                    r.read(to, r.word(frame - 8) * 14);
                    to += r.word(frame - 8) * 14;
                }
            }
        }
    }

    // The objects of each block at four distances, the track's own, and the
    // lights.
    x86::reg32 last = 0;
    for (block = 0; x86::sreg32(block) <= x86::sreg32(r.word(0x5dd83c)); ++block)
    {
        last = 0x571370 + block * 0x5c0;
        r.objects(last + 0x564);
        r.objects(last + 0x56c);
        r.objects(last + 0x574);
        r.objects(last + 0x57c);
    }
    r.objects(0x5dd070);
    cpu.eax = r.lights();
    if (r.mmx && r.word(0x5643b4) != 0)
        cpu.mmx.init();

    // The file freed (sub_4e1890).
    const x86::reg32 file = r.word(0x5dd07c);
    cpu.ebx = file;
    if (file != 0)
    {
        cpu.eax = file;
        cpu.ecx = last;
        cpu.edx = block;
        cpu.esi = piece;
        cpu.edi = edi;
        cpu.esp = frame - 0x24;
        if (!call(app, cpu, 0x4e1890))
            return;
    }
    else
    {
        logicFlags(cpu, 0);  // test ebx, ebx
    }
    saved.restore(cpu);
    ret(cpu);
}

/* ------------------------------------------------------------------------
 * NFS_NATIVE_CHECK=load
 * ------------------------------------------------------------------------ */

struct LoadCheckCounts
{
    unsigned calls = 0;
    unsigned mismatches = 0;
};

void describeLoadState(const x86::CPU& mine, const x86::CPU& cpu, char* what, std::size_t size)
{
    if (mine.eax != cpu.eax || mine.ebx != cpu.ebx || mine.ecx != cpu.ecx || mine.edx != cpu.edx
        || mine.esi != cpu.esi || mine.edi != cpu.edi || mine.ebp != cpu.ebp || mine.esp != cpu.esp)
    {
        SDL_snprintf(what, size,
                     "registers: eax %08x/%08x ebx %08x/%08x ecx %08x/%08x edx %08x/%08x esi %08x/%08x "
                     "edi %08x/%08x esp %08x/%08x",
                     unsigned(mine.eax), unsigned(cpu.eax), unsigned(mine.ebx), unsigned(cpu.ebx),
                     unsigned(mine.ecx), unsigned(cpu.ecx), unsigned(mine.edx), unsigned(cpu.edx),
                     unsigned(mine.esi), unsigned(cpu.esi), unsigned(mine.edi), unsigned(cpu.edi),
                     unsigned(mine.esp), unsigned(cpu.esp));
    }
    else if ((mine.flags.lo & 0xd5) != (cpu.flags.lo & 0xd5) || mine.flags.of != cpu.flags.of)
    {
        SDL_snprintf(what, size, "flags %02x/%02x of %u/%u", unsigned(mine.flags.lo), unsigned(cpu.flags.lo),
                     unsigned(mine.flags.of), unsigned(cpu.flags.of));
    }
    else if (mine.fpu.status.word != cpu.fpu.status.word || mine.fpu.count != cpu.fpu.count)
    {
        SDL_snprintf(what, size, "fpu status %04x/%04x depth %u/%u", unsigned(mine.fpu.status.word),
                     unsigned(cpu.fpu.status.word), unsigned(mine.fpu.count), unsigned(cpu.fpu.count));
    }
    else if (std::memcmp(&mine.mmx, &cpu.mmx, sizeof cpu.mmx) != 0)
    {
        SDL_snprintf(what, size, "mmx registers");
    }
}

void reportLoad(const char* name, LoadCheckCounts& counts, const char* what)
{
    ++counts.calls;
    if (what[0])
    {
        ++counts.mismatches;
        if (counts.mismatches <= 20)
            SDL_Log("[NATIVE] %s differs (call %u): %s", name, counts.calls, what);
    }
    if (counts.calls <= 3 || counts.calls % 1000 == 0)
        SDL_Log("[NATIVE] %s checked: %u calls, %u differ", name, counts.calls, counts.mismatches);
}

/* The native into `ranges` of memory, everything put back, the generated code
 * for real; then the ranges and the state the caller sees compared.  With no
 * ranges, all of guest memory but the stack below the returned esp. */
void checkLoad(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address,
               std::vector<std::pair<x86::reg32, x86::reg32>> ranges,
               void (*native)(win32::WinApplication*, x86::CPU&), LoadCheckCounts& counts)
{
    const bool whole = ranges.empty();
    if (whole)
        win32::MemMap::usedRanges(ranges);
    std::size_t total = 0;
    for (const auto& range : ranges)
        total += range.second;
    std::vector<x86::reg8> before(total);
    std::vector<x86::reg8> written(total);
    auto copyOut = [&](std::vector<x86::reg8>& to) {
        std::size_t at = 0;
        for (const auto& range : ranges)
        {
            std::memcpy(&to[at], &app->getMemory<x86::reg8>(range.first), range.second);
            at += range.second;
        }
    };
    copyOut(before);
    const x86::CPU entry = cpu;
    native(app, cpu);
    const x86::CPU mine = cpu;
    copyOut(written);
    std::size_t at = 0;
    for (const auto& range : ranges)
    {
        std::memcpy(&app->getMemory<x86::reg8>(range.first), &before[at], range.second);
        at += range.second;
    }
    cpu = entry;
    t_loadOriginal = true;
    app->dynamic_call(address, cpu);
    t_loadOriginal = false;

    const x86::reg32 stackEnd = cpu.esp;
    const x86::reg32 stackStart = cpu.esp - 0x10000;
    char what[200] = "";
    unsigned differing = 0;
    at = 0;
    for (const auto& range : ranges)
    {
        const x86::reg8* original = &app->getMemory<x86::reg8>(range.first);
        for (x86::reg32 i = 0; i < range.second; ++i)
        {
            const x86::reg32 address = range.first + i;
            if (original[i] == written[at + i] || (whole && address >= stackStart && address < stackEnd))
                continue;
            if (differing++ == 0)
                SDL_snprintf(what, sizeof what, "memory at %08x: %02x native, %02x original", unsigned(address),
                             unsigned(written[at + i]), unsigned(original[i]));
        }
        at += range.second;
    }
    if (differing > 1)
    {
        const std::size_t length = SDL_strlen(what);
        SDL_snprintf(what + length, sizeof what - length, " (%u bytes differ)", differing);
    }
    if (!what[0])
        describeLoadState(mine, cpu, what, sizeof what);
    reportLoad(name, counts, what);
}

/* The size sub_5102a4 will unpack, for its check: what the header says. */
x86::reg32 refpackSize(win32::WinApplication* app, const x86::CPU& cpu)
{
    const x86::reg8* m = guest(app);
    if (cpu.eax == 0 || cpu.ebx == 0)
        return 0;
    const x86::reg32 at = cpu.eax + 2 + ((m[cpu.eax] & 1) ? 3 : 0);
    return x86::reg32(m[at]) << 16 | x86::reg32(m[at + 1]) << 8 | m[at + 2];
}

template <void (*Native)(win32::WinApplication*, x86::CPU&)>
bool scan(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address, LoadCheckCounts& counts)
{
    if (stepAside())
        return false;
    if (loadChecking())
    {
        // The scans write nothing but their stack.
        checkLoad(app, cpu, name, address, {{0x563a88, 4}}, Native, counts);
        return true;
    }
    Native(app, cpu);
    return true;
}

}

bool refpack(win32::WinApplication* app, x86::CPU& cpu)
{
    if (stepAside() || cpu.flags.df)
        return false;
    if (loadChecking())
    {
        static LoadCheckCounts counts;
        checkLoad(app, cpu, "sub_5102a4", 0x5102a4, {{cpu.edx, refpackSize(app, cpu)}}, refpackNative, counts);
        return true;
    }
    refpackNative(app, cpu);
    return true;
}

bool alpha4444(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alpha4444Native>(app, cpu, "sub_4d18e0", 0x4d18e0, counts);
}

bool alpha8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alpha8888Native>(app, cpu, "sub_4d1950", 0x4d1950, counts);
}

bool alphaNibbles(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alphaNibblesNative>(app, cpu, "sub_4d19b0", 0x4d19b0, counts);
}

bool alphaPaletted(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alphaPalettedNative>(app, cpu, "sub_4d1a00", 0x4d1a00, counts);
}

bool alpha1555(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alpha1555Native>(app, cpu, "sub_4d1a90", 0x4d1a90, counts);
}

bool alpha565(win32::WinApplication* app, x86::CPU& cpu)
{
    static LoadCheckCounts counts;
    return scan<alpha565Native>(app, cpu, "sub_4d1ad0", 0x4d1ad0, counts);
}

bool fceRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (stepAside())
        return false;
    if (loadChecking())
    {
        static LoadCheckCounts counts;
        checkLoad(app, cpu, "sub_49cfa0", 0x49cfa0, {}, fceRecordsNative, counts);
        return true;
    }
    fceRecordsNative(app, cpu);
    return true;
}

bool frd(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4ea4f0's copy by the FPU (sub_503281), on a processor without MMX,
    // leaves the FPU's state behind as a memmove would not.
    const x86::reg8* m = guest(app);
    if (stepAside() || (read32(m, 0x5643b4) == 0 && read32(m, 0x5643ac) != 0))
        return false;
    if (loadChecking())
    {
        // The file is opened and read twice.
        static LoadCheckCounts counts;
        checkLoad(app, cpu, "sub_419c20", 0x419c20, {}, frdNative, counts);
        return true;
    }
    frdNative(app, cpu);
    return true;
}

}
