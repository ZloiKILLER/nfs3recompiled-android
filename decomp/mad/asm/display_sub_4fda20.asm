==== sub_4fda20 .. 4fdc90
004fda20  53                       push ebx
004fda21  51                       push ecx
004fda22  52                       push edx
004fda23  56                       push esi
004fda24  57                       push edi
004fda25  55                       push ebp
004fda26  89e5                     mov ebp, esp
004fda28  83ec14                   sub esp, 0x14
004fda2b  83e4f8                   and esp, 0xfffffff8
004fda2e  8944240c                 mov dword ptr [esp + 0xc], eax
004fda32  b9c0ffffff               mov ecx, 0xffffffc0
004fda37  bb71000000               mov ebx, 0x71
004fda3c  bf00180200               mov edi, 0x21800
004fda41  be0000802c               mov esi, 0x2c800000
004fda46  31d2                     xor edx, edx
004fda48  894c2410                 mov dword ptr [esp + 0x10], ecx
004fda4c  8d043e                   lea eax, [esi + edi]
004fda4f  db442410                 fild dword ptr [esp + 0x10]
004fda53  dc05b8e05400             fadd qword ptr [0x54e0b8]
004fda59  01d8                     add eax, ebx
004fda5b  dd1424                   fst qword ptr [esp]
004fda5e  dc0d98e05400             fmul qword ptr [0x54e098]
004fda64  8982185e9f00             mov dword ptr [edx + 0x9f5e18], eax
004fda6a  83ec04                   sub esp, 4
004fda6d  db1c24                   fistp dword ptr [esp]
004fda70  58                       pop eax
004fda71  dd0424                   fld qword ptr [esp]
004fda74  dc0da0e05400             fmul qword ptr [0x54e0a0]
004fda7a  89442410                 mov dword ptr [esp + 0x10], eax
004fda7e  83ec04                   sub esp, 4
004fda81  db1c24                   fistp dword ptr [esp]
004fda84  58                       pop eax
004fda85  dd0424                   fld qword ptr [esp]
004fda88  89442408                 mov dword ptr [esp + 8], eax
004fda8c  8b442410                 mov eax, dword ptr [esp + 0x10]
004fda90  dc0da8e05400             fmul qword ptr [0x54e0a8]
004fda96  25ff010000               and eax, 0x1ff
004fda9b  c1e00b                   shl eax, 0xb
004fda9e  89442410                 mov dword ptr [esp + 0x10], eax
004fdaa2  8b442408                 mov eax, dword ptr [esp + 8]
004fdaa6  25ff010000               and eax, 0x1ff
004fdaab  89442408                 mov dword ptr [esp + 8], eax
004fdaaf  8b442410                 mov eax, dword ptr [esp + 0x10]
004fdab3  03442408                 add eax, dword ptr [esp + 8]
004fdab7  898218609f00             mov dword ptr [edx + 0x9f6018], eax
004fdabd  83ec04                   sub esp, 4
004fdac0  db1c24                   fistp dword ptr [esp]
004fdac3  58                       pop eax
004fdac4  dd0424                   fld qword ptr [esp]
004fdac7  dc0db0e05400             fmul qword ptr [0x54e0b0]
004fdacd  89442408                 mov dword ptr [esp + 8], eax
004fdad1  83ec04                   sub esp, 4
004fdad4  db1c24                   fistp dword ptr [esp]
004fdad7  58                       pop eax
004fdad8  89442410                 mov dword ptr [esp + 0x10], eax
004fdadc  8b442408                 mov eax, dword ptr [esp + 8]
004fdae0  25ff010000               and eax, 0x1ff
004fdae5  c1e017                   shl eax, 0x17
004fdae8  89442408                 mov dword ptr [esp + 8], eax
004fdaec  8b442410                 mov eax, dword ptr [esp + 0x10]
004fdaf0  83c204                   add edx, 4
004fdaf3  25ff010000               and eax, 0x1ff
004fdaf8  81c700080000             add edi, 0x800
004fdafe  c1e00b                   shl eax, 0xb
004fdb01  81c600008000             add esi, 0x800000
004fdb07  89442410                 mov dword ptr [esp + 0x10], eax
004fdb0b  8b442408                 mov eax, dword ptr [esp + 8]
004fdb0f  41                       inc ecx
004fdb10  03442410                 add eax, dword ptr [esp + 0x10]
004fdb14  43                       inc ebx
004fdb15  898214629f00             mov dword ptr [edx + 0x9f6214], eax
004fdb1b  83f940                   cmp ecx, 0x40
004fdb1e  0f8c24ffffff             jl 0x4fda48
004fdb24  b99cfdffff               mov ecx, 0xfffffd9c
004fdb29  be1f000000               mov esi, 0x1f
004fdb2e  8b7c240c                 mov edi, dword ptr [esp + 0xc]
004fdb32  ba67ffffff               mov edx, 0xffffff67
004fdb37  8d0412                   lea eax, [edx + edx]
004fdb3a  0581000000               add eax, 0x81
004fdb3f  85c0                     test eax, eax
004fdb41  0f8c03010000             jl 0x4fdc4a
004fdb47  3dff000000               cmp eax, 0xff
004fdb4c  7e05                     jle 0x4fdb53
004fdb4e  b8ff000000               mov eax, 0xff
004fdb53  89c3                     mov ebx, eax
004fdb55  83c004                   add eax, 4
004fdb58  c1f803                   sar eax, 3
004fdb5b  c1fb03                   sar ebx, 3
004fdb5e  83f81f                   cmp eax, 0x1f
004fdb61  7e02                     jle 0x4fdb65
004fdb63  89f0                     mov eax, esi
004fdb65  83ff0f                   cmp edi, 0xf
004fdb68  0f85e3000000             jne 0x4fdc51
004fdb6e  c1e00a                   shl eax, 0xa
004fdb71  c1e31a                   shl ebx, 0x1a
004fdb74  01d8                     add eax, ebx
004fdb76  8981b45b9f00             mov dword ptr [ecx + 0x9f5bb4], eax
004fdb7c  42                       inc edx
004fdb7d  83c104                   add ecx, 4
004fdb80  81fa99000000             cmp edx, 0x99
004fdb86  7caf                     jl 0x4fdb37
004fdb88  b9f4fdffff               mov ecx, 0xfffffdf4
004fdb8d  ba7dffffff               mov edx, 0xffffff7d
004fdb92  bfff000000               mov edi, 0xff
004fdb97  be3f000000               mov esi, 0x3f
004fdb9c  8d0412                   lea eax, [edx + edx]
004fdb9f  0581000000               add eax, 0x81
004fdba4  85c0                     test eax, eax
004fdba6  0f8cb0000000             jl 0x4fdc5c
004fdbac  3dff000000               cmp eax, 0xff
004fdbb1  7e02                     jle 0x4fdbb5
004fdbb3  89f8                     mov eax, edi
004fdbb5  837c240c0f               cmp dword ptr [esp + 0xc], 0xf
004fdbba  0f85a3000000             jne 0x4fdc63
004fdbc0  89c3                     mov ebx, eax
004fdbc2  83c004                   add eax, 4
004fdbc5  c1f803                   sar eax, 3
004fdbc8  c1fb03                   sar ebx, 3
004fdbcb  83f81f                   cmp eax, 0x1f
004fdbce  7e05                     jle 0x4fdbd5
004fdbd0  b81f000000               mov eax, 0x1f
004fdbd5  c1e005                   shl eax, 5
004fdbd8  c1e315                   shl ebx, 0x15
004fdbdb  83c104                   add ecx, 4
004fdbde  01d8                     add eax, ebx
004fdbe0  42                       inc edx
004fdbe1  898140579f00             mov dword ptr [ecx + 0x9f5740], eax
004fdbe7  81fa83000000             cmp edx, 0x83
004fdbed  7cad                     jl 0x4fdb9c
004fdbef  b93cfdffff               mov ecx, 0xfffffd3c
004fdbf4  bf1f000000               mov edi, 0x1f
004fdbf9  beff000000               mov esi, 0xff
004fdbfe  ba4fffffff               mov edx, 0xffffff4f
004fdc03  8d0412                   lea eax, [edx + edx]
004fdc06  0581000000               add eax, 0x81
004fdc0b  85c0                     test eax, eax
004fdc0d  7c6f                     jl 0x4fdc7e
004fdc0f  3dff000000               cmp eax, 0xff
004fdc14  7e02                     jle 0x4fdc18
004fdc16  89f0                     mov eax, esi
004fdc18  89c3                     mov ebx, eax
004fdc1a  83c004                   add eax, 4
004fdc1d  c1f803                   sar eax, 3
004fdc20  c1fb03                   sar ebx, 3
004fdc23  83f81f                   cmp eax, 0x1f
004fdc26  7e02                     jle 0x4fdc2a
004fdc28  89f8                     mov eax, edi
004fdc2a  c1e310                   shl ebx, 0x10
004fdc2d  83c104                   add ecx, 4
004fdc30  01d8                     add eax, ebx
004fdc32  42                       inc edx
004fdc33  898170529f00             mov dword ptr [ecx + 0x9f5270], eax
004fdc39  81fab1000000             cmp edx, 0xb1
004fdc3f  7cc2                     jl 0x4fdc03
004fdc41  89ec                     mov esp, ebp
004fdc43  5d                       pop ebp
004fdc44  5f                       pop edi
004fdc45  5e                       pop esi
004fdc46  5a                       pop edx
004fdc47  59                       pop ecx
004fdc48  5b                       pop ebx
004fdc49  c3                       ret 
004fdc4a  31c0                     xor eax, eax
004fdc4c  e902ffffff               jmp 0x4fdb53
004fdc51  c1e00b                   shl eax, 0xb
004fdc54  c1e31b                   shl ebx, 0x1b
004fdc57  e918ffffff               jmp 0x4fdb74
004fdc5c  31c0                     xor eax, eax
004fdc5e  e952ffffff               jmp 0x4fdbb5
004fdc63  89c3                     mov ebx, eax
004fdc65  83c002                   add eax, 2
004fdc68  c1f802                   sar eax, 2
004fdc6b  c1fb02                   sar ebx, 2
004fdc6e  83f83f                   cmp eax, 0x3f
004fdc71  0f8e5effffff             jle 0x4fdbd5
004fdc77  89f0                     mov eax, esi
004fdc79  e957ffffff               jmp 0x4fdbd5
004fdc7e  31c0                     xor eax, eax
004fdc80  eb96                     jmp 0x4fdc18
004fdc82  8d8000000000             lea eax, [eax]
004fdc88  8d9200000000             lea edx, [edx]
004fdc8e  8bc0                     mov eax, eax

