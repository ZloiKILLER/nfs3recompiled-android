// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
switch (c.address)
{
case 0x4554e0:
    entry.eax = random(2);
    entry.edx = random(8);
    break;
case 0x455530:
    entry.eax = random(4) == 0 ? 0 : 0x6647b0 + 0x9ac0 * random(2) + 0x9ac * random(16);
    entry.edx = 0x5f3f80 + 4 * random(4);
    entry.ebx = 0x5f3f90 + 4 * random(4);
    break;
case 0x4557b0:
    entry.eax = 0x6647b0 + 0x9ac0 * random(2) + 0x9ac * random(16);
    entry.edx = random(64);
    entry.ebx = random(6);
    break;
case 0x455840:
    entry.eax = 0x5f3a80;
    entry.edx = 0x5f3a84;
    break;
case 0x455c10:
    entry.eax = random(2);
    entry.edx = random(10);
    entry.ebx = random(3);
    break;
case 0x455cd0:
    entry.eax = random(2);
    entry.edx = random(20);
    entry.ebx = random(2) ? 0 : random(64);
    entry.ecx = random(6);
    break;
case 0x455dd0:
    entry.eax = random(2);
    entry.edx = random(20);
    entry.ebx = random(10);
    entry.ecx = random(3);
    setWord32(entry.esp + 4, random(2) ? 0 : random(64));
    setWord32(entry.esp + 8, random(6));
    break;
case 0x44fa90:
    entry.eax = 0x677100;
    entry.edx = 0x677400;
    entry.ebx = 0x677800;
    break;
case 0x4cef00:
    entry.eax = (random(0) & 0xffff0000) | random(20);
    break;
case 0x44f670:
    entry.edx = random(3) == 0 ? random(0) : random(14);
    break;
case 0x44efd0:
    // Its table of tracks' names has ten; past them the code reads its own frame.
    setWord32(0x6fd4ac, random(10));
    break;
case 0x442580:
    // Whether the button is down, in the function's frame.
    setWord32(entry.esp - 24 - 0x14, random(2));
    break;
case 0x4461d0: case 0x4462e0: case 0x446420: case 0x4465e0: case 0x446800: case 0x4466a0:
    entry.eax = 0x5f3b00;
    break;
default:
    break;
}
