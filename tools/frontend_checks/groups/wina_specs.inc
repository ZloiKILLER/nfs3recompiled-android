// Read off nfs3.exe by specs_from_exe.py: what wina calls and no table had.
{ 0x401010, { 0, 1, 0, 0, Kind::Plain } },  // those calls may read EAX; reads 1 words above its return, the caller cleans
{ 0x42d500, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4516a0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ccbb7, { EDX, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbc9, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbd0, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbd7, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbde, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbe5, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbec, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbf3, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccbfa, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc01, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc08, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc0f, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc16, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc1d, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc24, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc2b, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc32, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc39, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc40, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc47, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc4e, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc55, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc5c, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4ccc63, { 0, 0, 0, 0, Kind::Plain } },  // stack lost: what its pops restore not trusted
{ 0x4df6c0, { 0, 1, 0, 0, Kind::Plain } },  // reads 1 words above its return, the caller cleans
{ 0x4df7a0, { EAX | EBX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },
{ 0x4df8e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4df930, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb00, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4dfb20, { EDX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb30, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfb70, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4dfc00, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e0721, { EAX | EBX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e08f0, { EAX, 1, 0, 0, Kind::Plain } },  // reads 1 words above its return, the caller cleans
{ 0x4e3330, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e7520, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4e75e0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4e7630, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4eb2e0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4eb360, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ecd50, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4ecf60, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read ECX
{ 0x4ed0f0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read ECX
{ 0x4ed2a0, { EAX, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },  // those calls may read ECX | ESI; stack lost: what its pops restore not trusted
{ 0x4ee310, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4ee4f8, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4f2a7d, { EBX | ECX | EDX | EDI, 0, 0, EBX | ECX | EDX | EDI, Kind::Plain } },
{ 0x4f46d0, { EAX, 0, 0, 0, Kind::NoEax } },
{ 0x4f4700, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read EBX
{ 0x4f7900, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f79f0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4f9060, { 0, 0, 0, 0, Kind::NoEax } },
{ 0x4f9070, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4f9088, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4fb300, { EBX | ECX | EDX, 1, 4, EBX | ECX | EDX, Kind::Plain } },  // EDX changed only if the indirect calls at 4fb33b 4fb35b change them; those calls may read EAX
{ 0x4fb6a0, { EAX | ESI | EDI, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },
{ 0x4fb6a7, { EDX | ESI | EDI, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },
{ 0x4fb6ae, { ECX | ESI | EDI, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },
{ 0x4fb6b5, { EBX | EDI, 0, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },
{ 0x4fb6c2, { 0, 2, 0, EBX | ECX | EDX | ESI | EDI, Kind::Plain } },  // reads 2 words above its return, the caller cleans
{ 0x4fb6d7, { 0, 1, 0, ESI | EDI, Kind::Plain } },  // stack lost: what its pops restore not trusted; reads 1 words above its return, the caller cleans
{ 0x4fb710, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4fb860, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fbd10, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4fc600, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fc780, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x501110, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50425d, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x509940, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EAX
{ 0x50dc20, { EAX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x50df60, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50e0a0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x50e5a0, { EAX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EBX
{ 0x50ea20, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x50ea40, { EAX, 0, 0, EDX, Kind::Plain } },
{ 0x517170, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x517370, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x517890, { EAX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDX
{ 0x5178d0, { 0, 0, 0, EBX | ECX | EDX | ESI, Kind::Plain } },  // those calls may read EBX | ECX | ESI; stack lost: what its pops restore not trusted
{ 0x524650, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read ECX | EDX
{ 0x52ee00, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x533e8c, { 0, 4, 16, ECX | EDX, Kind::Plain } },
{ 0x533e92, { 0, 4, 16, ECX | EDX, Kind::Plain } },
{ 0x533e98, { 0, 2, 8, ECX | EDX, Kind::Plain } },
// The group's own, for what stands in for them in the others' cases (winaStand).
{ 0x4cf1f0, { EAX | EBX | EDX, 0, 0, 0, Kind::Plain } },
{ 0x4cf220, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4cf280, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4cf2f0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4cf440, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4cf5e0, { EBX, 0, 0, EDX, Kind::Plain } },
{ 0x4cf6b0, { EAX | EDX | ESI, 0, 0, EDX, Kind::Plain } },
{ 0x4cf810, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4cfe30, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x4d0060, { EAX | EDX | ESI | EDI, 0, 0, EDX, Kind::Plain } },
{ 0x4d0220, { EAX | ESI | EDI, 0, 0, 0, Kind::Plain } },
{ 0x4d0300, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4d03c0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4d04c0, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fc8e0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4fc9b0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x4fca60, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4fca90, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // those calls may read EDI
{ 0x4fcab0, { EAX | EBX | ECX | EDX, 2, 8, EBX | ECX | EDX, Kind::Plain } },
{ 0x4fce50, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fb990, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4fbb60, { EAX, 0, 0, 0, Kind::Plain } },  // those calls may read EDI
{ 0x4fbbe0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4e4320, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4fd060, { EAX | EDX, 0, 0, EDX, Kind::Plain } },  // those calls may read EDI
{ 0x517580, { 0, 0, 0, EDI, Kind::Plain } },  // those calls may read ECX | EDX | ESI
{ 0x517920, { EAX, 0, 0, ECX | EDI, Kind::Plain } },  // those calls may read ECX | ESI
{ 0x51a170, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 51a181 525615 change them; those calls may read EBX | ECX
{ 0x519140, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX | ECX | ESI
{ 0x525600, { EAX, 0, 0, EBX, Kind::Plain } },  // EBX changed only if the indirect calls at 525615 change them; those calls may read EBX | ECX
{ 0x4cdaa0, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4cd350, { EAX, 0, 0, 0, Kind::Plain } },
{ 0x4cdb10, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4cdbd0, { EAX | EDX, 0, 0, EDX, Kind::Plain } },
{ 0x4f9120, { 0, 0, 0, 0, Kind::Plain } },
{ 0x4b6590, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EBX
{ 0x4cc6d0, { 0, 2, 8, ECX | EDX, Kind::Plain } },  // by hand: eax is written before any read (0x4eb2e0 always sets it)
{ 0x4cc7c0, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x503cd0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4fb470, { EAX | EBX | ECX | EDX, 0, 0, EBX | ECX | EDX, Kind::Plain } },
{ 0x4cc8e0, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x4cc890, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
