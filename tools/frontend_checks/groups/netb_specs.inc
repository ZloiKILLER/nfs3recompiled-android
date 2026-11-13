// Read off nfs3.exe by specs_from_exe.py: what netb calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4df7a0, { EAX | EBX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4df8e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4df930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb30, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e0716, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e08f0, { EAX, 1, 0, 0, Kind::Plain } },  // reads 1 words above its return, the caller cleans
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e7630, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e7720, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4eb390, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4eba70, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4f2790, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4f4f90, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 4010a3 401174 401186 ... change them; those calls may read EBX
{ 0x4f5040, { EAX, 0, 0, EBX | EDX, Kind::Plain } },  // EBX | EDX changed only if the indirect calls at 4f5047 change them; those calls may read EBX | EDX
{ 0x4f5130, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4f5230, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4f5900, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f5a30, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4f5c10, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4f5ea0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f97c0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX
{ 0x4f9920, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x501050, { 0, 3, 0, 0, Kind::Plain } },  // reads 3 words above its return, the caller cleans
{ 0x50e150, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // EDI left in another register as it came
{ 0x50e1d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50e470, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x5150f0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51a930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b030, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b1c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b1e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b200, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b2f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x51b480, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b530, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x51b710, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x51b8a0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51b900, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51ba50, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x52fb2e, { 0, 3, 12, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 52fb70 change them; those calls may read EBX | ECX | EDX
{ 0x530747, { EAX, 5, 20, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 53078f change them; those calls may read EBX | ECX | EDX
{ 0x53083d, { EAX, 3, 12, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 53087f change them; those calls may read EBX | ECX | EDX
{ 0x530a4e, { EAX, 6, 24, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 530a99 change them; those calls may read EBX | ECX | EDX
{ 0x5310be, { EAX, 5, 20, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 531106 change them; those calls may read EBX | ECX | EDX
{ 0x5311c0, { EAX, 5, 20, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 531208 change them; those calls may read EBX | ECX | EDX
{ 0x53138f, { EAX, 6, 24, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 5313da change them; those calls may read EBX | ECX | EDX
{ 0x531439, { EAX, 9, 36, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 53148d change them; those calls may read EBX | ECX | EDX
{ 0x531f31, { EAX, 4, 16, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 531f76 change them; those calls may read EBX | ECX | EDX
{ 0x532162, { EAX, 3, 12, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 5321a4 change them; those calls may read EBX | ECX | EDX
{ 0x5324d2, { EAX, 1, 4, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 53250c change them; those calls may read EBX | ECX | EDX
{ 0x533e08, { 0, 2, 8, ECX | EDX, Kind::Plain } },
{ 0x533e0e, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e14, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e1a, { 0, 2, 8, ECX | EDX, Kind::Plain } },
{ 0x533e20, { 0, 5, 20, ECX | EDX, Kind::Plain } },
{ 0x533e26, { 0, 4, 16, ECX | EDX, Kind::Plain } },
{ 0x533e2c, { 0, 4, 16, ECX | EDX, Kind::Plain } },
{ 0x533e32, { 0, 6, 24, ECX | EDX, Kind::Plain } },
{ 0x533e38, { 0, 0, 0, ECX | EDX, Kind::Plain } },
{ 0x533e3e, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e44, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e4a, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533e50, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e56, { 0, 5, 20, ECX | EDX, Kind::Plain } },
{ 0x533e5c, { 0, 6, 24, ECX | EDX, Kind::Plain } },
{ 0x533e9e, { 0, 0, 0, ECX | EDX, Kind::Plain } },
{ 0x533ea4, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533eaa, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533eb0, { 0, 2, 8, ECX | EDX, Kind::Plain } },
