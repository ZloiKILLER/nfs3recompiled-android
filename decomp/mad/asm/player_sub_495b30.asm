==== sub_495b30 .. 495b60
00495b30  51                       push ecx
00495b31  52                       push edx
00495b32  55                       push ebp
00495b33  89e5                     mov ebp, esp
00495b35  89c1                     mov ecx, eax
00495b37  e8c4a70300               call 0x4d0300
00495b3c  85c0                     test eax, eax
00495b3e  740d                     je 0x495b4d
00495b40  89c2                     mov edx, eax
00495b42  89c8                     mov eax, ecx
00495b44  e877a80300               call 0x4d03c0
00495b49  89c8                     mov eax, ecx
00495b4b  ebea                     jmp 0x495b37
00495b4d  5d                       pop ebp
00495b4e  5a                       pop edx
00495b4f  59                       pop ecx
00495b50  c3                       ret 
00495b51  8d8000000000             lea eax, [eax]
00495b57  8d9200000000             lea edx, [edx]
00495b5d  8d4000                   lea eax, [eax]

