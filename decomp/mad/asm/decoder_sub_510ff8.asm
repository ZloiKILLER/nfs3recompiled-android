==== sub_510ff8 .. 511200
00510ff8  51                       push ecx
00510ff9  56                       push esi
00510ffa  57                       push edi
00510ffb  55                       push ebp
00510ffc  89c6                     mov esi, eax
00510ffe  89d7                     mov edi, edx
00511000  89dd                     mov ebp, ebx
00511002  31c0                     xor eax, eax
00511004  31db                     xor ebx, ebx
00511006  31c9                     xor ecx, ecx
00511008  31d2                     xor edx, edx
0051100a  8a4602                   mov al, byte ptr [esi + 2]
0051100d  8a5f02                   mov bl, byte ptr [edi + 2]
00511010  8a4e06                   mov cl, byte ptr [esi + 6]
00511013  8a9702010000             mov dl, byte ptr [edi + 0x102]
00511019  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
0051101f  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
00511025  c1e008                   shl eax, 8
00511028  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
0051102e  c1e308                   shl ebx, 8
00511031  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
00511037  09c8                     or eax, ecx
00511039  09d3                     or ebx, edx
0051103b  c1e010                   shl eax, 0x10
0051103e  09d8                     or eax, ebx
00511040  31db                     xor ebx, ebx
00511042  894500                   mov dword ptr [ebp], eax
00511045  31c0                     xor eax, eax
00511047  8a460a                   mov al, byte ptr [esi + 0xa]
0051104a  8a5f06                   mov bl, byte ptr [edi + 6]
0051104d  8a4e0e                   mov cl, byte ptr [esi + 0xe]
00511050  8a9706010000             mov dl, byte ptr [edi + 0x106]
00511056  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
0051105c  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
00511062  c1e008                   shl eax, 8
00511065  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
0051106b  c1e308                   shl ebx, 8
0051106e  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
00511074  09c8                     or eax, ecx
00511076  09d3                     or ebx, edx
00511078  c1e010                   shl eax, 0x10
0051107b  09d8                     or eax, ebx
0051107d  31db                     xor ebx, ebx
0051107f  894504                   mov dword ptr [ebp + 4], eax
00511082  31c0                     xor eax, eax
00511084  8a4612                   mov al, byte ptr [esi + 0x12]
00511087  8a5f0a                   mov bl, byte ptr [edi + 0xa]
0051108a  8a4e16                   mov cl, byte ptr [esi + 0x16]
0051108d  8a970a010000             mov dl, byte ptr [edi + 0x10a]
00511093  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
00511099  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
0051109f  c1e008                   shl eax, 8
005110a2  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
005110a8  c1e308                   shl ebx, 8
005110ab  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
005110b1  09c8                     or eax, ecx
005110b3  09d3                     or ebx, edx
005110b5  c1e010                   shl eax, 0x10
005110b8  09d8                     or eax, ebx
005110ba  31db                     xor ebx, ebx
005110bc  894508                   mov dword ptr [ebp + 8], eax
005110bf  31c0                     xor eax, eax
005110c1  8a461a                   mov al, byte ptr [esi + 0x1a]
005110c4  8a5f0e                   mov bl, byte ptr [edi + 0xe]
005110c7  8a4e1e                   mov cl, byte ptr [esi + 0x1e]
005110ca  8a970e010000             mov dl, byte ptr [edi + 0x10e]
005110d0  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
005110d6  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
005110dc  c1e008                   shl eax, 8
005110df  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
005110e5  c1e308                   shl ebx, 8
005110e8  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
005110ee  09c8                     or eax, ecx
005110f0  09d3                     or ebx, edx
005110f2  c1e010                   shl eax, 0x10
005110f5  09d8                     or eax, ebx
005110f7  31db                     xor ebx, ebx
005110f9  89450c                   mov dword ptr [ebp + 0xc], eax
005110fc  31c0                     xor eax, eax
005110fe  8a4622                   mov al, byte ptr [esi + 0x22]
00511101  8a5f12                   mov bl, byte ptr [edi + 0x12]
00511104  8a4e26                   mov cl, byte ptr [esi + 0x26]
00511107  8a9712010000             mov dl, byte ptr [edi + 0x112]
0051110d  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
00511113  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
00511119  c1e008                   shl eax, 8
0051111c  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
00511122  c1e308                   shl ebx, 8
00511125  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
0051112b  09c8                     or eax, ecx
0051112d  09d3                     or ebx, edx
0051112f  c1e010                   shl eax, 0x10
00511132  09d8                     or eax, ebx
00511134  31db                     xor ebx, ebx
00511136  894510                   mov dword ptr [ebp + 0x10], eax
00511139  31c0                     xor eax, eax
0051113b  8a462a                   mov al, byte ptr [esi + 0x2a]
0051113e  8a5f16                   mov bl, byte ptr [edi + 0x16]
00511141  8a4e2e                   mov cl, byte ptr [esi + 0x2e]
00511144  8a9716010000             mov dl, byte ptr [edi + 0x116]
0051114a  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
00511150  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
00511156  c1e008                   shl eax, 8
00511159  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
0051115f  c1e308                   shl ebx, 8
00511162  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
00511168  09c8                     or eax, ecx
0051116a  09d3                     or ebx, edx
0051116c  c1e010                   shl eax, 0x10
0051116f  09d8                     or eax, ebx
00511171  31db                     xor ebx, ebx
00511173  894514                   mov dword ptr [ebp + 0x14], eax
00511176  31c0                     xor eax, eax
00511178  8a4632                   mov al, byte ptr [esi + 0x32]
0051117b  8a5f1a                   mov bl, byte ptr [edi + 0x1a]
0051117e  8a4e36                   mov cl, byte ptr [esi + 0x36]
00511181  8a971a010000             mov dl, byte ptr [edi + 0x11a]
00511187  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
0051118d  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
00511193  c1e008                   shl eax, 8
00511196  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
0051119c  c1e308                   shl ebx, 8
0051119f  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
005111a5  09c8                     or eax, ecx
005111a7  09d3                     or ebx, edx
005111a9  c1e010                   shl eax, 0x10
005111ac  09d8                     or eax, ebx
005111ae  31db                     xor ebx, ebx
005111b0  894518                   mov dword ptr [ebp + 0x18], eax
005111b3  31c0                     xor eax, eax
005111b5  8a463a                   mov al, byte ptr [esi + 0x3a]
005111b8  8a5f1e                   mov bl, byte ptr [edi + 0x1e]
005111bb  8a4e3e                   mov cl, byte ptr [esi + 0x3e]
005111be  8a971e010000             mov dl, byte ptr [edi + 0x11e]
005111c4  8a80380d9f00             mov al, byte ptr [eax + 0x9f0d38]
005111ca  8a9b380d9f00             mov bl, byte ptr [ebx + 0x9f0d38]
005111d0  c1e008                   shl eax, 8
005111d3  8a89380d9f00             mov cl, byte ptr [ecx + 0x9f0d38]
005111d9  c1e308                   shl ebx, 8
005111dc  8a92380d9f00             mov dl, byte ptr [edx + 0x9f0d38]
005111e2  09c8                     or eax, ecx
005111e4  09d3                     or ebx, edx
005111e6  c1e010                   shl eax, 0x10
005111e9  09d8                     or eax, ebx
005111eb  31db                     xor ebx, ebx
005111ed  89451c                   mov dword ptr [ebp + 0x1c], eax
005111f0  31c0                     xor eax, eax
005111f2  5d                       pop ebp
005111f3  5f                       pop edi
005111f4  5e                       pop esi
005111f5  59                       pop ecx
005111f6  c3                       ret 
005111f7  0000                     add byte ptr [eax], al
005111f9  0000                     add byte ptr [eax], al
005111fb  0000                     add byte ptr [eax], al
005111fd  0000                     add byte ptr [eax], al

