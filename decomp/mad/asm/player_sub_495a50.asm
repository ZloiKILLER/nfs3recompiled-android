==== sub_495a50 .. 495b10
00495a50  53                       push ebx
00495a51  51                       push ecx
00495a52  52                       push edx
00495a53  56                       push esi
00495a54  57                       push edi
00495a55  55                       push ebp
00495a56  89e5                     mov ebp, esp
00495a58  83ec10                   sub esp, 0x10
00495a5b  8b0d18f37900             mov ecx, dword ptr [0x79f318]
00495a61  e82acd0500               call 0x4f2790
00495a66  8b1528f37900             mov edx, dword ptr [0x79f328]
00495a6c  8b3538f37900             mov esi, dword ptr [0x79f338]
00495a72  89c3                     mov ebx, eax
00495a74  89c7                     mov edi, eax
00495a76  29d3                     sub ebx, edx
00495a78  85f6                     test esi, esi
00495a7a  0f8472000000             je 0x495af2
00495a80  8d55f0                   lea edx, [ebp - 0x10]
00495a83  a13cf37900               mov eax, dword ptr [0x79f33c]
00495a88  e8c3d40500               call 0x4f2f50
00495a8d  8b45f4                   mov eax, dword ptr [ebp - 0xc]
00495a90  3b0520f37900             cmp eax, dword ptr [0x79f320]
00495a96  7e5a                     jle 0x495af2
00495a98  8b0d18f37900             mov ecx, dword ptr [0x79f318]
00495a9e  89c6                     mov esi, eax
00495aa0  a320f37900               mov dword ptr [0x79f320], eax
00495aa5  89ca                     mov edx, ecx
00495aa7  89c8                     mov eax, ecx
00495aa9  c1fa1f                   sar edx, 0x1f
00495aac  c1e203                   shl edx, 3
00495aaf  1bc2                     sbb eax, edx
00495ab1  c1f803                   sar eax, 3
00495ab4  29de                     sub esi, ebx
00495ab6  29c1                     sub ecx, eax
00495ab8  01f1                     add ecx, esi
00495aba  85c9                     test ecx, ecx
00495abc  7e04                     jle 0x495ac2
00495abe  89c8                     mov eax, ecx
00495ac0  eb04                     jmp 0x495ac6
00495ac2  89c8                     mov eax, ecx
00495ac4  f7d8                     neg eax
00495ac6  3d08010000               cmp eax, 0x108
00495acb  7e1f                     jle 0x495aec
00495acd  89ca                     mov edx, ecx
00495acf  89c8                     mov eax, ecx
00495ad1  c1fa1f                   sar edx, 0x1f
00495ad4  c1e203                   shl edx, 3
00495ad7  1bc2                     sbb eax, edx
00495ad9  c1f803                   sar eax, 3
00495adc  290528f37900             sub dword ptr [0x79f328], eax
00495ae2  2b3d28f37900             sub edi, dword ptr [0x79f328]
00495ae8  31c9                     xor ecx, ecx
00495aea  89fb                     mov ebx, edi
00495aec  890d18f37900             mov dword ptr [0x79f318], ecx
00495af2  8b0d18f37900             mov ecx, dword ptr [0x79f318]
00495af8  89d8                     mov eax, ebx
00495afa  89ec                     mov esp, ebp
00495afc  5d                       pop ebp
00495afd  5f                       pop edi
00495afe  5e                       pop esi
00495aff  5a                       pop edx
00495b00  59                       pop ecx
00495b01  5b                       pop ebx
00495b02  c3                       ret 
00495b03  8d8000000000             lea eax, [eax]
00495b09  8d9200000000             lea edx, [edx]
00495b0f  90                       nop 

