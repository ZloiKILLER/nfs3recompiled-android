// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
    const x86::reg32 a = c.address;
    // A line of text the function reads: a string of the script's, not an address made up.
    specs[0x4d1850].kind = a == 0x43f550 || a == 0x43ff50 || a == 0x43faf0 ? Kind::EditText : Kind::Plain;
    // Four buttons here, more than the Focus kind's three.
    specs[0x4446c0].kind = a == 0x43faf0 ? Kind::KeyHandler : Kind::Focus;
    // strcmp: now and then an answer of 0 or 1, the script's else any.
    specs[0x4ee310].kind = round % 2 ? Kind::Bool : Kind::Plain;
    if ((a >= 0x43dab0 && a <= 0x440960) || a == 0x451940 || a == 0x458450 || (a >= 0x49c0e0 && a <= 0x4c4310))
    {
        // The games found: none listed yet, or a short list; the screen's list of them.
        constexpr x86::reg32 kGames = 0x5f3b00, kShortString = 0x5f3a80, kPeer = 0x5f3a00;
        setWord32(0x5f3ff0, random(4) == 0 ? 0 : kGames);
        g_memory[kGames] = x86::reg8(random(3) == 0 ? 0 : random(12));
        g_memory[kGames + 1] = 0;
        // What the list's stand-in hands back: the shapes' records, a short list each.
        for (x86::reg32 k = 0; k < 16; ++k)
        {
            g_memory[0x5f3c00 + 16 * k] = x86::reg8(random(3) == 0 ? 0 : random(12));
            g_memory[0x5f3c01 + 16 * k] = 0;
        }
        setWord32(0x5556c8, maybeItem(3));
        setWord32(0x79c1a4, random(0));
        setWord32(0x5f3fec, random(2) ? word32(0x79c1a4) + random(0x1000) - 0x800 : random(0));
        // The modems, their count on both sides of 1, the one chosen.
        setWord32(0x5f3df0, random(4));
        setWord32(0x6fbb5c, random(5));
        setWord32(0x6fbb58, random(6));
        setWord32(0x5f3fe8, random(4));
        g_memory[0x55eb2e] = x86::reg8(random(4) == 0 ? 0x10 : 0);
        // The connections, the games and the peers, in their tables' bounds.
        setWord32(0x55e718, random(9));
        setWord32(0x55e71c, random(4));
        setWord32(0x55e720, random(2));
        setWord32(0x55e864, random(2) ? 0 : random(0));
        for (x86::reg32 i = 0; i < 7; ++i)
        {
            setWord32(0x55e734 + 44 * i, random(4));
            setWord32(0x55e738 + 44 * i, random(2));
            setWord32(0x7a02e8 + 4 * i, random(2) ? 0 : 0x5f3e00 + 4 * i);
        }
        g_memory[0x55e858] = x86::reg8(random(4) == 0 ? 20 + random(3) : random(6));
        g_memory[0x55e859] = 0;
        setWord32(0x55e85c, 0x79fce0);
        setWord32(0x55e710, random(3) == 0);
        setWord32(0x7a01e0, random(9));
        setWord32(0x7a01e8, random(6));
        setWord32(0x7a0434, random(2));
        g_memory[0x7a2296] = x86::reg8(random(2)), g_memory[0x7a2297] = 0;
        g_memory[0x7a22a2] = x86::reg8(random(2)), g_memory[0x7a22a3] = 0;
        g_memory[0x7a22da] = x86::reg8(random(2)), g_memory[0x7a22db] = 0;
        g_memory[0x7a22dc] = x86::reg8(random(2)), g_memory[0x7a22dd] = 0;
        g_memory[0x564819] = 0;
        setWord32(0x7a22e0, random(4));
        setWord32(0x6fd3b0, random(5));
        setWord32(0x7d3654, random(2) ? word32(0x79c1a4) : random(0));
        // Strings the functions copy.
        for (const x86::reg32 at : { kShortString, kPeer + 0x48, 0x7d3658u, 0x6fd2fcu })
        {
            const x86::reg32 length = random(9);
            for (x86::reg32 i = 0; i < length; ++i)
                g_memory[at + i] = x86::reg8('a' + random(26));
            g_memory[at + length] = 0;
        }
        for (x86::reg32 i = 0; i < 20; ++i)
        {
            const x86::reg32 length = random(14);
            for (x86::reg32 k = 0; k < length; ++k)
                g_memory[0x79fce0 + 64 * i + k] = x86::reg8('a' + random(26));
            g_memory[0x79fce0 + 64 * i + length] = 0;
        }
        switch (a)
        {
        case 0x43dab0:
            entry.edx = 0x5f3e00 + 4 * random(16);
            break;
        case 0x49c0e0:
            entry.eax = x86::reg32(random(30)) - 5;
            break;
        case 0x49e360:
            entry.eax = random(5);
            break;
        case 0x49e3d0:
            entry.ebx = random(4);
            entry.ecx = random(2) ? 0 : 0x43f1f0;
            break;
        case 0x49e5f0:
            entry.eax = kShortString;
            entry.edx = random(4);
            break;
        case 0x49ea70:
            entry.eax = random(5) ? kShortString : 0;
            entry.edx = 0x5f3e00;
            break;
        case 0x49efb0:
            entry.eax = kPeer;
            entry.edx = (entry.edx & 0xffff0000) | random(7);
            break;
        case 0x49f200:
            entry.eax = kPeer;
            entry.edx = kShortString;
            break;
        case 0x49f380:
            entry.edx = kShortString;
            entry.ecx = kPeer;
            break;
        case 0x49f470:
        case 0x49f4b0:
            entry.edx = kPeer;
            break;
        case 0x49f5c0:
            entry.eax = kPeer;
            break;
        case 0x49f6a0:
            entry.eax = kShortString;
            entry.edx = kPeer;
            break;
        case 0x4c41d0:
            entry.eax = random(8) ? 0x5f3100 : 0;
            break;
        default:
            break;
        }
    }
}
