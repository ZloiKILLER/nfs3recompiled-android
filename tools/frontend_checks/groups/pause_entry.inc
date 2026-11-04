// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    // A language's line and a car's name are read through, in these cases: a
    // record's memory, not any number at all.
    const x86::reg32 pauseAt = c.address;
    const bool readsText = pauseAt == 0x44cab0;
    const bool readsCarName = pauseAt == 0x478010 || pauseAt == 0x446550 || pauseAt == 0x4468c0
        || pauseAt == 0x447800 || pauseAt == 0x4478c0;
    const_cast<Spec&>(kSpecs.at(0x4d1850)).kind = readsText ? Kind::Shape : Kind::Plain;
    const_cast<Spec&>(kSpecs.at(0x439520)).kind = readsCarName ? Kind::Shape : Kind::Plain;
    // A save's name.
    if (pauseAt == 0x446510 || pauseAt == 0x446550 || pauseAt == 0x446740 || pauseAt == 0x4468c0
        || pauseAt == 0x446950 || pauseAt == 0x446a90)
        entry.eax = 0x5f3f20;
    if (pauseAt == 0x478010)
        entry.eax = 0x5f5100;
    if (pauseAt == 0x4beb00 || pauseAt == 0x4beb40 || pauseAt == 0x4beb20)
    {
        // A rectangle; for reading it, the four it goes through.
        entry.eax = 0x7cdac0 + 0x20 * random(4);
        setWord32(entry.esp + 4, pauseAt == 0x4beb20 ? 0x5f3e40 + 4 * random(8) : random(0));
        if (pauseAt == 0x4beb20)
        {
            entry.edx = 0x5f3e40 + 4 * random(8);
            entry.ebx = 0x5f3e40 + 4 * random(8);
            entry.ecx = 0x5f3e40 + 4 * random(8);
        }
    }
}
