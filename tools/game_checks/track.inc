// game_checks.cpp's parts for the track (native_track.cpp), included in each table with the macro naming the part.
#if defined(TRACK_WINDOWS)
// The code's data the headlights copy (0x41a880..0x41a8c8), the colours'
// tint, the track's blocks, the views and the objects' states, the cars'
// pointers, the frame counter, the screen's bounds, the buckets' pointers,
// the lights' kinds, and the cases' own (0xd00000 to 0xd80000).
{0x41a000, 0x41b000}, {0x55d000, 0x55e000}, {0x571000, 0x575000}, {0x5dd000, 0x5de000}, {0x5ef000, 0x5f0000},
{0x5f1000, 0x5f2000}, {0x7d3000, 0x7d4000}, {0x7dc000, 0x7dd000}, {0x8b4000, 0x8b5000}, {0xd00000, 0xd80000},
#elif defined(TRACK_EFFECTS)
    kTrackRoom,     // Render_GetTm's room (0x4bbde0, 0x4bbe40): [eax] a place in the cases' buffer; 1, or 0 now and then
    kTrackOutside,  // a box out of view (0x41bf30): 1 now and then
    kTrackStep,     // a path's step (0x40f940): 1 mostly, its place to [edx] and its pose to [ebx] then
    kTrackPose,     // a car's pose (0x40f4b0): its place to [ecx] and its pose to [esp+4], as the real one writes
    kTrackLength,   // a path's length (0x40f800): its steps to [edx], its length, never 0, to [ebx]
    kTrackLit,      // track vertices lit (0x41a550): edx records of 0x1c bytes at ecx, each's vertex one of the cases'
    kTrackCount,    // a .qfs's textures, a .col's chunks: a few
    kTrackChunk,    // a .col's chunk edx: one of the cases' eight
    kTrackNearest,  // the nearest block (0x41e0f0; 0x4217e0 is trackdata.inc's): one of the cases' blocks, and a part
    kTrackPoints,   // points into a view (0x4e0430): eax of them, 12 bytes each, to [esp+4], as the real one writes
    kTrackMatrix,   // matrices multiplied (0x4e0280): nine floats into [ebx], as the real one writes
#elif defined(TRACK_EFFECT_CASES)
        case kTrackRoom:
            if (words[0] < kMemory - 4)
                word(words[0]) = 0xd40000 + 0x1c * (r % 16);
            result = (r >> 4) % 8 != 0 ? 1 : 0;
            break;
        case kTrackOutside:
            result = r % 4 == 0 ? 1 : 0;
            break;
        case kTrackStep:
        case kTrackPose:
        {
            // The place (3 floats) and the pose (9) written as the real one writes them: the objects
            // read them back from their frames (drawObjectList's), else the frame's garbage, which
            // differs once the natives' frames do.
            const bool step = s.effect == kTrackStep;
            if (step)
                result = r % 5 != 0 ? 1 : r % 2;
            const x86::reg32 place = step ? words[1] : words[3], pose = step ? words[2] : words[4];
            for (x86::reg32 i = 0; (!step || result == 1) && i < 12; ++i)
            {
                const x86::reg32 to = i < 3 ? place + 4 * i : pose + 4 * (i - 3);
                const float f = float(x86::sreg32(random(id + 0x60 + i) % 4001) - 2000) / (i < 3 ? 20.0f : 2000.0f);
                x86::reg32 b;
                std::memcpy(&b, &f, 4);
                if (to < kMemory - 4)
                    word(to) = b;
            }
            break;
        }
        case kTrackLength:
            if (words[1] < kMemory - 4)
                word(words[1]) = random(id + 1) % 0x200;
            if (words[2] < kMemory - 4)
                word(words[2]) = 1 + random(id + 2) % 0x100;
            break;
        case kTrackLit:
        {
            const x86::reg32 at = words[3];
            const bool ours = (at >= 0xd00000 && at < 0xd70000) || (at >= kStackLow && at < kStackHigh - 0x400);
            for (x86::reg32 i = 0; ours && i < words[1] && i < 32; ++i)
            {
                const x86::reg32 record = at + 0x1c * i;
                const x86::reg32 v = random(id + 3 * i), w = random(id + 3 * i + 1);
                getMemory<x86::reg16>(record + 4) = x86::reg16(v % 3 == 0 ? 0 : v >> 8);
                getMemory<x86::reg16>(record + 6) = x86::reg16(w % 3 == 0 ? 0 : w >> 8);
                word(record + 8) = 0xd60000 + 0x20 * ((v >> 16) % 0x80);
                word(record + 0x14) = random(id + 3 * i + 2);
                word(record + 0x18) = w;
            }
            break;
        }
        case kTrackCount:
            result = r % 7;
            break;
        case kTrackChunk:
            result = 0xd70000 + 0x100 * (words[1] % 8);
            break;
        case kTrackNearest:
            result = r % 0x40;
            break;
        case kTrackMatrix:
            // A product written, made up: turnFromAngles multiplies by the matrix it is handed
            // before it writes it (drawObjectList's turn, the frame's garbage for its first
            // object), so that what it writes there is never that garbage.
            for (x86::reg32 i = 0; i < 9 && words[2] + 4 * i < kMemory - 4; ++i)
            {
                const float f = float(x86::sreg32(random(id + 0x40 + i) % 2001) - 1000) / 1000.0f;
                x86::reg32 b;
                std::memcpy(&b, &f, 4);
                word(words[2] + 4 * i) = b;
            }
            break;
        case kTrackPoints:
            // Written as the real one writes them: what the natives read back of their frames
            // (a light's depth in view) is then never the frame's garbage, which differs once
            // the natives' frames do.
            for (x86::reg32 i = 0; i < 3 * (words[0] < 8 ? words[0] : 8) && words[4] + 4 * i < kMemory - 4; ++i)
            {
                const float f = float(x86::sreg32(random(id + 0x40 + i) % 4001) - 2000) / 10.0f;
                x86::reg32 b;
                std::memcpy(&b, &f, 4);
                word(words[4] + 4 * i) = b;
            }
            break;
#elif defined(TRACK_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// What the track's natives call that hands back something they read on: room
// from Render_GetTm, whether a box is out of view, a path's step and length,
// the track's vertices lit, the light's spread (a float on the x87's stack), a
// .qfs's textures, a .col's chunks.
{0x4bbde0, "Render_GetTm's room", kWatcom, 2, 0, kEax, -1, 0, kTrackRoom},
{0x4bbe40, "Render_GetTm's next", kWatcom, 2, 0, kEax, -1, 0, kTrackRoom},
{0x41bf30, "a box out of view", kWatcom, 4, 0, kEax, -1, 0, kTrackOutside},
{0x40f940, "a path's step", kWatcom, 3, 0, kEax, -1, 0, kTrackStep},
// A car's pose (aiplan::animationPose; aiplan.inc has it after this, writing nothing): written, as
// drawObjectList reads the place back from its frame for the headlights.
{0x40f4b0, "a car's pose", kStdcall, 4, 1, kEax, -1, 0, kTrackPose},
{0x40f800, "a path's length", kWatcom, 3, 0, kEax, -1, 0, kTrackLength},
{0x41a550, "track vertices lit", kStdcall, 4, 1, kEax, -1, 0, kTrackLit},
{0x4227e0, "the light's spread", kWatcom, 1, 0, kEax, -1, 0, kFloatBack},
{0x4ea654, "a .qfs's textures", kWatcom, 1, 0, kEax, -1, 0, kTrackCount},
// A .col's chunks' count logs no register: collide::collCount reads none, and loadCollisions
// calls it as C++ (guest::direct), eax at the call what the track's code happened to hold.
{0x431410, "a .col's chunks", kWatcom, 0, 0, kEax, -1, 0, kTrackCount},
{0x431420, "a .col's chunk", kWatcom, 2, 0, kEax, -1, 0, kTrackChunk},
{0x41e0f0, "the nearest block", kWatcom, 1, 0, kEax, -1, 0, kTrackNearest},
// Points into a view (vertices::transformPoints; decompiled_standins.inc has it too, writing
// nothing): written, as the lights behind a view read them back from their frames.
{0x4e0430, "points into a view", kStdcall, 4, 1, kEax, -1, 0, kTrackPoints},
// Matrices multiplied (vertices::multiplyMatrices; ai_race.inc has it after this, logging the
// second matrix and writing nothing): the product written, nothing logged past the three places --
// the second matrix is the frame's garbage where turnFromAngles hands it its own output.
{0x4e0280, "matrices multiplied", kWatcom, 3, 0, kEax, -1, 0, kTrackMatrix},
// trackdata's native that loadCollisions calls by address (the track's
// nodes taken from a .col chunk, with the .ccm and simhts.dat loaded): a
// stand-in, its header (eax) logged -- run for real it would load files from
// the cases' garbage.  Tested on its own (trackdata::openTrackNodes).
{0x41e370, "the track's nodes opened", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The port's sound rewrites the ambient sounds call (port_sound_effects.cpp),
// held to Watcom's convention as the port runs them -- the game's code they
// stand for leaves ebx and edx otherwise; each tested on its own.
{0x416fc0, "an effect in a slot", kStdcall, 4, 3, kEax, -1, 0, kNothing},
{0x416f60, "a slot stopped", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4185e0, "a side's pan", kWatcom, 2, 0, kEax, -1, 0, kNothing},
#elif defined(TRACK_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// shellOpen's: the file opened, its name's first 16 bytes logged.
{0x534480, "ShellExecuteA", kStdcall, 0, 6, kScratch, 6, 4, kSucceed},
#elif defined(TRACK_PROMISES)
// {"track::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, and the x87's stack and words.
{"track::shellOpen", true, true, 0, true, kNoHeap, true},
{"track::tintColours", true, true, 0, true, kNoHeap, true},
{"track::tintObjectList", true, true, 0, true, kNoHeap, true},
{"track::tintTrack", true, true, 0, true, kNoHeap, true},
{"track::greyColours", true, true, 0, true, kNoHeap, true},
{"track::greyObjectList", true, true, 0, true, kNoHeap, true},
{"track::greyTrack", true, true, 0, true, kNoHeap, true},
{"track::readObjectList", true, true, 0, true, kNoHeap, true},
{"track::readLights", true, true, 0, true, kNoHeap, true},
{"track::newArena", true, true, 0, true, kNoHeap, true},
{"track::freeArena", true, true, 0, true, kNoHeap, true},
{"track::arenaTake", true, true, 0, true, kNoHeap, true},
{"track::frdRead", true, true, 0, true, kNoHeap, true},
{"track::loadGeometry", true, true, 0, true, kNoHeap, true},
{"track::projectedTriangle", true, true, 0, true, kNoHeap, true},
{"track::shadeVertex", true, true, 0, true, kNoHeap, true},
{"track::setShadeDistances", true, true, 0, true, kNoHeap, true},
{"track::viewObjectVertices", true, true, 0, true, kNoHeap, true},
{"track::viewTurnedObjectVertices", true, true, 0, true, kNoHeap, true},
{"track::modelByDistance", true, true, 0, true, kNoHeap, true},
{"track::viewOnBlock", true, true, 0, true, kNoHeap, true},
{"track::lightObjectPolygons", true, true, 0, true, kNoHeap, true},
{"track::bracketList", true, true, 0, true, kNoHeap, true},
{"track::ambientSounds", true, true, 0, true, kNoHeap, true},
{"track::ambientWanted", true, true, 0, true, kNoHeap, true},
{"track::pointBehindView", true, true, 0, true, kNoHeap, true},
{"track::litBoxOutside", true, true, 0, true, kNoHeap, true},
{"track::flushBuckets", true, true, 0, true, kNoHeap, true},
{"track::headlightsOnObjects", true, true, 0, true, kNoHeap, true},
{"track::headlightsOnBlock", true, true, 0, true, kNoHeap, true},
{"track::blockLights", true, true, 0, true, kNoHeap, true},
{"track::drawObjectList", true, true, 0, true, kNoHeap, true},
{"track::drawTrack", true, true, 0, true, kNoHeap, true},
{"track::ambientSource", true, true, 0, true, kNoHeap, true},
{"track::selectView", true, true, 0, true, kNoHeap, true},
{"track::objectAnimation", true, true, 0, true, kNoHeap, true},
{"track::turnFromAngles", true, true, 0, true, kNoHeap, true},
{"track::turnFromPitchYaw", true, true, 0, true, kNoHeap, true},
{"track::setViewDistance", true, true, 0, true, kNoHeap, true},
{"track::nextViewDistance", true, true, 0, true, kNoHeap, true},
{"track::modelsUnlimited", true, true, 0, true, kNoHeap, true},
{"track::freeAt", true, true, 0, true, kNoHeap, true},
{"track::unloadTrack", true, true, 0, true, kNoHeap, true},
{"track::newPool", true, true, 0, true, kNoHeap, true},
{"track::newObjectPool", true, true, 0, true, kNoHeap, true},
{"track::freeTrackKinds", true, true, 0, true, kNoHeap, true},
{"track::loadTextures", true, true, 0, true, kNoHeap, true},
{"track::takeTextures", true, true, 0, true, kNoHeap, true},
{"track::loadCollisions", true, true, 0, true, kNoHeap, true},
{"track::freeCollisions", true, true, 0, true, kNoHeap, true},
{"track::clearObjectStates", true, true, 0, true, kNoHeap, true},
{"track::resetTrackViews", true, true, 0, true, kNoHeap, true},
{"track::resetViewsOuter", true, true, 0, true, kNoHeap, true},
{"track::resetViews", true, true, 0, true, kNoHeap, true},
{"track::blockInView", true, true, 0, true, kNoHeap, true},
#elif defined(TRACK_ARGUMENTS)
/* The track's world, where the cases keep it:
 *   0xd00000  the blocks' object lists' objects (0x400 a block, 0x100 a list), the track's at 0xd0f000
 *   0xd10000  the blocks' animated objects' records (0x100 a block); 0xd12000 sound sources, 0xd13000 lights
 *   0xd20000  the blocks' polygons (0x800 a block, 0x80 a model), objects' at 0xd28000 (0x80 each)
 *   0xd30000  the objects' colours (0x20 each), the blocks' at 0xd38000, vertices at 0xd3c000
 *   0xd40000  Render_GetTm's buffer: records of 0x1c bytes, each with a vertex at +8
 *   0xd48000  the .frd as loaded; 0xd50000 the arena; 0xd58000 the track's kinds' records
 *   0xd60000  the vertices records point at; 0xd61000 what the lights' kinds point at
 *   0xd70000  the .col's chunks (0x100 each); 0xd78000 cameras and lights; 0xd7a000 the cars (0x800 each)
 *   0xd7e000  lists, buckets and names handed in */
constexpr x86::reg32 kTrackBlocks = 0x571370, kTrackBlockSize = 0x5c0;
constexpr x86::reg32 kTrackCameras = 0xd78000, kTrackCars = 0xd7a000, kTrackMisc = 0xd7e000;

/* A float of the track's: within `range` either way mostly, 0 or anything
 * now and then. */
x86::reg32 aTrackFloat(float range)
{
    if (chance(5))
        return chance(50) ? 0 : chance(50) ? 0x80000000 : random32();
    const float f = (float(random32() % 20001) / 10000.0f - 1.0f) * range;
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

x86::reg32 aPositive(float range)
{
    const float f = float(random32() % 10001) / 10000.0f * range;
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return chance(4) ? 0 : bits;
}

/* An object list at `list` ({count, the objects}) of block `b`'s list `l`:
 * up to 3 objects of 0x34 bytes -- animated, a car's or fixed -- with their
 * vertices, colours and polygons. */
void makeTrackObjects(CheckApp& g, x86::reg32 list, x86::reg32 b, x86::reg32 l)
{
    const x86::reg32 count = chance(20) ? 0 : below(4);
    const x86::reg32 objects = 0xd00000 + b * 0x400 + l * 0x100;
    g.word(list) = count;
    g.word(list + 4) = objects;
    for (x86::reg32 o = 0; o < 4; ++o)
    {
        const x86::reg32 at = objects + o * 0x34, index = (b * 16 + l * 4 + o) & 0x7f;
        for (x86::reg32 i = 0; i < 0x34; i += 4)
            g.word(at + i) = below(4);
        g.word(at) = chance(35) ? 4 : chance(30) ? 3 : below(3);
        g.word(at + 4) = below(8);
        g.word(at + 8) = random32();
        for (x86::reg32 i = 0xc; i < 0x18; i += 4)
            g.word(at + i) = aTrackFloat(100);
        g.word(at + 0x1c) = below(8);
        g.word(at + 0x20) = below(9);
        g.word(at + 0x24) = 0xd3c000 + index * 0x80;
        g.word(at + 0x28) = 0xd30000 + index * 0x20;
        g.word(at + 0x2c) = below(5);
        g.word(at + 0x30) = 0xd28000 + index * 0x80;
    }
}

/* Polygons of 14 bytes at `at`: four corners' words, of the 16 first records
 * of Render_GetTm's buffer. */
void makeTrackPolygons(CheckApp& g, x86::reg32 at, x86::reg32 bytes)
{
    for (x86::reg32 i = 0; i + 2 <= bytes; i += 2)
        g.getMemory<x86::reg16>(at + i) = x86::reg16(below(16));
}

/* The track's world: blocks, views, cameras, cars, the objects' states, the
 * arena and the .frd, the buffers, the globals the natives read. */
void makeTrackDrawingWorld(CheckApp& g)
{
    // The code's data the headlights copy.
    static const x86::reg8 kCodeData[] = {
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xf5, 0x68,
        0x00, 0x00, 0xcc, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xcc, 0x8c, 0x00, 0x00, 0x66, 0x66, 0x00, 0x00};
    for (x86::reg32 i = 0; i < sizeof kCodeData; ++i)
        g.getMemory<x86::reg8>(0x41a880 + i) = kCodeData[i];
    // The cases' own: small numbers mostly.
    for (x86::reg32 at = 0xd00000; at < 0xd80000; at += 4)
        g.word(at) = chance(70) || (at >= 0xd48000 && at < 0xd58000) ? below(4) : random32();
    const x86::reg32 last = chance(10) ? 0 : below(8);
    g.word(0x5dd83c) = last;
    // A car's node (0x4217e0, trackdata.inc's row) as in trackdata's cases:
    // one of 0x40, so that the view's block (the node >> 3) is one of the
    // eight made here.
    g.word(0xd0fffc) = x86::reg32(g_seed);
    g.word(0x5dd940) = 0x40;
    // The blocks.
    for (x86::reg32 b = 0; b < 8; ++b)
    {
        const x86::reg32 block = kTrackBlocks + b * kTrackBlockSize;
        for (x86::reg32 i = 0; i < kTrackBlockSize; i += 4)
            g.word(block + i) = below(5);
        for (x86::reg32 m = 0; m < 11; ++m)
        {
            g.word(block + 4 * m) = below(5);
            g.word(block + 0x2c + 4 * m) = 0xd20000 + b * 0x800 + m * 0x80;
        }
        g.word(block + 0x58) = below(9);
        for (x86::reg32 m = 0; m < 5; ++m)
            g.word(block + 0x5c + 4 * m) = below(9);
        g.word(block + 0x70) = 0xd3c000 + 0x80 * (0x80 + b);
        g.word(block + 0x74) = 0xd38000 + b * 0x20;
        for (x86::reg32 i = 0x78; i < 0xb4; i += 4)
            g.word(block + i) = aTrackFloat(100);
        for (x86::reg32 i = 0; i < 300; ++i)
            g.getMemory<x86::reg16>(block + 0xb4 + 2 * i) =
                x86::reg16(chance(chance(10) ? 50 : 97) ? 0xffff : below(last + 1));
        for (x86::reg32 l = 0; l < 4; ++l)
            makeTrackObjects(g, block + 0x564 + 8 * l, b, l);
        g.word(block + 0x5a4) = 0xd10000 + b * 0x100;
        g.word(block + 0x5b0) = below(4);
        g.word(block + 0x5b4) = 0xd12000 + b * 0x40;
        g.word(block + 0x5b8) = below(4);
        g.word(block + 0x5bc) = 0xd13000 + b * 0x40;
        // Its animated objects' records: a state's number, a kind.
        for (x86::reg32 o = 0; o < 8; ++o)
        {
            const x86::reg32 at = 0xd10000 + b * 0x100 + o * 0x14;
            g.getMemory<x86::reg16>(at + 0xe) = x86::reg16(chance(80) ? below(0x10) : below(0x200));
            g.getMemory<x86::reg8>(at + 0x13) = x86::reg8(chance(60) ? 2 : chance(50) ? 3 : below(5));
        }
        // Its sound sources: a sound, a kind (1 mostly), a place.
        for (x86::reg32 s = 0; s < 4; ++s)
        {
            const x86::reg32 at = 0xd12000 + b * 0x40 + s * 0x10;
            g.word(at) = below(0x1000000) - 0x800000;
            g.word(at + 4) = below(0x1000000) - 0x800000;
            g.word(at + 8) = below(0x1000000) - 0x800000;
            g.getMemory<x86::reg16>(at + 0xc) = x86::reg16(below(0x40));
            g.getMemory<x86::reg16>(at + 0xe) = x86::reg16(chance(50) ? 1 : below(4));
        }
        makeTrackPolygons(g, 0xd20000 + b * 0x800, 0x800);
    }
    makeTrackPolygons(g, 0xd28000, 0x4000);
    makeTrackObjects(g, 0x5dd070, 15, 3);
    // The objects' colours and vertices.
    for (x86::reg32 at = 0xd30000; at < 0xd40000; at += 4)
        g.word(at) = (at >= 0xd3c000) ? aTrackFloat(100) : random32();
    // Render_GetTm's buffer: records of 0x1c bytes, their vertices among the
    // cases', lit (+4, +6) now and then.
    for (x86::reg32 i = 0; i < 0x400; ++i)
    {
        const x86::reg32 at = 0xd40000 + 0x1c * i;
        g.word(at) = random32();
        g.word(at + 4) = (chance(40) ? 0 : random32() & 0xffff) | (chance(40) ? 0 : random32() << 16);
        g.word(at + 8) = 0xd60000 + 0x20 * below(0x80);
        for (x86::reg32 k = 0xc; k < 0x1c; k += 4)
            g.word(at + k) = random32();
    }
    // The vertices those point at: a place, a depth at +8, the screen's
    // outcode at +0x14, colours at +0x10..+0x12.
    for (x86::reg32 at = 0xd60000; at < 0xd61000; at += 0x20)
    {
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            g.word(at + k) = random32();
        g.word(at + 8) = aTrackFloat(300);
        g.getMemory<x86::reg8>(at + 0x14) = x86::reg8(chance(60) ? 0 : below(0x20));
    }
    // What the lights' kinds point at, and their table.
    for (x86::reg32 at = 0x8b41a0; at < 0x8b5000; at += 4)
        g.word(at) = 0xd61000 + 4 * below(0x400);
    g.word(0x5dd858) = 0xd58000;
    // The .frd as loaded, the arena.
    g.word(0x5dd078) = 0xd48000 + 4 * below(0x100);
    g.word(0x5dd07c) = random32();
    g.word(0x552dfc) = 0xd50000;
    g.word(0x552e00) = chance(85) ? 4 * below(0x800) : 0x310000;
    g.word(0x552df8) = below(6);
    g.word(0x552df4) = 0xd50000 + 4 * below(0x100);
    // The views; the one drawn.
    for (x86::reg32 v = 0; v < 4; ++v)
    {
        const x86::reg32 view = 0x5dd0e0 + v * 0x54;
        for (x86::reg32 i = 0; i < 0x54; i += 4)
            g.word(view + i) = below(16);
        g.word(view) = chance(80) ? below(last + 1) : 0xffffffff;
        g.word(view + 0x30) = aPositive(400);
        g.word(view + 0x34) = aPositive(chance(50) ? 150 : 400);
        g.word(view + 0x40) = aPositive(100);
        g.word(view + 0x44) = aPositive(200);
    }
    g.word(0x5dd830) = 0x5dd0e0 + 0x54 * below(4);
    g.word(0x5dd0d8) = random32();
    g.word(0x5dd0dc) = random32();
    g.word(0x7a3d14) = chance(70) ? 0 : chance(70) ? 1 : 2;
    // The models by distance: the block's word of the model, the model, the
    // most polygons before the buckets are flushed.
    for (x86::reg32 m = 0; m < 3; ++m)
    {
        g.word(0x552e20 + 12 * m) = below(5);
        g.word(0x552e20 + 12 * m + 4) = chance(30) ? 4 : below(10);
        g.word(0x552e20 + 12 * m + 8) = chance(10) ? 0xffffffff : below(30);
    }
    // Cameras and lights: a kind (1 the mirror's pass), a place, a box, a
    // matrix, a light's distances.
    for (x86::reg32 c = 0; c < 8; ++c)
    {
        const x86::reg32 at = kTrackCameras + c * 0x100;
        for (x86::reg32 i = 0; i < 0x100; i += 4)
            g.word(at + i) = aTrackFloat(chance(50) ? 2 : 100);
        g.word(at) = chance(60) ? 0 : chance(50) ? 1 : below(4);
        g.word(at + 4) = chance(90) ? below(4) : below(0x10);
        g.word(at + 0x64) = random32();
        g.word(at + 0x70) = 0xd3c000 + 0x80 * below(0x100);
    }
    // The cars: their flags, place, speed, lights.
    const x86::reg32 cars = chance(10) ? 0 : 1 + below(4);
    g.word(0x5efd9c) = cars;
    for (x86::reg32 c = 0; c < 4; ++c)
    {
        const x86::reg32 car = kTrackCars + c * 0x800;
        g.word(0x5efac8 + 4 * c) = car;
        for (x86::reg32 i = 0; i < 0x800; i += 4)
            g.word(car + i) = aTrackFloat(chance(50) ? 2 : 100);
        g.word(car + 0xc) = kTrackBlocks + below(8) * kTrackBlockSize;
        g.getMemory<x86::reg8>(car + 0x200) = x86::reg8((chance(10) ? 0x40 : 0) | (chance(70) ? 0x20 : 0) | (chance(50) ? 2 : 0));
        g.getMemory<x86::reg8>(car + 0x88) = x86::reg8(chance(85) ? 0 : 1);
        g.getMemory<x86::reg8>(car + 0x89) = x86::reg8(chance(85) ? 1 : 0);
        for (x86::reg32 i = 0x98; i <= 0xa0; i += 4)
            g.word(car + i) = aTrackFloat(100);
        g.word(car + 0xa4) = aTrackFloat(40);
        g.word(car + 0xac) = aTrackFloat(40);
        g.word(car + 0x734) = chance(85) ? random32() | 1 : 0;
        g.word(car + 0x660) = chance(85) ? random32() | 1 : 0;
    }
    // The objects' states: still, turning, or on a path.
    for (x86::reg32 i = 0; i < 0x600; ++i)
        g.getMemory<x86::reg8>(0x5dd230 + i) =
            x86::reg8(i % 3 != 0 ? random32() : chance(40) ? 0xff : chance(35) ? 0xfe : random32());
    // The .col's chunks: a kind, a count, vertices after the head.
    for (x86::reg32 c = 0; c < 8; ++c)
    {
        const x86::reg32 at = 0xd70000 + c * 0x100;
        static const x86::reg16 kinds[] = {2, 2, 7, 8, 0xf, 1, 9, 0};
        g.getMemory<x86::reg16>(at + 4) = kinds[below(8)];
        g.getMemory<x86::reg16>(at + 6) = x86::reg16(below(0x1e));
    }
    // The globals.
    for (x86::reg32 i = 0; i < 3; ++i)
        g.word(0x55d148 + 4 * i) = chance(90) ? below(150) : random32();
    g.getMemory<x86::reg8>(0x7a3a58) = x86::reg8(random32());
    g.getMemory<x86::reg8>(0x7a3a59) = x86::reg8(chance(70) ? 0 : random32());
    g.word(0x7a3a60) = chance(70) ? 1 : 0;
    g.word(0x6fd4c8) = chance(50) ? 0 : 1;
    g.word(0x6fd4cc) = chance(70) ? 0 : 1;
    g.word(0x6fd4ac) = chance(40) ? 0 : below(20);
    g.word(0x6fd4c4) = chance(50) ? 0 : 1;
    g.word(0x6fbc38) = aPositive(1.3f);
    g.word(0x6fbc28) = chance(90) ? below(4) : random32();
    for (const x86::reg32 at : {0x552e04u, 0x552e08u, 0x552e0cu, 0x552e10u})
        g.word(at) = aPositive(100);
    g.word(0x552e44) = below(8);
    g.word(0x552e18) = chance(50) ? 0 : 0x1f0000 + 0x400 * below(8);
    g.word(0x5dd840) = chance(50) ? 0 : 0xd70000;
    g.word(0x5dd844) = random32();
    g.word(0x5dd850) = chance(50) ? 0 : 0x1f0000 + 0x400 * below(8);
    g.word(0x5dd854) = random32();
    g.word(0x5dd834) = random32();
    g.word(0x5f1fa8) = random32();
    g.word(0x55e88c) = chance(85) ? 0 : 1;
    for (x86::reg32 at = 0x7d34f8; at <= 0x7d350c; at += 4)
        g.word(at) = aPositive(800);
    g.word(0x7d3684) = random32();
    for (x86::reg32 at = 0x7dcfe4; at <= 0x7dcff8; at += 4)
        g.word(at) = random32();
}

bool makeTrackArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeTrackDrawingWorld(g);
    const auto camera = [] { return kTrackCameras + 0x100 * below(8); };
    const auto aBlock = [&] { return below(g.word(0x5dd83c) + 1); };
    const x86::reg32 misc = kTrackMisc;
    if (name == "shellOpen")
        cpu.eax = kStrings + 0x40 * below(64);
    else if (name == "tintColours" || name == "greyColours")
    {
        cpu.eax = chance(10) ? x86::reg32(-x86::sreg32(below(3))) : below(9);
        cpu.edx = 0xd30000 + 0x20 * below(0x80);
    }
    else if (name == "tintObjectList" || name == "greyObjectList")
        cpu.eax = chance(10) ? 0x5dd070 : kTrackBlocks + below(8) * kTrackBlockSize + 0x564 + 8 * below(4);
    else if (name == "readObjectList")
        cpu.eax = misc + 4 * below(0x40);
    else if (name == "arenaTake")
    {
        // Now and then just what is left of the arena, a byte either way.
        const x86::reg32 used = g.word(0x552e00);
        cpu.eax = chance(30) ? 0x310000 - used + below(3) - 1 : chance(80) ? below(0x2000) : random32();
    }
    else if (name == "frdRead")
    {
        cpu.eax = misc + 4 * below(0x100);
        cpu.edx = chance(90) ? below(0x400) : random32();
    }
    else if (name == "projectedTriangle")
    {
        // The records' place, then three corners of 7 words: a word, its
        // vertex, two words, a colour.
        g.word(misc) = 0xd40000 + 0x1c * below(0x100);
        cpu.eax = misc;
        // Past the nine words makeArguments writes, straight onto the stack.
        const auto word = [&](x86::reg32 i, x86::reg32 value) {
            if (i < 9)
                stack[i] = value;
            else
                g.word(cpu.esp + 4 + 4 * i) = value;
        };
        for (x86::reg32 k = 0; k < 3; ++k)
        {
            word(7 * k, random32());
            word(7 * k + 1, random32());
            word(7 * k + 2, 0xd60000 + 0x20 * below(0x80));
            word(7 * k + 3, random32());
            word(7 * k + 4, random32());
            word(7 * k + 5, chance(30) ? below(0x8000000) : random32());
            word(7 * k + 6, random32());
        }
    }
    else if (name == "shadeVertex")
    {
        stack[0] = aTrackFloat(chance(50) ? 2 : 300);
        stack[1] = aTrackFloat(chance(50) ? 2 : 300);
        stack[2] = aTrackFloat(300);
        stack[3] = random32();
        stack[4] = misc + 4 * below(0x40);
    }
    else if (name == "setShadeDistances")
    {
        cpu.eax = camera();
        stack[0] = aTrackFloat(2);
    }
    else if (name == "viewObjectVertices" || name == "viewTurnedObjectVertices")
    {
        cpu.eax = camera();
        cpu.ebx = camera() + 4 * below(0x30);
        cpu.ecx = camera() + 4 * below(0x30);
        cpu.edx = name == "viewObjectVertices" ? below(9) : camera() + 4 * below(0x30);
    }
    else if (name == "modelByDistance")
    {
        cpu.eax = camera();
        stack[0] = aPositive(chance(50) ? 2000 : 40000);
    }
    else if (name == "viewOnBlock" || name == "ambientSounds" || name == "drawTrack")
    {
        cpu.eax = camera();
        cpu.edx = name == "drawTrack" ? kTrackBlocks + aBlock() * kTrackBlockSize + 0xb4 : below(4);
        cpu.ebx = misc + 0x100;
    }
    else if (name == "lightObjectPolygons")
    {
        cpu.ebx = aBlock();
        cpu.ecx = below(11);
        stack[0] = 0xd40000;
    }
    else if (name == "bracketList")
    {
        g.word(misc) = chance(25) ? 0 : random32();
        cpu.eax = misc;
    }
    else if (name == "pointBehindView")
    {
        cpu.eax = camera();
        cpu.edx = camera();
    }
    else if (name == "litBoxOutside")
    {
        cpu.edx = camera();
        cpu.ebx = camera() + 0x44;
        cpu.ecx = camera() + 8;
    }
    else if (name == "flushBuckets")
    {
        for (x86::reg32 i = 0; i < 6; ++i)
        {
            g.word(misc + 0x200 + 4 * i) = chance(40) ? 0 : 0xd7f000 + 0x40 * i;
            g.word(misc + 0x220 + 4 * i) = 0xd7f800 + 0x40 * i;
        }
        cpu.eax = camera();
        cpu.ebx = misc + 0x200;
        cpu.edx = misc + 0x220;
        cpu.ecx = misc + 0x300;
    }
    else if (name == "headlightsOnObjects")
    {
        cpu.eax = below(5);
        cpu.edx = chance(30) ? 0 : camera() + 0x44;
        cpu.ebx = chance(30) ? 0 : camera() + 8;
        cpu.ecx = camera() + 8;
        stack[0] = 0xd40000;
        stack[1] = camera();
        stack[2] = kTrackBlocks + aBlock() * kTrackBlockSize;
    }
    else if (name == "headlightsOnBlock")
    {
        cpu.eax = camera();
        cpu.edx = below(9);
        cpu.ebx = aBlock();
        cpu.ecx = below(9);
        stack[0] = misc + 0x200;
        stack[1] = misc + 0x220;
        stack[2] = kTrackBlocks + cpu.ebx * kTrackBlockSize;
        stack[3] = chance(30) ? 4 : below(10);
    }
    else if (name == "blockLights" || name == "blockInView")
    {
        cpu.eax = camera();
        cpu.edx = aBlock();
    }
    else if (name == "drawObjectList")
    {
        const x86::reg32 b = aBlock();
        g.word(misc + 0x400) = chance(30) ? 0 : 0xd7f000;
        cpu.eax = camera();
        cpu.edx = misc + 0x400;
        cpu.ebx = misc + 0x404;
        cpu.ecx = b;
        stack[0] = chance(50) ? 0xbf800000 : aTrackFloat(2);
        stack[1] = chance(10) ? 0x5dd070 : kTrackBlocks + b * kTrackBlockSize + 0x564 + 8 * below(4);
        stack[2] = below(2);
    }
    else if (name == "ambientSource")
    {
        cpu.eax = camera();
        cpu.edx = 0xd12000 + 0x10 * below(0x20);
        cpu.ebx = below(2);
    }
    else if (name == "selectView")
        cpu.eax = below(4);
    else if (name == "objectAnimation")
    {
        cpu.eax = kTrackCars + 0x800 * below(4);
        cpu.edx = below(8);
    }
    else if (name == "turnFromAngles" || name == "turnFromPitchYaw")
    {
        cpu.eax = random32() % 0x20000 - 0x10000;
        cpu.ebx = name == "turnFromAngles" ? random32() % 0x20000 - 0x10000 : misc + 0x500;
        cpu.edx = random32() % 0x20000 - 0x10000;
        cpu.ecx = misc + 0x540;
    }
    else if (name == "setViewDistance")
        cpu.eax = below(4);
    else if (name == "freeAt")
    {
        g.word(misc) = chance(30) ? 0 : 0x1f0000 + 0x400 * below(8);
        cpu.eax = misc;
    }
    else if (name == "newPool")
    {
        cpu.eax = misc + 0x600;
        cpu.edx = below(0x1000);
    }
    else if (name == "loadTextures")
    {
        cpu.eax = random32();
        cpu.edx = misc + 0x700;
    }
    else if (name == "loadCollisions")
        cpu.eax = kStrings + 0x40 * below(64);
    return true;
}
#endif
