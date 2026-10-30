#include "port_view.h"
#include "port_text.h"

#include <cmath>
#include <cstdint>
#include <cstring>

/* The pan-and-zoom view, rewritten (port.h says how; port_view.h says what
 * it holds).  The arithmetic is double, rounded to a float where the game
 * stores a float -- no cpu.fpu.  Each function is a typed core and an adapter
 * under the game's name. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{
namespace port
{
namespace view
{

namespace
{

using x86::Float;

/* ---- The game's variables ---- */

constexpr Var<float> kPanBase{0x55a438};

// The view's pan: whether it is up, where it is panned to (floats), and the
// constants its keys use (a double: how much a zoom step adds; a float: how far
// the pan may go before it is put back to the edge).
constexpr Var<reg32> kPanActive{0x678950};
constexpr Var<float> kPanX{0x678954};
constexpr Var<float> kPanY{0x678958};
constexpr Var<double> kZoomStep{0x53ab6c};
constexpr Var<float> kPanLimit{0x53ab74};
constexpr Var<reg32> kMapX{0x55a43c}, kMapY{0x55a440};      // the map's offset, integers
constexpr Var<reg32> kDragging{0x67894c};                   // 1 while the mouse drags the map
constexpr Var<reg32> kDragMouseX{0x678948}, kDragMouseY{0x678944};
constexpr Var<reg32> kMouseX{0x55e5e4}, kMouseY{0x55e5e8};
constexpr reg32 kTranslucency = 0x7a3a58;                   // bit 1: no fades
constexpr reg32 kPanEdgeLow = 0xc1200000;   // -10.0f
constexpr reg32 kPanEdgeHigh = 0x41200000;  // 10.0f

// What the view's keys take: the arrows pan by 2.0, the space bar zooms.
constexpr reg32 kKeyLeft = 0x4b00, kKeyRight = 0x4d00, kKeySpace = 0x20;
constexpr reg32 kMenuShown = 0x44a660;
constexpr reg32 kMouseKey = 0x49c080;       // (object, 0): the mouse as a key, edx its second word
constexpr reg32 kPrepareView = 0x464eb0;    // (held, 0x21, ebx, ecx)
constexpr reg32 kSetRenderState = 0x431900;
constexpr reg32 kBoxFrame = 0x4d8930;       // (x, y, w, h) stdcall: the colour
constexpr reg32 kGrid = 0x464840;           // (x, y, w, h) stdcall: &mapX, &mapY, &scale
constexpr reg32 kEnter = 0xd;
constexpr reg32 kMenuView = 0x21;           // the item whose view this is

// The popup over the race's results: whether any is to be drawn, how many
// metrics an entry holds and where they are in it, and the fields of the
// record whose box it is (all words but the rows' and the first's, which are
// the high halves of dwords).
constexpr Var<reg32> kScreenKind{0x6fd3b8};          // 3: none to draw
constexpr Var<reg32> kMetricCount{0x6fd4b4};
constexpr reg32 kMetricValues = 0x298;               // an entry's floats
constexpr reg32 kEntryKind = 0x2c4;
constexpr reg32 kEntryNone = 4;
constexpr reg32 kBoxLeft = 0x8, kBoxPartA = 0x3c, kBoxPartB = 0x3e, kBoxPartC = 0x40, kBoxWidth = 0x42;
constexpr reg32 kBoxStep = 0x44, kBoxRows = 0x46, kBoxFirst = 0x84, kBoxTop = 0x8a;

// What it calls, by address (the language's text as C++, which the checks run
// on a world of its own lines).
constexpr reg32 kPointerInBox = 0x49c1f0;            // (&rect: four int16)
constexpr reg32 kEntryOf = 0x455cd0;                 // (0, index, 0, 0): the entry, or 0
constexpr reg32 kMetricText = 0x4cedd0;              // (value, &text), stdcall
constexpr reg32 kFieldBox = 0x4d8ac0;                // (x, y, w, h), stdcall: four colours
constexpr reg32 kDrawText = 0x452190;                // (text, x, y, font), stdcall: align, colour
constexpr reg32 kSprintf = 0x4df690;
constexpr guest::Native kLanguageText = text::languageText;      // 0x4d1850
constexpr reg32 kFormat = 0x53aeb4;                  // a metric's number
constexpr reg32 kFont = 0x12;
constexpr reg32 kBestColour = 0xffff0000, kOtherColour = 0xff007fff, kTitleColour = 0xffe440;
constexpr reg32 kTitleLine = 5;                      // the language's line for the title
constexpr reg32 kAlignTitle = 5, kAlignValue = 4, kAlignNumber = 3;

// A participant's record: its kind (a word) and the floats the HUD reads.
constexpr reg32 kKind = 0x42;
constexpr reg32 kFloats = 0x48;
constexpr reg32 kKindNone = 1;      // no float to give: 0.0
constexpr reg32 kKindIndexed = 2;   // the float of the index asked for

float bitsToFloat(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* ---- 0x4646e0, 0x469580, 0x46b030, 0x46ca70 ---- */

/* The pan's base: the float given, plus one, kept as a float. */
void storedIncremented(App* app, float value)
{
    kPanBase.set(app, float(double(value) + 1.0));
}

/* The record's first float. */
float firstFloat(App* app, reg32 record)
{
    return load<float>(app, record + kFloats);
}

/* The record's float number `index` (any 32-bit value: the address wraps). */
float floatAt(App* app, reg32 record, reg32 index)
{
    return load<float>(app, record + index * 4 + kFloats);
}

/* The float the record's kind gives: 0.0 for kind 1, the indexed one for kind
 * 2, the first for every other.  `edx` is what the game's code leaves in edx:
 * the index as it was where the indexed float was asked for or none was, 0
 * where the first was taken. */
float floatByKind(App* app, reg32 record, reg32 index, reg32& edx)
{
    const reg32 kind = app->getMemory<uint16_t>(record + kKind);
    edx = index;
    if (kind == kKindNone)
        return 0.0f;
    if (kind == kKindIndexed)
        return floatAt(app, record, index);
    edx = 0;
    return firstFloat(app, record);
}

/* ---- 0x4651e0 ---- */

/* What the view's key leaves in the registers it changes, and its result. */
struct KeyOutcome
{
    reg32 eax, ebx, edx;
};

/* The view's keys: the left and right arrows pan the view by 2.0, the space
 * bar zooms it by a step, in the direction its pan is on; whatever the pan
 * comes to is kept to a limit (past it, to the edge of -10.0 or 10.0).  Only
 * while the menu's item is shown and the view is up.  Gives 2 for a key it
 * took, else what the menu's call left in ecx. */
std::optional<KeyOutcome> keyPan(App* app, CPU& cpu, const Entry& e)
{
    reg32 shown;
    {
        const HeldEbp held(cpu, e.frame(2));
        if (!callGame(app, cpu, e.frame(2, 0x14), kMenuShown, {e.eax, kMenuView, e.ebx, 0}, {}, shown))
            return std::nullopt;
    }
    // What the call left: ebx is the key, ecx the pan's step in both axes (as bits).
    const reg32 key = cpu.ebx & 0xffff;
    reg32 ecx = cpu.ecx;
    reg32 edx = cpu.edx;
    KeyOutcome out{shown, cpu.ebx, edx};
    if (shown == 0)
        return out;
    if (kPanActive(app) == 0)
    {
        out.eax = 0;
        return out;
    }
    float dx = bitsToFloat(ecx);
    const float dy = bitsToFloat(ecx);
    if (key == kKeyLeft)
    {
        dx = 2.0f;
        ecx = 2;
    }
    if (key == kKeyRight)
    {
        dx = -2.0f;
        ecx = 2;
        edx = 0xc0000000;
    }
    if (key == kKeySpace)
    {
        ecx = 2;
        const double step = double(dx);
        if (0.0 > double(kPanX(app)))
            dx = float(step + step * kZoomStep(app));
        else
            dx = float(-step * kZoomStep(app) + step);
    }
    kPanX.set(app, float(double(kPanX(app)) + double(dx)));
    kPanY.set(app, float(double(kPanY(app)) + double(dy)));
    const float x = kPanX(app);
    const float magnitude = 0.0 >= double(x) ? -x : x;
    if (double(magnitude) > double(kPanLimit(app)))
    {
        const reg32 edge = 0.0 > double(kPanX(app)) ? kPanEdgeLow : kPanEdgeHigh;
        kPanX.set(app, bitsToFloat(edge));
    }
    out.eax = ecx;
    out.edx = edx;
    return out;
}

/* ---- 0x474db0 ---- */

constexpr reg32 kUnitVector = 0x4e01c0;              // (in, out): a vector normalized
constexpr reg32 kCrossProduct = 0x4e01a0;            // (b, a, out): the cross product, normalized

/* The mesh the large glyphs are made into: for a face (entry `face` of the
 * triangle list, three vertex numbers) and an edge of it (entries a and b of
 * the three), when the edge is still marked in the adjacency grid (width W: the
 * flag of the edge a-b at [a + W*b], and b-a at [b + W*a]), both marks are
 * cleared and two triangles are put on the list from its end -- the counter
 * counts down: the edge with the vertex d of the face `more + face`, then the
 * other way round, with c -- with the normal of each (the cross product of its
 * two edges from its first vertex, each normalized) put in the normals' list at
 * the same number.  The code's scratch is on the game's stack.  False when the
 * game stops in a call. */
bool edgeTrianglesOf(App* app, CPU& cpu, const Entry& e)
{
    const uint8_t a = uint8_t(e.eax), b = uint8_t(e.edx), c = uint8_t(e.ebx), d = uint8_t(e.ecx);
    const reg32 flags = e.arg(app, 0), triangles = e.arg(app, 1), vertices = e.arg(app, 2), normals = e.arg(app, 3);
    const reg32 width = e.arg(app, 4), more = e.arg(app, 5), face = e.arg(app, 6), counter = e.arg(app, 7);
    const reg32 ebp = e.frame(3);
    const reg32 base = e.frame(3, 0x38);
    const HeldEbp held(cpu, ebp);
    const auto local = [&](int32_t offset) { return ebp + reg32(offset); };
    const auto putInt = [&](int32_t offset, reg32 value) { app->getMemory<reg32>(local(offset)) = value; };
    const auto byteAt = [&](reg32 at) -> reg32 { return app->getMemory<uint8_t>(at); };
    const auto setByte = [&](reg32 at, reg32 value) { app->getMemory<uint8_t>(at) = uint8_t(value); };
    const auto vertex = [&](reg32 number) { return vertices + 12 * number; };

    app->getMemory<uint8_t>(local(-4)) = a;
    app->getMemory<uint8_t>(local(-0x10)) = b;
    app->getMemory<uint8_t>(local(-0xc)) = c;
    app->getMemory<uint8_t>(local(-8)) = d;
    const reg32 entry = triangles + 3 * face;
    const reg32 vertexB = byteAt(entry + b);
    putInt(-0x14, vertexB);
    putInt(-0x14, width * vertexB);
    const reg32 vertexAAt = entry + a;
    putInt(-0x18, vertexAAt);
    const reg32 vertexA = byteAt(vertexAAt);
    const reg32 mark = flags + vertexA + width * vertexB;
    if (app->getMemory<uint8_t>(mark) == 0)
        return true;
    setByte(mark, 0);
    setByte(flags + width * vertexA + vertexB, 0);

    // The vectors from a triangle's first vertex to its second and third, each normalized; the normal.
    const auto normalOf = [&](reg32 number) -> bool {
        const reg32 at = triangles + 3 * number;
        const reg32 first = vertex(byteAt(at)), second = vertex(byteAt(at + 1)), third = vertex(byteAt(at + 2));
        for (reg32 axis = 0; axis < 3; ++axis)
        {
            const float difference =
                float(double(load<float>(app, second + 4 * axis)) - double(load<float>(app, first + 4 * axis)));
            app->getMemory<float>(local(-0x2c) + 4 * axis) = difference;
        }
        for (reg32 axis = 0; axis < 3; ++axis)
        {
            const float difference =
                float(double(load<float>(app, third + 4 * axis)) - double(load<float>(app, first + 4 * axis)));
            app->getMemory<float>(local(-0x38) + 4 * axis) = difference;
        }
        if (!callGame(app, cpu, base, kUnitVector, {local(-0x2c), local(-0x2c), 0, 0}))
            return false;
        if (!callGame(app, cpu, base, kUnitVector, {local(-0x38), local(-0x38), 0, 0}))
            return false;
        return callGame(app, cpu, base, kCrossProduct,
                        {local(-0x38), local(-0x2c), normals + 12 * app->getMemory<reg32>(counter), 0});
    };

    reg32 number = app->getMemory<reg32>(counter);
    putInt(-0x14, number);
    setByte(triangles + 3 * number, byteAt(entry + b));
    setByte(triangles + 3 * number + 1, byteAt(vertexAAt));
    const reg32 otherFace = triangles + 3 * (more + face);
    putInt(-0x20, otherFace);
    const reg32 vertexDAt = otherFace + d;
    putInt(-0x1c, vertexDAt);
    setByte(triangles + 3 * number + 2, byteAt(vertexDAt));
    if (!normalOf(number))
        return false;
    // The second triangle, from the vertex c.
    number = app->getMemory<reg32>(counter) - 1;
    app->getMemory<reg32>(counter) = number;
    setByte(triangles + 3 * number, byteAt(otherFace + c));
    setByte(triangles + 3 * number + 1, byteAt(vertexDAt));
    setByte(triangles + 3 * number + 2, byteAt(vertexAAt));
    if (!normalOf(number))
        return false;
    app->getMemory<reg32>(counter) = app->getMemory<reg32>(counter) - 1;
    return true;
}

/* ---- 0x475240 ---- */

constexpr Var<reg32> kGlyphResource{0x679010};       // the resource last made or loaded
constexpr reg32 kGlyphArena = 0x53b064;              // the memory class it takes its blocks from
constexpr reg32 kGlyphCacheName = 0x53b060;          // the cache file's name's format
constexpr Var<float> kGlyphScale{0x53b068};
constexpr reg32 kGlyphHeader = 0x2010;               // the resource's header: 255 records of 32 bytes and its bounds
constexpr reg32 kFileExists = 0x4e0e60;              // (name): whether
constexpr reg32 kLoadFile = 0x4e0ec0;                // (name, flags): the file's bytes
constexpr reg32 kReserveMemory = 0x4e1620;           // (class, size, flags): a block
constexpr reg32 kPurgeMemory = 0x4e1890;             // (block)
constexpr reg32 kMemset = 0x4e0640;                  // (buffer, byte, count)
constexpr reg32 kSaveFile = 0x4eeae0;                // (name, bytes, size): the cache file written
constexpr reg32 kEmitEdges = 0x474db0;               // the edge pair emitter, stdcall 8 words

/* The large glyphs' geometry resource for the font named by `name`: from the
 * cache file the font's name makes when there is one (the header's records
 * given their pointers into the bytes after it), else made from the font's own
 * file -- the glyphs in it (each: its vertices as pairs of floats, its
 * triangles as three bytes each) turned into two layers of vertices, the
 * triangles of the front and the back, and between them the quads of the edges
 * that only one triangle has, found in an adjacency grid; the vertices' normals
 * summed over the triangles that share them and normalized -- all in blocks of
 * their own, then copied into one block, put in the cache file and given back.
 * The records' fields (at base + 32 * glyph): +0x10 the vertex count, +0x11 the
 * triangle count, +0x12 and +0x13 the half and the whole of the vertices'
 * layer, +0x14 to +0x1c the bounds, +0x20 the vertices, +0x24 the triangles'
 * normals, +0x28 the vertices' normals, +0x2c the triangles.  The code's
 * working is on the game's stack at the offsets it has them (it holds ebp 0x82
 * below its frame), and is kept there, in the order the code stores it.  Gives
 * the resource; nothing when the game stops in a call. */
std::optional<reg32> glyphResourceOf(App* app, CPU& cpu, const Entry& e)
{
    const reg32 name = e.eax;
    const reg32 frameTop = e.frame(5);
    const reg32 base = e.frame(5, 0xfc);
    const reg32 frame = frameTop - 0x82;                // the code's ebp
    const HeldEbp held(cpu, frame);
    const auto L = [&](int32_t offset) { return frame + reg32(offset); };
    const auto word = [&](int32_t offset) -> reg32 { return app->getMemory<reg32>(L(offset)); };
    const auto setWord = [&](int32_t offset, reg32 value) { app->getMemory<reg32>(L(offset)) = value; };
    const auto byteAt = [&](reg32 at) -> reg32 { return app->getMemory<uint8_t>(at); };
    const auto setByte = [&](reg32 at, reg32 value) { app->getMemory<uint8_t>(at) = uint8_t(value); };
    const auto at32 = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set32 = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto copy = [&](reg32 to, reg32 from, reg32 count) {
        for (reg32 i = 0; i < count; ++i)
            app->getMemory<uint8_t>(to + i) = app->getMemory<uint8_t>(from + i);
    };
    const auto bitsOf = [](float value) {
        reg32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        return bits;
    };
    // A call into the game's code, esp as the code has it there, ebp the biased frame.
    reg32 result = 0;
    const auto call = [&](reg32 target, Args a, std::initializer_list<reg32> stack, reg32 esp) {
        return callGame(app, cpu, esp, target, a, stack, result);
    };

    set32(L(0x36), e.edx);
    const reg32 cacheName = L(-0x7a);
    if (!call(kSprintf, {cacheName, e.edx, e.ebx, name}, {cacheName, kGlyphCacheName, name}, base))
        return std::nullopt;
    if (!call(kFileExists, {cacheName, e.edx, e.ebx, name}, {}, base))
        return std::nullopt;

    if (result != 0)
    {
        // The baked resource: its records given their pointers into what follows the header.
        if (!call(kLoadFile, {cacheName, cpu.edx, cpu.ebx, cpu.ecx}, {}, base))
            return std::nullopt;
        const reg32 loaded = result;
        kGlyphResource.set(app, loaded);
        reg32 cursor = loaded + kGlyphHeader;
        int32_t glyph = int32_t(byteAt(loaded));
        for (;;)
        {
            const reg32 resource = kGlyphResource(app);
            const int32_t end = int32_t(byteAt(resource + 1) + byteAt(resource));
            if (glyph >= end)
                break;
            const reg32 record = resource + 0x10 + (reg32(glyph) << 5);
            set32(record + 0x10, cursor);
            set32(record + 0x18, cursor + 12 * byteAt(record + 3));
            cursor += 24 * byteAt(record + 3);
            set32(record + 0x1c, cursor);
            cursor += 3 * byteAt(record + 1);
            set32(record + 0x14, cursor);
            ++glyph;
            cursor += 12 * byteAt(record + 1);
        }
        return kGlyphResource(app);
    }

    // Made from the font's own file.
    setWord(0x6a, result);
    setWord(0x42, 0xc4268000);
    setWord(0x46, 0x44268000);
    const reg32 reserveFlags = (app->getMemory<uint8_t>(L(0x36)) & 0x10) != 0 ? 0 : 0x10;
    reg32 total = word(0x6a);
    if (!call(kReserveMemory, {kGlyphArena, kGlyphHeader, reserveFlags, cpu.ecx}, {}, base))
        return std::nullopt;
    kGlyphResource.set(app, result);
    setWord(0x4e, reserveFlags);
    if (!call(kLoadFile, {cpu.ecx, reserveFlags, cpu.ebx, cpu.ecx}, {}, base))
        return std::nullopt;
    const reg32 source = result;
    reg32 resource = kGlyphResource(app);
    setByte(resource, byteAt(source));
    total += kGlyphHeader;
    setWord(0x3e, source);
    set32(resource + 4, 0x3f800000);
    reg32 cursor = source + 2;
    setByte(resource + 1, byteAt(source + 1));
    setWord(0x6a, total);
    setWord(0x66, byteAt(source));

    for (;;)
    {
        const int32_t end = int32_t(byteAt(word(0x3e) + 1) + byteAt(word(0x3e)));
        if (end <= int32_t(word(0x66)))
            break;
        const reg32 offset = word(0x66) << 5;
        reg32 record = kGlyphResource(app) + offset;
        setByte(record + 0x10, byteAt(cursor + 1));
        setWord(0x6e, byteAt(cursor + 1));
        setByte(record + 0x12, byteAt(cursor));
        setWord(0x7a, byteAt(cursor));
        setByte(record + 0x13, byteAt(cursor) + byteAt(cursor));
        const reg32 wide = (word(0x7a) + word(0x7a)) & 0xff;     // the whole layer's vertices
        cursor += 2;
        setWord(0x4a, wide);
        if (word(0x7a) == 0)
        {
            setByte(record + 0x11, 0);
            set32(record + 0x2c, 0);
            set32(record + 0x20, 0);
            setWord(0x66, word(0x66) + 1);
            continue;
        }
        // The vertices of the glyph: a block for both layers, and the bounds.
        const reg32 verticesSize = 24 * wide;
        if (!call(kReserveMemory, {kGlyphArena, verticesSize, word(0x4e), verticesSize}, {}, base))
            return std::nullopt;
        const reg32 vertices = result;
        set32(kGlyphResource(app) + offset + 0x20, vertices);
        setWord(0x5a, vertices);
        setWord(0x6a, word(0x6a) + verticesSize);
        setWord(0x56, 0x497423f0);
        setWord(0x32, 0x497423f0);
        setWord(0x2a, 0xc7c34f80);
        setWord(0x3a, 0xc7c34f80);
        setWord(0x2e, vertices + 12 * wide);
        const int32_t width = int32_t(word(0x7a));
        for (int32_t v = 0; v < width; ++v)
        {
            const reg32 upper = vertices + 12 * (reg32(width) + reg32(v));
            const reg32 lower = vertices + 12 * reg32(v);
            app->getMemory<float>(upper) = float(double(load<float>(app, cursor)));
            set32(lower, at32(upper));
            set32(upper + 4, at32(cursor + 4));
            app->getMemory<float>(lower + 4) = float(double(load<float>(app, upper + 4)));
            set32(lower + 8, 0);
            set32(upper + 8, 0x3f800000);
            cursor += 8;
            const double x = double(load<float>(app, lower)), y = double(load<float>(app, lower + 4));
            if (!(x >= double(bitsToFloat(word(0x32)))))
                setWord(0x32, at32(lower));
            else if (x > double(bitsToFloat(word(0x3a))))
                setWord(0x3a, at32(lower));
            if (!(y >= double(bitsToFloat(word(0x56)))))
                setWord(0x56, at32(lower + 4));
            else if (y > double(bitsToFloat(word(0x2a))))
                setWord(0x2a, at32(lower + 4));
        }
        resource = kGlyphResource(app);
        record = resource + (word(0x66) << 5);
        set32(record + 0x18, word(0x2a));
        app->getMemory<float>(record + 0x14) =
            float(double(bitsToFloat(word(0x3a))) * double(kGlyphScale(app)));
        set32(record + 0x1c, word(0x56));
        if (double(bitsToFloat(word(0x2a))) > double(bitsToFloat(word(0x42))))
            setWord(0x42, word(0x2a));
        if (!(double(bitsToFloat(word(0x56))) >= double(bitsToFloat(word(0x46)))))
            setWord(0x46, word(0x56));

        // The adjacency grid: which edges are used once.
        const reg32 grid_size = word(0x7a) * word(0x7a);
        if (!call(kReserveMemory, {kGlyphArena, grid_size, word(0x4e), grid_size}, {}, base))
            return std::nullopt;
        const reg32 grid = result;
        if (!call(kMemset, {grid, 0, grid_size, 0}, {}, base))
            return std::nullopt;
        reg32 edges = 0;
        setWord(0x76, 0);
        reg32 walk = cursor;
        const reg32 w = word(0x7a);
        for (int32_t t = 0; t < int32_t(word(0x6e)); ++t, walk += 3)
        {
            const reg32 corners[3] = {byteAt(walk), byteAt(walk + 1), byteAt(walk + 2)};
            for (int side = 0; side < 3; ++side)
            {
                const reg32 from = corners[side], to = corners[(side + 1) % 3];
                // The cell of the edge's reverse, [to * w + from] -- the first side's is
                // [i1 * w + i0], and so on round the triangle -- and the edge's own.
                const reg32 cell = grid + to * w + from;
                const reg32 own = grid + from * w + to;
                const reg32 mark = byteAt(cell);
                if (mark == 0)
                {
                    ++edges;
                    setWord(0x76, edges);
                    setByte(cell, 1);
                    setByte(own, 2);
                }
                else if (mark == 2)
                {
                    --edges;
                    setWord(0x76, edges);
                    setByte(cell, 0);
                    setByte(own, 0);
                }
            }
        }
        // The triangles of the glyph: front, back, and the edges' quads.
        const reg32 trianglesCount = 2 * (word(0x6e) + edges);
        setWord(0x1a, 3 * trianglesCount);
        if (!call(kReserveMemory, {kGlyphArena, 3 * trianglesCount, word(0x4e), trianglesCount}, {}, base))
            return std::nullopt;
        setWord(0x16, result);
        setWord(0x62, result);
        setWord(0x22, 12 * trianglesCount);
        if (!call(kReserveMemory, {kGlyphArena, 12 * trianglesCount, word(0x4e), trianglesCount}, {}, base))
            return std::nullopt;
        const reg32 normals = result;
        setWord(0x5e, normals);
        setWord(0x6a, word(0x6a) + 3 * trianglesCount + 12 * trianglesCount);
        setWord(0x22, kGlyphResource(app));
        record = kGlyphResource(app) + (word(0x66) << 5);
        set32(record + 0x24, normals);
        set32(record + 0x2c, word(0x16));
        setByte(record + 0x11, uint8_t(uint8_t(word(0x6e)) + uint8_t(word(0x76))) * 2);
        setWord(0x1e, trianglesCount - 1);
        setWord(0x72, 0);
        for (int32_t t = 0; t < int32_t(word(0x6e)); setWord(0x72, word(0x72) + 1), t = int32_t(word(0x72)))
        {
            const reg32 front = word(0x62) + 3 * reg32(t);
            setByte(front, byteAt(cursor));
            setByte(front + 1, byteAt(cursor + 1));
            setByte(front + 2, byteAt(cursor + 2));
            const reg32 frontNormal = word(0x5e) + 12 * reg32(t);
            set32(frontNormal, 0);
            set32(frontNormal + 4, 0);
            set32(frontNormal + 8, 0xbf800000);
            const uint8_t shift = uint8_t(word(0x7a));
            const uint8_t behind = uint8_t(byteAt(front + 2) + shift);
            app->getMemory<uint8_t>(L(0x7e)) = behind;
            const reg32 back = word(0x62) + 3 * (reg32(t) + word(0x6e));
            setByte(back, behind);
            setByte(back + 1, uint8_t(byteAt(front + 1) + shift));
            setByte(back + 2, uint8_t(byteAt(front) + shift));
            const reg32 backNormal = word(0x5e) + 12 * (reg32(t) + word(0x6e));
            set32(backNormal, 0);
            set32(backNormal + 4, 0);
            set32(backNormal + 8, 0x3f800000);
            const reg32 counter = L(0x1e);
            const reg32 triangles = word(0x62), verticesAt = word(0x5a), normalsAt = word(0x5e);
            const reg32 faces = word(0x6e), tt = reg32(t), ww = word(0x7a);
            cursor += 3;
            if (!call(kEmitEdges, {0, 1, 2, 1}, {grid, triangles, verticesAt, normalsAt, ww, faces, tt, counter}, base))
                return std::nullopt;
            if (!call(kEmitEdges, {1, 2, 1, 0}, {grid, triangles, verticesAt, normalsAt, ww, faces, tt, counter}, base))
                return std::nullopt;
            if (!call(kEmitEdges, {2, 0, 0, 2}, {grid, triangles, verticesAt, normalsAt, ww, faces, tt, counter}, base))
                return std::nullopt;
        }
        if (!call(kPurgeMemory, {grid, 0, 0, 0}, {}, base))
            return std::nullopt;
        // The vertices' normals: the sum of the normals of the triangles a vertex is a corner of.
        const reg32 sumX = L(6), sumY = L(0xa), sumZ = L(0xe);
        for (int32_t v = 0; v < int32_t(word(0x4a)); ++v)
        {
            set32(sumY, 0);
            set32(sumZ, 0);
            set32(sumX, 0);
            const reg32 count = 2 * (word(0x6e) + word(0x76));
            for (reg32 q = 0; q < count; ++q)
            {
                const reg32 triangle = word(0x62) + 3 * q;
                setWord(0x22, byteAt(triangle));
                const reg32 normal = word(0x5e) + 12 * q;
                if (reg32(v) == byteAt(triangle) || reg32(v) == byteAt(triangle + 1) || byteAt(triangle + 2) == reg32(v))
                {
                    app->getMemory<float>(sumX) = float(double(load<float>(app, sumX)) + double(load<float>(app, normal)));
                    app->getMemory<float>(sumY) =
                        float(double(load<float>(app, sumY)) + double(load<float>(app, normal + 4)));
                    app->getMemory<float>(sumZ) =
                        float(double(load<float>(app, sumZ)) + double(load<float>(app, normal + 8)));
                }
            }
            if (!call(kUnitVector, {sumX, word(0x2e) + 12 * reg32(v), 0, 0}, {}, base))
                return std::nullopt;
        }
        setWord(0x66, word(0x66) + 1);
    }

    // Done with the font's file; the resource is moved into one block.
    if (!call(kPurgeMemory, {word(0x3e), 0, 0, 0}, {}, base))
        return std::nullopt;
    resource = kGlyphResource(app);
    set32(resource + 8, word(0x42));
    set32(resource + 0xc, word(0x46));
    if (!call(kReserveMemory, {kGlyphArena, word(0x6a), word(0x36), kGlyphHeader}, {}, base))
        return std::nullopt;
    const reg32 joined = result;
    copy(joined, resource, kGlyphHeader);
    if (!call(kPurgeMemory, {resource, 0, 0, 0}, {}, base))
        return std::nullopt;
    kGlyphResource.set(app, joined);
    cursor = joined + kGlyphHeader;
    setWord(0x52, byteAt(joined));
    for (;;)
    {
        resource = kGlyphResource(app);
        const int32_t end = int32_t(byteAt(resource + 1) + byteAt(resource));
        if (end <= int32_t(word(0x52)))
            break;
        reg32 offset = word(0x52) << 5;
        reg32 record = resource + offset;
        if (byteAt(record + 0x12) != 0)
        {
            const reg32 size = 24 * byteAt(record + 0x13);
            setWord(0x12, size);
            copy(cursor, at32(record + 0x20), size);
            if (!call(kPurgeMemory, {at32(resource + offset + 0x20), 0, 0, 0}, {}, base))
                return std::nullopt;
            record = kGlyphResource(app) + offset;
            set32(record + 0x20, cursor);
            set32(record + 0x28, cursor + 12 * word(0x4a));
            cursor += word(0x12);
        }
        offset = word(0x52) << 5;
        record = kGlyphResource(app) + offset;
        setWord(0x26, offset);
        if (byteAt(record + 0x11) != 0)
        {
            const reg32 trianglesSize = 3 * byteAt(record + 0x11);
            copy(cursor, at32(record + 0x2c), trianglesSize);
            if (!call(kPurgeMemory, {at32(kGlyphResource(app) + offset + 0x2c), 0, 0, 0}, {}, base))
                return std::nullopt;
            record = kGlyphResource(app) + offset;
            set32(record + 0x2c, cursor);
            cursor += trianglesSize;
            const reg32 normalsSize = 12 * byteAt(record + 0x11);
            copy(cursor, at32(record + 0x24), normalsSize);
            if (!call(kPurgeMemory, {at32(kGlyphResource(app) + offset + 0x24), 0, 0, 0}, {}, base))
                return std::nullopt;
            set32(kGlyphResource(app) + offset + 0x24, cursor);
            cursor += normalsSize;
        }
        setWord(0x52, word(0x52) + 1);
    }
    if (!call(kSaveFile, {cacheName, kGlyphResource(app), word(0x6a), 0}, {}, base))
        return std::nullopt;
    return kGlyphResource(app);
}

/* ---- 0x4733c0 ---- */

constexpr reg32 kCarColourCount = 0x439760;          // (car): how many colours it has
constexpr reg32 kCarColourBytes = 0x439810;          // (car, index, 0, &c0) stdcall: &c1, &c2, &c3
constexpr reg32 kVectorLength = 0x4e06a0;            // (&{x, y, z}): its length on the x87's stack
constexpr reg32 kFarthest = 0x459c4000;              // float 5000.0: the distance to beat
constexpr reg32 kNoColour = 0xff00ff00;

/* The colour of the car that is nearest to the target (0xRRGGBB in edx), by
 * the length of the difference of the bytes of its first three channels, the
 * last colour left out of the count as the code has it; the four bytes of the
 * one found go to the object (+0x20 to +0x23), or the default green when the
 * car has no colour to choose from.  The working is on the stack as the code
 * has it.  False when the game stops in a call. */
bool nearestColour(App* app, CPU& cpu, const Entry& e)
{
    const reg32 object = e.eax;
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x34);
    const HeldEbp held(cpu, ebp);
    const auto local = [&](int32_t offset) { return ebp + reg32(offset); };
    const auto putInt = [&](int32_t offset, reg32 value) { app->getMemory<reg32>(local(offset)) = value; };
    const auto putFloat = [&](int32_t offset, double value) -> float {
        const float stored = float(value);
        app->getMemory<float>(local(offset)) = stored;
        return stored;
    };
    const reg32 car = app->getMemory<reg32>(object + 0x14);

    reg32 count;
    if (!callGame(app, cpu, base, kCarColourCount, {car, e.edx, 0, kFarthest}, {}, count))
        return false;
    // What the call leaves in ebx, ecx and edx is what the code goes on with.
    const reg32 target = cpu.edx;
    putInt(-0x18, cpu.ebx);
    putInt(-0xc, cpu.ecx);
    const int32_t last = int32_t(count - 1);
    putInt(-0x10, reg32(last));
    if (!(last > 0))
    {
        app->getMemory<reg32>(object + 0x20) = kNoColour;
        return true;
    }
    putInt(-0x14, (target >> 16) & 0xff);
    const double red = putFloat(-0x28, double((target >> 16) & 0xff));
    putInt(-0x14, target & 0xff);
    const double blue = putFloat(-0x20, double(target & 0xff));
    putInt(-0x14, (target >> 8) & 0xff);
    const double green = putFloat(-0x24, double((target >> 8) & 0xff));
    for (int32_t index = 0; index < last; ++index)
    {
        if (!callGame(app, cpu, base, kCarColourBytes, {car, reg32(index), 0, local(-0x1a)},
                      {local(-0x1b), local(-0x1c), local(-0x19)}))
            return false;
        const reg32 first = app->getMemory<uint8_t>(local(-0x1a));
        const reg32 second = app->getMemory<uint8_t>(local(-0x1b));
        const reg32 third = app->getMemory<uint8_t>(local(-0x1c));
        putInt(-4, first);
        putInt(-4, second);
        putFloat(-0x34, red - double(first));
        putInt(-4, third);
        putFloat(-0x30, green - double(second));
        putFloat(-0x2c, blue - double(third));
        if (!callGame(app, cpu, base, kVectorLength, {local(-0x34), 0, 0, 0}))
            return false;
        const double distance = double(cpu.fpu.st(0));
        cpu.fpu.pop();
        const float stored = putFloat(-8, distance);
        if (!(distance >= double(app->getMemory<float>(local(-0xc)))))
        {
            putInt(-0x18, reg32(index));
            putInt(-0xc, load<reg32>(app, local(-8)));
        }
        (void)stored;
    }
    return callGame(app, cpu, base, kCarColourBytes,
                    {car, app->getMemory<reg32>(local(-0x18)), 0, object + 0x20}, {object + 0x21, object + 0x22, object + 0x23});
}

/* ---- 0x420ff0 ---- */

constexpr Var<double> kPositionScale{0x536df4};      // a position's float to the track's integer units
constexpr Table<reg32> kTrackTable{0x5efa48};        // by the record's [+4]: a track's data, whose +0x1c is a section
constexpr Var<reg32> kSections{0x5dd948};            // the sections, 36 bytes each
constexpr Var<int32_t> kSectionCount{0x5dd940};
constexpr reg32 kSectionSize = 36;

/* fistp at the nearest, or the x87's "integer indefinite" past the integer's range. */
int32_t roundedToInteger(double value)
{
    const double rounded = std::nearbyint(value);
    if (!(rounded >= -2147483648.0 && rounded <= 2147483647.0))
        return int32_t(0x80000000);
    return int32_t(rounded);
}

/* A product of two 16.16 fixed-point numbers, rounded as the code does: the
 * 64-bit product's middle 32 bits, plus the bit under them. */
int32_t fixedProduct(int32_t a, int32_t b)
{
    const int64_t product = int64_t(a) * int64_t(b);
    const uint32_t low = uint32_t(product);
    const uint32_t high = uint32_t(product >> 32);
    return int32_t((low >> 16) + (high << 16) + ((low >> 15) & 1));
}

/* Whether the section ahead of (or behind) the one the record stands in is of
 * kind 2, where "ahead" is the way the record faces along the section's own
 * direction (the dot product of its offset from the section's start and the
 * three signed bytes that name the direction, in 16.16): its kind is a nibble
 * of the section's byte 0xe -- the low one if it is set, else the high one for
 * the way ahead, the other way round behind.  The record's position is
 * written as the code writes it, to its stack. */
reg32 aheadKindTwo(App* app, const Entry& e)
{
    const reg32 record = e.eax;
    const reg32 ebp = e.frame(6);
    const auto putInt = [&](int32_t offset, int32_t value) { app->getMemory<int32_t>(ebp + reg32(offset)) = value; };
    const double scale = kPositionScale(app);
    int32_t position[3];
    for (int i = 0; i < 3; ++i)
    {
        position[i] = roundedToInteger(double(load<float>(app, record + 8 + 4 * reg32(i))) * scale);
        putInt(-0x20, position[i]);
    }
    putInt(-4, position[2]);
    const reg32 track = kTrackTable(app, int32_t(app->getMemory<reg32>(record + 4)));
    const int32_t section = app->getMemory<int32_t>(track + 0x1c);
    const reg32 start = kSections(app) + reg32(section) * kSectionSize;
    const auto minus = [](int32_t a, int32_t b) { return int32_t(uint32_t(a) - uint32_t(b)); };
    const int32_t offset[3] = {minus(position[0], app->getMemory<int32_t>(start)),
                               minus(position[1], app->getMemory<int32_t>(start + 4)),
                               minus(position[2], app->getMemory<int32_t>(start + 8))};
    putInt(-0x1c, offset[0]);
    putInt(-0x18, offset[1]);
    putInt(-0x14, offset[2]);
    int32_t direction[3];
    for (int i = 0; i < 3; ++i)
        direction[i] = int32_t(int8_t(app->getMemory<uint8_t>(start + 0x18 + reg32(i)))) * 512;
    putInt(-0x10, direction[0]);
    putInt(-0xc, direction[1]);
    putInt(-8, direction[2]);
    const int32_t along = int32_t(uint32_t(fixedProduct(direction[0], offset[0])) +
                                  uint32_t(fixedProduct(direction[1], offset[1])) +
                                  uint32_t(fixedProduct(direction[2], offset[2])));
    const int32_t count = kSectionCount(app);
    int32_t next;
    if (app->getMemory<int32_t>(kTrackTable(app, int32_t(app->getMemory<reg32>(record + 4))) + 0x998) != 0)
    {
        next = int32_t(uint32_t(section) + 8);
        if (next >= count)
            next = int32_t(uint32_t(next) - uint32_t(count));
    }
    else
    {
        next = int32_t(uint32_t(section) - 8);
        if (next < 0)
            next = int32_t(uint32_t(next) + uint32_t(count));
    }
    const uint8_t kinds = app->getMemory<uint8_t>(kSections(app) + reg32(next) * kSectionSize + 0xe);
    const uint8_t low = kinds & 0xf, high = kinds >> 4;
    uint8_t chosen;
    if (along > 0)
        chosen = low != 0 ? low : high;
    else
        chosen = high != 0 ? high : low;
    return chosen == 2 ? 1 : 0;
}

/* ---- 0x464840 ---- */

constexpr Var<float> kGridWidthScale{0x53ab1c};      // times the box's width, the zoom: the grid's scale
constexpr Var<double> kGridDepth{0x53ab24};          // added to the far corners' depth
constexpr Var<double> kGridStep{0x53ab2c};           // a tile's side in texture coordinates
constexpr Var<double> kGridLimit{0x53ab34};          // how far a tile's texture runs before it is cut
constexpr reg32 kGridFloor = 0x47493277;             // float: the ground's squared radius
constexpr reg32 kGridDistance = 0x4362f34a;          // float: the view's distance
constexpr reg32 kGridOne = 0x3f800000, kGridJustUnderOne = 0x3f7f8000;
constexpr reg32 kVertex = 0x55a444, kVertexSize = 0x20, kVertexOutcode = 0x14;   // four, at 0x20 each
constexpr reg32 kTileTexture = 0x678960, kTileTextureStride = 44;
constexpr Var<int32_t> kClipNearZ{0x7d34f8}, kClipMinX{0x7d34fc}, kClipMaxY{0x7d3500}, kClipMinY{0x7d3504},
    kClipMaxX{0x7d350c};
constexpr reg32 kSquareRoot = 0x4ea920;              // (float) stdcall: the root on the x87's stack
constexpr reg32 kSetClip = 0x4bebe0;                 // (x, y, w, h)
constexpr reg32 kQuad = 0x4c12e0;                    // (v0, v1, v2, v3): four vertices drawn clipped
constexpr reg32 kPictureFlush = 0x431bc0;
constexpr reg32 kRenderTextureState = 0x2c2, kRenderTile = 1;
constexpr guest::Native kViewport = text::viewport;  // 0x4beb80

/* Half, towards zero, as the code halves a 32-bit integer. */
int32_t halved(int32_t value)
{
    return (value - (value >> 31)) >> 1;
}

/* A vertex's clip outcode: 0x10 behind the near plane, else the bits of the
 * sides it is out of (1 left, 2 right, 8 above, 4 below), read as the code
 * does, on the bits of the floats as signed integers. */
uint8_t outcodeOf(App* app, reg32 vertex)
{
    const reg32 z = app->getMemory<reg32>(vertex + 0xc);
    if ((z & 0x80000000) != 0 || int32_t(z) >= kClipNearZ(app))
        return 0x10;
    uint8_t bits = 0;
    const reg32 y = app->getMemory<reg32>(vertex + 4);
    if ((y & 0x80000000) != 0 || int32_t(y) < kClipMinY(app))
        bits |= 8;
    else if (int32_t(y) > kClipMaxY(app))
        bits |= 4;
    const reg32 x = app->getMemory<reg32>(vertex);
    if ((x & 0x80000000) != 0 || int32_t(x) < kClipMinX(app))
        bits |= 1;
    else if (int32_t(x) > kClipMaxX(app))
        bits |= 2;
    return bits;
}

/* The square root the game gives for a float: back from the x87's stack. */
bool rootOf(App* app, CPU& cpu, reg32 esp, float value, float& root)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    if (!callGame(app, cpu, esp, kSquareRoot, {}, {bits}))
        return false;
    root = float(cpu.fpu.st(0));
    cpu.fpu.pop();
    return true;
}

/* The ground's grid in a box (x, y, w, h) of the screen: tiles of 16 by 16,
 * five columns of textures wide, from the map's offset (*mapX, *mapY, kept to
 * what the box shows) and the zoom (*scale, at least 1.0), each tile's corners
 * put in perspective by the roots of the ground's radius round them, clipped
 * (outcodes) and drawn.  The code keeps its working in 0x6c bytes of the game's
 * stack and stores each as it goes; so does this, in the order it does.  False
 * when the game stops in a call. */
bool drawGrid(App* app, CPU& cpu, const Entry& e)
{
    const reg32 x = e.eax, y = e.edx, w = e.ebx, h = e.ecx;
    const reg32 mapX = e.arg(app, 0), mapY = e.arg(app, 1), scale = e.arg(app, 2);
    const reg32 ebp = e.frame(3);
    const reg32 base = e.frame(3, 0x6c);
    const HeldEbp held(cpu, ebp);
    const auto local = [&](int32_t offset) { return ebp + reg32(offset); };
    const auto putInt = [&](int32_t offset, int32_t value) { app->getMemory<int32_t>(local(offset)) = value; };
    const auto putFloat = [&](int32_t offset, double value) -> float {
        const float stored = float(value);
        app->getMemory<float>(local(offset)) = stored;
        return stored;
    };
    const auto vertexAt = [](int index) { return kVertex + kVertexSize * reg32(index); };
    const auto put = [&](reg32 at, double value) -> float {
        const float stored = float(value);
        app->getMemory<float>(at) = stored;
        return stored;
    };

    putInt(-0x44, int32_t(y));
    putInt(-0x38, int32_t(h));
    const int32_t halfRange = halved(0x100 - int32_t(h));
    putInt(-0x3c, halfRange);
    putInt(-0x48, int32_t(kGridFloor));
    putInt(-4, int32_t(x) + halved(int32_t(w)));
    putInt(-0x24, int32_t(kGridDistance));
    const double centreX = putFloat(-0x34, double(int32_t(x) + halved(int32_t(w))));
    putInt(-4, halved(int32_t(h)) + int32_t(y));
    const double centreY = putFloat(-0x10, double(halved(int32_t(h)) + int32_t(y)));
    if (!callGame(app, cpu, base, kViewport, {local(-0x68), local(-0x64), local(-0x6c), local(-0x60)}))
        return false;
    if (!callGame(app, cpu, base, kSetClip, {x, y, w, h}))
        return false;
    // The zoom, at least 1.0; the box's width times it, times the box's constant.
    if (1.0 > double(app->getMemory<float>(scale)))
        app->getMemory<reg32>(scale) = kGridOne;
    putInt(-4, int32_t(w));
    const reg32 scaleBits = app->getMemory<reg32>(scale);
    putInt(-8, int32_t(scaleBits));
    const double zoom =
        putFloat(-8, double(int32_t(w)) * double(kGridWidthScale(app)) * double(bitsToFloat(scaleBits)));
    // The offset down the box kept within what it shows.
    if (app->getMemory<int32_t>(mapY) > halfRange)
        app->getMemory<int32_t>(mapY) = halfRange;
    if (-halfRange > app->getMemory<int32_t>(mapY))
        app->getMemory<int32_t>(mapY) = -halfRange;
    if (!callGame(app, cpu, base, kSetRenderState, {kRenderTextureState, 1, cpu.ebx, cpu.ecx}))
        return false;
    const double distance = double(bitsToFloat(kGridDistance));
    const double floorRadius = double(bitsToFloat(kGridFloor));
    const bool translucent = (app->getMemory<uint8_t>(kTranslucency) & 2) != 0;

    int32_t row = 0;
    putInt(-0x14, row);
    for (;;)
    {
        putInt(-0x1c, 0);
        const int32_t baseY = (row << 8) + app->getMemory<int32_t>(mapY) - 0x80;
        putInt(-0x54, baseY);
        for (int32_t column = 0; column < 5;)
        {
            const int32_t sum = app->getMemory<int32_t>(mapX) + (column << 8);
            putInt(-0xc, sum);
            int32_t offsetX;
            if (sum < 0)
            {
                const int32_t remainder = int32_t(0u - uint32_t(sum)) % 0x500;
                putInt(-0xc, remainder);
                offsetX = 0x500 - remainder - 0x1a0;
            }
            else
                offsetX = sum % 0x500 - 0x1a0;
            putInt(-0x20, 0);
            putInt(-0xc, offsetX);
            for (int32_t line = 0; line < 16;)
            {
                const int32_t top = baseY + (line << 4);
                putInt(-0x30, top);
                if (-(halved(int32_t(h)) + 0x10) <= top && !(halved(int32_t(h)) < top))
                {
                    putInt(-0x18, 0);
                    for (int32_t tile = 0; tile < 16;)
                    {
                        const int32_t left = offsetX + (tile << 4);
                        if (left >= -176 && !(left > 160))
                        {
                            // The tile's corners, by the roots of the ground's radius round them.
                            putInt(-4, left);
                            const float a = putFloat(-0x28, double(left));
                            const double leftSquare = double(left) * double(a);
                            putInt(-4, left + 0x10);
                            const float b = putFloat(-0x2c, double(left + 0x10));
                            putInt(-4, top);
                            const float c = putFloat(-0x40, double(top));
                            putInt(-4, top + 0x10);
                            float root1, root2;
                            if (!rootOf(app, cpu, base, float(floorRadius - leftSquare), root1))
                                return false;
                            const float d = putFloat(-0x4c, double(top + 0x10));
                            const float first = putFloat(-0x50, double(root1));
                            if (!rootOf(app, cpu, base, float(floorRadius - double(b) * double(b)), root2))
                                return false;
                            const float second = putFloat(-0x58, double(root2));
                            if (!translucent)
                            {
                                const double inverseFirst = 1.0 / double(first), inverseSecond = 1.0 / double(second);
                                const double cScaled = double(c) * distance;
                                const float cScaledStored = putFloat(-0x5c, cScaled);
                                const double dScaled = distance * double(d);
                                const float dScaledStored = putFloat(-4, dScaled);
                                put(vertexAt(0) + 0, ((double(a) * distance) * inverseFirst) * zoom + centreX);
                                put(vertexAt(0) + 4, (cScaled * inverseFirst) * zoom + centreY);
                                put(vertexAt(1) + 0, ((double(b) * distance) * inverseSecond) * zoom + centreX);
                                put(vertexAt(1) + 4, (double(cScaledStored) * inverseSecond) * zoom + centreY);
                                put(vertexAt(2) + 4, ((inverseSecond * dScaled) * zoom) + centreY);
                                put(vertexAt(3) + 4, centreY + (zoom * (inverseFirst * double(dScaledStored))));
                                app->getMemory<reg32>(vertexAt(2) + 0) = app->getMemory<reg32>(vertexAt(1) + 0);
                                app->getMemory<float>(vertexAt(3) + 0) = app->getMemory<float>(vertexAt(0) + 0);
                            }
                            else
                            {
                                put(vertexAt(0) + 0, double(a) * zoom + centreX);
                                put(vertexAt(0) + 4, double(c) * zoom + centreY);
                                put(vertexAt(1) + 0, centreX + double(b) * zoom);
                                put(vertexAt(1) + 4, double(c) * zoom + centreY);
                                put(vertexAt(2) + 0, centreX + double(b) * zoom);
                                put(vertexAt(3) + 0, double(a) * zoom + centreX);
                                const float down = put(vertexAt(2) + 4, (zoom * double(d) + centreY) + kGridDepth(app));
                                app->getMemory<float>(vertexAt(3) + 4) = down;
                            }
                            // The tile's texture coordinates, and the tile's texture.
                            const double step = kGridStep(app);
                            putInt(-4, tile);
                            const double tileU = double(tile) * step, tileV = double(line) * step;
                            putInt(-4, line);
                            put(0x55a45c, tileU);
                            put(0x55a460, tileV);
                            const double farU = step + tileU;
                            put(0x55a47c, farU);
                            if (farU > kGridLimit(app))
                                app->getMemory<reg32>(0x55a47c) = kGridJustUnderOne;
                            const float tileUStored = app->getMemory<float>(0x55a45c);
                            const float tileVStored = app->getMemory<float>(0x55a460);
                            const float farUStored = app->getMemory<float>(0x55a47c);
                            app->getMemory<float>(0x55a480) = tileVStored;
                            app->getMemory<float>(0x55a49c) = farUStored;
                            app->getMemory<float>(0x55a4bc) = tileUStored;
                            put(0x55a4a0, double(tileVStored) + step);
                            app->getMemory<float>(0x55a4c0) = app->getMemory<float>(0x55a4a0);
                            const int32_t tileNumber = column + 5 * row;
                            const reg32 texture = app->getMemory<reg32>(kTileTexture + kTileTextureStride * reg32(tileNumber));
                            if (!callGame(app, cpu, base, kSetRenderState, {kRenderTile, texture, 0, 0}))
                                return false;
                            for (int corner = 0; corner < 4; ++corner)
                                app->getMemory<uint8_t>(vertexAt(corner) + kVertexOutcode) = outcodeOf(app, vertexAt(corner));
                            if (!callGame(app, cpu, base, kQuad, {vertexAt(0), vertexAt(1), vertexAt(2), vertexAt(3)}))
                                return false;
                        }
                        ++tile;
                        putInt(-0x18, tile);
                    }
                }
                ++line;
                putInt(-0x20, line);
            }
            ++column;
            putInt(-0x1c, column);
        }
        ++row;
        putInt(-0x14, row);
        if (row >= 1)
            break;
    }
    const reg32 left = app->getMemory<reg32>(local(-0x68)), top = app->getMemory<reg32>(local(-0x64));
    const reg32 width = app->getMemory<reg32>(local(-0x6c)), height = app->getMemory<reg32>(local(-0x60));
    if (!callGame(app, cpu, base, kSetClip, {left, top, width, height}))
        return false;
    return callGame(app, cpu, base, kPictureFlush, {left, top, width, height});
}

/* ---- 0x464fd0 ---- */

/* fistp after Watcom's __CHP: the value towards zero, or the x87's "integer
 * indefinite" past the integer's range. */
int32_t chopToInteger(double value)
{
    if (!(value > -2147483649.0 && value < 2147483648.0))
        return int32_t(0x80000000);
    return int32_t(value);
}

/* The view's frame: the mouse read as a key, the view's mode set up, and --
 * where the menu shows the view -- the pan applied to the map's offset, the
 * frame and then the grid drawn in the 320 by 192 box over the object, and the
 * mouse's drag turned into the offset: pressing starts it (the pan back to 0,
 * the mouse noted, bit 2 of the object's flag byte set), moving while held
 * shifts the offset by the mouse's move, letting go clears the bit.  Gives
 * what the mouse call left in edx.  False when the game stops in a call. */
std::optional<reg32> frameOfView(App* app, CPU& cpu, const Entry& e)
{
    const reg32 object = e.eax;
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x10);
    const HeldEbp held(cpu, ebp);
    const reg32 mouseFlag = ebp - 4, enterHeld = ebp - 8;

    reg32 key;
    if (!callGame(app, cpu, base, kMouseKey, {object, 0, e.ebx, e.edx}, {}, key))
        return std::nullopt;
    app->getMemory<reg32>(mouseFlag) = cpu.edx;
    app->getMemory<reg32>(enterHeld) = key == kEnter ? 1 : 0;
    if (!callGame(app, cpu, base, kPrepareView, {app->getMemory<reg32>(enterHeld), kMenuView, cpu.ebx, cpu.ecx}))
        return std::nullopt;
    kPanActive.set(app, cpu.ecx);
    reg32 shown;
    if (!callGame(app, cpu, base, kMenuShown, {object, cpu.edx, cpu.ebx, cpu.ecx}, {}, shown))
        return std::nullopt;
    if (shown != 0)
    {
        const reg32 ecx = cpu.ecx;
        // The map's offset, moved by the pan; each as the integer the code chops it to.
        const double x = double(int32_t(kMapX(app))) + double(kPanX(app));
        const double y = double(int32_t(kMapY(app))) + double(kPanY(app));
        kMapX.set(app, reg32(chopToInteger(x)));
        kMapY.set(app, reg32(chopToInteger(y)));
        const reg32 colour = ecx != 0 ? 0xffffe440 : 0xff000040;
        const bool translucent = (app->getMemory<uint8_t>(kTranslucency) & 2) != 0;
        const reg32 left = reg32(int16_t(app->getMemory<uint16_t>(object + 6)));
        const reg32 top = reg32(int16_t(app->getMemory<uint16_t>(object + 8)));
        if (translucent && !callGame(app, cpu, base, kSetRenderState, {6, 1, colour, ecx}))
            return std::nullopt;
        if (!callGame(app, cpu, base, kBoxFrame, {left, top, 0x140, 0xc0}, {colour}))
            return std::nullopt;
        if (translucent && !callGame(app, cpu, base, kSetRenderState, {6, 0, cpu.ebx, cpu.ecx}))
            return std::nullopt;
        if (!callGame(app, cpu, base, kGrid, {left, top, 0x140, 0xc0}, {kMapX.at, kMapY.at, kPanBase.at}))
            return std::nullopt;
        if (kDragging(app) == 0)
        {
            if (app->getMemory<reg32>(enterHeld) != 0 && kPanActive(app) != 0)
            {
                kPanX.set(app, 0.0f);
                kPanY.set(app, 0.0f);
                kDragMouseX.set(app, kMouseX(app));
                kDragMouseY.set(app, kMouseY(app));
                kDragging.set(app, 1);
                app->getMemory<uint8_t>(object + 5) = uint8_t(app->getMemory<uint8_t>(object + 5) | 4);
            }
        }
        else if (app->getMemory<reg32>(enterHeld) == 0)
        {
            kDragging.set(app, 0);
            app->getMemory<uint8_t>(object + 5) = uint8_t(app->getMemory<uint8_t>(object + 5) & 0xfb);
        }
        else
        {
            kMapX.set(app, kMapX(app) + (kMouseX(app) - kDragMouseX(app)));
            kMapY.set(app, kMapY(app) + (kMouseY(app) - kDragMouseY(app)));
            kDragMouseX.set(app, kMouseX(app));
            kDragMouseY.set(app, kMouseY(app));
        }
    }
    return app->getMemory<reg32>(mouseFlag);
}

/* ---- 0x46cf30 ---- */

/* The popup over the race's results for the participant whose box the pointer
 * is in: the metrics of that participant's entry, a line of text each, the
 * highest in red.  It works on a box of four int16 on the game's stack (left,
 * top, width, height), stepped down the rows until the pointer is in one; the
 * index of the highest metric is kept on the stack too, and read from there as
 * the code leaves it when none is above zero.  False when the game stops in a
 * call. */
bool popupMetrics(App* app, CPU& cpu, const Entry& e)
{
    const reg32 record = e.eax;
    const reg32 ebp = e.frame(6);                 // the frame pointer the code holds
    const reg32 base = e.frame(6, 0x118);         // esp past its locals
    const reg32 box = ebp - 0x18;                 // four int16
    const reg32 best = ebp - 0xc, bestValue = ebp - 0x10, count = ebp - 4, entryAt = ebp - 8, text = ebp - 0x118;
    const HeldEbp held(cpu, ebp);
    const auto word = [&](reg32 at) -> uint16_t { return app->getMemory<uint16_t>(at); };
    const auto field = [&](int index) -> int32_t { return int16_t(word(box + 2 * reg32(index))); };
    const auto recordWord = [&](reg32 offset) -> uint16_t { return word(record + offset); };

    if (kScreenKind(app) == 3)
        return true;
    // The box: the record's parts added up, as 16 bits.
    app->getMemory<uint16_t>(box + 2) =
        uint16_t(recordWord(kBoxTop) + recordWord(kBoxPartA) + recordWord(kBoxPartB) + recordWord(kBoxPartC));
    app->getMemory<uint16_t>(box) = uint16_t(recordWord(kBoxLeft) + recordWord(kBoxStep));
    app->getMemory<uint16_t>(box + 4) = uint16_t(recordWord(kBoxWidth) + 1);
    app->getMemory<uint16_t>(box + 6) = uint16_t(recordWord(kBoxStep) + 1);
    const int32_t rows = int16_t(recordWord(kBoxRows));
    int32_t step = 0;
    for (; step < rows; ++step)
    {
        reg32 inBox;
        if (!callGame(app, cpu, base, kPointerInBox, {box, record, 0, reg32(step)}, {}, inBox))
            return false;
        if (inBox != 0)
            break;
        app->getMemory<uint16_t>(box) = uint16_t(word(box) + recordWord(kBoxStep));
    }
    if (step >= rows)
        return true;
    // The entry of the participant that row is for.
    reg32 entry;
    if (!callGame(app, cpu, base, kEntryOf, {0, reg32(step + int16_t(recordWord(kBoxFirst))), 0, 0}, {}, entry))
        return false;
    app->getMemory<reg32>(entryAt) = entry;
    if (entry == 0 || app->getMemory<reg32>(entry + kEntryKind) == kEntryNone)
        return true;
    const reg32 metrics = kMetricCount(app);
    app->getMemory<reg32>(count) = metrics;
    app->getMemory<reg32>(bestValue) = 0;
    for (int32_t j = 0; j < int32_t(metrics); ++j)
    {
        const reg32 at = entry + reg32(j) * 4 + kMetricValues;
        if (double(load<float>(app, at)) > double(app->getMemory<float>(bestValue)))
        {
            app->getMemory<reg32>(best) = reg32(j);
            app->getMemory<reg32>(bestValue) = load<reg32>(app, at);
        }
    }
    // The panel behind the metrics, and the title over them.
    if (!callGame(app, cpu, base, kFieldBox,
                  {reg32(field(1) + 2), reg32(field(0) + 2), reg32(field(2) - 3), metrics * 15 + 0x20}, {0xa0000040, 0xa0000040, 0xd0222222, 0xd0222222}))
        return false;
    const reg32 titleY = reg32(field(1) + field(2) / 2), titleX = reg32(field(0) + 8);
    reg32 line;
    if (!callGame(app, cpu, base - 8, kLanguageText, {kTitleLine, titleY, titleX, kFont}, {}, line))
        return false;
    if (!callGame(app, cpu, base, kDrawText, {line, titleY, titleX, kFont}, {kAlignTitle, kTitleColour}))
        return false;
    // A line to a metric: its value, written to the buffer and drawn, then its number.
    reg32 edi = reg32(field(0) + 0x18);
    for (int32_t j = 0; j < int32_t(metrics); ++j)
    {
        const reg32 at = entry + reg32(j) * 4;
        if (!callGame(app, cpu, base, kMetricText, {at, titleY, app->getMemory<reg32>(best), kFont},
                      {load<reg32>(app, at + kMetricValues), text}))
            return false;
        const reg32 valueColour = reg32(j) == cpu.ebx ? kBestColour : kOtherColour;
        const reg32 valueY = reg32(field(1) + field(2) - 8);
        if (!callGame(app, cpu, base, kDrawText, {text, valueY, edi, kFont}, {kAlignValue, valueColour}))
            return false;
        if (!callGame(app, cpu, base, kSprintf, {text, valueY, edi, kFont}, {text, kFormat, reg32(j + 1)}))
            return false;
        const reg32 numberColour = reg32(j) == app->getMemory<reg32>(best) ? kBestColour : kOtherColour;
        const reg32 numberY = reg32(field(1) + 8);
        if (!callGame(app, cpu, base, kDrawText, {text, numberY, edi, kFont}, {kAlignNumber, numberColour}))
            return false;
        edi += 0xf;
    }
    return true;
}

}  // namespace

/* ---- The adapters ---- */

void storeIncremented(App* app, CPU& cpu)
{
    const Entry e(cpu);
    storedIncremented(app, bitsToFloat(e.arg(app, 0)));
    e.back(cpu, e.eax, 4);
}

void recordFloat(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.fpu.push(Float(double(firstFloat(app, e.eax))));
    e.back(cpu, e.eax);
}

void recordFloatAt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.fpu.push(Float(double(floatAt(app, e.eax, e.edx))));
    e.back(cpu, e.eax);
}

void panKey(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const auto out = keyPan(app, cpu, e);
    if (!out)
        return;
    e.back(cpu, out->eax);
    cpu.ebx = out->ebx;
    cpu.edx = out->edx;
}

void glyphResource(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const auto result = glyphResourceOf(app, cpu, e);
    if (result)
        e.back(cpu, *result);
}

void edgeTriangles(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (edgeTrianglesOf(app, cpu, e))
        e.back(cpu, e.eax, 0x20);
}

void colourNearest(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (nearestColour(app, cpu, e))
        e.back(cpu, e.eax);
}

void sectionAheadIsTwo(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, aheadKindTwo(app, e));
}

void gridDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (drawGrid(app, cpu, e))
        e.back(cpu, e.eax, 12);
}

void viewFrame(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const auto result = frameOfView(app, cpu, e);
    if (result)
        e.back(cpu, *result);
}

void metricPopup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (popupMetrics(app, cpu, e))
        e.back(cpu, e.eax);
}

void recordFloatByKind(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 edx;
    cpu.fpu.push(Float(double(floatByKind(app, e.eax, e.edx, edx))));
    e.back(cpu, e.eax);
    cpu.edx = edx;
}

}  // namespace view
}  // namespace port
}  // namespace nfs3hp
