// game_checks.cpp's parts for Hot Pursuit's police (native_pursuit.cpp), included in each table with the macro
// naming the part.
#if defined(PURSUIT_WINDOWS)
// {from, to}, -- what the functions write, compared
// The pursuit's classes and the radio's state (0x552274.., 0x5524c8..), the
// chases' and the cars' tables (0x553078, 0x5efa88.., 0x5efc08.., 0x5efc88..),
// the traffic's random numbers (0x554f10), the race's (0x6fd3a0.., the
// colours 0x6fd54c..), the radio's path (0x7a28b4) and the network's switch
// (0x7a22da), the frame counters, the radio's load (0x7dd378); the cases'
// own: the cars, their choices, handlings and records, the nodes, buffers.
{0x553000, 0x554000}, {0x56f000, 0x570000}, {0x5dd000, 0x5de000}, {0x5ef000, 0x5f0000}, {0x7a2000, 0x7a3000},
{0x7d3000, 0x7d4000}, {0x7dd000, 0x7de000}, {0xcb0000, 0xcb9000},
#elif defined(PURSUIT_EFFECTS)
kPursuitFlag,    // a yes or no of the pursuit's (a cop near, within reach, ...): 0 half the time, else 1 to 3
kPursuitYes,     // the cop active: 1 mostly, 0 now and then
kPursuitKind,    // the chase's kind (0x404610): 0, 1, 2, now and then more
kPursuitNode,    // a node of the case's track: 0 to 63
kPursuitLane,    // a node's lane (0x40e0c0): 1 to 15 mostly, 0 now and then
kPursuitSpeed,   // a node's speed onto the x87's stack (0x40ed40): 0 to 100
kPursuitPace,    // the track's pace onto the x87's stack (0x40ee80): 0.5 to 1.5
kPursuitRadar,   // the radar's word (0x4c8040): its record at eax, 1 mostly; 0 now and then
kPursuitPlace,   // a place named (0x4c8550, 0x4c8470): its record at edx -- phrase, distance, kind -- or none
kPursuitSmall,   // a count (0x40ef10): 0 to 11
kPursuitLine,    // a speech's line queued (0x4c85a0): the words the code wrote of its record logged
kPursuitDebug,   // a debug text (0x42d520): its arguments past the five logged, one for each % of its format
#elif defined(PURSUIT_EFFECT_CASES)
case kPursuitFlag:
    result = r % 2 == 0 ? 0 : 1 + (r >> 8) % 3;
    break;
case kPursuitYes:
    result = r % 5 == 0 ? 0 : 1;
    break;
case kPursuitKind:
    result = r % 8 == 7 ? (r >> 8) % 16 : r % 3;
    break;
case kPursuitNode:
    result = (r >> 4) % 64;
    break;
case kPursuitLane:
    result = r % 6 == 0 ? 0 : 1 + (r >> 4) % 15;
    break;
case kPursuitSpeed:
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(double(r % 100000) / 1000.0);
    break;
case kPursuitPace:
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(0.5 + double(r % 1000) / 1000.0);
    break;
case kPursuitRadar:
{
    // radarFactor's frame record (0x20 bytes at eax): +8 the car's speed (a
    // float, now and then past 3), +0xc how far ahead (a signed short, -60
    // to 60 mostly), +0x10 its kind (1 mostly).
    const x86::reg32 record = words[0];
    if (record < kMemory - 0x20)
    {
        const float speed = float(random(id + 1) % 6000) / 1000.0f;
        std::memcpy(&getMemory<x86::reg8>(record + 8), &speed, 4);
        const x86::reg32 ahead = random(id + 2) % 10 == 0 ? random(id + 3) : x86::reg32(x86::sreg32(random(id + 3) % 121) - 60);
        getMemory<x86::reg16>(record + 0xc) = x86::reg16(ahead);
        getMemory<x86::reg16>(record + 0xe) = x86::reg16(random(id + 4));
        word(record + 0x10) = random(id + 5) % 4 == 0 ? random(id + 5) % 3 : 1;
    }
    result = r % 5 == 0 ? 0 : 1;
    break;
}
case kPursuitPlace:
{
    const x86::reg32 record = words[1];
    if (record < kMemory - 0xc)
    {
        word(record) = random(id + 1) % 0x40;
        word(record + 4) = random(id + 2) % 10 == 0 ? random(id + 3) : random(id + 3) % 200;
        word(record + 8) = random(id + 4) % 4;
    }
    result = s.address == 0x4c8550 ? (r % 2 == 0 ? 0xffffffff : r % 8) : (r % 3 == 0 ? 0 : 1);
    break;
}
case kPursuitSmall:
    result = r % 12;
    break;
case kPursuitLine:
    // The line's record at eax, as sayPhrase, sayCarPhrase and radioNoise
    // write it: its words 0 to 6, and in the pursuit's own cases (their
    // world's mark, kPMark, the case's seed) its word 8 too, the car -- not
    // its eighth, a local the code never writes, whose stack garbage is the
    // runtime's __CHP's frame on the generated side, none on the native's
    // (watcom::CHP touches no stack).  The sound's callers (snd::) leave
    // their records' word 8 as it was: not logged for them.
    for (x86::reg32 k = 0; k < 9; ++k)
        if (k < 7 || (k == 8 && word(0xcb8ffc) == x86::reg32(g_seed)))
            calls.back().words.push_back(words[0] + 4 * k < kMemory - 4 ? word(words[0] + 4 * k) : 0xbad0bad0);
    break;
case kPursuitDebug:
{
    // The debug text's arguments past its five (the row's words): one for each % of its format, the
    // fifth word -- "V%d" has the chase's count, the blank none.  A sixth word logged for the blank
    // would be the caller's frame (its saved ebp), which the natives that keep their locals in C++ do
    // not write.
    const x86::reg32 format = word(esp + 0x14);
    x86::reg32 more = 0;
    for (x86::reg32 at = format; at < format + 0x40 && at < kMemory && getMemory<char>(at) != 0 && more < 4; ++at)
        if (getMemory<char>(at) == '%')
            calls.back().words.push_back(word(esp + 0x18 + 4 * more++));
    break;
}
#elif defined(PURSUIT_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The ramming rival's tick (airace::rivalTick, native too), called by address:
// a stand-in as it was while generated, held against its code on its own.
{0x404840, "the ramming rival's tick", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// After the others' rows (the first row of an address is the one used).
// The pursuit's other files' (0x401220 to 0x40fcf0, generated or another
// part's): their answers made up so that the code's paths run -- a cop near or
// not, a car within reach, the nodes ahead (within the case's 64), a lane
// (never so wild that scoreLane's walk into its node's lanes takes long), the
// speeds and the pace onto the x87's stack, a free node now and then (so that
// followCar's search ends).  No register logged for the track's pace, a car
// taken over and the places left: aiplan::trackTuningSpeed, answerOne and
// trackTuningFirst read none, and the pursuit's code calls them as C++.
{0x40ed40, "a node's speed", kWatcom, 1, 0, kEax, -1, 0, kPursuitSpeed},
{0x40ee80, "the track's pace", kWatcom, 0, 0, kEax, -1, 0, kPursuitPace},
{0x40f050, "a cop near", kWatcom, 2, 0, kEax, -1, 0, kPursuitFlag},
{0x404610, "the chase's kind", kWatcom, 1, 0, kEax, -1, 0, kPursuitKind},
{0x40e160, "a node ahead", kWatcom, 2, 0, kEax, -1, 0, kPursuitNode},
{0x40e0c0, "a node's lane", kWatcom, 1, 0, kEax, -1, 0, kPursuitLane},
{0x40f000, "a node within reach", kWatcom, 3, 0, kEax, -1, 0, kPursuitFlag},
{0x40f100, "a car taken over", kWatcom, 0, 0, kEax, -1, 0, kPursuitFlag},
{0x40df10, "the cop active", kWatcom, 1, 0, kEax, -1, 0, kPursuitYes},
{0x404b90, "the cop held", kWatcom, 1, 0, kEax, -1, 0, kPursuitFlag},
{0x401500, "the car out", kWatcom, 1, 0, kEax, -1, 0, kPursuitFlag},
{0x405c80, "a node taken", kWatcom, 1, 0, kEax, -1, 0, kPursuitFlag},
{0x40ef10, "the places left", kWatcom, 0, 0, kEax, -1, 0, kPursuitSmall},
// The game's the radio asks: the radar's word, the places named on the
// track (their records written), the speech's line queued (its 13 words
// logged as kPursuitLine says -- snd::speechLine, which the sound's
// natives call by address too: they meet this stand-in as well, held against
// its own code on its own), the car ticketed.
{0x4c8040, "the radar", kWatcom, 2, 0, kEax, -1, 0, kPursuitRadar},
{0x4c8550, "the nearest place named", kWatcom, 2, 0, kEax, -1, 0, kPursuitPlace},
{0x4c8470, "the next place named", kWatcom, 2, 0, kEax, -1, 0, kPursuitPlace},
{0x4c85a0, "a speech's line", kWatcom, 1, 0, kEax, -1, 0, kPursuitLine},
{0x4c7fd0, "the car ticketed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// Other parts' natives, by address: the car reset (racestart::resetCar),
// the debug text (raceload's empty hook: its five words and its format's
// arguments logged, no register changed), a colour's nearest point (collide::nearestPoint), the speech bank
// loaded (snd::voiceBankLoad, its path's 16 words logged) -- each held
// against its own code on its own; here their worlds are not made.
{0x429100, "a car reset", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x42d520, "a debug text", kCdecl, 0, 5, 0, -1, 0, kPursuitDebug},
{0x431280, "a colour's nearest", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x413750, "a speech bank loaded", kWatcom, 3, 0, kEax, 0, 16, kNothing},
#elif defined(PURSUIT_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(PURSUIT_PROMISES)
// {"pursuit::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, and the x87's stack in use, its words.
{"pursuit::raceStartHook", true, true, 0, true, kNoHeap, true},
{"pursuit::raceEndHook", true, true, 0, true, kNoHeap, true},
{"pursuit::roadsideTarget", true, true, 0, true, kNoHeap, true},
{"pursuit::countLaneTime", true, true, 0, true, kNoHeap, true},
{"pursuit::pulledOverTarget", true, true, 0, true, kNoHeap, true},
{"pursuit::classSpeedFactor", true, true, 0, true, kNoHeap, true},
{"pursuit::unitFactor", true, true, 0, true, kNoHeap, true},
{"pursuit::radarFactor", true, true, 0, true, kNoHeap, true},
{"pursuit::unitFactorToo", true, true, 0, true, kNoHeap, true},
{"pursuit::leaderFactor", true, true, 0, true, kNoHeap, true},
{"pursuit::nodeTargetSpeed", true, true, 0, true, kNoHeap, true},
{"pursuit::markChased", true, true, 0, true, kNoHeap, true},
{"pursuit::targetSpeed", true, true, 0, true, kNoHeap, true},
{"pursuit::targetSpeedOnly", true, true, 0, true, kNoHeap, true},
{"pursuit::matchChasedSpeed", true, true, 0, true, kNoHeap, true},
{"pursuit::scoreCloseCops", true, true, 0, true, kNoHeap, true},
{"pursuit::growLaneTimes", true, true, 0, true, kNoHeap, true},
{"pursuit::scoreLane", true, true, 0, true, kNoHeap, true},
{"pursuit::scoreChases", true, true, 0, true, kNoHeap, true},
{"pursuit::radioChaseCount", true, true, 0, true, kNoHeap, true},
{"pursuit::scoreSpeeds", true, true, 0, true, kNoHeap, true},
{"pursuit::alwaysOne", true, true, 0, true, kNoHeap, true},
{"pursuit::takeOverCars", true, true, 0, true, kNoHeap, true},
{"pursuit::startReset", true, true, 0, true, kNoHeap, true},
{"pursuit::checkProgress", true, true, 0, true, kNoHeap, true},
{"pursuit::copStep", true, true, 0, true, kNoHeap, true},
{"pursuit::loadRacerBanks", true, true, 0, true, kNoHeap, true},
{"pursuit::countRadioFrames", true, true, 0, true, kNoHeap, true},
{"pursuit::radioNoise", true, true, 0, true, kNoHeap, true},
{"pursuit::sayPhrase", true, true, 0, true, kNoHeap, true},
{"pursuit::sayCarPhrase", true, true, 0, true, kNoHeap, true},
// namePhrase leaves edx (its argument) as strncpy does: the game's code
// for it 0, watcom::strncpy as it was (its promise: 3 arguments).
{"pursuit::namePhrase", true, true, 2, true, kNoHeap, true},
{"pursuit::namePhrases", true, true, 0, true, kNoHeap, true},
{"pursuit::radioFree", true, true, 0, true, kNoHeap, true},
{"pursuit::radioNothing", true, true, 0, true, kNoHeap, true},
{"pursuit::quietVoice", true, true, 0, true, kNoHeap, true},
{"pursuit::sayVoicePhrase", true, true, 0, true, kNoHeap, true},
{"pursuit::sayCallSign", true, true, 0, true, kNoHeap, true},
{"pursuit::sayPhraseOnly", true, true, 0, true, kNoHeap, true},
{"pursuit::sayUnit", true, true, 0, true, kNoHeap, true},
{"pursuit::sayDescription", true, true, 0, true, kNoHeap, true},
{"pursuit::radioNothingToo", true, true, 0, true, kNoHeap, true},
{"pursuit::sayLocation", true, true, 0, true, kNoHeap, true},
{"pursuit::saySpeed", true, true, 0, true, kNoHeap, true},
{"pursuit::radioSpotted", true, true, 0, true, kNoHeap, true},
{"pursuit::radioPursuit", true, true, 0, true, kNoHeap, true},
{"pursuit::radioLost", true, true, 0, true, kNoHeap, true},
{"pursuit::radioBackup", true, true, 0, true, kNoHeap, true},
{"pursuit::radioPatrol", true, true, 0, true, kNoHeap, true},
{"pursuit::radioRoadblock", true, true, 0, true, kNoHeap, true},
{"pursuit::radioNothingThree", true, true, 0, true, kNoHeap, true},
{"pursuit::radioTicket", true, true, 0, true, kNoHeap, true},
{"pursuit::radioArrest", true, true, 0, true, kNoHeap, true},
{"pursuit::radioStopped", true, true, 0, true, kNoHeap, true},
{"pursuit::radioAmbulance", true, true, 0, true, kNoHeap, true},
{"pursuit::radioSpikes", true, true, 0, true, kNoHeap, true},
{"pursuit::radioHelp", true, true, 0, true, kNoHeap, true},
{"pursuit::radioResume", true, true, 0, true, kNoHeap, true},
{"pursuit::radioAbort", true, true, 0, true, kNoHeap, true},
{"pursuit::resetOnTrack", true, true, 0, true, kNoHeap, true},
{"pursuit::copRestart", true, true, 0, true, kNoHeap, true},
{"pursuit::topSpeedFactor", true, true, 0, true, kNoHeap, true},
{"pursuit::followChased", true, true, 0, true, kNoHeap, true},
{"pursuit::raceEndHookToo", true, true, 0, true, kNoHeap, true},
{"pursuit::raceStartHookToo", true, true, 0, true, kNoHeap, true},
{"pursuit::followCar", true, true, 0, true, kNoHeap, true},
{"pursuit::countdownDone", true, true, 0, true, kNoHeap, true},
{"pursuit::trafficTarget", true, true, 0, true, kNoHeap, true},
{"pursuit::trafficStep", true, true, 0, true, kNoHeap, true},
#elif defined(PURSUIT_ARGUMENTS)
/* Where the pursuit's cases keep their own: 8 cars (0xa00 each), their
 * choices (+0x14 the model, +0x30 the pursuit's class), handlings (+0x1cc
 * the top speed) and pursuit records, the track's 64 nodes, buffers. */
constexpr x86::reg32 kPCars = 0xcb0000, kPChoices = 0xcb5000, kPHandlings = 0xcb5400, kPRecords = 0xcb6400;
constexpr x86::reg32 kPNodes = 0xcb7000, kPArgs = 0xcb8000, kPEnd = 0xcb9000, kPMark = 0xcb8ffc;
constexpr x86::reg32 kPCarCount = 8, kPNodeCount = 64;

x86::reg32 pCar(x86::reg32 i)
{
    return kPCars + 0xa00 * i;
}

x86::reg32 aPCar()
{
    return pCar(below(kPCarCount));
}

x86::reg32 aPCarOrNone()
{
    return chance(15) ? 0 : aPCar();
}

x86::reg32 pFloatOf(float f)
{
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float in [lo, hi), 0 or -0 now and then. */
x86::reg32 pBetween(float lo, float hi)
{
    if (chance(4))
        return chance(50) ? 0 : 0x80000000;
    return pFloatOf(lo + (hi - lo) * float(below(100000)) / 100000.0f);
}

/* The pursuit's world: the race's switches, the cars and their records, the
 * chases, the classes, the nodes, the radio's state. */
void makePursuitWorld(CheckApp& g)
{
    // The race's.
    g.word(0x6fd3b8) = chance(50) ? 3 : below(5);
    g.word(0x6fd3b0) = chance(50) ? 0 : chance(60) ? 1 : 2 + below(3);
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x7d3684) = chance(20) ? below(0x480) : below(0x1000);
    g.word(0x7d3680) = chance(80) ? 1 : 0;
    g.word(0x6fd50c) = chance(50) ? 0 : 1;
    g.getMemory<x86::reg16>(0x7a22da) = x86::reg16(chance(50) ? 0 : 1);
    g.word(0x6fd518) = below(9);
    g.word(0x6fd4b4) = 1 + below(4);
    g.word(0x6fd470) = below(2);
    g.word(0x6fd4f0) = below(2);
    g.word(0x6fd4ac) = chance(40) ? 8 : below(11);
    g.getMemory<x86::reg8>(0x6fd4fc) = x86::reg8(random32());
    g.word(0x6fd3a0) = chance(85) ? 0 : 1;
    g.word(0x6fd4cc) = below(2);
    g.word(0x6fd4c8) = below(2);
    g.word(0x6fd4f4) = below(kPCarCount);
    g.word(0x6fbb4c) = below(5);
    // The low memory a name of a small address reads (the cars' names and
    // folders stood in for): empty strings.
    for (x86::reg32 at = 0; at < 0x400; at += 4)
        g.word(at) = 0;
    for (x86::reg32 at = 0x7700; at < 0x7800; at += 4)
        g.word(at) = 0;
    // The track's nodes: their lanes either side (never none at all, so that
    // a lane walked into them is found), widths, heading.
    g.word(0x5dd940) = chance(90) ? kPNodeCount : 1 + below(kPNodeCount);
    g.word(0x5dd948) = kPNodes;
    for (x86::reg32 i = 0; i < kPNodeCount; ++i)
    {
        const x86::reg32 node = kPNodes + 0x24 * i;
        for (x86::reg32 at = node; at < node + 0x24; ++at)
            g.getMemory<x86::reg8>(at) = x86::reg8(random32());
        x86::reg8 lanes = x86::reg8(random32());
        if ((lanes & 0xf) + (lanes >> 4) == 0)
            lanes = 0x11;
        g.getMemory<x86::reg8>(node + 0x13) = lanes;
    }
    // The classes.
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g.word(0x552274 + 0x18 * i) = below(4);
        g.word(0x552278 + 0x18 * i) = below(2);
        g.word(0x55227c + 0x18 * i) = below(5);
    }
    // The cars.
    for (x86::reg32 i = 0; i < kPCarCount; ++i)
    {
        const x86::reg32 car = pCar(i);
        for (x86::reg32 at = car; at < car + 0xa00; at += 4)
            g.word(at) = chance(50) ? pBetween(-50.0f, 50.0f) : below(0x100);
        const x86::reg32 choice = kPChoices + 0x40 * i, handling = kPHandlings + 0x200 * i;
        const x86::reg32 record = kPRecords + 0x40 * i;
        for (x86::reg32 at = choice; at < choice + 0x40; at += 4)
            g.word(at) = below(0x40);
        g.word(choice + 0x14) = chance(80) ? below(0x32) : 0x32 + below(0x200);
        g.word(choice + 0x30) = below(8);
        for (x86::reg32 at = handling; at < handling + 0x200; at += 4)
            g.word(at) = pBetween(0.0f, 100.0f);
        g.word(handling + 0x1cc) = pBetween(20.0f, 90.0f);
        for (x86::reg32 at = record; at < record + 0x40; at += 4)
            g.word(at) = chance(50) ? 0 : below(4);
        g.word(car + 8) = chance(20) ? below(9) : below(400);  // its progress along the track
        g.word(car + 0x1c) = below(kPNodeCount);
        g.word(car + 0x10c) = pBetween(0.0f, 2.0f);
        g.word(car + 0x114) = pBetween(0.5f, 3.0f);
        g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(80) ? 0 : 1);
        g.word(car + 0x184) = chance(80) ? 0 : 1;
        g.word(car + 0x1f0) = chance(90) ? i : below(8);
        g.word(car + 0x200) = random32();
        g.word(car + 0x204) = below(4);
        g.word(car + 0x208) = below(4);
        g.word(car + 0x220) = choice;
        g.word(car + 0x2bc) = below(10);
        g.word(car + 0x2c4) = chance(40) ? 3 + below(2) : below(6);
        g.word(car + 0x528) = handling;
        g.word(car + 0x5f4) = chance(70) ? 0 : 1;
        g.word(car + 0x620) = random32();
        g.word(car + 0x624) = aPCarOrNone();
        g.word(car + 0x628) = record;
        g.word(car + 0x638) = below(16);
        g.word(car + 0x650) = x86::reg32(x86::sreg32(below(5)) - 1);
        g.word(car + 0x654) = pBetween(-2.0f, 2.0f);
        g.word(car + 0x660) = below(2);
        g.word(car + 0x6d8) = aPCarOrNone();
        g.word(car + 0x6e0) = chance(90) ? below(8) : random32();
        g.word(car + 0x6e4) = x86::reg32(x86::sreg32(below(5)) - 1);
        g.word(car + 0x6ec) = below(0x1000);
        g.word(car + 0x700) = pBetween(0.0f, 2.0f);
        g.word(car + 0x704) = below(0x1000);
        g.word(car + 0x708) = g.word(car + 8) + below(5) - 2;
        g.word(car + 0x710) = random32();
        // Its voice: 0 to 15 (a negative one -- sayPhrase's switch past the 8
        // -- reads a voice's record by whatever edi the caller left, a car's
        // address in sayDescription: only in sayPhrase's own cases).
        g.word(car + 0x714) = chance(90) ? below(8) : 8 + below(8);
        g.word(car + 0x7b0) = pBetween(-60.0f, 60.0f);
        g.word(car + 0x7b4) = pBetween(-5.0f, 5.0f);
        g.word(car + 0x7b8) = pBetween(-8.0f, 8.0f);
        g.word(car + 0x7bc) = pBetween(0.0f, 1.5f);
        g.word(car + 0x7c0) = pBetween(-100.0f, 100.0f);
        g.word(car + 0x7c4) = pBetween(0.0f, 100.0f);
        g.word(car + 0x7d4) = chance(70) ? 0 : 1;
    }
    // The cars' tables: the cops', the cars a cop may take over, the racers.
    g.word(0x5efd88) = below(kPCarCount + 1);
    g.word(0x553078) = below(kPCarCount + 1);
    g.word(0x5efd9c) = below(kPCarCount + 1);
    for (x86::reg32 i = 0; i < kPCarCount; ++i)
    {
        g.word(0x5efa88 + 4 * i) = aPCar();
        g.word(0x5efc08 + 4 * i) = aPCarOrNone();
        g.word(0x5efc88 + 4 * i) = aPCar();
        // Each racer's tickets.
        g.word(0x56ea74 + 0x14 * i) = below(7);
    }
    g.word(0x5efd8c) = below(9);
    g.word(0x56ebc0) = below(4);
    g.word(0x5efd94) = 1 + below(8);
    g.word(0x5efdd0) = pBetween(0.0f, 3.0f);
    // The chases.
    g.word(0x56ebb8) = aPCarOrNone();
    g.word(0x56ebb4) = below(4);
    g.word(0x56f224) = pBetween(0.0f, 60.0f);
    g.word(0x56f230) = x86::reg32(x86::sreg32(below(3)) - 1);
    g.word(0x56ef3c) = aPCarOrNone();
    g.word(0x56ef48) = below(4);
    g.word(0x56ec80) = below(7);
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g.word(0x56ef38 + 4 * i) = aPCarOrNone();
        g.word(0x56ef44 + 4 * i) = x86::reg32(x86::sreg32(below(8)) - 1);
        g.word(0x56ef50 + 4 * i) = aPCarOrNone();
        g.word(0x56ef5c + 4 * i) = x86::reg32(x86::sreg32(below(15)) - 12);
        g.word(0x56eef0 + 4 * i) = below(100);
        g.word(0x56eefc + 4 * i) = below(100);
        g.word(0x56ef08 + 4 * i) = below(100);
        g.word(0x56ef20 + 4 * i) = below(100);
        g.word(0x56ef14 + 4 * i) = below(100);
    }
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        g.getMemory<x86::reg16>(0x56ef68 + 2 * i) = x86::reg16(below(2));
        g.getMemory<x86::reg16>(0x56ef6e + 2 * i) = x86::reg16(below(2));
    }
    for (const x86::reg32 at : {0x56eeb4u, 0x56eeb8u, 0x56eebcu})
        g.word(at) = chance(40) ? 0 : below(500);
    // The radio's.
    g.word(0x5524c8) = chance(50) ? below(0x400) : below(0x800);
    g.word(0x5524ec) = chance(30) ? 0xffffffff : below(8);
    g.word(0x5524f0) = aPCar();
    g.word(0x5524f4) = below(2);
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g.word(0x5524cc + 4 * i) = below(0x100);
        g.word(0x56ee68 + 4 * i) = below(0x400);
        g.word(0x5524f8 + 4 * i) = chance(30) ? 0xffffffff : below(0x40);
        g.word(0x552520 + 4 * i) = chance(50) ? 0 : chance(50) ? 0x7f : 0x7e;
        g.word(0x56ee28 + 4 * i) = below(23);
    }
    for (x86::reg32 i = 0; i < 4; ++i)
        g.word(0x552548 + 4 * i) = pBetween(-2.0f, 2.0f);
    for (const x86::reg32 at : {0x554f10u, 0x554f14u, 0x554f18u})
        g.word(at) = random32();
    g.word(0x7dd378) = below(13);
    // The racers' colours (0x6c bytes a car) and the speech's folder.
    for (x86::reg32 at = 0x6fd54c; at < 0x6fd54c + 0x6c * 8; at += 4)
        g.word(at) = random32();
    const char* folder = chance(50) ? "C:\\NFS3\\speech" : "";
    for (x86::reg32 k = 0; k < 0x20; ++k)
        g.getMemory<char>(0x7a28b4 + k) = k < std::strlen(folder) ? folder[k] : 0;
    // The buffers, and the mark of a pursuit's case (kPursuitLine's).
    for (x86::reg32 at = kPArgs; at < kPEnd; at += 4)
        g.word(at) = pBetween(-100.0f, 100.0f);
    g.word(kPMark) = x86::reg32(g_seed);
}

/* Whether the car's name's stand-in (0x439250, kNothing) keeps eax at the
 * case's `index`-th call -- the model, a small address, an empty name -- as
 * game_checks.cpp's standIn works it out; else it hands back a made-up
 * word, a wild pointer the name is read from. */
bool pNameKept(x86::reg32 index)
{
    const x86::reg32 more = x86::reg32(mix(g_seed ^ (std::uint64_t(index + 1) << 32) ^ (0x439250 ^ 0xabcd)));
    return more % 4 == 0;
}

/* Whether the police car's stand-in (0x4396b0, kMainThread) says yes at the
 * `index`-th call. */
bool pIsPolice(x86::reg32 index)
{
    return (x86::reg32(mix(g_seed ^ (std::uint64_t(index + 1) << 32) ^ 0x4396b0)) & 1) != 0;
}

bool makePursuitArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makePursuitWorld(g);
    cpu.eax = aPCar();
    cpu.edx = chance(50) ? aPCar() : below(3);
    // sayPhrase's voice past the 8 leaves edi as it was and reads a voice's
    // record by it: a small one.
    cpu.edi = below(16);
    if (name == "loadRacerBanks" || name == "namePhrases")
    {
        // The cars' names read where the stand-in hands them back: the case
        // stopped at the first call of the car's name whose stand-in would
        // make one up (pNameKept).  Up to two cars before the last of models
        // past 49 (their name and colour only: two calls each), the last of
        // a model mostly under 50, which -- not a police car -- has its
        // speech bank loaded, with a second name (strstr's "Pace") only past
        // the split screen's and the player's colours (call 2n + 4).
        g.word(0x6fd3b8) = chance(85) ? 3 : below(3);
        const x86::reg32 n = chance(10) ? 0 : chance(70) ? 1 : 2 + below(2);
        g.word(0x5efd9c) = n;
        for (x86::reg32 i = 0; i < n; ++i)
        {
            const x86::reg32 car = pCar(i);
            g.word(0x5efc88 + 4 * i) = car;
            g.word(g.word(car + 0x220) + 0x14) = i + 1 < n ? 0x32 + below(0x100) : chance(85) ? below(0x32) : 0x32 + below(8);
        }
        g_mostCalls = -1;
        for (x86::reg32 i = 0; i < n && g_mostCalls < 0; ++i)
            if (!pNameKept(2 * i))
                g_mostCalls = long(2 * i);
        if (g_mostCalls < 0 && n != 0)
        {
            const x86::reg32 last = pCar(n - 1);
            const x86::reg32 model = g.word(g.word(last + 0x220) + 0x14);
            const x86::reg32 index = g.word(last + 0x1f0);
            const bool split = (index == 0 || index == 1) && g.word(0x6fd3b0) == 1;
            const bool player = !split && g.word(0x6fd4f4) == index;
            if (model <= 0x31 && !pIsPolice(2 * n) && !split && !player && !pNameKept(2 * n + 4))
                g_mostCalls = long(2 * n + 4);
        }
    }
    else if (name == "radioNoise")
    {
        cpu.edx = below(2);
        cpu.ebx = aPCar();
    }
    else if (name == "sayPhrase" || name == "sayCarPhrase")
    {
        cpu.eax = random32();
        cpu.edx = chance(30) ? 0 : 1;
        cpu.ebx = chance(10) ? 0x21 : below(0x80);
        cpu.ecx = aPCar();
        stack[0] = below(23);
        if (chance(10))
            g.word(cpu.ecx + 0x714) = x86::reg32(-1 - x86::sreg32(below(8)));
    }
    else if (name == "namePhrase")
    {
        cpu.eax = below(0x33);
        cpu.edx = chance(70) ? 0x536560 + 8 * below(0x21) : kPArgs + below(0x100);
        cpu.ebx = random32();
    }
    else if (name == "radioFree")
    {
        cpu.eax = below(9);
        cpu.edx = chance(30) ? g.word(0x5524f0) : aPCar();
    }
    else if (name == "quietVoice")
    {
        cpu.eax = below(8);
        cpu.edx = below(3);
    }
    else if (name == "sayVoicePhrase")
        cpu.edx = chance(50) ? 0 : chance(70) ? 1 : 0x20;
    else if (name == "sayPhraseOnly")
        cpu.eax = below(0x80);
    else if (name == "sayDescription")
    {
        cpu.eax = below(2);
        cpu.ebx = chance(90) ? 8 + below(28) : random32();
    }
    else if (name == "sayLocation")
        cpu.eax = chance(50) ? 0 : 1;
    else if (name == "saySpeed")
    {
        cpu.eax = below(2);
        cpu.ebx = aPCar();
    }
    else if (name == "radioStopped")
        cpu.edx = below(3);
    else if (name == "resetOnTrack")
    {
        // The radar's record: +8 a speed, +0xa its word of how far (the high
        // half, past 200 now and then), +0x10 its kind.
        cpu.edx = kPArgs + 0x40 * below(8);
        g.word(cpu.edx + 8) = pBetween(0.0f, 60.0f);
        g.getMemory<x86::reg16>(cpu.edx + 0xa) = x86::reg16(random32());
        g.getMemory<x86::reg16>(cpu.edx + 0xc) =
            x86::reg16(chance(70) ? x86::sreg32(below(121)) - 60 : x86::sreg32(below(1201)) - 600);
        g.word(cpu.edx + 0x10) = below(3);
    }
    return true;
}
#endif
