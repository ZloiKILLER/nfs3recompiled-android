==== sub_510a50 .. 510bb3
00510a50  8b5e14                   mov ebx, dword ptr [esi + 0x14]
00510a53  8b460c                   mov eax, dword ptr [esi + 0xc]
00510a56  8b4e04                   mov ecx, dword ptr [esi + 4]
00510a59  8b561c                   mov edx, dword ptr [esi + 0x1c]
00510a5c  09d9                     or ecx, ebx
00510a5e  09c2                     or edx, eax
00510a60  8b7e08                   mov edi, dword ptr [esi + 8]
00510a63  8b6e18                   mov ebp, dword ptr [esi + 0x18]
00510a66  09ca                     or edx, ecx
00510a68  09ef                     or edi, ebp
00510a6a  8b4e10                   mov ecx, dword ptr [esi + 0x10]
00510a6d  09fa                     or edx, edi
00510a6f  09ca                     or edx, ecx
00510a71  0f8410010000             je 0x510b87
00510a77  8b4e04                   mov ecx, dword ptr [esi + 4]
00510a7a  8b561c                   mov edx, dword ptr [esi + 0x1c]
00510a7d  8d3c03                   lea edi, [ebx + eax]
00510a80  29c3                     sub ebx, eax
00510a82  8d0411                   lea eax, [ecx + edx]
00510a85  29d1                     sub ecx, edx
00510a87  891de8cd5600             mov dword ptr [0x56cde8], ebx
00510a8d  890deccd5600             mov dword ptr [0x56cdec], ecx
00510a93  8d2c38                   lea ebp, [eax + edi]
00510a96  29f8                     sub eax, edi
00510a98  db05e8cd5600             fild dword ptr [0x56cde8]
00510a9e  db05eccd5600             fild dword ptr [0x56cdec]
00510aa4  d9c0                     fld st(0)
00510aa6  d80d00ce5600             fmul dword ptr [0x56ce00]
00510aac  d9ca                     fxch st(2)
00510aae  dcc1                     fadd st(1), st(0)
00510ab0  d80dfccd5600             fmul dword ptr [0x56cdfc]
00510ab6  d9c9                     fxch st(1)
00510ab8  d80d04ce5600             fmul dword ptr [0x56ce04]
00510abe  f72df8cd5600             imul dword ptr [0x56cdf8]
00510ac4  dcc1                     fadd st(1), st(0)
00510ac6  deea                     fsubp st(2)
00510ac8  d80508ce5600             fadd dword ptr [0x56ce08]
00510ace  d9c9                     fxch st(1)
00510ad0  d80508ce5600             fadd dword ptr [0x56ce08]
00510ad6  8b4608                   mov eax, dword ptr [esi + 8]
00510ad9  8b5e18                   mov ebx, dword ptr [esi + 0x18]
00510adc  d1e2                     shl edx, 1
00510ade  dd1df0cd5600             fstp qword ptr [0x56cdf0]
00510ae4  dd1de8cd5600             fstp qword ptr [0x56cde8]
00510aea  8b0de8cd5600             mov ecx, dword ptr [0x56cde8]
00510af0  8b3df0cd5600             mov edi, dword ptr [0x56cdf0]
00510af6  01d1                     add ecx, edx
00510af8  01fa                     add edx, edi
00510afa  01ef                     add edi, ebp
00510afc  890deccd5600             mov dword ptr [0x56cdec], ecx
00510b02  8915f0cd5600             mov dword ptr [0x56cdf0], edx
00510b08  893df4cd5600             mov dword ptr [0x56cdf4], edi
00510b0e  8b0e                     mov ecx, dword ptr [esi]
00510b10  8b5610                   mov edx, dword ptr [esi + 0x10]
00510b13  8d3418                   lea esi, [eax + ebx]
00510b16  29d8                     sub eax, ebx
00510b18  8d1c11                   lea ebx, [ecx + edx]
00510b1b  29d1                     sub ecx, edx
00510b1d  f72df8cd5600             imul dword ptr [0x56cdf8]
00510b23  d1e2                     shl edx, 1
00510b25  8b3de4cd5600             mov edi, dword ptr [0x56cde4]
00510b2b  01d6                     add esi, edx
00510b2d  8b2deccd5600             mov ebp, dword ptr [0x56cdec]
00510b33  8d0411                   lea eax, [ecx + edx]
00510b36  29d1                     sub ecx, edx
00510b38  8d1433                   lea edx, [ebx + esi]
00510b3b  29f3                     sub ebx, esi
00510b3d  8d3429                   lea esi, [ecx + ebp]
00510b40  29e9                     sub ecx, ebp
00510b42  8b2de8cd5600             mov ebp, dword ptr [0x56cde8]
00510b48  897748                   mov dword ptr [edi + 0x48], esi
00510b4b  8b35f0cd5600             mov esi, dword ptr [0x56cdf0]
00510b51  898fb4000000             mov dword ptr [edi + 0xb4], ecx
00510b57  8d0c2b                   lea ecx, [ebx + ebp]
00510b5a  29eb                     sub ebx, ebp
00510b5c  8b2df4cd5600             mov ebp, dword ptr [0x56cdf4]
00510b62  894f6c                   mov dword ptr [edi + 0x6c], ecx
00510b65  899f90000000             mov dword ptr [edi + 0x90], ebx
00510b6b  8d1c30                   lea ebx, [eax + esi]
00510b6e  29f0                     sub eax, esi
00510b70  8d0c2a                   lea ecx, [edx + ebp]
00510b73  29ea                     sub edx, ebp
00510b75  890f                     mov dword ptr [edi], ecx
00510b77  895f24                   mov dword ptr [edi + 0x24], ebx
00510b7a  8987d8000000             mov dword ptr [edi + 0xd8], eax
00510b80  8997fc000000             mov dword ptr [edi + 0xfc], edx
00510b86  c3                       ret 
00510b87  8b06                     mov eax, dword ptr [esi]
00510b89  8b3de4cd5600             mov edi, dword ptr [0x56cde4]
00510b8f  8907                     mov dword ptr [edi], eax
00510b91  894724                   mov dword ptr [edi + 0x24], eax
00510b94  894748                   mov dword ptr [edi + 0x48], eax
00510b97  89476c                   mov dword ptr [edi + 0x6c], eax
00510b9a  898790000000             mov dword ptr [edi + 0x90], eax
00510ba0  8987b4000000             mov dword ptr [edi + 0xb4], eax
00510ba6  8987d8000000             mov dword ptr [edi + 0xd8], eax
00510bac  8987fc000000             mov dword ptr [edi + 0xfc], eax
00510bb2  c3                       ret 

