// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
if (c.address == 0x49c1c0)
{
    // A list's top around the pointer, its lines of some height: 0 and -1 only
    // an impossible world's (the game hands 0x10).
    entry.eax = random(3) == 0 ? random(0) : x86::reg32(random(60)) - 10;
    entry.edx = 1 + random(32);
}
if (c.address == 0x49c240)
{
    entry.eax = x86::reg32(random(60)) - 10;
    entry.edx = x86::reg32(random(60)) - 10;
    entry.ebx = random(5) == 0 ? random(0) : random(50);
    entry.ecx = random(5) == 0 ? random(0) : random(50);
}
if (c.address == 0x4a4c20)
{
    entry.edx = random(640);
    entry.ebx = random(480);
}
if (c.address == 0x49c240 && random(2))
{
    // On the rectangle's edges, where the comparisons turn.
    const std::int32_t pointerX = std::int32_t(word32(0x55e5e4)), pointerY = std::int32_t(word32(0x55e5e8));
    entry.edx = x86::reg32(pointerY - std::int32_t(random(3)));
    entry.ecx = x86::reg32(pointerY) - entry.edx + random(3) - 1;
    entry.eax = x86::reg32(pointerX - std::int32_t(random(3)));
    entry.ebx = x86::reg32(pointerX) - entry.eax + random(3) - 1;
}
if (c.address == 0x4a4c20)
{
    // The clock where it turns over, or anywhere in its round: the item's, always.
    const x86::reg32 clockAt = entry.eax + 0x46;
    const std::uint16_t clock = std::uint16_t(random(4) == 0 ? 0x3fe : random(0x400));
    std::memcpy(g_memory + clockAt, &clock, 2);
}
