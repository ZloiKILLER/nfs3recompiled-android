// game_checks.cpp's parts for the race's cars loaded, kept up and in collision, its messages and its controls'
// thread (native_raceload.cpp), included in each table with the macro naming the part.
#if defined(RACELOAD_WINDOWS)
// {from, to}, -- what the functions write, compared
// The cars' records and performance records, their lists, the collisions'
// frames and the controls' ticks (0x5ef000 on is the controls' already); the
// car's .viv, the time step and the message lines' head; the track's nodes;
// the network's mark of a message; the cases' own (cars' records' targets,
// the track's nodes, texts, the message lines, vectors).
{0x5e1000, 0x5ef000}, {0x553000, 0x554000}, {0x5dd000, 0x5de000}, {0x7a2000, 0x7a3000}, {0xd00000, 0xd10000},
#elif defined(RACELOAD_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kRaceloadVivEntry,  // 0x4ea290: a file's place in a .viv to [ecx], its size to [[esp+4]]: small, or none
kRaceloadFits,      // 0x441900: whether a message's text fits its line: 1 mostly, 0 now and then
#elif defined(RACELOAD_EFFECT_CASES)
// case kSomething: ... break;
case kRaceloadVivEntry:
    // (the .viv, the name, 0, &the place; &the size)
    if (words[3] < kMemory - 4)
        word(words[3]) = r % 5 == 0 ? 0 : (r >> 4) % 0x4000;
    if (words[4] < kMemory - 4)
        word(words[4]) = r % 7 == 0 ? 0 : (r >> 8) % 0x400;
    break;
case kRaceloadFits:
    result = r % 6 == 0 ? 0 : 1;
    break;
#elif defined(RACELOAD_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// A file found in the car's .viv (ret 4), its name's first bytes logged.
{0x4ea290, "a file found in a .viv", kStdcall, 4, 1, kEax, 1, 4, kRaceloadVivEntry},
// What leaves a float on the x87's stack: a car's speed, a vector's length,
// a vector normalized.
{0x4a2bc0, "a car's speed", kWatcom, 1, 0, kEax, -1, 0, kFloatBack},
{0x4972f0, "a vector's length", kWatcom, 2, 0, kEax, 0, 3, kFloatBack},
{0x4e06a0, "a vector normalized", kWatcom, 1, 0, kEax, 0, 3, kFloatBack},
// Whether a message's text fits its line, its first bytes logged.
{0x441900, "a message's text fits", kWatcom, 1, 0, kEax, 0, 4, kRaceloadFits},
// Natives of other files the cars' loading and the controls' thread call by
// address (graphics::drawLoadingProgress, stdcall's 4 bytes;
// controls::readRaceControls; racestart::placeOnGrid, the car eax): each held
// against its code on its own.
{0x495230, "the loading bar drawn", kStdcall, 0, 1, 0, -1, 0, kNothing},
{0x48f550, "the race's controls read", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x42bcb0, "a car placed on the grid", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// loadFile (loadfileadr), the name eax and the flags edx and no more: the
// callers' ebx and ecx are not its (it pushes them), and the cars' .viv's
// typed loading leaves its own there.  sound_lib.inc's row (4 registers)
// comes after this one, so this is the one every check has.
{0x4e0ec0, "a file loaded", kWatcom, 2, 0, kEax, -1, 0, kLoad},
#elif defined(RACELOAD_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(RACELOAD_PROMISES)
// {"raceload::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, the x87's stack and words too where the code
// uses it -- but ebx and edx after a message line is made: the game's
// strncpy (0x4e0e30) and native watcom::strncpy leave its argument
// registers differently, and insertMessageLine leaves them as it does.
{"raceload::loadCarPerformance", true, true, 0},
{"raceload::loadRaceCar", true, true, 0},
{"raceload::setCarHandlers", true, true, 0},
{"raceload::freeCarShapes", true, true, 0},
{"raceload::restartRaceCars", true, true, 0},
{"raceload::clearCarSlots", true, true, 0},
{"raceload::clearRaceCounts", true, true, 0},
{"raceload::clearCarList", true, true, 0},
{"raceload::loadRaceCars", true, true, 0, true, kNoHeap, true},
{"raceload::setTimeStep", true, true, 0},
{"raceload::addCarTasks", true, true, 0},
{"raceload::openCarViv", true, true, 0},
{"raceload::closeCarViv", true, true, 0},
{"raceload::removeCarTasks", true, true, 0},
{"raceload::countLaps", true, true, 0},
{"raceload::carAxesSpeed", true, true, 0, true, kNoHeap, true},
{"raceload::carForwardSpeed", true, true, 0, true, kNoHeap, true},
{"raceload::carRaceDistance", true, true, 0},
{"raceload::updateRaceCars", true, true, 0, true, kNoHeap, true},
{"raceload::frameFraction", true, true, 0, true, kNoHeap, true},
{"raceload::placeHeadlights", true, true, 0, true, kNoHeap, true},
{"raceload::placeCarShadow", true, true, 0, true, kNoHeap, true},
{"raceload::sortRaceCars", true, true, 0},
{"raceload::emptyHook", true, true, 0},
{"raceload::emptyHook2", true, true, 0},
{"raceload::showMessage", true, true, 0},
{"raceload::sendMessage", true, true, 0},
{"raceload::openMessages", true, true, 0},
{"raceload::closeMessages", true, true, 0},
{"raceload::clearMessages", true, true, 0},
{"raceload::sendTypedMessage", true, true, 0},
{"raceload::newMessageLine", true, true, 0},
{"raceload::freeMessageLine", true, true, 0},
{"raceload::insertMessageLine", true, true, 3},
{"raceload::freeMessageLines", true, true, 0},
{"raceload::clearMessageLines", true, true, 0},
{"raceload::showMessageLines", true, true, 0},
{"raceload::appendMessageLine", true, true, 0},
{"raceload::closeMessageLines", true, true, 0},
{"raceload::openMessageLines", true, true, 0},
{"raceload::raceControlsThread", true, true, 0},
{"raceload::wakeRaceControls", true, true, 0},
{"raceload::raceControlsTick", true, true, 0},
{"raceload::startRaceControlsTick", true, true, 0},
{"raceload::stopRaceControlsTick", true, true, 0},
{"raceload::cornerInBox", true, true, 0, true, kNoHeap, true},
{"raceload::cornerInBoxBack", true, true, 0, true, kNoHeap, true},
{"raceload::cornerInBox2", true, true, 0, true, kNoHeap, true},
{"raceload::cornerInBoxBack2", true, true, 0, true, kNoHeap, true},
{"raceload::clampSpin", true, true, 0, true, kNoHeap, true},
{"raceload::applyImpulse", true, true, 0, true, kNoHeap, true},
{"raceload::bounceOffWall", true, true, 0, true, kNoHeap, true},
{"raceload::hitWall", true, true, 0, true, kNoHeap, true},
{"raceload::boxHitsWall", true, true, 0, true, kNoHeap, true},
{"raceload::carsCollide", true, true, 0, true, kNoHeap, true},
#elif defined(RACELOAD_ARGUMENTS)
/* Where the cars' cases keep their own: the track's nodes' records (0x24
 * bytes each), texts, the message lines and their head, vectors. */
constexpr x86::reg32 kRaceloadNodes = 0xd00000, kRaceloadTexts = 0xd08000, kRaceloadLines = 0xd0a000;
constexpr x86::reg32 kRaceloadHead = 0xd0e000, kRaceloadVectors = 0xd0f000;
constexpr x86::reg32 kRaceloadCars = 0x5e1120, kRaceloadCarBytes = 0x9ac, kRaceloadChoices = 0x6fd52c;

/* A float of a car's: -50 to 50 mostly, small, 0, now and then large. */
x86::reg32 aRaceloadFloat()
{
    float f;
    switch (below(10))
    {
    case 0: f = 0.0f; break;
    case 1: f = float(x86::sreg32(below(2001)) - 1000) / 1000.0f; break;
    case 2: f = float(x86::sreg32(below(200001)) - 100000) * 10.0f; break;
    default: f = float(x86::sreg32(below(10001)) - 5000) / 100.0f; break;
    }
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float of 0.1 to `most`. */
x86::reg32 aRaceloadSize(float most)
{
    const float f = 0.1f + most * float(below(1000)) / 1000.0f;
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A text: printable, 0 to `most` long, into `at`. */
x86::reg32 aRaceloadText(CheckApp& g, x86::reg32 at, x86::reg32 most)
{
    const x86::reg32 length = chance(10) ? 0 : below(most + 1);
    for (x86::reg32 i = 0; i < length; ++i)
        g.getMemory<x86::reg8>(at + i) = x86::reg8(0x20 + below(0x5f));
    g.getMemory<x86::reg8>(at + length) = 0;
    return at;
}

/* The cars' world: 16 cars' records, floats with their counts, flags,
 * pointers and nodes among them; their choices; the lists, in order and
 * shuffled; the race's flags and counts; the track's nodes; the collisions'
 * frames; the message lines (a chain of a few from the head, or none); the
 * network's players; the controls' ticks. */
void makeRaceloadWorld(CheckApp& g)
{
    const x86::reg32 count = chance(5) ? 0 : 1 + below(chance(80) ? 8 : 16);
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 car = kRaceloadCars + kRaceloadCarBytes * i;
        for (x86::reg32 at = 0; at < kRaceloadCarBytes; at += 4)
            g.word(car + at) = aRaceloadFloat();
        // Its axes within -1 and 1, its sizes and masses positive.
        for (x86::reg32 at = 0xc0; at < 0xe4; at += 4)
            g.word(car + at) = aRaceloadFloat() & 0x80000000 | aRaceloadSize(0.9f);
        for (const x86::reg32 at : {0xb0u, 0xb4u, 0x114u, 0x118u, 0x11cu, 0x120u})
            g.word(car + at) = aRaceloadSize(at >= 0x114 && at < 0x120 ? 4.0f : 3.0f);
        // Its node and the last: about the line (0 and 500, where laps are
        // counted) mostly, anywhere, before the start now and then.
        const auto aNode = [] {
            return chance(35) ? below(60) : chance(50) ? 470 + below(60) : chance(85) ? below(700)
                                                                          : x86::reg32(-1 - x86::sreg32(below(50)));
        };
        g.word(car + 0x1c) = aNode();
        g.word(car + 4) = chance(20) ? g.word(car + 0x1c) : chance(20) ? 0 : aNode();
        g.word(car + 8) = random32() % 100000;
        g.word(car + 0x204) = chance(80) ? below(5) : random32() % 10 - 5;
        g.word(car + 0x208) = chance(70) ? 0 : chance(80) ? below(3) : x86::reg32(-1 - x86::sreg32(below(3)));
        g.getMemory<x86::reg16>(car + 0x74) = x86::reg16(chance(80) ? 1 : 0);
        g.getMemory<x86::reg8>(car + 0x89) = x86::reg8(chance(85) ? 1 : 0);
        g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
        g.getMemory<x86::reg8>(car + 0x201) = x86::reg8(random32());
        g.word(car + 0x1f0) = below(8);
        g.word(car + 0x220) = kRaceloadChoices + 0x6c * below(16);
        g.word(car + 0x528) = 0x5eabe0 + 0x4ac * i;
        for (x86::reg32 at = 0x5f8; at <= 0x61c; at += 4)
            g.word(car + at) = 0x400000 + below(0x100000);
        g.word(car + 0x660) = chance(30) ? 0 : chance(60) ? 1 : 2;
        g.word(car + 0x734) = chance(30) ? 0 : 1;
        g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(50) ? 0 : below(4));
        g.word(car + 0x18c) = random32();
        // Its spin: within 2 mostly, now and then the most it is held to (1.8).
        for (x86::reg32 at = 0xe4; at < 0xf0; at += 4)
            g.word(car + at) = chance(15) ? 0x3fe66666 ^ (random32() & 0x80000000)
                               : chance(70) ? aRaceloadSize(2.0f) ^ (random32() & 0x80000000) : g.word(car + at);
    }
    // The choices: a car's index -- one of the 50 mostly, a pursuit's, traffic.
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(kRaceloadChoices + 0x6c * i + 0x14) = chance(60) ? below(50) : chance(50) ? 500 + below(100)
                                                     : chance(50) ? 50 + below(450) : random32();
    // The lists: the cars in order, by place and by distance shuffled.
    x86::reg32 order[16];
    for (x86::reg32 i = 0; i < 16; ++i)
        order[i] = i;
    for (x86::reg32 list : {0x5efac8u, 0x5efcc8u, 0x5efc88u})
    {
        if (list != 0x5efac8)
            for (x86::reg32 i = 15; i > 0; --i)
                std::swap(order[i], order[below(i + 1)]);
        for (x86::reg32 i = 0; i < 16; ++i)
            g.word(list + 4 * i) = kRaceloadCars + kRaceloadCarBytes * order[i];
    }
    g.word(0x5efd9c) = count;
    g.word(0x6fd510) = chance(85) ? count : below(10);
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        g.word(0x5efd48 + 4 * i) = chance(50) ? 0 : random32();
        g.word(0x5ef6a0 + 4 * i) = chance(50) ? 0 : random32();
    }
    // The race.
    g.getMemory<x86::reg8>(0x6fd4fc) = x86::reg8(random32());
    g.word(0x6fd500) = below(30);
    g.word(0x6fd50c) = chance(50) ? 0 : 1;
    g.word(0x6fd3b0) = below(4);
    g.word(0x6fd4f4) = below(4);
    g.word(0x6fd514) = below(5);
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x553070) = chance(20) ? 0 : 0x7700 + below(0x10) * 0x10;
    for (const x86::reg32 at : {0x56484eu, 0x56483au, 0x564850u})
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(50) ? 0 : random32());
    g.word(0x7d3684) = chance(80) ? below(100000) : random32();
    g.word(0x6fbc3c) = below(3);
    g.word(0x7a3a60) = chance(50) ? 0 : 1;
    // The track: its nodes, their records' bytes.
    g.word(0x5dd940) = 100 + below(3000);
    g.word(0x5dd948) = kRaceloadNodes + 0x24 * 60;
    for (x86::reg32 at = kRaceloadNodes; at < kRaceloadNodes + 0x8000; at += 4)
        g.word(at) = random32();
    // The collisions' frames.
    for (x86::reg32 at = 0x5efe60; at < 0x5eff08; at += 4)
        g.word(at) = aRaceloadFloat();
    for (const x86::reg32 axes : {0x5efee0u, 0x5efe90u})
        for (x86::reg32 at = 0; at < 0x24; at += 4)
            g.word(axes + at) = aRaceloadFloat() & 0x80000000 | aRaceloadSize(0.9f);
    for (x86::reg32 at = 0x5efe60; at < 0x5efe78; at += 4)
        g.word(at) = chance(70) ? aRaceloadSize(2.0f) ^ (random32() & 0x80000000) : aRaceloadFloat();
    // The message lines: their head (its lock, the first line, the last) and
    // a chain of lines from the first.
    g.word(0x5537a0) = chance(15) ? 0 : kRaceloadHead;
    g.word(kRaceloadHead) = 0x630000 + below(0x100) * 0x10;
    const x86::reg32 lines = chance(15) ? 0 : 1 + below(6);
    g.word(kRaceloadHead + 4) = lines == 0 ? 0 : kRaceloadLines;
    g.word(kRaceloadHead + 8) = lines == 0 ? (chance(50) ? 0 : kRaceloadLines) : kRaceloadLines + 0x80 * (lines - 1);
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 line = kRaceloadLines + 0x80 * i;
        g.word(line) = random32();
        aRaceloadText(g, line + 4, 0x63);
        g.word(line + 0x68) = i + 1 < lines ? line + 0x80 : 0;
        g.word(line + 0x6c) = i == 0 ? 0 : line - 0x80;
    }
    g.word(0x55379c) = random32();
    // The network.
    g.getMemory<x86::reg16>(0x7a22da) = x86::reg16(chance(30) ? 0 : 1);
    g.word(0x7a22e4) = chance(80) ? 1 : below(3);
    g.word(0x7a2274) = (chance(80) ? below(9) : random32() % 20) << 16;
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(0x7a21f0 + 4 * i) = below(8);
    // The controls' thread.
    for (const x86::reg32 at : {0x5efdf0u, 0x5efdf4u, 0x5efdf8u, 0x5efdfcu, 0x5efe00u, 0x79c4acu})
        g.word(at) = chance(30) ? random32() : below(8);
    g.word(0x5537a4) = chance(50) ? 0 : 1;
    // The cases' vectors.
    for (x86::reg32 at = kRaceloadVectors; at < kRaceloadVectors + 0x100; at += 4)
        g.word(at) = aRaceloadFloat();
}

bool makeRaceloadArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeRaceloadWorld(g);
    // The x87's word as the game sets it (fldcw, so the arithmetic's rounding
    // goes with it): the race's single precision mostly, the menus'.
    cpu.fpu.setControl(x86::reg16(chance(60) ? 0x007f : chance(50) ? 0x027f : 0x037f));
    // The frames below: floats, as the vectors' stand-ins leave them unwritten
    // -- no NaNs of any bits, of which the doubles' arithmetic keeps either of
    // two as the host's compiler orders the operands.
    for (x86::reg32 at = cpu.esp - 0x800; at < cpu.esp; at += 4)
        g.word(at) = aRaceloadFloat();
    const auto aCar = [] { return kRaceloadCars + kRaceloadCarBytes * below(16); };
    const auto aVector = [] { return kRaceloadVectors + 4 * below(0x30); };
    const auto aSign = [] { return x86::reg32(x86::sreg32(below(3)) - 1); };
    if (name == "loadCarPerformance")
    {
        cpu.eax = aCar();
        cpu.edx = aRaceloadText(g, kRaceloadTexts, 0x20);
    }
    else if (name == "loadRaceCar")
    {
        cpu.eax = aCar();
        cpu.ebx = below(g.word(0x6fd510) + 2);
        cpu.edx = 0x5eabe0 + 0x4ac * below(16);
    }
    else if (name == "setCarHandlers" || name == "freeCarShapes" || name == "countLaps" || name == "carAxesSpeed"
             || name == "carForwardSpeed" || name == "carRaceDistance" || name == "placeHeadlights"
             || name == "placeCarShadow" || name == "clampSpin")
        cpu.eax = aCar();
    else if (name == "openCarViv")
        cpu.eax = kRaceloadChoices + 0x6c * below(16);
    else if (name == "showMessage")
        cpu.eax = aRaceloadText(g, kRaceloadTexts, chance(70) ? 0x60 : 0x200);
    else if (name == "sendMessage")
    {
        cpu.eax = below(8);
        cpu.edx = aRaceloadText(g, kRaceloadTexts, 0x60);
        cpu.ebx = below(0x60);
    }
    else if (name == "sendTypedMessage")
        cpu.edx = aRaceloadText(g, kRaceloadTexts, 0x60);
    else if (name == "freeMessageLine" || name == "freeMessageLines")
        cpu.eax = chance(15) ? 0 : kRaceloadLines + 0x80 * below(8);
    else if (name == "insertMessageLine")
    {
        cpu.eax = random32();
        cpu.edx = chance(10) ? 0 : aRaceloadText(g, kRaceloadTexts, 0x80);
        cpu.ebx = kRaceloadLines + 0x80 * below(8);
    }
    else if (name == "appendMessageLine")
        cpu.eax = chance(10) ? 0 : aRaceloadText(g, kRaceloadTexts, 0x80);
    else if (name == "raceControlsThread")
        // For ever: as many calls as the stand-ins let it.
        g_mostCalls = 41;
    else if (name == "cornerInBox" || name == "cornerInBoxBack" || name == "cornerInBox2" || name == "cornerInBoxBack2")
    {
        cpu.eax = aCar();
        cpu.edx = aCar();
        cpu.ecx = aSign();
        cpu.ebx = aSign();
        stack[0] = aSign();
    }
    else if (name == "applyImpulse")
    {
        cpu.eax = aCar();
        for (x86::reg32 i = 0; i < 6; ++i)
            stack[i] = aRaceloadFloat();
        stack[6] = aRaceloadFloat();
    }
    else if (name == "bounceOffWall")
    {
        cpu.eax = aCar();
        cpu.edx = aVector();
        stack[0] = aRaceloadFloat();
    }
    else if (name == "hitWall")
    {
        cpu.eax = aCar();
        cpu.edx = aVector();
        cpu.ebx = aVector();
        cpu.ecx = aVector();
    }
    else if (name == "boxHitsWall")
    {
        cpu.eax = aCar();
        cpu.edx = aVector();
        cpu.ebx = aVector();
        stack[0] = chance(70) ? aRaceloadSize(5.0f) : aRaceloadFloat();
    }
    else if (name == "carsCollide")
    {
        cpu.eax = aCar();
        cpu.edx = aCar();
        cpu.ebx = aVector();
    }
    return true;
}
#endif
