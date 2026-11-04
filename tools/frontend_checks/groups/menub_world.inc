// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
// The stand-ins these functions need that kSpecs does not hold, added to it the first time.
static const bool menubSpecsAdded = [] {
    auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
    const auto add = [&](x86::reg32 address, Spec spec) { specs.emplace(address, spec); };
    // The menu loop's.
    add(0x4a75e0, { 0, 0, 0, 0, Kind::Plain });
    add(0x4d1340, { 0, 0, 0, 0, Kind::Plain });
    add(0x434b60, { 0, 0, 0, 0, Kind::Plain });
    add(0x44a8b0, { EAX, 0, 0, 0, Kind::Plain });
    add(0x4c4490, { 0, 0, 0, 0, Kind::Plain });
    // The awards'.
    add(0x4c99f0, { 0, 0, 0, 0, Kind::Bool });
    add(0x4c9990, { EAX | EDX, 0, 0, EDX, Kind::Plain });
    // The records'.
    add(0x4ee4f8, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX, Kind::Plain });  // qsort: the order as it was
    add(0x4e9ec0, { EAX | EDX, 0, 0, 0, Kind::Bool, true });
    add(0x44f510, { 0, 0, 0, 0, Kind::Plain });
    add(0x44f5f0, { 0, 0, 0, 0, Kind::Plain });
    add(0x4cf160, { EAX, 0, 0, 0, Kind::Focus });                     // a class of laps: 0, 1 or 2
    add(0x46cce0, { EAX | EBX | EDX, 3, 12, 0, Kind::Plain });
    add(0x4d8860, { EAX | EBX | ECX | EDX, 4, 16, EBX | ECX | EDX, Kind::Plain });
    add(0x4d8930, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain });
    add(0x452190, { EAX | EBX | ECX | EDX, 2, 8, EBX | EDX, Kind::Plain, true });
    add(0x49c1f0, { EAX, 0, 0, 0, Kind::Bool });
    add(0x4181d0, { EAX | EDX, 0, 0, EDX, Kind::Plain });
    add(0x4cee40, { EAX | EDX, 0, 0, EDX, Kind::Plain });
    add(0x4cedd0, { 0, 2, 8, 0, Kind::Plain });
    add(0x449df0, { 0, 0, 0, 0, Kind::Bool });
    add(0x439ab0, { EAX | EDX, 0, 0, EDX, Kind::Shape });             // a car's line of text: memory to read
    add(0x439250, { EAX, 0, 0, 0, Kind::Shape });                     // a car's name
    // The codes'.
    add(0x4eede0, { EAX, 0, 0, 0, Kind::NoEax });                     // toupper: the letters are capitals
    add(0x439d90, { 0, 0, 0, 0, Kind::Plain });
    add(0x442b60, { EAX, 0, 0, 0, Kind::Plain });
    // The cars'.
    add(0x431900, { EAX | EDX, 0, 0, 0, Kind::Plain });
    add(0x4392b0, { EAX, 0, 0, 0, Kind::Tuning });
    add(0x4395c0, { EAX, 0, 0, 0, Kind::Bool });
    add(0x439290, { EAX, 0, 0, 0, Kind::Shape });
    add(0x4e0e60, { EAX, 0, 0, 0, Kind::Bool, true });
    add(0x4ee310, { EAX | EDX, 0, 0, EDX, Kind::Bool, true });
    add(0x439730, { EAX, 0, 0, 0, Kind::Focus });
    add(0x438bd0, { 0, 0, 0, 0, Kind::Plain });
    add(0x438f60, { EAX | EDX, 0, 0, EDX, Kind::Plain });
    add(0x49c240, { EAX | EBX | ECX | EDX, 0, 0, 0, Kind::Bool });
    add(0x4d7990, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
    add(0x4d7560, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
    add(0x4bebe0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain });
    // The results'.
    add(0x4cef80, { 0, 0, 0, 0, Kind::Bool });
    add(0x4c7ea0, { 0, 0, 0, 0, Kind::Plain });
    // The direction pad's.
    add(0x4654c0, { 0, 2, 8, 0, Kind::Plain });

    // A line of the language's goes through strcpy here.
    specs.at(0x4d1850).kind = Kind::Shape;

    // How many cars there are: a number the world chooses.
    g_methods[0x438bd0] = [](win32::WinApplication* app, x86::CPU& cpu) {
        nfs3hp::mock(app, cpu, 0x438bd0);
        cpu.eax = word32(0x5f3ff0);
    };
    // The speed written out: the double's words, which the stand-in's sprintf does not read.
    g_methods[0x4df690] = [](win32::WinApplication* app, x86::CPU& cpu) {
        if (word32(cpu.esp + 8) == 0x539900)
        {
            g_log.push_back(word32(cpu.esp + 12));
            g_log.push_back(word32(cpu.esp + 16));
            g_log.push_back(word32(cpu.esp + 20));
        }
        nfs3hp::mock(app, cpu, 0x4df690);
    };
    // What others' groups have: stood in for here, their own checks run them.
    g_methods.erase(0x4cef80);
    g_methods.erase(0x442b60);
    return true;
}();
(void)menubSpecsAdded;

// A car's statistic: small and often the same, or anything.
const_cast<Spec&>(kSpecs.at(0x438f60)).kind = random(2) ? Kind::Focus : Kind::Plain;

// The strings the stand-ins hand out: short, for they go into functions' buffers.
for (x86::reg32 at = 0x5f3c07; at < 0x5f3d00; at += 0x10)
    g_memory[at] = 0;
for (x86::reg32 at = 0x5f3e03; at < 0x5f3e20; at += 4)
    g_memory[at] = 0;
g_memory[0x7a28b4 + 4] = 0;
g_memory[0x7a3234 + 4] = 0;

// The records' file, away from everything else, or none (then its lists are the
// zeros low in memory).  Every list with a count that makes sense; the chosen one
// with its records and its holder.
std::memset(g_memory, 0, 0x50000);
std::memset(g_memory + 0x740000, 0, 0x50000);
const x86::reg32 recordsFile = random(4) == 0 ? 0 : 0x740000;
const auto recordsList = [&](x86::reg32 track, x86::reg32 laps) { return recordsFile + 0x16e0 + laps * 0x15634 + track * 0x1f1c; };
if (recordsFile != 0)
{
    for (x86::reg32 laps = 0; laps < 3; ++laps)
        for (x86::reg32 track = 0; track < 14; ++track)
            setWord32(recordsList(track, laps), random(4) == 0 ? 10 : random(11));
}
const auto someName = [&](x86::reg32 at, x86::reg32 most) {
    const x86::reg32 length = random(most);
    for (x86::reg32 c = 0; c < length; ++c)
        g_memory[at + c] = x86::reg8('a' + random(26));
    g_memory[at + length] = 0;
};
setWord32(0x559408, recordsFile);
setWord32(0x559414, random(8));
setWord32(0x559418, random(13));
setWord32(0x55941c, random(3));
{
    const x86::reg32 list = recordsList(word32(0x559418), word32(0x55941c));
    setWord32(0x55940c, list);
    for (x86::reg32 i = 0; i < 10; ++i)
    {
        const x86::reg32 record = list + 4 + i * 0x31c;
        setWord32(record, random(12));
        someName(record + 4, 10);
        someName(record + 0xf, 40);
        someName(record + 0x10f, 40);
        someName(record + 0x20f, 40);
        setWord32(record + 0x310, random(5));
        setFloat(record + 0x314, someValue(g_random));
        const std::uint16_t aids = std::uint16_t(random(3));
        const std::uint16_t isNew = std::uint16_t(random(3));
        std::memcpy(g_memory + record + 0x318, &aids, 2);
        std::memcpy(g_memory + record + 0x31a, &isNew, 2);
    }
    for (x86::reg32 i = 0; i < 11; ++i)
    {
        const std::uint16_t rank = std::uint16_t(random(4) == 0 ? 0xffff : random(11));
        std::memcpy(g_memory + 0x6645d0 + 2 * i, &rank, 2);
    }
    const x86::reg32 holder = recordsFile + 4 + word32(0x559418) * 0x214;
    setWord32(0x559410, holder);
    setWord32(holder, random(3) == 0 ? 0 : random(6));
    setWord32(holder + 4, random(4));
    someName(holder + 8, 10);
    someName(holder + 0x13, 40);
    someName(holder + 0x113, 40);
}
setWord32(0x662968, random(2));
setWord32(0x79c1a4, random(2) ? random(0x200) - 0x40 : 0x3e + random(5));
setWord32(0x6fd4b4, random(9));

// The items that are sheets: few rows, so few calls.
for (x86::reg32 i = 0; i < kPoolItems + 10; ++i)
{
    const x86::reg32 item = kItems + kItemSize * (kPoolFirst + i);
    const std::uint16_t rows = std::uint16_t(random(14));
    const std::uint16_t rowHeight = std::uint16_t(random(30));
    const std::uint16_t first = std::uint16_t(random(12));
    const std::uint16_t key = std::uint16_t(random(9));
    const std::uint16_t slide = std::uint16_t(random(5));
    const std::uint16_t slideX = std::uint16_t(random(3) == 0 ? 0 : random(800));
    const std::uint16_t shownTrack = std::uint16_t(random(13));
    const std::uint16_t shownLaps = std::uint16_t(random(3));
    std::memcpy(g_memory + item + 0x4a, &rows, 2);
    std::memcpy(g_memory + item + 0x48, &rowHeight, 2);
    std::memcpy(g_memory + item + 0xb4, &first, 2);
    std::memcpy(g_memory + item + 0xb6, &key, 2);
    std::memcpy(g_memory + item + 0xb8, &slide, 2);
    std::memcpy(g_memory + item + 0xba, &slideX, 2);
    std::memcpy(g_memory + item + 0xa4, &shownTrack, 2);
    std::memcpy(g_memory + item + 0xa6, &shownLaps, 2);
}

// The awards.
for (x86::reg32 at = 0x664440; at < 0x6644a8; at += 4)
    setWord32(at, random(3) == 0);
setWord32(0x55936c, random(6));
setWord32(0x6fd4f4, random(8));
setWord32(0x6fd4b8, random(4));
setWord32(0x55d020, random(13));
setWord32(0x6fdc30, random(3));
{
    const std::uint16_t track = std::uint16_t(random(13));
    std::memcpy(g_memory + 0x6fd408, &track, 2);
}

// The race's results: two groups of 16; their cars' records.
setWord32(0x677d30, random(17));
setWord32(0x677d34, random(17));
setWord32(0x6fd3a0, random(4) == 0);
for (x86::reg32 k = 0; k < 8; ++k)
{
    setWord32(0x5f3d00 + 0x20 * k + 0x10, random(16));
    setWord32(0x5f3d00 + 0x20 * k + 0x14, random(3));
}
const auto makeParticipant = [&](x86::reg32 part) {
    setWord32(part + 0x1f0, random(8));
    setWord32(part + 0x200, random(0));
    setWord32(part + 0x220, 0x5f3d00 + 0x20 * random(8));
    setWord32(part + 0x238, random(6));
    for (x86::reg32 lap = 0; lap < 8; ++lap)
        setWord32(part + 0x240 + 4 * lap, random(3) == 0 ? 0 : random(6));
    setFloat(part + 0x2b8, random(3) == 0 ? someValue(g_random) : 1.0f + someValue(g_random) * 100.0f);
    setWord32(part + 0x2bc, random(3));
    setWord32(part + 0x2c4, random(6));
    setWord32(part + 0x2c8, random(4));
    setWord32(part + 0x2cc, random(4));
    setWord32(part + 0x2d4, random(3));
};
for (x86::reg32 g = 0; g < 2; ++g)
{
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        makeParticipant(0x6647b0 + 0x9ac0 * g + 0x9ac * i);
        const std::uint16_t slot = std::uint16_t(random(3) == 0 ? 0xffff : random(18));
        std::memcpy(g_memory + 0x5595d8 + 32 * g + 2 * i, &slot, 2);
    }
}
setWord32(0x559618, random(10));
setWord32(0x55961c, random(3));
for (x86::reg32 i = 0; i < 2; ++i)
    setWord32(0x5f3a80 + 4 * i, 0x6647b0 + 0x9ac0 * random(2) + 0x9ac * random(16));
// The race's own cars, copied into the results.
setWord32(0x5efd9c, random(9));
for (x86::reg32 i = 0; i < 8; ++i)
{
    const x86::reg32 car = 0x7e1000 + 0x9ac * i;
    makeParticipant(car);
    setWord32(0x5efac8 + 4 * i, car);
}
// The drivers' names.
for (x86::reg32 i = 0; i < 16; ++i)
    g_memory[0x6fd52c + 0x6c * i + 0x34 + random(10)] = 0;
for (x86::reg32 i = 0; i < 16; ++i)
{
    const std::uint16_t dropped = std::uint16_t(random(2));
    std::memcpy(g_memory + 0x7a22ca + 2 * i, &dropped, 2);
}
setWord32(0x6fbc68, random(6));

// The codes: short ones, of a few letters, some way in.
setWord32(0x559538, random(5));
setWord32(0x55953c, random(2) ? 20 : random(30));
for (x86::reg32 i = 0; i < 21; ++i)
{
    static const char kLetters[] = "ABGO05";
    const x86::reg32 code = 0x5f3a00 + 8 * i;
    g_memory[code] = x86::reg8(kLetters[random(6)] + 9);
    g_memory[code + 1] = random(2) ? 0 : x86::reg8(kLetters[random(6)] + 9);
    g_memory[code + 2] = 0;
    setWord32(0x5594e4 + 4 * i, code);
    setWord32(0x6645f0 + 4 * i, random(3) == 0 ? 1 : 0);
}
g_memory[0x6fd2a8] = x86::reg8(random(2) ? 0 : random(32));

// The cars' sheets.
g_memory[0x7a3a58] = x86::reg8(random(2) * 2);
setWord32(0x55957c, random(4));
setWord32(0x55958c, random(8));
setWord32(0x664798, random(40));
setWord32(0x664784, random(40));
setWord32(0x664788, random(40));
setWord32(0x554f50, random(4));
setWord32(0x5f3ff0, word32(0x554f50) + random(30));
setWord32(0x5595d4, random(6));
setWord32(0x559570, random(6));
setWord32(0x559574, random(4));
setWord32(0x5595ac, random(2) ? 0 : random(0));
setWord32(0x5595b0, random(2) ? 0 : random(0));

// The direction pad: the pointer near its arrows, or its centre, or not.
setWord32(0x677da0, random(4) != 0);
for (x86::reg32 i = 0; i < 8; ++i)
{
    const std::uint16_t x = std::uint16_t(0x200 + random(0x60));
    const std::uint16_t y = std::uint16_t(0xf0 + random(0x60));
    std::memcpy(g_memory + 0x559874 + 2 * i, &x, 2);
    std::memcpy(g_memory + 0x559884 + 2 * i, &y, 2);
}
setWord32(0x55e5e4, 0x200 + random(0x60));
setWord32(0x55e5e8, 0xf0 + random(0x60));
