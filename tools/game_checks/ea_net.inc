// game_checks.cpp's parts for EA's network layer under the menus: packets sent, NET and CONN, the queues, LAYER
// (native_ea_net.cpp), included in each table with the macro naming the part.
#if defined(EA_WINDOWS)
// {from, to}, -- what the functions write, compared
// The NET object, the pending players and the links open (0x7a01e0 to
// 0x7a039c); the rest -- the network's state (0x7a2000), the sessions
// (0x55e000), EA's counts (0x566000), the queues (0x9f2000), the byte pairs'
// addresses (0xa0b000), the cases' own (0x61d000 to 0x620000) -- is in
// windows already.
{0x7a0000, 0x7a1000},
#elif defined(EA_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kEaNetSlot,  // playerMachine (0x4a6690): a machine, 0 to 7, or -1 now and then
kEaNetPriority,  // a queue's priority: 0 to 3, alike often
kEaNetFailing,  // a connection's send or a layer's packer that fails mostly: 0, now and then 1
kEaNetDone,  // a connection's, NET's, the link's or a layer's function: 0 now and then (not sent, not closed), else a little
#elif defined(EA_EFFECT_CASES)
// case kSomething: ... break;
case kEaNetSlot:
    result = r % 8 == 0 ? 0xffffffff : (r >> 8) % 8;
    break;
case kEaNetPriority:
    result = (r >> 8) % 4;
    break;
case kEaNetFailing:
    result = r % 8 == 0 ? 1 : 0;
    break;
case kEaNetDone:
    result = r % 3 == 0 ? 0 : 1 + (r >> 8) % 0x100;
    break;
#elif defined(EA_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The network's message (cmn, 0x512180): its four registers and the two
// words its ret takes off.
{0x512180, "a network message", kStdcall, 4, 2, kScratch | kEbx, -1, 0, kNothing},
// This file's that other natives call by address: the player's machine,
// and the unpackers (the source and the destination; the DXT1 image's and
// Huffman's ebx too, which they read).
{0x4a6690, "playerMachine", kWatcom, 1, 0, kEax, -1, 0, kEaNetSlot},
{0x50ed50, "huffUnpack", kWatcom, 3, 0, kEax, 0, 2, kNothing},
{0x510110, "runUnpack", kWatcom, 2, 0, kEax, 0, 2, kNothing},
{0x510760, "dxtUnpack", kWatcom, 3, 0, kEax, 0, 2, kNothing},
{0x510870, "pairUnpack", kWatcom, 2, 0, kEax, 0, 2, kNothing},
#elif defined(EA_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// The link's send (cdecl, 7 words; the address's first word logged).
{0x7a21b8, "the link's send", kCdecl, 0, 7, kScratch, 7, 1, kEaNetDone},
// The connections' and NET's functions, in the cases' table of them
// (0x61ee00): a connection's send (the block and its length; its first
// words logged) and close; NET's game shut, ended, a player killed, a
// player's check.
{0x61ee08, "a connection's send", kWatcom, 3, 0, kEax, 1, 2, kEaNetDone},
{0x61ee10, "a connection's close", kWatcom, 1, 0, kEax, -1, 0, kEaNetDone},
{0x61ee24, "NET's game shut", kWatcom, 2, 0, kEax, -1, 0, kEaNetDone},
{0x61ee2c, "NET's game ended", kWatcom, 2, 0, kEax, -1, 0, kEaNetDone},
{0x61ee34, "NET's player killed", kWatcom, 2, 0, kEax, -1, 0, kEaNetDone},
{0x61ee40, "NET's player's check", kWatcom, 2, 0, kEax, -1, 0, kEaNetDone},
// A queue's priority (its element's first word logged), a layer's packer
// (stdcall: four registers, the queue on the stack; the pieces' first
// logged).
{0x61ee80, "a queue's priority", kWatcom, 2, 0, kEax, 0, 1, kEaNetPriority},
{0x61ee84, "a layer's packer", kStdcall, 4, 1, kEax, 3, 2, kEaNetDone},
// The same failing mostly, for packets tried again until the connection is lost.
{0x61ee88, "a layer's packer that fails", kStdcall, 4, 1, kEax, 3, 2, kEaNetFailing},
{0x61eec8, "a connection's send that fails", kWatcom, 3, 0, kEax, 1, 2, kEaNetFailing},
#elif defined(EA_PROMISES)
// {"ea::name", watcom, result, arguments, fpu, heap, liveOnly},
{"ea::dropMachine", true, true, 0},
{"ea::announceMachineLeft", true, true, 0},
{"ea::endNetworkGame", true, true, 0},
{"ea::sessionIndex", true, true, 0},
{"ea::forgetPendingPlayer", true, true, 0},
{"ea::sendOverLink", true, true, 0},
{"ea::sendPacket", true, true, 0},
{"ea::resetNetwork", true, true, 0},
{"ea::closeNetwork", true, true, 0},
{"ea::machineNumber", true, true, 0},
{"ea::playerMachine", true, true, 0},
{"ea::netTrace", true, true, 0},
{"ea::netShutGame", true, true, 0},
{"ea::netEndGame", true, true, 0},
{"ea::netKillPlayer", true, true, 0},
{"ea::netCheckPlayer", true, true, 0},
{"ea::connRestore", true, true, 0},
{"ea::connSend", true, true, 0},
{"ea::connClose", true, true, 0},
{"ea::qnth", true, true, 0},
{"ea::qunlink", true, true, 0},
{"ea::qfree", true, true, 0},
{"ea::qremove", true, true, 0},
{"ea::qlength", true, true, 0},
{"ea::qposition", true, true, 0},
{"ea::qfindindex", true, true, 0},
{"ea::huffUnpack", true, true, 0},
// edx left as the game's memset (0x4e0640) leaves it after a fill: its code
// spreads the byte through edx, native watcom::memset keeps it (its own
// promise compares none of its arguments' registers); Watcom's callers count
// on neither.
{"ea::runUnpack", true, true, 2},
{"ea::dxtBlock", true, true, 0},
{"ea::dxtUnpack", true, true, 0},
{"ea::pairExpand", true, true, 0},
{"ea::pairUnpack", true, true, 0},
{"ea::layerSend", true, true, 0},
{"ea::layerCallPacker", true, true, 0},
{"ea::layerPackFromTop", true, true, 0},
{"ea::layerFree", true, true, 0},
{"ea::layerSendPacket", true, true, 0},
#elif defined(EA_ARGUMENTS)
/* Where the network's cases keep their own: 16 queues, 32 connections of
 * 0x40, 32 layers of 0x20, a packet, pieces and their bytes, the byte pairs'
 * tables, the functions' table and the pointers' slots, what is unpacked
 * and where to. */
constexpr x86::reg32 kNetQueues = 0x61d000, kNetElements = 0x61d200, kNetLayers = 0x61da00;
constexpr x86::reg32 kNetPacket = 0x61e000, kNetPieces = 0x61e200, kNetPieceBytes = 0x61e300;
constexpr x86::reg32 kNetPairs = 0x61e800, kNetFunctions = 0x61ee00, kNetPriority = 0x61ee80, kNetPacker = 0x61ee84;
constexpr x86::reg32 kNetSource = 0x61f000, kNetOut = 0x61f800;
// The game's queues: the players, the connections, those closing.
constexpr x86::reg32 kNetGameQueues[] = {0x9f25f0, 0x9f260c, 0x9f2628};

/* A queue at `q` holding `elements` in order: its count mostly theirs, its
 * flags, its first and last, a priority's function now and then, its lock
 * (now and then none: uninitialized). */
void netQueue(CheckApp& g, x86::reg32 q, const std::vector<x86::reg32>& elements, bool exact = false)
{
    for (size_t i = 0; i < elements.size(); ++i)
        g.word(elements[i]) = i + 1 < elements.size() ? elements[i + 1] : 0;
    g.word(q) = exact || chance(85) ? x86::reg32(elements.size()) : below(6);
    g.word(q + 4) = random32() & (chance(70) ? 3 : 0xff);
    g.word(q + 8) = elements.empty() ? 0 : elements.front();
    g.word(q + 0xc) = elements.empty() ? 0 : elements.back();
    g.word(q + 0x10) = chance(25) ? standInFor(kNetPriority) : 0;
    g.word(q + 0x14) = random32();
    g.word(q + 0x18) = chance(90) ? 0x7800 + below(0x100) * 4 : 0;
    if (q < 0x9f2000)
        g.word(q + 0x1c) = random32();
}

/* A connection's element: its functions, open, up, its layers' queue. */
void netConnection(CheckApp& g, x86::reg32 c, x86::reg32 layers)
{
    g.word(c + 4) = chance(95) ? kNetFunctions : 0x61ee50;
    g.word(c + 8) = kNetFunctions;  // as a NET object's: its player's check
    g.word(c + 0x10) = chance(75) ? 1 + below(3) : 0;
    g.word(c + 0x24) = chance(75) ? 1 : 0;
    // No layers only on a connection not up (one up has them: its packets go down them).
    g.word(c + 0x30) = g.word(c + 0x24) == 0 && chance(30) ? 0 : layers;
    g.getMemory<x86::reg8>(c + 0x34) = x86::reg8(random32());
}

/* The network's world: EA's counts, the functions' table, the queues -- the
 * game's three and 13 of layers, the elements none shares -- the 7
 * connections, the NET object, the sessions, the machines and the network's
 * state; the pieces a packet goes in. */
void makeEaNetWorld(CheckApp& g)
{
    g.word(0x566de0) = chance(50) ? 0 : chance(80) ? 1 : random32();
    g.word(0x566df4) = chance(80) ? 1 : 0;
    g.word(0x566df8) = chance(70) ? below(3) : random32();
    for (x86::reg32 at = kNetFunctions; at < kNetFunctions + 0x80; at += 4)
        g.word(at) = random32();
    for (const x86::reg32 slot : {0x61ee08u, 0x61ee10u, 0x61ee24u, 0x61ee2cu, 0x61ee34u, 0x61ee40u})
        g.word(slot) = standInFor(slot);
    g.word(kNetFunctions + 0xc) = chance(60) ? 1 : 0;
    // A table of another kind of connection: its send none (0, the end of the game), its close the first's.
    for (x86::reg32 at = 0x61ee50; at < 0x61ee80; at += 4)
        g.word(at) = 0;
    g.word(0x61ee50 + 0xc) = 0x61ee08;
    g.word(0x61ee50 + 0x10) = standInFor(0x61ee10);
    g.word(kNetPriority) = 0;
    g.word(kNetPacker) = 0;
    g.word(0x61ee88) = 0;
    // Functions whose send fails mostly (0x61eec0), its close the first's.
    for (x86::reg32 at = 0x61eec0; at < 0x61ef00; at += 4)
        g.word(at) = 0;
    g.word(0x61eec8) = standInFor(0x61eec8);
    g.word(0x61eec0 + 0xc) = 1;
    g.word(0x61eec0 + 0x10) = standInFor(0x61ee10);
    g.word(0x7a21b8) = standInFor(0x7a21b8);
    // The layers: each queue of them 0 to 3 of the 32, each a packer and its data.
    x86::reg32 nextLayer = 0;
    for (x86::reg32 at = kNetLayers; at < kNetLayers + 0x400; at += 4)
        g.word(at) = 0;
    for (x86::reg32 k = 3; k < 16; ++k)
    {
        std::vector<x86::reg32> layers;
        const x86::reg32 n = chance(30) ? 0 : 1 + below(3);
        for (x86::reg32 i = 0; i < n && nextLayer < 32; ++i, ++nextLayer)
        {
            const x86::reg32 l = kNetLayers + 0x20 * nextLayer;
            g.word(l) = 0;
            g.word(l + 4) = standInFor(kNetPacker);
            g.word(l + 8) = random32();
            g.word(l + 0xc) = random32();
            layers.push_back(l);
        }
        // Its count theirs: a layer past the list would be packed by what an
        // element 0 points at, which a block allocated as 0 may have been
        // written over by then.
        netQueue(g, kNetQueues + 0x20 * k, layers, true);
    }
    // What the packer of an element 0 reads (layerCallPacker after its Error).
    g.word(4) = standInFor(kNetPacker);
    const auto aLayers = [] { return kNetQueues + 0x20 * (3 + below(13)); };
    // The elements: connections, the first 32 shared out among the game's
    // three queues and three of the cases'.
    for (x86::reg32 i = 0; i < 32; ++i)
    {
        const x86::reg32 c = kNetElements + 0x40 * i;
        for (x86::reg32 at = c; at < c + 0x40; at += 4)
            g.word(at) = random32();
        g.word(c) = 0;
        netConnection(g, c, aLayers());
    }
    x86::reg32 next = 0;
    for (x86::reg32 q = 0; q < 6; ++q)
    {
        std::vector<x86::reg32> elements;
        const x86::reg32 n = chance(20) ? 0 : 1 + below(5);
        for (x86::reg32 i = 0; i < n && next < 32; ++i, ++next)
            elements.push_back(kNetElements + 0x40 * next);
        netQueue(g, q < 3 ? kNetGameQueues[q] : kNetQueues + 0x20 * (q - 3), elements);
    }
    // The 7 connections, the NET object, the pending players, the links.
    for (x86::reg32 i = 0; i < 7; ++i)
    {
        const x86::reg32 c = 0x7a22e8 + 0x6c * i;
        for (x86::reg32 at = c; at < c + 0x6c; at += 4)
            g.word(at) = random32();
        netConnection(g, c, aLayers());
    }
    g.word(0x7a0390) = random32();
    g.word(0x7a0394) = kNetFunctions;
    g.word(0x7a0398) = kNetFunctions;
    g.word(0x7a039c) = random32();
    for (x86::reg32 i = 0; i < 7; ++i)
        g.word(0x7a02e8 + 4 * i) = chance(50) ? 0 : random32();
    g.word(0x7a01e0) = below(8);
    // The sessions: a few, their ids and flags; the current one's id one of them mostly.
    const x86::reg32 sessions = chance(90) ? below(6) : chance(50) ? 0x80000000 | random32() : 0;
    g.word(0x55e718) = sessions;
    for (x86::reg32 i = 0; i < 6; ++i)
    {
        g.word(0x55e734 + 0x2c * i) = chance(50) ? below(4) : random32();
        g.word(0x55e738 + 0x2c * i) = chance(50) ? 0 : 1;
    }
    g.word(0x55e70c) = chance(50) ? 0 : 1;  // the flag before the first, of session -1
    g.word(0x55e71c) = chance(70) && sessions < 6 && sessions != 0 ? g.word(0x55e734 + 0x2c * below(sessions))
                                                                    : below(4);
    g.word(0x55e860) = chance(50) ? 0 : 1;
    g.word(0x55e864) = random32();
    g.word(0x55ecb0) = random32();
    g.word(0x7a215c) = random32();
    // The machines: their connections (the 7 mostly), their count, their players and states.
    for (x86::reg32 i = 0; i < 9; ++i)
        g.word(0x7a21f0 + 4 * i) = chance(80) ? 0x7a22e8 + 0x6c * below(7) : chance(50) ? 0 : random32();
    g.word(0x7a21ec) = random32();
    g.word(0x7a2274) = (chance(90) ? below(9) : random32()) << 16 | (random32() & 0xffff);
    for (x86::reg32 at = 0x7a2294; at < 0x7a22da; at += 2)
        g.getMemory<x86::reg16>(at) = x86::reg16(random32());
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g.getMemory<x86::reg16>(0x7a227a + 2 * i) = x86::reg16(chance(80) ? below(8) : random32());
        g.getMemory<x86::reg16>(0x7a22ca + 2 * i) = x86::reg16(chance(60) ? 0 : chance(80) ? below(3) : random32());
    }
    g.getMemory<x86::reg16>(0x7a22da) = x86::reg16(chance(50) ? 0 : chance(90) ? 1 : random32());
    g.getMemory<x86::reg16>(0x7a22dc) = x86::reg16(chance(80) ? 0 : 1);
    g.getMemory<x86::reg16>(0x7a22de) = x86::reg16(chance(80) ? 0 : 1);
    g.word(0x7a22e0) = chance(60) ? 3 : below(5);
    g.word(0x7a22e4) = chance(90) ? below(3) : random32();
    g.word(0x7a2290) = random32();
    g.word(0x6fd50c) = random32();
    // The pieces: up to 8 of {length, address}, 0x400 bytes at most in all.
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g.word(kNetPieces + 8 * i) = below(0x80);
        g.word(kNetPieces + 8 * i + 4) = kNetPieceBytes + below(0x380);
    }
    for (x86::reg32 at = kNetPieceBytes; at < kNetPieceBytes + 0x400; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = kNetPacket; at < kNetPacket + 0x200; at += 4)
        g.word(at) = random32();
}

/* Bits written from the top, as EA's Huffman reads them. */
struct NetBits
{
    std::vector<x86::reg8> bytes;
    x86::reg32 acc = 0;
    int count = 0;
    void put(x86::reg32 value, int n)
    {
        for (int i = n - 1; i >= 0; --i)
        {
            acc = acc << 1 | (value >> i & 1);
            if (++count == 8)
            {
                bytes.push_back(x86::reg8(acc));
                acc = 0;
                count = 0;
            }
        }
    }
    /* A count: 2 + z bits after z 0s and a 1, less 4. */
    void number(x86::reg32 value)
    {
        int n = 2;
        while (value >= (1u << (n + 1)) - 4)
            ++n;
        put(0, n - 2);
        put(1, 1);
        put(value - ((1u << n) - 4), n);
    }
};

/* A Huffman file at kNetSource, for kNetOut: a complete code of a few
 * symbols (up to 16 bits), the clue among them, bytes, runs, bytes escaped
 * and the end; its length as unpacked mostly. */
void makeEaHuffman(CheckApp& g)
{
    static const x86::reg32 types[] = {0x30fb, 0x31fb, 0x32fb, 0x33fb, 0x34fb, 0x35fb};
    const x86::reg32 type = chance(95) ? types[below(6)] : 0x30fb | (below(0x100) << 8);
    // The code: leaves split until there are enough of them.
    const x86::reg32 most = chance(10) ? 256 : chance(50) ? 2 + below(8) : 2 + below(60);
    // 15 bits at most: a 16-bit length's offset (the frame's +0x4c0) is where
    // a run's count keeps where it reads, the game's code's own clash.
    const int deepest = chance(30) ? 15 : 4 + int(below(10));
    std::vector<int> depths = {0};
    while (depths.size() < most)
    {
        std::vector<size_t> open;
        for (size_t i = 0; i < depths.size(); ++i)
            if (depths[i] < deepest)
                open.push_back(i);
        if (open.empty())
            break;
        const size_t pick = chance(50) ? open[below(x86::reg32(open.size()))] : open.back();
        const int d = depths[pick] + 1;
        depths[pick] = d;
        depths.push_back(d);
    }
    std::sort(depths.begin(), depths.end());
    const int longest = depths.back();
    std::vector<x86::reg32> counts(longest + 1, 0);
    for (const int d : depths)
        ++counts[d];
    // The symbols in their codes' order: distinct bytes, the clue among them.
    std::vector<x86::reg8> all(256);
    for (x86::reg32 i = 0; i < 256; ++i)
        all[i] = x86::reg8(i);
    for (x86::reg32 i = 255; i > 0; --i)
        std::swap(all[i], all[below(i + 1)]);
    std::vector<x86::reg8> symbols(all.begin(), all.begin() + depths.size());
    const x86::reg8 clue = symbols[below(x86::reg32(symbols.size()))];
    // Each symbol's code and length, as the decoder numbers them.
    std::vector<x86::reg32> code(256, 0), length(256, 0);
    x86::reg32 start = 0, index = 0;
    for (int l = 1; l <= longest; ++l)
    {
        start *= 2;
        for (x86::reg32 k = 0; k < counts[l]; ++k, ++index)
        {
            code[symbols[index]] = start + k;
            length[symbols[index]] = l;
        }
        start += counts[l];
    }
    NetBits bits;
    bits.put(clue, 8);
    for (int l = 1; l <= longest; ++l)
        bits.number(counts[l]);
    // The symbols, each as how many not yet taken to skip from the one before.
    std::vector<bool> used(256, false);
    x86::reg32 at = 0xff;
    for (const x86::reg8 s : symbols)
    {
        x86::reg32 skip = 0;
        for (x86::reg32 c = (at + 1) & 0xff; c != s; c = (c + 1) & 0xff)
            if (!used[c])
                ++skip;
        x86::reg32 unused = 0;
        for (x86::reg32 c = 0; c < 256; ++c)
            unused += !used[c];
        // Now and then many times round: the count with 16 0s first, which
        // the decoder reads 19 bits after.
        if (chance(2) && unused > 0)
        {
            const x86::reg32 bitsAfter = skip + unused * (0x80000 / unused + 1) - (0x80000 - 4);
            bits.put(0, 16);
            bits.put(1, 1);
            bits.put(bitsAfter, 19);
        }
        else
            bits.number(skip);
        used[s] = true;
        at = s;
    }
    const auto emit = [&](x86::reg8 s) { bits.put(code[s], int(length[s])); };
    x86::reg32 out = 0;
    const x86::reg32 target = chance(10) ? below(4) : below(0x600);
    // As much as kNetSource holds, the header and the end's bits aside.
    while (out < target && bits.bytes.size() < 0x7c0)
    {
        const x86::reg32 what = below(20);
        if (what == 0)
        {
            const x86::reg32 run = 1 + below(chance(80) ? 8 : 40);
            emit(clue);
            bits.number(run);
            out += run;
        }
        else if (what == 1)
        {
            emit(clue);
            bits.number(0);
            bits.put(0, 1);
            bits.put(random32() & 0xff, 8);
            ++out;
        }
        else
        {
            x86::reg8 s = symbols[below(x86::reg32(symbols.size()))];
            if (s == clue)
                continue;
            emit(s);
            ++out;
        }
    }
    emit(clue);
    bits.number(0);
    bits.put(1, 1);
    bits.put(random32(), 32);
    bits.put(random32(), 32);
    const x86::reg32 declared = chance(80) ? out : below(0x700);
    x86::reg32 p = kNetSource;
    g.getMemory<x86::reg8>(p++) = x86::reg8(type >> 8);
    g.getMemory<x86::reg8>(p++) = x86::reg8(type);
    if (type & 0x100)
        for (int i = 0; i < 3; ++i)
            g.getMemory<x86::reg8>(p++) = x86::reg8(random32());
    g.getMemory<x86::reg8>(p++) = x86::reg8(declared >> 16);
    g.getMemory<x86::reg8>(p++) = x86::reg8(declared >> 8);
    g.getMemory<x86::reg8>(p++) = x86::reg8(declared);
    for (const x86::reg8 b : bits.bytes)
        if (p < kNetOut)
            g.getMemory<x86::reg8>(p++) = b;
}

/* A runs' file at kNetSource: codes until their counts reach its length. */
void makeEaRuns(CheckApp& g)
{
    const x86::reg32 type = chance(80) ? 0x4afb : chance(50) ? 0x4b31 : random32() & 0xffff;
    const x86::reg32 length = chance(10) ? below(3) : below(0x600);
    x86::reg32 p = kNetSource;
    const auto put = [&](x86::reg32 b) {
        if (p < kNetOut)
            g.getMemory<x86::reg8>(p++) = x86::reg8(b);
    };
    put(type >> 8);
    put(type);
    if (type != 0x4b31)
        put(length >> 16);
    put(length >> 8);
    put(length);
    x86::reg32 done = 0;
    while (done < length && p < kNetOut - 0x50)
    {
        const x86::reg32 op = below(4), n = chance(5) ? 0 : 1 + below(chance(70) ? 16 : 63);
        put(op << 6 | n);
        if (op == 0)
            for (x86::reg32 i = 0; i < n; ++i)
                put(random32());
        else if (op == 1)
            put(random32());
        else if (op == 3)
            for (x86::reg32 i = 0; i < 1 + n / 2; ++i)
                put(random32());
        done += n;
    }
    while (p < kNetOut)
        put(random32());
}

/* A DXT1 image at kNetSource: its length (4 rows of blocks' worth, about),
 * its width, its blocks -- no more than kNetOut holds. */
void makeEaDxt(CheckApp& g)
{
    const x86::reg32 width = chance(80) ? 4 * (1 + below(16)) : 1 + below(64);
    const x86::reg32 rows = 1 + below(0x7f0 / (8 * width) < 3 ? 0x7f0 / (8 * width) : 3);
    x86::reg32 length = rows * 8 * width;
    if (chance(30))
        length -= below(8 * width);
    if (chance(5))
        length = 0;
    x86::reg32 p = kNetSource;
    g.getMemory<x86::reg8>(p++) = 0x18 | x86::reg8(below(2));
    g.getMemory<x86::reg8>(p++) = 0xfb;
    g.getMemory<x86::reg8>(p++) = x86::reg8(length >> 16);
    g.getMemory<x86::reg8>(p++) = x86::reg8(length >> 8);
    g.getMemory<x86::reg8>(p++) = x86::reg8(length);
    g.getMemory<x86::reg8>(p++) = x86::reg8(random32());
    g.getMemory<x86::reg8>(p++) = x86::reg8(width >> 8);
    g.getMemory<x86::reg8>(p++) = x86::reg8(width);
    for (; p < kNetOut; ++p)
        g.getMemory<x86::reg8>(p) = x86::reg8(random32());
}

/* The byte pairs: `count` pairs in `codes`, each of literals or pairs
 * before it, into the tables at kNetPairs (kinds, firsts, seconds); the
 * escape's two literals; each byte's length unpacked.  The literals: bytes
 * of no pair, not the escape. */
void makeEaPairs(CheckApp& g, x86::reg8 escape, std::vector<x86::reg8>& codes, std::vector<x86::reg8>& literals,
                 std::vector<x86::reg32>& lengths, std::vector<x86::reg8>& firsts, std::vector<x86::reg8>& seconds)
{
    std::vector<x86::reg8> all;
    for (x86::reg32 i = 0; i < 256; ++i)
        if (i != escape)
            all.push_back(x86::reg8(i));
    for (size_t i = all.size() - 1; i > 0; --i)
        std::swap(all[i], all[below(x86::reg32(i + 1))]);
    const x86::reg32 count = chance(10) ? 0 : 1 + below(chance(50) ? 8 : 60);
    codes.assign(all.begin(), all.begin() + count);
    literals.assign(all.begin() + count, all.end());
    lengths.assign(256, 1);
    firsts.assign(256, 0);
    seconds.assign(256, 0);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        const auto child = [&] {
            return i > 0 && chance(40) ? codes[below(i)] : literals[below(x86::reg32(literals.size()))];
        };
        firsts[codes[i]] = child();
        seconds[codes[i]] = child();
        lengths[codes[i]] = std::min<x86::reg32>(0x10000, lengths[firsts[codes[i]]] + lengths[seconds[codes[i]]]);
    }
    for (x86::reg32 at = kNetPairs; at < kNetPairs + 0x300; at += 4)
        g.word(at) = random32();
    for (x86::reg32 i = 0; i < 256; ++i)
        g.getMemory<x86::reg8>(kNetPairs + i) = 0;
    g.getMemory<x86::reg8>(kNetPairs + escape) = 1;
    firsts[escape] = literals[0];
    seconds[escape] = literals[1];
    g.getMemory<x86::reg8>(kNetPairs + 0x100 + escape) = firsts[escape];
    g.getMemory<x86::reg8>(kNetPairs + 0x200 + escape) = seconds[escape];
    for (const x86::reg8 c : codes)
    {
        g.getMemory<x86::reg8>(kNetPairs + c) = 0xff;
        g.getMemory<x86::reg8>(kNetPairs + 0x100 + c) = firsts[c];
        g.getMemory<x86::reg8>(kNetPairs + 0x200 + c) = seconds[c];
    }
}

/* A byte pairs' file at kNetSource: its header, pairs, then bytes up to
 * about 0x600 unpacked, the escape and 0 at the end. */
void makeEaPairFile(CheckApp& g)
{
    const x86::reg8 escape = x86::reg8(random32());
    std::vector<x86::reg8> codes, literals, firsts, seconds;
    std::vector<x86::reg32> lengths;
    makeEaPairs(g, escape, codes, literals, lengths, firsts, seconds);
    x86::reg32 p = kNetSource;
    const auto put = [&](x86::reg32 b) { g.getMemory<x86::reg8>(p++) = x86::reg8(b); };
    const bool longer = chance(50);
    put(longer ? 0x47 : 0x46);
    put(0xfb);
    if (longer)
        for (int i = 0; i < 3; ++i)
            put(random32());
    const x86::reg32 at = p;
    p += 3;
    put(escape);
    put(x86::reg32(codes.size()));
    for (const x86::reg8 c : codes)
    {
        put(c);
        put(firsts[c]);
        put(seconds[c]);
    }
    x86::reg32 out = 0;
    const x86::reg32 target = below(0x600);
    while (out < target && p < kNetOut - 8)
    {
        if (!codes.empty() && chance(40))
        {
            const x86::reg8 c = codes[below(x86::reg32(codes.size()))];
            if (lengths[c] > 0x80)
                continue;
            put(c);
            out += lengths[c];
        }
        else if (chance(10))
        {
            put(escape);
            put(1 + below(255));
            ++out;
        }
        else
        {
            put(literals[below(x86::reg32(literals.size()))]);
            ++out;
        }
    }
    put(escape);
    put(0);
    const x86::reg32 declared = chance(80) ? out : random32() & 0xffffff;
    g.getMemory<x86::reg8>(at) = x86::reg8(declared >> 16);
    g.getMemory<x86::reg8>(at + 1) = x86::reg8(declared >> 8);
    g.getMemory<x86::reg8>(at + 2) = x86::reg8(declared);
}

bool makeEaNetArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "dropMachine", "announceMachineLeft", "endNetworkGame", "sessionIndex", "forgetPendingPlayer",
        "sendOverLink", "sendPacket", "resetNetwork", "closeNetwork", "machineNumber", "playerMachine",
        "netTrace", "netShutGame", "netEndGame", "netKillPlayer", "netCheckPlayer", "connRestore", "connSend",
        "connClose", "qnth", "qunlink", "qfree", "qremove", "qlength", "qposition", "qfindindex", "huffUnpack",
        "runUnpack", "dxtBlock", "dxtUnpack", "pairExpand", "pairUnpack", "layerSend", "layerCallPacker",
        "layerPackFromTop", "layerFree", "layerSendPacket"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeEaNetWorld(g);
    // CONN_restore's loops go on while the stand-ins close nothing: bounded.
    g_mostCalls = 300;
    for (x86::reg32 at = kNetSource; at < kNetOut + 0x800; at += 4)
        g.word(at) = random32();
    const auto aConnection = [] {
        return chance(50) ? 0x7a22e8 + 0x6c * below(7) : kNetElements + 0x40 * below(32);
    };
    const auto aConnectionOrNone = [&] { return chance(10) ? 0 : aConnection(); };
    // A queue: the game's, the cases' with connections, or one of layers.
    const auto aQueue = [] {
        return chance(30) ? kNetGameQueues[below(3)] : kNetQueues + 0x20 * below(chance(60) ? 3 : 16);
    };
    // An element of the queue mostly, of another, or none.
    const auto anElementOf = [&](x86::reg32 q) {
        std::vector<x86::reg32> in;
        for (x86::reg32 e = g.word(q + 8); e != 0 && in.size() < 64; e = g.word(e))
            in.push_back(e);
        if (!in.empty() && chance(70))
            return in[below(x86::reg32(in.size()))];
        return chance(30) ? 0 : chance(50) ? kNetElements + 0x40 * below(32) : kNetLayers + 0x20 * below(32);
    };
    const auto aPieceCount = [] { return chance(90) ? below(5) : chance(50) ? 8 : x86::reg32(-1 - x86::sreg32(below(3))); };
    if (name == "dropMachine" || name == "connClose" || name == "layerFree")
        cpu.eax = aConnectionOrNone();
    else if (name == "announceMachineLeft")
        cpu.eax = chance(90) ? below(9) : 0xffffffff;
    else if (name == "sessionIndex")
        cpu.eax = chance(70) ? below(4) : random32();
    else if (name == "forgetPendingPlayer")
        cpu.eax = aConnection() + 0x34;
    else if (name == "sendOverLink" || name == "sendPacket" || name == "connSend")
    {
        cpu.eax = name == "connSend" ? aConnectionOrNone() : chance(40) ? 0 : aConnection();
        cpu.edx = kNetPacket + below(0x10);
        cpu.ebx = chance(90) ? 1 + below(0x40) : below(0x100);
        if (chance(30))
            g.getMemory<x86::reg8>(cpu.edx) = 6;
        /* The machines' states not 0, so that a machine announced gone from
         * here is not told to the others: that packet (0x12 bytes) lies in
         * announceMachineLeft's frame over the words connSend's layers pushed
         * there, its second byte and the two after it never set, and the
         * send's stand-in logs 9 words of it -- garbage the native layers do
         * not push.  announceMachineLeft's own cases send it. */
        if (name == "sendPacket")
            for (x86::reg32 i = 0; i < 8; ++i)
                g.getMemory<x86::reg16>(0x7a22ca + 2 * i) |= 1;
        // Connections that fail mostly, to be tried again until lost.
        if (name == "sendPacket" && chance(50))
        {
            for (x86::reg32 i = 0; i < 7; ++i)
                g.word(0x7a22e8 + 0x6c * i + 4) = 0x61eec0;
            for (x86::reg32 i = 0; i < 32; ++i)
                g.word(kNetLayers + 0x20 * i + 4) = standInFor(0x61ee88);
        }
    }
    else if (name == "resetNetwork")
        cpu.eax = chance(50) ? 0 : 1;
    else if (name == "machineNumber")
        cpu.eax = chance(10) ? 0 : chance(80) ? 0x7a22e8 + 0x6c * below(7) : random32();
    else if (name == "playerMachine")
        cpu.eax = (random32() & 0xffff0000) | (chance(80) ? below(9) : random32() & 0xffff);
    else if (name == "netTrace")
        cpu.eax = chance(50) ? 0x54d138 : aString();
    else if (name == "netShutGame" || name == "netEndGame" || name == "netKillPlayer" || name == "netCheckPlayer")
    {
        cpu.eax = chance(50) ? 0x7a0390 : kNetElements + 0x40 * below(32);
        cpu.edx = chance(50) ? aConnection() : random32();
    }
    else if (name == "qnth")
    {
        const x86::reg32 q = aQueue();
        cpu.eax = chance(80) ? q + 8 : kNetElements + 0x40 * below(32);
        cpu.edx = chance(85) ? below(7) : random32();
    }
    else if (name == "qunlink" || name == "qremove" || name == "qposition")
    {
        cpu.eax = aQueue();
        cpu.edx = anElementOf(cpu.eax);
    }
    else if (name == "qfree" || name == "qlength")
        cpu.eax = aQueue();
    else if (name == "qfindindex")
    {
        cpu.eax = aQueue();
        cpu.edx = chance(85) ? below(7) : random32();
    }
    else if (name == "huffUnpack")
    {
        makeEaHuffman(g);
        cpu.eax = chance(3) ? 0 : kNetSource;
        cpu.edx = kNetOut;
        cpu.ebx = chance(50) ? 1 : random32() | 1;
    }
    else if (name == "runUnpack")
    {
        makeEaRuns(g);
        cpu.eax = kNetSource;
        cpu.edx = kNetOut;
    }
    else if (name == "dxtBlock")
    {
        cpu.eax = kNetOut + below(0x100);
        cpu.edx = chance(80) ? 8 * (1 + below(32)) : below(0x100);
        cpu.ebx = kNetSource + below(0x700);
        // The two colours the same now and then.
        if (chance(20))
            g.getMemory<x86::reg16>(cpu.ebx + 2) = g.getMemory<x86::reg16>(cpu.ebx);
    }
    else if (name == "dxtUnpack")
    {
        makeEaDxt(g);
        cpu.eax = kNetSource;
        cpu.edx = kNetOut;
        cpu.ebx = chance(15) ? 0 : chance(50) ? 1 : random32();
    }
    else if (name == "pairExpand")
    {
        const x86::reg8 escape = x86::reg8(random32());
        std::vector<x86::reg8> codes, literals, firsts, seconds;
        std::vector<x86::reg32> lengths;
        makeEaPairs(g, escape, codes, literals, lengths, firsts, seconds);
        g.word(0xa0b040) = kNetPairs;
        g.word(0xa0b034) = kNetPairs + 0x100;
        g.word(0xa0b03c) = kNetPairs + 0x200;
        g.word(0xa0b030) = kNetOut + below(0x100);
        x86::reg8 c = literals[below(x86::reg32(literals.size()))];
        if (!codes.empty() && chance(70))
        {
            c = codes[below(x86::reg32(codes.size()))];
            for (int i = 0; i < 8 && lengths[c] > 0x400; ++i)
                c = firsts[c];
        }
        if (lengths[c] > 0x400 || chance(5))
            c = escape;
        cpu.eax = (random32() & 0xffffff00) | c;
    }
    else if (name == "pairUnpack")
    {
        makeEaPairFile(g);
        cpu.eax = chance(3) ? 0 : kNetSource;
        cpu.edx = kNetOut;
    }
    else if (name == "layerSend" || name == "layerSendPacket")
    {
        cpu.eax = name == "layerSend" ? aConnection() : aConnectionOrNone();
        cpu.edx = aPieceCount();
        cpu.ebx = kNetPieces;
    }
    else if (name == "layerCallPacker" || name == "layerPackFromTop")
    {
        cpu.eax = aConnection();
        const x86::reg32 q = kNetQueues + 0x20 * (3 + below(13));
        if (name == "layerCallPacker")
        {
            cpu.edx = chance(85) ? below(4) : random32();
            cpu.ebx = aPieceCount();
            cpu.ecx = kNetPieces;
            stack[0] = q;
        }
        else
        {
            cpu.edx = aPieceCount();
            cpu.ebx = kNetPieces;
            cpu.ecx = q;
        }
    }
    return true;
}
#endif
