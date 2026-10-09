==== sub_495b60 .. 495bc0
00495b60  53                       push ebx
00495b61  51                       push ecx
00495b62  52                       push edx
00495b63  55                       push ebp
00495b64  89e5                     mov ebp, esp
00495b66  89c1                     mov ecx, eax
00495b68  89c8                     mov eax, ecx
00495b6a  e891a70300               call 0x4d0300
00495b6f  89c2                     mov edx, eax
00495b71  85c0                     test eax, eax
00495b73  742e                     je 0x495ba3
00495b75  8b18                     mov ebx, dword ptr [eax]
00495b77  81fb4d41446d             cmp ebx, 0x6d44414d
00495b7d  7410                     je 0x495b8f
00495b7f  81fb4d414465             cmp ebx, 0x6544414d
00495b85  7408                     je 0x495b8f
00495b87  81fb4d41446b             cmp ebx, 0x6b44414d
00495b8d  750d                     jne 0x495b9c
00495b8f  89d0                     mov eax, edx
00495b91  ff052cf37900             inc dword ptr [0x79f32c]
00495b97  5d                       pop ebp
00495b98  5a                       pop edx
00495b99  59                       pop ecx
00495b9a  5b                       pop ebx
00495b9b  c3                       ret 
00495b9c  89c8                     mov eax, ecx
00495b9e  e81da80300               call 0x4d03c0
00495ba3  89c8                     mov eax, ecx
00495ba5  e816a90300               call 0x4d04c0
00495baa  85c0                     test eax, eax
00495bac  74ba                     je 0x495b68
00495bae  31c0                     xor eax, eax
00495bb0  5d                       pop ebp
00495bb1  5a                       pop edx
00495bb2  59                       pop ecx
00495bb3  5b                       pop ebx
00495bb4  c3                       ret 
00495bb5  8d8000000000             lea eax, [eax]
00495bbb  8d5200                   lea edx, [edx]
00495bbe  8bdb                     mov ebx, ebx

