==== sub_495860 .. 495890
00495860  52                       push edx
00495861  55                       push ebp
00495862  89e5                     mov ebp, esp
00495864  31d2                     xor edx, edx
00495866  3b1558f37900             cmp edx, dword ptr [0x79f358]
0049586c  7d0f                     jge 0x49587d
0049586e  8b04d594f27900           mov eax, dword ptr [edx*8 + 0x79f294]
00495875  e816c00400               call 0x4e1890
0049587a  42                       inc edx
0049587b  ebe9                     jmp 0x495866
0049587d  e88e9a0400               call 0x4df310
00495882  5d                       pop ebp
00495883  5a                       pop edx
00495884  c3                       ret 
00495885  8d8000000000             lea eax, [eax]
0049588b  8d5200                   lea edx, [edx]
0049588e  8bdb                     mov ebx, ebx

