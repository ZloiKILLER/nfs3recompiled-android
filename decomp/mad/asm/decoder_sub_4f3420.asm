==== sub_4f3420 .. 4f34f0
004f3420  56                       push esi
004f3421  57                       push edi
004f3422  55                       push ebp
004f3423  89c6                     mov esi, eax
004f3425  89d5                     mov ebp, edx
004f3427  89df                     mov edi, ebx
004f3429  89c8                     mov eax, ecx
004f342b  8b4c2410                 mov ecx, dword ptr [esp + 0x10]
004f342f  8d3c9d00000000           lea edi, [ebx*4]
004f3436  89f2                     mov edx, esi
004f3438  85ed                     test ebp, ebp
004f343a  7453                     je 0x4f348f
004f343c  31f6                     xor esi, esi
004f343e  8a5a02                   mov bl, byte ptr [edx + 2]
004f3441  00cb                     add bl, cl
004f3443  885802                   mov byte ptr [eax + 2], bl
004f3446  8a5a07                   mov bl, byte ptr [edx + 7]
004f3449  00cb                     add bl, cl
004f344b  885806                   mov byte ptr [eax + 6], bl
004f344e  8a5a06                   mov bl, byte ptr [edx + 6]
004f3451  00cb                     add bl, cl
004f3453  88580a                   mov byte ptr [eax + 0xa], bl
004f3456  8a5a0b                   mov bl, byte ptr [edx + 0xb]
004f3459  00cb                     add bl, cl
004f345b  88580e                   mov byte ptr [eax + 0xe], bl
004f345e  8a5a0a                   mov bl, byte ptr [edx + 0xa]
004f3461  00cb                     add bl, cl
004f3463  885812                   mov byte ptr [eax + 0x12], bl
004f3466  8a5a0f                   mov bl, byte ptr [edx + 0xf]
004f3469  00cb                     add bl, cl
004f346b  885816                   mov byte ptr [eax + 0x16], bl
004f346e  8a5a0e                   mov bl, byte ptr [edx + 0xe]
004f3471  00cb                     add bl, cl
004f3473  88581a                   mov byte ptr [eax + 0x1a], bl
004f3476  83c040                   add eax, 0x40
004f3479  8a5a13                   mov bl, byte ptr [edx + 0x13]
004f347c  46                       inc esi
004f347d  00cb                     add bl, cl
004f347f  01fa                     add edx, edi
004f3481  8858de                   mov byte ptr [eax - 0x22], bl
004f3484  83fe08                   cmp esi, 8
004f3487  7cb5                     jl 0x4f343e
004f3489  5d                       pop ebp
004f348a  5f                       pop edi
004f348b  5e                       pop esi
004f348c  c20400                   ret 4
004f348f  31f6                     xor esi, esi
004f3491  8a5a03                   mov bl, byte ptr [edx + 3]
004f3494  00cb                     add bl, cl
004f3496  885802                   mov byte ptr [eax + 2], bl
004f3499  8a5a02                   mov bl, byte ptr [edx + 2]
004f349c  00cb                     add bl, cl
004f349e  885806                   mov byte ptr [eax + 6], bl
004f34a1  8a5a07                   mov bl, byte ptr [edx + 7]
004f34a4  00cb                     add bl, cl
004f34a6  88580a                   mov byte ptr [eax + 0xa], bl
004f34a9  8a5a06                   mov bl, byte ptr [edx + 6]
004f34ac  00cb                     add bl, cl
004f34ae  88580e                   mov byte ptr [eax + 0xe], bl
004f34b1  8a5a0b                   mov bl, byte ptr [edx + 0xb]
004f34b4  00cb                     add bl, cl
004f34b6  885812                   mov byte ptr [eax + 0x12], bl
004f34b9  8a5a0a                   mov bl, byte ptr [edx + 0xa]
004f34bc  00cb                     add bl, cl
004f34be  885816                   mov byte ptr [eax + 0x16], bl
004f34c1  8a5a0f                   mov bl, byte ptr [edx + 0xf]
004f34c4  00cb                     add bl, cl
004f34c6  88581a                   mov byte ptr [eax + 0x1a], bl
004f34c9  83c040                   add eax, 0x40
004f34cc  8a5a0e                   mov bl, byte ptr [edx + 0xe]
004f34cf  46                       inc esi
004f34d0  00cb                     add bl, cl
004f34d2  01fa                     add edx, edi
004f34d4  8858de                   mov byte ptr [eax - 0x22], bl
004f34d7  83fe08                   cmp esi, 8
004f34da  7cb5                     jl 0x4f3491
004f34dc  5d                       pop ebp
004f34dd  5f                       pop edi
004f34de  5e                       pop esi
004f34df  c20400                   ret 4
004f34e2  8d8000000000             lea eax, [eax]
004f34e8  8d9200000000             lea edx, [edx]
004f34ee  8bc0                     mov eax, eax

