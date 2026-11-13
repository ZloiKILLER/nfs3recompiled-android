// Read off nfs3.exe by specs_from_exe.py: what rendd calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x420650, { EAX | EBX | EDX | ESI, 0, 0, EBX | EDX | ESI, Kind::Plain } },
{ 0x4206f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x422ba0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x42cff0, { 0, 0, 0, 0, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4349d0, { EAX, 2, 8, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 434979 change them; those calls may read EBX
{ 0x470a60, { 0, 0, 0, 0, Kind::Plain } },
{ 0x477d90, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x491120, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x491bc0, { EAX, 8, 32, 0, Kind::Plain } },
{ 0x492370, { EAX, 7, 28, 0, Kind::Plain } },
{ 0x492980, { EAX, 10, 40, 0, Kind::Plain } },
{ 0x496550, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4965b0, { EAX | EDX, 0, 0, 0, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4968a0, { EAX, 6, 24, 0, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4972f0, { EAX, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x49c410, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x49c420, { EAX | EDX, 1, 4, 0, Kind::Plain } },  // those calls may read EBX
{ 0x49c470, { ESI, 0, 0, ESI, Kind::Plain } },
{ 0x49c880, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x49ca50, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x49d5a0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x49d640, { 0, 0, 0, 0, Kind::Plain } },
{ 0x49d720, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x49d750, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x49d760, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x49d780, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x49d800, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x49d8f0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x49de20, { EAX | EBX | ECX | EDX, 5, 20, EBX | ECX | EDX, Kind::Plain } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4b56e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4b5e50, { EAX | EBX, 0, 0, EBX, Kind::Plain } },
{ 0x4b6400, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4b64c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4b97b0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4b9bc0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4bae00, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4bb200, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4bb5d0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4c7380, { ESI, 2, 8, ESI, Kind::Plain } },
{ 0x4c7400, { 0, 3, 12, 0, Kind::Plain } },
{ 0x4c7750, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4c7790, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4c79a0, { EAX | ESI, 0, 0, ESI, Kind::Plain } },
{ 0x4c9f70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfd80, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4dfde0, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4dfe40, { EAX, 1, 4, 0, Kind::Plain } },
{ 0x4dffb0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0000, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0050, { EAX | EDX, 2, 8, EDX, Kind::Plain } },
{ 0x4e01c0, { EAX | EDX, 0, 0, 0, Kind::NoEax } },
{ 0x4e01f0, { EAX | EDX, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4e0210, { EAX | EBX | ECX | EDX, 0, 0, ECX | EDX, Kind::Plain } },
{ 0x4e0280, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e0430, { EAX | EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4eb230, { 0, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
