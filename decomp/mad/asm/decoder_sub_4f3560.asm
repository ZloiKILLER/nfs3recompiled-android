==== sub_4f3560 .. 4f3600
004f3560  51                       push ecx
004f3561  89c1                     mov ecx, eax
004f3563  833d38259f0000           cmp dword ptr [0x9f2538], 0
004f356a  0f8486000000             je 0x4f35f6
004f3570  57                       push edi
004f3571  56                       push esi
004f3572  bf20000000               mov edi, 0x20
004f3577  83c104                   add ecx, 4
004f357a  891540259f00             mov dword ptr [0x9f2540], edx
004f3580  31f6                     xor esi, esi
004f3582  31c0                     xor eax, eax
004f3584  8b152cd05600             mov edx, dword ptr [0x56d02c]
004f358a  893d44259f00             mov dword ptr [0x9f2544], edi
004f3590  668b71fc                 mov si, word ptr [ecx - 4]
004f3594  668b41fe                 mov ax, word ptr [ecx - 2]
004f3598  c1e610                   shl esi, 0x10
004f359b  890d48259f00             mov dword ptr [0x9f2548], ecx
004f35a1  09c6                     or esi, eax
004f35a3  a1c4605600               mov eax, dword ptr [0x5660c4]
004f35a8  b904000000               mov ecx, 4
004f35ad  c1e00f                   shl eax, 0xf
004f35b0  89353c259f00             mov dword ptr [0x9f253c], esi
004f35b6  f7ea                     imul edx
004f35b8  c1e210                   shl edx, 0x10
004f35bb  c1e810                   shr eax, 0x10
004f35be  11d0                     adc eax, edx
004f35c0  a3380c9f00               mov dword ptr [0x9f0c38], eax
004f35c5  5e                       pop esi
004f35c6  5f                       pop edi
004f35c7  8b81c4605600             mov eax, dword ptr [ecx + 0x5660c4]
004f35cd  0fafc3                   imul eax, ebx
004f35d0  8b912cd05600             mov edx, dword ptr [ecx + 0x56d02c]
004f35d6  c1e00c                   shl eax, 0xc
004f35d9  83c104                   add ecx, 4
004f35dc  f7ea                     imul edx
004f35de  c1e210                   shl edx, 0x10
004f35e1  c1e810                   shr eax, 0x10
004f35e4  11d0                     adc eax, edx
004f35e6  8981340c9f00             mov dword ptr [ecx + 0x9f0c34], eax
004f35ec  81f900010000             cmp ecx, 0x100
004f35f2  75d3                     jne 0x4f35c7
004f35f4  59                       pop ecx
004f35f5  c3                       ret 
004f35f6  e8d5faffff               call 0x4f30d0
004f35fb  e970ffffff               jmp 0x4f3570

