// game_checks.cpp's parts for EA's trace and log, files loaded whole into memory, and .viv archives (native_ea_support.cpp), included in each
// table with the macro naming the part.
#if defined(EA_WINDOWS)
// The trace's, the loads' and EA's random numbers' globals (0x5644c4 to
// 0x5644f4, 0x56440c, 0x5643f8; 0x567860, 0x567868), the error's where
// (0x552190) and the cases' own (0x61d000 to 0x620000) are in windows
// already.
#elif defined(EA_EFFECTS)
    kSupportOpened,  // the game's file opened (0x4e28d0): its handle to [edx], where to [ebx], its size to [ecx]; or none
#elif defined(EA_EFFECT_CASES)
        case kSupportOpened:
        {
            // Found mostly -- with no handle or an empty file now and then --
            // or not, a handle left over now and then.  The size any, odd
            // or negative, now and then.
            const bool found = r % 5 != 0;
            const x86::reg32 handle = found ? ((r >> 3) % 9 == 0 ? 0 : 0x5000 + (r >> 8) % 0x100 * 4)
                                            : ((r >> 3) % 7 == 0 ? 0x5400 : 0);
            const x86::reg32 more = random(id + 1);
            const x86::reg32 size = (more % 6 == 0) ? 0 : (more >> 4) % 8 == 0 ? random(id + 2) : 1 + (more >> 8) % 0x20000;
            if (words[1] < kMemory - 4)
                word(words[1]) = handle;
            if (words[2] != 0 && words[2] < kMemory - 4)
                word(words[2]) = found ? random(id + 3) % 0x1000000 : 0;
            if (words[3] != 0 && words[3] < kMemory - 4)
                word(words[3]) = found ? size : 0;
            result = found ? 1 : 0;
            break;
        }
#elif defined(EA_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The files group's (native_ea_files.cpp, not stood in before): a file of the
// game's opened by name (its first 16 bytes logged), closed, read.
{0x4e28d0, "a game file opened", kWatcom, 4, 0, kEax | kEdx | kEbx | kEcx, 0, 4, kSupportOpened},
// The close's one register: ea::closeChecked reads only eax (native_ea_files.h), and the natives
// call it as C++ (guest::direct), edx their own.
{0x4e2da0, "a game file closed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4e2de0, "a game file read", kWatcom, 3, 0, kEax | kEdx | kEbx, -1, 0, kNothing},
// The sound stream's top-up around a load (native_snd_stream.cpp; no tested
// code calls them but the load): kept to their code -- every register but
// eax kept, the end's not even that; the registers they are handed logged.
{0x4ff7c0, "a top-up before a load", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4ff7e0, "a top-up's end", kWatcom, 0, 0, 0, -1, 0, kNothing},
{0x4ff820, "the load's block size", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x4ff830, "the load's flags", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The runtime's fopen (its name's first 8 bytes logged) and fclose, which the
// archives' open and close call (only graphics::gameMain else, by address
// too): a FILE or none.  The runtime's own are tested on their own.
{0x4edfe8, "fopen", kWatcom, 2, 0, kEax, 0, 2, kLoad},
{0x4ee100, "fclose", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// This file's own that the other natives call by address (the sound's banks
// the archives', the game's code the trace and the loads): as their code
// leaves the registers -- before they were native, the generated code's
// stand-ins (decompiled_standins.inc) were these.  Each tested on its own.
{0x42d510, "gameTrace", kWatcom, 0, 0, 0, -1, 0, kNothing},
{0x43b830, "openViv", kWatcom, 2, 0, kEax | kEdx, -1, 0, kNothing},
{0x43b9b0, "closeViv", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x43b9d0, "findInViv", kWatcom, 3, 0, kEax | kEcx | kEdx, -1, 0, kNothing},
{0x4e0e60, "fileExists", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4e0fe0, "loadFileQuietly", kWatcom, 2, 0, kEax, -1, 0, kNothing},
#elif defined(EA_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// The load's hook: the block, the name, the flags; another block, or none.
{0x567868, "the load's hook", kWatcom, 3, 0, kEax, -1, 0, kFilter},
#elif defined(EA_PROMISES)
// {"ea::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared -- but edx where an archive's entry is looked for:
// the game's _stricmp (0x4e9ec0) leaves the difference in edx, native
// watcom::stricmp its argument, and an entry not found leaves edx the last
// _stricmp's.  As the port runs, both sides call the native one.
{"ea::gameTrace", true, true, 0},
{"ea::openViv", true, true, 0},
{"ea::closeViv", true, true, 0},
{"ea::findInViv", true, true, 2},
{"ea::findVivEntry", true, true, 2},
{"ea::monoPrintf", true, true, 0},
{"ea::monoPrintfAt", true, true, 0},
{"ea::monoWrite", true, true, 0},
{"ea::monoScroll", true, true, 0},
{"ea::monoClear", true, true, 0},
{"ea::monoSetTop", true, true, 0},
{"ea::monoSetAttribute", true, true, 0},
{"ea::monoSetTab", true, true, 0},
{"ea::monoPage", true, true, 0},
{"ea::monoNoCursor", true, true, 0},
{"ea::seedRandom", true, true, 0},
{"ea::fileExists", true, true, 0},
{"ea::loadFile", true, true, 0},
{"ea::loadFileQuietly", true, true, 0},
#elif defined(EA_ARGUMENTS)
/* Where the support's cases keep their own: an archive's header, the
 * monitor's page, what is written out, names, a text written. */
constexpr x86::reg32 kSupportHeader = 0x61d000, kSupportPage = 0x61e000, kSupportOut = 0x61f000;
constexpr x86::reg32 kSupportNames = 0x61f400, kSupportText = 0x61fc00;

/* A file's name of the archives' sort, in any case now and then. */
std::string aVivName()
{
    static const char* const names[] = {"car.fce", "car.bnk", "carp.qfs", "dash.qfs", "ocar.fce",
                                        "fedata.cfg", "car00.tga", "a", "", "ctrack.frd"};
    std::string name = names[below(10)];
    if (chance(20))
        for (char& c : name)
            if (c >= 'a' && c <= 'z' && chance(50))
                c = char(c - 0x20);
    if (chance(5))
        name = std::string(0x18 + below(0x10), char('a' + below(26)));
    return name;
}

void putText(CheckApp& g, x86::reg32 at, const std::string& text)
{
    for (size_t i = 0; i <= text.size(); ++i)
        g.getMemory<x86::reg8>(at + x86::reg32(i)) = i < text.size() ? x86::reg8(text[i]) : 0;
}

/* The support's world: the trace's level and lock, the monitor -- its page
 * one of the cases', the cursor on it mostly, past it or before it now and
 * then, the window's top, the attribute, the tab -- and the page's cells;
 * the loads' complaint, hook and what the block size is handed; an archive's
 * header, of a few entries, its count now and then more than there are. */
void makeSupportWorld(CheckApp& g)
{
    g.word(0x5644c8) = chance(80) ? 2 + below(3) : chance(70) ? below(2) : random32();
    g.word(0x5644c4) = chance(25) ? 0 : 0x630000 + below(0x100) * 0x10;
    g.word(0x5644d0) = kSupportPage;
    g.word(0x5644d4) = chance(80)   ? 2 * below(0x7d0)
                       : chance(60) ? 0xfa0 + below(0x40)
                       : chance(30) ? below(0x1000)
                                    : x86::reg32(-x86::sreg32(below(0x400)));
    g.getMemory<x86::reg8>(0x5644d8) = x86::reg8(chance(85) ? below(0x19) : below(0x20));
    g.getMemory<x86::reg8>(0x5644d9) = x86::reg8(random32());
    g.getMemory<x86::reg8>(0x5644da) = x86::reg8(chance(70) ? 2 + below(8) : chance(50) ? below(2) : random32());
    for (x86::reg32 at = kSupportPage; at < kSupportPage + 0x1000; at += 4)
        g.word(at) = random32();
    g.word(0x56440c) = chance(50) ? 0 : chance(80) ? 1 : random32();
    g.word(0x567868) = chance(50) ? standInFor(0x567868) : 0;
    g.word(0x567860) = random32();
    for (x86::reg32 at = kSupportOut; at < kSupportOut + 0x400; at += 4)
        g.word(at) = random32();
    // The blocks the allocation's stand-in hands out, and past them as far as
    // an archive's header reaches: what was there before.
    for (x86::reg32 at = 0x1f0000; at < 0x200000; at += 4)
        g.word(at) = random32();
    // The header: the archive's four words, then the entries.
    for (x86::reg32 at = kSupportHeader; at < kSupportHeader + 0x800; at += 4)
        g.word(at) = 0;
    const x86::reg32 count = chance(10) ? 0 : 1 + below(12);
    x86::reg32 at = kSupportHeader + 0x10;
    for (x86::reg32 i = 0; i < count; ++i)
    {
        g.word(at) = random32();
        g.word(at + 4) = random32();
        const std::string name = aVivName();
        putText(g, at + 8, name);
        at += 8 + x86::reg32(name.size()) + 1;
    }
    g.word(kSupportHeader) = random32();
    g.word(kSupportHeader + 4) = random32();
    g.word(kSupportHeader + 8) = chance(85) ? count : count + below(5);
    g.word(kSupportHeader + 0xc) = at - kSupportHeader;
}

/* A name looked for: an entry's of the header mostly (another case of it now
 * and then), or another; with a path before it, of backslashes, now and then. */
x86::reg32 aWantedName(CheckApp& g)
{
    std::string name;
    const x86::reg32 count = std::min<x86::reg32>(g.word(kSupportHeader + 8), 12);
    if (count != 0 && chance(60))
    {
        x86::reg32 at = kSupportHeader + 0x10;
        for (x86::reg32 i = below(count); i != 0; --i)
            at += 8 + x86::reg32(std::strlen(&g.getMemory<char>(at + 8))) + 1;
        name = &g.getMemory<char>(at + 8);
        if (chance(20))
            for (char& c : name)
                if (c >= 'a' && c <= 'z')
                    c = char(c - 0x20);
    }
    else
        name = aVivName();
    if (chance(40))
        name = (chance(50) ? "gamedata\\carmodel\\" : "\\") + name;
    else if (chance(5))
        name += "\\";
    const x86::reg32 at = kSupportNames + 0x80 * below(8);
    putText(g, at, name);
    return at;
}

/* A text for the monitor: printable mostly, the control bytes it knows, the
 * attribute's escape, any byte now and then. */
x86::reg32 aMonoText(CheckApp& g)
{
    static const x86::reg8 controls[] = {8, 9, 0xa, 0xb, 0xc, 0xe, 0xe, 0x1b, 7, 0xd};
    const x86::reg32 length = chance(10) ? 0 : below(chance(80) ? 0x40 : 0x200);
    for (x86::reg32 i = 0; i < length; ++i)
        g.getMemory<x86::reg8>(kSupportText + i) =
            x86::reg8(chance(70) ? 0x20 + below(0x5f) : chance(85) ? controls[below(10)] : 1 + below(0xff));
    g.getMemory<x86::reg8>(kSupportText + length) = 0;
    return kSupportText;
}

bool makeEaSupportArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "gameTrace", "openViv", "closeViv", "findInViv", "findVivEntry", "monoPrintf", "monoPrintfAt",
        "monoWrite", "monoScroll", "monoClear", "monoSetTop", "monoSetAttribute", "monoSetTab", "monoPage",
        "monoNoCursor", "seedRandom", "fileExists", "loadFile", "loadFileQuietly"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeSupportWorld(g);
    // A format of the trace's, its conversions' words after it.
    static const char* const formats[] = {"%s(%d)\n", "Using custom color carindex: %d\n", "%d %d %s", "no format",
                                          ""};
    const x86::reg32 format = kSupportNames + 0x400;
    putText(g, format, formats[below(5)]);
    if (name == "openViv")
    {
        cpu.eax = aWantedName(g);
        cpu.edx = kSupportOut + 4 * below(8);
        // The header's size fread leaves in the frame's local (fread's stand-in
        // reads nothing): big-endian, a few K mostly; now and then one that
        // wraps with the 0x30 added, so that the block's first words are not
        // all cleared before they are byte-swapped.
        const x86::reg32 size = chance(85) ? below(0x3000) : chance(60) ? below(0xc000) : 0xffffffd0 + below(0x10);
        g.word(cpu.esp - 0x18) = size >> 24 | (size >> 8 & 0xff00) | (size << 8 & 0xff0000) | size << 24;
    }
    else if (name == "closeViv")
    {
        cpu.eax = chance(80) ? kSupportHeader : random32();
        cpu.edx = kSupportOut + 4 * below(8);
    }
    else if (name == "findInViv" || name == "findVivEntry")
    {
        cpu.eax = kSupportHeader;
        cpu.edx = aWantedName(g);
        cpu.ebx = kSupportOut + 4 * below(8);
        cpu.ecx = chance(90) ? kSupportOut + 0x40 + 4 * below(8) : cpu.ebx;
    }
    else if (name == "monoPrintf")
        stack[0] = format;
    else if (name == "monoPrintfAt")
    {
        stack[0] = chance(80) ? below(0x50) : chance(50) ? 0x50 + below(4) : random32();
        stack[1] = chance(80) ? below(0x19) : chance(50) ? 0x19 + below(4) : random32();
        stack[2] = format;
    }
    else if (name == "monoWrite")
        cpu.eax = aMonoText(g);
    else if (name == "monoSetTop")
        cpu.eax = chance(80) ? below(0x1c) : chance(50) ? x86::reg32(-1 - x86::sreg32(below(4))) : random32();
    else if (name == "monoSetTab" || name == "monoPage")
        cpu.eax = chance(70) ? below(10) : random32();
    else if (name == "monoNoCursor")
    {
        cpu.eax = kSupportOut + below(0x40);
        cpu.edx = kSupportOut + below(0x40);
    }
    else if (name == "fileExists" || name == "loadFile" || name == "loadFileQuietly")
        cpu.eax = aWantedName(g);
    return true;
}
#endif
