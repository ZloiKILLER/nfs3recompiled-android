// Read off nfs3.exe by specs_from_exe.py: what rendc calls and no table had.
{ 0x40e0c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x40e2b0, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x420650, { EAX | EBX | EDX | ESI, 0, 0, EBX | EDX | ESI, Kind::Plain } },
{ 0x4206f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4207e0, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x4349d0, { EAX, 2, 8, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 434979 change them; those calls may read EBX
{ 0x4529a0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x478290, { 0, 0, 0, 0, Kind::Plain } },
{ 0x47adc0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4956f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x49d450, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x49d9d0, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x49da30, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },
{ 0x49db70, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x49dbf0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4bbde0, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x4bbe70, { ESI, 0, 0, ESI, Kind::Plain } },
{ 0x4bbf20, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4bbfc0, { ESI, 0, 0, ESI, Kind::Plain } },
{ 0x4c7400, { 0, 3, 12, 0, Kind::Plain } },
{ 0x4d7990, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read ESI
{ 0x4daf80, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfd80, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4dfde0, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4dffb0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0000, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0280, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e0430, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0d88, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4ea920, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea930, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea950, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea9c0, { 0, 2, 8, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea9e0, { EAX, 2, 8, 0, Kind::Plain } },
{ 0x4eaab0, { EAX, 2, 8, 0, Kind::Plain } },
{ 0x4eab80, { EAX, 2, 8, 0, Kind::Plain } },
{ 0x4eb230, { 0, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4f1c30, { 0, 2, 8, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4f2d78, { 0, 2, 8, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
