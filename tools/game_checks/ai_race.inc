// game_checks.cpp's parts for the police of a Hot Pursuit race -- the cops'
// driver, the pursuit, the pull over and the ticket, the roadblocks, the radio
// -- and a few racers' rules (native_ai_race.cpp), included in each table with
// the macro naming the part.
#if defined(AI_WINDOWS)
// {from, to}, -- what the functions write, compared
// The random numbers (0x554f10); the race's switch 0x55eb2e; the pursuits,
// the orders, the ramming rules, the tick's sizing up, the lanes' votes, the
// leader (0x56e410 to 0x56f230); the track's nodes' pointer; the cars'
// tables and counts (0x5efa48 to 0x5efda4); the race's switches (0x6fd3b8
// to 0x6fd520); the clock; the sirens (0x7dd850); the cases' own: the cars,
// their models, the track's nodes (0xd90000 on).
{0x554000, 0x555000}, {0x55e000, 0x55f000}, {0x56e000, 0x570000}, {0x5dd000, 0x5de000},
{0x5ef000, 0x5f0000}, {0x6fd000, 0x6fe000}, {0x7d3000, 0x7d4000}, {0x7dd000, 0x7de000},
{0xd90000, 0xda0000},
#elif defined(AI_EFFECTS)
kAiNodeGap,  // the gap between two nodes (0x40e120): a few nodes mostly, signed; now and then many, or any
kAiSpeed,    // a node's speed (0x40ed40): 0 to 60 onto the x87's stack, 0 now and then
kAiFlag,     // a yes or no of the opponents' driving's: 0 half the time, else 1 or 2
kAiNode,     // a node so many on (0x40e160): one of the track's
kAiTrackPlace,  // a place snapped to the track (0x4205a0): its place on the track (30 words) at esi, esi back
#elif defined(AI_EFFECT_CASES)
case kAiNodeGap:
    result = r % 5 < 2 ? x86::reg32(x86::sreg32((r >> 4) % 41) - 20)
             : r % 5 < 3 ? x86::reg32(x86::sreg32((r >> 4) % 121) - 60)
             : r % 10 != 9 ? x86::reg32(x86::sreg32((r >> 4) % 1001) - 500)
                           : random(id + 1);
    break;
case kAiSpeed:
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = r % 9 == 0 ? x86::Float(0.0) : x86::Float(double((r >> 4) % 6001) / 100.0);
    break;
case kAiFlag:
    result = r % 2 == 0 ? 0 : 1 + (r >> 4) % 2;
    break;
case kAiNode:
    result = word(0x5dd940) != 0 ? r % word(0x5dd940) : 0;
    break;
case kAiTrackPlace:
    // What the pull-over's spot is checked by: the place's +0x14 (0 now and
    // then), its polygon (+0x18: one of the cases' at 0xd9f000, whose second
    // byte is its kind, or none), its +0x6c (0 mostly).
    if (cpu.esi < kMemory - 0x78)
    {
        for (x86::reg32 i = 0; i < 30; ++i)
            word(cpu.esi + 4 * i) = random(id + 1 + i);
        word(cpu.esi + 0x14) = r % 4 == 0 ? 0 : 1 + (r >> 4) % 4;
        word(cpu.esi + 0x18) = (r >> 8) % 5 == 0 ? 0 : 0xd9f000 + 4 * ((r >> 12) % 16);
        word(cpu.esi + 0x6c) = (r >> 16) % 4 == 0 ? random(id + 0x40) : 0;
    }
    result = cpu.esi;
    break;
#elif defined(AI_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// After the others' rows (the first row of an address is the one used).
// What the cops call of the opponents' driving (0x405000 on, other parts'):
// stood in for, by address, so that they stay so once those are native --
// the gap between nodes, a node's speed, a node so many on, a node blocked,
// a driver ready, at a node, a ram's wait (yes or no) -- and those that
// steer, stop, set a timer, reset a driver and the radio's words, their
// register arguments logged.
{0x40e120, "the gap between nodes", kWatcom, 2, 0, kEax, -1, 0, kAiNodeGap},
{0x40ed40, "a node's speed", kWatcom, 1, 0, kEax, -1, 0, kAiSpeed},
{0x40e160, "a node so many on", kWatcom, 2, 0, kEax, -1, 0, kAiNode},
{0x405c80, "a node blocked", kWatcom, 1, 0, kEax, -1, 0, kAiFlag},
{0x40df10, "a driver ready", kWatcom, 1, 0, kEax, -1, 0, kAiFlag},
{0x40f000, "at a node", kWatcom, 3, 0, kEax, -1, 0, kAiFlag},
{0x40f050, "a ram's wait", kWatcom, 2, 0, kEax, -1, 0, kAiFlag},
{0x405fd0, "a driver's timer", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x405220, "a driver reset", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x40dd60, "steered at the target", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40ddb0, "the lanes chosen", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40de30, "steered", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40bff0, "clear of the others", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40ce70, "a car stopped", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40cd80, "a chase off", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40d720, "the lanes taken", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40d800, "the lanes ahead", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// (No register logged: aiplan::penaliseCloseCars reads none, and the cops' code calls it as C++.)
{0x40d9f0, "the lanes' edges", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x40deb0, "a rival looked for", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40de50, "a lane by the rival", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b220, "the radio: a cop joins", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b330, "the radio: a second cop", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b480, "the radio: a cop gives up", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b530, "the radio: the car again", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b670, "the radio: the pursuit over", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b720, "the radio: backup", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b7f0, "the radio: a roadblock up", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b860, "the radio: two cars", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b910, "the radio: a roadblock", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40b9c0, "the radio: the roadblock's side", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x40bac0, "the radio: spikes", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40bba0, "the radio: stopping", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40bc30, "the radio: the car stopped", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40bca0, "the radio: the car passed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x40bd00, "the radio: missed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The game's others: a key's state (a yes or no), a car parked and settled,
// a siren off and on, two matrices multiplied (the rotation at edx logged,
// ea::yRotation's, which runs as it is); and natives of other parts that
// would walk their own worlds: a place snapped to the track (the place
// logged, its place on the track left at esi), a car's radio, a player's
// cop radio, a cop chase's sound.
{0x4be1a0, "a key's state", kWatcom, 2, 0, kEax, -1, 0, kAiFlag},
{0x4a0980, "a car parked", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x4a2690, "a car settled", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4c9f50, "a siren off", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4c9dd0, "a siren on", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x4e0280, "matrices multiplied", kWatcom, 3, 0, kEax, 1, 9, kNothing},
{0x4205a0, "trackdata::snapToTrack", kWatcom, 2, 0, kEax, 0, 3, kAiTrackPlace},
{0x4127c0, "snd::carRadio", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x412880, "snd::playerCopRadio", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x411750, "snd::copChaseSet", kWatcom, 4, 0, kEax, -1, 0, kNothing},
// This file's own that racestart's natives call by address (a car's update
// asks whether it is pulled over, its hits stop it for a cop and touch the
// others): stood in for them as before they were native, since their worlds
// have no pursuits.  Each tested on its own.
{0x401500, "airace::pulledOver", kWatcom, 1, 0, kEax, -1, 0, kAiFlag},
{0x401650, "airace::stopForCop", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x404a00, "airace::carContact", kWatcom, 1, 0, kEax, -1, 0, kNothing},
#elif defined(AI_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(AI_PROMISES)
// {"airace::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, and the x87's stack in use.
{"airace::nothingAtStart", true, true, 0, true, kNoHeap, true},
{"airace::nothingAtLoad", true, true, 0, true, kNoHeap, true},
{"airace::releaseEdge", true, true, 0, true, kNoHeap, true},
{"airace::setUpCops", true, true, 0, true, kNoHeap, true},
{"airace::radioOn", true, true, 0, true, kNoHeap, true},
{"airace::radioLevelDown", true, true, 0, true, kNoHeap, true},
{"airace::radioLevelUp", true, true, 0, true, kNoHeap, true},
{"airace::copTopSpeed", true, true, 0, true, kNoHeap, true},
{"airace::nearestCopGap", true, true, 0, true, kNoHeap, true},
{"airace::setWantedSpeed", true, true, 0, true, kNoHeap, true},
{"airace::noteSpeeding", true, true, 0, true, kNoHeap, true},
{"airace::pulledOver", true, true, 0, true, kNoHeap, true},
{"airace::copRadioCall", true, true, 0, true, kNoHeap, true},
{"airace::ticketRadio", true, true, 0, true, kNoHeap, true},
{"airace::stopIfCopNear", true, true, 0, true, kNoHeap, true},
{"airace::stopForCop", true, true, 0, true, kNoHeap, true},
{"airace::sizeUpTarget", true, true, 0, true, kNoHeap, true},
{"airace::joinPursuit", true, true, 0, true, kNoHeap, true},
{"airace::leaveCopList", true, true, 0, true, kNoHeap, true},
{"airace::giveUpPursuit", true, true, 0, true, kNoHeap, true},
{"airace::pullOverSpot", true, true, 0, true, kNoHeap, true},
{"airace::planPullOver", true, true, 0, true, kNoHeap, true},
{"airace::endPullOver", true, true, 0, true, kNoHeap, true},
{"airace::copStopsBy", true, true, 0, true, kNoHeap, true},
{"airace::stopOver", true, true, 0, true, kNoHeap, true},
{"airace::giveTicket", true, true, 0, true, kNoHeap, true},
{"airace::targetStopping", true, true, 0, true, kNoHeap, true},
{"airace::chaseSpeed", true, true, 0, true, kNoHeap, true},
{"airace::pursueTarget", true, true, 0, true, kNoHeap, true},
{"airace::votePursuitLane", true, true, 0, true, kNoHeap, true},
{"airace::sideTowardNode", true, true, 0, true, kNoHeap, true},
{"airace::keepSideOfTarget", true, true, 0, true, kNoHeap, true},
{"airace::placeCopAtStart", true, true, 0, true, kNoHeap, true},
{"airace::placeCop", true, true, 0, true, kNoHeap, true},
{"airace::alwaysOne", true, true, 0, true, kNoHeap, true},
{"airace::carReversing", true, true, 0, true, kNoHeap, true},
{"airace::copHitByCar", true, true, 0, true, kNoHeap, true},
{"airace::voteRoadLane", true, true, 0, true, kNoHeap, true},
{"airace::spotSpeeder", true, true, 0, true, kNoHeap, true},
{"airace::laneOffRoad", true, true, 0, true, kNoHeap, true},
{"airace::atOrderNode", true, true, 0, true, kNoHeap, true},
{"airace::sideOfOrder", true, true, 0, true, kNoHeap, true},
{"airace::takeOrder", true, true, 0, true, kNoHeap, true},
{"airace::parkCop", true, true, 0, true, kNoHeap, true},
{"airace::drivePatrol", true, true, 0, true, kNoHeap, true},
{"airace::copDriver", true, true, 0, true, kNoHeap, true},
{"airace::giveOrder", true, true, 0, true, kNoHeap, true},
{"airace::spreadCops", true, true, 0, true, kNoHeap, true},
{"airace::radioStopped", true, true, 0, true, kNoHeap, true},
{"airace::radioStop", true, true, 0, true, kNoHeap, true},
{"airace::initPursuits", true, true, 0, true, kNoHeap, true},
{"airace::callRoadblock", true, true, 0, true, kNoHeap, true},
{"airace::callTwoCars", true, true, 0, true, kNoHeap, true},
{"airace::roadblockSiren", true, true, 0, true, kNoHeap, true},
{"airace::watchRoadblock", true, true, 0, true, kNoHeap, true},
{"airace::watchSecondCar", true, true, 0, true, kNoHeap, true},
{"airace::copsWanted", true, true, 0, true, kNoHeap, true},
{"airace::radioJoined", true, true, 0, true, kNoHeap, true},
{"airace::radioGiveUp", true, true, 0, true, kNoHeap, true},
{"airace::releaseCops", true, true, 0, true, kNoHeap, true},
{"airace::dropCop", true, true, 0, true, kNoHeap, true},
{"airace::sendCopAhead", true, true, 0, true, kNoHeap, true},
{"airace::callBackup", true, true, 0, true, kNoHeap, true},
{"airace::orderTaken", true, true, 0, true, kNoHeap, true},
{"airace::nothingForCop", true, true, 0, true, kNoHeap, true},
{"airace::classRule", true, true, 0, true, kNoHeap, true},
{"airace::nothingAtEnd", true, true, 0, true, kNoHeap, true},
{"airace::nothingAtEndToo", true, true, 0, true, kNoHeap, true},
{"airace::setRammingRules", true, true, 0, true, kNoHeap, true},
{"airace::classValue", true, true, 0, true, kNoHeap, true},
{"airace::classValueToo", true, true, 0, true, kNoHeap, true},
{"airace::rivalTick", true, true, 0, true, kNoHeap, true},
{"airace::carContact", true, true, 0, true, kNoHeap, true},
{"airace::slowForClass", true, true, 0, true, kNoHeap, true},
{"airace::nothingAtStartToo", true, true, 0, true, kNoHeap, true},
{"airace::catchUpSpeed", true, true, 0, true, kNoHeap, true},
{"airace::findLeader", true, true, 0, true, kNoHeap, true},
#elif defined(AI_ARGUMENTS)
/* Where the police's cases keep their own: 16 cars of 0xa00 bytes (the
 * racers first, then the cops), their models (0x200 each, the physics' at
 * +0x100), the track's nodes. */
constexpr x86::reg32 kArCars = 0xd90000, kArModels = 0xd9a000, kArNodes = 0xd9c000;
constexpr x86::reg32 kArCarCount = 16, kArMaxNodes = 256;
// The game's, as native_ai_race.cpp names them.
constexpr x86::reg32 kArClock = 0x7d3684, kArNodeCount = 0x5dd940;
constexpr x86::reg32 kArCops = 0x5efa88, kArCopCount = 0x5efd88, kArRacers = 0x5efc48, kArRacerCount = 0x5efda0;
constexpr x86::reg32 kArPursuits = 0x56e410, kArOrders = 0x56ea10, kArSirens = 0x7dd850;

struct ArWorld
{
    x86::reg32 racers, cops, nodes, clock;
};
ArWorld g_ar;

x86::reg32 arCar(x86::reg32 i)
{
    return kArCars + i * 0xa00;
}

x86::reg32 arFloatOf(float f)
{
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float in [lo, hi), 0 now and then. */
x86::reg32 arFloat(float lo, float hi)
{
    if (chance(5))
        return 0;
    return arFloatOf(lo + (hi - lo) * float(below(100000)) / 100000.0f);
}

/* One of the race's cars, a racer or a cop; a racer; a cop. */
x86::reg32 arAnyCar()
{
    const x86::reg32 k = below(g_ar.racers + g_ar.cops);
    return arCar(k < g_ar.racers ? k : 6 + k - g_ar.racers);
}

x86::reg32 arRacer()
{
    return arCar(below(g_ar.racers));
}

x86::reg32 arCop()
{
    return arCar(6 + below(g_ar.cops));
}

x86::reg32 arPursuit(x86::reg32 racer)
{
    return kArPursuits + 0x60 * racer;
}

x86::reg32 arTime()
{
    return g_ar.clock + below(0x800) - 0x400;
}

x86::reg32 arNode()
{
    return below(g_ar.nodes);
}

/* The police's world: the race's clock and switches; the track's nodes; up
 * to 6 racers (cars 0..5) each with its pursuit, up to 8 cops (cars 6..13)
 * each with its order, the cars' tables; the cops on the pursuits, their
 * sirens, the ramming rules, the tick's sizing up and votes. */
void makeAiRaceWorld(CheckApp& g)
{
    // The cars there are; the tables' counts are 0 now and then.
    g_ar.racers = 1 + below(6);
    g_ar.cops = 1 + below(8);
    g_ar.nodes = chance(85) ? 60 + below(kArMaxNodes - 60) : 1 + below(60);
    g_ar.clock = chance(15) ? 0x481 + below(0x3e) : 0x400 + below(0x4000);
    // The race's clock and switches.
    g.word(kArClock) = g_ar.clock;
    g.word(0x6fd4c0) = chance(30) ? 1 : 0;
    g.word(0x6fd4bc) = 2 + below(3);
    g.word(0x6fd4b8) = below(5);
    g.word(0x6fd4c8) = chance(50) ? 1 : 0;
    g.word(0x6fd4f0) = chance(50) ? 1 : below(3);
    g.getMemory<x86::reg8>(0x6fd4fc) = x86::reg8(random32());
    g.word(0x6fd51c) = chance(70) ? 0 : 1;
    g.word(0x6fd518) = below(4);
    g.word(0x6fd520) = below(4);
    g.word(0x6fd3b8) = below(4);
    g.getMemory<x86::reg8>(0x55eb2e) = x86::reg8(random32() & (chance(70) ? 0xdf : 0xff));
    g.word(0x554f10) = below(0x10000);
    g.word(0x554f18) = random32() | 1;
    // handling.inc's mark (0xd13ffc): its dot product's small results.
    g.word(0xd13ffc) = x86::reg32(g_seed);
    // The track's nodes: their places in fixed point, their bytes.
    g.word(kArNodeCount) = g_ar.nodes;
    g.word(0x5dd948) = kArNodes;
    for (x86::reg32 i = 0; i < kArMaxNodes; ++i)
    {
        const x86::reg32 node = kArNodes + 0x24 * i;
        for (x86::reg32 k = 0; k < 3; ++k)
            g.word(node + 4 * k) = x86::reg32(x86::sreg32(below(0x8000000)) - 0x4000000);
        for (x86::reg32 at = node + 0xc; at < node + 0x24; ++at)
            g.getMemory<x86::reg8>(at) = x86::reg8(random32());
        g.getMemory<x86::reg8>(node + 0x13) = x86::reg8(below(7) << 4 | below(7));
    }
    // The models: a fast one's mark (+0x14 0x1f8), the class (+0x30), the top speed.
    for (x86::reg32 m = 0; m < 8; ++m)
    {
        const x86::reg32 model = kArModels + 0x200 * m;
        for (x86::reg32 at = model; at < model + 0x200; at += 4)
            g.word(at) = random32();
        g.word(model + 0x14) = chance(30) ? 0x1f8 : below(0x300);
        g.word(model + 0x30) = below(12);  // the classes' tables have 12 (0x55226c..)
        g.word(model + 0x100 + 0x1c8) = arFloat(20.0f, 80.0f);
    }
    // The cars' tables: the racers, the cops, all, the players' and the others.
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        g.word(kArRacers + 4 * i) = arCar(i < 6 ? i : below(6));
        g.word(kArCops + 4 * i) = arCar(6 + (i < 8 ? i : below(8)));
        g.word(0x5efcc8 + 4 * i) = arCar(i);
        g.word(0x5efa48 + 4 * i) = arCar(chance(80) ? i % 6 : below(kArCarCount));
        g.word(0x5efb88 + 4 * i) = arCar(below(kArCarCount));
    }
    g.word(kArRacerCount) = chance(5) ? 0 : g_ar.racers;
    g.word(kArCopCount) = chance(5) ? 0 : g_ar.cops;
    g.word(0x5efd9c) = chance(80) ? g_ar.racers + g_ar.cops : below(17);
    g.word(0x5efda4) = chance(10) ? 0 : 1 + below(g_ar.racers == 0 ? 1 : g_ar.racers);
    g.word(0x5efd94) = chance(20) ? 0 : below(17);
    g.word(0x5efac8) = arCar(0);
    // The cars.
    for (x86::reg32 k = 0; k < kArCarCount; ++k)
    {
        const x86::reg32 car = arCar(k);
        const bool cop = k >= 6;
        for (x86::reg32 at = car; at < car + 0xa00; at += 4)
            g.word(at) = chance(70) ? arFloat(-50.0f, 50.0f) : random32();
        // Its progress (+8): laps for the leader's count, nodes for the
        // cops' lap against the leader's (60 apart).
        g.word(car + 8) = chance(50) ? below(6) : below(400);
        g.word(car + 0x1c) = arNode();
        g.word(car + 0x10c) = arFloat(-2.0f, 3.0f);
        g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(70) ? 0 : random32());
        g.word(car + 0x168) = arFloat(0.0f, 30.0f);
        g.word(car + 0x16c) = chance(20) ? 0 : arAnyCar();
        g.word(car + 0x1f0) = below(8);
        g.word(car + 0x1f4) = below(4);
        x86::reg8 flags = 0;
        if (cop)
            flags = x86::reg8((chance(20) ? 4 : 0) | (chance(20) ? 0x20 : 0) | (chance(10) ? 1 : 0)
                              | (chance(20) ? 2 : 0) | (chance(15) ? 8 : 0));
        else
            flags = x86::reg8((chance(90) ? 1 : 0) | (chance(40) ? 4 : 0) | (chance(30) ? 8 : 0)
                              | (chance(15) ? 0x20 : 0) | (chance(20) ? 2 : 0));
        g.getMemory<x86::reg8>(car + 0x200) = flags;
        g.word(car + 0x210) = chance(10) ? 0 : x86::reg32(x86::sreg32(below(1024)) - 512);
        const x86::reg32 model = kArModels + 0x200 * below(8);
        g.word(car + 0x220) = model;
        g.word(car + 0x528) = model + 0x100;
        g.word(car + 0x2c4) = chance(70) ? below(3) : 3 + below(2);
        g.word(car + 0x2c8) = below(5);
        g.word(car + 0x2cc) = below(5);
        g.word(car + 0x4f4) = arFloat(-1.0f, 1.0f);
        g.getMemory<x86::reg8>(car + 0x508) = x86::reg8(below(24));
        g.getMemory<x86::reg8>(car + 0x50d) = x86::reg8(chance(30) ? 1 : 0);
        g.word(car + 0x5f4) = chance(50) ? 0 : 1;
        g.word(car + 0x62c) = below(4);
        g.word(car + 0x634) = arTime();
        g.word(car + 0x638) = below(14);
        g.word(car + 0x660) = chance(50) ? 0 : 1;
        g.word(car + 0x6e0) = chance(40) ? 6 + below(2) : below(9);
        g.word(car + 0x6e4) = chance(50) ? 0 : x86::reg32(x86::sreg32(below(106)) - 5);
        g.word(car + 0x6e8) = below(20);
        g.word(car + 0x6f0) = chance(10) ? 0x40000000 : arFloat(-1.0f, 8.0f);
        g.word(car + 0x70c) = below(13);
        g.word(car + 0x714) = cop ? k - 6 : below(16);
        g.word(car + 0x71c) = below(9);
        for (const x86::reg32 at : {0x720u, 0x724u, 0x728u, 0x72cu})
            g.word(car + at) = chance(30) ? 0 : arTime();
        g.word(car + 0x730) = chance(50) ? 0 : arRacer();
        g.word(car + 0x734) = below(2);
        g.word(car + 0x7ac) = kArOrders + 8 * (cop ? k - 6 : below(8));
        g.word(car + 0x7b0) = arFloat(-60.0f, 60.0f);
        g.word(car + 0x7c0) = arFloat(-60.0f, 60.0f);
        g.word(car + 0x7c4) = chance(15) ? arFloat(0.0f, 1.5f) : arFloat(0.0f, 60.0f);
        g.word(car + 0x7d4) = chance(30) ? 1 : chance(80) ? 0 : 2;
        g.word(car + 0x9a4) = chance(50) ? 0 : arAnyCar();
        // A racer has its pursuit; a cop chases a racer now and then, on its pursuit.
        if (!cop)
        {
            g.word(car + 0x628) = chance(90) ? arPursuit(k) : 0;
            g.word(car + 0x624) = chance(20) ? arAnyCar() : 0;
        }
        else if (chance(60))
        {
            const x86::reg32 racer = below(g_ar.racers);
            g.word(car + 0x624) = arCar(racer);
            g.word(car + 0x628) = arPursuit(racer);
        }
        else
        {
            g.word(car + 0x624) = 0;
            g.word(car + 0x628) = chance(20) ? arPursuit(below(6)) : 0;
        }
        g.word(car + 0x6d8) = arAnyCar();
    }
    // The pursuits: their counts and times, the cops on them.
    for (x86::reg32 i = 0; i < 6; ++i)
    {
        const x86::reg32 p = arPursuit(i);
        for (x86::reg32 at = p; at < p + 0x60; at += 4)
            g.word(at) = below(8);
        g.word(p + 0x14) = chance(50) ? 0 : below(3);
        g.word(p + 0x1c) = below(3);
        g.word(p + 0x20) = g_ar.clock - below(0x2000);
        g.word(p + 0x24) = g_ar.clock - below(0x400);
        g.word(p + 0x28) = arTime();
        const x86::reg32 n = below(g_ar.cops < 6 ? g_ar.cops + 1 : 6);
        g.word(p + 0x2c) = n;
        for (x86::reg32 s = 0; s < 8; ++s)
            g.word(p + 0x30 + 4 * s) = s < n ? arCop() : 0;
        g.word(p + 0x50) = below(3);
        g.word(p + 0x54) = random32() & 0xf;
        g.word(p + 0x58) = chance(50) ? 0 : arCop();
        g.word(p + 0x5c) = chance(50) ? 0 : arCop();
    }
    // The cops' orders: a kind, taken, a node, a word, a side, a flag.
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 o = kArOrders + 8 * i;
        g.getMemory<x86::reg8>(o) = x86::reg8(below(6));
        g.getMemory<x86::reg8>(o + 1) = x86::reg8(below(2));
        g.getMemory<x86::reg16>(o + 2) = x86::reg16(arNode());
        g.getMemory<x86::reg16>(o + 4) = x86::reg16(chance(50) ? arNode() : below(10));
        g.getMemory<x86::reg8>(o + 6) = x86::reg8(below(2));
        g.getMemory<x86::reg8>(o + 7) = x86::reg8(below(2));
    }
    // The stopping speed, the tick's sizing up, the ramming rules, the
    // catch-up speed, the leader.
    g.word(0x56ea50) = chance(80) ? 0x4132cfdf : arFloat(0.0f, 30.0f);
    g.word(0x56ea54) = below(2);
    g.word(0x56ea58) = below(2);
    g.word(0x56ea5c) = below(2);
    g.word(0x56ea60) = chance(10) ? 0 : x86::reg32(chance(70) ? x86::sreg32(below(61)) - 30 : x86::sreg32(below(241)) - 120);
    g.word(0x56ea64) = chance(95) ? below(3) : 3;
    g.word(0x56ea68) = below(2);
    for (x86::reg32 at = 0x56ea70; at < 0x56ebb0; at += 4)
        g.word(at) = below(40);
    g.word(0x56ebb0) = arFloat(0.0f, 30.0f);
    g.word(0x56ebb4) = below(3);
    g.word(0x56ebb8) = arAnyCar();
    // The lanes' votes, the rival, the leader's car, the order's side.
    for (x86::reg32 at = 0x56eeb4; at < 0x56ef40; at += 4)
        g.word(at) = below(200);
    g.word(0x56ef3c) = chance(30) ? 0 : arAnyCar();
    g.word(0x56ef48) = below(3);
    g.word(0x56ef54) = chance(50) ? arRacer() : arAnyCar();
    g.word(0x56f228) = chance(30) ? 1 : 0;
    g.word(0x56f230) = chance(40) ? 1 : chance(60) ? 0xffffffff : 0;
    // The polygons a place on the track points at (kAiTrackPlace): their
    // second byte's low six bits their kind, 14 or another, 0 now and then.
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        g.word(0xd9f000 + 4 * i) = random32();
        g.getMemory<x86::reg8>(0xd9f000 + 4 * i + 1) = x86::reg8(chance(30) ? 0xe : chance(20) ? 0 : random32());
    }
    // The sirens: on or off, their node.
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 s = kArSirens + 0x50 * i;
        for (x86::reg32 at = s; at < s + 0x50; at += 4)
            g.word(at) = random32();
        g.word(s) = chance(50) ? 0 : 1;
        g.word(s + 0x34) = arNode();
    }
}

bool makeAiRaceArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeAiRaceWorld(g);
    // The x87: a control word of every precision and rounding, set as the
    // game sets it (the generated __CHP sets the word back through
    // setControl, the native leaves it); one or none on its stack.
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10));
    cpu.fpu.count = below(2);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(double(x86::sreg32(below(20001)) - 10000) / 64.0);
    const auto cop = [&] { return arCop(); };
    const auto racer = [&] { return arRacer(); };
    // A cop on a pursuit, its target and its pursuit.
    const auto chasingCop = [&] {
        const x86::reg32 c = arCop();
        if (chance(85))
        {
            const x86::reg32 r = below(g_ar.racers);
            g.word(c + 0x624) = arCar(r);
            g.word(c + 0x628) = arPursuit(r);
        }
        return c;
    };
    if (name == "watchRoadblock" || name == "watchSecondCar")
    {
        // A cop set on a racer, the roadblock of that racer's pursuit now and then.
        cpu.eax = cop();
        const x86::reg32 r = below(g_ar.racers);
        g.word(cpu.eax + 0x730) = arCar(r);
        if (g.word(arCar(r) + 0x628) != 0 && chance(50))
            g.word(g.word(arCar(r) + 0x628) + 0x58) = cpu.eax;
    }
    else if (name == "copTopSpeed" || name == "leaveCopList" || name == "giveUpPursuit" || name == "placeCopAtStart"
        || name == "placeCop" || name == "spotSpeeder" || name == "atOrderNode" || name == "sideOfOrder"
        || name == "takeOrder" || name == "parkCop" || name == "radioGiveUp" || name == "dropCop"
        || name == "roadblockSiren" || name == "orderTaken"
        || name == "radioStop")
        cpu.eax = cop();
    else if (name == "sizeUpTarget" || name == "chaseSpeed" || name == "pursueTarget" || name == "votePursuitLane"
             || name == "keepSideOfTarget" || name == "targetStopping" || name == "copStopsBy"
             || name == "stopOver" || name == "giveTicket" || name == "radioStopped"
             || name == "callRoadblock" || name == "callTwoCars" || name == "radioJoined"
             || name == "releaseCops" || name == "callBackup")
    {
        cpu.eax = chasingCop();
        // The free cops the radio's calls look for: now and then all of
        // them free (no one chased, no order's marks, none a player's).
        if ((name == "callRoadblock" || name == "callTwoCars" || name == "callBackup") && chance(50))
            for (x86::reg32 i = 0; i < 8; ++i)
            {
                const x86::reg32 c = arCar(6 + i);
                if (c == cpu.eax)
                    continue;
                g.word(c + 0x624) = 0;
                g.getMemory<x86::reg8>(c + 0x711) &= 0xf3;
                g.getMemory<x86::reg8>(c + 0x200) &= 0xfb;
            }
        // A pursuit with its target and a cop on it for those that need one.
        if (name == "copStopsBy" || name == "giveTicket" || name == "radioStopped" || name == "radioJoined"
            || name == "releaseCops")
            if (g.word(cpu.eax + 0x628) == 0)
            {
                g.word(cpu.eax + 0x624) = arCar(0);
                g.word(cpu.eax + 0x628) = arPursuit(0);
            }
        if (name == "copStopsBy" && chance(50))
        {
            // The cop among the pursuit's cops, the first to stop.
            const x86::reg32 p = g.word(cpu.eax + 0x628);
            if (p != 0)
            {
                g.word(p + 0x50) = chance(70) ? 0 : 1;
                g.word(p + 0x30 + 4 * below(g.word(p + 0x2c) == 0 ? 1 : g.word(p + 0x2c))) = cpu.eax;
            }
        }
    }
    else if (name == "copDriver")
        cpu.eax = chance(50) ? chasingCop() : cop();
    else if (name == "nearestCopGap" || name == "stopIfCopNear" || name == "pulledOver" || name == "endPullOver"
             || name == "voteRoadLane" || name == "laneOffRoad" || name == "carReversing" || name == "classRule"
             || name == "setRammingRules" || name == "rivalTick" || name == "carContact" || name == "slowForClass"
             || name == "findLeader" || name == "releaseEdge" || name == "radioLevelDown" || name == "radioLevelUp")
        cpu.eax = chance(70) ? racer() : arAnyCar();
    else if (name == "noteSpeeding")
        cpu.eax = chance(10) ? 0 : arAnyCar();
    else if (name == "setWantedSpeed" || name == "drivePatrol")
    {
        cpu.eax = chance(80) ? cop() : arAnyCar();
        stack[0] = arFloat(0.5f, 2.5f);
        stack[1] = chance(50) ? 0 : arFloat(-10.0f, 10.0f);
    }
    else if (name == "copRadioCall" || name == "ticketRadio" || name == "copHitByCar" || name == "sendCopAhead")
    {
        cpu.eax = name == "copHitByCar" ? (chance(50) ? chasingCop() : cop()) : cop();
        cpu.edx = chance(80) ? racer() : arAnyCar();
    }
    else if (name == "copsWanted")
    {
        cpu.eax = chance(30) ? 0 : cop();
        cpu.edx = chance(80) ? racer() : arAnyCar();
        if (cpu.eax != 0 && chance(30))
            g.word(cpu.eax + 0x730) = cpu.edx;
    }
    else if (name == "stopForCop")
    {
        cpu.eax = chance(80) ? racer() : arAnyCar();
        cpu.edx = below(g_ar.cops);
    }
    else if (name == "joinPursuit")
    {
        // Its kind: 0 to 3 as the game's calls have them, now and then one
        // past (counted at the pursuit's +0x14 to +0x20).
        cpu.eax = chance(90) ? below(4) : 4 + below(4);
        cpu.edx = cop();
        cpu.ebx = racer();
        if (g.word(cpu.ebx + 0x628) == 0)
            g.word(cpu.ebx + 0x628) = arPursuit((cpu.ebx - kArCars) / 0xa00);
        if (chance(70))
            g.word(cpu.edx + 0x628) = 0;
        stack[0] = arFloat(-10.0f, 60.0f);
    }
    else if (name == "pullOverSpot")
    {
        cpu.eax = arAnyCar();
        cpu.edx = chance(50) ? 0 : chance(80) ? 1 : random32();
    }
    else if (name == "planPullOver")
        cpu.edx = arAnyCar();
    else if (name == "sideTowardNode")
    {
        cpu.eax = arAnyCar();
        cpu.edx = arNode();
    }
    else if (name == "giveOrder")
    {
        cpu.eax = chance(80) ? cop() : arAnyCar();
        cpu.edx = chance(20) ? 0 : arAnyCar();
        cpu.ebx = random32();
        cpu.ecx = random32();
        stack[0] = random32();
        stack[1] = random32();
        stack[2] = random32();
    }
    else if (name == "classValue" || name == "classValueToo")
        cpu.eax = kArModels + 0x200 * below(8);
    return true;
}
#endif
