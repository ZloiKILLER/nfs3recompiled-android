// game_checks.cpp's parts for the race's queue of inputs and the edges of the
// keys and actions (native_controls_queue.cpp), included in each table with
// the macro naming the part.
#if defined(CONTROLS_WINDOWS)
// {from, to}, -- what the functions write, compared
// The queue's slots taken and its counts (0x7d4714 to 0x7d5741, the pipeline's
// page before them); the network's state; the code's page with the switches'
// tables (0x4c6514, 0x4c6a2c, 0x4c6acc), which the harness has not loaded:
// written by makeQueueWorld as nfs3.exe has them, read only.
{0x7d4000, 0x7d6000}, {0x7a2000, 0x7a3000}, {0x4c6000, 0x4c7000},
#elif defined(CONTROLS_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kQueueMachinePlayer,  // the player a machine is: 1 to 7 mostly, 0, or -1 (none), a word
#elif defined(CONTROLS_EFFECT_CASES)
// case kSomething: ... break;
case kQueueMachinePlayer:
    result = (r & 0xffff0000) | (r % 7 == 0 ? 0xffff : (r >> 8) % 8);
    break;
#elif defined(CONTROLS_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The network's: a packet sent to machine eax (0 all) -- edx it, ebx its
// length; its bytes logged, as many as the longest the queue sends -- and
// the player a machine is.
{0x4a60e0, "a packet sent", kWatcom, 3, 0, kEax | kEdx | kEbx, 1, 9, kNothing},
{0x4a6650, "a machine's player", kWatcom, 1, 0, kEax, -1, 0, kQueueMachinePlayer},
#elif defined(CONTROLS_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// EnterCriticalSection and LeaveCriticalSection: the game's own (kPointers).
#elif defined(CONTROLS_PROMISES)
// {"controls::name", watcom, result, arguments, fpu, heap, liveOnly},
{"controls::takeCarControls", true, true, 0},
{"controls::stepMenuChoice", true, true, 0},
{"controls::stepMenuChoiceOfThree", true, true, 0},
{"controls::actionPressed", true, true, 0},
{"controls::functionKeyPressed", true, true, 0},
{"controls::pressFunctionKey", true, true, 0},
{"controls::functionKeyRepeats", true, true, 0},
{"controls::keyPressed", true, true, 0},
{"controls::firstFunctionKeyHeld", true, true, 0},
{"controls::appendToPacket", true, true, 0},
{"controls::sendOwnInput", true, true, 0},
{"controls::receiveInput", true, true, 0},
{"controls::queueInput", true, true, 0},
{"controls::queueRemoteInput", true, true, 0},
{"controls::takeQueuedFrame", true, true, 0},
{"controls::takenControls", true, true, 0},
{"controls::repeatMissingInputs", true, true, 0},
{"controls::syncFrame", true, true, 0},
{"controls::framesTaken", true, true, 0},
{"controls::framesQueued", true, true, 0},
{"controls::queueLead", true, true, 0},
{"controls::noteLag", true, true, 0},
{"controls::setQueueDelays", true, true, 0},
{"controls::queueFull", true, true, 0},
#elif defined(CONTROLS_ARGUMENTS)
/* Where the queue's cases keep their own: the packet being built, a
 * packet received, four bytes queued, the players' four bytes copied out. */
constexpr x86::reg32 kQueuePacketBuffer = 0x61d000, kQueuePacket = 0x61e000, kQueueRecord = 0x61e100;
constexpr x86::reg32 kQueueRecords = 0x61e200;

/* A player of the queue's: of the 8 mostly, 0 often, now and then 8 to 10
 * or -1 -- whose slots and counts are another's or the queue's own, where
 * the code reaches with them. */
x86::reg32 aQueuePlayer()
{
    return chance(30) ? 0 : chance(90) ? below(8) : chance(50) ? 0xffffffff : 8 + below(3);
}

/* The race's queue's world: the switches' tables as nfs3.exe has them; the
 * mode, the replay, the players, the network; the frames queued about those
 * taken, the slots taken mostly as the queue keeps them, the bytes queued;
 * the delays, the lag; the rings of actions and function keys, the keys and
 * their edges, the buttons held before, the car's four bytes. */
void makeQueueWorld(CheckApp& g)
{
    static const x86::reg32 tables[] = {
        0x4c6514, 0x4c6689, 0x4c6672, 0x4c6608, 0x4c6608, 0x4c6608,
        0x4c6a2c, 0x4c6aaf, 0x4c6aaf, 0x4c6a79, 0x4c6aa6, 0x4c6a69,
        0x4c6acc, 0x4c6c26, 0x4c6c22, 0x4c6b0a, 0x4c6b0a, 0x4c6b0a};
    for (size_t i = 0; i < std::size(tables); i += 6)
        for (x86::reg32 j = 0; j < 5; ++j)
            g.word(tables[i] + 4 * j) = tables[i + 1 + j];
    // The race.
    g.word(0x6fd3b0) = chance(92) ? below(5) : random32();
    g.word(0x6fd3a0) = chance(80) ? below(2) : chance(80) ? below(4) : random32();
    g.word(0x6fd4f4) = chance(40) ? 0 : chance(95) ? below(8) : random32() % 11;
    // The players: 1 to 8 mostly, none, up to 10, or fewer than none -- not
    // so many that the loops over them go through all memory.
    g.word(0x6fd514) = chance(90) ? 1 + below(8) : chance(70) ? below(11) : 0x80000000 | random32();
    g.word(0x6fd50c) = chance(60) ? 0 : chance(80) ? 1 : random32();
    g.word(0x7a22e4) = chance(92) ? below(3) : random32();
    g.getMemory<x86::reg16>(0x7a22da) = x86::reg16(chance(50) ? 0 : chance(80) ? 1 : random32());
    g.getMemory<x86::reg16>(0x7a22dc) = x86::reg16(chance(85) ? 0 : chance(80) ? 1 : random32());
    g.getMemory<x86::reg16>(0x7a22de) = x86::reg16(chance(85) ? 0 : random32());
    g.word(0x560100) = chance(85) ? 1 : 0;
    g.word(0x560104) = chance(80) ? below(0x1000) : random32();
    g.word(0x560108) = chance(70) ? 0 : chance(80) ? 1 : random32();
    g.word(0x56010c) = chance(60) ? 0 : chance(80) ? 1 : random32();
    // The queue: frames taken, each player's queued a few on (or behind) --
    // not so far that queueRemoteInput's moving them back goes on for ever --
    // their slots taken between, mostly; what is in them.
    const x86::reg32 taken = chance(90) ? below(0x1000) : below(0x70000000);
    g.word(0x7d5714) = taken;
    for (x86::reg32 p = 0; p < 8; ++p)
    {
        const x86::reg32 queued = taken + (chance(85) ? below(0x28) - 4 : below(0x200) - 0x100);
        g.word(0x7d5718 + 4 * p) = queued;
        const bool exact = chance(70);
        for (x86::reg32 f = 0; f < 0x80; ++f)
        {
            const x86::reg32 slot = p * 0x200 + 4 * f;
            const bool held = x86::reg32(f - taken) % 0x80 < x86::reg32(queued - taken) % 0x80;
            g.word(0x7d4714 + slot) = exact ? (held ? (chance(98) ? 1 : 0) : (chance(98) ? 0 : 1))
                                            : chance(50) ? 0 : chance(90) ? 1 : random32();
            g.word(0x7d3714 + slot) = random32();
        }
    }
    // The flags' last player's past where 0x200 bytes reach: player -1's queue.
    for (x86::reg32 at = 0x7d3514; at < 0x7d3714; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = 0x7d4514; at < 0x7d4714; at += 4)
        g.word(at) = chance(50) ? 0 : 1;
    for (x86::reg32 at = 0x7d36f0; at < 0x7d3710; at += 4)
        g.word(at) = random32();
    g.word(0x7d3710) = chance(85) ? below(0x20) : random32();
    // The delays: past the 8 players' counts, they are players 8 and 9's to
    // the code handed those -- not so many frames that moving them back takes for ever.
    g.word(0x7d5738) = chance(85) ? below(0x30) : below(0x2000) - 0x1000;
    g.word(0x7d573c) = chance(85) ? below(0x30) : below(0x2000) - 0x1000;
    g.getMemory<x86::reg8>(0x7d5740) = x86::reg8(chance(70) ? below(0x10) : random32());
    // The lag as much as a remote player's delay now and then, where queueFull halves it or not.
    if (chance(15))
        g.getMemory<x86::reg8>(0x7d5740) = x86::reg8(g.word(0x7d5738));
    // The network's lock, its input's packet, the packet being built.
    g.word(0x7d36d8) = random32();
    for (x86::reg32 at = 0x7d36e4; at < 0x7d36ec; at += 4)
        g.word(at) = random32();
    g.word(0x7d36d0) = below(0x400);
    g.word(0x7d36d4) = kQueuePacketBuffer + below(0x100);
    for (x86::reg32 at = kQueuePacketBuffer; at < kQueuePacketBuffer + 0x600; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = kQueuePacket; at < kQueueRecords + 0x40; at += 4)
        g.word(at) = random32();
    // The screens' frame, the rings: a slot often as the one before.
    g.word(0x7d3688) = chance(80) ? below(0x10000) : random32();
    for (const x86::reg32 ring : {0x79c238u, 0x79c2b8u})
        for (x86::reg32 i = 0; i < 0x20; ++i)
            g.word(ring + 4 * i) = i > 0 && chance(40) ? g.word(ring + 4 * (i - 1))
                                   : chance(30) ? 0
                                   : chance(50) ? 1u << below(32) | 1u << below(32) : random32();
    g.word(0x79c4b0) = chance(85) ? below(0x20) : random32();
    // The keys held, their edges, the players' buttons before, the car's.
    for (x86::reg32 at = 0x564718; at < 0x564918; ++at)
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(70) ? 0 : chance(70) ? 0x80 : random32());
    for (x86::reg32 at = 0x79c0b8; at < 0x79c238; ++at)
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(60) ? 0 : chance(70) ? 0xff : random32());
    g.word(0x79c4a8) = random32();
}

bool makeQueueArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    (void)stack;
    static const char* const names[] = {
        "takeCarControls", "stepMenuChoice", "stepMenuChoiceOfThree", "actionPressed", "functionKeyPressed",
        "pressFunctionKey", "functionKeyRepeats", "keyPressed", "firstFunctionKeyHeld", "appendToPacket",
        "sendOwnInput", "receiveInput", "queueInput", "queueRemoteInput", "takeQueuedFrame", "takenControls",
        "repeatMissingInputs", "syncFrame", "framesTaken", "framesQueued", "queueLead", "noteLag",
        "setQueueDelays", "queueFull"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeQueueWorld(g);
    // A bit of the rings: of the 32 mostly, any now and then (the shift takes 5 of it).
    const auto aBit = [] { return chance(90) ? (random32() & 0xffffff00) | below(32) : random32(); };
    const auto aRecord = [] {
        static const x86::reg32 records[] = {0x79c338, 0x79c33c, kQueueRecord};
        return records[below(3)];
    };
    if (name == "takeCarControls")
        cpu.eax = chance(90) ? below(8) : below(0x10);
    else if (name == "stepMenuChoice")
    {
        cpu.eax = random32();
        if (chance(80))
            cpu.al = x86::reg8(below(10));
        cpu.edx = chance(80) ? below(10) : chance(50) ? 0x100 + below(0x100) : random32();
    }
    else if (name == "stepMenuChoiceOfThree")
    {
        cpu.eax = random32();
        if (chance(80))
            cpu.al = x86::reg8(below(4));
    }
    else if (name == "actionPressed" || name == "functionKeyPressed" || name == "pressFunctionKey")
    {
        cpu.eax = aBit();
        cpu.edx = chance(50) ? 0 : chance(50) ? 1 : random32();
    }
    else if (name == "functionKeyRepeats")
    {
        cpu.eax = aBit();
        cpu.edx = chance(85) ? below(0x20) : random32();
    }
    else if (name == "keyPressed")
    {
        cpu.eax = chance(85) ? below(0x80) : chance(50) ? 0x80 + below(0x100) : x86::reg32(-1 - x86::sreg32(below(0x100)));
        cpu.edx = chance(50) ? 0 : chance(50) ? 1 : random32();
    }
    else if (name == "appendToPacket")
    {
        cpu.edx = kQueuePacket + below(0x40);
        cpu.ebx = 1 + (chance(80) ? below(0x24) : below(0x80));
    }
    else if (name == "sendOwnInput")
    {
        cpu.eax = aRecord();
        // No more than the game's 8 players: past them repeatMissingInputs writes its records on
        // beyond the packet in sendOwnInput's frame, over the registers it saved there.
        if (x86::sreg32(g.word(0x6fd514)) > 8)
            g.word(0x6fd514) = 8;
    }
    else if (name == "receiveInput")
    {
        cpu.eax = random32();
        cpu.edx = kQueuePacket + below(0x40);
    }
    else if (name == "queueInput" || name == "queueRemoteInput" || name == "queueFull")
    {
        cpu.eax = aQueuePlayer();
        cpu.edx = aRecord();
    }
    else if (name == "takeQueuedFrame")
        cpu.eax = chance(50) ? g.word(0x7d5714) : random32();
    else if (name == "takenControls")
    {
        cpu.eax = aQueuePlayer();
        cpu.edx = chance(50) ? 0x79c4a8 : kQueueRecord + below(0x20);
    }
    else if (name == "repeatMissingInputs")
        cpu.eax = kQueueRecords + below(4);
    else if (name == "noteLag")
        cpu.eax = chance(70) ? below(0x20) : chance(50) ? x86::reg32(x86::sreg32(x86::sreg8(random32()))) : random32();
    return true;
}
#endif
