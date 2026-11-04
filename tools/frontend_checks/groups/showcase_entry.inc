// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
// sub_454000 and sub_454210 write through eax and edx: two words of memory.
if (c.address == 0x454000 || c.address == 0x454210)
{
    entry.eax = 0x5f3f80;
    entry.edx = 0x5f3f84;
    // The two eras sub_454000 names apart, often enough to reach them.
    if (random(2))
        setWord32(0x55958c, 3 + random(2));
}
// The era the loop of sub_454210 ends on: outside 1 to 6, which the game never has (the
// screen opens on 1 and the loop wraps at 6), and with no page found it never comes round.
if ((c.address == 0x454210 || c.address == 0x454310) && (word32(0x55958c) < 1 || word32(0x55958c) > 6))
    setWord32(0x55958c, 1 + random(6));
// sub_454310 at the last era moves to a seventh, which that loop goes past for ever if no era
// has a page: the original's, for a car with no history at all.  Kept, not run.
if (c.address == 0x454310 && word32(0x55958c) >= 6)
    setWord32(0x55958c, 1 + random(5));
// What goes by sub_4ef270's answer: 0 now and then, as a name not found.
{
    const bool findsNothing = c.address == 0x454000 || c.address == 0x454210 || c.address == 0x454260
        || c.address == 0x454310 || c.address == 0x4543f0 || c.address == 0x454500 || c.address == 0x454690;
    const_cast<Spec&>(kSpecs.at(0x4ef270)).kind = findsNothing ? Kind::Tuning : Kind::Shape;
}
