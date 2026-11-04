// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
// The stand-ins the misc group's functions need that kSpecs does not hold, added to it the first time.
// (frontend_checks.cpp has no place for a group's own specs: const_cast until it has.)
static const bool miscSpecsAdded = [] {
    auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
    const auto add = [&](x86::reg32 address, Spec spec) { specs.emplace(address, spec); };
    add(0x4c9860, { 0, 0, 0, 0, Kind::Plain });                       // descriptionService
    add(0x4c99d0, { 0, 0, 0, 0, Kind::Plain });                       // spokenStop
    add(0x4961e0, { 0, 0, 0, 0, Kind::Plain });                       // resetAttractTimer
    add(0x4475a0, { EAX | EDX, 0, 0, EDX, Kind::Plain });             // listReplays
    add(0x435310, { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Plain });   // readMouse
    add(0x44f8d0, { EAX | EDX, 0, 0, 0, Kind::Bool });
    add(0x447a30, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4ee4f8, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX, Kind::Plain });  // qsort: the order as it was
    add(0x44f510, { 0, 0, 0, 0, Kind::Plain });                       // loadRecords
    add(0x44f5f0, { 0, 0, 0, 0, Kind::Plain });                       // saveRecords
    add(0x446990, { EAX, 0, 0, 0, Kind::Bool, true });                // loadKeptReplay
    add(0x4e0e60, { EAX, 0, 0, 0, Kind::Bool, true });                // fileExists
    add(0x4e0fe0, { EAX | EDX, 0, 0, 0, Kind::Tuning, true });        // loadFileQuietly: 0 or memory to read
    add(0x439250, { EAX, 0, 0, 0, Kind::Plain });                     // carName
    add(0x4e28a0, { EAX | EDX, 0, 0, 0, Kind::Plain, true });         // unlink
    add(0x452190, { EAX | EBX | ECX | EDX, 2, 8, EBX | EDX, Kind::Plain, true });  // drawMenuText
    add(0x4e0ec0, { EAX | EDX, 0, 0, 0, Kind::Plain, true });         // loadFile
    add(0x4edfe8, { EAX | EDX, 0, 0, 0, Kind::Bool, true });          // fopen
    add(0x4ee100, { EAX, 0, 0, 0, Kind::Plain });                     // fclose
    add(0x4ee310, { EAX | EDX, 0, 0, EDX, Kind::Bool, true });        // strcmp
    add(0x4e9ec0, { EAX | EDX, 0, 0, 0, Kind::Bool, true });          // stricmp
    add(0x4ef190, { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Shape });   // the canvas
    add(0x4eada0, { 0, 0, 0, 0, Kind::Plain });
    add(0x4ef1b0, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4ef3f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
    add(0x4ef520, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4ef760, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4ef830, { EAX | EDX, 0, 0, EDX, Kind::Plain });
    add(0x4efe30, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
    add(0x4efef0, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4eff00, { EAX | EDX, 0, 0, 0, Kind::Plain });
    add(0x472300, { 0, 0, 0, 0, Kind::Plain });                       // setCameraViews
    add(0x423a60, { 0, 0, 0, 0, Kind::Plain });                       // buildCameraCycle
    add(0x423ae0, { EAX | EDX, 0, 0, EDX, Kind::Plain });             // rememberMode
    add(0x4238b0, { EAX | EDX, 0, 0, EDX, Kind::Plain });             // setCameraMode
    add(0x472820, { 0, 0, 0, 0, Kind::Plain });                       // saveConfig
    add(0x446060, { EAX, 0, 0, 0, Kind::Plain, true });               // readReplayInfo
    add(0x477b50, { EAX, 0, 0, 0, Kind::Bool, true });
    add(0x478090, { EAX, 0, 0, 0, Kind::Bool });
    add(0x478100, { EAX, 0, 0, 0, Kind::Bool });
    add(0x49f790, { 0, 0, 0, 0, Kind::Bool });
    add(0x49f880, { EAX, 0, 0, 0, Kind::Bool });
    add(0x4a6a10, { EAX | EDX, 0, 0, 0, Kind::Plain });
    add(0x438ad0, { EAX | EDX, 0, 0, 0, Kind::Plain });               // makeCarId
    add(0x439990, { EAX, 0, 0, 0, Kind::Plain });                     // findCarById
    // A line of text, the screen shown: read through.  A shape found may be none.
    specs.at(0x4d1850).kind = Kind::Shape;
    specs.at(0x44aeb0).kind = Kind::Shape;
    specs.at(0x4ef270).kind = Kind::Tuning;
    return true;
}();
(void)miscSpecsAdded;

// Folders, short: they go into a function's own buffers.
for (const x86::reg32 folder : { 0x7a2f34u, 0x7a3234u, 0x7a2e34u })
    std::memcpy(g_memory + folder, "dir\\", 5);

// A record's names (records' file at 0x5f3200, Kind::Shape's lines too): short
// enough for a function's own buffers.
for (x86::reg32 at = 0x5f3200; at < 0x5f5000; at += 0x30)
    g_memory[at] = 0;

// loadsave: the list and its buttons among the pool, and every item of the pool
// a list that could be it.
setWord32(0x5591c0, maybeItem(8));
setWord32(0x5591c4, maybeItem(4));
setWord32(0x5591c8, maybeItem(4));
setWord32(0x5591bc, random(5));
for (x86::reg32 i = 0; i < 10; ++i)
{
    const x86::reg32 name = 0x5f3b00 + 16 * i;
    std::memcpy(g_memory + name, "name\0", 5);
    g_memory[name + 4] = x86::reg8(random(3) == 0 ? 0 : 'a' + i);
    g_memory[name + 5] = 0;
    setWord32(0x5f3a20 + 8 * i - 8, name);
}
for (std::uint32_t i = 0; i < kPoolItems + 10; ++i)
{
    const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
    setWord32(item + 0x48, random(6));
    setWord32(item + 0x4c, x86::reg32(random(7)) - 1);
    setWord32(item + 0x70, 0x5f3a20);
}
// The files loadFileQuietly hands out (Kind::Tuning): 0x5f3e00 and the seven
// after it, four bytes apart.
for (x86::reg32 at = 0x5f5000; at < 0x5f6000; at += 2)
{
    const std::uint16_t v = std::uint16_t(random(12)) - 1;
    std::memcpy(g_memory + at, &v, 2);
}
for (x86::reg32 k = 0; k < 8; ++k)
{
    const x86::reg32 file = 0x5f3e00 + 4 * k;
    setWord32(file + 0x19b4, random(3));
    setWord32(file + 0x196c, x86::reg32(random(12)) - 1);
    setWord32(file + 0x1974, random(9));
    setWord32(file + 0x1930, random(2));
}
for (x86::reg32 i = 0; i < 16; ++i)
    setWord32(0x55d004 + 4 * i, x86::reg32(random(12)) - 1);

// records.
setWord32(0x559424, maybeItem(4));
setWord32(0x559408, random(2) ? 0 : 0x5f3200);
setWord32(0x55940c, 0x5f3100 + 4 * random(8));
setWord32(0x559410, 0x5f3200 + 4 * random(8));

// postgame.
for (const x86::reg32 slot : { 0x5593f0u, 0x5593f4u, 0x5593f8u, 0x5593fcu, 0x559400u, 0x559404u })
    setWord32(slot, maybeItem(16));
setWord32(0x55936c, random(6));
for (x86::reg32 i = 0; i < 8; ++i)
{
    const std::uint16_t sheet = std::uint16_t(random(3) == 0 ? 0 : 1 + random(4));
    std::memcpy(g_memory + 0x559370 + 2 * i, &sheet, 2);
}
setWord32(0x6fd4f4, random(8));
setWord32(0x6fd4b8, random(4));
setWord32(0x6fd510, random(8));
setWord32(0x55d020, random(13));
setWord32(0x6fd3ac, random(2));
setWord32(0x6fd2ac, random(2));
g_memory[0x6fd40a] = x86::reg8(random(13)), g_memory[0x6fd40b] = 0;
for (const x86::reg32 at : { 0x6fd2b0u, 0x6fd2b2u })
{
    const std::uint16_t v = std::uint16_t(random(2) ? 0x1ff : random(0x200));
    std::memcpy(g_memory + at, &v, 2);
}
// The race's results: two groups of 16.
setWord32(0x677d30, random(17));
setWord32(0x677d34, random(17));
for (x86::reg32 k = 0; k < 8; ++k)
    setWord32(0x5f3d00 + 0x20 * k + 0x10, random(16));
for (x86::reg32 g = 0; g < 2; ++g)
{
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 part = 0x6647b0 + 0x9ac0 * g + 0x9ac * i;
        setWord32(part + 0x1f0, random(8));
        setWord32(part + 0x200, random(0));
        setWord32(part + 0x220, 0x5f3d00 + 0x20 * random(8));
        setWord32(part + 0x238, random(4));
        setFloat(part + 0x2b8, someValue(g_random));
        setWord32(part + 0x2bc, random(3));
        setWord32(part + 0x2c4, random(6));
        setWord32(part + 0x2c8, random(4));
        setWord32(part + 0x2cc, random(4));
        setWord32(part + 0x2d4, random(3));
        const std::uint16_t slot = std::uint16_t(random(3) == 0 ? 0xffff : random(18));
        std::memcpy(g_memory + 0x5595d8 + 32 * g + 2 * i, &slot, 2);
    }
}
setWord32(0x559618, random(10));
setWord32(0x55961c, random(3));
for (x86::reg32 i = 0; i < 2; ++i)
    setWord32(0x5f3a80 + 4 * i, 0x6647b0 + 0x9ac0 * random(2) + 0x9ac * random(16));

// credits: 41 pages with an end after them.
for (x86::reg32 p = 0; p < 42; ++p)
{
    const x86::reg32 row = 0x555758 + 16 * p;
    const x86::reg32 first = random(8) == 0 ? 0x46a : 1 + random(0x100);
    setWord32(row, random(6));
    setWord32(row + 4, p == 41 ? 0 : first);
    setWord32(row + 8, first + random(6) - 1);
    setWord32(row + 12, 0x5f3b40);
}
setWord32(0x555c28, random(41));
setWord32(0x555c34, random(2) ? word32(0x555c28) : random(41));
setWord32(0x5f40c0, random(6));
setWord32(0x555c44, random(3) == 0 ? 0 : 8 + random(6));
{
    static const x86::reg32 kEdges[] = { 0x3c0, 0x3bf, x86::reg32(-0x3c0), x86::reg32(-0x3bf), 0, 1, x86::reg32(-1), 3 };
    setWord32(0x555c2c, random(2) ? kEdges[random(std::size(kEdges))] : x86::reg32(random(2200)) - 1100);
    static const x86::reg32 kSpeeds[] = { 0, 3, 4, 0x3b, 0x3c, 0x3d };
    setWord32(0x555c30, random(2) ? kSpeeds[random(std::size(kSpeeds))] : random(70));
}
// The lines of a page of text, which marks some of them in their first characters.
for (x86::reg32 k = 0; k < 16; ++k)
{
    static const char kMarks[] = { '#', '@', '$', 'x' };
    const x86::reg32 text = 0x5f3c00 + 16 * k;
    g_memory[text] = x86::reg8(kMarks[random(4)]);
    g_memory[text + 1] = x86::reg8(random(2) ? kMarks[random(4)] : 'x');
}
setWord32(0x555754, random(4) == 0 ? 0 : 0x5f3c00);

// credits1.
setWord32(0x664794, random(4));
