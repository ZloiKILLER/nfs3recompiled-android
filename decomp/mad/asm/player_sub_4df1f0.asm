==== sub_4df1f0 .. 4df2b0
004df1f0  56                       push esi
004df1f1  57                       push edi
004df1f2  55                       push ebp
004df1f3  89e5                     mov ebp, esp
004df1f5  81ecd8020000             sub esp, 0x2d8
004df1fb  89c6                     mov esi, eax
004df1fd  e80e010000               call 0x4df310
004df202  8b4510                   mov eax, dword ptr [ebp + 0x10]
004df205  8935e0a28c00             mov dword ptr [0x8ca2e0], esi
004df20b  8915dca28c00             mov dword ptr [0x8ca2dc], edx
004df211  891dd0a28c00             mov dword ptr [0x8ca2d0], ebx
004df217  890dc4a28c00             mov dword ptr [0x8ca2c4], ecx
004df21d  31ff                     xor edi, edi
004df21f  ba02000000               mov edx, 2
004df224  8b4d14                   mov ecx, dword ptr [ebp + 0x14]
004df227  893dd8a28c00             mov dword ptr [0x8ca2d8], edi
004df22d  893dc0a28c00             mov dword ptr [0x8ca2c0], edi
004df233  a3cca28c00               mov dword ptr [0x8ca2cc], eax
004df238  8915d4a28c00             mov dword ptr [0x8ca2d4], edx
004df23e  85c9                     test ecx, ecx
004df240  0f8561000000             jne 0x4df2a7
004df246  e845c80000               call 0x4eba90
004df24b  bb6c010000               mov ebx, 0x16c
004df250  8d9528fdffff             lea edx, [ebp - 0x2d8]
004df256  899d94feffff             mov dword ptr [ebp - 0x16c], ebx
004df25c  52                       push edx
004df25d  8d9594feffff             lea edx, [ebp - 0x16c]
004df263  899d28fdffff             mov dword ptr [ebp - 0x2d8], ebx
004df269  52                       push edx
004df26a  8b08                     mov ecx, dword ptr [eax]
004df26c  50                       push eax
004df26d  ff512c                   call dword ptr [ecx + 0x2c]
004df270  85c0                     test eax, eax
004df272  7533                     jne 0x4df2a7
004df274  8aa599feffff             mov ah, byte ptr [ebp - 0x167]
004df27a  f6c420                   test ah, 0x20
004df27d  7411                     je 0x4df290
004df27f  f6c440                   test ah, 0x40
004df282  740c                     je 0x4df290
004df284  c705d4a28c0001000000     mov dword ptr [0x8ca2d4], 1
004df28e  eb17                     jmp 0x4df2a7
004df290  8ab599feffff             mov dh, byte ptr [ebp - 0x167]
004df296  f6c601                   test dh, 1
004df299  740c                     je 0x4df2a7
004df29b  f6c602                   test dh, 2
004df29e  7407                     je 0x4df2a7
004df2a0  31c0                     xor eax, eax
004df2a2  a3d4a28c00               mov dword ptr [0x8ca2d4], eax
004df2a7  89ec                     mov esp, ebp
004df2a9  5d                       pop ebp
004df2aa  5f                       pop edi
004df2ab  5e                       pop esi
004df2ac  c20800                   ret 8
004df2af  90                       nop 

