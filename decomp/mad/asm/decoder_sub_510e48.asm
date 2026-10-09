==== sub_510e48 .. 510ff8
00510e48  53                       push ebx
00510e49  51                       push ecx
00510e4a  52                       push edx
00510e4b  56                       push esi
00510e4c  57                       push edi
00510e4d  55                       push ebp
00510e4e  8b3548259f00             mov esi, dword ptr [0x9f2548]
00510e54  8b3d3c259f00             mov edi, dword ptr [0x9f253c]
00510e5a  8b2d44259f00             mov ebp, dword ptr [0x9f2544]
00510e60  89f8                     mov eax, edi
00510e62  c1f818                   sar eax, 0x18
00510e65  bb30cf5600               mov ebx, 0x56cf30
00510e6a  f72d380c9f00             imul dword ptr [0x9f0c38]
00510e70  a32ccf5600               mov dword ptr [0x56cf2c], eax
00510e75  31c0                     xor eax, eax
00510e77  c1e708                   shl edi, 8
00510e7a  83ed08                   sub ebp, 8
00510e7d  83fd10                   cmp ebp, 0x10
00510e80  7d14                     jge 0x510e96
00510e82  668b06                   mov ax, word ptr [esi]
00510e85  b910000000               mov ecx, 0x10
00510e8a  29e9                     sub ecx, ebp
00510e8c  83c602                   add esi, 2
00510e8f  d3e0                     shl eax, cl
00510e91  09c7                     or edi, eax
00510e93  83c510                   add ebp, 0x10
00510e96  31c0                     xor eax, eax
00510e98  b907000000               mov ecx, 7
00510e9d  8903                     mov dword ptr [ebx], eax
00510e9f  894304                   mov dword ptr [ebx + 4], eax
00510ea2  894308                   mov dword ptr [ebx + 8], eax
00510ea5  89430c                   mov dword ptr [ebx + 0xc], eax
00510ea8  894310                   mov dword ptr [ebx + 0x10], eax
00510eab  894314                   mov dword ptr [ebx + 0x14], eax
00510eae  894318                   mov dword ptr [ebx + 0x18], eax
00510eb1  89431c                   mov dword ptr [ebx + 0x1c], eax
00510eb4  894320                   mov dword ptr [ebx + 0x20], eax
00510eb7  83c324                   add ebx, 0x24
00510eba  49                       dec ecx
00510ebb  75e0                     jne 0x510e9d
00510ebd  bb01000000               mov ebx, 1
00510ec2  89f8                     mov eax, edi
00510ec4  c1e817                   shr eax, 0x17
00510ec7  8b148538199f00           mov edx, dword ptr [eax*4 + 0x9f1938]
00510ece  31c0                     xor eax, eax
00510ed0  80fa09                   cmp dl, 9
00510ed3  0f8eb9000000             jle 0x510f92
00510ed9  80fa20                   cmp dl, 0x20
00510edc  7362                     jae 0x510f40
00510ede  80fa10                   cmp dl, 0x10
00510ee1  7330                     jae 0x510f13
00510ee3  c1e709                   shl edi, 9
00510ee6  83ed09                   sub ebp, 9
00510ee9  83fd10                   cmp ebp, 0x10
00510eec  7d14                     jge 0x510f02
00510eee  668b06                   mov ax, word ptr [esi]
00510ef1  b910000000               mov ecx, 0x10
00510ef6  29e9                     sub ecx, ebp
00510ef8  83c602                   add esi, 2
00510efb  d3e0                     shl eax, cl
00510efd  09c7                     or edi, eax
00510eff  83c510                   add ebp, 0x10
00510f02  89f8                     mov eax, edi
00510f04  c1e818                   shr eax, 0x18
00510f07  8b148538119f00           mov edx, dword ptr [eax*4 + 0x9f1138]
00510f0e  e97f000000               jmp 0x510f92
00510f13  c1e706                   shl edi, 6
00510f16  83ed06                   sub ebp, 6
00510f19  83fd10                   cmp ebp, 0x10
00510f1c  7d14                     jge 0x510f32
00510f1e  668b06                   mov ax, word ptr [esi]
00510f21  b910000000               mov ecx, 0x10
00510f26  29e9                     sub ecx, ebp
00510f28  83c602                   add esi, 2
00510f2b  d3e0                     shl eax, cl
00510f2d  09c7                     or edi, eax
00510f2f  83c510                   add ebp, 0x10
00510f32  89f8                     mov eax, edi
00510f34  c1e818                   shr eax, 0x18
00510f37  8b148538159f00           mov edx, dword ptr [eax*4 + 0x9f1538]
00510f3e  eb52                     jmp 0x510f92
00510f40  80fa30                   cmp dl, 0x30
00510f43  732c                     jae 0x510f71
00510f45  c1e706                   shl edi, 6
00510f48  83ed06                   sub ebp, 6
00510f4b  83fd10                   cmp ebp, 0x10
00510f4e  7d14                     jge 0x510f64
00510f50  668b06                   mov ax, word ptr [esi]
00510f53  b910000000               mov ecx, 0x10
00510f58  29e9                     sub ecx, ebp
00510f5a  83c602                   add esi, 2
00510f5d  d3e0                     shl eax, cl
00510f5f  09c7                     or edi, eax
00510f61  83c510                   add ebp, 0x10
00510f64  89fa                     mov edx, edi
00510f66  81e20000ffff             and edx, 0xffff0000
00510f6c  83ca10                   or edx, 0x10
00510f6f  eb21                     jmp 0x510f92
00510f71  c1e702                   shl edi, 2
00510f74  83ed02                   sub ebp, 2
00510f77  83fd10                   cmp ebp, 0x10
00510f7a  7d61                     jge 0x510fdd
00510f7c  668b06                   mov ax, word ptr [esi]
00510f7f  b910000000               mov ecx, 0x10
00510f84  29e9                     sub ecx, ebp
00510f86  83c602                   add esi, 2
00510f89  d3e0                     shl eax, cl
00510f8b  09c7                     or edi, eax
00510f8d  83c510                   add ebp, 0x10
00510f90  eb4b                     jmp 0x510fdd
00510f92  89d0                     mov eax, edx
00510f94  31c9                     xor ecx, ecx
00510f96  c1e810                   shr eax, 0x10
00510f99  88d1                     mov cl, dl
00510f9b  83e03f                   and eax, 0x3f
00510f9e  29cd                     sub ebp, ecx
00510fa0  01c3                     add ebx, eax
00510fa2  31c0                     xor eax, eax
00510fa4  d3e7                     shl edi, cl
00510fa6  83fd10                   cmp ebp, 0x10
00510fa9  7d14                     jge 0x510fbf
00510fab  668b06                   mov ax, word ptr [esi]
00510fae  b910000000               mov ecx, 0x10
00510fb3  29e9                     sub ecx, ebp
00510fb5  83c602                   add esi, 2
00510fb8  d3e0                     shl eax, cl
00510fba  09c7                     or edi, eax
00510fbc  83c510                   add ebp, 0x10
00510fbf  8b0c9dcc885600           mov ecx, dword ptr [ebx*4 + 0x5688cc]
00510fc6  89d0                     mov eax, edx
00510fc8  c1f816                   sar eax, 0x16
00510fcb  43                       inc ebx
00510fcc  f7a9380c9f00             imul dword ptr [ecx + 0x9f0c38]
00510fd2  89812ccf5600             mov dword ptr [ecx + 0x56cf2c], eax
00510fd8  e9e5feffff               jmp 0x510ec2
00510fdd  893548259f00             mov dword ptr [0x9f2548], esi
00510fe3  893d3c259f00             mov dword ptr [0x9f253c], edi
00510fe9  892d44259f00             mov dword ptr [0x9f2544], ebp
00510fef  89d8                     mov eax, ebx
00510ff1  5d                       pop ebp
00510ff2  5f                       pop edi
00510ff3  5e                       pop esi
00510ff4  5a                       pop edx
00510ff5  59                       pop ecx
00510ff6  5b                       pop ebx
00510ff7  c3                       ret 

