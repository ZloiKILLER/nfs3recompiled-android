// game_checks.cpp's parts for EA's files: opened, read and written a block at a time, asynchronously (native_ea_files.cpp), included in each
// table with the macro naming the part.
#if defined(EA_WINDOWS)
// {from, to}, -- what the functions write, compared
// None of their own: the handles' table and the cases' buffers are in the
// harness's (0x61d000 to 0x620000), the globals on pages compared already
// (0x552000, 0x564000, 0x566000 to 0x568000, 0x9ef000), the pages Windows
// gives in the heap's slots.
#elif defined(EA_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kEaFilesSystemInfo,    // GetSystemInfo: a SYSTEM_INFO, its page 4K mostly, never 0
kEaFilesPhysical,      // the memory there is (0x5221f0): a few megabytes, little, or anything
kEaFilesDriveStrings,  // GetLogicalDriveStringsA: "X:\" of some drives if they fit, how long; now and then very long
kEaFilesVolume,        // GetVolumeInformationA: its longest name and its flags, or FALSE
kEaFilesUpper,         // CharUpperA: the string's letters as capitals
kEaFilesLookup,        // the archive's lookup: a file's place in it, or 0
kEaFilesView,          // MapViewOfFile: one of the cases' files' memory, or 0
kEaFilesSize,          // GetFileSize: a length, or -1
#elif defined(EA_EFFECT_CASES)
// case kSomething: ... break;
case kEaFilesSystemInfo:
{
    static const x86::reg32 pages[] = {0x1000, 0x1000, 0x1000, 0x2000, 0x10000, 0x200, 3};
    for (x86::reg32 i = 0; i < 0x24; i += 4)
        word(words[0] + i) = random(id + i);
    word(words[0] + 4) = pages[r % 7];
    break;
}
case kEaFilesPhysical:
    result = r % 4 == 0 ? (r >> 8) % 0x10000 : r % 4 == 1 ? r : 0x100000 * (1 + (r >> 8) % 64);
    break;
case kEaFilesDriveStrings:
{
    // (its room, the buffer): the drives' roots, an empty string after.
    x86::reg32 drives = random(id + 1) & random(id + 2) & 0x3ffffff;
    x86::reg32 length = 0;
    for (x86::reg32 d = 0; d < 26; ++d)
        length += (drives >> d & 1) * 4;
    if (r % 7 == 0)
        result = (r >> 4) % 2 ? 0xfff + (r >> 8) % 3 : 0x1000 + (r >> 8) % 0x2000;
    else if (words[0] > length)
    {
        x86::reg32 at = words[1];
        for (x86::reg32 d = 0; d < 26; ++d)
            if (drives >> d & 1)
            {
                getMemory<x86::reg8>(at) = x86::reg8('A' + d);
                getMemory<x86::reg8>(at + 1) = ':';
                getMemory<x86::reg8>(at + 2) = '\\';
                getMemory<x86::reg8>(at + 3) = 0;
                at += 4;
            }
        getMemory<x86::reg8>(at) = 0;
        result = length;
    }
    else
        result = length + 1;
    break;
}
case kEaFilesVolume:
    // (root, name, its room, &serial, &longest name, &flags, file system, its room)
    result = r % 6 != 0 ? 1 : 0;
    if (result != 0)
    {
        if (words[3] != 0)
            word(words[3]) = random(id + 3);
        if (words[4] != 0)
            word(words[4]) = r % 3 == 0 ? 12 : 255;
        if (words[5] != 0)
            word(words[5]) = (random(id + 5) & ~7u) | (r >> 8) % 8;
    }
    break;
case kEaFilesUpper:
    for (x86::reg32 i = 0; i < 0x400 && getMemory<x86::reg8>(words[0] + i) != 0; ++i)
    {
        const x86::reg8 c = getMemory<x86::reg8>(words[0] + i);
        if (c >= 'a' && c <= 'z')
            getMemory<x86::reg8>(words[0] + i) = x86::reg8(c - 0x20);
    }
    result = words[0];
    break;
case kEaFilesLookup:
    result = r % 4 == 0 ? 0 : r % 9 == 0 ? r : (r >> 8) % 0x400;
    break;
case kEaFilesView:
    result = r % 4 == 0 ? 0 : 0x61f000 + 0x100 * ((r >> 8) % 8);
    break;
case kEaFilesSize:
    result = r % 8 == 0 ? 0xffffffff : (r >> 8) % 0x800;
    break;
#elif defined(EA_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// The memory there is (GlobalMemoryStatus's total, the threads' code):
// touchMemoryPages asks it the first time.
{0x5221f0, "the memory there is", kWatcom, 0, 0, kEax, -1, 0, kEaFilesPhysical},
#elif defined(EA_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
// Windows' functions the files call that nothing else does.
{0x53449c, "CreateFileMappingA", kStdcall, 0, 6, kScratch, -1, 0, kEvent},
{0x5344c8, "FindClose", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
{0x534524, "GetFileSize", kStdcall, 0, 2, kScratch, -1, 0, kEaFilesSize},
{0x53453c, "GetLogicalDriveStringsA", kStdcall, 0, 2, kScratch, -1, 0, kEaFilesDriveStrings},
{0x534540, "GetLogicalDrives", kStdcall, 0, 0, kScratch, -1, 0, kNothing},
{0x534560, "GetSystemInfo", kStdcall, 0, 1, kScratch, -1, 0, kEaFilesSystemInfo},
{0x534578, "GetVolumeInformationA", kStdcall, 0, 8, kScratch, 4, 1, kEaFilesVolume},  // the root's first bytes
{0x534590, "MapViewOfFile", kStdcall, 0, 5, kScratch, -1, 0, kEaFilesView},
{0x534620, "UnmapViewOfFile", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
{0x5346f8, "CharUpperA", kStdcall, 0, 1, kScratch, 0 + 4, 4, kEaFilesUpper},  // the string's first bytes
// The archive's lookup, the game's (Watcom): the directory and the name, the name's first bytes logged.
{0x564588, "the archive's lookup", kWatcom, 2, 0, kEax, 1, 4, kEaFilesLookup},
#elif defined(EA_PROMISES)
// {"ea::name", watcom, result, arguments, fpu, heap, liveOnly},
// The registers a function is handed its arguments in -- eax, edx, ebx, ecx,
// as many as it takes -- are its own, by Watcom's convention (the code saves
// none of them: its callers count on none after); the rest compared.
// volumeRoot and closeAllHandles give nothing back (void: eax what their last
// call left, read by no caller).  touchMemoryPages leaves ebx and edx as its
// loops do, compared.
{"ea::initHandlesDefault", true, true, 0},
{"ea::openHandle", true, true, 4},
{"ea::openHandleErrorsAs", true, true, 4},
{"ea::openHandleToWrite", true, true, 4},
{"ea::openMapHandle", true, true, 4},
{"ea::closeHandle", true, true, 0},
{"ea::readHandle", true, true, 3},
{"ea::writeHandle", true, true, 3},
{"ea::seekHandle", true, true, 2},
{"ea::isSlash", true, true, 0},
{"ea::notSlash", true, true, 0},
{"ea::volumeRoot", true, false, 2},
{"ea::fullPathName", true, true, 3},
{"ea::logicalDrives", true, true, 0},
// edx, an argument, left as the game's _stricmp leaves its own (0x4e9ec0: the
// game's code moves it, native watcom::stricmp keeps it) where a drive matches.
{"ea::driveOfPath", true, true, 2},
{"ea::driveLetter", true, true, 0},
{"ea::diskFreeBytes", true, true, 0},
{"ea::driveNumber", true, true, 0},
{"ea::setDirectory", true, true, 0},
{"ea::getDirectory", true, true, 0},
{"ea::handlesReady", true, true, 0},
{"ea::initHandles", true, true, 0},
{"ea::closeAllHandles", true, false, 0},
{"ea::takeHandleSlot", true, true, 0},
{"ea::deleteFile", true, true, 0},
{"ea::openFileHandle", true, true, 4},
{"ea::handleInfo", true, true, 4},
{"ea::readHandleBytes", true, true, 3},
{"ea::writeHandleBytes", true, true, 3},
{"ea::seekHandleTo", true, true, 2},
{"ea::handlePosition", true, true, 0},
{"ea::closeHandleSlot", true, true, 0},
{"ea::lockHandleDrive", true, true, 0},
{"ea::unlockHandleDrive", true, true, 0},
{"ea::openMemoryHandle", true, true, 4},
{"ea::handleMemory", true, true, 0},
{"ea::checkSurfaceUnlocked", true, true, 0},
{"ea::fileReport", true, true, 0},
{"ea::virtualAllocPages", true, true, 0},
{"ea::virtualFreePages", true, true, 0},
{"ea::touchMemoryPages", true, true, 0},
{"ea::openBlockHandle", true, true, 4},
{"ea::blockHandleFile", true, true, 0},
{"ea::closeBlockHandle", true, true, 0},
{"ea::readBlockHandle", true, true, 3},
{"ea::seekBlockHandle", true, true, 2},
#elif defined(EA_ARGUMENTS)
/* Where the files' cases keep their own: the handles' table (0x80 slots at
 * most), paths, what is written out, the archive's directory, buffers read
 * and written, the files' memory (mapped views, buffers taken as files). */
constexpr x86::reg32 kEaFileTable = 0x61d000;
constexpr x86::reg32 kEaFilePaths = 0x61e000;      // 4 of 0x200
constexpr x86::reg32 kEaFileOut = 0x61e800;        // what is written out: 0x400
constexpr x86::reg32 kEaFileWords = 0x61ec00;      // pointers' targets: 0x100
constexpr x86::reg32 kEaFileDirectory = 0x61ed00;  // 0x300
constexpr x86::reg32 kEaFileMemory = 0x61f000;     // 0x1000

/* A path: a drive's, a server's share's, slashes either way, nothing, a |,
 * now and then longer than MAX_PATH -- into `at`, 0x1ff at most. */
x86::reg32 anEaPath(CheckApp& g, x86::reg32 at)
{
    static const char* const starts[] = {"C:\\", "d:/", "\\\\server\\share\\", "//host/vol/", "\\\\x", "\\",
                                         "", "a|b\\", "Z:", "data\\"};
    std::string path = starts[below(10)];
    const x86::reg32 parts = below(chance(5) ? 40 : 5);
    for (x86::reg32 i = 0; i < parts && path.size() < 0x1f0; ++i)
    {
        const x86::reg32 n = 1 + below(12);
        for (x86::reg32 j = 0; j < n; ++j)
            path += chance(3) ? "|/\\:"[below(4)] : char('a' + below(26));
        if (i + 1 < parts)
            path += chance(70) ? '\\' : '/';
    }
    if (path.size() > 0x1f0)
        path.resize(0x1f0);
    for (x86::reg32 i = 0; i < path.size(); ++i)
        g.getMemory<x86::reg8>(at + i) = x86::reg8(path[i]);
    g.getMemory<x86::reg8>(at + x86::reg32(path.size())) = 0;
    return at;
}

/* A handle: of the table's slots mostly (an index's bits flipped), past
 * them, before them, or anything. */
x86::reg32 anEaHandle(CheckApp& g)
{
    const x86::reg32 count = g.word(0x56459c);
    const x86::reg32 index = chance(85) && x86::sreg32(count) > 0 && count <= 0x80 ? below(count)
                             : chance(50) ? count + below(3) : chance(50) ? x86::reg32(-1 - x86::sreg32(below(3)))
                             : random32();
    return ~index;
}

/* The files' world: the table and its slots, the locks, the archive, the
 * reports' level and way out, the page's size and the memory there is. */
void makeEaFilesWorld(CheckApp& g)
{
    for (x86::reg32 at = kEaFilePaths; at < kEaFileMemory + 0x1000; at += 4)
        g.word(at) = chance(30) ? 0 : random32();
    for (x86::reg32 at = kEaFileMemory; at < kEaFileMemory + 0x1000; at += 4)
        g.word(at) = random32();
    g.word(0x5645a0) = chance(90) ? kEaFileTable : 0;
    g.word(0x56459c) = chance(70) ? 0x40 : chance(80) ? below(0x81) : chance(50) ? 0 : 0x80000000 | random32();
    for (x86::reg32 i = 0; i < 0x80; ++i)
    {
        const x86::reg32 slot = kEaFileTable + 0x20 * i;
        g.getMemory<x86::reg8>(slot) = x86::reg8(chance(60) ? 1 : chance(80) ? 0 : random32());
        g.getMemory<x86::reg8>(slot + 1) = x86::reg8(chance(90) ? below(27) : below(0x40));
        g.getMemory<x86::reg8>(slot + 2) = x86::reg8(chance(70) ? 0 : 1);
        g.getMemory<x86::reg8>(slot + 3) = x86::reg8(random32());
        g.word(slot + 4) = chance(85) ? 0x7000 + below(0x100) * 4 : 0xffffffff;
        g.word(slot + 8) = random32() & (chance(80) ? 0xfff : 0xffffffff);
        g.word(slot + 0xc) = chance(60) ? 0x200 << below(3) : 1 + below(0x400);
        g.word(slot + 0x10) = chance(40) ? 0xffffffff : chance(50) ? 0 : 0x7400 + below(0x100) * 4;
        const x86::reg32 size = chance(90) ? below(0x800) : below(0x1000);
        g.word(slot + 0x14) = chance(50) ? 0 : kEaFileMemory + below(0x800);
        g.word(slot + 0x18) = chance(85) ? below(size + 1) : chance(50) ? size + below(0x20) : x86::reg32(-1 - x86::sreg32(below(0x20)));
        g.word(slot + 0x1c) = size;
    }
    for (x86::reg32 at = 0x9ef760; at < 0x9ef860; at += 4)
        g.word(at) = chance(30) ? 0 : 0x7800 + below(0x100) * 4;
    g.word(0x564598) = chance(20) ? 0 : 0x7c00 + below(0x100) * 4;
    g.word(0x56440c) = chance(50) ? 0 : chance(90) ? 1 : random32();
    g.word(0x564364) = chance(50) ? 0 : 1;
    // The archive: its lookup or none, its name or none, its directory.
    g.word(0x564588) = chance(60) ? standInFor(0x564588) : 0;
    if (chance(70))
        g.text(0x564504, 1 + below(0x40), random32());
    else
        g.getMemory<x86::reg8>(0x564504) = 0;
    g.word(0x564584) = kEaFileDirectory;
    g.word(0x564590) = chance(85) ? below(0x300) : 0;
    g.word(0x56458c) = kEaFileWords + 4 * below(0x20);
    // The reports: a level, a way out.
    g.word(0x564594) = chance(80) ? below(4) : random32();
    g.word(0x56789c) = chance(90) ? below(4) : random32();
    // Windows' page, known or not; the memory there is.
    static const x86::reg32 pages[] = {0x1000, 0x1000, 0x1000, 0x10000, 0x200, 7};
    g.word(0x5678a0) = chance(30) ? 0 : pages[below(6)];
    g.word(0x5678a4) = chance(40) ? 0 : chance(80) ? 0x100000 * (1 + below(64)) : below(0x10000);
}

bool makeEaFilesArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "initHandlesDefault", "openHandle", "openHandleErrorsAs", "openHandleToWrite", "openMapHandle",
        "closeHandle", "readHandle", "writeHandle", "seekHandle", "isSlash", "notSlash", "volumeRoot",
        "fullPathName", "logicalDrives", "driveOfPath", "driveLetter", "diskFreeBytes", "driveNumber",
        "setDirectory", "getDirectory", "handlesReady", "initHandles", "closeAllHandles", "takeHandleSlot",
        "deleteFile", "openFileHandle", "handleInfo", "readHandleBytes", "writeHandleBytes", "seekHandleTo",
        "handlePosition", "closeHandleSlot", "lockHandleDrive", "unlockHandleDrive", "openMemoryHandle",
        "handleMemory", "checkSurfaceUnlocked", "fileReport", "virtualAllocPages", "virtualFreePages",
        "touchMemoryPages", "openBlockHandle", "blockHandleFile", "closeBlockHandle", "readBlockHandle",
        "seekBlockHandle"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeEaFilesWorld(g);
    // A loop of reads or writes a block at a time: bounded, whatever the stand-ins say.
    g_mostCalls = 400;
    // A pointer for what is written out, or none.
    const auto aWord = [] { return kEaFileWords + 4 * below(0x20); };
    const auto aWordOrNone = [&] { return chance(25) ? 0 : aWord(); };
    const auto aCount = [] { return chance(85) ? below(0x400) : chance(50) ? 0 : 0x400 + below(0x200); };
    const auto aPosition = [] {
        return chance(80) ? below(0x800) : chance(50) ? x86::reg32(-1 - x86::sreg32(below(0x40))) : random32();
    };
    const x86::reg32 esp = cpu.esp;
    if (name == "openHandle" || name == "openHandleErrorsAs" || name == "openHandleToWrite"
        || name == "openMapHandle" || name == "openBlockHandle")
    {
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = aWord();
        cpu.ebx = aWordOrNone();
        cpu.ecx = aWordOrNone();
        stack[0] = name == "openBlockHandle" ? aWord() : chance(50) ? 0 : 1;
    }
    else if (name == "closeHandle" || name == "closeHandleSlot" || name == "closeBlockHandle"
             || name == "handlePosition" || name == "handleMemory" || name == "lockHandleDrive"
             || name == "unlockHandleDrive")
        cpu.eax = anEaHandle(g);
    else if (name == "readHandle" || name == "readHandleBytes" || name == "writeHandle"
             || name == "writeHandleBytes")
    {
        cpu.eax = anEaHandle(g);
        cpu.edx = kEaFileOut;
        cpu.ebx = aCount();
    }
    else if (name == "readBlockHandle")
    {
        cpu.eax = anEaHandle(g);
        cpu.edx = kEaFileOut;
        cpu.ebx = below(4);
        // Its blocks small enough for the buffer.
        for (x86::reg32 i = 0; i < 0x80; ++i)
            g.word(kEaFileTable + 0x20 * i + 0xc) = 1 + below(0x100);
    }
    else if (name == "seekHandle" || name == "seekHandleTo" || name == "seekBlockHandle")
    {
        cpu.eax = anEaHandle(g);
        cpu.edx = aPosition();
    }
    else if (name == "isSlash" || name == "notSlash")
    {
        cpu.eax = random32();
        if (chance(70))
            cpu.al = x86::reg8("\\/a:"[below(4)]);
    }
    else if (name == "volumeRoot")
    {
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = kEaFileOut;
    }
    else if (name == "fullPathName")
    {
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = kEaFileOut;
        cpu.ebx = chance(80) ? 0x100 + below(0x100) : below(0x20);
        // What GetFullPathName leaves where it does not write it: the file's part.
        g.word(esp - 0x14) = kEaFileOut + below(0x100);
    }
    else if (name == "driveOfPath")
    {
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = kEaFileOut;
    }
    else if (name == "driveLetter" || name == "driveNumber")
        cpu.eax = chance(10) ? 0 : anEaPath(g, kEaFilePaths);
    else if (name == "diskFreeBytes" || name == "setDirectory" || name == "deleteFile")
        cpu.eax = anEaPath(g, kEaFilePaths);
    else if (name == "getDirectory")
        cpu.eax = kEaFileOut;
    else if (name == "initHandles")
        cpu.eax = chance(30) ? 0 : chance(80) ? 1 + below(0x80) : below(0x2000);
    else if (name == "openFileHandle")
    {
        cpu.eax = anEaPath(g, kEaFilePaths);
        cpu.edx = chance(60) ? (chance(50) ? 0x205 : chance(50) ? 0x805 : 0x232) : random32() & 0xfff;
        cpu.ebx = chance(40) ? 0 : chance(70) ? below(0x10000) : random32();
        cpu.ecx = aWord();
    }
    else if (name == "handleInfo")
    {
        cpu.eax = anEaHandle(g);
        cpu.edx = aWordOrNone();
        cpu.ebx = aWordOrNone();
        cpu.ecx = aWordOrNone();
        stack[0] = aWordOrNone();
    }
    else if (name == "openMemoryHandle")
    {
        cpu.eax = kEaFileMemory + below(0x800);
        cpu.edx = below(0x800);
        cpu.ebx = chance(90) ? below(27) : below(0x40);
        cpu.ecx = chance(30) ? 0 : 1 + below(0x400);
        stack[0] = aWord();
    }
    else if (name == "checkSurfaceUnlocked")
        cpu.eax = anEaPath(g, kEaFilePaths);
    else if (name == "fileReport")
    {
        stack[0] = chance(60) ? 0x564594 : chance(50) ? 0 : aWord();
        stack[1] = chance(80) ? below(4) : random32();
        stack[2] = chance(50) ? 0x54a36c : aString();
    }
    else if (name == "virtualAllocPages")
    {
        cpu.eax = aWord();
        g.word(cpu.eax) = chance(80) ? below(0x20000) : random32() & 0x7fffffff;
    }
    else if (name == "virtualFreePages")
        cpu.eax = chance(50) ? 0x100000 + 0x20000 * below(8) : random32();
    else if (name == "touchMemoryPages")
    {
        cpu.eax = random32();
        cpu.edx = chance(80) ? below(0x400000) : x86::reg32(-1 - x86::sreg32(below(0x1000)));
        // The memory there is, to the byte now and then.
        if (chance(20) && g.word(0x5678a4) != 0)
            cpu.edx = g.word(0x5678a4) - 1 + below(3);
        // No page known: the length none (it would be divided by 0).
        if (g.word(0x5678a0) == 0)
            cpu.edx = x86::reg32(-1 - x86::sreg32(below(0x1000)));
    }
    return true;
}
#endif
