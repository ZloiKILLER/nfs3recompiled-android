// game_checks.cpp's parts for the cops' Hot Pursuit, the extra cars and the
// computer's car's driving and physics (native_aidrive.cpp), included in
// each table with the macro naming the part.
#if defined(AIDRIVE_WINDOWS)
// {from, to}, -- what the functions write, compared
// The cases' own: the cars, their performance and pursuit records, the
// track's slices, the places handed in and out (0xd80000 to 0xdc0000).  The
// AI's globals (0x56e000), the random numbers (0x554000), the surfaces'
// grips (0x552000), the cars' lists (0x553000, 0x5ef000), the slices'
// pointer and count (0x5dd000), the race's (0x6fd000, 0x7d3000) and the
// roadblocks (0x7dd000) are on pages compared already.
{0xd80000, 0xdc0000},
#elif defined(AIDRIVE_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kAidSlice,    // a slice of the track's: below its count ([0x5dd940])
kAidGap,      // the gap of two slices: -70 to 70, about 0, 4 and 50 often
kAidFloat,    // a float onto the x87's stack: a slice's speed, two slices' curve, an opponent's pull
kAidHeading,  // the opponent's heading (0x4070c0): its error, -0.6 to 0.6, to [0x56ec60]
kAidChoice,   // a say of 0, 1 or 2 mostly
kAidSeldom,   // 0 mostly, now and then 1 to 3
#elif defined(AIDRIVE_EFFECT_CASES)
// case kSomething: ... break;
case kAidSlice:
{
    const x86::reg32 count = word(0x5dd940);
    result = count != 0 && count < 0x10000 ? r % count : r % 64;
    break;
}
case kAidGap:
{
    static const x86::sreg32 gaps[] = {0, 1, -1, 3, -3, 4, -4, 5, 49, 50, 51, -49, -50, -51};
    result = x86::reg32(r % 3 == 0 ? gaps[(r >> 4) % 14] : x86::sreg32((r >> 4) % 141) - 70);
    break;
}
case kAidFloat:
{
    float f;
    if (s.address == 0x40ed40)
        f = float(5 + (r >> 4) % 4001 / 100.0);
    else if (s.address == 0x40ef70)
        f = float(x86::sreg32((r >> 4) % 2001) - 1000) / 1000.0f;
    else
    {
        // An opponent's pull: its table's first now and then, which the
        // off-road steering looks for.
        f = float((r >> 4) % 3001) / 100.0f;
        const x86::reg32 car = words[0];
        if (r % 4 == 0 && car < kMemory - 0x600 && word(car + 0x528) < kMemory - 0x300)
            std::memcpy(&f, &word(word(car + 0x528) + 0x2ec), 4);
    }
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(f);
    break;
}
case kAidHeading:
{
    const float f = float(x86::sreg32((r >> 4) % 1201) - 600) / 1000.0f;
    std::memcpy(&word(0x56ec60), &f, 4);
    break;
}
case kAidChoice:
    result = r % 8 == 0 ? r >> 8 : r % 3;
    break;
case kAidSeldom:
    result = r % 4 == 0 ? 1 + (r >> 4) % 3 : 0;
    break;
#elif defined(AIDRIVE_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The traffic's functions of the other part (aiplan::, native too) called by
// address here and from the pursuit: a traffic car placed, a car's three
// slices -- stand-ins as they were while generated.
{0x40c590, "a traffic car placed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40e350, "a car's three slices", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The pursuit's functions of the other part (pursuit::, native too) the cops'
// steps call by address: the roadside target, a lane's time counted, the
// lanes' times grown, a lane scored, the countdown -- stand-ins as they were
// while generated, each held against its code on its own.
{0x408b30, "the roadside target", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x408bd0, "a lane's time counted", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x409660, "the lanes' times grown", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x4096b0, "a lane scored", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40c210, "the countdown done", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The police's functions of the other part (airace::, native too) the cops'
// steps call by address: a pursuit joined (ret 4) and given up, a cop stopping
// by, a cop placed at the start, the always-one rule, the race's leader --
// stand-ins as they were while generated, each held against its code on its own.
{0x4018b0, "a pursuit joined", kStdcall, 3, 1, kEax, -1, 0, kNothing},
{0x401a80, "a pursuit given up", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x401e40, "a cop stopping by", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4029c0, "a cop placed at the start", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x402c70, "the always-one rule", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x404c80, "the race's leader", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The track's slices as the AI walks them (another part's of the AI): a
// slice a few on (eax the car, edx how many; edx left), one by the car's
// pace (eax the car, ret 0xc: aiplan::nodesAheadBySpeed reads no other
// register), the gap of two (edx left), a slice's speed and two slices'
// curve onto the x87's stack.
{0x40e160, "a slice a few on", kWatcom, 2, 0, kEax | kEdx, -1, 0, kAidSlice},
{0x40e1b0, "a slice by the pace", kStdcall, 1, 3, kEax, -1, 0, kAidSlice},
{0x40e120, "the gap of two slices", kWatcom, 2, 0, kEax | kEdx, -1, 0, kAidGap},
{0x40ed40, "a slice's speed", kWatcom, 1, 0, 0, -1, 0, kAidFloat},
{0x40ef70, "two slices' curve", kWatcom, 2, 0, kEax, -1, 0, kAidFloat},
// The opponents' pull and heading (native_ai.cpp's decompiled C++ in the
// port; the game's code here): the pull onto the x87's stack, the heading's
// error to its global.
{0x4064f0, "an opponent's pull", kWatcom, 1, 0, kEax, -1, 0, kAidFloat},
{0x4070c0, "an opponent's heading", kWatcom, 1, 0, kEax, -1, 0, kAidHeading},
// Says the cops' and the cars' code goes by: a pursuit's end (0x401500,
// 0x401fb0), whether a car is off its way (0x40f110), 0x404610's kind, a
// car's turn to skip (0x4be1a0), traffic's life (0x40eed0: no register logged, for
// aiplan::trackTuningDistance reads none and the extra cars' code calls it as C++).
{0x401500, "a pursuit's say", kWatcom, 1, 0, kEax, -1, 0, kAidChoice},
{0x401fb0, "a bust's end", kWatcom, 1, 0, kEax, -1, 0, kAidChoice},
{0x40f110, "off its way", kWatcom, 1, 0, kEax, -1, 0, kAidChoice},
{0x404610, "a car's kind of AI", kWatcom, 1, 0, kEax, -1, 0, kAidChoice},
{0x4be1a0, "a car's turn skipped", kWatcom, 2, 0, kEax, -1, 0, kAidSeldom},
{0x40eed0, "traffic's life", kWatcom, 0, 0, kEax, -1, 0, kAidChoice},
// The racestart's car put back on the road and the trackdata's place on
// the track found again (native whole, held against their code on their
// own): stood in for -- they walk the whole track's data.
{0x429100, "a car put back on the road", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x41e940, "a place on the track found again", kWatcom, 2, 0, kEax, 1, 3, kNothing},
// This file's own that other parts call by address -- the collisions' (a
// cop put back behind the leader, a cop's driver, the reverse tests), the
// racestart's (a cop's start), the physics' and the raceload's (the
// computer's car's step): stood in for there, as before they were native;
// each held against its code on its own.
{0x404d30, "a cop's start", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x405220, "a cop put back behind the leader", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x405740, "a cop's driver", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x406120, "reversing when slow", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x406160, "reversing when tipped", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x4088f0, "the computer's car's step", kWatcom, 1, 0, kEax, -1, 0, kNothing},
#elif defined(AIDRIVE_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(AIDRIVE_PROMISES)
// {"aidrive::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, eax too, and the x87's stack, its control and
// status words (a compare's flags) -- its registers in use.
{"aidrive::copStartPlace", true, true, 0, true, kNoHeap, true},
{"aidrive::callRoadblock", true, true, 0, true, kNoHeap, true},
{"aidrive::sirenSwitch", true, true, 0, true, kNoHeap, true},
{"aidrive::watchRacers", true, true, 0, true, kNoHeap, true},
{"aidrive::jumpBehindLeader", true, true, 0, true, kNoHeap, true},
{"aidrive::clockRacers", true, true, 0, true, kNoHeap, true},
{"aidrive::startPursuit", true, true, 0, true, kNoHeap, true},
{"aidrive::bustCheck", true, true, 0, true, kNoHeap, true},
{"aidrive::pursuitStep", true, true, 0, true, kNoHeap, true},
{"aidrive::dropPursuit", true, true, 0, true, kNoHeap, true},
{"aidrive::copDriver", true, true, 0, true, kNoHeap, true},
{"aidrive::nearCar", true, true, 0, true, kNoHeap, true},
{"aidrive::nearHuman", true, true, 0, true, kNoHeap, true},
{"aidrive::humanNearSlice", true, true, 0, true, kNoHeap, true},
{"aidrive::trafficUnwatched", true, true, 0, true, kNoHeap, true},
{"aidrive::trafficStart", true, true, 0, true, kNoHeap, true},
{"aidrive::trafficStop", true, true, 0, true, kNoHeap, true},
{"aidrive::extraCarStep", true, true, 0, true, kNoHeap, true},
{"aidrive::setDrivingConstants", true, true, 0, true, kNoHeap, true},
{"aidrive::drivingNothing", true, true, 0, true, kNoHeap, true},
{"aidrive::setReverse", true, true, 0, true, kNoHeap, true},
{"aidrive::reverseCheck", true, true, 0, true, kNoHeap, true},
{"aidrive::reverseWhenSlow", true, true, 0, true, kNoHeap, true},
{"aidrive::reverseWhenTipped", true, true, 0, true, kNoHeap, true},
{"aidrive::engineRevs", true, true, 0, true, kNoHeap, true},
{"aidrive::brakeForce", true, true, 0, true, kNoHeap, true},
{"aidrive::speedAlongRoad", true, true, 0, true, kNoHeap, true},
{"aidrive::laneError", true, true, 0, true, kNoHeap, true},
{"aidrive::glideAlongRoad", true, true, 0, true, kNoHeap, true},
{"aidrive::lookAheadSlice", true, true, 0, true, kNoHeap, true},
{"aidrive::sidewaysToRoad", true, true, 0, true, kNoHeap, true},
{"aidrive::towardTarget", true, true, 0, true, kNoHeap, true},
{"aidrive::towardTargetAhead", true, true, 0, true, kNoHeap, true},
{"aidrive::towardSlice", true, true, 0, true, kNoHeap, true},
{"aidrive::towardSliceSlowing", true, true, 0, true, kNoHeap, true},
{"aidrive::towardLane", true, true, 0, true, kNoHeap, true},
{"aidrive::wayToGo", true, true, 0, true, kNoHeap, true},
{"aidrive::integrate", true, true, 0, true, kNoHeap, true},
{"aidrive::coastAfterFinish", true, true, 0, true, kNoHeap, true},
{"aidrive::steerOnRoad", true, true, 0, true, kNoHeap, true},
{"aidrive::steerReversing", true, true, 0, true, kNoHeap, true},
{"aidrive::steerOffRoad", true, true, 0, true, kNoHeap, true},
{"aidrive::rideWithTarget", true, true, 0, true, kNoHeap, true},
{"aidrive::aiPhysics", true, true, 0, true, kNoHeap, true},
{"aidrive::easeLane", true, true, 0, true, kNoHeap, true},
{"aidrive::aiStep", true, true, 0, true, kNoHeap, true},
#elif defined(AIDRIVE_ARGUMENTS)
/* Where the AI's cases keep their own: six cars (0xa00 each), two
 * performance records, eight pursuit records, the track's slices, the
 * places and vectors handed in and out. */
constexpr x86::reg32 kAidCars = 0xd80000, kAidCarCount = 6, kAidCarSize = 0xa00;
constexpr x86::reg32 kAidPerformance = 0xd84000, kAidPursuits = 0xd85000, kAidSlices = 0xd86000;
constexpr x86::reg32 kAidOut = 0xdbf000;

/* A float's bits: within `range` either side, the marks the code compares
 * with now and then (and a little either side), 0 now and then.  No NaNs:
 * which of two a sum keeps is the host compiler's. */
x86::reg32 anAidFloat(float range)
{
    static const float marks[] = {0.04f, 0.2f, 0.5f, 0.7f, 0.9f, 1.0f, 2.0f, 2.5f, 3.0f, 5.0f, 7.0f,
                                  10.0f, 13.41f, 16.41f, 20.0f, 22.35f, 44.7f, 300.0f};
    float f;
    const x86::reg32 kind = below(100);
    if (kind < 15)
        f = marks[below(18)] * (chance(40) ? -1.0f : 1.0f) * (chance(50) ? 1.0f : chance(50) ? 1.001f : 0.999f);
    else if (kind < 25)
        f = chance(20) ? -0.0f : 0.0f;
    else
        f = range * (float(below(20001)) / 10000.0f - 1.0f);
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

void putAidFloat(CheckApp& g, x86::reg32 at, float f)
{
    std::memcpy(&g.word(at), &f, 4);
}

x86::reg32 anAidCar()
{
    return kAidCars + kAidCarSize * below(kAidCarCount);
}

/* A car's record: floats mostly, then the fields the AI reads set as a race
 * has them. */
void makeAidCar(CheckApp& g, x86::reg32 car, x86::reg32 slices, x86::reg32 frame)
{
    for (x86::reg32 at = car; at < car + kAidCarSize; at += 4)
        g.word(at) = chance(80) ? anAidFloat(10.0f) : below(16);
    g.word(car) = random32();
    g.word(car + 8) = below(30);
    g.word(car + 0x1c) = below(slices);
    for (const x86::reg32 at : {car + 0x88u, car + 0x89u, car + 0x8au})
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(80) ? 0 : 1);
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g.word(car + 0x98 + 4 * i) = anAidFloat(chance(50) ? 20.0f : 600.0f);
        g.word(car + 0xa4 + 4 * i) = anAidFloat(60.0f);
        g.word(car + 0xe4 + 4 * i) = anAidFloat(3.0f);
        g.word(car + 0x2e8 + 4 * i) = anAidFloat(20.0f);
        g.word(car + 0x2f4 + 4 * i) = anAidFloat(4.0f);
        g.word(car + 0x4e0 + 4 * i) = anAidFloat(40.0f);
        g.word(car + 0x4ec + 4 * i) = anAidFloat(i == 2 ? 60.0f : 10.0f);
        g.word(car + 0x4f8 + 4 * i) = anAidFloat(2.0f);
    }
    for (x86::reg32 i = 0; i < 9; ++i)
    {
        g.word(car + 0xc0 + 4 * i) = anAidFloat(1.0f);
        g.word(car + 0x124 + 4 * i) = anAidFloat(1.0f);
    }
    putAidFloat(g, car + 0xb8, float(below(4001)) / 100.0f);
    putAidFloat(g, car + 0x10c, chance(80) ? 0.8f + float(below(41)) / 100.0f : float(below(101)) / 100.0f);
    g.word(car + 0x160) = below(3);
    g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(80) ? 0 : 1 + below(3));
    g.word(car + 0x168) = chance(70) ? anAidFloat(5.0f) : 0;
    g.word(car + 0x18c) = below(16);
    g.word(car + 0x1f0) = below(4);
    g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
    g.getMemory<x86::reg8>(car + 0x201) = x86::reg8(random32());
    g.word(car + 0x210) = x86::reg32(x86::sreg32(below(2049)) - 1024);
    g.word(car + 0x2bc) = below(8);
    g.word(car + 0x2c4) = chance(60) ? below(3) : 3 + below(3);
    for (x86::reg32 at = car + 0x508; at < car + 0x520; ++at)
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(60) ? 0 : chance(50) ? 0xff : random32());
    g.getMemory<x86::reg8>(car + 0x50b) = x86::reg8(below(8));
    g.word(car + 0x528) = kAidPerformance + 0x800 * below(2);
    g.word(car + 0x590) = below(9000);
    for (const x86::reg32 at : {car + 0x620u, car + 0x621u, car + 0x710u, car + 0x711u})
        g.getMemory<x86::reg8>(at) = x86::reg8(random32());
    g.word(car + 0x624) = chance(60) ? anAidCar() : 0;
    g.word(car + 0x628) = kAidPursuits + 0x40 * below(8);
    g.word(car + 0x638) = below(14);
    g.word(car + 0x64c) = x86::reg32(x86::sreg32(below(36)) - 5);
    g.word(car + 0x650) = below(100);
    g.word(car + 0x658) = below(12);
    g.word(car + 0x65c) = frame + below(64) - 32;
    g.word(car + 0x6d8) = chance(70) ? anAidCar() : 0;
    g.word(car + 0x6e0) = below(10);
    g.word(car + 0x6ec) = frame + below(64) - 32;
    g.word(car + 0x6f0) = anAidFloat(5.0f);
    g.word(car + 0x714) = below(4);
    g.word(car + 0x720) = frame + below(64) - 32;
    g.word(car + 0x730) = chance(70) ? anAidCar() : 0;
    g.word(car + 0x734) = below(2);
    g.word(car + 0x7b0) = chance(20) ? 0 : anAidFloat(50.0f);
    // The lane's offsets: across a lane mostly, now and then in the slices'
    // own units (the road's edges, a width by 0x8000 by its lanes).
    for (const x86::reg32 at : {car + 0x7b4u, car + 0x7b8u, car + 0x7c8u})
        g.word(at) = anAidFloat(chance(80) ? 10.0f : 4000000.0f);
    if (chance(20))
        g.word(car + 0x7c8) = g.word(car + 0x7b4);
    putAidFloat(g, car + 0x7bc, float(below(201)) / 100.0f);
    g.word(car + 0x7c0) = anAidFloat(60.0f);
    putAidFloat(g, car + 0x7c4, chance(10) ? 0.0f : float(below(6001)) / 100.0f);
    g.word(car + 0x7d0) = frame + below(64) - 32;
    g.word(car + 0x7d4) = chance(70) ? below(2) : below(4);
    g.word(car + 0x7d8) = chance(70) ? below(0x80) : below(1100);
}

/* The world: the cars and their records, the lists, the slices, the race's
 * globals, the AI's tuning, the x87 of every precision and rounding. */
void makeAidWorld(CheckApp& g, x86::CPU& cpu)
{
    const x86::reg32 slices = chance(50) ? 16 + below(64) : 0x240 + below(0x40);
    const x86::reg32 frame = chance(80) ? 0x480 + below(0x10000) : below(0x500);
    for (x86::reg32 car = kAidCars; car < kAidCars + kAidCarSize * kAidCarCount; car += kAidCarSize)
        makeAidCar(g, car, slices, frame);
    // The performance records: gears, a gear change's frames, the gears'
    // revs by speed, the idle and the top revs, the brake, the pull's table.
    for (x86::reg32 p = kAidPerformance; p < kAidPerformance + 0x1000; p += 0x800)
    {
        for (x86::reg32 at = p; at < p + 0x800; at += 4)
            g.word(at) = anAidFloat(30.0f);
        g.word(p + 8) = 2 + below(6);
        g.word(p + 0x10) = below(20);
        for (x86::reg32 i = 0; i < 8; ++i)
            putAidFloat(g, p + 0x94 + 4 * i, float(20 + below(400)) * (i == 0 ? -1.0f : 1.0f));
        g.word(p + 0x1c0) = 500 + below(1000);
        g.word(p + 0x1c4) = 4000 + below(5000);
        putAidFloat(g, p + 0x1d8, float(below(4001)) / 100.0f);
        if (chance(20))
            g.word(p + 0x248) = 0;
        for (x86::reg32 i = 0; i < 0x70; ++i)
            putAidFloat(g, p + 0x2ec + 4 * i, float(below(3001)) / 100.0f);
    }
    for (x86::reg32 at = kAidPursuits; at < kAidPursuits + 0x200; at += 4)
        g.word(at) = random32();
    for (x86::reg32 p = kAidPursuits; p < kAidPursuits + 0x200; p += 0x40)
    {
        g.word(p + 0x14) = below(2);
        g.word(p + 0x1c) = below(4);
        g.word(p + 0x28) = frame + below(64) - 32;
        g.word(p + 0x2c) = below(2);
    }
    // The slices: their points (16.16, near the cars'), lanes, widths and
    // vectors (signed bytes).
    for (x86::reg32 i = 0; i < slices + 1; ++i)
    {
        const x86::reg32 slice = kAidSlices + 0x24 * i;
        for (x86::reg32 at = slice; at < slice + 0x24; at += 4)
            g.word(at) = random32();
        for (x86::reg32 k = 0; k < 3; ++k)
            g.word(slice + 4 * k) = x86::reg32(x86::sreg32(below(0x2000000)) - 0x1000000);
        g.getMemory<x86::reg8>(slice + 0x13) = x86::reg8(chance(30) ? below(0x100) : below(3) << 4 | below(3));
        g.getMemory<x86::reg8>(slice + 0x17) = x86::reg8(chance(30) ? 0 : below(0x100));
        g.getMemory<x86::reg8>(slice + 0x1b) = x86::reg8(chance(30) ? 0 : below(0x100));
    }
    g.word(0x5dd948) = kAidSlices;
    g.word(0x5dd940) = slices;
    // The lists: the cops, the racers, the humans' cars, the first car.
    const x86::reg32 counts[] = {below(5), below(7), below(3)};
    const x86::reg32 lists[] = {0x5efc08, 0x5efc48, 0x5efa48};
    for (x86::reg32 l = 0; l < 3; ++l)
        for (x86::reg32 i = 0; i < 8; ++i)
            g.word(lists[l] + 4 * i) = anAidCar();
    g.word(0x553078) = counts[0];
    g.word(0x5efda0) = counts[1];
    g.word(0x5efda4) = counts[2];
    g.word(0x5efac8) = anAidCar();
    // The roadblocks: in use or not, their time.
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        g.word(0x7dd850 + 0x50 * i) = chance(50) ? 0 : 1;
        g.word(0x7dd850 + 0x50 * i + 0x4c) = frame + below(64) - 32;
    }
    // The surfaces' grips, by surface and track's kind.
    for (x86::reg32 i = 0; i < 16 * 3; ++i)
        putAidFloat(g, 0x5523d8 + 4 * i, float(20 + below(181)) / 100.0f);
    // The race's.
    g.word(0x6fd3b0) = below(3);
    g.word(0x6fd4f4) = below(2);
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x7d3684) = frame;
    g.word(0x7d3680) = chance(80) ? 1 : 0;
    g.word(0x6fd4a8) = below(2);
    g.word(0x6fd4ac) = chance(60) ? 0 : below(11);
    g.word(0x6fd494) = below(2);
    g.word(0x6fd4f0) = below(2);
    g.word(0x56ebc0) = below(8);
    putAidFloat(g, 0x56ebb0, float(below(1001)) / 100.0f);
    // The AI's tuning as a race has it, now and then any.
    static const x86::reg32 tuning[][2] = {
        {0x56ebd0, 0x40e00000}, {0x56ebd4, 0x40a00000}, {0x56ebd8, 0x42000000}, {0x56ebdc, 0x42180000},
        {0x56ebe0, 0x40800000}, {0x56ebe4, 0x40200000}, {0x56ebf0, 0x40a00000}, {0x56ebe8, 0x3f4ccccd},
        {0x56ebec, 0x3f4ccccd}, {0x56ebf4, 0x41b2cfdf}, {0x56ebf8, 0x3e041893}, {0x56ebfc, 0x3c449ba6},
        {0x56ec08, 0x42180000}, {0x56ec0c, 0x40400000}, {0x56ec04, 0x41800000}, {0x56ec10, 0x40200000},
        {0x56ec14, 0x3fcccccd}, {0x56ec18, 0x3f4ccccd}, {0x56ec24, 0x3c23d70a}, {0x56ec28, 0x3ca3d70a},
        {0x56ec1c, 0x41200000}, {0x56ec30, 0x42000000}, {0x56ec34, 0x42200000}, {0x56ec38, 0x40c00000},
        {0x56ec44, 0x3f800000}, {0x56ec3c, 0x40400000}, {0x56ec40, 0x40400000}, {0x56ec48, 0x41200000},
        {0x56ec50, 0x3e4ccccd}, {0x56ec54, 0x3d23d70a},
    };
    for (const auto& t : tuning)
        g.word(t[0]) = chance(90) ? t[1] : anAidFloat(40.0f);
    putAidFloat(g, 0x56ec58, chance(90) ? 1.0f / 32 : float(below(101)) / 1000.0f);
    g.word(0x56ec78) = chance(80) ? 1 : below(4);
    putAidFloat(g, 0x56ec60, float(x86::sreg32(below(1201)) - 600) / 1000.0f);
    g.word(0x56ec68) = chance(50) ? 0 : anAidFloat(1.0f);
    for (x86::reg32 i = 0; i < 3; ++i)
        g.word(0x56ec6c + 4 * i) = anAidFloat(40.0f);
    g.word(0x554f10) = random32() & 0xffff;
    g.word(0x554f18) = chance(90) ? 0x41c64e6d : random32();
    // The handling's mark: its dot product's stand-in hands back -40 to 40.
    g.word(0xd13ffc) = x86::reg32(g_seed);
    // The x87: a control word of every precision and rounding, one or none on its stack.
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(2);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(double(x86::sreg32(random32())) / 1000.0);
}

bool makeAidriveArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeAidWorld(g, cpu);
    cpu.eax = anAidCar();
    for (x86::reg32 at = kAidOut; at < kAidOut + 0x100; at += 4)
        g.word(at) = anAidFloat(100.0f);
    // Where a way is handed back (the ways, wayToGo).
    cpu.esi = kAidOut + 4 * below(16);
    if (name == "copStartPlace")
    {
        if (chance(70) && g.word(0x553078) != 0)
            g.word(0x5efc08 + 4 * below(g.word(0x553078))) = cpu.eax;
        cpu.edx = kAidOut + 0x80;
        cpu.ebx = kAidOut + 0x90;
    }
    else if (name == "nearCar")
    {
        const x86::reg32 car = anAidCar();
        cpu.edx = chance(10) ? 0 : car;
        cpu.eax = kAidOut;
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            float f;
            std::memcpy(&f, &g.word(car + 0x98 + 4 * i), 4);
            putAidFloat(g, kAidOut + 4 * i, f + float(x86::sreg32(below(1601)) - 800) / 2.0f);
        }
    }
    else if (name == "humanNearSlice")
        cpu.eax = below(g.word(0x5dd940));
    else if (name == "setReverse")
    {
        cpu.edx = below(4);
        cpu.ebx = chance(50) ? 0 : 0x80;
    }
    else if (name == "reverseWhenTipped")
        cpu.edx = kAidOut + 0x40;
    else if (name == "brakeForce")
        stack[0] = anAidFloat(10.0f);
    else if (name == "easeLane")
        stack[0] = chance(80) ? 0x3e4ccccd : anAidFloat(2.0f);
    else if (name == "nearHuman" || name == "trafficUnwatched")
    {
        // A human's car near now and then.
        if (chance(50) && g.word(0x5efda4) != 0)
            for (x86::reg32 i = 0; i < 3; ++i)
                g.word(g.word(0x5efa48) + 0x98 + 4 * i) = g.word(cpu.eax + 0x98 + 4 * i);
    }
    return true;
}
#endif
