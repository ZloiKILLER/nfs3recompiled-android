==== sub_4f34f0 .. 4f3560
004f34f0  56                       push esi
004f34f1  57                       push edi
004f34f2  89c6                     mov esi, eax
004f34f4  89df                     mov edi, ebx
004f34f6  89d0                     mov eax, edx
004f34f8  89ca                     mov edx, ecx
004f34fa  8b4c240c                 mov ecx, dword ptr [esp + 0xc]
004f34fe  8d3cdd00000000           lea edi, [ebx*8]
004f3505  01f0                     add eax, esi
004f3507  31f6                     xor esi, esi
004f3509  8a18                     mov bl, byte ptr [eax]
004f350b  00cb                     add bl, cl
004f350d  885a02                   mov byte ptr [edx + 2], bl
004f3510  8a5804                   mov bl, byte ptr [eax + 4]
004f3513  00cb                     add bl, cl
004f3515  885a06                   mov byte ptr [edx + 6], bl
004f3518  8a5808                   mov bl, byte ptr [eax + 8]
004f351b  00cb                     add bl, cl
004f351d  885a0a                   mov byte ptr [edx + 0xa], bl
004f3520  8a580c                   mov bl, byte ptr [eax + 0xc]
004f3523  00cb                     add bl, cl
004f3525  885a0e                   mov byte ptr [edx + 0xe], bl
004f3528  8a5810                   mov bl, byte ptr [eax + 0x10]
004f352b  00cb                     add bl, cl
004f352d  885a12                   mov byte ptr [edx + 0x12], bl
004f3530  8a5814                   mov bl, byte ptr [eax + 0x14]
004f3533  00cb                     add bl, cl
004f3535  885a16                   mov byte ptr [edx + 0x16], bl
004f3538  8a5818                   mov bl, byte ptr [eax + 0x18]
004f353b  00cb                     add bl, cl
004f353d  885a1a                   mov byte ptr [edx + 0x1a], bl
004f3540  83c220                   add edx, 0x20
004f3543  8a581c                   mov bl, byte ptr [eax + 0x1c]
004f3546  46                       inc esi
004f3547  00cb                     add bl, cl
004f3549  01f8                     add eax, edi
004f354b  885afe                   mov byte ptr [edx - 2], bl
004f354e  83fe08                   cmp esi, 8
004f3551  7cb6                     jl 0x4f3509
004f3553  5f                       pop edi
004f3554  5e                       pop esi
004f3555  c20400                   ret 4
004f3558  8d8000000000             lea eax, [eax]
004f355e  8bd2                     mov edx, edx

