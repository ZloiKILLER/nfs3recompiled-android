==== sub_495a10 .. 495a50
00495a10  52                       push edx
00495a11  55                       push ebp
00495a12  89e5                     mov ebp, esp
00495a14  a33cf37900               mov dword ptr [0x79f33c], eax
00495a19  85c0                     test eax, eax
00495a1b  7c07                     jl 0x495a24
00495a1d  b801000000               mov eax, 1
00495a22  eb02                     jmp 0x495a26
00495a24  31c0                     xor eax, eax
00495a26  a338f37900               mov dword ptr [0x79f338], eax
00495a2b  e860cd0500               call 0x4f2790
00495a30  31d2                     xor edx, edx
00495a32  a328f37900               mov dword ptr [0x79f328], eax
00495a37  891520f37900             mov dword ptr [0x79f320], edx
00495a3d  891518f37900             mov dword ptr [0x79f318], edx
00495a43  5d                       pop ebp
00495a44  5a                       pop edx
00495a45  c3                       ret 
00495a46  8d8000000000             lea eax, [eax]
00495a4c  8d542200                 lea edx, [edx]

