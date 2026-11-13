// game_checks.cpp's parts for a car's race state -- put back on the road, its
// wheels' marks and effects, the camera's shake, a crash's effects, the
// start's grid (native_racestart.cpp) -- included in each table with the
// macro naming the part.
#if defined(RACESTART_WINDOWS)
// {from, to}, -- what the functions write, compared
// The cases' car and its records are in the harness's (0x624000 to
// 0x62a000); the globals: the start's grid and the shake's tuning (0x5ef000),
// the track's slices (0x5dd000), the race's flags (0x6fd000, compared
// already), the reverse grid's flag (0x55d000), whether a car may be put
// back (0x7d3000).  The surfaces' tables (0x553080) are the game's own.
{0x5ef000, 0x5f0000}, {0x5dd000, 0x5de000}, {0x55d000, 0x55e000}, {0x7d3000, 0x7d4000},
#elif defined(RACESTART_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kRsWheelCell,  // a wheel's cell on the track (0x420650): its 0x78 bytes at esi, as wheelCells copies them
kRsObjectAgainst,  // the object a car is against (0x4ca160): 1 or 0, with 1 its handle to [edx]
#elif defined(RACESTART_EFFECT_CASES)
// case kSomething: ... break;
case kRsWheelCell:
{
    // Made up, but what the wheels' code walks as a race has it: a slice of
    // the cases' 0x100, whether on a piece of its own, its heights (the
    // cases' at 0x625800).
    for (x86::reg32 k = 0; k < 0x78; k += 4)
        word(cpu.esi + k) = random(id + k);
    word(cpu.esi + 0x10) = r % 0x100;
    word(cpu.esi + 0x14) = (r >> 8) % 2 == 0 ? 0 : 1 + (r >> 9) % 4;
    word(cpu.esi + 0x18) = (r >> 12) % 3 == 0 ? 0 : 1 + (r >> 13) % 4;
    word(cpu.esi + 0x1c) = 0x625800 + 0x10 * ((r >> 16) % 4) + 2 * ((r >> 18) % 2);
    break;
}
case kRsObjectAgainst:
    // As the real one: with an object found its handle to the word at edx, which objectContacts
    // hands on (0x4c9f50, 0x401650) -- else both sides would read the stack's garbage there.
    result = r & 1;
    if (result != 0)
        word(cpu.edx) = random(id ^ 0x5a5a5a5a);
    break;
#elif defined(RACESTART_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// What the car's functions call that leaves a float on the x87's stack: the
// game's sine of a stack float (scaled), a vector's length, a vector
// normalised (its length back) -- every register kept, a float made up.
{0x4ea930, "a sine", kStdcall, 0, 1, 0, -1, 0, kFloatBack},
{0x4e06a0, "a vector's length", kWatcom, 1, 0, 0, 0, 3, kFloatBack},
{0x4972f0, "a vector normalised", kWatcom, 1, 0, 0, 0, 3, kFloatBack},
// A wheel's cell on the track, by its place (eax) from the car's slice (ebx):
// the record to esi.
{0x420650, "a wheel's cell on the track", kWatcom, 3, 0, kEax | kEdx | kEbx, 0, 3, kRsWheelCell},
// The object the car is against, if any (its handle to [edx]): 1 or 0.
{0x4ca160, "an object the car is against", kWatcom, 2, 0, kEax, -1, 0, kRsObjectAgainst},
// The kind of polygon a wheel's place is on (trackdata::positionSurface,
// native too since trackdata's natives came in): by address, to a stand-in
// as any callee, its place (eax) and whether on a strip (edx) logged.
{0x4207e0, "the kind of polygon a place is on", kWatcom, 2, 0, kEax, -1, 0, kNothing},
#elif defined(RACESTART_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(RACESTART_PROMISES)
// {"racestart::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, eax too, and the x87's stack, its control and
// status words (a compare's flags) -- its registers in use.
{"racestart::respawnCar", true, true, 0, true, kNoHeap, true},
{"racestart::resetCar", true, true, 0, true, kNoHeap, true},
{"racestart::stuckCheck", true, true, 0, true, kNoHeap, true},
{"racestart::clearRespawn", true, true, 0, true, kNoHeap, true},
{"racestart::shakeAmplitude", true, true, 0, true, kNoHeap, true},
{"racestart::startShake", true, true, 0, true, kNoHeap, true},
{"racestart::startIdleShake", true, true, 0, true, kNoHeap, true},
{"racestart::shakeWave", true, true, 0, true, kNoHeap, true},
{"racestart::objectContacts", true, true, 0, true, kNoHeap, true},
{"racestart::wheelSlip", true, true, 0, true, kNoHeap, true},
{"racestart::wheelCells", true, true, 0, true, kNoHeap, true},
{"racestart::wheelSurface", true, true, 0, true, kNoHeap, true},
{"racestart::endSkidMark", true, true, 0, true, kNoHeap, true},
{"racestart::extendSkidMark", true, true, 0, true, kNoHeap, true},
{"racestart::skidSounds", true, true, 0, true, kNoHeap, true},
{"racestart::wheelSpray", true, true, 0, true, kNoHeap, true},
{"racestart::surfaceEffects", true, true, 0, true, kNoHeap, true},
{"racestart::wheelSkid", true, true, 0, true, kNoHeap, true},
{"racestart::wheelEffects", true, true, 0, true, kNoHeap, true},
{"racestart::updateShake", true, true, 0, true, kNoHeap, true},
{"racestart::crashEffects", true, true, 0, true, kNoHeap, true},
{"racestart::respawnEffects", true, true, 0, true, kNoHeap, true},
{"racestart::stopCarEffects", true, true, 0, true, kNoHeap, true},
{"racestart::carEffects", true, true, 0, true, kNoHeap, true},
{"racestart::airborneSpin", true, true, 0, true, kNoHeap, true},
{"racestart::updateCar", true, true, 0, true, kNoHeap, true},
{"racestart::defaultGridPlace", true, true, 0, true, kNoHeap, true},
{"racestart::loadStartGrid", true, true, 0, true, kNoHeap, true},
{"racestart::gridPlace", true, true, 0, true, kNoHeap, true},
{"racestart::placeOnGrid", true, true, 0, true, kNoHeap, true},
#elif defined(RACESTART_ARGUMENTS)
/* Where the car's cases keep their own: the car, its handling, its two other
 * records (+0x220, +0x8a4), its wheels' heights, the track's slices, what the
 * grid's places are written to. */
constexpr x86::reg32 kRsCar = 0x624000, kRsHandling = 0x625000, kRsRecord = 0x625400, kRsStart = 0x625600;
constexpr x86::reg32 kRsHeights = 0x625800, kRsOut = 0x625c00, kRsSlices = 0x626000, kRsSliceCount = 0x100;

/* A float as the car's fields have them: the thresholds its code compares
 * with, a little either side; small, middling, negative; 0. */
x86::reg32 aRaceFloat()
{
    static const float marks[] = {0.015625f, 0.1f, 0.2f, 0.5f, 0.6f, 0.75f, 0.9f, 1.0f, 1.5f, 2.0f,
                                  4.0f, 4.5f, 5.0f, 7.5f, 10.0f, 15.0f, 20.0f, 22.0f, 30.0f, 60.0f};
    float f;
    const x86::reg32 kind = below(100);
    if (kind < 40)
        f = marks[below(20)] * (chance(15) ? -1.0f : 1.0f) * (chance(50) ? 1.0f : chance(50) ? 1.001f : 0.999f);
    else if (kind < 50)
        f = 0.0f;
    else if (kind < 80)
        f = float(x86::sreg32(below(20001)) - 10000) / 1000.0f;
    else
        f = float(x86::sreg32(below(200001)) - 100000) / 1000.0f;
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A field of the car's: a float mostly, 0, a small count, anything. */
x86::reg32 aRaceField()
{
    const x86::reg32 kind = below(100);
    return kind < 40 ? aRaceFloat() : kind < 65 ? 0 : kind < 85 ? below(8) : kind < 93 ? below(0x100) : random32();
}

void putRaceFloat(CheckApp& g, x86::reg32 at, float f)
{
    std::memcpy(&g.word(at), &f, 4);
}

/* The car's world: the car -- each field made up, then those its code walks
 * set as a race has them: its records, its slice and surface, its wheels'
 * cells and marks, its flags and counts -- and the race's globals the
 * functions read, the track's slices, the grid. */
void makeRaceStartWorld(CheckApp& g)
{
    for (x86::reg32 at = kRsCar; at < kRsSlices + kRsSliceCount * 0x24 + 0x40; at += 4)
        g.word(at) = aRaceField();
    const x86::reg32 car = kRsCar;
    g.word(car + 0x528) = kRsHandling;
    g.word(car + 0x220) = kRsRecord;
    g.word(car + 0x8a4) = kRsStart;
    g.word(car + 0x1c) = below(kRsSliceCount);
    g.word(car + 0x184) = chance(60) ? 0 : below(kRsSliceCount);
    g.word(car + 0x160) = below(3);
    g.word(car + 0x18c) = chance(10) ? 0xe : below(16);
    g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(60) ? 0 : below(8));
    g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
    g.word(car + 0x2c4) = below(6);
    g.word(car + 0x8a0) = chance(30) ? below(4) : chance(50) ? below(0x40) : x86::reg32(-x86::sreg32(below(8)));
    g.word(car + 0x8c8) = chance(40) ? 0 : below(0x30);
    g.word(car + 0x8d0) = chance(40) ? 0 : below(0x30);
    g.word(car + 0x8d4) = chance(40) ? 0 : chance(50) ? below(0x60) : x86::reg32(-x86::sreg32(below(8)));
    g.word(car + 0x524) = chance(60) ? 0 : below(0x30);
    g.word(car + 0x99c) = chance(30) ? 0 : below(0x10);
    g.word(car + 0x9a0) = below(5);
    g.word(car + 0x170) = (chance(85) ? below(7) << 16 : random32() & 0xffff0000) | (chance(20) ? 0x10000000 : 0)
                          | below(12);
    g.word(car + 0x1f8) = below(8);
    g.word(car + 0x2c0) = random32();
    // Its speed, how hard it hit, its height, how far off the ground: as a
    // race has them, past the thresholds and short of them.
    const auto range = [&](x86::reg32 at, float from, float to) {
        if (chance(70))
            putRaceFloat(g, at, from + (to - from) * float(below(10001)) / 10000.0f);
    };
    range(car + 0xb8, 0.0f, 80.0f);
    range(car + 0x168, 0.0f, 100.0f);
    range(car + 0x15c, -2.0f, 10.0f);
    range(car + 0x10c, -2.0f, 2.0f);
    for (x86::reg32 at = car + 0x95c; at < car + 0x998; ++at)
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(50) ? 0 : chance(70) ? 1 : random32());
    g.getMemory<x86::reg8>(car + 0x504) = x86::reg8(random32());
    for (const x86::reg32 at : {car + 0x88, car + 0x89, car + 0x8b})
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(50) ? 0 : 1);
    // Its wheels: their cells (a slice, whether on a piece of its own, its
    // heights), and where they are.
    for (x86::reg32 w = 0; w < 4; ++w)
    {
        const x86::reg32 wheel = car + 0x300 + 0x78 * w;
        g.word(wheel + 0x10) = below(kRsSliceCount);
        g.word(wheel + 0x14) = chance(50) ? 0 : 1 + below(4);
        g.word(wheel + 0x18) = chance(30) ? 0 : 1 + below(4);
        g.word(wheel + 0x1c) = kRsHeights + 0x10 * w + 2 * below(2);
        for (x86::reg32 k = 4; k < 0x10; k += 4)
            g.word(wheel + k) = chance(80) ? x86::reg32(x86::sreg32(below(0x2000000)) - 0x1000000) : random32();
    }
    for (x86::reg32 at = kRsHeights; at < kRsHeights + 0x60; at += 4)
        g.word(at) = random32();
    // The race's: the reverse direction, the slices' step and count, the
    // shake's tuning, the players, the mode, the grid's.
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x5efdcc) = chance(80) ? 1 + below(8) : below(0x40);
    g.word(0x5dd940) = kRsSliceCount;
    g.word(0x5dd948) = kRsSlices;
    putRaceFloat(g, 0x5efdd0, chance(80) ? 2.0f + float(below(40)) / 10.0f : 0.0f);
    g.word(0x7d3680) = chance(80) ? 1 : 0;
    g.word(0x6fd3b8) = below(5);
    g.word(0x6fd3b0) = below(3);
    g.word(0x5efdc0) = below(0x200);
    g.word(0x5efdc4) = below(0x200);
    g.word(0x6fd4a0) = chance(50) ? 0 : 1;
    for (const x86::reg32 at : {0x5efda8u, 0x5efdacu, 0x5efdb0u, 0x5efdb4u, 0x5efdb8u, 0x5efdbcu, 0x5efde0u,
                                0x5efde4u})
        g.word(at) = aRaceFloat();
    g.word(0x6fd518) = below(8);
    g.word(0x6fd520) = below(8);
    g.word(0x6fd4ac) = below(0x40);
    g.word(0x55d000) = chance(50) ? 1 : below(3);
    g.word(0x6fd4fc) = below(4);
    g.word(0x6fd4cc) = chance(50) ? 0 : 1;
    g.word(0x6fd4c8) = chance(50) ? 0 : 1;
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x6fdbec + 4 * i) = below(12);
    for (x86::reg32 at = 0x5ef9b8; at < 0x5ef9b8 + 12 * 12; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = kRsOut; at < kRsOut + 0x40; at += 4)
        g.word(at) = random32();
}

bool makeRaceStartArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    // A crash's jolt (controls::crashJolt, called from crashEffects) played
    // from the case's car now and then: the force feedback's world as the
    // controls' cases make it (controls_race.inc), the force car this one.
    // That world's x87 too: a control word of every precision and rounding,
    // one or none on its stack.
    makeRaceWorld(g, cpu);
    makeRaceStartWorld(g);
    if (chance(60))
        g.word(0x678b54) = kRsCar;
    cpu.eax = kRsCar;
    // A wheel's index; a front wheel's or a back one's.
    if (name == "wheelSlip")
        cpu.edx = chance(50) ? 0 : 1 + below(3);
    else if (name == "wheelSurface" || name == "endSkidMark" || name == "extendSkidMark" || name == "wheelSpray"
             || name == "surfaceEffects" || name == "wheelSkid")
        cpu.edx = below(4);
    else if (name == "startShake")
    {
        cpu.edx = below(0x40);
        cpu.ebx = below(0x40);
    }
    else if (name == "shakeWave")
    {
        stack[0] = aRaceFloat();
        stack[1] = below(0x40);
    }
    else if (name == "defaultGridPlace" || name == "gridPlace")
    {
        cpu.eax = below(12);
        cpu.edx = kRsOut + 4 * below(4);
        cpu.ebx = kRsOut + 0x20 + 4 * below(4);
    }
    return true;
}
#endif
