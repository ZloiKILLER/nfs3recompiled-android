// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
switch (c.address)
{
case 0x44e7f0:
    entry.eax = random(13);
    entry.edx = random(2);
    break;
case 0x44e820:
{
    // One participant with the player's id, among the first places of the sort.
    entry.eax = random(2);
    setWord32(0x677d30, 2 + random(15));
    setWord32(0x6fd3b8, random(4));
    if (random(4))
        setWord32(0x55d020, word32(0x6fd4ac));
    for (x86::reg32 i = 0; i < 16; ++i)
        setWord32(0x6647b0 + 0x9ac * i + 0x1f0, 100 + i);
    setWord32(0x6647b0 + 0x9ac * random(4) + 0x1f0, entry.eax == 1 ? 1 : word32(0x6fd4f4));
    break;
}
case 0x44ea60:
    entry.eax = random(6);
    break;
case 0x44f6e0:
    entry.eax = 0x5f3a90;
    entry.edx = 0x5f3a94;
    setWord32(0x5f3a90, word32(0x55940c) + 4 + 0x31c * random(10));
    setWord32(0x5f3a94, word32(0x55940c) + 4 + 0x31c * random(10));
    break;
case 0x44f830:
    entry.eax = random(8);
    break;
case 0x44f8d0:
    entry.eax = random(13);
    entry.edx = word32(0x559408) != 0 && random(3) == 0 ? random(0) : random(14);
    break;
case 0x44f910:
    entry.eax = x86::reg32(random(16)) - 2;
    break;
case 0x44f960:
    entry.eax = random(6);
    break;
case 0x44fa10:
    for (x86::reg32 at = entry.esp + 4; at < entry.esp + 4 + 0x31c; at += 4)
        setWord32(at, random(0));
    break;
case 0x44fb30:
    entry.eax = 0x5f3c00 + 16 * random(16);
    entry.edx = 0x5f3c00 + 16 * random(16);
    entry.ebx = 0x5f3c00 + 16 * random(16);
    setWord32(entry.esp + 4, random(2));
    break;
case 0x44fc70:
    entry.edx = random(2);
    break;
case 0x450a90:
{
    static const x86::reg32 kKeys[] = { 0x0d, 0x0d, 0x4d00, 0x4b00, 0x4900, 0x5100, 0x20 };
    entry.eax = poolItem(random(kPoolItems));
    entry.ebx = (random(0) & 0xffff0000) | kKeys[random(7)];
    break;
}
case 0x450fe0:
case 0x455020:
case 0x4555c0:
case 0x455720:
    entry.eax = random(8) == 0 && c.address == 0x455020 ? 0 : 0x6647b0 + 0x9ac0 * random(2) + 0x9ac * random(16);
    entry.edx = 0x7e8000;
    break;
case 0x452090:
case 0x452120:
{
    // A text of up to 40 letters, short of the buffer it is copied into.
    const x86::reg32 text = 0x5f3a80 + 16;
    const x86::reg32 length = random(40);
    for (x86::reg32 i = 0; i < length; ++i)
        g_memory[text + i] = x86::reg8('a' + random(26));
    g_memory[text + length] = 0;
    entry.eax = text;
    if (c.address == 0x452090)
    {
        entry.edx = random(100);
        entry.ebx = random(8);
    }
    else
    {
        setWord32(entry.esp + 4, random(6));
        setWord32(entry.esp + 8, random(0));
        setWord32(entry.esp + 12, random(100));
    }
    break;
}
case 0x452420:
{
    const x86::reg32 code = random(21);
    entry.ebx = word32(0x5594e4 + 4 * code);
    entry.edx = 0x6645f0 + 4 * code;
    entry.eax = random(2) ? g_memory[entry.ebx + word32(entry.edx)] - 9u : 'A' + random(26);
    break;
}
case 0x4524e0:
{
    static const char kLetters[] = "ABGO012G";
    entry.eax = random(10) == 0 ? 0 : x86::reg32(std::uint8_t(kLetters[random(8)]));
    setWord32(0x6fd3b0, random(4) == 0 ? 2 : random(2));
    setWord32(0x6fd3b8, random(4) == 0 ? random(4) : random(2) * 3);
    break;
}
case 0x4534d0:
    entry.eax = 0x5f3f00;
    break;
case 0x455410:
    // Not a replay, and now and then two players side by side.
    setWord32(0x6fd3a0, random(8) == 0);
    setWord32(0x6fd3b0, random(4) == 0 ? 2 : random(4) == 0 ? 0 : 1);
    setWord32(0x6fd3b8, random(4));
    setWord32(0x5efd9c, 2 + random(7));
    if (random(2))
        setWord32(0x55d020, word32(0x6fd4ac));
    setWord32(0x7e1000 + 0x1f0, 1);
    setWord32(0x7e1000 + 0x200, 0xffffff01);
    break;
case 0x455210:
case 0x455280:
    entry.eax = 0x6647b0;
    entry.edx = 0x677d30;
    break;
case 0x455570:
case 0x455600:
case 0x455660:
case 0x4556c0:
    entry.eax = random(6) == 0 ? 0 : 0x6647b0 + 0x9ac * random(16);
    entry.edx = 0x7e8000;
    break;
case 0x457340:
    entry.eax = x86::reg32(random(11)) - 2;
    break;
case 0x4575a0:
{
    static const x86::reg32 kKeys[] = { 0x0d, 0x0d, 0x0d, 0x4d00, 0x4b00, 0x20, 0x1b };
    entry.eax = poolItem(random(kPoolItems));
    entry.ebx = (random(0) & 0xffff0000) | kKeys[random(7)];
    break;
}
default:
    break;
}
