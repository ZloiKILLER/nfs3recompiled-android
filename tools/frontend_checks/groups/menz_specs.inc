// Read off nfs3.exe by specs_from_exe.py: what menz calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4df7a0, { EAX | EBX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4df8e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4df930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb00, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfc10, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfc60, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e1230, { EAX | EBX | ECX | EDX, 8, 32, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e1580, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e3fe0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e7630, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eada0, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX | EDX
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4eb5e0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read ECX
{ 0x4eb880, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb8a0, { EAX | EBX | ECX | EDX, 0, 0, ECX | EDX, Kind::Plain } },
{ 0x4ed2a0, { EAX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },  // those calls may read ECX | ESI; stack lost: what its pops restore not trusted
{ 0x4ef190, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4ef520, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ef760, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ef830, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4efb50, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eff00, { EAX, 0, 0, EDI, Kind::Plain } },  // those calls may read EBX | ESI
{ 0x4f00d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f06e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f0860, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f09c0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4f0c90, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4f2a78, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f2a7d, { EBX | ECX | EDX | EDI, 0, 0, EBX | ECX | EDX | EDI, Kind::Plain } },
{ 0x4f2cb0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4f46d0, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x4f4700, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read EBX
{ 0x4f4740, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x4f4760, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fd540, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4feffc, { 0, 0, 0, 0, Kind::Plain } },
{ 0x501050, { 0, 3, 0, 0, Kind::Plain } },  // reads 3 words above its return, the caller cleans
{ 0x501180, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x5089d0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x508b80, { EAX | EBX | ECX | EDX, 3, 12, EBX | ECX | EDX, Kind::Plain } },
{ 0x509a30, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x509b10, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50c1e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x50c5d4, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50dcd0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x50dd60, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x50ded0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50e3f0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x50e5a0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x525b80, { EAX | EBX | EDX, 0, 0, 0, Kind::NoEax } },
{ 0x526674, { 0, 0, 0, EDX, Kind::Plain } },  // stack lost: what its pops restore not trusted; leaves -1 on the x87: no Kind::Plain does that; takes 1 from the x87
{ 0x5269d4, { 0, 8, 0, 0, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that; reads 8 words above its return, the caller cleans
{ 0x52b6c0, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::NoEax } },
{ 0x52b720, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x52b860, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x52b8b0, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x52b8d0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX | EDX
{ 0x52bb20, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX | EDX
{ 0x52c88a, { 0, 0, 0, EDX, Kind::Plain } },
{ 0x52c89b, { 0, 0, 0, EDX, Kind::Plain } },
{ 0x52c8a7, { EAX, 0, 0, EDX, Kind::Plain } },
