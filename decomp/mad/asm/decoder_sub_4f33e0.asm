==== sub_4f33e0 .. 4f3420
004f33e0  53                       push ebx
004f33e1  51                       push ecx
004f33e2  89d3                     mov ebx, edx
004f33e4  c1e302                   shl ebx, 2
004f33e7  31d2                     xor edx, edx
004f33e9  8b0d2ccf5600             mov ecx, dword ptr [0x56cf2c]
004f33ef  8908                     mov dword ptr [eax], ecx
004f33f1  894804                   mov dword ptr [eax + 4], ecx
004f33f4  894808                   mov dword ptr [eax + 8], ecx
004f33f7  89480c                   mov dword ptr [eax + 0xc], ecx
004f33fa  894810                   mov dword ptr [eax + 0x10], ecx
004f33fd  894814                   mov dword ptr [eax + 0x14], ecx
004f3400  894818                   mov dword ptr [eax + 0x18], ecx
004f3403  42                       inc edx
004f3404  89481c                   mov dword ptr [eax + 0x1c], ecx
004f3407  01d8                     add eax, ebx
004f3409  83fa08                   cmp edx, 8
004f340c  7cdb                     jl 0x4f33e9
004f340e  59                       pop ecx
004f340f  5b                       pop ebx
004f3410  c3                       ret 
004f3411  8d8000000000             lea eax, [eax]
004f3417  8d9200000000             lea edx, [edx]
004f341d  8d4000                   lea eax, [eax]

