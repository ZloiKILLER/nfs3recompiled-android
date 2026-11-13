// Read off nfs3.exe by specs_from_exe.py: what netc calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4df7a0, { EAX | EBX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4df8e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4df930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb00, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfb30, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb40, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfbf0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4dfc00, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e0820, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e08f0, { EAX, 1, 0, 0, Kind::Plain } },  // reads 1 words above its return, the caller cleans
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e7630, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e7720, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4eb360, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f0430, { EAX, 0, 0, 0, Kind::Plain } },  // the upper bits of EBX | EDX go through as they came
{ 0x4f06e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f2a78, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4780, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f4f90, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 4010a3 401174 401186 ... change them; those calls may read EBX
{ 0x4f5040, { EAX, 0, 0, EBX | EDX, Kind::Plain } },  // EBX | EDX changed only if the indirect calls at 4f5047 change them; those calls may read EBX | EDX
{ 0x4f5130, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x501050, { 0, 3, 0, 0, Kind::Plain } },  // reads 3 words above its return, the caller cleans
{ 0x50dcd0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x50dde0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50de50, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50df60, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50e0a0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x50e150, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // EDI left in another register as it came
{ 0x50e2d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50e470, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50e500, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50e800, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50e8e0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x50e9b0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x50ea20, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50ea40, { EAX, 0, 0, EDX, Kind::Plain } },
{ 0x512180, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },
{ 0x5126f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // EBX | EDX changed only if the indirect calls at 4010a3 401174 401186 ... change them
{ 0x512f60, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x512fc0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x512fd0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x513190, { EAX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x5132a0, { EAX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDX
{ 0x5133a0, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },  // stack lost: what its pops restore not trusted
