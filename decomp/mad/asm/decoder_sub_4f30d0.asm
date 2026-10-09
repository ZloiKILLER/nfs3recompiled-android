==== sub_4f30d0 .. 4f3350
004f30d0  51                       push ecx
004f30d1  52                       push edx
004f30d2  57                       push edi
004f30d3  56                       push esi
004f30d4  53                       push ebx
004f30d5  be3f000000               mov esi, 0x3f
004f30da  bbc0ffffff               mov ebx, 0xffffffc0
004f30df  ba80ffffff               mov edx, 0xffffff80
004f30e4  89d0                     mov eax, edx
004f30e6  83fac0                   cmp edx, -0x40
004f30e9  0f8dbd010000             jge 0x4f32ac
004f30ef  89d8                     mov eax, ebx
004f30f1  89d1                     mov ecx, edx
004f30f3  81e1ff000000             and ecx, 0xff
004f30f9  0440                     add al, 0x40
004f30fb  42                       inc edx
004f30fc  8881380d9f00             mov byte ptr [ecx + 0x9f0d38], al
004f3102  83fa7f                   cmp edx, 0x7f
004f3105  7cdd                     jl 0x4f30e4
004f3107  ba0f000000               mov edx, 0xf
004f310c  b804000000               mov eax, 4
004f3111  b91f000000               mov ecx, 0x1f
004f3116  891538199f00             mov dword ptr [0x9f1938], edx
004f311c  83c004                   add eax, 4
004f311f  898834199f00             mov dword ptr [eax + 0x9f1934], ecx
004f3125  83f820                   cmp eax, 0x20
004f3128  75f2                     jne 0x4f311c
004f312a  bb2f000000               mov ebx, 0x2f
004f312f  31c0                     xor eax, eax
004f3131  83c004                   add eax, 4
004f3134  899854199f00             mov dword ptr [eax + 0x9f1954], ebx
004f313a  83f820                   cmp eax, 0x20
004f313d  75f2                     jne 0x4f3131
004f313f  be3f000000               mov esi, 0x3f
004f3144  31c0                     xor eax, eax
004f3146  83c004                   add eax, 4
004f3149  89b0341d9f00             mov dword ptr [eax + 0x9f1d34], esi
004f314f  3d00020000               cmp eax, 0x200
004f3154  75f0                     jne 0x4f3146
004f3156  bb10000000               mov ebx, 0x10
004f315b  8b83d8525600             mov eax, dword ptr [ebx + 0x5652d8]
004f3161  89c6                     mov esi, eax
004f3163  8b8be0525600             mov ecx, dword ptr [ebx + 0x5652e0]
004f3169  81e600fc0000             and esi, 0xfc00
004f316f  25ff030000               and eax, 0x3ff
004f3174  c1e606                   shl esi, 6
004f3177  c1e016                   shl eax, 0x16
004f317a  8b93d4525600             mov edx, dword ptr [ebx + 0x5652d4]
004f3180  09f0                     or eax, esi
004f3182  f6c5fc                   test ch, 0xfc
004f3185  0f8431010000             je 0x4f32bc
004f318b  89ce                     mov esi, ecx
004f318d  b909000000               mov ecx, 9
004f3192  bf01000000               mov edi, 1
004f3197  29d1                     sub ecx, edx
004f3199  09d0                     or eax, edx
004f319b  d3e7                     shl edi, cl
004f319d  c1fe07                   sar esi, 7
004f31a0  89f9                     mov ecx, edi
004f31a2  85ff                     test edi, edi
004f31a4  7e1b                     jle 0x4f31c1
004f31a6  c1e602                   shl esi, 2
004f31a9  8d0cbd00000000           lea ecx, [edi*4]
004f31b0  89f2                     mov edx, esi
004f31b2  01f1                     add ecx, esi
004f31b4  83c204                   add edx, 4
004f31b7  898234199f00             mov dword ptr [edx + 0x9f1934], eax
004f31bd  39ca                     cmp edx, ecx
004f31bf  7cf3                     jl 0x4f31b4
004f31c1  83c310                   add ebx, 0x10
004f31c4  81fbf0050000             cmp ebx, 0x5f0
004f31ca  758f                     jne 0x4f315b
004f31cc  31db                     xor ebx, ebx
004f31ce  8b83c8585600             mov eax, dword ptr [ebx + 0x5658c8]
004f31d4  8b93c4585600             mov edx, dword ptr [ebx + 0x5658c4]
004f31da  89c6                     mov esi, eax
004f31dc  8b8bd0585600             mov ecx, dword ptr [ebx + 0x5658d0]
004f31e2  81e600fc0000             and esi, 0xfc00
004f31e8  25ff030000               and eax, 0x3ff
004f31ed  c1e606                   shl esi, 6
004f31f0  c1e016                   shl eax, 0x16
004f31f3  83c208                   add edx, 8
004f31f6  09f0                     or eax, esi
004f31f8  f6c580                   test ch, 0x80
004f31fb  0f85fe000000             jne 0x4f32ff
004f3201  89ce                     mov esi, ecx
004f3203  83ea09                   sub edx, 9
004f3206  b908000000               mov ecx, 8
004f320b  bf01000000               mov edi, 1
004f3210  29d1                     sub ecx, edx
004f3212  c1fe07                   sar esi, 7
004f3215  d3e7                     shl edi, cl
004f3217  09d0                     or eax, edx
004f3219  89f9                     mov ecx, edi
004f321b  85ff                     test edi, edi
004f321d  7e1b                     jle 0x4f323a
004f321f  c1e602                   shl esi, 2
004f3222  8d0cbd00000000           lea ecx, [edi*4]
004f3229  89f2                     mov edx, esi
004f322b  01f1                     add ecx, esi
004f322d  83c204                   add edx, 4
004f3230  898234119f00             mov dword ptr [edx + 0x9f1134], eax
004f3236  39ca                     cmp edx, ecx
004f3238  7cf3                     jl 0x4f322d
004f323a  83c310                   add ebx, 0x10
004f323d  81fb00080000             cmp ebx, 0x800
004f3243  7589                     jne 0x4f31ce
004f3245  bf01000000               mov edi, 1
004f324a  31c0                     xor eax, eax
004f324c  5b                       pop ebx
004f324d  5e                       pop esi
004f324e  83c004                   add eax, 4
004f3251  89b834109f00             mov dword ptr [eax + 0x9f1034], edi
004f3257  3d80000000               cmp eax, 0x80
004f325c  75f0                     jne 0x4f324e
004f325e  ba00004000               mov edx, 0x400000
004f3263  31c0                     xor eax, eax
004f3265  89d1                     mov ecx, edx
004f3267  83c004                   add eax, 4
004f326a  80c906                   or cl, 6
004f326d  81c200004000             add edx, 0x400000
004f3273  8988b4109f00             mov dword ptr [eax + 0x9f10b4], ecx
004f3279  83f840                   cmp eax, 0x40
004f327c  75e7                     jne 0x4f3265
004f327e  ba000000fc               mov edx, 0xfc000000
004f3283  31c0                     xor eax, eax
004f3285  89d1                     mov ecx, edx
004f3287  83c004                   add eax, 4
004f328a  80c906                   or cl, 6
004f328d  81c200004000             add edx, 0x400000
004f3293  8988f4109f00             mov dword ptr [eax + 0x9f10f4], ecx
004f3299  83f840                   cmp eax, 0x40
004f329c  75e7                     jne 0x4f3285
004f329e  c70538259f0001000000     mov dword ptr [0x9f2538], 1
004f32a8  5f                       pop edi
004f32a9  5a                       pop edx
004f32aa  59                       pop ecx
004f32ab  c3                       ret 
004f32ac  83fa3f                   cmp edx, 0x3f
004f32af  0f8e3cfeffff             jle 0x4f30f1
004f32b5  89f0                     mov eax, esi
004f32b7  e935feffff               jmp 0x4f30f1
004f32bc  89ce                     mov esi, ecx
004f32be  83ea06                   sub edx, 6
004f32c1  b908000000               mov ecx, 8
004f32c6  bf01000000               mov edi, 1
004f32cb  29d1                     sub ecx, edx
004f32cd  c1fe02                   sar esi, 2
004f32d0  d3e7                     shl edi, cl
004f32d2  09d0                     or eax, edx
004f32d4  89f9                     mov ecx, edi
004f32d6  85ff                     test edi, edi
004f32d8  0f8ee3feffff             jle 0x4f31c1
004f32de  c1e602                   shl esi, 2
004f32e1  8d0cbd00000000           lea ecx, [edi*4]
004f32e8  89f2                     mov edx, esi
004f32ea  01f1                     add ecx, esi
004f32ec  83c204                   add edx, 4
004f32ef  898234159f00             mov dword ptr [edx + 0x9f1534], eax
004f32f5  39ca                     cmp edx, ecx
004f32f7  0f8dc4feffff             jge 0x4f31c1
004f32fd  ebed                     jmp 0x4f32ec
004f32ff  89ce                     mov esi, ecx
004f3301  83ea06                   sub edx, 6
004f3304  b908000000               mov ecx, 8
004f3309  bf01000000               mov edi, 1
004f330e  29d1                     sub ecx, edx
004f3310  c1fe0a                   sar esi, 0xa
004f3313  d3e7                     shl edi, cl
004f3315  09d0                     or eax, edx
004f3317  89f9                     mov ecx, edi
004f3319  85ff                     test edi, edi
004f331b  0f8e19ffffff             jle 0x4f323a
004f3321  c1e602                   shl esi, 2
004f3324  8d0cbd00000000           lea ecx, [edi*4]
004f332b  89f2                     mov edx, esi
004f332d  01f1                     add ecx, esi
004f332f  83c204                   add edx, 4
004f3332  898234159f00             mov dword ptr [edx + 0x9f1534], eax
004f3338  39ca                     cmp edx, ecx
004f333a  0f8dfafeffff             jge 0x4f323a
004f3340  ebed                     jmp 0x4f332f
004f3342  8d8000000000             lea eax, [eax]
004f3348  8d9200000000             lea edx, [edx]
004f334e  8bc0                     mov eax, eax

