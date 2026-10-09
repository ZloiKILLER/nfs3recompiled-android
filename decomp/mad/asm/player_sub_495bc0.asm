==== sub_495bc0 .. 4960e0
00495bc0  56                       push esi
00495bc1  57                       push edi
00495bc2  55                       push ebp
00495bc3  89e5                     mov ebp, esp
00495bc5  83ec68                   sub esp, 0x68
00495bc8  89c7                     mov edi, eax
00495bca  8955d4                   mov dword ptr [ebp - 0x2c], edx
00495bcd  895dc4                   mov dword ptr [ebp - 0x3c], ebx
00495bd0  894dd0                   mov dword ptr [ebp - 0x30], ecx
00495bd3  ba00001000               mov edx, 0x100000
00495bd8  b888be5300               mov eax, 0x53be88
00495bdd  8b1df4435600             mov ebx, dword ptr [0x5643f4]
00495be3  e838ba0400               call 0x4e1620
00495be8  6800001000               push 0x100000
00495bed  bb02000000               mov ebx, 2
00495bf2  8945c0                   mov dword ptr [ebp - 0x40], eax
00495bf5  89c1                     mov ecx, eax
00495bf7  89da                     mov edx, ebx
00495bf9  89d8                     mov eax, ebx
00495bfb  e8809e0300               call 0x4cfa80
00495c00  ba02000000               mov edx, 2
00495c05  b953430000               mov ecx, 0x4353
00495c0a  bbffff0000               mov ebx, 0xffff
00495c0f  89c6                     mov esi, eax
00495c11  8945f8                   mov dword ptr [ebp - 8], eax
00495c14  e857a20300               call 0x4cfe70
00495c19  6a02                     push 2
00495c1b  ba01000000               mov edx, 1
00495c20  8945dc                   mov dword ptr [ebp - 0x24], eax
00495c23  89f0                     mov eax, esi
00495c25  e876a00300               call 0x4cfca0
00495c2a  89fa                     mov edx, edi
00495c2c  89f0                     mov eax, esi
00495c2e  31c9                     xor ecx, ecx
00495c30  31db                     xor ebx, ebx
00495c32  e8b9a20300               call 0x4cfef0
00495c37  8945cc                   mov dword ptr [ebp - 0x34], eax
00495c3a  89f0                     mov eax, esi
00495c3c  e81fffffff               call 0x495b60
00495c41  89c6                     mov esi, eax
00495c43  85c0                     test eax, eax
00495c45  750d                     jne 0x495c54
00495c47  688cbe5300               push 0x53be8c
00495c4c  e8bfb3f6ff               call 0x401010
00495c51  83c404                   add esp, 4
00495c54  ba01000000               mov edx, 1
00495c59  8b460c                   mov eax, dword ptr [esi + 0xc]
00495c5c  8b7dd0                   mov edi, dword ptr [ebp - 0x30]
00495c5f  a310f37900               mov dword ptr [0x79f310], eax
00495c64  8b460e                   mov eax, dword ptr [esi + 0xe]
00495c67  31c9                     xor ecx, ecx
00495c69  c1f810                   sar eax, 0x10
00495c6c  89152cf37900             mov dword ptr [0x79f32c], edx
00495c72  a324f37900               mov dword ptr [0x79f324], eax
00495c77  8b4610                   mov eax, dword ptr [esi + 0x10]
00495c7a  890d14f37900             mov dword ptr [0x79f314], ecx
00495c80  c1f810                   sar eax, 0x10
00495c83  890d34f37900             mov dword ptr [0x79f334], ecx
00495c89  a31cf37900               mov dword ptr [0x79f31c], eax
00495c8e  85ff                     test edi, edi
00495c90  7531                     jne 0x495cc3
00495c92  a124f37900               mov eax, dword ptr [0x79f324]
00495c97  8b1584435600             mov edx, dword ptr [0x564384]
00495c9d  29c2                     sub edx, eax
00495c9f  89d0                     mov eax, edx
00495ca1  c1fa1f                   sar edx, 0x1f
00495ca4  2bc2                     sub eax, edx
00495ca6  d1f8                     sar eax, 1
00495ca8  8b0d1cf37900             mov ecx, dword ptr [0x79f31c]
00495cae  8b1588435600             mov edx, dword ptr [0x564388]
00495cb4  29ca                     sub edx, ecx
00495cb6  89c7                     mov edi, eax
00495cb8  89d0                     mov eax, edx
00495cba  c1fa1f                   sar edx, 0x1f
00495cbd  2bc2                     sub eax, edx
00495cbf  d1f8                     sar eax, 1
00495cc1  eb2e                     jmp 0x495cf1
00495cc3  a184435600               mov eax, dword ptr [0x564384]
00495cc8  89c2                     mov edx, eax
00495cca  c1fa1f                   sar edx, 0x1f
00495ccd  2bc2                     sub eax, edx
00495ccf  d1f8                     sar eax, 1
00495cd1  89c7                     mov edi, eax
00495cd3  a188435600               mov eax, dword ptr [0x564388]
00495cd8  89c2                     mov edx, eax
00495cda  c1fa1f                   sar edx, 0x1f
00495cdd  2bc2                     sub eax, edx
00495cdf  d1f8                     sar eax, 1
00495ce1  8b1d24f37900             mov ebx, dword ptr [0x79f324]
00495ce7  8b151cf37900             mov edx, dword ptr [0x79f31c]
00495ced  29df                     sub edi, ebx
00495cef  29d0                     sub eax, edx
00495cf1  8b4d10                   mov ecx, dword ptr [ebp + 0x10]
00495cf4  51                       push ecx
00495cf5  8b5dd0                   mov ebx, dword ptr [ebp - 0x30]
00495cf8  53                       push ebx
00495cf9  8b55c4                   mov edx, dword ptr [ebp - 0x3c]
00495cfc  52                       push edx
00495cfd  8b0d1cf37900             mov ecx, dword ptr [0x79f31c]
00495d03  8b1d24f37900             mov ebx, dword ptr [0x79f324]
00495d09  6a06                     push 6
00495d0b  89c2                     mov edx, eax
00495d0d  89f8                     mov eax, edi
00495d0f  e88cfaffff               call 0x4957a0
00495d14  e877fbffff               call 0x495890
00495d19  31c9                     xor ecx, ecx
00495d1b  31ff                     xor edi, edi
00495d1d  894dfc                   mov dword ptr [ebp - 4], ecx
00495d20  89c1                     mov ecx, eax
00495d22  85f6                     test esi, esi
00495d24  0f84cf000000             je 0x495df9
00495d2a  85c9                     test ecx, ecx
00495d2c  0f84c7000000             je 0x495df9
00495d32  8b1e                     mov ebx, dword ptr [esi]
00495d34  8d4618                   lea eax, [esi + 0x18]
00495d37  81fb4d41446b             cmp ebx, 0x6b44414d
00495d3d  7506                     jne 0x495d45
00495d3f  31db                     xor ebx, ebx
00495d41  31d2                     xor edx, edx
00495d43  eb07                     jmp 0x495d4c
00495d45  31db                     xor ebx, ebx
00495d47  ba01000000               mov edx, 1
00495d4c  8a5e15                   mov bl, byte ptr [esi + 0x15]
00495d4f  e80cd80500               call 0x4f3560
00495d54  31c0                     xor eax, eax
00495d56  8945ec                   mov dword ptr [ebp - 0x14], eax
00495d59  8b45ec                   mov eax, dword ptr [ebp - 0x14]
00495d5c  3b051cf37900             cmp eax, dword ptr [0x79f31c]
00495d62  7d47                     jge 0x495dab
00495d64  31db                     xor ebx, ebx
00495d66  895df0                   mov dword ptr [ebp - 0x10], ebx
00495d69  8b45f0                   mov eax, dword ptr [ebp - 0x10]
00495d6c  3b0524f37900             cmp eax, dword ptr [0x79f324]
00495d72  7d31                     jge 0x495da5
00495d74  8b55ec                   mov edx, dword ptr [ebp - 0x14]
00495d77  8b1d24f37900             mov ebx, dword ptr [0x79f324]
00495d7d  0fafd3                   imul edx, ebx
00495d80  01c2                     add edx, eax
00495d82  89d0                     mov eax, edx
00495d84  c1fa1f                   sar edx, 0x1f
00495d87  2bc2                     sub eax, edx
00495d89  d1f8                     sar eax, 1
00495d8b  c1e002                   shl eax, 2
00495d8e  8d1401                   lea edx, [ecx + eax]
00495d91  8955b8                   mov dword ptr [ebp - 0x48], edx
00495d94  0345e0                   add eax, dword ptr [ebp - 0x20]
00495d97  8b55b8                   mov edx, dword ptr [ebp - 0x48]
00495d9a  e861d80500               call 0x4f3600
00495d9f  8345f010                 add dword ptr [ebp - 0x10], 0x10
00495da3  ebc4                     jmp 0x495d69
00495da5  8345ec10                 add dword ptr [ebp - 0x14], 0x10
00495da9  ebae                     jmp 0x495d59
00495dab  813e4d414465             cmp dword ptr [esi], 0x6544414d
00495db1  7403                     je 0x495db6
00495db3  894de0                   mov dword ptr [ebp - 0x20], ecx
00495db6  8b45f8                   mov eax, dword ptr [ebp - 8]
00495db9  89f2                     mov edx, esi
00495dbb  e800a60300               call 0x4d03c0
00495dc0  8b45fc                   mov eax, dword ptr [ebp - 4]
00495dc3  e8f8faffff               call 0x4958c0
00495dc8  8b0d10f37900             mov ecx, dword ptr [0x79f310]
00495dce  01cf                     add edi, ecx
00495dd0  89f8                     mov eax, edi
00495dd2  8b5dfc                   mov ebx, dword ptr [ebp - 4]
00495dd5  c1f810                   sar eax, 0x10
00495dd8  81e7ffff0000             and edi, 0xffff
00495dde  01c3                     add ebx, eax
00495de0  e8abfaffff               call 0x495890
00495de5  89c1                     mov ecx, eax
00495de7  8b45f8                   mov eax, dword ptr [ebp - 8]
00495dea  895dfc                   mov dword ptr [ebp - 4], ebx
00495ded  e86efdffff               call 0x495b60
00495df2  89c6                     mov esi, eax
00495df4  e929ffffff               jmp 0x495d22
00495df9  8d4598                   lea eax, [ebp - 0x68]
00495dfc  ba1e000000               mov edx, 0x1e
00495e01  e81a3d0500               call 0x4e9b20
00495e06  b801000000               mov eax, 1
00495e0b  e800dc0500               call 0x4f3a10
00495e10  8b1df4435600             mov ebx, dword ptr [0x5643f4]
00495e16  89c1                     mov ecx, eax
00495e18  89c2                     mov edx, eax
00495e1a  b888be5300               mov eax, 0x53be88
00495e1f  e8fcb70400               call 0x4e1620
00495e24  bb01000000               mov ebx, 1
00495e29  51                       push ecx
00495e2a  8d5598                   lea edx, [ebp - 0x68]
00495e2d  8945c8                   mov dword ptr [ebp - 0x38], eax
00495e30  50                       push eax
00495e31  b91e000000               mov ecx, 0x1e
00495e36  8b45dc                   mov eax, dword ptr [ebp - 0x24]
00495e39  e822dc0500               call 0x4f3a60
00495e3e  89c1                     mov ecx, eax
00495e40  8945bc                   mov dword ptr [ebp - 0x44], eax
00495e43  8b45dc                   mov eax, dword ptr [ebp - 0x24]
00495e46  e815a60300               call 0x4d0460
00495e4b  85c0                     test eax, eax
00495e4d  7507                     jne 0x495e56
00495e4f  b9ffffffff               mov ecx, 0xffffffff
00495e54  eb0e                     jmp 0x495e64
00495e56  8b5dcc                   mov ebx, dword ptr [ebp - 0x34]
00495e59  89c8                     mov eax, ecx
00495e5b  31d2                     xor edx, edx
00495e5d  e81edc0500               call 0x4f3a80
00495e62  89c1                     mov ecx, eax
00495e64  89c8                     mov eax, ecx
00495e66  e8a5fbffff               call 0x495a10
00495e6b  31c0                     xor eax, eax
00495e6d  a330f37900               mov dword ptr [0x79f330], eax
00495e72  85f6                     test esi, esi
00495e74  0f84f8010000             je 0x496072
00495e7a  833d30f3790000           cmp dword ptr [0x79f330], 0
00495e81  0f85eb010000             jne 0x496072
00495e87  31c0                     xor eax, eax
00495e89  e8a2170500               call 0x4e7630
00495e8e  e8ad940400               call 0x4df340
00495e93  83f802                   cmp eax, 2
00495e96  7509                     jne 0x495ea1
00495e98  8b45d4                   mov eax, dword ptr [ebp - 0x2c]
00495e9b  c70001000000             mov dword ptr [eax], 1
00495ea1  e8aafbffff               call 0x495a50
00495ea6  8b16                     mov edx, dword ptr [esi]
00495ea8  89c3                     mov ebx, eax
00495eaa  81fa4d414465             cmp edx, 0x6544414d
00495eb0  7557                     jne 0x495f09
00495eb2  8b55d4                   mov edx, dword ptr [ebp - 0x2c]
00495eb5  833a00                   cmp dword ptr [edx], 0
00495eb8  750d                     jne 0x495ec7
00495eba  8b55fc                   mov edx, dword ptr [ebp - 4]
00495ebd  29c2                     sub edx, eax
00495ebf  81fa85000000             cmp edx, 0x85
00495ec5  7d42                     jge 0x495f09
00495ec7  a134f37900               mov eax, dword ptr [0x79f334]
00495ecc  8b1510f37900             mov edx, dword ptr [0x79f310]
00495ed2  40                       inc eax
00495ed3  01d7                     add edi, edx
00495ed5  a334f37900               mov dword ptr [0x79f334], eax
00495eda  89f8                     mov eax, edi
00495edc  8b55fc                   mov edx, dword ptr [ebp - 4]
00495edf  c1f810                   sar eax, 0x10
00495ee2  01c2                     add edx, eax
00495ee4  8b45f8                   mov eax, dword ptr [ebp - 8]
00495ee7  8955fc                   mov dword ptr [ebp - 4], edx
00495eea  89f2                     mov edx, esi
00495eec  e8cfa40300               call 0x4d03c0
00495ef1  8b45f8                   mov eax, dword ptr [ebp - 8]
00495ef4  81e7ffff0000             and edi, 0xffff
00495efa  e861fcffff               call 0x495b60
00495eff  89c6                     mov esi, eax
00495f01  85c0                     test eax, eax
00495f03  0f8469010000             je 0x496072
00495f09  8b45fc                   mov eax, dword ptr [ebp - 4]
00495f0c  29d8                     sub eax, ebx
00495f0e  83f843                   cmp eax, 0x43
00495f11  7d59                     jge 0x495f6c
00495f13  813e4d414465             cmp dword ptr [esi], 0x6544414d
00495f19  7408                     je 0x495f23
00495f1b  ff0514f37900             inc dword ptr [0x79f314]
00495f21  eb06                     jmp 0x495f29
00495f23  ff0534f37900             inc dword ptr [0x79f334]
00495f29  8b1d10f37900             mov ebx, dword ptr [0x79f310]
00495f2f  01df                     add edi, ebx
00495f31  89f8                     mov eax, edi
00495f33  8b55fc                   mov edx, dword ptr [ebp - 4]
00495f36  c1f810                   sar eax, 0x10
00495f39  01c2                     add edx, eax
00495f3b  8b45f8                   mov eax, dword ptr [ebp - 8]
00495f3e  8955fc                   mov dword ptr [ebp - 4], edx
00495f41  89f2                     mov edx, esi
00495f43  e878a40300               call 0x4d03c0
00495f48  8b45f8                   mov eax, dword ptr [ebp - 8]
00495f4b  81e7ffff0000             and edi, 0xffff
00495f51  e80afcffff               call 0x495b60
00495f56  89c6                     mov esi, eax
00495f58  85c0                     test eax, eax
00495f5a  7408                     je 0x495f64
00495f5c  81384d41446b             cmp dword ptr [eax], 0x6b44414d
00495f62  75af                     jne 0x495f13
00495f64  85f6                     test esi, esi
00495f66  0f8406010000             je 0x496072
00495f6c  e81ff9ffff               call 0x495890
00495f71  8945e4                   mov dword ptr [ebp - 0x1c], eax
00495f74  837de400                 cmp dword ptr [ebp - 0x1c], 0
00495f78  750c                     jne 0x495f86
00495f7a  e8d1faffff               call 0x495a50
00495f7f  e89cf9ffff               call 0x495920
00495f84  ebe6                     jmp 0x495f6c
00495f86  8b16                     mov edx, dword ptr [esi]
00495f88  8d4618                   lea eax, [esi + 0x18]
00495f8b  81fa4d41446b             cmp edx, 0x6b44414d
00495f91  7506                     jne 0x495f99
00495f93  31db                     xor ebx, ebx
00495f95  31d2                     xor edx, edx
00495f97  eb07                     jmp 0x495fa0
00495f99  31db                     xor ebx, ebx
00495f9b  ba01000000               mov edx, 1
00495fa0  8a5e15                   mov bl, byte ptr [esi + 0x15]
00495fa3  e8b8d50500               call 0x4f3560
00495fa8  31db                     xor ebx, ebx
00495faa  895df4                   mov dword ptr [ebp - 0xc], ebx
00495fad  895dd8                   mov dword ptr [ebp - 0x28], ebx
00495fb0  8b45f4                   mov eax, dword ptr [ebp - 0xc]
00495fb3  3b051cf37900             cmp eax, dword ptr [0x79f31c]
00495fb9  7d5c                     jge 0x496017
00495fbb  31db                     xor ebx, ebx
00495fbd  895de8                   mov dword ptr [ebp - 0x18], ebx
00495fc0  8b45e8                   mov eax, dword ptr [ebp - 0x18]
00495fc3  3b0524f37900             cmp eax, dword ptr [0x79f324]
00495fc9  7d33                     jge 0x495ffe
00495fcb  8b55f4                   mov edx, dword ptr [ebp - 0xc]
00495fce  8b1d24f37900             mov ebx, dword ptr [0x79f324]
00495fd4  0fafd3                   imul edx, ebx
00495fd7  01c2                     add edx, eax
00495fd9  89d0                     mov eax, edx
00495fdb  c1fa1f                   sar edx, 0x1f
00495fde  2bc2                     sub eax, edx
00495fe0  d1f8                     sar eax, 1
00495fe2  8b55e4                   mov edx, dword ptr [ebp - 0x1c]
00495fe5  c1e002                   shl eax, 2
00495fe8  01c2                     add edx, eax
00495fea  8955b8                   mov dword ptr [ebp - 0x48], edx
00495fed  0345e0                   add eax, dword ptr [ebp - 0x20]
00495ff0  8b55b8                   mov edx, dword ptr [ebp - 0x48]
00495ff3  e808d60500               call 0x4f3600
00495ff8  8345e810                 add dword ptr [ebp - 0x18], 0x10
00495ffc  ebc2                     jmp 0x495fc0
00495ffe  837dd800                 cmp dword ptr [ebp - 0x28], 0
00496002  750d                     jne 0x496011
00496004  e847faffff               call 0x495a50
00496009  e812f9ffff               call 0x495920
0049600e  8945d8                   mov dword ptr [ebp - 0x28], eax
00496011  8345f410                 add dword ptr [ebp - 0xc], 0x10
00496015  eb99                     jmp 0x495fb0
00496017  813e4d414465             cmp dword ptr [esi], 0x6544414d
0049601d  7406                     je 0x496025
0049601f  8b45e4                   mov eax, dword ptr [ebp - 0x1c]
00496022  8945e0                   mov dword ptr [ebp - 0x20], eax
00496025  8b45f8                   mov eax, dword ptr [ebp - 8]
00496028  89f2                     mov edx, esi
0049602a  e891a30300               call 0x4d03c0
0049602f  8b45fc                   mov eax, dword ptr [ebp - 4]
00496032  e889f8ffff               call 0x4958c0
00496037  85c9                     test ecx, ecx
00496039  7d08                     jge 0x496043
0049603b  8b45dc                   mov eax, dword ptr [ebp - 0x24]
0049603e  e8edfaffff               call 0x495b30
00496043  8b3510f37900             mov esi, dword ptr [0x79f310]
00496049  01f7                     add edi, esi
0049604b  89f8                     mov eax, edi
0049604d  8b55fc                   mov edx, dword ptr [ebp - 4]
00496050  c1f810                   sar eax, 0x10
00496053  81e7ffff0000             and edi, 0xffff
00496059  01c2                     add edx, eax
0049605b  8b45f8                   mov eax, dword ptr [ebp - 8]
0049605e  8955fc                   mov dword ptr [ebp - 4], edx
00496061  e8fafaffff               call 0x495b60
00496066  89c6                     mov esi, eax
00496068  e8a3faffff               call 0x495b10
0049606d  e900feffff               jmp 0x495e72
00496072  e8d9f9ffff               call 0x495a50
00496077  3b45fc                   cmp eax, dword ptr [ebp - 4]
0049607a  7d0e                     jge 0x49608a
0049607c  e89ff8ffff               call 0x495920
00496081  31c0                     xor eax, eax
00496083  e8a8150500               call 0x4e7630
00496088  ebe8                     jmp 0x496072
0049608a  85c9                     test ecx, ecx
0049608c  7c25                     jl 0x4960b3
0049608e  8d55a8                   lea edx, [ebp - 0x58]
00496091  89c8                     mov eax, ecx
00496093  e8b8ce0500               call 0x4f2f50
00496098  31c0                     xor eax, eax
0049609a  e891150500               call 0x4e7630
0049609f  e86cfaffff               call 0x495b10
004960a4  837da803                 cmp dword ptr [ebp - 0x58], 3
004960a8  7409                     je 0x4960b3
004960aa  833d30f3790000           cmp dword ptr [0x79f330], 0
004960b1  74db                     je 0x49608e
004960b3  8b45bc                   mov eax, dword ptr [ebp - 0x44]
004960b6  e805e30500               call 0x4f43c0
004960bb  8b45f8                   mov eax, dword ptr [ebp - 8]
004960be  e86d9c0300               call 0x4cfd30
004960c3  8b45c0                   mov eax, dword ptr [ebp - 0x40]
004960c6  e8c5b70400               call 0x4e1890
004960cb  8b45c8                   mov eax, dword ptr [ebp - 0x38]
004960ce  e8bdb70400               call 0x4e1890
004960d3  e888f7ffff               call 0x495860
004960d8  89ec                     mov esp, ebp
004960da  5d                       pop ebp
004960db  5f                       pop edi
004960dc  5e                       pop esi
004960dd  c20400                   ret 4

