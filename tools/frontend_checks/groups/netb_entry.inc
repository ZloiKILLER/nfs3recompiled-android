// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    constexpr x86::reg32 kSession = 0xb00000, kNet = 0xb01000, kMessage = 0xb04000, kFrom = 0xb05000;
    constexpr x86::reg32 kQueue = 0xb07800, kPieces = 0xb07a00, kConnection = 0xb09f00, kBuffer = 0xb09800;
    const auto bigEndian = [](x86::reg32 v) {
        return (v << 24) | ((v & 0xff00) << 8) | ((v >> 8) & 0xff00) | (v >> 24);
    };
    switch (c.address)
    {
    case 0x5133a0:
    {
        static const x86::reg32 kTypes[] = {
            0xea616464, 0xea626164, 0xea627965, 0xea646174, 0xea646563, 0xea676574, 0xea6e6f21,
            0xea6f6c61, 0xea6f706e, 0xea6f7574, 0xea706c79, 0xea726571, 0xea727370, 0xea737276,
        };
        const x86::reg32 type = random(16) == 0 ? random(0) : kTypes[random(14)];
        entry.eax = kSession;
        entry.edx = kFrom;
        entry.ebx = type;
        entry.ecx = kMessage;
        setWord32(entry.esp + 4, random(0x200));
        if ((type == 0xea726571 || type == 0xea737276) && random(5) == 0)
            setWord32(kSession + 0x10, 0);
        // An id that is the session's, or none, or another.
        const x86::reg32 pick = random(3);
        setWord32(kMessage, bigEndian(pick == 0 ? 0 : pick == 1 ? word32(kSession) : random(8)));
        // The player asked for, or -1 for the host; the verdict of an answer, 1 often.
        const x86::reg32 which = random(3) == 0 ? 0xffffffff : random(4) == 0 ? 1 : random(9);
        setWord32(kMessage + 0x18, bigEndian(which));
        (void)kNet;
        break;
    }
    case 0x512180:
    {
        // A text short or long, or none; an address or none; data of any length or none.
        constexpr x86::reg32 kText = 0xb07000;
        const x86::reg32 length = random(2) ? random(8) : random(0x60);
        for (x86::reg32 i = 0; i < length; ++i)
            g_memory[kText + i] = x86::reg8('A' + random(26));
        g_memory[kText + length] = 0;
        entry.eax = random(5) == 0 ? 0 : kText;
        entry.edx = random(4) == 0 ? 0 : kFrom;
        entry.ebx = random(4) == 0 ? 0 : kMessage;
        entry.ecx = random(2) ? random(0x10) : random(0x80);
        setWord32(entry.esp + 4, random(2) ? 0 : random(0));
        setWord32(entry.esp + 8, random(0));
        break;
    }
    case 0x4f6350:
    {
        constexpr x86::reg32 kOwner = 0xb0b000;
        for (x86::reg32 i = 0; i < 0x1c; i += 4)
            setWord32(kOwner + 0x8c + i, word32(0x9f2628 + i));
        entry.eax = kOwner;
        entry.edx = random(3) == 0 ? 0 : kMessage;
        break;
    }
    case 0x4f6570:
    {
        constexpr x86::reg32 kSets = 0xb0a000;
        setWord32(kSets, 0xb09000);
        setWord32(kSets + 4, random(2) ? random(4) : 0x3e + random(4));
        setWord32(kSets + 0x108, random(2) ? random(4) : 0x3e + random(4));
        setWord32(kSets + 0x20c, random(2) ? random(4) : 0x3e + random(4));
        setWord32(kSets + 0x310, random(8));
        setWord32(0xb09000 + 0x38, random(3) == 0 ? 0xb08000 : 0);
        entry.eax = 0xb08000;
        setWord32(0xb08000 + 0x10, kBuffer);
        entry.edx = kSets;
        break;
    }
    case 0x4f6640:
        entry.eax = 0xb09000;
        setWord32(kConnection + 0x10, kBuffer);
        entry.edx = kConnection;
        break;
    case 0x4f67b0:
    case 0x4f7030:
    case 0x4f71a0:
        entry.eax = kConnection;
        setWord32(kConnection + 0x10, kBuffer);
        entry.edx = c.address == 0x4f67b0 ? kConnection : kMessage;
        entry.ebx = random(0x40);
        break;
    case 0x4f6870:
    case 0x4f6c50:
        entry.eax = c.address == 0x4f6c50 && random(8) == 0 ? 0 : kConnection;
        entry.ebx = random(0x40);
        break;
    case 0x4f7220:
    {
        for (x86::reg32 i = 0; i < 16; ++i)
            g_memory[kMessage + 0x40 + i] = g_memory[kMessage + i];
        if (random(4) != 0)
            g_memory[kMessage + 0x40 + random(16)] ^= 1;
        entry.eax = kMessage;
        entry.edx = kMessage + 0x40;
        break;
    }
    case 0x4f7280:
    case 0x4f7320:
    {
        constexpr x86::reg32 kNet2 = 0xb0b400, kTable = 0xb07b40;
        setWord32(kNet2 + 8, kTable);
        setWord32(kNet2 + 0xc, random(4) == 0 ? 0 : random(0));
        setWord32(kNet2 + 0xa0, random(2));
        setWord32(kTable + 0x14, random(2) ? 0x7e0804 : 0x7e0810);
        setWord32(kTable + 0x18, 0x7e0810);
        setWord32(kTable + 0x1c, random(2) ? 0x7e0808 : 0x7e0810);
        setWord32(kTable + 0x20, 0x7e0810);
        setWord32(kTable + 0x44, 0x7e0810);
        for (x86::reg32 i = 0; i < 16; ++i)
            g_memory[kMessage + i] = g_memory[kTable + 4 + i];
        if (random(4) == 0)
            g_memory[kMessage + random(16)] ^= 1;
        entry.eax = kNet2;
        entry.edx = kFrom;
        entry.ebx = c.address == 0x4f7280 ? kMessage : entry.ebx;
        setWord32(entry.esp + 4, kMessage + 0x20);
        setWord32(entry.esp + 8, kMessage + 0x40);
        break;
    }
    case 0x4f7380:
        entry.edx = random(3) == 0 ? 0 : kMessage;
        break;
    case 0x4f9a50:
        entry.eax = random(6);
        break;
    case 0x4fa030:
        entry.eax = random(4) == 0 ? 0 : kMessage;
        break;
    case 0x4fa500:
    case 0x4fa530:
    case 0x4fa730:
    case 0x4fa760:
        entry.eax = 0xb0b800;
        entry.edx = kMessage;
        entry.ebx = random(0x40);
        break;
    case 0x4fa310:
    case 0x4fa450:
    case 0x4fa480:
    case 0x4fa4e0:
    case 0x4fa4f0:
    case 0x4fa620:
    case 0x4fa6d0:
    case 0x4fa6e0:
    case 0x4fa710:
    case 0x4fa720:
        entry.eax = 0xb0b800;
        break;
    case 0x4fa550:
    case 0x4fa780:
        entry.eax = random(5) == 0 ? 0 : 0xb0b800;
        entry.ebx = random(2) ? 0xffffffff : random(4);
        break;
    case 0x4fa5f0:
        entry.eax = x86::reg32(random(8)) - 2;
        break;
    case 0x512890:
    {
        g_memory[kMessage + 0xc] = 0x04, g_memory[kMessage + 0xd] = x86::reg8(random(4) == 0 ? 0 : 0x52);
        g_memory[kFrom] = 0, g_memory[kFrom + 1] = x86::reg8(random(4) == 0 ? 1 : 2);
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            g_memory[kFrom + 2 + 0x40 * i] = 0x08;
            g_memory[kFrom + 3 + 0x40 * i] = x86::reg8(random(2) ? 0xbe : 0xbf);
        }
        entry.edx = kMessage;
        entry.ebx = kFrom;
        entry.ecx = 2 + 0x40 * random(5) + random(0x40);
        break;
    }
    case 0x512940:
        entry.eax = kFrom;
        break;
    case 0x512980:
        g_memory[kMessage] = 0, g_memory[kMessage + 1] = x86::reg8(random(3) ? 1 : 2);
        g_memory[kMessage + 2] = x86::reg8(random(2) ? 0x08 : 0xff), g_memory[kMessage + 3] = x86::reg8(random(2) ? 0xbe : 0xff);
        entry.eax = kMessage;
        break;
    case 0x5129e0:
        entry.eax = kMessage;
        break;
    case 0x512c80:
        entry.edx = kFrom;
        entry.ebx = kMessage;
        break;
    case 0x512f60:
        entry.eax = kMessage;
        entry.edx = 0xb07000;
        break;
    case 0x513190:
    {
        constexpr x86::reg32 kGame2 = 0xb0b400, kTable = 0xb07b40;
        setWord32(kGame2 + 8, kTable);
        setWord32(kTable + 0x48, random(2) ? 0 : 0x7e0810);
        setWord32(kTable + 0x54, 0x7e0800);
        setWord32(kSession + 4, random(4) == 0 ? 1 : 0);
        entry.eax = kGame2;
        entry.edx = kSession;
        entry.ecx = kMessage;
        setWord32(entry.esp + 4, random(0x100));
        setWord32(entry.esp + 8, random(0x100));
        break;
    }
    case 0x50dcd0:
        entry.eax = kQueue;
        entry.edx = random(2) ? 0 : random(0);
        break;
    case 0x50dd60:
        entry.eax = kQueue;
        entry.edx = random(2) ? 0 : random(0);
        entry.ecx = 0xb07900;
        setWord32(0xb07900 + 0x18, random(4) == 0 ? 0 : random(0));
        break;
    case 0x50de50:
    case 0x50ded0:
    case 0x50e0a0:
        entry.eax = kQueue;
        entry.edx = random(5) == 0 ? 0 : 0xb07e00;
        break;
    case 0x50df60:
    case 0x50dff0:
    case 0x50e2d0:
        entry.eax = kQueue;
        break;
    case 0x50e3f0:
        entry.eax = kQueue;
        entry.edx = random(5) == 0 ? 0 : 0x7e0804;
        break;
    case 0x50e500:
        entry.eax = kQueue;
        entry.edx = random(3);
        break;
    case 0x50e8e0:
    case 0x50e9b0:
        entry.eax = kQueue;
        entry.edx = 0x7e0804;
        break;
    case 0x512470:
        entry.eax = kConnection;
        entry.edx = random(4);
        entry.ebx = kPieces;
        break;
    case 0x5125b0:
        entry.eax = kConnection;
        entry.edx = x86::reg32(random(5)) - 1;
        entry.ebx = random(4);
        entry.ecx = kPieces;
        setWord32(entry.esp + 4, kQueue);
        break;
    case 0x512650:
        entry.eax = kConnection;
        entry.edx = random(4);
        entry.ebx = kPieces;
        entry.ecx = kQueue;
        break;
    case 0x512660:
        entry.eax = kConnection;
        break;
    case 0x5126f0:
        entry.eax = kConnection;
        entry.edx = kMessage;
        entry.ebx = random(0x40);
        break;
    case 0x513000:
        entry.eax = random(4) == 0 ? 0 : random(0) >> random(32);
        break;
    case 0x513030:
        entry.eax = 0xb03000;
        entry.edx = kFrom;
        entry.ebx = random(3) == 0 ? 0 : random(0);
        break;
    case 0x5132a0:
        entry.eax = 0xb03000;
        entry.edx = kFrom;
        break;
    case 0x513320:
        entry.eax = 0xb03000;
        entry.edx = kMessage;
        break;
    default:
        break;
    }
}
