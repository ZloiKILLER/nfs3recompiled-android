// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
switch (c.address)
{
case 0x4cedd0:
{
    // The format is %5.2f, which the check's sprintf does not read: its own
    // shows the double the code hands it.
    std::memcpy(g_memory + 0x5404cc, "%x%x", 5);
    const float kSpeeds[] = { 0.0f, 0.5f, 1.0f, 1.0000001f, 2.0f, 55.5f, 100.25f, -1.0f };
    if (random(8) == 0)
        setWord32(entry.esp + 4, random(0));
    else
        setFloat(entry.esp + 4, random(2) ? kSpeeds[random(8)] : float(random(100000)) / 137.0f);
    setWord32(entry.esp + 8, 0x5f3a00 + 4 * random(16));
    break;
}
case 0x4cee40:
    entry.eax = random(6) == 0 ? random(0) : random(4) == 0 ? x86::reg32(0) - random(1000000) : random(1000000);
    entry.edx = 0x5f3a00 + 4 * random(16);
    break;
case 0x4cf160:
    entry.eax = (random(0) & 0xffff0000) | (random(4) == 0 ? random(0x10000) : random(10));
    break;
case 0x4cf190:
    entry.eax = (random(0) & 0xffff0000) | (random(4) == 0 ? random(0x10000) : random(4));
    break;
case 0x4d7560:
    entry.eax = random(8);
    entry.edx = random(640);
    entry.ebx = random(4) == 0 ? x86::reg32(0) - random(200) : random(480);
    break;
case 0x4d7cc0:
    if (random(8) == 0)
    {
        setWord32(entry.esp + 4, random(0));
        setWord32(entry.esp + 8, random(0));
    }
    else
    {
        setFloat(entry.esp + 4, float(random(1000)) - 200.0f + float(random(4)) * 0.25f);
        setFloat(entry.esp + 8, float(random(700)) - 100.0f + float(random(4)) * 0.25f);
    }
    setWord32(entry.esp + 12, random(8) == 0 ? random(0) : random(256));
    break;
case 0x4d7d90:
    entry.eax = random(0);
    entry.edx = x86::reg32(random(800)) - 100;
    entry.ebx = x86::reg32(random(600)) - 100;
    entry.ecx = random(256);
    setWord32(entry.esp + 4, random(256));
    setWord32(entry.esp + 8, random(256));
    setWord32(entry.esp + 12, random(256));
    if (random(8) == 0)
        setWord32(entry.esp + 16, random(0));
    else
        setFloat(entry.esp + 16, someValue(g_random) * 3.0f);
    setWord32(entry.esp + 20, random(0));
    break;
case 0x4d88c0:
    for (x86::reg32 i = 0; i < 5; ++i)
        setWord32(entry.esp + 4 + 4 * i, random(0));
    break;
case 0x4d9180:
{
    entry.eax = random(0);
    const x86::reg32 kind = random(4);
    entry.edx = kind == 0 ? 0x100 : kind == 1 ? 0x40 : kind == 2 ? random(0x200) : random(0);
    break;
}
case 0x4daa10:
    entry.eax = random(0);
    break;
case 0x4dab10:
    // A race half the time, where only 9 of the kinds do anything.
    setWord32(0x559234, random(2));
    entry.eax = 0x5f3a00 + 4 * random(8);
    entry.edx = 0x5f3a40 + 4 * random(8);
    break;
default:
    break;
}
