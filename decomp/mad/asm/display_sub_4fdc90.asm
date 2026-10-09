==== sub_4fdc90 .. 4fe1d0
004fdc90  56                       push esi
004fdc91  57                       push edi
004fdc92  55                       push ebp
004fdc93  81ec480a0000             sub esp, 0xa48
004fdc99  89c6                     mov esi, eax
004fdc9b  899424100a0000           mov dword ptr [esp + 0xa10], edx
004fdca2  899c240c0a0000           mov dword ptr [esp + 0xa0c], ebx
004fdca9  89cf                     mov edi, ecx
004fdcab  8b94245c0a0000           mov edx, dword ptr [esp + 0xa5c]
004fdcb2  85d2                     test edx, edx
004fdcb4  0f85f4010000             jne 0x4fdeae
004fdcba  b901000000               mov ecx, 1
004fdcbf  31d2                     xor edx, edx
004fdcc1  8a1510505600             mov dl, byte ptr [0x565010]
004fdcc7  83fa0f                   cmp edx, 0xf
004fdcca  7409                     je 0x4fdcd5
004fdccc  83fa10                   cmp edx, 0x10
004fdccf  0f8503020000             jne 0x4fded8
004fdcd5  3b1518649f00             cmp edx, dword ptr [0x9f6418]
004fdcdb  740d                     je 0x4fdcea
004fdcdd  89d0                     mov eax, edx
004fdcdf  e83cfdffff               call 0x4fda20
004fdce4  891518649f00             mov dword ptr [0x9f6418], edx
004fdcea  31db                     xor ebx, ebx
004fdcec  31c0                     xor eax, eax
004fdcee  8b1524505600             mov edx, dword ptr [0x565024]
004fdcf4  8b0482                   mov eax, dword ptr [edx + eax*4]
004fdcf7  8b1520505600             mov edx, dword ptr [0x565020]
004fdcfd  03049a                   add eax, dword ptr [edx + ebx*4]
004fdd00  030514505600             add eax, dword ptr [0x565014]
004fdd06  83e003                   and eax, 3
004fdd09  6683e6fe                 and si, 0xfffe
004fdd0d  d1f8                     sar eax, 1
004fdd0f  09c6                     or esi, eax
004fdd11  89c8                     mov eax, ecx
004fdd13  0fafc7                   imul eax, edi
004fdd16  8d1406                   lea edx, [esi + eax]
004fdd19  8b8424580a0000           mov eax, dword ptr [esp + 0xa58]
004fdd20  0fafc1                   imul eax, ecx
004fdd23  899424080a0000           mov dword ptr [esp + 0xa08], edx
004fdd2a  8b9424100a0000           mov edx, dword ptr [esp + 0xa10]
004fdd31  01c2                     add edx, eax
004fdd33  899424440a0000           mov dword ptr [esp + 0xa44], edx
004fdd3a  89fa                     mov edx, edi
004fdd3c  89f8                     mov eax, edi
004fdd3e  c1fa1f                   sar edx, 0x1f
004fdd41  2bc2                     sub eax, edx
004fdd43  d1f8                     sar eax, 1
004fdd45  8b1500505600             mov edx, dword ptr [0x565000]
004fdd4b  898424040a0000           mov dword ptr [esp + 0xa04], eax
004fdd52  39d6                     cmp esi, edx
004fdd54  7d22                     jge 0x4fdd78
004fdd56  8d1c09                   lea ebx, [ecx + ecx]
004fdd59  89d0                     mov eax, edx
004fdd5b  8d53ff                   lea edx, [ebx - 1]
004fdd5e  29f0                     sub eax, esi
004fdd60  01c2                     add edx, eax
004fdd62  89d0                     mov eax, edx
004fdd64  c1fa1f                   sar edx, 0x1f
004fdd67  f7fb                     idiv ebx
004fdd69  0fafd8                   imul ebx, eax
004fdd6c  c1e002                   shl eax, 2
004fdd6f  01de                     add esi, ebx
004fdd71  0184240c0a0000           add dword ptr [esp + 0xa0c], eax
004fdd78  8b8424080a0000           mov eax, dword ptr [esp + 0xa08]
004fdd7f  8b3d08505600             mov edi, dword ptr [0x565008]
004fdd85  39f8                     cmp eax, edi
004fdd87  7e07                     jle 0x4fdd90
004fdd89  89bc24080a0000           mov dword ptr [esp + 0xa08], edi
004fdd90  8b8424100a0000           mov eax, dword ptr [esp + 0xa10]
004fdd97  8b2d04505600             mov ebp, dword ptr [0x565004]
004fdd9d  39e8                     cmp eax, ebp
004fdd9f  7d47                     jge 0x4fdde8
004fdda1  8b9424100a0000           mov edx, dword ptr [esp + 0xa10]
004fdda8  89e8                     mov eax, ebp
004fddaa  29d0                     sub eax, edx
004fddac  8d51ff                   lea edx, [ecx - 1]
004fddaf  01c2                     add edx, eax
004fddb1  89d0                     mov eax, edx
004fddb3  c1fa1f                   sar edx, 0x1f
004fddb6  f7f9                     idiv ecx
004fddb8  89ca                     mov edx, ecx
004fddba  0fafd0                   imul edx, eax
004fddbd  0faf8424040a0000         imul eax, dword ptr [esp + 0xa04]
004fddc5  8b9c24100a0000           mov ebx, dword ptr [esp + 0xa10]
004fddcc  8bac240c0a0000           mov ebp, dword ptr [esp + 0xa0c]
004fddd3  c1e002                   shl eax, 2
004fddd6  01d3                     add ebx, edx
004fddd8  01c5                     add ebp, eax
004fddda  899c24100a0000           mov dword ptr [esp + 0xa10], ebx
004fdde1  89ac240c0a0000           mov dword ptr [esp + 0xa0c], ebp
004fdde8  8b8424440a0000           mov eax, dword ptr [esp + 0xa44]
004fddef  8b150c505600             mov edx, dword ptr [0x56500c]
004fddf5  39d0                     cmp eax, edx
004fddf7  7e07                     jle 0x4fde00
004fddf9  899424440a0000           mov dword ptr [esp + 0xa44], edx
004fde00  8b9424080a0000           mov edx, dword ptr [esp + 0xa08]
004fde07  29f2                     sub edx, esi
004fde09  01c9                     add ecx, ecx
004fde0b  89d0                     mov eax, edx
004fde0d  c1fa1f                   sar edx, 0x1f
004fde10  f7f9                     idiv ecx
004fde12  89c7                     mov edi, eax
004fde14  83f802                   cmp eax, 2
004fde17  0f8c83000000             jl 0x4fdea0
004fde1d  8b8424040a0000           mov eax, dword ptr [esp + 0xa04]
004fde24  8b8c245c0a0000           mov ecx, dword ptr [esp + 0xa5c]
004fde2b  c1e002                   shl eax, 2
004fde2e  85c9                     test ecx, ecx
004fde30  0f85b3000000             jne 0x4fdee9
004fde36  8bac24100a0000           mov ebp, dword ptr [esp + 0xa10]
004fde3d  8b9c24440a0000           mov ebx, dword ptr [esp + 0xa44]
004fde44  8b8c240c0a0000           mov ecx, dword ptr [esp + 0xa0c]
004fde4b  39dd                     cmp ebp, ebx
004fde4d  7d51                     jge 0x4fdea0
004fde4f  898424200a0000           mov dword ptr [esp + 0xa20], eax
004fde56  89eb                     mov ebx, ebp
004fde58  89f0                     mov eax, esi
004fde5a  8b1524505600             mov edx, dword ptr [0x565024]
004fde60  8b0482                   mov eax, dword ptr [edx + eax*4]
004fde63  8b1520505600             mov edx, dword ptr [0x565020]
004fde69  03049a                   add eax, dword ptr [edx + ebx*4]
004fde6c  030514505600             add eax, dword ptr [0x565014]
004fde72  89fb                     mov ebx, edi
004fde74  89c2                     mov edx, eax
004fde76  89c8                     mov eax, ecx
004fde78  45                       inc ebp
004fde79  e89ef7ffff               call 0x4fd61c
004fde7e  8b8424200a0000           mov eax, dword ptr [esp + 0xa20]
004fde85  8b9424440a0000           mov edx, dword ptr [esp + 0xa44]
004fde8c  01c1                     add ecx, eax
004fde8e  39d5                     cmp ebp, edx
004fde90  7cc4                     jl 0x4fde56
004fde92  8d8000000000             lea eax, [eax]
004fde98  8d9200000000             lea edx, [edx]
004fde9e  8bc0                     mov eax, eax
004fdea0  31c0                     xor eax, eax
004fdea2  81c4480a0000             add esp, 0xa48
004fdea8  5d                       pop ebp
004fdea9  5f                       pop edi
004fdeaa  5e                       pop esi
004fdeab  c20800                   ret 8
004fdeae  83fa01                   cmp edx, 1
004fdeb1  750a                     jne 0x4fdebd
004fdeb3  b902000000               mov ecx, 2
004fdeb8  e902feffff               jmp 0x4fdcbf
004fdebd  83fa02                   cmp edx, 2
004fdec0  74f1                     je 0x4fdeb3
004fdec2  83fa03                   cmp edx, 3
004fdec5  74ec                     je 0x4fdeb3
004fdec7  b8ffffffff               mov eax, 0xffffffff
004fdecc  81c4480a0000             add esp, 0xa48
004fded2  5d                       pop ebp
004fded3  5f                       pop edi
004fded4  5e                       pop esi
004fded5  c20800                   ret 8
004fded8  b8feffffff               mov eax, 0xfffffffe
004fdedd  81c4480a0000             add esp, 0xa48
004fdee3  5d                       pop ebp
004fdee4  5f                       pop edi
004fdee5  5e                       pop esi
004fdee6  c20800                   ret 8
004fdee9  83f903                   cmp ecx, 3
004fdeec  0f857c010000             jne 0x4fe06e
004fdef2  8bac24100a0000           mov ebp, dword ptr [esp + 0xa10]
004fdef9  8b9c24440a0000           mov ebx, dword ptr [esp + 0xa44]
004fdf00  8b8c240c0a0000           mov ecx, dword ptr [esp + 0xa0c]
004fdf07  39dd                     cmp ebp, ebx
004fdf09  0f8d71000000             jge 0x4fdf80
004fdf0f  89fa                     mov edx, edi
004fdf11  4a                       dec edx
004fdf12  899424180a0000           mov dword ptr [esp + 0xa18], edx
004fdf19  8d143f                   lea edx, [edi + edi]
004fdf1c  83ea02                   sub edx, 2
004fdf1f  898424140a0000           mov dword ptr [esp + 0xa14], eax
004fdf26  899424280a0000           mov dword ptr [esp + 0xa28], edx
004fdf2d  8b9c24180a0000           mov ebx, dword ptr [esp + 0xa18]
004fdf34  89e2                     mov edx, esp
004fdf36  89c8                     mov eax, ecx
004fdf38  e855f9ffff               call 0x4fd892
004fdf3d  89eb                     mov ebx, ebp
004fdf3f  89f0                     mov eax, esi
004fdf41  8b1524505600             mov edx, dword ptr [0x565024]
004fdf47  8b0482                   mov eax, dword ptr [edx + eax*4]
004fdf4a  8b1520505600             mov edx, dword ptr [0x565020]
004fdf50  03049a                   add eax, dword ptr [edx + ebx*4]
004fdf53  030514505600             add eax, dword ptr [0x565014]
004fdf59  8b9c24280a0000           mov ebx, dword ptr [esp + 0xa28]
004fdf60  89c2                     mov edx, eax
004fdf62  89e0                     mov eax, esp
004fdf64  83c502                   add ebp, 2
004fdf67  e8b0f6ffff               call 0x4fd61c
004fdf6c  8b8424140a0000           mov eax, dword ptr [esp + 0xa14]
004fdf73  8b9424440a0000           mov edx, dword ptr [esp + 0xa44]
004fdf7a  01c1                     add ecx, eax
004fdf7c  39d5                     cmp ebp, edx
004fdf7e  7cad                     jl 0x4fdf2d
004fdf80  8b8424440a0000           mov eax, dword ptr [esp + 0xa44]
004fdf87  8b8c24100a0000           mov ecx, dword ptr [esp + 0xa10]
004fdf8e  8b9424080a0000           mov edx, dword ptr [esp + 0xa08]
004fdf95  29c8                     sub eax, ecx
004fdf97  29f2                     sub edx, esi
004fdf99  0fafc2                   imul eax, edx
004fdf9c  3d00580200               cmp eax, 0x25800
004fdfa1  0f8fae000000             jg 0x4fe055
004fdfa7  8bac24100a0000           mov ebp, dword ptr [esp + 0xa10]
004fdfae  8d57ff                   lea edx, [edi - 1]
004fdfb1  8b84240c0a0000           mov eax, dword ptr [esp + 0xa0c]
004fdfb8  899424340a0000           mov dword ptr [esp + 0xa34], edx
004fdfbf  8b9424040a0000           mov edx, dword ptr [esp + 0xa04]
004fdfc6  45                       inc ebp
004fdfc7  c1e202                   shl edx, 2
004fdfca  01ff                     add edi, edi
004fdfcc  899424240a0000           mov dword ptr [esp + 0xa24], edx
004fdfd3  8b9424440a0000           mov edx, dword ptr [esp + 0xa44]
004fdfda  83ef02                   sub edi, 2
004fdfdd  4a                       dec edx
004fdfde  89bc24300a0000           mov dword ptr [esp + 0xa30], edi
004fdfe5  899424380a0000           mov dword ptr [esp + 0xa38], edx
004fdfec  39d5                     cmp ebp, edx
004fdfee  0f8dacfeffff             jge 0x4fdea0
004fdff4  8bbc24240a0000           mov edi, dword ptr [esp + 0xa24]
004fdffb  8b8c24340a0000           mov ecx, dword ptr [esp + 0xa34]
004fe002  01c7                     add edi, eax
004fe004  89e3                     mov ebx, esp
004fe006  89fa                     mov edx, edi
004fe008  e8e1f8ffff               call 0x4fd8ee
004fe00d  89eb                     mov ebx, ebp
004fe00f  89f0                     mov eax, esi
004fe011  8b1524505600             mov edx, dword ptr [0x565024]
004fe017  8b0482                   mov eax, dword ptr [edx + eax*4]
004fe01a  8b1520505600             mov edx, dword ptr [0x565020]
004fe020  03049a                   add eax, dword ptr [edx + ebx*4]
004fe023  030514505600             add eax, dword ptr [0x565014]
004fe029  8b9c24300a0000           mov ebx, dword ptr [esp + 0xa30]
004fe030  89c2                     mov edx, eax
004fe032  89e0                     mov eax, esp
004fe034  83c502                   add ebp, 2
004fe037  e8e0f5ffff               call 0x4fd61c
004fe03c  89f8                     mov eax, edi
004fe03e  3bac24380a0000           cmp ebp, dword ptr [esp + 0xa38]
004fe045  7cad                     jl 0x4fdff4
004fe047  31c0                     xor eax, eax
004fe049  81c4480a0000             add esp, 0xa48
004fe04f  5d                       pop ebp
004fe050  5f                       pop edi
004fe051  5e                       pop esi
004fe052  c20800                   ret 8
004fe055  e886b0ffff               call 0x4f90e0
004fe05a  85c0                     test eax, eax
004fe05c  0f8545ffffff             jne 0x4fdfa7
004fe062  81c4480a0000             add esp, 0xa48
004fe068  5d                       pop ebp
004fe069  5f                       pop edi
004fe06a  5e                       pop esi
004fe06b  c20800                   ret 8
004fe06e  8bac24100a0000           mov ebp, dword ptr [esp + 0xa10]
004fe075  8b9c24440a0000           mov ebx, dword ptr [esp + 0xa44]
004fe07c  8b8c240c0a0000           mov ecx, dword ptr [esp + 0xa0c]
004fe083  39dd                     cmp ebp, ebx
004fe085  7d49                     jge 0x4fe0d0
004fe087  8984241c0a0000           mov dword ptr [esp + 0xa1c], eax
004fe08e  89eb                     mov ebx, ebp
004fe090  89f0                     mov eax, esi
004fe092  8b1524505600             mov edx, dword ptr [0x565024]
004fe098  8b0482                   mov eax, dword ptr [edx + eax*4]
004fe09b  8b1520505600             mov edx, dword ptr [0x565020]
004fe0a1  03049a                   add eax, dword ptr [edx + ebx*4]
004fe0a4  030514505600             add eax, dword ptr [0x565014]
004fe0aa  89fb                     mov ebx, edi
004fe0ac  89c2                     mov edx, eax
004fe0ae  89c8                     mov eax, ecx
004fe0b0  83c502                   add ebp, 2
004fe0b3  e837f6ffff               call 0x4fd6ef
004fe0b8  8b84241c0a0000           mov eax, dword ptr [esp + 0xa1c]
004fe0bf  8b9424440a0000           mov edx, dword ptr [esp + 0xa44]
004fe0c6  01c1                     add ecx, eax
004fe0c8  39d5                     cmp ebp, edx
004fe0ca  7cc2                     jl 0x4fe08e
004fe0cc  8d442000                 lea eax, [eax]
004fe0d0  83bc245c0a000002         cmp dword ptr [esp + 0xa5c], 2
004fe0d8  0f85c2fdffff             jne 0x4fdea0
004fe0de  8b9424440a0000           mov edx, dword ptr [esp + 0xa44]
004fe0e5  8b9c24100a0000           mov ebx, dword ptr [esp + 0xa10]
004fe0ec  8b8424080a0000           mov eax, dword ptr [esp + 0xa08]
004fe0f3  29da                     sub edx, ebx
004fe0f5  29f0                     sub eax, esi
004fe0f7  0fafc2                   imul eax, edx
004fe0fa  3d00580200               cmp eax, 0x25800
004fe0ff  0f8fab000000             jg 0x4fe1b0
004fe105  8b84240c0a0000           mov eax, dword ptr [esp + 0xa0c]
004fe10c  898424400a0000           mov dword ptr [esp + 0xa40], eax
004fe113  8b8424040a0000           mov eax, dword ptr [esp + 0xa04]
004fe11a  c1e002                   shl eax, 2
004fe11d  8984242c0a0000           mov dword ptr [esp + 0xa2c], eax
004fe124  8b8424440a0000           mov eax, dword ptr [esp + 0xa44]
004fe12b  8bac24100a0000           mov ebp, dword ptr [esp + 0xa10]
004fe132  48                       dec eax
004fe133  45                       inc ebp
004fe134  8984243c0a0000           mov dword ptr [esp + 0xa3c], eax
004fe13b  39c5                     cmp ebp, eax
004fe13d  0f8d5dfdffff             jge 0x4fdea0
004fe143  89eb                     mov ebx, ebp
004fe145  89f0                     mov eax, esi
004fe147  8b8c242c0a0000           mov ecx, dword ptr [esp + 0xa2c]
004fe14e  8b1524505600             mov edx, dword ptr [0x565024]
004fe154  8b0482                   mov eax, dword ptr [edx + eax*4]
004fe157  8b1520505600             mov edx, dword ptr [0x565020]
004fe15d  03049a                   add eax, dword ptr [edx + ebx*4]
004fe160  030514505600             add eax, dword ptr [0x565014]
004fe166  8b9424400a0000           mov edx, dword ptr [esp + 0xa40]
004fe16d  89c3                     mov ebx, eax
004fe16f  83c502                   add ebp, 2
004fe172  8b8424400a0000           mov eax, dword ptr [esp + 0xa40]
004fe179  01ca                     add edx, ecx
004fe17b  89f9                     mov ecx, edi
004fe17d  899424000a0000           mov dword ptr [esp + 0xa00], edx
004fe184  e82df6ffff               call 0x4fd7b6
004fe189  8b8424000a0000           mov eax, dword ptr [esp + 0xa00]
004fe190  8b9c243c0a0000           mov ebx, dword ptr [esp + 0xa3c]
004fe197  898424400a0000           mov dword ptr [esp + 0xa40], eax
004fe19e  39dd                     cmp ebp, ebx
004fe1a0  7ca1                     jl 0x4fe143
004fe1a2  31c0                     xor eax, eax
004fe1a4  81c4480a0000             add esp, 0xa48
004fe1aa  5d                       pop ebp
004fe1ab  5f                       pop edi
004fe1ac  5e                       pop esi
004fe1ad  c20800                   ret 8
004fe1b0  e82bafffff               call 0x4f90e0
004fe1b5  85c0                     test eax, eax
004fe1b7  0f8548ffffff             jne 0x4fe105
004fe1bd  81c4480a0000             add esp, 0xa48
004fe1c3  5d                       pop ebp
004fe1c4  5f                       pop edi
004fe1c5  5e                       pop esi
004fe1c6  c20800                   ret 8
004fe1c9  0000                     add byte ptr [eax], al
004fe1cb  0000                     add byte ptr [eax], al
004fe1cd  0000                     add byte ptr [eax], al

