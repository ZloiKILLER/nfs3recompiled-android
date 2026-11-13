// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const x86::reg32 player = random(2);
    const x86::reg32 view = kTailViews + 0x40 * player;
    switch (c.address)
    {
    case 0x4be240:
        entry.eax = random(3) == 0 ? 0 : kTailOut + 0x10 * random(16);
        break;
    case 0x4be3a0:
        entry.eax = random(2) ? kTailTasks : kTailTasksLate;
        break;
    case 0x4c8470:
    case 0x4c8550:
    {
        // At a place's end, or one off it, or anywhere about.
        const x86::reg32 place = kTailPlaces + 0x10 * random(6);
        const x86::reg32 mark = word32(place + 4 * random(2));
        entry.eax = random(4) == 0 ? random(100) : mark + random(3) - 1;
        entry.edx = kTailOut;
        break;
    }
    case 0x4c9a80:
    {
        const x86::reg32 node = random(32);
        const x86::reg32 lanes = g_memory[kTailNodes + node * 36 + 0x13];
        entry.eax = node;
        switch (random(4))
        {
        case 0:
            entry.edx = 6 - (lanes >> 4);
            break;
        case 1:
            entry.edx = (lanes & 0xf) + 7;
            break;
        default:
            entry.edx = random(16);
            break;
        }
        break;
    }
    case 0x4dd230:
    case 0x4dece0:
        entry.eax = view;
        entry.esi = kTailOutList;
        break;
    case 0x4dd330:
        entry.eax = random(4) == 0 ? 0 : random(1000);
        if (random(3) == 0)
            setWord32(0x7d3684, word32(0x8ca248) + word32(0x8ca1f0 + 4 * random(4)) + random(3) - 1);
        break;
    case 0x4dd3d0:
    {
        // At an end of the stretch or of a fade, or one off.
        const int first = int(std::int16_t(g_memory[0x8ca23c] | (g_memory[0x8ca23d] << 8))) + int(word32(0x8ca244));
        const int last = int(std::int16_t(g_memory[0x8ca23e] | (g_memory[0x8ca23f] << 8))) + int(word32(0x8ca244));
        const int fade = int(std::int16_t(g_memory[0x8ca240] | (g_memory[0x8ca241] << 8)));
        const int marks[] = { first, last, first + fade, last - fade, last + int(word32(0x5dd940)), last - fade + int(word32(0x5dd940)) };
        entry.eax = random(4) == 0 ? random(220) : x86::reg32(marks[random(6)] + int(random(3)) - 1);
        break;
    }
    case 0x4dd4c0:
        entry.eax = random(4) == 0 ? 0 : random(1000);
        entry.edx = player;
        if (random(3) == 0)
            setWord32(0x8ca234, x86::reg32(int(random(9)) - 4));
        break;
    case 0x4dd510:
        entry.eax = player;
        break;
    case 0x4dd5e0:
        entry.eax = view;
        entry.edx = random(6) == 0 ? x86::reg32(int(random(4)) - 2) : random(40);
        break;
    case 0x4eac60:
        entry.eax = random(2) ? random(0x400) : random(0);
        entry.edx = kTailOut;
        entry.ebx = random(8) == 0 ? kTailOut : kTailOut + 0x20;
        if (random(8) == 0)
            entry.edx = 0x5672ac + 4 * random(257);
        break;
    default:
        break;
    }
}
