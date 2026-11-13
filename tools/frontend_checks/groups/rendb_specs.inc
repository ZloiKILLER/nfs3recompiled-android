// Read off nfs3.exe by specs_from_exe.py: what rendb calls and no table had.
{ 0x431900, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4774b0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x47f650, { EAX | EBX | ECX | EDX, 1, 4, EBX | EDX, Kind::Plain } },
{ 0x485a60, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x487350, { EAX | EBX | ECX | EDX, 1, 4, ECX | EDX, Kind::Plain } },
{ 0x4873c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x488a80, { 0, 0, 0, 0, Kind::Plain } },
{ 0x48b5f0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4965b0, { EAX | EDX, 0, 0, 0, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x49bd10, { EAX | EBX | ECX | EDX, 0, 0, ECX, Kind::NoEax } },
{ 0x49bd30, { EAX | EBX | ECX | EDX, 1, 4, EBX | EDX, Kind::Plain } },
{ 0x49bdc0, { EAX | EBX | ECX | EDX, 1, 4, EBX | EDX, Kind::Plain } },
{ 0x4a33d0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4bed70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4c0b20, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4c12e0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4d4080, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e01f0, { EAX | EDX, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4e0280, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e0ec0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4ea9c0, { 0, 2, 8, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea9e0, { EAX, 2, 8, 0, Kind::Plain } },
{ 0x4eaab0, { EAX, 2, 8, 0, Kind::Plain } },
