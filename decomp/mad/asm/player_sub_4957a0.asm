==== sub_4957a0 .. 495860
004957a0  56                       push esi
004957a1  57                       push edi
004957a2  55                       push ebp
004957a3  89e5                     mov ebp, esp
004957a5  8b7d10                   mov edi, dword ptr [ebp + 0x10]
004957a8  89c6                     mov esi, eax
004957aa  83ff10                   cmp edi, 0x10
004957ad  7e0d                     jle 0x4957bc
004957af  6858be5300               push 0x53be58
004957b4  e857b8f6ff               call 0x401010
004957b9  83c404                   add esp, 4
004957bc  891548f37900             mov dword ptr [0x79f348], edx
004957c2  891d50f37900             mov dword ptr [0x79f350], ebx
004957c8  890d44f37900             mov dword ptr [0x79f344], ecx
004957ce  893d58f37900             mov dword ptr [0x79f358], edi
004957d4  89354cf37900             mov dword ptr [0x79f34c], esi
004957da  31d2                     xor edx, edx
004957dc  31c9                     xor ecx, ecx
004957de  891554f37900             mov dword ptr [0x79f354], edx
004957e4  891540f37900             mov dword ptr [0x79f340], edx
004957ea  891560f37900             mov dword ptr [0x79f360], edx
004957f0  89155cf37900             mov dword ptr [0x79f35c], edx
004957f6  3b0d58f37900             cmp ecx, dword ptr [0x79f358]
004957fc  7d2a                     jge 0x495828
004957fe  8b1550f37900             mov edx, dword ptr [0x79f350]
00495804  a144f37900               mov eax, dword ptr [0x79f344]
00495809  01d2                     add edx, edx
0049580b  0fafd0                   imul edx, eax
0049580e  8b1df4435600             mov ebx, dword ptr [0x5643f4]
00495814  b888be5300               mov eax, 0x53be88
00495819  41                       inc ecx
0049581a  e801be0400               call 0x4e1620
0049581f  8904cd8cf27900           mov dword ptr [ecx*8 + 0x79f28c], eax
00495826  ebce                     jmp 0x4957f6
00495828  8b551c                   mov edx, dword ptr [ebp + 0x1c]
0049582b  8b4d18                   mov ecx, dword ptr [ebp + 0x18]
0049582e  52                       push edx
0049582f  8b1d50f37900             mov ebx, dword ptr [0x79f350]
00495835  a14cf37900               mov eax, dword ptr [0x79f34c]
0049583a  51                       push ecx
0049583b  8b1548f37900             mov edx, dword ptr [0x79f348]
00495841  8b0d44f37900             mov ecx, dword ptr [0x79f344]
00495847  e8a4990400               call 0x4df1f0
0049584c  5d                       pop ebp
0049584d  5f                       pop edi
0049584e  5e                       pop esi
0049584f  c21000                   ret 0x10
00495852  8d8000000000             lea eax, [eax]
00495858  8d9200000000             lea edx, [edx]
0049585e  8bc0                     mov eax, eax

