// Read off nfs3.exe by specs_from_exe.py: what eab calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4cc970, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4df6c0, { 0, 1, 0, 0, Kind::Plain } },  // reads 1 words above its return, the caller cleans
{ 0x4df7a0, { EAX | EBX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4df8e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4df930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb00, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfb30, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfc00, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfc50, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfc80, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfcf0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfd04, { EAX, 0, 0, ECX, Kind::Plain } },  // those calls may read ECX
{ 0x4e1080, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e10e0, { EDX, 0, 0, 0, Kind::Plain } },
{ 0x4e1120, { EAX | EBX | ECX | EDX, 3, 12, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e11f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e1580, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e15d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e15e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e28d0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e2da0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e2de0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e3430, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e3600, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e3620, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e3980, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e3ae0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e3cc0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e3e60, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e3fe0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e41f0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e4230, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e7630, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4eb360, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb5e0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read ECX
{ 0x4ec0d0, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read ECX
{ 0x4ed730, { EAX | EDX, 0, 0, EBX | EDX | ESI | EDI, Kind::Plain } },  // EBX | EDX changed only if the indirect calls at 4ed2d2 4ed745 4ed752 change them; those calls may read EBX | ECX | ESI
{ 0x4f0850, { EAX | EDX, 0, 0, 0, Kind::NoEax } },
{ 0x4f29b0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4780, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f47c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4f90, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 4010a3 401174 401186 ... change them; those calls may read EBX
{ 0x4f6490, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f7c20, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f9120, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4fa0f0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4fd540, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4ff7c0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EAX | ECX | EDX
{ 0x501050, { 0, 3, 0, 0, Kind::Plain } },  // reads 3 words above its return, the caller cleans
{ 0x501110, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x501180, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x508da0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x5098d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x509a30, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x509b10, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x509c00, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x50a830, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50b0b0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x50c0d0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x50c180, { 0, 0, 0, 0, Kind::Plain } },
{ 0x50c3a0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50c584, { EAX, 0, 0, EBX | ECX, Kind::Plain } },  // EBX | ECX changed only if the indirect calls at 50c5a1 change them; those calls may read EBX | ECX | ESI | EDI
{ 0x50dcd0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x50dde0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50de50, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x50df60, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50e0a0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x50e150, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // EDI left in another register as it came
{ 0x50e9b0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x50ea20, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50ea40, { EAX, 0, 0, EDX, Kind::Plain } },
{ 0x512180, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },
{ 0x515050, { 0, 0, 0, 0, Kind::Plain } },
{ 0x51b200, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x524970, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },
{ 0x52f535, { 0, 3, 12, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 52f577 change them; those calls may read EBX | ECX | EDX
{ 0x52f661, { 0, 1, 4, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 52f69b change them; those calls may read EBX | ECX | EDX
{ 0x52f969, { 0, 1, 4, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 52f9a3 change them; those calls may read EBX | ECX | EDX
{ 0x530b4d, { EAX, 2, 8, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 530b8b change them; those calls may read EBX | ECX | EDX
{ 0x5324d2, { EAX, 1, 4, EBX | ECX | EDX, Kind::Plain } },  // EBX changed only if the indirect calls at 53250c change them; those calls may read EBX | ECX | EDX
{ 0x533dde, { 0, 2, 8, ECX | EDX, Kind::Plain } },
{ 0x533dea, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533df0, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533df6, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533dfc, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533e02, { 0, 2, 8, ECX | EDX, Kind::Plain } },
{ 0x533e3e, { 0, 3, 12, ECX | EDX, Kind::Plain } },
{ 0x533e4a, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x533ea4, { 0, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x4ea290, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e9f00, { EAX, 0, 0, 0, Kind::Plain } },
