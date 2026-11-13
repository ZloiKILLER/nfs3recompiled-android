// game_checks.cpp's parts for the collisions, the COLL file, the render
// states, the software blend, the polygon lists' drawers, the race's pointer
// and messages (native_collide.cpp), included in each table with the macro
// naming the part.
#if defined(COLLIDE_WINDOWS)
// The collisions' globals (0x5efd88 to 0x5eff20), the blend tables and the
// drawers' table (0x5f0780 to 0x5f1fac), the cars' list's count and the
// points (0x5537bc on), the wall test's tables in the code (0x42dd30), the
// polygons' records' pointer (0x5dd948), the controls' state (0x79c4a5), the
// screen's clip edges (0x7d34f8), the pointers' records (0x8b41dc), the fog
// colour (0x6ff9f8), and the cases' own (0xd40000 on).
    {0x5ef000, 0x5f2000}, {0x553000, 0x554000}, {0x42d000, 0x42e000}, {0x5dd000, 0x5de000},
    {0x79c000, 0x79d000}, {0x7d3000, 0x7d4000}, {0x8b4000, 0x8b5000}, {0x6ff000, 0x700000},
    {0xd40000, 0xd60000},
#elif defined(COLLIDE_EFFECTS)
    kCollideHit,     // a box's test (0x42dd60 and its like): a contact now and then, else 0
    kCollideLength,  // a vector normalized (0x4972f0): its length onto the x87's stack, 0 now and then
    kCollideCount,   // the track's objects near a car (0x41ff10): a few
    kCollideHow,     // how an object is hit (0x41d350): 0, 1, 2 or -1
    kCollideNext,    // a polygon list's drawer: the next record, or 0
    kCollideObject,  // a track's object (0x41ff20): its position near the cars to [ebx], its size to [ecx]
    kCollidePlace,   // a place on the track found (0x41f6e0, 0x41f870): its polygon's fields to the record
    kCollideEdges,   // a place's quad's edges (0x41fd60): bits 1 to 8 mostly, a height to [ecx]
#elif defined(COLLIDE_EFFECT_CASES)
        case kCollideHit:
            result = r % 5 == 0 ? 1 + (r >> 8) % 3 : 0;
            break;
        case kCollideLength:
            cpu.fpu.count += 1;
            cpu.fpu.st(0) = r % 5 == 0 ? x86::Float(0.0) : x86::Float(double(r % 2000) / 64.0);
            break;
        case kCollideCount:
            result = r % 6;
            break;
        case kCollideHow:
        {
            static const x86::reg32 hows[] = {0, 1, 1, 2, 2, 0xffffffff, 7};
            result = hows[r % 7];
            break;
        }
        case kCollideObject:
            // 16.16, within 8 of the cars mostly; a size up to 4 (a word, / 0x100).
            for (x86::reg32 i = 0; i < 3; ++i)
                if (words[2] + 4 * i < kMemory - 4)
                    word(words[2] + 4 * i) = random(id + 1 + i) % 8 == 0 ? random(id + 4 + i)
                                           : x86::reg32(x86::sreg32(random(id + 1 + i) % 0x100000) - 0x80000);
            if (words[3] < kMemory - 4)
                word(words[3]) = (random(id + 7) & 0xffff0000) | (r % 9 == 0 ? r >> 16 : r % 0x400);
            break;
        case kCollidePlace:
        {
            // The record (edx for 0x41f6e0, the place copied in at +4; ebx
            // for 0x41f870): what clearOfWall reads -- the polygon's index,
            // whether its corners are its own, its kind, something of its
            // own, its corners (16.16).
            const x86::reg32 place = s.address == 0x41f870 ? words[2] : words[1];
            if (place < kMemory - 0x80)
            {
                if (s.address == 0x41f6e0 && words[0] < kMemory - 0xc)
                    for (x86::reg32 i = 0; i < 3; ++i)
                        word(place + 4 + 4 * i) = word(words[0] + 4 * i);
                word(place + 0x10) = random(id + 1) % 64;
                word(place + 0x14) = random(id + 2) % 2 ? 0 : random(id + 3);
                word(place + 0x18) = random(id + 4) % 10 == 0 ? 0 : random(id + 5);
                word(place + 0x6c) = random(id + 6) % 10 < 7 ? 0 : random(id + 7);
                for (x86::reg32 i = 0; i < 15; ++i)
                    word(place + 0x24 + 4 * i) = x86::reg32(x86::sreg32(random(id + 8 + i) % 0x80000) - 0x40000);
            }
            result = r % 3 == 0 ? 0 : 1;
            break;
        }
        case kCollideEdges:
        {
            static const x86::reg32 edges[] = {0, 1, 2, 4, 8, 3, 5, 0xa, 0xf};
            result = edges[r % 9];
            if (words[3] < kMemory - 4)
                word(words[3]) = x86::reg32(x86::sreg32((r >> 8) % 0x80000) - 0x40000);
            break;
        }
        case kCollideNext:
            result = r % 3 == 0 ? 0 : 0xd58000 + 0x40 * ((r >> 8) % 8);
            break;
#elif defined(COLLIDE_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The boxes' tests findContact makes, each a point's signs in ebx, ecx and
// on the stack (ret 4).
{0x42dd60, "a face's centre in the first box", kStdcall, 4, 1, kEax, -1, 0, kCollideHit},
{0x42e010, "a face's centre in the second box", kStdcall, 4, 1, kEax, -1, 0, kCollideHit},
{0x42e2c0, "a corner in the first box", kStdcall, 4, 1, kEax, -1, 0, kCollideHit},
{0x42e4f0, "a corner in the second box", kStdcall, 4, 1, kEax, -1, 0, kCollideHit},
// A vector normalized: its length back on the x87's stack (the dot product,
// 0x4e01f0, is sound_lib.inc's).
{0x4972f0, "a vector normalized", kWatcom, 1, 0, 0, 0, 3, kCollideLength},
// The track's objects near a car, and how one is hit.
{0x41ff10, "the objects near", kWatcom, 1, 0, kEax, -1, 0, kCollideCount},
{0x41d350, "how an object is hit", kWatcom, 2, 0, kEax, -1, 0, kCollideHow},
{0x41ff20, "an object near", kWatcom, 4, 0, kEax, -1, 0, kCollideObject},
// A place on the track found again for a corner (the place's three words
// logged), its quad's edges it is past (the place's), the neighbour an edge
// leads to: trackdata's, which walk the track's data -- stood in for.
{0x41f6e0, "a place on the track followed", kWatcom, 3, 0, kEax, 0, 3, kCollidePlace},
{0x41fd60, "the edges a place is past", kWatcom, 4, 0, kEax, 0, 3, kCollideEdges},
{0x41f870, "the quad an edge leads to", kWatcom, 3, 0, kEax, -1, 0, kCollidePlace},
// The wall's surface by its corner (its three floats at ecx logged), and a
// quad drawn (its first corner's eight words logged).
{0x42ee10, "a wall's surface", kWatcom, 4, 0, kEax, 3, 3, kNothing},
{0x4c12e0, "a quad drawn", kWatcom, 4, 0, kEax, 0, 8, kNothing},
#elif defined(COLLIDE_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// A car's own controls (its +0x5fc), and a polygon list's drawer (the table
// at 0x5f0780), slots of the cases' own.
{0xd5ff00, "a car's own controls", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0xd5ff04, "a polygon list's drawer", kWatcom, 1, 0, kEax, 0, 2, kCollideNext},
#elif defined(COLLIDE_PROMISES)
// {"collide::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, and the x87's stack in use, its words.
{"collide::findContact", true, true, 0, true, kNoHeap, true},
{"collide::contactNormal", true, true, 0, true, kNoHeap, true},
{"collide::pushCarsApart", true, true, 0, true, kNoHeap, true},
{"collide::separateCars", true, true, 0, true, kNoHeap, true},
{"collide::polygonEdgeNormal", true, true, 0, true, kNoHeap, true},
{"collide::clearOfWall", true, true, 0, true, kNoHeap, true},
{"collide::carAgainstWalls", true, true, 0, true, kNoHeap, true},
{"collide::collideCars", true, true, 0, true, kNoHeap, true},
{"collide::clearCarList", true, true, 0, true, kNoHeap, true},
{"collide::objectInReach", true, true, 0, true, kNoHeap, true},
{"collide::carAgainstObjects", true, true, 0, true, kNoHeap, true},
{"collide::addCarToList", true, true, 0, true, kNoHeap, true},
{"collide::carCollisions", true, true, 0, true, kNoHeap, true},
{"collide::distanceSquared", true, true, 0, true, kNoHeap, true},
{"collide::nearestPoint", true, true, 0, true, kNoHeap, true},
{"collide::openCollFile", true, true, 0, true, kNoHeap, true},
{"collide::closeCollFile", true, true, 0, true, kNoHeap, true},
{"collide::collFileSize", true, true, 0, true, kNoHeap, true},
{"collide::readCollFile", true, true, 0, true, kNoHeap, true},
{"collide::collFileCount", true, true, 0, true, kNoHeap, true},
{"collide::collChunk", true, true, 0, true, kNoHeap, true},
{"collide::playerControls", true, true, 0, true, kNoHeap, true},
{"collide::forgetRenderStates", true, true, 0, true, kNoHeap, true},
{"collide::statesForText", true, true, 0, true, kNoHeap, true},
{"collide::statesForPictures", true, true, 0, true, kNoHeap, true},
{"collide::statesForHud", true, true, 0, true, kNoHeap, true},
{"collide::statesForScreens", true, true, 0, true, kNoHeap, true},
{"collide::passLayerNothing", true, true, 0, true, kNoHeap, true},
{"collide::passSecondLayer", true, true, 0, true, kNoHeap, true},
{"collide::passFirstLayer", true, true, 0, true, kNoHeap, true},
{"collide::makeBlendTables", true, true, 0, true, kNoHeap, true},
{"collide::blendSpan", true, true, 0, true, kNoHeap, true},
{"collide::subdivisionDepth", true, true, 0, true, kNoHeap, true},
{"collide::drawTexturedTriangles", true, true, 0, true, kNoHeap, true},
{"collide::drawFlatTriangles", true, true, 0, true, kNoHeap, true},
{"collide::drawTrianglesByZ", true, true, 0, true, kNoHeap, true},
{"collide::drawQuadsByZ", true, true, 0, true, kNoHeap, true},
{"collide::drawFlatTrianglesAgain", true, true, 0, true, kNoHeap, true},
{"collide::drawColouredTriangles", true, true, 0, true, kNoHeap, true},
{"collide::stateRecord", true, true, 0, true, kNoHeap, true},
{"collide::drawRecords", true, true, 0, true, kNoHeap, true},
{"collide::raceTextFrame", true, true, 0, true, kNoHeap, true},
{"collide::setMirrored", true, true, 0, true, kNoHeap, true},
{"collide::drawMirroredRecords", true, true, 0, true, kNoHeap, true},
{"collide::countFrame", true, true, 0, true, kNoHeap, true},
{"collide::raceViewDistance", true, true, 0, true, kNoHeap, true},
{"collide::raceFrameEnd", true, true, 0, true, kNoHeap, true},
{"collide::raceFrameNothing", true, true, 0, true, kNoHeap, true},
{"collide::raceEndNothing", true, true, 0, true, kNoHeap, true},
{"collide::raceStartDrawing", true, true, 0, true, kNoHeap, true},
{"collide::raceStartNothing", true, true, 0, true, kNoHeap, true},
{"collide::drawPointer", true, true, 0, true, kNoHeap, true},
{"collide::openMessages", true, true, 0, true, kNoHeap, true},
{"collide::closeMessages", true, true, 0, true, kNoHeap, true},
{"collide::clearAllMessages", true, true, 0, true, kNoHeap, true},
{"collide::clearMessages", true, true, 0, true, kNoHeap, true},
{"collide::clearMessageLine", true, true, 0, true, kNoHeap, true},
{"collide::addMessageText", true, true, 0, true, kNoHeap, true},
{"collide::addMessageCharacter", true, true, 0, true, kNoHeap, true},
{"collide::copyMessageLines", true, true, 0, true, kNoHeap, true},
{"collide::messagesScrolled", true, true, 0, true, kNoHeap, true},
{"collide::messagesNothing", true, true, 0, true, kNoHeap, true},
{"collide::seedHash", true, true, 0, true, kNoHeap, true},
#elif defined(COLLIDE_ARGUMENTS)
/* Where the collisions' cases keep their own: four cars' records (0x800
 * each), the track polygons' records, the walls' track positions,
 * buffers, the drawers' records and corners, the messages' blocks. */
constexpr x86::reg32 kCollideCars = 0xd40000, kCollidePolygons = 0xd48000, kCollideOut = 0xd4f000;
constexpr x86::reg32 kCollideRecords = 0xd58000, kCollideCorners = 0xd59000, kCollideTextures = 0xd5a000;
constexpr x86::reg32 kCollideTables = 0xd5b000, kCollideMessages = 0xd50000, kCollideCopy = 0xd53000;
constexpr x86::reg32 kCollideText = 0xd54000, kCollideEnd = 0xd60000;

/* A float's bits: a sensible one mostly -- within `range` either side --
 * now and then any. */
x86::reg32 aCollideFloat(float range)
{
    if (chance(3))
        return anyFloatBits(random32() | std::uint64_t(random32()) << 32);
    const float f = (float(below(20001)) / 10000.0f - 1.0f) * range;
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

void putCollideFloat(CheckApp& g, x86::reg32 at, float f)
{
    std::memcpy(&g.getMemory<x86::reg8>(at), &f, 4);
}

x86::reg32 aCar()
{
    return kCollideCars + 0x800 * below(4);
}

/* A car's record: the words random, then what the collisions read --
 * position near the others', axes about unit, half sizes, mass's share,
 * speed and turn, radii, its track position (corners in 16.16 near the
 * car, a polygon's index), flags, the controls' slot. */
void makeCollideCar(CheckApp& g, x86::reg32 car)
{
    for (x86::reg32 at = car; at < car + 0x800; at += 4)
        g.word(at) = chance(50) ? aCollideFloat(10) : random32();
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g.word(car + 0x98 + 4 * i) = aCollideFloat(chance(70) ? 4 : 40);
        g.word(car + 0xa4 + 4 * i) = aCollideFloat(chance(50) ? 5 : 60);
        g.word(car + 0xe4 + 4 * i) = aCollideFloat(3);
        // Nor a NaN in the velocity or the spin: the cars' response
        // (raceload::carsCollide's impulses) multiplies the point's speed's
        // NaN by another, whose bits the host's compiler picks (an infinity
        // instead, the NaNs it makes all of one kind).
        for (const x86::reg32 at : {car + 0xa4 + 4 * i, car + 0xe4 + 4 * i})
            if ((g.word(at) & 0x7f800000) == 0x7f800000)
                g.word(at) &= 0xff800000;
        // Never a NaN: a NaN times a NaN (a dot product's) takes either one's
        // bits as the host's compiler orders the product, on either side.
        g.word(car + 0x114 + 4 * i) = chance(95) ? aCollideFloat(1) & 0x7fffffff : aCollideFloat(3);
        if ((g.word(car + 0x114 + 4 * i) & 0x7f800000) == 0x7f800000)
            g.word(car + 0x114 + 4 * i) &= 0xff800000;
        if (!chance(3))
            putCollideFloat(g, car + 0x114 + 4 * i, 0.5f + float(below(300)) / 100.0f);
    }
    for (x86::reg32 i = 0; i < 9; ++i)
        g.word(car + 0xc0 + 4 * i) = aCollideFloat(1);
    putCollideFloat(g, car + 0xb0, 0.2f + float(below(200)) / 100.0f);
    putCollideFloat(g, car + 0x120, float(below(600)) / 100.0f);
    g.word(car + 0x15c) = aCollideFloat(4);
    g.word(car + 0xb8) = aCollideFloat(5);
    g.word(car + 0x168) = chance(30) ? 0 : aCollideFloat(20);
    g.word(car + 0x174) = chance(60) ? 0 : below(4);
    g.getMemory<x86::reg8>(car + 0x88) = x86::reg8(chance(80) ? 0 : 1);
    g.getMemory<x86::reg8>(car + 0x89) = x86::reg8(chance(80) ? 1 : 0);
    g.word(car + 0x188) = random32();
    g.word(car + 0x200) = random32();
    g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(50) ? 0 : 1 + below(3));
    g.word(car + 0x1c) = below(64);
    g.word(car + 0x1f0) = below(8);
    g.word(car + 0x528) = kCollideOut + 0x40 * below(8);
    g.word(car + 0x5fc) = standInFor(0xd5ff00);
    g.word(car + 0x660) = below(6);
    // The track position: the polygon's own corners or another's, walls or not.
    g.word(car + 0xc + 0x10) = below(64);
    g.word(car + 0xc + 0x14) = chance(50) ? 0 : random32();
    g.word(car + 0xc + 0x18) = chance(10) ? 0 : random32();
    g.word(car + 0xc + 0x6c) = chance(70) ? 0 : random32();
    for (x86::reg32 at = car + 0x24; at < car + 0x60; at += 4)
        g.word(at) = x86::reg32(x86::sreg32(below(0x80000)) - 0x40000);
}

void makeCollideWorld(CheckApp& g)
{
    for (x86::reg32 car = kCollideCars; car < kCollideCars + 0x2000; car += 0x800)
        makeCollideCar(g, car);
    for (x86::reg32 at = kCollidePolygons; at < kCollidePolygons + 0x24 * 64 + 0x40; at += 4)
        g.word(at) = chance(50) ? x86::reg32(x86::sreg32(below(0x80000)) - 0x40000) : random32();
    g.word(0x5dd948) = kCollidePolygons;
    for (x86::reg32 at = kCollideOut; at < kCollideRecords; at += 4)
        g.word(at) = aCollideFloat(10);
    // The collisions' globals.
    for (x86::reg32 at = 0x5efe60; at < 0x5eff20; at += 4)
        g.word(at) = aCollideFloat(chance(80) ? 3 : 30);
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x5efe10 + 4 * i) = aCar();
    g.word(0x5537bc) = below(4);
    for (const x86::reg32 at : {0x5efd98u, 0x5efd88u, 0x6fd494u})
        g.word(at) = chance(70) ? 0 : x86::reg32(x86::sreg32(below(5)) - 2);
    // Mostly the dot products as a race has them (handling.inc's mark: its
    // dot product's stand-in then hands back -40 to 40, objects come within
    // reach), now and then any.
    if (chance(70))
        g.word(0xd13ffc) = x86::reg32(g_seed);
    // The wall test's tables, as nfs3.exe's code has them.
    static const x86::reg32 tables[] = {0, 0, 0, 0, 0, 0, 4, 8, 1, 2};
    for (x86::reg32 i = 0; i < 10; ++i)
        g.word(0x42dd30 + 4 * i) = tables[i];
}

/* The drawers' records: a chain of a few, each its kind (+4: 0 a quad, 1 a
 * triangle, now and then another) and flags (+6), its corners, its texture
 * or none (+0x18: its +4 the handle, its coordinates, its flags +0x28). */
x86::reg32 makeDrawerRecords(CheckApp& g, x86::reg16 kind)
{
    for (x86::reg32 at = kCollideRecords; at < kCollideEnd; at += 4)
        g.word(at) = aCollideFloat(100);
    const x86::reg32 count = 1 + below(4);
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 record = kCollideRecords + 0x40 * i;
        g.word(record) = i + 1 < count ? record + 0x40 : 0;
        g.getMemory<x86::reg16>(record + 4) = x86::reg16(chance(85) ? kind : below(6));
        g.getMemory<x86::reg16>(record + 6) = x86::reg16(random32());
        for (x86::reg32 c = 0; c < 4; ++c)
            g.word(record + 8 + 4 * c) = kCollideCorners + 0x40 * below(16);
        g.word(record + 0x18) = chance(25) ? 0 : kCollideTextures + 0x40 * below(8);
    }
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(kCollideTextures + 0x40 * i + 4) = below(4);
    for (const x86::reg32 at : {0x554e54u, 0x554e58u, 0x554e5cu})
        g.word(at) = chance(30) ? 0xffffffff : below(3);
    g.word(0x7a3a64) = below(3);
    return kCollideRecords;
}

/* The messages' blocks: their settings and channels, in use mostly. */
void makeMessages(CheckApp& g)
{
    for (x86::reg32 at = kCollideMessages; at < kCollideEnd; at += 4)
        g.word(at) = random32();
    const x86::reg32 m = kCollideMessages;
    const x86::reg32 lines = 1 + below(10), channels = 1 + below(8);
    g.word(m) = chance(10) ? below(4) : 1 + below(0x60);
    g.word(m + 4) = lines;
    g.word(m + 8) = channels;
    g.word(m + 0xc) = chance(50) ? 1 : below(3);
    g.word(m + 0x10) = below(0x40);
    g.word(m + 0x20) = chance(50) ? 0 : 1;
    for (x86::reg32 c = 0; c < channels; ++c)
    {
        const x86::reg32 channel = m + 0x28 + 0x494 * c;
        for (x86::reg32 i = 0; i < 3; ++i)
            g.word(channel + 4 * i) = below(lines);
        for (x86::reg32 l = 0; l < lines; ++l)
        {
            const x86::reg32 line = channel + 0xc + 0x74 * l;
            g.word(line) = below(lines);
            g.word(line + 4) = below(lines);
            g.word(line + 8) = below(g.word(m) + 1);
            g.getMemory<x86::reg8>(line + 0xc + below(0x68)) = 0;
        }
    }
    g.word(kCollideCopy) = chance(40) ? 1 : below(11);
    g.word(0x554f04) = m;
    g.word(0x554f08) = kCollideCopy;
    g.word(0x554f0c) = chance(85) ? 1 : 0;
    for (x86::reg32 i = 0; i < 0x80; ++i)
        g.getMemory<x86::reg8>(kCollideText + i) = x86::reg8(chance(90) ? 0x20 + below(0x5f) : 0);
}

bool makeCollideArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeCollideWorld(g);
    cpu.eax = aCar();
    cpu.edx = aCar();
    if (chance(80) && cpu.edx == cpu.eax)
        cpu.edx = kCollideCars + (cpu.eax - kCollideCars + 0x800) % 0x2000;
    if (name == "contactNormal")
        cpu.ebx = chance(50) ? 0 : 1;
    else if (name == "pushCarsApart")
        for (int i = 0; i < 3; ++i)
            stack[i] = aCollideFloat(3);
    else if (name == "polygonEdgeNormal")
    {
        static const x86::reg32 edges[] = {1, 2, 4, 8, 0, 3};
        cpu.ebx = cpu.eax + 0xc;
        cpu.eax = edges[below(6)];
        cpu.edx = kCollideOut + 0x40 * below(8);
    }
    else if (name == "clearOfWall")
        cpu.edx = chance(50) ? cpu.eax + 0xc : aCar() + 0xc;
    else if (name == "objectInReach")
    {
        cpu.edx = kCollideOut + 0x40 * below(8);
        stack[0] = aCollideFloat(3);
        stack[1] = kCollideOut + 0x200 + 0x10 * below(8);
    }
    else if (name == "distanceSquared")
    {
        cpu.eax = random32() >> below(32);
        cpu.edx = random32() >> below(32);
        cpu.ebx = random32() >> below(32);
        cpu.ecx = random32() >> below(32);
        stack[0] = random32() >> below(32);
        stack[1] = random32() >> below(32);
    }
    else if (name == "nearestPoint")
    {
        for (x86::reg32 i = 0; i < 0x167; ++i)
            for (x86::reg32 k = 0; k < 4; ++k)
                g.word(0x5537c4 + 0x10 * i + 4 * k) = k == 3 ? random32() : below(0x2000);
        cpu.eax = below(0x2000);
        cpu.edx = below(0x2000);
        cpu.ebx = below(0x2000);
        if (chance(30))
        {
            const x86::reg32 i = below(0x167);
            cpu.eax = g.word(0x5537c4 + 0x10 * i);
            cpu.edx = g.word(0x5537c4 + 0x10 * i + 4);
            cpu.ebx = g.word(0x5537c4 + 0x10 * i + 8);
        }
    }
    else if (name == "openCollFile" || name == "closeCollFile" || name == "readCollFile"
             || name == "collFileSize" || name == "collFileCount")
    {
        g.word(0x5eff18) = chance(50) ? 0 : 0x5000 + below(0x100) * 4;
        g.word(0x5eff08) = chance(70) ? 0x4c4c4f43 : random32();
        g.word(0x5eff0c) = chance(70) ? 0xb : below(16);
        g.word(0x5eff10) = 0x10 + below(0x400);
        g.word(0x5eff14) = below(100);
        cpu.eax = name == "openCollFile" ? aString() : g.word(0x5eff18);
        cpu.edx = kCollideOut;
    }
    else if (name == "collChunk")
    {
        cpu.eax = kCollideOut;
        cpu.edx = below(8);
        for (x86::reg32 i = 0; i < 8; ++i)
            g.word(kCollideOut + 4 * i) = below(0x400);
    }
    else if (name == "playerControls")
    {
        for (x86::reg32 at = 0x79c4a4; at < 0x79c4b0; at += 4)
            g.word(at) = random32();
        for (x86::reg32 i = 0; i < 8; ++i)
            g.word(0x6fd548 + 0x6c * i) = chance(50) ? 1 : below(3);
    }
    else if (name == "makeBlendTables")
        g.word(0x56438c) = chance(50) ? 0xf : 0x10;
    else if (name == "blendSpan")
    {
        const bool fifteen = chance(50);
        static const x86::reg32 masks[2][6] = {{0x1f, 0x7e0, 0xf800, 0, 5, 0xb}, {0x1f, 0x3e0, 0x7c00, 0, 5, 0xa}};
        for (x86::reg32 i = 0; i < 6; ++i)
            g.word(0x554e70 + 4 * i) = masks[fifteen][i];
        const x86::reg32 r = kCollideRecords;
        for (x86::reg32 at = kCollideTables; at < kCollideTables + 0x1000; at += 4)
            g.word(at) = random32();
        for (x86::reg32 i = 0; i < 64; ++i)
            g.word(kCollideTables + 4 * i) = kCollideTables + 0x400 + 4 * below(0x80);
        g.word(0x5f1f98) = kCollideTables + 0x800;
        g.word(0x5f1fa4) = kCollideTables + 0x900;
        g.word(0x5f1f9c) = kCollideTables + 0xa00;
        g.word(r) = kCollideTables;
        g.word(r + 8) = 0xfffff;
        g.word(r + 0x14) = below(0x100000);
        g.word(r + 0x18) = below(0x100000);
        g.word(r + 0x1c) = below(0x4000);
        g.word(r + 0x20) = below(0x4000);
        g.word(r + 0xc) = kCollideTables + 0xc00 + 2 * below(0x40);
        g.word(r + 0x10) = g.word(r + 0xc) + (chance(10) ? 0 : 2 * below(0x40)) - (chance(5) ? 2 : 0);
        cpu.eax = r;
    }
    else if (name == "subdivisionDepth")
    {
        cpu.eax = kCollideOut;
        cpu.edx = kCollideOut + 0x40;
        cpu.ebx = kCollideOut + 0x80;
        cpu.ecx = kCollideOut + 0xc0;
        for (x86::reg32 i = 0; i < 4; ++i)
            if (chance(90))
                putCollideFloat(g, kCollideOut + 0x40 * i + 8, float(below(1600)) / 100.0f);
    }
    else if (name == "drawTexturedTriangles" || name == "drawTrianglesByZ" || name == "drawFlatTriangles"
             || name == "drawFlatTrianglesAgain" || name == "stateRecord")
        cpu.eax = makeDrawerRecords(g, 1);
    else if (name == "drawQuadsByZ")
        cpu.eax = makeDrawerRecords(g, 0);
    else if (name == "drawRecords" || name == "drawMirroredRecords")
    {
        cpu.eax = chance(10) ? 0 : makeDrawerRecords(g, x86::reg16(below(6)));
        for (x86::reg32 i = 0; i < 6; ++i)
            g.word(0x5f0780 + 4 * i) = standInFor(0xd5ff04);
        if (name == "drawMirroredRecords")
        {
            stack[0] = cpu.eax;
            cpu.eax = kCollideOut;
            g.word(kCollideOut) = below(3);
        }
    }
    else if (name == "setMirrored")
    {
        cpu.eax = kCollideOut;
        g.word(kCollideOut) = below(3);
    }
    else if (name == "drawPointer")
    {
        g.word(0x554ee8) = 0;  // no mouse: readMouse hands back nothing
        g.word(0x563d3c) = below(8);
        for (x86::reg32 at = 0x8b41dc; at < 0x8b41dc + 0x2c * 8; at += 4)
            g.word(at) = random32();
        for (x86::reg32 at = 0x7d34f8; at < 0x7d3510; at += 4)
            g.word(at) = aCollideFloat(600);
    }
    else if (name == "raceFrameEnd")
        g.word(0x6ff9f8) = random32();
    else if (name == "statesForHud")
    {
        g.word(0x6fbc34) = chance(50) ? 0 : 1;
        g.word(0x7a3a68) = below(2);
        g.word(0x7a3a64) = below(3);
    }
    else if (name.find("Message") != std::string::npos || name == "messagesScrolled")
    {
        makeMessages(g);
        const x86::reg32 channels = g.word(kCollideMessages + 8);
        cpu.eax = below(channels);
        cpu.edx = name == "addMessageText" ? kCollideText + below(0x40)
                : name == "clearMessageLine" ? below(g.word(kCollideMessages + 4))
                : (random32() & 0xffffff00) | (chance(80) ? 0x20 + below(0x5f) : 0);
        if (name == "openMessages")
        {
            cpu.eax = kCollideOut;
            g.word(kCollideOut) = 1 + below(0x40);
            g.word(kCollideOut + 4) = 1 + below(5);
            g.word(kCollideOut + 8) = 1 + below(4);
            g.word(kCollideOut + 0xc) = below(3);
        }
        else if (name == "closeMessages")
        {
            g.word(0x554f04) = chance(20) ? 0 : 0x1f0000 + 0x400 * below(8);
            g.word(0x554f08) = chance(20) ? 0 : 0x1f0000 + 0x400 * below(8);
        }
    }
    return true;
}
#endif
