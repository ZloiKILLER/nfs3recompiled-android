// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
if (c.address == 0x4c55a0)
{
    entry.eax = random(3) == 0 ? 0 : random(2) ? 1 : random(0);
    // Mostly a network race, and often with the hold free to ask for, or for the host.
    if (random(4) != 0)
    {
        setWord32(0x6fd3b0, 2 + random(3));
        setWord32(0x6fd3a0, random(4) == 0 ? 2 + random(2) : random(2));
        if (random(2) != 0)
        {
            setWord32(0x7d36c0, 0);
            setWord32(0x7d36c8, 0);
        }
    }
}
// The loop: often running, nothing in the way of its frames or the player's keys.
if (c.address == 0x4c4e40 && random(3) != 0)
{
    setWord32(0x7d369c, 0);
    setWord32(0x7d36a0, 0);
    setWord32(0x7d36a8, 0);
    setWord32(0x7d3688, word32(0x79c4ac) - random(3));
    g_memory[0x7a22dc] = x86::reg8(random(4) == 0);
    const x86::reg32 inpaMode = random(4);
    if (inpaMode < 2)
        setWord32(0x6fd3b0, 1);
    else if (inpaMode == 2)
        setWord32(0x6fd3b0, 2 + 2 * random(2));
    else
    {
        // One screen, showing another player than the views follow.
        setWord32(0x6fd3b0, 0);
        setWord32(0x6fd4f4, 1 + random(3));
        setWord32(0x6fd4f8, 0);
    }
    if (random(2) != 0)
        g_memory[0x55e5ed] = 2;
}
