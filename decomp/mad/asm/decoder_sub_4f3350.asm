==== sub_4f3350 .. 4f33b0
004f3350  51                       push ecx
004f3351  52                       push edx
004f3352  56                       push esi
004f3353  8b1544259f00             mov edx, dword ptr [0x9f2544]
004f3359  8b353c259f00             mov esi, dword ptr [0x9f253c]
004f335f  88c1                     mov cl, al
004f3361  29c2                     sub edx, eax
004f3363  d3e6                     shl esi, cl
004f3365  83fa10                   cmp edx, 0x10
004f3368  7c10                     jl 0x4f337a
004f336a  89353c259f00             mov dword ptr [0x9f253c], esi
004f3370  891544259f00             mov dword ptr [0x9f2544], edx
004f3376  5e                       pop esi
004f3377  5a                       pop edx
004f3378  59                       pop ecx
004f3379  c3                       ret 
004f337a  53                       push ebx
004f337b  a148259f00               mov eax, dword ptr [0x9f2548]
004f3380  31db                     xor ebx, ebx
004f3382  b910000000               mov ecx, 0x10
004f3387  668b18                   mov bx, word ptr [eax]
004f338a  83c002                   add eax, 2
004f338d  29d1                     sub ecx, edx
004f338f  83c210                   add edx, 0x10
004f3392  d3e3                     shl ebx, cl
004f3394  a348259f00               mov dword ptr [0x9f2548], eax
004f3399  09de                     or esi, ebx
004f339b  5b                       pop ebx
004f339c  89353c259f00             mov dword ptr [0x9f253c], esi
004f33a2  891544259f00             mov dword ptr [0x9f2544], edx
004f33a8  5e                       pop esi
004f33a9  5a                       pop edx
004f33aa  59                       pop ecx
004f33ab  c3                       ret 
004f33ac  8d442000                 lea eax, [eax]

