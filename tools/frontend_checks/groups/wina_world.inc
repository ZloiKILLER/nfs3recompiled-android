// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    using namespace nfs3hp;
    static const bool winaStandIns = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        // Imports: what is in the table in the file; their words on the stack, popped.
        struct ImportRow { x86::reg32 slot; std::uint8_t words; Kind kind; };
        const ImportRow imports[] = {
            { 0x5344b8, 1, Kind::NoEax }, { 0x534588, 1, Kind::NoEax }, { 0x5344dc, 1, Kind::Plain }, { 0x53458c, 1, Kind::Bool },
            { 0x534558, 2, Kind::Plain }, { 0x534534, 0, Kind::Bool }, { 0x534764, 4, Kind::Focus }, { 0x534700, 2, Kind::Bool },
            { 0x53473c, 2, Kind::Plain }, { 0x534788, 1, Kind::Plain }, { 0x534750, 1, Kind::Plain }, { 0x534794, 1, Kind::Plain },
            { 0x5347a0, 4, Kind::Plain }, { 0x5347a8, 1, Kind::Plain }, { 0x534790, 4, Kind::Plain }, { 0x53446c, 5, Kind::Bool },
            { 0x534470, 6, Kind::Bool }, { 0x5346ec, 3, Kind::Plain }, { 0x534780, 1, Kind::Plain }, { 0x534778, 4, Kind::Bool },
            { 0x534768, 4, Kind::Bool }, { 0x534784, 2, Kind::Plain }, { 0x534738, 1, Kind::Plain }, { 0x5346f4, 4, Kind::Plain },
            { 0x53476c, 1, Kind::Plain }, { 0x5346f0, 2, Kind::Plain }, { 0x534730, 2, Kind::Plain }, { 0x534724, 4, Kind::Plain },
            { 0x534704, 12, Kind::Bool }, { 0x53477c, 1, Kind::Plain }, { 0x53474c, 4, Kind::Bool }, { 0x5347a4, 1, Kind::Plain },
            { 0x53472c, 1, Kind::Plain }, { 0x534728, 1, Kind::Plain }, { 0x53475c, 1, Kind::Bool }, { 0x53454c, 1, Kind::Plain },
            { 0x534760, 2, Kind::Plain }, { 0x53445c, 1, Kind::Plain }, { 0x534774, 1, Kind::Bool }, { 0x534758, 2, Kind::Plain },
            { 0x53478c, 7, Kind::Plain }, { 0x534544, 3, Kind::Plain }, { 0x534568, 0, Kind::Plain }, { 0x534580, 1, Kind::Plain },
            { 0x534570, 1, Kind::Bool }, { 0x534734, 2, Kind::Bool }, { 0x53459c, 1, Kind::Plain }, { 0x534798, 2, Kind::Plain },
            { 0x5345f4, 1, Kind::Plain }, { 0x5345d8, 1, Kind::Plain }, { 0x5345f0, 4, Kind::Plain },
        };
        for (const ImportRow& row : imports)
            specs[word32(row.slot)] = { 0, row.words, std::uint8_t(row.words * 4), 0, row.kind };

        // What is called through a pointer.
        specs[kWinaCreate] = { 0, 3, 12, 0, Kind::Plain };
        specs[kWinaFn] = { EAX, 0, 0, 0, Kind::Plain };
        specs[kWinaFn2] = { EAX | EDX, 0, 0, 0, Kind::Bool };
        specs[kWinaFn3] = { EAX | EBX | EDX, 0, 0, 0, Kind::Plain };
        specs[kWinaFn0] = { 0, 0, 0, 0, Kind::Plain };
        specs[kWinaHandler] = { 0, 6, 24, 0, Kind::Bool };
        specs[kWinaPriority] = { EAX | EDX, 0, 0, 0, Kind::Plain };
        specs[kWinaQuery] = { 0, 3, 12, 0, Kind::Plain };
        specs[kWinaRelease] = { 0, 1, 4, 0, Kind::Plain };
        specs[kWinaMakeSurface] = { 0, 4, 16, 0, Kind::Plain };
        specs[kWinaCoop] = { 0, 3, 12, 0, Kind::Plain };
        g_methods[kWinaCreate] = &winaComCreate;
        g_methods[kWinaQuery] = &winaComQuery;
        g_methods[kWinaMakeSurface] = &winaComMakeSurface;
        g_methods[kWinaCoop] = &winaComCoop;
        g_methods[kWinaPriority] = &winaPriorityOf;

        // sub_4f2a7d leaves the game, and these two read as having lost their stack: nothing to clobber.
        specs[0x4f2a7d] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4ed2a0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x5178d0] = { 0, 0, 0, 0, Kind::Plain };
        // The empty hook's two arguments, compared though it reads none; the caller cleans.
        specs[0x42d500] = { 0, 2, 0, 0, Kind::NoEax };
        // What is read as an answer.
        for (const x86::reg32 address : { 0x4cf1f0u, 0x4cf440u, 0x4d04c0u, 0x4df930u, 0x4df7a0u, 0x4f9120u, 0x4f9070u, 0x4516a0u,
                                          0x50dc20u, 0x4ee310u, 0x533e98u, 0x533e92u, 0x533e8cu, 0x4f7900u })
            specs[address].kind = Kind::Bool;
        // The stand-ins' rows, without what the file's reading thinks may reach them by accident.
        for (const x86::reg32 address : { 0x4cf810u, 0x4cf6b0u, 0x4d0060u, 0x4d0220u, 0x4fca60u, 0x4fb990u, 0x517580u, 0x517920u })
            specs[address].in &= std::uint8_t(~(ESI | EDI));

        g_methods[0x4cf1f0] = &winaStand<0x4cf1f0>;
        g_methods[0x4cf220] = &winaStand<0x4cf220>;
        g_methods[0x4cf2f0] = &winaStand<0x4cf2f0>;
        g_methods[0x4cf440] = &winaStand<0x4cf440>;
        g_methods[0x4cf5e0] = &winaStand<0x4cf5e0>;
        g_methods[0x4cf6b0] = &winaStand<0x4cf6b0>;
        g_methods[0x4cf810] = &winaStand<0x4cf810>;
        g_methods[0x4cfe30] = &winaStand<0x4cfe30>;
        g_methods[0x4d0220] = &winaStand<0x4d0220>;
        g_methods[0x4d03c0] = &winaStand<0x4d03c0>;
        g_methods[0x4d04c0] = &winaStand<0x4d04c0>;
        g_methods[0x4fc8e0] = &winaStand<0x4fc8e0>;
        g_methods[0x4fc9b0] = &winaStand<0x4fc9b0>;
        g_methods[0x4fca60] = &winaStand<0x4fca60>;
        g_methods[0x4fca90] = &winaStand<0x4fca90>;
        g_methods[0x4fcab0] = &winaStand<0x4fcab0>;
        g_methods[0x4fce50] = &winaStand<0x4fce50>;
        g_methods[0x4fbb60] = &winaStand<0x4fbb60>;
        g_methods[0x4fbbe0] = &winaStand<0x4fbbe0>;
        g_methods[0x4e4320] = &winaStand<0x4e4320>;
        g_methods[0x4fd060] = &winaStand<0x4fd060>;
        g_methods[0x517580] = &winaStand<0x517580>;
        g_methods[0x517920] = &winaStand<0x517920>;
        g_methods[0x51a170] = &winaStand<0x51a170>;
        g_methods[0x519140] = &winaStand<0x519140>;
        g_methods[0x525600] = &winaStand<0x525600>;
        g_methods[0x4cdaa0] = &winaStand<0x4cdaa0>;
        g_methods[0x4cd350] = &winaStand<0x4cd350>;
        g_methods[0x4cdb10] = &winaStand<0x4cdb10>;
        g_methods[0x4cdbd0] = &winaStand<0x4cdbd0>;
        g_methods[0x4f9120] = &winaStand<0x4f9120>;
        g_methods[0x4b6590] = &winaStand<0x4b6590>;
        g_methods[0x4cc6d0] = &winaStand<0x4cc6d0>;
        g_methods[0x4cc7c0] = &winaStand<0x4cc7c0>;
        g_methods[0x503cd0] = &winaStand<0x503cd0>;
        g_methods[0x4fb470] = &winaStand<0x4fb470>;
        g_methods[0x4cc8e0] = &winaStand<0x4cc8e0>;
        g_methods[0x4cc890] = &winaStand<0x4cc890>;
        g_methods[0x4cf280] = &winaTakeFree;
        g_methods[0x4d0060] = &winaCloseFile;
        g_methods[0x4d0300] = &winaNextChunk;
        g_methods[0x4fb990] = &winaOp<0x4fb990>;
        g_methods[0x4fbd10] = &winaFindOp;
        g_methods[0x50df60] = &winaOpFromFree;
        g_methods[0x50e5a0] = &winaOpByPriority;
        g_methods[0x501110] = &winaNameCopy;
        g_methods[0x4fb300] = &winaBsearch;
        g_methods[0x533e8c] = &winaQueryValue;
        g_methods[0x4df8e0] = &winaWaiting<0x4df8e0>;
        g_methods[0x4e7630] = &winaWaiting<0x4e7630>;
        g_winaPeek = word32(0x53474c);
        g_winaProc = word32(0x534558);
        g_winaVersion = word32(0x534570);
        g_methods[g_winaPeek] = &winaPeek;
        g_methods[g_winaProc] = &winaProcAddress;
        g_methods[g_winaVersion] = &winaVersionInfo;

        // sub_4ccaa0's pieces.
        g_methods[0x4ccbb7] = &winaKeyPieceLow;
        g_methods[0x4ccbc9] = &winaKeyPiece<0x4900>;
        g_methods[0x4ccbd0] = &winaKeyPiece<0x5100>;
        g_methods[0x4ccbd7] = &winaKeyPiece<0x4f00>;
        g_methods[0x4ccbde] = &winaKeyPiece<0x4700>;
        g_methods[0x4ccbe5] = &winaKeyPiece<0x4b00>;
        g_methods[0x4ccbec] = &winaKeyPiece<0x4d00>;
        g_methods[0x4ccbf3] = &winaKeyPiece<0x5000>;
        g_methods[0x4ccbfa] = &winaKeyPiece<0x5200>;
        g_methods[0x4ccc01] = &winaKeyPiece<0x5300>;
        g_methods[0x4ccc08] = &winaKeyPiece<0x4c00>;
        g_methods[0x4ccc0f] = &winaKeyPiece<0x3b00>;
        g_methods[0x4ccc16] = &winaKeyPiece<0x3c00>;
        g_methods[0x4ccc1d] = &winaKeyPiece<0x3d00>;
        g_methods[0x4ccc24] = &winaKeyPiece<0x3e00>;
        g_methods[0x4ccc2b] = &winaKeyPiece<0x3f00>;
        g_methods[0x4ccc32] = &winaKeyPiece<0x4000>;
        g_methods[0x4ccc39] = &winaKeyPiece<0x4100>;
        g_methods[0x4ccc40] = &winaKeyPiece<0x4200>;
        g_methods[0x4ccc47] = &winaKeyPiece<0x4300>;
        g_methods[0x4ccc4e] = &winaKeyPiece<0x4400>;
        g_methods[0x4ccc55] = &winaKeyPiece<0x8700>;
        g_methods[0x4ccc5c] = &winaKeyPiece<0x8800>;
        g_methods[0x4ccc63] = &winaKeyPiece<0>;
        // sub_4fb470's.
        g_methods[0x4fb6a0] = &winaPointer6a0;
        g_methods[0x4fb6a7] = &winaPointer6a7;
        g_methods[0x4fb6ae] = &winaPointer6ae;
        g_methods[0x4fb6b5] = &winaPointer6b5;
        g_methods[0x4fb6c2] = &winaPointer6c2;
        g_methods[0x4fb6d7] = &winaPointer6d7;
        return true;
    }();
    (void)winaStandIns;

    const x86::reg32 S = kWinaStream;
    const auto oneIn = [](x86::reg32 n) { return random(n) == 0; };

    // A stream: its tables, as sub_4cfa80 lays them out.
    constexpr x86::reg32 kFileCount = 4, kRuleCount = 3, kChannelCount = 3;
    const x86::reg32 rules = S + 0x610, channels = kWinaChannels, ring = S + 0x1000;
    const x86::reg32 ringEnd = ring + (oneIn(2) ? 0x3000 : 0x8000);
    setWord32(S, 0x4d525453);
    setWord32(S + 8, kWinaFiles);
    setWord32(S + 0xc, kFileCount);
    setWord32(S + 0x10, rules);
    setWord32(S + 0x14, kRuleCount);
    setWord32(S + 0x18, channels);
    setWord32(S + 0x1c, kChannelCount);
    setWord32(S + 0x20, ring);
    setWord32(S + 0x24, ringEnd);
    setWord32(S + 0x28, random(3));
    setWord32(S + 0x2c, 0x96);
    setWord32(S + 0x30, 0x32);
    setWord32(S + 0x34, random(0x1000));
    setWord32(S + 0x38, random(2));
    setWord32(S + 0x3c, random(0x1000));
    setWord32(S + 0x160, oneIn(3) ? 0 : 0x123);
    setWord32(S + 0x164, random(0x1000));
    setWord32(S + 0x16c, oneIn(2) ? 0x2000 : random(0x2000));
    // The rules: a channel by the id's low bits, or none.
    for (x86::reg32 i = 0; i < kRuleCount; ++i)
    {
        setWord32(rules + i * 12, 3);
        setWord32(rules + i * 12 + 4, i);
        setWord32(rules + i * 12 + 8, i == 2 ? 0xfffffffe : i + 1);
    }

    // The chunks of the ring: some parsed and on their channels or freed, the rest not yet.
    const bool wraps = oneIn(3);
    const x86::reg32 start = ring + (wraps ? (oneIn(2) ? 0x800 : 0x2400) : (oneIn(2) ? 0 : 0x2400));
    const x86::reg32 count = 2 + random(5), parsedCount = random(count + 1);
    const x86::reg32 wrapAt = wraps ? 1 + random(count) : count + 1;
    x86::reg32 at = start;
    x86::reg32 boundary[10] = {};
    x86::reg32 bytes[kChannelCount] = {}, firsts[kChannelCount] = { ring, ring, ring };
    for (x86::reg32 i = 0; i < count; ++i)
    {
        if (i == wrapAt)
        {
            setWord32(at, 0xffffffff);
            setWord32(at + 4, 8);
            at = ring;
        }
        boundary[i] = at;
        const x86::reg32 length = 8 + 4 * random(16);
        const x86::reg32 id = random(0);
        if (i < parsedCount)
        {
            if (oneIn(4))
            {
                setWord32(at, 0xfffffffe);
                setWord32(at + 4, length);
            }
            else
            {
                const x86::reg32 channel = random(kChannelCount);
                setWord32(at, id);
                setWord32(at + 4, length | ((channel + 1) << 24));
                if (bytes[channel] == 0)
                    firsts[channel] = at;
                bytes[channel] += length;
            }
        }
        else
        {
            setWord32(at, id);
            setWord32(at + 4, oneIn(8) ? random(0) : length);
        }
        at += length;
    }
    boundary[count] = at;
    setWord32(S + 0x40, boundary[0]);
    setWord32(S + 0x44, boundary[parsedCount]);
    setWord32(S + 0x48, parsedCount < count && oneIn(4) ? at - random(8) : at);
    for (x86::reg32 i = 0; i < kChannelCount; ++i)
    {
        setWord32(channels + 16 * i, S);
        setWord32(channels + 16 * i + 4, i + 1);
        setWord32(channels + 16 * i + 8, bytes[i]);
        setWord32(channels + 16 * i + 12, firsts[i]);
    }

    // The file records: the first ones on the queue, the rest free.
    const x86::reg32 queued = 1 + random(3);
    setWord32(S + 0x4c, kWinaFiles);
    setWord32(S + 0x50, kWinaFiles + 0x128 * random(queued));
    setWord32(S + 0x54, kWinaFiles + 0x128 * (queued - 1));
    if (oneIn(10))
        setWord32(S + 0x50, 0);
    setWord32(S + 0x58, queued == kFileCount ? 0 : kWinaFiles + 0x128 * queued);
    if (oneIn(4))
        setWord32(S + 0x58, 0);
    // The path of the file open, and what a record may have instead.
    const char* const pathName = "dir\\a.mad";
    std::memcpy(g_memory + S + 0x5c, pathName, std::strlen(pathName) + 1);
    x86::reg32 place = 0;
    for (x86::reg32 i = 0; i < kFileCount; ++i)
    {
        const x86::reg32 record = kWinaFiles + 0x128 * i;
        const bool onQueue = i < queued;
        setWord32(record, (random(0x10000) << 8) | i);
        setWord32(record + 4, onQueue ? 1 + random(4) : 0);
        setWord32(record + 8, onQueue && i > 0 ? record - 0x128 : 0);
        setWord32(record + 0xc, onQueue ? (i + 1 < queued ? record + 0x128 : 0)
                                        : (i + 1 < kFileCount ? record + 0x128 : 0));
        setWord32(record + 0x10, random(2));
        const char* const name = oneIn(2) ? pathName : "dir\\b.mad";
        std::memcpy(g_memory + record + 0x14, name, std::strlen(name) + 1);
        setWord32(record + 0x118, kWinaBlock);
        setWord32(record + 0x11c, word32(S + 0x164) + 0x100 * random(0x40));
        setWord32(record + 0x120, oneIn(3) ? word32(boundary[random(count)]) : random(0));
        place = place + random(parsedCount + 1 - place);
        setWord32(record + 0x124, boundary[place]);
    }

    // A handle that is none.
    setWord32(kWinaBadHandle, kWinaBadHandle + 0x10);
    setWord32(kWinaBadHandle + 0x10, 0x1234);

    // The file library.
    setWord32(0x9f4f60, oneIn(20) ? 0 : kWinaSlots);
    for (x86::reg32 i = 0; i < 34; ++i)
    {
        setWord32(kWinaSlots + i * 104, random(2));
        setWord32(kWinaSlots + i * 104 + 0x54, random(0));
        setWord32(kWinaSlots + i * 104 + 0x60, oneIn(3) ? 0xffffff : random(0x1000));
    }
    for (x86::reg32 i = 0; i < 0x40; i += 4)
        setWord32(kWinaOp + i, random(0));
    setWord32(kWinaOp + 0x20, oneIn(2) ? 0 : kWinaFn3);

    // The table of open files.
    setWord32(0x5645a0, oneIn(10) ? 0 : kWinaFileTable);
    setWord32(0x56459c, 4 + random(4));
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g_memory[kWinaFileTable + 32 * i] = x86::reg8(random(3));
        g_memory[kWinaFileTable + 32 * i + 1] = x86::reg8(random(8));
        setWord32(kWinaFileTable + 32 * i + 4, random(0));
        setWord32(kWinaFileTable + 32 * i + 0x14, random(2));
    }

    // A queue of entries that have a priority.
    setWord32(kWinaQueue + 8, oneIn(5) ? 0 : kWinaQueue + 0x40);
    setWord32(kWinaQueue + 0x10, kWinaPriority);
    setWord32(kWinaQueue + 0x14, random(0));
    setWord32(kWinaQueue + 0x18, oneIn(8) ? 0 : 0x55);
    for (x86::reg32 i = 0; i < 4; ++i)
        setWord32(kWinaQueue + 0x40 + 16 * i, i == 3 ? 0 : kWinaQueue + 0x40 + 16 * (i + 1));

    // What the window's code reads through.
    setWord32(0x561298, oneIn(2) ? 0 : kWinaScanTable);
    for (x86::reg32 i = 0; i < 0x80; ++i)
        g_memory[kWinaScanTable + i] = x86::reg8(oneIn(4) ? (oneIn(2) ? 0x2a : 0x36) : random(0x80));
    setWord32(kWinaDriver + 8, kWinaFn);
    setWord32(kWinaDriver + 0x18, kWinaFn);
    setWord32(0x56718c, kWinaDriver);
    setWord32(0x7f7480, 1 + random(32));
    setWord32(0x7f747c, random(word32(0x7f7480)));
    setWord32(0x7f7474, oneIn(3) ? (word32(0x7f747c) + 1) % word32(0x7f7480) : random(word32(0x7f7480)));
    setWord32(0x7f7488, random(2));
    setWord32(0x7f7484, oneIn(2) ? 0 : kWinaFn);
    setWord32(0x567184, oneIn(2) ? 0 : kWinaFn0);
    setWord32(0x561288, oneIn(3) ? 0 : kWinaFn2);
    setWord32(0x56129c, oneIn(2) ? 0 : kWinaFn0);
    setWord32(0x56446c, oneIn(2) ? 0 : kWinaFn);
    setWord32(0x561290, oneIn(8) ? 0 : 0x66);
    setWord32(0x564368, oneIn(8) ? 0 : 1);
    setWord32(0x561268, random(0x82));
    g_memory[0x7f7490] = x86::reg8(random(2));
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        setWord32(0x7f7060 + 8 * i, random(0x200));
        setWord32(0x7f7064 + 8 * i, kWinaHandler);
    }
    setWord32(0x5612a4, oneIn(2) ? 0 : 0x5612ac);
    setWord32(0x9f38c0, oneIn(2) ? 0 : kWinaName);
    setWord32(0x561274, random(2));
    setWord32(0x561278, random(2));
    setWord32(0x56127c, oneIn(2) ? 0 : 0x88);
    setWord32(0x561280, random(2));
    setWord32(0x561284, random(2));
    setWord32(0x56126c, oneIn(2) ? 0 : 0x77);
    setWord32(0x561270, oneIn(4));
    setWord32(0x564480, oneIn(4) ? 0 : 0x99);
    setWord32(0x567194, random(2));
    setWord32(0x5650c0, oneIn(2) ? 0 : 0x4444);
    setWord32(0x564a6c, oneIn(2) ? 0 : 0x4445);
    setWord32(0x569058, oneIn(2) ? 0 : 0x4446);
    setWord32(0x56905c, oneIn(2) ? 0 : 0x4447);
    setWord32(0x564364, oneIn(2) ? 7 : random(2));
    setWord32(0x9f3d24, oneIn(3) ? 0 : 0x1000);
    setWord32(0x9f3d4c, random(2));
    g_memory[0x9f3d2d] = x86::reg8(random(2));
    setWord32(0xa18220, random(2));
    setWord32(0x9f3d0c, random(2));
    setWord32(0x9f38e0, random(2));
    setWord32(0x7a3acc, oneIn(2) ? random(0x800000) : random(0));
    setWord32(0x7a3ad4, oneIn(2) ? random(0x800000) : random(0));
    // The window record.
    setWord32(0x9f38d0, oneIn(3) ? 0 : kWinaCom);
    setWord32(0x9f38d0 + 0x1c, oneIn(4) ? 16 : 8);
    setWord32(0x9f38d0 + 0x454, oneIn(3) ? 0 : 0x1000);
    g_memory[0x9f38d0 + 0x45d] = x86::reg8(random(2));
    setWord32(0x9f38d0 + 0x43c, random(2));

    // The clip the mouse is held in, and where it last was.
    setWord32(0x9f4f4c, 100);
    setWord32(0x9f4f44, 300);
    setWord32(0x9f4f50, 50);
    setWord32(0x9f4f40, 200);
    setWord32(0x9f4f3c, 100 + random(3));
    setWord32(0x9f4f38, 50 + random(3));

    // Objects that are read and written, and DirectDraw's one.
    setWord32(kWinaCom, kWinaVtable);
    setWord32(kWinaVtable, kWinaQuery);
    setWord32(kWinaVtable + 8, kWinaRelease);
    setWord32(kWinaVtable + 0x18, kWinaMakeSurface);
    setWord32(kWinaVtable + 0x50, kWinaCoop);
    setWord32(kWinaTextPlace, kWinaText);
    for (x86::reg32 i = 0; i < 8; ++i)
        g_memory[kWinaText + i] = x86::reg8(i == 7 ? 0 : random(0x100) | 1);
    std::memcpy(g_memory + kWinaName, "name.dat", 9);
    std::memcpy(g_memory + kWinaNameBar, "na|me.dat", 10);
    setWord32(kWinaSurface + 0x54, oneIn(2) ? 0 : 0x4448);
    g_memory[kWinaSurface + 0x1e] = x86::reg8(oneIn(3) ? 0 : oneIn(2) ? 0xff : random(0x100));
    setWord32(kWinaSurface + 0x34, oneIn(2) ? 0 : 0x4449);
    setWord32(kWinaTable + 8, random(5));
    for (x86::reg32 i = 0; i < 8; ++i)
        setWord32(kWinaTable + 0x10 + 8 * i, random(0));
    // A shape's chain, to 0 or to the hot spots.
    const x86::reg32 links = 1 + random(3);
    for (x86::reg32 i = 0; i < links; ++i)
    {
        const bool lastOne = i + 1 == links;
        const x86::reg32 type = lastOne && oneIn(2) ? 0x7c : random(0x7c);
        setWord32(kWinaShape + 0x10 * i, type | (lastOne ? 0 : 0x10u << 8));
    }
    // The chunks of a movie's channel.
    setWord32(kWinaFrames, 0x6d44414d);
    setWord32(kWinaFrames + 4, oneIn(2) ? 0x6544414d : 0x6b44414d);
    setWord32(kWinaFrames + 8, 0x1234);
    setWord32(kWinaFrames + 12, 0x5678);
    // A block of chunks in memory.
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        setWord32(kWinaBlock + 0x10 * i, 0x100 + i);
        setWord32(kWinaBlock + 0x10 * i + 4, 0x10);
    }
}
