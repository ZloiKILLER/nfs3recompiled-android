// game_checks.cpp's parts for the traffic cars' driver and lane planner, the
// track nodes' lane helpers, the tracks' tuning records, the track's
// animation paths and the sound's start-up (native_aiplan.cpp), included in
// each table with the macro naming the part.
#if defined(AIPLAN_WINDOWS)
// The data among the code at 0x40cb00 (written by makeAiplanWorld as nfs3.exe
// has it); the speed tables' pointers and the sound's switches (0x552558..
// 0x552918); the random numbers (0x554f10); the options' byte (0x55eb2e); the
// lane costs, the sensed cars, the tracks' tuning, the lane widths, the
// animation players and paths (0x56eb00..0x570a50); the voices' records
// (0x571124); the nodes' pointer; the cars' lists; the sound setup
// (0x6fbbdc..); the race's switches; the clock and the ticks; the obstacles
// (0x7dd850); and the cases' own (0xd80000 on).
    {0x40c000, 0x40d000}, {0x552000, 0x553000}, {0x554000, 0x555000}, {0x55e000, 0x55f000},
    {0x56e000, 0x572000}, {0x5dd000, 0x5de000}, {0x5ef000, 0x5f0000}, {0x6fb000, 0x6fc000},
    {0x6fd000, 0x6fe000}, {0x79c000, 0x79d000}, {0x7d3000, 0x7d4000}, {0x7dd000, 0x7de000},
    {0xd80000, 0xda0000},
#elif defined(AIPLAN_EFFECTS)
    kAiplanFlag,  // a yes or no of the traffic's: 0 a third of the time, else 1
#elif defined(AIPLAN_EFFECT_CASES)
        case kAiplanFlag:
            result = r % 3 == 0 ? 0 : 1;
            break;
#elif defined(AIPLAN_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// After the others' rows (the first row of an address is the one used).
// The AI functions of the chunks below this one (0x401220..0x40c4f0) the
// traffic's driver calls by address, stood in for as they are while they are
// the game's code -- the traffic's top speed onto the x87's stack, whether a
// car is in play and its loading steps a yes or no -- so that they do not run
// on these worlds once they are natives too.
{0x40bfe0, "the traffic's top speed", kWatcom, 0, 0, kEax, -1, 0, kFloatBack},
{0x40bff0, "a traffic car's lane kept", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40c160, "a traffic car steered", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40c270, "a traffic car's speed set", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40c2f0, "a traffic car's route", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x404690, "a traffic car linked", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x401240, "a car in play", kWatcom, 1, 0, kEax, -1, 0, kAiplanFlag},
{0x405cd0, "a car's loading done", kWatcom, 1, 0, kEax, -1, 0, kAiplanFlag},
{0x405d40, "a car readied", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x405dc0, "a car started", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x405e20, "a car can start", kWatcom, 1, 0, kEax, -1, 0, kAiplanFlag},
// Other parts' natives that would run on these worlds: a police chase called
// (snd::copChaseSet), a car's drive reset (handling::resetDriveState), the
// opponents' speeds loaded (load::loadOpponentSpeeds), the sound library
// started, its rate set (snd::SNDinit, snd::SNDsetrate), the effect bus
// (port::busInit).
{0x411750, "a police chase called", kWatcom, 4, 0, kEax, -1, 0, kNothing},
// The drive reset reads the car in eax alone: the code's edx (the spin, for the scaling after
// it) is no argument of its, and native_aiplan.cpp's call as C++ (guest::direct) hands none.
{0x4aa79f, "a car's drive reset", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40e000, "the opponents' speeds loaded", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x4e2630, "the sound library started", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x4e2228, "the sound's rate set", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x418740, "the effect bus set up", kWatcom, 0, 0, kEax, -1, 0, kNothing},
// The game's the animation paths call: two poses blended (both orientations
// and the first place logged: 11 words from edx, the frame has them in a
// row), two places blended (both places logged), an orientation made a
// matrix (the orientation logged).
{0x4b70f0, "two poses blended", kStdcall, 4, 3, kEax | kEcx | kEdx, 1, 11, kNothing},
{0x4b6a30, "two places blended", kStdcall, 4, 3, kEax | kEcx | kEdx, 2, 6, kNothing},
{0x4b6ca0, "an orientation made a matrix", kWatcom, 2, 0, kEax, 0, 2, kNothing},
// This file's own that the other parts' natives call by address, stood in for
// as they were while they were the game's code (generate.py's rows of then),
// since those parts' worlds have no traffic or paths: the traffic car reset
// (raceload), the road slice (raceload), whether the road is wide
// (racestart), the paths freed, started, set, stopped and posed (track,
// carstate), a path's place (the ambient sounds).
{0x40cb70, "aiplan::resetTrafficCar", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40e2b0, "aiplan::roadSliceIndex", kStdcall, 1, 1, kEax, -1, 0, kNothing},
{0x40eca0, "aiplan::roadIsWide", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x40f330, "aiplan::freeAnimationPaths", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x40f3b0, "aiplan::startAnimation", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40f430, "aiplan::setAnimationMode", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x40f460, "aiplan::stopAnimation", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40f4b0, "aiplan::animationPose", kStdcall, 4, 1, kEax, -1, 0, kNothing},
{0x40f6d0, "aiplan::animationPlace", kStdcall, 4, 2, kEax, -1, 0, kNothing},
#elif defined(AIPLAN_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(AIPLAN_PROMISES)
// {"aiplan::name", watcom, result, arguments, fpu, heap, liveOnly},
// The registers each is handed (eax, edx, ebx, ecx), the caller's no longer;
// eax as each leaves it, but for the void ones (result false): what their code
// leaves there (a compare's status word, a loop's index, a callee's) the game's
// compiled callers never read; the x87's registers for those that leave a float
// on its stack.
{"aiplan::leastTrafficPlayer", true, true, 1},
{"aiplan::placeTrafficCar", true, true, 1},
{"aiplan::reactToTraffic", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::slowForPolice", true, true, 1},
{"aiplan::countStopped", true, true, 0},
{"aiplan::nearPoliceCar", true, true, 2},
{"aiplan::weighLaneVotes", true, true, 0},
{"aiplan::voteRoadside", true, true, 1},
{"aiplan::atRoadside", true, true, 1},
{"aiplan::advanceCarAi", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::clearSensedCars", true, true, 0},
{"aiplan::cacheTrafficSide", true, true, 0},
{"aiplan::resetTrafficCar", true, true, 1},
{"aiplan::clearTrafficRecord", true, true, 1},
{"aiplan::placeOnTrack", true, true, 1},
{"aiplan::brakeToStop", true, true, 1},
{"aiplan::voteCurrentLane", true, true, 1},
{"aiplan::senseTraffic", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::planLaneChange", true, false, 2},  // eax: what the code leaves no caller reads (void)
{"aiplan::edgeLaneCost", true, false, 2},  // eax: what the code leaves no caller reads (void)
{"aiplan::scoreLanesByRoad", true, true, 1},
{"aiplan::scoreLanesByObstacles", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::scoreLanesByCars", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::edgePenalty", true, true, 0},
{"aiplan::sumLaneVotes", true, true, 1},
{"aiplan::steerToLane", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::beginLanePlanning", true, false, 1},  // eax: what the code leaves no caller reads (void)
{"aiplan::chooseLane", true, true, 1},
{"aiplan::startHorn", true, true, 1},
{"aiplan::stopHorn", true, true, 1},
{"aiplan::stepTrafficLoading", true, true, 1},
{"aiplan::targetSpeedAt", true, true, 1, true, kNoHeap, true},
{"aiplan::roadKindAt", true, true, 1},
{"aiplan::cacheRoadWidth", true, true, 0},
{"aiplan::nodeGap", true, true, 2},
{"aiplan::nodeOffset", true, true, 2},
{"aiplan::nodesAheadBySpeed", true, true, 1},
{"aiplan::nodeLaneBits", true, true, 2},
{"aiplan::nodeHasLane", true, true, 2},
{"aiplan::laneBit", true, true, 1},
{"aiplan::roadSliceIndex", true, true, 1},
{"aiplan::carSlices", true, true, 1},
{"aiplan::fillTrackTuning", true, true, 4},
{"aiplan::setupTrackTuning", true, false, 0},  // eax: what the code leaves no caller reads (void)
{"aiplan::freeSpeedTable", true, true, 0},
{"aiplan::trafficSide", true, true, 0},
{"aiplan::roadIsWide", true, true, 0},
{"aiplan::trafficSpeedAt", true, true, 1, true, kNoHeap, true},
{"aiplan::tuningSpeed", true, true, 0, true, kNoHeap, true},
{"aiplan::tuningDistance", true, true, 0},
{"aiplan::tuningFirst", true, true, 0},
{"aiplan::nodeDirectionDot", true, true, 2, true, kNoHeap, true},
{"aiplan::nodeWithinRange", true, true, 3},
{"aiplan::carWithinReach", true, true, 2},
{"aiplan::alwaysOne", true, true, 0},
{"aiplan::carBoxedIn", true, true, 1},
{"aiplan::freeAnimationPaths", true, true, 0},
{"aiplan::findFreeAnimation", true, true, 0},
{"aiplan::startAnimation", true, true, 1},
{"aiplan::setAnimationMode", true, true, 4},
{"aiplan::stopAnimation", true, true, 1},
{"aiplan::animationPose", true, true, 4},
{"aiplan::animationPlace", true, true, 4},
{"aiplan::animationProgress", true, true, 3},
{"aiplan::animationLastPose", true, true, 4},
{"aiplan::pathEndPose", true, true, 4},
{"aiplan::animationStep", true, true, 3},
{"aiplan::animationPlaying", true, true, 1},
{"aiplan::startSound", true, false, 0},  // eax: what the code leaves no caller reads (void)
{"aiplan::restartSound", true, false, 1},  // eax: what the code leaves no caller reads (void)
#elif defined(AIPLAN_ARGUMENTS)
/* Where the cases keep their own: the cars (0xa00 each), the nodes, the speed
 * tables, the animation paths (0x400 each), the arguments' buffers. */
constexpr x86::reg32 kAipCars = 0xd80000, kAipNodes = 0xd88000, kAipSpeeds = 0xd8d000, kAipKinds = 0xd8d800;
constexpr x86::reg32 kAipPaths = 0xd8e000, kAipArgs = 0xd90000;
constexpr x86::reg32 kAipCarCount = 12, kAipNodeCount = 64, kAipMostNodes = 0x1c0;

x86::reg32 aipCar(x86::reg32 i)
{
    return kAipCars + i * 0xa00;
}

x86::reg32 aipBits(float f)
{
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float in [lo, hi); now and then 0 or -0. */
x86::reg32 aipFloat(float lo, float hi)
{
    if (chance(4))
        return chance(50) ? 0 : 0x80000000;
    return aipBits(lo + (hi - lo) * float(below(100000)) / 100000.0f);
}

/* A slice of the road's 14: in them mostly, past them now and then. */
x86::reg32 aipSlice()
{
    return chance(85) ? below(14) : x86::reg32(x86::sreg32(below(22)) - 4);
}

/* A node: of the track mostly, past it now and then. */
x86::reg32 aipNode(CheckApp& g)
{
    const x86::reg32 n = g.word(0x5dd940);
    return chance(92) ? below(n) : chance(50) ? n + below(4) : 0u - (1 + below(4));
}

/* A path of the animations': its points' count (2..12), frames per point
 * (1..400 mostly, 0 or past 400 now and then -- 6 then), its points' places
 * (16.16) and orientations (four shorts). */
void aipPath(CheckApp& g, x86::reg32 at)
{
    for (x86::reg32 i = 0; i < 0x400; i += 4)
        g.word(at + i) = random32();
    const x86::reg32 count = 2 + below(11);
    const x86::reg32 frames = chance(85) ? 1 + below(chance(50) ? 16 : 400) : chance(50) ? 0 : 401 + below(100);
    g.getMemory<x86::reg16>(at + 4) = x86::reg16(count);
    g.getMemory<x86::reg16>(at + 6) = x86::reg16(frames);
    for (x86::reg32 p = 0; p < count; ++p)
    {
        const x86::reg32 point = at + 8 + 0x14 * p;
        for (x86::reg32 k = 0; k < 3; ++k)
            g.word(point + 4 * k) = x86::reg32(x86::sreg32(below(0x2000000)) - 0x1000000);
        for (x86::reg32 k = 0; k < 4; ++k)
            g.getMemory<x86::reg16>(point + 0xc + 2 * k) = x86::reg16(x86::sreg32(below(0x8000)) - 0x4000);
    }
}

/* The world: the race's switches; the track's nodes; the speed tables; the
 * cars and their lists; the lane costs and the sensed cars; the tracks'
 * tuning records and the lane widths; the obstacles; the animation paths and
 * players; the sound's setup; the data at 0x40cb00. */
void makeAiplanWorld(CheckApp& g)
{
    static const x86::reg32 data[] = {0, 0, 0, 0, 0x640000, 0};
    for (x86::reg32 i = 0; i < 6; ++i)
        g.word(0x40cb00 + 4 * i) = data[i];
    // The race.
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x6fd4c8) = chance(70) ? 0 : 1;
    g.word(0x6fd4cc) = chance(70) ? 0 : 1;
    g.word(0x6fd4c4) = chance(70) ? 0 : 1;
    g.word(0x6fd4ac) = chance(90) ? below(9) : below(14);
    g.word(0x6fd3b8) = chance(90) ? below(4) : 4 + below(3);
    g.word(0x6fd470) = chance(60) ? 1 : below(3);
    g.word(0x6fd4f0) = chance(70) ? 0 : 1;
    g.getMemory<x86::reg8>(0x6fd4fc) = x86::reg8(random32());
    g.getMemory<x86::reg8>(0x55eb2e) = x86::reg8(random32());
    g.word(0x7d3684) = 0x1000 + below(0x100000);
    g.word(0x79c1a4) = random32();
    g.word(0x554f10) = random32();
    g.word(0x554f14) = random32();
    g.word(0x554f18) = chance(80) ? 0x41c64e6d : random32();
    g.word(0x56ebc0) = random32();
    // The nodes: their places, lane bits, directions, edge slices, halves'
    // widths and edges.
    g.word(0x5dd940) = chance(75) ? kAipNodeCount : chance(50) ? kAipMostNodes : 2 + below(kAipNodeCount - 2);
    g.word(0x5dd948) = kAipNodes;
    for (x86::reg32 i = 0; i < kAipMostNodes + 4; ++i)
    {
        const x86::reg32 node = kAipNodes + 0x24 * i;
        for (x86::reg32 at = node; at < node + 0x24; at += 4)
            g.word(at) = random32();
        if (chance(70))
            g.getMemory<x86::reg16>(node + 0xc) = x86::reg16(chance(50) ? 0xffff : random32());
        g.getMemory<x86::reg8>(node + 0x13) = x86::reg8(below(7) << 4 | below(7));
        g.getMemory<x86::reg8>(node + 0x17) = x86::reg8(chance(95) ? 4 + below(36) : random32());
        g.getMemory<x86::reg8>(node + 0x1b) = x86::reg8(chance(95) ? 4 + below(36) : random32());
        g.word(node + 0x1c) = x86::reg32(x86::sreg32(below(0x200000)) - 0x100000);
        g.word(node + 0x20) = x86::reg32(x86::sreg32(below(0x200000)) - 0x100000);
    }
    // The speed tables: a byte by half a node, a kind by node.
    g.word(0x552558) = chance(5) ? 0 : kAipSpeeds;
    g.word(0x55255c) = kAipKinds;
    for (x86::reg32 i = 0; i < 0x200; ++i)
    {
        g.getMemory<x86::reg8>(kAipSpeeds + i) = x86::reg8(below(256));
        g.getMemory<x86::reg8>(kAipKinds + i) = x86::reg8(random32());
    }
    g.word(0x56f22c) = chance(85) ? 0 : 1;
    g.word(0x56f228) = chance(50) ? 0 : 1;
    g.word(0x56f230) = chance(40) ? 1 : chance(50) ? 0xffffffff : below(3);
    g.word(0x56f220) = random32();
    g.word(0x56f224) = aipFloat(0.0f, 100.0f);
    // The cars: every field these read, sensible.
    const x86::reg32 count = 1 + below(kAipCarCount);
    g.word(0x5efd9c) = count;
    for (x86::reg32 i = 0; i < kAipCarCount; ++i)
    {
        const x86::reg32 car = aipCar(i);
        for (x86::reg32 at = car; at < car + 0xa00; at += 4)
            g.word(at) = chance(50) ? aipFloat(-50.0f, 50.0f) : random32();
        g.word(car + 8) = below(4000);
        g.word(car + 0x1c) = aipNode(g);
        g.getMemory<x86::reg8>(car + 0x89) = x86::reg8(chance(80) ? 1 : 0);
        g.word(car + 0x10c) = aipFloat(0.0f, 1.5f);
        g.word(car + 0x114) = aipFloat(0.5f, 3.0f);
        g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(70) ? 0 : 1);
        g.word(car + 0x1f0) = chance(80) ? i : below(kAipCarCount);
        g.word(car + 0x1fc) = below(300);
        g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
        g.word(car + 0x21c) = aipFloat(0.0f, 3.0f);
        g.getMemory<x86::reg8>(car + 0x620) = x86::reg8(random32());
        g.getMemory<x86::reg8>(car + 0x621) = x86::reg8(random32());
        g.word(car + 0x630) = chance(90) ? below(3) : below(6);
        for (const x86::reg32 at : {0x638u, 0x63cu, 0x640u})
            g.word(car + at) = aipSlice();
        g.word(car + 0x644) = g.word(0x7d3684) + x86::reg32(x86::sreg32(below(0x100)) - 0x80);
        g.word(car + 0x660) = below(4);
        g.word(car + 0x6dc) = chance(90) ? below(6) : below(8);
        g.getMemory<x86::reg8>(car + 0x710) = x86::reg8(random32());
        g.word(car + 0x734) = chance(60) ? 0 : 1;
        g.word(car + 0x7b0) = aipFloat(-60.0f, 60.0f);
        g.word(car + 0x7b4) = aipFloat(-10.0f, 10.0f);
        g.word(car + 0x7b8) = aipFloat(-10.0f, 10.0f);
        g.word(car + 0x7c0) = chance(10) ? aipFloat(-1.5f, 1.5f) : aipFloat(-60.0f, 60.0f);
        g.word(car + 0x7c4) = aipFloat(0.0f, 60.0f);
        g.word(car + 0x7d4) = chance(70) ? 0 : 1;
        g.word(0x5efac8 + 4 * i) = car;
    }
    // The players' cars (up to 4), the traffic (0x5efb48), the police (0x5efa88).
    const x86::reg32 players = chance(10) ? 0 : 1 + below(4);
    g.word(0x5efda4) = players;
    for (x86::reg32 i = 0; i < 4; ++i)
        g.word(0x5efa48 + 4 * i) = aipCar(below(kAipCarCount));
    g.word(0x56ebb8) = g.word(0x5efa48);
    const x86::reg32 traffic = below(kAipCarCount + 1);
    g.word(0x5efd8c) = traffic;
    for (x86::reg32 i = 0; i < kAipCarCount; ++i)
    {
        const x86::reg32 car = aipCar(below(kAipCarCount));
        g.word(0x5efb48 + 4 * i) = car;
        g.word(car + 0x6d8) = g.word(0x5efa48 + 4 * below(4));
    }
    const x86::reg32 police = below(9);
    g.word(0x5efd88) = police;
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x5efa88 + 4 * i) = aipCar(below(kAipCarCount));
    g.word(0x5efd94) = below(8);
    // The lane costs (14 rows of three), the sensed cars, the chosen lane.
    for (x86::reg32 at = 0x56ee90; at < 0x56ef38; at += 4)
        g.word(at) = x86::reg32(x86::sreg32(below(2000)) - 1000);
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g.word(0x56ef38 + 4 * i) = chance(30) ? 0 : aipCar(below(kAipCarCount));
        g.word(0x56ef44 + 4 * i) = chance(20) ? 7 : below(14);
        g.word(0x56ef50 + 4 * i) = chance(30) ? 0 : aipCar(below(kAipCarCount));
        g.word(0x56ef5c + 4 * i) = x86::reg32(0) - below(14);
        g.getMemory<x86::reg16>(0x56ef68 + 2 * i) = x86::reg16(chance(50) ? 0 : 1);
        g.getMemory<x86::reg16>(0x56ef6e + 2 * i) = x86::reg16(chance(50) ? 0 : 1);
    }
    g.word(0x56ef74) = aipSlice();
    g.word(0x56ef78) = chance(40) ? 0 : chance(50) ? 1 : 0xffffffff;
    g.word(0x56ef7c) = chance(60) ? 0 : 1;
    // The tracks' tuning records (11 of 0x20): small numbers and speeds.
    for (x86::reg32 t = 0; t < 11; ++t)
    {
        const x86::reg32 record = 0x56ef80 + 0x20 * t;
        g.word(record) = below(20);
        g.word(record + 4) = below(1000);
        g.word(record + 8) = below(2);
        g.word(record + 0xc) = chance(70) ? (chance(50) ? 1 : 0xffffffff) : 0;
        g.word(record + 0x10) = random32();
        g.word(record + 0x14) = aipFloat(0.5f, 1.2f);
        g.word(record + 0x18) = aipFloat(0.5f, 1.2f);
        g.word(record + 0x1c) = aipFloat(5.0f, 30.0f);
    }
    for (x86::reg32 i = 0; i < 0x50; ++i)
        g.word(0x56f0e0 + 4 * i) = aipFloat(0.1f, 6.0f);
    // The obstacles: live now and then, at a node and a slice near the cars'.
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 o = 0x7dd850 + 0x50 * i;
        for (x86::reg32 at = o; at < o + 0x50; at += 4)
            g.word(at) = random32();
        g.word(o) = chance(50) ? 0 : 1;
        g.word(o + 0x34) = aipNode(g);
        g.word(o + 0x38) = aipSlice();
    }
    // The paths (seven, some not loaded) and the players (mostly free).
    for (x86::reg32 i = 0; i < 7; ++i)
    {
        aipPath(g, kAipPaths + 0x400 * i);
        g.word(0x570a34 + 4 * i) = chance(20) ? 0 : kAipPaths + 0x400 * i;
    }
    const bool full = chance(10);
    for (x86::reg32 k = 0; k < 0x100; ++k)
    {
        const x86::reg32 slot = 0x56f234 + 0x18 * k;
        g.word(slot) = full || chance(15) ? kAipPaths + 0x400 * below(7) : 0;
        g.word(slot + 4) = g.word(0x7d3684) - below(chance(80) ? 0x400 : 0x4000);
        g.word(slot + 8) = random32();
        g.word(slot + 0xc) = below(4);
        g.word(slot + 0x10) = below(4);
        g.word(slot + 0x14) = random32();
    }
    // The sound's setup and switches.
    g.word(0x6fbc00) = below(5);
    g.word(0x6fbbdc) = below(3);
    g.word(0x6fbbe0) = below(2);
    g.word(0x55290c) = chance(70) ? 1 : 0;
    g.word(0x552910) = chance(70) ? 1 : 0;
    g.word(0x552918) = random32();
    g.word(0x5525a4) = below(2);
    // The arguments' buffers.
    for (x86::reg32 at = kAipArgs; at < kAipArgs + 0x8000; at += 4)
        g.word(at) = aipFloat(-50.0f, 50.0f);
}

/* A playing slot mostly, a free one now and then. */
x86::reg32 aipSlot(CheckApp& g, bool playing)
{
    for (int tries = 0; tries < 64; ++tries)
    {
        const x86::reg32 k = below(0x100);
        if ((g.word(0x56f234 + 0x18 * k) != 0) == playing)
            return k;
    }
    const x86::reg32 k = below(0x100);
    if (playing)
        g.word(0x56f234 + 0x18 * k) = kAipPaths;
    return k;
}

bool makeAiplanArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeAiplanWorld(g);
    const auto car = [&] { return aipCar(below(kAipCarCount)); };
    const auto buffer = [&] { return kAipArgs + 0x100 * below(0x70); };
    const auto path = [&] { return kAipPaths + 0x400 * below(7); };
    // The time into a path: within it mostly, past its end now and then.
    const auto time = [&](x86::reg32 p) {
        const x86::reg32 frames = g.getMemory<x86::reg16>(p + 6), count = g.getMemory<x86::reg16>(p + 4);
        const x86::reg32 per = frames >= 1 && frames <= 0x190 ? frames : 6;
        return below(per * count * (chance(80) ? 1 : 3) + 1);
    };
    cpu.eax = car();
    if (name == "nearPoliceCar")
        cpu.edx = below(40);
    else if (name == "planLaneChange" || name == "edgeLaneCost")
    {
        // edgeLaneCost's stretch of track 6 (nodes 0x127 to 0x168), for a
        // player's car in a forward race, now and then.
        if (name == "edgeLaneCost" && chance(30))
        {
            g.word(0x6fd4ac) = 6;
            g.word(0x6fd4c0) = 0;
            g.word(0x5dd940) = kAipMostNodes;
            g.getMemory<x86::reg8>(cpu.eax + 0x200) |= 1;
            g.word(cpu.eax + 0x1c) = 0x126 + below(0x45);
        }
        cpu.edx = chance(50) ? g.word(cpu.eax + 0x1c) : aipNode(g);
    }
    else if (name == "nodeOffset")
        cpu.edx = x86::reg32(x86::sreg32(below(41)) - 20);
    else if (name == "nodesAheadBySpeed")
    {
        stack[0] = aipFloat(0.5f, 3.0f);
        stack[1] = below(10);
        stack[2] = stack[1] + below(30);
    }
    else if (name == "nodeGap" || name == "nodeDirectionDot")
    {
        cpu.eax = aipNode(g);
        cpu.edx = aipNode(g);
    }
    else if (name == "nodeLaneBits")
    {
        cpu.eax = aipNode(g);
        cpu.edx = chance(50) ? 1u << below(16) : random32();
    }
    else if (name == "nodeHasLane")
    {
        cpu.eax = aipNode(g);
        cpu.edx = aipSlice();
    }
    else if (name == "laneBit")
        cpu.eax = aipSlice();
    else if (name == "roadSliceIndex")
    {
        cpu.eax = aipNode(g);
        stack[0] = aipFloat(-12.0f, 12.0f);
    }
    else if (name == "targetSpeedAt" || name == "roadKindAt")
        cpu.eax = below(g.word(0x5dd940) + 1);
    else if (name == "trafficSpeedAt")
    {
        // The stretches' ends, one either side now and then.
        static const x86::reg32 ends[] = {0x276, 0x1e8, 0x3e8, 0x316, 0xaa, 0x56e, 0x528,
                                          0x153, 0x127, 0x65e, 0x60d, 0x190};
        cpu.eax = chance(50) ? ends[below(12)] + below(3) - 1 : chance(90) ? below(0x700) : random32();
    }
    else if (name == "reactToTraffic")
    {
        // A car straight ahead in the lane, coming the other way, closing at
        // 15 to 30 now and then.
        if (chance(60))
        {
            const x86::reg32 other = car();
            g.word(0x56ef3c) = other;
            g.word(0x56ef48) = chance(80) ? 7 : below(14);
            g.getMemory<x86::reg8>(cpu.eax + 0x620) &= x86::reg8(chance(80) ? 0xf7 : 0xff);
            g.getMemory<x86::reg8>(cpu.eax + 0x621) |= x86::reg8(chance(80) ? 0x10 : 0);
            g.getMemory<x86::reg8>(other + 0x200) |= x86::reg8(chance(80) ? 4 : 0);
            const float mine = 2.0f + float(below(1500)) / 100.0f, closing = 15.0f + float(below(1500)) / 100.0f;
            const float sign = chance(50) ? 1.0f : -1.0f;
            g.word(cpu.eax + 0x7c0) = aipBits(sign * mine);
            g.word(other + 0x7c0) = aipBits(-sign * (closing - mine));
        }
    }
    else if (name == "nodeWithinRange")
    {
        cpu.eax = aipNode(g);
        cpu.edx = aipNode(g);
        cpu.ebx = x86::reg32(x86::sreg32(below(61)) - 30);
    }
    else if (name == "carWithinReach")
        cpu.edx = below(5000);
    else if (name == "fillTrackTuning")
    {
        cpu.eax = buffer();
        cpu.ebx = random32();
        cpu.ecx = random32();
        cpu.edx = random32();
    }
    else if (name == "startAnimation")
        cpu.eax = chance(90) ? below(7) : chance(50) ? 0x100 + below(4) : below(0x100);
    else if (name == "setAnimationMode")
    {
        cpu.eax = below(0x100);
        cpu.edx = below(4);
        cpu.ebx = below(4);
        cpu.ecx = random32();
    }
    else if (name == "stopAnimation" || name == "animationPlaying")
        cpu.eax = aipSlot(g, chance(70));
    else if (name == "animationProgress")
    {
        cpu.eax = aipSlot(g, true);
        cpu.edx = buffer();
        cpu.ebx = buffer() + 0x80;
    }
    else if (name == "animationStep")
    {
        cpu.eax = aipSlot(g, chance(85));
        cpu.edx = below(4);
        cpu.ebx = buffer();
    }
    else if (name == "animationPose" || name == "animationPlace")
    {
        cpu.eax = path();
        cpu.edx = below(4);
        cpu.ebx = time(cpu.eax);
        cpu.ecx = buffer();
        stack[0] = buffer();
        stack[1] = buffer() + 0x80;
    }
    else if (name == "animationLastPose")
    {
        cpu.eax = path();
        cpu.ecx = buffer();
        stack[0] = buffer() + 0x80;
    }
    else if (name == "pathEndPose")
    {
        cpu.eax = below(7);
        if (g.word(0x570a34 + 4 * cpu.eax) == 0)
            g.word(0x570a34 + 4 * cpu.eax) = path();
        cpu.ebx = random32();
        cpu.ecx = buffer();
        stack[0] = buffer() + 0x80;
    }
    else if (name == "restartSound")
        cpu.eax = below(5);
    return true;
}
#endif
