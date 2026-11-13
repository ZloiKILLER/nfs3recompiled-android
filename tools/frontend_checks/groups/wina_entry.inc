// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
{
    using namespace nfs3hp;
    const auto oneIn = [](x86::reg32 n) { return random(n) == 0; };
    const auto stackArgument = [&](x86::reg32 place, x86::reg32 value) { setWord32(entry.esp + 4 + 4 * place, value); };
    // A stream's handle: its channels are; now and then none, or one of no stream.
    const x86::reg32 winaHandle = oneIn(10) ? 0 : oneIn(10) ? kWinaBadHandle : kWinaChannels + 16 * random(3);
    const x86::reg32 winaRecord = kWinaFiles + 0x128 * random(4);
    const x86::reg32 winaHwnd = oneIn(4) ? random(0) : word32(0x9f3d24);
    switch (c.address)
    {
    case 0x4cf1c0:
        entry.eax = winaHandle;
        entry.edx = kWinaOut;
        entry.ebx = kWinaOut + 4;
        break;
    case 0x4cf1f0:
        entry.eax = random(8);
        entry.edx = random(8);
        entry.ebx = random(8);
        break;
    case 0x4cf220:
        entry.eax = kWinaStream;
        entry.edx = oneIn(2) ? random(0x2000) : word32(kWinaStream + 0x3c) - word32(kWinaStream + 0x34) + random(4);
        break;
    case 0x4cf280:
    case 0x4cf440:
        entry.eax = kWinaStream;
        if (c.address == 0x4cf280 && oneIn(4))
            setWord32(0x7f74b0, 0xffffff00);
        break;
    case 0x4cfa40:
        entry.eax = random(0x100);
        entry.edx = random(0x100);
        entry.ebx = random(0x100);
        break;
    case 0x4cf2f0:
        entry.eax = kWinaStream;
        entry.edx = kWinaFiles + 0x128 * 3;
        if (oneIn(3))
            setWord32(kWinaStream + 0x54, 0);
        break;
    case 0x4cf340:
    {
        entry.eax = kWinaStream;
        const x86::reg32 index = random(5);
        entry.edx = oneIn(4) ? random(0) : index < 4 ? word32(kWinaFiles + 0x128 * index) : index;
        break;
    }
    case 0x4cf380:
    {
        entry.eax = kWinaStream;
        // On the queue, a record of it: the first, the last or between.
        const x86::reg32 last = word32(kWinaStream + 0x54);
        const x86::reg32 places = (last - kWinaFiles) / 0x128 + 1;
        entry.edx = kWinaFiles + 0x128 * random(places);
        if (oneIn(2))
            setWord32(kWinaStream + 0x50, entry.edx);
        break;
    }
    case 0x4cf3f0:
        entry.eax = kWinaStream;
        entry.edx = word32(kWinaStream + 0x40);
        if (oneIn(4))
            setWord32(kWinaStream + 0x14, 0);
        break;
    case 0x4cf570:
    case 0x4cf5a0:
    case 0x4cf5e0:
        entry.ebx = kWinaStream;
        if (word32(kWinaStream + 0x50) == 0)
            setWord32(kWinaStream + 0x50, kWinaFiles);
        break;
    case 0x4cf6b0:
    case 0x4cf810:
        entry.eax = kWinaStream;
        entry.edx = 0x32 + random(4);
        if (c.address == 0x4cf810 && word32(kWinaStream + 0x50) == 0)
            setWord32(kWinaStream + 0x50, kWinaFiles);
        break;
    case 0x4cfa80:
    {
        entry.eax = oneIn(8) ? random(0x110) : 2 + random(8);
        entry.edx = oneIn(8) ? random(0x14) : 1 + random(8);
        entry.ebx = oneIn(8) ? random(0x14) : 1 + random(entry.edx != 0 ? entry.edx : 1);
        entry.ecx = 0xc20000;
        stackArgument(0, oneIn(8) ? random(0x8000) : 0x8000 + random(0x20000));
        break;
    }
    case 0x4cfca0:
    {
        entry.eax = winaHandle;
        entry.edx = random(6);
        entry.ebx = oneIn(3) ? 0 : random(0);
        entry.ecx = oneIn(3) ? 0 : random(0);
        const x86::reg32 pick = random(6);
        stackArgument(0, pick == 0 ? 0xffffffff : pick == 1 ? 0xfffffffe : pick == 2 ? 0xfffffffd : pick == 3 ? 0 : random(6));
        if (oneIn(2))
            setWord32(kWinaStream + 0x28, 0);
        break;
    }
    case 0x4cfd30:
    case 0x4d0220:
    case 0x4d0460:
    case 0x4d0490:
    case 0x4d04c0:
    case 0x4d0500:
    case 0x495b60:
    case 0x4d0300:
        entry.eax = winaHandle;
        if (c.address == 0x4d0300 || c.address == 0x495b60)
            entry.eax = oneIn(8) ? winaHandle : kWinaChannels + 16 * random(3);
        if (c.address == 0x4d04c0 && oneIn(2))
        {
            setWord32(kWinaStream + 0x28, 0);
            setWord32(kWinaChannels + 16 * random(3) + 8, 0);
        }
        if (c.address == 0x4d0220 && word32(kWinaStream + 0x50) == 0)
            setWord32(kWinaStream + 0x50, kWinaFiles);
        break;
    case 0x4cfdd0:
        entry.eax = winaHandle;
        entry.edx = oneIn(2) ? word32(kWinaStream + 0x3c) + random(3) - 1 : random(0x1000);
        break;
    case 0x4cfe30:
        entry.eax = winaHandle;
        entry.edx = random(3);
        break;
    case 0x4cfe70:
        entry.eax = winaHandle;
        entry.edx = random(5);
        break;
    case 0x4cfef0:
        entry.eax = winaHandle;
        entry.edx = kWinaName;
        entry.ebx = random(0x1000);
        entry.ecx = random(0);
        break;
    case 0x4cffb0:
        entry.eax = winaHandle;
        entry.edx = kWinaBlock;
        entry.ebx = oneIn(2) ? 0 : random(0x1000);
        entry.ecx = 0x100 + random(4);
        break;
    case 0x4d0060:
        entry.eax = winaHandle;
        entry.edx = oneIn(5) ? random(0x100) : word32(winaRecord);
        break;
    case 0x4d03c0:
    {
        entry.eax = winaHandle;
        const x86::reg32 ring = word32(kWinaStream + 0x20);
        entry.edx = oneIn(5) ? ring - 8 : oneIn(5) ? word32(kWinaStream + 0x24) - random(2) * 4 - 4 : word32(kWinaStream + 0x40);
        break;
    }
    case 0x4fb990:
        entry.eax = random(3);
        entry.edx = oneIn(8) ? random(0) : random(0x100);
        entry.ebx = random(0);
        entry.ecx = oneIn(8) ? random(0x22) : random(0x20);
        break;
    case 0x4fbb60:
    case 0x4fbbe0:
        entry.eax = kWinaOp;
        entry.edx = c.address == 0x4fbbe0 && oneIn(4) ? 0 : kWinaName;
        if (c.address == 0x4fbb60 && oneIn(10))
            setWord32(0x9f4f60, 0);
        break;
    case 0x4fc8e0:
        entry.eax = random(0);
        entry.edx = kWinaFn3;
        break;
    case 0x4fca60:
        entry.eax = kWinaName;
        entry.ecx = random(0);
        break;
    case 0x4fce50:
        entry.eax = oneIn(2) ? kWinaName : kWinaNameBar;
        break;
    case 0x4fcab0:
        stackArgument(0, random(0x100));
        stackArgument(1, random(0));
        break;
    case 0x4e4230:
    case 0x4e4320:
        entry.eax = oneIn(8) ? random(0) : ~(random(10) - 1);
        break;
    case 0x4cc650:
        entry.eax = kWinaTable;
        entry.edx = kWinaTable + 0x10;
        break;
    case 0x4cc660:
        stackArgument(0, random(0x200));
        break;
    case 0x4cc6d0:
        stackArgument(0, random(0x200));
        stackArgument(1, oneIn(3) ? 0 : kWinaHandler);
        break;
    case 0x4cc7c0:
        entry.ecx = random(2);
        break;
    case 0x4cc890:
        entry.eax = 0x9f38d0;
        break;
    case 0x4ccaa0:
        entry.eax = oneIn(8) ? random(0x200) : oneIn(2) ? 0x100 : 0x104;
        entry.edx = oneIn(8) ? random(0) : random(0x80);
        break;
    case 0x4ccc70:
    {
        stackArgument(0, oneIn(5) ? random(3) : 0);
        const x86::reg32 pick = random(8);
        stackArgument(1, pick == 0 ? 9 : pick == 1 ? 0x1b : pick == 2 ? 0x5b : pick == 3 ? 0x5c : pick == 4 ? 0x5d : pick == 5 ? 0x2c : random(0x100));
        stackArgument(2, random(0));
        setWord32(0x9f3d4c, oneIn(4) ? 0 : 1);
        break;
    }
    case 0x4ccd60:
        stackArgument(0, oneIn(4) ? 0 : 1);
        stackArgument(1, winaHwnd);
        break;
    case 0x4ccdd0:
        stackArgument(1, winaHwnd);
        break;
    case 0x4cce40:
    case 0x4cce60:
    case 0x4cce80:
        stackArgument(0, oneIn(2) ? 0 : random(0));
        break;
    case 0x4cce90:
    {
        const x86::reg32 pick = random(6);
        stackArgument(0, 0x9f38d0);
        stackArgument(3, pick == 0 ? 0xf060 : pick == 1 ? 0xf020 : pick == 2 ? 0xf030 : pick == 3 ? 0xf120 : random(0x10000));
        break;
    }
    case 0x4cced0:
        stackArgument(0, oneIn(4) ? 0 : 0x9f38d0);
        stackArgument(5, kWinaOut);
        break;
    case 0x4ccf50:
        stackArgument(0, oneIn(4) ? 0 : 0x9f38d0);
        break;
    case 0x4ccfc0:
        stackArgument(0, oneIn(4) ? 0 : 1);
        stackArgument(3, random(0x100));
        stackArgument(4, (random(0) & 0xffff0000) | random(40));
        break;
    case 0x4cd020:
        stackArgument(0, oneIn(4) ? 0 : 1);
        stackArgument(2, oneIn(2) ? 0x100 : 0x104);
        stackArgument(3, random(0x80));
        stackArgument(4, (random(0) & 0xffff0000) | random(40));
        break;
    case 0x4cd0e0:
        stackArgument(0, oneIn(4) ? 0 : 1);
        stackArgument(4, random(0));
        break;
    case 0x4cd130:
    case 0x4cd190:
        stackArgument(0, oneIn(4) ? 0 : 1);
        stackArgument(3, random(0x20));
        stackArgument(4, random(0x180) | (random(0x100) << 16));
        break;
    case 0x4cd1f0:
        stackArgument(0, oneIn(4) ? 0 : 0x9f38d0);
        stackArgument(1, random(0));
        stackArgument(5, kWinaOut);
        break;
    case 0x4cd2a0:
        stackArgument(0, oneIn(4) ? 0 : 0x9f38d0);
        stackArgument(1, oneIn(2) ? random(0) : word32(0x9f38d0 + 0x454));
        break;
    case 0x4cd2d0:
        stackArgument(0, random(0));
        stackArgument(1, random(0x200));
        break;
    case 0x4cd350:
        entry.eax = 0x9f38d0;
        break;
    case 0x4cd6c0:
        entry.ebx = random(2);
        break;
    case 0x4fb470:
        entry.eax = oneIn(2) ? 90 + random(30) : 280 + random(30);
        entry.edx = oneIn(2) ? 40 + random(30) : 190 + random(20);
        entry.ebx = random(8);
        entry.ecx = random(2);
        break;
    case 0x4a38f0:
        entry.eax = kWinaOut;
        entry.edx = random(6);
        break;
    case 0x4b5860:
        entry.eax = random(4);
        entry.edx = random(6);
        break;
    case 0x4b5e50:
    case 0x4b5f10:
        entry.eax = random(6);
        break;
    case 0x4b5ff0:
        stackArgument(3, random(3));
        break;
    case 0x4b6400:
    case 0x4b64c0:
        entry.eax = kWinaOut;
        break;
    case 0x4ea665:
        entry.eax = kWinaTable;
        entry.edx = random(8);
        entry.ebx = kWinaOut;
        break;
    case 0x4f2460:
        entry.eax = oneIn(8) ? 0 : kWinaShape;
        break;
    case 0x4fd4e0:
    case 0x4fd500:
        entry.edx = kWinaPixels;
        entry.ebx = 1 + random(16);
        break;
    case 0x503b00:
        entry.edx = oneIn(4) ? 0 : oneIn(4) ? random(0) : random(0x40);
        entry.esi = kWinaOut;
        break;
    case 0x503cd0:
        entry.eax = oneIn(3) ? 0 : 0x9f38d0;
        entry.edx = random(0x80);
        entry.ebx = random(0x40);
        setWord32(0x564364, oneIn(5) ? random(9) : 7);
        break;
    case 0x503e50:
    case 0x503ec0:
        entry.eax = random(0x80);
        entry.edx = random(0x40);
        entry.ebx = c.address == 0x503ec0 ? kWinaPixels : kWinaOut;
        break;
    case 0x50e800:
        entry.eax = kWinaQueue;
        entry.edx = random(5);
        break;
    case 0x517920:
        entry.eax = oneIn(4) ? 0 : kWinaSurface;
        break;
    case 0x51a170:
    case 0x525600:
        entry.eax = kWinaSurface;
        break;
    case 0x524500:
        stackArgument(0, oneIn(2) ? 0 : kWinaCom);
        if (word32(0x9f38d0) == 0)
            setWord32(0x9f38d0, kWinaCom);
        break;
    case 0x524550:
        stackArgument(0, random(2));
        break;
    case 0x5245a0:
        stackArgument(0, random(6));
        stackArgument(1, kWinaName);
        break;
    case 0x5245f0:
        stackArgument(0, random(4));
        break;
    case 0x5247c0:
        entry.eax = oneIn(3) ? 0 : random(0x1000);
        entry.edx = oneIn(3) ? 0 : random(0x1000);
        entry.ebx = oneIn(3) ? 0 : random(0x20);
        break;
    case 0x525630:
    case 0x525670:
        entry.eax = kWinaTextPlace;
        break;
    case 0x525650:
        entry.eax = oneIn(3) ? random(0) : random(0x90);
        break;
    default:
        break;
    }
}
