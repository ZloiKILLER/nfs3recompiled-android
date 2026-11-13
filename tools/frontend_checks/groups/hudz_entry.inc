// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    const x86::reg32 item = 0x79e000 + 0xc0 * random(16);
    // The item in another's way.
    const x86::reg32 otherItem = 0x79e000 + 0xc0 * random(16);
    switch (c.address)
    {
    case 0x459670:
    {
        // A rectangle mostly inside its half of the screen, now and then out of it; every kind of element.
        const float top = random(4) == 0 ? float(random(1200)) / 1000.0f - 0.1f : float(random(100)) / 1000.0f + (random(2) ? 0.5f : 0.0f);
        const float left = random(4) == 0 ? float(random(1200)) / 1000.0f - 0.1f : float(random(800)) / 1000.0f;
        setFloat(item + 0x40, top);
        setFloat(item + 0x44, left);
        setFloat(item + 0x48, top + float(random(400)) / 1000.0f);
        setFloat(item + 0x4c, left + float(random(200)) / 1000.0f);
        if (random(16) == 0)
            setWord32(item + 0x40 + 4 * random(4), random(0));
        // A NaN that passes the test's bounds, its sign making its bits negative.
        if (random(8) == 0)
            setWord32(item + 0x40 + 4 * random(4), 0xffc00000);
        // The two locals a pass with nothing of its own compares: NaNs now and then, both or one.
        if (random(4) == 0)
        {
            setWord32(entry.esp - 0x18, 0xffc00000);
            setWord32(entry.esp - 0x14, random(4) == 0 ? 0x3f000000 : 0x7fc00000);
        }
        entry.eax = item + 0x40;
        entry.edx = random(2);
        // Every kind; the speed's two often, which have second parts to weigh.
        entry.ebx = random(8) == 0 ? random(40) : random(3) == 0 ? random(2) : random(23);
        entry.ecx = random(2) ? 0 : random(2) ? 1 : random(0);
        setWord32(entry.esp + 4, 0x79f800 + 4 * random(16));
        break;
    }
    case 0x459db0:
    {
        const x86::reg32 pick = random(16);
        const x86::reg32 kind = pick == 0 ? x86::reg32(-1) - random(4) : pick == 1 ? 46 + random(8) : pick < 8 ? 17 + random(2) * 5 + random(2) * 23 : random(46);
        setWord32(item + 0x3c, kind);
        // Pixels on the screen, or anything.
        setWord32(item + 4, random(2) ? random(640) << 16 : random(0));
        setWord32(item + 8, random(2) ? random(480) : random(0));
        setWord32(item + 0x60, random(4) == 0 ? random(0) : random(640));
        setWord32(item + 0x64, random(4) == 0 ? random(0) : random(480));
        setWord32(item + 0x74, random(2) ? 0 : otherItem);
        // What the stack holds where the test may not write, read as an item.
        setWord32(entry.esp - 24 - 0x10, otherItem);
        entry.eax = item;
        break;
    }
    case 0x4808b0:
    case 0x480a00:
    case 0x481290:
    case 0x481510:
    case 0x4818b0:
    case 0x481f90:
    case 0x482190:
        entry.eax = random(3);
        break;
    case 0x4815d0:
        entry.eax = random(2) ? 0 : random(3);
        break;
    case 0x481170:
        entry.eax = random(3);
        entry.edx = random(2);
        entry.ebx = 0x79d000 + 0x80 * random(8);
        break;
    case 0x482030:
        entry.eax = random(4) == 0 ? x86::reg32(-1) - random(3) : random(8);
        entry.edx = random(3);
        entry.ebx = random(2) ? 0 : random(0);
        break;
    case 0x49bc80:
        entry.eax = 0x900000 + 0x10 * random(16);
        break;
    case 0x49bd10:
        entry.eax = 0x79f000 + 0x20 * random(4);
        entry.edx = random(0x100);
        // Where they go: the queue's own count, now and then.
        entry.ebx = random(6) == 0 ? entry.eax + 8 : 0x79f800 + 4 * random(16);
        entry.ecx = random(6) == 0 ? entry.eax + 8 : 0x79f900 + 4 * random(16);
        break;
    default:
        break;
    }
}
