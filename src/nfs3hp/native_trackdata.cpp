#include "game_settings.h"
#include "native_carstate.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_raceload.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <type_traits>

/* The track's data and a place on it: its nodes, the road's quads around
 * them, and what the cars, the sound and the renderer ask of them.
 * The game's code calls each function here by name.
 *
 * The track's nodes: [0x5dd948] (the track's header [0x5dd938] + 8), [0x5dd940]
 * of them (the header's word at 6), 0x24 bytes each -- x, y, z in 16.16 at 0,
 * 4, 8, signed bytes of its directions from 0xd on (each the top byte of the
 * word read 3 bytes before it), nibbles at 0xe and 0x13, widths at 0x17 and
 * 0x1b.  Every 8 nodes a block of 0x5c0 bytes from 0x571370: +0x3c its strips
 * (14 bytes each, four vertices' words), +0x70 its vertices (three floats),
 * +0x74 their colours, +0x584 its first node, +0x58c its nodes' entries (8
 * bytes: the first strip's word, the strips' count at 2, a kind at 3, the
 * linked nodes' words at 4 and 6), +0x594 the strips' polygons (8 bytes, the
 * normal's index at 0, the kind's nibble at 1, 0xe off the road), +0x59c the
 * normals (12 bytes), +0x5a0 and +0x5a4 its items (20 bytes each).
 *
 * A place on the track (0x78 bytes, 0x1e words): +0 its block, +4 where it
 * is, +0x10 its node, +0x14 its strip, +0x18 its polygon, +0x1c its normal,
 * +0x20 its node's entry, +0x24 the quad's four corners in 16.16 (12 words),
 * +0x54 the four corners' vertices, +0x64 what was found, +0x68 the node
 * changed, +0x6a the strip changed, +0x6c which end it fell off, +0x70 the
 * strip across the road, +0x74 whether it is followed.
 *
 * The track's .ccm (tr%02x.ccm, [0x552e48]: a count at 4, 0x1c bytes each
 * from 8, each placed on its nearest node, at +0x18, and sorted by it) and
 * simhts.dat ([0x552e54]) are loaded with the nodes.
 *
 * Each is a typed function under a name of its own, with a thunk under the game's name for
 * the callers still on registers and for the calls by address (asThunk): every register but eax as the caller
 * had it, eax what the core gives back, and a register the code leaves that is no argument of its
 * (ebx, edx, esi where they are) as that code leaves it.  The guest's
 * memory is read and written where the code does; a frame stays on the guest's stack, as deep as
 * the code's, only where a call is handed an address in it (GuestFrame).  The x87's arithmetic is
 * fp's in the game's order, a float kept where the game stores one; its compares fp::compare's
 * bits.  The surface's switch on the track goes by the targets nfs3.exe's table at 0x420814
 * holds.  Other typed files' functions are called as C++ (guest::direct, their headers); every
 * other function of the game's by address, its arguments in registers and on the guest's stack as
 * the code hands them, every other register kept. */
namespace nfs3hp
{
namespace trackdata
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The track's: the lanes' offsets, the header, its nodes, their count.
constexpr u32 kLaneSpan = 0x5dd930, kLaneShift = 0x5dd934;
constexpr u32 kHeader = 0x5dd938, kNodesToo = 0x5dd93c, kNodeCount = 0x5dd940, kNodes = 0x5dd948;
constexpr u32 kBlocks = 0x571370;  // 0x5c0 a block of 8 nodes
// The .ccm's block, its points' count and first point; simhts.dat's block.
constexpr u32 kCcm = 0x552e48, kCcmCount = 0x552e4c, kCcmPoints = 0x552e50, kSimHeights = 0x552e54;
// The searches' steps along the nodes: 11, 7 and 3 of them.
constexpr u32 kSteps = 0x552e58, kNearSteps = 0x552e88, kLinkSteps = 0x552ea8;
// The track's number, the data's path, the .ccm's and simhts.dat's names.
constexpr u32 kDataPath = 0x7a27b4;
constexpr u32 kCcmName = 0x536d7c, kSimHeightsName = 0x536d94, kNotFound = 0x536dc4;
// 65536 and its inverse, doubles; 0.2.
constexpr u32 kFixed = 0x536dac, kUnfixed = 0x536db4, kUnfixedToo = 0x536dbc, kNear = 0x536dcc;
constexpr u32 kFixedToo = 0x536dd4, kUnfixedThree = 0x536ddc, kUnfixedFour = 0x536de4;
constexpr u32 kFixedThree = 0x536dec;
// Other typed files' functions, called as C++ (guest::direct): sprintf, __CHP and the fill below;
// the loads and the free (0x4e0ec0, 0x4e0fe0, 0x4e1890), the ground's
// distance (0x41aab0), the trace (0x42d500), vectors less another and a dot product (0x4e0000,
// 0x4e01f0) and a car's node (0x4217e0).
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kChop = 0x4dfd56, kFillZero = 0x4e070c;

u16 half(App* app, u32 a) { return app->getMemory<u16>(a); }
void putHalf(App* app, u32 a, u16 v) { app->getMemory<u16>(a) = v; }
s32 s(u32 v) { return s32(v); }
Float f32(App* app, u32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, u32 a) { return Float(app->getMemory<double>(a)); }
Float i32(App* app, u32 a) { return Float(s32(at(app, a))); }
void putF32(App* app, u32 a, const Float& v) { app->getMemory<float>(a) = float(v); }

/* What fcom leaves in its status word for a below b, or unordered: C0. */
constexpr u16 kBelow = 0x100;

/* The block of node `node`: kBlocks + 0x5c0 * (node >> 3), as the code works it out. */
u32 blockOffset(u32 node)
{
    const u32 eighth = u32(s(node) >> 3);
    return ((eighth * 4 - eighth) * 8 - eighth) << 6;
}

// A place on the track's fields (see above).
constexpr u32 kPlaceBlock = 0, kPlacePoint = 4, kPlaceNode = 0x10, kPlaceStrip = 0x14, kPlacePolygon = 0x18,
                kPlaceNormal = 0x1c, kPlaceEntry = 0x20, kPlaceFixed = 0x24, kPlaceCorners = 0x54,
                kPlaceFound = 0x64, kPlaceNodeChanged = 0x68, kPlaceStripChanged = 0x6a, kPlaceEnd = 0x6c,
                kPlaceAcross = 0x70, kPlaceFollowed = 0x74;
constexpr u32 kPlaceSize = 0x78;
// A block's fields: its strips, vertices, colours, first node, nodes' entries, polygons, normals,
// items' count and items.
constexpr u32 kBlockStrips = 0x3c, kBlockVertices = 0x70, kBlockColours = 0x74, kBlockFirstNode = 0x584,
                kBlockEntries = 0x58c, kBlockPolygons = 0x594, kBlockNormals = 0x59c, kBlockItemCount = 0x5a0,
                kBlockItems = 0x5a4;
constexpr u32 kVertexSize = 12, kStripSize = 14, kItemSize = 20, kEntrySize = 8, kNodeSize = 0x24;
// A polygon's kind, the byte at 1.
constexpr u32 kPolygonKind = 1;
// What +0x64 holds: nothing found on the node, or the quad under the place.
constexpr u32 kFoundNothing = 1, kFoundQuad = 2;

/* The strip's k-th word (0 to 3): a vertex's index, signed, as the code reads it -- the first
 * as a word made a dword, the others as the top half of the dword two bytes before. */
u32 stripWord(App* app, u32 strip, u32 k)
{
    return u32(s32(nfs::s16(half(app, strip + 2 * k))));
}

// A node: its place (16.16, three words), its tunnel nibbles (the byte at 0xe; 2 a tunnel), its
// vectors' signed bytes (up at 0x10, ahead at 0x14, the side at 0x18), its widths' nibbles (the
// byte at 0x13: right low, left high) and bytes (left at 0x17, right at 0x1b).
constexpr u32 kNodeTunnel = 0xe, kNodeUp = 0x10, kNodeAhead = 0x14, kNodeSide = 0x18, kNodeWidths = 0x13,
                kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b;
constexpr u8 kTunnel = 2;

/* imul edx; shl edx, 16; shr eax, 16; adc eax, edx: a and b's product in 16.16, rounded by the
 * bit shifted out. */
u32 fixedProduct(u32 a, u32 b)
{
    const std::int64_t product = std::int64_t(s(a)) * s(b);
    const u32 low = u32(product), high = u32(std::uint64_t(product) >> 32);
    return (low >> 16) + (high << 16) + ((low >> 15) & 1);
}

/* fixedProduct's high dword << 16, which fixedMul leaves in edx. */
u32 fixedHigh(u32 a, u32 b)
{
    return u32(std::uint64_t(std::int64_t(s(a)) * s(b)) >> 32) << 16;
}

/* The signed byte at `a`, as the code reads it: the top of the dword three bytes before. */
u32 signedByte(App* app, u32 a)
{
    return u32(s32(nfs::s8(byte(app, a))));
}

/* A node's vector's k-th byte at `vector`, << 9 as the code scales it: an int of 16.16. */
u32 nodeAxis(App* app, u32 node, u32 vector, u32 k)
{
    return signedByte(app, node + vector + k) << 9;
}

/* n words from `from` to `to`, a word at a time, as movsd copies them. */
void copyWords(App* app, u32 from, u32 to, u32 n)
{
    for (u32 k = 0; k < n; ++k)
        put(app, to + 4 * k, at(app, from + 4 * k));
}

/* A thunk's run of its core: every register but eax as the caller had it after it (the code
 * pushes and pops what it uses; its other registers are its arguments, which no caller reads), eax
 * what the core gives -- a core that gives nothing, or answers only whether the game went on (a
 * bool), leaves eax as it is -- and the return address and `popped` bytes of the caller's words
 * popped.  False when the game stopped. */
template <typename T>
struct IsOptional : std::false_type
{
};

template <typename T>
struct IsOptional<std::optional<T>> : std::true_type
{
};

template <typename Core>
bool asThunk(ThreadState& ts, nfs::Args& args, Core&& core, u32 popped = 0)
{
    using Result = std::decay_t<decltype(core())>;
    const u32 esp = ts.sp;
    std::optional<u32> left;
    {
        if constexpr (std::is_void_v<Result>)
            core();
        else if constexpr (std::is_same_v<Result, bool>)
        {
            if (!core())
                return false;
        }
        else if constexpr (IsOptional<Result>::value)
        {
            const Result result = core();
            if (!result)
                return false;
            left = u32(*result);
        }
        else
            left = u32(core());
    }
    ts.sp = esp;
    if (left)
        args.eax = *left;
    ts.sp += 4 + popped;
    return true;
}

/* The caller's word `n` (from 1) on the guest's stack, above the return address. */
u32 word(App* app, const ThreadState& ts, u32 n) { return at(app, ts.sp + 4 * n); }

/* A function's frame on the guest's stack as deep as the code's, for what its callees are handed
 * there: `pushed` bytes of saved registers below the return address's slot, then `bytes` of
 * locals; esp as it was when it ends.  A core calling a core makes the call's slot with
 * GuestFrame(ts, 0, 4). */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }
    u32 operator+(u32 offset) const { return base + offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

}  // namespace

nfs::u32 loadCcmPoints(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 freeCcmPoints(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 loadSimHeights(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 freeSimHeights(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 openTrackNodes(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 closeTrackNodes(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nearestNodeCoarse(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 followNode(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nearestNode(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadVertices(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadStepForward(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadStepBack(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadToFixed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadSet(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nodeSection(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 resetPosition(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findQuad(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findQuadNearby(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findQuadByLinks(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findQuadFallback(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 locateOnRoad(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 updatePosition(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findMatchingQuad(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quadEdgeFlags(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sectionItemCount(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sectionItem(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setLaneOffsets(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lanePosition(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 positionFrame(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 snapToTrack(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 placeOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 positionColour(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 positionSurface(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 surfaceUnder(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 inTunnel(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carFacingWay(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carSectionKind(App* app, nfs::ThreadState& ts, nfs::Args& args);

namespace
{

/* sar edx, 31; idiv: the remainder of n by d, as the code's signed division leaves it in edx. */
u32 remainderOf(u32 n, u32 d)
{
    return u32(std::int64_t(s(n)) % std::int64_t(s(d)));
}

/* sprintf (cdecl, 0x4df690) of `format` and the words after it into `to`: its arguments pushed,
 * its call, the stack taken back.  False when the game stopped. */
bool print(App* app, ThreadState& ts, u32 to, u32 format, std::initializer_list<u32> words)
{
    guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, to, format, guest::Words(words));
    return !ts.terminate;
}

/* 0x41aab0 (vertices::groundDistanceOf): the distance from node `node` to the point; nullopt when
 * the game stopped. */
std::optional<u32> distance(App* app, ThreadState& ts, u32 node, u32 point)
{
    const u32 away = guest::direct<0x41aab0, vertices::groundDistanceAt>(app, ts, node, point);
    if (ts.terminate)
        return std::nullopt;
    return away;
}

/* ea::purgememadr (0x4e1890) of the block at `slot`, if there is one, and the slot cleared.  The
 * eax the free leaves, what it gives back -- `eax`, the code's, with no block; nullopt when the
 * game stopped. */
std::optional<u32> freeBlock(App* app, ThreadState& ts, u32 slot, u32 eax)
{
    const u32 block = at(app, slot);
    if (block == 0)
        return eax;
    const std::optional<u32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
    if (!freed)
        return std::nullopt;
    put(app, slot, 0);
    return freed;
}

}  // namespace

/* What 0x41e470 leaves: the node it settled on, and edx -- the last x difference it worked out,
 * or the last remainder when it went round the track's end, -1 when it never looked. */
struct Walked
{
    u32 node;
    u32 scratch;
};

/* 0x41e470's: the place on the track's node walked to the one nearest the point: the squares of
 * x's and z's differences (each >> 9) to the node on and the one before, round the track's end,
 * while one is nearer; the node to +0x10.  The code's 32-bit squares and sums, signed compares. */
Walked walkToNearest(App* app, u32 point, u32 place)
{
    const u32 nodes = at(app, kNodes);
    const u32 count = at(app, kNodeCount), last = count - 1;
    u32 scratch = 0xffffffff;
    const auto span = [&](u32 index) {
        const u32 node = nodes + index * kNodeSize;
        const u32 z = u32(s(at(app, point + 8) - at(app, node + 8)) >> 9);
        scratch = at(app, point) - at(app, node);
        const u32 x = u32(s(scratch) >> 9);
        return z * z + x * x;
    };
    u32 node = at(app, place + kPlaceNode);
    for (u32 before = 0xffffffff; node != before;)
    {
        before = node;
        const u32 here = span(s(node) < s(last) ? node : remainderOf(node, count));
        // The next node nearer?
        if (s(node) < s(last - 1))
        {
            if (s(span(node + 1)) < s(here))
            {
                ++node;
                continue;
            }
        }
        else if (s(span(remainderOf(node + 1, count))) < s(here))
        {
            node = remainderOf(node + 1, count);
            scratch = node;
            continue;
        }
        // The node before nearer?
        if (s(node) > 0)
        {
            if (s(span(node - 1)) < s(here))
                --node;
            continue;
        }
        node += count;
        if (s(span(remainderOf(node - 1, count))) < s(here))
            --node;
        node = remainderOf(node + count, count);
        scratch = node;
    }
    put(app, place + kPlaceNode, node);
    return {node, scratch};
}

/* 0x41e420's: the place on the track moved to the node nearest the point (walkToNearest), its
 * block taken, and whether its node changed to the words at +0x68 and +0x6a: 1 or 0. */
u32 followToNearest(App* app, u32 point, u32 place)
{
    const u32 before = at(app, place + kPlaceNode);
    walkToNearest(app, point, place);
    put(app, place + kPlaceBlock, kBlocks + blockOffset(at(app, place + kPlaceNode)));
    const u16 changed = before != at(app, place + kPlaceNode) ? 1 : 0;
    putHalf(app, place + kPlaceNodeChanged, changed);
    putHalf(app, place + kPlaceStripChanged, changed);
    return changed;
}

/* 0x41e3c0's: the node nearest the point (0x41aab0's distance), of every 8th, to the place on the
 * track's +0x10 (-1 for none).  eax as the code leaves it: the last distance, the count with no
 * node; nullopt when the game stopped. */
std::optional<u32> nearestEighth(App* app, ThreadState& ts, u32 point, u32 place)
{
    u32 nearest = 0x7fffffff, found = 0xffffffff;
    const u32 count = at(app, kNodeCount);
    u32 eax = count;
    for (u32 node = 0; s(node) < s(count); node += 8)
    {
        const std::optional<u32> d = distance(app, ts, at(app, kNodes) + node * kNodeSize, point);
        if (!d)
            return std::nullopt;
        eax = *d;
        if (s(*d) < s(nearest))
        {
            nearest = *d;
            found = node;
        }
    }
    put(app, place + kPlaceNode, found);
    return eax;
}

// A .ccm's point: 0x1c bytes, its node at 0x18.
constexpr u32 kCcmPointSize = 0x1c, kCcmPointNode = 0x18;

/* 0x41e180's: the track's tr%02x.ccm (of [0x6fd4ac], under the data's path) loaded whole
 * (loadFileQuietly), its count of points kept at 0x552e4c and its first at 0x552e50; each point
 * placed on its nearest node (nearestEighth, then followToNearest, through a place on the track in
 * the frame), the node at its +0x18, and the points sorted by it, a bubble sort.  The frame as deep
 * as the code's, for the name's address the calls are handed.  What the code leaves in eax: 0
 * without the file, else the count less one; nullopt when the game stopped. */
std::optional<u32> loadCcm(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x298);
    const u32 name = frame - 0x298, place = frame - 0x98;
    const u32 track = gameSettings(app).track;
    if (!print(app, ts, name, kCcmName, {kDataPath, track, track}))
        return std::nullopt;
    put(app, kCcmCount, 0);
    const std::optional<u32> loaded = guest::direct<0x4e0fe0, ea::loadQuietly>(app, ts, name, 0);
    if (!loaded)
        return std::nullopt;
    const u32 file = *loaded;
    put(app, kCcm, file);
    if (file == 0)
        return 0;
    put(app, kCcmCount, at(app, file + 4));
    put(app, kCcmPoints, file + 8);
    const auto point = [&](u32 i) { return at(app, kCcmPoints) + i * kCcmPointSize; };
    // Each point on its node.
    for (u32 i = 0; s(i) < s(at(app, kCcmCount)); ++i)
    {
        put(app, place + kPlaceNode, 0);
        if (!nearestEighth(app, ts, point(i), place))
            return std::nullopt;
        followToNearest(app, point(i), place);
        put(app, point(i) + kCcmPointNode, at(app, place + kPlaceNode));
    }
    // Sorted by their nodes.
    for (u32 pass = 0; s(at(app, kCcmCount) - 1) > s(pass); ++pass)
        for (u32 i = 0; s(i) < s(at(app, kCcmCount) - 1); ++i)
        {
            const u32 next = point(i + 1), here = point(i);
            if (s(at(app, here + kCcmPointNode)) > s(at(app, next + kCcmPointNode)))
            {
                u32 kept[kCcmPointSize / 4];
                for (u32 k = 0; k < kCcmPointSize / 4; ++k)
                    kept[k] = at(app, here + 4 * k);
                for (u32 k = 0; k < kCcmPointSize / 4; ++k)
                    put(app, here + 4 * k, at(app, next + 4 * k));
                for (u32 k = 0; k < kCcmPointSize / 4; ++k)
                    put(app, next + 4 * k, kept[k]);
            }
        }
    return at(app, kCcmCount) - 1;
}

/* 0x41e180: loadCcm. */
nfs::u32 loadCcmPoints(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return loadCcm(app, ts); });
    return args.eax;
}

/* 0x41e2c0: the .ccm's block freed, if there is one, and forgotten (freeBlock). */
nfs::u32 freeCcmPoints(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, eax = args.eax] { return freeBlock(app, ts, kCcm, eax); });
    return args.eax;
}

/* 0x41e2f0's: the track's simhts.dat (of [0x6fd4ac], under the data's path) loaded whole
 * (ea::loadFile, flags 0), its block to 0x552e54 and returned.  The frame as deep as the code's,
 * for the name's address.  nullopt when the game stopped. */
std::optional<u32> loadHeights(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 8, 0x200);
    const u32 name = frame - 0x200;
    if (!print(app, ts, name, kSimHeightsName, {kDataPath, gameSettings(app).track}))
        return std::nullopt;
    const std::optional<u32> block = guest::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, 0);
    if (!block)
        return std::nullopt;
    put(app, kSimHeights, *block);
    return *block;
}

/* 0x41e2f0: loadHeights; the block in eax, as the load leaves it. */
nfs::u32 loadSimHeights(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return loadHeights(app, ts); });
    return args.eax;
}

/* 0x41e340: simhts.dat's block freed, if there is one, and forgotten (freeBlock). */
nfs::u32 freeSimHeights(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, eax = args.eax] { return freeBlock(app, ts, kSimHeights, eax); });
    return args.eax;
}

/* 0x41e370's: the track's header taken: its nodes from +8, their count the word at +6; the .ccm
 * and simhts.dat loaded, each called as the code calls it, at its depth.  False when the game
 * stopped. */
bool openTrack(App* app, ThreadState& ts, u32 header)
{
    put(app, kHeader, header);
    put(app, kNodesToo, header + 8);
    put(app, kNodes, header + 8);
    put(app, kNodeCount, half(app, header + 6));
    const GuestFrame frame(ts, 8, 0);
    {
        const GuestFrame slot(ts, 0, 4);
        if (!loadCcm(app, ts))
            return false;
    }
    const GuestFrame slot(ts, 0, 4);
    return loadHeights(app, ts).has_value();
}

/* 0x41e370: openTrack of the header eax; 1. */
nfs::u32 openTrackNodes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, header = args.eax]() -> std::optional<u32> {
        if (!openTrack(app, ts, header))
            return std::nullopt;
        return 1;
    });
    return args.eax;
}

/* 0x41e3b0's: the .ccm and simhts.dat freed; eax as the last free leaves it (`eax`, the code's,
 * with neither).  Nullopt when the game stopped. */
std::optional<u32> closeTrackEax(App* app, ThreadState& ts, u32 eax)
{
    const std::optional<u32> freed = freeBlock(app, ts, kCcm, eax);
    if (!freed)
        return std::nullopt;
    return freeBlock(app, ts, kSimHeights, *freed);
}

bool closeTrack(App* app, ThreadState& ts)
{
    return closeTrackEax(app, ts, 0).has_value();
}

/* 0x41e3b0: closeTrackEax. */
nfs::u32 closeTrackNodes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, eax = args.eax] { return closeTrackEax(app, ts, eax); });
    return args.eax;
}

/* 0x41e3c0: nearestEighth of the point eax for the place edx. */
nfs::u32 nearestNodeCoarse(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx] { return nearestEighth(app, ts, point, place); });
    return args.eax;
}

/* 0x41e420: followToNearest of the point eax for the place edx. */
nfs::u32 followNode(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx] { return followToNearest(app, point, place); });
    return args.eax;
}

/* 0x41e470: walkToNearest of the point eax for the place edx; eax the place. */
nfs::u32 nearestNode(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx] {
        walkToNearest(app, point, place);
        return place;
    });
    return args.eax;
}

/* The vertex of the place on the track's block that its strip's k-th word names. */
u32 stripCorner(App* app, u32 place, u32 k)
{
    const u32 vertices = at(app, at(app, place + kPlaceBlock) + kBlockVertices);
    return vertices + kVertexSize * stripWord(app, at(app, place + kPlaceStrip), k);
}

/* 0x41e6a0's: the place on the track's quad, the four vertices its strip names: +0x54 the
 * strip's second word's, +0x58 its first's, +0x5c its fourth's, +0x60 its third's. */
void takeQuadVertices(App* app, u32 place)
{
    put(app, place + kPlaceCorners, stripCorner(app, place, 1));
    put(app, place + kPlaceCorners + 4, stripCorner(app, place, 0));
    put(app, place + kPlaceCorners + 8, stripCorner(app, place, 3));
    put(app, place + kPlaceCorners + 12, stripCorner(app, place, 2));
}

/* 0x41e6a0: takeQuadVertices for the place eax. */
nfs::u32 quadVertices(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { takeQuadVertices(app, place); });
    return args.eax;
}

/* 0x41e720's: the quad a strip on across the road: the old +0x54 and +0x60 now +0x58 and +0x5c,
 * the new strip's second and third words' vertices +0x54 and +0x60. */
void stepQuadForward(App* app, u32 place)
{
    put(app, place + kPlaceCorners + 4, at(app, place + kPlaceCorners));
    put(app, place + kPlaceCorners + 8, at(app, place + kPlaceCorners + 12));
    put(app, place + kPlaceCorners, stripCorner(app, place, 1));
    put(app, place + kPlaceCorners + 12, stripCorner(app, place, 2));
}

/* 0x41e720: stepQuadForward for the place eax. */
nfs::u32 quadStepForward(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { stepQuadForward(app, place); });
    return args.eax;
}

/* 0x41e770's: the quad a strip back: the old +0x58 and +0x5c now +0x54 and +0x60, the new strip's
 * first and fourth words' vertices +0x58 and +0x5c. */
void stepQuadBack(App* app, u32 place)
{
    put(app, place + kPlaceCorners, at(app, place + kPlaceCorners + 4));
    put(app, place + kPlaceCorners + 12, at(app, place + kPlaceCorners + 8));
    put(app, place + kPlaceCorners + 4, stripCorner(app, place, 0));
    put(app, place + kPlaceCorners + 8, stripCorner(app, place, 3));
}

/* 0x41e770: stepQuadBack for the place eax. */
nfs::u32 quadStepBack(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { stepQuadBack(app, place); });
    return args.eax;
}

/* 0x41e7c0's: the quad's four vertices (three floats each) in 16.16, to the place on the track's
 * +0x24 on: each * 65536 (a double), rounded as fistp does.  The last of them, which the code
 * leaves in eax. */
u32 fixQuad(App* app, u32 place)
{
    u32 last = 0;
    for (u32 k = 0; k < 12; ++k)
    {
        const u32 vertex = at(app, place + kPlaceCorners + 4 * (k / 3));
        const Float value = fp::mul(f32(app, vertex + 4 * (k % 3)), f64(app, kFixed));
        last = u32(fp::toInteger<s32>(value));
        put(app, place + kPlaceFixed + 4 * k, last);
    }
    return last;
}

/* 0x41e7c0: fixQuad for the place eax; eax the last word. */
nfs::u32 quadToFixed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { return fixQuad(app, place); });
    return args.eax;
}

/* 0x41e8d0's: the quad's vertices taken and made 16.16; fixQuad's last word. */
u32 setQuad(App* app, u32 place)
{
    takeQuadVertices(app, place);
    return fixQuad(app, place);
}

/* 0x41e8d0: setQuad for the place eax. */
nfs::u32 quadSet(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { return setQuad(app, place); });
    return args.eax;
}

/* 0x41e8f0's: the place on the track's block, of its node, and its node's entry in it (8 bytes
 * each from the block's first node).  The entry's offset, which the code leaves in eax. */
u32 takeNodeEntry(App* app, u32 place)
{
    const u32 block = kBlocks + blockOffset(at(app, place + kPlaceNode));
    put(app, place + kPlaceBlock, block);
    const u32 offset = (at(app, place + kPlaceNode) - at(app, block + kBlockFirstNode)) * kEntrySize;
    put(app, place + kPlaceEntry, at(app, block + kBlockEntries) + offset);
    return offset;
}

/* 0x41e8f0: takeNodeEntry for the place eax. */
nfs::u32 nodeSection(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { return takeNodeEntry(app, place); });
    return args.eax;
}

/* 0x41e940's: the place on the track at node `node`, nothing found on it; takeNodeEntry's
 * offset. */
u32 resetPlace(App* app, u32 node, u32 place)
{
    put(app, place + kPlaceEntry, 0);
    put(app, place + kPlaceStrip, 0);
    put(app, place + kPlacePolygon, 0);
    put(app, place + kPlaceNormal, 0);
    putHalf(app, place + kPlaceNodeChanged, 0);
    putHalf(app, place + kPlaceStripChanged, 0);
    put(app, place + kPlaceEnd, 0);
    put(app, place + kPlaceFound, kFoundNothing);
    put(app, place + kPlaceAcross, 0xffffffff);
    put(app, place + kPlaceNode, node);
    return takeNodeEntry(app, place);
}

/* 0x41e940: resetPlace at node eax for the place edx. */
nfs::u32 resetPosition(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, place = args.edx] { return resetPlace(app, node, place); });
    return args.eax;
}


namespace
{

// A node's entry's strips: the first's word (at 0), their count (at 2).  A polygon's kind (the
// low nibble at 1) off the road.
constexpr u32 kEntryStrips = 2;
constexpr u8 kOffRoad = 0xe;

/* The strips across the road at the place on the track's node. */
u32 stripsAcross(App* app, u32 place)
{
    return byte(app, at(app, place + kPlaceEntry) + kEntryStrips);
}

/* The place on the track's strip and polygon across the road (+0x70) as its block has them: the
 * node's first strip's word + the strip across. */
u32 stripIndex(App* app, u32 place)
{
    return half(app, at(app, place + kPlaceEntry)) + at(app, place + kPlaceAcross);
}

/* The place on the track's polygon, of its block's, to +0x18; whether it is on the road (its
 * kind not 0xe). */
bool onRoadPolygon(App* app, u32 place)
{
    const u32 polygon = at(app, at(app, place + kPlaceBlock) + kBlockPolygons) + stripIndex(app, place) * 8;
    put(app, place + kPlacePolygon, polygon);
    return (byte(app, polygon + kPolygonKind) & 0xf) != kOffRoad;
}

/* Found on the road: +0x64 2, +0x6c 0, the polygon's normal (its block's + 12 * the polygon's
 * first byte) to +0x1c, and the quad made 16.16 (fixQuad).  The polygon's byte, which the code
 * leaves in edx. */
u32 takeFoundNormal(App* app, u32 place)
{
    put(app, place + kPlaceFound, kFoundQuad);
    put(app, place + kPlaceEnd, 0);
    const u32 normals = at(app, at(app, place + kPlaceBlock) + kBlockNormals);
    const u32 first = byte(app, at(app, place + kPlacePolygon));
    put(app, place + kPlaceNormal, normals + first * kVertexSize);
    fixQuad(app, place);
    return first;
}

/* The place on the track's strip across the road, of its block's, to +0x14, and its quad's
 * vertices taken. */
void takeStripAcross(App* app, u32 place)
{
    put(app, place + kPlaceStrip,
        at(app, at(app, place + kPlaceBlock) + kBlockStrips) + stripIndex(app, place) * kStripSize);
    takeQuadVertices(app, place);
}

/* The place on the track's node walked n nodes on (or back), round the track's ends.  What the
 * code leaves in the register it steps back in: the last node stepped back to (the count less one
 * where it wrapped), else `unchanged`. */
u32 walkPlace(App* app, u32 place, bool on, u32 n, u32 unchanged)
{
    u32 left = unchanged;
    for (u32 k = 0; k != n; ++k)
        if (on)
        {
            const u32 node = at(app, place + kPlaceNode) + 1, count = at(app, kNodeCount);
            put(app, place + kPlaceNode, node);
            if (s(node) >= s(count))
                put(app, place + kPlaceNode, 0);
        }
        else
        {
            left = at(app, place + kPlaceNode) - 1;
            put(app, place + kPlaceNode, left);
            if (s(left) < 0)
            {
                left = at(app, kNodeCount) - 1;
                put(app, place + kPlaceNode, left);
            }
        }
    return left;
}

/* walkPlace by a step: its size, on where it is above 0. */
void walkPlaceBy(App* app, u32 place, u32 step)
{
    walkPlace(app, place, s(step) > 0, s(step) > 0 ? step : 0u - step, 0);
}

/* The steps' sum walked back, as the searches end when nothing is found: on where it is below 0.
 * walkPlace's register, the sum negated without a step back. */
u32 walkPlaceBack(App* app, u32 place, u32 sum)
{
    const u32 negated = 0u - sum;
    return walkPlace(app, place, s(negated) > 0, s(negated) > 0 ? negated : sum, negated);
}

/* Whether the point (px and pz, floats) is on the inner side of the quad's side from the vertex at
 * a to the one at b: (a.x - b.x) * (z - b.z) not below (a.z - b.z) * (x - b.x), a NaN below -- the
 * code's fcom and jb. */
bool innerSide(App* app, u32 a, u32 b, const Float& px, const Float& pz)
{
    const Float first = fp::mul(fp::sub(f32(app, a), f32(app, b)), fp::sub(pz, f32(app, b + 8)));
    const Float second = fp::mul(fp::sub(f32(app, a + 8), f32(app, b + 8)), fp::sub(px, f32(app, b)));
    return (fp::compare(second, first) & kBelow) == 0;
}

/* Whether the point is in the place on the track's quad: on the inner side of its four sides in
 * turn -- the corners at +0x58 to +0x5c, +0x54 to +0x58, +0x5c to +0x60, +0x60 to +0x54. */
bool insideQuad(App* app, u32 place, const Float& px, const Float& pz)
{
    const auto corner = [&](u32 k) { return at(app, place + kPlaceCorners + 4 * k); };
    return innerSide(app, corner(1), corner(2), px, pz) && innerSide(app, corner(0), corner(1), px, pz)
           && innerSide(app, corner(2), corner(3), px, pz) && innerSide(app, corner(3), corner(0), px, pz);
}

/* The x87's fabs in the code's two steps on a float: only a value below 0 negated, so a NaN and
 * -0 kept to the bit. */
float sizeOf(float value)
{
    return 0.0 > Float(value) ? float(-Float(value)) : value;
}

/* __CHP (0x4dfd56): the value truncated, then fistp'd; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<u32> chopped(App* app, ThreadState& ts, const Float& value)
{
    const double whole = guest::direct<kChop, watcom::clib::chop>(app, ts, double(value));
    return u32(fp::toInteger<s32>(Float(whole)));
}

/* How far the point is from the quad's corners at +0x54 and +0x5c, along x and z: each
 * difference's size a float, as the code stores them, summed as it does, truncated (chopped).
 * nullopt when the game stopped. */
std::optional<u32> cornersAway(App* app, ThreadState& ts, u32 place, const Float& px, const Float& pz)
{
    const u32 first = at(app, place + kPlaceCorners), third = at(app, place + kPlaceCorners + 8);
    const float firstZ = sizeOf(float(fp::sub(pz, f32(app, first + 8))));
    const float firstX = sizeOf(float(fp::sub(px, f32(app, first))));
    const float firstSum = float(fp::add(Float(firstX), Float(firstZ)));
    const float thirdZ = sizeOf(float(fp::sub(pz, f32(app, third + 8))));
    const float thirdX = sizeOf(float(fp::sub(px, f32(app, third))));
    return chopped(app, ts, fp::add(fp::add(Float(thirdX), Float(thirdZ)), Float(firstSum)));
}

/* The searches' point: its x and z (16.16) times a double, as floats. */
void pointFloats(App* app, u32 x, u32 z, u32 scale, Float& px, Float& pz)
{
    const Float k = f64(app, scale);
    px = Float(float(fp::mul(Float(s(x)), k)));
    pz = Float(float(fp::mul(k, Float(s(z)))));
}

}  // namespace

/* 0x41e990's: the road's quad under the point looked for about the place on the track's node --
 * across the road from the strip it was on (or the middle one) outwards, the quad's four sides'
 * tests, then inwards from the strip before; off either end, the end at +0x6c; on to the next
 * strip while its corners come no nearer (cornersAway) or the polygon there is off the road --
 * then the nodes on and back by 0x552e58's steps (1, -2, 3, ...), 11 of them.  Found: the polygon,
 * its normal and the quad in 16.16 taken, 1, edx the polygon's byte.  Else the node as it was, no
 * strip or polygon, 0, edx as the steps walked back leave it.  nullopt when the game stopped. */
std::optional<Searched> searchQuad(App* app, ThreadState& ts, u32 point, u32 place)
{
    Float px, pz;
    pointFloats(app, at(app, point), at(app, point + 8), kUnfixed, px, pz);
    put(app, place + kPlaceStrip, 0);
    put(app, place + kPlacePolygon, 0);
    takeNodeEntry(app, place);
    u32 sum = 0, start;
    const u32 across = at(app, place + kPlaceAcross);
    if (s(across) >= 0 && s(stripsAcross(app, place) - 1) > s(across) && at(app, place + kPlaceEnd) == 0)
        start = across;
    else
    {
        const u32 last = stripsAcross(app, place) - 1;
        start = u32(s(last - u32(s(last) >> 31)) >> 1);
        put(app, place + kPlaceAcross, start);
    }
    put(app, place + kPlaceEnd, 0);
    put(app, place + kPlaceAcross, start);
    for (u32 k = 0; s(k) < 0xb; ++k)
    {
        takeNodeEntry(app, place);
        put(app, place + kPlaceAcross, start);
        const u32 strips = stripsAcross(app, place);
        if (s(strips - 1) < s(at(app, place + kPlaceAcross)))
            put(app, place + kPlaceAcross, strips - 2);
        u32 nearest = 0x7fffffff;
        takeStripAcross(app, place);
        // Outwards.
        while (s(stripsAcross(app, place) - 1) >= s(at(app, place + kPlaceAcross)))
        {
            if (insideQuad(app, place, px, pz) && onRoadPolygon(app, place))
                return Searched{1, takeFoundNormal(app, place)};
            if (stripsAcross(app, place) - 1 == at(app, place + kPlaceAcross))
            {
                if (at(app, place + kPlaceEnd) == 0
                    && innerSide(app, at(app, place + kPlaceCorners + 4), at(app, place + kPlaceCorners + 8), px, pz))
                    put(app, place + kPlaceEnd, 2);
                break;
            }
            const std::optional<u32> away = cornersAway(app, ts, place, px, pz);
            if (!away)
                return std::nullopt;
            if (s(nearest) < s(*away) && onRoadPolygon(app, place))
                break;
            nearest = *away;
            put(app, place + kPlaceAcross, at(app, place + kPlaceAcross) + 1);
            put(app, place + kPlaceStrip, at(app, place + kPlaceStrip) + kStripSize);
            stepQuadForward(app, place);
        }
        // Inwards, from the strip before the one it started at.
        put(app, place + kPlaceAcross, start - 1);
        nearest = 0x7fffffff;
        takeStripAcross(app, place);
        while (s(at(app, place + kPlaceAcross)) >= 0)
        {
            if (insideQuad(app, place, px, pz) && onRoadPolygon(app, place))
                return Searched{1, takeFoundNormal(app, place)};
            if (at(app, place + kPlaceAcross) == 0)
            {
                if (at(app, place + kPlaceEnd) == 0
                    && innerSide(app, at(app, place + kPlaceCorners + 12), at(app, place + kPlaceCorners), px, pz))
                    put(app, place + kPlaceEnd, 1);
                break;
            }
            const std::optional<u32> away = cornersAway(app, ts, place, px, pz);
            if (!away)
                return std::nullopt;
            if (s(nearest) < s(*away) && onRoadPolygon(app, place))
                break;
            nearest = *away;
            put(app, place + kPlaceStrip, at(app, place + kPlaceStrip) - kStripSize);
            put(app, place + kPlaceAcross, at(app, place + kPlaceAcross) - 1);
            stepQuadBack(app, place);
        }
        // The next node of the steps.
        const u32 step = at(app, kSteps + k * 4);
        walkPlaceBy(app, place, step);
        sum += step;
    }
    // None: the steps walked back.
    const u32 scratch = walkPlaceBack(app, place, sum);
    takeNodeEntry(app, place);
    put(app, place + kPlaceStrip, 0);
    put(app, place + kPlacePolygon, 0);
    return Searched{0, scratch};
}

/* 0x41e990: searchQuad for the point eax about the place edx. */
nfs::u32 findQuad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx]() -> std::optional<u32> {
        const std::optional<Searched> searched = searchQuad(app, ts, point, place);
        if (!searched)
            return std::nullopt;
        return searched->found;
    });
    return args.eax;
}

/* 0x41f090's: the road's quad under the point (x and z, 16.16) looked for from the place on the
 * track's node -- across the road from the first strip outwards, then the nodes on and back by
 * 0x552e88's steps, 7 of them -- the quad's sides tested in turn.  Found: the polygon, its normal
 * and the quad in 16.16, 1; else the node as it was, no strip or polygon, 0. */
u32 searchNearby(App* app, u32 x, u32 z, u32 place)
{
    Float px, pz;
    pointFloats(app, x, z, kUnfixedToo, px, pz);
    put(app, place + kPlaceStrip, 0);
    put(app, place + kPlacePolygon, 0);
    put(app, place + kPlaceEnd, 0);
    u32 sum = 0;
    for (u32 k = 0; s(k) < 7; ++k)
    {
        takeNodeEntry(app, place);
        put(app, place + kPlaceAcross, 0);
        takeStripAcross(app, place);
        while (s(stripsAcross(app, place) - 1) >= s(at(app, place + kPlaceAcross)))
        {
            if (insideQuad(app, place, px, pz) && onRoadPolygon(app, place))
            {
                takeFoundNormal(app, place);
                return 1;
            }
            put(app, place + kPlaceStrip, at(app, place + kPlaceStrip) + kStripSize);
            put(app, place + kPlaceAcross, at(app, place + kPlaceAcross) + 1);
            stepQuadForward(app, place);
        }
        // Across the road: the next node of the steps.
        const u32 step = at(app, kNearSteps + k * 4);
        walkPlaceBy(app, place, step);
        sum += step;
    }
    walkPlaceBack(app, place, sum);
    takeNodeEntry(app, place);
    put(app, place + kPlaceStrip, 0);
    put(app, place + kPlacePolygon, 0);
    return 0;
}

/* 0x41f090: searchNearby for the point eax from the place edx. */
nfs::u32 findQuadNearby(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx] { return searchNearby(app, at(app, point), at(app, point + 8), place); });
    return args.eax;
}

// A node's entry's links: the nodes' words at 4 and 6, -1 for none.
constexpr u32 kEntryLinks = 4;
constexpr u16 kNoLink = 0xffff;

/* What 0x41f330 leaves: 1 found or 0, the step it found on (3 for none) in ebx, and edx as the
 * code leaves it. */
struct Linked
{
    u32 found;
    u32 step;
    u32 scratch;
};

/* 0x41f330's: the road's quad under the point looked for on the nodes the place on the track's
 * node (set to `node`) links to: for each of 0x552ea8's steps (0, 1, -2) along the nodes, of the
 * node's two links the nearer one (0x41aab0's distance, unsigned) followed and searched
 * (followToNearest, searchQuad) -- 1 when found; else the place as it was put back, without an
 * entry.  0 for none.  What the code leaves in edx: the search's, else the place or the count less
 * one as its steps along the nodes leave it, its low word the node's first link.  nullopt when the
 * game stopped. */
std::optional<Linked> searchLinks(App* app, ThreadState& ts, u32 point, u32 place, u32 node)
{
    put(app, place + kPlaceAcross, 0xffffffff);
    put(app, place + kPlaceNode, node);
    u32 scratch = place;
    for (u32 k = 0; s(k) < 3; ++k)
    {
        const u32 step = at(app, kLinkSteps + k * 4);
        const bool on = s(step) > 0;
        for (u32 n = on ? step : 0u - step; n != 0; --n)
        {
            scratch = place;
            if (on)
            {
                const u32 next = at(app, place + kPlaceNode) + 1, count = at(app, kNodeCount);
                put(app, place + kPlaceNode, next);
                if (s(next) >= s(count))
                    put(app, place + kPlaceNode, 0);
            }
            else
            {
                const u32 before = at(app, place + kPlaceNode) - 1;
                put(app, place + kPlaceNode, before);
                if (s(before) < 0)
                {
                    scratch = at(app, kNodeCount) - 1;
                    put(app, place + kPlaceNode, scratch);
                }
            }
        }
        takeNodeEntry(app, place);
        const auto link = [&](u32 which) {
            return half(app, at(app, place + kPlaceEntry) + kEntryLinks + 2 * which);
        };
        scratch = (scratch & 0xffff0000) | link(0);
        if (link(0) == kNoLink && link(1) == kNoLink)
            continue;
        // The place kept; the nearer link.
        u32 kept[kPlaceSize / 4];
        for (u32 w = 0; w < kPlaceSize / 4; ++w)
            kept[w] = at(app, place + 4 * w);
        u32 away[2] = {0xffffffff, 0xffffffff};
        for (u32 w = 0; w < 2; ++w)
            if (link(w) != kNoLink)
            {
                const u32 linked = u32(s32(nfs::s16(link(w))));
                const std::optional<u32> d = distance(app, ts, at(app, kNodes) + linked * kNodeSize, point);
                if (!d)
                    return std::nullopt;
                away[w] = *d;
            }
        put(app, place + kPlaceNode, u32(s32(nfs::s16(link(away[0] < away[1] ? 0 : 1)))));
        followToNearest(app, point, place);
        const std::optional<Searched> searched = searchQuad(app, ts, point, place);
        if (!searched)
            return std::nullopt;
        if (searched->found != 0)
            return Linked{1, k, searched->scratch};
        scratch = searched->scratch;
        // Not there: the place as it was, without its entry.
        for (u32 w = 0; w < kPlaceSize / 4; ++w)
            put(app, place + 4 * w, kept[w]);
        put(app, place + kPlaceEntry, 0);
        takeNodeEntry(app, place);
    }
    return Linked{0, 3, scratch};
}

/* 0x41f330: searchLinks for the point eax and the place edx, from node ebx. */
nfs::u32 findQuadByLinks(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx, node = args.ebx]() -> std::optional<u32> {
        const std::optional<Linked> linked = searchLinks(app, ts, point, place, node);
        if (!linked)
            return std::nullopt;
        return linked->found;
    });
    return args.eax;
}

// How far from its node a point may be before its node is looked for afresh: 1.0 in 16.16.
constexpr u32 kFarFromNode = 0x10000;

/* 0x41f500's: the place on the track for the point, where searchQuad and searchLinks found
 * nothing: off an end of the road (+0x6c 1 the first strip, else the last), that end's strip and
 * polygon taken as found (setQuad), ebx the polygon; more than 1.0 (0x41aab0's distance) from its
 * node, the node looked for afresh and searched (nearestEighth, walkToNearest, searchQuad); still
 * no strip, a place a 32nd of the way from the point to the node searched (searchNearby), ebx the
 * point's z, and 0x42d500 traced "c" if it is found there.  eax and ebx as the code leaves them
 * (`ebx` the caller's); nullopt when the game stopped. */
struct Fallback
{
    u32 eax, ebx;
};

std::optional<Fallback> fallbackQuad(App* app, ThreadState& ts, u32 point, u32 place, u32 ebx)
{
    if (at(app, place + kPlaceEnd) != 0)
    {
        put(app, place + kPlaceAcross, at(app, place + kPlaceEnd) == 1 ? 0 : stripsAcross(app, place) - 1);
        const u32 block = at(app, place + kPlaceBlock);
        put(app, place + kPlaceStrip, at(app, block + kBlockStrips) + stripIndex(app, place) * kStripSize);
        const u32 polygon = at(app, block + kBlockPolygons) + stripIndex(app, place) * 8;
        put(app, place + kPlaceFound, kFoundQuad);
        put(app, place + kPlacePolygon, polygon);
        ebx = polygon;
        const u32 normals = at(app, at(app, place + kPlaceBlock) + kBlockNormals);
        put(app, place + kPlaceNormal, normals + byte(app, at(app, place + kPlacePolygon)) * kVertexSize);
        setQuad(app, place);
    }
    const std::optional<u32> d =
        distance(app, ts, at(app, kNodes) + at(app, place + kPlaceNode) * kNodeSize, point);
    if (!d)
        return std::nullopt;
    u32 eax = *d;
    if (s(*d) > s(kFarFromNode))
    {
        if (!nearestEighth(app, ts, point, place))
            return std::nullopt;
        walkToNearest(app, point, place);
        const std::optional<Searched> searched = searchQuad(app, ts, point, place);
        if (!searched)
            return std::nullopt;
        eax = searched->found;
    }
    if (at(app, place + kPlaceStrip) != 0)
        return Fallback{eax, ebx};
    const auto node = [&] { return at(app, kNodes) + at(app, place + kPlaceNode) * kNodeSize; };
    const u32 x = at(app, point) + u32(s(at(app, node()) - at(app, point)) >> 5);
    ebx = at(app, point + 8);
    const u32 z = ebx + u32(s(at(app, node() + 8) - ebx) >> 5);
    eax = searchNearby(app, x, z, place);
    if (eax != 0)
    {
        // The trace's word pushed, as the code pushes it.
        const GuestFrame pushed(ts, 0, 4);
        put(app, pushed - 4, kNotFound);
        guest::direct<0x42d500, raceload::noHook>(app, ts);
        if (ts.terminate)
            return std::nullopt;
    }
    return Fallback{eax, ebx};
}

/* 0x41f500: fallbackQuad for the point eax and the place edx; ebx as the code leaves it (no
 * argument of its by its code's reading). */
nfs::u32 findQuadFallback(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<Fallback> left;
    asThunk(ts, args, [&, point = args.eax, place = args.edx, ebx = args.ebx]() -> std::optional<u32> {
        left = fallbackQuad(app, ts, point, place, ebx);
        if (!left)
            return std::nullopt;
        return left->eax;
    });
    if (left)
        args.ebx = left->ebx;
    return args.eax;
}

/* 0x41f650's: the place on the track found for the point: its nearest node followed
 * (followToNearest), its quad searched for (searchQuad, then searchLinks from the node it was at,
 * then fallbackQuad); +0x68 whether the node changed, +0x6a whether it or the strip did; the
 * first, a word made a dword, in eax.  Nullopt when the game stopped. */
std::optional<u32> locatePlace(App* app, ThreadState& ts, u32 point, u32 place)
{
    const u32 strip = at(app, place + kPlaceStrip), node = at(app, place + kPlaceNode);
    followToNearest(app, point, place);
    const std::optional<Searched> searched = searchQuad(app, ts, point, place);
    if (!searched)
        return std::nullopt;
    if (searched->found == 0)
    {
        const std::optional<Linked> linked = searchLinks(app, ts, point, place, node);
        if (!linked || (linked->found == 0 && !fallbackQuad(app, ts, point, place, linked->step)))
            return std::nullopt;
    }
    putHalf(app, place + kPlaceNodeChanged, node != at(app, place + kPlaceNode) ? 1 : 0);
    const bool changed = strip != at(app, place + kPlaceStrip) || half(app, place + kPlaceNodeChanged) != 0;
    putHalf(app, place + kPlaceStripChanged, changed ? 1 : 0);
    return u32(s32(nfs::s16(half(app, place + kPlaceNodeChanged))));
}

/* 0x41f650: locatePlace for the point eax and the place edx. */
nfs::u32 locateOnRoad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx] { return locatePlace(app, ts, point, place); });
    return args.eax;
}

namespace
{

/* A side of the place on the track's quad in 16.16 (corners at `a` and `b`, offsets of its +0x24
 * on): the point on its inner side when (a.x - b.x) * (z - b.z) is not above (a.z - b.z) *
 * (x - b.x), each a fixedProduct; the first product (ebx) and the second's high word (edx), as the
 * code leaves them. */
struct Side
{
    bool inside;
    u32 first;
    u32 high;
};

Side sideOf(App* app, u32 place, u32 point, u32 a, u32 b)
{
    const u32 first = fixedProduct(at(app, place + a) - at(app, place + b), at(app, point + 8) - at(app, place + b + 8));
    const u32 across = at(app, place + a + 8) - at(app, place + b + 8), along = at(app, point) - at(app, place + b);
    return {s(first) <= s(fixedProduct(across, along)), first, fixedHigh(across, along)};
}

}  // namespace

/* 0x41f6e0's: the place on the track brought to the point, followed (kept at +0x74) or not.
 * Followed and on a quad still -- the point on the inner side of its four sides, in 16.16
 * (sideOf) -- nothing changed, 0; else found again (locatePlace), the point copied to +4.  Not
 * followed: the place reset to the point and its nearest node (followToNearest), eax whether the
 * node changed.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> bringPlaceEax(App* app, ThreadState& ts, u32 point, u32 place, u32 followed)
{
    put(app, place + kPlaceFollowed, followed);
    if (followed == 0)
    {
        put(app, place + kPlaceEntry, 0);
        put(app, place + kPlaceStrip, 0);
        put(app, place + kPlacePolygon, 0);
        put(app, place + kPlaceFound, kFoundNothing);
        put(app, place + kPlaceAcross, 0xffffffff);
        copyWords(app, point, place + kPlacePoint, 3);
        return followToNearest(app, point, place);
    }
    // The quad's sides in turn, corner by corner: +0x30 to +0x3c, +0x24 to +0x30, +0x3c to +0x48,
    // +0x48 to +0x24.
    static constexpr u32 kSides[4][2] = {{0x30, 0x3c}, {0x24, 0x30}, {0x3c, 0x48}, {0x48, 0x24}};
    bool inside = at(app, place + kPlaceStrip) != 0;
    for (const auto& corners : kSides)
        if (inside)
            inside = sideOf(app, place, point, corners[0], corners[1]).inside;
    if (inside)
    {
        putHalf(app, place + kPlaceNodeChanged, 0);
        putHalf(app, place + kPlaceStripChanged, 0);
        return 0;
    }
    const std::optional<u32> located = locatePlace(app, ts, point, place);
    if (!located)
        return std::nullopt;
    copyWords(app, point, place + kPlacePoint, 3);
    return located;
}

std::optional<u32> bringPlace(App* app, ThreadState& ts, u32 point, u32 place, u32 followed)
{
    return bringPlaceEax(app, ts, point, place, followed);
}

/* 0x41f6e0: bringPlaceEax for the point eax and the place edx, followed ebx. */
nfs::u32 updatePosition(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, place = args.edx, followed = args.ebx] {
        return bringPlaceEax(app, ts, point, place, followed);
    });
    return args.eax;
}

// findMatchingQuad's ways: a node on, a node back, the strip on, the strip back.
constexpr u32 kMatchNodeOn = 4, kMatchNodeBack = 8, kMatchStripOn = 2, kMatchStripBack = 1;

/* The up's y where the place on the track is: its normal's second word doubled, or its node's byte
 * at 0x11 << 9 without a strip. */
u32 upHeight(App* app, u32 place)
{
    if (at(app, place + kPlaceStrip) == 0)
        return nodeAxis(app, at(app, kNodes) + at(app, place + kPlaceNode) * kNodeSize, kNodeUp, 1);
    return stripWord(app, at(app, place + kPlaceNormal), 1) * 2;
}

/* Whether corner `a` (a vertex, three floats) is within 0.2 of corner `b` along x, y and z: each
 * difference's size a float, as the code stores it, below the double (a NaN within). */
bool cornersNear(App* app, u32 a, u32 b)
{
    for (u32 k = 0; k < 12; k += 4)
    {
        const float size = sizeOf(float(fp::sub(f32(app, a + k), f32(app, b + k))));
        if ((fp::compare(Float(size), f64(app, kNear)) & kBelow) == 0)
            return false;
    }
    return true;
}

/* 0x41f870's: the place on the track `from` copied to `to` and moved by `way`: a node on or back,
 * then every strip across the road whose quad has two or more corners within 0.2 of from's
 * (cornersNear); the strip on, or the strip back (its quad stepped as forward, as the code does).
 * 1 when found on a polygon with a height (upHeight), else 0.  What the code leaves in edx: the
 * polygon's byte or the nodes (a node on or back), the polygon's byte or the node's offset (the
 * strip on), the normal or the node's offset (back), else what it last worked out -- the strip
 * across, the polygon, the strip's offset -- or `from`. */
Matched matchQuad(App* app, u32 way, u32 from, u32 to)
{
    copyWords(app, from, to, kPlaceSize / 4);
    const auto nodeOffset = [&] { return at(app, to + kPlaceNode) * kNodeSize; };
    if (way == kMatchNodeOn || way == kMatchNodeBack)
    {
        walkPlace(app, to, way == kMatchNodeOn, 1, 0);
        takeNodeEntry(app, to);
        put(app, to + kPlaceAcross, 0);
        while (s(stripsAcross(app, to) - 1) >= s(at(app, to + kPlaceAcross)))
        {
            takeStripAcross(app, to);
            if (onRoadPolygon(app, to))
            {
                u32 near = 0;
                for (u32 j = 0; j < 4; ++j)
                    for (u32 i = 0; i < 4; ++i)
                        if (to + kPlaceCorners != 0 && from + kPlaceCorners != 0
                            && at(app, to + kPlaceCorners + 4 * i) != 0 && at(app, from + kPlaceCorners + 4 * j) != 0
                            && cornersNear(app, at(app, to + kPlaceCorners + 4 * i), at(app, from + kPlaceCorners + 4 * j)))
                            ++near;
                if (s(near) >= 2)
                {
                    u32 scratch = takeFoundNormal(app, to);
                    if (at(app, to + kPlaceStrip) == 0)
                        scratch = at(app, kNodes);
                    return {upHeight(app, to) != 0 ? 1u : 0u, scratch};
                }
            }
            put(app, to + kPlaceAcross, at(app, to + kPlaceAcross) + 1);
        }
        return {0, from};
    }
    if (way == kMatchStripOn)
    {
        takeNodeEntry(app, to);
        takeStripAcross(app, to);
        const u32 across = at(app, to + kPlaceAcross);
        if (s(stripsAcross(app, to) - 1) <= s(across))
            return {0, across};
        put(app, to + kPlaceStrip, at(app, to + kPlaceStrip) + kStripSize);
        put(app, to + kPlaceAcross, at(app, to + kPlaceAcross) + 1);
        stepQuadForward(app, to);
        if (!onRoadPolygon(app, to))
            return {0, at(app, to + kPlacePolygon)};
        u32 scratch = takeFoundNormal(app, to);
        if (at(app, to + kPlaceStrip) == 0)
            scratch = nodeOffset();
        return {upHeight(app, to) != 0 ? 1u : 0u, scratch};
    }
    if (way == kMatchStripBack)
    {
        takeNodeEntry(app, to);
        u32 scratch = stripIndex(app, to) * kStripSize;
        put(app, to + kPlaceStrip, at(app, at(app, to + kPlaceBlock) + kBlockStrips) + scratch);
        takeQuadVertices(app, to);
        if (s(at(app, to + kPlaceAcross)) <= 0)
            return {0, scratch};
        scratch = at(app, to + kPlaceStrip) - kStripSize;
        const u32 across = at(app, to + kPlaceAcross) - 1;
        put(app, to + kPlaceStrip, scratch);
        put(app, to + kPlaceAcross, across);
        stepQuadForward(app, to);
        if (!onRoadPolygon(app, to))
            return {0, scratch};
        takeFoundNormal(app, to);
        scratch = at(app, to + kPlaceNormal);
        if (at(app, to + kPlaceStrip) == 0)
            scratch = nodeOffset();
        return {upHeight(app, to) != 0 ? 1u : 0u, scratch};
    }
    return {0, from};
}

/* 0x41f870: matchQuad by eax from the place edx to ebx. */
nfs::u32 findMatchingQuad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, way = args.eax, from = args.edx, to = args.ebx] { return matchQuad(app, way, from, to).found; });
    return args.eax;
}

// How far past a side the point may be: -1.5 in 16.16.
constexpr u32 kSideSlack = 0xfffe8000;
// quadEdgeFlags's bits: the two sides, the two ends.
constexpr u32 kPastFirstSide = 1, kPastSecondSide = 2, kPastFirstEnd = 4, kPastSecondEnd = 8;

/* 0x41fd60's: which edges of `other`'s quad (+0x24 on, 16.16) the point is past, for the place on
 * the track `place`: their polygons the same, only their ends' (+0x6c of each, or'd); else also
 * bits 1 and 2 the two sides (within -1.5) and 4 and 8 the ends -- (a.x - b.x) * (z - b.z) less
 * (a.z - b.z) * (x - b.x), fixedProducts -- and to `height` the point's height under the place's
 * quad's middle (its four corners' y's quarter less the point's). */
Edges edgesPast(App* app, u32 point, u32 other, u32 place, u32 height)
{
    u32 flags = at(app, place + kPlaceEnd) | at(app, other + kPlaceEnd);
    if (at(app, place + kPlacePolygon) == at(app, other + kPlacePolygon))
        return {flags, at(app, place + kPlacePolygon)};
    const u32 quad = other + kPlaceFixed;
    const auto past = [&](u32 a, u32 b) {
        const u32 first = fixedProduct(at(app, quad + a) - at(app, quad + b), at(app, point + 8) - at(app, quad + b + 8));
        const u32 second =
            fixedProduct(at(app, point) - at(app, quad + b), at(app, quad + a + 8) - at(app, quad + b + 8));
        return first - second;
    };
    if (s(past(0xc, 0x18)) >= s(kSideSlack))
        flags |= kPastFirstSide;
    if (s(past(0x24, 0)) >= s(kSideSlack))
        flags |= kPastSecondSide;
    if (s(past(0, 0xc)) >= 0)
        flags |= kPastFirstEnd;
    if (s(past(0x18, 0x24)) >= 0)
        flags |= kPastSecondEnd;
    const u32 corners = place + kPlaceFixed;
    const u32 firstTwo = at(app, corners + 4) + at(app, corners + 0x10);
    const u32 sum = at(app, corners + 0x28) + (at(app, corners + 0x1c) + firstTwo);
    put(app, height, u32(s(sum) >> 2) - at(app, point + 4));
    return {flags, firstTwo};
}

/* 0x41fd60: edgesPast of the point eax by the place edx's quad, for the place ebx, the height to
 * [ecx]; the flags in eax. */
nfs::u32 quadEdgeFlags(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, point = args.eax, other = args.edx, place = args.ebx, height = args.ecx] {
        return edgesPast(app, point, other, place, height).flags;
    });
    return args.eax;
}

/* 0x41ff10's: the items of the place on the track's block. */
u32 itemCount(App* app, u32 place)
{
    return at(app, at(app, place + kPlaceBlock) + kBlockItemCount);
}

/* 0x41ff10: itemCount for the place eax. */
nfs::u32 sectionItemCount(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { return itemCount(app, place); });
    return args.eax;
}

// An item's size (a word at 0xc) and its kind (0x13's low nibble): halved, doubled.
constexpr u32 kItemSizeWord = 0xc, kItemKind = 0x13;
constexpr u8 kItemHalved = 1, kItemDoubled = 2;

/* 0x41ff20's: the item `index` of the place on the track's block (20 bytes each): its first 12
 * bytes to `to`, its size's word to `size` -- halved where its kind is 1, doubled where it is 2. */
ItemTaken takeItem(App* app, u32 place, u32 index, u32 to, u32 size)
{
    const u32 block = at(app, place + kPlaceBlock);
    if (s(index) >= s(at(app, block + kBlockItemCount)))
        return {0xffffffff, 0, 0};
    const u32 items = at(app, block + kBlockItems);
    const u32 item = items + index * kItemSize;
    for (u32 k = 0; k < 12; k += 4)
        put(app, to + k, at(app, item + k));
    u16 word = half(app, item + kItemSizeWord);
    putHalf(app, size, word);
    if ((byte(app, item + kItemKind) & 0xf) == kItemHalved)
    {
        word = u16(nfs::s16(word) >> 1);
        putHalf(app, size, word);
    }
    if ((byte(app, item + kItemKind) & 0xf) == kItemDoubled)
        putHalf(app, size, u16(half(app, size) * 2));
    return {0, word, items};
}

/* 0x41ff20: takeItem edx of the place eax, to ebx and [ecx]. */
nfs::u32 sectionItem(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax, index = args.edx, to = args.ebx, size = args.ecx] {
        return takeItem(app, place, index, to, size).result;
    });
    return args.eax;
}

/* 0x41ffa0's: the lanes' span and shift, for lanePosition. */
void setLanes(App* app, u32 span, u32 shift)
{
    put(app, kLaneSpan, span);
    put(app, kLaneShift, shift);
}

/* 0x41ffa0: setLanes, the span edx and the shift eax. */
nfs::u32 setLaneOffsets(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, shift = args.eax, span = args.edx] { setLanes(app, span, shift); });
    return args.eax;
}

/* What 0x41ffd0 leaves: the lane's node, and the shift's step along the node's side vector's y,
 * which the code leaves in edx. */
struct InLane
{
    u32 node;
    u32 shiftY;
};

/* 0x41ffd0's: a place in a lane, to `out` (16.16), from the place on the track: the node half the
 * lanes' span (0x5dd930, halved towards 0) on from its node, then back to a multiple of the span
 * (the code's signed remainder), round the track's ends; that node's place moved out to its side
 * by its width -- right (the nibble at 0x13 by the byte at 0x1b << 15, along the vector at 0x18)
 * or left (the nibble at 0x13's top by the byte at 0x17) -- in 16.16 products, and along its
 * vector at 0x10 by the shift (0x5dd934), plain products. */
InLane lanePlace(App* app, u32 place, u32 out, u32 right)
{
    const u32 span = at(app, kLaneSpan), count = at(app, kNodeCount);
    const u32 half = u32(s(span - u32(s(span) >> 31)) >> 1);
    u32 ahead = half + at(app, place + kPlaceNode);
    if (s(half) >= 0)
    {
        if (s(ahead) >= s(at(app, kNodeCount)))
            ahead -= at(app, kNodeCount);
    }
    else if (s(ahead) < 0)
        ahead += at(app, kNodeCount);
    const u32 back = remainderOf(0u - ahead, span);
    u32 lane = ahead + back;
    if (s(back) >= 0)
    {
        if (s(lane) >= s(count))
            lane -= count;
    }
    else if (s(lane) < 0)
        lane += at(app, kNodeCount);
    const auto node = [&] { return at(app, kNodes) + lane * kNodeSize; };
    u32 position[3];
    for (u32 k = 0; k < 3; ++k)
        position[k] = at(app, node() + 4 * k);
    for (u32 k = 0; k < 3; ++k)
    {
        const u32 axis = nodeAxis(app, node(), kNodeSide, k);
        const u8 widths = byte(app, node() + kNodeWidths);
        if (right != 0)
            position[k] += fixedProduct((widths & 0xf) * (u32(byte(app, node() + kNodeRightWidth)) << 15), axis);
        else
            position[k] -= fixedProduct((u32(byte(app, node() + kNodeLeftWidth)) << 15) * (widths >> 4), axis);
    }
    const u32 shift = at(app, kLaneShift);
    u32 step[3];
    for (u32 k = 0; k < 3; ++k)
        step[k] = nodeAxis(app, node(), kNodeUp, k) * shift;
    for (u32 k = 0; k < 3; ++k)
        put(app, out + 4 * k, position[k] + step[k]);
    return {lane, step[1]};
}

/* 0x41ffd0: lanePlace of the place eax to edx, right ebx; the node in eax. */
nfs::u32 lanePosition(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax, out = args.edx, right = args.ebx] { return lanePlace(app, place, out, right).node; });
    return args.eax;
}

/* 0x420230's: the frame of the place on the track, to `axes` (nine words: the side, the up and the
 * ahead vectors, the side the up's cross the ahead's in 16.16 products) -- on a strip and on the
 * road (not off an end): the up and the ahead its normal's words doubled where it was found on
 * the quad (+0x64 2), else the node's bytes << 9; off the strip all nine the node's -- and its
 * quad's first corner (or the node's place, off the strip) copied to `corner`.  What the code
 * leaves in edx: the last product's high word, or the node's offset off the strip. */
u32 takeFrame(App* app, u32 place, u32 axes, u32 corner)
{
    const u32 found = at(app, place + kPlaceFound) == kFoundQuad ? 1 : 0;
    const auto node = [&] { return at(app, kNodes) + at(app, place + kPlaceNode) * kNodeSize; };
    u32 from, left;
    if (at(app, place + kPlaceStrip) != 0 && at(app, place + kPlaceEnd) == 0)
    {
        // The up (+0xc) and the ahead (+0x18): the normal's six words, or the node's bytes.
        static constexpr u32 kNodeBytes[6] = {kNodeUp, kNodeUp + 1, kNodeUp + 2, kNodeAhead, kNodeAhead + 1,
                                                kNodeAhead + 2};
        for (u32 k = 0; k < 6; ++k)
        {
            const bool normal = found != 0 && at(app, place + kPlaceStrip) != 0;
            const u32 word = normal ? stripWord(app, at(app, place + kPlaceNormal), k) * 2
                                      : signedByte(app, node() + kNodeBytes[k]) << 9;
            put(app, axes + 0xc + 4 * k, word);
        }
        const auto axis = [&](u32 k) { return at(app, axes + 4 * k); };
        put(app, axes, fixedProduct(axis(4), axis(8)) - fixedProduct(axis(5), axis(7)));
        put(app, axes + 4, fixedProduct(axis(5), axis(6)) - fixedProduct(axis(3), axis(8)));
        put(app, axes + 8, fixedProduct(axis(3), axis(7)) - fixedProduct(axis(4), axis(6)));
        left = fixedHigh(axis(4), axis(6));
        from = place + kPlaceFixed;
    }
    else
    {
        // The side, the up and the ahead: the node's bytes at 0x18, 0x10 and 0x14 on.
        static constexpr u32 kNodeBytes[9] = {kNodeSide, kNodeSide + 1, kNodeSide + 2, kNodeUp, kNodeUp + 1,
                                                kNodeUp + 2, kNodeAhead, kNodeAhead + 1, kNodeAhead + 2};
        const u32 nodes = at(app, kNodes);
        for (u32 k = 0; k < 9; ++k)
            put(app, axes + 4 * k,
                signedByte(app, nodes + at(app, place + kPlaceNode) * kNodeSize + kNodeBytes[k]) << 9);
        left = at(app, place + kPlaceNode) * kNodeSize;
        from = nodes + left;
    }
    u32 kept[3];
    for (u32 k = 0; k < 3; ++k)
        kept[k] = at(app, from + 4 * k);
    for (u32 k = 0; k < 3; ++k)
        put(app, corner + 4 * k, kept[k]);
    return left;
}

/* 0x420230: takeFrame of the place eax to edx and esi; esi in eax, and esi past its frame's copy
 * of the corner (the frame's -0x18), as the code leaves it. */
nfs::u32 positionFrame(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 corner = args.esi, frameEnd = ts.sp - 0x10 - 0x18;
    if (asThunk(ts, args, [&, place = args.eax, axes = args.edx] {
            takeFrame(app, place, axes, corner);
            return corner;
        }))
        args.esi = frameEnd;
    return args.eax;
}

namespace
{

/* 0x4e070c (fillZero): `bytes` at `to` zeroed; false when the game stopped. */
bool fillZero(App* app, ThreadState& ts, u32 to, u32 bytes)
{
    guest::direct<kFillZero, ea::clearBytes>(app, ts, to, bytes);
    return !ts.terminate;
}

/* 0x4969c0 (stdcall, six words: the corner's three, the point's three, the corner's first lowest):
 * the height of the surface at the point by the frame's axes at `axes` (eax), every other register
 * kept; nullopt when the game stopped. */
std::optional<u32> surfaceHeight(App* app, ThreadState& ts, u32 axes, u32 corner, u32 point)
{
    // The six words pushed below esp (the first lowest), the return slot under them by the call.
    const u32 esp = ts.sp;
    u32 slot = esp - 6 * 4;
    ts.sp = slot;
    for (const u32 word : {at(app, corner), at(app, corner + 4), at(app, corner + 8), at(app, point),
                             at(app, point + 4), at(app, point + 8)})
    {
        put(app, slot, word);
        slot += 4;
    }
    const u32 height = guest::direct<0x4969c0, frontend::simd::trackPointOnPlane>(app, ts, axes, esp - 6 * 4);
    ts.sp = esp;
    if (ts.terminate)
        return std::nullopt;
    return height;
}

}  // namespace

/* 0x420650's: the point (16.16) put on the track near node `node`: a place on the track made
 * afresh in the frame (fillZero, resetPlace, the point at its +4), brought there (bringPlace,
 * followed or not), its frame taken (takeFrame: its axes and a corner), and the height of the
 * surface there (0x4969c0) to the point's y; the point copied to the place's +4 again and the
 * place (0x78 bytes) to `out`.  The frame as deep as the code's, for what the calls are handed.
 * What the code leaves: eax `out`, ebx the point's old y and edx the corner's x (the registers it
 * pushed those through), esi past the place.  Nullopt when the game stopped. */
struct Placed
{
    u32 ebx, edx, esi;
};

std::optional<Placed> placeAtLeft(App* app, ThreadState& ts, u32 point, u32 followed, u32 node, u32 out)
{
    const GuestFrame frame(ts, 0xc, 0xb0);
    const u32 place = frame - 0xb0, axes = frame - 0x38, corner = frame - 0x14;
    if (!fillZero(app, ts, place, kPlaceSize))
        return std::nullopt;
    resetPlace(app, node, place);
    copyWords(app, point, place + kPlacePoint, 3);
    if (!bringPlace(app, ts, point, place, followed))
        return std::nullopt;
    takeFrame(app, place, axes, corner);
    const Placed left{at(app, point + 4), at(app, corner), place + kPlaceSize};
    const std::optional<u32> height = surfaceHeight(app, ts, axes, corner, point);
    if (!height)
        return std::nullopt;
    put(app, point + 4, *height);
    copyWords(app, point, place + kPlacePoint, 3);
    copyWords(app, place, out, kPlaceSize / 4);
    return left;
}

bool placeAt(App* app, ThreadState& ts, u32 point, u32 followed, u32 node, u32 out)
{
    return placeAtLeft(app, ts, point, followed, node, out).has_value();
}

/* 0x420650: placeAtLeft for the point eax near node ebx, followed edx, to esi: eax `out`, esi past
 * the frame's place, as the code leaves them. */
nfs::u32 placeOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<Placed> left;
    asThunk(ts, args, [&, point = args.eax, followed = args.edx, node = args.ebx, out = args.esi]() -> std::optional<u32> {
        left = placeAtLeft(app, ts, point, followed, node, out);
        if (!left)
            return std::nullopt;
        return out;
    });
    if (left)
        args.esi = left->esi;
    return args.eax;
}

/* 0x4205a0's: the place (three floats) snapped to the road: made 16.16 (* 65536, fistp's
 * rounding), put on the track (placeAt, into the frame), and its place there written back as
 * floats; the place on the track (0x78 bytes) copied to `out`.  The frame as deep as the code's.
 * What the code leaves: eax `out`, ebx and edx as placeAt leaves them, esi past the frame's place
 * on the track.  Nullopt when the game stopped. */
std::optional<Placed> snapPlaceLeft(App* app, ThreadState& ts, u32 place, u32 out, u32 followed, u32 node)
{
    const GuestFrame frame(ts, 0xc, 0x8c);
    const u32 fixed = frame - 0x14, found = frame - 0x8c;
    for (u32 k = 0; k < 12; k += 4)
        put(app, fixed + k, u32(fp::toInteger<s32>(fp::mul(f32(app, place + k), f64(app, kFixedToo)))));
    std::optional<Placed> placed;
    {
        const GuestFrame slot(ts, 0, 4);
        if (!(placed = placeAtLeft(app, ts, fixed, followed, node, found)))
            return std::nullopt;
    }
    const Float unfixed = f64(app, kUnfixedThree);
    putF32(app, place, fp::mul(i32(app, fixed), unfixed));
    putF32(app, place + 4, fp::mul(i32(app, fixed + 4), unfixed));
    putF32(app, place + 8, fp::mul(unfixed, i32(app, fixed + 8)));
    copyWords(app, found, out, kPlaceSize / 4);
    return Placed{placed->ebx, placed->edx, found + kPlaceSize};
}

bool snapPlace(App* app, ThreadState& ts, u32 place, u32 out, u32 followed, u32 node)
{
    return snapPlaceLeft(app, ts, place, out, followed, node).has_value();
}

/* 0x4205a0: snapPlaceLeft of the place eax to esi, followed edx, near node ebx: eax `out`, and
 * ebx, edx and esi as the code leaves them (ebx and edx no arguments of its by its code's
 * reading). */
nfs::u32 snapToTrack(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<Placed> left;
    asThunk(ts, args, [&, place = args.eax, out = args.esi, followed = args.edx, node = args.ebx]() -> std::optional<u32> {
        left = snapPlaceLeft(app, ts, place, out, followed, node);
        if (!left)
            return std::nullopt;
        return out;
    });
    if (!left)
        return args.eax;
    args.ebx = left->ebx;
    args.edx = left->edx;
    args.esi = left->esi;
    return args.eax;
}

// The light where there is no quad: grey, opaque.
constexpr u32 kNoColour = 0xff7f7f7f;

/* 0x4206f0's: the light where the place on the track is (followed, and on a strip): its quad's
 * four vertices' colours (its block's +0x74, by the strip's words) averaged byte by byte, the
 * sums shifted and masked as the code does, opaque; else grey. */
u32 placeColour(App* app, u32 place, u32 followed)
{
    if (followed == 0 || at(app, place + kPlaceStrip) == 0)
        return kNoColour;
    const u32 colours = at(app, at(app, place + kPlaceBlock) + kBlockColours);
    const u32 strip = at(app, place + kPlaceStrip);
    u32 corner[4];
    for (u32 k = 0; k < 4; ++k)
        corner[k] = at(app, colours + stripWord(app, strip, k) * 4);
    const auto average = [&](u32 mask) {
        const u32 sum = (corner[0] & mask) + (corner[1] & mask) + (corner[2] & mask) + (corner[3] & mask);
        return u32(s(sum) >> 2) & mask;
    };
    return 0xff000000 | average(0xff0000) | average(0xff00) | average(0xff);
}

/* 0x4206f0: placeColour of the place eax, followed edx. */
nfs::u32 positionColour(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax, followed = args.edx] { return placeColour(app, place, followed); });
    return args.eax;
}

// What the surface is taken as without a polygon.
constexpr u32 kNoSurface = 1;

/* 0x4207e0's: the kind of the polygon the place on the track is on, or 1 without one; followed
 * and on no strip, +0x74 -1 and 1. */
u32 placeSurface(App* app, u32 place, u32 followed)
{
    if (followed != 0 && at(app, place + kPlaceStrip) == 0)
    {
        put(app, place + kPlaceFollowed, 0xffffffff);
        return kNoSurface;
    }
    const u32 polygon = at(app, place + kPlacePolygon);
    return polygon != 0 ? byte(app, polygon + kPolygonKind) : kNoSurface;
}

/* 0x4207e0: placeSurface of the place eax, followed edx. */
nfs::u32 positionSurface(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax, followed = args.edx] { return placeSurface(app, place, followed); });
    return args.eax;
}

// The special grounds groundUnder finds.
constexpr u32 kGroundNone = 0, kGroundFirst = 0x10, kGroundSecond = 0x11, kGroundThird = 0x12;

/* 0x420840's: what is under a car at node `node` across lane `lane`, by the track ([0x6fd4ac], the
 * targets of nfs3.exe's table at 0x420814): the nodes of each track's special ground -- 0x10, 0x11
 * or 0x12 -- or 0. */
u32 groundUnder(App* app, s32 node, s32 lane)
{
    const u32 track = gameSettings(app).track;
    // node in (from, to), as the code's cmp eax, from; jle; cmp eax, to; jl.
    const auto in = [&](s32 from, s32 to) { return node > from && node < to; };
    switch (track <= 8 ? track : 1)
    {
    case 0:  // 0x42085d
    {
        if (in(0x25a, 0x260))
            return kGroundFirst;
        bool found = false;
        if (lane == 2)
            found = in(0x21d, 0x227);
        else if (lane == 3)
            found = in(0x215, 0x21e) || in(0x234, 0x238);
        else if (lane == 5)
        {
            if (in(0x22d, 0x235))
                return kGroundSecond;
        }
        else if (lane == 6)
            found = in(0x22d, 0x235);
        else if (lane == 7)
            found = in(0x1f1, 0x1f8) || in(0x212, 0x216);
        else if (lane == 8)
            found = in(0x1f7, 0x209) || in(0x20f, 0x213);
        if (!found && lane <= 4)
        {
            found = in(0x1f2, 0x209) || in(0x20f, 0x216) || in(0x237, 0x240);
            if (!found && lane <= 3)
            {
                found = in(0x212, 0x22c);
                if (!found && lane <= 2)
                    found = in(0x208, 0x210);
            }
        }
        if (!found && lane >= 5)
        {
            found = in(0x21d, 0x227);
            if (!found && lane >= 6)
            {
                found = in(0x208, 0x210) || in(0x215, 0x21e) || in(0x226, 0x22c) || in(0x234, 0x238);
                if (!found && lane >= 7)
                {
                    found = in(0x22b, 0x22e) || in(0x237, 0x240);
                    if (!found && lane >= 9)
                        found = in(0x22d, 0x235);
                }
            }
        }
        return found ? kGroundThird : kGroundNone;
    }
    case 2:  // 0x420a83
        if (lane == 4 && (in(0x3cd, 0x3d7) || in(0x3b7, 0x3c1) || in(0x40b, 0x417)))
            return kGroundSecond;
        return kGroundNone;
    case 3:  // 0x420acc
        if (lane == 3)
            return node == 0x557 || node == 0x556 || node == 0x553 || node == 0x554 ? kGroundSecond : kGroundNone;
        return in(0x41a, 0x42c) ? kGroundThird : kGroundNone;
    case 6:  // 0x420b16
        if (lane == 4 && (in(0x424, 0x42e) || in(0x43a, 0x444) || in(0x478, 0x484)))
            return kGroundSecond;
        return kGroundNone;
    case 7:  // 0x420b5c
        return lane == 3 && (node == 0x643 || node == 0x644) ? kGroundSecond : kGroundNone;
    case 8:  // 0x420b80
        if (node > 0x3c3 || node < 0xd6)
            return kGroundFirst;
        if (lane > 7 && lane < 0xb && in(0x305, 0x35b))
            return kGroundThird;
        if (lane == 2 && in(0x2a4, 0x2e1))
            return kGroundSecond;
        if ((lane == 9 || lane == 0xb) && in(0x364, 0x37d))
            return kGroundSecond;
        return kGroundNone;
    default:  // 1, 4, 5 (0x420bde) and past the table
        return kGroundNone;
    }
}

/* 0x420840: groundUnder at node eax across lane edx. */
nfs::u32 surfaceUnder(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = s(args.eax), lane = s(args.edx)] { return groundUnder(app, node, lane); });
    return args.eax;
}

namespace
{

/* The node's tunnel kind from its nibbles: the low one asked first (else the high one) or the
 * other way round; 1 for a tunnel, else 0. */
u32 tunnelBy(u8 nibbles, bool lowFirst)
{
    const u8 low = nibbles & 0xf, high = u8(nibbles >> 4) & 0xf;
    const u8 asked = lowFirst ? (low != 0 ? low : high) : (high != 0 ? high : low);
    return asked == kTunnel ? 1 : 0;
}

/* vertices::subtractVectors (0x4e0000): `count` vectors at `from` less those at `less`, to `to`.
 * False when the game stopped. */
bool subtractVectors(App* app, ThreadState& ts, u32 count, u32 from, u32 less, u32 to)
{
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, nfs::s32(count), from, less, to);
    return !ts.terminate;
}

/* vertices::dotPush (0x4e01f0): the dot product of the vectors at a and b, as the x87 holds it;
 * nullopt when the game stopped. */
std::optional<Float> dotOf(App* app, ThreadState& ts, u32 a, u32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, ts, a, b);
    if (ts.terminate)
        return std::nullopt;
    return Float(product);
}

/* carstate::cameraNode (0x4217e0): car `index`'s node; nullopt when the game stopped. */
std::optional<u32> carNode(App* app, ThreadState& ts, u32 index)
{
    const u32 node = guest::direct<0x4217e0, carstate::nodeOfCamera>(app, ts, index);
    if (ts.terminate)
        return std::nullopt;
    return node;
}

}  // namespace

/* 0x420bf0's: whether the place (three words at `place`, 16.16, as its caller hands them) is in a
 * tunnel by node `node`: the place less the node's (its x, y and z made floats, 0x4e0000), dotted
 * with its side vector (0x4e01f0), picks the side -- 0 below the product (or a NaN) the low nibble
 * is asked first.  The frame as deep as the code's, for the vectors the calls are handed.  1 or 0;
 * nullopt when the game stopped. */
std::optional<u32> placeInTunnel(App* app, ThreadState& ts, u32 place, u32 node)
{
    const GuestFrame frame(ts, 0x14, 0x28);
    const u32 side = frame - 0x28, nodePlace = frame - 0x1c, difference = frame - 0x10;
    const u32 offset = node * kNodeSize;
    const Float unfixed = f64(app, kUnfixedFour);
    for (u32 k = 0; k < 12; k += 4)
        putF32(app, nodePlace + k, fp::mul(i32(app, at(app, kNodes) + offset + k), unfixed));
    if (!subtractVectors(app, ts, 1, place, nodePlace, difference))
        return std::nullopt;
    for (u32 k = 0; k < 3; ++k)
        putF32(app, side + 4 * k, fp::mul(Float(s(nodeAxis(app, at(app, kNodes) + offset, kNodeSide, k))), unfixed));
    const std::optional<Float> dot = dotOf(app, ts, side, difference);
    if (!dot)
        return std::nullopt;
    const bool rightOf = (fp::compare(0.0, *dot) & kBelow) != 0;
    return tunnelBy(byte(app, at(app, kNodes) + offset + kNodeTunnel), rightOf);
}

/* 0x420bf0 (stdcall, four words: the place's x, y and z, 16.16, its node): placeInTunnel. */
nfs::u32 inTunnel(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args, [&, place = ts.sp + 4, node = word(app, ts, 4)] { return placeInTunnel(app, ts, place, node); }, 0x10);
    return args.eax;
}

// A car's index, its place and heading (floats), and how far round the node to look.
constexpr u32 kCarIndex = 4, kCarPlace = 8, kCarHeading = 0x2c;
constexpr u32 kLookNodes = 2;

/* 0x420d60's: whether the car is in a tunnel two nodes on (or back) from its node (0x4217e0): its
 * place and heading (floats) made 16.16 (each * 65536, fistp's rounding); the place less its
 * node's, dotted with the node's side vector in 16.16 products, picks which nibble is asked first
 * (the low one where it is above 0); the heading along the node's ahead vector (the track
 * reversed at 0x6fd4c0 turning it) picks two on or two back.  The code keeps a flag at 0 that
 * would look back too: never set, it is left out.  1 or 0; nullopt when the game stopped. */
std::optional<u32> carInTunnel(App* app, ThreadState& ts, u32 car)
{
    const std::optional<u32> found = carNode(app, ts, at(app, car + kCarIndex));
    if (!found)
        return std::nullopt;
    const u32 node = *found;
    const auto fixed = [&](u32 field) {
        return u32(fp::toInteger<s32>(fp::mul(f32(app, car + field), f64(app, kFixedThree))));
    };
    u32 place[3], heading[3];
    for (u32 k = 0; k < 3; ++k)
        place[k] = fixed(kCarPlace + 4 * k);
    for (u32 k = 0; k < 3; ++k)
        heading[k] = fixed(kCarHeading + 4 * k);
    const u32 offset = node * kNodeSize;
    const auto nodeAt = [&] { return at(app, kNodes) + offset; };
    u32 difference[3];
    for (u32 k = 0; k < 3; ++k)
        difference[k] = place[k] - at(app, nodeAt() + 4 * k);
    u32 axis[3];
    for (u32 k = 0; k < 3; ++k)
        axis[k] = nodeAxis(app, nodeAt(), kNodeSide, k);
    const u32 across = fixedProduct(axis[0], difference[0]) + fixedProduct(axis[1], difference[1])
                        + fixedProduct(axis[2], difference[2]);
    u32 along = 0;
    for (u32 k = 0; k < 3; ++k)
        along += fixedProduct(heading[k], signedByte(app, nodeAt() + kNodeAhead + k));
    const bool reversed = gameSettings(app).backwards != 0;
    const bool back = (!reversed && s(along) < 0) || (reversed && s(along) > 0);
    u32 looked;
    if (!back)
    {
        looked = node + kLookNodes;
        if (s(looked) >= s(at(app, kNodeCount)))
            looked -= at(app, kNodeCount);
    }
    else
    {
        looked = node - kLookNodes;
        if (s(looked) < 0)
            looked += at(app, kNodeCount);
    }
    return tunnelBy(byte(app, at(app, kNodes) + looked * kNodeSize + kNodeTunnel), s(across) > 0);
}

/* 0x420d60: carInTunnel of the car eax. */
nfs::u32 carFacingWay(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return carInTunnel(app, ts, car); });
    return args.eax;
}

// A car's node (a dword) and whether it goes the track's way backwards.
constexpr u32 kCarTrackNode = 0x1c, kCarBackwards = 0x998;
// A node's entry's kind, the byte at 3.
constexpr u32 kEntryKind = 3;

/* 0x4211c0's: the kind of the track at the car's node -- the byte at 3 of its entry in its block.
 * The code works out the sum of the car's speeds' sizes (+0xa4, +0xac) * 65536, fistp's it to its
 * stack and drops it: the nodes ahead it counts are 0 (negated for a car going backwards, still 0),
 * so the node is the car's own, wrapped at the track's end.  Nothing of that sum is seen, so it is
 * not worked out here. */
u32 carTrackKind(App* app, u32 car)
{
    u32 node = at(app, car + kCarTrackNode);
    if (s(node) >= s(at(app, kNodeCount)))
        node -= at(app, kNodeCount);
    const u32 block = kBlocks + blockOffset(node);
    const u32 entries = at(app, block + kBlockEntries);
    return byte(app, entries + (node - at(app, block + kBlockFirstNode)) * kEntrySize + kEntryKind);
}

/* 0x4211c0: carTrackKind of the car eax. */
nfs::u32 carSectionKind(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, place = args.eax] { return carTrackKind(app, place); });
    return args.eax;
}

/* The typed cores the game's other code calls (native_trackdata.h), under the game's addresses. */
u32 laneNodePlaced(App* app, u32 place, u32 out, u32 right)
{
    return lanePlace(app, place, out, right).node;
}

std::optional<u32> carAgainstTrack(App* app, ThreadState& ts, u32 car)
{
    return carInTunnel(app, ts, car);
}

u32 trackKindUnderCar(App* app, u32 car)
{
    return carTrackKind(app, car);
}

}  // namespace trackdata
}  // namespace nfs3hp
