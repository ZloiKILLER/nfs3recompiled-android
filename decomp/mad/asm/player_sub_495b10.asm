==== sub_495b10 .. 495b30
00495b10  55                       push ebp
00495b11  89e5                     mov ebp, esp
00495b13  b801000000               mov eax, 1
00495b18  e843befbff               call 0x451960
00495b1d  85c0                     test eax, eax
00495b1f  740a                     je 0x495b2b
00495b21  c70530f3790001000000     mov dword ptr [0x79f330], 1
00495b2b  5d                       pop ebp
00495b2c  c3                       ret 
00495b2d  8d4000                   lea eax, [eax]

