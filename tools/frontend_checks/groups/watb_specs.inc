// Read off nfs3.exe by specs_from_exe.py: what watb calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4ee144, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4ef760, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f7900, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f79f0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f90e0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX | EDX
{ 0x5042c4, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // the upper bits of ECX go through as they came
{ 0x50dc20, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x511d0c, { EAX | EDX, 0, 0, EDX | ESI | EDI, Kind::Plain } },
{ 0x516759, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x51de60, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x51df60, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x51e304, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51e45c, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x51f272, { 0, 0, 0, 0, Kind::Plain } },
{ 0x5236c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x523760, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x5239ec, { 0, 7, 0, ECX | EDX, Kind::Plain } },  // the upper bits of EAX go through as they came; reads 7 words above its return, the caller cleans
{ 0x523b00, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x52448c, { EAX, 0, 0, 0, Kind::Plain } },
