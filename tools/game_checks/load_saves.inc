// game_checks.cpp's parts for the files the game saves and EA's formats under the loaders: replays, records, config.dat; BIG archives, savefile, shape comments
// (native_load_saves.cpp), included in each table with the macro naming the part.
#if defined(LOAD_WINDOWS)
// {from, to}, -- what the functions write, compared
// The race's replay past its first page (its version, log and seed at
// 0x7a56b0, 0x7a5f40); the seed kept (0x55fe40); the random byte (0x57126b);
// config.dat made afresh (0x67900c); the controllers known (0x55b038).  The
// rest -- config.dat's settings (0x6fb000 on), the records (0x559408), the
// replays' lines (0x558d0c) -- on pages compared already; the cases' own in
// the harness's (0x61d000 on), and the replay a file loaded is in the
// raceload's (0xd0a000).
{0x7a4000, 0x7a6000}, {0x55f000, 0x560000}, {0x571000, 0x572000}, {0x679000, 0x67a000}, {0x55b000, 0x55c000},
#elif defined(LOAD_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kLoadSavesSaved,     // savefileadr (0x4eeae0, 0x4eebb0): the bytes written -- all of them mostly, or none
kLoadSavesFound,     // a file found (0x4f0330, 0x4f0280): its name, with its extension or not; or none
kLoadSavesSine,      // the fixed sine and cosine (0x4f2380, 0x4f23f0): a 16.16 value within one
kLoadSavesOlder,     // one file older than another (0x472940): 0 or 1
kLoadSavesOpen,      // _lopen: a file, HFILE_ERROR (-1), or 0
kLoadSavesFileTime,  // GetFileTime: the last write's FILETIME, or FALSE
kLoadSavesCompare,   // CompareFileTime: the two FILETIMEs' order, -1, 0 or 1
kLoadSavesRoom,      // whether there is room to save (0x4516a0): 1 mostly, 0 now and then
#elif defined(LOAD_EFFECT_CASES)
// case kSomething: ... break;
case kLoadSavesSaved:
    // (the name, the bytes, how many)
    result = r % 5 == 0 ? 0 : words[2];
    break;
case kLoadSavesFound:
{
    // Into the cases' name buffer (0x61fe00): a few letters, ".rp3" after
    // them mostly, none now and then.
    result = r % 5 == 0 ? 0 : 0x61fe00;
    if (result != 0)
    {
        const x86::reg32 length = 1 + (r >> 4) % 12;
        text(result, length, random64(id));
        for (x86::reg32 i = 0; i < length; ++i)
            if (getMemory<x86::reg8>(result + i) == '.')
                getMemory<x86::reg8>(result + i) = 'x';
        if ((r >> 8) % 6 != 0)
            std::memcpy(&getMemory<char>(result + length), ".rp3", 5);
    }
    break;
}
case kLoadSavesSine:
    result = x86::reg32(x86::sreg32(r % 0x20001) - 0x10000);
    break;
case kLoadSavesOlder:
    result = r & 1;
    break;
case kLoadSavesRoom:
    result = r % 4 != 0 ? 1 : 0;
    break;
case kLoadSavesOpen:
    result = r % 5 == 0 ? 0xffffffff : r % 7 == 0 ? 0 : 0x7000 + (r >> 8) % 0x100 * 4;
    break;
case kLoadSavesFileTime:
    // (the file, &created, &accessed, &written)
    result = r % 5 != 0 ? 1 : 0;
    if (result != 0 && words[3] != 0 && words[3] < kMemory - 8)
    {
        word(words[3]) = random(id + 1);
        word(words[3] + 4) = r % 3 == 0 ? 0x1bd0000 : random(id + 2) % 4 + 0x1bd0000;
    }
    break;
case kLoadSavesCompare:
{
    const auto time = [&](x86::reg32 at) {
        return at < kMemory - 8 ? std::uint64_t(word(at + 4)) << 32 | word(at) : 0;
    };
    const std::uint64_t a = time(words[0]), b = time(words[1]);
    result = a < b ? 0xffffffff : a > b ? 1 : 0;
    break;
}
#elif defined(LOAD_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// This file's that other files' natives call by address, each tested on its
// own: savefileadr and its quiet entry (the name's first bytes logged; ebx and
// edx as the open's and the write's left them), a file older than another
// (edx as CompareFileTime left it), the fixed sine and cosine.
{0x4eeae0, "savefileadr", kWatcom, 3, 0, kEax | kEbx | kEdx, 0, 4, kLoadSavesSaved},
{0x4eebb0, "savefileadr quietly", kWatcom, 3, 0, kEax | kEbx | kEdx, 0, 4, kLoadSavesSaved},
{0x472940, "a file older than another", kWatcom, 2, 0, kEax | kEdx, 0, 4, kLoadSavesOlder},
{0x4f2380, "fixed sine", kWatcom, 1, 0, kEax, -1, 0, kLoadSavesSine},
{0x4f23f0, "fixed cosine", kWatcom, 1, 0, kEax, -1, 0, kLoadSavesSine},
// The runtime's search of a folder (findfirst and findnext, the game's own):
// the pattern's first bytes logged; a name found, or none.
{0x4f0330, "a file found first", kWatcom, 1, 0, kEax, 0, 4, kLoadSavesFound},
{0x4f0280, "a file found next", kWatcom, 0, 0, kEax, -1, 0, kLoadSavesFound},
// controls::loadControllers, which only loadConfig calls: held against its
// code on its own.
{0x472b60, "the controllers loaded", kWatcom, 0, 0, kEax, -1, 0, kNothing},
// Whether there is room on the disk for so many bytes, which saveConfig asks.
{0x4516a0, "room to save", kWatcom, 1, 0, kEax, -1, 0, kLoadSavesRoom},
#elif defined(LOAD_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// Windows' functions a file's age is asked of: the name's first bytes logged,
// the first time's words.
{0x534648, "_lopen", kStdcall, 0, 2, kScratch, 4, 4, kLoadSavesOpen},
{0x534528, "GetFileTime", kStdcall, 0, 4, kScratch, -1, 0, kLoadSavesFileTime},
{0x534644, "_lclose", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
{0x53448c, "CompareFileTime", kStdcall, 0, 2, kScratch, 4, 2, kLoadSavesCompare},
#elif defined(LOAD_PROMISES)
// {"load::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared -- but edx where a BIG archive's entry is looked up
// by its name: the game's _stricmp (0x4e9ec0) leaves the difference in edx,
// native watcom::stricmp its argument, and an entry not found or found after
// one leaves edx the last _stricmp's.  As the port runs, both sides call the
// native one.
{"load::readReplayInfo", true, true, 0},
{"load::saveGhostAs", true, true, 0},
{"load::loadKeptReplay", true, true, 0},
{"load::saveReplayAs", true, true, 0},
{"load::listReplays", true, true, 0},
{"load::loadRecords", true, true, 0},
{"load::saveRecords", true, true, 0},
{"load::languageStub", true, true, 0},
{"load::setLanguage", true, true, 0},
{"load::setCameraViews", true, true, 0},
{"load::defaultConfig", true, true, 0},
{"load::saveConfig", true, true, 0},
{"load::fileWriteTime", true, true, 0},
{"load::fileOlder", true, true, 0},
{"load::loadConfig", true, true, 0},
{"load::saveRaceReplay", true, true, 0},
{"load::readRaceReplay", true, true, 0},
{"load::typeOfBigFile", true, true, 0},
{"load::bigHeaderLength", true, true, 0},
{"load::findBigEntry", true, true, 2},
{"load::locateBigEntry", true, true, 2},
{"load::savefileadr", true, true, 3},
{"load::savefileadrQuietly", true, true, 3},
{"load::fixedSine", true, true, 0},
{"load::fixedCosine", true, true, 0},
#elif defined(LOAD_ARGUMENTS)
/* Where the saves' cases keep their own: names, a BIG archive, the replays'
 * list's record, what is written out, a file's name found. */
constexpr x86::reg32 kSavesNames = 0x61d000;    // 4 of 0x100
constexpr x86::reg32 kSavesList = 0x61d400;     // the list's record: 0x80
constexpr x86::reg32 kSavesOut = 0x61d500;      // 0x100
constexpr x86::reg32 kSavesBig = 0x61e000;      // 0x1000
constexpr x86::reg32 kSavesReplay = 0xd0a000;   // the replay a file loaded is (trackdata.inc's .ccm)
constexpr x86::reg32 kSavesLoaded = 0x7700;     // what loadFile gives (kLoad): 0x7700 to 0x77f0

/* A name into `at`: one of the game's, or letters, or nothing. */
x86::reg32 aSavesName(CheckApp& g, x86::reg32 at, const char* const* names, x86::reg32 count)
{
    std::string name = chance(60) ? names[below(count)] : std::string();
    if (name.empty() && chance(80))
        for (x86::reg32 i = 0, n = 1 + below(16); i < n; ++i)
            name += char(chance(10) ? "._ \\"[below(4)] : 'a' + below(26));
    for (x86::reg32 i = 0; i < name.size(); ++i)
        g.getMemory<x86::reg8>(at + i) = x86::reg8(name[i]);
    g.getMemory<x86::reg8>(at + x86::reg32(name.size())) = 0;
    return at;
}

/* A BIG archive at kSavesBig: 0xc0fb's or BIGF's, its entries' names from a
 * few (case aside, the first four letters alike), its header's length as
 * long as its entries mostly -- or garbage.  Its kind back, its entries'
 * count to `count`, a name of one of them (or another, its case changed) at
 * `name`. */
x86::reg32 makeSavesBig(CheckApp& g, x86::reg32& count, x86::reg32 name)
{
    static const char* const names[] = {"car.fsh", "CAR.FSH", "carp.txt", "carP.tx", "dash.qfs", "a", "ab",
                                        "abc", "abcd", "ABCDE", "tr00.frd", "tr00.col", "x\xc1y.z", ""};
    const x86::reg32 kind = chance(45) ? 1 : chance(80) ? 2 : 0;
    for (x86::reg32 at = kSavesBig; at < kSavesBig + 0x1000; at += 4)
        g.word(at) = chance(50) ? 0 : random32();
    count = chance(10) ? 0 : 1 + below(12);
    std::vector<std::string> taken;
    x86::reg32 at = kSavesBig + (kind == 1 ? 6 : 0x10);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        const std::string n = names[below(14)];
        taken.push_back(n);
        const x86::reg32 bytes = kind == 1 ? 3 : 4;
        const x86::reg32 offset = random32(), size = chance(80) ? below(0x10000) : random32();
        for (x86::reg32 b = 0; b < bytes; ++b)
        {
            g.getMemory<x86::reg8>(at + b) = x86::reg8(offset >> (8 * (bytes - 1 - b)));
            g.getMemory<x86::reg8>(at + bytes + b) = x86::reg8(size >> (8 * (bytes - 1 - b)));
        }
        at += 2 * bytes;
        for (const char c : n)
            g.getMemory<x86::reg8>(at++) = x86::reg8(c);
        g.getMemory<x86::reg8>(at++) = 0;
    }
    // The header: its length to the entries' end mostly, short of it or past
    // it now and then.
    x86::reg32 length = at - kSavesBig;
    if (chance(15))
        length = chance(50) ? below(length + 1) : length + below(0x40);
    if (kind == 1)
    {
        g.getMemory<x86::reg8>(kSavesBig) = 0xc0;
        g.getMemory<x86::reg8>(kSavesBig + 1) = 0xfb;
        g.getMemory<x86::reg8>(kSavesBig + 2) = x86::reg8((length - 4) >> 8);
        g.getMemory<x86::reg8>(kSavesBig + 3) = x86::reg8(length - 4);
        g.getMemory<x86::reg8>(kSavesBig + 4) = 0;
        g.getMemory<x86::reg8>(kSavesBig + 5) = x86::reg8(count);
    }
    else if (kind == 2)
    {
        std::memcpy(&g.getMemory<char>(kSavesBig), "BIGF", 4);
        g.word(kSavesBig + 8) = count << 24;
        g.word(kSavesBig + 0xc) = length >> 24 | (length >> 8 & 0xff00) | (length << 8 & 0xff0000) | length << 24;
    }
    else if (chance(50))
        g.getMemory<x86::reg8>(kSavesBig) = 0xc0;
    // A name to look for: one of the entries', its case changed now and then;
    // or another.
    std::string wanted = !taken.empty() && chance(70) ? taken[below(x86::reg32(taken.size()))] : names[below(14)];
    for (char& c : wanted)
        if (chance(30) && (c | 0x20) >= 'a' && (c | 0x20) <= 'z')
            c = char(c ^ 0x20);
    for (x86::reg32 i = 0; i < wanted.size(); ++i)
        g.getMemory<x86::reg8>(name + i) = x86::reg8(wanted[i]);
    g.getMemory<x86::reg8>(name + x86::reg32(wanted.size())) = 0;
    return kind;
}

/* The saves' world: the replay a file loaded is (the .ccm trackdata.inc's
 * loadFileQuietly gives in its cases, marked as theirs), what loadFile gives,
 * the records, config.dat's settings and what they are read with, the
 * command line's flags, the race. */
void makeLoadSavesWorld(CheckApp& g)
{
    g.word(0xd0fffc) = x86::reg32(g_seed);
    for (x86::reg32 at = kSavesReplay - 0x10; at < kSavesReplay + 0x2000; at += 4)
        g.word(at) = chance(30) ? 0 : random32();
    g.word(kSavesReplay - 0xc) = chance(80) ? below(0x400) : random32() % 0x30000;
    g.word(kSavesReplay + 0x1878) = chance(85) ? below(4) : chance(50) ? 4 + below(4) : random32();
    g.word(kSavesReplay + 0x196c) = chance(85) ? below(9) : chance(50) ? x86::reg32(-1) : 9 + below(4);
    g.word(kSavesReplay + 0x1974) = below(9);
    g.word(kSavesReplay + 0x19b4) = chance(90) ? below(8) : below(0x40);
    for (x86::reg32 at = kSavesLoaded; at < kSavesLoaded + 0x100; at += 4)
        g.word(at) = chance(60) ? 0x10 : random32();
    g.word(0x559408) = chance(60) ? 0 : 0x1f0000 + 0x400 * below(8);
    g.getMemory<x86::reg8>(0x55eb2e) = x86::reg8(chance(50) ? 0 : chance(50) ? 0x20 : chance(50) ? 0x10 : random32());
    g.word(0x6fd3a0) = chance(80) ? 0 : 1 + below(3);
    g.word(0x55e88c) = chance(80) ? 0 : 1;
    g.word(0x6fd3b0) = chance(70) ? below(2) : below(5);
    g.word(0x6fd50c) = chance(50) ? 0 : 1;
    g.word(0x6fd3ac) = chance(85) ? 1 : 0;
    g.word(0x7cda58) = chance(50) ? 0 : random32();
    g.word(0x55d000) = chance(50) ? 1 : chance(50) ? 0 : random32();
    g.word(0x7a5f40) = chance(50) ? 0 : 1;
    g.word(0x6fbb40) = chance(70) ? 0x20f0 : random32() % 0x3000;
    g.word(0x7a3a58) = chance(50) ? 0 : random32();
    g.word(0x6fbc10) = chance(70) ? g.word(0x7a3a58) : random32();
    // Words of the settings and of the replay's head no code here sets: as
    // anything left them.
    for (const x86::reg32 at : {0x6fd3a4u, 0x6fdc28u, 0x6fdc2cu, 0x7a5f38u, 0x7a5f3cu, 0x7d3684u, 0x55fe40u})
        g.word(at) = random32();
    g.word(0x55b038) = chance(50) ? 0 : 1;
    // The camera's views chosen: of the table mostly, past it now and then.
    for (x86::reg32 i = 0; i < 4; ++i)
        g.word(0x6fbba8 + 4 * i) = chance(90) ? below(8) : below(0x8000);
    // The language's name.
    static const char* const languages[] = {"english", "german", "french", "spanish", "italian", "swedish",
                                            "English", "dutch"};
    aSavesName(g, 0x7a2634, languages, 8);
    static const char* const folders[] = {"", "c:\\nfs3\\", "saves\\", "replays\\"};
    for (const x86::reg32 at : {0x7a2f34u, 0x7a2fb4u, 0x7a3034u})
        aSavesName(g, at, folders, 4);
}

bool makeLoadSavesArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "readReplayInfo", "saveGhostAs", "loadKeptReplay", "saveReplayAs", "listReplays", "loadRecords",
        "saveRecords", "languageStub", "setLanguage", "setCameraViews", "defaultConfig", "saveConfig",
        "fileWriteTime", "fileOlder", "loadConfig", "saveRaceReplay", "readRaceReplay", "typeOfBigFile",
        "bigHeaderLength", "findBigEntry", "locateBigEntry", "savefileadr", "savefileadrQuietly", "fixedSine",
        "fixedCosine"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    static const char* const files[] = {"replay.rp3", "ghost.gst", "mine.rp3", "config.dat", "c:\\a\\b.rp3"};
    g_mostCalls = 1000;
    if (name == "savefileadr" || name == "savefileadrQuietly")
    {
        // EA's files' world (ea_files.inc), what savefileadr's open and write
        // go through.
        makeEaFilesWorld(g);
        g_mostCalls = 400;
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = kEaFileMemory + below(0x800);
        cpu.ebx = chance(85) ? below(0x400) : chance(50) ? 0 : 0x400 + below(0x200);
        return true;
    }
    makeLoadSavesWorld(g);
    if (name == "readReplayInfo" || name == "saveGhostAs" || name == "loadKeptReplay" || name == "saveReplayAs")
    {
        cpu.eax = aSavesName(g, kSavesNames, files, 5);
        if (chance(30))
            g.getMemory<x86::reg8>(cpu.eax) = 0;
    }
    else if (name == "listReplays")
    {
        static const char* const extensions[] = {"rp3", "gst", "RP3"};
        cpu.eax = kSavesList;
        for (x86::reg32 at = kSavesList; at < kSavesList + 0x80; at += 4)
            g.word(at) = random32();
        g.word(kSavesList + 0x4c) = chance(70) ? below(0x10) : chance(50) ? x86::reg32(-1 - x86::sreg32(below(4)))
                                                                           : random32();
        g.word(kSavesList + 0x70) = chance(50) ? 0 : 0x1f0000 + 0x400 * below(8);
        cpu.edx = aSavesName(g, kSavesNames, extensions, 3);
        // Byte 0 of the memory a name with no '.' clears: as the cases before left it.
        g.getMemory<x86::reg8>(0) = x86::reg8(random32());
        // "*.%s" printed into the frame's 0xc bytes below where the code
        // keeps the names' end ("*.rp3" in the game): sprintf's stand-in
        // prints up to 0x4f, which would write over it, the list's record's
        // address and the count.  The case's seed picked for one that
        // prints fewer than 0xc -- the calls before it as they go:
        // the block freed or not, the list's first entry or not, the
        // directory had (no Error) and set.
        const bool freed = g.word(kSavesList + 0x70) != 0;
        const bool first = std::strcmp(&g.getMemory<char>(cpu.edx), "rp3") == 0;
        for (;;)
        {
            const auto r = [&](x86::reg32 call, x86::reg32 id) {
                return x86::reg32(mix(g_seed ^ (std::uint64_t(call) << 32) ^ id));
            };
            x86::reg32 call = (freed ? 1 : 0) + 1 + (first ? 3 : 0) + 1;
            const bool fine = r(call + 1, standInFor(0x534500)) % 9 != 0 && r(call + 2, standInFor(0x5345d4)) % 5 != 0
                              && r(call + 3, 0x4df690) % 0x50 < 0xc;
            if (fine)
                break;
            g_seed = mix(g_seed + 1);
        }
        g.word(0xd0fffc) = x86::reg32(g_seed);
    }
    else if (name == "loadConfig" && (g.getMemory<x86::reg8>(0x55eb2e) & 0x20) == 0 && chance(40))
    {
        // config.dat as long as the settings: 0x4e28d0's stand-in (ea_support.inc)
        // gives so many bytes once in 0x20000 or so.  The case's seed picked
        // for it -- the open its fifth call (GetModuleFileNameA, sprintf,
        // the system's tasks, fileExists before it), the file found with a
        // handle.
        for (;;)
        {
            const auto r = [&](x86::reg32 id) { return x86::reg32(mix(g_seed ^ (std::uint64_t(5) << 32) ^ id)); };
            const x86::reg32 found = r(0x4e28d0), more = r(0x4e28d1);
            if (found % 5 != 0 && (found >> 3) % 9 != 0 && more % 6 != 0 && (more >> 4) % 8 != 0
                && (more >> 8) % 0x20000 == 0x20f0 - 1)
                break;
            g_seed = mix(g_seed + 1);
        }
        g.word(0xd0fffc) = x86::reg32(g_seed);
        // What the blocks allocated hold, which the read (a stand-in) leaves.
        for (x86::reg32 at = 0x1f0000; at < 0x1f2000; at += 4)
            g.word(at) = random32();
    }
    else if (name == "saveRaceReplay")
    {
        // 0x4cf040's stand-in keeps eax now and then: none on mostly.
        if (chance(75))
            cpu.eax = 0;
        // "replay.rp3" printed into the frame's 0x3c bytes: sprintf's stand-in
        // prints up to 0x4f, which would write over the registers the code
        // keeps above them.  The case's seed picked for one that prints fewer
        // than 0x3c -- sprintf the second call, after 0x4cf040's.
        while (x86::reg32(mix(g_seed ^ (std::uint64_t(2) << 32) ^ 0x4df690)) % 0x50 >= 0x3c)
            g_seed = mix(g_seed + 1);
        g.word(0xd0fffc) = x86::reg32(g_seed);
    }
    else if (name == "fileWriteTime")
    {
        cpu.eax = aSavesName(g, kSavesNames, files, 5);
        cpu.esi = kSavesOut + 4 * below(0x20);
    }
    else if (name == "fileOlder")
    {
        cpu.eax = aSavesName(g, kSavesNames, files, 5);
        cpu.edx = aSavesName(g, kSavesNames + 0x100, files, 5);
    }
    else if (name == "typeOfBigFile" || name == "bigHeaderLength" || name == "findBigEntry"
             || name == "locateBigEntry")
    {
        x86::reg32 count = 0;
        makeSavesBig(g, count, kSavesNames);
        g.word(0x56440c) = chance(50) ? 0 : 1;
        cpu.eax = chance(5) ? 0 : kSavesBig;
        // By its name, or by its index (of the entries mostly).
        cpu.edx = chance(65) ? kSavesNames : 0;
        cpu.ebx = chance(85) ? below(count + 2) : random32();
        cpu.ecx = chance(80) ? kSavesOut + 4 * below(0x20) : 0;
        stack[0] = chance(80) ? kSavesOut + 4 * below(0x20) : 0;
    }
    else if (name == "fixedSine" || name == "fixedCosine")
        cpu.eax = chance(70) ? random32() : chance(50) ? below(0x10000) : below(0x80) + 0x4000 * below(4) - 0x40;
    return true;
}
#endif
