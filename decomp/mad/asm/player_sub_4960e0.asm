==== sub_4960e0 .. 4961e0
004960e0  53                       push ebx
004960e1  51                       push ecx
004960e2  52                       push edx
004960e3  56                       push esi
004960e4  57                       push edi
004960e5  55                       push ebp
004960e6  89e5                     mov ebp, esp
004960e8  81ec00010000             sub esp, 0x100
004960ee  81ed82000000             sub ebp, 0x82
004960f4  89c1                     mov ecx, eax
004960f6  8b15dc1f7a00             mov edx, dword ptr [0x7a1fdc]
004960fc  85d2                     test edx, edx
004960fe  0f85c5000000             jne 0x4961c9
00496104  beb4327a00               mov esi, 0x7a32b4
00496109  8d7d82                   lea edi, [ebp - 0x7e]
0049610c  57                       push edi
0049610d  8a06                     mov al, byte ptr [esi]
0049610f  8807                     mov byte ptr [edi], al
00496111  3c00                     cmp al, 0
00496113  7410                     je 0x496125
00496115  8a4601                   mov al, byte ptr [esi + 1]
00496118  83c602                   add esi, 2
0049611b  884701                   mov byte ptr [edi + 1], al
0049611e  83c702                   add edi, 2
00496121  3c00                     cmp al, 0
00496123  75e8                     jne 0x49610d
00496125  5f                       pop edi
00496126  8d7d82                   lea edi, [ebp - 0x7e]
00496129  89ce                     mov esi, ecx
0049612b  57                       push edi
0049612c  2bc9                     sub ecx, ecx
0049612e  49                       dec ecx
0049612f  b000                     mov al, 0
00496131  f2ae                     repne scasb al, byte ptr es:[edi]
00496133  4f                       dec edi
00496134  8a06                     mov al, byte ptr [esi]
00496136  8807                     mov byte ptr [edi], al
00496138  3c00                     cmp al, 0
0049613a  7410                     je 0x49614c
0049613c  8a4601                   mov al, byte ptr [esi + 1]
0049613f  83c602                   add esi, 2
00496142  884701                   mov byte ptr [edi + 1], al
00496145  83c702                   add edi, 2
00496148  3c00                     cmp al, 0
0049614a  75e8                     jne 0x496134
0049614c  5f                       pop edi
0049614d  8d4582                   lea eax, [ebp - 0x7e]
00496150  e80bad0400               call 0x4e0e60
00496155  85c0                     test eax, eax
00496157  0f846c000000             je 0x4961c9
0049615d  e8dea00500               call 0x4f0240
00496162  e8c9fb0300               call 0x4d5d30
00496167  e834a9f7ff               call 0x410aa0
0049616c  8b1da4525600             mov ebx, dword ptr [0x5652a4]
00496172  89557e                   mov dword ptr [ebp + 0x7e], edx
00496175  81fb406bed07             cmp ebx, 0x7ed6b40
0049617b  7d07                     jge 0x496184
0049617d  b901000000               mov ecx, 1
00496182  eb14                     jmp 0x496198
00496184  81fb00c2eb0b             cmp ebx, 0xbebc200
0049618a  7d07                     jge 0x496193
0049618c  b902000000               mov ecx, 2
00496191  eb05                     jmp 0x496198
00496193  b903000000               mov ecx, 3
00496198  813da4525600406bed07     cmp dword ptr [0x5652a4], 0x7ed6b40
004961a2  7d07                     jge 0x4961ab
004961a4  c7457e01000000           mov dword ptr [ebp + 0x7e], 1
004961ab  6a01                     push 1
004961ad  8d557e                   lea edx, [ebp + 0x7e]
004961b0  8d4582                   lea eax, [ebp - 0x7e]
004961b3  31db                     xor ebx, ebx
004961b5  e806faffff               call 0x495bc0
004961ba  e8b1fb0300               call 0x4d5d70
004961bf  e87ca00500               call 0x4f0240
004961c4  e867a1f7ff               call 0x410330
004961c9  b801000000               mov eax, 1
004961ce  8da582000000             lea esp, [ebp + 0x82]
004961d4  5d                       pop ebp
004961d5  5f                       pop edi
004961d6  5e                       pop esi
004961d7  5a                       pop edx
004961d8  59                       pop ecx
004961d9  5b                       pop ebx
004961da  c3                       ret 
004961db  8d4000                   lea eax, [eax]
004961de  8bc9                     mov ecx, ecx

