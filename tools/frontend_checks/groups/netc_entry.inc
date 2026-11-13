// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const x86::reg32 netc = c.address;
    if (netc >= 0x5140e0 && netc < 0x515050)
    {
        entry.eax = kNetcConn;
        entry.edx = random(0);
        entry.ebx = random(0);
        entry.ecx = random(0);
        const x86::reg32 player = random(3) == 0 ? 0 : kNetcPlayer;
        if (netc == 0x5143d0)
            entry.edx = random(2);
        else if (netc == 0x5144b0)
            entry.edx = kNetcPlayer, entry.ebx = random(2) ? kNetcName : 0;
        else if (netc == 0x5145b0)
            entry.edx = player, entry.ebx = kNetcOut;
        else if (netc == 0x514650)
            entry.edx = kNetcInfo, entry.ebx = random(2) ? kNetcName : 0;
        else if (netc == 0x514820)
            entry.edx = kNetcInfo;
        else if (netc == 0x514850)
            entry.edx = random(2);
        else if (netc == 0x514af0)
            entry.edx = player, entry.ebx = kNetcFnCallback;
        else if (netc == 0x514dc0)
            entry.edx = kNetcPlayer;
        else if (netc == 0x514ec0)
            entry.edx = random(4) == 0 ? 0 : kNetcLink, entry.ecx = random(2) ? 0xffffffffu : random(8);
        else if (netc == 0x514f70)
        {
            setWord32(entry.esp + 4, random(0));
            setWord32(entry.esp + 8, kNetcOut);
            entry.edx = kNetcPlayer;
            entry.ecx = random(0x40);
            if (random(2))
                entry.ebx = random(2) ? 0xea6e6f21u : 0xea646174u;
        }
    }
    if (netc == 0x515050)
    {
        setWord32(0x568a34, random(3) == 0 ? random(0) : 0);
    }
    if (netc >= 0x51a470 && netc < 0x51bc70)
    {
        // A port's number: mostly one that can be open, now and then one out of the table's range.
        const x86::reg32 number = random(8) == 0 ? (random(2) ? 0xffffffffu : 16) : random(16);
        entry.eax = number;
        entry.edx = random(0x300);
        entry.ebx = kNetcOut;
        entry.ecx = random(0);
        if (netc == 0x51a470)
            entry.eax = random(16), entry.edx = random(200);
        else if (netc == 0x51a7b0)
            entry.eax = random(16), entry.edx = kNetcName, entry.ebx = random(2);
        else if (netc == 0x51a930)
            entry.eax = random(16);
        else if (netc == 0x51ab70)
            entry.eax = random(16), entry.edx = random(5);
        else if (netc >= 0x51abc0 && netc < 0x51aeb0)
            entry.eax = kNetcPorts + 0x450 * random(16);
        else if (netc == 0x51b2f0)
            entry.eax = random(3) == 0 ? 0xffffffffu : random(16), entry.edx = random(0);
        else if (netc == 0x51b710)
        {
            entry.edx = random(3) ? random(17) : 0x10000 + random(100);
            entry.ebx = random(5);
            entry.ecx = random(3);
            setWord32(entry.esp + 4, 3 + random(3));
        }
        else if (netc == 0x51b5f0 || netc == 0x51b530)
            entry.ebx = kNetcName;
        else if (netc == 0x51bbd0)
            entry.edx = random(8) == 0 ? 0x201 + random(100) : random(0x200), entry.ecx = random(1000);
    }
    if (netc >= 0x51bc70 && netc <= 0x51db00)
    {
        const bool reliable = netc >= 0x51c7a0;
        const x86::reg32 self = reliable ? kNetcRely : kNetcPlain;
        const x86::reg32 buffer = reliable ? kNetcRelyBuffer : kNetcPlainBuffer;
        const x86::reg32 packet = kNetcPackets + 0x200 * random(4);
        const x86::reg32 connection = random(8) == 0 ? 0 : kNetcLink;
        setWord32(kNetcLink + 0x10, random(8) == 0 && (netc == 0x51c690 || netc == 0x51d9e0) ? 0 : self);
        entry.edx = random(0);
        entry.ebx = random(0);
        entry.ecx = random(0);
        switch (netc)
        {
        case 0x51bc70: case 0x51bca0: case 0x51c7c0: case 0x51c7f0:
            entry.eax = buffer;
            entry.edx = random(0x20);
            break;
        case 0x51bcd0:
            entry.eax = kNetcFrames;
            entry.ebx = kNetcName;
            entry.ecx = random(32);
            break;
        case 0x51bcf0:
            entry.eax = kNetcFrames;
            entry.edx = self;
            break;
        case 0x51bd50:
            entry.eax = self;
            entry.edx = random(0x100);
            break;
        case 0x51be90: case 0x51cd50:
            entry.eax = self;
            entry.edx = kNetcOut;
            entry.ebx = buffer + random(3);
            entry.ecx = random(3) ? 6 + random(40) : 0x120;
            break;
        case 0x51bdc0: case 0x51bfd0: case 0x51c820: case 0x51cc80: case 0x51cf30:
            entry.eax = self;
            break;
        case 0x51c100: case 0x51d270:
            entry.eax = connection;
            entry.edx = kNetcOps;
            entry.ebx = random(4) == 0 ? random(6) : random(0x110);
            entry.ecx = x86::reg32(random(10)) - 1;
            setWord32(kNetcLink + 0x10, random(2) ? 0 : self);
            setWord32(kNetcLink + 0x24, random(6) == 0);
            break;
        case 0x51c510: case 0x51d810:
            entry.eax = connection;
            entry.edx = kNetcName;
            entry.ebx = random(0x40) + (random(4) == 0 ? 0x20 : 0);
            break;
        case 0x51c0a0: case 0x51c5c0: case 0x51c690: case 0x51d050: case 0x51d8f0: case 0x51d9e0:
            entry.eax = netc == 0x51c690 || netc == 0x51d9e0 ? connection : kNetcLink;
            break;
        case 0x51c7a0:
            entry.ebx = 0x7f;
            entry.eax = random(0x400);
            entry.edx = random(0x80);
            break;
        case 0x51c890:
            entry.eax = packet;
            entry.edx = self;
            entry.ebx = kNetcName;
            entry.ecx = random(32);
            break;
        case 0x51c8f0: case 0x51d040:
            entry.eax = packet;
            entry.edx = self;
            break;
        case 0x51ca50:
            entry.eax = packet;
            entry.edx = self;
            entry.ebx = random(2) ? 0xfb + random(5) : random(0x80);
            break;
        case 0x51caf0:
            entry.eax = random(6) == 0 ? 0 : packet;
            entry.edx = self;
            break;
        case 0x51cb50: case 0x51cc00:
            entry.eax = self;
            break;
        case 0x51d000: case 0x51d010:
            entry.eax = packet;
            break;
        }
    }
}
