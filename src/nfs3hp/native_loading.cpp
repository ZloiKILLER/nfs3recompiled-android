#include <nfs3hp.h>
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_guest.h"
#include "native_load_track.h"
#include "native_loading.h"
#include "native_thrash.h"
#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

/* Native functions for the loading of the game's data, as the vertex loops
 * are (native_vertices.cpp):
 *
 *   - EA's RefPack decoder (sub_5102a4), behind every .qfs and every packed
 *     file the game reads: the track's textures, the sky, the HUD, the menus.
 *     The game's code moves each byte of its output as a loop of its own
 *     (rep movsb, one step per byte);
 *   - an FCE model turned into the game's polygon records (sub_49cfa0);
 *   - a track's FRD file read into its blocks (sub_419c20 with its readers
 *     sub_419bf0, sub_419bc0, sub_4198e0 and sub_419ab0).
 *
 * The formats are the ones OpenNFS reads (FRD, FCE, FSH/QFS); the layouts in
 * memory are the game's own, which nothing but the game reads, so each of
 * these does what the game's function does, in the same order, to the same
 * bytes.  Memory, registers and flags come out as the game's code leaves
 * them -- the flags it computes, which are not PF and AF, and only where an
 * instruction's flags may be read (an xor before a ret leaves the cmp's
 * before it) -- and only the stack
 * below the returned esp is not written.  The calls the game's code makes
 * to the rest of the game (the file opened, the game's heap) are made the same
 * way, through a pointer (app->callAddress).
 */
namespace nfs3hp
{

namespace
{

inline nfs::u8* memoryOf(nfs::App* app)
{
    return &app->getMemory<nfs::u8>(0);
}

inline nfs::u16 read16(const nfs::u8* m, nfs::u32 at)
{
    nfs::u16 value;
    std::memcpy(&value, m + at, sizeof value);
    return value;
}

inline nfs::u32 read32(const nfs::u8* m, nfs::u32 at)
{
    nfs::u32 value;
    std::memcpy(&value, m + at, sizeof value);
    return value;
}

inline void write16(nfs::u8* m, nfs::u32 at, nfs::u16 value)
{
    std::memcpy(m + at, &value, sizeof value);
}

inline void write32(nfs::u8* m, nfs::u32 at, nfs::u32 value)
{
    std::memcpy(m + at, &value, sizeof value);
}

/* ret: the return address popped. */
inline void ret(nfs::ThreadState& ts, nfs::u32 pop = 0)
{
    ts.sp += 4 + pop;
}

/* The registers a function pushes as it starts and pops as it returns, esp
 * with them. */
struct Saved
{
    nfs::u32 ebx, ecx, edx, esi, edi, ebp, esp;
};

/* rep movsb forward: the same as memmove unless the destination starts inside
 * the source, where each byte read may be one just written (a run repeated). */
inline void moveBytes(nfs::u8* m, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    // most of RefPack's copies are a few bytes, for which a call costs more
    if (count <= 16)
    {
        for (nfs::u32 i = 0; i < count; ++i)
            m[nfs::u32(to + i)] = m[nfs::u32(from + i)];
        return;
    }
    if (nfs::u32(to - from) >= count)
    {
        std::memmove(m + to, m + from, count);
        return;
    }
    for (nfs::u32 i = 0; i < count; ++i)
        m[nfs::u32(to + i)] = m[nfs::u32(from + i)];
}

/* rep movsd forward, `count` doublewords: the same, a doubleword at a time. */
inline void moveDwords(nfs::u8* m, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    const nfs::u32 bytes = count * 4;
    if (nfs::u32(to - from) >= bytes)
    {
        std::memmove(m + to, m + from, bytes);
        return;
    }
    for (nfs::u32 i = 0; i < count; ++i)
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
nfs::u32 refpackNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    nfs::u8* m = memoryOf(app);
    const nfs::u32 source = args.eax;
    if (source == 0)
    {
        // or ebx, ebx; je
        args.eax = 0;
        args.ebx = 0;
        ret(ts);
        return args.eax;
    }
    nfs::u32 at = source + 2;
    if (m[source] & 1)
        at += 3;
    const nfs::u32 size = nfs::u32(m[at]) << 16 | nfs::u32(m[at + 1]) << 8 | m[at + 2];
    at += 3;
    if (args.ebx == 0)
    {
        // cmp ecx, 0; je
        args.eax = size;
        args.ebx = at;
        ret(ts);
        return args.eax;
    }

    nfs::u32 out = args.edx;
    for (;;)
    {
        const nfs::u32 b0 = m[at];
        if (!(b0 & 0x80))
        {
            // two bytes: 0-3 literals, then 3-10 bytes from up to 1024 back
            const nfs::u32 b1 = m[at + 1];
            const nfs::u32 literals = b0 & 3;
            moveBytes(m, out, at + 2, literals);
            at += 2 + literals;
            out += literals;
            const nfs::u32 back = (((b0 & 0x60) << 3) | b1) + 1;
            const nfs::u32 length = ((b0 & 0x1c) >> 2) + 3;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (!(b0 & 0x40))
        {
            // three bytes: 0-3 literals, then 4-67 bytes from up to 16384 back
            const nfs::u32 b1 = m[at + 1];
            const nfs::u32 b2 = m[at + 2];
            const nfs::u32 literals = b1 >> 6;
            moveBytes(m, out, at + 3, literals);
            at += 3 + literals;
            out += literals;
            const nfs::u32 back = (((b1 & 0x3f) << 8) | b2) + 1;
            const nfs::u32 length = (b0 & 0x3f) + 4;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (!(b0 & 0x20))
        {
            // four bytes: 0-3 literals, then 5-1028 bytes from up to 131072
            // back -- by doublewords when that far back, the same bytes
            const nfs::u32 b1 = m[at + 1];
            const nfs::u32 b2 = m[at + 2];
            const nfs::u32 b3 = m[at + 3];
            const nfs::u32 literals = b0 & 3;
            moveBytes(m, out, at + 4, literals);
            at += 4 + literals;
            out += literals;
            const nfs::u32 back = (((b0 & 0x10) << 12) | (b1 << 8) | b2) + 1;
            const nfs::u32 length = (((b0 & 0x0c) << 6) | b3) + 5;
            moveBytes(m, out, out - back, length);
            out += length;
        }
        else if (b0 < 0xfc)
        {
            // 4-128 literals, by doublewords
            const nfs::u32 dwords = (b0 & 0x1f) + 1;
            moveDwords(m, out, at + 1, dwords);
            at += 1 + dwords * 4;
            out += dwords * 4;
        }
        else
        {
            // the stop code, with its 0-3 literals: cmp dl, 0xfc; jae
            moveBytes(m, out, at + 1, b0 & 3);
            break;
        }
    }
    args.eax = size;
    args.ebx = at;
    args.edx = read32(m, at);
    ret(ts);
    return args.eax;
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
std::optional<nfs::u32> modelReadyOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fce)
{
    nfs::u8* m = memoryOf(app);
    const nfs::u32 savedEsp = ts.sp;
    const nfs::u32 entryEsp = ts.sp;
    const nfs::u32 data = fce + 0x1f04;
    const nfs::u32 frame = entryEsp - 0x18;  // the frame after six pushes
    const nfs::u32 size = read32(m, fce + 4) * 0x84 + read32(m, fce + 8) * 4;

    write32(m, frame - 0x10, data);
    ts.sp = frame - 0x14;
    const std::optional<nfs::u32> reserved = guest::direct<0x4e1620, ea::reserved>(app, ts, 0x53c60cu, size, 0u);
    if (!reserved || ts.terminate)
    {
        ts.sp = savedEsp;
        return std::nullopt;
    }
    const nfs::u32 records = *reserved;
    m = memoryOf(app);
    write32(m, fce, records);

    nfs::u32 part = 0;
    for (; nfs::s32(part) < nfs::s32(read32(m, fce + 0xf8)); ++part)
    {
        const nfs::u32 vertices = data + read32(m, fce + 0x1c) + read32(m, fce + 0x3fc + part * 4) * 0x20;
        const nfs::u32 first = read32(m, fce + 0x5fc + part * 4);
        nfs::u32 triangle = data + read32(m, fce + 0x18) + first * 0x38;
        nfs::u32 record = records + first * 0x84;
        for (nfs::u32 i = 0; nfs::s32(i) < nfs::s32(read32(m, fce + 0x4fc + part * 4)); ++i)
            write32(m, vertices + i * 0x20 + 0x14, 0);
        for (nfs::u32 i = 0; nfs::s32(i) < nfs::s32(read32(m, fce + 0x6fc + part * 4)); ++i)
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
            const nfs::u8 flags = m[triangle + 0x1c];
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
    ts.sp = savedEsp;
    return fce;
}

/* sub_49cfa0 as a call through a pointer reaches it: the FCE at eax. */
nfs::u32 fceRecordsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<nfs::u32> model = modelReadyOf(app, ts, args.eax);
    if (!model)
        return args.eax;
    args.eax = *model;
    ret(ts);
    return args.eax;
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
    nfs::App* app;
    nfs::u8* m;
    nfs::u32 word(nfs::u32 at) const { return read32(m, at); }

    /* sub_419bf0: `count` bytes from the file to `to`, through sub_4ea4f0 (a
     * memmove of src eax, dst edx, ebx bytes). */
    void read(nfs::u32 to, nfs::u32 count)
    {
        const nfs::u32 from = word(0x5dd078);
        std::memmove(m + to, m + from, count);
        write32(m, 0x5dd078, from + count);
    }

    /* sub_419bc0: `size` bytes of the arena, 0 when it is full. */
    nfs::u32 allocate(nfs::u32 size)
    {
        const nfs::u32 used = word(0x552e00);
        const nfs::u32 end = size + used;
        if (nfs::s32(end) > 0x310000)
            return 0;
        write32(m, 0x552e00, end);
        return word(0x552dfc) + used;
    }

    /* An array of `count` items of `size` bytes: allocated, then read. */
    nfs::u32 array(nfs::u32 count, nfs::u32 size)
    {
        const nfs::u32 at = allocate(count * size);
        read(at, count * size);
        return at;
    }

    /* sub_4198e0: a list of objects at `list` -- the count, then 0x34 bytes
     * each with the arrays they point at: their extra data, vertices (12
     * bytes), colours (4) and polygons (14). */
    void objects(nfs::u32 list)
    {
        read(list, 4);
        write32(m, list + 4, allocate(word(list) * 0x34));
        for (nfs::u32 i = 0; nfs::s32(i) < nfs::s32(word(list)); ++i)
        {
            const nfs::u32 object = word(list + 4) + i * 0x34;
            read(object, 4);
            read(object + 4, 4);
            read(object + 8, 4);
            read(object + 0xc, 0xc);
            read(object + 0x18, 4);
            const nfs::u32 extra = word(object + 0x18);
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
    nfs::u32 lights()
    {
        read(0x552df8, 4);
        nfs::u32 eax = allocate(word(0x552df8) * 0x2f);
        write32(m, 0x552df4, eax);
        for (nfs::u32 i = 0; nfs::s32(i) < nfs::s32(word(0x552df8)); ++i)
        {
            const nfs::u32 light = word(0x552df4) + i * 0x2f;
            read(light, 0x2f);
            const nfs::u32 kind = nfs::u32(nfs::s32(word(light + 0x2b)) >> 16);
            if (m[light + 0x2c] != 0)
                eax = word(word(kind * 4 + 0x8b41a0) + 4);
            else
                eax = word(kind * 0x2c + word(0x5dd858) + 4);
            write32(m, light + 4, eax);
        }
        return eax;
    }
};

nfs::u32 frdNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Saved saved{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp, ts.sp};
    const nfs::u32 frame = ts.sp - 0x18;  // ebp after six pushes
    args.ebp = frame;
    ts.sp = frame - 0x24;

    // The file's name (sub_41d8a0, "%s...%s.frd"), and the file (sub_4e0ec0).
    const std::optional<nfs::u32> name = guest::direct<0x41d8a0, load::trackFile>(app, ts, 0x536c64u);
    if (!name || ts.terminate)
        return args.eax;
    const std::optional<nfs::u32> loaded = guest::direct<0x4e0ec0, ea::loadWhole>(app, ts, *name, 0x10u);
    if (!loaded || ts.terminate)
        return args.eax;
    args.eax = *loaded;
    FrdReader r{app, memoryOf(app)};
    write32(r.m, 0x5dd078, args.eax);
    write32(r.m, 0x5dd07c, args.eax);

    // The header: 28 bytes the game has no use for, then the last block's number.
    r.read(frame - 0x18, 4);
    r.read(frame - 0x24, 8);
    r.read(frame - 0x24, 8);
    r.read(frame - 0x24, 8);
    r.read(0x5dd83c, 4);

    // Each block: its points, extents and vertices, its neighbours, and the
    // lists that go with it (positions, polygon groups, extra objects, ...).
    nfs::u32 block = 0;
    for (; nfs::s32(block) <= nfs::s32(r.word(0x5dd83c)); ++block)
    {
        const nfs::u32 b = 0x571370 + block * 0x5c0;
        r.read(b + 0x78, 0xc);
        r.read(b + 0x84, 0x30);
        r.read(b + 0x58, 4);
        r.read(b + 0x5c, 0x14);
        write32(r.m, b + 0x70, r.array(r.word(b + 0x58), 12));
        write32(r.m, b + 0x74, r.array(r.word(b + 0x58), 4));
        r.read(b + 0xb4, 0x4b0);
        for (nfs::u32 at : {0x584u, 0x588u, 0x590u, 0x598u, 0x5a0u, 0x5a8u, 0x5b0u, 0x5b8u})
            r.read(b + at, 4);
        // all seven allocated first, then all seven read
        static const nfs::u32 lists[7][2] = {
            {0x588, 8}, {0x590, 8}, {0x598, 12}, {0x5a0, 20}, {0x5a8, 20}, {0x5b0, 16}, {0x5b8, 16},
        };
        for (const auto& list : lists)
            write32(r.m, b + list[0] + 4, r.allocate(r.word(b + list[0]) * list[1]));
        for (const auto& list : lists)
            r.read(r.word(b + list[0] + 4), r.word(b + list[0]) * list[1]);
    }
    nfs::u32 edi = block;

    // Each block's polygons at its eleven levels of detail: a count, then
    // that many 14-byte polygons -- in pieces for the last four.
    nfs::u32 piece = 0;
    for (piece = 0; nfs::s32(piece) <= nfs::s32(r.word(0x5dd83c)); ++piece)
    {
        const nfs::u32 b = 0x571370 + piece * 0x5c0;
        write32(r.m, frame - 4, b);
        write32(r.m, 0x5dd080, 0);
        for (nfs::u32 level = 0; level < 0xb; ++level)
        {
            edi = level * 4;
            r.read(b + edi, 4);
            const nfs::u32 count = r.word(b + edi);
            if (count == 0)
            {
                write32(r.m, b + 0x2c + edi, 0);
                continue;
            }
            nfs::u32 to = r.allocate(count * 14);
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
    nfs::u32 last = 0;
    for (block = 0; nfs::s32(block) <= nfs::s32(r.word(0x5dd83c)); ++block)
    {
        last = 0x571370 + block * 0x5c0;
        r.objects(last + 0x564);
        r.objects(last + 0x56c);
        r.objects(last + 0x574);
        r.objects(last + 0x57c);
    }
    r.objects(0x5dd070);
    args.eax = r.lights();

    // The file freed (sub_4e1890).
    const nfs::u32 file = r.word(0x5dd07c);
    args.ebx = file;
    if (file != 0)
    {
        args.eax = file;
        args.ecx = last;
        args.edx = block;
        args.esi = piece;
        args.edi = edi;
        ts.sp = frame - 0x24;
        const std::optional<nfs::u32> purged = guest::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
        if (!purged || ts.terminate)
            return args.eax;
        args.eax = *purged;
    }
    else
    {
    }
    args.ebp = saved.ebp;
    ts.sp = saved.esp;
    ret(ts);
    return args.eax;
}

}

namespace loader
{

/* sub_5102a4: EA's RefPack decoder. */
nfs::u32 refpack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    refpackNative(app, ts, args);
    return args.eax;
}

/* sub_49cfa0: an FCE model turned into the game's polygon records. */
nfs::u32 fceRecords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    fceRecordsNative(app, ts, args);
    return args.eax;
}

}

/* Port: each block's list of the blocks seen from it made longer.  The track
 * draws only what the list of the block the view is in names (sub_41cdc0, 300
 * entries at +0xb4: the block's number, then its detail -- 0 to 2, or -1 for
 * sub_41aa60 to pick by the distance), however far the view reaches.  The
 * lists were made for a 4:3 picture, so on a wide screen blocks came into view
 * at its sides.  Every block whose centre (+0x78 x, +0x80 z) is within reach
 * of the block's is added, the nearest first, its detail by the distance; the
 * far distance and the view's edges still cull them as they cull the listed
 * ones.  The reach is View Distance Full's 700 (nfs3hp_main.cpp's
 * kFullFarDistance) and twice the track's longest step from one block's centre
 * to the next: the view can stand anywhere in its block, so a piece the far
 * distance takes in may lie that much further from the block's centre than
 * from the view.  NFS_TRACK_NEIGHBOURS=0 keeps the track's own lists. */
constexpr float kNeighbourFar = 700.0f;

void extendNeighbours(nfs::App* app)
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TRACK_NEIGHBOURS");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    nfs::u8* m = memoryOf(app);
    const nfs::s32 last = nfs::s32(read32(m, 0x5dd83c));
    if (!on || last < 0 || last >= 2000)
        return;
    const nfs::u32 count = nfs::u32(last) + 1;
    auto record = [](nfs::u32 block) { return 0x571370 + block * 0x5c0; };
    std::vector<float> x(count), z(count);
    float step = 0.0f;
    for (nfs::u32 b = 0; b < count; ++b)
    {
        x[b] = app->getMemory<float>(record(b) + 0x78);
        z[b] = app->getMemory<float>(record(b) + 0x80);
        if (b > 0)
            step = std::max(step, std::sqrt((x[b] - x[b - 1]) * (x[b] - x[b - 1]) + (z[b] - z[b - 1]) * (z[b] - z[b - 1])));
    }
    step = std::min(step, 200.0f);  // a jump between pieces that do not meet is no step
    const float reach = kNeighbourFar + 2.0f * step;
    constexpr nfs::u32 kEntries = 300;
    std::vector<bool> listed(count);
    std::vector<std::pair<float, nfs::u32>> near;
    nfs::u32 ownTotal = 0, ownMost = 0, total = 0, full = 0;
    for (nfs::u32 b = 0; b < count; ++b)
    {
        const nfs::u32 list = record(b) + 0xb4;
        std::fill(listed.begin(), listed.end(), false);
        nfs::u32 n = 0;
        for (; n < kEntries; ++n)
        {
            const nfs::s32 other = nfs::s16(app->getMemory<nfs::u16>(list + n * 4));
            if (other == -1)
                break;
            if (other >= 0 && nfs::u32(other) < count)
                listed[other] = true;
        }
        ownTotal += n;
        ownMost = std::max(ownMost, n);
        near.clear();
        for (nfs::u32 o = 0; o < count; ++o)
        {
            const float dx = x[o] - x[b], dz = z[o] - z[b];
            const float distance = dx * dx + dz * dz;
            if (!listed[o] && distance < reach * reach)
                near.push_back({distance, o});
        }
        std::sort(near.begin(), near.end());
        for (const auto& [distance, o] : near)
        {
            if (n >= kEntries)
                break;
            write32(m, list + n * 4, 0xffff0000 | o);
            ++n;
        }
        if (n < kEntries)
            write32(m, list + n * 4, 0xffffffff);
        total += n;
        full += n >= kEntries;
    }
    SDL_Log("[TRACK] %u blocks, longest step %.0f, reach %.0f; lists %.1f on average (most %u) -> %.1f, "
            "%u full at %u",
            unsigned(count), double(step), double(reach), double(ownTotal) / count, unsigned(ownMost),
            double(total) / count, unsigned(full), unsigned(kEntries));
}

namespace loader
{

std::optional<nfs::u32> fceModelReady(nfs::App* app, nfs::ThreadState& ts, nfs::u32 fce)
{
    return modelReadyOf(app, ts, fce);
}

/* sub_419c20: a track's FRD read into its blocks, the lists of what each block sees made longer. */
nfs::u32 frd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    frdNative(app, ts, args);
    if (!ts.terminate)
        extendNeighbours(app);
    return args.eax;
}

}

}
