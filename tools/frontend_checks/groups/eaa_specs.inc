// Read off nfs3.exe by specs_from_exe.py: what eaa calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4d0a10, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e0e60, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e0ec0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e15e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e19d0, { 0, 1, 4, EDX, Kind::Plain } },
{ 0x4e19f0, { 0, 1, 4, 0, Kind::Plain } },
{ 0x4e28d0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e2ab0, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e2da0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e2de0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4ea654, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ea665, { EAX | EBX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4ed0f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read ECX
{ 0x4ed2a0, { EAX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },  // those calls may read ECX | ESI; stack lost: what its pops restore not trusted
{ 0x4ed480, { EAX | EBX | ECX | EDX, 4, 16, EBX | ECX | EDX, Kind::Plain } },
{ 0x4ed730, { EAX | EDX, 0, 0, EBX | EDX | ESI | EDI, Kind::Plain } },  // EBX | EDX changed only if the indirect calls at 4ed2d2 4ed745 4ed752 change them; those calls may read EBX | ECX | ESI
{ 0x4ed770, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 4ed78e change them; those calls may read EBX
{ 0x4edfe8, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4ee100, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ee310, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4ee4f8, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4eeae0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4ef190, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4ef1b0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ef520, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ef760, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4efa70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f2a7d, { EBX | ECX | EDX | EDI, 0, 0, EBX | ECX | EDX | EDI, Kind::Plain } },
{ 0x4f45c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4610, { 0, 1, 4, 0, Kind::NoEax } },
{ 0x4f4650, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f46b0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4f46d0, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x4f4700, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read EBX
{ 0x4f4740, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x4f4780, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f47c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4a80, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f4b80, { 0, 2, 0, 0, Kind::Plain } },  // reads 2 words above its return, the caller cleans
{ 0x4f9060, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4f9120, { 0, 0, 0, 0, Kind::Plain } },
{ 0x501050, { 0, 3, 0, 0, Kind::Plain } },  // reads 3 words above its return, the caller cleans
{ 0x503bc0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x503ec0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x5040b8, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // the upper bits of ECX go through as they came
{ 0x5084e0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX
{ 0x508c70, { EAX | EBX | ECX | EDX, 3, 12, EBX | ECX | EDX, Kind::Plain } },
{ 0x508e70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x533e86, { 0, 3, 12, ECX | EDX, Kind::Plain } },
