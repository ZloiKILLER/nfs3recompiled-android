==== sub_4df310 .. 4df340
004df310  53                       push ebx
004df311  51                       push ecx
004df312  52                       push edx
004df313  55                       push ebp
004df314  89e5                     mov ebp, esp
004df316  e865c70000               call 0x4eba80
004df31b  3b05c0a28c00             cmp eax, dword ptr [0x8ca2c0]
004df321  7518                     jne 0x4df33b
004df323  8b0dc8a28c00             mov ecx, dword ptr [0x8ca2c8]
004df329  85c9                     test ecx, ecx
004df32b  740e                     je 0x4df33b
004df32d  51                       push ecx
004df32e  8b11                     mov edx, dword ptr [ecx]
004df330  31db                     xor ebx, ebx
004df332  ff5208                   call dword ptr [edx + 8]
004df335  891dc8a28c00             mov dword ptr [0x8ca2c8], ebx
004df33b  5d                       pop ebp
004df33c  5a                       pop edx
004df33d  59                       pop ecx
004df33e  5b                       pop ebx
004df33f  c3                       ret 

