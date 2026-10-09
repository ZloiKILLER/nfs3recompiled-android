==== sub_495920 .. 495a00
00495920  53                       push ebx
00495921  51                       push ecx
00495922  52                       push edx
00495923  56                       push esi
00495924  57                       push edi
00495925  55                       push ebp
00495926  89e5                     mov ebp, esp
00495928  89c1                     mov ecx, eax
0049592a  833d54f3790000           cmp dword ptr [0x79f354], 0
00495931  7507                     jne 0x49593a
00495933  31c0                     xor eax, eax
00495935  e9b2000000               jmp 0x4959ec
0049593a  3b0590f27900             cmp eax, dword ptr [0x79f290]
00495940  7d09                     jge 0x49594b
00495942  31c0                     xor eax, eax
00495944  5d                       pop ebp
00495945  5f                       pop edi
00495946  5e                       pop esi
00495947  5a                       pop edx
00495948  59                       pop ecx
00495949  5b                       pop ebx
0049594a  c3                       ret 
0049594b  ba01000000               mov edx, 1
00495950  3b1554f37900             cmp edx, dword ptr [0x79f354]
00495956  7d0c                     jge 0x495964
00495958  3b0cd590f27900           cmp ecx, dword ptr [edx*8 + 0x79f290]
0049595f  7c03                     jl 0x495964
00495961  42                       inc edx
00495962  ebec                     jmp 0x495950
00495964  8b04d58cf27900           mov eax, dword ptr [edx*8 + 0x79f28c]
0049596b  e840990400               call 0x4df2b0
00495970  8b04d588f27900           mov eax, dword ptr [edx*8 + 0x79f288]
00495977  8b1d60f37900             mov ebx, dword ptr [0x79f360]
0049597d  29c1                     sub ecx, eax
0049597f  8b3540f37900             mov esi, dword ptr [0x79f340]
00495985  89c8                     mov eax, ecx
00495987  8b0d5cf37900             mov ecx, dword ptr [0x79f35c]
0049598d  46                       inc esi
0049598e  01c1                     add ecx, eax
00495990  8d42ff                   lea eax, [edx - 1]
00495993  893540f37900             mov dword ptr [0x79f340], esi
00495999  01c3                     add ebx, eax
0049599b  890d5cf37900             mov dword ptr [0x79f35c], ecx
004959a1  891d60f37900             mov dword ptr [0x79f360], ebx
004959a7  31c0                     xor eax, eax
004959a9  8b0d58f37900             mov ecx, dword ptr [0x79f358]
004959af  29d1                     sub ecx, edx
004959b1  39c8                     cmp eax, ecx
004959b3  7d24                     jge 0x4959d9
004959b5  8d0c10                   lea ecx, [eax + edx]
004959b8  8d3cc590f27900           lea edi, [eax*8 + 0x79f290]
004959bf  8d34cd90f27900           lea esi, [ecx*8 + 0x79f290]
004959c6  8b1cc594f27900           mov ebx, dword ptr [eax*8 + 0x79f294]
004959cd  a5                       movsd dword ptr es:[edi], dword ptr [esi]
004959ce  a5                       movsd dword ptr es:[edi], dword ptr [esi]
004959cf  40                       inc eax
004959d0  891ccd94f27900           mov dword ptr [ecx*8 + 0x79f294], ebx
004959d7  ebd0                     jmp 0x4959a9
004959d9  8b3d54f37900             mov edi, dword ptr [0x79f354]
004959df  29d7                     sub edi, edx
004959e1  b801000000               mov eax, 1
004959e6  893d54f37900             mov dword ptr [0x79f354], edi
004959ec  5d                       pop ebp
004959ed  5f                       pop edi
004959ee  5e                       pop esi
004959ef  5a                       pop edx
004959f0  59                       pop ecx
004959f1  5b                       pop ebx
004959f2  c3                       ret 
004959f3  8d8000000000             lea eax, [eax]
004959f9  8d9200000000             lea edx, [edx]
004959ff  90                       nop 

