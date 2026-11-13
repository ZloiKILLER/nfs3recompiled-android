// Read off nfs3.exe by specs_from_exe.py: what sime calls and no table had.
{ 0x415010, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x41d960, { 0, 0, 0, 0, Kind::Plain } },
{ 0x41dbc0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x421480, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4217e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x434ae0, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x434af0, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x438340, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4386b0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4387e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4389d0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4782c0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x478340, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x47b390, { 0, 0, 0, 0, Kind::Plain } },
{ 0x484220, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x495230, { 0, 1, 4, 0, Kind::Plain } },
{ 0x4972f0, { EAX, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4a33d0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4bc530, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4bc870, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4be3d0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4cb6f0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4cb730, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4d56b0, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4dcd10, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e0000, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4e0110, { EAX | EDX, 0, 0, 0, Kind::NoEax } },
{ 0x4e0d88, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4ea920, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea930, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4ea950, { 0, 1, 4, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
{ 0x4eb230, { 0, 0, 0, 0, Kind::NoEax } },  // leaves +1 on the x87: no Kind::Plain does that
