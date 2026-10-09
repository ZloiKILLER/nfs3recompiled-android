==== sub_510bb3 .. 510cbd
00510bb3  8b5e14                   mov ebx, dword ptr [esi + 0x14]
00510bb6  8b460c                   mov eax, dword ptr [esi + 0xc]
00510bb9  8b4e04                   mov ecx, dword ptr [esi + 4]
00510bbc  8b561c                   mov edx, dword ptr [esi + 0x1c]
00510bbf  8d3c03                   lea edi, [ebx + eax]
00510bc2  29c3                     sub ebx, eax
00510bc4  8d0411                   lea eax, [ecx + edx]
00510bc7  29d1                     sub ecx, edx
00510bc9  891de8cd5600             mov dword ptr [0x56cde8], ebx
00510bcf  890deccd5600             mov dword ptr [0x56cdec], ecx
00510bd5  8d2c38                   lea ebp, [eax + edi]
00510bd8  29f8                     sub eax, edi
00510bda  db05e8cd5600             fild dword ptr [0x56cde8]
00510be0  db05eccd5600             fild dword ptr [0x56cdec]
00510be6  d9c0                     fld st(0)
00510be8  d80d00ce5600             fmul dword ptr [0x56ce00]
00510bee  d9ca                     fxch st(2)
00510bf0  dcc1                     fadd st(1), st(0)
00510bf2  d80dfccd5600             fmul dword ptr [0x56cdfc]
00510bf8  d9c9                     fxch st(1)
00510bfa  d80d04ce5600             fmul dword ptr [0x56ce04]
00510c00  f72df8cd5600             imul dword ptr [0x56cdf8]
00510c06  dcc1                     fadd st(1), st(0)
00510c08  deea                     fsubp st(2)
00510c0a  d80508ce5600             fadd dword ptr [0x56ce08]
00510c10  d9c9                     fxch st(1)
00510c12  d80508ce5600             fadd dword ptr [0x56ce08]
00510c18  8b4608                   mov eax, dword ptr [esi + 8]
00510c1b  8b5e18                   mov ebx, dword ptr [esi + 0x18]
00510c1e  d1e2                     shl edx, 1
00510c20  dd1df0cd5600             fstp qword ptr [0x56cdf0]
00510c26  dd1de8cd5600             fstp qword ptr [0x56cde8]
00510c2c  8b0de8cd5600             mov ecx, dword ptr [0x56cde8]
00510c32  8b3df0cd5600             mov edi, dword ptr [0x56cdf0]
00510c38  01d1                     add ecx, edx
00510c3a  01fa                     add edx, edi
00510c3c  01ef                     add edi, ebp
00510c3e  890deccd5600             mov dword ptr [0x56cdec], ecx
00510c44  8915f0cd5600             mov dword ptr [0x56cdf0], edx
00510c4a  893df4cd5600             mov dword ptr [0x56cdf4], edi
00510c50  8b0e                     mov ecx, dword ptr [esi]
00510c52  8b5610                   mov edx, dword ptr [esi + 0x10]
00510c55  8d3418                   lea esi, [eax + ebx]
00510c58  29d8                     sub eax, ebx
00510c5a  8d1c11                   lea ebx, [ecx + edx]
00510c5d  29d1                     sub ecx, edx
00510c5f  f72df8cd5600             imul dword ptr [0x56cdf8]
00510c65  d1e2                     shl edx, 1
00510c67  8b3de4cd5600             mov edi, dword ptr [0x56cde4]
00510c6d  01d6                     add esi, edx
00510c6f  8b2deccd5600             mov ebp, dword ptr [0x56cdec]
00510c75  8d0411                   lea eax, [ecx + edx]
00510c78  29d1                     sub ecx, edx
00510c7a  8d1433                   lea edx, [ebx + esi]
00510c7d  29f3                     sub ebx, esi
00510c7f  8d3429                   lea esi, [ecx + ebp]
00510c82  29e9                     sub ecx, ebp
00510c84  8b2de8cd5600             mov ebp, dword ptr [0x56cde8]
00510c8a  897708                   mov dword ptr [edi + 8], esi
00510c8d  8b35f0cd5600             mov esi, dword ptr [0x56cdf0]
00510c93  894f14                   mov dword ptr [edi + 0x14], ecx
00510c96  8d0c2b                   lea ecx, [ebx + ebp]
00510c99  29eb                     sub ebx, ebp
00510c9b  8b2df4cd5600             mov ebp, dword ptr [0x56cdf4]
00510ca1  894f0c                   mov dword ptr [edi + 0xc], ecx
00510ca4  895f10                   mov dword ptr [edi + 0x10], ebx
00510ca7  8d1c30                   lea ebx, [eax + esi]
00510caa  29f0                     sub eax, esi
00510cac  8d0c2a                   lea ecx, [edx + ebp]
00510caf  29ea                     sub edx, ebp
00510cb1  890f                     mov dword ptr [edi], ecx
00510cb3  895f04                   mov dword ptr [edi + 4], ebx
00510cb6  894718                   mov dword ptr [edi + 0x18], eax
00510cb9  89571c                   mov dword ptr [edi + 0x1c], edx
00510cbc  c3                       ret 

