// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kRecord = 0x920000, kText = 0x930000, kLine = 0x938000, kOut = 0x939000;
    // A line of values: numbers and now and then something that is none, short as the file's are.
    const auto valuesAt = [&](x86::reg32 at) {
        std::string line;
        const x86::reg32 numbers = 1 + random(random(2) ? 16 : 3);
        for (x86::reg32 n = 0; n < numbers; ++n)
        {
            const x86::reg32 pick = random(12);
            char piece[32];
            if (pick == 0)
                std::snprintf(piece, sizeof piece, "-%u", unsigned(random(1000)));
            else if (pick == 1)
                std::snprintf(piece, sizeof piece, "1a%u", unsigned(random(10)));
            else if (pick < 6)
                std::snprintf(piece, sizeof piece, "%u.%u", unsigned(random(1000)), unsigned(random(1000)));
            else
                std::snprintf(piece, sizeof piece, "%u", unsigned(random(100000)));
            line += piece;
            line += random(4) == 0 ? "\t" : ",";
        }
        if (random(8) == 0)
            line = "x" + line;
        std::memcpy(g_memory + at, line.c_str(), line.size() + 1);
    };
    switch (c.address)
    {
    case 0x4b0700:
    case 0x4b07b0:
    case 0x4b07d0:
    case 0x4b08b0:
        valuesAt(kLine);
        entry.eax = kLine;
        entry.edx = random(8);
        break;
    case 0x4b0900:
    {
        static const char kFirst[] = "0-9a.\0 Z";
        g_memory[kLine] = x86::reg8(kFirst[random(8)]);
        entry.eax = kLine;
        break;
    }
    case 0x4b0940:
    {
        // A key's name and number, by the table, or only one of them, or neither.
        const x86::reg32 field = random(0x50);
        const x86::reg32 name = word32(0x4af3b0 + 8 * field);
        std::string line = random(5) == 0 || name == 0 ? "nothing" : guestString(name);
        std::int16_t number;
        std::memcpy(&number, g_memory + 0x4af3b4 + 8 * field, 2);
        char tail[32];
        std::snprintf(tail, sizeof tail, "(%d)", int(number));
        if (random(4) != 0)
            line += tail;
        std::memcpy(g_memory + kLine, line.c_str(), line.size() + 1);
        entry.eax = kLine;
        entry.edx = kOut;
        break;
    }
    case 0x4b09d0:
    {
        const x86::reg32 size = 1 + random(60);
        for (x86::reg32 i = 0; i < size; ++i)
        {
            const x86::reg32 pick = random(12);
            g_memory[kText + i] = x86::reg8(pick == 0 ? '\r' : pick == 1 ? '\n' : pick == 2 ? 0 : pick == 3 ? ' ' : 'A' + random(40));
        }
        g_memory[kText + size] = 0;
        setWord32(0x55ed88, random(3) == 0 ? random(size + 4) : random(4) == 0 ? x86::reg32(0) - random(4) - 1 : 0);
        entry.eax = random(10) == 0 ? 0 : kText;
        entry.edx = kLine;
        entry.ebx = size;
        break;
    }
    case 0x4b0bb0:
        entry.eax = random(10) == 0 ? 0 : kRecord;
        entry.edx = random(5) == 0 ? 1 + random(0x10000) : 0;
        break;
    case 0x4b1380:
    {
        // Keys of the table each with its line of values, with blank lines between some.
        std::string file;
        const x86::reg32 keys = random(4) == 0 ? 0x50 : random(30);
        for (x86::reg32 k = 0; k < keys; ++k)
        {
            const x86::reg32 field = keys == 0x50 ? k : random(0x50);
            const x86::reg32 name = word32(0x4af3b0 + 8 * field);
            if (name == 0)
                continue;
            std::int16_t number;
            std::memcpy(&number, g_memory + 0x4af3b4 + 8 * field, 2);
            char tail[32];
            std::snprintf(tail, sizeof tail, "(%d)\r\n", int(number));
            file += guestString(name) + tail;
            if (random(12) == 0)
                continue;
            valuesAt(kLine);
            file += guestString(kLine) + (random(6) == 0 ? "\n\n" : "\r\n");
        }
        if (file.size() > 0x7000)
            file.resize(0x7000);
        std::memcpy(g_memory + kText, file.c_str(), file.size() + 1);
        entry.eax = kRecord;
        entry.edx = random(30) == 0 ? 0 : kText;
        entry.ebx = random(30) == 0 ? 0 : x86::reg32(file.size());
        entry.ecx = random(3) == 0 ? 0 : kOut;
        break;
    }
    case 0x4a3070:
    {
        static const x86::reg32 kCodes[] = { 0xc000008c, 0x80000003, 0x80000002, 0xc000008d, 0xc000008e, 0xc000008f, 0xc0000090, 0xc0000091,
                                             0xc0000092, 0xc0000093, 0xc000001d, 0xc0000006, 0xc0000094, 0xc0000095, 0xc0000026, 0xc0000025,
                                             0xc0000096, 0x80000004, 0xc00000fd, 0xc0000005, 0x80000001, 0xc0000024, 0xc00000fe };
        entry.eax = kCodes[random(23)];
        break;
    }
    case 0x4a5080:
    {
        constexpr x86::reg32 kWords = 0x941800, kArgv = 0x941c00;
        const x86::reg32 count = 1 + random(5);
        for (x86::reg32 i = 0; i < count; ++i)
        {
            const std::string word = random(3) == 0 ? "/x" + std::to_string(random(999)) : "-opt" + std::to_string(random(99)) + "xyz";
            std::memcpy(g_memory + kWords + 0x40 * i, word.c_str(), word.size() + 1);
            setWord32(kArgv + 4 * i, kWords + 0x40 * i);
        }
        entry.eax = count;
        entry.edx = kArgv;
        break;
    }
    case 0x4a54f0:
        entry.eax = kLine;
        entry.edx = random(4) == 0;
        break;
    case 0x4a5880:
    case 0x4a59c0:
        entry.eax = random(4);
        g_memory[0x7a2020] = 'a', g_memory[0x7a2021] = '/', g_memory[0x7a2022] = 'b', g_memory[0x7a2023] = '/', g_memory[0x7a2024] = 0;
        break;
    case 0x4f0280:
    case 0x4f0330:
        std::memcpy(g_memory + kLine, random(2) ? "c:\\games\\file.*" : "pattern", 16);
        entry.eax = kLine;
        break;
    case 0x47a970:
        entry.eax = random(0x10000);
        entry.edx = random(0x10000);
        break;
    case 0x4f0a10:
        setWord32(kLine + 0xc, random(0));
        entry.eax = kLine;
        break;
    case 0x4f0b60:
        std::memcpy(g_memory + kLine, random(4) ? "text" : "", 5);
        entry.eax = kLine;
        entry.edx = random(0x140);
        entry.ebx = kOut;
        entry.ecx = kOut + 4;
        setWord32(entry.esp + 4, kOut + 8);
        setWord32(entry.esp + 8, kOut + 12);
        break;
    case 0x4f0c90:
        std::memcpy(g_memory + kLine, "text", 5);
        entry.eax = kLine;
        entry.edx = kOut;
        entry.ebx = kOut + 4;
        entry.ecx = kOut + 8;
        setWord32(entry.esp + 4, kOut + 12);
        break;
    case 0x4f0af0:
        entry.eax = random(200);
        entry.edx = random(200);
        for (x86::reg32 i = 0; i < 4; ++i)
            setWord32(entry.esp + 4 + 4 * i, random(100));
        break;
    case 0x4f0cb0:
        setWord32(kLine + 4, random(4));
        entry.eax = kLine;
        entry.edx = random(4);
        break;
    case 0x4f0d20:
        // What is on this queue has its callback: that is how it came there.
        for (x86::reg32 r = 0; r < 16; ++r)
            setWord32(0x948000 + 0x40 * r + 0x24, 0x7e0200);
        break;
    case 0x4f0da0:
        entry.eax = 1 + random(4);
        break;
    case 0x4f1370:
        std::memcpy(g_memory + kLine, "file.dat", 9);
        entry.eax = random(3) == 0 ? 0 : kLine;
        break;
    case 0x4f1790:
        entry.eax = random(64);
        break;
    case 0x4f2170:
        entry.eax = x86::reg32(0) - random(3);
        break;
    case 0x4f06e0:
        entry.eax = random(5);
        break;
    case 0x4f4650:
        std::memcpy(g_memory + kLine, "entry", 6);
        entry.eax = kLine;
        break;
    case 0x4f46d0:
    case 0x4f4700:
    case 0x4f4740:
    case 0x4f4760:
        for (x86::reg32 at = 0; at < 0xb8; at += 4)
            setWord32(kOut + at, random(0));
        entry.eax = kOut;
        break;
    case 0x4f4780:
        for (x86::reg32 i = 0; i < 64; ++i)
            g_memory[kLine + i] = x86::reg8(random(256));
        entry.eax = kLine;
        entry.edx = random(64);
        break;
    case 0x4f47c0:
        g_memory[kLine + 3] = x86::reg8(random(2) ? 2 : random(256));
        setWord32(kLine + 4, 4 * random(8));
        for (x86::reg32 i = 0; i < 8; ++i)
            setWord32(kLine + 0x10 + 4 * i, random(2) ? 0x444e4542 : random(0));
        entry.eax = kLine;
        break;
    case 0x4f4920:
    {
        for (x86::reg32 at = 0; at < 0x20; at += 4)
            setWord32(kLine + at, random(0));
        const x86::reg32 least = random(10);
        entry.eax = kLine;
        entry.edx = random(30);
        entry.ebx = random(0);
        entry.ecx = least;
        setWord32(entry.esp + 4, random(2) ? least : least + random(10));
        setWord32(entry.esp + 8, random(2));
        break;
    }
    case 0x4f4a80:
        setWord32(kLine, random(4) ? 0x57494e44 : random(0));
        for (x86::reg32 at = 4; at < 0x1c; at += 4)
            setWord32(kLine + at, random(4) ? random(0x4000) : random(0));
        g_memory[kLine + 0x1c] = x86::reg8(random(40));
        g_memory[kLine + 0x1d] = x86::reg8(random(256));
        entry.eax = kLine;
        break;
    case 0x4f87e0:
        std::memcpy(g_memory + kLine, random(2) ? "!driver" : "driver", 8);
        entry.eax = kLine;
        break;
    case 0x4f9320:
        break;
    case 0x4f9390:
        std::memcpy(g_memory + kLine, "dir\\", 5);
        std::memcpy(g_memory + kOut, "file", 5);
        entry.eax = kLine;
        entry.edx = kOut;
        entry.ebx = random(16);
        break;
    case 0x4f93f0:
        g_memory[kLine] = x86::reg8('a' + random(26));
        g_memory[kLine + 1] = x86::reg8(random(3) ? ':' : 'x');
        entry.eax = kLine;
        break;
    case 0x4f9410:
        entry.eax = random(26);
        break;
    case 0x4f99f0:
    case 0x4f9920:
        entry.edx = random(4);
        break;
    case 0x4fb710:
        entry.eax = random(4);
        entry.edx = random(2);
        break;
    case 0x4fb790:
        std::memcpy(g_memory + kLine, random(2) ? "name" : "", 5);
        for (x86::reg32 at = 0; at < 0x40; at += 4)
            setWord32(kOut + at, random(0));
        std::memcpy(g_memory + kOut + 0x14, "name", 5);
        setWord32(kOut + 0x100, kLine);
        setWord32(kOut + 0x104, random(0));
        entry.eax = kOut;
        entry.edx = kOut + 0x100;
        break;
    case 0x459fe0:
        entry.eax = 0x944000;
        entry.edx = 0x944100;
        break;
    case 0x45a120:
        entry.eax = 0x944000;
        break;
    default:
        break;
    }
}
